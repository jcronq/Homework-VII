/*
Author: Jason Cronauist
*/
#ifndef SSI_h
#	define SSI_h
//include

#include "JLib.h"
#include "GPIO.h"
#include "SystemClock.h"
#include <stdlib.h>
#include <cmath>
#include <climits>

//GLOBAL VARIABLES
#ifndef MASTER
#	define MASTER true
#endif
#ifndef SLAVE 
#	define SLAVE false
#endif

typedef enum {FREESCALE, TI_SYNCHRONOUS_SERIAL_FRAME, MICROWIRE_FRAME} SSI_FRAME_FORMAT;

struct SSI_module{
	unsigned int SSIClk;

	bool master;
	bool output;

	unsigned char moduleNumber;
	unsigned char port;
	
	BYTE_ADDRESS moduleAddress;
	BYTE_ADDRESS portAddress;
	unsigned char pinMap;
	
	bool PIOSC;
	bool serialClockPhase;
	bool serialClockPolarity;
	SSI_FRAME_FORMAT frameFormat;
	unsigned char dataSize;
	
	BYTE SCR;
	BYTE prescale;
};

//using RAII (Resource Aquisition Is Initializaiont) so don't need this anymore.
//void SSI_enableModule(struct SSI_module*, bool);
struct SSI_module* new_SSIMasterModule(BYTE SSI_moduleNumber, SSI_FRAME_FORMAT ssiFF, unsigned int SSIClk, unsigned char dataSize, bool piosc_clkSrc, bool _serialClockPhase, bool _serialClockPolarity);

//internal functions
	void SSI_moduleStartStop(struct SSI_module* module, bool enable);
	void SSI_selectMasterSlave(struct SSI_module* module);
	void SSI_configureClockSource(struct SSI_module* module);
	void SSI_setClockPrescale(struct SSI_module* module);//Even value from 0-254
	void SSI_setSerialClockRate(struct SSI_module* module);//value from 0-255
	void SSI_setSSIParameters(struct SSI_module* module);
	void SSI_computePrescaleAndSCR(struct SSI_module* module);
	//interrupts
	void SSI_allowTxFIFOInterrupt(struct SSI_module* module, bool enable);
	void SSI_allowRxFIFOInterrupt(struct SSI_module* module, bool enable);
	void SSI_allowRxTimeoutInterrupt(struct SSI_module* module, bool enable);
	void SSI_setTxInterruptAtHalfEmpty(struct SSI_module* module, bool enable);
	void SSI_clearInterrupt(BYTE moduleNumber, BYTE bit);

//set Functions
void SSI_addDataToTXFIFO(struct SSI_module* module, HALF_WORD value);

//get functions
struct SSI_module* SSI_getModule(unsigned char moduleNumber);
HALF_WORD SSI_getNextValueFromRXQueue(unsigned char moduleNumber);

//FIFO checking
bool SSI_ReceiveNotEmpty(unsigned char moduleNumber);
bool SSI_TransmitNotFull(unsigned char moduleNumber);
bool SSI_isBusy(unsigned char moduleNumber);

void SSI_sendData(struct SSI_module* module, HALF_WORD msg);

#endif
