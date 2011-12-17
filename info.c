#include "info.h"
#include "player.h"
#include "game_display.h"
#include "icons.h"
#include "screen.h"
#include "debug.h"

#include <string.h>

void info_win_update_player_icon()
{
    win_icon_dump(info_win, 2, 2, ICON_PLAYER0 + player.player);
}


void info_win_update_map(su_t have_map)
{
    for (su_t i = 0, mb = 1; i < 4; i++, mb *= 2)
    {
        if (mb & have_map)
            info_win_dump_map(i);
    }

    wrefresh(info_win);
}


static void scale_dump_map(xy_t qx, xy_t qy)
{
    xy_t lx = qx + HMAP_W;
    xy_t ly = qy + HMAP_H;

    struct xy m[] = { {0, 0}, {1, 0}, {0, 1}, {1, 1} };

    for (xy_t x = qx; x < lx; x += 2)
    {
        for (xy_t y = qy; y < ly; y += 2)
        {
            su_t icon = 0;
            su_t i;

            for (i = 0; i < 4; ++i)
            {
                su_t t = map->buf[y + m[i].y][x + m[i].x];

                if (t == ICON_EXIT)
                {
                    icon = t;
                    break;
                }
                else if (t == ICON_MASK)
                    icon = t;
                else if (t == ICON_WALL && icon == 0)
                    icon = t;
            }

            switch (icon)
            {
            case ICON_WALL:
                wattrset(info_win, COLOR_PAIR(COL_I_MAP_WALL));
                mvwaddch(info_win,
                         screen_data->map_tly + y / 2,
                         screen_data->map_tlx + x / 2,
                                                icon_to_mapchar(icon));
                break;

            case ICON_MASK:
            case ICON_EXIT:
                wattrset(info_win, COLOR_PAIR(icon));
                mvwaddch(info_win,
                         screen_data->map_tly + y / 2,
                         screen_data->map_tlx + x / 2,
                                                icon_to_mapchar(icon));
                break;

            default:
                break;
            }
        }
    }
}


/* here, map pieces are numberd 0 - 3 clockwise from top-right */
void info_win_dump_map(su_t mappc)
{
    xy_t x = 0;
    xy_t y = 0;

    switch (mappc)
    {
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

    if (screen_data->scale_map)
    {
        scale_dump_map(x, y);
        return;
    }

    xy_t lx = x + HMAP_W;
    xy_t ly = y + HMAP_H;

    for (xy_t xx = x; xx < lx; xx++)
    {
        for (xy_t yy = y; yy < ly; yy++)
        {
            su_t icon = map->buf[yy][xx];
            switch (icon)
            {
            case ICON_WALL:
                wattrset(info_win, COLOR_PAIR(COL_I_MAP_WALL));
                mvwaddch(info_win,
                         screen_data->map_tly + yy,
                         screen_data->map_tlx + xx,
                                        icon_to_mapchar(icon));
                break;

            case ICON_MASK:
            case ICON_EXIT:
                wattrset(info_win, COLOR_PAIR(icon));
                mvwaddch(info_win,
                         screen_data->map_tly + yy,
                         screen_data->map_tlx + xx,
                                        icon_to_mapchar(icon));
                break;

            default:
                break;
            }
        }
    }
}


static void scale_map_erase_mask(xy_t x, xy_t y)
{
    struct xy m[] = { {0, 0}, {1, 0}, {0, 1}, {1, 1} };
    su_t mappc;
    su_t icon = 0;
    su_t i;

    if (y < HMAP_H)
        mappc = (x < HMAP_W) ? 8 : 1;
    else
        mappc = (x < HMAP_W) ? 4 : 2;

    if (!(player.have_map & mappc))
        return;

    x = x - (x % 2);
    y = y - (y % 2);

    for (i = 0; i < 4; ++i)
    {
        su_t t = map->buf[y + m[i].y][x + m[i].x];

        if (t == ICON_EXIT)
        {
            icon = t;
            break;
        }
        else if (t == ICON_MASK)
            icon = t;
        else if (t == ICON_WALL && icon == 0)
            icon = t;
    }

    wattrset(info_win,
            COLOR_PAIR(icon == ICON_WALL ? COL_I_MAP_WALL : icon));
    mvwaddch(info_win,  screen_data->map_tly + y / 2,
                        screen_data->map_tlx + x / 2,
                        icon_to_mapchar(icon));
}


void info_win_map_erase_mask(xy_t x, xy_t y)
{
    if (screen_data->scale_map)
    {
        scale_map_erase_mask(x, y);
        return;
    }

    wattrset(info_win, COLOR_PAIR(ICON_SPACE));
    mvwaddch(info_win,  screen_data->map_tly + y,
                        screen_data->map_tlx + x,
                        icon_to_mapchar(ICON_SPACE));
}


void info_win_display()
{
    wattrset(info_win, COLOR_PAIR(COL_I_TXT));
    mvwprintw(info_win, 0,
                        (screen_data->info_w - (strlen(map->name) + 2)) / 2,
                        " %d. %s ", map->level, map->name);

    mvwprintw(info_win, 2, 8, "masks: %2d of %d ", player.masks_collected,
                        map->mask_count);

    mvwprintw(info_win, 3, 8, "moves: %d ",
                                    MAX_MOVES - player.moves_remaining);

    mvwprintw(info_win, 4, 8, "x:%2d  y:%2d  ",
                                    player.xmv[player.player].from_x,
                                    player.xmv[player.player].from_y);

    if (screen_data->scale_map == TRUE)
    {
        mvwprintw(info_win,
                  screen_data->map_tly + HMAP_H,
                  screen_data->map_tlx + 1, "m - map toggle");
    }

    wrefresh(info_win);

    debug("view_x0: %3d view_y0: %3d\n", map->view[0].x, map->view[0].y);
    debug("view_x1: %3d view_y1: %3d\n", map->view[1].x, map->view[1].y);

    debug("map player 0 x:%2d y:%2d\n", map->player[0].x, map->player[0].y);
    debug("map player 1 x:%2d y:%2d\n", map->player[1].x, map->player[1].y);
}
