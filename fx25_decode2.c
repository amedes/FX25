#include <stdio.h>
#include <stdint.h>
#include <strings.h>
#include <string.h>

#include "modem.h"
#include "fx25tag.h"

#define STATE_SEARCH_TAG 1
#define STATE_DATA 2

#define BIT_TIME (80*1000*1000 / BAUD_RATE)
#define BIT_LEN_MAX 32

#define BUF_SIZE 1024
#define TAG_BYTE_LEN 8

#define FX25_CORRELATION_CNT 4

int fx25_search_tag(uint64_t *correlation_tag, int data_bit)
{
  uint64_t bits0, bits1;
  int count;
  int i;

  /* add 1 bit to tag data */
  *correlation_tag >>= 1;
  *correlation_tag |= ((uint64_t)data_bit << 63);

  /* compare tag with TAG_01 to TAG_0B */
  for (i = CO_TAG_01; i <= CO_TAG_0B; i++) {
    bits0 = *correlation_tag ^ tags[i].tag; // set bits if different 

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

int main(int argc, char *argv[])
{
  FILE *ifp = stdin;
  FILE *ofp = stdout;
  int state = STATE_SEARCH_TAG;
  uint32_t ts, ts0;
  int bit_len;
  uint8_t buf[BUF_SIZE];
  uint64_t fx25tag = 0;
  int tag_no;
  int codeblock_bits;
  int level;
  int bit_offset;
  int first = 1;

  fx25tag_init();

  while (fread(&ts, sizeof(ts), 1, ifp) == 1) {
    if (first) {
      ts0 = ts;
      first = 0;
      continue;
    }

    level = ts & 1;
    bit_len = (ts - ts0 + BIT_TIME/2) / BIT_TIME;
    ts0 = ts;

    while (bit_len > 0) {

      switch (state) {
      case STATE_SEARCH_TAG:
	if (bit_len > BIT_LEN_MAX) {
	  bit_len = 0;
	  continue;
	}
	if ((tag_no = fx25_search_tag(&fx25tag, level)) > 0) { // correlation tag found
	  state = STATE_DATA;
	  codeblock_bits = tags[tag_no].rs_code * 8; // FEC CODEBLOCK bit length
	  bzero(buf, BUF_SIZE);
	  memcpy(buf, &tags[tag_no].tag, TAG_BYTE_LEN);
	  bit_offset = TAG_BYTE_LEN * 8;
	}
	break;

      case STATE_DATA:
	if (level) buf[bit_offset / 8] |= 1 << (bit_offset % 8);
	bit_offset++;
	if (bit_offset >= codeblock_bits + TAG_BYTE_LEN * 8) {
	  afio_write(ofp, buf, bit_offset / 8, 0);
	  state = STATE_SEARCH_TAG;
	  fx25tag = 0;
	}
      }

      --bit_len;
    }
  }

  return 0;
}
