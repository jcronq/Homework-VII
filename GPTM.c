#include "GPTM.h"

static const unsigned char NUM_TIMERS = 12;

static BYTE_ADDRESS TimerAddresses [] = 
  {(BYTE_ADDRESS)0x40030000
	,(BYTE_ADDRESS)0x40031000
	,(BYTE_ADDRESS)0x40032000
	,(BYTE_ADDRESS)0x40033000
	,(BYTE_ADDRESS)0x40034000
	,(BYTE_ADDRESS)0x40035000
	,(BYTE_ADDRESS)0x40036000
	,(BYTE_ADDRESS)0x40037000
	,(BYTE_ADDRESS)0x4004C000
	,(BYTE_ADDRESS)0x4004D000
	,(BYTE_ADDRESS)0x4004E000
	,(BYTE_ADDRESS)0x4004F000
};	

static volatile struct Timer* TimerList[NUM_TIMERS] = {NULL};

static const HALF_WORD GPTMCFG  = 0x000; //R/W 0x0000.0000 GPTM Configuration
static const HALF_WORD GPTMTAMR = 0x004; //R/W 0x0000.0000 GPTM Timer A Mode
static const HALF_WORD GPTMTBMR = 0x008; //R/W 0x0000.0000 GPTM Timer B Mode
static const HALF_WORD GPTMCTL  = 0x00C; //R/W 0x0000.0000 GPTM Control
static const HALF_WORD GPTMTAILR= 0x028; //R/W 0xFFFF.FFFF GPTM Timer A Interval Load
static const HALF_WORD GPTMTBILR= 0x02C; //R/W - GPTM Timer B Interval Load
static const HALF_WORD GPTMTAPR = 0x038; //R/W 0x0000.0000 GPTM Timer A Prescale
static const HALF_WORD GPTMTBPR = 0x03C; //R/W 0x0000.0000 GPTM Timer B Prescale

//GPTMCTL
static const BYTE TAEN_bit   = 0;
static const BYTE TBEN_bit   = 8;
static const BYTE TAEVENT_bit = 2;
static const BYTE TBEVENT_bit = 10;
//GPTMTnMR
static const BYTE TnCDIR_bit = 4;
static const BYTE TnMR_bit   = 0;

//GPTMCTL
static const WORD TAEVENT_clear = 0x0000000C;
static const WORD TBEVENT_clear = 0x00000C00;
//GPTMTnMR
static const WORD TnMR_clear = 0x00000003;

struct TimerTypeWithData* new_oneShotTimer(long long countValue, UpOrDown countDir, bool startAtExternTrig, bool capFreeRunTmAtTimeOut){
	struct TimerTypeWithData* retVal = malloc(sizeof *retVal);
	
	retVal->type = OST_type;
	
	retVal->countValue.lowerHalf = (long)countValue;
	retVal->countValue.upperHalf = (long)(countValue >> 32);
	
	retVal->timer.OST.timerMode = 0x2;// this is the PT value.. .will change after debugging
	
