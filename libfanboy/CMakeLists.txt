cmake_minimum_required(VERSION 3.5)

project(libfanboy)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_library(fanboy STATIC libfanboy.c
    libfanboy.c
    include/libfanboy.h
    serial.h
    $<IF:$<PLATFORM_ID:Windows>,serial_win32.c,serial_unix.c>
)

target_compile_options(fanboy PRIVATE $<$<C_COMPILER_ID:GNU>:
	-Wall -pedantic -std=gnu99 $<$<CONFIG:Debug>: -O0>>)

target_include_directories(fanboy PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
