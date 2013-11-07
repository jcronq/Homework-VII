/*
Author: Brian Carrick
*/
#ifndef COMPASS_h
# define COMPASS_h
//include
#	ifndef JLib_h
#		include "JLib.h"
#	endif
#	ifndef GPIO_h
#		include "GPIO.h"
#	endif
#	ifndef I2C_h
#		include "I2C.h"
#	endif

typedef struct{
	BYTE upper;
	BYTE lower;
	WORD fullHeading;
	WORD degHeading;
	BYTE tenthDegHeading;
} Heading;
struct I2C_module* configCompass();

void writeEeprom(struct I2C_module* Compass, BYTE command, BYTE data);
WORD readEeprom(struct I2C_module* Compass, BYTE command);

void writeRam(struct I2C_module* Compass, BYTE command, BYTE data);
WORD readRam(struct I2C_module* Compass, BYTE command);

Heading readDataContinuous(struct I2C_module* Compass);

#endif

