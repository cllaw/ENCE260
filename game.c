/** @file   game.c
    @author Chuan Law (81677469), Elizabeth Wilson (53469493)
    @date   17 October 2017
    @brief  A simple throw and catch game, which gives the players
            different speed options and includes sound affects. The
            game may be played with multiple rounds, and keeps track
            of each players scores
*/

#include <stdbool.h>
#include <stdint.h>
#include "system.h"
#include "task.h"
#include "player.h"
#include "ball.h"
#include "navswitch.h"
#include "ir_uart.h"
#include "tinygl.h"
#include "../fonts/font3x5_1.h"
#include "mmelody.h"
#include "ticker.h"
#include "tweeter.h"
#include "pio.h"

// Defining tasks rates for the different tasks
#define TWEETER_TASK_RATE 5000
#define TUNE_TASK_RATE 100
#define DISPLAY_TASK_RATE 250
#define GAME_TASK_RATE 100
#define IR_TASK_RATE 100
#define NAVSWITCH_TASK_RATE 100

#define MIN_POS 0
#define MAX_ROW_POS (LEDMAT_ROWS_NUM - 1)
#define MAX_COL_POS (LEDMAT_COLS_NUM - 1)

// Defining rates to initialise tinygl with
#define PACER_RATE 500
#define MESSAGE_RATE 50

// Used for direction that the player is moving
#define LEFT (-1)
#define RIGHT 1

// Used for the direction that the ball is going
#define UP 1
#define DOWN (-1)

// The outcome of the game
#define LOSE 7
#define WIN 8

// The speed to initialise mmelody with
#define TUNE_BPM 200

// States of the game play, to keep track of the game state
typedef enum {STATE_INIT, STATE_SETUP, STATE_PLAYING,
              STATE_OVER} state_t;

// Initialising the game state
static state_t game_state = STATE_INIT;

char speeds[] = {'1', '2', '3'}; // Chars of speeds the player can choose from
bool speed_chosen = false;
uint8_t speed_index = 0;
bool ball_on_screen = false; // Indicates whether the ball is on the players screen

uint8_t game_outcome = 2;
bool reset = false; // Used to trigger a reset to the game in order to restart a new round
uint8_t score = 0; // To keep track of the players score over multiple games

// Connect piezo tweeter to the first and third pin
#define PIEZO1_PIO PIO_DEFINE (PORT_D, 4)
#define PIEZO2_PIO PIO_DEFINE (PORT_D, 6)

// Initializing variables used for the sound effects and music
static tweeter_scale_t scale_table[] = TWEETER_SCALE_TABLE (TWEETER_TASK_RATE);
static tweeter_t tweeter;
static mmelody_t melody;
static mmelody_obj_t melody_info;
static tweeter_obj_t tweeter_info;

/*
 * The song played on the winners board at the end of each round
 * of the game
 */
static const char win_tune[] =
{
#include "win_song.mmel"
"> "
};


/*
 * Sends the given uint8_t to_send over IR to the other board
 */
void send_ir(uint8_t to_send)
{
    ir_uart_putc(to_send);
}


/*
 *  Initialisation for the tweeter task. Configures the pins for 
 *  output and initialises the tweeter
 */
static void tweeter_task_init (void)
{
    tweeter = tweeter_init (&tweeter_info, TWEETER_TASK_RATE, scale_table);

    pio_config_set (PIEZO1_PIO, PIO_OUTPUT_LOW);
#ifdef PIEZO2_PIO
    pio_config_set (PIEZO2_PIO, PIO_OUTPUT_LOW);
#endif
}


/*
 * Initialises the tweeter for the music and sound effects
 */
static void tweeter_task (__unused__ void *data)
{
    bool state;
    state = tweeter_update (tweeter);

    pio_output_set (PIEZO1_PIO, state);
#ifdef PIEZO2_PIO
    pio_output_set (PIEZO2_PIO, !state);
#endif
}


/*
 * Initialises mmelody for the tune task
 */
static void tune_task_init (void)
{
    melody = mmelody_init (&melody_info, TUNE_TASK_RATE,
			   (mmelody_callback_t) tweeter_note_play, tweeter);

    mmelody_speed_set (melody, TUNE_BPM);
}


