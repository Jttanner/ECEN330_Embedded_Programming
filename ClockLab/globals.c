/*
 * globals.c
 *
 *  Created on: May 24, 2017
 *      Author: jttanner
 */

#include "globals.h"
#include <stdio.h>


#define GET_REMAINDER_FROM_MODULUS_OF_TEN 10
#define GET_INTEGER_FROM_DIVISION_BY_TEN 10
#define MAX_SECONDS_AND_MINUTES_VALUE 59
#define MAX_HOUR_VALUE 12
#define MIN_HOUR_VALUE 1

#define MINUTE_AND_SECOND_POSITIVE_ROLLOVER 60
#define MINUTE_AND_SECOND_NEGATIVE_ROLLOVER 255
#define MINUTE_AND_SECOND_MINIMUM 0
#define HOUR_NEGATIVE_ROLLOVER 0
#define HOUR_POSITIVE_ROLLOVER 13

//Global variables to use throughout clockControl and clockDisplay

static uint8_t currentHours; //variable holding current hour data
static uint8_t currentMinutes; //variable holding current minute data
static uint8_t currentSeconds; //variable holding current second data

static uint8_t previousHours; //variable holding old hour data
static uint8_t previousMinutes; //variable holding old minute data
static uint8_t previousSeconds; //variable holding old second data

void globals_initialize(){
    currentHours = MAX_HOUR_VALUE; //initialize global hour variables
    previousHours = MAX_HOUR_VALUE;
    currentMinutes = MAX_SECONDS_AND_MINUTES_VALUE; //initialize global minute variables
    previousMinutes = MAX_SECONDS_AND_MINUTES_VALUE;
    currentSeconds = MAX_SECONDS_AND_MINUTES_VALUE; // initialize global second variables
    previousSeconds = MAX_SECONDS_AND_MINUTES_VALUE;
}

//update previous variables to current variables before they change
void globals_updatePreviousValues(){
    previousHours = currentHours; //update previous hours
    previousMinutes = currentMinutes; //update previous minutes
    previousSeconds = currentSeconds; //update previous seconds
}

//stop any value from going over what it should be able to
//60 for minutes and seconds, 12 for hours
void rollIncrementOverflow(){
    if (currentSeconds == MINUTE_AND_SECOND_POSITIVE_ROLLOVER){ //check if seconds rolled over
        currentSeconds = MINUTE_AND_SECOND_MINIMUM; //if so, fix it
    }
    if (currentMinutes == MINUTE_AND_SECOND_POSITIVE_ROLLOVER){ //check if minutes rolled over
        currentMinutes = MINUTE_AND_SECOND_MINIMUM; //if so, fix it
    }
    if (currentHours >= HOUR_POSITIVE_ROLLOVER){ //check if hours rolled over
        currentHours = MIN_HOUR_VALUE; //if so, fix it
    }
}

//stop any value from going under what it should be able to
//none can go under 0
void rollDecrementOverflow(){
    if (currentSeconds == MINUTE_AND_SECOND_NEGATIVE_ROLLOVER){ //check if seconds rolled under
        currentSeconds = MAX_SECONDS_AND_MINUTES_VALUE; //if so, fix it
    }
    if (currentMinutes == MINUTE_AND_SECOND_NEGATIVE_ROLLOVER){ //check if minutes rolled under
        currentMinutes = MAX_SECONDS_AND_MINUTES_VALUE; //if so, fix it
    }
    if (currentHours <= HOUR_NEGATIVE_ROLLOVER){ //check if hours rolled under
        currentHours = MAX_HOUR_VALUE; //if so, fix it
    }
}

uint8_t globals_getCurrentHours(){ //get the current hour value
    return currentHours;
}

uint8_t globals_getCurrentMinutes(){ //get the current minute value
    return currentMinutes;
}

uint8_t globals_getCurrentSeconds(){ //get the current second value
    return currentSeconds;
}


uint8_t globals_getPreviousHours(){ //get the previous hour value
    return previousHours;
}

