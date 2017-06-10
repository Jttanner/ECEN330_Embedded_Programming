/*
 * globals.h
 *
 *  Created on: May 24, 2017
 *      Author: jttanner
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <stdint.h>

//initialize global variables
void globals_initialize();

//use to update previous states of variables
void globals_updatePreviousValues();

//moves the time to a new specific location, good for incrementing the clock
void globals_progressTime();

uint8_t globals_getCurrentHours(); //get the current hour value
uint8_t globals_getCurrentMinutes(); //get the current minute value
uint8_t globals_getCurrentSeconds(); //get the current second value

void globals_setCurrentHours(uint8_t newHours); //set the current hour value
void globals_setCurrentMinutes(uint8_t newMinutes); //set current minute value
void globals_setCurrentSeconds(uint8_t newSeconds); //set current second value

uint8_t globals_getPreviousHours(); //get previous hour value
uint8_t globals_getPreviousMinutes(); //get previous minute value
uint8_t globals_getPreviousSeconds(); //get previous second value

uint8_t globals_getLeftHourDigit(); //get current left hour digit value
uint8_t globals_getRightHourDigit(); //get current right hour digit value
uint8_t globals_getLeftMinuteDigit(); //get current left minute digit value
uint8_t globals_getRightMinuteDigit(); //get current right minute digit value
uint8_t globals_getLeftSecondDigit(); //get current left second digit value
uint8_t globals_getRightSecondDigit(); //get current right second digit value

uint8_t globals_getPreviousLeftHourDigit(); //get previous left hour digit value
uint8_t globals_getPreviousRightHourDigit(); //get previous right hour digit value
uint8_t globals_getPreviousLeftMinuteDigit(); //get previous left minute digit value
uint8_t globals_getPreviousRightMinuteDigit(); //get previous right minute digit value
uint8_t globals_getPreviousLeftSecondDigit(); //get previous left second digit value
uint8_t globals_getPreviousRightSecondDigit(); //get previous right digit value



#endif /* GLOBALS_H_ */
