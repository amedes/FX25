/*
  Enumerate Galois Field GF(2^8)
*/

#include <stdio.h>
#include <stdlib.h>

#define GF8_ELEMENT 256

typedef unsigned char gf8;

gf8 gf8_ind[GF8_ELEMENT];
gf8 gf8_pow[GF8_ELEMENT];

void gf8_init(void)
{
  int i;
  const int b = 0x1d; /* x^8 = x^4 + x^3 + x^2 + 1*/
  int n = 1;

  /* make index and power tables */
  for (i = 0; i < GF8_ELEMENT - 1; i++) {
    gf8_ind[n] = i; /* n = a^i */
    gf8_pow[i] = n; /* a^i = n */

    n <<= 1; /* a^(i+1) */
    if (n > 255) {
      n = (n & 0xff) ^ b; /* - a^8 + a^4 + a^3 + a^2 + 1 */
    }
  }
  gf8_ind[0] = 255;
  gf8_pow[255] = 0;
}

gf8 gf8_add(gf8 x, gf8 y)
{
  return x ^ y;
}


/* same as gf8_add */
gf8 gf8_sub(gf8 x, gf8 y)
{
  return x ^ y;
}

gf8 gf8_mul(gf8 x, gf8 y)
{
  int ind;

  if (x == 0 || y == 0) return 0;

  ind = gf8_ind[x] + gf8_ind[y];
  ind %= 255;

  return gf8_pow[ind];
}

/* reciplocal number x^(-1) */
gf8 gf8_recip(gf8 x)
{
  if (x <= 1) return x;

  return gf8_pow[255 - gf8_ind[x]];
}

gf8 gf8_div(gf8 x, gf8 y)
{
  return gf8_mul(x, gf8_recip(y));
}

#define RS_CODE_LEN 255
#define RS_MESG_LEN 239
#define RS_PARITY_LEN (RS_CODE_LEN - RS_MESG_LEN)
#define RS_GP_LEN_MAX 33 /* G(x) = x^32 + .. */

static gf8 rs_gp[RS_GP_LEN_MAX];
static int rs_gp_len;

gf8 *gen_code(gf8 mesg[], int mesg_len)
{
  /*  Generation polynominal for 16 dimension
      G(x) = (x - a^0) * (x - a^1) * ... * (x - a^15)
 */
#define RS_GP_LEN 16
  int i, j;
  /*static gf8 reminder[RS_GP_LEN];*/
  static gf8 dividend[RS_CODE_LEN];

  for (i = 0; i < rs_gp_len; i++)
    dividend[i] = 0;

  for (i = 0; i < mesg_len; i++)
    dividend[i + rs_gp_len] = mesg[i];
  
  for (i = mesg_len; i >= rs_gp_len; --i) {
    gf8 q = dividend[i]; /* assume divisor = 1, coeff of x^16 */
    printf("%d,%d ", i, q);
    for (j = rs_gp_len; j >= 0; --j) {
      /* dividend = dividend - quotinent * generation_polynominal */
      dividend[i + j - rs_gp_len] =
	gf8_sub(dividend[i + j - rs_gp_len],
		gf8_mul(q, rs_gp[j]));
    }
  }
  printf("\n");
  
  return dividend; /* remider */
}

/* make generation polynamial */
void gp_init(int n)
{
  gf8 ag[RS_GP_LEN_MAX];
  gf8 xg[RS_GP_LEN_MAX];
  int i, j;

  if (n >= RS_GP_LEN_MAX) return;

  rs_gp_len = n;

  for (i = 0; i <= n; i++) {
    ag[i] = 0;
    xg[i] = 0;
    rs_gp[i] = 0;
  }
  rs_gp[0] = 1;

  for (i = 0; i < n; i++) {
    gf8 a = gf8_pow[i];
    for (j = 0; j <= i; j++) {
      ag[j] = gf8_mul(rs_gp[j], a);
      if (j < i) xg[j + 1] = rs_gp[j]; /* x * g(x) */
    }
    rs_gp[i + 1] = rs_gp[i]; /* G(x) = x^(i+1) + ...  */
    xg[0] = 0; /* clear constant */

    for (j = 0; j <= i; j++) {
      rs_gp[j] = gf8_sub(xg[j], ag[j]);
    }
    for (j = 0; j <= i+1; j++) {
      printf("x^%d=%d,", j, rs_gp[j]);
    }
    printf("\n");
  }
}

