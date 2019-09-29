#ifndef UART_H
#define UART_H

void uart_init();

// called when a new byte received
// triggers uart_process_cmd() if appropriate
void uart_rx();

// processes the command in the buffer after \n was received
void uart_process_cmd();

// transmits the next byte from the send buffer
void uart_tx();

// starts tx and waits for completion (busy-wait)
void uart_tx_wait();

// sets the send buffer to contain string data with length len
void uart_set_string(const char* data, unsigned char len);

#endif //UART_H