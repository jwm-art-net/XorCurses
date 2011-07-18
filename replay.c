#include "replay.h"

#include "player.h"
#include "screen.h"
#include "game_display.h"
#include "info.h"
#include "play_xor.h"
#include "options.h"

#include <stdlib.h>
#include <time.h>

enum {
    REPLAY_MENU_TITLE,
    REPLAY_MENU_STATUS,     /*  when breakpoint hit     */
    REPLAY_MENU_CONTINUE,   /*  continue replay         */
    REPLAY_MENU_PLAY_GAME,  /*  play game               */
    REPLAY_MENU_YES,        /*  yes replay              */
    REPLAY_MENU_NO,         /*  no quit                 */
    REPLAY_MENU_QUIT,       /*  no quit                 */
    REPLAY_MENU_BREAK_DEL,  /*  delete breakpoint       */
    REPLAY_MENU_BREAK_SET,  /*  set breakpoint          */
    REPLAY_MENU_SAVE,       /*  save replay             */
    REPLAY_MENU_RESTART,    /*  restart game            */
    REPLAY_MENU_COUNT
};

static char** replaymenu = 0;
static int shortcuts[REPLAY_MENU_COUNT];

void replay_menu_create()
{
    replaymenu = malloc(sizeof(char*) * REPLAY_MENU_COUNT);

    replaymenu[REPLAY_MENU_STATUS] =    "*** REPLAY BREAKPOINT ***";
    shortcuts[REPLAY_MENU_STATUS] =     0 | MENU_STATUS;

    replaymenu[REPLAY_MENU_TITLE] =     "??? Replay ???";
    shortcuts[REPLAY_MENU_TITLE] =      0 | MENU_LABEL;

    replaymenu[REPLAY_MENU_YES] =       "Yes replay";
    shortcuts[REPLAY_MENU_YES] =        'y' | MENU_SHORTCUT_ACTIVATES;

    replaymenu[REPLAY_MENU_NO] =        "No quit game";
    shortcuts[REPLAY_MENU_NO] =         'n';

    replaymenu[REPLAY_MENU_QUIT] =      "";
    shortcuts[REPLAY_MENU_QUIT] =       'q' | MENU_HIDDEN;

    replaymenu[REPLAY_MENU_CONTINUE] =  "Continue replay";
    shortcuts[REPLAY_MENU_CONTINUE] =   'c' | MENU_SHORTCUT_ACTIVATES;

    replaymenu[REPLAY_MENU_PLAY_GAME] = "Play game";
    shortcuts[REPLAY_MENU_PLAY_GAME] =  'p' | MENU_SHORTCUT_ACTIVATES;

    /*  both shortcuts for the two breakpoints are set to same key.
        depending on context, atleast one of them is disabled at any
        given time.
    */
    replaymenu[REPLAY_MENU_BREAK_DEL] = "Unset breakpoint";
    shortcuts[REPLAY_MENU_BREAK_DEL] =  'b' | MENU_SHORTCUT_ACTIVATES;

    replaymenu[REPLAY_MENU_BREAK_SET] = "Set breakpoint";
    shortcuts[REPLAY_MENU_BREAK_SET] =  'b' | MENU_SHORTCUT_ACTIVATES;

    replaymenu[REPLAY_MENU_SAVE] =      "Save";
    shortcuts[REPLAY_MENU_SAVE] =       's';

    replaymenu[REPLAY_MENU_RESTART] =   "Restart game";
    shortcuts[REPLAY_MENU_RESTART] =    '*' | MENU_SHORTCUT_ACTIVATES;
}

void replay_menu_destroy()
{
}

