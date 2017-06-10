/*
 * intervalTimer.c
 *
 *  Created on: May 17, 2017
 *      Author: jttanner
 */

#include "intervalTimer.h"

#include <stdio.h>

#include "xil_io.h"

#include "supportFiles/utils.h"

#define TIMER_NUMBER_0 0
#define TIMER_NUMBER_1 1
#define TIMER_NUMBER_2 2

//#define INTERVAL_TIMER_STATUS_FAIL -1

#define TIMER_0_OFFSET 0
#define TIMER_1_OFFSET 0x00010000
#define TIMER_2_OFFSET 0x00020000
#define TCSR_INITIAL_VALUE 0

#define TCSR_0_OFFSET 0
#define TCSR_1_OFFSET 0x10
#define TLR_0_OFFSET 0x04
#define TLR_1_OFFSET 0x14
#define TCR_0_OFFSET 0x08
#define TCR_1_OFFSET 0x18
#define CACADE_BIT 0x800
#define RESET_VALUE 0
#define LOAD_BIT_ON 0x20
#define ALL_TIMERS_FREQUENCY 100000000
#define REGISTER_BIT_WIDTH 32
#define FAILED -1

#define TEST_SUCCESS 1
#define TEST_FAILED 0


#define ENABLE_TIMER_BIT 0x80 //bitwise or this with what is already there ...0010000000
                              //if the timer is on, subract this from the register
                              //in order to erase the bit

//bitwise & this to keep whatever the MDT0 bit is and clear the UDT0 no matter what
#define CLEAR_UDT_0_BIT 0x01

//list helper functions
int32_t intervalTimer_readRegister(uint32_t offset);
void intervalTimer_writeRegister(uint32_t offset, int32_t value);
void intervalTimer_writeInitialRegisters(uint32_t timerOffset);
void intervalTimer_stopHelper(uint32_t timerOffset);
void intervalTimer_startHelper(uint32_t timerOffset);
void intervalTimer_resetHelper(uint32_t timerOffset);
intervalTimer_status_t intervalTimer_testHelper(uint32_t timerNumber, uint32_t timerOffset);
double intervalTimer_getTotalDurationInSecondsHelper(uint32_t timerOffset);

intervalTimer_status_t intervalTimer_init(uint32_t timerNumber){
    //for the given timer passed in as an argument:
    //write 0 to TCSR 0 and 1 registers
    //set the cascade bit, clear the UDT0 bit in TCSR0 reg
    switch(timerNumber){
        case TIMER_NUMBER_0:
            //run through the helper function, write to timer0's registers
            intervalTimer_writeInitialRegisters(TIMER_0_OFFSET);
            return INTERVAL_TIMER_STATUS_OK;
        case TIMER_NUMBER_1:
            //run through the helper function, write to timer1's registers
            intervalTimer_writeInitialRegisters(TIMER_1_OFFSET);
            return INTERVAL_TIMER_STATUS_OK;
        case TIMER_NUMBER_2:
            //run through the helper function, write to timer2's registers
            intervalTimer_writeInitialRegisters(TIMER_2_OFFSET);
            return INTERVAL_TIMER_STATUS_OK;
        default:
            //return a failure if passed an incorrect timerNumber
            //so that we don't overwrite something unintentionally
            return INTERVAL_TIMER_STATUS_FAIL;
        }
}

//HELPER FUNCTION, writes initial registers for a particular timer
void intervalTimer_writeInitialRegisters(uint32_t timerOffset){
    //write a 0 to TCSR0 and TCSR1 to reset
    //this should also set UDT0 to 0
    intervalTimer_writeRegister((TCSR_0_OFFSET + timerOffset), TCSR_INITIAL_VALUE);
    intervalTimer_writeRegister((TCSR_1_OFFSET + timerOffset), TCSR_INITIAL_VALUE);
    //set the cascade bit to enable cascade mode
    //this should be the only bit written to 1 for TCSR
    intervalTimer_writeRegister((TCSR_0_OFFSET + timerOffset), (intervalTimer_readRegister(TCSR_0_OFFSET + timerOffset) + CACADE_BIT));
    //might need to do something extra for UDT? Don't think so though
}



