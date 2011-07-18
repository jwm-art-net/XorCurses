#include "scores.h"
#include "options.h"
#include "player.h"

#include <stdio.h>
#include <stdlib.h>

ctr_t *scores = 0;

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
    fread(scores, sizeof(ctr_t), MAX_LEVEL, fp);
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
    fwrite(scores, sizeof(ctr_t), MAX_LEVEL, fp);
    fclose(fp);
    return;
}

void
set_score_update_cb(void (*score_update_cb)(lvl_t level, ctr_t moves))
{
    score_update_callback = score_update_cb;
}
