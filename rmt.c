#include "esp_log.h"
#include "driver/rmt.h"

#include "cw.h"

#define RMT_TX_CHANNEL RMT_CHANNEL_0
#define RMT_TX_GPIO 12

#define RMT_RX_CHANNEL RMT_CHANNEL_1
#define RMT_RX_GPIO 14

#define RMT_CLK_DIV 163
#define RMT_DURATION ((80*1000*1000 / RMT_CLK_DIV + 600) / 1200) // 1200bps

/*
 * Initialize the RMT Tx channel
 */
void rmt_tx_init(void)
{
    rmt_config_t config = {
    	.rmt_mode = RMT_MODE_TX,
    	.channel = RMT_TX_CHANNEL,
    	.gpio_num = RMT_TX_GPIO,
    	.mem_block_num = 1,
    	.tx_config.loop_en = 0,
    	.tx_config.carrier_en = 0,
    	.tx_config.idle_output_en = 1,
    	.tx_config.idle_level = 1,
    	.clk_div = RMT_CLK_DIV
    };
    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
}

static void make_pulse(rmt_item16_t *item16, int du, int lv)
{
    item16->duration = RMT_DURATION * du;
    item16->level = lv;
#if 0
    printf("%d ", du);
    if (du >= 7) printf("\n");
#endif
}

/* make pulses from byte */
static int byte_to_pulse(rmt_item16_t *item16, int size, uint8_t c, int *dup, int *lvp, int bsflag)
{
    int i = 0;
    uint8_t b;
    int du = *dup;
    int lv = *lvp; 

    for (b = 1; b != 0; b <<= 1) { // LSB first
	if (b & c) { // send 1
	    du++;
	    if (bsflag && (du >= 6)) { // bit stuffing
		if (i >= size) break;
		make_pulse(&item16[i++], du, lv);
		du = 1;
		lv = !lv;
	    }
	} else { // send 0
	    if (i >= size) break;
	    make_pulse(&item16[i++], du, lv);
	    du = 1;
	    lv = !lv;
	}
    }
    *dup = du;
    *lvp = lv;

    return i;
}

#define ITEM32_SIZE (512*8/2)		// 32bit holds 2 pulse info
#define ITEM16_SIZE (ITEM32_SIZE/2)	// 16bit holds 1 pulse info

static rmt_item32_t item32[ITEM32_SIZE];

/*
 * convert to NRZI
 */
void send_packet(char data[], int len)
{
    int i;
    rmt_item16_t *item16 = (rmt_item16_t *)item32;
    int du = 1; // always start 0
    int lv = 1; // idle level is 1
    int idx = 0;
    int item32_len;

#define AX25_FLAG	0x7e
#define NUM_PREAMBLE	1 // 6.67ms x N
#define NUM_POSTAMBLE	1 // at least 1 

    /* preamble */
    for (i = 0; i < NUM_PREAMBLE; i++) {
	idx += byte_to_pulse(&item16[idx], ITEM16_SIZE - idx, AX25_FLAG, &du, &lv, false);
    }

    /* packet data */
    for (i = 0; i < len; i++) {
    	/* char data */
	 idx += byte_to_pulse(&item16[idx], ITEM16_SIZE - idx, data[i], &du, &lv, true);
    }

    /* postamble */
    for (i = 0; i < NUM_POSTAMBLE; i++) {
        idx += byte_to_pulse(&item16[idx], ITEM16_SIZE - idx, AX25_FLAG, &du, &lv, false);
    }

    // end mark
    if (idx < ITEM16_SIZE) {
        item16[idx].duration = 0;
    	item16[idx].level = 0;
	idx++;
    }

#if 1
    item32_len = (idx + 1) / 2;
    ESP_ERROR_CHECK(rmt_write_items(RMT_TX_CHANNEL, item32, item32_len, true));
#else
    i = 0;
    while ((du = item16[i].duration) > 0) {
	cw_bell202(item16[i].level);
	ets_delay_us(du / 409 * 830); // delay d/1200 sec
	i++;
    }
    cw_bell202(1); // idle state
#endif
}
