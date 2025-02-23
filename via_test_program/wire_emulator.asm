(program to turn AIM 65 into a wire)

 
  ROCKWELL AIM 65

(enter program)

<*>=0200

<I>
 0200    LDA #FF    A9 FF
 0202    STA A002    8D 02 A0     (A002 is the port B direction register)
 0205    LDA #00    A9 00
 0207    STA A003    8D 03 A0     (A003 is the port A direction register)
 020A    LDA A001    AD 01 A0     (A001 is the port A data register)
 020D    STA A000    8D 00 A0     (A000 is the port B data register)
 0210    JMP 020A    4C 0A 02
 0213

(the above program:
	 sets all 8 lines of port B as output
         sets all 8 lines of port A as input
	 loops on read of port A and write of port B
	 each pin of port A is mapped to each pin of port B)

(run the program)

<*>=0200

<G>/

(notice that the cursor does not return, because the program never ends)

(to test the program, apply a logic signal to port Ax and check that the 
logic signal appears at port Bx -- where x is 0,1,2,...,7)

(to recover control of the computer hit reset on the motherboard)

