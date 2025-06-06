; Author : Jorrel Rajan
; Full Final file with FSM


;       Lecky Routine    V3.1      typed by Tad Coburn
;       modified by Dan Lockwood (V2.0 -> V3.0)
;       MODIFIED BY MGL (V3.0 -> V3.1)
;		MODIFIED BY PYT (V3.1 -> V4.0)
;		Modified for use with the ADX-65
;       The modifications made to update Lecky to verion 3.0 were:
;               1) Get rid of the ability to store a train on a side track under
;                  "caution" status where the light is yellow.
;               2) Change light status updating so when the mailbox is cleared the
;                  light goes from red to yellow instead of from red to green.
;               3) When the Dispatcher asks for the block computer to send the block
;                  status back, the least two significant bits of CONTRL are sent to
;                  inform the Dispatcher which light (north or south) will turn green.
;               4) The protocol for communication has been changed. Now, the Dispatcher
;                  can tell Lecky3_0 to set a light from yellow to green. (see above 3).
;               NOTE: For more details, please see the Daniel C. Lockwood report.
;               NOTE: The old code was left in and commented out for reference.

; Label Declarations
DFLAG  EQU $3FF         ; 00 EQU No return data to send
WDATAH EQU $3FE         ; High byte of return data
WDATAL EQU $3FD         ; Low byte of return data
BLKID  EQU $3FC         ; Track computer block id number
RDATA  EQU $3FB         ; Controller data sent to this computer
FRANUM EQU $3FA         ; Current wave frame number
OKWR   EQU $3F9         ; 00 EQU Controller not ready to accept return data
INTMPU EQU $3F8         ; FF EQU MPU has issued a poll request
POS    EQU $3F7         ; Output during positive power cycle
NEG    EQU $3F6         ; Output during negative power cycle
PLRTY  EQU $3F5         ; 00 EQU Negative polarity, FF EQU Positive polarity
LCV    EQU $3F4         ; Local variable
MASK   EQU $3F3         ; Local constant
CNT    EQU $3F2         ; Local variable
TIME   EQU $3F1         ; Median period of a '0' and a '1' bit of
NRTHGN EQU $32B                 ; Flag to tell if north light is green #$00 off #$FF on
SUTHGN EQU $32A                 ; Flag to tell if south light green - #$00 off #$FF on
NOVRD  EQU $32C         ; *** V 3.1 *** OVERRIDE OF NORTH LIGHTS/KILLS
SOVRD  EQU $32D         ; *** V 3.1 *** OVERRIDE OF SOUTH LIGHTS/KILLS
WBUF   EQU $3D0         ; Output table - Data to be written to port B
FTAB   EQU $3AC         ; Reconstructed 'nibbles' of control data
LFLAG  EQU $377
STFLG  EQU $389         ; ALL NEW AND IMPORTANT
TABLE  EQU $38A         ; Input table - IFR contents for each input cycle
NTRAIN EQU $3CF         ; Train number on north track
STRAIN EQU $3CE         ; Train number on south track
STSDTR EQU $3CD         ; Student-use location for train on siding
BCRASM EQU $3C8         ; Location for bar code shift in
SCOUNT EQU $3C7         ; Delay count for reader passage of south train
NCOUNT EQU $3C6         ; Delay count for reader passage of north train
SASSEM EQU $3C5         ; Assemble area for south train number
NASSEM EQU $3C4         ; Assemble area for north train number
CONTRL EQU $3C2         ; Save area for control bits-lights, track kills
CBITS  EQU $3C1         ; Save area for communications bits during LOCCON
YLTOGN EQU $3C0		; Toggle for which light to turn from yellow to green

;	Jorrel's memory locations.
CMDIC		EQU $3CA		; Stores the command we wish to run. EX: 01, 02, or 03.
ENGNUM		EQU $3CB		; Stores our engine number. this is set in initialization.
ARDUSEND	EQU $3CC		; put data in here before sending stuff to arduino
ARDUREC		EQU $3CA		; return data from the arduino


;       Versatile Interace Adapter  (VIA)  Addresses
VDDRB  EQU $A002        ;  Data direction register B
VORB   EQU $A000        ; Output register B
VT2L   EQU $A008        ; Timer 2 low byte
VT2H   EQU $A009        ; Timer 2 high byte
VACR   EQU $A00B        ; Auxilliary control register
VPCR   EQU $A00C        ; Peripheral control register
VIFR   EQU $A00D        ; Interrupt flag register
VIER   EQU $A00E        ; Interrupt enable register

