/*
Author: Jason Cronquist
*/

#ifndef JLib_h
#define JLib_h

#include <stdlib.h>
#include <string.h>

//#define LANDSCAPE

typedef struct {
	char* value;
	unsigned int length;
} String;

typedef enum {false, true} bool;
typedef void (*Function)(unsigned char); 
#define ENABLE true
#define DISABLE false

#define BYTE unsigned char
#define HALF_WORD unsigned short
#define WORD unsigned int

#define BYTE_ADDRESS volatile unsigned char*
#define HALF_WORD_ADDRESS volatile unsigned short*
#define WORD_ADDRESS volatile unsigned int*

//SYNTAX:
// <action>_<allignment>(BYTE Alligned Address, offset, ... );

BYTE read_byte(BYTE_ADDRESS, unsigned short);
HALF_WORD read_halfWord(BYTE_ADDRESS, unsigned short);
WORD read_word(BYTE_ADDRESS, unsigned short);

void write_byte(BYTE_ADDRESS, unsigned short, BYTE);
void write_halfWord(BYTE_ADDRESS, unsigned short, HALF_WORD);
void write_word(BYTE_ADDRESS, unsigned short, WORD);


void setBit_word(BYTE_ADDRESS, unsigned short, unsigned char, bool);
void setBit_halfWord(BYTE_ADDRESS addr, volatile unsigned short offset, unsigned char bitNumber, bool enable);

BYTE getBit(BYTE_ADDRESS, unsigned short, unsigned char);

void readModWrite_byte(BYTE_ADDRESS, unsigned short, BYTE , bool);
void readModWrite_halfWord (BYTE_ADDRESS,unsigned short ,HALF_WORD ,bool);
void readModWrite_word(BYTE_ADDRESS, unsigned short, WORD , bool);

void clearModWriteRange_word(BYTE_ADDRESS, unsigned short, WORD, bool, unsigned char startBit, unsigned char size);

void selectiveClearAndWrite_word(BYTE_ADDRESS, unsigned short, WORD clear, WORD write);
void selectiveClearAndWrite_halfWord(BYTE_ADDRESS, unsigned short, HALF_WORD clear, HALF_WORD write);
void selectiveClearAndWrite_byte(BYTE_ADDRESS, unsigned short, BYTE clear, BYTE write);

#endif
