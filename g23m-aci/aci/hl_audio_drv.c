/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  AOC
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
|  Purpose :  This module defines the functions for the high level audio 
              driver interface.
+-----------------------------------------------------------------------------
*/


#ifndef HL_AUDIO_DRV_C
#define HL_AUDIO_DRV_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"
#include "aci_cmh.h"
#include "aci.h"
#include "psa.h"
#include "hl_audio_drv.h"

#ifdef VOCODER_FUNC_INTERFACE
#include "l4_tim.h"
#endif



/* ===============GLOBAL VARIABLES====================*/

LOCAL  T_HL_VOCODER_STATE currVocoderState;
#ifdef VOCODER_FUNC_INTERFACE
GLOBAL T_HL_VOCODER_ACTION currVocoderAction;
#endif

/* This flag helps enable the vocoder interface testing for
   specific tescases 
*/
#ifdef _SIMULATION_
BOOL vocoder_tst_flag; 
#endif


/* =============FUNCTION DEFINITIONS FOR L1===========*/
#ifndef _SIMULATION_
#ifdef VOCODER_FUNC_INTERFACE
void vocoder_mute_dl( BOOL mute );
void vocoder_mute_ul( BOOL mute );
void enable_tch_vocoder (BOOL vocoder_on);
#endif /* VOCODER_FUNC_INTERFACE */
#endif /* _SIMULATION_ */


