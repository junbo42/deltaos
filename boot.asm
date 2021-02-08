[bits 32]

KERNEL_PAGE_NUMBER equ (0xc0000000 >> 22)

section .data
align 0x1000
pgdir:
    dd 0x00000083
    times (KERNEL_PAGE_NUMBER - 1) dd 0
    dd 0x00000083
    dd 0x00400083
    dd 0x00800083
    dd 0x00c00083
    dd 0x01000083
    times (1024 - KERNEL_PAGE_NUMBER - 5) dd 0

MULTIBOOT_ALIGN equ 1<<0
MULTIBOOT_MEMINFO equ 1<<1

magic equ 0x1badb002
flags equ MULTIBOOT_ALIGN|MULTIBOOT_MEMINFO

    section .text
align 4
dd magic
dd flags
dd - (magic + flags)

global start
start:
    mov ecx, (pgdir - 0xC0000000)
    mov cr3, ecx

    mov ecx, cr4
    or ecx, 0x00000010
    mov cr4, ecx

    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx

    mov ecx, _startmain
    jmp ecx

extern kmain
_startmain:
    cli
    mov dword [pgdir], 0
    invlpg [0]

    mov esp, stack_space-0x10
    add ebx, 0xc0000000
    push ebx
    call kmain
    ret

global load_idt
load_idt:
	mov edx, [esp + 4]
	lidt [edx]
	;sti 				;turn on interrupts
	ret

global load_gdt
load_gdt:
    mov edx, [esp + 4]
    lgdt [edx]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:load_gdt_done

load_gdt_done:
    ret

global load_cr3
load_cr3:
    mov edx, [esp + 4]
    mov cr3, edx
    ret

global load_tss
load_tss:
   mov ax, 0x28
   ltr ax
   ret

%macro CREATE_IRQ 1
    global irq_%1
irq_%1:
    ;cli
    push byte 0
    push byte %1
    jmp irq_s1
%endmacro

CREATE_IRQ 0
CREATE_IRQ 1
CREATE_IRQ 2
CREATE_IRQ 3
CREATE_IRQ 4
CREATE_IRQ 5
CREATE_IRQ 6
CREATE_IRQ 7
CREATE_IRQ 8
CREATE_IRQ 9
CREATE_IRQ 10
CREATE_IRQ 11
CREATE_IRQ 12
CREATE_IRQ 13
CREATE_IRQ 14
CREATE_IRQ 15

extern irq_s2
irq_s1:
   pusha                    ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax

   mov ax, ds               ; Lower 16-bits of eax = ds.
   push eax                 ; save the data segment descriptor

   mov ax, 0x10  ; load the kernel data segment descriptor
   mov ds, ax
   mov es, ax
   mov fs, ax
   mov gs, ax

   call irq_s2

   pop eax        ; reload the original data segment descriptor
   mov ds, ax
   mov es, ax
   mov fs, ax
   mov gs, ax

   popa                     ; Pops edi,esi,ebp...
   add esp, 8     ; Cleans up the pushed error code and pushed ISR number
   ;sti	; no need?, iret will restore eflags from stack
   iret           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP

%macro TRAP_IRQ 1
    global irq_%1
    extern do_%1
irq_%1:
    push 0
    pusha                    ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax

    mov ax, ds               ; Lower 16-bits of eax = ds.
    push eax                 ; save the data segment descriptor

    mov ax, 0x10  ; load the kernel data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call do_%1

    pop eax        ; reload the original data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                     ; Pops edi,esi,ebp...
    add esp, 8
    iret           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP
%endmacro

TRAP_IRQ division
TRAP_IRQ protectionfault
TRAP_IRQ pagefault

global irq_syscall
extern do_syscall
irq_syscall:
    push 0
    push 0x80
    pusha                    ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax

    mov ax, ds               ; Lower 16-bits of eax = ds.
    push eax                 ; save the data segment descriptor

    mov ax, 0x10  ; load the kernel data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call do_syscall

    pop eax        ; reload the original data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                     ; Pops edi,esi,ebp...
    add esp, 8
    iret           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP

global trapret
trapret:
  	popa
  	pop gs
  	pop fs
  	pop es
  	pop ds
  	add esp, 8  ; trapno and errcode

  	iret

    section .bss
align 32
resb 0x1000
global stack_space
stack_space:
