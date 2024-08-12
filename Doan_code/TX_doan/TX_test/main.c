#include <avr/io.h> 
#define F_CPU 7372800UL 
#include <avr/interrupt.h> 
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <util/delay.h>
#include "nrf24l01.h" 
#include "nrf24l01-mnemonics.h" 
#include "uart.h" 
#include <myLCD.h>

// Khai b�o c�c h�m
nRF24L01 *setup_rf(void);

volatile bool rf_interrupt = false; // Bi?n c? ng?t truy?n RF
volatile bool send_message = false; // Bi?n c? ?? g?i tin nh?n
volatile uint8_t nut; // Bi?n l?u tr?ng th�i n�t nh?n

int main(void) {
	DDRB |= (1 << PB6); // C?u h�nh PB6 l�m ??u ra cho LED
	// C?u h�nh PC0 ??n PC3 l�m input n�t
	DDRC &= ~((1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3));
	 // K�ch ho?t ?i?n tr? k�o l�n tr�n PC0 ??n PC3
	PORTC |= ((1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3)); 
	init_LCD(); // Kh?i t?o LCD
	clr_LCD(); // X�a m�n h�nh LCD
	uint8_t to_address[5] = {0x01, 0x01, 0x01, 0x01, 0x01}; 
	// ??a ch? c?a ng??i nh?n
	sei(); // K�ch ho?t ng?t to�n c?c
	nRF24L01 *rf = setup_rf(); // Kh?i t?o module RF
	// C?u h�nh UART
	uart_init();

	while (true) {
		if (rf_interrupt) { // N?u c� ng?t RF
			rf_interrupt = false;
			int success = nRF24L01_transmit_success(rf); // Ki?m tra tr?ng th�i truy?n
			if (success != 0) {
				nRF24L01_flush_transmit_message(rf); // X�a b? ??m truy?n n?u kh�ng th�nh c�ng
			}
		}

		// Ki?m tra v� ch?ng rung cho n�t nh?n
		if (bit_is_clear(PINC, PC0)) {
			_delay_ms(150);
			if (bit_is_clear(PINC, PC0)) {
				nut = 0;
				send_message = true;
			}
			} else if (bit_is_clear(PINC, PC1)) {
			_delay_ms(150);
			if (bit_is_clear(PINC, PC1)) {
				nut = 1;
				send_message = true;
			}
			} else if (bit_is_clear(PINC, PC2)) {
			_delay_ms(150);
			if (bit_is_clear(PINC, PC2)) {
				nut = 2;
				send_message = true;
			}
			} else if (bit_is_clear(PINC, PC3)) {
			_delay_ms(150);
			if (bit_is_clear(PINC, PC3)) {
				nut = 3;
				send_message = true;
			}
		}

		if (send_message) { // N?u c� y�u c?u g?i tin nh?n
			send_message = false;
			nRF24L01Message msg;
			switch (nut) {
				case 0:
				clr_LCD();
				printf_LCD("MOTOR OFF");
				memcpy(msg.data, "A", 2); // Sao ch�p chu?i "A" v�o d? li?u
				PORTB ^= (1 << PB6);
				break;
				case 1:
				clr_LCD();
				printf_LCD("truyen 30% duty cycle");
				memcpy(msg.data, "B", 2); // Sao ch�p chu?i "B" v�o d? li?u
				PORTB ^= (1 << PB6);break;
				case 2:
				clr_LCD();
				printf_LCD("truyen 60% duty cycle");
				memcpy(msg.data, "C", 2); // Sao ch�p chu?i "C" v�o d? li?u
				PORTB ^= (1 << PB6);
				break;
				case 3:
				clr_LCD();
				printf_LCD("truyen 90% duty cycle");
				memcpy(msg.data, "D", 2); // Sao ch�p chu?i "D" v�o d? li?u
				PORTB ^= (1 << PB6);
				break;
			}
			msg.length = strlen((char *)msg.data) + 1; // T�nh ?? d�i c?a chu?i
			uart_string_tx((char *)msg.data); // ??c chu?i truy?n qua UART
			nRF24L01_transmit(rf, to_address, &msg); // G?i tin nh?n qua RF
			uart_char_tx('3');// ki?m tra c� l?i trong l?nh g?i ko
		}
	}
	
	return 0;
}

nRF24L01 *setup_rf(void) {
	nRF24L01 *rf = nRF24L01_init();
	rf->ss.port = &PORTB;
	rf->ss.pin = PB0; // Ch�n SS
	rf->ce.port = &PORTE;
	rf->ce.pin = PE3; // Ch�n CE
	rf->sck.port = &PORTB;
	rf->sck.pin = PB1; // Ch�n SCK
	rf->mosi.port = &PORTB;
	rf->mosi.pin = PB2; // Ch�n MOSI
	rf->miso.port = &PORTB;
	rf->miso.pin = PB3; // Ch�n MISO
	
	// Ng?t tr�n c?nh r?i c?a INT4 (PD4 tr�n ATmega128)
	EICRB |= (1 << ISC41);
	EIMSK |= (1 << INT4);
	
	nRF24L01_begin(rf);
	return rf;
}

ISR(INT4_vect) {
	rf_interrupt = true; // ??t c? ng?t RF
}
