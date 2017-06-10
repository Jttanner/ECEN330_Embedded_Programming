/*
 * clockDisplay.c
 *
 *  Created on: May 24, 2017
 *      Author: jttanner
 */

#include "clockDisplay.h"
#include "supportFiles/display.h"
#include "math.h"
#include "globals.h"
#include <stdio.h>
#include "supportFiles/utils.h"



#define TEXT_SIZE 6
#define NUMBER_WIDTH TEXT_SIZE * DISPLAY_CHAR_WIDTH
#define NUMBER_HEIGHT TEXT_SIZE * DISPLAY_CHAR_HEIGHT
#define TRIANGLE_SIDE_LENGTH 2*NUMBER_WIDTH
#define NUMBER_DRAW_Y NUMBER_HEIGHT/2
//320/3
#define HOUR_CORNER_COORD_X DISPLAY_WIDTH/3
#define MINUTE_CORNER_COORD_X 2*HOUR_CORNER_COORD_X
#define SECOND_CORNER_COORD_X 3*HOUR_CORNER_COORD_X
#define ALL_CORNER_COORD_Y DISPLAY_HEIGHT/2

#define INITIAL_X 0
#define INITIAL_Y 0
#define TRIANGLE_X_OFFSET 0
#define TRIANGLE_Y_OFFSET 0
#define TOP_TRIANGLE_BASE_Y (ALL_CORNER_COORD_Y - NUMBER_HEIGHT - NUMBER_HEIGHT/2)
#define BOTTOM_TRIANGLE_BASE_Y (ALL_CORNER_COORD_Y + NUMBER_HEIGHT + NUMBER_HEIGHT/2)
//triangle tip gotten from helper function

//x0 is furthest right
//x1 is furthest left
//x2 is top tip
#define HOUR_TRIANGLE_X0  HOUR_CORNER_COORD_X - TEXT_SIZE
#define HOUR_TRIANGLE_X1 HOUR_TRIANGLE_X0 - TRIANGLE_SIDE_LENGTH
#define HOUR_TRIANGLE_X2 (HOUR_TRIANGLE_X0 + HOUR_TRIANGLE_X1)/2


#define MINUTE_TRIANGLE_X0 HOUR_TRIANGLE_X0 + HOUR_CORNER_COORD_X
#define MINUTE_TRIANGLE_X1 HOUR_TRIANGLE_X1 + HOUR_CORNER_COORD_X
#define MINUTE_TRIANGLE_X2 HOUR_TRIANGLE_X2 + HOUR_CORNER_COORD_X

#define SECOND_TRIANGLE_X0 MINUTE_TRIANGLE_X0 + HOUR_CORNER_COORD_X
#define SECOND_TRIANGLE_X1 MINUTE_TRIANGLE_X1 + HOUR_CORNER_COORD_X
#define SECOND_TRIANGLE_X2 MINUTE_TRIANGLE_X2 + HOUR_CORNER_COORD_X

#define HOUR_NUMBER_PRINT_LOCATION HOUR_TRIANGLE_X2
#define MINUTE_NUMBER_PRINT_LOCATION MINUTE_TRIANGLE_X2
#define SECOND_NUMBER_PRINT_LOCATION SECOND_TRIANGLE_X2

#define OFFSET_EDGE_OF_QUADRANT_TO_CENTER 35

#define LEFT_HOUR_PRINT_CURSOR_LOCATION HOUR_TRIANGLE_X2 - NUMBER_WIDTH
#define RIGHT_HOUR_PRINT_CURSOR_LOCATION HOUR_TRIANGLE_X2
#define LEFT_MINUTE_PRINT_CURSOR_LOCATION MINUTE_TRIANGLE_X2 - NUMBER_WIDTH
#define RIGHT_MINUTE_PRINT_CURSOR_LOCATION MINUTE_TRIANGLE_X2
#define LEFT_SECOND_PRINT_CURSOR_LOCATION SECOND_TRIANGLE_X2 - NUMBER_WIDTH
#define RIGHT_SECOND_PRINT_CURSOR_LOCATION SECOND_TRIANGLE_X2

#define LEFT_HOUR_INITIAL_VALUE 1
#define RIGHT_HOUR_INITIAL_VALUE 2
#define LEFT_MINUTE_INITIAL_VALUE 5
#define RIGHT_MINUTE_INITIAL_VALUE 9
#define LEFT_SECOND_INITIAL_VALUE 5
#define RIGHT_SECOND_INITIAL_VALUE 9

