#include <msp430.h> 


#define WORD unsigned short


int heartbeat = 1;
int counter_button_handler = 0;
#define INTERVAL_BUTTON_HANDLER  10

#define FALSE 0
#define TRUE -1

#define EVENT_NULL          0
#define EVENT_BUTTON_PRESS  1
#define EVENT_FIRE          2
#define EVENT_LEFT          3
#define EVENT_FORWARD       4
#define EVENT_RIGHT         5




/********************************************************************/
/***  Event Queue                                                 ***/
/********************************************************************/

struct Event
{
    char event;
    char param1;
    char param2;
    char param3;
};

#define SIZE_OF_EVENT_Q   6

struct EventQueue
{
    int size;
    int head;
    int tail;
    struct Event queue[SIZE_OF_EVENT_Q];
};

void init_q(struct EventQueue *q)
{
    q->head = 0;
    q->tail = 0;
    q->size = 0;
}

int read_q(struct EventQueue *q, struct Event *e)
{
    int rc;
    int h;
    int s;

    rc = FALSE;

    s = q->size;

    if (s > 0)
    {
        h = q->head;
        e->event  = q->queue[h].event;
        e->param1 = q->queue[h].param1;
        e->param2 = q->queue[h].param2;
        e->param3 = q->queue[h].param3;
        h = (h + 1) % SIZE_OF_EVENT_Q;
        s--;
        q->head = h;
        q->size = s;
        rc = TRUE;
    }

    return rc;
}

int write_q(struct EventQueue *q, struct Event e)
{
    int rc;
    int t;
    int s;

    rc = FALSE;

    s = q->size;

    if (s < SIZE_OF_EVENT_Q)
    {
        t = q->tail;
        q->queue[t].event  = e.event;
        q->queue[t].param1 = e.param1;
        q->queue[t].param2 = e.param2;
        q->queue[t].param3 = e.param3;
        t = (t + 1) % SIZE_OF_EVENT_Q;
        s++;
        q->tail = t;
        q->size = s;
        rc = TRUE;
    }

    return rc;
}



/********************************************************************/
/***  End of Event Queue                                          ***/
/********************************************************************/

struct EventQueue led_q;
struct EventQueue tank_a_q;
struct EventQueue tank_b_q;







/********************************************************************/
/***  Button Handler                                              ***/
/********************************************************************/

void enter_button_state(int new_state);

#define BUTTON_NULL     0
#define BUTTON_RELEASED 1
#define BUTTON_PRESSED  2
#define BUTTON_HELD     3

#define MINIMUM_PRESS_TIME        70
#define BUTTON_HELD_TIME         200

int button_state;
int press_time;
struct Event e;


void init_button_handler()
{
    button_state = BUTTON_RELEASED;
    press_time = 0;
}



/*** BUTTON_RELEASED *********************/

void button_release_enter()
{

}

void button_release_timer()
{
    if ((P1IN & 0x02) == 0)
    {
        // Pressed

        enter_button_state(BUTTON_PRESSED);
    }
}



/*** BUTTON_PRESSED  *********************/

void button_pressed_enter()
{
    press_time = 0;
}

void button_pressed_timer()
{
    press_time++;
    if ((P1IN & 0x02) != 0)
    {
        // Released

        if (press_time >= (MINIMUM_PRESS_TIME/INTERVAL_BUTTON_HANDLER))
        {
            e.event = EVENT_BUTTON_PRESS;
            if (!write_q(&tank_b_q, e))
            {

            }
        }
        enter_button_state(BUTTON_RELEASED);
    }
    else
    {
        // Still pressed

        if (press_time >= (BUTTON_HELD_TIME/INTERVAL_BUTTON_HANDLER))
        {
            enter_button_state(BUTTON_HELD);
        }
    }
}



/*** BUTTON_HELD     *********************/

void button_held_enter()
{
    press_time = 0;
    e.event = EVENT_BUTTON_PRESS;
    if (!write_q(&tank_b_q, e))
    {

    }
}

