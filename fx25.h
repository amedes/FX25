#ifndef __FX25_H__
#define __FX25_H__

#include <stdint.h>

#define FX25_MAX_INFO 237 // 239 - 2 (AX.25 flag * 2)
#define FX25_MAX_BITS (FX25_MAX_INFO * 8)

/* make test packet */
int fx25_make_random_packet(uint8_t *buf, int buf_size);

/* search FX25 tag */
int fx25_search_tag(uint64_t *correlation_tag, int data_bit);

#endif /* __FX25_H__ */