; ACIA Addresses and variables
ACIA    EQU $8004		; Input/output address
ACIASR  EQU $8005		; ACIA Status register
ACIACMR EQU $8006        ; ACIA Command Register
ACIACCR EQU $8007		; ACIA Control Register

;   **** BEGINNING OF USER SECTION ****
;	VIA $8000 ADDRESSES
V8BI	EQU $8000
V8BD	EQU $8002
V8AO	EQU $8001
V8AD	EQU $8003



;	LOCAL VARIABLES
DISP	EQU $4000

;	INITIALIZATION
	ORG $E000
	SEI
	CLD
	LDX #$FF
	TXS
	LDA #$05	; THIS IS COMPUTER No. 5
	STA BLKID
	LDA #$00
	STA NOVRD	; NO OVERRIDE ON NORTH TRACK
	STA SOVRD	; NO OVERRIDE ON SOUTH TRACK
	STA DFLAG       ; NO DATA TO SEND
	
	JSR LONGDELAY
	LDA #$88
	STA DISP
	JSR LONGDELAY

	; Set Display to FF to acknowledge that Hornby isn't plugged in yet
	LDA #$FF
	STA DISP
	; Initialize Hornby
	JSR INIT        ; INITIALIZATION

	JSR INITACIA	; INITIALIZE THE ACIA

	; Set our engine number
	LDA #$12
	STA ENGNUM
	
	LDA #$99
	STA DISP
	
	JSR DELAY
	JSR DELAY
	JSR DELAY
	JSR DELAY
	
	; Initialize the arduino
	LDA #$20
	STA ARDUSEND
	JSR CALLARDUINO
	
	; Set Display to 00 to acknowledge that all initialization went smoothly
	LDA #$00 ; PUT 00 ON THE DISPLAY
	STA DISP	

; Temporary Start condition
;ARENA
;	LDA RDATA
;	STA DISP
;	CMP #$85
;	BEQ OUTSIDE
;	JMP ARENA

; Loop on NTRAIN. If it is the correct engine, then transition 
; to STARTSEQ. else, transition to SKIPSEQ.

	LDA #$00
	STA NTRAIN

CHECKNTRAIN
	LDA NTRAIN
	STA DISP
	CMP #$12
	BNE CHECKNTRAIN

STARTSEQ
	; NOTE: wait until HALL 1: LOW. then, transition to SLOW PICKUP
	LDA #$21
	STA ARDUSEND
	JSR CALLARDUINO

SLOWPICKUP
	
	; immediately after, cut track power
	LDA #$26
	STA ARDUSEND
	JSR CALLARDUINO

	; SET SPEED TO SLOW.
	LDA #$01
	STA CMDIC
	JSR SENDIC

	; resume track power with new speed in place
	LDA #$27
	STA ARDUSEND
	JSR CALLARDUINO
	
	JSR DELAY

	; NOTE: wait until HALL 2: LOW. then, transition to STOP PICKUP
	LDA #$25
	STA ARDUSEND
	JSR CALLARDUINO
	
	JMP STOPPICKUP

STOPPICKUP

	; NOTE: trigger the pickup routine
	LDA #$22
	STA ARDUSEND
	JSR CALLARDUINO
	
	; display result to TILs
	LDA ARDUREC
	STA DISP
	
	JMP FASTDROPOFF

FASTDROPOFF
	; set speed FAST
	LDA #$02
	STA CMDIC
	JSR SENDIC
	
	; resume track power with marble in place
	LDA #$27
	STA ARDUSEND
	JSR CALLARDUINO
	
	JSR LONGDELAY
	
	; NOTE: wait until HALL 3: LOW. then, transition to SLOW DROPOFF
	LDA #$21
	STA ARDUSEND
	JSR CALLARDUINO
	
	JSR DELAY
	
	JMP SLOWDROPOFF

SLOWDROPOFF
	; immediately after, cut track power
	LDA #$26
	STA ARDUSEND
	JSR CALLARDUINO

	; SET SPEED TO SLOW.
	LDA #$01
	STA CMDIC
	JSR SENDIC
	
	; resume track power with new speed in place
	LDA #$27
	STA ARDUSEND
	JSR CALLARDUINO
	
	JSR DELAY
 
	; NOTE: Check if HALL 4 OR 5: LOW. then, dropoff routine
	LDA #$25
	STA ARDUSEND
	JSR CALLARDUINO
	
	JMP STOPDROPOFF

