# Qman
A more modern manual page viewer for our terminals.

## Rationale
Linux manual pages are lovely. They are concise, well-written, complete, and downright useful. However, the standard way of accessing them from the command-line hasn't changed since the early days.

Qman aims to change that. It's a modern, full-featured manual page viewer that supports hyperlinks, web browser like navigation, incremental search, on-line help, and more. It also strives to be fast and tiny, so that it can be used everywhere. For this reason, it's been written in plain C and has only minimal dependencies.

## Features already implemented
- Index page that displays all manual pages available on the system, sorted alphabetically and organised by section
- Apropos and whatis pages
- Hyperlinks to other manual pages
- Hyperlinks for URLs and email addresses (handled with xdg-open by default)
- Incremental search for manual pages
- Incremental free text search
- Command-line options similar to those of `man` (most importantly, `-k` and `-f`)
- Keyboard mappings similar to that of `less`
- Navigation history
- On-line help
- Fully configurable using an INI-style config file

## Features not yet implemented
- Manual page
- Perhaps more???

The program's basic functionality is now mostly complete, and it's fully usable. However, it should still be considered to be alpha quality. Once all features have been implemented, I will tag it as 'rc1'.

## Building
Qman is written in plain C, and its only library dependencies are `glibc`, `ncurses`, and `hinit`. It uses the `meson` build system.

To compile it, you must do the following:

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
