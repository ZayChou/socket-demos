/*
 * linux/03_epoll/server.c
 *
 * epoll edge-triggered non-blocking TCP echo server.
 *
 * Model: epoll_create1() with EPOLLET (edge-triggered) + non-blocking fds.
 *        Each fd must be fully drained on each readable event.
 *        Exits when the last client disconnects.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../common/sock_helpers.h"

#define PORT       9003
#define BACKLOG    4
#define BUF        256
#define MAX_EVENTS 32

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

    int epfd = epoll_create1(EPOLL_CLOEXEC);
    if (epfd < 0) die("epoll_create1");

    struct epoll_event ev;
    ev.events  = EPOLLIN | EPOLLET;
    ev.data.fd = sfd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &ev) < 0) die("epoll_ctl add sfd");

    struct epoll_event events[MAX_EVENTS];
    int nclients = 0;

    for (;;) {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (n < 0) { perror("epoll_wait"); break; }

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            if (fd == sfd) {
                /* Accept all pending connections (ET: must drain accept queue) */
                for (;;) {
                    struct sockaddr_in ca;
                    socklen_t cl = sizeof(ca);
                    int cfd = accept(sfd, (struct sockaddr *)&ca, &cl);
                    if (cfd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        perror("accept");
                        break;
                    }
                    set_nonblocking(cfd);
                    printf("[server] client connected: %s\n", inet_ntoa(ca.sin_addr));
                    ev.events  = EPOLLIN | EPOLLET;
                    ev.data.fd = cfd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
                    nclients++;
                }
            } else {
                /* Read all available data (ET: must drain until EAGAIN) */
                int close_fd = 0;
                for (;;) {
                    char buf[BUF];
                    ssize_t r = recv(fd, buf, sizeof(buf) - 1, 0);
                    if (r < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        perror("recv");
                        close_fd = 1;
                        break;
                    }
                    if (r == 0) { close_fd = 1; break; }

                    buf[r] = '\0';
                    printf("[server] recv (fd=%d): %s", fd, buf);
                    write_all(fd, buf, (size_t)r);

                    if (strncmp(buf, "bye", 3) == 0) { close_fd = 1; break; }
                }
                if (close_fd) {
                    printf("[server] client disconnected (fd=%d)\n", fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                    if (--nclients == 0) goto done;
                }
            }
        }
    }

done:
    close(epfd);
    close(sfd);
    printf("[server] done.\n");
    return 0;
}

