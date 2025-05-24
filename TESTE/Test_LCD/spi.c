#include "spi.h"

void SPI_init(void)
{
    // Setăm SCK, MOSI, CS ca output
    DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2);
    // MISO ca input
    DDRB &= ~(1 << PB3);
    PORTB |= (1 << PB3); // Pull-up pe MISO


    // Activăm SPI, Master, frecvență fosc/128
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
    SPSR &= ~(1 << SPI2X); // fără dublare (fosc/128)
    SPI_DDR |= (1 << SPI_SS);    // CS pin ca output
    SPI_deselect();              // CS HIGH la start

}

void SPI_set_fast_speed(void)
{
    SPCR = (1 << SPE) | (1 << MSTR);  // SPR1=0, SPR0=0
    SPSR &= ~(1 << SPI2X);
}


uint8_t SPI_exchange(uint8_t data)
{
    // Trimite byte-ul
    SPDR = data;

    // Așteaptă finalizarea transmiterii
    while (!(SPSR & (1 << SPIF)));

    // Returnează byte-ul recepționat
    return SPDR;
}
