#include <stdio.h>

#include "packet_table.h"

int main(void)
{
  int i;
  int len;
  int count = 0;
  int total = 0;

  i = 0;
  while ((len = packet_table[i++]) > 0) {
    ++count;
    total += len;
    i += len;
  }
  printf("%d packets, %d total bytes\n", count, total);
  return 0;
}
