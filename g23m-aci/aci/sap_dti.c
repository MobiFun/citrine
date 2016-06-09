/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS
|  Modul   :  
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
|  Purpose :  MUX for DTI primitives
|             
|             Different entities sends the same DTI primitives. In this
|             file the correct PSA function will call
+----------------------------------------------------------------------------- 
*/ 
/*==== INCLUDES ===================================================*/

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#ifdef DTI

#ifndef SAP_DTI_C
#define SAP_DTI_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "line_edit.h"
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */

#include "aci.h"
#include "aci_lst.h"

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#ifdef UART
#include "psa_uart.h"
#include "cmh_uart.h"
#endif

#ifdef FF_PSI
#include "psa_psi.h"
#include "cmh_psi.h"
#include "ati_src_psi.h"
#endif /*FF_PSI*/
#include "aci_mem.h"
#ifdef UART
#include "ati_src_uart.h"
#endif


#include "sap_dti.h"
#include "psa.h"
#include "cmh.h"

#ifdef FF_BAT
#include "p_bat.h"
#include "aci_bat.h"
#ifdef _SIMULATION_
#include "ati_bat.h"
#endif
#endif

EXTERN T_ACI_LIST *ati_src_list;

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/


/* LOCAL FUNCTIONS */


/*
+------------------------------------------------------------------------------
|    Function    : sig_dti_connection_opened_ind
+------------------------------------------------------------------------------
|    PURPOSE     : dti connection to UART/PSI has been opened
+------------------------------------------------------------------------------
*/
LOCAL void sig_dti_connection_opened_ind(UBYTE src_id)
{
  T_DTI_CONN_LINK_ID link_id;
#ifdef FF_PSI
  T_ACI_DTI_PRC_PSI *src_infos = find_element (psi_src_params, src_id, cmhPSItest_srcId);
#endif /*FF_PSI*/
/* for tracing of establishing of CMD channels for dual port version */
#ifdef RMV_15_04_03
  extern CHAR gob_tst_buf[];
#endif

  TRACE_FUNCTION("sig_dti_connection_opened_ind()");

  link_id = dti_cntrl_get_link_id( DTI_ENTITY_ACI, src_id, DTI_SUB_NO_NOTPRESENT );

/* for tracing of establishing of CMD channels for dual port version */
#ifdef RMV_15_04_03
  sprintf(gob_tst_buf+strlen(gob_tst_buf), "oi:%d ", link_id);
#endif

  dti_cntrl_entity_connected( link_id, DTI_ENTITY_ACI, DTI_OK );

  aci_src_dti_params[src_id].isDtiConnected = TRUE;

  if (aci_src_dti_params[src_id].dtxState EQ READY)
  {
    /* send buffered data */
#ifdef FF_PSI
    if (src_infos NEQ NULL)
      psi_send_buffer_data(src_id);
    else
#endif /*FF_PSI*/
      uart_send_buffer_data(src_id);
  } 
}


/*
+------------------------------------------------------------------------------
|    Function    : sig_dti_connection_closed_ind
+------------------------------------------------------------------------------
|    PURPOSE     : dti connection to UART/PSI has been closed down
+------------------------------------------------------------------------------
*/
LOCAL void sig_dti_connection_closed_ind(UBYTE src_id)
{
  T_DTI_CONN_LINK_ID link_id;

  TRACE_FUNCTION("sig_dti_connection_closed_ind()");

  link_id = dti_cntrl_get_link_id( DTI_ENTITY_ACI, src_id, DTI_SUB_NO_NOTPRESENT );

  dti_cntrl_entity_disconnected( link_id, DTI_ENTITY_ACI );

  aci_src_dti_params[src_id].isDtiConnected = FALSE;
}


/*
+------------------------------------------------------------------------------
|    Function    : sig_dti_data_received_ind
+------------------------------------------------------------------------------
|    PURPOSE     : process reason parameter REASON_DATA_RECEIVED
+------------------------------------------------------------------------------
*/

