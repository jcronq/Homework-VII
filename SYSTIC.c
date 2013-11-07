#include "SYSTIC.h"

static const WORD MAX_COUNT = 0x00FFFFFF;

static const BYTE_ADDRESS SYSTICKBASE = (BYTE_ADDRESS)0xE000E000;

static const HALF_WORD STCTRL    = 0x010;
static const HALF_WORD STRELOAD  = 0x014;
static const HALF_WORD STCURRENT = 0x018;

static const BYTE ENABLE_bit = 0;
static const BYTE CLK_SRC_bit = 2;
static const BYTE COUNT_bit = 16;

bool hasTimerExpired(){
	return (bool)getBit((BYTE_ADDRESS)SYSTICKBASE, STCTRL, COUNT_bit);
}

/////////////////////////////////////////////////////////////////////////
// runDelay
// Delay a given number of milliseconds
// Inputs:
//		delay - int with the value of time delay needed in milliseconds
// Outputs:
//		NONE
/////////////////////////////////////////////////////////////////////////
void runDelay(WORD delay)
{	
	// initialize timer for baud rate
	setBit_word((BYTE_ADDRESS)SYSTICKBASE, STCTRL, ENABLE_bit, DISABLE);
	//SYSTICKBASE[0x010/4] = 0; // disable sys-tick
	selectiveClearAndWrite_word((BYTE_ADDRESS)SYSTICKBASE, STCURRENT, 0x00FFFFFF, 0x00000000);
	//SYSTICKBASE[0x018/4] = 0; // reset counter
	write_word((BYTE_ADDRESS)SYSTICKBASE, STRELOAD, delay);
	//SYSTICKBASE[0x014/4] = delay; // set count for millisecond delay
	setBit_word((BYTE_ADDRESS)SYSTICKBASE, STCTRL, CLK_SRC_bit, ENABLE);
	//SYSTICKBASE[0x010/4] = 0x05; // start counter
	setBit_word((BYTE_ADDRESS)SYSTICKBASE, STCTRL, ENABLE_bit, ENABLE);
	
	// wait for timer to expire
	while(!hasTimerExpired());
}
// End runDelay ///////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// MSDelay
// Delay a given number of milliseconds
// Inputs:
//		delay - int with the value of time delay needed in milliseconds
// Outputs:
//		NONE
/////////////////////////////////////////////////////////////////////////
void delayMS(unsigned int delay)
{ 
	int delayCount = 0;	
	delayCount = (delay * SysClk_getSysClkFreq()/1000); // calculate the delay counter value
	
	while(delayCount > MAX_COUNT)
	{
		delayCount -= MAX_COUNT;
		runDelay(MAX_COUNT);
	}
	runDelay(delayCount);
	
	
}
// End MSDelay ///////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// delayUS
// Delay a given number of microseconds
// Inputs:
//		delay - int with the value of time delay needed in microseconds
// Outputs:
//		NONE
/////////////////////////////////////////////////////////////////////////
void delayUS(unsigned int delay)
{ 
	int delayCount = 0;	
	delayCount = (delay * SysClk_getSysClkFreq()/1000000); // calculate the delay counter value
	
	while(delayCount > MAX_COUNT)
	{
		delayCount -= MAX_COUNT;
		runDelay(MAX_COUNT);
	}
	runDelay(delayCount);	
}
// End delayUS ///////////////////////////////////////////////////////////

