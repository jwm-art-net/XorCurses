#include "map.h"
#include "icons.h"

#include <string.h>
#include <stdlib.h>

#include "options.h"
#include "debug.h"

struct xor_map *map = 0;

/*  xor_map_validate ensures that any objects that gravitate
    are suspended by *something*. fish and h-bombs should not
    hang in the air, and chickens and v-bombs should not be
    being held back by nothing, waiting to race left. returns
    0 for invalid maps, 1 for valid maps.

    xor_map_validate is called by xor_map_load, which will exit
    on invalid map encounters, ooh-err.
*/
su_t xor_map_validate();

void
xor_map_create()
{
    if (map)
        xor_map_destroy();
    if (!(map = malloc(sizeof(struct xor_map)))) {
        err_msg("Could not allocate map struct!\n");
        exit(1);
    }
    for (xy_t row = 0; row < MAP_H; row++) {
        if (!(map->buf[row] = malloc(sizeof(map_t) * (MAP_W + 1)))) {
            free(map);
            err_msg("Could not allocate map buffer!\n");
            exit(1);
        }
    }
    map->teleport[0].x = map->teleport[0].y = 0;
    map->teleport[1].x = map->teleport[1].y = 0;
    map->mask_count = 0;
}

void
xor_map_destroy()
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

char *
xor_map_read_name(FILE * fp)
{
    char tmpbuf[80] = { 0 };
    if (fgets(tmpbuf, 80, fp) != tmpbuf)
        return 0;
    char *cp = 0;

    if ((cp = strchr(tmpbuf, '\r')))
        *cp = 0;
    else if ((cp = strchr(tmpbuf, '\n')))
        *cp = 0;
    su_t l = strlen(tmpbuf);

    if (l >= MAPNAME_MAXCHARS)
        l = MAPNAME_MAXCHARS;
    char *ret = malloc(l + 1);

    if (!ret)
        return 0;
    strncpy(ret, tmpbuf, l);
    ret[l] = 0;
    return ret;
}

char *
xor_map_load_read_name(su_t level)
{
    char *fn = 0;

    if (!(fn = options_map_filename(level)))
        if (!(fn = options_map_filename(level)))
            return 0;
    FILE *fp = fopen(fn, "r");

    free(fn);
    if (!fp)
        return 0;
    char *ret = xor_map_read_name(fp);

    fclose(fp);
    return ret;
}

void
xor_map_load(su_t level)
{
    if (!(map->filename = options_map_filename(level)))
        xor_map_load_error(0, 0, "Map name or level error");
    FILE *map_fp = fopen(map->filename, "r");

    if (!map_fp)
        xor_map_load_error(map_fp, map->filename, "Can't find map file!");
    if (!(map->name = xor_map_read_name(map_fp)))
        xor_map_load_error(map_fp, map->filename,
                                                "Could not read map name.");
    char tmpbuf[80];

    su_t tele_count = 0;

    for (xy_t row = 0; row < MAP_H; row++) {
        if (!fgets(tmpbuf, 80, map_fp))
            xor_map_load_error(map_fp, map->filename,
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
                    xor_map_load_error(map_fp, map->filename,
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
            xor_map_load_error(map_fp, map->filename,
                               "Error loading initial player views");
        else {
            map->view[i].x = tmpx;
            map->view[i].y = tmpy;
        }
    }
    for (i = 0; i < 4; i++) {
        if (fscanf(map_fp, "%d %d", &tmpx, &tmpy) < 2)
            xor_map_load_error(map_fp, map->filename,
                               "Error loading map piece data");
        else {
            map->mappc[i].x = tmpx;
            map->mappc[i].y = tmpy;
        }
        if (map->buf[map->mappc[i].y][map->mappc[i].x] != ICON_MAP)
            xor_map_load_error(map_fp, map->filename,
                               "Mismatched map-piece location data");
    }
    if (tele_count == 1)
        xor_map_load_error(map_fp, map->filename,
                                            "Only one teleport in map!");
    if (tele_count) {           /* read teleport views */
        for (i = 0; i < 2; i++) {
            if (fscanf(map_fp, "%d %d", &tmpx, &tmpy) < 2)
                xor_map_load_error(map_fp, map->filename,
                                   "Error loading teleport-exit view-data");
            else {
                map->tpview[i].x = tmpx;
                map->tpview[i].y = tmpy;
            }
        }
    }
    if (!xor_map_validate())
        xor_map_load_error(map_fp, map->filename,
                           "contains unsupported object");
    fclose(map_fp);
    map->level = level;
}

su_t
xor_map_validate()
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

su_t
map_get_teleport(xy_t x, xy_t y)
{
    for (su_t i = 0; i < 2; i++)
        if (map->teleport[i].x == x && map->teleport[i].y == y)
            return i;
    return 99;
}

void
xor_map_load_error(FILE * fp, char *filename, char *msg)
{
    if (fp)
        fclose(fp);
    endwin();
    if (filename)
    {
        err_msg("Error in map!\n\tfile:%s\n\t%s\n", filename, msg);
    }
    else
    {
        err_msg("Error in map!\n\t%s\n", msg);
    }
    xor_map_destroy(map);
    exit(1);
}