/*================= FUNCTIONS ========================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : HL_AUDIO_DRV            |
|                                 ROUTINE : hl_audio_drv_init       |
+-------------------------------------------------------------------+

  PURPOSE : This function is called to initialise the variables in 
            the High Level Audio driver
*/
GLOBAL void hl_audio_drv_init (void)
{
  TRACE_FUNCTION("hl_audio_drv_init()");

  currVocoderState = HL_VOCODER_DISABLED;
#ifdef VOCODER_FUNC_INTERFACE
  currVocoderAction= VOCODER_IDLE;
#endif
#ifdef _SIMULATION_
  vocoder_tst_flag = FALSE;
#endif

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : hl_drv_set_vocoder_state|
+-------------------------------------------------------------------+

  PURPOSE : This function will enable/disable vocoder OR put the 
            vocoder state to PENDING_ENABLE/DISABLE depending on the 
            vocoder states. Depending on the vocoder state and if 
            requires timer will be started. Timer implementation is 
            done to synchronize timing delay needed(appr 35ms) to actually 
            enable/disable of vocoder in DSP.
*/
GLOBAL void hl_drv_set_vocoder_state(BOOL user_attach)
{
  TRACE_FUNCTION("hl_drv_set_vocoder_state");

  #ifdef VOCODER_FUNC_INTERFACE
  if ( (user_attach EQ TRUE AND currVocoderState EQ HL_VOCODER_ENABLED) OR
       (user_attach EQ FALSE AND currVocoderState EQ HL_VOCODER_DISABLED) )
    return; /* Do nothing, when the timeout occurs corresponding action will be taken */

  switch (currVocoderAction)  /* currVocoderAction is the state */
  {
    case VOCODER_IDLE:
      break;

    case VOCODER_BUSY:
    {
      if ( (user_attach EQ TRUE) AND (currVocoderState EQ HL_VOCODER_DISABLED) )
      {
        currVocoderAction = VOCODER_PENDING_ENABLE;
        return;
      }
      else if ( (user_attach EQ FALSE) AND (currVocoderState EQ HL_VOCODER_ENABLED) )
      {
        currVocoderAction = VOCODER_PENDING_DISABLE;
        return;
      }
    }
    break;

    case VOCODER_PENDING_ENABLE:
    {
      if (user_attach EQ TRUE)
        return; /* Do nothing */
      else  /* user_attach EQ FALSE  */
      {
        currVocoderAction = VOCODER_PENDING_DISABLE;
        return;
      }
    }
    break;

    case VOCODER_PENDING_DISABLE:
    {
      if (user_attach EQ FALSE)
        return; /* Do nothing */
      else  /* user_attach EQ TRUE */
      {
        currVocoderAction = VOCODER_PENDING_ENABLE;
        return;
      }
    }
    break;

    default: /* Assumed to be a never as all values caught */
      TRACE_ERROR ("Illegal value of currVocoderAction");
      break;

  }
  #endif /* VOCODER_FUNC_INTERFACE */

  #ifdef _SIMULATION_
  if(vocoder_tst_flag)
  #endif
  {

      #ifndef VOCODER_FUNC_INTERFACE
      {
      if(user_attach AND (currVocoderState EQ HL_VOCODER_DISABLED OR currVocoderState EQ HL_VOCODER_DISABLE_INITIATED))
        {
          PALLOC(mmi_tch_vocoder_cfg_req, MMI_TCH_VOCODER_CFG_REQ);
          mmi_tch_vocoder_cfg_req->vocoder_state = VOCODER_ENABLE;
          PSENDX(L1, mmi_tch_vocoder_cfg_req);
        currVocoderState = HL_VOCODER_ENABLE_INITIATED;
      }
      if(!user_attach AND (currVocoderState EQ HL_VOCODER_ENABLED OR currVocoderState EQ HL_VOCODER_ENABLE_INITIATED))
        {
          PALLOC(mmi_tch_vocoder_cfg_req, MMI_TCH_VOCODER_CFG_REQ);
          mmi_tch_vocoder_cfg_req->vocoder_state = VOCODER_DISABLE;
          PSENDX(L1, mmi_tch_vocoder_cfg_req);
          currVocoderState = HL_VOCODER_DISABLE_INITIATED;
        }
      }
      #else
      #ifndef _SIMULATION_
      TRACE_EVENT("Function interface for Vocoder");

      currVocoderAction = VOCODER_BUSY;
      TIMERSTART(VOCODER_VALUE, ACI_VOCODER);
      if (user_attach)
      {
        enable_tch_vocoder(TRUE);
        vocoder_mute_dl (FALSE);      /* un-mute speaker    */
        vocoder_mute_ul (FALSE);      /* un-mute microphone */
      }
      else
      {
        vocoder_mute_dl (TRUE);       /* mute speaker    */
        vocoder_mute_ul (TRUE);       /* mute microphone */
        enable_tch_vocoder (FALSE);   /* disable vocoder */
      }
    #endif /* _SIMULATION_ */
    #endif /* VOCODER_FUNC_INTERFACE */
  }

  return;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)        MODULE  : HL_AUDIO_DRV             |
|                                ROUTINE : hl_drv_get_vocoder_state |
+-------------------------------------------------------------------+

  PURPOSE : This function returns the state of the vocoder
*/

GLOBAL T_HL_VOCODER_STATE hl_drv_get_vocoder_state (void)
{
  return currVocoderState;
}

#ifndef VOCODER_FUNC_INTERFACE
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)        MODULE  : HL_AUDIO_DRV             |
|                                ROUTINE : hl_drv_vocoder_enabled   |
+-------------------------------------------------------------------+

  PURPOSE : Currently this function does nothing
*/

GLOBAL void hl_drv_vocoder_state_set (void)
{
  TRACE_FUNCTION("hl_drv_vocoder_enabled()");

  switch(currVocoderState)
  {
  case HL_VOCODER_ENABLE_INITIATED:
    currVocoderState = HL_VOCODER_ENABLED;
    break;

  case HL_VOCODER_DISABLE_INITIATED:
    currVocoderState = HL_VOCODER_DISABLED;
    break;

  default:
    break;

  }
#ifdef VOCODER_FUNC_INTERFACE
  currVocoderAction = VOCODER_IDLE;
#endif
}
#endif

#ifdef _SIMULATION_
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)        MODULE  : HL_AUDIO_DRV             |
|                                ROUTINE : hl_audio_drv_initForTest |
+-------------------------------------------------------------------+

  PURPOSE : Initialise global variables related to simulaed 
*/

GLOBAL void hl_audio_drv_initForTest(void)
{
  vocoder_tst_flag = TRUE;
}
#endif
