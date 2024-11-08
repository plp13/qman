// Text user interface (definition)

#ifndef TUI_H

#define TUI_H

#include "lib.h"

//
// Types
//

// Terminal capabilities
typedef struct {
  char *term;     // contents of the TERM environment variable
  short colours;  // number of colors supported by the terminal, or 0 if the
                  // terminal is black and white
  bool rgb;       // true if terminal colors can be re-defined
  bool unicode;   // true if the terminal supports Unicode
  bool clipboard; // true if the terminal supports clipboard operations (OSC 52)
  unsigned escdelay; // terminal escape delay
} tcap_t;

// A mouse button
typedef enum {
  BT_NONE,  // n/a
  BT_LEFT,  // left button
  BT_RIGHT, // right button
  BT_WHEEL  // wheel button
} mouse_button_t;

// Mouse wheel activation
typedef enum {
  WH_NONE, // n/a
  WH_UP,   // up
  WH_DOWN  // down
} mouse_wheel_t;

// Compiled mouse status (not full; only essential parametres are recorded)
typedef struct {
  mouse_button_t button; // which mouse button
  bool down;             // the button was pressed
  bool up;               // the button was released
  bool dnd;    // we are in drag-and-drop (button was previously pressed but not
               // yet released)
  short dnd_y; // vertical position where drag-and-drop was initiated
  short dnd_x; // horizontal position where drag-and-drop was initiated
  mouse_wheel_t wheel; // activation of the mouse wheel
  short y;             // cursor vertical position
  short x;             // cursor horizontal position
} mouse_t;

//
// Constants
//

// empty mouse status (used for initialization)
#define MS_EMPTY {BT_NONE, false, false, false, -1, -1, WH_NONE, -1, -1}

//
// Global variables
//

// Terminal capabilities
extern tcap_t tcap;

// (ncurses windows)

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

// (program state)

// Latest action
extern action_t action;

// Latest mouse status
extern mouse_t mouse_status;

//
// Macros
//

// Initialize ncurses color pair in (colour_t value) col
#define init_colour(col) init_pair(col.pair, col.fg, col.bg);

// Change the color for window win to col (a variable of type colour_t)
#define change_colour(win, col)                                                \
  {                                                                            \
    if (tcap.colours) {                                                        \
      if (col.bold)                                                            \
        wattr_set(win, WA_BOLD, col.pair, NULL);                               \
      else                                                                     \
        wattr_set(win, WA_NORMAL, col.pair, NULL);                             \
    } else if (COLOR_BLACK == col.fg && (wmain == win || wimm == win))         \
      wattr_set(win, WA_REVERSE, config.colours.fallback.pair, NULL);          \
    else                                                                       \
      wattr_set(win, WA_NORMAL, config.colours.fallback.pair, NULL);           \
  }

// Change the color for window win to col, and also set text attribute for
// window win to attr
#define change_colour_attr(win, col, attr)                                     \
  {                                                                            \
    if (tcap.colours)                                                          \
      wattr_set(win, attr, col.pair, NULL);                                    \
    else if (wmain == win || wimm == win)                                      \
      wattr_set(win, attr, config.colours.fallback.pair, NULL);                \
  }

// Apply color col to n characters, starting at location (y, x) in window w
#define apply_colour(win, y, x, n, col)                                        \
  {                                                                            \
    if (tcap.colours) {                                                        \
      if (col.bold)                                                            \
        mvwchgat(win, y, x, n, WA_BOLD, col.pair, NULL);                       \
      else                                                                     \
        mvwchgat(win, y, x, n, WA_NORMAL, col.pair, NULL);                     \
    } else {                                                                   \
      if (COLOR_BLACK == col.fg && (wmain == win || wimm == win))              \
        mvwchgat(win, y, x, n, WA_REVERSE, config.colours.fallback.pair,       \
                 NULL);                                                        \
      else                                                                     \
        mvwchgat(win, y, x, n, WA_NORMAL, config.colours.fallback.pair, NULL); \
    }                                                                          \
  }

