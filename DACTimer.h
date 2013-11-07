
#ifndef DACTimer_h
#define DACTimer_h

#include "JLib.h"
#include "GPTM.h"
#include "GPIO.h"
#include "SSI.h"


void initTimersAndInterruptsForDacOutput(void);

void updateCount(unsigned int count);

#endif