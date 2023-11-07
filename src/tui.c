// Text user interface (implementation)

#include "tui.h"
#include "lib.h"
#include "program.h"
#include <curses.h>

//
// Global variables
//

WINDOW *wmain = NULL;

WINDOW *wsbar = NULL;

WINDOW *wstat = NULL;

WINDOW *wimm = NULL;

action_t action = PA_NULL;

//
// Helper macros and functions
//

// Helper of tui_open(), tui_open_apropos() and tui_open_whatis(). If page_flink
// isn't valid, error out and return false.
#define error_on_invalid_flink                                                 \
  if (!page_flink.ok || page_flink.line < page_top ||                          \
      page_flink.line >= page_top + config.layout.main_height ||               \
      page_flink.line >= page_len ||                                           \
      page_flink.link >= page[page_flink.line].links_length) {                 \
    tui_error(L"Unable to open link");                                         \
    return false;                                                              \
  }

//
// Functions (utility)
//

void init_tui() {
  // Initialize ncurses
  initscr();
  raw();
  keypad(stdscr, true);
  noecho();
  curs_set(1);
  timeout(2000);

  // Initialize colour (if available)
  start_color();
  if (COLOUR) {
    start_color();
    // Initialize colour pairs
    init_colour(config.colours.text);
    init_colour(config.colours.search);
    init_colour(config.colours.link_man);
    init_colour(config.colours.link_man_f);
    init_colour(config.colours.link_http);
    init_colour(config.colours.link_http_f);
    init_colour(config.colours.link_email);
    init_colour(config.colours.link_email_f);
    init_colour(config.colours.link_ls);
    init_colour(config.colours.link_ls_f);
    init_colour(config.colours.sb_line);
    init_colour(config.colours.sb_block);
    init_colour(config.colours.stat_indic_mode);
    init_colour(config.colours.stat_indic_name);
    init_colour(config.colours.stat_indic_loc);
    init_colour(config.colours.stat_input_prompt);
    init_colour(config.colours.stat_input_help);
    init_colour(config.colours.stat_input_em);
    init_colour(config.colours.imm_border);
    init_colour(config.colours.imm_title);
    // Initialize colour pairs used for transitions
    init_pair(config.colours.trans_mode_name, config.colours.stat_indic_mode.bg,
              config.colours.stat_indic_name.bg);
    init_pair(config.colours.trans_name_loc, config.colours.stat_indic_name.bg,
              config.colours.stat_indic_loc.bg);
    init_pair(config.colours.trans_prompt_help,
              config.colours.stat_input_prompt.bg,
              config.colours.stat_input_help.bg);
    init_pair(config.colours.trans_prompt_em,
              config.colours.stat_input_prompt.bg,
              config.colours.stat_input_em.bg);
  }
}

void init_windows() {
  if (NULL != wmain)
    delwin(wmain);
  wmain = newwin(config.layout.main_height, config.layout.main_width, 0, 0);

  if (NULL != wsbar)
    delwin(wsbar);
  wsbar = newwin(config.layout.main_height, config.layout.sbar_width, 0,
                 config.layout.main_width);

  if (NULL != wstat)
    delwin(wstat);
  wstat = newwin(config.layout.stat_height, config.layout.width,
                 config.layout.main_height, 0);

  wnoutrefresh(stdscr);
}

bool termsize_changed() {
  int width = getmaxx(stdscr);
  int height = getmaxy(stdscr);

  if (width != config.layout.width || height != config.layout.height) {
    config.layout.width = width;
    config.layout.height = height;
    config.layout.main_width = width - config.layout.sbar_width;
    config.layout.main_height = height - config.layout.stat_height;
    config.layout.imm_width = config.layout.width - 20;
    config.layout.imm_height_long = config.layout.height - 8;

    return true;
  }

  return false;
}

