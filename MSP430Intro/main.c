#include <msp430.h> 


/**
 * main.c
 */

int heartbeat = 1;

#pragma vector=TIMER0_A0_VECTOR
interrupt void Timer0_A0(void) {
    heartbeat++;

    if (heartbeat >= 1000) {
        heartbeat = 1;
        P4OUT ^= 0x40;
    }
}


int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	PM5CTL0 &= ~LOCKLPM5;

	P1DIR |= 0x01;

	P4DIR |= 0x40;
	TA0CCR0 = 1024;
	TA0CCTL0 = 0x10;
	TA0CTL = TASSEL_2 + MC_1;

	_BIS_SR(GIE);

	P1DIR &= ~0x02;
	P1REN |= 0x02;

	// Background Loop

	while (1) {
        if ((P1IN & 0x02) == 0) {
            P1OUT |= 0x01; // Set P1.0 on using OR
        } else {
            P1OUT &= (0xFF-0x01); // Set P1.0 off using AND
        }
	}

	return 0;
}
