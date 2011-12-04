#ifndef _FLETCHER_H
#define _FLETCHER_H

/* http://en.wikipedia.org/wiki/Fletcher%27s_checksum
 */

#include <stdint.h>
#include <stddef.h>


void fletcher16(uint8_t *checkA, uint8_t *checkB, uint8_t *data,
                                                   size_t len);


#endif
