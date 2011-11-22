#include "info.h"
#include "player.h"
#include "game_display.h"
#include "icons.h"
#include "screen.h"
#include "debug.h"

#include <string.h>

void
info_win_update_player_icon()
{
    win_icon_dump(info_win, 2, 2, ICON_PLAYER0 + player.player);
}

bool
map_overwrite_bad(xy_t x, xy_t y)
{
    xy_t xx = (x % 2) ? x - 1 : x;

    xy_t yy = (y % 2) ? y - 1 : y;

    if (x % 2) {
        if (map->buf[y][xx] == ICON_MASK)
            return TRUE;
    }
    else if (x < MAP_W - 1) {
        if (map->buf[y][x + 1] == ICON_MASK)
            return TRUE;
    }
    if (y % 2) {
        if (map->buf[yy][x] == ICON_MASK)
            return TRUE;
    }
    else if (y < MAP_H - 1) {
        if (map->buf[y + 1][x] == ICON_MASK)
            return TRUE;
    }
    if (x % 2 && y % 2) {
        if (map->buf[yy][xx] == ICON_MASK)
            return TRUE;
    }
    else if (x < MAP_W - 1 && y < MAP_H - 1) {
        if (map->buf[y + 1][x + 1] == ICON_MASK)
            return TRUE;
    }
    return FALSE;
}

void
info_win_update_map(su_t have_map)
{
    for (su_t i = 0, mb = 1; i < 4; i++, mb *= 2) {
        if (mb & have_map)
            info_win_dump_map(i);
    }
}

void
info_win_dump_map(su_t mappc)
{
    xy_t x = 0;
    xy_t y = 0;
    switch (mappc) {
    case 0:
        x = HMAP_W;
        break;
    case 1:
        x = HMAP_W;
        y = HMAP_H;
        break;
    case 2:
        y = HMAP_H;
        break;
    }
    xy_t d = (screen_data->scale_map == TRUE ? 2 : 1);
    xy_t lx = x + HMAP_W;
    xy_t ly = y + HMAP_H;
    for (xy_t xx = x; xx < lx; xx++) {
        for (xy_t yy = y; yy < ly; yy++) {
            su_t icon = map->buf[yy][xx];
            switch (icon) {
            case ICON_WALL:
                if (screen_data->scale_map == TRUE)
                    if (map_overwrite_bad(xx, yy))
                        break;
                wattrset(info_win, COLOR_PAIR(COL_I_MAP_WALL));
                mvwaddch(info_win,
                         screen_data->map_tly + yy / d,
                         screen_data->map_tlx + xx / d,
                                        icon_to_mapchar(icon));
                break;
            case ICON_MASK:
            case ICON_EXIT:
                wattrset(info_win, COLOR_PAIR(icon));
                mvwaddch(info_win,
                         screen_data->map_tly + yy / d,
                         screen_data->map_tlx + xx / d,
                                        icon_to_mapchar(icon));
                break;
            default:
                break;
            }
        }
    }
}

void
info_win_map_erase_mask(xy_t x, xy_t y)
{
    wattrset(info_win, COLOR_PAIR(ICON_SPACE));
    if (!screen_data->scale_map) {
        mvwaddch(info_win,
                 screen_data->map_tly + y, screen_data->map_tlx + x,
                 icon_to_mapchar(ICON_SPACE));
        return;
    }
    if (map_overwrite_bad(x, y))
        return;
    mvwaddch(info_win,
             screen_data->map_tly + y / 2,
             screen_data->map_tlx + x / 2, icon_to_mapchar(ICON_SPACE));
}

void
info_win_display()
{
    wattrset(info_win, COLOR_PAIR(COL_I_TXT));
    mvwprintw(info_win,
              0, (screen_data->info_w - (strlen(map->name) + 2)) / 2,
              " %d. %s ", map->level, map->name);
    mvwprintw(info_win, 2, 8, "masks: %d of %d ", player.masks_collected,
              map->mask_count);
    mvwprintw(info_win, 3, 8, "moves: %d ",
                                    MAX_MOVES - player.moves_remaining);
    mvwprintw(info_win, 4, 8, "x:%d  y:%d  ",
              map->player[player.player].x, map->player[player.player].y);
    if (screen_data->scale_map == TRUE)
        mvwprintw(info_win,
                  screen_data->map_tly + HMAP_H,
                  screen_data->map_tlx + 1, "m - map toggle");
    wrefresh(info_win);

    debug("view_x0: %3d view_y0: %3d\n", map->view[0].x, map->view[0].y);
    debug("view_x1: %3d view_y1: %3d\n", map->view[1].x, map->view[1].y);
}
