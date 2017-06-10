/*
 * simonDisplay.c
 *
 *  Created on: Jun 2, 2017
 *      Author: jontt
 */


#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "supportFiles/display.h"
#include "supportFiles/utils.h"
#include "simonDisplay.h"

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


int8_t simonDisplay_computeRegionNumber(int16_t x, int16_t y){
 if (x > HALF_DISPLAY_WIDTH){ //for the right half of the display
      if (y < HALF_DISPLAY_HEIGHT){ //for the top half of the display
          //together get region 1 (top right)
          return SIMON_DISPLAY_REGION_1;
      } else{ //the bottom half of the display
          //together get region 3 (bottom right)
          return SIMON_DISPLAY_REGION_3;
      }
 } else{ //is on the left half of the display
     if (y < HALF_DISPLAY_HEIGHT){ //is on the top half of the display
         //top left means region 0
         return SIMON_DISPLAY_REGION_0;
     } else{ //bottom half of hte display
         //bottom left means region 2
         return SIMON_DISPLAY_REGION_2;
     }
 }
}

//display_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)

// Draws a colored "button" that the user can touch.
// The colored button is centered in the region but does not fill the region.
void simonDisplay_drawButton(uint8_t regionNumber){
    //display is 320 x 240 --> each region is 160x120
    //each button is 60x60 --> 100x60 space left over --> buffer space: 50 on each side, 30 above and below
    switch(regionNumber){ //choose which region to draw based on the parameter passed in
    case SIMON_DISPLAY_REGION_0: //draw a button on region 0
        //red button
        //erase whatever is currently there
        display_fillRect(SIMON_DISPLAY_BUTTON_SIDE_BUFFER, SIMON_DISPLAY_BUTTON_VERTICAL_BUFFER, SIMON_DISPLAY_BUTTON_WIDTH, SIMON_DISPLAY_BUTTON_HEIGHT, DISPLAY_BLACK);
        //fill in with new rectangle
        display_fillRect(SIMON_DISPLAY_BUTTON_SIDE_BUFFER, SIMON_DISPLAY_BUTTON_VERTICAL_BUFFER, SIMON_DISPLAY_BUTTON_WIDTH,
                         SIMON_DISPLAY_BUTTON_HEIGHT, DISPLAY_RED);
        break;
    case SIMON_DISPLAY_REGION_1: //draw a button on region 1
        //yellow button
        display_fillRect(SIMON_DISPLAY_BUTTON_SIDE_BUFFER + HALF_DISPLAY_WIDTH, SIMON_DISPLAY_BUTTON_VERTICAL_BUFFER,
                SIMON_DISPLAY_BUTTON_WIDTH, SIMON_DISPLAY_BUTTON_HEIGHT, DISPLAY_BLACK);
        //fill in with new rectangle
        display_fillRect(SIMON_DISPLAY_BUTTON_SIDE_BUFFER + HALF_DISPLAY_WIDTH, SIMON_DISPLAY_BUTTON_VERTICAL_BUFFER,
                         SIMON_DISPLAY_BUTTON_WIDTH, SIMON_DISPLAY_BUTTON_HEIGHT, DISPLAY_YELLOW);
        break;
    case SIMON_DISPLAY_REGION_2: //draw a button on region 2
        //blue button
        display_fillRect(SIMON_DISPLAY_BUTTON_SIDE_BUFFER, SIMON_DISPLAY_BUTTON_VERTICAL_BUFFER + HALF_DISPLAY_HEIGHT,
                SIMON_DISPLAY_BUTTON_WIDTH, SIMON_DISPLAY_BUTTON_HEIGHT, DISPLAY_BLACK);
        //fill in with new rectangle
        display_fillRect(SIMON_DISPLAY_BUTTON_SIDE_BUFFER, SIMON_DISPLAY_BUTTON_VERTICAL_BUFFER + HALF_DISPLAY_HEIGHT,
                         SIMON_DISPLAY_BUTTON_WIDTH, SIMON_DISPLAY_BUTTON_HEIGHT, DISPLAY_BLUE);
        break;
    case SIMON_DISPLAY_REGION_3: //draw a button on region 3
        //green button
        display_fillRect(SIMON_DISPLAY_BUTTON_SIDE_BUFFER + HALF_DISPLAY_WIDTH, SIMON_DISPLAY_BUTTON_VERTICAL_BUFFER + HALF_DISPLAY_HEIGHT,
                SIMON_DISPLAY_BUTTON_WIDTH, SIMON_DISPLAY_BUTTON_HEIGHT, DISPLAY_BLACK);
        //fill in with new rectangle
        display_fillRect(SIMON_DISPLAY_BUTTON_SIDE_BUFFER + HALF_DISPLAY_WIDTH, SIMON_DISPLAY_BUTTON_VERTICAL_BUFFER + HALF_DISPLAY_HEIGHT,
                         SIMON_DISPLAY_BUTTON_WIDTH, SIMON_DISPLAY_BUTTON_HEIGHT, DISPLAY_GREEN);
        break;
    }
}

