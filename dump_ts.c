#include <stdio.h>
#include <stdint.h>

#include "modem.h"

#define TS_CLK (80*1000*1000) // 80MHz

int main(int argc, char *argv[])
{
  FILE *fp;
  uint32_t ts0 = 0, ts, tso = 0;
  uint32_t level;
  double f0 = 0, f;

  if (argc != 2) {
    fprintf(stderr, "usage: dump_ts ts_file\n");
    return 1;
  }

  fp = fopen(argv[1], "r");
  if (fp == NULL) {
    fprintf(stderr, "file can not open: %s\n", argv[1]);
    return 1;
  }

  //#define BAUD 1200

  while (fread(&ts, sizeof(ts), 1, fp) == 1) {
    if (tso == 0) {
      printf("0 %u\n", ts & 1);
      tso = ts;
      continue;
    }

    level = ts & 1;
    ts -= tso;
    f = (double)ts0 / (TS_CLK / BAUD_RATE);
    printf("%f %u\n", f, level);
    ts0 = ts;
    f = (double)ts0 / (TS_CLK / BAUD_RATE);
    if (f < f0) break; // wrap around

    printf("%f %u\n", f, level);
    f0 = f;
  }

  return 0;
}
