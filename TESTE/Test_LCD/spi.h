#ifndef SPI_H_
#define SPI_H_

#include <avr/io.h>
#include <stdint.h>

/* === SPI Pin Mapping pentru ATmega2560 (SPI0) === */
#define SPI_DDR     DDRB
#define SPI_PORT    PORTB


#define SPI_SS      PB0   // corect pentru SD_CS pe Digital 53 (PB0)
#define SPI_SCK     PB1   // Digital 52
#define SPI_MOSI    PB2   // Digital 51
#define SPI_MISO    PB3   // Digital 50


void SPI_init(void);
uint8_t SPI_exchange(uint8_t data);

static inline void SPI_select(void)
{
    SPI_PORT &= ~(1 << SPI_SS); // CS LOW
}

static inline void SPI_deselect(void)
{
    SPI_PORT |= (1 << SPI_SS); // CS HIGH
}

void SPI_set_fast_speed(void);
#endif // SPI_H_