STOPDROPOFF

	; NOTE: trigger DROPOFF ROUTINE.
	LDA #$23
	STA ARDUSEND
	JSR CALLARDUINO 
	
	LDA ARDUREC
	
	; store count to display
	STA DISP
	
	; look at dropoff return data. if its > marble limit, stop sorting marbles. else, keep going.
	CMP #$EE
	BEQ EXITSTATE
	JMP CONTINUESTATE
	
CONTINUESTATE
	; set speed FAST
	LDA #$02
	STA CMDIC
	JSR SENDIC
	
	; resume track power with marble in contanier now. wait some time before transition such that we can leave the halls
	LDA #$27
	STA ARDUSEND
	JSR CALLARDUINO
	
	JSR DELAY
	
	JMP STARTSEQ

EXITSTATE
	LDA #$DD
	STA DISP

	; set speed FAST
	LDA #$02
	STA CMDIC
	JSR SENDIC
	
	; resume track power with marble in contanier now. wait some time before transition such that we can leave the halls
	LDA #$27
	STA ARDUSEND
	JSR CALLARDUINO
	
	JSR DELAY
	
	; NOTE: wait until HALL 1: LOW. then, transition to SLOW PICKUP
	LDA #$21
	STA ARDUSEND
	JSR CALLARDUINO
	
	; immediately after, cut track power
	LDA #$26
	STA ARDUSEND
	JSR CALLARDUINO

	; SET SPEED TO SUPERFAST.
	LDA #$03
	STA CMDIC
	JSR SENDIC
	
	; tracks OUTSIDE
	LDA #$24
	STA ARDUSEND
	JSR CALLARDUINO

	; resume track power with new speed in place
	LDA #$27
	STA ARDUSEND
	JSR CALLARDUINO
	
FINALLOOP
	JMP FINALLOOP
	


CALLARDUINO
	; Reset the recieve information buffer
	LDA #$00
	STA ARDUREC

ACIATX	
	LDA ACIASR ; Load the status register
	AND #$10 ; Is the ACIA TX line clear?
	BEQ ACIATX; If it is not, then wait...

	LDA ARDUSEND ; load send data
	STA ACIA ; Send data to the ACIA TX 

; We now start to load the ACIA with some data...
ACIARX	
	; Is the buffer full? If not, wait for it to be filled
	LDA ACIASR 	; Load the status register  	
	AND #$08 	; Is buffer full?
	BEQ ACIARX	; If not, then wait...

	LDA ACIA ; Read the buffer
	STA ARDUREC ; store the code obtained

	; Maybe??
	BEQ ACIARX

	RTS

DELAY
	INX
	BNE DELAY
	INY
	BNE DELAY
	RTS
	
	
LONGDELAY
	JSR DELAY
	JSR DELAY
	JSR DELAY
	JSR DELAY
	RTS
		
; (Re-)Initialize the ACIA
INITACIA
	LDA #$00;		
	STA ACIASR; Soft reset
	LDA #$0B; No parity, no echo, no TX/RX interrupts
	STA ACIACMR;

	LDA #$1E; 8-N-1, 9600 Baud
	STA ACIACCR;

	; Read anything that was in the buffer to remove it from buffer (destructive read)
	LDA ACIA
	RTS



; ********************************************
;	INTERACTIVE CONTROL COMMANDS
; ********************************************
SENDIC	
	LDA CMDIC
	STA WDATAH
	LDA #$00
	STA RDATA
	LDA #$FF
	STA DFLAG
WAITIC
	LDA RDATA
	BEQ WAITIC

	RTS


;   **** END OF USER SECTION ****

;

;

;   **** LECKY ROUTINE START ****
	ORG $FB00
;       Procedure:  INIT
;       Function :  initialize variables, synchronize with track waveform
;       NOTE BENE:  This procedure has been altered since J. Gurian and D. Simon
;                   wrote their project report
INIT    SEI
	LDA #$FB
	STA VDDRB
	LDX #00

CKPLS   LDA #00
	STA VORB
	LDA #01
	STA VORB
	INX
	CPX #$08
	BNE CKPLS
	LDA #00
	STA LFLAG
	STA RDATA
	STA VACR
	STA INTMPU
	STA OKWR
	STA CONTRL
	STA NASSEM
	STA SASSEM
	STA NCOUNT
	STA SCOUNT
	STA BCRASM
	STA NTRAIN
	STA STRAIN
	STA NRTHGN
	STA SUTHGN
	STA NOVRD
	STA SOVRD
	LDA #$01          ;             Initialize YLTOGN to South mailbox
	STA YLTOGN
	LDA #$7F
	STA VIER      ;         Clear all interrupts
	LDA #$98
	STA VIER      ;         Enable CB1, CB2 Interrupts
	JSR PWRPOL    ;         Determine polarity of input signal
	JSR PANIC     ;         Synchronize with track waveform