void draw_box(WINDOW *w, unsigned tl_y, unsigned tl_x, unsigned br_y,
              unsigned br_x) {
  unsigned i;

  mvwaddnwstr(w, tl_y, tl_x, config.chars.box_tl, 1);
  mvwaddnwstr(w, tl_y, br_x, config.chars.box_tr, 1);
  mvwaddnwstr(w, br_y, tl_x, config.chars.box_bl, 1);
  mvwaddnwstr(w, br_y, br_x, config.chars.box_br, 1);
  for (i = tl_y + 1; i < br_y; i++) {
    mvwaddnwstr(w, i, tl_x, config.chars.box_vline, 1);
    mvwaddnwstr(w, i, br_x, config.chars.box_vline, 1);
  }
  for (i = tl_x + 1; i < br_x; i++) {
    mvwaddnwstr(w, tl_y, i, config.chars.box_hline, 1);
    mvwaddnwstr(w, br_y, i, config.chars.box_hline, 1);
  }
}

void draw_page(line_t *lines, unsigned lines_len, unsigned lines_top,
               link_loc_t flink) {
  werase(wmain);
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

      // Choose the the appropriate colour, based on link type and whether the
      // link is focused
      colour_t col;
      if (flink.ok && flink.line == ly && flink.link == i) {
        // Current link is the focused link
        switch (link.type) {
        case LT_MAN:
          col = config.colours.link_man_f;
          break;
        case LT_HTTP:
          col = config.colours.link_http_f;
          break;
        case LT_EMAIL:
          col = config.colours.link_email_f;
          break;
        case LT_LS:
        default:
          col = config.colours.link_ls_f;
        }
      } else {
        // Current link is not the focused link
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
      }

      // Apply said colour
      apply_colour(wmain, y, link.start, link.end - link.start, col);
    }
  }

  wnoutrefresh(wmain);
}

void draw_sbar(unsigned lines_len, unsigned lines_top) {
  unsigned height = getmaxy(wsbar); // scrollbar height
  unsigned block_pos;               // block position
  if (height > lines_len)
    block_pos = 1;
  else
    block_pos = MIN(height - 2,
                    1 + (height - 2) * lines_top / (lines_len - height + 1));
  unsigned i; // iterator

  werase(wsbar);

  // Draw vertical line
  change_colour(wsbar, config.colours.sb_line);
  mvwaddnwstr(wsbar, 0, 0, config.chars.sbar_top, 1);
  for (i = 1; i < height - 1; i++)
    mvwaddnwstr(wsbar, i, 0, config.chars.sbar_vline, 1);
  mvwaddnwstr(wsbar, height - 1, 0, config.chars.sbar_bottom, 1);

  // Draw the drag block
  change_colour(wsbar, config.colours.sb_block);
  mvwaddnwstr(wsbar, block_pos, 0, config.chars.sbar_block, -1);

  wnoutrefresh(wsbar);
}

