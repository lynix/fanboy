/* Copyright (c) 2020 Alexander Koch
 *
 * This file is part of a project that is distributed under the terms of the MIT
 * License, see file 'LICENSE'.
 */

#include <EEPROM.h>
#include <avr/wdt.h>

#include "config.h"
#include "serial.h"
#include "decl.h"

static const uint8_t pins_pwm[NUM_FAN] = PINS_PWM;
static const uint8_t pins_rpm[NUM_FAN] = PINS_RPM;
static const uint8_t pins_tmp[NUM_TEMP] = PINS_TMP;

static config_t    opts;
static status_t    status;
static version_t   version;
static char        buffer[SERIAL_BUFS];
static uint32_t    pid_last_time = 0;
static double      pid_last_error[NUM_FAN];
static double      pid_error_i[NUM_FAN];


void setup()
{
    // I/O pins
    FOREACH_FAN(i) {
        pinMode(pins_pwm[i], OUTPUT);
        pinMode(pins_rpm[i], INPUT);
    }
    FOREACH_TEMP(i)
        pinMode(pins_tmp[i], INPUT);

    // setup timer1 for 25 kHz PWM
    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1C = 0;
    TCNT1  = 0;
    TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(COM1C1) | _BV(WGM11);
    TCCR1B = _BV(WGM13) | _BV(CS10);
    TCCR1C = _BV(WGM13) | _BV(CS10);
    ICR1   = TIMER13_TOP;

    // setup timer3 for 25 kHz PWM
    TCCR3A = 0;
    TCNT3  = 0;
    TCCR3A = _BV(COM1A1) | _BV(WGM11);
    TCCR3B = _BV(WGM13) | _BV(CS10);
    ICR3   = TIMER13_TOP;

    // setup timer4 for ~25 kHz PWM (close enough)
    TCCR4A = 0;
    TCCR4B = 4;
    TCCR4C = 0;
    TCCR4D = 0;
    PLLFRQ = (PLLFRQ & 0xCF) | 0x30;
    OCR4C  = TIMER4_TOP;
    DDRD   |= 1<<7;
    TCCR4C |= 0x09;

    // prepare firmware version record
    memset(version.version, 0, STRL);
    strncpy(version.version, VERSION, STRL-1);
    memset(version.build, 0, STRL);
    strncpy(version.build, BUILD, STRL-1);

    // scan connected fans
    fan_scan();

    // default configuration
    FOREACH_FAN(i) {
        opts.temp_unit = DEF_UNIT;
        opts.fan[i].mode = DEF_MODE;
        opts.fan[i].duty = DEF_DUTY;
        opts.fan[i].sensor = DEF_MAP;
        opts.fan[i].param_linear.min_temp = DEF_LIN_TL;
        opts.fan[i].param_linear.min_duty = DEF_LIN_DL;
        opts.fan[i].param_linear.max_temp = DEF_LIN_TU;
        opts.fan[i].param_linear.max_duty = DEF_LIN_DU;
        opts.fan[i].param_pid.min_duty = DEF_PID_MIN;
        opts.fan[i].param_pid.max_duty = DEF_PID_MAX;
        opts.fan[i].param_pid.target_temp = DEF_PID_TGT;
        pid_last_error[i] = 0;
        pid_error_i[i] = 0;
    }

    // load configuration from EEPROM
    opts_load();

    // serial
    Serial.begin(SERIAL_BAUD);
    Serial.setTimeout(SERIAL_TIMO);
}

void loop()
{
    uint32_t now = millis();

    static uint32_t measure_next = 0;
    if (now > measure_next) {
        FOREACH_TEMP(i)
            status.temp[i] = get_temp(i);
        FOREACH_FAN(i) {
            if (status.fan[i].rpm != NCONN) {
                status.fan[i].rpm = get_rpm(i);
                if (opts.fan[i].mode == MODE_LINEAR)
                    set_duty_linear(i);
                else if (opts.fan[i].mode == MODE_PID)
                    set_duty_pid(i, now - pid_last_time);
            }
        }
        measure_next = now + UPDATE_INT;
        pid_last_time = now;
    }

    if (Serial.available())
        handle_serial();
}

