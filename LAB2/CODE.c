////////////////////////////////////////////////////////////////////////
//** ENGR-2350 Lab 2
//** NAMEs: Austin Schlutter, Zade
//** RINs: XXXX, YYYY
//** Section: 2
////////////////////////////////////////////////////////////////////////

#include "engr2350_msp432.h"

// Add function prototypes here, as needed.
void GPIOInit();
void timerInit(Timer_A_UpModeConfig *config);
void Timer_ISR();
int8_t readBumpers();
void setRGB(int8_t color);
uint8_t checkGuess(int8_t *sol,int8_t *guess,int8_t *result);
void printResult(int8_t *guess,int8_t *result);
void Colordle();
void resetColordle();
void generateRandom();
void wait(uint16_t time, uint16_t counter);

// Add global variables here, as needed.
const uint8_t Lpattern = 4; // The pattern length. Could be changed if desired.
Timer_A_UpModeConfig config;
uint16_t timer_reset_count = 0;
uint16_t button_time = 0;
int8_t sol[4];
int8_t guess[4];
int8_t color;
uint8_t guessCount = 0;
int8_t index = 0;
int8_t checkWin = 0;
int8_t res[] = {0, 0, 0, 0};
uint16_t totalTime = 0;
uint8_t start = 0;


int main(void) {    //// Main Function ////

    sysInit();
    GPIOInit();
    timerInit(&config);

    // Place initialization code (or run-once) code here

    printf("Welcome to Colordle!\r\nPress pushbutton to start\r\n\r\n");

    resetColordle();
    while (!start) // if the game is not currently running
    {
        if (GPIO_getInputPinValue(GPIO_PORT_P6, GPIO_PIN1)) // check if button press
        {
            while (GPIO_getInputPinValue(GPIO_PORT_P6, GPIO_PIN1))
            {
                __delay_cycles(240e3);
            }
            __delay_cycles(240e3);
            if (!GPIO_getInputPinValue(GPIO_PORT_P6,  GPIO_PIN1))
            {
                start = 1;
                generateRandom();
                setRBG(6);
                timer_reset_count = 0;
                wait(10, timer_reset_count);
                totalTime = 0;
            }
        }
    }

    while (start)
    {
        Colordle();
    }
}    //// Main Function ////

void Colordle()
{
    if (timer_reset_count >= 300)
    {
        int i = 0;
        for (i = 0; i < Lpattern; i++)
        {
            guess[i] = NULL;
        }

        printf("Timeout!\r\n----");
        guessCount++;
    }
    else
    {
        color = readBumpers();
        if (color != -1)
        {
            guess[index] = color;
            index++;
        }
        else if (GPIO_getInputPinValue(GPIO_PORT_P6,  GPIO_PIN1)) // if the button is pressed to reset the guess, flash a white light
        {
            setRGB(6);
            button_time = 0;
            wait(5, button_time);
            setRGB(-1);
            index = 0;
        }
    }

    if (index > Lpattern - 1)
    {
        checkWin = checkGuess(sol, guess, res);
        printResult(guess, res);
        int j = 0;
        for (j = 0; j < Lpattern; j++)
        {
            setRGB(res);
            timer_reset_count = 0;
            wait(5, timer_reset_count); // half second display
            setRGB(-1); // turn off after half second
            timer_reset_count = 0;
            wait(5, timer_reset_count); // wait another half second
        }
        guessCount++;
        timer_reset_count = 0;
    }

    if (guessCount > 5)
    {
        printf("Failure: (\r\n\r\n");
        start = 0;
    }
    if (checkWin == 4)
    {
        printf("Win! Total Time: %.1fs, Total Guesses: %d\r\n\r\n", (totalTime/10), guessCount);
        resetColordle();
        return;
    }
}

void resetColordle()
{
    timer_reset_count = 0;
    totalTime = 0;
    button_time = 0;
    guessCount = 0;
    start = 0;
}

void generateRandom()
{
    int i = 0;
    for (i = 0; i < Lpattern; i++)
    {
        sol[i] = rand() % 5;
    }
}

void wait(uint16_t time, uint16_t counter) // waits time in 1/10 seconds
{
    while (counter < time);
}

