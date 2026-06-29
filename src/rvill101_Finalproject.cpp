// /*  Your Name & E-mail: Russell Villanueva & rvill101@ucr.edu

// Discussion Section: 023

 
// Assignment: EE120B Project, Final Project submission

 
// Exercise Description: N/A


// I acknowledge all content contained herein, excluding template or example code, is my own original work.

//  *  Russell Villanueva

 
// Demo Link: <https://youtu.be/qUb6zL_hB5A>

//  */

#include "timerISR_lab7_sp2025.h"
#include "helper_lab7_sp2025.h"
#include "periph_lab7_sp2025.h"
#include "serialATmega-4 (1).h"
#include "rvill101_IRremote.h"
#include "rvill101_brisk_LCD.h"
#include <string.h>
#include <avr/interrupt.h>


// TASKS DECLARATION
#define NUM_TASKS 6 

// Power/Off Code definition
#define POWER_CODE   0xBA45FF00  // a 32-bit unsigned literal
#define POWER2_CODE 0xE917FC03
#define POWEROFF_CODE 0xE51BFC03
#define POWEROFF2_CODE  0xB946FF00

// Global Sprites
uint8_t car_char[8] = {
        0B00000,
        0B00000,
        0B11110,
        0B10001,
        0B10011,
        0B10001,
        0B11110,
        0B11110
};

// Checking last remote input
static uint32_t last_code = 0;

// Game Objects
uint8_t obstacleX = 15; 
uint8_t isJumping = 0;
uint8_t carX = 0;
uint8_t carY = 0;
uint8_t jumpTimer = 0;
bool crashed = false;
bool JUMPING = true;
bool revving = false;
bool usedRev = false;
uint8_t revTimer = 0;


// Score tracking
uint16_t score = 0;
uint16_t scoreTick = 0;




// Game Start
bool GAMEON = false;
bool OFF = false;
bool PLAYAGAIN = false;
static uint8_t playedIntro = 0;


// Song Definitions
#define NOTE_C4  261
#define NOTE_D4  294
#define NOTE_E4  329
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440

// Music sound
const uint16_t sounds[] = { NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4 };
const uint8_t durations[] = { 5, 5, 5, 5, 10 }; // Duration in ticks (100ms each)
const uint8_t num_notes = sizeof(sounds)/sizeof(sounds[0]);


// Jump sound
const uint16_t jump_sound[] = { NOTE_E4};
const uint8_t jump_duration[] = { 3 }; 

// Crash Sound
const uint16_t crash_sound[] = { NOTE_E4 };
const uint8_t crash_duration[] = { 3 };


//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;


//TODO: Define Periods for each task
// e.g. const unsined long TASK1_PERIOD = <PERIOD>
const unsigned long GCD_PERIOD = 10;//TODO:Set the GCD Period
const unsigned long ReadIr_Period = 1;
const unsigned long Lcd_Period = 20;
const unsigned long Music_Period = 100;
const unsigned long Game_Period = 200;
const unsigned long Jump_Period = 100;
const unsigned long Rev_Period = 100;




task tasks[NUM_TASKS]; // declared task array with 5 tasks


void TimerISR() {
	for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {                   // Iterate through each task in the task array
		if ( tasks[i].elapsedTime >= tasks[i].period ) {           // Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state); // Tick and set the next state for this task
			tasks[i].elapsedTime = 0;                          // Reset the elapsed time for the next tick
		}
		tasks[i].elapsedTime += GCD_PERIOD;                        // Increment the elapsed time by GCD_PERIOD
	}
}

void serial_printcode(uint32_t code) {
    char buf[15];
    strcpy(buf, "Code: ");
    for (int i = 0; i < 8; ++i) {
        uint8_t nib = (code >> ((7 - i) * 4)) & 0xF;
        buf[6 + i] = nib < 10 ? '0'+ nib : 'A'+ nib - 10;
    }
    buf[14] = '\0';
    serial_println(buf);
}




//TODO: Create your tick functions for each task

enum ReadIR_state {IR_WAIT, IR_ON};
int readIRTick(int state);


enum Lcd_state {lcdStart, waitPower, lcdON, lcdOFF};
int lcdTick(int state);


enum music_state {musicOFF, musicON, musicJump, musicCrash, musicDONE};
int musicTick(int state);

enum game_state {gameWait, gameRUNNING, gameOVER, goodBYE};
int gameTick(int state);

enum jump_state {jumpIdle, jumping};
int jumpTick(int state);


enum carRev_state {revIDLE, revON};
int revTick(int state);


ISR(TIMER0_COMPA_vect) {
    PORTD ^= (1 << PD6);    // Turn on the buzzer song
}


