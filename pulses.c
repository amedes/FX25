#include <stdio.h>
#include <stdint.h>
#include <strings.h>

#include "rmt.h"
#include "rs.h"

/* calculate time of pulse */
static void make_pulse(rmt_item16_t *item16, int du, int lv)
{
    item16->duration = (lv ? RMT_DURATION_MARK : RMT_DURATION_SPACE) * du;
    item16->level = lv;
}

/* make pulses from data byte */
static int byte_to_pulse(rmt_item16_t *item16, int item16_size, uint8_t data, int *dup, int *lvp, int bsflag)
{
  int i = 0; // index of item16[]
    uint8_t b;
    int du = *dup;
    int lv = *lvp; 

    for (b = 1; b != 0; b <<= 1) { // scan data byte, LSb first
      if (b & data) { // check data bit
	  // send 1
	    du++;
	    if (bsflag && (du >= 6)) { // insert 0 (bit stuffing)
		if (i >= item16_size) break;
		make_pulse(&item16[i++], du, lv);
		du = 1; // bit stuffing 0
		lv = !lv;
	    }
	} else {
	  // send 0
	    if (i >= item16_size) break;
	    make_pulse(&item16[i++], du, lv); // make '1' pulse
	    du = 1; // next '0' pulse duration
	    lv = !lv; // invert level
	}
    }
    *dup = du;
    *lvp = lv;

    return i; // number of item16
}

#if 0
#define MAX_PACKET_SIZE 1024		// maximum packet size in byte */

#define ITEM32_SIZE (MAX_PACKET_SIZE*8/2) // 32bit holds 2 pulse info
#define ITEM16_SIZE (ITEM32_SIZE/2)	// 16bit holds 1 pulse info

static rmt_item32_t item32[ITEM32_SIZE];
#endif

/*
 * make NRZI pulses from packet data
 */
int make_ax25_pulses(rmt_item32_t *item32, int item32_size, char data[], int data_len)
{
    int i;
    rmt_item16_t *item16 = (rmt_item16_t *)item32;
    int item16_size = item32_size * 2;
    int du = 1; // duration, always start 0 data bit
    int lv = 1; // level, idle level is 1
    int idx = 0; // index of item16[] to be stored next pulse
    int item32_len;

#define AX25_FLAG	0x7e
#define NUM_PREAMBLE	1 // 6.67ms x N
#define NUM_POSTAMBLE	1 // at least 1 

    /* preamble, bit stuffing off */
    for (i = 0; i < NUM_PREAMBLE; i++) {
	idx += byte_to_pulse(&item16[idx], item16_size - idx, AX25_FLAG, &du, &lv, 0);
    }

    /* packet data, bit stuffing on */
    for (i = 0; i < data_len; i++) {
    	/* char data */
	 idx += byte_to_pulse(&item16[idx], item16_size - idx, data[i], &du, &lv, 1);
    }

    /* postamble, bit stuffing off */
    for (i = 0; i < NUM_POSTAMBLE; i++) {
        idx += byte_to_pulse(&item16[idx], item16_size - idx, AX25_FLAG, &du, &lv, 0);
    }

    // end mark
    if (idx < item16_size) {
      item16[idx].duration = 0; // 0 means end of pulses
      item16[idx].level = 0;
      idx++;
    }
    item32_len = (idx + 1) / 2;

    return item32_len; // return value is number of item32
}

#define RS_CODE		255
#define MAX_RS_INFO	239

