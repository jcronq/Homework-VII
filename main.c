#include "JLib.h"
#include "SystemClock.h"
#include "DACTimer.h"
int main(void)
{
	setSysClkTo_80MHz();	
	initTimersAndInterruptsForDacOutput();
	
	while(1)
	{		
		
	}		
}
