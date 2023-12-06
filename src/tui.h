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

// Immediate (popup) window
extern WINDOW *wimm;

extern action_t action;

//
// Macros
//

// True if the terminal supports at least 8 colours
#define COLOUR (has_colors() && COLORS >= 8)

// Initialize colour pair col
#define init_colour(col) init_pair(col.pair, col.fg, col.bg);

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

// Draw a box in w, starting at (tl_y, tl_x) and ending at (br_y, br_x)
extern void draw_box(WINDOW *w, unsigned tl_y, unsigned tl_x, unsigned br_y,
                     unsigned br_x);

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

// Draw an immediate window. is_long specifies whether the window is long or
// short, while title is quite obvious.
void draw_imm(bool is_long, wchar_t *title);

// Delete the immediate window previously drawn with draw_imm()
void del_imm();

// Move to (y, x) in w and read a string into trgt (of length trgt_len)
extern bool get_str(WINDOW *w, unsigned y, unsigned x, wchar_t *trgt,
                    unsigned trgt_len);

// Interactive version of get_str() that returns after each keystroke. Use like
// strtok(), i.e. specify trgt and trgt_len on first call, and set them to NULL
// and 0 respectively on subsequent calls. The function updates trgt and returns
// whenever the user types something. Return value varies depending on the
// user's action:
// - 0, if the user hit ESC or CTRL-C
// - n, if the user hit ENTER, where n is the total number of typed
//   characters
// - -KEY_UP, if the user hit the UP arrow key
// - -KEY_DOWN, if the user hit the DOWN arrow key
// - -KEY_PPAGE, if the user hit PGUP
// - -KEY_NPAGE, if the user hit PGDN
// - -KEY_HOME, if the user hit HOME
// - -KEY_END, if the user hit END
// - -0x09 (TAB in ASCII), if the user hit TAB
// - -KEY_BACKSPACE, if the user hit BACKSPACE
// - -chr, if the user typed any text character chr
extern int get_str_next(WINDOW *w, unsigned y, unsigned x, wchar_t *trgt,
                        unsigned trgt_len);

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

// Redraw everythhing on the screen, calling draw_page(), draw_sbar() and
// draw_reset()
extern void tui_redraw();

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

// Handler for PA_SP_OPEN, PA_SP_APROPOS, and PA_SP_WHATIS. Opens a manual,
// apropos, or whatis page (depending on the value of rt) that is specified by
// the user in an immediate (pop-up) window.
extern bool tui_sp_open(request_type_t rt);

// Handler for PA_INDEX
extern bool tui_index();

// Handler for PA_BACK
extern bool tui_back();

// Handler for PA_FWRD
extern bool tui_fwrd();

// Handler for PA_SEARCH (whenever back is set to true) or PA_SEARCH_BACK
// (whenever back is set to false)
extern bool tui_search(bool back);

// Handler for PA_SEARCH_PREV (whenever back is set to true) or PA_SEARCH_NEXT
// (whenever back is set to false)
extern bool tui_search_next(bool back);

// Handler for PA_HELP
extern bool tui_help();

// Main handler/loop for the TUI
extern void tui();

#endif
