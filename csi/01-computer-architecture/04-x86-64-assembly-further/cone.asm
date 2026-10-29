default rel

section .text
global volume

; -----------------------------------------------------------------------------
; float volume(float radius, float height);
;
; Computes the volume of a right circular cone:
; V = (1/3) * pi * r^2 * h
;
; Calling Convention (System V AMD64 ABI - Floating Point):
; - Argument 1 (radius): passed in single-precision %xmm0
; - Argument 2 (height): passed in single-precision %xmm1
; - Return value: float placed in %xmm0
; - Float instructions: `mulss` (multiply scalar single), `divss` (divide scalar single), `movss`
;
; Constants:
; - pi: 3.14159
; - 3.0: constant divisor
; Use `default rel` for position-independent (RIP-relative) addressing in 64-bit mode.
; -----------------------------------------------------------------------------
volume:
    ; TODO: Load pi and 3.0 constants from .rodata or .data
    ; TODO: Compute r^2 * h * pi
    ; TODO: Divide by 3.0
    ; TODO: Return single-precision float in %xmm0
    ret

section .rodata
    ; TODO: Define constants (e.g., pi: dd 3.14159, const_3: dd 3.0)
