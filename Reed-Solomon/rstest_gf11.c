/*
  Reed Solomon encode/decode test program
 */

#include <stdio.h>
#include <stdlib.h>

#include "gf.h"
#include "poly.h"
#include "rs.h"

#define RS_CODE 10
#define RS_MESG 6
#define RS_PARI (RS_CODE - RS_MESG)

int main(void)
{
  int i;

  /* initialize RS library by rs_init(n, k) */
  if (rs_init(RS_CODE, RS_MESG)) {
    fprintf(stderr, "rs_init() return error\n");
    exit(1);
  }

  unsigned char code[RS_CODE] = {
    5, 0, 2, 3, 7, 1,
  };

  /* make message */
  /*
  for (i = 0; i < RS_MESG; i++) 
    code[i] = i;
  */

  if (rs_encode(code)) {
    fprintf(stderr, "rs_encode() return error\n");
    exit(1);
  }

  printf("Encoded code\n");
  for (i = 0; i < RS_CODE; i++) {
    printf("code[%d] = %d\n", i, code[i]);
  }

  poly_t err_p; // error polynomial

  /* generate error polynomial */
  poly_clear(&err_p);
  /*
  for (i = 0; i < RS_PARI / 2; i++) {
    err_p.coeff[i] = gf_pow(i);
  }
  err_p.degree = i - 1;
  */

  //err_p.coeff[0] = 5;
  err_p.coeff[6] = 9;
  err_p.degree = 6;

  printf("error(x)\n");
  poly_print(&err_p);

  /* add error */
  for (i = 0; i <= err_p.degree; i++) {
    code[i] = gf_add(code[i], err_p.coeff[i]);
  }

  printf("code with error\n");
  for (i = 0; i < RS_CODE; i++)
    printf("code[%d] = %d\n", i, code[i]);

  int cnt = rs_decode(code);

  if (cnt == 0) {
    printf("no error\n");
    exit(0);
  } else if (cnt < 0) {
    printf("too many errors to correct\n");
    exit(1);
  }

  printf("%d errors corrected\n", cnt);

  printf("corrected code\n");
  for (i = 0; i < RS_CODE; i++) {
    printf("code[%d] = %d\n", i, code[i]);
  }

  printf("syndrome from error\n");
  for (i = 0; i < RS_PARI; i++) {
    printf("J(a^%d) = %d\n", i, poly_subst(&err_p, gf_pow(i)));
  }

  poly_t sx;

  poly_clear(&sx);
  sx.coeff[0] = 1;

  printf("sigma(x)\n");
  for (i = 0; i  <= err_p.degree; i++) {
    if (err_p.coeff[i]) {
      poly_t xa, t0;

      printf("(x - a^%d) ", i);

      /* make (x - a^i) */
      xa.coeff[0] = gf_pow(i);
      xa.coeff[1] = 1;
      xa.degree = 1;

      poly_mul(&sx, &xa, &t0);
      poly_copy(&sx, &t0);
    }
  }
  printf("\n");
  poly_print(&sx);

  return 0;
}
