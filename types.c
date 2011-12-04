#include "types.h"

#include "debug.h"



int read_int(FILE* fp, char* fmt, int* result)
{
    char buf[80];

    if (!fgets(buf, 80, fp))
        return -1;

    int r = sscanf(buf, (fmt != NULL) ? fmt : "%d", result);

    if (!r || r == EOF)
    {
        debug("%s: unknown\n", fmt);
        return -1;
    }

    debug("%s:%d\n", fmt, *result);
    return 0;
}
