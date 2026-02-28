/*
 * windows/02_nonblocking_select_sync/server.c
 *
 * Non-blocking select()-based TCP echo server (Windows / Winsock).
 *
 * Model: single thread, all sockets in non-blocking mode (ioctlsocket).
 *        select() multiplexes the listening socket and connected clients.
 *        Exits when the last client disconnects.
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

#define PORT        9002
#define BACKLOG     4
#define BUF         256
#define MAX_CLIENTS 63

static void set_nonblocking_win(SOCKET s)
{
    u_long mode = 1;
    if (ioctlsocket(s, FIONBIO, &mode) != 0)
        die_wsa("ioctlsocket FIONBIO");
}

int main(void)
{
    winsock_init();

    SOCKET sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sfd == INVALID_SOCKET) die_wsa("socket");

    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
    set_nonblocking_win(sfd);

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

    SOCKET clients[MAX_CLIENTS];
    int nclients = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) clients[i] = INVALID_SOCKET;

    int running = 1;
    while (running) {
        fd_set rset;
        FD_ZERO(&rset);
        FD_SET(sfd, &rset);

        for (int i = 0; i < MAX_CLIENTS; i++)
            if (clients[i] != INVALID_SOCKET)
                FD_SET(clients[i], &rset);

        int ready = select(0, &rset, NULL, NULL, NULL);
        if (ready == SOCKET_ERROR) { fprintf(stderr, "select: %d\n", WSAGetLastError()); break; }

        /* Accept new connections */
        if (FD_ISSET(sfd, &rset)) {
            struct sockaddr_in ca;
            int cl = sizeof(ca);
            SOCKET cfd = accept(sfd, (struct sockaddr *)&ca, &cl);
            if (cfd != INVALID_SOCKET) {
                set_nonblocking_win(cfd);
                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &ca.sin_addr, ip, sizeof(ip));
                printf("[server] client connected: %s\n", ip);
                int placed = 0;
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i] == INVALID_SOCKET) {
                        clients[i] = cfd; nclients++; placed = 1; break;
                    }
                }
                if (!placed) { fprintf(stderr, "too many clients\n"); closesocket(cfd); }
            }
        }

        /* Service existing clients */
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] == INVALID_SOCKET || !FD_ISSET(clients[i], &rset))
                continue;

            char buf[BUF];
            int n = recv(clients[i], buf, BUF - 1, 0);
            if (n <= 0) {
                if (n == SOCKET_ERROR) {
                    int err = WSAGetLastError();
                    if (err == WSAEWOULDBLOCK) continue;
                }
                printf("[server] client disconnected (fd=%llu)\n", (unsigned long long)clients[i]);
                closesocket(clients[i]);
                clients[i] = INVALID_SOCKET;
                if (--nclients == 0) running = 0;
                continue;
            }
            buf[n] = '\0';
            printf("[server] recv: %s", buf);
            send_all(clients[i], buf, n);
            if (strncmp(buf, "bye", 3) == 0) {
                closesocket(clients[i]);
                clients[i] = INVALID_SOCKET;
                if (--nclients == 0) running = 0;
            }
        }
    }

    closesocket(sfd);
    winsock_cleanup();
    printf("[server] done.\n");
    return 0;
}

