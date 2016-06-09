/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_MMIS
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
|             protocol stack adapter for the man machine interface.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_MMIS_C
#define CMH_MMIS_C
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
#include "psa_mmi.h"
#include "cmh_mmi.h"
#include "pcm.h"

#ifdef UART
#include "dti.h"      /* functionality of the dti library */
#include "dti_conn_mng.h"
#endif

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */
#include "psa.h"
#include "psa_sim.h"
#include "cmh.h"
#include "cmh_sim.h"
#include "cmh_cc.h"
#include "aoc.h"

#include "aci_prs.h"

#ifdef ACI
#include "gdi.h"
#include "audio.h"
#include "aci_lst.h"
#include "ati_int.h"
#endif

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*=================================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMIS                 |
| STATE   : code                  ROUTINE : sAT_PercentDRV           |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %DRV AT command
            which is responsible to set the parameters for relatet 
            operations regarding audio and keyboard.

            <device>:      defines the addressed device.
            <function>:    defines the function to access 
            <val1>,<val2>: additional parameters
*/
#if 0
GLOBAL T_ACI_RETURN sAT_PercentDRV( T_ACI_CMD_SRC srcId,
                                    T_ACI_DRV_DEV device, 
                                    T_ACI_DRV_FCT function,
                                    UBYTE         val1,
                                    UBYTE         val2,
                                    void*         ptr1 )
{

  TRACE_FUNCTION ("sAT_PercentDRV()");
  
  /* process the <device> and <function> parameter */  
  if(device EQ DRV_DEV_Display )
  { 
    if(function EQ DRV_FCT_DisplayReq )
    {
       mmiShrdPrm.dspRq = ptr1;
       psaMMI_Display();
       return( AT_CMPL );
    }
  }
  return(AT_FAIL);
}
#endif

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_MMIS                     |
| STATE   : code             ROUTINE : sAT_PlusCLAN                 |
+-------------------------------------------------------------------+

  PURPOSE : PURPOSE : This is the functional counterpart to the +CLAN
            AT command which is responsible for test supporetd 
            language  code in ME.

            <lngCode>: string language Code
*/
GLOBAL T_ACI_RETURN sAT_PlusCLAN ( T_ACI_CMD_SRC  srcId, 
                                   T_ACI_LAN_SUP  *lngCode)
                                   
{
/* Implements Measure#32: Row 976 */
  EF_CLNG       lng; 
  T_SIM_CMD_PRM *pSIMCmdPrm; /* points to SIM command parameters  */
  T_PHB_CMD_PRM *pPHBCmdPrm; /* points to PHB command parameter */
  T_ACI_LAN_SUP LngPCMsupLst[MAX_LAN]; 
  SHORT         lastIdx;
  BOOL          Suplng = FALSE;
  
  TRACE_FUNCTION ("sAT_PlusCLAN ()");

  /*
   *-----------------------------------------------------------------
   * check command source
   *-----------------------------------------------------------------
   */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    return( AT_FAIL );
  }

  pSIMCmdPrm = &cmhPrm[srcId].simCmdPrm; 
  pPHBCmdPrm = &cmhPrm[srcId].phbCmdPrm;

/*
 *-------------------------------------------------------------------
 *  if Automatic language is set then read language from the sim   
 *-------------------------------------------------------------------
 */  
  if ( lngCode->lng EQ CLAN_LNG_AUT )
  {
    /*
     *-------------------------------------------------------------
     * check entity status
     *-------------------------------------------------------------
     */  
     if( simEntStat.curCmd NEQ AT_CMD_NONE )
      return( AT_BUSY );

     pSIMCmdPrm->CLANact=CLAN_ACT_Write;
    /*
     *-------------------------------------------------------------
     * request EF ELP from SIM
     *-------------------------------------------------------------
     */  
    /* Implements Measure 150 and 159 */
    return cmhSIM_ReqLanguage_LP_or_ELP ( srcId, SIM_ELP );
  }     
  
  /*
   *-------------------------------------------------------------
   * check if the language is supported in PCM
   *-------------------------------------------------------------
   */      
   getSupLangFromPCM(&LngPCMsupLst[0], &lastIdx);
   Suplng=checkSuppLang(&LngPCMsupLst[0],lastIdx, lngCode); 
   
   if (Suplng)
   {
     /*
      *-------------------------------------------------------------
      * copy the set code and write it in PCM
      *-------------------------------------------------------------
      */    
      
      memcpy ( lng.data, lngCode->str,CLAN_CODE_LEN);

/* Implements Measure#32: Row 976 */
      if ( pcm_WriteFile (( UBYTE* )ef_clng_id,SIZE_EF_CLNG,( UBYTE*) &lng) EQ PCM_OK)
      {
        
       /*
        *-------------------------------------------------------------
        * check if the result code after setting is enabled
        *-------------------------------------------------------------
        */      
        if (pPHBCmdPrm->CLAEmode EQ CLAE_MOD_Enable)  
        {R_AT( RAT_CLAE, srcId ) (lngCode);}
        return( AT_CMPL );
      }
      else 
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_MemFail );
        return( AT_FAIL );
      }

   }
   else 
   {
     ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown );
     return( AT_FAIL );
   }

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMIS                 |
| STATE   : code                  ROUTINE : sAT_PlusCLAE             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CLAE AT command
            which is responsible to enable or disable the presentation
            of Language Event.

