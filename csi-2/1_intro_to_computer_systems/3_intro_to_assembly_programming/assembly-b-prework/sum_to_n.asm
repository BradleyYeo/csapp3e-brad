global sum_to_n

xor RAX, RAX
xor RCX, RCX

section .text
start_loop:
	cmp RCX, RDI
	jg end_loop

	add RAX, RCX
	inc RCX
	jmp start_loop

end_loop:
	mov rax, 0x02000001         ; system call for exit
    xor rdi, rdi                ; exit code 0
    syscall                           ; invoke operating system to exit
