#include "moves.h"
#include "game_display.h"
#include "explode.h"
#include "options.h"
#include <stdlib.h>
#include <time.h>
#include "debug.h"


ct_t move_object_init(struct xor_move * move)
{
    move->to_x = move->from_x;
    move->to_y = move->from_y;

    switch (move->dir)
    {
    case MV_LEFT:   move->to_x--; break;
    case MV_RIGHT:  move->to_x++; break;
    case MV_UP:     move->to_y--; break;
    case MV_DOWN:   move->to_y++; break;
    default:    return CT_ERROR;
    }

    move->from_obj = map->buf[move->from_y][move->from_x];
    move->to_obj =   map->buf[move->to_y][move->to_x];
    ct_t cont = actions[move->to_obj].cont;

    /* ------------------------------------------ */
    /* conditions apply both to player and object */
    if (cont == CT_BLOCK || cont == CT_PASS)
        return cont;

    if (move->from_obj == ICON_DOLL)
        return CT_BLOCK;

    if (cont == CT_FILTER)
        return (actions[move->to_obj].cont_dir & move->dir)
                    ? CT_PASS
                    : CT_BLOCK;

    /* -------------------------------- */
    /* conditions which apply to player */
    if (actions[move->from_obj].mvini & MVI_PLAYER)
    {
        switch (cont)
        {
        case CT_EXIT:
        case CT_PICKUP:
        case CT_HARDPUSH:
        case CT_TELEPORT:
            return cont;

        default:
            if (cont & CT_PUSH)
                return (actions[move->to_obj].cont_dir & move->dir)
                    ? CT_PUSH
                    : CT_BLOCK;

            return CT_ERROR;
        }
    }

    /* -------------------------------- */
    /* conditions which apply to object */

    return (cont & CT_EXPLODE) ? CT_EXPLODE : CT_BLOCK;
}


void move_gravity_process(struct xor_move *xmv)
{
    debug("\nmove_gravity_process(xor_move* xmv=%lx)\n",(unsigned long)xmv);

    struct timespec rpause;
    struct timespec repause;

    xmvlist_create();
    xmvlist_append_xor_move(xmv);
    xmvlist_first();
    rpause.tv_sec = 0;

    if (player.replay)
        rpause.tv_nsec = options_replay_speed(options->replay_speed);
    else
        rpause.tv_nsec = 20000000L;

    while (xmvlist->current)
    {
        struct xor_move *xmv = xmvlist->current->xmv;

        debug("xmvlist->current=%lx->xmv=%lx\n",
               (unsigned long) xmvlist->current, (unsigned long) xmv);

        struct xor_move *cmv = xmv;

        while (cmv)
        {
            nanosleep(&rpause, &repause);

            debug("mv_grv_proc:\n"
                    "\t*(%lx)cmv->from_x:%d,from_y:%d,from_obj:%s\n",
                    (unsigned long) cmv, cmv->from_x, cmv->from_y,
                    icons[cmv->from_obj].name);

            struct xor_move *cmv_next = cmv->chain;

            ct_t cont = move_object_init(cmv);

            if (actions[cmv->to_obj].mvini == MVI_PLAYER)
            {
                if (cmv->moves_count > 0)
                {
                    player_death(cmv->to_obj);
                    cont = CT_PASS;
                }
            }

            if (cont == CT_EXPLODE)
            {

                debug("\nBANG!!!\n");

                explode_process_detonator(xmvlist->current);
                cmv = cmv_next;

                debug("\nreturn to ... (xor_move* xmv=%lx)\n",
                        (unsigned long) xmv);
                debug("\tcmv=cmv_next=%lx\n", (unsigned long) cmv);

#ifdef DEBUG /* not just debug message */
                if (cmv)
                    debug("\t*(%lx)cmv->from_x:%d,from_y:%d,from_obj:%s\n",
                           (unsigned long) cmv, cmv->from_x, cmv->from_y,
                           icons[cmv->from_obj].name);
#endif
            }
            else if (cont == CT_PASS)
            {
                xy_t tmpx = cmv->from_x;
                xy_t tmpy = cmv->from_y;
                map->buf[tmpy][tmpx] = ICON_SPACE;
                map->buf[cmv->to_y][cmv->to_x] = cmv->from_obj;
                game_win_move_object(cmv);

                cmv->from_x = cmv->to_x;
                cmv->from_y = cmv->to_y;
                cmv->moves_count++;

                if (cmv_next)
                {
                    cmv = cmv_next;
                    if (!options->replay_hyper)
                        rpause.tv_nsec /= 1.1;
                }
                else
                {
                    if (cmv->dir == MV_DOWN)
                        cmv = create_gravity_chain_xydir(tmpx + 1, tmpy,
                                                                MV_LEFT);
                    else
                        cmv = create_gravity_chain_xydir(tmpx, tmpy - 1,
                                                                MV_DOWN);
                    if (cmv)
                    {
                        xmvlist_append_xor_move(cmv);
                        if (!options->replay_hyper)
                            rpause.tv_nsec /= 1.1;
                    }
                    xmvlist_cycle_next();
                    cmv = xmv = xmvlist->current->xmv;

                    debug("xmvlist->current=%lx->xmv=%lx\n",
                           (unsigned long) xmvlist->current,
                           (unsigned long) xmv);
                }
            }
            else if ((cmv = move_unchain_blocked_bomb(xmvlist->current)))
                xmv = cmv;
            else {              /* chain blocked - remove it */
                debug("mv_grav_proc() chain blocked removing...\n");

                struct xmv_link *tmplnk = xmvlist->current;

#ifdef DEBUG /* not just debug message */
                if (tmplnk->xmv != xmv)
                    debug("\n***** current->xmv != xmv *******\n");
#endif
                destroy_gravity_chain(xmv);
                if (xmvlist_cycle_next() == tmplnk)
                    cmv = xmv = 0;      /* about to remove last chain */
                else
                    cmv = xmv = xmvlist->current->xmv;
                xmvlist_unlink_xor_move(tmplnk);
            }
        }
    }
    xmvlist_destroy();

    debug("\nEXITING ... (xor_move* xmv=%lx)\n", (unsigned long) xmv);
}

