/*
  Galoris Feild processing routines
*/

#ifndef GF_H
#define GF_H

/* uncomment one of these identfiers */

#define USE_GF2P8 1
//#define USE_GFP 1

#ifdef USE_GF2P8

#include "gf2p8.h"

#define GF_ORDER GF2P8_ORDER

#define gf_t gf2p8_t

#define gf_init gf2p8_init
#define gf_add gf2p8_add
#define gf_sub gf2p8_sub
#define gf_mul gf2p8_mul
#define gf_div gf2p8_div
#define gf_pow gf2p8_pow
#define gf_ind gf2p8_ind
#define gf_neg gf2p8_neg

#elif defined(USE_GFP)

#ifndef GFP_PRIME

/* must define GFP_PRIME and GFP_PRIMITIVE */
#define GFP_PRIME 11
#define GFP_PRIMITIVE 2

#endif

#include "gfp.h"

#define gf_t gfp_t

#define GF_ORDER (GFP_PRIME-1)

#define gf_init gfp_init
#define gf_add gfp_add
#define gf_sub gfp_sub
#define gf_mul gfp_mul
#define gf_div gfp_div
#define gf_pow gfp_pow
#define gf_ind gfp_ind
#define gf_neg gfp_neg

#endif

#endif
