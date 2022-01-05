;
; Dominic Beesley 14.04.2005
;
; void __fastcall__ gotoxy (unsigned char x, unsigned char y);
;

	.include "os.inc"

	.export		_gotoxy, gotoxy
	.import		popa

gotoxy:
	jsr	popa		; Get Y

_gotoxy:			; Set the cursor position
	pha
	lda	#31
	jsr	OSWRCH
	jsr 	popa		; Get X
	jsr	OSWRCH
	pla
	jmp	OSWRCH
