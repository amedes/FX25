#include <stdio.h>
#include <stdint.h>
#include <strings.h>

#include "afio.h"

#define BIT_STUFF_LEN 5 // 5 continuos "1" needs delete next "0"
#define BUF_SIZE 1024

#define STATE_SEARCH_FLAG 1
#define STATE_DATA 2

int main(int argc, char *argv[])
{
  FILE *ifp = stdin;
  FILE *ofp = stdout;
  uint8_t nrzi[BUF_SIZE];
  uint8_t buf[BUF_SIZE];
  int level, level0;
  int bit_offset;
  int bit_stuff;
  int delete_zero;
  int bit_len;
  int byte_size;
  int padding;
  int state;
  uint8_t flag;
  int first;
  int bit_length;
  int i;
  int flag_found;
  int bit;

  while ((bit_length = afio_read(ifp, nrzi, BUF_SIZE)) >= 0) {

    state = STATE_SEARCH_FLAG;
    delete_zero = 0;
    bit_stuff = 0;
    bit_offset = 0;
    first = 1;
    flag = 0;

    for (i = 0; i < bit_length; i++) {
      level = (nrzi[i / 8] >> (i % 8)) & 1; // extract one bit value, 0 or 1

      // search AX.25 flag
      flag >>= 1;
      flag |= level << 7; // insert MSb
      if (flag == 0x80 || flag == 0x7f) { // NRZI flag values
	flag_found = 1;
      } else {
	flag_found = 0;
      }

      // assume level changed at begining of packet
      if (first) {
	level0 = !level;
	first = 0;
      }

      // bit value
      if (level != level0) {
	bit = 0;
	bit_len = 0;
      } else {
	bit = 1;
	bit_len++;
      }
      level0 = level;

      switch (state) {
      case STATE_SEARCH_FLAG:
	if (flag_found) {
	  state = STATE_DATA;
	  bit_stuff = 0;
	  bit_offset = 0;
	  bzero(buf, BUF_SIZE);
	}
	break;

      case STATE_DATA:
	if (flag_found) { // AX.25 packet end

	  bit_offset -= 7; // delete flag

	  byte_size = (bit_offset + 7) / 8;
	  padding = byte_size * 8 - bit_offset;
	  if (byte_size > 0) {
	    afio_write(ofp, buf, byte_size, padding);
	  }

	  bit_stuff = 0;
	  bit_offset = 0;
	  bzero(buf, BUF_SIZE);

	  state = STATE_SEARCH_FLAG;
	  continue;
	}
    
	if (bit_len >= BIT_STUFF_LEN) {
	  bit_stuff = 1;
	  bit_len = 0;
	}

	// de-NRZI
	if (delete_zero && bit == 0) {
	  // do not add "0"
	} else {
	  buf[bit_offset / 8] |= bit << (bit_offset % 8);
	  bit_offset++;
	}
	delete_zero = 0; 

	if (bit_stuff) {
	  delete_zero = 1; // delete next "0"
	  bit_stuff = 0;
	}
      }
    }
  }

  return 0;
}
