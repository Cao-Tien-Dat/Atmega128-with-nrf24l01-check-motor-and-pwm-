#include <avr/io.h>
#define F_CPU 7372800UL
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>
#include <util/delay.h>
#include "nrf24l01.h"
#include "nrf24l01-mnemonics.h"
#include "uart.h"
#include "myLCD.h"
nRF24L01 *setup_rf(void);
void process_message(char *message);
// Bi?n to�n c?c ?? theo d�i ng?t nh?n RF
volatile bool rf_interrupt = false;

int main(void) {
		uart_init();
		init_LCD();
		clr_LCD();
	//uart_char_tx('2');
	DDRE |= (1 << PE4); // LED ki?m tra
	DDRB |= (1 << PB6); //Thi?t l?p ch�n PB6 l�m ??u ra cho OC1B
	// ??a ch? RF
	uint8_t address[5] = { 0x01, 0x01, 0x01, 0x01, 0x01 };
	nRF24L01 *rf = setup_rf();
	nRF24L01_listen(rf, 0, address);//nghe tr�n pip 0
		// Thi?t l?p Timer1 ch�n OCR1B cho PWM mode 14; chia tan so cho 1
		TCCR1A = (1<<COM1B1) | (1<<WGM11);
		TCCR1B = (1<<WGM13) | (1<<WGM12) | (1<<CS10);
		TCNT1 = 0;
		OCR1B = 0;
		ICR1 = 10000;//top value

	sei(); // K�ch ho?t ng?t to�n c?c

	while (true) {
		uart_string_tx("da nhan: ");//ki?m tra ho?t ??ng c?a nh?n tr�n uart
		if (rf_interrupt) {//Ki?m tra n?u c� ng?t nh?n RF
			rf_interrupt = false;
			// Ki?m tra xem c� d? li?u RF nh?n ???c kh�ng
			while (nRF24L01_data_received(rf)) {
				nRF24L01Message msg;
				nRF24L01_read_received_data(rf, &msg);//??c d? li?u RF nh?n ???c
				uart_string_tx((char *)msg.data);//ki?m tra ho?t ??ng c?a nh?n tr�n uart
				uart_string_tx("\n-----\n");
				process_message((char *)msg.data);// X? l� th�ng ?i?p nh?n ???c
				
				//uart_char_tx('7');
			}
			nRF24L01_listen(rf, 0, address); //l?i nghe tren pip 0
		}
		_delay_ms(200);
	}
	return 0;
}

nRF24L01 *setup_rf(void) {
	nRF24L01 *rf = nRF24L01_init();// kh?i t?o rf24
	// Thi?t l?p c�c ch�n ?i?u khi?n c?a nRF24L01
	rf->ss.port = &PORTB;
	rf->ss.pin = PB0; // Ch�n SS
	rf->ce.port = &PORTD;
	rf->ce.pin = PD5; // Ch�n CE
	rf->sck.port = &PORTB;
	rf->sck.pin = PB1; // Ch�n SCK
	rf->mosi.port = &PORTB;
	rf->mosi.pin = PB2; // Ch�n MOSI
	rf->miso.port = &PORTB;
	rf->miso.pin = PB3; // Ch�n MISO

	// Thi?t l?p ng?t tr�n c?nh r?i c?a INT3
	EICRA |= (1 << ISC31);
	EIMSK |= (1 << INT3);

	nRF24L01_begin(rf);// ch?n mode cho ch�n
	return rf;// Tr? v? con tr? t?i c?u tr�c nRF24L01
}

void process_message(char *message) {
	if (strcmp(message, "A") == 0) {
		// n?u d? li?u truy?n t?i l� k� t? A
		clr_LCD();
		printf_LCD("MOTOR OFF");
		OCR1B = 0;// t?t motor
		PORTB &=~ (1 << PB6);
		PORTE ^=(1 << PE4); //ki?m tra
		} else if (strcmp(message, "B") == 0) {
			clr_LCD();
			printf_LCD("30% duty cycle");
		OCR1B = 3000;	//30% duty cycle
		PORTE ^= (1 << PE4); 
	}
		 else if (strcmp(message, "C") == 0) {
		clr_LCD();
		printf_LCD("60% duty cycle");
		OCR1B = 6000;//60% duty cycle
		PORTE ^=(1 << PE4); 
	}
	else if (strcmp(message, "D") == 0) {
		clr_LCD();
		printf_LCD("90% duty cycle");
		OCR1B = 9000;//90% duty cycle
		PORTE ^= (1 << PE4); //d�ng led ki?m tra ho?t ??ng
	}
	
}

ISR(INT3_vect) {
	rf_interrupt = true;
	// ng?t nh?n
}