void replay_menu_config_for(int flow)
{
    bool replay_continue = FALSE;
    int mv_i = player.moves_remaining + 1;
    if (player.replay)
        if (!(replay.moves[player.moves_remaining] & MV_PLAYER_QUIT))
            if ((flow & FLOW_INTERUPT_MENU)
              ||(flow & FLOW_INTERUPT_BREAK))
                replay_continue = TRUE;
    if (replay_continue)
        shortcuts[REPLAY_MENU_CONTINUE]
            &= ~ (MENU_HIDDEN | MENU_DISABLED);
    else
        shortcuts[REPLAY_MENU_CONTINUE]
            |= MENU_HIDDEN | MENU_DISABLED;
    if (!(flow & FLOW_DEATH) && player.moves_remaining >= 0)
        shortcuts[REPLAY_MENU_PLAY_GAME]
            &= ~ (MENU_HIDDEN | MENU_DISABLED);
    else
        shortcuts[REPLAY_MENU_PLAY_GAME]
            |= MENU_HIDDEN | MENU_DISABLED;
    if (replay.moves[mv_i] & MV_REPLAY_BREAK) {
        shortcuts[REPLAY_MENU_BREAK_DEL]
            &= ~ (MENU_HIDDEN | MENU_DISABLED);
        shortcuts[REPLAY_MENU_BREAK_SET]
            |= MENU_HIDDEN | MENU_DISABLED;
        shortcuts[REPLAY_MENU_STATUS]
            &= ~ (MENU_HIDDEN | MENU_DISABLED);
    }
    else {
        shortcuts[REPLAY_MENU_BREAK_DEL]
            |= MENU_HIDDEN | MENU_DISABLED;
        shortcuts[REPLAY_MENU_BREAK_SET]
            &= ~ (MENU_HIDDEN | MENU_DISABLED);
        shortcuts[REPLAY_MENU_STATUS]
            |= MENU_HIDDEN | MENU_DISABLED;
    }
}

int replay_menu(int flow)
{
    int select = 0;
    do {
        replay_menu_config_for(flow);
        #ifdef DEBUG
        mvwprintw(game_win,0,0,"FLOW:%4x",flow);
        #endif
        if (flow & FLOW_INTERUPT_BREAK)
            select = REPLAY_MENU_CONTINUE;
        select = scr_menu(game_win,
                          replaymenu, REPLAY_MENU_COUNT, shortcuts,
                          select, 0);
        switch(select) {
        case REPLAY_MENU_YES:
            flow = replay_xor(FLOW_START);
            break;
        case REPLAY_MENU_NO:
            return FLOW_DO_QUIT;
        case REPLAY_MENU_CONTINUE:
            if (player.set_breakpoint)
                player.set_breakpoint = FALSE;
            flow = replay_xor(FLOW_CONTINUE);
            break;
        case REPLAY_MENU_PLAY_GAME:
            return FLOW_CONTINUE;
        case REPLAY_MENU_BREAK_DEL:
            if (player.moves_remaining < MAX_MOVES) {
                replay.moves[player.moves_remaining + 1]
                    &= ~ MV_REPLAY_BREAK;
                player.set_breakpoint = FALSE;
            }
            if (!player.replay)
                select = REPLAY_MENU_PLAY_GAME;
            break;
        case REPLAY_MENU_BREAK_SET:
            if (player.moves_remaining < MAX_MOVES) {
                replay.moves[player.moves_remaining + 1]
                    |= MV_REPLAY_BREAK;
                player.set_breakpoint = TRUE;
            }
            select = REPLAY_MENU_PLAY_GAME;
            break;
        case REPLAY_MENU_SAVE:
            game_win_display();
            replay_save();
            select = REPLAY_MENU_PLAY_GAME;
            break;
        case REPLAY_MENU_RESTART:
            return FLOW_START;
        }
    } while (1);
}

