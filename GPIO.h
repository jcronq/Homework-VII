/*
Author: Jason Cronquist
*/
#ifndef GPIO_h
#define GPIO_h

#include "stdlib.h"
#include "NVIC.h"
#include "JLib.h"

#define IO_IN false
#define IO_OUT true
#define IO_PULL_UP true
#define IO_PULL_DOWN false
#define IO_OPEN_DRAIN true

typedef enum {DEFAULT_GPIOPCTL_OPTION, UART, SSI, I2C, JTAG_SWD} GPIOPCTL_OPTIONS;
typedef enum {GPIO_type,SSI_type} typeLL; 

//Clock Gating Functions
void GPIO_initPortClocksWithMap(BYTE portMap, bool enable);
void GPIO_initPortClock(unsigned char port, bool enable);

//Module Initialization Functions
void GPIO_initAndRegisterSSI(unsigned char port, unsigned char pin, unsigned char uID);
void GPIO_initPortPinsForIO(BYTE_ADDRESS port, BYTE pins, bool direction, bool pullUp, bool openDrain);
void GPIO_initPortPinsForSSI(BYTE_ADDRESS port, BYTE pins);
void GPIO_initPortPinsForUART(BYTE_ADDRESS port, BYTE pins);
void GPIO_initPortPinsForI2C(BYTE_ADDRESS port, BYTE pins);
void GPIO_initPortPinForInterrupt(struct NVIC_interruptModule_GPIO* module);

//Initialization Functions
	//General
	void GPIO_setDirectionPinMap(BYTE_ADDRESS port, BYTE pins, bool enable);
	void GPIO_setOpenDrainPinMap(BYTE_ADDRESS port, BYTE pins, bool enable);
	void GPIO_setDigEnPinMap(BYTE_ADDRESS port, BYTE pins, bool enable);
	void GPIO_setPullupPinMap(BYTE_ADDRESS port, BYTE pins, bool enable);
	void GPIO_setAltFuncPinMap(BYTE_ADDRESS port, BYTE pins, bool enable);
	void GPIO_setAltFuncPortControl(BYTE_ADDRESS port, BYTE pins, GPIOPCTL_OPTIONS option);
	void GPIO_unlockPort(BYTE_ADDRESS port, BYTE pins);//does nothing if no unlock required
	//Interrupts
	void GPIO_setInterruptSense(BYTE_ADDRESS port, unsigned char pin, InterruptSense);
	void GPIO_setBothEdges(BYTE_ADDRESS port, unsigned char pin, BothEdges orSingle_edge);
	void GPIO_setEdgeLevel(BYTE_ADDRESS port, unsigned char pin, EdgeLevel lowOrHigh);
	void GPIO_setInterruptMask(BYTE_ADDRESS port, unsigned char pin, bool enable);
	
//Set Functions
void GPIO_writeToPort(BYTE_ADDRESS port, BYTE toWrite);
void GPIO_writeToPin(BYTE_ADDRESS port, unsigned char pinNumber, bool value);
void GPIO_readModifyWriteByte(BYTE_ADDRESS port, BYTE pinMap, bool enable);

//Get Functions
bool GPIO_readFromPin(BYTE_ADDRESS port, unsigned char pinNumber);
BYTE GPIO_readFromPort(BYTE_ADDRESS port);
BYTE_ADDRESS GPIO_getPortAddress(unsigned char port);
unsigned char GPIO_getPortNumberFromAddress(BYTE_ADDRESS addr);

//Interrupt Status functions
unsigned char GPIO_getInterruptBitAndClear(unsigned char port);

//Useful Functions
unsigned char GPIO_standardizePort(unsigned char port);


#endif
