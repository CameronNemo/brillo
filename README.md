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
- [Copyright](#copyright)


Introduction
------------

`brillo` is a program to control backlight controllers on Linux.

Notable features include:

* Automatic best controller detection
* Ability to save and restore brightness across boots
* Directly using `sysfs` to set brightness without relying on X
* Unpriveleged access with no new setuid binaries

Let's get started with a few examples, for details, see below for the
full description of the different commands, options and how to access
different controllers.


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

Typical build and installation process:

    make dist
    # PREFIX is /usr by default
    sudo make install

To simply compile the binary without installing, run `make`. Note that unpriveleged access may not be available.

Unpriveleged Access
-------------------

### polkit

Any active usercan invoke `brillo` through the `pkexec` command to escalate priveleges to root. This will only work from an active session created by (e)logind or ConsoleKit.

Examples:

    pkexec brillo -O
    pkexec brillo -A 5

### udev

`brillo` uses a udev rule to grant necessary permissions to the `video` group. Any user in this group can modify the brightness directly without the use of `pkexec`.

>Note: two features, storing brightness and setting mincap values, will only work with one user at a time in this mode.

### lightscript

`brillo` is a fork of the popular [light](https://github.com/haikarainen/light) program. A compatibility script is provided that is a drop-in replacement for `light`. To install it, run `sudo make install-lightscript`. This is a wrapper around the polkit access method.

Copyright
-------------------

Copyright (C) 2018 Cameron Nemo, 2014 Fredrik Haikarainen

This is free software, see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
