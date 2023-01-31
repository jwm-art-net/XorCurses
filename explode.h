/****************************************************************************
    This file is part of XorCurses a port/remake of the game Xor
    (originally by Astral Software) to the Linux console using
    ncurses.

    XorCurses written by James W. Morris - jwm.art.net@gmail.com

    All code licensed under GNU GPL v3.

    file:       explode.h
    purpose:    handles exploding objects, animates explosion, detects
                consequences upon gravitating objects.

****************************************************************************/
#ifndef _EXPLODE_H
#define _EXPLODE_H

#include "types.h"
#include "movelist.h"

/*
    explode_process_detonator explodes a bomb. it calculates the
    direction of the blast (pah!) and removes objects from the
    map (if appropriate - ie don't blast holes in map boundary).
*/

void explode_process_detonator(struct xmv_link *detlnk);

void explode_process_h_blast(xy_t bombx, xy_t bomby, su_t detdir);

void explode_process_v_blast(xy_t bombx, xy_t bomby, su_t detdir);

#endif
