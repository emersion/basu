/* SPDX-License-Identifier: LGPL-2.1+ */

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "alloc-util.h"
#include "ctype.h"
#include "def.h"
#include "escape.h"
#include "fd-util.h"
#include "fileio.h"
#include "fs-util.h"
#include "hexdecoct.h"
#include "log.h"
#include "macro.h"
#include "missing.h"
#include "parse-util.h"
#include "path-util.h"
#include "process-util.h"
#include "random-util.h"
#include "stdio-util.h"
#include "string-util.h"
#include "strv.h"
#include "time-util.h"
#include "umask-util.h"
#include "utf8.h"

#define READ_FULL_BYTES_MAX (4U*1024U*1024U)

int read_one_line_file(const char *fn, char **line) {
        _cleanup_fclose_ FILE *f = NULL;
        int r;

        assert(fn);
        assert(line);

        f = fopen(fn, "re");
        if (!f)
                return -errno;

        (void) __fsetlocking(f, FSETLOCKING_BYCALLER);

        r = read_line(f, LONG_LINE_MAX, line);
        return r < 0 ? r : 0;
}

int read_full_stream(FILE *f, char **contents, size_t *size) {
        _cleanup_free_ char *buf = NULL;
        struct stat st;
        size_t n, l;
        int fd;

        assert(f);
        assert(contents);

        n = LINE_MAX;

        fd = fileno(f);
        if (fd >= 0) { /* If the FILE* object is backed by an fd (as opposed to memory or such, see fmemopen(), let's
                        * optimize our buffering) */

                if (fstat(fileno(f), &st) < 0)
                        return -errno;

                if (S_ISREG(st.st_mode)) {

                        /* Safety check */
                        if (st.st_size > READ_FULL_BYTES_MAX)
                                return -E2BIG;

                        /* Start with the right file size, but be prepared for files from /proc which generally report a file
                         * size of 0. Note that we increase the size to read here by one, so that the first read attempt
                         * already makes us notice the EOF. */
                        if (st.st_size > 0)
                                n = st.st_size + 1;
                }
        }

        l = 0;
        for (;;) {
                char *t;
                size_t k;

                t = realloc(buf, n + 1);
                if (!t)
                        return -ENOMEM;

                buf = t;
                errno = 0;
                k = fread(buf + l, 1, n - l, f);
                if (k > 0)
                        l += k;

                if (ferror(f))
                        return errno > 0 ? -errno : -EIO;

                if (feof(f))
                        break;

                /* We aren't expecting fread() to return a short read outside
                 * of (error && eof), assert buffer is full and enlarge buffer.
                 */
                assert(l == n);

                /* Safety check */
                if (n >= READ_FULL_BYTES_MAX)
                        return -E2BIG;

                n = MIN(n * 2, READ_FULL_BYTES_MAX);
        }

        buf[l] = 0;
        *contents = TAKE_PTR(buf);

        if (size)
                *size = l;

        return 0;
}

int read_full_file(const char *fn, char **contents, size_t *size) {
        _cleanup_fclose_ FILE *f = NULL;

        assert(fn);
        assert(contents);

        f = fopen(fn, "re");
        if (!f)
                return -errno;

        (void) __fsetlocking(f, FSETLOCKING_BYCALLER);

        return read_full_stream(f, contents, size);
}

