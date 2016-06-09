/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CC_CSF
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
|  Purpose :  This Modul defines the custom specific functions
|             for the component CC of the mobile station
|             
|             Use this module to integrate the entity
|             in your target system !
+----------------------------------------------------------------------------- 
*/ 

#ifndef CC_CSF_C
#define CC_CSF_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_CC
/*==== INCLUDES ===================================================*/

#include <string.h>
#include "typedefs.h"
#include "pcm.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "message.h"
#include "ccdapi.h"
#include "prim.h"
#include "cnf_cc.h"
#include "mon_cc.h"
#include "pei.h"
#include "tok.h"
#include "cc.h"

#include "m_rr_com.h"

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+------------------------------------------------------------------------------
|  Function     :  cc_csf_ms_cap
+------------------------------------------------------------------------------
|  Description  :  Reads the MS's capabilities from non volatile memory (flash).
|
|  Parameters   :  cc_data - Pointer to the CC data
|
|  Return       :  -
+------------------------------------------------------------------------------
*/
GLOBAL void cc_csf_ms_cap (void)
{
  GET_INSTANCE_DATA;
  UBYTE         version;
  T_mob_class_2 mob_class_2;
  EXTERN  void  rr_csf_get_classmark2 (T_mob_class_2 *mob_class_2);

  TRACE_FUNCTION ("cc_csf_ms_cap()");

  (void)pcm_ReadFile ((UBYTE *)EF_MSCAP_ID, SIZE_EF_MSCAP,
                      (UBYTE *)&cc_data->mscap, &version);

  TRACE_EVENT_P6 ("MSCAP: %02x %02x %02x %02x %02x %02x",
                  cc_data->mscap.chnMode,
                  cc_data->mscap.datCap1,
                  cc_data->mscap.datCap2,
                  cc_data->mscap.featLst1,
                  cc_data->mscap.featLst2,
                  cc_data->mscap.featLst3);

  rr_csf_get_classmark2 (&mob_class_2);

  memset (&cc_data->call_ctrl_cap, 0, sizeof (T_M_CC_call_ctrl_cap));
  /* G23 always supports DTMF regardless of PCM when voice is supported */
  cc_data->call_ctrl_cap.dtmf = cc_voice_capability ();
  /* Use CMSP bit of mobile class 2 as PCP bit in CC capabilities */
  cc_data->call_ctrl_cap.pcp = mob_class_2.cmsp;

#ifdef REL99
  /* Always one bearer in this release 99 version of the protocol */
  cc_data->call_ctrl_cap.max_nof_supp_bearers = M_CC_MAX_NOF_SUPP_BEARER_1;
  if (cc_voice_capability ())
  {
    cc_data->call_ctrl_cap.max_nof_supp_speech_bearers = M_CC_MAX_NOF_SUPP_SPEECH_BEARER_1;
  }
  else
  {
    cc_data->call_ctrl_cap.max_nof_supp_speech_bearers = M_CC_MAX_NOF_SUPP_SPEECH_BEARER_0;
  }
#endif

}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CSF                     |
| STATE   : code                ROUTINE : bc_prio_x                  |
+--------------------------------------------------------------------+

  PURPOSE :  To set the Bearer capabilities for triple vocoder.

  radio channel requirement
  content 1  means full rate support only mobile station
  content 2  means dual rate support mobile station, half rate preferred
  content 3  means dual rate support mobile station, full rate preferred

*/

/*
 * Setting of Radio Channel Requirement
 */
GLOBAL const UBYTE bc_prio_0 [4] =
{
    1,        /* support of FR,          full rate support only mobile station */
    3,        /* support of FR+HR,       dual rate, fullrate preferred         */
    1,        /* support of FR+EFR,      full rate support only mobile station */
    3,        /* support of FR+HR+EFR,   dual rate, fullrate preferred         */
};

#endif
