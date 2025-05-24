#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

// Buffer pentru mesaje UART
char buffer[64];

// UART init la 9600 baud (pentru 16 MHz UBRR = 103)
void uart_init(unsigned int ubrr) {
    UBRR0H = (ubrr >> 8);
    UBRR0L = ubrr;
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

// Transmitere caracter
void uart_transmit(char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

// Transmitere șir de caractere
void uart_print(const char* str) {
    while (*str) {
        uart_transmit(*str++);
    }
}

// Inițializare PA0–PA3 ca intrări cu pull-up activ
void buttons_init() {
    DDRB &= ~((1 << PB4) | (1 << PB5) | (1 << PB6) | (1 << PB7));   // input pinii 13-10
    PORTB |= (1 << PB4) | (1 << PB5) | (1 << PB6) | (1 << PB7);     // pull-up activ
}

// Citește starea unui pin (1 = apăsat, 0 = eliberat)
uint8_t read_button(uint8_t pin) {
    return !(PINB & (1 << pin));
}

int main(void) {
    uart_init(103);     // Baud rate 9600 pentru 16 MHz
    buttons_init();     // Configurare pini

    while (1) {
        // Afișează starea tuturor celor 4 butoane
        snprintf(buffer, sizeof(buffer), "PB4=%u | PB5=%u | PB6=%u | PB7=%u\r\n",
                 read_button(PB4), read_button(PB5),
                 read_button(PB6), read_button(PB7));
        uart_print(buffer);

        _delay_ms(500); // Delay pentru lizibilitate
    }
}
