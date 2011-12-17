#include "map.h"
#include "icons.h"

#include <string.h>
#include <stdlib.h>

#include "debug.h"
#include "fletcher.h"


const char* XORCURSES_MAP_ID = "XorCurses__Map";


struct xor_map *map = 0;


int  xor_map_create(void)
{
    int i;

    if (map)
        xor_map_destroy();

    if (!(map = malloc(sizeof(struct xor_map))))
    {
        err_msg("Could not allocate map struct!\n");
        return 0;
    }

    for (xy_t row = 0; row < MAP_H; row++)
    {
        if (!(map->buf[row] = malloc(sizeof(map_t) * (MAP_W + 1))))
        {
            free(map);
            err_msg("Could not allocate map buffer!\n");
            return 0;
        }
    }

    map->name = 0;

    for (i = 0; i < 2; ++i)
    {
        map->player[i].x =    map->player[i].y = 0;
        map->view[i].x =      map->view[i].y = 0;
        map->teleport[i].x =  map->teleport[i].y = 0;
        map->tpview[i].x =    map->tpview[i].y = 0;
        map->mappc[i].x =     map->mappc[i].y = 0;
        map->mappc[i + 2].x = map->mappc[i + 2].y = 0;
    }

    map->level = -1;
    map->mask_count = 0;
    map->best_moves = 2000;

    return 1;
}


void xor_map_destroy(void)
{
    if (!map)
        return;

    if (map->name)
        free(map->name);

    for (xy_t row = 0; row < MAP_H; row++)
        free(map->buf[row]);

    free(map);
    map = 0;
}


int xor_map_load_by_datafile(struct df* df)
{
    if (!df)
        return 0;

    map->name = df_read_string(df, 20);

    if (!map->name)
    {
        debug("failed to read map name\n");
        return 0;
    }

    int i;
    uint16_t n2;

    if (!df_read_hex_word(df, &n2))
    {
        debug("failed to read default best move count\n");
        return 0;
    }

    map->best_moves = n2;

    for (i = 0; i < 2; ++i)
    {
        if (!df_read_hex_byte(df, &map->view[i].x)
         || !df_read_hex_byte(df, &map->view[i].y))
        {
            debug("failed to read default player%d view\n", i + 1);
            return 0;
        }
    }

    for (i = 0; i < 4; ++i)
    {
        if (!df_read_hex_byte(df, &map->mappc[i].x)
         || !df_read_hex_byte(df, &map->mappc[i].y))
        {
            debug("failed to read map%d position\n", i + 1);
            return 0;
        }
    }

    for (i = 0; i < 2; ++i)
    {
        if (!df_read_hex_byte(df, &map->tpview[i].x)
         || !df_read_hex_byte(df, &map->tpview[i].y))
        {
            debug("failed to read teleport%d data\n", i + 1);
            debug("whether it exists or not ;-)\n");
            return 0;
        }
    }

    for (i = 0; i < MAP_H; i += 2)
    {
        if (!df_read_hex_nibble_array(df, map->buf[i], MAP_W))
        {
            debug("failed to read map data row %d\n", i + 1);
            return 0;
        }
    }

    for (i = 1; i < MAP_H; i += 2)
    {
        if (!df_read_hex_nibble_array(df, map->buf[i], MAP_W))
        {
            debug("failed to read map data row %d\n", i + 1);
            return 0;
        }
    }

    if (!df_read_v_chksum(df, &map->chka, &map->chkb))
    {
        debug("fail!\n");
        return 0;
    }

    return xor_map_validate();
}


int xor_map_load_by_filename(const char* filename)
{
    FILE *fp;
    struct df* df = 0;

    if (!(fp = fopen(filename, "r")))
        return 0;

    if (!(df = df_open(fp, DF_READ, XORCURSES_MAP_ID, 48)))
        return 0;

    if (!xor_map_load_by_datafile(df))
        return 0;

    df_close(df);
    return 1;
}


char* xor_map_read_name(const char* filename, ctr_t* best_moves)
{
    FILE* fp;
    struct df* df;
    char* map_name;

    if (!(fp = fopen(filename, "r")))
        return 0;

    if (!(df = df_open(fp, DF_READ, XORCURSES_MAP_ID, 48)))
        return 0;

    map_name = df_read_string(df, MAPNAME_MAXCHARS);

    if (!map_name)
    {
        debug("failed to read map name\n");
        return 0;
    }

    if (!best_moves)
        return map_name;

    uint16_t n2;

    if (!df_read_hex_word(df, &n2))
    {
        debug("failed to read default score\n");
        return 0;
    }

    if (n2 <= 0 || n2 > 2000)
    {
        debug("invalid default score\n");
        n2 = 1000;
    }

    *best_moves = n2;

    return map_name;
}


