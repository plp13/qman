// Program-specific infrastructure (implementation)

#include "lib.h"

//
// Global variables
//

option_t options[] = {
    {"index", 'n',
     L"Show all manual pages (default behaviour if no PAGE has been specified)",
     OA_NONE, true},
    {"apropos", 'k',
     L"Show a list of all pages whose name and/or description contains PAGE "
     L"(apropos)",
     OA_NONE, true},
    {"whatis", 'f',
     L"Show a list of all pages whose name matches PAGE (whatis)", OA_NONE,
     true},
    {"local-file", 'l', L"Interpret PAGE argument(s) as local filename(s)",
     OA_NONE, true},
    {"global-apropos", 'K',
     L"Show the contents of all pages whose name and/or description contains "
     L"PAGE (global apropos)",
     OA_NONE, true},
    {"all", 'a',
     L"Show the contents of all pages whose name matches PAGE (global whatis)",
     OA_NONE, true},
    {"cli", 'T', L"Suppress the TUI and output directly to the terminal",
     OA_NONE, true},
    {"config-path", 'C', L"Use ARG as the configuration file path", OA_REQUIRED,
     true},
    {"version", 'V', L"Print program version", OA_NONE, true},
    {"help", 'h', L"Display this help message", OA_NONE, true},
    {0, 0, 0, 0, false}};

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

toc_entry_t *toc = NULL;

unsigned toc_len = 0;

bool err = false;

wchar_t err_msg[BS_LINE];

result_t *results = NULL;

unsigned results_len = 0;

mark_t mark = {false, 0, 0, 0, 0};

full_regex_t re_man, re_http, re_email;

//
// Helper macros and functions
//

// Helper of man() and aprowhat_render(). Increase ln, and reallocate res in
// memory, if ln has exceeded its size.
#define inc_ln                                                                 \
  ln++;                                                                        \
  if (ln == res_len) {                                                         \
    res_len += BS_LINE;                                                        \
    res = xreallocarray(res, res_len, sizeof(line_t));                         \
  }

// Helper of toc(). Increase en, and reallocate res in memory, if en has
// exceeded its size
#define inc_en                                                                 \
  en++;                                                                        \
  if (en == res_len) {                                                         \
    res_len += BS_SHORT;                                                       \
    res = xreallocarray(res, res_len, sizeof(toc_entry_t));                    \
  }

// Helper of search(). Increase i, and reallocate res in memory, if i has
// exceeded its size.
#define inc_i                                                                  \
  i++;                                                                         \
  if (i == res_len) {                                                          \
    res_len += BS_LINE;                                                        \
    res = xreallocarray(res, res_len, sizeof(result_t));                       \
  }

// Helper of man() and aprowhat_render(). Add a link to a line. Allocate memory
// using line_realloc_link() to do so. Use start, end, link_next type, and trgt
// to populate the new link's members.
void add_link(line_t *line, unsigned start, unsigned end, bool in_next,
              unsigned start_next, unsigned end_next, link_type_t type,
              const wchar_t *trgt) {
  link_t link;
  int i;

  link.start = start;
  link.end = end;
  link.type = type;
  link.in_next = in_next;
  link.start_next = start_next;
  link.end_next = end_next;
  link.trgt = walloc(wcslen(trgt));
  wcscpy(link.trgt, trgt);

  line_realloc_link((*line));
  if (1 == line->links_length)
    i = 0;
  else {
    for (i = line->links_length - 2; i >= 0; i--)
      if (line->links[i].start > link.end)
        line->links[i + 1] = line->links[i];
      else
        break;
    i++;
  }
  line->links[i] = link;
}

// Helper of man(). Discover links that match re in the text of line, and add
// them to said line. line_next is necessary to support hyphenated links. type
// signifies the link type to add.
void discover_links(const full_regex_t *re, line_t *line, line_t *line_next,
                    const link_type_t type) {
  // Ignore empty lines
  if (line->length < 2)
    return;

  wchar_t ltext[BS_LINE * 2]; // text of line (or text of line merged with text
                              // of line_next, if line is hyphenated)
  memset(ltext, 0, sizeof(wchar_t) * BS_LINE * 2);
  const bool lhyph =
      line->text[line->length - 2] == L'‐'; // whether line is hyphenated
  unsigned loff = 0;     // offset (in ltext) to start searching for links
  range_t lrng;          // location of link in ltext
  wchar_t trgt[BS_LINE]; // link target

  // Prepare ltext
  wcsncpy(ltext, line->text, line->length - 1);
  if (lhyph) {
    unsigned lnme =
        wmargend(line_next->text, NULL); // left margin end of line_next
    wcsncpy(&ltext[line->length - 2], &line_next->text[lnme],
            line_next->length - lnme);
  }

  // While a link has been found...
  lrng = fr_search(re, &ltext[loff]);
  while (lrng.beg != lrng.end) {
    // Extract link target from line
    wcsncpy(trgt, &ltext[loff + lrng.beg], lrng.end - lrng.beg);
    trgt[lrng.end - lrng.beg] = L'\0';

    if (lhyph && loff + lrng.beg < line->length &&
        loff + lrng.end >= line->length) {
      // Link is broken by a hyphen

      const unsigned lnme = wmargend(
          line_next->text, NULL); // position where actual text (without
                                  // margin) of line_next starts
      const unsigned lstart = loff + lrng.beg; // starting pos. in line
      const unsigned lend = line->length - 2;  // ending pos. in line
      const unsigned nlstart = lnme;           // starting pos in next line
      const unsigned nlend = lnme + (lrng.end - lrng.beg) -
                             (lend - lstart); // ending pos in next line
      // Add the link to line
      if (LT_MAN == type) {
        if (aprowhat_has(trgt, aw_all, aw_all_len))
          add_link(line, lstart, lend, true, nlstart, nlend, type, trgt);
      } else
        add_link(line, lstart, lend, true, nlstart, nlend, type, trgt);
    } else if (loff + lrng.end < line->length) {
      // Link is not broken by a hyphen

      // Add the link to line
      if (LT_MAN == type) {
        if (aprowhat_has(trgt, aw_all, aw_all_len))
          add_link(line, loff + lrng.beg, loff + lrng.end, //
                   false, 0, 0, type, trgt);
      } else
        add_link(line, loff + lrng.beg, loff + lrng.end, //
                 false, 0, 0, type, trgt);
    }

    // Calculate next offset
    loff += lrng.end;
    if (loff < line->length) {
      // Offset is not beyond the end of line; look for another link
      lrng = fr_search(re, &ltext[loff]);
    } else {
      // Offset is beyond the end of line; exit the loop
      lrng.beg = 0;
      lrng.end = 0;
    }
  }
}