void GPIOInit() {
    // Complete for Part B
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2);
    GPIO_setAsInputPin(GPIO_PORT_P6, GPIO_PIN1);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN7 | GPIO_PIN6 | GPIO_PIN5 | GPIO_PIN2 | GPIO_PIN3 |GPIO_PIN0);
}

void timerInit(Timer_A_UpModeConfig *config)
{
    // Complete for Part B. Also add interrupt function
    config->clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    config->clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_40;
    config->timerPeriod = 60000;

    config->timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    Timer_A_startCounter(TIMER_A2_BASE, TIMER_A_UP_MODE);
    Timer_A_registerInterrupt(TIMER_A2_BASE,TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT, Timer_ISR);
    Timer_A_configureUpMode(TIMER_A2_BASE, config);
}

int8_t readBumpers()
{
   if (!GPIO_getInputPinValue(GPIO_PORT_P4,  GPIO_PIN0))
   {
        while (!GPIO_getInputPinValue(GPIO_PORT_P4,  GPIO_PIN0))
        {
            setRGB(0);
            __delay_cycles(240e3);
        }

       __delay_cycles(240e3);

       if (GPIO_getInputPinValue(GPIO_PORT_P4,  GPIO_PIN0))
       {
           return 0;
       }
   }

   else if (!GPIO_getInputPinValue(GPIO_PORT_P4,  GPIO_PIN2))
   {
       while (!GPIO_getInputPinValue(GPIO_PORT_P4,  GPIO_PIN2))
       {
            setRGB(1);
           __delay_cycles(240e3);
       }

       __delay_cycles(240e3);

       if (GPIO_getInputPinValue(GPIO_PORT_P4,  GPIO_PIN2))
       {
           return 1;
       }
   }

   else if (!GPIO_getInputPinValue(GPIO_PORT_P4,  GPIO_PIN3))
   {
       while (!GPIO_getInputPinValue(GPIO_PORT_P4,  GPIO_PIN3))
       {
            setRGB(2);
           __delay_cycles(240e3);
       }
       __delay_cycles(240e3);

       if (GPIO_getInputPinValue(GPIO_PORT_P4,  GPIO_PIN3))
       {
           return 2;
       }
   }

   else if (!GPIO_getInputPinValue(GPIO_PORT_P4,  GPIO_PIN5))
   {
       while (!GPIO_getInputPinValue(GPIO_PORT_P4,  GPIO_PIN5))
       {
            setRGB(3);
           __delay_cycles(240e3);
       }
       __delay_cycles(480e3);

       if (GPIO_getInputPinValue(GPIO_PORT_P4,  GPIO_PIN6))
       {
           return 3;
       }
   }

   else if (!GPIO_getInputPinValue(GPIO_PORT_P4,  GPIO_PIN6))
   {
       while (!GPIO_getInputPinValue(GPIO_PORT_P4,  GPIO_PIN6))
       {
           setRGB(4);
           __delay_cycles(240e3);
       }
       __delay_cycles(240e3);

       if(GPIO_getInputPinValue(GPIO_PORT_P4,  GPIO_PIN6))
       {
           return 4;
       }
   }

   else if (!GPIO_getInputPinValue(GPIO_PORT_P4,  GPIO_PIN7))
   {
       while (!GPIO_getInputPinValue(GPIO_PORT_P4,  GPIO_PIN7))
       {
            setRGB(5);
           __delay_cycles(240e3);
       }
       __delay_cycles(240e3);

       if (GPIO_getInputPinValue(GPIO_PORT_P4,  GPIO_PIN7))
       {
           return 5;
       }
   }

   else
   {
       setRGB(-1);
       return -1;
   }
}

void setRGB(int8_t color) {
    // Complete for Part B
    if (color == -1) // off
    {
        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2);
    }
    else if (color == 0) // red
    {
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1 | GPIO_PIN2);
    }
    else if (color == 1) // green
    {
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN2);
    }
    else if (color == 2) // blue
    {
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1 | GPIO_PIN0);
    }
    else if (color == 3) // yellow
    {
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
    }
    else if (color == 4) // megenta
    {
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN2);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
    }
    else if (color == 5) // cyan
    {
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1 | GPIO_PIN2);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
    }
    else if (color == 6) // white
    {
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN0);
    }
}

