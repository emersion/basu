/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

bool uid_is_valid(uid_t uid);

static inline bool gid_is_valid(gid_t gid) {
        return uid_is_valid((uid_t) gid);
}

int parse_uid(const char *s, uid_t* ret_uid);

static inline int parse_gid(const char *s, gid_t *ret_gid) {
        return parse_uid(s, (uid_t*) ret_gid);
}

char* uid_to_name(uid_t uid);

int reset_uid_gid(void);

#define UID_INVALID ((uid_t) -1)

#define UID_NOBODY ((uid_t) 65534U)

int maybe_setgroups(size_t size, const gid_t *list);

bool synthesize_nobody(void);

