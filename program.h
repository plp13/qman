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
  wchar_t *program_name;  // this program's name
  char *config_path;      // path to configuration file
  char *man_path;         // path to 'man' command
  char *whatis_path;      // path to 'whatis' command
  char *apropos_path;     // path to 'apropos' command
  unsigned requests_size; // size of page request history
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

// A line of text
typedef struct {
  wchar_t *text; // the line's text
  // Places in the line the font becomes...
  bitarr_t reg;    // regular
  bitarr_t bold;   // bold
  bitarr_t italic; // italic
  bitarr_t uline;  // underlined
  bitarr_t lman;   // a manual page link
  bitarr_t lhttp;  // an http or https link
  bitarr_t lmail;  // an email link
} line_t;

// Choice between apropos and whatis
typedef enum { AW_APROPOS, AW_WHATIS } aprowhat_cmd_t;

// An apropos or whatis result
typedef struct {
  wchar_t *page;    // Manual page
  wchar_t *section; // Section
  wchar_t *descr;   // Description
} aprowhat_t;

// An array of apropos results

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
// Functions (comments are in program.c)
//

extern void init();

extern int parse_options(int argc, char *const *argv);

extern void usage();

extern unsigned aprowhat(aprowhat_t **dst, aprowhat_cmd_t cmd,
                         const char *args);

extern unsigned aprowhat_sections(wchar_t ***dst, const aprowhat_t *buf,
                                  unsigned buf_len);

extern void aprowhat_render(line_t *dst, const aprowhat_t *aw, unsigned aw_len,
                            wchar_t *const *sc, unsigned sc_len,
                            const wchar_t *key, const wchar_t *title,
                            const wchar_t *ver, const wchar_t *date);

extern void aprowhat_free(aprowhat_t *res, unsigned res_len);

extern void winddown(int ec, const wchar_t *em);

#endif
