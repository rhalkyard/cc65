; The TGI driver code for the different VDU modes is identical except for the
; scale factor to adjust the TGI coordinates to the virtual coordinate system
; used by VDU primitives.
;
; The tgi_vdu_template macro generates a TGI driver for the supplied mode,
; using the constants defined in the if-statement at the beginning of the macro.

        .include        "zeropage.inc"

        .include        "tgi-kernel.inc"
        .include        "tgi-error.inc"

        .macpack        generic
        .macpack        module

OSBYTE := $FFF4
OSWORD := $FFF1
OSWRCH := $FFEE

; Shift the 16-bit quantity at addr left n times
.macro lsh16 addr, n
    .if (n > 0)
        .repeat n
            clc
            rol addr
            rol addr+1
        .endrepeat
    .endif
.endmacro

; TGI coordinate system has origin at top-left.
; This macro inverts and offsets the scaled Y coordinate to have origin at bottom-left
; *addr = -(*addr) + 1023
.macro invert addr
    clc
    lda     addr
    eor     #$ff
    adc     #<(1023 + 1)  ; twos complement
    sta     addr
    lda     addr+1
    eor     #$ff
    adc     #>(1023 + 1)
    sta     addr+1
.endmacro

; Macro to generate a TGI driver for a given mode;
;
; 'mode' argument should be a valid graphics screen mode (0, 1, 2, 4, 5), or -1
; to build a 'universal' driver that operates in any mode using the virtual
; 1280x1024 coordinate space.
;
; The reason that both mode-specific and universal drivers are offered, is that
; the scaled virtual coordinate space is large enough to cause undesired results
; with some cross-platform software. For BBC-specific programs, the generic
; 'tgi-vdu' driver is smaller and faster, since it does not perform a redundant
; scaling step to convert physical screen coordinates to virtual screen 
; coordinates.
.macro tgi_vdu_template mode
    .if (mode = 0)          ; MODE 0
        _X_RES = 640        ; 640x256
        _Y_RES = 256
        _X_SHIFT = 1        ; Multiply X coordinates by 2^1 = 2
        _Y_SHIFT = 2        ; Multiply Y coordinates by 2^2 = 4
        _ASPECT = $0100     ; 1:1 - VDU driver handles aspect scaling
        _PALETTE_SIZE = 2   ; 2-colour palette
        .define _PALETTE_DEFAULT .byte 0, 7
    .elseif (mode = 1)      ; MODE 1
        _X_RES = 320        ; 320x256
        _Y_RES = 256
        _X_SHIFT = 2        ; Multiply X coordinates by 2^2 = 4
        _Y_SHIFT = 2        ; Multiply Y coordinates by 2^2 = 4
        _ASPECT = $0100
        _PALETTE_SIZE = 4   ; 4-colour palette
        .define _PALETTE_DEFAULT .byte 0, 3, 5, 7
    .elseif (mode = 2)      ; MODE 2
        _X_RES = 160        ; 160x256
        _Y_RES = 256
        _X_SHIFT = 3        ; Multiply X coordinates by 2^3 = 8
        _Y_SHIFT = 2        ; Multiply Y coordinates by 2^2 = 4
        _ASPECT = $0100     ; 1:1 - VDU driver handles aspect scaling
        _PALETTE_SIZE = 16  ; 16-colour palette (8 colours + flashing)
        .define _PALETTE_DEFAULT .byte 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    .elseif (mode = 4)      ; MODE 4
        _X_RES = 320        ; 320x256
        _Y_RES = 256
        _X_SHIFT = 2        ; Multiply X coodinates by 2^2 = 4
        _Y_SHIFT = 2        ; Multiply Y coordinates by 2^2 = 4
        _ASPECT = $0100     ; 1:1 - VDU driver handles aspect scaling
        _PALETTE_SIZE = 2   ; 2-colour palette
        .define _PALETTE_DEFAULT .byte 0, 7
    .elseif (mode = 5)      ; MODE 5
        _X_RES = 160        ; 160x256
        _Y_RES = 256        ; 1:1 - VDU driver handles aspect scaling
        _X_SHIFT = 3        ; Multiply X coordinates by 2^3 = 8
        _Y_SHIFT = 2        ; Multiply Y coordinates by 2^2 = 4
        _ASPECT = $0100     ; 1:1 - VDU driver handles aspect scaling
        _PALETTE_SIZE = 4   ; 4-colour palette
        .define _PALETTE_DEFAULT .byte 0, 3, 5, 7
    .elseif (mode = -1)     ; Not actually a MODE, used to build a 'generic' driver that works in any mode
        _X_RES = 1280       ; Expose VDU 'virtual' screen resolution directly
        _Y_RES = 1024
        _X_SHIFT = 0
        _Y_SHIFT = 0
        _ASPECT = $0100     ; 1:1 - VDU driver handles aspect scaling
        _PALETTE_SIZE = 16  ; 16-colour palette, not all modes will use whole palette
        .define _PALETTE_DEFAULT .byte 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    .else
        .fatal .concat("TGI driver template not defined for mode: ", .string(mode))
    .endif

