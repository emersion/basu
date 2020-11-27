/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>

#include "macro.h"

int close_nointr(int fd);
int safe_close(int fd);
void safe_close_pair(int p[]);

static inline int safe_close_above_stdio(int fd) {
        if (fd < 3) /* Don't close stdin/stdout/stderr, but still invalidate the fd by returning -1 */
                return -1;

        return safe_close(fd);
}

void close_many(const int fds[], size_t n_fd);

int fclose_nointr(FILE *f);
FILE* safe_fclose(FILE *f);

static inline void closep(int *fd) {
        safe_close(*fd);
}

static inline void close_pairp(int (*p)[2]) {
        safe_close_pair(*p);
}

static inline void fclosep(FILE **f) {
        safe_fclose(*f);
}

DEFINE_TRIVIAL_CLEANUP_FUNC(DIR*, closedir);

#define _cleanup_close_ _cleanup_(closep)
#define _cleanup_fclose_ _cleanup_(fclosep)
#define _cleanup_closedir_ _cleanup_(closedirp)
#define _cleanup_close_pair_ _cleanup_(close_pairp)

int fd_nonblock(int fd, bool nonblock);
int fd_cloexec(int fd, bool cloexec);

int fd_move_above_stdio(int fd);

int rearrange_stdio(int original_input_fd, int original_output_fd, int original_error_fd);
