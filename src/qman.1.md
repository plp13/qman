---
title: QMAN
section: 1
header: General Commands Manual
footer: Qman nightly
date: December 15, 2023
---

# NAME
**Qman** - A more modern manual page viewer for our terminals

# SYNOPSIS
**qman** [_options_]  
**qman** [_options_] **-n**  
**qman** [_options_] [[_section_] _page_]  
**qman** [_options_] **-k** _regexp_ ...  
**qman** [_options_] **-f** _page_ ...  

# DESCRIPTION
**Qman** is a modern, interactive manual page viewer for our terminals. It
strives to be easy to use for anyone familiar with the **man(1)** command, and
also to be fast and tiny, so that it can be used everywhere.

# EXAMPLES
**qman** or **qman -n**
: Show the index (home) page, a collection of all manual pages available on the
  system

**qman ls**
: Show the manual page for **ls**

**qman 1 ls** or **qman 'ls(1)'**
: Show the manual page for **ls** from manual section **1**

**qman -w open**
: Show all manual pages named **open** across all sections (whatis)

**qman -k open**
: Show all manual pages whose short description matches the term **open**
  (apropos)

# USER INTERFACE
**Qman**'s reactions to user input are similar to what one would expect from a
pager such as **less(1)**, and from an ncurses-based browser such as
**links(1)**. Manual, apropos, and whatis pages are adorned with links to other
manual pages, HTTP locations, e-mail addresses, or in-page locations. These
links can be selected and opened. The program also offers incremental search
facilities for locating manual pages pages and/or excerpts in the text of the
page currently being displayed, together with on-line search facilities.

The table below summarizes the program's actions and their default associated
keyboard mappings:

| Action name     | Description                           | Key mappings       |
|-----------------|---------------------------------------|--------------------|
| UP              | Scroll up one line and/or focus on the previous link | UP, 'y', 'k'       |
| DOWN            | Scroll down one line and/or focus on the next link | DOWN, 'e', 'j'     |
| PGUP            | Scroll up one page                    | PGUP, 'b'          |
| PGDN            | Scroll down one page                  | PGDN, 'f'          |
| HOME            | Go to page top                        | HOME, 'g'          |
| END             | Go to page bottom                     | END, 'G'           |
| OPEN            | Open focused link                     | ENTER, 'o'         |
| OPEN_APROPOS    | Perform apropos on focused link       | 'a'                |
| OPEN_WHATIS     | Perform whatis on focused link        | 'w'                |
| SP_OPEN         | Open a manual page using a dialog     | 'O'                |
| SP_APROPOS      | Perform apropos on a manual page using a dialog |  'A'     |
| SP_WHATIS       | Perform whatis on a manual page using a dialog  |  'W'     |
| INDEX           | Go to index (home) page               | 'i', 'I'           |
| BACK            | Go back one step in history           | BACKSPACE, '['     |
| FWRD            | Go forward one step in history        | ']'                |
| SEARCH          | (Free text) search forward            | '/'                |
| SEARCH_BACK     | (Free text) search backward           | '?'                |
| SEARCH_NEXT     | Go to next search result              | 'n'                |
| SEARCH_PREV     | Go to previous search result          | 'N'                |
| HELP            | Show the help message dialog          | 'h', 'H'           |
| QUIT            | Exit the program                      | 'q', 'Q'           |

All of the aforementioned keyboard mappings are customizable. For more
information, see **CONFIGURATION**.

# OPTIONS
The program accepts the following non-argument options:

**-n, \-\-index**
: Show a list of all manual pages on the system, together with their sections
  and short descriptions. (This is the default behaviour, if the program is
  launched with no command-line options and no arguments.)

**-k, \-\-apropos** _regexp_ ...
: Approximately equivalent to **apropos(1)**. Search for manual pages whose
  names and/or short descriptions match any of the _regexp_ arguments, and
  display their names, sections, and short descriptions.

**-f, \-\-whatis** _page_ ...
: Approximately equivalent to **whatis(1)**. Display the name, sections, and
  short descriptions of each of the manual _page_ arguments.

**-l, \-\-local-file** _file_ ...
: Activate "local" mode. Format and display each local manual _file_ instead of
  searching through the system's manual collection. Each _file_ will be
  interpreted as an nroff source file in the correct format.

**-T, \-\-cli**
: Suppress the text user interface and output directly to the terminal. This
  option can be used to redirect the program's formatted output to a text file
  or to another command.

**-C, \-\-config-path=file**
: Use this user configuration file rather than the default.

**-h, \-\-help**
: Print a help message and exit.

# CONFIGURATION
**Qman** will attempt to load its configuration from _~/home/.config/qman.conf_
(the user config file). Failing that, it will try to load it from
_/etc/xdg/qman.conf_ (the system config file). The process stops once a config
file has been found and loaded; in other words, the system and user config files
are not cumulative.

If the user specifies the **-C** option, the program instead tries to load its
configuration from the file specified by the user.

