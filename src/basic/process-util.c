/* SPDX-License-Identifier: LGPL-2.1+ */

#include <ctype.h>
#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <syslog.h>
#include <unistd.h>
#if HAVE_VALGRIND_VALGRIND_H
#endif

#include "alloc-util.h"
#include "def.h"
#include "escape.h"
#include "fd-util.h"
#include "fileio.h"
#include "fs-util.h"
#include "process-util.h"
#include "string-util.h"
#include "util.h"

static int reset_all_signal_handlers(void) {
        static const struct sigaction sa = {
                .sa_handler = SIG_DFL,
                .sa_flags = SA_RESTART,
        };
        int sig, r = 0;

        for (sig = 1; sig < _NSIG; sig++) {

                /* These two cannot be caught... */
                if (IN_SET(sig, SIGKILL, SIGSTOP))
                        continue;

                /* On Linux the first two RT signals are reserved by
                 * glibc, and sigaction() will return EINVAL for them. */
                if ((sigaction(sig, &sa, NULL) < 0))
                        if (errno != EINVAL && r >= 0)
                                r = -errno;
        }

        return r;
}

static int reset_signal_mask(void) {
        sigset_t ss;

        if (sigemptyset(&ss) < 0)
                return -errno;

        if (sigprocmask(SIG_SETMASK, &ss, NULL) < 0)
                return -errno;

        return 0;
}

int get_process_state(pid_t pid) {
        const char *p;
        char state;
        int r;
        _cleanup_free_ char *line = NULL;

        assert(pid >= 0);

        p = procfs_file_alloca(pid, "stat");

        r = read_one_line_file(p, &line);
        if (r == -ENOENT)
                return -ESRCH;
        if (r < 0)
                return r;

        p = strrchr(line, ')');
        if (!p)
                return -EIO;

        p++;

        if (sscanf(p, " %c", &state) != 1)
                return -EIO;

        return (unsigned char) state;
}

int get_process_comm(pid_t pid, char **ret) {
        _cleanup_free_ char *escaped = NULL, *comm = NULL;
        const char *p;
        int r;

        assert(ret);
        assert(pid >= 0);

        escaped = new(char, TASK_COMM_LEN);
        if (!escaped)
                return -ENOMEM;

        p = procfs_file_alloca(pid, "comm");

        r = read_one_line_file(p, &comm);
        if (r == -ENOENT)
                return -ESRCH;
        if (r < 0)
                return r;

        /* Escape unprintable characters, just in case, but don't grow the string beyond the underlying size */
        cellescape(escaped, TASK_COMM_LEN, comm);

        *ret = TAKE_PTR(escaped);
        return 0;
}

