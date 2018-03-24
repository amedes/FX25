#include <stdio.h>
#include <stdint.h>

#include "afio.h"

#define PACKET_SIZE 197 // maximum data size before NRZI
#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
  FILE *ifp = stdin;
  FILE *ofp = stdout;
  int len;
  char buf[BUF_SIZE];

  while ((len = fread(buf, sizeof(char), PACKET_SIZE, ifp)) > 0) {
    afio_write(ofp, buf, len, 0);
  }

  return 0;
}
