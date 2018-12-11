/*
 * main.c
 *
 *  Created on: 8 Nov 2016
 *      Author: Len Biro
 */
#include <msp430.h>
#include "lcdHandler.h"
//#include "buttonHandler.h"
//#include "ledHandler.h"
//#include "MenuHandler.h"
#include "SliderHandler.h"
#include "Uart_A0.h"
#include "timer.h"
#include "stdio.h"
#include "string.h"
#include "stdint.h"

volatile uint8_t _10ms_timer;
volatile uint16_t _500ms_timer;

int main(void) {
  // Stop watchdog timer
  WDTCTL = WDTPW | WDTHOLD;
  // Disable the GPIO power-on default high-impedance mode to activate
  // previously configured port settings
  PM5CTL0 &= ~LOCKLPM5;

  //p4.6
  P4DIR &= ~0x40;
  // Set P4.6 pull-up resistor enabled
  P4REN |= 0x40;
  P4OUT |= 0x40;
  // Set P4.6 to output direction
  P4DIR |= 0x40;

  _10ms_timer = 0;
  _500ms_timer = 0;

  initTimer();
  initDisplay();
  outputDisplayBuffer(0, 96);
//  init_button_handler();
//  initLedHandler();
//	initMenuHandler();
  initSliderHandler();
  initUart();
  //Enable global interrupts.
  _BIS_SR(GIE);

  setText(0, 1, "Week Xmas");
  outputDisplayBuffer(0, 96);

  putString("Week Xmas");

  while (1) {
    slider_handler();
  }

  return 0;
}	//main
