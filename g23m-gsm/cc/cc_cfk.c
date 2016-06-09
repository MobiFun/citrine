/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CC_CFK
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
|  Purpose :  This Modul defines the functions called by the primitive
|             processing functions of the SDL process CC.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CC_CFK_C
#define CC_CFK_C

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

/*==== EXPORT =====================================================*/

/*==== PROTOTYPE ==================================================*/
/* Implements Measure#  21 */
LOCAL void cc_set_neg_bearer_cap (const T_M_CC_bearer_cap  * bearer_cap, 
                                        T_MNCC_bcpara     * bcpara, 
                                        T_M_CC_bearer_cap * neg_bearer_cap,
                                        EF_MSCAP       mscap);
/* Implements Measure#  15 */
LOCAL void cc_fill_struct   (UBYTE is_num_pres,
                             T_MNCC_calling_party *calling_party,
                             const T_M_CC_calling_num *calling_num);
/* Implements Measure#  24, 25 and 26 */
LOCAL void cc_fill_sub_struct   (UBYTE is_num_pres, 
                                 T_MNCC_connected_number_sub *connected_number_sub,
                                 const T_M_CC_connect_subaddr *connect_subaddr);
/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_voice_capability        |
+--------------------------------------------------------------------+

  PURPOSE :   This function returns TRUE if the mobile has voice 
              cababilities (support for any speech codec).

*/

GLOBAL BOOL cc_voice_capability (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_voice_capability()");

  return (FldGet (cc_data->mscap.chnMode, spchSupV1) OR
          FldGet (cc_data->mscap.chnMode, AHS) OR
          FldGet (cc_data->mscap.chnMode, AFS));
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)	MODULE	: CC_CFK		     |
| STATE   : code		ROUTINE : cc_bcs_compatible	     |
+--------------------------------------------------------------------+

  PURPOSE : Compare of two bearer capabilities.

*/

