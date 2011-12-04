/****************************************************************************
    This file is part of XorCurses a port/remake of the game Xor
    (originally by Astral Software) to the Linux console using
    ncurses.

    XorCurses written by James W. Morris - james@jwm-art.net
    http://www.jwm-art.net/

    All code licensed under GNU GPL v3.

    file:       replay.h
    purpose:    data structure for storing each move player makes and
                functions for playing these moves back as a 'replay'.
                also creates the menu for the replay options, and load/save
                functions.

****************************************************************************/
#ifndef _REPLAY_H
#define _REPLAY_H

#include "types.h"
#include "play_xor.h" /* for enum param in replay() */

#include <ncurses.h> /* for bool! */

struct xor_replay
{
    su_t level;
    su_t moves[MAX_MOVES + 1];
    su_t canplay;
    su_t hasexit;
} replay;

void replay_menu_create();
void replay_menu_destroy();

/*  replay_menu now called before play_xor actually quits out of
    its main loop - this should simplify the code for continuing
    game play somewhat. RM_CANCEL_QUIT and RM_DO_QUIT returned
    by replay_menu and checked by the quit case statement within
    the play_xor main loop - rather than immediately quitting out
    of the main loop only to call it again because quit was cancelled.
*/
int replay_menu(int flow);

int replay_xor(int flow);

void replay_save();

lvl_t replay_load();

#ifdef DEBUG
/*  replay_dump_break_quit_moves steps through player.moves_remaining,
    -10 to +10 and any replay.moves with the MV_PLAYER_QUIT or
    MV_REPLAY_BREAK bits set - the relative move index to moves_remaining
    is displayed.
*/
void replay_dump_break_quit_moves();
#endif

#endif
