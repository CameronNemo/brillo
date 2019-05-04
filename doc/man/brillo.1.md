% brillo(1)
% Cameron Nemo
% OCTOBER 2018
# NAME
brillo - control the brightness of backlight and keyboard LED devices

# SYNOPSIS
**brillo** [**operation** [*value*]] [**-k**] [**-q**|**-r**] [**-m**|**-c**] [**-e**|**-s** *ctrl*] [**-u** *usecs*] [**-v** *loglevel*]

# DESCRIPTION

**brillo** is a tool for controlling the brightness of backlight
and LED devices on Linux. Notable features include:

* Automatic best controller detection
* Smooth transitions and exponential (natural) adjustments
* Ability to save and restore brightness across boots
* Directly using **sysfs** to set brightness without relying on X
* Unprivileged access with no new setuid binaries
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
To select every controller available, use the **-e** option.
To select a specific controller, use the **-s** option.

* **-a**:	Automatic controller selection (default)
* **-e**:	Operate on every controller available
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
can be used for exponential percentages. Exponential mode offers a more
natural and gradual brightness scale.

Raw mode will use the same format and range given by the device driver;
this mode is most useful when a high degree of precision is required,
such as for keyboard controllers.

* **-p**:	Linear percentages (default)
* **-q**:	Exponential percentages
* **-r**:	Raw values

*Smooth adjustment*

**brillo** is capable of gradually adjusting the brightness over a specified
time period. Use the **-u** *microseconds* option to specify how long the operation
should take. This flag is silently ignored when not setting the brightness.

* **-u** *microseconds*:	time used to space the operation out

*Verbosity*

By default, **brillo** outputs only warnings or more severe messages.
To enable more verbose logs on **stderr**, use the verbosity argument:
**-v** *loglevel*.
The loglevel is a value between 0 and 8 (corresponding to syslog severities).

# EXAMPLES

Get the current brightness in percent:

    brillo [-G]

Increase brightness by 5 percent:

    brillo -A 5

Specify the controller to use:

    brillo -s intel_backlight -A 5

Set the brightness to 50% for every controller:

    brillo -e -S 50

Retrieve or increase the brightness using an exponential scale:

    brillo -q
    brillo -q -A 5

Decrease the brightness and smooth the operation over 1500 microseconds:

    brillo -u 150000 -U 5

Get the raw maximum brightness value:

    brillo -rm

Set the minimum cap for the *acpi_video0* controller to a raw value of 2:

    brillo -rc -s acpi_video0 -S 2

*Note*: subsequent attempts to set the controller's brightness to a raw value less than 2 will then be raised to this minimum threshold.

List keyboard controllers:

    brillo -Lk

Activate a specific controller LED:

    brillo -k -s "input15::scrolllock" -S 100

*Note*: LEDs often only take 0 or 1 in raw value (i.e. for off/on). In these cases, you can use any non-zero value instead of 100.

# COPYRIGHT

Copyright (C) 2018-2019 Cameron Nemo, 2014 Fredrik Haikarainen

This is free software, see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
