/* SPDX-License-Identifier: LGPL-2.1+ */

#include <errno.h>
#include <inttypes.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc-util.h"
#include "locale-util.h"
#include "parse-util.h"
#include "process-util.h"
#include "string-util.h"

int parse_boolean(const char *v) {
        assert(v);

        if (streq(v, "1") || strcaseeq(v, "yes") || strcaseeq(v, "y") || strcaseeq(v, "true") || strcaseeq(v, "t") || strcaseeq(v, "on"))
                return 1;
        else if (streq(v, "0") || strcaseeq(v, "no") || strcaseeq(v, "n") || strcaseeq(v, "false") || strcaseeq(v, "f") || strcaseeq(v, "off"))
                return 0;

        return -EINVAL;
}

int parse_pid(const char *s, pid_t* ret_pid) {
        unsigned long ul = 0;
        pid_t pid;
        int r;

        assert(s);
        assert(ret_pid);

        r = safe_atolu(s, &ul);
        if (r < 0)
                return r;

        pid = (pid_t) ul;

        if ((unsigned long) pid != ul)
                return -ERANGE;

        if (!pid_is_valid(pid))
                return -ERANGE;

        *ret_pid = pid;
        return 0;
}

int parse_size(const char *t, uint64_t base, uint64_t *size) {

        /* Soo, sometimes we want to parse IEC binary suffixes, and
         * sometimes SI decimal suffixes. This function can parse
         * both. Which one is the right way depends on the
         * context. Wikipedia suggests that SI is customary for
         * hardware metrics and network speeds, while IEC is
         * customary for most data sizes used by software and volatile
         * (RAM) memory. Hence be careful which one you pick!
         *
         * In either case we use just K, M, G as suffix, and not Ki,
         * Mi, Gi or so (as IEC would suggest). That's because that's
         * frickin' ugly. But this means you really need to make sure
         * to document which base you are parsing when you use this
         * call. */

        struct table {
                const char *suffix;
                unsigned long long factor;
        };

        static const struct table iec[] = {
                { "E", 1024ULL*1024ULL*1024ULL*1024ULL*1024ULL*1024ULL },
                { "P", 1024ULL*1024ULL*1024ULL*1024ULL*1024ULL },
                { "T", 1024ULL*1024ULL*1024ULL*1024ULL },
                { "G", 1024ULL*1024ULL*1024ULL },
                { "M", 1024ULL*1024ULL },
                { "K", 1024ULL },
                { "B", 1ULL },
                { "",  1ULL },
        };

        static const struct table si[] = {
                { "E", 1000ULL*1000ULL*1000ULL*1000ULL*1000ULL*1000ULL },
                { "P", 1000ULL*1000ULL*1000ULL*1000ULL*1000ULL },
                { "T", 1000ULL*1000ULL*1000ULL*1000ULL },
                { "G", 1000ULL*1000ULL*1000ULL },
                { "M", 1000ULL*1000ULL },
                { "K", 1000ULL },
                { "B", 1ULL },
                { "",  1ULL },
        };

        const struct table *table;
        const char *p;
        unsigned long long r = 0;
        unsigned n_entries, start_pos = 0;

        assert(t);
        assert(IN_SET(base, 1000, 1024));
        assert(size);

        if (base == 1000) {
                table = si;
                n_entries = ELEMENTSOF(si);
        } else {
                table = iec;
                n_entries = ELEMENTSOF(iec);
        }

        p = t;
        do {
                unsigned long long l, tmp;
                double frac = 0;
                char *e;
                unsigned i;

                p += strspn(p, WHITESPACE);

                errno = 0;
                l = strtoull(p, &e, 10);
                if (errno > 0)
                        return -errno;
                if (e == p)
                        return -EINVAL;
                if (*p == '-')
                        return -ERANGE;

                if (*e == '.') {
                        e++;

                        /* strtoull() itself would accept space/+/- */
                        if (*e >= '0' && *e <= '9') {
                                unsigned long long l2;
                                char *e2;

                                l2 = strtoull(e, &e2, 10);
                                if (errno > 0)
                                        return -errno;

                                /* Ignore failure. E.g. 10.M is valid */
                                frac = l2;
                                for (; e < e2; e++)
                                        frac /= 10;
                        }
                }

                e += strspn(e, WHITESPACE);

                for (i = start_pos; i < n_entries; i++)
                        if (startswith(e, table[i].suffix))
                                break;

                if (i >= n_entries)
                        return -EINVAL;

                if (l + (frac > 0) > ULLONG_MAX / table[i].factor)
                        return -ERANGE;

                tmp = l * table[i].factor + (unsigned long long) (frac * table[i].factor);
                if (tmp > ULLONG_MAX - r)
                        return -ERANGE;

                r += tmp;
                if ((unsigned long long) (uint64_t) r != r)
                        return -ERANGE;

                p = e + strlen(table[i].suffix);

                start_pos = i + 1;

        } while (*p);

        *size = r;

        return 0;
}

