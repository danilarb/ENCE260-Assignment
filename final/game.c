/** @file   game.c
 *  @author D. Bublik & C. O'Reilly
 *  @date   14 October 2020
 *  @brief  Rock, paper, scissors game
 */


#include <string.h>
#include <stdlib.h>
#include "pio.h"
#include "system.h"
#include "button.h"
#include "display.h"
#include "../fonts/font5x7_1.h"
#include "font.h"
#include "pacer.h"
#include "tinygl.h"
#include "task.h"
#include "navswitch.h"
#include "led.h"
#include "ir_uart.h"
#include "uint8toa.h"
#include "timer0.h"
// Our module (initialises/defines some variables/structs)
#include "init_m.h"

static state_t state = STATE_INIT; // Initialisation state
static game_data_t game_data;

// Initialises game_data variables
void user_init_task (void)
{
    game_data.win = 0;
    game_data.send = 0;
    game_data.sent = 0;
    game_data.sentLT = 0;
    game_data.receive = 0;
    game_data.gameRec = 0;
    game_data.gameNumber = 1;
    game_data.gameString = '1';
    game_data.gameCurrent = 1;
    game_data.userScore = '0';
    if (state == STATE_INIT) {
        state = STATE_INITCOMPLETE;
    }
}


// Resets necesary game_data variables
void user_reset (void)
{
    game_data.win = 0;
    game_data.send = 0;
    game_data.sent = 0;
    game_data.sentLT = 0;
    game_data.receive = 0;
    game_data.gameCurrent++;
    state = STATE_SELECT;
}

// Basic display function using tinygl
void display_character (char character)
{
    char buffer[2];
    buffer[0] = character;
    buffer[1] = '\0';
    tinygl_text (buffer);
}

// Mass display task for all different states. Utilises display_character()
void display_task (void)
{
    // Shows game_data.gameString if we're in STATE_LEVEL
    if (state == STATE_LEVEL) {
        display_character (game_data.gameString);
    }
    if (state == STATE_SELECT) {
        display_character (game_data.send);
    }
    if (state == STATE_TRANSFER) {
        display_character (game_data.send);
    }
    // Displays W/D/L depending on win/draw/loss also error checking
    if (state == STATE_GAME) {
        if (game_data.win == 1) {
            display_character ('W'); // Win
        } else if (game_data.win == 2) {
            display_character ('D'); // Draw
        } else if (game_data.win == 3) {
            display_character ('L'); // Loss
        } else if (game_data.win  == 4) {
            display_character ('E'); // Error in game_data.send or game_data.receive!
        } else {
            display_character ('N'); // Error in game_data.win variable!
        }
    }
    // Displays your final score
    if (state == STATE_END) {
        display_character (game_data.userScore);
    }
    tinygl_update ();
}

// Compares between what you and your opponent have selected (R/P/S)
void compare_task (void)
{
    if (state == STATE_COMPARE) {
        // Win = 1, Draw = 2, Loss = 3, Error = 4
        if ((game_data.send == 'R') && (game_data.receive == 'S')) {
            game_data.win = 1;
        } else if ((game_data.send == 'R') && (game_data.receive == 'R')) {
            game_data.win = 2;
        } else if ((game_data.send == 'R') && (game_data.receive == 'P')) {
            game_data.win = 3;
        } else if ((game_data.send == 'P') && (game_data.receive == 'R')) {
            game_data.win = 1;
        } else if ((game_data.send == 'P') && (game_data.receive == 'P')) {
            game_data.win = 2;
        } else if ((game_data.send == 'P') && (game_data.receive == 'S')) {
            game_data.win = 3;
        } else if ((game_data.send == 'S') && (game_data.receive == 'P')) {
            game_data.win = 1;
        } else if ((game_data.send == 'S') && (game_data.receive == 'S')) {
            game_data.win = 2;
        } else if ((game_data.send == 'S') && (game_data.receive == 'R')) {
            game_data.win = 3;
        } else {
            if (game_data.receive != 0) {
                game_data.win = 4; // ERROR!
            }
        }
        // If you win, your score increments by 1
        if (game_data.win == 1) {
            game_data.userScore++;
        }
        // New state once function happens (very quickly)
        state = STATE_GAME;
    }
}

// Task to transfer IR transmission for 'R'/'P'/'S'
// Only occurs if state == STATE_TRANSFER
void transfer_task (void)
{
    if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
        ir_uart_putc(game_data.send);
        // Easy way to know if we sent it
        game_data.sent = 1;
    }
    // If we have sent and received something go to STATE_COMPARE
    if (game_data.receive != 0 && game_data.sent == 1) {
        state = STATE_COMPARE;
    }
}

