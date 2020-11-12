/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include "util.h"

int unlink_noerrno(const char *path);

int rename_noreplace(int olddirfd, const char *oldpath, int newdirfd, const char *newpath);

int readlinkat_malloc(int fd, const char *p, char **ret);
int readlink_malloc(const char *p, char **r);
int readlink_and_make_absolute(const char *p, char **r);

int fchmod_umask(int fd, mode_t mode);

#define laccess(path, mode) faccessat(AT_FDCWD, (path), (mode), AT_SYMLINK_NOFOLLOW)

int touch_file(const char *path, bool parents, usec_t stamp, uid_t uid, gid_t gid, mode_t mode);
int touch(const char *path);

int tmp_dir(const char **ret);
int var_tmp_dir(const char **ret);

enum {
        CHASE_PREFIX_ROOT = 1 << 0, /* If set, the specified path will be prefixed by the specified root before beginning the iteration */
        CHASE_NONEXISTENT = 1 << 1, /* If set, it's OK if the path doesn't actually exist. */
        CHASE_NO_AUTOFS   = 1 << 2, /* If set, return -EREMOTE if autofs mount point found */
        CHASE_SAFE        = 1 << 3, /* If set, return EPERM if we ever traverse from unprivileged to privileged files or directories */
        CHASE_OPEN        = 1 << 4, /* If set, return an O_PATH object to the final component */
        CHASE_TRAIL_SLASH = 1 << 5, /* If set, any trailing slash will be preserved */
        CHASE_STEP        = 1 << 6, /* If set, just execute a single step of the normalization */
        CHASE_NOFOLLOW    = 1 << 7, /* Only valid with CHASE_OPEN: when the path's right-most component refers to symlink return O_PATH fd of the symlink, rather than following it. */
};

/* How many iterations to execute before returning -ELOOP */
#define CHASE_SYMLINKS_MAX 32

int chase_symlinks(const char *path_with_prefix, const char *root, unsigned flags, char **ret);

int fsync_directory_of_file(int fd);
