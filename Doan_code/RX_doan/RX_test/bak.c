#include <avr/io.h>
#define F_CPU 7372800UL
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>
#include <util/delay.h>
#include "nrf24l01.h"
#include "nrf24l01-mnemonics.h"

nRF24L01 *setup_rf(void);
void process_message(char *message);
inline void prepare_led_pin(void);

void uart_char_tx(unsigned char chr);

void uart_char_tx(unsigned char chr) {
	while (bit_is_clear(UCSR0A, UDRE0)) { };
	UDR0 = chr;
}

volatile bool rf_interrupt = false;

int main(void) {
	uint8_t address[5] = { 0x01, 0x01, 0x01, 0x01, 0x01 };

	UBRR0H = 0;
	UBRR0L = 3;
	UCSR0A = 0x00;
	UCSR0B |= (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);
	// Select 8-bit data
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
	
	prepare_led_pin();
	sei();

	nRF24L01 *rf = setup_rf();
	nRF24L01_listen(rf, 0, address);

	// SPI configuration
	SPCR |= (1 << SPE) | (1 << SPIE);

	while (1) {
	
		if (rf_interrupt==true) {
			rf_interrupt = false;
			uart_char_tx('1');
			while (nRF24L01_data_received(rf)) {
				nRF24L01Message msg;
				nRF24L01_read_received_data(rf, &msg);
				
				process_message((char *)msg.data);
				//uart_char_tx('1');
			}

			nRF24L01_listen(rf, 0, address);
		}
	}

	return 0;
}

nRF24L01 *setup_rf(void) {
	nRF24L01 *rf = nRF24L01_init();
	rf->ss.port = &PORTB;
	rf->ss.pin = PB0; // SS pin
	rf->ce.port = &PORTC;
	rf->ce.pin = PC0; // CE pin
	rf->sck.port = &PORTB;
	rf->sck.pin = PB1; // SCK pin
	rf->mosi.port = &PORTB;
	rf->mosi.pin = PB2; // MOSI pin
	rf->miso.port = &PORTB;
	rf->miso.pin = PB3; // MISO pin
	// Interrupt on falling edge of INT4 (PD4 on ATmega128)
	EICRB |= (1 << ISC41);
	EIMSK |= (1 << INT4);
	sei();
	nRF24L01_begin(rf);
	return rf;
}

void process_message(char *message) {
	
	if (strcmp(message, "ON") == 0)
	PORTB |= (1 << PB6);
	
	else if (strcmp(message, "OFF") == 0)
	PORTB &= ~(1 << PB6);
}
inline void prepare_led_pin(void) {
	DDRB |= (1 << PB6);
	PORTB &= ~(1 << PB6);
}


// nRF24L01 interrupt
ISR(INT4_vect) {
	rf_interrupt = true;
	
}
//}

// SPI interrupt
//ISR(SPI_STC_vect) {
	////unsigned char received_data = SPDR; // Read received data from SPI data register
	////uart_char_tx(received_data)); // Transmit received data via UART
	////uart_char_tx('1'); // Indicate that data was received
//}
