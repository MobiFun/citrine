/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  J:\g23m-aci\aci\ati_src_uart_io.c
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

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#ifdef UART

#ifndef ATI_SRC_UART_IO_C
#define ATI_SRC_UART_IO_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */

#include "aci_lst.h"
#include "dti_conn_mng.h"

#include "psa_uart.h"
#include "cmh_uart.h"

#ifdef _SIMULATION_
#include "aci.h"
#include "ati_src_tst.h"

#   ifdef SMI
#   undef  hCommMMI
#   define hCommMMI   hCommSMI
#   endif

EXTERN T_ACI_LIST *ati_src_list;
#endif

#include "aci_io.h"
#include "ati_io.h"
#include "aci_mem.h"
#include "ati_src_uart.h"
#include "aci.h"
#include "sap_dti.h"

/*==== CONSTANTS ==================================================*/
#define MAX_NORMAL_BUFFER_LEN (100-4)     // Since Malloc uses 4 additional internal bytes of the Blocks
#ifdef GPRS
#define MAX_LARGE_BUFFER_LEN (600-4)
#else
#define MAX_LARGE_BUFFER_LEN (400-4)
#endif
/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/
/*==== FUNCTIONS ==================================================*/
LOCAL void io_DTIsendString (UBYTE *string, USHORT string_len,
                             T_ACI_DTI_PRC     *src_infos,
                             T_ATI_OUTPUT_TYPE output_type);

/* LOCAL void uart_buffer_data(T_ACI_DTI_PRC * src_infos, T_desc2 * send_data); */

#ifdef _SIMULATION_
LOCAL BOOL get_source_type( UBYTE src_id, T_ATI_SRC_TYPE *src_type );
#endif
/*=================================================================*/


GLOBAL void uart_src_result_cb (UBYTE             src_id,
                                T_ATI_OUTPUT_TYPE	output_type,
                                UBYTE             *output,
                                USHORT            output_len)
{
  T_ACI_DTI_PRC *src_infos;
  TRACE_FUNCTION("uart_src_result_cb()");

#ifdef RMV_01_04_03
#ifdef FF_TWO_UART_PORTS
  /* disable output to UART on data channel */
  if (src_id EQ UART_DATA_CHANNEL)
  {
    return;
  }
#endif
#endif
  src_infos = find_element (uart_src_params, (UBYTE)src_id, cmhUARTtest_srcId);
  if (src_infos EQ NULL)
  {
    TRACE_EVENT_P1("[ERR] uart_src_result_cb: src_id=%d not found", src_id) ;
    return ;
  }

  switch (src_infos->LineState)
  {
    case LINE_CONNECTING:
    case LINE_CONNECTED:
    case LINE_TMP_CONNECTED:
      break;

    case LINE_TMP_DISCONNECTING:
    case LINE_TMP_DISCONNECTED:
      if (IS_FORCED_OUTPUT (output_type) || IS_CONFIRM_OUTPUT (output_type))
      {
        break;
      }
      return;

    case LINE_DISCONNECTED:
      return;
  }

  if (src_infos->first_output EQ TRUE)
  {
    src_infos->first_output = FALSE;
    output_type |= ATI_BEGIN_CRLF_OUTPUT;
  }
  io_DTIsendString (output, output_len, src_infos, output_type);  
}


