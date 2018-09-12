/** @file   ball.h
    @author Chuan Law (81677469), Elizabeth Wilson (53469493)
    @date   17 October 2017
    @brief  Interface for the ball module of the catch throw game
*/

#ifndef BALL_H
#define BALL_H

#include "system.h"
#include "tinygl.h"

uint8_t ball_x_cor;
uint8_t ball_y_cor;
bool ball_thrown;
uint8_t ball_ticks;

void ball_init (void);

void display_ball (void);

void change_ball_pos (uint8_t);

void set_ball_thrown (uint8_t);

void move_ball (void);

void set_ball_speed (uint8_t);

uint8_t get_ball_y_pos(void);

uint8_t get_ball_x_pos(void);

void receive_ball (uint8_t);

int8_t get_ball_direction(void);

void set_ball_direction(int8_t);

void ball_caught (uint8_t);

#endif //BALL_H
