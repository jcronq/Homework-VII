/* function prototypes for SSD1289 LCD on Tiva C Launchpad */

#ifndef LCD_H
#define LCD_H

#include "SSI.h"
#include "font8x8.h"
#include "JLib.h"
#include "SYSTIC.h"

typedef enum {white, black, grey, blue, red, magenta, green, cyan, yellow} Color;

typedef enum {Font8x8, Font8x8_basic, BigFont, Terminus, Ubuntu} Font;

struct Vector2{
	HALF_WORD x;
	HALF_WORD y;
};
struct font_module{
	Font fontType;
	BYTE * characterTable;
	BYTE height;
	BYTE width;
	HALF_WORD foreground;
	HALF_WORD background;
};
struct print_Character{
	BYTE c;
	BYTE xPos;
	BYTE yPos;
	BYTE* font;
};

// function to init LCD and GPIO
void lcdInit(void); //base LCD config

// functions to send cmd/data to LCD
void writeCmd(unsigned short cmd); //command: where we write data to
void writeDat(unsigned short dat); //data: what gets written to address given by cmd
void writeReg(unsigned short cmd,unsigned short dat); //either wrapper for above or combine above to avoid function calls

// helper functions
void clearLCD(unsigned short rgb); //set entire LCD to the color rgb (useful for debugging above)
void setCursor(unsigned short x,unsigned short y); //set current pixel to x,y
void setCursorIndex(unsigned int index); //set current pixel to ind (use index to address pixel instead of x,y)

// functions to draw boxes
void makeBox(struct Vector2* pos, struct Vector2* size, HALF_WORD rgb); //create a solid box @x,y of color given by rgb
void makeOpenBox(struct Vector2* pos, struct Vector2* size, BYTE border, struct font_module* font); //create outline of box @x,y of color given by rgb

//Functions for Chars
void drawAsciiChar(BYTE c, struct Vector2* coord, struct font_module* font);
#endif
