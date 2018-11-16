#include <msp430.h>
#include <string.h>


int display_state = 0;
char line;
char column;

const char Font[8][16] =
{
        //  0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F  SPACE
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x28, 0x08, 0x18, 0x3C, 0x10, 0x3C, 0x08, 0x3E, 0x3C, 0x3C, 0x18, 0x38, 0x38, 0x30, 0x3C, 0x3C, 0x00},
        {0x44, 0x18, 0x24, 0x04, 0x10, 0x20, 0x10, 0x02, 0x24, 0x44, 0x24, 0x24, 0x44, 0x28, 0x20, 0x20, 0x00},
        {0x44, 0x08, 0x08, 0x08, 0x20, 0x38, 0x20, 0x04, 0x18, 0x44, 0x24, 0x24, 0x40, 0x24, 0x38, 0x3C, 0x00},
        {0x44, 0x08, 0x10, 0x18, 0x24, 0x04, 0x3C, 0x08, 0x18, 0x3C, 0x3C, 0x38, 0x40, 0x24, 0x20, 0x20, 0x00},
        {0x44, 0x08, 0x10, 0x04, 0x3E, 0x04, 0x24, 0x10, 0x24, 0x04, 0x24, 0x24, 0x44, 0x24, 0x20, 0x20, 0x00},
        {0x28, 0x3C, 0x3C, 0x38, 0x04, 0x38, 0x18, 0x10, 0x3C, 0x04, 0x42, 0x38, 0x38, 0x38, 0x3C, 0x20, 0x00},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};


const char Tank[8][4] =
{
        {0x49, 0xFF, 0x82, 0x00},
        {0x49, 0x1C, 0x82, 0xFF},
        {0x49, 0x1C, 0xFE, 0x38},
        {0x7F, 0xFC, 0xFE, 0x38},
        {0x7F, 0x1C, 0xFE, 0x3F},
        {0x7F, 0x1C, 0x92, 0x38},
        {0x41, 0xFF, 0x92, 0x38},
        {0x41, 0x00, 0x92, 0xFF}
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

int findNumberInArray(int number, int *haystack)
{
    int i;
    haystack++;

    for (i=0; i<3; i++) {
        if (*(haystack) == number) {
            return i;
        }
        haystack= haystack + 2;
    }

    return -1;
}


void printWord(char word[], int height)
{
    int k = 0;
    int i = 0;
    int l = 0;
    int index;
    int count = strlen(word);

    int letterLinkTable[3][2] = {
       {10, 97} ,
       {11, 98} ,
       {12, 99} ,
       {13, 100},
       {14, 101},
       {15, 102},
    };

    for ( l=0; l<count; l++)
    {
        index = letterLinkTable[findNumberInArray((int)word[l], &letterLinkTable[0][0])][0];

        for (i=0; i<8; i++)
        {
            DisplayBuffer[i+height][k] = ~Font[i][index];
        }
        k++;
    }
}

int k = 0;

void buildScene()
{
    int i;
    int j;

    initDisplayBuffer(0xFF);

    printWord("ababababdef", 0);
    printWord("aaaaaaaaaaaa", 10);
    printWord("abababababab", 20);
    printWord("aaaaaaaaaaaa", 30);

    printWord("a", 40);

    //Display Numbers

//    for (i=0; i<8; i++)
//    {
//        for(j=0; j<10; j++)
//        {
//            DisplayBuffer[i+72][j] = ~Font[i][j];
//        }
//    }

    //Display letters A - F

//    for (i=0; i<8; i++)
//    {
//        for(j=0; j<6; j++)
//        {
//            DisplayBuffer[i+80][j] = ~Font[i][j+10];
//        }
//    }

    //Display Tanks
//    for (i=0; i<8; i++)
//    {
//        DisplayBuffer[i+10][3] = ~Tank[i][0];
//        DisplayBuffer[i+10][5] = ~Tank[i][1];
//        DisplayBuffer[i+10][7] = ~Tank[i][2];
//        DisplayBuffer[i+10][9] = ~Tank[i][3];
//    }

    //A moving tank downwards
//    for (i=0; i<8; i++)
//    {
//        DisplayBuffer[(i/8)+k+10][3] = ~Tank[i][0];
//        k = (k+1) % 80;
//    }

    //Moving a downwards
    for (i=0; i<8; i++)
    {
        DisplayBuffer[(i/8)+k+10][3] = ~Font[i][10];
        k = (k+1) % 80;
    }

//    for (i=0; i<8; i++)
//    {
//        DisplayBuffer[70][(i)+k+10] = ~Font[i][10];
//        k = (k+1) % 80;
//    }

}


/********************************************************************/
/***  End of LCD Handler                                          ***/
/********************************************************************/


void LCD_Handler()
{
    switch (display_state)
    {
    case 0: // build scene
        buildScene();
        display_state = 1;
        break;

    case 1: // display scene
        if (UCB0IFG & UCTXIFG)
        {
            if (line == 0)
            {
                P2OUT |= 0x10;                  // Enable LCD CS
                UCB0TXBUF = 0x80;
                column = 0;
                line++;
            }
            else if ((line >= 1) && (line <= 96))
            {
                if (column == 0)
                {
                    UCB0TXBUF = reverse(line);
                    column++;
                }
                else if ((column >= 1) && (column <= 12))
                {
                    UCB0TXBUF =  DisplayBuffer[line-1][column-1];
                    column++;
                }
                else
                {
                    UCB0TXBUF = 0x00;
                    column = 0;
                    line++;
                }
            }
            else if (line == 97)
            {
                UCB0TXBUF = 0x00;
                line++;
            }
            else if (line == 98)
            {
                line++;
            }
            else
            {
                if ((UCB0STATW & UCBBUSY) == 0)
                {
                    //Ensure a 2us min delay to meet the LCD's thSCS
                    //__delay_cycles(16);
                    line = 0;
                    P2OUT &= ~0x10;                 // Disable LCD CS
                    display_state = 0;
                }
            }
        }
        break;

    default:
        display_state = 0;
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
//        LCD_Handler();
    }

}






//old print word func

//    for ( l=0; l<12; l++)
//    {
//         switch(word[l]) {
//             case 'a':
//                 for (i=0; i<8; i++)
//                 {
//                    DisplayBuffer[i+height][k] = ~Font[i][10];
//                 }
//                 k++;
//                 break;
//             case 'b':
//                 for (i=0; i<8; i++)
//                 {
//                     DisplayBuffer[i+height][k] = ~Font[i][11];
//                 }
//                 k++;
//                 break;
//             case 'c':
//                 for (i=0; i<8; i++)
//                 {
//                     DisplayBuffer[i+height][k] = ~Font[i][12];
//                 }
//                 k++;
//                 break;
//         }
//    }
