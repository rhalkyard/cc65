;
; Dummy IRQ subroutines - we don't actually support IRQs yet
;

        .export         initirq, doneirq

; ------------------------------------------------------------------------

.segment        "ONCE"

initirq:
        rts

; ------------------------------------------------------------------------

.code

doneirq:
        rts
