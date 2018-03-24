#include <stdio.h>

int main(int argc, char *argv[])
{
  static char data[] = {
    0x7e,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x7e,
  };
  int i, j;

#define STDOUT 1

  for (j = 0; j < 100; j++)
    for (i = 0; i < 256; i++) putc(i, stdout);

  //write(STDOUT, data, sizeof(data));

  return 0;
}
