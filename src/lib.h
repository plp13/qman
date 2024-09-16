// Header file includes

#ifndef LIB_H

#define LIB_H

#include <assert.h>
#include <alloca.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <locale.h>
#include <string.h>
#include <wchar.h>
#include <time.h>
#include <regex.h>
#include <curses.h>
#include <term.h>
#undef lines
#include <ini.h>

#define _GNU_SOURCE
extern char *program_invocation_short_name;
#undef _GNU_SOURCE

#include "util.h"
#include "base64.h"
#include "config.h"
#include "program.h"
#include "cli.h"
#include "tui.h"

#endif