GLOBAL void uart_src_line_state_cb (UBYTE                 src_id,
                                    T_ATI_LINE_STATE_TYPE	line_state_type,
                                    ULONG                 line_state_param)
{
  T_ACI_DTI_PRC   *src_infos;

  TRACE_FUNCTION("uart_src_line_state_cb()");

  src_infos = find_element (uart_src_params, (UBYTE)src_id, cmhUARTtest_srcId);
  if (src_infos EQ NULL)
  {
    TRACE_EVENT ("[ERR] uart_src_line_state_cb: src_id not found");
    return;
  }

  /* tlu: */
  /*
  switch (src_infos->LineState)
  {
    case LINE_TMP_DISCONNECTED:
    case LINE_DISCONNECTED:
      return;
  }
  */

  switch (line_state_type)
  {
    case ATI_LINE_STATE_OUTPUT_TYPE:
      TRACE_EVENT_P1 ("output type: %d",line_state_param);
      src_infos->run_cmd = TRUE;
      src_infos->first_output = TRUE;

      if( line_state_param EQ ATI_OUTPUT_TYPE_LARGE )
      {
        src_infos->large_type = TRUE;
      }
      else
      {
        src_infos->large_type = FALSE;
      }
      break;

    case ATI_LINE_STATE_DCD:

      if(line_state_param EQ IO_DCD_ON)
      {
        BITFIELD_SET (src_infos->data_cntr, UART_DTI_SB_BIT);
      }
      else
      {
        BITFIELD_CLEAR (src_infos->data_cntr, UART_DTI_SB_BIT);
      }
      /* tlu: */
      /*
      if (src_infos->LineState EQ LINE_CONNECTED)
      {
        psaUART_DCDreq( src_id, (UBYTE)line_state_param );
      }
      */
      psaUART_DCDreq( src_id, (UBYTE)line_state_param );

      break;

    case ATI_LINE_STATE_RNG:
    {
      T_IO_RING_PARAMS rng_params;
#ifdef RMV_01_04_03
#ifdef FF_TWO_UART_PORTS
      /* disable RING on data channel */
      if (src_id EQ UART_DATA_CHANNEL)
      {
        break;
      }
#endif
#endif
      memcpy (&rng_params, (T_IO_RING_PARAMS*)line_state_param,
              sizeof(T_IO_RING_PARAMS));
      psaUART_RINGreq( src_id, (UBYTE)rng_params.ring_stat );
      break;
    }

    case ATI_LINE_STATE_START:
      break;
    
    case ATI_LINE_STATE_END:
      src_infos->run_cmd = FALSE;
      break;

    default:
    {
      TRACE_EVENT_P1("[WRN] uart_src_line_state_cb (): "\
               "UNKNOWN line_state_type = %d", line_state_type);
      break;
    }
  }
}

LOCAL T_desc2 *get_last_buffer (T_desc2 *data_head)
{
  T_desc2 *next_data;

  if (data_head EQ NULL)
  {
    return (NULL);
  }

  next_data = data_head;
  while ((T_desc2*)next_data->next NEQ NULL)
  {
    next_data = (T_desc2*)next_data->next;
  }

  return (next_data);
}


LOCAL T_desc2 *get_new_buffer (BOOL large_type, USHORT *max_buffer_size)
{
  T_desc2 *new_buffer;

  if (large_type EQ TRUE)
    *max_buffer_size = MAX_LARGE_BUFFER_LEN;
  else
    *max_buffer_size = MAX_NORMAL_BUFFER_LEN;

  ACI_MALLOC (new_buffer, *max_buffer_size);
  *max_buffer_size -= (sizeof(T_desc2) - 1);

  new_buffer->next = (ULONG)NULL;
  new_buffer->len  = 0;
  new_buffer->size = *max_buffer_size;
  new_buffer->offset = 0;

  return (new_buffer);
}

LOCAL void free_data_buffer (T_ACI_DTI_DATA_BUF *data_buffer)
{
  T_desc2 *p_desc=NULL,*p_desc_next=NULL;

  if (data_buffer->data_list EQ NULL)
  {
    return;
  }

  p_desc = data_buffer->data_list;
  if (p_desc NEQ NULL)
  {
    p_desc_next = (T_desc2 *)p_desc->next;
  }

  while (p_desc NEQ NULL)
  {
    ACI_MFREE (p_desc);

    p_desc = p_desc_next;
    if (p_desc NEQ NULL)
    {
      p_desc_next = (T_desc2 *)p_desc->next;
    }
  }

  data_buffer->data_list       = NULL;
  data_buffer->max_buffer_size = 0;
  data_buffer->create_time     = 0;
}


