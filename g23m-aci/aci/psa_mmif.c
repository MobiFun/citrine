/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_MMIF
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
|             stack adapter for man machine interface.
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_MMIF_C
#define PSA_MMIF_C
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
#ifdef ACI
#include "cmh_mmi.h"
#endif /* ACI */
/*==== CONSTANTS ==================================================*/

#define ITM_WDT         (14)    /* item width in chars */
#define HDR_WDT         (10)    /* header width in chars */

/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMIF                |
|                                 ROUTINE : psaMMI_Init             |
+-------------------------------------------------------------------+

  PURPOSE : initialize the protocol stack adapter for MMI.

*/

GLOBAL void psaMMI_Init ( void )
{

/*
 *-------------------------------------------------------------------
 * set default parms
 *-------------------------------------------------------------------
 */  
#ifdef ACI
  ibt_params.ati_currIbt     = CPI_IBT_NotPresent;
  ibt_params.ati_currTch     = CPI_TCH_NotPresent;
  ibt_params.last_action     = SWITCH_AUDIO_OFF;
  cmhMMI_setAudioVolume (AUDIO_IN, AUDIO_OUT);
#endif
}


/*==== EOF ========================================================*/
 
