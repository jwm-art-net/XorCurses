/****************************************************************************
    This file is part of XorCurses a port/remake of the game Xor
    (originally by Astral Software) to the Linux console using
    ncurses.

    XorCurses written by James W. Morris - james@jwm-art.net
    http://www.jwm-art.net/

    All code licensed under GNU GPL v3.

    file:       splash.h
    purpose:    displays splash screen.

****************************************************************************/
#ifndef _SPLASH_H
#define _SPLASH_H


#include "types.h"


void splash();

void splash_mask(xy_t offx, xy_t offy, int randcol);

void splatter_masks(void);


void splash_wipe_out(void);

void splash_wipe_anim(int dir); /* 1 fwd -1 back */


#endif