static int parse_env_file_internal(
                FILE *f,
                const char *fname,
                const char *newline,
                int (*push) (const char *filename, unsigned line,
                             const char *key, char *value, void *userdata, int *n_pushed),
                void *userdata,
                int *n_pushed) {

        size_t key_alloc = 0, n_key = 0, value_alloc = 0, n_value = 0, last_value_whitespace = (size_t) -1, last_key_whitespace = (size_t) -1;
        _cleanup_free_ char *contents = NULL, *key = NULL, *value = NULL;
        unsigned line = 1;
        char *p;
        int r;

        enum {
                PRE_KEY,
                KEY,
                PRE_VALUE,
                VALUE,
                VALUE_ESCAPE,
                SINGLE_QUOTE_VALUE,
                SINGLE_QUOTE_VALUE_ESCAPE,
                DOUBLE_QUOTE_VALUE,
                DOUBLE_QUOTE_VALUE_ESCAPE,
                COMMENT,
                COMMENT_ESCAPE
        } state = PRE_KEY;

        assert(newline);

        if (f)
                r = read_full_stream(f, &contents, NULL);
        else
                r = read_full_file(fname, &contents, NULL);
        if (r < 0)
                return r;

        for (p = contents; *p; p++) {
                char c = *p;

                switch (state) {

                case PRE_KEY:
                        if (strchr(COMMENTS, c))
                                state = COMMENT;
                        else if (!strchr(WHITESPACE, c)) {
                                state = KEY;
                                last_key_whitespace = (size_t) -1;

                                if (!GREEDY_REALLOC(key, key_alloc, n_key+2))
                                        return -ENOMEM;

                                key[n_key++] = c;
                        }
                        break;

                case KEY:
                        if (strchr(newline, c)) {
                                state = PRE_KEY;
                                line++;
                                n_key = 0;
                        } else if (c == '=') {
                                state = PRE_VALUE;
                                last_value_whitespace = (size_t) -1;
                        } else {
                                if (!strchr(WHITESPACE, c))
                                        last_key_whitespace = (size_t) -1;
                                else if (last_key_whitespace == (size_t) -1)
                                         last_key_whitespace = n_key;

                                if (!GREEDY_REALLOC(key, key_alloc, n_key+2))
                                        return -ENOMEM;

                                key[n_key++] = c;
                        }

                        break;

                case PRE_VALUE:
                        if (strchr(newline, c)) {
                                state = PRE_KEY;
                                line++;
                                key[n_key] = 0;

                                if (value)
                                        value[n_value] = 0;

                                /* strip trailing whitespace from key */
                                if (last_key_whitespace != (size_t) -1)
                                        key[last_key_whitespace] = 0;

                                r = push(fname, line, key, value, userdata, n_pushed);
                                if (r < 0)
                                        return r;

                                n_key = 0;
                                value = NULL;
                                value_alloc = n_value = 0;

                        } else if (c == '\'')
                                state = SINGLE_QUOTE_VALUE;
                        else if (c == '\"')
                                state = DOUBLE_QUOTE_VALUE;
                        else if (c == '\\')
                                state = VALUE_ESCAPE;
                        else if (!strchr(WHITESPACE, c)) {
                                state = VALUE;

                                if (!GREEDY_REALLOC(value, value_alloc, n_value+2))
                                        return  -ENOMEM;

                                value[n_value++] = c;
                        }

                        break;

                case VALUE:
                        if (strchr(newline, c)) {
                                state = PRE_KEY;
                                line++;

                                key[n_key] = 0;

                                if (value)
                                        value[n_value] = 0;

                                /* Chomp off trailing whitespace from value */
                                if (last_value_whitespace != (size_t) -1)
                                        value[last_value_whitespace] = 0;

                                /* strip trailing whitespace from key */
                                if (last_key_whitespace != (size_t) -1)
                                        key[last_key_whitespace] = 0;

                                r = push(fname, line, key, value, userdata, n_pushed);
                                if (r < 0)
                                        return r;

                                n_key = 0;
                                value = NULL;
                                value_alloc = n_value = 0;

                        } else if (c == '\\') {
                                state = VALUE_ESCAPE;
                                last_value_whitespace = (size_t) -1;
                        } else {
                                if (!strchr(WHITESPACE, c))
                                        last_value_whitespace = (size_t) -1;
                                else if (last_value_whitespace == (size_t) -1)
                                        last_value_whitespace = n_value;

                                if (!GREEDY_REALLOC(value, value_alloc, n_value+2))
                                        return -ENOMEM;

                                value[n_value++] = c;
                        }

                        break;

                case VALUE_ESCAPE:
                        state = VALUE;

                        if (!strchr(newline, c)) {
                                /* Escaped newlines we eat up entirely */
                                if (!GREEDY_REALLOC(value, value_alloc, n_value+2))
                                        return -ENOMEM;

                                value[n_value++] = c;
                        }
                        break;

                case SINGLE_QUOTE_VALUE:
                        if (c == '\'')
                                state = PRE_VALUE;
                        else if (c == '\\')
                                state = SINGLE_QUOTE_VALUE_ESCAPE;
                        else {
                                if (!GREEDY_REALLOC(value, value_alloc, n_value+2))
                                        return -ENOMEM;

                                value[n_value++] = c;
                        }

                        break;

                case SINGLE_QUOTE_VALUE_ESCAPE:
                        state = SINGLE_QUOTE_VALUE;

                        if (!strchr(newline, c)) {
                                if (!GREEDY_REALLOC(value, value_alloc, n_value+2))
                                        return -ENOMEM;

                                value[n_value++] = c;
                        }
                        break;

                case DOUBLE_QUOTE_VALUE:
                        if (c == '\"')
                                state = PRE_VALUE;
                        else if (c == '\\')
                                state = DOUBLE_QUOTE_VALUE_ESCAPE;
                        else {
                                if (!GREEDY_REALLOC(value, value_alloc, n_value+2))
                                        return -ENOMEM;

                                value[n_value++] = c;
                        }

                        break;

                case DOUBLE_QUOTE_VALUE_ESCAPE:
                        state = DOUBLE_QUOTE_VALUE;

                        if (!strchr(newline, c)) {
                                if (!GREEDY_REALLOC(value, value_alloc, n_value+2))
                                        return -ENOMEM;

                                value[n_value++] = c;
                        }
                        break;

                case COMMENT:
                        if (c == '\\')
                                state = COMMENT_ESCAPE;
                        else if (strchr(newline, c)) {
                                state = PRE_KEY;
                                line++;
                        }
                        break;

                case COMMENT_ESCAPE:
                        state = COMMENT;
                        break;
                }
        }

        if (IN_SET(state,
                   PRE_VALUE,
                   VALUE,
                   VALUE_ESCAPE,
                   SINGLE_QUOTE_VALUE,
                   SINGLE_QUOTE_VALUE_ESCAPE,
                   DOUBLE_QUOTE_VALUE,
                   DOUBLE_QUOTE_VALUE_ESCAPE)) {

                key[n_key] = 0;

                if (value)
                        value[n_value] = 0;

                if (state == VALUE)
                        if (last_value_whitespace != (size_t) -1)
                                value[last_value_whitespace] = 0;

                /* strip trailing whitespace from key */
                if (last_key_whitespace != (size_t) -1)
                        key[last_key_whitespace] = 0;

                r = push(fname, line, key, value, userdata, n_pushed);
                if (r < 0)
                        return r;

                value = NULL;
        }

        return 0;
}

