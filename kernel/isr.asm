extern isr0, scan_queue, enqueue, kernel_panic_pf

section .text

global isr20_wrapper
isr20_wrapper:
	cld
	push eax
	call isr0
	
	mov al, 0x20
	out 0x20, al		;send EOI
	
	pop eax
	iretd

global isr21_wrapper
isr21_wrapper:
	push eax
	xor eax, eax

	in al, 0x60
	push eax
	push scan_queue

	call enqueue

	mov al, 0x20
	out 0x20, al
	
	add esp, 8
	pop eax
	iretd

global isr0E
isr0E:	
	push ecx
	mov ecx, cr2
	push ecx
	call kernel_panic_pf
