#include <avr/io.h>
#include <util/delay.h>
#include "ILI9486.h"

int main(void)
{
    // Inițializare LCD
    lcd_init();

    // Culori de test
    uint16_t colors[] = {
        0xF800, // Roșu
        0x07E0, // Verde
        0x001F, // Albastru
        0xFFFF, // Alb
        0x0000, // Negru
        0xFFE0, // Galben
        0xF81F, // Magenta
        0x07FF  // Cyan
    };

    while (1)
    {
        for (uint8_t i = 0; i < sizeof(colors)/sizeof(colors[0]); i++)
        {
            lcd_fill_screen(colors[i]);
            _delay_ms(500);
        }
    }

    return 0;
}
