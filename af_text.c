#include <stdio.h>
#include <string.h>

#include "afio.h"

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
  FILE *ifp = stdin;
  FILE *ofp = stdout;
  char buf[BUF_SIZE];

  while (fgets(buf, BUF_SIZE, ifp) != NULL) {
    afio_write(ofp, buf, strlen(buf), 0);
  }
  
  return 0;
}
