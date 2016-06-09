/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  ATI
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
|  Purpose :  
+----------------------------------------------------------------------------- 
*/ 

#ifndef ATI_IO_C
#define ATI_IO_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#undef DUMMY_ATI_STRINGS

#include "aci_all.h"
#include "aci.h"
#ifdef UART
#include "dti.h"      /* functionality of the dti library */
#endif
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_io.h"
#include "aci_cmd.h"
#include "l4_tim.h"

#include "aci_lst.h"

#ifdef UART
#include "dti_conn_mng.h"
#include "psa_uart.h"
#include "cmh_uart.h"
#endif

#include "ati_io.h"
#include "aci_mem.h"
#include "aci_fd.h"
#include "psa.h"
#include "cmh.h"

#include "cmh_cc.h"
#include "psa_cc.h"

#include "ati_int.h"

#if !defined (STRACE_LEN)
  #define STRACE_LEN 80
#endif

#define LIST_MAX_LENGTH 30

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : io_SendMessage     |
+--------------------------------------------------------------------+

  PURPOSE :
*/

GLOBAL void io_sendMessage(UBYTE srcId, CHAR *msg,
                           T_ATI_OUTPUT_TYPE output_type)
{
  T_ATI_SRC_PARAMS *src_params;

  src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  if (src_params EQ NULL)
  {
    TRACE_EVENT_P1 ("[ERR] io_sendMessage: srcId=%d not found", srcId);
    return;
  }
  if (IS_INDICATION_OUTPUT (output_type))
  {
    io_sendIndication(srcId, msg, output_type);
    return;
  }
  else if (!IS_ECHO_OUTPUT ( output_type))   /* ECHO_OUTPUT must not be terminated      */
  {                                          /*  with CRLF, see definition in ati_cmh.h */
    output_type |= ATI_END_CRLF_OUTPUT;
  }

  if (src_params->result_cb NEQ NULL)
  {
    src_params->result_cb (srcId, output_type,
                           (UBYTE*)msg, (USHORT)strlen (msg));
  }
  else
  {
    TRACE_EVENT_P1 ("[WRN] io_sendMessage: no result callback srcId=%d",
                    srcId);

    if ( (strlen (msg)) >= STRACE_LEN )
    {
      /* terminate traced string if too long */
      msg[STRACE_LEN-1] = '\0';
    }

    TRACE_EVENT_P1 ("%s", msg);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : io_SendMessageEx   |
+--------------------------------------------------------------------+

  PURPOSE :
*/

GLOBAL BOOL check_should_buffer_ind(T_ATI_SRC_PARAMS *src_params)
{  

  switch(src_params->buff_uns_mode)
  {
  case(NO_BUFF):
    break;

  case(BUFF_TYPING):
    if(src_params->cmd_state EQ CMD_TYPING)
    {
      TRACE_EVENT("check_should_buffer_ind(): cmd_state EQ CMD_TYPING");
      return(TRUE);
    }   
    break;

  case(BUFF_RUNNING):
    if(src_params->cmd_state NEQ CMD_IDLE )
    {
      TRACE_EVENT("check_should_buffer_ind(): cmd_state NEQ CMD_IDLE");
      return(TRUE);
    } 
    break;
  }
  return(FALSE); 
}

LOCAL void buffer_indication( T_ATI_SRC_PARAMS *src_params, 
                              CHAR *msg, 
                              T_ATI_OUTPUT_TYPE output_type )
{
  T_ATI_INDIC_BUFF *buff_indic_elem;
  CHAR *output;

  if(src_params->indication_buffer EQ NULL)
  {
    src_params->indication_buffer = new_list ();
  }
  
  ACI_MALLOC(output, (USHORT)(strlen(msg)+1));
  memcpy(output, msg, (USHORT)(strlen(msg)+1));

  ACI_MALLOC( buff_indic_elem, sizeof(T_ATI_INDIC_BUFF) );
  buff_indic_elem->output = output;
  buff_indic_elem->output_type = output_type;
  /* if list count is the maximum then the available elements are reused*/
  if(get_list_count(src_params->indication_buffer) >= LIST_MAX_LENGTH)
  {
    insert_shift_list(src_params->indication_buffer, buff_indic_elem);
  }
  else
  {
    insert_list( src_params->indication_buffer, buff_indic_elem);
  }

  if ( (strlen (msg)) >= STRACE_LEN )
  {
    /* terminate traced string if too long */
    msg[STRACE_LEN-1] = '\0';
  }

  TRACE_EVENT_P2("buffering on src %d: %s", src_params->src_id, msg);
}

GLOBAL void io_sendMessageEx (UBYTE srcId, CHAR *msg,
                             T_ATI_OUTPUT_TYPE output_type)
{
  T_ATI_SRC_PARAMS *src_params;

  src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  if (src_params EQ NULL)
  {
    TRACE_EVENT_P1 ("[ERR] io_sendMessageEx: srcId=%d not found", srcId);
    return;
  }

  /*
   *  ATQ suppesses intermidiate, final and unsolicited result codes.
   *  Only information text in responce to commands is not affected.
   */
  if(IS_CONFIRM_OUTPUT(output_type))
  {
    io_setCommandState(src_params, ATI_LINE_STATE_END);
  }
  
  if (ati_user_output_cfg[srcId].atQ)
  {
    if(! IS_NORMAL_OUTPUT(output_type) AND ! IS_ECHO_OUTPUT(output_type))
    {
      return;
    }
  }
  
if ( IS_INDICATION_OUTPUT (output_type)
    AND check_should_buffer_ind(src_params) )
  {
    buffer_indication( src_params, msg, output_type );
    return;
  }

  if (src_params->result_cb NEQ NULL)
  {
    src_params->result_cb (srcId, output_type,
                           (UBYTE*)msg, (USHORT)strlen (msg));
  }
  else
  {
    TRACE_EVENT_P1 ("[WRN] io_sendMessageEx: no result callback srcId=%d",
                    srcId);

    if ( (strlen (msg)) >= STRACE_LEN )
    {
      /* terminate traced string if too long */
      msg[STRACE_LEN-1] = '\0';
    }

    TRACE_EVENT_P1 ("%s", msg);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : io_SendIndication  |
+--------------------------------------------------------------------+

  PURPOSE :
*/

GLOBAL void io_sendIndication(UBYTE srcId, CHAR *msg,
                              T_ATI_OUTPUT_TYPE output_type)
{
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
    
  if (src_params EQ NULL)
  {
    TRACE_EVENT_P1 ("[ERR] io_sendIndication: srcId=%d not found", srcId);
    return;
  }

  /*
   *  ATQ suppesses intermidiate, final and unsolicited result codes.
   *  Only information text in responce to commands is not affected.
   */
  if (ati_user_output_cfg[srcId].atQ)
  {
    return;
  }

  output_type |= ATI_INDICATION_OUTPUT | 
                 ATI_BEGIN_CRLF_OUTPUT | 
                 ATI_END_CRLF_OUTPUT;

  if ( check_should_buffer_ind(src_params) )
  {
    buffer_indication( src_params, msg, output_type );
    return;
  }
  if (src_params->result_cb NEQ NULL)
  {
    src_params->result_cb (srcId, output_type,
                           (UBYTE*)msg, (USHORT)strlen (msg));
  }
  else
  {
    TRACE_EVENT_P1 ("[WRN] io_sendIndication: no result callback srcId=%d",
                    srcId);

    if ( (strlen (msg)) >= STRACE_LEN )
    {
      /* terminate traced string if too long */
      msg[STRACE_LEN-1] = '\0';
    }

    TRACE_EVENT_P1 ("%s", msg);
  }
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : io_sendConfirm     |
+--------------------------------------------------------------------+

  PURPOSE : output to AT interface
*/

GLOBAL void send_buffered_indication( T_ATI_SRC_PARAMS *src_params )
{
  BOOL            go_on = TRUE;
  T_ATI_INDIC_BUFF *indic = src_params->indication_buffer; /* to be sure it's not NULL */

  TRACE_EVENT_P1("send buffered indications for source: %d", src_params->src_id);
  while( go_on )
  {
    indic = remove_first_element( src_params->indication_buffer ); 

    if( indic NEQ NULL )
    {
      io_sendIndication( src_params->src_id,
                         indic->output,
                         indic->output_type );
      ACI_MFREE( indic->output );
      ACI_MFREE( indic );    
    }
    else
    {
      ACI_MFREE( src_params->indication_buffer );
      src_params->indication_buffer = NULL;
      go_on = FALSE;
    }
  }
}
                                       
GLOBAL void io_sendConfirm(UBYTE srcId, CHAR *msg,
                           T_ATI_OUTPUT_TYPE output_type)
{
  BOOL is_CONNECT = FALSE;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("io_sendConfirm()");

  if (src_params EQ NULL)
  {
    TRACE_EVENT_P1 ("[ERR] io_sendConfirm: srcId=%d not found", srcId);
    return;
  }

  if (BITFIELD_CHECK (output_type, ATI_CONNECT_OUTPUT))
  {
    is_CONNECT = TRUE;
    BITFIELD_CLEAR (output_type, ATI_CONNECT_OUTPUT);/*lint !e64 (Warning: type mismatch)*/
  }

  output_type |= ATI_CONFIRM_OUTPUT    | 
                 ATI_BEGIN_CRLF_OUTPUT | 
                 ATI_END_CRLF_OUTPUT;

  if (src_params->result_cb NEQ NULL)
  {
    /*
     *  ATQ suppesses intermidiate, final and unsolicited result codes.
     *  Only information text in responce to commands is not affected.
     */
    if (ati_user_output_cfg[srcId].atQ EQ 0)
    {
      src_params->result_cb (srcId, output_type,
                             (UBYTE*)msg, (USHORT)strlen (msg));
    }
  }
  else
  {
    TRACE_EVENT_P1 ("[WRN] io_sendConfirm: no result callback srcId=%d",
                    srcId);

    if ( (strlen (msg)) >= STRACE_LEN )
    {
      /* terminate traced string if too long */
      msg[STRACE_LEN-1] = '\0';
    }

    TRACE_EVENT_P1 ("%s", msg);
  }

  io_setCommandState(src_params, ATI_LINE_STATE_END);
  trace_cmd_state(src_params->src_id, src_params->cmd_state, CMD_IDLE);
  src_params->cmd_state = CMD_IDLE;

  if ((src_params->indication_buffer NEQ NULL) /* send buffered indications */
    AND (is_CONNECT EQ FALSE))                  /* but for data call not yet --> when back in CMD mode */
  {
    send_buffered_indication( src_params );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_DTIS                |
|                                 ROUTINE : io_sendChar             |
+-------------------------------------------------------------------+

  PURPOSE : send one character through DTI
*/

GLOBAL void io_sendChar (CHAR out, UBYTE srcId)
{
  T_ATI_SRC_PARAMS *src_params;

  src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  if (src_params EQ NULL)
  {
    TRACE_EVENT_P1 ("[ERR] io_sendChar: srcId=%d not found", srcId);
    return;
  }
  if (src_params->result_cb NEQ NULL)
  {
    src_params->result_cb (srcId, ATI_ECHO_OUTPUT, (UBYTE*)&out, 1);
  }
  else
  {
    TRACE_EVENT_P1 ("[WRN] io_sendChar: no result callback srcId=%d",
                    srcId);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_URT            |
| STATE   : code                        ROUTINE : io_setRngInd       |
+--------------------------------------------------------------------+

  PURPOSE: sets the V.24 ring indicator line

*/
GLOBAL void io_setRngInd (T_IO_RING_STAT state, T_ACI_CRING_SERV_TYP bt1, T_ACI_CRING_SERV_TYP bt2)
{
  T_ATI_SRC_PARAMS *src_params;
  T_IO_RING_PARAMS ring_params;

  src_params = get_next_element (ati_src_list, NULL);
  while (src_params NEQ NULL)
  {
    if (src_params->line_state_cb NEQ NULL)
    {
      ring_params.ring_stat = state;
      ring_params.b_cap_1 = bt1;
      ring_params.b_cap_2 = bt2;

      src_params->line_state_cb (src_params->src_id, 
                                 ATI_LINE_STATE_RNG, 
                                 (ULONG)&ring_params);
    }
    src_params = get_next_element (ati_src_list, src_params);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_URT            |
| STATE   : code                        ROUTINE : io_setDCD          |
+--------------------------------------------------------------------+

  PURPOSE: sets the V.24 data carrier detect line

*/
#ifdef UART
GLOBAL void io_setDCD (T_ACI_CMD_SRC src_id, T_IO_DCD_STAT state)
{
  T_ATI_SRC_PARAMS *src_params;
  T_ACI_DCD_MOD     ATandC_setting;
  
  TRACE_FUNCTION("io_setDCD()");

  src_params = find_element (ati_src_list, (UBYTE)src_id, search_ati_src_id);
  if (src_params EQ NULL)
  {
    TRACE_EVENT_P1 ("[ERR] io_setDCD: srcId=%d not found", src_id);
    return;
  }
  
  qAT_AndC(src_id, &ATandC_setting);
                                    
  /* don't set DCD off if DCD shall be allways on */
  if( (ATandC_setting EQ DCD_ALWAYS_ON) AND
      (state EQ IO_DCD_OFF ) )
  {
    TRACE_EVENT ("[DBG] io_setDCD: DCD_ALWAYS_ON");
    return;
  }

  if (src_params->line_state_cb NEQ NULL)
  {
    src_params->line_state_cb ((UBYTE)src_id, ATI_LINE_STATE_DCD, (ULONG)state);
  }
  else
  {
    TRACE_EVENT_P1 ("[WRN] io_setDCD: no callback for srcId=%d", src_id);
  }
}
#endif /* UART */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_URT            |
| STATE   : code                        ROUTINE : io_setCommandState |
+--------------------------------------------------------------------+

  PURPOSE: sets the command state to start or end

*/
#define UNUSED_ULONG_PARAMETER (ULONG) 0L

GLOBAL void io_setCommandState(T_ATI_SRC_PARAMS *src_params, T_ATI_LINE_STATE_TYPE line_state_type)
{

  TRACE_FUNCTION("io_setCommandState()");

  if (src_params->line_state_cb NEQ NULL)
  {
    src_params->line_state_cb (src_params->src_id, line_state_type, UNUSED_ULONG_PARAMETER);
  }
  else
  {
    TRACE_EVENT_P1 ("[WRN] io_setCommandState: no callback for srcId=%d",
                    src_params->src_id);
  }
}

#endif /* ATI_IO_C */