void button_held_timer()
{
    press_time++;
    if ((P1IN & 0x02) != 0)
    {
        // Released

        enter_button_state(BUTTON_RELEASED);
    }
    else
    {
        // Still pressed

        if (press_time >= (BUTTON_HELD_TIME/INTERVAL_BUTTON_HANDLER))
        {
            enter_button_state(BUTTON_HELD);
        }
    }
}


/***  Button entry code   *******************************************/

void enter_button_state(int new_state)
{
    switch (new_state)
    {
    case BUTTON_NULL:
        break;

    case BUTTON_RELEASED:
        button_release_enter();
        break;

    case BUTTON_PRESSED:
        button_pressed_enter();
        break;

    case BUTTON_HELD:
        button_held_enter();
        break;

    default:
        break;
    }

    button_state = new_state;
}



/***  Button timer code   *******************************************/

void button_timer()
{
    switch (button_state)
    {
    case BUTTON_NULL:
        break;

    case BUTTON_RELEASED:
        button_release_timer();
        break;

    case BUTTON_PRESSED:
        button_pressed_timer();
        break;

    case BUTTON_HELD:
        button_held_timer();
        break;

    default:
        break;
    }
}

/********************************************************************/
/***  End of Button Handler                                       ***/
/********************************************************************/






/********************************************************************/
/***  LED Handler                                                 ***/
/********************************************************************/


#define LED_OFF  0
#define LED_ON   1
#define LED_PERIOD 50

int led_state  = LED_OFF;
int led_period = 0;
struct Event e;

void led_handler()
{
    led_period++;
    if (led_period >= LED_PERIOD)
    {
        led_period = 0;
        switch (led_state)
        {
        case LED_OFF:
            if (read_q(&led_q, &e))
            {
                if (e.event == EVENT_BUTTON_PRESS)
                {
                    P4OUT |= 0x40;          // Set P4.6 on using OR
                    led_state = LED_ON;
                }
            }
            break;

        case LED_ON:
            P4OUT &= (0xFF-0x40);   // Set P4.6 off using AND
            led_state = LED_OFF;
            break;

        default:
            break;
        }
    }
}

/********************************************************************/
/***  End of LED Handler                                          ***/
/********************************************************************/





/********************************************************************/
/***  Tank Handler                                                ***/
/********************************************************************/

#define INITIAL_AMMO    20

#define TANK_UP     0
#define TANK_LEFT   1
#define TANK_DOWN   2
#define TANK_RIGHT  3
#define TANK_NONE   4



struct Tank
{
    int ammo;
    int direction;
    int x;
    int y;
};

struct Tank tank_a;
struct Tank tank_b;

int new_direction[4][3] =
{
// event =  left      forward    right        // Current direction
        {TANK_LEFT,  TANK_UP,   TANK_RIGHT},  // Up
        {TANK_DOWN,  TANK_LEFT, TANK_UP   },  // Left
        {TANK_RIGHT, TANK_DOWN, TANK_LEFT },  // Down
        {TANK_UP,    TANK_RIGHT,TANK_DOWN }   // Right
};



void init_tank(struct Tank* tank, int direction, int x, int y)
{
    tank->ammo = INITIAL_AMMO;
    tank->direction = direction;
    tank->x = x;
    tank->y = y;
}


void tank_handler(struct Tank* tank, struct EventQueue* tank_q)
{
    struct Event e;

    if (read_q(tank_q, &e))
    {
        switch (e.event)
        {
        case EVENT_BUTTON_PRESS:
            if (tank->ammo > 0)  // If we have ammo pass the event on
            {
                if (!write_q(&led_q, e))
                {

                }
                tank->ammo--;
            }
            break;

        case EVENT_LEFT:
            tank->direction = new_direction[tank->direction][0];
            break;

        case EVENT_FORWARD:
            if (tank->direction == TANK_UP)
            {
                tank->y--;
            }
            else if (tank->direction == TANK_LEFT)
            {
                tank->x--;
            }
            else if (tank->direction == TANK_DOWN)
            {
                tank->y++;
            }
            else if (tank->direction == TANK_RIGHT)
            {
                tank->x++;
            }
            if (tank->x < 4 ) tank->x = 4;
            if (tank->x > 92) tank->x = 92;
            if (tank->y < 4 ) tank->y = 4;
            if (tank->y > 92) tank->y = 92;
            break;

        case EVENT_RIGHT:
            tank->direction = new_direction[tank->direction][2];
            break;

        default:
            break;
        }
    }
}

