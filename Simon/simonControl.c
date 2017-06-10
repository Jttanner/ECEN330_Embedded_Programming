/*
 * simonControl.c
 *
 *  Created on: Jun 8, 2017
 *      Author: jontt
 */

#include "flashSequence.h"
#include "simonControl.h"
#include "verifySequence.h"
#include "buttonHandler.h"
#include "xparameters.h"
#include <stdio.h>
#include "supportFiles/leds.h"
#include "supportFiles/globalTimer.h"
#include "supportFiles/interrupts.h"
#include <stdbool.h>
#include <stdint.h>
#include "supportFiles/display.h"
#include "supportFiles/utils.h"
#include "simonDisplay.h"
#include "src/lab2/buttons.h"
#include "globals.h"
#include <stdlib.h>
#include "src/intervalTimerLab/intervalTimer.h"


//for reference from the simon display
// Width, height of the simon "buttons"
#define SIMON_DISPLAY_BUTTON_WIDTH 60
#define SIMON_DISPLAY_BUTTON_HEIGHT 60
#define SIMON_DISPLAY_BUTTON_SIDE_BUFFER 50
#define SIMON_DISPLAY_BUTTON_VERTICAL_BUFFER 30

// WIdth, height of the simon "squares.
// Note that the video shows the squares as larger but you
// can use this smaller value to make the game easier to implement speed-wise.
#define SIMON_DISPLAY_SQUARE_WIDTH  120
#define SIMON_DISPLAY_SQUARE_HEIGHT 120

// Given coordinates from the touch pad, computes the region number.

// The entire touch-screen is divided into 4 rectangular regions, numbered 0 - 3.
// Each region will be drawn with a different color. Colored buttons remind
// the user which square is associated with each color. When you press
// a region, computeRegionNumber returns the region number that is used
// by the other routines.
/*
|----------|----------|
|          |          |
|    0     |     1    |
|  (RED)   | (YELLOW) |
-----------------------
|          |          |
|     2    |    3     |
|  (BLUE)  |  (GREEN) |
-----------------------
*/

// These are the definitions for the regions.
#define SIMON_DISPLAY_REGION_0 0
#define SIMON_DISPLAY_REGION_1 1
#define SIMON_DISPLAY_REGION_2 2
#define SIMON_DISPLAY_REGION_3 3
#define ZERO_CORNER_COORDINATES 0

#define HALF_DISPLAY_WIDTH DISPLAY_WIDTH/2
#define HALF_DISPLAY_HEIGHT DISPLAY_HEIGHT/2



enum simonControl_states{
    sc_initState,
    sc_titleScreenState,
    sc_setGlobalsState,
    sc_flashSequenceState,
    sc_verifySequenceState,
    sc_displayResultState,
    sc_newLevelState,
    sc_bestLevelDisplayState
}sc_currentState = sc_initState;

#define MESSAGE_X (display_width()/10)
#define MESSAGE_Y (display_height()/3)
#define TITLE_MAIN_TEXT "SIMON\n"
#define TITLE_MAIN_TEXT_SIZE 8
#define TITLE_SUB_TEXT "     Touch to start.\n"
#define TITLE_SUB_TEXT_SIZE 2

#define TIME_OUT_TEXT "You took too long!"
#define BAD_INPUT_TEXT "Bad Input!"
#define WIN_TEXT "You win! Touch again for next level."

#define NEW_GAME_WAIT_TIMER_START 0
#define NEW_GAME_WAIT_TIMER_MAX 20

#define BEST_LEVEL_STRING_BUFFER 20
#define BUTTON_DIMENSION 60

#define CORNER_X 0
#define CORNER_Y 0

static uint8_t newGameWaitTimer = 0; //timer for timing how long we should wait


//extra display functions to supplement simonDisplay
void simonControl_eraseButton(uint8_t regionNumber){
    //display is 320 x 240 --> each region is 160x120
    //each button is 60x60 --> 100x60 space left over --> buffer space: 50 on each side, 30 above and below
    switch(regionNumber){ //choose which region to draw based on the parameter passed in
    case SIMON_DISPLAY_REGION_0: //draw a button on region 0
        //fill in with new rectangle
        display_fillRect(SIMON_DISPLAY_BUTTON_SIDE_BUFFER, SIMON_DISPLAY_BUTTON_VERTICAL_BUFFER, BUTTON_DIMENSION, BUTTON_DIMENSION, DISPLAY_BLACK);
        break;
    case SIMON_DISPLAY_REGION_1: //draw a button on region 1
        //fill in with new rectangle
        display_fillRect(SIMON_DISPLAY_BUTTON_SIDE_BUFFER + HALF_DISPLAY_WIDTH, SIMON_DISPLAY_BUTTON_VERTICAL_BUFFER,
                BUTTON_DIMENSION, BUTTON_DIMENSION, DISPLAY_BLACK);
        break;
    case SIMON_DISPLAY_REGION_2: //draw a button on region 2
        //fill in with new rectangle
        display_fillRect(SIMON_DISPLAY_BUTTON_SIDE_BUFFER, SIMON_DISPLAY_BUTTON_VERTICAL_BUFFER + HALF_DISPLAY_HEIGHT,
                BUTTON_DIMENSION, BUTTON_DIMENSION, DISPLAY_BLACK);
        break;
    case SIMON_DISPLAY_REGION_3: //draw a button on region 3
        //fill in with new rectangle
        display_fillRect(SIMON_DISPLAY_BUTTON_SIDE_BUFFER + HALF_DISPLAY_WIDTH, SIMON_DISPLAY_BUTTON_VERTICAL_BUFFER + HALF_DISPLAY_HEIGHT,
                BUTTON_DIMENSION, BUTTON_DIMENSION, DISPLAY_BLACK);
        break;
    }
}




