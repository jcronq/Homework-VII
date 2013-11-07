
#include "ADC.h"



static BYTE_ADDRESS CORE_PERIPHERALS = ((BYTE_ADDRESS)0xE000E000);
static BYTE_ADDRESS SYSTEM_CONTROL = ((BYTE_ADDRESS)0x400FE000);
static BYTE_ADDRESS TIMER0 = ((BYTE_ADDRESS)0x40030000);
static BYTE_ADDRESS GPIOD = ((BYTE_ADDRESS)0x40007000); 
static BYTE_ADDRESS ADC0 = ((BYTE_ADDRESS)0x40038000); 

static const HALF_WORD RCGCTIMER = 0x604;//TIMER run mode clock gating control
static const HALF_WORD RCGCGPIO = 0x608;//GPIO run mode clock gating control
static const HALF_WORD RCGCADC = 0x638;//ADC run mode clock gating control

static const HALF_WORD GPTMCFG  = 0x000; //R/W 0x0000.0000 GPTM Configuration
static const HALF_WORD GPTMTAMR = 0x004; //R/W 0x0000.0000 GPTM Timer A Mode
static const HALF_WORD GPTMTBMR = 0x008; //R/W 0x0000.0000 GPTM Timer B Mode
static const HALF_WORD GPTMCTL  = 0x00C; //R/W 0x0000.0000 GPTM Control
static const HALF_WORD GPTMTAILR= 0x028; //R/W 0xFFFF.FFFF GPTM Timer A Interval Load
static const HALF_WORD GPTMTBILR= 0x02C; //R/W - GPTM Timer B Interval Load
static const HALF_WORD GPTMTAMATCHR= 0x030; //R/W - GPTM Timer A Match
static const HALF_WORD GPTMTBMATCHR= 0x030; //R/W - GPTM Timer B Match
static const HALF_WORD GPTMTAPR = 0x038; //R/W 0x0000.0000 GPTM Timer A Prescale
static const HALF_WORD GPTMTBPR = 0x03C; //R/W 0x0000.0000 GPTM Timer B Prescale


// initialize ADC0 with an interrupt
// initialize GPIOE.3 for ADC0
// initialize GPTM to trigger ADC at 2ms
void initADC02msWithInterrupt(void)
{
	// enable interrupts for ADC0 in the NVIC
	readModWrite_word(CORE_PERIPHERALS, 0x100, 0x00004000,ENABLE); 
	
	// enable TIMER0A sysclk
	readModWrite_byte(SYSTEM_CONTROL, RCGCTIMER, 0x01,ENABLE); 
	// enable GPIOD sysclk
	readModWrite_byte(SYSTEM_CONTROL, RCGCGPIO, 0x08,ENABLE); 
	// enable ADC0 sysclk
	readModWrite_byte(SYSTEM_CONTROL, RCGCADC, 0x01,ENABLE); 
	// configure GPIOF for ADC
	readModWrite_byte(GPIOD, 0x420, 0x08,ENABLE);
	readModWrite_byte(GPIOD, 0x51C, 0x08,DISABLE);
	readModWrite_byte(GPIOD, 0x528, 0x08,ENABLE);
	
	// configure TIMER0A for 2ms TAOTE enabled
	// disable TIMER0A
	write_word(TIMER0, 0x000, 0); // disable timer
	write_word(TIMER0, 0x004, 0x02);	// set to periodic mode
	write_word(TIMER0, 0x028, 160000);	// set timer to 2ms
	write_word(TIMER0, 0x00C, 0x0021); // enable timer with output enable for adc
		
	// configure ADC0
	write_word(ADC0, 0x000, 0);	// disable ADC SS
	write_word(ADC0, 0x014, 0x0005);	// select timer as trigger source
	write_word(ADC0, 0x040, 4);	// select the ADC sample sequence
	write_word(ADC0, 0x044, 0x06);	// select the ADC sample sequence to end on first sample and generate an interrupt
	write_word(ADC0, 0x008, 0x01);	// enable interrupts to get adc data
	write_word(ADC0, 0x000, 0x01);	// enable ADC SS
	
}


