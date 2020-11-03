/* SPDX-License-Identifier: LGPL-2.1+ */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "alloc-util.h"
#include "def.h"
#include "fd-util.h"
#include "fileio.h"
#include "hostname-util.h"
#include "macro.h"
#include "string-util.h"

static bool hostname_valid_char(char c) {
        return
                (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                IN_SET(c, '-', '_', '.');
}

/**
 * Check if s looks like a valid host name or FQDN. This does not do
 * full DNS validation, but only checks if the name is composed of
 * allowed characters and the length is not above the maximum allowed
 * by Linux (c.f. dns_name_is_valid()). Trailing dot is allowed if
 * allow_trailing_dot is true and at least two components are present
 * in the name. Note that due to the restricted charset and length
 * this call is substantially more conservative than
 * dns_name_is_valid().
 */
bool hostname_is_valid(const char *s, bool allow_trailing_dot) {
        unsigned n_dots = 0;
        const char *p;
        bool dot;

        if (isempty(s))
                return false;

        /* Doesn't accept empty hostnames, hostnames with
         * leading dots, and hostnames with multiple dots in a
         * sequence. Also ensures that the length stays below
         * HOST_NAME_MAX. */

        for (p = s, dot = true; *p; p++) {
                if (*p == '.') {
                        if (dot)
                                return false;

                        dot = true;
                        n_dots++;
                } else {
                        if (!hostname_valid_char(*p))
                                return false;

                        dot = false;
                }
        }

        if (dot && (n_dots < 2 || !allow_trailing_dot))
                return false;

        if (p-s > HOST_NAME_MAX) /* Note that HOST_NAME_MAX is 64 on
                                  * Linux, but DNS allows domain names
                                  * up to 255 characters */
                return false;

        return true;
}

