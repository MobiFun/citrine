/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_L2RF
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
|  Purpose :  This module defines the functions for the protocol
|             stack adapter for Layer 2 Relay.
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_L2RF_C
#define PSA_L2RF_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/

#include "dti.h"      /* functionality of the dti library */
#include "dti_conn_mng.h"

#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_fd.h"
#include "aci.h"
#include "psa.h"
#include "cmh.h"
#include "cmh_ra.h"
#include "psa_l2r.h"
#include "cmh_l2r.h"
#include "psa_util.h"

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_L2R                 |
|                                 ROUTINE : psaL2R_Init             |
+-------------------------------------------------------------------+

  PURPOSE : initialize the protocol stack adapter for L2R.

*/

GLOBAL void psaL2R_Init ( void )
{
  memset (&l2rShrdPrm, 0, sizeof (T_L2R_SHRD_PRM));
}

/*==== EOF ========================================================*/