// Helper of man(). If line is a section header, return its level. Otherwise,
// return -1. This function must initially be called with a NULL argument, every
// time man() is invoked.
bool section_header_level(line_t *line) {
  static unsigned lnme_set[] = {
      0, 0, 0, 0,
      0, 0, 0, 0}; // unique and (hopefully) ordered lnme values for all section
                   // headers encountered so far
  unsigned i;      // iterator

  // Line is NULL; re-initialize lnme_set and return -1
  if (NULL == line) {
    for (i = 0; i < 8; i++)
      lnme_set[i] = 0;

    return -1;
  }

  unsigned lnme = wmargend(
      line->text, NULL); // position in line's text where margin whitespace ends

  if (bget(line->bold, lnme) && bget(line->reg, line->length - 1)) {
    // Line is a section header; return the level that corresponds to its lnme
    for (i = 0; i < 8; i++) {
      if (0 == lnme_set[i])
        lnme_set[i] = lnme;
      if (lnme == lnme_set[i])
        return i;
    }
    return 7;
  } else {
    // Not a section header
    return -1;
  }
}

// The following are helpers of man()

// true if tmpw[i] contains a 'bold' terminal escape sequence
#define got_bold                                                               \
  ((i + 4 < len) && (tmpw[i] == L'\e') && (tmpw[i + 1] == L'[') &&             \
   (tmpw[i + 2] == L'1') && (tmpw[i + 3] == L'm'))

// true if tmpw[i] contains a 'bold' typewriter (NO_SGR) sequence
#define got_bold_nosgr                                                         \
  ((i + 3 < len) && (tmpw[i] == tmpw[i + 2]) && (tmpw[i + 1] == L'\b'))

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

// true if tmpw[i] contains a 'underline' typewriter (NO_SGR) sequence
#define got_uline_nosgr                                                        \
  ((i + 3 < len) && (tmpw[i] == L'_') && (tmpw[i + 1] == L'\b'))

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

// true if tmpw[i] contains an esape-]8 (link embedding) sequence
#define got_esc8                                                               \
  ((i + 4 < len) && (tmpw[i] == L'\e') && (tmpw[i + 1] == L']') &&             \
   (tmpw[i + 2] == L'8') && (tmpw[i + 3] == L';'))

// The following are helpers of man_toc()

// true if gline is a section header
#define got_sh                                                                 \
  ((glen >= 3) && (L'.' == gline[0]) &&                                        \
   (L'S' == gline[1] || L's' == gline[1]) &&                                   \
   (L'H' == gline[2] || L'h' == gline[2]))

// true if gline is a sub-section header
#define got_ss                                                                 \
  ((glen >= 3) && (L'.' == gline[0]) &&                                        \
   (L'S' == gline[1] || L's' == gline[1]) &&                                   \
   (L'S' == gline[2] || L's' == gline[2]))

// true if gline is a tagged paragraph
#define got_tp                                                                 \
  ((glen >= 3) && (L'.' == gline[0]) &&                                        \
   (L'T' == gline[1] || L't' == gline[1]) &&                                   \
   (L'P' == gline[2] || L'p' == gline[2]))

// true if gline is a command to output delayed text
#define got_pd                                                                 \
  ((glen >= 3) && (L'.' == gline[0]) &&                                        \
   (L'P' == gline[1] || L'p' == gline[1]) &&                                   \
   (L'D' == gline[2] || L'd' == gline[2]))

// true if a tag line is a comment
#define got_comment                                                            \
  ((glen > 2) && (gline[0] == L'.') && (gline[1] == L'\\') &&                  \
   ((gline[2] == L'\"' || gline[2] == L'#')))

// Components of got_trap and got_ok
#define got_b                                                                  \
  ((glen > 1) && (L'.' == gline[0]) && (L'B' == gline[1] || L'b' == gline[1]))
#define got_i                                                                  \
  ((glen > 1) && (L'.' == gline[0]) && (L'I' == gline[1] || L'i' == gline[1]))
#define got_sm                                                                 \
  ((glen > 2) && (L'.' == gline[0]) &&                                         \
   (L's' == gline[1] || L'S' == gline[1]) &&                                   \
   (L'm' == gline[2] || L'M' == gline[2]))
#define got_sb                                                                 \
  ((glen > 2) && (L'.' == gline[0]) &&                                         \
   (L's' == gline[1] || L'S' == gline[1]) &&                                   \
   (L'b' == gline[2] || L'B' == gline[2]))
#define got_bi                                                                 \
  ((glen > 2) && (L'.' == gline[0]) &&                                         \
   (L'b' == gline[1] || L'B' == gline[1]) &&                                   \
   (L'i' == gline[2] || L'I' == gline[2]))
#define got_br                                                                 \
  ((glen > 2) && (L'.' == gline[0]) &&                                         \
   (L'b' == gline[1] || L'B' == gline[1]) &&                                   \
   (L'r' == gline[2] || L'R' == gline[2]))
#define got_ib                                                                 \
  ((glen > 2) && (L'.' == gline[0]) &&                                         \
   (L'i' == gline[1] || L'I' == gline[1]) &&                                   \
   (L'b' == gline[2] || L'B' == gline[2]))
#define got_ir                                                                 \
  ((glen > 2) && (L'.' == gline[0]) &&                                         \
   (L'i' == gline[1] || L'I' == gline[1]) &&                                   \
   (L'r' == gline[2] || L'R' == gline[2]))
#define got_rb                                                                 \
  ((glen > 2) && (L'.' == gline[0]) &&                                         \
   (L'r' == gline[1] || L'R' == gline[1]) &&                                   \
   (L'b' == gline[2] || L'B' == gline[2]))
#define got_ri                                                                 \
  ((glen > 2) && (L'.' == gline[0]) &&                                         \
   (L'r' == gline[1] || L'R' == gline[1]) &&                                   \
   (L'i' == gline[2] || L'I' == gline[2]))

// true if a tag line starts with a formatting command that sets a trap
#define got_trap (got_b || got_i || got_sm || got_sb)

// true if a tag line atarts with an allowed formatting command
#define got_ok                                                                 \
  (got_b || got_i || got_sm || got_sb || got_bi || got_br || got_ib ||         \
   got_ir || got_rb || got_ri)

