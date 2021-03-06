/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_RAR
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
|             rate adaptation.
+----------------------------------------------------------------------------- 
*/ 

#ifdef DTI

#ifndef CMH_RAR_C
#define CMH_RAR_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#ifdef FAX_AND_DATA
  #include "aci_fd.h"
#endif

#include "aci.h"
#include "psa.h"
#include "aci_io.h"
#include "psa_ra.h"
#include "psa_cc.h"
#include "cmh.h"
#include "cmh_ra.h"
#include "cmh_cc.h"

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_RAR                      |
|                            ROUTINE : cmhRA_Activated              |
+-------------------------------------------------------------------+

  PURPOSE : RA entity activated, connection established

*/

GLOBAL void cmhRA_Activated ( SHORT cId )
{
  TRACE_FUNCTION ("cmhRA_Activated()");

  /* inform call control about successful operation */
  cmhCC_RA_Activated(cId);
  
  raEntStat.curCmd = AT_CMD_NONE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_RAR                      |
|                            ROUTINE : cmhRA_Deactivated            |
+-------------------------------------------------------------------+

  PURPOSE : RA entity deactivated, connection disconnected

*/

GLOBAL void cmhRA_Deactivated ( SHORT cId )
{
  TRACE_FUNCTION ("cmhRA_Deactivated()");

  /* inform call control about disconnection */
  cmhCC_RA_Deactivated(cId);
  
  /* reset parameters only if data connection should be disconnected */
  if( ccShrdPrm.datStat NEQ DS_TCH_MDF )
  {
    raShrdPrm.cId    = NO_ENTRY;
    raEntStat.curCmd = AT_CMD_NONE;
  }
}

#endif /* DTI */

/*==== EOF ========================================================*/
 
