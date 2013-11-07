#include "LCD.H"

#ifdef LANDSCAPE
	#define P_HEIGHT 240
	#define P_WIDTH 320
#else
	#define P_HEIGHT 320
	#define P_WIDTH 240
#endif

//Port E
#define ctrlPort ((BYTE_ADDRESS)0x40024000)
//Port B
#define dataPort ((BYTE_ADDRESS)0x40005000)
// (RD)PC.4 bit band address
#define RD  ((volatile unsigned char *) 0x420C7F90)
// (WR)PC.5 bit band address
#define WR ((volatile unsigned char *) 0x420C7F94)
// (RST)PC.6 bit band address
#define RST ((volatile unsigned char *) 0x420C7F98)
// (DEN)PE.1 bit band address
#define DEN = ((volatile unsigned char *) 0x42487F84)
// (DDIR)PE.2 bit band address
#define DDIR ((volatile unsigned char *) 0x42487F88)
// (RS)PE.3 bit band address
#define RS ((volatile unsigned char *) 0x42487F8C)
// (DLE)PE.4 bit band address
#define DLE  ((volatile unsigned char *) 0x42487F90)
// (CS)PE.5 bit band address
#define CS  ((volatile unsigned char *) 0x42487F94)


/////////////////////////////////////////////////////////////////////////
// lcdPortConfig
// Setup GPIO for LCD Screen
// Inputs:
//		NONE
// Outputs:
//		NONE
/////////////////////////////////////////////////////////////////////////
void lcdPortConfig(void) //setup GPIO pins
{
	GPIO_initPortClocksWithMap(0x17, ENABLE);	
	GPIO_initPortPinsForIO(GPIO_getPortAddress('b'), 0xFF, IO_OUT, IO_PULL_UP, false);
	GPIO_initPortPinsForIO(GPIO_getPortAddress('c'), 0x70, IO_OUT, IO_PULL_UP, false);
	GPIO_initPortPinsForIO(GPIO_getPortAddress('e'), 0x3E, IO_OUT, IO_PULL_UP, false);
	CS[0] = 1;	// set chip select to high
	RD[0] = 1;
	WR[0] = 0;
}
// End lcdPortConfig ///////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
// lcdInit
// Delay a given number of milliseconds
// Inputs:
//		delay - int with the value of time delay needed in milliseconds
// Outputs:
//		NONE
/////////////////////////////////////////////////////////////////////////
void lcdInit(void)
{
	lcdPortConfig();
	//It may be a good idea to reset the LCD and turn on the backlight here (can be done using GPIO)
	//LCD reset
	//backlight
	
	RST[0] = 1; // Ensure reset is turned off
	
	//see the datasheet for an explanation of the following
	//  (shouldn't need to change anything below)
	writeReg(0x0000,0x0001);    delayMS(50);   /* Enable LCD Oscillator */
	writeReg(0x0003,0xA8A4);    delayMS(50);   // Power control(1)
	writeReg(0x000C,0x0000);    delayMS(50);   // Power control(2)
	writeReg(0x000D,0x080C);    delayMS(50);   // Power control(3)
	writeReg(0x000E,0x2B00);    delayMS(50);   // Power control(4)
	writeReg(0x001E,0x00B0);    delayMS(50);   // Power control(5)
	writeReg(0x0001,0x2B3F);    delayMS(50);   // Driver Output Control /* 320*240 0x2B3F */
	writeReg(0x0002,0x0600);    delayMS(50);   // LCD Drive AC Control
	writeReg(0x0010,0x0000);    delayMS(50);   // Sleep Mode off
	writeReg(0x0011,0x6070);    delayMS(50);   // Entry Mode
	writeReg(0x0005,0x0000);    delayMS(50);   // Compare register(1)
	writeReg(0x0006,0x0000);    delayMS(50);   // Compare register(2)
	writeReg(0x0016,0xEF1C);    delayMS(50);   // Horizontal Porch
	writeReg(0x0017,0x0003);    delayMS(50);   // Vertical Porch
	writeReg(0x0007,0x0133);    delayMS(50);   // Display Control
	writeReg(0x000B,0x0000);    delayMS(50);   // Frame Cycle control
	writeReg(0x000F,0x0000);    delayMS(50);   // Gate scan start position
	writeReg(0x0041,0x0000);    delayMS(50);   // Vertical scroll control(1)
	writeReg(0x0042,0x0000);    delayMS(50);   // Vertical scroll control(2)
	writeReg(0x0048,0x0000);    delayMS(50);   // First window start
	writeReg(0x0049,0x013F);    delayMS(50);   // First window end
	writeReg(0x004A,0x0000);    delayMS(50);   // Second window start
	writeReg(0x004B,0x0000);    delayMS(50);   // Second window end
	writeReg(0x0044,0xEF00);    delayMS(50);   // Horizontal RAM address position
	writeReg(0x0045,0x0000);    delayMS(50);   // Vertical RAM address start position
	writeReg(0x0046,0x013F);    delayMS(50);   // Vertical RAM address end position
	writeReg(0x0030,0x0707);    delayMS(50);   // gamma control(1)
	writeReg(0x0031,0x0204);    delayMS(50);   // gamma control(2)
	writeReg(0x0032,0x0204);    delayMS(50);   // gamma control(3)
	writeReg(0x0033,0x0502);    delayMS(50);   // gamma control(4)
	writeReg(0x0034,0x0507);    delayMS(50);   // gamma control(5)
	writeReg(0x0035,0x0204);    delayMS(50);   // gamma control(6)
	writeReg(0x0036,0x0204);    delayMS(50);   // gamma control(7)
	writeReg(0x0037,0x0502);    delayMS(50);   // gamma control(8)
	writeReg(0x003A,0x0302);    delayMS(50);   // gamma control(9)
	writeReg(0x003B,0x0302);    delayMS(50);   // gamma control(10)
	writeReg(0x0023,0x0000);    delayMS(50);   // RAM write data mask(1)
	writeReg(0x0024,0x0000);    delayMS(50);   // RAM write data mask(2)
	writeReg(0x0025,0x8000);    delayMS(50);   // Frame Frequency
	writeReg(0x004f,0);                         // Set GDDRAM Y address counter
	writeReg(0x004e,0);                         // Set GDDRAM X address counter
	
	delayMS(50);
	
	#ifdef LANDSCAPE
		writeReg(0x0011, 0x6068);
	#endif
}
// End lcdInit ///////////////////////////////////////////////////////////



