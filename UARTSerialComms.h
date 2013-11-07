#ifndef UARTSerialComms_h
#define UARTSerialComms_h

#include "JLib.h"
#include "UART.h"
#include "NVIC.h"
#include "GPIO.h"

typedef enum {Rx, Tx} RxTx;

struct UARTSerialComms_module{
	struct UART_module* p_UARTModule;
	struct NVIC_interruptModule_UART* p_NVICModule;
	HALF_WORD bufferSize[2];
};

struct UARTSerialComms_module* new_UARTSCModule(struct UART_module* module, HALF_WORD bufferRx_size, HALF_WORD bufferTx_size, Function dataReceived);

//state checking functions
bool UARTSC_bufferNotEmpty(unsigned char moduleNumber, RxTx RT);
bool UARTSC_bufferFull(unsigned char moduleNumber, RxTx RT);

//set functions
void UARTSC_transmit(unsigned char moduleNumber, HALF_WORD size, BYTE* data);
void UARTSC_ForceRxFIFOFlush(unsigned char moduleNumber);

//get functions
BYTE UARTSC_getNextRx(unsigned char moduleNumber);

//Internal functions
BYTE UARTSC_popBuffer(unsigned char moduleNumber, RxTx RT);
void UARTSC_pushBuffer(unsigned char moduleNumber, BYTE value, RxTx RT);

//Handles
void UARTSC_Rx_FIFOFULL_Handler(unsigned char moduleNumber);
void UARTSC_Tx_FIFOEMPTY_Handler(unsigned char moduleNumber);
void UARTSC_parityError_Handler(unsigned char moduleNumber);
void UARTSC_callBackWrapper(unsigned char moduleNumber);

#endif