void draw_stat(wchar_t *mode, wchar_t *name, unsigned lines_len,
               unsigned lines_pos, wchar_t *prompt, wchar_t *help,
               wchar_t *em) {
  werase(wstat);

  unsigned width = getmaxx(wstat); // width of both status lines

  // Starting columns and widths of the various sections
  unsigned mode_col = 0, mode_width = 10, name_col = 10,
           name_width = width - 24, loc_col = width - 14, loc_width = 14,
           prompt_col = 0, prompt_width = width / 2, help_col = prompt_width,
           help_em_width = width - prompt_width;

  wchar_t tmp[BS_LINE], tmp2[BS_LINE];

  // Draw the indicator line
  swprintf(tmp, BS_LINE, L" %-*ls", mode_width - 1, mode);
  change_colour(wstat, config.colours.stat_indic_mode);
  mvwaddnwstr(wstat, 0, mode_col, tmp, mode_width);
  swprintf(tmp, BS_LINE, L" %-*ls", name_width - 1, name);
  change_colour(wstat, config.colours.stat_indic_name);
  mvwaddnwstr(wstat, 0, name_col, tmp, name_width);
  swprintf(tmp2, BS_LINE, L"%d:%d", lines_pos, lines_len);
  swprintf(tmp, BS_LINE, L"%*ls ", loc_width - 1, tmp2);
  change_colour(wstat, config.colours.stat_indic_loc);
  mvwaddnwstr(wstat, 0, loc_col, tmp, loc_width);
  wattr_set(wstat, WA_NORMAL, config.colours.trans_mode_name, NULL);
  mvwaddnwstr(wstat, 0, name_col - 1, config.chars.trans_mode_name, 1);
  wattr_set(wstat, WA_NORMAL, config.colours.trans_name_loc, NULL);
  mvwaddnwstr(wstat, 0, loc_col - 1, config.chars.trans_name_loc, 1);

  // Draw the input line
  if (NULL != help) {
    swprintf(tmp, BS_LINE, L"%*ls ", help_em_width - 1, help);
    change_colour(wstat, config.colours.stat_input_help);
    mvwaddnwstr(wstat, 1, help_col, tmp, help_em_width);
  } else if (NULL != em) {
    swprintf(tmp, BS_LINE, L"%*ls ", help_em_width - 1, em);
    change_colour(wstat, config.colours.stat_input_em);
    mvwaddnwstr(wstat, 1, help_col, tmp, help_em_width);
  }
  swprintf(tmp, BS_LINE, L"%ls", prompt);
  wattr_set(wstat, WA_NORMAL, config.colours.trans_prompt_help, NULL);
  mvwaddnwstr(wstat, 1, help_col - 1, config.chars.trans_prompt_help, 1);
  change_colour(wstat, config.colours.stat_input_prompt);
  mvwaddnwstr(wstat, 1, prompt_col, tmp, prompt_width);

  wnoutrefresh(wstat);
}

void draw_imm(bool is_long, wchar_t *title) {
  unsigned height, width, y, x;

  timeout(-1);

  if (is_long) {
    height = config.layout.imm_height_long;
    y = (config.layout.height - height) / 2;
  } else {
    height = config.layout.imm_height_short;
    y = (config.layout.height - height) / 4;
  }
  width = config.layout.imm_width;
  x = (config.layout.width - width) / 2;

  if (NULL != wimm)
    delwin(wimm);
  wimm = newwin(height, width, y, x);

  werase(wimm);

  change_colour(wimm, config.colours.imm_border);
  draw_box(wimm, 0, 0, height - 1, width - 1);
  change_colour(wimm, config.colours.imm_title);
  wchar_t *tmp = walloca(width - 2);
  swprintf(tmp, width - 1, L" %-*ls", width - 2, title);
  mvwaddnwstr(wimm, 1, 1, tmp, width - 2);

  wnoutrefresh(wimm);
}

void del_imm() {
  if (NULL != wimm)
    delwin(wimm);
  wimm = NULL;
  timeout(2000);
}

bool get_str(WINDOW *w, unsigned y, unsigned x, wchar_t *trgt,
             unsigned trgt_len) {
  int res;

  echo();
  res = mvwgetn_wstr(w, y, x, (wint_t *)trgt, trgt_len);
  noecho();

  if (OK == res)
    return true;
  else
    return false;
}

action_t get_action(int chr) {
  action_t i;
  unsigned j;

  if (ERR == chr)
    return PA_NULL;

  for (i = PA_NULL; i <= PA_QUIT; i++)
    for (j = 0; j < 8; j++)
      if (config.keys[i][j] == chr)
        return i;

  return PA_NULL;
}

void cbeep() {
  if (config.layout.beep)
    beep();
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
}

//
// Functions (handlers)
//

void tui_redraw() {
  unsigned pos = page_flink.line;

  if (pos < page_top || pos >= page_top + config.layout.main_height)
    pos = page_top;

  draw_page(page, page_len, page_top, page_flink);
  draw_sbar(page_len, page_top);
  draw_stat(request_type_str(history[history_cur].request_type), page_title,
            page_len, pos + 1, L":", L"Press 'h' for help or 'q' to quit",
            NULL);
}

