#include <stdio.h>
#include "ADC.H"
#include "JLib.h"
#include "UART.h"
#include "SystemClock.h"
#include "DACTimer.h"
#include "SSI.h"
#include "Screen.h"


struct SSI_module *ssi;

static BYTE_ADDRESS ADC0 = ((BYTE_ADDRESS)0x40038000); 
static BYTE_ADDRESS TIMER1 = ((BYTE_ADDRESS)0x40031000);

volatile WORD ADCdata = 0;
volatile WORD averageADCvalue = 0;
volatile WORD sumADCvalue = 0;
volatile WORD ADCcount = 0;
volatile double newTimerValue = 0;
volatile double newFreqValue = 0;
char stringBuffer[40];
volatile bool updateText = false;

static const int sineWave[40] = {512,592,670,744,813,874,926,968,998,1017,1023,1017,998,968,926,874,813,744,670,592,512,432,354,280,211,150,98,56,26,7,0,7,26,56,98,150,211,280,354,432};
volatile unsigned char sineCount = 0;	
	
void ADC0Seq0_Handler(void);
void Timer1A_Handler(void);

	int i = 0;
	
int main(void)
{
	startScreen();
	//println("hello");
	clearLCD(black);
	//ssi = malloc(sizeof(*ssi)); <-- new_SSIMasterModule allocates memory space. This statement would result in a memory leak.	
	ssi = new_SSIMasterModule(0, FREESCALE, 1000000, 16, false, true, true);
	setSysClkTo_80MHz();
	
	initADC02msWithInterrupt();
	initTimerForDacOutput();
	
	while(1)
	{		
		//println("hello");
	//	if(updateText)
	//	{
	//		clearText();			
			//clearLCD(black);
		++i;
		sprintf(stringBuffer, "%u", i);
			println(stringBuffer);
	//		updateText = false;
	//	}
	}		
}

void ADC0Seq0_Handler(void)
{	
	ADCdata = read_word(ADC0, 0x048); // get adc data
	ADCcount++;
	sumADCvalue += ADCdata;
	averageADCvalue = sumADCvalue/ADCcount;
	
	if(ADCcount == 250)
	{		
		newTimerValue = (4.39453125)*averageADCvalue + 2000;
		newFreqValue = (-0.05)*newTimerValue + 1100                                                                                                                                                                                                                                                                                                                                                                        ;
		ADCcount = 0;
		averageADCvalue = 0;
		sumADCvalue = 0;
		// write new timer load value
		updateCount((int)(newTimerValue));
		// write new frequency to LCD
		sprintf(stringBuffer, "%u Hz", (int)newFreqValue);
		updateText = true;
		
	}
			
	write_word(ADC0, 0x00C, 0xFF); // clear interrupt
}



void Timer1A_Handler(void)
{
	// send data to DAC
	SSI_sendData(ssi, ((sineWave[sineCount]<<2)&0xFFC));
	sineCount++;
	if(sineCount == 40)
		sineCount = 0;	
	// clear interrupt
	write_word(TIMER1, 0x024, 0xFFFFF);
	
}