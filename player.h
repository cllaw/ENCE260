/** @file   player.h
    @author Chuan Law (81677469), Elizabeth Wilson (53469493)
    @date   17 October 2017
    @brief  Interface for the player module of the catch throw game
*/

#ifndef PLAYER_H
#define PLAYER_H

#include "system.h"
#include "tinygl.h"

bool player_has_ball;

void player_init (void);

void display_player (void);

void change_player_pos (int);

void throw_ball (void);

void catch_ball (void);

uint8_t get_player_pos(void);

#endif
