/* 
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_F
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
|  Purpose :  This module defines global functions of the protocol
|             stack adapters.
+-----------------------------------------------------------------------------
*/

#ifndef PSA_F_C
#define PSA_F_C
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
#include "psa.h"
#include "aci.h"
#include "l4_tim.h"

#include "psa_cc.h"
#if defined FF_EOTD
#include "psa_lc.h"
#endif

#ifdef UART
#include "dti_conn_mng.h"
#endif

#ifdef GPRS
#include "gaci_cmh.h"
#include "psa_gmm.h"
#include "psa_sm.h"
#include "psa_gppp.h"
#include "ffs/ffs.h"
#include "ffs_coat.h"
#endif /* GPRS */

#include "aci_lst.h"

#ifdef UART
#include "dti.h"      /* functionality of the dti library */
#include "psa_uart.h"
#include "cmh_uart.h"
#endif

#ifdef VOCODER_FUNC_INTERFACE
#include "hl_audio_drv.h"
#endif

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH                     |
|                                 ROUTINE : psa_Init                |
+-------------------------------------------------------------------+

  PURPOSE : initialize the protocol stack adapters.

*/

GLOBAL void psa_Init ( void )
{

/*
 *-------------------------------------------------------------------
 * set default parms for protocol stack adapters
 *-------------------------------------------------------------------
 */

 /* --- psa init functions --- */
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH                     |
|                                 ROUTINE : psa_IsVldOwnid          |
+-------------------------------------------------------------------+

  PURPOSE : check for a valid owner id.

*/

GLOBAL BOOL psa_IsVldOwnId ( T_OWN ownId )
{
  if( ownId > ((T_OWN)CMD_SRC_NONE) AND ownId < OWN_SRC_MAX )

    return TRUE;

  return( FALSE );
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  ACI_CMD             |
| STATE  : code                         ROUTINE: psa_timeout         |
+--------------------------------------------------------------------+

  PURPOSE : A timeout has occured for a timer. The function returns
            TRUE, if it is a timer which is handled by PSA, else FALSE
            is returned to indicate that the timer has not been
            processed.

*/

GLOBAL UBYTE psa_timeout (USHORT index)
{
  TRACE_FUNCTION ("psa_timeout()");

  switch(index)
  {
  case(ACI_TDTMF):
    /* timeout dtmf timer */
    psaCC_DTMFTimeout();
    return(TRUE);

#ifdef VOCODER_FUNC_INTERFACE
  case(ACI_VOCODER):
  {
    T_HL_VOCODER_ACTION tmpcurrVocoderAction = currVocoderAction;
    currVocoderAction = VOCODER_IDLE;
    /*
     * If the vocoder state is PENDING_ENABLE/DISABLE then  
     * call the function hl_drv_set_vocoder_state() to Enable/Disable 
     * vocoder depending on the state.
     */
    switch(tmpcurrVocoderAction)
    {
      case VOCODER_PENDING_DISABLE:
        hl_drv_set_vocoder_state(FALSE); /* Disable vocoder */
        break;

      case VOCODER_PENDING_ENABLE:
        hl_drv_set_vocoder_state(TRUE);  /* Enable vocoder */
        break;

      default:
        TRACE_ERROR ("Unexpected timeout in state");
        break;
    }

    return (TRUE); /* Timeout event handled */
  }
#endif

#ifdef UART
  case(ACI_IPR):
    /* timeout AT+IPR: change baud rate */
    timeout_PlusIPR();
    return(TRUE);
#endif
    
#if defined FF_EOTD
  case(ACI_LC_1):
  case(ACI_LC_2):
  case(ACI_LC_3):
  case(ACI_LC_4):
  case(ACI_LC_5):
    psaLC_posDataReq(index);
    return(TRUE);
#endif /* FF_EOTD */
  }

  /* the timeout is not for PSA */
  return FALSE;
}

#if defined (GPRS) AND defined (DTI)
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH                     |
|                                 ROUTINE : psa_GPRSInit            |
+-------------------------------------------------------------------+

  PURPOSE : initialize the protocol stack adapters.

*/
#define FACTOR_T3314   1000
#define FACTOR_T3312  60000

GLOBAL void psa_GPRSInit ( void )
{
  T_FFS_GPRS_ACI ffsBuffer;

  T_FFS_SIZE size = FFS_fread("/GPRS/baspar_gprs", &ffsBuffer, sizeof(T_FFS_GPRS_ACI));
  
  if ( size NEQ sizeof(T_FFS_GPRS_ACI) )
  {
#   ifndef _SIMULATION_
    if ( size < 0 )
    {
      TRACE_EVENT_P1("FFS can not read \"/GPRS/baspar_gprs\" (%d)", size);
    }
    else
    {
      TRACE_EVENT_P2("FFS contains old file of \"/GPRS/baspar_gprs\": %dBytes long, but %dBytes expected",
                      size, sizeof(T_FFS_GPRS_ACI));
    }
#   endif /* NOT DEFINED _SIMULATION_ */

    ffsBuffer.max_cid               = 2;     /* not supported at this moment */
    ffsBuffer.auto_attach           = (UBYTE) CGAATT_ATTACH_MODE_MANUAL;/* 1 */
    ffsBuffer.auto_detach           = (UBYTE) CGAATT_DETACH_MODE_OFF;   /* 1 */
    ffsBuffer.default_mobile_class  = (UBYTE) GMMREG_CLASS_BG;          /* 4 */
    ffsBuffer.accm                  = 0;                               /*  3 */
    ffsBuffer.restart_timer         = PPP_RT_DEFAULT;                  /*  3 */
    ffsBuffer.max_configure         = PPP_MC_DEFAULT;                  /* 10 */
    ffsBuffer.max_terminate         = PPP_MT_DEFAULT;                  /*  2 */
    ffsBuffer.max_failure           = PPP_MF_DEFAULT;                  /*  5 */
  }

  psaGPPP_Init( ffsBuffer.accm, ffsBuffer.restart_timer,
                ffsBuffer.max_configure, ffsBuffer.max_terminate, ffsBuffer.max_failure );
  psaGMM_Init( ffsBuffer.auto_attach,ffsBuffer.auto_detach, ffsBuffer.default_mobile_class   );
  psaSM_Init();
}
#endif /* GPRS */

/*==== EOF ========================================================*/
