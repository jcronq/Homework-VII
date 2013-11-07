/*
Author: Jason Cronquist
Date: 10/17/2013

notes: Has no ability to check to make sure ports don't overlap.
I intend to implement a feature that will handle this problem.
For now, use CAUTION when designing pin maps.

This file, and it's matching c file, contain function calls for
initializing and using up to 8 (module 0 through 7) UART modules
*/

#ifndef UART_h
#define UART_h

#include "JLib.h"
#include "GPIO.h"
#include "SystemClock.h"
#include <math.h>
#include <stdlib.h>

typedef enum {FirstEigth, FirstQuarter, Half, ThirdQuarter, LastEigth, DefaultFIFOLevel} FIFOLevelSelect; 

struct UART_module{
	//User Readable/Set values
	unsigned char moduleNumber;
	unsigned int baudRate;
	unsigned char wordLength;
	unsigned char port;
	
	//Auto Generated Values
	BYTE pinMap;
	BYTE encodedWordLength;
	BYTE_ADDRESS UART_port;
	BYTE_ADDRESS GPIO_port;
	
	FIFOLevelSelect RxFullAt;
	FIFOLevelSelect TxFullAt;
};

struct UART_module* UART_newModule(unsigned char moduleNumber, unsigned int baudRate, unsigned char wordLength, FIFOLevelSelect RxFullAt, FIFOLevelSelect TxFullAt);

//FUNCTIONS FOR USER
	//initialization function
	struct UART_module* UART_initialize(unsigned char moduleNumber, unsigned int baudRate, unsigned char wordLength, FIFOLevelSelect RxFullAt, FIFOLevelSelect TxFullAt);
	void UART_initModule(struct UART_module* module);
	
	//Module Read/Write
	BYTE UART_getNextByte(struct UART_module* module);
	void UART_sendNextByte(struct UART_module* module, BYTE valueToSend);

	//state checking functions	
	bool UART_TxQueueEmpty(struct UART_module* module);
	bool UART_TxQueueFull(struct UART_module* module);
	bool UART_RxQueueEmpty(struct UART_module* module);
	bool UART_RxQueueFull(struct UART_module* module);
	bool UART_isBusy(struct UART_module* module);

//INTERNALLY USED FUNCTIONS
	//initialization
	void UART_initClockGatingUART(struct UART_module* module);
	void UART_initClockGatingGPIO(struct UART_module* module);
	void UART_configure(struct UART_module* module);
	float calculateBRD(struct UART_module*);
	int getIntBRD(float BRD);
	int getFracBRD(float BRD);
	
//configuration
	//General
	void UART_enableDisableModule(struct UART_module* module, bool enable);
	void UART_setIntegerPartOfBRD(struct UART_module* module, HALF_WORD intPartBrd);
	void UART_setFractionalPartOfBRD(struct UART_module* module, HALF_WORD fracPartBRD);
	void UART_enableDisableFIFO(struct UART_module* module, bool enable);
	void UART_setWordLength(struct UART_module* module);
	void UART_setClockSourceTo_systemClock(struct UART_module* module);
	void UART_setClockSourceTo_PIOSC(struct UART_module* module);
	//Interrupt
	void UART_allowTxFIFOInterrupt(struct UART_module* module, bool enable);
	void UART_allowRxFIFOInterrupt(struct UART_module* module, bool enable);
	void UART_allowRxTimeoutInterrupt(struct UART_module* module, bool enable);
	void UART_allowRxParityErrorInterrupt(struct UART_module* module, bool enable);
	void UART_setTxEmptyLevel(struct UART_module* module);
	void UART_setRxFullLevel(struct UART_module* module);
	void UART_useCustomRxFullLevel(struct UART_module* module, bool enable);

	//random
	BYTE_ADDRESS UART_getUARTAddress(unsigned char moduleNumber);
	BYTE_ADDRESS UART_getGPIOAddress(unsigned char moduleNumber);
	bool UART_checkFlag(struct UART_module*, BYTE comparer);


#endif
