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


enum verifySequence_states{ //state machine state names
    vs_disabledState,
    vs_enabledState,
    vs_readState,
    vs_verifyingState,
    vs_badInputState,
    vs_completedState
} vs_currentState = vs_disabledState;



#define BEGIN_TOUCH_TIME 0
#define BEGIN_INDEX 0
#define MAX_TOUCH_TIME 25 //just a few seconds for each touch


static uint16_t waitDuration = BEGIN_TOUCH_TIME; //timer for waiting a certain amount of time
static bool badInput = false; //flag to show bad input

// State machine will run when enabled.
void verifySequence_enable(){
    badInput = false; //reset the badInput flag
    vs_currentState = vs_enabledState; //enables the verifySequence state machine
    //which will enable their tick functions
}

// This is part of the interlock. You disable the state-machine and then enable it again.
void verifySequence_disable(){
    waitDuration = 0; //reset the timer
    vs_currentState = vs_disabledState; //turns off the verifySequence state machine
    badInput = false; //reset the badInput flag
}

// Used to detect if there has been a time-out error.
bool verifySequence_isTimeOutError(){
    if (waitDuration >= MAX_TOUCH_TIME){ //if they have taken too long
        return true; //return that it was a timeout error
    } else{
        return false; //return that we are not timed out
    }
}

// Used to detect if the user tapped the incorrect sequence.
bool verifySequence_isUserInputError(){
    if (vs_currentState == vs_badInputState){ //if we are in the bad input state
        return true; //return bad input
    } else{
        return false; //return that input is ok
    }
}

// Used to detect if the verifySequence state machine has finished verifying.
bool verifySequence_isComplete(){
    if (vs_currentState == vs_completedState || vs_currentState == vs_badInputState){ //check if the state machine is in its completed state
        return true; //if so, return true to show it is complete
    } else{
        return false; //otherwise it's not done and will return false
    }
}



// Standard tick function.
void verifySequence_tick(){
    //state actions
    static uint8_t currentIndex; //use to get our current index
    srand(waitDuration * currentIndex); //seed random number
    switch(vs_currentState){ //decide what to do based on the current state
    case vs_disabledState: //if it is disabled
        break;
    case vs_enabledState: //if it is enabled
        if (currentIndex == BEGIN_INDEX) { //if we are just starting
            badInput = false; //lower bad input flag
        }
        buttonHandler_enable();
         //enables the buttonHandler state machine
        break;
    case vs_readState:
        if (!buttonHandler_releaseDetected()){ //if no touch is finished
            ++waitDuration; //increment the wait timer
        } else{
            //if the touched region and the correct sequence value dont match:
            if (buttonHandler_getRegionNumber() != globals_getSequenceValue(currentIndex)){
                    badInput = true; //raise bad input flag
            }
            buttonHandler_disable(); //disable the buttonhandler
            waitDuration = 0; //reset the timer
        }
        break;
    case vs_verifyingState:
        break;
    case vs_badInputState:
        break;
    case vs_completedState: //if it is done
        break;
    }

    //state transitions
    switch(vs_currentState){ //decide how to transition based on the current state
    case vs_disabledState: //if it is disabled
        break;
    case vs_enabledState: //if it is enabled
        vs_currentState = vs_readState; //advance to the next state
        break;
    case vs_readState:
        if (waitDuration >= MAX_TOUCH_TIME){ // if we have gotten to the max timer value with no problems
            vs_currentState = vs_completedState; //move to the completed state
            currentIndex = BEGIN_INDEX; //reset the index
            break;
        }
        if (badInput){ //if there was bad input
            vs_currentState = vs_badInputState; //go to the bad input state until disabled
            currentIndex = BEGIN_INDEX; //reset the index
            break;
        }
        if (buttonHandler_releaseDetected()){ //if there was a release
            vs_currentState = vs_verifyingState; //go to the state to verify it
            ++currentIndex; //iterate the index

        }
        if (currentIndex >= globals_getSequenceIterationLength()){ //if we have gotten to the max iteration length
            currentIndex = BEGIN_INDEX; //reset the index
            vs_currentState = vs_completedState; // go to the completed state
        }
        break;
    case vs_verifyingState:
        vs_currentState = vs_enabledState; //go back to the enabled state for more work
        break;
    case vs_badInputState:
        badInput = true; //raise bad input flag, wait here
        break;
    case vs_completedState: //if it is done
        badInput = false; //make sure that we have good input showing
        break;
    }
}


