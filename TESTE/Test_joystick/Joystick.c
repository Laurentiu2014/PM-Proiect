#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

void uart_init(unsigned int ubrr) {
    // Set baud rate
    UBRR0H = (ubrr >> 8);
    UBRR0L = ubrr;
    // Enable transmit
    UCSR0B = (1 << TXEN0);
    // Set frame format: 8data, 1stop bit
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_transmit(char data) {
    while (!(UCSR0A & (1 << UDRE0))); // Wait for empty buffer
    UDR0 = data;
}

void uart_print(const char* str) {
    while (*str) {
        uart_transmit(*str++);
    }
}

void adc_init() {
    // Vref = AVcc, selectăm canalul ADC0 implicit
    ADMUX = (1 << REFS0);
    // Activare ADC, prescaler = 128 (16MHz/128 = 125kHz)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adc_read(uint8_t channel) {
    // Selectăm canalul (0 - 15)
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    // Pornim conversia
    ADCSRA |= (1 << ADSC);
    // Așteptăm să se termine
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

char buffer[64];

int main() {
    uart_init(103); // 9600 baud pentru 16MHz (UBRR = 103)
    adc_init();

    while (1) {
        uint16_t x = adc_read(0); // A0
        uint16_t y = adc_read(1); // A1

        snprintf(buffer, sizeof(buffer), "X = %4u | Y = %4u\r\n", x, y);
        uart_print(buffer);

        _delay_ms(300);
    }
}
