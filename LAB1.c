//////////////////////////
// Lab 1
// ENGR-2350 S25
// Names: Austin
// Section: 02
// Side: A
// Seat: 6
//////////////////////////

#include "engr2350_msp432.h"
#include "lab1lib.h"


void GPIOInit();
void testIO();
void controlSystem();



uint8_t LEDFL = 0; // Two variables to store the state of
uint8_t LEDFR = 0; // the front left/right LEDs (on-car)
uint8_t push1;
uint8_t push2;
uint8_t slide1;
uint8_t buttonOne;
uint8_t buttonTwo;
uint8_t bumperZero;
uint8_t bumperTwo;
uint8_t bumperThree;
uint8_t bumperFive;
uint8_t bumperSix;
uint8_t bumperSeven;

uint8_t isRunning = 1;

int main() {    //// Main Function ////

    sysInit(); // Basic car initialization
    initSequence(); // Initializes the lab1Lib Driver
    GPIOInit();

    printf("\r\n\n"
           "===========\r\n"
           "Lab 1 Begin\r\n"
           "===========\r\n");
    GPIO_setOutputLowOnPin(GPIO_PORT_P8,GPIO_PIN5);
    GPIO_setOutputHighOnPin(GPIO_PORT_P8,GPIO_PIN0);

    while(1){
        //testIO(); Used in Part A to test the IO
        getInputs();
        controlSystem(); // Used in Part B to implement the desired functionality
    }
}    //// Main Function ////


void GPIOInit(){
    // Add initializations of inputs and output
    //Out
    GPIO_setAsInputPin( GPIO_PORT_P5, GPIO_PIN6);
    GPIO_setAsInputPin( GPIO_PORT_P2, GPIO_PIN4 | GPIO_PIN5);

    GPIO_setAsOutputPin( GPIO_PORT_P6, GPIO_PIN0 | GPIO_PIN1);
    GPIO_setAsOutputPin( GPIO_PORT_P8, GPIO_PIN0 | GPIO_PIN5);
    GPIO_setAsOutputPin( GPIO_PORT_P3, GPIO_PIN7 | GPIO_PIN6);
    GPIO_setAsOutputPin( GPIO_PORT_P5, GPIO_PIN4 | GPIO_PIN5);

    GPIO_setAsInputPinWithPullUpResistor( GPIO_PORT_P4, GPIO_PIN7 | GPIO_PIN6 | GPIO_PIN5 | GPIO_PIN2 | GPIO_PIN3 |GPIO_PIN0);
}


void testIO(){
    // Add printf statement(s) for testing inputs
    printf("slide1: %u, push1: %u, push2: %u\r\n", slide1, push1, push2);
    // Example code for testing outputs
    while(1){
        uint8_t cmd = getchar();
        if(cmd == 'a')
        {
            printf("pressing a");
            // Turn LEDL On
            GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN1);
        }
        else if(cmd == 'z')
        {
            printf("pressing z");
            // Turn LEDL Off
            GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN1);
        }
        else if (cmd == 's')
        {
            printf("pressing s");
            // turn LEDR On
            GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN5);
        }
        else if (cmd == 'x')
        {
            printf("pressing x");
            // turn LEDR Off
            GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN5);

        }
        else if (cmd == 'e')
        {
            printf("pressing e");
            // BiLED1 Red
            GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN1);
            GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN0);
        }
        else if (cmd == 'q')
        {
            printf("pressing q");
            // BiLED1 Green
            GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN0);
            GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN1);
        }
        else if (cmd == 'w')
        {
            printf("pressing w");
            // BiLED1 Off
            GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN1 | GPIO_PIN0);

        }
    }
}

void getInputs()
{
    slide1 = GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN6);
    buttonOne = GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN4);
    buttonTwo = GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN5);
    bumperZero = GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0);
    bumperTwo = GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN2);
    bumperThree = GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN3);
    bumperFive = GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN5);
    bumperSix = GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN6);
    bumperSeven = GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN7);
}


void controlSystem()
{
    if (slide1 == 1)
    {
        if (isRunning == 0)
        {
            GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN1);
            GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN0);

            printf("isRunning\n");
            printf("isRunning seq: %u\r\n", runSequence());
            while (statusSequence() != 100)
            {
                printf("Status seq: %u\r\n", statusSequence());
            }
            printf("done\n");
            isRunning = 1;
            GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN0);
            GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN1);
        }
        else
        {
            GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN0);
            GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN1);
        }
    }
    else
    {
        isRunning = 0;
        GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN1);
        GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN0);
        __delay_cycles(480e3);

        if (bumperZero == 0)
        {
            recordSegment(-2);
            GPIO_toggleOutputOnPin(GPIO_PORT_P8, GPIO_PIN5 | GPIO_PIN0);
            printf("Drive and turn left 90 degrees\r\n");
            __delay_cycles(480e3);
            while (bumperZero == 0)
            {
                bumperZero = GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0);
            }
            __delay_cycles(480e3);
        }
        else if (bumperTwo == 0)
        {
            recordSegment(-1);
            GPIO_toggleOutputOnPin(GPIO_PORT_P8, GPIO_PIN5 | GPIO_PIN0);
            printf("Drive and turn left 45 degrees\r\n");
            __delay_cycles(480e3);
            while (bumperTwo == 0)
            {
                bumperTwo = GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN2);
            }
            __delay_cycles(480e3);
        }
        else if (bumperThree == 0)
        {
            recordSegment(0);
            GPIO_toggleOutputOnPin(GPIO_PORT_P8, GPIO_PIN5 | GPIO_PIN0);
            printf("Drive forward\r\n");
            __delay_cycles(480e3);
            while (bumperThree == 0)
            {
                bumperThree = GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN3);
            }
            __delay_cycles(480e3);
        }
        else if (bumperFive == 0)
        {
            recordSegment(1);
            GPIO_toggleOutputOnPin(GPIO_PORT_P8, GPIO_PIN5 | GPIO_PIN0);
            printf("Drive and turn right 45 degrees\r\n");
            __delay_cycles(480e3);
            while (bumperFive == 0)
            {
                bumperFive = GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN5);
            }
            __delay_cycles(480e3);
        }
        else if (bumperSix == 0)
        {
            recordSegment(2);
            GPIO_toggleOutputOnPin(GPIO_PORT_P8, GPIO_PIN5 | GPIO_PIN0);
            printf("Drive and turn right 90 degrees\r\n");
            __delay_cycles(480e3);
            while (bumperSix == 0)
            {
                bumperSix = GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN6);
            }
            __delay_cycles(480e3);
        }
        else if (bumperSeven == 0)
        {
            recordSegment(127);
            GPIO_toggleOutputOnPin(GPIO_PORT_P8 , GPIO_PIN5 | GPIO_PIN0);
            printf("stop for 1s\r\n");
            __delay_cycles(480e3);
            while (bumperSeven == 0)
            {
                bumperSeven = GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN7);
            }
            __delay_cycles(480e3);
        }
        else if (buttonOne == 1)
        {
            popSegment();
            printf("Delete last seg\r\n");
            while (buttonOne == 1)
            {
                buttonOne = GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN4);
            }
            __delay_cycles(480e3);
        }
        else if (buttonTwo == 1)
        {
            clearSequence();
            printf("clear sequence\r\n");
            while (buttonTwo == 1)
            {
                buttonTwo = GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN5);
            }
            __delay_cycles(480e3);

        }

    }
}


