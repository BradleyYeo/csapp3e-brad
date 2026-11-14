/* Write the function itob(n,s,b) that converts the integer n
into a base b character representation in the string s .
In particular, itob(n,s,16) formats n as a hexadecimal integer in s

Note: Negative numbers return their two's complement hex representations
*/

#include<stdio.h>
#include<assert.h>
#include<stdlib.h>
#include<string.h>

void itob(unsigned n, char s[], int b);
char hexMapping []= {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

int main() {
    char s[33];

    itob(12234, s, 16);
    assert(strcmp(s, "2FCA") == 0);

    itob(255, s, 16);
    assert(strcmp(s, "FF") == 0);

    itob(0, s, 16);
    assert(strcmp(s, "0") == 0);

    itob(0, s, 2);
    assert(strcmp(s, "0") == 0);

    itob(10, s, 2);
    assert(strcmp(s, "1010") == 0);

    itob(255, s, 8);
    assert(strcmp(s, "377") == 0);

    itob(12234, s, 12);
    assert(strcmp(s, "70B6") == 0);

    itob(123456789, s, 10);
    assert(strcmp(s, "123456789") == 0);

    itob((unsigned)-1, s, 16);
    assert(strcmp(s, "FFFFFFFF") == 0);

    itob((unsigned)-1, s, 10);
    assert(strcmp(s, "4294967295") == 0);

    printf("[PASS] Exercise 19: itob verified.\n");
    return EXIT_SUCCESS;
}

/*
 * itob(n, s, b): writes n in base b (2..16) into s, most significant
 * digit first, using hexMapping for the digit characters (so bases
 * above 10 get uppercase letters, e.g. itob(12234, s, 12) -> "70B6").
 * n is unsigned: pass (unsigned)x to see a negative x in its
 * two's-complement representation. s needs room for 32 digits plus
 * '\0' - 33 bytes covers every base >= 2. n == 0 writes "0".
 */
void itob(unsigned n, char s[], int b) {
    (void)n;
    (void)b;
    s[0] = '\0'; /* TODO: implement */
}
