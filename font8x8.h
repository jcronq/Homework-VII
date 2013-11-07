#ifndef font8x8_h
#define font8x8_h
/*  
    8x8 CGA font - 0x00 through 0x7f
    from http://code.google.com/p/surveyor-srv1-firmware/source/browse/trunk/blackfin/srv/font8x8.h
*/

#include "JLib.h"

bool f8x8_getPixel(BYTE x, BYTE y, BYTE character);

#endif
