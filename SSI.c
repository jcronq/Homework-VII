#include "SSI.h"
/*
Author: Jason Cronquist
*/

//Number of SSI Modules available on uController
static const unsigned char NUM_MODULES = 4;
static BYTE_ADDRESS SSI_base[] =
		{((BYTE_ADDRESS)0x40008000)
		,((BYTE_ADDRESS)0x40009000)
		,((BYTE_ADDRESS)0x4000A000)
		,((BYTE_ADDRESS)0x4000B000)
		};
		
volatile static struct SSI_module* SSI_modules[NUM_MODULES]= {NULL};

static BYTE_ADDRESS SYSTEM_CONTROL = ((BYTE_ADDRESS)0x400FE000);
static const HALF_WORD RCGCSSI = 0x61C;//SSI Run Mode Clock Gating Control
static const HALF_WORD SSICR1  = 0x004;//SSI Control 1
static const HALF_WORD SSICR0  = 0x000;//SSI Control 1
static const HALF_WORD SSICC   = 0xFC8;//SSI Clock Configuration
static const HALF_WORD SSICPSR = 0x010;//SSI Clock Prescale
static const HALF_WORD SSIDR   = 0x008;//SSI Data Register
static const HALF_WORD SSISR   = 0x00C;//SSI Status Register
static const HALF_WORD SSIIM   = 0x014;//SSI Interrupt Mask
static const HALF_WORD SSIRIS  = 0x014;//SSI Raw Interrupt Status
static const HALF_WORD SSIMIS  = 0x01C;//SSI Masked Interrupt Status
static const HALF_WORD SSIICR  = 0x020;//SSI Interrupt Clear

		
//BIT LOCATIONS
	//SSICR1
	static const BYTE SSE_bit = 1;
	static const BYTE EOT_bit = 4;
	//SSICR0
	static const BYTE SCR_bit = 8;
	static const BYTE FRF_bit = 4;
	//SSISR
	static const BYTE RFF_bit = 3;//receive full
	static const BYTE RNE_bit = 2;//receive not empty
	static const BYTE TNF_bit = 1;//transmit fifo not full
	static const BYTE TFE_bit = 0;//transmit fifo empty
	static const BYTE BSY_bit = 4;//currently transmitting/recieving
	//SSIIM
	static const BYTE TXIM_bit = 3;//0: disallow Tx interrupt 1: allow
	static const BYTE RXIM_bit = 2;//0: disallow Rx interrupt 1: allow
	static const BYTE RTRIS_bit= 1;//0: disallow Rx timeout interrupt 1: allow

//CLEAR VALUES
	//SSICR0
	static const WORD SCR_clear = (WORD)0x0000FF00;
	static const BYTE SSICR0_params_clear = (BYTE)0xF0;
	
//Pin/Port maps
	static const BYTE SSI_pinMap[] = {0x3C, 0x0F, 0xF0, 0x0F};
	static const BYTE SSI_portMap[] = {'a', 'f', 'b', 'd'};


//*********************************************
//Enable's SSI_module structure passed to it
//manages:
//-Clock Gating for SSI MODULE
//-GPIO Port Initialization for SSI (if enable)
//*********************************************

//////////////////////////////////////////////////////////////////////////
//INPUTS: module (struct SSI_module*)
//					-Pointer to Module data structure
//				enable (bool)
//					-ENABLE : enables clock for gpio and SSI
//										sets alt func of gpio portPins
//					-DISABLE: disables clock for SSI module
//OUTPUTS:
//					returns SSI_module* from inputs
//////////////////////////////////////////////////////////////////////////
void SSI_enableModule(struct SSI_module* module, bool enable)
{
	//allow only valid moduleNumber to be initialized
	if(module->moduleNumber <= NUM_MODULES)
	{
		setBit_word(SYSTEM_CONTROL, RCGCSSI, module->moduleNumber, enable);
		if(enable)
		{
			GPIO_initPortClocksWithMap((0x01 << module->port), ENABLE);
			GPIO_initPortPinsForSSI(module->portAddress, module->pinMap);
			
			SSI_moduleStartStop(module, DISABLE);
			SSI_selectMasterSlave(module);
			SSI_configureClockSource(module);
			SSI_setClockPrescale(module);
			SSI_setSerialClockRate(module);
			SSI_setSSIParameters(module);
			SSI_moduleStartStop(module, ENABLE);
		}
	}
}
// END SSI_enableModule /////////////////////////////////////////////////////

