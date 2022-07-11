/* Copyright (c) 2020 Alexander Koch
 *
 * This file is part of a project that is distributed under the terms of the MIT
 * License, see file 'LICENSE'.
 */

#ifndef _DECL_H
#define _DECL_H

#include <stdint.h>

#include "serial.h"


/**
 * @brief Integrity shell for saving/loading settings to/from EEPROM
 *
 *   magic:  Predefined magic constant for fast record checking
 *   opts:   Settings structure, @see config_t
 *   crc:    CRC8 covering `opts`
 */
struct eeprom_t
{
    uint8_t       magic;
    config_t      opts;
    uint8_t       crc;
};

/**
 * @brief CRC8 helper function
 *
 * Used to protect settings stored in EEPROM from bit-rot.
 *
 * @param[in]  data  Pointer to beginning of data to cover
 * @param      len   Number of bytes to cover
 * @returns    8-bit check value for given data
 */
uint8_t crc8(const uint8_t *data, uint16_t len);

/**
 * @brief Determine current fan RPM
 *
 * Uses Arduino's `pulseIn()` mechanism to measure the length of the LOW-pulse
 * emitted by the Hall sensor of the fan.
 *
 * @param    fan  Fan no.
 * @returns  Current fan speed in RPM
 * @note     This function can take a considerable amount of time
 *           (waiting for the next LOW pulse), avoid calling it for
 *           unconnected fans.
 */
uint16_t get_rpm(uint8_t fan);

/**
 * @brief Determine current sensor temperature
 *
 * @param    sensor  Sensor no.
 * @returns  Current temperature multiplied by 100 as integer
 */
uint16_t get_temp(uint8_t sensor);

/**
 * @brief Set fan duty
 *
 * @param  fan    Fan no.
 * @param  value  Duty value to apply, in percent (0-100)
 */
void set_duty(uint8_t fan, uint8_t value);

/**
 * @brief Set duty according linear control curve for given fan
 *
 *       Fan Duty
 *          ^
 *          |
 *   d_max  -               ------
 *          |              /
 *          |             /
 *          |            /
 *          |           /
 *          |          /
 *   d_min  ----------/
 *          |
 *          +---------|-----|---------> Temperature
 *                  t_min  t_max
 *
 * @param  fan  Fan no.
 */
void set_duty_linear(uint8_t fan);

/**
 * @brief Save current settings to EEPROM
 */
void opts_save();

/**
 * @brief Load and apply settings from EEPROM
 *
 * @returns  `true` on success, `false` if no settings found in EEPROM
 */
bool opts_load();

/**
 * @brief Detect connected fans
 *
 * Ramps up all fans to fixed duty (`SCAN_DUTY`) and checks for valid RPM
 * signal. Marks fans without the latter as unconnected. This is used to save
 * execution time consumed by unnecessary calls to `get_rpm()`.
 */
void fan_scan();

/**
 * @brief Detect connected fans
 *
 * Determines fan characteristic by ramping duty values from 100% down to 0% in
 * `CURVE_STEP`% steps, taking `CURVE_SAMPLE_NUM` samples of the fan RPM. Saves
 * measured RPM values in serial buffer.
 */
void fan_curve();

/**
 * @brief Handle serial communication
 */
void handle_serial();

/**
 * @brief Reset MCU
 *
 * Uses the integrated watchdog to hard-reset the device, i.e. jump to boot
 * loader. This clears all registers and re-initializes USB.
 */
 void reset();

#endif

/* vim: set ts=4 sw=4 et */
