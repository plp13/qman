# Build instructions

These generic instructions should work with most Linux distributions. For other
operating systems, see [OS_SPECIFIC.md](OS_SPECIFIC.md).

## Downloading

Clone the [main](https://github.com/plp13/qman/tree/main) branch, which contains
Qman's latest stable version:

```
$ git clone -b main https://github.com/plp13/qman qman
```

[Tagged releases](https://github.com/plp13/qman/tags) with tarballs are also
available, starting with version 1.2.1.

## Dependencies

Qman was designed to be portable. Because of this, it was written in plain C
and only has minimal dependencies.

Runtime dependencies:
- A Unix O/S that provides the standard `man`, `apropos` and `whatis` commands
- `ncursesw`
  - For best results, use the
    [official version](https://invisible-island.net/ncurses/) by Thomas E.
    Dickey. Most Linux distributions provide this by default.
- `groff`

Build dependencies:
- `gcc` or `clang`
- `meson`
  - On some O/S you may need to manually install additional support packages for
    `meson`, such as `cmake`, `ninja` and `pkg-config`
- `cogapp`,
  [a code generator written in Python](https://pypi.org/project/cogapp/)
  - If your O/S doesn't provide a package for `cogapp`, you can install it using
    `pip` or `pipx`

Optional dependencies:
- `zlib`: support for manual pages compressed with `gzip`
- `bzip2`: support for manual pages compressed with `bzip2`
- `liblzma`: support for manual pages compressed with `xz`
- `cunit`: used for unit testing

## Building and installing

Make sure that all runtime and build dependencies are installed, and do the
following:

```
$ meson setup build/
$ cd build/
$ meson compile
$ sudo meson install
```

> **:bulb: Note**
>
> If using an older version of `meson`, you may need to substitute the
> aforementioned `meson compile` command with `ninja`

> **:bulb: Note**
>
> These instructions might differ for certain operating systems. See
> [OS_SPECIFIC.md](OS_SPECIFIC.md).

### Meson options

The following optional arguments can be passed to `meson setup`:
- `-Dman-pages=disabled`: do not install the manual page
- `-Ddocs=disabled`: do not install any documentation
- `-Dconfig=disabled`: do not install any configuration files
- `-Dgzip=disabled`: disable support for manual pages compressed with `gzip`
- `-Dbzip2=disabled`: disable support for manual pages compressed with `bzip2`
- `-Dlzma=disabled`: disable support for manual pages compressed with `xz`
- `-Dtests=enabled`: enable unit testing
- `-Dconfigdir=...`: where to install configuration files
- `-Ddocdir=...`: where to install documentation

## Configuration

Qman should work without additional configuration on most Linux distributions.
However, for some operating systems and Linux distros you may need to add
extra directives to Qman's config file. See [OS_SPECIFIC.md](OS_SPECIFIC.md).

The config file is located at `~/.config/qman/qman.conf` (user-specific) or
`/etc/xdg/qman/qman.conf` (system-wide).

All configuration directives are documented in Qman's manual page:

```
$ qman qman
```
