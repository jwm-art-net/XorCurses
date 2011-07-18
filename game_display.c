#include "game_display.h"
#include "actions.h"
#include "player.h"
#include "options.h"
#include "screen.h"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>

xy_t _ga_w = 0;
xy_t _ga_h = 0;

void
game_win_init_views()
{
    _ga_w = screen_data->garea_w;
    _ga_h = screen_data->garea_h;
    su_t scrt = (player.replay ? 1 : options->scroll_thresh);
    for (su_t p = 0; p < 2; p++) {
        xy_t ox = map->view[p].x;
        /*  prevent void beyond right hand wall of map from displaying
           during resize. note that test condition is over-enthusiastic
           to the point of paranoia, but it needs to be. Unfortunately
           this removes the player when right side of view is on edge
           of right side map, the garea is being shrunk, and the player
           is on the left of the view - the player pos test corrects it.
         */
        if (ox + _ga_w >= MAP_W - 1) {
            ox -= (ox + _ga_w) - MAP_W;
            if (map->player[p].x - scrt < ox)
                ox -= ox - (map->player[p].x - scrt);
        }
        else if (ox + _ga_w <= map->player[p].x + scrt)
            ox += (map->player[p].x + scrt + 1) - (ox + _ga_w);
        map->view[p].x = ox;
        xy_t oy = map->view[p].y;
        if (oy + _ga_h >= MAP_H - 1) {
            oy -= oy + _ga_h - MAP_H;
            if (map->player[p].y - scrt < oy)
                oy -= oy - (map->player[p].y - scrt);
        }
        else if (oy + _ga_h <= map->player[p].y + scrt)
            oy += (map->player[p].y + scrt + 1) - (oy + _ga_h);
        map->view[p].y = oy;
    }
}

void
game_win_display()
{
    if (_ga_w != screen_data->garea_w || _ga_h != screen_data->garea_h)
        game_win_init_views();
    xy_t ox = map->view[player.player].x;
    xy_t oy = map->view[player.player].y;
    for (xy_t y = 0; y < _ga_h; y++) {
        for (xy_t x = 0; x < _ga_w; x++) {
            game_win_icon_dump(x * ICON_W, y * ICON_H,
                               map->buf[oy + y][ox + x]);
        }
    }
    wrefresh(game_win);
}

void
game_win_show(xy_t tlx, xy_t tly)
{
    if (tlx + _ga_w >= MAP_W)
        tlx -= (tlx + _ga_w) - MAP_W;
    if (tly + _ga_h >= MAP_H)
        tly -= (tly + _ga_h) - MAP_W;
    for (xy_t y = 0; y < _ga_h; y++) {
        for (xy_t x = 0; x < _ga_w; x++) {
            game_win_icon_dump(x * ICON_W, y * ICON_H,
                               map->buf[tly + y][tlx + x]);
        }
    }
    wrefresh(game_win);
}

void
game_win_swap_update()
{
    su_t p = (player.player) ? 0 : 1;

    if (map->player[player.player].x > map->view[p].x
        && map->player[player.player].x <
        map->view[p].x + screen_data->garea_w - 1) {
        if (map->player[player.player].y > map->view[p].y
            && map->player[player.player].y <
            map->view[p].y + screen_data->garea_h - 1) {
            map->view[player.player].x = map->view[p].x;
            map->view[player.player].y = map->view[p].y;
        }
    }
}

void
game_win_move_player(struct xor_move *pmv)
{
    xy_t vx = map->view[player.player].x;

    xy_t vy = map->view[player.player].y;

    xy_t ovx = vx;

    xy_t ovy = vy;

    su_t scrt = (player.replay ? 1 : options->scroll_thresh);

    switch (pmv->dir) {
    case MV_LEFT:
        if (pmv->to_x < vx + scrt && vx > 0)
            vx--;
        break;
    case MV_RIGHT:
        if (pmv->to_x > vx + _ga_w - 1 - scrt && vx + _ga_w < MAP_W)
            vx++;
        break;
    case MV_UP:
        if (pmv->to_y < vy + scrt && vy > 0)
            vy--;
        break;
    case MV_DOWN:
        if (pmv->to_y > vy + _ga_h - 1 - scrt && vy + _ga_h < MAP_H)
            vy++;
        break;
    default:
        break;
    }
    if (ovx != vx || ovy != vy) {
        map->view[player.player].x = vx;
        map->view[player.player].y = vy;
        game_win_display(map);
        return;
    }
    game_win_icon_display(pmv->from_x, pmv->from_y, ICON_SPACE);
    game_win_icon_display(pmv->to_x, pmv->to_y, pmv->from_obj);
    wrefresh(game_win);
}

void
game_win_move_object(struct xor_move *omv)
{
    game_win_icon_display(omv->from_x, omv->from_y, ICON_SPACE);
    game_win_icon_display(omv->to_x, omv->to_y, omv->from_obj);
    wrefresh(game_win);
}

struct xy *
game_win_map_coord(xy_t x, xy_t y)
{
    struct xy *ret = malloc(sizeof(struct xy));

    if (!ret)
        return 0;
    struct xy *pv = &map->view[player.player];

    if (x >= pv->x && x < pv->x + _ga_w && y >= pv->y && y < pv->y + _ga_h) {
        ret->x = (x - pv->x) * ICON_W;
        ret->y = (y - pv->y) * ICON_H;
        return ret;
    }
    free(ret);
    return 0;
}

