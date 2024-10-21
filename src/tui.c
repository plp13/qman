// Text user interface (implementation)

#include "lib.h"

//
// Global variables
//

tcap_t tcap;

WINDOW *wmain = NULL;

WINDOW *wsbar = NULL;

WINDOW *wstat = NULL;

WINDOW *wimm = NULL;

action_t action = PA_NULL;

mouse_t mouse_status = MS_EMPTY;

//
// Helper macros and functions
//

// Helper of draw_page(). Set col to the appropriate color for link no. linkno
// of line number lineno. Consider the type of the link, and also query flink to
// check whether it's focused.
#define set_link_col(lineno, linkno, type)                                     \
  if (flink.ok && flink.line == lineno && flink.link == linkno) {              \
    switch (type) {                                                            \
    case LT_MAN:                                                               \
      col = config.colours.link_man_f;                                         \
      break;                                                                   \
    case LT_HTTP:                                                              \
      col = config.colours.link_http_f;                                        \
      break;                                                                   \
    case LT_EMAIL:                                                             \
      col = config.colours.link_email_f;                                       \
      break;                                                                   \
    case LT_LS:                                                                \
    default:                                                                   \
      col = config.colours.link_ls_f;                                          \
    }                                                                          \
  } else {                                                                     \
    switch (type) {                                                            \
    case LT_MAN:                                                               \
      col = config.colours.link_man;                                           \
      break;                                                                   \
    case LT_HTTP:                                                              \
      col = config.colours.link_http;                                          \
      break;                                                                   \
    case LT_EMAIL:                                                             \
      col = config.colours.link_email;                                         \
      break;                                                                   \
    case LT_LS:                                                                \
    default:                                                                   \
      col = config.colours.link_ls;                                            \
    }                                                                          \
  }

// Helper of tui_open(). Re-initialize ncurses after shelling out.
#define tui_reset                                                              \
  {                                                                            \
    winddown_tui();                                                            \
    init_tui();                                                                \
    init_tui_tcap();                                                           \
    init_tui_colours();                                                        \
    init_tui_mouse();                                                          \
    termsize_changed();                                                        \
    init_windows();                                                            \
    populate_page();                                                           \
    if (err)                                                                   \
      winddown(ES_OPER_ERROR, err_msg);                                        \
    if (termsize_changed())                                                    \
      termsize_adjust();                                                       \
    tui_redraw();                                                              \
    doupdate();                                                                \
  }

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

// Helper of ls_jump(), i.e. of tui_open() and tui_toc(). Return the line number
// that best matches local searh target trgt.
int ls_discover(wchar_t *trgt) {
  wchar_t **trgt_words = walloca(BS_SHORT); // words in trgt
  unsigned trgt_words_len;                  // no. of words in trgt
  wchar_t **cand_words = walloca(BS_SHORT); // words in current candidate line
  unsigned cand_words_len;         // no. of words in current candidate line
  unsigned ln;                     // current line number
  unsigned line_nos[BS_LINE];      // candidate line numbers
  unsigned line_scores[BS_LINE];   // candidate line scores
  unsigned line_margends[BS_LINE]; // candidate line left margin endpoints
  unsigned max_no = 0;             // line number with maximum score
  unsigned max_score = 0;          // maximum score
  unsigned i, j, max_i = 0;        // iterators & friends

  trgt_words_len = wsplit(&trgt_words, BS_SHORT, trgt, L".,?!/:;");
  if (0 == trgt_words_len)
    return -1;

  // Record candidate lines and their scores
  j = 0;
  for (ln = 0; ln < page_len && j < BS_LINE; ln++) {
    wchar_t text[BS_LINE]; // current line text
    wcscpy(text, page[ln].text);
    if (wcsstr(text, trgt_words[0]) == &text[wmargend(text, NULL)]) {
      // In order for a line to be a candidate, it must begin with the first
      // word in trgt
      line_nos[j] = ln;
      line_scores[j] = 0;
      cand_words_len = wsplit(&cand_words, BS_SHORT, text, L".,?!/:;");
      for (i = 0; i < MIN(trgt_words_len, cand_words_len); i++)
        if (0 == wcscmp(cand_words[i], trgt_words[i])) {
          line_scores[j]++;
        }
      // Not sure if this improves things; disabling for now
      // if (trgt_words_len == cand_words_len)
      //   line_scores[j]++;
      line_margends[j] = wmargend(page[ln].text, NULL);
      j++;
    }
  }

  // Return the candidate line with the highest score
  for (i = 0; i < j; i++)
    if (line_scores[i] >= max_score)
      if (0 == max_score || line_margends[i] < line_margends[max_i]) {
        max_no = line_nos[i];
        max_score = line_scores[i];
        max_i = i;
      }

  return max_no;
}

