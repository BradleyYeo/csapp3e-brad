CC = clang

# ── One modern standard for the whole curriculum ────────────────────────────
CSTD = -std=c23
WARN = -Wall -Wextra -pedantic

# Exercises (incl. your in-progress stubs): warnings are errors — ship clean.
CFLAGS = $(CSTD) $(WARN) -Werror -g
# Repair targets keep deliberate legacy defects; warnings are the to-do list,
# so no -Werror there until you fix them.
CFLAGS_REPAIR = $(CSTD) $(WARN) -g
CFLAGS_SAN = $(CSTD) $(WARN) -Werror -g -fsanitize=address,undefined \
             -fno-omit-frame-pointer
CFLAGS_BENCH = $(CSTD) $(WARN) -O2

# 01/02 are complete and compile warning-free under C23 strict.
DONE = 01_hello_world 02_kr103_temp_f_to_c

# Stubbed exercises: spec comments + assert tests; implement until green.
EXERCISES = 03_kr105_temp_for_loop 04_kr106_eof_value 05_kr108_count_blanks \
            06_kr110_escape_sequences 07_kr112_one_word_per_line \
            08_kr114_char_histogram 09_kr119_reverse_line 10_kr120_detab \
            11_kr122_fold_lines 12_kr123_remove_comments 13_kr201_type_ranges \
            14_kr203_htoi 15_kr206_setbits 16_kr208_rightrot \
            18_kr303_expand_ranges 19_kr305_itob 20_pointer_traversal \
            21_anchored_matching 22_enum_state_machine 23_char_group_slicing \
            24_kr401_strrindex 26_kr504_strend 27_kr505_strn_family \
            28_kr501_getch_ungetch 30_pointer_arithmetic_strides \
            31_process_memory_layout 32_getline_ingestion \
            34_dynamic_memory_lifecycle 35_memory_safety_sanitizers \
            36_bump_allocator 37_valgrind_fundamentals 38_kr803_malloc_flushbuf \
            40_kr602_getword_groups 41_recursion_patterns 42_macro_fundamentals \
            43_kr706_compare_files 45_mmap_search 46_filesystem_inode_ops \
            47_pipe_pingpong 48_process_pipeline_exec

# Fix-the-existing-code targets (REMEDIATION.md). 17 and 50 deliberately do
# NOT compile — the syntax/TODO errors are the exercise. Build with `make repair`.
REPAIR = 17_kr301_binsearch_count 25_kr503_strcat 29_pointer_decay \
         33_kr507_sortlines 44_kr801_fcopy 50_pipelining_hazards

# Exercises whose runtime is dominated by your code — worth benchmarking.
BENCH_TARGETS = 32_getline_ingestion 36_bump_allocator 38_kr803_malloc_flushbuf \
                41_recursion_patterns 45_mmap_search 47_pipe_pingpong
REPS ?= 5

.PHONY: all exercises repair test run-exercises test-python test-sanitizers \
        valgrind-37 bench clean clean-obj

all: $(DONE) $(EXERCISES)

exercises: $(EXERCISES)

# 40 builds on the shared pushback getch (reference infrastructure, unchanged)
40_kr602_getword_groups: 40_kr602_getword_groups.c _resources/getch.c
	$(CC) $(CFLAGS) -I_resources -o $@ $< _resources/getch.c
# 0b literals are standard in C23 but clang <19 only accepts them as an
# extension even under -std=c23; they appear in the frozen test asserts.
42_macro_fundamentals: CFLAGS += -Wno-gnu-binary-literal
# usleep() is hidden by strict -std=c23 without this
48_process_pipeline_exec: CFLAGS += -D_DEFAULT_SOURCE

$(DONE) $(filter-out 40_kr602_getword_groups,$(EXERCISES)): %: %.c
	$(CC) $(CFLAGS) -o $@ $<

$(REPAIR): %: %.c
	$(CC) $(CFLAGS_REPAIR) -o $@ $<

repair: $(filter-out 17_kr301_binsearch_count 50_pipelining_hazards,$(REPAIR))

test: run-exercises test-python

