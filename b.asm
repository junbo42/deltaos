
	global _start
	section .text
_start:
l:
    push 1
    push msg
    mov eax, 0
    int 0x80
    add esp, 8

    jmp l
	section .data
msg: db "prog2", 0
