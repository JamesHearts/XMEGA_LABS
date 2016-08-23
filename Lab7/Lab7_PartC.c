/*
 * lab7c.c
 *
 * Created: 7/26/2016 12:19:36 AM
 * Author : James Mak
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

uint32_t table = 0;

float triLuT[] = {4,8,12,16,20,23,27,31,
	35,39,43,47,51,55,59,63,
	66,70,74,78,82,86,90,94,
	98,102,105,109,113,117,121,125,
	129,133,137,141,145,148,152,156,
	160,164,168,172,176,180,184,188,
	191,195,199,203,207,211,215,219,
	223,227,230,234,238,242,246,250,
	246,242,238,234,230,227,223,219,
	215,211,207,203,199,195,191,188,
	184,180,176,172,168,164,160,156,
	152,148,145,141,137,133,129,125,
	121,117,113,109,105,102,98,94,
	90,86,82,78,74,70,66,63,
	59,55,51,47,43,39,35,31,
	27,23,20,16,12,8,4,0};

float sineLuT[] = {125,131,137,143,149,155,161,167,
	173,178,184,189,194,199,204,209,
	213,218,222,225,229,232,235,238,
	240,243,245,246,248,249,249,250,
	250,250,249,249,248,246,245,243,
	240,238,235,232,229,225,222,218,
	213,209,204,199,194,189,184,178,
	173,167,161,155,149,143,137,131,
	125,119,113,107,101,95,89,83,
	77,72,66,61,56,51,46,41,
	37,32,28,25,21,18,15,12,
	10,7,5,4,2,1,1,0,
	0,0,1,1,2,4,5,7,
	10,12,15,18,21,25,28,32,
	37,41,46,51,56,61,66,72,
	77,83,89,95,101,107,113,119};

int sineLuT_conv[128];

void Delay_XuS(int a) // Delay a certain amount of uS.
{
	for(int i = 0; i < a; i++);
}

void DAC_INIT()
{
	PORTB_DIRSET = 0x04; //Pin 2 to output.
	PORTB_OUT = 0x00;
	DACB.CTRLA = DAC_CH0EN_bm | DAC_ENABLE_bm; //Enable Ch0 and enable the entire DAC.
	DACB.CTRLB = 0x01; //Auto triggered ch0. When the event system is triggered the DAC converts the next value.
	DACB.CTRLC = DAC_REFSEL_AREFB_gc;
	DACB.EVCTRL = 0x00; //CH0 event triggers DAC.
}

void DAC_POLL() //Polls the busy flag of the DAC.
{
	int bf = DACB_STATUS;
	Delay_XuS(400);
	
	while((bf & 0xFD) == 0x00)
	{
		bf = DACB_STATUS;
	}
	
}
void DAC_WRITE(int number)
{
	DAC_POLL();
	Delay_XuS(400);
	DACB_CH0DATA = number;
	
}

void Convert()
{
	float value = 0;
	int value_int = 0;
	
	for(int i = 0; i < 128; i++)
	{
		value = ((sineLuT[i]/100) * 4095) / 2.5;
		sineLuT_conv[i] = (int)value;
	}
}

void DAC_WRITE_WAVE()
{
	for(int i = 0; i < 128; i++)
	{
		DAC_WRITE(sineLuT_conv[i]);
		Delay_XuS(400);
	}		
}

void DMA_INIT(uint32_t table_addr)
{
	DMA.CTRL = DMA_ENABLE_bm;
	DMA.CH0.CTRLA = DMA_CH_REPEAT_bm | DMA_CH_BURSTLEN_2BYTE_gc | DMA_CH_SINGLE_bm;
	DMA.CH0.REPCNT = 0;
	DMA.CH0.ADDRCTRL = DMA_CH_SRCRELOAD_TRANSACTION_gc | DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTRELOAD_BURST_gc | DMA_CH_DESTDIR_INC_gc;
	DMA.CH0.TRIGSRC = DMA_CH_TRIGSRC_EVSYS_CH0_gc;
	DMA.CH0.TRFCNT = 256;
	
	DMA.CH0.SRCADDR0 = table_addr >> 0;
	DMA.CH0.SRCADDR1 = table_addr >> 8;
	DMA.CH0.SRCADDR2 = table_addr >> 16;
	
	DMA.CH0.DESTADDR0 = (uint32_t)(&DACB_CH0DATA) >> 0;
	DMA.CH0.DESTADDR1 = (uint32_t)(&DACB_CH0DATA) >> 8;
	DMA.CH0.DESTADDR2 = (uint32_t)(&DACB_CH0DATA) >> 16;
	
	DMA_CH0_CTRLA = 0xA5;
}

void TC0_INIT(void)
{
	//We are using Port E TC because it is the only port available that isn't taken.
	TCE0.CTRLA = TC_CLKSEL_DIV1_gc;  //Turn on the Counter 2MHZ.
	TCE0.CTRLB = TC_WGMODE_FRQ_gc | TC0_CCAEN_bm;  //Waveform FRQ generation mode and Compare enable A set.
	TCE0.CTRLE = TC_BYTEM_NORMAL_gc; //Normal mode TC.
	TCE0.CTRLFCLR = TC0_DIR_bm;  //Incrementing Counter.
	TCE0.CNT = 0; //Start count at zero.
	TCE0_CCA = 78; //Controls the TOP of the counter effectively controls frequency.
	EVSYS_CH0MUX = EVSYS_CHMUX_TCE0_OVF_gc; //When counter overflows then the event system is triggered (see DAC_INIT).
	
}

ISR(TCE0_OVF_vect)
{
	TCE0_INTFLAGS = 0x01; //Clear the flag.
}

int main(void)
{
    DAC_INIT();
	TC0_INIT();
	Convert(); //Converts the look up table to the proper values.
	table = (uint32_t)&(sineLuT_conv);
	DMA_INIT(table);
	
	PORTB_PIN2CTRL = (PORT_OPC_TOTEM_gc | PORT_ISC_INPUT_DISABLE_gc); //These are set by default anyways. No need to set.
	
	
    while(1) 
    {	
		Delay_XuS(400);
    }
}





