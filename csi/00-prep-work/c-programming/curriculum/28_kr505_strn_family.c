/* K&R 5-5: write your own versions of the <string.h> family strncpy,
 * strncat and strncmp, each bounded by a count n. The discipline: every
 * operation stops at the limit, and only the cat variant guarantees a
 * terminator.
 */
#include<stdio.h>
#include<assert.h>
#include<stdlib.h>
#include<string.h>

char *strnCpy(char *d, char *s, int n);
char *strnCat(char *d, char *s, int n);
int strnCmp(char *s, char *t, int n);

int main() {
    char d[16];
    char s[] = "ono";

    /* strnCpy: source ends before the limit - copied, terminated, padded */
    strcpy(d, "Heybaby");
    assert(strnCpy(d, s, 4) == d);
    assert(strcmp(d, "ono") == 0);

    strcpy(d, "Heybaby");
    assert(strnCpy(d, "ab", 5) == d);
    assert(memcmp(d, "ab\0\0\0", 5) == 0);

    /* strnCpy: limit reached first - truncated, NOT terminated */
    strcpy(d, "Heybaby");
    assert(strnCpy(d, "abcd", 2) == d);
    assert(memcmp(d, "ab", 2) == 0);
    assert(d[2] == 'y');

    /* strnCpy: exact fit consumes the n slots without a terminator */
    strcpy(d, "abcdef");
    assert(strnCpy(d, "xyz", 3) == d);
    assert(strcmp(d, "xyzdef") == 0);

    /* strnCpy: n == 0 copies nothing */
    strcpy(d, "Heybaby");
    assert(strnCpy(d, s, 0) == d);
    assert(strcmp(d, "Heybaby") == 0);

    /* strnCat: append within and beyond the limit, always terminated */
    strcpy(d, "Heybaby");
    assert(strnCat(d, "!!", 10) == d);
    assert(strcmp(d, "Heybaby!!") == 0);

    strcpy(d, "foo");
    assert(strnCat(d, "barbaz", 3) == d);
    assert(strcmp(d, "foobar") == 0);

    strcpy(d, "foo");
    assert(strnCat(d, "bar", 0) == d);
    assert(strcmp(d, "foo") == 0);

    /* strnCmp: compares at most n characters */
    assert(strnCmp("abc", "abc", 3) == 0);
    assert(strnCmp("abc", "abd", 3) < 0);
    assert(strnCmp("abd", "abc", 3) > 0);
    assert(strnCmp("abc", "abd", 2) == 0);
    assert(strnCmp("abc", "abcd", 3) == 0);
    assert(strnCmp("abc", "abcd", 4) < 0);
    assert(strnCmp("abc", "abd", 0) == 0);

    printf("[PASS] Exercise 27: strn family verified.\n");
    return EXIT_SUCCESS;
}

/*
 * strnCpy(d, s, n): strncpy analogue. Copies at most n characters from
 * s into d; if s ends first, pads d with '\0' up to the n-th character.
 * Like strncpy, the result is only guaranteed NUL-terminated when the
 * terminator of s was copied (strlen(s) < n).
 * Output: start of d. Example: strnCpy(d, "ono", 4) on "Heybaby" -> "ono"
 */
char *strnCpy(char *d, char *s, int n) {
    (void)s;
    (void)n;
    return d; /* TODO: implement */
}

/*
 * strnCat(d, s, n): strncat analogue. Appends at most n characters of s
 * to the end of d (after its current terminator) and always re-terminates
 * the result.
 * Output: start of d. Example: strnCat(d, "barbaz", 3) on "foo" -> "foobar"
 */
char *strnCat(char *d, char *s, int n) {
    (void)s;
    (void)n;
    return d; /* TODO: implement */
}

/*
 * strnCmp(s, t, n): strncmp analogue. Compares at most n characters,
 * stopping early at a difference or at the end of s.
 * Output: < 0, 0, > 0 as s sorts before, equal to, or after the prefix
 * of t. Example: strnCmp("abc", "abd", 2) == 0 but ...("abc","abd",3) < 0
 */
int strnCmp(char *s, char *t, int n) {
    (void)s;
    (void)t;
    (void)n;
    return 0; /* TODO: implement */
}