// Helper of tui_open() and tui_toc(). Search the current page for a line whose
// text begins with search_term, and jump to said line.
#define ls_jump(trgt)                                                          \
  {                                                                            \
    wchar_t trgt_clone[BS_LINE];                                               \
    wcscpy(trgt_clone, trgt);                                                  \
    int best = ls_discover(trgt_clone);                                        \
    if (best < 0) {                                                            \
      tui_error(L"Unable to jump to requested location");                      \
      return false;                                                            \
    } else {                                                                   \
      page_top = MIN(best, page_len - config.layout.main_height);              \
      const link_loc_t fl = first_link(                                        \
          page, page_len, page_top, page_top + config.layout.main_height - 1); \
      if (fl.ok)                                                               \
        page_flink = fl;                                                       \
    }                                                                          \
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
  while (-1 != pos && ln < lines) {
    res[ln] = pos;
    pos = aprowhat_search(needle, aw_all, aw_all_len, ++pos);
    ln++;
  }
  lines = ln; // lines becomes exact no. of lines to display

  // Display the search results
  const unsigned width =
      config.layout.imm_width_wide - 4; // immediate window width
  wchar_t *tmp = walloca(width - 4);    // temporary
  unsigned ident_len = 0;               // space dedicated to ident column
  for (ln = 0; ln < lines; ln++)
    ident_len = MAX(ident_len, wcslen(aw_all[res[ln]].ident));
  const unsigned descr_len =
      width - ident_len - 5; // space left for descr column
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

// Helper of tui_help(). Draw the help menu into wimm. keys_names contains the
// string representations of key character mappings corresponding to all program
// actions, keys_names_max is the length of the longest string in keys_names,
// top is the first action to print help for, and focus is the action to focus
// on.
void draw_help(const wchar_t *const *keys_names, unsigned keys_names_max,
               unsigned top, unsigned focus) {
  const unsigned width = getmaxx(wimm);  // help window width
  const unsigned height = getmaxy(wimm); // help window height
  const unsigned end = MIN(PA_QUIT,
                           top + height - 6); // last action to print help for
  wchar_t *buf = walloca(width - 2);          // temporary
  unsigned i, j;                              // iterators

  j = 2;
  for (i = top; i <= end; i++) {
    wchar_t glyph;
    if (i == top && i > 1)
      glyph = *config.chars.arrow_up;
    else if (i == end && i < PA_QUIT)
      glyph = *config.chars.arrow_down;
    else
      glyph = L' ';

    swprintf(buf, width - 1, L" %-*ls  %-*.*ls %lc", keys_names_max,
             keys_names[i], width - keys_names_max - 7,
             width - keys_names_max - 7, keys_help[i], glyph);
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

// Helper for tui_history(). Draw the history menu into wimm. history,
// history_cur, and history_top have the same meanings as the history,
// history_cur, and history_top globals respectively. top is the first history
// entry to print, and focus indicates the entry to focus on.
void draw_history(request_t *history, unsigned history_cur,
                  unsigned history_top, unsigned top, unsigned focus) {
  const unsigned width = getmaxx(wimm);  // history window width
  const unsigned height = getmaxy(wimm); // history window height
  const unsigned end = MIN(history_top,
                           top + height - 6); // last entry to print
  wchar_t *buf = walloca(width - 2);          // temporary
  unsigned i, j;                              // iterators

  j = 2;
  for (i = top; i <= end; i++) {
    wchar_t glyph;
    if (i == top && i > 0)
      glyph = *config.chars.arrow_up;
    else if (i == end && i < history_top)
      glyph = *config.chars.arrow_down;
    else
      glyph = L' ';

    swprintf(buf, width - 1, L"%1ls %-7ls  %-*ls %lc",
             i == history_cur ? L"»" : L" ",
             request_type_str(history[i].request_type), width - 15,
             NULL == history[i].args ? L"" : history[i].args, glyph);
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

// Helper for tui_toc(). Draw a table of contents into wimm. toc contains the
// table of contents entries, toc_len is the number of entries in toc, top is
// the first entry to print. and focus the entry to focus on.
void draw_toc(toc_entry_t *toc, unsigned toc_len, unsigned top,
              unsigned focus) {
  const unsigned width = getmaxx(wimm);  // TOC window width
  const unsigned height = getmaxy(wimm); // TOC window height
  const unsigned end = MIN(toc_len - 1,
                           top + height - 6); // last header to print
  wchar_t *buf = walloca(width - 2);          // temporary
  unsigned i, j;                              // iterator

  j = 2;
  for (i = top; i <= end; i++) {
    wchar_t glyph;
    if (i == top && i > 0)
      glyph = *config.chars.arrow_up;
    else if (i == end && i < toc_len - 1)
      glyph = *config.chars.arrow_down;
    else
      glyph = L' ';

    swprintf(buf, width - 1, L"%*ls%-*ls %lc", 2 * toc[i].type, L"",
             width - 4 - 2 * toc[i].type, toc[i].text, glyph);
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
  // Initialize and set up ncurses
  initscr();
  raw();
  keypad(stdscr, true);
  noecho();
  curs_set(0);
  timeout(2000);
  start_color();
}

void init_tui_tcap() {
  tcap.term = getenv("TERM");

  if (-1 == config.tcap.colours) {
    if (has_colors())
      tcap.colours = COLORS;
    else
      tcap.colours = 0;
  } else {
    tcap.colours = config.tcap.colours;
  }

  switch (config.tcap.rgb) {
  case t_true:
    tcap.rgb = true;
    break;
  case t_false:
    tcap.rgb = false;
    break;
  case t_auto:
    tcap.rgb = (tcap.colours >= 256) && can_change_color();
  }

  switch (config.tcap.unicode) {
  case t_true:
    tcap.unicode = true;
    break;
  case t_false:
    tcap.unicode = false;
    break;
  case t_auto:
    tcap.unicode = 0 != strcmp(tcap.term, "linux") && tcap.colours >= 256;
  }

  switch (config.tcap.clipboard) {
  case t_true:
    tcap.clipboard = true;
    break;
  case t_false:
    tcap.clipboard = false;
    break;
  case t_auto:
    tcap.clipboard = 0 == strcmp(tcap.term, "xterm-kitty");
  }
}

void init_tui_colours() {
  // Always initialize fallback color for B&W terminals
  init_colour(config.colours.fallback);

  // Initialize other colors only if the terminal supports color
  if (tcap.colours) {
    init_colour(config.colours.text);
    init_colour(config.colours.search);
    init_colour(config.colours.mark);
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
    // Color pairs used for transitions
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

void init_tui_mouse() {
  if (config.mouse.enable) {
    mousemask(BUTTON1_PRESSED | BUTTON1_RELEASED | BUTTON3_PRESSED |
                  BUTTON3_RELEASED | BUTTON2_PRESSED | BUTTON2_RELEASED |
                  BUTTON4_PRESSED | BUTTON5_PRESSED | REPORT_MOUSE_POSITION,
              NULL);

    // Initialize terminal to enable drag-and-drop
    char *term = getenv("TERM");
    if (0 != strcmp(term, "xterm-1002")) {
      sendescseq("[?1002h");
    }
  }
}

void sendescseq(char *s) {
  putchar('\033');

  unsigned i = 0;
  while ('\0' != s[i])
    putchar(s[i++]);

  fflush(stdout);
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
  const int width = getmaxx(stdscr);
  const int height = getmaxy(stdscr);

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

    if (config.layout.width > 100) {
      config.layout.imm_width_wide = config.layout.width - 20;
      config.layout.imm_width_narrow = 54;
    } else if (config.layout.width > 60) {
      config.layout.imm_width_wide = config.layout.width - 6;
      config.layout.imm_width_narrow = 54;
    } else {
      config.layout.imm_width_wide = config.layout.width - 6;
      config.layout.imm_width_narrow = config.layout.width - 6;
    }

    if (config.layout.height > 18) {
      config.layout.imm_height_long = config.layout.height - 8;
      config.layout.imm_height_short = 6;
    } else {
      config.layout.imm_height_long = config.layout.height - 4;
      config.layout.imm_height_short = config.layout.height - 4;
    }

    return true;
  }

  return false;
}

int cgetch() {
  curs_set(1);
  int ret = getch();
  curs_set(0);

  return ret;
}

wchar_t *ch2name(int k) {
  static wchar_t fkeys[12][4]; // placeholders for function key representations
  static wchar_t okeys[94]
                      [4]; // placeholders for representations of all other keys
  unsigned i;              // iterator

  // Navigation and "special" keys
  switch (k) {
  case KEY_UP:
    return L"UP";
    break;
  case KEY_DOWN:
    return L"DOWN";
    break;
  case KEY_LEFT:
    return L"LEFT";
    break;
  case KEY_RIGHT:
    return L"RIGHT";
    break;
  case KEY_PPAGE:
    return L"PGUP";
    break;
  case KEY_NPAGE:
    return L"PGDN";
    break;
  case KEY_HOME:
    return L"HOME";
    break;
  case KEY_END:
    return L"END";
    break;
  case '\e':
    return L"ESC";
    break;
  case KEY_BREAK:
  case 0x03:
    return L"CTRL-C";
    break;
  case KEY_ENTER:
  case '\n':
    return L"ENTER";
    break;
  case KEY_BACKSPACE:
  case '\b':
    return L"BACKSPACE";
    break;
  case '\t':
    return L"TAB";
    break;
  case ' ':
    return L"SPACE";
    break;
  }

  // F1 to F9
  for (i = 0; i <= 8; i++) {
    if (KEY_F(i + 1) == k) {
      fkeys[i][0] = L'F';
      fkeys[i][1] = i + 48;
      fkeys[i][2] = L'\0';
      return fkeys[i];
    }
  }

  // F10 to F12
  for (i = 9; i <= 11; i++) {
    if (KEY_F(i + 1) == k) {
      fkeys[i][0] = L'F';
      fkeys[i][1] = L'1';
      fkeys[i][2] = i + 38;
      fkeys[i][3] = L'\0';
      return fkeys[i];
    }
  }

  // All other keys
  if (k >= 33 && k <= 126) {
    // Key corresponds to a printable character; return it
    okeys[k - 33][0] = L'\'';
    okeys[k - 33][1] = k;
    okeys[k - 33][2] = L'\'';
    okeys[k - 33][3] = L'\0';
    return okeys[k - 33];
  } else {
    // Key does not correspond to a printable character; return "???"
    return L"???";
  }
}

void termsize_adjust() {
  if (page_top + config.layout.main_height > page_len) {
    page_top = page_len - config.layout.main_height;
    const link_loc_t ll = last_link(page, page_len, page_top,
                                    page_top + config.layout.main_height - 1);
    if (ll.ok)
      page_flink = ll;
  } else {
    const link_loc_t fl = first_link(page, page_len, page_top,
                                     page_top + config.layout.main_height - 1);
    if (fl.ok)
      page_flink = fl;
  }
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
  // Clear screen and reset color
  werase(wmain);
  wbkgd(wmain, COLOR_PAIR(config.colours.text.pair));
  change_colour_attr(wmain, config.colours.text, WA_NORMAL);

  unsigned y;     // current terminal row
  unsigned ly;    // current line
  unsigned x;     // current terminal column
  unsigned lx;    // current column in line
  unsigned l;     // current link
  unsigned s = 0; // current search result

  // For each terminal row...
  for (y = 0; y < getmaxy(wmain); y++) {
    ly = lines_top + y;
    if (ly >= lines_len)
      break;

    // For each terminal column...
    for (x = 0; x < getmaxx(wmain); x++) {
      lx = x + page_left;
      if (lx >= lines[ly].length)
        break;

      // Set text attributes
      if (bget(lines[ly].bold, lx))
        change_colour_attr(wmain, config.colours.text, WA_BOLD);
      if (bget(lines[ly].italic, lx))
        change_colour_attr(wmain, config.colours.text, WA_STANDOUT);
      if (bget(lines[ly].uline, lx))
        change_colour_attr(wmain, config.colours.text, WA_UNDERLINE);
      if (bget(lines[ly].reg, lx))
        change_colour_attr(wmain, config.colours.text, WA_NORMAL);

      // Place character on screen
      mvwaddnwstr(wmain, y, x, &lines[ly].text[lx], 1);
    }

    // For each link...
    for (l = 0; l < lines[ly].links_length; l++) {
      const link_t link = lines[ly].links[l];

      // Apply the the appropriate color, based on link type and whether the
      // link is focused
      colour_t col;
      set_link_col(ly, l, link.type);
      if (page_left <= link.start)
        apply_colour(wmain, y, link.start - page_left, link.end - link.start,
                     col);
    }

    // If we are below the first line, and the previous line has links...
    if (ly > 0 && lines[ly - 1].links_length > 0) {
      l = lines[ly - 1].links_length - 1;

      // ...and its last link is hyphenated...
      if (lines[ly - 1].links[l].in_next) {
        const link_t link = lines[ly - 1].links[l];

        // Apply the the appropriate color, based on link type and whether the
        // link is focused
        colour_t col;
        set_link_col(ly - 1, l, link.type);
        if (page_left <= link.start)
          apply_colour(wmain, y, link.start_next - page_left,
                       link.end_next - link.start_next, col);
      }
    }

    // Skip all search results prior to current line
    while (s < results_len && results[s].line < ly)
      s++;

    // Go through all search results for current line, and highlight them
    while (s < results_len && results[s].line == ly) {
      if (page_left <= results[s].start)
        apply_colour(wmain, y, results[s].start - page_left,
                     results[s].end - results[s].start, config.colours.search);
      s++;
    }

    // If some text is marked, apply the appropriate color to it
    if (mark.enabled) {
      unsigned cx = 0, cn = 0; // x and n parameters for apply_colour()
      if (ly == mark.start_line && ly == mark.end_line) {
        cx = MAX(0, (int)mark.start_char - (int)page_left);
        cn = MAX(0, (int)mark.end_char - (int)mark.start_char + 1);
        if (mark.start_char < page_left)
          cn = MAX(0, (int)cn - (int)(page_left - mark.start_char));
      } else if (ly == mark.start_line && ly < mark.end_line) {
        cx = MAX(0, (int)mark.start_char - (int)page_left);
        cn = getmaxx(wmain) - 1;
      } else if (ly > mark.start_line && ly < mark.end_line) {
        cx = 0;
        cn = getmaxx(wmain) - 1;
      } else if (ly > mark.start_line && ly == mark.end_line) {
        cx = 0;
        cn = MAX(0, (int)mark.end_char - (int)page_left + 1);
      }
      apply_colour(wmain, y, cx, cn, config.colours.mark);
    }
  }

  wnoutrefresh(wmain);
}

void draw_sbar(unsigned lines_len, unsigned lines_top) {
  const unsigned height = getmaxy(wsbar); // scrollbar height
  const unsigned block_pos =
      height > lines_len
          ? 1
          : MIN(height - 2,
                1 + (height - 2) * lines_top /
                        (lines_len - height + 1)); // scrollbar knob position
  unsigned i;                                      // iterator

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

void draw_stat(const wchar_t *mode, const wchar_t *name, unsigned lines_len,
               unsigned lines_pos, const wchar_t *prompt, const wchar_t *help,
               const wchar_t *em) {
  werase(wstat);
  wbkgd(wstat, COLOR_PAIR(config.colours.stat_input_prompt.pair));

  const unsigned width = getmaxx(wstat); // width of both status lines

  // Starting columns and widths of the various sections
  const unsigned mode_col = 0, mode_width = 10, name_col = 10,
                 name_width = width - 28, loc_col = width - 18, loc_width = 18,
                 prompt_col = 0, prompt_width = width / 2,
                 help_col = prompt_width, help_em_width = width - prompt_width;

  wchar_t tmp[BS_LINE], tmp2[BS_LINE];

  // Draw the indicator line
  swprintf(tmp, BS_LINE, L" %-*ls", mode_width - 1, mode);
  change_colour(wstat, config.colours.stat_indic_mode);
  mvwaddnwstr(wstat, 0, mode_col, tmp, mode_width);
  swprintf(tmp, BS_LINE, L" %-*ls", name_width - 1, name);
  change_colour(wstat, config.colours.stat_indic_name);
  mvwaddnwstr(wstat, 0, name_col, tmp, name_width);
  swprintf(tmp2, BS_LINE, L"%d:%d /%d", lines_pos,
           page_left / config.layout.tabstop, lines_len);
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

void draw_imm(bool is_long, bool is_wide, const wchar_t *title,
              const wchar_t *help) {
  unsigned height, width, y, x;

  timeout(-1);

  if (is_long) {
    height = config.layout.imm_height_long;
    y = (config.layout.height - height) / 2;
  } else {
    height = config.layout.imm_height_short;
    y = (config.layout.height - height) / 4;
  }
  if (is_wide) {
    width = config.layout.imm_width_wide;
    x = (config.layout.width - width) / 2;
  } else {
    width = config.layout.imm_width_narrow;
    x = (config.layout.width - width) - 4;
  }

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
  mouse_t ms = MS_EMPTY;   // mouse status corresponding to wget_stat

  if (NULL != trgt) {
    // First call; initialize res, res_len, and pos
    res = trgt;
    res_len = trgt_len;
    pos = 0;
  }

  // Get input from user
  curs_set(1);
  wget_stat = mvwget_wch(w, y, x + pos, (wint_t *)&chr);
  curs_set(0);
  ms = get_mouse_status(chr);

  if (BT_RIGHT == ms.button && ms.up) {
    // User pressed right mouse button; act as if she hit ESC or CTRL-C
    res[0] = L'\0';
    wnoutrefresh(w);
    return 0;
  } else if (BT_LEFT == ms.button && ms.up && config.mouse.left_click_open) {
    // User clicked the left mouse button; act if she hit ENTER (but only if the
    // left_click_open config option is true)
    res[pos] = L'\0';
    wnoutrefresh(w);
    return pos;
  } else if (BT_WHEEL == ms.button && ms.up) {
    // User pressed the wheel button; act as if she hit ENTER
    res[pos] = L'\0';
    wnoutrefresh(w);
    return pos;
  } else {
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

mouse_t get_mouse_status(int chr) {
  mouse_t ret = MS_EMPTY;  // return value
  static bool dnd = false; // set to true when we are in drag-and-drop
  static short dnd_y = -1,
               dnd_x = -1; // cursor position where drag-and-drop was initiated

  if (!config.mouse.enable || !has_mouse()) {
    // If mouse is disabled, always return an empty status
    return ret;
  }

  if (chr == KEY_MOUSE) {
    MEVENT ev;

    if (OK == getmouse(&ev)) {
      // Record cursor position
      ret.y = ev.y;
      ret.x = ev.x;

      // Record button up and down events
      if (ev.bstate & BUTTON1_PRESSED) {
        ret.button = BT_LEFT;
        ret.down = true;
      } else if (ev.bstate & BUTTON1_RELEASED) {
        ret.button = BT_LEFT;
        ret.up = true;
      } else if (ev.bstate & BUTTON3_PRESSED) {
        ret.button = BT_RIGHT;
        ret.down = true;
      } else if (ev.bstate & BUTTON3_RELEASED) {
        ret.button = BT_RIGHT;
        ret.up = true;
      } else if (ev.bstate & BUTTON2_PRESSED) {
        ret.button = BT_WHEEL;
        ret.down = true;
      } else if (ev.bstate & BUTTON2_RELEASED) {
        ret.button = BT_WHEEL;
        ret.up = true;
      }

      // Swap left and right buttons if user is left handed
      if (config.mouse.left_handed) {
        if (BT_LEFT == ret.button)
          ret.button = BT_RIGHT;
        else if (BT_RIGHT == ret.button)
          ret.button = BT_LEFT;
      }

      // Record drag-and-drop
      if (BT_LEFT == ret.button && ret.down) {
        dnd = true;
        dnd_y = ev.y;
        dnd_x = ev.x;
      } else if (dnd) {
        if (!(ev.bstate & REPORT_MOUSE_POSITION)) {
          dnd = false;
          dnd_y = -1;
          dnd_x = -1;
        }
        ret.dnd = dnd;
        ret.dnd_y = dnd_y;
        ret.dnd_x = dnd_x;
      }

      // Record mouse wheel activations
      if (ev.bstate & BUTTON4_PRESSED)
        ret.wheel = WH_UP;
      else if (ev.bstate & BUTTON5_PRESSED)
        ret.wheel = WH_DOWN;
    }
  }

  return ret;
}

void cbeep() {
  if (config.layout.beep)
    beep();
}

void ctbeep() {
  const int width = getmaxx(stdscr);
  const int height = getmaxy(stdscr);

  if (width == config.layout.width && height == config.layout.height)
    cbeep();
}

void editcopy(wchar_t *src) {
  unsigned srcs_len = 3 * wcslen(src); // Length of char* version of src
  char *srcs = salloca(srcs_len);      // char* version of src
  wcstombs(srcs, src, srcs_len);

  size_t src64_len; // Base64-encoded version of src
  char *src64 = base64_encode((unsigned char *)srcs, srcs_len, &src64_len);

  if (tcap.clipboard) {
    // If supported, copy using escape code 52
    char *seq = salloca(7 + strlen(src64));
    sprintf(seq, "]52;c;%s\07", src64);
    sendescseq(seq);
  } else {
    // Fallback: copy using xclip and/or wl-copy
    struct stat sb;
    if (stat("/usr/bin/xclip", &sb) == 0 && sb.st_mode & S_IXUSR) {
      FILE *pp = xpopen("/usr/bin/xclip -i -selection clipboard", "w");
      fprintf(pp, "%s\r", srcs);
      xpclose(pp);
    } else if (stat("/usr/bin/wl-copy", &sb) == 0 && sb.st_mode & S_IXUSR) {
      FILE *pp = xpopen("/usr/bin/wl-copy", "w");
      fputs(srcs, pp);
      xpclose(pp);
    }
  }
}

void winddown_tui() {
  if (NULL != wmain)
    delwin(wmain);
  wmain = NULL;

  if (NULL != wsbar)
    delwin(wsbar);
  wsbar = NULL;

  if (NULL != wstat)
    delwin(wstat);
  wstat = NULL;

  reset_color_pairs();

  // Initialize terminal to disable drag-and-drop
  char *term = getenv("TERM");
  if (0 != strcmp(term, "xterm-1002")) {
    sendescseq("[?1002l");
  }

  endwin();
}

//
// Functions (handlers)
//

void tui_redraw() {
  // Main page
  draw_page(page, page_len, page_top, page_flink);

  // Scrollbar
  draw_sbar(page_len, page_top);

  // Status bar
  unsigned pos = page_top;
  if (page_flink.ok)
    pos = page_flink.line;
  if (pos < page_top || pos >= page_top + config.layout.main_height)
    pos = page_top;
  wchar_t help[BS_SHORT];
  swprintf(help, BS_SHORT, L"Press %ls for help or %ls to quit",
           ch2name(config.keys[PA_HELP][0]), ch2name(config.keys[PA_QUIT][0]));
  draw_stat(request_type_str(history[history_cur].request_type), page_title,
            page_len, pos + 1, L":", help, NULL);
}

void tui_error(wchar_t *em) {
  unsigned pos = page_top;
  if (page_flink.ok)
    pos = page_flink.line;
  if (pos < page_top || pos >= page_top + config.layout.main_height)
    pos = page_top;

  draw_stat(request_type_str(history[history_cur].request_type), page_title,
            page_len, pos + 1, L":", NULL, em);

  cbeep();
}

bool tui_up() {
  const link_loc_t pl =
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
  const link_loc_t nl =
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

bool tui_left() {
  if (0 == page_left) {
    tui_error(L"Already at the leftmost position");
    return false;
  }

  if (page_left < config.layout.tabstop)
    page_left = 0;
  else
    page_left -= config.layout.tabstop;
  return true;
}

bool tui_right() {
  if (page_left + config.layout.width + config.layout.tabstop >= BS_LINE) {
    tui_error(L"Already at the rightmost position");
    return false;
  }

  page_left += config.layout.tabstop;
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
    const link_loc_t fl = first_link(page, page_len, page_top,
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
  const link_loc_t ll = last_link(page, page_len, page_top,
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
    const link_loc_t ll = last_link(page, page_len, page_top,
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
  const link_loc_t fl = first_link(page, page_len, page_top,
                                   page_top + config.layout.main_height - 1);
  if (fl.ok)
    page_flink = fl;

  return true;
}

bool tui_home() {
  // Go to the very top
  page_top = 0;

  // Focus on the first link in the visible portion
  const link_loc_t fl = first_link(page, page_len, page_top,
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
  const link_loc_t ll = last_link(page, page_len, page_top,
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
      page_left = 0;
      page_flink = first_link(page, page_len, page_top,
                              page_top + config.layout.main_height - 1);
    }
    break;
  case LT_HTTP:
    // The link is http(s); open it with the external web browser
    {
      char trgt[BS_SHORT];
      snprintf(trgt, BS_SHORT, "%s '%ls' 2>>/dev/null",
               config.misc.browser_path,
               page[page_flink.line].links[page_flink.link].trgt);

      // Shell out
      xsystem(trgt, true);

      // Re-initialize ncurses (unless using xdg-open)
      if (config.misc.reset_after_http)
        tui_reset;
    }
    break;
  case LT_EMAIL:
    // The link is an e-mail address; open it with the external mailer
    {
      char trgt[BS_SHORT];
      snprintf(trgt, BS_SHORT, "%s '%ls' 2>>/dev/null", config.misc.mailer_path,
               page[page_flink.line].links[page_flink.link].trgt);

      // Shell out
      xsystem(trgt, true);

      // Re-initialize ncurses
      if (config.misc.reset_after_email)
        tui_reset;
    }
    break;
  case LT_LS:
    // The link is a local search link; jump to the appropriate page location
    ls_jump(page[page_flink.line].links[page_flink.link].trgt);
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
      page_left = 0;
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
      page_left = 0;
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
  wchar_t inpt[BS_SHORT - 2] = L""; // string typed by user
  wchar_t trgt[BS_SHORT]; // final string that specifies the page to be opened
  wchar_t help[BS_SHORT]; // help message
  swprintf(help, BS_SHORT, L"%ls: query string   %ls/%ls: abort",
           ch2name(KEY_ENTER), ch2name(KEY_BREAK), ch2name('\e'));
  int got_inpt; // current return value of get_str_next()

  // Draw immediate window and title bar
  if (RT_MAN == rt)
    draw_imm(true, true, L"Manual page to open?", help);
  else if (RT_APROPOS == rt)
    draw_imm(true, true, L"Apropos what?", help);
  else if (RT_WHATIS == rt)
    draw_imm(true, true, L"Whatis what?", help);
  doupdate();

  // Get input (and show quick search results as the user types)
  change_colour(wimm, config.colours.sp_text);
  aw_quick_search(inpt);
  doupdate();
  change_colour(wimm, config.colours.sp_input);
  got_inpt = get_str_next(wimm, 2, 2, inpt,
                          MIN(BS_SHORT - 3, config.layout.imm_width_wide - 4));
  while (got_inpt < 0) {
    // If terminal size has changed, regenerate page and redraw everything
    if (termsize_changed()) {
      del_imm();
      init_windows();
      populate_page();
      if (err)
        winddown(ES_OPER_ERROR, err_msg);
      termsize_adjust();
      tui_redraw();
      if (RT_MAN == rt)
        draw_imm(true, true, L"Manual page to open?", help);
      else if (RT_APROPOS == rt)
        draw_imm(true, true, L"Apropos what?", help);
      else if (RT_WHATIS == rt)
        draw_imm(true, true, L"Whatis what?", help);
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
    page_left = 0;
    page_flink = first_link(page, page_len, page_top,
                            page_top + config.layout.main_height - 1);
    return true;
  } else {
    // User hit ESC or CTRL-C or pressed right mouse button; abort
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
  page_left = 0;
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

bool tui_history() {
  wchar_t help[BS_SHORT]; // help message
  swprintf(help, BS_SHORT, L"%ls/%ls: choose   %ls: jump   %ls/%ls: abort",
           ch2name(config.keys[PA_UP][0]), ch2name(config.keys[PA_DOWN][0]),
           ch2name(config.keys[PA_OPEN][0]), ch2name(KEY_BREAK), ch2name('\e'));
  int hinput;                 // keyboard/mouse input from the user
  mouse_t hms = MS_EMPTY;     // mouse status corresponding to hinput
  action_t haction = PA_NULL; // program action corresponding to hinput
  unsigned height;            // history window height
  unsigned top;               // first history entry to be printed
  int focus = history_cur;    // focused history entry

  // Create the history window, retrieve height, and calculate top
  draw_imm(true, false, L"History", help);
  height = getmaxy(wimm);
  if (focus > height - 6)
    top = focus - height + 6;
  else
    top = 0;

  // Main loop
  while (true) {
    // Draw the history text in the history window
    draw_history(history, history_cur, history_top, top, focus);
    doupdate();

    // Get user input
    hinput = cgetch();
    hms = get_mouse_status(hinput);
    if ('\e' == hinput || KEY_BREAK == hinput || 0x03 == hinput ||
        (BT_RIGHT == hms.button && hms.up)) {
      // User hit ESC or CTRL-C or pressed right mouse button; abort
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
      if (-1 == focus)
        focus = history_top;
      break;
    case PA_DOWN:
      focus++;
      if (history_top + 1 == focus)
        focus = 0;
      break;
    case PA_PGUP:
      focus -= MAX(1, height - 6);
      if (focus < 0)
        focus = history_top;
      break;
    case PA_PGDN:
      focus += MAX(1, height - 6);
      if (focus > history_top)
        focus = 0;
      break;
    case PA_OPEN:
      del_imm();
      if (history_jump(focus))
        populate_page();
      if (err)
        winddown(ES_OPER_ERROR, err_msg);
      return true;
      break;
    case PA_NULL:
    default:
      if (WH_UP == hms.wheel) {
        // When mouse wheel scrolls up, select previous history entry
        focus--;
        if (-1 == focus)
          focus = history_top;
      } else if (WH_DOWN == hms.wheel) {
        // When mouse wheel scrolls down, select next history entry
        focus++;
        if (history_top + 1 == focus)
          focus = 0;
      } else if (BT_LEFT == hms.button && hms.up) {
        // On left button release, focus on the history entry under the cursor
        int iy = hms.y, ix = hms.x;
        unsigned ih = getmaxy(wimm);
        if (wmouse_trafo(wimm, &iy, &ix, false))
          if (iy > 1 && iy < ih - 3 && history_top >= top + iy - 2) {
            focus = top + iy - 2;
            if (config.mouse.left_click_open) {
              // If the left_click_open option is set, go to the appropriate
              // history entry
              del_imm();
              if (history_jump(focus))
                populate_page();
              if (err)
                winddown(ES_OPER_ERROR, err_msg);
              return true;
            }
          }
      } else if (BT_WHEEL == hms.button && hms.up) {
        // On mouse wheel click, go to the appropriate history entry
        del_imm();
        if (history_jump(focus))
          populate_page();
        if (err)
          winddown(ES_OPER_ERROR, err_msg);
        return true;
      }
      break;
    }

    // Adjust top (in case the entire menu won't fit in the immediate window)
    if (focus < top)
      top = focus;
    else if (focus > top + height - 6)
      top = focus - height + 6;

    // If terminal size has changed, regenerate page and redraw everything
    if (termsize_changed()) {
      del_imm();
      init_windows();
      populate_page();
      if (err)
        winddown(ES_OPER_ERROR, err_msg);
      termsize_adjust();
      tui_redraw();
      top = 0;
      focus = 0;
      draw_imm(true, false, L"History", help);
      draw_history(history, history_cur, history_top, top, focus);
      doupdate();
      height = getmaxy(wimm);
    }
  }

  return true;
}

bool tui_toc() {
  wchar_t help[BS_SHORT]; // help message
  swprintf(help, BS_SHORT, L"%ls/%ls: choose   %ls: jump   %ls/%ls: abort",
           ch2name(config.keys[PA_UP][0]), ch2name(config.keys[PA_DOWN][0]),
           ch2name(config.keys[PA_OPEN][0]), ch2name(KEY_BREAK), ch2name('\e'));
  int hinput;                 // keyboard/mouse input from the user
  mouse_t hms = MS_EMPTY;     // mouse status corresponding to hinput
  action_t haction = PA_NULL; // program action corresponding to hinput
  unsigned height;            // TOC window height
  unsigned top = 0;           // first TOC entry to be printed
  int focus = 0;              // focused TOC entry

  // Perform late populateion of toc and toc_len, if necessary
  populate_toc();

  // Create the TOC window, and retrieve height
  draw_imm(true, true, L"Table of Contents", help);
  height = getmaxy(wimm);

  // Main loop
  while (true) {
    // Draw the TOC text in the history window
    draw_toc(toc, toc_len, top, focus);
    doupdate();

    // Get user input
    hinput = cgetch();
    hms = get_mouse_status(hinput);
    if ('\e' == hinput || KEY_BREAK == hinput || 0x03 == hinput ||
        (BT_RIGHT == hms.button && hms.up)) {
      // User hit ESC or CTRL-C or pressed right mouse button; abort
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
      if (-1 == focus)
        focus = toc_len - 1;
      break;
    case PA_DOWN:
      focus++;
      if (toc_len == focus)
        focus = 0;
      break;
    case PA_PGUP:
      focus -= MAX(1, height - 6);
      if (focus < 0)
        focus = toc_len - 1;
      break;
    case PA_PGDN:
      focus += MAX(1, height - 6);
      if (focus >= toc_len)
        focus = 0;
      break;
    case PA_OPEN:
      del_imm();
      ls_jump(toc[focus].text);
      return true;
      break;
    case PA_NULL:
    default:
      if (WH_UP == hms.wheel) {
        // When mouse wheel scrolls up, select previous TOC entry
        focus--;
        if (-1 == focus)
          focus = toc_len - 1;
      } else if (WH_DOWN == hms.wheel) {
        // When mouse wheel scrolls down, select next TOC entry
        focus++;
        if (toc_len == focus)
          focus = 0;
      } else if (BT_LEFT == hms.button && hms.up) {
        // On left button release, focus on the TOC entry under the cursor
        int iy = hms.y, ix = hms.x;
        unsigned ih = getmaxy(wimm);
        if (wmouse_trafo(wimm, &iy, &ix, false))
          if (iy > 1 && iy < ih - 3 && toc_len > top + iy - 2) {
            focus = top + iy - 2;
            if (config.mouse.left_click_open) {
              // If the left_click_open option is set, go to the appropriate
              // TOC entry
              del_imm();
              ls_jump(toc[focus].text);
              return true;
            }
          }
      } else if (BT_WHEEL == hms.button && hms.up) {
        // On mouse wheel click, go to the appropriate TOC entry
        del_imm();
        ls_jump(toc[focus].text);
        return true;
      }
      break;
    }

    // Adjust top (in case the entire menu won't fit in the immediate window)
    if (focus < top)
      top = focus;
    else if (focus > top + height - 6)
      top = focus - height + 6;

    // If terminal size has changed, regenerate page and redraw everything
    if (termsize_changed()) {
      del_imm();
      init_windows();
      populate_page();
      if (err)
        winddown(ES_OPER_ERROR, err_msg);
      populate_toc();
      termsize_adjust();
      tui_redraw();
      top = 0;
      focus = 0;
      draw_imm(true, true, L"History", help);
      draw_toc(toc, toc_len, top, focus);
      doupdate();
      height = getmaxy(wimm);
    }
  }

  return true;
}

bool tui_search(bool back) {
  wchar_t *prompt = back ? L"?" : L"/"; // search prompt
  wchar_t help[BS_SHORT];               // help message
  swprintf(help, BS_SHORT, L"Press %ls to search or %ls/%ls to abort",
           ch2name(KEY_ENTER), ch2name(KEY_BREAK), ch2name('\e'));
  wchar_t inpt[BS_SHORT - 2]; // search string
  wchar_t pout[BS_SHORT];     // search prompt and string printout
  const unsigned width = config.layout.width / 2 - 1; // search string width
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
      termsize_adjust();
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
      results_len = search(&results, inpt, page, page_len, true);
      if (back) {
        const int tmp = search_prev(results, results_len, my_top);
        my_top = -1 == tmp ? my_top : tmp;
      } else {
        const int tmp = search_next(results, results_len, my_top);
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
    const link_loc_t fl = first_link(page, page_len, page_top,
                                     page_top + config.layout.main_height - 1);
    if (fl.ok)
      page_flink = fl;
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
  const link_loc_t fl = first_link(page, page_len, page_top,
                                   page_top + config.layout.main_height - 1);
  if (fl.ok)
    page_flink = fl;
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
  mouse_t hms = MS_EMPTY;     // mouse status corresponding to hinput
  action_t haction = PA_NULL; // program action corresponding to hinput
  unsigned top = 1;           // first action to be printed
  int focus = 1;              // focused action
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
      cur_key_names[j] = NULL;
    }
    keys_names_max = MAX(keys_names_max, wcslen(keys_names[i]));
  }

  // Create the help window, and retrieve its height
  draw_imm(true, true, L"Program Actions and Keyboard Help", help);
  height = getmaxy(wimm);

  // Main loop
  while (true) {
    // Draw the help text in the help window
    draw_help((const wchar_t **)keys_names, keys_names_max, top, focus);
    doupdate();

    // Get user input
    hinput = cgetch();
    hms = get_mouse_status(hinput);
    if ('\e' == hinput || KEY_BREAK == hinput || 0x03 == hinput ||
        (BT_RIGHT == hms.button && hms.up)) {
      // User hit ESC or CTRL-C or pressed right mouse button; abort
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
    case PA_PGUP:
      focus -= MAX(1, height - 6);
      if (focus < 1)
        focus = PA_QUIT;
      break;
    case PA_PGDN:
      focus += MAX(1, height - 6);
      if (focus > PA_QUIT)
        focus = 1;
      break;
    case PA_OPEN:
      del_imm();
      ungetch(config.keys[focus][0]);
      return true;
      break;
    case PA_NULL:
    default:
      if (WH_UP == hms.wheel) {
        // When mouse wheel scrolls up, select previous help entry
        focus--;
        if (0 == focus)
          focus = PA_QUIT;
      } else if (WH_DOWN == hms.wheel) {
        // When mouse wheel scrolls down, select next help entry
        focus++;
        if (PA_QUIT + 1 == focus)
          focus = 1;
      } else if (BT_LEFT == hms.button && hms.up) {
        // On left button release, focus on the help entry under the cursor
        int iy = hms.y, ix = hms.x;
        unsigned ih = getmaxy(wimm);
        if (wmouse_trafo(wimm, &iy, &ix, false))
          if (iy > 1 && iy < ih - 3 && PA_QUIT >= top + iy - 2) {
            focus = top + iy - 2;
            if (config.mouse.left_click_open) {
              // If the left_click_open option is set, execute the entry's
              // program action
              del_imm();
              ungetch(config.keys[focus][0]);
              return true;
            }
          }
      } else if (BT_WHEEL == hms.button && hms.up) {
        // On mouse wheel click, execute the focused entry's program action
        del_imm();
        ungetch(config.keys[focus][0]);
        return true;
      }
      break;
    }

    // Adjust top (in case the entire menu won't fit in the immediate window)
    if (focus < top)
      top = focus;
    else if (focus > top + height - 6)
      top = focus - height + 6;

    // If terminal size has changed, regenerate page and redraw everything
    if (termsize_changed()) {
      del_imm();
      init_windows();
      populate_page();
      if (err)
        winddown(ES_OPER_ERROR, err_msg);
      termsize_adjust();
      tui_redraw();
      top = 1;
      focus = 1;
      draw_imm(true, true, L"Program Actions and Keyboard Help", help);
      draw_help((const wchar_t **)keys_names, keys_names_max, top, focus);
      doupdate();
      height = getmaxy(wimm);
    }
  }

  return true;
}

bool tui_mouse_click(short y, short x) {
  int my = y, mx = x; // locations in wmain that correspond to y and x
  int sy = y, sx = x; // locations in wsbar that correspond to y and x

  // If text was being marked with tui_mouse_dnd(), copy it to clipboard and
  // clear the selection
  if (mark.enabled) {
    wchar_t *mt;
    get_mark(&mt, mark, page, page_len);
    editcopy(mt);
    free(mt);

    mark.enabled = false;

    tui_redraw();
    tui_error(L"Copied to clipboard");
    return false;
  }

  // If the cursor is on a link, make it the focused link
  if (wmouse_trafo(wmain, &my, &mx, false)) {
    unsigned ln = page_top + my; // line number that corresponds to my
    if (ln < page_len) {
      for (unsigned i = 0; i < page[ln].links_length; i++) {
        if (mx >= page[ln].links[i].start && mx < page[ln].links[i].end) {
          page_flink.ok = true;
          page_flink.line = ln;
          page_flink.link = i;

          if (config.mouse.left_click_open && mouse_status.up) {
            // If left_click_open is set, open the link as well
            return tui_open();
          } else
            return true;
        }
      }
    }
  }

  // If the cursor is on the scrollbar, jump to the appropriate page position
  if (wmouse_trafo(wsbar, &sy, &sx, false)) {
    unsigned sh = getmaxy(wsbar); // scrollbar window height
    unsigned bp = (MAX(
        1, MIN(sh - 2, sy))); // where the scrollbar knob should be repositioned

    page_top = bp == 1 ? 0 : (bp * (page_len - sh + 1) - 1) / (sh - 2);
    const link_loc_t fl = first_link(page, page_len, page_top,
                                     page_top + config.layout.main_height - 1);
    if (fl.ok && (page_flink.line != fl.line || page_flink.link != fl.link))
      page_flink = fl;

    return true;
  }

  return false;
}

bool tui_mouse_dnd(short y, short x, short dy, short dx) {
  int my = y, mx = x;     // locations in wmain that correspond to y and x
  int sy = y;             // location in wsbar that corresponds to y
  int mdy = dy, mdx = dx; // locations in wmain that correspond to dy and dx
  int sdy = dy, sdx = dx; // locations in wsbar that correspond to dy and dx

  // If dragging was initiated on the main window, mark text
  if (wmouse_trafo(wmain, &mdy, &mdx, false)) {
    // Make sure my and mx are always within the confines of wmain
    if (my >= getmaxy(wmain)) {
      my = getmaxy(wmain) - 1;
      mx = getmaxx(wmain) - 1;
    }
    if (mx >= getmaxx(wmain))
      mx = getmaxx(wmain) - 1;

    unsigned start_line = MIN(page_len - 1, page_top + mdy);
    unsigned start_char = MIN(page[start_line].length - 1, page_left + mdx);
    unsigned end_line = MIN(page_len - 1, page_top + my);
    unsigned end_char = MIN(page[end_line].length - 1, page_left + mx);

    // If drag was right-to-left and/or bottom-to-top, swap start_char with
    // end_char and/or start_line with end_line as needed
    if (start_line > end_line) {
      swap(start_line, end_line);
      swap(start_char, end_char);
    } else if (start_line == end_line && start_char > end_char) {
      swap(start_char, end_char);
    }

    // Mark text
    mark.enabled = true;
    mark.start_line = start_line;
    mark.start_char = start_char;
    mark.end_line = end_line;
    mark.end_char = end_char;

    return true;
  }

  // If dragging was initiated on the scrollbar, jump to the appropriate page
  // position
  if (wmouse_trafo(wsbar, &sdy, &sdx, false)) {
    unsigned sh = getmaxy(wsbar); // scrollbar window height
    unsigned bp = (MAX(
        1, MIN(sh - 2, sy))); // where the scrollbar knob should be repositioned

    page_top = bp == 1 ? 0 : (bp * (page_len - sh + 1) - 1) / (sh - 2);
    const link_loc_t fl = first_link(page, page_len, page_top,
                                     page_top + config.layout.main_height - 1);
    if (fl.ok && (page_flink.line != fl.line || page_flink.link != fl.link))
      page_flink = fl;

    return true;
  }

  return false;
}

void tui() {
  int input;                // keyboard/mouse input from user
  bool redraw = true;       // set this to true to redraw the screen
  wchar_t errmsg[BS_SHORT]; // error message
  swprintf(errmsg, BS_SHORT, L"Invalid keystroke; press %ls for help",
           ch2name(config.keys[PA_HELP][0]));

  // Initialize TUI
  init_tui();
  configure();
  init_tui_tcap();
  if (-1 == config.tcap.colours || t_auto == config.tcap.rgb ||
      t_auto == config.tcap.unicode || t_auto == config.tcap.clipboard) {
    // Options were defined in the [pcap] configuration section; we must run
    // configure() again, to re-initialize configuration options whose final
    // value might depend on terminal capabilities
    configure();
  }
  init_tui_colours();
  init_tui_mouse();
  termsize_changed();
  init_windows();

  // Initialize page, page_len, ge_title, page_top, page_left, and page_flink
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
      termsize_adjust();
      redraw = true;
    }

    // If redraw is necessary, redraw
    if (redraw) {
      tui_redraw();
      redraw = false;
    }
    doupdate();

    // Get user input
    input = cgetch();
    action = get_action(input);
    mouse_status = get_mouse_status(input);

    // Perform the requested action
    switch (action) {
    case PA_UP:
      redraw = tui_up();
      break;
    case PA_DOWN:
      redraw = tui_down();
      break;
    case PA_LEFT:
      redraw = tui_left();
      break;
    case PA_RIGHT:
      redraw = tui_right();
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
    case PA_HISTORY:
      redraw = tui_history();
      break;
    case PA_TOC:
      redraw = tui_toc();
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
    default:
      if (WH_UP == mouse_status.wheel) {
        // Mouse wheel scroll up causes PA_UP
        redraw = tui_up();
      } else if (WH_DOWN == mouse_status.wheel) {
        // Mouse wheel scroll down causes PA_DOWN
        redraw = tui_down();
      } else if (BT_WHEEL == mouse_status.button && mouse_status.up) {
        // Mouse wheel click causes PA_OPEN
        redraw = tui_open();
      } else if (BT_RIGHT == mouse_status.button && mouse_status.up) {
        // Right mouse button click causes PA_HELP
        redraw = tui_help();
      } else if (BT_LEFT == mouse_status.button && mouse_status.up) {
        // On left mouse button release, call tui_mouse_click()
        redraw = tui_mouse_click(mouse_status.y, mouse_status.x);
      } else if (mouse_status.dnd) {
        // On left mouse drag-and-drop, call tui_mouse_dnd()
        redraw = tui_mouse_dnd(mouse_status.y, mouse_status.x,
                               mouse_status.dnd_y, mouse_status.dnd_x);
      } else
        redraw = true;
      break;
    }
  }
}
