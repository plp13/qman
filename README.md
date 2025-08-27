# Qman
A more modern manual page viewer for our terminals

Version 1.5.0-14-gf4bcae5 -- [see what's new](#new-in-this-version)

![Screenhot -- Viewing a Manual Page](/screenshots/qman_man.png)

## Get started

- [Generic build instructions](doc/BUILDING.md) (works for most Linux distros)
- [O/S specific instructions](doc/OS_SPECIFIC.md)
- [Troubleshooting](doc/TROUBLESHOOTING.md)
- [Packages](doc/PACKAGES.md)
- [Configuration and themes](config/)
- [Contributing](doc/CONTRIBUTING.md)

## Rationale

Unix manual pages are lovely. They are concise, well-written, complete, and
downright useful. However, the standard way of accessing them from the
command-line hasn't changed since the early days.

Qman aims to change that. It's a modern, full-featured manual page viewer
featuring hyperlinks, web browser like navigation, a table of contents for each
page, incremental search, on-line help, and more. It also strives to be fast and
tiny, so that it can be used everywhere. For this reason, it's been written in
plain C and has only minimal dependencies.

## Features

- Index page that displays all manual pages available on the system, sorted
  alphabetically and organised by section
- Pages for apropos and whatis results
- Hyperlinks to other manual pages
- Hyperlinks for URLs and email addresses
- Hyperlinks to files or directories in the local filesystem
- In-page hyperlinks
- A table of contents for each manual page
- Incremental search for manual pages
- Incremental free page text search
- Command-line options similar to those of `man` (most importantly, `-k` and
  `-f`)
- Keyboard mappings similar to those of `less`
- Mouse support
- Navigation history
- On-line help
- Fully configurable using INI-style config files
- Manual page

## Project status 

All basic functionality has been completed. The software has been in use since
late 2023 and is considered stable. Bugs, of course, still happen. If you think
you have found one, please open an
[issue](https://github.com/plp13/qman/issues).

## New in this version

Version 1.6.0 introduces the following:
- *Italic text* support
- Themes can now use the terminal's default colors
- New `minimal-16` theme that follows the host terminal's color scheme
  (contributed by [denn-moe](https://github.com/denn-moe))
- Space bar is now associated with DOWN, in order to conform with common UX
  expectations
- Miscellaneous bug fixes and documentation updates

> **:bulb: Note**
>
> Users using custom config files may need to update them after a new release.
> For more information, please refer to Qman's manual page and the documentation
> in [config/README.md](config/README.md).

## More screenshots

Index page:
![Index Page](/screenshots/qman_index.png)

Opening a manual page using a pop-up:
![Screenshot -- Opening a Manual Page](/screenshots/qman_open.png)

Using the table of contents:
![Using the Table of Contents](/screenshots/qman_toc.png)

Searching incrementally:
![Searching incrementally](/screenshots/qman_search.png)

Performing apropos:
![Performing Apropos](/screenshots/qman_apropos.png)

Online help:
![Online Help](/screenshots/qman_help.png)
