// Text user interface (implementation)

#include "tui.h"
#include "lib.h"
#include "program.h"
#include "util.h"
#include <curses.h>

//
// Global variables
//

WINDOW *wmain = NULL;

WINDOW *wsbar = NULL;

WINDOW *wstat = NULL;

//
// Functions
//

void init_tui() {
  // Initialize ncurses
  initscr();
  raw();
  keypad(stdscr, TRUE);
  noecho();
  curs_set(1);

  // Initialize colour (if available)
  start_color();
  if (COLOUR) {
    start_color();
    init_pair(config.colours.text.pair, config.colours.text.fg,
              config.colours.text.bg);
    init_pair(config.colours.search.pair, config.colours.search.fg,
              config.colours.search.bg);
    init_pair(config.colours.link_man.pair, config.colours.link_man.fg,
              config.colours.link_man.bg);
    init_pair(config.colours.link_http.pair, config.colours.link_http.fg,
              config.colours.link_http.bg);
    init_pair(config.colours.link_email.pair, config.colours.link_email.fg,
              config.colours.link_email.bg);
    init_pair(config.colours.link_ls.pair, config.colours.link_ls.fg,
              config.colours.link_ls.bg);
    init_pair(config.colours.sb_line.pair, config.colours.sb_line.fg,
              config.colours.sb_line.bg);
    init_pair(config.colours.sb_block.pair, config.colours.sb_block.fg,
              config.colours.sb_block.bg);
    init_pair(config.colours.stat_indic1.pair, config.colours.stat_indic1.fg,
              config.colours.stat_indic1.bg);
    init_pair(config.colours.stat_indic2.pair, config.colours.stat_indic2.fg,
              config.colours.stat_indic2.bg);
    init_pair(config.colours.stat_input.pair, config.colours.stat_input.fg,
              config.colours.stat_input.bg);
  }
}

void init_windows() {
  if (NULL != wmain)
    delwin(wmain);
  wmain = newwin(config.layout.height - config.layout.stat_height,
                 config.layout.width - config.layout.sb_width, 0, 0);

  if (NULL != wsbar)
    delwin(wsbar);
  wsbar = newwin(config.layout.height - config.layout.stat_height,
                 config.layout.sb_width, 0,
                 config.layout.width - config.layout.sb_width);

  if (NULL != wstat)
    delwin(wstat);
  wstat = newwin(config.layout.stat_height, config.layout.width,
                 config.layout.height - config.layout.stat_height, 0);

  wnoutrefresh(stdscr);
}

bool termsize_changed() {
  int width = getmaxx(stdscr);
  int height = getmaxy(stdscr);

  if (width != config.layout.width || height != config.layout.height) {
    config.layout.width = width;
    config.layout.height = height;

    return true;
  }

  return false;
}

void draw_page(line_t *lines, unsigned lines_len, unsigned lines_top) {
  wclear(wmain);
  change_colour(wmain, config.colours.text);
  wattrset(wmain, WA_NORMAL);

  unsigned y;  // current terminal row
  unsigned ly; // current line
  unsigned x;  // current column (in both line and terminal)
  unsigned i;  // current link

  // For each terminal row...
  for (y = 0; y < getmaxy(wmain); y++) {
    ly = lines_top + y;
    if (ly >= lines_len)
      break;

    // For each terminal column...
    for (x = 0; x < getmaxx(wmain); x++) {
      if (x >= lines[ly].length)
        break;

      // Set text attributes
      if (bget(lines[ly].reg, x))
        wattrset(wmain, WA_NORMAL);
      if (bget(lines[ly].bold, x))
        wattrset(wmain, WA_BOLD);
      if (bget(lines[ly].italic, x))
        wattrset(wmain, WA_STANDOUT);
      if (bget(lines[ly].uline, x))
        wattrset(wmain, WA_UNDERLINE);

      // Place character on screen
      mvwaddnwstr(wmain, y, x, &lines[ly].text[x], 1);
    }

    // For each link...
    for (i = 0; i < lines[ly].links_length; i++) {
      link_t link = lines[ly].links[i];

      // Colourise the the affected text
      colour_t col;
      switch (link.type) {
      case LT_MAN:
        col = config.colours.link_man;
        break;
      case LT_HTTP:
        col = config.colours.link_http;
        break;
      case LT_EMAIL:
        col = config.colours.link_email;
        break;
      case LT_LS:
      default:
        col = config.colours.link_ls;
      }
      apply_colour(wmain, y, link.start, link.end - link.start, col);
    }
  }

  wnoutrefresh(wmain);
}

void draw_sbar(unsigned lines_len, unsigned lines_top) {
  unsigned height = getmaxy(wsbar); // scrollbar height
  unsigned block_pos =
      1 + (((height - 2) * lines_top) / lines_len); // block position

  wclear(wsbar);

  // Draw vertical line
  change_colour(wsbar, config.colours.sb_line);
  mvwvline_set(wsbar, 0, 0, WACS_BTTT, 1);
  mvwvline_set(wsbar, 1, 0, WACS_T_VLINE, height - 2);
  mvwvline_set(wsbar, height - 1, 0, WACS_TTBT, 1);

  // Draw the block
  change_colour(wsbar, config.colours.sb_block);
  mvwaddnwstr(wsbar, block_pos, 0, L"█", -1);

  wnoutrefresh(wsbar);
}

void draw_stat(unsigned lines_len, unsigned lines_pos, wchar_t *help,
               wchar_t *prompt) {
  wclear(wstat);

  unsigned width = getmaxx(wstat); // status lines width

  // Draw no. of lines and position on the indicator line (secondary colour)
  wchar_t tmp[BS_SHORT];
  swprintf(tmp, BS_SHORT, L"%d:%d", lines_len, lines_pos);
  wchar_t *indic2 = walloca(width);
  swprintf(indic2, width + 1, L"%*ls ", width - 1, tmp);
  change_colour(wstat, config.colours.stat_indic2);
  mvwaddnwstr(wstat, 0, 0, indic2, -1);

  // Draw the help text on the indicator line (primary colour)
  change_colour(wstat, config.colours.stat_indic1);
  mvwaddnwstr(wstat, 0, 0, help, -1);

  // Draw the input line
  wchar_t *input = walloca(width);
  swprintf(input, width + 1, L"%-*ls", width, prompt);
  change_colour(wstat, config.colours.stat_input);
  mvwaddnwstr(wstat, 1, 0, input, -1);
  wmove(wstat, 1, wcslen(prompt));

  wnoutrefresh(wstat);
}

void winddown_tui() {
  if (NULL != wmain)
    delwin(wmain);
  if (NULL != wsbar)
    delwin(wsbar);
  if (NULL != wstat)
    delwin(wstat);

  reset_color_pairs();
  endwin();
  // _nc_freeall();
}