GLOBAL void uart_buffer_data (T_ACI_DTI_PRC *src_infos, T_desc2 *send_data)
{
  T_desc2 *active_buffer, *last_buffer;
  USHORT i, j;
  T_TIME current_time;
  TRACE_FUNCTION("uart_buffer_data()");

  last_buffer = get_last_buffer(src_infos->data_buffer.data_list);
  active_buffer = last_buffer;

  if (last_buffer EQ NULL)
  {
    active_buffer = get_new_buffer (src_infos->large_type,
                                    &src_infos->data_buffer.max_buffer_size);
    src_infos->data_buffer.data_list = active_buffer;
    last_buffer = active_buffer;
    /* store the creation time for the data buffer */
    vsi_t_time (VSI_CALLER &src_infos->data_buffer.create_time);
  }
  else
  {
    /*
        If no device is connected to UART it is possible that the buffer
        use all available memory.
        Solution:
        When the line is connected, check the life time of the
        data buffer. Remove the buffer if the live time is greater than
        30 sec. (only if line is in data mode (line is disconnected)).
    */
//    if (src_infos->LineState EQ LINE_CONNECTED) // line is always connected
    {
      vsi_t_time (VSI_CALLER &current_time);
      if ((current_time - src_infos->data_buffer.create_time) > MAX_DATA_BUFFER_LIFE_TIME)
      {
        TRACE_EVENT_P1 ("[WRN] uart_buffer_data(): life time expired, buffer deleted (%d)",
                        current_time - src_infos->data_buffer.create_time);
        free_data_buffer (&src_infos->data_buffer);
        return;
      }
    }
  }

  i = 0;
  while (i < send_data->len)
  {
    if (active_buffer->len EQ active_buffer->size)   // (src_infos->data_buffer.max_buffer_size))
    {
      active_buffer = get_new_buffer (src_infos->large_type,
                                      &src_infos->data_buffer.max_buffer_size);
      last_buffer->next = (ULONG)active_buffer;
      last_buffer = active_buffer;
    }
    // Optimize for Speed
    j = MINIMUM((active_buffer->size - active_buffer->len), (send_data->len - i));
    if (j>1)
    {
      memcpy((char *)&active_buffer->buffer[active_buffer->len], (char *)&send_data->buffer[i], j);/*lint !e670 (Warning: Possible access beyond array for function)*/
      active_buffer->len += j;
      i += j;
    }
    else
      active_buffer->buffer[active_buffer->len++] = send_data->buffer[i++];
  }
}


GLOBAL void uart_send_buffer_data (UBYTE src_id)
{
  T_ACI_DTI_PRC   *src_infos = NULL;

  TRACE_FUNCTION("uart_send_buffer_data()");

  /* dti_id should be supported by DTI_ENTITY_ID or something, here.. */
  src_infos = find_element (uart_src_params, src_id,
                            cmhUARTtest_srcId);
#ifdef DTI
  if (aci_src_dti_params[src_id].isDtiConnected EQ FALSE)
  {
    TRACE_EVENT ("DTI_READY_IND: line not connected");
    return;
  }
#endif /* DTI */
  /*
   * send data which may have been stored ..
   */
  if (src_infos->data_buffer.data_list NEQ NULL)
  {
#ifdef DTI
#ifdef _SIMULATION_
    {
      T_ATI_SRC_TYPE src_type;

      if(get_source_type(src_infos->srcId, &src_type))
      {
        if( ATI_SRC_TYPE_TST EQ src_type )
        {
          ati_src_tst_sendString(src_infos->data_buffer.data_list, src_infos->srcId);
        }
        else
        {
          psaDTI_data_req (src_infos->data_buffer.data_list, src_infos, DTI_ENTITY_UART);
        }
      }
    }
#else
    psaDTI_data_req (src_infos->data_buffer.data_list, src_infos, DTI_ENTITY_UART);
#endif /* _SIMULATION_ */
#endif /* DTI */
    src_infos->data_buffer.data_list = NULL;
    src_infos->data_buffer.max_buffer_size = 0;
    src_infos->data_buffer.create_time = 0;
  }

/* this is wrong here, moved to ati_src_uart.c */
/*
  if (uartShrdPrm.dtr_clearcall EQ TRUE)
  {
    T_ACI_RETURN ret;

    ret = sAT_H(src_infos->srcId);
    if (ret != AT_EXCT && ret != AT_CMPL)
    {
      uartShrdPrm.dtr_clearcall = FALSE;
      TRACE_ERROR("sAT_H() error");
    }
  }
*/
}


