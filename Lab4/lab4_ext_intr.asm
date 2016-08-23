;
; lab4a.asm
;
; Created: 7/3/2016 7:22:06 PM
; Author : James Mak
;


.include "ATxmega128A1Udef.inc"

.equ stack_init = 0x3FFF
.equ Port_Start = 0x8000

.org 0x0000
	rjmp MAIN

.org PORTF_INT0_VECT
	jmp ISR_LED_COUNT 

.org 0x100

MAIN:	
				ldi YL, high(stack_init)
				out CPU_SPH, YL					;Initialize the high byte of the stack pointer.
				ldi YL, low(stack_init)
				out CPU_SPL, YL					;Initialize the low byte of the stack pointer.

				ldi R16, 0xFF
				sts PORTE_DIRSET, R16

				rcall INIT_INT					;Call the routine to intialize the interrupt.
				nop
				ldi r17, 0x00						;Initialize the interrupt count.

LOOP:											;Endless loop while waiting for interrupt trigger.
				rjmp LOOP		


INIT_INT:	
				ldi R16, 0x01		
				sts PORTF_INT0MASK, R16			;Interrupt to trigger from PIN 0 Port F.
				sts PORTF_OUT, R16				;Output Default '1' from PIN 0 Port F.
				sts PORTF_DIRCLR, R16			;Set PIN0 Port F to input.
				sts PORTF_INTCTRL, R16			;Set the interrupt level to low level.
				sts PORTF_PIN0CTRL, R16			;Set the interrupt to trigger on a rising edge.
				sts PMIC_CTRL, R16				;Turn on low-level interrupts.

				sei								;Turn on global interrupt flag.
				ret


ISR_LED_COUNT: 
				push R19
				lds R19, CPU_SREG				;Push the status register onto the stack.
				push R19
				call DELAY_p5ms
				nop
				lds R18, PORTF_IN				;Read port E data.
				SBRC R18, 0						;If bit 0 is cleared skip the next instruction.
				inc R17							;Increment R16 if the bit 0 was set.
				sts PORTE_OUT, R17		        ;Output to LEDs.
				nop
				sts PORTF_INTFLAGS, R16			;Clear the interrupt flag.
				pop R19
				sts CPU_SREG, R19
				pop R19
				reti
				
DELAY_p5ms:		;Delay assumes each instruction is .5uS long and also that the entire delay routine is 10 instructions long. (5uS)							
				ldi R21, 4						;.125ms x 4 = .5ms
DELAY_LOOP:	    ldi R20, 50						;25 cycles. .25uS x 50  = .125ms
DELAY_p15ms:	dec R20
				nop
				nop
				nop
				brne DELAY_p15ms
				dec R21
				brne DELAY_LOOP
				ret



