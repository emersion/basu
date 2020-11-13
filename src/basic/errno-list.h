/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

#include <stdbool.h>

const char *errno_to_name(int id);
int errno_from_name(const char *name);