;       Find the period of a '0' or '1' and compute an intermediate value
	LDA VORB       ;        Clear interrupt flag in IFR
	JSR WTHF       ;        Wait for high frequency portion of waveform
	LDA #255
	STA VT2L       ;        Set timer for 255 u-secs
	JSR WTCB1      ;        Align to a CB1 interrupt
	LDA #00
	STA VT2H
	LDA VORB
	JSR WTCB1
	LDA VT2L
	STA TIME
	SEC
	LDA #255
	SBC TIME
	SBC #02
	CMP #110
	BMI SHORT
	LSR A

SHORT
	STA TIME
	CLC
	LSR A
	CLC
	ADC TIME
	SBC #38        ;        Subtract constant due to timing errors from execution of instructions
	STA TIME
	CLI
	RTS

;       Routine:  ISERV
;       Function: Interrupt service routine, calls lower level routines
ISERV   SEI
	; Push registers on system stack
	PHA
	TXA
	PHA
	TYA
	PHA
	LDA #$10
	BIT VIFR      ; Test for CB1 interrupt
	BNE VCB1      ; Branch if found
	LDA #$08
	BIT VIFR      ; Test for CB2 interrput
	BEQ RETURN    ; Skip if not found
	JMP VCB2

;       RETURN  - restore stack and leave
RETURN  PLA
	TAY
	PLA
	TAX
	PLA
	RTI

;       Routine :   VCB1
;       Function:   Handle CB1 interrputs

VCB1    INC FRANUM
	LDA FRANUM
	CMP #06
	BMI LESS
	JMP PANIC2

LESS    LDA #$20
	BIT VIFR
	BNE L1
	JMP PANIC2

L1      LDA #00
	STA VT2L
	LDA #$30
	STA VT2H      ; Set timer2 EQU 12288 u-secs
	LDA NEG
	ORA #01       ; Make sure PB0 is set
	STA VORB
	LDA FRANUM
	CMP #02
	BNE L2
	JSR LOCCON
L2      JMP RETURN

;       Routine :   PWRPOL
;       Function:   Determine polarity of track signal
PWRPOL  LDA #$40
	STA POS
	STA VPCR
	LDA #$FF
	STA PLRTY     ; Set polarity to positive
	LDA #$20
	STA NEG
	LDA #00
	STA VT2L      ; Set low Timer2 byte to zero
	LDX #100
	LDA VORB
	JSR WTCB2

TSTLP   DEX
	BEQ TSTEND
	LDA #$30
	STA VT2H
	LDA VORB
	JSR WTCB2
	LDA VT2H
	STA TIME      ; Store time between CB2 interrputs
	LDA VIFR
	BIT T2TEST
	BNE TSTLP
	LDA TIME
	CMP #$11
	BPL TSTLP

	LDA #00
	STA PLRTY
	LDA #$20
	STA POS
	LDA #$10
	STA VPCR
	LDA #$40
	STA NEG

TSTEND  RTS

;       Procedure:   PANIC
;       Function :   Synchronize ISERV with track waveform
PANIC   JSR WTHF      ; Wait for data portion of waveform
	LDA #00
	STA VT2L      ; Zero low byte of Timer2
PLOOP   LDA #$01
	STA VT2H      ; Set Timer2 for 255 u-secs
	JSR WTCB1
	LDA VORB      ; Clear IFR of CB2 interrput
	LDA VIFR
	BIT T2TEST    ; Has Timer2 timed out?
	BEQ PLOOP     ; If not, go back

	LDA #02
	STA FRANUM

	RTS

;       Routine :   PANIC2
;       Function:   Calls subroutine PANIC and then branches to return
PANIC2  JSR PANIC
	JMP RETURN

;       Procedure:   PARITY
;       Function :   Performs four-bit addition of controller nibbles
;                    Returns '0F' in register A if there is no parity error
PARITY  LDA #$08
	STA CNT
	CLC
	LDA #00
	TAX

ALOOP   ADC FTAB,X    ; Adds up nibbles
	BIT AMASK
	BEQ A1
	SEC
	BCS A2

A1      CLC

A2      INX
	AND #$EF
	DEC CNT
	BNE ALOOP
	RTS

AMASK   DB $10

;       Procedure:   WTCB1
;       Function :   Wait for a CB1 interrupt and then return
WTCB1  LDA #$10

