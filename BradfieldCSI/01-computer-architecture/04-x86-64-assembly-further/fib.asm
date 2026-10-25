section .text
global fib

; -----------------------------------------------------------------------------
; int fib(int n);
;
; Computes the n-th Fibonacci number:
; fib(0) = 0
; fib(1) = 1
; fib(2) = 1
; fib(3) = 2
; fib(n) = fib(n-1) + fib(n-2)
;
; Calling Convention (System V AMD64 ABI):
; - Argument n: passed in %rdi (32-bit: %edi)
; - Return value: placed in %rax (32-bit: %eax)
;
; Implementation Strategies:
; - Iterative (O(n) time, O(1) space): Maintain two running registers (prev=0, curr=1)
;   and loop n times.
; - Recursive (demonstrates stack frame allocation and callee-saved preservation):
;   Must follow 16-byte stack alignment invariants before `call fib`.
; -----------------------------------------------------------------------------
fib:
    ; TODO: Handle base cases n = 0, n = 1
    ; TODO: Loop or recurse to calculate n-th Fibonacci number
    ; TODO: Return result in %rax
    xor rax, rax
    ret
