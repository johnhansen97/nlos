BITS 32
ORG 0x00100000

main:
	;; print message
	mov eax, 0x01
	mov ebx, message
	int 0x80

loop:	
	mov eax, 0x02
	mov ebx, char
	int 0x80

	mov eax, 0x01
	mov ebx, char
	int 0x80

	jmp loop

exit:	
	;; exit process
	mov eax, 0x00
	int 0x80

	jmp $

message:
	db 'Go ahead and type!', 10, 0
char:
	db 0
	db 0
