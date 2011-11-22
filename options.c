#include "options.h"

#include "map.h"
#include "debug.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const char *map_dir = "maps/";

const char *data_dir = DATADIR;

char *map_name[MAX_LEVEL + 1] = { 0 };

struct xor_options *options = 0;

su_t
options_create()
{
    if (!(options = malloc(sizeof(struct xor_options)))) {
        err_msg("Memory allocation error!\n");
        exit(1);
    }
    options->oldschool_play = 0;
    options->scroll_thresh = 2;
    options->good_opt_dir = 0;
    options->replay_speed = 4;
    options->replay_hyper = 0;
    options->replay_step = 0;
    options->dir_opt = 0;
    options->data_dir = 0;
    options->user_dir = 0;
    char *tmp = getenv("HOME");

    if (tmp) {
        if ((options->user_dir = malloc(strlen(tmp) + 1)))
            strcpy(options->user_dir, tmp);
    }
    if (options_set_dir_opt(DATA_INST_LOC))
        return 1;
    free(options->map_dir);
    options->map_dir = 0;
    if (options_set_dir_opt(DATA_CWD_LOC))
        return 1;
    options_destroy();
    return 0;
}

void
options_destroy()
{
    if (options) {
        if (options->map_dir)
            free(options->map_dir);
        if (options->user_dir)
            free(options->user_dir);
        free(options);
        options = 0;
    }
}

su_t
options_set_dir_opt(enum DATA_LOC loc)
{
    switch ((options->dir_opt = loc)) {
    case DATA_CWD_LOC:
        options->data_dir = 0;
        break;
    default:
        options->data_dir = data_dir;
        break;
    }
    if (!(options->map_dir = options_file_path(map_dir, options->data_dir)))
        return 0;
    FILE *fp = fopen(options->map_dir, "r");

    if (!fp)
        return 0;
    fclose(fp);
    return 1;
}

su_t
options_create_map_names()
{
    for (su_t i = MIN_LEVEL; i <= MAX_LEVEL; i++)
        if (!(map_name[i] = xor_map_load_read_name(i)))
            return 0;
    return 1;
}

void
options_destroy_map_names()
{
    for (su_t i = MIN_LEVEL; i <= MAX_LEVEL; i++)
        free(map_name[i]);
}

long
options_replay_speed(char n)
{
    if (options->replay_hyper)
        return 0L;
    if (options->replay_step)
        return (long) (200000000L / 5.0f );
    if (n < 1)
        n = 1;
    else if (n > 9) {
        if (n >= '1' && n <= '9')
            n = 9 - ('9' - n);
        else
            n = 9;
    }
    options->replay_speed = n;
    return (long) (200000000L / (float) n );
}

char *
options_map_filename(su_t level)
{
    if (level < MIN_LEVEL || level > MAX_LEVEL)
        return 0;
    char *dir = options->map_dir;

    su_t l = strlen(dir);

    l += ((level < 10 ? 5 : 6) + 1);
    char *fname = malloc(l);

    if (!fname)
        return 0;
    if (snprintf(fname, l, "%s%d.txt", dir, level) != l - 1) {
        free(fname);
        return 0;
    }
    return fname;
}

char *
options_file_path(const char *fname, const char *path)
{
    if (!fname)
        return 0;
    char *fp = 0;

    if (path) {
        su_t n = strlen(path);

        if (path[n] != '/')
            n++;
        if (!(fp = calloc(strlen(fname) + n + 1, sizeof(char))))
            return 0;
        strcpy(fp, path);
        if (fp[n - 1] != '/') {
            fp[n - 1] = '/';
            fp[n] = 0;
        }
    }
    else {
        if (!(fp = calloc(strlen(fname) + 1, sizeof(char))))
            return 0;
    }
    strcat(fp, fname);
    return fp;
}
