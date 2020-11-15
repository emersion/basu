/* SPDX-License-Identifier: LGPL-2.1+ */

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#if HAVE_LINUX_MEMFD_H
#endif

#include "alloc-util.h"
#include "memfd-util.h"

int memfd_set_sealed(int fd) {
        int r;

        assert(fd >= 0);

        r = fcntl(fd, F_ADD_SEALS, F_SEAL_SHRINK | F_SEAL_GROW | F_SEAL_WRITE | F_SEAL_SEAL);
        if (r < 0)
                return -errno;

        return 0;
}

int memfd_get_size(int fd, uint64_t *sz) {
        struct stat stat;
        int r;

        assert(fd >= 0);
        assert(sz);

        r = fstat(fd, &stat);
        if (r < 0)
                return -errno;

        *sz = stat.st_size;
        return 0;
}

int memfd_set_size(int fd, uint64_t sz) {
        int r;

        assert(fd >= 0);

        r = ftruncate(fd, sz);
        if (r < 0)
                return -errno;

        return 0;
}
