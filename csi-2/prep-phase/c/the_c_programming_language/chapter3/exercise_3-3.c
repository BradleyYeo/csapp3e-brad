#include <stdio.h>

#define MAX_LENGTH 1000

void expand(char s1[], char s2[]) {
  int i, j, k;

  for (i = j = 0; s1[i] != '\0'; i++) {
    if (s1[i] == '-' && i > 0 && s1[i + 1] != '\0') {
      for (k = s1[i - 1] + 1; k < s1[i + 1]; k++) {
        s2[j++] = k;
      }
    } else if (!(s1[i] == '-' && (i == 0 || s1[i + 1] == '\0'))) {
      s2[j++] = s1[i];
    }
  }
  s2[j] = '\0';
}

int main() {
  char s1[] = "a-z0-9";
  char s2[MAX_LENGTH];

  expand(s1, s2);

  printf("%s\n", s2);

  return 0;
}
