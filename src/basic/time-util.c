/* SPDX-License-Identifier: LGPL-2.1+ */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <sys/timex.h>
#include <sys/types.h>
#include <unistd.h>

#include "alloc-util.h"
#include "def.h"
#include "fd-util.h"
#include "fileio.h"
#include "fs-util.h"
#include "io-util.h"
#include "log.h"
#include "macro.h"
#include "parse-util.h"
#include "path-util.h"
#include "process-util.h"
#include "stat-util.h"
#include "string-util.h"
#include "strv.h"
#include "time-util.h"

static clockid_t map_clock_id(clockid_t c) {

        /* Some more exotic archs (s390, ppc, …) lack the "ALARM" flavour of the clocks. Thus, clock_gettime() will
         * fail for them. Since they are essentially the same as their non-ALARM pendants (their only difference is
         * when timers are set on them), let's just map them accordingly. This way, we can get the correct time even on
         * those archs. */

        switch (c) {

        case CLOCK_BOOTTIME_ALARM:
                return CLOCK_BOOTTIME;

        case CLOCK_REALTIME_ALARM:
                return CLOCK_REALTIME;

        default:
                return c;
        }
}

usec_t now(clockid_t clock_id) {
        struct timespec ts;

        assert_se(clock_gettime(map_clock_id(clock_id), &ts) == 0);

        return timespec_load(&ts);
}

dual_timestamp* dual_timestamp_get(dual_timestamp *ts) {
        assert(ts);

        ts->realtime = now(CLOCK_REALTIME);
        ts->monotonic = now(CLOCK_MONOTONIC);

        return ts;
}

triple_timestamp* triple_timestamp_get(triple_timestamp *ts) {
        assert(ts);

        ts->realtime = now(CLOCK_REALTIME);
        ts->monotonic = now(CLOCK_MONOTONIC);
        ts->boottime = clock_boottime_supported() ? now(CLOCK_BOOTTIME) : USEC_INFINITY;

        return ts;
}

usec_t triple_timestamp_by_clock(triple_timestamp *ts, clockid_t clock) {

        switch (clock) {

        case CLOCK_REALTIME:
        case CLOCK_REALTIME_ALARM:
                return ts->realtime;

        case CLOCK_MONOTONIC:
                return ts->monotonic;

        case CLOCK_BOOTTIME:
        case CLOCK_BOOTTIME_ALARM:
                return ts->boottime;

        default:
                return USEC_INFINITY;
        }
}

usec_t timespec_load(const struct timespec *ts) {
        assert(ts);

        if (ts->tv_sec < 0 || ts->tv_nsec < 0)
                return USEC_INFINITY;

        if ((usec_t) ts->tv_sec > (UINT64_MAX - (ts->tv_nsec / NSEC_PER_USEC)) / USEC_PER_SEC)
                return USEC_INFINITY;

        return
                (usec_t) ts->tv_sec * USEC_PER_SEC +
                (usec_t) ts->tv_nsec / NSEC_PER_USEC;
}

nsec_t timespec_load_nsec(const struct timespec *ts) {
        assert(ts);

        if (ts->tv_sec < 0 || ts->tv_nsec < 0)
                return NSEC_INFINITY;

        if ((nsec_t) ts->tv_sec >= (UINT64_MAX - ts->tv_nsec) / NSEC_PER_SEC)
                return NSEC_INFINITY;

        return (nsec_t) ts->tv_sec * NSEC_PER_SEC + (nsec_t) ts->tv_nsec;
}

struct timespec *timespec_store(struct timespec *ts, usec_t u)  {
        assert(ts);

        if (u == USEC_INFINITY ||
            u / USEC_PER_SEC >= TIME_T_MAX) {
                ts->tv_sec = (time_t) -1;
                ts->tv_nsec = (long) -1;
                return ts;
        }

        ts->tv_sec = (time_t) (u / USEC_PER_SEC);
        ts->tv_nsec = (long int) ((u % USEC_PER_SEC) * NSEC_PER_USEC);

        return ts;
}

struct timeval *timeval_store(struct timeval *tv, usec_t u) {
        assert(tv);

        if (u == USEC_INFINITY ||
            u / USEC_PER_SEC > TIME_T_MAX) {
                tv->tv_sec = (time_t) -1;
                tv->tv_usec = (suseconds_t) -1;
        } else {
                tv->tv_sec = (time_t) (u / USEC_PER_SEC);
                tv->tv_usec = (suseconds_t) (u % USEC_PER_SEC);
        }

        return tv;
}

