/** @file   player.c
    @author Chuan Law (81677469), Elizabeth Wilson (53469493)
    @date   17 October 2017
    @brief  A module to handle the ball of the throw
            and catch game
*/

#include <stdbool.h>
#include "system.h"
#include "ball.h"
#include "tinygl.h"

/* Used for the direction the ball is going */
#define UP 1
#define DOWN (-1)

/*
 * Speeds for the motion of the ball
 * Indicates the number of ticks between movements of the ball
 */
#define SPEED_SLOW 90;
#define SPEED_MEDIUM 40;
#define SPEED_FAST 20;

#define BALL_START_ROW (LEDMAT_ROWS_NUM / 2) // Sets start row to the mid point of the LED matrix
#define BALL_START_COL (LEDMAT_COLS_NUM - 2) // Sets start column to be one column above the player

/* Ball position initialisation */
uint8_t ball_x_cor;
uint8_t ball_y_cor;


/* Ball characteristics initialisation*/
bool ball_thrown = false;
uint8_t ball_ticks = 0;
uint8_t ball_speed;
int8_t ball_direction = UP;


/*
 * Only called for the player that starts with the ball
 * Initialises with ball to be above the player
 */
void ball_init (void) {
    ball_x_cor = BALL_START_ROW;
    ball_y_cor = BALL_START_COL;
    ball_thrown = false;
    tinygl_draw_point (tinygl_point (ball_y_cor, ball_x_cor), 1);
    tinygl_update ();
}


/**
 * Called by receiving board to load the position
 * the ball was sent from. Position is then used to draw the
 * point on the corresponding board
 */
void receive_ball (uint8_t position) {
    ball_y_cor = 0;
    ball_x_cor = position;
    tinygl_draw_point (tinygl_point (ball_y_cor, ball_x_cor), 1);
}


/**
 * Used to set the amount of ticks to delay the speed at which
 * the ball is flashed.
 * @param int i actual speed value chosen by user via navswitch input
 */
void set_ball_speed (uint8_t speed_index) {
    if (speed_index == 0) {
        ball_speed = SPEED_SLOW;
    } else if (speed_index == 1) {
        ball_speed = SPEED_MEDIUM;
    } else if (speed_index == 2) {
        ball_speed = SPEED_FAST;
    } else {
        ball_speed = SPEED_MEDIUM; // Default for the ball speed
    }
}


/*
 * To check if the ball has been thrown calls the move_ball()
 * function if it has been. This displays the ball moving.
 */
void display_ball (void) {
    if (ball_thrown)
        move_ball ();
}


/*
 * Sets the ball to not being thrown which stops it moving.
 * Draws the now stationary point to row 3 and right above the
 * player position
 * Ball follows player after this is called
 * @param uint8_t player pos the row position the player is currently on
 */
void ball_caught (uint8_t player_pos) {
    ball_x_cor = player_pos;
    ball_y_cor = 3;
    ball_thrown = false;
    tinygl_draw_point (tinygl_point (ball_y_cor, ball_x_cor), 1);
}


/*
 * To make the ball stick with the player movement
 * @param int i the current player row position
 */
void change_ball_pos (uint8_t x_position) {
    tinygl_draw_point (tinygl_point (ball_y_cor, ball_x_cor), 0);
    ball_x_cor += x_position;
    tinygl_draw_point (tinygl_point (ball_y_cor, ball_x_cor), 1);
}


/*
 * Increments ticks as a delay until it matches the chosen ball speed
 * Resets the tick counter then moves the ball down the column
 */
void move_ball (void) {
    if (ball_ticks == ball_speed) {
        ball_ticks = 0;
        tinygl_draw_point (tinygl_point (ball_y_cor, ball_x_cor), 0);
        ball_y_cor -= ball_direction * 1;
        tinygl_draw_point (tinygl_point (ball_y_cor, ball_x_cor), 1);
    }
    ball_ticks++;
}


/* To get y coordinate of the ball n */
uint8_t get_ball_y_pos(void) {
    return ball_y_cor;
}


/* To get x coordinate of the ball */
uint8_t get_ball_x_pos(void) {
    return ball_x_cor;
}


/* To set that a ball has been thrown */
void set_ball_thrown (uint8_t thrown) {
    ball_thrown = thrown;
}


/* To get ball direction */
int8_t get_ball_direction(void) {
    return ball_direction;
}


/* To set the direction the ball is thrown */
void set_ball_direction(int8_t direction) {
    ball_direction = direction;
}