void tui_error(wchar_t *em) {
  unsigned pos = page_flink.line;
  if (pos < page_top || pos >= page_top + config.layout.main_height)
    pos = page_top;

  draw_stat(request_type_str(history[history_cur].request_type), page_title,
            page_len, pos + 1, L":", NULL, em);

  cbeep();
}

bool tui_up() {
  link_loc_t pl =
      prev_link(page, page_len, page_flink); // link right before page_flink

  if (pl.ok && pl.line >= page_top &&
      pl.line < page_top + config.layout.main_height) {
    // pl exists and is in visible portion; focus on pl
    page_flink = pl;
  } else if (page_top > 0) {
    // Visible portion isn't already at the beginning of page; scroll up one
    // line
    page_top--;
    if (page[page_top].links_length > 0) {
      // Newly revealed line has links; focus on the last one
      page_flink =
          (link_loc_t){true, page_top, page[page_top].links_length - 1};
    }
  } else {
    // None of the above; error out
    tui_error(L"Already at the top of page");
    return false;
  }

  return true;
}

bool tui_down() {
  link_loc_t nl =
      next_link(page, page_len, page_flink); // link right after page_flink

  if (nl.ok && nl.line >= page_top &&
      nl.line < page_top + config.layout.main_height) {
    // nl exists and is in visible portion; focus on nl
    page_flink = nl;
  } else if (page_top + config.layout.main_height < page_len) {
    // Visible portion isn't at the very end of page; scroll down one line
    page_top++;
    if (page[page_top + config.layout.main_height - 1].links_length > 0) {
      // Newly revealed line has links; focus on the first one
      page_flink =
          (link_loc_t){true, page_top + config.layout.main_height - 1, 0};
    }
  } else {
    // None of the above; error out
    tui_error(L"Already at the bottom of page");
    return false;
  }

  return true;
}

bool tui_pgup() {
  if (page_top >= config.layout.main_height) {
    // If there's space, scroll up one window height
    page_top -= config.layout.main_height;
  } else if (page_top > 0) {
    // If not, but we're still not at the very top, go there
    page_top = 0;
  } else {
    // None of the above; focus on page's first link, or error out if already
    // there
    link_loc_t fl = first_link(page, page_len, page_top,
                               page_top + config.layout.main_height - 1);
    if (fl.ok && (page_flink.line != fl.line || page_flink.link != fl.link)) {
      page_flink = fl;
      return true;
    } else {
      tui_error(L"Already at the top of page");
      return false;
    }
  }

  // Focus on the last link in new visible portion
  link_loc_t ll = last_link(page, page_len, page_top,
                            page_top + config.layout.main_height - 1);
  if (ll.ok)
    page_flink = ll;

  return true;
}

bool tui_pgdn() {
  if (page_top + 2 * config.layout.main_height < page_len) {
    // If there's space, scroll down one window height
    page_top += config.layout.main_height;
  } else if (page_top + config.layout.main_height < page_len &&
             config.layout.main_height <= page_len) {
    // If not, but we're still not at the very bottom, go there
    page_top = page_len - config.layout.main_height;
  } else {
    // None of the above; focus on page's last link, or error out if already
    // there
    link_loc_t ll = last_link(page, page_len, page_top,
                              page_top + config.layout.main_height - 1);
    if (ll.ok && (page_flink.line != ll.line || page_flink.link != ll.link)) {
      page_flink = ll;
      return true;
    } else {
      tui_error(L"Already at the bottom of page");
      return false;
    }
  }

  // Focus on the first link in new visible portion
  link_loc_t fl = first_link(page, page_len, page_top,
                             page_top + config.layout.main_height - 1);
  if (fl.ok)
    page_flink = fl;

  return true;
}

bool tui_home() {
  // Go to the very top
  page_top = 0;

  // Focus on the first link in the visible portion
  link_loc_t fl = first_link(page, page_len, page_top,
                             page_top + config.layout.main_height - 1);
  if (fl.ok)
    page_flink = fl;

  return true;
}

