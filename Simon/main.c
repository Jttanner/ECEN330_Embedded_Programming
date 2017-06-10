/*
 * main.c
 *
 *  Created on: Jun 2, 2017
 *      Author: jontt
 */

#include <stdio.h>
#include "supportFiles/display.h"
#include "simonDisplay.h"
#include "buttonHandler.h"
#include "flashSequence.h"
#include "verifySequence.h"
#include "simonControl.h"
#include "src/intervalTimerLab/intervalTimer.h"
#include <stdlib.h>


#include "xparameters.h"
#include <stdio.h>
#include "supportFiles/leds.h"
#include "supportFiles/globalTimer.h"
#include "supportFiles/interrupts.h"
#include <stdbool.h>
#include <stdint.h>

#include "supportFiles/display.h"

#define TIMER_0 0
#define TIMER_1 1


#define TOTAL_SECONDS 60
// The formula for computing the load value is based upon the formula from 4.1.1 (calculating timer intervals)
// in the Cortex-A9 MPCore Technical Reference Manual 4-2.
// Assuming that the prescaler = 0, the formula for computing the load value based upon the desired period is:
// load-value = (period * timer-clock) - 1
#define TIMER_PERIOD 150.0E-3 // You can change this value to a value that you select.
#define TIMER_CLOCK_FREQUENCY (XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ / 2)
#define TIMER_LOAD_VALUE ((TIMER_PERIOD * TIMER_CLOCK_FREQUENCY) - 1.0)



typedef void (functionPointer_t)();
#define MAX_DURATION_TIMER_INDEX 2
static double maxDuration_g;                    // The max duration across all ticks.
static const char* maxDurationFunctionName_g;   // Name of the function with longest duration is here.
#define SIMON_CONTROL_TICK "simonControl_tick()"
#define VERIFY_SEQUENCE_TICK "verifySequence_tick()"
#define FLASH_SEQUENCE_TICK "flashSequency_tick()"
#define BUTTON_HANDLER_TICK "buttonHandler_tick()"

// This assumes that the interval timer was initialized.
static void tickTimer(functionPointer_t* fp,
                      const char* functionName) {
    intervalTimer_reset(MAX_DURATION_TIMER_INDEX);  // Reset the timer.
    intervalTimer_start(MAX_DURATION_TIMER_INDEX);  // Start the timer.
    fp();                                           // Invoke the tick function passed in by argument.
    intervalTimer_stop(MAX_DURATION_TIMER_INDEX);   // Stop the timer.
    double duration;                                // Keep track of tick duration.
    duration = intervalTimer_getTotalDurationInSeconds(MAX_DURATION_TIMER_INDEX);  // Get the duration.
    if (maxDuration_g < duration) {                 // Keep the max duration.
        maxDuration_g = duration;
        maxDurationFunctionName_g = functionName;   // Keep track of which function had the max. duration.
    }
}

int main(){
    //uncomment any of these to do their runTest

    //simonDisplay_runTest(8);

    //buttonHandler_runTest(100);

    //flashSequence_runTest();

    //verifySequence_runTest();

    display_init();//initialize the display
    display_fillScreen(DISPLAY_BLACK); //and erase anything from a previous program

    intervalTimer_init(TIMER_0); //init and start the interval timer
    intervalTimer_start(TIMER_0); //used to help seed the random number generator

    // Init all interrupts (but does not enable the interrupts at the devices).
    // Prints an error message if an internal failure occurs because the argument = true.
    interrupts_initAll(true);
    interrupts_setPrivateTimerLoadValue(TIMER_LOAD_VALUE);
    u32 privateTimerTicksPerSecond = interrupts_getPrivateTimerTicksPerSecond();
    printf("private timer ticks per second: %ld\n\r", privateTimerTicksPerSecond);
    // Allow the timer to generate interrupts.
    interrupts_enableTimerGlobalInts();
    // Initialization of the clock display is not time-dependent, do it outside of the state machine.
    // Keep track of your personal interrupt count. Want to make sure that you don't miss any interrupts.
     int32_t personalInterruptCount = 0;
    // Start the private ARM timer running.
    interrupts_startArmPrivateTimer();
    // Enable interrupts at the ARM.
    interrupts_enableArmInts();
    // interrupts_isrInvocationCount() returns the number of times that the timer ISR was invoked.
    // This value is maintained by the timer ISR. Compare this number with your own local
    // interrupt count to determine if you have missed any interrupts.
      while (interrupts_isrInvocationCount() < (TOTAL_SECONDS * privateTimerTicksPerSecond)) {
    //while(1){
      if (interrupts_isrFlagGlobal) {  // This is a global flag that is set by the timer interrupt handler.
          // Count ticks.
        personalInterruptCount++;
        //simonControl_tick();
        //flashSequence_tick(); //call the flash sequence's tick function
        //verifySequence_tick(); //tick the verifySequence machine
      //  buttonHandler_tick(); //tick the buttonHandler
        tickTimer(flashSequence_tick, FLASH_SEQUENCE_TICK);    // Tick the flashSequence state machine.
        tickTimer(verifySequence_tick, VERIFY_SEQUENCE_TICK);  // Tick theVerifySequence state machine.
        tickTimer(buttonHandler_tick, BUTTON_HANDLER_TICK);    // Tick the buttonHandler state machine.
        intervalTimer_init(TIMER_1); //init and start the interval timer
        intervalTimer_start(TIMER_1); //used to help test the time of a tick
        tickTimer(simonControl_tick, SIMON_CONTROL_TICK);      // Tick the SimonControl state machine.
        intervalTimer_stop(TIMER_1); //used to test the time of a tick
        interrupts_isrFlagGlobal = 0;
      }
   }
   //printf("MAX DURATION: %d,  MAX FUNCT NAME, %s\n", duration, functionName);
   interrupts_disableArmInts();
   printf("isr invocation count: %ld\n\r", interrupts_isrInvocationCount());
   printf("internal interrupt count: %ld\n\r", personalInterruptCount);
   return 0;

}

void isr_function() {

  }


