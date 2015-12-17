/* Nerf gun speed meter using atmega8 and photodiodes
 * Pinout:
 * PB0 - Input Capture (OR'd led sensors, active high)
 * PD0-PD6 - 7-segment display cathodes pin a-f (active low)
 * PB1-PB3 - 7-segment display anode driver (BC560 or similar)
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdbool.h>
#include <string.h>

#include "hd44780.h"

/* TIMER2 FREQUENCY CALCULATION
 * target: 30 fps => 30*8=240 Hz layer clock
 * main clock: 16MHz
 * max prescaler: 1024 -> 19.53125 kHz
 * compare value: 20MHz/1024/(30Hz*8) = 81.38 */
#define PLEXDELAY 81

unsigned char currentLayer = 0; //currently multiplexed segment (equals decade)
unsigned char currentRow = 0; //currently transmitted row (if 7 after transfer, row is complete)
unsigned char currentImage[64] = {0}; //segment code to display on every segment

unsigned short frameCounter = 0;

#define LED_OFF(x,y,z) (currentImage[8*z+y] |= (1<<x))
#define LED_ON(x,y,z) (currentImage[8*z+y] &= ~(1<<x))

#define BACKLIGHT_ON (PORTB |= (1 << 0))
#define BACKLIGHT_OFF (PORTB &= ~(1 << 0))

#define RCK_ON (PORTB |= (1 << 2))
#define RCK_OFF (PORTB &= ~(1 << 2))

//timer2 compare match
ISR(TIMER2_COMP_vect) {
  if (currentRow != 7) //spi was too slow!
    ; //TODO error signaling
  currentRow = 0;
  frameCounter++;
  if (currentLayer == 7)
    currentLayer = 0;
  else
    currentLayer++;
  SPDR = currentImage[8*currentLayer+currentRow];
}

//serial transfer complete
ISR(SPI_STC_vect) {
  if (currentRow == 7) { //layer complete, shift layer driver)
    RCK_ON;
    PORTC = currentRow | (PORTC & ~7); //attention: also disables LCD RS,RW,E pins, but should not matter
    RCK_OFF;
  } else { //layer not complete yet, shift next row
    SPDR = currentImage[8*currentLayer+currentRow];
    currentRow++;
  }
}

int main() {
  DDRB = 0b00010000; //5=SCK (shift register data clock), 4=MISO (unused), 3=MOSI (shift register data), 2=RCK (shift register latch clock, was SS), 1=CS (unused), 0=lcd backlight
  DDRC = 0b01000000; //6=reset (disabled, otherwise input), 5=RS, 4=RW, 3=E, 2-0=layer mux
  DDRD = 0b00001101; //7-4=lcd data, 3-2=buttons, 1=txd, 0=rxd

  PORTB = 0b00010010; //enable pullups on unused inputs, disable clock and data lines, pull CS high (unused, but has 10k pullup), disable backlight
  PORTC = 0; //disable all mux pins (select layer 0), disable lcd outputs
  PORTD = 0b00001100; //disable lcd data, enable pullups on buttons, disable serial output data, disable pullup on serial input

  //Timer2 for layer multiplexing
  OCR2 = PLEXDELAY; //set compare match value
  TCCR2 = 0b00001111; //CTC mode, prescaler 1024
  //TIMSK = 0b10000000; //enable output compare match interrupt

  //SPI for shift register operation
  SPCR = 0b11110000; //interrupt enable, spi enable, LSB first, master mode, CPOL=0, CPHA=0, SPR1:0=00 (->clk/4)
  SPSR = 0b00000001; //double speed -> clk/2

   //display
  init();
  setMode1(1,0);
  setMode2(1,0,0);
  setMovement(0,1);
  setMode3(0,1,0);

  setDDRAMAddress(0);
  const char helloStr[] = "LEDCUBE";
  uchar i;
  for( i = 0; i < strlen(helloStr); i++ ) {
    writeRAM(helloStr[i]);
  }
  BACKLIGHT_ON;

  sei();

  memset(currentImage, 0xff, sizeof(currentImage));

  //unsigned char x=0, y=0, z=0;
  while(1) {
  /*  sleep_mode();
    if (frameCounter == 30) {
  BACKLIGHT_ON;
      LED_OFF(x,y,z);
      if (z == 7 && y == 7 && x == 7)
        x = y = z = 0;
      else {
        if (y == 7 && x == 7) {
          z++;
          x = y = 0;
        } else {
          if (x == 7) {
            x = 0;
            y++;
          } else
            x++;
        }
      }
      LED_ON(x,y,z);
      frameCounter = 0;
  BACKLIGHT_OFF;
    }*/
    _delay_ms(1000);
    if (currentLayer == 7) { //layer complete, shift layer driver)
      //RCK_ON;
      //RCK_OFF;
      currentLayer = 0;
    } else { //layer not complete yet, shift next row
      //SPDR = currentImage[8*currentLayer+currentRow];
      currentLayer++;
    }
    PORTC = currentLayer; // | (PORTC & ~7); //attention: also disables LCD RS,RW,E pins, but should not matter
  }

  return 0;
}
