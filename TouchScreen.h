#ifndef TouchScreen_h
#define TouchScreen_h

#include "SSI.h"
#include "SSISerialComms.h"
#include "NVIC.h"
#include "SYSTIC.h"
#include "GPTM.h"

void TOUCH_initialize(BYTE interruptPort, BYTE interruptPin, BYTE SSI_moduleNumber);
void TOUCH_handle(unsigned char);
void TOUCH_dataReceived_handle(unsigned char);
void sampleTouch_handle(unsigned char moduleNumber);

#endif