/*
 * Updates mmelody, keeps the song playing
 */
static void tune_task (__unused__ void *data)
{
    mmelody_update (melody);
}


/**
 * Handles displaying messages, the player and the ball
 * at different stages of the game
 */
static void display_task (__unused__ void *data)
{
    static bool init = false;
    static bool game_init = false;

    if (!init) {
        tinygl_init(PACER_RATE);
        tinygl_font_set(&font3x5_1);
        tinygl_text_dir_set(TINYGL_TEXT_DIR_ROTATE);
        tinygl_text_mode_set(TINYGL_TEXT_MODE_SCROLL);
        tinygl_text_speed_set(MESSAGE_RATE);
        tinygl_text("CATCH! PRESS TO CHOOSE SPEED");
        init = true;
    }

    switch (game_state) {
        static bool game_over_init = false;

        case STATE_INIT:
            tinygl_update();
            break;
        case STATE_SETUP: ; // Empty statement so that the label may be followed by a declaration
            char speed[] = {speeds[speed_index], '\0'}; // to display the speed as a char
            tinygl_text(speed);
            tinygl_update();
            break;
        case STATE_PLAYING:
            if (!game_init) {
                player_init();
                game_init = true;
            }
            display_ball();
            tinygl_update();
            break;
        case STATE_OVER:
            if (!game_over_init) {
                char buffer[] = {score + '0', '\0'}; // to display the score as a char
                tinygl_text(buffer);
                tinygl_update();
                game_over_init = true;
            }
            tinygl_update();
            if (reset) {
                tinygl_clear();
                tinygl_text_mode_set(TINYGL_TEXT_MODE_SCROLL);
                tinygl_text("CATCH! PRESS TO CHOOSE SPEED");
                tinygl_text_speed_set(10);
                game_init = false;
                game_over_init = false;
            }
            break;
    }
}


/**
 * Handles game tasks throughout the game.
 * This includes checking where the ball is and
 * triggering other events based on this:
 * Triggers the balls position to be sent when it reaches the
 * top of the players screen
 * Also checks whether the player has caught the ball when it
 * reaches the bottom of the screen and causes the game
 * to end if the player does not catch the ball
 */
static void game_task (__unused__ void *data)
{
    switch (game_state) {
        case STATE_INIT:
            break;
        case STATE_SETUP:
            break;
        case STATE_PLAYING: ; // Empty statement so that the label may be followed by a declaration
            uint8_t x_pos = get_ball_x_pos ();
            uint8_t y_pos = get_ball_y_pos ();
            if (ball_on_screen && get_ball_direction() == UP && y_pos == 0) { // Checks if the ball has reached the top
                send_ir(TINYGL_HEIGHT - x_pos - 1); // Reverses the position for the other board
                ball_on_screen = false;
            } else if (ball_on_screen && get_ball_direction() == DOWN && y_pos == MAX_COL_POS) {
                    if (x_pos == get_player_pos()) {
                        catch_ball();
                        mmelody_play(melody, "C");  // Beeps when the player catches the ball
                        ball_caught(get_player_pos());
                        set_ball_direction(UP);
                    } else {
                        send_ir(WIN); // Indicating to the other player that they have won
                        game_outcome = LOSE;
                        game_state = STATE_OVER;
                        tinygl_clear();
                    }
            }
            break;
        case STATE_OVER:
            break;
    }
}


/**
 * Handles navswitch events in different stages of the game
 * In setup it triggers the displayed speed option to change
 * with the north/south navswitch press
 * In gameplay it triggers the player to move with the
 * north/south navswitch and causes the ball to be thrown when
 * the navswitch it pushed
 */
