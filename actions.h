/****************************************************************************
    This file is part of XorCurses a port/remake of the game Xor
    (originally by Astral Software) to the Linux console using
    ncurses.

    XorCurses written by James W. Morris - james@jwm-art.net
    http://www.jwm-art.net/

    All code licensed under GNU GPL v3.

    file:       actions.h
    purpose:    enumerations for different movements/interactions between
                objects. defines allowable movements/interactions for each
                object. defines xor_move data structure containing data
                about movement of an object, gravitating objects of same
                type are chained together. processes consequences from
                movement of one object upon gravitating objects (if any)
                surrounding object.

****************************************************************************/

#ifndef _ACTIONS_H
#define _ACTIONS_H

#include "types.h"
#include "icons.h"

/*
    contact here means one object (in movement) attempting to
    move onto another object - ie two objects are not in
    contact when situated side by side.
*/

enum MOVES
{
    /* movement directions */
    MV_NONE =   0x00,
    MV_LEFT =   0x01,
    MV_RIGHT =  0x02,
    MV_UP =     0x04,
    MV_DOWN =   0x08,

    /* combos - happenings */
    MV_HORIZ =  0x03,
    MV_VERT =   0x0c,
    MV_ANY =    0x0f,       /* ALL/ANY (direction (context)) */

    /* special extra-ordinary cases (not used by routines here) */
    MV_PLAYER_SWAP = 0x10,
    MV_PLAYER_QUIT = 0x20,
    MV_REPLAY_BREAK= 0x40,  /* or'd with any of above to autostop       */
                            /* replay.                                  */
    MV_PLAYER_EXIT = 0x80   /* player finishes and exits                */
};

enum MVINIT                 /* movement without contact */
{
    MVI_NONE =      0,      /* does not move (but might be pushed)      */
    MVI_PLAYER =    1,      /* moved by player hitting cursor keys ;-)  */
    MVI_GRAVITY =   2       /* moved by gravity (when path unblocked)   */
};

enum CONTACT                /* happenings initiated by contact */
{
    CT_BLOCK =      0x000,  /* blocks movement in all directions        */
    CT_PASS =       0x001,  /* object does nothing (ie space)           */
    CT_FILTER =     0x002,  /* object filters movement                  */
    CT_PICKUP =     0x004,  /* object is collected (player)             */
    CT_PUSH =       0x008,  /* object is pushed (player)                */
    CT_HARDPUSH =   0x010,  /* object keeps moving (needs momentum)     */
    CT_EXPLODE =    0x020,  /* object explodes (non player contact)     */
    CT_TELEPORT =   0x040,  /* object teleports player                  */
    CT_PDEATH =     0x080,  /* death to player (object contact player)  */
    CT_EXIT =       0x100,  /* easy coded exit level if masks collected */
    CT_PAUSE =      0x200,  /* used during processing of explosions     */
    CT_ERROR =      0x400
};

struct xor_action
{
    unsigned mvini:     4;
    unsigned cont:      12;
    unsigned mvi_dir:   4;
    unsigned cont_dir:  4;
};

struct xor_move
{
    xy_t from_x;
    xy_t from_y;
    su_t from_obj;
    su_t dir;
    xy_t to_x;
    xy_t to_y;
    su_t to_obj;
    su_t moves_count;
    struct xor_move *chain;
};

extern struct xor_action actions[ICON_XXX];

/*
    create_xor_move:
        to move a known object in specified direction
*/
struct xor_move *create_xor_move(xy_t x, xy_t y, su_t move);

/*
    these two functions create a chain of xor_moves where
    the chain begins at map x,y and continues in the opposite
    direction to dir, where each following xor_move holds the
    same characteristics. the chain ends when an object in the
    map is encountered which does not have the relevant
    characteristics. returns the head of the chain or 0 if
    no object with the relevant characteristics is found.
    second form takes x,y,dir from the xor_move passed.
*/
struct xor_move *create_gravity_chain_xydir(xy_t x, xy_t y, su_t dir);

struct xor_move *create_gravity_chain(struct xor_move *xmv);

void destroy_gravity_chain(struct xor_move *xmv);
#endif
