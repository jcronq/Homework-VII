#ifndef NVIC_h
#define NVIC_h

#include "stdlib.h"
#include "JLib.h"

typedef enum {EDGE, LEVEL} InterruptSense;
typedef enum {SINGLE_EDGE, BOTH_EDGE} BothEdges;
typedef enum {LOW, HIGH} EdgeLevel;

struct NVIC_interruptModule
{
	unsigned char bit;
	Function f;
};
struct NVIC_interruptModule_GPIO
{
	unsigned char port;
	struct NVIC_interruptModule base;
	
	InterruptSense edgeOrLevel; 
	BothEdges orSingle_edge;
  EdgeLevel highOrLow;
	
	BYTE encodedPort;
};
struct NVIC_interruptModule_SSI
{
	unsigned char moduleNumber;
	BYTE_ADDRESS moduleAddress;
	
	unsigned char numInterrupts;
	struct NVIC_interruptModule* base;
	
	bool RxFIFOHalfFull;
	bool TxFIFOHalfEmpty;
	bool RxTimeOut;
	bool RxOverrun;
};
struct NVIC_interruptModule_UART
{
	unsigned char moduleNumber;
	BYTE_ADDRESS moduleAddress;
	
	unsigned char numInterrupts;
	struct NVIC_interruptModule* base;
	
	bool RxFIFOHalfFull;
	bool TxFIFOHalfEmpty;
	bool RxTimeOut;
	bool RxOverrun;
};
struct NVIC_interruptModule_GPTM
{
	unsigned char timerNumber;
	unsigned char aOrb;
	BYTE_ADDRESS moduleAddress;
	
	unsigned char numInterrupts;
	struct NVIC_interruptModule* base;
	
	bool timeOut;
	bool captureMode;
	bool captureModeEdge;
	bool timerMatch;
	bool RTC;
};
struct NVIC_interruptModule_ADC
{
  unsigned char moduleNumber;
  unsigned char sequenceNumber;
  BYTE_ADDRESS moduleAddress;
  
  unsigned char numInterrupts;
  struct NVIC_interruptModule* base;
  bool DigitalComparatorInterrupt;
	//more interrupt options required
};
struct NVIC_GPIOInterruptDirectory
{
	Function pinInterrupts[8];
};
struct NVIC_SSIInterruptDirectory
{
	Function ssiInterrupts[4];
};
struct NVIC_UARTInterruptDirectory
{
	Function uartInterrupts[9];
};
struct NVIC_ADCInterruptDirectory
{
    Function adcInterrupts[5];
};
struct NVIC_GPTM_A_B
{
	Function a;
	Function b;
};
struct NVIC_GPTMInterruptDirectory
{
	struct NVIC_GPTM_A_B gptmInterrupts[4];
	Function RTCMIS;
};

struct NVIC_interruptModule_GPIO* new_GPIOInterrupt(unsigned char port, unsigned char pin, Function f, InterruptSense edgeOrLevel, BothEdges orSingle_edge, EdgeLevel highOrLow);
struct NVIC_interruptModule_GPTM* new_GPTMInterrupt(BYTE timerNumber, BYTE aOrb, Function* f, bool timeOut, bool captureMode, bool captureModeEdge, bool timerMatch, bool RTC);
struct NVIC_interruptModule_SSI* new_SSIInterrupt(BYTE moduleNumber, Function* f, bool RxFIFOHalfFull, bool TxFIFOHalfEmpty, bool RxTimeOut, bool RxOverrun);
struct NVIC_interruptModule_UART* new_UARTInterrupt(BYTE moduleNumber, Function* f, bool RxFIFOFull, bool TxFIFOEmpty, bool RxTimeOut, bool ParityError, bool breakError, bool _9BitMode, bool RxOverrun);

//Register Interrupt Functions
void NVIC_registerGPIOInterrupt(struct NVIC_interruptModule_GPIO* module);
void NVIC_registerSSIInterrupt(struct NVIC_interruptModule_SSI* module);
void NVIC_registerUARTInterrupt(struct NVIC_interruptModule_UART* module);
void NVIC_registerGPTMInterrupt(struct NVIC_interruptModule_GPTM* module);

//Root interrupt Data
bool NVIC_isEnabled(HALF_WORD interruptNumber);
void NVIC_setEnable(HALF_WORD interruptNumber, bool enable);
bool NVIC_isPending(HALF_WORD interruptNumber);
void NVIC_setPending(HALF_WORD interruptNumber, bool enable);
void NVIC_setPriority(HALF_WORD interruptNumber, BYTE priority);
bool NVIC_isActive(HALF_WORD interruptNumber);

//Get NVIC specific data
HALF_WORD NVIC_getInterruptNumber_SSI(unsigned char moduleNumber);
HALF_WORD NVIC_getInterruptNumber_GPIO(unsigned char portNumber);
HALF_WORD NVIC_getInterruptNumber_GPTM(unsigned char timerNumber);


void GPIOPortA_Handler(void);
void GPIOPortB_Handler(void);
void GPIOPortC_Handler(void);
void GPIOPortD_Handler(void);
void GPIOPortE_Handler(void);
void GPIOPortF_Handler(void);

#endif
