#include <stdio.h>
#include <time.h>
#include <stdint.h>

#define TEST_LOOPS 10000000

/*
 * pagecount calculates how many memory pages of size pageSize
 * are required to hold memorySize bytes.
 *
 * Example:
 * pageSize = 4096 (2^12)
 * memorySize = 4096 -> 1 page
 * memorySize = 4097 -> 2 pages
 *
 * Optimization Stages to explore:
 * 1. Naive integer arithmetic: (memorySize + pageSize - 1) / pageSize
 * 2. Bitwise arithmetic (when pageSize is a power of 2):
 *    (memorySize + pageSize - 1) >> shift
 * 3. Compare execution time and generated assembly across compiler flags:
 *    gcc -O0 pagecount.c && ./a.out
 *    gcc -O1 pagecount.c && ./a.out
 *    gcc -O2 pagecount.c && ./a.out
 *    gcc -O3 pagecount.c && ./a.out
 * 4. Profile hardware metrics using Linux perf:
 *    perf stat ./a.out
 */

// Baseline calculation using integer division
uint64_t pagecount_div(uint64_t memorySize, uint64_t pageSize) {
    return (memorySize + pageSize - 1) / pageSize;
}

// TODO: Implement pagecount using bitshift and bitmask operations
// Assume pageSize is always a power of 2 (e.g. 4096 = 1 << 12)
uint64_t pagecount_bitwise(uint64_t memorySize, uint64_t pageSize, unsigned int shift) {
    // TODO: Replace division with right-shift
    return (memorySize + pageSize - 1) >> shift;
}

int main(void) {
    uint64_t pageSize = 4096;
    unsigned int shift = 12; // 2^12 = 4096
    uint64_t memorySize = 1000000;
    uint64_t result = 0;

    clock_t start = clock();
    for (int i = 0; i < TEST_LOOPS; i++) {
        result += pagecount_div(memorySize + i, pageSize);
    }
    clock_t end = clock();
    double time_div = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Division: %.4fs for %d iterations (res=%llu)\n", time_div, TEST_LOOPS, (unsigned long long)result);

    result = 0;
    start = clock();
    for (int i = 0; i < TEST_LOOPS; i++) {
        result += pagecount_bitwise(memorySize + i, pageSize, shift);
    }
    end = clock();
    double time_shift = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Bitshift: %.4fs for %d iterations (res=%llu)\n", time_shift, TEST_LOOPS, (unsigned long long)result);

    return 0;
}
