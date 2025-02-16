# Dependencies

This document is provided as an aid to prospective packagers.

The following commands should install all dependencies needed by Qman, for
different operating systems and distros:

## Arch Linux
```
  # pacman -Sy
  # pacman -S base-devel git meson libinih python-cogapp man-db man-pages
```

## Debian 12
```
  # apt update
  # apt install build-essential git meson pkg-config libncurses-dev libinih-dev zlib1g-dev libbz2-dev pipx man-db
  $ pipx install cogapp
```

> **:bulb: Note**
>
> By default, `pipx` will install the `cog` executable in `~/.local/bin`. You
> may need to add this directory to your path before proceeding with
> `meson setup`.

## Debian 11
```
  # apt update
  # apt install build-essential git meson pkg-config libncurses-dev libinih-dev zlib1g-dev libbz2-dev python3-pip man-db
  # pip install cogapp
```

## Rocky Linux 9
```
  # dnf install epel-release
  # dnf update
  # dnf group install "Development Tools"
  # dnf install man-db man-pages ncurses-devel bzip2-devel python3-pip
  # dnf --enablerepo devel install meson inih-devel
  # pip install cogapp
```

> **:bulb: Note**
>
> These steps should also work with Red Hat Enterprise Linux 9 and AlmaLinux 9

## Rocky Linux 8
```
  # dnf install epel-release
  # dnf update
  # dnf group install "Development Tools"
  # dnf install man-db man-pages ncurses-devel inih-devel bzip2-devel python3-pip
  # dnf --enablerepo devel install meson
  # pip3 install cogapp
```

> **:bulb: Note**
>
> Rocky 8 supplies GNU `man` version 2.7, which is not compatible with Qman,
> as it doesn't support specifying the manual section using brackets (i.e.
> `man 'ls(1)'` results in 'No manual entry for ls(1)'). You will thus have to
> install a newer version of GNU `man` on your system.

> **:bulb: Note**
>
> These steps should also work with Red Hat Enterprise Linux 8 and AlmaLinux 8

## Ubuntu 24.04
```
  # apt update
  # apt install build-essential git meson cmake pkg-config libncurses-dev libinih-dev zlib1g-dev libbz2-dev python3-cogapp man-db
```

## Ubuntu 22.04 and 20.04
```
  # apt update
  # apt install build-essential git meson cmake pkg-config libncurses-dev libinih-dev libbz2-dev python3-pip man-db
  # pip install cogapp
```

> **:bulb: Note**
>
> For Ubuntu 20.04, you must substitute `meson compile` with `ninja` during the
> build process
