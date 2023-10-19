// Text user interface (definition)

#ifndef TUI_H

#define TUI_H

#include "lib.h"
#include "program.h"
#include "util.h"

//
// Global variables
//

// Main window where the current page is displayed
extern WINDOW *wmain;

// Scrollbar window
extern WINDOW *wsbar;

// Status bar window
extern WINDOW *wstat;

//
// Macros
//

// True if the terminal supports at least 8 colours
#define COLOUR (has_colors() && COLORS >= 8)

// Set the colour for window win to col (a variable of type colour_t)
#define set_colour(win, col)                                                   \
  if (COLOUR) {                                                                \
    if (col.bold)                                                              \
      wattr_set(win, WA_BOLD, col.pair, NULL);                                 \
    else                                                                       \
      wattr_set(win, WA_NORMAL, col.pair, NULL);                               \
  }

//
// Functions
//

// Initialize ncurses
extern void init_tui();

// init_windows() and all draw_...() functions call wnoutrefresh() in order to
// update the virtual screen, before returning. It's your responsibility to call
// doupdate() afterwards, to update the physical screen.

// Delete and re-initialize all windows. After calling this function, you must
// also call all draw..() functions as needed.
extern void init_windows();

// If terminal width and/or height have changed, update config.layout and return
// true. Otherwise, return false.
extern bool termsize_changed();

// Draw the portion of a page the user is supposed to see in wmain. The page is
// contained in lines and is lines_len long. lines_top specifies the line where
// the portion begins.
extern void draw_page(line_t *lines, unsigned lines_len, unsigned lines_top);

// Draw the scrollbar in wsbar. lines_len indicates the total number of lines in
// the page being displayed, and lines_top the line number where the visible
// portion of said page begins.
extern void draw_sbar(unsigned lines_len, unsigned lines_top);

// Draw the status bar in wstat. lines_len indicates the total number of lines
// in the page being displayed, and lines_cur the line number currently in
// focus. help and prompt are respectively the help text and prompt strings.
extern void draw_stat(unsigned lines_len, unsigned lines_pos, wchar_t *help,
                      wchar_t *prompt);

// Delete all windows and wind down ncurses. No need to call this function
// normally, as it's called by winddown().
extern void winddown_tui();

#endif
