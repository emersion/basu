/* SPDX-License-Identifier: LGPL-2.1+ */

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "alloc-util.h"
#include "dirent-util.h"
#include "fd-util.h"
#include "fileio.h"
#include "fs-util.h"
#include "log.h"
#include "macro.h"
#include "missing.h"
#include "parse-util.h"
#include "path-util.h"
#include "process-util.h"
#include "stat-util.h"
#include "stdio-util.h"
#include "string-util.h"
#include "strv.h"
#include "time-util.h"
#include "user-util.h"
#include "util.h"

int unlink_noerrno(const char *path) {
        PROTECT_ERRNO;
        int r;

        r = unlink(path);
        if (r < 0)
                return -errno;

        return 0;
}

int rename_noreplace(int olddirfd, const char *oldpath, int newdirfd, const char *newpath) {
        int r;

        /* Try the ideal approach first */
        if (renameat2(olddirfd, oldpath, newdirfd, newpath, RENAME_NOREPLACE) >= 0)
                return 0;

        /* renameat2() exists since Linux 3.15, btrfs and FAT added support for it later. If it is not implemented,
         * fall back to a different method. */
        if (!IN_SET(errno, EINVAL, ENOSYS, ENOTTY))
                return -errno;

        /* Let's try to use linkat()+unlinkat() as fallback. This doesn't work on directories and on some file systems
         * that do not support hard links (such as FAT, most prominently), but for files it's pretty close to what we
         * want — though not atomic (i.e. for a short period both the new and the old filename will exist). */
        if (linkat(olddirfd, oldpath, newdirfd, newpath, 0) >= 0) {

                if (unlinkat(olddirfd, oldpath, 0) < 0) {
                        r = -errno; /* Backup errno before the following unlinkat() alters it */
                        (void) unlinkat(newdirfd, newpath, 0);
                        return r;
                }

                return 0;
        }

        if (!IN_SET(errno, EINVAL, ENOSYS, ENOTTY, EPERM)) /* FAT returns EPERM on link()… */
                return -errno;

        /* OK, neither RENAME_NOREPLACE nor linkat()+unlinkat() worked. Let's then fallback to the racy TOCTOU
         * vulnerable accessat(F_OK) check followed by classic, replacing renameat(), we have nothing better. */

        if (faccessat(newdirfd, newpath, F_OK, AT_SYMLINK_NOFOLLOW) >= 0)
                return -EEXIST;
        if (errno != ENOENT)
                return -errno;

        if (renameat(olddirfd, oldpath, newdirfd, newpath) < 0)
                return -errno;

        return 0;
}

int readlinkat_malloc(int fd, const char *p, char **ret) {
        size_t l = FILENAME_MAX+1;
        int r;

        assert(p);
        assert(ret);

        for (;;) {
                char *c;
                ssize_t n;

                c = new(char, l);
                if (!c)
                        return -ENOMEM;

                n = readlinkat(fd, p, c, l-1);
                if (n < 0) {
                        r = -errno;
                        free(c);
                        return r;
                }

                if ((size_t) n < l-1) {
                        c[n] = 0;
                        *ret = c;
                        return 0;
                }

                free(c);
                l *= 2;
        }
}

int readlink_malloc(const char *p, char **ret) {
        return readlinkat_malloc(AT_FDCWD, p, ret);
}

int readlink_and_make_absolute(const char *p, char **r) {
        _cleanup_free_ char *target = NULL;
        char *k;
        int j;

        assert(p);
        assert(r);

        j = readlink_malloc(p, &target);
        if (j < 0)
                return j;

        k = file_in_same_dir(p, target);
        if (!k)
                return -ENOMEM;

        *r = k;
        return 0;
}

int fchmod_umask(int fd, mode_t m) {
        mode_t u;
        int r;

        u = umask(0777);
        r = fchmod(fd, m & (~u)) < 0 ? -errno : 0;
        umask(u);

        return r;
}

int touch_file(const char *path, bool parents, usec_t stamp, uid_t uid, gid_t gid, mode_t mode) {
        char fdpath[STRLEN("/proc/self/fd/") + DECIMAL_STR_MAX(int)];
        _cleanup_close_ int fd = -1;
        int r, ret = 0;

        assert(path);

        /* Note that touch_file() does not follow symlinks: if invoked on an existing symlink, then it is the symlink
         * itself which is updated, not its target
         *
         * Returns the first error we encounter, but tries to apply as much as possible. */

        /* Initially, we try to open the node with O_PATH, so that we get a reference to the node. This is useful in
         * case the path refers to an existing device or socket node, as we can open it successfully in all cases, and
         * won't trigger any driver magic or so. */
        fd = open(path, O_PATH|O_CLOEXEC|O_NOFOLLOW);
        if (fd < 0) {
                if (errno != ENOENT)
                        return -errno;

                /* if the node doesn't exist yet, we create it, but with O_EXCL, so that we only create a regular file
                 * here, and nothing else */
                fd = open(path, O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC, IN_SET(mode, 0, MODE_INVALID) ? 0644 : mode);
                if (fd < 0)
                        return -errno;
        }

        /* Let's make a path from the fd, and operate on that. With this logic, we can adjust the access mode,
         * ownership and time of the file node in all cases, even if the fd refers to an O_PATH object — which is
         * something fchown(), fchmod(), futimensat() don't allow. */
        xsprintf(fdpath, "/proc/self/fd/%i", fd);

        if (mode != MODE_INVALID)
                if (chmod(fdpath, mode) < 0)
                        ret = -errno;

        if (uid_is_valid(uid) || gid_is_valid(gid))
                if (chown(fdpath, uid, gid) < 0 && ret >= 0)
                        ret = -errno;

        if (stamp != USEC_INFINITY) {
                struct timespec ts[2];

                timespec_store(&ts[0], stamp);
                ts[1] = ts[0];
                r = utimensat(AT_FDCWD, fdpath, ts, 0);
        } else
                r = utimensat(AT_FDCWD, fdpath, NULL, 0);
        if (r < 0 && ret >= 0)
                return -errno;

        return ret;
}