; ------------------------------------------------------------------------
; Header. Includes jump table and constants.

; First part of the header is a structure that has a magic and defines the
; capabilities of the driver

        .byte   $74, $67, $69           ; "tgi"
        .byte   TGI_API_VERSION         ; TGI API version number
        .addr   $0000                   ; Library reference
        .word   _X_RES                  ; X resolution
        .word   _Y_RES                  ; Y resolution
        .byte   _PALETTE_SIZE           ; Number of drawing colors
        .byte   1                       ; Number of screens available
        .byte   8                       ; System font X size
        .byte   8                       ; System font Y size
        .word   _ASPECT                 ; Aspect ratio (based on 4/3 display)
        .byte   0                       ; TGI driver flags

; Next comes the jump table. With the exception of IRQ, all entries must be
; valid and may point to an RTS for test versions (function not implemented).

        .addr   INSTALL
        .addr   UNINSTALL
        .addr   INIT
        .addr   DONE
        .addr   GETERROR
        .addr   CONTROL
        .addr   CLEAR
        .addr   SETVIEWPAGE
        .addr   SETDRAWPAGE
        .addr   SETCOLOR
        .addr   SETPALETTE
        .addr   GETPALETTE
        .addr   GETDEFPALETTE
        .addr   SETPIXEL
        .addr   GETPIXEL
        .addr   LINE
        .addr   BAR
        .addr   TEXTSTYLE
        .addr   OUTTEXT

; ------------------------------------------------------------------------
; Data.

; Variables mapped to the zero page segment variables. Some of these are
; used for passing parameters to the driver.

X1              := ptr1
Y1              := ptr2
X2              := ptr3
Y2              := ptr4
TEXT            := ptr3

; Absolute variables used in the code

.bss

ERROR:          .res    1                   ; Error code
PALETTE:        .res    _PALETTE_SIZE       ; The current palette

; Constants and tables

.rodata

DEFPALETTE:     _PALETTE_DEFAULT
PALETTESIZE     = * - DEFPALETTE

.code

; ------------------------------------------------------------------------
; INSTALL routine. Is called after the driver is loaded into memory. May
; initialize anything that has to be done just once. Is probably empty
; most of the time.
;
; Must set an error code: NO
;

INSTALL:
;       rts                     ; Fall through


; ------------------------------------------------------------------------
; UNINSTALL routine. Is called before the driver is removed from memory. May
; clean up anything done by INSTALL but is probably empty most of the time.
;
; Must set an error code: NO
;

UNINSTALL:
        rts


; ------------------------------------------------------------------------
; INIT: Changes an already installed device from text mode to graphics
; mode.
; Note that INIT/DONE may be called multiple times while the driver
; is loaded, while INSTALL is only called once, so any code that is needed
; to initializes variables and so on must go here. Setting palette and
; clearing the screen is not needed because this is called by the graphics
; kernel later.
; The graphics kernel will never call INIT when a graphics mode is already
; active, so there is no need to protect against that.
;
; Must set an error code: YES
;

