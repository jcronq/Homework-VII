/*
Author: Jason Cronquist
*/
#include "UART.h"

static const unsigned char NUM_UART_MODULES = 8;

static BYTE_ADDRESS UART_address[NUM_UART_MODULES] = 
		{((BYTE_ADDRESS)0x4000C000)
		,((BYTE_ADDRESS)0x4000D000)
		,((BYTE_ADDRESS)0x4000E000)
		,((BYTE_ADDRESS)0x4000F000)
		,((BYTE_ADDRESS)0x40010000)
		,((BYTE_ADDRESS)0x40011000)
		,((BYTE_ADDRESS)0x40012000)
		,((BYTE_ADDRESS)0x40013000)
		};

//ADDRESSES
static BYTE_ADDRESS SYSTEM_CONTROL = ((BYTE_ADDRESS)0x400FE000);
//OFFSETS
static const HALF_WORD UARTCTL  = 0x030;//UART Control
static const HALF_WORD UARTIBRD = 0x024;//UART Integer BaudRateDivisor
static const HALF_WORD UARTFBRD = 0x028;//UART Fraction BaudRateDivisor
static const HALF_WORD UARTLCRH = 0x02C;//UART Line Control
static const HALF_WORD UARTCC   = 0xFC8;//UART Clock Configuration
static const HALF_WORD UARTFR   = 0x018;//UART Flag Register
static const HALF_WORD RCGCUART = 0x618;//UART RunModeClockGatingControl
static const HALF_WORD UARTDR   = 0x000;//UART Data Register
static const HALF_WORD UARTIFLS = 0x034;//UART Interrupt FIFO Level Select
static const HALF_WORD UARTIM   = 0x038;//UART Interrupt Mask
		
//BIT LOCATION
	//UARTLCRH
	static const BYTE FEN_bit  = 4;
	static const BYTE WLEN_bit = 5;
	//UARTFR
	static const BYTE BUSY_bit = 3;
	static const BYTE RXFE_bit = 4;
	static const BYTE TXFF_bit = 5;
	static const BYTE RXFF_bit = 6;
	static const BYTE TXFE_bit = 7;
	//UARTCTL
	static const BYTE HSE_bit  = 5;
	static const BYTE EOT_bit  = 4;
	//UARTIFLS
	static const BYTE RXIFLSEL_bit = 3;
	static const BYTE TXIFLSEL_bit = 0;
	//UARTIM
	static const BYTE PEIM_bit = 8; //parity error
	static const BYTE RTIM_bit = 6; //timeout
	static const BYTE TXIM_bit = 5; //TransmitEmpty
	static const BYTE RXIM_bit = 4; //Receive Full
	
	
static const BYTE UART_pinMap[NUM_UART_MODULES] = {0x03, 0x03, 0xC0, 0xC0, 0x30, 0x30, 0x30, 0x03};
static const BYTE UART_portMap[NUM_UART_MODULES] = {'a', 'b', 'd', 'c', 'c', 'e', 'd', 'e'};
static const BYTE BRD_ClkDiv[2] = {16, 8};

//CLEAR VALUES
	//UARTLCRH
	static const WORD WLEN_clear = (WORD)0x00000060;
	//UARTIFLS
	static const WORD RXIFLSEL_clear = (WORD)0x00000038;
	static const WORD TXIFLSEL_clear = (WORD)0x00000007;

