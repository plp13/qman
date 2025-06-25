# Testing

Instructions on how to test Qman before creating a new release.

## Preliminaries

Thoroughly debug the program and make sure there are no memory leaks, using the
methodology described in [DEBUGGING.md](DEBUGGING.md).

## Operating systems

Test against the latest stable versions of these popular Linux distributions:

- Arch Linux
- Debian
- Rocky Linux
- Ubuntu (non-LTS)

Also test against:

- macOS
- FreeBSD

## Terminal emulators

Test against the following terminal emulators:

- console
- xterm
- urxvt
- gnome-terminal
- konsole
- terminator
- guake
- tilix
- tilda
- edex-ui
- cool-retro-term
- alacritty
- kitty
- st
- ghostty

## Configuration files

Test using the following configurations:

- [provided configuration](../config)
- empty configuration file
- custom configuration file, depending on the test case
