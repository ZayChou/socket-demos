/*
 * linux/01_blocking_sync/server.c
 *
 * Blocking synchronous TCP echo server.
 *
 * Model: single thread — accept one client, echo text lines until "bye",
 *        then exit.  Simplest possible TCP server; no concurrency.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../common/sock_helpers.h"

#define PORT    9001
#define BACKLOG 4
#define BUF     256

static void handle_client(int cfd)
{
    char buf[BUF];
    ssize_t n;

    while ((n = recv(cfd, buf, sizeof(buf) - 1, 0)) > 0) {
        buf[n] = '\0';
        printf("[server] recv: %s", buf);
        if (write_all(cfd, buf, (size_t)n) < 0) {
            perror("send");
            break;
        }
        if (strncmp(buf, "bye", 3) == 0)
            break;
    }
    close(cfd);
}

int main(void)
{
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) die("socket");

    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(PORT);

    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) die("bind");
    if (listen(sfd, BACKLOG) < 0) die("listen");
    printf("[server] listening on port %d\n", PORT);

    struct sockaddr_in ca;
    socklen_t cl = sizeof(ca);
    int cfd = accept(sfd, (struct sockaddr *)&ca, &cl);
    if (cfd < 0) die("accept");
    printf("[server] client connected: %s\n", inet_ntoa(ca.sin_addr));

    handle_client(cfd);

    close(sfd);
    printf("[server] done.\n");
    return 0;
}

