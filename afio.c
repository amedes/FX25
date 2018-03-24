/*
  Asynchronous frame input output routine
*/
#include <stdio.h>
#include <stdint.h>

#include "afio.h"

int afio_read(FILE *fp, uint8_t *buf, int size)
{
  int c;
  int state;
  int index;
  int padding_bits;

  state = STATE_OUTFRAME;
  index = 0;
  padding_bits = 0; // 0 means last byte has full 8 bit information
  
  while ((c = getc(fp)) != EOF) {
    switch (state) {
    case STATE_OUTFRAME:
      if (c == FEND) {
	state = STATE_INFRAME;
      }
      break;

    case STATE_INFRAME:
      switch (c) {
      case FEND:
	return index * 8 - padding_bits;
	break;
      case FESC:
	state = STATE_ESCAPE;
	break;
      default:
	buf[index++] = c;
      }
      break;

    case STATE_ESCAPE:
      switch (c) {
      case TFEND:
	buf[index++] = FEND;
	break;
      case TFESC:
	buf[index++] = FESC;
	break;
      default:
	if (c >= 0x01 && c <= 0x07) padding_bits = c;
      }
      state = STATE_INFRAME;
    }

    if (index >= size) {
      return size * 8;
    }
  }

  if (index > 0) {
    return index * 8;
  }

  return EOF;
}

int afio_write(FILE *fp, uint8_t buf[], int size, int padding)
{
  int i;
  int c;
  int n;
  int padding_bits;

  if (size <= 0) return -1;

  if (padding > 0 && padding < 8) {
    padding_bits = padding;
  } else {
    padding_bits = 0;
  }

  putc(FEND, fp); // frame start
  n = 1;

  /* add padding information if padding is exist */
  if (padding_bits > 0) {
    putc(FESC, fp);
    putc(padding_bits, fp);
    n += 2;
  }

  for (i = 0; i < size; i++) {
    c = buf[i];
    switch (c) {
    case FEND:
      putc(FESC, fp);
      putc(TFEND, fp);
      n += 2;
      break;
    case FESC:
      putc(FESC, fp);
      putc(TFESC, fp);
      n += 2;
      break;
    default:
      putc(c, fp);
      n++;
    }
  }

  putc(FEND, fp); // frame end
  n++;

  return n;
}
