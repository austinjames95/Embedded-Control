#include "engr2350_msp432.h"
#include <math.h>

// Function Prototypes
void GPIOinit();
void timerInit();
void encoderISR();
void updatePWM();

// Global Variables
Timer_A_UpModeConfig timer;
Timer_A_ContinuousModeConfig timerCon;
Timer_A_CompareModeConfig compare_L, compare_R;
Timer_A_CaptureModeConfig capture_L, capture_R;

// PWM Variables
float dutycycle_L = 0.5, dutycycle_R = 0.5;
uint16_t compareL=240, compareR=240;
uint16_t period = 960; //480

// Encoder Variables
uint32_t enc_total_L = 0, enc_total_R = 0;
int32_t enc_counts_L = 0, enc_counts_R = 0;
int32_t enc_counts_track_L = 0, enc_counts_track_R = 0;
uint8_t enc_flag_L = 0, enc_flag_R = 0;

// Averaging Variables
uint32_t sum_counts_L = 0, sum_counts_R = 0;
uint8_t count_samples_L = 0, count_samples_R = 0;

const uint16_t setpoint = 65000;

int main()
{
    sysInit();
    GPIOinit();
    timerInit();

    while (1)
    {
        //GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN7); // Turn On Left Motor
        //GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN6); // Turn On Right Motor

        //printf(" encoder up right  %u\r\n", enc_flag_R);
//        if (enc_flag_L || enc_flag_R)
//        {
//            printf("test");
//            enc_flag_L = 0;
//            enc_flag_R = 0;
//
//            float delta_t_L = enc_counts_L / 24000000.0;
//            float delta_t_R = enc_counts_R / 24000000.0;
//
//            uint16_t speed_L = (1.0 / delta_t_L) / 360.0 * 60.0;
//            uint16_t speed_R = (1.0 / delta_t_R) / 360.0 * 60.0;
//
//            printf("%5u mm\t%6u\t%.4f s\t%5u rpm\t%2f\t|\t%5u mm\t%6u\t%.4f s\t%5u rpm\t%2f\r\n",
//                   enc_total_L, enc_counts_L, delta_t_L, speed_L, dutycycle_L,
//                   enc_total_R, enc_counts_R, delta_t_R, speed_R, dutycycle_R);
//        }
    }
}

// GPIO Initialization
void GPIOinit()
{
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN7);  // Left Motor Enable
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN6);  // Right Motor Enable

    GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN7);
    GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN6);

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
    compare_R.compareValue = roundf(period * dutycycle_R);
    Timer_A_initCompare(TIMER_A0_BASE, &compare_R);

    // Left Wheel PWM
    compare_L.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_4;
    compare_L.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    compare_L.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    compare_L.compareValue = roundf(period * dutycycle_L);
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

// PWM Adjustment Logic
void updatePWM()
{
    uint16_t avg_L = sum_counts_L / 6;
    uint16_t avg_R = sum_counts_R / 6;

    if (avg_L > setpoint)
    {
//        if (dutycycle_L > 0.1)
//        {
//            dutycycle_L -= 0.01;
//        }
        compareL --;
    }
    else if (avg_L < setpoint)
    {
//        if (dutycycle_L < 0.9)
//        {
//            dutycycle_L += 0.01;
//        }
        compareL ++;
    }

    if (avg_R > setpoint)
    {
//        if (dutycycle_R > 0.1)
//        {
//            dutycycle_R -= 0.01;
//        }
        compareR --;
    }
    else if (avg_R < setpoint)
    {
//        if (dutycycle_R < 0.9)
//        {
//            dutycycle_R += 0.01;
//        }
        compareR ++;
    }
    //Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4, roundf(period * dutycycle_L));
    //Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3, roundf(period * dutycycle_R));
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

