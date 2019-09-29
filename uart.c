#include <avr/interrupt.h>
#include <avr/io.h>
#include <string.h>

#include "uart.h"
#include "macros.h"

static unsigned char uart_rxbytes;
static char uart_rxbuf[32];
static unsigned char uart_txbytes; // number of bytes already transferred
static unsigned char uart_txlen; // length off buffer content
static char uart_txbuf[32];

#define CRLF "\r\n"
static const char str_prompt[] = CRLF"LC# ";
static const char str_welcome[] = CRLF"LEDCUBE INIT"CRLF;

void uart_process_cmd() {
  if (uart_rxbytes != 0) { // skip empty commands (i.e. enter pressed without command)
    if (strcmp(uart_rxbuf, "status") == 0) {
      uart_set_string(CRLF"OK", 2+2);
      uart_tx_wait();
    } else if (strcmp(uart_rxbuf, "cmd1") == 0) {
      uart_set_string(CRLF"test cmd1", 2+9);
      uart_tx_wait();
    }
  }

  // sent the prompt
  uart_set_string(str_prompt, sizeof(str_prompt));
  uart_tx_wait();
}

void uart_tx() {
  if (!(UCSRA & (1 << UDRE)))
    return;
  if (uart_txbytes == uart_txlen)
    return;
  WAIT_UART_READY;
  UDR = uart_txbuf[uart_txbytes++];
    PORTB ^= (1 << 0);
}

void uart_tx_wait() {
  while (uart_txbytes != uart_txlen) {
    if (!(UCSRA & (1 << TXC)))
      uart_tx();
  }
}

void uart_rx() {
  if (!(UCSRA & (1 << RXC)))
    return;
  unsigned char temp = UDR;
  if (temp == '\r') {
    uart_process_cmd();
    uart_rxbytes = 0;
  } else if (uart_rxbytes < sizeof(uart_rxbuf)) {
    uart_rxbuf[uart_rxbytes] = UDR; // store byte and increment count
    WAIT_UART_READY;
    UDR = uart_rxbuf[uart_rxbytes];
    uart_rxbytes++;
  }
}

void uart_set_string(const char* data, unsigned char len) {
  strncpy(uart_txbuf, data, len);
  uart_txlen = len;
  uart_txbytes = 0;
}

void uart_init() {
  UCSRA = 0;
  UCSRB = (1 << RXCIE) | (1 << TXCIE) | (1 << RXEN) | (1 << TXEN);
  UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
  UBRRL = 103;

  uart_rxbytes = 0;

  uart_set_string(str_welcome, sizeof(str_welcome));
  uart_tx();
}
