////////////////////////////////////////////////////////////////////////
//** ENGR-2350 Lab 4 Template
//** NAME: Austin Schlutter
//** RIN: 662080932
////////////////////////////////////////////////////////////////////////
//**
//** README!!!!!
//** README!!!!!
//** README!!!!!
//**
//** This template project has all initializations required to both control the motors
//** via PWM and measure the speed of the motors. The PWM is configured using a 25 kHz
//** period (960 counts). The motors are initialized to be DISABLED and in FORWARD mode.
//** The encoders measurements are stored within the variables TachR and TachL for the
//** right and left motors, respectively. A maximum value for TachR and TachL is
//** enforced to be 1e6 such that when the wheel stops, a reasonable value for the
//** encoders exists: a very large number that can be assumed to be stopped.
//** Finally, a third timer is added to measure a 100 ms period for control system
//** timing. The variable runControl is set to 1 each period and then reset in the main.

#include "engr2350_msp432.h"
#include <stdio.h>

void GPIOInit();
void TimerInit();
void ADCInit();
void Encoder_ISR();
void T2_100ms_ISR();

Timer_A_UpModeConfig TA0cfg; // PWM timer
Timer_A_UpModeConfig TA2cfg; // 100 ms timer
Timer_A_ContinuousModeConfig TA3cfg; // Encoder timer
Timer_A_CompareModeConfig TA0_ccr3; // PWM Right
Timer_A_CompareModeConfig TA0_ccr4; // PWM Left
Timer_A_CaptureModeConfig TA3_ccr0; // Encoder Right
Timer_A_CaptureModeConfig TA3_ccr1; // Encoder Left


// Encoder total events
uint32_t enc_total_L,enc_total_R;
// Speed measurement variables
// Note that "Tach" stands for "Tachometer," or a device used to measure rotational speed
int32_t TachL_count,TachL,TachL_sum,TachL_sum_count,TachL_avg; // Left wheel
int32_t TachR_count,TachR,TachR_sum,TachR_sum_count,TachR_avg; // Right wheel
    // TachL,TachR are equivalent to enc_counts from Activity 10/Lab 3
    // TachL/R_avg is the averaged TachL/R value after every 12 encoder measurements
    // The rest are the intermediate variables used to assemble TachL/R_avg


int potSpeed;
int potSteer;

uint8_t runControl = 0; // Flag to denote that 100ms has passed and control should be run.

float kI = 0.1;

float vdMax = 50.0;
float vdMin = 10.0;
float diffMax = 16.875;
float diffSpeed = 0.0;
float desiredR = 0.0;
float desiredL = 0.0;
float measuredL = 0.0;
float measuredR = 0.0;

float errorSumR = 0.0;
float errorSumL = 0.0;