LOCAL T_desc2 *make_send_data (UBYTE *data, USHORT data_len,
                              T_ATI_OUTPUT_TYPE output_type)
{
  T_desc2  *send_data;
  USHORT  send_len = 0;
#if !defined (_SIMULATION_) OR defined(ACIDTI_TEST_SIM)
static  UBYTE   formated_output[MAX_CMD_LEN+5];
  USHORT  formated_len;

  formated_len = format_output (data, data_len, formated_output, output_type);
  send_len     = sizeof(T_desc2) - 1 + formated_len;

  ACI_MALLOC(send_data, send_len);
  send_data->next   = (ULONG)NULL;
  send_data->len    = formated_len;
  send_data->size   = formated_len;
  send_data->offset = 0;
  memcpy(send_data->buffer, formated_output, formated_len);

#else  /* _SIMULATION_ */

  send_len = sizeof(T_desc2) - 1 + data_len;

  ACI_MALLOC(send_data, send_len);
  send_data->next   = (ULONG)NULL;
  send_data->len    = data_len;
  send_data->size   = data_len;
  send_data->offset = 0;
  memcpy(send_data->buffer, data, data_len);

#endif /* _SIMULATION_ */

  return (send_data);
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_DTIS                |
|                                 ROUTINE : io_DTIsendString        |
+-------------------------------------------------------------------+

  PURPOSE : format output string and send to the UART through DTI
*/

LOCAL void io_DTIsendString (UBYTE *string, USHORT string_len,
                             T_ACI_DTI_PRC     *src_infos,
                             T_ATI_OUTPUT_TYPE output_type)
{
  T_desc2         *send_data;
#ifdef _SIMULATION_
  T_ATI_SRC_TYPE src_type;

  /*
   * get_source_type() writes an trace_event in the error case.
   */
  if(!get_source_type(src_infos->srcId, &src_type))
    return;
#endif /* _SIMULATION_ */

  TRACE_FUNCTION("io_DTIsendString");
  if (!IS_ECHO_OUTPUT (output_type))
  {
#ifdef _SIMULATION_
    if( ATI_SRC_TYPE_TST NEQ src_type )
#endif /* _SIMULATION_ */
    {
      trace_cmd_line ("OUT:", (CHAR*)string, src_infos->srcId, string_len);
    }
  }

  if (string_len > MAX_LARGE_BUFFER_LEN)
  {
    TRACE_ERROR ("String too large for io_DTIsendString!");
    TRACE_EVENT_P2 ("Truncating string from %d to %d", string_len, MAX_LARGE_BUFFER_LEN);
    string_len = MAX_LARGE_BUFFER_LEN;
  }

  send_data = make_send_data (string, string_len, output_type);

#ifdef DTI
  if ((aci_src_dti_params[src_infos->srcId].isDtiConnected) AND
        (aci_src_dti_params[src_infos->srcId].dtxState EQ READY))
  {
#ifdef _SIMULATION_
    if( ATI_SRC_TYPE_TST EQ src_type )
    {
      ati_src_tst_sendString(send_data, src_infos->srcId);
      ACI_MFREE(send_data);
    }
    else
    {
      psaDTI_data_req (send_data, src_infos, DTI_ENTITY_UART);
    }
#else
    /* send message through DTI */
    psaDTI_data_req (send_data, src_infos, DTI_ENTITY_UART);
#endif /* _SIMULATION_ */
  }
  else
  {
    /* buffer these datas */
    uart_buffer_data (src_infos, send_data);
    ACI_MFREE (send_data);
  }
#else
  ACI_MFREE (send_data);
#endif /* DTI */
}

#ifdef _SIMULATION_

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ATI_SRC_UART_IO_C       |
|                                 ROUTINE : get_source_type         |
+-------------------------------------------------------------------+

  PURPOSE : provides the source type for a source ID

            returns FALSE if source ID is not registered
*/
LOCAL BOOL get_source_type( UBYTE src_id, T_ATI_SRC_TYPE *src_type )
{
  T_ATI_SRC_PARAMS *src_params = NULL;

  src_params = find_element (ati_src_list, src_id, search_ati_src_id);
  if (src_params EQ NULL)
  {
    TRACE_EVENT_P1("[ERR] get_source_type: source ID %d not found", src_id);
    *src_type = ATI_SRC_TYPE_UNKNOWN;
    return FALSE;
  }

  *src_type = src_params->src_type;
  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ATI_SRC_UART_IO_C        |
|                                 ROUTINE : uart_src_test_sendString |
+--------------------------------------------------------------------+

  PURPOSE : Provides the io_DTIsendString() for the test-source-callback
            and apends a separator to the message.
            The separator is needed to isolates the messages in the
            case of buffering messages during UART is not connected
            with ACI
*/
GLOBAL void uart_src_test_sendString (UBYTE *string, USHORT string_len,
                             T_ACI_DTI_PRC     *src_infos,
                             T_ATI_OUTPUT_TYPE output_type)
{
  UBYTE *tmp;

  ACI_MALLOC(tmp, string_len + 1);
  strncpy(tmp, string, string_len);
  tmp[string_len] = (UBYTE) ATI_TEST_SRC_SEPARATOR;

  io_DTIsendString(tmp, (USHORT) (string_len + 1), src_infos, output_type);

  ACI_MFREE(tmp);
}
#endif

#endif /* UART */
