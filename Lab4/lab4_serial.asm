;
; lab4c.asm
;
; Created: 7/4/2016 3:54:55 PM
; Author : James Mak
;


.include "ATxmega128A1Udef.inc"

.equ stack_init = 0x3FFF
.equ bsel = 107
.equ bscale = -5

.dseg

Input_String: .byte 10
.cseg
 
.org 0x0000
	rjmp MAIN

.org 0x0100

String: .db "Hello World", 0x0D, 0x0A, 0x00

MAIN:
					ldi YL, high(stack_init)	;Initialize the stack pointer.
					sts CPU_SPH, YL
					ldi YL, low(stack_init)
					sts CPU_SPL, YL
					rcall USART_INIT			;Initialize the USART.

					ldi ZL, low(String << 1)	;Initialize Z pointer to read data.
					ldi ZH, high(String << 1)
					ldi XL, low(Input_String)	;Initialize X pointer to store data.
					ldi XH, high(Input_String)
					

					;ldi R16, 'a'
START:				rcall IN_CHAR
					cpi R16, 'S'				;If character is 'S' then start Output.
					brne START
					;rcall CHAR_OUT
					rcall OUT_STRING
REPEAT:					
     				rjmp REPEAT					;Endless loop of outputting the same table.

					

USART_INIT:
					ldi R16, 0x0A
					sts PORTQ_DIRSET, R16		;Set Port Q pins 1 and 3 to output.
					sts	PORTQ_OUTCLR, R16		;Output Defaul 0 to pins 1 and 3 of Port Q.
					ldi R16, 0x08				;Set Pin 3 of Port D to output (Tx).
					sts PORTD_DIRSET, R16		
					sts PORTD_OUTSET, R16		;Set the default value as 1.
					ldi R16, 0x04
					sts PORTD_DIRCLR, R16		;Set RX pin as input.
					ldi R16, 0x18				
					sts USARTD0_CTRLB, R16		;Turn on the transmitter (Tx) and receiver (Rx)
					ldi R16, 0x03
					sts USARTD0_CTRLC, R16		;Async Mode, No Parity, 1 Stop bit, 8-bit Frame.
					ldi R16, (bsel & 0xFF)		;Lower 8-bits of the 12-bit bsel value in register BAUDCTRLA
					sts USARTD0_BAUDCTRLA, R16
					ldi R16, ((bscale << 4) & 0xF0) | ((bsel >>  8) & 0x0F)
					sts USARTD0_BAUDCTRLB, R16	;Most signification 4 bits of bsel value in the least significant 4 bits of BAUDCTRLB.
					ret							;

CHAR_OUT:
					push R17
TX_POLL:			
					lds R17, USARTD0_STATUS		;Load the USART status register.
					sbrs R17, 5					;If the DREIF flag (Bit 5) is set then send to out or else wait until set.
					rjmp TX_POLL
					sts USARTD0_DATA, R16		;Send the character in R16 out.
					pop R17
					ret

IN_CHAR:

RX_POLL:
					lds R17, USARTD0_STATUS		;Load the USART status register.
					sbrs R17, 7					;If the Rx Flag is set then send to in or else wait until set.
					rjmp RX_POLL
					lds R16, USARTD0_DATA		;Read the character into R16.
					ret




					
OUT_STRING:
					lpm R16, Z+					;Load a value from string table to R16.
					cpi R16, 0x00				;If the character is NULL then end the output of the string.
					breq END_OUT
					rcall CHAR_OUT				;Output the next character
					rjmp OUT_STRING				;Repeat until NULL character is reached.
END_OUT:    		ret
							

IN_STRING:
					rcall IN_CHAR				;Read the input
					cpi R16, 0x00				;If the character is null then end the reading of the string.
					breq END_IN
					st X+, R16					;Else store the character in the location X is pointed to.
					rjmp IN_STRING				;Repeat until NULL character is reached.
END_IN:				ret
					