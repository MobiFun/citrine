/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_TRAS
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
#ifdef DTI

#ifndef PSA_TRAS_C
#define PSA_TRAS_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "dti.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "aci.h"
#include "psa.h"
#include "cmh.h"
#include "aci_fd.h"
#include "cmh_ra.h"
#include "psa_l2r.h"
#include "dti_conn_mng.h"
#include "cmh_l2r.h"
#include "psa_uart.h"

#include "psa_util.h"
#include "psa_tra.h"

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_TRA                 |
|                                 ROUTINE : psaTRA_Activate         |
+-------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void psaTRA_Activate (void)
{
  TRACE_FUNCTION ("psaTRA_Activate()");
  TRA_is_activated = FALSE;
  {
    PALLOC (tra_activate_req, TRA_ACTIVATE_REQ);
#ifdef TI_PS_HCOMM_CHANGE
    PSEND (hCommTRA, tra_activate_req);
#else 
    PSENDX (TRA, tra_activate_req);
#endif
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_TRA                 |
|                                 ROUTINE : psaTRA_Dti_Req          |
+-------------------------------------------------------------------+

  PURPOSE : connect or disconnect TRA (depends on dti_conn).

*/

GLOBAL void psaTRA_Dti_Req (T_DTI_CONN_LINK_ID link_id, UBYTE dti_conn, UBYTE peer)
{
  
  UBYTE dti_id = EXTRACT_DTI_ID( link_id );

  TRACE_FUNCTION ("psaTRA_Dti_Req()");

  {
    PALLOC (tra_dti_req, TRA_DTI_REQ);

    tra_dti_req->dti_conn      = dti_conn;
    tra_dti_req->link_id       = link_id;

    if (IS_SRC_BT(dti_id))
    {
      TRACE_EVENT("psa_tras - BT_ADAPTER Act");
      strcpy((CHAR*)tra_dti_req->entity_name, BTI_NAME);
      tra_dti_req->dti_direction   = DTI_CHANNEL_TO_HIGHER_LAYER;
    }
#ifdef FF_SAT_E    
    else if (peer EQ DTI_ENTITY_SIM)
    {
      strcpy((CHAR*)tra_dti_req->entity_name, SIM_NAME);
      tra_dti_req->dti_direction   = DTI_CHANNEL_TO_HIGHER_LAYER;
    }
#endif /* FF_SAT_E */    
    else if (peer EQ DTI_ENTITY_UART)
    {
      strcpy((CHAR*)tra_dti_req->entity_name, UART_NAME);
      tra_dti_req->dti_direction   = DTI_CHANNEL_TO_LOWER_LAYER;
    }
    else
    {
      strcpy((CHAR*)tra_dti_req->entity_name, PSI_NAME);
      tra_dti_req->dti_direction   = DTI_CHANNEL_TO_LOWER_LAYER; 
    }
    if (dti_conn EQ TRA_CONNECT_DTI)
    {
      if (dti_cntrl_set_conn_parms(link_id, DTI_ENTITY_TRA, DTI_INSTANCE_NOTPRESENT, DTI_SUB_NO_NOTPRESENT) EQ FALSE)
      {
        return;
      }
    }

#ifdef TI_PS_HCOMM_CHANGE
    PSEND (hCommTRA, tra_dti_req);
#else 
    PSENDX (TRA, tra_dti_req);
#endif
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_TRA                 |
|                                 ROUTINE : psaTRA_Deactivate       |
+-------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void psaTRA_Deactivate (void)
{
  TRACE_FUNCTION ("psaTRA_Deactivate()");

  if (TRA_is_activated EQ FALSE)
    return;
  else
    TRA_is_activated = FALSE;

  {
    PALLOC (tra_deactivate_req, TRA_DEACTIVATE_REQ);
#ifdef TI_PS_HCOMM_CHANGE
    PSEND (hCommTRA, tra_deactivate_req);
#else 
    PSENDX (TRA, tra_deactivate_req);
#endif
  }
}
#endif /* DTI */
