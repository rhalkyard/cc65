;
; Serial driver for the BBC Model B / Master using the built-in RS423 interface
;

        .include        "zeropage.inc"
        .include        "ser-kernel.inc"
        .include        "ser-error.inc"

        .macpack        module

OSBYTE := $FFF4
OSRDCH := $FFE0
OSWRCH := $FFEE

; ------------------------------------------------------------------------
; Header. Includes jump table

        module_header   _bbc_ser_std

        .byte   $73, $65, $72           ; "ser"
        .byte   SER_API_VERSION         ; Serial API version number

; Library reference

        .addr   $0000

; Jump table

        .word   SER_INSTALL
        .word   SER_UNINSTALL
        .word   SER_OPEN
        .word   SER_CLOSE
        .word   SER_GET
        .word   SER_PUT
        .word   SER_STATUS
        .word   SER_IOCTL
        .word   SER_IRQ

.rodata

; Tables used to translate RS232 params into register values

BaudTable:                      ; bit7 = 1 means setting is invalid
        .byte   $FF             ; SER_BAUD_45_5
        .byte   $FF             ; SER_BAUD_50
        .byte   $01             ; SER_BAUD_75
        .byte   $FF             ; SER_BAUD_110
        .byte   $FF             ; SER_BAUD_134_5
        .byte   $02             ; SER_BAUD_150
        .byte   $03             ; SER_BAUD_300
        .byte   $FF             ; SER_BAUD_600
        .byte   $04             ; SER_BAUD_1200
        .byte   $FF             ; SER_BAUD_1800
        .byte   $08             ; SER_BAUD_2400
        .byte   $FF             ; SER_BAUD_3600
        .byte   $06             ; SER_BAUD_4800
        .byte   $FF             ; SER_BAUD_7200
        .byte   $07             ; SER_BAUD_9600
        .byte   $08             ; SER_BAUD_19200
        .byte   $FF             ; SER_BAUD_38400
        .byte   $FF             ; SER_BAUD_57600
        .byte   $FF             ; SER_BAUD_115200
        .byte   $FF             ; SER_BAUD_230400

.code

;----------------------------------------------------------------------------
; SER_INSTALL routine. Is called after the driver is loaded into memory. If
; possible, check if the hardware is present.
; Must return an SER_ERR_xx code in a/x.

SER_INSTALL:
        lda     #<SER_ERR_OK
        tax                     ; A is zero
        rts

;----------------------------------------------------------------------------
; SER_UNINSTALL routine. Is called before the driver is removed from memory.
; Must return an SER_ERR_xx code in a/x.

SER_UNINSTALL:
        lda     #<SER_ERR_OK
        tax                     ; A is zero
        rts

;----------------------------------------------------------------------------
; PARAMS routine. A pointer to a ser_params structure is passed in ptr1.
; Must return an SER_ERR_xx code in a/x.

SER_OPEN:
; We only support hardware handshaking
        ldy     #SER_PARAMS::HANDSHAKE  ; Handshake
        lda     (ptr1),y
        cmp     #SER_HS_HW              ; This is all we support
        bne     InvParam

; Set baud rate
        ldy     #SER_PARAMS::BAUDRATE
        lda     (ptr1),y                ; Baudrate index
        tay
        lda     BaudTable,y             ; Get 6551 value
        bmi     InvBaud                 ; Branch if rate not supported
        pha
        tax
        lda     #7
        jsr     OSBYTE  ; OSBYTE 7, x - set serial tx baud
        pla
        tax
        lda     #8
        jsr     OSBYTE  ; OSBYTE 8, x - set serial rx baud

; We only support 8N1
; TODO: support other formats by manipulating control register with OSBYTE $9c
; if we do that, should probably clean up when we're done.
        ldy     #SER_PARAMS::DATABITS   ; Databits
        lda     (ptr1),y
        cmp     #SER_BITS_8
        bne     InvParam

        ldy     #SER_PARAMS::STOPBITS   ; Stopbits
        lda     (ptr1),y
        cmp     #SER_STOP_1
        bne     InvParam

        ldy     #SER_PARAMS::PARITY     ; Parity
        lda     (ptr1),y
        cmp     #SER_PAR_NONE
        bne     InvParam
; Done

        lda     #<SER_ERR_OK
        tax                             ; A is zero
        rts

