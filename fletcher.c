/* http://en.wikipedia.org/wiki/Fletcher%27s_checksum
 */

#include "fletcher.h"


void fletcher16(uint8_t *checkA, uint8_t *checkB, uint8_t *data, size_t len)
{
    uint16_t sum1 = 0xff, sum2 = 0xff;

    while (len)
    {
        size_t tlen = len > 21 ? 21 : len;
        len -= tlen;

        do
        {
            sum1 += *data++;
            sum2 += sum1;
        } while (--tlen);

        sum1 = (sum1 & 0xff) + (sum1 >> 8);
        sum2 = (sum2 & 0xff) + (sum2 >> 8);
    }

    /* Second reduction step to reduce sums to 8 bits */
    sum1 = (sum1 & 0xff) + (sum1 >> 8);
    sum2 = (sum2 & 0xff) + (sum2 >> 8);
    *checkA = (uint8_t)sum1;
    *checkB = (uint8_t)sum2;
}