int main() {
    serial_init(9600);
    ADC_init();     // initializes ADC
    
    IRinit();
    PORTB |= _BV(PB0);

    // Default Display
    lcd_init();
    lcd_clear();
    lcd_goto_xy(0, 0);
    lcd_write_str(" Turn On Power");
    lcd_goto_xy(1, 0);
    lcd_write_str("    To Play");



    //TODO: initialize all your inputs and ouputs
    DDRC = 0x00;    // Setting Inputs for potentiometer and Button
    PORTC = 0xFF;   
    

    DDRD  |= SetBit(DDRD, PB2, 1);   // GREEN LED
    PORTD = SetBit(PORTD, PB2, 0);  // off initially


    DDRB  |= SetBit(DDRB, PD7, 1);   // RED LED
    PORTB = SetBit(PORTB, PD7, 0);  // off initially 

    DDRB |= SetBit(DDRB, PB1, 1);   // Active Buzzer
    PORTB = SetBit(PORTB, PB1, 0);  // off initially


    DDRD  |= SetBit(DDRD, PD6, 1);   // Passive Buzzer
    PORTD = SetBit(PORTD, PD6, 0);  // off initially
    

    //TODO: Initialize tasks here

    tasks[0].period = Lcd_Period;
    tasks[0].state = lcdStart;
    tasks[0].elapsedTime = 0;
    tasks[0].TickFct = &lcdTick;
    
    tasks[1].period = ReadIr_Period;
    tasks[1].state = IR_WAIT;
    tasks[1].elapsedTime = 0;
    tasks[1].TickFct = &readIRTick;

    tasks[2].period = Music_Period;
    tasks[2].state = musicOFF;
    tasks[2].elapsedTime = 0;
    tasks[2].TickFct = &musicTick;

    tasks[3].period = Game_Period;
    tasks[3].state = gameWait;
    tasks[3].elapsedTime = 0;
    tasks[3].TickFct = &gameTick;

    tasks[4].period = Jump_Period;
    tasks[4].state = jumpIdle;
    tasks[4].elapsedTime = 0;
    tasks[4].TickFct = &jumpTick;

    tasks[5].period = Rev_Period;
    tasks[5].state = revIDLE;
    tasks[5].elapsedTime = 0;
    tasks[5].TickFct = &revTick;


    

    TimerSet(GCD_PERIOD);
    TimerOn();

    // Ensure the sprite is created and displayed correctly
    lcd_create_char(0, car_char); // Load the car sprite into CGRAM
    lcd_goto_xy(0, 0); // Set the initial position of the car
    lcd_write_char(0); // Display the car sprite


    while (1) {}
    return 0;
}


int readIRTick(int state) {
    uint32_t code;
    if (ir_read(&code)) {
        IRresume(); // be able to read the next input
        if (code != 0xFFFFFFFF) {
            serial_printcode(code); // Prints out the code
            last_code = code; // global variable
            switch(state) {
                case IR_WAIT:
                    if (code == POWER_CODE || code == POWER2_CODE) {
                        state = IR_ON;
                    }
                    break;
                
                case IR_ON:
                    if (code == POWEROFF_CODE || code == POWEROFF2_CODE) {
                        state = IR_WAIT;
                    }
                    break;
            }

        }
   
    }
    return state;
}





int lcdTick(int state) {
    static int prevState = -1;

    switch(state) { // Transitions
        case lcdStart:
            state = waitPower;
            break;

        case waitPower:
            if (last_code == POWER_CODE || last_code == POWER2_CODE) {
                state = lcdON;
            }
            else if (last_code == POWEROFF_CODE || last_code == POWEROFF2_CODE) {
                state = lcdOFF;
            }
            break;

        case lcdON:
            if (last_code == POWEROFF_CODE || last_code == POWEROFF2_CODE) {
                state = lcdOFF;
            }
            break;


        case lcdOFF:    
            if (last_code == POWER_CODE || last_code == POWER2_CODE) {
                state = lcdON;
            }
            break;


        default:
            break;
            
    }
   
    // Only activate state actions if state is different
    if (state != prevState) {
        switch(state) { // Actions
            case lcdStart:
                break;

            case waitPower:
                break;
                
            case lcdOFF:
                // RED LED on
                PORTB &= ~(1 << PB2);
                PORTD |= (1 << PD7);
                lcd_clear();
                lcd_goto_xy(0, 0);
                lcd_write_str("Shutting Down");
                lcd_goto_xy(1, 0);
                lcd_write_str("GoodBye :)");

                // Turn off game at any moment flag
                OFF = true;
                GAMEON = false;

                break;

            case lcdON:
                lcd_clear();
                lcd_goto_xy(0, 0);
                lcd_write_str("Game Started");
                lcd_goto_xy(1, 0);
                lcd_write_str("Un Momento");

                // Delay to start up game get ready, activate start flag
                _delay_ms(1000);
                lcd_clear();
                OFF = false;
                GAMEON = true;

                break;
        
            default:
                break; 
        }
        prevState = state;
    }

    return state;
}



