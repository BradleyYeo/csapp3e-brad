CC ?= clang
CFLAGS ?= -std=c23 -Wall -Wextra -Werror -pedantic -g

TARGETS = 01_pointer_traversal 02_anchored_matching 03_getline_ingestion 04_enum_state_machine 05_char_group_slicing 06_mmap_search 07_process_memory_layout 08_pointer_arithmetic_strides 09_dynamic_memory_lifecycle 10_memory_safety_sanitizers 11_bump_allocator

CFLAGS_SAN ?= -std=c23 -Wall -Wextra -Werror -pedantic -g -fsanitize=address,undefined -fno-omit-frame-pointer

.PHONY: all test clean test-sanitizers

all: $(TARGETS)

01_pointer_traversal: 01_pointer_traversal.c
	$(CC) $(CFLAGS) -o $@ $<

02_anchored_matching: 02_anchored_matching.c
	$(CC) $(CFLAGS) -o $@ $<

03_getline_ingestion: 03_getline_ingestion.c
	$(CC) $(CFLAGS) -o $@ $<

04_enum_state_machine: 04_enum_state_machine.c
	$(CC) $(CFLAGS) -o $@ $<

05_char_group_slicing: 05_char_group_slicing.c
	$(CC) $(CFLAGS) -o $@ $<

06_mmap_search: 06_mmap_search.c
	$(CC) $(CFLAGS) -o $@ $<

07_process_memory_layout: 07_process_memory_layout.c
	$(CC) $(CFLAGS) -o $@ $<

08_pointer_arithmetic_strides: 08_pointer_arithmetic_strides.c
	$(CC) $(CFLAGS) -o $@ $<

09_dynamic_memory_lifecycle: 09_dynamic_memory_lifecycle.c
	$(CC) $(CFLAGS) -o $@ $<

10_memory_safety_sanitizers: 10_memory_safety_sanitizers.c
	$(CC) $(CFLAGS) -o $@ $<

11_bump_allocator: 11_bump_allocator.c
	$(CC) $(CFLAGS) -o $@ $<

test: all
	@echo "Running Exercise 01..."
	@./01_pointer_traversal
	@echo "Running Exercise 02..."
	@./02_anchored_matching
	@echo "Running Exercise 03..."
	@./03_getline_ingestion
	@echo "Running Exercise 04..."
	@./04_enum_state_machine
	@echo "Running Exercise 05..."
	@./05_char_group_slicing
	@echo "Running Exercise 06..."
	@./06_mmap_search
	@echo "Running Exercise 07..."
	@./07_process_memory_layout
	@echo "Running Exercise 08..."
	@./08_pointer_arithmetic_strides
	@echo "Running Exercise 09..."
	@./09_dynamic_memory_lifecycle
	@echo "Running Exercise 10..."
	@./10_memory_safety_sanitizers
	@echo "Running Exercise 11..."
	@./11_bump_allocator
	@echo "All tests passed successfully."

test-sanitizers: clean
	@echo "Compiling exercises with AddressSanitizer and UndefinedBehaviorSanitizer..."
	@$(MAKE) CFLAGS="$(CFLAGS_SAN)" all
	@echo "Running exercises under sanitizers..."
	@$(MAKE) test

clean:
	rm -rf $(TARGETS) *.dSYM
