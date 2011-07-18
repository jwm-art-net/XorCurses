/****************************************************************************
    This file is part of XorCurses a port/remake of the game Xor
    (originally by Astral Software) to the Linux console using
    ncurses.

    XorCurses written by James W. Morris - james@jwm-art.net
    http://www.jwm-art.net/

    All code licensed under GNU GPL v3.

    file:       moves.h
    purpose:    functions for initialising movement (detecting possibility
                of movement), and for gravitating gravitational objects.

****************************************************************************/
#ifndef _MOVES_H
#define _MOVES_H

#include "movelist.h"
#include "player.h"

/* determines if movement of object (not player) is possible */

ct_t move_object_init(struct xor_move *move);

/* move_gravity_process
    processes the movement of an object, how this object comes
    to be moving is decided by player_process_old_pos, and
    pushed_process_new_pos. the xor_move* xmv is the object
    (decided by the previous two functions) which is to move.
    this function checks if other objects need to move because
    this object is moving, and moves them also.
*/
void move_gravity_process(struct xor_move *xmv);

/* move_unchain_blocked_bomb
    when a chain of objects has gravitated and the path blocked,
    look to see if there is a bomb in the chain. if so, remove
    it and let the remaining chain continue to be processed -
    returns TRUE if this is to happen
*/
struct xor_move *move_unchain_blocked_bomb(struct xmv_link *lnk);

void move_hard_push_doll(struct xor_move *xmv);

#endif
