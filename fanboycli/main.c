/* Copyright (c) 2020 Alexander Koch
 *
 * This file is part of a project that is distributed under the terms of
 * the MIT License, see file 'LICENSE'.
 */

#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include "libfanboy.h"

#if defined linux
const char *DEF_DEVICE = "/dev/ttyACM0";
#elif defined WIN32
const char *DEF_DEVICE = "COM1";
#endif
const char *PARAM_DELIMITER = ":";


static inline const char *peek_device(int argc, char *argv[])
{
    for (int i=argc-2; i>=0; i--)
        if (strcmp("-D", argv[i]) == 0)
            return argv[i+1];

    return DEF_DEVICE;
}

static inline void print_help()
{
    puts("Usage: fanboycli [ARGUMENT(S)]\n");

    puts(  "Device Status:");
    puts(  "  -s       Show current fan / sensor readings");
    puts(  "  -c       Show current configuration\n");

    puts(  "Fan Control:");
    printf("  -f FAN   Select fan FAN to control (1-%d)\n", NUM_FAN);
    puts(  "  -d DUTY  Set selected fan to fixed duty (0-100)");
    puts(  "  -m MODE  Set fan control mode ('manual' or 'linear')");
    printf("  -M TEMP  Set mapped sensor no. (1-%d)\n", NUM_TEMP);
    puts(  "  -l PARA  Set linear control parameters (format see below)\n");

    puts(  "Device Management:");
    puts(  "  -L       Load configuration from EEPROM");
    puts(  "  -S       Save current configuration to EEPROM");
    puts(  "  -C       Generate fan curve as CSV samples");
    puts(  "  -R       Reset FanBoy (re-initializes USB as well)\n");

    puts(  "Misc:");
    printf("  -D DEV   Set serial interface (default: '%s')\n", DEF_DEVICE);
    puts(  "  -V       Show FanBoy firmware version and build timestamp");
    puts(  "  -h       Show usage help text\n");

    puts(  "Linear parameter format: 'LOW_DUTY:LOW_TEMP:HIGH_DUTY:HIGH_TEMP'");
    puts(  "  LOW_TEMP   Low temperature");
    puts(  "  HIGH_TEMP  High temperature");
    puts(  "  LOW_DUTY   Fan duty applied when temperature <= TEMP_LOW");
    puts(  "  HIGH_DUTY  Fan duty applied when temperature >= TEMP_HIGH\n");

    puts(  "Fan duty follows a linear curve between LOW_DUTY and HIGH_DUTY.\n");

    puts(  "This version of fanboycli was built " __DATE__ " " __TIME__ "\n");
}

static inline bool get_params(char *string, linear_t *params)
{
    const char *ptr = strtok(string, PARAM_DELIMITER);
    if (ptr == NULL)
        return false;
    params->min_duty = atoi(ptr);

    ptr = strtok(NULL, PARAM_DELIMITER);
    if (ptr == NULL)
        return false;
    params->min_temp = atof(ptr) * 100.0;

    ptr = strtok(NULL, PARAM_DELIMITER);
    if (ptr == NULL)
        return false;
    params->max_duty = atoi(ptr);

    ptr = strtok(NULL, PARAM_DELIMITER);
    if (ptr == NULL)
        return false;
    params->max_temp = atof(ptr) * 100.0;

    if (params->min_duty > 100 || params->max_duty > 100 ||
            params->min_temp > 10000 || params->max_temp > 10000)
        return false;

    return true;
}

static inline void print_config(const fb_config_t *config)
{
    puts("FanBoy config:");

    char unit = config->temp_unit == DEG_C ? 'C' : 'F';
    printf("  Temperature unit: %c\n", unit);

    for (int i=0; i<NUM_FAN; i++) {
        printf("  Fan %d:\n", i+1);
        printf("    Mode:         %s\n", config->fan[i].mode == MODE_MANUAL ?
               "manual" : "linear");
        printf("    Manual duty:  %02d%%\n", config->fan[i].duty);
        printf("    Sensor:       %d\n", config->fan[i].sensor+1);
        puts("    Linear params:");
        printf("      Low:   %02d%% @ %.2f %c\n", config->fan[i].param.min_duty,
               (double)config->fan[i].param.min_temp/100.0, unit);
        printf("      High:  %02d%% @ %.2f %c\n", config->fan[i].param.max_duty,
               (double)config->fan[i].param.max_temp/100.0, unit);
    }
}

static inline void print_status(const fb_status_t *status)
{
    puts("FanBoy status:");

    for (int i=0; i<NUM_FAN; i++) {
        printf("  Fan %d: ", i+1);
        if (status->fan[i].rpm != NCONN)
            printf("%d%% @ %d rpm\n", status->fan[i].duty, status->fan[i].rpm);
        else
            puts("disconnected");
    }
    for (int i=0; i<NUM_TEMP; i++) {
        printf("  Temp %d: ", i+1);
        if (status->temp[i] != NCONN)
            printf("%.2f\n", (double)status->temp[i] / 100.0);
        else
            puts("disconnected");
    }
}