int musicTick(int state) {
	static uint8_t note_index = 0;  // To go through the sounds
	static uint8_t ticks_remaining = 0; // how long the SM will operate
    unsigned char btn = ((PINC & (1 << PC1)) >> PC1);

    if (btn) {
        PORTB |= SetBit(PORTB, PB1, 1);
    }
    else {
        PORTB &= SetBit(PORTB, PB1, 0);
    }


    if (crashed) {
        TIMSK0 &= ~(1 << OCIE0A);   // Once done turning off the timer0
        TCCR0A = 0;                
        TCCR0B = 0;
        PORTD &= ~(1 << PD6); // Stop playing music
    }


	switch(state) {
		case musicOFF:
            // Play music
			if (!playedIntro && (last_code == POWER_CODE || last_code == POWER2_CODE)) {
				note_index = 0;
				ticks_remaining = durations[0];

				// Setting up the use of Timer0 so the buzzer can create its sound
                // Staging setting up the timer so the passive buzzer can make the song/Intro
				TCCR0A = (1 << WGM01);
                TCCR0B = (1 << CS01);
                TCNT0 = 0;
                OCR0A = 16000000 / (8 * sounds[note_index]) / 2; // Toggles between each sound in the sound array
                TIMSK0 |= (1 << OCIE0A);

				state = musicON;       // starts the music 
			}
            if (playedIntro) {
                if (GAMEON && JUMPING) {
                    note_index = 0;
                    ticks_remaining = jump_duration[0];

                    // Setting up the use of Timer0 so the buzzer can create its sound
                    // Staging setting up the timer so the passive buzzer can make the song/Intro
                    TCCR0A = (1 << WGM01);
                    TCCR0B = (1 << CS01);
                    TCNT0 = 0;
                    OCR0A = 16000000 / (8 * jump_sound[0]) / 2; // Toggles between each sound in the sound array
                    TIMSK0 |= (1 << OCIE0A);

                    state = musicJump;
                }

                if (GAMEON && obstacleX == carX && carY == 1) {
                    note_index = 0;
                    ticks_remaining = crash_duration[0];

                    // Setting up the use of Timer0 so the buzzer can create its sound
                    // Staging setting up the timer so the passive buzzer can make the song/Intro
                    TCCR0A = (1 << WGM01);
                    TCCR0B = (1 << CS01);
                    TCNT0 = 0;
                    OCR0A = 16000000 / (8 * crash_sound[0]) / 2; // Toggles between each sound in the sound array
                    TIMSK0 |= (1 << OCIE0A);

                    state = musicCrash;
                }
            }
            
			break;

		case musicON:
			if (ticks_remaining > 0) {
				ticks_remaining--;      // Wait for the current sound to finish
			} else {
				note_index++;   // move on to the next sound
				if (note_index < num_notes) {
					OCR0A = 16000000 / (8 * sounds[note_index]) / 2;
					ticks_remaining = durations[note_index];    // setting the length of the note
				} else {
					TIMSK0 &= ~(1 << OCIE0A);   // Once done turning off the timer0
                    TCCR0A = 0;                
                    TCCR0B = 0;
					PORTD &= ~(1 << PD6); // Stop playing music
                    
                    playedIntro = true;
                    state = musicDONE;     // music is done 
				}
			}
			break;


        case musicJump:
            if (ticks_remaining > 0) {
				ticks_remaining--;      // Wait for the current sound to finish
			} else {
				note_index++;   // move on to the next sound
				if (note_index < num_notes) {
					OCR0A = 16000000 / (8 * sounds[note_index]) / 2;
					ticks_remaining = jump_duration[note_index];    // setting the length of the note
				} else {
					TIMSK0 &= ~(1 << OCIE0A);   // Once done turning off the timer0
                    TCCR0A = 0;                
                    TCCR0B = 0;
					PORTD &= ~(1 << PD6); // Stop playing music

                    JUMPING = false;
					state = musicOFF;     // music is done 
				}
			}
            break;

        case musicCrash:
           if (ticks_remaining > 0) {
				ticks_remaining--;      // Wait for the current sound to finish
			} else {
				note_index++;   // move on to the next sound
				if (note_index < num_notes) {
					OCR0A = 16000000 / (8 * sounds[note_index]) / 2;
					ticks_remaining = jump_duration[note_index];    // setting the length of the note
				} else {
					TIMSK0 &= ~(1 << OCIE0A);   // Once done turning off the timer0
                    TCCR0A = 0;                
                    TCCR0B = 0;
					PORTD &= ~(1 << PD6); // Stop playing music
					state = musicOFF;     // music is done 
				}
			}
            break;

        
		case musicDONE:
            state = musicOFF;
			break;
	}

   
	return state;
}


