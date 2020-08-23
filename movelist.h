/****************************************************************************
    This file is part of XorCurses a port/remake of the game Xor
    (originally by Astral Software) to the Linux console using
    ncurses.

    XorCurses written by James W. Morris - jwm.art.net@gmail.com
    http://www.jwm-art.net/

    All code licensed under GNU GPL v3.

    file:       movelist.h
    purpose:    the consequences of an object moving might be that other
                gravitating objects are unblocked and must now gravitate.
                the move list contains a list of these objects. this is
                different to a chain of xor_moves as any gravitating object
                type can appear in the move list, but only objects of the
                same type can be chained together via xor_move (ie a row of
                chickens or a collumn of fish).

****************************************************************************/
#ifndef _MOVELIST_H
#define _MOVELIST_H

#include "actions.h"
#include "types.h"

struct xmv_link
{
    struct xor_move *xmv;
    struct xmv_link *prev;
    struct xmv_link *next;
};

struct xmv_list
{
    struct xmv_link *first;
    struct xmv_link *last;
    struct xmv_link *current;
};

extern struct xmv_list *xmvlist;

/* basic list functionality */
struct xmv_list *xmvlist_create();

void xmvlist_destroy();

struct xmv_link *xmvlist_first();

struct xmv_link *xmvlist_next();

struct xmv_link *xmvlist_cycle_next();

struct xmv_link *xmvlist_append_xor_move(struct xor_move *xmv);

struct xor_move *xmvlist_unlink_xor_move(struct xmv_link *lnk);

/* xor_move relevant functions */

/*
    xmvlist_contains_coord searches the xmvlist for xor_moves
    satisfying (from_x == x && from_y == y). the function restores
    xmv_list->current to the value it had before xmvlist_contains_coord
    was called.

    when the link stop_at is encountered the search is abandonded.

    the xor_move* res_prev pointer is set to point, when a result is
    found, to the xor_move preceding the result in the chain, or 0.
    (for purposes of splitting the chain)
*/
struct xmv_link *xmvlist_contains_coord(xy_t x,
                                        xy_t y,
                                        struct xmv_link *stop_at,
                                        struct xor_move **res_prev);

/*  not needed:
struct xmv_link *xmvlist_last();

struct xmv_link *xmvlist_prev();

struct xmv_link *xmvlist_cycle_prev();
*/

#endif
