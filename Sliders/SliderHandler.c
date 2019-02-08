/*
 * SliderHandler.c
 *
 *  Created on: 28 Nov 2016
 *      Author: Len Biro
 */
#include <msp430.h>
#include "lcdHandler.h"
#include "stdio.h"
#include "string.h"

#define WORD unsigned int
#define ACCUMULATION_CYCLES 1000

/*!
 *  ======== TIMER3_A0_ISR ========
 *  @ingroup ISR_GROUP
 *  @brief  TIMER3_A0_ISR
 *
 *          This ISR clears the LPM bits found in the Status Register (SR/R2).
 * 
 *  @param none
 *  @return none
 */
#pragma vector=TIMER3_A0_VECTOR
__interrupt void TIMER3_A0_ISR(void)

{
  __bic_SR_register_on_exit(LPM3_bits);           // Exit LPM3 on reti
}

#define NOOFELEMENTS	6
#define LEFT_UPPER 		0
#define LEFT_CENTRE 	1
#define LEFT_LOWER 		2
#define RIGHT_UPPER 	3
#define RIGHT_CENTRE 	4
#define RIGHT_LOWER 	5

unsigned int elementNumber = 0;
char capacitance[12] = "";

void initSliderHandler(void) {
  //Set relevant digital I/O pins for Capacitance Touch.
  P1REN |= BIT3;
  P1DIR |= BIT3;
  P1OUT &= ~BIT3;
  P1REN |= BIT4;
  P1DIR |= BIT4;
  P1OUT &= ~BIT4;
  P1REN |= BIT5;
  P1DIR |= BIT5;
  P1OUT &= ~BIT5;

  P3REN |= BIT4;
  P3DIR |= BIT4;
  P3OUT &= ~BIT4;
  P3REN |= BIT5;
  P3DIR |= BIT5;
  P3OUT &= ~BIT5;
  P3REN |= BIT6;
  P3DIR |= BIT6;
  P3OUT &= ~BIT6;

  //TA2 set as timer with RO as clock and TA3 as gated enable. CCIS is
  // toggled between gnd and vcc.
  TA2CTL = TASSEL_3 + MC_2;

  //TA3 set as gate.
  TA3CCR0 = ACCUMULATION_CYCLES;
  TA3CTL = TASSEL_2 + MC_1;
  TA3CCTL0 = CCIE;

  // Capacitive Touch.
  CAPTIO0CTL = CAPTIOEN + (3 << 4) + (4 << 1);
}

void slider_handler() {
  // Capture initialisation.
  // Clear the capacitance counter, set the trigger input to GND.
  // Clear gate flags.
  TA2CTL |= TACLR;
  TA2CCTL0 = CM_1 + CCIS_2 + CAP + SCS;
  TA3CTL &= ~CCIFG;
  TA3CTL |= TACLR;
  //Put the CPU to sleep, for ACCUMULATION_CYCLES of SMCLK.
  __bis_SR_register(LPM0_bits + GIE);
  //Set capacitance counter trigger to Vcc.
  TA2CCTL0 |= CCIS_3;
  //Process capacitance count
  sprintf(capacitance, "  %d  ", TA2CCR0);
  setText(0, 8, "            ");
  setText(0, 16 + (8 * elementNumber), capacitance);
  outputDisplayBuffer(0, 96);

  //Sense next element.
  if (++elementNumber >= (NOOFELEMENTS))
    elementNumber = 0;
  switch (elementNumber) {
    case LEFT_UPPER:
      CAPTIO0CTL = CAPTIOEN + (3 << 4) + (4 << 1);
      break;
    case LEFT_CENTRE:
      CAPTIO0CTL = CAPTIOEN + (3 << 4) + (5 << 1);
      break;
    case LEFT_LOWER:
      CAPTIO0CTL = CAPTIOEN + (3 << 4) + (6 << 1);
      break;
    case RIGHT_UPPER:
      CAPTIO0CTL = CAPTIOEN + (1 << 4) + (5 << 1);
      break;
    case RIGHT_CENTRE:
      CAPTIO0CTL = CAPTIOEN + (1 << 4) + (4 << 1);
      break;
    case RIGHT_LOWER:
      CAPTIO0CTL = CAPTIOEN + (1 << 4) + (3 << 1);
      break;
  }

  setText(60, 20, "test");

  char elementStr[10];

  sprintf(elementStr, "%d", elementNumber);

  if (TA2CCR0 < 1000) {
    setText(60, 80, elementStr);
  }




}