ABC     BIT VIFR
	BEQ ABC
	RTS

;       Procedure:   WTCB2
;       Function :   Wait for a CB2 interrupt and then return
WTCB2  LDA #$08

XYZ     BIT VIFR
	BEQ XYZ
	RTS

;       Procedure:   WT2
;       Function :   Wait for Timer2 to time out and then return
WT2     LDA #$20

CBA     BIT VIFR
	BEQ CBA
	RTS

;       Procedure:   WTHF
;       Function :   Wait for the high frequency portion of the waveform
WTHF    LDA VORB
	JSR WTCB1
	LDA #00
	STA VT2L

HFLOOP  LDA #$01
	STA VT2H
	LDA VORB
	JSR WTCB1
	LDA #$20
	BIT VIFR
	BNE HFLOOP

	LDA VORB      ; Clear CB interrupts
	RTS

;       Procedure:   VCB2
;       Function :   Handle CB2 interrupts
VCB2    INC FRANUM
	LDA #05
	CMP FRANUM
	BMI PANIC2
	BEQ CONT1
	LDA POS
	ORA #$01
	STA VORB
	JMP RETURN

CONT1   LDA #$81
	STA VORB      ; Set PB7,PB0 on - signal data frame to outside world

;       NEW SECTION FOR AUTOMATIC STATUS REPORTING
	LDA STFLG
	BEQ CONTQ     ; If STFLG EQU 0, then there is no status request
	LDA WDATAL    ; If STFLG <> 0, then save  WDATAH and WDATAL
	PHA
	LDA WDATAH    ;         onto the stack they go!
	PHA
	LDA BCRASM    ; BCRASM contains NTRAIN in high four bits,
		      ;   and STRAIN in low four bits
	STA WDATAH    ; Send train numbers in high byte of return data
	LDA CONTRL
		AND #$FC          ; DL AND out 2 least significant bits i.e. track kill bits
		ORA YLTOGN    ; DL ORs the last two bits so Master Comp can see toggle
	STA WDATAL    ; Send light and track kill data back in low byte
	JMP CONT4     ; And send immediately - don't mess with other flags

CONTQ   LDA DFLAG
	BNE CONT2

	JSR ZERO

	LDA #$00

	STA INTMPU    ;  ********* CLEAR LATENT INTMPU FLAG!! ************

	STA OKWR      ;  ********* CLEAR LATENT OKWR FLAG!!!! ************

	JMP RDWR

CONT2   LDA INTMPU    ; Should we signal the MPU that we want to send data?

	CMP #$FF

	BNE CONT3     ; Branch if INTMPU not set

	JSR ZERO

	LDA BLKID

	ASL A

	TAX

	LDA #$90

	STA WBUF,X

	STA WBUF+1,X  ; Remember, we have to set two bits to signal MPU

	LDA #00

	STA INTMPU

	STA OKWR      ;  ********** CLEAR LATENT OKWR FLAG!!!! ***********

	BEQ RDWR

CONT3   LDA OKWR

	CMP #$FF

	BEQ CONT4     ; Branch if we can send data

	JSR ZERO

	JMP RDWR

CONT4   LDX #30       ; Decimal 30

JHG0    ROR WDATAH

	ROR WDATAL

	BCC JHG1

	LDA #$90      ; Load register A with  'data frame present, data EQU 1'

	BNE JHG2

JHG1    LDA #$80      ; Load register A with  'data frame present, data EQU 0'

JHG2    STA WBUF,X

	STA WBUF+1,X

	DEX

	DEX

	BPL JHG0

	LDA #$80

	STA WBUF+32   ; Set bit 17 to zero

	LDA #00

;

;               MORE NEW CODE FOR SENDING STATUS REPORTS

;

	LDY STFLG     ; Check status flag

	BNE ZSTAT     ; If it is set, then zero STFLG and don't mess with

;                          other flags

	STA DFLAG     ; Zero out DFLAG

	STA OKWR      ; Zero OKWR, too

	BEQ RDWR      ; Now go write the stuff during this data frame

ZSTAT   STA STFLG     ; Zero out the STFLG

	PLA           ; Restore WDATAH...

	STA WDATAH

	PLA           ; And WDATAL...

	STA WDATAL    ; To their correct places

	JMP RDWR      ; And send the status data

;

;       Procedure:   ZERO

;       Function :   Fill the output buffer with zero data bytes

;

ZERO    LDA #$80

	LDX #00

LP2     STA WBUF,X

	INX

	CPX #33

	BNE LP2

	RTS

;

;       Routine :   RDWR