*/

GLOBAL T_ACI_RETURN sAT_PlusCLAE ( T_ACI_CMD_SRC srcId,
                                   T_ACI_CLAE_MOD mode )
{
  T_PHB_CMD_PRM * pPHBCmdPrm; /* points to PHB command parameter */

  TRACE_FUNCTION ("sAT_PlusCLAE()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  pPHBCmdPrm = &cmhPrm[srcId].phbCmdPrm;

/*
 *-------------------------------------------------------------------
 * process the mode parameter
 *-------------------------------------------------------------------
 */
  switch( mode )
  {
    case( CLAE_MOD_Disable ):
    case( CLAE_MOD_Enable  ):
      pPHBCmdPrm -> CLAEmode = mode;
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

  return( AT_CMPL );
}

#ifdef TI_PS_FF_AT_P_CMD_CUST
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMIS                |
|                                 ROUTINE : sAT_PercentCUST  |
+-------------------------------------------------------------------+

PURPOSE :    This function will set the customisation mode for the ACI and
                    other required entities
*/ 

GLOBAL T_ACI_RETURN sAT_PercentCUST( T_ACI_CMD_SRC srcId,
                                          T_CUST_MOD customisation_mode)
{
    /*
    *-------------------------------------------------------------------
    * check command source
    *-------------------------------------------------------------------
    */  
    if(!cmh_IsVldCmdSrc (srcId)) 
    { 
        return( AT_FAIL );
    }

    /*
    ** This command is only valid before the ME is put into Fully Functional Mode
    */
    if (psaSIM_hasCustModeBeenSet() != FALSE)
    {
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Unknown);
        return( AT_FAIL );
    }

    /*
    ** Switch on the Customisation mode
    */
    switch (customisation_mode)
    {
        case CUST_NORMAL_BEHAVIOUR: /* Default Value, No Customisation */
        case CUST_MODE_BEHAVIOUR_1: /* Cust1 */

            if (simShrdPrm.setPrm[srcId].cust_mode != customisation_mode)
            {
                /*
                ** Set the customisation mode, and notify ALL other affected entities
                */
                simShrdPrm.setPrm[srcId].cust_mode = customisation_mode;
            }
            break;

        default:
            /*
            ** This is an error
            */
            ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
            return (AT_FAIL);
    }

    return (AT_CMPL);

}
#endif /* TI_PS_FF_AT_P_CMD_CUST */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMIS                |
|                                 ROUTINE : sAT_PercentSATCC  |
+-------------------------------------------------------------------+

PURPOSE :    This function will set the call and short message control by SIM functionality
                    to Enabled or Disabled according to the value in sat_cc_mode.
*/ 

GLOBAL T_ACI_RETURN sAT_PercentSATCC( T_ACI_CMD_SRC srcId,
                                          T_SAT_CC_MOD sat_cc_mode)
{
    /*
    *-------------------------------------------------------------------
    * check command source
    *-------------------------------------------------------------------
    */  
    if(!cmh_IsVldCmdSrc (srcId)) 
    { 
        return( AT_FAIL );
    }

    /*
    ** Enable or Disbale the Call or Short Message Control by SIM, according to the passed parameter
    */
    switch (sat_cc_mode)
    {
        case SATCC_CONTROL_BY_SIM_INACTIVE:
        case SATCC_CONTROL_BY_SIM_ACTIVE:
            simShrdPrm.setPrm[srcId].sat_cc_mode = sat_cc_mode;
            break;

        default:
            ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
            return( AT_FAIL );
    }
    
    return (AT_CMPL);

}


/* 
 * Implemetation of IBT handling by ME for ACI only stacks
 */

#ifdef ACI

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMIS                |
|                                 ROUTINE : cmhMMI_setAudioVolume   |
+-------------------------------------------------------------------+

PURPOSE : This function sets the volume for the audio input and
          audio output path.

          <audioIn>:  input volume
          <audioOut>: output volume
*/ 

GLOBAL void cmhMMI_setAudioVolume (BYTE audioIn, BYTE audioOut)
{
  TRACE_FUNCTION("cmhMMI_setAudioVolume");

  audio_SetAmplf (AUDIO_MICROPHONE, audioIn);
  audio_SetAmplf (AUDIO_SPEAKER,    audioOut);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMIS                |
|                                 ROUTINE : cmhMMI_handleAudioTone  |
+-------------------------------------------------------------------+

PURPOSE : This function is to switch on/off a specified tone.

          <cmdId>:    command identifier
          <response>: response type
          <msgType>:  CPI message type
*/ 

GLOBAL void cmhMMI_handleAudioTone (T_ACI_AT_CMD  cmdId,
                                    UBYTE         response,
                                    T_ACI_CPI_MSG msgType )
{
  T_ACI_IBT_ACT new_audio_action = SWITCH_AUDIO_NONE;

  TRACE_FUNCTION("cmhMMI_handleAudioTone");

  /* only for test purposes */
  /*ibt_params.ati_currTch = CPI_TCH_False;*/

  switch (response)
  {
  case RAT_CCWA:
    new_audio_action = SWITCH_AUDIO_CALLWAIT;
    break;

  case RAT_BUSY:
    new_audio_action = SWITCH_AUDIO_BUSY;
    break;

  case RAT_OK:
    if ((cmdId EQ AT_CMD_H) OR (cmdId EQ AT_CMD_D) OR (cmdId EQ AT_CMD_CTFR))
    {
      new_audio_action = SWITCH_AUDIO_OFF;
    }
    break;

  case RAT_CONNECT:
    if ((ibt_params.ati_currIbt NEQ CPI_IBT_True) OR 
        (ibt_params.ati_currTch NEQ CPI_TCH_True))
    {
      new_audio_action = SWITCH_AUDIO_OFF;
    }
    break;

  case RAT_NO_ANSWER:
  case RAT_NO_CARRIER:
    if (cmdId EQ AT_CMD_D)
    {
      new_audio_action = SWITCH_AUDIO_OFF;
    }
    break;

  case RAT_CPI:
    /* The network is not allowed to stop the BUSY tone.
       This can only be done by ATH */
    if (ibt_params.last_action NEQ SWITCH_AUDIO_BUSY)
    {
      if ((ibt_params.ati_currIbt EQ CPI_IBT_True) AND
          (ibt_params.ati_currTch EQ CPI_TCH_True))
      {
        new_audio_action = SWITCH_AUDIO_OFF;
      }
      else
      {
        if ((msgType EQ CPI_MSG_Disc) OR (msgType EQ CPI_MSG_Conn))
        {
          new_audio_action = SWITCH_AUDIO_OFF;
        }
        else if (msgType EQ CPI_MSG_Alert)
        {
          new_audio_action = SWITCH_AUDIO_ALERT;
        }
      }
    } /* if (ibt_params.last_action NEQ SWITCH_AUDIO_BUSY) */
    break;

  case RAT_CME:
    if ((cmdId EQ AT_CMD_D) OR (cmdId EQ AT_CMD_CHLD))
    {
      if (curAbrtCmd NEQ AT_CMD_ABRT)
      {
        new_audio_action = SWITCH_AUDIO_OFF;
      }
    }
    break;

  default:
    break;
  }

  if (new_audio_action EQ ibt_params.last_action)
  {
    if(!((rdlPrm.rdlMod EQ AUTOM_REPEAT_ON) AND (rdlPrm.rdlcId NEQ -1)))
    {
    /* it is not necessary to perform the same action twice */
      return;
    }
  }

  switch (new_audio_action)
  {
  case SWITCH_AUDIO_OFF:
    if (ibt_params.last_action EQ SWITCH_AUDIO_ALERT)
    {
      audio_StopSoundbyID (AUDIO_SPEAKER, TONES_RINGING_TONE);
    }
    else if (ibt_params.last_action EQ SWITCH_AUDIO_BUSY)
    {
      audio_StopSoundbyID (AUDIO_SPEAKER, TONES_BUSY);
    }
    else if (ibt_params.last_action EQ SWITCH_AUDIO_CALLWAIT)
    {
      audio_StopSoundbyID (AUDIO_SPEAKER, TONES_CW);
    }
    
    TRACE_EVENT("AUDIO FUNCTION CALLED: OFF");
    ibt_params.last_action = SWITCH_AUDIO_OFF;
    break;

  case SWITCH_AUDIO_ALERT:
    audio_PlaySoundID (AUDIO_SPEAKER,
                       TONES_RINGING_TONE,
                       RINGER,
                       AUDIO_PLAY_INFINITE);
    TRACE_EVENT("AUDIO FUNCTION CALLED: ALERT");
    ibt_params.last_action = SWITCH_AUDIO_ALERT;
    break;

  case SWITCH_AUDIO_BUSY:
    audio_PlaySoundID (AUDIO_SPEAKER,
                       TONES_BUSY,
                       RINGER,
                       AUDIO_PLAY_ONCE);
    TRACE_EVENT("AUDIO FUNCTION CALLED: BUSY");
    ibt_params.last_action = SWITCH_AUDIO_BUSY;
    break;

  case SWITCH_AUDIO_CALLWAIT:
    audio_PlaySoundID (AUDIO_SPEAKER,
                       TONES_CW,
                       RINGER,
                       AUDIO_PLAY_ONCE);
    TRACE_EVENT("AUDIO FUNCTION CALLED: CALL WAIT");
    ibt_params.last_action = SWITCH_AUDIO_CALLWAIT;
  
  default:
    break;
  }
}


#endif
/*==== EOF ========================================================*/
