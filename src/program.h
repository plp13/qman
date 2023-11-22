// Program-specific infrastructure (definition)

#ifndef PROGRAM_H

#define PROGRAM_H

#include "lib.h"
#include "util.h"

//
// Types
//

// Option argument type
typedef enum {
  OA_NONE,     // no argument
  OA_OPTIONAL, // argument is optional
  OA_REQUIRED  // argument is required
} option_arg_t;

// A command-line option
typedef struct {
  char *long_opt;     // i.e. --*verbose* (value in stars)
  char short_opt;     // i.e. -*v*
  wchar_t *help_text; // i.e. *Print verbose output*
  option_arg_t arg;   // i.e. --config=*myconfrc*
  bool _cont;         // when false, indicates end of array
} option_t;

// A colour
typedef struct {
  short fg;   // foreground colour no.
  bool bold;  // true if foreground colour is bold, false otherwise
  short bg;   // background colour no.
  short pair; // pair no.
} colour_t;

// A program action
typedef enum {
  PA_NULL,         // do nothing (must be first member of action_t)
  PA_UP,           // scroll up one line
  PA_DOWN,         // scroll down one line
  PA_PGUP,         // scroll up one page
  PA_PGDN,         // scroll down one page
  PA_HOME,         // go to page top
  PA_END,          // go to page end
  PA_OPEN,         // open focused link
  PA_OPEN_APROPOS, // perform apropos on focused link
  PA_OPEN_WHATIS,  // perform whatis on focused link
  PA_SP_OPEN,      // open a manual page specified by the user
  PA_SP_APROPOS,   // perform apropos on a manual page specified by the user
  PA_SP_WHATIS,    // perform whatis on a manual page specified by the user
  PA_INDEX,        // go to index (home) page
  PA_BACK,         // go back one step in history
  PA_FWRD,         // go forward one step in history
  PA_SEARCH,       // search forward
  PA_SEARCH_BACK,  // search backward
  PA_SEARCH_NEXT,  // go to next search result
  PA_SEARCH_PREV,  // go to previous search result
  PA_HELP,         // go to keyboard help page
  PA_QUIT          // exit the program (must be last member of action_t)
} action_t;

// Program configuration: characters used for drawing the TUI
typedef struct {
  wchar_t *sbar_top;          // status bar top
  wchar_t *sbar_vline;        // status bar vertical line
  wchar_t *sbar_bottom;       // status bar bottom
  wchar_t *sbar_block;        // status bar drag block
  wchar_t *trans_mode_name;   // mode to name transition (on indicator line)
  wchar_t *trans_name_loc;    // name to location transition (on indicator line)
  wchar_t *trans_prompt_help; // prompt to help transition (on input line)
  wchar_t
      *trans_prompt_em; // prompt to error message transition (on input line)
  wchar_t *box_hline;   // box horizontal line
  wchar_t *box_vline;   // box vertical line
  wchar_t *box_tl;      // box top left
  wchar_t *box_tr;      // box top right
  wchar_t *box_bl;      // box bottom left
  wchar_t *box_br;      // box bottom right
} config_chars_t;

// Program configuration: colours
typedef struct {
  colour_t text;              // normal text
  colour_t search;            // highlighted search results
  colour_t link_man;          // links to manual pages
  colour_t link_man_f;        // links to manual pages (focused)
  colour_t link_http;         // links to http(s) URLs
  colour_t link_http_f;       // links to http(s) URLs (focused)
  colour_t link_email;        // links to email addresses
  colour_t link_email_f;      // links to email addresses (focused)
  colour_t link_ls;           // links to local searches
  colour_t link_ls_f;         // links to local searches (focused)
  colour_t sb_line;           // scrollbar line
  colour_t sb_block;          // scrollbar indicator block
  colour_t stat_indic_mode;   // mode section of status bar indicator line
  colour_t stat_indic_name;   // name section of status bar indicator line
  colour_t stat_indic_loc;    // location section of status bar indicator line
  colour_t stat_input_prompt; // prompt section of status bar input line
  colour_t stat_input_help;   // help section of status bar input line
  colour_t stat_input_em;     // error message section of status bar input line
  colour_t imm_border;        // immediate window border
  colour_t imm_title;         // immediate title bar
  colour_t sp_input;          // input field in tui_sp_open() window
  colour_t sp_text;           // text in tui_sp_open() window
  colour_t sp_text_f;         // focuesd text in tui_sp_open() window
  unsigned trans_mode_name;   // colour pair for mode to name transition
  unsigned trans_name_loc;    // colour pair for name to location transition
  unsigned trans_prompt_help; // colour pair for prompt to help transition
  unsigned
      trans_prompt_em; // colour pair for prompt to error message transition
} config_colours_t;

