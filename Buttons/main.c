#include <msp430.h>
#include <string.h>


#define BUTTON_RELEASED 0
#define BUTTON_PRESSED 1

int button_state = BUTTON_RELEASED;

void button_handler() {
    switch (button_state) {
        case BUTTON_RELEASED:
            P1OUT &= (0xFF-0x01); // Set P1.0 off using AND ....P1OUT = P1OUT & (0xFF-0x01)
            if ((P1IN & 0x02) == 0) {
                button_state = BUTTON_PRESSED;
            }
            break;

        case BUTTON_PRESSED:
            P1OUT |= 0x01; // Set P1.0 on using OR
            if ((P1IN & 0x02) != 0) {
                 button_state = BUTTON_RELEASED;
            }
            break;
        default:
            break;
    }
}


int heartbeat = 1;

#pragma vector=TIMER0_A0_VECTOR
interrupt void Timer0_A0(void) {
    heartbeat++;

    if (heartbeat >= 500) {
        heartbeat = 1;
        P4OUT ^= 0x40;
    }
}

/**
 * main.c
 */
int main(void)
{

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    PM5CTL0 &= ~LOCKLPM5;

    //set up buttons, send val, then set resistor with return val
    P4DIR |= BIT3;
    P4REN |= BIT3;
    P4OUT |= BIT3;
    P4DIR = 0xFF ^ BIT3;

    P3DIR |= BIT0;
    P3REN &= ~BIT0;
    P3OUT |= BIT0;
    P3DIR &= (0xFF ^ BIT0);

    P1OUT &= ~BIT1;                           // Clear LED to start
    P1DIR |= BIT1;                            // Set P1.0/LED to output

    TA0CCR0 = 1024;
    TA0CCTL0 = 0x10;
    TA0CTL = TASSEL_2 + MC_1;

    _BIS_SR(GIE);

    // LCD
//    P4DIR |= 0x04; // Set P4.2 to output direction (LCD Power On)
//    P4DIR |= 0x08; // Set P4.3 to output direction (LCD Enable)
    // SPI Ports
//    P1SEL0 &= ~0x40; // Set P1.6 to output direction (SPI MOSI)
//    P1SEL1 |= 0x40; // Set P1.6 to output direction (SPI MOSI)
//    P1DIR |= 0x40; // Set P1.6 to output direction (SPI MOSI)
//    P2SEL0 &= ~0x04; // Set P2.2 to SPI mode (SPI CLK)
//    P2SEL1 |= 0x04; // Set P2.2 to SPI mode (SPI CLK)
//    P2DIR |= 0x04; // Set P2.2 to output direction (SPI CLK)
//    P2DIR |= 0x10; // Set P2.4 to output direction (SPI CS)
//    // SPI Interface
//    UCB0CTLW0 |= UCSWRST;
//    UCB0CTLW0 &= ~(UCCKPH + UCCKPL + UC7BIT + UCMSB);
//    UCB0CTLW0 &= ~(UCSSEL_3);
//    UCB0CTLW0 |= UCSSEL__SMCLK;
//    UCB0BRW = 8;
//    UCB0CTLW0 |= (UCMSB + UCCKPH + 0x00 + UCMST + UCSYNC + UCMODE_0);
//    UCB0CTLW0 &= ~(UCSWRST);
//    P4OUT |= 0x04; // Turn LCD Power On
//    P4OUT |= 0x08; // Enable LCD
//    P1OUT = 0x01; // Set P1.0 off (Green LED)

    // Background Loop

    for (;;) {
//        button_handler();
    }

}


