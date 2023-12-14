// Text user interface (implementation)

#include "tui.h"
#include "lib.h"
#include "program.h"

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

// Helper of tui_sp_open(). Print quick search results in wimm as the user
// types.
void aw_quick_search(wchar_t *needle) {
  // Search aw_all for needle and store the results in res;
  unsigned lines =
      config.layout.imm_height_long - 7; // maximum no. of lines to display
  unsigned *res =
      aalloca(lines, unsigned); // search results as positions in aw_all
  unsigned pos = 0;             // current position in aw_all
  unsigned ln = 0;              // current line
  pos = aprowhat_search(needle, aw_all, aw_all_len, pos);
  FILE *fp = fopen("qman.out", "a");
  fwprintf(fp, L"%ls %d\n", needle, pos);
  fclose(fp);
  while (-1 != pos && ln < lines) {
    res[ln] = pos;
    pos = aprowhat_search(needle, aw_all, aw_all_len, ++pos);
    FILE *fp = fopen("qman.out", "a");
    fwprintf(fp, L"%ls %d\n", needle, pos);
    fclose(fp);
    ln++;
  }
  lines = ln; // lines becomes exact no. of lines to display

  // Display the search results
  unsigned width = config.layout.imm_width - 4; // immediate window width
  wchar_t *tmp = walloca(width - 4);            // temporary
  unsigned ident_len = 0; // space dedicated to ident column
  unsigned descr_len;     // space left for descr column
  for (ln = 0; ln < lines; ln++)
    ident_len = MAX(ident_len, wcslen(aw_all[res[ln]].ident));
  descr_len = width - ident_len - 5;
  for (ln = 0; ln < lines; ln++) {
    swprintf(tmp, width - 3, L"%-*ls %-*ls", ident_len, aw_all[res[ln]].ident,
             descr_len, aw_all[res[ln]].descr);
    mvwaddnwstr(wimm, ln + 4, 2, tmp, width - 4);
  }
  for (ln = lines; ln < config.layout.imm_height_long - 7; ln++) {
    swprintf(tmp, width - 3, L"%*ls", width - 4, L"");
    mvwaddnwstr(wimm, ln + 4, 2, tmp, width - 4);
  }

  wnoutrefresh(wimm);
}

// Helper of tui_help(). Return a string representation of key mapping k.
wchar_t *ch2name(int k) {
  wchar_t *ret; // return value
  unsigned i;   // iterator

  // Navigation and "special" keys
  switch (k) {
  case KEY_UP:
    return xwcsdup(L"UP");
    break;
  case KEY_DOWN:
    return xwcsdup(L"DOWN");
    break;
  case KEY_LEFT:
    return xwcsdup(L"LEFT");
    break;
  case KEY_RIGHT:
    return xwcsdup(L"RIGHT");
    break;
  case KEY_PPAGE:
    return xwcsdup(L"PGUP");
    break;
  case KEY_NPAGE:
    return xwcsdup(L"PGDN");
    break;
  case KEY_HOME:
    return xwcsdup(L"HOME");
    break;
  case KEY_END:
    return xwcsdup(L"END");
    break;
  case '\e':
    return xwcsdup(L"ESC");
    break;
  case KEY_BREAK:
  case 0x03:
    return xwcsdup(L"CTRL-C");
    break;
  case KEY_ENTER:
  case '\n':
    return xwcsdup(L"ENTER");
    break;
  case KEY_BACKSPACE:
  case '\b':
    return xwcsdup(L"BACKSPACE");
    break;
  case '\t':
    return xwcsdup(L"TAB");
    break;
  case ' ':
    return xwcsdup(L"SPACE");
    break;
  }

  // F1 to F9
  for (i = 1; i <= 9; i++) {
    if (KEY_F(i) == k) {
      ret = walloc(2);
      ret[0] = L'F';
      ret[1] = i + 48;
      ret[2] = L'\0';
      return ret;
    }
  }

  // F10 to F12
  for (i = 10; i <= 12; i++) {
    if (KEY_F(i) == k) {
      ret = walloc(3);
      ret[0] = L'F';
      ret[1] = L'1';
      ret[2] = i + 38;
      ret[3] = L'\0';
      return ret;
    }
  }

  // All other keys
  ret = walloc(3);
  if (k >= 33 && k <= 126) {
    // Key corresponds to a printable character; return it
    ret[0] = L'\'';
    ret[1] = k;
    ret[2] = L'\'';
  } else {
    // Key does not correspond to a printable character; return "???"
    ret[0] = '?';
    ret[1] = '?';
    ret[2] = '?';
  }
  ret[3] = L'\0';
  return ret;
}

