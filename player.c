#include "player.h"
#include "moves.h"
#include "game_display.h"
#include "actions.h"
#include "moves.h"
#include "movelist.h"
#include "info.h"
#include "teleport.h"
#include "screen.h"
#include "debug.h"

#include <time.h>
#include <stdlib.h>


static enum PLAY_STATE player_process_push(struct xor_move* pmv);

struct xor_player player;

void
player_init()
{
    player.masks_collected = 0;
    player.moves_remaining = MAX_MOVES;
    player.have_map = 0;
    player.p0_alive = 1;
    player.p1_alive = 1;
    player.player = 1;
    player.replay = FALSE;
    player.wall_vis = TRUE;
    player.map_view_y = 0;
    player.set_breakpoint = FALSE;

    int p;

    for (p = 0; p < 2; ++p)
    {
        player.xmv[p].from_x = map->player[p].x;
        player.xmv[p].from_y = map->player[p].y;
        player.xmv[p].from_obj = ICON_PLAYER0 + p;
    }
}

enum PLAY_STATE
player_move(su_t move)
{
    if (player.moves_remaining == 0)
        return PLAY_ZERO_MOVES;

    if (move & MV_PLAYER_QUIT)
        return (player.moves_remaining == MAX_MOVES)
                ? PLAY_QUIT
                : PLAY_QUIT | PLAY_RECORD;

    if (move == MV_PLAYER_SWAP)
    {
        if ((player.player ? player.p0_alive : player.p1_alive))
        {
            player.player ^= 1;
            game_win_swap_update();
            game_win_display();
            info_win_update_player_icon();
            return PLAY_CONTINUE | PLAY_RECORD;
        }
        return PLAY_CONTINUE;
    }

    struct xor_move* pmv = &player.xmv[player.player];
    pmv->dir = move;
    ct_t cont = move_object_init(pmv);

    switch (cont)
    {
    case CT_BLOCK:
        return PLAY_CONTINUE;

    case CT_EXIT:

        if (player.masks_collected == map->mask_count)
            return PLAY_COMPLETE | PLAY_RECORD;

        return PLAY_CONTINUE;

    case CT_TELEPORT:
        debug("before teleport pmv->from: %d, %d\n",
                                        pmv->from_x, pmv->from_y);
        if (player_teleport(pmv))
        {
            debug("after teleport pmv->from: %d, %d\n",
                                        pmv->from_x, pmv->from_y);
            return PLAY_CONTINUE | PLAY_RECORD;
        }

        return PLAY_CONTINUE;

    default:
        break;
    }

    if (cont & CT_PICKUP)
        player_process_collect(pmv);

    if (cont & CT_PUSH || cont & CT_HARDPUSH)
        return player_process_push(pmv);

    map->buf[pmv->to_y]  [pmv->to_x] = pmv->from_obj;
    map->buf[pmv->from_y][pmv->from_x] = ICON_SPACE;

    game_win_move_player(pmv);
    player_process_old_pos(pmv);
    pmv->from_x = pmv->to_x;
    pmv->from_y = pmv->to_y;
    pmv->from_obj = pmv->to_obj;

    return PLAY_PROCESS_MOVE | PLAY_RECORD;
}


enum PLAY_STATE player_process_old_pos(struct xor_move *pmv)
{
    debug("(xor_move* pmv=%lx)\n", (unsigned long) pmv);

    su_t check1 = (MV_HORIZ & pmv->dir) ? MV_UP    : MV_RIGHT;
    su_t check2 = (check1 == MV_UP)     ? MV_RIGHT : MV_UP;

    su_t gdir1 = (check1 == MV_UP) ? MV_DOWN : MV_LEFT;
    su_t gdir2 = (check2 == MV_UP) ? MV_DOWN : MV_LEFT;

    xy_t x = pmv->from_x + (check1 == MV_RIGHT ? 1 : 0);
    xy_t y = pmv->from_y - (check1 == MV_UP    ? 1 : 0);

    struct xor_move *gravmv = create_gravity_chain_xydir(x, y, gdir1);

    if (gravmv)
        move_gravity_process(gravmv);

    x = pmv->from_x + (check2 == MV_RIGHT ? 1 : 0);
    y = pmv->from_y - (check2 == MV_UP ? 1 : 0);

    if ((gravmv = create_gravity_chain_xydir(x, y, gdir2)))
        move_gravity_process(gravmv);

    return PLAY_PROCESS_MOVE;
}


