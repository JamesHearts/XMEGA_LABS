/*
 * Lab6b.c
 *
 * Created: 7/18/2016 4:03:04 PM
 * Author : James Mak
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "ebi_driver.h"
#include <string.h>
#define F_CPU 2000000
#define CS0_Start 0x8000
#define CS1_Start 0x420000
#define LCD_Start 0x422000
#define LCD_Write 0x422001
#define bsel 107
#define bscale -5

int CMA[] = {0x0719,0x5FF,0x510,0x3C9,0x0301,0x0283,0x01E4};
int SCL[] = {0x03C9,0x0360,0x0301,0x02D6,0x0283,0x0240};

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

void TC0_INIT(void)
{
	//We are using Port E TC because it is the only port available that isn't taken.
	PORTE_DIRSET = 0x01; //Set Pin 1 to output.
	TCE0.CTRLA = TC_CLKSEL_DIV1_gc;  //Turn off the Counter.
	TCE0.CTRLB = TC_WGMODE_FRQ_gc | TC0_CCAEN_bm;  //Waveform FRQ generation mode and Compare enable A set.
	TCE0.CTRLE = TC_BYTEM_NORMAL_gc; //Normal mode TC.
	TCE0.CTRLFCLR = TC0_DIR_bm;  //Incrementing Counter.
	TCE0.CNT = 0; //Start count at zero.
	
}

void TC1_INIT(void)
{
	TCE1.CTRLA = TC_CLKSEL_DIV256_gc; //Turn off the Counter.
	TCE1.CTRLB = 0x00; //Normal Waveform mode.
	TCE1.CTRLE = TC_BYTEM_NORMAL_gc; //Normal mode TC.
	TCE1.CTRLFCLR = TC1_DIR_bm; //Incrementing Counter.
	TCE1.INTCTRLA = TC_OVFINTLVL_LO_gc; //Enable low level interrupt.
	PMIC_CTRL = PMIC_LOLVLEN_bm; //Enable low level interrupts.
	sei();
}

ISR(TCE1_OVF_vect)
{
	TCE0_CCA = 0x00;
	TCE1_PER = 0x00; //Set the duration register to zero.
	TCE1_INTFLAGS = 0x01; //Clear the flag.
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

void UART_INIT(void)
{
	PORTQ.DIRSET = 0x0A;
	PORTQ.OUTCLR = 0x0A;
	PORTD.DIRSET = 0x08;
	PORTD.OUTSET = 0x08;
	PORTD.DIRCLR = 0x04;
	USARTD0.CTRLB = 0x18;
	USARTD0.CTRLC = 0x03;
	USARTD0.BAUDCTRLA = (bsel & 0xFF);
	USARTD0.BAUDCTRLB = ((bscale << 4) & 0xF0) | ((bsel >> 8) & 0x0F );
	
}

uint32_t IN_CHAR(void)
{
	uint8_t busyflag = USARTD0_STATUS;
	
	if((busyflag & 0x80) == 0x80)
	{
		return USARTD0_DATA;
	}
	else
		return 0x00;
}

int main(void)
{
	EBI_INIT();
    TC0_INIT();
	TC1_INIT();
	LCD_INIT();
	UART_INIT();
	uint8_t key = 0x00;
	
    while (1) 
    {
		key = IN_CHAR();
		switch(key)
		{
			case 0x30 :
			    __far_mem_write(LCD_Start, 0x01);	// Select function clear display, clear display.
		    	BF_POLL();
				OUT_STRING("A6");
				BF_POLL();
				__far_mem_write(LCD_Start, 0xC0);
				BF_POLL();
				OUT_STRING("1760.00 Hz");
			
				TCE0.CCA = 0x0240;
				TCE1.PER = 0x897;
			
			break;
			case 0x31 :
				__far_mem_write(LCD_Start, 0x01);	// Select function clear display, clear display.
				BF_POLL();
				OUT_STRING("C6");
				BF_POLL();
				__far_mem_write(LCD_Start, 0xC0);
				BF_POLL();
				OUT_STRING("1046.50 Hz");
			
				TCE0.CCA = 0x03C9;
				TCE1.PER = 0x897;
			
			break;
			case 0x32 :
				__far_mem_write(LCD_Start, 0x01);	// Select function clear display, clear display.
				BF_POLL();
				OUT_STRING("C#6/Db6");
				BF_POLL();
				__far_mem_write(LCD_Start, 0xC0);
				BF_POLL();
				OUT_STRING("1108.73 Hz");
			
				TCE0.CCA = 0x0394;
				TCE1.PER = 0x897;
			
			break;
			case 0x33 :
				__far_mem_write(LCD_Start, 0x01);	// Select function clear display, clear display.
				BF_POLL();
				OUT_STRING("D6");
				BF_POLL();
				__far_mem_write(LCD_Start, 0xC0);
				BF_POLL();
				OUT_STRING("1174.66 Hz");
			
				TCE0.CCA = 0x0360;
				TCE1.PER = 0x897;
			
			break;
			case 0x34 :
				__far_mem_write(LCD_Start, 0x01);	// Select function clear display, clear display.
				BF_POLL();
				OUT_STRING("D#6/Eb6");
				BF_POLL();
				__far_mem_write(LCD_Start, 0xC0);
				BF_POLL();
				OUT_STRING("1244.51");
			
				TCE0.CCA = 0x032F;
				TCE1.PER = 0x897;
			
			break;
			case 0x35 :
				__far_mem_write(LCD_Start, 0x01);	// Select function clear display, clear display.
				BF_POLL();
				OUT_STRING("E6");
				BF_POLL();
				__far_mem_write(LCD_Start, 0xC0);
				BF_POLL();
				OUT_STRING("1318.51 Hz");
			
				TCE0.CCA = 0x0301;
				TCE1.PER = 0x897;
			
			break;
			case 0x36 :
				__far_mem_write(LCD_Start, 0x01);	// Select function clear display, clear display.
				BF_POLL();
				OUT_STRING("F6");
				BF_POLL();
				__far_mem_write(LCD_Start, 0xC0);
				BF_POLL();
				OUT_STRING("1396.91 Hz");
			
				TCE0.CCA = 0x02D6;
				TCE1.PER = 0x897;
			
			break;
			case 0x37 :
				__far_mem_write(LCD_Start, 0x01);	// Select function clear display, clear display.
				BF_POLL();
				OUT_STRING("F#6/Gb6");
				BF_POLL();
				__far_mem_write(LCD_Start, 0xC0);
				BF_POLL();
				OUT_STRING("1479.98 Hz");
			
				TCE0.CCA = 0x02AE;
				TCE1.PER = 0x897;
			
			break;
			case 0x38 :
				__far_mem_write(LCD_Start, 0x01);	// Select function clear display, clear display.
				BF_POLL();
				OUT_STRING("G6");
				BF_POLL();
				__far_mem_write(LCD_Start, 0xC0);
				BF_POLL();
				OUT_STRING("1577.98 Hz");
			
				TCE0.CCA = 0x0283;
				TCE1.PER = 0x897;
			
			break;
			case 0x39 :
				__far_mem_write(LCD_Start, 0x01);	// Select function clear display, clear display.
				BF_POLL();
				OUT_STRING("G#6/Ab6");
				BF_POLL();
				__far_mem_write(LCD_Start, 0xC0);
				BF_POLL();
				OUT_STRING("1864.66 Hz");
			
				TCE0.CCA = 0x0262;
				TCE1.PER = 0x897;
			
			break;
			case 0x41 :
				__far_mem_write(LCD_Start, 0x01);	// Select function clear display, clear display.
				BF_POLL();
				OUT_STRING("A#6/Bb6");
				BF_POLL();
				__far_mem_write(LCD_Start, 0xC0);
				BF_POLL();
				OUT_STRING("1864.66 Hz");
			
				TCE0.CCA = 0x0220;
				TCE1.PER = 0x897;
			
			break;
			case 0x42 :
				__far_mem_write(LCD_Start, 0x01);	// Select function clear display, clear display.
				BF_POLL();
				OUT_STRING("B6");
				BF_POLL();
				__far_mem_write(LCD_Start, 0xC0);
				BF_POLL();
				OUT_STRING("1975.53 Hz");
			
				TCE0.CCA = 0x0201;
				TCE1.PER = 0x897;
			
			
			break;
			case 0x43 :
				__far_mem_write(LCD_Start, 0x01);	// Select function clear display, clear display.
				BF_POLL();
				OUT_STRING("C7");
				BF_POLL();
				__far_mem_write(LCD_Start, 0xC0);
				BF_POLL();
				OUT_STRING("2093.00 Hz");
			
				TCE0.CCA = 0x01E4;
				TCE1.PER = 0x897;
			
			break;
			case 0x44 :
				__far_mem_write(LCD_Start, 0x01);	// Select function clear display, clear display.
				BF_POLL();
				OUT_STRING("C#7/Db7");
				BF_POLL();
				__far_mem_write(LCD_Start, 0xC0);
				BF_POLL();
				OUT_STRING("2217.46 Hz");
			
				TCE0.CCA = 0x01C9;
				TCE1.PER = 0x897;
			
			break;
			case 0x2A :
			__far_mem_write(LCD_Start, 0x01);	// Select function clear display, clear display.
			BF_POLL();
			OUT_STRING("C Major Arpeggio");
			BF_POLL();
			
			for(int i = 0; i < 7; i++)
			{
				TCE0.CCA = CMA[i];
				TCE1.PER = 0x897;
				
				while((TCE1_INTFLAGS & 0x01) == 0x00)
				{
					
				}
			}
			
			Delay_XuS(400);
			
			for(int j = 7; j > 0; j--)
			{
				TCE0.CCA = CMA[j];
				TCE1.PER = 0x897;	
				
				while((TCE1_INTFLAGS & 0x01) == 0x00)
				{
					
				}
			}
			break;
			case 0x23 :
			__far_mem_write(LCD_Start, 0x01);	// Select function clear display, clear display.
			BF_POLL();
			OUT_STRING("C Major Scale");
			BF_POLL();
			
			for(int i = 0; i < 7; i++)
			{
				TCE0.CCA = SCL[i];
				TCE1.PER = 0x897;
				
				while((TCE1_INTFLAGS & 0x01) == 0x00)
				{
					
				}
			}
			break;
			
			default:
			break;
		}
    }
}



