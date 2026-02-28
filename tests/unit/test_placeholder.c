#include <stdio.h>
#include <stdlib.h>

/* Minimal test runner - no external framework required */
static int failures = 0;

#define ASSERT(cond)                                              \
    do {                                                          \
        if (!(cond)) {                                            \
            fprintf(stderr, "FAIL: %s:%d: %s\n",                 \
                    __FILE__, __LINE__, #cond);                   \
            failures++;                                           \
        }                                                         \
    } while (0)

static void test_arithmetic(void) {
    ASSERT(1 + 1 == 2);
    ASSERT(10 - 3 == 7);
}

static void test_string_literal(void) {
    const char *hello = "hello";
    ASSERT(hello[0] == 'h');
    ASSERT(hello[4] == 'o');
}

int main(void) {
    test_arithmetic();
    test_string_literal();

    if (failures == 0) {
        printf("All tests passed.\n");
        return EXIT_SUCCESS;
    }
    fprintf(stderr, "%d test(s) failed.\n", failures);
    return EXIT_FAILURE;
}