#define ONE_IN_TENS_PLACE_FOR_HOUR 1

#define HOUR_MAX 12
#define MINUTE_AND_SECOND_MAX 59
#define IS_MAX 1
#define IS_MIN 2
#define IS_NOT_MAX 0
#define MIN_CLOCK_VALUE 0

#define ROLLOVER_MINUTES 60
#define SQUARED_POWER 2

#define INC_HOURS_X_BORDER HOUR_CORNER_COORD_X
#define INC_HOURS_Y_BORDER 120
#define INC_MINUTES_X_BORDER MINUTE_CORNER_COORD_X
#define INC_MINUTES_Y_BORDER 120
#define INC_SECONDS_X_BORDER SECOND_CORNER_COORD_X
#define INC_SECONDS_Y_BORDER 120


enum numberDisplayLocation { //enums to help the updateNumberDisplay know what to update
    leftHourLocation, //will direct to print the first digit of hours
    rightHourLocation, // "" the right digit
    leftMinuteLocation,// for the left digit of minutes
    rightMinuteLocation, //for the right digit of minutes
    leftSecondLocation, //for the left second of minutes
    rightSecondLocation //for the right second of minutes
};


/**
 * HELPER FUNCTION
 * given the text size, calculate triangle height
 */
uint16_t CalculateTriangleHeight(){
    //use pythag theorem to find the height of the equilateral triangle
    //casted to an int to ensure the right value is returned since sqrt returns a double
    return ((uint16_t)(sqrt(pow(TRIANGLE_SIDE_LENGTH, SQUARED_POWER)) - sqrt(pow(NUMBER_WIDTH, SQUARED_POWER))));
}


/**
 * HELPER FUNCTION
 * Draws the triangles above and below the numbers of the clock
 */
void displayAllTriangles(){
    //top left quadrant triangle draw:
       display_fillTriangle(HOUR_TRIANGLE_X0, TOP_TRIANGLE_BASE_Y,
                            HOUR_TRIANGLE_X1, TOP_TRIANGLE_BASE_Y,
                            HOUR_TRIANGLE_X2, TOP_TRIANGLE_BASE_Y - CalculateTriangleHeight(),
                            DISPLAY_GREEN);
       //bottom left quadrant triangle draw:
       display_fillTriangle(HOUR_TRIANGLE_X0, BOTTOM_TRIANGLE_BASE_Y,
                            HOUR_TRIANGLE_X1, BOTTOM_TRIANGLE_BASE_Y,
                            HOUR_TRIANGLE_X2, BOTTOM_TRIANGLE_BASE_Y + CalculateTriangleHeight(),
                            DISPLAY_GREEN);

       //top middle quadrant triangle draw:
       display_fillTriangle(MINUTE_TRIANGLE_X0, TOP_TRIANGLE_BASE_Y,
                            MINUTE_TRIANGLE_X1, TOP_TRIANGLE_BASE_Y,
                            MINUTE_TRIANGLE_X2, TOP_TRIANGLE_BASE_Y - CalculateTriangleHeight(),
                            DISPLAY_GREEN);
       //bottom middle quadrant triangle draw:
       display_fillTriangle(MINUTE_TRIANGLE_X0, BOTTOM_TRIANGLE_BASE_Y,
                            MINUTE_TRIANGLE_X1, BOTTOM_TRIANGLE_BASE_Y,
                            MINUTE_TRIANGLE_X2, BOTTOM_TRIANGLE_BASE_Y + CalculateTriangleHeight(),
                            DISPLAY_GREEN);

       //top right quadrant triangle draw:
       display_fillTriangle(SECOND_TRIANGLE_X0, TOP_TRIANGLE_BASE_Y,
                            SECOND_TRIANGLE_X1, TOP_TRIANGLE_BASE_Y,
                            SECOND_TRIANGLE_X2, TOP_TRIANGLE_BASE_Y - CalculateTriangleHeight(),
                            DISPLAY_GREEN);
       //bottom right quadrant triangle draw
       display_fillTriangle(SECOND_TRIANGLE_X0, BOTTOM_TRIANGLE_BASE_Y,
                            SECOND_TRIANGLE_X1, BOTTOM_TRIANGLE_BASE_Y,
                            SECOND_TRIANGLE_X2, BOTTOM_TRIANGLE_BASE_Y + CalculateTriangleHeight(),
                            DISPLAY_GREEN);
}

