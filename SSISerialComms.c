#include "SSISerialComms.h"

static const unsigned char NUM_MODULES = 4;

static volatile HALF_WORD* ExtendedBuffer[2][NUM_MODULES];

static HALF_WORD_ADDRESS head[2][NUM_MODULES];
static HALF_WORD_ADDRESS tail[2][NUM_MODULES];

static volatile struct SSISerialComms_module* modules[NUM_MODULES]= {NULL};

static volatile Function callbackFuncs[NUM_MODULES];

//Initialization Functions
struct SSISerialComms_module* new_SSISCModule(struct SSI_module* SSIModule, HALF_WORD bufferRx_size, HALF_WORD bufferTx_size, Function dataReceived){
	struct SSISerialComms_module* retVal = malloc(sizeof *retVal);
	Function handles[] = {SSISC_Rx_FIFOFULL_Handler,SSISC_Tx_FIFOEMPTY_Handler, SSISC_callBackWrapper};
	//attach modules
	retVal->p_SSIModule = SSIModule;
	callbackFuncs[SSIModule->moduleNumber] = dataReceived;
	retVal->p_NVICModule = new_SSIInterrupt(SSIModule->moduleNumber, handles, true, true, true, false);
	//set the bufferSize
	retVal->bufferSize[Rx] = bufferRx_size;
	retVal->bufferSize[Tx] = bufferTx_size;
	//associate SSISerialComms Module with buffers
	modules[retVal->p_SSIModule->moduleNumber] = retVal;
	//initialize buffer
	ExtendedBuffer[Rx][SSIModule->moduleNumber] = malloc(bufferRx_size*2);
	ExtendedBuffer[Tx][SSIModule->moduleNumber] = malloc(bufferTx_size*2);
	//initialize head/tail pointers
	head[Rx][SSIModule->moduleNumber] = ExtendedBuffer[Rx][SSIModule->moduleNumber];
	tail[Rx][SSIModule->moduleNumber] = ExtendedBuffer[Rx][SSIModule->moduleNumber];
	head[Tx][SSIModule->moduleNumber] = ExtendedBuffer[Tx][SSIModule->moduleNumber];
	tail[Tx][SSIModule->moduleNumber] = ExtendedBuffer[Tx][SSIModule->moduleNumber];
	//register the interrupt handles
	NVIC_registerSSIInterrupt(retVal->p_NVICModule);
	//Unmask the Interrupt Masks
	SSI_allowTxFIFOInterrupt(SSIModule, DISABLE);
	SSI_allowRxFIFOInterrupt(SSIModule, ENABLE);
	SSI_allowRxTimeoutInterrupt(SSIModule, ENABLE);
	
	//setup the Interrupt
	SSI_setTxInterruptAtHalfEmpty(SSIModule, ENABLE);
	return retVal;
}

//set functions
void SSISC_transmit(unsigned char moduleNumber, HALF_WORD size, HALF_WORD* data){
	int i = 0;
	for(i=0; i < size; ++i)
	{
		//make sure this doesn't write to a full buffer
		SSISC_pushBuffer(moduleNumber, data[i], Tx);
	}
	SSI_allowTxFIFOInterrupt(modules[moduleNumber]->p_SSIModule, ENABLE);
}
void SSISC_ForceRxFIFOFlush(unsigned char moduleNumber){
	SSISC_Rx_FIFOFULL_Handler(moduleNumber);
}

//get functions
HALF_WORD SSISC_getNextRx(unsigned char moduleNumber){
	if(SSISC_bufferNotEmpty(moduleNumber, Rx)){
		return SSISC_popBuffer(moduleNumber, Rx);
	}
	return 0;
}

void SSISC_pushBuffer(unsigned char moduleNumber, HALF_WORD value, RxTx RT){
	*(tail[RT][moduleNumber]) = value;
	++(tail[RT][moduleNumber]);
	if((tail[RT][moduleNumber]) > &(ExtendedBuffer[RT][moduleNumber][modules[moduleNumber]->bufferSize[RT]-1]))
		(tail[RT][moduleNumber]) = &(ExtendedBuffer[RT][moduleNumber][0]);
}
HALF_WORD SSISC_popBuffer(unsigned char moduleNumber, RxTx RT){
	if(SSISC_bufferNotEmpty(moduleNumber, RT))
	{
		HALF_WORD retVal = *(head[RT][moduleNumber]);
		++(head[RT][moduleNumber]);
		if((head[RT][moduleNumber]) > &(ExtendedBuffer[RT][moduleNumber][modules[moduleNumber]->bufferSize[RT]-1]))
			(head[RT][moduleNumber]) = &(ExtendedBuffer[RT][moduleNumber][0]);
		return retVal;
	}
	return 0;
}
bool SSISC_bufferNotEmpty(unsigned char moduleNumber, RxTx RT){
	if(head[RT][moduleNumber] == tail[RT][moduleNumber])
		return false;
	else
		return true;
}

void SSISC_Rx_FIFOFULL_Handler(unsigned char moduleNumber){
	while(SSI_ReceiveNotEmpty(moduleNumber))
		SSISC_pushBuffer(moduleNumber, SSI_getNextValueFromRXQueue(moduleNumber), Rx);
	if(!SSI_isBusy(moduleNumber))
		callbackFuncs[moduleNumber](moduleNumber);
}
void SSISC_Tx_FIFOEMPTY_Handler(unsigned char moduleNumber)
{
	while(SSI_TransmitNotFull(moduleNumber) && SSISC_bufferNotEmpty(moduleNumber, Tx))
		SSI_addDataToTXFIFO(modules[moduleNumber]->p_SSIModule, SSISC_popBuffer(moduleNumber, Tx));
	if(!SSISC_bufferNotEmpty(moduleNumber,Tx))
		SSI_allowTxFIFOInterrupt(modules[moduleNumber]->p_SSIModule, DISABLE);
}
void SSISC_callBackWrapper(unsigned char moduleNumber)
{
	SSISC_ForceRxFIFOFlush(moduleNumber);
	callbackFuncs[moduleNumber](moduleNumber);
}