int xor_map_validate(void)
{
    int players = 0;
    int teleports = 0;
    int maps = 0;
    int i;

    size_t len = strlen(map->name);

    if (!len || len > MAPNAME_MAXCHARS)
    {
        debug("invalid map name\n");
        return 0;
    }

    if (map->best_moves <= 0 || map->best_moves > 2000)
    {
        debug("invalid map default score\n");
        return 0;
    }

    for (xy_t y = 0; y < MAP_H - 1; y++)
    {
        for (xy_t x = 0; x < MAP_W - 1; x++)
        {
            if (x == 0 || x == MAP_W - 1
             || y == 0 || y == MAP_H - 1)
            {
                map->buf[y][x] = ICON_WALL;
            }

            su_t i = map->buf[y][x];

            switch (i)
            {
            case ICON_PLAYER0:
            case ICON_PLAYER1:
                if (players < 2)
                {
                    su_t p = i - ICON_PLAYER0;
                    map->player[p].x = x;
                    map->player[p].y = y;
                }

                players++;
                break;

            case ICON_MASK:
                map->mask_count++;
                break;

            case ICON_FISH:
            case ICON_H_BOMB:
                if (map->buf[y + 1][x] == ICON_SPACE
                 || map->buf[y + 1][x] == ICON_V_FIELD)
                {
                    debug("unsupported gravitating object at x, y %d, %d\n",
                                                                    x, y);
                    return 0;
                }
                break;

            case ICON_CHICKEN:
            case ICON_V_BOMB:
                if (map->buf[y][x - 1] == ICON_SPACE
                 || map->buf[y][x - 1] == ICON_H_FIELD)
                {
                    debug("unsupported gravitating object at x, y %d, %d\n",
                                                                    x, y);
                    return 0;
                }
                break;

            case ICON_TELEPORT:
                if (teleports < 2)
                {
                    map->teleport[teleports].x = x;
                    map->teleport[teleports].y = y;
                }

                teleports++;

                break;

            case ICON_MAP:
                maps++;

            default:
                break;
            }
        }
    }

    if (players != 2)
    {
        debug("invalid number of players %d\n", players);
        return 0;
    }

    if (teleports != 0 && teleports != 2)
    {
        debug("invalid number of teleports %d\n", teleports);
        return 0;
    }

    if (maps != 4)
    {
        debug("invalid number of map pieces %d\n", maps);
        return 0;
    }

    for (i = 0; i < 2; ++i)
    {
        struct xy* p = &map->player[i];
        struct xy* t = &map->teleport[i];
        struct xy* pv = &map->view[i];
        struct xy* tv = &map->tpview[i];

        if (p->x <= pv->x || p->x >= pv->x + 8
         || p->y <= pv->y || p->y >= pv->y + 8)
        {
            debug("invalid player %d view (%d, %d) and position "
                  "(%d, %d) combination", i + 1, pv->x, pv->y, p->x, p->y);
            return 0;
        }

        if (teleports && (t->x <= tv->x || t->x >= tv->x + 8
                       || t->y <= tv->y || t->y >= tv->y + 8))
        {
            debug("invalid teleport %d view (%d, %d) and position "
                  "(%d, %d) combination", i + 1, tv->x, tv->y, t->x, t->y);
            return 0;
        }
    }

    for (i = 0; i < 4; ++i)
    {
        struct xy* m = &map->mappc[i];

        if (map->buf[m->y][m->x] != ICON_MAP)
        {
            debug("invalid map-piece %d location (%d, %d)", i, m->x, m->y);
            return 0;
        }
    }

    debug("players:%d teleports:%d maps:%d\n", players, teleports, maps);

    return 1;
}


su_t map_get_teleport(xy_t x, xy_t y)
{
    for (su_t i = 0; i < 2; i++)
        if (map->teleport[i].x == x && map->teleport[i].y == y)
            return i;
    return 99;
}


su_t mapchar_to_icon(char c)
{
    switch (c) {
    case ' ':
        return ICON_SPACE;
    case '#':
        return ICON_WALL;
    case '-':
        return ICON_H_FIELD;
    case '|':
        return ICON_V_FIELD;
    case '@':
        return ICON_MASK;
    case '!':
        return ICON_FISH;
    case '<':
        return ICON_CHICKEN;
    case 'o':
        return ICON_H_BOMB;
    case 'x':
        return ICON_V_BOMB;
    case 'D':
        return ICON_DOLL;
    case 'S':
        return ICON_SWITCH;
    case 'M':
        return ICON_MAP;
    case 'E':
        return ICON_EXIT;
    case '+':
        return ICON_TELEPORT;
    case '1':
        return ICON_PLAYER0;
    case '2':
        return ICON_PLAYER1;
    }
    return ICON_SPACE;
}


char icon_to_mapchar(su_t icon)
{
    switch (icon) {
    case ICON_SPACE:
        return ' ';
    case ICON_WALL:
        return '#';
    case ICON_H_FIELD:
        return '-';
    case ICON_V_FIELD:
        return '|';
    case ICON_MASK:
        return '@';
    case ICON_FISH:
        return '!';
    case ICON_CHICKEN:
        return '<';
    case ICON_H_BOMB:
        return 'o';
    case ICON_V_BOMB:
        return 'x';
    case ICON_DOLL:
        return 'D';
    case ICON_SWITCH:
        return 'S';
    case ICON_MAP:
        return 'M';
    case ICON_EXIT:
        return 'E';
    case ICON_TELEPORT:
        return '+';
    case ICON_PLAYER0:
        return '1';
    case ICON_PLAYER1:
        return '2';
    }
    return 0;
}