/* make bit stuffed data */
int make_bit_stuffing(uint8_t *bit_stuff, uint8_t *data, int size)
{
  int stuff_offset_bit;
  int byte_offset = 0;
  uint32_t bit_data = 0;
  int bit1_length = 0;
  int i;
  int d;
  uint8_t b;
  
  /* clear buffer */
  bzero(bit_stuff, RS_CODE);

  stuff_offset_bit = 0;
  bit_stuff[stuff_offset_bit / 8] = AX25_FLAG;
  stuff_offset_bit += 8;

  for (i = 0; i < size * 8; i++) { // bit offset
    if (data[i / 8] & (1 << (i % 8))) { // find 1
      bit_stuff[stuff_offset_bit / 8] |= (1 << (stuff_offset_bit % 8));
      bit1_length++;
      if (bit1_length >= 5) { // need bit stuffing
	stuff_offset_bit++;
	bit1_length = 0;
      }
    } else { // find 0
      bit1_length = 0; // clear bit '1' counter
    }
    stuff_offset_bit++;
    if (stuff_offset_bit > MAX_RS_INFO * 8) return -1; // packet too large
  }

  if (stuff_offset_bit + 8 > MAX_RS_INFO * 8) return -1; // no room for AX25_FLAG

  /* add flag */
  d = AX25_FLAG;
  for (i = 0; i < 8; i++) {
    if (d & (1 << (i % 8))) { // find 1
      bit_stuff[stuff_offset_bit / 8] |= (1 << (stuff_offset_bit % 8));
    }
    stuff_offset_bit++;
  }

  return stuff_offset_bit / 8;
}


uint64_t correlation_tag[] = {
  0x566ED2717946107E,	// Tag_00 Reserved
  0xB74DB7DF8A532F3E,	// Tag_01 RS(255, 239)
};

static uint8_t rs_buf[RS_CODE];

int make_fx25_pulses(rmt_item32_t *item32, int item32_size, char data[], int data_len)
{
    int i;
    rmt_item16_t *item16 = (rmt_item16_t *)item32;
    int item16_size = item32_size * 2;
    int du = 1; // duration, always start 0
    int lv = 1; // level, idle level is 1
    int index16 = 0; // index of item16[] to be stored next pulse
    int item32_len;
    int rs_info_size;
    int n;

#define AX25_FLAG	0x7e
#define FX25_PREAMBLE	4 // 6.67ms x N
#define FX25_POSTAMBLE	1 // at least 1 

    /* preamble, bit stuffing off */
    for (i = 0; i < FX25_PREAMBLE; i++) {
	n = byte_to_pulse(&item16[index16], item16_size - index16, AX25_FLAG, &du, &lv, 0);
	index16 += n;
    }

    /* correlation tag */
    for (i = 0; i < sizeof(correlation_tag[1]); i++) {
      uint8_t *tag = (uint8_t *)&correlation_tag[1]; // RS(255, 239)

      n = byte_to_pulse(&item16[index16], item16_size - index16, tag[i], &du, &lv, 0);
      index16 += n;
    }

    /* clear buf */
    bzero(rs_buf, RS_CODE);

    /* make bit stuffing data (with AX25 flags) */
    rs_info_size = make_bit_stuffing(rs_buf, data, data_len);
#if 0
    for (i = 0; i < rs_info_size; i++) {
      printf("%02x ", rs_buf[i]);
      if ((i % 16) == 15) printf("\n");
    }
    printf("\nrs_info_size: %d\n", rs_info_size);
#endif
    if (rs_info_size < 0) {
      printf("AX25 packet too large\n");
      return;
    }

    /* add Reed-Solomon FEC */
    if (rs_encode(rs_buf) < 0) {
      printf("rs_encode error\n");
      return 0;
    }

#if 0
    printf("RS parity\n");
    for (i = MAX_RS_INFO; i < RS_CODE; i++) {
      printf("%02x ", rs_buf[i]);
    }
    printf("\n");
#endif

    /* make packet data */
    for (i = 0; i < RS_CODE; i++) {
    	/* char data */
	 n = byte_to_pulse(&item16[index16], item16_size - index16, rs_buf[i], &du, &lv, 0);
	 index16 += n;
    }

    /* postamble, bit stuffing off */
    for (i = 0; i < FX25_POSTAMBLE; i++) {
      n = byte_to_pulse(&item16[index16], item16_size - index16, AX25_FLAG, &du, &lv, 0);
      index16 += n;
    }

    // end mark
    if (index16 < item16_size) {
      item16[index16].duration = 0; // 0 means end of pulses
      item16[index16].level = 0;
      index16++;
    }
    item32_len = (index16 + 1) / 2;

    return item32_len; // return value is number of item32
}
