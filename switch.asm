global switch_to
switch_to:
	mov eax, [esp+4]
	mov ecx, [esp+8]

	push ebp
	push ebx
	push esi
	push edi

	mov [eax], esp
	mov esp, ecx

	pop edi
	pop esi
	pop ebx
	pop ebp

	ret
 
global switch_done
switch_done:
	;pop ds
	;pop es
	;pop fs
	;pop gs
	pop eax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	popa
	add esp, 0x8
  	iret
