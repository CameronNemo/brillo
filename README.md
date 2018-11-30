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

To build the binary `brillo` and install it:

    make
    sudo make install

A full install, including the man page, polkit action, udev rule, and apparmor
profile in addition to the binary:

    make dist
    sudo make install-dist

> Note: the Makefile uses the `PREFIX` and `DESTDIR` variables to compose the
> installation path. If set, they should be consistent for each make target.

Unpriveleged Access
-------------------

### polkit

Active sessions can invoke `brillo` via `pkexec` to escalate priveleges.

Examples:

    pkexec /usr/bin/brillo -O
    pkexec /usr/bin/brillo -A 5

> Note: this requires polkitd and (e)logind or ConsoleKit.

### udev

`brillo`'s udev rule grants necessary permissions to the `video` group for
backlight devices and to the `input` group for keyboard LED devices.

Any user in these groups can modify values directly.

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

Copyright (C) 2018 Cameron Nemo, 2014 Fredrik Haikarainen

This is free software, see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
