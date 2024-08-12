#include "uart.h"

void uart_init(void) {
	// Cau hinh UART baud rate la 115200
		UBRR1H = 0;
		UBRR1L = 3;

		UCSR1A = 0x00;
		UCSR1B |= (1 << TXEN1) | (1 << RXEN1) | (1 << RXCIE1);  // Enable TX, RX and RX interrupt for UART1
		UCSR1C |= (1 << UCSZ11) | (1 << UCSZ10);  // 8-bit data

}

void uart_char_tx(unsigned char chr) {
	while (bit_is_clear(UCSR1A, UDRE1));
	UDR1 = chr;
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