GLOBAL BOOL cc_bcs_compatible (const T_M_CC_bearer_cap * bc1,
                               const T_M_CC_bearer_cap * bc2,
                                     BOOL         full)
{
  BOOL itc_chg = FALSE;

  TRACE_FUNCTION ("cc_bcs_compatible()");

  /*
   * Compare Octett 3, Coding Standard
   */
  if (bc1->code NEQ bc2->code)
    return FALSE;

  /*
   * Compare Transfer Mode
   */
  if (bc1->trans_mode NEQ bc2->trans_mode)
    return FALSE;

  /*
   * Compare Information Transfer Capability
   * Note: The combination UDI <-> AUDIO is allowed
   */
  if (bc1->trans_cap NEQ bc2->trans_cap)
  {
    itc_chg = TRUE;

    if (((bc1->trans_cap NEQ M_CC_ITC_DIGITAL_UNRESTRICTED)
         AND
         (bc1->trans_cap NEQ M_CC_ITC_AUDIO))
        OR
        ((bc2->trans_cap NEQ M_CC_ITC_DIGITAL_UNRESTRICTED)
         AND
         (bc2->trans_cap NEQ M_CC_ITC_AUDIO)))
      return FALSE;
  }

  /*
   * if speech is used the compare is successfull
   */
  if (bc1->trans_cap EQ M_CC_ITC_SPEECH OR
      bc1->trans_cap EQ M_CC_ITC_AUXILIARY_SPEECH)
    return TRUE;

  /*
   * Octett 4, Check Existence
   */
  if (bc1->v_compress NEQ bc2->v_compress)
    return FALSE;

  /*
   * Compare Compression
   */
  if (bc1->compress NEQ bc2->compress)
    return FALSE;

  /*
   * Compare Duplex Mode
   */
  if (bc1->duplex NEQ bc2->duplex)
    return FALSE;

  /*
   * Compare Configuration
   */
  if (bc1->config NEQ bc2->config)
    return FALSE;

#ifdef REL99
  if (bc1->v_nirr != bc2->v_nirr)
    return FALSE;
  if (bc1->v_nirr AND (bc1->nirr != bc2->nirr))
    return FALSE;
#endif



  /*
   * Compare Establishment
   */
  if (bc1->establish NEQ bc2->establish)
    return FALSE;

  /*
   * Octett 5, Check Existence
   */
  if (bc1->v_access_ident NEQ bc2->v_access_ident)
    return FALSE;

  /*
   * Compare Rate Adaption
   * Note: Error if same ITC but different rate adaption
   */
  if (bc1->rate_adapt NEQ bc2->rate_adapt)
  {
    if (itc_chg EQ FALSE)
      return FALSE;
  }

  /*
   * Compare Signalling Access Protocol
   */
  if (bc1->sig_access_prot NEQ bc2->sig_access_prot)
    return FALSE;

#ifdef REL99	
  /*
   * Octet 5a, Check Existence and contents
   */
  if (bc1->v_other_itc != bc2->v_other_itc)
    return FALSE;
  if (bc1->v_other_itc AND (bc1->other_itc != bc2->other_itc))
    return FALSE;
  if (bc1->v_other_ra != bc2->v_other_ra)
    return FALSE;
  if (bc1->v_other_ra AND (bc1->other_ra != bc2->other_ra))
    return FALSE;
#endif

  /*
   * Octett 6, Check Existence
   */
  if (bc1->v_l1_ident NEQ bc2->v_l1_ident)
    return FALSE;

  /*
   * Compare User Information Layer 1 Protocol
   */
  if (bc1->v_user_inf_l1_prot NEQ bc2->v_user_inf_l1_prot)
    return FALSE;

  /*
   * Compare Synchronous / Asynchronous
   */
  if (bc1->sync_async NEQ bc2->sync_async)
    return FALSE;

  /*
   * Octett 6a, Check Existence
   */
  if (bc1->v_num_stop NEQ bc2->v_num_stop)
    return FALSE;

  /*
   * Compare negotiation
   */
  if (bc1->negotiate NEQ bc2->negotiate)
    return FALSE;

  /*
   * Compare User Rate
   * Note: For Fax the user rate is unimportant
   */
  if (bc1->user_rate NEQ bc2->user_rate)
  {
    if (bc1->trans_cap NEQ M_CC_ITC_FAX_GROUP_3)
      return FALSE;
  }

  /*
   * Octett 6b, Check Existence
   */
  if (bc1->v_intermed_rate NEQ bc2->v_intermed_rate)
    return FALSE;

  /*
   * Compare Network Independent Clock TX
   */
  if (bc1->nic_tx NEQ bc2->nic_tx)
    return FALSE;

  /*
   * Compare Network Independent Clock RX
   */
  if (bc1->nic_rx NEQ bc2->nic_rx)
    return FALSE;

  /*
   * Octett 6c, Check Existence
   */
  if (bc1->v_conn_elem NEQ bc2->v_conn_elem)
    return FALSE;

  /*
   * Compare Connection Element
   * Note: The combination transparent and non-transparent
   *       is not allowed.
   *       For transparent the structure must be UNSTRUCTED.
   *       For non-transparent the structure must be DATA UNIT INTEGRITY.
   */
  if (((bc1->conn_elem EQ M_CC_CE_TRANSPA) AND
       (bc2->conn_elem EQ M_CC_CE_RLP)) OR
      ((bc1->conn_elem EQ M_CC_CE_RLP) AND
       (bc2->conn_elem EQ M_CC_CE_TRANSPA)))
    return FALSE;

  if (bc1->v_structure AND bc1->v_conn_elem)
  {
    if (bc1->conn_elem EQ M_CC_CE_TRANSPA)
    {
      if (bc1->structure NEQ M_CC_STRC_UNSTRUCTURED)
        return FALSE;
    }
    else
    {
      if (bc1->structure NEQ M_CC_STRC_SERV_DATA_INTEG)
        return FALSE;
    }
  }

  /*
   * Compare Modem Type
   * Note: Error if no change in information
   *       Transfer Capability.
   */
  if (bc1->modem_type NEQ bc2->modem_type)
    if (itc_chg EQ FALSE)
      return FALSE;

  /*
   * Compare of all parameters ?
   */
  if (full EQ FALSE)
    return TRUE;

  /*
   * Compare Number of Stop Bits
   */
  if (bc1->num_stop NEQ bc2->num_stop)
    return FALSE;

  /*
   * Compare Number of Data Bits
   */
  if (bc1->num_data NEQ bc2->num_data)
    return FALSE;

  /*
   * Compare Intermediate Rate
   */
  if (bc1->intermed_rate NEQ bc2->intermed_rate)
    return FALSE;

  /*
   * Compare Parity
   */
  if (bc1->parity NEQ bc2->parity)
    return FALSE;

#ifdef REL99
  /*
   * Octet 6d, Compare Existence only 
   */
  /*(subject to negotiation)*/
  if (bc1->v_modem_type_2 != bc2->v_modem_type_2)
    return FALSE;
  /*(subject to negotiation)*/
  if (bc1->v_fnur != bc2->v_fnur)
    return FALSE;

  if(bc1->v_fnur)
  {
    /*
     * Octet 6e, Compare Existence and contents
     */
    if (bc1->v_acc != bc2->v_acc)
      return FALSE;
    if (bc1->v_acc AND (bc1->acc != bc2->acc))
      return FALSE;
    if (bc1->v_mTch != bc2->v_mTch)
      return FALSE;
    if (bc1->v_mTch AND (bc1->mTch != bc2->mTch))
      return FALSE;

    /* 
     * Theres a extra condition here with MO NT pref negotiated to T because
     * according to 24.008 Octet 6e (UIMI+WAIUR) is not sent in T Bearer Cap
     */
    if(bc1->conn_elem != M_CC_CE_TRANSPA && bc2->conn_elem != M_CC_CE_TRANSPA)
    {
      /*
       * Octet 6f, Compare Existence and contents
       */
      if (bc1->v_uimi != bc2->v_uimi)
        return FALSE;
      if (bc1->v_uimi AND (bc1->uimi != bc2->uimi))
        return FALSE;
      if (bc1->v_waiur != bc2->v_waiur)
        return FALSE;
      if (bc1->v_waiur AND (bc1->waiur != bc2->waiur))
        return FALSE;
    }

    /*
     * Octet 6g, Compare Existence and contents
     */
    if (bc1->v_acc_ext_288 != bc2->v_acc_ext_288)
      return FALSE;
    if (bc1->v_acc_ext_288 AND (bc1->acc_ext_288 != bc2->acc_ext_288))
      return FALSE;
    if (bc1->v_acc_ext_320 != bc2->v_acc_ext_320)
      return FALSE;
    if (bc1->v_acc_ext_320 AND (bc1->acc_ext_320 != bc2->acc_ext_320))
      return FALSE;
    if (bc1->v_acc_ext_432 != bc2->v_acc_ext_432)
      return FALSE;
    if (bc1->v_acc_ext_432 AND (bc1->acc_ext_432 != bc2->acc_ext_432))
      return FALSE;

    if (bc1->v_ch_cod_asym != bc2->v_ch_cod_asym)
      return FALSE;
    if (bc1->v_ch_cod_asym AND (bc1->ch_cod_asym != bc2->ch_cod_asym))
      return FALSE;
  }

#endif  

  /*
   * Octett 7, Compare Existence
   */
  if (bc1->v_l2_ident NEQ bc2->v_l2_ident)
    return FALSE;

  /*
   * Compare User Information Layer 2 Protocol
   */
  if (bc1->v_l2_ident AND
      (bc1->user_inf_l2_prot NEQ bc2->user_inf_l2_prot))
    return FALSE;

  return TRUE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_bc                |
+--------------------------------------------------------------------+

  PURPOSE : Define bearer capability depending on the bearer capability
            parameters.

*/
GLOBAL void cc_build_bc (T_M_CC_bearer_cap * bearer_cap,   /* out */
                         UBYTE * serv,                /* out */
                         const T_MNCC_bcpara * bc_params)  /* in*/
{

  TRACE_FUNCTION ("cc_build_bc()");

  cc_csf_ms_cap ();

  switch (bc_params->bearer_serv)
  {
    case MNCC_BEARER_SERV_SPEECH:
    case MNCC_BEARER_SERV_AUX_SPEECH:
    case MNCC_BEARER_SERV_SPEECH_CTM:
    case MNCC_BEARER_SERV_AUX_SPEECH_CTM:
      * serv = MNCC_SERV_SPEECH;
      memset (bearer_cap, 0, sizeof (T_M_CC_bearer_cap));
      cc_set_trans_cap (bearer_cap, bc_params);
             
      cc_set_radio_channel_requirement (bearer_cap, bc_params);
      break;

    case MNCC_BEARER_SERV_FAX:
      /*
       * Only transparent is supported
       */
      * serv = MNCC_SERV_DATA;
      memset (bearer_cap, 0, sizeof (T_M_CC_bearer_cap));
      cc_set_trans_cap (bearer_cap, bc_params);
      cc_set_radio_channel_requirement (bearer_cap, bc_params);
      cc_set_data_default_parameter (bearer_cap);
      cc_set_conn_elem (bearer_cap, MNCC_CONN_ELEM_TRANS, bc_params->flow_control);
      cc_set_user_rate (bearer_cap, bc_params->rate, M_CC_MT_NONE);
      cc_set_sync_async (bearer_cap, bc_params);
      break;

    case MNCC_BEARER_SERV_PACKET_ACCESS:
    case MNCC_BEARER_SERV_SYNC:
      * serv = MNCC_SERV_DATA;
      memset (bearer_cap, 0, sizeof (T_M_CC_bearer_cap));
      cc_set_trans_cap (bearer_cap, bc_params);
      cc_set_radio_channel_requirement (bearer_cap, bc_params);
      cc_set_data_default_parameter (bearer_cap);
      cc_set_conn_elem (bearer_cap, bc_params->conn_elem, bc_params->flow_control);
      cc_set_user_rate (bearer_cap, bc_params->rate, bc_params->modem_type);
      cc_set_sync_async (bearer_cap, bc_params);
      break;

    case MNCC_BEARER_SERV_PAD_ACCESS:
    case MNCC_BEARER_SERV_ASYNC:
      * serv = MNCC_SERV_DATA;
      memset (bearer_cap, 0, sizeof (T_M_CC_bearer_cap));
      cc_set_trans_cap (bearer_cap, bc_params);
      cc_set_radio_channel_requirement (bearer_cap, bc_params);
      cc_set_data_default_parameter (bearer_cap);
      cc_set_conn_elem (bearer_cap, bc_params->conn_elem, bc_params->flow_control);
      cc_set_user_rate (bearer_cap, bc_params->rate, bc_params->modem_type);
      cc_set_sync_async (bearer_cap, bc_params);
      break;
    
    default:
      TRACE_ERROR ("Unexpected bearer_serv");
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CC_CFK                        |
| STATE   : code             ROUTINE : cc_set_data_default_parameter |
+--------------------------------------------------------------------+

  PURPOSE : Set bearer capability parameters for data.

*/

GLOBAL void cc_set_data_default_parameter (T_M_CC_bearer_cap * bearer_cap)
{
  TRACE_FUNCTION ("cc_set_data_default_parameter()");

  /*
   * Octet 3
   */
#ifdef REL99
  bearer_cap->v_code         = TRUE;
  bearer_cap->code           = M_CC_CSTD_GSM;
  bearer_cap->v_trans_mode   = TRUE;
  bearer_cap->trans_mode     = M_CC_TM_CIRCUIT;
#endif

  /*
   * Octet 4
   */
  bearer_cap->v_compress = TRUE;      /* now spare, used in previous releases */
  bearer_cap->compress   = M_CC_COMP_NO;
                                      /* structure -> by other function       */
  bearer_cap->v_duplex   = TRUE;
  bearer_cap->duplex     = M_CC_DUPLEX_FULL;

  bearer_cap->v_config   = TRUE;
  bearer_cap->config     = M_CC_CONF_POINT_TO_POINT;
                                       /* nirr -> by other function           */
  bearer_cap->v_establish= TRUE;
  bearer_cap->establish  = M_CC_ESTAB_DEMAND;

  /*
   * Octet 5
   */
  bearer_cap->v_access_ident = TRUE;
  bearer_cap->access_ident   = M_CC_AI_OCT_ID;

  bearer_cap->v_sig_access_prot = TRUE;
  bearer_cap->sig_access_prot = M_CC_SIAP_I440;

  /*
   * Octet 6
   */
  bearer_cap->v_l1_ident = TRUE;
  bearer_cap->l1_ident = M_CC_L1_OCT_ID;

  bearer_cap->v_user_inf_l1_prot = TRUE;
  bearer_cap->user_inf_l1_prot = M_CC_L1_DEFAULT;
                                       /* sync_async -> by other function    */
  /*
   * Octet 6a
   */
                                       /* num_stop -> by other function      */
  bearer_cap->v_negotiate = TRUE;
  bearer_cap->negotiate   = M_CC_NEGOTIATE_NO;
                                       /* num_data -> by other function      */
                                       /* user_rate -> by other function     */
  /*
   * Octet 6b
   */
                                       /* intermed_rate -> by other function */
  bearer_cap->v_nic_tx = TRUE;
  bearer_cap->nic_tx   = M_CC_NIC_TX_NO;
  bearer_cap->v_nic_rx = TRUE;
  bearer_cap->nic_rx   = M_CC_NIC_RX_NO;
                                       /* parity -> by other function        */
                                       /* conn_elem -> by other function     */
                                       /* modem_type -> by other function    */
  /*
   * Octet 7
   */
                                        /* l2_ident -> by other function         */
                                        /* user_inf_l2_prot -> by other function */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147) MODULE  : CC_CFK                           |
| STATE   : code          ROUTINE : cc_set_radio_channel_requirement |
+--------------------------------------------------------------------+

  PURPOSE : Set bearer capability parameters for data.

*/
GLOBAL void cc_set_radio_channel_requirement (T_M_CC_bearer_cap * bearer_cap,
                                        const T_MNCC_bcpara     * bcpara)
{
  GET_INSTANCE_DATA;
  UBYTE index, prio;
  const UBYTE codec_prio[5] = {M_CC_SPEECH_VERS_AMR_FR, M_CC_SPEECH_VERS_AMR_HR, M_CC_SPEECH_VERS_EFR,
                               M_CC_SPEECH_VERS_FR, M_CC_SPEECH_VERS_HR};
        UBYTE codec_val[5]  = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  TRACE_FUNCTION ("cc_set_radio_channel_requirement()");

  switch (bcpara->bearer_serv)
  {
    case MNCC_BEARER_SERV_SPEECH:
    case MNCC_BEARER_SERV_AUX_SPEECH:
    case MNCC_BEARER_SERV_SPEECH_CTM:
    case MNCC_BEARER_SERV_AUX_SPEECH_CTM:
      bearer_cap->v_rad_chan_req = TRUE;
      bearer_cap->v_code         = TRUE;
      bearer_cap->code           = M_CC_CSTD_GSM;
      bearer_cap->v_trans_mode   = TRUE;
      bearer_cap->trans_mode     = M_CC_TM_CIRCUIT;
      index = (UBYTE)FldGet(cc_data->mscap.chnMode, HR_EFRSup);

      TRACE_EVENT_P1 ("index = %d", index);
     if(bcpara->rad_chan_req NEQ 0)
     	{
     	 TRACE_EVENT_P1 ("bearer_cap->rad_chan_req = %d", bearer_cap->rad_chan_req);
       bearer_cap->rad_chan_req   = bcpara->rad_chan_req ; 
     	}
     else
      bearer_cap->rad_chan_req   = bc_prio_0 [index];
      index = 0;
      for (prio = 0; prio < MAX_SPEECH_CODECS; prio++)
      {
        switch (codec_prio[prio])
        {
          case M_CC_SPEECH_VERS_FR: /* Full rate speech version 1 */
            /* Always */
            codec_val[index] = M_CC_SPEECH_VERS_FR;
            index++;
            break;
          case M_CC_SPEECH_VERS_HR:     /* Half rate speech version 1 */
            if (FldGet (cc_data->mscap.chnMode, hrSup) NEQ 0)
            {
              codec_val[index] = M_CC_SPEECH_VERS_HR;
              index++;
            }
            break;
          case M_CC_SPEECH_VERS_EFR:    /* Full rate speech version 2 - EFR */
            if (FldGet (cc_data->mscap.chnMode, EFRSupV2) NEQ 0)
            {
              codec_val[index] = M_CC_SPEECH_VERS_EFR;
              index++;
            }
            break;
          case M_CC_SPEECH_VERS_AMR_HR: /* Half rate speech version 3 - AMR */
            if ((FldGet (cc_data->mscap.chnMode, AHS) NEQ 0) 
                 #if (CHIPSET != 15)
                 AND !(bcpara->bearer_serv EQ MNCC_BEARER_SERV_SPEECH_CTM OR
                      bcpara->bearer_serv EQ MNCC_BEARER_SERV_AUX_SPEECH_CTM)
                 #endif
               ) /* TTY over AMR is currently not supported by DSP, except Locosto Chipset 15*/
            {
              codec_val[index] = M_CC_SPEECH_VERS_AMR_HR;   
              index++;
            }
              
            break;
          case M_CC_SPEECH_VERS_AMR_FR: /* Full rate speech version 3 - AMR */
            if ((FldGet (cc_data->mscap.chnMode, AFS) NEQ 0) 
                 #if (CHIPSET != 15) 
                 AND !(bcpara->bearer_serv EQ MNCC_BEARER_SERV_SPEECH_CTM OR
                       bcpara->bearer_serv EQ MNCC_BEARER_SERV_AUX_SPEECH_CTM)
                 #endif   
               ) /* TTY over AMR is currently not supported by the DSP, except Locosto Chipset 15*/
              {
                codec_val[index] = M_CC_SPEECH_VERS_AMR_FR;
                index++;
              }
              break;
          } /* switch (codec_prio[prio])*/
        } /* for */

        if (bcpara->bearer_serv EQ MNCC_BEARER_SERV_SPEECH_CTM OR
            bcpara->bearer_serv EQ MNCC_BEARER_SERV_AUX_SPEECH_CTM)
        {
          bearer_cap->v_ctm = TRUE;
          bearer_cap->ctm   = M_CC_CTM_YES;
          bearer_cap->v_coding_bc3x1 = TRUE;
          bearer_cap->coding_bc3x1   = M_CC_CODING_BC3X_SPEECH;
          bearer_cap->v_speech_vers1 = TRUE;

          if (codec_val[0] NEQ NOT_PRESENT_8BIT)
          {
            bearer_cap->speech_vers1 = codec_val[0];
          }
          else switch (bearer_cap->rad_chan_req)
          {
            case M_CC_RCR_HALF_PREF:
              bearer_cap->speech_vers1 = M_CC_SPEECH_VERS_HR;
              break;
            case M_CC_RCR_FULL_PREF:
            default:
              bearer_cap->speech_vers1 = M_CC_SPEECH_VERS_FR;
              break;
          }
        }
        else if (codec_val[0] NEQ NOT_PRESENT_8BIT)
        {
          bearer_cap->v_ctm = TRUE;
          bearer_cap->ctm   = M_CC_CTM_NO;
          bearer_cap->v_coding_bc3x1 = TRUE;
          bearer_cap->coding_bc3x1   = M_CC_CODING_BC3X_SPEECH;
          bearer_cap->v_speech_vers1 = TRUE;
          bearer_cap->speech_vers1 = codec_val[0];
        }
        if (codec_val[1] NEQ NOT_PRESENT_8BIT)
        {
          bearer_cap->v_speech_vers2 = TRUE;
          bearer_cap->speech_vers2   = codec_val[1];
          bearer_cap->v_coding_bc3x2 = TRUE;
          bearer_cap->coding_bc3x2   = M_CC_CODING_BC3X_SPEECH;
        }
        if (codec_val[2] NEQ NOT_PRESENT_8BIT)
        {
          bearer_cap->v_speech_vers3 = TRUE;
          bearer_cap->speech_vers3   = codec_val[2];
          bearer_cap->v_coding_bc3x3 = TRUE;
          bearer_cap->coding_bc3x3   = M_CC_CODING_BC3X_SPEECH;
        }
        if (codec_val[3] NEQ NOT_PRESENT_8BIT)
        {
          bearer_cap->v_speech_vers4 = TRUE;
          bearer_cap->speech_vers4   = codec_val[3];
          bearer_cap->v_coding_bc3x4 = TRUE;
          bearer_cap->coding_bc3x4   = M_CC_CODING_BC3X_SPEECH;
        }
        if (codec_val[4] NEQ NOT_PRESENT_8BIT)
        {
          bearer_cap->v_speech_vers5 = TRUE;
          bearer_cap->speech_vers5   = codec_val[4];
          bearer_cap->v_coding_bc3x5 = TRUE;
          bearer_cap->coding_bc3x5   = M_CC_CODING_BC3X_SPEECH;
        }
      break;
    default:
      bearer_cap->v_rad_chan_req = TRUE;
      bearer_cap->v_code         = TRUE;
      bearer_cap->code           = M_CC_CSTD_GSM;
      bearer_cap->v_trans_mode   = TRUE;
      bearer_cap->trans_mode     = M_CC_TM_CIRCUIT;

      if (FldGet (cc_data->mscap.datCap2, DHRSup) AND
          bcpara->bearer_serv NEQ MNCC_BEARER_SERV_FAX)  /* TA 29.2.3.7 */
        /* Halfrate support */
        bearer_cap->rad_chan_req = M_CC_RCR_FULL_PREF;
      else
        bearer_cap->rad_chan_req = M_CC_RCR_FULL_ONLY;
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147) MODULE  : CC_CFK                           |
| STATE   : code          ROUTINE : cc_set_trans_cap                 |
+--------------------------------------------------------------------+

  PURPOSE : Set bearer capability parameters for data.

*/

GLOBAL void cc_set_trans_cap (T_M_CC_bearer_cap * bearer_cap,
                        const T_MNCC_bcpara     * bcpara)
{
  TRACE_FUNCTION ("cc_set_trans_cap()");

  bearer_cap->v_trans_cap  = TRUE;

  switch (bcpara->bearer_serv)
  {
    case MNCC_BEARER_SERV_SPEECH:
    case MNCC_BEARER_SERV_SPEECH_CTM:
      bearer_cap->trans_cap    = M_CC_ITC_SPEECH;
      break;

    case MNCC_BEARER_SERV_AUX_SPEECH:
    case MNCC_BEARER_SERV_AUX_SPEECH_CTM:
      /*
       * Alternate Line Service
       */
      bearer_cap->trans_cap    = M_CC_ITC_AUXILIARY_SPEECH;
      break;

    case MNCC_BEARER_SERV_FAX:
      bearer_cap->trans_cap    = M_CC_ITC_FAX_GROUP_3;
      bearer_cap->rate_adapt   = M_CC_RA_NONE;
      bearer_cap->v_rate_adapt = TRUE;
      break;

    case MNCC_BEARER_SERV_ASYNC:
    case MNCC_BEARER_SERV_SYNC:
    case MNCC_BEARER_SERV_PAD_ACCESS:
    case MNCC_BEARER_SERV_PACKET_ACCESS:
#ifdef REL99
    /* MT - Modem type digital */
    if (bcpara->modem_type EQ M_CC_MT_NONE)
    {
      /* ITC - Information Transfer Capability */
      if (bcpara->transfer_cap == MNCC_ITC_UDI)
      {
        bearer_cap->trans_cap = M_CC_ITC_DIGITAL_UNRESTRICTED;
      }
      else if (bcpara->transfer_cap == MNCC_ITC_RDI) 
      {
        bearer_cap->trans_cap   = M_CC_ITC_OTHER;
        bearer_cap->other_itc   = M_CC_ITC_DIGITAL_RESTRICTED;
        bearer_cap->v_other_itc = TRUE;
      }
      /* RA - Rate adaption */
      if (bcpara->rate_adaption == MNCC_RATE_ADAPT_V110)
      {
        bearer_cap->rate_adapt  = M_CC_RA_V110;
      }
      else if (bcpara->rate_adaption == MNCC_RATE_ADAPT_V120)
      {
        bearer_cap->rate_adapt  = M_CC_RA_OTHER;
        bearer_cap->other_ra    = M_CC_RA_V120;
        bearer_cap->v_other_itc  = TRUE; /* set for CCD encoding of Octet 5a */
        bearer_cap->v_other_ra  = TRUE;
      }
    }
    /* MT - Modem type audio */
    else
    {
      bearer_cap->trans_cap     =   M_CC_ITC_AUDIO;
      if (bcpara->rate_adaption == MNCC_RATE_ADAPT_H223_H245)
      {
        bearer_cap->rate_adapt  = M_CC_RA_OTHER;
        bearer_cap->v_other_itc  = TRUE; /* set for CCD encoding of Octet 5a */
        bearer_cap->v_other_ra  = TRUE;
        bearer_cap->other_ra    = M_CC_RA_H223_H245;
      }
      else
      {
        bearer_cap->rate_adapt    =   M_CC_RA_NONE;
      }
    }
    bearer_cap->v_rate_adapt = TRUE; 
    break;
#else
    if (bcpara->modem_type EQ M_CC_MT_NONE)
      {
        bearer_cap->trans_cap    = M_CC_ITC_DIGITAL_UNRESTRICTED;
        bearer_cap->rate_adapt   = M_CC_RA_V110;
      }
      else
      {
        bearer_cap->trans_cap    = M_CC_ITC_AUDIO;
        bearer_cap->rate_adapt   = M_CC_RA_NONE;
      }
      bearer_cap->v_rate_adapt = TRUE;
      break;
#endif

  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_set_conn_elem           |
+--------------------------------------------------------------------+

  PURPOSE : Set bearer capability parameters according to connection
            element.

*/

GLOBAL void cc_set_conn_elem (T_M_CC_bearer_cap * bearer_cap,
                              UBYTE          conn_elem,
                              UBYTE          flow_control)
{
  TRACE_FUNCTION ("cc_set_conn_elem()");

  bearer_cap->v_structure = TRUE;
  bearer_cap->v_conn_elem = TRUE;
  switch (conn_elem)
  {
    case MNCC_CONN_ELEM_TRANS:
      bearer_cap->conn_elem = M_CC_CE_TRANSPA;
      bearer_cap->structure = M_CC_STRC_UNSTRUCTURED;
      flow_control          = MNCC_OUTBAND_FLOW_CONTROL;
      break;
    case MNCC_CONN_ELEM_NON_TRANS:
      bearer_cap->conn_elem = M_CC_CE_RLP;
      bearer_cap->structure = M_CC_STRC_SERV_DATA_INTEG;
      flow_control          = MNCC_OUTBAND_FLOW_CONTROL;
      break;
    case MNCC_CONN_ELEM_TRANS_PREF:
      bearer_cap->conn_elem = M_CC_CE_TRANSPA_PREF;
      bearer_cap->structure = M_CC_STRC_SERV_DATA_INTEG;
      flow_control          = MNCC_OUTBAND_FLOW_CONTROL;
      break;
    case MNCC_CONN_ELEM_NON_TRANS_PREF:
      bearer_cap->conn_elem = M_CC_CE_RLP_PREF;
      bearer_cap->structure = M_CC_STRC_SERV_DATA_INTEG;
      flow_control          = MNCC_OUTBAND_FLOW_CONTROL;
      break;
    default:
      break;
  }
  switch (flow_control)
  {
    case MNCC_INBAND_FLOW_CONTROL:
    case MNCC_NO_FLOW_CONTROL:
      bearer_cap->v_l2_ident = TRUE;
      bearer_cap->l2_ident   = M_CC_L2_OCT_ID;
      bearer_cap->v_user_inf_l2_prot = TRUE;
      bearer_cap->user_inf_l2_prot = flow_control;
      break;
    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_set_sync_async          |
+--------------------------------------------------------------------+

  PURPOSE : Set bearer capability parameters according to synchronous
            flag.

*/

GLOBAL void cc_set_sync_async (T_M_CC_bearer_cap * bearer_cap,
                         const T_MNCC_bcpara     * bc_para)
{
  TRACE_FUNCTION ("cc_set_sync_async()");

  bearer_cap->v_sync_async = TRUE;
  bearer_cap->v_num_stop   = TRUE;
  bearer_cap->v_num_data   = TRUE;
  bearer_cap->v_parity     = TRUE;

  switch (bc_para->bearer_serv)
  {
    case MNCC_BEARER_SERV_SYNC:
    case MNCC_BEARER_SERV_PACKET_ACCESS:
    case MNCC_BEARER_SERV_FAX:
      bearer_cap->sync_async = 0;
      bearer_cap->num_stop   = M_CC_NSTOP_1;
      bearer_cap->num_data   = M_CC_NDATA_8;
      bearer_cap->parity     = bc_para->parity;
      break;
    default:
      bearer_cap->sync_async = 1;
      bearer_cap->num_stop   = bc_para->stop_bits;
      bearer_cap->num_data   = bc_para->data_bits;
      bearer_cap->parity     = bc_para->parity;
     break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_set_user_rate           |
+--------------------------------------------------------------------+

  PURPOSE : Set bearer capability parameters according to user rate.

*/


GLOBAL void cc_set_user_rate (T_M_CC_bearer_cap * bearer_cap,
                              UBYTE          rate,
                              UBYTE          modem_type)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_set_user_rate()");

  bearer_cap->v_user_rate  = TRUE;
  bearer_cap->v_nirr       = TRUE;
  bearer_cap->nirr         = M_CC_NIRR_NO_MEANING;

  bearer_cap->v_intermed_rate = TRUE;
  bearer_cap->v_modem_type    = TRUE;

  switch( modem_type )
  {
    case MNCC_MT_V32_BIS:
      bearer_cap->modem_type = M_CC_MT_V32;
      bearer_cap->v_modem_type_2 = TRUE;
      bearer_cap->modem_type_2   = M_CC_OTHER_MODEM_TYPE_V32BIS;
      break;

    case MNCC_MT_V34:
      bearer_cap->modem_type = M_CC_MT_V32;
      bearer_cap->v_modem_type_2 = TRUE;
      bearer_cap->modem_type_2   = M_CC_OTHER_MODEM_TYPE_V34;
      break;

    default:
      bearer_cap->modem_type   = modem_type;
      if( rate EQ MNCC_UR_14_4_KBIT )
      {
        bearer_cap->v_modem_type_2 = TRUE;
        bearer_cap->modem_type_2   = M_CC_OTHER_MODEM_TYPE_NONE;
      }
  }

  switch (rate)
  {
    case MNCC_UR_0_3_KBIT:
    case MNCC_UR_1_2_KBIT:
    case MNCC_UR_2_4_KBIT:
    case MNCC_UR_4_8_KBIT:
    case MNCC_UR_1_2_KBIT_V23:
      bearer_cap->user_rate       = rate;
      if (bearer_cap->conn_elem EQ M_CC_CE_RLP)
        bearer_cap->intermed_rate = M_CC_IR_16_KBIT;
      else
        bearer_cap->intermed_rate = M_CC_IR_8_KBIT;
     
      /*
       * PZ removed because R&S does not support 6 kbit channels
      if (bearer_cap->conn_elem EQ M_CC_CE_RLP AND
          bearer_cap->rad_chan_req EQ RCR_FULL_ONLY)
        bearer_cap->nirr = M_CC_NIRR_DATA;
       */
      break;
    case MNCC_UR_14_4_KBIT:
      bearer_cap->user_rate      = MNCC_UR_9_6_KBIT;
      bearer_cap->intermed_rate  = M_CC_IR_16_KBIT;
      bearer_cap->v_fnur         = TRUE;
      bearer_cap->fnur           = M_CC_FNUR_14400;
      bearer_cap->v_mTch         = TRUE;
      bearer_cap->mTch           = M_CC_MAX_TCH_1;
 
      /* 
       * These parameters are only used for NT data.
       * Otherwise they are optional (i.e. omitted).
       */
      if (bearer_cap->conn_elem == M_CC_CE_RLP)
      {
        bearer_cap->v_uimi         = TRUE;
        bearer_cap->uimi           = M_CC_UIMI_NOT_ALLOWED;
        bearer_cap->v_waiur        = TRUE;
        bearer_cap->waiur          = M_CC_WAIUR_14400;
      }

      bearer_cap->v_acc          = TRUE;
      bearer_cap->acc            = M_CC_ACC_96;        

      if (FldGet (cc_data->mscap.datCap2, DHRSup))
        bearer_cap->acc         += M_CC_ACC_48;
      if (FldGet (cc_data->mscap.datCap1, Dr14_4Sup))
        bearer_cap->acc         += M_CC_ACC_144;
      break;
    default:
      bearer_cap->user_rate     = rate;
      bearer_cap->intermed_rate = M_CC_IR_16_KBIT;
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_call_confirm      |
+--------------------------------------------------------------------+

  PURPOSE : Filling of the C-Structure for call confirm message.

*/

GLOBAL void cc_build_call_confirm ( T_U_CALL_CONF * call_cnf,
                                   USHORT          cause)
{
  GET_INSTANCE_DATA;	
  TRACE_FUNCTION ("cc_build_call_confirm()");

  call_cnf->msg_type = U_CALL_CONF;

  if (cc_data->negotiation)
  {
    if (cc_data->neg_ri EQ NOT_PRESENT_8BIT)
      call_cnf->v_repeat = FALSE;
    else
    {
      call_cnf->v_repeat = TRUE;
      call_cnf->repeat   = cc_data->neg_ri;
    }

    if (cc_data->neg_serv1 EQ NOT_PRESENT_8BIT)
      call_cnf->v_bearer_cap = FALSE;
    else
    {
      call_cnf->v_bearer_cap = TRUE;
      call_cnf->bearer_cap   = cc_data->neg_bc1;
    }

    if (cc_data->neg_serv2 EQ NOT_PRESENT_8BIT)
      call_cnf->v_bearer_cap_2 = FALSE;
    else
    {
      call_cnf->v_bearer_cap_2 = TRUE;
      memcpy (&call_cnf->bearer_cap_2, &cc_data->neg_bc2,
              sizeof (T_M_CC_bearer_cap));
    }
  }
  else
  {
      call_cnf->v_repeat = FALSE;
      call_cnf->v_bearer_cap = FALSE;
      call_cnf->v_bearer_cap_2 = FALSE;
  }
  if (IS_CAUSE_INVALID(cause))
    call_cnf->v_cc_cause = FALSE;
  else
  {
    call_cnf->v_cc_cause = TRUE;
    cc_encode_cause (&call_cnf->cc_cause, cause);
  }

  /*
   * GSM 24.008 has no clear advice when to include the CC capabilities,
   * neither in clause 5.2.2.3 nor in clause 9.3.2, but it is assumed that 
   * it has to be included if at least one capability is present.
   * This means, with R99 and above always.
   */
  call_cnf->v_call_ctrl_cap = (cc_data->call_ctrl_cap.dtmf NEQ 0) OR
                              (cc_data->call_ctrl_cap.pcp NEQ 0);
  if (call_cnf->v_call_ctrl_cap)
  {
    call_cnf->call_ctrl_cap   = cc_data->call_ctrl_cap; /* Struct copy */
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_cc_est_confirm    |
+--------------------------------------------------------------------+

  PURPOSE : Filling of the C-Structure for CC EST. CONFIRMED message.

*/

GLOBAL void cc_build_cc_est_confirm (T_U_CC_EST_CONF * cc_est_conf,
                                     USHORT            cause)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_build_cc_est_confirm()");

  cc_est_conf->msg_type = U_CC_EST_CONF;
  if (cc_data->neg_ri EQ NOT_PRESENT_8BIT) 
    cc_est_conf->v_repeat = FALSE;
  else
  {
    cc_est_conf->v_repeat = TRUE;
    cc_est_conf->repeat = cc_data->neg_ri;
  }

  cc_est_conf->v_bearer_cap = TRUE;
  cc_est_conf->bearer_cap   = cc_data->neg_bc1;

  if (cc_data->neg_serv2 EQ NOT_PRESENT_8BIT)
    cc_est_conf->v_bearer_cap_2 = FALSE;
  else
  {
    cc_est_conf->v_bearer_cap_2 = TRUE;
    memcpy (&cc_est_conf->bearer_cap_2, &cc_data->neg_bc2, 
            sizeof (T_M_CC_bearer_cap));
  }

  if (IS_CAUSE_INVALID(cause))
    cc_est_conf->v_cc_cause = FALSE;
  else
  {
    cc_est_conf->v_cc_cause = TRUE;
    cc_encode_cause (&cc_est_conf->cc_cause, cause);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_congestion_control|
+--------------------------------------------------------------------+

  PURPOSE : Filling of the C-Structure for congestion control message.

*/

GLOBAL void cc_build_congestion_control (const T_MNCC_USER_REQ  * user,
                                               T_B_CONGEST_CTRL * cong_ctrl)
{
  TRACE_FUNCTION ("cc_build_congestion_control()");

  cong_ctrl->msg_type     = B_CONGEST_CTRL;
  cong_ctrl->congest_lev  = user->congest_lev;
  cong_ctrl->v_cc_cause   = FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_disconnect        |
+--------------------------------------------------------------------+

  PURPOSE : Filling of the C-Structure for disconnect message.

*/
GLOBAL void cc_build_disconnect (T_U_DISCONNECT * disconnect,
                                 USHORT           cause,
                           const T_NAS_fac_inf      * fac_inf,
                                 UBYTE            ss_ver)
{

  TRACE_FUNCTION ("cc_build_disconnect()");

  disconnect->msg_type     = U_DISCONNECT;
  cc_encode_cause (&disconnect->cc_cause, cause);

  disconnect->v_facility   = FALSE;
  disconnect->v_user_user  = FALSE;
  disconnect->v_ss_version = FALSE;

  if ((fac_inf NEQ NULL) AND (fac_inf->l_fac NEQ 0))
  {
    /* Send facility IE */
    disconnect->v_facility = TRUE;
    disconnect->facility.c_fac = fac_inf->l_fac >> 3;
    memcpy (disconnect->facility.fac, 
      &fac_inf->fac[fac_inf->o_fac >> 3],
      disconnect->facility.c_fac);

    if (ss_ver NEQ MNCC_SS_VER_NOT_PRES)
    {
      disconnect->v_ss_version      = TRUE;
      disconnect->ss_version.c_ver  = 1;
      disconnect->ss_version.ver[0] = ss_ver;
    }
  }

  /* no security check for validity of cause; cause is mandatory in DISCONNECT */
  cc_encode_cause (&disconnect->cc_cause, cause);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_emergency_setup   |
+--------------------------------------------------------------------+

  PURPOSE : Filling of the C-Structure for emergency setup message.

*/

GLOBAL void cc_build_emergency_setup (T_U_EMERGE_SETUP * emergency_setup)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_build_emergency_setup()");

  emergency_setup->msg_type     = U_EMERGE_SETUP;
/*emergency_setup->v_bearer_cap = FALSE;*/
  emergency_setup->v_bearer_cap = TRUE;
  emergency_setup->bearer_cap   = cc_data->bc1;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_facility          |
+--------------------------------------------------------------------+

  PURPOSE : Filling of the C-Structure for facility message.

*/

GLOBAL void cc_build_facility (const T_MNCC_FACILITY_REQ * facility,
                                     T_U_FACILITY        * facility_msg)
{
  UBYTE length;

  TRACE_FUNCTION ("cc_build_facility()");

  length = facility->fac_inf.l_fac>>3;

  if (length > 251)
    return;

  facility_msg->msg_type       = U_FACILITY;
  facility_msg->facility.c_fac = length;
  memcpy (facility_msg->facility.fac, 
          &facility->fac_inf.fac[facility->fac_inf.o_fac>>3], length);

  if (facility->ss_version EQ NOT_PRESENT_8BIT)
    facility_msg->v_ss_version = FALSE;
  else
  {
    facility_msg->v_ss_version      = TRUE;
    facility_msg->ss_version.c_ver  = 1;
    facility_msg->ss_version.ver[0] = facility->ss_version;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_mncc_alert_ind    |
+--------------------------------------------------------------------+

  PURPOSE : Filling of the C-Structure for MNCC_ALERT_IND.

*/

GLOBAL void cc_build_mncc_alert_ind ( const T_D_ALERT  * alert,
                                     T_MNCC_ALERT_IND * alert_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_build_mncc_alert_ind()");

  alert_ind->ti = cc_data->ti;
  
  if (alert->v_progress)
  {
    alert_ind->progress_desc = alert->progress.progress_desc;
  }
  else
  {
    alert_ind->progress_desc = NOT_PRESENT_8BIT;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_mncc_proceed_ind  |
+--------------------------------------------------------------------+

  PURPOSE : Filling of the C-Structure for MNCC_CALL_PROCEED_IND.

*/

GLOBAL void cc_build_mncc_proceed_ind (const T_D_CALL_PROCEED * proceed,
                                       T_MNCC_CALL_PROCEED_IND * proceed_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_build_mncc_proceed_ind()");

  proceed_ind->ti = cc_data->ti;
  proceed_ind->ri = cc_data->neg_ri;
  memcpy (&proceed_ind->bcpara, &cc_data->neg_bcpara1, sizeof (T_MNCC_bcpara));
  memcpy (&proceed_ind->bcpara2, &cc_data->neg_bcpara2, sizeof (T_MNCC_bcpara));

#if defined (WIN32)
  TRACE_EVENT_P1 ("Rate = %d", cc_data->neg_bcpara1.rate);
  TRACE_EVENT_P1 ("Bearer Serv = %d", cc_data->neg_bcpara1.bearer_serv);
  TRACE_EVENT_P1 ("Conn Elem = %d", cc_data->neg_bcpara1.conn_elem);
  TRACE_EVENT_P1 ("Stop Bits = %d", cc_data->neg_bcpara1.stop_bits);
  TRACE_EVENT_P1 ("Data Bits = %d", cc_data->neg_bcpara1.data_bits);
  TRACE_EVENT_P1 ("Parity = %d", cc_data->neg_bcpara1.parity);
  TRACE_EVENT_P1 ("Flow Control = %d", cc_data->neg_bcpara1.flow_control);
#endif

  if (proceed->v_progress)
  {
    proceed_ind->progress_desc = proceed->progress.progress_desc;
  }
  else
  {
    proceed_ind->progress_desc = NOT_PRESENT_8BIT;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_mncc_progress_ind |
+--------------------------------------------------------------------+

  PURPOSE : Filling of the C-Structure for MNCC_PROGRESS_IND.

*/

GLOBAL void cc_build_mncc_progress_ind (const T_D_PROGRESS  * progress,
                                        T_MNCC_PROGRESS_IND * progress_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_build_mncc_progress_ind()");

  progress_ind->ti = cc_data->ti;
  progress_ind->progress_desc = progress->progress.progress_desc;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_mncc_setup_cnf    |
+--------------------------------------------------------------------+

  PURPOSE : Filling of the C-Structure for MNCC_SETUP_CNF.

*/

GLOBAL void cc_build_mncc_setup_cnf (const T_D_CONNECT * connect,
                                     T_MNCC_SETUP_CNF  * setup_cnf)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_build_mncc_setup_cnf()");

  setup_cnf->ti  = cc_data->ti;
  setup_cnf->cause = MNCC_CAUSE_SUCCESS;

  if (connect->v_progress)
  {
    setup_cnf->progress_desc = connect->progress.progress_desc;
  }
  else
  {
    setup_cnf->progress_desc = NOT_PRESENT_8BIT;
  }
/* Implements Measure#  15 */
  cc_fill_struct (connect->v_connect_num,
                  &setup_cnf->connected_number,
                  (T_M_CC_calling_num*)&connect->connect_num);
/* Implements Measure#  24 */
  cc_fill_sub_struct (connect->v_connect_subaddr,
                      &setup_cnf->connected_number_sub,
                      &connect->connect_subaddr);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_mncc_setup_ind    |
+--------------------------------------------------------------------+

  PURPOSE : Filling of the C-Structure for MNCC_SETUP_IND.

*/

GLOBAL void cc_build_mncc_setup_ind (const T_D_SETUP  * setup,
                                     T_MNCC_SETUP_IND * setup_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_build_mncc_setup_ind()");

  setup_ind->ti = cc_data->ti;

  setup_ind->ri = cc_data->neg_ri;
  memcpy (&setup_ind->bcpara, &cc_data->neg_bcpara1, sizeof (T_MNCC_bcpara));
  memcpy (&setup_ind->bcpara2, &cc_data->neg_bcpara2, sizeof (T_MNCC_bcpara));

  if (setup->v_progress)
  {
    setup_ind->progress_desc = setup->progress.progress_desc;
  }
  else
  {
    setup_ind->progress_desc = MNCC_PROG_NOT_PRES;
  }

  if (setup->v_signal)
    setup_ind->sig = setup->signal;
  else
    setup_ind->sig = MNCC_SIG_NOT_PRES;
/* Implements Measure#  15 */
  cc_fill_struct (setup->v_calling_num,
                  &setup_ind->calling_party,
                  &setup->calling_num);
/* Implements Measure#  24 */
  cc_fill_sub_struct (setup->v_calling_subaddr,
                      &setup_ind->calling_party_sub,
                      (T_M_CC_connect_subaddr*)&setup->calling_subaddr);
  if (setup->v_dl_called_num)
  {
    setup_ind->called_party.ton          = setup->dl_called_num.ton;
    setup_ind->called_party.npi          = setup->dl_called_num.npi;
    setup_ind->called_party.c_called_num = setup->dl_called_num.c_num;
    memcpy (setup_ind->called_party.called_num,
            setup->dl_called_num.num, setup->dl_called_num.c_num);
  }
  else
  {
    setup_ind->called_party.ton          = MNCC_TON_NOT_PRES;
    setup_ind->called_party.npi          = MNCC_NPI_NOT_PRES;
    setup_ind->called_party.c_called_num = 0;
  }
/* Implements Measure#  25 */
  cc_fill_sub_struct (setup->v_called_subaddr,
                      &setup_ind->called_party_sub,
                      (T_M_CC_connect_subaddr *)&setup->called_subaddr);
  if (setup->v_redirecting_num)
  {
    setup_ind->redirecting_party.ton = setup->redirecting_num.ton;
    setup_ind->redirecting_party.npi = setup->redirecting_num.npi;
    setup_ind->redirecting_party.c_redir_num = setup->redirecting_num.c_num;
    memcpy (setup_ind->redirecting_party.redir_num, 
            setup->redirecting_num.num,
            setup->redirecting_num.c_num);
    if (setup->redirecting_num.v_screen)
      setup_ind->redirecting_party.screen = setup->redirecting_num.screen;
    else
      setup_ind->redirecting_party.screen = MNCC_SCREEN_IND_NOT_PRES;
    if (setup->redirecting_num.v_present)
      setup_ind->redirecting_party.present = setup->redirecting_num.present;
    else
      setup_ind->redirecting_party.present = MNCC_PRES_NOT_PRES;
  }
  else
  {
    setup_ind->redirecting_party.ton         = MNCC_TON_NOT_PRES;
    setup_ind->redirecting_party.npi         = MNCC_NPI_NOT_PRES;
    setup_ind->redirecting_party.present     = MNCC_PRES_NOT_PRES;
    setup_ind->redirecting_party.screen      = MNCC_SCREEN_IND_NOT_PRES;
    setup_ind->redirecting_party.c_redir_num = 0;
  }
/* Implements Measure#  26 */
  cc_fill_sub_struct (setup->v_redirecting_subaddr,
                      &setup_ind->redirecting_party_sub,
                      (T_M_CC_connect_subaddr *)&setup->redirecting_subaddr);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_modify            |
+--------------------------------------------------------------------+

  PURPOSE : Filling of the C-Structure for modify message.

*/

GLOBAL void cc_build_modify (const T_MNCC_MODIFY_REQ * modify,
                                   T_B_MODIFY        * modify_msg)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_build_modify()");

  modify_msg->msg_type = B_MODIFY;
  if (modify->serv EQ cc_data->neg_serv1)
  {
    modify_msg->bearer_cap = cc_data->neg_bc1;
    cc_data->new_itc = cc_data->neg_bc1.trans_cap;
    cc_data->old_itc = cc_data->neg_bc2.trans_cap;
  }
  else
  {
    modify_msg->bearer_cap = cc_data->neg_bc2;
    cc_data->new_itc = cc_data->neg_bc2.trans_cap;
    cc_data->old_itc = cc_data->neg_bc1.trans_cap;
  }

  if (modify_msg->bearer_cap.trans_cap EQ M_CC_ITC_DIGITAL_UNRESTRICTED)
  {
    cc_build_llc (&modify_msg->v_low_layer_comp, 
                  &modify_msg->low_layer_comp,
                  &modify_msg->bearer_cap);
  }
  else
    modify_msg->v_low_layer_comp  = FALSE;
  modify_msg->v_high_layer_comp = FALSE;

  if (cc_data->call_type [cc_data->index_ti] EQ CALL_TYPE_MTC)
    modify_msg->v_reverse_call = TRUE;
  else
    modify_msg->v_reverse_call = FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_modify_complete   |
+--------------------------------------------------------------------+

  PURPOSE : Filling of the C-Structure for modify complete message.

*/

GLOBAL void cc_build_modify_complete (T_B_MODIFY_COMP   * modify_com)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_build_modify_complete()");

  modify_com->msg_type = B_MODIFY_COMP;
  if (cc_data->active_service EQ cc_data->neg_serv1)
    modify_com->bearer_cap = cc_data->neg_bc1;
  else
    modify_com->bearer_cap = cc_data->neg_bc2;

  if (cc_data->call_type [cc_data->index_ti] EQ CALL_TYPE_MTC)
  {
    modify_com->v_reverse_call = TRUE;
    modify_com->v_low_layer_comp = FALSE;
  }
  else
  {
    modify_com->v_reverse_call = FALSE;
    if (modify_com->bearer_cap.trans_cap EQ M_CC_ITC_DIGITAL_UNRESTRICTED)
    {
      cc_build_llc (&modify_com->v_low_layer_comp, 
                    &modify_com->low_layer_comp,
                    &modify_com->bearer_cap);
    }
    else
      modify_com->v_low_layer_comp  = FALSE;
  }

  modify_com->v_high_layer_comp = FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_modify_reject     |
+--------------------------------------------------------------------+

  PURPOSE : Filling of the C-Structure for modify reject message.

*/

GLOBAL void cc_build_modify_reject (T_B_MODIFY_REJ * modify_rej,
                                    USHORT           cause)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_build_modify_reject()");

  modify_rej->msg_type = B_MODIFY_REJ;
  if (cc_data->active_service EQ cc_data->neg_serv1)
    modify_rej->bearer_cap = cc_data->neg_bc1;
  else
    modify_rej->bearer_cap = cc_data->neg_bc2;

  if (cc_data->call_type [cc_data->index_ti] EQ CALL_TYPE_MTC)
  {
    modify_rej->v_low_layer_comp = FALSE;
  }
  else
  {
    if (modify_rej->bearer_cap.trans_cap EQ M_CC_ITC_DIGITAL_UNRESTRICTED)
    {
      cc_build_llc (&modify_rej->v_low_layer_comp, 
                    &modify_rej->low_layer_comp,
                    &modify_rej->bearer_cap);
    }
    else
      modify_rej->v_low_layer_comp  = FALSE;
  }

  modify_rej->v_high_layer_comp = FALSE;

  /* no security check for validity of cause; cause is mandatory in MODIFY REJECT */
  cc_encode_cause (&modify_rej->cc_cause, cause);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_release           |
+--------------------------------------------------------------------+

  PURPOSE : Filling of the C-Structure for release message.

*/
GLOBAL void cc_build_release (T_U_RELEASE * release,
                              USHORT        cause,
                        const T_NAS_fac_inf   * fac_inf,
                              UBYTE         ss_ver)
{

  TRACE_FUNCTION ("cc_build_release()");

  release->msg_type = U_RELEASE;
  if (IS_CAUSE_INVALID(cause))
  {
    release->v_cc_cause = FALSE;
  }
  else
  {
    release->v_cc_cause = TRUE;
    cc_encode_cause (&release->cc_cause, cause);
  }

  release->v_cc_cause_2 = FALSE;
  release->v_facility   = FALSE;
  release->v_user_user  = FALSE;
  release->v_ss_version = FALSE;

  if ((fac_inf NEQ NULL) AND (fac_inf->l_fac NEQ 0))
  {
    /* Send facility IE */
    release->v_facility = TRUE;
    release->facility.c_fac = fac_inf->l_fac >> 3;
    memcpy (release->facility.fac, 
      &fac_inf->fac[fac_inf->o_fac >> 3],
      release->facility.c_fac);

    if (ss_ver NEQ MNCC_SS_VER_NOT_PRES)
    {
      release->v_ss_version = TRUE;
      release->ss_version.c_ver = 1;
      release->ss_version.ver[0] = ss_ver;
    }
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_release_complete  |
+--------------------------------------------------------------------+

  PURPOSE : Filling of the C-Structure for release complete message.

*/

GLOBAL void cc_build_release_complete (T_U_RELEASE_COMP * rel_com,
                                       USHORT             cause)
{

  TRACE_FUNCTION ("cc_build_release_complete()");

  rel_com->msg_type     = U_RELEASE_COMP;
  if (IS_CAUSE_INVALID(cause))
  {
    rel_com->v_cc_cause = FALSE;
  }
  else
  {
    rel_com->v_cc_cause = TRUE;
    cc_encode_cause (&rel_com->cc_cause, cause);
  }

  rel_com->v_facility   = FALSE;
  rel_com->v_user_user  = FALSE;
  rel_com->v_ss_version = FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_start_cc          |
+--------------------------------------------------------------------+

  PURPOSE : Filling of the C-Structure for a START CC message.

*/

GLOBAL void cc_build_start_cc (T_U_START_CC * start_cc)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_build_start_cc()");

  start_cc->msg_type        = U_START_CC;
  start_cc->v_call_ctrl_cap = (cc_data->call_ctrl_cap.dtmf NEQ 0) OR
                              (cc_data->call_ctrl_cap.pcp NEQ 0);
  if (start_cc->v_call_ctrl_cap)
  {
    start_cc->call_ctrl_cap   = cc_data->call_ctrl_cap; /* Struct copy */
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_fill_llc_hlc            |
+--------------------------------------------------------------------+

  PURPOSE : Filling of the C-Structure for LLC and HLC in the 
            setup message.

*/

GLOBAL void cc_fill_llc_hlc (T_U_SETUP * setup)
{

  TRACE_FUNCTION ("cc_fill_llc_hlc()");

  /*
   * add lower layer capability if 
   * at least one bearer cap indicates
   * information transfer capability = UDI
   */
  if ((setup->v_bearer_cap AND
       setup->bearer_cap.trans_cap EQ M_CC_ITC_DIGITAL_UNRESTRICTED) OR
      (setup->v_bearer_cap_2 AND
       setup->bearer_cap_2.trans_cap EQ M_CC_ITC_DIGITAL_UNRESTRICTED))
  {
    /*
     * copy repeat indicator if available
     */
    setup->v_repeat_2 = setup->v_repeat;
    setup->repeat_2   = setup->repeat;
    /*
     * Fill LLC if bearer cap is available
     */
    if (setup->v_bearer_cap)
      cc_build_llc (&setup->v_low_layer_comp, &setup->low_layer_comp,
                    &setup->bearer_cap);
    if (setup->v_bearer_cap_2)
      cc_build_llc (&setup->v_low_layer_comp_2, (T_M_CC_low_layer_comp *)&setup->low_layer_comp_2,
                    (T_M_CC_bearer_cap *)&setup->bearer_cap_2);
  }
  else
  {
    /*
     * default dont include LLC
     */
    setup->v_repeat_2            = FALSE;
    setup->v_low_layer_comp      = FALSE;
    setup->v_low_layer_comp_2    = FALSE;
  }
  setup->v_repeat_3            = FALSE;
  setup->v_high_layer_comp     = FALSE;
  setup->v_high_layer_comp_2   = FALSE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_setup             |
+--------------------------------------------------------------------+

  PURPOSE : Filling of the C-Structure for setup message.

*/

GLOBAL void cc_build_setup (      T_U_SETUP * setup_msg,
                            const T_MNCC_SETUP_REQ *setup_prm)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_build_setup()");

  TRACE_EVENT_P2 ("SERV2=%x RI=%x", cc_data->serv2, cc_data->ri);

  setup_msg->msg_type     = U_SETUP;
  setup_msg->v_bearer_cap = TRUE;
  setup_msg->bearer_cap   = cc_data->bc1;
  if (cc_data->serv2 NEQ NOT_PRESENT_8BIT)
  {
    setup_msg->v_bearer_cap_2      = TRUE;
    memcpy (&setup_msg->bearer_cap_2, 
            &cc_data->bc2,
            sizeof (T_M_CC_bearer_cap));
    setup_msg->v_repeat            = TRUE;
    setup_msg->repeat              = cc_data->ri;
  }
  else
  {
    setup_msg->v_repeat            = FALSE;
    setup_msg->v_bearer_cap_2      = FALSE;
  }

  cc_fill_llc_hlc (setup_msg);

  setup_msg->v_user_user           = FALSE;
  setup_msg->v_ss_version          = FALSE;
  setup_msg->v_fac_adv             = FALSE;

  if (cc_data->my_party_subaddr.c_subaddr NEQ 0)
  {
    setup_msg->v_calling_subaddr = TRUE;
    setup_msg->calling_subaddr   = cc_data->my_party_subaddr; /* Struct copy */
  }
  else
  {
    setup_msg->v_calling_subaddr = FALSE;
  }

  if (setup_prm->called_party.c_called_num NEQ 0)
  {
    setup_msg->v_ul_called_num = TRUE;
    setup_msg->ul_called_num.v_ton = TRUE;
    setup_msg->ul_called_num.ton   = setup_prm->called_party.ton;
    setup_msg->ul_called_num.v_npi = TRUE;
    setup_msg->ul_called_num.npi   = setup_prm->called_party.npi;
    setup_msg->ul_called_num.c_num = setup_prm->called_party.c_called_num;
    memcpy (&setup_msg->ul_called_num.num[0], 
            &setup_prm->called_party.called_num[0],
            setup_prm->called_party.c_called_num);
  }
  else
  {
    setup_msg->v_ul_called_num = FALSE;
  }

  if (setup_prm->called_party_sub.c_subaddr NEQ 0)
  {
    setup_msg->v_called_subaddr = TRUE;
    setup_msg->called_subaddr.v_tos      = TRUE;
    setup_msg->called_subaddr.tos        = setup_prm->called_party_sub.tos;
    setup_msg->called_subaddr.v_odd_even = TRUE;
    setup_msg->called_subaddr.odd_even   = setup_prm->called_party_sub.odd_even;
    setup_msg->called_subaddr.c_subaddr  = setup_prm->called_party_sub.c_subaddr;
    memcpy (&setup_msg->called_subaddr.subaddr[0], 
            &setup_prm->called_party_sub.subaddr[0], 
            setup_prm->called_party_sub.c_subaddr);
  }
  else
  {
    setup_msg->v_called_subaddr = FALSE;
  }

  switch (setup_prm->clir_sup)
  {
    case MNCC_CLR_SUP:
      setup_msg->v_clir_suppr = TRUE;
      setup_msg->v_clir_invoc = FALSE;
      break;

    case MNCC_CLR_SUP_NOT:
      setup_msg->v_clir_suppr = FALSE;
      setup_msg->v_clir_invoc = TRUE;
      break;

    default: /* CLR_NOT_PRES */
      setup_msg->v_clir_suppr = FALSE;
      setup_msg->v_clir_invoc = FALSE;
      break;
  }

  setup_msg->v_call_ctrl_cap = (cc_data->call_ctrl_cap.dtmf NEQ 0) OR
                               (cc_data->call_ctrl_cap.pcp NEQ 0);
  if (setup_msg->v_call_ctrl_cap)
  {
    setup_msg->call_ctrl_cap   = cc_data->call_ctrl_cap; /* Struct copy */
  }

  if (setup_msg->bearer_cap.conn_elem EQ MNCC_CONN_ELEM_TRANS)
  {
    /*
     * set parameters invalid according to GSM 07.01 V7.1.1 Rel 1998 - 
     * CC-ENH-2378 
     */
    setup_msg->bearer_cap.v_uimi  = FALSE;
    setup_msg->bearer_cap.v_waiur = FALSE;
  }

  if (setup_prm->fac_inf.l_fac EQ 0)
  {
    setup_msg->v_facility  = FALSE;
  }
  else
  {
    setup_msg->v_facility  = TRUE;
    setup_msg->facility.c_fac = setup_prm->fac_inf.l_fac >> 3;
    memcpy (setup_msg->facility.fac, 
            &setup_prm->fac_inf.fac[setup_prm->fac_inf.o_fac>>3],
            setup_msg->facility.c_fac);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_start_dtmf        |
+--------------------------------------------------------------------+

  PURPOSE : Filling of the C-Structure for start dtmf message.

*/

GLOBAL void cc_build_start_dtmf (UBYTE            key,
                                 T_U_START_DTMF * start_dtmf)
{
  TRACE_FUNCTION ("cc_build_start_dtmf()");

  start_dtmf->msg_type         = U_START_DTMF;
  start_dtmf->v_key_facility   = TRUE;
  start_dtmf->key_facility.key = key;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_alert             |
+--------------------------------------------------------------------+

  PURPOSE : Filling of the C-Structure for alert message.

*/

GLOBAL void cc_build_alert (T_U_ALERT * alert_msg)
{
  TRACE_FUNCTION ("cc_build_alert()");

  alert_msg->msg_type     = U_ALERT;
  alert_msg->v_facility   = FALSE;
  alert_msg->v_user_user  = FALSE;
  alert_msg->v_ss_version = FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_connect           |
+--------------------------------------------------------------------+

  PURPOSE : Filling of the C-Structure for connect message.

*/

GLOBAL void cc_build_connect (T_U_CONNECT * connect_msg)
{
  TRACE_FUNCTION ("cc_build_connect()");

  connect_msg->msg_type           = U_CONNECT;
  connect_msg->v_facility         = FALSE;
  connect_msg->v_connect_subaddr  = FALSE;
  connect_msg->v_user_user        = FALSE;
  connect_msg->v_ss_version       = FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_status            |
+--------------------------------------------------------------------+

  PURPOSE : Filling of the C-Structure for status message.

*/

GLOBAL void cc_build_status (T_B_STATUS * status,
                             USHORT       cause)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_build_status()");

  status->msg_type         = B_STATUS;

  /* no security check for validity of cause; cause is mandatory in STATUS */
  cc_encode_cause (&status->cc_cause, cause);

  status->call_state.cs    = M_CC_CS_GSM_PLMN;                /* vk 04-jul-97 */
  status->call_state.state = cc_data->state[cc_data->index_ti];

  switch (status->call_state.state)
  {
    case M_CC_CS_10:
    case M_CC_CS_26:
      if (cc_data->hold_state[cc_data->index_ti] NEQ M_CC_HLD_IDLE OR 
          cc_data->mpty_state[cc_data->index_ti] NEQ M_CC_MPTY_IDLE)
      {

        TRACE_EVENT_P3 ("  ti=%d, hold=%d, mpty=%d", 
                        cc_data->ti,
                        cc_data->hold_state[cc_data->index_ti],
                        cc_data->mpty_state[cc_data->index_ti]);

        status->v_aux_states      = TRUE;
        status->aux_states.v_hold = TRUE;
        status->aux_states.hold   = cc_data->hold_state[cc_data->index_ti];
        status->aux_states.v_mpty = TRUE;
        status->aux_states.mpty   = cc_data->mpty_state[cc_data->index_ti];
      }
      else
      {
        status->v_aux_states = FALSE;
      }
      break;

    default:
      status->v_aux_states = FALSE;
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_user_ind_from_cong|
+--------------------------------------------------------------------+

  PURPOSE : Build primitive MNCC_USER_IND from congestion control
            message.

*/

GLOBAL void cc_build_user_ind_from_cong (T_MNCC_USER_IND  * user,
                                   const T_B_CONGEST_CTRL * cong)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_build_user_ind_from_cong()");

  user->ti          = cc_data->ti;
  user->more_data   = NOT_PRESENT_8BIT;
  user->congest_lev = cong->congest_lev;
  memset (&user->user, 0, sizeof (T_MNCC_user));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_user_ind_from_user|
+--------------------------------------------------------------------+

  PURPOSE : Build primitive MNCC_USER_IND from user information
            message.

*/

GLOBAL void cc_build_user_ind_from_user (T_MNCC_USER_IND * user_ind,
                                   const T_B_USER_INFO   * user)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_build_user_ind_from_user()");

  user_ind->ti          = cc_data->ti;
  user_ind->user.info_context = MNCC_USER_IN_USER;
  user_ind->user.pd     = user->user_user.pd;
  user_ind->user.c_info = user->user_user.c_info;
  memcpy (user_ind->user.info, user->user_user.info, MNCC_USER_LEN);
  user_ind->congest_lev = NOT_PRESENT_8BIT;
  user_ind->more_data   = user->v_more_data;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_user_information  |
+--------------------------------------------------------------------+

  PURPOSE : Filling the C-structure for user information message.

*/

GLOBAL void cc_build_user_information (T_MNCC_USER_REQ * user,
                                       T_B_USER_INFO   * user_msg)
{
  TRACE_FUNCTION ("cc_build_user_information()");

  user_msg->msg_type          = B_USER_INFO;
  user_msg->user_user.pd      = user->user.pd;
  user_msg->user_user.c_info  = user->user.c_info;
  memcpy (user_msg->user_user.info, user->user.info, MNCC_USER_LEN);
  user_msg->v_more_data       = user->more_data;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_check_error_flag        |
+--------------------------------------------------------------------+

  PURPOSE : Checks the error variable of CC data and sends status
            message if necessary.

*/

GLOBAL BOOL cc_check_error_flag (void)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("cc_check_error_flag()");

  switch (cc_data->error)
  {
    case M_CC_CAUSE_INVALID_MAND_INFO:
    case M_CC_CAUSE_COND_INFO_ELEM:
      CCD_END;
/* Implements Measure#  7 and streamline encoding*/
      cc_send_status (cc_data->error);  
      return FALSE;

    default:
      return TRUE;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_check_ms_cap_repeat_indicator_support  |
+--------------------------------------------------------------------+

  PURPOSE : The mobile station repeat indicator capabilities are checked
            against the request.

  RETURN:   OKAY if capabilities supported.
            ERROR if capabilitis not supported.

*/
U8 cc_check_ms_cap_repeat_indicator_support(const U8 repeat)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_check_ms_cap_repeat_indicator_support()");

  cc_csf_ms_cap ();

  switch (repeat)
  {
    case M_CC_REPEAT_CIRCULAR:
      if (FldGet (cc_data->mscap.datCap2, NAltSrvSup) EQ TRUE) /* still needed for TS61 - not for BS61 */
      {
        TRACE_EVENT("ERROR - alternate services not allowed by manufacturer");
        return ERROR; /* No alternate services allowed by manufacturer */
      }

      if(
            ((cc_data->neg_bcpara1.bearer_serv EQ MNCC_BEARER_SERV_FAX)    AND
             (cc_data->neg_bcpara2.bearer_serv EQ MNCC_BEARER_SERV_SPEECH)     )
          OR
            ((cc_data->neg_bcpara2.bearer_serv EQ MNCC_BEARER_SERV_FAX)    AND
             (cc_data->neg_bcpara1.bearer_serv EQ MNCC_BEARER_SERV_SPEECH)     )
        )
        return OKAY;

      TRACE_EVENT("ERROR - no TS61 found - wrong repeat indicator - BS61 not supported at all");
      return ERROR;

    case M_CC_REPEAT_FALLBACK:
        return OKAY; /* MS cap checked earlier */

    default:
      return ERROR;
  } /* switch (repeat) */

}/* cc_check_ms_cap_repeat_indicator_support */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_compatibility_check     |
+--------------------------------------------------------------------+

  PURPOSE : Checks the compatibility of incoming setup message to
            its own configuration.

*/

GLOBAL UBYTE cc_compatibility_check (const T_D_SETUP * setup)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_compatibility_check()");

  cc_csf_ms_cap ();

  cc_data->negotiation = FALSE;

  cc_data->neg_serv1   = NOT_PRESENT_8BIT;
  memset (&cc_data->neg_bcpara1, 0, sizeof (T_MNCC_bcpara));
  cc_data->neg_bcpara1.bearer_serv = NOT_PRESENT_8BIT;

  cc_data->neg_serv2   = NOT_PRESENT_8BIT;
  memset (&cc_data->neg_bcpara2, 0, sizeof (T_MNCC_bcpara));
  cc_data->neg_bcpara2.bearer_serv = NOT_PRESENT_8BIT;
  cc_data->neg_ri = NOT_PRESENT_8BIT;

  if (setup->v_bearer_cap)
  {
    if (cc_check_bc (&setup->bearer_cap,
                     &cc_data->neg_bc1,
                     &cc_data->neg_bcpara1,
                     &cc_data->neg_serv1,
                     &cc_data->negotiation) EQ ERROR)
    {
#if 0 /*  HM 03-09-2002, code to merge is superflous, I don't take it
          The part of the code between #if 0 to #endif is never executed.*/
      /*
       * set parameters invalid according to GSM 07.01 V7.1.1 Rel 1998 - 
       * CC-ENH-2378
       */
      if ((setup->bearer_cap.v_modem_type_2 EQ FALSE) OR
          (setup->bearer_cap.v_fnur EQ FALSE))
      {
        cc_data->neg_bc1.v_modem_type_2 = FALSE;
        cc_data->neg_bc1.v_fnur         = FALSE;
        cc_data->neg_bc1.v_acc          = FALSE;
        cc_data->neg_bc1.v_mTch         = FALSE;
        cc_data->neg_bc1.v_uimi         = FALSE;
        cc_data->neg_bc1.v_waiur        = FALSE;
      }
#endif 
      return ERROR;
    }
  }
  else
  {
    /*
     * The incoming setup message contains no bearer capabilities
     *
     * This indicates single numbering scheme
     *
     * use the parameter of sns_mode to set the negotiated answer
     */
    cc_data->negotiation = TRUE;
    memset (&cc_data->neg_bcpara1, 0, sizeof (T_MNCC_bcpara));
    memset (&cc_data->neg_bcpara2, 0, sizeof (T_MNCC_bcpara));

    switch (cc_data->sns_mode)
    {
      case MNCC_SNS_MODE_VOICE: /* VOICE calls */
        if (!cc_voice_capability ())
          return ERROR;
        memcpy (&cc_data->neg_bcpara1, &cc_data->sns_bcpara,
                sizeof (T_MNCC_bcpara));
        cc_data->neg_bcpara2.bearer_serv = NOT_PRESENT_8BIT;
        cc_data->neg_ri    = NOT_PRESENT_8BIT;
        cc_data->neg_serv1 = MNCC_SERV_SPEECH;
        cc_data->neg_serv2 = NOT_PRESENT_8BIT;
        cc_build_bc (&cc_data->neg_bc1, &cc_data->neg_serv1,
                     &cc_data->neg_bcpara1);
        break;
      case MNCC_SNS_MODE_VAF_VOICE: /* Voice Alternating Fax, VOICE first  */
      case MNCC_SNS_MODE_VAD_VOICE: /* Voice Alternating Data, VOICE first */
        if (!cc_voice_capability ())
          return ERROR;
        memset (&cc_data->neg_bcpara1, 0, sizeof (T_MNCC_bcpara));
        cc_data->neg_bcpara1.bearer_serv = (cc_data->ctm_ena EQ MNCC_CTM_ENABLED)?
                                           MNCC_BEARER_SERV_SPEECH_CTM: MNCC_BEARER_SERV_SPEECH;
        memcpy (&cc_data->neg_bcpara2, &cc_data->sns_bcpara,
                sizeof (T_MNCC_bcpara));
        cc_data->neg_ri    = MNCC_RI_CIRCULAR;
        cc_data->neg_serv1 = MNCC_SERV_SPEECH;
        cc_data->neg_serv2 = MNCC_SERV_DATA;
        cc_build_bc (&cc_data->neg_bc1, &cc_data->neg_serv1,
                     &cc_data->neg_bcpara1);
        cc_build_bc (&cc_data->neg_bc2, &cc_data->neg_serv2,
                     &cc_data->neg_bcpara2);
        break;
      case MNCC_SNS_MODE_FAX:  /* FAX calls  */
      case MNCC_SNS_MODE_DATA: /* DATA calls */
        memcpy (&cc_data->neg_bcpara1, &cc_data->sns_bcpara,
                sizeof (T_MNCC_bcpara));
        cc_data->neg_bcpara2.bearer_serv = NOT_PRESENT_8BIT;
        cc_data->neg_ri    = NOT_PRESENT_8BIT;
        cc_data->neg_serv1 = MNCC_SERV_DATA;
        cc_data->neg_serv2 = NOT_PRESENT_8BIT;
        cc_build_bc (&cc_data->neg_bc1, &cc_data->neg_serv1,
                     &cc_data->neg_bcpara1);
        break;
      case MNCC_SNS_MODE_VAF_FAX:  /* Voice Alternating Fax, FAX first   */ 
      case MNCC_SNS_MODE_VAD_DATA: /* Voice Alternating Data, DATA first */
        if (!cc_voice_capability ())
          return ERROR;
        memset (&cc_data->neg_bcpara2, 0, sizeof (T_MNCC_bcpara));
        cc_data->neg_bcpara2.bearer_serv = (cc_data->ctm_ena EQ MNCC_CTM_ENABLED)?
                                           MNCC_BEARER_SERV_SPEECH_CTM: MNCC_BEARER_SERV_SPEECH;
        memcpy (&cc_data->neg_bcpara1, &cc_data->sns_bcpara,
                sizeof (T_MNCC_bcpara));
        cc_data->neg_ri    = MNCC_RI_CIRCULAR;
        cc_data->neg_serv1 = MNCC_SERV_DATA;
        cc_data->neg_serv2 = MNCC_SERV_SPEECH;
        cc_build_bc (&cc_data->neg_bc1, &cc_data->neg_serv1,
                     &cc_data->neg_bcpara1);
        break;
      case MNCC_SNS_MODE_VFD: /* Voice Followed by Data */
        if (!cc_voice_capability ())
          return ERROR;
        memset (&cc_data->neg_bcpara1, 0, sizeof (T_MNCC_bcpara));
        cc_data->neg_bcpara1.bearer_serv = (cc_data->ctm_ena EQ MNCC_CTM_ENABLED)?
                                           MNCC_BEARER_SERV_SPEECH_CTM: MNCC_BEARER_SERV_SPEECH;
        memcpy (&cc_data->neg_bcpara2, &cc_data->sns_bcpara,
                sizeof (T_MNCC_bcpara));
        cc_data->neg_ri    = MNCC_RI_SEQUENTIAL;
        cc_data->neg_serv1 = MNCC_SERV_SPEECH;
        cc_data->neg_serv2 = MNCC_SERV_DATA;
        cc_build_bc (&cc_data->neg_bc1, &cc_data->neg_serv1,
                     &cc_data->neg_bcpara1);
        cc_build_bc (&cc_data->neg_bc2, &cc_data->neg_serv2,
                     &cc_data->neg_bcpara2);
        break;
      default:
        TRACE_ERROR ("Unexpected default"); /* All cases caught */
        break;
    }
  }
  if (setup->v_bearer_cap_2)
  {
    if (cc_check_bc ((T_M_CC_bearer_cap *)&setup->bearer_cap_2,
                     &cc_data->neg_bc2,
                     &cc_data->neg_bcpara2,
                     &cc_data->neg_serv2, 
                     &cc_data->negotiation) EQ ERROR)
      return ERROR;
  }

  /*Check for repeat Indicator*/
  if (setup->v_repeat)
  {
    if (cc_check_ms_cap_repeat_indicator_support(setup->repeat) EQ ERROR)
      return ERROR;

    cc_data->neg_ri = setup->repeat;
  }


    /*
     * Two bearer capabilities are available. The 
     * single numbering scheme command on ACI level can 
     * be used to swap bearer capabilities. For example
     * SNS Mode is set to voice alternating fax, voice first
     * and the network indicates voice alternating fax, fax first.
     * Then the BCs are swapped.
     */
  if (setup->v_bearer_cap_2)
  {
    UBYTE swap = FALSE;
    switch (cc_data->sns_mode)
    {
      case MNCC_SNS_MODE_VAF_VOICE:
        if (cc_data->neg_bcpara1.bearer_serv EQ MNCC_BEARER_SERV_FAX AND
            (cc_data->neg_serv2 EQ MNCC_SERV_SPEECH) AND
            cc_data->neg_ri EQ MNCC_RI_CIRCULAR)
          swap = TRUE;
        break;
      case MNCC_SNS_MODE_VAD_VOICE:
        if (cc_data->neg_bcpara1.bearer_serv NEQ MNCC_BEARER_SERV_FAX AND
            (cc_data->neg_serv1 NEQ MNCC_SERV_SPEECH) AND
            (cc_data->neg_serv2 EQ MNCC_SERV_SPEECH) AND
            cc_data->neg_ri EQ MNCC_RI_CIRCULAR)
          swap = TRUE;
        break;
      case MNCC_SNS_MODE_VAF_FAX:
        if (cc_data->neg_bcpara2.bearer_serv EQ MNCC_BEARER_SERV_FAX AND
            (cc_data->neg_serv1 EQ MNCC_SERV_SPEECH) AND
            cc_data->neg_ri EQ MNCC_RI_CIRCULAR)
          swap = TRUE;
        break;
      case MNCC_SNS_MODE_VAD_DATA:
        if (cc_data->neg_bcpara2.bearer_serv NEQ MNCC_BEARER_SERV_FAX AND
            (cc_data->neg_serv2 NEQ MNCC_SERV_SPEECH) AND
            (cc_data->neg_serv1 EQ MNCC_SERV_SPEECH) AND
            cc_data->neg_ri EQ MNCC_RI_CIRCULAR)
          swap = TRUE;
        break;
    }
    if (swap)
    {
      /*
       * Both bearer capabilities shall be swapped
       */
      T_MNCC_bcpara      temp_bcpara;
      UBYTE         temp_serv;
      T_M_CC_bearer_cap  temp_bc; /*lint !e813 length info of struct*/

      memcpy (&temp_bcpara, &cc_data->neg_bcpara1, sizeof (T_MNCC_bcpara));
      memcpy (&cc_data->neg_bcpara1, &cc_data->neg_bcpara2, sizeof (T_MNCC_bcpara));
      memcpy (&cc_data->neg_bcpara2, &temp_bcpara, sizeof (T_MNCC_bcpara)); 
      
      temp_serv = cc_data->neg_serv1;
      cc_data->neg_serv1 = cc_data->neg_serv2;
      cc_data->neg_serv2 = temp_serv;

      memcpy (&temp_bc, &cc_data->neg_bc1, sizeof (T_M_CC_bearer_cap));
      memcpy (&cc_data->neg_bc1, &cc_data->neg_bc2, sizeof (T_M_CC_bearer_cap));
      memcpy (&cc_data->neg_bc2, &temp_bc, sizeof (T_M_CC_bearer_cap)); 

      cc_data->negotiation = TRUE;
    }
  }

  /*
   * set parameters invalid according to GSM 07.01 V7.1.1 Rel 1998 - 
   * CC-ENH-2378
   */
  if ((setup->bearer_cap.v_modem_type_2 EQ FALSE) OR
      (setup->bearer_cap.v_fnur EQ FALSE))
  {
    cc_data->neg_bc1.v_modem_type_2 = FALSE;
    cc_data->neg_bc1.v_fnur         = FALSE;
    cc_data->neg_bc1.v_acc          = FALSE;
    cc_data->neg_bc1.v_mTch         = FALSE;
    cc_data->neg_bc1.v_uimi         = FALSE;
    cc_data->neg_bc1.v_waiur        = FALSE;
  }

  cc_data->active_service = cc_data->neg_serv1;

  if (cc_mtc_check_subaddr (setup) EQ BAD_SUBADDRESS)
    return BAD_SUBADDRESS;

  if (cc_data->negotiation)
    return NEGOTIATION;
  else
    return OKAY;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_basic_service_align()   |
+--------------------------------------------------------------------+

  PURPOSE:    Checks the compatibility of incoming uplink setup message 
              to its own configuration and performs basic service group
              alignment. This is used for Network Initiated MO Call (CCBS).
              See GSM 04.08 subclause 5.2.3.2.1 "Basic service group 
              alignment".
                
              The following fields in cc_data are changed by a call 
              of this function:
              
              cc_data->serv1;
              cc_data->serv2;
              cc_data->ri;
              cc_data->bc1;
              cc_data->bc2;
              cc_data->bcpara1;
              cc_data->bcpara2;
              cc_data->neg_serv1;
              cc_data->neg_serv2;
              cc_data->neg_ri;
              cc_data->neg_bc1;
              cc_data->neg_bc2;
              cc_data->neg_bcpara1;
              cc_data->neg_bcpara2;
              cc_data->negotiation;
            
  PARAMETERS: cc_data: IN/OUT
              setup:   IN/OUT

  RETURN:     OKAY if setup (bearer caps) is compatible with mobile,
              ERROR otherwise.
  
*/

GLOBAL UBYTE cc_basic_service_align (T_U_SETUP * setup)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_basic_service_align()");

  cc_csf_ms_cap ();

  if (FldGet (cc_data->mscap.datCap2, NAltSrvSup) AND setup->v_repeat)
    return ERROR; /* No alternate services allowed by manufacturer */

  cc_data->ri     = (setup->v_repeat) ? setup->repeat : NOT_PRESENT_8BIT;
  cc_data->neg_ri = cc_data->ri;

  /* BC is mandatory IE in uplink setup message */
  if (!setup->v_bearer_cap)
    return ERROR;
  if (cc_check_bc (&setup->bearer_cap,
                   &cc_data->bc1,          /* Not used */
                   &cc_data->bcpara1,
                   &cc_data->serv1,        /* Not used */
                   &cc_data->negotiation)  /* Not used */
		  EQ ERROR)
    return ERROR; /* Not expected to catch */

  if (!cc_check_capabilities (&cc_data->bcpara1))
    return ERROR; /* Not expected to catch */

  cc_build_bc (&cc_data->bc1, &cc_data->serv1, &cc_data->bcpara1);
  setup->bearer_cap = cc_data->bc1;         /* Struct copy */
  cc_data->neg_serv1   = cc_data->serv1;
  cc_data->neg_bc1     = cc_data->bc1;      /* Struct copy */
  cc_data->neg_bcpara1 = cc_data->bcpara1;  /* Struct copy */

  /* BC2 is optional IE in uplink setup message */
  if (setup->v_bearer_cap_2) 
  {
    if (cc_check_bc ((T_M_CC_bearer_cap *)&setup->bearer_cap_2,
                     &cc_data->bc2,         /* Not used */
                     &cc_data->bcpara2,
                     &cc_data->serv2,       /* Not used */
                     &cc_data->negotiation) 
		    EQ ERROR)
      return ERROR; /* Bearer caps 2 incompatible */
    if (!cc_check_capabilities (&cc_data->bcpara2))
      return ERROR; /* Not expected to catch */

    cc_build_bc (&cc_data->bc2, &cc_data->serv2, &cc_data->bcpara2);
    setup->bearer_cap_2 = *(T_M_CC_bearer_cap_2*)&cc_data->bc2; /* Struct copy */
    cc_data->neg_bc2     = cc_data->bc2;      /* Struct copy */
  }
  else
  {
    cc_data->serv2 = NOT_PRESENT_8BIT;
    cc_data->bcpara2.bearer_serv = NOT_PRESENT_8BIT;
  }
  cc_data->neg_serv2 = cc_data->serv2;
  cc_data->neg_bcpara2 = cc_data->bcpara2;    /* Struct copy */

  /* check repeat indicator */
  if (setup->v_repeat EQ TRUE)
  {
    if (cc_check_ms_cap_repeat_indicator_support(setup->repeat) EQ ERROR)
      return ERROR;

    cc_data->ri = setup->repeat;
  }
  else
    cc_data->ri = NOT_PRESENT_8BIT;

  cc_data->neg_ri = cc_data->ri;

  cc_fill_llc_hlc (setup);
  
  return OKAY;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_count_active_connections|
+--------------------------------------------------------------------+

  PURPOSE : Counts the number of active connections.

*/

GLOBAL UBYTE cc_count_active_connections (void)
{
  GET_INSTANCE_DATA;
  UBYTE i;
  UBYTE x;

  TRACE_FUNCTION ("cc_count_active_connections()");

  for (i=0,x=0 ; i< MAX_CC_CALLS; i++)
    if (cc_data->state[i] NEQ M_CC_CS_0)
      x++;

  return x;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_disconnect_after_timeout|
+--------------------------------------------------------------------+

  PURPOSE : Start disconnection after timeout. A disconnect message
            is send to the infrastructure. MMI is informed and timer
            T305 is started.

*/

GLOBAL void cc_disconnect_after_timeout (void)
{
  GET_INSTANCE_DATA;
  PALLOC (setup_cnf, MNCC_SETUP_CNF);

  TRACE_FUNCTION ("cc_disconnect_after_timeout()");

  CCD_START;
  {
    MCAST (disconnect, U_DISCONNECT);
    cc_build_disconnect (disconnect, CAUSE_MAKE(DEFBY_STD, 
                                                ORIGSIDE_MS, 
                                                MNCC_CC_ORIGINATING_ENTITY,
                                                M_CC_CAUSE_TIMER),
                         NULL, MNCC_SS_VER_NOT_PRES);
    for_disconnect (disconnect);
  }
  CCD_END;

  memset (setup_cnf, 0, sizeof (T_MNCC_SETUP_CNF));
  setup_cnf->ti  = cc_data->ti;
  setup_cnf->cause = CAUSE_MAKE(DEFBY_STD, 
                                ORIGSIDE_MS, 
                                MNCC_CC_ORIGINATING_ENTITY,
                                M_CC_CAUSE_TIMER);
  PSENDX (MMI, setup_cnf);

  TIMERSTART (T305, T305_VALUE);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_encode_cause            |
+--------------------------------------------------------------------+

  PURPOSE : Coding of the cc_cause element and storing of the cause
            value in cc data. Dependent on the cause the diagnostic
            field is filled. It is expected that checking of validity
            is done outside this function.

*/

static const UBYTE timername[] = { '3', '0', '3', 0,
                                   '3', '0', '5', 0,
                                   '3', '0', '8', 0,
                                   '3', '1', '0', 0,
                                   '3', '1', '3', 0,
                                   '3', '2', '3', 0,
                                   '3', '3', '2', 0,
                                   '3', '3', '5', 0,
                                   '3', '3', '6', 0,
                                   '3', '3', '7', 0 };     

GLOBAL void cc_encode_cause (T_M_CC_cc_cause * cc_cause,
                             USHORT       cause)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_encode_cause()");

  /* perform "security" checks of application of cause concept */
  switch (GET_CAUSE_DEFBY(cause))
  {
    case DEFBY_STD:
      {
        switch (GET_CAUSE_ORIGIN_ENTITY(cause))
        {
          case MNCC_CC_ORIGINATING_ENTITY:
          case MNCC_ACI_ORIGINATING_ENTITY:
            {
              switch  (GET_CAUSE_ORIGSIDE(cause))
              {
                case ORIGSIDE_NET:
                  TRACE_EVENT("Warning: NET originated cause");
                  /* no error correction */
                  break;
                default: /* ORIGSIDE_MS */
                  break; /* this is the expected case */
              }
              break; /* MNCC_ACI_ORIGINATING_ENTITY, ACI_ORIGINATING_ENTITY */
            }
          default:
            TRACE_ERROR("Non ACI or CC cause originator");
            /* no error correction */
            break;
        }
      }
      break;
    default: /* DEFBY_CONDAT */
      TRACE_ERROR("Illegal use of cause: DEFBY_CONDAT");
      /* overwrite the cause with a standard cause */
      cause = CAUSE_MAKE(DEFBY_STD, ORIGSIDE_MS, MNCC_ACI_ORIGINATING_ENTITY, M_CC_CAUSE_UNSPECIFIED);
      break;
  }

  cc_cause->v_cs    = TRUE;
  cc_cause->cs      = M_CC_CS_GSM_PLMN;
  cc_cause->v_loc   = TRUE;
  cc_cause->loc     = M_CC_LOC_USER;
  cc_cause->v_rec   = FALSE;
  cc_cause->v_cause = TRUE;
  cc_cause->cause   = GET_CAUSE_VALUE(cause);

  /* 
   * store the cause for later usage (e.g. timeout)
   * store the entire 16 bits
   */
  if (cc_cause->cause NEQ M_CC_CAUSE_STATUS_ENQUIRY)
    cc_data->cc_cause [cc_data->index_ti] = cause;

  /* feed diagnostic information if necessary */
  switch (cc_cause->cause)
  {
    case M_CC_CAUSE_MESSAGE_TYPE_INCOMPAT:
    case M_CC_CAUSE_MESSAGE_TYPE_NOT_IMPLEM:
    case M_CC_CAUSE_MESSAGE_INCOMPAT:
      cc_cause->c_diag  = 1;
      cc_cause->diag[0] = cc_data->mt;
      break;

    case M_CC_CAUSE_INVALID_MAND_INFO:
    case M_CC_CAUSE_COND_INFO_ELEM:
      cc_cause->c_diag  = cc_data->error_count;
      memcpy (cc_cause->diag, cc_data->error_inf, cc_cause->c_diag);
      break;

    case M_CC_CAUSE_TIMER:
      cc_cause->c_diag  = 3;
      memcpy (cc_cause->diag,
              &timername[cc_data->timer[cc_data->index_ti]<<2], 3);
      break;

    default:
      cc_cause->c_diag  = 0;
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_mtc_check_subaddr       |
+--------------------------------------------------------------------+

  PURPOSE : Check the subaddress if available for the
            mobile terminated call.

*/

GLOBAL UBYTE cc_mtc_check_subaddr (const T_D_SETUP  * setup)
{
  GET_INSTANCE_DATA;
  UBYTE i;

  TRACE_FUNCTION ("cc_mtc_check_subaddr()");

  if (cc_data->my_party_subaddr.c_subaddr AND setup->v_called_subaddr)
  {
    if (cc_data->my_party_subaddr.tos NEQ setup->called_subaddr.tos)
      return BAD_SUBADDRESS;

    if (cc_data->my_party_subaddr.c_subaddr NEQ
        setup->called_subaddr.c_subaddr)
      return BAD_SUBADDRESS;

    for (i=0; i<cc_data->my_party_subaddr.c_subaddr; i++)
      if (cc_data->my_party_subaddr.subaddr[i] NEQ
          setup->called_subaddr.subaddr[i])
        return BAD_SUBADDRESS;
  }

  return OKAY;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_bcpara            |
+--------------------------------------------------------------------+

  PURPOSE : Build the local transmission parameters (for MMI)(bcpara)
            of the incoming bearer caps to the mobile station configuration
            for the Mobile Originated Call (after reception of a call
            proceeding message).

*/

LOCAL void cc_build_bcpara(      T_MNCC_bcpara *bcpara,
                           const T_M_CC_bearer_cap *bearer_cap)
{
  TRACE_FUNCTION ("cc_build_bcpara()");

  memset (bcpara, 0, sizeof (T_MNCC_bcpara));
  
  switch (bearer_cap->trans_cap)
  {
    case M_CC_ITC_SPEECH:
      /*
       * Speech bearer capability
       */
      {
        bcpara->bearer_serv = MNCC_BEARER_SERV_SPEECH;

        break;
      }
    case M_CC_ITC_AUXILIARY_SPEECH:
      /*
       * Speech bearer capability for alternate line service
       */
      {
        bcpara->bearer_serv = MNCC_BEARER_SERV_AUX_SPEECH;

        break;
      }
    case M_CC_ITC_AUDIO:
    case M_CC_ITC_DIGITAL_UNRESTRICTED:
      {
        bcpara->bearer_serv  = MNCC_BEARER_SERV_ASYNC;
        bcpara->stop_bits    = bearer_cap->num_stop;
        bcpara->data_bits    = bearer_cap->num_data;
        bcpara->parity       = bearer_cap->parity;
        bcpara->modem_type   = bearer_cap->modem_type;
        if (bearer_cap->v_fnur AND (bearer_cap->fnur EQ M_CC_FNUR_14400))
        {
          bcpara->rate       = MNCC_UR_14_4_KBIT;
        }
        else
        {
          bcpara->rate       = bearer_cap->user_rate;
        }
        switch (bearer_cap->conn_elem)
        {
        case MNCC_CONN_ELEM_TRANS:
          {
            bcpara->conn_elem    = MNCC_CONN_ELEM_TRANS;
            bcpara->flow_control = NOT_PRESENT_8BIT;

            break;
          }
        case MNCC_CONN_ELEM_NON_TRANS:
          {
            bcpara->conn_elem    = MNCC_CONN_ELEM_NON_TRANS;
            if (bearer_cap->v_user_inf_l2_prot)
            {
              switch (bearer_cap->user_inf_l2_prot)
              {
              case M_CC_L2_ISO6429:
                {
                  bcpara->flow_control = MNCC_INBAND_FLOW_CONTROL;

                  break;
                }
              case M_CC_L2_COPFC:
                {
                  bcpara->flow_control = MNCC_NO_FLOW_CONTROL;

                  break;
                }
              default:
                {
                  bcpara->flow_control = MNCC_OUTBAND_FLOW_CONTROL;

                  break;
                }
              }
            }
            else
            {
              bcpara->flow_control = MNCC_OUTBAND_FLOW_CONTROL;
            }
            break;
          }
        case MNCC_CONN_ELEM_TRANS_PREF:
        case MNCC_CONN_ELEM_NON_TRANS_PREF:
        case MNCC_CONN_ELEM_NOT_PRES:
          {
            TRACE_EVENT ("Call setup failed due to wrong conn_elem in bearer caps");

            break;
          }
        }
        break;
      }
    case M_CC_ITC_FAX_GROUP_3:
      {
        if (bearer_cap->v_fnur AND (bearer_cap->fnur EQ M_CC_FNUR_14400))
        {
          bcpara->rate       = MNCC_UR_14_4_KBIT;
        }
        else
        {
          bcpara->rate         = bearer_cap->user_rate;
        }
        bcpara->bearer_serv  = MNCC_BEARER_SERV_FAX;
        bcpara->conn_elem    = MNCC_CONN_ELEM_TRANS;
        bcpara->stop_bits    = bearer_cap->num_stop;
        bcpara->data_bits    = bearer_cap->num_data;
        bcpara->parity       = bearer_cap->parity;
        bcpara->modem_type   = bearer_cap->modem_type;
        bcpara->flow_control = NOT_PRESENT_8BIT;
        break;
      }
    default:
      {
        TRACE_EVENT ("Call setup failed due to wrong ITC in bearer caps");

        break;
      }
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_moc_compatibility       |
+--------------------------------------------------------------------+

  PURPOSE : Check the compatibility of the incoming bearer caps
            to the mobile station configuration for the
            mobile originated call (after reception of a call
            proceeding message).

*/

GLOBAL UBYTE cc_moc_compatibility (const T_D_CALL_PROCEED * proceed)
{
  GET_INSTANCE_DATA;
  UBYTE proceed_ri;

  TRACE_FUNCTION ("cc_moc_compatibility()");

  if (proceed->v_bearer_cap)
  {
    if (proceed->v_repeat)
      proceed_ri = proceed->repeat;
    else
      proceed_ri = NOT_PRESENT_8BIT;
    if ((proceed_ri EQ NOT_PRESENT_8BIT) && (cc_data->ri NEQ NOT_PRESENT_8BIT))
    {
      /*
       * No repeat indicator from the network
       * that means only one bearer capability.
       * Mobile station has send two bearer
       * capabilities. Check whether bearer
       * capability is compatible to one of the
       * mobile station bearer capabilities.
       */
      if (cc_bcs_compatible (&proceed->bearer_cap,
                             &cc_data->bc1, TRUE))
      {
        cc_data->neg_ri = NOT_PRESENT_8BIT;
        cc_data->neg_bc1 = proceed->bearer_cap;
        cc_data->neg_serv1 = cc_data->serv1;
        cc_data->neg_serv2 = NOT_PRESENT_8BIT;
        cc_build_bcpara(&cc_data->neg_bcpara1,&proceed->bearer_cap);
        cc_data->neg_bcpara2.bearer_serv = NOT_PRESENT_8BIT; /* jk: wg. cc705 */
        cc_data->active_service = cc_data->serv1;
        return OKAY;
      }
      else
      {
        if (cc_bcs_compatible (&proceed->bearer_cap,
                               &cc_data->bc2, TRUE))
        {
          cc_data->neg_ri = NOT_PRESENT_8BIT;
          memcpy (&cc_data->neg_bc1, &proceed->bearer_cap,sizeof (T_M_CC_bearer_cap));
          cc_data->neg_serv1 = cc_data->serv2;
          cc_data->neg_serv2 = NOT_PRESENT_8BIT;
          cc_build_bcpara(&cc_data->neg_bcpara1,&proceed->bearer_cap);
          cc_data->active_service = cc_data->serv2;
          return OKAY;
        }
        else
        {
          return ERROR;
        }
      }
    }
    else
    {
      if (proceed_ri EQ cc_data->ri)
      {
        /*
         * both Repeat Indicators are the same
         * e.g. both not present OR both valid
         * but not the Bearer Capabilities.
         */
        if (cc_bcs_compatible (&proceed->bearer_cap,
            &cc_data->bc1, TRUE) EQ FALSE)
        {
           return ERROR;
        }
        else
        {
          if (proceed_ri NEQ NOT_PRESENT_8BIT)
          {
            if (cc_bcs_compatible ((T_M_CC_bearer_cap*)&proceed->bearer_cap_2,
               &cc_data->bc2, TRUE) EQ FALSE)
               return ERROR;
            /* 2 Bearer Capabilities from the Network sent */
            cc_data->neg_ri = proceed_ri;
            cc_data->neg_bc1 = proceed->bearer_cap;
            memcpy (&cc_data->neg_bc2, &proceed->bearer_cap_2,sizeof (T_M_CC_bearer_cap));
            cc_data->neg_serv1 = cc_data->serv1;
            cc_data->neg_serv2 = cc_data->serv2;
            cc_build_bcpara(&cc_data->neg_bcpara1,&cc_data->neg_bc1);
            cc_build_bcpara(&cc_data->neg_bcpara2,&cc_data->neg_bc2);
            cc_data->active_service = cc_data->serv1;
            return OKAY;
          }
          else
          {
            /* only 1 BC from the Network sent*/
            cc_data->neg_ri = proceed_ri;
            cc_data->neg_bc1 = proceed->bearer_cap;
            cc_data->neg_serv1 = cc_data->serv1;
            cc_data->neg_serv2 = NOT_PRESENT_8BIT;
            cc_build_bcpara(&cc_data->neg_bcpara1,&cc_data->neg_bc1);
            cc_data->neg_bcpara2 = cc_data->bcpara2;
            cc_data->active_service = cc_data->serv1;
            return OKAY;
          }
        }/*else*/
      }/*if*/
      else
      {
        /*
         * network has changed repeat indicator in an illegal way
         */
        return ERROR;
      }
    } /*else*/
  }
  cc_data->neg_ri = cc_data->ri;
  cc_data->neg_bc1 = cc_data->bc1;
  cc_data->neg_bc2 = cc_data->bc2;
  cc_data->neg_serv1 = cc_data->serv1;
  cc_data->neg_serv2 = cc_data->serv2;
  cc_data->neg_bcpara1 = cc_data->bcpara1;
  cc_data->neg_bcpara2 = cc_data->bcpara2;
  cc_data->active_service = cc_data->serv1;
  return OKAY;
}


#ifndef NTRACE
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_trace_state             |
+--------------------------------------------------------------------+

  PURPOSE : Maps CC state numbers to string constants.

*/
LOCAL T_S2I_STRING cc_sname (UBYTE state)
{
    switch (state)
    {
      case M_CC_CS_0   : return S2I_STRING("U0 ");
      case M_CC_CS_01  : return S2I_STRING("U0.1");
      case M_CC_CS_03  : return S2I_STRING("U0.3");
      case M_CC_CS_05  : return S2I_STRING("U0.5");
      case M_CC_CS_06  : return S2I_STRING("U0.6");
      case M_CC_CS_1   : return S2I_STRING("U1 ");
      case M_CC_CS_3   : return S2I_STRING("U3 ");
      case M_CC_CS_4   : return S2I_STRING("U4 ");
      case M_CC_CS_6   : return S2I_STRING("U6 ");
      case M_CC_CS_7   : return S2I_STRING("U7 ");
      case M_CC_CS_8   : return S2I_STRING("U8 ");
      case M_CC_CS_9   : return S2I_STRING("U9 ");
      case M_CC_CS_10  : return S2I_STRING("U10");
      case M_CC_CS_11  : return S2I_STRING("U11");
      case M_CC_CS_12  : return S2I_STRING("U12");
      case M_CC_CS_19  : return S2I_STRING("U19");
      case M_CC_CS_26  : return S2I_STRING("U26");
      case M_CC_CS_27  : return S2I_STRING("U27");
      case M_CC_CS_28  : return S2I_STRING("U28");
      case CS_101 : return S2I_STRING("U10.1");
      case CS_261 : return S2I_STRING("U26.1");
      default     : return S2I_STRING("???");
    }
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_set_state               |
+--------------------------------------------------------------------+

  PURPOSE : Sets the new state for call control. If necessary the
            instance administration is updated.

*/

GLOBAL void cc_set_state (UBYTE new_state)
{
  GET_INSTANCE_DATA;
#ifndef NTRACE
  vsi_o_state_ttrace ("CC[%d]: %s -> %s", 
                      cc_data->index_ti,
                      S2I_STRING(cc_sname(cc_data->state[cc_data->index_ti])),
                      S2I_STRING(cc_sname(new_state)));
#endif

  cc_data->state[cc_data->index_ti] = new_state;
  if ((new_state EQ M_CC_CS_0) AND (cc_data->ti NEQ cc_data->setup_reattempt_ti))
    srv_free_ti ();
  if (new_state EQ M_CC_CS_10) /* // Questionable after RLF/REEST */
    cc_data->hold_state[cc_data->index_ti] = M_CC_HLD_IDLE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_check_bc                |
+--------------------------------------------------------------------+

  PURPOSE :   Checks an incoming bearer capability. 
              But not only checks are done, also 
              neg_bearer_cap, bcpara and service are 
              computed. If negotiation is required, 
              negotiation will be set to TRUE 
              (but never to FALSE by this function).

  RETURN:     OKAY if bearer_cap could be processed,
              otherwise ERROR.

*/


GLOBAL UBYTE cc_check_bc (const T_M_CC_bearer_cap * bearer_cap,     /* in  */
                                T_M_CC_bearer_cap * neg_bearer_cap, /* out */
                                T_MNCC_bcpara     * bcpara,         /* out */
                                UBYTE        * service,        /* out */
                                BOOL         * negotiation )   /* in/out */
{
  GET_INSTANCE_DATA;	
  UBYTE found, temp;

  TRACE_FUNCTION ("cc_check_bc()");

  memcpy (neg_bearer_cap, bearer_cap, sizeof (T_M_CC_bearer_cap));

  cc_csf_ms_cap ();

  switch (bearer_cap->trans_cap)
  {
    case M_CC_ITC_SPEECH:
    case M_CC_ITC_AUXILIARY_SPEECH:
      if (!cc_voice_capability ())
        return ERROR;
      temp = bcpara->rad_chan_req; 
      memset (bcpara, 0, sizeof (T_MNCC_bcpara));
      bcpara->rad_chan_req = temp; 
      *service = MNCC_SERV_SPEECH;
      if (bearer_cap->trans_cap EQ M_CC_ITC_SPEECH) 
      {
        /* Speech bearer capability */
        if (cc_data->ctm_ena EQ MNCC_CTM_ENABLED)
        {
          bcpara->bearer_serv = MNCC_BEARER_SERV_SPEECH_CTM;
          cc_build_bc (neg_bearer_cap, service, bcpara);
          *negotiation = TRUE;
          return OKAY;
        }
        else
          bcpara->bearer_serv = MNCC_BEARER_SERV_SPEECH;
      }
      else
      {
        /* Speech bearer capability for alternate line service */
        if (cc_data->ctm_ena EQ MNCC_CTM_ENABLED)
        {
          bcpara->bearer_serv = MNCC_BEARER_SERV_AUX_SPEECH_CTM;
          cc_build_bc (neg_bearer_cap, service, bcpara);
          *negotiation = TRUE;
          return OKAY;
        }
        else
          bcpara->bearer_serv = MNCC_BEARER_SERV_AUX_SPEECH;
      }

      /* 
       * For negotiation, the rules of GSM 04.08 clause 9.3.2.2 shall apply
       */
      if ((FldGet(cc_data->mscap.chnMode, EFRSupV2) OR 
           FldGet(cc_data->mscap.chnMode, AHS) OR
           FldGet(cc_data->mscap.chnMode, AFS)) AND 
          (bearer_cap->v_speech_vers1 EQ FALSE))
      {
        /*
         * if the incoming message indicates only full rate channel, 
         * all supported bearer caps shall be returned.
         */
        cc_build_bc (neg_bearer_cap, service, bcpara);
        *negotiation = TRUE;
      }
      return OKAY;

    case M_CC_ITC_AUDIO:
    case M_CC_ITC_DIGITAL_UNRESTRICTED:
      *service = MNCC_SERV_DATA;
      found = FALSE;
      switch (cc_data->conn_elem)
      {
        case MNCC_CONN_ELEM_TRANS:
          cc_check_transparent_async     (&found, bearer_cap, neg_bearer_cap,
                                          bcpara, negotiation );
          break;
        case MNCC_CONN_ELEM_NON_TRANS:
          cc_check_non_transparent_async (&found, bearer_cap, neg_bearer_cap,
                                          bcpara, negotiation);
          break;
        case MNCC_CONN_ELEM_TRANS_PREF:
          cc_check_transparent_async     (&found, bearer_cap, neg_bearer_cap,
                                          bcpara, negotiation);
          cc_check_non_transparent_async (&found, bearer_cap, neg_bearer_cap,
                                          bcpara, negotiation);
          break;
        case MNCC_CONN_ELEM_NON_TRANS_PREF:
          cc_check_non_transparent_async (&found, bearer_cap, neg_bearer_cap,
                                          bcpara, negotiation);
          cc_check_transparent_async     (&found, bearer_cap, neg_bearer_cap,
                                          bcpara, negotiation);
          break;
        case MNCC_CONN_ELEM_NOT_PRES:
          TRACE_EVENT ("Call setup failed due to uninitialised conn_elem in cc_data");
          break;
      }
/*
 *    not yet supported (this stuff must be included in the switch (cc_data->conn_elem) above )
 *
 *    cc_check_transparent_sync      (&found, bearer_cap, neg_bearer_cap,
 *                                     bcpara, cc_data);
 *    cc_check_non_transparent_sync  (&found, bearer_cap, neg_bearer_cap,
 *                                     bcpara, cc_data);
 *
 *    cc_check_transparent_packet_sync
 *    cc_check_non_transparent_packet_sync
 *    cc_check_transparent_pad_async
 *    cc_check_non_transparent_pad_async
 *
 */
      if (found)
      {
        return OKAY;
      }
      return ERROR;

    case M_CC_ITC_FAX_GROUP_3:
      *service = MNCC_SERV_DATA;
      if (cc_check_transparent_fax (bearer_cap, neg_bearer_cap,
                                    bcpara, negotiation))
      {
        return OKAY;
      }
      return ERROR;

    default:
      /* reserved ITC value e.g. */
      return ERROR;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_check_transparent_async |
+--------------------------------------------------------------------+

  PURPOSE : Checks an incoming bearer capability for the transparent
            asynchronous data service.

*/
GLOBAL void cc_check_transparent_async (UBYTE        * found,
                                  const T_M_CC_bearer_cap * bearer_cap,
                                        T_M_CC_bearer_cap * neg_bearer_cap,
                                        T_MNCC_bcpara     * bcpara,
                                        BOOL         * negotiation )
{
  GET_INSTANCE_DATA;	
  TRACE_FUNCTION ("cc_check_transparent_async()");

  if (*found EQ FALSE)
  {
    memcpy (neg_bearer_cap, bearer_cap, sizeof (T_M_CC_bearer_cap));

    if (FldGet(cc_data->mscap.datCap1, AsySup))
    {
      /*
       * check only if transparent M_CC_ASYNCHRONOUS data services are supported
       */
      if (bearer_cap->sync_async NEQ M_CC_ASYNCHRONOUS OR
          bearer_cap->sig_access_prot NEQ M_CC_SIAP_I440) /* no async service */
       return;
      switch (bearer_cap->conn_elem)
      {
        case M_CC_CE_TRANSPA:
          neg_bearer_cap->compress = M_CC_COMP_NO;
          break;
        case M_CC_CE_TRANSPA_PREF:               /* set to trans async */
        case M_CC_CE_RLP_PREF:
          *negotiation = TRUE;
          neg_bearer_cap->conn_elem = M_CC_CE_TRANSPA;
          neg_bearer_cap->structure = M_CC_STRC_UNSTRUCTURED;
          neg_bearer_cap->compress = M_CC_COMP_NO;
          if (bearer_cap->modem_type EQ M_CC_MT_AUTO)
          {
            switch (bearer_cap->user_rate) /* set the modem type matching a data rate */
            {
            case MNCC_UR_0_3_KBIT:
                neg_bearer_cap->modem_type = M_CC_MT_V21;
                break;
            case MNCC_UR_1_2_KBIT:
                neg_bearer_cap->modem_type = M_CC_MT_V22;
                break;
            case MNCC_UR_2_4_KBIT:
                neg_bearer_cap->modem_type = M_CC_MT_V22BIS;
                break;
            case MNCC_UR_4_8_KBIT:
                neg_bearer_cap->modem_type = M_CC_MT_V32;
                break;
            case MNCC_UR_9_6_KBIT:
            case MNCC_UR_12_0_KBIT_TRANS:
            case MNCC_UR_14_4_KBIT:
                neg_bearer_cap->modem_type = M_CC_MT_V32;
                break;
            case MNCC_UR_1_2_KBIT_V23:
                neg_bearer_cap->modem_type = M_CC_MT_V23;
                break;
            default:
                neg_bearer_cap->modem_type = M_CC_MT_UNDEF;
                break;
            } /* END OF set the modem type matching the data rate */
          } /* END OF if (bearer_cap->modem_type EQ M_CC_MT_AUTO) */
          if ((bearer_cap->user_rate EQ M_CC_UR_9_6_KBIT) OR
              (bearer_cap->user_rate EQ M_CC_UR_12_0_KBIT_TRANS))
            neg_bearer_cap->intermed_rate = M_CC_IR_16_KBIT;
          else
            neg_bearer_cap->intermed_rate = M_CC_IR_8_KBIT;
          neg_bearer_cap->v_l2_ident = FALSE;
          neg_bearer_cap->v_user_inf_l2_prot = FALSE;
          break;

        case M_CC_CE_RLP:                       /* non-transparent service        */
          return;

        default:
          return;
      } /* end SWITCH (bearer_cap->conn_elem)*/
      /*
       * copy important parameters
       */
      *found                = TRUE;

       /*
        * this is a first step to 14.4
        * if FNUR is available, use this as user rate
        */
/* Implements Measure#  21 */
       cc_set_neg_bearer_cap(bearer_cap, bcpara, neg_bearer_cap, cc_data->mscap);
       bcpara->flow_control = NOT_PRESENT_8BIT;
       bcpara->conn_elem    = MNCC_CONN_ELEM_TRANS;
               
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)   MODULE  : CC_CFK                         |
| STATE   : code            ROUTINE : cc_check_non_transparent_async |
+--------------------------------------------------------------------+

  PURPOSE : Checks an incoming bearer capability for the
            non-transparent asynchronous data service.

*/

GLOBAL void cc_check_non_transparent_async (UBYTE        * found,
                                      const T_M_CC_bearer_cap * bearer_cap,
                                            T_M_CC_bearer_cap * neg_bearer_cap,
                                            T_MNCC_bcpara     * bcpara,
                                            BOOL         * negotiation ) 
{
  GET_INSTANCE_DATA;	
  TRACE_FUNCTION ("cc_check_non_transparent_async()");

  if (*found EQ FALSE)
  {
    memcpy (neg_bearer_cap, bearer_cap, sizeof (T_M_CC_bearer_cap));

    if (FldGet (cc_data->mscap.datCap1, RLPSup))
    {
      /*
       * check only if non-transparent M_CC_ASYNCHRONOUS data services are supported
       */
      if (bearer_cap->sync_async NEQ M_CC_ASYNCHRONOUS OR
          bearer_cap->sig_access_prot NEQ M_CC_SIAP_I440) /* no async service */
       return;
      switch (bearer_cap->conn_elem)
      {
        case M_CC_CE_RLP:
          break;

        case M_CC_CE_RLP_PREF:                     /* set to non-trans async */
        case M_CC_CE_TRANSPA_PREF:
          *negotiation = TRUE;
          neg_bearer_cap->conn_elem = M_CC_CE_RLP;
          neg_bearer_cap->structure = M_CC_STRC_SERV_DATA_INTEG;
          /*
           * TA 29.2.3.5 Negotiation of intermediate rate
           */
          if ((bearer_cap->nirr EQ M_CC_NIRR_DATA) AND
              (bearer_cap->user_rate <= M_CC_UR_4_8_KBIT))
            neg_bearer_cap->intermed_rate = M_CC_IR_8_KBIT;
          else
            neg_bearer_cap->intermed_rate = M_CC_IR_16_KBIT;
          break;

        case M_CC_CE_TRANSPA:                       /* transparent service */
          return;

        default:
          return;
      }

      /*
       * copy important parameters
       */
      *found                = TRUE;
       /*
        * this is a first step to 14.4
        * if FNUR is available, use this as user rate
        */
/* Implements Measure#  21 */
       cc_set_neg_bearer_cap(bearer_cap, bcpara, neg_bearer_cap, cc_data->mscap);
       bcpara->conn_elem    = MNCC_CONN_ELEM_NON_TRANS;
                 
       if (bearer_cap->v_user_inf_l2_prot)
       {
         switch (bearer_cap->user_inf_l2_prot)
         {
           case M_CC_L2_ISO6429:
             bcpara->flow_control = MNCC_INBAND_FLOW_CONTROL;
             break;
           case M_CC_L2_COPFC:
             bcpara->flow_control = MNCC_NO_FLOW_CONTROL;
             break;
           default:
             bcpara->flow_control = MNCC_OUTBAND_FLOW_CONTROL;
             break;
         }
       }
       else
         bcpara->flow_control = MNCC_OUTBAND_FLOW_CONTROL;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_check_transparent_fax   |
+--------------------------------------------------------------------+

  PURPOSE : Checks an incoming bearer capability for the transparent
            fax service.

*/

GLOBAL UBYTE cc_check_transparent_fax (const T_M_CC_bearer_cap * bearer_cap,
                                             T_M_CC_bearer_cap * neg_bearer_cap,
                                             T_MNCC_bcpara     * bcpara,
                                             BOOL         * negotiation )
{                                     
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_check_transparent_fax()");

  memcpy (neg_bearer_cap, bearer_cap, sizeof (T_M_CC_bearer_cap));

  if (FldGet (cc_data->mscap.datCap1, TFaxSup))
  {
    /*
     * check only if transparent fax services are supported
     */

    if (bearer_cap->sync_async NEQ M_CC_SYNCHRONOUS OR
        bearer_cap->sig_access_prot NEQ M_CC_SIAP_I440) /* no async service */
      return FALSE;

    switch (bearer_cap->conn_elem)
    {
      case M_CC_CE_TRANSPA:
        break;

      case M_CC_CE_TRANSPA_PREF:               /* set to trans async */
      case M_CC_CE_RLP_PREF:
        *negotiation = TRUE;
        neg_bearer_cap->conn_elem = M_CC_CE_TRANSPA;
        neg_bearer_cap->structure = M_CC_STRC_UNSTRUCTURED;
        if ((bearer_cap->user_rate EQ M_CC_UR_9_6_KBIT) OR
            (bearer_cap->user_rate EQ M_CC_UR_12_0_KBIT_TRANS))
          neg_bearer_cap->intermed_rate = M_CC_IR_16_KBIT;
        else
          neg_bearer_cap->intermed_rate = M_CC_IR_8_KBIT;
        neg_bearer_cap->v_l2_ident = FALSE;
        neg_bearer_cap->v_user_inf_l2_prot = FALSE;
        break;


      case M_CC_CE_RLP:                       /* non-transparent service */
      default:
        return FALSE;
    }

     /*
      * copy important parameters
      */
       /*
        * this is a first step to 14.4
        * if FNUR is available, use this as user rate
        */
     if (bearer_cap->v_fnur AND (bearer_cap->fnur EQ M_CC_FNUR_14400))
     {
       bcpara->rate       = 8;
       /*
        * define acceptable channel codings
        */
       neg_bearer_cap->acc           = M_CC_ACC_96;        
       if (FldGet (cc_data->mscap.datCap2, DHRSup))
         neg_bearer_cap->acc        += M_CC_ACC_48;
       if (FldGet (cc_data->mscap.datCap1, Dr14_4Sup))
         neg_bearer_cap->acc        += M_CC_ACC_144;
       neg_bearer_cap->v_acc          = TRUE;

       neg_bearer_cap->intermed_rate = M_CC_IR_16_KBIT;

       neg_bearer_cap->v_mTch         = TRUE;
       neg_bearer_cap->mTch           = M_CC_MAX_TCH_1;
       neg_bearer_cap->v_uimi         = TRUE;
       neg_bearer_cap->uimi           = M_CC_UIMI_NOT_ALLOWED;
       neg_bearer_cap->modem_type_2   = M_CC_OTHER_MODEM_TYPE_V34;
     }
     else
       bcpara->rate         = bearer_cap->user_rate;
     bcpara->bearer_serv  = MNCC_BEARER_SERV_FAX;
     bcpara->conn_elem    = MNCC_CONN_ELEM_TRANS;
     bcpara->stop_bits    = bearer_cap->num_stop;
     bcpara->data_bits    = bearer_cap->num_data;
     bcpara->parity       = bearer_cap->parity;
     bcpara->modem_type   = bearer_cap->modem_type;
     bcpara->flow_control = NOT_PRESENT_8BIT;
     return TRUE;
  }
  return FALSE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_facility_ind      |
+--------------------------------------------------------------------+

  PURPOSE : Forwards a facility information element to the upper layer
            if available.

*/

GLOBAL void cc_build_facility_ind (UBYTE        context,
                                   UBYTE        valid,
                             const T_M_CC_facility * facility )
{
  GET_INSTANCE_DATA;	
  TRACE_FUNCTION ("cc_build_facility_ind()");

  if (valid)
  {
    PALLOC (facility_ind, MNCC_FACILITY_IND);

    facility_ind->ti = cc_data->ti;
    facility_ind->fac_context   = context;
    facility_ind->fac_inf.l_fac = facility->c_fac<<3;
    facility_ind->fac_inf.o_fac = 0;

    memcpy (facility_ind->fac_inf.fac, facility->fac, facility->c_fac);

    PSENDX (MMI, facility_ind);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_user_user_ind     |
+--------------------------------------------------------------------+

  PURPOSE : Forwards a user user information element to the upper layer
            if available.

*/

GLOBAL void cc_build_user_user_ind (UBYTE         context,
                                    UBYTE         valid,
                                    T_M_CC_user_user * user_user )
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_build_user_user_ind()");

  if (valid)
  {
    PALLOC (user_ind, MNCC_USER_IND);

    user_ind->ti = cc_data->ti;
    user_ind->user.info_context   = context;
    user_ind->user.c_info    = user_user->c_info;
    user_ind->user.pd        = user_user->pd;
    memcpy (user_ind->user.info, user_user->info,
            MNCC_USER_LEN);

    PSENDX (MMI, user_ind);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_check_capabilities      |
+--------------------------------------------------------------------+

  PURPOSE : The mobile station capabilities are checked against the
            upper layer request.

  RETURN:   TRUE if bearer capabilities supported.
            FALSE if bearer capabilitis not supported.

*/

GLOBAL UBYTE cc_check_capabilities (const T_MNCC_bcpara    * bcpara)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_check_capabilities()");

  cc_csf_ms_cap ();
  switch (bcpara->bearer_serv)
  {
    case MNCC_BEARER_SERV_SYNC:
      if (bcpara->conn_elem EQ MNCC_CONN_ELEM_TRANS)
      {
        return (FldGet (cc_data->mscap.datCap1, TSynSup) NEQ 0);
      }
      return (FldGet (cc_data->mscap.datCap1, NTSynSup) NEQ 0);

    case MNCC_BEARER_SERV_ASYNC:
      if (bcpara->conn_elem EQ MNCC_CONN_ELEM_TRANS)
      {
        return (FldGet (cc_data->mscap.datCap1, AsySup) NEQ 0);
      }
      return (FldGet (cc_data->mscap.datCap1, RLPSup) NEQ 0);

    case MNCC_BEARER_SERV_PAD_ACCESS:
      if (bcpara->conn_elem EQ MNCC_CONN_ELEM_TRANS)
      {
        return (FldGet (cc_data->mscap.datCap2,TPadSup) NEQ 0);
      }
      return (FldGet (cc_data->mscap.datCap2,NTPadSup) NEQ 0);

    case MNCC_BEARER_SERV_PACKET_ACCESS:
      if (bcpara->conn_elem EQ MNCC_CONN_ELEM_TRANS)
      {
        return (FldGet (cc_data->mscap.datCap2, TPackSup) NEQ 0);
      }
      return (FldGet (cc_data->mscap.datCap2, NTPackSup) NEQ 0);

    case MNCC_BEARER_SERV_FAX:
      if (bcpara->conn_elem EQ MNCC_CONN_ELEM_TRANS)
      {
        return (FldGet (cc_data->mscap.datCap1, TFaxSup) NEQ 0);
      }
      return FALSE;

    case MNCC_BEARER_SERV_SPEECH:
    case MNCC_BEARER_SERV_AUX_SPEECH:
    case MNCC_BEARER_SERV_SPEECH_CTM:
    case MNCC_BEARER_SERV_AUX_SPEECH_CTM:
      return cc_voice_capability ();

    case MNCC_BEARER_SERV_NOT_PRES:
      return TRUE;

    default:
      return FALSE;
  }
}

#ifdef SIM_TOOLKIT
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_bearer_cap_code         |
+--------------------------------------------------------------------+

  PURPOSE:      Checks the compatibility of bearer capabilities. 
                Bearer capabilities in paramter form are tranformed to 
                coded form.
   
  RETURN VALUE: ERROR if bearer caps not supported, otherwise OKAY
                Special case of no bearer caps given is OKAY.

*/

GLOBAL UBYTE cc_bearer_cap_code (const T_MNCC_bcpara *bcpara, T_MNCC_bcconf *bcconf )
{
  T_M_CC_bearer_cap bearer_cap;  /* A lot of stuff on stack */  /*lint !e813 length info of struct*/
  UBYTE dummy_service;
  UBYTE *ptr;
  UBYTE result;

  TRACE_FUNCTION ("cc_bearer_cap_code()"); 

  /* Clear output */
  bcconf->bc_len = 0;
  memset (bcconf->bc, 0x00, sizeof (bcconf->bc));
  
  if (bcpara->bearer_serv EQ MNCC_BEARER_SERV_NOT_PRES)
    return OKAY; /* No BC in -> no BC out */
  
   /* 
    * Check requested upper layer capabilities 
    * against mobile station capabilities
    */
  if (cc_check_capabilities (bcpara) EQ FALSE)
  {
      /* MS doesn't support the requested services */
    return ERROR;
  }

  cc_build_bc (&bearer_cap, 
                &dummy_service, /* Not used */
                bcpara);
  
  /*           
   * Build a U_EMERGE_SETUP message which contains the coded bearer caps.
   * The U_EMERGE_SETUP message was chosen because it is the most 
   * simple message containing bearer capabilities. In the next step, 
   * read the coded bearer capabilities directly from this message. 
   */
  CCD_START;
  {
    MCAST (emerge_setup, U_EMERGE_SETUP);
    PALLOC_MSG (data, MMCM_DATA_REQ, U_EMERGE_SETUP);

    emerge_setup->msg_type     = U_EMERGE_SETUP;
    emerge_setup->v_bearer_cap = TRUE;
    emerge_setup->bearer_cap   = bearer_cap;
    data->sdu.o_buf = CC_ENCODE_OFFSET;
    ccd_codeMsg (CCDENT_CC,
                 UPLINK,
                 (T_MSGBUF *) &data->sdu,
                 (UBYTE *) emerge_setup,
                 NOT_PRESENT_8BIT);

    ptr = &data->sdu.buf[CC_ENCODE_OFFSET >> 3];

    /* 
     * The following is an internal check and not expected to fail.
     * Short circuit evaluation, ANSI C guarantees execution order LR here.
     */
    if ((*ptr++ EQ U_EMERGE_SETUP) AND /* Message type */
        (*ptr++ EQ 0x04) AND /* BC tag (0x04) */
        (*ptr <= sizeof (bcconf->bc))) /* Field length not exceeded */
    {
      result = OKAY;
      bcconf->bc_len = *ptr++;
      memcpy (bcconf->bc, ptr, bcconf->bc_len);
    }
    else
      result = ERROR;
    PFREE (data);
  }
  CCD_END;

  return result;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_bearer_cap_decode       |
+--------------------------------------------------------------------+

  PURPOSE:      Checks the compatibility of bearer capabilities. 
                Bearer capabilities in coded form are tranformed to 
                parameter form.
   
  RETURN VALUE: ERROR if bearer caps not supported
                Special case of empty bearer caps given is OKAY

*/

GLOBAL UBYTE cc_bearer_cap_decode (const T_MNCC_bcconf *bcconf, T_MNCC_bcpara *bcpara )
                                   
{
  T_M_CC_bearer_cap dummy_neg_bearer_cap; /* A lot of stuff on stack */ /*lint !e813 length info of struct*/
  UBYTE        dummy_service;       
  BOOL         dummy_negotiation;
  UBYTE        * start_ptr;
  UBYTE        * ptr;
  BYTE         result;

  TRACE_FUNCTION ("cc_bearer_cap_decode()");

  bcpara->bearer_serv = MNCC_BEARER_SERV_NOT_PRES;
    
  if (bcconf->bc_len EQ 0)
    return OKAY; /* No BC in -> no BC out */
  
  CCD_START 
  {
    /*           
     * Build a U_EMERGE_SETUP message which contains the coded bearer caps.
     * The U_EMERGE_SETUP message was chosen because it is the most 
     * simple message containing bearer capabilities.
     */
    MCAST (emerge_setup, U_EMERGE_SETUP);
    PALLOC_MSG (data, MMCM_DATA_REQ, U_EMERGE_SETUP);
    data->sdu.o_buf = CC_ENCODE_OFFSET;
    ptr = start_ptr = &data->sdu.buf[CC_ENCODE_OFFSET >> 3];
    *ptr++ = U_EMERGE_SETUP; /* Message type field */
    *ptr++ = 0x04;           /* BC tag */
    *ptr++ = bcconf->bc_len; /* BC length */
    memcpy(ptr, bcconf->bc, bcconf->bc_len);
    ptr += bcconf->bc_len;
    data->sdu.l_buf = (ptr - start_ptr) << 3;
    
    result = ERROR;
    if ((ccd_decodeMsg (CCDENT_CC,
                       UPLINK,
                       (T_MSGBUF *) &data->sdu,
                       (UBYTE *)_decodedMsg,
                       NOT_PRESENT_8BIT) EQ ccdOK))
    {
      if (emerge_setup->v_bearer_cap)
      {
       /* 
        * CCD was able to decode emergency setup message 
        * and contained BC. Check them.
        * This also transforms coded form into parameter form as desired
        */
        if(emerge_setup->bearer_cap.v_rad_chan_req)
        {
           bcpara->rad_chan_req = emerge_setup->bearer_cap.rad_chan_req ;
           TRACE_EVENT_P1 ("bcpara->rad_chan_req  = %d", bcpara->rad_chan_req );
        }
         result = cc_check_bc (&emerge_setup->bearer_cap, /* Input BC */
                             &dummy_neg_bearer_cap,     /* Not used */
                             bcpara,                    /* BC parameter */ 
                             &dummy_service,            /* Not used */
                             &dummy_negotiation );      /* Not used */
       }
    }
    PFREE (data);
  }
  CCD_END;

  return result;
}

#endif /*SIM_TOOLKIT */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_llc               |
+--------------------------------------------------------------------+

  PURPOSE : Define low layer compat depending on the bearer capability
            parameters.

*/

static const UBYTE BC_LLC_USERRATE[8] = 
{
      /*            reserved         0000  */  0,
      /*            0.3              0001  */  0x1E,
      /*            1.2              0010  */  2,
      /*            2.4              0011  */  3,
      /*            4.8              0100  */  5,
      /*            9.6              0101  */  8,
      /*            12.0 transp      0110  */  0x1F,
      /*            12.0/75          0111  */  0x18 
};

static const UBYTE BC_LLC_INTERMED_RATE [4] =
{
      /*            not used         00    */   0x00,
      /*            not used         01    */   0x00,
      /*            8 kbit/s         10    */   0x20,
      /*            16 kbit/s        11    */   0x40
};

GLOBAL void cc_build_llc (UBYTE            * v_low_layer_compat,
                          T_M_CC_low_layer_comp * low_layer_compat,
                    const T_M_CC_bearer_cap     * bc)
{
  TRACE_FUNCTION ("cc_build_llc()");

  /*
   * infoelement shall be included
   */
  *v_low_layer_compat = TRUE;

  if (bc->trans_cap EQ M_CC_ITC_DIGITAL_UNRESTRICTED)
  {
    /*
     * length is fixed with 6 bytes
     */
    low_layer_compat->c_llc = 6;

    /*
     * octet 3
     *
     * bit 8     :  extension = 1 (fixed)
     * bit 76    :  coding standard = GSM (fixed)
     * bit 54321 :  information transfer capability = UDI (fixed)
     */
    low_layer_compat->llc[0] = 0x88;

    /*
     * octet 4
     *
     * bit 8     :  extension = 1 (fixed)
     * bit 76    :  transfer mode = circuit (fixed)
     * bit 54321 :  information transfer rate = 64 kbit/s (fixed)
     */
    low_layer_compat->llc[1] = 0x90;

    /*
     * octet 5
     *
     * bit 8     :  extension = 0 (fixed)
     * bit 76    :  layer 1 identifier = 1 (fixed)
     * bit 54321 :  user information layer 1 protocol = V.110 (fixed)
     */
    low_layer_compat->llc[2] = 0x21;

    /*
     * octet 5a
     * 
     * bit 8     :  extension = 0 (fixed)
     * bit 7     :  sync/async -> from BC
     *                                BC        LLC
     *              M_CC_ASYNCHRONOUS       1          1
     *              M_CC_SYNCHRONOUS        0          0
     * bit 6     :  negotiation = 0 (fixed)
     * bit 54321 :  user rate -> from BC
                                      BC        LLC
                    0.3              0001       11110
                    1.2              0010       00010
                    2.4              0011       00011
                    4.8              0100       00101
                    9.6              0101       01000
                    12.0 transp      0110       11111
                    12.0/75          0111       11000
    */

    if (bc->sync_async)
      low_layer_compat->llc[3] = 0x40;
    else
      low_layer_compat->llc[3] = 0x00;

    low_layer_compat->llc[3] += BC_LLC_USERRATE[bc->user_rate & 7];

    /*
     * octet 5b
     *
     * bit 8     :  extension = 0 (fixed)
     * bit 76    :  intermediate rate -> from BC
     *                                BC         LLC
     *              not used          00/01      00
     *              8 kbit/s          10         01
     *              16 kbit/s         11         10
     *              32 kbit/s         --         11
     * bit 5     :  NIC/TX = 0 (fixed)
     * bit 4     :  NIC/RX = 0 (fixed)
     * bit 3     :  Flow Control on Tx = 0 (fixed)
     * bit 2     :  Flow Control on Rx = 0 (fixed)
     * bit 1     :  Spare = 0 (fixed)
     */
    low_layer_compat->llc[4] = BC_LLC_INTERMED_RATE[bc->intermed_rate];

    /*
     * octet 5c
     *
     * bit 8     :  extension = 1 (fixed)
     * bit 76    :  number of stop bits -> BC
     *                               BC          LLC
     *              not used         --          00
     *              1   bit          0           01
     *              1.5 bits         --          10
     *              2   bits         1           11
     * bit 54    :  number of data bits -> BC
     *                               BC          LLC
     *              not used                     00
     *              5 bits                       01
     *              7 bits           0           10
     *              8 bits           1           11
     * bit 321   :  parity -> BC
     *                               BC          LLC
     *              odd              000         000
     *              even             010         010
     *              none             011         011
     *              forced to 0      100         100
     *              forced to 1      101         101
     */
     low_layer_compat->llc[5] = 0x80 + bc->parity;

     if (bc->num_stop)
       low_layer_compat->llc[5] += 0x60;
     else
       low_layer_compat->llc[5] += 0x20;

     if (bc->num_data)
       low_layer_compat->llc[5] += 0x18;
     else
       low_layer_compat->llc[5] += 0x10;
  }
  else
  {
    /*
     * set length to zero -> not applicable
     */
    memset (low_layer_compat, 0, sizeof (T_M_CC_low_layer_comp));
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_get_ss_diag             |
+--------------------------------------------------------------------+

  PURPOSE : Added for CQ23619: Extracts the supplementary diagnostic 
  from the disconnect indication if there a respective cause given.
  For further details see: TS 24.008 section 10.5.4.11 "NOTE 1"
  (This implementation follows version V3.19.0 (2004-06)) 

  IN: - curr_cause: current cause 
      - disconnect indication received by CC
  OUT:- ss_diag: ss diagnostic
*/

GLOBAL UBYTE cc_get_ss_diag ( USHORT curr_cause,
                              T_D_DISCONNECT * disc)
{

  UBYTE ss_diag = MNCC_SS_DIAG_NOT_PROVIDED; /* holds the ss diagnostic */

  TRACE_FUNCTION ("cc_get_ss_diag()");

  /*
   * Only the following causes may also provide a supplementary diagnostic value
   * - cause 17: User busy - MNCC_CAUSE_USER_BUSY
   * - cause 29: Facility rejected - MNCC_CAUSE_FACILITY_REJECT
   * - cause 34: No circuit/channel available - MNCC_CAUSE_NO_CHAN_AVAIL
   * - cause 50: Requested facility not subscribed - MNCC_CAUSE_FACILITY_UNSUBSCRIB
   * - cause 55: Incoming call barred within the CUG - MNCC_CAUSE_BARRED_IN_CUG
   * - cause 69: Requested facility not implemented - MNCC_CAUSE_FACILITY_NOT_IMPLEM
   * - cause 87: User not member of CUG - MNCC_CAUSE_USER_NOT_IN_CUG
   */
  switch (curr_cause)
  {
    case MNCC_CAUSE_USER_BUSY:
    case MNCC_CAUSE_FACILITY_REJECT:
    case MNCC_CAUSE_NO_CHAN_AVAIL:
    case MNCC_CAUSE_FACILITY_UNSUBSCRIB:
    case MNCC_CAUSE_BARRED_IN_CUG:
    case MNCC_CAUSE_FACILITY_NOT_IMPLEM:
    case MNCC_CAUSE_USER_NOT_IN_CUG:
      /* 
       * parse ss diagnostic, find details of encoding in TS 24.008 
       * section 10.5.4.11 "NOTE 1"
       */
      if(disc->cc_cause.c_diag > 0) /* check count of diag elements */
      {
        /* take the first byte, ignore the rest */
        ss_diag = disc->cc_cause.diag[0] & 0x7F;
        if (ss_diag EQ 0x7F)
        {
          ss_diag = MNCC_SS_DIAG_NOT_PROVIDED;
        }
      }
      break;
    default:
      /* force MNCC_SS_DIAG_NOT_PROVIDED return */      
      break;
  } /* end of switch (curr_cause) */

  return ss_diag;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_build_cause             |
+--------------------------------------------------------------------+

  PURPOSE : Added for CQ62853: We have to pass the cause byte stream as 
  sent by the network to ACI. This has been done to satisfy the SAT test
  case 27.22.4.16-2 for event download(CALL DISCONNECTED).  

  IN: - decoded_cause_val: decoded cause value      
  OUT:- raw_cause_length: Number of bytes in raw_cause_bytes
        raw_cause_bytes: Encoded bytes as in 24.008 v3.15.0 sec 10.5.4.11
*/

GLOBAL UBYTE cc_build_cause (T_M_CC_cc_cause   * decoded_cause_val,
                             UBYTE   * raw_cause_bytes)
{

  UBYTE cause_ix = 0;

  TRACE_FUNCTION ("cc_build_cause()");

  memset(raw_cause_bytes, 0, MNCC_MAX_CC_CAUSE_LENGTH);

  /* Fill the coding standard and the location fields */
  raw_cause_bytes[cause_ix] = (decoded_cause_val->cs << 5) | decoded_cause_val->loc;

  if(decoded_cause_val->v_rec)
  {  
    /* ext bit is not set in the first byte. Move to the next byte */  
    cause_ix++;
    /* recommendation value is copied and ext bit is set */
    raw_cause_bytes[cause_ix++] = decoded_cause_val->rec | 0x80;
  }
  else
  {
   /* If the recommendation field is not present, set the ext bit in the first byte*/ 
   raw_cause_bytes[cause_ix] = raw_cause_bytes[cause_ix] | 0x80;
   cause_ix++;
  }

  /* Fill the cause value and set the ext bit */
  raw_cause_bytes[cause_ix++] = decoded_cause_val->cause | 0x80;

  /* Fill the diagnostics value */
  if(decoded_cause_val->c_diag NEQ 0)
  {
   memcpy (raw_cause_bytes + cause_ix,
           decoded_cause_val->diag,
           decoded_cause_val->c_diag);
  }

  /* Return the number of bytes that have been filled */ 
  return (cause_ix + decoded_cause_val->c_diag);   
}


/* Implements Measure#  21 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_set_neg_bearer_cap      |
+--------------------------------------------------------------------+

PURPOSE   : Copy some bearer capabilities values
*/

LOCAL void cc_set_neg_bearer_cap (const T_M_CC_bearer_cap * bearer_cap, 
                                        T_MNCC_bcpara     * bcpara, 
                                        T_M_CC_bearer_cap * neg_bearer_cap, 
                                        EF_MSCAP mscap)
{

  TRACE_FUNCTION ("cc_set_neg_bearer_cap()");

  if (bearer_cap->v_fnur AND (bearer_cap->fnur EQ M_CC_FNUR_14400))
  {
     bcpara->rate       = MNCC_UR_14_4_KBIT;
    /*
     * define acceptable channel codings
     */
      neg_bearer_cap->acc           = M_CC_ACC_96;        
     if (FldGet (mscap.datCap2, DHRSup))
     {
       neg_bearer_cap->acc        += M_CC_ACC_48;
     }
     if (FldGet (mscap.datCap1, Dr14_4Sup))
     {
       neg_bearer_cap->acc        += M_CC_ACC_144;
     }
     neg_bearer_cap->v_acc          = TRUE;

     neg_bearer_cap->v_intermed_rate = TRUE;
     neg_bearer_cap->intermed_rate = M_CC_IR_16_KBIT;
     neg_bearer_cap->v_mTch         = TRUE;
     neg_bearer_cap->mTch           = M_CC_MAX_TCH_1;
     neg_bearer_cap->v_uimi         = TRUE;
     neg_bearer_cap->uimi           = M_CC_UIMI_NOT_ALLOWED;
     neg_bearer_cap->v_waiur        = TRUE;
     neg_bearer_cap->waiur          = M_CC_WAIUR_14400;
     neg_bearer_cap->v_modem_type_2 = TRUE;
     neg_bearer_cap->modem_type_2   = M_CC_OTHER_MODEM_TYPE_V34;
  }
  else
  {
    bcpara->rate         = bearer_cap->user_rate;
  }

  /* ITC - Information Transfer Capability */
  if (bearer_cap->trans_cap == M_CC_ITC_DIGITAL_UNRESTRICTED)
  {
  	bcpara->transfer_cap = MNCC_ITC_UDI;
  }
  else if (bearer_cap->trans_cap == M_CC_ITC_AUDIO)
  {
        bcpara->transfer_cap = MNCC_ITC_NONE;
  }
  bcpara->bearer_serv  = MNCC_BEARER_SERV_ASYNC;
  bcpara->stop_bits    = bearer_cap->num_stop;
  bcpara->data_bits    = bearer_cap->num_data;
  bcpara->parity       = bearer_cap->parity;

  if (bearer_cap->modem_type_2 == M_CC_OTHER_MODEM_TYPE_V34)
  {
    bcpara->modem_type   = MNCC_MT_V34;
  }
  else
  {
    bcpara->modem_type   = bearer_cap->modem_type;
  }

}

/* Implements Measure#  15 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_fill_struct             |
+--------------------------------------------------------------------+

PURPOSE   : Copy values from setup_ind to setup_cnf
*/

LOCAL void cc_fill_struct         (UBYTE is_num_pres, 
                                   T_MNCC_calling_party *calling_party,
                                   const T_M_CC_calling_num *calling_num)
{

  if (is_num_pres)
  {
    calling_party->ton   = calling_num->ton;
    calling_party->npi   = calling_num->npi;
    calling_party->c_num = calling_num->c_num;
    memcpy (calling_party->num, calling_num->num,
            calling_num->c_num);
    if (calling_num->v_screen)
    {
      calling_party->screen = calling_num->screen;
    }
    else
    {
      calling_party->screen = NOT_PRESENT_8BIT;
    }
    if (calling_num->v_present)
    {
      calling_party->present = calling_num->present;
    }
    else
    {
      calling_party->present = NOT_PRESENT_8BIT;
    }
  }
  else
  {
    calling_party->ton     = MNCC_TON_NOT_PRES;
    calling_party->npi     = MNCC_NPI_NOT_PRES;
    calling_party->present = MNCC_PRES_NOT_PRES;
    calling_party->screen  = MNCC_SCREEN_IND_NOT_PRES;
    calling_party->c_num   = 0;
  }
}
     

/* Implements Measure#  24, 25 and 26 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_CFK                     |
| STATE   : code                ROUTINE : cc_fill_sub_struct         |
+--------------------------------------------------------------------+

PURPOSE   : Copy values from setup_cnf to setup_ind
*/

LOCAL void cc_fill_sub_struct       (UBYTE is_num_pres, 
                                     T_MNCC_connected_number_sub *connected_number_sub,
                                     const T_M_CC_connect_subaddr *connect_subaddr)
{
  if (is_num_pres)
  {
    connected_number_sub->tos  = connect_subaddr->tos;
    connected_number_sub->odd_even  =
               connect_subaddr->odd_even;
    connected_number_sub->c_subaddr =
               connect_subaddr->c_subaddr;
    memcpy (connected_number_sub->subaddr,
            connect_subaddr->subaddr, MNCC_SUB_LENGTH);
  }
  else
  {
    connected_number_sub->tos       = NOT_PRESENT_8BIT;
    connected_number_sub->c_subaddr = 0;
  }
}                                  
#endif