int rename_process(const char name[]) {
        static size_t mm_size = 0;
        static char *mm = NULL;
        bool truncated = false;
        size_t l;

        /* This is a like a poor man's setproctitle(). It changes the comm field, argv[0], and also the glibc's
         * internally used name of the process. For the first one a limit of 16 chars applies; to the second one in
         * many cases one of 10 (i.e. length of "/sbin/init") — however if we have CAP_SYS_RESOURCES it is unbounded;
         * to the third one 7 (i.e. the length of "systemd". If you pass a longer string it will likely be
         * truncated.
         *
         * Returns 0 if a name was set but truncated, > 0 if it was set but not truncated. */

        if (isempty(name))
                return -EINVAL; /* let's not confuse users unnecessarily with an empty name */

        if (!is_main_thread())
                return -EPERM; /* Let's not allow setting the process name from other threads than the main one, as we
                                * cache things without locking, and we make assumptions that PR_SET_NAME sets the
                                * process name that isn't correct on any other threads */

        l = strlen(name);

        /* First step, change the comm field. The main thread's comm is identical to the process comm. This means we
         * can use PR_SET_NAME, which sets the thread name for the calling thread. */
        if (prctl(PR_SET_NAME, name) < 0)
                log_debug_errno(errno, "PR_SET_NAME failed: %m");
        if (l >= TASK_COMM_LEN) /* Linux process names can be 15 chars at max */
                truncated = true;

        /* Second step, change glibc's ID of the process name. */
        if (program_invocation_name) {
                size_t k;

                k = strlen(program_invocation_name);
                strncpy(program_invocation_name, name, k);
                if (l > k)
                        truncated = true;
        }

        /* Third step, completely replace the argv[] array the kernel maintains for us. This requires privileges, but
         * has the advantage that the argv[] array is exactly what we want it to be, and not filled up with zeros at
         * the end. This is the best option for changing /proc/self/cmdline. */

        /* Let's not bother with this if we don't have euid == 0. Strictly speaking we should check for the
         * CAP_SYS_RESOURCE capability which is independent of the euid. In our own code the capability generally is
         * present only for euid == 0, hence let's use this as quick bypass check, to avoid calling mmap() if
         * PR_SET_MM_ARG_{START,END} fails with EPERM later on anyway. After all geteuid() is dead cheap to call, but
         * mmap() is not. */
        if (geteuid() != 0)
                log_debug("Skipping PR_SET_MM, as we don't have privileges.");
        else if (mm_size < l+1) {
                size_t nn_size;
                char *nn;

                nn_size = PAGE_ALIGN(l+1);
                nn = mmap(NULL, nn_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
                if (nn == MAP_FAILED) {
                        log_debug_errno(errno, "mmap() failed: %m");
                        goto use_saved_argv;
                }

                strncpy(nn, name, nn_size);

                /* Now, let's tell the kernel about this new memory */
                if (prctl(PR_SET_MM, PR_SET_MM_ARG_START, (unsigned long) nn, 0, 0) < 0) {
                        /* HACK: prctl() API is kind of dumb on this point.  The existing end address may already be
                         * below the desired start address, in which case the kernel may have kicked this back due
                         * to a range-check failure (see linux/kernel/sys.c:validate_prctl_map() to see this in
                         * action).  The proper solution would be to have a prctl() API that could set both start+end
                         * simultaneously, or at least let us query the existing address to anticipate this condition
                         * and respond accordingly.  For now, we can only guess at the cause of this failure and try
                         * a workaround--which will briefly expand the arg space to something potentially huge before
                         * resizing it to what we want. */
                        log_debug_errno(errno, "PR_SET_MM_ARG_START failed, attempting PR_SET_MM_ARG_END hack: %m");

                        if (prctl(PR_SET_MM, PR_SET_MM_ARG_END, (unsigned long) nn + l + 1, 0, 0) < 0) {
                                log_debug_errno(errno, "PR_SET_MM_ARG_END hack failed, proceeding without: %m");
                                (void) munmap(nn, nn_size);
                                goto use_saved_argv;
                        }

                        if (prctl(PR_SET_MM, PR_SET_MM_ARG_START, (unsigned long) nn, 0, 0) < 0) {
                                log_debug_errno(errno, "PR_SET_MM_ARG_START still failed, proceeding without: %m");
                                goto use_saved_argv;
                        }
                } else {
                        /* And update the end pointer to the new end, too. If this fails, we don't really know what
                         * to do, it's pretty unlikely that we can rollback, hence we'll just accept the failure,
                         * and continue. */
                        if (prctl(PR_SET_MM, PR_SET_MM_ARG_END, (unsigned long) nn + l + 1, 0, 0) < 0)
                                log_debug_errno(errno, "PR_SET_MM_ARG_END failed, proceeding without: %m");
                }

                if (mm)
                        (void) munmap(mm, mm_size);

                mm = nn;
                mm_size = nn_size;
        } else {
                strncpy(mm, name, mm_size);

                /* Update the end pointer, continuing regardless of any failure. */
                if (prctl(PR_SET_MM, PR_SET_MM_ARG_END, (unsigned long) mm + l + 1, 0, 0) < 0)
                        log_debug_errno(errno, "PR_SET_MM_ARG_END failed, proceeding without: %m");
        }

use_saved_argv:
        /* Fourth step: in all cases we'll also update the original argv[], so that our own code gets it right too if
         * it still looks here */

        if (saved_argc > 0) {
                int i;

                if (saved_argv[0]) {
                        size_t k;

                        k = strlen(saved_argv[0]);
                        strncpy(saved_argv[0], name, k);
                        if (l > k)
                                truncated = true;
                }

                for (i = 1; i < saved_argc; i++) {
                        if (!saved_argv[i])
                                break;

                        memzero(saved_argv[i], strlen(saved_argv[i]));
                }
        }

        return !truncated;
}

static int get_process_link_contents(const char *proc_file, char **name) {
        int r;

        assert(proc_file);
        assert(name);

        r = readlink_malloc(proc_file, name);
        if (r == -ENOENT)
                return -ESRCH;
        if (r < 0)
                return r;

        return 0;
}

int get_process_exe(pid_t pid, char **name) {
        const char *p;
        char *d;
        int r;

        assert(pid >= 0);

        p = procfs_file_alloca(pid, "exe");
        r = get_process_link_contents(p, name);
        if (r < 0)
                return r;

        d = endswith(*name, " (deleted)");
        if (d)
                *d = '\0';

        return 0;
}

int wait_for_terminate(pid_t pid, siginfo_t *status) {
        siginfo_t dummy;

        assert(pid >= 1);

        if (!status)
                status = &dummy;

        for (;;) {
                zero(*status);

                if (waitid(P_PID, pid, status, WEXITED) < 0) {

                        if (errno == EINTR)
                                continue;

                        return negative_errno();
                }

                return 0;
        }
}

/*
 * Return values:
 * < 0 : wait_for_terminate() failed to get the state of the
 *       process, the process was terminated by a signal, or
 *       failed for an unknown reason.
 * >=0 : The process terminated normally, and its exit code is
 *       returned.
 *
 * That is, success is indicated by a return value of zero, and an
 * error is indicated by a non-zero value.
 *
 * A warning is emitted if the process terminates abnormally,
 * and also if it returns non-zero unless check_exit_code is true.
 */
int wait_for_terminate_and_check(const char *name, pid_t pid, WaitFlags flags) {
        _cleanup_free_ char *buffer = NULL;
        siginfo_t status;
        int r, prio;

        assert(pid > 1);

        if (!name) {
                r = get_process_comm(pid, &buffer);
                if (r < 0)
                        log_debug_errno(r, "Failed to acquire process name of " PID_FMT ", ignoring: %m", pid);
                else
                        name = buffer;
        }

        prio = flags & WAIT_LOG_ABNORMAL ? LOG_ERR : LOG_DEBUG;

        r = wait_for_terminate(pid, &status);
        if (r < 0)
                return log_full_errno(prio, r, "Failed to wait for %s: %m", strna(name));

        if (status.si_code == CLD_EXITED) {
                if (status.si_status != EXIT_SUCCESS)
                        log_full(flags & WAIT_LOG_NON_ZERO_EXIT_STATUS ? LOG_ERR : LOG_DEBUG,
                                 "%s failed with exit status %i.", strna(name), status.si_status);
                else
                        log_debug("%s succeeded.", name);

                return status.si_status;

        } else if (IN_SET(status.si_code, CLD_KILLED, CLD_DUMPED)) {

                log_full(prio, "%s terminated by signal %d.", strna(name), status.si_status);
                return -EPROTO;
        }

        log_full(prio, "%s failed due to unknown reason.", strna(name));
        return -EPROTO;
}

void sigterm_wait(pid_t pid) {
        assert(pid > 1);

        if (kill_and_sigcont(pid, SIGTERM) > 0)
                (void) wait_for_terminate(pid, NULL);
}

int kill_and_sigcont(pid_t pid, int sig) {
        int r;

        r = kill(pid, sig) < 0 ? -errno : 0;

        /* If this worked, also send SIGCONT, unless we already just sent a SIGCONT, or SIGKILL was sent which isn't
         * affected by a process being suspended anyway. */
        if (r >= 0 && !IN_SET(sig, SIGCONT, SIGKILL))
                (void) kill(pid, SIGCONT);

        return r;
}

bool pid_is_unwaited(pid_t pid) {
        /* Checks whether a PID is still valid at all, including a zombie */

        if (pid < 0)
                return false;

        if (pid <= 1) /* If we or PID 1 would be dead and have been waited for, this code would not be running */
                return true;

        if (pid == getpid_cached())
                return true;

        if (kill(pid, 0) >= 0)
                return true;

        return errno != ESRCH;
}

bool pid_is_alive(pid_t pid) {
        int r;

        /* Checks whether a PID is still valid and not a zombie */

        if (pid < 0)
                return false;

        if (pid <= 1) /* If we or PID 1 would be a zombie, this code would not be running */
                return true;

        if (pid == getpid_cached())
                return true;

        r = get_process_state(pid);
        if (IN_SET(r, -ESRCH, 'Z'))
                return false;

        return true;
}

bool is_main_thread(void) {
        static thread_local int cached = 0;

        if (_unlikely_(cached == 0))
                cached = getpid_cached() == gettid() ? 1 : -1;

        return cached > 0;
}

pid_t getpid_cached(void) {
        return getpid();
}

int must_be_root(void) {

        if (geteuid() == 0)
                return 0;

        log_error("Need to be root.");
        return -EPERM;
}

int safe_fork_full(
                const char *name,
                const int except_fds[],
                size_t n_except_fds,
                ForkFlags flags,
                pid_t *ret_pid) {

        pid_t original_pid, pid;
        sigset_t saved_ss, ss;
        bool block_signals = false;
        int r;

        /* A wrapper around fork(), that does a couple of important initializations in addition to mere forking. Always
         * returns the child's PID in *ret_pid. Returns == 0 in the child, and > 0 in the parent. */

        original_pid = getpid_cached();

        if (flags & (FORK_RESET_SIGNALS|FORK_DEATHSIG)) {

                /* We temporarily block all signals, so that the new child has them blocked initially. This way, we can
                 * be sure that SIGTERMs are not lost we might send to the child. */

                if (sigfillset(&ss) < 0)
                        return log_full_errno(LOG_DEBUG, errno, "Failed to reset signal set: %m");

                block_signals = true;

        }

        if (block_signals)
                if (sigprocmask(SIG_SETMASK, &ss, &saved_ss) < 0)
                        return log_full_errno(LOG_DEBUG, errno, "Failed to set signal mask: %m");

        pid = fork();
        if (pid < 0) {
                r = -errno;

                if (block_signals) /* undo what we did above */
                        (void) sigprocmask(SIG_SETMASK, &saved_ss, NULL);

                return log_full_errno(LOG_DEBUG, r, "Failed to fork: %m");
        }
        if (pid > 0) {
                /* We are in the parent process */

                log_debug("Successfully forked off '%s' as PID " PID_FMT ".", strna(name), pid);

                if (block_signals) /* undo what we did above */
                        (void) sigprocmask(SIG_SETMASK, &saved_ss, NULL);

                if (ret_pid)
                        *ret_pid = pid;

                return 1;
        }

        /* We are in the child process */

        if (name) {
                r = rename_process(name);
                if (r < 0)
                        log_full_errno(flags & LOG_DEBUG,
                                       r, "Failed to rename process, ignoring: %m");
        }

        if (flags & FORK_DEATHSIG)
                if (prctl(PR_SET_PDEATHSIG, SIGTERM) < 0) {
                        log_full_errno(LOG_DEBUG, errno, "Failed to set death signal: %m");
                        _exit(EXIT_FAILURE);
                }

        if (flags & FORK_RESET_SIGNALS) {
                r = reset_all_signal_handlers();
                if (r < 0) {
                        log_full_errno(LOG_DEBUG, r, "Failed to reset signal handlers: %m");
                        _exit(EXIT_FAILURE);
                }

                /* This implicitly undoes the signal mask stuff we did before the fork()ing above */
                r = reset_signal_mask();
                if (r < 0) {
                        log_full_errno(LOG_DEBUG, r, "Failed to reset signal mask: %m");
                        _exit(EXIT_FAILURE);
                }
        } else if (block_signals) { /* undo what we did above */
                if (sigprocmask(SIG_SETMASK, &saved_ss, NULL) < 0) {
                        log_full_errno(LOG_DEBUG, errno, "Failed to restore signal mask: %m");
                        _exit(EXIT_FAILURE);
                }
        }

        if (flags & FORK_DEATHSIG) {
                pid_t ppid;
                /* Let's see if the parent PID is still the one we started from? If not, then the parent
                 * already died by the time we set PR_SET_PDEATHSIG, hence let's emulate the effect */

                ppid = getppid();
                if (ppid == 0)
                        /* Parent is in a differn't PID namespace. */;
                else if (ppid != original_pid) {
                        log_debug("Parent died early, raising SIGTERM.");
                        (void) raise(SIGTERM);
                        _exit(EXIT_FAILURE);
                }
        }

        if (flags & FORK_CLOSE_ALL_FDS) {
                /* Close the logs here in case it got reopened above, as close_all_fds() would close them for us */
                r = close_all_fds(except_fds, n_except_fds);
                if (r < 0) {
                        log_full_errno(LOG_DEBUG, r, "Failed to close all file descriptors: %m");
                        _exit(EXIT_FAILURE);
                }
        }

        if (ret_pid)
                *ret_pid = getpid_cached();

        return 0;
}

