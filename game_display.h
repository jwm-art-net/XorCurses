/****************************************************************************
    This file is part of XorCurses a port/remake of the game Xor
    (originally by Astral Software) to the Linux console using
    ncurses.

    XorCurses written by James W. Morris - james@jwm-art.net
    http://www.jwm-art.net/

    All code licensed under GNU GPL v3.

    file:       game_display.h
    purpose:    displays the game as it is played, basically.

****************************************************************************/
#ifndef _GAME_DISPLAY_H
#define _GAME_DISPLAY_H

#include "types.h"
#include "actions.h"

void game_win_init_views();

void game_win_display();

void game_win_show(xy_t tlx, xy_t tly);

void game_win_swap_update();

void game_win_move_player(struct xor_move *xmv);

void game_win_move_object(struct xor_move *xmv);

/*  game_win_map_coord returns pointer to a new xy struct
    of *screen* coordinates of map coordinate x,y. returns
    0 if map coord is off-screen.
*/
struct xy *game_win_map_coord(xy_t x, xy_t y);

/* -------------------------
    game_win_icon_dump x,y parameters are WINDOW coordinates.
*/
void game_win_icon_dump(xy_t x, xy_t y, su_t icon);

/* -------------------------
    game_win_icon_display x,y parameters are map coordinates.
    these are translated to display the icon only if it should
    be displayed in the window
*/
void game_win_icon_display(xy_t x, xy_t y, su_t icon);

void game_win_map_display();

#endif
