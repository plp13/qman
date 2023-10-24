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
    init_pair(config.colours.stat_indic_mode.pair,
              config.colours.stat_indic_mode.fg,
              config.colours.stat_indic_mode.bg);
    init_pair(config.colours.stat_indic_name.pair,
              config.colours.stat_indic_name.fg,
              config.colours.stat_indic_name.bg);
    init_pair(config.colours.stat_indic_loc.pair,
              config.colours.stat_indic_loc.fg,
              config.colours.stat_indic_loc.bg);
    init_pair(config.colours.stat_input_prompt.pair,
              config.colours.stat_input_prompt.fg,
              config.colours.stat_input_prompt.bg);
    init_pair(config.colours.stat_input_help.pair,
              config.colours.stat_input_help.fg,
              config.colours.stat_input_help.bg);
    init_pair(config.colours.trans_mode_name, config.colours.stat_indic_mode.bg,
              config.colours.stat_indic_name.bg);
    init_pair(config.colours.trans_name_loc, config.colours.stat_indic_name.bg,
              config.colours.stat_indic_loc.bg);
    init_pair(config.colours.trans_prompt_help,
              config.colours.stat_input_prompt.bg,
              config.colours.stat_input_help.bg);
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

void draw_stat(wchar_t *mode, wchar_t *name, unsigned lines_len,
               unsigned lines_pos, wchar_t *prompt, wchar_t *help) {
  wclear(wstat);

  unsigned width = getmaxx(wstat); // width of both status lines

  // Starting columns and widths of the various sections
  unsigned mode_col = 0, mode_width = 10, name_col = 10,
           name_width = width - 24, loc_col = width - 14, loc_width = 14,
           prompt_col = 0, prompt_width = width / 2, help_col = prompt_width,
           help_width = width - prompt_width;

  wchar_t tmp[BS_SHORT], tmp2[BS_SHORT];

  // Draw the indicator line
  swprintf(tmp, BS_SHORT, L" %-*ls", mode_width - 1, mode);
  change_colour(wstat, config.colours.stat_indic_mode);
  mvwaddnwstr(wstat, 0, mode_col, tmp, mode_width);
  swprintf(tmp, BS_SHORT, L" %-*ls", name_width - 1, name);
  change_colour(wstat, config.colours.stat_indic_name);
  mvwaddnwstr(wstat, 0, name_col, tmp, name_width);
  swprintf(tmp2, BS_SHORT, L"%d:%d", lines_len, lines_pos);
  swprintf(tmp, BS_SHORT, L"%*ls ", loc_width - 1, tmp2);
  change_colour(wstat, config.colours.stat_indic_loc);
  mvwaddnwstr(wstat, 0, loc_col, tmp, loc_width);
  wattr_set(wstat, WA_NORMAL, config.colours.trans_mode_name, NULL);
  mvwaddnwstr(wstat, 0, name_col - 1, L"┇", 1);
  wattr_set(wstat, WA_NORMAL, config.colours.trans_name_loc, NULL);
  mvwaddnwstr(wstat, 0, loc_col - 1, L"┇", 1);

  // Draw the input line
  swprintf(tmp, BS_SHORT, L"%*ls ", help_width - 1, help);
  change_colour(wstat, config.colours.stat_input_help);
  mvwaddnwstr(wstat, 1, help_col, tmp, help_width);
  swprintf(tmp, BS_SHORT, L"%ls", prompt);
  wattr_set(wstat, WA_NORMAL, config.colours.trans_prompt_help, NULL);
  mvwaddnwstr(wstat, 1, help_col - 1, L"┇", 1);
  change_colour(wstat, config.colours.stat_input_prompt);
  mvwaddnwstr(wstat, 1, prompt_col, tmp, prompt_width);

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
