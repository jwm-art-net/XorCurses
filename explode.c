#include "explode.h"

#include "game_display.h"
#include "player.h"
#include "options.h"
#include "info.h"
#include "screen.h"
#include "debug.h"

#include <stdlib.h>
#include <time.h>

void
explode_process_detonator(struct xmv_link *detlnk)
{
    debug("explode_process_detonator(detlnk=%lx)\n", (unsigned long)detlnk);
    debug("detlnk->xmv=%lx\n", (unsigned long)detlnk->xmv);

    struct xor_move *detmv = detlnk->xmv;

    xy_t bombx = detmv->to_x;

    xy_t bomby = detmv->to_y;

    su_t bomb = map->buf[bomby][bombx];
    xy_t exp_x[4] = { detmv->from_x, bombx, 0, 0 };
    xy_t exp_y[4] = { detmv->from_y, bomby, 0, 0 };
    bool exp[4] = { TRUE, TRUE, TRUE, TRUE };
    enum MOVES detdir = detmv->dir;

    switch (bomb) {
    case ICON_H_BOMB:
        exp_x[2] = bombx - 1;
        exp_y[2] = bomby;
        if (detdir == MV_LEFT)
            exp[3] = FALSE;
        else {
            exp_x[3] = bombx + 1;
            exp_y[3] = bomby;
        }
        break;
    case ICON_V_BOMB:
        if (detdir == MV_DOWN)
            exp[2] = FALSE;
        else {
            exp_x[2] = bombx;
            exp_y[2] = bomby - 1;
        }
        exp_x[3] = bombx;
        exp_y[3] = bomby + 1;
        break;
    }

    su_t i;

    for (i = 0; i < 4; i++)
    {
        if (exp_x[i] == 0 || exp_x[i] > MAP_W - 2
         || exp_y[i] == 0 || exp_y[i] > MAP_H - 2)
        {
            exp[i] = FALSE;
        }
        else
        {
            debug("valid explosion point [%d] x:%d y%d\n",
                                        i, exp_x[i], exp_y[i]);
        }
    }

    struct timespec rpause;

    struct timespec repause;

    rpause.tv_sec = 0;
    if (player.replay)
        rpause.tv_nsec = options_replay_speed(options->replay_speed);
    else
        rpause.tv_nsec = 60000000L;
    for (xy_t xi = ICON_EXPLOSION1; xi <= ICON_EXPLOSION3; xi++) {
        for (i = 0; i < 4; i++) {
            if (exp[i]) {
                game_win_icon_display(exp_x[i], exp_y[i], xi);
            }
        }
        wrefresh(game_win);
        if (!options->replay_hyper)
            rpause.tv_nsec /= 1.175;
        nanosleep(&rpause, &repause);
    }

    su_t dead1 = 0;

    su_t dead2 = 0;

    for (i = 0; i < 4; i++)
        if (exp[i]) {
            game_win_icon_display(exp_x[i], exp_y[i], ICON_SPACE);
            su_t icon = map->buf[exp_y[i]][exp_x[i]];
            map->buf[exp_y[i]][exp_x[i]] = ICON_SPACE;

            if (actions[icon].mvini == MVI_PLAYER) {
                if (dead1 == 0)
                    dead1 = icon;
                else
                    dead2 = icon;
            }
            else {
                switch (icon) {
                case ICON_SWITCH:
                    player.wall_vis = (player.wall_vis ? FALSE : TRUE);
                    init_wall(map->level, player.wall_vis);
                    game_win_display();
                    break;
                case ICON_MASK:
                    info_win_map_erase_mask(exp_x[i], exp_y[i]);
                    break;
                case ICON_WALL:
                    if (screen_data->scale_map == FALSE)
                        info_win_map_erase_mask(exp_x[i], exp_y[i]);
                    break;
                }
            }
        }
    wrefresh(game_win);
    /*  if detonator part of chain, remove it from chain, delete it,
       and set detlnk to point to new head of chain. if not part
       of a chain, unlink, and delete.
     */
    if (dead1)
        player_death(dead1);
    if (dead2)
        player_death(dead2);
    if (detmv->chain) {
        debug("pruning chain: detlink->xmv=(was)%lx "
              "(now %lx->right)= %lx\n",
               (unsigned long) detlnk->xmv, (unsigned long) detmv,
               (unsigned long) detmv->chain);
        debug("freeing detmv\n");

        detlnk->xmv = detmv->chain;
        free(detmv);
        detmv = 0;
    }
    else
        free(xmvlist_unlink_xor_move(detlnk));
    if (bomb == ICON_H_BOMB)
        explode_process_h_blast(bombx, bomby, detdir);
    else
        explode_process_v_blast(bombx, bomby, detdir);
}

void
explode_process_h_blast(xy_t bombx, xy_t bomby, su_t detdir)
{
    xy_t chkx[4];

    xy_t chky[4];

    xy_t dir[4];

    chkx[0] = chkx[1] = bombx + 1;
    chky[0] = chky[1] = bomby - 1;
    dir[0] = MV_DOWN;
    dir[1] = MV_LEFT;
    if (detdir == MV_DOWN) {
        chkx[2] = bombx + 2;
        chky[2] = bomby;
        dir[2] = MV_LEFT;
    }
    else {
        chkx[2] = bombx;
        chky[2] = bomby - 1;
        dir[2] = MV_DOWN;
    }
    chkx[3] = bombx - 1;
    chky[3] = bomby - 1;
    dir[3] = MV_DOWN;
    for (su_t i = 0; i < 4; i++)
        if (chkx[i] > 0 && chkx[i] < MAP_W - 1 && chky[i] > 0
            && chky[i] < MAP_H - 1)
            if (!xmvlist_contains_coord(chkx[i], chky[i], 0, 0)) {
                struct xor_move *xmv =
                    create_gravity_chain_xydir(chkx[i], chky[i], dir[i]);
                if (xmv)
                    xmvlist_append_xor_move(xmv);
            }
}

void
explode_process_v_blast(xy_t bombx, xy_t bomby, su_t detdir)
{
    xy_t chkx[4];

    xy_t chky[4];

    su_t dir[4];

    chkx[0] = chkx[1] = bombx + 1;
    chky[0] = chky[1] = bomby - 1;
    dir[0] = MV_LEFT;
    dir[1] = MV_DOWN;
    if (detdir == MV_LEFT) {
        chkx[2] = bombx;
        chky[2] = bomby - 2;
        dir[2] = MV_DOWN;
    }
    else {
        chkx[2] = bombx + 1;
        chky[2] = bomby;
        dir[2] = MV_LEFT;
    }
    chkx[3] = bombx + 1;
    chky[3] = bomby + 1;
    dir[3] = MV_LEFT;
    for (su_t i = 0; i < 4; i++)
        if (chkx[i] > 0 && chkx[i] < MAP_W - 1 && chky[i] > 0
            && chky[i] < MAP_H - 1)
            if (!xmvlist_contains_coord(chkx[i], chky[i], 0, 0)) {
                struct xor_move *xmv =
                    create_gravity_chain_xydir(chkx[i], chky[i], dir[i]);
                if (xmv)
                    xmvlist_append_xor_move(xmv);
            }
}
