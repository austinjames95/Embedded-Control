////////////////////////////////////////////////////////////////////////

//** ENGR-2350 Activity: Stopwatch Template Project

//** NAME: Austin Schlutter

//** RIN: 662080932

////////////////////////////////////////////////////////////////////////


#include "engr2350_msp432.h"


// Function Prototypes

void GPIO_Init();

void Timer_Init();

void UpdateTime(uint8_t * time_arry); // Take time array and add 1/100 s to it.

// **ACTIVITY** Add needed function prototypes

void Timer_ISR();

void Port1_ISR();


// Global Variables

Timer_A_UpModeConfig config;

uint8_t total_time[4]; // array to keep track of [tenths of seconds,seconds,minutes,hours]

uint8_t lap_time[4]; // Current lap time (previous lap not stored, only printed once)

uint8_t lap_num = 1; // Current lap number, start on lap 1

uint8_t print_flag = 1; // Flag to note that time should be printed on screen.

uint8_t timerFlag = 0; // 1 = on 0 = off

uint8_t lapFlag = 0; // 1 = off 0 = on

uint8_t i = 0;


                        // We start this at 1 to print time at 0 (instead of leaving blank)

uint8_t counter = 0; // For toggling the LED

// **ACTIVITY** Add remaining variables needed here

uint16_t timer_reset_count = 0;

uint8_t lap_flag = 0;


int main() {    //// Main Function ////

    sysInit();

    GPIO_Init();

    Timer_Init();


    printf("Lap #\tLap Time\tTotal Time \r\n"); // Print header

    while(1){  

        // Place code that runs continuously in here


        // Check if new lap is requested. To allow first compile, you can just put a 0 in here

        if(lapFlag == 1) {

            putchar('\n'); // Make a new line in the printing for next lap

            // **ACTIVITY** Reset flag, reset lap time array and increment lap number

            Timer_A_clearInterruptFlag(TIMER_A2_BASE);

            lap_time[0] = 0;

            lap_time[1] = 0;

            lap_time[2] = 0;

            lap_time[3] = 0;

            lap_num++;

            lapFlag = 0;

        }



        // Check if time to update print. This is complete, no changes needed

        if(print_flag) {

            // This print takes ~65k cycles, or 2.7ms at 24 MHz

            printf("\r%6u\t%02u:%02u:%02u.%02u\t%02u:%02u:%02u.%02u\t",lap_num,lap_time[3],lap_time[2],lap_time[1],lap_time[0]

                                                              ,total_time[3],total_time[2],total_time[1],total_time[0]);

            print_flag = 0;

        }

    }

}    //// Main Function ////  


// Add function declarations here as needed

void GPIO_Init() {

    // **ACTIVITY**

    // Configure pin for P1.0 for LED1 to blink

    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);


    // Configure used GPIO - P1.1,P1.4 for Devboard switches S1 and S2, OR

                       //  - P4.0,P4.2 for RSLK bumpers BMP0 and BMP1

    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1 | GPIO_PIN4);


    GPIO_registerInterrupt(GPIO_PORT_P1, Port1_ISR);


    GPIO_interruptEdgeSelect(GPIO_PORT_P1 ,GPIO_PIN1 | GPIO_PIN4 , GPIO_HIGH_TO_LOW_TRANSITION);


    GPIO_enableInterrupt(GPIO_PORT_P1 ,GPIO_PIN1 | GPIO_PIN4);

    // The pins should be configured as inputs and should trigger

    // the associated interrupt ON PRESS ONLY.


}


void Timer_Init(){

    // **ACTIVITY** Configure a timer to run at 20 Hz and trigger overflow interrupt (most copied from Activity 7)

    config.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;

    config.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_28;

    config.timerPeriod = 42857;


    config.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;

    Timer_A_registerInterrupt(TIMER_A2_BASE,TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT,Timer_ISR);

    Timer_A_configureUpMode(TIMER_A2_BASE, &config);


}


// Use this function to update the time arrays. Note that the argument requested is

// the time array itself (arrays are by definition pointers, no & needed).

void UpdateTime(uint8_t * time_arry){

    time_arry[0]++;  // Increment tenths of seconds

    if(time_arry[0] == 10){  // If a whole second has passed...

        time_arry[0] = 0;    // Reset tenths of seconds

        time_arry[1]++;      // And increment seconds

        if(time_arry[1] == 60){ // If a minute has passed...

            time_arry[1] = 0;   // Reset seconds

            time_arry[2]++;     // Increment minutes

            if(time_arry[2] == 60){  // and so on...

                time_arry[2] = 0;

                time_arry[3]++;

                if(time_arry[3] == 24){

                    time_arry[3] = 0;

                }

            }

        }

    }

}


// **ACTIVITY** Interrupt for the timer

void Timer_ISR()

{

    Timer_A_clearInterruptFlag(TIMER_A2_BASE);


    timer_reset_count++;

    if (timer_reset_count % 2 == 0)

    {

        UpdateTime(total_time);

        UpdateTime(lap_time);

        print_flag = 1;

    }

    if (timer_reset_count == 20)

    {

        timer_reset_count = 0;

        GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);

    }

}


// **ACTIVITY** Interrupt for the switches/bumpers

void Port1_ISR()

{

    __delay_cycles(240e3);

    uint16_t active_pins = GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);

    if(active_pins & GPIO_PIN1)

    {

        GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);

        if(!GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN1))

        {

            if (timerFlag)

            {

                Timer_A_stopTimer(TIMER_A2_BASE);

                timerFlag = 0;


            }

            else

            {

                Timer_A_startCounter(TIMER_A2_BASE, TIMER_A_UP_MODE);

                timerFlag = 1;

            }

        }

    }

    if(active_pins & GPIO_PIN4)

    {

        GPIO_clearInterruptFlag(GPIO_PORT_P1,GPIO_PIN4);

        if(!GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN4))

        {

            if (timerFlag == 0)

            {

                for (i = 0; i < 5; i++)

                {

                    total_time[i] = 0;

                    lap_time[i] = 0;

                }

                print_flag = 1;

            }

            else

            {

                lapFlag = 1;

            }

        }

    }

}
