CC = clang

# Strict: modern C23 drills (must compile warning-free)
CFLAGS_STRICT = -std=c23 -Wall -Wextra -Werror -pedantic -g
CFLAGS_STRICT_SAN = -std=c23 -Wall -Wextra -Werror -pedantic -g \
                    -fsanitize=address,undefined -fno-omit-frame-pointer
# Lax: historical K&R drafts — old idioms, warnings allowed, repair targets
CFLAGS_KR = -std=gnu17 -Wno-implicit-int -Wno-builtin-declaration-mismatch -g

DRILLS = 20_pointer_traversal 21_anchored_matching 22_enum_state_machine \
         23_char_group_slicing 30_pointer_arithmetic_strides \
         31_process_memory_layout 32_getline_ingestion \
         34_dynamic_memory_lifecycle 35_memory_safety_sanitizers \
         36_bump_allocator 37_valgrind_fundamentals 41_recursion_patterns \
         42_macro_fundamentals 45_mmap_search 46_filesystem_inode_ops \
         47_pipe_pingpong 48_process_pipeline_exec 50_pipelining_hazards

# 17_kr301 excluded on purpose: it has a syntax error — fixing it IS the exercise.
KR = 01_hello_world 02_kr103_temp_f_to_c 03_kr105_temp_for_loop \
     04_kr106_eof_value 05_kr108_count_blanks 06_kr110_escape_sequences \
     07_kr112_one_word_per_line 08_kr114_char_histogram 09_kr119_reverse_line \
     10_kr120_detab 11_kr122_fold_lines 12_kr123_remove_comments \
     13_kr201_type_ranges 14_kr203_htoi 15_kr206_setbits 16_kr208_rightrot \
     18_kr303_expand_ranges 19_kr305_itob 24_kr401_strrindex 25_kr503_strcat \
     26_kr504_strend 27_kr505_strn_family 29_pointer_decay 33_kr507_sortlines \
     38_kr803_malloc_flushbuf 43_kr706_compare_files 44_kr801_fcopy

KR_LINKED = 28_kr501_getch_ungetch 40_kr602_getword_groups

.PHONY: all drills kr test run-drills test-python test-sanitizers valgrind-37 clean

all: drills kr

drills: $(DRILLS)
kr: $(KR) $(KR_LINKED)

$(DRILLS): %: %.c
	$(CC) $(CFLAGS_STRICT) -o $@ $<

$(KR): %: %.c
	$(CC) $(CFLAGS_KR) -o $@ $<

$(KR_LINKED): %: %.c _resources/getch.c
	$(CC) $(CFLAGS_KR) -I_resources -o $@ $< _resources/getch.c

test: run-drills test-python

run-drills: drills
	@for t in $(DRILLS); do echo "== $$t"; ./$$t || exit 1; done
	@echo "All drill tests passed."

test-python:
	@echo "== 49_hoare_logic_refactoring"; python3 49_hoare_logic_refactoring.py
	@echo "== 51_python_pointer_chasing"; python3 51_python_pointer_chasing.py --test

test-sanitizers: clean
	@$(MAKE) CFLAGS_STRICT="$(CFLAGS_STRICT_SAN)" run-drills
	@$(MAKE) clean-obj

valgrind-37: 37_valgrind_fundamentals
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
		--error-exitcode=1 ./37_valgrind_fundamentals

clean:
	rm -f $(DRILLS) $(KR) $(KR_LINKED) a.out *.o
	rm -rf *.dSYM

clean-obj:
	rm -f *.o
