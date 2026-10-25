section .text
global pangram

; -----------------------------------------------------------------------------
; int pangram(char *phrase);
;
; Determines if the input null-terminated ASCII string contains every letter
; of the English alphabet (a-z) at least once, case-insensitively.
;
; Returns:
; - 1 (true) if the phrase is a pangram
; - 0 (false) otherwise
;
; Calling Convention (System V AMD64 ABI):
; - Argument phrase: pointer passed in %rdi
; - Return value: 1 or 0 in %rax
;
; Bitmask Technique:
; - An English alphabet has 26 letters ('a' through 'z').
; - A 32-bit register (e.g. %edx) can serve as a bitset where bit k represents
;   whether the k-th letter has been seen.
; - Target mask: (1 << 26) - 1 = 0x03ffffff (bits 0 to 25 all set to 1).
; - For each character:
;     If 'A' <= c <= 'Z', normalize to lowercase (or 'a' <= c <= 'z').
;     Compute letter index: index = c - 'a' (0 <= index <= 25).
;     Set bit: mask |= (1 << index).
; - After parsing, compare mask == 0x03ffffff.
; -----------------------------------------------------------------------------
pangram:
    ; TODO: Initialize 32-bit bitmask to 0
    ; TODO: Iterate through characters at %rdi
    ; TODO: Filter letters, convert case, and set corresponding bit in bitmask
    ; TODO: Return 1 if all 26 bits set, else 0
    xor rax, rax
    ret