/*
 * Provided functions for the runTest method
 */

// Standard runTest function.
#define MESSAGE_X 0
//#define MESSAGE_Y (display_width()/4)
#define MESSAGE_Y (display_height()/2)
#define MESSAGE_TEXT_SIZE 2
//#define MESSAGE_STARTING_OVER
#define BUTTON_0 0  // Index for button 0
#define BUTTON_1 1  // Index for button 1
#define BUTTON_2 2  // Index for button 2
#define BUTTON_3 3  // Index for button 3
// Prints the instructions that the user should follow when
// testing the verifySequence state machine.
// Takes an argument that specifies the length of the sequence so that
// the instructions are tailored for the length of the sequence.
// This assumes a simple incrementing pattern so that it is simple to
// instruct the user.
void verifySequence_printInstructions(uint8_t length, bool startingOver) {
    display_fillScreen(DISPLAY_BLACK);              // Clear the screen.
    display_setTextSize(MESSAGE_TEXT_SIZE);     // Make it readable.
    display_setCursor(MESSAGE_X, MESSAGE_Y);    // Rough center.
    if (startingOver) {                                             // Print a message if you start over.
        display_fillScreen(DISPLAY_BLACK);          // Clear the screen if starting over.
        display_setTextColor(DISPLAY_WHITE);        // Print whit text.
        display_println("Starting Over. ");         // Starting over message.
    }
    // Print messages are self-explanatory, no comments needed.
    // These messages request that the user touch the buttons in a specific sequence.
    display_println("Tap: ");
    display_println();
    switch (length) {
    case 1:
        display_println("red");
        break;
    case 2:
        display_println("red, yellow ");
        break;
    case 3:
        display_println("red, yellow, blue ");
        break;
    case 4:
        display_println("red, yellow, blue, green ");
        break;
    default:
        break;
    }
    display_println("in that order.");
    display_println();
    display_println("hold BTN0 to quit.");
}

// Just clears the screen and draws the four buttons used in Simon.
void verifySequence_drawButtons() {
    display_fillScreen(DISPLAY_BLACK);  // Clear the screen.
    simonDisplay_drawButton(BUTTON_0);  // Draw the four buttons.
    simonDisplay_drawButton(BUTTON_1);
    simonDisplay_drawButton(BUTTON_2);
    simonDisplay_drawButton(BUTTON_3);
}

// This will set the sequence to a simple sequential pattern.
#define MAX_TEST_SEQUENCE_LENGTH 4  // the maximum length of the pattern
uint8_t verifySequence_testSequence[MAX_TEST_SEQUENCE_LENGTH] = {0, 1, 2, 3};  // A simple pattern.
#define MESSAGE_WAIT_MS 500  // Display messages for this long.


// Increment the sequence length making sure to skip over 0.
// Used to change the sequence length during the test.
int16_t incrementSequenceLength(int16_t sequenceLength) {
    int16_t value = (sequenceLength + 1) % (MAX_TEST_SEQUENCE_LENGTH+1);
    if (value == 0) value++;
    return value;
}

// Used to select from a variety of informational messages.
enum verifySequence_infoMessage_t {
    user_time_out_e,            // means that the user waited too long to tap a color.
    user_wrong_sequence_e,      // means that the user tapped the wrong color.
    user_correct_sequence_e,    // means that the user tapped the correct sequence.
    user_quit_e                 // means that the user wants to quite.
};

