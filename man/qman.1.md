---
title: QMAN
section: 1
header: General Commands Manual
footer: Qman 1.4.1-88-g0c54909
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

**qman -f open**
: Show all manual pages named **open** across all sections (whatis)

**qman -k open**
: Show all manual pages whose short description matches the term **open**
  (apropos)

# USER INTERFACE
**Qman**'s reactions to user input are similar to what one would expect from a
pager such as **less(1)**, and from an ncurses-based browser such as
**links(1)**. Manual, apropos, and whatis pages are adorned with links to other
pages, HTTP locations, e-mail addresses, files in the local filesystem, or
(sub)sections within the current page. These links can be selected and opened.

The program provides a scrollbar, a status line, incremental search facilities
for locating manual pages, and facilities for searching through the text of the
page currently being displayed. A table of contents is generated for each page,
allowing for easy navigation to its sections, sub-sections, and paragraphs.
Navigation history and on-line help are also available.

The table below summarizes the program's actions and their default associated
keyboard mappings:

| Action name     | Description                           | Key mappings       |
|-----------------|---------------------------------------|--------------------|
| UP              | Scroll up one line and/or focus on the previous link | **UP**, **y**, **k** |
| DOWN            | Scroll down one line and/or focus on the next link | **DOWN**, **e**, **j** |
| LEFT            | Scroll left one tab stop              | **LEFT**, **<**    |
| RIGHT           | Scroll right one tab stop             | **RIGHT**, **>**   |
| PGUP            | Scroll up one page                    | **PGUP**, **b**    |
| PGDN            | Scroll down one page                  | **PGDN**, **f**    |
| HOME            | Go to page top                        | **HOME**, **g**    |
| END             | Go to page bottom                     | **END**, **G**     |
| OPEN            | Open focused link                     | **ENTER**, **o**   |
| OPEN_APROPOS    | Perform apropos on focused link       | **a**              |
| OPEN_WHATIS     | Perform whatis on focused link        | **w**              |
| SP_OPEN         | Open a manual page using a dialog     | **O**              |
| SP_APROPOS      | Perform apropos on a manual page using a dialog | **A**    |
| SP_WHATIS       | Perform whatis on a manual page using a dialog  | **W**    |
| INDEX           | Go to index (home) page               | **i**, **I**       |
| BACK            | Go back one step in history           | **BACKSPACE**, **[** |
| FWRD            | Go forward one step in history        | **]**              |
| HISTORY         | Show history menu                     | **s**, **S**       |
| TOC             | Show table of contents                | **t**, **T**       |
| SEARCH          | (Free text) search forward            | **/**              |
| SEARCH_BACK     | (Free text) search backward           | **?**              |
| SEARCH_NEXT     | Go to next search result              | **n**              |
| SEARCH_PREV     | Go to previous search result          | **N**              |
| HELP            | Show the help dialog                  | **h**, **H**       |
| QUIT            | Exit the program                      | **q**, **Q**       |

All of the aforementioned keyboard mappings are customizable. For more
information, see **CONFIGURATION**.

# MOUSE SUPPORT

Mouse input is supported but is considered experimental and is disabled by
default. The **CONFIGURATION** section contains instructions on how to enable
it. Most terminal emulators still provide basic mouse support when mouse input
is disabled.

When mouse input is enabled:

- The scroll wheel can be used as an alternative way for scrolling, invoking the
  UP and DOWN program actions
- Pressing and dragging the left mouse button over page text causes it to be
  selected and copied to the clipboard (see **NOTES 1 & 2**)
- Pressing and dragging the left mouse button over the scrollbar allows for
  scrolling through the page (see **NOTE 2**)
- Clicking the left mouse button on a link causes the link under the cursor to
  be selected (see **NOTE 2**)
- Clicking the middle button (the scroll wheel for most mice) invokes the OPEN
  action, opening the currently selected link
- Clicking the right button invokes the HELP action
- When inputting a search query or selecting from a menu, the middle button acts
  as a substitute for the ENTER key, and the right button as a substitute for
  CTRL-C
- When selecting from a menu, clicking the left button causes the menu entry
  under the cursor to be selected

The above behavior can be customized. For more information, see
**CONFIGURATION**.

**NOTE 1**
: There is no reliable method for terminal clients to copy data to the
  clipboard. An escape code (OSC 52) does exist but is only reliably supported
  by **kitty(1)** and **ghostty(1)**. For all other terminals, **Qman** will
  default to using commands such as **xclip(1)**, **wl-clipboard(1)**, or
  **pbcopy(1)**. However, these will only work when running locally and within a
  desktop environment (not when using SSH).

**NOTE 2**
: Some terminals may report mouse cursor position inaccurately, causing
  difficulties with clicking and dragging.

# OPTIONS
The program accepts the following non-argument options:

**-n, \-\-index**
: Show a list of all manual pages on the system, together with their sections
  and short descriptions. (This is the default behavior when the program is
  launched with no command-line options and no arguments.)

**-k, \-\-apropos** _regexp_ ...
: Approximately equivalent to **apropos(1)**. Search for manual pages whose
  names and/or short descriptions match any of the _regexp_ arguments, and
  display their names, sections, and short descriptions.

**-f, \-\-whatis** _page_ ...
: Approximately equivalent to **whatis(1)**. Display the name, sections, and
  short descriptions of each of the manual _page_ arguments.

**-l, \-\-local\-file** _file_ ...
: Activate "local" mode. Format and display each local manual _file_ instead of
  searching through the system's manual collection. Each _file_ will be
  interpreted as an nroff source file in the correct format.

**-K \-\-global\-apropos** _regexp_ ...
: Show the contents of all manual pages whose names and/or short descriptions
  match any of the _regexp_ arguments. Beware that this option might cause
  long execution times. If not used in conjunction with **-T**, it is ignored.

**-a \-\-all** _page_ ...
: Show the contents of all manual pages whose names match any of the _page_
  arguments. Beware that this option might cause long execution times. If not
  used in conjunction with **-T**, it is ignored.

**-T, \-\-cli**
: Suppress the text user interface and output directly to the terminal. This
  option can be used to redirect the program's formatted output to a text file
  or to another command.

**-A, \-\-action** _action_name_
: Automatically perform program action _action_name_ upon startup. The list of
  valid action names can be found under **USER INTERFACE**.

**-C, \-\-config\-path** _file_
: Use _file_ as the configuration file for **Qman**.

**-v, \-\-version**
: Print program version and exit.

**-h, \-\-help**
: Print a help message and exit.

# CONFIGURATION
**Qman** expects to find its configuration file in the standard locations
defined by the [XDG Base Directory Specification](https://specifications.freedesktop.org/basedir-spec/latest/).
The following locations are searched in sequence:

- Any file specified using **-C** or **\-\-config\-path**
- _\${XDG_CONFIG_HOME}/qman/qman.conf_
- _\${HOME}/.config/qman/qman.conf_
- _\<path\>/qman/qman.conf_ where _\<path\>_ is an entry in
  _\${XDG_CONFIG_DIRS}_
- _\<configdir\>/qman.conf_ where _\<configdir\>_ is a compile-time option
- _/etc/xdg/qman/qman.conf_
- _/etc/qman/qman.conf_

The process stops once a configuration file has been found.

**Qman**'s configuration file uses the basic
[INI file format](https://en.wikipedia.org/wiki/INI_file), extended with an
**include** directive to allow for the configuration to be spread across
multiple files.

Different configuration options are grouped into sections. The paragraphs
below summarize the sections and configuration options that are available:

## Section [chars]
Options in this section specify what characters will be used to draw the text
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
| arrow_up          | up arrow                                                 |
| arrow_down        | down arrow                                               |

Each configuration option value must consist of a single Unicode character.

The default values for this section have been chosen to allow **Qman** to work
correctly with virtually all terminals, including the venerable **xterm(1)** and
the Linux console, and with all fonts. Depending on the terminal's capabilities,
**Qman** may choose to revert to said defaults, and ignore any options you have
specified in this section. This behavior can be overridden by adding
**unicode=true** to the **[tcap]** section.

## Section [colours]
Options in this section specify the user interface colors:

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
| link_file         | links to files in the local filesystem                   |
| link_file_f       | links to files in the local filesystem (focused)         |
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
| imm_border        | pop-up dialogs border                                    |
| imm_title         | pop-up dialogs title                                     |
| sp_input          | pop-up input dialog prompt                               |
| sp_text           | pop-up input dialog incremental search text              |
| sp_text_f         | pop-up input dialog incremental search text (focused)    |
| help_text         | help dialog entries text                                 |
| help_text_f       | help dialog entries text (focused)                       |
| history_text      | history dialog entries text                              |
| history_text_f    | history dialog entries text (focused)                    |
| toc_text          | table of contents dialog entries text                    |
| toc_text_f        | table of contents dialog entries text (focused)          |

Each color is defined using three words separated by whitespace:

_foreground_ _background_ _bold_

_foreground_ and _background_ can be one of **black**, **red**, **green**,
**yellow**, **blue**, **magenta**, **cyan**, or **white**. Alternatively, they
can be a number between 0 and 255, or a hexadecimal RGB value using the #RRGGBB
notation. Users should beware that not all terminals support numeric color
values higher than 7 and/or RGB values.

_bold_ is a boolean that signifies whether the foreground color will have a
high (true) or low (false) intensity.

The default values for this section have been chosen to allow **Qman** to work
correctly with virtually all terminals, including the venerable **xterm(1)** and
the Linux console. Depending on the terminal's capabilities, **Qman** may
choose to revert to said defaults, and ignore any options you have specified in
this section. This behavior can be overridden by adding **colors=256** and/or
**rgb=true** to the **[tcap]** section.

## Section [keys]
Options in this section specify which keys are mapped to each program action.

The section contains 25 configuration options, each corresponding to one of the
program actions described in the **USER INTERFACE** section of this manual page.
Their value is a tuple of up to 8 key definitions, separated by whitespace:

_key_1_ _key_2_ _key_3_ _key_4_ _key_5_ _key_6_ _key_7_ _key_8_

The value of each _key_i_ can take one of the following values:

- Any character, surch as **a**, **b**, **c**, etc.
- Any ncurses(3x) keycode, such as **KEY_UP** or **KEY_HOME**
- **F1** to **F12** (for the function keys)
- **ESC** (for the ESC key)
- **EXT** (for CTRL-C)
- **LF** (for the ENTER key)
- **BS** (for the BACKSPACE key)
- **HT** (for the TAB key)
- **SPACE** (for the spacebar)

For reasons of compatibility with various terminals, mapping the ENTER key
requires specifying both **KEY_ENTER** and **LF**. Similarly, mapping CTRL-C
requires specifying both **KEY_BREAK** and **ETX**, and mapping BACKSPACE
requires specifying both **KEY_BACKSPACE** and **BS**.

## Section [mouse]
This section contains the following options that pertain to mouse support:

| Option   | Type         | Def. value | Description                           |
|----------|--------------|------------|---------------------------------------|
| enable   | boolean      | false      | Enables mouse support                 |
| left_handed | boolean   | false      | Swaps the left and right mouse buttons |
| left_click_open | boolean | false    | Causes the left mouse button to invoke the OPEN action and/or act as the ENTER key |

## Section [layout]
This section contains various options that concern the layout of the text user
interface:

| Option   | Type         | Def. value | Description                           |
|----------|--------------|------------|---------------------------------------|
| sbar     | boolean      | true       | Indicates whether the scrollbar will be displayed |
| beep     | boolean      | true       | Indicates whether to beep the terminal on error |
| lmargin  | unsigned int | 2          | Size of margin between the left side of the screen, and the page text |
| rmargin  | unsigned int | 2          | Size of margin between the page text and the scroll bar and/or the right side of the screen |
| tabstop  | unsigned int | 8          | Number of characters in a tab stop (used by actions LEFT and RIGHT) |

## Section [tcap]
Normally, **Qman** detects the terminal's capabilities automatically. Options in
this section provide the ability to specify them explicitly, overriding this
behavior:

| Option   | Type         | Def. value | Description                           |
|----------|--------------|------------|---------------------------------------|
| colours  | int          | -1         | Number of colors supported by the terminal, or -1 to auto-detect |
| rgb      | ternary      | auto       | True if terminal can re-define colors, false if not, auto to auto-detect |
| unicode  | ternary      | auto       | True if terminal supports Unicode, false if not, auto to auto-detect |
| clipboard| ternary      | auto       | True if terminal supports clipboard operations (OSC 52), false if not, auto to auto-detect |
| escdelay | int          | 60         | Number of miliseconds to wait after receving ESC from the keyboard before interpreting it as the escape key. Users with historical terminals or very unreliable network connections may want to increase this. |

Beware that **Qman** uses these capabilities to decide whether to either honor
or ignore various configuration options specified elsewhere, particularly in
the **[chars]** and **[colours]** sections mentioned above. Auto-detection
should work correctly in most cases; it's therefore recommended to not modify
any of the options in this section, except when discovering or reporting bugs.

## Section [capabilities]
This section allows the user to disable various non-core features:

| Option       | Type         | Def. value | Description                       |
|--------------|--------------|------------|-----------------------------------|
| sections_on_top | boolean   | true       | Show a list of (links to the page's) sections at the top of each page |
| http_links   | boolean      | true       | Generate hyperlinks to HTTP URLs |
| email_links  | boolean      | true       | Generate hyperlinks to email addresses |
| file_links   | boolean      | true       | Generate hyperlinks to local files and directories |
| hyphenate    | boolean      | true       | Hyphenate long words in manual pages |
| justify      | boolean      | true       | Justify text in manual pages |
| icase_search | boolean      | true       | Ignore case when performing page text search | 
| sp_substrings | boolean     | true       | Include substring matches when performing incremental search of manual pages |

All features are enabled by default.

## Section [misc]
This section contains various miscellaneous options:

| Option       | Type         | Def. value | Description                       |
|--------------|--------------|------------|-----------------------------------|
| system_type  | string       | mandb      | Manual system type                |
| man_path     | string       | /usr/bin/man | Path to the **man(1)** command  |
| groff_path   | string       | /usr/bin/groff | Path to the **groff(1)** command |
| whatis_path  | string       | /usr/bin/whatis | Path to the **whatis(1)** command |
| apropos_path | string       | /usr/bin/apropos | Path to the **apropos(1)** command |
| browser_path | string       | /usr/bin/xdg-open | Path to the command that will be used to open HTTP links (i.e. your web browser) |
| mailer_path  | string       | /usr/bin/xdg-email | Path to the command that will be used to open e-mail links (i.e. your e-mail software) |
| viewer_path  | string       | /usr/bin/xdg-open | Path to the command that will be used to open links to files in the local filesystem |
| reset_after_http | boolean  | true       | Re-initialize curses after opening an http(s) link |
| reset_after_email| boolean  | true       | Re-initialize curses after opening an e-mail link |
| reset_after_viewer | boolean | true      | Re-initialize curses after opening a link to a local filesystem file |
| terminfo_reset | boolean    | false      | Reset the terminal using the strings provided by terminfo on shutdown |
| history_size | unsigned int | 256k       | Maximum number of history entries |
_system_type_ must match the Unix manual system used by your O/S:

- **[mandb](https://gitlab.com/man-db/man-db)** - most Linux distributions
- **[mandoc](https://mandoc.bsd.lv/)** - Void Linux, Haiku, others?
- **[freebsd](https://www.freebsd.org/)** - FreeBSD
- **[darwin](https://www.apple.com/macos/)** - macOS

To avoid an annoying screen redraw, options _reset_after_http_,
_reset_after_email_, or _reset_after_viewer_ can be set to **false** whenever
_browser_path_, _mailer_path_, or _viewer_path_ point to a GUI program
respectively.

Setting _terminfo_reset_ to **true** will initiate a full terminal reset, using
the strings provided by **terminfo(5)**, upon program shutdown. This may be
necessary if your ncurses implementation doesn't completely restore terminal
settings (e.g.  colors) upon exit, but will also clear the screen and erase your
scroll history as a side effect.

When using a horizontally narrow terminal, setting _hyphenate_ to **true**
and/or _justify_ to **false** can improve the program's output.

Setting _sp_substrings_ to **false** causes incremental search results to
only include pages whose names start with the user's input. Setting it to
**true** (the default) will also include pages whose names contain the input as
a substring, provided there is enough space left in the window.

## Include directive
Supplemental configuration files can be included using:

**include** _path_

Where _path_ is either an absolute path to the supplemental, or a path relative
to the directory component of the configuration file that performs the
inclusion.

## Notes on syntax
Include paths and option values may optionally be placed inside single or double
quotes. They can include the following escape sequences:

- **\\a**, **\\b**, **\\t**, **\\n**, **\\v**, **\\f**, and **\\r** are
  interpreted according to the ASCII standard
- **\\e** is interpreted as an escape (0x1b) character
- **\\\\** is interpreted as a backslash
- **\\'** and **\\"** are interpreted as a single and double quotes respectively

All text following a **;** until the end of the line is considered a comment and
is discarded.

# ENVIRONMENT
Users should take care setting their **TERM** environment variable to match
their virtual terminal.

When invoked using **-T**, the program tries to set its page width to the value
of the **MANWIDTH** environment variable. If **MANWIDTH** hasn't been set, it
tries to set it to the value of **COLUMNS** and, if that also fails, it sets
it to the default value of 80.

# SIGNALS

Upon receiving **SIGUSR1**, the program interrupts its operation and attempts
to locate and parse a configuration file, using the process outlined in
**CONFIGURATION**.

This feature fails to work with certain terminals, and should be considered
experimental.

# EXIT STATUS
| Value | Description                                                          |
|-------|----------------------------------------------------------------------|
| 0     | Successful program execution                                         |
| 1     | Usage or syntax error                                                |
| 2     | Operational error                                                    |
| 3     | A child process returned a non-zero exit status                      |
| 4     | Configuration file error                                             |
| 16    | No manual page(s) found matching the user's request                  |

The above are identical to the exit values of **man(1)**.

# SEE ALSO
**man(1)**, **apropos(1)**, **whatis(1)**, **pinfo(1)**

# AUTHOR
Written by Pantelis Panayiotou / plp13 on GitHub

# BUGS
Please report bugs at https://github.com/plp13/qman/issues

