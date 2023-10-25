// Program-specific infrastructure (implementation)

#include "program.h"
#include "lib.h"
#include "tui.h"
#include "util.h"
#include <regex.h>
#include <stdlib.h>
#include <wchar.h>

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

unsigned page_len = 0;

unsigned page_top = 0;

unsigned page_left = 0;

full_regex_t re_man, re_http, re_email;

//
// Helper macros and functions
//

// Helper of parse_args(). Surround all arguments of argv with  single quotes,
// and place them in tmp.
#define flatten_args                                                           \
  tmp_len = 0;                                                                 \
  wcscpy(tmp, L"");                                                            \
  for (i = 1; i < argc; i++) {                                                 \
    swprintf(tmp2, BS_SHORT, L"'%s'", argv[i]);                                \
    if (tmp_len + wcslen(tmp2) < BS_LINE) {                                    \
      wcscat(tmp, tmp2);                                                       \
      tmp_len += wcslen(tmp2);                                                 \
    }                                                                          \
    if (i < argc - 1 && tmp_len + 1 < BS_LINE) {                               \
      wcscat(tmp, L" ");                                                       \
      tmp_len++;                                                               \
    }                                                                          \
  }

// Helper of man() and aprowhat_render(). Increase ln, and reallocate res in
// memory, if ln has exceeded its size.
#define inc_ln                                                                 \
  ln++;                                                                        \
  if (ln == res_len) {                                                         \
    res_len += 1024;                                                           \
    res = xreallocarray(res, res_len, sizeof(line_t));                         \
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
  line->links[line->links_length - 1].type = LT_MAN;
  wcscpy(line->links[line->links_length - 1].trgt, trgt);
}

// Helper of man(). Discover links that match in the text of line, and add them
// to said line.
void discover_links(const full_regex_t *re, line_t *line) {
  unsigned loff = 0; // offset (in line text) to start searching for links
  range_t lrng =
      fr_search(&re_man, &line->text[loff]); // location of link in line
  wchar_t *tmp = walloca(BS_LINE);           // temporary

  while (lrng.beg != lrng.end) {
    // While a link has been found, add it to the line
    wcsncpy(tmp, &line->text[loff + lrng.beg], lrng.end - lrng.beg);
    tmp[lrng.end - lrng.beg] = L'\0';
    add_link(line, loff + lrng.beg, loff + lrng.end, LT_MAN, tmp);
    loff += lrng.end;
    if (loff < line->length) {
      // Link wasn't at the very end of line; look for another link after it
      lrng = fr_search(&re_man, &line->text[loff]);
    } else {
      // Link was at the very end of line; exit the loop
      lrng.beg = 0;
      lrng.end = 0;
    }
  }
}

// All got_... macros are helpers of man()

// true if we tmpw[i] contains a 'bold' terminal escape sequence
#define got_bold                                                               \
  ((i + 4 < len) && (tmpw[i] == L'\e') && (tmpw[i + 1] == L'[') &&             \
   (tmpw[i + 2] == L'1') && (tmpw[i + 3] == L'm'))

// true if we tmpw[i] contains a 'not bold' terminal escape sequence
#define got_not_bold                                                           \
  ((i + 4 < len) && (tmpw[i] == L'\e') && (tmpw[i + 1] == L'[') &&             \
   (tmpw[i + 2] == L'0') && (tmpw[i + 3] == L'm'))

// true if we tmpw[i] contains a 'italic' terminal escape sequence
#define got_italic                                                             \
  ((i + 4 < len) && (tmpw[i] == L'\e') && (tmpw[i + 1] == L'[') &&             \
   (tmpw[i + 2] == L'3') && (tmpw[i + 3] == L'm'))

// true if we tmpw[i] contains a 'not italic' terminal escape sequence
#define got_not_italic                                                         \
  ((i + 5 < len) && (tmpw[i] == L'\e') && (tmpw[i + 1] == L'[') &&             \
   (tmpw[i + 2] == L'2') && (tmpw[i + 3] == L'3') && (tmpw[i + 4] == L'm'))

