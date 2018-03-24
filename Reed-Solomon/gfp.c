/*
  Operations on Galois Field GF(prime)
*/

#include "gfp.h"

static gfp_t gfp_index[GFP_ELEMENTS]; /* table for ind */
static gfp_t gfp_power[GFP_ELEMENTS]; /* table for pow */
static gfp_t gfp_reciprocal[GFP_ELEMENTS]; /* table for reciprocal number */

/*
  initialize internal tables
*/
void gfp_init(void)
{
  static int init_flag = 0;
  int i, j;
  gfp_t a = GFP_PRIMITIVE; /* primitive element */
  int n = 1; /* a^0 */

  if (init_flag) return;

  /* make index and power tables */
  for (i = 0; i < GFP_ELEMENTS - 1; i++) {
    gfp_index[n] = i; /* n = a^i */
    gfp_power[i] = n; /* a^i = n */

    n = gfp_mul(n, a); /* a^(i+1) = a^n * a, increment index of a^n */
  }
  gfp_index[0] = GFP_PRIME - 1; // 0 has no index, assign maximum element
  gfp_power[GFP_PRIME - 1] = 1; // a^(GF_PRIME - 1) = 1

  /* make reciplocal number table */
  for (i = 1; i < GFP_PRIME; i++) {
    for (j = 1; j < GFP_PRIME; j++) {
      if (gfp_mul(i, j) == 1) {
	gfp_reciprocal[i] = j;
	break;
      }
    }
  }
  gfp_reciprocal[0] = 0; // 0 has no reciprocal number;

  init_flag = 1;
}

/*
  addtion
  return x + y
*/
gfp_t gfp_add(gfp_t x, gfp_t y)
{
  return (x + y) % GFP_PRIME;
}

/*
  subtraction
  return x - y
*/
gfp_t gfp_sub(gfp_t x, gfp_t y)
{
  return (x + gfp_neg(y)) % GFP_PRIME;
}


/*
  index
  return y where a^y = x
 */
gfp_t gfp_ind(gfp_t x)
{
  return gfp_index[x];
}

/*
  power
  return a^x
 */
gfp_t gfp_pow(gfp_t x)
{
  return gfp_power[x];
}

/*
  multiplication
  return x * y
*/
gfp_t gfp_mul(gfp_t x, gfp_t y)
{
  return (x * y) % GFP_PRIME;
}

/*
  reciplocal number
  return y where x * y = 1
 */
gfp_t gfp_recip(gfp_t x)
{
  if (x <= 1) return x; // return 0 if x == 0

  return gfp_reciprocal[x];
}

/*
  dividing
  return x / y
*/
gfp_t gfp_div(gfp_t x, gfp_t y)
{
  return (x * gfp_recip(y)) % GFP_PRIME;
}

/*
  negate
  return -x
*/
gfp_t gfp_neg(gfp_t x)
{
  if (x == 0) return 0;

  return GFP_PRIME - x;
}
