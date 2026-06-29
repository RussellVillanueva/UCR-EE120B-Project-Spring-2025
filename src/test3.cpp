// /*  Your Name & E-mail: Russell Villanueva & rvill101@ucr.edu

// Discussion Section: 023

 
// Assignment: EE120B Project Demo progess

 
// Exercise Description: N/A


// I acknowledge all content contained herein, excluding template or example code, is my own original work.

//  *  Russell Villanueva

 
// Demo Link: <https://youtu.be/HXOt5mN6YM8>

//  */











// // Print a 32-bit value in full 8-nibble hex (no sign)
// void serial_printhex32(uint32_t v) {
//     char buf[9];
//     for (int i = 0; i < 8; ++i) {
//         uint8_t nib = (v >> ((7 - i) * 4)) & 0xF;
//         buf[i] = nib < 10 ? '0' + nib : 'A' + nib - 10;
//     }
//     buf[8] = '\0';
//     serial_println(buf);
// }






// #include <avr/io.h>
// #include <util/delay.h>
// #include "brisk_LCD.h"


// int main(void) {
// 	lcd_init();
// 	lcd_goto_xy(0, 0);
// 	lcd_write_str("HELLO LCD");
// 	lcd_goto_xy(1, 0);
// 	lcd_write_str("IT WORKS!");

// 	while (1) {}
// }