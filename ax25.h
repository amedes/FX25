#ifndef __AX25_H__
#define __AX25_H__

#include <stdint.h>

int ax25_fcs(uint8_t packet[], int packet_len);
int ax25_count_bit_length(uint8_t packet[], int length);

#endif /* __AX25_H__ */
