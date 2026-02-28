#ifndef WINSOCK_HELPERS_H
#define WINSOCK_HELPERS_H

/*
 * windows/common/winsock_helpers.h
 *
 * Shared header-only helpers for all Windows socket demos.
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

/* Initialise Winsock 2.2.  Call once at program start. */
static inline void winsock_init(void)
{
    WSADATA wsa;
    int rc = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (rc != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", rc);
        exit(EXIT_FAILURE);
    }
}

/* Clean up Winsock.  Call once before program exit. */
static inline void winsock_cleanup(void)
{
    WSACleanup();
}

/* Print a Winsock error message and exit. */
static inline void die_wsa(const char *msg)
{
    fprintf(stderr, "%s failed with error: %d\n", msg, WSAGetLastError());
    WSACleanup();
    exit(EXIT_FAILURE);
}

/*
 * Write exactly len bytes; retries on partial sends.
 * Returns 0 on success, -1 on error.
 */
static inline int send_all(SOCKET sock, const void *buf, int len)
{
    const char *p = (const char *)buf;
    int remaining = len;
    while (remaining > 0) {
        int n = send(sock, p, remaining, 0);
        if (n == SOCKET_ERROR) return -1;
        p += n;
        remaining -= n;
    }
    return 0;
}

#endif /* WINSOCK_HELPERS_H */
