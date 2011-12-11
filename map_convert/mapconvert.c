#include "map.h"
#include "icons.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "fletcher.h"
#include "data_file.h"


int map_load_txt_file(const char* filename);
void map_convert(void);
void map_convert_draft(void);
void test(void);

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        goto fail;
    }

    if (!xor_map_create()
     || !map_load_txt_file(argv[1]))
    {
        goto badfail;
    }

    map_convert();


    test();

    return 0;

fail:
    fprintf(stderr, "usage: mapconvert NN\n"
            "Where NN is the level number (0 - 15)\n");

badfail:
    return -1;


}

void test(void)
{
    const char* str = 
"hey there, it's me a gain. a gain involved here. not for me, nor you."
"no gain. just wait a second 2nd. how you may sleep reigning so very"
"soup preened sewage duct. anyway all that's beside the point. i'm"
"writing to you all again to let you know all about the great stuff the"
"great james 'shithead' morris has been up to as no doubt about it you"
"can't fucking wait to hear about my sad little poring over dribbly"
"drivel. poring over digital code binary data blahdy blahdy blahdy"
"blahdy. well as the other day on facebook i may have hinted about.. or"
"p'raps not. and yes this is about the poxy little 8bit game i've"
"rewritten for 64bit desktop pcs. using.. .cut to the chase james. ok"
"ok. ha yes i know my writing style is green-gauge ox flute. flux"
"perambulator. hey! twit! right yes. ho ho ho. grumpy bear flew towards"
"the seagull with a chair toothed grin. right. so 8 bit games yes,"
"memories oh yes. 8 bit code. BASIC. typing in code printed out in"
"magazines. especially machine code. well not machine code. but"
"hexadecimal data assembly language. typing lines of hex say 32"
"characters long from a magazine had certain pitfalls. therfor at the"
"end of each ln a usually f4urry digital beasty saat lurking to pounce"
"upon uncautious typists. the name of this furry beasty? checky checksum"
"was the name, don't wear it out. so to keep in the spirit of things."
"certainly not out of necessity. the spirit of things, the joy of"
"writing code and not giving a flying fuck about the fucking fact you're"
"swimming with joy in the sea of all those really evil bad fucking"
"shitty things programmers can do such as OMFG... re-invent the wheel"
"kill kill hang hang heresy heresy flog him to DEATH. DIE DIE DEADBEEF."
"so that's what i did and i'm kinda proud of it as usual ----- why else"
"would i post it here? what's the meaning? i mean, why? i'll tell ya why"
"oh god, yes, here's the great big looming why, why, well i'll why you,"
"why, you, it's because i have a little sense of achievement and want to"
"let you all know. there. but perhaps yes i'm overestimating the"
"interest it may have. just because i find it quite funky (not the right"
"word jimbob not the write wurzel). hommage to hoodies. huddlelums."
"gritty beer turned to the snivelling chair-toothed tinfoil hat wearing"
"prickly bum burned gooseberry and said 'i am dream. you are dreamer. we"
"are dreary. they are serious. you must not mock them fool. treat"
"them nicely don't be a tool.'...";

    struct df* df = df_open(stdout, DF_WRITE, "JWM-ART.NET__MSG:", 48);

    if (!df)
        return;

    df_write_string(df, "2012/12/11 01:27", 20);
    df_write_string(df, str, strlen(str) + 4);
    df_close(df);

}


int map_load_txt_file(const char* filename)
{
    FILE *map_fp = fopen(filename, "r");

    if (!map_fp)
        return xor_map_load_error(map_fp, filename, "Can't find map file!");

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


void map_convert(void)
{
    int i;
    struct df* df = df_open(stdout, DF_WRITE, XORCURSES_MAP_ID, 48);

    if (!df)
        return;

    df_write_string(df, map->name, 20);
    df_write_hex_word(df, (map->level < 6) ? 1000 : 2000);

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