//****************************************************************
// functions to send cmd/data to LCD
//****************************************************************

/////////////////////////////////////////////////////////////////////////
// writeCmd
// write a command to LCD
// Inputs:
//		cmd - short with command value
// Outputs:
//		NONE
/////////////////////////////////////////////////////////////////////////
void writeCmd(unsigned short cmd) 
{
	ctrlPort[0x3FC] = 0x34;
	dataPort[0x3FC] = (cmd & 0xFF);	// send first 8 bits of command to latch
	ctrlPort[0x3FC] = 0x24;
	dataPort[0x3FC] = ((cmd >> 8) & 0xFF);	// send second 8 bits of command to latch
	ctrlPort[0x3FC] = 0x04;
	ctrlPort[0x3FC] = 0x24;
}
// End writeCmd ///////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
// writeDat
// Write data to LCD
// Inputs:
//		dat - short with data value
// Outputs:
//		NONE
// NOTE: HIGHLY Optimized. Do not touch.
/////////////////////////////////////////////////////////////////////////
void writeDat(unsigned short dat) 
{
	ctrlPort[0x3FC] = 0x3C;
	dataPort[0x3FC] = (dat & 0xFF);	// send first 8 bits of command to latch
	ctrlPort[0x3FC] = 0x2C;	
	dataPort[0x3FC] = ((dat >> 8) & 0xFF);	// send second 8 bits of command to latch
	ctrlPort[0x3FC] = 0x0C;
	ctrlPort[0x3FC] = 0x2C;
}
// End writeDat ///////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
// writeReg
// Writes a command and data to the LCD
// Inputs:
//		cmd - short with command value
//		dat - short with data value
// Outputs:
//		NONE
/////////////////////////////////////////////////////////////////////////
void writeReg(unsigned short cmd,unsigned short dat) 
{
	writeCmd(cmd);	// write command
	writeDat(dat);	// write data
}
// End writeReg ///////////////////////////////////////////////////////////


//****************************************************************
// helper functions
//****************************************************************


