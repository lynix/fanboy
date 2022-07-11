/* Copyright (c) 2020 Alexander Koch
 *
 * This file is part of a project that is distributed under the terms of the MIT
 * License, see file 'LICENSE'.
 */

#ifndef _LIBFANBOY_H
#define _LIBFANBOY_H

#include "firmware/serial.h"

typedef msg_status_t     fb_status_t;
typedef msg_version_t    fb_version_t;
typedef msg_config_t     fb_config_t;
typedef msg_fan_curve_t  fb_curve_t;
typedef linear_t         fb_linear_t;
typedef pidc_t           fb_pid_t;

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

/**
 * @brief Initialize library and serial communication
 *
 * @param[in] dev  Serial device name
 *
 * @return true on success, false otherwise
 *
 * @note In case of failure this function makes an error message available to
 *       be retrieved using `fb_error()`.
 */
bool fb_init(const char *dev);

/**
 * @brief Un-initialize library (i.e. free resources)
 */
void fb_exit();

/**
 * @brief Get current fan and temperature sensor status
 *
 * @param[out] result  Buffer to write values to
 *
 * @return true on success, false otherwise
 *
 * @note In case of failure this function makes an error message available to
 *       be retrieved using `fb_error()`.
 */
bool fb_status(fb_status_t *result);

/**
 * @brief Get firmware version and build timestamp
 *
 * @param[out] result  Buffer to write version structure to
 *
 * @return true on success, false otherwise
 *
 * @note In case of failure this function makes an error message available to
 *       be retrieved using `fb_error()`.
 */
bool fb_version(fb_version_t *result);

/**
 * @brief Get device configuration (fan modes, parameters, etc.)
 *
 * @param[out] result  Buffer to write config structure to
 *
 * @return true on success, false otherwise
 *
 * @note In case of failure this function makes an error message available to
 *       be retrieved using `fb_error()`.
 */
bool fb_config(fb_config_t *result);

/**
 * @brief Set fan mode
 *
 * @param fan   No. of fan to set mode for (counted by zero)
 * @param mode  Mode to apply (`MODE_MANUAL` or `MODE_LINEAR`)
 *
 * @return true on success, false otherwise
 *
 * @note In case of failure this function makes an error message available to
 *       be retrieved using `fb_error()`.
 */
bool fb_set_mode(uint8_t fan, fan_mode_t mode);

/**
 * @brief Set manual fan duty
 *
 * @param fan   No. of fan to set duty for (counted by zero)
 * @param duty  Manual duty value in percent (0-100)
 *
 * @return true on success, false otherwise
 *
 * @note Setting manual fan duty implies setting fan to mode `MODE_MANUAL`.
 *
 * @note In case of failure this function makes an error message available to
 *       be retrieved using `fb_error()`.
 */
bool fb_set_duty(uint8_t fan, uint8_t duty);

/**
 * @brief Set fan <-> sensor mapping
 *
 * @param fan     No. of fan to set mapping for (counted by zero)
 * @param sensor  No. of sensor to be mapped to (counted by zero)
 *
 * @return true on success, false otherwise
 *
 * @note In case of failure this function makes an error message available to
 *       be retrieved using `fb_error()`.
 */
bool fb_set_map(uint8_t fan, uint8_t sensor);

/**
 * @brief Generate fan duty <-> RPM correlation data
 *
 * @param[out] result  Buffer to write data to
 *
 * @return true on success, false otherwise
 */
bool fb_fan_curve(fb_curve_t *result);

/**
 * @brief Set linear fan control parameters
 *
 * @param     fan    No. of fan to set parameters for (counted by zero)
 * @param[in] param  Control parameter values
 *
 * @return true on success, false otherwise
 *
 * @note In case of failure this function makes an error message available to
 *       be retrieved using `fb_error()`.
 */
bool fb_set_linear(uint8_t fan, fb_linear_t *param);

/**
 * @brief Set PID fan control parameters
 *
 * @param     fan    No. of fan to set parameters for (counted by zero)
 * @param[in] param  Control parameter values
 *
 * @return true on success, false otherwise
 *
 * @note In case of failure this function makes an error message available to
 *       be retrieved using `fb_error()`.
 */
bool fb_set_pid(uint8_t fan, fb_pid_t *param);

/**
 * @brief Save current configuration to EEPROM
 *
 * @return true on success, false otherwise
 *
 * @note In case of failure this function makes an error message available to
 *       be retrieved using `fb_error()`.
 */
bool fb_save();

/**
 * @brief Load and apply configuration from EEPROM
 *
 * @return true on success, false otherwise
 */
bool fb_load();

/**
 * @brief Trigger device reset
 *
 * @note It is required to tear down and re-initialize the library (`fb_exit()`
 *       and `fb_init()`) after calling this function.
 */
void fb_reset();

/**
 * @brief Get message indicating latest error
 *
 * @return Latest error message
 */
const char *fb_error();

#ifdef __cplusplus
}
#endif

#endif
