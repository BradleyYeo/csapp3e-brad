section .text
global sum_to_n

; -----------------------------------------------------------------------------
; int sum_to_n(int n);
;
; Computes the sum of integers from 1 through n:
; sum = 1 + 2 + ... + n
;
; Calling Convention (System V AMD64 ABI):
; - Argument n: passed in %rdi (32-bit: %edi)
; - Return value: placed in %rax (32-bit: %eax)
; - Caller-saved registers: %rax, %rcx, %rdx, %rsi, %rdi, %r8-%r11
; - Callee-saved registers: %rbx, %rbp, %r12-%r15 (must restore if modified)
;
; Constraints:
; - If n <= 0, return 0.
; - Handle positive values of n up to 1000.
; -----------------------------------------------------------------------------
sum_to_n:
    ; TODO: Initialize accumulator (%rax) to 0
    ; TODO: Implement loop adding n (%rdi) to %rax, decrementing n until 0
    ; TODO: Return with result in %rax
    xor rax, rax
    ret
