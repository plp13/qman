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
                in_config
            )
        },
        ...
    }

Where:
    - section: one of the config file's sections (a string)
    - option: one of the options in said section (a string)
    - type and constraint: the option's type, i.e. one of the following:
        - "bool": a boolean
        - "int", min, max: an integer bounded by integer values min and max
        - "string": an 8-bit string
        - "wstring": a wide string
        - "colour": a colour definition (see manual page)
        - "key": a key mapping (see manual page)
    - in_config: set to True if option is to be allowed in the config file, or
      to False if it's internal to the program
"""

config_def = {
    "chars": {
        "sbar_top": (("wstring"), ("┬"), True),
        "sbar_vline": (("wstring"), ("│"), True),
        "sbar_bottom": (("wstring"), ("┴"), True),
        "sbar_block": (("wstring"), ("█"), True),
        "trans_mode_name": (("wstring"), ("│"), True),
        "trans_name_loc": (("wstring"), ("│"), True),
        "trans_prompt_help": (("wstring"), (" "), True),
        "trans_prompt_em": (("wstring"), (" "), True),
        "box_hline": (("wstring"), ("─"), True),
        "box_vline": (("wstring"), ("│"), True),
        "box_tl": (("wstring"), ("┌"), True),
        "box_tr": (("wstring"), ("┐"), True),
        "box_bl": (("wstring"), ("└"), True),
        "box_br": (("wstring"), ("┘"), True),
    },
    "colours": {
        "text": (("colour"), ("white", "black", "false"), True),
        "search": (("colour"), ("black", "white", "false"), True),
        "link_man": (("colour"), ("green", "black", "false"), True),
        "link_man_f": (("colour"), ("black", "green", "false"), True),
        "link_http": (("colour"), ("magenta", "black", "false"), True),
        "link_http_f": (("colour"), ("black", "magenta", "false"), True),
        "link_email": (("colour"), ("magenta", "black", "false"), True),
        "link_email_f": (("colour"), ("black", "magenta", "false"), True),
        "link_ls": (("colour"), ("yellow", "black", "false"), True),
        "link_ls_f": (("colour"), ("black", "yellow", "false"), True),
        "sb_line": (("colour"), ("yellow", "black", "false"), True),
        "sb_block": (("colour"), ("yellow", "black", "false"), True),
        "stat_indic_mode": (("colour"), ("yellow", "red", "true"), True),
        "stat_indic_name": (("colour"), ("white", "blue", "true"), True),
        "stat_indic_loc": (("colour"), ("black", "white", "false"), True),
        "stat_input_prompt": (("colour"), ("white", "black", "false"), True),
        "stat_input_help": (("colour"), ("yellow", "black", "true"), True),
        "stat_input_em": (("colour"), ("red", "black", "true"), True),
        "imm_border": (("colour"), ("yellow", "black", "false"), True),
        "imm_title": (("colour"), ("yellow", "red", "true"), True),
        "sp_input": (("colour"), ("white", "black", "true"), True),
        "sp_text": (("colour"), ("black", "black", "true"), True),
        "sp_text_f": (("colour"), ("black", "white", "false"), True),
        "help_text": (("colour"), ("white", "black", "true"), True),
        "help_text_f": (("colour"), ("black", "white", "false"), True)
    },
    "keys": {
        "up": (("key"), ("KEY_UP", "y", ("k"), True)),
        "down": (("key"), ("KEY_DOWN", "e", ("j"), True)),
        "pgup": (("key"), ("KEY_PPAGE", ("b"), True)),
        "pgdn": (("key"), ("KEY_NPAGE", ("f"), True)),
        "home": (("key"), ("KEY_HOME", ("g"), True)),
        "end": (("key"), ("KEY_END", ("G"), True)),
        "open": (("key"), ("KEY_ENTER", "LF", ("o"), True)),
        "open_apropos": (("key"), ("a"), True),
        "open_whatis": (("key"), ("w"), True),
        "sp_open": (("key"), ("O"), True),
        "sp_apropos": (("key"), ("A"), True),
        "sp_whatis": (("key"), ("W"), True),
        "index": (("key"), ("i", ("I"), True)),
        "back": (("key"), ("KEY_BACKSPACE", "BS", ("["), True)),
        "fwrd": (("key"), ("]"), True),
        "search": (("key"), ("/"), True),
        "search_back": (("key"), ("?"), True),
        "search_next": (("key"), ("n"), True),
        "search_prev": (("key"), ("N"), True),
        "help": (("key"), ("h", ("H"), True)),
        "quit": (("key"), ("q", ("Q"), True))
    },
    "layout": {
        "tui": (("bool"), ("true"), False),
        "fixedwidth": (("bool"), ("true"), False),
        "sbar": (("bool"), ("true"), True),
        "beep": (("bool"), ("true"), True),
        "width": (("int", 0, 400), ("80"), False),
        "height": (("int", 0, 100), ("25"), False),
        "sbar_width": (("int", 0, 100), ("1"), False),
        "stat_height": (("int", 0, 100), ("2"), False),
        "main_width": (("int", 0, 400), ("79"), False),
        "main_height": (("int", 0, 100), ("23"), False),
        "imm_width": (("int", 0, 400), ("59"), False),
        "imm_height_short": (("int", 0, 100), ("4"), False),
        "imm_height_long": (("int", 0, 100), ("15"), False),
        "lmargin": (("int", 0, 200), ("2"), True),
        "rmargin": (("int", 0, 200), ("2"), True)
    },
    "misc": {
        "program_name": (("wstring"), ("qman"), False),
        "program_version": (("wstring"), ("qman nightly"), False),
        "man_path": (("string"), ("/usr/bin/man"), True),
        "whatis_path": (("string"), ("/usr/bin/whatis"), True),
        "apropos_path": (("string"), ("/usr/bin/apropos"), True),
        "browser_path": (("string"), ("/usr/bin/xdg-open"), True),
        "mailer_path": (("string"), ("/usr/bin/xdg-email"), True),
    }
}