GLOBAL void sig_dti_data_received_ind(UBYTE src_id,
                                            T_DTI2_DATA_IND *dti_data_ind)
{
  USHORT  tot_len;
  SHORT   pos = 0;
  T_desc2 *p_desc,
          *old_desc;
  UBYTE   *chars_received;
  BOOL    get_data = TRUE;
  USHORT  new_len = 0;
  T_ACI_DTI_PRC *src_infos = find_element (uart_src_params, src_id, cmhUARTtest_srcId);
#ifdef FF_PSI
  T_ACI_DTI_PRC_PSI *src_infos_psi = find_element (psi_src_params, src_id, cmhPSItest_srcId);
#endif /*FF_PSI*/
  TRACE_FUNCTION("sig_dti_data_received_ind()");

  /* prevent dti library from automatically sending
     getdata primitives */
#ifdef FF_PSI
   if (src_infos EQ NULL AND src_infos_psi EQ NULL)
#else
   if (src_infos EQ NULL)
#endif /*FF_PSI*/
   {
     TRACE_EVENT_P1("[ERR] sig_dti_data_received_ind: link_id=%d not found",
                dti_data_ind->link_id) ;
     return ;
   }

  if (src_infos NEQ NULL)
  {
#ifndef FF_ATI_BAT /* we came from gdd_aci_send_data() --> faked DTI primitive */
    dti_stop (aci_hDTI, src_id, DTI_ENTITY_UART, ACI_DTI_DN_CHANNEL);
#endif
    if(src_infos->RecState EQ READY_REC)
    {
      src_infos->RecState = RECEIVING;
    }
    if (dti_data_ind->parameters.st_lines.st_line_sa EQ DTI_SA_ON)
    {
      BITFIELD_SET (src_infos->data_cntr, UART_DTI_SA_BIT);
    }
    else
    {
      BITFIELD_CLEAR (src_infos->data_cntr, UART_DTI_SA_BIT);
    }
  }
#ifdef FF_PSI
  else if (src_infos_psi NEQ NULL)
  {
#ifndef FF_ATI_BAT /* we came from gdd_aci_send_data() --> faked DTI primitive */
    dti_stop (aci_hDTI, src_id, DTI_ENTITY_UART, ACI_DTI_DN_CHANNEL);
#endif

    if(src_infos_psi->RecState EQ READY_REC)
    {
      src_infos_psi->RecState = RECEIVING;
    }
    if (dti_data_ind->parameters.st_lines.st_line_sa EQ DTI_SA_ON)
    {
      BITFIELD_SET (src_infos_psi->data_cntr, PSI_DTI_SA_BIT);
    }
    else
    {
      BITFIELD_CLEAR (src_infos_psi->data_cntr, PSI_DTI_SA_BIT);
    }
  }
#endif /*FF_PSI*/
  tot_len = dti_data_ind -> desc_list2.list_len;
  p_desc = (T_desc2 *)dti_data_ind -> desc_list2.first;

  if ( aci_src_dti_params[src_id].isDtiConnected EQ FALSE ) 
  {
    TRACE_EVENT ("DTI2_DATA_IND: line not connected");
    PFREE(dti_data_ind);
    return;
  }
  
#ifdef FF_BAT
  /* we want only to see data from src_infos_psi */
  if (aci_cmd_src_mode_get((T_ACI_CMD_SRC)src_id) EQ CMD_MODE_BAT) 
  {
#if defined _SIMULATION_ && FF_ATI_BAT
    /*
     * Simulation of BAT with ATI as application has the following problem:
     * test cases uses the TST and UART source, but BAT needs a PSI source maintenance.
     * Therefore there is in parallel to the TST or UART source a PSI source maintenance and
     * the TST/UART source has been set to CMD_MODE_BAT.
     * When the TAP sends an ASCII AT command string the command must be processed
     * at first with uart_src_proc_chars(). On ATI level we use the BAT lib and the special
     * BAT adapter GDD_ACI will call this function once again, because we have to simulate
     * an incoming DTI primitive. This DTI primitive carries now a BAT command.
     * That's why we have to check the bytes to distinuish between BAT and ASCII commands.
     */
    T_desc2 *tmp = p_desc;
    int i = 0;
    get_data = TRUE; /* we use this boolean here to check for valid ASCII AT command chars */
    while (i < tmp->len-1)
    {
      if ((p_desc->buffer[i] < 0x20) OR (p_desc->buffer[i] > 0x7F))
      {
        if (at.S[3] EQ p_desc->buffer[i]) /* CR */
          continue;
        if (at.S[4] EQ p_desc->buffer[i]) /* LF */
          continue;
        if (at.S[5] EQ p_desc->buffer[i]) /* BS */
          continue;
        if (0x1a EQ p_desc->buffer[i]) /* CTRL-Z = SMS end */
          continue;

        get_data = FALSE; /* not a V25.ter, 07.07 char, so very likely a BAT command */
        break;
      }
      i++;
    }
    if (get_data EQ FALSE)
    {
#endif /* _SIMULATION_ && FF_ATI_BAT */  
      aci_bat_rcv(src_infos_psi,dti_data_ind);
      
      /* clean up the DTI primitive after BAT processing */
      while(p_desc NEQ NULL)
      {    
        old_desc = p_desc;
        p_desc = (T_desc2 *)p_desc->next;
        ACI_MFREE( old_desc );
      }   
      PFREE(dti_data_ind);
      return;
#if defined _SIMULATION_ && FF_ATI_BAT        
    }
#endif /* _SIMULATION && FF_ATI_BAT */ 
  }
#endif /* FF_BAT */


  ACI_MALLOC(chars_received, tot_len + 1);
  while(p_desc NEQ NULL)
  {
    new_len += p_desc -> len;
    if( new_len > tot_len)
    {
      TRACE_EVENT("inconsistance in datas received from DTI: ACI cannot process");
      ACI_MFREE(chars_received);
      return;
    }
    memcpy(chars_received+pos,p_desc -> buffer,p_desc -> len);
    /*    trace_buffer (p_desc->buffer, p_desc->len);*/
    pos = new_len;    
    
    old_desc = p_desc;      /* save to free descriptor at the end */
    p_desc = (T_desc2 *)p_desc -> next;   /* point to next one */
    ACI_MFREE( old_desc );
  }

  if (tot_len > 0)
  {
    if (src_infos NEQ NULL)
       get_data = uart_src_proc_chars (chars_received, pos, src_infos);
#ifdef FF_PSI
    else
       get_data = psi_src_proc_chars (chars_received, pos, src_infos_psi);
#endif /*FF_PSI*/
  }
  else
  {
    get_data = TRUE;
  }
  
  ACI_MFREE( chars_received );
  chars_received = NULL;

#ifdef FF_ATI_BAT
#ifdef _SIMULATION_
  if (aci_cmd_src_mode_get(src_id) EQ CMD_MODE_BAT) /* simulation with BAT library */
  {
    T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, src_id, search_ati_src_id);
    T_ACI_DTI_PRC_PSI *src_infos_psi = find_element (psi_src_params, src_id, cmhPSItest_srcId);
    while (ledit_ctrl(src_params->src_id,LEDIT_CTRL_MORE_CMDS, NULL) EQ LEDIT_CMPL)
    {
      ati_bat_response_simulation(&(src_infos_psi->res)); /* intermediate response */
    }
    if (src_params->cmd_state NEQ CMD_IDLE)
    {
      if (src_infos_psi->res.response)
      {
        ati_bat_response_simulation(&(src_infos_psi->res)); /* final response */
        Perform_ati_bat_maint(src_id);
      }
    }
  }
