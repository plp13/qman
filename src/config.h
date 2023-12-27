// Configuration and configuration file handling (definition)

#ifndef CONFIG_H

#define CONFIG_H

#include "lib.h"

//
// Types
//

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
  PA_HELP,         // show help message
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
  colour_t sp_text_f;         // focused text in tui_sp_open() window
  colour_t help_text;         // help text
  colour_t help_text_f;       // focused line of help text
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

//
// Constants
//

// Alternate configuration file locations
#define CONFIG_PATHS [ "~/.config/qman.conf", "/etc/xdg/qman.conf" ]

//
// Global variables
//

// Program configuration
extern config_t config;

//
// Functions
//

// Initialize configuration to sane defaults
extern void conf_init();

// Read the configuration file and set members of config appropriately. This
// function tries to find the config file in the following locations:
// (1) value of config.misc.config_path, set with '-C'
// (2) ~/.config/qman.conf
// (3) /etc/xdg/qman.conf
extern void configure();

#endif
