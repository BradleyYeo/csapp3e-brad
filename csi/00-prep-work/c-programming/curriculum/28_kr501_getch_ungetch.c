/* K&R 5-1: As written, getint treats a + or - not followed by a digit as a
 * valid representation of zero. Fix it to push such a character back on the
 * input.
 */
#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

constexpr size_t BUFSIZE = 100;

static const char *test_src = nullptr;
static size_t test_pos = 0;
static int pushback_buf[BUFSIZE];
static size_t pushback_sp = 0;

static void reset_input(const char *s) {
  test_src = s;
  test_pos = 0;
  pushback_sp = 0;
}

int my_getch(void) {
  if (pushback_sp > 0) {
    return pushback_buf[--pushback_sp];
  }
  if (test_src == nullptr || test_src[test_pos] == '\0') {
    return EOF;
  }
  return (unsigned char)test_src[test_pos++];
}

void my_ungetch(int c) {
  (void)c;
  /* TODO: implement pushback into pushback_buf */
}

/*
 * getint: gets next integer from input into *pn.
 * Returns EOF for end of file, 0 if the next input is not a number
 * (pushing back any non-digit / lone '+' or '-' onto the input),
 * and a positive value if the input contains a valid number.
 */
int getint(int *pn) {
  (void)pn;
  /* TODO: implement using my_getch / my_ungetch */
  return 0;
}

int main(void) {
  int val = 0;

  reset_input("  42  -17 +99");
  assert(getint(&val) > 0 && val == 42);
  assert(getint(&val) > 0 && val == -17);
  assert(getint(&val) > 0 && val == 99);
  assert(getint(&val) == EOF);

  /* Lone '+' must return 0 and leave '+' (or not consume past it) */
  reset_input("+abc");
  assert(getint(&val) == 0);
  assert(my_getch() == '+');

  /* Non-number returns 0 and pushes back the character */
  reset_input("xyz");
  assert(getint(&val) == 0);
  assert(my_getch() == 'x');

  printf("[PASS] Exercise 28: getint & getch/ungetch verified.\n");
  return EXIT_SUCCESS;
}
