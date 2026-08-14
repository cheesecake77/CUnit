#ifndef CUNIT_H
#define CUNIT_H

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>

/* === ANSI escape codes === */
#define BLUE       "\e[0;94m"
#define GREEN      "\x1B[32m"
#define RED        "\x1B[31m"
#define BOLD       "\e[1m"
#define ANSI_CLOSE "\x1B[0m"

/* ====================== Print utils ========================================================================= */
#define PRINT_PASSED(str)     printf("%s%s%s: %sPASSED%s\n\n", BLUE, str, ANSI_CLOSE, GREEN, ANSI_CLOSE);
#define PRINT_FAILED(str)     printf("%s%s%s: %sFAILED%s\n",   BLUE, str, ANSI_CLOSE, RED  , ANSI_CLOSE);
#define PRINT_FAILED_SIG(sig) printf(" -> Test failed with signal %d\n\n", sig);
#define PRINT_FAILED_SEGFAULT printf(" -> Test failed due to SEGFAULT\n\n");

/* ================= Assertions ================= */
#define ASSERT_TRUE(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "Assert failed: Condition was false\n"); \
        exit(EXIT_FAILURE); \
    } \
} while(0)

#define ASSERT_FALSE(condition) do { \
    if ((condition)) { \
        fprintf(stderr, "Assert failed: Condition was true\n"); \
        exit(EXIT_FAILURE); \
    } \
} while(0)

#define ASSERT_EQ_INT(expected, actual) if((expected) != (actual)) { \
    fprintf(stderr, "Assert failed: Expected %d, got %d\n", expected, actual); \
    exit(EXIT_FAILURE); \
}

#define ASSERT_EQ_FLOAT(expected, actual, eps) do { \
    double diff = fabs((actual) - (expected)); \
    if(diff > eps) { \
        fprintf(stderr, "Assert failed: Expected %f, got %f. Epsilon is %f\n", (double)expected, (double)actual, (double)eps); \
        exit(EXIT_FAILURE); \
    } \
} while(0)

#define ASSERT_EQ_STR(expected, actual) do { \
    int i = 0; \
    for (; expected[i] && actual[i]; i++) { \
        if (expected[i] != actual[i]) { \
            fprintf(stderr, "Assertion failed: Strings differ at index %d.\n Expected \"%s\", got \"%s\"\n", i, expected, actual); \
            exit(EXIT_FAILURE); \
        } \
    } \
    if (expected[i] != actual[i]) { \
        fprintf(stderr, "Assertion failed: Strings differ at index %d.\n Expected \"%s\", got \"%s\"\n", i, expected, actual); \
        exit(EXIT_FAILURE); \
    } \
} while(0)

#define ASSERT_EQ_CUSTOM(expected, actual, comparer, fail_string) do { \
    if (!(comparer((actual),(expected)))) { \
        fprintf(stderr, "Assert failed: %s\n", fail_string); \
        exit(EXIT_FAILURE); \
    } \
} while(0)

#define ASSERT_NULL(ptr) do { \
    if((ptr) != NULL) { \
        fprintf(stderr, "Assert failed: Pointer is not null\n"); \
        exit(EXIT_FAILURE); \
    } \
} while(0)

#define ASSERT_NOT_NULL(ptr) do {\
    if((ptr) == NULL) { \
        fprintf(stderr, "Assert failed: Pointer is null\n"); \
        exit(EXIT_FAILURE); \
    } \
} while(0)


/* ================================== Implementation ================================== */
#define TEST(name) \
    void name(void); \
    __attribute__((constructor)) void register_##name() { register_test(#name, name); } \
    void name(void)

typedef void (*testcase)(void);

typedef struct {
    const char *name;
    testcase test;
} cunit_test_t;

static int test_count = 0;
static int register_size = 0;
static int passed = 0;
static int failed = 0;
static cunit_test_t *tests = NULL;
static const int buffer_size = 10;

void register_test(const char *name, testcase test)
{
    if (test_count >= (register_size - 1)) {
        cunit_test_t* tmp = (cunit_test_t *)realloc(tests, sizeof(cunit_test_t) * (register_size + buffer_size));
        if (tmp == NULL) {
            fprintf(stderr, "Failed to allocate memory for test register\n");
            exit(1);
        }
        tests = tmp;
        register_size += buffer_size;
    }

    tests[test_count].name = name;
    tests[test_count].test = test;
    test_count++;
}

void run_tests(void)
{
    int pid, status;
    for (int i = 0; i < test_count; i++)
    {
        int pipefd[2];

        if (pipe(pipefd) < 0) {
            perror("Failed to create pipe\n");
            exit(1);
        }

        if ((pid = fork()) < 0) {
            perror("Failed to fork\n");
            exit(1);
        }

        if (pid == 0) {
            close(pipefd[0]);
            dup2(pipefd[1], STDERR_FILENO);
            close(pipefd[1]);
            tests[i].test();
            exit(0);
        }

        else {
            close(pipefd[1]);
            waitpid(pid, &status, 0);

            char cunit_error[1024] = {0};
            ssize_t n = read(pipefd[0], cunit_error, sizeof(cunit_error) - 1);
            close(pipefd[0]);

            if (WIFSIGNALED(status)) {
                PRINT_FAILED(tests[i].name);
                if (WTERMSIG(status) == SIGSEGV)
                    PRINT_FAILED_SEGFAULT
                else
                    PRINT_FAILED_SIG(WTERMSIG(status));
                failed++;
            }

            else if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                PRINT_PASSED(tests[i].name);
                passed++;
            }

            else {
                PRINT_FAILED(tests[i].name);
                if (n > 0) printf(" -> %s\n", cunit_error);
                failed++;
            }
        }
    }

    printf(BOLD "== SUMMARY ==\n" ANSI_CLOSE);
    if (passed > 0) printf(GREEN "PASSED: %d/%d\n" ANSI_CLOSE, passed, test_count);
    if (failed > 0) printf(RED   "FAILED: %d/%d\n" ANSI_CLOSE, failed, test_count);

    free(tests);
    tests = NULL;
}

#endif /* CUNIT_H */