void simonDisplay_eraseButton(uint8_t regionNumber){
    //display is 320 x 240 --> each region is 160x120
    //each button is 60x60 --> 100x60 space left over --> buffer space: 50 on each side, 30 above and below
    switch(regionNumber){ //choose which region to draw based on the parameter passed in
    case SIMON_DISPLAY_REGION_0: //erase a button on region 0
        //erase whatever is currently there
        //red button
        display_fillRect(SIMON_DISPLAY_BUTTON_SIDE_BUFFER, SIMON_DISPLAY_BUTTON_VERTICAL_BUFFER, SIMON_DISPLAY_SQUARE_WIDTH, SIMON_DISPLAY_SQUARE_HEIGHT, DISPLAY_BLACK);
        break;
    case SIMON_DISPLAY_REGION_1: //erase a button on region 1
        //yellow button
        display_fillRect(SIMON_DISPLAY_BUTTON_SIDE_BUFFER + HALF_DISPLAY_WIDTH, SIMON_DISPLAY_BUTTON_VERTICAL_BUFFER,
                SIMON_DISPLAY_SQUARE_WIDTH, SIMON_DISPLAY_SQUARE_HEIGHT, DISPLAY_BLACK);
        break;
    case SIMON_DISPLAY_REGION_2: //erase a button on region 2
        //blue button
        display_fillRect(SIMON_DISPLAY_BUTTON_SIDE_BUFFER, SIMON_DISPLAY_BUTTON_VERTICAL_BUFFER + HALF_DISPLAY_HEIGHT,
                SIMON_DISPLAY_SQUARE_WIDTH, SIMON_DISPLAY_SQUARE_HEIGHT, DISPLAY_BLACK);
        break;
    case SIMON_DISPLAY_REGION_3: //erase a button on region 3
        //green button
        display_fillRect(SIMON_DISPLAY_BUTTON_SIDE_BUFFER + HALF_DISPLAY_WIDTH, SIMON_DISPLAY_BUTTON_VERTICAL_BUFFER + HALF_DISPLAY_HEIGHT,
                SIMON_DISPLAY_SQUARE_WIDTH, SIMON_DISPLAY_SQUARE_HEIGHT, DISPLAY_BLACK);
        break;
    }
}

// Convenience function that draws all of the buttons.
void simonDisplay_drawAllButtons(){
    //erase everything currently drawn
    //simonDisplay_eraseAllButtons();
    simonDisplay_drawButton(SIMON_DISPLAY_REGION_0);//draw on region 0
    simonDisplay_drawButton(SIMON_DISPLAY_REGION_1);//draw on region 1
    simonDisplay_drawButton(SIMON_DISPLAY_REGION_2);//draw on region 2
    simonDisplay_drawButton(SIMON_DISPLAY_REGION_3);//draw on region 3

}

// Convenience function that erases all of the buttons.
void simonDisplay_eraseAllButtons(){
    simonDisplay_eraseButton(SIMON_DISPLAY_REGION_0);//draw on region 0
    simonDisplay_eraseButton(SIMON_DISPLAY_REGION_1);//draw on region 1
    simonDisplay_eraseButton(SIMON_DISPLAY_REGION_2);//draw on region 2
    simonDisplay_eraseButton(SIMON_DISPLAY_REGION_3);//draw on region 3
}