/**
 * HELPER FUNCTION
 * Draws the colons to the screen
 */
void displayColons(){
    //set the cursor at the correct location for the left colon and print it to the display
    display_setCursor(HOUR_CORNER_COORD_X, ALL_CORNER_COORD_Y - NUMBER_DRAW_Y );
    display_println(":");

    //set the cursor at the correct location for the right colon and print it to the display
    display_setCursor(MINUTE_CORNER_COORD_X, ALL_CORNER_COORD_Y - NUMBER_DRAW_Y);
    display_println(":");
}


/**
 * HELPER FUNCTION
 * Reduce code repetition, the second and minute displays behave the same
 */
void writeOneMinuteOrSecondNumber(uint16_t cursor_x, uint8_t cursor_y, uint8_t numberToInsert){
    //clear the previous digit at the correct location
    display_fillRect(cursor_x, cursor_y, NUMBER_WIDTH, NUMBER_HEIGHT, DISPLAY_BLACK);
    //place the cursor at the correct location
    display_setCursor(cursor_x, cursor_y);
    //print the digit to the display at the cursor's location
    display_println(numberToInsert);
}

/**
 * HELPER FUNCTION
 * Updates the display of a number
 */
void updateNumberDisplay(uint8_t numberToInsert, numberDisplayLocation locationToUpdate){
    switch(locationToUpdate){
    case leftHourLocation: // update the left hour digit display
        //is different than the other case since it can only be a 1 or blank.
        //erase the previous number
        display_fillRect(LEFT_HOUR_PRINT_CURSOR_LOCATION, ALL_CORNER_COORD_Y - NUMBER_DRAW_Y,
                         NUMBER_WIDTH, NUMBER_HEIGHT, DISPLAY_BLACK);
        //place where we want to print
        display_setCursor(LEFT_HOUR_PRINT_CURSOR_LOCATION, ALL_CORNER_COORD_Y - NUMBER_DRAW_Y);
        //only print to the screen for the first hour spot if it is a '1'
        if(numberToInsert == ONE_IN_TENS_PLACE_FOR_HOUR) display_println(numberToInsert);
        break;
    case rightHourLocation:
        //update the right hour digit display
        //uses the same format as the seconds and minutes
        writeOneMinuteOrSecondNumber(RIGHT_HOUR_PRINT_CURSOR_LOCATION,
                                     ALL_CORNER_COORD_Y - NUMBER_DRAW_Y, numberToInsert);
        break;
    case leftMinuteLocation: //update the left minute digit display
        writeOneMinuteOrSecondNumber(LEFT_MINUTE_PRINT_CURSOR_LOCATION,
                                     ALL_CORNER_COORD_Y - NUMBER_DRAW_Y, numberToInsert);
        break;
    case rightMinuteLocation: //update the right minute digit display
        writeOneMinuteOrSecondNumber(RIGHT_MINUTE_PRINT_CURSOR_LOCATION,
                                     ALL_CORNER_COORD_Y - NUMBER_DRAW_Y, numberToInsert);
        break;
    case leftSecondLocation: //update the left second digit display
        writeOneMinuteOrSecondNumber(LEFT_SECOND_PRINT_CURSOR_LOCATION,
                                     ALL_CORNER_COORD_Y - NUMBER_DRAW_Y, numberToInsert);
        break;
    case rightSecondLocation: //update the right second digit display
        writeOneMinuteOrSecondNumber(RIGHT_SECOND_PRINT_CURSOR_LOCATION,
                                     ALL_CORNER_COORD_Y - NUMBER_DRAW_Y,
                                     numberToInsert);
        break;
    }
    //globals_updatePreviousValues();
}

/**
 * HELPER FUNCTION
 * Initialize the numbers on display to 12:59:59
 */
void initializeNumbers(){
    globals_initialize(); //initialize the global variables
    updateNumberDisplay(LEFT_HOUR_INITIAL_VALUE, leftHourLocation); //prints the first hour number
    updateNumberDisplay(RIGHT_HOUR_INITIAL_VALUE, rightHourLocation); //prints the second hour number
    updateNumberDisplay(LEFT_MINUTE_INITIAL_VALUE, leftMinuteLocation); //prints the first minute number
    updateNumberDisplay(RIGHT_MINUTE_INITIAL_VALUE, rightMinuteLocation); //prints the second minute number
    updateNumberDisplay(LEFT_SECOND_INITIAL_VALUE, leftSecondLocation); //prints the first second number
    updateNumberDisplay(RIGHT_SECOND_INITIAL_VALUE, rightSecondLocation); //prints the second second number
}

    //initialize the clockDisplay
