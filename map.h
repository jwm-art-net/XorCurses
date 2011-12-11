/****************************************************************************
    This file is part of XorCurses a port/remake of the game Xor
    (originally by Astral Software) to the Linux console using
    ncurses.

    XorCurses written by James W. Morris - james@jwm-art.net
    http://www.jwm-art.net/

    All code licensed under GNU GPL v3.

    file:       map.h
    purpose:    defines map data structure, methods to create it, destroy it
                and load it, or load just the map names. etc.

****************************************************************************/
#ifndef _MAP_H
#define _MAP_H

#include "types.h"
#include <stdio.h>

#define MAP_H 32
#define MAP_W 32
#define HMAP_H 16
#define HMAP_W 16

#define MAPNAME_MAXCHARS 20

extern const char* XORCURSES_MAP_ID;

struct xor_map
{
    char*   filename;
    char*   name;

    struct xy   player[2];  /* initial player positions */
    struct xy   view[2];    /* initial player views */
    struct xy   mappc[4];   /* map positions */
    struct xy   teleport[2];/* teleport positions */
    struct xy   tpview[2];  /* teleport exit views */

    ctr_t   mask_count;
    su_t    level;

    map_t *buf[MAP_H + 1];
};

extern struct xor_map *map;

int     xor_map_create(void); /* return 0 on failure */
void    xor_map_destroy(void);

int     xor_map_load(su_t level); /* return 0 on failure */
int     xor_map_load_file(const char*); /* return 0 on failure */
char*   xor_map_read_name(FILE * fp, ctr_t* best_moves);

/* best_moves will have the default best moves value read from map */
char*   xor_map_load_read_name(su_t level, ctr_t* best_moves);

/*  xor_map_validate ensures that any objects that gravitate
    are suspended by *something*. fish and h-bombs should not
    hang in the air, and chickens and v-bombs should not be
    being held back by nothing, waiting to race left. returns
    0 for invalid maps, 1 for valid maps.
*/
su_t    xor_map_validate(void);

/*  map_get_teleport checks the coordinates passed against
    those in the map->teleports array and returns 0 if
    coordinate is teleport 0, and 1 if coordinate is teleport
    1. returns 99 if coord is not a teleport.
*/
su_t    map_get_teleport(xy_t x, xy_t y);

/* always returns 0  */
int     xor_map_load_error(FILE * fp, const char *filename, char *msg);

su_t    mapchar_to_icon(char c);
char    icon_to_mapchar(su_t icon);

#endif /* _MAP_H */