;       Function:   Read controller data in while simultaneously writing

;                       return data out on PB4

;

RDWR    LDA VORB

	LDX #00

	JSR WTCB1

	LDA #$20

	BIT VIFR      ; Check to see if Timer2 has timed out...

	BEQ NEXTONE   ; Branch if Timer2 is still running...

	JMP PANIC2    ; Else Panic!!

NEXTONE LDA TIME

	STA VT2L      ; Set timer to intermediate value between '0' and '1'

	LDA #00

	STA VT2H      ; Zero high byte ofTimer2

	LDA WBUF      ; Get first byte of output buffer

	ORA #$01      ; Turn on PB0

	STA VORB      ; And output the byte

;

;               Start of the loop that actually does the reading and writing

;

RDLOOP  JSR WTCB1

	LDA WBUF+1,X

	ORA #$01

	STA VORB

	LDA VIFR

	STA TABLE,X

	INX

	LDA #00       ; Restart Timer2

	STA VT2H

	CPX #32

	BNE RDLOOP

	LDA #64

	STA VT2L      ; Set Timer2 to wait past any extra bits

	LDA #01

	STA VT2H      ; Set Timer2 for about 200 u-secs

	JSR WT2

;

;

;

	LDA VORB

	LDX #00

	LDY #00

	STY FTAB

OUTLOP  LDA #04

	STA LCV

	LDA #01

	STA MASK

INLOOP  LDA TABLE,X

	BIT T2TEST

	BEQ GZERO

	LDA FTAB,Y

	ORA MASK

	STA FTAB,Y

GZERO   INX

	ASL MASK

	DEC LCV

	BNE INLOOP

	INY

	LDA #00

	STA FTAB,Y

	CPY #$08

	BNE OUTLOP

	JSR PARITY

	CMP #$0F

	BNE RELAY     ; Exit if it is bad data

;                                               Going to relay because too long for a branch

;

;

	LDA FTAB+1

	ORA FTAB+2

	BNE RELAY     ; Going to relay because too long for a branch

	LDA FTAB+3    ; Load up block ID

	BNE SKIP          ; If not zero then see if return info for RDATA or

					  ;    if request for to send back WDATAHI and WDATALO

;

;               LAST NEW SECTION FOR SENDING STATUS DATA AUTOMATICALLY

;

	LDA FTAB+5    ; Get second nibble of task code

	BEQ STATCK    ; Branch if its a right arrow

	CMP #$08      ; Is it a left arrow ?          NOTE: NEW CODE !!!!

		BEQ LIGHT         ; If it is then test if switch light to green or toggle

;        BEQ GLOBPL    ; If it is then set INTMPU flag.   NOTE: NEW CODE !!!!

	BNE RELAY     ; Otherwise leave

 

LIGHT   LDA FTAB+4    ; Get first nibble of task code

		CMP #$01          ; Is it a one

		BEQ TGGLE         ; If it is a one then toggle YLTOGN

		CMP BLKID         ; Is it a request to set yellow light to green

		BEQ SETGFL

;               LDA FTAB+4    ; 

		LDA #$0F

		BNE CHCB2         ;

 

SETGFL  LDA YLTOGN        ; If it is then check state of YLTOGN and set light to grn

		CMP #$02          ; Check if YLTOGN is set to north light

		BEQ SETNFL        ; If it is then set north light to green

		LDA #$44

		STA LFLAG

		LDA #$0F

		STA SUTHGN

;               LDA CONTRL        ; If not, set south light to green

;               AND #$1D      ;

;               ORA #$60      ;

;               STA CONTRL    ;

RELAY   BNE CHCB2     ;

 

SETNFL  LDA #$33

		STA LFLAG

		LDA #$0F

		STA NRTHGN

;        LDA CONTRL       ; Set north light to green

;               AND #$E2      ;

;               ORA #$0C      ;

;               STA CONTRL    ;

		BNE CHCB2     ;

 

TGGLE   LDA YLTOGN    ; Load YLTOGN to check to present value

		CMP #$02          ; Is it set to north light

		BEQ TGGSTH        ; If it is toggle to south

		LDA #$02      ;

		STA YLTOGN    ;

		BNE CHCB2     ;

TGGSTH  LDA #$01          ; If set at north toggle to south

		STA YLTOGN    ;

		BNE CHCB2     ;

 

;GLOBPL  LDA #$FF      ; Set GLOBAL POLL flag

;        STA INTMPU    ;

;        BNE CHCB2     ; And scram

 

