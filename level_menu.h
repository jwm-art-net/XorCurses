/****************************************************************************
    This file is part of XorCurses a port/remake of the game Xor
    (originally by Astral Software) to the Linux console using
    ncurses.

    XorCurses written by James W. Morris - jwm.art.net@gmail.com
    http://www.jwm-art.net/

    All code licensed under GNU GPL v3.

    file:       level_menu.h
    purpose:    main menu containing choice of levels to play and other
                options.

****************************************************************************/
#ifndef _LEVEL_MENU_H
#define _LEVEL_MENU_H

#include "types.h"

void level_menu_create();
void level_menu_destroy();

void level_menu();

#endif
