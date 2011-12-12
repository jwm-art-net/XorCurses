#ifndef _DATA_FILE_H
#define _DATA_FILE_H


#include <stdio.h>
#include <stdint.h>

/*  data structure and functions for reading and writing ascii/hex
    data with fixed line width, per line checksums, and a checksum
    of the checksums.

    7 words of warning: it is not designed to be friendly.
 */

enum DF_FLAGS
{
    DF_READ  =      0x0001,
    DF_WRITE =      0x0002,
    DF_RESERVED =   0xF000
};

enum DF_STR_FLAGS
{
    DF_RW_DEFAULT = 0x0000,  /* LTRIM on read, LPAD on write */
    DF_RSTR_NOTRIM =0x0010,  /* NO TRIM on read */
    DF_WSTR_RPAD =  0x0020   /* RPAD on write */
};


struct df
{
    char*   type_id;
    FILE*   fp;
    int     flags;

    size_t  width;          /* expected data width, excluding checksum */

    char    buf[80];        /* buffer holding current line */
    char*   cp;             /* points to current position in buf */
    char*   chksum;         /* points to position of checksum in buf */

    char    chkdata[256];   /* vertical checksum data */
    char*   chkcp;
};


/* unless otherwise stated, return value of zero indicates failure */


struct df*  df_open(FILE*, int flags, const char* type_id,
                                                    size_t data_width);

int         df_read_v_chksum(struct df*);
void*       df_close(struct df*); /* *always* returns 0 */

char*       df_read_string(struct df*, size_t length);
int         df_read_hex_byte(struct df*, uint8_t* result);
int         df_read_hex_word(struct df*, uint16_t* result);

int         df_read_hex_nibble_array(struct df*, uint8_t*, size_t count);

int         df_write_string(struct df*, const char*, size_t length);
int         df_write_hex_byte(struct df*, uint8_t);
int         df_write_hex_word(struct df*, uint16_t);

int         df_write_hex_nibble_array(struct df*, uint8_t*, size_t count);

#endif
