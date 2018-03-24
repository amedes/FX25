#include <stdio.h>

#include "ax25.h"

#define CRC16_POLY 0x10811 /* G(x) = 1 + x^5 + x^12 + x^16 */

int ax25_fcs(uint8_t packet[], int length)
{
    uint32_t crc;
    int i, j;

    if (length <= 0) return -1; // packet too short

    // calculate CRC x^16 + x^12 + x^5 + 1
    crc = 0xffff; /* initial value */
    for (i = 0; i < length; i++) {
	crc ^= packet[i];
	for (j = 0; j < 8; j++) {
	    if (crc & 1) crc ^= CRC16_POLY;
	    crc >>= 1;
	}
    }
    crc ^= 0xffff; // invert

    return crc;
}

int ax25_count_bit_length(uint8_t packet[], int length)
{
  int bit_offset = 0;
  int bit_length;
  int data_bit;
  int bit1_len;

  bit_length = 0;
  bit1_len = 0;
  for (bit_offset = 0; bit_offset < length * 8; bit_offset++) {
    data_bit = (packet[bit_offset / 8] >> bit_offset % 8) & 1;
    bit_length++;

    /* count countinuous '1' */
    if (data_bit) { // '1'
      if (++bit1_len >= 5) { // need bit stuffing
	bit_length++;
	bit1_len = 0;
      }
    } else { // '0'
      bit1_len = 0;
    }
  }

  return bit_length;
}
