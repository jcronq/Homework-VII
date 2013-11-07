#include "NVIC.h"

static BYTE_ADDRESS CORE_PERIPHERALS = ((BYTE_ADDRESS)0xE000E000);

static const HALF_WORD EN     = 0x100;
static const HALF_WORD DIS    = 0x180;
static const HALF_WORD PEND   = 0x200;
static const HALF_WORD UNPEND = 0x280;
static const HALF_WORD ACTIVE = 0x300;
static const HALF_WORD PRI    = 0x400;

static const HALF_WORD regWidth = 0x004;
static const HALF_WORD wordSize = 32;
static const HALF_WORD prioritiesPerReg = 4;

static const BYTE interruptPriority_bit = 5;
static const BYTE interruptPriority_clear = 0xE0;

volatile static struct NVIC_GPIOInterruptDirectory GPIO_portInterrupts[6];
volatile static struct NVIC_SSIInterruptDirectory SSI_moduleInterrupts[4];
volatile static struct NVIC_UARTInterruptDirectory UART_moduleInterrupts[8];
volatile static struct NVIC_GPTMInterruptDirectory GPTM_moduleInterrupts[12];
volatile static struct NVIC_ADCInterruptDirectory ADC_moduleInterrupts[2][4];

//InterruptNumber (bit in interrupt registers) : values
static const HALF_WORD GPIO_InterruptNumber[] = {0, 1, 2, 3, 4, 30};
static const HALF_WORD SSI_InterruptNumber[]  = {7, 34, 57, 58};
static const HALF_WORD UART_InterruptNumber[] = {5, 6, 33, 59, 60, 61, 62, 63};
static const HALF_WORD GPTM_InterruptNumber[] = {19, 21, 23, 35, 70, 92, 94, 96, 98, 100, 102, 104};
static const HALF_WORD ADC_InterruptNumber[] = {14, 15, 16, 17, 48, 49, 50, 51};
	
static BYTE_ADDRESS GPIO_address[] = 
		{((BYTE_ADDRESS)0x40004000)
		,((BYTE_ADDRESS)0x40005000)
		,((BYTE_ADDRESS)0x40006000)
		,((BYTE_ADDRESS)0x40007000)
		,((BYTE_ADDRESS)0x40024000)
		,((BYTE_ADDRESS)0x40025000)
		};

static BYTE_ADDRESS SSI_address[] =
		{((BYTE_ADDRESS)0x40008000)
		,((BYTE_ADDRESS)0x40009000)
		,((BYTE_ADDRESS)0x4000A000)
		,((BYTE_ADDRESS)0x4000B000)
		};
static BYTE_ADDRESS UART_address[] =
		{((BYTE_ADDRESS)0x4000C000)
		,((BYTE_ADDRESS)0x4000D000)
		,((BYTE_ADDRESS)0x4000E000)
		,((BYTE_ADDRESS)0x4000F000)
		,((BYTE_ADDRESS)0x40010000)
		,((BYTE_ADDRESS)0x40011000)
		,((BYTE_ADDRESS)0x40012000)
		,((BYTE_ADDRESS)0x40013000)
		};
static BYTE_ADDRESS GPTM_address[] = 
		{(BYTE_ADDRESS)0x40030000
		,(BYTE_ADDRESS)0x40031000
		,(BYTE_ADDRESS)0x40032000
		,(BYTE_ADDRESS)0x40033000
		,(BYTE_ADDRESS)0x40034000
		,(BYTE_ADDRESS)0x40035000
		,(BYTE_ADDRESS)0x40036000
		,(BYTE_ADDRESS)0x40037000
		,(BYTE_ADDRESS)0x4004C000
		,(BYTE_ADDRESS)0x4004D000
		,(BYTE_ADDRESS)0x4004E000
		,(BYTE_ADDRESS)0x4004F000
		};	
static BYTE_ADDRESS ADC_address[] = 
        {(BYTE_ADDRESS)0x40038000
        ,(BYTE_ADDRESS)0x40039000
        };   
         
		
