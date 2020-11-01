/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>

#include "macro.h"

/* Regular colors */
#define ANSI_BLACK   "\x1B[0;30m"
#define ANSI_RED     "\x1B[0;31m"
#define ANSI_GREEN   "\x1B[0;32m"
#define ANSI_YELLOW  "\x1B[0;33m"
#define ANSI_BLUE    "\x1B[0;34m"
#define ANSI_MAGENTA "\x1B[0;35m"
#define ANSI_CYAN    "\x1B[0;36m"
#define ANSI_WHITE   "\x1B[0;37m"

/* Bold/highlighted */
#define ANSI_HIGHLIGHT_BLACK   "\x1B[0;1;30m"
#define ANSI_HIGHLIGHT_RED     "\x1B[0;1;31m"
#define ANSI_HIGHLIGHT_GREEN   "\x1B[0;1;32m"
#define ANSI_HIGHLIGHT_YELLOW  "\x1B[0;1;33m"
#define ANSI_HIGHLIGHT_BLUE    "\x1B[0;1;34m"
#define ANSI_HIGHLIGHT_MAGENTA "\x1B[0;1;35m"
#define ANSI_HIGHLIGHT_CYAN    "\x1B[0;1;36m"
#define ANSI_HIGHLIGHT_WHITE   "\x1B[0;1;37m"

/* Underlined */
#define ANSI_HIGHLIGHT_BLACK_UNDERLINE   "\x1B[0;1;4;30m"
#define ANSI_HIGHLIGHT_RED_UNDERLINE     "\x1B[0;1;4;31m"
#define ANSI_HIGHLIGHT_GREEN_UNDERLINE   "\x1B[0;1;4;32m"
#define ANSI_HIGHLIGHT_YELLOW_UNDERLINE  "\x1B[0;1;4;33m"
#define ANSI_HIGHLIGHT_BLUE_UNDERLINE    "\x1B[0;1;4;34m"
#define ANSI_HIGHLIGHT_MAGENTA_UNDERLINE "\x1B[0;1;4;35m"
#define ANSI_HIGHLIGHT_CYAN_UNDERLINE    "\x1B[0;1;4;36m"
#define ANSI_HIGHLIGHT_WHITE_UNDERLINE   "\x1B[0;1;4;37m"

/* Other ANSI codes */
#define ANSI_UNDERLINE "\x1B[0;4m"
#define ANSI_HIGHLIGHT "\x1B[0;1;39m"
#define ANSI_HIGHLIGHT_UNDERLINE "\x1B[0;1;4m"

/* Reset/clear ANSI styles */
#define ANSI_NORMAL "\x1B[0m"

/* Erase characters until the end of the line */
#define ANSI_ERASE_TO_END_OF_LINE "\x1B[K"

/* Move cursor up one line */
#define ANSI_REVERSE_LINEFEED "\x1BM"

/* Set cursor to top left corner and clear screen */
#define ANSI_HOME_CLEAR "\x1B[H\x1B[2J"

int open_terminal(const char *name, int mode);

int fd_columns(int fd);
unsigned columns(void);
int fd_lines(int fd);
unsigned lines(void);

bool on_tty(void);
bool terminal_is_dumb(void);
bool colors_enabled(void);
bool underline_enabled(void);

#define DEFINE_ANSI_FUNC(name, NAME)                            \
        static inline const char *ansi_##name(void) {           \
                return colors_enabled() ? ANSI_##NAME : "";     \
        }

#define DEFINE_ANSI_FUNC_UNDERLINE(name, NAME, REPLACEMENT)             \
        static inline const char *ansi_##name(void) {                   \
                return underline_enabled() ? ANSI_##NAME :              \
                        colors_enabled() ? ANSI_##REPLACEMENT : "";     \
        }

DEFINE_ANSI_FUNC(highlight,                  HIGHLIGHT);
DEFINE_ANSI_FUNC(highlight_red,              HIGHLIGHT_RED);
DEFINE_ANSI_FUNC(highlight_green,            HIGHLIGHT_GREEN);
DEFINE_ANSI_FUNC(highlight_yellow,           HIGHLIGHT_YELLOW);
DEFINE_ANSI_FUNC(highlight_blue,             HIGHLIGHT_BLUE);
DEFINE_ANSI_FUNC(highlight_magenta,          HIGHLIGHT_MAGENTA);
DEFINE_ANSI_FUNC(normal,                     NORMAL);

DEFINE_ANSI_FUNC_UNDERLINE(underline,                  UNDERLINE, NORMAL);
DEFINE_ANSI_FUNC_UNDERLINE(highlight_underline,        HIGHLIGHT_UNDERLINE, HIGHLIGHT);
DEFINE_ANSI_FUNC_UNDERLINE(highlight_red_underline,    HIGHLIGHT_RED_UNDERLINE, HIGHLIGHT_RED);
DEFINE_ANSI_FUNC_UNDERLINE(highlight_green_underline,  HIGHLIGHT_GREEN_UNDERLINE, HIGHLIGHT_GREEN);
DEFINE_ANSI_FUNC_UNDERLINE(highlight_yellow_underline, HIGHLIGHT_YELLOW_UNDERLINE, HIGHLIGHT_YELLOW);
DEFINE_ANSI_FUNC_UNDERLINE(highlight_blue_underline,   HIGHLIGHT_BLUE_UNDERLINE, HIGHLIGHT_BLUE);

int get_ctty_devnr(pid_t pid, dev_t *d);
int get_ctty(pid_t, dev_t *_devnr, char **r);

int terminal_urlify(const char *url, const char *text, char **ret);
int terminal_urlify_man(const char *page, const char *section, char **ret);

