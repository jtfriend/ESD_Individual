#include <msp430.h> 

/*
 * main.c
 */
#include "lcdHandler.h"
#include "menuHandler.h"
#include "timer.h"
//#include "ADC_1.h"
#include "stdio.h"
#include "string.h"



int main(void) {

	  WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	  PM5CTL0 &= ~LOCKLPM5; // Disable the GPIO power-on default high-impedance mode


	  initTimer();
	  initDisplay();
	  outputDisplayBuffer(0, 96);
	  _BIS_SR (GIE);

	  outputDisplayBuffer(0,96);

	  while (1) {
	    menuHandler(option);
				    outputDisplayBuffer(0, 96);
	  }

	  return 0;
}
