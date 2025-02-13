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

#include<stdint.h>

// Add function prototypes here as needed.

typedef struct _homework_t {

    float average;      // The struct has a "field" for each value listed

    float stdev;        // in the "Homework Gradebook" table.

    uint8_t min;        // Note that the fields can be all different types

    uint8_t max;

    uint16_t subs;

} homework_t;


// Add global variables here as needed.

homework_t h1;

homework_t array[5];


float remove_min_from_avg(homework_t *homework);



int main() {    //// Main Function ////

  

    // Add local variables here as needed.

    h1.average = 94.2;

    h1.stdev = 10.8;

    h1.min = 55;

    h1.max = 100;

    h1.subs = 109;


    printf("Homework 1 Stats\r\n"

           "    Average: %.2f\r\n"

           "  Std. Dev.: %.2f\r\n"

           "    Minimum: %u\r\n"

           "    Maximum: %u\r\n"

           "Submissions: %u\r\n",

           h1.average,h1.stdev,h1.min,

           h1.max,h1.subs);


    array[0].average = h1.average;

    array[0].stdev = h1.stdev;

    array[0].min = h1.min;

    array[0].max = h1.max;

    array[0].subs = h1.subs;


    array[1].average = 76.7;

    array[1].stdev = 12.6;

    array[1].min = 40;

    array[1].max = 100;

    array[1].subs = 106;


    uint8_t i, j;

    for (i = 0; i < 2; i++)

    {

        printf("Homework %u Stats\r\n"

                   "    Average: %.2f\r\n"

                   "  Std. Dev.: %.2f\r\n"

                   "    Minimum: %u\r\n"

                   "    Maximum: %u\r\n"

                   "Submissions: %u\r\n",

                   i + 1,array[i].average,array[i].stdev,array[i].min,

                   array[i].max,array[i].subs);

    }


    printf("Homework 1 New Average: %.2f\r\n", remove_min_from_avg(&h1));

    printf("Homework 2 New Average: %.2f\r\n", remove_min_from_avg(&array[1]));



    // We always call the sysInit function first to set up the 

    // microcontroller for how we are going to use it.

    sysInit();


    // Place initialization code (or run-once) code here


    while(1){  

        // Place code that runs continuously in here


    }   

}    //// Main Function ////  


// Add function declarations here as needed

// Declaration. Add below main function

// This function calculates the average of a homework without the minimum grade.

// The function will return the value of the new average.

float remove_min_from_avg(homework_t *homework){

    float homework_sum = homework->average*homework->subs;

    homework_sum -= homework->min;

    return homework_sum/(homework->subs-1);

}

// Add interrupt functions last so they are easy to find