// Helper of man_toc(). Massage text member of every entry in toc (of size
// toc_len) with the groff command, in order to remove escaped characters, etc.
void tocgroff(toc_entry_t *toc, unsigned toc_len) {
  char *tpath;               // temporary file path
  char cmdstr[BS_LINE] = ""; // groff 'massage' command
  char texts[BS_LINE];       // 8-bit version of current toc text
  unsigned i;                // iterator

  // Prepare tpath and cmdstr
  tpath = xtempnam(NULL, "qman");
  snprintf(cmdstr, BS_LINE, "%s -man -rLL=1024m -Tutf8 %s 2>>/dev/null",
           config.misc.groff_path, tpath);

  // Prepare the environment
  unsetenv("GROFF_SGR");
  setenv("GROFF_NO_SGR", "1", true);

  // Write all texts of toc into temporary file
  FILE *fp = xfopen(tpath, "w");
  xfputs(".TH A A A A A", fp);
  xfputs(".ll 1024m\n", fp);
  for (i = 0; i < toc_len; i++)
    if (NULL != toc[i].text) {
      wcstombs(texts, toc[i].text, BS_LINE);
      xfputs(texts, fp);
      xfputs("\n.br 0\n", fp);
    }
  xfclose(fp);

  // Massage temporary file with groff and put the results back into the texts
  // of toc
  FILE *pp = xpopen(cmdstr, "r");
  xfgets(texts, BS_LINE, pp); // discarded
  xfgets(texts, BS_LINE, pp); // discarded
  for (i = 0; i < toc_len;) {
    xfgets(texts, BS_LINE, pp);
    mbstowcs(toc[i].text, texts, BS_LINE);
    wbs(toc[i].text);
    wmargtrim(toc[i].text, L"\n");

    // Discard empty line output
    if (L'\0' != toc[i].text[0])
      i++;

    // Failsafe: exit loop on EOF
    if (feof(pp))
      break;
  }
  xpclose(pp);

  // Uncomment the following to log output into qman.log for debugging
  // logprintf("tocgroff()");
  // for (i = 0; i < toc_len; i++)
  //   logprintf("%d: type=%d text=%ls", i, toc[i].type, toc[i].text);

  // Tidy up and restore the environment
  unlink(tpath);
  free(tpath);
}

// Helper of man_sections(). Massage every entry in sections (of size
// sections_len) with the groff command, in order to remove escaped characters,
// etc.
void secgroff(wchar_t **sections, unsigned sections_len) {
  char *tpath;               // temporary file path
  char cmdstr[BS_LINE] = ""; // groff 'massage' command
  char texts[BS_LINE];       // 8-bit version of current toc text
  unsigned i;                // iterator

  // Prepare tpath and cmdstr
  tpath = xtempnam(NULL, "qman");
  snprintf(cmdstr, BS_LINE, "%s -man -rLL=1024m -Tutf8 %s 2>>/dev/null",
           config.misc.groff_path, tpath);

  // Prepare the environment
  unsetenv("GROFF_SGR");
  setenv("GROFF_NO_SGR", "1", true);

  // Write all sections into temporary file
  FILE *fp = xfopen(tpath, "w");
  xfputs(".TH A A A A A", fp);
  xfputs(".ll 1024m\n", fp);
  for (i = 0; i < sections_len; i++)
    if (NULL != sections[i]) {
      wcstombs(texts, sections[i], BS_LINE);
      xfputs(texts, fp);
      xfputs("\n.br 0\n", fp);
    }
  xfclose(fp);

  // Massage temporary file with groff and put the results back into the texts
  // of toc
  FILE *pp = xpopen(cmdstr, "r");
  xfgets(texts, BS_LINE, pp); // discarded
  xfgets(texts, BS_LINE, pp); // discarded
  for (i = 0; i < sections_len;) {
    xfgets(texts, BS_LINE, pp);
    mbstowcs(sections[i], texts, BS_LINE);
    wbs(sections[i]);
    wmargtrim(sections[i], L"\n");

    // Discard empty line output
    if (L'\0' != sections[i][0])
      i++;

    // Failsafe: exit loop on EOF
    if (feof(pp))
      break;
  }
  xpclose(pp);

  // Tidy up and restore the environment
  unlink(tpath);
  free(tpath);
}

//
// Functions
//

void init() {
  // Use the system locale
  setlocale(LC_ALL, "");

  // Initialize configuration to sane defaults
  conf_init();

  // Initialize history
  history_cur = 0;
  history_top = 0;
  history = aalloc(config.misc.history_size, request_t);
  history_replace(RT_NONE, NULL);

  // Initialize aw_all and sc_all
  aw_all_len = aprowhat_exec(&aw_all, AW_APROPOS, L"''");
  sc_all_len = aprowhat_sections(&sc_all, aw_all, aw_all_len);

  // Initialize page_title
  wcscpy(page_title, L"");

  // initialize regular expressions
  fr_init(&re_man, "[a-zA-Z0-9\\.:@_-]+\\([a-zA-Z0-9]+\\)", L")");
  fr_init(&re_http,
          "https?:\\/\\/[a-zA-Z0-9\\.\\/\\?\\+:@_#&%=~-]*[a-zA-Z0-9\\/"
          "\\?\\+:@_#&%=~-]",
          L"http");
  fr_init(
      &re_email,
      "[a-zA-Z0-9\\.\\$\\*\\+\\?\\^\\|!#%&'/=_`{}~-][a-zA-Z0-9\\.\\$\\*\\+\\/"
      "\\?\\^\\|\\.!#%&'=_`{}~-]*@[a-zA-Z0-9-][a-zA-Z0-9-]+\\.[a-zA-Z0-9-][a-"
      "zA-Z0-9\\.-]+[a-zA-Z0-9-]",
      L"@");
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
    int cur = getopt_long(argc, argv, optstring, longopts, &cur_i);
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
    case 'K':
      // -k or --global-apropos was passed; make sure it will be passed on to
      // man
      config.misc.global_apropos = true;
      break;
    case 'a':
      // -a or --all was passed; make sure it will be passed on to man
      config.misc.global_whatis = true;
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
    case 'V':
      // -v or --version was passed; print program version and exit
      version();
      free(longopts);
      winddown(ES_SUCCESS, NULL);
      break;
    case 'h':
      // -h or --help was passed; print usage and exit
      usage();
      free(longopts);
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

  // If the user hasn't asked for a specific request type...
  if (RT_NONE == history[history_cur].request_type) {
    if (0 == argc) {
      // ...and hasn't provided any arguments, show the index page
      history_replace(RT_INDEX, NULL);
    } else {
      // ...but has provided arguments, try to show the manual page that
      // corresponds to said arguments
      history_replace(RT_MAN, NULL);
    }
  }

  // If we are showing a manual, apropos, whatis, or local page...
  if (history[history_cur].request_type > RT_INDEX) {
    // ...but the user hasn't specified an argument...
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

    wcscpy(tmp, L"");
    tmp_len = 0;

    if (history[history_cur].request_type == RT_MAN && !config.layout.tui) {
      // If we are showing a manual page, we are in CLI mode...
      if (config.misc.global_apropos) {
        // ...and the user has requested global apropos, add '-K' to tmp
        wcscpy(tmp, L"-K ");
        tmp_len = 3;
      } else if (config.misc.global_whatis) {
        // ...and the user has requested global whatis, add '-a' to tmp
        wcscpy(tmp, L"-a ");
        tmp_len = 3;
      }
    }

    // Surround all members of argv with single quotes, and flatten them into
    // the tmp string
    for (i = 0; i < argc; i++) {
      swprintf(tmp2, BS_SHORT, L"'%s'", argv[i]);
      if (tmp_len + wcslen(tmp2) < BS_LINE) {
        wcscat(tmp, tmp2);
        tmp_len += wcslen(tmp2);
      }
      if (i < argc - 1 && tmp_len + 4 < BS_LINE) {
        wcscat(tmp, L" ");
        tmp_len++;
      }
    }

    // Set history[history_cur].args to tmp
    history_replace(history[history_cur].request_type, tmp);
  }
}

