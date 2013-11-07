
#include "DACTimer.h"

static BYTE_ADDRESS CORE_PERIPHERALS = ((BYTE_ADDRESS)0xE000E000);
static BYTE_ADDRESS SYSTEM_CONTROL = ((BYTE_ADDRESS)0x400FE000);
static BYTE_ADDRESS TIMER1 = ((BYTE_ADDRESS)0x40031000);


static const HALF_WORD RCGCTIMER = 0x604;//TIMER run mode clock gating control

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



void initTimerForDacOutput(void)
{
	
	// enable interrupts for TIMER1A in the NVIC
	readModWrite_word(CORE_PERIPHERALS, 0x100, 0x00200000,ENABLE); 
	
	// enable TIMER1A sysclk
	readModWrite_byte(SYSTEM_CONTROL, RCGCTIMER, 0x02,ENABLE); 
	
	
	// configure TIMER1A 	
	write_word(TIMER1, 0x000, 0); // disable timer
	write_word(TIMER1, 0x004, 0x02);	// set to periodic mode
	write_word(TIMER1, 0x028, 20000);	// set timer to 250uSec
	write_word(TIMER1, 0x018, 0x01);	// enable interrupts
	write_word(TIMER1, 0x00C, 0x0001); // enable timer
}

void updateCount(unsigned int count)
{
	//write_word(TIMER1, 0x000, 0); // disable timer
	write_word(TIMER1, 0x028, count);	// set timer to new value
	//write_word(TIMER1, 0x00C, 0x0001); // enable timer 
}



