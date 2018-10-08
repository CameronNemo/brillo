% brillo(1)
% Cameron Nemo
% OCTOBER 2018
# NAME
brillo - Backlight and Keyboard LED control tool

# SYNOPSIS
**brillo** [**operation** [*value*]] [**-k**] [**-q**|**-r**] [**-m**|**-c**] [**-s** *ctrl*] [**-u** *usecs*] [**-v** *loglevel*]

# DESCRIPTION

**brillo** is a tool for controlling the brightness of backlight
and LED devices on Linux. Notable features include:

* Automatic best controller detection
* Ability to save and restore brightness across boots
* Directly using **sysfs** to set brightness without relying on X
* Unpriveleged access with no new setuid binaries
* Containment with AppArmor

# OPTIONS

*Operations*

* **-G**:	Get brightness value (default)
* **-S** *VALUE*:	Set brightness to value
* **-A** *VALUE*:	Increment brightness by given value
* **-U** *VALUE*:	Decrement brightness by given value
* **-O**:	Store the current brightness
* **-I**:	Restore cached brightness
* **-L**:	List available devices
* **-H**:	Show a short help output
* **-V**:	Report the version

*Controllers*

The default controller is automatically selected to maximize precision.
This can be overridden using the **-s** option.

* **-a**:	Automatic controller selection (default)
* **-s** *CONTROLLER*:	Manual controller selection

The list operation (**-L**) can be used to discover available controllers.

*Targets*

By default, **brillo** acts on the display devices, but the **-k** option
can be used to adjust keyboard backlights instead. In either case, it may be
necessary to specify an alternative controller.

* **-l**:	Act on display backlight (default)
* **-k**:	Act on keyboard backlight and LEDs

*Fields*

By default, **brillo** acts on the brightness property. With these options,
the maximum brightness of a controller can be retrieved. In addition, it is
possible to set (or retrieve) a minimum cap, which is used to prevent
lowering the brightness beyond a certain threshold. This is especially
useful for devices that become pitch black when the brightness is set to 0.

* **-b**:	Current brightness (default)
* **-m**:	Maximum brightness
* **-c**:	Minimum brightness

*Value modes*

Values may be given, or presented, in percent or raw mode.

The default value mode is linear percentages, however the **-q** option
can be used for logarithmic percentages. Logarithmic mode offers a more
natural and gradual brightness scale.

Raw mode will use the same format and range given by the device driver;
this mode is most useful when a high degree of precision is required,
such as for keyboard controllers.

* **-p**:	Linear percentages (default)
* **-q**:	Logarithmic percentages
* **-r**:	Raw values

*Smooth adjustment*

**brillo** is capable of gradually adjusting the brightness over a specified
time period. Use the **-u** *microseconds* option to specify how long the operation
should take. This flag is silently ignored when not setting the brightness.

* **-u** *microseconds*:	microseconds used to space the operation out

*Verbosity*

By default, **brillo** outputs zero debugging information. To enable logs
on **stderr**, use the verbosity argument: **-v** *loglevel*. The loglevel
is a value between 0 and 3, corresponding to the following priorities:

* **0**:	No debug output (default)
* **1**:	Errors
* **2**:	Errors, warnings
* **3**:	Errors, warnings, notices

# EXAMPLES

Get the current brightness in percent:

    brillo [-G]

Increase brightness by 5 percent:

    brillo -A 5

Specify the controller to use:

    brillo -s intel_backlight -A 5

Get the raw maximum brightness value:

    brillo -mr

Set the minimum cap for the *acpi_video0* controller to a raw value of 2:

    brillo -cr -s acpi_video0 -S 2

*Note*: subsequent attempts to set the controller's brightness to a raw value less than 2 will then be raised to this minimum threshold.

List keyboard controllers:

    brillo -Lk

Activate a specific controller LED:

    brillo -k -s "input15::scrolllock" -S 100

*Note*: LEDs often only take 0 or 1 in raw value (i.e. for off/on). In these cases, you can use any non-zero value instead of 100.

Adjust or retrieve the brightness using a logarithmic scale:

    brillo -q
    brillo -q -A 5

# COPYRIGHT

Copyright (C) 2018 Cameron Nemo, 2014 Fredrik Haikarainen

This is free software, see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
