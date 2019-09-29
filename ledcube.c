/* 8x8x8 LED Cube software
 */

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdbool.h>

#include "hd44780.h"
#include "ledcube.h"
#include "cube.h"
#include "uart.h"
#include "ui.h"

/* TIMER2 FREQUENCY CALCULATION
 * target: 60 fps => 60 Hz*8=480 Hz layer clock
 * main clock: 16 MHz
 * compare value: 16MHz/1024/(60Hz*8) = 32.55 */
#define FRAMEDELAY 32

//timer2 compare match
ISR(TIMER2_COMP_vect) {
  cube_frameInterrupt();
}

//serial transfer complete
ISR(SPI_STC_vect) {
  cube_rowInterrupt();
}

//uart byte received
ISR(USART_RXC_vect) {
  uart_rx();
}

//uart tx complete
ISR(USART_TXC_vect) {
  uart_tx();
}

int main() {
  DDRB = 0b00101101; //5=SCK (shift register data clock), 4=MISO (unused), 3=MOSI (shift register data), 2=RCK (shift register latch clock, was SS), 1=CS (unused), 0=lcd backlight
  DDRC = 0b00111111; //6=reset (disabled, otherwise input), 5=RS, 4=RW, 3=E, 2-0=layer mux
  DDRD = 0b11110010; //7-4=lcd data, 3-2=buttons, 1=txd, 0=rxd

  PORTB = 0b00010010; //enable pullups on unused inputs, disable clock and data lines, pull CS high (unused, but has 10k pullup), disable backlight
  PORTC = 0; //disable all mux pins (select layer 0), disable lcd outputs
  PORTD = 0b00001100; //disable lcd data, enable pullups on buttons, disable serial output data, disable pullup on serial input

  //Timer2 for layer multiplexing
  OCR2 = FRAMEDELAY; //set compare match value
  TCCR2 = 0b00001111; //CTC mode, prescaler 1024
  TIMSK = 0b10000000; //enable output compare match interrupt

  //SPI for shift register operation
  SPCR = 0b11110000; //interrupt enable, spi enable, LSB first, master mode, CPOL=0, CPHA=0, SPR1:0=00 (->clk/4)
  SPSR = 0b00000001; //double speed -> clk/2

  ui_init();
  uart_init();
  cube_init();

  sei();

  unsigned char debounce = 0;
  while(1) {
    sleep_mode();

    if (debounce == 60) {
      ui_pollButtons();
      debounce = 0;
    } else
      debounce++;

    cube_run();
  }

  return 0;
}