#endif /* _SIMULATION_ */
#endif /* FF_ATI_BAT */

  if( get_data )
  {
    if (src_infos NEQ NULL)
       cmhUART_getdata( (T_ACI_CMD_SRC)src_id );
#ifdef FF_PSI
    else if (src_infos_psi NEQ NULL)
       cmhPSI_getdata ((T_ACI_CMD_SRC)src_id);
#endif /*FF_PSI*/
  }
  PFREE(dti_data_ind);
}


/*
+------------------------------------------------------------------------------
|    Function    : sig_dti_tx_buffer_full_ind
+------------------------------------------------------------------------------
|    PURPOSE     : 
+------------------------------------------------------------------------------
*/
GLOBAL void sig_dti_tx_buffer_full_ind(UBYTE src_id)
{
  TRACE_FUNCTION("sig_dti_tx_buffer_full_ind");

  aci_src_dti_params[src_id].dtxState = NOT_READY;
}


/*
+------------------------------------------------------------------------------
|    Function    : sig_dti_tx_buffer_ready_ind
+------------------------------------------------------------------------------
|    PURPOSE     : 
+------------------------------------------------------------------------------
*/
GLOBAL void sig_dti_tx_buffer_ready_ind(UBYTE src_id)
{
  T_ACI_DTI_PRC   *src_infos = NULL;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, src_id, search_ati_src_id); 
