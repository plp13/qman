// Program-specific infrastructure (implementation)

#include "program.h"
#include "lib.h"
#include "tui.h"

//
// Global variables
//

option_t options[] = {
    {"index", 'n',
     L"Show all manual pages (default behaviour if no PAGE has been specified)",
     OA_NONE, true},
    {"apropos", 'k',
     L"Search among manual pages and their descriptions for PAGE (apropos)",
     OA_NONE, true},
    {"whatis", 'f', L"Show all pages whose name matches PAGE (whatis)", OA_NONE,
     true},
    {"local-file", 'l', L"Interpret PAGE argument(s) as local filename(s)",
     OA_NONE, true},
    {"cli", 'T', L"Suppress the TUI and output directly to the terminal",
     OA_NONE, true},
    {"config-path", 'C', L"Use ARG as the configuration file path", OA_REQUIRED,
     true},
    {"help", 'h', L"Display this help message", OA_NONE, true},
    {0, 0, 0, 0, false}};

config_t config;

request_t *history = NULL;

unsigned history_cur = 0;

unsigned history_top = 0;

aprowhat_t *aw_all = NULL;

unsigned aw_all_len = 0;

wchar_t **sc_all = NULL;

unsigned sc_all_len = 0;

line_t *page = NULL;

wchar_t page_title[BS_SHORT];

unsigned page_len = 0;

link_loc_t page_flink = {true, 0, 0};

unsigned page_top = 0;

unsigned page_left = 0;

bool err = false;

wchar_t err_msg[BS_LINE];

result_t *results = NULL;

unsigned results_len = 0;

full_regex_t re_man, re_http, re_email;

const wchar_t *keys_help[PA_QUIT + 1] = {
    L"Do nothing",
    L"Scroll up one line",
    L"Scroll down one line",
    L"Scroll up one page",
    L"Scroll down one page",
    L"Go to page top",
    L"Go to page end",
    L"Open focused link",
    L"Perform apropos on focused link",
    L"Perform whatis on focused link",
    L"Open a manual page using a dialog",
    L"Perform apropos on a manual page using a dialog",
    L"Perform whatis on a manual page using a dialog",
    L"Go to index (home) page",
    L"Go back one step in history",
    L"Go forward one step in history",
    L"Search forward",
    L"Search backward",
    L"Go to next search result",
    L"Go to previous search result",
    L"Show this help message",
    L"Exit the program"};

//
// Helper macros and functions
//

// Helper of init() and conf_handler(). Initialize the values of all colour
// pairs used for transitions.
#define set_transition_colours                                                 \
  config.colours.trans_mode_name = 100 * config.colours.stat_indic_mode.pair + \
                                   config.colours.stat_indic_name.pair;        \
  config.colours.trans_name_loc = 100 * config.colours.stat_indic_name.pair +  \
                                  config.colours.stat_indic_loc.pair;          \
  config.colours.trans_prompt_help =                                           \
      100 * config.colours.stat_input_prompt.pair +                            \
      config.colours.stat_input_help.pair;                                     \
  config.colours.trans_prompt_em =                                             \
      100 * config.colours.stat_input_prompt.pair +                            \
      config.colours.stat_input_em.pair;

// Helper of various conf_...() functions and methods. Return true if src is
// equal to 'true', 'yes', or '1'. Return false otherwise.
bool str2bool(const char *src) {
  if (0 == strcasecmp(src, "TRUE") || 0 == strcasecmp(src, "YES") ||
      0 == strcmp(src, "1"))
    return true;
  else
    return false;
}

// Helper of various conf_...() functions and methods. Return the integer value
// held into src as an integer, or INT_MIN in case of error.
int str2int(const char *src) {
  int val, vals_read;
  vals_read = sscanf(src, "%d", &val);
  if (1 != vals_read)
    return INT_MIN;
  else
    return val;
}

// Helper of various conf_...() functions and methods. Return an ncurses colour
// that corresponds to 'src'. Allowed values are 'black', 'red', 'green',
// 'yellow', 'blue', 'magenta', 'cyan', 'white', or '#rrggbb'.
short str2colour(const char *src) {
  int srci = str2int(src);
  if (srci >= 0)
    return srci;
  else if (0 == strcasecmp(src, "BLACK"))
    return COLOR_BLACK;
  else if (0 == strcasecmp(src, "RED"))
    return COLOR_RED;
  else if (0 == strcasecmp(src, "GREEN"))
    return COLOR_GREEN;
  else if (0 == strcasecmp(src, "YELLOW"))
    return COLOR_YELLOW;
  else if (0 == strcasecmp(src, "BLUE"))
    return COLOR_BLUE;
  else if (0 == strcasecmp(src, "MAGENTA"))
    return COLOR_MAGENTA;
  else if (0 == strcasecmp(src, "CYAN"))
    return COLOR_CYAN;
  else if (0 == strcasecmp(src, "WHITE"))
    return COLOR_WHITE;
  else {
    static short ret = 8;
    unsigned red = 0, green = 0, blue = 0;
    if (3 != sscanf(src, "#%2x%2x%2x", &red, &green, &blue)) {
      // Unable to parse colour; return -1
      return -1;
    } else {
      red = (red * 1000) / 256;
      green = (green * 1000) / 256;
      blue = (blue * 1000) / 256;
      init_color(ret, red, green, blue);
      return ret++;
    }
  }
}

int str2ch(const char *src) {
  if (0 == strcasecmp(src, "KEY_UP"))
    return KEY_UP;
  else if (0 == strcasecmp(src, "KEY_DOWN"))
    return KEY_DOWN;
  else if (0 == strcasecmp(src, "KEY_LEFT"))
    return KEY_LEFT;
  else if (0 == strcasecmp(src, "KEY_RIGHT"))
    return KEY_RIGHT;
  else if (0 == strcasecmp(src, "KEY_PPAGE"))
    return KEY_PPAGE;
  else if (0 == strcasecmp(src, "KEY_NPAGE"))
    return KEY_NPAGE;
  else if (0 == strcasecmp(src, "KEY_HOME"))
    return KEY_HOME;
  else if (0 == strcasecmp(src, "KEY_END"))
    return KEY_END;
  else if (0 == strcasecmp(src, "ESC"))
    return '\e';
  else if (0 == strcasecmp(src, "KEY_BREAK"))
    return KEY_BREAK;
  else if (0 == strcasecmp(src, "ETX"))
    return 0x03;
  else if (0 == strcasecmp(src, "KEY_ENTER"))
    return KEY_ENTER;
  else if (0 == strcasecmp(src, "LF"))
    return '\n';
  else if (0 == strcasecmp(src, "KEY_BACKSPACE"))
    return KEY_BACKSPACE;
  else if (0 == strcasecmp(src, "BS"))
    return '\b';
  else if (0 == strcasecmp(src, "HT"))
    return '\t';
  else if (0 == strcasecmp(src, "SPACE"))
    return ' ';
  else if (0 == strcasecmp(src, "F1"))
    return KEY_F(1);
  else if (0 == strcasecmp(src, "F2"))
    return KEY_F(2);
  else if (0 == strcasecmp(src, "F3"))
    return KEY_F(3);
  else if (0 == strcasecmp(src, "F4"))
    return KEY_F(4);
  else if (0 == strcasecmp(src, "F5"))
    return KEY_F(5);
  else if (0 == strcasecmp(src, "F6"))
    return KEY_F(6);
  else if (0 == strcasecmp(src, "F7"))
    return KEY_F(7);
  else if (0 == strcasecmp(src, "F8"))
    return KEY_F(8);
  else if (0 == strcasecmp(src, "F9"))
    return KEY_F(9);
  else if (0 == strcasecmp(src, "F10"))
    return KEY_F(10);
  else if (0 == strcasecmp(src, "F11"))
    return KEY_F(11);
  else if (0 == strcasecmp(src, "F12"))
    return KEY_F(12);
  else if (src[0] >= 33 && src[0] <= 126 && 1 == strlen(src))
    return src[0];
  else
    return -1;
}

// Helper of conf_handler(). It defines a code block that is executed if section
// is equal to s.
#define conf_section(s) if (0 == strcmp(section, s))

// Helper of conf_handler(). Set trgt to the colour that corresponds to value if
// name is equal to n (trgt being a member of config.chars).
#define conf_set_colour(n, trgt)                                               \
  if (0 == strcmp(name, n)) {                                                  \
    char fg[10], bg[10], bold[10];                                             \
    int vals_read;                                                             \
    vals_read = sscanf(value, "%9s%9s%9s", fg, bg, bold);                      \
    if (3 != vals_read) {                                                      \
      swprintf(errmsg, BS_SHORT,                                               \
               L"Config parse error at %s/%s: invalid colour value '%s'",      \
               section, name, value);                                          \
      winddown(ES_CONFIG_ERROR, errmsg);                                       \
    }                                                                          \
    trgt.fg = str2colour(fg);                                                  \
    if (-1 == trgt.fg) {                                                       \
      swprintf(errmsg, BS_SHORT,                                               \
               L"Config parse error at %s/%s: invalid foreground colour '%s'", \
               section, name, fg);                                             \
      winddown(ES_CONFIG_ERROR, errmsg);                                       \
    }                                                                          \
    trgt.bg = str2colour(bg);                                                  \
    if (-1 == trgt.bg) {                                                       \
      swprintf(errmsg, BS_SHORT,                                               \
               L"Config parse error at %s/%s: invalid background colour '%s'", \
               section, name, bg);                                             \
      winddown(ES_CONFIG_ERROR, errmsg);                                       \
    }                                                                          \
    trgt.bold = str2bool(bold);                                                \
    return true;                                                               \
  }

