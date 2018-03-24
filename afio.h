#ifndef __AFIO_H__
#define __AFIO_H__

#include <stdint.h>

#define FEND 0xc0
#define FESC 0xdb
#define TFEND 0xdc
#define TFESC 0xdd

#define STATE_INFRAME 1
#define STATE_OUTFRAME 2
#define STATE_ESCAPE 3

int afio_read(FILE *fp, uint8_t *buf, int size);
int afio_write(FILE *fp, uint8_t *buf, int size, int padding);

#endif /* __AFIO_H__ */