#ifdef FF_PSI
  T_ACI_DTI_PRC_PSI *src_infos_psi = NULL;
  T_DTI_ENTITY_ID entity_list[2];
#endif /*FF_PSI*/  
  TRACE_FUNCTION("sig_dti_tx_buffer_ready_ind()");

  aci_src_dti_params[src_id].dtxState = READY;

  if (aci_src_dti_params[src_id].isDtiConnected)
  {
    src_infos = find_element (uart_src_params, src_id, cmhUARTtest_srcId);    
#ifdef FF_PSI
    src_infos_psi = find_element (psi_src_params, src_id, cmhPSItest_srcId);    
#endif /*FF_PSI*/
    /*
     * send data which may have been stored ..
     */
    if (src_infos NEQ NULL)
    {
      /* Need to return only when the data_list is not NULL so that the CMUX 
         part of the code will get executed. */
      if (src_infos->data_buffer.data_list NEQ NULL)
      {
        uart_send_buffer_data(src_id);
        if (src_infos->data_buffer.data_list EQ NULL)
        { 
          switch(src_params->curAtCmd)
          {
            case AT_CMD_COPN:
             cmd_PlusCOPN_CB(src_id);
             break;

            case AT_CMD_CPBR:
             cmd_PlusCPBR_CB(src_id);
             break;

            default:;  /* Do nothing */
          } 
        } 
        return;
      }
    }
#ifdef FF_PSI
    if (src_infos_psi NEQ NULL)
    {
      if (src_infos_psi->data_buffer.data_list NEQ NULL)
      {
#ifdef FF_BAT      
        if (aci_cmd_src_mode_get((T_ACI_CMD_SRC)src_id) EQ CMD_MODE_BAT)
        {
          aci_bat_send_buffer_dti_data(src_id);

          /*
           * CPBR and similar BAT commands
           */
          aci_bat_check_for_multi_line_response_command(src_id);
        }
        else
#endif          
        {
          psi_send_buffer_data(src_id);
        }
        if(src_infos_psi->data_buffer.data_list EQ NULL)
        {
          switch(src_params->curAtCmd)
          {
            case AT_CMD_COPN:
              cmd_PlusCOPN_CB(src_id);
              break;

            case AT_CMD_CPBR:
              cmd_PlusCPBR_CB(src_id);
              break;

            default:;  /* Do nothing */
          }
        }
        return;
      }
    }
#endif /*FF_PSI*/    
    {
      T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, src_id, search_ati_src_id); 
      if (uartEntcurCmd[src_id] EQ AT_CMD_CMUX)
      {
        if (cmhUART_CMUX ((T_ACI_CMD_SRC)src_id) EQ FALSE)
        {  
          /*
           * if we are here, we have a problem, because it has been sent an OK in advance to the source,
           * which works according to GSM 27.010
           * So, how to inform the source, that the muxer has not been started ?
           * see  sAT_PlusCMUX() in cmh_uarts.c
           */
          TRACE_EVENT("[ERR] sig_dti_tx_buffer_ready_ind(): UART muxer not ready");
        }
        uartEntcurCmd[src_id] = AT_CMD_NONE;
      }
#ifdef FF_PSI
      else
      {
        if((psi_ato.last_cmd EQ AT_CMD_O) AND (src_infos_psi NEQ NULL))
        {
          entity_list[0] = psi_ato.entity_to_conn;
          dti_cntrl_est_dpath_indirect (psi_ato.src_id,
                                        entity_list,
                                        psi_ato.num_entities,
                                        psi_ato.mode,
                                        psi_ato.cb,
                                        psi_ato.capability,
                                        psi_ato.cid);
          memset(&psi_ato,0,sizeof(T_ACI_PSI_CALL_TYPE));
        }
      }
#endif /*FF_PSI*/
    }
  } 
}





