/****************************************************************************
    This file is part of XorCurses a port/remake of the game Xor
    (originally by Astral Software) to the Linux console using
    ncurses.

    XorCurses written by James W. Morris - james@jwm-art.net
    http://www.jwm-art.net/

    All code licensed under GNU GPL v3.

    file:       play_xor.h
    purpose:    main loop for playing the game

****************************************************************************/
#ifndef _PLAY_XOR_H
#define _PLAY_XOR_H

#include "types.h"

#include "control_flow.h"

#include <ncurses.h> /* for bool! */

void info_win_repaint();

int play_xor(lvl_t level);

#endif
