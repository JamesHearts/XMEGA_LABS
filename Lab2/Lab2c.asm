;
; lab2c.asm
;
; Created: 5/30/2016 3:59:22 PM
; Author : James Mak
;


.include "ATxmega128A1Udef.inc"

;My keypad will have the rows as low and the columns as high.

.equ row1 = 0b0111	     	;This turns on the first row for scanning.
.equ row2 = 0b1011			;This turns on the second row for scanning.
.equ row3 = 0b1101			;This turns on the third row for scanning.
.equ row4 = 0b1110			;This turns on the fourth row for scanning.
.equ config = 0x0F      	;This is the configuration used for DIR_SET.
.equ stack_init = 0x2FFF    ;This is where we'll place our stack.
.equ pull_up = 0x18       ;The configuration bits for pull-up resistor.
.equ table_size = 200       ;Reserve 200 bytes of memory for data.
.equ no_press = 0x07		;Zero.

.org 0x100                  ;We want to place a table here.

table: .db 0x78, 0x74, 0x72, 0x71, 0xB8, 0xB4, 0xB2, 0xB1, 0xD8, 0xD4, 0xD2, 0xD1, 0xE8, 0xE4, 0xE2, 0xE1 ;This table represents the combinations of a keypad button.
key:   .db 0x1, 0x2, 0x3, 0xA, 0x4, 0x5, 0x6, 0xB, 0x7, 0x8, 0x9, 0xC, 0xE, 0x0, 0xF, 0xD ;This is the key that the above table corresponds to.

.dseg 
.org 0x2000				;This is where our outputs will go.

out_table: .byte table_size		;Reserve some space for data.


.cseg
.org 0x0000
			rjmp MAIN

.org 0x0200

MAIN:	
				ldi XL, low(out_table)          ;Load the low byte of the new table location (2FFF).
				ldi XH, high(out_table)         ;Load the high byte of the new table location (2FFF).
				ldi R16, config					;Load the DIR_SET configuration 0x0F.
				sts PORTF_DIRSET, R16	     	;Configure the I/O pins of Port F.
				ldi R16, pull_up				;Load the pull-up resistor configuration.
				sts PORTF_PIN7CTRL, R16		    ;Set pull-up resistor to pin 7.
				sts PORTF_PIN6CTRL, R16			;Set pull-up resistor to pin 6.
				sts PORTF_PIN5CTRL, R16			;Set pull-up resistor to pin 5.
				sts PORTF_PIN4CTRL, R16		    ;Set pull-up resistor to pin 4.
				ldi YL, low(stack_init)         ;Load the low byte of the stack pointer.
				out CPU_SPL, YL                 ;Initialize the low byte of the stack pointer.
				ldi YL, high(stack_init)        ;Load the high byte of the stack pointer.
				out CPU_SPH, YL					;Initialize the high byte of the stack pointer.
				
				      
LOOP:
				rcall SCAN                      ;Call the SCAN subroutine.
				rjmp LOOP                       ;Repeat Infinitely.

SCAN:			

ROW:			ldi R16, row1                   ;Load row 1 into R16.
				sts PORTF_OUT, R16              ;Turn row 1 on.
				lds R18, PORTF_IN               ;Load Input values from Port F into R18.
				nop
				cpi R18, 0xF7                   ;Check to see if a key was pressed.
				brne COL                        ;If a key is pressed branch to see what the column is.
				ldi R16, row2                   ;Load row 2 into R16.
				sts PORTF_OUT, R16              ;Turn row 2 on.
				lds R18, PORTF_IN               ;Load Input values from Port F into R18.
				nop
				cpi R18, 0xFB                   ;Check to see if a key was pressed.
				brne COL                        ;If a key is pressed branch to see what the column is.
				ldi R16, row3                   ;Load row 3 into R16.
				sts PORTF_OUT, R16              ;Turn row 3 on.
				lds R18, PORTF_IN               ;Load Input values from Port F into R18.
				nop
				cpi R18, 0xFD                   ;Check to see if a key was pressed.
				brne COL                        ;If a key is pressed branch to see what the column is.
				ldi R16, row4                   ;Load row 4 into R16.
				sts PORTF_OUT, R16              ;Turn row 4 on.
				lds R18, PORTF_IN               ;Load Input values from Port F into R18.
				nop
				cpi R18, 0xFE               ;Check to see if a key was pressed.
				brne COL                        ;If a key is pressed branch to see what the column is.
				rjmp ROW                        ;Repeat if no key is pressed.

COL:
				ldi ZL, low(key << 1)			;Load the low byte of table location.
				ldi ZH, high(key << 1)          ;Load the high byte of the table location.
       
OUTPUT:			
				cpi R18, 0x77                   ;Check to see if 1 is pressed.
				lpm R19, Z+                     ;Access Key.
				breq WRITE
				cpi R18, 0xB7					;Check to see if 2 is pressed.
				lpm R19, Z+                     ;Access Key.
				breq WRITE                
				cpi R18, 0xD7                   ;Check to see if 3 i pressed.
				lpm R19, Z+                     ;Access Key.
				breq WRITE                    
				cpi R18, 0xE7                   ;Check to see if A is pressed.
				lpm R19, Z+                     ;Access Key.
				breq WRITE 
				cpi R18, 0x7B                   ;Check to see if 4 is pressed.
				lpm R19, Z+                     ;Access Key.
				breq WRITE 
				cpi R18, 0xBB                   ;Check to see if 5 is pressed.
				lpm R19, Z+                     ;Access Key.
				breq WRITE 
				cpi R18, 0xDB                   ;Check to see if 6 is pressed.
				lpm R19, Z+                     ;Access Key.
				breq WRITE 
				cpi R18, 0xEB                   ;Check to see if B is pressed.
				lpm R19, Z+                     ;Access Key.
				breq WRITE 
				cpi R18, 0x7D                   ;Check to see if 7 is pressed.
				lpm R19, Z+                     ;Access Key.
				breq WRITE 
				cpi R18, 0xBD                   ;Check to see if 8 is pressed.
				lpm R19, Z+                     ;Access Key.
				breq WRITE 
				cpi R18, 0xDD                   ;Check to see if 9 is pressed.
				lpm R19, Z+                     ;Access Key.
				breq WRITE 
				cpi R18, 0xED                   ;Check to see if C is pressed.
				lpm R19, Z+                     ;Access Key.
				breq WRITE 
				cpi R18, 0x7E                   ;Check to see if E is pressed.
				lpm R19, Z+                     ;Access Key.
				breq WRITE 
				cpi R18, 0xBE                   ;Check to see if 0 is pressed.
				lpm R19, Z+                     ;Access Key.
				breq WRITE 
				cpi R18, 0xDE                   ;Check to see if F is pressed.
				lpm R19, Z+                     ;Access Key.
				breq WRITE 
				cpi R18, 0xEE                   ;Check to see if D is pressed.
				lpm R19, Z+                     ;Access Key.
				breq WRITE 
WRITE:	
				st X+, R19                      ;Store E if it is equal.
				ret                             ;Return to start of subroutine.	