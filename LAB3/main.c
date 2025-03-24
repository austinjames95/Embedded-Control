
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

// Function Prototypes
void GPIOinit();
void timerInit();
void encoderISR();
void Drive(uint16_t directions[], uint8_t size);

// Global Variables
Timer_A_UpModeConfig timer;
Timer_A_ContinuousModeConfig timerCon;
Timer_A_CompareModeConfig compare_L, compare_R;
Timer_A_CaptureModeConfig capture_L, capture_R;

// PWM Variables
uint16_t compareL = 240, compareR = 240;
uint16_t period = 480;

// Timer

// Encoder Variables
uint32_t enc_total_L = 0, enc_total_R = 0;
int32_t enc_counts_L = 0, enc_counts_R = 0;
int32_t enc_counts_track_L = 0, enc_counts_track_R = 0;
uint8_t enc_flag_L = 0, enc_flag_R = 0;

// Averaging Variables
uint32_t sum_counts_L = 0, sum_counts_R = 0;
uint8_t count_samples_L = 0, count_samples_R = 0;

// Movement Vector
uint16_t pathA[] = {120, 2, 3, 4};
uint16_t pathB[];

const uint16_t setpoint = 65000;

int main()
{
    sysInit();
    GPIOinit();
    timerInit();


    while (1)
    {
        if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0))
        {
            __delay_cycles(240e3);
            while (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0));
            __delay_cycles(240e3);
            Drive(pathA, 4);
        }
        else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN2))
        {
            __delay_cycles(240e3);
            while (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN2));
            __delay_cycles(240e3);
            Drive(pathB, 0);
        }
    }

}

void Drive(uint16_t directions[], uint8_t size)
{
    uint8_t i = 0;
    for (; i < size; i++)
    {
        uint16_t movement = directions[i];
        uint16_t distance = 0;
        if (movement >= 100)
        {
            distance = movement - 100;
            movement = 1;
        }
        if (movement == 1)
        {
            printf("moving forward\r\n");
            enc_total_L = 0;
            enc_total_R = 0;

            GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN7);
            GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN6);

            uint16_t target_counts = distance * 100;

            Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3, compareR);
            Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4, compareL);

            while(enc_total_R < target_counts || enc_total_L < target_counts);

            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN7);
            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN6);
        }
        else if (movement == 2)
        {
            printf("Turning right\r\n");
            enc_total_R = 0;
            uint16_t target_counts = 360;
            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN7);
            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN6);
            __delay_cycles(240e3);

            GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN6);
            GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN5);

            period = 100;
            Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3, compareR);

            while(enc_total_R < target_counts);
            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN6);
            //move 90 degrees right
        }
        else if (movement == 3)
        {
            printf("Turning left\r\n");
            enc_total_L = 0;
            uint16_t target_counts = 360;
            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN7);
            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN6);
            __delay_cycles(240e3);

            GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN7);
            GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN4);

            period = 100;
            Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4, compareL);

            while(enc_total_L < target_counts);
            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN7);
            //move 90 degrees left
        }
        else if (movement == 4)
        {
            //turn around 180 degrees
            printf("Turning 180 right\r\n");
            enc_total_R = 0;
            uint16_t target_counts = 720;
            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN7);
            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN6);
            __delay_cycles(240e3);

            GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN6);
            GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN5);

            period = 100;
            Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3, compareR);

            while(enc_total_R < target_counts);
            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN6);
            //move 180 degrees right ways
        }
        else if (movement == 5)
        {
            //turn 48 degrees down
            printf("Turning 48 degrees down\r\n");
            enc_total_R = 0;
            uint16_t target_counts = 192;
            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN7);
            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN6);
            __delay_cycles(240e3);

            GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN6);
            GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN5);

            period = 100;
            Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3, compareR);

            while(enc_total_R < target_counts);
            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN6);
            //move 90 degrees right
        }
    }
}

// GPIO Initialization
void GPIOinit()
{
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN0); // bumper zero
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN2); // bumper one

    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN7);  // Left Motor Enable
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN6);  // Right Motor Enable

    //GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN7);
    //GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN6);

    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN4); // Left Motor Direction
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN5); // Right Motor Direction

    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN4);
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN5);

    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P10, GPIO_PIN5, GPIO_PRIMARY_MODULE_FUNCTION);  // Left Encoder
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P10, GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);   // Right Encoder
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);  // Left Motor Speed
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION);  // Right Motor Speed
}

