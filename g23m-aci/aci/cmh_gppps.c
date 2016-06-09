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
|  Purpose :  This module provides the set functions related to the
|             protocol stack adapter for GPRS Point-to-Point Protocol ( PPP ).
+----------------------------------------------------------------------------- 
*/ 

#if defined (GPRS) && defined (DTI)

#ifndef CMH_PPPS_C
#define CMH_PPPS_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"

#include "dti_conn_mng.h"

#include "gaci.h"
#include "psa.h"
#include "psa_gppp.h"
#include "cmh.h"
#include "cmh_gppp.h"

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PPPS                |
| STATE   : finished              ROUTINE : sAT_PercentCGPPP        |
+-------------------------------------------------------------------+

  PURPOSE : set authentification protocol.

*/

GLOBAL T_ACI_RETURN sAT_PercentCGPPP( T_ACI_CMD_SRC srcId,
                                      T_ACI_PPP_PROT protocol )
{

/*
 *-------------------------------------------------------------------
 * process protocol parameter
 *-------------------------------------------------------------------
 */
  
  switch( protocol )
  {
    case( A_NO_AUTH   ):
      gpppShrdPrm.ppp_authentication_protocol = PPP_AP_NO; 
      break;

    case( A_PAP       ):
      gpppShrdPrm.ppp_authentication_protocol = PPP_AP_PAP; 
      break;

    case( A_CHAP      ):
      gpppShrdPrm.ppp_authentication_protocol = PPP_AP_CHAP; 
      break;

    case( A_AUTO_AUTH ):
      gpppShrdPrm.ppp_authentication_protocol = PPP_AP_AUTO; 
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

  return( AT_CMPL );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PPPS                |
| STATE   : finished              ROUTINE : qAT_PercentCGPPP        |
+-------------------------------------------------------------------+

  PURPOSE : query authentification protocol.

*/

GLOBAL T_ACI_RETURN qAT_PercentCGPPP( T_ACI_CMD_SRC srcId,
                                      T_ACI_PPP_PROT *protocol )
{

  TRACE_FUNCTION ("qAT_PercentCGPPP()");

  *protocol = (T_ACI_PPP_PROT)gpppShrdPrm.ppp_authentication_protocol;

  return (AT_CMPL);
}

#endif  /* GPRS */
/*==== EOF ========================================================*/
