#ifndef GPTM_h
#define GPTM_h

#include "JLib.h"
#include "NVIC.h"
#include "SystemClock.h"
#include <stdlib.h>

typedef enum {OST_type, PT_type, IEC_type, IET_type, PWM_type} TMTypes;
typedef enum {GPTM_a, GPTM_b, GPTM_both} TimerLetter;
typedef enum { _16Bit, _32Bit, _64Bit} TimerSize;
typedef enum {count_down, count_up} UpOrDown;

//struct Timer{
//	GPTM_TimerMode mode;
//	unsigned char timerNumber;
//	TimerLetter letter;
//	
//	BYTE_ADDRESS timerAddress;
//	bool cDir;
//	BYTE modeCode;
//	BYTE eventCode;
//};
struct _64BitValue{
	int lowerHalf;
	int upperHalf;
};
struct oneShotTimer{
	//user readable values
	BYTE timerMode;
	UpOrDown countDirection;
	bool startAtExternTrigger;
	bool captureFreeRunningTimerAtTimeOut;
	//configure additionalTrigger or interrupt
	//appropriate interrupts? -> yes =(
};
struct periodicTimer{
	int x;
};
struct realTimeClock{
	int x;
};
struct inputEdgeCount{
	int x;
};
struct inputEdgeTiming{
	int x;
};
struct pulseWidthModulation{
	int x;
};
union TimerType{
	struct oneShotTimer OST;
	struct periodicTimer PT;
	struct realTimeClock RTC;
	struct inputEdgeCount IEC;
	struct inputEdgeTiming IET;
	struct pulseWidthModulation PWM;
};
struct TimerTypeWithData{
	union TimerType timer;
	bool active;
	TimerLetter letter;
	TMTypes type;
	
	//auto-generated values
	struct _64BitValue countValue;
};
struct timerAB{
  struct TimerTypeWithData* a;
	struct TimerTypeWithData* b;
};
union SpecificTimerVariables{
	struct timerAB ab;
	struct TimerTypeWithData* both;
};
struct Timer{
	BYTE timerNumber;
	BYTE_ADDRESS timerAddress;
	bool useBothTimers;
	union SpecificTimerVariables STV;
};

struct TimerTypeWithData* new_oneShotTimer(long long countValue, UpOrDown countDir, bool startAtExternTrig, bool capFreeRunTmAtTimeOut);
struct periodicTimer* new_periodicTimer();
struct inputEdgeCount* new_inputEdgeCount();
struct inputEdgeTiming* new_inputEdgeTimingMode();
struct pulseWidthModulation* new_pulseWidthModulation();

struct Timer* new_Timer(struct TimerTypeWithData* tm, unsigned char timerNumber, TimerLetter abOrBoth);


void GPTM_initializeTimer(BYTE timerNumber, TimerLetter tmLetter);
void GPTM_initPT(BYTE_ADDRESS, struct TimerTypeWithData*);


void GPTM_disableTimer(BYTE_ADDRESS addr, struct TimerTypeWithData* tm);
void GPTM_setConfigRegister(struct Timer* tm, WORD regValue);
void GPTM_configureOneShotVsPeriodic(struct Timer* tm);
void GPTM_setStartValue(struct Timer* tm);
void GPTM_configureInterrupts(struct Timer* tm);
void GPTM_enableTimer(struct Timer* tm);
void GPTM_setTimerMode(struct Timer* tm);

BYTE_ADDRESS GPTM_getAddress(BYTE);

#endif
