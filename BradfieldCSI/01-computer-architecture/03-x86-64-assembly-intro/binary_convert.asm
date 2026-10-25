section .text
global binary_convert

; -----------------------------------------------------------------------------
; int binary_convert(char *bits);
;
; Parses a null-terminated ASCII string containing '0' and '1' characters
; and converts it into its equivalent unsigned integer value.
;
; Example:
; "110"      -> 6
; "1111"     -> 15
; "10101101" -> 173
;
; Calling Convention (System V AMD64 ABI):
; - Argument bits: pointer passed in %rdi
; - Return value: integer placed in %rax
;
; Algorithm Outline:
; 1. Initialize accumulator %rax = 0
; 2. Loop through each character at (%rdi):
;    a. Read byte: movzx edx, byte [rdi]
;    b. If byte is 0 (null terminator), break loop
;    c. Left shift accumulator: shl rax, 1 (multiply by 2)
;    d. If character is '1' (ASCII 0x31), add 1 to %rax
;    e. Advance pointer: inc rdi
; 3. Return %rax
; -----------------------------------------------------------------------------
binary_convert:
    ; TODO: Initialize result register to 0
    ; TODO: Iterate over characters in string at %rdi
    ; TODO: Shift and accumulate bit values
    xor rax, rax
    ret
