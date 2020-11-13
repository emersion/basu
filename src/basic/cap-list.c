/* SPDX-License-Identifier: LGPL-2.1+ */

#include <errno.h>
#include <string.h>

#include "alloc-util.h"
#include "capability-util.h"
#include "cap-list.h"
#include "macro.h"
#include "missing.h"
#include "parse-util.h"
#include "util.h"

#include "cap-to-name.h"

const char *capability_to_name(int id) {

        if (id < 0)
                return NULL;

        if (id >= (int) ELEMENTSOF(capability_names))
                return NULL;

        return capability_names[id];
}
