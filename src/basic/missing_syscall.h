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

/* ======================================================================= */

#if !HAVE_RENAMEAT2
#  ifndef __NR_renameat2
#    if defined __x86_64__
#      define __NR_renameat2 316
#    elif defined __arm__
#      define __NR_renameat2 382
#    elif defined __aarch64__
#      define __NR_renameat2 276
#    elif defined _MIPS_SIM
#      if _MIPS_SIM == _MIPS_SIM_ABI32
#        define __NR_renameat2 4351
#      endif
#      if _MIPS_SIM == _MIPS_SIM_NABI32
#        define __NR_renameat2 6315
#      endif
#      if _MIPS_SIM == _MIPS_SIM_ABI64
#        define __NR_renameat2 5311
#      endif
#    elif defined __i386__
#      define __NR_renameat2 353
#    elif defined __powerpc64__
#      define __NR_renameat2 357
#    elif defined __s390__ || defined __s390x__
#      define __NR_renameat2 347
#    elif defined __arc__
#      define __NR_renameat2 276
#    else
#      warning "__NR_renameat2 unknown for your architecture"
#    endif
#  endif

static inline int missing_renameat2(int oldfd, const char *oldname, int newfd, const char *newname, unsigned flags) {
#  ifdef __NR_renameat2
        return syscall(__NR_renameat2, oldfd, oldname, newfd, newname, flags);
#  else
        errno = ENOSYS;
        return -1;
#  endif
}

#  define renameat2 missing_renameat2
#endif

/* ======================================================================= */

#if !HAVE_KCMP
static inline int missing_kcmp(pid_t pid1, pid_t pid2, int type, unsigned long idx1, unsigned long idx2) {
#  ifdef __NR_kcmp
        return syscall(__NR_kcmp, pid1, pid2, type, idx1, idx2);
#  else
        errno = ENOSYS;
        return -1;
#  endif
}

#  define kcmp missing_kcmp
#endif

/* ======================================================================= */

#if !HAVE_KEYCTL
static inline long missing_keyctl(int cmd, unsigned long arg2, unsigned long arg3, unsigned long arg4,unsigned long arg5) {
#  ifdef __NR_keyctl
        return syscall(__NR_keyctl, cmd, arg2, arg3, arg4, arg5);
#  else
        errno = ENOSYS;
        return -1;
#  endif

#  define keyctl missing_keyctl
}

static inline key_serial_t missing_add_key(const char *type, const char *description, const void *payload, size_t plen, key_serial_t ringid) {
#  ifdef __NR_add_key
        return syscall(__NR_add_key, type, description, payload, plen, ringid);
#  else
        errno = ENOSYS;
        return -1;
#  endif

#  define add_key missing_add_key
}

static inline key_serial_t missing_request_key(const char *type, const char *description, const char * callout_info, key_serial_t destringid) {
#  ifdef __NR_request_key
        return syscall(__NR_request_key, type, description, callout_info, destringid);
#  else
        errno = ENOSYS;
        return -1;
#  endif

#  define request_key missing_request_key
}
#endif

/* ======================================================================= */

#if !HAVE_COPY_FILE_RANGE
#  ifndef __NR_copy_file_range
#    if defined(__x86_64__)
#      define __NR_copy_file_range 326
#    elif defined(__i386__)
#      define __NR_copy_file_range 377
#    elif defined __s390__
#      define __NR_copy_file_range 375
#    elif defined __arm__
#      define __NR_copy_file_range 391
#    elif defined __aarch64__
#      define __NR_copy_file_range 285
#    elif defined __powerpc__
#      define __NR_copy_file_range 379
#    elif defined __arc__
#      define __NR_copy_file_range 285
#    else
#      warning "__NR_copy_file_range not defined for your architecture"
#    endif
#  endif

static inline ssize_t missing_copy_file_range(int fd_in, loff_t *off_in,
                                              int fd_out, loff_t *off_out,
                                              size_t len,
                                              unsigned int flags) {
#  ifdef __NR_copy_file_range
        return syscall(__NR_copy_file_range, fd_in, off_in, fd_out, off_out, len, flags);
#  else
        errno = ENOSYS;
        return -1;
#  endif
}

#  define copy_file_range missing_copy_file_range
#endif

/* ======================================================================= */

#ifndef __IGNORE_pkey_mprotect
#  ifndef __NR_pkey_mprotect
#    if defined __i386__
#      define __NR_pkey_mprotect 380
#    elif defined __x86_64__
#      define __NR_pkey_mprotect 329
#    elif defined __arm__
#      define __NR_pkey_mprotect 394
#    elif defined __aarch64__
#      define __NR_pkey_mprotect 394
#    elif defined __powerpc__
#      define __NR_pkey_mprotect 386
#    elif defined _MIPS_SIM
#      if _MIPS_SIM == _MIPS_SIM_ABI32
#        define __NR_pkey_mprotect 4363
#      endif
#      if _MIPS_SIM == _MIPS_SIM_NABI32
#        define __NR_pkey_mprotect 6327
#      endif
#      if _MIPS_SIM == _MIPS_SIM_ABI64
#        define __NR_pkey_mprotect 5323
#      endif
#    else
#      warning "__NR_pkey_mprotect not defined for your architecture"
#    endif
#  endif
#endif
