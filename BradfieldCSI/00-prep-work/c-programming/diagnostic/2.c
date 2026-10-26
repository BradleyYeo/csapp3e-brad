#include <stdio.h>

char *create_greeting(const char *name) {
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "Hello, %s!", name);
  return buffer;
}

/*
Explain what happens to the memory backing buffer after create_greeting returns,
and why dereferencing the return value in the caller produces undefined
behavior. Contrast the caller-allocates pattern (passing a destination buffer
and capacity) versus the callee-allocates pattern (malloc). Explain the
maintainability and resource-ownership tradeoffs between caller-allocates and
callee-allocates in C systems programming.
*/