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
- Mouse support
- Navigation history
- On-line help
- Fully configurable using an INI-style config file
- Manual page

All basic functionality has now been completed. However, the program should
still be considered as beta quality. Please try it out and report any
[issues](https://github.com/plp13/qman/issues).

## Dependencies
Qman is written in plain C, and thus requires a compiler such as `gcc` or
`clang`. Its only library dependencies are `glibc`, `ncurses`, and `hinit`. It
uses the `meson` build system. Its manual page is written in Markdown, and is
compiled using `pandoc`.

In order for the program to make sense, a Unix manual pages database must also
be installed.

The following commands should install all necessary dependencies for different
operating systems and distros:

### Arch Linux
```
  # pacman -Sy
  # pacman -S base-devel git meson libinih python-cogapp pandoc man-db man-pages
```

### Debian 12
```
  # apt update
  # apt install build-essential git meson pkg-config libncurses-dev libinih-dev pipx pandoc man-db
  $ pipx install cogapp
```

Note that, by default, `pipx` will install the `cog` executable in
`~/.local/bin`. You will have to add this directory to your path before
proceeding with `meson setup`.

### Debian 11
```
  # apt update
  # apt install build-essential git meson pkg-config libncurses-dev libinih-dev python3-pip pandoc man-db
  # pip install cogapp
```

### Rocky Linux 9
```
  # dnf install epel-release
  # dnf update
  # dnf group install "Development Tools"
  # dnf install man-db man-pages ncurses-devel python3-pip pandoc
  # dnf --enablerepo devel install meson inih-devel
  # pip install cogapp
```

These steps should also work with Red Hat Enterprise Linux 9 and AlmaLinux 9.

### Rocky Linux 8
```
  # dnf install epel-release
  # dnf update
  # dnf group install "Development Tools"
  # dnf install man-db man-pages ncurses-devel python3-pip inih-devel
  # dnf --enablerepo devel install meson pandoc
  # pip3 install cogapp
```

These steps should also work with Red Hat Enterprise Linux 8 and AlmaLinux 8.

### Ubuntu 24.04
```
  # apt update
  # apt install build-essential git meson cmake pkg-config libncurses-dev libinih-dev python3-cogapp pandoc man-db
```

### Ubuntu 22.04 and 20.04
```
  # apt update
  # apt install build-essential git meson cmake pkg-config libncurses-dev libinih-dev python3-pip pandoc man-db
  # pip install cogapp
```

## Building and installing
Make sure all of the above dependencies are installed on your system, and do the
following:

```
$ meson setup build/ src/
$ cd build/
$ meson compile
$ sudo meson install
```

Note that, if using an older version of `meson`, you may need to substitute the
aforementioned `meson compile` command with `ninja`.

### Packages
For Arch Linux users, there is a an
[AUR package](https://aur.archlinux.org/packages/qman-git).

## Troubleshooting
Always make sure you are up-to-date with the `main` branch. And, of course,
RTFM:

```
$ qman qman
```

*What is the location of the configuration file?*

`~/.config/qman.conf` (user-specific) or `/etc/xdg/qman.conf` (system-wide).

*Calling `qman` without any parameters fails with message `Apropos '': nothing
appropriate`*

Your system does not have a manual page indexi cache. This can be fixed by
running (as root):

```
# mandb
```

*I have enabled mouse support by adding `enable=true` to the `[mouse]` section
of my configuration file, but now I'm unable to copy text to the clipboard using
the mouse, and/or my mouse behaves erratically*

Mouse support is experimental, and depends on features that are not fully
implemented by all terminals. See the manual page for more information.

*Trying to open an HTTP or e-mail causes the program to terminate (or does
nothing)*

By default, Qman uses `xdg-open` to open such links. On desktop Linux systems,
this is sufficient to open them using the default browser / email client. On all
other systems, you must specify alternative programs with the `browser_path` and
`mailer_path` options in the `misc` section of `qman`'s configuration file.
To avoid opening such links altogether, set both options to a command that does
nothing, e.g. `/usr/bin/false`.

*Qman does not look as pretty on my system as in the screenshots*

That look can be achieved by using one of the supplied
[modernity.conf](https://github.com/plp13/qman/blob/main/config/modernity.conf)
and [modernity_gui.conf](https://github.com/plp13/qman/blob/main/config/modernity_gui.conf)
configuration files. Both files require a modern virtual terminal with support
for Unicode fonts and 256 colors.
