#include <stdlib.h>
#include <curses.h>

#include "level_menu.h"
#include "options.h"
#include "scores.h"
#include "screen.h"
#include "replay.h"
#include "help.h"
#include "version.h"
#include "debug.h"


int
main(void)
{
    int ret = 0;
    /* options need to be created first, as screen_create requires them. */
    if (!options_create()) {
        err_msg("Please either install XorCurses by running "
                "'sudo make install' within the\n"
                "XorCurses-" VERSION " source directory, or always "
                "start XorCurses from within that\ndirectory.\n");
        ret = 1;
        goto bail;
    }
    if (!screen_create()) {
        ret = 1;
        goto bail;
    }
    if (!screen_resize()) {
        err_msg("Terminal too small!\n"
                "Please resize and try again,\n"
                "if possible.\n");
        ret = 1;
        goto bail;
    }

    if (!options_create_map_names()) {
        err_msg("A problem occurred establishing the "
                "map names, sorry :-(\n");
        ret = 1;
        goto bail;
    }
    create_scores();
    load_scores();
    init_icons();
    level_menu_create();
    replay_menu_create();
    help_menu_create();

    /* main */
    level_menu();

  bail:
    help_menu_destroy();
    replay_menu_destroy();
    level_menu_destroy();
    screen_destroy();
    destroy_scores();
    options_destroy_map_names();
    options_destroy();
    return ret;
}