// Helper of tui_help(). Draw the help text into wimm. keys_names contains the
// string representations of key character mappings corresponding to all program
// actions, keys_names_max is the length of the longest string in keys_names,
// top is the first action to print help for, and focus is the action to focus
// on.
void draw_help(wchar_t *const *keys_names, unsigned keys_names_max,
               unsigned top, unsigned focus) {
  unsigned width = getmaxx(wimm);  // help window width
  unsigned height = getmaxy(wimm); // help window height
  unsigned end = MIN(PA_QUIT,
                     top + height - 4); // last action to print help for
  wchar_t *buf = walloca(width - 2);    // temporary
  unsigned i, j;                        // iterator

  j = 2;
  for (i = top; i <= end; i++) {
    swprintf(buf, width - 1, L" %-*ls  %-*.*ls ", keys_names_max, keys_names[i],
             width - keys_names_max - 6, width - keys_names_max - 6,
             keys_help[i]);
    if (i == focus) {
      change_colour(wimm, config.colours.help_text_f);
    } else {
      change_colour(wimm, config.colours.help_text);
    }
    mvwaddnwstr(wimm, j, 1, buf, width - 1);
    j++;
  }

  wmove(wimm, height - 2, width - 2);
  wnoutrefresh(wimm);
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
    init_colour(config.colours.sp_input);
    init_colour(config.colours.sp_text);
    init_colour(config.colours.sp_text_f);
    init_colour(config.colours.help_text);
    init_colour(config.colours.help_text_f);
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
  keypad(wmain, true);

  if (NULL != wsbar)
    delwin(wsbar);
  wsbar = newwin(config.layout.main_height, config.layout.sbar_width, 0,
                 config.layout.main_width);
  keypad(wsbar, true);

  if (NULL != wstat)
    delwin(wstat);
  wstat = newwin(config.layout.stat_height, config.layout.width,
                 config.layout.main_height, 0);
  keypad(wstat, true);

  wnoutrefresh(stdscr);
}

