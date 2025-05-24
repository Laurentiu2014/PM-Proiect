#include <avr/io.h>
#include <util/delay.h>
#include "ILI9486.h"
#include "spi.h"

// Pini de control
#define LCD_CS_LOW()    (PORTG &= ~(1 << PG1))
#define LCD_CS_HIGH()   (PORTG |=  (1 << PG1))

#define LCD_RS_LOW()    (PORTD &= ~(1 << PD7))
#define LCD_RS_HIGH()   (PORTD |=  (1 << PD7))

#define LCD_WR_LOW()    (PORTG &= ~(1 << PG2))
#define LCD_WR_HIGH()   (PORTG |=  (1 << PG2))

#define LCD_RST_LOW()   (PORTG &= ~(1 << PG0))
#define LCD_RST_HIGH()  (PORTG |=  (1 << PG0))
uint8_t buffer[512];

void lcd_set_data(uint16_t data) {
    // DB0 - DB7 -> PC0 - PC7
    PORTC = data & 0xFF;

    // DB8 - DB15 -> PA0 - PA7
    PORTA = (data >> 8) & 0xFF;
}

void lcd_write_command(uint16_t cmd) {
    LCD_RS_LOW();
    LCD_CS_LOW();
    lcd_set_data(cmd);
    LCD_WR_LOW();
    _delay_us(1);
    LCD_WR_HIGH();
    LCD_CS_HIGH();
}

void lcd_write_data(uint16_t data) {
    LCD_RS_HIGH();
    LCD_CS_LOW();
    lcd_set_data(data);
    LCD_WR_LOW();
    _delay_us(1);
    LCD_WR_HIGH();
    LCD_CS_HIGH();
}

void lcd_reset() {
    LCD_RST_LOW();
    _delay_ms(20);
    LCD_RST_HIGH();
    _delay_ms(150);
}

void lcd_init() {
    // Setăm direcția pinilor de date ca output
    DDRA = 0xFF; // PA0 - PA7 (DB8 - DB15)
    DDRC = 0xFF; // PC0 - PC7 (DB0 - DB7)

    // Pini de control
    DDRD |= (1 << PD7);             // RS
    DDRG |= (1 << PG0) | (1 << PG1) | (1 << PG2); // RST, CS, WR

    lcd_reset();

    // Secvență minimă de inițializare (pentru ILI9486)
    lcd_write_command(0x01); // Software reset
    _delay_ms(5);

    lcd_write_command(0x28); // Display OFF
    lcd_write_command(0x3A); // Pixel format
    lcd_write_data(0x55);    // 16-bit/pixel

    lcd_write_command(0x36); // Memory Access Control
    lcd_write_data(0x48);    // MX, BGR

    lcd_write_command(0x11); // Sleep out
    _delay_ms(120);

    lcd_write_command(0x29); // Display ON
    _delay_ms(20);
}



void lcd_fill_screen(uint16_t color) {
    lcd_write_command(0x2A); // Set column address
    lcd_write_data(0x00);
    lcd_write_data(0x00);
    lcd_write_data(0x01);
    lcd_write_data(0x3F); // 319

    lcd_write_command(0x2B); // Set page address
    lcd_write_data(0x00);
    lcd_write_data(0x00);
    lcd_write_data(0x01);
    lcd_write_data(0xDF); // 479

    lcd_write_command(0x2C); // Memory Write

    for (uint32_t i = 0; i < 320UL * 480UL; i++) {
        lcd_write_data(color);
    }
}

void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    lcd_write_command(0x2A); // Col
    lcd_write_data(x0 >> 8);
    lcd_write_data(x0 & 0xFF);
    lcd_write_data(x1 >> 8);
    lcd_write_data(x1 & 0xFF);

    lcd_write_command(0x2B); // Row
    lcd_write_data(y0 >> 8);
    lcd_write_data(y0 & 0xFF);
    lcd_write_data(y1 >> 8);
    lcd_write_data(y1 & 0xFF);

    lcd_write_command(0x2C); // RAM write
}


void draw_raw_image(uint32_t start_sector, uint16_t width, uint16_t height) {
    lcd_write_command(0x2A); // Set column address
    lcd_write_data(0x00);
    lcd_write_data(0x00);
    lcd_write_data((width - 1) >> 8);
    lcd_write_data((width - 1) & 0xFF);

    lcd_write_command(0x2B); // Set page address
    lcd_write_data(0x00);
    lcd_write_data(0x00);
    lcd_write_data((height - 1) >> 8);
    lcd_write_data((height - 1) & 0xFF);

    lcd_write_command(0x2C); // Memory Write

    uint32_t num_pixels = (uint32_t)width * height;
    uint32_t sector = start_sector;
    uint16_t buffer_index = 0;
#define RES_OK 0

    while (num_pixels > 0) {
        if (disk_readp(buffer, sector++, 0, 512) != RES_OK)
            break;

        for (buffer_index = 0; buffer_index < 512 && num_pixels > 0; buffer_index += 2, num_pixels--) {
            uint16_t color = buffer[buffer_index] | (buffer[buffer_index + 1] << 8);
            lcd_write_data(color);
        }
    }
}