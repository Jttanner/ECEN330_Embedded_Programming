/*
 * clockControl.c
 *
 *  Created on: May 24, 2017
 *      Author: jttanner
 */

#include "clockControl.h"
#include "clockDisplay.h"
#include "globals.h"
#include "supportFiles/display.h"
#include <stdio.h>

#define ADC_COUNTER_MAX_VALUE 1
#define AUTO_COUNTER_MAX_VALUE 10
#define RATE_COUNTER_MAX_VALUE 2
#define WAIT_COUNTER_MAX_VALUE 20

enum clockControlStates { //enums to guide state machine transitions
    init_st, //initial state
    waiting_for_first_touch_st,
    waiting_for_touch_st,
    adc_Counter_running_st,
    auto_Counter_running_st,
    rate_Counter_running_st,
    rate_Counter_expired_st
} clockControlState;

//static variables keep track of all of the countesr
static uint8_t adcCounter;
static uint8_t autoCounter;
static uint8_t rateCounter;
static uint8_t waitCounter; //counter to tell how much longer until we increment a second on the clock


// This is a debug state print routine. It will print the names of the states each
// time tick() is called. It only prints states if they are different than the
// previous state.
void debugStatePrint() {
  static clockControlStates previousState;
  clockControlStates currentState = clockControlState;
  static bool firstPass = true;
  // Only print the message if:
  // 1. This the first pass and the value for previousState is unknown.
  // 2. previousState != currentState - this prevents reprinting the same state name over and over.
  if (previousState != currentState || firstPass) {
    firstPass = false;                // previousState will be defined, firstPass is false.
    previousState = currentState;     // keep track of the last state that you were in.
    printf("msCounter:%d\n\r", waitCounter);
    switch(currentState) {            // This prints messages based upon the state that you were in.
      case init_st:
        printf("init_st\n\r");//print the current state
        break;
      case waiting_for_first_touch_st:
        printf("waiting_for_first_touch_st\n\r"); //print the current state
        break;
      case waiting_for_touch_st:
        printf("waiting_for_touch_st\n\r"); //print the current state
        break;
      case adc_Counter_running_st:
        printf("ad_timer_running_st\n\r"); //print the current state
        break;
      case auto_Counter_running_st:
        printf("auto_timer_running_st\n\r"); //print the current state
        break;
      case rate_Counter_running_st:
        printf("rate_timer_running_st\n\r"); //print the current state
        break;
      case rate_Counter_expired_st:
        printf("rate_timer_expired_st\n\r"); //print the current state
        break;
     }
  }
}


// Standard tick function.
void clockControl_tick(){
    //debugStatePrint(); use this to print the current state if desired
    //transitions
    switch(clockControlState){
        case init_st:
            clockControlState = waiting_for_first_touch_st; //initialize transition
            break;
        case waiting_for_first_touch_st: //here before the first touch causes the clock to start
            if (display_isTouched()){ //check if the screen has been touched
                clockControlState = adc_Counter_running_st; //advance to next state
                display_clearOldTouchData(); //clear any stored touch data
            }
            break;
        case waiting_for_touch_st:
            if (display_isTouched()){ //move to the correct state if a touch is detected
                clockControlState = adc_Counter_running_st; //advance state
                waitCounter = 0; //reset the waitCounter so that the clock will work correctly when we go back to this state.
                display_clearOldTouchData();  //clear any old data that the touch screen might have
            }
            if(!display_isTouched() && waitCounter >= WAIT_COUNTER_MAX_VALUE){ //if its not touched and enough time has passed
                clockDisplay_advanceTimeOneSecond(); //increment the clock
                waitCounter = 0; //reset the second timer
            }
            break;
        case adc_Counter_running_st:
            if(!display_isTouched() && adcCounter == ADC_COUNTER_MAX_VALUE){ //actions for after the ADC delay if there is no touch
                clockDisplay_performIncDec(); //update the clock and its variables
                clockControlState = waiting_for_touch_st; //reset to the beginning state
            } else if(display_isTouched() && adcCounter == ADC_COUNTER_MAX_VALUE){ //actions for after ADC delay if there is still touched
                clockControlState = auto_Counter_running_st; //next state transition for if it's still touched
            }
            break;
        case auto_Counter_running_st:
            if (!display_isTouched()){ //actions if there is no more touch
                clockControlState = waiting_for_touch_st; //move back to the beginning state
                clockDisplay_performIncDec(); //update the display and its variables
            } else if(display_isTouched() && autoCounter == AUTO_COUNTER_MAX_VALUE){ //actions for if there is still touch
                clockDisplay_performIncDec(); //update the display and its variables
                clockControlState = rate_Counter_running_st; //move to the next state
            }
            break;
        case rate_Counter_running_st:
            if(!display_isTouched()){ //go back to the beginning state once there is no more touch
                clockControlState = waiting_for_touch_st;
            } else if(display_isTouched() && rateCounter == RATE_COUNTER_MAX_VALUE){ // begin loop through the rate_counter states while touch lasts
                clockControlState = rate_Counter_expired_st;
            }
            break;
        case rate_Counter_expired_st:
            if(display_isTouched()){ //loop through the rate_counter states while it is still being touched
                clockDisplay_performIncDec(); //update display and variables
                clockControlState = rate_Counter_running_st;
            } else if(!display_isTouched()){
                clockControlState = waiting_for_touch_st;
            }
            break;
    }

    //State actions
    switch(clockControlState){
        case init_st: //has no actions
            break;
        case waiting_for_first_touch_st: //initialize the state machine's counter variables to 0
            adcCounter = 0; //for each of the three counters
            autoCounter = 0;
            rateCounter = 0;
            waitCounter = 0;
            break;
        case waiting_for_touch_st: //initialize the state machine's counter variables to 0
            adcCounter = 0; //for each of the three counters
            autoCounter = 0;
            rateCounter = 0;
            ++waitCounter;
            break;
        case adc_Counter_running_st:
            ++adcCounter; //increment the adc counter for each iteration
            break;
        case auto_Counter_running_st:
            ++autoCounter; //increment the autoCounter for each iteration
            break;
        case rate_Counter_running_st:
            ++rateCounter; //increment the rateCounter for each iteration
            break;
        case rate_Counter_expired_st:
            rateCounter = 0; //reset the rateCounter each time the rate counter expires and it is still being touched.
            break;
    }
}

// Call this before you call clockControl_tick().
void clockControl_init(){
    clockControlState = init_st; //initialize state machine
}
