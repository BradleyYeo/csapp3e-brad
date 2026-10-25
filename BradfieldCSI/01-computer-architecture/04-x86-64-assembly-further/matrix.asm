section .text
global index

; -----------------------------------------------------------------------------
; int index(int *matrix, int rows, int cols, int rindex, int cindex);
;
; Returns the integer value at 2D coordinates (rindex, cindex) in a flattened
; row-major matrix array of integers (4 bytes per element).
;
; Calling Convention (System V AMD64 ABI):
; - %rdi: pointer to int matrix array base address
; - %rsi: int rows (number of rows)
; - %rdx: int cols (number of columns)
; - %rcx: int rindex (row index i)
; - %r8:  int cindex (column index j)
; - Return value: int in %rax (%eax)
;
; Memory Addressing Formula:
; For an array of 4-byte integers in row-major layout:
; Linear Offset = (rindex * cols + cindex)
; Byte Address  = base + (Linear Offset * 4)
;
; Hint:
; Use `imul` or polynomial `lea` instructions to compute effective address.
; -----------------------------------------------------------------------------
index:
    ; TODO: Calculate linear element index = (rindex * cols) + cindex
    ; TODO: Calculate byte address = matrix_base + (index * 4)
    ; TODO: Load 32-bit int from memory into %eax
    xor rax, rax
    ret
