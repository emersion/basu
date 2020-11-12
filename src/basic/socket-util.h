/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include "macro.h"
#include "missing.h"
#include "util.h"

union sockaddr_union {
        /* The minimal, abstract version */
        struct sockaddr sa;

        /* The libc provided version that allocates "enough room" for every protocol */
        struct sockaddr_storage storage;

        /* Protoctol-specific implementations */
        struct sockaddr_in in;
        struct sockaddr_in6 in6;
        struct sockaddr_un un;
};

int fd_inc_sndbuf(int fd, size_t n);
int fd_inc_rcvbuf(int fd, size_t n);

int getpeercred(int fd, struct ucred *ucred);
int getpeersec(int fd, char **ret);
int getpeergroups(int fd, gid_t **ret);

#define CMSG_FOREACH(cmsg, mh)                                          \
        for ((cmsg) = CMSG_FIRSTHDR(mh); (cmsg); (cmsg) = CMSG_NXTHDR((mh), (cmsg)))

/* Covers only file system and abstract AF_UNIX socket addresses, but not unnamed socket addresses. */
#define SOCKADDR_UN_LEN(sa)                                             \
        ({                                                              \
                const struct sockaddr_un *_sa = &(sa);                  \
                assert(_sa->sun_family == AF_UNIX);                     \
                offsetof(struct sockaddr_un, sun_path) +                \
                        (_sa->sun_path[0] == 0 ?                        \
                         1 + strnlen(_sa->sun_path+1, sizeof(_sa->sun_path)-1) : \
                         strnlen(_sa->sun_path, sizeof(_sa->sun_path))+1); \
        })

static inline int setsockopt_int(int fd, int level, int optname, int value) {
        if (setsockopt(fd, level, optname, &value, sizeof(value)) < 0)
                return -errno;

        return 0;
}