// Helper of conf_handler(). Set trgt to the set of key mappings that
// corresponds to value, if name is equal to n (trgt being a member of
// config.keys).
#define conf_set_key(n, trgt)                                                  \
  if (0 == strcmp(name, n)) {                                                  \
    unsigned i;                                                                \
    char *keys[8];                                                             \
    for (i = 0; i < 8; i++)                                                    \
      keys[i] = walloca(BS_SHORT);                                             \
    int keys_read;                                                             \
    keys_read = sscanf("%s%s%s%s%s%s%s%s", keys[0], keys[1], keys[2], keys[3], \
                       keys[4], keys[5], keys[6], keys[7]);                    \
    if (0 == keys_read) {                                                      \
      swprintf(errmsg, BS_SHORT,                                               \
               L"Config parse error at %s/%s: invalid key mapping value '%s'", \
               section, name, value);                                          \
      winddown(ES_CONFIG_ERROR, errmsg);                                       \
    }                                                                          \
    for (i = 0; i < keys_read; i++) {                                          \
      trgt[i] = str2ch(keys[i]);                                               \
      if (-1 == trgt[i]) {                                                     \
        swprintf(                                                              \
            errmsg, BS_SHORT,                                                  \
            L"Config parse error at %s/%s: key mapping no. %d is invalid",     \
            section, name, value);                                             \
        winddown(ES_CONFIG_ERROR, errmsg);                                     \
      }                                                                        \
    }                                                                          \
    for (i = keys_read; i < 8; i++)                                            \
      trgt[i] = 0;                                                             \
    return true;                                                               \
  }

// Helper of conf_handler(). Set trgt to the boolean stored in value,
// if name is equal to n.
#define conf_set_bool(n, trgt)                                                 \
  if (0 == strcmp(name, n)) {                                                  \
    trgt = str2bool(value);                                                    \
    return true;                                                               \
  }

// Helper of conf_handler(). Set trgt to the integer stored in value,
// if name is equal to n. Use min and max to define boundaries for values, for
// error checking.
#define conf_set_int(n, trgt, min, max)                                        \
  if (0 == strcmp(name, n)) {                                                  \
    int val = str2int(value);                                                  \
    if (INT_MIN == val) {                                                      \
      swprintf(errmsg, BS_SHORT,                                               \
               L"Config parse error at %s/%s: not an integer", section, name); \
      winddown(ES_CONFIG_ERROR, errmsg);                                       \
    }                                                                          \
    if (val < min || val > max) {                                              \
      swprintf(errmsg, BS_SHORT,                                               \
               L"Config parse error at %s/%s: value out of range", section,    \
               name);                                                          \
      winddown(ES_CONFIG_ERROR, errmsg);                                       \
    }                                                                          \
    trgt = val;                                                                \
    return true;                                                               \
  }

// Helper of conf_handler(). Set trgt to a string copy of value, if name is
// equal to n.
#define conf_set_string(n, trgt)                                               \
  if (0 == strcmp(name, n)) {                                                  \
    if (NULL != trgt)                                                          \
      free(trgt);                                                              \
    trgt = strdup(value);                                                      \
    return true;                                                               \
  }

// Helper of conf_handler(). Set trgt to a wide string copy of value, if name is
// equal to n.
#define conf_set_wstring(n, trgt)                                              \
  if (0 == strcmp(name, n)) {                                                  \
    unsigned value_len = strlen(value);                                        \
    if (NULL != trgt)                                                          \
      free(trgt);                                                              \
    trgt = walloc(value_len);                                                  \
    mbstowcs(trgt, value, value_len);                                          \
    return true;                                                               \
  }

// Helper of configure(), used as a handler function for init_parse()
static int conf_handler(void *user, const char *section, const char *name,
                        const char *value) {
  wchar_t errmsg[BS_SHORT]; // error message

  conf_section("chars") {
    conf_set_wstring("sbar_top", config.chars.sbar_top);
    conf_set_wstring("sbar_vline", config.chars.sbar_vline);
    conf_set_wstring("sbar_bottom", config.chars.sbar_bottom);
    conf_set_wstring("sbar_block", config.chars.sbar_block);
    conf_set_wstring("trans_mode_name", config.chars.trans_mode_name);
    conf_set_wstring("trans_name_loc", config.chars.trans_name_loc);
    conf_set_wstring("trans_prompt_help", config.chars.trans_prompt_help);
    conf_set_wstring("trans_prompt_em", config.chars.trans_prompt_em);
    conf_set_wstring("box_hline", config.chars.box_hline);
    conf_set_wstring("box_vline", config.chars.box_vline);
    conf_set_wstring("box_tl", config.chars.box_tl);
    conf_set_wstring("box_tr", config.chars.box_tr);
    conf_set_wstring("box_bl", config.chars.box_bl);
    conf_set_wstring("box_br", config.chars.box_br);

    swprintf(errmsg, BS_SHORT,
             L"Config parse error: no option '%s' exists in section '%s'", name,
             section);
    winddown(ES_CONFIG_ERROR, errmsg);
    return false;
  }

  conf_section("colours") {
    conf_set_colour("text", config.colours.text);
    conf_set_colour("search", config.colours.search);
    conf_set_colour("link_man", config.colours.link_man);
    conf_set_colour("link_man_f", config.colours.link_man_f);
    conf_set_colour("link_http", config.colours.link_http);
    conf_set_colour("link_http_f", config.colours.link_http_f);
    conf_set_colour("link_email", config.colours.link_email);
    conf_set_colour("link_email_f", config.colours.link_email_f);
    conf_set_colour("link_ls", config.colours.link_ls);
    conf_set_colour("link_ls_f", config.colours.link_ls_f);
    conf_set_colour("sb_line", config.colours.sb_line);
    conf_set_colour("sb_block", config.colours.sb_block);
    conf_set_colour("stat_indic_mode", config.colours.stat_indic_mode);
    conf_set_colour("stat_indic_name", config.colours.stat_indic_name);
    conf_set_colour("stat_indic_loc", config.colours.stat_indic_loc);
    conf_set_colour("stat_input_prompt", config.colours.stat_input_prompt);
    conf_set_colour("stat_input_help", config.colours.stat_input_help);
    conf_set_colour("stat_input_em", config.colours.stat_input_em);
    conf_set_colour("imm_border", config.colours.imm_border);
    conf_set_colour("imm_title", config.colours.imm_title);
    conf_set_colour("sp_input", config.colours.sp_input);
    conf_set_colour("sp_text", config.colours.sp_text);
    conf_set_colour("sp_text_f", config.colours.sp_text_f);
    conf_set_colour("help_text", config.colours.help_text);
    conf_set_colour("help_text_f", config.colours.help_text_f);

    swprintf(errmsg, BS_SHORT,
             L"Config parse error: no option '%s' exists in section '%s'", name,
             section);
    winddown(ES_CONFIG_ERROR, errmsg);
    return false;
  }

  conf_section("keys") {
    conf_set_key("up", config.keys[PA_UP]);
    conf_set_key("down", config.keys[PA_DOWN]);
    conf_set_key("pgup", config.keys[PA_PGUP]);
    conf_set_key("pgdn", config.keys[PA_PGDN]);
    conf_set_key("home", config.keys[PA_HOME]);
    conf_set_key("end", config.keys[PA_END]);
    conf_set_key("open", config.keys[PA_OPEN]);
    conf_set_key("open_apropos", config.keys[PA_OPEN_APROPOS]);
    conf_set_key("open_whatis", config.keys[PA_OPEN_WHATIS]);
    conf_set_key("sp_open", config.keys[PA_SP_OPEN]);
    conf_set_key("sp_apropos", config.keys[PA_SP_APROPOS]);
    conf_set_key("sp_whatis", config.keys[PA_SP_WHATIS]);
    conf_set_key("index", config.keys[PA_INDEX]);
    conf_set_key("back", config.keys[PA_BACK]);
    conf_set_key("fwrd", config.keys[PA_FWRD]);
    conf_set_key("search", config.keys[PA_SEARCH]);
    conf_set_key("search_back", config.keys[PA_SEARCH_BACK]);
    conf_set_key("search_next", config.keys[PA_SEARCH_NEXT]);
    conf_set_key("search_prev", config.keys[PA_SEARCH_PREV]);
    conf_set_key("help", config.keys[PA_HELP]);
    conf_set_key("quit", config.keys[PA_QUIT]);

    swprintf(errmsg, BS_SHORT,
             L"Config parse error: no option '%s' exists in section '%s'", name,
             section);
    winddown(ES_CONFIG_ERROR, errmsg);
    return false;
  }

  conf_section("layout") {
    conf_set_bool("sbar", config.layout.sbar);
    conf_set_bool("beep", config.layout.beep);
    conf_set_int("lmargin", config.layout.lmargin, 0, BS_SHORT);
    conf_set_int("rmargin", config.layout.rmargin, 0, BS_SHORT);

    swprintf(errmsg, BS_SHORT,
             L"Config parse error: no option '%s' exists in section '%s'", name,
             section);
    winddown(ES_CONFIG_ERROR, errmsg);
    return false;
  }

  conf_section("misc") {
    conf_set_string("man_path", config.misc.man_path);
    conf_set_string("whatis_path", config.misc.whatis_path);
    conf_set_string("apropos_path", config.misc.apropos_path);
    conf_set_string("browser_path", config.misc.browser_path);
    conf_set_string("mailer_path", config.misc.mailer_path);
    conf_set_int("history_size", config.misc.history_size, 0, 0xfffff);

    swprintf(errmsg, BS_SHORT,
             L"Config parse error: no option '%s' exists in section '%s'", name,
             section);
    winddown(ES_CONFIG_ERROR, errmsg);
    return false;
  }

  swprintf(errmsg, BS_SHORT, L"Config parse error: no such section '%s'",
           section);
  winddown(ES_CONFIG_ERROR, errmsg);
  return false;
}

