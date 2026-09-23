#include <stdio.h>

int my_getline(char s[], int lim);

int main() {
  char s[100];
  int lim = 4;

  return my_getline(s, lim);
}

int my_getline(char s[], int lim) {
  int c, i;
  i = 0;

  while (--lim > 0 && (c = getchar()) != EOF && c != '\n') {
    s[i++] = c;
  }

  if (c == '\n') {
    s[i++] = c;
  }

  s[i] = '\0';
  return i;
}
