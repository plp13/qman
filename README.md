# Qman
A more modern manual page viewer for our terminals.

## Rationale
Linux manual pages are lovely. They are concise, well-written, complete, and downright useful. However, the standard way of accessing them from the command-line hasn't changed since the early days. Qman aims to change that. It's a modern, full-featured manual page viewer that supports hyperlinks, web browser like navigation, on-line help, and a plethora of search options.

## Features already implemented
- Index page that displays all manual pages available on the system, sorted alphabetically and organised by section
- Apropos and whatis pages
- Hyperlinks to other manual pages
- Hyperlinks for URLs and email addresses (handled with xdg-open)
- Free text search
- Command-line options similar to those of 'man' (most importantly, '-k' and '-f')
- Keyboard mapping similar to that of 'less'
- Navigation history
- On-line help

## Features not yet implemented
- Config file (the necessary infrastructure to support this has already been coded, though)
- Manual page
- Perhaps more???

Note that the program is still under heavy development, and should be considered alpha quality. Once all the above features are implemented, I plan to tag it as 'beta1'.

## Building
Qman is written in plain C, and its only dependencies are `glibc` and `ncurses`. It uses the `meson` build system, so you need to do the following in order to compile it:

  $ meson setup build/ src/
  $ cd build/
  $ meson compile

## Screenshots

Index page:
![Index Page](/screenshots/qman_index.png)

Viewing a manual page:
![Viewing a Manual Page](/screenshots/qman_man.png)

Performing apropos:
![Performing Apropos](/screenshots/qman_apropos.png)

Online help:
![On-line Help](/screenshots/qman_help.png)
