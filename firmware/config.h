/* Copyright (c) 2020 Alexander Koch
 *
 * This file is part of a project that is distributed under the terms of the MIT
 * License, see file 'LICENSE'.
 */

#ifndef _CONFIG_H
#define _CONFIG_H

#define NUM_FAN        4                      // No. of fans
#define NUM_TEMP       2                      // No. of sensors

#define SERIAL_BAUD    57600                  // Serial baud rate
#define SERIAL_TIMO    500                    // Serial RX timeout (ms)
#define SERIAL_BUFS    64                     // Serial buffer size (bytes)

#define PINS_PWM       { 9, 6, 5, 10 }        // Pins for fan PWM signal
#define PINS_RPM       { 8, 7, 4, 2 }         // Pins for fan RPM signal
#define PINS_TMP       { A1, A0 }             // Pins for sensors

#define TIMER13_TOP    320                    // Timer1/3 top value for 25 kHz PWM
#define TIMER4_TOP     240                    // Timer4 top value for 25 kHz PWM

#define UPDATE_INT     1000                   // Measurement interval (ms)

#define TMP_R          10000.0                // Sensor resistor (10 kOhm)

#define RPM_TIMEOUT    500000                 // RPM pulse detection timeout (us)
#define RPM_TMIN       3000                   // Minimum valid RPM pulse length (us)
#define RPM_SNUM       2                      // No. of samples for RPM measurement
#define RPM_RETRIES    2                      // No. of retries for RPM measurement

#define DEF_UNIT       DEG_C                  // Default temperature unit (C)
#define DEF_MODE       MODE_MANUAL            // Default operation mode
#define DEF_DUTY       50                     // Default fixed fan duty (%)
#define DEF_MAP        0                      // Default sensor mapping
#define DEF_LIN_TL     2000                   // Default linear lower temperature
#define DEF_LIN_TU     4000                   // Default linear upper temperature
#define DEF_LIN_DL     33                     // Default linear lower duty (%)
#define DEF_LIN_DU     80                     // Default linear upper duty (%)

#define SCAN_DUTY      50                     // Fan scan duty (%)
#define SCAN_SETTLE    2000                   // Fan scan settle delay (ms)
#define SCAN_TRIES     3                      // Fan scan tries per channel

#define EEPROM_MAGIC   0xFB                   // Settings record start byte
#define EEPROM_GOFFS   15                     // Offset of generation indicator
#define EEPROM_LEN     1024                   // 1 kB EEPROM on Leonardo

#define CURVE_STEP     10                     // Curve duty step size (%)
#define CURVE_SDELAY   5000                   // Curve settle delay (ms)
#define CURVE_SMPNUM   3                      // Curve sample num per duty
#define CURVE_SMPDEL   50                     // Curve delay between samples (ms)

#ifndef VERSION
#define VERSION        "unknown"              // Fallback version string
#endif
#define BUILD		   __DATE__ " " __TIME__  // Build timestamp

#define MIN(X, Y)             (X <= Y ? X : Y)

#define FOREACH_U8(V, LIMIT)  for (uint8_t V=0; V<LIMIT; V++)
#define FOREACH_FAN(V)        FOREACH_U8(V, NUM_FAN)
#define FOREACH_TEMP(V)       FOREACH_U8(V, NUM_TEMP)

#define EEPROM_GEN_NUM        ((EEPROM_LEN - EEPROM_GOFFS-1) / sizeof(eeprom_t))
#define EEPROM_OPT_OFFS(GEN)  (EEPROM_GOFFS + 1 + GEN * sizeof(eeprom_t))

#endif

/* vim: set ts=4 sw=4 et */
