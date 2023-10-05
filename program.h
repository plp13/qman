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

// Program configuration: colors
typedef struct {

} config_colors_t;

// Program configuration: keyboard mappings
typedef struct {

} config_keys_t;

// Program configuration: screen layout
typedef struct {
  bool tui;         // true if we're displaying the TUI, false if we're printing
                    // to stdout instead
  bool fixedwidth;  // if true, don't change the width to match the current
                    // terminal width
  unsigned width;   // current terminal width
  unsigned height;  // current terminal height
  unsigned lmargin; // size of left margin
  unsigned rmargin; // size of right margin
} config_layout_t;

// program configuration: miscellaneous
typedef struct {
  wchar_t *program_name;    // this program's name
  wchar_t *program_version; // version string
  char *config_path;        // path to configuration file
  char *man_path;           // path to 'man' command
  char *whatis_path;        // path to 'whatis' command
  char *apropos_path;       // path to 'apropos' command
  unsigned requests_size;   // size of page request history
} config_misc_t;

// Program configuration (main)
typedef struct {
  config_colors_t colors;
  config_keys_t keys;
  config_layout_t layout;
  config_misc_t misc;
} config_t;

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
  wchar_t
      *page; // manual page (or apropos/whatis query, depending on request_type)
  wchar_t *section; // section (only applicable if request_type is RT_MAN)
} request_t;

// Choice between apropos and whatis
typedef enum { AW_APROPOS, AW_WHATIS } aprowhat_cmd_t;

// An apropos or whatis result
typedef struct {
  wchar_t *page;    // Manual page
  wchar_t *section; // Section
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
  link_t *links;         // links on the line
  // Places in the line the font becomes...
  bitarr_t reg;    // regular
  bitarr_t bold;   // bold
  bitarr_t italic; // italic
  bitarr_t uline;  // underlined
} line_t;

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
// Global variables (comments are in program.c)
//

extern option_t options[];

extern config_t config;

extern request_t *requests;

extern unsigned current;

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

// Allocate memory for an extra link for line. trgt_length is the length of the
// new link's trgt member.
#define line_realloc_link(line, trgt_len)                                      \
  line.links_length++;                                                         \
  line.links = xreallocarray(line.links, line.links_length, sizeof(link_t));   \
  line.links[line.links_length - 1].trgt = walloc(trgt_len);

// Free memory for all members of (line_t variable) line
#define line_free(line)                                                        \
  free(line.text);                                                             \
  for (unsigned line_free_i = 0; line_free_i < line.links_length;              \
       line_free_i++)                                                          \
    free(line.links[line_free_i].trgt);                                        \
  free(line.links);                                                            \
  free(line.reg);                                                              \
  free(line.bold);                                                             \
  free(line.italic);                                                           \
  free(line.uline);

//
// Functions
//

// Initialize the software
extern void init();

// Retrieve argc and argv from main() and parse the command line options.
// Modify config and requests appropriately, and return optind
extern int parse_options(int argc, char *const *argv);

// Print usage information
extern void usage();

// Execute apropos or whatis, and place their result in dst. Return the number
// of entries found. Arguments cmd and args respectively specify the command to
// run and its arguments.
extern unsigned aprowhat_exec(aprowhat_t **dst, aprowhat_cmd_t cmd,
                              const char *args);

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

// Execute apropos or whatis, and place the final rendered result in dst. Return
// the number of lines in said output. Arguments cmd and args respectively
// specify the command to run and its arguments. key and title specify a short
// and long title respectively, to be inserted in the header and footer.
extern unsigned aprowhat(line_t **dst, aprowhat_cmd_t cmd, const char *args,
                         const wchar_t *key, const wchar_t *title);

// Execute man, and place its final rendeered output in dst. Return the number
// of lines in said output. args specifies the arguments for the man command.
extern unsigned man(line_t **dst, const char *args);

// Free the memory occupied by the result of aprowhat() aw (of length aw_len)
extern void aprowhat_free(aprowhat_t *res, unsigned res_len);

// Free the memory occupied by the result of aprowhat() or man() lines (of
// length lines_len)
extern void lines_free(line_t *lines, unsigned lines_len);

// Exit the program gracefully, with exit code ec. If em is not NULL, echo it
// on stdout before exiting.
extern void winddown(int ec, const wchar_t *em);

#endif
