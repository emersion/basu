/* SPDX-License-Identifier: LGPL-2.1+ */

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>

#include "alloc-util.h"
#include "copy.h"
#include "macro.h"
#include "missing.h"

#define COPY_BUFFER_SIZE (16U*1024U)

static ssize_t try_copy_file_range(
                int fd_in, loff_t *off_in,
                int fd_out, loff_t *off_out,
                size_t len,
                unsigned flags) {

        static int have = -1;
        ssize_t r;

        if (have == 0)
                return -ENOSYS;

        r = copy_file_range(fd_in, off_in, fd_out, off_out, len, flags);
        if (have < 0)
                have = r >= 0 || errno != ENOSYS;
        if (r < 0)
                return -errno;

        return r;
}

enum {
        FD_IS_NO_PIPE,
        FD_IS_BLOCKING_PIPE,
        FD_IS_NONBLOCKING_PIPE,
};

static int fd_is_nonblock_pipe(int fd) {
        struct stat st;
        int flags;

        /* Checks whether the specified file descriptor refers to a pipe, and if so if O_NONBLOCK is set. */

        if (fstat(fd, &st) < 0)
                return -errno;

        if (!S_ISFIFO(st.st_mode))
                return FD_IS_NO_PIPE;

        flags = fcntl(fd, F_GETFL);
        if (flags < 0)
                return -errno;

        return FLAGS_SET(flags, O_NONBLOCK) ? FD_IS_NONBLOCKING_PIPE : FD_IS_BLOCKING_PIPE;
}