// Program configuration: program action to key character mappings
typedef int config_keys_t[PA_QUIT + 1][8];

// Program configuration: screen layout
typedef struct {
  bool tui;        // true if we're displaying the TUI, false if we're printing
                   // to stdout instead
  bool fixedwidth; // if true, don't change the width to match the current
                   // terminal width
  bool sbar;       // if true, show the scrollbar
  bool beep;       // if true, beep the terminal
  unsigned width;  // current terminal width
  unsigned height; // current terminal height
  unsigned sbar_width;       // scrollbar width
  unsigned stat_height;      // status bar height
  unsigned main_width;       // main window width
  unsigned main_height;      // main window height
  unsigned imm_width;        // immediate window width
  unsigned imm_height_short; // height of short immediate windows
  unsigned imm_height_long;  // height of long immediate windows
  unsigned lmargin;          // size of left margin
  unsigned rmargin;          // size of right margin
} config_layout_t;

// Program configuration: miscellaneous
typedef struct {
  wchar_t *program_name;    // this program's name
  wchar_t *program_version; // version string
  char *config_path;        // path to configuration file
  char *man_path;           // path to 'man' command
  char *whatis_path;        // path to 'whatis' command
  char *apropos_path;       // path to 'apropos' command
  char *browser_path;       // path to the user's web browser
  char *mailer_path;        // path to the user's email program
  unsigned history_size;    // size of page request history
} config_misc_t;

// Program configuration (main)
typedef struct {
  config_chars_t chars;
  config_colours_t colours;
  config_keys_t keys;
  config_layout_t layout;
  config_misc_t misc;
} config_t;

// Location of a link in an array of lines
typedef struct {
  bool ok;       // true if the location exists, false otherwise
  unsigned line; // line number
  unsigned link; // link number
} link_loc_t;

// Page request type
typedef enum {
  RT_INDEX,   // show a list of all manual pages
  RT_MAN,     // show a manual page
  RT_APROPOS, // search for manual pages and their descriptions
  RT_WHATIS   // show all available manual pages with this name
} request_type_t;

// A page request
typedef struct {
  request_type_t request_type;
  wchar_t *args; // arguments for the man/apropos/whatis command
  // The following are used by history_...() functions, to record the user's
  // location in history entries (which are instances of request_t)
  unsigned top;     // recorded page_top
  link_loc_t flink; // recorded page_flink
} request_t;

// Choice between apropos and whatis
typedef enum { AW_APROPOS, AW_WHATIS } aprowhat_cmd_t;

// An apropos or whatis result
typedef struct {
  wchar_t *page;    // Manual page
  wchar_t *section; // Section
  wchar_t *ident;   // Combined <manual page>(<section>)
  wchar_t *descr;   // Description
} aprowhat_t;

// Link type
typedef enum {
  LT_MAN,   // manual page
  LT_HTTP,  // http(s) URL
  LT_EMAIL, // email address
  LT_LS     // local search: find trgt in the current document
} link_type_t;

// A link
typedef struct {
  unsigned start;   // character no. where the link starts
  unsigned end;     // character no. the link ends
  link_type_t type; // type of link
  wchar_t *trgt;    // link target (e.g. "ls(1)" or "http://www.google.com/")
} link_t;

// A line of text
typedef struct {
  unsigned length;       // the line's length
  wchar_t *text;         // the line's text
  unsigned links_length; // number of links in line
  link_t *links;         // links in line
  // Places in the line the text becomes...
  bitarr_t reg;    // regular
  bitarr_t bold;   // bold
  bitarr_t italic; // italic
  bitarr_t uline;  // underlined
} line_t;

// A search result
typedef struct {
  unsigned line;  // line number
  unsigned start; // character no. where the result starts
  unsigned end;   // character no. where the result ends
} result_t;

//
// Constants
//

// Exit statuses (same as those of man command)
#define ES_SUCCESS 0     // successful completion
#define ES_USAGE_ERROR 1 // user provided wrong command-line option
#define ES_OPER_ERROR 2  // program error
#define ES_CHILD_ERROR 3 // child process error
#define ES_NOT_FOUND 16  // manual page(s) not found

//
// Global variables
//

// Program options
extern option_t options[];

