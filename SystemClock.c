#include "SystemClock.h"

static BYTE_ADDRESS SYSTEM_CONTROL = ((BYTE_ADDRESS)0x400FE000);
static const HALF_WORD RCC  = ((HALF_WORD)0x060);
static const HALF_WORD RCC2 = ((HALF_WORD)0x070);

static const BYTE MOSCDIS_bit = 1;
static const BYTE USESYSDIV_bit = 22;
static const BYTE BYPASS_bit = 11;

static const BYTE OSCSRC_bit = 4;
static const BYTE XTAL_bit = 6;
static const BYTE SYSDIV_bit = 23;

static const WORD OSCSRC_clear = (WORD)0x00000300;
static const WORD XTAL_clear   = (WORD)0x000007C0;
static const WORD SYSDIV_clear = (WORD)0x07800000;

volatile WORD SYSTEM_CLOCK = 16000000;

void SysClk_setClock(unsigned int Hz)
{
	
}

void setSysClkTo_80MHz(void)
{
	SYSTEM_CLOCK = 80000000;
	SysClk_mainOsc (DISABLE);
	SysClk_setOscSource (PreciseInternOsc);
	SysClk_setXTAL (0x15);
	SysClk_bypassPLL (DISABLE);
	SysClk_setFreqDivider (0x2);
	SysClk_useFreqDivider (ENABLE);
	write_word(SYSTEM_CONTROL, RCC2, 0xC0800000);
}

void SysClk_mainOsc(bool enable)
{
	//disable/Enable MainOsc
	setBit_word(SYSTEM_CONTROL, RCC, MOSCDIS_bit, enable);
}
void SysClk_setOscSource(OscSrc src)
{
	selectiveClearAndWrite_word
		(SYSTEM_CONTROL
		,RCC
		,OSCSRC_clear
		,((WORD)src)<<OSCSRC_bit
		);
}
void SysClk_setXTAL(unsigned char xtal)
{
	selectiveClearAndWrite_word
		(SYSTEM_CONTROL
		,RCC
		,XTAL_clear
		,xtal<<XTAL_bit
		);
}
void SysClk_setFreqDivider (unsigned char divisor)
{
	selectiveClearAndWrite_word
		(SYSTEM_CONTROL
		,RCC
		,SYSDIV_clear
		,divisor<<SYSDIV_bit
		);
}
void SysClk_useFreqDivider (bool enable)
{
	setBit_word(SYSTEM_CONTROL, RCC, USESYSDIV_bit, enable);
}
void SysClk_bypassPLL (bool enable)
{
	readModWrite_word
		(SYSTEM_CONTROL, RCC, BYPASS_bit, enable);
}
WORD SysClk_getSysClkFreq(void)
{
	volatile WORD retVal = (WORD)SYSTEM_CLOCK;
	return retVal;
}


