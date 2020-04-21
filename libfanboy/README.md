# FanBoy ![FanBoy Logo](https://github.com/lynix/fanboy/blob/master/artwork/logo.png)

Open Source PWM Fan Controller

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://travis-ci.org/lynix/fanboy.svg?branch=master)](https://travis-ci.org/lynix/fanboy)


## Component: libfanboy

*libfanboy* is a static library written in C that provides a cross-platform
abstraction of the binary serial protocol between *FanBoy* and host.

### Building

*libfanboy* uses [CMake](https://cmake.org) to provide a uniform cross-platform
build that can be integrated in other projects.

In order to build it separately (example on Linux):

```
$ cd libfanboy
$ cmake .
$ make
```


## License

This project is published under the terms of the *MIT License*. See the file
`LICENSE` for more information.
