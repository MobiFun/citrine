/* 
+----------------------------------------------------------------------------- 
|  Project :  WAP
|  Modul   :  PSA_PPPF
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
|  Purpose :  Shared 
+----------------------------------------------------------------------------- 
*/ 
#if defined (FF_WAP) || defined (FF_PPP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E)

#ifndef PSA_PPPF_C
#define PSA_PPPF_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "dti.h"      /* functionality of the dti library */
#include "dti_conn_mng.h"

#include "aci_fd.h"
#include "psa.h"
#include "wap_aci.h"
#include "psa_ppp_w.h"

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : WAP			          MODULE  : PSA_PPP                 |
|                                 ROUTINE : psaPPP_Init             |
+-------------------------------------------------------------------+

  PURPOSE : Initialize the protocol stack adapter for PPP.

*/

GLOBAL void psaPPP_Init ( void )
{
  /* All shared parameter set to zero */	
  memset (&pppShrdPrm, 0, sizeof (T_PPP_SHRD_PRM));
  /* Dafault is PAP prot for PPP */
  pppShrdPrm.auth_prot = A_PAP;
  pppShrdPrm.state = PPP_UNDEFINED;
}

#endif /* FF_WAP or FF_PPP OR FF_GPF_TCPIP or SAT E */

/*==== EOF ========================================================*/