void simonControl_displayTitleScreen(){ //print the title screen
    //display_init(); //initialize the display
    display_setTextSize(TITLE_MAIN_TEXT_SIZE); //set the text size and color for the main title
    display_setTextColor(DISPLAY_WHITE);
    display_setCursor(MESSAGE_X, MESSAGE_Y); //set the cursor at the correct spot to print the title
    display_println(TITLE_MAIN_TEXT); //print the title
    display_setTextSize(TITLE_SUB_TEXT_SIZE); //change the text size for the subtitle
    display_println(TITLE_SUB_TEXT); //print the subtitle
}

void simonControl_eraseTitleScreen(){ //erase the title screen
    display_setTextSize(TITLE_MAIN_TEXT_SIZE); //set the text size and color for the main title
    display_setTextColor(DISPLAY_BLACK);//change color to black so that it erases
    display_setCursor(MESSAGE_X, MESSAGE_Y); //set the cursor at the correct spot to print the title
    display_println(TITLE_MAIN_TEXT); //print the title
    display_setTextSize(TITLE_SUB_TEXT_SIZE); //change the text size for the subtitle
    display_println(TITLE_SUB_TEXT); //print the subtitle
    display_setTextColor(DISPLAY_WHITE); //reset the text color to be able to write again
}

void simonControl_eraseButtons(){
    simonControl_eraseButton(SIMON_DISPLAY_REGION_0);  //erase whatever might be left on the screen previously
    simonControl_eraseButton(SIMON_DISPLAY_REGION_1);  //also in region 1
    simonControl_eraseButton(SIMON_DISPLAY_REGION_2);  //also in region 2
    simonControl_eraseButton(SIMON_DISPLAY_REGION_3);  //also in region 3
}

#define INITIAL_SEQUENCE_LENGTH 4
#define ARRAY_BUFFER_SIZE 1000
#define NUMBER_OF_REGIONS 4
#define INITIAL_SEQUENCE_ITERATION_LENGTH 1
#define TIMER_0 0

