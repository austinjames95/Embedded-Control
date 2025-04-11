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
eUSCI_I2C_MasterConfig config;
void GPIOInit();
void I2CInit();
uint16_t readCompass();
uint16_t readRanger();
uint8_t array[1];

// Add global variables here as needed.

int main() {    //// Main Function ////
  
    // Add local variables here as needed.

    // We always call the sysInit function first to set up the 
    // microcontroller for how we are going to use it.
    sysInit();
    GPIOInit();
    I2CInit();

    // Place initialization code (or run-once) code here
    __delay_cycles(2.4e6);
    printf("start\r\n");
    while(1){  
        // Place code that runs continuously in here
        printf("Compass: %4u\tRanger: %4u\r\n",readCompass(),readRanger());
        //printf("Compass: %4u\r\n",readCompass());

        __delay_cycles(2.4e6); // Wait 1/10 of a second
    }   
}    //// Main Function ////  

// Add function declarations here as needed
void GPIOInit()
{
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P6, GPIO_PIN6, GPIO_SECONDARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P6, GPIO_PIN7, GPIO_SECONDARY_MODULE_FUNCTION);
}

void I2CInit()
{
    config.selectClockSource = EUSCI_B_I2C_CLOCKSOURCE_SMCLK;
    config.i2cClk = 24000000;
    config.dataRate = EUSCI_B_I2C_SET_DATA_RATE_100KBPS;
    config.byteCounterThreshold = 2;
    config.autoSTOPGeneration = EUSCI_B_I2C_NO_AUTO_STOP;
    I2C_initMaster(EUSCI_B3_BASE, &config);
    I2C_enableModule(EUSCI_B3_BASE);
}

uint16_t readCompass()
{
    uint8_t arry[2];

    I2C_readData(EUSCI_B3_BASE,0x60,2,arry,2); // read values from register 4-6
    uint16_t result = arry[0] << 8;
    result += arry[1];
    printf("4: %u\t5: %u\r\n",arry[0],arry[1]); // print out values
    return result;
}

uint16_t readRanger()
{
    uint8_t rangeData[2];
    uint16_t distance = 0;

    I2C_readData(EUSCI_B3_BASE, 0x70, 2, rangeData, 2);
    distance = ((uint16_t)rangeData[0] << 8) | rangeData[1];


    array[0] = 0x51;
    I2C_writeData(EUSCI_B3_BASE, 0x70, 0, array, 1);

    return distance;
}
// Add interrupt functions last so they are easy to find
