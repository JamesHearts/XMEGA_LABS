;
; lab1.asm
;
; Created: 5/23/2016 5:32:12 PM
; Author : James Mak
;


; Replace with your application code

.nolist
.include "ATxmega128A1Udef.inc"

.equ twelve = 0x0C          ;Constant 12.
.equ null  = 0x00           ;Null character.
.equ test1 = 0x6E			;First test variable.
.equ test2 = 0x48		    ;Secon test variable.
.equ table_size = 21		;Intialize the table as 21.
.def counter = r16			;Counter will be stored in register 16.
.def new_counter = r17		;Counter will store the size of the new filtered table in r17.
.org 0x0000					;This is where our program will start.
	rjmp main				;Jump to start of program.

.org 0x2A18					;Table location in memory.

table: .db 0x48, 0x6E, 0x5C, 0x55, 0x2B, 0x68, 0x53, 0x6B, 0x79, 0x55, 0x67, 0x45, 0x53, 0x34, 0x59, 0x55, 0x37, 0x67, 0x6D, 0x22, 0x00 ;Define the table, .db means bytes.

.dseg					    ;Data Segment for variables.

.org 0x3701			        ;This is where our new table will be located.

new_table: .byte table_size ;We reserve table size that is the same size as our original table for our new filtered table.

.cseg					    ;Code Segment.

.org 0x0200



MAIN:		ldi ZL, low(table <<1)		;Load the low byte of the table address.
			ldi ZH, high(table <<1)		;Load the high byte of the table address.
			ldi YL, low(new_table)      ;Load the low byte of the new table location (3701).
			ldi YH, high(new_table)     ;Load the high byte of the new table location (3701).
										;It is noteable that you cannot use ldi with r0 - r15.

LOOP:		lpm r18, Z+                 ;Loading the value at Z into r18 and then incrementing the pointer.
			cpi r18, null               ;Compare r18 with the NULL character to see if the program should end.
			breq END_LOOP               ;Branch to the end of the LOOP if we are done.
			cpi r18, test1              ;Compare r18 with test1(6E).
			brsh LOOP                   ;If it is not less than test1(6E) we repeat the process.
			cpi r18, test2              ;Compare r18 with test2(48).
			brlo LOOP                   ;If it is not greaer than test2(48) we repeat the process.
			ldi r19, twelve             ;Load the decimal value of 12 into register r19.
			add  r18, r19    			;Add the decimal value of 12 to the number.
			st Y+, r18                  ;Store the value where Y is pointed to.
			inc new_counter             ;Add one to the new_counter since we tested the value is less than 6E.
			rjmp LOOP


END_LOOP:	inc new_counter				;Account for the null character.
			st Y+, r18                  ;Store the null character.
DONE:
			rjmp DONE					;Infinite Loop.