// Helper of man() and aprowhat_render(). Increase ln, and reallocate res in
// memory, if ln has exceeded its size.
#define inc_ln                                                                 \
  ln++;                                                                        \
  if (ln == res_len) {                                                         \
    res_len += 1024;                                                           \
    res = xreallocarray(res, res_len, sizeof(line_t));                         \
  }

// Helper of search(). Increase i, and reallocate res in memory, if i has
// exceeded its size.
#define inc_i                                                                  \
  i++;                                                                         \
  if (i == res_len) {                                                          \
    res_len += 1024;                                                           \
    res = xreallocarray(res, res_len, sizeof(result_t));                       \
  }

// Helper of man() and aprowhat_render(). Add a link to a line. Allocate memory
// using line_realloc_link() to do so. Use start, end, type, and trgt to
// populate the new link's members.
void add_link(line_t *line, unsigned start, unsigned end, link_type_t type,
              wchar_t *trgt) {
  unsigned trgt_len = wcslen(trgt);

  line_realloc_link((*line), trgt_len);
  line->links[line->links_length - 1].start = start;
  line->links[line->links_length - 1].end = end;
  line->links[line->links_length - 1].type = type;
  wcscpy(line->links[line->links_length - 1].trgt, trgt);
}

// Helper of man(). Discover links that match re in the text of line, and add
// them to said line. type signifies the link type to add.
void discover_links(const full_regex_t *re, line_t *line, link_type_t type) {
  unsigned loff = 0; // offset (in line text) to start searching for links
  range_t lrng = fr_search(re, &line->text[loff]); // location of link in line
  wchar_t trgt[BS_LINE];                           // link target
  wchar_t *sc;        // temporary; holds manual page section of link target
  wchar_t *tmp, *buf; // temporary

  while (lrng.beg != lrng.end) {
    // While a link has been found, add it to the line
    wcsncpy(trgt, &line->text[loff + lrng.beg], lrng.end - lrng.beg);
    trgt[lrng.end - lrng.beg] = L'\0';
    if (LT_MAN == type) {
      // Ugly hack: if type is LT_MAN, check that the link's section is listed
      // in global lc_all before adding it
      tmp = xwcsdup(trgt);
      sc = wcstok(tmp, L"()", &buf);
      if (NULL != sc)
        sc = wcstok(NULL, L"()", &buf);
      if (NULL != sc && wcasememberof(sc_all, sc, sc_all_len))
        add_link(line, loff + lrng.beg, loff + lrng.end, type, trgt);
      free(tmp);
    } else
      add_link(line, loff + lrng.beg, loff + lrng.end, type, trgt);
    loff += lrng.end;
    if (loff < line->length) {
      // Link wasn't at the very end of line; look for another link after it
      lrng = fr_search(re, &line->text[loff]);
    } else {
      // Link was at the very end of line; exit the loop
      lrng.beg = 0;
      lrng.end = 0;
    }
  }
}

// All got_... macros are helpers of man()

// true if tmpw[i] contains a 'bold' terminal escape sequence
#define got_bold                                                               \
  ((i + 4 < len) && (tmpw[i] == L'\e') && (tmpw[i + 1] == L'[') &&             \
   (tmpw[i + 2] == L'1') && (tmpw[i + 3] == L'm'))

// true if tmpw[i] contains a 'not bold' terminal escape sequence
#define got_not_bold                                                           \
  ((i + 4 < len) && (tmpw[i] == L'\e') && (tmpw[i + 1] == L'[') &&             \
   (tmpw[i + 2] == L'0') && (tmpw[i + 3] == L'm'))

// true if tmpw[i] contains a 'italic' terminal escape sequence
#define got_italic                                                             \
  ((i + 4 < len) && (tmpw[i] == L'\e') && (tmpw[i + 1] == L'[') &&             \
   (tmpw[i + 2] == L'3') && (tmpw[i + 3] == L'm'))

// true if tmpw[i] contains a 'not italic' terminal escape sequence
#define got_not_italic                                                         \
  ((i + 5 < len) && (tmpw[i] == L'\e') && (tmpw[i + 1] == L'[') &&             \
   (tmpw[i + 2] == L'2') && (tmpw[i + 3] == L'3') && (tmpw[i + 4] == L'm'))

// true if tmpw[i] contains a 'underline' terminal escape sequence
#define got_uline                                                              \
  ((i + 4 < len) && (tmpw[i] == L'\e') && (tmpw[i + 1] == L'[') &&             \
   (tmpw[i + 2] == L'4') && (tmpw[i + 3] == L'm'))

// true if tmpw[i] contains a 'not underline' terminal escape sequence
#define got_not_uline                                                          \
  ((i + 5 < len) && (tmpw[i] == L'\e') && (tmpw[i + 1] == L'[') &&             \
   (tmpw[i + 2] == L'2') && (tmpw[i + 3] == L'4') && (tmpw[i + 4] == L'm'))

// true if tmpw[i] contains a 'normal / not dim' terminal escape sequence
#define got_normal                                                             \
  ((i + 5 < len) && (tmpw[i] == L'\e') && (tmpw[i + 1] == L'[') &&             \
   (tmpw[i + 2] == L'2') && (tmpw[i + 3] == L'2') && (tmpw[i + 4] == L'm'))

// true if tmpw[i] contains any single-digit terminal formatting sequence
#define got_any_1                                                              \
  ((i + 4 < len) && (tmpw[i] == L'\e') && (tmpw[i + 1] == L'[') &&             \
   (tmpw[i + 3] == L'm'))

// true if tmpw[i] contains any two-digit terminal formatting sequence
#define got_any_2                                                              \
  ((i + 5 < len) && (tmpw[i] == L'\e') && (tmpw[i + 1] == L'[') &&             \
   (tmpw[i + 4] == L'm'))

// true if tmpw[i] contains any single-digit terminal formatting sequence
#define got_any_3                                                              \
  ((i + 6 < len) && (tmpw[i] == L'\e') && (tmpw[i + 1] == L'[') &&             \
   (tmpw[i + 5] == L'm'))

//
// Functions
//

