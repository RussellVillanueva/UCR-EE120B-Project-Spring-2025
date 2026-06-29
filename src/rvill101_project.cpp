// /*  Your Name & E-mail: Russell Villanueva & rvill101@ucr.edu

// Discussion Section: 023

 
// Assignment: EE120B Project Demo progess

 
// Exercise Description: N/A


// I acknowledge all content contained herein, excluding template or example code, is my own original work.

//  *  Russell Villanueva

 
// Demo Link: <>

//  */






// // #include <avr/io.h>
// // #include <util/delay.h>
// // #include "brisk_LCD.h"
// // #include "serialATmega-4 (1).h"
// // #include "IRremote.h"

// // #define POWER_CODE   0xBA45FF00  // a 32-bit unsigned literal
// // #define POWER2_CODE 0xE917FC03
// // #define POWEROFF_CODE 0xE51BFC03
// // #define POWEROFF2_CODE  0xB946FF00


// // // Print a 32-bit value in full 8-nibble hex (no sign)
// // void serial_printhex32(uint32_t v) {
// //     char buf[9];
// //     for (int i = 0; i < 8; ++i) {
// //         uint8_t nib = (v >> ((7 - i) * 4)) & 0xF;
// //         buf[i] = nib < 10 ? '0' + nib : 'A' + nib - 10;
// //     }
// //     buf[8] = '\0';
// //     serial_println(buf);
// // }



// // int main() {
// //     serial_init(9600);
// //     ir_init();
// //     lcd_init();
// // 	lcd_goto_xy(0, 0);
// // 	lcd_write_str("Turn On Power");
// // 	lcd_goto_xy(1, 0);
// // 	lcd_write_str("To Play");

// //     while (1) {
// //         uint32_t code = ir_read();
// //         if (code) {
// //             serial_printhex32(code);

// //             // Power On
// //             if (code == POWER_CODE || code == POWER2_CODE) {
// //                 lcd_clear();
// //                 lcd_goto_xy(0, 0);
// //                 lcd_write_str("Game Started");
// //                 lcd_goto_xy(1, 0);
// //                 lcd_write_str("Game ON");
// //             }
// //             // Power Off
// //             else if (code == POWEROFF_CODE || code == POWEROFF2_CODE) {
// //                 lcd_clear();
// //                 lcd_goto_xy(0, 0);
// //                 lcd_write_str("Shutting Down");
// //                 lcd_goto_xy(1, 0);
// //                 lcd_write_str("GoodBye :)");
// //             }
// //         }
// //     }
// // }
