/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_UARTR
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

#ifndef CMH_UARTR_C
#define CMH_UARTR_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"


#include "aci_io.h"

#include "psa_uart.h"

#include "aci_lst.h"
#include "cmh_uart.h"
#include "gaci.h"


#include "psa.h"
#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif
#include "cmh.h"

#include "aci_mem.h"

#include "psa_l2r.h"
#include "cmh_sm.h"

EXTERN T_ACI_UART_MUX_PARMS holdMuxParms;

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_UART                     |
|                            ROUTINE : cmhUART_ParmsUpdated         |
+-------------------------------------------------------------------+

  PURPOSE : .

*/

GLOBAL void cmhUART_ParmsUpdated( UBYTE device )
{
  T_ACI_DTI_PRC        *src_infos = NULL;
  T_ACI_DEVICE_TYPE     curr_device_type;
  T_ACI_UART_MUX_PARMS *MuxParms;
  T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_ACI};

  TRACE_FUNCTION("cmhUART_ParmsUpdated");

  src_infos = cmhUART_find_dlci (uart_src_params,
                                 device,
                                 UART_DLCI_NOT_MULTIPLEXED);

  if (src_infos EQ NULL)
  {
    TRACE_EVENT ("Error: wrong device number");
    return;
  }

  curr_device_type = cmhUART_GetDeviceType (src_infos->srcId);

  if (curr_device_type EQ DEVICE_TYPE_URT)
  {
    switch ( uartEntcurCmd[src_infos->srcId] )
    {
    case AT_CMD_IPR:
      /* do not send OK: this has been done at the beginning */
      TRACE_EVENT("IPR successfully processed");
      break;
    
    case AT_CMD_ICF:
    case AT_CMD_IFC:
  #ifdef FF_FAX
    case AT_CMD_FLO:
  #endif
      R_AT( RAT_OK, (T_ACI_CMD_SRC)src_infos->srcId )( uartEntcurCmd[src_infos->srcId] );
      break;

    default:
#ifdef DTI
      /* request of a DTI channel to communicate with UART */
      dti_cntrl_est_dpath_indirect ( src_infos->srcId,
                                     entity_list,
                                     1,
                                     SPLIT,
                                     atiUART_dti_cb,
                                     DTI_CPBLTY_CMD,
                                     DTI_CID_NOTPRESENT);
#endif
      break;
    }
    /* reinitialize */
    uartEntcurCmd[src_infos->srcId] = AT_CMD_NONE;
  }
  else if (curr_device_type EQ DEVICE_TYPE_UNKNOWN)
  {
    MuxParms = src_infos->MuxParms;
    if( MuxParms EQ NULL )
    {
      TRACE_EVENT("Multiplexer could not be initialized: wrong parameters");
      return;
    }

    psaUART_StartMux( device,
                      MuxParms->mode,
                      MuxParms->subset,
                      MuxParms->N1,
                      MuxParms->T1,
                      MuxParms->N2,
                      MuxParms->T2,
                      MuxParms->T3);

    memcpy( (CHAR *)&holdMuxParms, (CHAR *)src_infos->MuxParms, sizeof(T_ACI_UART_MUX_PARMS));
    ACI_MFREE( MuxParms );
    src_infos->MuxParms = NULL;
  }
  else
  {
    TRACE_EVENT("Multiplexer could not be initialized: wrong state");
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_UART                     |
|                            ROUTINE : cmhUART_DetectedESC_DTR      |
+-------------------------------------------------------------------+

  PURPOSE : .

*/

GLOBAL void cmhUART_DetectedESC_DTR( UBYTE device, UBYTE dlci, UBYTE cause )
{
  T_DTI_ENTITY_ID  peer_id;

  T_ACI_DTI_PRC*   src_infos = NULL;
  T_DTI_ENTITY_ID  entity_list[] = {DTI_ENTITY_ACI};

  TRACE_FUNCTION("cmhUART_DetectedESC_DTR()");

  uartShrdPrm.dtr_clearcall = FALSE;
  uartShrdPrm.escape_seq    = cause;

  src_infos = cmhUART_find_dlci (uart_src_params,
                                 device,
                                 dlci);

  if (src_infos EQ NULL)
  {
    TRACE_EVENT("[ERR] Wrong dlci");
    return;
  }

  /* escape sequence detected */
  if ((cause EQ UART_DETECT_ESC) OR
     ((cause EQ UART_DETECT_DTR) AND
     ((uartShrdPrm.dtr_behaviour EQ DTR_BEHAVIOUR_CommandMode) OR
     (uartShrdPrm.dtr_behaviour EQ DTR_BEHAVIOUR_ClearCall)) ))
  {
#ifdef DTI
    peer_id = dti_cntrl_get_peer( DTI_ENTITY_UART, device, dlci );
    if ( (uartShrdPrm.dtr_behaviour EQ DTR_BEHAVIOUR_ClearCall) AND
         (cause EQ UART_DETECT_DTR) AND
         (   (peer_id EQ DTI_ENTITY_TRA)
          OR (peer_id EQ DTI_ENTITY_L2R)
/*        OR (peer_id EQ DTI_ENTITY_ACI)     don't disconnect if we are currently in CMD-Mode,
                                             otherwise this would kill the next call attempt */
          OR (peer_id EQ DTI_ENTITY_PPPS) /* also drop PPP sessions */
          OR (peer_id EQ DTI_ENTITY_PPPC)
          ) )
    {
      TRACE_EVENT("uartShrdPrm.dtr_clearcall = TRUE");
      uartShrdPrm.dtr_clearcall = TRUE; /* this is only for CSD (TRA, L2R) */
    }
   /*
    * Inform the relevant CMH or close the user plane.
    */
    switch ( peer_id )
    {

#if defined FAX_AND_DATA
      case ( DTI_ENTITY_L2R ):
    {
      psaL2R_ESC ( src_infos->srcId );
        break;
      }
#endif /* FAX_AND_DATA */

#if defined GPRS
      case ( DTI_ENTITY_PPPS ):
      {       
        T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = pdp_context_find_node_from_dti_id ( EXTRACT_DTI_ID( dti_cntrl_get_link_id( DTI_ENTITY_UART, device, dlci ) ) );
        if( !p_pdp_context_node )
        {
          TRACE_EVENT("ERROR: PDP context not defined");
          return;
        }
        cmhSM_deactivateAContext(CMD_SRC_NONE, p_pdp_context_node->cid);
        break;
      }
#endif /* GPRS */
      default:
      {
        dti_cntrl_est_dpath_indirect ( src_infos->srcId,
                                       entity_list,
                                       1,
                                       SPLIT,
                                       atiUART_dti_cb,
                                       DTI_CPBLTY_CMD,
                                       DTI_CID_NOTPRESENT);
        break;
      }
    }
#endif
  }
  /* DTR line of serial link drops */
  else if (cause EQ UART_DETECT_DTR)
  {
    if (uartShrdPrm.dtr_behaviour EQ DTR_BEHAVIOUR_Ignore)
    {
      TRACE_EVENT("DCE ignores DTR");
    }
    else
    {
      TRACE_EVENT("[ERR] Wrong dtr_behaviour value");
    }
  }
  else
  {
    TRACE_EVENT("[ERR] Wrong cause value in UART_DETECTED_IND");
  }
}

#endif /* UART */