void init() {
  // Use the system locale
  setlocale(LC_ALL, "");

  // Initialize config with sane defaults
  config.chars.sbar_top = wcsdup(L"┬");
  config.chars.sbar_vline = wcsdup(L"│");
  config.chars.sbar_bottom = wcsdup(L"┴");
  config.chars.sbar_block = wcsdup(L"█");
  config.chars.trans_mode_name = wcsdup(L"│");
  config.chars.trans_name_loc = wcsdup(L"│");
  config.chars.trans_prompt_help = wcsdup(L" ");
  config.chars.trans_prompt_em = wcsdup(L" ");
  config.chars.box_hline = wcsdup(L"─");
  config.chars.box_vline = wcsdup(L"│");
  config.chars.box_tl = wcsdup(L"┌");
  config.chars.box_tr = wcsdup(L"┐");
  config.chars.box_bl = wcsdup(L"└");
  config.chars.box_br = wcsdup(L"┘");
  config.colours.text.fg = COLOR_WHITE;
  config.colours.text.bold = false;
  config.colours.text.bg = COLOR_BLACK;
  config.colours.text.pair = 10;
  config.colours.search.fg = COLOR_BLACK;
  config.colours.search.bold = false;
  config.colours.search.bg = COLOR_WHITE;
  config.colours.search.pair = 11;
  config.colours.link_man.fg = COLOR_GREEN;
  config.colours.link_man.bold = false;
  config.colours.link_man.bg = COLOR_BLACK;
  config.colours.link_man.pair = 20;
  config.colours.link_man_f.fg = COLOR_BLACK;
  config.colours.link_man_f.bold = false;
  config.colours.link_man_f.bg = COLOR_GREEN;
  config.colours.link_man_f.pair = 30;
  config.colours.link_http.fg = COLOR_MAGENTA;
  config.colours.link_http.bold = false;
  config.colours.link_http.bg = COLOR_BLACK;
  config.colours.link_http.pair = 21;
  config.colours.link_http_f.fg = COLOR_BLACK;
  config.colours.link_http_f.bold = false;
  config.colours.link_http_f.bg = COLOR_MAGENTA;
  config.colours.link_http_f.pair = 31;
  config.colours.link_email.fg = COLOR_MAGENTA;
  config.colours.link_email.bold = false;
  config.colours.link_email.bg = COLOR_BLACK;
  config.colours.link_email.pair = 22;
  config.colours.link_email_f.fg = COLOR_BLACK;
  config.colours.link_email_f.bold = false;
  config.colours.link_email_f.bg = COLOR_MAGENTA;
  config.colours.link_email_f.pair = 32;
  config.colours.link_ls.fg = COLOR_YELLOW;
  config.colours.link_ls.bold = false;
  config.colours.link_ls.bg = COLOR_BLACK;
  config.colours.link_ls.pair = 23;
  config.colours.link_ls_f.fg = COLOR_BLACK;
  config.colours.link_ls_f.bold = false;
  config.colours.link_ls_f.bg = COLOR_YELLOW;
  config.colours.link_ls_f.pair = 33;
  config.colours.sb_line.fg = COLOR_YELLOW;
  config.colours.sb_line.bold = false;
  config.colours.sb_line.bg = COLOR_BLACK;
  config.colours.sb_line.pair = 40;
  config.colours.sb_block.fg = COLOR_YELLOW;
  config.colours.sb_block.bold = false;
  config.colours.sb_block.bg = COLOR_BLACK;
  config.colours.sb_block.pair = 41;
  config.colours.stat_indic_mode.fg = COLOR_YELLOW;
  config.colours.stat_indic_mode.bold = true;
  config.colours.stat_indic_mode.bg = COLOR_RED;
  config.colours.stat_indic_mode.pair = 50;
  config.colours.stat_indic_name.fg = COLOR_WHITE;
  config.colours.stat_indic_name.bold = true;
  config.colours.stat_indic_name.bg = COLOR_BLUE;
  config.colours.stat_indic_name.pair = 51;
  config.colours.stat_indic_loc.fg = COLOR_BLACK;
  config.colours.stat_indic_loc.bold = false;
  config.colours.stat_indic_loc.bg = COLOR_WHITE;
  config.colours.stat_indic_loc.pair = 52;
  config.colours.stat_input_prompt.fg = COLOR_WHITE;
  config.colours.stat_input_prompt.bold = false;
  config.colours.stat_input_prompt.bg = COLOR_BLACK;
  config.colours.stat_input_prompt.pair = 53;
  config.colours.stat_input_help.fg = COLOR_YELLOW;
  config.colours.stat_input_help.bold = true;
  config.colours.stat_input_help.bg = COLOR_BLACK;
  config.colours.stat_input_help.pair = 54;
  config.colours.stat_input_em.fg = COLOR_RED;
  config.colours.stat_input_em.bold = true;
  config.colours.stat_input_em.bg = COLOR_BLACK;
  config.colours.stat_input_em.pair = 55;
  config.colours.imm_border.fg = COLOR_YELLOW;
  config.colours.imm_border.bold = true;
  config.colours.imm_border.bg = COLOR_BLACK;
  config.colours.imm_border.pair = 60;
  config.colours.imm_title.fg = COLOR_YELLOW;
  config.colours.imm_title.bold = true;
  config.colours.imm_title.bg = COLOR_RED;
  config.colours.imm_title.pair = 61;
  config.colours.sp_input.fg = COLOR_WHITE;
  config.colours.sp_input.bold = true;
  config.colours.sp_input.bg = COLOR_BLACK;
  config.colours.sp_input.pair = 70;
  config.colours.sp_text.fg = COLOR_BLACK;
  config.colours.sp_text.bold = true;
  config.colours.sp_text.bg = COLOR_BLACK;
  config.colours.sp_text.pair = 71;
  config.colours.sp_text_f.fg = COLOR_BLACK;
  config.colours.sp_text_f.bold = false;
  config.colours.sp_text_f.bg = COLOR_MAGENTA;
  config.colours.sp_text_f.pair = 72;
  config.colours.help_text.fg = COLOR_WHITE;
  config.colours.help_text.bold = false;
  config.colours.help_text.bg = COLOR_BLACK;
  config.colours.help_text.pair = 80;
  config.colours.help_text_f.fg = COLOR_BLACK;
  config.colours.help_text_f.bold = false;
  config.colours.help_text_f.bg = COLOR_WHITE;
  config.colours.help_text_f.pair = 81;
  set_transition_colours;
  config.layout.tui = true;
  config.layout.fixedwidth = false;
  config.layout.sbar = true;
  config.layout.beep = true;
  config.layout.width = 80;
  config.layout.height = 25;
  config.layout.sbar_width = 1;
  config.layout.stat_height = 2;
  config.layout.main_width = 79;
  config.layout.main_height = 23;
  config.layout.imm_width = 59;
  config.layout.imm_height_short = 4;
  config.layout.imm_height_long = 15;
  config.layout.lmargin = 2;
  config.layout.rmargin = 2;
  config.misc.program_name = xwcsdup(L"qman");
  config.misc.program_version = xwcsdup(L"qman nightly");
  config.misc.man_path = xstrdup("/usr/bin/man");
  config.misc.whatis_path = xstrdup("/usr/bin/whatis");
  config.misc.apropos_path = xstrdup("/usr/bin/apropos");
  config.misc.browser_path = xstrdup("/usr/bin/xdg-open");
  config.misc.mailer_path = xstrdup("/usr/bin/xdg-email");
  config.misc.config_path = NULL;
  config.misc.history_size = 0xffff;

  // Initialize key characters mappings
  arr8(config.keys[PA_UP], KEY_UP, (int)'y', (int)'k', 0, 0, 0, 0, 0);
  arr8(config.keys[PA_DOWN], KEY_DOWN, (int)'e', (int)'j', 0, 0, 0, 0, 0);
  arr8(config.keys[PA_PGUP], KEY_PPAGE, (int)'b', 0, 0, 0, 0, 0, 0);
  arr8(config.keys[PA_PGDN], KEY_NPAGE, (int)'f', 0, 0, 0, 0, 0, 0);
  arr8(config.keys[PA_HOME], KEY_HOME, (int)'g', 0, 0, 0, 0, 0, 0);
  arr8(config.keys[PA_END], KEY_END, (int)'G', 0, 0, 0, 0, 0, 0);
  arr8(config.keys[PA_OPEN], KEY_ENTER, (int)'\n', (int)'o', 0, 0, 0, 0, 0);
  arr8(config.keys[PA_OPEN_APROPOS], (int)'a', 0, 0, 0, 0, 0, 0, 0);
  arr8(config.keys[PA_OPEN_WHATIS], (int)'w', 0, 0, 0, 0, 0, 0, 0);
  arr8(config.keys[PA_SP_OPEN], (int)'O', 0, 0, 0, 0, 0, 0, 0);
  arr8(config.keys[PA_SP_APROPOS], (int)'A', 0, 0, 0, 0, 0, 0, 0);
  arr8(config.keys[PA_SP_WHATIS], (int)'W', 0, 0, 0, 0, 0, 0, 0);
  arr8(config.keys[PA_INDEX], (int)'i', (int)'I', 0, 0, 0, 0, 0, 0);
  arr8(config.keys[PA_BACK], KEY_BACKSPACE, (int)'\b', (int)'[', 0, 0, 0, 0, 0);
  arr8(config.keys[PA_FWRD], (int)']', 0, 0, 0, 0, 0, 0, 0);
  arr8(config.keys[PA_SEARCH], (int)'/', 0, 0, 0, 0, 0, 0, 0);
  arr8(config.keys[PA_SEARCH_BACK], (int)'?', 0, 0, 0, 0, 0, 0, 0);
  arr8(config.keys[PA_SEARCH_NEXT], (int)'n', 0, 0, 0, 0, 0, 0, 0);
  arr8(config.keys[PA_SEARCH_PREV], (int)'N', 0, 0, 0, 0, 0, 0, 0);
  arr8(config.keys[PA_HELP], (int)'h', (int)'H', 0, 0, 0, 0, 0, 0);
  arr8(config.keys[PA_QUIT], (int)'q', (int)'Q', 0, 0, 0, 0, 0, 0);

  // Initialize history
  history_cur = 0;
  history_top = 0;
  history = aalloc(config.misc.history_size, request_t);
  history_replace(RT_INDEX, NULL);

  // Initialize aw_all and sc_all
  aw_all_len = aprowhat_exec(&aw_all, AW_APROPOS, L"''");
  sc_all_len = aprowhat_sections(&sc_all, aw_all, aw_all_len);

  // Initialize page_title
  wcscpy(page_title, L"");

  // initialize regular expressions
  fr_init(&re_man, "[a-zA-Z0-9\\.:@_-]+\\([a-zA-Z0-9]+\\)");
  fr_init(&re_http, "https?:\\/\\/[a-zA-Z0-9\\.\\/\\?\\+:@_#%=-]+");
  fr_init(
      &re_email,
      "[a-zA-Z0-9\\.\\$\\*\\+\\?\\^\\|!#%&'/=_`{}~-][a-zA-Z0-9\\.\\$\\*\\+\\/"
      "\\?\\^\\|\\.!#%&'=_`{}~-]*@[a-zA-Z0-9-][a-zA-Z0-9-]+\\.[a-zA-Z0-9-][a-"
      "zA-Z0-9\\.-]+");
}

