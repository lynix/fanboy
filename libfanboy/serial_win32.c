/* Copyright (c) 2020 Alexander Koch
 *
 * This file is part of a project that is distributed under the terms of the MIT
 * License, see file 'LICENSE'.
 */

#ifdef __GNUC__
#ifdef __MINGW32__
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
// for AcquireSRWLockExclusive and the-like
#define _WIN32_WINNT 0x0600
#endif
#endif

#include <Windows.h>
#include <stdio.h>
#include <synchapi.h>

#include "serial.h"

#define ERR_LEN 1024

extern const char *error;

static HANDLE fd = INVALID_HANDLE_VALUE;
static SRWLOCK fd_lock = SRWLOCK_INIT;
static char err_string[ERR_LEN];
static const int SERIAL_TIMEOUT = 50;
static const int SERIAL_MULT = 20;


bool serial_send(const void *data, size_t len)
{
    AcquireSRWLockExclusive(&fd_lock);

    DWORD written = 0;
    if (WriteFile(fd, data, len, &written, NULL) == FALSE)
    {
        snprintf(err_string, ERR_LEN-1, "Failed to write to serial port (%lu)",
                 GetLastError());
        error = err_string;
        return false;
    }

    ReleaseSRWLockExclusive(&fd_lock);

    return written == len;
}

bool serial_receive(void *data, size_t len, int retries)
{
    AcquireSRWLockExclusive(&fd_lock);

    size_t nread = 0;
    DWORD ret = 0;
    int try = 0;
    while (nread < len && try <= retries) {
        if (!ReadFile(fd, (char *)data+nread, len-nread, &ret, NULL)) {
            // I/O error
            snprintf(err_string, ERR_LEN-1,
                     "Failed to read from serial port (%lu)", GetLastError());
            error = err_string;
            break;
        } else if (ret == 0) {
            // no data
            try++;
        } else {
            // successful read (may be partial)
            try = 0;
            nread += ret;
        }
    }

    if (ret == 0)
        error = "timeout receiving data";

    ReleaseSRWLockExclusive(&fd_lock);

    return nread == len;
}

bool serial_open(const char *dev)
{
    AcquireSRWLockExclusive(&fd_lock);
    bool ret = false;

    if (fd != INVALID_HANDLE_VALUE) {
        error = "already initialized";
        goto cleanup;
    }

    fd = CreateFile(dev, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                    0, NULL);
    if (fd == INVALID_HANDLE_VALUE) {
        snprintf(err_string, ERR_LEN-1, "Failed to open serial port (%lu)",
                 GetLastError());
        error = err_string;
        goto cleanup;
    }

    DCB params = { 0 };
    params.DCBlength = sizeof(params);
    if (GetCommState(fd, &params) == FALSE)
    {
        error = "Failed to read serial port state";
        goto cleanup;
    }

    DCB old_params = params;

    params.BaudRate = CBR_57600;
    params.ByteSize = 8;
    params.Parity = NOPARITY;
    params.StopBits = ONESTOPBIT;

    if (memcmp(&params, &old_params, sizeof(DCB))) {
        if (SetCommState(fd, &params) == FALSE) {
            error = "Failed to set serial port parameters";
            goto cleanup;
        }
    }

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = SERIAL_TIMEOUT;
    timeouts.ReadTotalTimeoutConstant = SERIAL_TIMEOUT;
    timeouts.ReadTotalTimeoutMultiplier = SERIAL_MULT;
    timeouts.WriteTotalTimeoutConstant = SERIAL_TIMEOUT;
    timeouts.WriteTotalTimeoutMultiplier = SERIAL_MULT;

    if (SetCommTimeouts(fd, &timeouts) == FALSE)
    {
        error = "Failed to set serial timeouts";
        goto cleanup;
    }

    ret = true;

cleanup:
    ReleaseSRWLockExclusive(&fd_lock);

    return ret;
}

void serial_close()
{
    AcquireSRWLockExclusive(&fd_lock);

    if (fd != INVALID_HANDLE_VALUE) {
        CloseHandle(fd);
        fd = INVALID_HANDLE_VALUE;
    }

    ReleaseSRWLockExclusive(&fd_lock);
}

