#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdio.h>

#define err_msg(...) \
{                   \
    fprintf(stderr, "%20s:%5d\t%30s", __FILE__, __LINE__, __FUNCTION__); \
    fprintf(stderr, ": ");          \
    fprintf(stderr, __VA_ARGS__);   \
}

#if DEBUG
# define debug(...) err_msg(__VA_ARGS__)
#else
# define debug(...)
#endif

#if DEBUG
#define ENUM_MSG(N, ENUM) \
    fprintf(stderr, #ENUM ": %s\n", (N == ENUM) ? "1" : "0");

#define EBIT_MSG(N, BIT) \
    fprintf(stderr, #BIT ": %s\n", (N & BIT) ? "1" : "0");
#endif


#endif
