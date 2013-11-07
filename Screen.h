#ifndef Screen_h
#define Screen_h

#include "LCD.h"
#include "font8x8.h"

void startScreen(void);

void setTextColor(Color color);
void setBackgroundColor(Color color);

void println(char* string);
void clearText(void);

HALF_WORD getColorCode(Color color);

void redrawText(void);

#endif