int parse_options(int argc, char *const *argv) {
  // Initialize opstring and longopts arguments of getopt()
  char optstring[3 * asizeof(options)];
  struct option *longopts = aalloc(1 + asizeof(options), struct option);
  unsigned i, optstring_i = 0;
  for (i = 0; i < asizeof(options); i++) {
    optstring[optstring_i] = options[i].short_opt;
    optstring_i++;
    if (options[i].arg != OA_NONE) {
      optstring[optstring_i] = ':';
      optstring_i++;
    }
    if (options[i].arg == OA_OPTIONAL) {
      optstring[optstring_i] = ':';
      optstring_i++;
    }
    longopts[i].name = options[i].long_opt;
    if (options[i].arg == OA_NONE)
      longopts[i].has_arg = no_argument;
    else if (options[i].arg == OA_OPTIONAL)
      longopts[i].has_arg = optional_argument;
    else
      longopts[i].has_arg = required_argument;
    longopts[i].flag = 0;
    longopts[i].val = options[i].short_opt;
  }
  optstring[optstring_i] = '\0';
  longopts[i] = (struct option){0, 0, 0, 0};

  // Parse the options and modify config and history
  while (true) {
    int cur_i;
    char cur = getopt_long(argc, argv, optstring, longopts, &cur_i);
    switch (cur) {
    case -1:
      free(longopts);
      return optind;
      break;
    case 'n':
      // -n or --index was passed; show index page
      history_replace(RT_INDEX, NULL);
      break;
    case 'k':
      // -k or --apropos was passed; try to show apropos results
      history_replace(RT_APROPOS, NULL);
      break;
    case 'f':
      // -f or --whatis was passed; try to show whatis results
      history_replace(RT_WHATIS, NULL);
      break;
    case 'l':
      // -l or --local-file was passed; try to show a man page from a local file
      history_replace(RT_MAN_LOCAL, NULL);
      break;
    case 'T':
      // -T or --cli was passed; do not launch the TUI
      config.layout.tui = false;
      break;
    case 'C':
      // -C or --config-path was passed; read from a different config file
      if (NULL != config.misc.config_path)
        free(config.misc.config_path);
      config.misc.config_path = xstrdup(optarg);
      break;
    case 'h':
      // -h or --help was passed; print usage and exit
      usage();
      winddown(ES_SUCCESS, NULL);
      break;
    case '?':
      // an unknown option was passed; error out
      free(longopts);
      winddown(ES_USAGE_ERROR, L"Unable to parse program arguments");
      break;
    }
  }
}

void parse_args(int argc, char *const *argv) {
  unsigned i;                           // iterator
  wchar_t tmp[BS_LINE], tmp2[BS_SHORT]; // temporary
  unsigned tmp_len; // length of tmp (used to guard against buffer overflows)

  // If the user has specified at least one argument, we should show a manual
  // page rather than the index page; set the request type of
  // history[history_cur] to RT_MAN
  if (RT_INDEX == history[history_cur].request_type && argc >= 1)
    history_replace(RT_MAN, NULL);

  // If we are showing a manual, apropos, or whatis page...
  if (RT_INDEX != history[history_cur].request_type) {
    // But the user hasn't specified an argument...
    if (0 == argc) {
      // Exit with error message
      switch (history[history_top].request_type) {
      case RT_MAN:
      case RT_MAN_LOCAL:
        winddown(ES_USAGE_ERROR, L"What manual page do you want?");
        break;
      case RT_APROPOS:
        winddown(ES_USAGE_ERROR, L"Apropos what?");
        break;
      case RT_WHATIS:
      default:
        winddown(ES_USAGE_ERROR, L"Whatis what?");
        break;
      }
    }

    // Surround all members of argv with single quotes, and flatten them into
    // the tmp string
    tmp_len = 0;
    wcscpy(tmp, L"");
    for (i = 0; i < argc; i++) {
      swprintf(tmp2, BS_SHORT, L"'%s'", argv[i]);
      if (tmp_len + wcslen(tmp2) < BS_LINE) {
        wcscat(tmp, tmp2);
        tmp_len += wcslen(tmp2);
      }
      if (i < argc - 1 && tmp_len + 1 < BS_LINE) {
        wcscat(tmp, L" ");
        tmp_len++;
      }
    }

    // Set history[history_cur].args to tmp
    history_replace(history[history_cur].request_type, tmp);
  }
}

void usage() {
  // Header
  wprintf(L"Usage: %s [OPTION...] [SECTION] [PAGE]...\n\n",
          config.misc.program_name);

  // Command-line options
  unsigned i = 0;
  wchar_t short_opt_str[BS_SHORT];
  wchar_t long_opt_str[BS_SHORT];
  wchar_t help_text_str[BS_LINE];
  wchar_t tmp_str[BS_LINE];
  do {
    // Short option
    swprintf(short_opt_str, BS_SHORT, L"-%c", options[i].short_opt);
    // long option
    if (options[i].arg == OA_NONE)
      swprintf(long_opt_str, BS_SHORT, L"--%s", options[i].long_opt);
    else if (options[i].arg == OA_OPTIONAL)
      swprintf(long_opt_str, BS_SHORT, L"--%s=[ARG]", options[i].long_opt);
    else
      swprintf(long_opt_str, BS_SHORT, L"--%s=ARG", options[i].long_opt);
    // Help text
    wcscpy(tmp_str, options[i].help_text);
    wwrap(tmp_str, 52);
    wcrepl(help_text_str, tmp_str, L'\n', L"\n                           ");
    wprintf(L"  %ls, %-20ls %ls\n", short_opt_str, long_opt_str, help_text_str);
    i++;
  } while (options[i]._cont);

  // Footer
  wprintf(L"\nMandatory or optional arguments to long options are also "
          L"mandatory or optional\nfor any corresponding short options.\n");
}

void configure() {
  FILE *fp;                 // config file
  wchar_t errmsg[BS_SHORT]; // error message

  // Find and open the config file
  if (NULL != config.misc.config_path) {
    // -C option was used; try to open specified config file, and fail if not
    // possible
    fp = fopen(config.misc.config_path, "r");
    if (NULL == fp) {
      swprintf(errmsg, BS_SHORT, L"Unable to open config file '%s'",
               config.misc.config_path);
      winddown(ES_CONFIG_ERROR, errmsg);
    }
  } else {
    // No -C option; try to open specified config file at the standard locations
    char *home = getenv("HOME");
    unsigned hc_len = strlen(home) + 19;
    char *hc = salloca(hc_len);
    snprintf(hc, hc_len, "%s/.config/qman.conf", home);
    fp = fopen(hc, "r");
    if (NULL == fp)
      fp = fopen("/etc/xdg/qman.conf", "r");
  }

  // If we have a config file, process it
  if (NULL != fp) {
    ini_parse_file(fp, conf_handler, &config);
    fclose(fp);
  }
  set_transition_colours;
}

void history_replace(request_type_t rt, wchar_t *args) {
  history[history_cur].request_type = rt;

  if (NULL != history[history_cur].args)
    free(history[history_cur].args);

  if (NULL == args)
    history[history_cur].args = NULL;
  else {
    history[history_cur].args = wcsdup(args);
  }

  history[history_cur].top = 0;
  history[history_cur].flink = (link_loc_t){false, 0, 0};
}

void history_push(request_type_t rt, wchar_t *args) {
  unsigned i;

  // Save user's position
  history[history_cur].top = page_top;
  history[history_cur].flink = page_flink;

  // Increase history_cur and history_top as needed
  history_cur++;
  if (history_top < history_cur)
    // If we're pushing at the top of the history stack, history_top becomes
    // equal to history_cur
    history_top = history_cur;
  else if (history_top > history_cur)
    // If we're pushing in the middle of the history stack, all subsequent
    // history entries are lost, and we must free any memory used by their args
    for (i = history_cur + 1; i <= history_top; i++)
      if (NULL != history[i].args) {
        free(history[i].args);
        history[i].args = NULL;
      }

  // Failsafe: in the unlikely case history_top exceeds history size, free all
  // memory used by history and start over
  if (history_top >= config.misc.history_size) {
    requests_free(history, config.misc.history_size);
    history_top = 0;
    history_cur = 0;
    history = aalloc(config.misc.history_size, request_t);
  }

  // Populate the new history entry
  history_replace(rt, args);
}