// true if we tmpw[i] contains a 'underline' terminal escape sequence
#define got_uline                                                              \
  ((i + 4 < len) && (tmpw[i] == L'\e') && (tmpw[i + 1] == L'[') &&             \
   (tmpw[i + 2] == L'4') && (tmpw[i + 3] == L'm'))

// true if we tmpw[i] contains a 'not underline' terminal escape sequence
#define got_not_uline                                                          \
  ((i + 5 < len) && (tmpw[i] == L'\e') && (tmpw[i + 1] == L'[') &&             \
   (tmpw[i + 2] == L'2') && (tmpw[i + 3] == L'4') && (tmpw[i + 4] == L'm'))

// true if we tmpw[i] contains a 'normal / not dim' terminal escape sequence
#define got_normal                                                             \
  ((i + 5 < len) && (tmpw[i] == L'\e') && (tmpw[i + 1] == L'[') &&             \
   (tmpw[i + 2] == L'2') && (tmpw[i + 3] == L'2') && (tmpw[i + 4] == L'm'))

// true if we tmpw[i] contains any terminal escape sequence that resets it to
// normal text
#define got_reg (got_not_bold || got_not_italic || got_not_uline || got_normal)

//
// Functions
//

void init() {
  // Use the system locale
  setlocale(LC_ALL, "");

  // Initialize config with sane defaults
  config.chars.sbar_top = L"┳";
  config.chars.sbar_vline = L"┃";
  config.chars.sbar_bottom = L"┻";
  config.chars.sbar_block = L"█";
  config.chars.trans_mode_name = L"┇";
  config.chars.trans_name_loc = L"┇";
  config.chars.trans_prompt_help = L"┇";
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
  config.colours.link_http.fg = COLOR_GREEN;
  config.colours.link_http.bold = false;
  config.colours.link_http.bg = COLOR_BLACK;
  config.colours.link_http.pair = 21;
  config.colours.link_http_f.fg = COLOR_BLACK;
  config.colours.link_http_f.bold = false;
  config.colours.link_http_f.bg = COLOR_GREEN;
  config.colours.link_http_f.pair = 31;
  config.colours.link_email.fg = COLOR_GREEN;
  config.colours.link_email.bold = false;
  config.colours.link_email.bg = COLOR_BLACK;
  config.colours.link_email.pair = 22;
  config.colours.link_email_f.fg = COLOR_BLACK;
  config.colours.link_email_f.bold = false;
  config.colours.link_email_f.bg = COLOR_GREEN;
  config.colours.link_email_f.pair = 32;
  config.colours.link_ls.fg = COLOR_GREEN;
  config.colours.link_ls.bold = false;
  config.colours.link_ls.bg = COLOR_BLACK;
  config.colours.link_ls.pair = 23;
  config.colours.link_ls_f.fg = COLOR_BLACK;
  config.colours.link_ls_f.bold = false;
  config.colours.link_ls_f.bg = COLOR_GREEN;
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
  config.colours.trans_mode_name = 100 * config.colours.stat_indic_mode.pair +
                                   config.colours.stat_indic_name.pair;
  config.colours.trans_name_loc = 100 * config.colours.stat_indic_name.pair +
                                  config.colours.stat_indic_loc.pair;
  config.colours.trans_prompt_help =
      100 * config.colours.stat_input_prompt.pair +
      config.colours.stat_input_help.pair;
  config.keys.up = (int[]){KEY_UP, KEY_BACKSPACE, 'y', 'k', 0};
  config.keys.down = (int[]){KEY_DOWN, KEY_ENTER, 'e', 'j', 0};
  config.keys.help = (int[]){'h', '?', 0};
  config.keys.quit = (int[]){'q', KEY_BREAK, 0};
  config.layout.tui = false;
  config.layout.fixedwidth = false;
  config.layout.sb = true;
  config.layout.width = 80;
  config.layout.height = 25;
  config.layout.sbar_width = 1;
  config.layout.stat_height = 2;
  config.layout.main_width = 79;
  config.layout.main_height = 23;
  config.layout.lmargin = 2;
  config.layout.rmargin = 2;
  config.misc.program_name = walloc(BS_SHORT);
  wcscpy(config.misc.program_name, L"qman");
  config.misc.program_version = walloc(BS_SHORT);
  wcscpy(config.misc.program_version, L"qman nightly");
  config.misc.man_path = salloc(BS_SHORT);
  strcpy(config.misc.man_path, "/usr/bin/man");
  config.misc.whatis_path = salloc(BS_SHORT);
  strcpy(config.misc.whatis_path, "/usr/bin/whatis");
  config.misc.apropos_path = salloc(BS_SHORT);
  strcpy(config.misc.apropos_path, "/usr/bin/apropos");
  config.misc.config_path = salloc(BS_SHORT);
  strcpy(config.misc.config_path, "~/qman.conf");
  config.misc.history_size = 256;

  // Initialize history
  history_cur = 0;
  history_top = 0;
  history = aalloc(config.misc.history_size, request_t);
  history[0].request_type = RT_INDEX;
  history[0].args = NULL;

  // Initialize aw_all and sc_all
  aw_all_len = aprowhat_exec(&aw_all, AW_APROPOS, "''");
  sc_all_len = aprowhat_sections(&sc_all, aw_all, aw_all_len);

  // initialize regular expressions
  fr_init(&re_man, "[a-zA-Z0-9\\.:@_-]+\\([a-zA-Z0-9]+\\)");
  fr_init(&re_http, "https?:\\/\\/[a-zA-Z0-9\\.\\[\\]\\/\\?\\+:@_#%-]+");
  fr_init(&re_email, "[a-zA-Z0-9\\.\\$\\*\\+\\?\\^\\|!#%&'/"
                     "=_`{}~-][a-zA-Z0-9\\.\\$\\*\\+\\?\\^\\|\\.!#%&'/"
                     "=_`{}~-]*@[a-zA-Z0-9-][a-zA-Z0-9\\.-]*");
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

  // Parse the options and modify config and requests
  while (true) {
    int cur_i;
    char cur = getopt_long(argc, argv, optstring, longopts, &cur_i);
    switch (cur) {
    case -1:
      free(longopts);
      return optind;
      break;
    case 'n': {
      history[0].request_type = RT_INDEX;
    } break;
    case 'k': {
      history[0].request_type = RT_WHATIS;
    } break;
    case 'f': {
      history[0].request_type = RT_APROPOS;
    } break;
    case 'T': {
      config.layout.tui = false;
    } break;
    case 'C': {
      unsigned optarg_len = strlen(optarg);
      config.misc.config_path = walloc(optarg_len);
      strcpy(config.misc.config_path, optarg);
    } break;
    case 'h': {
      usage();
      winddown(ES_SUCCESS, NULL);
    } break;
    case '?': {
      free(longopts);
      wchar_t *msg = L"Unable to parse program arguments";
      winddown(ES_USAGE_ERROR, msg);
    } break;
    }
  }
}

