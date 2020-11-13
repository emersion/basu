/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <uchar.h>

#include "string-util.h"
#include "missing.h"

char *cescape(const char *s);
char *cescape_length(const char *s, size_t n);
int cescape_char(char c, char *buf);
