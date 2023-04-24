#ifndef INIT_M_H
#define INIT_M_H

// Max number of games
#define GAME_MAX_CHA '5'
#define GAME_MAX_NUM 5
// Clockspeed
#define CLOCK_HERTZ 1000
// Display speed
#define DISPLAY_TASK_RATE 200

// All different states to be used
typedef enum {STATE_INIT, STATE_INITCOMPLETE, STATE_SELECT, STATE_TRANSFER, STATE_LEVEL, STATE_LT, STATE_COMPARE, STATE_GAME, STATE_END} state_t;

// game_data_t struct which we use to store most player variables
typedef struct {
    uint8_t send; // 'R'/'P'/'S' to send to opponent
    uint8_t sent; // Whether game_data.send has been sent (used in transfer_task())
    uint8_t sentLT; // Whether the level has been sent (used in transfer_LT())
    uint8_t receive; // 'R'/'P'/'S' sent by opponent received by us
    uint8_t userScore; // Current score for user (number of wins only)
    uint8_t gameCurrent; // Current game being played
    uint8_t gameNumber; // How many games you want to play (max is GAME_MAX_NUM)
    uint8_t gameRec; // How many games opponent wants to play
    char gameString; // gameNumber as a string
    uint8_t win; // Used for scoring. 1=W 2=D 3=L
} game_data_t;
#endif
