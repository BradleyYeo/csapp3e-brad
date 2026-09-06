#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Memory Segment Classification Enum
 * Avoids magic numbers when identifying virtual memory regions.
 */
typedef enum {
  SEG_UNKNOWN = 0,
  SEG_TEXT,
  SEG_DATA,
  SEG_BSS,
  SEG_HEAP,
  SEG_STACK
} SegmentType;

/*
 * Memory Region Boundaries captured at runtime.
 */
typedef struct {
  uintptr_t text_sample;
  uintptr_t data_sample;
  uintptr_t bss_sample;
  uintptr_t heap_sample;
  uintptr_t stack_sample;
} MemoryBounds;

/*
 * Static and Global Variable Declarations
 * - Initialized globals reside in the .data segment.
 * - Uninitialized globals reside in the .bss segment (zero-filled by OS).
 */
static int g_initialized_data = 100;
static int g_uninitialized_bss;

/*
 * Target function used to sample .text segment address.
 */
static void dummy_function(void) {
  // Empty function body for text segment address sampling.
}

/*
 * Helper for stack frame address inspection.
 * Captures local variable address in nested call frame via out-pointer.
 */
static void sample_nested_stack(uintptr_t *out_addr) {
  int nested_local = 0;
  *out_addr = (uintptr_t)&nested_local;
}

/*
 * Drill 1: Memory Segment Classifier
 * Implement:
 *   SegmentType classify_segment(uintptr_t addr, const MemoryBounds *bounds)
 *
 * Requirements:
 * - Return SEG_UNKNOWN if addr == 0 or bounds is nullptr.
 * - Classify addr by comparing relative ranges between sampled segment addresses.
 * - Standard Unix virtual address space ordering:
 *     text <= addr < data  -> SEG_TEXT
 *     data <= addr < bss   -> SEG_DATA
 *     bss  <= addr < heap  -> SEG_BSS
 *     heap <= addr < stack -> SEG_HEAP
 *     addr >= stack        -> SEG_STACK
 */
/* TODO: Implement classify_segment */
static SegmentType classify_segment(uintptr_t addr, const MemoryBounds *bounds) {
  if (addr == 0 || bounds == nullptr) {
    return SEG_UNKNOWN;
  }

  // Type your implementation here.
  return SEG_UNKNOWN;
}

/*
 * Drill 2: Stack Growth Direction & Stride
 * Implement:
 *   ptrdiff_t calculate_stack_growth_offset(uintptr_t parent_frame)
 *
 * Requirements:
 * - Declare a local variable in this function frame.
 * - Compute difference: (ptrdiff_t)(parent_frame - (uintptr_t)&child_local).
 * - On architectures where stack grows downward (x86_64 and ARM64):
 *     parent_frame > child_local, returning a positive offset.
 */
/* TODO: Implement calculate_stack_growth_offset */
static ptrdiff_t calculate_stack_growth_offset(uintptr_t parent_frame) {
  if (parent_frame == 0) {
    return 0;
  }

  // Type your implementation here.
  return 0;
}

int main(void) {
  int local_stack_var = 1;
  int *heap_var = malloc(sizeof(int));
  assert(heap_var != nullptr);
  *heap_var = 50;

  MemoryBounds bounds = {
    .text_sample = (uintptr_t)&dummy_function,
    .data_sample = (uintptr_t)&g_initialized_data,
    .bss_sample = (uintptr_t)&g_uninitialized_bss,
    .heap_sample = (uintptr_t)heap_var,
    .stack_sample = (uintptr_t)&local_stack_var
  };

  /*
   * Segment Ordering Invariants Verification:
   * Virtual address spaces arrange segments with relative order:
   * &text < &data < &bss < heap < stack
   */
  assert(bounds.text_sample < bounds.data_sample);
  assert(bounds.data_sample < bounds.bss_sample);
  assert(bounds.bss_sample < bounds.heap_sample);
  assert(bounds.heap_sample < bounds.stack_sample);

  /*
   * Stack Downward Growth Verification:
   * Nested call frame allocates locals at strictly lower address.
   */
  uintptr_t nested_stack = 0;
  sample_nested_stack(&nested_stack);
  assert(bounds.stack_sample > nested_stack);

  /*
   * Drill 2 Verification:
   * Positive offset confirms stack grows downwards.
   */
  ptrdiff_t stack_growth = calculate_stack_growth_offset(bounds.stack_sample);
  assert(stack_growth > 0);

  /*
   * Drill 1 Verification:
   * Segment classification tests.
   */
  assert(classify_segment((uintptr_t)&dummy_function, &bounds) == SEG_TEXT);
  assert(classify_segment((uintptr_t)&g_initialized_data, &bounds) == SEG_DATA);
  assert(classify_segment((uintptr_t)&g_uninitialized_bss, &bounds) == SEG_BSS);
  assert(classify_segment((uintptr_t)heap_var, &bounds) == SEG_HEAP);
  assert(classify_segment((uintptr_t)&local_stack_var, &bounds) == SEG_STACK);
  assert(classify_segment(0, &bounds) == SEG_UNKNOWN);
  assert(classify_segment((uintptr_t)&dummy_function, nullptr) == SEG_UNKNOWN);

  free(heap_var);
  heap_var = nullptr;

  printf("[PASS] Exercise 07: Process Memory Layout verified.\n");
  return EXIT_SUCCESS;
}
