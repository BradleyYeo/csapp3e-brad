#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static int g_tests_run = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;
static int g_current_test_failed = 0;

#define TEST_SUITE_START() do { \
    g_tests_run = 0; \
    g_tests_passed = 0; \
    g_tests_failed = 0; \
    printf(ANSI_COLOR_CYAN "=== Running Test Suite ===" ANSI_COLOR_RESET "\n\n"); \
} while (0)

#define TEST_SUITE_END() do { \
    printf("\n" ANSI_COLOR_CYAN "=== Test Summary ===" ANSI_COLOR_RESET "\n"); \
    printf("Total Tests Run: %d\n", g_tests_run); \
    printf(ANSI_COLOR_GREEN "Passed: %d" ANSI_COLOR_RESET "\n", g_tests_passed); \
    if (g_tests_failed > 0) { \
        printf(ANSI_COLOR_RED "Failed: %d" ANSI_COLOR_RESET "\n", g_tests_failed); \
    } else { \
        printf(ANSI_COLOR_GREEN "All tests passed successfully!" ANSI_COLOR_RESET "\n"); \
    } \
    return (g_tests_failed > 0) ? 1 : 0; \
} while (0)

#define RUN_TEST(test_func) do { \
    g_tests_run++; \
    g_current_test_failed = 0; \
    printf("Running %s... ", #test_func); \
    test_func(); \
    if (g_current_test_failed == 0) { \
        g_tests_passed++; \
        printf(ANSI_COLOR_GREEN "[PASS]" ANSI_COLOR_RESET "\n"); \
    } else { \
        g_tests_failed++; \
        printf(ANSI_COLOR_RED "[FAIL]" ANSI_COLOR_RESET "\n"); \
    } \
} while (0)

#define ASSERT_TRUE(cond) do { \
    if (!(cond)) { \
        printf("\n  " ANSI_COLOR_RED "Assertion Failed: " #cond ANSI_COLOR_RESET " at %s:%d\n", __FILE__, __LINE__); \
        g_current_test_failed = 1; \
        return; \
    } \
} while (0)

#define ASSERT_FALSE(cond) ASSERT_TRUE(!(cond))

#define ASSERT_EQ_INT(actual, expected) do { \
    long long act = (long long)(actual); \
    long long exp = (long long)(expected); \
    if (act != exp) { \
        printf("\n  " ANSI_COLOR_RED "Assertion Failed:" ANSI_COLOR_RESET " expected %lld, got %lld at %s:%d\n", exp, act, __FILE__, __LINE__); \
        g_current_test_failed = 1; \
        return; \
    } \
} while (0)

#define ASSERT_NOT_NULL(ptr) do { \
    if ((ptr) == NULL) { \
        printf("\n  " ANSI_COLOR_RED "Assertion Failed: pointer is NULL" ANSI_COLOR_RESET " at %s:%d\n", __FILE__, __LINE__); \
        g_current_test_failed = 1; \
        return; \
    } \
} while (0)

#endif // TEST_FRAMEWORK_H