static char *format_timestamp_internal(
                char *buf,
                size_t l,
                usec_t t,
                bool utc,
                bool us) {

        /* The weekdays in non-localized (English) form. We use this instead of the localized form, so that our
         * generated timestamps may be parsed with parse_timestamp(), and always read the same. */
        static const char * const weekdays[] = {
                [0] = "Sun",
                [1] = "Mon",
                [2] = "Tue",
                [3] = "Wed",
                [4] = "Thu",
                [5] = "Fri",
                [6] = "Sat",
        };

        struct tm tm;
        time_t sec;
        size_t n;

        assert(buf);

        if (l <
            3 +                  /* week day */
            1 + 10 +             /* space and date */
            1 + 8 +              /* space and time */
            (us ? 1 + 6 : 0) +   /* "." and microsecond part */
            1 + 1 +              /* space and shortest possible zone */
            1)
                return NULL; /* Not enough space even for the shortest form. */
        if (t <= 0 || t == USEC_INFINITY)
                return NULL; /* Timestamp is unset */

        /* Let's not format times with years > 9999 */
        if (t > USEC_TIMESTAMP_FORMATTABLE_MAX) {
                assert(l >= STRLEN("--- XXXX-XX-XX XX:XX:XX") + 1);
                strcpy(buf, "--- XXXX-XX-XX XX:XX:XX");
                return buf;
        }

        sec = (time_t) (t / USEC_PER_SEC); /* Round down */

        if (!localtime_or_gmtime_r(&sec, &tm, utc))
                return NULL;

        /* Start with the week day */
        assert((size_t) tm.tm_wday < ELEMENTSOF(weekdays));
        memcpy(buf, weekdays[tm.tm_wday], 4);

        /* Add the main components */
        if (strftime(buf + 3, l - 3, " %Y-%m-%d %H:%M:%S", &tm) <= 0)
                return NULL; /* Doesn't fit */

        /* Append the microseconds part, if that's requested */
        if (us) {
                n = strlen(buf);
                if (n + 8 > l)
                        return NULL; /* Microseconds part doesn't fit. */

                sprintf(buf + n, ".%06"PRI_USEC, t % USEC_PER_SEC);
        }

        /* Append the timezone */
        n = strlen(buf);
        if (utc) {
                /* If this is UTC then let's explicitly use the "UTC" string here, because gmtime_r() normally uses the
                 * obsolete "GMT" instead. */
                if (n + 5 > l)
                        return NULL; /* "UTC" doesn't fit. */

                strcpy(buf + n, " UTC");

        } else if (!isempty(tm.tm_zone)) {
                size_t tn;

                /* An explicit timezone is specified, let's use it, if it fits */
                tn = strlen(tm.tm_zone);
                if (n + 1 + tn + 1 > l) {
                        /* The full time zone does not fit in. Yuck. */

                        if (n + 1 + _POSIX_TZNAME_MAX + 1 > l)
                                return NULL; /* Not even enough space for the POSIX minimum (of 6)? In that case, complain that it doesn't fit */

                        /* So the time zone doesn't fit in fully, but the caller passed enough space for the POSIX
                         * minimum time zone length. In this case suppress the timezone entirely, in order not to dump
                         * an overly long, hard to read string on the user. This should be safe, because the user will
                         * assume the local timezone anyway if none is shown. And so does parse_timestamp(). */
                } else {
                        buf[n++] = ' ';
                        strcpy(buf + n, tm.tm_zone);
                }
        }

        return buf;
}

char *format_timestamp(char *buf, size_t l, usec_t t) {
        return format_timestamp_internal(buf, l, t, false, false);
}

char *format_timestamp_utc(char *buf, size_t l, usec_t t) {
        return format_timestamp_internal(buf, l, t, true, false);
}

char *format_timestamp_us(char *buf, size_t l, usec_t t) {
        return format_timestamp_internal(buf, l, t, false, true);
}

char *format_timestamp_us_utc(char *buf, size_t l, usec_t t) {
        return format_timestamp_internal(buf, l, t, true, true);
}

