/***************************************************************************
    This file is part of XorCurses a port/remake of the game Xor
    (originally by Astral Software) to the Linux console using
    ncurses.

    XorCurses written by James W. Morris - james@jwm-art.net
    http://www.jwm-art.net/

    All code licensed under GNU GPL v3.

    file:       icons.h
    purpose:    defines icon types, creates ASCII representations of each
                icon. initialises wall texture/colour dependant upon level
                to be played. misc conversion routines for map character to
                icon type enumeration, etc.

***************************************************************************/
#ifndef _ICONS_H
#define _ICONS_H

#define ICON_H 3
#define ICON_W 5

#include <curses.h>

#include "types.h"

struct xor_icon
{
    NCURSES_COLOR_T bg;
    NCURSES_COLOR_T fg;

    char chrs[ICON_H][ICON_W];
    char *name;
};

enum icon_no
{
    ICON_SPACE,
    ICON_WALL,
    ICON_H_FIELD,
    ICON_V_FIELD,
    ICON_MASK,
    ICON_FISH,
    ICON_CHICKEN,
    ICON_H_BOMB,
    ICON_V_BOMB,
    ICON_DOLL,
    ICON_SWITCH,
    ICON_MAP,
    ICON_EXIT,
    ICON_TELEPORT,
    ICON_PLAYER0,
    ICON_PLAYER1,
    ICON_EXPLOSION1,
    ICON_EXPLOSION2,
    ICON_EXPLOSION3,
    ICON_XXX
};

extern struct xor_icon icons[ICON_XXX];

extern struct xor_icon wall_icons[4];

void init_icons();

void init_wall(lvl_t level, bool show);

void win_icon_dump(WINDOW * win, xy_t x, xy_t y, su_t icon);


#endif /* _ICONS_H */
