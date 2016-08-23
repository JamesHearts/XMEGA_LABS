;
; lab2a.asm
;
; Created: 5/28/2016 10:17:41 PM
; Author : James Mak
;

.include "ATxmega128A1Udef.inc"

.org 0x0000

			rjmp main

.org 0x0100

main:		
		
		ldi R16, 0xFF				;Load R16 with FF.
		sts PORTE_DIRSET, R16		;Set GPIO's in four bit PORTE as outputs.
		ldi R16, 0xFF				;Load R16 with 01.
		sts PORTE_OUT, R16			;Set the output of PortD pin 1.
		
end:	
		rjmp end					;Endless loop.  
		