int main() {    /** Main Function ****/
    sysInit();
    GPIOInit();
    ADCInit();
    TimerInit();

    __delay_cycles(24e6);

    GPIO_setOutputHighOnPin(GPIO_PORT_P3,GPIO_PIN6);
    GPIO_setOutputHighOnPin(GPIO_PORT_P3,GPIO_PIN7);
    Timer_A_setCompareValue(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_3, 480);

    while (1) {
        if (runControl) {
            runControl = 0;  // Reset 100 ms flag

            ADC14_toggleConversionTrigger();
            while (ADC14_isBusy());

            potSpeed = ADC14_getResult(ADC_MEM0);   // Speed potentiometer
            if (potSpeed > 16382)
            {
                potSpeed = 16382;
            }

            float speedNorm = (8191.0 - potSpeed) / 8191.0;
            float desiredSpeed = speedNorm * vdMax;

            potSteer = ADC14_getResult(ADC_MEM1);
            if (potSteer > 16382)
            {
                potSteer = 16382;
            }
            if (potSteer >= 10648.3 && potSteer <=  16382)
            {
                float steerNorm = (potSteer - 10648.3)/5733.7;
                diffSpeed = steerNorm * diffMax;
            }
            else if (potSteer >= 0 && potSteer <= 5733.7)
            {
                float steerNorm = (5733.7 - potSteer)/5733.7;
                diffSpeed = steerNorm * diffMax;
            }
            else
            {
                diffSpeed = 0;
            }

            if (potSteer < 8191.0)
            {
                desiredL = abs(desiredSpeed) + diffSpeed;
                desiredR = abs(desiredSpeed);
            }
            else
            {
                desiredR = abs(desiredSpeed) + diffSpeed;
                desiredL = abs(desiredSpeed);
            }

            if (desiredSpeed > 0)
            {
                printf("forward\n");
                GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN5);
                GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN4);
            }
            else
            {
                printf("reverse,\n");
                GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN5);
                GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN4);
            }

            if (fabs(desiredR) < 10.0)
            {
                Timer_A_setCompareValue(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_3, 0);
            }
            else
            {
                float speedR = 1500000.0 / TachR_avg;
                float speedErrorR = desiredR - speedR;
                errorSumR = errorSumR + speedErrorR;

                float correctR = desiredR + kI * errorSumR;
                int cmpR = (int)(correctR/100.0 * 960);
                Timer_A_setCompareValue(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_3, cmpR);
            }

            if (fabs(desiredL) < 10.0)
            {
                Timer_A_setCompareValue(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_4, 0);
            }
            else
            {
                float speedL = 1500000.0 / TachL_avg;
                float speedErrorL = desiredR - speedL;
                errorSumL = errorSumL + speedErrorL;

                float correctL = desiredL + kI * errorSumL;
                int cmpL = (int)(correctL/100.0 * 960);
                Timer_A_setCompareValue(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_4, cmpL);
            }
        }
    }
}    /** End Main Function ****/   

void ADCInit(){
    // Add your ADC initialization code here.
    //  Don't forget the GPIO, either here or in GPIOInit()!!
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_SMCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_4, ADC_NOROUTE);
    ADC14_setResolution(ADC_14BIT);
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A9, false);
    ADC14_configureConversionMemory(ADC_MEM1, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A12, false);
    //ADC14_configureSingleSampleMode(ADC_MEM12, false);
    ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM1, false);
    ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);
    ADC14_enableConversion();

}

void GPIOInit(){
    GPIO_setAsOutputPin(GPIO_PORT_P5,GPIO_PIN4|GPIO_PIN5);   // Motor direction pins
    GPIO_setAsOutputPin(GPIO_PORT_P3,GPIO_PIN6|GPIO_PIN7);   // Motor enable pins
        // Motor PWM pins

    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN1, GPIO_TERTIARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN4, GPIO_TERTIARY_MODULE_FUNCTION);
        // Add ADC Pins
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2,GPIO_PIN6|GPIO_PIN7,GPIO_PRIMARY_MODULE_FUNCTION);
        // Motor Encoder pins
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P10,GPIO_PIN4|GPIO_PIN5,GPIO_PRIMARY_MODULE_FUNCTION);

    GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN4|GPIO_PIN5);   // Motors set to forward
    GPIO_setOutputLowOnPin(GPIO_PORT_P3,GPIO_PIN6|GPIO_PIN7);   // Motors are OFF
}

