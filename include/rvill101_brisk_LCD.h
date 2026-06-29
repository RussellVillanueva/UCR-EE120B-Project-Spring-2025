#ifndef LCD_H_
#define LCD_H_H

#include <avr/io.h>
#include <util/delay.h>

// LCD data pins D4–D7
#define LCD_D4 PD5  // D5
#define LCD_D5 PD4  // D4
#define LCD_D6 PD3  // D3
#define LCD_D7 PD2  // D2

// Control pins on PORTB
#define LCD_RS PB4  // D12
#define LCD_EN PB3  // D11

#define DATA_DDR DDRD
#define DATA_PORT PORTD
#define CTL_DDR  DDRB
#define CTL_PORT PORTB

// LCD commands
#define LCD_CMD_CLEAR_DISPLAY 0x01
#define LCD_CMD_RETURN_HOME   0x02
#define LCD_CMD_ENTRY_MODE    0x06
#define LCD_CMD_DISPLAY_ON    0x0C
#define LCD_CMD_FUNCTION_SET  0x28
#define LCD_CMD_SET_DDRAM     0x80

// Internal helper: write nibble to PD2–PD5
void lcd_write_nibble(uint8_t nibble) {
	// Clear bits PD2–PD5
	DATA_PORT &= ~((1 << LCD_D4) | (1 << LCD_D5) | (1 << LCD_D6) | (1 << LCD_D7));

	if (nibble & 0x01) DATA_PORT |= (1 << LCD_D4); // D4
	if (nibble & 0x02) DATA_PORT |= (1 << LCD_D5); // D5
	if (nibble & 0x04) DATA_PORT |= (1 << LCD_D6); // D6
	if (nibble & 0x08) DATA_PORT |= (1 << LCD_D7); // D7
}

void lcd_pulse_enable() {
	CTL_PORT |= (1 << LCD_EN);
	_delay_us(1);
	CTL_PORT &= ~(1 << LCD_EN);
	_delay_us(100);
}

void lcd_send_command(uint8_t cmd) {
	CTL_PORT &= ~(1 << LCD_RS); // RS = 0 for command
	lcd_write_nibble((cmd >> 4) & 0x0F);
	lcd_pulse_enable();
	lcd_write_nibble(cmd & 0x0F);
	lcd_pulse_enable();
	_delay_ms(2);
}

void lcd_write_char(char c) {
	CTL_PORT |= (1 << LCD_RS); // RS = 1 for data
	lcd_write_nibble((c >> 4) & 0x0F);
	lcd_pulse_enable();
	lcd_write_nibble(c & 0x0F);
	lcd_pulse_enable();
	_delay_us(100);
}

void lcd_write_str(char* str) {
	while (*str) {
		lcd_write_char(*str++);
	}
}

void lcd_goto_xy(uint8_t row, uint8_t col) {
	uint8_t addr = (row == 1) ? 0x40 + col : col;
	lcd_send_command(LCD_CMD_SET_DDRAM | addr);
}

void lcd_clear() {
	lcd_send_command(LCD_CMD_CLEAR_DISPLAY);
	_delay_ms(2);
}

void lcd_init() {
	// Set pin modes
	DATA_DDR |= (1 << LCD_D4) | (1 << LCD_D5) | (1 << LCD_D6) | (1 << LCD_D7);
	CTL_DDR  |= (1 << LCD_RS) | (1 << LCD_EN);

	_delay_ms(50);

	// Init sequence
	lcd_write_nibble(0x03);
	lcd_pulse_enable();
	_delay_ms(5);

	lcd_write_nibble(0x03);
	lcd_pulse_enable();
	_delay_us(150);

	lcd_write_nibble(0x03);
	lcd_pulse_enable();
	_delay_us(150);

	lcd_write_nibble(0x02); // set 4-bit mode
	lcd_pulse_enable();
	_delay_us(150);

	lcd_send_command(LCD_CMD_FUNCTION_SET);  // 4-bit, 2-line, 5x8 font
	lcd_send_command(LCD_CMD_DISPLAY_ON);    // display on, cursor off
	lcd_send_command(LCD_CMD_ENTRY_MODE);    // entry mode: increment
	lcd_clear();                             // clear screen
}


// Making the obstacles and cars
void lcd_create_char(uint8_t location, uint8_t charmap[]) {
	location &= 0x7; 
	lcd_send_command(0x40 | (location << 3));

	for (uint8_t i = 0; i < 8; i++) {
		lcd_write_char(charmap[i]); 
	}
}

// Making the car to look like it can jump, clear the char
void clear_char(uint8_t row, uint8_t column) {
	lcd_goto_xy(row, column);
	lcd_write_char(' ');
}

// To display the score for the user at the end
void lcd_write_int(uint16_t num) {
    char buf[6];
    uint8_t i = 5;
    buf[i--] = '\0';
    if (num == 0) {
        buf[i] = '0';
    } else {
        while (num > 0 && i < 6) {
            buf[i--] = (num % 10) + '0';
            num /= 10;
        }
        i++;
    }
    lcd_write_str(&buf[i]);
}





#endif /* LCD_H_ */