**Qman** uses the INI file format (https://en.wikipedia.org/wiki/INI_file). The
following sections and configuration options are accepted:

**Section [chars]**
: Options in this section specify what characters will be used to draw the text
  user interface:

| Option            | Description                                              |
|-------------------|----------------------------------------------------------|
| sbar_top          | scrollbar top end                                        |
| sbar_vline        | scrollbar track line                                     |
| sbar_bottom       | scrollbar bottom end                                     |
| sbar_block        | scrollbar knob                                           |
| trans_mode_name   | transition between the mode and name sections of the status bar |
| trans_name_loc    | transition between the name and location sections of the status bar |
| trans_prompt_help | transition between the prompt and help sections of the status bar |
| trans_prompt_em   | transition between the prompt and error message sections of the status bar|
| box_hline         | dialog box horizontal line                               |
| box_vline         | dialog box vertical line                                 |
| box_tl            | dialog box top left corner                               |
| box_tr            | dialog box top right corner                              |
| box_bl            | dialog box bottom left corner                            |
| box_br            | dialog box bottom right corner                           |

Each configuration option value in this section must consist of a single
Unicode character.

**Section [colours]**
: Options in this section specify the user interface colours:

| Option            | Description                                              |
|-------------------|----------------------------------------------------------|
| text              | page text                                                |
| search            | matched search terms in page text                        |
| link_man          | links to manual pages                                    |
| link_man_f        | links to manual pages (focused)                          |
| link_http         | HTTP links                                               |
| link_http_f       | HTTP links (focused)                                     |
| link_email        | e-mail links                                             |
| link_email_f      | e-mail links (focused)                                   |
| link_ls           | in-page links                                            |
| link_ls_f         | in-page links (focused)                                  |
| sb_line           | scrollbar track line                                     |
| sb_block          | scrollbar knob                                           |
| stat_indic_mode   | status bar mode section                                  |
| stat_indic_name   | status bar name section                                  |
| stat_indic_loc    | status bar location section                              |
| stat_input_prompt | status bar input prompt                                  |
| stat_input_help   | status bar help section                                  |
| stat_input_em     | status bar error message section                         |
| imm_border        | pop-up dialogues border                                  |
| imm_title         | pop-up dialogues title                                   |
| sp_input          | pop-up input dialogue prompt                             |
| sp_text           | pop-up input dialogue progressive search text            |
| sp_text_f         | pop-up input dialogue progressive search text (focused)  |
| help_text         | help dialogue etries text                                |
| help_text_f       | help dialogue entries text (focused)                     |

Each colour is defined using three words separated by whitespace:

_foreground_ _background_ _bold_

_foreground_ and _background_ can be one of 'black', 'red', 'green',
'yellow', 'blue', 'magenta', 'cyan', or 'white'. Alternatively, they can be a
number between 0 and 255, or a hexadecimal RGB value using the #RRGGBB
notation. Users should beware that not all terminals support numeric colour
values higher than 7 and/or RGB values.

_bold_ is a boolean that signifies whether the foreground colour will have a
high (true) or low (false) intensity.

**Section [keys]**
: Options in this section specify which keys are mapped to each program action.

The section contains 21 configuration options, each corresponding to one of the
program actions described in the **USER INTERFACE** section of this manual page.
Their value is a tuple of up to 8 key definitions, separated by whitespace:

_key_1_ _key_2_ _key_3_ _key_4_ _key_5_ _key_6_ _key_7_ _key_8_

The value of each _key_i_ can take one of the following values:

1. Any ncurses(3x) keycode, such as **KEY_UP** or **KEY_HOME**
2. 'F1' to 'F12' (for the function keys)
3. 'ESC' (for the ESC key)
4. 'EXT' (for CTRL-C)
5. 'LF' (for the ENTER key)
6. 'BS' (for the BACKSPACE key)
7. 'HT' (for the TAB key)
8. 'SPACE' (for the spacebar)

For reasons of compatibility with various terminals, mapping the ENTER key
requires specifying both 'KEY_ENTER' and 'LF'. Similarly, mapping CTRL-C
requires specifying both 'KEY_BREAK' and 'ETX', and mapping BACKSPACE requires
specifying both 'KEY_BACKSPACE' and 'BS'.

**Section [layout]**
: This section contains various options that concern the layout of the text user
  interface:

| Option   | Type         | Description                                        |
|----------|--------------|----------------------------------------------------|
| sbar     | boolean      | Indicates whether the scrollbar will be displayed  |
| beep     | boolean      | Indicates whether to beep the terminal on error    |
| lmargin  | unsigned int | Size of margin between the left side of the screen, and the page text |
| rmargin  | unsigned int | Size of margin between the page text and the scroll bar and/or the right side of the screen |

**Section [misc]**
: This section contains various miscellaneous options:

| Option       | Type         | Description                                    |
|--------------|--------------|------------------------------------------------|
| man_path     | string       | Path to the **man(1)** command                 |
| whatis_path  | string       | Path to the **whatis(1)** command              |
| apropos_path | string       | Path to the **apropos(1)** command             |
| browser_path | string       | Path to the command that will be used to open HTTP links (i.e. your web browser) |
| mailer_path  | string       | Path to the command that will be used to open e-mail links (i.e. your e-mail software) |
| history_size | unsigned int | Maximum number of history entries              |

# ENVIRONMENT
When invoked using **-C**, the program tries to set the page width to the value
of the **MANWIDTH** environment variable. If **MANWIDTH** hasn't been set, it
tries to set it to the value of **COLUMNS** and, if that also fails, it sets
it to the default value of 80.

# EXIT STATUS
| Value | Description                                                          |
|-------|----------------------------------------------------------------------|
| 0     | Successful program execution                                         |
| 1     | Usage or syntax error                                                |
| 2     | Operational error                                                    |
| 3     | A child process returned a non-zero exit status                      |
| 4     | Configuration file error                                             |
| 16    | No manual page(s) found matching the user's request                  |

# SEE ALSO
**man(1)**, **apropos(1)**, **whatis(1)**, **pinfo(1)**

# AUTHOR
Written by Pantelis Panayiotou / plp13 on GitHub

# BUGS
Please report bugs at https://github.com/plp13/qman/issues