struct UART_module* UART_initialize(unsigned char moduleNumber, unsigned int baudRate, unsigned char wordLength, FIFOLevelSelect RxFullAt, FIFOLevelSelect TxFullAt)
{
	struct UART_module* module = UART_newModule(moduleNumber, baudRate, wordLength, RxFullAt, TxFullAt);
	UART_initModule(module);
	return module;
}
void UART_initModule(struct UART_module* module)
{
	UART_initClockGatingUART(module);
	UART_initClockGatingGPIO(module);
	GPIO_initPortPinsForUART(module->GPIO_port, module->pinMap);
	UART_configure(module);
}
struct UART_module* UART_newModule(unsigned char moduleNumber, unsigned int baudRate, unsigned char wordLength, FIFOLevelSelect RxFullAt, FIFOLevelSelect TxFullAt)
{
	struct UART_module* retVal = (struct UART_module*)malloc(sizeof(struct UART_module*));
	if(moduleNumber <NUM_UART_MODULES){
		retVal->moduleNumber = moduleNumber;
		retVal->baudRate = baudRate;
		retVal->wordLength = wordLength;
		retVal->port = UART_portMap[moduleNumber];
		
		retVal->pinMap = UART_pinMap[moduleNumber];		
		if(wordLength >4 && wordLength <=8)
			retVal->encodedWordLength = wordLength - 5;
		else
			retVal->encodedWordLength = 3;
		retVal->UART_port = UART_getUARTAddress(moduleNumber);
		retVal->GPIO_port = UART_getGPIOAddress(moduleNumber);
		
		retVal->RxFullAt = RxFullAt;
		retVal->TxFullAt = TxFullAt;
	}
	return retVal;
}

void UART_initClockGatingUART(struct UART_module* module)
{
	setBit_word(SYSTEM_CONTROL, RCGCUART, module->moduleNumber, ENABLE);
}
void UART_initClockGatingGPIO(struct UART_module* module)
{
	GPIO_initPortClock(module->port, ENABLE);
}

void UART_configure(struct UART_module* module)
{	
	float BRD = calculateBRD(module);
	int intPartBRD = getIntBRD(BRD);
	int fracPartBRD = getFracBRD(BRD);
	
	UART_enableDisableModule(module, DISABLE);
	UART_setIntegerPartOfBRD(module, (HALF_WORD) intPartBRD);
	UART_setFractionalPartOfBRD(module, (HALF_WORD) fracPartBRD);
	UART_enableDisableFIFO(module, ENABLE);
	UART_setWordLength(module);
	UART_setClockSourceTo_systemClock(module);
	if(module->RxFullAt != DefaultFIFOLevel){
		UART_useCustomRxFullLevel(module, ENABLE);
		UART_setRxFullLevel(module);
	}
	if(module->TxFullAt != DefaultFIFOLevel){
		UART_setTxEmptyLevel(module);
	}
	UART_enableDisableModule(module, ENABLE);	
}
//**************************************************************************//
//								UART Configuration Functions
	//General
	void UART_enableDisableModule(struct UART_module* module, bool enable)
	{
		setBit_word(module->UART_port, UARTCTL, 0, enable);
	}
	void UART_setIntegerPartOfBRD(struct UART_module* module, HALF_WORD intPartBrd)
	{
		write_halfWord(module->UART_port, UARTIBRD,(HALF_WORD)intPartBrd);
	}
	void UART_setFractionalPartOfBRD(struct UART_module* module, HALF_WORD fracPartBRD)
	{
		write_byte(module->UART_port, UARTFBRD,(BYTE)fracPartBRD);	
	}
	void UART_enableDisableFIFO(struct UART_module* module, bool enable)
	{
		setBit_word(module->UART_port, UARTLCRH, FEN_bit, enable);
	}
	void UART_setWordLength(struct UART_module* module)
	{
		selectiveClearAndWrite_word(module->UART_port, UARTLCRH, WLEN_clear, (module->encodedWordLength<<WLEN_bit));
	}
	void UART_setClockSourceTo_systemClock(struct UART_module* module)
	{
		write_byte(module->UART_port, UARTCC, 0x00);
	}
	void UART_setClockSourceTo_PIOSC(struct UART_module* module)
	{
		write_byte(module->UART_port, UARTCC, 0x05);
	}
	
	//INTERRUPTS
	void UART_allowRxParityErrorInterrupt(struct UART_module* module, bool enable){
		setBit_word(module->UART_port, UARTIM, PEIM_bit, enable);
	}
	void UART_allowRxFullInterrupt(struct UART_module* module, bool enable){
		setBit_word(module->UART_port, UARTIM, RXIM_bit, enable);
	}
	void UART_allowTxEmptyInterrupt(struct UART_module* module, bool enable){
		setBit_word(module->UART_port, UARTIM, TXIM_bit, enable);
	}
	void UART_allowRxTimeoutInterrupt(struct UART_module* module, bool enable){
		setBit_word(module->UART_port, UARTIM, RTIM_bit, enable);
	}
	void UART_useCustomRxFullLevel(struct UART_module* module, bool enable){
		setBit_word(module->UART_port, UARTCTL, EOT_bit, !enable);
	}
	void UART_setTxEmptyLevel(struct UART_module* module){
		selectiveClearAndWrite_word(module->UART_port, UARTIFLS, TXIFLSEL_clear, (module->TxFullAt)<<TXIFLSEL_bit);
	}
	void UART_setRxFullLevel(struct UART_module* module){
		selectiveClearAndWrite_word(module->UART_port, UARTIFLS, RXIFLSEL_clear, (module->RxFullAt)<<RXIFLSEL_bit);
	}
