# O/S specific instructions

Unless otherwise specified, instructions are for the latest version of the
respective O/S.

## Linux

The instructions in [BUILDING.md](BUILDING.md) should be sufficient for most
Linux distributions. (Please pay attention to the note about `libbsd`.)

### Older versions of `mandb`

Limitations in `mandb` versions 2.10 and earlier prevent Qman from performing
certain actions, such as opening links to manual pages. This affects
distributions such as RHEL / Rocky Linux /AlmaLinux 8 or earlier, and Debian
11 or earlier.

To work around this problem, add the following to your
[config file](BUILDING.md#configuration):

```
[misc]
legacy_mandb=true
```

### Void Linux

Void uses `mandoc`, therefore you must add the following to your
[config file](BUILDING.md#configuration):


```
[misc]
system_type=mandoc
```

## macOS

Install all [dependencies](BUILDING.md#dependencies) using `brew`. Then, run the
following to build and install Qman:

```
$ meson setup build/ -Dpkg_config_path=/opt/homebrew/opt/ncurses/lib/pkgconfig
$ cd build/
$ meson compile
$ sudo meson install
```

> **:bulb: Note**
>
> On Intel-based macs, use:
>
> ```
> -Dpkg_config_path=/usr/local/opt/ncurses/lib/pkgconfig
> ```

Then add the following to your [config file](BUILDING.md#configuration):

```
[misc]
system_type=darwin
groff_path=/usr/local/bin/groff
```

### Improving performance

Performance issues with the `whatis` and `apropos` commands in macOS can cause
Qman to lag. One way to fix this is to create a set of scripts that emulate
these commands for Qman:

/usr/local/bin/mkfakewhatis.sh
```
#!/usr/bin/env bash

DB=/usr/local/share/fakewhatis.txt

apropos "." > "${DB}"

exit $?
```

/usr/local/bin/fakewhatis.sh
```
#!/usr/bin/env bash

DB=/usr/local/share/fakewhatis.txt

if [ $# -eq 1 ]
then
  grep -e " ${1}[ \t\(,-].*-.*" "${DB}"
  exit $?
else
  exit -1
fi
```

/usr/local/bin/fakeapropos.sh
```
#!/usr/bin/env bash

DB=/usr/local/share/fakewhatis.txt

if [ $# -eq 1 ]
then
  if [ "X${1}" -eq "X." ]
  then
    cat "${DB}"
    exit $?
  else
    grep "${1}" "${DB}"
    exit $?
  fi
else
  exit -1
fi
```

Then, change the `[misc]` section of your config file as follows:

```
[misc]
system_type=darwin
groff_path=/usr/local/bin/groff
apropos=/usr/local/bin/fakeapropos.sh
whatis=/usr/local/bin/fakewhatis.sh
```

And, finally, run `mkfakewhatis.sh` as root:

```
# mkfakewhatis.sh
```

This command must be executed regularly, to ensure that
`/usr/local/share/fakewhatis.txt` is kept up-to-date.

Qman should now be lag-free!

## FreeBSD

Install all [dependencies](BUILDING.md#dependencies) using `pkg`. Then use the
[standard instructions](BUILDING.md#building-and-installing) to build and
install Qman.

FreeBSD-supplied `ncursesw` is known to cause compiler warnings, but otherwise
works. You can also use the
[Linux version](https://invisible-island.net/ncurses/) of `ncursesw`, which
can be installed using `pkg install ncurses`.

Finally, the following must be added to your
[config file](BUILDING.md#configuration):

```
[misc]
system_type=freebsd
groff_path=/usr/local/bin/groff
```

## Haiku

Haiku doesn't provide optional dependency `cunit`. Install all other
[dependencies](BUILDING.md#dependencies) using `pkgman`, and then build
and install Qman using:

```
$ meson setup build/ -D
$ cd build/
$ meson compile
$ sudo meson install
```

Then, add the following to your [config file](BUILDING.md#configuration):

```
[misc]
system_type=mandoc
man_path=/bin/man
whatis_path=/bin/whatis
apropos_path=/bin/apropos
groff_path=/bin/groff
```

An unidentified bug can cause Qman to crash on Haiku. If this happens, comment
out the line that contains `free(wline)` in function
`src/program.c:aprowhat_exec()` and re-compile.
