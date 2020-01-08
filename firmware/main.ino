/* Copyright (c) 2019 Alexander Koch
 *
 * This file is part of a project that is distributed under the terms of
 * the MIT License, see file 'LICENSE'.
 */

#include <EEPROM.h>

#include "defs.h"
#include "decl.h"


static const uint8_t pins_pwm[NUM_FAN] = PINS_PWM;
static const uint8_t pins_rpm[NUM_FAN] = PINS_RPM;
static const uint8_t pins_tmp[NUM_TMP] = PINS_TMP;

static const command_t commands[] = {
    { "set",        cmd_set },
    { "status",     cmd_status },
    { "curve",      cmd_curve },
    { "save",       cmd_save },
    { "load",       cmd_load },
    { "map",        cmd_map },
    { "help",       cmd_help },
    { "version",    cmd_version }
};

static opts_t      opts;
static bool        fan_connected[NUM_FAN];
static uint8_t     fan_duty[NUM_FAN];
static uint16_t    fan_rpm[NUM_FAN];
static double      temp[NUM_TMP];
static char        buffer[SERIAL_BUFS];


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
        set_duty(i, opts.duty[i]);

    return true;
}

uint16_t get_rpm(uint8_t fan)
{
    uint32_t time = pulseIn(pins_rpm[fan], LOW, RPM_TIMEOUT);

    return time ? (60UL * 1000UL * 1000UL / 4UL) / time : 0;
}

double get_temp(uint8_t sensor)
{
    // Steinhart–Hart coefficients
    static const float a = 1.009249522e-03;
    static const float b = 2.378405444e-04;
    static const float c = 2.019202697e-07;

    int v0 = analogRead(pins_tmp[sensor]);
    if (!v0)
        return TMP_NCONN;

    float r2 = TMP_R * (1023.0 / (float)v0 - 1.0);
    float log_r2 = log(r2);
    float temp = 1.0 / (a + b*log_r2 + c*log_r2*log_r2*log_r2);

    return temp - 273.15;
}

void set_duty(uint8_t fan, uint8_t value)
{
    if (value > 100)
        value = 100;
    fan_duty[fan] = value;

    int duty = (int)value * (fan == 1 ? TIMER4_TOP : TIMER13_TOP);
    duty /= 100;

    switch (fan) {
        case 0: OCR1A = duty; break;
        case 1: OCR4D = duty; break;
        case 2: OCR3A = duty; break;
        case 3: OCR1B = duty; break;
    }
}

void fan_scan()
{
    FOREACH_FAN(i)
        set_duty(i, SCAN_DUTY);

    delay(SCAN_SETTLE);

    FOREACH_FAN(i) {
        FOREACH_U8(n, SCAN_TRIES) {
            fan_connected[i] = get_rpm(i) > 0;
            if (fan_connected[i])
                break;
        }
    }
}

void print_status()
{
    FOREACH_FAN(i) {
        char *pbuf = buffer;
        pbuf += sprintf(pbuf, "Fan %d: ", i+1);
        if (fan_connected[i])
            pbuf += sprintf(pbuf, "%02d%% @ %u rpm", fan_duty[i], fan_rpm[i]);
        else
            pbuf += sprintf(pbuf, "disconnected");
        Serial.println(buffer);
    }

    FOREACH_TMP(i) {
        char *pbuf = buffer;
        pbuf += sprintf(pbuf, "Temp %d: ", i+1);
        if (temp[i] != TMP_NCONN) {
            dtostrf(temp[i], 4, 2, pbuf);
            pbuf += strlen(pbuf);
            pbuf += sprintf(pbuf, " °C");
        } else {
            pbuf += sprintf(pbuf, "disconnected");
        }
        Serial.println(buffer);
    }
}

void handle_serial()
{
    byte read = Serial.readBytesUntil('\n', buffer, SERIAL_BUFS);
    if (read == 0)
        return;
    buffer[read] = '\0';
    if (buffer[read-1] == '\r')
        buffer[read-1] = '\0';

    char *command = strtok(buffer, " ");
    if (command == NULL) {
        S_EPUTS("no command given");
        return;
    }
    char *arg1 = strtok(NULL, " ");
    char *arg2 = strtok(NULL, " ");

    FOREACH_U8(i, sizeof(commands) / sizeof(command_t)) {
        if (strcmp(command, commands[i].name) == 0) {
            commands[i].handler(arg1, arg2);
            return;
        }
    }

    Serial.print("Error: unknown command '");
    Serial.print(command);
    Serial.println("'");
}

void cmd_set(const char *s_fan, const char *s_duty)
{
    if (s_fan == NULL) {
        S_EPUTS("no fan no. given");
        return;
    }
    if (s_duty == NULL) {
        S_EPUTS("no duty value given");
        return;
    }
    int fan = atoi(s_fan);
    int duty = atoi(s_duty);

    if (fan <= 0 || fan > NUM_FAN) {
        S_ERROR("invalid fan no. '%d'", fan);
        return;
    }
    if (duty < 0 || duty > 100) {
        S_ERROR("invalid fan duty '%d'", duty);
        return;
    }

    S_PRINTF("Setting Fan %d duty %d%%", fan, duty);

    fan--;
    set_duty(fan, duty);
    opts.duty[fan] = duty;
}