//HELPER FUNCTION read a particular register
int32_t intervalTimer_readRegister(uint32_t offset){
    //return the data at the register at the apporpriate register determined by the offset
    return Xil_In32(XPAR_AXI_TIMER_0_BASEADDR + offset);
}

//HELPER FUNCTION write a particular register
void intervalTimer_writeRegister(uint32_t offset, int32_t value){
    //write the value to the register at the appropriate register determined by the offset
    Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR + offset, value);
}


//initialize all of the interval timers
intervalTimer_status_t intervalTimer_initAll(){
    uint32_t success = 1; //act as a boolean to check if there is ever a failure

    //initialize timer 0, indicate if there is a failure
    if (intervalTimer_init(TIMER_NUMBER_0) == INTERVAL_TIMER_STATUS_FAIL) success = 0;
    //do the same for timer 1
    if (intervalTimer_init(TIMER_NUMBER_1) == INTERVAL_TIMER_STATUS_FAIL) success = 0;
    //and again for timer 2
    if (intervalTimer_init(TIMER_NUMBER_2) == INTERVAL_TIMER_STATUS_FAIL) success = 0;

    //check if any timer initializations failed
    if (success == 1){
        //return that it succeeded if there were no failures
        return INTERVAL_TIMER_STATUS_OK;
    } else{
        //indicate that the initialization failed if any timer did not initialize properly
        return INTERVAL_TIMER_STATUS_FAIL;
    }
}

// This function starts the interval timer running.
// timerNumber indicates which timer should start running.
void intervalTimer_start(uint32_t timerNumber){
    //choose which timer to start depending on the passed in argument
    switch(timerNumber){
        case TIMER_NUMBER_0:
            //start timer 0
            intervalTimer_startHelper(TIMER_0_OFFSET);
            break;
        case TIMER_NUMBER_1:
            //start timer 1
            intervalTimer_startHelper(TIMER_1_OFFSET);
            break;
        case TIMER_NUMBER_2:
            //start timer 2
            intervalTimer_startHelper(TIMER_2_OFFSET);
            break;
        default: //do nothing if it's an invalid timer is passed in
            break;
    }
}

//HELPER FUNCTION the process for starting a particular timer
void intervalTimer_startHelper(uint32_t timerOffset){
    //turn on the ENT0 bit for the given timer
    intervalTimer_writeRegister(TCSR_0_OFFSET + timerOffset, intervalTimer_readRegister(TCSR_0_OFFSET + timerOffset) | ENABLE_TIMER_BIT);
}


// This function stops the interval timer running.
// timerNumber indicates which timer should stop running.
void intervalTimer_stop(uint32_t timerNumber){
    //choose which timer to stop depending on the passed in argument
    switch(timerNumber){
        case TIMER_NUMBER_0:
            //stop timer 0
            intervalTimer_stopHelper(TIMER_0_OFFSET);
            break;
        case TIMER_NUMBER_1:
            //stop timer 1
            intervalTimer_stopHelper(TIMER_1_OFFSET);
            break;
        case TIMER_NUMBER_2:
            //stop timer 2
            intervalTimer_stopHelper(TIMER_2_OFFSET);
            break;
        default: //do nothing if it's an invalid timer is passed in
            break;
    }
}

//HELPER FUNCTTION the process for stopping a particular timer
void intervalTimer_stopHelper(uint32_t timerOffset){
    //turn the ENT0 bit off for the given timer
    intervalTimer_writeRegister(TCSR_0_OFFSET + timerOffset, intervalTimer_readRegister(TCSR_0_OFFSET + timerOffset) - ENABLE_TIMER_BIT);
}