/* GLOBAL FUNCTIONS */


/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)      MODULE  : SAP_DTI                       |
| STATE   : code                ROUTINE : dti_lib_dti_dti_connect_cnf   |
+-----------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL void dti_lib_dti_dti_connect_cnf (
                    T_DTI2_CONNECT_CNF   *dti_connect_cnf)
{
  TRACE_FUNCTION("dti_lib_dti_dti_connect_cnf()");
  dti_dti_connect_cnf(aci_hDTI, dti_connect_cnf);
}


/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)      MODULE  : SAP_DTI                       |
| STATE   : code                ROUTINE : dti_lib_dti_dti_connect_ind   |
+-----------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL void dti_lib_dti_dti_connect_ind (
                    T_DTI2_CONNECT_IND   *dti_connect_ind)
{
  TRACE_FUNCTION("dti_lib_dti_dti_connect_ind()");
  dti_dti_connect_ind(aci_hDTI, dti_connect_ind);
}


/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)      MODULE  : SAP_DTI                       |
| STATE   : code                ROUTINE : dti_lib_dti_dti_disconnect_ind|
+-----------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL void dti_lib_dti_dti_disconnect_ind (
                    T_DTI2_DISCONNECT_IND   *dti_disconnect_ind)
{
  TRACE_FUNCTION("dti_lib_dti_dti_disconnect_ind()");
  dti_dti_disconnect_ind (aci_hDTI, dti_disconnect_ind);
}


/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)      MODULE  : SAP_DTI                       |
| STATE   : code                ROUTINE : dti_lib_dti_dti_data_ind      |
+-----------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL void dti_lib_dti_dti_data_ind (
                    T_DTI2_DATA_IND   *dti_data_ind)
{
  TRACE_FUNCTION("dti_lib_dti_dti_data_ind()");
  dti_dti_data_ind (aci_hDTI, dti_data_ind);
}


/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)      MODULE  : SAP_DTI                       |
| STATE   : code                ROUTINE : dti_lib_dti_dti_ready_ind     |
+-----------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL void dti_lib_dti_dti_ready_ind (
                    T_DTI2_READY_IND   *dti_ready_ind)
{
  TRACE_FUNCTION("dti_lib_dti_dti_ready_ind()");
  dti_dti_ready_ind (aci_hDTI, dti_ready_ind);
}


#ifdef _SIMULATION_
/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)      MODULE  : SAP_DTI                       |
| STATE   : code                ROUTINE : dti_lib_dti_dti_data_test_ind |
+-----------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL const void dti_lib_dti_dti_data_test_ind (
                    T_DTI2_DATA_TEST_IND   *dti_data_test_ind)
{
  TRACE_FUNCTION("dti_lib_dti_dti_data_test_ind()");
  dti_dti_data_test_ind (aci_hDTI, dti_data_test_ind);
}
#endif /* _SIMULATION_ */



/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)      MODULE  : SAP_DTI                       |
| STATE   : code                ROUTINE : psaACI_Init                   |
+-----------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL void psaACI_Init()
{
  UBYTE i;

  TRACE_FUNCTION("psaACI_Init()");

  for (i=0; i<CMD_SRC_MAX; i++)
  {
    aci_src_dti_params[i].isDtiConnected = FALSE;
    aci_src_dti_params[i].dtxState = NOT_READY;
  }
}


