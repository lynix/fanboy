/* Copyright (c) 2020 Alexander Koch
 *
 * This file is part of a project that is distributed under the terms of the MIT
 * License, see file 'LICENSE'.
 */

#include <pthread.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "serial.h"


extern const char *error;

static const speed_t  BAUD   = B57600;
static const uint8_t  TMO_CS = 5;

static int fd = -1;
pthread_mutex_t fd_lock = PTHREAD_MUTEX_INITIALIZER;


bool serial_send(const void *data, size_t len)
{
    pthread_mutex_lock(&fd_lock);

    size_t nwritten = 0;
    ssize_t ret = 0;
    while (nwritten < len &&
           (ret = write(fd, (char *)data+nwritten, len-nwritten)) > 0)
        nwritten += ret;

    if (ret < 0)
        error = strerror(errno);

    pthread_mutex_unlock(&fd_lock);

    return nwritten == len;
}

bool serial_receive(void *data, size_t len, int retries)
{
    pthread_mutex_lock(&fd_lock);

    size_t nread = 0;
    ssize_t ret = 0;
    int try = 0;
    while (nread < len && try <= retries) {
        ret = read(fd, (char *)data+nread, len-nread);
        if (ret < 0) {
            // I/O error
            error = strerror(errno);
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

    pthread_mutex_unlock(&fd_lock);

    return nread == len;
}



bool serial_open(const char *dev)
{
    pthread_mutex_lock(&fd_lock);
    bool ret = false;
    
    if (fd >= 0) {
        error = "already initialized";
        goto cleanup;
    }
    
    fd = open(dev, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        error = strerror(errno);
        goto cleanup;
    }
	usleep(10000);
    
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        error = strerror(errno);
        goto cleanup;
    }
    struct termios old = tty;
    
    // 57600 baud
    cfsetspeed(&tty, BAUD);

    // 8N1, ignore modem ctrl
    tty.c_cflag |= CS8 | CLOCAL | CREAD;
    tty.c_iflag |= IGNPAR | IGNBRK;
    tty.c_oflag &= ~OPOST;

	cfmakeraw(&tty);

    // set non-canonical mode: blocking read and timeout
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = TMO_CS;

    // apply only if changes
    if (memcmp(&tty, &old, sizeof(struct termios)) != 0) {
        if (tcsetattr(fd, TCSANOW, &tty) != 0) {
            error = strerror(errno);
            goto cleanup;
        }
    }    
    tcflush(fd, TCIOFLUSH);
	usleep(10000);

    ret = true;
    
cleanup:
    pthread_mutex_unlock(&fd_lock);

    return ret;
}

void serial_close()
{
    pthread_mutex_lock(&fd_lock);
    
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
    
    pthread_mutex_unlock(&fd_lock);
}

