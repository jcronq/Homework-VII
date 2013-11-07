/*
Author: Brian Carrick
*/
#ifndef I2C_h
# define I2C_h
//include
#	ifndef JLib_h
#		include "JLib.h"
#	endif
#	ifndef GPIO_h
#		include "GPIO.h"
#	endif


struct I2C_module{
	//unsigned int I2CCLK;
	
	unsigned char I2CmoduleNumber;
	unsigned char I2Cport;
	unsigned char I2Cpin;	
	
	BYTE_ADDRESS I2CmoduleAddress;
	BYTE_ADDRESS I2CportAddress;
	unsigned char I2CpinMap;	
};

struct I2C_module* I2C_enableModule(struct I2C_module*, bool);
struct I2C_module* I2C_masterModule(unsigned char moduleNumber, unsigned char port, unsigned char pin);



void I2C_setMasterMode(struct I2C_module* module);
void I2C_configureClock(struct I2C_module* module);
void I2C_setSlaveAddress(struct I2C_module* module, BYTE slaveAddress);
void I2C_setDataRegister(struct I2C_module* module, BYTE data);
BYTE I2C_getDataRegister(struct I2C_module* module);

bool I2C_sendDataBytes(struct I2C_module* module, BYTE slaveAddress, BYTE data[], unsigned int count);
bool I2C_receiveDataBytes(struct I2C_module* module, BYTE slaveAddress, BYTE data[], unsigned int count);


bool I2C_checkBusBusyStatus(struct I2C_module* module);
bool I2C_checkBusyStatus(struct I2C_module* module);
bool I2C_checkForACK(struct I2C_module* module);

#endif
