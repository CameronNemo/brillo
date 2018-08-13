brillo - Backlight and Keyboard LED control tool
==================================================

- [Introduction](#introduction)
- [Examples](#examples)
- [Usage](#usage)
  - [Operation mode](#operation-mode)
  - [Value mode](#value-mode)
  - [Target](#target)
  - [Field](#field)
  - [Controller](#controller)
- [Installation](#installation)
- [Unpriveleged Access](#unpriveleged-access)
  - [polkit](#polkit)
  - [udev](#udev)
- [Copyright](#copyright)


Introduction
------------

`brillo` is a program to control backlight controllers on Linux.

Notable features include:

* Automatic best controller detection
* Ability to save and restore brightness across boots
* Directly using `sysfs` to set brightness without relying on X
* Unpriveleged access with no new setuid binaries
* Containment with AppArmor

Let's get started with a few examples; see below for the
full description of available commands and options.

Examples
--------

Get the current brightness in percent

    brillo -G

or

    brillo

Increase brightness by 5 percent

    brillo -A 5

Set the minimum cap to 2 in raw value on the `acpi_video0` controller:

    brillo -cr -s acpi_video0 -S 2

Try to set the brightness to 0 after that, it will be changed to the
minimum 2:

    brillo -r -s acpi_video0 -S 0

Find keyboard controllers:

    brillo -k -L

Activate `ScrollLock` LED, here `input15` is used, but this varies
between different systems:

    brillo -k -s "input15::scrolllock" -S 100

Usually, LEDs only take 0 or 1 in raw value (i.e. for off/on), so you
can instead write:

    brillo -kr -s "input15::scrolllock" -S 1

Verify by reading back the max brightness, you should get a value of 1:

    brillo -kr -m -s "input15::scrolllock


Usage
-----

### Operation mode

* `-G`: Get (read) brightness/data from controllers/files
* `-S VAL`: Set (write)brightness/data to controllers/files
* `-A VAL`: Like `-S`, but adds the given value
* `-U VAL`: Like `-S`, but subtracts the given value
* `-O`: Save the current brightness for later use (usually used on shutdown)
* `-I`: Restore the previously saved brightness (usually used on boot)
* `-L`: List available controllers, see below `-k` option as well

Without any options (below) the commands operate on the brightness of an
automatically selected controller.  Values are given in percent, unless
the below `r` option is also given.

**Note:** like most UNIX applications, brillo only gives output on
  errors.  If something goes wrong try the verbosity option `-v VAL`:

* 0: No debug output
* 1: Errors
* 2: Errors, warnings
* 3: Errors, warnings, notices

### Value mode

Values may be given, or presented, in percent or raw mode.  Raw mode is
the format specific to the controller.  The default is in percent, but
raw mode may be required for precise control, or when the steps are very
few, e.g. for most keyboard backlight controllers.

* `-p`: Percent, default
* `-r`: Raw mode

### Target

By default the screen is the active target for all commands, use `-k` to
select the keyboard instead.  In either case you may need to select a
different controller, see below.

* `-l`: Act on screen backlight, default
* `-k`: Act on keyboard backlight and LEDs

### Field

By default commands act on the brightness property, which is read+write.
The maximum brightness is a read-only property.  The minimum brightness
cap is a feature implemented to protect against setting brightness too
low, since some controllers make the screen go pitch black at 0%.  For
controllers like that it is recommended to set this value.

* `-b`: Current brightness of selected controller, default
* `-m`: Max. brightness of selected controller
* `-c`: Min. brightness (cap) of selected controller (recommend raw mode)

### Controller

The default controller is automatically selected for maximum precision, but this can be overridden.

* `-a`: Automatic controller selection (default)
* `-s ARG`: Manual controller selection

The `-L` and `-Lk` commands list available controllers for the backlight and keyboard targets, respectively.

Installation
------------

To build the binary `brillo` and install it:

    make
    sudo make install

A full install, generating the man page, polkit action, udev rule, and apparmor profile in addition to the binary:

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

`brillo`'s udev rule grants necessary permissions to the `video` group.

Any user in this group can modify the brightness directly.

> Note: in this mode, stored brightness and minimum cap files will be in the user's cache home (typically "~/.cache").

Copyright
-------------------

Copyright (C) 2018 Cameron Nemo, 2014 Fredrik Haikarainen

This is free software, see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
