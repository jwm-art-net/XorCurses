#include "replay.h"

#include "player.h"
#include "screen.h"
#include "game_display.h"
#include "info.h"
#include "play_xor.h"
#include "options.h"
#include "debug.h"
#include "fletcher.h"
#include "exit.h"
#include "splash.h"
#include "scores.h"

#include <string.h>
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
    REPLAY_MENU_RESTART,    /*  restart level           */
    REPLAY_MENU_COUNT
};

enum { REPLAY_FILE_LINE_WIDTH = 48 };

struct xor_replay replay;

static char** replaymenu = 0;
static int shortcuts[REPLAY_MENU_COUNT];
static const char* REPLAY_ID = "XorCurses__Replay";

static char mv_to_char(int mv);
static int  char_to_mv(char c);
static int write_replay_file(FILE* fp);
static int read_replay_file(FILE* fp);


void replay_menu_create(void)
{
    replaymenu = malloc(sizeof(char*) * REPLAY_MENU_COUNT);

    replaymenu[REPLAY_MENU_STATUS] =    "*** REPLAY BREAKPOINT ***";
    shortcuts[REPLAY_MENU_STATUS] =     0 | MENU_STATUS;

    replaymenu[REPLAY_MENU_TITLE] =     "??? Replay ???";
    shortcuts[REPLAY_MENU_TITLE] =      0 | MENU_LABEL;

    replaymenu[REPLAY_MENU_YES] =       "Yes replay";
    shortcuts[REPLAY_MENU_YES] =        'y' | MENU_SHORTCUT_ACTIVATES;

    replaymenu[REPLAY_MENU_NO] =        "No quit level";
    shortcuts[REPLAY_MENU_NO] =         'n';

    replaymenu[REPLAY_MENU_QUIT] =      "";
    shortcuts[REPLAY_MENU_QUIT] =       'q' | MENU_HIDDEN;

    replaymenu[REPLAY_MENU_CONTINUE] =  "Continue replay";
    shortcuts[REPLAY_MENU_CONTINUE] =   'c' | MENU_SHORTCUT_ACTIVATES;

    replaymenu[REPLAY_MENU_PLAY_GAME] = "Play level";
    shortcuts[REPLAY_MENU_PLAY_GAME] =  'p' | MENU_SHORTCUT_ACTIVATES;

    /*  both shortcuts for the two breakpoints are set to same key.
        depending on context, atleast one of them is disabled at any
        given time.
    */
    replaymenu[REPLAY_MENU_BREAK_DEL] = "Unset breakpoint";
    shortcuts[REPLAY_MENU_BREAK_DEL] =  'b' | MENU_SHORTCUT_ACTIVATES;

    replaymenu[REPLAY_MENU_BREAK_SET] = "Set breakpoint";
    shortcuts[REPLAY_MENU_BREAK_SET] =  'b' | MENU_SHORTCUT_ACTIVATES;

    replaymenu[REPLAY_MENU_SAVE] =      "Save replay";
    shortcuts[REPLAY_MENU_SAVE] =       's';

    replaymenu[REPLAY_MENU_RESTART] =   "Restart level";
    shortcuts[REPLAY_MENU_RESTART] =    '*' | MENU_SHORTCUT_ACTIVATES;
}


void replay_menu_destroy(void)
{
}


