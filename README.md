# Qman
A more modern manual page viewer for our terminals

Version 1.0.0

## Screenshots

Index page:
![Index Page](/screenshots/qman_index.png)

Opening a manual page:
![Opening a Manual Page](/screenshots/qman_open.png)

Viewing a manual page:
![Viewing a Manual Page](/screenshots/qman_man.png)

Using the table of contents:
![Searching incrementally](/screenshots/qman_toc.png)

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
featuring hyperlinks, web browser like navigation, a table of contents for each
page, incremental search, on-line help, and more. It also strives to be fast and
tiny, so that it can be used everywhere. For this reason, it's been written in
plain C and has only minimal dependencies.

## Features
- Index page that displays all manual pages available on the system, sorted
  alphabetically and organised by section
- Pages for apropos and whatis results
- Hyperlinks to other manual pages
- Hyperlinks for URLs and email addresses (handled with xdg-open by default)
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
- Fully configurable using an INI-style config file
- Manual page

## Project status 
All basic functionality has been completed. The software has been in use for
over a year now, and should considered stable. Bugs may still arise, of course,
and should be reported using the [issues](https://github.com/plp13/qman/issues)
page.

## Downloading
Clone the [main](https://github.com/plp13/qman/tree/main) branch, which contains
Qman's latest stable version:

```
$ git clone -b main https://github.com/plp13/qman qman
```

If you wish to contribute to the program's development, clone the
[devel](https://github.com/plp13/qman/tree/devel) branch.

```
$ git clone -b devel https://github.com/plp13/qman qman
```

## Dependencies
Qman is written in plain C, and thus requires a compiler such as `gcc` or
`clang`. Its only library dependencies are `glibc`, `ncurses`, `hinit`, and
`zlib`. It uses the `meson` build system. Its manual page is written in
Markdown, and is compiled using `pandoc`.

<<<<<<< HEAD
CAUTION: `pandoc` is written in Haskell and requires installing a large number
of dependencies. These are only necessary for compiling Qman's manual page, and
can be removed after building. 

In order for the program to make sense, a Unix manual pages database must also
be installed.
=======
> **:warning: Warning**
>
> `pandoc` is written in Haskell and requires installing a large number of
> dependencies. These are only necessary for compiling Qman's manual page, and
> can be removed after building.

The program is a front-end to GNU `man`, and therefore requires `man` and
`groff` to be installed. In order for it to make sense, a Unix manual pages
database must also be present.
>>>>>>> devel

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

> **:bulb: Note**
>
> By default, `pipx` will install the `cog` executable in `~/.local/bin`. You
> will have to add this directory to your path before proceeding with
> `meson setup`.

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

> **:bulb: Note**
>
> These steps should also work with Red Hat Enterprise Linux 9 and AlmaLinux 9.

### Rocky Linux 8
```
  # dnf install epel-release
  # dnf update
  # dnf group install "Development Tools"
  # dnf install man-db man-pages ncurses-devel python3-pip inih-devel
  # dnf --enablerepo devel install meson pandoc
  # pip3 install cogapp
```

> **:bulb: Note**
>
> These steps should also work with Red Hat Enterprise Linux 9 and AlmaLinux 9.

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
Make sure all of the above dependencies are installed, and do the following:

```
$ meson setup build/ src/
$ cd build/
$ meson compile
$ sudo meson install
```

> **:bulb: Note**
>
> If using an older version of `meson`, you may need to substitute the
> aforementioned `meson compile` command with `ninja`.

### Packages
For Arch Linux users, there is a an
[AUR package](https://aur.archlinux.org/packages/qman-git).

## Troubleshooting
Always make sure you are up-to-date with the `main` branch. And, of course,
RTFM:

```
$ qman qman
```

> :question: What is the location of the configuration file?

`~/.config/qman.conf` (user-specific) or `/etc/xdg/qman.conf` (system-wide).

> :question: Calling `qman` without any parameters fails with message
> `Apropos '': nothing appropriate`

Your system does not have a manual page index cache. This can be fixed by
running (as root):

```
# mandb
```

> :question: I have enabled mouse support by adding `enable=true` to the
> `[mouse]` section of my configuration file, but now I'm unable to copy text to
> the clipboard using the mouse, and/or my mouse behaves erratically

Mouse support is experimental, and depends on features that are not fully
implemented by all terminals. See Qman's manual page for more information.

> :question: Trying to open an HTTP or e-mail link causes the program to
> terminate (or does nothing)

By default, Qman uses `xdg-open` to open such links. On desktop Linux systems,
this is sufficient to open them using the default browser / email client. On
other systems, you may need to specify alternative programs with the
`browser_path` and `mailer_path` options in the `misc` section of Qman's
configuration file. To avoid opening such links altogether, set both options to
a command that does nothing, e.g. `/usr/bin/false`.

> :question: Qman does not look as pretty on my system as in the screenshots

That look can be achieved by using one of the supplied
[modernity.conf](https://github.com/plp13/qman/blob/main/config/modernity.conf)
and [modernity_gui.conf](https://github.com/plp13/qman/blob/main/config/modernity_gui.conf)
configuration files. Both files require a modern virtual terminal with support
for Unicode fonts and 256 colors.