uint8_t crc8(const uint8_t *data, uint16_t len)
{
    uint8_t crc = 0x00;

    while (len--) {
        uint8_t val = *data++;
        for (uint8_t i=0; i<8; i++) {
            uint8_t sum = (crc ^ val) & 0x01;
            crc >>= 1;
            if (sum)
                crc ^= 0x8C;
            val >>= 1;
        }
    }

    return crc;
}

void opts_save()
{
    uint8_t gen = (EEPROM[EEPROM_GOFFS] + 1) % EEPROM_GEN_NUM;
    EEPROM[EEPROM_GOFFS] = gen;

    eeprom_t e;
    e.magic = EEPROM_MAGIC;
    e.opts = opts;
    e.crc = crc8((const uint8_t *)&e.opts, sizeof(e.opts));

    EEPROM.put(EEPROM_OPT_OFFS(gen), e);
}

bool opts_load()
{
    uint8_t gen = EEPROM[EEPROM_GOFFS] % EEPROM_GEN_NUM;

    eeprom_t e;
    EEPROM.get(EEPROM_OPT_OFFS(gen), e);

    if (e.magic != EEPROM_MAGIC)
        return false;
    uint8_t crc = crc8((const uint8_t *)&e.opts, sizeof(e.opts));
    if (crc != e.crc)
        return false;

    opts = e.opts;
    FOREACH_FAN(i)
        if (opts.fan[i].mode == MODE_MANUAL)
            set_duty(i, opts.fan[i].duty);

    return true;
}

uint16_t get_rpm(uint8_t fan)
{
    uint16_t rpm = 0;

    FOREACH_U8(i, RPM_SNUM) {
        uint32_t time;
        FOREACH_U8(n, RPM_RETRIES) {
            time = pulseIn(pins_rpm[fan], LOW, RPM_TIMEOUT);
            if (time >= RPM_TMIN || status.fan[fan].duty == 0)
                break;
        }
        if (time >= RPM_TMIN)
            rpm += 15000000UL / time;
    }

    return rpm / RPM_SNUM;
}

uint16_t get_temp(uint8_t sensor)
{
    // Steinhart–Hart coefficients
    static const float a = 1.009249522e-03;
    static const float b = 2.378405444e-04;
    static const float c = 2.019202697e-07;

    int v0 = analogRead(pins_tmp[sensor]);
    if (!v0)
        return NCONN;

    float r2 = TMP_R * (1023.0 / (float)v0 - 1.0);
    float log_r2 = log(r2);
    float t = 1.0 / (a + b*log_r2 + c*log_r2*log_r2*log_r2);

    double temp = t - 273.15;
    if (opts.temp_unit == DEG_F)
        temp = t * 1.8 + 32.0;

    return (uint16_t)(temp * 100.0);
}

void set_duty(uint8_t fan, uint8_t value)
{
    if (value > 100)
        value = 100;
    status.fan[fan].duty = value;

    int duty = (int)value * (fan == 1 ? TIMER4_TOP : TIMER13_TOP);
    duty /= 100;

    switch (fan) {
        case 0: OCR1A = duty; break;
        case 1: OCR4D = duty; break;
        case 2: OCR3A = duty; break;
        case 3: OCR1B = duty; break;
    }
}