int touch(const char *path) {
        return touch_file(path, false, USEC_INFINITY, UID_INVALID, GID_INVALID, MODE_INVALID);
}

static int getenv_tmp_dir(const char **ret_path) {
        const char *n;
        int r, ret = 0;

        assert(ret_path);

        /* We use the same order of environment variables python uses in tempfile.gettempdir():
         * https://docs.python.org/3/library/tempfile.html#tempfile.gettempdir */
        FOREACH_STRING(n, "TMPDIR", "TEMP", "TMP") {
                const char *e;

                e = secure_getenv(n);
                if (!e)
                        continue;
                if (!path_is_absolute(e)) {
                        r = -ENOTDIR;
                        goto next;
                }
                if (!path_is_normalized(e)) {
                        r = -EPERM;
                        goto next;
                }

                r = is_dir(e, true);
                if (r < 0)
                        goto next;
                if (r == 0) {
                        r = -ENOTDIR;
                        goto next;
                }

                *ret_path = e;
                return 1;

        next:
                /* Remember first error, to make this more debuggable */
                if (ret >= 0)
                        ret = r;
        }

        if (ret < 0)
                return ret;

        *ret_path = NULL;
        return ret;
}

static int tmp_dir_internal(const char *def, const char **ret) {
        const char *e;
        int r, k;

        assert(def);
        assert(ret);

        r = getenv_tmp_dir(&e);
        if (r > 0) {
                *ret = e;
                return 0;
        }

        k = is_dir(def, true);
        if (k == 0)
                k = -ENOTDIR;
        if (k < 0)
                return r < 0 ? r : k;

        *ret = def;
        return 0;
}

int var_tmp_dir(const char **ret) {

        /* Returns the location for "larger" temporary files, that is backed by physical storage if available, and thus
         * even might survive a boot: /var/tmp. If $TMPDIR (or related environment variables) are set, its value is
         * returned preferably however. Note that both this function and tmp_dir() below are affected by $TMPDIR,
         * making it a variable that overrides all temporary file storage locations. */

        return tmp_dir_internal("/var/tmp", ret);
}

int tmp_dir(const char **ret) {

        /* Similar to var_tmp_dir() above, but returns the location for "smaller" temporary files, which is usually
         * backed by an in-memory file system: /tmp. */

        return tmp_dir_internal("/tmp", ret);
}

static int open_parent(const char *path, int flags, mode_t mode);

int fsync_directory_of_file(int fd) {
        _cleanup_free_ char *path = NULL;
        _cleanup_close_ int dfd = -1;
        int r;

        r = fd_verify_regular(fd);
        if (r < 0)
                return r;

        r = fd_get_path(fd, &path);
        if (r < 0) {
                log_debug_errno(r, "Failed to query /proc/self/fd/%d%s: %m",
                                fd,
                                r == -EOPNOTSUPP ? ", ignoring" : "");

                if (r == -EOPNOTSUPP)
                        /* If /proc is not available, we're most likely running in some
                         * chroot environment, and syncing the directory is not very
                         * important in that case. Let's just silently do nothing. */
                        return 0;

                return r;
        }

        if (!path_is_absolute(path))
                return -EINVAL;

        dfd = open_parent(path, O_CLOEXEC, 0);
        if (dfd < 0)
                return dfd;

        if (fsync(dfd) < 0)
                return -errno;

        return 0;
}

static int open_parent(const char *path, int flags, mode_t mode) {
        _cleanup_free_ char *parent = NULL;
        int fd;

        if (isempty(path))
                return -EINVAL;
        if (path_equal(path, "/")) /* requesting the parent of the root dir is fishy, let's prohibit that */
                return -EINVAL;

        parent = dirname_malloc(path);
        if (!parent)
                return -ENOMEM;

        /* Let's insist on O_DIRECTORY since the parent of a file or directory is a directory. Except if we open an
         * O_TMPFILE file, because in that case we are actually create a regular file below the parent directory. */

        if ((flags & O_PATH) == O_PATH)
                flags |= O_DIRECTORY;

        fd = open(parent, flags, mode);
        if (fd < 0)
                return -errno;

        return fd;
}
