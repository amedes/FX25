#include <stdio.h>

#include "afio.h"

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
  FILE *ifp = stdin;
  FILE *ofp = stdout;
  char buf[BUF_SIZE];
  int bit_length;

  while ((bit_length = afio_read(ifp, buf, BUF_SIZE)) >= 0) {
    if (bit_length > 0) {
      fwrite(buf, sizeof(char), bit_length / 8, ofp);
    }
  }
}
