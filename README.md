# FanBoy ![FanBoy Logo](https://github.com/lynix/fanboy/blob/master/artwork/logo.png)

Open Source PWM Fan Controller

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://travis-ci.org/lynix/fanboy.svg?branch=master)](https://travis-ci.org/lynix/fanboy)


## Summary

*FanBoy* is an Open Source DIY approach to providing a cross-platform PWM fan
controller with temperature sensor support that can easily be modified to fit
your needs.


## Features

* **Four discrete PWM channels**  
    Supports multiple fans per channel (up to 6&thinsp;W max. total power draw
    per channel or 24&thinsp;W over all channels), auto-detection for
    disconnected fans and RPM sensing
* **Two temperature sensor inputs**  
    Supports standard 10&thinsp;k&Omega; thermistors
* **Based on well-known Arduino platform**  
    Uses the ATmega32U4 of an Arduino Leonardo as MCU for minimal development
    overhead
* **Multiple operation modes**  
    Including *fixed duty*, *linear* and *target temperature*
    <sup>1</sup>
* **Persistent data storage**  
    Stores all settings as well as last operation mode CRC-protected in EEPROM
* **Simple plaintext protocol**  
    Can be configured and queried using your favorite serial terminal or simple
    shell I/O functionality

<sup>1</sup> Planned but not implemented yet

## Hardware

*FanBoy*'s core is the *Micro* version of an *Arduino Leonardo* (e.g.
*Paradisetronic Pro Micro* or *SparkFun Pro Micro*) carried by a very simple
PCB:

![FanBoy](https://github.com/lynix/fanboy/blob/master/artwork/fanboy.jpg)

The board has been designed using [Fritzing](https://fritzing.org) and can be
manufactured very cheap via common PCB fabrication services.

### Board description

| Connector            | Description                                                        |
|:-------------------- |:------------------------------------------------------------------ |
| `12V`                | 12&thinsp;V DC input                                               |
| `TEMP1`, `TEMP2`     | Temperature sensor connectors                                      |
| `FAN1-4`             | PWM fan connectors                                                 |
| `R_RPM1-4`           | Pull-up resistors for fan RPM signal (4.7&thinsp;k&Omega;)         |
| `R_TEMP1`, `R_TEMP2` | Thermistor reference resistors (10&thinsp;k&Omega;)                |
| `PWR`                | Arduino power supply selector (open: USB, shorted: 12&thinsp;V DC) |

:warning: **Warning:** Never select 12&thinsp;V DC power supply for the Arduino
(shorted PWR pins) and have USB connected at the same time! This will fry the
Arduino!

### Parts list

In order to build your own *FanBoy* you need the following:

* Arduino Leonardo, Micro version
* *FanBoy* PCB
* 4x 4.7&thinsp;k&Omega; resistors
* 2x 10&thinsp;k&Omega; resistors
* 4x KF2510 3+1pin fan headers
* 2x 2.54&thinsp;mm 2pin thermistor headers
* 2x 2.54&thinsp;mm 12pin headers for Arduino
* 1x 3.5&thinsp;mm DC 2pin header


## Firmware

The firmware is based on the official Arduino core. Previous attempts of using
[avr-libc](https://www.nongnu.org/avr-libc) instead have shown to require too
much effort as the ATmega32U4 requires a USB stack for its serial interface.
[LUFA](https://github.com/Palatis/Arduino-Lufa) may be a solution to this and
shall be evaluated.

### Building

In order to avoid having to use the official *Arduino IDE* the firmware makes
use of the
[Arduino-Makefile](https://github.com/sudar/Arduino-Makefile) project.

Make sure you have the following installed (package names may vary between
distros): `arduino`, `arduino-mk`

The firmware can be built and uploaded as follows:

```
$ cd firmware
$ make
$ make upload
```

You may use `make monitor` to open a serial TTY for interaction with the
device (requires `screen` to be installed).

## Usage

*FanBoy* uses a standard 8N1 serial line with 57600 baud over USB and is
therefore compatible with any operating system that has a USB TTY driver.

The following example sets up the serial port and configures Fan 2 for 75% duty
on Linux:

```
$ stty -F /dev/ttyACM0 57600 cs8 -cstopb -parenb
$ echo 'set 2 75' > /dev/ttyACM0
```

### Commands

The serial interface supports the following commands:

| Command              | Description                                                                      |
|:-------------------- |:-------------------------------------------------------------------------------- |
| `set FAN DUTY`       | Set fixed duty of given fan (1-4) to given value (in percent)                    |
| `status [INT]`       | Print fan and temperature sensor status (INT: interval in seconds, 0=off)        |
| `curve`              | Start fan curve scan (output as CSV)                                             |
| `save`               | Save current settings to EEPROM                                                  |
| `load`               | Load settings from EEPROM (done automatically on power on)                       |
| `map FAN [SENSOR]`   | Set/show fan -> sensor mapping for temperature-based fan control                 |
| `linear FAN [PARAM]` | Set/show parameters for linear temperature-based fan control (format see below)  |
| `help`               | Print list of available commands                                                 |
| `version`            | Show firmware version and build information                                      |

#### Linear Parameters

The configuration string for linear temperature-based fan control have the form of

```
Tmin,Dmin,Tmax,Dmax
```

with

| Variable | Description                      |
|:-------- |:-------------------------------- |
| `Tmin`   | Lower temperature                |
| `Dmin`   | Duty value for lower temperature |
| `Tmax`   | Upper temperature                |
| `Dmax`   | Duty value for upper temperature |


## Code

The code uses 4-space indentation and K&R style bracing. Usage of Arduino
libraries is kept low to keep the possibility of switching to avr-libc.
Documentation is done using [Doxygen](http://www.doxygen.nl) blocks in
`decl.h`.


## Bugs / Contact

Pull requests are always welcome. Feel free to report bugs or post questions
using the *issues* function on GitHub.


## License

This project is published under the terms of the *MIT License*. See the file
`LICENSE` for more information.
