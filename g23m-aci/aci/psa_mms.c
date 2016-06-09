/*   
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_MM
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
|             mobility management.
+----------------------------------------------------------------------------- 
*/ 

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#if !defined (DTI) || !defined(GPRS)

#ifndef PSA_MMS_C
#define PSA_MMS_C
#endif

#include "aci_all.h"
#ifdef DTI
#include "dti.h"
#include "dti_conn_mng.h"
#endif
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci.h"
#include "psa.h"
#include "psa_mm.h"

#include "aci_ext_pers.h"    
#include "aci_slock.h"  
#include "cmh.h"
#include "psa_sim.h"
#include "cmh_sim.h"

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/
LOCAL BOOL  frstFlg = TRUE;  /* flags first attempt */

/*==== FUNCTIONS ==================================================*/


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMS                 |
|                                 ROUTINE : psaMM_Registrate        |
+-------------------------------------------------------------------+

  PURPOSE : start network registration 

*/

GLOBAL SHORT psaMM_Registrate ( void )
{
  TRACE_FUNCTION ("psaMM_Registrate()");


/*
 *-------------------------------------------------------------------
 * check owner id
 *-------------------------------------------------------------------
 */  
  if(!psa_IsVldOwnId(mmShrdPrm.owner)) 
    
    return( -1 );

  /*
   * Set automatic registration mode in MM
   */
  psaMM_SetRegMode ( MODE_AUTO );

/*
 *-------------------------------------------------------------------
 * create and send primitive for network registration
 *-------------------------------------------------------------------
 */
  {
    PALLOC (mmr_reg_req, MMR_REG_REQ);
    /* mmr_reg_req->service_mode = SERVICE_MODE_FULL; */
    /* OVK Set proper service mode dependent on Sim lock state */
   #ifdef SIM_PERS
    if (AciSLockShrd.blocked EQ TRUE)
    {
       mmr_reg_req->service_mode = SERVICE_MODE_LIMITED;   
    }
    else
   #endif
    if (simShrdPrm.imei_blocked EQ TRUE)
    {
       mmr_reg_req->service_mode = SERVICE_MODE_LIMITED;   
    }
    else
    {
       mmr_reg_req->service_mode = SERVICE_MODE_FULL;
    }
   


    PSENDX (MM, mmr_reg_req);
  }

  frstFlg = FALSE;  
  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMS                 |
|                                 ROUTINE : psaMM_DeRegistrate      |
+-------------------------------------------------------------------+

  PURPOSE : start network de-registration 

*/

GLOBAL SHORT psaMM_DeRegistrate ( void )
{

  TRACE_FUNCTION ("psaMM_DeRegistrate()");

  /*
   *-------------------------------------------------------------------
   * create and send primitive for deregistration
   *-------------------------------------------------------------------
   */  
  {
    PALLOC (mmr_nreg_req, MMR_NREG_REQ); /* T_MMR_NREG_REQ */
      
    mmr_nreg_req->detach_cause = mmShrdPrm.nrgCs;

    PSENDX (MM, mmr_nreg_req);
  }

  frstFlg = TRUE;  
  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMS                 |
|                                 ROUTINE : psaMM_NetSrch           |
+-------------------------------------------------------------------+

  PURPOSE : request network search

*/

GLOBAL SHORT psaMM_NetSrch ( void )
{
  TRACE_FUNCTION ("psaMM_NetSrch()");

/*
 *-------------------------------------------------------------------
 * check owner id
 *-------------------------------------------------------------------
 */  
  if(!psa_IsVldOwnId(mmShrdPrm.owner)) 
    
    return( -1 );

  /*
   * set manual registration mode in MM 
   */
  psaMM_SetRegMode ( MODE_MAN );

/*
 *-------------------------------------------------------------------
 * create and send primitive for network search
 *-------------------------------------------------------------------
 */
  if( frstFlg EQ TRUE
#ifdef FF_DUAL_SIM
    AND CFUNfun EQ CFUN_FUN_Full
#endif /*FF_DUAL_SIM*/
    )
  {
    PALLOC (mmr_reg_req, MMR_REG_REQ);
    mmr_reg_req->service_mode = SERVICE_MODE_FULL;
    PSENDX (MM, mmr_reg_req);
    frstFlg = FALSE;
  }
  else
  {
    PALLOC (mmr_net_req, MMR_NET_REQ);
    PSENDX (MM, mmr_net_req);
    frstFlg = FALSE;
  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMS                 |
|                                 ROUTINE : psaMM_NetSel            |
+-------------------------------------------------------------------+

  PURPOSE : select a network

*/

GLOBAL SHORT psaMM_NetSel ( void )
{
  TRACE_FUNCTION ("psaMM_NetSel()");

/*
 *-------------------------------------------------------------------
 * check owner id
 *-------------------------------------------------------------------
 */  
  if(!psa_IsVldOwnId(mmShrdPrm.owner)) 
    
    return( -1 );

  /*
   * set manual registration mode in MM
   */
  psaMM_SetRegMode ( MODE_MAN );

/*
 *-------------------------------------------------------------------
 * create and send primitive for network select
 *-------------------------------------------------------------------
 */  
  {
    PALLOC (mmr_plmn_res, MMR_PLMN_RES);

    /* fill in primitive parameter: selected PLMN */
    mmr_plmn_res -> plmn = mmShrdPrm.slctPLMN;

    PSENDX (MM, mmr_plmn_res);
  }

  frstFlg = FALSE;

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMS                 |
|                                 ROUTINE : psaMM_SetRegMode        |
+-------------------------------------------------------------------+

  PURPOSE : set the registration mode 

*/

GLOBAL SHORT psaMM_SetRegMode ( UBYTE mode )
{
  TRACE_FUNCTION ("psaMM_SetRegMode()");

  /*
   *-------------------------------------------------------------------
   * create and send primitive for registration mode
   *-------------------------------------------------------------------
   */
  {
    PALLOC (mmr_plmn_mode_req, MMR_PLMN_MODE_REQ); /* T_MMR_PLMN_MODE_REQ */
    mmr_plmn_mode_req -> mode = mode;
    PSENDX (MM, mmr_plmn_mode_req);
  }

  return 0;
}
#endif  /* DTI */

/*==== EOF ========================================================*/
 
