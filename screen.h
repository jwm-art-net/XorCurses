/****************************************************************************
    This file is part of XorCurses a port/remake of the game Xor
    (originally by Astral Software) to the Linux console using
    ncurses.

    XorCurses written by James W. Morris
    http://www.jwm-art.net/

    All code licensed under GNU GPL.

    file:       screen.h
    purpose:    handles screen (terminal) sizing and resizing, allocates
                windows for the game area and info area. provides general
                purpose functions used by various routines for user I/O.

****************************************************************************/
#ifndef SCREEN_H
#define SCREEN_H

#include <curses.h>

#include "types.h"
#include "icons.h"

#define MIN_W       80      /*  minimum width of terminal.          */

#define DEF_GAREA_W 8       /*  for oldschool purists.              */
#define DEF_GAREA_H 8       

#define MAX_GAREA_W 12      /*  maximum width of game area.         */
#define MAX_GAREA_H 12      /*  maximum width of game area.         */

#define MIN_INFO_W  32      /*  width of the info window.           */

/*  info window height control settings for oldschool and newschool
    difficulty levels:                                              */
#define MAX_INFO_H MAX_GAREA_H * ICON_H
#define MAX_INFO_H_OLDSCHOOL 40

extern WINDOW *info_win;

extern WINDOW *game_win;

struct xor_screen
{
    int ga_tlx;     /*  game area top left x and y.                 */
    int ga_tly;
    int garea_w;    /*  count of icons width-wise in game area.     */
    int garea_h;    /*  count of icons height-wise in game area.    */

    int i_tlx;      /*  info window top left x and y                */
    int i_tly;
    int info_w;     /*  width of info win (chars).                  */
    int info_h;     /*  height of info win (chars).                 */

    int map_tlx;    /*  top left x and y of map display within the  */
    int map_tly;    /*  info window.                                */

    bool scale_map; /*  TRUE when screen is too small to display    */
                    /*  both game area and full-size map.           */

    /*  callbacks for repainting called by screen_resize            */
    void (*game_win_repaint_cb)();
    void (*info_win_repaint_cb)();
};

extern struct xor_screen *screen_data;

su_t screen_create();   /* init curses and create the screen_data   */

void screen_destroy();

/*  screen_resize() should be called after screen_create, and whenever
    the terminal is resized - either when KEY_RESIZE is returned by
    getch, or something changes which effects the geometries of the
    game area and info window (changing the difficulty level is one
    example).
*/
su_t screen_resize();

int scr_wmsg_pause(WINDOW * win, char *msg, int len, struct scrxy *msgxy, bool pause);
/*
int scr_wmsg(WINDOW *win, char *msg, int len, struct scrxy *msgxy);
*/
#define scr_wmsg(win, msg, len, msgxy)\
    scr_wmsg_pause(win, msg, len, msgxy, FALSE)

char* scr_wmsg_read(WINDOW *win, char *msg, int readlen);


enum XC_COLOR_PAIRS{
    COL_G_TXT = ICON_XXX + 1,
    COL_G_TXT_HI,
    COL_G_TXT_DISABLED,
    COL_G_TXT_STATUS,
    COL_I_TXT,
    COL_I_TXT_HI,
    COL_I_TXT_DISABLED,
    COL_I_TXT_STATUS,
    COL_I_MAP_WALL,
    COL_TOO_SMALL,
    COL_SPLASH_TITLE,
    COL_SPLASH_MASK,
    COL_SPLASH_MASK_SOLID,
};

enum {
    /* or'd with each menu shortcut */
    MENU_LABEL =                0x0100, /* label automatically adds...  */
    MENU_BLANK =                0x0200, /* a blank line                 */
    MENU_DISABLED =             0x0400,
    MENU_STATUS =               0x0800,

    MENU_HIDDEN =               0x1000, /* allows extra shortcuts       */

    MENU_SHORTCUT_ACTIVATES =   0x2000,
    MENU_SHORTCUT_NUMERIC =     0x4000,
    MENU_NONSHORTCUT_MASK =     0x0f00,
    MENU_NONSELECT_MASK =       0x1f00,
};

/*  MENU_HIDDEN allows hidden menu entries to provide alternate
    keyboard shortcuts. If the hidden item does is not a
    MENU_SHORTCUT_ACTIVATES item, then the previous menu item is
    selected, but if it is, then it is the returned item.
*/

char
scr_menu(WINDOW *win,
    char** menu, int count, int* shortcuts,
    int select, int *restore);


#endif

