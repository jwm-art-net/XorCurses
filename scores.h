/****************************************************************************
    This file is part of XorCurses a port/remake of the game Xor
    (originally by Astral Software) to the Linux console using
    ncurses.

    XorCurses written by James W. Morris - jwm.art.net@gmail.com
    http://www.jwm-art.net/

    All code licensed under GNU GPL v3.

    file:       scores.h
    purpose:    loads scores - least number of moves taken to complete
                each level (and sets initial scores).

****************************************************************************/
#ifndef _SCORES_H
#define _SCORES_H

#include "types.h"

extern ctr_t*   scores;
extern char**   map_names;


int     create_scores();
void    destroy_scores();

int     load_scores();
void    set_score(lvl_t level, ctr_t moves);
int     save_scores();

/*  the score_update_cb param of set_score_update_cb is called by
    save_score. the level menu needs (sets) this to update the scores
    for a level in the menu when that level is completed.
*/
void    set_score_update_cb(void (*score_update_cb)(lvl_t, ctr_t));

#endif
