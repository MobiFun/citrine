/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_MMIT
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
|  Purpose :  This module provides the test functions related to the 
|             protocol stack adapter for the man machine interface.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_MMIT_C
#define CMH_MMIT_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "pcm.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#ifdef UART
#include "dti.h"      /* functionality of the dti library */
#include "dti_conn_mng.h"
#endif

#include "psa.h"
#include "psa_sim.h"
#include "cmh.h"
#include "cmh_sim.h"

/* #include "m_fac.h" */
#include "aoc.h"

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSQ                  |
| STATE   : code                  ROUTINE : tAT_PlusCLAN             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CLAN AT command
            which is responsible for test supporetd language code in 
            ME.
*/

GLOBAL T_ACI_RETURN tAT_PlusCLAN ( T_ACI_CMD_SRC srcId,SHORT *lastIdx,
                                   T_ACI_LAN_SUP *lanlst )              
{
  T_ACI_RETURN    ret ;
  TRACE_FUNCTION ("tAT_PlusCLAN()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
   if(!cmh_IsVldCmdSrc (srcId)) 
   { 
     return( AT_FAIL );
   }
/*
 *-------------------------------------------------------------------
 *   read supported language from ME 
 *-------------------------------------------------------------------
 */   
   ret= getSupLangFromPCM(lanlst,lastIdx);
   return(ret);
}  

/*==== EOF ========================================================*/