void set_duty_linear(uint8_t fan)
{
    double temp = status.temp[opts.fan[fan].sensor];
    double t_min = (double)opts.fan[fan].param_linear.min_temp;
    double t_max = (double)opts.fan[fan].param_linear.max_temp;

    uint8_t duty;
    if (temp <= t_min)
        duty = opts.fan[fan].param_linear.min_duty;
    else if (temp >= t_max)
        duty = opts.fan[fan].param_linear.max_duty;
    else
        duty = opts.fan[fan].param_linear.min_duty + (temp - t_min) *
            (opts.fan[fan].param_linear.max_duty -
            opts.fan[fan].param_linear.min_duty) / (t_max - t_min);

    if (status.fan[fan].duty != duty)
        set_duty(fan, duty);
}

void set_duty_pid(uint8_t fan, uint32_t time_diff)
{
    double error = opts.fan[fan].param_pid.target_temp -
        status.temp[opts.fan[fan].sensor];
    double error_d = (error - pid_last_error[fan]) / (double)time_diff;
    pid_error_i[fan] += error * (double)time_diff;

    double fduty = PID_P * error + PID_I * pid_error_i[fan] + PID_D * error_d;
    if (fduty < opts.fan[fan].param_pid.min_duty)
        fduty = opts.fan[fan].param_pid.min_duty;
    else if (fduty > opts.fan[fan].param_pid.max_duty)
        fduty = opts.fan[fan].param_pid.max_duty;

    uint8_t duty = (uint8_t)fduty;
    if (status.fan[fan].duty != duty)
        set_duty(fan, duty);

    pid_last_error[fan] = error;
}

void fan_scan()
{
    FOREACH_FAN(i)
        set_duty(i, SCAN_DUTY);

    delay(SCAN_SETTLE);

    FOREACH_FAN(i) {
        FOREACH_U8(n, SCAN_TRIES) {
            status.fan[i].rpm = get_rpm(i);
            if (status.fan[i].rpm > 0)
                break;
        }
        if (status.fan[i].rpm == 0)
            status.fan[i].rpm = NCONN;
        set_duty(i, DEF_DUTY);
    }
}

void reset()
{
    wdt_enable(50);
    while (true) {};
}

