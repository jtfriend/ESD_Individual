/*
 * timer.h
 *
 *  Created on: 17 Nov 2015
 *      Author: Len Biro
 */

#ifndef TIMER_H_
#define TIMER_H_

extern int myGlobal;
extern int heartbeat;
extern int step;
extern int option;

extern volatile int adcReading;
void initTimer(void);

#endif /* TIMER_H_ */
