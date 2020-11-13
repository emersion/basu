/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

#include <alloca.h>
#include <stdbool.h>
#include <stddef.h>

#include "macro.h"
#include "string-util.h"
#include "time-util.h"

bool is_path(const char *p) _pure_;
bool path_is_absolute(const char *p) _pure_;
char* path_startswith(const char *path, const char *prefix) _pure_;
int path_compare(const char *a, const char *b) _pure_;
bool path_equal(const char *a, const char *b) _pure_;
bool path_equal_or_files_same(const char *a, const char *b, int flags);

bool filename_is_valid(const char *p) _pure_;
bool path_is_valid(const char *p) _pure_;
bool path_is_normalized(const char *p) _pure_;

bool hidden_or_backup_file(const char *filename) _pure_;

bool dot_or_dot_dot(const char *path);

bool empty_or_root(const char *root);
