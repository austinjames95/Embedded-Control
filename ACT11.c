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
void timerInit();
void encoderISR();

// Add global variables here as needed.
Timer_A_ContinuousModeConfig config;
Timer_A_CaptureModeConfig capture;

uint32_t enc_total;
int32_t enc_counts_track;
int32_t enc_counts;
uint8_t enc_flag;

sint main() {    //// Main Function ////
  
    // Add local variables here as needed.

    // We always call the sysInit function first to set up the 
    // microcontroller for how we are going to use it.
    sysInit();
    GPIOInit();
    timerInit();


    // Place initialization code (or run-once) code here
    printf("\r\nDistance\tEnc_Counts\tDelta T\t\tAng.Speed\tLin.Speed\r\n"); // Column headers
    while(1)
    {
        // Place code that runs continuously in here
        if(enc_flag)
        { // Check to see if capture occurred
            enc_flag = 0; // reset capture flag
            uint16_t distance = enc_total * 220.0/360.0 ; // Calculate distance travelled in mm
            float delta_t = enc_counts/24000000.0; // Calculate the time between previous and current event
            uint16_t speed_rpm = (1.0 / 360.0) * (1.0 / delta_t) * 60.0; // Calculate the instantaneous wheel speed in rpm
            uint16_t speed_mm = ((2.0 * 3.14) / (360.0 * delta_t)) * 35.0; // Calculate the instantaneous car speed in mm/s
            printf("%5u mm\t%6u\t\t%.4f s\t%5u rpm\t%5u mm/s\r\n",distance,enc_counts,delta_t,speed_rpm,speed_mm);
        }
    }   
}    //// Main Function ////  

// Add function declarations here as needed
void GPIOInit()
{
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P10, GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);
}

void timerInit()
{
    config.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    config.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    config.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    Timer_A_configureContinuousMode(TIMER_A3_BASE, &config);
    //TA3.CCI0A
    capture.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_0;
    capture.captureInputSelect = TIMER_A_CAPTURE_INPUTSELECT_CCIxA;
    capture.captureOutputMode = 0;
    capture.captureMode = TIMER_A_CAPTUREMODE_RISING_EDGE;
    capture.captureInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    capture.synchronizeCaptureSource = TIMER_A_CAPTURE_SYNCHRONOUS;

    Timer_A_initCapture(TIMER_A3_BASE, &capture);

    Timer_A_registerInterrupt(TIMER_A3_BASE, TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT, encoderISR);
    Timer_A_registerInterrupt(TIMER_A3_BASE, TIMER_A_CCR0_INTERRUPT, encoderISR);
    Timer_A_startCounter(TIMER_A3_BASE, TIMER_A_CONTINUOUS_MODE);
}
// Add interrupt functions last so they are easy to find

void encoderISR()
{
    if (Timer_A_getEnabledInterruptStatus(TIMER_A3_BASE) == TIMER_A_INTERRUPT_PENDING)
    {
        Timer_A_clearInterruptFlag(TIMER_A3_BASE);
        enc_counts_track += 65536;
    }
    else if(Timer_A_getCaptureCompareEnabledInterruptStatus(TIMER_A3_BASE ,TIMER_A_CAPTURECOMPARE_REGISTER_0) & TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG)
    {
        uint16_t temp = Timer_A_getCaptureCompareCount(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
        //printf("test");
        Timer_A_clearCaptureCompareInterrupt(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
        enc_total++;
        enc_counts = temp + enc_counts_track;
        enc_counts_track = -(temp);
        enc_flag = 1;
    }
}
