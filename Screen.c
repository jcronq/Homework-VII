#include "Screen.h"
#ifdef LANDSCAPE
	#define HEIGHT 30
	#define WIDTH 40
#else
	#define HEIGHT 40
	#define WIDTH 30
#endif
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 8


const HALF_WORD ColorTable[] = {0xFFFF,0x0000,0xF7DE,0x001F,0xF800,0xF81F,0x07E0,0x7FFF,0xFFE0};

struct ScreenLine{
	struct ScreenLine* next;
	struct ScreenLine* prev;
	BYTE charSpace[WIDTH];
	HALF_WORD length;
};

static struct {
	WORD curLoc;
	struct ScreenLine* head;
	struct ScreenLine* tail;
	
	WORD yOffset;
	struct font_module font;
} Screen;

void startScreen(void){
	lcdInit();
	Screen.curLoc = 0;
	Screen.head = malloc (sizeof *Screen.head);
	Screen.head->next = NULL;
	Screen.head->prev = NULL;
	Screen.tail = Screen.head;
	Screen.font.background = getColorCode(black);
	Screen.font.foreground = getColorCode(green);
	Screen.font.fontType = Font8x8;
	Screen.font.height = CHAR_HEIGHT;
	Screen.font.width = CHAR_WIDTH;
	Screen.yOffset = 160;
}
HALF_WORD getXStartPixel(WORD xCoord){
	return (HALF_WORD)(xCoord*CHAR_WIDTH);
}
HALF_WORD getYStartPixel(WORD yCoord){
	return (HALF_WORD)(yCoord * CHAR_HEIGHT)+Screen.yOffset;
}
void initLineToNull(BYTE* line, HALF_WORD count){
	int i = 0;
	count = (count == 0)?WIDTH:count;
	for(; i < count; ++i)
		line[i] = NULL;
}
HALF_WORD getDepth(struct ScreenLine* line){
	HALF_WORD retVal = 0;
	while(line != Screen.head){
		line = line->prev;
		++retVal;
	}
	return retVal;
}
void removeLineByPointer(struct ScreenLine* line){
	if(Screen.head == Screen.tail){
		Screen.curLoc = 0;
		initLineToNull(line->charSpace, line->length);
		line->length = 0;
		return;
	}
	else if(line == Screen.head){
		Screen.head = Screen.head->next;
		Screen.head->prev = NULL;
	}
	else if(line == Screen.tail){
		Screen.tail = Screen.tail->prev;
		Screen.tail->next = NULL;
	}
	else {
		line->prev->next = line->next;
		line->next->prev = line->prev;
	}
	free(line);
}
void removeLineByNumber(HALF_WORD lineNumber){
	if(lineNumber == 0){
		Screen.curLoc = 0;
		initLineToNull(Screen.head->charSpace, Screen.head->length);
	}
}
void removeLastLine(bool alterCurLoc){
	removeLineByPointer(Screen.tail);
}
void drawLine(struct ScreenLine* line){
	struct Vector2* coord = malloc(sizeof *coord);
	WORD x,y;
	y = getYStartPixel(getDepth(line));
	for(x=0; x < line->length;++x){
		coord->x = getXStartPixel(x);coord->y = y;
		drawAsciiChar(line->charSpace[x], coord, &Screen.font);
	}
	free (coord);
}
void addLine(bool alterCurLoc){
	drawLine(Screen.tail);
	if(alterCurLoc){
		Screen.curLoc += WIDTH;
		Screen.curLoc -= (Screen.curLoc%WIDTH);
	}
	Screen.tail->next = malloc(sizeof *Screen.head);
	Screen.tail->next->prev = Screen.tail;
	Screen.tail = Screen.tail->next;
}
void advanceCurLoc(void){
	Screen.curLoc++;
	if(Screen.curLoc%WIDTH == 0)
		addLine(false);
}
void whiteOutLine(struct ScreenLine* line){
	struct Vector2* pos = malloc(sizeof *pos);
	struct Vector2* size = malloc(sizeof *size);
	pos->x  = 0;pos->y = getYStartPixel(getDepth(line));
	size->x = line->length*CHAR_WIDTH;size->y = CHAR_HEIGHT;
	makeBox(pos, size, Screen.font.background);
	free(pos); free(size);
}
//Globally Scoped Functions
void setTextColor(Color color){
	Screen.font.foreground = ColorTable[color];
}
void setBackgroundColor(Color color){
	Screen.font.background = ColorTable[color];
}
void clearText(void){
	while(Screen.head != Screen.tail){
		whiteOutLine(Screen.tail);
		removeLineByPointer(Screen.tail);
	}
	whiteOutLine(Screen.tail);
	removeLineByPointer(Screen.tail);
}
void println(char* string){
	static int rep = 0;
	int i = 0;
	for(; i < strlen(string); ++i){
		if(string[i] == 0x0A)
			addLine(true);
		else{
			Screen.tail->charSpace[Screen.curLoc%WIDTH] = string[i];
			Screen.tail->length++;
		}
		advanceCurLoc();
	}
	rep++;
	if(rep==14)
		rep++;
	addLine(true);
}
void redrawText(void){
	struct ScreenLine* line = Screen.head;
	do{
		drawLine(line);
		line = line->next;
	}while(line != NULL);
}

HALF_WORD getColorCode(Color color){
	return ColorTable[color];
}
