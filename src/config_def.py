"""
Program configuration registry

A dictionary that describes the configuration options used by the program.

Structure:
    {
        section: {
            option: (
                (type, [constraint], ...),
                (default_value, [default_value], ...),
                in_config,
                comment
            )
        },
        ...
    }

Where:
    - section: one of the config file's sections (a string)
    - option: one of the options in said section (a string)
    - type: the option's type. One of the following strings:
        - "bool": a boolean
        - "trit": a ternary ("true", "false", or "auto")
        - "int": an integer
        - "string": an 8-bit string
        - "wstring": a wide string
        - "colour": a color definition (see manual page)
        - "key": a key mapping (see manual page)
    - constraint: optional constraints for a type. For the time being, the only
      type that has constraints is "int", and those are used to define a minimum
      and a maximum value for the type. For example `("int", 0, 80)` defines an
      integer whose value must be between 0 and 80 inclusive.
    - default_value: a default value for the option. Must be specified as a
      tuple of strings. For types "string" and "wstring", default_value can also
      be None, in which case the option's default value will be set to NULL.
    - in_config: set to True if option is to be allowed in the config file, or
      to False if it's internal to the program
    - comment: a brief description of the option, that will be used to generate
      comments in the header file


Note that section "keys" is special and can only contain options of type "key".
Also, all other sections must contain options of any type other than "key".
"""

