/*
 * lab5a.c
 *
 * Created: 7/11/2016 4:31:21 PM
 * Author : James Mak
 */

#include <avr/io.h>
#include "ebi_driver.h"

#define F_CPU 2000000
#define CS0_Start 0x8000
#define CS1_Start 0x420000
#define LCD_Start 0x422000
#define LCD_Write 0x422001

void Delay_1sec(void)
{
	volatile uint32_t count;
	
	for(count = 0; count < F_CPU; count++);
	
}

void Delay_XuS(int a) // Delay a certain amount of uS.
{
	for(int i = 0; i < a; i++);	
}

void EBI_INIT(void)
{
	PORTH.DIR = 0x37;
	PORTH.OUT = 0x33;
	PORTK.DIR = 0xFF;
	
	EBI.CTRL = EBI_CS_MODE_SRAM_gc | EBI_IFMODE_3PORT_gc; //3 port (H,J,K) SRAM mode.
	
	EBI.CS0.BASEADDRH = (uint8_t) (CS0_Start>>16) & 0xFF;
	EBI.CS0.BASEADDRL = (uint8_t) (CS0_Start>>8) & 0xFF;
	EBI.CS0.CTRLA = EBI_CS_MODE_SRAM_gc | EBI_CS_ASPACE_16KB_gc; //SRAM mode 16k address space.
	
	EBI.CS1.BASEADDR = (uint16_t) (CS1_Start>>8) & 0xFFFF;
	EBI.CS1.CTRLA = EBI_CS_MODE_SRAM_gc | EBI_CS_ASPACE_64KB_gc; //SRAM mode 64k address space.
}

void BF_POLL(void)
{
	uint8_t BF = __far_mem_read(LCD_Start); 
	
	while((BF & 0x80) == 0x80) //Poll the busy flag (bit 7 until it is clear.
	{
		BF = __far_mem_read(LCD_Start);
	}
}

void LCD_INIT(void)
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

void OUT_CHAR(char character)
{
	__far_mem_write(LCD_Write, character);
	BF_POLL();
	Delay_XuS(400);
}

void OUT_STRING(char str[]) //Store the characters in strings.
{
	__far_mem_write(LCD_Start, 0x01);	// Select function clear display, clear display.
	BF_POLL();
	Delay_XuS(400);
	
	for(int i = 0; i < strlen(str); i++)
	{
		OUT_CHAR(str[i]);
	}
}

int main(void)
{
	EBI_INIT();
	LCD_INIT();
	OUT_STRING("James is awesome!");
	while(1)
	{
		// Infinite loop.
	}
}