// Program configuration
extern config_t config;

// History of page requests
extern request_t *history;

// Location of current request in history array
extern unsigned history_cur;

// Location of top request in history array (i.e. the last page inserted in
// history)
extern unsigned history_top;

// All manual pages on this system
extern aprowhat_t *aw_all;

// Number of entries in aw_all
extern unsigned aw_all_len;

// All manual sections on this system
extern wchar_t **sc_all;

// Number of entries in sc_all
extern unsigned sc_all_len;

// Current page being displayed
extern line_t *page;

// Title of current page
extern wchar_t page_title[BS_SHORT];

// Number of lines in page
extern unsigned page_len;

// Focused link in page
extern link_loc_t page_flink;

// Line where the portion of page displayed to the user begins
extern unsigned page_top;

// Column where the portion of page displayed to the user begins
extern unsigned page_left;

// True if last man/apropos/whatis command didn't produce any result
extern bool err;

// Formatted error message for last man/apropos/whatis failure
extern wchar_t err_msg[BS_LINE];

// Search results in current page
extern result_t *results;

// Total number of search results in current page
extern unsigned results_len;

// Regular expressions for links to a...
extern full_regex_t re_man, // manual page
    re_url,                 // http(s) URL
    re_email;               // email address

//
// Macros
//

// Allocate memory for all members of a line of length len. Then, initialize its
// length member to len, and its text member to an empty string.
#define line_alloc(line, len)                                                  \
  line.length = len;                                                           \
  line.text = walloc(len);                                                     \
  line.text[0] = '\0';                                                         \
  line.links_length = 0;                                                       \
  line.links = NULL;                                                           \
  if (len > 0) {                                                               \
    line.reg = balloc(len);                                                    \
    bclearall(line.reg, len);                                                  \
    line.bold = balloc(len);                                                   \
    bclearall(line.bold, len);                                                 \
    line.italic = balloc(len);                                                 \
    bclearall(line.italic, len);                                               \
    line.uline = balloc(len);                                                  \
    bclearall(line.uline, len);                                                \
  } else {                                                                     \
    line.reg = NULL;                                                           \
    line.bold = NULL;                                                          \
    line.italic = NULL;                                                        \
    line.uline = NULL;                                                         \
  }

// Allocate memory for an extra link for line, and increase its link_length
// by 1. trgt_length is the length of the new link's trgt member.
#define line_realloc_link(line, trgt_len)                                      \
  line.links_length++;                                                         \
  line.links = xreallocarray(line.links, line.links_length, sizeof(link_t));   \
  line.links[line.links_length - 1].trgt = walloc(trgt_len);

// Free memory for all members of (line_t variable) line
#define line_free(line)                                                        \
  free(line.text);                                                             \
  links_free(line.links, line.links_length);                                   \
  free(line.reg);                                                              \
  free(line.bold);                                                             \
  free(line.italic);                                                           \
  free(line.uline);

// Free memory for all members of (array of link_t) links
#define links_free(links, links_len)                                           \
  for (unsigned link_free_i = 0; link_free_i < links_len; link_free_i++)       \
    free(links[link_free_i].trgt);                                             \
  free(links);

// String representation of a page request type
#define request_type_str(t)                                                    \
  RT_INDEX == t                                                                \
      ? L"INDEX"                                                               \
      : (RT_MAN == t ? L"MAN" : (RT_APROPOS == t ? L"APROPOS" : L"WHATIS"))
//
// Functions
//

// Initialize all program components, except ncurses
extern void init();

// Retrieve argc and argv from main() and parse the command line options.
// Modify config and history appropriately, and return optind. Exit in case of
// usage error.
extern int parse_options(int argc, char *const *argv);

// Retrieve argc and argv with the command line options removed, and modify
// history appropriately. Exit in case of usage error.
extern void parse_args(int argc, char *const *argv);

// Print usage information
extern void usage();

// All history_...() functions also save and restore page_top and page_flink
// inside the history entries they manipulate, to keep track of the user's
// position in each history entry.

// Populate the current history entry (i.e. history[history_cur]), setting its
// request_type member to rt, and its args to args
extern void history_replace(request_type_t rt, wchar_t *args);

// Push a new entry into history, as follows:
// Add a new history entry after history_cur, and populate it with rt and args
// using history_replace(). Increase history_cur, and adjust history_top so that
// it remains equal to or greater than it.
extern void history_push(request_type_t rt, wchar_t *args);

