/*
 * tests/integration/test_echo_integration.c
 *
 * End-to-end integration test for all three Linux echo demos.
 *
 * For each demo pair (server + client):
 *   1. fork() a server process
 *   2. poll until the server's port is in use (up to 2 s)
 *   3. fork() a client process
 *   4. wait for the client to exit (exit code 0 = pass)
 *   5. wait for the server to exit on its own (it exits after last client)
 *
 * The SERVER_01/CLIENT_01 … macros are injected at compile time by CMake
 * using generator expressions, so the test always finds the correct binary
 * regardless of the build directory layout.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

/* CMake injects these as -DSERVER_01="..." etc. */
#ifndef SERVER_01
#  define SERVER_01 "linux01_server"
#endif
#ifndef CLIENT_01
#  define CLIENT_01 "linux01_client"
#endif
#ifndef SERVER_02
#  define SERVER_02 "linux02_server"
#endif
#ifndef CLIENT_02
#  define CLIENT_02 "linux02_client"
#endif
#ifndef SERVER_03
#  define SERVER_03 "linux03_server"
#endif
#ifndef CLIENT_03
#  define CLIENT_03 "linux03_client"
#endif

static int failures = 0;

/* Sleep for ms milliseconds. */
static void sleep_ms(int ms)
{
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

/*
 * Poll until the given TCP port is in use (server has called bind+listen).
 * Detects readiness by attempting to bind to the same port ourselves:
 *   - bind succeeds  → port is still free → not ready yet, retry
 *   - bind fails with EADDRINUSE → server is listening → ready
 * This approach never establishes a TCP connection so it cannot consume
 * the server's single accept() slot.
 * Returns 1 if ready, 0 on timeout.
 */
static int wait_for_server(int port, int timeout_ms)
{
    int elapsed = 0;
    while (elapsed < timeout_ms) {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) { sleep_ms(20); elapsed += 20; continue; }

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port        = htons((uint16_t)port);

        int rc = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
        close(fd);

        if (rc < 0 && errno == EADDRINUSE)
            return 1;   /* server has the port bound — ready to accept */

        sleep_ms(20);
        elapsed += 20;
    }
    return 0;
}

/*
 * Run one server+client pair.
 * Returns 0 on success, non-zero on failure.
 */
static int run_pair(const char *server_bin, const char *client_bin,
                    const char *name, int port)
{
    printf("[integration] running %s\n", name);

    /* Fork server */
    pid_t spid = fork();
    if (spid < 0) { perror("fork server"); return 1; }
    if (spid == 0) {
        execl(server_bin, server_bin, (char *)NULL);
        perror("execl server");
        _exit(127);
    }

    /* Wait for server to bind and listen (port-in-use probe, up to 2 s) */
    if (!wait_for_server(port, 2000)) {
        fprintf(stderr, "[integration] %s: server did not become ready\n", name);
        kill(spid, SIGTERM);
        waitpid(spid, NULL, 0);
        failures++;
        return 1;
    }

    /* Fork client */
    pid_t cpid = fork();
    if (cpid < 0) {
        kill(spid, SIGTERM);
        waitpid(spid, NULL, 0);
        perror("fork client");
        return 1;
    }
    if (cpid == 0) {
        execl(client_bin, client_bin, (char *)NULL);
        perror("execl client");
        _exit(127);
    }

    /* Wait for client */
    int cstatus = 0;
    waitpid(cpid, &cstatus, 0);

    /* Wait for server (it exits after the last client; give it 2 s) */
    for (int i = 0; i < 20; i++) {
        pid_t r = waitpid(spid, NULL, WNOHANG);
        if (r == spid) break;
        sleep_ms(100);
    }
    /* If still running, kill it */
    if (kill(spid, 0) == 0) {
        kill(spid, SIGTERM);
        waitpid(spid, NULL, 0);
    }

    int ok = WIFEXITED(cstatus) && WEXITSTATUS(cstatus) == 0;
    if (ok) {
        printf("[integration] %s PASSED\n", name);
    } else {
        fprintf(stderr, "[integration] %s FAILED (client exit %d)\n",
                name, WEXITSTATUS(cstatus));
        failures++;
    }
    return ok ? 0 : 1;
}

int main(void)
{
    run_pair(SERVER_01, CLIENT_01, "01_blocking_sync",           9001);
    run_pair(SERVER_02, CLIENT_02, "02_nonblocking_select_sync", 9002);
    run_pair(SERVER_03, CLIENT_03, "03_epoll",                   9003);

    if (failures == 0) {
        printf("[integration] all tests PASSED\n");
        return EXIT_SUCCESS;
    }
    fprintf(stderr, "[integration] %d test(s) FAILED\n", failures);
    return EXIT_FAILURE;
}


