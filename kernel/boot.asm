extern kernel_main, idt, multiboot_info
	
extern upper_half
extern kernel_physical_start
extern kernel_virtual_start
extern kernel_physical_end
extern kernel_virtual_end
	
extern _upper_half
extern _kernel_physical_start
extern _kernel_virtual_start
extern _kernel_physical_end
extern _kernel_virtual_end
global _loader, loader
	;constants
ALN		equ	1<<0
MEMINFO		equ	1<<1
FLAGS		equ	ALN | MEMINFO
MAGIC		equ	0x1BADB002
CHECKSUM	equ	-(MAGIC + FLAGS)

	;multiboot header
section .multiboot
align 4
dd MAGIC
dd FLAGS
dd CHECKSUM

section .bss
align 16
stack_bottom:
resb 0x00010000		;64kb
stack_top:

align 16
tss_pl0_bottom:
resb 0x00010000		;64kb
tss_pl0_top:	

KERNEL_VIRTUAL_BASE equ 0xC0000000
KERNEL_PAGE_NUMBER equ (KERNEL_VIRTUAL_BASE >> 22)

	
section .data

align 0x1000
global boot_page_directory
boot_page_directory:
	;; identity map first 4mb as rw
	dd 0x00000083
	times (KERNEL_PAGE_NUMBER - 1) dd 0 ;empty pages before kernel
	;; map 0xC0000000 to first 8mb
	dd 0x00000183
	dd 0x00400183
	times (1024 - KERNEL_PAGE_NUMBER - 2) dd 0 ;empty pages after kernel
	
idt_desc:
	dw 0x07ff
	dd idt

align 0x10
global_descriptor_table:
	;; first entry is null
	times 8 db 0

	;; second entry is tss, initialized at runtime
tss_descriptor:	
	times 8 db 0
	
	;; kernel code segment
	dw 0xFFFF		;lower limit
	dw 0x0000		;lower base
	db 0x00			;mid base
	db 0x9A			;access byte
	db 0xCF			;flags and upper limit
	db 0x00			;upper base

	;; kernel data segment
	dw 0xFFFF		;lower limit
	dw 0x0000		;lower base
	db 0x00			;mid base
	db 0x92			;access byte
	db 0xCF			;flags and upper limit
	db 0x00			;upper base

	;; user code segment
	dw 0xFFFF		;lower limit
	dw 0x0000		;lower base
	db 0x00			;mid base
	db 0xFA			;access byte
	db 0xCF			;flags and upper limit
	db 0x00			;upper base

	;; user data segment
	dw 0xFFFF		;lower limit
	dw 0x0000		;lower base
	db 0x00			;mid base
	db 0xF2			;access byte
	db 0xCF			;flags and upper limit
	db 0x00			;upper base

gdt_desc:
	dw 0x002F
	dd global_descriptor_table

align 0x1000
task_state_segment:
	dd 0			;prev. task link
	dd tss_pl0_top		;esp0
	dd 0x18			;ss0
	times 22 dd 0		;zero remaining fields

	

section .text

loader equ _loader - KERNEL_VIRTUAL_BASE

_loader:
	;; load page directory
	mov ecx, (boot_page_directory - KERNEL_VIRTUAL_BASE)
	mov cr3, ecx

	;; enable 4mb pages
	mov ecx, cr4
	or ecx, 0x00000090
	mov cr4, ecx

	;; enable paging
	mov ecx, cr0
	or ecx, 0x80000000
	mov cr0, ecx

	;; install tss descriptor
	mov ecx, tss_descriptor
	mov edx, task_state_segment
	mov word [ecx], 0x67	;seg limit
	mov [ecx+2], dx		;base lower
	shr edx, 16
	mov [ecx+4], dl		;base mid
	mov byte [ecx+5], 0x89	;type
	mov byte [ecx+6], 0x00	;upper limit
	mov byte [ecx+7], dh	;upper base

	lgdt [gdt_desc]
	mov cx, 0x08
	ltr cx
	
	;; long jump to the higher half
	lea ecx, [start_higher]
	jmp ecx

start_higher:
	;; delete identity map for first 4mb
	mov dword [boot_page_directory], 0
	invlpg [0]
	
	mov esp, stack_top
	mov ebp, esp

	;; load global variables
	mov ecx, multiboot_info
	add ebx, _upper_half
	mov [ecx], ebx

	mov eax, upper_half
	mov dword [eax], _upper_half

	mov eax, kernel_virtual_start
	mov dword [eax], _kernel_virtual_start

	mov eax, kernel_physical_start
	mov dword [eax], _kernel_physical_start

	mov eax, kernel_virtual_end
	mov dword [eax], _kernel_virtual_end

	mov eax, kernel_physical_end
	mov dword [eax], _kernel_physical_end
	
	call kernel_main
	
halt:
	hlt
	jmp halt

global load_idt
load_idt:
	
	lidt[idt_desc]
	sti
	ret

;; size _start, . - _start
