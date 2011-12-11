#include "data_file.h"

#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "fletcher.h"


enum {
    DF_CHKSUM = 0x8000
};


static int df_read_line(struct df*);
static int df_write_line(struct df*);
static int df_write_close(struct df*);


struct df* df_open(FILE* fp, int flags, const char* type_id,
                                                size_t data_width)
{
    size_t len = strlen(type_id);
    struct df* df;

    if (flags != DF_READ && flags != DF_WRITE)
    {
        debug("invalid flags %d type id '%s'\n", flags, type_id);
        return 0;
    }

    debug("opening file type id '%s'\n", type_id);

    df = malloc(sizeof(*df));

    if (!df)
    {
        debug("failed to allocate data for file '%s'\n", type_id);
        fclose(fp);
        return 0;
    }

    df->type_id = malloc(strlen(type_id) + 1);
    strcpy(df->type_id, type_id);
    df->fp = fp;
    df->width = data_width;
    df->chkcp = df->chkdata;
    *df->chkdata = '\0';
    df->cp = df->buf;
    df->flags = flags;

    if (flags == DF_READ)
    {
        if (!df_read_line(df))
            return df_close(df);

        if (strncmp(df->buf, type_id, len) != 0)
        {
            debug("file failed type id '%s' verification\n", type_id);
            return df_close(df);
        }

        df->cp = df->buf + len;
    }
    else
    {
        if (!df_write_string(df, type_id, strlen(type_id)))
        {
            debug("failed to save type id data\n");
            return df_close(df);
        }
    }

    return df;
}


void* df_close(struct df* df)
{
    if (!df)
        return 0;

    debug("closing file type id '%s'\n", df->type_id);

    if (df->flags & DF_WRITE)
        df_write_close(df);

    free(df->type_id);
    free(df);

    return 0;
}


static int df_read_line(struct df* df)
{
    uint8_t calc_chka = 0;
    uint8_t calc_chkb = 0;
    uint8_t read_chka = 0;
    uint8_t read_chkb = 0;
    char* chksum = df->buf + df->width;

    if (!(df->flags & DF_READ))
        return 0;

    if (!fgets(df->buf, 80, df->fp))
    {
        debug("failed to read line in '%s'\n", df->type_id);
        return 0;
    }

    df->buf[79] = '\0';
    df->cp = df->buf;

    while (*df->cp >= ' ')
        ++df->cp;

    *df->cp = '\0';
    df->cp = df->buf;

    if (strlen(df->buf) - 4 != df->width)
        return 0;

    df->flags |= DF_CHKSUM;
    df->cp = chksum;

    if (!df_read_hex_byte(df, &read_chka)
     || !df_read_hex_byte(df, &read_chkb))
    {
        debug("failed to read line checksum in '%s'\n", df->type_id);
        return 0;
    }

    fletcher16(&calc_chka, &calc_chkb, (uint8_t*)df->buf, df->width);

    if (read_chka != calc_chka || read_chkb != calc_chkb)
    {
        debug("checksum mismatch in file '%s'\n", df->type_id);
        return 0;
    }

    strncpy(df->chkcp, chksum, 4);
    *chksum = '\0';

    df->flags = DF_READ;

    return 1;
}


char* df_read_string(struct df* df, size_t length)
{
    if (!(df->flags & DF_READ))
        return 0;

    char* ret = malloc(length + 1);
    char* r = ret;
    unsigned int c = 0;

    if (!ret)
    {
        debug("failed to read string of %zd\n", length);
        return 0;
    }

    for (c = 0; c < length && *df->cp == ' '; ++c)
        ++df->cp;

    /* in case string only contains space characters */
    *r = '\0';

    while (c < length)
    {
        if (*df->cp == '\0' && !df_read_line(df))
            return 0;

        while (c++ < length && *df->cp)
            *r++ = *df->cp++;
    }

    *r = '\0';
    return ret;
}


int df_read_hex_byte(struct df* df, uint8_t* result)
{
    if (!(df->flags & DF_READ))
        return 0;

    unsigned int i, c, r;

    for (c = r = 0; c < 2; ++c)
    {
        if (*df->cp == '\0')
            if (df->flags & DF_CHKSUM || !df_read_line(df))
                return 0;

        if (*df->cp >= '0' && *df->cp <= '9')
            i = *df->cp - '0';
        else if (*df->cp >= 'a' && *df->cp <= 'f')
            i = 10 + *df->cp - 'a';
        else
            return 0;

        r = (r << 4) | i;
        df->cp++;
    }

    *result = r;
    return 1;
}


