// (C) Alan Ludwig, All Rights Reserved.

#include "pico/stdlib.h"
#include "pico/util/queue.h"

#include <string>
#include "error.h"
#include "game.h"
#include "uart.h"
#include "ai.h"
#include "led.h"

queue_t move_fifo;
Game_State game_state;

Move my_move;
Move their_move;

bool gameIsInSession = false;

void set_isGameInSession(bool val){
    gameIsInSession = val;
    reset_move_number();
}

bool get_isGameInSession(){
    return gameIsInSession;
}

Game_State game_get_state()
{
    return game_state;
}

Move get_result(const Move &them, const Move &me)
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

    ai_their_last_move(them);
    ai_move_complete();

    return ret;
}

int init_game_engine()
{
    queue_init(&move_fifo, sizeof(Move), MOVE_FIFO_LENGTH);
    game_state = GAME_STATE_IDLE;
    return 0;
}

int game_push_move(Move move)
{
    bool ret = false;
    ret = queue_try_add(&move_fifo, &move);

    // Return 0 on success or
    return ret ? ERROR_SUCCESS : ERROR_MOVE_QUEUE_FULL;
}

void send_move(const Move &move)
{
    std::string msg = "ERROR";
    switch (move)
    {
    case MOVE_PLAY:
        msg = "PLAY?";
        break;
    case MOVE_YES:
        msg = "YES!";
        break;
    case MOVE_ROCK:
        msg = "ROCK";
        break;
    case MOVE_PAPER:
        msg = "PAPER";
        break;
    case MOVE_SCISSORS:
        msg = "SCISSORS";
        break;
    case MOVE_YOU_WIN:
        msg = "YOU WIN";
        break;
    case MOVE_I_WIN:
        msg = "I WIN";
        break;
    case MOVE_TIE:
        msg = "TIE";
        break;
    case MOVE_GAME:
        msg = "GAME";
        break;
    default:
        msg = "ERROR";
        break;
    }
    //uncomment below for debugging purposes
    // std::string moveNum = std::to_string(get_current_move_number());
    // std::string gameIsPlaying = std::to_string(get_isGameInSession());
    // msg = msg + "_" + moveNum + "-" + gameIsPlaying;
    uart_puts(UART_ID, msg.append("\n").c_str());
    return;
}

int game_process_moves()
{
    int ret = ERROR_SUCCESS;
    Move move;
    while (queue_try_remove(&move_fifo, &move))
    {
        // uncomment to echo the recieved command
        // send_move(move);
        switch (game_state)
        {
        case GAME_STATE_IDLE:
            switch (move)
            {
            case MOVE_START:
                send_move(MOVE_PLAY);
                game_state = GAME_STATE_INVITE;
                break;
            case MOVE_GAME:
                send_move(MOVE_GAME);
                game_state = GAME_STATE_INVITE;
                break;
            case MOVE_PLAY:
                send_move(MOVE_YES);
                led_clear();
                // Call the AI here and get a move
                my_move = ai_get_move();
                send_move(my_move);
                game_state = GAME_STATE_MOVE;
                break;
            default:
                send_move(MOVE_ERROR);
                // we're already in the idle state.
                break;
            }
            break;
        case GAME_STATE_INVITE:
            switch (move)
            {
            case MOVE_YES:
                my_move = ai_get_move();
                send_move(my_move);
                game_state = GAME_STATE_MOVE;
                break;
            default:
                send_move(MOVE_ERROR);
                game_state = GAME_STATE_IDLE;
                break;
            }
            break;
        case GAME_STATE_MOVE:
            switch (move)
            {
            case MOVE_ROCK:
            case MOVE_PAPER:
            case MOVE_SCISSORS:
            {
                their_move = move;
                Move result = get_result(their_move, my_move);
                send_move(result);
                led_clear();
                led_set_my_move(my_move);
                led_set_their_move(their_move);
                led_set_result(result);
                game_state = GAME_STATE_RESULT;
            }
            break;
            default:
                send_move(MOVE_ERROR);
                game_state = GAME_STATE_IDLE;
                break;
            }
            break;
        case GAME_STATE_RESULT:
            switch (move)
            {
            case MOVE_YOU_WIN:
            case MOVE_I_WIN:
            case MOVE_TIE:
                game_state = GAME_STATE_IDLE;
                break;
            default:
                send_move(MOVE_ERROR);
                game_state = GAME_STATE_IDLE;
                break;
            }
            break;
        default:
            // We should never get here
            ret = ERROR_UNEXPECTED_GAME_ENGINE_STATE;
            game_state = GAME_STATE_IDLE;
            break;
        }
    }
    return ret;
}