#ifndef SSISerialComms_h
#define SSISerialComms_h

#include "JLib.h"
#include "SSI.h"
#include "NVIC.h"
#include "GPIO.h"

typedef enum {Rx, Tx} RxTx;

struct SSISerialComms_module{
	struct SSI_module* p_SSIModule;
	struct NVIC_interruptModule_SSI* p_NVICModule;
	HALF_WORD bufferSize[2];
};

//Initialization Functions
//Activates and registers SSI_module, and its interruptModule.
//using RAII
struct SSISerialComms_module* new_SSISCModule(struct SSI_module* module, HALF_WORD bufferRx_size, HALF_WORD bufferTx_size, Function dataReceived);

//state checking functions
bool SSISC_bufferNotEmpty(unsigned char moduleNumber, RxTx RT);
bool SSISC_bufferFull(unsigned char moduleNumber, RxTx RT);

//set functions
void SSISC_transmit(unsigned char moduleNumber, HALF_WORD size, HALF_WORD * data);
void SSISC_ForceRxFIFOFlush(unsigned char moduleNumber);

//get functions
HALF_WORD SSISC_getNextRx(unsigned char moduleNumber);

//Internal functions
HALF_WORD SSISC_popBuffer(unsigned char moduleNumber, RxTx RT);
void SSISC_pushBuffer(unsigned char moduleNumber, HALF_WORD value, RxTx RT);

//Handles
void SSISC_Rx_FIFOFULL_Handler(unsigned char moduleNumber);
void SSISC_Tx_FIFOEMPTY_Handler(unsigned char moduleNumber);
void SSISC_callBackWrapper(unsigned char moduleNumber);

#endif
