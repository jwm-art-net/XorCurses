#include "map.h"
#include "icons.h"

#include <string.h>
#include <stdlib.h>

#include "debug.h"
#include "fletcher.h"


const char* XORCURSES_MAP_ID = "XorCurses__map";


struct xor_map *map = 0;

/*  xor_map_validate ensures that any objects that gravitate
    are suspended by *something*. fish and h-bombs should not
    hang in the air, and chickens and v-bombs should not be
    being held back by nothing, waiting to race left. returns
    0 for invalid maps, 1 for valid maps.

    xor_map_validate is called by xor_map_load, which will exit
    on invalid map encounters, ooh-err.
*/
su_t xor_map_validate(void);

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

    map->filename = 0;
    map->name = 0;

    for (i = 0; i < 2; ++i)
    {
        map->player[i].x = map->player[i].y = 0;
        map->view[i].x = map->view[i].y = 0;
        map->teleport[i].x = map->teleport[i].y = 0;
        map->tpview[i].x = map->tpview[i].y = 0;
    }

    for (i = 0; i < 4; ++i)
    {
        map->mappc[i].x = map->mappc[i].y = 0;
    }

    map->level = -1;
    map->mask_count = 0;

    return 1;
}


void xor_map_destroy(void)
{
    if (map) {
        if (map->filename)
            free(map->filename);
        if (map->name)
            free(map->name);
        for (xy_t row = 0; row < MAP_H; row++)
            free(map->buf[row]);
        free(map);
        map = 0;
    }
}


FILE* xor_map_open(const char* filename)
{
}

char* xor_map_read_name(FILE * fp, ctr_t* best_moves)
{
    char buf[80];
    char* cp;
    char tmp[80];
    int i;
    char* ret = 0;

    if (fgets(buf, 80, fp) != buf)
    {
        debug("failed to read data line in file\n");
        return 0;
    }

    buf[79] = '\0';
    cp = buf;
    while (*cp >= ' ')
        ++cp;
    *cp = '\0';

    strncpy(tmp, buf, 20);
    tmp[20] = '\0';
    cp = tmp;

    while (*cp == ' ')
        ++cp;

    size_t l = strlen(cp);

    if (!(ret = malloc(l + 1)))
    {
        debug("failed to allocate %ld bytes for map name\n", (long)l);
        return 0;
    }

    strncpy(ret, cp, l);
    ret[l] = '\0';

    if (!best_moves)
        return ret;

    cp = buf + 20;

    if (sscanf(cp, "%04d", &i) != 1)
    {
        debug("failed to read best score.\n");
        free(ret);
        return 0;
    }

    *best_moves = i;

    return ret;
}


int xor_map_load_file(const char* filename)
{
    FILE *map_fp = fopen(((filename) ? filename : map->filename), "r");

    if (!map_fp)
        return xor_map_load_error(map_fp, map->filename,
                                          "Can't find map file!");

    if (!(map->name = xor_map_read_name(map_fp, 0)))
        return xor_map_load_error(map_fp, map->filename,
                                                "Could not read map name.");
    char tmpbuf[80];

    su_t tele_count = 0;

    for (xy_t row = 0; row < MAP_H; row++) {
        if (!fgets(tmpbuf, 80, map_fp))
            return xor_map_load_error(map_fp, map->filename,
                                                    "Error in map layout");
        for (xy_t col = 0; col < MAP_W; col++) {
            su_t i = tmpbuf[col];

            switch (i) {
            case '1':
            case '2':
                map->player[i - '1'].x = col;
                map->player[i - '1'].y = row;
                break;
            case '@':
                map->mask_count++;
                break;
            case '+':
                if (tele_count < 2) {
                    map->teleport[tele_count].x = col;
                    map->teleport[tele_count].y = row;
                    tele_count++;
                }
                else
                    return xor_map_load_error(map_fp, map->filename,
                                       "Too many teleports in map!");
                break;
            }
            if (col == 0 || col == MAP_W - 1
             || row == 0 || row == MAP_H - 1)
                map->buf[row][col] = ICON_WALL;
            else
                map->buf[row][col] = mapchar_to_icon(i);
        }
    }
    int tmpx, tmpy;             /* for reading %d if xy_t is char */

    su_t i = 0;

    for (i = 0; i < 2; i++) {
        if (fscanf(map_fp, "%d %d", &tmpx, &tmpy) < 2)
            return xor_map_load_error(map_fp, map->filename,
                               "Error loading initial player views");
        else {
            map->view[i].x = tmpx;
            map->view[i].y = tmpy;
        }
    }
    for (i = 0; i < 4; i++) {
        if (fscanf(map_fp, "%d %d", &tmpx, &tmpy) < 2)
            return xor_map_load_error(map_fp, map->filename,
                               "Error loading map piece data");
        else {
            map->mappc[i].x = tmpx;
            map->mappc[i].y = tmpy;
        }
        if (map->buf[map->mappc[i].y][map->mappc[i].x] != ICON_MAP)
            return xor_map_load_error(map_fp, map->filename,
                               "Mismatched map-piece location data");
    }
    if (tele_count == 1)
        return xor_map_load_error(map_fp, map->filename,
                                            "Only one teleport in map!");
    if (tele_count) {           /* read teleport views */
        for (i = 0; i < 2; i++) {
            if (fscanf(map_fp, "%d %d", &tmpx, &tmpy) < 2)
                return xor_map_load_error(map_fp, map->filename,
                                   "Error loading teleport-exit view-data");
            else {
                map->tpview[i].x = tmpx;
                map->tpview[i].y = tmpy;
            }
        }
    }
    if (!xor_map_validate())
        return xor_map_load_error(map_fp, map->filename,
                           "contains unsupported object");
    fclose(map_fp);
    map->level = -1;

    return 1;
}


su_t xor_map_validate()
{
    su_t ret = 1;

    for (xy_t y = 1; y < MAP_H - 2; y++) {
        for (xy_t x = 1; x < MAP_W - 2; x++) {
            switch (map->buf[y][x]) {
            case ICON_FISH:
            case ICON_H_BOMB:
                if (map->buf[y + 1][x] == ICON_SPACE
                 || map->buf[y + 1][x] == ICON_V_FIELD)
                    ret = 0;
                break;
            case ICON_CHICKEN:
            case ICON_V_BOMB:
                if (map->buf[y][x - 1] == ICON_SPACE
                 || map->buf[y][x - 1] == ICON_H_FIELD)
                    ret = 0;
                break;
            default:
                break;
            }
        }
    }
    return ret;
}


su_t map_get_teleport(xy_t x, xy_t y)
{
    for (su_t i = 0; i < 2; i++)
        if (map->teleport[i].x == x && map->teleport[i].y == y)
            return i;
    return 99;
}


int  xor_map_load_error(FILE * fp, const char *filename, char *msg)
{
    if (fp)
        fclose(fp);

    if (filename)
    {
        debug("Error in map!\n\tfile:%s\n\t%s\n", filename, msg);
    }
    else
    {
        debug("Error in map!\n\t%s\n", msg);
    }

    xor_map_destroy();
    return 0;
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
