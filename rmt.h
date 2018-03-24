#ifndef __RMT_H__
#define __RMT_H__

#include <stdint.h>

#include "modem.h"

//#define BAUD_RATE 1200

#define APB_CLK (80*1000*1000) // 80MHz
#define RMT_CLK_DIV 16 // RMT clock divider

#define RMT_DURATION ((APB_CLK / RMT_CLK_DIV + BAUD_RATE/2) / BAUD_RATE) /* 1/1200 RMT ticks */
#define RMT_DURATION_MARK  ((80*1000*1000 / RMT_CLK_DIV + BAUD_RATE_MARK/2) / BAUD_RATE_MARK) /* 1/1200 RMT ticks */
#define RMT_DURATION_SPACE ((80*1000*1000 / RMT_CLK_DIV + BAUD_RATE_SPACE/2) / BAUD_RATE_SPACE) /* 1/1200 RMT ticks */

#if 0
typedef struct {
  unsigned int duration:15;
  unsigned int level:1;
} rmt_item16_t;

typedef struct {
  rmt_item16_t item16[2];
} rmt_item32_t;
#endif

/* from include/soc/rmt_struct.h */
typedef struct {
    union {
        struct {
            uint32_t duration0 :15;
            uint32_t level0 :1;
            uint32_t duration1 :15;
            uint32_t level1 :1;
        };
        uint32_t val;
    };
} rmt_item32_t;

typedef struct {
    union {
        struct {
            uint16_t duration :15;
            uint16_t level :1;
        };
        uint16_t val;
    };
} rmt_item16_t;

#endif /* __RMT_H__ */
