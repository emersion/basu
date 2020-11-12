/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

#include <netinet/ether.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <linux/netlink.h>
#include <linux/if_infiniband.h>
#include <linux/if_packet.h>

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
        struct sockaddr_nl nl;
        struct sockaddr_ll ll;
        struct sockaddr_vm vm;

        /* Ensure there is enough space to store Infiniband addresses */
        uint8_t ll_buffer[offsetof(struct sockaddr_ll, sll_addr) + CONST_MAX(ETH_ALEN, INFINIBAND_ALEN)];

        /* Ensure there is enough space after the AF_UNIX sun_path for one more NUL byte, just to be sure that the path
         * component is always followed by at least one NUL byte. */
        uint8_t un_buffer[sizeof(struct sockaddr_un) + 1];
};

int fd_inc_sndbuf(int fd, size_t n);
int fd_inc_rcvbuf(int fd, size_t n);

int getpeercred(int fd, struct ucred *ucred);
int getpeersec(int fd, char **ret);
int getpeergroups(int fd, gid_t **ret);

#define CMSG_FOREACH(cmsg, mh)                                          \
        for ((cmsg) = CMSG_FIRSTHDR(mh); (cmsg); (cmsg) = CMSG_NXTHDR((mh), (cmsg)))

/*
 * Certain hardware address types (e.g Infiniband) do not fit into sll_addr
 * (8 bytes) and run over the structure. This macro returns the correct size that
 * must be passed to kernel.
 */
#define SOCKADDR_LL_LEN(sa)                                             \
        ({                                                              \
                const struct sockaddr_ll *_sa = &(sa);                  \
                size_t _mac_len = sizeof(_sa->sll_addr);                \
                assert(_sa->sll_family == AF_PACKET);                   \
                if (be16toh(_sa->sll_hatype) == ARPHRD_ETHER)           \
                        _mac_len = MAX(_mac_len, (size_t) ETH_ALEN);    \
                if (be16toh(_sa->sll_hatype) == ARPHRD_INFINIBAND)      \
                        _mac_len = MAX(_mac_len, (size_t) INFINIBAND_ALEN); \
                offsetof(struct sockaddr_ll, sll_addr) + _mac_len;      \
        })

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
