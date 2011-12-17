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


static char** replaymenu = 0;
static int shortcuts[REPLAY_MENU_COUNT];
static const char* REPLAY_ID = "XorCurses__Replay";

static char mv_to_char(int mv);
static int  char_to_mv(char c);
static int write_replay_file(FILE* fp);
static int read_replay_file(FILE* fp);


void replay_menu_create()
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


void replay_menu_destroy()
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
                if (!options->oldschool_play)
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


void replay_save()
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

    if (write_replay_file(fp) == 0)
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


lvl_t replay_load()
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
    int i, ln;
    char chkdata[256];
    char* cdp = chkdata;

    fprintf(fp, "%s\n",             REPLAY_ID);
    fprintf(fp, "level %d %s\n",    replay.level,
                                    map_names[replay.level]);
    fprintf(fp, "oldschool %d\n",   options->oldschool_play);
    fprintf(fp, "scrollthresh %d\n",options->scroll_thresh);

    for (ln = i = 0; ln < 63; ++ln)
    {
        uint8_t chka = 0;
        uint8_t chkb = 0;
        char data[33];
        char* dp = data;
        size_t j;
        size_t len = (ln < 62) ? 32 : 17;

        for (j = 0; j < len; ++j)
            *dp++ = mv_to_char(replay.moves[i++]);

        if (ln == 62) /* checksum of checksums */
        {
            char* play = (replay.hasexit) ? "notplay" : "canplay";
            int c;
            fletcher16(&chka, &chkb, (uint8_t*)chkdata, 248);
            c = snprintf(dp, 5, "%02x%02x", chka, chkb);
            dp += c;
            len += c;
            c = snprintf(dp, strlen(play) + 1, "%s", play);
            dp += c;
            len += c;
            /* pad */
            for (; len < 32;  ++len)
                *dp++ = '0';
        }

        *dp = '\0';
        fletcher16(&chka, &chkb, (uint8_t*)data, len);
        fprintf(fp, "%02x%02x%s\n", chka, chkb, data);
        cdp += snprintf(cdp, 5, "%02x%02x", chka, chkb);
    }

    return 0;
}


int read_replay_file(FILE* fp)
{
    int level = -1, oldschool = -1, scrollthresh = -1;
    uint8_t calc_chka = 0;
    uint8_t calc_chkb = 0;
    unsigned int read_chka = 0;
    unsigned int read_chkb = 0;
    char chkdata[256];
    char* cdp = chkdata;

    char buf[80];
    char str[80];

    if (!fgets(buf, 80, fp))
        return -1;

    sscanf(buf, "%s", str);

    if (strcmp(str, REPLAY_ID) != 0)
        return -1;

    if (read_int(fp, "level %d", &level) < 0)
        return -1;

    if (read_int(fp, "oldschool %d", &oldschool) < 0)
        return -1;

    if (read_int(fp, "scrollthresh %d", &scrollthresh) < 0)
        return -1;

    debug("level:%d\n",level);
    debug("oldschool:%d\n",oldschool);
    debug("scrollthresh:%d\n",scrollthresh);

    int i, ln;
    char* bp;

    for (i = ln = 0; ln < 63; ++ln)
    {
        size_t len;
        size_t j;

        if (!fgets(buf, 80, fp))
        {
            debug("failed to get replay line:%d\n", ln);
            return -1;
        }

        buf[79] = '\0';
        len = strlen(buf);

        while(len && buf[len - 1] <= ' ')
            buf[--len] = '\0';

        if (len != 36)
        {
            debug("length failure (%ld) in replay line:%d\n", len, ln);
            return -1;
        }

        if (sscanf(buf, "%02x%02x", &read_chka, &read_chkb) != 2)
        {
            debug("failed to read line:%d checksum\n", ln);
            return -1;
        }

        bp = buf + 4;
        len -= 4;
        fletcher16(&calc_chka, &calc_chkb, (uint8_t*)bp, len);

        if (read_chka != calc_chka || read_chkb != calc_chkb)
        {
            debug("checksum mismatch line:%d\n", ln);
            debug("read:%02x%02x calc:%02x%02x\n",  read_chka, read_chkb,
                                                    calc_chka, calc_chkb);
            return -1;
        }

        cdp += snprintf(cdp, 5, "%02x%02x", read_chka, read_chkb);

        len = (ln < 62) ? len : 17;

        for (j = 0; j < len; ++j, ++i)
            replay.moves[i] = char_to_mv(*bp++);
    }

    sscanf(bp, "%02x%02x", &read_chka, &read_chkb);
    fletcher16(&calc_chka, &calc_chkb, (uint8_t*)chkdata, 248);

    if (read_chka != calc_chka || read_chkb != calc_chkb)
    {
        debug("vertical checksum mismatch\n");
        debug("read:%02x%02x calc:%02x%02x\n",  read_chka, read_chkb,
                                                calc_chka, calc_chkb);
        return -1;
    }

    bp += 4;
    sscanf(bp, "%s", str);

    replay.canplay = (strncmp(str, "canplay", 7) == 0) ? 1 : 0;
    replay.level = level;
    oldschool = oldschool > 0 ? 1 : 0;

    if (oldschool != options->oldschool_play)
    {
        options->oldschool_play = oldschool;
        screen_resize();
    }

    if (oldschool && scrollthresh > 0 && scrollthresh < 4)
        options->scroll_thresh = scrollthresh;

    return 0;
}


#ifdef DEBUG /* replay_dump_break_quit_moves() */
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
            mvwprintw(game_win, n, 10,
                                "MV_PLAYER_QUIT = moves_remain + %d ", i);
        if (replay.moves[player.moves_remaining + i] & MV_REPLAY_BREAK)
            mvwprintw(game_win, n, 10,
                                "MV_REPLAY_BREAK = moves_remain + %d ", i);
    }
}
#endif