void timerInit()
{
    timer.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    timer.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    timer.timerPeriod = period;
    timer.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    Timer_A_configureUpMode(TIMER_A0_BASE, &timer);

    // TA3.CCI0A (Right Encoder) CCR3
    capture_R.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_0;
    capture_R.captureInputSelect = TIMER_A_CAPTURE_INPUTSELECT_CCIxA;
    capture_R.captureOutputMode = 0;
    capture_R.captureMode = TIMER_A_CAPTUREMODE_RISING_EDGE;
    capture_R.captureInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    capture_R.synchronizeCaptureSource = TIMER_A_CAPTURE_SYNCHRONOUS;
    Timer_A_initCapture(TIMER_A3_BASE, &capture_R);

    // TA3.CCI1A (Left Encoder) CCR2
    capture_L.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
    capture_L.captureInputSelect = TIMER_A_CAPTURE_INPUTSELECT_CCIxA;
    capture_L.captureOutputMode = 0;
    capture_L.captureMode = TIMER_A_CAPTUREMODE_RISING_EDGE;
    capture_L.captureInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    capture_L.synchronizeCaptureSource = TIMER_A_CAPTURE_SYNCHRONOUS;
    Timer_A_initCapture(TIMER_A3_BASE, &capture_L);

    // Right Wheel PWM
    compare_R.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_3;
    compare_R.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    compare_R.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    compare_R.compareValue = period;
    Timer_A_initCompare(TIMER_A0_BASE, &compare_R);

    // Left Wheel PWM
    compare_L.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_4;
    compare_L.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    compare_L.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    compare_L.compareValue = period;
    Timer_A_initCompare(TIMER_A0_BASE, &compare_L);

    timerCon.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    timerCon.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    timerCon.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    Timer_A_configureContinuousMode(TIMER_A3_BASE, &timerCon);
    //printf("09867");
    Timer_A_registerInterrupt(TIMER_A3_BASE, TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT, encoderISR);
    Timer_A_registerInterrupt(TIMER_A3_BASE, TIMER_A_CCR0_INTERRUPT, encoderISR);
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
    Timer_A_startCounter(TIMER_A3_BASE, TIMER_A_CONTINUOUS_MODE);
}

// Encoder ISR
void encoderISR()
{
    if (Timer_A_getCaptureCompareEnabledInterruptStatus(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1) & TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG)
    {
        uint16_t temp_L = Timer_A_getCaptureCompareCount(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1);
        Timer_A_clearCaptureCompareInterrupt(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1);
        enc_total_L++;
        enc_counts_L = temp_L + enc_counts_track_L;
        enc_counts_track_L = -(temp_L);
        enc_flag_L = 1;

        sum_counts_L += enc_counts_L;

        count_samples_L++;

        if (count_samples_L == 6)
        {
            uint32_t avg_L = sum_counts_L / 6;
            if (avg_L > setpoint)
            {
                compareL++;
            }
            else if (avg_L < setpoint)
            {
                compareL--;
            }
            count_samples_L = 0;
            sum_counts_L = 0;
            printf("encoder counts %u,  compare value %u\r\n", avg_L, compareL);
            Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4, compareL);
        }
    }

    if (Timer_A_getCaptureCompareEnabledInterruptStatus(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0) & TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG)
    {
        //printf("23132");
        uint16_t temp_R = Timer_A_getCaptureCompareCount(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
        Timer_A_clearCaptureCompareInterrupt(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
        enc_total_R++;
        enc_counts_R = temp_R + enc_counts_track_R;
        enc_counts_track_R = -(temp_R);
        enc_flag_R = 1;

        sum_counts_R += enc_counts_R;

        count_samples_R++;

        if (count_samples_R == 6)
        {
            uint32_t avg_R = sum_counts_R / 6;
            if (avg_R > setpoint)
            {
                compareR++;
            }
            else if (avg_R < setpoint)
            {
                compareR--;
            }
            count_samples_R = 0;
            sum_counts_R = 0;
            printf("encoder counts %u,  compare value %u\r\n", avg_R, compareR);
            Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3, compareR);
        }
    }

    if (Timer_A_getEnabledInterruptStatus(TIMER_A3_BASE) == TIMER_A_INTERRUPT_PENDING)
    {
        Timer_A_clearInterruptFlag(TIMER_A3_BASE);
        enc_counts_track_L += 65536;
        enc_counts_track_R += 65536;
    }
}

