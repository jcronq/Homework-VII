
#include "DACTimer.h"

static BYTE_ADDRESS CORE_PERIPHERALS = ((BYTE_ADDRESS)0xE000E000);
static BYTE_ADDRESS SYSTEM_CONTROL = ((BYTE_ADDRESS)0x400FE000);
static BYTE_ADDRESS TIMER1 = ((BYTE_ADDRESS)0x40031000);
static BYTE_ADDRESS TIMER2 = ((BYTE_ADDRESS)0x40032000);

static const int sineWave[] = {512,592,670,744,813,874,926,968,998,1017,1023,1017,998,968,926,874,813,744,670,592,512,432,354,280,211,150,98,56,26,7,0,7,26,56,98,150,211,280,354,432};

struct SSI_module *ssi;
	
volatile BYTE sineCount1 = 0;
volatile BYTE sineCount2 = 0;
	
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
	
static struct NVIC_interruptModule_GPTM* timer1A_interrupt = NULL;
static struct NVIC_interruptModule_GPTM* timer1B_interrupt = NULL;
static struct NVIC_interruptModule_GPTM* signalManager_interrupt = NULL;

void signalManager_handler(BYTE timerNumber){
	// send data to DAC
	SSI_sendData(ssi, ((((sineWave[sineCount1]/2)+(sineWave[sineCount2]/2))<<2)&0xFFC));
}
void signal1_handler(BYTE timerNumber)
{
	sineCount1++;
	if(sineCount1 == 40)
		sineCount1 = 0;	
}
void signal2_handler(BYTE timerNumber)
{
	sineCount2++;
	if(sineCount2 == 40)
		sineCount2 = 0;	
}
void startTimer1a(void){
	// configure TIMER1A 	
	setBit_word(TIMER1, 0x00C, 0x0, DISABLE);//set timer to 16bit mode
	write_word(TIMER1, 0x000, 0x4); // disable timer
	write_word(TIMER1, 0x004, 0x32);	// set to periodic mode
	write_word(TIMER1, 0x028, 910);	// for 80MHz clk.  generates pulse at 2_200 Hz
	setBit_word(TIMER1, 0x018, 0, ENABLE);	// enable interrupts
	setBit_word(TIMER1, 0x00C, 0, ENABLE);//enable timer
}
void startTimer1b(void){
		// configure TIMER1B 	-- values not configured for 1B yet
	setBit_word(TIMER1, 0x00C, 8, DISABLE);//disable timer
	write_byte(TIMER1, 0x008, 0x32);//configure timer
	write_word(TIMER1, 0x02C, 1176);//for 80MHz signal.  generates pulse at 1_700 Hz
	setBit_word(TIMER1, 0x018, 8, ENABLE);	// enable interrupts
	setBit_word(TIMER1, 0x00C, 8, ENABLE);//enable timer
}
void startTimer2a(void){
	// configure TIMER2A 	-- values not configured for 1B yet
	setBit_word(TIMER2, 0x00C, 0x0, DISABLE);//set timer to 16bit mode
	write_word(TIMER2, 0x000, 0x4); // disable timer
	write_word(TIMER2, 0x004, 0x32);	// set to periodic mode
	write_word(TIMER2, 0x028, 100);	// make sure it's faster than the others
	setBit_word(TIMER2, 0x018, 0, ENABLE);	// enable interrupts
	setBit_word(TIMER2, 0x00C, 0, ENABLE);//enable timer
}

void initTimersAndInterruptsForDacOutput(void)
{
	Function handles[1];
	ssi = new_SSIMasterModule(0, FREESCALE, 1000000, 16, false, true, true); 
	
	// enable TIMER1A sysclk
	readModWrite_byte(SYSTEM_CONTROL, RCGCTIMER, 0x06,ENABLE); 

	startTimer1a();
	startTimer1b();
	startTimer2a();
	//enable interrupts for corresponding timers
	handles[0] = signal1_handler;
	timer1A_interrupt = new_GPTMInterrupt(1, GPTM_a, handles, true, false, false, false, false);
	NVIC_registerGPTMInterrupt(timer1A_interrupt);
	handles[0] = signal2_handler;
	timer1B_interrupt = new_GPTMInterrupt(1, GPTM_b, handles, true, false, false, false, false);
	NVIC_registerGPTMInterrupt(timer1B_interrupt);
	handles[0] = signalManager_handler;
	timer1B_interrupt = new_GPTMInterrupt(1, GPTM_a, handles, true, false, false, false, false);
}

