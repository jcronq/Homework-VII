#include "TouchScreen.h"

static struct SSISerialComms_module* serialInterface;
static struct NVIC_interruptModule_GPIO* interruptInterface;
static struct NVIC_interruptModule_GPTM* touchTimerInterruptInterface;

static bool currentlyTouched;

static BYTE_ADDRESS touchTimerAddr = (BYTE_ADDRESS)0x40036000;

//touchscreen commands
static HALF_WORD TOUCH_init[] = {0x80, 0x00, 0x00};
static HALF_WORD TOUCH_getX[] = {0x90, 0x00, 0x00};
static HALF_WORD TOUCH_getY[] = {0xD0, 0x00, 0x00};
static HALF_WORD commandSize  = 3;

//ScreenTouch Variables
static unsigned int averageX = 0;
static unsigned int averageY = 0;

//Will remove this function after GPTM is completed
void initializeTouchClock(){
	write_word((BYTE_ADDRESS)0x400FE000,0x65C, 0x0000000F);
	setBit_word(touchTimerAddr, 0x00C, 0, DISABLE);//disable timer
	write_byte(touchTimerAddr, 0x004, 0x32);//configure timer
	write_word(touchTimerAddr, 0x028, 800000);//for 80MHz signal.  generates pulse at 100Hz
	setBit_word(touchTimerAddr, 0x00C, 0, ENABLE);//enable timer
}
void startTouchClock(){
	Function handles[] = {sampleTouch_handle};
	initializeTouchClock();
	touchTimerInterruptInterface = new_GPTMInterrupt(6, 0, handles, true, false, false, false, false);
	NVIC_registerGPTMInterrupt(touchTimerInterruptInterface);
}
void startTouchSense(BYTE interruptPort, BYTE interruptPin){
	interruptInterface = new_GPIOInterrupt(interruptPort, interruptPin, TOUCH_handle, EDGE, BOTH_EDGE, LOW);
	NVIC_registerGPIOInterrupt(interruptInterface);
	GPIO_initPortPinForInterrupt(interruptInterface);
};
void startSerialCommunications(BYTE SSI_moduleNumber){
	serialInterface = new_SSISCModule
		(new_SSIMasterModule(SSI_moduleNumber, FREESCALE, 200000, 8, false,true,true)//I think true 3rd arg
		,100//Rx buffer size
		,100//Tx buffer size
		,TOUCH_dataReceived_handle
	); 
	//Send Data to start TouchScreen
	SSISC_transmit(serialInterface->p_SSIModule->moduleNumber, commandSize, TOUCH_init);
}
void TOUCH_initialize(BYTE interruptPort, BYTE interruptPin, BYTE SSI_moduleNumber){
	startTouchClock();
	startTouchSense(interruptPort, interruptPin);
	startSerialCommunications(SSI_moduleNumber);
}

void TOUCH_sendGetXYInstruction(){
	SSISC_transmit(serialInterface->p_SSIModule->moduleNumber, commandSize, TOUCH_getX);
	SSISC_transmit(serialInterface->p_SSIModule->moduleNumber, commandSize, TOUCH_getY);
}
void TOUCH_handle(unsigned char moduleNumber)
{
	currentlyTouched = (bool)(!((bool)getBit(GPIO_getPortAddress(interruptInterface->encodedPort), 0x3FC, interruptInterface->base.bit)));
	if(currentlyTouched)
			write_word(touchTimerAddr, 0x018, 0x00000001);//enable touchclockinterrupt
	else
			write_word(touchTimerAddr, 0x018, 0x00000000);//disable touchclockinterrupt
}

void TOUCH_dataReceived_handle(unsigned char moduleNumber)
{
	unsigned int runningTotalX = 0;
	unsigned int runningTotalY = 0;
	
	unsigned int tempX = 0;
	unsigned int tempY = 0;
	
	unsigned int count = 0;
	
	while(SSISC_bufferNotEmpty(serialInterface->p_SSIModule->moduleNumber, Rx)){
		//delete first byte
		tempX = SSISC_getNextRx(serialInterface->p_SSIModule->moduleNumber);
		tempY = SSISC_getNextRx(serialInterface->p_SSIModule->moduleNumber);
		//save second byte
		tempX = (SSISC_getNextRx(serialInterface->p_SSIModule->moduleNumber)<<5);
		tempY = (SSISC_getNextRx(serialInterface->p_SSIModule->moduleNumber)<<5);
		//save third byte
		tempX = (SSISC_getNextRx(serialInterface->p_SSIModule->moduleNumber)>>3);
		tempY = (SSISC_getNextRx(serialInterface->p_SSIModule->moduleNumber)>>3);
		
		runningTotalX += tempX;
		runningTotalY += tempY;
		
		++count;
	}
	averageX = runningTotalX/count;
	averageY = runningTotalY/count;
	//PROCESS DATA

}

void sampleTouch_handle(unsigned char moduleNumber)
{
	int i = 0;
	for(i =0; i < 12; ++i)
		TOUCH_sendGetXYInstruction();
}
