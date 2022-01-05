
	.export disable_cursor_edit
	.export restore_cursor_edit

	.include "os.inc"
	.include "osbyte.inc"
	
	.bss
oldcc:	.res 1
	.code

disable_cursor_edit:

	; disable cursor editing
	lda	#osbyte_INTERPRETATION_ARROWS
	ldx	#1
	ldy	#0
	jsr	OSBYTE
	stx	oldcc
	rts

restore_cursor_edit:

	; restore cursor editing
	lda	#osbyte_INTERPRETATION_ARROWS
	ldx	oldcc
	ldy	#0
	jsr	OSBYTE
	rts