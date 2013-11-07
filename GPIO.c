/*
Author: Jason Cronquist
*/
#include "GPIO.h"

static const unsigned char NUM_PORTS = 6;

//Base Addresses
static BYTE_ADDRESS SYSTEM_CONTROL = ((BYTE_ADDRESS)0x400FE000);
static BYTE_ADDRESS GPIO_port[] = 
		{((BYTE_ADDRESS)0x40004000)
		,((BYTE_ADDRESS)0x40005000)
		,((BYTE_ADDRESS)0x40006000)
		,((BYTE_ADDRESS)0x40007000)
		,((BYTE_ADDRESS)0x40024000)
		,((BYTE_ADDRESS)0x40025000)
		};
		
//OFFSETS
	//GPIO
	static const HALF_WORD GPIOIEV   = 0x30C; // GPIO Interrupt Event
	static const HALF_WORD GPIODATA  = 0x3FC; // GPIO Data
	static const HALF_WORD GPIODIR   = 0x400; // GPIO Direction
	static const HALF_WORD GPIOIS    = 0x404; // GPIO Interrupt Sense
	static const HALF_WORD GPIOIBE   = 0x408; // GPIO Interrupt Both Edges
	static const HALF_WORD GPIOIM    = 0x410; // GPIO Interrupt Mask
	static const HALF_WORD GPIOMIS   = 0x418; // GPIO Masked Interrupt Status
	static const HALF_WORD GPIOICR   = 0x41C; // GPIO Interrupt Clear
	static const HALF_WORD GPIOAFSEL = 0x420; // GPIO Alternate Functionality Select
	static const HALF_WORD GPIOODR   = 0x50C; // GPIO Open Drain Select
	static const HALF_WORD GPIOPUR   = 0x510; // GPIO Pull-Up Select
	static const HALF_WORD GPIODEN   = 0x51C; // GPIO Digital Enable
	static const HALF_WORD GPIOLOCK  = 0x520; // GPIO Lock
	static const HALF_WORD GPIOCR    = 0x524; // GPIO Commit
	static const HALF_WORD GPIOPCTL  = 0x52C; // GPIO Port Control
	//SYSTEM_CONTROL
	static const HALF_WORD RCGCGPIO  = 0x608; // GPIO Run Mode Clock Gating Control
		
//Constants
	//GPIO
	static const WORD UNLOCK = 0x4C4F434B;


//Clock Gating Functions
void GPIO_initPortClocksWithMap(BYTE portMap, bool enable){
	readModWrite_byte(SYSTEM_CONTROL, RCGCGPIO, portMap, enable);
}
void GPIO_initPortClock(unsigned char port, bool enable){
	port = GPIO_standardizePort(port);
	setBit_word(SYSTEM_CONTROL, RCGCGPIO, port, enable);
}

//Module Initialization Functions
void GPIO_initPortPinsForIO(BYTE_ADDRESS port, BYTE pins, bool direction, bool pullUp, bool openDrain){
	GPIO_initPortClock(GPIO_getPortNumberFromAddress(port),ENABLE);
	GPIO_unlockPort(port, pins);
	GPIO_setAltFuncPinMap (port, pins, DISABLE );
	GPIO_setDirectionPinMap (port, pins, direction);
	GPIO_setPullupPinMap (port, pins, pullUp);
	GPIO_setOpenDrainPinMap (port, pins, openDrain);
	GPIO_setDigEnPinMap (port, pins, ENABLE);
}
void GPIO_initPortPinsForSSI(BYTE_ADDRESS port, BYTE pinMap){
	GPIO_initPortClock(GPIO_getPortNumberFromAddress(port),ENABLE);
	GPIO_unlockPort(port, (0x01<<pinMap));
	GPIO_setAltFuncPinMap (port, pinMap, ENABLE);
	GPIO_setAltFuncPortControl (port, pinMap, SSI);
	GPIO_setDigEnPinMap(port, pinMap, ENABLE);
}
void GPIO_initPortPinsForUART(BYTE_ADDRESS port, BYTE pins){
	GPIO_initPortClock(GPIO_getPortNumberFromAddress(port),ENABLE);
	GPIO_unlockPort(port, pins);
	GPIO_setAltFuncPinMap(port, pins, ENABLE); 
	GPIO_setAltFuncPortControl(port, pins, UART);
	GPIO_setDigEnPinMap(port, pins, ENABLE);
}
void GPIO_initPortPinForInterrupt(struct NVIC_interruptModule_GPIO* module){
	GPIO_initPortClock(module->encodedPort,ENABLE);
	GPIO_unlockPort(GPIO_port[module->encodedPort], (0x01<<module->base.bit));
	GPIO_setAltFuncPinMap(GPIO_port[module->encodedPort], (0x01<<module->base.bit), DISABLE);
	GPIO_setDirectionPinMap(GPIO_port[module->encodedPort], (0x01<<module->base.bit), IO_IN);
	GPIO_setDigEnPinMap(GPIO_port[module->encodedPort], (0x01<<module->base.bit), ENABLE);
	GPIO_setInterruptSense(GPIO_port[module->encodedPort], module->base.bit, module->edgeOrLevel);
	GPIO_setBothEdges(GPIO_port[module->encodedPort], module->base.bit, module->orSingle_edge);
	GPIO_setEdgeLevel(GPIO_port[module->encodedPort], module->base.bit, module->highOrLow);
	GPIO_setInterruptMask(GPIO_port[module->encodedPort],module->base.bit,ENABLE);
}
void GPIO_initPortPinsForI2C(BYTE_ADDRESS port, BYTE pin)
{
	BYTE SDA;
	GPIO_setAltFuncPinMap (port, pin, ENABLE);
	GPIO_setAltFuncPortControl (port, pin, I2C);
	
	// modify pinMap to mask off lower bit for open drain
	// I2C SDA should be open drain
	// I2C SDA is always upper bit
	if(pin == 0x03)
		SDA = 0x02;
	else if(pin == 0x0C)
		SDA = 0x08;
	else if(pin ==  0x30)
		SDA = 0x20;
	else if(pin == 0xC0)
		SDA = 0x80;	
	GPIO_setOpenDrainPinMap(port, SDA, ENABLE);
	
	GPIO_setDigEnPinMap(port, pin, ENABLE);
}

