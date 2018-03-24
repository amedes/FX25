#include <stdio.h>
#include <stdint.h>

#include "rmt.h"

#define BUF_SIZE 1024
#define ITEM32_SIZE (BUF_SIZE*8+1)
#define TXD_DURATION 833 // 1/1200 sec

void make_item32(char buf[], int len)
{
  rmt_item32_t item32[ITEM32_SIZE];
  rmt_item16_t *item16 = (rmt_item16_t *)item32;
  int i;
  int j;
  int c;
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

  if (len > BUF_SIZE) return;

  cnt = 0;
  index = 0;
  duration = TXD_DURATION;
  level = 1; // assume idle state is MARK(1)
  for (i = 0; i < len; i++) {
    c = buf[i];
    for (j = 0; j < 8; j++) {
      bit = c & 1;
      du = ((cnt++ % 3) == 1) ? TXD_DURATION + 1 : TXD_DURATION;

      if (bit == level) {
	duration += du;
      } else {
	item16[index].duration = duration;
	item16[index].level = level;
	index++;
	level = bit;
	duration = du;
      }
      c >>= 1;
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
  fwrite((char *)&header, sizeof(header), 1, stdout);

  fwrite((char *)item32, sizeof(item32[0]), size, stdout);
}

int main(int argc, char *argv[])
{
  char buf[BUF_SIZE];
  int len;

  while ((len = fread(buf, sizeof(char), BUF_SIZE, stdin)) > 0) {
    make_item32(buf, len);
  }

  return 0;
}
