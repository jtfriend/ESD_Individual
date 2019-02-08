/*
 * timer.c
 *
 *  Created on: 17 Nov 2015
 *      Author: Len Biro
 */
#include <msp430.h>

int heartbeat = 1;
int adcReading = 1;
int step = 1;
int option = 1;

#pragma vector=TIMER0_A0_VECTOR

__interrupt void Timer0_A0(void) { // Timer0 A0 1ms interrupt service routine

  step++;

  if (step >= 3000) {
    step = 0;
    option++;
  }

	//Time to take a reading
	adcReading++;
	if (adcReading >= 40){
		;
		adcReading = 1;
	}
	// Toggle the red LED 500ms on and 500ms off
	heartbeat++;
	if (heartbeat >= 500) {
		heartbeat = 1;
		P4OUT ^= 0x40;
	}
} //Timer




void initTimer(void) {
    P4DIR &= ~0x40; //p4.6
	P4REN |= 0x40; // Set P1.1 pull-up resistor enabled
    P4OUT |= 0x40;
	P4DIR |= 0x40;  // Set P4.6 to output direction

	TA0CCR0 = 1023; // Count up to 1024
	TA0CCTL0 = 0x10; // Enable counter interrupts, bit 4=1
	  TA0CCTL1 = OUTMOD_3;                      // TACCR1 set/reset
	  TA0CCR1 = 1023;                           // TACCR1 PWM Duty Cycle
	TA0CTL = TASSEL_2 + MC_1; // Timer A using subsystem master clock, SMCLK(1.1 MHz)
							  // and count UP to create a 1ms interrupt
	                          // PWM Period
}