int copy_bytes_full(
                int fdf, int fdt,
                uint64_t max_bytes,
                CopyFlags copy_flags,
                void **ret_remains,
                size_t *ret_remains_size) {

        bool try_cfr = true, try_sendfile = true, try_splice = true;
        int r, nonblock_pipe = -1;
        size_t m = SSIZE_MAX; /* that is the maximum that sendfile and c_f_r accept */

        assert(fdf >= 0);
        assert(fdt >= 0);

        /* Tries to copy bytes from the file descriptor 'fdf' to 'fdt' in the smartest possible way. Copies a maximum
         * of 'max_bytes', which may be specified as UINT64_MAX, in which no maximum is applied. Returns negative on
         * error, zero if EOF is hit before the bytes limit is hit and positive otherwise. If the copy fails for some
         * reason but we read but didn't yet write some data an ret_remains/ret_remains_size is not NULL, then it will
         * be initialized with an allocated buffer containing this "remaining" data. Note that these two parameters are
         * initialized with a valid buffer only on failure and only if there's actually data already read. Otherwise
         * these parameters if non-NULL are set to NULL. */

        if (ret_remains)
                *ret_remains = NULL;
        if (ret_remains_size)
                *ret_remains_size = 0;

        for (;;) {
                ssize_t n;

                if (max_bytes <= 0)
                        return 1; /* return > 0 if we hit the max_bytes limit */

                if (max_bytes != UINT64_MAX && m > max_bytes)
                        m = max_bytes;

                /* First try copy_file_range(), unless we already tried */
                if (try_cfr) {
                        n = try_copy_file_range(fdf, NULL, fdt, NULL, m, 0u);
                        if (n < 0) {
                                if (!IN_SET(n, -EINVAL, -ENOSYS, -EXDEV, -EBADF))
                                        return n;

                                try_cfr = false;
                                /* use fallback below */
                        } else if (n == 0) /* EOF */
                                break;
                        else
                                /* Success! */
                                goto next;
                }

                /* First try sendfile(), unless we already tried */
                if (try_sendfile) {
                        n = sendfile(fdt, fdf, NULL, m);
                        if (n < 0) {
                                if (!IN_SET(errno, EINVAL, ENOSYS))
                                        return -errno;

                                try_sendfile = false;
                                /* use fallback below */
                        } else if (n == 0) /* EOF */
                                break;
                        else
                                /* Success! */
                                goto next;
                }

                /* Then try splice, unless we already tried. */
                if (try_splice) {

                        /* splice()'s asynchronous I/O support is a bit weird. When it encounters a pipe file
                         * descriptor, then it will ignore its O_NONBLOCK flag and instead only honour the
                         * SPLICE_F_NONBLOCK flag specified in its flag parameter. Let's hide this behaviour here, and
                         * check if either of the specified fds are a pipe, and if so, let's pass the flag
                         * automatically, depending on O_NONBLOCK being set.
                         *
                         * Here's a twist though: when we use it to move data between two pipes of which one has
                         * O_NONBLOCK set and the other has not, then we have no individual control over O_NONBLOCK
                         * behaviour. Hence in that case we can't use splice() and still guarantee systematic
                         * O_NONBLOCK behaviour, hence don't. */

                        if (nonblock_pipe < 0) {
                                int a, b;

                                /* Check if either of these fds is a pipe, and if so non-blocking or not */
                                a = fd_is_nonblock_pipe(fdf);
                                if (a < 0)
                                        return a;

                                b = fd_is_nonblock_pipe(fdt);
                                if (b < 0)
                                        return b;

                                if ((a == FD_IS_NO_PIPE && b == FD_IS_NO_PIPE) ||
                                    (a == FD_IS_BLOCKING_PIPE && b == FD_IS_NONBLOCKING_PIPE) ||
                                    (a == FD_IS_NONBLOCKING_PIPE && b == FD_IS_BLOCKING_PIPE))

                                        /* splice() only works if one of the fds is a pipe. If neither is, let's skip
                                         * this step right-away. As mentioned above, if one of the two fds refers to a
                                         * blocking pipe and the other to a non-blocking pipe, we can't use splice()
                                         * either, hence don't try either. This hence means we can only use splice() if
                                         * either only one of the two fds is a pipe, or if both are pipes with the same
                                         * nonblocking flag setting. */

                                        try_splice = false;
                                else
                                        nonblock_pipe = a == FD_IS_NONBLOCKING_PIPE || b == FD_IS_NONBLOCKING_PIPE;
                        }
                }

                if (try_splice) {
                        n = splice(fdf, NULL, fdt, NULL, m, nonblock_pipe ? SPLICE_F_NONBLOCK : 0);
                        if (n < 0) {
                                if (!IN_SET(errno, EINVAL, ENOSYS))
                                        return -errno;

                                try_splice = false;
                                /* use fallback below */
                        } else if (n == 0) /* EOF */
                                break;
                        else
                                /* Success! */
                                goto next;
                }

                /* As a fallback just copy bits by hand */
                {
                        uint8_t buf[MIN(m, COPY_BUFFER_SIZE)], *p = buf;
                        ssize_t z;

                        n = read(fdf, buf, sizeof buf);
                        if (n < 0)
                                return -errno;
                        if (n == 0) /* EOF */
                                break;

                        z = (size_t) n;
                        do {
                                ssize_t k;

                                k = write(fdt, p, z);
                                if (k < 0) {
                                        r = -errno;

                                        if (ret_remains) {
                                                void *copy;

                                                copy = memdup(p, z);
                                                if (!copy)
                                                        return -ENOMEM;

                                                *ret_remains = copy;
                                        }

                                        if (ret_remains_size)
                                                *ret_remains_size = z;

                                        return r;
                                }

                                assert(k <= z);
                                z -= k;
                                p += k;
                        } while (z > 0);
                }

        next:
                if (max_bytes != (uint64_t) -1) {
                        assert(max_bytes >= (uint64_t) n);
                        max_bytes -= n;
                }
                /* sendfile accepts at most SSIZE_MAX-offset bytes to copy,
                 * so reduce our maximum by the amount we already copied,
                 * but don't go below our copy buffer size, unless we are
                 * close the limit of bytes we are allowed to copy. */
                m = MAX(MIN(COPY_BUFFER_SIZE, max_bytes), m - n);
        }

        return 0; /* return 0 if we hit EOF earlier than the size limit */
}

