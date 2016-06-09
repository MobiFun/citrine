/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_T30S
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
|  Purpose :  This module defines the signalling functions of the
|             protocol stack adapter for T30.
+----------------------------------------------------------------------------- 
*/ 

#if defined (DTI) || defined (FF_FAX)

#ifndef PSA_T30S_C
#define PSA_T30S_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "dti.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "aci_fd.h"
#include "aci.h"
#include "psa.h"
#include "psa_t30.h"
#include "cmh.h"
#include "cmh_t30.h"


/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_T30                      |
|                            ROUTINE : psaT30_Dti_Req               |
+-------------------------------------------------------------------+

  PURPOSE : connect or disconnect T30 (depends on dti_conn).
*/

GLOBAL void psaT30_Dti_Req(T_DTI_CONN_LINK_ID  link_id, UBYTE dti_conn)
{
  T_DTI_CNTRL  device_info;
  dti_cntrl_get_info_from_dti_id(EXTRACT_DTI_ID(link_id), &device_info);

  TRACE_FUNCTION ("psaT30_Dti_Req()");

  {
    PALLOC (t30_dti_req, T30_DTI_REQ);
    
    t30_dti_req->dti_conn      = dti_conn;
    if (device_info.dev_id EQ DTI_ENTITY_UART)
      strcpy((CHAR*)t30_dti_req->entity_name, UART_NAME);
    else if (device_info.dev_id EQ DTI_ENTITY_PSI)
      strcpy((CHAR*)t30_dti_req->entity_name, PSI_NAME);

    t30_dti_req->link_id       = link_id;
    t30_dti_req->dti_direction = DTI_CHANNEL_TO_LOWER_LAYER;

    if (dti_conn EQ T30_CONNECT_DTI)
    {
      if (dti_cntrl_set_conn_parms(link_id, DTI_ENTITY_T30, DTI_INSTANCE_NOTPRESENT, DTI_SUB_NO_NOTPRESENT) EQ FALSE)
      {
        return;
      }
    }

    PSENDX (T30, t30_dti_req);
  }
}     
       
/*

+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_T30                      |
|                            ROUTINE : psaT30_Activate              |
+-------------------------------------------------------------------+

  PURPOSE : activate T30.

*/

GLOBAL void psaT30_Activate (void)
{
  TRACE_FUNCTION ("psaT30_Activate()");

  {
    PALLOC (t30_activate_req, T30_ACTIVATE_REQ); 

    t30_activate_req->trans_rate      = t30ShrdPrm.trans_rate;     
    t30_activate_req->half_rate       = t30ShrdPrm.half_rate;      
    t30_activate_req->threshold       = t30ShrdPrm.threshold;      
    t30_activate_req->frames_per_prim = t30ShrdPrm.frames_per_prim;
    t30_activate_req->bitorder        = t30ShrdPrm.bitord;

    PSENDX (T30, t30_activate_req);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_T30                      |
|                            ROUTINE : psaT30_Capabilities          |
+-------------------------------------------------------------------+

  PURPOSE : send selected capabilities.

*/

GLOBAL void psaT30_Capabilities (void)
{
  TRACE_FUNCTION ("psaT30_Capabilities()");

  {
    PALLOC (t30_cap_req, T30_CAP_REQ);

    memcpy (&t30_cap_req->hdlc_info, &t30ShrdPrm.hdlc_snd,
            sizeof (T_hdlc_info));

    PSENDX (T30, t30_cap_req);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_T30                      |
|                            ROUTINE : psaT30_Config                |
+-------------------------------------------------------------------+

  PURPOSE : t30 configuration.

*/

GLOBAL void psaT30_Config (void)
{
  TRACE_FUNCTION ("psaT30_Config()");

  {
    PALLOC (t30_config_req, T30_CONFIG_REQ);

    t30_config_req->hdlc_report = t30ShrdPrm.hdlc_report;
    t30_config_req->test_mode   = t30ShrdPrm.test_mode;

    PSENDX (T30, t30_config_req);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_T30                      |
|                            ROUTINE : psaT30_Deactivate            |
+-------------------------------------------------------------------+

  PURPOSE : deactivate T30.

*/

GLOBAL void psaT30_Deactivate (void)
{
  if (t30ShrdPrm.T30_is_activated EQ FALSE)
    return;
  else
    t30ShrdPrm.T30_is_activated = FALSE;

  TRACE_FUNCTION ("psaT30_Deactivate()");

  {
    PALLOC (t30_deactivate_req, T30_DEACTIVATE_REQ);

    PSENDX (T30, t30_deactivate_req);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_T30                      |
|                            ROUTINE : psaT30_Disconnect            |
+-------------------------------------------------------------------+

  PURPOSE : disconnect request.

*/

GLOBAL void psaT30_Disconnect (void)
{
  TRACE_FUNCTION ("psaT30_Disconnect()");

  {
    PALLOC (t30_sgn_req, T30_SGN_REQ);

    t30_sgn_req->sgn = SGN_DCN;

    PSENDX (T30, t30_sgn_req);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_T30                      |
|                            ROUTINE : psaT30_Modify                |
+-------------------------------------------------------------------+

  PURPOSE : modification of T30.

*/

GLOBAL void psaT30_Modify (void)
{
  TRACE_FUNCTION ("psaT30_Modify()");

  {
    PALLOC (t30_modify_req, T30_MODIFY_REQ);

    t30_modify_req->trans_rate = t30ShrdPrm.trans_rate;     
    t30_modify_req->half_rate  = t30ShrdPrm.half_rate;      

    PSENDX (T30, t30_modify_req);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_T30                      |
|                            ROUTINE : psaT30_Ppm                   |
+-------------------------------------------------------------------+

  PURPOSE : post-page message.

*/

GLOBAL SHORT psaT30_Ppm (void)
{
  TRACE_FUNCTION ("psaT30_Ppm()");

  {
    PALLOC (t30_sgn_req, T30_SGN_REQ);

    t30_sgn_req->sgn = t30ShrdPrm.sgn_snd;

    PSENDX (T30, t30_sgn_req);
  }

  return 0;
}

#endif /* DTI OR FF_FAX */

/*==== EOF ========================================================*/