gf8 *rs_rem(gf8 *mesg, int mesg_len)
{
  static gf8 rem[RS_GP_LEN_MAX];
  int i, j;

  for (i = 0; i < rs_gp_len; i++)
    rem[i] = 0;

  for (i = 0; i < mesg_len; i++) {
    gf8 m = mesg[i] ^ rem[rs_gp_len - 1];
    for (j = rs_gp_len - 2; j >= 0; --j) {
      rem[j + 1] = gf8_add(gf8_mul(m, rs_gp[j + 1]) ,rem[j]);
    }
    rem[0] = gf8_mul(m, rs_gp[0]);
  }

  return rem;
}

/* calculate syndrome */
gf8 *syndrome(gf8 code[], int code_len)
{
  int i, j;
  static gf8 syn[RS_GP_LEN_MAX];
  gf8 an;
  gf8 sum;
  gf8 root;

  for (j = 0; j < rs_gp_len; j++) {
    an = 1; /* a^0 */
    root = gf8_pow[j]; /* x = a^j */
    sum = 0;
    for (i = code_len - 1; i >= 0; --i) {
      sum = gf8_add(sum, gf8_mul(an, code[i]));
      an = gf8_mul(an, root); /* next x^n */
    }
    syn[j] = sum;
  }

  return syn;
}

#define POLY_SIZE 255 /* Maximum number of coefficients */

typedef struct POLY {
  int degree;
  gf8 coeff[POLY_SIZE];
} poly_t;

void poly_copy(poly_t *to, poly_t *from)
{
  int i;

  for (i = 0; i <= from->degree; i++)
    to->coeff[i] = from->coeff[i];

  to->degree = from->degree;
}

void poly_normalize(poly_t *poly)
{
  int i;

  //i = poly->degree;
  i = POLY_SIZE - 1;
  while ((i > 0) && (poly->coeff[i] == 0)) {
    --i;
  }
  poly->degree = i;
}

void poly_clear(poly_t *poly)
{
  int i;

  for (i = 0; i <= POLY_SIZE; i++)
    poly->coeff[i] = 0;

  poly->degree = 0;
}

/* return true if poly is zero */
int poly_iszero(poly_t *poly)
{
  poly_normalize(poly);

  return (poly->degree == 0) && (poly->coeff[0] == 0);
}

/* polynomial division */
int poly_div(
	     poly_t *dividend,
	     poly_t *divisor,
	     poly_t *quotient,
	     poly_t *remainder
	     )
{
  int i, j;
  poly_t dd; /* work for divide */

  poly_copy(&dd, dividend);

  /* degree of quotient */
  int qi = dividend->degree - divisor->degree;
  quotient->degree = qi;

  /* reciplocal number of most significant coefficient of divisor */
  gf8 d = divisor->coeff[divisor->degree];

  for (i = dd.degree; i >= divisor->degree; --i) {
    gf8 q = gf8_div(dd.coeff[i], d); /* quotient = dd.coeff / divisor.coeff */
    quotient->coeff[qi--] = q;
    /* subtract by divisor * m */
    for (j = 0; j <= divisor->degree; j++) {
      dd.coeff[i - divisor->degree + j] = gf8_sub(dd.coeff[i - divisor->degree + j], gf8_mul(divisor->coeff[j], q));
    }
  }

  poly_clear(remainder);
  poly_copy(remainder, &dd);
  poly_normalize(remainder);

  return 0;
}

