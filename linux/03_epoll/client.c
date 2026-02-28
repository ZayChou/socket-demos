/*
 * linux/03_epoll/client.c
 *
 * epoll demo client — same echo protocol as the other demos.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../common/sock_helpers.h"

#define HOST "127.0.0.1"
#define PORT 9003
#define BUF  256

static void send_echo(int fd, const char *msg)
{
    char buf[BUF];
    size_t len = strlen(msg);

    if (write_all(fd, msg, len) < 0) die("send");

    ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0);
    if (n <= 0) die("recv");
    buf[n] = '\0';
    printf("[client] echo: %s", buf);

    if ((size_t)n != len || memcmp(buf, msg, len) != 0) {
        fprintf(stderr, "[client] echo mismatch!\n");
        exit(EXIT_FAILURE);
    }
}

int main(void)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) die("socket");

    struct sockaddr_in addr = {0};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = inet_addr(HOST);
    addr.sin_port        = htons(PORT);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) die("connect");
    printf("[client] connected to %s:%d\n", HOST, PORT);

    send_echo(fd, "hello\n");
    send_echo(fd, "ping\n");
    send_echo(fd, "bye\n");

    close(fd);
    printf("[client] done.\n");
    return 0;
}

