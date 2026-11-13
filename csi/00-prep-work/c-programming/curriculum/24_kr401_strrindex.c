/* 	Write the function strrindex(s,t) , which returns the position of the
rightmost occurrence of t in s , or -1 if there is none. */

/* NOTE: striindex in the book is much more succinct - can be converted into this with 1 additional variable */

/* Here the function is pointer-based: it yields the pointer into s instead
of the index (index == found ? ptr - s : -1), and nullptr instead of -1. */

/* nullptr is a C23 keyword; define it for older dialects */
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
#ifndef nullptr
#define nullptr NULL
#endif
#endif

#include<stdio.h>
#include<assert.h>
#include<stdlib.h>

const char *strrindex(const char *s, const char *t);

int main() {
    const char *s;

    s = "abcabc";
    assert(strrindex(s, "abc") == s + 3);

    s = "hello";
    assert(strrindex(s, "l") == s + 3);
    assert(strrindex(s, "z") == nullptr);
    assert(strrindex(s, "hello") == s);

    s = "aaaa";
    assert(strrindex(s, "aa") == s + 2);

    s = "abababa";
    assert(strrindex(s, "aba") == s + 4);

    s = "abc";
    assert(strrindex(s, "abcd") == nullptr);

    s = "the cat in the hat";
    assert(strrindex(s, "the") == s + 11);

    s = "mississippi";
    assert(strrindex(s, "issi") == s + 4);

    s = "xabcx";
    assert(strrindex(s, "abc") == s + 1);

    printf("[PASS] Exercise 24: strrindex verified.\n");
    return EXIT_SUCCESS;
}

/*
 * strrindex(s, t): locates the rightmost occurrence of t inside s.
 * Input:  s - haystack, t - needle, both NUL-terminated.
 * Output: pointer to the first character of the last match in s,
 *         or nullptr when t does not occur in s. Overlapping matches
 *         count: needle "aa" in "aaaa" is found at s + 2.
 */
const char *strrindex(const char *s, const char *t) {
    (void)s;
    (void)t;
    return nullptr; /* TODO: implement */
}