/********************************************************************/
/***  End of Tank Handler                                         ***/
/********************************************************************/




/********************************************************************/
/***  Bullet Handler                                              ***/
/********************************************************************/

#define MAX_BULLETS 30

struct Bullet
{
    char x;
    char y;
    char direction;
    signed char next_bullet;
};

struct Bullet bullets[MAX_BULLETS];

int free_bullets;
int used_bullets;

void init_bullets()
{
    int i;

    free_bullets = 0;
    used_bullets = -1;

    for (i=0; i<MAX_BULLETS-1; i++)
    {
        bullets[i].next_bullet = i+1;
    }

    bullets[MAX_BULLETS-1].next_bullet = -1;
}

void bullet_handler()
{

}

/********************************************************************/
/***  End of Bullet Handler                                       ***/
/********************************************************************/





/********************************************************************/
/***  Slider Handler                                              ***/
/********************************************************************/

#define SLIDER_NULL             0
#define SLIDER_READ_UPPER_LEFT  1
#define SLIDER_READ_UPPER_RIGHT 2
#define SLIDER_READ_LOWER_LEFT  3
#define SLIDER_READ_LOWER_RIGHT 4

#define SLIDER_TURN_THRESHOLD       100
#define SLIDER_FORWARD_THRESHOLD    10

int slider_state = SLIDER_READ_UPPER_LEFT;

int slider_reading     = 0;
int slider_upper_left  = 0;
int slider_upper_right = 0;
int slider_lower_left  = 0;
int slider_lower_right = 0;

int slider_left_left     = 0;
int slider_left_forward  = 0;
int slider_left_right    = 0;
int slider_right_left    = 0;
int slider_right_forward = 0;
int slider_right_right   = 0;

WORD dummy;


