/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

#include <alloca.h>
#include <stdbool.h>
#include <stddef.h>

#include "macro.h"
#include "string-util.h"
#include "time-util.h"

#define PATH_SPLIT_SBIN_BIN(x) x "sbin:" x "bin"
#define PATH_NORMAL_SBIN_BIN(x) x "bin"

#if HAVE_SPLIT_BIN
#  define PATH_SBIN_BIN(x) PATH_SPLIT_SBIN_BIN(x)
#else
#  define PATH_SBIN_BIN(x) PATH_NORMAL_SBIN_BIN(x)
#endif

#define DEFAULT_PATH_NORMAL PATH_SBIN_BIN("/usr/local/") ":" PATH_SBIN_BIN("/usr/")
#define DEFAULT_PATH_SPLIT_USR DEFAULT_PATH_NORMAL ":" PATH_SBIN_BIN("/")

#if HAVE_SPLIT_USR
#  define DEFAULT_PATH DEFAULT_PATH_SPLIT_USR
#else
#  define DEFAULT_PATH DEFAULT_PATH_NORMAL
#endif

bool is_path(const char *p) _pure_;
bool path_is_absolute(const char *p) _pure_;
int safe_getcwd(char **ret);
int path_make_absolute_cwd(const char *p, char **ret);
char* path_startswith(const char *path, const char *prefix) _pure_;
int path_compare(const char *a, const char *b) _pure_;
bool path_equal(const char *a, const char *b) _pure_;
bool path_equal_or_files_same(const char *a, const char *b, int flags);
char* path_join(const char *root, const char *path, const char *rest);
char* path_simplify(char *path, bool kill_dots);

/* Note: the search terminates on the first NULL item. */
#define PATH_IN_SET(p, ...)                                     \
        ({                                                      \
                char **_s;                                      \
                bool _found = false;                            \
                STRV_FOREACH(_s, STRV_MAKE(__VA_ARGS__))        \
                        if (path_equal(p, *_s)) {               \
                               _found = true;                   \
                               break;                           \
                        }                                       \
                _found;                                         \
        })

int path_strv_make_absolute_cwd(char **l);
char** path_strv_resolve(char **l, const char *root);
char** path_strv_resolve_uniq(char **l, const char *root);

int find_binary(const char *name, char **filename);

/* Iterates through the path prefixes of the specified path, going up
 * the tree, to root. Also returns "" (and not "/"!) for the root
 * directory. Excludes the specified directory itself */
#define PATH_FOREACH_PREFIX(prefix, path) \
        for (char *_slash = ({ path_simplify(strcpy(prefix, path), false); streq(prefix, "/") ? NULL : strrchr(prefix, '/'); }); _slash && ((*_slash = 0), true); _slash = strrchr((prefix), '/'))

char *prefix_root(const char *root, const char *path);

/* Similar to prefix_root(), but returns an alloca() buffer, or
 * possibly a const pointer into the path parameter */
#define prefix_roota(root, path)                                        \
        ({                                                              \
                const char* _path = (path), *_root = (root), *_ret;     \
                char *_p, *_n;                                          \
                size_t _l;                                              \
                while (_path[0] == '/' && _path[1] == '/')              \
                        _path ++;                                       \
                if (empty_or_root(_root))                               \
                        _ret = _path;                                   \
                else {                                                  \
                        _l = strlen(_root) + 1 + strlen(_path) + 1;     \
                        _n = alloca(_l);                                \
                        _p = stpcpy(_n, _root);                         \
                        while (_p > _n && _p[-1] == '/')                \
                                _p--;                                   \
                        if (_path[0] != '/')                            \
                                *(_p++) = '/';                          \
                        strcpy(_p, _path);                              \
                        _ret = _n;                                      \
                }                                                       \
                _ret;                                                   \
        })

char* dirname_malloc(const char *path);

bool filename_is_valid(const char *p) _pure_;
bool path_is_valid(const char *p) _pure_;
bool path_is_normalized(const char *p) _pure_;

char *file_in_same_dir(const char *path, const char *filename);

bool hidden_or_backup_file(const char *filename) _pure_;

bool dot_or_dot_dot(const char *path);

bool empty_or_root(const char *root);
static inline const char *empty_to_root(const char *path) {
        return isempty(path) ? "/" : path;
}

