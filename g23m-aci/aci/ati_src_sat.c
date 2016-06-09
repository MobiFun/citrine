/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  ATI_SRC_SATC
+----------------------------------------------------------------------------- 
|  Copyright 2002 Texas Instruments Berlin, AG 
|                 All rights reserved. 
| 
|                 This file is confidential and a trade secret of Texas 
|                 Instruments Berlin, AG 
|                 The receipt of or possession of this file does not convey 
|                 any rights to reproduce or disclose its contents or to 
|                 manufacture, use, or sell anything it may describe, in 
|                 whole, or in part, without the specific written consent of 
|                 Texas Instruments Berlin, AG. 
+----------------------------------------------------------------------------- 
|  Purpose :  .
+----------------------------------------------------------------------------- 
*/ 

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#ifdef SIM_TOOLKIT

#ifndef ATI_SRC_SAT_C
#define ATI_SRC_SAT_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#ifdef UART
#include "dti.h"      /* functionality of the dti library */
#endif

#include "aci_lst.h"

#include "ati_src_sat.h"
#include "psa.h"
#include "psa_cc.h"
#include "psa_sat.h"
#include "cmh.h"

#include "aci_mem.h"

#ifdef UART
#include "dti_conn_mng.h"
#include "psa_uart.h"
#endif

#include "ati_io.h"

#include "ati_int.h"

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/
EXTERN T_ACI_LIST *ati_src_list;

/*==== VARIABLES ==================================================*/
USHORT sat_out_buf_len = 0;
UBYTE  *sat_out_buf    = NULL;

UBYTE         run_at_id = 0xFF;              /* srce id of source where run at cmd is processed */

/*==== FUNCTIONS ==================================================*/
EXTERN USHORT format_output (UBYTE *src, USHORT src_len, UBYTE *dst,
                             T_ATI_OUTPUT_TYPE output_type);

