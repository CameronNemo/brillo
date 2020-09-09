Introduction
------------

`brillo` controls the brightness of backlight and LED devices on Linux.

Notable features include:

* Automatic best controller detection
* Smooth transitions and natural brightness adjustments
* Ability to save and restore brightness across boots
* Directly using `sysfs` to set brightness without relying on X
* Unprivileged access with no new setuid binaries
* Containment with AppArmor

For detailed usage, please refer to the [man page](doc/man/brillo.1.md).

Installation
------------

[![Packaging status](https://repology.org/badge/vertical-allrepos/brillo.svg)](https://repology.org/project/brillo/versions)

### From source

Building the manpage requires `go-md2man`.

To build and install the binary, manpage, and udev rule:

```
$ make
# make install
```

Users in the configured group (`video` by default) will be able to adjust
the brightness.
An alternative group can be configured using the `GROUP` variable:

```
# make install GROUP=_brillo
```

If you would prefer to allow any user, regardless of group, to change the
brightness, you may install the binary as `setgid`:

```
# make install.setgid GROUP=_brillo
```

To additionally install the apparmor profile:

```
# make install.apparmor
```

To additionally install the polkit rule:

```
# make install.polkit
```

> Note: the `install*` targets use the `PREFIX` and `DESTDIR` variables to
>       compose the installation path and generate configuration files.

Unprivileged Access
-------------------

### polkit

Active sessions can invoke `brillo` via `pkexec` to escalate priveleges.

Examples:

```
$ pkexec /usr/bin/brillo -O
$ pkexec /usr/bin/brillo -A 5
```

> Note: this requires polkitd and (e)logind or ConsoleKit.

### udev

`brillo`'s udev rule grants necessary permissions to the `video` group for
backlight and keyboard LED devices.

Users in these groups can modify values directly.
Alternately, the binary may be installed as `setgid` so that any user may
change the brightness.

> Note: in this mode, stored brightness and minimum cap files will be in
> the user's cache home (typically "~/.cache").

See Also
--------

[clight](https://github.com/FedeDP/Clight): provides dimming based on ambient
light and color correction based on location.

[ddcci-driver-linux](https://gitlab.com/ddcci-driver-linux/ddcci-driver-linux):
exposes external monitor brightness in sysfs, so `brillo` can access them.

[ddcutil](http://www.ddcutil.com/): designed to control brightness and color
correction for external monitors.

Copyright
---------

Copyright (C) 2018-2019 Cameron Nemo, 2014 Fredrik Haikarainen

This is free software, see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