/* output polynomial coefficients */
void poly_print(poly_t *poly)
{
  int i;
  gf8 c;

  for (i = 0; i <= poly->degree; i++) {
    c = poly->coeff[i];
    if ((c != 0) || (i == 0)) printf("coeff[%d] = %d, a^%d\n", i, c, gf8_ind[c]);
  }
}

/*
  make generation polynamial
  
  return product((x - a^i), i, 0, n)
*/
void make_gp(poly_t *gp, int n)
{
  int i, j;

  if (n > POLY_SIZE) return;

  poly_clear(gp);
  gp->coeff[0] = 1; // initialize

  for (i = 0; i < n; i++) {
    gf8 a = gf8_pow[i];
    gf8 b = 0;
    for (j = 0; j <= i; j++) {
      gf8 c = gp->coeff[j]; /* save value */
      gp->coeff[j] = gf8_sub(b, gf8_mul(c, a)); /* a_i = a_i * a^i + a_i-1 */
      b = c; /* carry next degree of x */
    }
    gp->coeff[j] = b; /* increase most significant degree */
  }
  gp->degree = n;
}

/* calculate polynomial value */
gf8 poly_subst(poly_t *poly, gf8 x)
{
  int i;
  gf8 v = 0;

  for (i = poly->degree; i >= 0; --i) {
    v = gf8_add(gf8_mul(v, x), poly->coeff[i]);
  }

  return v;
}

/* polynomial addition */
void poly_add(poly_t *p1, poly_t *p2, poly_t *result)
{
  int i;

  poly_clear(result);

  i = 0;
  while ((i <= p1->degree) || (i <= p2->degree)) {
    result->coeff[i] = gf8_add((i <= p1->degree) ? p1->coeff[i] : 0,
			       (i <= p2->degree) ? p2->coeff[i] : 0);
    //printf("poly_add: coeff[%d] = %d\n", i, result->coeff[i]);
    i++;
  }

  result->degree = i - 1;
  poly_normalize(result);
  //printf("poly_add: degree: %d\n", result->degree);
}

/* polynomial subtraction */
void poly_sub(poly_t *p1, poly_t *p2, poly_t *result)
{
  int i;

  poly_clear(result);

  i = 0;
  while ((i <= p1->degree) || (i <= p2->degree)) {
    result->coeff[i] = gf8_sub((i <= p1->degree) ? p1->coeff[i] : 0,
			       (i <= p2->degree) ? p2->coeff[i] : 0);
    //printf("poly_sub: coeff[%d] = %d\n", i, result->coeff[i]);
    i++;
  }

  result->degree = i - 1;
  //printf("poly_add: degree: %d\n", result->degree);
}

/* polynomial multiplication */
void poly_mul(poly_t *p1, poly_t *p2, poly_t *result)
{
  int i, j;

  poly_clear(result);

  for (i = 0; i <= p1->degree; i++) {
    gf8 m = p1->coeff[i];
    for (j = 0; j <= p2->degree; j++) {
      gf8 t;
      t = gf8_mul(m, p2->coeff[j]);
      result->coeff[i + j] = gf8_add(result->coeff[i + j], t);
    }
  }
  result->degree = p1->degree + p2->degree;
  poly_normalize(result);
}

