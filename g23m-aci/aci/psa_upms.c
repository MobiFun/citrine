/* 
+----------------------------------------------------------------------------- 
|  Project :  
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
|  Purpose :  This module defines the signalling functions of the
|             protocol stack adapter for the registration part of
|             the UPM entity.
+----------------------------------------------------------------------------- 
*/ 
#if defined GPRS

#ifndef PSA_UPMS_C
#define PSA_SNDS_C
#endif

/*==== INCLUDES ===================================================*/
#include "aci_all.h"
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "aci.h"

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "gaci.h"
#include "gaci_cmh.h"
#include "psa.h"
#include "psa_sm.h"

#include "cmh.h"
#include "cmh_sm.h"

#ifdef FF_WAP
#include "psa_tcpip.h"
#include "wap_aci.h"
#endif /* FF_WAP */
/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : PSA_UPM                 |
| STATE   : finished              ROUTINE : psa_upm_count_req       |
+-------------------------------------------------------------------+

  PURPOSE : Requests UPM count.

*/
GLOBAL void psa_upm_count_req ( UBYTE c_id, BOOL reset )
{
  PALLOC (upm_count_req, UPM_COUNT_REQ);

  TRACE_FUNCTION ("psa_upm_count_req()");

  upm_count_req -> nsapi   = c_id;

  if( reset )
  {
    upm_count_req -> reset = NAS_RESET_YES;
  }
  else
  {
    upm_count_req -> reset = NAS_RESET_NO;
  }

  PSENDX (UPM, upm_count_req);
}


#endif /* GPRS */
