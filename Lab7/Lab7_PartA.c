/*
 * lab7a.c
 *
 * Created: 7/25/2016 3:18:40 PM
 * Author : James Mak
 */ 

#include <avr/io.h>

void DAC_INIT()
{
	PORTB_DIRSET = 0x04; //Pin 2 to output.
	PORTB_OUT = 0x00;
	DACB.CTRLA = DAC_CH0EN_bm | DAC_ENABLE_bm;
	DACB.CTRLB = 0x00; 
	DACB.CTRLC = DAC_REFSEL_AREFB_gc;
	
}

void DAC_WRITE(int number)
{
	    while(!(DACB_STATUS & DAC_CH0DRE_bm));
		DACB_CH0DATA = number;
	
}


int main(void)
{
    DAC_INIT();
	
	float value = 0;
	int value_int = 0;
	
	value = ((1.3 * 2047) / 2.5) - 4095;
	value_int = 2048 - (int)value;
	
	PORTB_PIN2CTRL = 0x40;
    while (1) 
    {
		
		DAC_WRITE(value_int);
		
    }
}

