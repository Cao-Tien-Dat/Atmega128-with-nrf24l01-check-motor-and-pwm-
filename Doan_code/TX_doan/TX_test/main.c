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

// Khai báo các hàm
nRF24L01 *setup_rf(void);

volatile bool rf_interrupt = false; // Bi?n c? ng?t truy?n RF
volatile bool send_message = false; // Bi?n c? ?? g?i tin nh?n
volatile uint8_t nut; // Bi?n l?u tr?ng thái nút nh?n

int main(void) {
	DDRB |= (1 << PB6); // C?u hình PB6 làm ??u ra cho LED
	// C?u hình PC0 ??n PC3 làm input nút
	DDRC &= ~((1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3));
	 // Kích ho?t ?i?n tr? kéo lên trên PC0 ??n PC3
	PORTC |= ((1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3)); 
	init_LCD(); // Kh?i t?o LCD
	clr_LCD(); // Xóa màn hình LCD
	uint8_t to_address[5] = {0x01, 0x01, 0x01, 0x01, 0x01}; 
	// ??a ch? c?a ng??i nh?n
	sei(); // Kích ho?t ng?t toàn c?c
	nRF24L01 *rf = setup_rf(); // Kh?i t?o module RF
	// C?u hình UART
	uart_init();

	while (true) {
		if (rf_interrupt) { // N?u có ng?t RF
			rf_interrupt = false;
			int success = nRF24L01_transmit_success(rf); // Ki?m tra tr?ng thái truy?n
			if (success != 0) {
				nRF24L01_flush_transmit_message(rf); // Xóa b? ??m truy?n n?u không thành công
			}
		}

		// Ki?m tra và ch?ng rung cho nút nh?n
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

		if (send_message) { // N?u có yêu c?u g?i tin nh?n
			send_message = false;
			nRF24L01Message msg;
			switch (nut) {
				case 0:
				clr_LCD();
				printf_LCD("MOTOR OFF");
				memcpy(msg.data, "A", 2); // Sao chép chu?i "A" vào d? li?u
				PORTB ^= (1 << PB6);
				break;
				case 1:
				clr_LCD();
				printf_LCD("truyen 30% duty cycle");
				memcpy(msg.data, "B", 2); // Sao chép chu?i "B" vào d? li?u
				PORTB ^= (1 << PB6);break;
				case 2:
				clr_LCD();
				printf_LCD("truyen 60% duty cycle");
				memcpy(msg.data, "C", 2); // Sao chép chu?i "C" vào d? li?u
				PORTB ^= (1 << PB6);
				break;
				case 3:
				clr_LCD();
				printf_LCD("truyen 90% duty cycle");
				memcpy(msg.data, "D", 2); // Sao chép chu?i "D" vào d? li?u
				PORTB ^= (1 << PB6);
				break;
			}
			msg.length = strlen((char *)msg.data) + 1; // Tính ?? dài c?a chu?i
			uart_string_tx((char *)msg.data); // ??c chu?i truy?n qua UART
			nRF24L01_transmit(rf, to_address, &msg); // G?i tin nh?n qua RF
			uart_char_tx('3');// ki?m tra có l?i trong l?nh g?i ko
		}
	}
	
	return 0;
}

nRF24L01 *setup_rf(void) {
	nRF24L01 *rf = nRF24L01_init();
	rf->ss.port = &PORTB;
	rf->ss.pin = PB0; // Chân SS
	rf->ce.port = &PORTE;
	rf->ce.pin = PE3; // Chân CE
	rf->sck.port = &PORTB;
	rf->sck.pin = PB1; // Chân SCK
	rf->mosi.port = &PORTB;
	rf->mosi.pin = PB2; // Chân MOSI
	rf->miso.port = &PORTB;
	rf->miso.pin = PB3; // Chân MISO
	
	// Ng?t trên c?nh r?i c?a INT4 (PD4 trên ATmega128)
	EICRB |= (1 << ISC41);
	EIMSK |= (1 << INT4);
	
	nRF24L01_begin(rf);
	return rf;
}

ISR(INT4_vect) {
	rf_interrupt = true; // ??t c? ng?t RF
}
