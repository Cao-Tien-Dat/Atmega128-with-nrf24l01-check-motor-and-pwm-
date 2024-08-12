#include "uart.h"

void uart_init(void) {
	// Cau hinh UART baud rate la 115200
	UBRR0H = 0;
	UBRR0L = 3;

	// Cho phep truyen va nhan UART
	UCSR0A = 0x00;
	UCSR0B |= (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);

	// Cau hinh frame format: 8-bit data, 1-bit stop
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_char_tx(unsigned char chr) {
	// Cho den khi san sang truyen
	while (bit_is_clear(UCSR0A, UDRE0)) { };
	// Gui ky tu
	UDR0 = chr;
}

void uart_string_tx(const char* str) {
	while (*str) {
		uart_char_tx(*str); // Gui tung ky tu cua chuoi
		str++;
	}
}

void uart_print_address(const uint8_t *address) {
	char buffer[3];
	for (int i = 0; i < 5; i++) {
		sprintf(buffer, "%02X", address[i]); // Chuyen doi byte thanh chuoi hex
		uart_string_tx(buffer); // Gui chuoi hex qua UART
		if (i < 4) {
			uart_char_tx(':'); // Gui ky tu phan cach ':'
		}
	}
	uart_string_tx("\n"); // Gui ky tu xuong dong
}