void version() { wprintf(L"%ls\n", config.misc.program_version); }

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

void history_replace(request_type_t rt, const wchar_t *args) {
  history[history_cur].request_type = rt;

  if (NULL != history[history_cur].args)
    free(history[history_cur].args);

  if (NULL == args)
    history[history_cur].args = NULL;
  else {
    history[history_cur].args = wcsdup(args);
  }

  history[history_cur].top = 0;
  history[history_cur].left = 0;
  history[history_cur].flink = (link_loc_t){false, 0, 0};
}

void history_push(request_type_t rt, const wchar_t *args) {
  unsigned i;

  // Save user's position
  history[history_cur].top = page_top;
  history[history_cur].left = page_left;
  history[history_cur].flink = page_flink;

  // Increase history_cur
  history_cur++;

  // If we're pushing in the middle of the history stack, all subsequent
  // history entries are lost, and we must free any memory used by their args
  if (history_top > history_cur)
    for (i = history_cur + 1; i <= history_top; i++)
      if (NULL != history[i].args) {
        free(history[i].args);
        history[i].args = NULL;
      }

  // Make history_top equal to history_cur
  history_top = history_cur;

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

bool history_jump(int pos) {
  history[history_cur].top = page_top;
  history[history_cur].left = page_left;
  history[history_cur].flink = page_flink;

  if (pos >= 0 && pos <= history_top) {
    history_cur = pos;
    page_top = history[history_cur].top;
    page_left = history[history_cur].left;
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
  char cmdstr[BS_LINE];
  if (AW_WHATIS == cmd)
    snprintf(cmdstr, BS_LINE, "%s -l %ls 2>>/dev/null", config.misc.whatis_path,
             args);
  else
    snprintf(cmdstr, BS_LINE, "%s -l %ls 2>>/dev/null",
             config.misc.apropos_path, args);

  // Execute apropos, and enter its result into a temporary file. lines
  // becomes the total number of lines copied.
  FILE *pp = xpopen(cmdstr, "r");
  FILE *fp = xtmpfile();
  const unsigned lines = scopylines(pp, fp);
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
    if (-1 == sreadline(line, BS_LINE, fp))
      winddown(ES_OPER_ERROR, L"Malformed temporary apropos/whatis file");

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
    if (!wmemberof((const wchar_t **)res, aw[i].section, res_i)) {
      res[res_i] = wcsdup(aw[i].section);
      res_i++;
    }
  }

  wsort(res, res_i, false);

  *dst = res;
  return res_i;
}

unsigned aprowhat_render(line_t **dst, const aprowhat_t *aw,
                         const unsigned aw_len, const wchar_t *const *sc,
                         const unsigned sc_len, const wchar_t *key,
                         const wchar_t *title, const wchar_t *ver,
                         const wchar_t *date) {
  // Text blocks widths
  const unsigned line_width = MAX(60, config.layout.main_width);
  const unsigned lmargin_width = config.layout.lmargin; // left margin
  const unsigned rmargin_width = config.layout.rmargin; // right margin
  const unsigned text_width =
      line_width - lmargin_width - rmargin_width; // main text area
  const unsigned hfc_width =
      text_width / 2 + text_width % 2; // header/footer centre area
  const unsigned hfl_width =
      (text_width - hfc_width) / 2; // header/footer left area
  const unsigned hfr_width =
      hfl_width + (text_width - hfc_width) % 2; // header/footer right area

  unsigned ln = 0;      // current line number
  unsigned i, j;        // iterators
  wchar_t tmp[BS_LINE]; // temporary
  memset(tmp, 0, sizeof(wchar_t) * BS_LINE);

  unsigned res_len = BS_LINE;            // result buffer length
  line_t *res = aalloc(res_len, line_t); // result buffer

  // Header
  line_alloc(res[ln], 0);
  inc_ln;
  line_alloc(res[ln], line_width);
  const unsigned title_len = wcslen(title); // title length
  const unsigned key_len = wcslen(key);     // key length
  const unsigned lts_len =
      (hfc_width - title_len) / 2 +
      (hfc_width - title_len) % 2; // length of space on the left of title
  const unsigned rts_len =
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

  // Only if list of sections is enabled
  if (config.layout.sections_on_top) {
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
    const unsigned sc_maxwidth = MIN(
        text_width / 2 - 4, wmaxlen(sc, sc_len)); // length of longest section
    const unsigned sc_cols =
        text_width / (4 + sc_maxwidth); // number of columns for sections
    const unsigned sc_lines =
        sc_len % sc_cols > 0
            ? 1 + sc_len / sc_cols
            : MAX(1, sc_len / sc_cols); // number of lines for sections
    unsigned sc_i;                      // index of current section
    for (i = 0; i < sc_lines; i++) {
      inc_ln;
      line_alloc(res[ln], line_width + 4); // +4 for section margin
      swprintf(res[ln].text, line_width + 1, L"%*s", lmargin_width, "");
      for (j = 0; j < sc_cols; j++) {
        sc_i = sc_cols * i + j;
        if (sc_i < sc_len) {
          swprintf(tmp, sc_maxwidth + 5, L" %-*ls", sc_maxwidth + 3, sc[sc_i]);
          wcscat(res[ln].text, tmp);
          swprintf(tmp, BS_LINE, L"MANUAL PAGES IN SECTION '%ls'", sc[sc_i]);
          add_link(&res[ln], lmargin_width + j * (sc_maxwidth + 4) + 1,
                   lmargin_width + j * (sc_maxwidth + 4) +
                       MIN(sc_maxwidth + 3, wcslen(sc[sc_i]) + 1),
                   false, 0, 0, LT_LS, tmp);
        }
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
        const unsigned lc_width = text_width / 3;        // left column width
        const unsigned rc_width = text_width - lc_width; // right column width
        const unsigned page_width = wcslen(aw[j].page) + wcslen(aw[j].section) +
                                    2; // width of manual page name and section
        const unsigned spcl_width =
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
                 false, 0, 0, LT_MAN, aw[j].ident);

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
  const unsigned date_len = wcslen(date); // date length
  const unsigned lds_len =
      (hfc_width - date_len) / 2 +
      (hfc_width - date_len) % 2; // length of space on the left of date
  const unsigned rds_len =
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

  if (NULL == needle)
    return -1;

  for (i = pos; i < hayst_len; i++)
    if (wcsstr(hayst[i].ident, needle) == hayst[i].ident)
      return i;

  return -1;
}

bool aprowhat_has(const wchar_t *needle, const aprowhat_t *hayst,
                  unsigned hayst_len) {
  unsigned i;

  if (NULL == needle)
    return false;

  for (i = 0; i < hayst_len; i++)
    if (0 == wcscasecmp(hayst[i].ident, needle))
      return true;

  return false;
}

unsigned man_sections(wchar_t ***dst, const wchar_t *args, bool local_file) {
  char gpath[BS_LINE];    // path to Groff document for manual page
  int glen;               // length of current line in Groff document
  wchar_t gline[BS_LINE]; // current line in Groff document
  unsigned en = 0;        // current entry in TOC
  char tmp[BS_LINE];      // temporary

  unsigned res_len = BS_SHORT;                // result buffer length
  wchar_t **res = aalloc(res_len, wchar_t *); // result buffer

  // Use GNU man to figure out gpath
  char cmdstr[BS_LINE];
  if (local_file)
    snprintf(cmdstr, BS_LINE,
             "%s --warnings='!all' --path --local-file %ls 2>>/dev/null",
             config.misc.man_path, args);
  else {
    snprintf(cmdstr, BS_LINE, "%s --warnings='!all' --path %ls 2>>/dev/null",
             config.misc.man_path, args);
  }
  FILE *pp = xpopen(cmdstr, "r");
  if (-1 == sreadline(gpath, BS_LINE, pp))
    winddown(ES_CHILD_ERROR, L"GNU man returned invalid output");
  xpclose(pp);

  // Open gpath
  archive_t gp = aropen(gpath);

  // For each line in gpath, gline...
  argets(gp, tmp, BS_LINE);
  while (!areof(gp)) {
    glen = mbstowcs(gline, tmp, BS_LINE);

    if (-1 == glen)
      winddown(ES_OPER_ERROR, L"Failed to read compressed manual page source");

    // If line is a section heading, add the corresponding data to res
    if (got_sh) {
      // Section heading
      unsigned textsp = wmargend(&gline[3], L"\"");
      if (textsp > 0) {
        res[en] = walloc(BS_LINE);
        wcscpy(res[en], &gline[3 + textsp]);
        wmargtrim(res[en], L"\"");
        inc_en;
      }
    }

    argets(gp, tmp, BS_LINE);
  }

  arclose(gp);

  secgroff(res, en);
  *dst = res;
  return en;
}

unsigned index_page(line_t **dst) {
  wchar_t key[] = L"INDEX";
  wchar_t title[] = L"All Manual Pages";
  time_t now = time(NULL);
  wchar_t date[BS_SHORT];
  wcsftime(date, BS_SHORT, L"%x", gmtime(&now));

  line_t *res;
  unsigned res_len = aprowhat_render(&res, aw_all, aw_all_len,
                                     (const wchar_t **)sc_all, sc_all_len, key,
                                     title, config.misc.program_version, date);

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
  unsigned res_len =
      aprowhat_render(&res, aw, aw_len, (const wchar_t **)sc, sc_len, key,
                      title, config.misc.program_version, date);

  aprowhat_free(aw, aw_len);
  wafree(sc, sc_len);

  *dst = res;
  return res_len;
}

unsigned man(line_t **dst, const wchar_t *args, bool local_file) {
  // Text blocks widths
  const unsigned line_width = MAX(60, config.layout.main_width);
  const unsigned lmargin_width = config.layout.lmargin; // left margin
  const unsigned rmargin_width = config.layout.rmargin; // right margin
  const unsigned text_width =
      line_width - lmargin_width - rmargin_width; // main text area

  unsigned ln = 0;                 // current line number
  int len;                         // length of current line text
  unsigned i, j;                   // iterators
  wchar_t *tmpw = walloc(BS_LINE); // temporary
  char *tmps = salloc(BS_LINE);    // temporary

  bool ilink = false;   // we are inside an embedded HTTP link
  unsigned ilink_ln;    // embedded link line
  int ilink_start;      // embedded link start position
  int ilink_end;        // embedded link end position
  int ilink_start_next; // embedded link start position (in next line, for
                        // hyphenated links)
  int ilink_end_next;   // embedded link end position (in next line, for
                        // hypehnated links)
  wchar_t ilink_trgt[BS_LINE]; // embedded link URL

  unsigned res_len = BS_LINE;            // result buffer length
  line_t *res = aalloc(res_len, line_t); // result buffer

  // Set up the environment for man to create its output as we want it
  char *old_term = getenv("TERM");
  setenv("TERM", "xterm", true);
  sprintf(tmps, "%d", 1 + text_width);
  setenv("MANWIDTH", tmps, true);
  sprintf(tmps, "%s %s", config.misc.hyphenate ? "" : "--nh",
          config.misc.justify ? "" : "--nj");
  setenv("MANOPT", tmps, true);
  setenv("MAN_KEEP_FORMATTING", "1", true);
  setenv("MANROFFOPT", "", true);
  setenv("GROFF_SGR", "1", true);
  unsetenv("GROFF_NO_SGR");

  // Prepare man command
  char cmdstr[BS_LINE];
  if (local_file)
    snprintf(cmdstr, BS_LINE,
             "%s --warnings='!all' --local-file %ls 2>>/dev/null",
             config.misc.man_path, args);
  else
    snprintf(cmdstr, BS_LINE, "%s --warnings='!all' %ls 2>>/dev/null",
             config.misc.man_path, args);

  // Execute man
  FILE *pp = xpopen(cmdstr, "r");

  section_header_level(NULL);

  // Discard any empty lines on top, and read the first non-empty line into
  // tmps/tmpw
  xfgets(tmps, BS_LINE, pp);
  len = mbstowcs(tmpw, tmps, BS_LINE);
  while (0 == len || L'\n' == tmpw[wmargend(tmpw, L"\n")]) {
    xfgets(tmps, BS_LINE, pp);
    len = mbstowcs(tmpw, tmps, BS_LINE);
  }

  // For each line of man's output (read into tmps/tmpw)...
  while (!feof(pp)) {
    // At line 1, insert the list of sections (if enabled)
    if (config.layout.sections_on_top && 1 == ln) {
      // Newline
      line_alloc(res[ln], 0);

      // Section title for sections
      inc_ln;
      line_alloc(res[ln], line_width);
      wcscpy(tmpw, L"SECTIONS");
      swprintf(res[ln].text, line_width + 1, L"%*s%-*ls", //
               lmargin_width, "",                         //
               text_width, tmpw);
      bset(res[ln].bold, lmargin_width);
      bset(res[ln].reg, lmargin_width + wcslen(tmpw));

      // Sections
      wchar_t **sc;                                          // sections
      unsigned sc_len = man_sections(&sc, args, local_file); // no. of sections
      const unsigned sc_maxwidth =
          MIN(text_width / 2 - 4, wmaxlen((const wchar_t *const *)sc,
                                          sc_len)); // length of longest section
      const unsigned sc_cols =
          text_width / (4 + sc_maxwidth); // number of columns for sections
      const unsigned sc_lines =
          sc_len % sc_cols > 0
              ? 1 + sc_len / sc_cols
              : MAX(1, sc_len / sc_cols); // number of lines for sections
      unsigned sc_i;                      // index of current section
      for (i = 0; i < sc_lines; i++) {
        inc_ln;
        line_alloc(res[ln], line_width + 4); // +4 for section margin
        swprintf(res[ln].text, line_width + 1, L"%*s", lmargin_width, "");
        for (j = 0; j < sc_cols; j++) {
          sc_i = sc_cols * i + j;
          if (sc_i < sc_len) {
            swprintf(tmpw, sc_maxwidth + 5, L" %-*ls", sc_maxwidth + 3,
                     sc[sc_i]);
            wcslower(tmpw);
            wcscat(res[ln].text, tmpw);
            add_link(&res[ln], lmargin_width + j * (sc_maxwidth + 4) + 1,
                     lmargin_width + j * (sc_maxwidth + 4) +
                         MIN(sc_maxwidth + 3, wcslen(sc[sc_i])) + 1,
                     false, 0, 0, LT_LS, sc[sc_i]);
          }
        }
      }
      inc_ln;

      wafree(sc, sc_len);
      len = mbstowcs(tmpw, tmps, BS_LINE);
    }

    if (-1 == len)
      winddown(ES_CHILD_ERROR, L"GNU man returned invalid output");

    // Allocate memory for a new line in res
    line_alloc(res[ln], config.layout.lmargin + len + 1);

    // Add spaces for left margin
    for (j = 0; j < config.layout.lmargin; j++)
      res[ln].text[j] = L' ';

    // Read the contents of tmpw one character at a time, and build the line's
    // text, reg, bold, italic, and uline members
    bool bold_nosgr = false;  // a 'bold' typewriter sequence has been seen
    bool uline_nosgr = false; // a 'underline' typewriter sequence has been seen
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
      } else if (bold_nosgr && !got_bold_nosgr) {
        bset(res[ln].reg, j);
        bold_nosgr = false;
        res[ln].text[j] = tmpw[i];
        j++;
      } else if (got_bold_nosgr) {
        if (!bold_nosgr)
          bset(res[ln].bold, j);
        bold_nosgr = true;
        i += 2;
        res[ln].text[j] = tmpw[i];
        j++;
      } else if (got_italic) {
        bset(res[ln].italic, j);
        i += 3;
      } else if (got_uline) {
        bset(res[ln].uline, j);
        i += 3;
      } else if (uline_nosgr && !got_uline_nosgr) {
        bset(res[ln].reg, j);
        uline_nosgr = false;
        res[ln].text[j] = tmpw[i];
        j++;
      } else if (got_uline_nosgr) {
        if (!uline_nosgr)
          bset(res[ln].uline, j);
        uline_nosgr = true;
        i += 2;
        res[ln].text[j] = tmpw[i];
        j++;
      } else if (got_esc8) {
        i += 3;
        if (ilink) {
          if (ilink_ln == ln) {
            ilink_end = j;
            if (ilink_end > ilink_start)
              add_link(&res[ln], ilink_start, j, false, 0, 0, LT_HTTP,
                       ilink_trgt);
          } else if (ln > 0) {
            ilink_end = res[ln - 1].length - 1;
            ilink_start_next = wmargend(res[ln].text, NULL);
            ilink_end_next = j;
            if (ilink_end > ilink_start && ilink_end_next > ilink_start_next)
              add_link(&res[ln - 1], ilink_start, ilink_end, true,
                       ilink_start_next, ilink_end_next, LT_HTTP, ilink_trgt);
          }
          ilink = false;
        } else {
          if (tmpw[i] == L';' && tmpw[i + 1] == L';') {
            i += 2;
            unsigned k = 0;
            while (!(tmpw[i] == L'\e' && tmpw[i + 1] == L'\\')) {
              ilink_trgt[k] = tmpw[i];
              i++;
              k++;
            }
            ilink_trgt[k] = L'\0';
            i += 1;
            ilink = true;
            ilink_ln = ln;
            ilink_start = j;
          }
        }
        while (i < len && !(tmpw[i - 1] == L'\e' && tmpw[i] == L'\\'))
          i++;
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

    // Insert the obligatory 0 byte at the end of the line's text, and set its
    // exact length
    res[ln].text[j] = L'\0';
    res[ln].length = j + 1;

    // Read next line of man output into tmps/tmpw
    xfgets(tmps, BS_LINE, pp);
    len = mbstowcs(tmpw, tmps, BS_LINE);

    inc_ln;
  }

  // Restore the environment
  if (NULL != old_term)
    setenv("TERM", old_term, true);

  xpclose(pp);
  free(tmpw);
  free(tmps);

  // Discover and add links (skipping the first two lines, and the last line)
  if (ln >= 2) {
    for (unsigned i = 2; i < ln - 1; i++) {
      discover_links(&re_man, &res[i], &res[i + 1], LT_MAN);
      discover_links(&re_http, &res[i], &res[i + 1], LT_HTTP);
      discover_links(&re_email, &res[i], &res[i + 1], LT_EMAIL);
    }
  }

  // If no results were returned by man, set err and err_msg
  err = false;
  if (0 == ln) {
    err = true;
    swprintf(err_msg, BS_LINE, L"No manual page for %ls", args);
  }

  *dst = res;
  return ln;
}

unsigned man_toc(toc_entry_t **dst, const wchar_t *args, bool local_file) {
  char gpath[BS_LINE];    // path to Groff document for manual page
  int glen;               // length of current line in Groff document
  wchar_t gline[BS_LINE]; // current line in Groff document
  unsigned en = 0;        // current entry in TOC
  bool sh_seen = false;   // whether a section header has been seen
  char tmp[BS_LINE];      // temporary
  unsigned textsp; // real beginning of gline's text (ignoring whitespace)

  unsigned res_len = BS_LINE;                      // result buffer length
  toc_entry_t *res = aalloc(res_len, toc_entry_t); // result buffer

  // Section header for the list of sections (if enabled)
  if (config.layout.sections_on_top) {
    res[en].type = TT_HEAD;
    res[en].text = walloc(BS_LINE);
    wcscpy(res[en].text, L"SECTIONS");
    inc_en;
  }

  // Use GNU man to figure out gpath
  char cmdstr[BS_LINE];
  if (local_file)
    snprintf(cmdstr, BS_LINE,
             "%s --warnings='!all' --path --local-file %ls 2>>/dev/null",
             config.misc.man_path, args);
  else {
    snprintf(cmdstr, BS_LINE, "%s --warnings='!all' --path %ls 2>>/dev/null",
             config.misc.man_path, args);
  }
  FILE *pp = xpopen(cmdstr, "r");
  if (-1 == sreadline(gpath, BS_LINE, pp))
    winddown(ES_CHILD_ERROR, L"GNU man returned invalid output");
  xpclose(pp);

  // Open gpath
  archive_t gp = aropen(gpath);

  // For each line in gpath, gline...
  argets(gp, tmp, BS_LINE);
  while (!areof(gp)) {
    glen = mbstowcs(gline, tmp, BS_LINE);

    if (-1 == glen)
      winddown(ES_OPER_ERROR, L"Failed to read compressed manual page source");

    // If line can be a TOC entry, add the corresponding data to res
    if (got_sh) {
      // Section heading
      res[en].type = TT_HEAD;
      textsp = wmargend(&gline[3], L"\"");
      if (textsp > 0) {
        res[en].text = walloc(BS_LINE);
        wcscpy(res[en].text, &gline[3 + textsp]);
        wmargtrim(res[en].text, L"\"");
        inc_en;
        sh_seen = true;
      }
    } else if (got_ss && sh_seen) {
      // Subsection heading
      res[en].type = TT_SUBHEAD;
      textsp = wmargend(&gline[3], L"\"");
      if (textsp > 0) {
        res[en].text = walloc(BS_LINE);
        wcscpy(res[en].text, &gline[3 + textsp]);
        wmargtrim(res[en].text, L"\"");
        inc_en;
      }
    } else if (got_tp && sh_seen) {
      // Tagged paragraph
      argets(gp, tmp, BS_LINE);
      if (!areof(gp)) {
        glen = mbstowcs(gline, tmp, BS_LINE);
        {
          // Edge case: the tag line contains only a comment or a line that must
          // otherwise be skipped; skip to next line
          while (got_comment || got_tp || got_pd) {
            argets(gp, tmp, BS_LINE);
            if (areof(gp))
              break;
            glen = mbstowcs(gline, tmp, BS_LINE);
          }
        }
        {
          // Edge case: the tag line starts with a formatting command that sets
          // a trap for the next line; skip to the next line
          while (got_trap && wmargtrim(gline, NULL) < 4) {
            argets(gp, tmp, BS_LINE);
            if (areof(gp))
              break;
            glen = mbstowcs(gline, tmp, BS_LINE);
          }
          if (areof(gp))
            break;
        }
        {
          // Edge case: the tag line starts with a non-acceptable command;
          // ignore it
          if (L'.' == gline[0] && (!got_ok))
            continue;
        }
        textsp = wmargend(gline, NULL);
        res[en].type = TT_TAGPAR;
        res[en].text = walloc(BS_LINE);
        wcscpy(res[en].text, &gline[textsp]);
        glen = wmargtrim(res[en].text, L"\n");
        {
          // Edge case: there's a line escape at the end of the tag line; remove
          // it
          if (glen >= 1 && L'\\' == res[en].text[glen - 1])
            res[en].text[glen - 1] = L'\0';
          if (glen >= 2 && L'\\' == res[en].text[glen - 2] &&
              L'c' == res[en].text[glen - 1]) {
            res[en].text[glen - 2] = L'\0';
            res[en].text[glen - 1] = L'\0';
          }
        }
        inc_en;
      }
    }

    argets(gp, tmp, BS_LINE);
  }

  arclose(gp);

  // Uncomment the following to log output into qman.log for debugging
  // logprintf("man_toc()");
  // for (unsigned i = 0; i < en; i++)
  //   logprintf("%d: type=%d text=%ls", i, res[i].type, res[i].text);

  tocgroff(res, en);
  *dst = res;
  return en;
}

unsigned sc_toc(toc_entry_t **dst, const wchar_t *const *sc,
                const unsigned sc_len) {
  unsigned i;      // iterator
  unsigned en = 0; // current entry in TOC

  unsigned res_len = BS_SHORT;                     // result buffer length
  toc_entry_t *res = aalloc(res_len, toc_entry_t); // result buffer

  // Section header for the list of sections (if enabled)
  if (config.layout.sections_on_top) {
    res[en].type = TT_HEAD;
    res[en].text = walloc(BS_LINE);
    wcscpy(res[en].text, L"SECTIONS");
    inc_en;
  }

  // All other other section headers
  for (i = 0; i < sc_len; i++) {
    res[en].type = TT_HEAD;
    res[en].text = walloc(BS_LINE);
    swprintf(res[en].text, BS_LINE, L"MANUAL PAGES IN SECTION '%ls'", sc[i]);
    inc_en;
  }

  *dst = res;
  return en;
}

link_loc_t prev_link(const line_t *lines, unsigned lines_len,
                     link_loc_t start) {
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

link_loc_t next_link(const line_t *lines, unsigned lines_len,
                     link_loc_t start) {
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

link_loc_t first_link(const line_t *lines, unsigned lines_len, unsigned start,
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

link_loc_t last_link(const line_t *lines, unsigned lines_len, unsigned start,
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

unsigned search(result_t **dst, const wchar_t *needle, const line_t *lines,
                unsigned lines_len, bool cs) {
  unsigned ln;                                // current line no.
  unsigned i = 0;                             // current result no.
  const unsigned needle_len = wcslen(needle); // length of needle
  wchar_t *cur_hayst;         // current haystuck (i.e. text of current line)
  wchar_t *hit = NULL;        // current return value of wcscasestr()
  unsigned res_len = BS_LINE; // result buffer length
  result_t *res = aalloc(res_len, result_t); // result buffer

  // For each line...
  for (ln = 0; ln < lines_len; ln++) {
    // Start at the beginning of the line's text
    cur_hayst = lines[ln].text;
    // Search for needle
    if (cs)
      hit = wcscasestr(cur_hayst, needle);
    else
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
        if (cs)
          hit = wcscasestr(cur_hayst, needle);
        else
          hit = wcsstr(cur_hayst, needle);
      else
        hit = NULL;
      // Increment i (and reallocate memory if necessary)
      inc_i;
    }
  }

  // If no results were found, free the result buffer
  if (0 == i)
    free(res);

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
  int i;

  for (i = res_len - 1; i >= 0; i--)
    if (res[i].line <= from)
      return res[i].line;

  return -1;
}

extern unsigned get_mark(wchar_t **dst, mark_t mark, const line_t *lines,
                         unsigned lines_len) {
  // Return if no text is marked
  if (!mark.enabled) {
    *dst = NULL;
    return 0;
  }

  wchar_t *res = walloc(BS_LINE * config.layout.height); // return value
  memset(res, 0, sizeof(wchar_t) * BS_LINE * config.layout.height);
  wchar_t tmp[BS_LINE]; // temporary
  unsigned ln;          // current line number

  // Generate return value
  if (mark.start_line == mark.end_line) {
    // Marked text is in a single line
    wcsncpy(res, &lines[mark.start_line].text[mark.start_char],
            1 + mark.end_char - mark.start_char);
  } else {
    // Marked text is in multiple lines
    for (ln = mark.start_line; ln <= mark.end_line; ln++) {
      if (ln == mark.start_line) {
        // First line; append text from start_char to end of line
        wcscat(res, &lines[ln].text[mark.start_char]);
      } else if (ln == mark.end_line) {
        // Last line; append text from beginning of line to end_char
        wcsncpy(tmp, lines[ln].text, 1 + mark.end_char);
        tmp[1 + mark.end_char] = L'\0';
        wcscat(res, tmp);
      } else {
        // Intermediary lines; append entire line text
        wcscat(res, lines[ln].text);
      }
    }
  }

  *dst = res;
  return wcslen(res);
}

void populate_page() {
  // If page is already populated, free its allocated memory
  if (NULL != page && page_len > 0) {
    lines_free(page, page_len);
    page = NULL;
    page_len = 0;
  }

  // Reset TOC
  if (NULL != toc && toc_len > 0)
    toc_free(toc, toc_len);
  toc = NULL;
  toc_len = 0;

  // Populate page according to the request type of history[history_cur]
  switch (history[history_cur].request_type) {
  case RT_INDEX:
    wcscpy(page_title, L"All Manual Pages");
    entitle(page_title);
    page_len = index_page(&page);
    break;
  case RT_MAN:
    swprintf(page_title, BS_SHORT, L"Manual page(s) for: %ls",
             history[history_cur].args);
    entitle(page_title);
    page_len = man(&page, history[history_cur].args, false);
    break;
  case RT_MAN_LOCAL:
    swprintf(page_title, BS_SHORT, L"Manual page in local file(s): %ls",
             history[history_cur].args);
    entitle(page_title);
    page_len = man(&page, history[history_cur].args, true);
    break;
  case RT_APROPOS:
    swprintf(page_title, BS_SHORT, L"Apropos for: %ls",
             history[history_cur].args);
    entitle(page_title);
    page_len = aprowhat(&page, AW_APROPOS, history[history_cur].args,
                        L"APROPOS", page_title);
    break;
  case RT_WHATIS:
    swprintf(page_title, BS_SHORT, L"Whatis for: %ls",
             history[history_cur].args);
    entitle(page_title);
    page_len = aprowhat(&page, AW_WHATIS, history[history_cur].args, L"WHATIS",
                        page_title);
    break;
  default:
    winddown(ES_OPER_ERROR, L"Unexpected program request");
  }

  // Reset search results
  if (NULL != results && results_len > 0)
    free(results);
  results = NULL;
  results_len = 0;
}

void populate_toc() {
  // If the TOC doesn't exist yet, use man_toc() or sc_toc() to generate it now
  if (NULL == toc || 0 == toc_len) {
    request_type_t rt =
        history[history_cur].request_type;     // current request type
    wchar_t *args = history[history_cur].args; // arguments for current request
    aprowhat_t *aw;                            // temporary
    unsigned aw_len;                           // "
    wchar_t **sc;                              // "
    unsigned sc_len;                           // "

    switch (rt) {
    case RT_INDEX:
      toc_len = sc_toc(&toc, (const wchar_t *const *)sc_all, sc_all_len);
      break;
    case RT_MAN:
      toc_len = man_toc(&toc, args, false);
      break;
    case RT_MAN_LOCAL:
      toc_len = man_toc(&toc, args, true);
      break;
    case RT_APROPOS:
      aw_len = aprowhat_exec(&aw, AW_APROPOS, args);
      if (err)
        winddown(ES_OPER_ERROR, err_msg);
      sc_len = aprowhat_sections(&sc, aw, aw_len);
      toc_len = sc_toc(&toc, (const wchar_t *const *)sc, sc_len);
      if (NULL != aw && aw_len > 0)
        aprowhat_free(aw, aw_len);
      if (NULL != sc && sc_len > 0)
        wafree(sc, sc_len);
      break;
    default:
      aw_len = aprowhat_exec(&aw, AW_WHATIS, args);
      if (err)
        winddown(ES_OPER_ERROR, err_msg);
      sc_len = aprowhat_sections(&sc, aw, aw_len);
      toc_len = sc_toc(&toc, (const wchar_t *const *)sc, sc_len);
      if (NULL != aw && aw_len > 0)
        aprowhat_free(aw, aw_len);
      if (NULL != sc && sc_len > 0)
        wafree(sc, sc_len);
      break;
    }
  }

  // If the TOC still doesn't exist, something must have gone wrong
  if (0 == toc_len || NULL == toc)
    winddown(ES_OPER_ERROR, L"Unable to generate table of contents");
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

void toc_free(toc_entry_t *toc, unsigned toc_len) {
  unsigned i;

  for (i = 0; i < toc_len; i++) {
    if (NULL != toc[i].text)
      free(toc[i].text);
  }

  free(toc);
}

void winddown(int ec, const wchar_t *em) {
  // Shut ncurses down
  winddown_tui();

  // Deallocate memory used by base64
  base64_cleanup();

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
  if (NULL != config.chars.arrow_up)
    free(config.chars.arrow_up);
  if (NULL != config.chars.arrow_down)
    free(config.chars.arrow_down);
  if (NULL != config.misc.program_version)
    free(config.misc.program_version);
  if (NULL != config.misc.config_path)
    free(config.misc.config_path);
  if (NULL != config.misc.man_path)
    free(config.misc.man_path);
  if (NULL != config.misc.groff_path)
    free(config.misc.groff_path);
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

  // Deallocate memory used by the TOC
  if (NULL != toc && toc_len > 0)
    toc_free(toc, toc_len);

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
