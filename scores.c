#include "scores.h"
#include "options.h"
#include "player.h"
#include "fletcher.h"
#include "replay.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ctr_t *scores = 0;

static const char* SCORES_ID = "XorCurses__scores";

static void (*score_update_callback)(lvl_t level, ctr_t moves) = 0;

void
create_scores()
{
    if (scores)
        destroy_scores();
    if ((scores = malloc(sizeof(ctr_t) * (MAX_LEVEL + 1))))
        for (su_t n = 0; n <= MAX_LEVEL; n++)
            scores[n] = (n < 6 ? 1000 : 2000);
}

void
destroy_scores()
{
    if (scores)
        free(scores);
}

void
load_scores()
{
    if (!scores)
        return;
    if (!options->user_dir)
        return;
    char *fname = options_file_path(".xorcurses", options->user_dir);

    if (!fname)
        return;

    FILE *fp = fopen(fname, "r");

    free(fname);

    if (!fp) {
        write_scores();
        return;
    }

    char buf[80];
    char str[80];
    int i;

    if (!fgets(buf, 80, fp))
        return;

    sscanf(buf, "%s", str);

    if (strcmp(str, SCORES_ID) != 0)
    {
        debug("invalid scores file\n");
        return;
    }

    for (i = 1; i <= MAX_LEVEL; ++i)
    {
        uint8_t calc_chka = 0;
        uint8_t calc_chkb = 0;
        unsigned int read_chka = 0;
        unsigned int read_chkb = 0;
        int lvl = -1, score = -1;
        char* bp;

        if (!fgets(buf, 80, fp))
        {
            debug("failed to get level %d score\n", i);
            return;
        }

        buf[79] = '\0';

        bp = buf; /* trim newlines etc */
        while (*bp >= ' ')
            bp++;
        *bp = '\0';

        if (sscanf(buf, "%02x%02x", &read_chka, &read_chkb) != 2)
        {
            debug("failed to read level %d score checksum\n", i);
            return;
        }

        bp = buf + 4;

        fletcher16(&calc_chka, &calc_chkb, (uint8_t*)bp, strlen(bp));

        if (read_chka != calc_chka || read_chkb != calc_chkb)
        {
            debug("scores level %d checksum mismatch\n", i);
            debug("read:%02x%02x calc:%02x%02x\n",  read_chka, read_chkb,
                                                    calc_chka, calc_chkb);
            return;
        }

        if (sscanf(bp, "%02d%04d", &lvl, &score) != 2 || lvl != i)
        {
            debug("failed to read level %d == %d score\n", lvl, i);
            return;
        }
    }

    fclose(fp);
}

void
save_score(lvl_t level, ctr_t moves)
{
    if (!scores)
        return;
    if (level < MIN_LEVEL || level > MAX_LEVEL)
        return;
    if (moves > MAX_MOVES)
        return;
    if (!replay.hasexit)
        return;
    if (scores[level] > moves) {
        scores[level] = moves;
        write_scores();
        if (score_update_callback)
            (score_update_callback)(level, moves);
    }
}

void
write_scores()
{
    if (!scores)
        return;
    if (!options->user_dir)
        return;
    char *fname = options_file_path(".xorcurses", options->user_dir);

    if (!fname)
        return;
    FILE *fp = fopen(fname, "w");

    free(fname);
    if (!fp)
        return;

    int i;
    fprintf(fp, "%s\n",     SCORES_ID);

    for (i = 1; i <= MAX_LEVEL; ++i)
    {
        uint8_t chka = 0;
        uint8_t chkb = 0;
        char buf[80];
        snprintf(buf, 79, "%02d%04d%s", i, scores[i], map_name[i]);
        fletcher16(&chka, &chkb, (uint8_t*)buf, strlen(buf));
        fprintf(fp, "%02x%02x%s\n", chka, chkb, buf);
    }
    fclose(fp);
    return;
}

void
set_score_update_cb(void (*score_update_cb)(lvl_t level, ctr_t moves))
{
    score_update_callback = score_update_cb;
}
