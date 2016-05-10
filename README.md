# Light

Copyright (C) 2012 - 2014, Fredrik Haikarainen
This is free software, see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE


## Description

Light is a program to control backlight controllers under GNU/Linux, it is the successor of lightscript, which was a bash script with the same purpose, and tries to maintain the same functionality.


## Features

* Works excellent where other software have been proven unusable or problematic, thanks to how it operates internally and the fact that it does not rely on X.
* Can automatically figure out the best controller to use, making full use of underlying hardware.
* Possibility to set a minimum brightness value, as some controllers set the screen to be pitch black at a vaĺue of 0 (or higher).


## Installation

### Arch Linux

If you run Arch Linux, there exists 2 packages;
* [light-git](https://aur.archlinux.org/packages/light-git) - For the absolutely latest version
* [light](https://aur.archlinux.org/packages/light) - For the latest tagged release

I recommend you go with light-git as you might miss important features and bugfixes if you do not.

### Manual

`make && make install`

**Optional:** If you want to use udev rules instead of suid to manage sysfs permissions, you may skip the `make install` step and instead add something like the following to `/etc/udev/rules.d/90-backlight.rules` after copying your binaries:
```
ACTION=="add", SUBSYSTEM=="backlight", RUN+="/bin/chgrp video /sys/class/backlight/%k/brightness"
ACTION=="add", SUBSYSTEM=="backlight", RUN+="/bin/chmod g+w /sys/class/backlight/%k/brightness"
```


## Usage

This application usually has 4 different criteria on flags to use, which are operation modes, value mode, target and controller mode. Flags from these different modes can never be used in conjunction, but all of them do not always have to be specified (although it is recommended to do so for verbosity).

**Note:** This application will only print errors if you are using it incorrectly. If something goes wrong, and you can't figure out why, try setting the verbosity flag with -v:

* 0: No debug output
* 1: Errors
* 2: Errors, warnings
* 3: Errors, warnings, notices

### Operation modes

The operation modes describe **what** you want to do.

* -G: Which **reads/gets** brightness/data from controllers/files
* -S: Which **writes/sets** brightness/data to controllers/files
* -A: Which does like -S but instead **adds** the value
* -U: Which does like -S but instead '**subtracts** the value
* -O: Save the current brightness for later use (usually used on shutdown)
* -I: Restore the previously saved brightness (usually used on boot)
* -L: List the available controllers

When used by themselves operate on the brightness of a controller that is selected automatically. S, A and U needs another argument -- except for the main 4 criteria -- which is the value to set/add/subtract.   This can be specified either in percent or in raw values, but remember to specify the value mode (read below) if you want to write raw values.

### Value modes

The value mode specify in what unit you want to read or write values in. The default one (if not specified) is in percent, the other one is raw mode and should always be used when you need very precise values (or only have a controller with a very small amount of brightness levels).

* -p: Percent
* -r: Raw mode

Remember, this is the unit that will be used when you set, get, add or subtract brightness values.

### Target

As you can not only handle the **brightness** of controllers, you may also specify a target to read/write from/to:

* -b: Current brightness of selected controller
* -m: Maximum brightness of selected controller
* -c: Minimum brightness (cap) of selected controller

The minimum brightness is a feature implemented as some controllers make the screen go pitch black at 0%, if you have a controller like that, it is recommended to set this value (in either percent or in raw mode). These values will be saved in raw mode though, so if you specify it in percent it might not be too accurate depending on your controller.

### Controller modes

Finally, you can either use the built-in controller selection to get the controller with the maximum precision, or you can specify one manually with the -s flag. The -a flag will force automatic mode and is default. Use -L to get a list of controllers to use with the -s flag (to specify which controller to use).

### Examples

Get the current brightness in percent

`light -G`

Increase brightness by 5 percent

`light -A 5`

Set the minimum cap to 2 in raw value on the acpi_video0 controller:

`light -Scrs "acpi_video0" 2`

