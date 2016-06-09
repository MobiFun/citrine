/*
 * This module is a FreeCalypso addition; it contains code implementing
 * our custom voice TCH rerouting feature.
 */

#include "config.h"
#include "l1_confg.h"
#include "l1_types.h"
#include "sys_types.h"
#include <string.h>
#include "l1_const.h"
#include "l1_defty.h"
#include "l1_varex.h"
#include "l1_trace.h"
#include "tch_feature.h"

T_RVT_USER_ID tch_reroute_rvt_id;
BOOL tch_reroute_downlink;

void tch_send_downlink_bits(API *dsp_buffer)
{
	T_RVT_BUFFER buf;
	T_RVT_RET rc;
	UINT8 *dp;
	UWORD16 apiword;
	int i;

	rc = rvt_mem_alloc(tch_reroute_rvt_id, 41, &buf);
	if (rc != RVT_OK)
		return;
	dp = buf;
	*dp++ = TCH_DLBITS_IND;
	for (i = 0; i < 20; i++) {
		apiword = dsp_buffer[i];
		*dp++ = apiword >> 8;
		*dp++ = apiword;
	}
	rvt_send_trace_no_cpy(buf, tch_reroute_rvt_id, 41, RVT_BINARY_FORMAT);
}

static void send_tch_ulbits_conf()
{
	T_RVT_BUFFER buf;
	T_RVT_RET rc;

	rc = rvt_mem_alloc(tch_reroute_rvt_id, 1, &buf);
	if (rc == RVT_OK) {
		buf[0] = TCH_ULBITS_CONF;
		rvt_send_trace_no_cpy(buf, tch_reroute_rvt_id, 1,
					RVT_BINARY_FORMAT);
	}
}

#define	UPLINK_QUEUE_SIZE	5
#define	WORDS_PER_ENTRY		17

static UWORD16 uplink_data[UPLINK_QUEUE_SIZE][WORDS_PER_ENTRY];
static volatile int ul_read_ptr, ul_write_ptr;

void tch_substitute_uplink(API *dsp_buffer)
{
	int read_ptr;
	int i;

	read_ptr = ul_read_ptr;
	if (read_ptr == ul_write_ptr) {
		/* no uplink substitution */
		l1s_dsp_com.dsp_ndb_ptr->d_tch_mode &= ~B_PLAY_UL;
		return;
	}
	for (i = 0; i < WORDS_PER_ENTRY; i++)
		dsp_buffer[i+3] = uplink_data[read_ptr][i];
	// Fill data block Header...
	dsp_buffer[0] = (1 << B_BLUD);		// 1st word: Set B_BLU bit.
	dsp_buffer[1] = 0;			// 2nd word: cleared.
	dsp_buffer[2] = 0;			// 3rd word: cleared.
	l1s_dsp_com.dsp_ndb_ptr->d_tch_mode |= B_PLAY_UL;
	/* advance the read pointer and send TCH_ULBITS_CONF */
	read_ptr++;
	if (read_ptr >= UPLINK_QUEUE_SIZE)
		read_ptr = 0;
	ul_read_ptr = read_ptr;
	send_tch_ulbits_conf();
}

static void handle_tch_ulbits_req(T_RVT_BUFFER pkt)
{
	int write_ptr, write_next, i;
	UINT8 *sp;

	write_ptr = ul_write_ptr;
	write_next = write_ptr + 1;
	if (write_next >= UPLINK_QUEUE_SIZE)
		write_next = 0;
	if (write_next == ul_read_ptr)	/* queue full */
		return;
	sp = pkt + 1;
	for (i = 0; i < WORDS_PER_ENTRY; i++) {
		uplink_data[write_ptr][i] = (sp[0] << 8) | sp[1];
		sp += 2;
	}
	ul_write_ptr = write_next;
}

static void handle_tch_config_req(T_RVT_BUFFER pkt)
{
	UWORD8 config;
	T_RVT_BUFFER buf;
	T_RVT_RET rc;

	config = pkt[1] & 0x01;
	tch_reroute_downlink = config;

	/* send TCH_CONFIG_CONF response */
	rc = rvt_mem_alloc(tch_reroute_rvt_id, 2, &buf);
	if (rc == RVT_OK) {
		buf[0] = TCH_CONFIG_CONF;
		buf[1] = config;
		rvt_send_trace_no_cpy(buf, tch_reroute_rvt_id, 2,
					RVT_BINARY_FORMAT);
	}
}

/*
 * The following function is the callback registered with RVT; it gets
 * called in RVT HISR context.
 */
static void tch_rvt_input_callback(T_RVT_BUFFER pkt, UINT16 pktlen)
{
	if (pktlen < 1)
		return;
	switch (pkt[0]) {
	case TCH_CONFIG_REQ:
		if (pktlen != 2)
			return;
		handle_tch_config_req(pkt);
		break;
	case TCH_ULBITS_REQ:
		if (pktlen < 34)
			return;
		handle_tch_ulbits_req(pkt);
		break;
	}
}

void feature_tch_reroute_init()
{
	rvt_register_id("TCH", &tch_reroute_rvt_id, tch_rvt_input_callback);
	tch_reroute_downlink = FALSE;
	ul_read_ptr = 0;
	ul_write_ptr = 0;
}