// If n is smaller than or equal to history_cur, go back n steps in history and
// return true. Otherwise, return false.
extern bool history_back(unsigned n);

// If n + history_cur is smaller than or equal to history_top, go forward n
// steps in history and return true. Otherwise, return false.
extern bool history_forward(unsigned n);

// Discard all history entries after history_cur, and make history_top equal to
// history_cur
extern void history_reset();

// Execute apropos or whatis, and place their result in dst. Return the number
// of entries found. Arguments cmd and args respectively specify the command to
// run and its arguments.
extern unsigned aprowhat_exec(aprowhat_t **dst, aprowhat_cmd_t cmd,
                              const wchar_t *args);

// Given a result of aprowhat() in aw (of length aw_len), extract the names of
// its manual sections into dst. Return the total number of sections found.
extern unsigned aprowhat_sections(wchar_t ***dst, const aprowhat_t *buf,
                                  unsigned buf_len);

// Render a result of aprowhat() aw (of length aw_len), and a result of
// aprowhat_sections() sc (of length aw_len) into dst, as an array of lines of
// text. Return the number of lines. key, title, ver, and date are used for the
// header and footer.
extern unsigned aprowhat_render(line_t **dst, const aprowhat_t *aw,
                                unsigned aw_len, wchar_t *const *sc,
                                unsigned sc_len, const wchar_t *key,
                                const wchar_t *title, const wchar_t *ver,
                                const wchar_t *date);

// Search for elements of hayst (of length hayst_len), whose ident starts with
// needle. Return the first matching position in hayst after pos, or -1 if
// nothing can be matched.
extern int aprowhat_search(const wchar_t *needle, const aprowhat_t *hayst,
                           unsigned hayst_len, unsigned pos);

// Render a page that contains an index of all manual pages in dst
extern unsigned index_page(line_t **dst);

// In case a man/apropos/whatis command fails to produce any results, aprowhat()
// and man() set err to true and err_msg to an appropriate error message.

// Execute apropos or whatis, and place the final rendered result in dst. Return
// the number of lines in said output. Arguments cmd and args respectively
// specify the command to run and its arguments. key and title specify a short
// and long title respectively, to be inserted in the header and footer.
extern unsigned aprowhat(line_t **dst, aprowhat_cmd_t cmd, const wchar_t *args,
                         const wchar_t *key, const wchar_t *title);

// Execute man, and place its final rendeered output in dst. Return the number
// of lines in said output. args specifies the arguments for the man command.
extern unsigned man(line_t **dst, const wchar_t *args);

// Find the previous link in lines (of linegth lines_len), starting at location
// start. Return said link's location.
extern link_loc_t prev_link(line_t *lines, unsigned lines_len,
                            link_loc_t start);

// Find the next link in lines (of length lines_len), starting at location
// start. Return said link's location.
extern link_loc_t next_link(line_t *lines, unsigned lines_len,
                            link_loc_t start);

// Return the first link in lines (of length lines_len) that appears in line
// number range [start, stop]
extern link_loc_t first_link(line_t *lines, unsigned lines_len, unsigned start,
                             unsigned stop);

// Return the last link in lines (of length lines_len) that appears in line
// number range [start, stop]
extern link_loc_t last_link(line_t *lines, unsigned lines_len, unsigned start,
                            unsigned stop);

// Search for needle in lines (of length lines_len). Place all results into dst
// and return total number of results.
extern unsigned search(result_t **dst, wchar_t *needle, line_t *lines,
                       unsigned lines_len);

// Return the line number of the member of res that immediately follows line
// number from. If no such line exists, return -1. res_len is the length of res.
extern int search_next(result_t *res, unsigned res_len, unsigned from);

// Return the line number of the member of res that immediately precedes line
// number from. If no such line exists, return -1. res_len is the length of res.
extern int search_prev(result_t *res, unsigned res_len, unsigned from);

// Populate page, page_title, and page_len, based on the contents of
// history[history_cur].
extern void populate_page();

// Free the memory occupied by reqs (of length reqs_len)
extern void requests_free(request_t *reqs, unsigned reqs_len);

// Free the memory occupied by aw (of length aw_len)
extern void aprowhat_free(aprowhat_t *res, unsigned res_len);

// Free the memory occupied by lines (of length lines_len)
extern void lines_free(line_t *lines, unsigned lines_len);

// Exit the program gracefully, with exit code ec. If em is not NULL, echo it
// on stdout before exiting.
extern void winddown(int ec, const wchar_t *em);

#endif
