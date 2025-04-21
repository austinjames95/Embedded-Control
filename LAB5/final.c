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

void GPIOInit();
void TimerInit();
void I2CInit();
void Encoder_ISR();
void T2_100ms_ISR();
void readCompass();

float updateHeadingControl(int Heading, float* calcHeading, int enc_L, int enc_R, float kp, float kd);
float limitPWM(float pwmVal, float minVal, float maxVal);
void checkZeroMotion(float targetSpeed, float steerDiff);

Timer_A_UpModeConfig TA0cfg; // PWM timer
Timer_A_UpModeConfig TA2cfg; // 100 ms timer
Timer_A_ContinuousModeConfig TA3cfg; // Encoder timer
Timer_A_CompareModeConfig TA0_ccr3; // PWM Right
Timer_A_CompareModeConfig TA0_ccr4; // PWM Left
Timer_A_CaptureModeConfig TA3_ccr0; // Encoder Right
Timer_A_CaptureModeConfig TA3_ccr1; // Encoder Left


// Encoder total events
int enc_total_L,enc_total_R;
// Speed measurement variables
// Note that "Tach" stands for "Tachometer," or a device used to measure rotational speed
int32_t TachL_count,TachL,TachL_sum,TachL_sum_count,TachL_avg; // Left wheel
int32_t TachR_count,TachR,TachR_sum,TachR_sum_count,TachR_avg; // Right wheel
    // TachL,TachR are equivalent to enc_counts from Activity 10/Lab 3
    // TachL/R_avg is the averaged TachL/R value after every 12 encoder measurements
    // The rest are the intermediate variables used to assemble TachL/R_avg

uint8_t runControl = 0; // Flag to denote that 100ms has passed and control should be run.


float targetSpeed, targetSpeedR, targetSpeedL, steerDiff = 0;
float calcHeading = 0, delta_theta = 0;
float headingError = 0, prevHeadingError = 0;
float kpHead = 0.25;
float kdHead = 0.02;
uint16_t Heading = 0;
float CCR_valR, CCR_valL = 0.0;
float pwmR, pwmL = 0.0;
float iSumL, iSumR = 0.0;
float ki, kp = 0.0;
float errorR, errorL;
float measuredPWMR, measuredPWML = 0.0;

eUSCI_I2C_MasterConfig config;

float headingTotal, deltaTheta = 0.0;
int prev_enc_L, prev_enc_R = 0;
int total_enc_L, total_enc_R = 0;
const float C = 0.61; // units of mm
const float a = 0.149; // units of mm

int8_t tilt = 0;
int8_t roll = 0;

   int main() {    /** Main Function ****/
    sysInit();
    GPIOInit();
    I2CInit();
    TimerInit();

    __delay_cycles(24e6);
    GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN7 | GPIO_PIN6);
    while(1)
    {

        if(runControl){    // If 100 ms has passed
            runControl = 0;    // Reset the 100 ms flag

            readCompass();
            if (!GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN3))
            {
                steerDiff = updateHeadingControl(Heading, &calcHeading, enc_total_L, enc_total_R, kpHead, kdHead);
                enc_total_L = 0;
                enc_total_R = 0;
            }
            else
            {
                steerDiff = 0;
            }

            targetSpeed = tilt / 256.0;

            if (targetSpeed < 0.1 && targetSpeed > -0.1)
            {
                targetSpeed = 0;
            }

            if (steerDiff < 0.15 && steerDiff > -0.15)
            {
                steerDiff = 0;
            }

            targetSpeedR = targetSpeed - steerDiff;
            targetSpeedL = targetSpeed + steerDiff;

            if (targetSpeed == 0 && steerDiff == 0)
            {
                measuredPWML = 0;
                measuredPWMR = 0;
            }
            else
            {
                measuredPWMR = ((15000.0 / TachR_avg));
                measuredPWML = ((15000.0 / TachL_avg));
            }

            errorR = fabs(targetSpeedR) - fabs(measuredPWMR);
            errorL = fabs(targetSpeedL) - fabs(measuredPWML);

            iSumR += errorR;
            iSumL += errorL;

            pwmR += kp * errorR + ki * iSumR;
            pwmL += kp * errorL + ki * iSumL;

            CCR_valR = pwmR * 959;
            CCR_valL = pwmL * 959;

            CCR_valR = limitPWM(CCR_valR, 95, 479);
            CCR_valL = limitPWM(CCR_valL, 95, 479);

            checkZeroMotion(targetSpeed, steerDiff);

            if ((targetSpeedL) > 0)
            {
                 // forward
                GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN4);
            }
            else
            {
                // backward
                GPIO_setOutputHighOnPin(GPIO_PORT_P5,GPIO_PIN4);
            }

            if ((targetSpeedR) > 0)
            {
                // forward
                GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN5);
            }
            else
            {
                // backward
                GPIO_setOutputHighOnPin(GPIO_PORT_P5,GPIO_PIN5);
            }

            int deltaEncL = enc_total_L - prev_enc_L;
            int deltaEncR = enc_total_R - prev_enc_R;

            if (!GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN2))
            {
                deltaEncL *= -1;
            }
            if (!GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN0))
            {
                deltaEncR *= -1;
            }

            total_enc_L += deltaEncL;
            total_enc_R += deltaEncR;

            deltaTheta = (C / a) * (deltaEncL - deltaEncR);
            headingTotal += deltaTheta;

            //printf("deltaTheta: %.4f rad, Compass Heading: %d\r\n", deltaTheta, Heading);

            Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3, fabs(CCR_valR));
            Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4, fabs(CCR_valL));
        }
    }
}    /** End Main Function ****/

