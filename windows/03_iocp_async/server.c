/*
 * windows/03_iocp_async/server.c
 *
 * I/O Completion Port (IOCP) asynchronous TCP echo server (Windows).
 *
 * Model:
 *   - Main thread: accept clients, associate each socket with the IOCP,
 *     post an initial WSARecv.
 *   - Worker thread: GetQueuedCompletionStatus loop.
 *     On recv completion  → echo data back with WSASend, post next WSARecv.
 *     On send completion  → post next WSARecv.
 *     On zero-byte recv   → client disconnected; free context.
 *   - Special completion key SHUTDOWN_KEY signals the worker to exit.
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/winsock_helpers.h"

#define PORT         9003
#define BACKLOG      4
#define BUF          256

/* Completion key that tells the worker to stop */
#define SHUTDOWN_KEY ((ULONG_PTR)1)

/* Per-socket I/O context */
typedef struct {
    OVERLAPPED  ov;          /* must be first */
    SOCKET      sock;
    WSABUF      wsabuf;
    char        buf[BUF];
    int         op;          /* 0 = recv, 1 = send */
    int         send_len;    /* bytes to send (send op) */
} io_ctx_t;

static HANDLE g_iocp;
static HANDLE g_done_event;  /* signalled when the last client session ends */


static io_ctx_t *alloc_ctx(SOCKET s)
{
    io_ctx_t *ctx = (io_ctx_t *)calloc(1, sizeof(*ctx));
    if (!ctx) { fprintf(stderr, "calloc failed\n"); exit(EXIT_FAILURE); }
    ctx->sock = s;
    return ctx;
}

static void post_recv(io_ctx_t *ctx)
{
    memset(&ctx->ov, 0, sizeof(ctx->ov));
    ctx->op           = 0;
    ctx->wsabuf.buf   = ctx->buf;
    ctx->wsabuf.len   = BUF - 1;
    DWORD flags       = 0;
    DWORD bytes       = 0;
    int rc = WSARecv(ctx->sock, &ctx->wsabuf, 1, &bytes, &flags,
                     &ctx->ov, NULL);
    if (rc == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        fprintf(stderr, "WSARecv failed: %d\n", WSAGetLastError());
        closesocket(ctx->sock);
        free(ctx);
    }
}

static void post_send(io_ctx_t *ctx, int len)
{
    memset(&ctx->ov, 0, sizeof(ctx->ov));
    ctx->op           = 1;
    ctx->send_len     = len;
    ctx->wsabuf.buf   = ctx->buf;
    ctx->wsabuf.len   = (ULONG)len;
    DWORD bytes       = 0;
    int rc = WSASend(ctx->sock, &ctx->wsabuf, 1, &bytes, 0,
                     &ctx->ov, NULL);
    if (rc == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        fprintf(stderr, "WSASend failed: %d\n", WSAGetLastError());
        closesocket(ctx->sock);
        free(ctx);
    }
}

/* Worker thread: processes completion events */
static DWORD WINAPI worker_thread(LPVOID arg)
{
    (void)arg;
    for (;;) {
        DWORD bytes      = 0;
        ULONG_PTR key    = 0;
        OVERLAPPED *pov  = NULL;

        BOOL ok = GetQueuedCompletionStatus(g_iocp, &bytes, &key, &pov, INFINITE);

        if (key == SHUTDOWN_KEY) break;   /* graceful shutdown signal */

        io_ctx_t *ctx = (io_ctx_t *)pov;

        if (!ok || bytes == 0) {
            /* Connection closed or error */
            printf("[server] client disconnected\n");
            closesocket(ctx->sock);
            free(ctx);
            continue;
        }

        if (ctx->op == 0) {
            /* Recv completed: echo data back */
            ctx->buf[bytes] = '\0';
            printf("[server] recv: %s", ctx->buf);
            int is_bye = (strncmp(ctx->buf, "bye", 3) == 0);
            post_send(ctx, (int)bytes);
            if (is_bye) {
                /* After the send completes the connection will be torn down */
            }
        } else {
            /* Send completed */
            int is_bye = (strncmp(ctx->buf, "bye", 3) == 0);
            if (is_bye) {
                printf("[server] client done.\n");
                closesocket(ctx->sock);
                free(ctx);
                SetEvent(g_done_event);  /* signal main thread */
            } else {
                post_recv(ctx);
            }
        }
    }
    return 0;
}

int main(void)
{
    winsock_init();

    g_done_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!g_done_event) {
        fprintf(stderr, "CreateEvent failed: %lu\n", GetLastError());
        exit(EXIT_FAILURE);
    }

    g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
    if (!g_iocp) {
        fprintf(stderr, "CreateIoCompletionPort failed: %lu\n", GetLastError());
        exit(EXIT_FAILURE);
    }

    /* Start one worker thread */
    HANDLE wt = CreateThread(NULL, 0, worker_thread, NULL, 0, NULL);
    if (!wt) {
        fprintf(stderr, "CreateThread failed: %lu\n", GetLastError());
        exit(EXIT_FAILURE);
    }

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

    /* Accept one client for this demo */
    struct sockaddr_in ca;
    int cl = sizeof(ca);
    SOCKET cfd = accept(sfd, (struct sockaddr *)&ca, &cl);
    if (cfd == INVALID_SOCKET) die_wsa("accept");

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ca.sin_addr, ip, sizeof(ip));
    printf("[server] client connected: %s\n", ip);

    /* Associate client socket with IOCP */
    if (!CreateIoCompletionPort((HANDLE)cfd, g_iocp, 0, 0)) {
        fprintf(stderr, "IOCP associate failed: %lu\n", GetLastError());
        exit(EXIT_FAILURE);
    }

    /* Post initial recv */
    io_ctx_t *ctx = alloc_ctx(cfd);
    post_recv(ctx);

    /* Wait for worker to finish (signalled when "bye" send completes) */
    WaitForSingleObject(g_done_event, 5000);
    PostQueuedCompletionStatus(g_iocp, 0, SHUTDOWN_KEY, NULL);
    WaitForSingleObject(wt, 5000);
    CloseHandle(wt);
    CloseHandle(g_done_event);

    closesocket(sfd);
    CloseHandle(g_iocp);
    winsock_cleanup();
    printf("[server] done.\n");
    return 0;
}

