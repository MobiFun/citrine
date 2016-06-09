/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_MMIR
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
|             the subscriber identity module.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_MMIR_C
#define CMH_MMIR_C
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
#include "aci.h"
#include "psa_mmi.h"
/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_MMIR                     |
|                            ROUTINE : cmhMMI_btIndication          |
+-------------------------------------------------------------------+

  PURPOSE : battery level indication

*/

GLOBAL void cmhMMI_btIndication ( void )
{

  TRACE_FUNCTION ("cmhMMI_btIndication()");

  /* process event */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  rAT_PercentBC( mmiShrdPrm.btLev );
#endif
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_MMIR                     |
|                            ROUTINE : cmhMMI_rxIndication          |
+-------------------------------------------------------------------+

  PURPOSE : rx level indication

*/

GLOBAL void cmhMMI_rxIndication ( void )
{
  TRACE_FUNCTION ("cmhMMI_rxIndication()");

  /* process event */
  /* actually only needed for SMI */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  rAT_PercentSQ( mmiShrdPrm.rxLev );
#endif
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_MMIR                     |
|                            ROUTINE : cmhMMI_keyIndication         |
+-------------------------------------------------------------------+

  PURPOSE : keypad indication

*/

GLOBAL void cmhMMI_keyIndication ( void )
{

  TRACE_FUNCTION ("cmhMMI_keyIndication()");

  /* process event */
#if defined SMI OR defined MFW
  rAT_PercentDRV( DRV_DEV_Keypad, DRV_FCT_KeypadInd,
                  mmiShrdPrm.keyCd, mmiShrdPrm.keySt);
#endif
}

/*==== EOF ========================================================*/
 
