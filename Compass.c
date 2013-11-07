#include "Compass.h"
/*
Author: Brian Carrick
*/

struct I2C_module* configCompass()
{
	// initialize I2C0 for compass
	struct I2C_module* Compass;
	volatile WORD test = 0;
	Compass = I2C_masterModule(0,'b',0x0C);
	I2C_enableModule(Compass, ENABLE);
	writeEeprom(Compass, 0x06, 0x0F); // set compass to average 16 times
	writeEeprom(Compass, 0x08, 0x72); // set to continuous mode, 20HZ, Periodic set/reset
	
	return Compass;
}

void writeEeprom(struct I2C_module* Compass, BYTE command, BYTE data)
{
	int count = 0;
	BYTE dat[3];
	dat[0] = 0x77;
	dat[1] = command;
	dat[2] = data;	
	count = sizeof(dat);
	I2C_sendDataBytes(Compass,0x42, dat, count);
}


WORD readEeprom(struct I2C_module* Compass, BYTE command)
{
	int count = 2;
	BYTE returnData[1];
	BYTE data[2];
	data[0] = 0x67;
	data[1] = command;
	count = sizeof(data);
	
	I2C_sendDataBytes(Compass, 0x42, data, count);
	I2C_receiveDataBytes(Compass, 0x43, returnData, 1);	
	return returnData[0];
}

void writeRam(struct I2C_module* Compass, BYTE command, BYTE data)
{
	int count = 0;
	BYTE dat[3];
	dat[0] = 0x47;
	dat[1] = command;
	dat[2] = data;	
	count = sizeof(dat);
	I2C_sendDataBytes(Compass,0x42, dat, count);
}

WORD readRam(struct I2C_module* Compass, BYTE command)
{
	int count = 2;
	BYTE returnData[1];
	BYTE data[2];
	data[0] = 0x67;
	data[1] = command;
	count = sizeof(data);
	
	I2C_sendDataBytes(Compass, 0x42, data, count);
	I2C_receiveDataBytes(Compass, 0x43, returnData, 1);	
	return returnData[0];	
}

Heading readDataContinuous(struct I2C_module* Compass)
{
	Heading measurement;
	int count = 2;
	BYTE returnData[2];
	
	I2C_receiveDataBytes(Compass, 0x43, returnData, 2);	
	measurement.upper = returnData[0];
	measurement.lower = returnData[1];
	measurement.fullHeading = ((returnData[0]<<8) | returnData[1]);	
	measurement.degHeading = measurement.fullHeading / 10;
	measurement.tenthDegHeading = measurement.fullHeading % 10;
	return measurement;
}