//************************************************************************//
//								State Checking Functions
//returns true if write queue is empty
bool UART_TxQueueEmpty (struct UART_module* module)
{
	return UART_checkFlag(module, TXFE_bit);
}
//return true if write queue is full
bool UART_TxQueueFull (struct UART_module* module)
{
	return UART_checkFlag(module, TXFF_bit);
}
//returns true if read queue is empty
bool UART_RxQueueEmpty (struct UART_module* module)
{
	return UART_checkFlag(module, RXFE_bit);
}
//returns true if read queue is full
bool UART_RxQueueFull (struct UART_module* module)
{
	return UART_checkFlag(module, RXFF_bit);
}
//returns true if UART module is currently sending/receiving
bool UART_isBusy (struct UART_module* module)
{
	return UART_checkFlag(module, BUSY_bit);
}

//always check to make sure READ_QUEUE is NOT EMPTY before calling
BYTE UART_getNextByte(struct UART_module* module)
{//Not sure if this has error for word lengths other than 8
	return read_byte(module->UART_port, UARTDR);
}
//Always check to make sure WRITEQUEUE is NOT FULL before calling
void UART_sendNextByte(struct UART_module* module, BYTE value)
{
	write_byte(module->UART_port, UARTDR, value);
}
bool UART_checkFlag(struct UART_module* module, BYTE bitLocation)
{
	BYTE flag = read_byte(module->UART_port, UARTFR);
	flag &= (((BYTE)0x01) << bitLocation);
	if(flag != 0)
		return true;
	else
		return false;
}
BYTE_ADDRESS UART_getUARTAddress(unsigned char moduleNumber)
{
	if(moduleNumber < NUM_UART_MODULES)
		return UART_address[moduleNumber];
	return 0;
}
BYTE_ADDRESS UART_getGPIOAddress(unsigned char moduleNumber)
{ 
	if(moduleNumber < NUM_UART_MODULES)
		return GPIO_getPortAddress(UART_portMap[moduleNumber]);
	return 0;
}
float calculateBRD(struct UART_module* module)
{
	float retVal = (float)SysClk_getSysClkFreq();
	float divisor = (float)module->baudRate;
	//ClkDiv = 16 if HSE is clear, 8 if HSE is set (in UART CTRL)
	divisor *= (float)BRD_ClkDiv[getBit(module->UART_port, UARTCTL, HSE_bit)];
	retVal /= divisor;
	return retVal;
}
int getIntBRD(float BRD)
{
	return (int)BRD;
}
int getFracBRD(float BRD)
{
	BRD -= (float)((int)BRD);
	return (int)((BRD * 64) + 0.5f);
}
