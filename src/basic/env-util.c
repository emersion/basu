/* SPDX-License-Identifier: LGPL-2.1+ */

#include <errno.h>
#include <stdlib.h>

#include "env-util.h"
#include "parse-util.h"

int getenv_bool(const char *p) {
        const char *e;

        e = getenv(p);
        if (!e)
                return -ENXIO;

        return parse_boolean(e);
}