bool tui_end() {
  // Go to the very bottom
  if (config.layout.main_height <= page_len)
    page_top = page_len - config.layout.main_height;

  // Focus on the last link in the visible portion
  link_loc_t ll = last_link(page, page_len, page_top,
                            page_top + config.layout.main_height - 1);
  if (ll.ok)
    page_flink = ll;

  return true;
}

bool tui_open() {
  wchar_t wtrgt[BS_SHORT];
  char strgt[BS_SHORT];

  error_on_invalid_flink;

  // Open the link
  switch (page[page_flink.line].links[page_flink.link].type) {
  case LT_MAN:
    // The link is a manual page; add a new page request to show it
    swprintf(wtrgt, BS_SHORT, L"'%ls'",
             page[page_flink.line].links[page_flink.link].trgt);
    history_push(RT_MAN, wtrgt);
    populate_page();
    if (err) {
      history_back(1);
      history_reset();
      populate_page();
      tui_redraw();
      tui_error(err_msg);
      return false;
    }
    page_top = 0;
    page_flink = first_link(page, page_len, page_top,
                            page_top + config.layout.main_height - 1);
    break;
  case LT_HTTP:
    // The link is http(s); open it with the external web browser
    snprintf(strgt, BS_SHORT, "%s '%ls'", config.misc.browser_path,
             page[page_flink.line].links[page_flink.link].trgt);
    system(strgt);
    break;
  case LT_EMAIL:
    // The link is an email address; open it with the external mailer
    snprintf(strgt, BS_SHORT, "%s '%ls'", config.misc.mailer_path,
             page[page_flink.line].links[page_flink.link].trgt);
    system(strgt);
    break;
  case LT_LS:
    // TBD
    break;
  }

  return true;
}

bool tui_open_apropos() {
  wchar_t wtrgt[BS_LINE];
  wchar_t *wtrgt_stripped, *buf;

  error_on_invalid_flink;

  if (LT_MAN == page[page_flink.line].links[page_flink.link].type) {
    wcscpy(wtrgt, page[page_flink.line].links[page_flink.link].trgt);
    wtrgt_stripped = wcstok(wtrgt, L"()", &buf);

    if (NULL == wtrgt_stripped) {
      tui_error(L"Unable to open link");
      return false;
    } else {
      history_push(RT_APROPOS, wtrgt);
      populate_page();
      if (err) {
        history_back(1);
        history_reset();
        populate_page();
        tui_redraw();
        tui_error(err_msg);
        return false;
      }
      page_top = 0;
      page_flink = first_link(page, page_len, page_top,
                              page_top + config.layout.main_height - 1);
      return true;
    }
  } else {
    tui_error(L"Apropos can only be performed on manual pages");
    return false;
  }
}

bool tui_open_whatis() {
  wchar_t wtrgt[BS_LINE];
  wchar_t *wtrgt_stripped, *buf;

  error_on_invalid_flink;

  if (LT_MAN == page[page_flink.line].links[page_flink.link].type) {
    wcscpy(wtrgt, page[page_flink.line].links[page_flink.link].trgt);
    wtrgt_stripped = wcstok(wtrgt, L"()", &buf);

    if (NULL == wtrgt_stripped) {
      tui_error(L"Unable to open link");
      return false;
    } else {
      history_push(RT_WHATIS, wtrgt);
      populate_page();
      if (err) {
        history_back(1);
        history_reset();
        populate_page();
        tui_redraw();
        tui_error(err_msg);
        return false;
      }
      page_top = 0;
      page_flink = first_link(page, page_len, page_top,
                              page_top + config.layout.main_height - 1);
      return true;
    }
  } else {
    tui_error(L"Whatis can only be performed on manual pages");
    return false;
  }
}

