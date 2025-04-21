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


float desSpeed, desSpeedR, desSpeedL, diffSpeed = 0;
float calcHeading = 0, delta_theta = 0;
float headingError = 0, prevHeadingError = 0;
float kpHead.25, kdHead = 0.02;
uint16_t Heading = 0;
float CCR_valR, CCR_valL = 0.0;
float PWM_setR, PWM_setL = 0.0;
float error_sumR, error_sumL;
float ki, kp = 0.0;
float error_R, error_L;
int prev_heading_error = 0.0;
float real_pwmR, real_pwmL = 0.0;

eUSCI_I2C_MasterConfig config;

int heading_desired = 900;

float heading_rm, deltaTheta = 0.0;
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
            if (GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN2))
            {
                diffSpeed = 0;
            }
            else
            {
                //delta theta calculations
                deltaTheta = (0.610865/149)*(enc_total_L - enc_total_R);
                calcHeading += deltaTheta * 57.29578 * 10;

                prevHeadingError = headingError;
                headingError = Heading - calcHeading;

                if (headingError > 1800)
                {
                    headingError -= 3600;
                }
                
                if (headingError < -1800 )
                {
                    headingError += 3600;
                }
                
                diffSpeed = kpHead * headingError + kdHead * headingError - prevHeadingError;
                printf("H: %u MH: %f Ds %f  kd: %f\r\n", Heading, calcHeading, diffSpeed, kdHead * headingError - prevHeadingError);
                enc_total_L = 0;
                enc_total_R = 0;
            }


            desSpeed = tilt/256.0;
            //desSpeed = 0;

            if (desSpeed < 0.1 && desSpeed > -0.1)
            {
                desSpeed = 0;
            }

            if (diffSpeed < 0.15 && diffSpeed > -0.15)
            {
                diffSpeed = 0;
            }
            desSpeedR = desSpeed - diffSpeed;
            desSpeedL = desSpeed + diffSpeed;

            if ((desSpeedL) > 0)
            {
                 // forward
                GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN4);
            }
            else
            {
                // backward
                GPIO_setOutputHighOnPin(GPIO_PORT_P5,GPIO_PIN4);
            }

            if ((desSpeedR) > 0)
            {
                // forward
                GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN5);
            }
            else
            {
                // backward
                GPIO_setOutputHighOnPin(GPIO_PORT_P5,GPIO_PIN5);
            }

            if (desSpeed == 0 && diffSpeed == 0)
            {
                real_pwmL = 0;
                real_pwmR = 0;
            }
            else
            {
                real_pwmR = ((15000.0/TachR_avg));
                real_pwmL = ((15000.0/TachL_avg));
            }

            error_R = fabs(desSpeedR) - fabs(real_pwmR);
            error_L = fabs(desSpeedL) - fabs(real_pwmL);

            error_sumR += error_R;
            error_sumL += error_L;

            PWM_setR += kp*error_R + ki*error_sumR;
            PWM_setL += kp*error_L + ki*error_sumL;

            CCR_valR = PWM_setR * 959;
            CCR_valL = PWM_setL * 959;

            // PWM speed settings

            if (CCR_valR > 479)
            {
                CCR_valR = 479;
            }
            else if (CCR_valR < 95)
            {
                CCR_valR = 95;
            }
            if (CCR_valL > 479)
            {
                CCR_valL = 479;
            }
            else if (CCR_valL < 95)
            {
                CCR_valL = 95;
            }
            if (desSpeed == 0 && diffSpeed == 0)
            {
                CCR_valR = 0;
                CCR_valL = 0;
                PWM_setR = 0;
                PWM_setL = 0;
            }
            //printf("real_pwmR: %.2f PWM_setR; %.2f DES: %.2f    ERR: %.4f    SUM: %.2f    CCR: %.2f\r\n", real_pwmR, PWM_setR, desSpeed, error_R, error_sumR, CCR_valR);

            int deltaEncL = enc_total_L - prev_enc_L;
            int deltaEncR = enc_total_R - prev_enc_R;

            if (!GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN2)) { deltaEncL *= -1; }
            if (!GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN0)) { deltaEncR *= -1; }

            total_enc_L += deltaEncL;
            total_enc_R += deltaEncR;

            deltaTheta = (C / a) * (deltaEncL - deltaEncR);
            heading_rm += deltaTheta;

            //printf("deltaTheta: %.4f rad, Compass Heading: %d\r\n", deltaTheta, Heading);

            Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3, fabs(CCR_valR));
            Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4, fabs(CCR_valL));
        }
    }
}    /** End Main Function ****/

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
    GPIO_setAsInputPin(GPIO_PORT_P3,GPIO_PIN2); // switch pin
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
