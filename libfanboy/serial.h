/* Copyright (c) 2020 Alexander Koch
 *
 * This file is part of a project that is distributed under the terms of the MIT
 * License, see file 'LICENSE'.
 */

#ifndef _FB_SERIAL_H
#define _FB_SERIAL_H

/**
 * @file
 * @brief Platform abstraction for serial I/O
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif


/**
 * @brief Open serial interface, set connection parameters (baud rate, parity,
 *        etc.)
 *
 * @param[in] dev  Serial device to use
 *
 * @return true on success, false otherwise
 */
bool serial_open(const char *dev);

/**
 * @brief Close serial interface
 */
void serial_close();

/**
 * @brief Send data via serial interface
 *
 * @param[in] data  Pointer to data buffer to send
 * @param     len   Number of bytes to read from buffer
 *
 * @return true on success, false otherwise
 */
bool serial_send(const void *data, size_t len);

/**
 * @brief Receive data from serial interface
 *
 * @param[out] data     Pointer to data buffer
 * @param      len      Number of bytes to receive
 * @param      retries  Retries per timed-out read() call
 *
 * @return true if requested amount of bytes has been received, false otherwise
 */
bool serial_receive(void *data, size_t len, int retries);

#ifdef __cplusplus
}
#endif

#endif