bool history_back(unsigned n) {
  history[history_cur].top = page_top;
  history[history_cur].flink = page_flink;

  int pos = history_cur - n;

  if (pos >= 0) {
    history_cur = pos;
    page_top = history[history_cur].top;
    page_flink = history[history_cur].flink;
    return true;
  }

  return false;
}

bool history_forward(unsigned n) {
  history[history_cur].top = page_top;
  history[history_cur].flink = page_flink;

  int pos = history_cur + n;

  if (pos <= history_top) {
    history_cur = pos;
    page_top = history[history_cur].top;
    page_flink = history[history_cur].flink;
    return true;
  }

  return false;
}

void history_reset() {
  unsigned i;

  for (i = history_cur + 1; i <= history_top; i++) {
    if (NULL != history[i].args) {
      free(history[i].args);
      history[i].args = NULL;
    }
  }

  history_top = history_cur;
}

unsigned aprowhat_exec(aprowhat_t **dst, aprowhat_cmd_t cmd,
                       const wchar_t *args) {
  // Prepare apropos/whatis command
  char cmdstr[BS_SHORT];
  if (AW_WHATIS == cmd)
    sprintf(cmdstr, "%s -l %ls 2>>/dev/null", config.misc.whatis_path, args);
  else
    sprintf(cmdstr, "%s -l %ls 2>>/dev/null", config.misc.apropos_path, args);

  // Execute apropos, and enter its result into a temporary file. lines becomes
  // the total number of lines copied.
  FILE *pp = xpopen(cmdstr, "r");
  FILE *fp = xtmpfile();
  unsigned lines = scopylines(pp, fp);
  xpclose(pp);
  rewind(fp);

  // Result
  aprowhat_t *res = aalloc(lines, aprowhat_t);

  char line[BS_LINE];     // current line of text, as returned by the command
  char page[BS_SHORT];    // current manual page
  char section[BS_SHORT]; // current section
  char *word;             // used by strtok() to compile descr
  char descr[BS_LINE];    // current page description

  unsigned page_len, section_len, descr_len, i;

  // For each line returned by apropos/whatis...
  for (i = 0; i < lines; i++) {
    sreadline(line, BS_LINE, fp);

    // Extract page, section, and descr, together with their lengths
    strcpy(page, strtok(line, " ("));
    page_len = strlen(page);
    strcpy(section, strtok(NULL, " ()"));
    section_len = strlen(section);
    word = strtok(NULL, " )-");
    descr[0] = '\0';
    while (NULL != word) {
      if ('\0' != descr[0])
        strcat(descr, " ");
      strcat(descr, word);
      word = strtok(NULL, " ");
    }
    descr_len = strlen(descr);

    // Populate the i'th element of res (allocating when necessary)
    res[i].page = walloc(page_len);
    mbstowcs(res[i].page, page, page_len);
    res[i].section = walloc(section_len);
    mbstowcs(res[i].section, section, section_len);
    res[i].ident = walloc(page_len + section_len + 3);
    swprintf(res[i].ident, page_len + section_len + 3, L"%s(%s)", page,
             section);
    res[i].descr = walloc(descr_len);
    mbstowcs(res[i].descr, descr, descr_len);
  }

  xfclose(fp);

  // If no results were returned by apropos/whatis, set err and err_msg
  err = false;
  if (0 == lines) {
    err = true;
    if (AW_WHATIS == cmd)
      swprintf(err_msg, BS_LINE, L"Whatis %ls: nothing apropriate", args);
    else
      swprintf(err_msg, BS_LINE, L"Apropos %ls: nothing apropriate", args);
  }

  *dst = res;
  return lines;
}

unsigned aprowhat_sections(wchar_t ***dst, const aprowhat_t *aw,
                           unsigned aw_len) {
  unsigned i;

  wchar_t **res = aalloc(BS_SHORT, wchar_t *);
  unsigned res_i = 0;

  for (i = 0; i < aw_len && res_i < BS_SHORT; i++) {
    if (!wmemberof(res, aw[i].section, res_i)) {
      res[res_i] = wcsdup(aw[i].section);
      res_i++;
    }
  }

  wsort(res, res_i, false);

  *dst = res;
  return res_i;
}

unsigned aprowhat_render(line_t **dst, const aprowhat_t *aw, unsigned aw_len,
                         wchar_t *const *sc, unsigned sc_len,
                         const wchar_t *key, const wchar_t *title,
                         const wchar_t *ver, const wchar_t *date) {

  // Text blocks widths
  unsigned line_width = MAX(60, config.layout.main_width);
  unsigned lmargin_width = config.layout.lmargin; // left margin
  unsigned rmargin_width = config.layout.rmargin; // right margin
  unsigned text_width =
      line_width - lmargin_width - rmargin_width; // main text area
  unsigned hfc_width =
      text_width / 2 + text_width % 2; // header/footer centre area
  unsigned hfl_width = (text_width - hfc_width) / 2; // header/footer left area
  unsigned hfr_width =
      hfl_width + (text_width - hfc_width) % 2; // header/footer right area

  unsigned ln = 0;      // current line number
  unsigned i, j;        // iterators
  wchar_t tmp[BS_LINE]; // temporary

  unsigned res_len = 1024;               // result buffer length
  line_t *res = aalloc(res_len, line_t); // result buffer

  // Header
  line_alloc(res[ln], 0);
  inc_ln;
  line_alloc(res[ln], line_width);
  unsigned title_len = wcslen(title); // title length
  unsigned key_len = wcslen(key);     // key length
  unsigned lts_len =
      (hfc_width - title_len) / 2 +
      (hfc_width - title_len) % 2; // length of space on the left of title
  unsigned rts_len =
      (hfc_width - title_len) / 2; // length of space on the right of title
  swprintf(res[ln].text, line_width + 1, L"%*s%-*ls%*s%ls%*s%*ls%*s", //
           lmargin_width, "",                                         //
           hfl_width, key,                                            //
           lts_len, "",                                               //
           title,                                                     //
           rts_len, "",                                               //
           hfr_width, key,                                            //
           rmargin_width, ""                                          //
  );
  bset(res[ln].uline, lmargin_width);
  bset(res[ln].reg, lmargin_width + key_len);
  bset(res[ln].uline,
       lmargin_width + hfl_width + hfc_width + hfr_width - key_len);
  bset(res[ln].reg, lmargin_width + hfl_width + hfc_width + hfr_width);

  // Newline
  inc_ln;
  line_alloc(res[ln], 0);

  // Section title for sections
  inc_ln;
  line_alloc(res[ln], line_width);
  wcscpy(tmp, L"SECTIONS");
  swprintf(res[ln].text, line_width + 1, L"%*s%-*ls", //
           lmargin_width, "",                         //
           text_width, tmp);
  bset(res[ln].bold, lmargin_width);
  bset(res[ln].reg, lmargin_width + wcslen(tmp));

  // Sections
  unsigned sc_maxwidth = wmaxlen(sc, sc_len); // length of longest section
  unsigned sc_cols =
      text_width / (4 + sc_maxwidth);   // number of columns for sections
  unsigned sc_lines = sc_len / sc_cols; // number of lines for sections
  unsigned sc_i;                        // index of current section
  if (sc_len % sc_cols > 0)
    sc_lines++;
  for (i = 0; i < sc_lines; i++) {
    inc_ln;
    line_alloc(res[ln], line_width + 4); // +4 for section margin
    swprintf(res[ln].text, line_width + 1, L"%*s", lmargin_width, "");
    for (j = 0; j < sc_cols; j++) {
      sc_i = sc_cols * i + j;
      if (sc_i < sc_len) {
        swprintf(tmp, sc_maxwidth + 5, L"%-*ls", sc_maxwidth + 4, sc[sc_i]);
        wcscat(res[ln].text, tmp);
        swprintf(tmp, BS_LINE, L"MANUAL PAGES IN SECTION '%ls'", sc[sc_i]);
        add_link(&res[ln],                                                 //
                 lmargin_width + j * (sc_maxwidth + 4),                    //
                 lmargin_width + j * (sc_maxwidth + 4) + wcslen(sc[sc_i]), //
                 LT_LS,                                                    //
                 tmp);
      }
    }
  }

  // For each section...
  for (i = 0; i < sc_len; i++) {
    // Newline
    inc_ln;
    line_alloc(res[ln], 0);
    // Section title
    inc_ln;
    line_alloc(res[ln], line_width);
    swprintf(tmp, text_width + 1, L"MANUAL PAGES IN SECTION '%ls'", sc[i]);
    swprintf(res[ln].text, line_width + 1, L"%*s%-*ls", //
             lmargin_width, "",                         //
             text_width, tmp);
    bset(res[ln].bold, lmargin_width);
    bset(res[ln].reg, lmargin_width + wcslen(tmp));
    // For each manual page...
    for (j = 0; j < aw_len; j++) {
      // If manual page is in current section...
      if (0 == wcscmp(aw[j].section, sc[i])) {
        unsigned lc_width = text_width / 3;        // left column width
        unsigned rc_width = text_width - lc_width; // right column width
        unsigned page_width = wcslen(aw[j].page) + wcslen(aw[j].section) +
                              2; // width of manual page name and section
        unsigned spcl_width =
            MAX(line_width,
                lmargin_width + page_width +
                    rmargin_width); // used in place of line_width; might be
                                    // longer, in which case we'll scroll

        // Page name and section (ident)
        inc_ln;
        line_alloc(res[ln], spcl_width);
        swprintf(res[ln].text, spcl_width + 1, L"%*s%-*ls", //
                 lmargin_width, "",                         //
                 lc_width, aw[j].ident);
        add_link(&res[ln], lmargin_width, lmargin_width + wcslen(aw[j].ident),
                 LT_MAN, aw[j].ident);

        // Description
        wcscpy(tmp, aw[j].descr);
        wwrap(tmp, rc_width);
        wchar_t *buf;
        wchar_t *ptr = wcstok(tmp, L"\n", &buf);
        if (NULL != ptr && page_width < lc_width) {
          wcscat(res[ln].text, ptr);
          ptr = wcstok(NULL, L"\n", &buf);
        }
        while (NULL != ptr) {
          inc_ln;
          line_alloc(res[ln], line_width);
          swprintf(res[ln].text, line_width + 1, L"%*s%ls", //
                   lmargin_width + lc_width, "",            //
                   ptr);
          ptr = wcstok(NULL, L"\n", &buf);
        }
      }
    }
  }

  // Newline
  inc_ln;
  line_alloc(res[ln], 0);

  // Footer
  inc_ln;
  line_alloc(res[ln], line_width);
  unsigned date_len = wcslen(date); // date length
  unsigned lds_len =
      (hfc_width - date_len) / 2 +
      (hfc_width - date_len) % 2; // length of space on the left of date
  unsigned rds_len =
      (hfc_width - date_len) / 2; // length of space on the right of date
  swprintf(res[ln].text, line_width + 1, L"%*s%-*ls%*s%ls%*s%*ls%*s", //
           lmargin_width, "",                                         //
           hfl_width, ver,                                            //
           lds_len, "",                                               //
           date,                                                      //
           rds_len, "",                                               //
           hfr_width, key,                                            //
           rmargin_width, ""                                          //
  );
  bset(res[ln].uline,
       lmargin_width + hfl_width + hfc_width + hfr_width - key_len);
  bset(res[ln].reg, lmargin_width + hfl_width + hfc_width + hfr_width);

  *dst = res;
  return ln + 1;
}

