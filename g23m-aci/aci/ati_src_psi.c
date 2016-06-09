/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  ATI_SRC_UARTC
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
#ifdef FF_PSI

#define ATI_SRC_PSI_C


#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */

#include "aci_lst.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"
#include "psa_psi.h"
#include "cmh_psi.h"
#include "aci_io.h"
#include "ati_src_psi.h"
#include "aci_mem.h"
#include "aci.h"
#include "psa.h"
#include "cmh.h"

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

#ifdef _SIMULATION_
EXTERN T_ACI_LIST *ati_src_list;
#endif /* _SIMULATION_ */

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

GLOBAL void psi_InitCmdStruct( T_ACI_DTI_PRC_PSI *cmd_struct )
{
  cmd_struct->devId            = 0;
  cmd_struct->dlci              = NOT_PRESENT_8BIT;
  cmd_struct->RecState          = NOT_INITIALIZED;
  cmd_struct->LineState         = LINE_CONNECTING;
  cmd_struct->run_cmd           = FALSE;
  cmd_struct->first_output      = FALSE;
  cmd_struct->data_cntr         = 0;
  cmd_struct->MuxParms          = NULL;
  cmd_struct->large_type        = FALSE;
  cmd_struct->data_buffer.data_list       = NULL;
  cmd_struct->data_buffer.max_buffer_size = 0;
  cmd_struct->data_buffer.create_time     = 0;
  cmd_struct->device_type       = DEVICE_TYPE_UNKNOWN;
#ifdef FF_BAT
  cmd_struct->active_client     = 0xFE; /* BAT control channel */
  cmd_struct->bat_client        = NULL;
  cmd_struct->max_clients       = 0;
  cmd_struct->queue             = NULL; /* queue for pending BAT commands */
  cmd_struct->send_data         = NULL; /* T_desc2 of the control and broadcast cahnnel */
  cmd_struct->search_str_ptr    = NULL; /* search string pointer */
#endif /* FF_BAT */
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  :                         |
|                                 ROUTINE : psi_new_source          |
+-------------------------------------------------------------------+

  PURPOSE : create a new PSI source. 
            In case of BAT (DIO_DATA_PKT), do not create a ATI source.
            For the phase 2 of the BAT project, where ATI is split fom ACI,
            we need a psi_new_source() function on ACI level !!!
*/
GLOBAL UBYTE psi_new_source( U32 devId, UBYTE dlci , U32 dio_driver_type) // RM 26-05-04 Nice
{
  T_ACI_DTI_PRC_PSI *cmd_struct;
  UBYTE src_id;

  TRACE_FUNCTION("psi_new_source()");
 
  src_id = ati_init(ATI_SRC_TYPE_PSI, psi_src_result_cb, psi_src_line_state_cb);

#ifdef FF_BAT
  if (dio_driver_type EQ DIO_DATA_PKT)
  {
    /* 
     * we are only interested in getting a src_id for BAT, but not in a ATI source
     * WHAT HAPPENS WITH THIS SOURCE ID WHEN A REAL ATI SOURCE IS OPENED ?
     * DO WE HAVE IT THEN TWICE FOR TWO DIFFERENT SOURCES (BAT and ATI) ???
     */
     
#ifndef _SIMULATION_     
    ati_finit(src_id); /* SAVES MEMORY !!!*/

    { /* special SKA hack to make the BAT source marked as used */
      int i;
      U8  tst_src;

      for (i = 0; i < CMD_SRC_MAX; i++)
      {
        tst_src = (0x01 << i) & used_sources;
        if (!tst_src)
          break;
      }
      if (tst_src)
      {
        TRACE_EVENT_P1("psi_new_source(): No more sources available ! used_sources: 0x%04X", used_sources);
        return (0);
      }
      used_sources |= (0x01 << i);
    }
#endif

  }
#endif

  TRACE_EVENT_P1 ("psi_new_source: srcId=%d", src_id);

  ACI_MALLOC (cmd_struct, sizeof(T_ACI_DTI_PRC_PSI));
  
  psi_InitCmdStruct( cmd_struct );

  cmd_struct->srcId = src_id;
  cmd_struct->devId = devId;
  cmd_struct->dlci  = dlci;

  /*
   *   Determine the mode of the command source based on the
   *   source ID and 'dio_driver_type'.
   */
  if (src_id < CMD_SRC_MAX)
  {
    if (src_id EQ CMD_SRC_LCL)
    {
      aci_cmd_src_mode_set(src_id,CMD_MODE_ACI);
    }
#ifdef FF_BAT
    else if (dio_driver_type EQ DIO_DATA_PKT)
    {
      aci_cmd_src_mode_set(src_id,CMD_MODE_BAT);
      sAT_PercentCSQ((T_ACI_CMD_SRC)src_id, CSQ_Enable); /* CSQ indication regular enabled for BAT source */
    }
#endif
    else
    {
      aci_cmd_src_mode_set(src_id,CMD_MODE_ATI);
    }
  }
  
  if (insert_list (psi_src_params, cmd_struct) NEQ TRUE)
  {
    TRACE_EVENT ("new source is not able to creat");
  }
  
  cmhPSI_AddDeviceToDcbParameter(devId ,dio_driver_type);
  
  return(src_id);
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  :                         |
|                                 ROUTINE : psi_erase_source        |
+-------------------------------------------------------------------+

  PURPOSE : erase an entry element from the psi_src_params list.
*/
EXTERN void dti_cntrl_erase_entry(UBYTE dti_id); 

GLOBAL void psi_erase_source( UBYTE srcId )
{
  T_DTI_CNTRL info;

  TRACE_FUNCTION("psi_erase_source");
 
  ati_finit (srcId);

  dti_cntrl_get_info_from_src_id (srcId, &info);

  dti_cntrl_erase_entry( info.dti_id );

  dti_cntrl_clear_conn_parms( info.dti_id );

  TRACE_EVENT_P1("Source nb %d deleted", srcId);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_PSI                     |
|                            ROUTINE : atiPSI_dti_cb         |
+-------------------------------------------------------------------+

  PURPOSE : callback for dti manager.

*/
GLOBAL BOOL atiPSI_dti_cb( UBYTE dti_id, T_DTI_CONN_STATE result_type )
{
#ifdef _SIMULATION_
  T_ATI_SRC_PARAMS *src_params;
#endif /* _SIMULATION_ */

  T_ACI_DTI_PRC_PSI *srcInfos = NULL;
  UBYTE saveState;
  BOOL  is_psi_src = TRUE;      /* for simulation */
  T_DTI_CNTRL    info;
  
  TRACE_FUNCTION("atiPSI_dti_cb");


  if (dti_cntrl_get_info_from_dti_id( dti_id, &info) EQ FALSE)
  {
    TRACE_EVENT_P1("cannot find info for dti_id=%d", dti_id);
    return FALSE;
  }

  srcInfos = find_element (psi_src_params, info.src_id, cmhPSItest_srcId);
  if (srcInfos EQ NULL)
  {
    TRACE_EVENT_P1 ("[ERR] atiPSI_dti_cb: srcId=%d not found", info.src_id) ;
    return FALSE ;
  }

  saveState = srcInfos->LineState;      

#ifdef _SIMULATION_
  src_params = find_element (ati_src_list, info.src_id, search_ati_src_id);
  if (src_params EQ NULL)
  {
    TRACE_EVENT ("[ERR] SIMULATION: atiPSI_dti_cb: source ID not found");
    return FALSE;
  }
  if (src_params->src_type EQ ATI_SRC_TYPE_TST)
  {
    is_psi_src = FALSE;
  }
#endif /* _SIMULATION_ */

  switch(result_type)
  {
    /* PSI and ACI connected */
    case(DTI_CONN_STATE_CONNECTED):
      /* first set the line state, then care for DCD */
      srcInfos->LineState = LINE_CONNECTED;

      ati_switch_mode(info.src_id, ATI_CMD_MODE);

      if (saveState EQ LINE_CONNECTING)
      {

        if (is_psi_src)
        {
          /* creation of a new connection: set DCD */
          io_setDCD ((T_ACI_CMD_SRC)info.src_id, IO_DCD_OFF);
        }

#ifndef _SIMULATION_
#ifdef FF_BAT
        if (aci_cmd_src_mode_get((T_ACI_CMD_SRC)info.src_id) NEQ CMD_MODE_BAT)
#endif
        {
          io_sendMessage (srcInfos->srcId, "AT-Command Interpreter ready", 
                          ATI_NORMAL_OUTPUT);
        }
#endif /* _SIMULATION_ */
      }

      if (is_psi_src)
      {
        /* tells PSI that ACI is ready to get some data */
        cmhPSI_getdata ((T_ACI_CMD_SRC)srcInfos->srcId);
      }
      break;

    /* PSI and data entity temporary disconnected */ 
    case(DTI_CONN_STATE_CONNECTING):
      break;
      
    case(DTI_CONN_STATE_DISCONNECTING):
      srcInfos->LineState = LINE_TMP_DISCONNECTING;
      ati_switch_mode(info.src_id, ATI_DATA_MODE);
      break;

    case(DTI_CONN_STATE_DISCONNECTED):
      srcInfos->LineState = LINE_TMP_DISCONNECTED;
      dti_cntrl_clear_conn_parms( dti_id );
      break;

    case(DTI_CONN_STATE_ERROR):
      dti_cntrl_close_dpath_from_dti_id( dti_id );
      break;

    default:
      break;
  }

  /* Maybe drop call if DTR is asserted */
  if (psiShrdPrm.dtr_clearcall EQ TRUE)
  {
    T_ACI_RETURN l_ret;

    l_ret = sAT_H((T_ACI_CMD_SRC)info.src_id);
    if (l_ret NEQ AT_EXCT AND l_ret NEQ AT_CMPL)
    {
      psiShrdPrm.dtr_clearcall = FALSE;
      TRACE_ERROR("DTR-Drop sAT_H() error");
    }
  }

  return TRUE;  
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_DTIR                |
|                                 ROUTINE : psi_src_proc_chars     |
+-------------------------------------------------------------------+

  PURPOSE : Check if characters received are ok, and pack them into the command line.
            return TRUE if ACI allows more data from DTI.
*/

GLOBAL BOOL psi_src_proc_chars ( UBYTE *chars, 
                                  USHORT len, 
                                  T_ACI_DTI_PRC_PSI *elem )
{
  if (len >= MAX_CMD_LEN)
  {
    TRACE_ERROR ("[ERR] [ATI]: command line too long");
    return (TRUE);
  }
  if ((UBYTE)(elem->run_cmd))
  {
    ati_abort (elem->srcId);
  }
  else
  {
    ati_execute ((UBYTE)(elem->srcId), chars, len);
  }
  return (TRUE);
}

#endif /*FF_PSI*/
/*==== EOF =======================================================*/