void cmd_status(const char *s_interval, const char*)
{
    if (s_interval == NULL) {
        print_status();
        return;
    }

    int interval = atoi(s_interval);
    if (interval < 0 || interval > UINT8_MAX) {
        S_ERROR("invalid interval '%d'", interval);
        return;
    }

    S_PRINTF("Setting status interval %d", interval);

    opts.stats_int = (uint8_t)interval;
}

void cmd_load(const char *, const char*)
{
    if (opts_load())
        S_PUTS("Settings loaded from EEPROM");
    else
        S_PUTS("Failed to load settings from EEPROM!");
}

void cmd_save(const char*, const char*)
{
    opts_save();
    S_PUTS("Settings saved to EEPROM");
}

void cmd_map(const char *s_fan, const char *s_tmp)
{
    int fan = atoi(s_fan);
    if (fan <= 0 || fan > NUM_FAN) {
        S_ERROR("invalid fan no. '%d'", fan);
        return;
    }

    if (s_tmp == NULL) {
        S_PRINTF("Fan %d mapped to sensor %d", fan, opts.mapping[fan-1]+1);
        return;
    }

    int tmp = atoi(s_tmp);
    if (tmp <= 0 || tmp > NUM_TMP) {
        S_ERROR("invalid sensor no. '%d'", tmp);
        return;
    }

    S_PRINTF("Mapping fan %d to sensor %d", fan, tmp);

    opts.mapping[fan-1] = (uint8_t)tmp-1;
}

void cmd_curve(const char*, const char*)
{
    uint16_t rpm[NUM_FAN][CURVE_SMPNUM];

    // CSV header
    char *pbuf = buffer;
    pbuf += sprintf(pbuf, "duty,");
    FOREACH_FAN(i)
        pbuf += sprintf(pbuf, "fan%d,", i+1);
    Serial.println(buffer);

    // CSV data rows
    for (int duty=100; duty>=0; duty-=CURVE_STEP) {
        FOREACH_FAN(i)
            set_duty(i, duty);
        delay(CURVE_SDELAY);

        FOREACH_U8(n, CURVE_SMPNUM) {
            FOREACH_FAN(i)
                rpm[i][n] = get_rpm(i);
            delay(CURVE_SMPDEL);
        }
        FOREACH_FAN(i) {
            for (int n=1; n<CURVE_SMPNUM; n++)
                rpm[i][0] += rpm[i][n];
            rpm[i][0] /= CURVE_SMPNUM;
        }

        pbuf = buffer;
        pbuf += sprintf(pbuf, "%d,", duty);
        FOREACH_FAN(i)
            pbuf += sprintf(pbuf, "%u,", rpm[i][0]);
        Serial.println(buffer);
    }

    // restore manual duty
    FOREACH_FAN(i)
        set_duty(i, opts.duty[i]);
}

void cmd_help(const char*, const char*)
{
    S_PUTS("Available commands:");
    FOREACH_U8(i, sizeof(commands) / sizeof(command_t)) {
        S_PRINTF("    %s", commands[i].name);
    }
}

void cmd_version(const char*, const char*)
{
    S_PUTS("Version: " VERSION);
    S_PUTS("Built:   " __DATE__ " " __TIME__);
}


void setup()
{
    // I/O pins
    FOREACH_FAN(i) {
        pinMode(pins_pwm[i], OUTPUT);
        pinMode(pins_rpm[i], INPUT);
    }
    FOREACH_TMP(i)
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

    // scan connected fans
    fan_scan();

    // initialize defaults
    opts.mode = DEF_MODE;
    opts.stats_int = DEF_SINT;
    FOREACH_FAN(i) {
        fan_rpm[i] = 0;
        opts.duty[i] = DEF_DUTY;
        opts.mapping[i] = DEF_MAP;
        set_duty(i, fan_connected[i] ? DEF_DUTY : 0);
    }

    // load settings
    opts_load();

    // serial
    Serial.begin(SERIAL_BAUD);
    Serial.setTimeout(SERIAL_TIMO);

    S_PUTS("Initialization complete");
}

void loop()
{
    uint32_t now = millis();

    static uint32_t measure_next = 0;
    if (now > measure_next) {
        measure_next = now + UPDATE_INT;
        FOREACH_FAN(i)
            if (fan_connected[i])
                fan_rpm[i] = get_rpm(i);
        FOREACH_TMP(i)
            temp[i] = get_temp(i);
    }

    static uint32_t status_next = 0;
    if (opts.stats_int && now > status_next) {
        status_next = now + opts.stats_int * 1000;
        print_status();
    }

    if (Serial.available())
        handle_serial();
}

/* vim: set ts=4 sw=4 et */
