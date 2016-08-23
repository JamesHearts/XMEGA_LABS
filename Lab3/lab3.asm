;
; lab3.asm
;
; Created: 6/13/2016 6:28:22 PM
; Author : James Mak
;


.include "ATxmega128A1Udef.inc"

.set Port_Start = 0x8000					;Base address of our expansion port.
.set Port_End = 0xBFFF						;Ending address of our expansion port.
.equ stack_init = 0x2FFF			;Initialize stack pointer.


.org 0x0000 
				rjmp MAIN					;Jump to the start of our main program.

.org 0x0200

MAIN:
				ldi YL, low(stack_init)			;Initialize low byte of stack pointer.
				out CPU_SPL, YL             
				ldi YL, high(stack_init)	    ;Initialize high byte of stack pointer.
				out CPU_SPH, YL			        ;We can use the same register YL for both high and low.

				ldi R16, 0b10111				;Bit 4 = CS0(L), Bit 2 = ALE1(H), Bit 1 = RE(L), Bit 0 = WE(L)
				sts PORTH_DIRSET, R16			;Configure PortH setting bits 4,2,1,0 as output bits.
				
				ldi R16, 0b10011				;The manual says that active low pins should be set to 1 and that active high pins should be set to 0.  
				sts PORTH_OUTSET, R16			;Output 1 at bits 4,1,0 (active low pins).

				ldi R16, 0xFF					;Bit configurations for port K. (A15-A0)
				sts PORTK_DIRSET, R16			;Set all Port K pins to output pins.

				ldi R16, 0xFF					;Bit configurations for port J. (D7-D0)
				sts PORTJ_DIRSET, R16			;Set all Port J pins to output pins.

				ldi R16, 0x01					;Configuration bits for EBI mode.
				sts EBI_CTRL, R16				;Set IFMODE (EBI mode) to 3-Port Configuration.

				ldi ZH, high(EBI_CS0_BASEADDR)  ;Point High bytes of Z register to point to base address register for CS0.
				ldi ZL, low(EBI_CS0_BASEADDR)	;Point Low bytes of Z register to point to base address register for CS0.

				ldi R16, byte2(Port_Start)      ;Load the second byte of Starting address 0x8000, Byte 2 = 0x80.
				st  Z+, R16						;Store Byte 2 into base address register.

				ldi R16, byte3(Port_Start)		;Load the third byte of the starting address.
				st  Z, R16						;Store Byte 3 into base address register.

				ldi R16, 0x19					;16K chip select space 0x8000 - 0xBFFF. SRAM mode.
				sts EBI_CS0_CTRLA, R16

				ldi R16, byte3(Port_Start)		;Load the third byte of Starting address to R16.
				sts CPU_RAMPX, R16

				ldi XH, high(Port_Start)		;Point X register to high bytes of starting address.
				ldi XL, low(Port_start)			;Point X register to low bytes of starting address.

TEST:
				ld R16, X						;Test input port to R16.
				nop
				st X, R16						;Output to LED.	

				ldi R17, 0x08					;Load counter value 8 to R19.
SHIFT_LEFT:	
				sbrc R16, 7			            ;If the most significant bit of r16 is a 1 then set the carry to one.
				sec
				rol R16							;Rotate R16 left.
				rcall DELAY						;Wait .25 seconds.
				rcall DELAY						;Wait .25 seconds.
				st X, R16						;Display shifted number to LEDs.
				dec R17							;Decrement the counter.
				cpi R17, 0x00					;Is the counter zero?
				brne SHIFT_LEFT					;Do the shift 8 times.
				ldi R17, 0x08					;Reset counter.
SHIFT_RIGHT:	
				sbrc R16, 0						;If bit 0 is a 0 skip next instruction.
				sec 
				ror R16							;Rotate R16 right.
				rcall DELAY						;Wait .25 seconds.
				rcall DELAY						;Wait .25 seconds.
				rcall DELAY						;Wait .25 seconds.
				rcall DELAY						;Wait .25 seconds.
				st X, R16						;Display shifted number to LEDs.
				dec R17							;Decrement the counter.
				cpi R17, 0x00					;Is the counter zero?
				brne SHIFT_RIGHT				;Do the shift 8 times.
				rjmp TEST						;Repeat after 8 times.

				

DELAY:											;500 us delay divided by two for 50% duty cycle.
				ldi R18, 125		     		;125 cycles for .25ms of Delay. 125 x 500 for .5s of Delay.
				ldi R19, 250					;250x125 for .25s of Delay.
delay_250:
				dec R19                         ;Decrement R19.
				cpi R19, 0x00		     		;Compare R19 with zero.
				brne delay_125                  ;If not equal to zero,loop.
				ret
delay_125:		
				dec R18                         ;Decrement R18.
				cpi R18, 0x00		     		;Compare R18 with zero.
				brne delay_125                  ;If not equal to zero loop.
				rjmp delay_250