int aprowhat_search(const wchar_t *needle, const aprowhat_t *hayst,
                    unsigned hayst_len, unsigned pos) {
  unsigned i;

  for (i = pos; i < hayst_len; i++)
    if (NULL != needle && wcsstr(hayst[i].ident, needle) == hayst[i].ident)
      return i;

  return -1;
}

unsigned index_page(line_t **dst) {
  wchar_t key[] = L"INDEX";
  wchar_t title[] = L"All Manual Pages";
  time_t now = time(NULL);
  wchar_t date[BS_SHORT];
  wcsftime(date, BS_SHORT, L"%x", gmtime(&now));

  line_t *res;
  unsigned res_len =
      aprowhat_render(&res, aw_all, aw_all_len, sc_all, sc_all_len, key, title,
                      config.misc.program_version, date);

  *dst = res;
  return res_len;
}

unsigned aprowhat(line_t **dst, aprowhat_cmd_t cmd, const wchar_t *args,
                  const wchar_t *key, const wchar_t *title) {
  aprowhat_t *aw;
  unsigned aw_len = aprowhat_exec(&aw, cmd, args);
  wchar_t **sc;
  unsigned sc_len = aprowhat_sections(&sc, aw, aw_len);
  time_t now = time(NULL);
  wchar_t date[BS_SHORT];
  wcsftime(date, BS_SHORT, L"%x", gmtime(&now));

  line_t *res;
  unsigned res_len = aprowhat_render(&res, aw, aw_len, sc, sc_len, key, title,
                                     config.misc.program_version, date);

  aprowhat_free(aw, aw_len);
  wafree(sc, sc_len);

  *dst = res;
  return res_len;
}

unsigned man(line_t **dst, const wchar_t *args, bool local_file) {
  unsigned ln = 0; // current line number
  unsigned len;    // length of current line text
  // range_t lrng;    // location of a link found in current line
  // unsigned loff;
  unsigned i, j;                   // iterators
  wchar_t *tmpw = walloc(BS_LINE); // temporary
  char *tmps = salloc(BS_LINE);    // temporary

  unsigned res_len = 1024;               // result buffer length
  line_t *res = aalloc(res_len, line_t); // result buffer

  // Set up the environment for man to create its output as we want it
  char *old_term = getenv("TERM");
  setenv("TERM", "xterm", true);
  sprintf(tmps, "%d",
          4 + config.layout.main_width - config.layout.lmargin -
              config.layout.rmargin);
  setenv("MANWIDTH", tmps, true);
  setenv("MAN_KEEP_FORMATTING", "1", true);
  setenv("GROFF_SGR", "1", true);
  unsetenv("GROFF_NO_SGR");

  // Prepare man command
  char cmdstr[BS_SHORT];
  if (local_file)
    sprintf(cmdstr, "%s --warnings='!all' --local-file %ls 2>>/dev/null",
            config.misc.man_path, args);
  else
    sprintf(cmdstr, "%s --warnings='!all' %ls 2>>/dev/null",
            config.misc.man_path, args);

  // Execute man and, read its output, and process it into res
  FILE *pp = xpopen(cmdstr, "r");

  // For each line...
  xfgets(tmps, BS_LINE, pp);
  while (!feof(pp)) {
    // Ugly hack: sometimes ncurses rudely interrupts xfgets(), which causes its
    // return value to return NULL. If that happens, we skip the current loop
    // step.
    if (NULL == tmps)
      continue;

    // Process text and formatting attirbutes
    len = mbstowcs(tmpw, tmps, BS_LINE);
    line_alloc(res[ln], config.layout.lmargin + len);
    for (j = 0; j < config.layout.lmargin; j++)
      res[ln].text[j] = L' ';
    for (i = 0; i < len; i++) {
      if (got_not_bold) {
        bset(res[ln].reg, j);
        i += 3;
      } else if (got_not_italic || got_not_uline || got_normal) {
        bset(res[ln].reg, j);
        i += 4;
      } else if (got_bold) {
        bset(res[ln].bold, j);
        i += 3;
      } else if (got_italic) {
        bset(res[ln].italic, j);
        i += 3;
      } else if (got_uline) {
        bset(res[ln].uline, j);
        i += 3;
      } else if (got_any_1) {
        i += 3;
      } else if (got_any_2) {
        i += 4;
      } else if (got_any_3) {
        i += 5;
      } else if (tmpw[i] != L'\n') {
        res[ln].text[j] = tmpw[i];
        j++;
      }
    }

    xfgets(tmps, BS_LINE, pp);

    // Discover and add links (skipping the first two lines, and the last line)
    if (ln > 1 && !feof(pp)) {
      discover_links(&re_man, &res[ln], LT_MAN);
      discover_links(&re_http, &res[ln], LT_HTTP);
      discover_links(&re_email, &res[ln], LT_EMAIL);
      for (unsigned i = 0; i < res[ln].links_length; i++) {
      }
    }

    inc_ln;
  }

  // Restore the environment
  setenv("TERM", old_term, true);

  xpclose(pp);
  free(tmpw);
  free(tmps);

  // If no resulst were returned by man, set err and err_msg
  err = false;
  if (0 == ln) {
    err = true;
    swprintf(err_msg, BS_LINE, L"No manual page for %ls", args);
  }

  *dst = res;
  return ln;
}

link_loc_t prev_link(line_t *lines, unsigned lines_len, link_loc_t start) {
  unsigned i;
  link_loc_t res;

  // If start was not found, return not found
  if (!start.ok) {
    res.ok = false;
    return res;
  }

  // If line no. start.line has a link before start.link, return that link
  if (start.link > 0) {
    res.ok = true;
    res.line = start.line;
    res.link = start.link - 1;
    return res;
  }

  // Otherwise, return the last link of the first line before line no.
  // start.line that has links
  for (i = start.line - 1; i > 0; i--) {
    if (lines[i].links_length > 0) {
      res.ok = true;
      res.line = i;
      res.link = lines[i].links_length - 1;
      return res;
    }
  }

  // Return not found if that fails
  res.ok = false;
  return res;
}

