# Configuration files

`qman.conf` is a barebones configuration file that includes the most commonly
used options. It should be installed at `/etc/xdg/qman/qman.conf` for
system-wide configuration, or at `${HOME}/.config/qman/qman.conf` if configuring
for a local user.

> **:bulb: Note**
>
> Qman's man page includes a complete reference on config file locations and
> structure.

## Themes

A number of different themes for Qman can be found in the `themes` subfolder.
These can be included into your main configuration file using the `include`
directive:

```
include themes/modernity.conf
```

> **:bulb: Note**
>
> Include paths are relative to the location of your config file. For example,
> if your config file is located at `/home/user/.config/qman/qman.conf`, using
> the above directive tells Qman that `modernity.conf` is inside
> `/home/user/.config/qman/themes/`. When in doubt, use absolute paths.

> **:bulb: Note**
>
> Each theme has its own requirements and may not work with all terminal
> emulators. For example, a theme may require support for 8-bit or 24-bit color,
> or a specific Unicode font. These requirements are generally documented in
> the theme's comments.