static const HALF_WORD GPIOICR = 0x41C;
static const HALF_WORD GPIOMIS = 0x418;
static const HALF_WORD SSIICR  = 0x020;
static const HALF_WORD SSIMIS  = 0x01C;
static const HALF_WORD UARTICR = 0x044;
static const HALF_WORD UARTMIS = 0x040;
static const HALF_WORD GPTMICR = 0x024;
static const HALF_WORD GPTMMIS = 0x020;
static const HALF_WORD ADCISC  = 0x00C;
		
struct NVIC_interruptModule_GPIO* new_GPIOInterrupt(unsigned char port, unsigned char pin, Function f, InterruptSense edgeOrLevel, BothEdges orSingle_edge, EdgeLevel highOrLow){
	struct NVIC_interruptModule_GPIO* retVal = malloc(sizeof *retVal);
	
	retVal->port = port;
	retVal->base.bit = pin;
	retVal->base.f = f;
	
	retVal->edgeOrLevel = edgeOrLevel;
	retVal->orSingle_edge = orSingle_edge;
	retVal->highOrLow = highOrLow;
	
	if(port-'a'<6)
		retVal->encodedPort = port-'a';
	else if(port-'A'<6)
		retVal->encodedPort = port-'A';
	else if(port < 6)
		retVal->encodedPort = port;
	else
	{
		free(retVal);
		return NULL;
	}
	return retVal;
}
//Function * must be ordered RxFifoFull, TxFifoempty, RxTimeout, RxOverrun
//if one of the above functions is missing, just leave it blank.
//consider improving...
struct NVIC_interruptModule_SSI* new_SSIInterrupt(BYTE moduleNumber, Function* f, bool RxFIFOHalfFull, bool TxFIFOHalfEmpty, bool RxTimeOut, bool RxOverrun){
	int count = 0, i = 0;
	struct NVIC_interruptModule_SSI* retVal = malloc(sizeof *retVal);
	retVal->moduleNumber = moduleNumber;
	retVal->moduleAddress = SSI_address[moduleNumber];
	retVal->RxFIFOHalfFull = RxFIFOHalfFull;
	retVal->TxFIFOHalfEmpty = TxFIFOHalfEmpty;
	retVal->RxTimeOut = RxTimeOut;
	retVal->RxOverrun = RxOverrun;
	if(RxFIFOHalfFull)
		count++;
	if(TxFIFOHalfEmpty)
		count++;
	if(RxTimeOut)
		count++;
	if(RxOverrun)
		count++;
	retVal->numInterrupts = count;
	retVal->base = malloc((sizeof *retVal->base)*count);		
	if(RxFIFOHalfFull){
		retVal->base[i].bit = 2;
		retVal->base[i].f = f[i];
		i++;
	}
	if(TxFIFOHalfEmpty){
		retVal->base[i].bit = 3;
		retVal->base[i].f = f[i];
		i++;
	}
	if(RxTimeOut){
		retVal->base[i].bit = 1;
		retVal->base[i].f = f[i];
		i++;
	}
	if(RxOverrun){
		retVal->base[i].bit = 0;
		retVal->base[i].f = f[i];
		i++;
	}
	return retVal;
}
//Function * must be ordered RxFifoFull, TxFifoempty, RxTimeout, RxOverrun
//if one of the above functions is missing, just leave it blank.
//consider improving...
struct NVIC_interruptModule_UART* new_UARTInterrupt(BYTE moduleNumber, Function* f, bool RxFIFOFull, bool TxFIFOEmpty, bool RxTimeOut, bool ParityError, bool breakError, bool _9BitMode, bool RxOverrun){
	int count = 0, i = 0;
	struct NVIC_interruptModule_UART* retVal = malloc(sizeof *retVal);
	retVal->moduleNumber = moduleNumber;
	retVal->moduleAddress = UART_address[moduleNumber];
	retVal->RxFIFOHalfFull = RxFIFOFull;
	retVal->TxFIFOHalfEmpty = TxFIFOEmpty;
	retVal->RxTimeOut = RxTimeOut;
	retVal->RxOverrun = RxOverrun;
	if(RxFIFOFull)
		count++;
	if(TxFIFOEmpty)
		count++;
	if(RxTimeOut)
		count++;
	if(RxOverrun)
		count++;
	if(ParityError)
		count++;
	if(breakError)
		count++;
	if(_9BitMode)
		count++;
	retVal->numInterrupts = count;
	retVal->base = malloc((sizeof *retVal->base)*count);		
	if(RxFIFOFull){
		retVal->base[i].bit = 4;
		retVal->base[i].f = f[i];
		i++;
	}
	if(TxFIFOEmpty){
		retVal->base[i].bit = 5;
		retVal->base[i].f = f[i];
		i++;
	}
	if(RxTimeOut){
		retVal->base[i].bit = 6;
		retVal->base[i].f = f[i];
		i++;
	}
	if(ParityError){
		retVal->base[i].bit = 8;
		retVal->base[i].f = f[i];
		i++;
	}
	if(breakError){
		retVal->base[i].bit = 9;
		retVal->base[i].f = f[i];
		i++;
	}
	if(_9BitMode){
		retVal->base[i].bit = 12;
		retVal->base[i].f = f[i];
		i++;
	}
	if(RxOverrun){
		retVal->base[i].bit = 10;
		retVal->base[i].f = f[i];
		i++;
	}
	return retVal;
}
struct NVIC_interruptModule_GPTM* new_GPTMInterrupt(BYTE timerNumber, BYTE aOrb, Function* f, bool timeOut, bool captureMode, bool captureModeEdge, bool timerMatch, bool RTC){
		int count = 0, i = 0;
	struct NVIC_interruptModule_GPTM* retVal = malloc(sizeof *retVal);
	retVal->aOrb = aOrb;
	retVal->timerNumber = timerNumber;
	retVal->moduleAddress = GPTM_address[timerNumber];
	retVal->timeOut = timeOut;
	retVal->captureMode = captureMode;
	retVal->captureModeEdge = captureModeEdge;
	retVal->timerMatch = timerMatch;
	retVal->RTC = RTC;
	if(timeOut)
		count++;
	if(captureMode)
		count++;
	if(captureModeEdge)
		count++;
	if(timerMatch)
		count++;
	if(RTC)
		count++;
	retVal->numInterrupts = count;
	retVal->base = malloc((sizeof *retVal->base)*count);		
	if(timeOut){
		retVal->base[i].bit = (aOrb)?8:0;
		retVal->base[i].f = f[i];
		i++;
	}
	if(captureMode){
		retVal->base[i].bit = (aOrb)?9:1;
		retVal->base[i].f = f[i];
		i++;
	}
	if(captureModeEdge){
		retVal->base[i].bit = (aOrb)?10:2;
		retVal->base[i].f = f[i];
		i++;
	}
	if(timerMatch){
		retVal->base[i].bit = (aOrb)?11:4;
		retVal->base[i].f = f[i];
		i++;
	}
	if(RTC){
		retVal->base[i].bit = 3;
		retVal->base[i].f = f[i];
		i++;
	}
	return retVal;
}
bool NVIC_isEnabled(HALF_WORD vectorNumber){
	return (bool)getBit(CORE_PERIPHERALS, EN+((vectorNumber/wordSize)*regWidth),vectorNumber%wordSize);
}
void NVIC_setEnable(HALF_WORD vectorNumber, bool enable){
	if(enable)
		setBit_word(CORE_PERIPHERALS, EN+((vectorNumber/wordSize)*regWidth), vectorNumber%wordSize, ENABLE);
	else
		setBit_word(CORE_PERIPHERALS, DIS+((vectorNumber/wordSize)*regWidth), vectorNumber%wordSize, ENABLE);
}
bool NVIC_isPending(HALF_WORD vectorNumber){
	return (bool)getBit(CORE_PERIPHERALS, PEND+((vectorNumber/wordSize)*regWidth), vectorNumber%wordSize);
}
void NVIC_setPending(HALF_WORD vectorNumber, bool enable){
	if(enable)
		setBit_word(CORE_PERIPHERALS, PEND+((vectorNumber/wordSize)*regWidth), vectorNumber%wordSize, ENABLE);
	else
		setBit_word(CORE_PERIPHERALS, UNPEND+((vectorNumber/wordSize)*regWidth), vectorNumber%wordSize, ENABLE);
}
void NVIC_setPriority(HALF_WORD vectorNumber, BYTE priority){
	selectiveClearAndWrite_word
		(CORE_PERIPHERALS
		,PRI+((vectorNumber/prioritiesPerReg)*regWidth)
		,((WORD)interruptPriority_clear)<<(vectorNumber%prioritiesPerReg)
		,(((WORD)priority<<interruptPriority_bit))<<(vectorNumber%prioritiesPerReg)
		);
}
bool NVIC_isActive(HALF_WORD vectorNumber){
	return (bool)getBit(CORE_PERIPHERALS, ACTIVE+((vectorNumber/32)*regWidth), vectorNumber%32);
}
HALF_WORD NVIC_getInterruptNumber_SSI(unsigned char moduleNumber){
	return SSI_InterruptNumber[moduleNumber];
}
HALF_WORD NVIC_getInterruptNumber_GPIO(unsigned char portNumber){
	return GPIO_InterruptNumber[portNumber];
}
HALF_WORD NVIC_getInterruptNumber_GPTM(unsigned char timerNumber){
	return GPTM_InterruptNumber[timerNumber];
}
void NVIC_registerGPIOInterrupt(struct NVIC_interruptModule_GPIO* module){
	NVIC_setEnable(GPIO_InterruptNumber[module->encodedPort],ENABLE);
	GPIO_portInterrupts[module->encodedPort].pinInterrupts[module->base.bit] = module->base.f;
}
void NVIC_registerSSIInterrupt(struct NVIC_interruptModule_SSI* module){
	int i = 0;
	NVIC_setEnable(SSI_InterruptNumber[module->moduleNumber],ENABLE);
	for(i=0; i<module->numInterrupts;++i)
		SSI_moduleInterrupts[module->moduleNumber].ssiInterrupts[module->base[i].bit] = module->base[i].f;
}
void NVIC_registerUARTInterrupt(struct NVIC_interruptModule_UART* module){
	int i = 0;
	unsigned char hashBit = 0;
	NVIC_setEnable(UART_InterruptNumber[module->moduleNumber],ENABLE);
	for(i=0; i<module->numInterrupts;++i){
		hashBit = (module->base[i].bit == 1)? 0:((module->base[i].bit <= 10)?(module->base[i].bit-3):8);
		UART_moduleInterrupts[module->moduleNumber].uartInterrupts[hashBit] = module->base[i].f;
	}
}
void NVIC_registerGPTMInterrupt(struct NVIC_interruptModule_GPTM* module){
	int i = 0;
	NVIC_setEnable(GPTM_InterruptNumber[module->timerNumber]+((module->aOrb)?1:0),ENABLE);
	for(i =0; i < module->numInterrupts;++i){
		if(module->base[i].bit == 3)
			GPTM_moduleInterrupts[module->timerNumber].RTCMIS = module->base[i].f;
		else if(module->aOrb)
			GPTM_moduleInterrupts[module->timerNumber].gptmInterrupts[module->base[i].bit-8].b = module->base[i].f;
		else
			GPTM_moduleInterrupts[module->timerNumber].gptmInterrupts[((module->base[i].bit == 4)?3:module->base[i].bit)].a = module->base[i].f;
	}
}
void NVIC_registerADCInterrupt(struct NVIC_interruptModule_ADC* module){
    
}
void NVIC_callAppropriateGPIOHandle(unsigned char port){
	unsigned char i;
	unsigned char pin = 8;
	BYTE mis = read_byte(GPIO_address[port], GPIOMIS);
	for(i = 0; i < 8; ++i)
	{
		if((mis&0x01) == 0x01)
			pin = i;
		mis = mis>>1;
	}
	if(pin <8){
		setBit_word(GPIO_address[port], GPIOICR, pin, ENABLE);
		GPIO_portInterrupts[port].pinInterrupts[pin](port);
	}
	NVIC_setPending(GPIO_InterruptNumber[port], DISABLE);
}
void NVIC_callAppropriateSSIHandle(unsigned char moduleNumber)
{
	unsigned char i;
	unsigned char bit = 4;
	BYTE mis = read_byte(SSI_address[moduleNumber], SSIMIS);
	for(i = 0; i < 4; ++i)
	{
		if((mis&0x01) == 0x01)
			bit = i;
		mis = mis>>1;
	}
	if(bit < 4)
	{
		if(bit == 2)
			mis++;
		if(bit < 2)
			setBit_word(SSI_address[moduleNumber], SSIICR, bit, ENABLE);
		SSI_moduleInterrupts[moduleNumber].ssiInterrupts[bit](moduleNumber);
	}
	if(!read_byte(SSI_address[moduleNumber], SSIMIS))
		NVIC_setPending(SSI_InterruptNumber[moduleNumber], DISABLE);
}
void NVIC_callAppropriateUARTHandle(unsigned char moduleNumber)
{
	unsigned char i;
	unsigned char bit = 13;
	unsigned char hashBit = 0;
	HALF_WORD mis = read_halfWord(UART_address[moduleNumber], UARTMIS);
	for(i = 0; i < 13; ++i)
	{
		if((mis&0x01) == 0x01)
			bit = i;
		mis = mis>>1;
	}
	if(bit < 13)
	{
		hashBit = (bit == 1)? 0:((bit <= 10)?(bit-3):8);
		setBit_word(UART_address[moduleNumber], UARTICR, bit, ENABLE);
		UART_moduleInterrupts[moduleNumber].uartInterrupts[hashBit](moduleNumber);
	}
	if(!read_byte(UART_address[moduleNumber], UARTMIS))
		NVIC_setPending(UART_InterruptNumber[moduleNumber], DISABLE);
}
void NVIC_callAppropriateGPTMHandle(unsigned char timerNumber, unsigned char aOrb){
	unsigned char i;
	unsigned char bit = 12;
	WORD mis = read_word(GPTM_address[timerNumber], GPTMMIS);
	for(i = ((aOrb)?8:0); i < ((aOrb)?11:4); ++i)
	{
		if((mis&(0x01<<i)) == (0x01<<i))
			bit = i;
	}
	if((bit <= ((aOrb)?11:4)) && (bit >=((aOrb)?8:0)))
	{
		setBit_word(GPTM_address[timerNumber], GPTMICR, bit, ENABLE);
		if(bit == 3)
			GPTM_moduleInterrupts[timerNumber].RTCMIS(timerNumber);
		else if(aOrb)//b
			GPTM_moduleInterrupts[timerNumber].gptmInterrupts[bit-8].b(timerNumber);
		else 
			GPTM_moduleInterrupts[timerNumber].gptmInterrupts[(bit==4)?3:bit].a(timerNumber);
	}
	NVIC_setPending(GPTM_InterruptNumber[timerNumber]+((aOrb)?1:0), DISABLE);
}
//Untested, incomplete
void ADC_callAppropriateADCHandle(unsigned char moduleNumber, unsigned char sequenceNumber){
  unsigned char i;
	unsigned char hashBit;
	unsigned char bit = 4;
	WORD mis = read_word(ADC_address[moduleNumber], ADCISC);
	for(i = 0; i < 17; ++i)
	{
		if((mis&0x01) == 0x01)
			bit = i;
		mis = mis>>1;
	}
	hashBit = (bit == 16)?4:bit;
	if(hashBit < 5)
	{
		setBit_word(ADC_address[moduleNumber], ADCISC, bit, ENABLE);
		ADC_moduleInterrupts[moduleNumber][sequenceNumber].adcInterrupts[bit]((moduleNumber*4)+sequenceNumber);
	}
	if(!read_byte(ADC_address[moduleNumber], ADCISC))
		NVIC_setPending(ADC_InterruptNumber[moduleNumber], DISABLE);
}
//GPIO
void GPIOPortA_Handler(void){ NVIC_callAppropriateGPIOHandle(0);}
void GPIOPortB_Handler(void){ NVIC_callAppropriateGPIOHandle(1);}
void GPIOPortC_Handler(void){	NVIC_callAppropriateGPIOHandle(2);}
void GPIOPortD_Handler(void){	NVIC_callAppropriateGPIOHandle(3);}
void GPIOPortE_Handler(void){	NVIC_callAppropriateGPIOHandle(4);}
void GPIOPortF_Handler(void){	NVIC_callAppropriateGPIOHandle(5);}
//SSI
void SSI0_Handler(void){ NVIC_callAppropriateSSIHandle(0);}
void SSI1_Handler(void){ NVIC_callAppropriateSSIHandle(1);}
void SSI2_Handler(void){ NVIC_callAppropriateSSIHandle(2);}
void SSI3_Handler(void){ NVIC_callAppropriateSSIHandle(3);}
//UART
void UART0_Handler(void) { NVIC_callAppropriateUARTHandle(0);}
void UART1_Handler(void) { NVIC_callAppropriateUARTHandle(1);}
void UART2_Handler(void) { NVIC_callAppropriateUARTHandle(2);}
void UART3_Handler(void) { NVIC_callAppropriateUARTHandle(3);}
void UART4_Handler(void) { NVIC_callAppropriateUARTHandle(4);}
void UART5_Handler(void) { NVIC_callAppropriateUARTHandle(5);}
void UART6_Handler(void) { NVIC_callAppropriateUARTHandle(6);}
void UART7_Handler(void) { NVIC_callAppropriateUARTHandle(7);}
//GPTM
void Timer0A_Handler(void){ NVIC_callAppropriateGPTMHandle(0, 0);}
void Timer0B_Handler(void){ NVIC_callAppropriateGPTMHandle(0, 1);}
//void Timer1A_Handler(void){ NVIC_callAppropriateGPTMHandle(1, 0);}
void Timer1B_Handler(void){ NVIC_callAppropriateGPTMHandle(1, 1);}
void Timer2A_Handler(void){ NVIC_callAppropriateGPTMHandle(2, 0);}
void Timer2B_Handler(void){ NVIC_callAppropriateGPTMHandle(2, 1);}
void Timer3A_Handler(void){ NVIC_callAppropriateGPTMHandle(3, 0);}
void Timer3B_Handler(void){ NVIC_callAppropriateGPTMHandle(3, 1);}
void Timer4A_Handler(void){ NVIC_callAppropriateGPTMHandle(4, 0);}
void Timer4B_Handler(void){ NVIC_callAppropriateGPTMHandle(4, 1);}
void Timer5A_Handler(void){ NVIC_callAppropriateGPTMHandle(5, 0);}
void Timer5B_Handler(void){ NVIC_callAppropriateGPTMHandle(5, 1);}
void WideTimer0A_Handler(void){ NVIC_callAppropriateGPTMHandle(6, 0);}
void WideTimer0B_Handler(void){ NVIC_callAppropriateGPTMHandle(6, 1);}
void WideTimer1A_Handler(void){ NVIC_callAppropriateGPTMHandle(7, 0);}
void WideTimer1B_Handler(void){ NVIC_callAppropriateGPTMHandle(7, 1);}
void WideTimer2A_Handler(void){ NVIC_callAppropriateGPTMHandle(8, 0);}
void WideTimer2B_Handler(void){ NVIC_callAppropriateGPTMHandle(8, 1);}
void WideTimer3A_Handler(void){ NVIC_callAppropriateGPTMHandle(9, 0);}
void WideTimer3B_Handler(void){ NVIC_callAppropriateGPTMHandle(9, 1);}
void WideTimer4A_Handler(void){ NVIC_callAppropriateGPTMHandle(10, 0);}
void WideTimer4B_Handler(void){ NVIC_callAppropriateGPTMHandle(10, 1);}
void WideTimer5A_Handler(void){ NVIC_callAppropriateGPTMHandle(11, 0);}
void WideTimer5B_Handler(void){ NVIC_callAppropriateGPTMHandle(11, 1);}

//void ADC0Seq0_Handler(void){ ADC_callAppropriateADCHandle(0, 0);}
void ADC0Seq1_Handler(void){ ADC_callAppropriateADCHandle(0, 1);}
void ADC0Seq2_Handler(void){ ADC_callAppropriateADCHandle(0, 2);}
void ADC0Seq3_Handler(void){ ADC_callAppropriateADCHandle(0, 3);}
void ADC1Seq0_Handler(void){ ADC_callAppropriateADCHandle(0, 0);}
void ADC1Seq1_Handler(void){ ADC_callAppropriateADCHandle(0, 1);}
void ADC1Seq2_Handler(void){ ADC_callAppropriateADCHandle(0, 2);}
void ADC1Seq3_Handler(void){ ADC_callAppropriateADCHandle(0, 3);}
