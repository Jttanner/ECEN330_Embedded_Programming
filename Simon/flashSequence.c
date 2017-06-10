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
#include "flashSequence.h"
#include "globals.h"

#define ERASE true
#define DONT_ERASE false
#define HALF_FLASH_ITERATION_WAIT_SIZE 1
#define FLASH_ITERATION_WAIT_SIZE 2


static uint32_t flashWaitTimer = 0; //timer for timing how long to wait
static uint8_t currentIndex = 0; //way to check where the current index of the sequence to flash


enum flashSequenceStateMachine{
    fsDisabledState, //state for while its not on
    fsCalculatingState, //state for while its figuring out how to flash
    fsFlashState, //state for flashing
    fsWaitState,
    fsEndState //state to be finished in

}fsCurrentState = fsDisabledState;


// Turns on the state machine. Part of the interlock.
void flashSequence_enable(){
    //enable the state, put it into its state to figure out how it will flash
    flashWaitTimer = 0; //reset timer
    currentIndex = 0; //reset current index
    fsCurrentState = fsCalculatingState; //go to an enabled state
}

// Turns off the state machine. Part of the interlock.
void flashSequence_disable(){
    fsCurrentState = fsDisabledState; //disable
}

// Other state machines can call this to determine if this state machine is finished.
bool flashSequence_isComplete(){
    if (fsCurrentState == fsEndState){ //check if it is in the finished state
        return true; //it is complete
    } else{
        return false; //it is not complete
    }
}

// Standard tick function.
void flashSequence_tick(){
    switch(fsCurrentState){ //state actions
    case fsDisabledState:
        break;
    case fsCalculatingState:
        //globals_setSequenceIterationLength(globals_getSequenceIterationLength() + 1); //iterate the iteration length

        break;
    case fsFlashState:
        simonDisplay_drawSquare(globals_getSequenceValue(currentIndex), DONT_ERASE); //draw the flashing square
        ++currentIndex;//iterate hte current index
        break;
    case fsWaitState:
        ++flashWaitTimer; //iterate the wait timer
        if (flashWaitTimer >= HALF_FLASH_ITERATION_WAIT_SIZE){ //if half the timer's max value has come, erase it.  Do this so the next flash doesn't come too fast
            simonDisplay_drawSquare(globals_getSequenceValue(currentIndex - 1), ERASE); //erase the square
        }
        break;
    case fsEndState: //no actions in the end state
        break;
    }


    switch(fsCurrentState){ //state transitions
    case fsDisabledState: //no actions in the disabled state.  only transitions when enable is called
        break;
    case fsCalculatingState: //build the current sequence
        fsCurrentState = fsFlashState; //once it gets here it will have been calculated already
        break;
    case fsFlashState: //flash the current sequence, go back to calculate state if its not over
        fsCurrentState = fsWaitState;
        break;
    case fsWaitState:
        if (currentIndex >= globals_getSequenceIterationLength()){ //if we have reached teh end of the iteration
            fsCurrentState = fsEndState; //go to the last state to be finished
        } else if (flashWaitTimer >= FLASH_ITERATION_WAIT_SIZE){  //if we have waited long enough to get the next flash
            fsCurrentState = fsFlashState; //go back to the flash state
        }
        break;
    case fsEndState: //just wait here until disabled, so we can tell if it is completed
        break;
    }

}

// This will set the sequence to a simple sequential pattern.
// It starts by flashing the first color, and then increments the index and flashes the first
// two colors and so forth. Along the way it prints info messages to the LCD screen.
#define TEST_SEQUENCE_LENGTH 8  // Just use a short test sequence.
uint8_t flashSequence_testSequence[TEST_SEQUENCE_LENGTH] = {
    SIMON_DISPLAY_REGION_0,
    SIMON_DISPLAY_REGION_1,
    SIMON_DISPLAY_REGION_2,
    SIMON_DISPLAY_REGION_3,
    SIMON_DISPLAY_REGION_3,
    SIMON_DISPLAY_REGION_2,
    SIMON_DISPLAY_REGION_1,
    SIMON_DISPLAY_REGION_0};    // Simple sequence.
#define INCREMENTING_SEQUENCE_MESSAGE1 "Incrementing Sequence"  // Info message.
#define RUN_TEST_COMPLETE_MESSAGE "Runtest() Complete"          // Info message.
#define MESSAGE_TEXT_SIZE 2     // Make the text easy to see.
#define TWO_SECONDS_IN_MS 2000  // Two second delay.
#define TICK_PERIOD 75          // 200 millisecond delay.
#define TEXT_ORIGIN_X 0                  // Text starts from far left and
#define TEXT_ORIGIN_Y (DISPLAY_HEIGHT/2) // middle of screen.

// Print the incrementing sequence message.
void flashSequence_printIncrementingMessage() {
  display_fillScreen(DISPLAY_BLACK);  // Otherwise, tell the user that you are incrementing the sequence.
  display_setCursor(TEXT_ORIGIN_X, TEXT_ORIGIN_Y);// Roughly centered.
  display_println(INCREMENTING_SEQUENCE_MESSAGE1);// Print the message.
  utils_msDelay(TWO_SECONDS_IN_MS);   // Hold on for 2 seconds.
  display_fillScreen(DISPLAY_BLACK);  // Clear the screen.
}

// Run the test: flash the sequence, one square at a time
// with helpful information messages.
void flashSequence_runTest() {
  display_init();                   // We are using the display.
  display_fillScreen(DISPLAY_BLACK);    // Clear the display.
  globals_setSequence(flashSequence_testSequence, TEST_SEQUENCE_LENGTH);    // Set the sequence.
  flashSequence_enable();             // Enable the flashSequence state machine.
  int16_t sequenceLength = 1;         // Start out with a sequence of length 1.
  globals_setSequenceIterationLength(sequenceLength);   // Set the iteration length.
  display_setTextSize(MESSAGE_TEXT_SIZE); // Use a standard text size.
  while (1) {                             // Run forever unless you break.
    flashSequence_tick();             // tick the state machine.
    utils_msDelay(TICK_PERIOD);   // Provide a 1 ms delay.
    if (flashSequence_isComplete()) {   // When you are done flashing the sequence.
      flashSequence_disable();          // Interlock by first disabling the state machine.
      flashSequence_tick();             // tick is necessary to advance the state.
      utils_msDelay(TICK_PERIOD);       // don't really need this here, just for completeness.
      flashSequence_enable();           // Finish the interlock by enabling the state machine.
      utils_msDelay(TICK_PERIOD);       // Wait 1 ms for no good reason.
      sequenceLength++;                 // Increment the length of the sequence.
      if (sequenceLength > TEST_SEQUENCE_LENGTH)  // Stop if you have done the full sequence.
        break;
      // Tell the user that you are going to the next step in the pattern.
      flashSequence_printIncrementingMessage();
      globals_setSequenceIterationLength(sequenceLength);  // Set the length of the pattern.
    }
  }
  // Let the user know that you are finished.
  display_fillScreen(DISPLAY_BLACK);              // Blank the screen.
  display_setCursor(TEXT_ORIGIN_X, TEXT_ORIGIN_Y);// Set the cursor position.
  display_println(RUN_TEST_COMPLETE_MESSAGE);     // Print the message.
}


