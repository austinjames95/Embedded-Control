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

void GPIO_Init();

int32_t getchar(void);


// Add global variables here as needed.



int main() {    //// Main Function ////

  

    // Add local variables here as needed.


    // We always call the sysInit function first to set up the 

    // microcontroller for how we are going to use it.

    sysInit();

    GPIO_Init();

    // Place initialization code (or run-once) code here

    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN3 | GPIO_PIN2);

    GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN7 | GPIO_PIN6);


    while(1){  

        // Place code that runs continuously in here

        uint8_t cmd = getchar();

        if(cmd=='f'){ // forward

            printf("letter f is pressed\r\n");

            GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN2);

            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN3);

            GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN7);

            GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN6);

            __delay_cycles(240);

            // Turn the necessary transistors on

        }else if(cmd=='r'){ // reverse

            printf("letter r is pressed\r\n");

            GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN3);

            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN2);

            GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN6);

            GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN7);

            __delay_cycles(240);

            // Turn the necessary transistors on

        }else if(cmd=='s'){ // stop

            printf("letter s is pressed\r\n");

            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN3 | GPIO_PIN2);

            GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN7 | GPIO_PIN6);

        }


    }   

}    //// Main Function ////


// Add function declarations here as needed


void GPIO_Init()

{

    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN2 | GPIO_PIN3);

    GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN7 | GPIO_PIN6);


}


// Add interrupt functions last so they are easy to find
