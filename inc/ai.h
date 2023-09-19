#pragma once

#include "game.h"

// (C) Alan Ludwig, All rights reserved

void init_ai();
Move ai_get_move();
void ai_their_last_move(const Move move);
void ai_move_complete();
int get_current_move_number();
Move compute_move_based_on_last_game_outcome(int takeLast);