float updateHeadingControl(int Heading, float* calcHeading, int enc_L, int enc_R, float kp, float kd)
{
    float deltaTheta = (C / 149.0) * (enc_L - enc_R);
    *calcHeading += deltaTheta * 572.96;

    static float previousError = 0.0;
    float headingError = Heading - *calcHeading;

    if (headingError > 1800)
    {
        headingError -= 3600;
    }
    else if (headingError < -1800)
    {
        headingError += 3600;
    }

    float deltaError = headingError - previousError;
    float steer = kp * headingError + kd * deltaError;

    previousError = headingError;

    //printf("Compass: %d | Est: %.2f | Err: %.2f | Diff: %.2f\r\n", Heading, *calcHeading, headingError, steer);

    return steer;
}

float limitPWM(float pwmVal, float minVal, float maxVal)
{
    if (pwmVal > maxVal)
    {
        return maxVal;
    }
    else if (pwmVal < minVal)
    {
        return minVal;
    }
    else
    {
        return pwmVal;
    }
}

void checkZeroMotion(float targetSpeed, float steerDiff)
{
    if (targetSpeed == 0.0 && steerDiff == 0.0)
    {
        pwmR = 0;
        pwmL = 0;
        CCR_valR = 0;
        CCR_valL = 0;
    }
}

void I2CInit()
{
    config.selectClockSource = EUSCI_B_I2C_CLOCKSOURCE_SMCLK;
    config.i2cClk = 24000000;
    config.dataRate = EUSCI_B_I2C_SET_DATA_RATE_100KBPS;
    config.byteCounterThreshold = 0;
    I2C_initMaster(EUSCI_B3_BASE, &config);
    I2C_enableModule(EUSCI_B3_BASE);
}

void GPIOInit(){
    GPIO_setAsOutputPin(GPIO_PORT_P5,GPIO_PIN4|GPIO_PIN5);   // Motor direction pins
    GPIO_setAsOutputPin(GPIO_PORT_P3,GPIO_PIN6|GPIO_PIN7);   // Motor enable pins
    GPIO_setAsInputPin(GPIO_PORT_P3,GPIO_PIN3); // switch pin
    // Motor PWM pins
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2,GPIO_PIN6|GPIO_PIN7,GPIO_PRIMARY_MODULE_FUNCTION);
    // Motor Encoder pins
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P10,GPIO_PIN4|GPIO_PIN5,GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsInputPin(GPIO_PORT_P5, GPIO_PIN0 | GPIO_PIN2); // direction pin
    // Setting I2C pins
    GPIO_setAsPeripheralModuleFunctionOutputPin( GPIO_PORT_P6 , GPIO_PIN6 , GPIO_SECONDARY_MODULE_FUNCTION );
    GPIO_setAsPeripheralModuleFunctionOutputPin( GPIO_PORT_P6 , GPIO_PIN7 , GPIO_SECONDARY_MODULE_FUNCTION );

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
    TA2cfg.timerPeriod = 18749;
    Timer_A_configureUpMode(TIMER_A2_BASE,&TA2cfg);
    Timer_A_registerInterrupt(TIMER_A2_BASE,TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT,T2_100ms_ISR);
    // Start all the timers
    Timer_A_startCounter(TIMER_A0_BASE,TIMER_A_UP_MODE);
    Timer_A_startCounter(TIMER_A2_BASE,TIMER_A_UP_MODE);
    Timer_A_startCounter(TIMER_A3_BASE,TIMER_A_CONTINUOUS_MODE);
}

void readCompass()
{
    uint8_t arry[4];
    I2C_readData(EUSCI_B3_BASE, 0x60, 2, arry, 4);
    tilt = arry[2];
    roll = arry[3];
    Heading = ((uint16_t) arry[0] << 8) | arry[1];
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
    // Otherwise if the Right Encoder triggered...
    }else if(Timer_A_getCaptureCompareEnabledInterruptStatus(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0)&TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG){
        Timer_A_clearCaptureCompareInterrupt(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);

        if (GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN0))
        {
            enc_total_R++;
            total_enc_R++;
        }
        else
        {
            enc_total_R--;
            total_enc_R--;
        }
        // Calculate and track the encoder count values
        TachR = TachR_count + Timer_A_getCaptureCompareCount(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);
        TachR_count = -Timer_A_getCaptureCompareCount(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);
        // Sum values for averaging
        TachR_sum_count++;
        // Check foward and reverse
        if(GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN0)){
            TachR_sum += TachR;
        }else{
            TachR_sum -= TachR;
        }
        // If 6 values have been received, average them.
        if(TachR_sum_count == 6){
            TachR_avg = TachR_sum/6;
            TachR_sum_count = 0;
            TachR_sum = 0;
        }
    // Otherwise if the Left Encoder triggered...
    }else if(Timer_A_getCaptureCompareEnabledInterruptStatus(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_1)&TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG){
        Timer_A_clearCaptureCompareInterrupt(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_1);
        if (GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN2))
        {
            enc_total_L++;
            total_enc_L++;
        }
        else
        {
            enc_total_L--;
            total_enc_L--;
        }
        TachL = TachL_count + Timer_A_getCaptureCompareCount(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_1);
        TachL_count = -Timer_A_getCaptureCompareCount(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_1);
        TachL_sum_count++;

        if(GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN2)){
            TachL_sum += TachL;
            //printf("foward\r\n");
        }else{
            TachL_sum -= TachL;
            //printf("reverse\r\n");
        }

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