void intervalTimer_reset(uint32_t timerNumber){
    //select which timer to reset based on the passed in argument
    switch(timerNumber){
    case TIMER_NUMBER_0:
        //do the reset steps for timer 0
        intervalTimer_resetHelper(TIMER_0_OFFSET);
        break;
    case TIMER_NUMBER_1:
        //do the reset steps for timer 1
        intervalTimer_resetHelper(TIMER_1_OFFSET);
        break;
    case TIMER_NUMBER_2:
        //do the reset steps for timer 2
        intervalTimer_resetHelper(TIMER_2_OFFSET);
        break;
    default: //they didn't say to reset a valid timer, so do nothing
        break;
    }
}

//HELPER FUNCTION the process for resetting a particular timer
void intervalTimer_resetHelper(uint32_t timerOffset){ //the steps for resetting a timer
    //store a 0 into counter 0:
    //write 0 into TLR0 register
    intervalTimer_writeRegister(TLR_0_OFFSET + timerOffset, RESET_VALUE);
    //write a 1 into LOAD0 in TCSR0,
    intervalTimer_writeRegister(TCSR_0_OFFSET + timerOffset, intervalTimer_readRegister(TCSR_0_OFFSET + timerOffset) | LOAD_BIT_ON);

    //store a 0 into counter 1:
    //write 0 into TLR1 register
    intervalTimer_writeRegister(TLR_1_OFFSET + timerOffset, RESET_VALUE);
    //write a 1 into LOAD1 in TCSR1,
    intervalTimer_writeRegister(TCSR_1_OFFSET + timerOffset, intervalTimer_readRegister(TCSR_0_OFFSET + timerOffset) | LOAD_BIT_ON);

    //after it has been reset, reset the TCSR values to 0
    //similar to initializing in the first place,
    //but is separated as this it an addition to the reset, not the initialization



    //set TCSR0 to its initial state
    intervalTimer_writeRegister(TCSR_0_OFFSET + timerOffset, TCSR_INITIAL_VALUE);
    //set TCSR1 to its initial state
    intervalTimer_writeRegister(TCSR_1_OFFSET + timerOffset, TCSR_INITIAL_VALUE);
    //maintain the cascade bit
    intervalTimer_writeRegister(TCSR_0_OFFSET + timerOffset, (intervalTimer_readRegister(TCSR_0_OFFSET + timerOffset) + CACADE_BIT));
}




void intervalTimer_resetAll(){
    //reset timer 0
    intervalTimer_reset(TIMER_NUMBER_0);
    //reset timer 1
    intervalTimer_reset(TIMER_NUMBER_1);
    //reset timer 2
    intervalTimer_reset(TIMER_NUMBER_2);
}

// Runs a test on a single timer as indicated by the timerNumber argument.
// Returns INTERVAL_TIMER_STATUS_OK if successful, something else otherwise.
intervalTimer_status_t intervalTimer_test(uint32_t timerNumber){
    //choose which timer to test based on the passed in argument
    switch(timerNumber){
        case TIMER_NUMBER_0:
            return intervalTimer_testHelper(TIMER_NUMBER_0, TIMER_0_OFFSET);
        case TIMER_NUMBER_1:
            return intervalTimer_testHelper(TIMER_NUMBER_1, TIMER_1_OFFSET);
        case TIMER_NUMBER_2:
            return intervalTimer_testHelper(TIMER_NUMBER_2, TIMER_2_OFFSET);
        default:
            //return failure if an invalid timer is passed in as an argument
            return INTERVAL_TIMER_STATUS_FAIL;
    }
}

