///*
///* strcat:  concatenate t to end of s; s must be big enough */
//   void strcat(char s[], char t[])
//   {
//
// int i, j;
//       i = j = 0;
//       while (s[i] != '\0') /* find end of s */
//           i++;
//       while ((s[i++] = t[j++]) != '\0') /* copy t */
//; }

#include <stdio.h>
#include <stdlib.h>

void strcatcopy(char *s, char *t);
int getstrlen(char *s);

int main() {
  char *t = "HELLO!";
  char *s = "goes at the end -> ";

  strcatcopy(s, t);
}

void strcatcopy(char *s, char *t) {
  char *result = malloc(getstrlen(s) + getstrlen(t) + 1);

  if (result == NULL) {
    printf("Memory allocation failed.\n");
    return;
  }

  char *dst = result;

  while ((*dst++ = *s++))
    ;

  dst--;

  while ((*dst++ = *t++))
    ;

  printf("%s\n", result);

  free(result);
}

int getstrlen(char *s) {
  int count;

  while ((*s++))
    count++;

  return count;
}
