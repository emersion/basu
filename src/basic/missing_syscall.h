/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

/* Missing glibc definitions to access certain kernel APIs */

#include <sys/syscall.h>
#include <sys/types.h>

/* ======================================================================= */

#if !HAVE_GETRANDOM
static inline int missing_getrandom(void *buffer, size_t count, unsigned flags) {
#  ifdef __NR_getrandom
        return syscall(__NR_getrandom, buffer, count, flags);
#  else
        errno = ENOSYS;
        return -1;
#  endif
}

#  define getrandom missing_getrandom
#endif

/* ======================================================================= */

#if !HAVE_GETTID
static inline pid_t missing_gettid(void) {
        return (pid_t) syscall(__NR_gettid);
}

#  define gettid missing_gettid
#endif