void slider_handler()
{
    struct Event e;

    //if ((TA2CCTL1&CCI) == CCI)
    {
        dummy  = TA2CCR1;

        TA2CCTL1 |= COV;
        TA2CTL &= ~TAIFG;                           // Clear interrupt flag
        TA2CTL   = ID_3 + TBSSEL_2 + MC_2;          // Timer A2 using subsystem master clock, SMCLK(1.1 MHz)
        TA2CCTL1 = CM_1+CCIS_0+CAP+SCS;             // Capture initialised

        switch (slider_state)
        {
        case SLIDER_READ_UPPER_LEFT:
            P1OUT |= BIT5;
            P1OUT |= BIT4;
            P1OUT |= BIT3;
            P3OUT &= ~BIT4;
            P3OUT |= BIT5;
            P3OUT |= BIT6;
            CAPTIO0CTL = CAPTIOEN + (3<<4) + (4<<1);    // set up for Pin 3.4
            if ((CAPTIO0CTL&CAPTIO) == 0)
            {
                slider_upper_left = 1;  // finger touching upper left pad
            }
            else
            {
                slider_upper_left = 0;
            }
            slider_state = SLIDER_READ_UPPER_RIGHT;
            break;

        case SLIDER_READ_UPPER_RIGHT:
            //P1OUT &= ~BIT5;
            P1OUT |= BIT4;
            P1OUT |= BIT3;
            P3OUT |= BIT4;
            P3OUT |= BIT5;
            P3OUT |= BIT6;
            CAPTIO0CTL = CAPTIOEN + (1<<4) + (5<<1);    // set up for Pin 1.5
            if ((CAPTIO0CTL&CAPTIO) == 0)
            {
                slider_upper_right = 1; // finger touching upper right pad
            }
            else
            {
                slider_upper_right = 0;
            }
            slider_state = SLIDER_READ_LOWER_LEFT;
            break;

        case SLIDER_READ_LOWER_LEFT:
            P1OUT |= BIT5;
            P1OUT |= BIT4;
            P1OUT |= BIT3;
            P3OUT |= BIT4;
            //P3OUT &= ~BIT5;
            P3OUT |= BIT6;
            CAPTIO0CTL = CAPTIOEN + (3<<4) + (5<<1);    // set up for Pin 3.5
            if ((CAPTIO0CTL&CAPTIO) == 0)
            {
                slider_lower_left = 1;  // finger touching lower left pad
            }
            else
            {
                slider_lower_left = 0;
            }
            slider_state = SLIDER_READ_LOWER_RIGHT;
            break;

        case SLIDER_READ_LOWER_RIGHT:
            P1OUT |= BIT5;
            P1OUT &= ~BIT4;
            P1OUT |= BIT3;
            P3OUT |= BIT4;
            P3OUT |= BIT5;
            P3OUT |= BIT6;
            CAPTIO0CTL = CAPTIOEN + (1<<4) + (4<<1);    // set up for Pin 1.4
            if ((CAPTIO0CTL&CAPTIO) == 0)
            {
                slider_lower_right = 1; // finger touching lower right pad
            }
            else
            {
                slider_lower_right = 0;
            }
            slider_state = SLIDER_READ_UPPER_LEFT;
            break;

        default:
            slider_state = SLIDER_READ_UPPER_LEFT;
            break;
        }

        // Debounce Tank A slider control

        if ((slider_upper_left == 1) && (slider_lower_left == 0))
        {
            slider_left_left++;
            slider_left_forward = 0;
            slider_left_right   = 0;

            if (slider_left_left > SLIDER_TURN_THRESHOLD)
            {
                e.event = EVENT_LEFT;
                write_q(&tank_a_q, e);
                slider_left_left = 0;
            }
        }
        else if ((slider_upper_left == 1) && (slider_lower_left == 1))
        {
            slider_left_left  = 0;
            slider_left_forward++;
            slider_left_right = 0;

            if (slider_left_forward > SLIDER_FORWARD_THRESHOLD)
            {
                e.event = EVENT_FORWARD;
                write_q(&tank_a_q, e);
                slider_left_forward = 0;
            }
        }
        else if ((slider_upper_left == 0) && (slider_lower_left == 1))
        {
            slider_left_left    = 0;
            slider_left_forward = 0;
            slider_left_right++;

            if (slider_left_right > SLIDER_TURN_THRESHOLD)
            {
                e.event = EVENT_RIGHT;
                write_q(&tank_a_q, e);
                slider_left_right = 0;
            }
        }
        else
        {
            slider_left_left     = 0;
            slider_left_forward  = 0;
            slider_left_right    = 0;
        }

        // Debounce Tank B slider control

        if ((slider_upper_right == 1) && (slider_lower_right == 0))
        {
            slider_right_left++;
            slider_right_forward = 0;
            slider_right_right   = 0;

            if (slider_right_left > SLIDER_TURN_THRESHOLD)
            {
                e.event = EVENT_RIGHT;
                write_q(&tank_b_q, e);
                slider_right_left = 0;
            }
        }
        else if ((slider_upper_right == 1) && (slider_lower_right == 1))
        {
            slider_right_left  = 0;
            slider_right_forward++;
            slider_right_right = 0;

            if (slider_right_forward > SLIDER_FORWARD_THRESHOLD)
            {
                e.event = EVENT_FORWARD;
                write_q(&tank_b_q, e);
                slider_right_forward = 0;
            }
        }
        else if ((slider_upper_right == 0) && (slider_lower_right == 1))
        {
            slider_right_left    = 0;
            slider_right_forward = 0;
            slider_right_right++;

            if (slider_right_right > SLIDER_TURN_THRESHOLD)
            {
                e.event = EVENT_LEFT;
                write_q(&tank_b_q, e);
                slider_right_right = 0;
            }
        }
        else
        {
            slider_right_left     = 0;
            slider_right_forward  = 0;
            slider_right_right    = 0;
        }
    }

}




