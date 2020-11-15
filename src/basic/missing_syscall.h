/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

/* Missing glibc definitions to access certain kernel APIs */

#include <sys/types.h>

/* ======================================================================= */

#if !HAVE_MEMFD_CREATE
#  ifndef __NR_memfd_create
#    if defined __x86_64__
#      define __NR_memfd_create 319
#    elif defined __arm__
#      define __NR_memfd_create 385
#    elif defined __aarch64__
#      define __NR_memfd_create 279
#    elif defined __s390__
#      define __NR_memfd_create 350
#    elif defined _MIPS_SIM
#      if _MIPS_SIM == _MIPS_SIM_ABI32
#        define __NR_memfd_create 4354
#      endif
#      if _MIPS_SIM == _MIPS_SIM_NABI32
#        define __NR_memfd_create 6318
#      endif
#      if _MIPS_SIM == _MIPS_SIM_ABI64
#        define __NR_memfd_create 5314
#      endif
#    elif defined __i386__
#      define __NR_memfd_create 356
#    elif defined __arc__
#      define __NR_memfd_create 279
#    else
#      warning "__NR_memfd_create unknown for your architecture"
#    endif
#  endif

static inline int missing_memfd_create(const char *name, unsigned int flags) {
#  ifdef __NR_memfd_create
        return syscall(__NR_memfd_create, name, flags);
#  else
        errno = ENOSYS;
        return -1;
#  endif
}

#  define memfd_create missing_memfd_create
#endif

/* ======================================================================= */

#if !HAVE_GETRANDOM
#  ifndef __NR_getrandom
#    if defined __x86_64__
#      define __NR_getrandom 318
#    elif defined(__i386__)
#      define __NR_getrandom 355
#    elif defined(__arm__)
#      define __NR_getrandom 384
#   elif defined(__aarch64__)
#      define __NR_getrandom 278
#    elif defined(__ia64__)
#      define __NR_getrandom 1339
#    elif defined(__m68k__)
#      define __NR_getrandom 352
#    elif defined(__s390x__)
#      define __NR_getrandom 349
#    elif defined(__powerpc__)
#      define __NR_getrandom 359
#    elif defined _MIPS_SIM
#      if _MIPS_SIM == _MIPS_SIM_ABI32
#        define __NR_getrandom 4353
#      endif
#      if _MIPS_SIM == _MIPS_SIM_NABI32
#        define __NR_getrandom 6317
#      endif
#      if _MIPS_SIM == _MIPS_SIM_ABI64
#        define __NR_getrandom 5313
#      endif
#    elif defined(__arc__)
#      define __NR_getrandom 278
#    else
#      warning "__NR_getrandom unknown for your architecture"
#    endif
#  endif

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

/* ======================================================================= */

#if !HAVE_SETNS
#  ifndef __NR_setns
#    if defined(__x86_64__)
#      define __NR_setns 308
#    elif defined(__i386__)
#      define __NR_setns 346
#    elif defined(__arc__)
#      define __NR_setns 268
#    else
#      error "__NR_setns is not defined"
#    endif
#  endif

static inline int missing_setns(int fd, int nstype) {
#  ifdef __NR_setns
        return syscall(__NR_setns, fd, nstype);
#  else
        errno = ENOSYS;
        return -1;
#  endif
}

#  define setns missing_setns
#endif
