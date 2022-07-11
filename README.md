# FanBoy ![FanBoy Logo](https://github.com/lynix/fanboy/blob/master/artwork/logo.png)

Open Source PWM Fan Controller

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://github.com/lynix/fanboy/actions/workflows/build.yml/badge.svg)](https://github.com/lynix/fanboy/actions/workflows/build.yml)


## Summary

*FanBoy* is an Open Source DIY approach to providing a cross-platform PWM fan
controller with temperature sensor support that can easily be modified to fit
your needs.

![FanBoy](https://github.com/lynix/fanboy/blob/master/artwork/fanboy.jpg)

## Features

* **Four discrete PWM channels**  
    Supports multiple fans per channel (up to 40&thinsp;W max. total power
    draw), auto-detection for disconnected fans and RPM sensing
* **Two temperature sensor inputs**  
    Supports standard 10&thinsp;k&Omega; thermistors
* **Based on well-known Arduino platform**  
    Uses the ATmega32U4 of an Arduino Leonardo as MCU for minimal development
    overhead
* **Multiple operation modes per channel**  
    including *fixed duty*, *linear* and *target temperature*
    <sup>1</sup>
* **Persistent data storage**  
    Stores all settings as well as last operation mode CRC-protected in EEPROM
* **Simple serial protocol**  
    Comes with a simple static library for communication abstraction as well
    as a command line utility for configuration

<sup>1</sup> Planned but not implemented yet

## Components

This DIY kit consists of multiple components that are reflected as
subdirectories:

| Folder                             | Description                                                                 | Platforms         |
|:-----------------------------------|:----------------------------------------------------------------------------| :---------------- |
| [pcb](https://github.com/lynix/fanboy/tree/master/pcb)             | PCB that can be manufactured very cheap via common PCB fabrication services | Linux, Win32, Mac |
| [firmware](https://github.com/lynix/fanboy/tree/master/firmware)   | Firmware based on *Arduino* Core                                            | Linux, Win32, Mac |
| [libfanboy](https://github.com/lynix/fanboy/tree/master/libfanboy) | Static C library that implements serial interface between host and *FanBoy* | Linux, Win32, Mac |
| [enclosure](https://github.com/lynix/fanboy/tree/master/enclosure) | Simple 3D printable enclosure that fits a 2.5" drive slot                   | -                 |
| [fanboycli](https://github.com/lynix/fanboy/tree/master/fanboycli) | Command line client based on `libfanboy`                                    | Linux, Win32, Mac |

:information_source: In addition to these components there is a Qt based GUI
called [FanMan](https://github.com/lynix/fanman).

## Contributing

### Code

The code uses 4-space indentation and K&R style bracing. Usage of Arduino
libraries is kept low to keep the possibility of switching to avr-libc. Code
documentation is done inline (headers) using [Doxygen](http://www.doxygen.nl).


### Bugs / Features

Pull requests are always welcome. Feel free to report bugs or post questions
using the *issues* function on GitHub.


## License

This project is published under the terms of the *MIT License*. See the file
`LICENSE` for more information.