# Run every exercise, report PASS/FAIL, wall time per exercise, and total.
# Continues past failures — the table doubles as your progress dashboard.
run-exercises: $(EXERCISES)
	@pass=0; fail=0; total=0; \
	for t in $(EXERCISES); do \
	  echo "== $$t"; \
	  t0=$$(date +%s%N); \
	  ./$$t; rc=$$?; \
	  t1=$$(date +%s%N); \
	  us=$$(( (t1 - t0) / 1000 )); \
	  total=$$(( total + us )); \
	  if [ $$rc -eq 0 ]; then \
	    printf -- "-- %-40s PASS  %10d us\n" "$$t" $$us; \
	    pass=$$((pass + 1)); \
	  else \
	    printf -- "-- %-40s FAIL  %10d us (exit %d)\n" "$$t" $$us $$rc; \
	    fail=$$((fail + 1)); \
	  fi; \
	done; \
	echo "--------------------------------------------------------------"; \
	printf 'summary: %d passed, %d failed, total runtime %d.%03d ms\n' \
	  $$pass $$fail $$((total / 1000000)) $$(( (total / 100) % 1000 )); \
	[ $$fail -eq 0 ]

test-python:
	@echo "== 49_hoare_logic_refactoring"; python3 49_hoare_logic_refactoring.py
	@echo "== 51_python_pointer_chasing"; python3 51_python_pointer_chasing.py --test

test-sanitizers: clean
	@$(MAKE) CFLAGS="$(CFLAGS_SAN)" run-exercises
	@$(MAKE) clean-obj

valgrind-37: 37_valgrind_fundamentals
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
		--error-exitcode=1 ./37_valgrind_fundamentals

# ── Benchmark lane: compare implementations ─────────────────────────────────
# Builds each timed exercise also at -O2 and reports best-of-$(REPS) wall time
# for -O0 (the `make` build) vs -O2, plus speedup. Exercises that don't pass
# their asserts are reported SKIP with no numbers. `make bench LOG=1` appends
# each row to bench-results.csv (timestamp + git rev) so you can compare your
# implementations across days and rewrites.
%.opt: %.c
	$(CC) $(CFLAGS_BENCH) -o $@ $<

bench: $(EXERCISES) $(addsuffix .opt,$(BENCH_TARGETS))
	@printf '%-36s %14s %14s %9s\n' exercise "-O0 best (us)" "-O2 best (us)" speedup; \
	rev=$$(git rev-parse --short HEAD 2>/dev/null || echo nogit); \
	if [ "$(LOG)" = "1" ]; then \
	  [ -f bench-results.csv ] || echo 'timestamp,git_rev,exercise,o0_us,o2_us' > bench-results.csv; \
	fi; \
	for t in $(BENCH_TARGETS); do \
	  if ! ./$$t >/dev/null 2>&1; then \
	    printf '%-36s %14s %14s %9s\n' $$t SKIP '—' '(tests red)'; \
	    continue; \
	  fi; \
	  b0=99999999999; b2=99999999999; i=0; \
	  while [ $$i -lt $(REPS) ]; do \
	    s=$$(date +%s%N); ./$$t >/dev/null 2>&1; e=$$(date +%s%N); \
	    u=$$(( (e - s) / 1000 )); [ $$u -lt $$b0 ] && b0=$$u; \
	    s=$$(date +%s%N); ./$$t.opt >/dev/null 2>&1; e=$$(date +%s%N); \
	    u=$$(( (e - s) / 1000 )); [ $$u -lt $$b2 ] && b2=$$u; \
	    i=$$((i + 1)); \
	  done; \
	  sp=$$([ $$b2 -gt 0 ] && echo $$(( 100 * b0 / b2 )) || echo 0); \
	  printf '%-36s %14d %14d %8d%%\n' $$t $$b0 $$b2 $$sp; \
	  if [ "$(LOG)" = "1" ]; then \
	    echo "$$(date -u +%Y-%m-%dT%H:%M:%SZ),$$rev,$$t,$$b0,$$b2" >> bench-results.csv; \
	  fi; \
	done

clean:
	rm -f $(DONE) $(EXERCISES) $(REPAIR) $(addsuffix .opt,$(BENCH_TARGETS)) \
	  a.out *.o .err-*
	rm -rf *.dSYM

clean-obj:
	rm -f *.o