/********************************************************************/
/***  End of Slider Handler                                       ***/
/********************************************************************/







/********************************************************************/
/***  LCD Handler                                                 ***/
/********************************************************************/


const int Font[8][16] =
{
        //  0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x28, 0x08, 0x18, 0x3C, 0x10, 0x3C, 0x08, 0x3E, 0x3C, 0x3C, 0x18, 0x38, 0x38, 0x30, 0x3C, 0x3C},
        {0x44, 0x18, 0x24, 0x04, 0x10, 0x20, 0x10, 0x02, 0x24, 0x44, 0x24, 0x24, 0x44, 0x28, 0x20, 0x20},
        {0x44, 0x08, 0x08, 0x08, 0x20, 0x38, 0x20, 0x04, 0x18, 0x44, 0x24, 0x24, 0x40, 0x24, 0x38, 0x3C},
        {0x44, 0x08, 0x10, 0x18, 0x24, 0x04, 0x3C, 0x08, 0x24, 0x3C, 0x3C, 0x38, 0x40, 0x24, 0x20, 0x20},
        {0x44, 0x08, 0x10, 0x04, 0x3E, 0x04, 0x24, 0x10, 0x24, 0x04, 0x24, 0x24, 0x44, 0x24, 0x20, 0x20},
        {0x28, 0x3C, 0x3C, 0x38, 0x04, 0x38, 0x18, 0x10, 0x3C, 0x04, 0x24, 0x38, 0x38, 0x38, 0x3C, 0x20},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};


const int Tank[8][5] =
{
        // up  left  down  right none
        {0x49, 0xFF, 0x82, 0x00, 0x00},
        {0x49, 0x1C, 0x82, 0xFF, 0x00},
        {0x49, 0x1C, 0xFE, 0x38, 0x00},
        {0x7F, 0xFC, 0xFE, 0x38, 0x00},
        {0x7F, 0x1C, 0xFE, 0x3F, 0x00},
        {0x7F, 0x1C, 0x92, 0x38, 0x00},
        {0x41, 0xFF, 0x92, 0x38, 0x00},
        {0x41, 0x00, 0x92, 0xFF, 0x00}
};


char DisplayBuffer[96][96/8];

char row_modified[96];
char row_previously_modified[96];


int reverse(int inint)
{
    int outint;

    outint = 0;

    if ((inint & BIT0) > 0) outint |= BIT7;
    if ((inint & BIT1) > 0) outint |= BIT6;
    if ((inint & BIT2) > 0) outint |= BIT5;
    if ((inint & BIT3) > 0) outint |= BIT4;
    if ((inint & BIT4) > 0) outint |= BIT3;
    if ((inint & BIT5) > 0) outint |= BIT2;
    if ((inint & BIT6) > 0) outint |= BIT1;
    if ((inint & BIT7) > 0) outint |= BIT0;

    return outint;
}


void initDisplayWriting()
{
    int i;

    for (i=0; i<96; i++)
    {
        row_modified[i] = 1;
    }
}


void initDisplayBuffer(int setting)
{
    int i;
    int j;

    for (i=0; i<96; i++)
    {
        for(j=0; j<12; j++)
        {
            DisplayBuffer[i][j] = setting;
        }

        row_previously_modified[i] = row_modified[i];
        row_modified[i] = 0;
    }
}


void outputDisplayBuffer()
{
    int line;
    int column;
    int command = 0x80;             // Write lines

    command = command ^ 0x40;       // VCOM bit

    P2OUT |= 0x10;                  // LCD CS high

    while (!(UCB0IFG & UCTXIFG));
    UCB0TXBUF = command;

    for (line=0; line<96; line++)
    {
        if ((row_modified[line] | row_previously_modified[line]) != 0)
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
    }

    while (!(UCB0IFG & UCTXIFG));
    UCB0TXBUF = 0x00;               // End of block signal

    while ((UCB0STATW & UCBBUSY));

    __delay_cycles(8);              //Ensure a 2us min delay to meet the LCD's thSCS

    P2OUT &= ~0x10;                 // LCD CS low
}


