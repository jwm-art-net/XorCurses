/****************************************************************************
    This file is part of XorCurses a port/remake of the game Xor
    (originally by Astral Software) to the Linux console using
    ncurses.

    XorCurses written by James W. Morris - james@jwm-art.net
    http://www.jwm-art.net/

    All code licensed under GNU GPL v3.

    file:       info.h
    purpose:    displays necessary in-game information, during game play.
                displays map, scales map if terminal not large enough to
                display unscaled map.

****************************************************************************/
#ifndef _INFO_H
#define _INFO_H

#include "types.h"

void info_win_update_player_icon();

void info_win_update_map(su_t have_map);

/* here, map pieces are numberd 0 - 3 clockwise from top-right */
void info_win_dump_map(su_t mappc);

void info_win_map_erase_mask(xy_t x, xy_t y);

void info_win_display();

#endif
