Introduction
------------

`brillo` controls the brightness of backlight and LED devices on Linux.

Notable features include:

* Automatic best controller detection
* Ability to save and restore brightness across boots
* Directly using `sysfs` to set brightness without relying on X
* Unpriveleged access with no new setuid binaries
* Containment with AppArmor

For detailed usage, please refer to the [man page](doc/man/brillo.1.md).

Installation
------------

To build the binary `brillo` and install it:

    make
    sudo make install

A full install, including the man page, polkit action, udev rule, and apparmor profile in addition to the binary:

    make dist
    sudo make install-dist

> Note: the Makefile uses `PREFIX` and `DESTDIR` variables to build the installation path. If specified, they should be consistent between each command.

Unpriveleged Access
-------------------

### polkit

Active sessions can invoke `brillo` via `pkexec` to escalate priveleges.

Examples:

    pkexec brillo -O
    pkexec brillo -A 5

> Note: this requires polkitd and (e)logind or ConsoleKit.

### udev

`brillo`'s udev rule grants necessary permissions to the `video` group for backlight devices and to the `input` group for keyboard LED devices.

Any user in these groups can modify values directly.

> Note: in this mode, stored brightness and minimum cap files will be in the user's cache home (typically "~/.cache").

See Also
--------

[clight](https://github.com/FedeDP/Clight): provides dimming based on ambient light and color correction (like redshift or flux) using GeoClue

[ddcutil](http://www.ddcutil.com/): designed to control brightness and color correction for external monitors

[ddcci-driver-linux](https://gitlab.com/ddcci-driver-linux/ddcci-driver-linux): exposes external monitor brightness in sysfs, so it can be used by `brillo` and similar programs

Copyright
---------

Copyright (C) 2018 Cameron Nemo, 2014 Fredrik Haikarainen

This is free software, see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
