# Qman
A more modern manual page viewer for our terminals

Version 1.4.1-72-g339a299 -- [see what's new](#new-in-this-version)

![Screenshot -- Opening a Manual Page](/screenshots/qman_open.png)

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
late 2023 and is considered to be stable. Bugs, of course, still happen. If
believe you have found one, please open an
[issue](https://github.com/plp13/qman/issues).

## New in this version

Version 1.5.0 introduces the following:
- Support for several different operating systems. Qman is no longer just for
  Linux. The following manual systems are currently supported:
  - **[mandb](https://gitlab.com/man-db/man-db)** - most Linux distributions
  - **[mandoc](https://mandoc.bsd.lv/)** - Void Linux, Haiku, others?
  - **[freebsd](https://www.freebsd.org/)** - FreeBSD
  - **[darwin](https://www.apple.com/macos/)** - macOS
- Three new themes:
  - `adwaita`, designed to match the colors of dark Adwaita desktops
  - `adwaita-light`, designed to match the colors of light Adwaita desktops
  - `modernity-light`, a generic light theme to complement `modernity`
- Re-organized and extended documentation
- Improved error messages, esp. when it comes to errors caused by
  a misconfiguration or a missing manual pages database
- Miscellaneous bug fixes and documentation updates

> **:bulb: Note**
>
> Users using custom config files may need to update them after a new release.
> For more information, please refer to Qman's manual page and the documentation
> in [config/README.md](config/README.md).

## More screenshots

Index page:
![Index Page](/screenshots/qman_index.png)

Viewing a manual page:
![Viewing a Manual Page](/screenshots/qman_man.png)

Using the table of contents:
![Using the Table of Contents](/screenshots/qman_toc.png)

Searching incrementally:
![Searching incrementally](/screenshots/qman_search.png)

Performing apropos:
![Performing Apropos](/screenshots/qman_apropos.png)

Online help:
![Online Help](/screenshots/qman_help.png)
