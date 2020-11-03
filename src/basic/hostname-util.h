/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

#include <stdbool.h>
#include <stdio.h>

#include "macro.h"

bool hostname_is_valid(const char *s, bool allow_trailing_dot) _pure_;

#define machine_name_is_valid(s) hostname_is_valid(s, false)
