#ifndef __FCS_H__
#define __FCS_H__

#include <stdint.h>

#define GENERATOR_POLY 0x10811 /* G(x) = 1 + x^5 + x^12 + x^16 */
#define GOOD_CRC 0xf0b8 /* always this value if CRC is correct */

uint32_t fcs_calc(uint8_t packet[], int length);

#endif /* __FCS_H__ */
