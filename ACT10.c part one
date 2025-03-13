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
void Timer_ISR();

// Global variables
Timer_A_UpModeConfig timer;
Timer_A_CompareModeConfig compare;

float dutycycle = 0.5;
uint16_t period = 16384;

// Main Function
int main() {
    sysInit();
    GPIOinit();
    timerInit();

    while (1) {
        // Increase blink frequency by 10% when BMP0 (P4.0) is pressed
        if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0))
        {
            __delay_cycles(240e3);
            while (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0));
            __delay_cycles(240e3);

            if ((period - 3276) < 1638)
            {
                period = 1638; // Min limit
            }
            else
            {
                period -= 3276;
            }
            printf("+10 per\r\n");
            printf("%u\r\n", period);
            timerInit();
        }

        // Decrease blink frequency by 10% when BMP1 (P4.2) is pressed
        else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN2))
        {
            __delay_cycles(240e3);
            while (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN2));
            __delay_cycles(240e3);

            if ((period + 3276) > 32768)
            {
                period = 32768; // Max limit
            }
            else
            {
                period += 3276;
            }
            printf("-10 per\r\n");
            printf("%u\r\n", period);
            timerInit();
        }

        // Increase duty cycle by 10% when BMP3 (P4.6) is pressed
        else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN6))
        {
            __delay_cycles(240e3);
            while (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN6));
            __delay_cycles(240e3);

            if ((dutycycle + 0.1) > 1)
            {
                dutycycle = 1;  // Cap at 100%
            }
            else
            {
                dutycycle += 0.1;
            }
            printf("+10 dut\r\n");
            printf("%2f\r\n", dutycycle);
            timerInit();
        }

        // Decrease duty cycle by 10% when BMP4 (P4.7) is pressed
        else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN7))
        {
            __delay_cycles(240e3);
            while (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN7));
            __delay_cycles(240e3);

            if ((dutycycle - 0.1) < 0)
            {
                dutycycle = 0;  // Cap at 0%
            }
            else
            {
                dutycycle -= 0.1;
            }
            printf("-10 dut\r\n");
            printf("%2f\r\n", dutycycle);
            timerInit();
        }
    }
}

// Function to initialize GPIO
void GPIOinit()
{
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0); // LED1 Output
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN0);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN2);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN6);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN7);
}

// Function to initialize and update Timer
void timerInit()
{
    timer.clockSource = TIMER_A_CLOCKSOURCE_ACLK;
    timer.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_2;
    timer.timerPeriod = period;
    timer.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;

    compare.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
    compare.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    compare.compareOutputMode = 0;
    compare.compareValue = roundf(period * dutycycle);

    Timer_A_initCompare(TIMER_A2_BASE, &compare);
    Timer_A_registerInterrupt(TIMER_A2_BASE, TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT, Timer_ISR);
    Timer_A_configureUpMode(TIMER_A2_BASE, &timer);
    Timer_A_startCounter(TIMER_A2_BASE, TIMER_A_UP_MODE);
}

// Timer ISR function
void Timer_ISR()
{
     if(Timer_A_getEnabledInterruptStatus(TIMER_A2_BASE) == TIMER_A_INTERRUPT_PENDING)
     {
         Timer_A_clearInterruptFlag(TIMER_A2_BASE);
         GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
     }
     // next check if a CCR1 event occurred
     else if(TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG)
     {
         Timer_A_clearCaptureCompareInterrupt(TIMER_A2_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_1);
         GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
     }

}

