"""
Program configuration registry

A dictionary that describes the config structures used by the program. To add,
remove, or change the members of these structures, please modify this
dictionary and re-compile.

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
        - "int": an integer
        - "string": an 8-bit string
        - "wstring": a wide string
        - "colour": a colour definition (see manual page)
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
    "chars": {
        "sbar_top": (("wstring",), ("┬",), True, "scrollbar top end"),
        "sbar_vline": (("wstring",), ("│",), True, "scrollbar track line"),
        "sbar_bottom": (("wstring",), ("┴",), True, "scrollbar bottom end"),
        "sbar_block": (("wstring",), ("█",), True, "scrollbar knob"),
        "trans_mode_name": (("wstring",), ("│",), True, "transition between the mode to name sections of the status bar"),
        "trans_name_loc": (("wstring",), ("│",), True, "transition between the name and location sections of the status bar"),
        "trans_prompt_help": (("wstring",), (" ",), True, "transition between the prompt and help sections of the status bar"),
        "trans_prompt_em": (("wstring",), (" ",), True, "transition between the prompt and error message sections of the status bar"),
        "box_hline": (("wstring",), ("─",), True, "dialogue box horizontal line"),
        "box_vline": (("wstring",), ("│",), True, "dialogue box vertical line"),
        "box_tl": (("wstring",), ("┌",), True, "dialogue box top left corner"),
        "box_tr": (("wstring",), ("┐",), True, "dialogue box top right corner"),
        "box_bl": (("wstring",), ("└",), True, "dialogue box bottom left corner"),
        "box_br": (("wstring",), ("┘",), True, "dialogue box bottom right corner"),
    },
    "colours": {
        "text": (("colour",), ("white", "black", "false"), True, "page text"),
        "search": (("colour",), ("black", "white", "false"), True, "matched search terms in page text"),
        "link_man": (("colour",), ("green", "black", "false"), True, "links to manual pages"),
        "link_man_f": (("colour",), ("black", "green", "false"), True, "links to manual pages (focused)"),
        "link_http": (("colour",), ("magenta", "black", "false"), True, "HTTP links"),
        "link_http_f": (("colour",), ("black", "magenta", "false"), True, "HTTP links (focused)"),
        "link_email": (("colour",), ("magenta", "black", "false"), True, "e-mail links"),
        "link_email_f": (("colour",), ("black", "magenta", "false"), True, "e-mail links (focused)"),
        "link_ls": (("colour",), ("yellow", "black", "false"), True, "in-page local search links"),
        "link_ls_f": (("colour",), ("black", "yellow", "false"), True, "in-page local search links (focused)"),
        "sb_line": (("colour",), ("yellow", "black", "false"), True, "scrollbar track line"),
        "sb_block": (("colour",), ("yellow", "black", "false"), True, "scrollbar knob"),
        "stat_indic_mode": (("colour",), ("yellow", "red", "true"), True, "status bar mode section"),
        "stat_indic_name": (("colour",), ("white", "blue", "true"), True, "status bar name section"),
        "stat_indic_loc": (("colour",), ("black", "white", "false"), True, "status bar location section"),
        "stat_input_prompt": (("colour",), ("white", "black", "false"), True, "status bar input prompt"),
        "stat_input_help": (("colour",), ("yellow", "black", "true"), True, "status bar help section"),
        "stat_input_em": (("colour",), ("red", "black", "true"), True, "status bar error message section"),
        "imm_border": (("colour",), ("yellow", "black", "false"), True, "pop-up dialogues border"),
        "imm_title": (("colour",), ("yellow", "red", "true"), True, "pop-up dialogues title"),
        "sp_input": (("colour",), ("white", "black", "true"), True, "pop-up input dialogue prompt"),
        "sp_text": (("colour",), ("black", "black", "true"), True, "pop-up input dialogue progressive help text"),
        "sp_text_f": (("colour",), ("black", "white", "false"), True, "pop-up input dialogue progressive help text (focused)"),
        "help_text": (("colour",), ("white", "black", "true"), True, "help dialogue entries text"),
        "help_text_f": (("colour",), ("black", "white", "false"), True, "help dialogue entries text (focused)"),
        "trans_mode_name": (("int", 0, 65535), ("0",), False, "colour pair for mode to name transition character"),
        "trans_name_loc": (("int", 0, 65535), ("0",), False, "colour pair for name to location transition character"),
        "trans_prompt_help": (("int", 0, 65535), ("0",), False, "colour pair for prompt to help transition character"),
        "trans_prompt_em": (("int", 0, 65535), ("0",), False, "colour pair for prompt to error_message transition character"),
    },
    "keys": {
        # Comments for program actions are located in config.h.cog
        "up": (("key",), ("KEY_UP", "y", "k"), True, None),
        "down": (("key",), ("KEY_DOWN", "e", "j"), True, None),
        "pgup": (("key",), ("KEY_PPAGE", "b"), True, None),
        "pgdn": (("key",), ("KEY_NPAGE", "f"), True, None),
        "home": (("key",), ("KEY_HOME", "g"), True, None),
        "end": (("key",), ("KEY_END", "G"), True, None),
        "open": (("key",), ("KEY_ENTER", "LF", "o"), True, None),
        "open_apropos": (("key",), ("a"), True, None),
        "open_whatis": (("key",), ("w"), True, None),
        "sp_open": (("key",), ("O"), True, None),
        "sp_apropos": (("key",), ("A"), True, None),
        "sp_whatis": (("key",), ("W"), True, None),
        "index": (("key",), ("i", "I"), True, None),
        "back": (("key",), ("KEY_BACKSPACE", "BS", "["), True, None),
        "fwrd": (("key",), ("]"), True, None),
        "search": (("key",), ("/"), True, None),
        "search_back": (("key",), ("?"), True, None),
        "search_next": (("key",), ("n"), True, None),
        "search_prev": (("key",), ("N"), True, None),
        "help": (("key",), ("h", "H"), True, None),
        "quit": (("key",), ("q", "Q"), True, None)
    },
    "layout": {
        "tui": (("bool",), ("true",), False, "true if we are in TUI mode, false if we are in CLI mode"),
        "fixedwidth": (("bool",), ("true",), False, "if true, don't change width to match the terminal size"),
        "sbar": (("bool",), ("true",), True, "if true, show the scrollbar"),
        "beep": (("bool",), ("true",), True, "if true, beep the terminal"),
        "width": (("int", 0, 400), ("80",), False, "current terminal width"),
        "height": (("int", 0, 100), ("25",), False, "current terminal height"),
        "sbar_width": (("int", 0, 100), ("1",), False, "scrollbar width"),
        "stat_height": (("int", 0, 100), ("2",), False, "status bar height"),
        "main_width": (("int", 0, 400), ("79",), False, "main window width"),
        "main_height": (("int", 0, 100), ("23",), False, "main window height"),
        "imm_width": (("int", 0, 400), ("59",), False, "pop-up dialogues width"),
        "imm_height_short": (("int", 0, 100), ("4",), False, "short pop-up dialogues height"),
        "imm_height_long": (("int", 0, 100), ("15",), False, "long pop-up dialogues height"),
        "lmargin": (("int", 0, 200), ("2",), True, "left margin size"),
        "rmargin": (("int", 0, 200), ("2",), True, "right margin size")
    },
    "misc": {
        "program_name": (("wstring",), ("Qman",), False, "this program's name"),
        "program_version": (("wstring",), ("Qman nightly",), False, "version string"),
        "config_path": (("string",), None, False, "path to the configuration file"),
        "man_path": (("string",), ("/usr/bin/man",), True, "path to the man(1) command"),
        "whatis_path": (("string",), ("/usr/bin/whatis",), True, "path to the whatis(1) command"),
        "apropos_path": (("string",), ("/usr/bin/apropos",), True, "path to the apropos(1) command"),
        "browser_path": (("string",), ("/usr/bin/xdg-open",), True, "path to web browser command"),
        "mailer_path": (("string",), ("/usr/bin/xdg-email",), True, "path to mailer command"),
        "history_size": (("int", 0, 256 * 1024), ("65536",), True, "maximum number of history entries")
    }
}