void outputHex(int row, int column, WORD number)
{
    int i;

    for (i=0; i<8; i++)
    {
        DisplayBuffer[i+row][column  ] ^= Font[i][(number>>12) & 0x0F];
        DisplayBuffer[i+row][column+1] ^= Font[i][(number>>8 ) & 0x0F];
        DisplayBuffer[i+row][column+2] ^= Font[i][(number>>4 ) & 0x0F];
        DisplayBuffer[i+row][column+3] ^= Font[i][(number    ) & 0x0F];
    }
}



void displayTank(struct Tank* tank)
{
    int row;
    int column;
    int direction;
    int shift;
    int i;

    row = tank->y - 3;              // adjust for tank center
    column = (tank->x - 3) / 8;     // adjust for tank center and adjust for bytes
    shift  = (tank->x - 3) % 8;
    direction = tank->direction;

    for (i=0; i<8; i++)
    {
        if (((i+row) >= 0) && ((i+row) < 96) && (column >= 0) && (column < 12))
        {
            DisplayBuffer[i+row][column] ^= (Tank[i][direction] >> shift);

            if (((column+1) >= 0) && ((column+1) < 12) && (shift > 0))
            {
                DisplayBuffer[i+row][column+1] ^= (Tank[i][direction] << (8-shift));
            }

            row_modified[i+row] = 1;
        }
    }

}


void displayBullets()
{
    int bullet;
    int row;
    int column;
    int shift;

    bullet = used_bullets;

    while (bullet >= 0)
    {
        column = bullets[bullet].x / 8;
        shift  = bullets[bullet].x % 8;
        row    = bullets[bullet].y;
        if ((row >= 0) && (row < 96) && (column >= 0) && (column < 12))
        {
            DisplayBuffer[row][column] ^= (0x80 >> shift);
            row_modified [row] = 1;
        }
        bullet = bullets[bullet].next_bullet;
    }
}



int k = 0;

void buildScene()
{
    initDisplayBuffer(0xFF);
/*
    outputHex(30, 1, slider_upper_left );
    outputHex(40, 1, slider_lower_left );
    outputHex(30, 7, slider_upper_right);
    outputHex(40, 7, slider_lower_right);
*/
    displayTank(&tank_a);
    displayTank(&tank_b);

    displayBullets();
}



int display_state = 0;
int line;
int column;

