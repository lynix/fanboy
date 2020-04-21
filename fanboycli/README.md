# FanBoy ![FanBoy Logo](https://github.com/lynix/fanboy/blob/master/artwork/logo.png)

Open Source PWM Fan Controller

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://travis-ci.org/lynix/fanboy.svg?branch=master)](https://travis-ci.org/lynix/fanboy)


## Component: fanboycli

*fanboycli* is a command line interface application for *FanBoy* written in C
that makes use of *libfanboy*.

### Building

*fanboycli* uses [CMake](https://cmake.org) to provide a uniform cross-platform
build.

In order to build it separately (example on Linux):

```
$ cd fanboycli
$ cmake .
$ make
```

### Usage

The executable understands the following arguments:

| Argument  | Description                                              |
|:----------|:---------------------------------------------------------|
| `-s`      | Show current fan / sensor readings                       |
| `-c`      | Show current configuration                               |
| `-f FAN`  | Select fan to control (1-4)                              |
| `-d DUTY` | Set selected fan to fixed duty (0-100)                   |
| `-m MODE` | Set fan control mode (*manual* or *linear*)              |
| `-M TEMP` | Set mapped sensor no. (1-2)                              |
| `-l PARA` | Set linear control parameters (format see below)         |
| `-L`      | Load configuration from EEPROM                           |
| `-S`      | Save current configuration to EEPROM                     |
| `-C`      | Generate fan curves as CSV samples (duty vs. RPM)        |
| `-R`      | Reset FanBoy (re-initializes USB as well)                |
| `-D DEV`  | Set serial interface (default value depends on platform) |
| `-V`      | Show FanBoy firmware version and build timestamp         |
| `-h`      | Show usage help text                                     |

Note that some argument(s) may be repeated for combination (see below).

#### Examples

Show current readings:

```
$ fanboycli -s
```

Set Fan 1 to 25% fixed duty, Fan 2 to 50% and make changes permanent:

```
$ fanboycli -f 1 -d 25 -f 2 -d 50 -S
```

Set Fan 3 to linear fan control with given parameters:

```
$ fanboycli -f 3 -l 20:19.5:80:40.5
```

### Linear Fan Control

Linear fan control makes the fan duty follow a linear curve between a given
minimum and maximum value.

Parameter format for `-l` argument: `LOW_DUTY:LOW_TEMP:HIGH_DUTY:HIGH_TEMP`

* `LOW_TEMP`: Low temperature
* `HIGH_TEMP`: High temperature
* `LOW_DUTY`: Fan duty applied when temperature <= `TEMP_LOW`
* `HIGH_DUTY`: Fan duty applied when temperature >= `TEMP_HIGH`


## License

This project is published under the terms of the *MIT License*. See the file
`LICENSE` for more information.
