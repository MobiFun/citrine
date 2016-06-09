/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_L2RR
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
|  Purpose :  This module defines the functions which are responsible
|             for the responses of the protocol stack adapter for
|             layer 2 relay.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_L2RR_C
#define CMH_L2RR_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_lst.h"
#include "dti.h"      /* functionality of the dti library */

#include "aci_io.h"

#include "aci_fd.h"

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "aci.h"
#include "psa.h"
#include "psa_l2r.h"
#include "psa_cc.h"
#include "cmh.h"
#include "psa_ra.h"
#include "cmh_ra.h"
#include "cmh_l2r.h"
#include "cmh_cc.h"

#include "psa_tra.h"


#include "cmh_ra.h"
#ifdef UART
#include "psa_uart.h"
#include "cmh_uart.h"
#endif

#ifdef FF_PSI
#include "psa_psi.h"
#include "cmh_psi.h"
#include "ati_src_psi.h"
#endif /*FF_PSI*/

#ifdef DTI
#include "sap_dti.h"
#endif

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_L2RR                     |
|                            ROUTINE : cmhL2R_TRA_Enabled           |
+-------------------------------------------------------------------+

  PURPOSE : L2R entity enabled, data mode

*/

GLOBAL void cmhL2R_TRA_Enabled ( void )
{
  SHORT cId;          /* holds call id */

  TRACE_FUNCTION ("cmhL2R_TRA_Enabled()");

  /* find active call */
  cId = psaCC_ctbFindCall( OWN_SRC_INV, CS_ACT, NO_VLD_CT );

  if( cId NEQ NO_ENTRY AND psaCC_ctb(cId)->curCmd EQ AT_CMD_O )
  {
    cmhCC_PrepareCmdEnd (cId, NULL, NULL);
  }
}
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_L2RR                     |
|                            ROUTINE : cmhL2R_Deactivated           |
+-------------------------------------------------------------------+

  PURPOSE : L2R entity deactivated, connection disconnected

*/

