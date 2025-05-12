	; NAME: via_test_program.asm

	org $e000       ; NOTE: we use $e000 as the base address of the 27C64
					; DON'T FORGET TO USE E000 AS THE BASE ADDRESS ON THE EMP-20

init    
	sei             ; turn off interrupts
	cld             ; set CPU to the binary mode
	ldx #$ff        ; load the stack pointer with $ff
	txs    

	; set port B as output, port A as input
	lda #ff
	sta $a002
	lda #00
	sta $a003

	lda #$00        ; set accumulator to 0

loop    

	sta $a000       ; store accumulator in port B
	ldx $a001       ; load x register with port A
	stx $4000       ; store x register in TIL display
	
	jsr delay       ; call delay subroutine
	clc             
	adc #$01        ; add 1 to the accumulator
	jmp loop        ; repeat

delay   
	inx             ; subroutine to kill time (0.25 second)
	bne delay
	iny
	bne delay
	rts



; restart vector

	org $fffc       ; restart vector
	dw init
	end