void handle_serial()
{
    bool sof = false;
    while (Serial.available()) {
        int read = Serial.read();
        if (read == -1)
            continue;
        if ((uint8_t)read == SOF) {
            sof = true;
            break;
        }
    }
    if (!sof)
        return;

    int command = Serial.read();
    if (command == -1)
        return;

    size_t reply_len = 0;
    char *reply = buffer;
    switch ((cmd_t)command) {
        case CMD_VERSION:
            reply_len = sizeof(version_t);
            reply = (char *)&version;
            break;
        case CMD_STATUS:
            reply_len = sizeof(status_t);
            reply = (char *)&status;
            break;
        case CMD_CONFIG:
            reply_len = sizeof(config_t);
            reply = (char *)&opts;
            break;
        case CMD_FAN_MODE:
        {
            reply_len = 1;
            buffer[0] = RESULT_ERR;
            if (Serial.readBytes(buffer, sizeof(msg_fan_mode_t)) ==
                    sizeof(msg_fan_mode_t)) {
                msg_fan_mode_t *msg = (msg_fan_mode_t *)buffer;
                if (msg->fan < NUM_FAN && (msg->mode == MODE_MANUAL ||
                        msg->mode == MODE_LINEAR || msg->mode == MODE_PID)) {
                    opts.fan[msg->fan].mode = msg->mode;
                    if (msg->mode == MODE_MANUAL)
                        set_duty(msg->fan, opts.fan[msg->fan].duty);
                    else if (msg->mode == MODE_LINEAR)
                        set_duty_linear(msg->fan);
                    buffer[0] = RESULT_OK;
                }
            }
            break;
        }
        case CMD_FAN_DUTY:
        {
            reply_len = 1;
            buffer[0] = RESULT_ERR;
            if (Serial.readBytes(buffer, sizeof(msg_fan_duty_t)) ==
                    sizeof(msg_fan_duty_t)) {
                msg_fan_duty_t *msg = (msg_fan_duty_t *)buffer;
                if (msg->fan < NUM_FAN && msg->duty <= 100) {
                    opts.fan[msg->fan].mode = MODE_MANUAL;
                    opts.fan[msg->fan].duty = msg->duty;
                    set_duty(msg->fan, msg->duty);
                    buffer[0] = RESULT_OK;
                }
            }
            break;
        }
        case CMD_FAN_MAP:
        {
            reply_len = 1;
            buffer[0] = RESULT_ERR;
            if (Serial.readBytes(buffer, sizeof(msg_fan_map_t)) ==
                    sizeof(msg_fan_map_t)) {
                msg_fan_map_t *msg = (msg_fan_map_t *)buffer;
                if (msg->fan < NUM_FAN && msg->sensor < NUM_TEMP) {
                    buffer[0] = RESULT_OK;
                    opts.fan[msg->fan].sensor = msg->sensor;
                }
            }
            break;
        }
        case CMD_LINEAR:
        {
            reply_len = 1;
            buffer[0] = RESULT_ERR;
            if (Serial.readBytes(buffer, sizeof(msg_fan_linear_t)) ==
                    sizeof(msg_fan_linear_t)) {
                msg_fan_linear_t *msg = (msg_fan_linear_t *)buffer;
                if (msg->fan < NUM_FAN && msg->param.min_duty <= 100 &&
                        msg->param.max_duty <= 100) {
                    opts.fan[msg->fan].param_linear = msg->param;
                    buffer[0] = RESULT_OK;
                }
            }
            break;
        }
        case CMD_PID:
        {
            reply_len = 1;
            buffer[0] = RESULT_ERR;
            if (Serial.readBytes(buffer, sizeof(msg_fan_pid_t)) ==
                    sizeof(msg_fan_pid_t)) {
                msg_fan_pid_t *msg = (msg_fan_pid_t *)buffer;
                if (msg->fan < NUM_FAN && msg->param.min_duty <= 100 &&
                        msg->param.max_duty <= 100) {
                    opts.fan[msg->fan].param_pid = msg->param;
                    buffer[0] = RESULT_OK;
                }
            }
            break;
        }
        case CMD_FAN_CURVE:
            reply_len = sizeof(msg_fan_curve_t);
            fan_curve();
            break;
        case CMD_SAVE:
            reply_len = 1;
            buffer[0] = RESULT_OK;
            opts_save();
            break;
        case CMD_LOAD:
            reply_len = 1;
            buffer[0] = opts_load() ? RESULT_OK : RESULT_ERR;
            break;
        case CMD_RESET:
            reset();
            break;
        default:
            Serial.write(SOF);
            Serial.write(CMD_INVALID);
            return;
    }

    Serial.write(SOF);
    Serial.write((uint8_t)command);
    Serial.write(reply, reply_len);
}

void fan_curve()
{
    msg_fan_curve_t *data = (msg_fan_curve_t *)buffer;
    uint16_t rpm[NUM_FAN][CURVE_SMPNUM];

    for (int i=0; i<=100/CURVE_STEP; i++) {
        uint8_t duty = 100 - i * CURVE_STEP;
        FOREACH_FAN(f)
            set_duty(f, duty);
        data->points[i].duty = duty;

        delay(CURVE_SDELAY);

        FOREACH_U8(n, CURVE_SMPNUM) {
            FOREACH_FAN(f)
                rpm[f][n] = get_rpm(f);
            delay(CURVE_SMPDEL);
        }

        FOREACH_FAN(f) {
            for (int n=1; n<CURVE_SMPNUM; n++)
                rpm[f][0] += rpm[f][n];
            data->points[i].rpm[f] = rpm[f][0] / CURVE_SMPNUM;
        }
    }

    // restore manual duty
    FOREACH_FAN(i)
        if (opts.fan[i].mode == MODE_MANUAL)
            set_duty(i, opts.fan[i].duty);
}

/* vim: set ts=4 sw=4 et */
