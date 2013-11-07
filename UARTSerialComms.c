#include "UARTSerialComms.h"

static const unsigned char NUM_MODULES = 4;

static volatile BYTE* ExtendedBuffer[2][NUM_MODULES];

static BYTE_ADDRESS head[2][NUM_MODULES];
static BYTE_ADDRESS tail[2][NUM_MODULES];

static volatile struct UARTSerialComms_module* modules[NUM_MODULES]= {NULL};

static volatile Function callbackFuncs[NUM_MODULES];

//Initialization Functions
struct UARTSerialComms_module* new_UARTSCModule(struct UART_module* UARTModule, HALF_WORD bufferRx_size, HALF_WORD bufferTx_size, Function dataReceived){
	struct UARTSerialComms_module* retVal = malloc(sizeof *retVal);
	Function handles[] = {UARTSC_Rx_FIFOFULL_Handler,UARTSC_Tx_FIFOEMPTY_Handler, UARTSC_callBackWrapper, UARTSC_parityError_Handler};
	//attach modules
	retVal->p_UARTModule = UARTModule;
	callbackFuncs[UARTModule->moduleNumber] = dataReceived;
	retVal->p_NVICModule = new_UARTInterrupt(UARTModule->moduleNumber, handles,true, true, true, true, false, false, false);
	//set the bufferSize
	retVal->bufferSize[Rx] = bufferRx_size;
	retVal->bufferSize[Tx] = bufferTx_size;
	//associate UARTSerialComms Module with buffers
	modules[retVal->p_UARTModule->moduleNumber] = retVal;
	//initialize buffer
	ExtendedBuffer[Rx][UARTModule->moduleNumber] = malloc(bufferRx_size*2);
	ExtendedBuffer[Tx][UARTModule->moduleNumber] = malloc(bufferTx_size*2);
	//initialize head/tail pointers
	head[Rx][UARTModule->moduleNumber] = ExtendedBuffer[Rx][UARTModule->moduleNumber];
	tail[Rx][UARTModule->moduleNumber] = ExtendedBuffer[Rx][UARTModule->moduleNumber];
	head[Tx][UARTModule->moduleNumber] = ExtendedBuffer[Tx][UARTModule->moduleNumber];
	tail[Tx][UARTModule->moduleNumber] = ExtendedBuffer[Tx][UARTModule->moduleNumber];
	//register the interrupt handles
	NVIC_registerUARTInterrupt(retVal->p_NVICModule);
	//Unmask the Interrupt Masks
	UART_allowTxFIFOInterrupt(UARTModule, DISABLE);
	UART_allowRxFIFOInterrupt(UARTModule, ENABLE);
	UART_allowRxTimeoutInterrupt(UARTModule, ENABLE);
	UART_allowRxParityErrorInterrupt(UARTModule, ENABLE);
	
	return retVal;
}

//set functions
void UARTSC_transmit(unsigned char moduleNumber, HALF_WORD size, BYTE* data){
	int i = 0;
	for(i=0; i < size; ++i)
	{
		//make sure this doesn't write to a full buffer
		UARTSC_pushBuffer(moduleNumber, data[i], Tx);
	}
	UART_allowTxFIFOInterrupt(modules[moduleNumber]->p_UARTModule, ENABLE);
}
void UARTSC_ForceRxFIFOFlush(unsigned char moduleNumber){
	UARTSC_Rx_FIFOFULL_Handler(moduleNumber);
}

//get functions
BYTE UARTSC_getNextRx(unsigned char moduleNumber){
	if(UARTSC_bufferNotEmpty(moduleNumber, Rx)){
		return UARTSC_popBuffer(moduleNumber, Rx);
	}
	return 0;
}

void UARTSC_pushBuffer(unsigned char moduleNumber, BYTE value, RxTx RT){
	*(tail[RT][moduleNumber]) = value;
	++(tail[RT][moduleNumber]);
	if((tail[RT][moduleNumber]) > &(ExtendedBuffer[RT][moduleNumber][modules[moduleNumber]->bufferSize[RT]-1]))
		(tail[RT][moduleNumber]) = &(ExtendedBuffer[RT][moduleNumber][0]);
}
BYTE UARTSC_popBuffer(unsigned char moduleNumber, RxTx RT){
	if(UARTSC_bufferNotEmpty(moduleNumber, RT))
	{
		HALF_WORD retVal = *(head[RT][moduleNumber]);
		++(head[RT][moduleNumber]);
		if((head[RT][moduleNumber]) > &(ExtendedBuffer[RT][moduleNumber][modules[moduleNumber]->bufferSize[RT]-1]))
			(head[RT][moduleNumber]) = &(ExtendedBuffer[RT][moduleNumber][0]);
		return retVal;
	}
	return 0;
}
bool UARTSC_bufferNotEmpty(unsigned char moduleNumber, RxTx RT){
	if(head[RT][moduleNumber] == tail[RT][moduleNumber])
		return false;
	else
		return true;
}

void UARTSC_Rx_FIFOFULL_Handler(unsigned char moduleNumber){
	while(!UART_RxQueueEmpty(modules[moduleNumber]->p_UARTModule))
		UARTSC_pushBuffer(moduleNumber, UART_getNextByte(modules[moduleNumber]->p_UARTModule), Rx);
	if(!UART_isBusy(modules[moduleNumber]->p_UARTModule))
		callbackFuncs[moduleNumber](moduleNumber);
}
void UARTSC_Tx_FIFOEMPTY_Handler(unsigned char moduleNumber)
{
	while(!UART_TxQueueFull(modules[moduleNumber]->p_UARTModule) && UARTSC_bufferNotEmpty(moduleNumber, Tx))
		UART_sendNextByte(modules[moduleNumber]->p_UARTModule, UARTSC_popBuffer(moduleNumber, Tx));
	if(!UARTSC_bufferNotEmpty(moduleNumber,Tx))
		UART_allowTxFIFOInterrupt(modules[moduleNumber]->p_UARTModule, DISABLE);
}
void UARTSC_parityError_Handler(unsigned char moduleNumber){
	
}
void SSISC_callBackWrapper(unsigned char moduleNumber)
{
	UARTSC_ForceRxFIFOFlush(moduleNumber);
	callbackFuncs[moduleNumber](moduleNumber);
}