int main(int argc, char *argv[])
{
    const char *device = peek_device(argc, argv);

    if (!fb_init(device)) {
        fprintf(stderr, "Failed to connect to '%s': %s\n", device, fb_error());
        return 1;
    }

    bool ret = true;
    uint8_t fan = 255;
    char c;
    while ((c = getopt(argc, argv, "D:sf:d:m:M:cl:CSLRhV")) != -1) {
        switch (c) {
            case 'h':
            {
                print_help();
                break;
            }
            case 'c':
            {
                fb_config_t config;
                if (fb_config(&config)) {
                    print_config(&config);
                } else {
                    fprintf(stderr, "Failed to read config: %s\n", fb_error());
                    ret = false;
                }
                break;
            }
            case 'D':
            {
                // already handled, skip
                break;
            }
            case 's':
            {
                fb_status_t status;
                if (fb_status(&status)) {
                    print_status(&status);
                } else {
                    fprintf(stderr, "Failed to read status: %s\n", fb_error());
                    ret = false;
                }
                break;
            }
            case 'f':
            {
                fan = atoi(optarg) - 1;
                if (fan >= NUM_FAN) {
                    fprintf(stderr, "Error: invalid fan no. '%s'\n", optarg);
                    ret = false;
                    goto cleanup;
                }
                break;
            }
            case 'd':
            {
                uint8_t duty = atoi(optarg);
                if (duty > 100) {
                    fprintf(stderr, "Error: invalid fan duty '%d%%'\n", duty);
                    ret = false;
                    goto cleanup;
                }
                if (fan >= NUM_FAN) {
                    fprintf(stderr, "Error: invalid fan no. '%d'\n", fan);
                    ret = false;
                    goto cleanup;
                }
                if (!fb_set_duty(fan, duty)) {
                    fprintf(stderr, "Failed to set fan duty: %s\n", fb_error());
                    ret = false;
                }
                break;
            }
            case 'm':
            {
                fan_mode_t mode;
                if (strcmp("manual", optarg) == 0)
                    mode = MODE_MANUAL;
                else if (strcmp("linear", optarg) == 0)
                    mode = MODE_LINEAR;
                else {
                    fprintf(stderr, "Error: invalid fan mode '%s'\n", optarg);
                    ret = false;
                    goto cleanup;
                }
                if (fan >= NUM_FAN) {
                    fprintf(stderr, "Error: invalid fan no. '%d'\n", fan);
                    ret = false;
                    goto cleanup;
                }
                if (!fb_set_mode(fan, mode)) {
                    fprintf(stderr, "Failed to set fan mode: %s\n", fb_error());
                    ret = false;
                }
                break;
            }
            case 'M':
            {
                uint8_t sensor = atoi(optarg) - 1;
                if (sensor >= NUM_TEMP) {
                    fprintf(stderr, "Error: invalid sensor no. '%s'\n", optarg);
                    ret = false;
                    goto cleanup;
                }
                if (fan >= NUM_FAN) {
                    fprintf(stderr, "Error: invalid fan no. '%d'\n", fan);
                    ret = false;
                    goto cleanup;
                }
                if (!fb_set_map(fan, sensor)) {
                    fprintf(stderr, "Failed to set mapping: %s\n", fb_error());
                    ret = false;
                }
                break;
            }
            case 'l':
            {
                fb_linear_t params;
                if (!get_params(optarg, &params)) {
                    fprintf(stderr, "Error: invalid parameter string\n");
                    ret = false;
                    goto cleanup;
                }
                if (fan >= NUM_FAN) {
                    fprintf(stderr, "Error: invalid fan no. '%d'\n", fan);
                    ret = false;
                    goto cleanup;
                }
                if (!fb_set_linear(fan, &params)) {
                    fprintf(stderr, "Failed to set linear parameters: %s\n",
                            fb_error());
                    ret = false;
                }
                break;
            }
            case 'C':
            {
                fb_curve_t curve;
                printf("Generating fan curve (this may take some time)...\n");
                if (!fb_fan_curve(&curve)) {
                    fprintf(stderr, "Failed to generate fan curve: %s\n",
                            fb_error());
                    ret = false;
                } else {
                    for (size_t i=0; i<sizeof(curve.points)/sizeof(curve_point_t); i++) {
                        curve_point_t p = curve.points[i];
                        printf("%d%%", p.duty);
                        for (int n=0; n<NUM_FAN; n++)
                            printf(",%u", p.rpm[n]);
                        putchar('\n');
                    }
                }
                break;
            }
            case 'S':
            {
                if (!fb_save()) {
                    fprintf(stderr, "Failed to save configuration: %s\n",
                            fb_error());
                    ret = false;
                }
                break;
            }
            case 'L':
            {
                if (!fb_load()) {
                    fprintf(stderr, "Failed to load configuration: %s\n",
                            fb_error());
                    ret = false;
                }
                break;
            }
            case 'R':
            {
                puts("Triggering FanBoy reset");
                fb_reset();
                goto cleanup;
            }
            case 'V':
            {
                fb_version_t vers;
                if (fb_version(&vers)) {
                    printf("FanBoy firmware:\n");
                    printf("  Version: %s\n", vers.version);
                    printf("  Built:   %s\n", vers.build);
                } else {
                    fprintf(stderr, "failed to get firmware info\n");
                    ret = false;
                }
                break;
            }
            case '?':
            {
                fprintf(stderr, "option -%c requires an argument\n", optopt);
                ret = false;
                goto cleanup;
                break;
            }
            default:
            {
                fprintf(stderr, "invalid argument(s). Try -h for help.");
                ret = false;
                goto cleanup;
            }
        }
    }

cleanup:
	fb_exit();
	
    return !ret;
}