/*
  extended Euclidean algorithm

  input x, y
  output a, b, c where a*x + b*y = c (deg c < t)
*/
void exeuclidean(poly_t *x, poly_t *y, poly_t *a, poly_t *b, poly_t *c, int t)
{
  poly_t a0, a1, a2;
  poly_t b0, b1, b2;
  poly_t r0, r1, r2;
  poly_t q1;
  poly_t tmp;

  printf("t = %d\n", t);

  poly_copy(&r0, x);
  poly_copy(&r1, y);

  poly_clear(&a0); a0.coeff[0] = 1;
  poly_clear(&a1);
  poly_clear(&b0);
  poly_clear(&b1); b1.coeff[0] = 1;

  while (r1.degree >= t) {

    poly_div(&r0, &r1, &q1, &r2); // q1 ... r2 = r0 / r1

    poly_mul(&q1, &a1, &tmp); // tmp = aq * a1
    poly_sub(&a0, &tmp, &a2); // a2 = a0 - tmp

    poly_mul(&q1, &b1, &tmp); // tmp = q1 * b1
    poly_sub(&b0, &tmp, &b2); // b2 = b0 - tmp

    poly_copy(&r0, &r1); // r0 = r1
    poly_copy(&r1, &r2); // r1 = r2

    poly_copy(&a0, &a1); // a0 = a1
    poly_copy(&a1, &a2); // a1 = a2

    poly_copy(&b0, &b1); // b0 = b1
    poly_copy(&b1, &b2); // b1 = b2
  }

  poly_copy(c, &r1); // c = r0
  poly_copy(a, &b1); // a = a0
  poly_copy(b, &a1); // b = b0
}

static int rs_init_flag = 0;
static int rs_code_len;
static int rs_mesg_len;
static int rs_gp_n;
static int rs_t;
static poly_t rs_gen_p; // generating polynomial

#define RS_ERR (-1)
#define RS_OK 0
#define GF_ELEMENTS 255


int rs_init(int code_len, int mesg_len)
{
  if (code_len <= 0) return RS_ERR;
  if (code_len > GF_ELEMENTS) return RS_ERR;

  rs_code_len = code_len;

  if (mesg_len >= code_len) return RS_ERR;
  if (mesg_len <= 0) return RS_ERR;

  rs_mesg_len = mesg_len;

  rs_gp_n = code_len - mesg_len;
  rs_t = rs_gp_n / 2;

  if (rs_t <= 0) return RS_ERR;

  make_gp(&rs_gen_p, rs_gp_n);

  printf("rs_gen_p: %d\n", rs_gp_n);
  poly_print(&rs_gen_p);

  if (rs_init_flag == 0) {
    gf8_init();
    rs_init_flag = 1;
  }

  return RS_OK;
}

/*
  Differentiating polynomial
 */
void poly_diff(poly_t *f, poly_t *df)
{
  int i;

  poly_clear(df);
  for (i = 1; i <= f->degree; i++) {
    gf8 c = 0;
    gf8 a = f->coeff[i];
    int j = i;

    while (j-- > 0) { /* add i times (can not use gf8_mul) */
      c = gf8_add(c, a);
    }

    df->coeff[i - 1] = c; /* coefficient of x^(i-1) */
  }
  poly_normalize(df);
}

/*
  generate RS parity from message
*/
int rs_encode(gf8 code[])
{
  int i;
  poly_t dividend, quotinent, remainder;

  poly_clear(&dividend);

  /* copy message to dividend */
  for (i = 0; i < rs_mesg_len; i++) {
    dividend.coeff[(rs_code_len - 1) - i] = code[i];
  }
  dividend.degree = rs_code_len - 1;

  /* divide by generating polynomial */
  poly_div(&dividend, &rs_gen_p, &quotinent, &remainder);

  /* copy parity to code */
  for (i = 0; i < rs_gp_n; i++) {
    code[rs_mesg_len + i] = remainder.coeff[(rs_gp_n - 1) - i];
  }

  return RS_OK;
}