int df_read_hex_word(struct df* df, uint16_t* result)
{
    if (!(df->flags & DF_READ))
        return 0;

    unsigned int i, c, r;

    for (c = r = 0; c < 4; ++c)
    {
        if (*df->cp == '\0')
            if (df->flags & DF_CHKSUM || !df_read_line(df))
                return 0;

        if (*df->cp >= '0' && *df->cp <= '9')
            i = *df->cp - '0';
        else if (*df->cp >= 'a' && *df->cp <= 'f')
            i = 10 + *df->cp - 'a';
        else
            return 0;

        r = (r << 4) | i;
        df->cp++;
    }

    *result = r;
    return 1;
}


static int df_write_close(struct df* df)
{
    if (!(df->flags & DF_WRITE))
        return 0;

    debug("writing file type id '%s'\n", df->type_id);

    char* chksum = df->buf + df->width;
    uint8_t chka = 0;
    uint8_t chkb = 0;

    if (chksum - df->cp < 4)
    {
        debug("no room for v-checksum in line\n");
        return 0;
    }

    debug("chkdata: '%s'\n", df->chkdata);

    fletcher16(&chka, &chkb, (uint8_t*)df->chkdata, strlen(df->chkdata));
    df_write_hex_byte(df, chka);
    df_write_hex_byte(df, chkb);

    while(df->cp < chksum)
        *df->cp++ = '0';

    *chksum = '\0';
    df_write_line(df);
    return 1;
}


static int df_write_line(struct df* df)
{
    if (!(df->flags & DF_WRITE))
        return 0;

    uint8_t chka = 0;
    uint8_t chkb = 0;
    char* chksum = df->buf + df->width;

    debug("writing... buf:'%s'\n", df->buf);

    if (df->cp != chksum)
    {
        debug("cp not at end\n"); debug("'%s'\n",df->cp);
        return 0;
    }

    if (strlen(df->buf) != df->width)
    {
        debug("buf fill not of width\n");
        return 0;
    }

    fletcher16(&chka, &chkb, (uint8_t*)df->buf, df->width);
    snprintf(chksum, 5, "%02x%02x", chka, chkb);
    df->chkcp += snprintf(df->chkcp, 5, "%s", chksum);
    fprintf(df->fp, "%s\n", df->buf);
    df->cp = df->buf;

    return 1;
}


int df_write_string(struct df* df, const char* str, size_t length)
{
    if (!(df->flags & DF_WRITE))
    {
        debug("not for writing\n");
        return 0;
    }

    debug("writing string...'%s'\n",str);

    char* chksum = df->buf + df->width;
    size_t slen = strlen(str);

    while (length)
    {
        if (df->cp == chksum)
        {
            *chksum = '\0';

            if (!df_write_line(df))
                return 0;
        }

        if (length > slen)
            *df->cp++ = ' ';
        else
        {
            *df->cp++ = *str++;
            --slen;
        }

        --length;
    }

    return 1;
}


int df_write_hex_byte(struct df* df, uint8_t n)
{
    if (!(df->flags & DF_WRITE))
        return 0;

    char* chksum = df->buf + df->width;
    unsigned int c;

    for (c = 0; c < 2; ++c)
    {
        if (df->cp == chksum)
        {
            *chksum = '\0';

            if (!df_write_line(df))
                return 0;
        }

        unsigned int d = (n & 0xf0) >> 4;

        if (d < 10)
            *df->cp++ = '0' + d;
        else if (d <= 15)
            *df->cp++ = 'a' + (d - 10);
        else
            return 0;

        n = n << 4;
    }

    return 1;
}


int df_write_hex_word(struct df* df, uint16_t n)
{
    if (!(df->flags & DF_WRITE))
        return 0;

    char* chksum = df->buf + df->width;
    unsigned int c;

    for (c = 0; c < 4; ++c)
    {
        if (df->cp == chksum)
        {
            *chksum = '\0';

            if (!df_write_line(df))
                return 0;
        }

        unsigned int d = (n & 0xf000) >> 12;

        if (d < 10)
            *df->cp++ = '0' + d;
        else if (d <= 15)
            *df->cp++ = 'a' + (d - 10);
        else
            return 0;

        n = n << 4;
    }

    return 1;
}


int df_write_hex_nibble_array(struct df* df, uint8_t* data, size_t count)
{
    if (!(df->flags & DF_WRITE))
        return 0;

    char* chksum = df->buf + df->width;

    while(count--)
    {
        unsigned int n = *data++ & 0x0f;

        if (df->cp == chksum)
        {
            *chksum = '\0';

            if (!df_write_line(df))
                return 0;
        }

        if (n < 10)
            *df->cp++ = '0' + n;
        else if (n <= 15)
            *df->cp++ = 'a' + (n - 10);
        else
            return 0;
    }

    return 1;
}

