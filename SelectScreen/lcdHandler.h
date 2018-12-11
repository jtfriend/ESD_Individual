/*
 * lcdHandler.h
 *
 *  Created on: 17 Nov 2015
 *      Author: Len Biro
 */

#ifndef LCDHANDLER_H_
#define LCDHANDLER_H_
void initDisplay (void);
extern char DisplayBuffer[96][12];
void initDisplayBuffer(char);
void outputDisplayBuffer(char,char);
void setText (char, char, char *, int, int);
//void setText(char x, char y, char * str, int reverseBit = 0) {

#endif /* LCDHANDLER_H_ */
