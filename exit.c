#include "exit.h"

#include "splash.h"
#include "player.h"
#include "game_display.h"
#include "options.h"
#include "icons.h"
#include "screen.h"
#include "debug.h"

#include <time.h>


void player_exit_animate(struct xor_move* pmv)
{
    struct timespec rpause;
    struct timespec repause;
    struct xy* pxy = game_win_map_coord(pmv->from_x, pmv->from_y);
    struct xor_icon* icon = &icons[pmv->from_obj];
    int i;

    if (!pxy)
        return;

    rpause.tv_sec = 0;
    rpause.tv_nsec = 0;

    if (player.replay)
    {
        if (!options->replay_hyper)
            rpause.tv_nsec =
                options_replay_speed(options->replay_speed) / 5;
    }
    else
        rpause.tv_nsec = 50000000L;

    if (pmv->dir & MV_VERT)
        rpause.tv_nsec *= (double)ICON_W / (double)ICON_H;

    switch(pmv->dir)
    {
    case MV_LEFT:
        for (i = 0; i <= ICON_W; ++i)
        {
            for (xy_t y = 0; y < ICON_H; ++y)
            {
                xy_t ix = i;

                for (xy_t x = 0; x < ICON_W; ++x)
                {
                    if (ix < ICON_W)
                    {
                        wattrset(game_win, COLOR_PAIR(pmv->from_obj));
                        mvwaddch(game_win, pxy->y + y, pxy->x + x,
                                                icon->chrs[y][ix++]);
                    }
                    else
                    {
                        wattrset(game_win, COLOR_PAIR(ICON_SPACE));
                        mvwaddch(game_win, pxy->y + y, pxy->x + x, ' ');
                    }
                }
            }
            wrefresh(game_win);
            nanosleep(&rpause, &repause);
        }
        break;

    case MV_RIGHT:
        for (i = 0; i <= ICON_W; ++i)
        {
            for (xy_t y = 0; y < ICON_H; ++y)
            {
                xy_t ix = 0;

                for (xy_t x = 0; x < ICON_W; ++x)
                {
                    if (i <= x)
                    {
                        wattrset(game_win, COLOR_PAIR(pmv->from_obj));
                        mvwaddch(game_win, pxy->y + y, pxy->x + x,
                                                icon->chrs[y][ix++]);
                    }
                    else
                    {
                        wattrset(game_win, COLOR_PAIR(ICON_SPACE));
                        mvwaddch(game_win, pxy->y + y, pxy->x + x, ' ');
                    }
                }
            }
            wrefresh(game_win);
            nanosleep(&rpause, &repause);
        }
        break;

    case MV_UP:
        for (i = 0; i <= ICON_H; ++i)
        {
            for (xy_t y = 0; y < ICON_H; ++y)
            {
                xy_t iy = i + y;

                for (xy_t x = 0; x < ICON_W; ++x)
                {
                    if (iy < ICON_H)
                    {
                        wattrset(game_win, COLOR_PAIR(pmv->from_obj));
                        mvwaddch(game_win, pxy->y + y, pxy->x + x,
                                                icon->chrs[iy][x]);
                    }
                    else
                    {
                        wattrset(game_win, COLOR_PAIR(ICON_SPACE));
                        mvwaddch(game_win, pxy->y + y, pxy->x + x, ' ');
                    }
                }
            }
            wrefresh(game_win);
            nanosleep(&rpause, &repause);
        }
        break;
    case MV_DOWN:
        for (i = 0; i <= ICON_H; ++i)
        {
            xy_t iy = 0;

            for (xy_t y = 0; y < ICON_H; ++y)
            {
                for (xy_t x = 0; x < ICON_W; ++x)
                {
                    if (i < y)
                    {
                        wattrset(game_win, COLOR_PAIR(pmv->from_obj));
                        mvwaddch(game_win, pxy->y + y, pxy->x + x,
                                                icon->chrs[iy][x]);
                    }
                    else
                    {
                        wattrset(game_win, COLOR_PAIR(ICON_SPACE));
                        mvwaddch(game_win, pxy->y + y, pxy->x + x, ' ');
                    }
                }

                if (i < y)
                    ++iy;
            }
            wrefresh(game_win);
            nanosleep(&rpause, &repause);
        }
        break;
    }

    splatter_masks();
}



