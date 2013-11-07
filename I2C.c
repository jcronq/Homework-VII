#include "I2C.h"
/*
Author: Brian Carrick
*/
static BYTE_ADDRESS I2C0_base = ((BYTE_ADDRESS)0x40020000);
static BYTE_ADDRESS I2C1_base = ((BYTE_ADDRESS)0x40021000);
static BYTE_ADDRESS I2C2_base = ((BYTE_ADDRESS)0x40022000);
static BYTE_ADDRESS I2C3_base = ((BYTE_ADDRESS)0x40023000);

static BYTE_ADDRESS SYSTEM_CONTROL = ((BYTE_ADDRESS)0x400FE000);
static const HALF_WORD RCGCI2C = 0x620;//I2C run mode clock gating contorl
static const HALF_WORD I2CMSA = 0x000;//I2C Master Slave Address
static const HALF_WORD I2CMCS = 0x004;//I2C Master Control/Status
static const HALF_WORD I2CMDR = 0x008;//I2C Master Data
static const HALF_WORD I2CMTPR = 0x00C;//I2C Master Timer Period
static const HALF_WORD I2CMIMR = 0x010;//I2C Master Interrupt Mask
static const HALF_WORD I2CMICR = 0x01C;//I2C Master Interrupt Clear
static const HALF_WORD I2CMCR = 0x020;//I2C Master Configuration
static const HALF_WORD I2CMCLKOCNT = 0x024;//I2C Master Clock Low Timeout Count
static const HALF_WORD I2CMBMON = 0x02C;//I2C Master Bus Monitor
static const HALF_WORD I2CMCR2 = 0x038;//I2C Master Configuration 2

//Number of I2C Modules available on uController
static const unsigned char NUM_MODULES = 4;

//*********************************************
//Enable's I2C_module structure passed to it
//manages:
//-Clock Gating for I2C MODULE
//-GPIO Port Initialization for I2C (if enable)
//*********************************************

//////////////////////////////////////////////////////////////////////////
//INPUTS: module (struct I2C_module*)
//					-Pointer to Module data structure
//				enable (bool)
//					-ENABLE : enables clock for gpio and I2C
//										sets alt func of gpio portPins
//					-DISABLE: disables clock for I2C module
//OUTPUTS:
//					returns I2C_module* from inputs
//////////////////////////////////////////////////////////////////////////
struct I2C_module* I2C_enableModule(struct I2C_module* module, bool enable)
{
	//allow only valid moduleNumber to be initialized
	if(module->I2CmoduleNumber <= NUM_MODULES)
	{
		setBit_word(SYSTEM_CONTROL, RCGCI2C, module->I2CmoduleNumber, enable);
		if(enable)
		{
			GPIO_initPortClocksWithMap(module->I2Cport, ENABLE);
			GPIO_initPortPinsForI2C(module->I2CportAddress, module->I2CpinMap);
			
			I2C_setMasterMode(module);
			I2C_configureClock(module);			
			
		}
	}
	
	return module;
}
// END I2C_enableModule /////////////////////////////////////////////////////


BYTE_ADDRESS I2C_getModuleAddress(unsigned char moduleNumber)
{
	if(moduleNumber <= NUM_MODULES)
	{
		switch(moduleNumber)
		{
		case 0:
			return I2C0_base;
		case 1:
			return I2C1_base;
		case 2:
			return I2C2_base;
		case 3:
			return I2C3_base;
		default:
			return NULL;
		}
	}
	return NULL;
}


struct I2C_module* I2C_masterModule(unsigned char moduleNumber, unsigned char port, unsigned char pin)
{
	struct I2C_module* I2CretVal = malloc(sizeof(*I2CretVal));
			
	I2CretVal->I2CmoduleNumber = moduleNumber;
	if(port < 6)
		I2CretVal->I2Cport = port;
	else if(port-96 < 6)
		I2CretVal->I2Cport = (port-96);
	
	I2CretVal->I2Cpin = pin;
		
	
	I2CretVal->I2CmoduleAddress = I2C_getModuleAddress(moduleNumber);
	I2CretVal->I2CportAddress = GPIO_getPortAddress(port);
	I2CretVal->I2CpinMap = pin;
		
	return I2CretVal;
}


