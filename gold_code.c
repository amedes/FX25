#include <stdio.h>
#include <stdint.h>

#define I_POLY 0x30
#define Q_POLY 0x36

#define CARRY_BIT 0x20

#define I_SEED 0x3f

#define CO_TAG_SIZE 0x41

#define CO_TAG_00 0x00
#define CO_TAG_01 0x01
#define CO_TAG_0B 0x0b
#define CO_TAG_3F 0x3f
#define CO_TAG_40 0x40

#define CO_TAG_BITS 63

uint64_t co_tag[CO_TAG_SIZE];

uint64_t gold_code(uint8_t iseed, uint8_t qseed)
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
#if 1
    // LSb <- MSb order
    ct <<= 1;
    ct |= ic ^ qc;
#else
    // MSb -> LSb order
    ct >>= 1;
    ct |= (uint64_t)(ic ^ qc) << 63;
#endif
  }
  return ct;
}

int gc_init(void)
{
  int i;
  uint64_t tag;

  for (i = 0; i < CO_TAG_40; i++) {
    co_tag[i] = gold_code(I_SEED, i);
  }
  co_tag[CO_TAG_40] = gold_code(0x00, 0x3f);
  
  return 0;
}

int main(void)
{
  int i;

  gc_init();

  for (i = 0; i <= CO_TAG_40; i++) {
    printf("Tag_%02X: 0x%016llX\n", i, co_tag[i]);
  }

  return 0;
}
