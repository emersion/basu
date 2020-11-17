/* SPDX-License-Identifier: LGPL-2.1+ */

#include <sys/sysmacros.h>
#include <unistd.h>

#include "alloc-util.h"
#include "env-util.h"
#include "fileio.h"
#include "fs-util.h"
#include "path-util.h"
#include "process-util.h"
#include "string-util.h"
#include "terminal-util.h"

static volatile int cached_on_tty = -1;
static volatile int cached_colors_enabled = -1;
static volatile int cached_underline_enabled = -1;

bool on_tty(void) {

        /* We check both stdout and stderr, so that situations where pipes on the shell are used are reliably
         * recognized, regardless if only the output or the errors are piped to some place. Since on_tty() is generally
         * used to default to a safer, non-interactive, non-color mode of operation it's probably good to be defensive
         * here, and check for both. Note that we don't check for STDIN_FILENO, because it should fine to use fancy
         * terminal functionality when outputting stuff, even if the input is piped to us. */

        if (cached_on_tty < 0)
                cached_on_tty =
                        isatty(STDOUT_FILENO) > 0 &&
                        isatty(STDERR_FILENO) > 0;

        return cached_on_tty;
}

int get_ctty_devnr(pid_t pid, dev_t *d) {
        int r;
        _cleanup_free_ char *line = NULL;
        const char *p;
        unsigned long ttynr;

        assert(pid >= 0);

        p = procfs_file_alloca(pid, "stat");
        r = read_one_line_file(p, &line);
        if (r < 0)
                return r;

        p = strrchr(line, ')');
        if (!p)
                return -EIO;

        p++;

        if (sscanf(p, " "
                   "%*c "  /* state */
                   "%*d "  /* ppid */
                   "%*d "  /* pgrp */
                   "%*d "  /* session */
                   "%lu ", /* ttynr */
                   &ttynr) != 1)
                return -EIO;

        if (major(ttynr) == 0 && minor(ttynr) == 0)
                return -ENXIO;

        if (d)
                *d = (dev_t) ttynr;

        return 0;
}

int get_ctty(pid_t pid, dev_t *_devnr, char **r) {
        char fn[STRLEN("/dev/char/") + 2*DECIMAL_STR_MAX(unsigned) + 1 + 1], *b = NULL;
        _cleanup_free_ char *s = NULL;
        const char *p;
        dev_t devnr;
        int k;

        assert(r);

        k = get_ctty_devnr(pid, &devnr);
        if (k < 0)
                return k;

        sprintf(fn, "/dev/char/%u:%u", major(devnr), minor(devnr));

        k = readlink_malloc(fn, &s);
        if (k < 0) {

                if (k != -ENOENT)
                        return k;

                /* This is an ugly hack */
                if (major(devnr) == 136) {
                        if (asprintf(&b, "pts/%u", minor(devnr)) < 0)
                                return -ENOMEM;
                } else {
                        /* Probably something like the ptys which have no
                         * symlink in /dev/char. Let's return something
                         * vaguely useful. */

                        b = strdup(fn + 5);
                        if (!b)
                                return -ENOMEM;
                }
        } else {
                if (startswith(s, "/dev/"))
                        p = s + 5;
                else if (startswith(s, "../"))
                        p = s + 3;
                else
                        p = s;

                b = strdup(p);
                if (!b)
                        return -ENOMEM;
        }

        *r = b;
        if (_devnr)
                *_devnr = devnr;

        return 0;
}

static bool getenv_terminal_is_dumb(void) {
        const char *e;

        e = getenv("TERM");
        if (!e)
                return true;

        return streq(e, "dumb");
}

bool terminal_is_dumb(void) {
        if (!on_tty())
                return true;

        return getenv_terminal_is_dumb();
}

bool colors_enabled(void) {

        /* Returns true if colors are considered supported on our stdout. For that we check $SYSTEMD_COLORS first
         * (which is the explicit way to turn colors on/off). If that didn't work we turn colors off unless we are on a
         * TTY. And if we are on a TTY we turn it off if $TERM is set to "dumb". There's one special tweak though: if
         * we are PID 1 then we do not check whether we are connected to a TTY, because we don't keep /dev/console open
         * continously due to fear of SAK, and hence things are a bit weird. */

        if (cached_colors_enabled < 0) {
                int val;

                val = getenv_bool("SYSTEMD_COLORS");
                if (val >= 0)
                        cached_colors_enabled = val;
                else if (getpid_cached() == 1)
                        /* PID1 outputs to the console without holding it open all the time */
                        cached_colors_enabled = !getenv_terminal_is_dumb();
                else
                        cached_colors_enabled = !terminal_is_dumb();
        }

        return cached_colors_enabled;
}

bool underline_enabled(void) {

        if (cached_underline_enabled < 0) {

                /* The Linux console doesn't support underlining, turn it off, but only there. */

                if (colors_enabled())
                        cached_underline_enabled = !streq_ptr(getenv("TERM"), "linux");
                else
                        cached_underline_enabled = false;
        }

        return cached_underline_enabled;
}

static bool urlify_enabled(void) {
        static int cached_urlify_enabled = -1;

        /* Unfortunately 'less' doesn't support links like this yet 😭, hence let's disable this as long as there's a
         * pager in effect. Let's drop this check as soon as less got fixed a and enough time passed so that it's safe
         * to assume that a link-enabled 'less' version has hit most installations. */

        if (cached_urlify_enabled < 0) {
                int val;

                val = getenv_bool("SYSTEMD_URLIFY");
                if (val >= 0)
                        cached_urlify_enabled = val;
                else
                        cached_urlify_enabled = colors_enabled();
        }

        return cached_urlify_enabled;
}

int terminal_urlify(const char *url, const char *text, char **ret) {
        char *n;

        assert(url);

        /* Takes an URL and a pretty string and formats it as clickable link for the terminal. See
         * https://gist.github.com/egmontkob/eb114294efbcd5adb1944c9f3cb5feda for details. */

        if (isempty(text))
                text = url;

        if (urlify_enabled())
                n = strjoin("\x1B]8;;", url, "\a", text, "\x1B]8;;\a");
        else
                n = strdup(text);
        if (!n)
                return -ENOMEM;

        *ret = n;
        return 0;
}

int terminal_urlify_man(const char *page, const char *section, char **ret) {
        const char *url, *text;

        url = strjoina("man:", page, "(", section, ")");
        text = strjoina(page, "(", section, ") man page");

        return terminal_urlify(url, text, ret);
}

