/****************************************************************************
    This file is part of XorCurses a port/remake of the game Xor
    (originally by Astral Software) to the Linux console using
    ncurses.

    XorCurses written by James W. Morris - james@jwm-art.net
    http://www.jwm-art.net/

    All code licensed under GNU GPL v3.

    file:       player.h
    purpose:    defines data for player movement and state, and functions
                for moving the player and calculating the consequences.

****************************************************************************/
#ifndef _PLAYER_H
#define _PLAYER_H

#include "actions.h"
#include "map.h"

enum PLAY_STATE
{
    PLAY_CONTINUE,              /* no movement, as such                 */
    PLAY_PROCESS_MOVE,          /* process movement                     */
    PLAY_SWAP,                  /* player swap                          */
    PLAY_QUIT,                  /* user quit                            */
    PLAY_ZERO_MOVES,            /* all moves depleted                   */
    PLAY_GOTCHA,                /* player death                         */
    PLAY_COMPLETE,              /* masks collected, exit reached        */
    PLAY_RECORD = 0x1000        /* record move to replay - or'd with above values if appropriate */
};

struct xor_player
{
    ctr_t   masks_collected;
    ctr_t   moves_remaining;

    struct xor_move xmv[2];

    xy_t    map_view_y;

    unsigned player:1;          /* set = player 2, not set = player 1 */
    unsigned p0_alive:1;
    unsigned p1_alive:1;
    unsigned wall_vis:1;
    unsigned replay:1;
    unsigned set_breakpoint:1;  /* this set from replay, recorded by play */
    unsigned have_map:4;        /* 1 bit set for each piece collected */
} player;

void player_init();

/*
    see map.h map.c for xor_map definition. see actions.h for
    values for move variable passed to player_move.
*/

/* player move, initiates player movement, but does not move player */
enum PLAY_STATE player_move(su_t move);

/* player_process_move

    moves player - and thing that player pushes - if any.
    return values:
        a) 0    if object player can push is blocked
        b) a new xor_move pointer
                if object can be pushed - pointer is the xor_move
                for object to be pushed.
        c) the same xor_move pointer as player's xor_move
                if there is nothing to push

*/
struct xor_move *player_process_move(struct xor_move *pmv);

enum PLAY_STATE player_process_old_pos(struct xor_move *pmv);

enum PLAY_STATE pushed_process_new_pos(struct xor_move *omv);

/* player_process_collect
    processes collection of objects by player,
    ie masks (and sad masks/switch), map-pieces.
    refers to the xor_move (rather than map) for
    identification of object.
*/
void player_process_collect(struct xor_move *pmv);

void player_death(su_t icon);

void player_process_map_pc(struct xor_move *pmv);

#if DEBUG
void player_state_print(int);
#endif

#endif

