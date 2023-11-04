// Text user interface (definition)

#ifndef TUI_H

#define TUI_H

#include "lib.h"
#include "program.h"

//
// Global variables
//

// Main window where the current page is displayed
extern WINDOW *wmain;

// Width and height of the main window
extern unsigned wmain_width, wmain_height;

// Scrollbar window
extern WINDOW *wsbar;

// Status bar window
extern WINDOW *wstat;

extern action_t action;

//
// Macros
//

// True if the terminal supports at least 8 colours
#define COLOUR (has_colors() && COLORS >= 8)

// Change the colour for window win to col (a variable of type colour_t)
#define change_colour(win, col)                                                \
  if (COLOUR) {                                                                \
    if (col.bold)                                                              \
      wattr_set(win, WA_BOLD, col.pair, NULL);                                 \
    else                                                                       \
      wattr_set(win, WA_NORMAL, col.pair, NULL);                               \
  }

// Apply colour col to n characters, starting at location (y, x) in window w
#define apply_colour(win, y, x, n, col)                                        \
  if (COLOUR) {                                                                \
    if (col.bold)                                                              \
      mvwchgat(win, y, x, n, WA_BOLD, col.pair, NULL);                         \
    else                                                                       \
      mvwchgat(win, y, x, n, WA_NORMAL, col.pair, NULL);                       \
  }

//
// Functions (utility)
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
// the portion begins. flink indicates the location of the focused link.
extern void draw_page(line_t *lines, unsigned lines_len, unsigned lines_top,
                      link_loc_t flink);

// Draw the scrollbar in wsbar. lines_len is total number of lines in the page
// being displayed, and lines_top the line number where the visible portion of
// said page begins.
extern void draw_sbar(unsigned lines_len, unsigned lines_top);

// Draw the status bar in wstat:
// mode -- current mode of operation (INDEX, MAN, APROPOS, WHATIS, etc.)
// name -- current page name
// lines_len -- total number of lines in page
// lines_pos -- focused line number in page
// prompt -- cursor prompt
// help -- help text
// em -- error message
// (Normally, we will display help. If, however, help is NULL, we'll display
// em.)
extern void draw_stat(wchar_t *mode, wchar_t *name, unsigned lines_len,
                      unsigned lines_pos, wchar_t *prompt, wchar_t *help,
                      wchar_t *em);

// Return the program action number that corresponds to input character chr. If
// no such action, return -1.
extern action_t get_action(int chr);

// Beeo if config.layout.beep is true
extern void cbeep();

// Delete all windows and wind down ncurses. No need to call this function
// normally, as it's called by winddown().
extern void winddown_tui();

//
// Functions (handlers)
//

// Error handler: display em in the status bar, and call cbeep();
void tui_error(wchar_t *em);

// Handler for PA_UP
extern bool tui_up();

// Handler for PA_DOWN
extern bool tui_down();

// Handler for PA_PGUP
extern bool tui_pgup();

// Handler for PA_PGDN
extern bool tui_pgdn();

// Handler for PA_HOME
extern bool tui_home();

// Handler for PA_END
extern bool tui_end();

// Handler for PA_OPEN
extern bool tui_open();

// Handler of PA_OPEN_APROPOS
extern bool tui_open_apropos();

// Handler of PA_OPEN_WHATIS
extern bool tui_open_whatis();

// Handler for PA_INDEX
extern bool tui_index();

// Handler for PA_BACK
extern bool tui_back();

// Handler for PA_FWRD
extern bool tui_fwrd();

// Main handler/loop for the TUI
extern void tui();

#endif
