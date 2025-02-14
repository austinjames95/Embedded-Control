////////////////////////////////////////////////////////////////////////
//** ENGR-2350 Activity: Simple GPIO Project 
//** NAME: Austin Schlutter
//** RIN: 662080932

////////////////////////////////////////////////////////////////////////

#include "engr2350_msp432.h"

// Add function prototypes here, as needed.
void GPIOInit();
void getInputs();
void registerLogic();
void driverLibLogic();

// Add global variables here, as needed.
uint8_t push1;
uint8_t push2;
uint8_t slide1;
     // **ACTIVITY**: Add push2 and slide1 variables


int main() {  //// Main Function ////  
    // We always call the sysInit function first to set up the 
    // microcontroller for how we are going to use it.
    sysInit();

    // Place initialization code (or run-once) code here
    GPIOInit();

    while(1) {  
        // Place code that runs continuously in here
        getInputs();


        // Functions to calculate outputs
        registerLogic();
        driverLibLogic();



        //// TEST CODE BELOW ////

        // Test print of inputs
        printf("slide1: %u, push1: %u, push2: %u\r\n",slide1,push1,push2);

        // Code to test the outputs. Remove the /* and */ to use this code.
        /*
        uint32_t count;
        // &= (AND=) sets bits low, |= (OR=) sets bits high,
        // ^= (Exclusive OR=) toggles the value of a bits set high in number
        P6OUT ^= 0x01;  // Replace number to toggle one leg of BiLED1
        P5OUT ^= 0x02;  // Replace number to toggle LED1
        for(count=100000; count>0; count--); // This Creates a crude delay
        P6OUT ^= 0x02;  // Replace number to toggle the other leg of BiLED1
        for(count=100000; count>0; count--); // This Creates a crude delay
        */

        //// END OF TEST CODE ////
    }
}  //// Main Function ////

// Add function declarations here as needed
void GPIOInit( ) {
    // Configure inputs and outputs
    P6DIR |= 0x03; // Set P6.0 and P6.1 to output
    P3DIR &= ~0x20; // Set P3.5 to input
    // **ACTIVITY**: Add initializations for missing inputs and outputs
    P5DIR |= 0x02;
    P5DIR &= ~0xC0;


}

void getInputs( ) {
    // Read the input values
    slide1 = (P3IN & 0x20) != 0;    // Determine slide1 value.
                       // Notice the repeated use of 0x20 from GPIOInit().
    // **ACTIVITY**: Get input state for push1 and push2
    push1 = (P5IN & 0x40) != 0;
    push2 = (P5IN & 0x80) != 0;

}

void registerLogic( ) {
    // **ACTIVITY**: Finish the code in this function (look at comments)
    if( slide1 ){    // Check if slide switch is ON
        if(push1 && push2){ // **ACTIVITY**: Check if Both pushbuttons are pressed (replace 0)
            // Turn BiLED1 OFF by setting both pins to the same value
            P6OUT |= 0x03;  // This sets both 6.0&6.1 to 1 (xxxxxx11)
            // **ACTIVITY**: Turn LED1 ON (add command below)
            P5OUT |= 0x02;

        }else if(push1){ // **ACTIVITY**: Check if pushbutton 1 is pressed (replace 0)
            // Turn BiLED1 to 1 color by setting both pins to the opposite value
           P6OUT |= 0x02;  // Set P6.1 to 1 (xxxxxx1x)
           P6OUT &= ~0x01; // Then set P6.0 to 0 (xxxxxx10)
           // **ACTIVITY**: Turn LED1 OFF (add command below)
           P5OUT &= ~0x02;

        }else if( push2 ){ // Check if pushbutton 2 is pressed
            // **ACTIVITY**: Turn BiLED1 to the other color by... and also turn LED1 ON
            P5OUT |= 0x02;
            P6OUT |= 0x01;
            P6OUT &= ~0x02;

        }else{
            // **ACTIVITY**: Turn everything off
            P6OUT &= ~0x03;
            P5OUT &= ~0x02;

        }
    }else{
        // Turn everything off
        P6OUT &= ~0x03;  // This sets both 6.0&6.1 to 0 (xxxxxx00)
        P5OUT &= ~0x02; // set pin 5.1 to 0 (xxxxxx0x)
    }
}

void driverLibLogic( ) {
    // **ACTIVITY**: Finish the code in this function (look at comments)
    if( slide1 ){    // Check if slide switch is ON
        if(push1 && push2){ // **ACTIVITY**: Check if Both pushbuttons are pressed (replace 0)
            // Turn BiLED1 OFF by setting both pins to the same value
            GPIO_setOutputLowOnPin(GPIO_PORT_P6,GPIO_PIN0|GPIO_PIN1);
            // **ACTIVITY**: Turn LED1 ON (add command below)
            GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN1);


        }else if( push1 ){ // **ACTIVITY**: Check if pushbutton 1 is pressed (replace 0)
            // Turn BiLED1 to 1 color by setting both pins to the opposite value
            GPIO_setOutputHighOnPin(GPIO_PORT_P6,GPIO_PIN1);
            GPIO_setOutputLowOnPin(GPIO_PORT_P6,GPIO_PIN0);
           // **ACTIVITY**: Turn LED1 OFF (add command below)
            P5OUT &= ~0x02;

        }else if( push2 ){ // Check if pushbutton 2 is pressed
            // **ACTIVITY**: Turn BiLED1 to the other color by... and also turn LED1 ON
            P5OUT |= 0x01;
            P6OUT |= 0x01;
            P6OUT &= ~0x02;

        }else{
            // **ACTIVITY**: Turn everything off
            GPIO_setOutputLowOnPin(GPIO_PORT_P6,GPIO_PIN0);
            GPIO_setOutputLowOnPin(GPIO_PORT_P6,GPIO_PIN1);
            GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN1);

        }
    }else{
        // Turn everything off
        GPIO_setOutputLowOnPin(GPIO_PORT_P6,GPIO_PIN0);
        GPIO_setOutputLowOnPin(GPIO_PORT_P6,GPIO_PIN1);
        GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN1);
    }
}

// Add interrupt functions last so they are easy to find
