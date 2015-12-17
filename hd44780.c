#include "hd44780.h"

#include <util/delay.h>

void commit() {
  CONTROL_PORT |= (1 << CONTROL_E);
  CONTROL_PORT &= ~(1 << CONTROL_E);
}

uchar readByte() {
  CONTROL_PORT |= (1 << CONTROL_E);
  _delay_us(10);
#ifdef FOUR_BITS
  uchar d = (DATA_PORT & DATA_MASK) >> (4-DATA_OFFSET);
  CONTROL_PORT &= ~(1 << CONTROL_E);
  _delay_us(10);
  CONTROL_PORT |= (1 << CONTROL_E);
  _delay_us(10);
  d = (DATA_PORT & DATA_MASK) >> DATA_OFFSET;
  CONTROL_PORT &= ~(1 << CONTROL_E);
#else
  //TODO copy original implementation
#endif
  return d;
}

bool isBusy() {
  return (readByte() & 0b10000000);
}

void writeByte(uchar byte) {
  //while( isBusy() );
  CONTROL_PORT &= ~(1 << CONTROL_RW); //set WRITE
#ifdef FOUR_BITS
  DATA_DDR |= 0xFF & DATA_MASK; //outputs
  DATA_PORT = ((byte << (4-DATA_OFFSET)) & DATA_MASK) | (DATA_PORT & ~DATA_MASK);
  commit();
  DATA_PORT = ((byte << (DATA_OFFSET)) & DATA_MASK) | (DATA_PORT & ~DATA_MASK);
  commit();
  DATA_PORT &= ~DATA_MASK;
  DATA_DDR &= ~DATA_MASK;
#else
  //TODO copy original implementation
#endif
  CONTROL_PORT |= (1 << CONTROL_RW); //set READ
  _delay_us(40);
}

void init() {
#ifdef FOUR_BITS
  DATA_DDR &= ~DATA_MASK; //set data pins as output (clear bits)
  DATA_PORT &= ~DATA_MASK; //disable data pins
#else
  DATA_DDR = 0;
  DATA_PORT = 0x00;
#endif
  CONTROL_DDR |= (1 << CONTROL_RS) | (1 << CONTROL_RW) | (1 << CONTROL_E); //every CONTROL pin is output
  CONTROL_PORT &= ~((1 << CONTROL_RS) | (0 << CONTROL_RW) | (1 << CONTROL_E)); //disable CONTROL pins, RW=1 is default
}

void clearDisplay() {
  writeByte(0x01);
  _delay_ms(2);
}

void setCursorStart() {
  writeByte(0x02);
}

void setMode1(bool cursorIncrement, bool displayShifting) {
  writeByte( 0x04 | (cursorIncrement << 1) | displayShifting );
}

void setMode2(bool displayOn, bool cursorOn, bool cursorFlash) {
  writeByte( 0x08 | (displayOn << 2) | (cursorOn << 1) | cursorFlash );
}

void setMovement(bool moveContent, bool moveRight) {
  writeByte( 0x10 | (moveContent << 3) | (moveRight << 2) );
}

void setMode3(bool eightBit, bool multiLine, bool bigFont) {
  writeByte( 0x20 | (eightBit << 4) | (multiLine << 3) | (bigFont << 2) );
}

void setCGRAMAddress(uchar address) {
  writeByte( 0x40 | (address & 0x3F) );
}

void setDDRAMAddress(uchar address) {
  writeByte( 0x80 | (address & 0x7F) );
}

displayStatus_t getStatus() {
  displayStatus_t d;
  d.address = readByte();
  d.busy = (d.address >> 7);
  d.address &= 0x7F;
  return d;
}

void writeRAM(uchar data) {
  CONTROL_PORT |= (1 << CONTROL_RS);
  writeByte(data);
  CONTROL_PORT &= ~(1 << CONTROL_RS);
}

uchar readRAM() {
  CONTROL_PORT |= (1 << CONTROL_RS);
  uchar d = readByte();
  CONTROL_PORT &= ~(1 << CONTROL_RS);
  return d;
}
