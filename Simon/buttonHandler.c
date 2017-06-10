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
#include "src/intervalTimerLab/intervalTimer.h"

#define ERASE true
#define DONT_ERASE false

enum buttonHandlerStates{
    bhDisabledState, //state to stay in while disabled
    bhWaitingForTouchState, //waiting to be touched
    bhWaitTickState, //wait at tick fo hardware to catch up
    bhTouchedState //for while it is touched
}bhCurrentState = bhDisabledState; //initialize current state

static bool wasRemoved = false;

// Get the simon region numbers. See the source code for the region numbering scheme.
uint8_t buttonHandler_getRegionNumber(){
    int16_t x; //x coord for touch area
    int16_t y; //y coord for touch area
    uint8_t z; //pressure, we dont need to use other than to call the function
    display_getTouchedPoint(&x, &y, &z); //store the touch coordinates
    uint8_t region = simonDisplay_computeRegionNumber(x, y); //find the correct region based on the coordinates
    return region; //return the region number found
}

// Turn on the state machine. Part of the interlock.
void buttonHandler_enable(){
    bhCurrentState = bhWaitingForTouchState; //move it to wait for a touch when enabled
}

// Turn off the state machine. Part of the interlock.
void buttonHandler_disable(){
    bhCurrentState = bhDisabledState;
    //wasRemoved = false;
}


// The only thing this function does is return a boolean flag set by the buttonHandler state machine. To wit:
// Once enabled, the buttonHandler state-machine first waits for a touch. Once a touch is detected, the
// buttonHandler state-machine computes the region-number for the touched area. Next, the buttonHandler
// state-machine waits until the player removes their finger. At this point, the state-machine should
// set a bool flag that indicates the the player has removed their finger. Once the buttonHandler()
// state-machine is disabled, it should clear this flag.
// All buttonHandler_releasedDetected() does is return the value of this flag.
// As such, the body of this function should only contain a single line of code.
bool buttonHandler_releaseDetected(){
    return wasRemoved;
}

#define WAIT_COUNTER_LIMIT 0

// Standard tick function.
void buttonHandler_tick(){
    static uint8_t touchedButtonRegion;
    switch(bhCurrentState){ //state actions
    case bhDisabledState://do nothing if disabled
        break;
    case bhWaitingForTouchState:
        if(display_isTouched()){
            //display_clearOldTouchData();
        }
        break;
    case bhWaitTickState: //extra tick for the clear touch data and obtained region to catch up
        break;
    case bhTouchedState: //when it is touched, do mealy actions
        break;
    }
    switch(bhCurrentState){ //state transitions
    case bhDisabledState:
        wasRemoved = false; //make sure that it doesn't think that there was a touch release
        break;
    case bhWaitingForTouchState:
        if(display_isTouched()){ //check if it is touched
            bhCurrentState = bhWaitTickState; //if touched, change states
            display_clearOldTouchData(); //clear old touch data
        }

        break;
    case bhWaitTickState:
        touchedButtonRegion = buttonHandler_getRegionNumber(); //obtain the touched region
        simonDisplay_drawSquare(touchedButtonRegion, DONT_ERASE); //draw a square for where it is touched
        bhCurrentState = bhTouchedState; //advance states
        break;
    case bhTouchedState:
        simonDisplay_drawSquare(touchedButtonRegion, ERASE); //dont erase this time
        if(!display_isTouched()){ //if it is not touched anymore
            bhCurrentState = bhWaitingForTouchState; //go back ot the waiting for touch stat
            wasRemoved = true; //removal was true
            simonDisplay_drawButton(touchedButtonRegion); // draws the original button back
        }
        break;
    }
}

// This tests the functionality of the buttonHandler state machine.
// buttonHandler_runTest(int16_t touchCount) runs the test until
// the user has touched the screen touchCount times. It indicates
// that a button was pushed by drawing a large square while
// the button is pressed and then erasing the large square and
// redrawing the button when the user releases their touch.

#define RUN_TEST_TERMINATION_MESSAGE1 "buttonHandler_runTest()"  // Info message.
#define RUN_TEST_TERMINATION_MESSAGE2 "terminated."              // Info message.
#define RUN_TEST_TEXT_SIZE 2            // Make text easy to see.
#define RUN_TEST_TICK_PERIOD_IN_MS 100  // Assume a 100 ms tick period.
#define TEXT_MESSAGE_ORIGIN_X 0                  // Text is written starting at the right, and
#define TEXT_MESSAGE_ORIGIN_Y (DISPLAY_HEIGHT/2) // middle.

// buttonHandler_runTest(int16_t touchCount) runs the test until
// the user has touched the screen touchCount times. It indicates
// that a button was pushed by drawing a large square while
// the button is pressed and then erasing the large square and
// redrawing the button when the user releases their touch.

void buttonHandler_runTest(int16_t touchCount){
    int16_t currentTouchCount = 0;          // Keep track of the number of touches.
    display_init();                         // Always have to init the display.
    display_fillScreen(DISPLAY_BLACK);      // Clear the display.
       // Draw all the buttons for the first time so the buttonHandler doesn't need to do this in an init state.
       // Ultimately, simonControl will do this when the game first starts up.
    simonDisplay_drawAllButtons();
    buttonHandler_enable();
    while (currentTouchCount < touchCount) {    // Loop here while touchCount is less than the touchCountArg
        buttonHandler_tick();               // Advance the state machine.
        utils_msDelay(RUN_TEST_TICK_PERIOD_IN_MS);
        if (buttonHandler_releaseDetected()) {  // If a release is detected, then the screen was touched.
            currentTouchCount++;                       // Keep track of the number of touches.
               // Get the region number that was touched.
            //printf("button released: %d\n\r", buttonHandler_getRegionNumber());
               // Interlocked behavior: handshake with the button handler (now disabled).
            buttonHandler_disable();
            utils_msDelay(RUN_TEST_TICK_PERIOD_IN_MS);
            buttonHandler_tick();               // Advance the state machine.
            buttonHandler_enable();             // Interlocked behavior: enable the buttonHandler.
            utils_msDelay(RUN_TEST_TICK_PERIOD_IN_MS);
            buttonHandler_tick();               // Advance the state machine.
        }
    }
    display_fillScreen(DISPLAY_BLACK);        // clear the screen.
    display_setTextSize(RUN_TEST_TEXT_SIZE);  // Set the text size.
    display_setCursor(TEXT_MESSAGE_ORIGIN_X, TEXT_MESSAGE_ORIGIN_Y); // Move the cursor to a rough center point.
    display_println(RUN_TEST_TERMINATION_MESSAGE1); // Print the termination message on two lines.
    display_println(RUN_TEST_TERMINATION_MESSAGE2);
}