/*
+----------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  :                      |
|                                 ROUTINE : sat_buffer_output    |
+----------------------------------------------------------------+

  PURPOSE : Buffer messages for RUN AT. Send buffered messages with
            confirm respond later
*/
LOCAL void sat_buffer_output(T_ACI_SAT_TERM_RESP resp_data)
{
  UBYTE *new_buf;
  UBYTE *old_buf;
  
  TRACE_FUNCTION("sat_buffer_output");

  if ( sat_out_buf EQ NULL ) /* first message to buffer */
  {
    ACI_MALLOC(sat_out_buf, resp_data.at_resp_count +1);
  }
  else 
  {
    /* enlarge buffer */
    old_buf = sat_out_buf;
    ACI_MALLOC(new_buf, sat_out_buf_len + resp_data.at_resp_count +1);
    memcpy(new_buf, sat_out_buf, sat_out_buf_len);
    sat_out_buf = new_buf;
    ACI_MFREE(old_buf);
  }

  /* add new message text to the end of the output buffer */
  memcpy(sat_out_buf + sat_out_buf_len, 
         resp_data.at_resp, resp_data.at_resp_count);
  sat_out_buf_len += resp_data.at_resp_count;
  sat_out_buf[sat_out_buf_len] = 0;
  
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  :                         |
|                                 ROUTINE :sat_send_buffered_output |
+-------------------------------------------------------------------+

  PURPOSE : for RUN AT : send buffered messages with confirm respond
*/
LOCAL void sat_send_buffered_output(T_ACI_SAT_TERM_RESP resp_data)
{
  TRACE_FUNCTION("sat_send_buffered_output");

  /* copy confirm respond to end of output buffer */
  sat_buffer_output(resp_data);

  /* send buffered respond */
  resp_data.at_resp       = sat_out_buf;
  resp_data.at_resp_count = sat_out_buf_len;
  TRACE_EVENT_P2("Buffer (%d): %s", resp_data.at_resp_count, (char*) resp_data.at_resp);
  psaSAT_SendTrmResp( RSLT_PERF_SUCCESS, &resp_data );

  /* free buffered output */
  ACI_MFREE(sat_out_buf);
  sat_out_buf     = NULL;
  sat_out_buf_len = 0;

}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  :                         |
|                                 ROUTINE : sat_src_result_cb       |
+-------------------------------------------------------------------+

  PURPOSE : 
*/


GLOBAL void sat_src_result_cb ( UBYTE             src_id,
                                T_ATI_OUTPUT_TYPE	output_type,
                                UBYTE             *output,
                                USHORT            output_len)
{
  UBYTE *formated_response;
  T_ACI_SAT_TERM_RESP resp_data;
  T_ATI_SRC_PARAMS *src_params;
//  T_at_resp *at_response;
  
  psaSAT_InitTrmResp( &resp_data );

  TRACE_FUNCTION("sat_src_result_cb()");

  /* search for SAT source Id */
  src_params = find_element (ati_src_list, src_id, search_ati_src_id);
  if (src_params EQ NULL)
  {
    TRACE_EVENT ("[ERR] source ID not found");
    return;
  }
  else if( src_params->src_type NEQ ATI_SRC_TYPE_SAT)
  {
    TRACE_EVENT ("[ERR] source ID is not from type SAT");
    return;
  }
  else if( !IS_INDICATION_OUTPUT(output_type) )
  {
    /* size of response goes up to MAX_CMD_AT_LEN
    The rest will be truncated */
    ACI_MALLOC(formated_response, MAX_CMD_AT_LEN+5);
    if( output_len > MAX_CMD_AT_LEN )
    {
      output_len = MAX_CMD_AT_LEN;
    }

    resp_data.at_resp_count  
             = format_output(output, output_len, formated_response, output_type);
    resp_data.at_resp = formated_response;


    if ( resp_data.at_resp NEQ NULL AND !IS_CONFIRM_OUTPUT(output_type))
    { /* Buffer messages for RUN AT. Send buffered messages with
         confirm respond later */

      TRACE_EVENT("buffer OUTPUT");
      sat_buffer_output(resp_data);
    }
    else if ( resp_data.at_resp NEQ NULL AND sat_out_buf_len > 0 )
    { /* for RUN AT: send buffered messages with confirm respond */
      sat_send_buffered_output(resp_data);
    }
    else
    { /* send respond */
      psaSAT_SendTrmResp( RSLT_PERF_SUCCESS, &resp_data );
    }

    ACI_MFREE(formated_response);
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  :                         |
|                                 ROUTINE : sat_new_source          |
+-------------------------------------------------------------------+

  PURPOSE : Create a new source for ATI with the corresponding struct.
*/

GLOBAL void sat_new_source( void )
{
/*#ifdef _SIMULATION_
  static BOOL already_done = FALSE;
  static UBYTE id;

  TRACE_FUNCTION("sat_new_source: Win 32");

  if( already_done )
  {
    TRACE_EVENT_P1("SAT related id: %d", id);
    satShrdPrm.run_at_id = id;
    return;
  }
  already_done = TRUE;
#else
  UBYTE id;

  TRACE_FUNCTION("sat_new_source: target");
#endif*/
  /* _SIMULATION_ */
  run_at_id = ati_init(ATI_SRC_TYPE_SAT, sat_src_result_cb, NULL);
  if (run_at_id<CMD_SRC_MAX)
  {
    if (run_at_id==CMD_SRC_LCL)
      aci_cmd_src_mode_set(run_at_id,CMD_MODE_ACI);
    else
      aci_cmd_src_mode_set(run_at_id,CMD_MODE_ATI);
  }

  TRACE_EVENT_P1 ("sat_new_source: SAT srcId=%d", run_at_id);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_DTIR                |
|                                 ROUTINE : sat_src_proc_chars      |
+-------------------------------------------------------------------+

  PURPOSE : 
*/

GLOBAL BOOL sat_src_proc_chars ( UBYTE *chars )
{
  T_ATI_SRC_PARAMS *src_params = NULL;
  T_ACI_SAT_TERM_RESP resp_data;
  
  psaSAT_InitTrmResp( &resp_data );

  TRACE_FUNCTION ("sat_src_proc_chars()");

  /* search for SAT source Id */
  src_params = find_element (ati_src_list, run_at_id, search_ati_src_id);
  if (src_params EQ NULL)
  {
    TRACE_EVENT ("[ERR] ati_execute: no SAT source ID present");
    psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
    return(FALSE);
  }
  
  /* do not echo AT-cmds which are sent by SAT */
  ati_user_output_cfg[src_params->src_id].atE = 0;

  return(ati_execute_sat_cmd (src_params, chars, MAX_CMD_AT_LEN));
}



#endif /* SIM_TOOLKIT */
