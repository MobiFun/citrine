/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  ACI_CSF
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
|  Purpose :  This Modul defines the custom specific functions for the
|             AT Command Interpreter
+----------------------------------------------------------------------------- 
*/ 

#ifndef ACI_CSF_C
#define ACI_CSF_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

#include "aci_cmh.h"

#ifdef FF_ATI
#include "ati_cmd.h"
#endif

#include "aci_cmd.h"
#include "aci.h"

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CSF            |
| STATE   : code                        ROUTINE : aci_init_timer     |
+--------------------------------------------------------------------+

  PURPOSE : Initialize the ACI timer.

*/

GLOBAL void aci_init_timer (void)
{
  TRACE_FUNCTION ("aci_init_timer()");

#if !defined (NEW_FRAME)

#ifdef FF_ATI
  t_ring_handle = vsi_t_open (VSI_CALLER TRING );
#endif
  t_ect_handle  = vsi_t_open (VSI_CALLER TMPTY );
  t_mpty_handle = vsi_t_open (VSI_CALLER TMPTY );
  t_dtmf_handle = vsi_t_open (VSI_CALLER TDTMF );

#ifdef FAX_AND_DATA
  t_fit_handle  = vsi_t_open (VSI_CALLER TFIT );
#endif

#endif
  /* AoC timers are created if AoC is really supported */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CSF            |
| STATE   : code                        ROUTINE : aci_exit_timer     |
+--------------------------------------------------------------------+

  PURPOSE : Closes all timer.

*/

GLOBAL void aci_exit_timer (void)
{
  TRACE_FUNCTION ("aci_exit_timer()");

#if !defined (NEW_FRAME)

#ifdef FF_ATI
  vsi_t_close (VSI_CALLER t_ring_handle);
  t_ring_handle = VSI_ERROR;
#endif

#ifdef FAX_AND_DATA
  vsi_t_close (VSI_CALLER t_fit_handle);
  t_fit_handle = VSI_ERROR;
#endif

#endif
}

#endif