/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)      MODULE  : SAP_DTI                       |
| STATE   : code                ROUTINE : aci_pei_sig_callback          |
+-----------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL void aci_pei_sig_callback(U8 instance, U8 interfac, U8 channel,
                                 U8 reason, T_DTI2_DATA_IND *dti_data_ind)
{
  UBYTE src_id = instance;

  TRACE_FUNCTION("aci_pei_sig_callback()");


#ifdef _SIMULATION_
  if(channel NEQ ACI_DTI_DN_CHANNEL)
  {
    TRACE_ERROR("[DTI_MNG_SIG_CALLBACK] channel not valid!");
    return; /* error, not found */
  }
#endif /* _SIMULATION_ */

  if (aci_hDTI NEQ D_NO_DATA_BASE)
  {
    switch (reason)
    {
      case DTI_REASON_CONNECTION_OPENED:
        sig_dti_connection_opened_ind(src_id);
        break;

      case DTI_REASON_CONNECTION_CLOSED:
        sig_dti_connection_closed_ind(src_id);
        break;

      case DTI_REASON_DATA_RECEIVED:
        sig_dti_data_received_ind(src_id, dti_data_ind);
        break;

      case DTI_REASON_TX_BUFFER_FULL:
        sig_dti_tx_buffer_full_ind(src_id);
        break;

      case DTI_REASON_TX_BUFFER_READY:
        sig_dti_tx_buffer_ready_ind(src_id);
        break;

      default:
        TRACE_ERROR("unknown DTILIB reason parameter");
        break;
    } /* end switch */
  } /* end if */
  else
  {
    TRACE_ERROR("Pointer to DTILIB database not existing");
  }
} /* dti_cntrl_mng_cb() */


/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)      MODULE  : PSA_ACI                       |
| STATE   : code                ROUTINE : psaACI_Dti_Req                |
+-----------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL BOOL psaACI_Dti_Req ( T_DTI_CONN_LINK_ID link_id, 
                             T_DTI_ENTITY_ID    peer_ent_id, 
                             UBYTE              dti_conn)
{
  T_DTI_CNTRL info;

  TRACE_FUNCTION("psaACI_Dti_Req()");


  if (dti_cntrl_get_info_from_dti_id( EXTRACT_DTI_ID(link_id), &info) EQ FALSE)
  {
    TRACE_EVENT_P1("cannot find info for dti_id=%d", EXTRACT_DTI_ID(link_id));
    return FALSE;
  }

  if (dti_conn EQ ACI_CONNECT_DTI)
  {
    aci_src_dti_params[info.src_id].dtxState = NOT_READY;

    if (dti_cntrl_set_conn_parms(link_id, DTI_ENTITY_ACI, info.src_id, DTI_SUB_NO_NOTPRESENT) EQ FALSE)
    {
      return FALSE;
    }
    if( !dti_open(
      aci_hDTI,                      /* hDTI            */
      info.src_id,                   /* instance        */
      (U8)peer_ent_id,               /* interface       */
      ACI_DTI_DN_CHANNEL,            /* channel         */
      /* Since DTI-buffering wastes memory we have to implement our own buffers */
      /* This is vital for large outputs like AT+COPN, so please do not modify! */
      /*    0, */                          /* queue_size      */
      1,                       /* limit to 1 DTI buffer */
      DTI_CHANNEL_TO_LOWER_LAYER,    /* direction       */
      /*    DTI_QUEUE_UNBOUNDED, */        /* link_options    */
      DTI_QUEUE_WATERMARK,     /* to enable ACI Buffers */
      DTI_VERSION_10,                /* version         */
      (UBYTE*)dti_entity_name[peer_ent_id].name,  /* neighbor_entity */
      link_id                        /* link_id         */ 
      ))
      return FALSE;
  }
  else
  {
    dti_close
    (
      aci_hDTI,                      /* hDTI      */
      info.src_id,                   /* instance  */
      (U8)peer_ent_id,               /* interface */
      ACI_DTI_DN_CHANNEL,            /* channel   */
      TRUE                           /* flush     */
    );
  }

  return TRUE;
}
#endif /* DTI */