struct xor_move *
move_unchain_blocked_bomb(struct xmv_link *lnk)
{
    struct xor_move *xmv = lnk->xmv;

    debug("move_unchain_blocked_bomb(xmv_link* lnk=%lx)\n",
           (unsigned long) lnk);
    debug("moves count:%d\n", xmv->moves_count);

    if (xmv->moves_count == 0)
        return 0;
    struct xor_move *tmp;

    su_t bomb_type = (xmv->dir == MV_DOWN) ? ICON_H_BOMB : ICON_V_BOMB;

    while (xmv) {
        if (xmv->from_obj == bomb_type) {
            if ((tmp = xmv->chain)) {
                xmv->chain = 0;
                destroy_gravity_chain(lnk->xmv);
                return (lnk->xmv = tmp);
            }
            return 0;
        }
        else {
            tmp = xmv;
            xmv = xmv->chain;
        }
    }
    return 0;
}

/*  thankfully, the doll cannot move through force-fields, nor
    detonate bombs. so all we need do is check that where it is
    moving is ICON_SPACE to continue moving it, otherwise stop.
*/
void
move_hard_push_doll(struct xor_move *xmv)
{
    struct timespec rpause;

    struct timespec repause;

    rpause.tv_sec = 0;
    if (player.replay)
        rpause.tv_nsec = options_replay_speed(options->replay_speed);
    else
        rpause.tv_nsec = 20000000L;
    while (map->buf[xmv->to_y][xmv->to_x] == ICON_SPACE) {
        map->buf[xmv->from_y][xmv->from_x] = ICON_SPACE;
        map->buf[xmv->to_y][xmv->to_x] = ICON_DOLL;
        game_win_move_object(xmv);
        xmv->from_x = xmv->to_x;
        xmv->from_y = xmv->to_y;
        switch (xmv->dir) {
        case MV_LEFT:
            xmv->to_x--;
            break;
        case MV_RIGHT:
            xmv->to_x++;
            break;
        case MV_UP:
            xmv->to_y--;
            break;
        case MV_DOWN:
            xmv->to_y++;
            break;
        default:
            return;
        }
        nanosleep(&rpause, &repause);
    }
    free(xmv);
}
