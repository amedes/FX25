/*
  routine for FX.25 packet
*/
#include <stdio.h>
#include <stdint.h>
#include <strings.h>

#include "fx25.h"
#include "ax25.h"
#include "tag.h"

#define FX25_CORRELATION_CNT 4

static char test_data[] = {
  0x00, 0x00, 0x00, 0x00, // NRZI 0 1 pattern
  0xaa, 0xaa, 0xaa, 0xaa, // NRZI 00 11 pattern
  0xb6, 0x6d, 0xdb, 0xb6, // NRZI 000 111 pattern
  0xee, 0xee, 0xee, 0xee, // NRZI 0000 1111 pattern
  0xde, 0x7b, 0xef, 0xbd, // NRZI 00000 11111 pattern
  0xf7, 0xff, 0xff, 0xff, // NRZI 000000 111111 pattern
};

int fx25_make_random_packet(uint8_t *buf, int buf_size)
{
  int fx25_info_size;
  int fcs;
  int i;
  static int count = 0;
  int bit_length;

  //printf("buf_size: %d\n", buf_size);

  // check packet size
  if (buf_size < 3 || buf_size > FX25_MAX_INFO) return -1;

  snprintf(&buf[0], 7, "%06d", count++); // set packet count for check at address part
  for (i = 0; i < 6; i++) buf[i] <<= 1;

  // make random data
  fx25_info_size = buf_size - 2; // 2 for FCS bytes
  for (i = 16; i < fx25_info_size; i++) {
#if 0
    buf[i] = rand() & 0xff;
#else
    buf[i] = test_data[i % (sizeof(test_data) - 1)];
#endif
  }

  while (1) {
    // calculate FCS
    fcs = ax25_fcs(buf, fx25_info_size); // FCS is bit reversed
    buf[fx25_info_size++] = fcs & 0xff;
    buf[fx25_info_size++] = (fcs >> 8);
  
    bit_length = ax25_count_bit_length(buf, fx25_info_size);
    if (bit_length <= buf_size * 8) break; // fit in buffer

    fx25_info_size -= 3; // decrement info size
  }
  printf("fx25_info_szie: %d\n", fx25_info_size);

  return fx25_info_size;
}


int fx25_search_tag(uint64_t *correlation_tag, int data_bit)
{
  //static const uint64_t tag_01 = FX25_TAG_01;
  //static uint64_t tag_bits = 0;
  uint64_t bits0, bits1;
  int count;
  int i;

  //printf("data_bit: %d\n", data_bit);

  //printf("FX25_TAG_01: %016llx\n", tag_01);

  /* add 1 bit to tag data */
#if 1
  *correlation_tag <<= 1;
  *correlation_tag |= data_bit;
#else
  *correlation_tag >>= 1;
  *correlation_tag |= ((uint64_t)data_bit << 63);
#endif
  //printf("tag_bits: %016llx\n", tag_bits);

  /* compare tag with TAG_01 to TAG_0B */
  for (i = CO_TAG_01; i <= CO_TAG_0B; i++) {
    bits0 = *correlation_tag ^ co_tag[i];

    /* count number of bit '1' after XOR with TAG_XX */
    bits1 = (bits0 & 0xaaaaaaaaaaaaaaaaLLU) >> 1;
    bits0 &=         0x5555555555555555LLU;

    bits0 += bits1; /* 01 + 01 = 10 */

    bits1 = (bits0 & 0xccccccccccccccccLLU) >> 2;
    bits0 &=         0x3333333333333333LLU;

    bits0 += bits1; /* 0010 + 0010 = 0100 */

    bits1 = (bits0 >> 32); /* higher 32bit */
    bits0 &= 0x07070707; /* lower 32bit */

    bits0 += bits1; /* 0100 + 0100 = 1000 */

    bits1 = (bits0 & 0xf0f0f0f0) >> 4; /* separate each 4bit */
    bits0 &=         0x0f0f0f0f;

    bits0 += bits1; /* 0x08 + 0x08 = 0x10 */

    bits1 = (bits0 >> 16); /* higher 16bit */
    bits0 &= 0x1f1f; 	   /* lower 16bit */

    bits0 += bits1; /* 0x10 + 0x10 = 0x20 */

    bits1 = (bits0 >> 8); /* higher 8bit */
    bits0 &= 0x3f;	/* lower 8bit */

    bits0 += bits1; /* 0x20 + 0x20 = 0x40 */
    count = bits0; /* bit count */

    if (count <= FX25_CORRELATION_CNT) {
      return i; // find ith TAG
    }
  }

  return -1; // not found tag
}
