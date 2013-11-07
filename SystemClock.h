#ifndef SystemClock_h
#define SystemClock_h

#include "JLib.h"



typedef enum{MainOsc, PreciseInternOsc, PIOSC4th, LowFreqIntOsc}OscSrc;

void setSysClkTo_80MHz(void);

void SysClk_setClock(unsigned int Hz);
void SysClk_mainOsc(bool enable);
void SysClk_internalOsc(bool enable);

void SysClk_setOscSource(OscSrc src);
void SysClk_setXTAL(unsigned char xtal);
void SysClk_setFreqDivider(unsigned char divisor);//frequency divider value

void SysClk_useFreqDivider(bool enable);					//frequency divider on/off
void SysClk_bypassPLL(bool enable);									//frequency multiplier on/off

WORD SysClk_getSysClkFreq(void);

#endif