INIT:
.if (mode < 0)
; Universal 'mode -1' driver needs to validate the current mode
; set error ahead of time; this gets overwritten later if successful
        lda     #TGI_ERR_INV_MODE
        sta     ERROR

        lda     #$A0
        ldx     #$55
        jsr     OSBYTE  ; OSBYTE $A0, $55 - read VDU workspace, offset $55 = current mode
        cpy     #7
        beq     errexit
        cpy     #6
        beq     errexit
        cpy     #3
        beq     errexit
.else
; Mode-specific drivers: enter desired mode
        lda     #22
        jsr     OSWRCH
        lda     #mode | $80 ; use shadow flag
        jsr     OSWRCH
.endif
        lda     #20
        jsr     OSWRCH  ; VDU 20 - reset palette

; Done, reset the error code
        lda     #TGI_ERR_OK
        sta     ERROR
errexit:   
        rts

; ------------------------------------------------------------------------
; DONE: Will be called to switch the graphics device back into text mode.
; The graphics kernel will never call DONE when no graphics mode is active,
; so there is no need to protect against that.
;
; Must set an error code: NO
;

DONE:
    jsr     CLEAR
    lda     #20
    jsr     OSWRCH  ; VDU 20 - reset palette
    lda     #4
    jmp     OSWRCH  ; VDU 4 - select text cursor

; ------------------------------------------------------------------------
; GETERROR: Return the error code in A and clear it.

GETERROR:
    ldx     #TGI_ERR_OK
    lda     ERROR
    stx     ERROR
    rts

; ------------------------------------------------------------------------
; CONTROL: Platform/driver specific entry point.
;
; Must set an error code: YES
;

CONTROL:
    lda     #TGI_ERR_INV_FUNC
    sta     ERROR
    rts

; ------------------------------------------------------------------------
; CLEAR: Clears the screen.
;
; Must set an error code: NO
;

CLEAR:
    lda     #16
    jmp     OSWRCH  ; VDU 16 - clear graphics screen

; ------------------------------------------------------------------------
; SETVIEWPAGE: Set the visible page. Called with the new page in A (0..n).
; The page number is already checked to be valid by the graphics kernel.
;
; Must set an error code: NO (will only be called if page ok)
;

SETVIEWPAGE:
;       rts                     ; Fall through

; ------------------------------------------------------------------------
; SETDRAWPAGE: Set the drawable page. Called with the new page in A (0..n).
; The page number is already checked to be valid by the graphics kernel.
;
; Must set an error code: NO (will only be called if page ok)
;

SETDRAWPAGE:
        rts

; ------------------------------------------------------------------------
; SETCOLOR: Set the drawing color (in A). The new color is already checked
; to be in a valid range (0..maxcolor-1).
;
; Must set an error code: NO (will only be called if color ok)
;

SETCOLOR:
    tax
    lda     #18
    jsr     OSWRCH
    lda     #0
    jsr     OSWRCH
    txa
    jmp     OSWRCH  ; VDU 18, 0, x - set drawing colour

; ------------------------------------------------------------------------
; SETPALETTE: Set the palette (not available with all drivers/hardware).
; A pointer to the palette is passed in ptr1. Must set an error if palettes
; are not supported
;
; Must set an error code: YES
;

SETPALETTE:
    ldy     #$ff
:   iny
    lda     #19
    jsr     OSWRCH
    tya
    jsr     OSWRCH
    lda     (ptr1), y
    sta     PALETTE, y
    jsr     OSWRCH
    lda     #0
    jsr     OSWRCH
    jsr     OSWRCH
    jsr     OSWRCH  ; VDU 19, a, b, 0, 0, 0 - set palette entry a to colour b
    cpy     #_PALETTE_SIZE-1
    bcc     :-

    lda     #TGI_ERR_OK
    sta     ERROR
    rts