//HELPER FUNCTION run the steps of a test on a particular timer
intervalTimer_status_t intervalTimer_testHelper(uint32_t timerNumber, uint32_t timerOffset){
    //reset the interval timer
    intervalTimer_reset(timerNumber);

    //test to see if the value reset properly
    int32_t checkMe = intervalTimer_readRegister(timerOffset);
    //if it did not reset properly, return a failure state
    if (checkMe != RESET_VALUE) return INTERVAL_TIMER_STATUS_FAIL;

    //start the counter
    intervalTimer_start(timerNumber);
    //read the register after starting
    checkMe = intervalTimer_readRegister(timerOffset);
    //see if it changes when we read again.  Return failure if it doesn't change
    if(checkMe == intervalTimer_readRegister(timerOffset)) return INTERVAL_TIMER_STATUS_FAIL;
    //check it again for good measure
    if(checkMe == intervalTimer_readRegister(timerOffset)) return INTERVAL_TIMER_STATUS_FAIL;

    //stop the counter
    intervalTimer_stop(timerNumber);
    //store the value it is stopped at
    checkMe = intervalTimer_readRegister(timerOffset);
    //check whether or not the timer is still incrementing.  if it is, return failure
    if(checkMe != intervalTimer_readRegister(timerOffset)) return INTERVAL_TIMER_STATUS_FAIL;
    //do the same check one more time for good measure
    if(checkMe!= intervalTimer_readRegister(timerOffset)) return INTERVAL_TIMER_STATUS_FAIL;

    //if it gets to here, then things are running properly, so return an OK status
    return INTERVAL_TIMER_STATUS_OK;
}

// Convenience function that invokes test on all interval timers.
// Returns INTERVAL_TIMER_STATUS_OK if successful, something else otherwise.
intervalTimer_status_t intervalTimer_testAll(){
    uint32_t success = TEST_SUCCESS; //act as a boolean to check if there is ever a failure
    //test timer 0, indicate if there is a failure
    if (intervalTimer_test(TIMER_NUMBER_0) == INTERVAL_TIMER_STATUS_FAIL) success = TEST_FAILED;
    //do the same for timer 1
    if (intervalTimer_test(TIMER_NUMBER_1) == INTERVAL_TIMER_STATUS_FAIL) success = TEST_FAILED;
    //and again for timer 2
    if (intervalTimer_test(TIMER_NUMBER_2) == INTERVAL_TIMER_STATUS_FAIL) success = TEST_FAILED;

    //check if any timer tests failed
    if (success == TEST_SUCCESS){

        //return that it succeeded if there were no failures
        return INTERVAL_TIMER_STATUS_OK;

    } else{
            //otherwise indicate that the test failed
            return INTERVAL_TIMER_STATUS_FAIL;
        }
}

//convert the data in the counter to seconds
double intervalTimer_getTotalDurationInSeconds(uint32_t timerNumber){
    //choose which timer's time duration depending upon which argument is passed
    switch(timerNumber){
    case TIMER_NUMBER_0:
        //get the time in timer 1
        return intervalTimer_getTotalDurationInSecondsHelper(TIMER_0_OFFSET);
    case TIMER_NUMBER_1:
        //get the timme in timer 2
        return intervalTimer_getTotalDurationInSecondsHelper(TIMER_1_OFFSET);
    case TIMER_NUMBER_2:
        //get the time in timer 3
        return intervalTimer_getTotalDurationInSecondsHelper(TIMER_2_OFFSET);
    default:
        //show that it was an error for reading an invalid timer
        return FAILED;
    }
}

//HELPER FUNCTION  the process of changing the values inside of the counter to seconds
double intervalTimer_getTotalDurationInSecondsHelper(uint32_t timerOffset){
    //get the values of each of the TCR registers
    uint32_t TCR_0_value = intervalTimer_readRegister(TCR_0_OFFSET + timerOffset);
    uint64_t TCR_1_value = intervalTimer_readRegister(TCR_1_OFFSET + timerOffset);

    //set a 64 bit value to place the total number in
    uint64_t TCR_cascaded_value = 0;
    //shift the more 32 significant bits first
    //then or that with the less 32 significant bits to conacatenate
    TCR_cascaded_value = (TCR_1_value << REGISTER_BIT_WIDTH) | TCR_0_value;


    //cast the number to a double before division to get double precision, calculation described below
    double returnValue = (((double)TCR_cascaded_value) / ALL_TIMERS_FREQUENCY);

    // the frequency for each of the timers is 100000000 HZ, or 100 MHZ
    // 1/100MHZ = 1x10^-8 seconds = 10 ns
    // means there are 10 nano seconds for frequency
    // divide the cascaded value by the frequency to get seconds, return this value
    return returnValue;
}




