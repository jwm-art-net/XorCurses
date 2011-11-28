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
    if (move == MV_PLAYER_SWAP) {
        if ((player.player ? player.p0_alive : player.p1_alive)) {
            player.player ^= 1;
            game_win_swap_update();
            game_win_display(map);
            info_win_update_player_icon();
            return PLAY_CONTINUE | PLAY_RECORD;
        }
        return PLAY_CONTINUE;
    }
    struct xor_move *pmv = create_xor_move(map->player[player.player].x,
                                           map->player[player.player].y, move);

    /* cont required after switch */
    ct_t cont = move_object_init(pmv);

    switch (cont) {
    case CT_BLOCK:
        free(pmv);
        return PLAY_CONTINUE;
    case CT_EXIT:
        free(pmv);
        if (player.masks_collected == map->mask_count)
            return PLAY_COMPLETE | PLAY_RECORD;
        return PLAY_CONTINUE;
    case CT_TELEPORT:
        if (player_teleport(pmv)) {
            player_process_old_pos(pmv);
            free(pmv);
            return PLAY_CONTINUE | PLAY_RECORD;
        }
        free(pmv);
        return PLAY_CONTINUE;
        break;
    default:
        break;
    }
    struct xor_move *omv = player_process_move(pmv);

    if (omv) {
        if (omv == pmv) {
            game_win_move_player(pmv);
            if (cont == CT_PICKUP)
                player_process_collect(pmv);
            player_process_old_pos(pmv);
        }
        else {
            game_win_move_object(omv);
            game_win_move_player(pmv);
            if (cont == CT_PICKUP)
                player_process_collect(pmv);
            pushed_process_new_pos(omv);
            player_process_old_pos(pmv);
        }
        free(pmv);
        return PLAY_PROCESS_MOVE | PLAY_RECORD;
    }
    free(pmv);
    return PLAY_CONTINUE;
}

struct xor_move *
player_process_move(struct xor_move *pmv)
{
    struct xor_move *omv = pmv;

    enum CONTACT cont = actions[pmv->to_obj].cont;

    if (cont != CT_PASS) {
        if (cont & CT_PUSH || cont == CT_HARDPUSH) {
            omv = create_xor_move(pmv->to_x, pmv->to_y, pmv->dir);
            if (move_object_init(omv) != CT_PASS) {
                free(omv);
                return 0;       /* blocked, blocking player's move */
            }
            map->buf[omv->from_y][omv->from_x] = ICON_SPACE;
            map->buf[omv->to_y][omv->to_x] = omv->from_obj;
        }
    }
    map->player[player.player].x = pmv->to_x;
    map->player[player.player].y = pmv->to_y;
    map->buf[pmv->to_y][pmv->to_x] = pmv->from_obj;
    map->buf[pmv->from_y][pmv->from_x] = ICON_SPACE;
    return omv;
}

enum PLAY_STATE
player_process_old_pos(struct xor_move *pmv)
{
    debug("\nplayer_process_old_pos(xor_move* pmv=%lx)\n",
           (unsigned long) pmv);

    su_t check1 = (MV_HORIZ & pmv->dir) ? MV_UP : MV_RIGHT;

    su_t check2 = (check1 == MV_UP) ? MV_RIGHT : MV_UP;

    su_t gdir1 = (check1 == MV_UP) ? MV_DOWN : MV_LEFT;

    su_t gdir2 = (check2 == MV_UP) ? MV_DOWN : MV_LEFT;

    xy_t x = pmv->from_x + (check1 == MV_RIGHT ? 1 : 0);

    xy_t y = pmv->from_y - (check1 == MV_UP ? 1 : 0);

    struct xor_move *gravmv = create_gravity_chain_xydir(x, y, gdir1);

    if (gravmv)
        move_gravity_process(gravmv);
    x = pmv->from_x + (check2 == MV_RIGHT ? 1 : 0);
    y = pmv->from_y - (check2 == MV_UP ? 1 : 0);
    if ((gravmv = create_gravity_chain_xydir(x, y, gdir2)))
        move_gravity_process(gravmv);
    return PLAY_PROCESS_MOVE;
}

enum PLAY_STATE
pushed_process_new_pos(struct xor_move *omv)
{
    debug("\npushed_process_new_pos(xor_move* omv=%lx)\n",
           (unsigned long) omv);

    if (omv->from_obj != ICON_DOLL)
        omv->dir = actions[omv->from_obj].mvi_dir;
    omv->from_x = omv->to_x;
    omv->from_y = omv->to_y;
    if (move_object_init(omv) == CT_PASS) {
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

void
player_process_collect(struct xor_move *pmv)
{
    switch (pmv->to_obj) {
    case ICON_MASK:
        player.masks_collected++;
        info_win_map_erase_mask(pmv->to_x, pmv->to_y);
        if ((player.masks_collected % 4) == 0)
            info_win_update_map(player.have_map);
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

void
player_death(su_t icon)
{
    su_t p = (icon == ICON_PLAYER0) ? 0 : 1;
    char *msg[2] = { " Oops! ", "Gotcha!" };
    map->buf[map->player[p].y][map->player[p].x] = ICON_SPACE;
    if (p)
        player.p1_alive = 0;
    else
        player.p0_alive = 0;
    if ((p ? player.p0_alive : player.p1_alive)) {
        player.player ^= 1;
        info_win_update_player_icon();
    }
    scr_wmsg_pause(game_win, msg[p], 0, 0, TRUE);
    if (player.replay)
        nodelay(game_win, TRUE);

    debug("\n\n%s\n\n", msg[p]);

    game_win_display();
}

void
player_process_map_pc(struct xor_move *pmv)
{
    su_t i;

    su_t mb = 1;

    for (i = 0; i < 4; i++, mb *= 2) {
        if (pmv->to_x == map->mappc[i].x && pmv->to_y == map->mappc[i].y) {
            player.have_map |= mb;
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
