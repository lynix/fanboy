# FanBoy ![FanBoy Logo](https://github.com/lynix/fanboy/blob/master/artwork/logo.png)

Open Source PWM Fan Controller

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://travis-ci.org/lynix/fanboy.svg?branch=master)](https://travis-ci.org/lynix/fanboy)


## Component: PCB

*FanBoy*'s core is the *Micro* version of an *Arduino Leonardo* (e.g.
*Paradisetronic Pro Micro* or *SparkFun Pro Micro*) carried by a very simple
PCB:

![FanBoy](https://github.com/lynix/fanboy/blob/master/artwork/fanboy.jpg)

The board has been designed using [Fritzing](https://fritzing.org) and can be
manufactured easily via common PCB fabrication services.

**Note:** The Fritzing file makes use of the 'SparkFun Pro Micro' part created
by [Hauke Thorenz](https://github.com/htho/fritzing-parts).

### Board Description

| Connector            | Description                                                        |
|:-------------------- |:------------------------------------------------------------------ |
| `12V`                | 12&thinsp;V DC input                                               |
| `TEMP1`, `TEMP2`     | Temperature sensor connectors                                      |
| `FAN1-4`             | PWM fan connectors                                                 |
| `R_RPM1-4`           | Pull-up resistors for fan RPM signal (4.7&thinsp;k&Omega;)         |
| `R_TEMP1`, `R_TEMP2` | Thermistor reference resistors (10&thinsp;k&Omega;)                |
| `PWR`                | Arduino power supply selector (open: USB, shorted: 12&thinsp;V DC) |

:warning: **Warning:** When using cheap *Pro Micro* clones like the ones from
Paradisetronic make sure that solder bridge `J1` is closed! You otherwise risk
frying the Arduino when connecting USB and 12V at the same time!

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


## License

This project is published under the terms of the *MIT License*. See the file
`LICENSE` for more information.