// Draws a bigger square that completely fills the region.
// If the erase argument is true, it draws the square as black background to "erase" it.
void simonDisplay_drawSquare(uint8_t regionNo, bool erase){
    if (erase){
        switch(regionNo){
        case SIMON_DISPLAY_REGION_0:
        //red square
        //erase whatever is currently there
            display_fillRect(ZERO_CORNER_COORDINATES, ZERO_CORNER_COORDINATES, SIMON_DISPLAY_SQUARE_WIDTH, SIMON_DISPLAY_SQUARE_HEIGHT, DISPLAY_BLACK);
        break;
        case SIMON_DISPLAY_REGION_1:
        //yellow square
        //erase old
            display_fillRect(ZERO_CORNER_COORDINATES + HALF_DISPLAY_WIDTH, ZERO_CORNER_COORDINATES,
                             SIMON_DISPLAY_SQUARE_WIDTH, SIMON_DISPLAY_SQUARE_HEIGHT, DISPLAY_BLACK);
        break;
        case SIMON_DISPLAY_REGION_2:
        //blue square
        //erase old
        display_fillRect(ZERO_CORNER_COORDINATES, ZERO_CORNER_COORDINATES + HALF_DISPLAY_HEIGHT,
                SIMON_DISPLAY_SQUARE_WIDTH, SIMON_DISPLAY_SQUARE_HEIGHT, DISPLAY_BLACK);
        break;
        case SIMON_DISPLAY_REGION_3:
        //green square
        //erase old
        display_fillRect(ZERO_CORNER_COORDINATES + HALF_DISPLAY_WIDTH, ZERO_CORNER_COORDINATES + HALF_DISPLAY_HEIGHT,
                SIMON_DISPLAY_SQUARE_WIDTH, SIMON_DISPLAY_SQUARE_HEIGHT, DISPLAY_BLACK);
        break;
        }
    } else{
        switch(regionNo){
        case SIMON_DISPLAY_REGION_0:
            //red square
            display_fillRect(ZERO_CORNER_COORDINATES, ZERO_CORNER_COORDINATES, SIMON_DISPLAY_SQUARE_WIDTH, SIMON_DISPLAY_SQUARE_HEIGHT, DISPLAY_RED);
            break;
        case SIMON_DISPLAY_REGION_1:
            //yellow square
            //fill in with new
            display_fillRect(ZERO_CORNER_COORDINATES + HALF_DISPLAY_WIDTH, ZERO_CORNER_COORDINATES,
                    SIMON_DISPLAY_SQUARE_WIDTH, SIMON_DISPLAY_SQUARE_HEIGHT, DISPLAY_YELLOW);
            break;
        case SIMON_DISPLAY_REGION_2:
            //blue square
            //fill in with new rectangle
            display_fillRect(ZERO_CORNER_COORDINATES, ZERO_CORNER_COORDINATES + HALF_DISPLAY_HEIGHT,
                    SIMON_DISPLAY_SQUARE_WIDTH, SIMON_DISPLAY_SQUARE_HEIGHT, DISPLAY_BLUE);

            break;
        case SIMON_DISPLAY_REGION_3:
            //green square
            //fill in with new rectangle
            display_fillRect(ZERO_CORNER_COORDINATES + HALF_DISPLAY_WIDTH, ZERO_CORNER_COORDINATES + HALF_DISPLAY_HEIGHT,
                    SIMON_DISPLAY_SQUARE_WIDTH, SIMON_DISPLAY_SQUARE_HEIGHT, DISPLAY_GREEN);
            break;
        }
    }
}

// Runs a brief demonstration of how buttons can be pressed and squares lit up to implement the user
// interface of the Simon game. The routine will continue to run until the touchCount has been reached, e.g.,
// the user has touched the pad touchCount times.

// I used a busy-wait delay (utils_msDelay) that uses a for-loop and just blocks until the time has passed.
// When you implement the game, you CANNOT use this function as we discussed in class. Implement the delay
// using the non-blocking state-machine approach discussed in class.
#define TOUCH_PANEL_ANALOG_PROCESSING_DELAY_IN_MS 60 // in ms
#define MAX_STR 255
#define TEXT_SIZE 2
#define TEXT_VERTICAL_POSITION 0
#define TEXT_HORIZONTAL_POSITION (DISPLAY_HEIGHT/2)
#define INSTRUCTION_LINE_1 "Touch and release to start the Simon demo."
#define INSTRUCTION_LINE_2 "Demo will terminate after %d touches."
#define DEMO_OVER_MESSAGE_LINE_1 "Simon demo terminated"
#define DEMO_OVER_MESSAGE_LINE_2 "after %d touches."
#define TEXT_VERTICAL_POSITION 0 // Start at the far left.
#define ERASE_THE_SQUARE true  // drawSquare() erases if this is passed in.
#define DRAW_THE_SQUARE false  // drawSquare() draws the square if this is passed in.
// Runs a brief demonstration of how buttons can be pressed and squares lit up to implement the user
// interface of the Simon game. The routine will continue to run until the touchCount has been reached, e.g.,
// the user has touched the pad touchCount times.

