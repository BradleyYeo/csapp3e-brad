#
# Student makefile for Cache Lab
# Note: requires a 64-bit x86-64 system
#
CC = gcc
CFLAGS = -g -Wall -Werror -std=c23 -m64

HANDIN_USER ?= $(if $(USER),$(USER),student)

all: csim test-trans tracegen
	# Generate a handin tar file each time you compile
	-tar -cvf $(HANDIN_USER)-handin.tar csim.c trans.c

csim: csim.c cachelab.c cachelab.h
	$(CC) $(CFLAGS) -o csim csim.c cachelab.c -lm

test-trans: test-trans.c trans.o cachelab.c cachelab.h
	$(CC) $(CFLAGS) -o test-trans test-trans.c cachelab.c trans.o

tracegen: tracegen.c trans.o cachelab.c
	$(CC) $(CFLAGS) -O0 -o tracegen tracegen.c trans.o cachelab.c

trans.o: trans.c
	$(CC) $(CFLAGS) -O0 -c trans.c

#
# Progressive csim tests: make test STAGE=<1..5>
#   1 structs/constants, 2 cache_init, 3 cache_destroy,
#   4 cache_access, 5 end-to-end traces
#
STAGE ?= 1
SANFLAGS ?= -fsanitize=address,undefined -fno-omit-frame-pointer

.PHONY: test test-unit test-memcheck
test: test-unit csim
	@chmod +x tests/run_traces.sh
	@if [ $(STAGE) -ge 5 ]; then ./tests/run_traces.sh; fi

test-unit:
	$(CC) $(CFLAGS) $(SANFLAGS) -DSTAGE=$(STAGE) -o tests/test_csim tests/test_csim.c cachelab.c
	./tests/test_csim

# Leak check (Linux/container only): make test-memcheck STAGE=4
test-memcheck:
	$(CC) $(CFLAGS) -DSTAGE=$(STAGE) -o tests/test_csim tests/test_csim.c cachelab.c
	valgrind --leak-check=full --error-exitcode=1 ./tests/test_csim

#
# Clean the src dirctory
#
clean:
	rm -rf *.o
	rm -f -- *.tar
	rm -f csim
	rm -f test-trans tracegen
	rm -f trace.all trace.f* trace.tmp
	rm -f .csim_results .marker
	rm -rf tests/test_csim tests/test_csim.dSYM
