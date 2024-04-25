# Qman
A more modern manual page viewer for our terminals

## Screenshots

Index page:
![Index Page](/screenshots/qman_index.png)

Opening a manual page:
![Opening a Manual Page](/screenshots/qman_open.png)

Viewing a manual page:
![Viewing a Manual Page](/screenshots/qman_man.png)

Searching incrementally:
![Searching incrementally](/screenshots/qman_search.png)

Performing apropos:
![Performing Apropos](/screenshots/qman_apropos.png)

Online help:
![On-line Help](/screenshots/qman_help.png)

## Rationale
Linux manual pages are lovely. They are concise, well-written, complete, and
downright useful. However, the standard way of accessing them from the
command-line hasn't changed since the early days.

Qman aims to change that. It's a modern, full-featured manual page viewer
featuring hyperlinks, web browser like navigation, incremental search, on-line
help, and more. It also strives to be fast and tiny, so that it can be used
everywhere. For this reason, it's been written in plain C and has only minimal
dependencies.

## Features already implemented
- Index page that displays all manual pages available on the system, sorted
  alphabetically and organised by section
- Apropos and whatis pages
- Hyperlinks to other manual pages
- Hyperlinks for URLs and email addresses (handled with xdg-open by default)
- In-page hyperlinks
- Incremental search for manual pages
- Incremental free page text search
- Command-line options similar to those of `man` (most importantly, `-k` and
  `-f`)
- Keyboard mappings similar to those of `less`
- Navigation history
- On-line help
- Fully configurable using an INI-style config file
- Manual page

## Future features
- Navigation history dialog (view and select history entries using a menu)
- More???

All basic functionality has now been completed. However, the program should
still be considered as beta quality. Please try it out and report any
[issues](https://github.com/plp13/qman/issues).

## Dependencies
Qman is written in plain C, and thus requires a compiler such as `gcc` or
`clang`. Its only library dependencies are `glibc`, `ncurses`, and `hinit`. It
uses the `meson` build system. Its manual page is written in Markdown, and is
compiled using `pandoc`.

## Building and installing
Make sure all of the above dependencies are installed on your system, and do the
following:

```
  $ meson setup build/ src/
  $ cd build/
  $ meson compile
  $ sudo meson install
```

For Arch Linux users, there is a an [AUR package](https://aur.archlinux.org/packages/qman-git).

## Troubleshooting
Always make sure you are up-to-date with the `main` branch. And, of course,
RTFM:

```
qman qman
```

*What is the location of the configuration file?*

`~/.config/qman.conf` (user-specific) or `/etc/xdg/qman.conf` (system-wide).

*Calling `qman` without any parameters fails with message `Apropos '': nothing
appropriate`*

Your system does not have a manual page index. This can be fixed by running (as
root):

```
  # mandb
```

*Trying to open an HTTP or e-mail causes the program to terminate (or does
nothing)*

By default, `qman` uses `xdg-open` to open such links. On desktop Linux systems,
this is sufficient to open them using the default browser / email client. On all
other systems, you must specify alternative programs with the `browser_path` and
`mailer_path` options in the `misc` section of `qman`'s configuration file.
To avoid opening such links altogether, set both options to a command that does
nothing, e.g. `/usr/bin/false`.

*`qman` does not look as pretty on my system as in the screenshots*

That look can be achieved by pasting the directives in the supplied
[modernity.conf](https://github.com/plp13/qman/blob/main/config/modernity.conf).
into your configuration file. Your terminal must support at least 256 colours
and Unicode fonts.
