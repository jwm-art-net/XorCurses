#include "map.h"
#include "icons.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "fletcher.h"
#include "data_file.h"


int     map_load_txt(const char* filename);
void    map_write(void);

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        goto fail;
    }

    if (!xor_map_create()
     || !map_load_txt(argv[1]))
    {
        goto badfail;
    }

    map_write();

    xor_map_destroy();
    return 0;

fail:
    fprintf(stderr, "usage: mapconvert [FILE]\n");

badfail:
    xor_map_destroy();
    return -1;
}


void map_write(void)
{
    int i;
    struct df* df = df_open(stdout, DF_WRITE,
                                    XORCURSES_MAP_ID, 48);

    if (!df)
        return;

    df_write_string(df, map->name, 20);
    df_write_hex_word(df,  map->best_moves);

    for (i = 0; i < 2; ++i)
    {
        df_write_hex_byte(df, map->view[i].x);
        df_write_hex_byte(df, map->view[i].y);
    }

    for (i = 0; i < 4; ++i)
    {
        df_write_hex_byte(df, map->mappc[i].x);
        df_write_hex_byte(df, map->mappc[i].y);
    }

    for (i = 0; i < 2; ++i)
    {
        df_write_hex_byte(df, map->tpview[i].x);
        df_write_hex_byte(df, map->tpview[i].y);
    }

    for (i = 0; i < MAP_H; i += 2)
        df_write_hex_nibble_array(df, map->buf[i], MAP_W);

    for (i = 1; i < MAP_H; i += 2)
        df_write_hex_nibble_array(df, map->buf[i], MAP_W);

    df_close(df);
}


static int xor_map_load_error(FILE * fp, const char *filename, char *msg)
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

    return 0;
}


int map_load_txt(const char* filename)
{
    int best_moves;
    FILE *map_fp = fopen(filename, "r");

    if (!map_fp)
        return xor_map_load_error(map_fp, filename, "Can't find map file!");

    char tmpbuf[80];
    char* cp = tmpbuf;

    if (!fgets(tmpbuf, 80, map_fp))
        return xor_map_load_error(map_fp, filename,
                                            "failed to read map name");

    while(*cp >= ' ')
        ++cp;

    *cp = '\0';

    if (cp - tmpbuf > 20)
        return xor_map_load_error(map_fp, filename, "map name too long\n");

    map->name = malloc(strlen(tmpbuf) + 1);
    strcpy(map->name, tmpbuf);

    if (!fgets(tmpbuf, 80, map_fp))
        return xor_map_load_error(map_fp, filename,
                                        "failed to read map best moves");
    if (sscanf(tmpbuf, "%d", &best_moves) != 1)
        return xor_map_load_error(map_fp, filename,
                    "failed to read default best moves\n");

    map->best_moves = best_moves;
    su_t tele_count = 0;

    for (xy_t row = 0; row < MAP_H; row++) {
        if (!fgets(tmpbuf, 80, map_fp))
            return xor_map_load_error(map_fp, filename,
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
                    return xor_map_load_error(map_fp, filename,
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
            return xor_map_load_error(map_fp, filename,
                               "Error loading initial player views");
        else {
            map->view[i].x = tmpx;
            map->view[i].y = tmpy;
        }
    }
    for (i = 0; i < 4; i++) {
        if (fscanf(map_fp, "%d %d", &tmpx, &tmpy) < 2)
            return xor_map_load_error(map_fp, filename,
                               "Error loading map piece data");
        else {
            map->mappc[i].x = tmpx;
            map->mappc[i].y = tmpy;
        }
        if (map->buf[map->mappc[i].y][map->mappc[i].x] != ICON_MAP)
            return xor_map_load_error(map_fp, filename,
                               "Mismatched map-piece location data");
    }
    if (tele_count == 1)
        return xor_map_load_error(map_fp, filename,
                                            "Only one teleport in map!");
    if (tele_count) {           /* read teleport views */
        for (i = 0; i < 2; i++) {
            if (fscanf(map_fp, "%d %d", &tmpx, &tmpy) < 2)
                return xor_map_load_error(map_fp, filename,
                                   "Error loading teleport-exit view-data");
            else {
                map->tpview[i].x = tmpx;
                map->tpview[i].y = tmpy;
            }
        }
    }
    if (!xor_map_validate())
        return xor_map_load_error(map_fp, filename,
                           "contains unsupported object");
    fclose(map_fp);
    map->level = -1;

    return 1;
}