//INITIALIZATION FUNCTIONS
	//General
	void GPIO_setAltFuncPinMap(BYTE_ADDRESS port, BYTE pins, bool enable){
		readModWrite_byte(port, GPIOAFSEL, pins, enable);
	}
	void GPIO_setAltFuncPortControl(BYTE_ADDRESS port, BYTE pins, GPIOPCTL_OPTIONS option){
		int i = 0;
		WORD clearMap = 0;
		WORD map_32bit = 0;
		
		if (option == JTAG_SWD)
			option = UART;
		
		for(i = 0; i < sizeof(pins)*8; ++i){
			if((pins&(0x01<<i)) != 0){
				clearMap |= ((WORD)(0xF)<<(i*4));
				map_32bit |= ((unsigned char)option<<(i*4));
			}
		}
		selectiveClearAndWrite_word(port, GPIOPCTL, clearMap, map_32bit);
	}
	void GPIO_setDirectionPinMap(BYTE_ADDRESS port, BYTE pins, bool enable){
		readModWrite_byte(port, GPIODIR, pins, enable);
	}
	void GPIO_setOpenDrainPinMap(BYTE_ADDRESS port, BYTE pins, bool enable){
		readModWrite_byte(port, GPIOODR, pins, enable);
	}
	void GPIO_setDigEnPinMap(BYTE_ADDRESS port, BYTE pins, bool enable){
		readModWrite_byte(port, GPIODEN, pins, enable);
	}
	void GPIO_setPullupPinMap(BYTE_ADDRESS port, BYTE pins, bool enable){
		readModWrite_byte(port, GPIOPUR, pins, enable);
	}
	void GPIO_unlockPort(BYTE_ADDRESS port, BYTE pins){
		if(read_word(port, GPIOLOCK)){
			write_word(port, GPIOLOCK, UNLOCK);
			write_byte(port, GPIOCR, 0xFF);
		}
	}
	//interrupts
	void GPIO_setInterruptSense(BYTE_ADDRESS port, unsigned char pin, InterruptSense edgeOrLevel){
		setBit_word(port, GPIOIS, pin, (bool)edgeOrLevel);
	}
	void GPIO_setBothEdges(BYTE_ADDRESS port, unsigned char pin, BothEdges orSingle_edge){
		setBit_word(port, GPIOIBE, pin, (bool)orSingle_edge);
	}
	void GPIO_setEdgeLevel(BYTE_ADDRESS port, unsigned char pin, EdgeLevel lowOrHigh){
		setBit_word(port, GPIOIEV, pin, (bool)lowOrHigh);
	}
	void GPIO_setInterruptMask(BYTE_ADDRESS port, unsigned char pin, bool enable){
		setBit_word(port, GPIOIM, pin, enable);
	}
	
//Get Functions
BYTE readFromPort_GPIO(BYTE_ADDRESS port){
	return read_byte(port, GPIODATA);
}
bool readFromPin_GPIO(BYTE_ADDRESS port, unsigned char pinNumber){
	BYTE pins = (0x01 << pinNumber);
	return ((read_byte(port,GPIODATA) & pins) != 0)? true : false;
}
BYTE_ADDRESS GPIO_getPortAddress(unsigned char port){
	port = GPIO_standardizePort(port);
	return GPIO_port[port];
}
unsigned char GPIO_getPortNumberFromAddress(BYTE_ADDRESS addr){
	unsigned char i = 0;
	for(i = 0; i < NUM_PORTS; ++i){
		if(addr == GPIO_port[i])
			return i;
	}
	return NUM_PORTS;
}

//WRITE DATA
void writeToPort_GPIO(BYTE_ADDRESS port, BYTE toWrite){
	write_byte(port,GPIODATA, toWrite);
}
void writeToPin_GPIO(BYTE_ADDRESS port, unsigned char pinNumber, bool value){
	BYTE pins = (0x01 << pinNumber);
	readModWrite_byte (port ,GPIODATA ,pins ,value);
}
void readModifyWriteByte_GPIO(BYTE_ADDRESS port, BYTE pinMap, bool enable){
	readModWrite_byte(port, GPIODATA, pinMap, enable);
}

//Interrupt Status Functions
unsigned char GPIO_getInterruptBitAndClear(unsigned char port){
	unsigned char i;
	unsigned char pin = 8;
	BYTE mis = read_byte(GPIO_getPortAddress(port), GPIOMIS);
	for(i = 0; i < 8; ++i){
		if((mis&0x01) == 0x01)
			pin = i;
		mis = mis>>1;
	}
	if(pin < 8)
		setBit_word(GPIO_getPortAddress(port), GPIOICR, pin, ENABLE);
	return pin;
}

//Useful Functions
unsigned char GPIO_standardizePort(unsigned char port){
	if(port < NUM_PORTS)
		return port;
	else if((port - 'a') < NUM_PORTS)
		return (port - 'a');
	else if((port - 'A') < NUM_PORTS)
		return (port - 'A');
	else
		return 0;
}

