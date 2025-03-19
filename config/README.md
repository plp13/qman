# Configuration files

`qman.conf` is a basic configuration file that should be sufficient for most
users. It's automatically installed at `/etc/xdg/qman/qman.conf`. Alternatively,
and in order to configure Qman on a per-user basis, it can be copied to
`${HOME}/.config/qman/qman.conf`.

A complete reference on configuration file locations and contents can be found
in Qman's manual page.

## Themes

A number of different themes for Qman can be found in the `themes` subfolder.
These can be included into your main configuration file with the `include`
directive, for example:

```
include themes/modernity.conf
```

> **:bulb: Note**
>
> Include paths are relative to the location of the configuration file that
> contains the `include` directive. For example, if the above line of code is in
> `/home/user/.config/qman/qman.conf`, Qman will expect to find `modernity.conf`
> in `/home/user/.config/qman/themes/`. When in doubt, use absolute paths.

> **:bulb: Note**
>
> Each theme has its own requirements and may not work with all terminal
> emulators. For example, a theme may require support for 8-bit or 24-bit color,
> or a specific Unicode font. These requirements are generally documented in
> the theme's comments.
