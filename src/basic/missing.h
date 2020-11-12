/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

/* Missing glibc definitions to access certain kernel APIs */

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/capability.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <uchar.h>
#include <unistd.h>

#if HAVE_AUDIT
#include <libaudit.h>
#endif

#ifdef ARCH_MIPS
#include <asm/sgidefs.h>
#endif

#if HAVE_LINUX_VM_SOCKETS_H
#include <linux/vm_sockets.h>
#else
#define VMADDR_CID_ANY -1U
struct sockaddr_vm {
        unsigned short svm_family;
        unsigned short svm_reserved1;
        unsigned int svm_port;
        unsigned int svm_cid;
        unsigned char svm_zero[sizeof(struct sockaddr) -
                               sizeof(unsigned short) -
                               sizeof(unsigned short) -
                               sizeof(unsigned int) -
                               sizeof(unsigned int)];
};
#endif /* !HAVE_LINUX_VM_SOCKETS_H */

#ifndef RLIMIT_RTTIME
#define RLIMIT_RTTIME 15
#endif

/* If RLIMIT_RTTIME is not defined, then we cannot use RLIMIT_NLIMITS as is */
#define _RLIMIT_MAX (RLIMIT_RTTIME+1 > RLIMIT_NLIMITS ? RLIMIT_RTTIME+1 : RLIMIT_NLIMITS)

#ifndef F_LINUX_SPECIFIC_BASE
#define F_LINUX_SPECIFIC_BASE 1024
#endif

#ifndef F_ADD_SEALS
#define F_ADD_SEALS (F_LINUX_SPECIFIC_BASE + 9)
#define F_GET_SEALS (F_LINUX_SPECIFIC_BASE + 10)

#define F_SEAL_SEAL     0x0001  /* prevent further seals from being set */
#define F_SEAL_SHRINK   0x0002  /* prevent file from shrinking */
#define F_SEAL_GROW     0x0004  /* prevent file from growing */
#define F_SEAL_WRITE    0x0008  /* prevent writes */
#endif

#ifndef MFD_ALLOW_SEALING
#define MFD_ALLOW_SEALING 0x0002U
#endif

#ifndef MFD_CLOEXEC
#define MFD_CLOEXEC 0x0001U
#endif

#ifndef IP_FREEBIND
#define IP_FREEBIND 15
#endif

#ifndef OOM_SCORE_ADJ_MIN
#define OOM_SCORE_ADJ_MIN (-1000)
#endif

#ifndef OOM_SCORE_ADJ_MAX
#define OOM_SCORE_ADJ_MAX 1000
#endif

#ifndef TIOCVHANGUP
#define TIOCVHANGUP 0x5437
#endif

#ifndef IP_TRANSPARENT
#define IP_TRANSPARENT 19
#endif

#ifndef CLONE_NEWCGROUP
#define CLONE_NEWCGROUP 0x02000000
#endif

#ifndef TMPFS_MAGIC
#define TMPFS_MAGIC 0x01021994
#endif

#ifndef OCFS2_SUPER_MAGIC
#define OCFS2_SUPER_MAGIC 0x7461636f
#endif

#ifndef MS_REC
#define MS_REC 16384
#endif

#ifndef MS_REC
#define MS_REC          (1<<19)
#endif

#ifndef PR_SET_MM_ARG_START
#define PR_SET_MM_ARG_START 8
#endif

#ifndef PR_SET_MM_ARG_END
#define PR_SET_MM_ARG_END 9
#endif

#ifndef CIFS_MAGIC_NUMBER
#  define CIFS_MAGIC_NUMBER 0xFF534D42
#endif

#ifndef TFD_TIMER_CANCEL_ON_SET
#  define TFD_TIMER_CANCEL_ON_SET (1 << 1)
#endif

#ifndef SO_REUSEPORT
#  define SO_REUSEPORT 15
#endif

#ifndef SO_PEERGROUPS
#  define SO_PEERGROUPS 59
#endif

#ifndef IPV6_MIN_MTU
#define IPV6_MIN_MTU 1280
#endif

#ifndef IPV4_MIN_MTU
#define IPV4_MIN_MTU 68
#endif

#ifndef RENAME_NOREPLACE
#define RENAME_NOREPLACE (1 << 0)
#endif

#ifndef KCMP_FILE
#define KCMP_FILE 0
#endif

typedef int32_t key_serial_t;

#ifndef KEYCTL_DESCRIBE
#define KEYCTL_DESCRIBE 6
#endif

#ifndef KEYCTL_READ
#define KEYCTL_READ 11
#endif

#ifndef KEY_POS_VIEW
#define KEY_POS_VIEW    0x01000000
#define KEY_POS_READ    0x02000000
#define KEY_POS_WRITE   0x04000000
#define KEY_POS_SEARCH  0x08000000
#define KEY_POS_LINK    0x10000000
#define KEY_POS_SETATTR 0x20000000

#define KEY_USR_VIEW    0x00010000
#define KEY_USR_READ    0x00020000
#define KEY_USR_WRITE   0x00040000
#define KEY_USR_SEARCH  0x00080000
#define KEY_USR_LINK    0x00100000
#define KEY_USR_SETATTR 0x00200000

#define KEY_GRP_VIEW    0x00000100
#define KEY_GRP_READ    0x00000200
#define KEY_GRP_WRITE   0x00000400
#define KEY_GRP_SEARCH  0x00000800
#define KEY_GRP_LINK    0x00001000
#define KEY_GRP_SETATTR 0x00002000

#define KEY_OTH_VIEW    0x00000001
#define KEY_OTH_READ    0x00000002
#define KEY_OTH_WRITE   0x00000004
#define KEY_OTH_SEARCH  0x00000008
#define KEY_OTH_LINK    0x00000010
#define KEY_OTH_SETATTR 0x00000020
#endif

#ifndef AF_VSOCK
#define AF_VSOCK 40
#endif

#ifndef FALLOC_FL_KEEP_SIZE
#define FALLOC_FL_KEEP_SIZE 0x01
#endif

#ifndef FALLOC_FL_PUNCH_HOLE
#define FALLOC_FL_PUNCH_HOLE 0x02
#endif

#ifndef PF_KTHREAD
#define PF_KTHREAD 0x00200000
#endif

/* The maximum thread/process name length including trailing NUL byte. This mimics the kernel definition of the same
 * name, which we need in userspace at various places but is not defined in userspace currently, neither under this
 * name nor any other. */
#ifndef TASK_COMM_LEN
#define TASK_COMM_LEN 16
#endif

#include "missing_syscall.h"
