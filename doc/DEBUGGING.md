# Debugging

How to debug and profile Qman.

## Debugger

Use `gdb` (standalone or integrated with your editor/IDE).

To make sure the program gets fully rebuilt between debug sessions, use:

```
meson compile --clean && meson compile
```

## Memory leaks

Test for memory leaks using:

```
valgrind --leak-check=full --show-leak-kinds=all --show-reachable=no --track-origins=yes --log-file=valgrind.out ./qman <arguments>
```

Then, examine `valgrind.out`.

Reports of lost memory that has been allocated by `initscr()` and
`doupdate_sp()` can safely be ignored. They are caused by `ncurses`, which has
a [tendency](https://invisible-island.net/ncurses/ncurses.faq.html#config_leaks)
not to fully deallocate its own memory.

For more granularity, we may want to pass `--show-reachable=yes` to `valgrind`.
This, however, will result in even more spurious errors caused by `ncurses`.

## Profiling

Use the following command:

```
valgrind --tool=callgrind --dump-instr=yes --simulate-cache=yes --collect-jumps=yes ./qman <arguments>
```

This will produce a file named `callgrind.out.<pid>` where `<pid>` is the
process ID assigned to `valgrind` by the O/S. Use `kcachegrind` to open this
file for further examination.

## Debug logging

Developers can use function `loggit()` or macro `logprintf()`, both in
[util.h](../src/util.h), to print messages to `./qman.log` during development.

All calls to `loggit()` and `logprintf()` must be removed before committing to
either [main](https://github.com/plp13/qman/tree/main) or
[devel](https://github.com/plp13/qman/tree/devel).
