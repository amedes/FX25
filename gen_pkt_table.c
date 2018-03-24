/*
  generate packet data table
*/
#include <stdio.h>

int main(int argc, char *argv[])
{
  FILE *fp;
  int i;
  int ch;

  if (argc < 2) {
    fprintf(stderr, "usage: %s packet_file\n", argv[0]);
    return 1;
  }

  fp = fopen(argv[1], "r");
  if (fp == NULL) {
    perror("fopen");
    return 1;
  }

  printf("const unsigned char packet_table[] = {\n");
  
  i = 0;
  while ((ch = fgetc(fp)) != EOF) {
    if ((i % 16) == 0) {
      printf("  ");
    }

    if ((i % 16) != 15) {
      printf("0x%02x, ", ch);
    } else {
      printf("0x%02x,\n", ch);
    }
    i++;
  }

  if ((i % 16) == 0) {
    printf("  ");
  }
  printf("0x00\n"); /* end mark */
  printf("};\n");

  return 0;
}