void parse_args(int argc, char *const *argv) {
  unsigned i;                           // iterator
  wchar_t tmp[BS_LINE], tmp2[BS_SHORT]; // temporary
  unsigned tmp_len; // length of tmp (used to guard against buffer overflows)

  // If the user has specified at least an argument, try to show a manual page
  // rather than the index page
  if (argc >= 2 && RT_INDEX == history[history_top].request_type)
    history[history_top].request_type = RT_MAN;

  // If we are showing a manual, apropos, or whatis page...
  if (history[history_top].request_type != RT_INDEX) {
    // But the user hasn't specified an argument...
    if (argc < 2) {
      // Exit with error message
      switch (history[history_top].request_type) {
      case RT_MAN:
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

    // Flatten all arguments into history[history_top].args
    flatten_args;
    history[history_top].args = walloc(wcslen(tmp));
    wcscpy(history[history_top].args, tmp);
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

unsigned aprowhat_exec(aprowhat_t **dst, aprowhat_cmd_t cmd, const char *args) {
  // Prepare apropos/whatis command
  char cmdstr[BS_SHORT];
  if (cmd == AW_WHATIS)
    sprintf(cmdstr, "%s -l %s", config.misc.whatis_path, args);
  else
    sprintf(cmdstr, "%s -l %s", config.misc.apropos_path, args);

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

  // For each line returned by apropos...
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
    res[i].descr = walloc(descr_len);
    mbstowcs(res[i].descr, descr, descr_len);
  }

  xfclose(fp);

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
      res[res_i] = walloc(wcslen(aw[i].section));
      wcscpy(res[res_i], aw[i].section);
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
  unsigned line_width = MAX(60, config.layout.width - config.layout.sbar_width);
  unsigned lmargin_width = config.layout.lmargin; // left margin
  unsigned rmargin_width = config.layout.rmargin; // right margin
  unsigned main_width =
      line_width - lmargin_width - rmargin_width; // main text area
  unsigned hfc_width =
      main_width / 2 + main_width % 2; // header/footer centre area
  unsigned hfl_width = (main_width - hfc_width) / 2; // header/footer left area
  unsigned hfr_width =
      hfl_width + (main_width - hfc_width) % 2; // header/footer right area

  unsigned ln = 0;                // current line number
  unsigned i, j;                  // iterators
  wchar_t *tmp = walloc(BS_LINE); // temporary

  unsigned res_len = 1024;               // result buffer length
  line_t *res = aalloc(res_len, line_t); // result buffer

  // Header
  line_alloc(res[ln], line_width);
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
           main_width, tmp);
  bset(res[ln].bold, lmargin_width);
  bset(res[ln].reg, lmargin_width + wcslen(tmp));

  // Sections
  unsigned sc_maxwidth = wmaxlen(sc, sc_len); // length of longest section
  unsigned sc_cols =
      main_width / (4 + sc_maxwidth);   // number of columns for sections
  unsigned sc_lines = sc_len / sc_cols; // number of lines for sections
  unsigned sc_i;                        // index of current section
  if (sc_len % sc_cols > 0)
    sc_lines++;
  for (i = 0; i < sc_lines; i++) {
    inc_ln;
    line_alloc(res[ln], line_width + 1); // +1 because of wcscat()
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

  // Newline
  inc_ln;
  line_alloc(res[ln], 0);

  // For each section...
  for (i = 0; i < sc_len; i++) {
    // Newline
    inc_ln;
    line_alloc(res[ln], 0);
    // Section title
    inc_ln;
    line_alloc(res[ln], line_width);
    swprintf(tmp, main_width + 1, L"MANUAL PAGES IN SECTION '%ls'", sc[i]);
    swprintf(res[ln].text, line_width + 1, L"%*s%-*ls", //
             lmargin_width, "",                         //
             main_width, tmp);
    bset(res[ln].bold, lmargin_width);
    bset(res[ln].reg, lmargin_width + wcslen(tmp));
    // For each manual page...
    for (j = 0; j < aw_len; j++) {
      // If manual page is in current section...
      if (0 == wcscmp(aw[j].section, sc[i])) {
        unsigned lc_width = main_width / 3;        // left column width
        unsigned rc_width = main_width - lc_width; // right column width
        unsigned page_width = wcslen(aw[j].page) + wcslen(aw[j].section) +
                              2; // width of manual page name and section
        unsigned spcl_width =
            MAX(line_width,
                lmargin_width + page_width +
                    rmargin_width); // used in place of line_width; might be
                                    // longer, in which case we'll scroll

        // Page name and section
        inc_ln;
        line_alloc(res[ln], spcl_width);
        swprintf(tmp, page_width + 1, L"%ls(%ls)", aw[j].page, aw[j].section);
        swprintf(res[ln].text, spcl_width + 1, L"%*s%-*ls", //
                 lmargin_width, "",                         //
                 lc_width, tmp);
        add_link(&res[ln], lmargin_width, lmargin_width + wcslen(tmp), LT_MAN,
                 tmp);

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
  unsigned ver_len = wcslen(ver);   // ver length
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
  bset(res[ln].uline, lmargin_width);
  bset(res[ln].reg, lmargin_width + ver_len);
  bset(res[ln].uline,
       lmargin_width + hfl_width + hfc_width + hfr_width - key_len);
  bset(res[ln].reg, lmargin_width + hfl_width + hfc_width + hfr_width);

  free(tmp);

  *dst = res;
  return ln + 1;
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

unsigned aprowhat(line_t **dst, aprowhat_cmd_t cmd, const char *args,
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

unsigned man(line_t **dst, const char *args) {
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
          4 + config.layout.width - config.layout.sbar_width -
              config.layout.lmargin - config.layout.rmargin);
  setenv("MANWIDTH", tmps, true);
  setenv("MAN_KEEP_FORMATTING", "1", true);

  // Prepare man command
  char cmdstr[BS_SHORT];
  sprintf(cmdstr, "%s %s", config.misc.man_path, args);

  // Execute man and, read its output, and process it into res
  FILE *pp = xpopen(cmdstr, "r");

  // For each line...
  xfgets(tmps, BS_LINE, pp);
  while (!feof(pp)) {
    // Process text and formatting attirbutes
    len = mbstowcs(tmpw, tmps, BS_LINE);
    line_alloc(res[ln], config.layout.lmargin + len);
    for (j = 0; j < config.layout.lmargin; j++)
      res[ln].text[j] = L' ';
    for (i = 0; i < len; i++) {
      if (got_reg) {
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
      } else if (tmpw[i] != '\n') {
        res[ln].text[j] = tmpw[i];
        j++;
      }
    }

    xfgets(tmps, BS_LINE, pp);

    // Discover and add links (skipping the first two lines, and the last line)
    if (ln > 1 && !feof(pp)) {
      discover_links(&re_man, &res[ln]);
      discover_links(&re_http, &res[ln]);
      discover_links(&re_email, &res[ln]);
    }

    inc_ln;
  }

  // Restore the environment
  setenv("TERM", old_term, true);

  xpclose(pp);
  free(tmpw);
  free(tmps);

  *dst = res;
  return ln;
}

link_loc_t next_link(line_t *lines, unsigned lines_len, link_loc_t start) {
  unsigned i;
  link_loc_t res;

  // If line no. start.line is longer than lines_len, return not found
  if (start.line >= lines_len) {
    res.ok = FALSE;
    return res;
  }

  // If line no. start.line has a link after start.link, return that link
  if (lines[start.line].links_length > start.link + 1) {
    res.ok = TRUE;
    res.line = start.line;
    res.link = start.link + 1;
    return res;
  }

  // Otherwise, return the first link of the first line after line no.
  // start.line that has links
  for (i = start.line + 1; i < lines_len; i++) {
    if (lines[i].links_length > 0) {
      res.ok = TRUE;
      res.line = i;
      res.link = 0;
      return res;
    }
  }

  // Return not found if that fails
  res.ok = FALSE;
  return res;
}

void aprowhat_free(aprowhat_t *aw, unsigned aw_len) {
  unsigned i;

  for (i = 0; i < aw_len; i++) {
    free(aw[i].page);
    free(aw[i].section);
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

  // Deallocate memory used by history global
  unsigned i;
  for (i = 0; i <= history_top; i++) {
    if (NULL != history[i].args)
      free(history[i].args);
  }
  if (NULL != history)
    free(history);

  // Deallocate memory used by aw_all global
  if (NULL != aw_all && aw_all_len > 0)
    aprowhat_free(aw_all, aw_all_len);

  // Deallocate memory used by sc_all global
  if (NULL != sc_all && sc_all_len > 0)
    wafree(sc_all, sc_all_len);

  // Deallocate memory used by page global
  if (NULL != page && page_len > 0)
    lines_free(page, page_len);

  // Deallocate memory used by re_... regular expression globals
  regfree(&re_man.re);
  regfree(&re_http.re);
  regfree(&re_email.re);

  // (Optionally) print em and exit
  if (NULL != em)
    wprintf(L"%ls\n", em);
  exit(ec);
}