void simonControl_tick(){
    //the next two static variables are used to make sure that we are in the same place
    //as the appropriate global variables, and will be used to let us know how the user is doing on the game
    static uint8_t currentSequenceLength = INITIAL_SEQUENCE_LENGTH;
    static uint8_t currentSequenceIterationLength = INITIAL_SEQUENCE_ITERATION_LENGTH;
    static bool updateSequence = true; //flag for updating the sequence's regions
    static bool timedOut = false; //flag for checking timed out input
    static bool badInput = false; //flag for checking bad input
    switch(sc_currentState){ //state actions
    case sc_initState: //reset values to ensure they are where we want them to start
        globals_setSequenceIterationLength(INITIAL_SEQUENCE_ITERATION_LENGTH); //set the global value correctly
        currentSequenceLength = INITIAL_SEQUENCE_LENGTH; //reset to initial value
        currentSequenceIterationLength = INITIAL_SEQUENCE_ITERATION_LENGTH; //reset to initial value
        simonControl_displayTitleScreen(); //print the title screen
        timedOut = false; //reset this flag
        badInput = false; //reset this flag
        updateSequence = true; //this is the first time through, so we will update the sequence
        newGameWaitTimer = NEW_GAME_WAIT_TIMER_START; //reset the timer
        break;
    case sc_titleScreenState:
        newGameWaitTimer = NEW_GAME_WAIT_TIMER_START; //reset the timer
        //remain here and do nothing for this state
        //reinitialize title screen display as mealy action to this state
        break;
    case sc_setGlobalsState:
        uint8_t mySequence[ARRAY_BUFFER_SIZE]; //hold a new array for a sequence
        uint32_t randomLocation; //hold a random location which will be generated
        srand(intervalTimer_getTotalDurationInSeconds(TIMER_0)); //seed the random number generator based on what the interval timer is at
        if (currentSequenceIterationLength > currentSequenceLength) { //if the current iteration is longer than the correct sequence we need to update
            updateSequence = true; //raise the update flag
            currentSequenceIterationLength = INITIAL_SEQUENCE_ITERATION_LENGTH; //reset the current iteration length
        }
        if (updateSequence){ //if we need to generate a new sequence
            for (uint8_t i = 0; i < currentSequenceLength; ++i){ //iterate through the number of regions
                randomLocation = rand() % NUMBER_OF_REGIONS; //generate a random region
                mySequence[i] = randomLocation; //put a random location into this part of the sequence
            }
            globals_setSequence(mySequence, INITIAL_SEQUENCE_LENGTH); //update the global sequence value
            updateSequence = false; //make sure to not update the sequence again since we just did
        }
        if (currentSequenceIterationLength > currentSequenceLength){ //check to see if we have made it to the end of the current sequence
            globals_setSequenceIterationLength(INITIAL_SEQUENCE_ITERATION_LENGTH); //reset the global variable after the sequence is done
            currentSequenceIterationLength = INITIAL_SEQUENCE_ITERATION_LENGTH; //reset our state machine's static variable after th sequence is done
        }
        break;
    case sc_flashSequenceState:
        //tick does the work
        break;
    case sc_verifySequenceState:
        //tick does the work
        newGameWaitTimer = NEW_GAME_WAIT_TIMER_START; //reset the wait timer
        break;
    case sc_displayResultState:
        if  (newGameWaitTimer++ == NEW_GAME_WAIT_TIMER_START){ //print the result only on the first tick, and iterate the timer
            if (timedOut){ //if they took too long
                display_setCursor(MESSAGE_X, MESSAGE_Y); //set the cursor at the correct spot to print the title
                simonControl_eraseButtons(); //erase whatever might be left on the screen previously
                display_println(TIME_OUT_TEXT); //print the title
            } else if (badInput){ //if they pressed the wrong button
                display_setCursor(MESSAGE_X, MESSAGE_Y); //set the cursor at the correct spot to print the title
                simonControl_eraseButtons(); //erase whatever might be left on the screen previously
                display_println(BAD_INPUT_TEXT); //print the title
            }
        }
        if (newGameWaitTimer == NEW_GAME_WAIT_TIMER_MAX){ //if its been long enough to erase
            if (timedOut){ //if they took too long
                display_setCursor(MESSAGE_X, MESSAGE_Y); //set the cursor at the correct spot to print the title
                display_setTextColor(DISPLAY_BLACK); //set text to erase
                display_println(TIME_OUT_TEXT); //erase the message
                display_setTextColor(DISPLAY_WHITE); //reset text to be able to write)
            } else if (badInput){ //if they pressed the wrong button
                display_setCursor(MESSAGE_X, MESSAGE_Y); //set the cursor at the correct spot to print the title
                display_setTextColor(DISPLAY_BLACK); //set text to erase
                display_println(BAD_INPUT_TEXT); //erase the message
                display_setTextColor(DISPLAY_WHITE); //reset text to be able to write)
            }
        }
        break;
    case sc_newLevelState:
        if  (newGameWaitTimer++ == NEW_GAME_WAIT_TIMER_START){ //if it's on its first ick
            simonControl_eraseButtons(); //erase the buttons
            display_setCursor(MESSAGE_X, MESSAGE_Y); //set the cursor at the correct spot to print the title
            display_println(WIN_TEXT); //print the title
        }
        if (newGameWaitTimer == NEW_GAME_WAIT_TIMER_MAX){ //if we have waited long enough to erase
            display_setCursor(MESSAGE_X, MESSAGE_Y); //set the cursor at the correct spot to print the title
            display_setTextColor(DISPLAY_BLACK); //set text color to erase
            display_println(WIN_TEXT); //erase the message
            display_setTextColor(DISPLAY_WHITE); //reset the text color to be able to write again next time it needs to
        }
        break;
    case sc_bestLevelDisplayState:
        if (newGameWaitTimer++ == NEW_GAME_WAIT_TIMER_START){ //if it's the first tick, also iterate the value each time it checks
            display_setCursor(MESSAGE_X, MESSAGE_Y); //set the cursor at the correct spot to print the title
            char bestLevelString[BEST_LEVEL_STRING_BUFFER]; //placeholder for the best level
            sprintf(bestLevelString, "Best Level: %d", currentSequenceLength); //print the best level to a string
            display_println(bestLevelString); //print the title
        } else if (newGameWaitTimer == NEW_GAME_WAIT_TIMER_MAX){
            display_setCursor(MESSAGE_X, MESSAGE_Y); //set the cursor at the correct spot to print the title
            char bestLevelString[BEST_LEVEL_STRING_BUFFER]; //placeholder for the best level
            sprintf(bestLevelString, "Best Level: %d", currentSequenceLength); //print the best level to a string
            display_setTextColor(DISPLAY_BLACK); //set the text color so that it can erase
            display_println(bestLevelString); //erase the message
            display_setTextColor(DISPLAY_WHITE); //reset the text color to be able to write later
        }
        break;
    }


    switch(sc_currentState){ //state transitions
    case sc_initState:
        sc_currentState = sc_titleScreenState; //move to the title screen
        break;
    case sc_titleScreenState:
        if (display_isTouched()){ //start the game when it is touched
            simonControl_eraseTitleScreen(); //erase the screen
            sc_currentState = sc_setGlobalsState; //start the game by calculating the sequence
        }
        break;
    case sc_setGlobalsState:
        flashSequence_enable(); //turn on the flash sequence since we have finished calculating
        sc_currentState = sc_flashSequenceState; //advance to the flash sequence state
        break;
    case sc_flashSequenceState:
        if (flashSequence_isComplete()){ //check if its done flashing
            flashSequence_disable(); //turn off the flash sequence state machine
            sc_currentState = sc_verifySequenceState; //move to the verifySequenceState
            simonDisplay_drawAllButtons(); //TAKES TONS OF TIME
            verifySequence_enable(); //turn on the verifySequence state machine
        }
        break;
    case sc_verifySequenceState:
        if (verifySequence_isTimeOutError()){ //if it the user took too long to press a button
            verifySequence_disable(); //turn off the state machine
            timedOut = true; //raise timeout flag
            sc_currentState = sc_displayResultState; //go to display result
        } else if (verifySequence_isUserInputError()){ //if they pressed the wrong button
            verifySequence_disable(); //turn off the state machine
            badInput = true; //raise the bad input flag
            sc_currentState = sc_displayResultState; //go to display the result
        } else if (verifySequence_isComplete()){
            verifySequence_disable(); //disable the state machine since it is finished
            if (currentSequenceIterationLength < currentSequenceLength){ //if we finished an iteration but the iteration isn't done
                flashSequence_enable(); // start the flash sequence when it gets here
                simonControl_eraseButtons(); //erase the buttons
                globals_setSequenceIterationLength(++currentSequenceIterationLength); //update global variable
                sc_currentState = sc_flashSequenceState; //sequence not finished, go back to the next
            } else{
                sc_currentState = sc_displayResultState; //current part of game is done, go to the display result state
            }
        }
        break;
    case sc_displayResultState:

        if (timedOut){ //if its timed out
            if (newGameWaitTimer >= NEW_GAME_WAIT_TIMER_MAX){ //once we have waited long tnough
                buttonHandler_disable(); //The buttonHandler does not get disabled when the verifySequence times out, so do it here
                sc_currentState = sc_bestLevelDisplayState; //go to the next display state
                newGameWaitTimer = 0; //reset the timer
            }
        } else if (badInput){ //if there wa bad input
            if (newGameWaitTimer >= NEW_GAME_WAIT_TIMER_MAX){ //one we have waited long enough
                sc_currentState = sc_bestLevelDisplayState; //move to the next display state
                newGameWaitTimer = 0; //reset the timer
            }
        } else{
            newGameWaitTimer = 0; //reset the timer since it's done
            sc_currentState = sc_newLevelState; //go to the next display state
        }
        break;
    case sc_newLevelState:
        if (newGameWaitTimer >= NEW_GAME_WAIT_TIMER_MAX){ //check if we have waited long enough
            newGameWaitTimer = 0; //reset the timer
            sc_currentState = sc_bestLevelDisplayState; //go to the next display state
        }
        if (display_isTouched()){
            ++currentSequenceLength; //iterate the current sequence length for this machine
            currentSequenceIterationLength = INITIAL_SEQUENCE_ITERATION_LENGTH; //reset the iteration length locally
            globals_setSequenceIterationLength(currentSequenceIterationLength); //and globally
            updateSequence = true; //we need a new sequence next time we go through
            sc_currentState = sc_setGlobalsState; //go back and calculate the globals again
            display_setCursor(MESSAGE_X, MESSAGE_Y); //set the cursor at the correct spot to print the title
            display_setTextColor(DISPLAY_BLACK); //set text color to erase
            display_println(WIN_TEXT); //erase the message
            display_setTextColor(DISPLAY_WHITE); //reset the text color to be able to write again next time it needs to
        }
        break;
    case sc_bestLevelDisplayState:
        if (newGameWaitTimer >= NEW_GAME_WAIT_TIMER_MAX){ //if we have waited at this display screen long enough
            newGameWaitTimer = 0; //reset the timer
            sc_currentState = sc_initState; //go back to the first state
            updateSequence = true; //we will need to update the sequence next time
        }
        break;
    }
}
