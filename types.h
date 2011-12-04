/***************************************************************************
    This file is part of XorCurses a port/remake of the game Xor
    (originally by Astral Software) to the Linux console using
    ncurses.

    XorCurses written by James W. Morris - james@jwm-art.net
    http://www.jwm-art.net/

    All code licensed under GNU GPL v3.

    file:       types.h
    purpose:    define types and a couple of useful widely used enumerations.

***************************************************************************/
#ifndef _TYPES_H
#define _TYPES_H


#include <stdint.h>
#include <stdio.h>


/*  type to be used for storing map data within
    n*n array                                   */

typedef uint8_t map_t;

/*  type to be used for storing coordinates in map.
    min 0,max 32 (MAP_W,MAP_H defaults). note: no
    longer requires signed.                     */
typedef uint8_t xy_t;

/*  type to be used for counters, ie moves_remaining
    has max 2000                                */
typedef int16_t ctr_t;

/*  type to be used for storing the level number,
    requires signed.                            */
typedef int16_t lvl_t;

/*  type to be used for storing contact types,
    see actions.h CONTACT                       */
typedef int16_t ct_t;

/*  small unsigned type, miscellaneous          */
typedef uint8_t su_t;


struct xy
{
    xy_t x;
    xy_t y;
};


struct scrxy
{
    int x;
    int y;
};


/* levels constants placed here - why not? */
#ifdef TESTMAP
enum { MIN_LEVEL = 0, MAX_LEVEL = 15 };
#else
enum { MIN_LEVEL = 1, MAX_LEVEL = 15 };
#endif

enum { MAX_MOVES = 2000 };


int read_int(FILE* fp, char* fmt, int* result);



#endif
