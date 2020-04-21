cmake_minimum_required(VERSION 3.5)

project(fanboycli)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_subdirectory(../libfanboy libfanboy)
set_property(TARGET fanboy PROPERTY POSITION_INDEPENDENT_CODE ON)

add_executable(fanboycli main.c)

target_compile_options(fanboycli PRIVATE $<$<C_COMPILER_ID:GNU>:
    -Wall -pedantic -std=gnu99 $<$<CONFIG:Debug>: -O0>>)

target_link_libraries(fanboycli fanboy)

install(TARGETS fanboycli DESTINATION bin)
