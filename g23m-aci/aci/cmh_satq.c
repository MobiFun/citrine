/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_SATQ
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
|  Purpose :  This module provides the query functions related to the 
|             SIM application toolkit module.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_SATQ_C
#define CMH_SATQ_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "psa.h"
#include "psa_cc.h"
#include "psa_sat.h"
#include "psa_sim.h"
#include "cmh.h"
#include "cmh_sat.h"

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SATS                 |
| STATE   : code                  ROUTINE : qAT_PercentSATC          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %SATC? AT command
            which is responsible to query the SIM toolkit profile. 
*/

GLOBAL T_ACI_RETURN qAT_PercentSATC ( T_ACI_CMD_SRC    srcId,
                                      SHORT *len, UBYTE *satCnfg )
{
  
  TRACE_FUNCTION ("qAT_PercentSATC()");
/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
    
    return( AT_FAIL );

/*
 *-------------------------------------------------------------------
 * update SIM toolkit profile
 *-------------------------------------------------------------------
 */  
  if( !len OR !satCnfg  ) return ( AT_FAIL );

  memcpy( satCnfg, simShrdPrm.setPrm[srcId].STKprof, MAX_STK_PRF );

  *len = MAX_STK_PRF;

  return (AT_CMPL);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SATS                 |
| STATE   : code                  ROUTINE : qAT_PercentSIMEF         |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %SIMEF? AT command
            which queries the EF update reporting mode. 
*/

GLOBAL T_ACI_RETURN qAT_PercentSIMEF (T_ACI_CMD_SRC srcId,
                                      T_ACI_SIMEF_MODE *mode)
{
  TRACE_FUNCTION ("qAT_PercentSIMEF()");

  /*
  *   Check command source
  */
  if(!cmh_IsVldCmdSrc (srcId)) 
    return(AT_FAIL);

  *mode=simShrdPrm.SIMEFMode[srcId];

  return (AT_CMPL);
}



/*==== EOF ========================================================*/