int gameTick(int state) {
    static uint8_t hasDisplayed = 0;

    switch(state) {
		case gameWait:
			if (GAMEON) {
                OFF = false;
                crashed = false;
				score = 0;
				carY = 1;
				isJumping = 0;
				obstacleX = 15;
                revving = false;
                usedRev = false;
				state = gameRUNNING;
			}
			break;
		case gameRUNNING:
			if (OFF) {
				state = gameWait;
			}

            // Green LED on
            PORTB |= (1 << PB2);
            PORTD &= ~(1 << PD7);


            // Check collision BEFORE drawing
            if (obstacleX == carX && carY == 1) {
                crashed = true;
                OFF = true;
                _delay_ms(1000);
                state = gameOVER;
            }
			break;
		case gameOVER:
            // RED LED on
            PORTB &= ~(1 << PB2);
            PORTD |= (1 << PD7);

            if (PLAYAGAIN) {
                crashed = false;
                PLAYAGAIN = false;
                lcd_clear();
                state = gameWait;
            }

            else if (!PLAYAGAIN) {
                state = goodBYE;
            }
			break;

        case goodBYE:
            if (PLAYAGAIN) {
                crashed = false;
                PLAYAGAIN = false;
                lcd_clear();
                state = gameWait;
            }
            break;
	}

	switch(state) {
		case gameWait:
			break;

		case gameRUNNING:
            clear_char(1, obstacleX);
            clear_char(0, carX);
            clear_char(1, carX);

            obstacleX--;
            if (obstacleX == 255) obstacleX = 15;  // wrap around if underflows

            if (OFF) {
                state = gameWait;
            }

            if(revving) {
                lcd_goto_xy(carY, carX);
                lcd_write_char(0);
                lcd_goto_xy(1, obstacleX);
                lcd_write_char('#');
            }


            if (!revving) {
                _delay_ms(15);
                lcd_goto_xy(carY, carX);
                lcd_write_char(0);
                lcd_goto_xy(1, obstacleX);
                lcd_write_char('#');
            }
            

            if (++scoreTick >= 6) {
                score++;
                scoreTick = 0;
            }
			break;

		case gameOVER:
            if (!hasDisplayed) {
                PORTB &= SetBit(PORTB, PB1, 0); // No horn in game over

                lcd_clear();
                lcd_goto_xy(0, 0); lcd_write_str("GM Over");
                lcd_goto_xy(1, 0); lcd_write_str("Score:");
                lcd_write_int(score);
                _delay_ms(4000);
            }
			

			break;
        
        case goodBYE:
            crashed = true;
            isJumping = 1;

            PORTB &= SetBit(PORTB, PB1, 0);  // Don't buzzer when you lose

            lcd_clear();
            lcd_goto_xy(0, 0);
            lcd_write_str("PowerDwn PG? Y/N");
            lcd_goto_xy(1, 0);
            lcd_write_str("GoodBye :)");

            if (last_code != 0) {
                crashed = false;
                PLAYAGAIN = true;
            }

            break;
            
	}
    
    return state;
}

int jumpTick(int state) {
	if (!GAMEON || OFF) { isJumping = 0; return jumpIdle;} 

	switch(state) {
		case jumpIdle:
			if (!isJumping && last_code != 0) {
				isJumping = 1;
                JUMPING = true;
				jumpTimer = 10;
				state = jumping;
			}
			break;

		case jumping:
			if (jumpTimer > 0) jumpTimer--;
			else {
				isJumping = 0;
				state = jumpIdle;
			}
			break;
	}

	carY = isJumping ? 0 : 1;   // If the car isJumping it gets moved to row 0 otherwise goes to row 1
	last_code = 0; 

	return state;
}




int revTick(int state) {
    unsigned int revVAL = ADC_read(2);

    switch(state) { // Transisitions
        case revIDLE:
            if (!usedRev && GAMEON && revVAL > 700) {
                usedRev = true;
                revTimer = 30;
                revving = true;
                score += 20;
                state = revON;
            }
            break;
        
        case revON:
            if (revTimer == 0) {
                revving = false;
                state = revIDLE;
            }
            break;

    }

    switch(state) { // Actions
        case revIDLE:
            break;

        case revON:
            if (revTimer > 0) {
                revTimer--;
            }
            break;
    }


    return state;
}


