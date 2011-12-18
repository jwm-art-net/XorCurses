#include "scores.h"
#include "options.h"
#include "player.h"
#include "fletcher.h"
#include "replay.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



enum { SCORES_FILE_LINE_WIDTH = 48 };

static const char* SCORES_ID = "XorCurses__scores";

ctr_t* scores = 0;
char** map_names = 0;

static void (*score_update_callback)(lvl_t level, ctr_t moves) = 0;


int create_scores()
{
    int i;

    if (scores)
        destroy_scores();

    scores = malloc(sizeof(*scores) * (MAX_LEVEL + 1));
    map_names = malloc(sizeof(*map_names) * (MAX_LEVEL + 1));

    if (!scores || !map_names)
        return 0;

    for (i = 1; i <= MAX_LEVEL; ++i)
    {
        char* fn = options_map_filename(i);

        if (!fn)
        {
            debug("failed to obtain map filename %d\n", i);
            return 0;
        }

        map_names[i] = xor_map_read_name(fn, &scores[i]);

        if (!map_names[i])
        {
            debug("failed to read map name %d\n", i);
            free(fn);
            return 0;
        }

        free(fn);
    }

    return 1;
}


void destroy_scores()
{
    int i;

    if (scores)
        free(scores);

    for (i = 1; i <= MAX_LEVEL; ++i)
        free(map_names[i]);

    free(map_names);
}


static int load_scores_df(struct df* df)
{
    int i;

    for (i = 1; i <= MAX_LEVEL; ++i)
    {
        uint8_t     level;
        uint16_t    best_moves;
        char*       map_name;

        if (!df_read_hex_byte(df, &level))
        {
            debug("failed to read level number %d from scores file\n", i);
            return 0;
        }

        if (!df_read_hex_word(df, &best_moves))
        {
            debug("failed to read best moves %d from scores file\n", i);
            return 0;
        }

        if (!(map_name = df_read_string(df, MAPNAME_MAXCHARS)))
        {
            debug("failed to read map name %d from scores file\n", i);
            return 0;
        }

        if (strcmp(map_name, map_names[i]) != 0)
        {
            debug("failed to match map name %d from scores file\n"
                  "with currently loaded maps\n", i);
            return 0;
        }

        free(map_name);

        scores[i] = best_moves;
    }

    return 1;
}


int load_scores()
{
    if (!scores)
        return 0;

    if (!options->user_dir)
        return 0;

    debug("loading scores file\n");

    char *fname = options_file_path(".xorcurses", options->user_dir);

    if (!fname)
        return 0;

    FILE *fp = fopen(fname, "r");

    free(fname);

    if (!fp)
    {
        save_scores();
        return 0;
    }

    struct df* df = df_open(fp, DF_READ, SCORES_ID,
                                         SCORES_FILE_LINE_WIDTH);

    if (!df)
    {
        ctr_t tmp[MAX_LEVEL + 1];
        int i;
        debug("failed to load scores file\n");
        debug("attempting import of old scores dump file\n");

        fseek(fp, 0L, SEEK_SET);
        fread(tmp, sizeof(ctr_t), MAX_LEVEL + 1, fp);

        /*  the old dump file based scores file didn't save the score
            for map 15.
         */

        for (i = 1; i < MAX_LEVEL; ++i)
        {
            if (tmp[i] <= 0 || tmp[i] > 2000)
            {
                debug("invalid score for map %d: %d \n", i, tmp[i]);
                i = 0;
                break;
            }
        }

        for (--i; i > 0; --i)
            scores[i] = tmp[i];

        fclose(fp);
        save_scores();
        return 0;
    }

    load_scores_df(df);
    df_close(df);
    fclose(fp);
    return 1;
}


void set_score(lvl_t level, ctr_t moves)
{
    if (!scores)
        return;

    if (level < MIN_LEVEL || level > MAX_LEVEL)
        return;

    if (moves > MAX_MOVES)
        return;

    if (!replay.hasexit)
        return;

    if (scores[level] > moves)
    {
        scores[level] = moves;
        save_scores();
        if (score_update_callback)
            (score_update_callback)(level, moves);
    }
}


int save_scores()
{
    if (!scores)
        return 0;

    if (!options->user_dir)
        return 0;

    debug("saving scores file\n");

    char *fname = options_file_path(".xorcurses", options->user_dir);

    if (!fname)
        return 0;

    FILE *fp = fopen(fname, "w");

    free(fname);

    if (!fp)
        return 0;

    struct df* df = df_open(fp, DF_WRITE, SCORES_ID,
                                          SCORES_FILE_LINE_WIDTH);

    if (!df)
    {
        debug("failed to write scores file\n");
        fclose(fp);
        return 0;
    }

    int i;
    int ret = 0;

    for (i = 1; i <= MAX_LEVEL; ++i)
    {
        uint8_t level = i & 0x0f;

        if (!df_write_hex_byte(df, level))
        {
            debug("failed to write level number %d to scores file\n", i);
            goto fail;
        }

        if (!df_write_hex_word(df, scores[i]))
        {
            debug("failed to write best moves %d to scores file\n", i);
            goto fail;
        }

        if (!df_write_string(df, map_names[i], MAPNAME_MAXCHARS))
        {
            debug("failed to write map name %d to scores file\n", i);
            goto fail;
        }
    }

    ret = 1;

fail:
    df_close(df);
    fclose(fp);
    return ret;
}


void set_score_update_cb(void (*score_update_cb)(lvl_t level, ctr_t moves))
{
    score_update_callback = score_update_cb;
}
