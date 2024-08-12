#ifndef UART_H_
#define UART_H_

#include <avr/io.h>
#include <stdio.h>

// Define baud rate
#define BAUD 115200

// Function prototypes
void uart_init(void);
void uart_char_tx(unsigned char chr);
void uart_string_tx(const char* str);
void uart_print_address(const uint8_t *address);

#endif /* UART_H_ */