#include <stdio.h>
#include <string.h>

int strend(char *s, char *t) {
  int length_s = strlen(s);
  int length_t = strlen(t);

  if (length_t > length_s) {
    return 0;
  }

  s += length_s - length_t;
  printf("%c", *s);

  while (*s++ == *t++) {
    if (*s == '\0') {
      return 1;
    }
  }

  return 0;
}
