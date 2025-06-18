// Header file includes

#ifndef LIB_H

#define LIB_H

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <locale.h>
#include <string.h>
#include <wctype.h>
#include <wchar.h>
#include <time.h>
#include <regex.h>
#include <curses.h>
#include <term.h>

#ifdef QMAN_DARWIN
// I can't understand why these are not defined in macOS X; they are definitely
// included in the library. As a workaround, let's re-define them here
#define BUTTON5_PRESSED         NCURSES_MOUSE_MASK(5, NCURSES_BUTTON_PRESSED)
extern NCURSES_EXPORT(int) mvwaddnwstr (WINDOW *, int, int, const wchar_t *, int);
extern NCURSES_EXPORT(int) mvwgetn_wstr (WINDOW *, int, int, wint_t *, int);
extern NCURSES_EXPORT(int) mvwget_wch (WINDOW *, int, int, wint_t *);
extern NCURSES_EXPORT(void) reset_color_pairs (void);
#endif

#undef lines

#ifdef QMAN_GZIP
#include <zlib.h>
#endif

#ifdef QMAN_BZIP2
#include <bzlib.h>
#endif

#ifdef QMAN_LZMA
#include <lzma.h>
#endif

#if defined(__GLIBC__)
extern char *program_invocation_short_name;
#endif

#include "util.h"
#include "eini.h"
#include "base64.h"
#include "config.h"
#include "program.h"
#include "cli.h"
#include "tui.h"

#endif
