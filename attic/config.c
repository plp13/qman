// Configuration and configuration file handling (implementation)

#include "lib.h"

//
// Global variables
//

config_t config;

//
// Helper macros and functions
//

// Helper of conf_init() conf_handler(). Initialize the values of all colour
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
// held into src as an integer. If src cannot be parsed into an integer,
// return INT_MIN.
int str2int(const char *src) {
  int val, vals_read;
  vals_read = sscanf(src, "%d", &val);
  if (1 != vals_read)
    return INT_MIN;
  else
    return val;
}

// Helper of various conf_...() functions and methods. Return the ncurses colour
// that corresponds to src. Allowed values for src are 'black', 'red', 'green',
// 'yellow', 'blue', 'magenta', 'cyan', 'white', a positive integer, or
// '#rrggbb'.
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

// Helper of various conf_...() functions and methods. Return the ncurses key
// mapping that corresponds to src.
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

// Helper of conf_handler(). Return an if statement that matches if section is
// equal to s.
#define conf_section(s) if (0 == strcmp(section, s))

// Helper of conf_handler(). Set trgt to the colour that corresponds to value,
// if name is equal to n. value is formatted as 3 colour definitions (as
// accepted by str2colour) separated by whitespace.
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
// correspond to value, if name is equal to n. value is formatted as 8 key
// mapping definitions (as accepted by str2ch) separated by whitespace.
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

// Helper of conf_handler(). Set trgt to the boolean stored in value, using
// str2bool(), if name is equal to n.
#define conf_set_bool(n, trgt)                                                 \
  if (0 == strcmp(name, n)) {                                                  \
    trgt = str2bool(value);                                                    \
    return true;                                                               \
  }

// Helper of conf_handler(). Set trgt to the integer stored in value, using
// str2int(), if name is equal to n. min and max can be used to define a lower
// and an upper boundary for value, for error-checking purposes.
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

//
// Functions
//

void conf_init() {
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
