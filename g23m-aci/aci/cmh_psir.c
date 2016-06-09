/*
+------------------------------------------------------------------------------
|  File:       cmh_psir.c
+------------------------------------------------------------------------------
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
+------------------------------------------------------------------------------
| Purpose:     This module defines the functions used by the command
|              handler for packet I/O (PSI).
+------------------------------------------------------------------------------
*/
#ifdef FF_PSI

#ifndef CMH_PSIR_C
#define CMH_PSIR_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/

#ifdef GPRS
#include "gprs.h"
#endif
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "aci.h"
#include "psa.h"

#include "cmh.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"
#ifdef GPRS
#include "gaci.h"
#include "gaci_cmh.h"
#endif


#include "sap_dti.h"
#include "cmh_sm.h"

#include "psa_psi.h"
#include "cmh_psi.h"
#include "ati_src_psi.h"


#ifndef CMH_UARTR_C
#define CMH_UARTR_C
#endif

#include "aci_all.h"


/*==== CONSTANTS ==================================================*/
#define MAX_BAUD_RATE 20
/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                      MODULE  : CMH_PSI       |
| STATE   : finished                         ROUTINE : cmhPSI_Ind    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void cmhPSI_Ind (T_ACI_PSI *psi_indic_msg, UBYTE src_id,U32 dio_cap)
{
#ifdef DTI
  UBYTE capability = DTI_CPBLTY_NO;
  UBYTE dti_capability;
  BOOL ret_val1 = FALSE;
  BOOL ret_val2 = FALSE;
  T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_ACI};
 
  TRACE_FUNCTION ("cmhPSI_Ind()");
  /* register device to the DTI Control Manager */
 switch (dio_cap)
  {
     case DIO_DATA_SER:
         capability = DTI_CPBLTY_SER;
         break;
     case DIO_DATA_MUX:
         capability = DTI_CPBLTY_SER;
         break;
     case DIO_DATA_PKT:
         capability = DTI_CPBLTY_PKT;
         break;
  }
  dti_capability = capability;
  ret_val1 = dti_cntrl_new_device ( src_id,   /* ATI source ID */
                         DTI_ENTITY_PSI,         /* device ID  */
                         (UBYTE)(psi_indic_msg->devId&DIO_DEVICE_MASK), /* device number  */ 
                         UART_DLCI_NOT_MULTIPLEXED,/* sub-channel number */
                         DTI_PORT_NUMBER_NOTPRESENT,/* ATI-RIV port number */
                         (UBYTE)(dti_capability|DTI_CPBLTY_CMD),/* capability */
                         (UBYTE)((psi_indic_msg->devId&DIO_DRV_MASK)>>24),/* driver id */
                         (UBYTE)((psi_indic_msg->devId&ACI_DIO_ID_MASK)>>8)); /* dio id */

  if((psi_indic_msg->psi_data_mode EQ DRIVER_MODE_AT) OR
     (psi_indic_msg->psi_data_mode EQ DRIVER_MODE_AT_DATA))
  {

    ret_val2 = dti_cntrl_est_dpath_indirect ( src_id,
                                              entity_list,
                                              1,
                                              SPLIT,
                                              atiPSI_dti_cb,
                            /*dti_capability*/DTI_CPBLTY_CMD,
                                              DTI_CID_NOTPRESENT);
  }
  else
  {
    ret_val2 = TRUE;
  }
  if ((ret_val1 EQ TRUE) AND (ret_val2 EQ TRUE))
  {
    psaPSI_ConnRes ( psi_indic_msg->devId);
  }
  else
  {
    TRACE_EVENT ("ERROR: DTI MANAGER CAN NOT REGISTER DEVICE OR SET UP DTI CONNECTION");
  }
#else
    TRACE_EVENT ("ERROR: DTI MANAGER NOT ENABLED");
#endif /* DTI */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                     MODULE  : CMH_PSI        |
| STATE   : finished                    ROUTINE : cmhPSI_DTI_OpenCnf |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void cmhPSI_DTI_OpenCnf (U32 devId, U32 link_id, T_DTI_CONN_RESULT result)
{
#ifdef DTI
  
  TRACE_FUNCTION ("cmhPSI_DTI_OpenCnf()");
   
  dti_cntrl_entity_connected( link_id, DTI_ENTITY_PSI, result);
#endif /* DTI */
}

/*
+--------------------------------------------------------------------+
| PROJECT :                             MODULE  : CMH_PSI            |
| STATE   : finished                    ROUTINE : cmhPSI_DTI_Close   |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void cmhPSI_DTI_Close (U32 devId, T_PSI_DTI_CLS_TYP  state, U32 link_id)
{
#ifdef DTI
  #ifdef GPRS
  UBYTE cid = PDP_CONTEXT_CID_INVALID;
 #endif

  TRACE_FUNCTION ("cmhPSI_Close()");
  #ifdef GPRS
  cid = gaci_get_cid_over_link_id(link_id);  
  #endif
  
  switch(state)
  {
    case PSI_DTI_CLS_CNF:
      /*DTI connection has been closed*/
      dti_cntrl_entity_disconnected(link_id, DTI_ENTITY_PSI);
      break;

    case PSI_DTI_CLS_IND:
      /*dti_cntrl_close_dpath_from_dti_id(EXTRACT_DTI_ID(link_id));*/
      dti_cntrl_entity_disconnected(link_id, DTI_ENTITY_PSI);
      break;

    default:
      break;
  }
#ifdef GPRS
   if( PDP_CONTEXT_CID_INVALID NEQ cid)
   {
    /* Make sure the context gets de-activated in any case */
    cmhSM_deactivateAContext(CMD_SRC_NONE, cid);

    gaci_RAT_caller(RAT_NO_CARRIER, cid, AT_CMD_CGDATA, CME_ERR_Unknown);
  }
#endif

#endif /* DTI */
}

#endif /*FF_PSI*/
/*==== EOF ========================================================*/