; ------------------------------------------------------------------------
; GETPALETTE: Return the current palette in A/X. Even drivers that cannot
; set the palette should return the default palette here, so there's no
; way for this function to fail.
;
; Must set an error code: NO
;

GETPALETTE:
    lda     #<PALETTE
    ldx     #>PALETTE
    rts

; ------------------------------------------------------------------------
; GETDEFPALETTE: Return the default palette for the driver in A/X. All
; drivers should return something reasonable here, even drivers that don't
; support palettes, otherwise the caller has no way to determine the colors
; of the (not changeable) palette.
;
; Must set an error code: NO (all drivers must have a default palette)
;

GETDEFPALETTE:
    lda     #<DEFPALETTE
    ldx     #>DEFPALETTE
    rts

; ------------------------------------------------------------------------
; SETPIXEL: Draw one pixel at X1/Y1 = ptr1/ptr2 with the current drawing
; color. The coordinates passed to this function are never outside the
; visible screen area, so there is no need for clipping inside this function.
;
; Must set an error code: NO
;

SETPIXEL:
    lsh16 ptr1, _X_SHIFT    ; scale X and Y to virtual coordinate system
    lsh16 ptr2, _Y_SHIFT
    invert ptr2             ; Invert Y axis

    lda     #25
    jsr     OSWRCH
    lda     #69
    jsr     OSWRCH
    lda     ptr1
    jsr     OSWRCH
    lda     ptr1+1
    jsr     OSWRCH
    lda     ptr2
    jsr     OSWRCH
    lda     ptr2+1
    jmp     OSWRCH  ; VDU 25,69,x;y; - plot point at (x,y)

; ------------------------------------------------------------------------
; GETPIXEL: Read the color value of a pixel and return it in A/X. The
; coordinates passed to this function are never outside the visible screen
; area, so there is no need for clipping inside this function.


GETPIXEL:
    lsh16 ptr1, _X_SHIFT    ; scale X and Y to virtual coordinate system
    lsh16 ptr2, _Y_SHIFT
    invert ptr2             ; Invert Y axis

    lda     #9
    ldx     #<ptr1
    ldy     #>ptr1
    jsr     OSWORD  ; OSWORD 9, <ptr> - get color at (x,y) coord
    lda     ptr1+3  ; returns colour in 4th byte
    ldx     #$00
    rts

; ------------------------------------------------------------------------
; LINE: Draw a line from X1/Y1 to X2/Y2, where X1/Y1 = ptr1/ptr2 and
; X2/Y2 = ptr3/ptr4 using the current drawing color.
;
; X1,X2 etc. are set up above (x2=LINNUM in particular)
; Format is LINE x2,y2,x1,y1
;
; Must set an error code: NO
;

LINE:
    lsh16 ptr1, _X_SHIFT    ; Scale coordinates to virtual coordinate system
    lsh16 ptr2, _Y_SHIFT
    lsh16 ptr3, _X_SHIFT
    lsh16 ptr4, _Y_SHIFT
    invert ptr2             ; Invert Y axis
    invert ptr4

    lda     #25
    jsr     OSWRCH
    lda     #4
    jsr     OSWRCH
    lda     ptr1
    jsr     OSWRCH
    lda     ptr1+1
    jsr     OSWRCH
    lda     ptr2
    jsr     OSWRCH
    lda     ptr2+1
    jsr     OSWRCH  ; VDU 25, 4, x; y; - move cursor to (x,y)

    lda     #25
    jsr     OSWRCH
    lda     #5
    jsr     OSWRCH
    lda     ptr3
    jsr     OSWRCH
    lda     ptr3+1
    jsr     OSWRCH
    lda     ptr4
    jsr     OSWRCH
    lda     ptr4+1
    jmp     OSWRCH  ; VDU 25, 5, x; y; - draw line to (x,y)


