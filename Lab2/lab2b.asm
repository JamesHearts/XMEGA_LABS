;
; lab2b.asm
;
; Created: 5/29/2016 7:32:19 PM
; Author : James Mak
;

.include "ATxmega128A1Udef.inc"

.equ stack_init = 0x2FFF			;Initialize stack pointer.		
.equ even = 0x55					;Even numbered bits on.
.equ odd = 0xAA						;Odd numbered bits on.
.equ delay_counter = 125           ;Because the delay takes 3 instructions 250/3 is 166. 
.equ zero = 0x00                    ;Zero constant.
.equ ones = 0xFF					;FF for dir_set.

.org 0x0000
	rjmp main

.org 0x200

main:		
				ldi YL, low(stack_init)		;Initialize low byte of stack pointer.
				out CPU_SPL, YL             
				ldi YL, high(stack_init)    ;Initialize high byte of stack pointer.
				out CPU_SPH, YL             ;We can use the same register YL for both high and low.

				ldi R16, even               ;Store even in R16.
				ldi R17, odd                ;Store odd in R17.
				ldi R18, delay_counter      ;Store delay_counter into  R18.
				ldi R19, ones               ;Store FF into R19.
				ldi R20, zero               ;Store zero into R20.
				sts PORTE_DIRSET, R19       ;Set the directon bits of PortE to output.


				

loop:			
			
				sts PORTE_OUT, R16          ;Turn on Even LEDs.
				rcall DELAY_250us           ;Delay for 2Khz. Call the delay subroutine.
     		    sts PORTE_OUT, R17          ;Turn on Odd LEDs.
				rcall DELAY_250us           ;Delay for 2Khz. Call the delay subroutine.
				rjmp loop                  ;Endless repetitive loop.

				
DELAY_250us:								;500 us delay divided by two for 50% duty cycle.

				push R18                    ;Push R18 onto stack for future use.
delay_loop:		dec R18                     ;Decrement R18.
				cpi R18, zero				;Compare R18 with zero.
				brne delay_loop             ;If not equal to zero loop.
				pop R18                     ;Pop R18 so we can use it again.
				ret 


			