/////////////////////////////////////////////////////////////////////////
// clearLCD
// sets the entire screen to one color
// Inputs:
//		rgb - 16bit color value for screen color
// Outputs:
//		NONE
/////////////////////////////////////////////////////////////////////////
void clearLCD(unsigned short rgb) //set entire LCD to the color rgb (useful for debugging above)
{
	unsigned int i = 0;
	
	setCursor(0,0);	// set current pixel address to 0, 0
	writeCmd(0x0022); // tell lcd to expect pixel data for current pixel address
	for(i=0; i<76800; i++)
	{
		writeDat(rgb); // write pixel to rgb color
	}
}
// End clearLCD ///////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////
// setCursor
// set the cursor to the given x, y coordinate
// Inputs:
//		x - x coordinate for cursor
//		y - y coordinate for cursor
// Outputs:
//		NONE
/////////////////////////////////////////////////////////////////////////
void setCursor(unsigned short x,unsigned short y) //set current pixel to x,y
{
	#ifdef LANDSCAPE
		writeReg(0x004f,x);
		writeReg(0x004e,(P_HEIGHT-y)-1);
	#else
		writeReg(0x004f,y);                         // Set GDDRAM Y address counter
		writeReg(0x004e,x);                         // Set GDDRAM X address counter
	#endif
}
// End setCursor ///////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////
// setCursorIndex
// calculates coordinate of pixels and moves cursor to that index
// Inputs:
//		index - index coordinate for cursor
// Outputs:
//		NONE
/////////////////////////////////////////////////////////////////////////
void setCursorIndex(unsigned int index) //set current pixel to ind (use index to address pixel instead of x,y)
{
	
}
// End setCursorIndex ///////////////////////////////////////////////////////////

void clearAsciiChar(struct Vector2* coord, struct font_module* font){
	WORD i = coord->x;
	WORD j = coord->y;
	for(;i  <font->width+coord->x;++i){
		setCursor(i,j);
		writeCmd(0x0022);
		for(; j < font->height+(coord->y);++j)
			writeDat(font->background);
	}
}

void drawAsciiChar(BYTE c, struct Vector2* coord, struct font_module* font){
	WORD i = 0;
	WORD j = 0;
	WORD x = coord->x;
	WORD y = coord->y;
	
	setCursor(x,y);
	for(i= 0; i < font->height;++i){
		writeCmd(0x0022);
		for(j = 0; j < font->width;++j){
			if(f8x8_getPixel(j,i,c))
				writeDat(font->foreground);
			else
				writeDat(font->background);
		}
		setCursor(x,++y);
	}
}

//****************************************************************
// functions to draw boxes
//****************************************************************

/////////////////////////////////////////////////////////////////////////
// makeBox
// Draws a box on the screen with the given color and at the index given
// Inputs:
//		x - x coordinate for cursor
//		y - y coordinate for cursor
//		rgb - color of box outline
// Outputs:
//		NONE
/////////////////////////////////////////////////////////////////////////
void makeBox(struct Vector2* pos, struct Vector2* size,unsigned short rgb){ //create a solid box @x,y of color given by rgb
	WORD x = 0;
	WORD y = 0;
	setCursor(pos->x,pos->y);
	for(y = 0; y < size->y; ++y){
		writeCmd(0x0022); // tell lcd to expect pixel data for current pixel address
		for(x = 0; x < size->x; ++x){
			writeDat(rgb); // write box pixels to rgb color
		}
		setCursor(pos->x,++pos->y); // set cursor to next row in box
	}
}
// End makeBox ///////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// makeOpenBox
// Draws an open box on the screen with the given color and at the index given
// Inputs:
//		x - x coordinate for cursor
//		y - y coordinate for cursor
//		rgb - color of box outline
// Outputs:
//		NONE
/////////////////////////////////////////////////////////////////////////
void makeOpenBox(struct Vector2* pos, struct Vector2* size, BYTE border, struct font_module* font) //create outline of box @x,y of color given by rgb
{
	WORD x = 0;
	WORD y = 0;
	setCursor(x,y);
	for(y = 0; y < size->y; ++y){
		writeCmd(0x0022); // tell lcd to expect pixel data for current pixel address
		for(x = 0; x < size->x; ++y){
			if((x < border) || (x > (size->x-border)) || (y < border) || (y > (size->y-border)))
				writeDat(font->foreground);
			else
				writeDat(font->background);
		}
		setCursor(x++,y); // reset cursor to next row in box
	}
}
// End makeOpenBox ///////////////////////////////////////////////////////////
