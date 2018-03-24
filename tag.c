#include <stdio.h>
#include <stdint.h>

#include "tag.h"

#define REVERSED 1 // bit order reversed if defined

uint64_t co_tag[CO_TAG_SIZE];

static uint64_t gold_code(uint8_t iseed, uint8_t qseed)
{
  uint8_t ix = iseed;
  uint8_t qx = qseed;
  uint8_t ib, qb;
  uint8_t ic, qc;
  uint8_t qt;
  uint64_t ct = 0;
  int i;

  for (i = 0; i < CO_TAG_BITS; i++) {
    /* I(x) */
    ic = (ix & CARRY_BIT) != 0; // read x^6
    ib = ((ix ^ (ix << 1)) & CARRY_BIT) != 0; // I(x) = x^6 + x^5
    ix <<= 1;
    ix |= ib;

    /* Q(x) */
    qc = (qx & CARRY_BIT) != 0; // read x^6
    qt = (qx ^ (qx << 1));		       // Q(x) = x^6 + x^5 + x^3 + x^2
    qb = ((qt ^ (qt << 3)) & CARRY_BIT) != 0;
    qx <<= 1;
    qx |= qb;

    /* Correlation Tag */
#ifdef REVERSED
    ct <<= 1;
    ct |= ic ^ qc;
#else
    ct >>= 1;
    ct |= (uint64_t)(ic ^ qc) << 63;
#endif
  }
  return ct;
}

int tag_init(void)
{
  int i;
  uint64_t tag;

  for (i = 0; i < CO_TAG_40; i++) {
    co_tag[i] = gold_code(I_SEED, i);
  }
  co_tag[CO_TAG_40] = gold_code(0x00, 0x3f);
  
  return 0;
}