/*
  decode RS code
*/
int rs_decode(gf8 code[])
{
  int i;
  poly_t code_p, syn_p;

  /* make polynomial from received code */
  poly_clear(&code_p);
  for (i = 0; i < rs_code_len; i++) {
    code_p.coeff[(rs_code_len - 1) - i] = code[i];
  }
  code_p.degree = rs_code_len;
  poly_normalize(&code_p);

  /* calculate syndrome */

  printf("code_p\n");
  poly_print(&code_p);

  poly_clear(&syn_p);
  printf("Syndrome\n");
  for (i = 0; i < rs_gp_n; i++) { // rs_gp_n is number of parity
    gf8 s = poly_subst(&code_p, gf8_pow[i]);
    syn_p.coeff[(rs_gp_n - 1) - i] = s;
    printf("C(a^%d) = %d, a^%d\n", i, s, gf8_ind[s]);
  }
  syn_p.degree = rs_gp_n - 1;
  poly_normalize(&syn_p);

  if ((syn_p.degree == 0) && (syn_p.coeff[0] == 0)) {
    return RS_OK; // no error
  }

  poly_t x_2t;

  poly_clear(&x_2t);
  x_2t.degree = rs_gp_n;
  x_2t.coeff[rs_gp_n] = 1;

  poly_t sigma, omega;
  poly_t phi;

  printf("x^(2*t)\n");
  poly_print(&x_2t);
  printf("S(x)\n");
  poly_print(&syn_p);


  /* calculate sigma(x), omega(x) */
  exeuclidean(&x_2t, &syn_p, &sigma, &omega, &phi, rs_gp_n / 2);

  printf("sigma(x)\n");
  poly_print(&sigma);
  printf("omega(x)\n");
  poly_print(&omega);

  printf("phi(x)\n");
  poly_print(&phi);

  poly_t t0, t1, t2;

  poly_mul(&x_2t, &sigma, &t0);
  poly_mul(&syn_p, &omega, &t1);
  poly_add(&t0, &t1, &t2);

  printf("a*x + b*y =\n");
  poly_print(&t2);

  /* error correction */

  poly_t dsigma;

  /* differentiating sigma */
  poly_diff(&sigma, &dsigma);

  /* find error position */
  int errs = 0;
  printf("find error position\n");
  for (i = 0; i < rs_code_len; i++) {
    gf8 an = gf8_pow[i];

    if (poly_subst(&sigma, an) == 0) {
      gf8 c, e, n;

      errs++;
      printf("sigma(a^%d) = 0\n", i);

      /* calculate error vaule */
      e = gf8_div(poly_subst(&omega, an), poly_subst(&dsigma, an));
      
      /* correct message */
      c = code[(rs_code_len - 1) - i];
      n = gf8_sub(c, e);
      code[(rs_code_len - 1) - i] = n;

      printf("m[%d]: old: %d, err: %d, new: %d\n", i, c, e, n);

    }
  }

  if (errs == 0) { // parity check wheather error symbols > parity / 2 ?
    poly_t quotinent, remainder;

    /* divide by generating polynomial */
    poly_div(&code_p, &rs_gen_p, &quotinent, &remainder);
    
    if (!poly_iszero(&remainder)) return -1; // parity error
  }

  return errs;
}

