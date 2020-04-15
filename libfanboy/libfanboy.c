/* Copyright (c) 2020 Alexander Koch
 *
 * This file is part of a project that is distributed under the terms of the MIT
 * License, see file 'LICENSE'.
 */

#include "libfanboy.h"
#include "serial.h"

#ifndef WIN32
#include <unistd.h>
#else
#include <Windows.h>
#endif

static const int RETRIES       = 2;       // retry count for regular receive
static const int CURVE_RETRIES = 200;     // retry count for curve sampling

const char *error = NULL;

static bool query(cmd_t command, const void *payload, size_t payload_len,
                  void *result, size_t result_len)
{
    error = NULL;

    // send header
    header_t header = { .sof = SOF, .cmd = command };
    if (!serial_send(&header, sizeof(header)))
        return false;

    // send payload (if any)
    if (payload)
        if (!serial_send(payload, payload_len))
            return false;

    // scan for reply header
    header.sof = 0;
    while (header.sof != SOF &&
           serial_receive(&header.sof, sizeof(header.sof), RETRIES)) {}
    if (header.sof != SOF)
        return false;

    if (!serial_receive(&header.cmd, sizeof(header.cmd), RETRIES))
        return false;
    if (header.cmd != command) {
        error = "protocol error";
        return false;
    }

    // receive reply payload
    if (!serial_receive(result, result_len, RETRIES))
        return false;

    return true;
}

static bool simple_query(cmd_t command, const void *payload, size_t len)
{
    msg_result_t result;

    if (!query(command, payload, len, &result, sizeof(result)))
        return false;
    if (result.retult != RESULT_OK) {
        error = "device reported error";
        return false;
    }

    return true;
}

bool fb_init(const char *dev)
{
    return serial_open(dev);
}

void fb_exit()
{
    serial_close();
}

const char *fb_error()
{
    return error;
}

bool fb_status(fb_status_t *result)
{
    return query(CMD_STATUS, NULL, 0, result, sizeof(fb_status_t));
}

bool fb_version(fb_version_t *result)
{
    return query(CMD_VERSION, NULL, 0, result, sizeof(fb_version_t));
}

bool fb_config(fb_config_t *result)
{
    return query(CMD_CONFIG, NULL, 0, result, sizeof(fb_config_t));
}

bool fb_set_mode(uint8_t fan, fan_mode_t mode)
{
    msg_fan_mode_t msg = { .fan = fan, .mode = mode };

    return simple_query(CMD_FAN_MODE, &msg, sizeof(msg));
}

bool fb_set_duty(uint8_t fan, uint8_t duty)
{
    msg_fan_duty_t msg = { .fan = fan, .duty = duty };

    return simple_query(CMD_FAN_DUTY, &msg, sizeof(msg));
}

bool fb_set_map(uint8_t fan, uint8_t sensor)
{
    msg_fan_map_t msg = { .fan = fan, .sensor = sensor };

    return simple_query(CMD_FAN_MAP, &msg, sizeof(msg));
}

bool fb_set_linear(uint8_t fan, fb_linear_t *param)
{
    msg_fan_linear_t msg = { .fan = fan, .param = *param };

    return simple_query(CMD_LINEAR, &msg, sizeof(msg));
}

bool fb_fan_curve(fb_curve_t *result)
{
    error = NULL;

    // send header
    header_t header = { .sof = SOF, .cmd = CMD_FAN_CURVE };
    if (!serial_send(&header, sizeof(header)))
        return false;

    // wait for fan curve to be sampled
    uint16_t delay_ms = (100/CURVE_STEP + 1) * (CURVE_SDELAY +
                        (CURVE_SMPNUM - 1) * CURVE_SMPDEL);
#ifndef WIN32
    usleep(delay_ms * 1000);
#else
    Sleep(delay_ms);
#endif

    // scan for reply header, extended timeout due to unknown sampling delay
    header.sof = 0;
    while (header.sof != SOF &&
           serial_receive(&header.sof, sizeof(header.sof), CURVE_RETRIES)) {}
    if (header.sof != SOF)
        return false;

    if (!serial_receive(&header.cmd, sizeof(header.cmd), RETRIES))
        return false;
    if (header.cmd != CMD_FAN_CURVE) {
        error = "protocol error";
        return false;
    }

    // receive reply payload
    if (!serial_receive(result, sizeof(fb_curve_t), RETRIES))
        return false;

    return true;
}

bool fb_save()
{
    return simple_query(CMD_SAVE, NULL, 0);
}

bool fb_load()
{
    return simple_query(CMD_LOAD, NULL, 0);
}

void fb_reset()
{
    simple_query(CMD_RESET, NULL, 0);
}
