BITS 32
ORG 0x00100000

main:
	;; print message
	mov eax, 0x01
	mov ebx, message
	int 0x80

	;; exit process
	mov eax, 0x00
	int 0x80

	jmp $

message:
	db "Hello World!"