//
// Functions (utility)
//

// Initialize and set up ncurses, and also initialize the tcap global
extern void init_tui();

// Initialize tcap with the correct terminal capabilities. These are normally
// sniffed, but this can be overridden in the [tcap] configuration section.
extern void init_tui_tcap();

// Initialize ncurses color pairs
extern void init_tui_colours();

// Initialize ncurses mouse support
extern void init_tui_mouse();

// Send escape secuense s to the terminal. This is done by bypassing ncurses. s
// must not include the initial escape character.
extern void sendescseq(char *s);

// init_windows() and all draw_...() functions call wnoutrefresh() in order to
// update the virtual screen before returning. It's your responsibility to call
// doupdate() afterwards, to update the physical screen.

// Delete and re-initialize all windows. After calling this function, you must
// also call all draw..() functions as needed.
extern void init_windows();

// If terminal width and/or height have changed, update config.layout and return
// true. Otherwise, return false.
extern bool termsize_changed();

// Wrapper for getch(). Makes the cursor visible right before getch() is called,
// invisible right after
extern int cgetch();

// Return a (statically allocated) string representation of key character k
extern wchar_t *ch2name(int k);

// Corrects page_top and page_flink whenever the terminal has been resized. Must
// be called whenever termsize_changed() returned true and right before calling
// tui_redraw().
extern void termsize_adjust();

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
extern void draw_stat(const wchar_t *mode, const wchar_t *name,
                      unsigned lines_len, unsigned lines_pos,
                      const wchar_t *prompt, const wchar_t *help,
                      const wchar_t *em);

// Draw an immediate window. is_long specifies whether the window is long or
// short. is_wide whether it is wide or narrow. The title and help strings is
// quite obvious what they are.
void draw_imm(bool is_long, bool is_wide, const wchar_t *title,
              const wchar_t *help);

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
// - 0, if the user hit ESC or CTRL-C (or pressed left mouse)
// - n, if the user hit ENTER (or pressed right mouse), where n is the total
//      number of typed characters
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

// Return the current compiled mouse status, after receiving input character chr
extern mouse_t get_mouse_status(int chr);

// Beep if config.layout.beep is true
extern void cbeep();

// Beep if config.layout.beep is true, and terminal size has not been changed
extern void ctbeep();

// Copy src to clipboard. This is done using the escape code 52 (which may or
// may not be supported by the user's terminal) and also via xclip (if running
// in X11) or wl-copy (if running in Wayland), to ensure the maximum possible
// success rate.
extern void editcopy(wchar_t *src);

// Delete all windows and wind down ncurses. No need to call this function
// normally, as it's called by winddown().
extern void winddown_tui();

//
// Functions (handlers)
//

// Redraw everythhing on the screen, calling draw_page(), draw_sbar() and
// draw_stat()
extern void tui_redraw();

// Error handler: display em in the status bar, and call cbeep();
void tui_error(wchar_t *em);

// Handler for PA_UP
extern bool tui_up();

// Handler for PA_DOWN
extern bool tui_down();

// Handler for PA_LEFT
extern bool tui_left();

// Handler for PA_RIGHT
extern bool tui_right();

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

// Handler for PA_HISTORY
extern bool tui_history();

// Handler for PA_TOC
extern bool tui_toc();

// Handler for PA_SEARCH (whenever back is set to true) or PA_SEARCH_BACK
// (whenever back is set to false)
extern bool tui_search(bool back);

// Handler for PA_SEARCH_PREV (whenever back is set to true) or PA_SEARCH_NEXT
// (whenever back is set to false)
extern bool tui_search_next(bool back);

// Handler for PA_HELP
extern bool tui_help();

// Called whenever the left mouse button is pressed at position (y, x)
extern bool tui_mouse_click(short y, short x);

// Called whenever the mouse is left-button dragged to position (y, x). (dy, dx)
// indicates the position the dragging was initiated.
extern bool tui_mouse_dnd(short y, short x, short dy, short dx);

// Main handler/loop for the TUI
extern void tui();

#endif
