/*
 * lab5d.c
 *
 * Created: 7/11/2016 4:31:21 PM
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

void ADC_INIT(void)
{
	PORTB_DIRCLR = 0x04; //Pin 4 to input.
	ADCB.CTRLA = 0x07; //Channel 0, Flush, Enable ADC.
	ADCB.CTRLB = 0xCC; //Free-running, Unsigned, 8-bit.
	ADCB.REFCTRL = 0x30; //Port B as reference.
	ADCB.PRESCALER = 0x07; //Div/32 Pre-scaler.
	ADCB.EVCTRL = 0x00;
	ADCB.CH0.CTRL = 0x81;
	ADCB.CH0.MUXCTRL = 0x20; //ADC pin 7.
}

void BF_POLL(void)
{
	uint8_t BF = __far_mem_read(LCD_Start); 
	
	while((BF & 0x80) == 0x80) //Poll the busy flag (bit 7 until it is clear.
	{
		BF = __far_mem_read(LCD_Start);
	}
} 

/*int8_t fetch_bin_voltage()
{
	while((ADCB.CH0.INTFLAGS & 0x01) == 0)
	return 
}*/

float bin_to_decimal(uint8_t bin_voltage)
{
	float conv_voltage = (float)(bin_voltage * 5) / 255;
	return conv_voltage;
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

void print_voltage(float dec_voltage, uint8_t binary_voltage)
{
	char volt_display[10];
	int first = 0;
	int second = 0;
	int third = 0;
	char hexa = 'x';
	char volts = 'V';
	char decimal = '.';
	char space = ' ';
	char zero = '0';
	
	first = (int)dec_voltage;
	second = (int)((dec_voltage - first) * 10);
	third = (int)((((dec_voltage - first) * 10) - second) * 10);
	
	first = first + 0x30;
	second = second + 0x30;
	third = third + 0x30;
	
	volt_display[0] = first;
	volt_display[1] = decimal;
	volt_display[2] = second;
	volt_display[3] = third;
	volt_display[4] = volts;
	volt_display[5] = space;
	volt_display[6] = zero;
	volt_display[7] = hexa;
	
	uint8_t first_hex = (binary_voltage >> 4) & 0x0F;
	if(first_hex > 9)
	{
		switch(first_hex)
		{	
			case 10: volt_display[8] = 'A';
			break;
			case 11: volt_display[8] = 'B';
			break;
			case 12: volt_display[8] = 'C';
			break;
			case 13: volt_display[8] = 'D';
			break;
			case 14: volt_display[8] = 'E';
			break;
			case 15: volt_display[8] = 'F';
			break;
		}
	}
	else
	{
		volt_display[8] = (first_hex + 0x30);
	}
	
	uint8_t second_hex = (binary_voltage & 0x0F);
	if(second_hex > 9)
	{
		switch(second_hex)
		{
			case 10: volt_display[9] = 'A';
			break;
			case 11: volt_display[9] = 'B';
			break;
			case 12: volt_display[9] = 'C';
			break;
			case 13: volt_display[9] = 'D';
			break;
			case 14: volt_display[9] = 'E';
			break;
			case 15: volt_display[9] = 'F';
			break;
		}
	}
	else
	{
		volt_display[9] = (second_hex + 0x30);
	}
	
	for(int i = 10; i < strlen(volt_display); i++)
	{
		
		volt_display[i] = NULL;
	}

	OUT_STRING(volt_display);
		
}

int main(void)
{
	EBI_INIT();
	LCD_INIT();
	ADC_INIT();
	
	OUT_STRING("James is awesome!");
	
	while(1)
	{
		uint8_t bin_voltage = ADCB.CH0.RES;
		Delay_XuS(200);
		float dec_voltage = bin_to_decimal(bin_voltage);
		print_voltage(dec_voltage, bin_voltage);
		Delay_XuS(20000);
		Delay_XuS(20000);	
	}
}