enum PLAY_STATE pushed_process_new_pos(struct xor_move *omv)
{
    debug("\npushed_process_new_pos(xor_move* omv=%lx)\n",
           (unsigned long) omv);

    if (omv->from_obj != ICON_DOLL)
        omv->dir = actions[omv->from_obj].mvi_dir;

    omv->from_x = omv->to_x;
    omv->from_y = omv->to_y;

    if (move_object_init(omv) == CT_PASS)
    {
        /*  the call to move_object_init will be repeated by
           move_gravity_process... but more importantly, having
           it here prevents a bomb exploding when an object is
           pushed on to it. */
        if (omv->from_obj == ICON_DOLL)
            move_hard_push_doll(omv);
        else if (create_gravity_chain(omv))
            move_gravity_process(omv);
        else
            free(omv);
    }
    else
        free(omv);

    return PLAY_PROCESS_MOVE;
}


void player_process_collect(struct xor_move *pmv)
{
    switch (pmv->to_obj)
    {
    case ICON_MASK:
        player.masks_collected++;
        map->buf[pmv->to_y]  [pmv->to_x] = ICON_SPACE;
        info_win_map_erase_mask(pmv->to_x, pmv->to_y);
        break;

    case ICON_MAP:
        player_process_map_pc(pmv);
        break;

    case ICON_SWITCH:
        player.wall_vis = (player.wall_vis ? FALSE : TRUE);
        init_wall(map->level, player.wall_vis);
        game_win_display();

    default:
        break;
    }
}


static enum PLAY_STATE player_process_push(struct xor_move* pmv)
{
    struct xor_move* omv =
        create_xor_move(pmv->to_x, pmv->to_y, pmv->dir);

    if (move_object_init(omv) != CT_PASS)
    {
        destroy_gravity_chain(omv);
        return PLAY_CONTINUE;
    }

    map->buf[omv->to_y]  [omv->to_x] =   omv->from_obj;
    map->buf[omv->from_y][omv->from_x] = ICON_SPACE;
    map->buf[pmv->to_y]  [pmv->to_x] =   pmv->from_obj;
    map->buf[pmv->from_y][pmv->from_x] = ICON_SPACE;

    game_win_icon_display(omv->from_x, omv->from_y, ICON_SPACE);
    game_win_icon_display(omv->to_x,   omv->to_y,   omv->from_obj);

    /*game_win_display();*/

    game_win_move_player(pmv);
    omv->from_x = omv->to_x;
    omv->from_y = omv->to_y;
    pushed_process_new_pos(omv);/* and free(omv) */
    player_process_old_pos(pmv);
    pmv->from_obj = pmv->to_obj;
    pmv->from_x = pmv->to_x;
    pmv->from_y = pmv->to_y;

    return PLAY_PROCESS_MOVE | PLAY_RECORD;
}


void player_death(su_t icon)
{
    char *msg[2] = { " Oops! ", "Gotcha!" };
    su_t p = icon - ICON_PLAYER0;

    if (p)
        player.p1_alive = 0;
    else
        player.p0_alive = 0;

    if (p == player.player)
    {
        if ((p ? player.p0_alive : player.p1_alive))
        {
            player.player ^= 1;
            info_win_update_player_icon();
        }
    }

    scr_wmsg_pause(game_win, msg[p], 0, 0, TRUE);

    if (player.replay)
        nodelay(game_win, TRUE);

    debug("\n\n%s\n\n", msg[p]);

    game_win_display();
}


void player_process_map_pc(struct xor_move *pmv)
{
    su_t i;

    su_t mb = 1;

    for (i = 0; i < 4; i++, mb *= 2)
    {
        if (pmv->to_x == map->mappc[i].x && pmv->to_y == map->mappc[i].y)
        {
            debug("collected map piece ix:%d id:%d\n", i, mb);
            player.have_map |= mb;
            debug("player.have_map:%d\n", player.have_map);
            info_win_dump_map(i);
        }
    }
}


#if DEBUG

void player_state_print(int state)
{
    int r = state & PLAY_RECORD;
    state ^= PLAY_RECORD;
    ENUM_MSG(state, PLAY_CONTINUE);
    ENUM_MSG(state, PLAY_PROCESS_MOVE);
    ENUM_MSG(state, PLAY_SWAP);
    ENUM_MSG(state, PLAY_QUIT);
    ENUM_MSG(state, PLAY_ZERO_MOVES);
    ENUM_MSG(state, PLAY_GOTCHA);
    ENUM_MSG(state, PLAY_COMPLETE);
    ENUM_MSG(r, PLAY_RECORD);
}

#endif
