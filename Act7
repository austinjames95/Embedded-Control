////////////////////////////////////////////////////////////////////////

//** ENGR-2350 Template Project 

//** NAME: Austin Schlutter

//** RIN: 662080932

//** This is the base project for several activities and labs throughout

//** the course.  The outline provided below isn't necessarily *required*

//** by a C program; however, this format is required within ENGR-2350

//** to ease debugging/grading by the staff.

////////////////////////////////////////////////////////////////////////



// We'll always add this include statement. This basically takes the

// code contained within the "engr_2350_msp432.h" file and adds it here.

#include "engr2350_msp432.h"



// Add function prototypes here as needed.

void GPIOInit();

// Add global variables here as needed.



uint8_t minutes = 0;

uint8_t seconds = 0;

uint8_t ovfl = 0;



Timer_A_UpModeConfig timer;



uint8_t updateCountDown();

void GPIO_Init();

void Timer_Init();



 int main() {    //// Main Function ////

  

    // Add local variables here as needed.



    // We always call the sysInit function first to set up the 

    // microcontroller for how we are going to use it.

    sysInit();

    GPIO_Init();

    Timer_Init();



    seconds = 0;

    minutes = 1;



    // Place initialization code (or run-once) code here

    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);



    while(1){  

        // Place code that runs continuously in here



        if (Timer_A_getInterruptStatus(TIMER_A0_BASE))

        {

            Timer_A_clearInterruptFlag(TIMER_A0_BASE);



            GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN4);

            printf("Timer Period Completed\n\r");



            ovfl++;

            if (ovfl == 20)

            {

                ovfl = 0;

                GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);

                if (updateCountDown() == 1)

                {

                    printf("0:0\n\r");

                    Timer_A_stopTimer(TIMER_A0_BASE);

                }

            }

        }



    }   

}    //// Main Function ////  



// Add function declarations here as needed



void Timer_Init()

{

    timer.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;

    timer.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_28;

    //timer.timerPeriod = 42857;

    timer.timerPeriod = 214285;



    Timer_A_configureUpMode(TIMER_A0_BASE, &timer);

}



void GPIO_Init()

{

    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN4);

}



uint8_t updateCountDown()

{

    if (minutes == 0 && seconds == 0)

    {

        return 1;

    }



    if (seconds == 0)

    {

        seconds = 60;

        minutes = minutes - 1;

    }



    seconds = seconds - 1;

    printf("%02u:%02u\n\r", minutes, seconds);

    return 0;

}





// Add interrupt functions last so they are easy to find