STATCK  LDA FTAB+4    ; Get first nibble of task code

		CMP #$01          ; Is it a 1

		BEQ GLOBPL        ; If it is then set INTMPU flag  DL NOTE: NEW CODE V.3

	CMP BLKID     ; Is it a status request?

	BNE CHCB2     ; If not, then leave

	LDA #01

	STA STFLG     ; If yes, then set STFLG EQU 1

	BNE CHCB2     ;

GLOBPL  LDA #$FF      ; Set GLOBAL POLL flag

	STA INTMPU    ;

	BNE CHCB2     ; And scram

;

;               END OF THIS NEW SECTION

 

;

SKIP    CMP BLKID

	BNE CHCB2

	LDA FTAB+5

	ASL A

	ASL A

	ASL A

	ASL A

	ORA FTAB+4

	BNE SKIP2

	LDA #$FF

	STA OKWR      ; If task code is '00' then set OKWR

	BNE CHCB2

SKIP2   STA RDATA

CHCB2   LDA #$08

	BIT VIFR      ; Have we got a CB2 interrupt?

	BEQ CHCB2     ; Not yet, so check again

	LDA #01

	STA FRANUM

	LDA POS

	ORA #$01

	STA VORB

	JMP RETURN

T2TEST DB $20

 

;

;               This is the start of the LOCCON routine

;

LOCCON  LDA VORB        ; Get bits from VIA B that signal data frame and

			;  power polarity to user and save them

	AND #$F0        ;  but first mask out low bits

	STA CBITS

	PHP             ; Save carry bit for first ROL operation

	ORA #$09        ; Send out a clock pulse to bar code reader

	STA VORB        ; Outputs the pulse

	AND #$F7        ; End pulse

	STA VORB

	LDX #00         ; Set bit counter to zero

IOLOOP  ASL BCRASM      ; Shift bar code in left

	LDA CBITS       ; Take clock low

	STA VORB

	LDA VORB        ; Data to read now available - read it

	AND #$04        ; Zero out output and communications bits

	CMP #$04        ; Is it a one?

	BNE ZEROIN      ; If not, goto ZEROIN

	INC BCRASM      ; If so, set low bit of BCRASM = 1

ZEROIN  LDY CBITS       ; Set up Y to output a zero

	LDA CONTRL      ; Get control bits

	BPL ZEROUT      ; If high bit is a zero, goto ZEROUT

	INY             ;   else set up for a one output

	INY             ;   by setting second bit = 1

ZEROUT  STY VORB        ; Dump out Y register

	PLP             ; Restore carry bit

	ROL CONTRL      ; Move next bit of CONTRL into position

	PHP             ; Save new carry bit

	INY             ; Same data as above in Y, now increment for clock

	STY VORB        ; Take data clock high with same output data

	INX             ; Increment bit counter

	CPX #$08         ; 8 bits of I/O?

	BNE IOLOOP      ; No, so send and get another

	PLP             ; If yes, clear stack

	ROL CONTRL      ; CONTRL is back in its entry state

CONVRT  LDA BCRASM      ; Get north train number

	LSR A

	LSR A           ; Shift it into the four lower bits

	LSR A

	LSR A

	CMP NASSEM      ; Compare to last read train number

	BEQ SAME1       ; If same, goto SAME1

	STA NASSEM      ; Else save in NASSEM for next compare

	LDA #00

	STA NCOUNT      ; Reset NCOUNT

	BEQ SOUTH       ; NOTE BENE:  This is changed from a JMP instruction

;

SAME1   INC NCOUNT      ; We have one more consecutive read

	LDA #06         ; Have we read it 6 times (1/4+ second) ?

	CMP NCOUNT

	BNE SOUTH       ; If not, do south routine

	LDA NASSEM      ; Otherwise, put NASSEM in NTRAIN

	STA NTRAIN

	LDA #00

	STA NCOUNT      ; Reset NCOUNT

;

SOUTH   LDA BCRASM

	AND #$0F        ; Clear NTRAIN bits out - we just want STRAIN

	CMP SASSEM

	BEQ SAME2

	STA SASSEM

	LDA #00

	STA SCOUNT

		BEQ NTCK

;        BEQ HSKEEP

SAME2   INC SCOUNT

	LDA #06

	CMP SCOUNT

		BNE NTCK

;        BNE HSKEEP

	LDA SASSEM

	STA STRAIN

	LDA #00

	STA SCOUNT

;

;       Old housekeeping routine for side tracks in version 2.0

;

;HSKEEP  LDA SIDTR       ; Control routine - is the siding clear?

;        BEQ NTCK        ; If so, go to NTCK

