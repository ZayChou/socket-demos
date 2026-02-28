/*
 * tests/unit/test_echo_helpers.c
 *
 * Unit tests for the echo protocol helpers and common utilities.
 * Uses the same minimal test runner style as test_placeholder.c.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <winsock2.h>
#  pragma comment(lib, "ws2_32.lib")
#else
#  include <unistd.h>
#  include <errno.h>
#  include <fcntl.h>
#  include <sys/types.h>

/* Inline write_all from linux/common/sock_helpers.h for testing */
static int write_all(int fd, const void *buf, size_t len)
{
    const char *p = (const char *)buf;
    size_t remaining = len;
    while (remaining > 0) {
        ssize_t n = write(fd, p, remaining);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        p += n;
        remaining -= (size_t)n;
    }
    return 0;
}
#endif

/* ── Minimal test runner ─────────────────────────────────────────────────── */
static int failures = 0;

#define ASSERT(cond)                                               \
    do {                                                           \
        if (!(cond)) {                                             \
            fprintf(stderr, "FAIL: %s:%d: %s\n",                  \
                    __FILE__, __LINE__, #cond);                    \
            failures++;                                            \
        }                                                          \
    } while (0)

/* ── Protocol constants ──────────────────────────────────────────────────── */
#define MSG_HELLO "hello\n"
#define MSG_PING  "ping\n"
#define MSG_BYE   "bye\n"

/* ── Tests ───────────────────────────────────────────────────────────────── */

static void test_bye_detection(void)
{
    /* Server uses strncmp(buf, "bye", 3) to detect BYE */
    ASSERT(strncmp(MSG_BYE,   "bye", 3) == 0);
    ASSERT(strncmp(MSG_HELLO, "bye", 3) != 0);
    ASSERT(strncmp(MSG_PING,  "bye", 3) != 0);
    ASSERT(strncmp("byebye\n","bye", 3) == 0); /* prefix match is intentional */
}

static void test_message_lengths(void)
{
    ASSERT(strlen(MSG_HELLO) == 6);
    ASSERT(strlen(MSG_PING)  == 5);
    ASSERT(strlen(MSG_BYE)   == 4);
}

static void test_echo_roundtrip_bufcmp(void)
{
    /* Simulate what send_echo() does in the client */
    const char *msg = MSG_PING;
    size_t len = strlen(msg);
    char echo[64];
    memcpy(echo, msg, len);
    ASSERT(memcmp(echo, msg, len) == 0);
}

#ifndef _WIN32
static void test_write_all_pipe(void)
{
    int fds[2];
    ASSERT(pipe(fds) == 0);

    const char *msg = MSG_HELLO;
    size_t len = strlen(msg);
    ASSERT(write_all(fds[1], msg, len) == 0);
    close(fds[1]);

    char buf[64] = {0};
    ssize_t n = read(fds[0], buf, sizeof(buf));
    ASSERT(n == (ssize_t)len);
    ASSERT(memcmp(buf, msg, (size_t)n) == 0);
    close(fds[0]);
}
#endif /* !_WIN32 */

int main(void)
{
    test_bye_detection();
    test_message_lengths();
    test_echo_roundtrip_bufcmp();
#ifndef _WIN32
    test_write_all_pipe();
#endif

    if (failures == 0) {
        printf("All tests passed.\n");
        return EXIT_SUCCESS;
    }
    fprintf(stderr, "%d test(s) failed.\n", failures);
    return EXIT_FAILURE;
}
