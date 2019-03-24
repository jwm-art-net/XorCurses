#include "level_menu.h"

#include "options.h"
#include "map.h"
#include "game_display.h"
#include "info.h"
#include "scores.h"
#include "screen.h"
#include "splash.h"
#include "play_xor.h"
#include "replay.h"
#include "help.h"
#include "control_flow.h"

#include <curses.h>
#include <string.h>
#include <stdlib.h>

#define LVL_NAME_CHARS 24

enum {
#ifdef TESTMAP
    LVL_MENU_FIRST,
    LVL_MENU_LAST = MAX_LEVEL,
#else
    LVL_MENU_TITLE,
    LVL_MENU_FIRST,
    LVL_MENU_LAST = LVL_MENU_FIRST + MAX_LEVEL - 1,
#endif
    LVL_MENU_BLANK1,
    LVL_MENU_QUIT,
    LVL_MENU_LOAD_REPLAY,
    LVL_MENU_DIFFICULTY,
    LVL_MENU_HELP,
    LVL_MENU_COUNT
};

static char** lvlmenu = 0;
static int shortcuts[LVL_MENU_COUNT];

void level_menu_score_update(lvl_t i, ctr_t moves)
{
    if (i < MIN_LEVEL || i > MAX_LEVEL)
        return;

    if (lvlmenu[i])
        free(lvlmenu[i]);

    lvlmenu[i] = malloc((MAPNAME_MAXCHARS + 8) * sizeof(char));
    sprintf(lvlmenu[i], "%-*s %4d", MAPNAME_MAXCHARS - 1,
            map_names[i], moves);
}

void level_menu_create()
{
    char* d = "Difficulty: new school";
    lvlmenu = malloc(sizeof(char*) * LVL_MENU_COUNT);
    set_score_update_cb(level_menu_score_update);
    for (int i =0; i < LVL_MENU_COUNT; ++i){
        lvlmenu[i] = 0;
        if (i >= MIN_LEVEL && i <= MAX_LEVEL) {
            level_menu_score_update(i, scores[i]);
            shortcuts[i] = i | MENU_SHORTCUT_NUMERIC;
        }
        else {
            switch(i){
#ifndef TESTMAP
            case LVL_MENU_TITLE:
                lvlmenu[i] = "Select level:";
                shortcuts[i] = 0 | MENU_LABEL;
                break;
#endif
            case LVL_MENU_BLANK1:
                lvlmenu[i] = "";
                shortcuts[i] = 0 | MENU_BLANK;
                break;
            case LVL_MENU_QUIT:
                lvlmenu[i] = "Quit";
                shortcuts[i] = 'q' | MENU_SHORTCUT_ACTIVATES;
                break;
            case LVL_MENU_LOAD_REPLAY:
                lvlmenu[i] = "Load Replay";
                shortcuts[i] = 'l' | MENU_SHORTCUT_ACTIVATES;
                break;
            case LVL_MENU_DIFFICULTY:
                lvlmenu[i] = malloc((strlen(d) + 1) * sizeof(char));
                strcpy(lvlmenu[i], d);
                shortcuts[i] = 'd' | MENU_SHORTCUT_ACTIVATES;
                break;
            case LVL_MENU_HELP:
                lvlmenu[i] = "Help";
                shortcuts[i] = 'h' | MENU_SHORTCUT_ACTIVATES;
                break;
            }
        }
    }
}

void level_menu_destroy()
{
    if (lvlmenu) {
        free(lvlmenu[LVL_MENU_DIFFICULTY]);
        for (int i =0; i < LVL_MENU_COUNT; ++i){
            if (i >= MIN_LEVEL && i <= MAX_LEVEL)
                free(lvlmenu[i]);
        }
        free(lvlmenu);
    }
}

void level_menu_repaint()
{
    wattrset(info_win, COLOR_PAIR(0));
    box(info_win, 0 ,0);
}

void
level_menu()
{
    int select = LVL_MENU_FIRST;
    int restore = -1;
    char* dif_ptr = strstr(lvlmenu[LVL_MENU_DIFFICULTY], "new");

    while (1)
    {
        wclear(info_win);
        screen_data->game_win_repaint_cb = &splash;
        screen_data->info_win_repaint_cb = &level_menu_repaint;
        level_menu_repaint();
        splash();

        if (restore>=0)
            select = restore;

        select = scr_menu(info_win,
                          lvlmenu, LVL_MENU_COUNT, shortcuts,
                          select, &restore);

        su_t oldschool = options->oldschool_play;

        switch(select)
        {
        case LVL_MENU_QUIT:
            return;

        case LVL_MENU_LOAD_REPLAY:
            control_flow(0 | FLOW_LOAD_REPLAY);
            break;

        case LVL_MENU_DIFFICULTY:
            options->oldschool_play ^= 1;
            break;

        case LVL_MENU_HELP:
            help_menu();
            if (restore)
                select = restore;
            break;

        default:
            if (select >= MIN_LEVEL && select <= MAX_LEVEL)
                control_flow(select);
        }

        if (oldschool != options->oldschool_play)
        {   /* more than one route to changing oldschool state */
            if (options->oldschool_play)
            {
                /* remove truncated null warning
                 * due to strncpy(dif_ptr, "old", 3);*/
                *dif_ptr = 'o';
                *(dif_ptr + 1) = 'l';
                *(dif_ptr + 2) = 'd';
                options->scroll_thresh = 1;
            }
            else {
                *dif_ptr = 'n';
                *(dif_ptr + 1) = 'e';
                *(dif_ptr + 2) = 'w';
                options->scroll_thresh = 2;
            }

            if (restore)
                select = restore;

            screen_resize();
        }
    }
}