void lcd_handler()
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
                if ((row_modified[line-1] | row_previously_modified[line-1]) != 0)
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
                else
                {
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



/********************************************************************/
/***  End of LCD Handler                                          ***/
/********************************************************************/






/*
 * main.c
 */
int main(void)
{

    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer

    PM5CTL0 &= ~LOCKLPM5;           // Disable the GPIO power-on default high-impedance mode
                                    // to activate previously configured port settings

    // Initialisation

    // Initialisation - POST


    // Initialisation - hardware


    P1DIR  |= 0x01;                 // Set P1.0 to output direction
    P1DIR  &= ~0x02;                // Set P1.1 to input direction (switch 2)
    P1REN  |= 0x02;                 // Set P1.1 pull-up resistor enabled
    P1OUT  |= 0x02;                 // Set P1.1 kick to start input
    P4DIR  |= 0x40;                 // Set P4.6 to output direction


    // Timer A0 (1ms interrupt)
    TA0CCR0 =  1024;                // Count up to 1024
    TA0CCTL0 = 0x10;                // Enable counter interrupts, bit 4=1
    TA0CTL =  TASSEL_2 + MC_1;      // Timer A using subsystem master clock, SMCLK(1.1 MHz)
                                    // and count UP to create a 1ms interrupt



     // Timer A2 used for capture
    TA2CTL |= TACLR;
    TA2CTL   = ID_3 + TBSSEL_2 + MC_2;  // Timer A2 using subsystem master clock, SMCLK(1.1 MHz)
    TA2CCTL1 = CM_1+CCIS_0+CAP+SCS;     // Capture set up for Timer A2



    // Capacitive Touch
    CAPTIO0CTL = CAPTIOEN + (3<<4) + (4<<1);

    P1REN  |= BIT3;
    P1DIR  |= BIT3;
    P1OUT  &= ~BIT3;
    P1REN  |= BIT4;
    P1DIR  |= BIT4;
    P1OUT  &= ~BIT4;
    P1REN  |= BIT5;
    P1DIR  |= BIT5;
    P1OUT  &= ~BIT5;

    P3REN  |= BIT4;
    P3DIR  |= BIT4;
    P3OUT  &= ~BIT4;
    P3REN  |= BIT5;
    P3DIR  |= BIT5;
    P3OUT  &= ~BIT5;
    P3REN  |= BIT6;
    P3DIR  |= BIT6;
    P3OUT  &= ~BIT6;


    // LCD
    P4DIR  |= 0x04;                 // Set P4.2 to output direction (LCD Power On)
    P4DIR  |= 0x08;                 // Set P4.3 to output direction (LCD Enable)

    // SPI Ports
    P1SEL0 &= ~0x40;                // Set P1.6 to output direction (SPI MOSI)
    P1SEL1 |= 0x40;                 // Set P1.6 to output direction (SPI MOSI)
    P1DIR  |= 0x40;                 // Set P1.6 to output direction (SPI MOSI)
    P2SEL0 &= ~0x04;                // Set P2.2 to SPI mode (SPI CLK)
    P2SEL1 |= 0x04;                 // Set P2.2 to SPI mode (SPI CLK)
    P2DIR  |= 0x04;                 // Set P2.2 to output direction (SPI CLK)
    P2DIR  |= 0x10;                 // Set P2.4 to output direction (SPI CS)

    // SPI Inteface
    UCB0CTLW0 |= UCSWRST;
    UCB0CTLW0 &= ~(UCCKPH + UCCKPL + UC7BIT + UCMSB);
    UCB0CTLW0 &= ~(UCSSEL_3);
    UCB0CTLW0 |= UCSSEL__SMCLK;
    UCB0BRW    = 8;
    UCB0CTLW0 |= (UCMSB + UCCKPH + 0x00 + UCMST + UCSYNC + UCMODE_0);
    UCB0CTLW0 &= ~(UCSWRST);

    P4OUT |= 0x04;                  // Turn LCD Power On
    P4OUT |= 0x08;                  // Enable LCD
    P1OUT &= ~0x01;                 // Set P1.0 off (Green LED)
    P4OUT &= ~0x40;                 // Set P4.6 off (Red LED)

    // Initialisation - software

    init_button_handler();
    init_q(&led_q);
    init_q(&tank_a_q);
    init_q(&tank_b_q);
    init_tank(&tank_a, TANK_RIGHT,  4, 45);
    init_tank(&tank_b, TANK_LEFT,  92, 55);
    initDisplayWriting();
    init_bullets();

    _BIS_SR(GIE);                   // interrupts enabled

    //buildScene();


    // Background Loop

    for (;;)
    {
        tank_handler(&tank_a, &tank_a_q);
        tank_handler(&tank_b, &tank_b_q);

        lcd_handler();
    }

    return 0;
}



#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 (void)    // Timer0 A0 1ms interrupt service routine
{
    counter_button_handler++;
    if (counter_button_handler >= INTERVAL_BUTTON_HANDLER)
    {
        counter_button_handler = 0;
        button_timer();
        led_handler();
        slider_handler();
        bullet_handler();
    }

    // Toggle the red LED 500ms on and 500ms off

    heartbeat++;

    if (heartbeat >= 500)
    {
            heartbeat = 1;
            P1OUT ^= 0x01;        // Toggle P1.0 using exclusive-OR
    }
}


