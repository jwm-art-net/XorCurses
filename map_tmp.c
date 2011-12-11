#include "map_tmp.h"
#include "options.h"
#include "debug.h"
#include "map.h"

#include <stdlib.h>


char* xor_map_load_read_name(su_t level, ctr_t* best_moves)
{
    char *fn = 0;

    if (!(fn = options_map_filename(level)))
    {
        if (!(fn = options_map_filename(level)))
        {
            debug("options filename failure\n");
            return 0;
        }
    }

    debug("map %d filename: '%s'\n", level, fn);

    FILE *fp = fopen(fn, "r");

    if (!fp)
    {
        debug("failed to open map %d '%s'\n",level, fn);
        free(fn);
        return 0;
    }

    free(fn);

    char *ret = xor_map_read_name(fp, best_moves);

    debug("returning '%s'\n",ret);

    fclose(fp);
    return ret;
}


int xor_map_load(su_t level)
{
    if (!(map->filename = options_map_filename(level)))
        return xor_map_load_error(0, 0, "Map name or level error");

    if (!xor_map_load_file(0))
        return 0;

    map->level = level;
    return 1;
}