int safe_atou_full(const char *s, unsigned base, unsigned *ret_u) {
        char *x = NULL;
        unsigned long l;

        assert(s);
        assert(ret_u);
        assert(base <= 16);

        /* strtoul() is happy to parse negative values, and silently
         * converts them to unsigned values without generating an
         * error. We want a clean error, hence let's look for the "-"
         * prefix on our own, and generate an error. But let's do so
         * only after strtoul() validated that the string is clean
         * otherwise, so that we return EINVAL preferably over
         * ERANGE. */

        s += strspn(s, WHITESPACE);

        errno = 0;
        l = strtoul(s, &x, base);
        if (errno > 0)
                return -errno;
        if (!x || x == s || *x != 0)
                return -EINVAL;
        if (s[0] == '-')
                return -ERANGE;
        if ((unsigned long) (unsigned) l != l)
                return -ERANGE;

        *ret_u = (unsigned) l;
        return 0;
}

int safe_atoi(const char *s, int *ret_i) {
        char *x = NULL;
        long l;

        assert(s);
        assert(ret_i);

        errno = 0;
        l = strtol(s, &x, 0);
        if (errno > 0)
                return -errno;
        if (!x || x == s || *x != 0)
                return -EINVAL;
        if ((long) (int) l != l)
                return -ERANGE;

        *ret_i = (int) l;
        return 0;
}

int safe_atollu(const char *s, long long unsigned *ret_llu) {
        char *x = NULL;
        unsigned long long l;

        assert(s);
        assert(ret_llu);

        s += strspn(s, WHITESPACE);

        errno = 0;
        l = strtoull(s, &x, 0);
        if (errno > 0)
                return -errno;
        if (!x || x == s || *x != 0)
                return -EINVAL;
        if (*s == '-')
                return -ERANGE;

        *ret_llu = l;
        return 0;
}

int safe_atolli(const char *s, long long int *ret_lli) {
        char *x = NULL;
        long long l;

        assert(s);
        assert(ret_lli);

        errno = 0;
        l = strtoll(s, &x, 0);
        if (errno > 0)
                return -errno;
        if (!x || x == s || *x != 0)
                return -EINVAL;

        *ret_lli = l;
        return 0;
}

int safe_atou8(const char *s, uint8_t *ret) {
        char *x = NULL;
        unsigned long l;

        assert(s);
        assert(ret);

        s += strspn(s, WHITESPACE);

        errno = 0;
        l = strtoul(s, &x, 0);
        if (errno > 0)
                return -errno;
        if (!x || x == s || *x != 0)
                return -EINVAL;
        if (s[0] == '-')
                return -ERANGE;
        if ((unsigned long) (uint8_t) l != l)
                return -ERANGE;

        *ret = (uint8_t) l;
        return 0;
}

int safe_atou16_full(const char *s, unsigned base, uint16_t *ret) {
        char *x = NULL;
        unsigned long l;

        assert(s);
        assert(ret);
        assert(base <= 16);

        s += strspn(s, WHITESPACE);

        errno = 0;
        l = strtoul(s, &x, base);
        if (errno > 0)
                return -errno;
        if (!x || x == s || *x != 0)
                return -EINVAL;
        if (s[0] == '-')
                return -ERANGE;
        if ((unsigned long) (uint16_t) l != l)
                return -ERANGE;

        *ret = (uint16_t) l;
        return 0;
}

int safe_atoi16(const char *s, int16_t *ret) {
        char *x = NULL;
        long l;

        assert(s);
        assert(ret);

        errno = 0;
        l = strtol(s, &x, 0);
        if (errno > 0)
                return -errno;
        if (!x || x == s || *x != 0)
                return -EINVAL;
        if ((long) (int16_t) l != l)
                return -ERANGE;

        *ret = (int16_t) l;
        return 0;
}

int safe_atod(const char *s, double *ret_d) {
        _cleanup_(freelocalep) locale_t loc = (locale_t) 0;
        char *x = NULL;
        double d = 0;

        assert(s);
        assert(ret_d);

        loc = newlocale(LC_NUMERIC_MASK, "C", (locale_t) 0);
        if (loc == (locale_t) 0)
                return -errno;

        errno = 0;
        d = strtod_l(s, &x, loc);
        if (errno > 0)
                return -errno;
        if (!x || x == s || *x != 0)
                return -EINVAL;

        *ret_d = (double) d;
        return 0;
}