char *format_timespan(char *buf, size_t l, usec_t t, usec_t accuracy) {
        static const struct {
                const char *suffix;
                usec_t usec;
        } table[] = {
                { "y",     USEC_PER_YEAR   },
                { "month", USEC_PER_MONTH  },
                { "w",     USEC_PER_WEEK   },
                { "d",     USEC_PER_DAY    },
                { "h",     USEC_PER_HOUR   },
                { "min",   USEC_PER_MINUTE },
                { "s",     USEC_PER_SEC    },
                { "ms",    USEC_PER_MSEC   },
                { "us",    1               },
        };

        size_t i;
        char *p = buf;
        bool something = false;

        assert(buf);
        assert(l > 0);

        if (t == USEC_INFINITY) {
                strncpy(p, "infinity", l-1);
                p[l-1] = 0;
                return p;
        }

        if (t <= 0) {
                strncpy(p, "0", l-1);
                p[l-1] = 0;
                return p;
        }

        /* The result of this function can be parsed with parse_sec */

        for (i = 0; i < ELEMENTSOF(table); i++) {
                int k = 0;
                size_t n;
                bool done = false;
                usec_t a, b;

                if (t <= 0)
                        break;

                if (t < accuracy && something)
                        break;

                if (t < table[i].usec)
                        continue;

                if (l <= 1)
                        break;

                a = t / table[i].usec;
                b = t % table[i].usec;

                /* Let's see if we should shows this in dot notation */
                if (t < USEC_PER_MINUTE && b > 0) {
                        usec_t cc;
                        signed char j;

                        j = 0;
                        for (cc = table[i].usec; cc > 1; cc /= 10)
                                j++;

                        for (cc = accuracy; cc > 1; cc /= 10) {
                                b /= 10;
                                j--;
                        }

                        if (j > 0) {
                                k = snprintf(p, l,
                                             "%s"USEC_FMT".%0*"PRI_USEC"%s",
                                             p > buf ? " " : "",
                                             a,
                                             j,
                                             b,
                                             table[i].suffix);

                                t = 0;
                                done = true;
                        }
                }

                /* No? Then let's show it normally */
                if (!done) {
                        k = snprintf(p, l,
                                     "%s"USEC_FMT"%s",
                                     p > buf ? " " : "",
                                     a,
                                     table[i].suffix);

                        t = b;
                }

                n = MIN((size_t) k, l);

                l -= n;
                p += n;

                something = true;
        }

        *p = 0;

        return buf;
}

static const char* extract_multiplier(const char *p, usec_t *multiplier) {
        static const struct {
                const char *suffix;
                usec_t usec;
        } table[] = {
                { "seconds", USEC_PER_SEC    },
                { "second",  USEC_PER_SEC    },
                { "sec",     USEC_PER_SEC    },
                { "s",       USEC_PER_SEC    },
                { "minutes", USEC_PER_MINUTE },
                { "minute",  USEC_PER_MINUTE },
                { "min",     USEC_PER_MINUTE },
                { "months",  USEC_PER_MONTH  },
                { "month",   USEC_PER_MONTH  },
                { "M",       USEC_PER_MONTH  },
                { "msec",    USEC_PER_MSEC   },
                { "ms",      USEC_PER_MSEC   },
                { "m",       USEC_PER_MINUTE },
                { "hours",   USEC_PER_HOUR   },
                { "hour",    USEC_PER_HOUR   },
                { "hr",      USEC_PER_HOUR   },
                { "h",       USEC_PER_HOUR   },
                { "days",    USEC_PER_DAY    },
                { "day",     USEC_PER_DAY    },
                { "d",       USEC_PER_DAY    },
                { "weeks",   USEC_PER_WEEK   },
                { "week",    USEC_PER_WEEK   },
                { "w",       USEC_PER_WEEK   },
                { "years",   USEC_PER_YEAR   },
                { "year",    USEC_PER_YEAR   },
                { "y",       USEC_PER_YEAR   },
                { "usec",    1ULL            },
                { "us",      1ULL            },
                { "µs",      1ULL            },
        };
        size_t i;

        for (i = 0; i < ELEMENTSOF(table); i++) {
                char *e;

                e = startswith(p, table[i].suffix);
                if (e) {
                        *multiplier = table[i].usec;
                        return e;
                }
        }

        return p;
}

