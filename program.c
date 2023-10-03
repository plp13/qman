// Program-specific infrastructure (implementation)

#include "program.h"
#include "lib.h"
#include "util.h"

//
// Global variables
//

// Program options
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

// Program configuration
config_t config;

// History of page requests
request_t *requests = NULL;

// Location of current request in requests array
unsigned current;

//
// Functions
//

void init() {
  // Use the system locale
  setlocale(LC_ALL, "");

  // Initialize config with sane defaults
  config.layout.tui = false;
  config.layout.fixedwidth = false;
  config.layout.width = 80;
  config.layout.height = 25;
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
  config.misc.requests_size = 256;

  // Initialize requests and current
  current = 0;
  requests = aalloc(config.misc.requests_size, request_t);
  requests[current].request_type = RT_INDEX;
  requests[current].page = NULL;
  requests[current].section = NULL;
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
      requests[0].request_type = RT_INDEX;
    } break;
    case 'k': {
      requests[0].request_type = RT_WHATIS;
    } break;
    case 'f': {
      requests[0].request_type = RT_APROPOS;
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

void usage() {
  // Header
  wprintf(L"Usage: %s [OPTION...] [SECTION] [PAGE]...\n\n",
          config.misc.program_name);

  // Command-line options
  unsigned i = 0, j;
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

  unsigned len, page_len, section_len, descr_len, i;

  // For each line returned by apropos...
  for (i = 0; i < lines; i++) {
    len = sreadline(line, BS_LINE, fp);

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

// Helper of aprowhat_render(). Increase ln, and reallocate res in memory, if ln
// has exceeded its size.
#define inc_ln                                                                 \
  ln++;                                                                        \
  if (ln == res_len) {                                                         \
    res_len += 1024;                                                           \
    res = xreallocarray(res, res_len, sizeof(line_t));                         \
  }

unsigned aprowhat_render(line_t **dst, const aprowhat_t *aw, unsigned aw_len,
                         wchar_t *const *sc, unsigned sc_len,
                         const wchar_t *key, const wchar_t *title,
                         const wchar_t *ver, const wchar_t *date) {

  // Text blocks widths
  unsigned line_width = MAX(60, config.layout.width);
  unsigned lmargin_width = config.layout.lmargin; // left margin
  unsigned rmargin_width = config.layout.rmargin; // right margin
  unsigned main_width =
      line_width - lmargin_width - rmargin_width; // main text area
  unsigned hfc_width =
      main_width / 2 + main_width % 2; // header/footer centre area
  unsigned hfl_width = (main_width - hfc_width) / 2; // header/footer left area
  unsigned hfr_width =
      hfl_width + (main_width - hfc_width) % 2; // header/footer right area

  unsigned ln;                    // current line number
  unsigned i, j;                  // iterators
  wchar_t *tmp = walloc(BS_LINE); // temporary

  unsigned res_len = 1024;               // result buffer length
  line_t *res = aalloc(res_len, line_t); // result buffer

  // Header
  ln = 0;
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
  bset(res[ln].uline, lmargin_width + hfl_width + hfc_width);
  bset(res[ln].reg, lmargin_width + hfl_width + hfc_width + key_len);

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
  if (sc_len % sc_cols > 0)
    sc_lines++;
  for (i = 0; i < sc_lines; i++) {
    inc_ln;
    line_alloc(res[ln], line_width);
    swprintf(res[ln].text, line_width + 1, L"%*s", lmargin_width, "");
    for (j = 0; j < sc_cols; j++) {
      if (sc_cols * i + j < sc_len) {
        swprintf(tmp, sc_maxwidth + 5, L"%-*ls    ", sc_maxwidth,
                 sc[sc_cols * i + j]);
        wcscat(res[ln].text, tmp);
        bset(res[ln].lman, lmargin_width + j * (4 + sc_maxwidth));
        bset(res[ln].reg, lmargin_width + j * (4 + sc_maxwidth) +
                              wcslen(sc[sc_cols * i + j]));
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
        unsigned descr_width =
            wcslen(aw[j].descr); // width of manual page description
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
        bset(res[ln].lman, lmargin_width);
        bset(res[ln].reg, lmargin_width + page_width);
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
  bset(res[ln].uline, lmargin_width + hfl_width + hfc_width);
  bset(res[ln].reg, lmargin_width + hfl_width + hfc_width + key_len);

  free(tmp);

  *dst = res;
  return ln + 1;
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
  unsigned ln;                    // current line number
  unsigned i, j;                  // iterators
  wchar_t *tmp = walloc(BS_LINE); // temporary
  char *stmp = salloc(BS_SHORT);  // temporary

  unsigned res_len = 1024;               // result buffer length
  line_t *res = aalloc(res_len, line_t); // result buffer

  // Set up the environment for man to create the output we want
  sprintf(stmp, "%d",
          config.layout.width - config.layout.lmargin - config.layout.rmargin);
  setenv("MANWIDTH", stmp, true);
  setenv("MAN_KEEP_FORMATTING", "1", true);

  // Prepare man command
  char cmdstr[BS_SHORT];
  sprintf(cmdstr, "%s -l %s", config.misc.man_path, args);

  free(tmp);

  *dst = res;
  return ln + 1;
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
  // Deallocate memory
  unsigned i;
  for (i = 0; i <= current; i++) {
    if (NULL != requests[i].page)
      free(requests[i].page);
    if (NULL != requests[i].section)
      free(requests[i].section);
  }
  free(requests);
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

  // (Optionally) print em and exit
  if (NULL != em)
    wprintf(L"%ls\n", em);
  exit(ec);
}
