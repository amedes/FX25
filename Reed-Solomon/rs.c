/*
  Reed Solomon code library
*/

#include <stdio.h>

#include "rs.h"
#include "poly.h"

static int rs_init_flag = 0;
static int rs_code_len;
static int rs_mesg_len;
static int rs_gp_n;
static int rs_t;
static poly_t rs_gen_p; // generating polynomial

#define RS_ERR (-1)
#define RS_OK 0
#define GF_ELEMENTS 255

/*
  calculate generation polynamial

  n is maximum dimension of generation polynomial.
  return product((x - a^i), i, 0, n)
*/
int make_gp(poly_t *gp, int n)
{
  int i, j;

  if (n >= POLY_SIZE) return -1; // too big

  poly_clear(gp);
  gp->coeff[0] = 1; // initialize

  for (i = 0; i < n; i++) {
    gf_t a = gf_pow(i);
    gf_t b = 0;

    for (j = 0; j <= i; j++) {
      gf_t c = gp->coeff[j]; /* save value */

      gp->coeff[j] = gf_sub(b, gf_mul(c, a)); /* a_i = a_i * a^i + a_i-1 */
      b = c; /* carry next degree of x */
    }
    gp->coeff[j] = b; /* increase most significant degree */
  }
  gp->degree = n;

  return 0;
}

/*
  make generating polynomial G_n(x)
*/
static int make_genp(poly_t *gp, int n)
{
  int i;
  poly_t xa, tmp;
  gf_t an;

  if (n >= POLY_SIZE) return -1; /* too big */

  poly_clear(gp);
  gp->coeff[0] = 1; /* G_0(x) = 1 */

  /* initialize polynomial (x - a^n) */
  poly_clear(&xa);
  xa.degree = 1;
  xa.coeff[1] = 1;

  for (i = 0; i  < n; i++) {
    an = gf_pow(i); /* a^i */
    xa.coeff[0] = gf_neg(an); /* make (x - a^i) */

    poly_mul(gp, &xa, &tmp); /* G_i+i(x) = G_i(x) * (x - a^i) */
    poly_copy(gp, &tmp);
  }

  return 0;
}

int rs_init(int code_len, int mesg_len)
{
  gf_init();

  if (code_len <= 0) return RS_ERR;
  if (code_len > GF_ELEMENTS) return RS_ERR;

  rs_code_len = code_len;

  if (mesg_len >= code_len) return RS_ERR;
  if (mesg_len <= 0) return RS_ERR;

  rs_mesg_len = mesg_len;

  rs_gp_n = code_len - mesg_len;
  rs_t = rs_gp_n / 2;

  if (rs_t <= 0) return RS_ERR;

  make_genp(&rs_gen_p, rs_gp_n);

  /*
  printf("rs_gen_p: %d\n", rs_gp_n);
  poly_print(&rs_gen_p);
  */

  if (rs_init_flag == 0) {
    gf_init();
    rs_init_flag = 1;
  }

  return RS_OK;
}

/*
  generate RS parity from message
*/
int rs_encode(gf_t code[])
{
  int i;
  poly_t dividend, quotinent, remainder;

  /* clear low coefficient of dividend */
  for (i = 0; i < rs_gp_n; i++) {
    dividend.coeff[i] = 0;
  }

  /* copy message to dividend */
  for (i = 0; i < rs_mesg_len; i++) {
    dividend.coeff[(rs_code_len - 1) - i] = code[i];
  }

  dividend.degree = rs_code_len - 1;
  poly_normalize(&dividend);


  /* divide by generating polynomial */
  poly_div(&dividend, &rs_gen_p, &quotinent, &remainder);

  /* copy negated remainder */
  for (i = 0; i < rs_gp_n; i++) {
    code[rs_mesg_len + i] = gf_neg(remainder.coeff[(rs_gp_n - 1) - i]);
  }

  return RS_OK;
}

/*
  decode RS code

  input: received RS code
  output: corrected code
  return value: zero or positive, correction successful, value is number of corrected symbols
  negative value: unsuccessful of error correction
*/
int rs_decode(gf_t code[])
{
  int i;
  poly_t code_p, syn_p;
  poly_t quotient, remainder;

 /* make polynomial from received code */
  for (i = 0; i < rs_code_len; i++) {
    code_p.coeff[(rs_code_len - 1) - i] = code[i];
  }
  code_p.degree = rs_code_len - 1;
  poly_normalize(&code_p);

  /* calculate syndrome */

  /*
  printf("code_p\n");
  poly_print(&code_p);

  printf("Syndrome\n");
  */

  /* calculate syndrome and make polinomial S(x) */

  for (i = 0; i < rs_gp_n; i++) { // rs_gp_n is number of parity
    gf_t s = poly_subst(&code_p, gf_pow(i));
    syn_p.coeff[(rs_gp_n - 1) - i] = s;
    /*printf("C(a^%d) = %d, a^%d\n", i, s, gf_ind(s));*/
  }
  syn_p.degree = rs_gp_n - 1;
  poly_normalize(&syn_p);

  /* check, is syndrome all zero ? */

  if (poly_iszero(&syn_p)) {
#if 1
    return 0; /* no error */
#else
    /* parity check */
    poly_div(&code_p, &rs_gen_p, &quotient, &remainder);
    if (poly_iszero(&remainder)) return 0; /* no error */
 
    return -3; /* something wrong */
#endif
  }

  poly_t x_2t; /* polinomial X^(2*t) */

  poly_clear(&x_2t);
  x_2t.degree = rs_gp_n;
  x_2t.coeff[rs_gp_n] = 1;

  poly_t sigma, omega;
  poly_t phi;

  /* calculate sigma(x), omega(x) */
  poly_euclid(&x_2t, &syn_p, &sigma, &omega, &phi, rs_gp_n / 2);

  /* error correction */

  /* calculate -sigma(x) */
  poly_neg(&sigma);

  poly_t dsigma;

  /* differentiating sigma */
  poly_diff(&sigma, &dsigma);

  /* find error position */
  int errs = 0;

  for (i = 0; (i < rs_code_len) && (errs < rs_t); i++) {
    gf_t an = gf_pow(i);

    if (poly_subst(&sigma, an) == 0) { /* found error position */
      gf_t c, e, n;

      errs++;

      /* calculate error value */
      e = gf_div(poly_subst(&omega, an), poly_subst(&dsigma, an));

      /* correct message */
      c = code[(rs_code_len - 1) - i];
      n = gf_sub(c, e);
      code[(rs_code_len - 1) - i] = n;

      /* correct code polinomial for parity check */
      code_p.coeff[i] = n;
    }
  }

  if (errs == 0) { /* there is no error, something wrong */
    return -1;
  }

  /* check correctness of RS decoded data */
  code_p.degree = rs_code_len - 1;
  poly_normalize(&code_p);
  poly_div(&code_p, &rs_gen_p, &quotient, &remainder);
  if (!poly_iszero(&remainder)) {
    return -2; /* error correction unsuccessful */
  }

  return errs;
}
