#include <stdio.h>
#include <stdint.h>
#include <strings.h>

#include "afio.h"


#define BUF_SIZE 1024
#define ITEM32_SIZE (BUF_SIZE*8+1)
#define TXD_DURATION (1000*1000 / BAUD_RATE) // 1/1200 usec

#define BIT_STUFF_LEN 5
#define NRZI_SIZE ((BUF_SIZE * 6 + 2) / 5) // need 6/5 times

#define FEND 0xc0
#define FESC 0xdb
#define TFEND 0xdc
#define TFESC 0xdd

int nrzi_byte(char nrzi[], int byte, int bt_flag)
{
  static int nrzi_bit;
  static int bit_offset;
  static int bit1_count;
  int i;
  int c;

  // inilialize variables
  if (nrzi == NULL) {
    nrzi_bit = 1;
    bit_offset = 0;
    bit1_count = 0;

    return 0;
  }

  c = byte;
  for (i = 0; i < 8; i++) {
    if ((c & 1) == 0) { // bit "0"
      nrzi_bit = !nrzi_bit; // invert bit value
      bit1_count = 0;
    } else { // bit "1"
      bit1_count++; // count number of "1"
    }
    c >>= 1; // shift next bit

    if (nrzi_bit) nrzi[bit_offset / 8] |= 1 << (bit_offset % 8); // set bit "1"
    bit_offset++;

    if (bit1_count >= BIT_STUFF_LEN) { // need bit stuffing
      if (bt_flag) {
	nrzi_bit = !nrzi_bit; // invert bit value
	if (nrzi_bit) nrzi[bit_offset / 8] |= 1 << (bit_offset % 8); // set bit "1"
	bit_offset++;
      }
      bit1_count = 0;
    }
  }

  //fprintf(stderr, "%x, %d, %d\n", byte, bt_flag, bit_offset);

  return bit_offset;
}

#define AX25_FLAG 0x7e

void nrzi_encode(FILE *fp, char buf[], int len)
{
  char nrzi[NRZI_SIZE];
  int byte_size;
  int bit_offset;
  int c;
  int i;
  int padding;

  // clear buffer
  bzero(nrzi, NRZI_SIZE);

  // initialize internal variables
  nrzi_byte(NULL, 0, 0);

  // packet start flag
  bit_offset = nrzi_byte(nrzi, AX25_FLAG, 0); // do not bit stuffing

  // add each byte with bit stuffing
  for (i = 0; i < len; i++) {
    bit_offset = nrzi_byte(nrzi, buf[i], 1);
  }

  // packet end flag
  bit_offset = nrzi_byte(nrzi, AX25_FLAG, 0); // do not bit stuffing

  /* output NRZI packet as SLIP frame */
  byte_size = (bit_offset + 7) / 8;
  padding = byte_size * 8 - bit_offset;
  afio_write(fp, nrzi, byte_size, padding);
}

#define STATE_INFRAME 1
#define STATE_OUTFRAME 2
#define STATE_ESCAPE 3

int main(int argc, char *argv[])
{
  FILE *ifp = stdin;
  FILE *ofp = stdout;
  char buf[BUF_SIZE];
  int c;
  int index;
  int state;
  int bit_length;

  while ((bit_length = afio_read(ifp, buf, BUF_SIZE)) >= 0) {
    nrzi_encode(ofp, buf, bit_length / 8);
  }
  return 0;
}
