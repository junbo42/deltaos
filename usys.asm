
%macro SYSCALL 2
    global $%1
$%1:
	mov eax, %2
	int 0x80
	ret
%endmacro

SYSCALL write, 0
SYSCALL read, 1
SYSCALL fork, 2
SYSCALL wait, 3
SYSCALL execve, 4
SYSCALL _exit, 5
SYSCALL open, 6
SYSCALL getpid, 7
SYSCALL _sbrk, 8
SYSCALL close, 9
SYSCALL isatty, 10
SYSCALL fstat, 11
SYSCALL lseek, 12
SYSCALL kill, 13
SYSCALL stat, 14
SYSCALL chdir, 15
