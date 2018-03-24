#include <stdint.h>

#include "fcs.h"

uint32_t fcs_calc(uint8_t packet[], int length)
{
    uint32_t crc;
    int i, j;

    // calculate CRC x^16 + x^12 + x^5 + 1
    crc = 0xffff; /* initial value */
    for (i = 0; i < length; i++) {
	crc ^= packet[i];
	for (j = 0; j < 8; j++) {
	    if (crc & 1) crc ^= GENERATOR_POLY;
	    crc >>= 1;
	}
    }

    return crc;
}