void replay_menu_config_for(int flow)
{
    bool replay_continue = FALSE;
    bool play_continue = TRUE;
    int mv_i = player.moves_remaining + 1;

    if (player.replay)
    {
        if (!(replay.moves[player.moves_remaining] & MV_PLAYER_QUIT))
        {
            if ((flow & FLOW_INTERUPT_MENU)
              ||(flow & FLOW_INTERUPT_BREAK))
            {
                replay_continue = TRUE;
            }
        }
    }

    if (replay_continue)
        shortcuts[REPLAY_MENU_CONTINUE]
            &= ~ (MENU_HIDDEN | MENU_DISABLED);
    else
        shortcuts[REPLAY_MENU_CONTINUE]
            |= MENU_HIDDEN | MENU_DISABLED;

    if (options->oldschool_play && !(flow & FLOW_CAN_PLAY))
	play_continue = FALSE;

    if (play_continue
     && !(flow & FLOW_DEATH)
     && !(flow & FLOW_COMPLETE)
     && player.moves_remaining >= 0
     && replay.canplay)
    {
        shortcuts[REPLAY_MENU_PLAY_GAME]
            &= ~ (MENU_HIDDEN | MENU_DISABLED);
    }
    else
        shortcuts[REPLAY_MENU_PLAY_GAME]
            |= MENU_HIDDEN | MENU_DISABLED;

    if (options->oldschool_play || !replay.canplay)
    {
        shortcuts[REPLAY_MENU_BREAK_DEL]
            |= (MENU_HIDDEN | MENU_DISABLED);
        shortcuts[REPLAY_MENU_BREAK_SET]
            |= MENU_HIDDEN | MENU_DISABLED;
        shortcuts[REPLAY_MENU_STATUS]
            |= (MENU_HIDDEN | MENU_DISABLED);
    }
    else if (replay.moves[mv_i] & MV_REPLAY_BREAK)
    {
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

        debug("FLOW:%4x\n", flow);

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


int replay_xor(int flow)
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

    if (flow & FLOW_START)
    {
        char* fn = options_map_filename(replay.level);

        if (!xor_map_create()
         || !xor_map_load_by_filename(fn))
        {
            scr_wmsg(game_win, "Failed to load map!", 0, 0);
            wgetch(game_win);
            free(fn);
            return FLOW_DO_QUIT;
        }

        free(fn);
        player_init();
        player.replay = TRUE;
        init_wall(replay.level, TRUE);
        game_win_init_views();
        map->level = replay.level;
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
         || state == PLAY_PROCESS_MOVE)
         && breakpoint == FALSE)
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
                if (!options->oldschool_play && replay.canplay)
                    breakpoint = TRUE;
            }
            state = player_move(move) ^ PLAY_RECORD;

            if (state != PLAY_QUIT)
                player.moves_remaining--;

            if (move & MV_PLAYER_EXIT)
                state = PLAY_GOTCHA;
        }
        info_win_display();
        if ((!player.p0_alive) && (!player.p1_alive))
            state = PLAY_GOTCHA;
    }
    nodelay(game_win, FALSE);

    fprintf(stderr, "\n");

#if DEBUG
player_state_print(state);
#endif

    if (state == PLAY_ZERO_MOVES || state == PLAY_GOTCHA)
        return FLOW_DEATH;

    if (state == PLAY_COMPLETE)
    {
        player_exit_animate(&player.xmv[player.player]);
        return FLOW_COMPLETE;
    }

    if (interupt)
        return FLOW_INTERUPT_MENU;

    if (breakpoint)
        return FLOW_INTERUPT_BREAK;

    if (state == PLAY_QUIT)
        return FLOW_CAN_PLAY;

    return FLOW_START;
}


void replay_save(void)
{
    FILE *fp = 0;
    char *filename = scr_wmsg_read(game_win, "Filename:", 30);
    game_win_display();

    if ((fp = fopen(filename, "r")))
    {
        int key;

        scr_wmsg(game_win, "Exists! Overwrite (y/n)?", 0, 0);
        fclose(fp);
        key = wgetch(game_win);
        game_win_display();

        if (key != 'y' && key != 'Y')
        {
            free(filename);
            scr_wmsg(game_win, "Aborted", 0, 0);
            return;
        }
    }

    if (!(fp = fopen(filename, "w")))
    {
        scr_wmsg(game_win, "Error!", 0, 0);
        goto bail;
    }

    if (write_replay_file(fp))
        scr_wmsg(game_win, "Saved!", 0, 0);
    else
        scr_wmsg(game_win, "Error saving replay!", 0, 0);

bail:
    wgetch(game_win);
    game_win_display();
    if (filename)
        free(filename);
    if (fp)
        fclose(fp);
}


lvl_t replay_load(void)
{
    FILE *fp = 0;
    char *filename = scr_wmsg_read(game_win, "Filename:", 30);
    lvl_t ret = 999;

    screen_data->game_win_repaint_cb();

    if (!(fp = fopen(filename, "r")) || read_replay_file(fp) < 0)
    {
        scr_wmsg(game_win, "Open Error!", 0, 0);
        wgetch(game_win);
        screen_data->game_win_repaint_cb();
    }
    else
        ret = replay.level;


    if (filename)
        free(filename);
    if (fp)
        fclose(fp);
    return ret;
}


inline char mv_to_char(int mv)
{
    if (mv & MV_REPLAY_BREAK)
    {
        mv &= ~MV_REPLAY_BREAK;

        switch(mv)
        {
        case MV_LEFT:       return 'L';
        case MV_RIGHT:      return 'R';
        case MV_UP:         return 'U';
        case MV_DOWN:       return 'D';
        case MV_PLAYER_SWAP:return 'S';
        case MV_PLAYER_QUIT:return 'Q';
        case MV_PLAYER_EXIT:return 'E';
        default:            return '0';
        }
    }

    switch(mv)
    {
    case MV_LEFT:       return 'l';
    case MV_RIGHT:      return 'r';
    case MV_UP:         return 'u';
    case MV_DOWN:       return 'd';
    case MV_PLAYER_SWAP:return 's';
    case MV_PLAYER_QUIT:return 'q';
    case MV_PLAYER_EXIT:return 'e';
    default: return '0';
    }
}


