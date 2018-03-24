/*
  Operations on Galois Field GF(prime)
*/

#ifndef GFP_H

#ifndef GFP_PRIME

#define GFP_PRIME 11
#define GFP_PRIMITIVE 2 /* primitive element */

#endif

#define GFP_ELEMENTS GFP_PRIME
#define GFP_ORDER (GFP_PRIME-1)

#if GFP_PRIME < 256

typedef unsigned char gfp_t;

#elif GFP_PRIME < 65536

typedef unsigned short int gfp_t;

#else

typedef unsigned long int gfp_t;

#endif

/*
  initialize internal tables
*/
void gfp_init(void);


/*
  addtion
  return x + y
*/
gfp_t gfp_add(gfp_t x, gfp_t y);

/*
  subtraction
  return x - y
*/
gfp_t gfp_sub(gfp_t x, gfp_t y);

/*
  index
  return y where a^y = x
 */
gfp_t gfp_ind(gfp_t x);

/*
  power
  return a^x
 */
gfp_t gfp_pow(gfp_t x);

/*
  multiplication
  return x * y
*/
gfp_t gfp_mul(gfp_t x, gfp_t y);

/*
  reciplocal number
  return y where x * y = 1
 */
gfp_t gfp_recip(gfp_t x);

/*
  dividing
  return x / y
*/
gfp_t gfp_div(gfp_t x, gfp_t y);

/*
  negate
  return -x
*/
gfp_t gfp_neg(gfp_t x);

#define GFP_H 1
#endif
