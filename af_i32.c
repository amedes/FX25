#include <stdio.h>
#include <stdint.h>

#include "modem.h"
#include "rmt.h"
#include "afio.h"

#define BUF_SIZE 1024
#define ITEM32_SIZE (BUF_SIZE*8+1)

#define RMT_TICK (APB_CLK / RMT_CLK_DIV)

#define TXD_DURATION (RMT_TICK / BAUD_RATE) // 1/1200 us
#define TXD_DURATION_MARK  (1000*1000 / BAUD_RATE_MARK) // 1/1200 us
#define TXD_DURATION_SPACE (1000*1000 / BAUD_RATE_SPACE) // 1/1200 us

/* Euclidean Algorithm */
int euclid(int a, int b)
{
  int dividend;
  int divisor;
  int remainder;
  int quotient;

  if (a <= 0 || b <= 0) return -1;

  if (a > b) {
    dividend = a;
    divisor = b;
  } else {
    dividend = b;
    divisor = a;
  }
  remainder = dividend % divisor;
  if (remainder == 0) return divisor;

  return euclid(divisor, remainder);
}

void make_item32(FILE *fp, char buf[], int bit_length)
{
  rmt_item32_t item32[ITEM32_SIZE];
  rmt_item16_t *item16 = (rmt_item16_t *)item32;
  int i;
  int j;
  int size;
  int index;
  struct {
    uint32_t size:16;
    uint32_t id:16;
  } header;
  int level;
  int duration;
  int bit;
  int cnt;
  int du;
  int gcd, dividend;
  static int divisor = 0, remainder = 0;

  /* initialize pulse length calculation parameter */
  if (divisor == 0) {
    gcd = euclid(RMT_TICK, BAUD_RATE); // calculation Great Common Divider
    dividend = RMT_TICK / gcd; // 1 bit time is dividend / divisor
    divisor = BAUD_RATE / gcd;
    if (divisor > 1) {	   // need fraction processing
      remainder = dividend % divisor;
    }
    fprintf(stderr, "af_i32: gcd: %d, divisor: %d, remainder: %d\n", gcd, divisor, remainder);
  }

  if (bit_length > BUF_SIZE * 8) return;

  cnt = remainder / 2; // prepair for DDA
  index = 0;
  duration = 0;
  level = 1; // assume idle state is MARK(1)

  for (i = 0; i < bit_length; i++) {
    bit = (buf[i / 8] >> (i % 8)) & 1;
    du = TXD_DURATION;

    // fractional correction using DDA
    if (divisor != 1) {
      if (cnt >= divisor) {
	du++; // increment frequency is remainder / divisor
	cnt -= divisor;
      }
      cnt += remainder;
    }

    if (bit == level) {
      duration += du;
    } else {
      while (duration > 0) {
	int d = (duration < 32768) ? duration : 32767;
	item16[index].duration = d;
	item16[index].level = level;
	index++;
	duration -= d;
      }
      level = bit;
      duration = du;
    }
  }
  if (level != 1) { // not idle state
    item16[index].duration = duration;
    item16[index].level = level;
    index++;
  }
    
  /* end mark */
  item16[index].duration = 0;
  item16[index].level = 0;
  index++;

  size = (index + 1) / 2;

#define ID_TX_DATA 0x0001

  /* header */
  header.id = ID_TX_DATA;
  header.size = size;
  fwrite((char *)&header, sizeof(header), 1, fp);

  fwrite((char *)item32, sizeof(item32[0]), size, fp);
}

int main(int argc, char *argv[])
{
  FILE *ifp = stdin;
  FILE *ofp = stdout;
  char buf[BUF_SIZE];
  int bit_length;

  while ((bit_length = afio_read(ifp, buf, BUF_SIZE)) >= 0) {
    if (bit_length > 0) {
      make_item32(ofp, buf, bit_length);
    }
  }

  return 0;
}