void clockDisplay_init(){
    //initialize the screen, and then erase whatever might already be there from before
    display_init();
    display_fillRect(INITIAL_X, INITIAL_Y, DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_BLACK);

    //set the text to our desired size
    display_setTextSize(TEXT_SIZE);
    //set the desired text color
    display_setTextColor(DISPLAY_GREEN);

    //draw whatever does not update

    //equilateral triangles
    //one triangle is the width of two chars
    displayAllTriangles();

    //draw the colons to the screen
    displayColons();

    //initialize to 12:59:59
    initializeNumbers();
}






void checkAndUpdateNumberDisplay(){
    if(globals_getLeftHourDigit() != globals_getPreviousLeftHourDigit()){ //compare current and previous Left Hour Digit Global Variables
        updateNumberDisplay(globals_getLeftHourDigit(), leftHourLocation); //if different, update the screen
    }
    if(globals_getRightHourDigit() != globals_getPreviousRightHourDigit()){ //same for right hour digit
        updateNumberDisplay(globals_getRightHourDigit(), rightHourLocation); //update display
    }
    if(globals_getLeftMinuteDigit() != globals_getPreviousLeftMinuteDigit()){ //same for left minute digit
        updateNumberDisplay(globals_getLeftMinuteDigit(), leftMinuteLocation); //update display
    }
    if(globals_getRightMinuteDigit() != globals_getPreviousRightMinuteDigit()){ //same for right minute digit
        updateNumberDisplay(globals_getRightMinuteDigit(), rightMinuteLocation); //update display
    }
    if(globals_getLeftSecondDigit() != globals_getPreviousLeftSecondDigit()){ //same for the left second digit
        updateNumberDisplay(globals_getLeftSecondDigit(), leftSecondLocation); //update display
    }
    if(globals_getRightSecondDigit() != globals_getPreviousRightSecondDigit()){ //same for the right second digit
        updateNumberDisplay(globals_getRightSecondDigit(), rightSecondLocation); //update display
    }

}

// Updates the time display with latest time
void clockDisplay_updateTimeDisplay(bool forceUpdateAll){
    if (forceUpdateAll){
        //if forceUpdateAll is true, we have to update every single digit
        //update:
        updateNumberDisplay(globals_getLeftHourDigit(), leftHourLocation); //left hour digit
        updateNumberDisplay(globals_getRightHourDigit(), rightHourLocation); //right hour digit
        updateNumberDisplay(globals_getLeftMinuteDigit(), leftMinuteLocation); //left minute digit
        updateNumberDisplay(globals_getRightMinuteDigit(), rightMinuteLocation); //right minute digit
        updateNumberDisplay(globals_getLeftSecondDigit(), leftSecondLocation); //left second digit
        updateNumberDisplay(globals_getRightSecondDigit(), rightSecondLocation); //right second digit
        checkAndUpdateNumberDisplay(); //update the display
    } else{
        checkAndUpdateNumberDisplay(); //update the display
    }
}



// Reads the touched coordinates and performs the increment or decrement,
// depending upon the touched region.
void clockDisplay_performIncDec(){
    int16_t x, y; //variables used to store touchpoint data
    uint8_t z; //from the next function
    display_getTouchedPoint(&x, &y, &z); //get the touch data and store them into x, y, and z
    if(x < INC_HOURS_X_BORDER){
        if (y < INC_HOURS_Y_BORDER){
            //inc hours
            globals_setCurrentHours(globals_getCurrentHours() + 1);
            checkAndUpdateNumberDisplay();
        } else{
            //dec hours
            globals_setCurrentHours(globals_getCurrentHours() - 1);
            checkAndUpdateNumberDisplay();
        }
    } else if(x < INC_MINUTES_X_BORDER){
        if (y < INC_MINUTES_Y_BORDER){
            //inc minutes
            globals_setCurrentMinutes(globals_getCurrentMinutes() + 1);
            checkAndUpdateNumberDisplay();
        } else{
            //dec minutes
            globals_setCurrentMinutes(globals_getCurrentMinutes() - 1);
            checkAndUpdateNumberDisplay();
        }
    }  else if(x < INC_SECONDS_X_BORDER){
        if (y < INC_SECONDS_Y_BORDER){
            //inc seconds
            globals_setCurrentSeconds(globals_getCurrentSeconds() + 1);
            checkAndUpdateNumberDisplay();
        } else{
            //dec seconds
            globals_setCurrentSeconds(globals_getCurrentSeconds() - 1);
            checkAndUpdateNumberDisplay();
        }
    }
}

