			; NAME: ACIA_Test_New.asm
						
			; Edited for ADX65

                        ; This program tests the MPU, EPROM, and DISPLAY

                        ; The program loops while putting a number out to DISPLAY

                        ; The DISPLAY counts up in hexadecimal. ACIA is tested by 
                        
                        ; Sending the count number through the serial link.
			
			; Updated so only the printable ASCII 

			; chars from ! (21 hex) to } (7D hex) are counted

                        ;

        org $e000       ; NOTE we use $e000 as the base address of the 27C64

                        ; DON'T FORGET TO USE E000 AS THE BASE ADDRESS ON THE EMP-20

                        ;

init    sei             ; turn off interupts

        cld             ; set CPU to the binary mode

        ldx #$ff        ;

        txs             ; load the stack pointer with $ff

        lda #$00        ; set accumulator to 0
        
                        ;
        
        sta $8001       ; soft reset ACIA  

        lda #$0B        ; no parity, no echo, no Tx/Rx interrupts

        sta $8002       ; set command register

        lda #$1E	; 8-N-1, 9600 baud
        
        sta $8003       ; set control register

	ldx #$21        ; initialize x-register at 21 hex (! char)


loop	

	jsr delay
	jsr delay
	jsr delay
	jsr delay

	ldx $8000       ; load ACIA into x-register (receive data)

	stx $4000       ; store x-register to TIL display

	jsr delay
	jsr delay
	jsr delay
	jsr delay

	ldx $8000        

	stx $4000

        jsr delay	; waste time
	jsr delay
	jsr delay
	jsr delay

	ldx #$BA

	stx $4000


	jmp loop


delay   inx             ; subroutine to kill time (0.25 second)

        bne delay

        iny

        bne delay

	rts

                        ;

        org $fffc       ; restart vector

        dw init

        end