BYTE_ADDRESS SSI_getModuleAddress(unsigned char moduleNumber)
{
	if(moduleNumber <= NUM_MODULES)
	{
		return SSI_base[moduleNumber];
	}
	return 0;
}
struct SSI_module* new_SSIMasterModule(BYTE SSI_moduleNumber, SSI_FRAME_FORMAT ssiFF, unsigned int SSIClk, unsigned char dataSize, bool piosc_clkSrc, bool _serialClockPhase, bool _serialClockPolarity)
{
	struct SSI_module* retVal = malloc(sizeof *retVal);
	
	retVal->SSIClk = SSIClk;
	
	retVal->master = true;
	retVal->output = true;
	
	retVal->moduleNumber = SSI_moduleNumber;

	retVal->port = (SSI_portMap[SSI_moduleNumber]-'a');
	
	retVal->moduleAddress = SSI_getModuleAddress(SSI_moduleNumber);
	retVal->portAddress = GPIO_getPortAddress(SSI_portMap[SSI_moduleNumber]);
	retVal->pinMap = SSI_pinMap[SSI_moduleNumber];
	
	retVal->PIOSC = piosc_clkSrc;
	retVal->serialClockPhase = _serialClockPhase;
	retVal->serialClockPolarity = _serialClockPolarity;
	retVal->frameFormat = ssiFF;
	if(dataSize < 4)
		dataSize = 4;
	else if(dataSize >16)
		dataSize = 16;
	retVal->dataSize =  dataSize-1;
	
	SSI_computePrescaleAndSCR(retVal);
	
	SSI_modules[SSI_moduleNumber] = retVal;
	
	SSI_enableModule(retVal, ENABLE);
	
