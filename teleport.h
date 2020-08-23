/****************************************************************************
    This file is part of XorCurses a port/remake of the game Xor
    (originally by Astral Software) to the Linux console using
    ncurses.

    XorCurses written by James W. Morris - jwm.art.net@gmail.com
    http://www.jwm-art.net/

    All code licensed under GNU GPL v3.

    file:       teleport.h
    purpose:    teleport player, animate this, etc.

****************************************************************************/
#ifndef _TELEPORT_H
#define _TELEPORT_H

#include "types.h"
#include "actions.h"

enum TP_ANIM
{
    TP_IN,
    TP_OUT
};

bool player_teleport(struct xor_move *pmv);

void player_teleport_animate(struct xor_move *pmv, enum TP_ANIM dir);

#endif
