#include <stdio.h>

#define MAXLINE 1000 /* maximum input line length */

void reverse(char s[], int size);
void printReverse(char s[]);

int main() {
  char line[MAXLINE];
  int c;
  int i = 0;

  while ((c = getchar()) != EOF) {
    line[i] = c;
    if (c == '\n' && i < MAXLINE - 2) {
      line[i + 1] = '\0';
      reverse(line, i - 1);
      printReverse(line);
      i = 0;
    } else {
      ++i;
    }
  }
}

void printReverse(char s[]) {
  for (int i = 0; s[i] != '\0'; i++) {
    printf("%c", s[i]);
  }
}

void reverse(char s[], int size) {
  int low = 0;
  int high = size;

  while (low < high) {
    char temp = s[low];
    s[low] = s[high];
    s[high] = temp;

    ++low;
    --high;
  }
}
