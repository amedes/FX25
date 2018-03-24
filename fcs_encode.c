#include <stdio.h>
#include <stdint.h>

#include "fcs.h"
#include "afio.h"

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
  FILE *ifp = stdin;
  FILE *ofp = stdout;
  char buf[BUF_SIZE];
  int bit_length;
  int byte_size;
  uint32_t crc;

  while ((bit_length = afio_read(ifp, buf, BUF_SIZE - 2)) >= 0) {
    byte_size = (bit_length + 7) / 8;
    crc = fcs_calc(buf, byte_size);
    crc ^= 0xffff;
    buf[byte_size++] = crc & 0xff;
    buf[byte_size++] = crc >> 8;
    afio_write(ofp, buf, byte_size, 0);
  }

  return 0;
}
