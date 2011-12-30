#include "teleport.h"
#include "game_display.h"
#include "player.h"
#include "options.h"
#include "screen.h"

#include <time.h>
#include <stdlib.h>


bool player_teleport(struct xor_move * pmv)
{
    su_t t = map_get_teleport(pmv->to_x, pmv->to_y) ^ 1;

    if (t > 1)
        return FALSE;

    player_teleport_animate(pmv, TP_IN);
    map->buf[pmv->from_y][pmv->from_x] = ICON_SPACE;
    game_win_show(map->tpview[t].x, map->tpview[t].y);

    /* check teleport destination has not been blown up */
    if (map->buf[map->teleport[t].y][map->teleport[t].x] == ICON_TELEPORT)
    {
        /* check teleport exits are not blocked */
        xy_t chkx[4] = { 1, 0, -1, 0 };
        xy_t chky[4] = { 0, -1, 0, 1 };

        for (su_t i = 0; i < 4; i++)
        {
            xy_t px = map->teleport[t].x + chkx[i];
            xy_t py = map->teleport[t].y + chky[i];

            if (map->buf[py][px] == ICON_SPACE)
            {
                map->buf[py][px] = pmv->from_obj;
                player_process_old_pos(pmv);
                pmv->from_x = pmv->to_x = px;
                pmv->from_y = pmv->to_y = py;
                xy_t tlx = map->tpview[t].x;
                xy_t tly = map->tpview[t].y;

                if (tlx + screen_data->garea_w >= MAP_W)
                    tlx -= (tlx + screen_data->garea_w) - MAP_W;

                if (tly + screen_data->garea_h >= MAP_H)
                    tly -= (tly + screen_data->garea_h) - MAP_W;

                map->view[player.player].x = tlx;
                map->view[player.player].y = tly;
                player_teleport_animate(pmv, TP_OUT);
                game_win_display();
                flushinp();

                return TRUE;
            }
        }
    }

    /* sorry, we can't teleport you today due to snow on the road */
    struct timespec rpause;
    struct timespec repause;

    rpause.tv_sec = 0;
    rpause.tv_nsec = 0;

    if (player.replay)
    {
        if (!options->replay_hyper)
            rpause.tv_nsec =
                options_replay_speed(options->replay_speed) / 5;
    }
    else
        rpause.tv_nsec = 750000000L;

    nanosleep(&rpause, &repause);
    pmv->to_x = pmv->from_x;
    pmv->to_y = pmv->from_y;
    game_win_display();
    player_teleport_animate(pmv, TP_OUT);
    map->buf[pmv->from_y][pmv->from_x] = pmv->from_obj;
    flushinp();

    return FALSE;
}


void player_teleport_animate(struct xor_move *pmv, enum TP_ANIM dir)
{
    struct xy *dmxy = 0;
    struct xor_icon *icon = 0;

    if (dir == TP_IN)
    {
        if (!(dmxy = game_win_map_coord(pmv->from_x, pmv->from_y)))
            return;             /* should be unlikely */

        icon = &icons[pmv->from_obj];
        wattrset(game_win, COLOR_PAIR(ICON_SPACE));
    }
    else
    {
        if (!(dmxy = game_win_map_coord(pmv->to_x, pmv->to_y)))
            return;             /* should be unlikely */

        icon = &icons[pmv->from_obj];
        wattrset(game_win, COLOR_PAIR(pmv->from_obj));
    }

    struct timespec rpause;
    struct timespec repause;

    rpause.tv_sec = 0;
    rpause.tv_nsec = 0;

    if (player.replay)
    {
        if (!options->replay_hyper)
            rpause.tv_nsec =
                options_replay_speed(options->replay_speed) / 5;
    }
    else
        rpause.tv_nsec = 12500000L;

    for (xy_t y = dmxy->y; y < dmxy->y + ICON_H; y++)
    {
        xy_t stx, enx, xinc;

        if (y & 1)
        {
            stx = dmxy->x + ICON_W - 1;
            enx = dmxy->x - 1;
            xinc = -1;
        }
        else
        {
            stx = dmxy->x;
            enx = dmxy->x + ICON_W;
            xinc = 1;
        }

        for (xy_t x = stx; x != enx; x += xinc)
        {
            mvwaddch(game_win, y, x, icon->chrs[y - dmxy->y][x - dmxy->x]);
            wrefresh(game_win);
            nanosleep(&rpause, &repause);
        }
    }
    free(dmxy);
}
