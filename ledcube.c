/* 8x8x8 LED Cube software
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
 * compare value: 16MHz/1024/(30Hz*8) = 65.10 */
//#define PLEXDELAY 65
#define PLEXDELAY 25

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

#define BUTTON_L (!(PIND & (1<<PD3)))
#define BUTTON_R (!(PIND & (1<<PD2)))

//timer2 compare match
ISR(TIMER2_COMP_vect) {
  if (frameCounter == 255) {
    frameCounter=0;
  } else {
    frameCounter++;
    return;
  }
  if (currentRow != 7) //spi was too slow!
    ; //TODO error signaling
  if (currentLayer == 7) {
    frameCounter++;
    currentLayer = 0;
  } else
    currentLayer++;
  currentRow = 1; // row 0 is transferred below
  SPDR = currentImage[8*0+0];
  //SPDR = currentImage[8*currentLayer+0];
}

//serial transfer complete
ISR(SPI_STC_vect) {
  if (currentRow == 8) { //layer complete, shift layer driver)
    RCK_ON;
    PORTC = currentLayer | (PORTC & ~7);
    RCK_OFF;
  } else { //layer not complete yet, shift next row
    //SPDR = currentImage[8*currentLayer+currentRow];
    SPDR = currentImage[8*0+currentRow];
    currentRow++;
  }
}

int main() {
  DDRB = 0b00101101; //5=SCK (shift register data clock), 4=MISO (unused), 3=MOSI (shift register data), 2=RCK (shift register latch clock, was SS), 1=CS (unused), 0=lcd backlight
  DDRC = 0b00111111; //6=reset (disabled, otherwise input), 5=RS, 4=RW, 3=E, 2-0=layer mux
  DDRD = 0b11110010; //7-4=lcd data, 3-2=buttons, 1=txd, 0=rxd

  PORTB = 0b00010010; //enable pullups on unused inputs, disable clock and data lines, pull CS high (unused, but has 10k pullup), disable backlight
  PORTC = 0; //disable all mux pins (select layer 0), disable lcd outputs
  PORTD = 0b00001100; //disable lcd data, enable pullups on buttons, disable serial output data, disable pullup on serial input

  //Timer2 for layer multiplexing
  OCR2 = PLEXDELAY; //set compare match value
  TCCR2 = 0b00001111; //CTC mode, prescaler 1024
  TIMSK = 0b10000000; //enable output compare match interrupt

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
  BACKLIGHT_OFF;

  sei();

  memset(currentImage, 0xff, 64);

  unsigned char x=0, y=0, z=0;
  //currentLayer=0;
  while(1) {
    _delay_ms(100);
    //currentRow=0;
    currentImage[8*z+y] = 0;//xff;

    if (BUTTON_L) {
      if (i==0) {
        x==7?x=0:x++;
        i=1;
      }
    } else {
      if (BUTTON_R) {
        if (i==0) {
          y==7?y=0:y++;
          i=1;
        }
      } else
        i=0;
    }
    currentImage[8*z+y] = (1<<x);

    //SPDR = currentImage[8*currentLayer+currentRow]; //starts input, rest is done via ISR

    /*sleep_mode();
    if (frameCounter >= 30) {
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
    }*/
  }

  return 0;
}
