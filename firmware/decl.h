/* Copyright (c) 2019 Alexander Koch
 *
 * This file is part of a project that is distributed under the terms of
 * the MIT License, see file 'LICENSE'.
 */

#ifndef _DECL_H
#define _DECL_H

#include <stdint.h>

#include "defs.h"


/**
 * @brief Operation mode
 *
 * Operation mode abstraction. May be one of:
 *
 *   MODE_MANUAL:  Fan duty cycles set to fixed value
 *   MODE_LINEAR:  Linear curve between two points, flat-out
 *   MODE_TARGET:  Target temperature for PID-controlled fan duty
 */
enum mode_t
{
    MODE_MANUAL = 0,
    MODE_LINEAR = 1,    // TODO: implement me
    MODE_TARGET = 2     // TODO: implement me
};

/**
 * @brief Settings structure
 *
 *   mode:       Operation mode, @see mode_t
 *   fan_duty:   Array of fixed fan duties for `MODE_MANUAL`
 *   fan_map:    Temperature sensor selection for each fan
 *   stats_int:  Interval for status output via serial
 */
struct opts_t
{
    mode_t     mode;
    uint8_t    fan_duty[NUM_FAN];
    uint8_t    fan_map[NUM_FAN];
    uint8_t    stats_int;
};

/**
 * @brief Integrity shell for saving/loading settings to/from EEPROM
 *
 *   magic:  Predefined magic constant for fast record checking
 *   opts:   Settings structure, @see opts_t
 *   crc:    CRC8 covering `opts`
 */
struct eeprom_t
{
    uint8_t    magic;
    opts_t     opts;
    uint8_t    crc;
};

/** Function pointer definition for command handlers **/
typedef void (*cmd_handler_t)(const char *arg1, const char *arg2);

/**
 * @brief Command definition structure
 * 
 *   name:     String identifier representing the command
 *   handler:  Handler function for command
 */
struct command_t
{
    const char      *name;
    cmd_handler_t   handler;
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
 * @note     This function can take a considerable amount of time (waiting for
 *           the next LOW pulse), avoid calling it for unconnected fans.
 */
uint16_t get_rpm(uint8_t fan);

/**
 * @brief Determine current sensor temperature
 * 
 * @param    sensor  Sensor no.
 * @returns  Current temperature in degrees Celsius
 */
double get_temp(uint8_t sensor);

/**
 * @brief Set fan duty
 * 
 * @param  fan    Fan no.
 * @param  value  Duty value to apply, in percent (0-100)
 */
void set_duty(uint8_t fan, uint8_t value);

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
 * signal. Marks fans without the latter as unconnected. This is used to
 * save execution time consumed by unnecessary calls to `get_rpm()`.
 */
void fan_scan();

/**
 * @brief Print latest fan duty, rpm and temperature readings
 */
void print_status();

/**
 * @brief Handle serial input
 */
void handle_serial();

/**
 * @brief Handler for 'set' command
 * 
 * Sets fan duty using `set_duty()`.
 * 
 * @param[in]  s_fan   String containing fan no.
 * @param[in]  s_duty  String containing duty value
 */
void cmd_set(const char *s_fan, const char *s_duty);

/**
 * @brief Handler for 'status' command
 * 
 * Prints current status using `print_status()`. Accepts an optional interval
 * to be specified for periodic status output.
 * 
 * @param[in]  s_interval  Interval for periodic status outout, in seconds
 *                         (0 = disabled)
 */
void cmd_status(const char *s_interval, const char *);

/**
 * @brief Handler for 'save' command
 * 
 * @see opts_save()
 */
void cmd_save(const char *, const char *);

/**
 * @brief Handler for 'load' command
 * 
 * @see opts_load()
 */
void cmd_load(const char *, const char *);

/**
 * @brief Handler for 'map' command
 *
 * Sets sensor no. assigned to given fan no. This is used for temperature based
 * fan duty control. If no second argument given: show current mapping.
 *
 * @param[in]  s_fan   Fan no. to set/get mapping for
 * @param[in]  s_tmp   Temperature sensor no. to assign (optional)
 */
void cmd_map(const char *s_fan, const char *s_tmp);

/**
 * @brief Handler for 'curve' command
 * 
 * Determines fan characteristic by ramping duty values from 100% down to 0%
 * in `CURVE_STEP`% steps, taking `CURVE_SAMPLE_NUM` samples of the fan RPM.
 * Prints summary as CSV with duty value as rows and fan RPM as columns.
 */
void cmd_curve(const char *, const char *);

/**
 * @brief Handler for 'help' command
 * 
 * Prints all available commands.
 */
void cmd_help(const char *, const char *);

/**
 * @brief Handler for 'version' command
 * 
 * Prints firmware version and build timestamp.
 */
void cmd_version(const char *, const char *);

#endif

/* vim: set ts=4 sw=4 et */
