/* SPDX-License-Identifier: LGPL-2.1+ */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "stat-util.h"

int files_same(const char *filea, const char *fileb, int flags) {
        struct stat a, b;

        assert(filea);
        assert(fileb);

        if (fstatat(AT_FDCWD, filea, &a, flags) < 0)
                return -errno;

        if (fstatat(AT_FDCWD, fileb, &b, flags) < 0)
                return -errno;

        return a.st_dev == b.st_dev &&
               a.st_ino == b.st_ino;
}
