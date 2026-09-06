# Packages

Starting with Qman version 1.6.0, the following statically compiled binaries
are being made available:

- Debian, Ubuntu, Ubunbtu LTS
  - [`.deb` package](https://github.com/plp13/qman/releases)
- Fedora, Red Hat Enterprise Linux, Rocky Linux, Alpine Linux
  - [`.rpm` package](https://github.com/plp13/qman/releases)
- Other Linux
  - [Generic `.tar.gz` archive](https://github.com/plp13/qman/releases)

The source code of the scripts responsible for building these binaries can be
found [here](https://github.com/plp13/qman-binaries).

> **:bulb: Disclaimer**
>
> A lot of effort has been undertaken to ensure that the binaries are
> functional. The `.deb` package has been tested on Debian 11 to 13, Ubuntu 2504
> and Ubuntu LTS 2004, 2204 and 2404. The `.rpm` package, on Fedora 42, Rocky
> Linux 8 and Rocky Linux 9. And the `.tar.gz`, on all of the aforementioned
> systems.
>
> That said, nobody can guarantee that the packages will work with all Linux
> distributions or situations.

For other operating systems, the following third-party packages are known to
exist:

- Alpine Linux
  - [qman](https://pkgs.alpinelinux.org/package/edge/testing/armv7/qman): latest
    release
- Arch Linux
  - [qman](https://aur.archlinux.org/packages/qman) (AUR): latest release
  - [qman-git](https://aur.archlinux.org/packages/qman-git) (AUR):
    [devel](https://github.com/plp13/qman/tree/devel) branch
- Darwin / macOS
  - [qman](https://formulae.brew.sh/formula/qman) (Homebrew): latest release
- FreeBSD ports
  - [misc/qman](https://github.com/freebsd/freebsd-ports/tree/main/misc/qman):
    latest release
- Gentoo Linux
  - [app-misc/qman](https://gitweb.gentoo.org/repo/proj/guru.git/tree/app-misc/qman)
    (GURU): latest release

Packagers for other operating systems are always welcome. If you've created a
package, open an [issue](https://github.com/plp13/qman/issues) to add it to
this list.
