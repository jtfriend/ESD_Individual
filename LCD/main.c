#include <msp430.h>
#include <string.h>


int display_state = 0;
char line;
char column;


const char Font[8][38] =
{
        //  0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F     G     H     I     J     K     L     M     N     O     P     Q     R     S     T     U     V     W     X     Y     Z  SPACE
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x28, 0x08, 0x18, 0x3C, 0x10, 0x3C, 0x08, 0x3E, 0x3C, 0x3C, 0x18, 0x38, 0x38, 0x30, 0x3C, 0x3C, 0x7C, 0x24, 0x7C, 0x7C, 0x40, 0x40, 0x00, 0x44, 0x00, 0x78, 0x3C, 0x7C, 0x3E, 0x7E, 0x44, 0x82, 0x42, 0x00, 0x42, 0x00, 0x00},
        {0x44, 0x18, 0x24, 0x04, 0x10, 0x20, 0x10, 0x02, 0x24, 0x44, 0x24, 0x24, 0x44, 0x28, 0x20, 0x20, 0x44, 0x24, 0x10, 0x10, 0x48, 0x40, 0x42, 0x64, 0x3C, 0x44, 0x42, 0x42, 0x40, 0x08, 0x44, 0x44, 0x42, 0x42, 0x24, 0x7E, 0x00},
        {0x44, 0x08, 0x08, 0x08, 0x20, 0x38, 0x20, 0x04, 0x18, 0x44, 0x24, 0x24, 0x40, 0x24, 0x38, 0x3C, 0x40, 0x24, 0x10, 0x10, 0x50, 0x40, 0x66, 0x54, 0x42, 0x44, 0x42, 0x42, 0x7C, 0x08, 0x44, 0x44, 0x42, 0x24, 0x18, 0x0C, 0x00},
        {0x44, 0x08, 0x10, 0x18, 0x24, 0x04, 0x3C, 0x08, 0x18, 0x3C, 0x3C, 0x38, 0x40, 0x24, 0x20, 0x20, 0x5C, 0x3C, 0x10, 0x10, 0x60, 0x40, 0x5A, 0x4C, 0x42, 0x78, 0x4A, 0x7C, 0x02, 0x08, 0x44, 0x28, 0x5A, 0x18, 0x10, 0x10, 0x00},
        {0x44, 0x08, 0x10, 0x04, 0x3E, 0x04, 0x24, 0x10, 0x24, 0x04, 0x24, 0x24, 0x44, 0x24, 0x20, 0x20, 0x48, 0x24, 0x10, 0x10, 0x50, 0x40, 0x42, 0x44, 0x42, 0x40, 0x3C, 0x44, 0x02, 0x08, 0x44, 0x28, 0x7E, 0x24, 0x20, 0x20, 0x00},
        {0x28, 0x3C, 0x3C, 0x38, 0x04, 0x38, 0x18, 0x10, 0x3C, 0x04, 0x42, 0x38, 0x38, 0x38, 0x3C, 0x20, 0x78, 0x24, 0x7C, 0x70, 0x48, 0x78, 0x42, 0x44, 0x3C, 0x40, 0x02, 0x44, 0x7C, 0x08, 0x38, 0x10, 0x24, 0x42, 0x40, 0x7E, 0x00},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};


char DisplayBuffer[96][96/8];


char reverse(char inchar)
{
    char outchar;

    outchar = 0;

    if ((inchar & BIT0) > 0) outchar |= BIT7;
    if ((inchar & BIT1) > 0) outchar |= BIT6;
    if ((inchar & BIT2) > 0) outchar |= BIT5;
    if ((inchar & BIT3) > 0) outchar |= BIT4;
    if ((inchar & BIT4) > 0) outchar |= BIT3;
    if ((inchar & BIT5) > 0) outchar |= BIT2;
    if ((inchar & BIT6) > 0) outchar |= BIT1;
    if ((inchar & BIT7) > 0) outchar |= BIT0;

    return outchar;
}


void initDisplayBuffer(char setting)
{
    int i;
    int j;

    for (i=0; i<96; i++)
    {
        for(j=0; j<12; j++)
        {
            DisplayBuffer[i][j] = setting;
        }
    }
}

