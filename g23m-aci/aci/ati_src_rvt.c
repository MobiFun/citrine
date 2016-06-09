/*
 * This module is a FreeCalypso addition.  Here we are going to implement
 * a mechanism for passing AT commands and responses over RVTMUX.
 */

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

/* includes copied from ati_cmd.c */

#undef DUMMY_ATI_STRINGS

#include "aci_all.h"

#include "dti.h"      /* functionality of the dti library */
#include "line_edit.h" /* SKA 2002-09-05 */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_io.h"
#include "aci_cmd.h"
#include "l4_tim.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "psa.h"
#include "cmh.h"

#include "aci_lst.h"
#include "dti_conn_mng.h"
#ifdef UART
#include "psa_uart.h"
#endif
#include "ati_io.h"
#include "aci_mem.h"

#ifdef SIM_TOOLKIT
#include "ati_src_sat.h"
#include "psa_cc.h"
#include "psa_sat.h"
#endif /* SIM_TOOLKIT */

#ifdef GPRS
#include "gaci_cmh.h"
#include "gaci_cmd.h"
#endif /* GPRS */

#include "aci_prs.h"


#ifndef _SIMULATION_
#ifdef UART
#include "cmh_uart.h"
#endif
#endif

#include "psa_sms.h"
#include "aci.h"
#include "ati_ext_mech.h"

#ifdef FF_ATI_BAT
#include "ati_bat.h"
#include "aci_bat.h"
#endif

EXTERN T_ACI_LIST *ati_src_list;

#include "../../riviera/rvt/rvt_gen.h"

static UBYTE rvt_src_id;
static T_RVT_USER_ID rvt_user_id;

/*
 * The following function is the callback registered with RVT; it gets
 * called in RVT HISR context.  We allocate an ACI_CMD_REQ primitive,
 * copy the received string into it and post it to ACI to be processed
 * in our own task context.
 */
GLOBAL void ati_src_rvt_input_callback (T_RVT_BUFFER in_str, UINT16 in_str_len)
{
	PALLOC (aci_cmd_req, ACI_CMD_REQ);

	aci_cmd_req->cmd_src = rvt_src_id;
	aci_cmd_req->cmd_len = in_str_len;
	bcopy(in_str, aci_cmd_req->cmd_seq, in_str_len);

	PSENDX (ACI, aci_cmd_req);
}

/*
 * The following function is called from aci_aci.c when an ACI_CMD_REQ
 * primitive (sent by the previous function) has been received.
 */
GLOBAL BOOL ati_src_rvt_proc_cmd (T_ACI_CMD_REQ *aci_cmd_req)
{
    T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, rvt_src_id,
						 search_ati_src_id);

    ati_user_output_cfg[rvt_src_id].atE = 0;

    if (src_params->text_mode EQ CMD_MODE)
    {
      TRACE_FUNCTION ("ati_src_rvt_proc_cmd () CMD MODE");

      aci_cmd_req->cmd_seq[aci_cmd_req->cmd_len] = '\r';     /* make it V.25 ter compatible */
      aci_cmd_req->cmd_seq[(aci_cmd_req->cmd_len) + 1] = '\0';
      aci_cmd_req->cmd_len++;
    }
    else /* text has to be terminated by Ctrl-Z */
    {
      TRACE_FUNCTION ("ati_src_rvt_proc_cmd () TEXT MODE");

      aci_cmd_req->cmd_seq[aci_cmd_req->cmd_len] = 0x1a;     /* make it V.25 ter compatible */
      aci_cmd_req->cmd_seq[(aci_cmd_req->cmd_len) + 1] = '\0';
      aci_cmd_req->cmd_len++;
    }

    return (ati_execute (rvt_src_id,
                         aci_cmd_req->cmd_seq,
                         aci_cmd_req->cmd_len));
}

/*
 * The following function is the callback registered with ATI; it gets
 * called when ATI has something to send to the user.
 */
GLOBAL void ati_src_rvt_result_cb (UBYTE              src_id,
                                   T_ATI_OUTPUT_TYPE  output_type,
                                   UBYTE              *output,
                                   USHORT             output_len)
{
  TRACE_FUNCTION ("ati_src_rvt_result_cb ()");
  rvt_send_trace_cpy (output, rvt_user_id, output_len, RVT_ASCII_FORMAT);
}

/*
 * The following function is the other callback registered with ATI.
 */
GLOBAL void ati_src_rvt_line_state_cb (UBYTE                 src_id,
                                       T_ATI_LINE_STATE_TYPE line_state_type,
                                       ULONG                 line_state_param)
{
  TRACE_FUNCTION ("ati_src_rvt_line_state_cb ()");

  switch (line_state_type)
  {
    case ATI_LINE_STATE_OUTPUT_TYPE:
      TRACE_EVENT_P1 ("[DBG] ati_src_rvt_line_state_cb (): ATI_LINE_STATE_OUTPUT_TYPE = %d", line_state_param);
      break;

    case ATI_LINE_STATE_DCD:
      TRACE_EVENT_P1 ("[DBG] ati_src_rvt_line_state_cb (): ATI_LINE_STATE_DCD = %d", line_state_param);
      break;

    case ATI_LINE_STATE_RNG:  /* TODO */
    {
      T_IO_RING_PARAMS rng_params;

      memcpy (&rng_params, (T_IO_RING_PARAMS*)line_state_param,
              sizeof(T_IO_RING_PARAMS));

      TRACE_EVENT_P1 ("[DBG] ati_src_rvt_line_state_cb (): ATI_LINE_STATE_RNG = %d", rng_params.ring_stat);

      break;
    }

    default:
      TRACE_EVENT_P1 ("[WRN] ati_src_rvt_line_state_cb (): UNKNOWN line_state_type = %d", line_state_type);
      break;
  }
}

/*
 * The following function is called from ati_cmd_init() in ati_cmd.c
 * to register our mechanism.
 */
GLOBAL void ati_src_rvt_register (void)
{
  rvt_register_id ("AT", &rvt_user_id, ati_src_rvt_input_callback);
  rvt_src_id = ati_init (ATI_SRC_TYPE_TST,
                         ati_src_rvt_result_cb,
                         ati_src_rvt_line_state_cb);
  ati_switch_mode(rvt_src_id, ATI_CMD_MODE);
}
