/*
 * Lab6a.c
 *
 * Created: 7/18/2016 4:03:04 PM
 * Author : James Mak
 */ 

#include <avr/io.h>
#include "ebi_driver.h"
#include <string.h>
#define F_CPU 2000000
#define CS0_Start 0x8000
#define CS1_Start 0x420000
#define LCD_Start 0x422000
#define LCD_Write 0x422001

void TC_INIT(void)
{
	//We are using Port E TC because it is the only port available that isn't taken.
	PORTE_DIRSET = 0x01; //Set Pin 1 to output.
	TCE0.CTRLA = TC_CLKSEL_OFF_gc;  //Turn of the Counter.
	TCE0.CTRLB = TC_WGMODE_FRQ_gc | TC0_CCAEN_bm;  //Waveform FRQ generation mode and Compare enable A set.
	TCE0.CTRLE = TC_BYTEM_NORMAL_gc; //Normal mode TC.
	TCE0.CTRLFCLR = TC0_DIR_bm;  //Incrementing Counter.
	TCE0.CNT = 0; //Start count at zero.
	
}

void Delay_XuS(int a) // Delay a certain amount of uS.
{
	for(int i = 0; i < a; i++);
}

void BF_POLL(void)
{
	uint8_t BF = __far_mem_read(LCD_Start);
	
	while((BF & 0x80) == 0x80) //Poll the busy flag (bit 7 until it is clear.
	{
		BF = __far_mem_read(LCD_Start);
	}
}

void LCD_INIT(void) // LCD_INIT from lab 5.
{
	__far_mem_write(LCD_Start, 0x38); //Configure function.
	BF_POLL();
	__far_mem_write(LCD_Start, 0x0F); //Configure display.
	BF_POLL();
	__far_mem_write(LCD_Start, 0x01); //Clear display.
	BF_POLL();
	__far_mem_write(LCD_Start, 0x06); //Configure entry mode.
	BF_POLL();
}

void OUT_CHAR(char character) // OUT_CHAR from lab 5.
{
	__far_mem_write(LCD_Write, character);
	BF_POLL();
	Delay_XuS(400);
}

void OUT_STRING(char str[]) //Store the characters in strings. OUT_STRING from lab 5.
{
	__far_mem_write(LCD_Start, 0x01);	// Select function clear display, clear display.
	BF_POLL();
	Delay_XuS(400);
	
	for(int i = 0; i < strlen(str); i++)
	{
		OUT_CHAR(str[i]);
		
		if(i == 16)
		{
			__far_mem_write(LCD_Start, 0xC0); // Go to second line.
			BF_POLL();
			Delay_XuS(400);
		}
	}
}

int main(void)
{
    TC_INIT();
	LCD_INIT();
	PORTF.DIRCLR = 0x01; //Set port F pin as switch input.
	
    while (1) 
    {
		TCE0.CCA = 0x01E4; //The count is always compared with this register.
		
		if((PORTF_IN & 0x01) == 0x01)
		{
			TCE0.CTRLA = TC_CLKSEL_DIV1_gc; // Turn on the counter which should count/pulse until the period set in CCA.
		}
		else
		{
			TCE0.CTRLA = TC_CLKSEL_OFF_gc; // Turn off the counter.
		}
    }
}