; ------------------------------------------------------------------------
; BAR: Draw a filled rectangle with the corners X1/Y1, X2/Y2, where
; X1/Y1 = ptr1/ptr2 and X2/Y2 = ptr3/ptr4 using the current drawing color.
; Contrary to most other functions, the graphics kernel will sort and clip
; the coordinates before calling the driver, so on entry the following
; conditions are valid:
;       X1 <= X2
;       Y1 <= Y2
;       (X1 >= 0) && (X1 < XRES)
;       (X2 >= 0) && (X2 < XRES)
;       (Y1 >= 0) && (Y1 < YRES)
;       (Y2 >= 0) && (Y2 < YRES)
;
; Must set an error code: NO
;

BAR:
    lsh16 ptr1, _X_SHIFT    ; Scale coordinates to virtual coordinate system
    lsh16 ptr2, _Y_SHIFT
    lsh16 ptr3, _X_SHIFT
    lsh16 ptr4, _Y_SHIFT
    invert ptr2             ; Invert Y axis
    invert ptr4

.ifdef __BBCMASTER__
        lda     #25
        jsr     OSWRCH
        lda     #4
        jsr     OSWRCH
        lda     ptr1
        jsr     OSWRCH
        lda     ptr1+1
        jsr     OSWRCH
        lda     ptr2
        jsr     OSWRCH
        lda     ptr2+1
        jsr     OSWRCH  ; VDU 25, 4, x; y; - move cursor to (x,y)

        lda     #25
        jsr     OSWRCH
        lda     #101
        jsr     OSWRCH
        lda     ptr3
        jsr     OSWRCH
        lda     ptr3+1
        jsr     OSWRCH
        lda     ptr4
        jsr     OSWRCH
        lda     ptr4+1
        jmp     OSWRCH  ; VDU 25, 101, x; y; - fill rectangle from last coordinates to (x,y)
.else ; __BBCMASTER__
; plain Model Bs don't have rectangle primitives, draw two filled triangles instead
        lda     #25
        jsr     OSWRCH
        lda     #4
        jsr     OSWRCH
        lda     ptr1
        jsr     OSWRCH
        lda     ptr1+1
        jsr     OSWRCH
        lda     ptr4
        jsr     OSWRCH
        lda     ptr4+1
        jsr     OSWRCH  ; VDU 25, 4, x; y; - move cursor to (x,y)

        lda     #25
        jsr     OSWRCH
        lda     #4
        jsr     OSWRCH
        lda     ptr1
        jsr     OSWRCH
        lda     ptr1+1
        jsr     OSWRCH
        lda     ptr2
        jsr     OSWRCH
        lda     ptr2+1
        jsr     OSWRCH  ; VDU 25, 4, x; y; - move cursor to (x,y)

        lda     #25
        jsr     OSWRCH
        lda     #85
        jsr     OSWRCH
        lda     ptr3
        jsr     OSWRCH
        lda     ptr3+1
        jsr     OSWRCH
        lda     ptr4
        jsr     OSWRCH
        lda     ptr4+1
        jsr     OSWRCH  ; VDU 25, 85, x; y; - fill triangle from last 2 points to (x,y)

        lda     #25
        jsr     OSWRCH
        lda     #85
        jsr     OSWRCH
        lda     ptr3
        jsr     OSWRCH
        lda     ptr3+1
        jsr     OSWRCH
        lda     ptr2
        jsr     OSWRCH
        lda     ptr2+1
        jmp     OSWRCH  ; VDU 25, 85, x; y; - fill triangle from last 2 points to (x,y)
.endif ; __BBCMASTER__

; ------------------------------------------------------------------------
; TEXTSTYLE: Set the style used when calling OUTTEXT. Text scaling in X and Y
; direction is passend in X/Y, the text direction is passed in A.
;
; Must set an error code: NO
;

TEXTSTYLE:  ; TODO
        rts


; ------------------------------------------------------------------------
; OUTTEXT: Output text at X/Y = ptr1/ptr2 using the current color and the
; current text style. The text to output is given as a zero terminated
; string with address in ptr3.
;
; Must set an error code: NO
;

OUTTEXT:    ; TODO
        rts
.endmacro