void outputDisplayBuffer()
{
    int line;
    int column;
    char command = 0x80;            // Write lines

    command = command ^ 0x40;       // VCOM bit

    P2OUT |= 0x10;                  // LCD CS high

    while (!(UCB0IFG & UCTXIFG));
    UCB0TXBUF = command;

    for (line=0; line<96; line++)
    {
        while (!(UCB0IFG & UCTXIFG));
        UCB0TXBUF = reverse(line+1);

        for (column=0; column<12; column++)
        {
            while (!(UCB0IFG & UCTXIFG));
            UCB0TXBUF = DisplayBuffer[line][column];
        }

        while (!(UCB0IFG & UCTXIFG));
        UCB0TXBUF = 0x00;            // End of line signal
    }

    while (!(UCB0IFG & UCTXIFG));
    UCB0TXBUF = 0x00;               // End of block signal

    while ((UCB0STATW & UCBBUSY));

    __delay_cycles(8);              //Ensure a 2us min delay to meet the LCD's thSCS

    P2OUT &= ~0x10;                 // LCD CS low
}


/* Desc:    Checks to see if the number is in an array*/
/* Returns: An ingeter, the index in the array or -1 if not found*/
/* Parameters: int needle - a number to be checked against*/
/*             int *haystack - a pointer to an array to be indexed through*/

int findNumberInArray(int needle, int *haystack)
{
    int i;
    /*move to next position in 2d array*/
    haystack++;

    for (i=0; i<28; i++) {
        if (*(haystack) == needle) {
            return i;
        }
        /*move to next first column position in 2d array*/
        haystack= haystack + 2;
    }

    return -1;
}

/* Desc:    Prints word to LCD display*/
/* Returns: void*/
/* Parameters: char word[] - a string to be printed*/
/*             int height - a position in y axis of the LCD display*/

void printWord(char word[], int height)
{
    int k = 0;
    int i = 0;
    int l = 0;
    int index;
    int count = strlen(word);

    /*table to link ascii char to index in array of char*/
    int letterLinkTable[27][2] = {
        {10, 97} , //a
        {11, 98} , //b
        {12, 99} , //c
        {13, 100},
        {14, 101},
        {15, 102},
        {16, 103},
        {17, 104},
        {18, 105},
        {19, 106},
        {20, 107},
        {21, 108},
        {22, 109},
        {23, 110},
        {24, 111},
        {25, 112},
        {26, 113},
        {27, 114},
        {28, 115},
        {29, 116},
        {30, 117},
        {31, 118},
        {32, 119},
        {33, 120},
        {34, 121},
        {35, 122},
        {36, 32}  //space
    };

    for ( l=0; l<count; l++)
    {
        /*finds index to link to table*/
        index = letterLinkTable[findNumberInArray((int)word[l], &letterLinkTable[0][0])][0];

        for (i=0; i<8; i++)
        {
            /*print to display at height*/
            DisplayBuffer[i+height][k] = ~Font[i][index];
        }
        k++;
    }
}

int k = 0;

void buildScene()
{
    initDisplayBuffer(0xFF);

    printWord("    hello   ", 30);
    printWord("    world   ", 40);

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

    P1DIR |= ~0x01;

    P4DIR |= ~0x40;
    TA0CCR0 = 1024;
    TA0CCTL0 = 0x10;
    TA0CTL = TASSEL_2 + MC_1;

    _BIS_SR(GIE);

    // LCD
    P4DIR |= 0x04; // Set P4.2 to output direction (LCD Power On)
    P4DIR |= 0x08; // Set P4.3 to output direction (LCD Enable)
    // SPI Ports
    P1SEL0 &= ~0x40; // Set P1.6 to output direction (SPI MOSI)
    P1SEL1 |= 0x40; // Set P1.6 to output direction (SPI MOSI)
    P1DIR |= 0x40; // Set P1.6 to output direction (SPI MOSI)
    P2SEL0 &= ~0x04; // Set P2.2 to SPI mode (SPI CLK)
    P2SEL1 |= 0x04; // Set P2.2 to SPI mode (SPI CLK)
    P2DIR |= 0x04; // Set P2.2 to output direction (SPI CLK)
    P2DIR |= 0x10; // Set P2.4 to output direction (SPI CS)
    // SPI Interface
    UCB0CTLW0 |= UCSWRST;
    UCB0CTLW0 &= ~(UCCKPH + UCCKPL + UC7BIT + UCMSB);
    UCB0CTLW0 &= ~(UCSSEL_3);
    UCB0CTLW0 |= UCSSEL__SMCLK;
    UCB0BRW = 8;
    UCB0CTLW0 |= (UCMSB + UCCKPH + 0x00 + UCMST + UCSYNC + UCMODE_0);
    UCB0CTLW0 &= ~(UCSWRST);
    P4OUT |= 0x04; // Turn LCD Power On
    P4OUT |= 0x08; // Enable LCD
    P1OUT &= ~0x01; // Set P1.0 off (Green LED)

    // Background Loop
    for (;;) {
        buildScene();
        outputDisplayBuffer();
    }
}

