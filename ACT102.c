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
#include <math.h>

// Function prototypes
void GPIOinit();
void timerInit();
//void Timer_ISR();

// Global variables
Timer_A_UpModeConfig config;
Timer_A_CompareModeConfig compare3, compare4;

float dutyCycle1 = 0.1;
float dutyCycle2 = 0.1;

float incrementValue;

uint16_t timerPeriod = 480;

// Main Function
int main() {
    sysInit();
    GPIOinit();
    timerInit();

    incrementValue = timerPeriod / 10.0;


    while (1) {
        if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0))
        {
            __delay_cycles(240e3);
            while (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0));
            __delay_cycles(240e3);

            dutyCycle1 += incrementValue / timerPeriod;
            if (dutyCycle1 > 0.9) dutyCycle1 = 0.9;
            Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3, roundf(timerPeriod * dutyCycle1));
            printf("+10% Left Speed\r\n");
            timerInit();
        }

        else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN2))
        {
            __delay_cycles(240e3);
            while (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN2));
            __delay_cycles(240e3);

            dutyCycle1 -= incrementValue / timerPeriod;
            if (dutyCycle1 < 0.1) dutyCycle1 = 0.1;  // Min 10%
            Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3, roundf(timerPeriod * dutyCycle1));
            printf("-10% Left Speed\r\n");
        }

        // Reverse Left Wheel Direction (BMP2 - P4.3)
        else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN3))
        {
            __delay_cycles(240e3);
            while (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN3));
            __delay_cycles(240e3);

            GPIO_toggleOutputOnPin(GPIO_PORT_P3, GPIO_PIN7);
            printf("Reverse Left Wheel\r\n");
        }

        // Increase Right Wheel Speed (BMP3 - P4.5)
        else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN5))
        {
            __delay_cycles(240e3);
            while (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN5));
            __delay_cycles(240e3);

            dutyCycle2 += incrementValue / timerPeriod;
            if (dutyCycle2 > 0.9) dutyCycle2 = 0.9;  // Max 90%
            Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4, roundf(timerPeriod * dutyCycle2));
            printf("+10% Right Speed\r\n");
        }

        // Decrease Right Wheel Speed (BMP4 - P4.6)
        else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN6))
        {
            __delay_cycles(240e3);
            while (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN6));
            __delay_cycles(240e3);

            dutyCycle2 -= incrementValue / timerPeriod;
            if (dutyCycle2 < 0.1) dutyCycle2 = 0.1;  // Min 10%
            Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4, roundf(timerPeriod * dutyCycle2));
            printf("-10% Right Speed\r\n");
        }

        // Reverse Right Wheel Direction (BMP5 - P4.7)
        else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN7))
        {
            __delay_cycles(240e3);
            while (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN7));
            __delay_cycles(240e3);

            GPIO_toggleOutputOnPin(GPIO_PORT_P3, GPIO_PIN6);
            printf("Reverse Right Wheel\r\n");
        }
    }
}
// Function to initialize GPIO
void GPIOinit()
{
    //GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0); // LED1 Output
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN4);
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN5);

    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN4);
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN5);

    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION); // right motor speed
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION); // left motor speed

    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN0);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN2);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN6);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN7);
}

// Function to initialize and update Timer
void timerInit()
{
    config.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    config.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    config.timerPeriod = timerPeriod;
    config.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;

    // Configure PWM for Motor 1 (CCR3)
    compare3.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_3;
    compare3.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE;
    compare3.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    compare3.compareValue = 0;
    Timer_A_initCompare(TIMER_A0_BASE, &compare3);

    // Configure PWM for Motor 2 (CCR4)
    compare4.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_4;
    compare4.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE;
    compare4.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    compare4.compareValue = 0;
    Timer_A_initCompare(TIMER_A0_BASE, &compare4);

    //Timer_A_registerInterrupt(TIMER_A0_BASE, TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT, Timer_ISR);
    Timer_A_configureUpMode(TIMER_A0_BASE, &config);
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
}



