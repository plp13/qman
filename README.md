# Qman
A more modern manual page viewer for our terminals

Version 1.3.1-28-g8f3804c -- [see what's new](#new-in-this-version)

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
- Fully configurable using INI-style config files
- Manual page

## Project status 
All basic functionality has been completed. The software has been in use for
over a year now, and should considered stable. Bugs may still arise, of course,
and should be reported using the [issues](https://github.com/plp13/qman/issues)
page.

## New in this version
- A new configuration subsystem that provides an `include` directive, allowing
  Qman's configuration to be broken into multiple files
- A basic config file, `qman.conf`, together with a number of 'theme' config
  files, are now installed by default at `/etc/xdg/qman`. This gives Qman a
  handsome default look and feel.
- `hinit` is no longer a dependency
- Support for manual pages compressed using `xz`
- Improved clipboard support when using the `ghostty` terminal
- Miscellaneous bug fixes and documentation enhancements

> **:warning: Caution**
> This version changes the location of Qman's system-wide config file to
> `/etc/xdg/qman/qman.conf`, and the location of the user-specific config file
> to `/${HOME}/.config/qman/qman.conf`

> **:bulb: Note**
>
> Users using custom config files might need to update them when a new version
> version of Qman comes out. For more information, please refer to Qman's manual
> page and the documentation in
> [config](https://github.com/plp13/qman/blob/main/config].

## Downloading
Clone the [main](https://github.com/plp13/qman/tree/main) branch, which contains
Qman's latest stable version:

```
$ git clone -b main https://github.com/plp13/qman qman
```

[Tagged releases](https://github.com/plp13/qman/tags) with tarballs are also
available, starting with version 1.2.1.

## Dependencies
Qman is written in plain C, and thus requires a compiler such as `gcc` or
`clang`. Its only required library dependencies are `glibc` and `ncurses`. It
uses the `meson` build system.

There are also a number of optional dependencies:
- `zlib`: suport for manual pages compressed with `gzip`
- `bzip2`: support for manual pages compressed with `bzip2`
- `liblzma`: support for manual pages compressed with `xz`

Note that Qman is a front-end to GNU `man`, and therefore requires `man` and
`groff` to be installed. In order for it to make sense, a Unix manual pages
database must also be present.

## Building and installing
Make sure all of the above dependencies are installed, and do the following:

```
$ meson setup build/
$ cd build/
$ meson compile
$ sudo meson install
```

### Meson options

Option `-Ddocdir=<path>` can be passed to `meson setup`, to set the directory
where Qman's documentation will be installed.

If building for an embedded system, the following optional arguments can be
passed to `meson setup`, to disable certain program features:
- `-Dman-pages=disabled`: do not install the manual page
- `-Ddocs=disabled`: do not install documentation
- `-Dgzip=disabled`: disable support for manual pages compressed with `gzip`
- `-Dbzip2=disabled`: disable support for manual pages compressed with `bzip2`
- `-Dlzma=disabled`: disable support for manual pages compressed with `xz`

By default, all the above features are enabled.

> **:bulb: Note**
>
> If using an older version of `meson`, you may need to substitute the
> aforementioned `meson compile` command with `ninja`

### Packages
Arch Linux: packages [qman](https://aur.archlinux.org/packages/qman) and
[qman-git](https://aur.archlinux.org/packages/qman-git) (which follows
[devel](https://github.com/plp13/qman/tree/devel) are available on AUR

Gentoo Linux: package
[app-misc/qman](https://gitweb.gentoo.org/repo/proj/guru.git/tree/app-misc/qman)
is available on GURU

Packagers for other operating systems are always welcome. To get started, please
look at [PACKAGING.md](PACKAGING.md), which provides guidance on how to build
Qman on some popular Linux distributions.

## Troubleshooting
Always make sure you are up-to-date with the
[main](https://github.com/plp13/qman/tree/main) branch. And, of course, RTFM:

```
$ qman qman
```

> ***:question: What is the location of the configuration file?***

`~/.config/qman/qman.conf` (user-specific) or `/etc/xdg/qman/qman.conf`
(system-wide).

> ***:question: Calling `qman` without any parameters fails with message
> `Apropos '': nothing appropriate`***

Your system does not have a manual page index cache. This can be fixed by
running (as root):

```
# mandb
```

> ***:question: I'm unable to copy text to the clipboard using the mouse, and/or
> my mouse behaves erratically***

Mouse support is experimental, and depends on features that are not fully
implemented by all terminals. If you are having trouble with the mouse,
you can disable mouse support by commenting out the following lines in your
config file:

```
; [mouse]
; enable=true
```

> ***:question: Trying to open an HTTP or e-mail link causes the program to
> terminate (or does nothing)***

By default, Qman uses `xdg-open` to open such links. On desktop Linux systems,
this is sufficient to open them using the default browser / email client. On
other systems, you may need to specify alternative programs using the
`browser_path` and `mailer_path` options in the `misc` section your config file,
for example:

```
[misc]
browser_path=/usr/bin/links
mailer_path=/usr/bin/mutt
```

To avoid opening such links altogether, set both options to a command that does
nothing, e.g. `/usr/bin/false`.

> ***:question: I don't like the way Qman looks***

Use a different one of supplied
[themes](https://github.com/plp13/qman/config/themes). Or build your own (and
open a pull request to to add it to the repository).

## Contributing
If you wish to contribute to the program's development, clone the
[devel](https://github.com/plp13/qman/tree/devel) branch:

```
$ git clone -b devel https://github.com/plp13/qman qman
```

You should also take a look at [TESTING.md](TESTING.md), which describes the
procedures and tools we use for testing.
