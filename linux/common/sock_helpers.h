#ifndef SOCK_HELPERS_H
#define SOCK_HELPERS_H

/*
 * linux/common/sock_helpers.h
 *
 * Shared header-only helpers for all Linux socket demos.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

/* Print errno message and exit. */
static inline void die(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

/* Set fd to non-blocking mode. */
static inline void set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) die("fcntl F_GETFL");
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) die("fcntl F_SETFL");
}

/*
 * Write exactly len bytes; retries on EINTR and partial writes.
 * Returns 0 on success, -1 on error.
 */
static inline int write_all(int fd, const void *buf, size_t len)
{
    const char *p = (const char *)buf;
    size_t remaining = len;
    while (remaining > 0) {
        ssize_t n = write(fd, p, remaining);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        p += n;
        remaining -= (size_t)n;
    }
    return 0;
}

#endif /* SOCK_HELPERS_H */
