/* HD44780 Library     *
 * 2013 Fabian Lesniak */

#ifndef HD44780_H
#define HD44780_H

#include <stdbool.h>
#include <avr/io.h>

#define DATA_DDR DDRD
#define DATA_PIN PIND
#define DATA_PORT PORTD
#define CONTROL_DDR DDRC
#define CONTROL_PORT PORTC
#define CONTROL_RS PC5
#define CONTROL_RW PC4
#define CONTROL_E  PC3

#define FOUR_BITS

#ifdef FOUR_BITS
  #define DATA_MASK 0xF0
  #define DATA_OFFSET 4 //between 0 and 4
#else
  #define DATA_MASK 0xFF
  #define DATA_OFFSET 0
#endif

typedef unsigned char uchar;

typedef struct {
  bool busy;
  uchar address;
} displayStatus_t;

//Initializes DATA and CONTROL ports
void init();

//Clears the display's content
void clearDisplay();

//Moves the cursor to the first segment
void setCursorStart();

//Sets whether the cursor should be incremented or decremented, and whether the display's content
//shall remain or be shifted
void setMode1(bool cursorIncrement, bool displayShifting);

//Enables/disables display, and controls cursor displaying
void setMode2(bool displayOn, bool cursorOn, bool cursorFlash);

//Sets movement to Cursor/Content and direction
void setMovement(bool moveContent, bool moveRight);

//Sets 4/8-bit interface, 1 or 2/4 row display and 5x7/5x10 font
void setMode3(bool eightBit, bool multiLine, bool bigFont);

//Sets DDRAM address to address, msb of address is ignored
void setDDRAMAddress(uchar address);

//Sets CGRAM address to address, two msb are ignored
void setCGRAMAddress(uchar address);

//Gets the display's current status
displayStatus_t getStatus();

//Writes data to selected DDRAM/CGRAM address
void writeRAM(uchar byte);

//Reads data from selected DDRAM/CGRAM address
uchar readRAM();

/***********************************************
 * you should not need the following functions *
 * they are mainly used by the library itself  *
 ***********************************************/

//Writes byte as control sequence
void writeByte(uchar byte);

//Reads current control byte
uchar readByte();

//Gives an impulse on CONTROL_E
void commit();

//Checks the display's busy state
bool isBusy();

#endif
