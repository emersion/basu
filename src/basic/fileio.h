/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

int read_one_line_file(const char *fn, char **line);
int read_full_file(const char *fn, char **contents, size_t *size);
int read_full_stream(FILE *f, char **contents, size_t *size);

int parse_env_filev(FILE *f, const char *fname, const char *separator, va_list ap);
int parse_env_file(FILE *f, const char *fname, const char *separator, ...) _sentinel_;

int fflush_and_check(FILE *f);

int read_line(FILE *f, size_t limit, char **ret);