void I2C_setMasterMode(struct I2C_module* module)
{
	write_word(module->I2CmoduleAddress, I2CMCR, (BYTE)0x10); // set to master mode with glitch filter
	//write_word(module->moduleAddress, I2CMCR2, (BYTE)0x50);
}

void I2C_configureClock(struct I2C_module* module)
{
	write_word(module->I2CmoduleAddress, I2CMTPR, (BYTE)0x08);	
}

void I2C_setSlaveAddress(struct I2C_module* module, BYTE slaveAddress)
{
	write_word(module->I2CmoduleAddress, I2CMSA, slaveAddress);
}

void I2C_setDataRegister(struct I2C_module* module, BYTE data)
{
	write_byte(module->I2CmoduleAddress, I2CMDR, data);
}
BYTE I2C_getDataRegister(struct I2C_module* module)
{
	return read_byte(module->I2CmoduleAddress, I2CMDR);
}

bool I2C_checkBusBusyStatus(struct I2C_module* module)
{
	bool status = false;	
	BYTE temp = 0x00;
	temp = read_word(module->I2CmoduleAddress, I2CMCS);
	if(!((temp & 0x40)==0x40))
		status = true;
	return status;
}

bool I2C_checkBusyStatus(struct I2C_module* module)
{
	bool status = false;	
	BYTE temp = 0x00;
	temp = read_word(module->I2CmoduleAddress, I2CMCS);
	if(!((temp & 0x01)==0x01))
		status = true;
	return status;
}

bool I2C_checkForACK(struct I2C_module* module)
{
	bool status = false;
	BYTE temp = 0x00;
	temp = read_word(module->I2CmoduleAddress, I2CMCS);
	if(!((temp & 0x02)==0x02))
		status = true;	
	return status;
}

bool I2C_sendDataBytes(struct I2C_module* module, BYTE slaveAddress, BYTE data[], unsigned int count)
{
	bool status = false;
	int i = 0;
	I2C_setSlaveAddress(module, slaveAddress);	
	while(!I2C_checkBusBusyStatus(module)); // wait for busy signal to end
	
	for(i=0; i<count; i++)
	{
		I2C_setDataRegister(module, data[i]);
		
		if(count == 1)
		{
			write_word(module->I2CmoduleAddress, I2CMCS, 0x07); // write single byte
		}
		else
		{
			if(i == 0)
			{
				write_word(module->I2CmoduleAddress, I2CMCS, 0x03); // write start condition
			}
			else if(i == (count-1))
			{
				write_word(module->I2CmoduleAddress, I2CMCS, 0x05); // write transmit followed by a stop
			}
			else
			{
				write_word(module->I2CmoduleAddress, I2CMCS, 0x01); // write transmit condition
			}
		}
		while(!I2C_checkBusyStatus(module)); // wait for busy signal to end
		if(I2C_checkForACK(module))	// check for ack
			status = true;		
		else
		{
			write_word(module->I2CmoduleAddress, I2CMCS, 0x04); // send stop condition
			return false;	
		}			
	}
	return status;
}

bool I2C_receiveDataBytes(struct I2C_module* module, BYTE slaveAddress, BYTE data[], unsigned int count)
{
	bool status = false;
	int i = 0;
	I2C_setSlaveAddress(module, slaveAddress);	
	while(!I2C_checkBusBusyStatus(module)); // wait for busy signal to end
	
	for(i=0; i<count; i++)
	{
		if(count == 1)
		{
			write_word(module->I2CmoduleAddress, I2CMCS, 0x07); // read single byte
		}
		else
		{
			if(i == 0)
			{
				write_word(module->I2CmoduleAddress, I2CMCS, 0x0B); // read byte start conditoin
			}
			else if(i == (count - 1))
			{
				write_word(module->I2CmoduleAddress, I2CMCS, 0x05); // read byte followed by a stop condition
			}
			else
			{
				write_word(module->I2CmoduleAddress, I2CMCS, 0x09); // read byte condition
			}
		}
		while(!I2C_checkBusyStatus(module)); // wait for busy signal to end
		
		if(I2C_checkForACK(module))	// check for ack
			status = true;		
		else
		{
			write_word(module->I2CmoduleAddress, I2CMCS, 0x04); // send stop condition
			return false;
		}
		
		data[i] = I2C_getDataRegister(module); // read data
	}	
	return status;	
}