int parse_time(const char *t, usec_t *usec, usec_t default_unit) {
        const char *p, *s;
        usec_t r = 0;
        bool something = false;

        assert(t);
        assert(usec);
        assert(default_unit > 0);

        p = t;

        p += strspn(p, WHITESPACE);
        s = startswith(p, "infinity");
        if (s) {
                s += strspn(s, WHITESPACE);
                if (*s != 0)
                        return -EINVAL;

                *usec = USEC_INFINITY;
                return 0;
        }

        for (;;) {
                usec_t multiplier = default_unit, k;
                long long l;
                char *e;

                p += strspn(p, WHITESPACE);

                if (*p == 0) {
                        if (!something)
                                return -EINVAL;

                        break;
                }

                if (*p == '-') /* Don't allow "-0" */
                        return -ERANGE;

                errno = 0;
                l = strtoll(p, &e, 10);
                if (errno > 0)
                        return -errno;
                if (l < 0)
                        return -ERANGE;

                if (*e == '.') {
                        p = e + 1;
                        p += strspn(p, DIGITS);
                } else if (e == p)
                        return -EINVAL;
                else
                        p = e;

                s = extract_multiplier(p + strspn(p, WHITESPACE), &multiplier);
                if (s == p && *s != '\0')
                        /* Don't allow '12.34.56', but accept '12.34 .56' or '12.34s.56'*/
                        return -EINVAL;

                p = s;

                if ((usec_t) l >= USEC_INFINITY / multiplier)
                        return -ERANGE;

                k = (usec_t) l * multiplier;
                if (k >= USEC_INFINITY - r)
                        return -ERANGE;

                r += k;

                something = true;

                if (*e == '.') {
                        usec_t m = multiplier / 10;
                        const char *b;

                        for (b = e + 1; *b >= '0' && *b <= '9'; b++, m /= 10) {
                                k = (usec_t) (*b - '0') * m;
                                if (k >= USEC_INFINITY - r)
                                        return -ERANGE;

                                r += k;
                        }

                        /* Don't allow "0.-0", "3.+1", "3. 1", "3.sec" or "3.hoge"*/
                        if (b == e + 1)
                                return -EINVAL;
                }
        }

        *usec = r;

        return 0;
}

int parse_sec(const char *t, usec_t *usec) {
        return parse_time(t, usec, USEC_PER_SEC);
}

bool timezone_is_valid(const char *name, int log_level) {
        bool slash = false;
        const char *p, *t;
        _cleanup_close_ int fd = -1;
        char buf[4];
        int r;

        if (isempty(name))
                return false;

        if (name[0] == '/')
                return false;

        for (p = name; *p; p++) {
                if (!(*p >= '0' && *p <= '9') &&
                    !(*p >= 'a' && *p <= 'z') &&
                    !(*p >= 'A' && *p <= 'Z') &&
                    !IN_SET(*p, '-', '_', '+', '/'))
                        return false;

                if (*p == '/') {

                        if (slash)
                                return false;

                        slash = true;
                } else
                        slash = false;
        }

        if (slash)
                return false;

        if (p - name >= PATH_MAX)
                return false;

        t = strjoina("/usr/share/zoneinfo/", name);

        fd = open(t, O_RDONLY|O_CLOEXEC);
        if (fd < 0) {
                log_full_errno(log_level, errno, "Failed to open timezone file '%s': %m", t);
                return false;
        }

        r = fd_verify_regular(fd);
        if (r < 0) {
                log_full_errno(log_level, r, "Timezone file '%s' is not  a regular file: %m", t);
                return false;
        }

        r = loop_read_exact(fd, buf, 4, false);
        if (r < 0) {
                log_full_errno(log_level, r, "Failed to read from timezone file '%s': %m", t);
                return false;
        }

        /* Magic from tzfile(5) */
        if (memcmp(buf, "TZif", 4) != 0) {
                log_full(log_level, "Timezone file '%s' has wrong magic bytes", t);
                return false;
        }

        return true;
}

bool clock_boottime_supported(void) {
        static int supported = -1;

        /* Note that this checks whether CLOCK_BOOTTIME is available in general as well as available for timerfds()! */

        if (supported < 0) {
                int fd;

                fd = timerfd_create(CLOCK_BOOTTIME, TFD_NONBLOCK|TFD_CLOEXEC);
                if (fd < 0)
                        supported = false;
                else {
                        safe_close(fd);
                        supported = true;
                }
        }

        return supported;
}

clockid_t clock_boottime_or_monotonic(void) {
        if (clock_boottime_supported())
                return CLOCK_BOOTTIME;
        else
                return CLOCK_MONOTONIC;
}

bool clock_supported(clockid_t clock) {
        struct timespec ts;

        switch (clock) {

        case CLOCK_MONOTONIC:
        case CLOCK_REALTIME:
                return true;

        case CLOCK_BOOTTIME:
                return clock_boottime_supported();

        case CLOCK_BOOTTIME_ALARM:
                if (!clock_boottime_supported())
                        return false;

                _fallthrough_;
        default:
                /* For everything else, check properly */
                return clock_gettime(clock, &ts) >= 0;
        }
}

struct tm *localtime_or_gmtime_r(const time_t *t, struct tm *tm, bool utc) {
        return utc ? gmtime_r(t, tm) : localtime_r(t, tm);
}
