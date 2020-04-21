# FanBoy ![FanBoy Logo](https://github.com/lynix/fanboy/blob/master/artwork/logo.png)

Open Source PWM Fan Controller

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://travis-ci.org/lynix/fanboy.svg?branch=master)](https://travis-ci.org/lynix/fanboy)


## Component: Firmware

The firmware is based on the official Arduino core. Previous attempts of using
[avr-libc](https://www.nongnu.org/avr-libc) turned out to require too much
effort as the ATmega32U4 requires a USB stack for its serial interface.
[LUFA](https://github.com/Palatis/Arduino-Lufa) may be a solution to this and
shall be evaluated.

### Building

The firmware can either be built and uploaded using the official *Arduino* IDE
or using the provided Makefile that is based on
[Arduino-Makefile](https://github.com/sudar/Arduino-Makefile).

In case of the latter, make sure you have the following installed (package names
may vary between distros): `arduino`, `arduino-mk`

The firmware can be built and uploaded as follows:

```
$ cd firmware
$ make
$ make upload
```


## License

This project is published under the terms of the *MIT License*. See the file
`LICENSE` for more information.