static int check_utf8ness_and_warn(
                const char *filename, unsigned line,
                const char *key, char *value) {

        if (!utf8_is_valid(key)) {
                _cleanup_free_ char *p = NULL;

                p = utf8_escape_invalid(key);
                log_error("%s:%u: invalid UTF-8 in key '%s', ignoring.", strna(filename), line, p);
                return -EINVAL;
        }

        if (value && !utf8_is_valid(value)) {
                _cleanup_free_ char *p = NULL;

                p = utf8_escape_invalid(value);
                log_error("%s:%u: invalid UTF-8 value for key %s: '%s', ignoring.", strna(filename), line, key, p);
                return -EINVAL;
        }

        return 0;
}

static int parse_env_file_push(
                const char *filename, unsigned line,
                const char *key, char *value,
                void *userdata,
                int *n_pushed) {

        const char *k;
        va_list aq, *ap = userdata;
        int r;

        r = check_utf8ness_and_warn(filename, line, key, value);
        if (r < 0)
                return r;

        va_copy(aq, *ap);

        while ((k = va_arg(aq, const char *))) {
                char **v;

                v = va_arg(aq, char **);

                if (streq(key, k)) {
                        va_end(aq);
                        free(*v);
                        *v = value;

                        if (n_pushed)
                                (*n_pushed)++;

                        return 1;
                }
        }

        va_end(aq);
        free(value);

        return 0;
}

int parse_env_filev(
                FILE *f,
                const char *fname,
                const char *newline,
                va_list ap) {

        int r, n_pushed = 0;
        va_list aq;

        if (!newline)
                newline = NEWLINE;

        va_copy(aq, ap);
        r = parse_env_file_internal(f, fname, newline, parse_env_file_push, &aq, &n_pushed);
        va_end(aq);
        if (r < 0)
                return r;

        return n_pushed;
}