inline int char_to_mv(char c)
{
    switch(c)
    {
    case 'l':   return MV_LEFT;
    case 'r':   return MV_RIGHT;
    case 'u':   return MV_UP;
    case 'd':   return MV_DOWN;
    case 's':   return MV_PLAYER_SWAP;
    case 'q':   return MV_PLAYER_QUIT;
    case 'e':   return MV_PLAYER_EXIT;
    case 'L':   return MV_REPLAY_BREAK | MV_LEFT;
    case 'R':   return MV_REPLAY_BREAK | MV_RIGHT;
    case 'U':   return MV_REPLAY_BREAK | MV_UP;
    case 'D':   return MV_REPLAY_BREAK | MV_DOWN;
    case 'S':   return MV_REPLAY_BREAK | MV_PLAYER_SWAP;
    case 'Q':   return MV_REPLAY_BREAK | MV_PLAYER_QUIT;
    case 'E':   return MV_REPLAY_BREAK | MV_PLAYER_EXIT;
    default:    return MV_NONE;
    }
}


int write_replay_file(FILE* fp)
{
    int i;
    int ret = 0;
    struct df* df = df_open(fp, DF_WRITE, REPLAY_ID,
                                          REPLAY_FILE_LINE_WIDTH);

    if (!df)
    {
        debug("failed to write replay file\n");
        goto fail;
    }

    if (!df_write_hex_byte(df, replay.level))
    {
        debug("failed to write level number\n");
        goto fail;
    }

    if (!df_write_hex_byte(df, options->oldschool_play))
    {
        debug("failed to write oldschool\n");
        goto fail;
    }

    if (!df_write_hex_byte(df, options->scroll_thresh))
    {
        debug("failed to write scroll thresh\n");
        goto fail;
    }

    char tmp[MAX_MOVES + 1];

    for (i = 0; i <= MAX_MOVES; ++i)
        tmp[i] = mv_to_char(replay.moves[i]);

    if (!df_write_string(df, tmp, MAX_MOVES + 1))
    {
        debug("failed to write replay moves\n");
        goto fail;
    }

    if (!df_write_string(df, (replay.hasexit ? "notplay" : "canplay"), 7))
    {
        debug("failed to write play permission\n");
        goto fail;
    }

    ret = 1;

fail:
    df_close(df);
    return ret;
}


int read_replay_file(FILE* fp)
{
    int     i;
    char*   tmp;
    uint8_t level;
    uint8_t oldschool;
    uint8_t scroll;
    int     ret = 0;
    struct  df* df = df_open(fp, DF_READ, REPLAY_ID,
                                         REPLAY_FILE_LINE_WIDTH);
    if (!df)
    {
        debug("failed to read replay file\n");
        goto fail;
    }

    if (!df_read_hex_byte(df, &level))
    {
        debug("failed to read level number\n");
        goto fail;
    }

    if (level < 1 || level > 15)
    {
        debug("invalid level number\n");
        goto fail;
    }

    if (!df_read_hex_byte(df, &oldschool))
    {
        debug("failed to read oldschool\n");
        goto fail;
    }

    if (!df_read_hex_byte(df, &scroll))
    {
        debug("failed to read scroll thresh\n");
        goto fail;
    }

    tmp = df_read_string(df, MAX_MOVES + 1);

    if (!tmp)
    {
        debug("failed to read replay moves\n");
        goto fail;
    }

    for (i = 0; i < MAX_MOVES + 1; ++i)
        replay.moves[i] = char_to_mv(tmp[i]);

    free(tmp);

    tmp = df_read_string(df, 7);

    if (!tmp)
    {
        debug("failed to write play permission\n");
        goto fail;
    }

    replay.canplay = strcmp(tmp, "canplay") == 0;

    free(tmp);

    replay.level = level;
    options->scroll_thresh = (scroll < 1 || scroll > 3) ? 2 : scroll;
    options->oldschool_play = (oldschool != 0);

    ret = 1;

fail:
    df_close(df);
    return ret;
}


#ifdef DEBUG /* replay_dump_break_quit_moves() */
void replay_dump_break_quit_moves(void)
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
            mvwprintw(game_win, n, 10,
                                "MV_PLAYER_QUIT = moves_remain + %d ", i);
        if (replay.moves[player.moves_remaining + i] & MV_REPLAY_BREAK)
            mvwprintw(game_win, n, 10,
                                "MV_REPLAY_BREAK = moves_remain + %d ", i);
    }
}
#endif

