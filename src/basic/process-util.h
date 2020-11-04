/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

#include <alloca.h>
#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>

#include "format-util.h"
#include "macro.h"
#include "time-util.h"

#define procfs_file_alloca(pid, field)                                  \
        ({                                                              \
                pid_t _pid_ = (pid);                                    \
                const char *_r_;                                        \
                if (_pid_ == 0) {                                       \
                        _r_ = ("/proc/self/" field);                    \
                } else {                                                \
                        _r_ = alloca(STRLEN("/proc/") + DECIMAL_STR_MAX(pid_t) + 1 + sizeof(field)); \
                        sprintf((char*) _r_, "/proc/"PID_FMT"/" field, _pid_);                       \
                }                                                       \
                _r_;                                                    \
        })

int get_process_state(pid_t pid);
int get_process_comm(pid_t pid, char **name);
int get_process_exe(pid_t pid, char **name);

int wait_for_terminate(pid_t pid, siginfo_t *status);

typedef enum WaitFlags {
        WAIT_LOG_ABNORMAL             = 1 << 0,
        WAIT_LOG_NON_ZERO_EXIT_STATUS = 1 << 1,

        /* A shortcut for requesting the most complete logging */
        WAIT_LOG = WAIT_LOG_ABNORMAL|WAIT_LOG_NON_ZERO_EXIT_STATUS,
} WaitFlags;

int wait_for_terminate_and_check(const char *name, pid_t pid, WaitFlags flags);
int wait_for_terminate_with_timeout(pid_t pid, usec_t timeout);

void sigkill_wait(pid_t pid);
void sigterm_wait(pid_t pid);

int kill_and_sigcont(pid_t pid, int sig);

int rename_process(const char name[]);

int getenv_for_pid(pid_t pid, const char *field, char **_value);

bool pid_is_alive(pid_t pid);
bool pid_is_unwaited(pid_t pid);

bool is_main_thread(void);

static inline pid_t PTR_TO_PID(const void *p) {
        return (pid_t) ((uintptr_t) p);
}

static inline void* PID_TO_PTR(pid_t pid) {
        return (void*) ((uintptr_t) pid);
}

static inline bool nice_is_valid(int n) {
        return n >= PRIO_MIN && n < PRIO_MAX;
}

static inline bool pid_is_valid(pid_t p) {
        return p > 0;
}

pid_t getpid_cached(void);

int must_be_root(void);

typedef enum ForkFlags {
        FORK_RESET_SIGNALS = 1 << 0,
        FORK_CLOSE_ALL_FDS = 1 << 1,
        FORK_DEATHSIG      = 1 << 2,
        FORK_NULL_STDIO    = 1 << 3,
        FORK_REOPEN_LOG    = 1 << 4,
        FORK_LOG           = 1 << 5,
        FORK_WAIT          = 1 << 6,
        FORK_NEW_MOUNTNS   = 1 << 7,
        FORK_MOUNTNS_SLAVE = 1 << 8,
} ForkFlags;

int safe_fork_full(const char *name, const int except_fds[], size_t n_except_fds, ForkFlags flags, pid_t *ret_pid);

static inline int safe_fork(const char *name, ForkFlags flags, pid_t *ret_pid) {
        return safe_fork_full(name, NULL, 0, flags, ret_pid);
}

#if SIZEOF_PID_T == 4
/* The highest possibly (theoretic) pid_t value on this architecture. */
#define PID_T_MAX ((pid_t) INT32_MAX)
/* The maximum number of concurrent processes Linux allows on this architecture, as well as the highest valid PID value
 * the kernel will potentially assign. This reflects a value compiled into the kernel (PID_MAX_LIMIT), and sets the
 * upper boundary on what may be written to the /proc/sys/kernel/pid_max sysctl (but do note that the sysctl is off by
 * 1, since PID 0 can never exist and there can hence only be one process less than the limit would suggest). Since
 * these values are documented in proc(5) we feel quite confident that they are stable enough for the near future at
 * least to define them here too. */
#define TASKS_MAX 4194303U
#elif SIZEOF_PID_T == 2
#define PID_T_MAX ((pid_t) INT16_MAX)
#define TASKS_MAX 32767U
#else
#error "Unknown pid_t size"
#endif

assert_cc(TASKS_MAX <= (unsigned long) PID_T_MAX)