uint8_t globals_getPreviousMinutes(){ //get the previous minute value
    return previousMinutes;
}
uint8_t globals_getPreviousSeconds(){ //get the previous second value
    return previousSeconds;
}



void globals_setCurrentHours(uint8_t newHours){ //set a new current hour value
    globals_updatePreviousValues();//update previous time variables
    currentHours = newHours; //update current hour
    rollIncrementOverflow(); //prevent bad numbers
    rollDecrementOverflow(); ///by checking overflow
}

void globals_setCurrentMinutes(uint8_t newMinutes){ //set a new current minute value
    globals_updatePreviousValues(); //update previous time variables
    currentMinutes = newMinutes; //update current minutes
    rollIncrementOverflow(); //prevent bad numbers
    rollDecrementOverflow(); //by checking overflow
}

void globals_setCurrentSeconds(uint8_t newSeconds){ //set a new current second value
    globals_updatePreviousValues(); //update previous time variables
    currentSeconds = newSeconds; //update current seconds
    rollIncrementOverflow(); //prevent bad numbers
    rollDecrementOverflow(); //by checking overflow
}

    //advances time like a clock, rolling over seconds to minutes and minutes to hours
void globals_progressTime(){
    globals_updatePreviousValues(); //update previous variables
    if (currentSeconds == MAX_SECONDS_AND_MINUTES_VALUE){ //if seconds is at 59 we need to increase minutes
        if (currentMinutes == MAX_SECONDS_AND_MINUTES_VALUE){ //if minutes is at 59 we need to increas hours
            currentHours++; //increment hours
        }
        currentMinutes++; //increment minutes
    }
    currentSeconds++; //increment seconds
    rollIncrementOverflow(); //check for overflow
}



uint8_t globals_getLeftHourDigit(){
    return currentHours / GET_INTEGER_FROM_DIVISION_BY_TEN; //extract the left digit from current hours
}

uint8_t globals_getRightHourDigit(){
    return currentHours % GET_REMAINDER_FROM_MODULUS_OF_TEN; //extract the right digit from current hours
}
uint8_t globals_getLeftMinuteDigit(){
    return currentMinutes / GET_INTEGER_FROM_DIVISION_BY_TEN; //extract the left digit from current minutes
}
uint8_t globals_getRightMinuteDigit(){
    return currentMinutes % GET_REMAINDER_FROM_MODULUS_OF_TEN; //extract the right digit from current minutes
}
uint8_t globals_getLeftSecondDigit(){
    return currentSeconds / GET_INTEGER_FROM_DIVISION_BY_TEN; //extract the left digit from current seconds
}
uint8_t globals_getRightSecondDigit(){
    return currentSeconds % GET_REMAINDER_FROM_MODULUS_OF_TEN; //extract the right digit from current seconds
}

uint8_t globals_getPreviousLeftHourDigit(){
    return previousHours / GET_INTEGER_FROM_DIVISION_BY_TEN; //extract the left digit from previous hours
}

uint8_t globals_getPreviousRightHourDigit(){
    return previousHours % GET_REMAINDER_FROM_MODULUS_OF_TEN; //extract the right digit from previous hours
}
uint8_t globals_getPreviousLeftMinuteDigit(){
    return previousMinutes / GET_INTEGER_FROM_DIVISION_BY_TEN; //extract the left digit from previous minutes
}
uint8_t globals_getPreviousRightMinuteDigit(){
    return previousMinutes % GET_REMAINDER_FROM_MODULUS_OF_TEN; //extract the right digit from previous minutes
}
uint8_t globals_getPreviousLeftSecondDigit(){
    return previousSeconds / GET_INTEGER_FROM_DIVISION_BY_TEN; //extract the left digit from previous seconds
}
uint8_t globals_getPreviousRightSecondDigit(){
    return previousSeconds % GET_REMAINDER_FROM_MODULUS_OF_TEN; //extract the right digit from previous seconds
}