bool tui_sp_open(request_type_t rt) {
  wchar_t inpt[BS_SHORT - 2];
  wchar_t trgt[BS_SHORT];
  bool got_inpt;

  if (RT_MAN == rt)
    draw_imm(false, L"Manual page to open?");
  else if (RT_APROPOS == rt)
    draw_imm(false, L"Apropos what?");
  else if (RT_WHATIS == rt)
    draw_imm(false, L"Whatis what?");
  doupdate();

  change_colour(wimm, config.colours.text);
  got_inpt =
      get_str(wimm, 2, 2, inpt, MIN(BS_SHORT - 3, config.layout.imm_width - 1));
  del_imm();

  if (got_inpt) {
    swprintf(trgt, BS_SHORT, L"'%ls'", inpt);
    history_push(rt, trgt);
    populate_page();
    if (err) {
      history_back(1);
      history_reset();
      populate_page();
      tui_redraw();
      tui_error(err_msg);
      return false;
    }
    page_top = 0;
    page_flink = first_link(page, page_len, page_top,
                            page_top + config.layout.main_height - 1);
    return true;
  } else {
    tui_error(L"Unable to retrieve string");
    return false;
  }
}

bool tui_index() {
  history_push(RT_INDEX, NULL);
  populate_page();
  if (err)
    winddown(ES_OPER_ERROR, err_msg);
  page_top = 0;
  page_flink = first_link(page, page_len, page_top,
                          page_top + config.layout.main_height - 1);

  return true;
}

bool tui_back() {
  if (history_back(1)) {
    populate_page();
    if (err)
      winddown(ES_OPER_ERROR, err_msg);
    return true;
  } else {
    tui_error(L"Already at the first page in history");
    return false;
  }
}

bool tui_fwrd() {
  if (history_forward(1)) {
    populate_page();
    if (err)
      winddown(ES_OPER_ERROR, err_msg);
    return true;
  } else {
    tui_error(L"Already at the last page in history");
    return false;
  }
  cbeep();
}

void tui() {
  int input;          // keyboard/mouse input from user
  bool redraw = true; // set this to true to redraw the screen

  // Initialize TUI
  init_tui();
  termsize_changed();
  init_windows();

  // Initialize page, page_len, page_top, page_left, and page_flink
  populate_page();
  if (err)
    winddown(ES_NOT_FOUND, err_msg);
  page_top = 0;
  page_left = 0;
  page_flink = next_link(page, page_len, page_flink);

  // Initialize action
  action = PA_NULL;

  while (PA_QUIT != action) {
    // If terminal size has changed, regenerate page and ask for a redraw
    if (termsize_changed()) {
      init_windows();
      populate_page();
      if (err)
        winddown(ES_OPER_ERROR, err_msg);
      redraw = true;
    }

    // If redraw is necessary, redraw
    if (redraw) {
      tui_redraw();
      redraw = false;
    }
    doupdate();

    // Get user input
    input = getch();
    action = get_action(input);

    // Perform the requested action
    switch (action) {
    case PA_UP:
      redraw = tui_up();
      break;
    case PA_DOWN:
      redraw = tui_down();
      break;
    case PA_PGUP:
      redraw = tui_pgup();
      break;
    case PA_PGDN:
      redraw = tui_pgdn();
      break;
    case PA_HOME:
      redraw = tui_home();
      break;
    case PA_END:
      redraw = tui_end();
      break;
    case PA_OPEN:
      redraw = tui_open();
      break;
    case PA_OPEN_APROPOS:
      redraw = tui_open_apropos();
      break;
    case PA_OPEN_WHATIS:
      redraw = tui_open_whatis();
      break;
    case PA_SP_OPEN:
      redraw = tui_sp_open(RT_MAN);
      break;
    case PA_SP_APROPOS:
      redraw = tui_sp_open(RT_APROPOS);
      break;
    case PA_SP_WHATIS:
      redraw = tui_sp_open(RT_WHATIS);
      break;
    case PA_INDEX:
      redraw = tui_index();
      break;
    case PA_BACK:
      redraw = tui_back();
      break;
    case PA_FWRD:
      redraw = tui_fwrd();
      break;
    case PA_NULL:
      redraw = true;
    case PA_QUIT:
      break;
    default:
      tui_error(L"Invalid keystroke; press 'h' for help");
      break;
    }
  }
}
