/** @file   player.c
    @author Chuan Law (81677469), Elizabeth Wilson (53469493)
    @date   17 October 2017
    @brief  A module to handle the player of the throw
            and catch game
*/

#include <stdbool.h>
#include "system.h"
#include "player.h"
#include "ball.h"
#include "tinygl.h"

/* Variable initialization */
uint8_t player_pos; // Column number of the player
bool player_has_ball = false; // Indicates whether the player is holding the ball

#define PLAYER_START_ROW (LEDMAT_ROWS_NUM / 2) // Sets start row to the mid point of the LED matrix
#define PLAYER_START_COL (LEDMAT_COLS_NUM - 1) // Sets start column to be the bottom column

#define PACER_RATE 100


/*
 * Called for both players
 * Draws the player paddle at fixed location
 * Checks if player starts with ball and draws it
 */
void player_init(void) {
    tinygl_init(PACER_RATE);
    player_pos = PLAYER_START_ROW;
    tinygl_draw_point(tinygl_point (PLAYER_START_COL, player_pos), 1);
    if (player_has_ball)
        ball_init(); // Initialises the ball above the player
}


/*
 * Simple function used to update the tinygl with any changes
 */
void display_player(void) {
    tinygl_update();
}


/*
 * Changes player position left or right depending on navswitch push
 * Checks if player has the ball currently and moves it along with the
 * player movement.
 * @param int i defined int value either 1 (right) or -1 (left)
 */
void change_player_pos(int position) {
    if ((player_pos < 6 && position == 1) || (player_pos > 0 && position == -1)) {
        tinygl_draw_point (tinygl_point (4, player_pos), 0);
        player_pos += position;
        tinygl_draw_point (tinygl_point (4, player_pos), 1);
        if (player_has_ball)
            change_ball_pos (position);
        tinygl_update ();
    }
}


/*
 * Sets player to not have the ball
 */
void throw_ball(void) {
    player_has_ball = false;
}


/*
 * Sets the player to have the ball
 */
void catch_ball(void) {
    player_has_ball = true;
}


/*
 * To get current player position (row number)
 */
uint8_t get_player_pos(void) {
    return player_pos;
}