GLOBAL void cmhL2R_Deactivated ( void )
{
  TRACE_FUNCTION ("cmhL2R_Deactivated()");

  cmhCC_L2R_or_TRA_Deactivated(raShrdPrm.cId); /* see issue 7527 why raShrdPrm.cId */

  l2rEntStat.curCmd = AT_CMD_NONE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_L2RR                     |
|                            ROUTINE : cmhTRA_Deactivated           |
+-------------------------------------------------------------------+

  PURPOSE : TRA entity deactivated, connection disconnected

*/

GLOBAL void cmhTRA_Deactivated ( void )
{
  TRACE_FUNCTION ("cmhTRA_Deactivated()");

  cmhCC_L2R_or_TRA_Deactivated(raShrdPrm.cId);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_L2RR                     |
|                            ROUTINE : cmhL2R_TRA_Disabled          |
+-------------------------------------------------------------------+

  PURPOSE : L2R entity disabled, escape from data mode

*/

GLOBAL void cmhL2R_TRA_Disabled ( T_DTI_ENTITY_ID entityId )
{
  S8 source = psaCC_ctb(raShrdPrm.cId)->calOwn;

  TRACE_FUNCTION ("cmhL2R_TRA_Disabled()");
  switch (entityId)
  {
#ifdef UART
     case DTI_ENTITY_UART:
       /* return to command mode */
        if (uartShrdPrm.dtr_clearcall EQ FALSE)
        {
           R_AT( RAT_OK, (T_ACI_CMD_SRC)source )
           ( AT_CMD_NONE );
        }
        break;
#endif
#if defined (FF_PSI) AND defined (DTI)
     case DTI_ENTITY_PSI:
       /* return to command mode */
        if (psiShrdPrm.dtr_clearcall EQ FALSE)
        {
           R_AT( RAT_OK, (T_ACI_CMD_SRC)source )
           ( AT_CMD_NONE );
        }
        break;
#endif /*FF_PSI*/
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_L2RR                     |
|                            ROUTINE : cmhL2R_Failure               |
+-------------------------------------------------------------------+

  PURPOSE : L2R entity failure, connection will be disconnected

*/

GLOBAL void cmhL2R_Failure ( void )
{

  TRACE_EVENT_P1("RLP ERROR: 0x%04x", l2rShrdPrm.set_prm[l2rShrdPrm.owner].err_cause );

/*
 *-------------------------------------------------------------------
 * general failure indication
 *-------------------------------------------------------------------
 */
  cmhCC_L2R_Failed();
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_L2RR           |
| STATE   : code                        ROUTINE : L2R_connect_dti_cb |
+--------------------------------------------------------------------+

  PURPOSE : Callback for connection between L2R and UART.

*/
#ifdef DTI
GLOBAL BOOL L2R_connect_dti_cb(UBYTE dti_id, T_DTI_CONN_STATE result_type)
{
  T_ACI_DTI_PRC *srcInfos;
#ifdef FF_PSI
  T_ACI_DTI_PRC_PSI *srcInfos_psi;
#endif /*FF_PSI*/
  T_DTI_ENTITY_ID entityId = DTI_ENTITY_UART;
  T_DTI_CNTRL    info;

  TRACE_FUNCTION("L2R_connect_dti_cb");


  if (dti_cntrl_get_info_from_dti_id( dti_id, &info) EQ FALSE)
  {
    TRACE_EVENT_P1("cannot find info for dti_id=%d", dti_id);
    return FALSE;
  }

  if ((srcInfos = find_element (uart_src_params, info.src_id, cmhUARTtest_srcId)) NEQ NULL)
    entityId = DTI_ENTITY_UART;
#if defined (FF_PSI) AND defined (DTI)
  if ((srcInfos_psi = find_element (psi_src_params, info.src_id, cmhPSItest_srcId)) NEQ NULL)
    entityId = DTI_ENTITY_PSI;
  else if (srcInfos EQ NULL AND srcInfos_psi EQ NULL)
#else
  else if (srcInfos EQ NULL)
#endif /*FF_PSI*/
  {
    TRACE_EVENT_P1 ("[ERR] L2R_connect_dti_cb: srcId=%d not found", info.src_id);
    return FALSE;
  }

  switch( result_type)
  {
    case(DTI_CONN_STATE_CONNECTED):
      cmhL2R_TRA_Enabled();
      break;

    case(DTI_CONN_STATE_DISCONNECTED):

      dti_cntrl_clear_conn_parms( dti_id );
#ifdef FF_PSI
      if (srcInfos_psi EQ NULL)
      {
        TRACE_EVENT ("[ERR] L2R_connect_dti_cb: srcInfos_psi = NULL");
        return FALSE;
      }
#endif /*FF_PSI*/
      if (srcInfos EQ NULL)
      {
        TRACE_EVENT ("[ERR] L2R_connect_dti_cb: srcInfos = NULL");
        return FALSE;
      }
	  
      if (l2rEntStat.isTempDisconnected)
      {
#ifdef FF_PSI
        if (entityId EQ DTI_ENTITY_PSI)
          BITFIELD_SET (srcInfos_psi->data_cntr, UART_DTI_SB_BIT);
        else
#endif /*FF_PSI*/
          BITFIELD_SET (srcInfos->data_cntr, UART_DTI_SB_BIT);
        l2rEntStat.isTempDisconnected = FALSE;
        cmhL2R_TRA_Disabled(entityId);
      }
      else
      {
        /* action already initiated in psa_l2r */
        /* cmhL2R_Deactivate(); */
#ifdef FF_PSI
        if (entityId EQ DTI_ENTITY_PSI)
          BITFIELD_CLEAR (srcInfos_psi->data_cntr, UART_DTI_SB_BIT);
        else
#endif /*FF_PSI*/
          BITFIELD_CLEAR (srcInfos->data_cntr, UART_DTI_SB_BIT);
      }

      if (dti_cntrl_is_dti_id_to_reconnect(dti_id))
      {
        T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_ACI};
        dti_cntrl_clear_dti_id_to_reconnect(dti_id);
#ifdef FF_PSI
        if (entityId EQ DTI_ENTITY_PSI)
          dti_cntrl_est_dpath_indirect ( info.src_id,
                                       entity_list,
                                       1,
                                       SPLIT,
                                       atiPSI_dti_cb,
                                       DTI_CPBLTY_CMD,
                                       DTI_CID_NOTPRESENT);

       else
#endif /*FF_PSI*/
          dti_cntrl_est_dpath_indirect ( info.src_id,
                                       entity_list,
                                       1,
                                       SPLIT,
                                       atiUART_dti_cb,
                                       DTI_CPBLTY_CMD,
                                       DTI_CID_NOTPRESENT);
      }

      break;

    case(DTI_CONN_STATE_CONNECTING):
    case(DTI_CONN_STATE_DISCONNECTING):
      break;
    case(DTI_CONN_STATE_ERROR):
      dti_cntrl_close_dpath_from_dti_id( dti_id );
      break;

    default:
      TRACE_EVENT("L2R_connect_dti_cb call with not awaited value");
      break;
  }
  return TRUE;
}
#endif /* DTI */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_L2RR           |
| STATE   : code                        ROUTINE : TRA_connect_dti_cb |
+--------------------------------------------------------------------+

  PURPOSE : Callback for connection between TRA and UART.

*/

#ifdef DTI
GLOBAL BOOL TRA_connect_dti_cb(UBYTE dti_id, T_DTI_CONN_STATE result_type)
{
  T_ACI_DTI_PRC *srcInfos;
#ifdef FF_PSI
  T_ACI_DTI_PRC_PSI *srcInfos_psi;
#endif /*FF_PSI*/
  T_DTI_ENTITY_ID entityId = DTI_ENTITY_UART;
  T_DTI_CNTRL    info;

  TRACE_FUNCTION("TRA_connect_dti_cb");

  
  if (dti_cntrl_get_info_from_dti_id( dti_id, &info) EQ FALSE)
  {
    TRACE_EVENT_P1("cannot find info for dti_id=%d", dti_id);
    return FALSE;
  }

  if ((srcInfos = find_element (uart_src_params, info.src_id, cmhUARTtest_srcId)) NEQ NULL)
    entityId = DTI_ENTITY_UART;
#if defined (FF_PSI) AND defined (DTI)
  if((srcInfos_psi= find_element (psi_src_params, info.src_id, cmhPSItest_srcId)) NEQ NULL)
    entityId = DTI_ENTITY_PSI;
  else if (srcInfos EQ NULL AND srcInfos_psi EQ NULL)
#else
  else if (srcInfos EQ NULL)
#endif /*FF_PSI*/
  {
    TRACE_EVENT_P1 ("[ERR] TRA_connect_dti_cb: srcId=%d not found", info.src_id);
    return FALSE;
  }

  switch( result_type)
  {
    case(DTI_CONN_STATE_CONNECTED):
      cmhL2R_TRA_Enabled();
      break;

    case(DTI_CONN_STATE_DISCONNECTED):

      dti_cntrl_clear_conn_parms( dti_id );
#ifdef FF_PSI
      if (srcInfos_psi EQ NULL)
      {
        TRACE_EVENT ("[ERR] L2R_connect_dti_cb: srcInfos_psi = NULL");
        return FALSE;
      }
#endif /*FF_PSI*/
      if (srcInfos EQ NULL)
      {
        TRACE_EVENT ("[ERR] L2R_connect_dti_cb: srcInfos = NULL");
        return FALSE;
      }
	  
      if (raEntStat.isTempDisconnected)
      {
#ifdef FF_PSI
        if (entityId EQ DTI_ENTITY_PSI)
          BITFIELD_SET (srcInfos_psi->data_cntr, UART_DTI_SB_BIT);
        else
#endif /*FF_PSI*/
          BITFIELD_SET (srcInfos->data_cntr, UART_DTI_SB_BIT);
        raEntStat.isTempDisconnected = FALSE;
        cmhL2R_TRA_Disabled(entityId);
      }
      else
      {
#ifdef FF_PSI
        if (entityId EQ DTI_ENTITY_PSI)
          BITFIELD_CLEAR (srcInfos_psi->data_cntr, UART_DTI_SB_BIT);
        else
#endif /*FF_PSI*/
          BITFIELD_CLEAR (srcInfos->data_cntr, UART_DTI_SB_BIT);
        /* psaTRA_Deactivate(); */  /* This is done in psa_trap later, otherwise TRA will crash! */
      }
      break;

    case(DTI_CONN_STATE_CONNECTING):
    case(DTI_CONN_STATE_DISCONNECTING):
      break;
    case(DTI_CONN_STATE_ERROR):
      dti_cntrl_close_dpath_from_dti_id( dti_id );
      break;

    default:
      TRACE_EVENT("TRA_connect_dti_cb call with not awaited value");
      break;
  }
  return TRUE;
}
#endif /* DTI */

/*==== EOF ========================================================*/