	retVal->timer.OST.countDirection = countDir;
	retVal->timer.OST.startAtExternTrigger = startAtExternTrig;
	retVal->timer.OST.captureFreeRunningTimerAtTimeOut = capFreeRunTmAtTimeOut;
	//additional trigger or interrupt?
	//appropriate interrupts
	retVal->active = false;
	return retVal;
}
struct periodicTimer* new_periodicTimer(){
	struct periodicTimer* retVal = malloc(sizeof *retVal);
	return retVal;
}
struct inputEdgeCount* new_inputEdgeCount(){
	struct inputEdgeCount* retVal = malloc(sizeof *retVal);
	return retVal;
}
struct inputEdgeTiming* new_inputEdgeTiming(){
	struct inputEdgeTiming* retVal = malloc(sizeof *retVal);
	return retVal;
}
struct pulseWidthModulation* new_pulseWidthModulation(){
	struct pulseWidthModulation* retVal = malloc(sizeof *retVal);
	return retVal;
}
unsigned char getBest16BitTimerPosition(unsigned char seed){
	if(seed >= 6)
		return 6;
	if(TimerList[seed] == NULL)
		return seed;
	else{
		if(TimerList[seed]->useBothTimers){
			if(TimerList[seed]->STV.ab.a == NULL || TimerList[seed]->STV.ab.b == NULL)
				return seed;
			else
				return getBest16BitTimerPosition(seed+1);
		}else
			return getBest16BitTimerPosition(seed+1);
	}
}
struct Timer* new_Timer(struct TimerTypeWithData* tm, unsigned char timerNumber, TimerLetter abOrBoth){
	struct Timer* retVal = NULL;
	if(TimerList[timerNumber] == NULL){
		TimerList[timerNumber] = retVal = malloc(sizeof *retVal);
		TimerList[timerNumber]->timerNumber = timerNumber;
		TimerList[timerNumber]->timerAddress = TimerAddresses[timerNumber];
		if(abOrBoth!=GPTM_both){
			retVal->STV.ab.a = NULL;
			retVal->STV.ab.b = NULL;
		}
	}
	else
		retVal = (struct Timer*)TimerList[timerNumber];
	if(abOrBoth == GPTM_both)
	{
		TimerList[timerNumber]->useBothTimers = true;
		TimerList[timerNumber]->STV.both = tm;
	}else if(abOrBoth == GPTM_a){
		TimerList[timerNumber]->STV.ab.a = tm;
		TimerList[timerNumber]->useBothTimers = false;
	}
	else{
		TimerList[timerNumber]->STV.ab.a = tm;
		TimerList[timerNumber]->useBothTimers = false;
	}
	return retVal;
}
void GPTM_destroyTimer(struct Timer* tm)
{
	//BYTE timerNumber = tm->timerNumber;
	//free(TimerList[timerNumber]);
	//TimerList[timerNumber] = NULL;
}
void GPTM_initOST(BYTE_ADDRESS addr, struct TimerTypeWithData* tm){
	//1.
	GPTM_disableTimer(addr, tm);
	//2.
	write_word(addr, GPTMCFG, 0x00000000);
	//3.
	selectiveClearAndWrite_word
		(addr
		,(tm->letter == GPTM_b)?GPTMTBMR:GPTMTAMR
		,TnMR_clear
		,tm->timer.OST.timerMode<<TnMR_bit
		);
	//4.
	setBit_word
		(addr
		,(tm->letter == GPTM_b)?GPTMTBMR:GPTMTAMR
		,TnCDIR_bit
		,(bool)(tm->timer.OST.countDirection)
		);
	//5.
	write_word(addr,(tm->letter == GPTM_b)?GPTMTBILR:GPTMTAILR, tm->countValue.lowerHalf);
	
}
void GPTM_initT(BYTE timerNumber, struct TimerTypeWithData* tm){
	switch(tm->type)
	{
	case OST_type:
		GPTM_initOST(TimerAddresses[timerNumber], tm);
		break;
	case PT_type:
		//GPTM_initPT (TimerAddresses[timerNumber], tm);
		break;
	//etc.
	default:
		break;
	}
}
void GPTM_initializeTimer(BYTE timerNumber, TimerLetter tmLetter){
	switch(tmLetter)
	{
		case GPTM_a:
			GPTM_initT(timerNumber, TimerList[timerNumber]->STV.ab.a);
			break;
		case GPTM_b:
			GPTM_initT(timerNumber, TimerList[timerNumber]->STV.ab.b);
			break;
		case GPTM_both:
			GPTM_initT(timerNumber, TimerList[timerNumber]->STV.both);
			break;
		default:
			break;
	}
}

void GPTM_disableTimer(BYTE_ADDRESS addr, struct TimerTypeWithData* tm){
	setBit_word
		(addr
		,GPTMCTL
		,(tm->letter == GPTM_b)?TBEN_bit:TAEN_bit
		,DISABLE
	);
}

void GPTM_setPrescale(struct Timer* tm){
	
}
void GPTM_setConfigRegister(struct Timer* tm, WORD regValue){
	write_word
		(tm->timerAddress
		,GPTMCFG
		,regValue
	);
}
void configureOneShotVsPeriodic(struct Timer* tm){
	
}
void setStartValue(struct Timer* tm){
	
}
void configureInterrupts(struct Timer* tm){
	
}
void enableTimer(struct Timer* tm){
	
}
