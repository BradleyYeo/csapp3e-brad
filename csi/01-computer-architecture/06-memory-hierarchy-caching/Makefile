CC ?= clang
CFLAGS = -std=c23 -Wall -Wextra -pedantic -g -O0
OPT_CFLAGS = -std=c23 -Wall -Wextra -pedantic -g -O2

# Default target
all: test

# ============================================================================
# Practice Targets (Your code stubs)
# ============================================================================

01: 01_address_decoding
	./01_address_decoding

01_address_decoding: 01_address_decoding.c cache_sim.h
	$(CC) $(CFLAGS) -o $@ 01_address_decoding.c -lm

03: 03_miss_prediction
	./03_miss_prediction

03_miss_prediction: 03_miss_prediction.c cache_sim.h
	$(CC) $(CFLAGS) -o $@ 03_miss_prediction.c -lm

04: 04_loop_order_measure
	./04_loop_order_measure

04_loop_order_measure: 04_loop_order_measure.c
	$(CC) -std=gnu17 -O0 -g -o $@ 04_loop_order_measure.c

05: 05_lru_cache_sim
	./05_lru_cache_sim

05_lru_cache_sim: 05_lru_cache_sim.c cache_sim.h
	$(CC) $(CFLAGS) -o $@ 05_lru_cache_sim.c -lm

06: 06_conflict_misses
	./06_conflict_misses

06_conflict_misses: 06_conflict_misses.c 05_lru_cache_sim.c cache_sim.h
	$(CC) $(CFLAGS) -DNO_MAIN -c 05_lru_cache_sim.c -o 05_lru_cache_sim_lib.o
	$(CC) $(CFLAGS) -o $@ 06_conflict_misses.c 05_lru_cache_sim_lib.o -lm
	@rm -f 05_lru_cache_sim_lib.o

07: 07_matmul_loop_orders
	./07_matmul_loop_orders

07_matmul_loop_orders: 07_matmul_loop_orders.c 05_lru_cache_sim.c cache_sim.h
	$(CC) $(CFLAGS) -DNO_MAIN -c 05_lru_cache_sim.c -o 05_lru_cache_sim_lib.o
	$(CC) $(CFLAGS) -o $@ 07_matmul_loop_orders.c 05_lru_cache_sim_lib.o -lm
	@rm -f 05_lru_cache_sim_lib.o

08:
	cd 08_matmul_benchmark && $(CC) -Wall -O2 -o matmul matrix-multiply.c benchmark.c -lm && ./matmul 1000

09: 09_tiling
	./09_tiling

09_tiling: 09_tiling.c 05_lru_cache_sim.c cache_sim.h
	$(CC) $(CFLAGS) -DNO_MAIN -c 05_lru_cache_sim.c -o 05_lru_cache_sim_lib.o
	$(CC) $(CFLAGS) -o $@ 09_tiling.c 05_lru_cache_sim_lib.o -lm
	@rm -f 05_lru_cache_sim_lib.o

10: 10_memory_mountain
	./10_memory_mountain

10_memory_mountain: 10_memory_mountain.c
	$(CC) $(OPT_CFLAGS) -o $@ 10_memory_mountain.c

# Run all practice exercises sequentially
test: 01_address_decoding 03_miss_prediction 05_lru_cache_sim 06_conflict_misses 07_matmul_loop_orders 09_tiling
	@echo "Running all practice unit tests..."
	./01_address_decoding
	./03_miss_prediction
	./05_lru_cache_sim
	./06_conflict_misses
	./07_matmul_loop_orders
	./09_tiling

# ============================================================================
# Reference Solution Targets
# ============================================================================

solution-01:
	$(CC) $(CFLAGS) -o reference-solution/01_address_decoding reference-solution/01_address_decoding.c -lm
	./reference-solution/01_address_decoding

solution-03:
	$(CC) $(CFLAGS) -o reference-solution/03_miss_prediction reference-solution/03_miss_prediction.c -lm
	./reference-solution/03_miss_prediction

solution-05:
	$(CC) $(CFLAGS) -o reference-solution/05_lru_cache_sim reference-solution/05_lru_cache_sim.c -lm
	./reference-solution/05_lru_cache_sim

solution-06:
	$(CC) $(CFLAGS) -DNO_MAIN -c reference-solution/05_lru_cache_sim.c -o reference-solution/05_lib.o
	$(CC) $(CFLAGS) -o reference-solution/06_conflict_misses reference-solution/06_conflict_misses.c reference-solution/05_lib.o -lm
	@rm -f reference-solution/05_lib.o
	./reference-solution/06_conflict_misses

solution-07:
	$(CC) $(CFLAGS) -DNO_MAIN -c reference-solution/05_lru_cache_sim.c -o reference-solution/05_lib.o
	$(CC) $(CFLAGS) -o reference-solution/07_matmul_loop_orders reference-solution/07_matmul_loop_orders.c reference-solution/05_lib.o -lm
	@rm -f reference-solution/05_lib.o
	./reference-solution/07_matmul_loop_orders

solution-08:
	cd 08_matmul_benchmark && $(CC) -Wall -O2 -o matmul_sol ../reference-solution/08_matrix-multiply.c benchmark.c -lm && ./matmul_sol 1000 && rm -f matmul_sol

solution-09:
	$(CC) $(CFLAGS) -DNO_MAIN -c reference-solution/05_lru_cache_sim.c -o reference-solution/05_lib.o
	$(CC) $(CFLAGS) -o reference-solution/09_tiling reference-solution/09_tiling.c reference-solution/05_lib.o -lm
	@rm -f reference-solution/05_lib.o
	./reference-solution/09_tiling

solution-10:
	$(CC) $(OPT_CFLAGS) -o reference-solution/10_memory_mountain reference-solution/10_memory_mountain.c
	./reference-solution/10_memory_mountain

test-solution: solution-01 solution-03 solution-05 solution-06 solution-07 solution-09 solution-10
	@echo "All reference solutions PASSED successfully!"

clean:
	rm -rf 01_address_decoding 01_address_decoding.dSYM \
	       03_miss_prediction 03_miss_prediction.dSYM \
	       04_loop_order_measure 04_loop_order_measure.dSYM \
	       05_lru_cache_sim 05_lru_cache_sim.dSYM \
	       06_conflict_misses 06_conflict_misses.dSYM \
	       07_matmul_loop_orders 07_matmul_loop_orders.dSYM \
	       09_tiling 09_tiling.dSYM \
	       10_memory_mountain 10_memory_mountain.dSYM \
	       08_matmul_benchmark/matmul 08_matmul_benchmark/matmul.dSYM \
	       reference-solution/01_address_decoding reference-solution/01_address_decoding.dSYM \
	       reference-solution/03_miss_prediction reference-solution/03_miss_prediction.dSYM \
	       reference-solution/05_lru_cache_sim reference-solution/05_lru_cache_sim.dSYM \
	       reference-solution/06_conflict_misses reference-solution/06_conflict_misses.dSYM \
	       reference-solution/07_matmul_loop_orders reference-solution/07_matmul_loop_orders.dSYM \
	       reference-solution/09_tiling reference-solution/09_tiling.dSYM \
	       reference-solution/10_memory_mountain reference-solution/10_memory_mountain.dSYM \
	       *.o reference-solution/*.o

.PHONY: all 01 03 04 05 06 07 08 09 10 test test-solution clean \
        solution-01 solution-03 solution-05 solution-06 solution-07 solution-08 solution-09 solution-10