config_def = {
    "tcap": {
        "colours": (("int", -1, 256), ("-1",), True, "Number of colors supported by the terminal (set to -1 to auto-configure)"),
        "rgb": (("trit",), ("auto",), True, "Terminal colors can be re-defined"),
        "unicode": (("trit",), ("auto",), True, "Terminal supports Unicode"),
        "clipboard": (("trit",), ("auto",), True, "Terminal supports clipboard operations (OSC 52)"),
        "escdelay": (("int", 1, 10000), ("60",), True, "Terminal escape delay")
    },
    "chars": {
        "sbar_top": (("wstring",), ("┬",), True, "Scrollbar top end"),
        "sbar_vline": (("wstring",), ("│",), True, "Scrollbar track line"),
        "sbar_bottom": (("wstring",), ("┴",), True, "Scrollbar bottom end"),
        "sbar_block": (("wstring",), ("█",), True, "Scrollbar knob"),
        "trans_mode_name": (("wstring",), ("│",), True, "Transition between the mode to name sections of the status bar"),
        "trans_name_loc": (("wstring",), ("│",), True, "Transition between the name and location sections of the status bar"),
        "trans_prompt_help": (("wstring",), (" ",), True, "Transition between the prompt and help sections of the status bar"),
        "trans_prompt_em": (("wstring",), (" ",), True, "Transition between the prompt and error message sections of the status bar"),
        "box_hline": (("wstring",), ("─",), True, "Dialog box horizontal line"),
        "box_vline": (("wstring",), ("│",), True, "Dialog box vertical line"),
        "box_tl": (("wstring",), ("┌",), True, "Dialog box top left corner"),
        "box_tr": (("wstring",), ("┐",), True, "Dialog box top right corner"),
        "box_bl": (("wstring",), ("└",), True, "Dialog box bottom left corner"),
        "box_br": (("wstring",), ("┘",), True, "Dialog box bottom right corner"),
        "arrow_up": (("wstring",), ("↑",), True, "Up arrow"),
        "arrow_down": (("wstring",), ("↓",), True, "Down arrow")
    },
    "colours": {
        "fallback": (("colour",), ("white", "black", "false"), False, "Fallback for B&W terminals"),
        "text": (("colour",), ("white", "black", "false"), True, "Page text"),
        "search": (("colour",), ("black", "white", "false"), True, "Matched search terms in page text"),
        "mark": (("colour",), ("white", "cyan", "false"), True, "Marked text"),
        "link_man": (("colour",), ("green", "black", "false"), True, "Links to manual pages"),
        "link_man_f": (("colour",), ("black", "green", "false"), True, "Links to manual pages (focused)"),
        "link_http": (("colour",), ("magenta", "black", "false"), True, "HTTP links"),
        "link_http_f": (("colour",), ("black", "magenta", "false"), True, "HTTP links (focused)"),
        "link_email": (("colour",), ("magenta", "black", "false"), True, "E-mail links"),
        "link_email_f": (("colour",), ("black", "magenta", "false"), True, "E-mail links (focused)"),
        "link_ls": (("colour",), ("cyan", "black", "false"), True, "In-page local search links"),
        "link_ls_f": (("colour",), ("black", "cyan", "false"), True, "In-page local search links (focused)"),
        "sb_line": (("colour",), ("yellow", "black", "true"), True, "Scrollbar track line"),
        "sb_block": (("colour",), ("yellow", "black", "true"), True, "Scrollbar knob"),
        "stat_indic_mode": (("colour",), ("yellow", "red", "true"), True, "Status bar mode section"),
        "stat_indic_name": (("colour",), ("white", "blue", "true"), True, "Status bar name section"),
        "stat_indic_loc": (("colour",), ("black", "white", "false"), True, "Status bar location section"),
        "stat_input_prompt": (("colour",), ("white", "black", "false"), True, "Status bar input prompt"),
        "stat_input_help": (("colour",), ("yellow", "black", "true"), True, "Status bar help section"),
        "stat_input_em": (("colour",), ("red", "black", "true"), True, "Status bar error message section"),
        "imm_border": (("colour",), ("yellow", "black", "true"), True, "Pop-up dialogs border"),
        "imm_title": (("colour",), ("yellow", "red", "true"), True, "Pop-up dialogs title"),
        "sp_input": (("colour",), ("white", "black", "false"), True, "Pop-up input dialog prompt"),
        "sp_text": (("colour",), ("cyan", "black", "false"), True, "Pop-up input dialog progressive help text"),
        "sp_text_f": (("colour",), ("white", "black", "false"), True, "Pop-up input dialog progressive help text (focused)"),
        "help_text": (("colour",), ("cyan", "black", "false"), True, "Help dialog entries text"),
        "help_text_f": (("colour",), ("black", "cyan", "false"), True, "Help dialog entries text (focused)"),
        "history_text": (("colour",), ("cyan", "black", "false"), True, "History dialog entries text"),
        "history_text_f": (("colour",), ("black", "cyan", "false"), True, "History dialog entries text (focused)"),
        "toc_text": (("colour",), ("cyan", "black", "false"), True, "TOC dialog entries text"),
        "toc_text_f": (("colour",), ("black", "cyan", "false"), True, "TOC dialog entries text (focused)"),
        "trans_mode_name": (("int", 0, 65535), ("0",), False, "Color pair for mode to name transition character"),
        "trans_name_loc": (("int", 0, 65535), ("0",), False, "Color pair for name to location transition character"),
        "trans_prompt_help": (("int", 0, 65535), ("0",), False, "Color pair for prompt to help transition character"),
        "trans_prompt_em": (("int", 0, 65535), ("0",), False, "Color pair for prompt to error_message transition character"),
    },
    "keys": {
        "up": (("key",), ("KEY_UP", "y", "k"), True, "Scroll up one line"),
        "down": (("key",), ("KEY_DOWN", "e", "j"), True, "Scroll down one line"),
        "left": (("key",), ("KEY_LEFT", "<"), True, "Scroll left one tab stop"),
        "right": (("key",), ("KEY_RIGHT", ">"), True, "Scroll right one tab stop"),
        "pgup": (("key",), ("KEY_PPAGE", "b"), True, "Scroll up one page"),
        "pgdn": (("key",), ("KEY_NPAGE", "f"), True, "Scroll down one page"),
        "home": (("key",), ("KEY_HOME", "g"), True, "Go to page top"),
        "end": (("key",), ("KEY_END", "G"), True, "Go to page end"),
        "open": (("key",), ("KEY_ENTER", "LF", "o"), True, "Open focused link"),
        "open_apropos": (("key",), ("a"), True, "Perform apropos on the focused link"),
        "open_whatis": (("key",), ("w"), True, "Perform whatis on the focused link"),
        "sp_open": (("key",), ("O"), True, "Open a manual page using a dialog"),
        "sp_apropos": (("key",), ("A"), True, "Perform apropos using a dialog"),
        "sp_whatis": (("key",), ("W"), True, "Perform whatis using a dialog"),
        "index": (("key",), ("i", "I"), True, "Go to index (home) page"),
        "back": (("key",), ("KEY_BACKSPACE", "BS", "["), True, "Go back one step in history"),
        "fwrd": (("key",), ("]"), True, "Go forward one step in history"),
        "history": (("key",), ("s", "S"), True, "Show history menu"),
        "toc": (("key",), ("t", "T"), True, "Show table of contents"),
        "search": (("key",), ("/"), True, "Forward search"),
        "search_back": (("key",), ("?"), True, "Backward search"),
        "search_next": (("key",), ("n"), True, "Go to next search result"),
        "search_prev": (("key",), ("N"), True, "Go to previous search results"),
        "help": (("key",), ("h", "H"), True, "Show this help message"),
        "quit": (("key",), ("q", "Q"), True, "Exit the program")
    },
    "mouse": {
        "enable": (("bool",), ("false",), True, "Enable mouse support"),
        "left_handed": (("bool",), ("false",), True, "Swap the left and right mouse buttons"),
        "left_click_open": (("bool",), ("false",), True, "Causes the left mouse button to invoke the OPEN action and/or act as the ENTER key")
    },
    "layout": {
        "tui": (("bool",), ("true",), False, "True if we are in TUI mode, false if we are in CLI mode"),
        "fixedwidth": (("bool",), ("true",), False, "If true, don't change width to match the terminal size"),
        "width": (("int", 0, 400), ("80",), False, "Current terminal width"),
        "height": (("int", 0, 100), ("25",), False, "Current terminal height"),
        "sbar_width": (("int", 0, 100), ("1",), False, "Scrollbar width"),
        "stat_height": (("int", 0, 100), ("2",), False, "Status bar height"),
        "main_width": (("int", 0, 400), ("79",), False, "Main window width"),
        "main_height": (("int", 0, 100), ("23",), False, "Main window height"),
        "imm_width_narrow": (("int", 0, 400), ("54",), False, "Narrow pop-up dialogs width"),
        "imm_width_wide": (("int", 0, 400), ("54",), False, "Wide pop-up dialogs width"),
        "imm_height_short": (("int", 0, 100), ("6",), False, "Short pop-up dialogs height"),
        "imm_height_long": (("int", 0, 100), ("10",), False, "Long pop-up dialogs height"),
        "sections_on_top": (("bool",), ("true",), True, "If true, show a list of sections at the top of each page"),
        "lmargin": (("int", 0, 200), ("2",), True, "Left margin size"),
        "rmargin": (("int", 0, 200), ("2",), True, "Right margin size"),
        "tabstop": (("int", 0, 100), ("8",), True, "Number of characters in a tab stop"),
        "sbar": (("bool",), ("true",), True, "If true, show the scrollbar"),
        "beep": (("bool",), ("true",), True, "If true, beep the terminal")
    },
    "misc": {
        "program_name": (("string",), None, False, "Program executable basename (discovered automatically)"),
        "program_version": (("wstring",), ("Qman 1.3.1-32-ga771e1f",), False, "Formal program name and version"),
        "config_path": (("string",), None, False, "Path to the configuration file"),
        "man_path": (("string",), ("/usr/bin/man",), True, "Path to the man(1) command"),
        "groff_path": (("string",), ("/usr/bin/groff",), True, "Path to the groff(1) command"),
        "whatis_path": (("string",), ("/usr/bin/whatis",), True, "Path to the whatis(1) command"),
        "apropos_path": (("string",), ("/usr/bin/apropos",), True, "Path to the apropos(1) command"),
        "browser_path": (("string",), ("/usr/bin/xdg-open",), True, "Path to web browser command"),
        "mailer_path": (("string",), ("/usr/bin/xdg-email",), True, "Path to mailer command"),
        "reset_after_http": (("bool",), ("true",), True, "Re-initialize curses after opening an http(s) link"),
        "reset_after_email": (("bool",), ("true",), True, "Re-initialize curses after opening an e-mail link"),
        "history_size": (("int", 0, 256 * 1024), ("65536",), True, "Maximum number of history entries"),
        "hyphenate": (("bool",), ("true",), True, "Hyphenate long words in manual pages"),
        "justify": (("bool",), ("true",), True, "Justify manual pages text"),
        "global_whatis": (("bool",), ("false",), False, "-a / --all option was passed"),
        "global_apropos": (("bool",), ("false",), False, "-k / --global-whatis option was passed")
    }
}
