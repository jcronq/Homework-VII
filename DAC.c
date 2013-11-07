#include "DAC.h"

static BYTE_ADDRESS touchTimerAddr = (BYTE_ADDRESS)0x40036000;

struct DAC_module* new_DACModule(){
    struct DAC_module* retVal = malloc(sizeof *retVal);
    struct SSI_module* ssi = new_SSIMasterModule(0, FREESCALE, 100000, 16, false, true, true);
    retVal->serialInterface = new_SSISCModule(ssi, 1, 100, doNothing_handle);
    return retVal;
}

void doNothing_handle(unsigned char moduleNumber){}

