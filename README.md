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
- In-page hyperlinks
- Incremental search for manual pages
- Incremental free page text search
- Command-line options similar to those of `man` (most importantly, `-k` and `-f`)
- Keyboard mappings similar to that of `less`
- Navigation history
- On-line help
- Fully configurable using an INI-style config file
- Manual page

## Future features
- Navigation history dialog / view and select history entries using a menu
- Fix ugly hacks and make parts of the code DRYer
- More???

The program's basic functionality is now complete, and it will soon be tagged as a release candidate.

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