void
game_win_icon_display(xy_t x, xy_t y, su_t icon)
{
    xy_t vx = map->view[player.player].x;

    xy_t vy = map->view[player.player].y;

    if (x < vx || x >= vx + _ga_w || y < vy || y >= vy + _ga_h)
        return;
    xy_t wx = (x - vx) * ICON_W;

    xy_t wy = (y - vy) * ICON_H;

    wattrset(game_win, COLOR_PAIR(icon));
    for (xy_t yy = 0; yy < ICON_H; yy++)
        for (xy_t xx = 0; xx < ICON_W; xx++)
            mvwaddch(game_win, wy + yy, wx + xx, icons[icon].chrs[yy][xx]);
}

void
game_win_icon_dump(xy_t x, xy_t y, su_t icon)
{
    wattrset(game_win, COLOR_PAIR(icon));
    for (xy_t yy = 0; yy < ICON_H; yy++)
        for (xy_t xx = 0; xx < ICON_W; xx++)
            mvwaddch(game_win, y + yy, x + xx, icons[icon].chrs[yy][xx]);
}

void
game_win_dump_map_sect(xy_t topy, su_t sect, bool show)
{
    xy_t mapstx = 0;
    xy_t mapsty = 0;
    xy_t mapmaxx, mapmaxy;

    xy_t sy = 0;

    switch (sect) {
    case 0:
        mapstx = HMAP_W;
        mapsty = topy;
        mapmaxx = MAP_W;
        mapmaxy = HMAP_H;
        break;
    case 1:
        mapstx = HMAP_W;
        mapsty = HMAP_H;
        mapmaxx = MAP_W;
        mapmaxy = MAP_H;
        sy = HMAP_H - topy;
        break;
    case 2:
        mapsty = HMAP_H;
        mapmaxx = HMAP_W;
        mapmaxy = MAP_H;
        sy = HMAP_H - topy;
        break;
    case 3:
        mapsty = topy;
        mapmaxx = HMAP_W;
        mapmaxy = HMAP_H;
        break;
    default:
        return;
    }
    short ox = (screen_data->garea_w * ICON_W - MAP_W) / 2;
    short oy = (screen_data->garea_h * ICON_H - MAP_H) / 2;
    if (oy == 0)
        topy = 0;
    else if (oy > 0) {
        topy = 0;
        char spcs[MAP_W + 2 + 1];
        memset(spcs, ' ', MAP_W + 2);
        spcs[MAP_W + 2] = '\0';
        wattrset(game_win, COLOR_PAIR(ICON_SPACE));
        mvwaddstr(game_win, oy - 1, ox - 1, spcs);
        mvwaddstr(game_win, oy + MAP_H, ox - 1, spcs);
    }
    else
        oy = 0;
    for (xy_t y = mapsty; y < mapmaxy; y++, sy++) {
        if (ox > 0) {
            wattrset(game_win, COLOR_PAIR(ICON_SPACE));
            mvwaddstr(game_win, oy + sy, ox - 1, " ");
            mvwaddstr(game_win, oy + sy, MAP_W + ox, " ");
        }
        for (xy_t x = mapstx; x < mapmaxx; x++) {
            if (show) {
                xy_t icon = map->buf[y][x];
                switch (icon) {
                case ICON_WALL:
                    wattrset(game_win, COLOR_PAIR(COL_I_MAP_WALL));
                    mvwaddch(game_win, oy + sy, ox + x, icon_to_mapchar(icon));
                    break;
                case ICON_MASK:
                case ICON_EXIT:
                    wattrset(game_win, COLOR_PAIR(icon));
                    mvwaddch(game_win, oy + sy, ox + x, icon_to_mapchar(icon));
                    break;
                default:
                    wattrset(game_win, COLOR_PAIR(ICON_SPACE));
                    mvwaddch(game_win,
                             oy + sy, ox + x, icon_to_mapchar(ICON_SPACE));
                    break;
                }
            }
            else {
                wattrset(game_win, COLOR_PAIR(ICON_SPACE));
                mvwaddch(game_win, oy + sy, ox + x, ' ');
            }
        }
    }
}

void
game_win_dump_map(xy_t topy)
{
    for (su_t i = 0, mb = 1; i < 4; i++, mb *= 2)
        game_win_dump_map_sect(topy, i, mb & player.have_map);
    wrefresh(game_win);
    return;
}

void
game_win_map_display()
{
    bool map_scrollable =
        (screen_data->garea_h * ICON_H < MAP_H ? TRUE : FALSE);
    xy_t y = (map_scrollable ? player.map_view_y : 0);
    xy_t maxscrolly = MAP_H - ICON_H * screen_data->garea_h;
    game_win_dump_map(y);
    while (1) {
        int key = wgetch(game_win);
        if (key != ERR) {
            switch (key) {
            case KEY_RESIZE:
                screen_resize();
                map_scrollable =
                    (screen_data->garea_h * ICON_H < MAP_H ? TRUE : FALSE);
                if (!map_scrollable)
                    y = 0;
                else
                    maxscrolly = MAP_H - ICON_H * screen_data->garea_h;
                if (!screen_data->scale_map)
                    return;
                game_win_dump_map(y);
                break;
            case '\'':
            case KEY_UP:
                if (y > 0 && map_scrollable) {
                    y--;
                    game_win_dump_map(y);
                }
                break;
            case '/':
            case KEY_DOWN:
                if (y < maxscrolly && map_scrollable) {
                    y++;
                    game_win_dump_map(y);
                }
                break;
            case 'm':
            case 'M':
            case 'q':
            case 'Q':
                game_win_display(y);
                player.map_view_y = y;
                return;
            }
        }
    }
}

