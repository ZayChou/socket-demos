/*
 * linux/02_nonblocking_select_sync/server.c
 *
 * Non-blocking select()-based TCP echo server.
 *
 * Model: single thread, all sockets in non-blocking mode.
 *        select() multiplexes the listening socket and up to MAX_CLIENTS
 *        connected clients.  Exits when the last client disconnects.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../common/sock_helpers.h"

#define PORT        9002
#define BACKLOG     4
#define BUF         256
#define MAX_CLIENTS 63   /* FD_SETSIZE is 1024; keep array small for demo */

int main(void)
{
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) die("socket");

    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    set_nonblocking(sfd);

    struct sockaddr_in addr = {0};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(PORT);

    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) die("bind");
    if (listen(sfd, BACKLOG) < 0) die("listen");
    printf("[server] listening on port %d\n", PORT);

    int clients[MAX_CLIENTS];
    int nclients = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) clients[i] = -1;

    int running = 1;
    while (running) {
        fd_set rset;
        FD_ZERO(&rset);
        FD_SET(sfd, &rset);
        int maxfd = sfd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] >= 0) {
                FD_SET(clients[i], &rset);
                if (clients[i] > maxfd) maxfd = clients[i];
            }
        }

        int ready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (ready < 0) { perror("select"); break; }

        /* Accept new connections */
        if (FD_ISSET(sfd, &rset)) {
            struct sockaddr_in ca;
            socklen_t cl = sizeof(ca);
            int cfd = accept(sfd, (struct sockaddr *)&ca, &cl);
            if (cfd >= 0) {
                set_nonblocking(cfd);
                printf("[server] client connected: %s\n", inet_ntoa(ca.sin_addr));
                int placed = 0;
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i] < 0) {
                        clients[i] = cfd;
                        nclients++;
                        placed = 1;
                        break;
                    }
                }
                if (!placed) {
                    fprintf(stderr, "[server] too many clients\n");
                    close(cfd);
                }
            }
        }

        /* Service existing clients */
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] < 0 || !FD_ISSET(clients[i], &rset))
                continue;

            char buf[BUF];
            ssize_t n = recv(clients[i], buf, sizeof(buf) - 1, 0);
            if (n <= 0) {
                if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
                    continue;
                printf("[server] client disconnected (fd=%d)\n", clients[i]);
                close(clients[i]);
                clients[i] = -1;
                if (--nclients == 0) running = 0;
                continue;
            }

            buf[n] = '\0';
            printf("[server] recv (fd=%d): %s", clients[i], buf);
            write_all(clients[i], buf, (size_t)n);

            if (strncmp(buf, "bye", 3) == 0) {
                close(clients[i]);
                clients[i] = -1;
                if (--nclients == 0) running = 0;
            }
        }
    }

    close(sfd);
    printf("[server] done.\n");
    return 0;
}

