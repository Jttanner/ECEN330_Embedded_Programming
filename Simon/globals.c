#include "globals.h"

#define SEQUENCE_INITIAL_VALUE 0

static uint8_t globals_sequence[GLOBALS_MAX_FLASH_SEQUENCE];

// The length of the sequence.
// The static keyword means that globals_sequenceLength can only be accessed
// by functions contained in this file.
static uint16_t globals_sequenceLength = SEQUENCE_INITIAL_VALUE;  // The length of the sequence.

static uint16_t globals_sequenceIterationLength = SEQUENCE_INITIAL_VALUE; //length of the current iteration of the sequence

// Make it big so you can use it for a splash screen.

// This is the length of the complete sequence at maximum length.
// You must copy the contents of the sequence[] array into the global variable that you maintain.
// Do not just grab the pointer as this will fail.
void globals_setSequence(const uint8_t sequence[], uint16_t length){
    globals_sequenceLength = length;
    for (uint8_t i = 0; i < length; ++i){ //iterate through each index of the array
        globals_sequence[i] = sequence[i]; //and copy the contents of the passed array into the global value
    }
}


// This returns the value of the sequence at the index.
uint8_t globals_getSequenceValue(uint16_t index){
    return globals_sequence[index];//gets the value at the passed index
}

// Retrieve the sequence length.
uint16_t globals_getSequenceLength(){
    return globals_sequenceLength;
}

//Retreive the sequence current iteration length
uint16_t globals_getSequenceIterationLength(){
    return globals_sequenceIterationLength;
}

// This is the length of the sequence that you are currently working on.
void globals_setSequenceIterationLength(uint16_t length){
    globals_sequenceIterationLength = length;
}


