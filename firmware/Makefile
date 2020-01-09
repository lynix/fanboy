# Copyright (c) 2019 Alexander Koch
#
# This file is part of a project that is distributed under the terms of
# the MIT License, see file 'LICENSE'.

BOARD_TAG = leonardo
ARDUINO_PORT ?= /dev/ttyACM0
MONITOR_BAUDRATE = 57600

ARDUINO_QUIET = 1
OPTIMIZATION_LEVEL = 3
OBJDIR = build
CXXFLAGS = -Wno-deprecated-declarations

OS := $(shell egrep '^ID=' /etc/os-release 2>/dev/null | cut -d '=' -f 2)
ifeq ($(OS), arch)
ARDMK_VENDOR := archlinux-arduino
endif

GIT_VERSION := $(shell git describe --exact-match 2>/dev/null)
ifndef GIT_VERSION
GIT_VERSION := $(shell git rev-parse --short HEAD 2>/dev/null)
endif
ifdef GIT_VERSION
CPPFLAGS += -DVERSION="\"$(GIT_VERSION)\""
endif

include /usr/share/arduino/Arduino.mk

CXXFLAGS := $(filter-out -fpermissive,$(CXXFLAGS))
