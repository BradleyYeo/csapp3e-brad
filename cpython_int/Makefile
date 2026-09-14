CC ?= clang
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c11 -O2
SAN_FLAGS = -fsanitize=address,undefined -fno-omit-frame-pointer -g

SRC = cpython_int.c test_cpython_int.c
HDR = cpython_int.h test_framework.h

.PHONY: all test test_sanitizer clean

all: test

test_runner: $(SRC) $(HDR)
	$(CC) $(CFLAGS) $(SRC) -o test_runner

test_runner_san: $(SRC) $(HDR)
	$(CC) $(CFLAGS) $(SAN_FLAGS) $(SRC) -o test_runner_san

test: test_runner
	./test_runner

test_sanitizer: test_runner_san
	./test_runner_san

clean:
	rm -f test_runner test_runner_san
