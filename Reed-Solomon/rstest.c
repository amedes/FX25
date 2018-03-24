/*
  Reed Solomon encode/decode test program
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "gf.h"
#include "poly.h"
#include "rs.h"

#define RS_CODE 255
#define RS_MESG 239
#define RS_PARI (RS_CODE - RS_MESG)

#define ERR_RATE 255 /* error rate is 1/ERR_RATE */
#define LOOP_COUNT 10000

int main(void)
{
  int i, j, k, n, m, l;
  unsigned char code1[RS_CODE], code2[RS_CODE], code3[RS_CODE], code4[RS_CODE];
  int err_symbols = 0;
  int err_uncorrectable = 0;
  int err_miss_correction = 0;
  int err_noerror = 0;
  int err_bits = 0;
  int err_noerrpkts = 0;
  int err_after_rs = 0;
  int packet_count = 0;
  int err_syndrome = 0;

  /* initialize RS library by rs_init(n, k) */
  if (rs_init(RS_CODE, RS_MESG)) {
    fprintf(stderr, "rs_init() return error\n");
    exit(1);
  }

#if 0
  struct timeval t0, t1;

  for (i = 0; i < RS_MESG; i++) {
    code1[i] = rand() & 0xff;
  }

#define RS_TIMES 10000

  gettimeofday(&t0, NULL);
  for (i = 0; i < RS_TIMES; i++) {
    if (rs_encode(code1) < 0) {
      printf("rs_encode() return error\n");
      exit(1);
    }
  }
  gettimeofday(&t1, NULL);

  t1.tv_sec -= t0.tv_sec;
  if ((t1.tv_usec -= t0.tv_usec) < 0) {
    t1.tv_usec += 1000000; /* 1sec == 1000000usec */
    --t1.tv_sec;
  }

  printf("rs(%d, %d) encode %d times %d.%06d sec\n",
	 RS_CODE, RS_MESG, RS_TIMES,
	 (int)t1.tv_sec, (int)t1.tv_usec);

  gettimeofday(&t0, NULL);
  for (j = 0; j < RS_TIMES; j++) {
    for (i = 0; i < RS_CODE; i++) {
      code2[i] = code1[i];
    }
    for (i = 0; i < RS_PARI / 2; i++) {
      code2[rand() % RS_CODE] ^= rand() & 0xff; /* add error */
    }
    for (i = 0; i < RS_CODE; i++) {
      code3[i] = code2[i];
    }
    if ((i = rs_decode(code3)) < 0) {
      printf("rs_decode(): %d return error\n", i);
      for (i = 0; i < RS_CODE; i++) {
	if (code1[i] != code2[i]) {
	  printf("code1[%d] = %d, code2[%d] = %d\n", i, code1[i], i, code2[i]);
	}
      }
      for (i = 0; i < RS_CODE; i++) {
	if (code2[i] != code3[i]) {
	  printf("code2[%d] = %d, code3[%d] = %d\n", i, code2[i], i, code3[i]);
	}
      }
      exit(1);
    }
  }
  gettimeofday(&t1, NULL);

  t1.tv_sec -= t0.tv_sec;
  if ((t1.tv_usec -= t0.tv_usec) < 0) {
    t1.tv_usec += 1000000; /* 1sec == 1000000usec */
    --t1.tv_sec;
  }

  printf("rs(%d, %d) decode %d times %d.%06d sec\n",
	 RS_CODE, RS_MESG, RS_TIMES,
	 (int)t1.tv_sec, (int)t1.tv_usec);

  exit(0);

#else

  while (1) {
    printf("RS(%d, %d), bit error rate 1/%d\n", RS_CODE, RS_MESG, ERR_RATE);

  for (j = 0; j < LOOP_COUNT; ++j) {
    /* make original message */
    for (i = 0; i < RS_MESG; i++) 
      code1[i] = rand() & 0xff;

    if (rs_encode(code1)) {
      fprintf(stderr, "rs_encode() return error\n");
      exit(1);
    }

    /* add errors to each bit */
    k = 0; /* symbol errors */
    l = 0; /* bit errors */
    for (i = 0; i < RS_CODE; i++) {
      int e;
      int b;

      e = 0; /* error bit pattern */
      for (b = 1; b < 256; b <<= 1) {
	if (rand() < (RAND_MAX / ERR_RATE)) {
	  e |= b; /* one bit error */
	  ++err_bits;
	  ++l;
	}
      }
      if (e != 0) {
	++err_symbols;
	++k;
      }
      /* save error data */
      code3[i] = code2[i] = gf_add(code1[i], e);
      code4[i] = e;
    }

    if (k == 0) {
      ++err_noerrpkts;
    }

    /* decode received data */
    n = rs_decode(code3);

    if (n < 0) {
      ++err_uncorrectable;
      if (n == -2) ++err_after_rs;
      else if (n == -3) ++err_syndrome;
    }

    m = 0;
    for (i = 0; i < RS_CODE; ++i) {
      /* compare original and decode data */
      if (code1[i] != code3[i]) {
	/*printf("i:%d, code1[i]: %d, code2[i]: %d\n", i, code1[i], code2[i]);*/
	++m;
      }
    }
    if ((n >= 0) && (m > 0)) {
      ++err_miss_correction;

      for (i = 0; i < RS_CODE; i++) {
	if (code1[i] != code3[i]) {
	  printf("[%d]: code1: %d, code2: %d, code3: %d\n", i, code1[i], code2[i], code3[i]);
	}
      }
      printf("corrected symbols: %d\n", n);
      printf("symbol errors: %d\n", k);
      printf("bit errors: %d\n", l);

      poly_t g16, code_p;

      make_gp(&g16, RS_PARI);

      poly_clear(&code_p);
      for (i = 0; i < RS_CODE; i++) {
	code_p.coeff[(RS_CODE - 1) - i] = code3[i];
      }
      code_p.degree = RS_CODE - 1;
      poly_normalize(&code_p);

      poly_t qo, re;

      poly_div(&code_p, &g16, &qo, &re);

      printf("remainder\n");
      poly_print(&re);

      printf("encode, add error, decode\n");
      n = 0;
      for (i = 0; i < RS_CODE; i++) {
	int c;

	if (code1[i] != code3[i]) {
	  ++n;
	  c = '*';
	} else {
	  c = ' ';
	}
	printf("[%d] = %03o, %03o, %03o, %c\n", i, code1[i], code2[i], code3[i], c);
      };
      printf("error symbols: %d\n", n);

      printf("error values\n");
      for (i = 0; i < RS_CODE; i++) {
	int e = code4[i];
	if (e) printf("error[%d] = %03o\n", i, code4[i]);
      }

      exit(1);
    }
    if ((k > 0) && (m == 0)) ++err_noerror;
  }

  packet_count += LOOP_COUNT;

  printf("%d packets\n", packet_count);
  printf("%d miss corrected packets\n", err_miss_correction);
  printf("%d uncorrectable packets\n", err_uncorrectable);
  printf("%d error remain after RS decode\n", err_after_rs);
  printf("%d syndrome is zero, but parity error\n", err_syndrome);
  printf("%d RS decode with with no error packets\n", err_noerror);
  printf("%d no error packets\n", err_noerrpkts);
  printf("%d total error symbols\n", err_symbols);
  printf("%d total error bits\n", err_bits);
}
#endif
  return 0;
}
