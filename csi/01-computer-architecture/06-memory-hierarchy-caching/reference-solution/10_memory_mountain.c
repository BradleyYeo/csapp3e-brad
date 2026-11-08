/*
 * Reference Solution: Exercise 10 (The Memory Mountain)
 */
#define _POSIX_C_SOURCE 200809L
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
#include <stdbool.h>
#ifndef nullptr
#define nullptr NULL
#endif
#endif

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_BYTES (64 * 1024 * 1024ULL) /* 64 MiB */
#define MAX_ELEMS (MAX_BYTES / sizeof(uint64_t))

static uint64_t g_data[MAX_ELEMS];

uint64_t run_mountain_pass(const uint64_t *data, size_t elems, size_t stride) {
  uint64_t acc = 0;
  for (size_t i = 0; i < elems; i += stride) {
    acc += data[i];
  }
  return acc;
}

static double measure_throughput(const uint64_t *data, size_t bytes, size_t stride) {
  size_t elems = bytes / sizeof(uint64_t);
  if (elems < stride) elems = stride;

  run_mountain_pass(data, elems, stride);

  size_t iterations = 1;
  while (true) {
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    uint64_t acc = 0;
    for (size_t it = 0; it < iterations; ++it) {
      acc += run_mountain_pass(data, elems, stride);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);

    double sec = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) * 1e-9;
    if (sec >= 0.02 || iterations >= 100000) {
      if (acc == 0xDEADBEEF) printf("Impossible!\n");

      double accessed_bytes = (double)(elems / stride) * sizeof(uint64_t) * (double)iterations;
      return (accessed_bytes / sec) / (1024.0 * 1024.0);
    }
    iterations *= 2;
  }
}

int main(void) {
  printf("Running Reference Solution: Exercise 10 (Memory Mountain)...\n\n");

  for (size_t i = 0; i < MAX_ELEMS; ++i) {
    g_data[i] = (uint64_t)(i * 6364136223846793005ULL + 1ULL);
  }

  uint64_t smoke = run_mountain_pass(g_data, 1024, 1);
  assert(smoke != 0);

  const size_t strides[] = {1, 2, 4, 8, 16, 32};
  const size_t num_strides = sizeof(strides) / sizeof(strides[0]);

  printf("Memory Mountain Read Throughput (MB/s):\n");
  printf("%-10s", "Size");
  for (size_t s = 0; s < num_strides; ++s) {
    printf("  s%-2zu   ", strides[s]);
  }
  printf("\n--------------------------------------------------------------\n");

  double l1_mb_s = 0.0;
  double dram_mb_s = 0.0;

  for (size_t bytes = 4096; bytes <= MAX_BYTES; bytes *= 2) {
    if (bytes < 1024 * 1024) {
      printf("%-4zu KB    ", bytes / 1024);
    } else {
      printf("%-4zu MB    ", bytes / (1024 * 1024));
    }

    for (size_t s = 0; s < num_strides; ++s) {
      double mb_s = measure_throughput(g_data, bytes, strides[s]);
      printf("%7.0f  ", mb_s);

      if (bytes == 16 * 1024 && strides[s] == 1) {
        l1_mb_s = mb_s;
      }
      if (bytes == MAX_BYTES && strides[s] == 1) {
        dram_mb_s = mb_s;
      }
    }
    printf("\n");
  }

  printf("\nSummary:\n");
  printf("  L1  Ridge Throughput (16 KB, Stride-1): %.0f MB/s\n", l1_mb_s);
  printf("  DRAM Plateau Throughput (64 MB, Stride-1): %.0f MB/s\n", dram_mb_s);

  assert(l1_mb_s > dram_mb_s);

  printf("\nReference Solution 10 PASSED!\n");
  return EXIT_SUCCESS;
}

