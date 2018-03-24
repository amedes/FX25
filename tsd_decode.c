#include <stdio.h>
#include <stdint.h>
#include <strings.h>

#define BIT_TIME (80*1000*1000/1200) /* 80MHz / 1200baud */

#define TS_SIZE 1024
#define BUF_SIZE 4096

#define BIT_LEN_MAX 32

#define FEND  0xc0
#define FESC  0xdb
#define TFEND 0xdc
#define TFESC 0xdd

void write_packet(FILE *fp, uint8_t buf[], int bit_offset)
{
  int i;
  int size;
  int c;

  size = (bit_offset + 7) / 8;
  if (size == 0) return;

  putc(FEND, fp);

  for (i = 0; i < size; i++) {
    c = buf[i];
    switch (c) {
    case FEND:
      putc(FESC, fp);
      putc(TFEND, fp);
      break;
    case FESC:
      putc(FESC, fp);
      putc(TFESC, fp);
      break;
    default:
      putc(c, fp);
    }
  }

  putc(FEND, fp);
}

int main(int argc, char *argv[])
{
  FILE *ifp = stdin;
  FILE *ofp = stdout;
  uint32_t ts_data[TS_SIZE];
  char buf[BUF_SIZE];
  uint32_t ts0 = 0, ts;
  int nmemb;
  int bit_offset = 0;
  int bit_len;
  int level;
  int i;

  bzero(buf, BUF_SIZE);

  while ((nmemb = fread(ts_data, sizeof(uint32_t), TS_SIZE, ifp)) > 0) {
    for (i = 0; i < nmemb; i++) {
      ts = ts_data[i];
      bit_len = ((ts - ts0) + BIT_TIME/2) / BIT_TIME;
      if (bit_len == 0) continue;
      ts0 = ts;
      level = ts & 1;

      if (bit_len > BIT_LEN_MAX) {
	write_packet(ofp, buf, bit_offset);
	bit_offset = 0;
	bzero(buf, BUF_SIZE);
	continue;
      }

      while (bit_len-- > 0) {
	if (level) buf[bit_offset / 8] |= 1 << (bit_offset % 8);
	bit_offset++;
	if (bit_offset / 8 >= BUF_SIZE) {
	  write_packet(ofp, buf, bit_offset);
	  bit_offset = 0;
	  bzero(buf, BUF_SIZE);
	}
      }
    }
  }
  if (bit_offset > 0) {
    write_packet(ofp, buf, bit_offset);
  }

  return 0;
}