	return retVal;
}
void SSI_destroyModule(struct SSI_module* module)
{
	unsigned char moduleNumber = module->moduleNumber;
	free((struct SSI_module*)SSI_modules[moduleNumber]);
	SSI_modules[moduleNumber] = NULL;
}
//struct SSI_module* SSI_newSlaveModule(unsigned char moduleNumber, unsigned char port, unsigned char pin, SSI_FRAME_FORMAT ssiFF, unsigned int SSIClk, unsigned char dataSize, bool output, bool piosc_clkSrc, bool serialClockPhase, bool serialClockPolarity)
//{
//	struct SSI_module* retVal = SSI_newMasterModule(moduleNumber, port, pin, ssiFF, SSIClk, dataSize, piosc_clkSrc, serialClockPhase, serialClockPolarity);
//	
//	retVal->master = false;
//	retVal->output = output;
//	
//	return retVal;
//}
void SSI_moduleStartStop(struct SSI_module* module, bool enable)
{
	setBit_word(module->moduleAddress, SSICR1, SSE_bit, enable);
}
void SSI_setSlave(struct SSI_module* module)
{
	if(module->output)
		write_word(module->moduleAddress, SSICR1, (WORD)0x04);
	else
		write_word(module->moduleAddress, SSICR1, (WORD)0x0C);	
}
void SSI_selectMasterSlave(struct SSI_module* module)
{
	if(module->master)
		write_word(module->moduleAddress, SSICR1,(WORD)0x00);
	else
		SSI_setSlave(module);
}
void SSI_configureClockSource(struct SSI_module* module)
{
	if(module->PIOSC)
		write_byte(module->moduleAddress,SSICC,(BYTE)0x05);
	else
		write_byte(module->moduleAddress,SSICC,(BYTE)0x00);
}
void SSI_setClockPrescale(struct SSI_module* module)//Even value 2-254
{
	write_byte(module->moduleAddress, SSICPSR, module->prescale);
}
void SSI_setSerialClockRate(struct SSI_module* module)//Any value 0-255
{
	selectiveClearAndWrite_word(module->moduleAddress, SSICR0, SCR_clear,(module->SCR<<SCR_bit));
}
void SSI_allowTxFIFOInterrupt(struct SSI_module* module, bool enable){
	setBit_word(module->moduleAddress, SSIIM, TXIM_bit, enable);
}
void SSI_allowRxFIFOInterrupt(struct SSI_module* module, bool enable){
	setBit_word(module->moduleAddress, SSIIM, RXIM_bit, enable);	
}
void SSI_allowRxTimeoutInterrupt(struct SSI_module* module, bool enable){
	setBit_word(module->moduleAddress, SSIIM, RTRIS_bit, enable);
}
void SSI_setTxInterruptAtHalfEmpty(struct SSI_module* module, bool enable){
	setBit_word(module->moduleAddress, SSICR1, EOT_bit, DISABLE);
}
void SSI_clearInterrupt(BYTE moduleNumber, BYTE bit){
	setBit_word(SSI_base[moduleNumber], SSIICR, bit, ENABLE);
}
void SSI_computePrescaleAndSCR(struct SSI_module* module)
{//BRUTE FORCE ALGORITHM - Slow, only runs once per SSI_module however.
	volatile WORD systemClock = 0;
	int i = 0;
	int j = 0;
	int tempDiff = 0;
	int difference = INT_MAX;
	BYTE SCR = 0;
	BYTE prescale = 0;
	systemClock = SysClk_getSysClkFreq();
	for(i = 2; i < 256; i+=2)
	{
		for(j = 0; j < 256; ++j)
		{
			tempDiff = abs(((int)(((float)systemClock)/(((float)i) * (1.0f + ((float)j)))))- module->SSIClk);
			if(tempDiff < difference)
			{
				prescale = i;
				SCR = j;
				difference = tempDiff;
			}
		}
	}
	module->prescale = prescale;
	module->SCR = SCR;
}
void SSI_setSSIParameters(struct SSI_module* module)
{
	//serial clock rate (SCR)
	//Desired clock phase/polarity, if using (Freescale Spi mode(SPH and spo))
	//protocol mode: Freescale SPI, TI SSF, MICROWIRE(FRF)
	//data size (DSS)
	BYTE ctrl0Params= 0x00;
	if(module->serialClockPhase)
		ctrl0Params |= 0x80;
	if(module->serialClockPolarity)
		ctrl0Params |= 0x40;
	ctrl0Params |= ((BYTE)module->frameFormat)<<FRF_bit;
	ctrl0Params |= module->dataSize;
	selectiveClearAndWrite_byte(module->moduleAddress, SSICR0, SSICR0_params_clear, ctrl0Params);
}

bool SSI_ReceiveFull(struct SSI_module* module)
{
	return (bool)getBit(module->moduleAddress, SSISR, RFF_bit);
}
bool SSI_TransmitNotFull(BYTE moduleNumber)
{
	return (bool)getBit(SSI_modules[moduleNumber]->moduleAddress, SSISR, TNF_bit);
}
bool SSI_ReceiveNotEmpty(BYTE moduleNumber)
{
	return (bool)getBit(SSI_modules[moduleNumber]->moduleAddress, SSISR, RNE_bit);
}
bool SSI_isBusy(BYTE moduleNumber)
{
	return (bool)getBit(SSI_modules[moduleNumber]->moduleAddress, SSISR, BSY_bit);
}
void SSI_addDataToTXFIFO(struct SSI_module* module, HALF_WORD value)
{
	write_halfWord(module->moduleAddress, SSIDR, value);
}
HALF_WORD SSI_getNextValueFromRXQueue(unsigned char moduleNumber)
{
	return read_halfWord(SSI_modules[moduleNumber]->moduleAddress, SSIDR);
}

void SSI_sendData(struct SSI_module* module, HALF_WORD msg)
{
	write_halfWord(module->moduleAddress, SSIDR, msg);
}