;        LDA #01         ; Else, check north caution bit

;        BIT CAUT

;        BEQ DECDEL      ; If clear, goto DECDEL

;        LDA CONTRL

;        AND #$E2        ;

;        ORA #$14        ; Otherwise, set north yellow light, north track on

;        STA CONTRL      ;

;        JMP STCK

;

;DECDEL  LDA CONTRL      ;

;        AND #$E2        ; Set north track red light, kill the track

;        ORA #$19        ;

;        STA CONTRL

;        DEC CLRDEL      ; Decrement assured clear track delay

;        BNE STCK        ; If delay not over, go to south routine

;        LDA SIDTR       ; Else get train number on siding...

;        STA LEAVTR      ; And note it as being on the mainline north

;        LDA #00

;        STA SIDTR       ; Zero siding train to signal students

;        LDA #$FF

;        STA RELDEL      ; Set maximum keep clear delay

;        BNE STCK        ; And go to south routine

;

NTCK    LDA NOVRD       ; *** V 3.1 ***

	BNE NRDOFF      ; *** V 3.1 ***  PROTECT THE BLOCK

	LDA NTRAIN      ; Anything on north track ?

		BNE NRDOFF              ; If so, go set north light red

;        BNE RESCNT      ; If so, go reset count (RESCNT)

;        LDA LEAVTR      ; Anything cleared to leave siding ?

;        BNE OUTDEL      ; If yes, go to OUTDEL

		LDA NRTHGN

		BNE SETNGN

	LDA CONTRL

	AND #$E2        ; DL new code

		ORA #$14                ; Otherwise turn on north yellow light, track power

;        ORA #$0C        ; Otherwise turn on north green light and track power

	STA CONTRL      ;

	JMP STCK

 

SETNGN  LDA CONTRL        ; Set north light to green

		AND #$E2      ;

		ORA #$0C      ;

		STA CONTRL    ;

		JMP STCK      ;

 

;

;       Old code to set delays for leaving side track  version 2.0

;

;RESCNT  LDA #44         ; Set up 2 second delay to insure clear track

;        STA CLRDEL

;        LDA #01         ; Check north caution bit

;        BIT CAUT

;        BEQ NRDOFF      ; If not set, go to NRDOFF

;        LDA NTRAIN      ; Otherwise, the train goes on the siding

;        STA SIDTR

;        LDA #00         ; And the north track may be used under caution

;        STA NTRAIN

;        LDA CONTRL      ; Set north caution light, track on

;        AND #$E2

;        ORA #$14

;        STA CONTRL

;        JMP STCK

;

;OUTDEL  DEC RELDEL      ; Decrement delay count for track held clear

;        BNE NRDOFF      ; If not timed out, go to NRDOFF

;        LDA #00         ; Otherwise, zero out LEAVTR

;        STA LEAVTR

;        BEQ STCK

 

NRDOFF  LDA CONTRL      ;

	AND #$E2        ; Set north red light and turn off track

	ORA #$19        ;

	STA CONTRL

		LDA #$00

		STA NRTHGN

 

STCK    LDA SOVRD       ; *** V 3.1 ***

	BNE SRDOFF      ; *** V 3.1 ***  PROTECT THE BLOCK

	LDA STRAIN      ; Get south train number

		BEQ YELON

;        BEQ GRON        ; If zero (no train) go to GRON

;        LDA #02

;        BIT CAUT        ; Check south caution bit

;        BNE YELON       ; If set, turn south yellow light on

SRDOFF  LDA CONTRL      ;

	AND #$1D        ; Else set south read light, turn track off

	ORA #$C2        ;

	STA CONTRL

		LDA #$00

		STA SUTHGN

		RTS

;        JMP FINI

 

YELON   LDA SUTHGN

		BNE SETSGN

		LDA CONTRL

	AND #$1D        ;

	ORA #$A0        ; Set south yellow light, track on

	STA CONTRL      ;

		RTS

;        JMP FINI

 

SETSGN  LDA CONTRL        ; If not, set south light to green

		AND #$1D      ;

		ORA #$60      ;

		STA CONTRL    ;

		RTS

 

;GRON    LDA CONTRL

;        AND #$1D        ;

;        ORA #$60        ; Set south green light, track on

;        STA CONTRL      ;

 

;FINI    LDA SIDTR       ; Move SIDTR to STSDTR for student use

;        STA STSDTR

;        RTS

 

	ORG $FFFC       ; Set interrupt vectors

	DW $E000        ; Set RESET vector

	DW ISERV ; Set IRQ vector

	END





