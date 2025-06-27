# Troubleshooting

1. Always make sure you are up-to-date with the
[main](https://github.com/plp13/qman/tree/main) branch.

2. Read [BUILDING.md](BUILDING.md) and [OS_SPECIFIC.md](OS_SPECIFIC.md).

3. And, of course, RTFM:

```
$ qman qman
```

---

> **:question: Calling `qman` without any parameters fails with message
> `'apropos .' failed; did you run mandb/makewhatis?`**

Most probably, your manual pages database hasn't been initialized. For most
Linux users this can be fixed by running (as root):

```
# mandb
```

Users of other operating systems may need to run the following instead:

```
# makewhatis
```

On some operating systems, this error may also be caused by Qman not having been
correctly configured. See [OS_SPECIFIC.md](OS_SPECIFIC.md).

---

> **:question:: Some manual pages don't show up, or manual pages of software
> that I have uninstalled still do show up**

Again, this is probably a manual pages database issue. Qman uses your operating
system's `apropos` to discover manual pages. If the database is out of date,
`apropos` (and therefore Qman) will produce inaccurate results.

The database must be updated by running `mandb` or `makewhatis` every time
manual pages are installed or uninstalled. Regrettably, some O/S fail to do this
automatically. If that's the case, you have to be doing it manually or create
your own automation.

---

> **:question: I'm unable to copy text to the clipboard using the mouse, and/or
> my mouse behaves erratically**

Mouse support is experimental, and depends on features that are not fully
implemented by all terminals. If you are having trouble with the mouse,
you can disable mouse support by commenting out the following lines in your
[config file](BUILDING.md#configuration):

```
; [mouse]
; enable=true
```

---

> **:question: Trying to open an HTTP or e-mail link causes the program to
> terminate (or does nothing)**

By default, Qman uses `xdg-open` to open such links. On desktop Linux systems,
this is sufficient to open them using the default browser / email client. On
other systems you may need to specify different commands in your
[config file](BUILDING.md#configuration).

For example, the following directives will cause Qman to open HTTP links with
`links` and e-mail links with `mutt`:

```
[misc]
browser_path=/usr/bin/links
mailer_path=/usr/bin/mutt
```

To avoid opening such links altogether, you can set both options to a command
that does nothing, e.g. `/usr/bin/false`.

---

> **:question: I don't like the way Qman looks**

Use a different one of the supplied [themes](../config/themes). Or build your
own theme (and open a pull request to to add it to the repository).

For instructions on how to use themes, refer to Qman's manual page or look
inside [config/qman.conf](../config/qman.conf)

---

> **:question: I modified my config file to include a theme, but Qman's colors
> still don't look right**

All provided themes require a terminal that supports 256 colors. Some themes,
such as `adwaita`, `adwaita-light` and `catpuccin_latte`, also require a
terminal that can re-define colors from RGB values.
