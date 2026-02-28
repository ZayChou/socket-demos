/*
 * windows/01_blocking_sync/server.c
 *
 * Blocking synchronous TCP echo server (Windows / Winsock).
 *
 * Model: single thread — accept one client, echo text lines until "bye",
 *        then exit.  Mirrors linux/01_blocking_sync using Winsock APIs.
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

#define PORT    9001
#define BACKLOG 4
#define BUF     256

static void handle_client(SOCKET cfd)
{
    char buf[BUF];
    int n;

    while ((n = recv(cfd, buf, BUF - 1, 0)) > 0) {
        buf[n] = '\0';
        printf("[server] recv: %s", buf);
        if (send_all(cfd, buf, n) < 0) {
            fprintf(stderr, "send failed: %d\n", WSAGetLastError());
            break;
        }
        if (strncmp(buf, "bye", 3) == 0)
            break;
    }
    closesocket(cfd);
}

int main(void)
{
    winsock_init();

    SOCKET sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sfd == INVALID_SOCKET) die_wsa("socket");

    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(PORT);

    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
        die_wsa("bind");
    if (listen(sfd, BACKLOG) == SOCKET_ERROR)
        die_wsa("listen");
    printf("[server] listening on port %d\n", PORT);

    struct sockaddr_in ca;
    int cl = sizeof(ca);
    SOCKET cfd = accept(sfd, (struct sockaddr *)&ca, &cl);
    if (cfd == INVALID_SOCKET) die_wsa("accept");

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ca.sin_addr, ip, sizeof(ip));
    printf("[server] client connected: %s\n", ip);

    handle_client(cfd);

    closesocket(sfd);
    winsock_cleanup();
    printf("[server] done.\n");
    return 0;
}