bool termsize_changed() {
  int width = getmaxx(stdscr);
  int height = getmaxy(stdscr);

  if (width != config.layout.width || height != config.layout.height) {
    config.layout.width = width;
    config.layout.height = height;

    if (width > config.layout.sbar_width)
      config.layout.main_width = width - config.layout.sbar_width;
    else
      config.layout.main_width = 0;

    if (height > config.layout.stat_height)
      config.layout.main_height = height - config.layout.stat_height;
    else
      config.layout.main_height = 0;

    if (config.layout.width > 100)
      config.layout.imm_width = config.layout.width - 20;
    else if (config.layout.width > 60)
      config.layout.imm_width = config.layout.width - 6;
    else
      config.layout.imm_width = 40;

    if (config.layout.height > 18)
      config.layout.imm_height_long = config.layout.height - 8;
    else
      config.layout.imm_height_long = 10;

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
  wbkgd(wmain, COLOR_PAIR(config.colours.text.pair));
  change_colour_attr(wmain, config.colours.text, WA_NORMAL);

  unsigned y;     // current terminal row
  unsigned ly;    // current line
  unsigned x;     // current column (in both line and terminal)
  unsigned l;     // current link
  unsigned s = 0; // current search result

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
      if (bget(lines[ly].bold, x))
        change_colour_attr(wmain, config.colours.text, WA_BOLD);
      if (bget(lines[ly].italic, x))
        change_colour_attr(wmain, config.colours.text, WA_STANDOUT);
      if (bget(lines[ly].uline, x))
        change_colour_attr(wmain, config.colours.text, WA_UNDERLINE);
      if (bget(lines[ly].reg, x))
        change_colour_attr(wmain, config.colours.text, WA_NORMAL);

      // Place character on screen
      mvwaddnwstr(wmain, y, x, &lines[ly].text[x], 1);
    }

    // For each link...
    for (l = 0; l < lines[ly].links_length; l++) {
      link_t link = lines[ly].links[l];

      // Choose the the appropriate colour, based on link type and whether the
      // link is focused
      colour_t col;
      if (flink.ok && flink.line == ly && flink.link == l) {
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

    // Skip all search results prior to current line
    while (s < results_len && results[s].line < ly)
      s++;

    // Go through all search results for current line, and highlight them
    while (s < results_len && results[s].line == ly) {
      apply_colour(wmain, y, results[s].start,
                   results[s].end - results[s].start, config.colours.search);
      s++;
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
  wbkgd(wstat, COLOR_PAIR(config.colours.stat_input_prompt.pair));

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

void draw_imm(bool is_long, wchar_t *title, wchar_t *help) {
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

  keypad(wimm, true);
  werase(wimm);
  wbkgd(wimm, COLOR_PAIR(config.colours.help_text.pair));

  change_colour(wimm, config.colours.imm_border);
  draw_box(wimm, 0, 0, height - 1, width - 1);
  change_colour(wimm, config.colours.imm_title);
  wchar_t *tmp = walloca(width - 2);
  swprintf(tmp, width - 1, L" %-*ls", width - 2, title);
  mvwaddnwstr(wimm, 1, 1, tmp, width - 2);
  change_colour(wimm, config.colours.help_text);
  mvwaddnwstr(wimm, height - 2, 2, help, width - 4);

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

int get_str_next(WINDOW *w, unsigned y, unsigned x, wchar_t *trgt,
                 unsigned trgt_len) {
  static wchar_t *res;     // copy of trgt
  static unsigned res_len; // copy of trgt_len
  static unsigned pos;     // position in res/trgt
  int ret;                 // next return value
  int chr = '\0';          // user character input
  int wget_stat;           // mvwget_wch() return value

  if (NULL != trgt) {
    // First call; initialize res, res_len, and pos
    res = trgt;
    res_len = trgt_len;
    pos = 0;
  }

  // Get input from user
  wget_stat = mvwget_wch(w, y, x + pos, (wint_t *)&chr);

  switch (chr) {
  case L'\e':
  case KEY_BREAK:
  case 0x03:
    // User hit ESC or CTRL-C
    res[0] = L'\0';
    ret = 0;
    break;
  case KEY_ENTER:
  case L'\n':
    // User hit ENTER
    res[pos] = L'\0';
    ret = pos;
    break;
  case KEY_BACKSPACE:
  case L'\b':
    // User hit BACKSPACE
    if (pos > 0)
      pos--;
    else
      ctbeep();
    res[pos] = L'\0';
    mvwaddnwstr(w, y, x + pos, L" ", 1);
    ret = -KEY_BACKSPACE;
    break;
  case L'\t':
    // User hit TAB
    ret = -0x09;
    break;
  case KEY_UP:
  case KEY_DOWN:
  case KEY_PPAGE:
  case KEY_NPAGE:
  case KEY_HOME:
  case KEY_END:
    // Key hit UP, DOWN, PGUP, PGDN, HOME, or END
    ret = -chr;
    break;
  default:
    // User typed a character
    if (OK != wget_stat) {
      // Reject function keys, arrow keys, etc.
      ctbeep();
    } else if (pos < res_len) {
      res[pos] = (wchar_t)chr;
      res[pos + 1] = L'\0';
      mvwaddnwstr(w, y, x + pos, &res[pos], 1);
      pos++;
    } else
      ctbeep();
    ret = -chr;
  }

  wnoutrefresh(w);
  return ret;
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

void ctbeep() {
  int width = getmaxx(stdscr);
  int height = getmaxy(stdscr);

  if (width == config.layout.width && height == config.layout.height)
    cbeep();
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
  wchar_t help[BS_SHORT];

  if (pos < page_top || pos >= page_top + config.layout.main_height)
    pos = page_top;

  draw_page(page, page_len, page_top, page_flink);
  draw_sbar(page_len, page_top);
  swprintf(help, BS_SHORT, L"Press %ls for help or %ls to quit",
           ch2name(config.keys[PA_HELP][0]), ch2name(config.keys[PA_QUIT][0]));
  draw_stat(request_type_str(history[history_cur].request_type), page_title,
            page_len, pos + 1, L":", help, NULL);
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
  error_on_invalid_flink;

  // Open the link
  switch (page[page_flink.line].links[page_flink.link].type) {
  case LT_MAN:
    // The link is a manual page; add a new page request to show it
    {
      wchar_t trgt[BS_SHORT];
      swprintf(trgt, BS_SHORT, L"'%ls'",
               page[page_flink.line].links[page_flink.link].trgt);
      history_push(RT_MAN, trgt);
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
    }
    break;
  case LT_HTTP:
    // The link is http(s); open it with the external web browser
    {
      char trgt[BS_SHORT];
      snprintf(trgt, BS_SHORT, "%s '%ls'", config.misc.browser_path,
               page[page_flink.line].links[page_flink.link].trgt);
      system(trgt);
    }
    break;
  case LT_EMAIL:
    // The link is an email address; open it with the external mailer
    {
      char trgt[BS_SHORT];
      snprintf(trgt, BS_SHORT, "%s '%ls'", config.misc.mailer_path,
               page[page_flink.line].links[page_flink.link].trgt);
      system(trgt);
    }
    break;
  case LT_LS:
    // The link is a local search link; jump to the appropriate page location
    {
      result_t *sr;
      unsigned sr_len;
      sr_len = search(&sr, page[page_flink.line].links[page_flink.link].trgt,
                      page, page_len);
      if (0 == sr_len) {
        tui_error(L"Unable to open link");
        return false;
      }
      page_top = MIN(sr[0].line, page_len - config.layout.main_height);
      page_flink = first_link(page, page_len, page_top,
                              page_top + config.layout.main_height - 1);
      free(sr);
    }
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
  wchar_t inpt[BS_SHORT - 2]; // string typed by user
  wchar_t trgt[BS_SHORT]; // final string that specifies the page to be opened
  wchar_t help[BS_SHORT]; // help message
  swprintf(help, BS_SHORT, L"%ls query string (%ls/%ls to abort)",
           ch2name(KEY_ENTER), ch2name(KEY_BREAK), ch2name('\e'));
  int got_inpt; // current return value of get_str_next()

  // Draw immediate window and title bar
  if (RT_MAN == rt)
    draw_imm(true, L"Manual page to open?", help);
  else if (RT_APROPOS == rt)
    draw_imm(true, L"Apropos what?", help);
  else if (RT_WHATIS == rt)
    draw_imm(true, L"Whatis what?", help);
  doupdate();

  // Get input (and show quick search results as the user types)
  change_colour(wimm, config.colours.sp_text);
  aw_quick_search(inpt);
  doupdate();
  change_colour(wimm, config.colours.sp_input);
  got_inpt = get_str_next(wimm, 2, 2, inpt,
                          MIN(BS_SHORT - 3, config.layout.imm_width - 4));
  while (got_inpt < 0) {
    // If terminal size has changed, regenerate page and redraw everything
    if (termsize_changed()) {
      del_imm();
      init_windows();
      populate_page();
      if (err)
        winddown(ES_OPER_ERROR, err_msg);
      tui_redraw();
      if (RT_MAN == rt)
        draw_imm(true, L"Manual page to open?", help);
      else if (RT_APROPOS == rt)
        draw_imm(true, L"Apropos what?", help);
      else if (RT_WHATIS == rt)
        draw_imm(true, L"Whatis what?", help);
      change_colour(wimm, config.colours.sp_input);
      mvwaddnwstr(wimm, 2, 2, inpt, wcslen(inpt));
      aw_quick_search(inpt);
      doupdate();
    }

    change_colour(wimm, config.colours.sp_text);
    aw_quick_search(inpt);
    doupdate();
    change_colour(wimm, config.colours.sp_input);
    got_inpt = get_str_next(wimm, 2, 2, NULL, 0);
  }
  del_imm();

  if (got_inpt > 0) {
    // Input succeeded; show requested page
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
    // User hit ESC or CTRL-C; abort
    tui_redraw();
    tui_error(L"Aborted");
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
}

bool tui_search(bool back) {
  wchar_t *prompt; // search prompt
  if (back)
    prompt = L"?";
  else
    prompt = L"/";
  wchar_t help[BS_SHORT]; // help message
  swprintf(help, BS_SHORT, L"%ls search string (%ls/%ls to abort)",
           ch2name(KEY_ENTER), ch2name(KEY_BREAK), ch2name('\e'));
  wchar_t inpt[BS_SHORT - 2]; // search string
  wchar_t pout[BS_SHORT];     // search prompt and string printout
  unsigned width = config.layout.width / 2 - 1; // search string width
  int got_inpt; // current return value of get_str_next()
  unsigned my_top =
      page_top; // temporary page_top that will be set to the line noumber of
                // the first search result, as the user types

  // Get search string
  swprintf(pout, BS_SHORT, prompt);
  draw_stat(L"SEARCH", page_title, page_len, page_top + 1, pout, help, NULL);
  got_inpt = get_str_next(wstat, 1, 1, inpt, MIN(BS_SHORT - 3, width));

  // As the user types something...
  while (got_inpt < 0) {
    // If terminal size has changed, regenerate page and redraw everything
    if (termsize_changed()) {
      init_windows();
      populate_page();
      if (err)
        winddown(ES_OPER_ERROR, err_msg);
      tui_redraw();
      doupdate();
    }

    // Free previous results
    if (NULL != results && results_len > 0)
      free(results);

    // Populate results and results_len
    if (0 == wcslen(inpt)) {
      // Input is empty; set results to NULL, results_len to 0, and my_top to
      // page_top
      results = NULL;
      results_len = 0;
      my_top = page_top;
    } else {
      // Input is not empty; populate results and results_len from input, and
      // set my_top to the location of the first match
      results_len = search(&results, inpt, page, page_len);
      if (back) {
        int tmp = search_prev(results, results_len, my_top);
        my_top = -1 == tmp ? my_top : tmp;
      } else {
        int tmp = search_next(results, results_len, my_top);
        my_top = -1 == tmp ? my_top : tmp;
      }
      if (my_top + config.layout.main_height > page_len) {
        if (page_len >= config.layout.main_height)
          my_top = MIN(my_top, page_len - config.layout.main_height);
        else
          my_top = 0;
      }
    }

    // Redraw all windows, scrolling over to my_top
    draw_page(page, page_len, my_top, page_flink);
    draw_sbar(page_len, my_top);
    swprintf(pout, BS_SHORT, L"%ls%ls", prompt, inpt);
    if (0 == results_len) {
      draw_stat(L"SEARCH", page_title, page_len, my_top + 1, pout, NULL,
                L"Search string not found");
      cbeep();
    } else {
      draw_stat(L"SEARCH", page_title, page_len, my_top + 1, pout, help, NULL);
    }
    doupdate();

    // Get next user input
    got_inpt = get_str_next(wstat, 1, 1, NULL, 0);
  }

  if (got_inpt > 0) {
    // User entered a string and hit ENTER; retain search results
    page_top = my_top;
    page_flink = first_link(page, page_len, page_top,
                            page_top + config.layout.main_height - 1);
    return true;
  } else {
    // User hit ESC or CTRL-C; clear search results
    if (NULL != results && results_len > 0)
      free(results);
    results = NULL;
    results_len = 0;
    tui_redraw();
    tui_error(L"Aborted");
    return false;
  }
}

bool tui_search_next(bool back) {
  unsigned my_top;

  // Store the previous/next search result into my_top
  if (back)
    my_top = search_prev(results, results_len, MAX(0, page_top - 1));
  else
    my_top = search_next(results, results_len, MIN(page_len - 1, page_top + 1));

  // If result wasn't found, show error message
  if (-1 == my_top) {
    tui_redraw();
    tui_error(L"No more search results");
    return false;
  }

  // Massage my_top to avoid scrolling out of page_len
  if (my_top + config.layout.main_height > page_len) {
    if (page_len >= config.layout.main_height)
      my_top = MIN(my_top, page_len - config.layout.main_height);
    else
      my_top = 0;
  }

  // If result was found, but we have reached the end of page, show error
  // message
  if (page_top == my_top) {
    tui_redraw();
    tui_error(L"No more search results");
    return false;
  }

  // Set page_top and page_flink to jump to the search result
  page_top = my_top;
  page_flink = first_link(page, page_len, page_top,
                          page_top + config.layout.main_height - 1);
  return true;
}

bool tui_help() {
  unsigned i, j, k;                 // iterators
  wchar_t *keys_names[PA_QUIT + 1]; // string representations of key character
                                    // mappings corresponding to all program
                                    // actions (as unified strings)
  unsigned keys_names_max = 0;      // length of longest string in keys_names
  wchar_t *cur_key_names[8] = {
      NULL, NULL, NULL, NULL, NULL,
      NULL, NULL, NULL};  // string representations of key character mappings
                          // corresponding to the current action (as array of
                          // strings)
  wchar_t *tmp;           // temporary
  wchar_t help[BS_SHORT]; // help message
  swprintf(help, BS_SHORT,
           L"%ls/%ls: choose action   %ls: fire   %ls/%ls: abort",
           ch2name(config.keys[PA_UP][0]), ch2name(config.keys[PA_DOWN][0]),
           ch2name(config.keys[PA_OPEN][0]), ch2name(KEY_BREAK), ch2name('\e'));
  int hinput;                 // keyboard/mouse input from the user
  action_t haction = PA_NULL; // program action corresponding to hinput
  unsigned top = 1;           // first action to be printed
  unsigned focus = 1;         // focused action
  unsigned height;            // help window height

  // For each action...
  for (i = 0; i <= PA_QUIT; i++) {
    // Populate cur_key_names
    k = 0;
    for (j = 0; j < 8 && 0 != config.keys[i][j]; j++) {
      tmp = ch2name(config.keys[i][j]);
      if (!in8(tmp, cur_key_names, wcsequal)) {
        cur_key_names[k] = tmp;
        k++;
      }
    }

    // Produce keys_names[i] and update keys_names_max, using cur_key_names
    keys_names[i] = walloca(BS_SHORT);
    wcscpy(keys_names[i], L"");
    for (j = 0; NULL != cur_key_names[j]; j++) {
      if (0 != j)
        wcscat(keys_names[i], L", ");
      wcscat(keys_names[i], cur_key_names[j]);
      free(cur_key_names[j]);
      cur_key_names[j] = NULL;
    }
    keys_names_max = MAX(keys_names_max, wcslen(keys_names[i]));
  }

  // Create the help window, and retrieve its height
  draw_imm(true, L"Program Actions and Keyboard Help", help);
  height = getmaxy(wimm);

  // Main loop
  while (true) {
    // Draw the help text in the help window
    draw_help(keys_names, keys_names_max, top, focus);
    doupdate();

    // Get user input
    hinput = getch();
    if ('\e' == hinput || KEY_BREAK == hinput || 0x03 == hinput) {
      // User hit ESC or CTRL-C; abort
      del_imm();
      tui_error(L"Aborted");
      tui_redraw();
      return false;
    }
    haction = get_action(hinput);

    // Perform the requested action
    switch (haction) {
    case PA_UP:
      focus--;
      if (0 == focus)
        focus = PA_QUIT;
      break;
    case PA_DOWN:
      focus++;
      if (PA_QUIT + 1 == focus)
        focus = 1;
      break;
    case PA_OPEN:
      del_imm();
      ungetch(config.keys[focus][0]);
      return true;
      break;
    case PA_NULL:
    default:
      break;
    }

    // Adjust top (in case the entire menu won't fit in the immediate window)
    if (focus < top)
      top = focus;
    else if (focus > top + height - 5)
      top = focus - height + 5;

    // If terminal size has changed, regenerate page and redraw everything
    if (termsize_changed()) {
      del_imm();
      init_windows();
      populate_page();
      if (err)
        winddown(ES_OPER_ERROR, err_msg);
      tui_redraw();
      draw_imm(true, L"Program Actions and Keyboard Help", help);
      draw_help(keys_names, keys_names_max, top, focus);
      doupdate();
      height = getmaxy(wimm);
    }
  }

  return true;
}

void tui() {
  int input;                // keyboard/mouse input from user
  bool redraw = true;       // set this to true to redraw the screen
  wchar_t errmsg[BS_SHORT]; // error message
  wprintf(errmsg, BS_SHORT, L"Invalid keystroke; press %ls for help",
          ch2name(config.keys[PA_HELP][0]));

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
    case PA_SEARCH:
      redraw = tui_search(false);
      break;
    case PA_SEARCH_BACK:
      redraw = tui_search(true);
      break;
    case PA_SEARCH_NEXT:
      redraw = tui_search_next(false);
      break;
    case PA_SEARCH_PREV:
      redraw = tui_search_next(true);
      break;
    case PA_HELP:
      redraw = tui_help();
      break;
    case PA_QUIT:
      break;
    case PA_NULL:
      redraw = true;
      break;
    default:
      tui_error(errmsg);
      break;
    }
  }
}