; Invalid parameter

InvParam:
        lda     #<SER_ERR_INIT_FAILED
        ldx     #>SER_ERR_INIT_FAILED
        rts

; Baud rate not available

InvBaud:
        lda     #<SER_ERR_BAUD_UNAVAIL
        ldx     #>SER_ERR_BAUD_UNAVAIL
        rts

;----------------------------------------------------------------------------
; SER_CLOSE: Close the port, disable interrupts and flush the buffer. Called
; without parameters. Must return an error code in a/x.
;

SER_CLOSE:
        lda     #21
        ldx     #1
        jsr     OSBYTE  ; OSBYTE 21,1 - flush serial input buffer
        lda     #21
        lda     #2
        jsr     OSBYTE  ; OSBYTE 21,2 - flush serial output buffer
        lda     #<SER_ERR_OK
        tax                             ; A is zero
        rts

;----------------------------------------------------------------------------
; SER_GET: Will fetch a character from the receive buffer and store it into the
; variable pointer to by ptr1. If no data is available, SER_ERR_NO_DATA is
; return.
;

SER_GET:
        lda     #2
        ldx     #1
        jsr     OSBYTE  ; OSBYTE 2, 1 - get input from RS423

        lda     #128
        ldx     #<(-2)
        ldy     #>(-2)
        jsr     OSBYTE  ; OSBYTE 128, -2 - get serial input buffer level
        txa
        beq     nodata

        jsr     OSRDCH

        ldy     #0
        sta     (ptr1), y
        lda     #2
        ldx     #0
        jsr     OSBYTE  ; OSBYTE 2, 0 - get input from console again

        lda     #<SER_ERR_OK
        ldx     #>SER_ERR_OK
        rts
nodata:
        lda     #2
        ldx     #0
        jsr     OSBYTE  ; OSBYTE 2, 0 - get input from console

        lda     #<SER_ERR_NO_DATA
        ldx     #>SER_ERR_NO_DATA
        rts

;----------------------------------------------------------------------------
; SER_PUT: Output character in A.
; Must return an error code in a/x.
;

SER_PUT:
;        pha
;        lda     #128
;        ldx     #253
;        ldy     #255
;        jsr     OSBYTE  ; OSBYTE 128, -3 - get serial output buffer free space
;        txa
;        beq     full

;        lda     #3
;        ldx     #7
;        ldy     #0
;        jsr     OSBYTE  ; OSBYTE 3, 7 - send output to rs423 only
;        pla
;        jsr     OSWRCH

;        lda     #3
;        ldx     #0
;        ldy     #0
;        jsr     OSBYTE  ; OSBYTE 3, 0 - restore default output

;        lda     #<SER_ERR_OK
;        ldx     #>SER_ERR_OK
;        rts
        pha
        lda     #128
        ldx     #<(-3)
        ldy     #>(-3)
        jsr     OSBYTE          ; OSBYTE 128, -3 - check available space in serial output buffer
        beq     full

        pla
        tay
        lda     #138
        ldx     #2
        jsr     OSBYTE          ; OSBYTE 138, 2, y - insert character into serial output buffer
        lda     #<SER_ERR_OK
        ldx     #>SER_ERR_OK
        rts


full:   lda     #<SER_ERR_OVERFLOW 
        ldx     #>SER_ERR_OVERFLOW 
        rts

;----------------------------------------------------------------------------
; SER_STATUS: Return the status in the variable pointed to by ptr1.
; Must return an error code in a/x.
;

SER_STATUS:
        lda     #150
        ldx     #8
        jsr     OSBYTE  ; OSBYTE 150, 8 - read from $fe00 + 8 (ACIA control reg.)
        tya     ; returns value in y
        ldx     #0
        sta     (ptr1,x)
        txa                             ; SER_ERR_OK
        rts

;----------------------------------------------------------------------------
; SER_IOCTL: Driver defined entry point. The wrapper will pass a pointer to ioctl
; specific data in ptr1, and the ioctl code in A.
; Must return an error code in a/x.
;

SER_IOCTL:
        lda     #<SER_ERR_INV_IOCTL     ; We don't support ioclts for now
        ldx     #>SER_ERR_INV_IOCTL
        rts

;----------------------------------------------------------------------------
; SER_IRQ: Not used

SER_IRQ         = $0000