static void navswitch_task (__unused__ void *data)
{
    static bool init = false;

    if (!init) {
        navswitch_init();
        init = true;
    }

    navswitch_update ();

    switch (game_state) {
        case STATE_INIT:
            if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
                game_state = STATE_SETUP;
                tinygl_text_mode_set(TINYGL_TEXT_MODE_STEP);
                tinygl_clear();
            }
            break;
        case STATE_SETUP:
            if (navswitch_push_event_p(NAVSWITCH_WEST)) {
                if (speed_index < 2)
                    speed_index ++;
            }
            if (navswitch_push_event_p(NAVSWITCH_EAST)) {
                if (speed_index > 0)
                    speed_index --;
            }
            if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
                speed_chosen = true;
                set_ball_speed(speed_index);
                ball_on_screen = true;
                player_has_ball = true;
                tinygl_clear();
            }
            break;
        case STATE_PLAYING:
            if (navswitch_push_event_p(NAVSWITCH_NORTH))
                change_player_pos(LEFT);
            if (navswitch_push_event_p(NAVSWITCH_SOUTH))
                change_player_pos(RIGHT);
            if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
                    throw_ball();
                    set_ball_thrown(true);
                    mmelody_play(melody, "E");  // Beeps when the player throws the ball
            }
            break;
        case STATE_OVER:
            if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
                send_ir(true); // Sends 1 to the other board to start a new game
                reset = true;
            }
            break;
    }
}


/**
 * Function to reset all the necessary game variables in order
 * to replay the game
 */
void reset_game(void)
{
    game_state = STATE_INIT;
    ball_on_screen = false;
    speed_chosen = false;
    speed_index = 0;
    player_has_ball = false;
    reset = false;
    mmelody_play(melody, "");
    set_ball_direction(UP);
    set_ball_thrown(false);
}


/**
 * Handles sending and receiving over IR to and from the other board
 * Triggers events to happen based on what is received
 */
static void send_recv_task (__unused__ void *data)
{
    static bool init = false;
    // reset_tick allows one cycle for all the tasks to reinitialise for a new round
    static uint8_t reset_tick = 0;

    if (!init) {
        ir_uart_init ();
        init = true;
        reset_tick = 0;
    }

    switch(game_state) {
        case STATE_INIT:
            break;
        case STATE_SETUP:
            if (speed_chosen) {
                ir_uart_putc(speed_index); // sending the speed to the other board
            }
            if (ir_uart_read_ready_p()) {
                char actual_speed = ir_uart_getc();
                set_ball_speed(actual_speed);
                speed_chosen = true;
            }
            if (speed_chosen) {
                 game_state = STATE_PLAYING; // changes to state_playing when speed is chosen
            }
            break;
        case STATE_PLAYING:
             if (ir_uart_read_ready_p()) {
                char received = ir_uart_getc();
                if (received == WIN) {
                    game_outcome = WIN;
                    score++;
                    game_state = STATE_OVER; // changes to state over when win condition is met
                    tinygl_clear();
                    mmelody_play(melody, win_tune); // only winning board plays melody
                } else if (received >= MIN_POS && received <= MAX_ROW_POS) { // game still being played
                    receive_ball (received);
                    set_ball_direction(DOWN);
                    ball_on_screen = true;
                    set_ball_thrown(true);
                }
             }
            break;
        case STATE_OVER:
            if (ir_uart_read_ready_p()) {
                char received = ir_uart_getc();
                if (received == true) {
                    reset = true;
                }
            }
            if (reset && reset_tick == 1)
            {
                reset_game(); // reset all necessary variables and return to state_init, to replay game
                init = false;
            }
            if (reset) {
                reset_tick = 1 - reset_tick; // toggles between 0 and 1
            }
            break;
    }
}


int main (void)
{
    task_t tasks[] =
    {
            {.func = tweeter_task, .period = TASK_RATE / TWEETER_TASK_RATE, .data = 0},
            {.func = tune_task, .period = TASK_RATE / TUNE_TASK_RATE, .data = 0},
            {.func = display_task, .period = TASK_RATE / DISPLAY_TASK_RATE, .data = 0},
            {.func = game_task, .period = TASK_RATE / GAME_TASK_RATE, .data = 0},
            {.func = send_recv_task, .period = TASK_RATE / IR_TASK_RATE, .data = 0},
            {.func = navswitch_task, .period = TASK_RATE / NAVSWITCH_TASK_RATE, .data = 0},
    };

    system_init ();
    tweeter_task_init ();
    tune_task_init ();

    task_schedule (tasks, ARRAY_SIZE (tasks));

    return 0;
}
