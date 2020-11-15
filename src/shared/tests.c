/* SPDX-License-Identifier: LGPL-2.1+ */

#include <stdlib.h>
#include <util.h>

#include "env-util.h"
#include "tests.h"

bool slow_tests_enabled(void) {
        int r;

        r = getenv_bool("SYSTEMD_SLOW_TESTS");
        if (r >= 0)
                return r;

        if (r != -ENXIO)
                log_warning_errno(r, "Cannot parse $SYSTEMD_SLOW_TESTS, ignoring.");
        return SYSTEMD_SLOW_TESTS_DEFAULT;
}

void test_setup_logging(int level) {
        log_set_max_level(level);
        log_parse_environment();
}

int log_tests_skipped(const char *message) {
        log_notice("%s: %s, skipping tests.",
                   program_invocation_short_name, message);
        return EXIT_TEST_SKIP;
}
