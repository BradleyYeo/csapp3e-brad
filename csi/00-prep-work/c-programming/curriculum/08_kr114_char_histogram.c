#include <stdio.h>

int main() {
  int charFrequencies[128];
  int c;

  for (int i = 0; i < 128; i++) {
    charFrequencies[i] = 0;
  }

  while ((c = getchar()) != EOF) {
     ++charFrequencies[c];
  }

  // Print the horizontal bars based on char frequencies
  for (int i = 0; i < 128; i++) {
    printf("%c: ", i);
    for (int j = 0; j < charFrequencies[i]; j++) {
        printf("*");
    }
    printf("\n");
  }
}