// I used a busy-wait delay (utils_msDelay) that uses a for-loop and just blocks until the time has passed.
// When you implement the game, you CANNOT use this function as we discussed in class. Implement the delay
void simonDisplay_runTest(uint16_t touchCount){
    display_init();         // Always initialize the display.
      char str[MAX_STR];      // Enough for some simple printing.
      uint8_t regionNumber = 0;   // Convenience variable.
      uint16_t touches = 0;   // Terminate when you receive so many touches.
      // Write an informational message and wait for the user to touch the LCD.
      display_fillScreen(DISPLAY_BLACK);              // clear the screen.
      display_setCursor(TEXT_VERTICAL_POSITION, TEXT_HORIZONTAL_POSITION); // move to the middle of the screen.
      display_setTextSize(TEXT_SIZE);                 // Set the text size for the instructions.
      display_setTextColor(DISPLAY_RED, DISPLAY_BLACK);   // Reasonable text color.
      sprintf(str, INSTRUCTION_LINE_1);                   // Copy the line to a buffer.
      display_println(str);                               // Print to the LCD.
      display_println();                                  // new-line.
      sprintf(str, INSTRUCTION_LINE_2, touchCount);       // Copy the line to a buffer.
      display_println(str);                               // Print to the LCD.
      while (!display_isTouched());       // Wait here until the screen is touched.
      while (display_isTouched());        // Now wait until the touch is released.
      display_fillScreen(DISPLAY_BLACK);  // Clear the screen.
      simonDisplay_drawAllButtons();      // Draw all of the buttons.
      bool touched = false;         // Keep track of when the pad is touched.
      int16_t x, y;                     // Use these to keep track of coordinates.
      uint8_t z;                        // This is the relative touch pressure.
      while (touches < touchCount) {  // Run the loop according to the number of touches passed in.
        if (!display_isTouched() && touched) {          // user has stopped touching the pad.
          simonDisplay_drawSquare(regionNumber, ERASE_THE_SQUARE);  // Erase the square.
          simonDisplay_drawButton(regionNumber);        // DISPLAY_REDraw the button.
          touched = false;                  // Released the touch, set touched to false.
        } else if (display_isTouched() && !touched) {   // User started touching the pad.
          touched = true;                             // Just touched the pad, set touched = true.
          touches++;                                  // Keep track of the number of touches.
          display_clearOldTouchData();                // Get rid of data from previous touches.
          // Must wait this many milliseconds for the chip to do analog processing.
          utils_msDelay(TOUCH_PANEL_ANALOG_PROCESSING_DELAY_IN_MS);
          display_getTouchedPoint(&x, &y, &z);        // After the wait, get the touched point.
          regionNumber = simonDisplay_computeRegionNumber(x, y);// Compute the region number, see above.
          simonDisplay_drawSquare(regionNumber, DRAW_THE_SQUARE);  // Draw the square (erase = false).
        }
      }
      // Done with the demo, write an informational message to the user.
      display_fillScreen(DISPLAY_BLACK);        // clear the screen.
      // Place the cursor in the middle of the screen.
      display_setCursor(TEXT_VERTICAL_POSITION, TEXT_HORIZONTAL_POSITION);
      display_setTextSize(TEXT_SIZE); // Make it readable.
      display_setTextColor(DISPLAY_RED, DISPLAY_BLACK);  // red is foreground color, black is background color.
      sprintf(str, DEMO_OVER_MESSAGE_LINE_1);    // Format a string using sprintf.
      display_println(str);                     // Print it to the LCD.
      sprintf(str, DEMO_OVER_MESSAGE_LINE_2, touchCount);  // Format the rest of the string.
      display_println(str);  // Print it to the LCD.
}