int parse_env_file(
                FILE *f,
                const char *fname,
                const char *newline,
                ...) {

        va_list ap;
        int r;

        va_start(ap, newline);
        r = parse_env_filev(f, fname, newline, ap);
        va_end(ap);

        return r;
}

static int load_env_file_push_pairs(
                const char *filename, unsigned line,
                const char *key, char *value,
                void *userdata,
                int *n_pushed) {
        char ***m = userdata;
        int r;

        r = check_utf8ness_and_warn(filename, line, key, value);
        if (r < 0)
                return r;

        r = strv_extend(m, key);
        if (r < 0)
                return -ENOMEM;

        if (!value) {
                r = strv_extend(m, "");
                if (r < 0)
                        return -ENOMEM;
        } else {
                r = strv_push(m, value);
                if (r < 0)
                        return r;
        }

        if (n_pushed)
                (*n_pushed)++;

        return 0;
}

int load_env_file_pairs(FILE *f, const char *fname, const char *newline, char ***rl) {
        char **m = NULL;
        int r;

        if (!newline)
                newline = NEWLINE;

        r = parse_env_file_internal(f, fname, newline, load_env_file_push_pairs, &m, NULL);
        if (r < 0) {
                strv_free(m);
                return r;
        }

        *rl = m;
        return 0;
}

int fflush_and_check(FILE *f) {
        assert(f);

        errno = 0;
        fflush(f);

        if (ferror(f))
                return errno > 0 ? -errno : -EIO;

        return 0;
}

int fputs_with_space(FILE *f, const char *s, const char *separator, bool *space) {
        int r;

        assert(s);

        /* Outputs the specified string with fputs(), but optionally prefixes it with a separator. The *space parameter
         * when specified shall initially point to a boolean variable initialized to false. It is set to true after the
         * first invocation. This call is supposed to be use in loops, where a separator shall be inserted between each
         * element, but not before the first one. */

        if (!f)
                f = stdout;

        if (space) {
                if (!separator)
                        separator = " ";

                if (*space) {
                        r = fputs(separator, f);
                        if (r < 0)
                                return r;
                }

                *space = true;
        }

        return fputs(s, f);
}

DEFINE_TRIVIAL_CLEANUP_FUNC(FILE*, funlockfile);

int read_line(FILE *f, size_t limit, char **ret) {
        _cleanup_free_ char *buffer = NULL;
        size_t n = 0, allocated = 0, count = 0;

        assert(f);

        /* Something like a bounded version of getline().
         *
         * Considers EOF, \n and \0 end of line delimiters, and does not include these delimiters in the string
         * returned.
         *
         * Returns the number of bytes read from the files (i.e. including delimiters — this hence usually differs from
         * the number of characters in the returned string). When EOF is hit, 0 is returned.
         *
         * The input parameter limit is the maximum numbers of characters in the returned string, i.e. excluding
         * delimiters. If the limit is hit we fail and return -ENOBUFS.
         *
         * If a line shall be skipped ret may be initialized as NULL. */

        if (ret) {
                if (!GREEDY_REALLOC(buffer, allocated, 1))
                        return -ENOMEM;
        }

        {
                _unused_ _cleanup_(funlockfilep) FILE *flocked = f;
                flockfile(f);

                for (;;) {
                        int c;

                        if (n >= limit)
                                return -ENOBUFS;

                        errno = 0;
                        c = fgetc_unlocked(f);
                        if (c == EOF) {
                                /* if we read an error, and have no data to return, then propagate the error */
                                if (ferror_unlocked(f) && n == 0)
                                        return errno > 0 ? -errno : -EIO;

                                break;
                        }

                        count++;

                        if (IN_SET(c, '\n', 0)) /* Reached a delimiter */
                                break;

                        if (ret) {
                                if (!GREEDY_REALLOC(buffer, allocated, n + 2))
                                        return -ENOMEM;

                                buffer[n] = (char) c;
                        }

                        n++;
                }
        }

        if (ret) {
                buffer[n] = 0;

                *ret = TAKE_PTR(buffer);
        }

        return (int) count;
}
