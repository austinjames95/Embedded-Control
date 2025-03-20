#include "engr2350_msp432.h"
#include <math.h>

// Function Prototypes
void GPIOinit();
void timerInit();
void encoderISR();
void updatePWM();

// Global Variables
Timer_A_UpModeConfig timer;
Timer_A_CompareModeConfig compare_L, compare_R;

// PWM Variables
float dutycycle_L = 0.5, dutycycle_R = 0.5;
uint16_t period = 16384;

// Encoder Variables
uint32_t enc_total_L = 0, enc_total_R = 0;
int32_t enc_counts_L = 0, enc_counts_R = 0;
int32_t enc_counts_track_L = 0, enc_counts_track_R = 0;
uint8_t enc_flag_L = 0, enc_flag_R = 0;

// Averaging Variables
uint32_t sum_counts_L = 0, sum_counts_R = 0;
uint8_t count_samples_L = 0, count_samples_R = 0;

// Setpoint for PWM adjustment
const uint16_t setpoint = 40000;

int main()
{
    sysInit();
    GPIOinit();
    timerInit();

    printf("\r\nDistance_L\tEnc_L\tDelta_T_L\tSpeed_L\tPWM_L\t|\tDistance_R\tEnc_R\tDelta_T_R\tSpeed_R\tPWM_R\r\n");

    while (1)
    {
        GPIO_toggleOutputOnPin(GPIO_PORT_P3, GPIO_PIN7); // Turn On Left Motor
        GPIO_toggleOutputOnPin(GPIO_PORT_P3, GPIO_PIN6); // Turn On Right Motor

        if (enc_flag_L || enc_flag_R)
        {
            enc_flag_L = 0;
            enc_flag_R = 0;

            float delta_t_L = enc_counts_L / 24000000.0;
            float delta_t_R = enc_counts_R / 24000000.0;

            sum_counts_L += enc_counts_L;
            sum_counts_R += enc_counts_R;

            count_samples_L++;
            count_samples_R++;

            if (count_samples_L == 6 && count_samples_R == 6)
            {
                updatePWM();
                count_samples_L = 0;
                count_samples_R = 0;
                sum_counts_L = 0;
                sum_counts_R = 0;
            }

            uint16_t speed_L = (1.0 / delta_t_L) / 360.0 * 60.0;
            uint16_t speed_R = (1.0 / delta_t_R) / 360.0 * 60.0;

            printf("%5u mm\t%6u\t%.4f s\t%5u rpm\t%2f\t|\t%5u mm\t%6u\t%.4f s\t%5u rpm\t%2f\r\n",
                   enc_total_L, enc_counts_L, delta_t_L, speed_L, dutycycle_L,
                   enc_total_R, enc_counts_R, delta_t_R, speed_R, dutycycle_R);
        }
    }
}

// GPIO Initialization
void GPIOinit()
{
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN6);

    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P10, GPIO_PIN5, GPIO_PRIMARY_MODULE_FUNCTION);  // Left Encoder
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P10, GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);   // Right Encoder
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);  // Left Motor Speed
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION);  // Right Motor Speed
}

void timerInit()
{
    timer.clockSource = TIMER_A_CLOCKSOURCE_ACLK;
    timer.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_2;
    timer.timerPeriod = period;
    timer.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;

    // Left Wheel PWM
    compare_L.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
    compare_L.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    compare_L.compareOutputMode = 0;
    compare_L.compareValue = roundf(period * dutycycle_L);
    Timer_A_initCompare(TIMER_A2_BASE, &compare_L);

    // Right Wheel PWM
    compare_R.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_2;
    compare_R.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    compare_R.compareOutputMode = 0;
    compare_R.compareValue = roundf(period * dutycycle_R);
    Timer_A_initCompare(TIMER_A2_BASE, &compare_R);

    Timer_A_registerInterrupt(TIMER_A2_BASE, TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT, encoderISR);
    Timer_A_configureUpMode(TIMER_A2_BASE, &timer);
    Timer_A_startCounter(TIMER_A2_BASE, TIMER_A_UP_MODE);
}

// PWM Adjustment Logic
void updatePWM()
{
    uint16_t avg_L = sum_counts_L / 6;
    uint16_t avg_R = sum_counts_R / 6;

    if (avg_L > setpoint)
    {
        if (dutycycle_L > 0.1)
        {
            dutycycle_L -= 0.01;
        }
    }
    else if (avg_L < setpoint)
    {
        if (dutycycle_L < 0.9)
        {
            dutycycle_L += 0.01;
        }
    }

    if (avg_R > setpoint)
    {
        if (dutycycle_R > 0.1)
        {
            dutycycle_R -= 0.01;
        }
    }
    else if (avg_R < setpoint)
    {
        if (dutycycle_R < 0.9)
        {
            dutycycle_R += 0.01;
        }
    }

    timerInit(); // Update PWM settings
}

// Encoder ISR
void encoderISR()
{
    if (Timer_A_getCaptureCompareEnabledInterruptStatus(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0) & TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG)
    {
        uint16_t temp_L = Timer_A_getCaptureCompareCount(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
        Timer_A_clearCaptureCompareInterrupt(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
        enc_total_L++;
        enc_counts_L = temp_L + enc_counts_track_L;
        enc_counts_track_L = -temp_L;
        enc_flag_L = 1;
    }

    if (Timer_A_getCaptureCompareEnabledInterruptStatus(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1) & TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG)
    {
        uint16_t temp_R = Timer_A_getCaptureCompareCount(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1);
        Timer_A_clearCaptureCompareInterrupt(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1);
        enc_total_R++;
        enc_counts_R = temp_R + enc_counts_track_R;
        enc_counts_track_R = -temp_R;
        enc_flag_R = 1;
    }
}