int main(void)
{
  const int b = 0x1d; /* x^8 = x^4 + x^3 + x^2 + 1 */
  int i;
  int n = 1;
  int ind[256];
  int an[256];

  gf8_init();

  printf("n, ind(n), pow(n), recip(n}, n * recip(n)\n");
  for (i = 0; i < 256; i++) {
    int r = gf8_recip(i);
    printf("%3d: %3d, %3d, %3d, %3d\n",
	   i, gf8_ind[i], gf8_pow[i], r, gf8_mul(i, r));
  }

  n = 2;
  gp_init(n); /* number of parity is 3 */
  for (i = 0; i <= n; i++) {
    printf("coeff of x^%d = %d, a^%d\n", i, rs_gp[i], gf8_ind[rs_gp[i]]);
  }

  n = 8;
  gp_init(n); /* number of parity is 8 */
  for (i = 0; i <= n; i++) {
    printf("coeff of x^%d = %d, a^%d\n", i, rs_gp[i], gf8_ind[rs_gp[i]]);
  }

  n = 16;
  gp_init(n); /* number of parity is 16 */
  for (i = 0; i <= n; i++) {
    printf("coeff of x^%d = %d, a^%d\n", i, rs_gp[i], gf8_ind[rs_gp[i]]);
  }
  //  exit(0);

  gf8 m[239];
  gf8 *r;
  gf8 c[255];

  n = 17;
  gp_init(n); /* number of parity is 17 */
  for (i = 0; i <= n; i++) {
    printf("coeff of x^%d = %d, a^%d\n", i, rs_gp[i], gf8_ind[rs_gp[i]]);
  }

  /* set message values */
  for (i = 0; i < 239; i++) {
    m[i] = i;
  }

  m[0] = 32;
  m[1] = 65;
  m[2] = 205;
  m[3] = 69;
  m[4] = 41;
  m[5] = 220;
  m[6] = 46;
  m[7] = 128;
  m[8] = 236;

  for (i = 0; i < 9; i++) {
    printf("%d, ", m[i]);
  }
  printf("\n");

  r = gen_code(m, 9+n);

  for (i = 0; i < n; i++) {
    printf("r[%d]: %d\n", i, r[i]);
  }

  for (i = 0; i < 9; i++) {
    printf("%d, ", m[i]);
  }
  printf("\n");

  r = rs_rem(m, 9);

  for (i = 0; i < n; i++) {
    printf("r[%d]: %d\n", i, r[i]);
  }

  /* Is check parity correct? */
  for (i = 0; i < n; i++) {
    m[9 + n - i - 1] = r[i];
  }

  //m[9] ^= 0x01; /* error */

  r = rs_rem(m, 9 + n);

  for (i = 0; i < n; i++) {
    printf("r[%d]: %d\n", i, r[i]);
  }

  /* syndrome */
  r = syndrome(m, 9 + n);
  for (i = 0; i < n; i++) {
    printf("syndrome[%d] = %d\n", i, r[i]);
  }

  poly_t dd, dv, qo, re;

  poly_clear(&dd);

  for (i = 0; i < 9; i++) {
    dd.coeff[9 + n - i - 1] = m[i];
  }
  dd.degree = 9 + n - 1;

  for (i = 0; i <= n; i++) {
    dv.coeff[i] = rs_gp[i];
  }
  dv.degree = n;

  printf("dividend\n");
  poly_print(&dd);

  printf("divisor\n");
  poly_print(&dv);

  poly_div(&dd, &dv, &qo, &re);

  printf("quotient\n");
  poly_print(&qo);

  printf("reminder\n");
  poly_print(&re);

  for (i = 0; i <= re.degree; i++) {
    printf("re[%d] = %d\n", i, re.coeff[i]);
  }

  poly_t g4;

  make_gp(&g4, 4);
  printf("g4\n");
  poly_print(&g4);

  /* RS(10,6) */
  n = 10;
  int k = 6;
  int t = 2;
  poly_t rs10_6;

  poly_clear(&rs10_6);

  int info[] = { 1, 7, 3, 2, 0, 5 };

  for (i = 0; i < sizeof(info) / sizeof(info[0]); i++) {
    rs10_6.coeff[n - k + i] = info[i];
  }
  poly_normalize(&rs10_6);
  printf("rs10_6\n");
  poly_print(&rs10_6);

  poly_div(&rs10_6, &g4, &qo, &re);

  printf("remainder\n");
  poly_print(&re);

  for (i = 0; i <= re.degree; i++) {
    rs10_6.coeff[i] = re.coeff[i];
  }

  printf("rs10_6\n");
  poly_print(&rs10_6);

  poly_div(&rs10_6, &g4, &qo, &re);

  printf("remainder\n");
  poly_print(&re);

  /* add error */
  rs10_6.coeff[3] = gf8_add(rs10_6.coeff[3], 9);
  rs10_6.coeff[9] = gf8_add(rs10_6.coeff[9], 5);

  printf("rs10_6\n");
  poly_print(&rs10_6);

  poly_div(&rs10_6, &g4, &qo, &re);

  printf("remainder\n");
  poly_print(&re);

  poly_t syn;

  poly_clear(&syn);

  printf("syndrome\n");
  for (i = 0; i < n - k; i++) {
    gf8 a = gf8_pow[i]; // a^i
    gf8 s = poly_subst(&rs10_6, a); // J(a^i)
    printf("J(a^%d) = %d\n", i, s);
    syn.coeff[n - k - i - 1] = s;
    //syn.coeff[i] = s;
  }

  poly_normalize(&syn);
  printf("S(x)\n");
  poly_print(&syn);

  poly_t ax, bx;

  t = 2;

  poly_clear(&ax);
  ax.coeff[t * 2] = 1; /* a(x) = x^(2 * t) */
  ax.degree = t * 2;
  printf("a(x)\n");
  poly_print(&ax);

  poly_copy(&bx, &syn); /* S(x) */

/*
  Euclidean Algorithm
  matrix for sigma(x), omega(x)
  (x y)
  (u v)
*/
  poly_t r0, r1, r2, a0, a1, a2, b0, b1, b2, q1, tmp;

  if (ax.degree > syn.degree) {

    poly_copy(&r0, &ax); // x^(2 * t)
    poly_copy(&r1, &syn); // S(x)
  } else {
    poly_copy(&r0, &syn); // S(x)
    poly_copy(&r1, &ax); // x^(2 * t)
  }

  poly_clear(&a0); a0.coeff[0] = 1;
  poly_clear(&a1);
  poly_clear(&b0);
  poly_clear(&b1); b1.coeff[0] = 1;

  printf("r1.degree: %d, t = %d\n", r1.degree, t);

  while (r1.degree >= t) {
    printf("r1.degree: %d, t = %d\n", r1.degree, t);

    poly_div(&r0, &r1, &q1, &r2);

    printf("poly_div\n");
    printf("dividend\n");
    poly_print(&r0);
    printf("divisor\n");
    poly_print(&r1);
    printf("quotinent\n");
    poly_print(&q1);
    printf("remainder\n");
    poly_print(&r2);

    poly_mul(&q1, &a1, &tmp);
    poly_add(&a0, &tmp, &a2);

    printf("a2\n");
    poly_print(&a2);

    poly_mul(&q1, &b1, &tmp);
    poly_add(&b0, &tmp, &b2);

    printf("b2\n");
    poly_print(&b2);

    poly_copy(&r0, &r1);
    poly_copy(&r1, &r2);

    poly_copy(&a0, &a1);
    poly_copy(&a1, &a2);

    poly_copy(&b0, &b1);
    poly_copy(&b1, &b2);

    printf("r1.degree: %d, t = %d\n", r1.degree, t);
  }

  printf("c\n");
  poly_print(&r0);

  printf("omega(x)\n");
  poly_print(&a1);

  printf("sigma(x)\n");
  poly_print(&b0);

  /* differentiating sigma(x) */
  poly_t dsigma;

  poly_clear(&dsigma);
  for (i = 1; i <= a1.degree; i++) {
    gf8 c = 0;
    gf8 a = a1.coeff[i];
    int j = i;

    while (j-- > 0) { /* add i times */
      c = gf8_add(c, a);
    }

    dsigma.coeff[i - 1] = c; /* coefficient of x^(i-1) */
  }
  dsigma.degree = (a1.degree > 0) ? a1.degree - 1 : 0;
  while ((dsigma.degree > 0) && (dsigma.coeff[dsigma.degree] == 0)) {
    --dsigma.degree;
  }

  printf("diff sigma(x)\n");
  poly_print(&dsigma);

  /* find error position */
  for (i = 0; i < n; i++) {
    gf8 a = gf8_pow[i];
    gf8 v = poly_subst(&b0, a);

    printf("sigma(a^%d) = %d\n", i, v);

    if (v == 0) { /* error found */
      gf8 e = gf8_div(poly_subst(&b1, a), poly_subst(&dsigma, a));
      printf("e(%d) = %d\n", i, e);
      printf("correct: %d\n", gf8_sub(rs10_6.coeff[i], e));
    }
  }

#define RS_CODE 255
#define RS_MESG 239
#define RS_PARI (RS_CODE - RS_MESG)

  if (rs_init(RS_CODE, RS_MESG)) {
    fprintf(stderr, "rs_init() return error\n");
    exit(1);
  }

  unsigned char code[RS_CODE];

  /* make message */
  for (i = 0; i < RS_MESG; i++) 
    code[i] = i;

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
  for (i = 0; i < RS_PARI / 2; i++) {
    err_p.coeff[i] = gf8_pow[i];
  }
  poly_normalize(&err_p);

  printf("error(x)\n");
  poly_print(&err_p);

  /* add error */
  for (i = 0; i <= err_p.degree; i++) {
    code[i] = gf8_add(code[i], err_p.coeff[i]);
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
    printf("J(a^%d) = %d\n", i, poly_subst(&err_p, gf8_pow[i]));
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
      xa.coeff[0] = gf8_pow[i];
      xa.coeff[1] = 1;
      xa.degree = 1;

      poly_mul(&sx, &xa, &t0);
      poly_copy(&sx, &t0);
    }
  }
  printf("\n");
  poly_print(&sx);

  exit(0);

  poly_t g, g0, g1;

  for (i = 0; i < 17; i++) {
    make_gp(&g, i);
    printf("G_%d(x)\n", i);
    poly_print(&g);
  }

  poly_clear(&g0); g0.coeff[1] = 1; g0.degree = 1;
  poly_clear(&g); g.coeff[0] = 1;

  for (i = 0; i < 17; i++) {
    g0.coeff[0] = gf8_pow[i];
    poly_mul(&g, &g0, &g1);
    poly_copy(&g, &g1);
    printf("G_%d(x)\n", i);
    poly_print(&g);

    poly_div(&g, &g0, &qo, &re);
    printf("qotinent\n");
    poly_print(&qo);
    printf("remainder\n");
    poly_print(&re);
  }

  /* RS(255, 239) */
  n = 255;
  k = 239;
  t = 8;

  poly_t rs255_239;
  poly_t g16;

  make_gp(&g16, t * 2);

  printf("G_16(x)\n");
  poly_print(&g16);

  /* make message */
  poly_clear(&rs255_239);
  for (i = 0; i < k; i++) {
    rs255_239.coeff[i + t * 2] = i;
  }
  rs255_239.degree = 255;

  poly_div(&rs255_239, &g16, &qo, &re);

  printf("parity of rs(255, 239)\n");
  poly_print(&re);

  /* copy parity */
  for (i = 0; i <= re.degree; i++) {
    rs255_239.coeff[i] = re.coeff[i];
  }

  /* add error */
  rs255_239.coeff[128] = gf8_add(rs255_239.coeff[128], 1);

  /* syndrome */
  printf("syndrome\n");
  for (i = 0; i < t * 2; i++) {
    gf8 s = poly_subst(&rs255_239, gf8_pow[i]);
    printf("J(a^%d) = %d\n", i, s);
    syn.coeff[t * 2 - 1 - i] = s; /* reverse order */
  }


  exit(0);

  for (i = 0; i < 255; i++) {
    an[i] = n;
    ind[n] = i;

    printf("%3d, ", i);
    int j = (n << 1) | 1 /* sentinel */;

    /* repeat eight times */
    while (j != 0x100) {
      if (j & 0x100) {
	printf("1");
	j &= 0xff;
      } else {
	printf("0");
      }
      j <<= 1;
    }
    printf("\n");

    n <<= 1;
    if (n & 0x100) {
      n = b ^ (n & 0xff);
    }
  }
}