/**
 * checkGuess is used to check the player's guess against the solution
 * and produce the associated correct positions, incorrect positions, and
 * incorrect colors.
 *
 *  !!! WARNING !!! All of these inputs are expected to be pointers. Arrays are
 *              !!! technically pointers already! They should not have an & in
 *              !!! front of them when passed into the function.
 * Input Parameters:
 *      int8_t * sol: A 4-element array that stores the game solution (input)
 *      int8_t * guess: A 4-element array that stores the player's guess (input)
 *      int8_t * result: A 4-element array that stores the guess correctness result
 *                    This array is really an output of this function. It is
 *                    modified within the function, with the changes persistent
 *                    after the function is complete.
 *                    This array will only have the values of:
 *                      0: Red - Incorrect color
 *                      1: Green - Correct color and position
 *                      3: Yellow - Correct color, incorrect position
 * Outputs:
 *      uint8_t - the number of correct positions. This may be used to determine
 *                is the guess was correct.
 */
uint8_t checkGuess(int8_t *sol,int8_t *guess,int8_t *result) {
    uint8_t _i,_j; // Loop variables. underscores added to avoid conflict with possible globals.
    uint8_t matched[4]; // Array to store if a color in the answer has been matched yet or not
    for(_i=0;_i<Lpattern;_i++){ // set default values of arrays
        result[_i] = 0; // Answer is incorrect (RED)
        matched[_i] = 0; // Guess position is not used yet
    }
    uint8_t Ncorrect = 0; // Number of positions correct.
    // Fist loop through and find corrects
    for(_i=0;_i<Lpattern;_i++){
        if(sol[_i] == guess[_i]){ // If the guess and answer match...
            Ncorrect++; // Increment number of correct guesses
            result[_i] = 1; // 1 for green
            matched[_i] = 1; // 1 for used (can't compare this position again)
        }
    }
    // Now check for correct color, incorrect position
    for(_i=0;_i<Lpattern;_i++){ // Loop through guess positions
        if(result[_i] == 1) continue; // If this position is marked correct, skip it
        for(_j=0;_j<Lpattern;_j++){ // Loop through answer positions, looking for the same color
         // if(i==j) continue; // If checking the same position, skip. This isn't necessary as it would correspond
                               // the correct case and would be skipped by the "checked" array anyway
            if(matched[_j]) continue; // If this answer color is already taken by a correct or close, skip it
            if(guess[_i] == sol[_j]){ // If the colors are the same (correct color, incorrect position)
                result[_i] = 3; // 3 for yellow
                matched[_j] = 1; // 1 for used (can't compare this position again)
            }
        }
    }
    return Ncorrect; // return number of correct positions
}

/*
 * printResult will take the players guess and the checked result and print them
 * in the necessary format on the terminal. The colors in the player's guess will be
 * printed first, using the first letter of each color. Afterwards the result of
 * the guess is printed using the characters:
 *              $ - correct color and position (Green result)
 *              O - correct color, incorrect position (Yellow result)
 *              X - incorrect color (Red result)
 *
 *  !!! WARNING !!! Both of these inputs are expected to be pointers. Arrays are
 *                  technically pointers already! They should not have an & in
 *                  front of them when passed into the function.
 * Input Parameters:
 *      int8_t * guess: A 4-element array that stores the player's guess (input)
 *      int8_t * result: A 4-element array that stores the guess correctness result
 */
void printResult(int8_t *guess,int8_t *result){
    uint8_t _i = 0; // loop variable
    for(_i=0;_i<Lpattern;_i++){
        switch(guess[_i]){
        case 0: putchar('R'); break;
        case 1: putchar('G'); break;
        case 2: putchar('B'); break;
        case 3: putchar('Y'); break;
        case 4: putchar('M'); break;
        case 5: putchar('C'); break;
        }
    }
    putchar(' '); // put a space in
    for(_i=0;_i<Lpattern;_i++){
        switch(result[_i]){
        case 0: putchar('X'); break;
        case 3: putchar('O'); break;
        case 1: putchar('$'); break;
        }
    }
    putchar('\r');putchar('\n');
}

// Add interrupt functions last so they are easy to find
void Timer_ISR()
{
    timer_reset_count++;
    button_time++;
    totalTime++;
    GPIO_toggleOutputOnPin(GPIO_PORT_P1,GPIO_PIN0);
    Timer_A_clearInterruptFlag(TIMER_A2_BASE);

}
