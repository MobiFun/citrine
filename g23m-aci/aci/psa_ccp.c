/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_CCP
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
|  Purpose :  This module defines the processing functions for the
|             primitives send to the protocol stack adapter by call
|             control.
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_CCP_C
#define PSA_CCP_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "ccdapi.h"

#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_mem.h"
#include "aci_lst.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "aci.h"
#include "psa.h"
#include "psa_cc.h"
#include "psa_ss.h"
#include "psa_mmi.h"
#include "psa_mm.h"
#include "cmh.h"
#include "cmh_cc.h"

#ifdef SIM_TOOLKIT
#include "psa_sat.h"
#include "cmh_sat.h"
#endif    /* of #ifdef SIM_TOOLKIT */

#include "aoc.h"
#include "psa_sim.h"

#if defined (FF_WAP) || defined (FF_SAT_E)
#include "wap_aci.h"
#endif /* of FF_WAP */

#include "l4_tim.h"

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/

#ifdef FF_EM_MODE
EXTERN USHORT em_relcs;
#endif /* FF_EM_MODE */

/*==== VARIABLES ==================================================*/

EXTERN T_PCEER causeMod;
EXTERN SHORT causeCeer;

/*==== FUNCTIONS ==================================================*/


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCP                 |
|                                 ROUTINE : psa_mncc_setup_ind      |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNCC_SETUP_IND primitive send by CC.
            this indicates an incoming call.

*/

