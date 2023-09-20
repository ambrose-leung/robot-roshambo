// (C) Alan Ludwig, All rights reserved

#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/rand.h"
#include "ai.h"


static Move my_moves[MOVES_PER_GAME];
static Move their_moves[MOVES_PER_GAME];
static Move outcomes[MOVES_PER_GAME];
static int current_move;


int randint(int n) {
  if ((n - 1) == RAND_MAX) {
    return get_rand_32();
  } else {
    int end = RAND_MAX / n; // truncate skew
    end *= n;

    int r;
    while ((r = get_rand_32()) >= end);

    return r % n;
  }
}

void init_ai()
{
    for (int i = 0; i < MOVES_PER_GAME; ++i)
    {
        my_moves[i] = MOVE_INVALID;
        their_moves[i] = MOVE_INVALID;
    }
    current_move = 0;
}

Move random_move()
{
    int randomNum = randint(3) + 4;
    return (Move) randomNum;
}

Move ai_get_move()
{
    Move my_move;
    my_move = compute_move_based_on_last_game_outcome(20);
    my_moves[current_move % MOVES_PER_GAME] = my_move;
    return my_move;
}

void ai_their_last_move(const Move move)
{
    their_moves[current_move % MOVES_PER_GAME] = move;
    return;
}

Move get_result_of_match(const Move &them, const Move &me)
{
    Move ret;

    if (MOVE_ROCK == them)
    {
        if (MOVE_PAPER == me)
        {
            ret = MOVE_I_WIN;
        }
        else if (MOVE_SCISSORS == me)
        {
            ret = MOVE_YOU_WIN;
        }
        else
        {
            ret = MOVE_TIE;
        }
    }
    else if (MOVE_PAPER == them)
    {
        if (MOVE_SCISSORS == me)
        {
            ret = MOVE_I_WIN;
        }
        else if (MOVE_ROCK == me)
        {
            ret = MOVE_YOU_WIN;
        }
        else
        {
            ret = MOVE_TIE;
        }
    }
    else // scissors
    {
        if (MOVE_ROCK == me)
        {
            ret = MOVE_I_WIN;
        }
        else if (MOVE_PAPER == me)
        {
            ret = MOVE_YOU_WIN;
        }
        else
        {
            ret = MOVE_TIE;
        }
    }

    return ret;
}

void ai_move_complete(){
    Move theirMove = their_moves[current_move % MOVES_PER_GAME];
    Move myMove = my_moves[current_move % MOVES_PER_GAME];
    outcomes[current_move % MOVES_PER_GAME] = get_result_of_match(theirMove, myMove);
    current_move++;
    current_move %= MOVES_PER_GAME;
}

Move random_move_weighted(int r, int p, int s)
{
	int total = r+p+s;
	int x = randint(total);
	if(x < r)
		return MOVE_ROCK;
	if(x >=r && x < (r+p)) 
		return MOVE_PAPER;
	return MOVE_SCISSORS;
}

Move compute_move_based_on_aggregate(int moveNum, int takeLast = -1, int addR=0, int addP=0, int addS=0)
{
	if(takeLast == -1 || takeLast > current_move)
	{
		takeLast = current_move;
	}
    int r = 0;
    int p = 0;
    int s = 0;
    for(int i = current_move-takeLast; i < current_move; i++)
    {
        if(their_moves[i] == MOVE_ROCK)
            r++;
        if(their_moves[i] == MOVE_PAPER)
            p++;
        if(their_moves[i] == MOVE_SCISSORS)
            s++;
    }
	return random_move_weighted(s+addR,r+addP,p+addS);
}

Move compute_move_based_on_last_game_outcome(int takeLast = -1)
{
	if (current_move == 0)
	{
		return random_move();
	}
	int r = 0;
    int p = 0;
    int s = 0;
    for(int i = 0; i < current_move; i++)
    {
        if(their_moves[i] == MOVE_ROCK)
            r++;
        if(their_moves[i] == MOVE_PAPER)
            p++;
        if(their_moves[i] == MOVE_SCISSORS)
            s++;
    }

	int addR = 0;
	int addP = 0;
	int addS = 0;
	if (outcomes[current_move - 1] == MOVE_I_WIN) {
		if(my_moves[current_move - 1] == MOVE_ROCK){
			addR += (int)(current_move * .5);
		}
		if (my_moves[current_move - 1] == MOVE_PAPER)
		{
			addP += (int)(current_move * .5);
		}
		if (my_moves[current_move - 1] == MOVE_SCISSORS)
		{
			addS += (int)(current_move * .5);
		}
		
	}
	else if (outcomes[current_move - 1] == MOVE_YOU_WIN) {
		// decrease the weight of the losing play
		if (my_moves[current_move - 1] == MOVE_ROCK)
		{
			addR -= (int)(current_move * .5);
		}
		if (my_moves[current_move - 1] == MOVE_PAPER)
		{
			addP -= (int)(current_move * .5);
		}
		if (my_moves[current_move - 1] == MOVE_SCISSORS)
		{
			addS -= (int)(current_move * .5);
		}
	}

	return compute_move_based_on_aggregate(current_move,takeLast,addR,addP,addS);
}

void reset_move_number(){
    current_move = 1;
}
int get_current_move_number(){
    return current_move;
}