int
replay_xor(int flow)
{
    int key;

    su_t state = PLAY_CONTINUE;

    struct timespec rpause;

    struct timespec repause;

    screen_data->game_win_repaint_cb = &game_win_display;
    screen_data->info_win_repaint_cb = &info_win_repaint;

    options->replay_hyper = 0;
    options->replay_step = 0; /* must do before setting rpause */
    rpause.tv_sec = 0;
    rpause.tv_nsec = options_replay_speed(options->replay_speed);

    if (flow & FLOW_START) {
        xor_map_create();
        xor_map_load(replay.level);
        player_init();
        player.replay = TRUE;
        init_wall(replay.level, TRUE);
        game_win_init_views();
    }
    game_win_display();
    info_win_update_map(player.have_map);
    info_win_repaint();
    info_win_display();
    info_win_update_player_icon();
    state = PLAY_CONTINUE;
    nodelay(game_win, TRUE);
    bool breakpoint = FALSE;
    bool interupt = FALSE;
    while ((state == PLAY_CONTINUE
          ||state == PLAY_PROCESS_MOVE)
         &&breakpoint == FALSE)
    {
        nanosleep(&rpause, &repause);
        key = wgetch(game_win);
        if (key == KEY_RESIZE)
            screen_resize();
        else if (key == 'q' || key == 'Q') {
            state = player_move(MV_PLAYER_QUIT);
            interupt = TRUE;
        }
        else if (key != ERR && options->replay_step == FALSE) {
            switch(key){
            case ')':
                options->replay_hyper = 1;
                rpause.tv_nsec = 0;
                break;
            case 's':
            case 'S':
                options->replay_step = 1;
                nodelay(game_win, FALSE);
                break;
            default:
                if (key >= '1' && key <= '9')
                    rpause.tv_nsec = options_replay_speed(key);
                break;
            }
        }
        if (key == ERR ||
            (options->replay_step && state != PLAY_QUIT))
        {
            su_t move = replay.moves[player.moves_remaining];
            if (move & MV_REPLAY_BREAK)
            {
                move &= ~ MV_REPLAY_BREAK;
                breakpoint = TRUE;
            }
            state = player_move(move) ^ PLAY_RECORD;
            if (state != PLAY_QUIT)
                player.moves_remaining--;
        }
        info_win_display();
            if ((!player.p0_alive) && (!player.p1_alive))
                state = PLAY_GOTCHA;
    }
    nodelay(game_win, FALSE);
    if (interupt)
        return FLOW_INTERUPT_MENU;
    if (breakpoint)
        return FLOW_INTERUPT_BREAK;
    return FLOW_START;
}

void replay_save()
{
    FILE *fp = 0;
    char *filename = scr_wmsg_read(game_win, "Filename:", 30);
    game_win_display();
    if ((fp = fopen(filename, "r"))) {
        scr_wmsg(game_win, "Exists! Overwrite (y/n)?", 0, 0);
        fclose(fp);
        int key = wgetch(game_win);
        game_win_display();
        if (key == 'n' || key == 'N') {
            free(filename);
            return;
        }
    }
    if (!(fp = fopen(filename, "w"))) {
        scr_wmsg(game_win, "Error!", 0, 0);
        goto bail;
    }
    if (!fwrite(&replay, sizeof(struct xor_replay), 1, fp)) {
        scr_wmsg(game_win, "Error!", 0, 0);
        goto bail;
    }
    scr_wmsg(game_win, "Saved!", 0, 0);

bail:
    wgetch(game_win);
    game_win_display();
    if (filename)
        free(filename);
    if (fp)
        fclose(fp);
}

lvl_t replay_load()
{
    FILE *fp = 0;
    char *filename = scr_wmsg_read(game_win, "Filename:", 30);
    screen_data->game_win_repaint_cb();
    if (!(fp = fopen(filename, "r"))) {
        scr_wmsg(game_win, "Open Error!", 0, 0);
        goto bail;
    }
    free(filename);
    if (!fread(&replay, sizeof(struct xor_replay), 1, fp)) {
        scr_wmsg(game_win, "Read Error!", 0, 0);
        goto bail;
    }
    fclose(fp);
    return replay.level;

bail:
    wgetch(game_win);
    screen_data->game_win_repaint_cb();
    if (filename)
        free(filename);
    if (fp)
        fclose(fp);
    return 999;
}


#ifdef DEBUG
void replay_dump_break_quit_moves()
{
    int mv_st = - 10;
    int mv_en = 10;
    if (player.moves_remaining + mv_st < 0)
        mv_st -= player.moves_remaining + mv_st;
    if (player.moves_remaining + mv_en > MAX_MOVES)
        mv_en += MAX_MOVES - (player.moves_remaining + mv_en);
    int n = 0;
    for (int i = -4; i < 6; ++i, ++n) {
        if (replay.moves[player.moves_remaining + i] & MV_PLAYER_QUIT)
            mvwprintw(game_win, n, 10, "MV_PLAYER_QUIT = moves_remain + %d ", i);
        if (replay.moves[player.moves_remaining + i] & MV_REPLAY_BREAK)
            mvwprintw(game_win, n, 10, "MV_REPLAY_BREAK = moves_remain + %d ", i);
    }
}
#endif

