;
; lab4e.asm
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

String: .db "James' Favorite :", 0x0D, 0x0A, "1. Book", 0x0D, 0x0A, "2. Actor,Actress,Reality Star", 0x0D, 0x0A, "3. IceCream,Yogurt Flavor", 0x0D, 0x0A, "4. Food", 0x0D, 0x0A, "5. Pizza Topping", 0x0D, 0x0A, "6. Redisplay Menu", 0x0D, 0x0A, "ESC: Exit", 0x0D, 0x0A, 0x00
End_Words: .db "Goodbye!", 0x0D, 0x0A, 0x00

Ans1: .db "James' favorite book is, I don't read.", 0x0D, 0x0A, 0x00
Ans2: .db "James' favorite Actor/Actress/Reality Star, don't have one", 0x0D, 0x0A, 0x00
Ans3: .db "James' favorite IceCream/Yogurt Flavor is coconut!", 0x0D, 0x0A, 0x00
Ans4: .db "James' favorite Food is homecooked!", 0x0D, 0x0A, 0x00
Ans5: .db "James' favorite Pizza Topping is peperonis!, duh", 0x0D, 0x0A, 0x00

MAIN:
					ldi YL, high(stack_init)	;Initialize the stack pointer.
					sts CPU_SPH, YL
					ldi YL, low(stack_init)
					sts CPU_SPL, YL
					rcall USART_INIT			;Initialize the USART.

					
					ldi XL, low(Input_String)	;Initialize X pointer to store data.
					ldi XH, high(Input_String)
					

					
START:				rcall IN_CHAR
					cpi R16, 'S'				;If character is 'S' then start Output.
					brne START
					
MENU:				ldi ZL, low(String << 1)	;Initialize Z pointer to read data.
					ldi ZH, high(String << 1)
					rcall OUT_STRING			;Display the Menu.
REPEAT:				rcall IN_CHAR
					cpi R16, 0x00				;If there is nothing in the Input the repeat. (Just so we don't get an input we don't want).
					breq REPEAT
					cpi R16, 0x31				;Check to see if first choice was chosen. Then the next, it will run down the menu.
					brne CHOICE_2
					ldi ZL, low(Ans1 << 1)
					ldi ZH, high(Ans1 << 1)
					rcall OUT_STRING
					rjmp REPEAT					;Go back to the top when an answer is output.
CHOICE_2:			cpi R16, 0x32
					brne CHOICE_3			
					ldi ZL, low(Ans2 << 1)
					ldi ZH, high(Ans2 << 1)
					rcall OUT_STRING
					rjmp REPEAT
CHOICE_3:			cpi R16, 0x33
					brne CHOICE_4
					ldi ZL, low(Ans3 << 1)
					ldi ZH, high(Ans3 << 1)
					rcall OUT_STRING
					rjmp REPEAT
CHOICE_4:			cpi R16, 0x34
					brne CHOICE_5
					ldi ZL, low(Ans4 << 1)
					ldi ZH, high(Ans4 << 1)
					rcall OUT_STRING
					rjmp REPEAT
CHOICE_5:			cpi R16, 0x35
					brne CHOICE_6
					ldi ZL, low(Ans5 << 1)
					ldi ZH, high(Ans5 << 1)
					rcall OUT_STRING
					rjmp REPEAT
CHOICE_6:			cpi R16, 0x36
					brne CHOICE_ESC
					rjmp MENU
CHOICE_ESC:			cpi R16, 0x1B
					brne REPEAT
					ldi ZL, low(End_Words)
					ldi ZH, high(End_Words)
					rcall OUT_STRING

					rjmp START
													;Endless loop of outputting the same table.

					

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
					