link_loc_t next_link(line_t *lines, unsigned lines_len, link_loc_t start) {
  unsigned i;
  link_loc_t res;

  // If start was not found, return not found
  if (!start.ok) {
    res.ok = false;
    return res;
  }

  // If start.line is larger than lines_len, return not found
  if (start.line >= lines_len) {
    res.ok = false;
    return res;
  }

  // If line no. start.line has a link after start.link, return that link
  if (lines[start.line].links_length > start.link + 1) {
    res.ok = true;
    res.line = start.line;
    res.link = start.link + 1;
    return res;
  }

  // Otherwise, return the first link of the first line after line no.
  // start.line that has links
  for (i = start.line + 1; i < lines_len; i++) {
    if (lines[i].links_length > 0) {
      res.ok = true;
      res.line = i;
      res.link = 0;
      return res;
    }
  }

  // Return not found if that fails
  res.ok = false;
  return res;
}

link_loc_t first_link(line_t *lines, unsigned lines_len, unsigned start,
                      unsigned stop) {
  unsigned i;
  link_loc_t res;

  // Sanitize arguments, and return not found if they don't make sense
  if (stop > lines_len)
    stop = lines_len;
  if (start > lines_len || start > stop) {
    res.ok = false;
    return res;
  }

  // Attempt to find and return the first link in the line range
  for (i = start; i <= stop; i++) {
    if (lines[i].links_length > 0) {
      res.ok = true;
      res.line = i;
      res.link = 0;
      return res;
    }
  }

  // If that fails, return not found
  res.ok = false;
  return res;
}

link_loc_t last_link(line_t *lines, unsigned lines_len, unsigned start,
                     unsigned stop) {
  unsigned i;
  link_loc_t res;

  // Sanitize arguments, and return not found if they don't make sense
  if (stop > lines_len)
    stop = lines_len;
  if (start > lines_len || start > stop) {
    res.ok = false;
    return res;
  }

  // Attempt to find and return the last link in the line range
  for (i = stop; i >= start && i != (unsigned)-1; i--) {
    if (lines[i].links_length > 0) {
      res.ok = true;
      res.line = i;
      res.link = lines[i].links_length - 1;
      return res;
    }
  }

  // If that fails, return not found
  res.ok = false;
  return res;
}

unsigned search(result_t **dst, wchar_t *needle, line_t *lines,
                unsigned lines_len) {
  unsigned ln;                          // current line no.
  unsigned i = 0;                       // current result no.
  unsigned needle_len = wcslen(needle); // length of needle
  wchar_t *cur_hayst;      // current haystuck (i.e. text of current line)
  wchar_t *hit = NULL;     // current return value of wcsstr()
  unsigned res_len = 1024; // result buffer length
  result_t *res = aalloc(res_len, result_t); // result buffer

  // For each line...
  for (ln = 0; ln < lines_len; ln++) {
    // Start at the beginning of the line's text
    cur_hayst = lines[ln].text;
    // Search for needle
    hit = wcsstr(cur_hayst, needle);
    // While needle has been found...
    while (NULL != hit) {
      // Add the search result to res[i]
      res[i].line = ln;
      res[i].start = hit - lines[ln].text;
      res[i].end = res[i].start + needle_len;
      // Go to the part of the line's text that follows needle
      cur_hayst = hit + needle_len;
      // And search for needle again (except in case of overflow)
      if (cur_hayst - lines[ln].text < lines[ln].length)
        hit = wcsstr(cur_hayst, needle);
      else
        hit = NULL;
      // Increment i (and reallocate memory if necessary)
      inc_i;
    }
  }

  *dst = res;
  return i;
}

int search_next(result_t *res, unsigned res_len, unsigned from) {
  unsigned i;

  for (i = 0; i < res_len; i++)
    if (res[i].line >= from)
      return res[i].line;

  return -1;
}

int search_prev(result_t *res, unsigned res_len, unsigned from) {
  unsigned i;
  unsigned prev_line = -1;

  for (i = 0; i < res_len && res[i].line < from; i++)
    prev_line = res[i].line;

  return prev_line;
}

void populate_page() {
  // If page is already populated, free its allocated memory
  if (NULL != page && page_len > 0) {
    lines_free(page, page_len);
    page = NULL;
    page_len = 0;
  }

  // Populate page according to the request type of history[history_cur]
  switch (history[history_cur].request_type) {
  case RT_INDEX:
    wcscpy(page_title, L"All Manual Pages");
    page_len = index_page(&page);
    break;
  case RT_MAN:
    swprintf(page_title, BS_SHORT, L"Manual page(s) for: %ls",
             history[history_cur].args);
    page_len = man(&page, history[history_cur].args, false);
    break;
  case RT_MAN_LOCAL:
    swprintf(page_title, BS_SHORT, L"Manual page(s) in local file(s): %ls",
             history[history_cur].args);
    page_len = man(&page, history[history_cur].args, true);
    break;
  case RT_APROPOS:
    swprintf(page_title, BS_SHORT, L"Apropos for: %ls",
             history[history_cur].args);
    page_len = aprowhat(&page, AW_APROPOS, history[history_cur].args,
                        L"APROPOS", page_title);
    break;
  case RT_WHATIS:
    swprintf(page_title, BS_SHORT, L"Whatis for: %ls",
             history[history_cur].args);
    page_len = aprowhat(&page, AW_WHATIS, history[history_cur].args, L"WHATIS",
                        page_title);
  }

  // Reset search results
  if (NULL != results && results_len > 0)
    free(results);
  results = NULL;
  results_len = 0;
}

void requests_free(request_t *reqs, unsigned reqs_len) {
  unsigned i;

  for (i = 0; i < reqs_len; i++)
    if (NULL != reqs[i].args)
      free(reqs[i].args);

  free(reqs);
}

void aprowhat_free(aprowhat_t *aw, unsigned aw_len) {
  unsigned i;

  for (i = 0; i < aw_len; i++) {
    free(aw[i].page);
    free(aw[i].section);
    free(aw[i].ident);
    free(aw[i].descr);
  }

  free(aw);
}

void lines_free(line_t *lines, unsigned lines_len) {
  unsigned i;

  for (i = 0; i < lines_len; i++) {
    line_free(lines[i]);
  }

  free(lines);
}

void winddown(int ec, const wchar_t *em) {
  // Shut ncurses down
  winddown_tui();

  // Deallocate memory used by config global
  if (NULL != config.chars.sbar_top)
    free(config.chars.sbar_top);
  if (NULL != config.chars.sbar_vline)
    free(config.chars.sbar_vline);
  if (NULL != config.chars.sbar_bottom)
    free(config.chars.sbar_bottom);
  if (NULL != config.chars.sbar_block)
    free(config.chars.sbar_block);
  if (NULL != config.chars.trans_mode_name)
    free(config.chars.trans_mode_name);
  if (NULL != config.chars.trans_name_loc)
    free(config.chars.trans_name_loc);
  if (NULL != config.chars.trans_prompt_help)
    free(config.chars.trans_prompt_help);
  if (NULL != config.chars.trans_prompt_em)
    free(config.chars.trans_prompt_em);
  if (NULL != config.chars.box_hline)
    free(config.chars.box_hline);
  if (NULL != config.chars.box_vline)
    free(config.chars.box_vline);
  if (NULL != config.chars.box_tl)
    free(config.chars.box_tl);
  if (NULL != config.chars.box_tr)
    free(config.chars.box_tr);
  if (NULL != config.chars.box_bl)
    free(config.chars.box_bl);
  if (NULL != config.chars.box_br)
    free(config.chars.box_br);
  if (NULL != config.misc.program_name)
    free(config.misc.program_name);
  if (NULL != config.misc.program_version)
    free(config.misc.program_version);
  if (NULL != config.misc.config_path)
    free(config.misc.config_path);
  if (NULL != config.misc.man_path)
    free(config.misc.man_path);
  if (NULL != config.misc.whatis_path)
    free(config.misc.whatis_path);
  if (NULL != config.misc.apropos_path)
    free(config.misc.apropos_path);
  if (NULL != config.misc.browser_path)
    free(config.misc.browser_path);
  if (NULL != config.misc.mailer_path)
    free(config.misc.mailer_path);

  // Deallocate memory used by history global
  requests_free(history, config.misc.history_size);

  // Deallocate memory used by aw_all global
  if (NULL != aw_all && aw_all_len > 0)
    aprowhat_free(aw_all, aw_all_len);

  // Deallocate memory used by sc_all global
  if (NULL != sc_all && sc_all_len > 0)
    wafree(sc_all, sc_all_len);

  // Deallocate memory used by page global
  if (NULL != page && page_len > 0)
    lines_free(page, page_len);

  // Deallocate memory used by results
  if (NULL != results && results_len > 0)
    free(results);

  // Deallocate memory used by re_... regular expression globals
  regfree(&re_man.re);
  regfree(&re_http.re);
  regfree(&re_email.re);

  // (Optionally) print em and exit
  if (NULL != em)
    fwprintf(stderr, L"%ls\n", em);
  exit(ec);
}