// Prints out informational messages based upon a message type (see above).
void verifySequence_printInfoMessage(verifySequence_infoMessage_t messageType) {
    // Setup text color, position and clear the screen.
    display_setTextColor(DISPLAY_WHITE);
    display_setCursor(MESSAGE_X, MESSAGE_Y);
    display_fillScreen(DISPLAY_BLACK);
  switch(messageType) {
  case user_time_out_e:  // Tell the user that they typed too slowly.
        display_println("Error:");
        display_println();
        display_println("  User tapped sequence");
        display_println("  too slowly.");
    break;
  case user_wrong_sequence_e:  // Tell the user that they tapped the wrong color.
        display_println("Error: ");
        display_println();
        display_println("  User tapped the");
        display_println("  wrong sequence.");
    break;
  case user_correct_sequence_e:  // Tell the user that they were correct.
        display_println("User tapped");
        display_println("the correct sequence.");
    break;
  case user_quit_e:             // Acknowledge that you are quitting the test.
    display_println("quitting runTest().");
    break;
  default:
    break;
  }
}

#define TICK_PERIOD_IN_MS 100
// Tests the verifySequence state machine.
// It prints instructions to the touch-screen. The user responds by tapping the
// correct colors to match the sequence.
// Users can test the error conditions by waiting too long to tap a color or
// by tapping an incorrect color.
void verifySequence_runTest() {
    display_init();  // Always must do this.
    buttons_init();  // Need to use the push-button package so user can quit.
    int16_t sequenceLength = 1;  // Start out with a sequence length of 1.
    verifySequence_printInstructions(sequenceLength, false);  // Tell the user what to do.
    utils_msDelay(MESSAGE_WAIT_MS);  // Give them a few seconds to read the instructions.
    verifySequence_drawButtons();    // Now, draw the buttons.
    // Set the test sequence and it's length.
    globals_setSequence(verifySequence_testSequence, MAX_TEST_SEQUENCE_LENGTH);
    globals_setSequenceIterationLength(sequenceLength);
    // Enable the verifySequence state machine.
    verifySequence_enable();  // Everything is interlocked, so first enable the machine.
    // Need to hold button until it quits as you might be stuck in a delay.
    while (!(buttons_read() & BUTTONS_BTN0_MASK)) {
        // verifySequence uses the buttonHandler state machine so you need to "tick" both of them.
        verifySequence_tick();  // Advance the verifySequence state machine.
        buttonHandler_tick();   // Advance the buttonHandler state machine.
        utils_msDelay(TICK_PERIOD_IN_MS);       // Wait for a tick period.
        // If the verifySequence state machine has finished, check the result,
        // otherwise just keep ticking both machines.
        if (verifySequence_isComplete()) {
            if (verifySequence_isTimeOutError()) {                // Was the user too slow?
                verifySequence_printInfoMessage(user_time_out_e); // Yes, tell the user that they were too slow.
            } else if (verifySequence_isUserInputError()) {       // Did the user tap the wrong color?
                verifySequence_printInfoMessage(user_wrong_sequence_e); // Yes, tell them so.
            } else {
                verifySequence_printInfoMessage(user_correct_sequence_e); // User was correct if you get here.
            }
            utils_msDelay(MESSAGE_WAIT_MS);                            // Allow the user to read the message.
            sequenceLength = incrementSequenceLength(sequenceLength);  // Increment the sequence.
            globals_setSequenceIterationLength(sequenceLength);        // Set the length for the verifySequence state machine.
            verifySequence_printInstructions(sequenceLength, true);    // Print the instructions.
            utils_msDelay(MESSAGE_WAIT_MS);                            // Let the user read the instructions.
            verifySequence_drawButtons();                              // Draw the buttons.
            verifySequence_disable();                                  // Interlock: first step of handshake.
            verifySequence_tick();                                     // Advance the verifySequence machine.
            utils_msDelay(TICK_PERIOD_IN_MS);                          // Wait for tick period.
            verifySequence_enable();                                   // Interlock: second step of handshake.
            utils_msDelay(TICK_PERIOD_IN_MS);                          // Wait for tick period.
        }
    }
    verifySequence_printInfoMessage(user_quit_e);  // Quitting, print out an informational message.
}
