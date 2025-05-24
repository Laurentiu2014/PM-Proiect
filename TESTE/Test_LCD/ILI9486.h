#ifndef ILI9486_H
#define ILI9486_H

#include <avr/io.h>

void lcd_init(void);
void lcd_write_command(uint16_t cmd);
void lcd_write_data(uint16_t data);
void lcd_fill_screen(uint16_t color);
void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void lcd_write_data(uint16_t color);
void draw_raw_image(uint32_t start_sector, uint16_t width, uint16_t height);

#endif // ILI9486_H
