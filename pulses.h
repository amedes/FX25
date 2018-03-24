#include "rmt.h"

/* convert from data bytes to pulses */
int make_ax25_pulses(rmt_item32_t *item32, int item32_size, const char data[], int data_len);
int make_fx25_pulses(rmt_item32_t *item32, int item32_size, const char data[], int data_len);
