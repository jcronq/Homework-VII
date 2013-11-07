/*
Author: Jason Cronquist
*/
#include "JLib.h"
//Write
void write_byte(BYTE_ADDRESS addr, unsigned short offset, BYTE map_8Bit)
{
	addr[offset] = map_8Bit;
}
void write_halfWord(BYTE_ADDRESS addr, unsigned short offset, HALF_WORD map_16Bit)
{
	((HALF_WORD_ADDRESS)addr)[offset/2] = map_16Bit;
}
void write_word(BYTE_ADDRESS addr, unsigned short offset, WORD map_32Bit)
{
	((WORD_ADDRESS)addr)[offset/4] = map_32Bit;
}
void setBit_word(BYTE_ADDRESS addr, volatile unsigned short offset, unsigned char bitNumber, bool enable)
{
	if(enable)
		((WORD_ADDRESS)addr)[offset/4] |=  (((WORD)0x01)<<bitNumber);
	else
		((WORD_ADDRESS)addr)[offset/4] &= ~(((WORD)0x01)<<bitNumber);
		
}
void setBit_halfWord(BYTE_ADDRESS addr, volatile unsigned short offset, unsigned char bitNumber, bool enable){
	if(enable)
		((HALF_WORD_ADDRESS)addr)[offset/2] |=  (((HALF_WORD)0x01)<<bitNumber);
	else
		((HALF_WORD_ADDRESS)addr)[offset/2] &= ~(((HALF_WORD)0x01)<<bitNumber);
}

//Read
BYTE read_byte(BYTE_ADDRESS addr, unsigned short offset){
	return addr[offset];
}
HALF_WORD read_halfWord(BYTE_ADDRESS addr, unsigned short offset){
	return ((HALF_WORD_ADDRESS)addr)[offset/2];
}
WORD read_word(BYTE_ADDRESS addr, unsigned short offset){
	return ((WORD_ADDRESS)addr)[offset/4];
}

BYTE getBit(BYTE_ADDRESS addr, unsigned short offset, unsigned char pin){
	return (((((WORD_ADDRESS)addr)[offset/4])>>pin)&0x01);
}

//Read Modify Write
void readModWrite_byte(BYTE_ADDRESS port, unsigned short offset, BYTE map_8bit, bool enable){
	if(enable)
		((WORD_ADDRESS)port)[offset/4] |=  ((WORD)map_8bit)<<(offset%4);
	else
		((WORD_ADDRESS)port)[offset/4] &= ~((WORD)map_8bit)<<(offset%4);
}
void readModWrite_halfWord (BYTE_ADDRESS port ,unsigned short offset ,HALF_WORD map_16bit ,bool enable ){
	if(enable)
		((WORD_ADDRESS)port)[offset/4] |=  ((WORD)map_16bit)<<(offset%2);
	else
		((WORD_ADDRESS)port)[offset/4] &= ~((WORD)map_16bit)<<(offset%2);
}
void readModWrite_word(BYTE_ADDRESS port ,unsigned short offset ,WORD map_32bit ,bool enable){
	if(enable)
		((WORD_ADDRESS)port)[offset/4] |=  map_32bit;
	else
		((WORD_ADDRESS)port)[offset/4] &= ~map_32bit;
}

//DO NOT USE -> Out Of Date (selectiveClearAndWrite_# better)
void clearModWriteRange_word(BYTE_ADDRESS port, unsigned short offset, WORD map_32Bit, bool enable, unsigned char startBit, unsigned char size){
	WORD clearBits = 0x01;
	int i = 0;
	for(i = 1; i < size; ++i)
	{
			clearBits = clearBits << 1;
			clearBits |= 0x01;
	}
	clearBits = clearBits << startBit;
	readModWrite_word(port,offset,clearBits, DISABLE);
	readModWrite_word(port, offset, map_32Bit, ENABLE);
}
//END 
//DO NOT USE

void selectiveClearAndWrite_word(BYTE_ADDRESS port, unsigned short offset, WORD clear_32bit, WORD write_32bit){
	((WORD_ADDRESS)port)[offset/4] = ((((WORD_ADDRESS)port)[offset/4]&(~clear_32bit))|write_32bit);
}
void selectiveClearAndWrite_halfWord(BYTE_ADDRESS port, unsigned short offset, HALF_WORD clear_16bit, HALF_WORD write_16bit){
	((WORD_ADDRESS)port)[offset/4] = ((((WORD_ADDRESS)port)[offset/4]&(((WORD)(~clear_16bit))<<(offset%2)))|(((WORD)write_16bit)<<(offset%2)));
}
void selectiveClearAndWrite_byte(BYTE_ADDRESS port, unsigned short offset, BYTE clear_8bit, BYTE write_8bit){
	((WORD_ADDRESS)port)[offset/4] = ((((WORD_ADDRESS)port)[offset/4]&(((WORD)(~clear_8bit))<<(offset%4)))|(((WORD)write_8bit)<<(offset%4)));
}

