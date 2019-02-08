#include <msp430.h> 

#define LONG unsigned long
#define WORD unsigned short
#define BYTE unsigned char


void delay(WORD delay_in_ms);



///////////////////////////
// Start of Use Programs //
///////////////////////////


void red_led()
{
    for (;;)
    {
        P4OUT |=  0x40;                 // Set P4.6 on  (Red LED)
        delay(500);

        P4OUT &= ~0x40;                 // Set P4.6 off (Red LED)
        delay(500);
    }
}



void green_led()
{
    for (;;)
    {
        P1OUT |=  0x01;                 // Set P1.0 on  (Green LED)
        delay(500);

        P1OUT &= ~0x01;                 // Set P1.0 off (Green LED)
        delay(500);
    }
}


void idle_process()
{
    for (;;)
    {
    }
}



///////////////////////////
// End  of Use Programs  //
///////////////////////////



#define MAX_PROCESSES   3
#define STACK_SIZE      100

#define UNUSED      0
#define RUNNABLE    1
#define WAITING     2

struct ProcessControlBlock
{
    WORD pcb_status;
    WORD delay_timer;
    LONG sp;
    BYTE stack[STACK_SIZE];
};


struct ProcessControlBlock process[MAX_PROCESSES];

unsigned int current_process = 0;

LONG status;
LONG stack_pointer;
LONG program_counter;
LONG saved_sp;

WORD pc1;
WORD pc2;

void initialise_process(unsigned int process_index, void (*funct)())
{
    if (process_index < MAX_PROCESSES)
    {
        asm(
                " movx.a SR,&status\n"
            );

        status |= GIE;
        stack_pointer = (LONG)&process[process_index] + STACK_SIZE - 2;
        program_counter = (LONG)funct;

        // Construct combined PC+SR used by interrupt

        pc1 = (WORD)program_counter;
        pc2 = (WORD)(((program_counter>>4)&0x0F000) | status&0x00FFF);

        asm(
                " movx.a sp,&saved_sp\n"
                " movx.a &stack_pointer,sp\n"
                " push.w &pc1\n"
                " push.w &pc2\n"
                " push.a #0\n"
                " push.a #0\n"
                " push.a #0\n"
                " push.a #0\n"
                " push.a #0\n"
                " push.a #0\n"
                " push.a #0\n"
                " push.a #0\n"
                " push.a #0\n"
                " push.a #0\n"
                " push.a #0\n"
                " push.a #0\n"
                " push.a #0\n"
                " movx.a sp,&stack_pointer\n"
                " movx.a &saved_sp,sp\n"
            );

        process[process_index].sp = stack_pointer;
        process[process_index].pcb_status = RUNNABLE;
        process[process_index].delay_timer = 0;
    }
}


void run_process(unsigned int process_index)
{
    if (process_index < MAX_PROCESSES)
    {
        stack_pointer = process[process_index].sp;

        asm(
                " movx.a &stack_pointer,SP \n"
                " pop.a R3 \n"
                " pop.a R4 \n"
                " pop.a R5 \n"
                " pop.a R6 \n"
                " pop.a R7 \n"
                " pop.a R8 \n"
                " pop.a R9 \n"
                " pop.a R10 \n"
                " pop.a R11 \n"
                " pop.a R12 \n"
                " pop.a R13 \n"
                " pop.a R14 \n"
                " pop.a R15 \n"
                " RETI \n"
        );
    }
}


void delay(WORD delay_in_ms)
{
    asm(
            " dint \n"
    );

    // Set up the delay

    process[current_process].delay_timer = delay_in_ms;
    process[current_process].pcb_status = WAITING;

    asm(
            " eint \n"
    );

    // Invoke the timer interrupt as it will switch between processes for us

    TA0CTL |= TAIFG;

    // Don't do anything until the interrupt kicks in...

    asm(
            " nop \n"
            " nop \n"
            " nop \n"
            " nop \n"
            " nop \n"
            " nop \n"
            " nop \n"
            " nop \n"
            " nop \n"
            " nop \n"
            " nop \n"
            " nop \n"
            " nop \n"
            " nop \n"
            " nop \n"
            " nop \n"
            " nop \n"
            " nop \n"
            " nop \n"
            " nop \n"
    );
}




/*
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer

    // Initialisation

    // Initialisation - Hardware

    PM5CTL0 &= ~LOCKLPM5;           // Disable the GPIO power-on default high-impedance mode
                                    // to activate previously configured port settings

    P1DIR |=  0x01;                 // Set P1.0 to output direction
    P4DIR |=  0x40;                 // Set P4.6 to output direction
    P1OUT &= ~0x01;                 // Set P1.0 off (Green LED)
    P4OUT &= ~0x40;                 // Set P4.6 off (Red LED)

    //P4OUT |=  0x40;               // Set P4.6 on  (Red LED)

                                    // Timer A0 (1ms interrupt)
    TA0CCR0 =  1024;                // Count up to 1024
    TA0CCTL0 = 0x10;                // Enable counter interrupts, bit 4=1
    TA0CTL =  TASSEL_2 + MC_1;      // Timer A using subsystem master clock, SMCLK(1.1 MHz)
                                    // and count UP to create a 1ms interrupt

    // Initialisation - Software

    initialise_process(0, red_led);
    initialise_process(1, green_led);
    initialise_process(2, idle_process);


    run_process(current_process);


    // Background loop

    _BIS_SR(GIE);                   // interrupts enabled

    for (;;)
    {

    }

    return 0;
}



#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 (void)    // Timer0 A0 1ms interrupt service routine
{
    int i;

    // Save first process details...

    asm(
            " push.a R10\n"
            " push.a R9\n"
            " push.a R8\n"
            " push.a R7\n"
            " push.a R6\n"
            " push.a R5\n"
            " push.a R4\n"
            " push.a R3\n"
            " movx.a sp,&stack_pointer\n"
        );

    process[current_process].sp = stack_pointer;

    // Decrement any wait timers

    for (i=0; i<MAX_PROCESSES; i++)
    {
        if (process[i].pcb_status == WAITING)
        {
            if (process[i].delay_timer > 0)
            {
                process[i].delay_timer--;
            }

            if (process[i].delay_timer == 0)
            {
                process[i].pcb_status = RUNNABLE;
            }
        }
    }

    // Switch processes (highest priority is process 0) - real-time dispatcher

    for (i=0; i<MAX_PROCESSES; i++)
    {
        if (process[i].pcb_status == RUNNABLE)
        {
            current_process = i;
            break;
        }
    }

    stack_pointer = process[current_process].sp;

    asm(
            " movx.a &stack_pointer,SP \n"
            " pop.a R3 \n"
            " pop.a R4 \n"
            " pop.a R5 \n"
            " pop.a R6 \n"
            " pop.a R7 \n"
            " pop.a R8 \n"
            " pop.a R9 \n"
            " pop.a R10 \n"
    );
}