/**
 * HELPER FUNCTION
 * Checks if all fields are needed to be updated
 * (only at 00:00:00 being decremented or 12:59:59 being incremented)
 */
uint8_t checkUpdateAll(){
    //Check each of the current values vs their maximum values
    if (globals_getCurrentHours() == HOUR_MAX &&  //hours
        globals_getCurrentMinutes() == MINUTE_AND_SECOND_MAX &&  //minutes
        globals_getCurrentSeconds() == MINUTE_AND_SECOND_MAX){ //seconds
        return IS_MAX; //return that it was the max
    } //check each value vs their minimum values
    else if(globals_getCurrentHours() == MIN_CLOCK_VALUE && //hours
        globals_getCurrentMinutes() == MIN_CLOCK_VALUE && //minutes
        globals_getCurrentSeconds() == MIN_CLOCK_VALUE){ //seconds
        return IS_MIN; //return that it was the minimum value
    } else{
        return IS_NOT_MAX; //return that it was neither max nor min
    }

}



// Advances the time forward by 1 second and update the display.
void clockDisplay_advanceTimeOneSecond(){
    if (checkUpdateAll() == IS_MAX) //checks if we need to update all digits
    {
        //incrementSeconds();
        globals_progressTime();
        clockDisplay_updateTimeDisplay(IS_MAX);
    }else{
        //incrementSeconds();
        globals_progressTime();
        clockDisplay_updateTimeDisplay(IS_NOT_MAX); //print the new current second value to the display
    }

}

#define ONE_MS 100

// Run a test of clock-display functions.
void clockDisplay_runTest(){
    for (int i = 0; i < HOUR_MAX; ++i){ //interate enough times to hit each value of an hour
        utils_msDelay(ONE_MS); //wait one millisecond
        globals_setCurrentHours(globals_getCurrentHours() + 1); //increment hours by one
        checkAndUpdateNumberDisplay(); //update the dsplay
    }
    for (int i = 0; i < HOUR_MAX; ++i){ //iterate enough times to hit every hour value
        utils_msDelay(ONE_MS); //wait a millisecond
        globals_setCurrentHours(globals_getCurrentHours() - 1); //decrement hours
        checkAndUpdateNumberDisplay(); //update to the screen
        }

    for (int i = 0; i < MINUTE_AND_SECOND_MAX; ++i){ //iterate enough to hit every minute
        utils_msDelay(ONE_MS); //wait one ms
        globals_setCurrentMinutes(globals_getCurrentMinutes() + 1); //increment minutes
        checkAndUpdateNumberDisplay(); //update
        }
    for (int i = 0; i < MINUTE_AND_SECOND_MAX; ++i){ //iterate enough to hit every minute
        utils_msDelay(ONE_MS); //wait one ms
        globals_setCurrentMinutes(globals_getCurrentMinutes() - 1);checkAndUpdateNumberDisplay(); //decrement minutes
        checkAndUpdateNumberDisplay(); //update display
        }
    for (int i = 0; i < MINUTE_AND_SECOND_MAX; ++i){ //iterate enough to hit each second
        utils_msDelay(ONE_MS); //wait one ms
        globals_setCurrentSeconds(globals_getCurrentSeconds() + 1);checkAndUpdateNumberDisplay(); //increment seconds
        checkAndUpdateNumberDisplay(); //update display
        }

    for (int i = 0; i < MINUTE_AND_SECOND_MAX; ++i){ //iterate enough to hit each second
        utils_msDelay(ONE_MS); // wait one mas
        globals_setCurrentSeconds(globals_getCurrentSeconds() - 1); //decrement seconds
        checkAndUpdateNumberDisplay(); //update dsiplay
        }
    while(1){ //loop through to test the clock until we kill it
        utils_msDelay(ONE_MS); //wait one ms
        clockDisplay_advanceTimeOneSecond(); //progress time one second
    }



}



