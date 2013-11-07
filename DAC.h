#ifndef DAC_h
#define DAC_h

#include "JLib.h"
#include "SSISerialComms.h"
#include "SSI.h"

struct DAC_module{
    struct SSISerialComms_module* serialInterface;
    
};

struct DAC_module* new_DACModule();

void doNothing_handle(unsigned char);

#endif
