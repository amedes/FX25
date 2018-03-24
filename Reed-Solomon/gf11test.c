/*
  GF11 test program
*/

#include <stdio.h>
#include <stdlib.h>


#define USE_GFP 1
#define GFP_PRIME 257
#define GFP_PRIMITIVE 3


#include "gf.h"
#include "poly.h"
#include "rs.h"

#define CODE_LEN 10
#define DATA_LEN 6

int main(void)
{
  int i;
  gf_t code[CODE_LEN] = { 5, 0, 2, 3, 7, 1 };

  if (rs_init(CODE_LEN, DATA_LEN)) {
    fprintf(stderr, "rs_init() return error\n");
    exit(1);
  }

  if (rs_encode(code)) {
    fprintf(stderr, "rs_encode() return error\n");
    exit(1);
  }

  printf("encode result\n");
  for (i = 0; i < CODE_LEN; i++) {
    printf("%d ", code[i]);
  }
  printf("\n");

  code[0] = 10;
  code[6] = 2;

  printf("received data\n");
  for (i = 0; i < CODE_LEN; i++) {
    printf("%d ", code[i]);
  }
  printf("\n");

  if (rs_decode(code) < 0) {
    fprintf(stderr, "rs_decode() return error\n");
  }

  printf("decode result\n");
  for (i = 0; i < CODE_LEN; i++) {
    printf("%d ", code[i]);
  }
  printf("\n");

  for (i = 0; i < GFP_PRIME; i++) {
    printf("neg(%d) = %d\n", i, gf_neg(i));
  }

  return 0;
}