GLOBAL void psa_mncc_setup_ind
                   ( T_MNCC_SETUP_IND *mncc_setup_ind )
{
  SHORT cId;                 /* holds call id */
  T_CC_CALL_TBL * pCtbNtry;  /* holds pointer to call table entry */
  int i;

  TRACE_FUNCTION ("psa_mncc_setup_ind()");

  causeMod  = P_CEER_mod;      /* Clear module which was set for ceer */
  causeCeer = CEER_NotPresent; /* Clear extended error cause */

  /* stop redialling timer if necessary */
  if (rdlPrm.rdlcId NEQ NO_ENTRY)
  {
    TIMERSTOP(ACI_REPEAT_HND);
    
    #ifdef SIM_TOOLKIT
      if( ( ccShrdPrm.ctb[rdlPrm.rdlcId]->SATinv & SAT_REDIAL ) )
      { /* This is the call invoked by SAT */
        T_ACI_SAT_TERM_RESP resp_data;

        psaSAT_InitTrmResp( &resp_data );
        psaSAT_SendTrmResp( RSLT_USR_CLR_DWN, &resp_data );
      }
    #endif /* SIM_TOOLKIT */

    psaCC_FreeCtbNtry (rdlPrm.rdlcId);  
    for(i = 0; i < CMD_SRC_MAX; i++)
    {
      R_AT(RAT_RDL, (T_ACI_CMD_SRC)i)(REDIAL_STOP);
    }
    /* reset some redial parameter */
    rdlPrm.rdlcId =   NO_ENTRY;
  }

/*
 *-------------------------------------------------------------------
 * check for new entry in call table
 *-------------------------------------------------------------------
 */
  cId = psaCC_ctbNewEntry();

  if( cId < 0 )
  {
   /*
    * disconnect call, due to full call table.
    */
    {
      UBYTE ti = mncc_setup_ind -> ti;
      PREUSE (mncc_setup_ind, mncc_disconnect_req, MNCC_DISCONNECT_REQ);
      mncc_disconnect_req -> ti    = ti;
      mncc_disconnect_req -> cause = MNCC_CAUSE_USER_BUSY;
      mncc_disconnect_req -> fac_inf.l_fac = 0;
      mncc_disconnect_req -> ss_version = MNCC_SS_VER_NOT_PRES;
      PSENDX (CC, mncc_disconnect_req);
    }

    TRACE_EVENT ("MTC rejected due to full call table");

    return;
  }

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */
  pCtbNtry = ccShrdPrm.ctb[cId];
  pCtbNtry -> curCs = MNCC_CAUSE_NO_MS_CAUSE;

  pCtbNtry -> ti         = mncc_setup_ind -> ti;
  pCtbNtry -> rptInd     = mncc_setup_ind -> ri;
  pCtbNtry -> sigInf     = mncc_setup_ind -> sig;
  pCtbNtry -> clgPty     = mncc_setup_ind -> calling_party;
  pCtbNtry -> clgPtySub  = mncc_setup_ind -> calling_party_sub;

  /*
   * Assign called party 
   */
  pCtbNtry -> cldPty.ton = mncc_setup_ind -> called_party.ton;
  pCtbNtry -> cldPty.npi = mncc_setup_ind -> called_party.npi;
  pCtbNtry -> cldPty.c_called_num = 
    mncc_setup_ind -> called_party.c_called_num;
  if (pCtbNtry -> cldPty.called_num NEQ NULL)
  {
    ACI_MFREE (pCtbNtry -> cldPty.called_num);
    pCtbNtry -> cldPty.called_num = NULL;
  }
  if (pCtbNtry -> cldPty.c_called_num NEQ 0)
  {
    ACI_MALLOC (pCtbNtry -> cldPty.called_num, 
                pCtbNtry -> cldPty.c_called_num);
    memcpy (pCtbNtry -> cldPty.called_num, 
            mncc_setup_ind -> called_party.called_num,
            mncc_setup_ind -> called_party.c_called_num);
  }

  /*
   * Assign called party subaddress
   */
  pCtbNtry -> cldPtySub  = mncc_setup_ind -> called_party_sub;

  /* 
   * Assign redirecting party
   */
  pCtbNtry -> rdrPty.ton     = mncc_setup_ind -> redirecting_party.ton; 
  pCtbNtry -> rdrPty.npi     = mncc_setup_ind -> redirecting_party.npi;
  pCtbNtry -> rdrPty.present = mncc_setup_ind -> redirecting_party.present; 
  pCtbNtry -> rdrPty.screen  = mncc_setup_ind -> redirecting_party.screen;
  pCtbNtry -> rdrPty.c_redir_num = 
    mncc_setup_ind -> redirecting_party.c_redir_num;
  if (pCtbNtry -> rdrPty.redir_num NEQ NULL)
  {
    ACI_MFREE (pCtbNtry -> rdrPty.redir_num);
    pCtbNtry -> rdrPty.redir_num = NULL;
  }
  if (pCtbNtry -> rdrPty.c_redir_num NEQ 0)
  {
    ACI_MALLOC (pCtbNtry -> rdrPty.redir_num,
                pCtbNtry -> rdrPty.c_redir_num);
    memcpy (pCtbNtry -> rdrPty.redir_num, 
            mncc_setup_ind -> redirecting_party.redir_num,
            mncc_setup_ind -> redirecting_party.c_redir_num);
  }

  /* 
   * Assign redirecting party subaddress
   */
  pCtbNtry -> rdrPtySub.tos       = 
    mncc_setup_ind -> redirecting_party_sub.tos; 
  pCtbNtry -> rdrPtySub.odd_even  = 
    mncc_setup_ind -> redirecting_party_sub.odd_even;
  pCtbNtry -> rdrPtySub.c_subaddr = 
    mncc_setup_ind -> redirecting_party_sub.c_subaddr;
  if (pCtbNtry -> rdrPtySub.subaddr NEQ NULL)
  {
    ACI_MFREE (pCtbNtry -> rdrPtySub.subaddr);
    pCtbNtry -> rdrPtySub.subaddr = NULL;
  }
  if (pCtbNtry -> rdrPtySub.c_subaddr NEQ 0)
  {
    ACI_MALLOC (pCtbNtry -> rdrPtySub.subaddr, 
                pCtbNtry -> rdrPtySub.c_subaddr);
    memcpy (pCtbNtry -> rdrPtySub.subaddr, 
            mncc_setup_ind -> redirecting_party_sub.subaddr,
            mncc_setup_ind -> redirecting_party_sub.c_subaddr);
  }

  /* 
   * Assign bearer caps
   */  
  memcpy( &(pCtbNtry->BC[0]),&(mncc_setup_ind->bcpara),
          sizeof( T_MNCC_bcpara) );
  memcpy( &(pCtbNtry->BC[1]),&(mncc_setup_ind->bcpara2),
          sizeof( T_MNCC_bcpara) );

  psaCC_phbSrchNum( cId, CT_MTC );

  pCtbNtry -> calStat = CS_ACT_REQ;
  pCtbNtry -> calType = CT_MTC;

  pCtbNtry -> prio  = MNCC_PRIO_NORM_CALL;
  pCtbNtry -> curBC = 0;

  psaCC_send_satevent( EVENT_MT_CALL, cId, FAR_END, FALSE );

/*
 *-------------------------------------------------------------------
 * indicate alerting
 *-------------------------------------------------------------------
 */

  /* replace invalid/unknown sigInf by SIG_NOT_PRES */
  switch (pCtbNtry -> sigInf)
  {
    case MNCC_SIG_DIAL_TONE_ON:
    case MNCC_SIG_RING_BACK_TONE_ON:
    case MNCC_SIG_INT_TONE_ON:
    case MNCC_SIG_NET_CONG_TONE_ON:
    case MNCC_SIG_BUSY_TONE_ON:
    case MNCC_SIG_CONF_TONE_ON:
    case MNCC_SIG_ANS_TONE_ON:
    case MNCC_SIG_CALL_WAIT_TONE_ON:
    case MNCC_SIG_OFF_HOOK_WARN_TONE_ON:
    case MNCC_SIG_TONES_OFF:
    case MNCC_SIG_ALERT_OFF:
    case MNCC_SIG_NOT_PRES:
      break; /* leave as it is */
    default: /* replace unknown value by something well known */
      pCtbNtry -> sigInf = MNCC_SIG_NOT_PRES;
  }

  if( pCtbNtry -> sigInf NEQ MNCC_SIG_NOT_PRES OR
      ccShrdPrm.TCHasg EQ TRUE )
  {
    {
      /* indicate alerting to network */
      UBYTE ti = mncc_setup_ind -> ti;
      PREUSE (mncc_setup_ind, mncc_alert_req, MNCC_ALERT_REQ);
      mncc_alert_req -> ti    = ti;
      PSENDX (CC, mncc_alert_req);
    }

    cmhCC_IncomingCall (cId);    /* indicate an incoming call */
    return;
  }
  else
  {
    pCtbNtry -> alrtStat = AS_PND;
  }

  /*
   * check for progress descriptions
   */
  psaCC_chkPrgDesc ( cId, mncc_setup_ind -> progress_desc, MT_SETUP );

  /*
   * start call time measurements
   */
  aoc_info (cId, AOC_START_TIME);
/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (mncc_setup_ind);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCP                 |
|                                 ROUTINE : psa_mncc_setup_cnf      |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNCC_SETUP_CNF primitive send by CC.
            this terminates the call establishment request.

*/

GLOBAL void psa_mncc_setup_cnf
                                 ( T_MNCC_SETUP_CNF *mncc_setup_cnf )
{
  SHORT cId;                 /* holds call id */
  T_CC_CALL_TBL * pCtbNtry;  /* holds pointer to call table entry */
  int i;
  BOOL flag_redial = FALSE;

  TRACE_FUNCTION ("psa_mncc_setup_cnf()");

/*
 *-------------------------------------------------------------------
 * find call in call table
 *-------------------------------------------------------------------
 */
  cId = psaCC_ctbFindTi( mncc_setup_cnf -> ti );

  if( cId < 0 )
  {
   /*
    * ignore primitive, due to not found transaction identifier.
    */
    TRACE_EVENT ("primitive rejected due to unused ti");
    PFREE(mncc_setup_cnf);
    return;
  }

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */
  pCtbNtry = ccShrdPrm.ctb[cId];
  pCtbNtry -> curCs = mncc_setup_cnf -> cause;
  pCtbNtry -> rslt = mncc_setup_cnf -> cause;


  switch( mncc_setup_cnf -> cause )
  {
    case( MNCC_CAUSE_SUCCESS ):    /* successful establishment, call connected */

      /* 
       * MNCC_SETUP_CNF is received in call state other than CS_ACT_REQ
       * so reject the primitive.
      */
      if( pCtbNtry -> calStat NEQ CS_ACT_REQ)
      {
        TRACE_EVENT ("MNCC_SETUP_CNF is rejected as it is received in call state other than CS_ACT_REQ");
        break;
      }
      pCtbNtry -> clgPty.ton     = mncc_setup_cnf -> connected_number.ton;
      pCtbNtry -> clgPty.npi     = mncc_setup_cnf -> connected_number.npi;
      pCtbNtry -> clgPty.present = mncc_setup_cnf -> connected_number.present;
      pCtbNtry -> clgPty.screen  = mncc_setup_cnf -> connected_number.screen;
      pCtbNtry -> clgPty.c_num   = mncc_setup_cnf -> connected_number.c_num;

      memcpy( pCtbNtry -> clgPty.num,
              mncc_setup_cnf -> connected_number.num, MNCC_MAX_CC_CALLING_NUMBER );

      pCtbNtry -> clgPtySub.tos = mncc_setup_cnf -> connected_number_sub.tos;
      pCtbNtry -> clgPtySub.odd_even = mncc_setup_cnf -> connected_number_sub.odd_even;
      pCtbNtry -> clgPtySub.c_subaddr = mncc_setup_cnf -> connected_number_sub.c_subaddr;

      memcpy( pCtbNtry -> clgPtySub.subaddr,
              mncc_setup_cnf -> connected_number_sub.subaddr, MNCC_SUB_LENGTH );

      pCtbNtry -> inBndTns = FALSE;
      pCtbNtry -> calStat  = CS_ACT;

      /* check for progress descriptions */
      psaCC_chkPrgDesc ( cId, mncc_setup_cnf -> progress_desc, MT_CONN );


      if(pCtbNtry->calType EQ CT_MOC_RDL)
      {
        pCtbNtry->calType = CT_MOC;
        flag_redial = TRUE;
      }
      /* if redialling procedure was successful we finish redialling */
      if((rdlPrm.rdlMod EQ AUTOM_REPEAT_ON) AND flag_redial)
      {
        for(i = 0; i < CMD_SRC_MAX; i++)
        {
          R_AT(RAT_RDL, (T_ACI_CMD_SRC)i)(CALL_ATTEMPT_SUCCESSFUL); 
        }
        pCtbNtry->rdlCnt = 0;
        rdlPrm.rdlcId = NO_ENTRY;
        pCtbNtry->rdlTimIndex = RDL_TIM_INDEX_NOT_PRESENT;
        flag_redial = FALSE;
        /* Once we stop the redialling procedure
         * We ahev to stop as well the SAT max
         * duration timer (satshrdprm.dur)
         */
#ifdef SIM_TOOLKIT
        if(satShrdPrm.ownSAT)
        {
          TIMERSTOP(ACI_SAT_MAX_DUR_HND);
          satShrdPrm.dur  = -1;
        }
#endif /* SIM_TOOLKIT */
      }
    
      /* 
       * The MO call count is increment in MNCC_SETUP_REQ 
       * psaCC_chngCalTypCnt( cId, +1 );
       */

      cmhCC_CallConnected( cId );

      psaCC_send_satevent( EVENT_CALL_CONN, cId, FAR_END, FALSE );

      aoc_info (cId,AOC_CALL_CONNECTED);

      /* check for TTY service */
      cmhCC_TTY_Control ( cId, TTY_START );

      /* check for DTMF tones to send automatically */
      psaCC_DTMFSent ( cId );

#if defined (FF_WAP) || defined (FF_SAT_E)
      /* WAP STACK CALL is activ */
      if (Wap_Call)
        ccShrdPrm.wapStat = CC_WAP_STACK_UP;
#endif

      break;

    default:            /* unexpected result, assumed call disconnected */
      {
        TRACE_EVENT_P1("[ERR] UNEXP CAUSE IN SETUP CNF=%4x", mncc_setup_cnf -> cause);
      }
      /*FALLTHROUGH*/ /*lint -fallthrough*/

    /* unsuccessful establishment, call disconnected */
    case( MNCC_CAUSE_MS_TIMER ):

      pCtbNtry -> nrmCs = CAUSE_MAKE(DEFBY_CONDAT, ORIGSIDE_MS, MNCC_CC_ORIGINATING_ENTITY, NOT_PRESENT_8BIT);
      pCtbNtry -> rejCs = CAUSE_MAKE(DEFBY_CONDAT, ORIGSIDE_MS, MNCC_CC_ORIGINATING_ENTITY, NOT_PRESENT_8BIT);
#ifdef FF_EM_MODE
      em_relcs = MNCC_CAUSE_MS_TIMER;
#endif /* FF_EM_MODE */
      ccShrdPrm.cIdFail = cId;

      /* 
       * At this point the call has not yet gone completely always.
       * Call Control may not be in state U0, but in state U11, having
       * sent a DISCONNECT to the network, expecting the RELEASE
       * from the network or expiry of T308 in CC.
       * Implementation works, but it is not very nice here, releasing
       * the ti and decrementing the call count too early.
       */
      pCtbNtry -> calStat = CS_IDL;

      psaCC_chngCalTypCnt( cId, -1 );
      psaCC_retMOCTi( pCtbNtry -> ti );
      cmhCC_CallDisconnected (cId);
//TISH, patch for OMAPS00129162
//Don't free the ti here, MMI will receive release_ind later and release the ti there.
//start
//      psaCC_FreeCtbNtry (cId);
//end
      break;
  }
/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (mncc_setup_cnf);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCP                 |
|                                 ROUTINE : psa_mncc_setup_compl_ind|
+-------------------------------------------------------------------+

  PURPOSE : processes the MNCC_SETUP_COMPL_IND primitive send by CC.
            this terminates the call establishment of an incoming call.

*/

GLOBAL void psa_mncc_setup_compl_ind
                       ( T_MNCC_SETUP_COMPL_IND *mncc_setup_compl_ind )
{
  SHORT cId;                 /* holds call id */
  T_CC_CALL_TBL * pCtbNtry;  /* holds pointer to call table entry */

  TRACE_FUNCTION ("psa_mncc_setup_compl_ind()");

/*
 *-------------------------------------------------------------------
 * find call in call table
 *-------------------------------------------------------------------
 */
  cId = psaCC_ctbFindTi( mncc_setup_compl_ind -> ti );

  if( cId < 0 )
  {
   /*
    * ignore primitive, due to not found transaction identifier.
    */
    TRACE_EVENT ("primitive rejected due to unused ti");
    PFREE(mncc_setup_compl_ind);
    return;
  }

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */
  pCtbNtry = ccShrdPrm.ctb[cId];

  pCtbNtry -> rslt = mncc_setup_compl_ind -> cause;
  pCtbNtry -> curCs = mncc_setup_compl_ind -> cause;

  switch( mncc_setup_compl_ind -> cause )
  {
    case( MNCC_CAUSE_SUCCESS ):    /* successful establishment, call connected */

      pCtbNtry -> inBndTns = FALSE;
      pCtbNtry -> calStat  = CS_ACT;

      psaCC_setSpeechMode ();

      /* check for TTY service */
      cmhCC_TTY_Control ( cId, TTY_START );

      /*
       * Inform that a call goes to active state
       */
      psaCC_chngCalTypCnt( cId, +1 );

      cmhCC_CallConnected( cId );

      aoc_info (cId, AOC_CALL_CONNECTED);
      break;

    default:            /* unexpected result, assumed call disconnected */
      {
        TRACE_EVENT_P1("[ERR] UNEXP CAUSE IN SETUP CMPL IND=%4x", mncc_setup_compl_ind -> cause);
      }
      /*FALLTHROUGH*/ /*lint -fallthrough*/

    /* unsuccessful establishment, call disconnected */
    case( MNCC_CAUSE_MS_TIMER ):

      pCtbNtry -> nrmCs = CAUSE_MAKE(DEFBY_CONDAT, ORIGSIDE_MS, MNCC_CC_ORIGINATING_ENTITY, NOT_PRESENT_8BIT);
      pCtbNtry -> rejCs = CAUSE_MAKE(DEFBY_CONDAT, ORIGSIDE_MS, MNCC_CC_ORIGINATING_ENTITY, NOT_PRESENT_8BIT);
#ifdef FF_EM_MODE
      em_relcs = CAUSE_MAKE(DEFBY_CONDAT, ORIGSIDE_MS, MNCC_CC_ORIGINATING_ENTITY, NOT_PRESENT_8BIT);
#endif /* FF_EM_MODE */
      ccShrdPrm.cIdFail = cId;
      
      /*
       * What happens below is somewhat questionable.
       * ACI sets the call to CS_IDL and free the transaction identifier.
       * What happened is that on CC level, in state U8 CONNECT REQUEST the
       * CONNECT message sent by CC has not been answered by the network by a 
       * CONNECT ACNOWLEDGE message.
       * In this situation CC sends a DISCONNECT to the network and enters
       * state U11 DISCONNECT REQUEST. This means, the call state in CC is
       * still different from U0 NULL.
       * However, problems in the field are not expected here.
       */
      pCtbNtry -> calStat = CS_IDL;

      psaCC_chngCalTypCnt( cId, -1 );
      psaCC_retMOCTi( pCtbNtry -> ti );

      cmhCC_CallDisconnected (cId);
      cmhCC_CallReleased (cId);

      break;
  }

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (mncc_setup_compl_ind);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCP                 |
|                                 ROUTINE : psa_mncc_reject_ind     |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNCC_REJECT_IND primitive send by CC.
            this indicates a cause for rejecting a call establishment.

*/

GLOBAL void psa_mncc_reject_ind
                               ( T_MNCC_REJECT_IND *mncc_reject_ind )
{
  SHORT cId;                 /* holds call id */
  T_CC_CALL_TBL * pCtbNtry;  /* holds pointer to call table entry */

  TRACE_FUNCTION ("psa_mncc_reject_ind()");

/*
 *-------------------------------------------------------------------
 * find call in call table
 *-------------------------------------------------------------------
 */
  cId = psaCC_ctbFindTi( mncc_reject_ind -> ti );

  if( cId < 0 )
  {
   /*
    * ignore primitive, due to not found transaction identifier.
    */
    TRACE_EVENT ("primitive rejected due to unused ti");
    PFREE(mncc_reject_ind);
    return;
  }
  psaCC_StopDTMF (cId); /* HM 27.07.00*/

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */
  pCtbNtry = ccShrdPrm.ctb[cId];
  pCtbNtry -> curCs = mncc_reject_ind -> cause;

  pCtbNtry -> rslt  = CAUSE_MAKE(DEFBY_CONDAT, ORIGSIDE_MS, MNCC_CC_ORIGINATING_ENTITY, NOT_PRESENT_8BIT);
  pCtbNtry -> nrmCs = CAUSE_MAKE(DEFBY_CONDAT, ORIGSIDE_MS, MNCC_CC_ORIGINATING_ENTITY, NOT_PRESENT_8BIT);
  pCtbNtry -> rejCs = mncc_reject_ind -> cause;
  /* In this case, the cause values are internal and not sent by network */
  pCtbNtry -> numRawCauseBytes = 0; 

#ifdef FF_EM_MODE
  em_relcs = mncc_reject_ind -> cause;
#endif /* FF_EM_MODE */  

  if( GET_CAUSE_VALUE(pCtbNtry -> rejCs) NEQ NOT_PRESENT_8BIT ) ccShrdPrm.cIdFail = cId;

  pCtbNtry -> calStat = CS_IDL;

  psaCC_chngCalTypCnt( cId, -1 );
  psaCC_retMOCTi( pCtbNtry -> ti );

  /* monitoring for SAT radio-link failure */
  psaCC_send_satevent( EVENT_CALL_DISC, cId, NEAR_END, TRUE );

  /* check conditions for redialling */
  cmhCC_redialCheck(cId);

#ifdef SIM_TOOLKIT
        if( pCtbNtry->SATinv )
        {
          cmhSAT_NtwErr( ADD_NO_CAUSE );
          pCtbNtry->SATinv = FALSE;
        }
#endif
  /* cmhCC_CallDisconnected(cId);*/
  cmhCC_CPIrejectMsg(cId);
  cmhCC_CallReleased(cId);
  cmhCC_ChngWaitingToIncoming();

/* free the primitive buffer */
  PFREE (mncc_reject_ind);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCP                 |
|                                 ROUTINE : psa_mncc_release_ind    |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNCC_RELEASE_IND primitive send by CC.
  this indicates the release of a call.
  
*/

GLOBAL void psa_mncc_release_ind
( T_MNCC_RELEASE_IND *mncc_release_ind )
{
  SHORT cId;                 /* holds call id */
  T_CC_CALL_TBL * pCtbNtry;  /* holds pointer to call table entry */
  
  TRACE_FUNCTION ("psa_mncc_release_ind()");
  
  /*
  *-------------------------------------------------------------------
  * find call in call table
  *-------------------------------------------------------------------
  */
  cId = psaCC_ctbFindTi( mncc_release_ind -> ti );

    
  if( cId < 0 )
  {
    psaCC_retMOCTi( mncc_release_ind -> ti );
    
    /*
    * ignore primitive, due to not found transaction identifier.
    */
    TRACE_EVENT ("primitive rejected due to unused ti");
    PFREE(mncc_release_ind);
    return;
  }
  psaCC_StopDTMF ( cId ); /* HM 27.07.00 */
  
 /*
  *-------------------------------------------------------------------
  * update shared parameter and notify ACI
  *-------------------------------------------------------------------
  */
  pCtbNtry = ccShrdPrm.ctb[cId];
  
  pCtbNtry -> calStat = CS_DSC_REQ; /* Leave active state */
  pCtbNtry -> curCs = mncc_release_ind -> cause;
  pCtbNtry -> numRawCauseBytes = mncc_release_ind -> c_raw_cause;
  pCtbNtry -> rawCauseBytes    = mncc_release_ind -> raw_cause;

  psaCC_setSpeechMode ();
  
  /* check for TTY service */
  /*cmhCC_TTY_Control ( cId );*/
  
  if (pCtbNtry -> nrmCs EQ MNCC_CAUSE_NO_MS_CAUSE)    /* no Disconnect Cause available...*/
  {
    pCtbNtry -> nrmCs= mncc_release_ind -> cause; /* Check ### */
    ccShrdPrm.cIdFail = cId; /* Check ### */
#ifdef FF_EM_MODE
    em_relcs = mncc_release_ind -> cause; /* Check ### */
#endif /* FF_EM_MODE */  
  }
  
  pCtbNtry -> rejCs = CAUSE_MAKE(DEFBY_CONDAT, ORIGSIDE_MS, MNCC_CC_ORIGINATING_ENTITY, NOT_PRESENT_8BIT);
  pCtbNtry -> rslt  = CAUSE_MAKE(DEFBY_CONDAT, ORIGSIDE_MS, MNCC_CC_ORIGINATING_ENTITY, NOT_PRESENT_8BIT);
  
  psaCC_chngCalTypCnt( cId, -1 );
  psaCC_retMOCTi( pCtbNtry -> ti );
  
  psaCC_send_satevent( EVENT_CALL_DISC, cId, FAR_END, TRUE );

  pCtbNtry -> numRawCauseBytes = 0; /* Reset the cause value after sending the info to SAT */

  cmhCC_redialCheck(cId);
  
  /*
  if( pCtbNtry -> CCBSstat EQ CCBSS_PSSBL OR
  pCtbNtry -> calType  EQ CT_NI_MOC      )
  */
  switch (pCtbNtry->curCmd)
  {
  case( AT_CMD_A ):
  case( AT_CMD_D ):
  case( AT_CMD_NONE ):
  case( AT_CMD_ABRT ):
    {
      cmhCC_CallDisconnected(cId);
      cmhCC_CallReleased( cId );
    }
    break;
    
  case( AT_CMD_H ):
  case( AT_CMD_Z ):
  case( AT_CMD_CHUP ):
  case( AT_CMD_CHLD ):
    
#ifdef FF_FAX
  case( AT_CMD_FKS ):
  case( AT_CMD_FDT ):
  case( AT_CMD_FDR ):
#endif
    {
      cmhCC_CallDisconnected(cId);
      cmhCC_CPIReleaseMsg(cId);
    }
    break;
  }

  if (rdlPrm.rdlcId NEQ cId)
  {/* if redialling is active no clean cId entry */
    /*patch BE 30.06.00 to remove every other call drops*/
    psaCC_FreeCtbNtry (cId);
    /*end patch BE */
  }
  cmhCC_ChngWaitingToIncoming();
  /* free the primitive buffer */
  PFREE (mncc_release_ind);
  
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCP                 |
|                                 ROUTINE : psa_mncc_release_cnf    |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNCC_RELEASE_CNF primitive send by CC.
            this confirmates the release request for a call.

*/

GLOBAL void psa_mncc_release_cnf
                             ( T_MNCC_RELEASE_CNF *mncc_release_cnf )
{
  SHORT cId;                 /* holds call id */
  T_CC_CALL_TBL * pCtbNtry;  /* holds pointer to call table entry */

  TRACE_FUNCTION ("psa_mncc_release_cnf()");

/*
 *-------------------------------------------------------------------
 * find call in call table
 *-------------------------------------------------------------------
 */
  cId = psaCC_ctbFindTi( mncc_release_cnf -> ti );

  if( cId < 0 )
  {
    psaCC_retMOCTi( mncc_release_cnf -> ti );

   /*
    * ignore primitive, due to not found transaction identifier.
    */
    TRACE_EVENT ("primitive rejected due to unused ti");
    PFREE(mncc_release_cnf);
    return;
  }

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */
  pCtbNtry = ccShrdPrm.ctb[cId];

  /* HM 02-Aug-2002, done while merging SBK code, check again this code*/
#if 0 /* Old code */
  if( mncc_release_cnf -> cause NEQ CAUSE_NOT_PRES )
  {
    pCtbNtry -> nrmCs = mncc_release_cnf -> cause;
    ccShrdPrm.cIdFail = cId;
#ifdef FF_EM_MODE
    em_relcs = mncc_release_cnf -> cause;
#endif /* FF_EM_MODE */  
  }
#endif /* #if 0 */

  if (pCtbNtry -> nrmCs EQ MNCC_CAUSE_NO_MS_CAUSE)    /* no Disconnect Cause available...*/
  {
    pCtbNtry -> nrmCs = mncc_release_cnf -> cause; /* Recheck ###*/
    ccShrdPrm.cIdFail = cId;  /* This line is considered harmful, only if cause present ### */
#ifdef FF_EM_MODE
    em_relcs = mncc_release_cnf -> cause;  /* Recheck ### */
#endif /* FF_EM_MODE */  
  }
  pCtbNtry -> rslt  = CAUSE_MAKE(DEFBY_CONDAT, ORIGSIDE_MS, MNCC_CC_ORIGINATING_ENTITY, NOT_PRESENT_8BIT);
  pCtbNtry -> rejCs = CAUSE_MAKE(DEFBY_CONDAT, ORIGSIDE_MS, MNCC_CC_ORIGINATING_ENTITY, NOT_PRESENT_8BIT);
  pCtbNtry -> curCs = pCtbNtry -> rejCs;
  pCtbNtry -> numRawCauseBytes = mncc_release_cnf -> c_raw_cause;
  pCtbNtry -> rawCauseBytes    = mncc_release_cnf -> raw_cause;
#ifdef SIM_TOOLKIT
  if(!(pCtbNtry -> SATinv & SAT_REDIAL))
#endif
    pCtbNtry -> calStat = CS_IDL;

  psaCC_chngCalTypCnt( cId, -1 );
  psaCC_retMOCTi( pCtbNtry -> ti );

  psaCC_send_satevent( EVENT_CALL_DISC, cId , FAR_END, TRUE );

  pCtbNtry -> numRawCauseBytes = 0; /* Reset the cause value after sending the info to SAT */

  cmhCC_redialCheck( cId);

  cmhCC_CallReleased (cId);

  /*patch BE 30.06.00 to remove every other call drops */
  {
#ifdef SIM_TOOLKIT
    if ( (pCtbNtry -> SATinv & SAT_REDIAL) EQ 0 )
    {
#endif  /* SIM_TOOLKIT */
      if (rdlPrm.rdlcId NEQ cId)
      {
        psaCC_FreeCtbNtry (cId);
      }
#ifdef SIM_TOOLKIT
    }
#endif  /* SIM_TOOLKIT */
  }
  /*end patch BE */

  switch(CHLDaddInfo)
  {
    case (CHLD_ADD_INFO_DIAL_CAL):
      cId = psaCC_ctbFindCall( OWN_SRC_INV, NO_VLD_CS, CT_MOC );
      if (cId NEQ NO_ENTRY)
      {
        cmhCC_NewCall(cId,(T_ACI_CMD_SRC)psaCC_ctb(cId)->curSrc,AT_CMD_D);
      } 
      break;
    default:
      break;
  }
  cmhCC_ChngWaitingToIncoming();
  /* free the primitive buffer */
  PFREE (mncc_release_cnf);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCP                 |
|                                 ROUTINE : psa_mncc_disconnect_ind |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNCC_DISCONNECT_IND primitive send by CC.
            this indicates the disconnection of a call.

*/

GLOBAL void psa_mncc_disconnect_ind
                       ( T_MNCC_DISCONNECT_IND *mncc_disconnect_ind )
{
  SHORT cId;                 /* holds call id */
  T_CC_CALL_TBL * pCtbNtry;  /* holds pointer to call table entry */
  BOOL held_call = FALSE;

  TRACE_FUNCTION ("psa_mncc_disconnect_ind()");

/*
 *-------------------------------------------------------------------
 * find call in call table
 *-------------------------------------------------------------------
 */
  cId = psaCC_ctbFindTi( mncc_disconnect_ind -> ti );

  if( cId < 0 )
  {
   /*
    * ignore primitive, due to not found transaction identifier.
    */
    TRACE_EVENT ("primitive rejected due to unused ti");
    PFREE(mncc_disconnect_ind);
    return;
  }
  psaCC_StopDTMF ( cId ); /* HM 27.07.00 */

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */
  pCtbNtry = ccShrdPrm.ctb[cId];

  pCtbNtry -> curCs = mncc_disconnect_ind -> cause;

  if( mncc_disconnect_ind->diagnostic EQ MNCC_DIAG_CCBS_POSSIBLE )
  {
    pCtbNtry -> CCBSstat = CCBSS_PSSBL;
  }

  /* CQ 23619: copy ss diagnostic to shared parameters >> */
  pCtbNtry->ssDiag = mncc_disconnect_ind->ss_diag;
  /* CQ 23619 << */
  
#if 0
 if( mncc_disconnect_ind -> cause NEQ CAUSE_NOT_PRES )
  {
    pCtbNtry -> nrmCs = mncc_disconnect_ind -> cause;
    ccShrdPrm.cIdFail = cId;
#ifdef FF_EM_MODE
    em_relcs = mncc_disconnect_ind -> cause;
#endif /* FF_EM_MODE */
  }
#endif 

  pCtbNtry -> nrmCs = mncc_disconnect_ind -> cause; /* Recheck ### */
  ccShrdPrm.cIdFail = cId; /* Recheck ### */
#ifdef FF_EM_MODE
  em_relcs = mncc_disconnect_ind -> cause; /* Recheck ### */
#endif /* FF_EM_MODE */

  /* remember if call is currently on hold */
  if (pCtbNtry -> calStat EQ CS_HLD)
    held_call = TRUE;
  
  /* calStat has to be set before calling psaCC_chkPrgDesc, otherwise
     no CPI event will be send */
  pCtbNtry -> calStat = CS_DSC_REQ; 

  /* check for TTY service */
  /* cmhCC_TTY_Control ( cId ); */

  /* check for progress descriptions */
  psaCC_chkPrgDesc ( cId, mncc_disconnect_ind -> progress_desc, MT_DISC );

  pCtbNtry -> rslt  = CAUSE_MAKE(DEFBY_CONDAT, ORIGSIDE_MS, MNCC_CC_ORIGINATING_ENTITY, NOT_PRESENT_8BIT);
  pCtbNtry -> rejCs = CAUSE_MAKE(DEFBY_CONDAT, ORIGSIDE_MS, MNCC_CC_ORIGINATING_ENTITY, NOT_PRESENT_8BIT);

  pCtbNtry -> numRawCauseBytes = mncc_disconnect_ind -> c_raw_cause;
  pCtbNtry -> rawCauseBytes    = mncc_disconnect_ind -> raw_cause;

  psaCC_send_satevent( EVENT_CALL_DISC, cId , FAR_END, TRUE );

  pCtbNtry -> numRawCauseBytes = 0; /* Reset the cause value after sending the info to SAT */

  /* If call was held upon mncc_disconnect_ind it is cleared anyway */
  if (held_call EQ TRUE)
  {
    TRACE_EVENT ("Release a held call");
    cmhCC_CallDisconnected (cId);
    psaCC_ClearCall (cId);
  }
  else
  {
    switch( cmhCC_getcalltype(cId) )
    {
      case( VOICE_CALL ):
      {
        /*
         * if in-band tones are available and TCH is assigned, request to disconnect the call
         * GSM 04.08/5.4.4.1.1.1
         */
        /*
          TRACE_EVENT("psa_mncc_disconnect_ind(): --> VOICE CALL");
          if ((pCtbNtry -> inBndTns EQ TRUE) 
          AND (pCtbNtry -> CCBSstat EQ NO_VLD_CCBSS)
          AND (ccShrdPrm.TCHasg EQ TRUE)) 
          {
            cmhCC_DisconnectCall (cId);
          }
          else
          {
            cmhCC_CallDisconnected (cId);
          }
        */
         /* check conditions for redialling */
         if((pCtbNtry->curCmd NEQ AT_CMD_H) AND
            (pCtbNtry->curCmd NEQ AT_CMD_CHUP))
         {
           cmhCC_redialCheck(cId);
         }


        /*
         * for CCBS support the "Prolonged Clearing Procedure"
         * GSM 04.08/5.4.4.2.1
         */
        cmhCC_CallDisconnected (cId);

        if ((pCtbNtry->inBndTns EQ TRUE)                  /* 5.4.4.2.1.1 */
          AND (pCtbNtry->CCBSstat EQ NO_VLD_CCBSS))
        {
          if (ccShrdPrm.TCHasg NEQ TRUE)
          {
            TRACE_EVENT ("4.08/5.4.4.2.1.1 i)");
            psaCC_ClearCall (cId);
          }
          else
          {
            TRACE_EVENT ("4.08/5.4.4.2.1.1 ii)");
            psaCC_setSpeechMode();
          }
        }
        else if (pCtbNtry->CCBSstat EQ CCBSS_PSSBL)       /* 5.4.4.2.2.1 */
        {
          TRACE_EVENT ("4.08/5.4.4.2.2.1");
          psaCC_setSpeechMode();
          cmhCC_SndDiscRsn( cId );                        /* FIXME: to emmit BUSY, but this is not good here
                                                             since BUSY is final result code and will be
                                                             emitted on Release again !!!!! */
        }
        else if ((pCtbNtry->inBndTns NEQ TRUE)            /* 5.4.4.2.3.1 */
          AND (pCtbNtry->CCBSstat EQ NO_VLD_CCBSS))
        {
          TRACE_EVENT ("4.08/5.4.4.2.3.1");
          psaCC_ClearCall (cId);
        }
        else                                              /* 5.4.4.2.3.2 */
        {
          TRACE_EVENT ("4.08/5.4.4.2.3.2");
          psaCC_ClearCall (cId);
        }
        break;
      }
      default: /* all non voice calls */
      {
        /*
         * see cmh_cc.h  T_CC_CALL_TYPE for all other call types
         */
        TRACE_EVENT("psa_mncc_disconnect_ind(): --> DATA CALL");
        cmhCC_CallDisconnected (cId);
        psaCC_ClearCall (cId);
        break;
      }
    }
  } /* else */

  /* free the primitive buffer */
  PFREE (mncc_disconnect_ind);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCP                 |
|                                 ROUTINE : psa_mncc_alert_ind      |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNCC_ALERT_IND primitive send by CC.
            this indicates that the called party is alerted.

*/

GLOBAL void psa_mncc_alert_ind
                             ( T_MNCC_ALERT_IND *mncc_alert_ind )
{
  SHORT cId;                 /* holds call id */

  TRACE_FUNCTION ("psa_mncc_alert_ind()");

/*
 *-------------------------------------------------------------------
 * find call in call table
 *-------------------------------------------------------------------
 */
  cId = psaCC_ctbFindTi( mncc_alert_ind -> ti );

  if( cId < 0 )
  {
   /*
    * ignore primitive, due to not found transaction identifier.
    */
    TRACE_EVENT ("primitive rejected due to unused ti");
    PFREE(mncc_alert_ind);
    return;
  }

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */
  psaCC_ctb(cId)->alrtStat = AS_SND;
  psaCC_ctb(cId)->curCs = MNCC_CAUSE_NO_MS_CAUSE;

  /* check for progress descriptions */
  psaCC_chkPrgDesc ( cId, mncc_alert_ind -> progress_desc, MT_ALRT );

  /* check for TTY service */
  cmhCC_TTY_Control ( cId, TTY_START );
/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (mncc_alert_ind);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCP                 |
|                                 ROUTINE : psa_mncc_call_proceed_ind|
+-------------------------------------------------------------------+

  PURPOSE : processes the MNCC_CALL_PROCEED_IND primitive send by CC.
            this indicates a call proceeding information for a call.

*/

GLOBAL void psa_mncc_call_proceed_ind
                   ( T_MNCC_CALL_PROCEED_IND *mncc_call_proceed_ind )
{
  SHORT cId;                 /* holds call id */
  T_CC_CALL_TBL * pCtbNtry;  /* holds pointer to call table entry */

  TRACE_FUNCTION ("psa_mncc_call_proceed_ind()");

/*
 *-------------------------------------------------------------------
 * find call in call table
 *-------------------------------------------------------------------
 */
  cId = psaCC_ctbFindTi( mncc_call_proceed_ind -> ti );

  if( cId < 0 )
  {
   /*
    * ignore primitive, due to not found transaction identifier.
    */
    TRACE_EVENT ("primitive rejected due to unused ti");
    PFREE(mncc_call_proceed_ind);
    return;
  }

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */
  pCtbNtry = ccShrdPrm.ctb[cId];
  pCtbNtry -> curCs = MNCC_CAUSE_NO_MS_CAUSE;

  memcpy( &(pCtbNtry->BC[0]),&(mncc_call_proceed_ind->bcpara),
          sizeof( T_MNCC_bcpara) );
  memcpy( &(pCtbNtry->BC[1]),&(mncc_call_proceed_ind->bcpara2),
          sizeof( T_MNCC_bcpara) );
  pCtbNtry -> rptInd = mncc_call_proceed_ind -> ri;

  /* check for progress descriptions */
  psaCC_chkPrgDesc ( cId, mncc_call_proceed_ind -> progress_desc, MT_PROC );

  /* check for TTY service */
  cmhCC_TTY_Control ( cId, TTY_START );
/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (mncc_call_proceed_ind);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCP                 |
|                                 ROUTINE : psa_mncc_progress_ind   |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNCC_PROGRESS_IND primitive send by CC.
            this indicates a progress description for a call.

*/

GLOBAL void psa_mncc_progress_ind
                          ( T_MNCC_PROGRESS_IND *mncc_progress_ind )
{
  SHORT cId;                 /* holds call id */

  TRACE_FUNCTION ("psa_mncc_progress_ind()");

/*
 *-------------------------------------------------------------------
 * find call in call table
 *-------------------------------------------------------------------
 */
  cId = psaCC_ctbFindTi( mncc_progress_ind -> ti );


  if( cId < 0 )
  {
   /*
    * ignore primitive, due to not found transaction identifier.
    */
    TRACE_EVENT ("primitive rejected due to unused ti");
    PFREE(mncc_progress_ind);
    return;
  }
  psaCC_ctb(cId)->curCs = MNCC_CAUSE_NO_MS_CAUSE;

  /*
   * check for progress descriptions
   */
  psaCC_chkPrgDesc ( cId, mncc_progress_ind -> progress_desc, MT_PROGR );

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (mncc_progress_ind);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCP                 |
|                                 ROUTINE : psa_mncc_hold_cnf       |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNCC_HOLD_CNF primitive send by CC.
            this confirmates that a call is being held.

*/

GLOBAL void psa_mncc_hold_cnf
                                ( T_MNCC_HOLD_CNF *mncc_hold_cnf )
{
  SHORT cId;                 /* holds call id */
  T_CC_CALL_TBL * pCtbNtry;  /* holds pointer to call table entry */

  TRACE_FUNCTION ("psa_mncc_hold_cnf()");

/*
 *-------------------------------------------------------------------
 * find call in call table
 *-------------------------------------------------------------------
 */
  cId = psaCC_ctbFindTi( mncc_hold_cnf -> ti );

  if( cId < 0 )
  {
   /*
    * ignore primitive, due to not found transaction identifier.
    */
    TRACE_EVENT ("primitive rejected due to unused ti");
    PFREE(mncc_hold_cnf);
    return;
  }

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */
  pCtbNtry = ccShrdPrm.ctb[cId];

  switch( mncc_hold_cnf -> cause )
  {
    case( MNCC_CAUSE_HOLD_SUCCESS ):    /* successful, call held */
    case( MNCC_CAUSE_SUCCESS ):
    case( MNCC_CAUSE_NO_MS_CAUSE ):

      pCtbNtry -> calStat = CS_HLD;

      psaCC_setSpeechMode ();

      /* check for TTY service */
      cmhCC_TTY_Control ( cId, TTY_PAUSE );
      break;

    default:    /* unsuccessful, call state unchanged */

      ccShrdPrm.cIdFail   = cId;
      pCtbNtry -> calStat = CS_ACT;
      break;
  }

  pCtbNtry -> rslt = mncc_hold_cnf -> cause;

  cmhCC_CallHeld(cId);

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (mncc_hold_cnf);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCP                 |
|                                 ROUTINE : psa_mncc_retrieve_cnf   |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNCC_RETRIEVE_CNF primitive send by CC.
            this confirmates that a call is retrieved.

*/

GLOBAL void psa_mncc_retrieve_cnf
                           ( T_MNCC_RETRIEVE_CNF *mncc_retrieve_cnf )
{
  SHORT cId;                 /* holds call id */
  T_CC_CALL_TBL * pCtbNtry;  /* holds pointer to call table entry */

  TRACE_FUNCTION ("psa_mncc_retrieve_cnf()");

/*
 *-------------------------------------------------------------------
 * find call in call table
 *-------------------------------------------------------------------
 */
  cId = psaCC_ctbFindTi( mncc_retrieve_cnf -> ti );

  if( cId < 0 )
  {
   /*
    * ignore primitive, due to not found transaction identifier.
    */
    TRACE_EVENT ("primitive rejected due to unused ti");
    PFREE(mncc_retrieve_cnf);
    return;
  }

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */
  pCtbNtry = ccShrdPrm.ctb[cId];

  switch( mncc_retrieve_cnf -> cause )
  {
    case( MNCC_CAUSE_RETRIEVE_SUCCESS ):    /* successful, call active */

      pCtbNtry -> calStat = CS_ACT;

      /* switch on vocoder */
      psaCC_setSpeechMode ();

      /* check for TTY service */
      cmhCC_TTY_Control ( cId, TTY_START );
      break;

    default:    /* unsuccessful, call state unchanged */

      ccShrdPrm.cIdFail   = cId;
      pCtbNtry -> calStat = CS_HLD;
      break;
  }

  pCtbNtry -> rslt = mncc_retrieve_cnf -> cause;

  cmhCC_CallRetrieved (cId);

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (mncc_retrieve_cnf);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCP                 |
|                                 ROUTINE : psa_mncc_sync_ind       |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNCC_SYNC_IND primitive send by CC.
            this indicates the change of the channel mode.

*/

GLOBAL void psa_mncc_sync_ind( T_MNCC_SYNC_IND *mncc_sync_ind )
{
  SHORT cId = 0;                /* holds call id */

  TRACE_FUNCTION ("psa_mncc_sync_ind()");

  /*
  cause         cause 4.6   UBYTE
  channel mode  chm     4.14  STRUCT

  ch_type 0 CH_SDCCH  SDCCH channel
            1 CH_TCH_F  TCH Fullrate
            2 CH_TCH_H  TCH Halfrate

  ch_mode 0 CHM_SIG_ONLY  signalling only
            1 CHM_SPEECH      speech full rate or half rate version 1
            33  CHM_SPEECH_V2 speech full rate or half rate version 2
            65  CHM_SPEECH_V3 speech full rate or half rate version 3
            3 CHM_DATA_9_6  data 9.6 kBit/s
            11  CHM_DATA_4_8  data 4.8 kBit/s
            19  CHM_DATA_2_4  data 2.4 kBit/s
            15  CHM_DATA_14_4 data 14.4 kBit/s
  */

  TRACE_EVENT_P4("MNCC_SYNC_IND ti: %d, cause: %d, ch_type: %d, ch_mode: %d",
                 mncc_sync_ind -> ti,
                 mncc_sync_ind -> cause,
                 mncc_sync_ind -> ch_info.ch_type,
                 mncc_sync_ind -> ch_info.ch_mode);

  /* update shared parameter */
  ccShrdPrm.syncCs = mncc_sync_ind -> cause;
#ifdef FF_EM_MODE
  em_relcs = mncc_sync_ind -> cause;
#endif /* FF_EM_MODE */

  /* Inform Advice of Charge Module */
  switch (mncc_sync_ind->cause)
  {
    case MNCC_CAUSE_REEST_STARTED:
      for( cId = 0; cId < MAX_CALL_NR; cId++ )
        aoc_info (cId, AOC_SUSPEND_AOC);
      break;
    case MNCC_CAUSE_REEST_FINISHED:
      ccShrdPrm.aocRsmpPend = 1;
      break;
  }

  /* switch channel mode */
  if( mncc_sync_ind -> ch_info.ch_mode NEQ NOT_PRESENT_8BIT )
  {
    ccShrdPrm.chMod   = mncc_sync_ind -> ch_info.ch_mode;
    ccShrdPrm.chType  = mncc_sync_ind -> ch_info.ch_type;
    ccShrdPrm.TCHasg  = TRUE;
  }
  
    if ( ccShrdPrm.aocRsmpPend AND
          (ccShrdPrm.chMod EQ MNCC_CHM_SPEECH)    OR
          (ccShrdPrm.chMod EQ MNCC_CHM_SPEECH_V2) OR
          (ccShrdPrm.chMod EQ MNCC_CHM_SPEECH_V3)    )
    {
      ccShrdPrm.aocRsmpPend = 0;
      for( cId = 0; cId < MAX_CALL_NR; cId++ )
        aoc_info (cId, AOC_RESUME_AOC);
    }

if( mncc_sync_ind -> ch_info.ch_mode NEQ NOT_PRESENT_8BIT )
  {
    /* service a pending alert */
    for( cId = 0; cId < MAX_CALL_NR; cId++ )
    {
      if (ccShrdPrm.ctb[cId] NEQ NULL)
      {
        psaCC_ctb(cId)->curCs = mncc_sync_ind -> cause;

        if( psaCC_ctb(cId)->alrtStat EQ AS_PND AND
            psaCC_ctb(cId)->calStat  EQ CS_ACT_REQ )
        {
          {
            /* indicate alerting to network */
            PALLOC (mncc_alert_req, MNCC_ALERT_REQ);
            mncc_alert_req -> ti    = psaCC_ctb(cId)->ti;
            PSENDX (CC, mncc_alert_req);
            psaCC_ctb(cId)->alrtStat = AS_SND;
          }
          cmhCC_IncomingCall (cId); /* indicate an incoming call */
        }
        if (psaCC_ctb(cId)->calStat NEQ CS_HLD AND
            psaCC_ctb(cId)->calStat NEQ CS_IDL)
        {
          /* check for TTY service */
          cmhCC_TTY_Control ( cId, TTY_TCH );
        }
      }
    }
  }
psaCC_setSpeechMode ();

  /* Remember cause for Call Reestablishment*/

  if (mncc_sync_ind -> cause EQ MNCC_CAUSE_REEST_FINISHED 
    OR mncc_sync_ind -> cause EQ MNCC_CAUSE_REEST_STARTED)
  {
    for( cId = 0; cId < MAX_CALL_NR; cId++ )
    {
      if (ccShrdPrm.ctb[cId] NEQ NULL)
      {
        psaCC_ctb(cId)->curCs = mncc_sync_ind -> cause;
      }
    }
   }

  cId = psaCC_ctbFindTi( mncc_sync_ind -> ti );

  /* For channel synchronization events the cId needs not to be valid here */
  cmhCC_Synchronized( cId );


  /* free the primitive buffer */
  PFREE (mncc_sync_ind);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCP                 |
|                                 ROUTINE : psa_mncc_user_ind       |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNCC_USER_IND primitive send by CC.
            this indicates the receiving of user data.
*/

GLOBAL void psa_mncc_user_ind( T_MNCC_USER_IND *mncc_user_ind )
{
  TRACE_FUNCTION ("psa_mncc_user_ind()");

  PFREE (mncc_user_ind);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCP                 |
|                                 ROUTINE : psa_mncc_start_dtmf_cnf |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNCC_START_DTMF_CNF primitive send by CC.
            this is a confirmation for the sent DTMF tones.

*/

GLOBAL void psa_mncc_start_dtmf_cnf
                       ( T_MNCC_START_DTMF_CNF *mncc_start_dtmf_cnf )
{
  SHORT cId;                 /* holds call id */

  TRACE_FUNCTION ("psa_mncc_start_dtmf_cnf()");

/*
 *-------------------------------------------------------------------
 * find call in call table
 *-------------------------------------------------------------------
 */
  cId = psaCC_ctbFindTi( mncc_start_dtmf_cnf -> ti );

  if( cId < 0 )
  {
   /*
    * ignore primitive, due to not found transaction identifier.
    */
    TRACE_EVENT ("primitive rejected due to unused ti");
    PFREE(mncc_start_dtmf_cnf);
    return;
  }

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */
  psaCC_ctb(cId)->nrmCs = mncc_start_dtmf_cnf -> cause;
  if ((mncc_start_dtmf_cnf->cause EQ MNCC_CAUSE_DTMF_STOP_SUCCESS) AND
        (mncc_start_dtmf_cnf->dtmf_mod EQ MNCC_DTMF_MOD_MAN_STOP))
  {
    /* Reset the current dtmf digit */
    ccShrdPrm.dtmf.cur = 0;
  }
#ifdef FF_EM_MODE
  em_relcs = mncc_start_dtmf_cnf -> cause;
#endif /* FF_EM_MODE */

  psaCC_DTMFSent( cId );

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (mncc_start_dtmf_cnf);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCP                 |
|                                 ROUTINE : psa_mncc_modify_ind     |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNCC_MODIFY_IND primitive send by CC.
            this indicates a mobile terminated in-call modification.

*/

GLOBAL void psa_mncc_modify_ind
                               ( T_MNCC_MODIFY_IND *mncc_modify_ind )
{
  SHORT cId;                 /* holds call id */
  T_CC_CALL_TBL * pCtbNtry;  /* holds pointer to call table entry */

  TRACE_FUNCTION ("psa_mncc_modify_ind()");

/*
 *-------------------------------------------------------------------
 * find call in call table
 *-------------------------------------------------------------------
 */
  cId = psaCC_ctbFindTi( mncc_modify_ind -> ti );

  if( cId < 0 )
  {
   /*
    * ignore primitive, due to not found transaction identifier.
    */
    TRACE_EVENT ("primitive rejected due to unused ti");
    PFREE(mncc_modify_ind);
    return;
  }

  /* check for TTY service */
  cmhCC_TTY_Control ( cId, TTY_PAUSE );

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */
  pCtbNtry = ccShrdPrm.ctb[cId];

  pCtbNtry -> curBC = (pCtbNtry -> curBC EQ 0)?1:0;
  pCtbNtry -> rslt  = MNCC_CAUSE_MODIFY_SUCCESS; /* SBK-02-07-30: possibly to be revised as strictly speaking ACI is orignating entity */

  cmhCC_CallModified (cId);

  /* check for TTY service */
  cmhCC_TTY_Control ( cId, TTY_START );

  psaCC_StopDTMF ( cId );
/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (mncc_modify_ind);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCP                 |
|                                 ROUTINE : psa_mncc_modify_cnf     |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNCC_MODIFY_CNF primitive send by CC.
            this indicates the result for a mobile originated in-call
            modification.

*/

GLOBAL void psa_mncc_modify_cnf
                               ( T_MNCC_MODIFY_CNF *mncc_modify_cnf )
{
  SHORT cId;                 /* holds call id */
  T_CC_CALL_TBL * pCtbNtry;  /* holds pointer to call table entry */

  TRACE_FUNCTION ("psa_mncc_modify_cnf()");

/*
 *-------------------------------------------------------------------
 * find call in call table
 *-------------------------------------------------------------------
 */
  cId = psaCC_ctbFindTi( mncc_modify_cnf -> ti );

  if( cId < 0 )
  {
   /*
    * ignore primitive, due to not found transaction identifier.
    */
    TRACE_EVENT ("primitive rejected due to unused ti");
    PFREE(mncc_modify_cnf);
    return;
  }

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */
  pCtbNtry = ccShrdPrm.ctb[cId];

  pCtbNtry -> rslt = mncc_modify_cnf -> cause;

  switch( mncc_modify_cnf -> cause )
  {
    case( MNCC_CAUSE_MODIFY_SUCCESS ):    /* successful modification */

      pCtbNtry -> curBC = (pCtbNtry -> curBC EQ 0)?1:0;
      break;

    default:    /* unsuccessful modification */

      ccShrdPrm.cIdFail = cId;
      break;
  }
  psaCC_ctb(cId)->calStat = CS_ACT;

  cmhCC_CallModified (cId);

  /* check for TTY service */
  cmhCC_TTY_Control ( cId, TTY_START );

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (mncc_modify_cnf);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCP                 |
|                                 ROUTINE : psa_mncc_bearer_cap_cnf |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNCC_BEARER_CAP_CNF primitive send by CC.
            this is the result of the requested bearer coding/decoding.

*/

GLOBAL void psa_mncc_bearer_cap_cnf
                         ( T_MNCC_BEARER_CAP_CNF *mncc_bearer_cap_cnf )
{

  TRACE_FUNCTION ("psa_mncc_bearer_cap_cnf()");

/*
 *-------------------------------------------------------------------
 * determine request id
 *-------------------------------------------------------------------
 */
  switch( mncc_bearer_cap_cnf -> req_id )
  {

#ifdef SIM_TOOLKIT

  case( BCRI_SAT ):

    switch( mncc_bearer_cap_cnf -> bc_mod )
    {
      case( MNCC_BC_MOD_CODE ):

        cmhSAT_ResCapCode ( mncc_bearer_cap_cnf -> cause,
                            &mncc_bearer_cap_cnf -> bcconf,
                            &mncc_bearer_cap_cnf -> bcconf2);

        break;

      case( MNCC_BC_MOD_DECODE ):

         cmhSAT_ResCapDecode ( mncc_bearer_cap_cnf -> cause,
                              &mncc_bearer_cap_cnf -> bcpara,
                              &mncc_bearer_cap_cnf -> bcpara2 );

        break;
    }
    break;

#endif
  }

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (mncc_bearer_cap_cnf);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCP                 |
|                                 ROUTINE : psa_mncc_prompt_ind     |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNCC_PROMPT_IND primitive send by CC.
            this requests a transaction identifier for a CCBS recall.

*/

GLOBAL void psa_mncc_prompt_ind
                         ( T_MNCC_PROMPT_IND *mncc_prompt_ind )
{
  SHORT ti;

  TRACE_FUNCTION ("psa_mncc_prompt_ind()");

/*
 *-------------------------------------------------------------------
 * allocate transaction identifier
 *-------------------------------------------------------------------
 */
  ti = psaCC_getMOCTi( -1 );

  if( ti NEQ -1 )
  {
    PALLOC (mncc_prompt_res, MNCC_PROMPT_RES);
    mncc_prompt_res -> ti    = (UBYTE)ti;
    PSENDX (CC, mncc_prompt_res);
  }
  else
  {
    PALLOC (mncc_prompt_rej, MNCC_PROMPT_REJ);
    PSENDX (CC, mncc_prompt_rej);
  }

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (mncc_prompt_ind);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCP                 |
|                                 ROUTINE : psa_mncc_recall_ind     |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNCC_RECALL_IND primitive send by CC.
            this indicates a recall attempt from the network in case
            of CCBS.

*/

GLOBAL void psa_mncc_recall_ind
                         ( T_MNCC_RECALL_IND *mncc_recall_ind )
{
  SHORT cId;                 /* holds call table identifier */
  T_CC_CALL_TBL * pCtbNtry;  /* holds pointer to call table entry */

  TRACE_FUNCTION ("psa_mncc_recall_ind()");

/*
 *-------------------------------------------------------------------
 * check for K.O. criteria
 *-------------------------------------------------------------------
 */
  if( aoc_check_moc() EQ FALSE )
  {
    PALLOC (mncc_reject_req, MNCC_REJECT_REQ);

    mncc_reject_req -> ti    = mncc_recall_ind -> ti;
    mncc_reject_req -> cause = MNCC_CAUSE_ACM_MAX;

    PSENDX (CC, mncc_reject_req);
  }

  else if((cId = psaCC_ctbNewEntry()) < 0 )
  {
    PALLOC (mncc_reject_req, MNCC_REJECT_REQ);

    mncc_reject_req -> ti    = mncc_recall_ind -> ti;
    mncc_reject_req -> cause = MNCC_CAUSE_USER_BUSY;

    PSENDX (CC, mncc_reject_req);
  }

/*
 *-------------------------------------------------------------------
 * insert recall parameters into call table entry
 *-------------------------------------------------------------------
 */
  else
  {
    pCtbNtry = ccShrdPrm.ctb[cId];

    pCtbNtry -> ti         = mncc_recall_ind -> ti;
    pCtbNtry -> rptInd     = mncc_recall_ind -> ri;

    /* 
     * Assign called party 
     */
    pCtbNtry -> cldPty.ton = mncc_recall_ind -> called_party.ton;
    pCtbNtry -> cldPty.npi = mncc_recall_ind -> called_party.npi;
    pCtbNtry -> cldPty.c_called_num = 
      mncc_recall_ind -> called_party.c_called_num;
    if (pCtbNtry -> cldPty.called_num NEQ NULL)
    {
      ACI_MFREE (pCtbNtry -> cldPty.called_num);
      pCtbNtry -> cldPty.called_num = NULL;    
    }
    if (pCtbNtry -> cldPty.c_called_num NEQ 0)
    {
      ACI_MALLOC (pCtbNtry -> cldPty.called_num, 
                  pCtbNtry -> cldPty.c_called_num);
      memcpy (pCtbNtry -> cldPty.called_num, 
              mncc_recall_ind -> called_party.called_num,
              mncc_recall_ind -> called_party.c_called_num);
    }

    /* 
     * Assign called party subaddress
     */    
    pCtbNtry -> cldPtySub  = mncc_recall_ind -> called_party_sub;

    /*
     * Assign bearer caps
     */    
    memcpy( &(pCtbNtry->BC[0]),&(mncc_recall_ind->bcpara),
            sizeof( T_MNCC_bcpara) );
    memcpy( &(pCtbNtry->BC[1]),&(mncc_recall_ind->bcpara2),
            sizeof( T_MNCC_bcpara) );

    psaCC_phbSrchNum( cId, CT_NI_MOC );

    pCtbNtry -> calStat = CS_ACT_REQ;
    pCtbNtry -> calType = CT_NI_MOC;

    pCtbNtry -> prio  = MNCC_PRIO_NORM_CALL;
    pCtbNtry -> curBC = 0;

  }

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (mncc_recall_ind);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCP                 |
|                                 ROUTINE : psa_mncc_status_ind     |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNCC_STATUS_IND primitive sent by CC.
            This indicates that a STATUS ENQUIRY message has been
            received by CC, the network awaits a CC STATUS message.
            Used to ensure that the auxiliary states between ACI and
            CC were synchronized before the STATUS message is sent.

*/

GLOBAL void psa_mncc_status_ind (T_MNCC_STATUS_IND *mncc_status_ind)
{
  UBYTE ti;

  TRACE_FUNCTION ("psa_mncc_status_ind()");

  ti = mncc_status_ind->ti;

  {
    PREUSE (mncc_status_ind, mncc_status_res, MNCC_STATUS_RES);
    mncc_status_res->ti = ti;
    PSENDX (CC, mncc_status_res);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCP                 |
|                                 ROUTINE : psa_mncc_facility_ind   |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNCC_FACILITY_IND primitive send by CC.
            this indicates the receiving of a facility information
            element. Handles multiple components in the facility info.

*/

GLOBAL void psa_mncc_facility_ind(T_MNCC_FACILITY_IND * mncc_facility_ind)
{
  UBYTE  tlen = 0;
  USHORT offset_comp = 0, total_len = mncc_facility_ind->fac_inf.l_fac/8;
  UBYTE len_comp = 0;
  UBYTE *tPtrfac = mncc_facility_ind->fac_inf.fac;

  TRACE_FUNCTION ("psa_mncc_facility_ind()");

  if (tPtrfac[1] > 0x7F)/* For the case, when length is coded in non-standard way (0x80, 0x81, 0x82) we will be able to process only one component */
  {
     psaCC_ProcessCmp((T_MNCC_FACILITY_IND *)mncc_facility_ind);
     return;
  }

  offset_comp = mncc_facility_ind->fac_inf.o_fac;
  len_comp = tlen = tPtrfac[1] + 2;

  for ( ; total_len >= len_comp; len_comp+=tlen)
  {
    if ((*tPtrfac EQ 0xA1) OR
        (*tPtrfac EQ 0xA2) OR
        (*tPtrfac EQ 0xA3) OR
        (*tPtrfac EQ 0xA4))
    {
      PALLOC(new_mncc_facility_ind, MNCC_FACILITY_IND);

      new_mncc_facility_ind->ti = mncc_facility_ind->ti;
      new_mncc_facility_ind->fac_context = mncc_facility_ind->fac_context;
      new_mncc_facility_ind->fac_inf.l_fac = tlen * 8;
      new_mncc_facility_ind->fac_inf.o_fac = offset_comp;
      memcpy(new_mncc_facility_ind->fac_inf.fac, tPtrfac, tlen);

      psaCC_ProcessCmp((T_MNCC_FACILITY_IND *)new_mncc_facility_ind);

      tPtrfac = tPtrfac + tlen;
      tlen = tPtrfac[1] + 2;
      offset_comp = 0;
    }
  }
  
  PFREE(mncc_facility_ind);
}

/*==== EOF =========================================================*/
