CC ?= clang
CFLAGS = -std=c23 -Wall -Wextra -pedantic -g -O0
STAGE ?= 0

# Practice binary: complete the TODOs in cache_exercises.c
cache_exercises: cache_exercises.c
	$(CC) $(CFLAGS) -o $@ $< -lm

# Run all stages, or one stage: make test STAGE=3
test: cache_exercises
	./cache_exercises $(STAGE)

# Verify the reference solution (do not read before attempting each stage)
test-solution: reference-solution/cache_exercises_solution.c
	$(CC) $(CFLAGS) -o reference-solution/cache_exercises_solution $< -lm
	./reference-solution/cache_exercises_solution $(STAGE)

# Part 2 benchmarks (cachegrind needs Linux/Valgrind)
loop-order: loop-order.c
	$(CC) -std=gnu17 -O0 -g -o $@ $<

matmul: matrix-multiply.c benchmark.c
	$(CC) -std=gnu17 -Wall -O2 -o $@ matrix-multiply.c benchmark.c

clean:
	rm -rf cache_exercises cache_exercises.dSYM loop-order loop-order.dSYM matmul matmul.dSYM \
	  reference-solution/cache_exercises_solution reference-solution/cache_exercises_solution.dSYM

.PHONY: test test-solution clean

