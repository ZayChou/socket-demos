/*
 * windows/03_iocp_async/client.c
 *
 * IOCP demo client — same echo protocol as the other Windows demos.
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/winsock_helpers.h"

#define HOST "127.0.0.1"
#define PORT 9003
#define BUF  256

static void send_echo(SOCKET fd, const char *msg)
{
    char buf[BUF];
    int len = (int)strlen(msg);

    if (send_all(fd, msg, len) < 0) die_wsa("send");

    int n = recv(fd, buf, BUF - 1, 0);
    if (n <= 0) die_wsa("recv");
    buf[n] = '\0';
    printf("[client] echo: %s", buf);

    if (n != len || memcmp(buf, msg, (size_t)len) != 0) {
        fprintf(stderr, "[client] echo mismatch!\n");
        exit(EXIT_FAILURE);
    }
}

int main(void)
{
    winsock_init();

    SOCKET fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == INVALID_SOCKET) die_wsa("socket");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, HOST, &addr.sin_addr);
    addr.sin_port = htons(PORT);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
        die_wsa("connect");
    printf("[client] connected to %s:%d\n", HOST, PORT);

    send_echo(fd, "hello\n");
    send_echo(fd, "ping\n");
    send_echo(fd, "bye\n");

    closesocket(fd);
    winsock_cleanup();
    printf("[client] done.\n");
    return 0;
}

