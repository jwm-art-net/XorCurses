/***************************************************************************
    This file is part of XorCurses a port/remake of the game Xor
    (originally by Astral Software) to the Linux console using
    ncurses.

    XorCurses written by James W. Morris - james@jwm-art.net
    http://www.jwm-art.net/

    All code licensed under GNU GPL v3.

    file:       options.h
    purpose:    initializes options and reads map names from maps.

***************************************************************************/
#ifndef _OPTIONS_H
#define _OPTIONS_H

#include "types.h"

/*  dir in this context = directory
    elsewhere, dir = direction...
*/


enum DATA_LOC
{
    DATA_INST_LOC = 0,          /* installed location                   */
    DATA_CWD_LOC = 1,           /* current working directory location   */
    DATA_USER_LOC = 2           /* unused - user specified loc cmdline  */
};

extern char *map_name[MAX_LEVEL + 1];

struct xor_options
{
    unsigned oldschool_play:1;  /* scroll thresh of 1 and 8x8 game area.*/
    unsigned scroll_thresh:2;
    unsigned good_opt_dir:1;
    unsigned replay_speed:4;
    unsigned replay_hyper:1;
    unsigned replay_step:1;
    unsigned dir_opt:2;         /* 0=default, 1=cwd, 2=other */
    const char *data_dir;       /* ie install dir where maps & help are. */
    char *user_dir;             /* ie /home/username */
    char *map_dir;
};

extern struct xor_options *options;

su_t options_create();

void options_destroy();

su_t options_set_dir_opt(enum DATA_LOC loc);

su_t options_create_map_names();

void options_destroy_map_names();

/*  options_replay_speed accepts either 1-9 or '1'-'9'  */
long options_replay_speed(char n);

char *options_map_filename(su_t level);

char *options_file_path(const char *fname, const char *path);
#endif
