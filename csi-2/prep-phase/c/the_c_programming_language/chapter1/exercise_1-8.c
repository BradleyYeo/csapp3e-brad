#include <stdio.h>

int main() {
  long blanks, tabs, newLines, c;

  blanks = 0;
  tabs = 0;
  newLines = 0;

  while ((c = getchar()) != EOF) {
    if (c == '\t')
      ++tabs;
    else if (c == ' ')
      ++blanks;
    else if (c == '\n')
      ++newLines;
  }

  printf("Number of tabs: %ld\n", tabs);
  printf("Number of blanks: %ld\n", blanks);
  printf("Number of new lines: %ld\n", newLines);
}
