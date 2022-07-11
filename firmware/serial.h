/* Copyright (c) 2020 Alexander Koch
 *
 * This file is part of a project that is distributed under the terms of the MIT
 * License, see file 'LICENSE'.
 */

#ifndef _SERIAL_H
#define _SERIAL_H

#include <stdint.h>

#include "config.h"


#define SOF    0x42    // Start-of-Frame delimiter byte value
#define NCONN  0xffff  // Disconnected RPM/temp value
#define STRL   32      // Length for fixed strings

#pragma pack(push, 1)

/**
 * @brief Command byte definitions
 */
typedef enum {
    CMD_VERSION    = 0x00,  //< get firmware version and build timestamp
    CMD_STATUS     = 0x01,  //< get current fan duty/rpm and temps
    CMD_CONFIG     = 0x02,  //< get configuration
    CMD_FAN_MODE   = 0x03,  //< set fan mode
    CMD_FAN_DUTY   = 0x04,  //< set fan duty (implies `MODE_MANUAL`)
    CMD_FAN_MAP    = 0x05,  //< set fan<->sensor mapping
    CMD_FAN_CURVE  = 0x06,  //< generate fan curves
    CMD_LINEAR     = 0x07,  //< set linear fan control parameters
    CMD_SAVE       = 0x08,  //< save settings to EEPROM
    CMD_LOAD       = 0x09,  //< load settings from EEPROM
    CMD_PID        = 0x0a,  //< set PID fan control parameters
    CMD_INVALID    = 0xfe,  //< invalid command
    CMD_RESET      = 0xff   //< reset device
} cmd_t;

/**
 * @brief Generic success/failure result
 */
typedef enum {
    RESULT_OK  = 0x00,   //< Success
    RESULT_ERR = 0xff    //< Failure
} result_t;

/**
 * @brief Fan operation mode
 */
typedef enum {
    MODE_MANUAL  = 0x00,    //< manual duty
    MODE_LINEAR  = 0x01,    //< linear curve between two points
    MODE_PID     = 0x02
} fan_mode_t;

/**
 * @brief Temperature unit
 */
typedef enum {
    DEG_C = 0x00,           //< degrees Celsius
    DEG_F = 0x01            //< degrees Fahrenheit
} temp_unit_t;

/**
 * @brief Serial message header
 */
typedef struct {
    uint8_t  sof;           //< start-of-frame delimiter
    uint8_t  cmd;           //< command byte (@see cmd_t)
} header_t;

/**
 * @brief Linear fan control parameters dataset
 */
typedef struct {
    uint16_t min_temp;      //< low temp
    uint8_t  min_duty;      //< low duty
    uint16_t max_temp;      //< high temp
    uint8_t  max_duty;      //< high duty
} linear_t;

/**
 * @brief PID fan control parameters dataset
 */
typedef struct {
    uint16_t target_temp;   //< target temperature
    uint8_t  min_duty;      //< min fan duty (cutoff)
    uint8_t  max_duty;      //< max fan duty (cutoff)
} pidc_t;

/**
 * @brief Fan status dataset
 */
typedef struct {
    uint8_t  duty;          //< current duty
    uint16_t rpm;           //< current RPM
} fan_status_t;

/**
 * @brief Fan and temperature status dataset
 */
typedef struct {
    fan_status_t  fan[NUM_FAN];     //< fan status
    uint16_t      temp[NUM_TEMP];   //< temperatures (*100 deg)
} status_t;

/**
 * @brief Fan configuration dataset
 */
typedef struct {
    uint8_t   mode;          //< fan mode
    uint8_t   duty;          //< fan duty (for manual mode)
    uint8_t   sensor;        //< fan<->sensor mapping (for linear mode)
    linear_t  param_linear;  //< linear control parameters
    pidc_t    param_pid;     //< PID control parameters
} fan_config_t;

/**
 * @brief Configuration dataset
 */
typedef struct {
    uint8_t       temp_unit;     //< temperature unit (@see temp_unit_t)
    fan_config_t  fan[NUM_FAN];  //< fan configurations
} config_t;

/**
 * @brief Fan curve sample point
 */
typedef struct {
    uint8_t   duty;
    uint16_t  rpm[NUM_FAN];
} curve_point_t;

/**
 * @brief Firmware version dataset
 */
typedef struct {
    char  version[STRL];   //< firmware version
    char  build[STRL];     //< build timestamp
} version_t;

/**
 * @brief Payload for `CMD_VERSION` message (reply)
 */
typedef version_t msg_version_t;

/**
 * @brief Payload for `CMD_CONFIG` message (reply)
 */
typedef config_t msg_config_t;

/**
 * @brief Payload for `CMD_STATUS` message (reply)
 */
typedef status_t msg_status_t;

/**
 * @brief Payload for generic gesponse message indicating success or failure.
 */
typedef struct {
    uint8_t retult;
} msg_result_t;

/**
 * @brief Payload for `CMD_FAN_DUTY` message, setting fixed duty
 */
typedef struct {
    uint8_t  fan;           //< fan no. (counted from zero)
    uint8_t  duty;          //< fan duty in % (0-100)
} msg_fan_duty_t;

/**
 * @brief Payload for `CMD_FAN_MODE` message, setting fan mode
 */
typedef struct {
    uint8_t  fan;           //< fan no. (counted from zero)
    uint8_t  mode;          //< fan mode (@see fan_mode_t)
} msg_fan_mode_t;

/**
 * @brief Payload for `CMD_FAN_MAP` message, setting fan<->sensor
 *        mapping
 */
typedef struct {
    uint8_t  fan;           //< fan no. (counted from zero)
    uint8_t  sensor;        //< sensor no. (counted from zero)
} msg_fan_map_t;

/**
 * @brief Payload for `CMD_FAN_CURVE` message (reply) containing curve points
 */
typedef struct {
    curve_point_t  points[100/CURVE_STEP+1];
} msg_fan_curve_t;

/**
 * @brief Payload for `CMD_LINEAR` message, setting linear fan control
 *        parameters
 */
typedef struct {
    uint8_t   fan;          //< fan no. (counted from zero)
    linear_t  param;        //< linear control parameters
} msg_fan_linear_t;

/**
 * @brief Payload for `CMD_PID` message, setting PID fan control parameters
 */
typedef struct {
    uint8_t   fan;          //< fan no. (counted from zero)
    pidc_t    param;        //< PID control parameters
} msg_fan_pid_t;


#pragma pack(pop)

#endif


/* vim: set ts=4 sw=4 et */
