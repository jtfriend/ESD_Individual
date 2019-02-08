/*
 * Uart_A0.h
 *
 *  Created on: 14 Dec 2016
 *      Author: Len Biro
 */

#ifndef UART_A0_H_
#define UART_A0_H_

/**
 * 17/12/2016 L.O'Brien
 * @param
 * Modified example code, original clock setting disabled and new divisors
 * set to produce 9600 baud with launchpad default clock.
 */
extern void initUart (void);
/**
 * 17/12/2016 L.O'Brien
 * @param pointer to null terminated char array
 * First character is copied to the transmit buffer and interrupt enabled,
 * thereafter and until termination character the string is transmitted by
 * interrupts.
 * Char array ought to be no more than 32, change it if you wish
 */
extern void putString ( char *);
/**
 * 17/12/2016 L.O'Brien
 * @param
 */
extern void setBaudRate (int);

#endif /* UART_A0_H_ */
