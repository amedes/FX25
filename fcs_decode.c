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
  int crc_err = 0;
  FILE *efp;

  efp = fopen("fcs_decode_err.af", "w");

  while ((bit_length = afio_read(ifp, buf, BUF_SIZE)) >= 0) {
    byte_size = (bit_length + 7) / 8;
    if (byte_size < 3) continue;

    crc = fcs_calc(buf, byte_size);
    if (crc != GOOD_CRC) {
      fprintf(stderr, "fcs_decode: CRC check error\n");
      afio_write(efp, buf, byte_size, 0);
      fflush(efp);
      crc_err++;
      continue;
    }
    afio_write(ofp, buf, byte_size - 2, 0);
  }

  fclose(efp);
  fprintf(stderr, "fcs_decode: %d crc errors\n", crc_err);

  return 0;
}