// Task to transfer IR for how many games to play
void transfer_lt (void)
{
    if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
        ir_uart_putc(game_data.gameNumber);
        game_data.sentLT = 1;
    }
    if (game_data.gameRec != 0 && game_data.sentLT == 1) {
        // Number of games you play is the average of you and your opponent's choice (rounded down)
        game_data.gameNumber = (game_data.gameNumber + game_data.gameRec) / 2;
        // If the num of games is even, add 1. This way it's more likely for there to be an ultimate winner!
        if (game_data.gameNumber % 2 == 0) {
            game_data.gameNumber++;
        }
        state = STATE_SELECT;
    }
}

// Changes what navswitches do for each state
void navswitch_task (void)
{
    if (state == STATE_SELECT) {
        // Selection of Rock, Paper or Scissors
        if (game_data.send == 0) {
            game_data.send = 'R';
        }
        if (navswitch_push_event_p (NAVSWITCH_EAST)) {
            game_data.send = 'R';
        }
        if (navswitch_push_event_p (NAVSWITCH_WEST)) {
            game_data.send = 'S';
        }
        if (navswitch_push_event_p (NAVSWITCH_NORTH)) {
            game_data.send = 'P';
        }
        // Push south to save it
        if (navswitch_push_event_p (NAVSWITCH_SOUTH)) {
            state = STATE_TRANSFER;
        }
    }
    if (state == STATE_LEVEL) {
        // EAST/WEST to increment/decrement number of games that you want to play
        // Cannot go above GAME_MAX_NUM games or below 1 game
        if (navswitch_push_event_p (NAVSWITCH_EAST)) {
            game_data.gameNumber++;
            game_data.gameString++;
            if (game_data.gameNumber >= GAME_MAX_NUM) {
                game_data.gameNumber = GAME_MAX_NUM;
                game_data.gameString = GAME_MAX_CHA;
            }
        }
        if (navswitch_push_event_p (NAVSWITCH_WEST)) {
            game_data.gameNumber--;
            game_data.gameString--;
            if (game_data.gameNumber == 0) {
                game_data.gameNumber = 1;
                game_data.gameString = '1';
            }
        }
        // Push south to save
        if (navswitch_push_event_p (NAVSWITCH_SOUTH)) {
            state = STATE_LT;
        }
    }
    // Push button1 to go to the next game
    if (state == STATE_GAME) {
        if (button_push_event_p (BUTTON1)) {
            if (game_data.gameCurrent < game_data.gameNumber) {
                user_reset();
            } else {
                // Happens once you've reached max num of games
                state = STATE_END;
            }
        }
    }

    if (state == STATE_END) {
        if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
            user_init_task ();
            state = STATE_LEVEL;
        }
    }
    // Updates button and navswitch to allow it to detect pushes
    button_update ();
    navswitch_update ();
}

// Initialises most things
void game_init (void)
{
    if (state == STATE_INIT) {
        // System initialisation
        system_init ();
        // TinyGL initialisation with the DISPLAY_TASK_RATE
        tinygl_init (DISPLAY_TASK_RATE);
        // Sets tinygl font
        tinygl_font_set (&font5x7_1);
        // Initialises pacer (while loop)
        pacer_init(CLOCK_HERTZ);
        navswitch_init();
        button_init ();
        // Our user_init_task to initialise game_data struct
        user_init_task();
        ir_uart_init();
    }
}

// Main game function
int main(void)
{
    // Initialisation
    game_init ();
    // State becomes STATE_LEVEL to choose level
    state = STATE_LEVEL;
    // Main loop
    while (1) {
        // Waits CLOCK_HERTZ Hz
        pacer_wait ();
        // Does functions after waiting, some are state-dependent
        navswitch_task ();
        display_task ();

        if (state == STATE_LT) {
            transfer_lt ();
        }
        if (state == STATE_TRANSFER) {
            transfer_task ();
        }
        if (state == STATE_COMPARE) {
            compare_task ();
        }
        // If ir has received something
        if (ir_uart_read_ready_p()) {
            if (state == STATE_LT) {
                game_data.gameRec = ir_uart_getc();
            }
            if (state == STATE_TRANSFER) {
                game_data.receive = ir_uart_getc();
            }
        }

    }
    return 0;
}

