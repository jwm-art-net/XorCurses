#include "play_xor.h"

#include "options.h"
#include "scores.h"
#include "screen.h"
#include "info.h"
#include "player.h"
#include "replay.h"
#include "game_display.h"
#include "help.h"
#include "exit.h"

#include <stdlib.h>
#include <time.h>
#include <curses.h>


void
info_win_repaint()
{
    wclear(info_win);
    wattrset(info_win, COLOR_PAIR(0));
    box(info_win, 0, 0);
    info_win_update_player_icon();
    info_win_update_map(player.have_map);
    info_win_display();
}

int
play_xor(lvl_t level)
{
    player.replay = 0;

    if (level & FLOW_START)
    {
        level &= 0x000f;
        replay.level = level;
        replay.canplay = 1;
        replay.hasexit = 0;

        for (ctr_t mv = MAX_MOVES; mv >= 0; mv--)
            replay.moves[mv] = 0;

        init_wall(level, TRUE);
        xor_map_create();
        xor_map_load(level);
        player_init();
        game_win_init_views();
        game_win_wipe_out();
    }
    else
        level = replay.level;

    game_win_display();
    info_win_repaint();
    screen_data->game_win_repaint_cb = &game_win_display;
    screen_data->info_win_repaint_cb = &info_win_repaint;
    info_win_display();
    int state = PLAY_CONTINUE;

    while (state == PLAY_CONTINUE || state == PLAY_PROCESS_MOVE)
    {
        int key = wgetch(game_win);
        su_t move = MV_NONE;

        switch (key)
        {
        case KEY_LEFT:  case 'z':   move = MV_LEFT;     break;
        case KEY_RIGHT: case 'x':   move = MV_RIGHT;    break;
        case KEY_UP:    case '\'':  move = MV_UP;       break;
        case KEY_DOWN:  case '/':   move = MV_DOWN;     break;

        case '\r':  case 'p':
            move = MV_PLAYER_SWAP;
            break;

        case 'b':
            if (!options->oldschool_play
             && player.moves_remaining < MAX_MOVES)
            {
                replay.moves[player.moves_remaining + 1]
                    ^= MV_REPLAY_BREAK;
                if (replay.moves[player.moves_remaining + 1]
                  & MV_REPLAY_BREAK)
                    scr_wmsg_pause(game_win, "Breakpoint set", 0, 0, TRUE);
                else
                    scr_wmsg_pause(game_win, "Breakpoint unset", 0, 0,TRUE);
                game_win_display();
            }
            break;

        case '1': case '2': case '3':
            if (!options->oldschool_play)
            {
                options->scroll_thresh = key - '0';
                char buf[40];
                snprintf(buf, 39, "Scroll threshold %d set",
                                    options->scroll_thresh);
                scr_wmsg_pause(game_win, buf, 0, 0, TRUE);
                game_win_display();
            }
            break;

        case 'm':   case 'M':   game_win_map_display(); break;
        case 'h':   case 'H':   help_menu(); game_win_display(); break;
        case 'q':   case 'Q':   move = MV_PLAYER_QUIT;  break;

        case KEY_RESIZE:
            screen_resize();
            break;
        }

        if (move)
        {
            state = player_move(move);

            if (state & PLAY_RECORD)
            {
                if (player.set_breakpoint)
                {
                    player.set_breakpoint = FALSE;
                    replay.moves[player.moves_remaining + 1]
                        |= MV_REPLAY_BREAK;
                }

                if (move == MV_PLAYER_QUIT)
                    replay.moves[player.moves_remaining] = move;
                else
                    replay.moves[player.moves_remaining--] = move;

                state ^= PLAY_RECORD;
            }

            info_win_display();

            if (!(player.p0_alive || player.p1_alive))
                return FLOW_DEATH;
        }
    }

    if (state == PLAY_COMPLETE)
    {
        player_exit_animate(&player.xmv[player.player]);
        replay.moves[player.moves_remaining] = MV_PLAYER_EXIT;
        replay.hasexit = 1;
        save_score(map->level, MAX_MOVES - player.moves_remaining);
        return FLOW_COMPLETE;
    }

    if (player.moves_remaining == MAX_MOVES)
        return FLOW_DO_QUIT;

    if (state == PLAY_QUIT)
        return FLOW_CAN_PLAY;

    return FLOW_CONTINUE;
}