void TimerInit(){
    // Configure PWM timer for 24 kHz
    TA0cfg.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    TA0cfg.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    TA0cfg.timerPeriod = 959;
    Timer_A_configureUpMode(TIMER_A0_BASE,&TA0cfg);
    // Configure TA0.CCR3 for PWM output, Right Motor
    TA0_ccr3.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_3;
    TA0_ccr3.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    TA0_ccr3.compareValue = 0;
    Timer_A_initCompare(TIMER_A0_BASE,&TA0_ccr3);
    // Configure TA0.CCR4 for PWM output, Left Motor
    TA0_ccr4.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_4;
    TA0_ccr4.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    TA0_ccr4.compareValue = 0;
    Timer_A_initCompare(TIMER_A0_BASE,&TA0_ccr4);
    // Configure Encoder timer in continuous mode
    TA3cfg.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    TA3cfg.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    TA3cfg.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    Timer_A_configureContinuousMode(TIMER_A3_BASE,&TA3cfg);
    // Configure TA3.CCR0 for Encoder measurement, Right Encoder
    TA3_ccr0.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_0;
    TA3_ccr0.captureMode = TIMER_A_CAPTUREMODE_RISING_EDGE;
    TA3_ccr0.captureInputSelect = TIMER_A_CAPTURE_INPUTSELECT_CCIxA;
    TA3_ccr0.synchronizeCaptureSource = TIMER_A_CAPTURE_SYNCHRONOUS;
    TA3_ccr0.captureInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    Timer_A_initCapture(TIMER_A3_BASE,&TA3_ccr0);
    // Configure TA3.CCR1 for Encoder measurement, Left Encoder
    TA3_ccr1.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
    TA3_ccr1.captureMode = TIMER_A_CAPTUREMODE_RISING_EDGE;
    TA3_ccr1.captureInputSelect = TIMER_A_CAPTURE_INPUTSELECT_CCIxA;
    TA3_ccr1.synchronizeCaptureSource = TIMER_A_CAPTURE_SYNCHRONOUS;
    TA3_ccr1.captureInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    Timer_A_initCapture(TIMER_A3_BASE,&TA3_ccr1);
    // Register the Encoder interrupt
    Timer_A_registerInterrupt(TIMER_A3_BASE,TIMER_A_CCR0_INTERRUPT,Encoder_ISR);
    Timer_A_registerInterrupt(TIMER_A3_BASE,TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT,Encoder_ISR);
    // Configure 10 Hz timer
    TA2cfg.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    TA2cfg.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_64;
    TA2cfg.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    TA2cfg.timerPeriod = 37499;
    Timer_A_configureUpMode(TIMER_A2_BASE,&TA2cfg);
    Timer_A_registerInterrupt(TIMER_A2_BASE,TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT,T2_100ms_ISR);
    // Start all the timers
    Timer_A_startCounter(TIMER_A0_BASE,TIMER_A_UP_MODE);
    Timer_A_startCounter(TIMER_A2_BASE,TIMER_A_UP_MODE);
    Timer_A_startCounter(TIMER_A3_BASE,TIMER_A_CONTINUOUS_MODE);
}


void Encoder_ISR(){
    // If encoder timer has overflowed...
    if(Timer_A_getEnabledInterruptStatus(TIMER_A3_BASE) == TIMER_A_INTERRUPT_PENDING){
        Timer_A_clearInterruptFlag(TIMER_A3_BASE);
        TachR_count += 65536;
        if(TachR_count >= 1e6){ // Enforce a maximum count to TachR so stopped can be detected
            TachR_count = 1e6;
            TachR = 1e6;
        }
        TachL_count += 65536;
        if(TachL_count >= 1e6){ // Enforce a maximum count to TachL so stopped can be detected
            TachL_count = 1e6;
            TachL = 1e6;
        }
    // Otherwise if the Left Encoder triggered...
    }else if(Timer_A_getCaptureCompareEnabledInterruptStatus(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0)&TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG){
        Timer_A_clearCaptureCompareInterrupt(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);
        enc_total_R++;   // Increment the total number of encoder events for the left encoder
        // Calculate and track the encoder count values
        TachR = TachR_count + Timer_A_getCaptureCompareCount(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);
        TachR_count = -Timer_A_getCaptureCompareCount(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);
        // Sum values for averaging
        TachR_sum_count++;
        TachR_sum += TachR;
        // If 6 values have been received, average them.
        if(TachR_sum_count == 6){
            TachR_avg = TachR_sum/6;
            TachR_sum_count = 0;
            TachR_sum = 0;
        }
    // Otherwise if the Right Encoder triggered...
    }else if(Timer_A_getCaptureCompareEnabledInterruptStatus(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_1)&TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG){
        Timer_A_clearCaptureCompareInterrupt(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_1);
        enc_total_L++;
        TachL = TachL_count + Timer_A_getCaptureCompareCount(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_1);
        TachL_count = -Timer_A_getCaptureCompareCount(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_1);
        TachL_sum_count++;
        TachL_sum += TachL;
        if(TachL_sum_count == 6){
            TachL_avg = TachL_sum/6;
            TachL_sum_count = 0;
            TachL_sum = 0;
        }
    }
}

void T2_100ms_ISR(){
    Timer_A_clearInterruptFlag(TIMER_A2_BASE);
    runControl = 1;
}
