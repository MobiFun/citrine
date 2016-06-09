/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_CCR
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
|             call control.
+-----------------------------------------------------------------------------
*/

#ifndef CMH_CCR_C
#define CMH_CCR_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

/*==== INCLUDES ===================================================*/

#include "aci_all.h"
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_mem.h"
#include "phb.h"
#include "l4_tim.h"
#include "aci_lst.h"


#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "aci.h"
#include "psa.h"
#include "ksd.h"
#include "psa_cc.h"
#include "psa_ss.h"
#include "psa_util.h"
#include "cmh.h"
#include "cmh_cc.h"
#include "cmh_ss.h"

#ifdef FF_ATI
#include "aci_io.h"
#endif   /* of #ifdef FF_ATI */

#ifdef UART
#include "dti.h"      /* functionality of the dti library */
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"
#endif

#ifdef FAX_AND_DATA

#include "psa_ra.h"
#include "cmh_ra.h"
#include "psa_l2r.h"
#include "cmh_l2r.h"
#include "psa_tra.h"

#ifdef FF_FAX
#include "psa_t30.h"
#include "cmh_t30.h"
#endif /* FF_FAX */

#endif /* FAX_AND_DATA */

#ifdef SIM_TOOLKIT
#include "psa_sat.h"
#include "cmh_sat.h"
#include "psa_sim.h"
#endif    /* of #ifdef SIM_TOOLKIT */

#include "aoc.h"
#include "audio.h"

#if defined (FF_WAP) || defined (FF_PPP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E)
#include "wap_aci.h"
#include "psa_ppp_w.h"
#include "cmh_ppp.h"
#endif /* of FF_WAP or FF_PPP || FF_GPF_TCPIP FF_SAT_E */

#if defined (CO_UDP_IP) || defined (FF_GPF_TCPIP)
#include "psa_tcpip.h"
#include "cmh_ipa.h"
#endif /* CO_UDP_IP || FF_GPF_TCPIP */

#ifdef FF_GPF_TCPIP
#include "dcm_utils.h"
#include "dcm_state.h"
#include "dcm_env.h"
#endif
#include "dcm_f.h"
#ifdef UART
#include "psa_uart.h"
#include "cmh_uart.h"
#endif /*UART*/

#ifdef FF_PSI
#include "psa_psi.h"
#include "cmh_psi.h"
#endif /*FF_PSI*/

#include "cmh_phb.h"

#include "psa_sim.h"
#include "cmh_sim.h"

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/* Implements Measure 90,91 */

/* 
 * Conversion Array that convert CSSX codes to CSSI codes.
 * CSSX_CODE is being used as the index of the array and 
 * CSSI_CODE is being used as the contents of the array.
 */

const T_ACI_CSSI_CODE cssx2I_Code_Convert[]=
{
  CSSI_CODE_ForwardedCall,
  CSSI_CODE_CUGCall,
  CSSI_CODE_NotPresent,
  CSSI_CODE_NotPresent,
  CSSI_CODE_NotPresent,
  CSSI_CODE_NotPresent,
  CSSI_CODE_NotPresent,
  CSSI_CODE_NotPresent,
  CSSI_CODE_NotPresent,
  CSSI_CODE_CFUActive,
  CSSI_CODE_SomeCCFActive,
  CSSI_CODE_CallWaiting,
  CSSI_CODE_OutCallsBarred,
  CSSI_CODE_IncCallsBarred,
  CSSI_CODE_CLIRSupRej,
  CSSI_CODE_DeflectedCall
};

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

#if defined (FF_WAP) || defined (FF_PPP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E)
ULONG ipAddress;

GLOBAL SHORT cmhCC_PPP_Established (ULONG ip_address,USHORT max_receive_unit,
                                    ULONG dns1, ULONG dns2);
GLOBAL SHORT cmhCC_PPP_Terminated (void);
#endif

#ifdef CO_UDP_IP 
GLOBAL SHORT cmhCC_IPA_Configurated (void);
GLOBAL SHORT cmhCC_UDPA_Configurated (void);
GLOBAL SHORT cmhCC_UDPA_Deconfigurated (void);
#endif /* CO_UDP_IP */

LOCAL void cmhCC_tstAndUnflag_MPTYCall( SHORT cId,
                                        UBYTE srcBuf,
                                        UBYTE cmdBuf,
                                        T_CC_CMD_PRM *pCCCmdPrm );
LOCAL BOOL cmhCC_check_isCallServiced( SHORT cId );
LOCAL void cmhCC_Call_Progress_Information( SHORT cId, T_ACI_CPI_MSG msgType, USHORT cause );

/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_CallAlerted            |
+-------------------------------------------------------------------+

  PURPOSE : called party is alerted

*/

GLOBAL void cmhCC_CallAlerted ( SHORT cId )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

  TRACE_FUNCTION ("cmhCC_CallAlerted()");

/*
 *-------------------------------------------------------------------
 * process event
 *-------------------------------------------------------------------
 */
  if (ctb->prgDesc EQ MNCC_PROG_NOT_PRES)
  {
    /* Implements Measure 207 */
    cmhCC_Call_Progress_Information( cId, CPI_MSG_Alert, ctb->curCs );
  }
  return;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_CallProceeding         |
+-------------------------------------------------------------------+

  PURPOSE : call proceed indication

*/

GLOBAL void cmhCC_CallProceeding ( SHORT cId )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
  T_ACI_CPI_MSG msgType;

  TRACE_FUNCTION ("cmhCC_CallProceeding()");

/*
 *-------------------------------------------------------------------
 * process event
 *-------------------------------------------------------------------
 */
  switch( ccShrdPrm.msgType )
  {
  case( MT_SETUP ): msgType = CPI_MSG_Setup; break;
  case( MT_DISC  ):
    switch (ctb->calStat)
    {
    case NO_VLD_CS:
    case CS_ACT_REQ:
    case CS_CPL_REQ:
    case CS_DSC_REQ:
      break;
    default:
      return;
    }
    msgType = CPI_MSG_Disc;
    break;
  case( MT_ALRT  ): msgType = CPI_MSG_Alert; break;
  case( MT_PROC  ): msgType = CPI_MSG_Proc;  break;
  case( MT_PROGR ): msgType = CPI_MSG_Progr; break;
  /* case( MT_CONN  ): msgType = CPI_MSG_Conn;  break; */
  case( MT_CONN  ): return; /* CPI is generated in cmhCC_CallConnected */
  default:          TRACE_EVENT("UNEXP MSG TYPE IN CC SHRD PARM");
                    return;
  }

/*  ctb->curCs = MNCC_CAUSE_NO_MS_CAUSE; */
  /* Implements Measure 207 */
  cmhCC_Call_Progress_Information( cId, msgType, ctb->curCs );

  return;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_CallConnected          |
+-------------------------------------------------------------------+

  PURPOSE : call is connected

*/

GLOBAL void cmhCC_CallConnected ( SHORT cId )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
  T_CC_CMD_PRM * pCCCmdPrm;   /* points to CC command parameters */
  CHAR numBuf[MNCC_MAX_CC_CALLING_NUMBER+1]; /* buffers calling number + '\0' */
  CHAR subBuf[MNCC_SUB_LENGTH+1];  /* buffers subaddress 'subaddress\0' */
  T_ACI_TOA toaBuf;           /* buffers type of address */
  T_ACI_TOS tosBuf;           /* buffers type of subaddress */
  UBYTE cmdBuf;               /* buffers current command */
  UBYTE srcBuf;               /* buffers current command source */
  T_CC_CALL_TYPE calltype = cmhCC_getcalltype(cId);

  TRACE_FUNCTION ("cmhCC_CallConnected()");

  /* check for command context */
  switch(ctb->curCmd)
  {
  case(AT_CMD_D):
  case(AT_CMD_A):
  case(AT_CMD_CHLD):
    break;

  default:
    TRACE_EVENT_P1("cmhCC_CallConnected(): wrong command context %d", ctb->curCmd);
    return;
  }

  /* call progress information */
  /* Implements Measure 207 */
  cmhCC_Call_Progress_Information( cId, CPI_MSG_Conn, ctb->curCs );

  /* process event for D, A and +CHLD command */
  pCCCmdPrm = &cmhPrm[ctb->curSrc].ccCmdPrm;

  if (ctb->curCmd EQ AT_CMD_D)
  {
    /* Check if user allows Connected Line Presentation: FTA 31.1.3.1 */
    switch (ctb->clgPty.present)
    {
      case (MNCC_PRES_PRES_ALLOW): /* presentation allowed FTA 31.1.3.1 */
        TRACE_EVENT ("cmhCC_CallConnected(): presentation allowed !");
        break;

      case (MNCC_PRES_NOT_PRES):
        TRACE_EVENT ("cmhCC_CallConnected(): presentation not present !");
        break;

      case (MNCC_PRES_PRES_REST): /* presentation restricted FTA 31.1.4.2 procedure 1 */
        TRACE_EVENT ("cmhCC_CallConnected(): presentation restricted !");
        /*
         * R_AT( RAT_COLR, ccShrdPrm.???.???.??? )
         *  ( COLR_STAT_??? );
         */
        break;

      case (MNCC_PRES_NUMB_NOT_AVAIL): /* number not available due to interworking FTA 31.1.4.2 procedure 2 */
        TRACE_EVENT ("cmhCC_CallConnected(): number not available due to interworking !");
        /*
         * R_AT( RAT_COLR, ccShrdPrm.???.???.??? )
         *  ( COLR_STAT_??? );
         */
        break;

      default:
        TRACE_EVENT ("[ERR] cmhCC_CallConnected(): pCtbNtry->clgPty.present = UNKNOWN");
        return;

    }

    if ((ctb->clgPty.present NEQ MNCC_PRES_NOT_PRES)  /* any .present indicator received from MSC? */
    AND (ctb->clgPty.c_num EQ 0) )               /* but no number available? */
    {
      ;   /* do nothig */
    }
    else
    {
      /*
       * This function call (rAT_PlusCOLP) informs MWF/BMI about
       * presentation of the called number on display.
       * If MFW did not receive this function call, then it should
       * not display the number after querying the call state with
       * qAT_PercentCAL() where it gets the typed in called number, unfortunately
       */
      R_AT( RAT_COLP, (T_ACI_CMD_SRC)ctb->curSrc )
        ( COLP_STAT_NotPresent,
          psaCC_ctbClrAdr2Num( cId, numBuf, sizeof (numBuf) ),
          cmhCC_ctbGetClrNumTyp( cId, &toaBuf ),
          psaCC_ctbClrAdr2Sub( cId, subBuf ),
          cmhCC_ctbGetClrSubTyp( cId, &tosBuf ),
          psaCC_ctbGetAlpha( cId) );
    }
  }


  /* reset CMOD to single mode */
  ccShrdPrm.CMODmode = CMOD_MOD_Single;

  switch(calltype)
  {
  /* Speech Call */
  case( VOICE_CALL ):
    cmdBuf = ctb->curCmd;
    srcBuf = ctb->curSrc;

    /* reset CMOD to single mode */
    if( ctb->BC[1].bearer_serv NEQ MNCC_BEARER_SERV_NOT_PRES )
    {
      ccShrdPrm.CMODmode = CMOD_MOD_Single;
    }

    /*
     *  check if connect is expected
     */
    if( cmhCC_tstAndUnflagCall( cId, &(pCCCmdPrm -> mltyCncFlg)))
    {
      if( pCCCmdPrm -> mltyCncFlg EQ 0 )  /* if last call */
      {

#ifdef SIM_TOOLKIT
        if( ctb->SATinv )
        {
          cmhSAT_CallCncted();
          ctb->SATinv = FALSE; /* unsent */
        }
#endif /* SIM_TOOLKIT */
        if(!cmhCC_atdsendok(cId)) /* if OK hasn't been sent right after MNCC_SETUP_REQ, then send it now */
        {
          /* needed for RAT_OK in MFW... Temp until tasks are separated */
          cmhCC_PrepareCmdEnd (cId, NULL, NULL);

          R_AT( RAT_OK, (T_ACI_CMD_SRC)srcBuf ) ( cmdBuf );
        }

        /* log result */
        cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_OK,(T_ACI_AT_CMD)cmdBuf, (SHORT)(cId+1), 
                                         BS_SPEED_NotPresent, CME_ERR_NotPresent );
      }
    }
    cmhCC_PrepareCmdEnd (cId, NULL, NULL);
    break;

#if defined (FAX_AND_DATA) AND defined (DTI)

  /* in case of a supported data call */
  case(TRANS_CALL):
  case(NON_TRANS_CALL):
  case(FAX_CALL):
#ifdef FF_PPP
  /* PPP call initialisation (AAA - PPP - L2R) */
  case(PPP_CALL):
#endif /* FF_PPP */
   /* wait for TCH if not assigned */
    if( ccShrdPrm.TCHasg )
    {
      /* activate RA connection: in case of failure clear call ! */
      ccShrdPrm.datStat = DS_ACT_REQ;

      if (cmhRA_Activate( (T_ACI_CMD_SRC)ctb->curSrc, (T_ACI_AT_CMD)ctb->curCmd, cId ) NEQ AT_EXCT)
      {
        TRACE_EVENT("RA ACTIVATION FAILURE -> DISC CALL");
        ccShrdPrm.datStat = DS_IDL;
        ctb->nrmCs = MNCC_CAUSE_CALL_CLEAR;
        psaCC_ClearCall (cId);
#if defined (SIM_TOOLKIT) AND defined (FF_SAT_E)
          /* check impact for SAT commands */
        cmhSAT_OpChnCSDDown( cId, TPL_NONE );
#endif /* SIM TOOLKIT AND FF_SAT_E */
      }
    }
    break;

#if defined(CO_UDP_IP) OR defined(FF_GPF_TCPIP)
#ifdef CO_UDP_IP
  case(UDPIP_CALL):
#endif
#ifdef FF_GPF_TCPIP
  case(TCPIP_CALL):
#endif
    /* wait for TCH if not assigned */
    if( ccShrdPrm.TCHasg )
    {
      if (wap_state NEQ Wap_Not_Init) /* release call */
      {
        TRACE_EVENT("FATAL ERROR in cmhCC_CallConnected: WAP Call already in process");
        ctb->nrmCs = MNCC_CAUSE_CALL_CLEAR;
        psaCC_ClearCall (cId);
#if defined (SIM_TOOLKIT) AND defined (FF_SAT_E)  
        cmhSAT_OpChnCSDDown( cId, UDP );
#endif /* SIM TOOLKIT AND FF_SAT_E */
        break;
      }
      else
      {
        TRACE_EVENT("WAP call initialisation");
#if defined (SIM_TOOLKIT) AND defined (FF_SAT_E)
        if( cmhSAT_OpChnChckCSD(UDP) )
        {
          psaTCPIP_Activate(ctb->curSrc, 0,
                            cId,
                            TCPIP_ACT_OPTION_V4 | TCPIP_ACT_OPTION_UDP,
                            TCPIP_CONNECTION_TYPE_CSD_WAP,
                            cmhSAT_OpChnUDPActiveCsd) ;
        }
        else
#endif /* SIM TOOLKIT AND FF_SAT_E */
        {
          if(is_gpf_tcpip_call())
          {
            psaTCPIP_Activate(ctb->curSrc, 0 /* Change this! */,
                            cId,
                              TCPIP_ACT_OPTION_V4 | TCPIP_ACT_OPTION_TCPIP,
                            TCPIP_CONNECTION_TYPE_CSD_WAP,
                            psaTCPIP_act_csd_callback) ;
        }
          else
          {
            psaTCPIP_Activate(ctb->curSrc, 0 /* Change this! */,
                              cId,
                              TCPIP_ACT_OPTION_V4 | TCPIP_ACT_OPTION_UDP,
                              TCPIP_CONNECTION_TYPE_CSD_WAP,
                              psaTCPIP_act_csd_callback) ;
          }
        }
      }
    }
    break;
#endif /* defined(WAP) OR defined(FF_GPF_TCPIP) */

 /* in case of an unsupported data call */
  case(UNSUP_DATA_CALL):
    TRACE_EVENT("UNSUPPORTED DATA CALL -> DISC CALL");
    ctb->nrmCs = MNCC_CAUSE_CALL_CLEAR;
    psaCC_ClearCall (cId);
#if defined (SIM_TOOLKIT) AND defined (FF_SAT_E) 
    /* check impact for SAT commands */
    cmhSAT_OpChnCSDDown( cId, TPL_NONE );
#endif /* SIM TOOLKIT AND SAT_SAT_E */
    break;

#endif  /* of #ifdef FAX_AND_DATA */

  default:
    TRACE_ERROR("Wrong call type");
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : process_CHLDaddInfo          |
+-------------------------------------------------------------------+

  PURPOSE : Local function: processes parameter CHLDaddInfo (continue action that
            had to wait for a call to be connected first).
*/
/* Held call is being disconnected */
#define DISCONNECT_HELD (0)
/* Call is being put on hold */
#define CALL_HELD       (1)
/* Multyparty call is being put on hold */
#define MTPTY_HELD      (2)

LOCAL void process_CHLDaddInfo( UBYTE srcId, UBYTE context )
{
  /* param context allows to differentiate some special case */
  SHORT addId;
  T_CC_CMD_PRM *pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;

  TRACE_FUNCTION("process_CHLDaddInfo( )");

  for( addId = 0; !((pCCCmdPrm -> mltyCncFlg >> addId) & 0x01); addId++ )
    ;

  switch( CHLDaddInfo )
  {
    case( CHLD_ADD_INFO_ACC_CAL ):
      cmhCC_AcceptCall(addId, (T_ACI_CMD_SRC)srcId, AT_CMD_CHLD);
      return;

    case (CHLD_ADD_INFO_DIAL_CAL):
      cmhCC_NewCall(addId, (T_ACI_CMD_SRC)srcId, AT_CMD_D);
      return;

    case( CHLD_ADD_INFO_RTV_CAL ):
      if(context NEQ CALL_HELD)
      {
        cmhCC_RetrieveCall(addId, (T_ACI_CMD_SRC)srcId);
        return;
      }
      break;

    case (NO_CHLD_ADD_INFO):
      if(context EQ DISCONNECT_HELD)
      {
        return; /* May happen for CCBS recall, disconnecting the active call */
      }
      break;
  }

  TRACE_EVENT("UNEXP ADD CHLD INFO");
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_CallDisconnected       |
+-------------------------------------------------------------------+

  PURPOSE : call is disconnected: process event for D and A command, abort command or no command
*/

#ifdef SIM_TOOLKIT
LOCAL void cmhCC_CallDisc_whiledialling_sat(SHORT cId, BOOL *send_atresult)
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
  UBYTE         network_cause;

  if( !ctb->SATinv )
  {
    return;
  }

  TRACE_FUNCTION("cmhCC_CallDisc_whiledialling_sat");

  switch(ctb->curCmd)
  {
  case( AT_CMD_A ):
    /* if connect is expected */
    if( cmhPrm[ctb->curSrc].ccCmdPrm.mltyCncFlg NEQ 0 )
    {
      if( ctb->SATinv & SAT_REDIAL )
      {
        ctb->calStat = CS_SAT_REQ;
        return;
      }

      /* return network error cause GSM 11.14 / 12.12.3 */
      if(GET_CAUSE_VALUE(ctb->nrmCs) EQ NOT_PRESENT_8BIT)
      {
        network_cause = ADD_NO_CAUSE;
      }
      else
        network_cause = GET_CAUSE_VALUE(ctb->nrmCs)|0x80;

      *send_atresult = cmhSAT_NtwErr(network_cause);
    }
    break;

  case( AT_CMD_ABRT ):
    /* if disconnect is expected */
    if( cmhPrm[ctb->curSrc].ccCmdPrm.mltyDscFlg NEQ 0 )
    {
      cmhSAT_UserClear();
    }
    break;
#ifdef FF_SAT_E
  default:
    cmhSAT_OpChnCSDDown( cId, TPL_DONT_CARE );
    break;
#endif /* FF_SAT_E */
  }
}
#endif /*#ifdef SIM_TOOLKIT*/
/*
+------------------------------------------------------------------------------
|   Function    :  cmhCC_CallDisc_connectingphase
+------------------------------------------------------------------------------
|   Description :  This function checks on call type and disconnects the
|                  different calltypes.
|
|   Parameters  :  call id
|
|   Return      :
|
+------------------------------------------------------------------------------
*/

LOCAL void cmhCC_CallDisc_connectingphase ( SHORT cId )
{
  T_CC_CMD_PRM * pCCCmdPrm;   /* points to CC command parameters */
  T_CC_CALL_TYPE calltype = cmhCC_getcalltype(cId);
  T_CC_CALL_TBL  *ctb     = ccShrdPrm.ctb[cId];
  BOOL           atRslt   = TRUE;        /* flags to send a AT result code */

  TRACE_FUNCTION("cmhCC_CallDisc_connectingphase");

  pCCCmdPrm = &cmhPrm[ctb->curSrc].ccCmdPrm;

  /* reset CMOD to single mode */
  ccShrdPrm.CMODmode = CMOD_MOD_Single;

#ifdef SIM_TOOLKIT
  /* handling of SAT invoked call */
  cmhCC_CallDisc_whiledialling_sat(cId, &atRslt);
#endif /* SIM_TOOLKIT */

  /* Unflag Multiconnect/disconnect Flags */
  if( ctb->curSrc NEQ CMD_SRC_NONE )
  {
    cmhCC_tstAndUnflagCall( cId,
                            &pCCCmdPrm->mltyCncFlg );
    cmhCC_tstAndUnflagCall( cId,
                            &pCCCmdPrm->mltyDscFlg );
  }

  /* CCBS handling */
  if( ctb->curCmd EQ AT_CMD_D      AND
      ctb->CCBSstat EQ CCBSS_PSSBL )
  {
    cmhrat_ccbs( CMD_SRC_MAX, CCBS_IND_Possible,
                 CCBS_STAT_NotPresent, NULL );
  }

  switch( calltype )
  {
    case( VOICE_CALL ): /* voice call */
      /* CSSU indications handling */
      if( ctb->calStat EQ CS_HLD )
      {
        if( ctb->curCmd NEQ AT_CMD_NONE )
        {
           send_CSSX_notification(cId,
                              CSSX_CODE_HeldCallRel,
                              ACI_NumParmNotPresent,
                              NULL, NULL, NULL, NULL);
        }
      }
      /* Check for CHLDmode and if all calls are disconnected send 
         OK to the terminal and reset CHLDmode */
      if (pCCCmdPrm->CHLDmode EQ CHLD_MOD_Ect AND 
          (pCCCmdPrm->mltyCncFlg EQ 0) AND
          (pCCCmdPrm->mltyDscFlg EQ 0))
      {
        pCCCmdPrm->CHLDmode = CHLD_MOD_NotPresent;
        R_AT( RAT_OK, (T_ACI_CMD_SRC)ctb->curSrc ) ( AT_CMD_CHLD );

        /* log result */
        cmh_logRslt ( (T_ACI_CMD_SRC)ctb->curSrc, RAT_OK,
                       AT_CMD_CHLD, (SHORT)(cId+1),BS_SPEED_NotPresent, CME_ERR_NotPresent );
      }
      break;

#if defined (FAX_AND_DATA) AND defined (DTI)
    case(TRANS_CALL):
      if( ccShrdPrm.datStat NEQ DS_IDL )
      {
        ccShrdPrm.datStat = DS_STOP_REQ;
        cmhTRA_Deactivate ();
        return;
      }
      break;

    case(NON_TRANS_CALL):
      if( ccShrdPrm.datStat NEQ DS_IDL )
      {
        ccShrdPrm.datStat = DS_STOP_REQ;
        cmhL2R_Deactivate ();
        return;
      }
      break;

#ifdef FF_FAX
    case( FAX_CALL ):
      if( ccShrdPrm.datStat NEQ DS_IDL )
      {
        ccShrdPrm.datStat = DS_STOP_REQ;
        cmhT30_Deactivate ();
        return;
      }
      break;
#endif /* FF_FAX */

#ifdef CO_UDP_IP 
    case(UDPIP_CALL):
    /* check if a UDP/IP call is in process... If yes then end call */
      if (ccShrdPrm.datStat NEQ DS_IDL)
      {
        ccShrdPrm.datStat = DS_STOP_REQ;

        /* Check command context to know if lower layers of PPP already are down or not */
        if(ctb->curCmd EQ AT_CMD_NONE OR
           ctb->curCmd EQ AT_CMD_D )
        {
          /* If PPP connected - send terminate request to PPP */
          if(pppShrdPrm.state EQ PPP_ESTABLISHED)
          {
            cmhPPP_Terminate(DWN);
          }
          return;
        }
      }
      break;
#endif  /* CO_UDP_IP */

#endif /* of #ifdef FAX_AND_DATA */
  }

  /* Command is done: send response to user if not suppressed */
  /* if(atRslt) cmhCC_SndDiscRsn( cId ); */  /* to pass 26.8.1.2.8.1 */
}

LOCAL void call_disconnected_data_call(T_CC_CALL_TYPE calltype, SHORT cId)
{
  TRACE_FUNCTION("call_disconnected_data_call");

  /* Local disconnection for PPP if peer shuts down the line
     instead of normal PPP shutdown */
  switch( calltype )
  {
#if defined (FAX_AND_DATA) AND defined (DTI)
 /* check if a WAP call is in process... If yes then end call */
#if defined(CO_UDP_IP) || defined(FF_PPP)

#ifdef CO_UDP_IP
  case(UDPIP_CALL):
#endif /* CO_UDP_IP */

#if defined(FF_GPF_TCPIP)
  case (TCPIP_CALL):
#endif /* FF_GPF_TCPIP */
#ifdef FF_PPP
  case PPP_CALL:
  pppShrdPrm.is_PPP_CALL = FALSE;
  TRACE_EVENT ("is_PPP_CALL resetted");
#endif /* FF_PPP */
    if (ccShrdPrm.datStat NEQ DS_IDL)
    {
      ccShrdPrm.datStat = DS_STOP_REQ;

      /* Only local disconnection is possible in this case, the call is already disconnected !
      if(pppShrdPrm.state EQ PPP_ESTABLISHED)
        cmhPPP_Terminate(UP); */

      if((pppShrdPrm.state EQ PPP_ESTABLISHED) OR
         (pppShrdPrm.state EQ PPP_ESTABLISH)   OR
         (pppShrdPrm.state EQ PPP_TERMINATE))
      {
        cmhPPP_Terminate(DWN);
      }
      else if (wap_state > Wap_Not_Init      AND
               wap_state <= UDPA_Deactivation)
      {
        cmhPPP_Terminated();   /* Trigger termination of TCPIP if PPP did not yet start (ACI-FIX-9317)*/
      }
    }
    break;
#endif  /* of WAP || FF_PPP || SAT E */

  case(TRANS_CALL):
  case(NON_TRANS_CALL):
    if( ccShrdPrm.datStat NEQ DS_IDL )
    {
      ccShrdPrm.datStat = DS_STOP_REQ;
      if(calltype EQ TRANS_CALL)
      {
        cmhTRA_Deactivate ();
      }
      else
      {
        cmhL2R_Deactivate ();
      }
    }
    break;
#endif /* FAX_AND_DATA */

  default:
    break;
  } /* end switch */
} /* end switch */

LOCAL void cmhCC_CallDisc_userinitiated ( SHORT cId )
{
  T_CC_CMD_PRM * pCCCmdPrm;   /* points to CC command parameters */
  T_CC_CALL_TYPE calltype = cmhCC_getcalltype(cId);
  T_CC_CALL_TBL  *ctb     = ccShrdPrm.ctb[cId];
#ifdef FF_PSI
  T_ACI_DTI_PRC_PSI  *src_infos = NULL;
#endif /*FF_PSI*/
  UBYTE cmdBuf;               /* buffers current command */
  UBYTE srcBuf;               /* buffers current command source */
  BOOL dtr_clearcall = FALSE;

  TRACE_FUNCTION("cmhCC_CallDisc_userinitiated");

  /* process event for H and +CHUP command */
  pCCCmdPrm = &cmhPrm[ctb->curSrc].ccCmdPrm;

  /* reset CMOD to single mode */
  ccShrdPrm.CMODmode = CMOD_MOD_Single;

  call_disconnected_data_call(calltype, cId);

  cmhCC_PrepareCmdEnd (cId, &cmdBuf, &srcBuf);

  /* if ATH before the call is connected, the flag must be cleared */
  cmhCC_tstAndUnflagCall (cId, &(pCCCmdPrm -> mltyCncFlg));

  /* check if disconnect is expected */
  if( cmhCC_tstAndUnflagCall( cId, &(pCCCmdPrm -> mltyDscFlg)))
  {
#if defined (FF_WAP) || defined (FF_SAT_E)
    /* Check if the WAP stack is down  */
    if((pCCCmdPrm -> mltyDscFlg EQ 0)               AND
       (ccShrdPrm.wapStat       EQ CC_WAP_STACK_DOWN))
#else
    if( pCCCmdPrm -> mltyDscFlg EQ 0 )
#endif /* WAP || SAT E */
    /****************/
    /* if last call */
    /****************/
    {
#ifdef SIM_TOOLKIT
      if( ctb->SATinv )
      {
        cmhSAT_StartPendingCall( );
      }
      else
#endif  /* SIM_TOOLKIT */
      {
        if (cmdBuf EQ AT_CMD_Z)
        {
          cmh_Reset ( (T_ACI_CMD_SRC)srcBuf, TRUE );
          R_AT(RAT_Z, (T_ACI_CMD_SRC)srcBuf)();
        }
#if defined (FF_PSI) AND defined (DTI)
        src_infos = find_element (psi_src_params, srcBuf, cmhPSItest_srcId);
        if (src_infos EQ NULL)
#endif /*FF_PSI*/
#ifdef UART
        {
          if (uartShrdPrm.dtr_clearcall EQ TRUE)
          {
            uartShrdPrm.dtr_clearcall = FALSE;
            dtr_clearcall = TRUE;
          }
        } 
#endif /*UART*/
#if defined (FF_PSI) AND defined (DTI)
        else if (psiShrdPrm.dtr_clearcall EQ TRUE)
        { 
           psiShrdPrm.dtr_clearcall = FALSE;
           dtr_clearcall = TRUE;
        }
#endif /*FF_PSI*/
        /* To avoid the redudancy of the code */
        /* If UART and PSI is disabled, still OK has to be sent to MMI */
        if (dtr_clearcall)
        {
          R_AT( RAT_NO_CARRIER,(T_ACI_CMD_SRC)srcBuf ) ( cmdBuf, cId+1 );
          cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf,
          RAT_NO_CARRIER, (T_ACI_AT_CMD)cmdBuf, (SHORT)(cId+1), BS_SPEED_NotPresent, CME_ERR_NotPresent );
        }
        else
        {
          R_AT( RAT_OK, (T_ACI_CMD_SRC)srcBuf ) ( cmdBuf );
          /* log result */
          cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf,
          RAT_OK, (T_ACI_AT_CMD)cmdBuf, (SHORT)(cId+1), BS_SPEED_NotPresent,CME_ERR_NotPresent );
        }
      }
    }
  }

  /* inform call owner about disconnect */
  if( srcBuf EQ ctb->calOwn
#ifdef SIM_TOOLKIT
    AND
      !ctb->SATinv )
#else
    )
#endif
  {
    /* do nothing: owner knows already... */
    /* EXCEPT if !ctb->SATinv: a SAT call setup has been started, and all previous
    calls had to be disconnected (because of SAT command qualifer) */
  }
  else
  {
    R_AT( RAT_NO_CARRIER, (T_ACI_CMD_SRC)ctb->calOwn ) ( AT_CMD_NONE, cId+1 );
    cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_NO_CARRIER, (T_ACI_AT_CMD)cmdBuf,
                  (SHORT)(cId+1), BS_SPEED_NotPresent, CME_ERR_NotPresent);
  }
}

/* process disconnection of an held call: +CHLD command context */
LOCAL void cmhCC_CallDisc_heldcall ( SHORT cId )
{
  T_CC_CMD_PRM * pCCCmdPrm;   /* points to CC command parameters */
  T_CC_CALL_TYPE calltype = cmhCC_getcalltype(cId);
  T_CC_CALL_TBL  *ctb     = ccShrdPrm.ctb[cId];
  UBYTE cmdBuf;               /* buffers current command */
  UBYTE srcBuf;               /* buffers current command source */
  UBYTE cmd_id = 0;           /* Command scr id */

  TRACE_FUNCTION("cmhCC_CallDisc_heldcall");

  pCCCmdPrm = &cmhPrm[ctb->curSrc].ccCmdPrm;

  cmhCC_PrepareCmdEnd (cId, &cmdBuf, &srcBuf);

  if( pCCCmdPrm -> mltyDscFlg EQ 0 AND
      pCCCmdPrm -> mltyCncFlg EQ 0     )  /* if last call */
  {
    R_AT( RAT_OK,  (T_ACI_CMD_SRC)srcBuf ) ( cmdBuf );

    /* log result */
    cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_OK,
                   (T_ACI_AT_CMD)cmdBuf, (SHORT)(cId+1), BS_SPEED_NotPresent, CME_ERR_NotPresent );
  }
  /* check if disconnect is expected */
  else if( cmhCC_tstAndUnflagCall( cId, &(pCCCmdPrm -> mltyDscFlg)))
  {
    if( pCCCmdPrm -> CHLDmode EQ CHLD_MOD_RelActAndAcpt AND
        pCCCmdPrm -> mltyCncFlg AND pCCCmdPrm -> mltyDscFlg EQ 0 )
    {
      process_CHLDaddInfo( srcBuf, DISCONNECT_HELD );
    }
    else if( pCCCmdPrm -> CHLDmode EQ CHLD_MOD_Ect )
    {
      cmhCC_tstAndUnflagCall( cId, &(pCCCmdPrm -> mltyCncFlg)); /* now disconnected */
    }

    if( pCCCmdPrm -> mltyDscFlg EQ 0 AND
        pCCCmdPrm -> mltyCncFlg EQ 0     )  /* if last call */
    { 
      /* Reset the CHLDmode to default value */
      if( pCCCmdPrm -> CHLDmode EQ CHLD_MOD_Ect )
      {
        pCCCmdPrm -> CHLDmode = CHLD_MOD_NotPresent;
      }
      R_AT( RAT_OK, (T_ACI_CMD_SRC)srcBuf ) ( cmdBuf );

      /* log result */
      cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_OK,
                     (T_ACI_AT_CMD)cmdBuf, (SHORT)(cId+1),BS_SPEED_NotPresent, CME_ERR_NotPresent );
    }
  }
  /* check if disconnect is unexpected */
  else if( cmhCC_tstAndUnflagCall( cId, &(pCCCmdPrm -> mltyCncFlg)))
  {
    if( pCCCmdPrm -> CHLDmode EQ CHLD_MOD_HldActAndAcpt AND
        pCCCmdPrm -> mltyCncFlg AND pCCCmdPrm -> mltyDscFlg EQ 0 )
    {
      process_CHLDaddInfo( srcBuf, DISCONNECT_HELD );
    }
    else
    {
      pCCCmdPrm -> mltyCncFlg = 0;        /* unflag calls */

      R_AT( RAT_CME, (T_ACI_CMD_SRC)srcBuf ) ( cmdBuf, CME_ERR_NotPresent );

      /* log result */
      cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_CME, (T_ACI_AT_CMD)cmdBuf,
                    -1, BS_SPEED_NotPresent, CME_ERR_NotPresent );
    }
  }

   /* inform all the sources about call disconnect */
  for( cmd_id = 0; cmd_id < CMD_SRC_MAX; cmd_id++ )
  {
    R_AT( RAT_NO_CARRIER, cmd_id )( AT_CMD_NONE, cId+1 );
  }
}

/* process event for the +CTFR command.*/
LOCAL void cmhCC_CallDisc_transferedcall ( SHORT cId )
{
  T_CC_CMD_PRM   *pCCCmdPrm;   /* points to CC command parameters */
  T_CC_CALL_TYPE calltype = cmhCC_getcalltype(cId);
  T_CC_CALL_TBL  *ctb     = ccShrdPrm.ctb[cId];
  UBYTE          cmdBuf;       /* buffers current command */
  UBYTE          srcBuf;       /* buffers current command source */
  RAT_ID         rat_id;
  T_ACI_CME_ERR  cme_err;

  TRACE_FUNCTION("cmhCC_CallDisc_transferedcall");

  /* Get a pointer to the appropriate CC call table entry */
  pCCCmdPrm = &cmhPrm[ctb->curSrc].ccCmdPrm;

  /* End of command execution here */
  cmhCC_PrepareCmdEnd (cId, &cmdBuf, &srcBuf);

  (void)cmhCC_tstAndUnflagCall( cId, &(pCCCmdPrm -> mltyDscFlg));

  switch (ctb->CDStat)
  {
    case CD_Succeeded:
      rat_id  = RAT_OK;
      cme_err = CME_ERR_NotPresent;
      break;

    case CD_Requested: /* No answer from the network */
      rat_id  = RAT_CME;
      cme_err = CME_ERR_Timeout;
      break;

    case CD_Failed: /* SS transaction failed */
      rat_id  = RAT_CME;
      cme_err = CME_ERR_OpNotSupp;
      break;

    default: /* Unexpected here */
      rat_id  = RAT_CME;
      cme_err = CME_ERR_Unknown;
      break;
  }

  if(rat_id EQ RAT_OK)
  {
    R_AT( RAT_OK,  (T_ACI_CMD_SRC)srcBuf ) ( cmdBuf );
  }
  else if(rat_id EQ RAT_CME)
  {
    R_AT( RAT_CME, (T_ACI_CMD_SRC)srcBuf ) ( cmdBuf, cme_err );
  }
  cmh_logRslt(  (T_ACI_CMD_SRC)srcBuf, rat_id, (T_ACI_AT_CMD)cmdBuf,
               (SHORT)(cId+1), BS_SPEED_NotPresent,cme_err );

  ctb->CDStat = NO_VLD_CD;
}

GLOBAL void cmhCC_CallDisconnected ( SHORT cId )
{
  #if defined(FF_GPF_TCPIP)
  T_CC_CALL_TYPE calltype = cmhCC_getcalltype(cId);
  #endif /* FF_GPF_TCPIP */
  T_CC_CALL_TBL  *ctb = ccShrdPrm.ctb[cId];
  
  TRACE_FUNCTION ("cmhCC_CallDisconnected()");

  /* inform advice of charge module */
  aoc_info (cId, AOC_STOP_TIME);

#if defined(FF_GPF_TCPIP)
  /* Needed if user stops call setup before app got connection conf */
 if (calltype EQ TCPIP_CALL)
 {
    T_DCM_STATUS_IND_MSG msg;
    msg.hdr.msg_id = DCM_NEXT_CMD_STOP_MSG;
    msg.result     = DCM_PS_CONN_BROKEN;
    dcm_send_message(msg, DCM_SUB_WAIT_SATDN_CNF);
 }
#endif /* FF_GPF_TCPIP */

  /*
   *----------------------------------------------------------------
   * call progress information
   *----------------------------------------------------------------
   */
  /* Implements Measure 207 */
  cmhCC_Call_Progress_Information( cId, CPI_MSG_Disc, ctb->curCs );

  /*************************
  call was not serviced yet
  ***************************/
   /* Implements Measure 32 */
  if (cmhCC_check_isCallServiced( cId ) EQ TRUE )
  {
    return;
  }

  /*************************
   check for command context
  ***************************/
  switch( ctb->curCmd )
  {
    case( AT_CMD_NONE ):
    case( AT_CMD_A ):
    case( AT_CMD_D ):
    case( AT_CMD_ABRT ):
      cmhCC_CallDisc_connectingphase(cId);
      break;

    case( AT_CMD_H ):
    case( AT_CMD_Z ):
    case( AT_CMD_CHUP ):

#ifdef FF_FAX
    case( AT_CMD_FKS ):
    case( AT_CMD_FDT ):
    case( AT_CMD_FDR ):
#endif /* FF_FAX */

      cmhCC_CallDisc_userinitiated(cId);
      break;

    case( AT_CMD_CHLD ):
      cmhCC_CallDisc_heldcall(cId);
      break;

    case( AT_CMD_CTFR ):
      cmhCC_CallDisc_transferedcall(cId);
      break;
  } /* switch */

/* PATCH BE 03.07.00
 * DO NOT declare the call table entry as unused:
  do not free call table at this time then every second MTC will fail if early
  assignment is used. */

  return;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_CallReleased           |
+-------------------------------------------------------------------+

  PURPOSE : call is released

*/

GLOBAL void cmhCC_CallReleased ( SHORT cId )
{
  UBYTE cmdBuf;               /* buffers current command */
  UBYTE srcBuf;               /* buffers current command source */
  T_CC_CMD_PRM   *pCCCmdPrm;   /* points to CC command parameters */
  T_CC_CALL_TYPE calltype = cmhCC_getcalltype(cId);
  T_CC_CALL_TBL  *ctb = ccShrdPrm.ctb[cId];
  
  TRACE_FUNCTION ("cmhCC_CallReleased()");

  /* inform advice of charge module */
  aoc_info (cId, AOC_STOP_TIME);

#if defined(FF_GPF_TCPIP)
  /* This will happen if network got lost during a CSD call */
  if (calltype EQ TCPIP_CALL)
  {
    T_DCM_STATUS_IND_MSG msg;
    msg.hdr.msg_id = DCM_ERROR_IND_MSG;
    msg.result     = DCM_PS_CONN_BROKEN;
    dcm_send_message(msg, DCM_SUB_WAIT_SATDN_CNF);
  }
#endif /* FF_GPF_TCPIP */

  /* check for command context */
  switch( ctb->curCmd )
  {
  case( AT_CMD_A ):
  case( AT_CMD_D ):
  case( AT_CMD_NONE ):
    /* process event for A, D or no command */

    /* handling for CCBS prolonged session */

    if( ctb->curSrc NEQ CMD_SRC_NONE )
    {
    cmhCC_tstAndUnflagCall( cId, &cmhPrm[ctb->curSrc].ccCmdPrm.mltyCncFlg );
    cmhCC_tstAndUnflagCall( cId, &cmhPrm[ctb->curSrc].ccCmdPrm.mltyDscFlg );
    }
      
    if (ctb->CCBSstat EQ CCBSS_PSSBL )
    {
      cmhrat_ccbs (CMD_SRC_MAX, CCBS_IND_PossibilityTimedOut,
        CCBS_STAT_NotPresent, NULL );
      break;  /* skip the rest */
    }
    if (ctb->CCBSstat EQ CCBSS_REQ)
    {
      break;  /* skip the rest */
    }
    if (ctb->calType EQ CT_NI_MOC)
    {
      cmhrat_ccbs( CMD_SRC_MAX, CCBS_IND_RecallTimedOut,
        CCBS_STAT_NotPresent, NULL );
      break;  /* skip the rest */
    }

    /*  call clean up */
    switch( calltype )
    {
    case( VOICE_CALL ):
      ;
      break;

#ifdef FF_GPF_TCPIP
    case( TCPIP_CALL ):
      if( ccShrdPrm.datStat NEQ DS_IDL )
      {
        ccShrdPrm.datStat = DS_STOP_REQ;
        pppShrdPrm.state = PPP_UNDEFINED; /* Otherwise PPP and TCPIP doesn't go down well */
        cmhPPP_Terminate(DWN);
        ctb->curCmd = AT_CMD_NONE;
      }
      break;
#endif /* FF_GPF_TCPIP */

#if defined (FAX_AND_DATA) AND defined (DTI)
    case( TRANS_CALL ):
      if( ccShrdPrm.datStat NEQ DS_IDL )
      {
        ccShrdPrm.datStat = DS_STOP_REQ;
        cmhTRA_Deactivate ();
      }
      break;

    case( NON_TRANS_CALL ):
      if( ccShrdPrm.datStat NEQ DS_IDL )
      {
        ccShrdPrm.datStat = DS_STOP_REQ;
        cmhL2R_Deactivate ();
      }
      break;

#ifdef CO_UDP_IP
    case( UDPIP_CALL ):
      if( ccShrdPrm.datStat NEQ DS_IDL )
      {
        ccShrdPrm.datStat = DS_STOP_REQ;
        cmhPPP_Terminate(DWN);
        ctb->curCmd = AT_CMD_NONE;
      }
      break;
#endif  /* CO_UDP_IP */

#ifdef FF_FAX
    case( FAX_CALL ):
      if( ccShrdPrm.datStat NEQ DS_IDL )
      {
        ccShrdPrm.datStat = DS_STOP_REQ;
        cmhT30_Deactivate ();
      }
#endif /* FF_FAX */
#endif /* FAX_AND_DATA */

    }
    cmhCC_SndDiscRsn( cId );
    break;

    case( AT_CMD_H ):
    case( AT_CMD_Z ):
    case( AT_CMD_CHUP ):
    case( AT_CMD_ABRT ):
    case( AT_CMD_CHLD ):

#ifdef FF_FAX
    case( AT_CMD_FKS ):
    case( AT_CMD_FDT ):
    case( AT_CMD_FDR ):
#endif
    /*
    *----------------------------------------------------------------
    * process event for H, Z and +CHUP command
    *----------------------------------------------------------------
      */
      /*
      * Bugfix for DISCONNECT_IND in CC U3 with PROGRESS_IND #8
      */
      cmdBuf = ctb->curCmd;
      srcBuf = ctb->curSrc;
      pCCCmdPrm = &cmhPrm[srcBuf].ccCmdPrm;

      cmhCC_tstAndUnflagCall( cId, &(pCCCmdPrm->mltyCncFlg) );
      cmhCC_tstAndUnflagCall( cId, &(pCCCmdPrm->mltyDscFlg) );

      if ( cmdBuf EQ AT_CMD_Z           AND
        pCCCmdPrm -> mltyDscFlg EQ 0 )
      {
        cmh_Reset ( (T_ACI_CMD_SRC)srcBuf, TRUE );
        R_AT( RAT_Z, (T_ACI_CMD_SRC)srcBuf)();
      }

      cmhCC_PrepareCmdEnd (cId, NULL, NULL);

      if (ctb->curCmd NEQ AT_CMD_CHLD)      /* Do not emmit OK for CHLD since this would
                                               occour in multiple OKs for a multi-party call */
      {
        R_AT( RAT_OK, (T_ACI_CMD_SRC)srcBuf ) ( cmdBuf );
      }

      /* log result */
      cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_OK, (T_ACI_AT_CMD)cmdBuf, (SHORT)(cId+1), 
                               BS_SPEED_NotPresent,CME_ERR_NotPresent );
      break;
  }

  /* call progress information */
  /* Implements Measure 207 */
  cmhCC_Call_Progress_Information( cId, CPI_MSG_Rls, ctb->curCs );
 
  if (rdlPrm.rdlcId NEQ cId)
  { /* if redialling is active no clean cId entry */
    /* declare the call table entry as unused */
    psaCC_FreeCtbNtry (cId);
  }
  return;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                  |
| ROUTINE : cmhCC_CallReleaseIndication                                |
+-------------------------------------------------------------------+

  PURPOSE : call release indication to give out correct call process information
                 interface so that psa function can report a release ind
*/

GLOBAL void cmhCC_CPIReleaseMsg ( SHORT cId )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
  TRACE_FUNCTION ("cmhCC_CPIReleaseMsg()");

  /* call progress information */
  /* Implements Measure 207 */
  cmhCC_Call_Progress_Information( cId, CPI_MSG_Rls, ctb->curCs );

  return;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_CPIrejectMsg           |
+-------------------------------------------------------------------+

  PURPOSE : interface so that psa function can report a reject ind.

*/
GLOBAL void cmhCC_CPIrejectMsg ( SHORT cId )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

  TRACE_FUNCTION ("cmhCC_CPIrejectMsg()");

  /* Implements Measure 207 */
  cmhCC_Call_Progress_Information( cId, CPI_MSG_Rjct, ctb->curCs );
    /*************************
  call was not serviced yet
  ***************************/
  /* Implements Measure 32 */
  if (cmhCC_check_isCallServiced( cId ) EQ TRUE)
  {
    return;
  }

  return;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_CallModified           |
+-------------------------------------------------------------------+

  PURPOSE : call was modified

*/

GLOBAL void cmhCC_CallModified ( SHORT cId )
{
#if defined (FAX_AND_DATA)
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
  T_CC_CALL_TYPE calltype = cmhCC_getcalltype(cId);
  UBYTE cmdBuf;               /* buffers current command */
  UBYTE srcBuf;               /* buffers current command source */

  TRACE_FUNCTION ("cmhCC_CallModified()");

  /* check for command context */
  switch (ctb->curCmd )
  {
    case( AT_CMD_D ):
    case( AT_CMD_A ):
    case( AT_CMD_H ):
      /* process event for D, A and H command */
      cmdBuf = ctb->curCmd;
      srcBuf = ctb->curSrc;
      break;

    default:
      cmdBuf = AT_CMD_NONE;
      srcBuf = ctb->calOwn;
      break;
  }

  /* determine modifaction status */
  switch (ctb->rslt)
  {
    /* successful modification */
    case( MNCC_CAUSE_MODIFY_SUCCESS ):

      switch( calltype )
      {
        /* modified to speech call */
        case( VOICE_CALL ):
          cmhCC_PrepareCmdEnd (cId, NULL, NULL);

          R_AT( RAT_OK, (T_ACI_CMD_SRC)srcBuf ) ( cmdBuf );

          /* log result */
          cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_OK,
                      (T_ACI_AT_CMD)cmdBuf, (SHORT)(cId+1), BS_SPEED_NotPresent,CME_ERR_NotPresent );
          break;

        /*  modified to transparent or non-transparent or fax data call */
        case( TRANS_CALL ):
        case( NON_TRANS_CALL ):
        case( FAX_CALL ):
          ccShrdPrm.datStat = DS_ACT_REQ;

          if( cmhRA_Activate  ( (T_ACI_CMD_SRC)srcBuf, (T_ACI_AT_CMD)cmdBuf, cId ) NEQ AT_EXCT )
          {
            ctb->nrmCs = MNCC_CAUSE_CALL_CLEAR;
            psaCC_ClearCall (cId);
          }
          break;
      }
      break;

    /* unsuccessful modification */
    case( MNCC_CAUSE_MS_TIMER ):
    default: /* network can send any cause */

      TRACE_EVENT_P1 ("MODIFICATION FAILURE: cause=%d",ctb->rslt);


      R_AT( RAT_CME, (T_ACI_CMD_SRC)srcBuf ) ( cmdBuf, CME_ERR_NotPresent );

      /* log result */
      cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_CME, (T_ACI_AT_CMD)cmdBuf, -1, 
                              BS_SPEED_NotPresent,CME_ERR_NotPresent );

      if( calltype EQ TRANS_CALL     OR
          calltype EQ NON_TRANS_CALL OR
          calltype EQ FAX_CALL )
      {
        /* back to transparent or non-transparent or fax data call */
        ccShrdPrm.datStat = DS_REST_REQ;

        if( cmhRA_Activate  ( (T_ACI_CMD_SRC)srcBuf, (T_ACI_AT_CMD)cmdBuf, cId ) NEQ AT_EXCT )
        {
          ctb->nrmCs = MNCC_CAUSE_CALL_CLEAR;
          psaCC_ClearCall (cId);
        }
      }
      break;
  }
#endif /* FAX_AND_DATA */
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_DTMFsent               |
+-------------------------------------------------------------------+

  PURPOSE : confirmation for sent DTMF

*/

LOCAL void dtmf_sent_after_vts ( SHORT cId )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
  UBYTE current_cmd_source = ctb->dtmfSrc; /* buffers current command source */

  TRACE_FUNCTION ("dtmf_sent_after_vts()");

  ctb->dtmfCmd = AT_CMD_NONE;
  ctb->dtmfSrc = (T_OWN)CMD_SRC_NONE;

  ccShrdPrm.dtmf.cId = NO_ENTRY;
  ccShrdPrm.dtmf.cnt = 0;

  if ((GET_CAUSE_VALUE(ctb->nrmCs) NEQ NOT_PRESENT_8BIT) AND
      (ctb->nrmCs NEQ MNCC_CAUSE_DTMF_START_SUCCESS) AND
      (ctb->nrmCs NEQ MNCC_CAUSE_DTMF_STOP_SUCCESS))
  {
    TRACE_EVENT_P1("DTMF response cause value: %d", ctb->nrmCs);
    R_AT( RAT_CME, (T_ACI_CMD_SRC)current_cmd_source ) ( AT_CMD_VTS, CME_ERR_NotPresent );
    return;
  }

  R_AT( RAT_OK, (T_ACI_CMD_SRC)current_cmd_source ) ( AT_CMD_VTS );
}

LOCAL void dtmf_sent_within_number ( SHORT cId )
{
  BOOL  dtmf_sent;    /* result of sending of next DTMF char */

  TRACE_FUNCTION ("dtmf_sent_within_number()");

  if ((ccShrdPrm.dtmf.cId NEQ cId) OR
      !is_call_ok_for_dtmf(cId))
  {
    TRACE_EVENT_P2("Wrong cId for send DTMF. ccShrdPrm.dtmf.cId: %d, cId: %d", ccShrdPrm.dtmf.cId, cId);
    return;
  }

  while( ccShrdPrm.dtmf.cnt                      AND
         ccShrdPrm.dtmf.cur < ccShrdPrm.dtmf.cnt )
  {
    dtmf_sent = cmhCC_SendDTMFdig ( AT_CMD_NONE,
                                    cId,
                                    ccShrdPrm.dtmf.dig[ccShrdPrm.dtmf.cur],
                                    MNCC_DTMF_MOD_AUTO );
    ccShrdPrm.dtmf.cur++;
    if( dtmf_sent )
    {
      return;
    }
  }

  TRACE_EVENT_P1("End of sending DTMF tones: %d", ccShrdPrm.dtmf.dig[ccShrdPrm.dtmf.cur]);
  ccShrdPrm.dtmf.cnt = ccShrdPrm.dtmf.cur = 0;
  ccShrdPrm.dtmf.cId = NO_ENTRY;
}

GLOBAL void cmhCC_DTMFsent ( SHORT cId )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

  TRACE_FUNCTION ("cmhCC_DTMFsent()");

  switch( ctb->dtmfCmd )
  {
    case( AT_CMD_VTS ):
      dtmf_sent_after_vts(cId);
      return;

    case( AT_CMD_NONE ):
      dtmf_sent_within_number(cId);
      return;

    default:
      TRACE_EVENT_P1("Unexpected DTMF command value: %d", ctb->dtmfCmd);
      break;
  }
}


/*
    This will be called when the call was disconnected while sending DTMF tones

*/
GLOBAL void cmhCC_DTMFstopped ( SHORT cId )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

  TRACE_FUNCTION ("cmhCC_DTMFstopped()");

  switch( ctb->dtmfCmd )
  {
  case( AT_CMD_VTS ):
    TRACE_EVENT ("DTMF stopped within VTS");
    R_AT( RAT_CME,(T_ACI_CMD_SRC)ctb->dtmfSrc )
            ( AT_CMD_VTS, CME_ERR_Unknown );
    return;

  case( AT_CMD_NONE ):
    TRACE_EVENT ("DTMF stopped within NONE");

    return;

  default:
    TRACE_EVENT_P1("Unexpected DTMF command value: %d", ctb->dtmfCmd);
    break;
  }
}


#if 0 // HM 11-May-2005: Dead code
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_DisconnectCall         |
+-------------------------------------------------------------------+

  PURPOSE : request to disconnect the call (in-band tones available)

*/

GLOBAL void cmhCC_DisconnectCall ( SHORT cId )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
  T_CC_CMD_PRM  *pCCCmdPrm;   /* points to CC command parameters */
  UBYTE cmdBuf;               /* buffers current command */
  UBYTE srcBuf;               /* buffers current command source */

  TRACE_FUNCTION ("cmhCC_DisconnectCall()");

  /* check for command context */
  switch (ctb->curCmd)
  {
    case( AT_CMD_NONE ):
    case( AT_CMD_D ):
    case( AT_CMD_A ):
     /*
      *----------------------------------------------------------------
      * process event for D, A and no command
      *----------------------------------------------------------------
      */
      /* for data calls proceed with call completion */
      if( cmhCC_getcalltype(cId) NEQ VOICE_CALL )
      {
        psaCC_ClearCall (cId);
      }
      break;

    case( AT_CMD_H ):
    case( AT_CMD_Z ):
    case( AT_CMD_CHUP ):

     /*
      *----------------------------------------------------------------
      * process event for H, Z and +CHUP command
      *----------------------------------------------------------------
      */
      cmdBuf = ctb->curCmd;
      srcBuf = ctb->curSrc;

      psaCC_ClearCall (cId);

      cmhCC_PrepareCmdEnd (cId, NULL, NULL);

      pCCCmdPrm = &cmhPrm[srcBuf].ccCmdPrm;

      if(cmdBuf EQ AT_CMD_Z           AND
         pCCCmdPrm -> mltyDscFlg EQ 0 )
      {
        cmh_Reset ( srcBuf, TRUE );
        R_AT( RAT_Z, srcBuf)();
      }

      R_AT( RAT_OK, (T_ACI_CMD_SRC)srcBuf ) ( (T_ACI_AT_CMD)cmdBuf );

      /* log result */
      cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_OK,
                    (T_ACI_AT_CMD)cmdBuf, (SHORT)(cId+1), BS_SPEED_NotPresent, CME_ERR_NotPresent );
      break;
  }

  return;
}
#endif /* #if 0 */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_IncomingCall           |
+-------------------------------------------------------------------+

  PURPOSE : indication of a mobile terminated call

*/

GLOBAL void cmhCC_IncomingCall ( SHORT cId )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
  SHORT idx;                            /* holds index  counter */
  T_ACI_CRING_MOD mode;                 /* holds mode of call */
  T_ACI_CRING_SERV_TYP type1, type2;         /* holds types of bearer services */
  CHAR numBuf[MAXIMUM(MNCC_MAX_CC_CALLING_NUMBER, MNCC_MAX_CC_REDIR_NUMBER)+1];
                                        /* buffers calling number + '\0' */
  CHAR subBuf[MNCC_SUB_LENGTH+1];            /* buffers subaddress 'subaddress\0' */
  T_ACI_TOA toaBuf;                     /* buffers type of address */
  T_ACI_TOS tosBuf;                     /* buffers type of subaddress */
  USHORT listcounter;
  T_ACI_LIST *msg2send;

  TRACE_FUNCTION ("cmhCC_IncomingCall()");

/*
 *-------------------------------------------------------------------
 * ring for call, if no call is in use
 *-------------------------------------------------------------------
 */
  if( ! psaCC_ctbCallInUse() )
  {

    switch (ctb->rptInd) /* determine call mode */
    {
      case( MNCC_RI_NOT_PRES ):
      case( MNCC_RI_SEQUENTIAL ):
        mode = CRING_MOD_Direct;
        break;
      case( MNCC_RI_CIRCULAR ):
        mode = CRING_MOD_Alternate;
        break;

      default:
        TRACE_EVENT("UNEXP REPEAT INDICATOR IN CTB");
        mode = CRING_MOD_NotPresent;
        break;
    }

    /* determine 1st service */
    type1 = (T_ACI_CRING_SERV_TYP)cmhCC_GetCallType_from_bearer( &ctb->BC[0] );

    /* determine 2nd service */
    type2 = (T_ACI_CRING_SERV_TYP)cmhCC_GetCallType_from_bearer( &ctb->BC[1] );

#ifdef FF_ATI
    io_setRngInd (IO_RING_ON, type1, type2); /* V.24 Ring Indicator Line */
#endif

    for( idx = 0; idx < CMD_SRC_MAX; idx++ )
    {
      if (ctb->prgDesc EQ MNCC_PROG_NOT_PRES)
      {
        R_AT( RAT_CPI, (T_ACI_CMD_SRC)idx )
          ( cId+1,
            CPI_MSG_Setup,
            (ctb->inBndTns)? CPI_IBT_True: CPI_IBT_False,
            (ccShrdPrm.TCHasg)? CPI_TCH_True: CPI_TCH_False,
            ctb->curCs );
      }
      R_AT( RAT_CRING, (T_ACI_CMD_SRC)idx )( mode, type1, type2 );

      /* calling line presentation, if no call is in use */

      /* check of presentation indicator removed, due to the fact that
       * other mobiles show the same behaviour.
      if( ctb->clgPty.present NEQ PRES_NUMB_NOT_AVAIL ){} */

      R_AT( RAT_CLIP, (T_ACI_CMD_SRC)idx )
        ( CLIP_STAT_NotPresent,
          psaCC_ctbClrAdr2Num( cId, numBuf, sizeof (numBuf) ),
          cmhCC_ctbGetClrNumTyp( cId, &toaBuf ),
          ctb->clgPty.present,
          psaCC_ctbClrAdr2Sub( cId, subBuf ),
          cmhCC_ctbGetClrSubTyp( cId, &tosBuf ),
          psaCC_ctbGetAlpha( cId ) );

      R_AT ( RAT_CDIP, (T_ACI_CMD_SRC)idx )
           (psaCC_ctbCldAdr2Num( cId, numBuf, sizeof (numBuf) ),
            cmhCC_ctbGetCldNumTyp( cId, &toaBuf ),
          	psaCC_ctbCldAdr2Sub( cId, subBuf ),
          	cmhCC_ctbGetCldSubTyp( cId, &tosBuf ));
      
    } /* for */

    if (ctb->CDStat EQ CD_Notified)
    {
      send_CSSX_notification(cId,
          CSSX_CODE_DeflectedCall,
          ACI_NumParmNotPresent,
          psaCC_ctbRdrAdr2Num( cId, numBuf, sizeof (numBuf) ),
          cmhCC_ctbGetRdrNumTyp( cId, &toaBuf ),
          psaCC_ctbRdrAdr2Sub( cId, subBuf ),
          cmhCC_ctbGetRdrSubTyp( cId, &tosBuf));
    }

    if (ctb->CDStat EQ CD_Notified)
    {
      psaCC_FreeRdrPty (cId);
      ctb->CDStat = NO_VLD_CD;
    }
    /* check for TTY service */
    cmhCC_TTY_Control ( cId, TTY_START );

    /* Find out if there is any MNCC_FACILITY_IND stored */
    listcounter  = 0;
    if ((listcounter = get_list_count(ccShrdPrm.facility_list)) > 0)
    {
      for (;listcounter > 0 ; listcounter --)
      {
        msg2send = remove_first_element(ccShrdPrm.facility_list);

        /* Send  MNCC_FACILITY_IND */
        psa_mncc_facility_ind((T_MNCC_FACILITY_IND *)msg2send);
      }
    }

  } /* endif */


/*
 *-------------------------------------------------------------------
 * call waiting indication, if a call is in use
 *-------------------------------------------------------------------
 */
  else
  {
    /* check of presentation indicator removed, due to the fact that
       other mobiles show the same behaviour.
    if( ctb->clgPty.present NEQ PRES_NUMB_NOT_AVAIL ) {} */

    if (ctb->CDStat EQ CD_Notified)
    {
      send_CSSX_notification( cId,
                              CSSX_CODE_DeflectedCall,
                              ACI_NumParmNotPresent,
                              psaCC_ctbRdrAdr2Num( cId, numBuf, sizeof (numBuf) ),
                              cmhCC_ctbGetRdrNumTyp( cId, &toaBuf ),
                              psaCC_ctbRdrAdr2Sub( cId, subBuf ),
                              cmhCC_ctbGetRdrSubTyp( cId, &tosBuf ));
    }

    for( idx = 0; idx < CMD_SRC_MAX; idx++ )
    {
      if (ctb->prgDesc EQ MNCC_PROG_NOT_PRES)
      {
        R_AT( RAT_CPI, (T_ACI_CMD_SRC)idx )
          ( cId+1,
            CPI_MSG_Setup,
            (ctb->inBndTns)? CPI_IBT_True: CPI_IBT_False,
            (ccShrdPrm.TCHasg)? CPI_TCH_True: CPI_TCH_False,
            ctb->curCs );
      }

      R_AT( RAT_CCWA, (T_ACI_CMD_SRC)idx )
        ( NULL,
          psaCC_ctbClrAdr2Num( cId, numBuf, sizeof (numBuf) ),
          cmhCC_ctbGetClrNumTyp( cId, &toaBuf ),
          ctb->clgPty.present,
          cmhCC_GetCallClass( cId ),
          psaCC_ctbGetAlpha( cId ) );
    }
    if (ctb->CDStat EQ CD_Notified)
    {
      psaCC_FreeRdrPty (cId);
      ctb->CDStat = NO_VLD_CD;
    }
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : psaCC_Synchronized           |
+-------------------------------------------------------------------+

  PURPOSE : channel mode has changed or
            call reestablishment event received.
            If a channel mode change is indicated cId needs not
            to be valid.

*/

GLOBAL void cmhCC_Synchronized ( SHORT cId ) 
{
  SHORT act_cId;      /* holds call identifier */
#if defined (FAX_AND_DATA) AND defined (DTI)
  T_CC_CALL_TYPE calltype;
#endif    /* of #ifdef FAX_AND_DATA */

  TRACE_FUNCTION ("cmhCC_Synchronized()");

  /* process event */
  switch( ccShrdPrm.syncCs )
  {
    case( MNCC_CAUSE_CHANNEL_SYNC ):   /* change of TCH state */

      if( ccShrdPrm.TCHasg EQ TRUE )
      {
        /* search for an active call */
        if((act_cId = psaCC_ctbFindCall( OWN_SRC_INV, CS_ACT, NO_VLD_CT ))
            NEQ NO_ENTRY )
        {
#if defined (FAX_AND_DATA) AND defined (DTI)
          calltype = cmhCC_getcalltype(act_cId);

          switch( calltype )
          {
#ifdef FF_FAX
          case( FAX_CALL ):
              switch( ccShrdPrm.datStat )
              {
                case( DS_ACT ):
                case( DS_TCH_MDF ):
                  ccShrdPrm.datStat = DS_TCH_MDF;
                  /*
                   * use entity owner instead of current source, because
                   * command is out of scope for CC
                   */
                  if( cmhRA_Modify ( raEntStat.entOwn, act_cId ) NEQ AT_EXCT )
                  {
                    ccShrdPrm.datStat = DS_IDL;
                    cmhT30_Deactivate();
                    psaCC_ctb(act_cId)->nrmCs = MNCC_CAUSE_CALL_CLEAR;
                    psaCC_ClearCall (act_cId);
                  }
                  break;

                case( DS_IDL ):
                  ccShrdPrm.datStat = DS_ACT_REQ;
                  if( cmhRA_Activate  ( (T_ACI_CMD_SRC)psaCC_ctb(act_cId)->curSrc,
                                        (T_ACI_AT_CMD)psaCC_ctb(act_cId)->curCmd,
                                        act_cId )
                      NEQ AT_EXCT )
                  {
                    ccShrdPrm.datStat = DS_IDL;
                    psaCC_ctb(act_cId)->nrmCs = MNCC_CAUSE_CALL_CLEAR;
                    psaCC_ClearCall (act_cId);
                  }
            }
            break;
#endif /* FF_FAX */  

            case( TRANS_CALL ):    /* activate RA */
            case( NON_TRANS_CALL ):    /* activate RA */
              switch( ccShrdPrm.datStat )
              {
                case( DS_IDL ):
                  ccShrdPrm.datStat = DS_ACT_REQ;
                  if( cmhRA_Activate  ( (T_ACI_CMD_SRC)psaCC_ctb(act_cId)->curSrc,
                                        (T_ACI_AT_CMD)psaCC_ctb(act_cId)->curCmd,
                                        act_cId )
                      NEQ AT_EXCT )
                  {
                    ccShrdPrm.datStat = DS_IDL;
                    psaCC_ctb(act_cId)->nrmCs = MNCC_CAUSE_CALL_CLEAR;
                    psaCC_ClearCall (act_cId);
                  }
                  break;

                default:
                  TRACE_EVENT("modify data call ignored");
                  break;
            }
            break;

            case( UNSUP_DATA_CALL  ):
              TRACE_EVENT("UNSUPPORTED DATA CALL -> DISC CALL");
              psaCC_ctb(act_cId)->nrmCs = MNCC_CAUSE_CALL_CLEAR;
              psaCC_ClearCall (act_cId);
              break;
          }
#endif /* FAX_AND_DATA */
        }
      }

      /* Information about Traffic Channel Assignment should be sent
      for all existing act_cIds */
      for( act_cId = 0; act_cId < MAX_CALL_NR; act_cId++ )
      {
        if (ccShrdPrm.ctb[act_cId] NEQ NULL)
        {
          /* Implements Measure 207 */
          cmhCC_Call_Progress_Information( act_cId, CPI_MSG_Sync, psaCC_ctb(act_cId)->curCs );
        }
      }
      break;

    case( MNCC_CAUSE_REEST_STARTED ):                /* call reestablishment */
    case( MNCC_CAUSE_REEST_FINISHED ):
/* Implements Measure#32: Row 81, 82 */
      if(ccShrdPrm.syncCs  EQ MNCC_CAUSE_REEST_STARTED)
      {
        TRACE_EVENT("SYNC -> CALL REESTABLISHMENT STARTED"); 
      }
      else
      {
        TRACE_EVENT("SYNC -> CALL REESTABLISHMENT FINISHED");
      }
      if (psaCC_ctbIsValid (cId))
      {
      /* Send %CPI notification for call reestablishment */
      /* Implements Measure 207 */
      cmhCC_Call_Progress_Information( cId, CPI_MSG_Sync, psaCC_ctb(cId)->curCs );
      }
      else
      {
        TRACE_ERROR ("No call table entry");
      }
      break;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : psaCC_CallHeld               |
+-------------------------------------------------------------------+

  PURPOSE : call hold result

*/

GLOBAL void cmhCC_CallHeld ( SHORT cId )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
  T_CC_CMD_PRM * pCCCmdPrm;   /* points to CC command parameters */
  UBYTE cmdBuf;               /* buffers current command */
  UBYTE srcBuf;               /* buffers current command source */
  SHORT addId;                /* holds additional call id */
  TRACE_FUNCTION ("cmhCC_CallHeld()");

  /* check for command context */
  switch (ctb->curCmd )
  {
    case( AT_CMD_D    ):
    case( AT_CMD_A    ):
    case( AT_CMD_CHLD ):
      break;

    default:
      TRACE_EVENT_P1("Error: wrong command context: %d", ctb->curCmd);
      return;  /* wrong command context */
  }

  cmhCC_PrepareCmdEnd (cId, &cmdBuf, &srcBuf);
  pCCCmdPrm = &cmhPrm[srcBuf].ccCmdPrm;

  /* check result of hold request */
  TRACE_EVENT_P1("Hold CNF cause value: %04X", ctb->rslt);
  switch( ctb->rslt )
  {
    case( MNCC_CAUSE_HOLD_SUCCESS ):
    case( MNCC_CAUSE_SUCCESS ):
    case( MNCC_CAUSE_NO_MS_CAUSE ):

      /* Held information displayed if call is on hold */
      /* Implements Measure 207 */
      cmhCC_Call_Progress_Information( cId, CPI_MSG_Hld, ctb->rslt );

      if( cmhCC_tstAndUnflagCall( cId, &(pCCCmdPrm -> mltyCncFlg)))
      {
        if((pCCCmdPrm -> CHLDmode EQ CHLD_MOD_HldActExc OR
            pCCCmdPrm -> CHLDmode EQ CHLD_MOD_HldActAndAcpt OR
            pCCCmdPrm -> CHLDmode EQ CHLD_MOD_HldActDial) AND
            pCCCmdPrm -> mltyCncFlg )
        {
          process_CHLDaddInfo( srcBuf, CALL_HELD );
        }

        if( pCCCmdPrm -> mltyCncFlg EQ 0 AND
            pCCCmdPrm -> mltyDscFlg EQ 0     )  /* if last call */
        {
#ifdef SIM_TOOLKIT
          if (ctb->SATinv )
          {
            cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_OK,
                      (T_ACI_AT_CMD)cmdBuf, (SHORT)(cId+1), BS_SPEED_NotPresent,CME_ERR_NotPresent );

            cmhSAT_StartPendingCall( );
          }
          else
#endif /* SIM_TOOLKIT */
          {
            R_AT( RAT_OK, (T_ACI_CMD_SRC)srcBuf ) ( cmdBuf );

            /* log result */
            cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_OK,
                (T_ACI_AT_CMD)cmdBuf, (SHORT)(cId+1), BS_SPEED_NotPresent,CME_ERR_NotPresent);
          }
        }
      }
      break;

    default: /* no positive result, i.e. negative result of Hold operation */

      if( cmhCC_tstAndUnflagCall( cId, &(pCCCmdPrm -> mltyCncFlg)))
      {

#ifdef SIM_TOOLKIT
        if (ctb->SATinv)
        {
          psaCC_ctb(ccShrdPrm.cIdMPTY)->SATinv = FALSE;
          /* return network error cause GSM 11.14 / 12.12.3 */
          cmhSAT_NtwErr( ADD_NO_CAUSE );
        }
#endif
        if( pCCCmdPrm -> CHLDmode EQ CHLD_MOD_HldActDial AND
            pCCCmdPrm -> mltyCncFlg )
        {
          for( addId = 0; !((pCCCmdPrm -> mltyCncFlg >> addId) & 0x01); addId++ )
            ;

          if( CHLDaddInfo EQ CHLD_ADD_INFO_DIAL_CAL )
          {
              CHLDaddInfo = NO_CHLD_ADD_INFO;
              psaCC_FreeCtbNtry (addId);
          }
        }

        pCCCmdPrm -> mltyCncFlg = 0;        /* unflag calls */
        CHLDaddInfo = NO_CHLD_ADD_INFO;

        R_AT( RAT_CME, (T_ACI_CMD_SRC)srcBuf ) ( cmdBuf, CME_ERR_OpNotAllow );

        /* log result */
        cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_CME,
                      (T_ACI_AT_CMD)cmdBuf, -1, BS_SPEED_NotPresent, CME_ERR_OpNotAllow );
      }
      break;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : psaCC_CallRetrieved          |
+-------------------------------------------------------------------+

  PURPOSE : call retrieve result

*/

GLOBAL void cmhCC_CallRetrieved ( SHORT cId )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
  T_CC_CMD_PRM * pCCCmdPrm;   /* points to CC command parameters */
  UBYTE cmdBuf;               /* buffers current command */
  UBYTE srcBuf;               /* buffers current command source */

  TRACE_FUNCTION ("cmhCC_CallRetrieved()");

  /* check for command context */
  if (ctb->curCmd NEQ AT_CMD_CHLD)
  {
    return;
  }

  pCCCmdPrm = &cmhPrm[ctb->curSrc].ccCmdPrm;
  cmhCC_PrepareCmdEnd (cId, &cmdBuf, &srcBuf);

  /*
   *  check result of retrieve request
   */
  switch (ctb->rslt)
  {
    case( MNCC_CAUSE_RETRIEVE_SUCCESS ):
      /* Implements Measure 184, 185 and 187 */
      cmhCC_tstAndUnflag_MPTYCall( cId, srcBuf, cmdBuf, pCCCmdPrm );
      break;

    default: /* no positive result of retrieve operation */

      if( cmhCC_tstAndUnflagCall( cId, &(pCCCmdPrm -> mltyCncFlg)))
      {
        pCCCmdPrm -> mltyCncFlg = 0;        /* unflag calls */

        R_AT( RAT_CME, (T_ACI_CMD_SRC)srcBuf ) ( cmdBuf, CME_ERR_OpNotAllow );

        /* log result */
        cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_CME,
                      (T_ACI_AT_CMD)cmdBuf, -1, BS_SPEED_NotPresent, CME_ERR_OpNotAllow );
      }
      break;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_RA_Activated           |
+-------------------------------------------------------------------+

  PURPOSE : RA entity activated, data path established

*/
#ifdef DTI
GLOBAL void cmhCC_RA_Activated ( SHORT cId )
{
#ifdef FAX_AND_DATA
 
  UBYTE cmdBuf;               /* buffers current command */
  UBYTE srcBuf;               /* buffers current command source */
  T_CC_CALL_TYPE calltype = cmhCC_getcalltype(cId);

  TRACE_FUNCTION ("cmhCC_RA_Activated()");

  srcBuf = psaCC_ctb(cId)->curSrc;
  cmdBuf = psaCC_ctb(cId)->curCmd;

  switch( ccShrdPrm.datStat )
  {
  case( DS_ACT_REQ ):
  case( DS_REST_REQ ):
    switch( calltype )
    {
    /* transparent data call */
    case( TRANS_CALL ):
      psaTRA_Activate( );
      break;

    /* non-transparent data call */
    case( NON_TRANS_CALL ):
    /* a WAP call is a non-transparent data call */
#ifdef CO_UDP_IP 
    case( UDPIP_CALL ):
#endif /* CO_UDP_IP */
#ifdef FF_PPP
    case( PPP_CALL ):
#endif /* FF_PPP */
#if defined(FF_GPF_TCPIP)
    case (TCPIP_CALL):
#endif /* FF_GPF_TCPIP */

      if( cmhL2R_Activate((T_ACI_CMD_SRC)srcBuf, (T_ACI_AT_CMD)cmdBuf, cId) NEQ AT_EXCT )
      {
        ccShrdPrm.datStat = DS_DSC_REQ;
        cmhRA_Deactivate ();
      }
      break;

#ifdef FF_FAX
    case( FAX_CALL ):
      if( cmhT30_Activate((T_ACI_CMD_SRC)srcBuf, (T_ACI_AT_CMD)cmdBuf, cId) NEQ AT_EXCT )
      {
        ccShrdPrm.datStat = DS_DSC_REQ;
        cmhRA_Deactivate ();
      }
      break;
#endif /* FF_FAX */

   /* in case of other types of call */
    default:
      ccShrdPrm.datStat = DS_DSC_REQ;
      cmhRA_Deactivate ();
      break;
    }
    break;
  }
#endif    /* of #ifdef FAX_AND_DATA */
}
#endif /* DTI */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_RA_Deactivated         |
+-------------------------------------------------------------------+

  PURPOSE : RA entity deactivated, data path disconnected

*/
#ifdef DTI
GLOBAL void cmhCC_RA_Deactivated ( SHORT cId )
{
#ifdef FAX_AND_DATA
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
  T_CC_CALL_TYPE calltype = cmhCC_getcalltype(cId);
  UBYTE cmdBuf;               /* buffers current command */

  TRACE_FUNCTION ("cmhCC_RA_Deactivated()");

  cmdBuf = ctb->curCmd;

  switch( calltype )
  {
    case( TRANS_CALL ):
     /* transparent data call */
      switch( ccShrdPrm.datStat )
      {
        case( DS_ACT ):
        case( DS_ACT_REQ ):
        case( DS_DSC_REQ ):
        case( DS_TCH_MDF ):

          ccShrdPrm.datStat = DS_IDL;
          ctb->nrmCs = MNCC_CAUSE_CALL_CLEAR;
          psaCC_ClearCall (cId);
#if defined (SIM_TOOLKIT) AND defined (FF_SAT_E)
              /* check impact for SAT commands */
              cmhSAT_OpChnCSDDown( cId, TPL_NONE );
#endif /* SIM TOOLKIT AND FF_SAT_E */
          break;

        case( DS_MDF_REQ ):

          ccShrdPrm.datStat = DS_IDL;
          psaCC_ModifyCall(cId);
          break;

        case( DS_STOP_REQ ):

          ccShrdPrm.datStat = DS_IDL;
          cmhCC_CallDisconnected(cId);
          break;
      }
      break;

     /* non-transparent data call */
    case( NON_TRANS_CALL ):
#ifdef CO_UDP_IP
    case( UDPIP_CALL ):
#endif /* CO_UDP_IP */
#ifdef FF_PPP
    case( PPP_CALL ):
#endif /* FF_PPP */
#ifdef FF_GPF_TCPIP
    case (TCPIP_CALL):
#endif /* FF_GPF_TCPIP */

      switch( ccShrdPrm.datStat )
      {
        case( DS_ACT ):
        case( DS_ACT_REQ ):
        case( DS_DSC_REQ ):
        case( DS_REST_REQ ):
        case( DS_TCH_MDF ):

          ccShrdPrm.datStat        = DS_IDL;
          ctb->nrmCs        = MNCC_CAUSE_CALL_CLEAR;

#if defined(CO_UDP_IP) OR defined(FF_GPF_TCPIP)
          /* WAP STACK is down */

          /* The following statements check which configuration is active,
             because there are three possibilities:
             1. UDP/IP only - WAP1.x and SAT_E
             2. TCP/IP only - WAP2.x and higher and App's
             3. UDP/IP and TCP/IP - WAP2.x and higher and App's and SAT_E 
          */
#if defined(FF_GPF_TCPIP) AND !defined(CO_UDP_IP)
          if(calltype EQ TCPIP_CALL)
#elif !defined(FF_GPF_TCPIP) AND defined(CO_UDP_IP)
          if(calltype EQ UDPIP_CALL)
#elif defined(FF_GPF_TCPIP) AND defined(CO_UDP_IP)
          if(calltype EQ UDPIP_CALL OR calltype EQ TCPIP_CALL)
#endif
          {
#if defined (SIM_TOOLKIT) AND defined (FF_SAT_E)
            /* check impact for SAT commands */
            cmhSAT_OpChnCSDDown( cId, UDP );
#endif /* SIM TOOLKIT AND FF_SAT_E */
            /* Have CC disconnected before ? */
            if (ctb->calStat EQ CS_ACT)
            {
              psaCC_ClearCall (cId);
            }
            else
            {
               /* Message to MMI */
              R_AT( RAT_OK, (T_ACI_CMD_SRC)ctb->calOwn ) ( AT_CMD_NONE, cId+1 );

#ifdef FF_ATI
              io_setDCD ((T_ACI_CMD_SRC)ctb->calOwn, IO_DCD_OFF); /* V.24 DCD Line */
#endif
              /* log result */
              cmh_logRslt ((T_ACI_CMD_SRC)ctb->calOwn, RAT_OK,
                   (T_ACI_AT_CMD)cmdBuf, (SHORT)(cId+1), BS_SPEED_NotPresent,CME_ERR_NotPresent );
            }
            break;
          }
#endif /* CO_UDP_IP || FF_GPF_TCPIP */

          psaCC_ClearCall (cId);
#if defined (SIM_TOOLKIT) AND defined (FF_SAT_E) 
          /* check impact for SAT commands */
          cmhSAT_OpChnCSDDown( cId, TPL_NONE );
#endif /* SIM TOOLKIT AND FF_SAT_E */
          break;

        case( DS_STOP_REQ ):

          ccShrdPrm.datStat = DS_IDL;
          cmhCC_CallDisconnected(cId);
#if defined (FF_WAP) || defined (FF_SAT_E)
          /* WAP STACK is down */
          ccShrdPrm.wapStat = CC_WAP_STACK_DOWN;
#endif /* FF_WAP || SAT E */
          break;

        case( DS_ABO_REQ ):   /* during modification */
          switch( ctb->rptInd )
          {
            case( MNCC_RI_SEQUENTIAL ):      /* no way back */

              ccShrdPrm.datStat = DS_IDL;
              ctb->nrmCs = MNCC_CAUSE_CALL_CLEAR;
              psaCC_ClearCall (cId);
              break;

            case( MNCC_RI_CIRCULAR ):

              ccShrdPrm.datStat = DS_IDL;
              psaCC_ModifyCall(cId);  /* back to former service */
              break;
          }
          break;

        case( DS_MDF_REQ ):
          ccShrdPrm.datStat = DS_IDL;
          psaCC_ModifyCall(cId);
          break;
      }
      break;

    /* in case of a fax call */
    case( FAX_CALL ):
      switch( ccShrdPrm.datStat )
      {
        case( DS_ACT ):
        case( DS_ACT_REQ ):
        case( DS_DSC_REQ ):
        case( DS_REST_REQ ):
        case( DS_TCH_MDF ):
          ccShrdPrm.datStat = DS_IDL;
          ctb->nrmCs = MNCC_CAUSE_CALL_CLEAR;
          psaCC_ClearCall (cId);
          break;

        case( DS_STOP_REQ ):
          ccShrdPrm.datStat = DS_IDL;
          cmhCC_CallDisconnected(cId);
          break;

        case( DS_ABO_REQ ):   /* abort during modification */
          switch (ctb->rptInd)
          {
            case( MNCC_RI_SEQUENTIAL ):      /* no way back */

              ccShrdPrm.datStat = DS_IDL;
              ctb->nrmCs = MNCC_CAUSE_CALL_CLEAR;
              psaCC_ClearCall (cId);
              break;

            case( MNCC_RI_CIRCULAR ):

              ccShrdPrm.datStat = DS_IDL;
              psaCC_ModifyCall(cId);  /* back to former service */
              break;
          }
          break;

        case( DS_MDF_REQ ):
          ccShrdPrm.datStat = DS_IDL;
          psaCC_ModifyCall(cId);
          break;
      }
      break;
  }
#endif    /* of #ifdef FAX_AND_DATA */
}
#endif /* DTI */

#ifdef FF_FAX
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_RA_Modified            |
+-------------------------------------------------------------------+

  PURPOSE : RA entity modified after CMM

*/
#ifdef DTI
GLOBAL void cmhCC_RA_Modified ( SHORT cId )
{
#ifdef FAX_AND_DATA

  T_CC_CALL_TYPE calltype = cmhCC_getcalltype(cId);

  TRACE_FUNCTION ("cmhCC_RA_Modified()");

  if( calltype EQ FAX_CALL )
  {
   /* only in case of a fax call */
    if( ccShrdPrm.datStat EQ DS_TCH_MDF )
    {
      ccShrdPrm.datStat = DS_ACT;

      if( cmhT30_Modify() NEQ AT_EXCT )
      {
        ccShrdPrm.datStat = DS_DSC_REQ;
        cmhRA_Deactivate ();
      }
    }
  }
#endif    /* of #ifdef FAX_AND_DATA */
}
#endif /* DTI */
#endif /* FF_FAX */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_L2R_or_TRA_Activated   |
+-------------------------------------------------------------------+

  PURPOSE : L2R entity activated, data path established

*/
#ifdef DTI
GLOBAL void cmhCC_L2R_or_TRA_Activated ( T_DTI_ENTITY_ID activated_module, SHORT cId )
{
#ifdef FAX_AND_DATA
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
  T_CC_CMD_PRM * pCCCmdPrm;    /* points to CC command parameters */
  UBYTE cmdBuf;                /* buffers current command */
  UBYTE srcBuf;                /* buffers current command source */
  T_CC_CALL_TYPE calltype = cmhCC_getcalltype(cId);

  TRACE_FUNCTION ("cmhCC_L2R_or_TRA_Activated()");

  if (!psaCC_ctbIsValid (cId))
  {
    /*
     * If for some no good reason we got a L2R_CONNECT_CNF, L2R_CONNECT_IND
     * or TRA_ACTIVATE_CNF without having a valid entry in the call table,
     * we do here the same as for the FAX case where those events are also 
     * unexpected. We send the respective deativation request.
     */
    TRACE_ERROR ("Call table entry disappeared");
    ccShrdPrm.datStat = DS_DSC_REQ;
    switch( activated_module )
    {
      case( DTI_ENTITY_TRA ):
        psaTRA_Deactivate();
        break;
      case( DTI_ENTITY_L2R ):
        cmhL2R_Deactivate();
        break;
      default: /* Unexpected to happen */
        break;
    }
    return;
  }

  cmdBuf = ctb->curCmd;
  srcBuf = ctb->curSrc;
  pCCCmdPrm = &cmhPrm[srcBuf].ccCmdPrm;

  switch( calltype )
  {
    case( TRANS_CALL ):
    case( NON_TRANS_CALL ):

#ifdef CO_UDP_IP
case( UDPIP_CALL ):
#endif /* CO_UDP_IP */
#ifdef FF_PPP
    case( PPP_CALL ):
#endif /* FF_PPP */
#if defined(FF_GPF_TCPIP)
    case (TCPIP_CALL):
#endif /* FF_GPF_TCPIP */

      switch( ccShrdPrm.datStat )
      {
        case( DS_ACT_REQ ):

          ccShrdPrm.datStat = DS_ACT;
          cmhCC_PrepareCmdEnd (cId, NULL, NULL);

          /* reset CMOD to single mode */
          ccShrdPrm.CMODmode = CMOD_MOD_Single;

          /* check if connect is expected */
          if( cmhCC_tstAndUnflagCall( cId, &(pCCCmdPrm -> mltyCncFlg)))
          {
            if( pCCCmdPrm -> mltyCncFlg EQ 0 )  /* if last call */
            {
              R_AT( RAT_CR, (T_ACI_CMD_SRC)srcBuf )
                (cmhCC_GetSrvType (&ctb->BC[ctb->curBC]));

              R_AT( RAT_ILRR, (T_ACI_CMD_SRC)srcBuf )
                (cmhCC_GetDataRate (&ctb->BC[ctb->curBC]),
                 cmhCC_GetFormat (&ctb->BC[ctb->curBC]),
                 cmhCC_GetParity (&ctb->BC[ctb->curBC]));
#if defined (CO_UDP_IP) AND defined (FF_GPF_TCPIP)
              if ( (calltype NEQ UDPIP_CALL) AND (calltype NEQ TCPIP_CALL) )
#endif
#if defined (CO_UDP_IP) AND !defined(FF_GPF_TCPIP)
              if (calltype NEQ UDPIP_CALL)
#endif
#if defined (FF_GPF_TCPIP) AND !defined(CO_UDP_IP)
              if (calltype NEQ TCPIP_CALL)
#endif
#ifdef FF_PPP
              if (calltype NEQ PPP_CALL)
#endif /* FF_PPP */
              {
#if defined (SIM_TOOLKIT) AND defined (FF_SAT_E)
                if (!cmhSAT_OpChnChckCSD( TPL_NONE))
#endif /* of SIM_TOOLKIT AND FF_SAT_E*/
                {
                  R_AT( RAT_CONNECT, (T_ACI_CMD_SRC)srcBuf )
                    (cmdBuf, cmhCC_GetDataRate(&ctb->BC[ctb->curBC]), cId+1, FALSE);
                }
              }

              /* log result */
              cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_CONNECT,
                      (T_ACI_AT_CMD)cmdBuf, (SHORT)(cId+1),
                      (T_ACI_BS_SPEED)cmhCC_GetDataRate(&ctb->BC[ctb->curBC]), CME_ERR_NotPresent );

              switch( activated_module )
              {
                case( DTI_ENTITY_TRA ):
                  if (IS_SRC_BT(srcBuf))
                  {
                    T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_BLUETOOTH, DTI_ENTITY_TRA};
                    UBYTE dti_id = dti_cntrl_new_dti(DTI_DTI_ID_NOTPRESENT);
                    dti_cntrl_est_dpath ( dti_id,
                                          entity_list,
                                          2,
                                          SPLIT,
                                          TRA_connect_dti_cb);
                  }
#if defined (SIM_TOOLKIT) AND defined (FF_SAT_E)
                  if (cmhSAT_OpChnChckCSD( TPL_NONE))
                  {
                    cmhSAT_OpChnSIMCnctReq( DTI_ENTITY_TRA );
                  }
#endif /* of SIM_TOOLKIT AND FF_SAT_E */
                  else
                  {
                    T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_TRA};

                    dti_cntrl_est_dpath_indirect ( srcBuf,
                                                   entity_list,
                                                   1,
                                                   SPLIT,
                                                   TRA_connect_dti_cb,
                                                   DTI_CPBLTY_SER,
                                                   DTI_CID_NOTPRESENT);
                  }
                  break;

                case( DTI_ENTITY_L2R ):
#ifdef CO_UDP_IP
                  if ( calltype EQ UDPIP_CALL )
                  {
                    UBYTE dti_id;
                    T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_IP, DTI_ENTITY_PPPC, DTI_ENTITY_L2R};
#if defined (SIM_TOOLKIT) AND defined (FF_SAT_E) 
                    if ( satShrdPrm.opchStat EQ OPCH_EST_REQ )
                      dti_id = simShrdPrm.sat_class_e_dti_id;
                    else
#endif /* SIM_TOOLKIT AND FF_SAT_E */
                      dti_id = wap_dti_id;

                    dti_cntrl_est_dpath ( dti_id,
                                          entity_list,
                                          3,
                                          APPEND,
                                          IP_UDP_connect_dti_cb);
                    break;
                  }
#endif /* CO_UDP_IP */

#if defined (FF_PPP) AND defined (DTI)
                  if ( calltype EQ PPP_CALL )
                  {
                    T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_PPPC, DTI_ENTITY_L2R};
                    dti_cntrl_est_dpath_indirect ( srcBuf,
                                                   entity_list,
                                                   2,
                                                   SPLIT,
                                                   L2R_connect_dti_cb,
                                                   DTI_CPBLTY_PKT,
                                                   DTI_CID_NOTPRESENT);
                    break;
                  }
#endif /* FF_PPP */

#ifdef FF_GPF_TCPIP
                  if ( calltype EQ TCPIP_CALL )
                  {
                    UBYTE dti_id;
                    T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_TCPIP, DTI_ENTITY_PPPC, DTI_ENTITY_L2R};
                    /* create a WAP DTI if not present */
                    if (wap_dti_id EQ DTI_DTI_ID_NOTPRESENT)
                    {
                       wap_dti_id = dti_cntrl_new_dti (DTI_DTI_ID_NOTPRESENT);
                       TRACE_EVENT_P1("tcpip_wap_dti_id = %d", wap_dti_id);
                    }
                    dti_id = wap_dti_id;
                    dti_cntrl_est_dpath ( dti_id,
                                          entity_list,
                                          3,
                                          SPLIT,
                                          TCPIP_connect_dti_cb);
                    break;
                  }
#endif /* FF_GPF_TCPIP */

#if defined (SIM_TOOLKIT) AND defined (FF_SAT_E)
                  if (cmhSAT_OpChnChckCSD( TPL_NONE))
                  {
                    cmhSAT_OpChnSIMCnctReq( DTI_ENTITY_L2R );
                  }
#endif /* of SIM_TOOLKIT AND FF_SAT_E */

#ifdef DTI
                  if (IS_SRC_BT(srcBuf))
                  {
                    T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_BLUETOOTH, DTI_ENTITY_L2R};
                    UBYTE dti_id = dti_cntrl_new_dti(DTI_DTI_ID_NOTPRESENT);
                    dti_cntrl_est_dpath ( dti_id,
                                          entity_list,
                                          2,
                                          SPLIT,
                                          L2R_connect_dti_cb);
                  }
                  else
                  {
                    T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_L2R};
                    dti_cntrl_est_dpath_indirect ( srcBuf,
                                                   entity_list,
                                                   1,
                                                   SPLIT,
                                                   L2R_connect_dti_cb,
                                                   DTI_CPBLTY_SER,
                                                   DTI_CID_NOTPRESENT);
                  }
#endif /* DTI */
                  break;
              } /* switch( activated_module ) */
            } /* if( pCCCmdPrm -> mltyCncFlg EQ 0 ) */
          } /* if( cmhCC_tstAndUnflagCall( cId, &(pCCCmdPrm -> mltyCncFlg))) */
          break;

        case( DS_REST_REQ ):
          ccShrdPrm.datStat = DS_ACT;
          break;
        }
        break;
    /* in case of a fax call */
    case( FAX_CALL  ):

      TRACE_EVENT( "UNEXP BEARER SERV FAX" );
      ccShrdPrm.datStat = DS_DSC_REQ;
      switch( activated_module )
      {
      case( DTI_ENTITY_TRA ):
        psaTRA_Deactivate();
        break;
      case( DTI_ENTITY_L2R ):
        cmhL2R_Deactivate();
        break;
      }
      break;
    }
#endif    /* of #ifdef FAX_AND_DATA */
}
#endif /* UART */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_L2R_or_TRA_Deactivated |
+-------------------------------------------------------------------+

  PURPOSE : L2R entity deactivated, data path disconnected

*/
#ifdef UART
GLOBAL void cmhCC_L2R_or_TRA_Deactivated ( SHORT cId )
{
#ifdef FAX_AND_DATA

  UBYTE srcBuf; 
#ifdef DTI
  T_DTI_CNTRL dti_cntrl_info;
#endif /* DTI */
  T_CC_CALL_TYPE calltype = cmhCC_getcalltype(cId);

  /* if there is a redirection get proper source */
#ifdef DTI
  if (dti_cntrl_get_info_from_dti_id(raShrdPrm.dti_id, &dti_cntrl_info))
  {
    srcBuf = dti_cntrl_info.src_id;
  }
  else /* if it is not a valid dti_id - take stored src_id */
  {
    TRACE_EVENT_P2("No valid dti_id:%d taking owner:%d",
                   raShrdPrm.dti_id,
                   raShrdPrm.owner);
#endif /* DTI */
    srcBuf = raShrdPrm.owner;
#ifdef DTI
  }
  /* reset ra dti_id storage */
  raShrdPrm.dti_id = NOT_PRESENT_8BIT;
#endif /* DTI */

  TRACE_FUNCTION ("cmhCC_L2R_or_TRA_Deactivated()");

  io_setDCD ((T_ACI_CMD_SRC)srcBuf, IO_DCD_OFF);

  switch( calltype )
  {
    case( TRANS_CALL ):
    case( NON_TRANS_CALL ):

#ifdef CO_UDP_IP
    case( UDPIP_CALL ):
#endif /* CO_UDP_IP */
#ifdef FF_PPP
    case( PPP_CALL ):
#endif /* FF_PPP */
#if defined(FF_GPF_TCPIP)
    case (TCPIP_CALL):
#endif /* FF_GPF_TCPIP */

      switch( ccShrdPrm.datStat )
      {
        case( DS_ACT ):
        case( DS_ACT_REQ ):
        case( DS_DSC_REQ ):
        case( DS_MDF_REQ ):
        case( DS_STOP_REQ ):

          cmhRA_Deactivate ();
          break;
      }
      break;

    case( FAX_CALL ):
      TRACE_EVENT( "UNEXP BEARER SERV FAX" );
      ccShrdPrm.datStat = DS_DSC_REQ;
      cmhRA_Deactivate();
      break;
  }
#endif    /* of #ifdef FAX_AND_DATA */
}
#endif /* UART */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_L2R_Failed             |
+-------------------------------------------------------------------+

  PURPOSE : L2R entity failure

*/

GLOBAL void cmhCC_L2R_Failed ( void )
{
#ifdef FAX_AND_DATA
  SHORT          cId = raShrdPrm.cId;  /* holds call identifier */
  T_CC_CALL_TYPE calltype;

  TRACE_FUNCTION ("cmhCC_L2R_Failed()");

  calltype = cmhCC_getcalltype(cId);

  switch( calltype )
  {
    case( NON_TRANS_CALL ):
      switch( ccShrdPrm.datStat )
      {
        case( DS_ACT_REQ ):
          if( psaCC_ctbIsValid (cId) AND
              psaCC_ctb(cId)->calStat EQ CS_MDF_REQ )
          {
            ccShrdPrm.datStat = DS_ABO_REQ;
          }
          /*lint -fallthrough*/
        case( DS_ACT ):
        case( DS_REST_REQ ):
            cmhL2R_Deactivate ();
          break;
      }
      break;


    case( FAX_CALL  ):

      TRACE_EVENT( "UNEXP BEARER SERV FAX" );
      ccShrdPrm.datStat = DS_DSC_REQ;
      cmhL2R_Deactivate();
      break;
    }

#endif    /* of #ifdef FAX_AND_DATA */
}

#ifdef FF_FAX
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_T30_Activated          |
+-------------------------------------------------------------------+

  PURPOSE : T30 entity activated, data path established

*/

GLOBAL void cmhCC_T30_Activated ( void )
{
#if defined (FAX_AND_DATA) AND defined (DTI)

  SHORT cId = t30ShrdPrm.cId;  /* holds call identifier */
  T_CC_CALL_TYPE calltype = cmhCC_getcalltype(cId);

  TRACE_FUNCTION ("cmhCC_T30_Activated()");

  (void)&cmhPrm[psaCC_ctb(cId)->calOwn].ccCmdPrm;
  (void)psaCC_ctb(cId)->curCmd;

  switch( calltype )
  {
    case( FAX_CALL ):
      switch( ccShrdPrm.datStat )
      {
        case( DS_ACT_REQ ):

          ccShrdPrm.datStat = DS_ACT;

          /* reset CMOD to single mode */
          ccShrdPrm.CMODmode = CMOD_MOD_Single;

          if( psaCC_ctb(cId)->calType EQ CT_MTC )
          {
            cmhT30_SendCaps( (T_ACI_CMD_SRC)psaCC_ctb(cId)->curSrc, FRT_DIS );
          }
          break;
      }
      break;

   /* in case of a asynchronous call */
    case( TRANS_CALL ):
    case( NON_TRANS_CALL ):
      TRACE_EVENT( "UNEXP BEARER SERV ASYNC" );
      ccShrdPrm.datStat = DS_DSC_REQ;
      cmhT30_Deactivate();
      break;
    }

#endif    /* of #ifdef FAX_AND_DATA */
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_T30_Deactivated        |
+-------------------------------------------------------------------+

  PURPOSE : T30 entity deactivated, data path disconnected

*/

GLOBAL void cmhCC_T30_Deactivated ( void )
{
#ifdef FAX_AND_DATA

  SHORT cId = t30ShrdPrm.cId;  /* holds call identifier */
  T_CC_CALL_TYPE calltype = cmhCC_getcalltype(cId);

  TRACE_FUNCTION ("cmhCC_T30_Deactivated()");

  switch( calltype )
  {
    case( FAX_CALL ):
      switch( ccShrdPrm.datStat )
      {
        case( DS_ACT ):
        case( DS_ACT_REQ ):
        case( DS_DSC_REQ ):
        case( DS_MDF_REQ ):
        case( DS_STOP_REQ ):
        case( DS_TCH_MDF ):

          cmhRA_Deactivate ();
          break;
      }
      break;

    /* in case of a asynchronous data call */
    case( TRANS_CALL ):
    case( NON_TRANS_CALL ):

      TRACE_EVENT( "UNEXP BEARER SERV ASYNC" );
      ccShrdPrm.datStat = DS_DSC_REQ;
      cmhRA_Deactivate();
      break;
  }

#endif    /* of #ifdef FAX_AND_DATA */
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_T30_Failed             |
+-------------------------------------------------------------------+

  PURPOSE : T30 entity failure

*/

GLOBAL void cmhCC_T30_Failed ( void )
{
#if defined FAX_AND_DATA AND defined (DTI)

  SHORT cId = t30ShrdPrm.cId;  /* holds call identifier */
  T_CC_CALL_TYPE calltype = cmhCC_getcalltype(cId);

  TRACE_FUNCTION ("cmhCC_T30_Failed()");

  switch( calltype )
  {
    case( FAX_CALL ):

      switch( ccShrdPrm.datStat )
      {
        case( DS_ACT_REQ ):

          if( psaCC_ctb(cId)->calStat EQ CS_MDF_REQ )
          {
            ccShrdPrm.datStat = DS_ABO_REQ;
          }
          /*lint -fallthrough*/
        case( DS_ACT ):
        case( DS_REST_REQ ):
        case( DS_TCH_MDF ):

          cmhT30_Deactivate ();
          break;
      }
      break;

    /* in case of a asynchronous data call */
    case( TRANS_CALL ):
    case( NON_TRANS_CALL ):

      TRACE_EVENT( "UNEXP BEARER SERV ASYNC" );
      ccShrdPrm.datStat = DS_DSC_REQ;
      cmhT30_Deactivate();
      break;
    }

#endif    /* of #ifdef FAX_AND_DATA */
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_T30_RmtCaps            |
+-------------------------------------------------------------------+

  PURPOSE : T30 remote capability indication

*/

GLOBAL void cmhCC_T30_RmtCaps ( void )
{
#ifdef FAX_AND_DATA

  T_CC_CMD_PRM * pCCCmdPrm;    /* points to CC command parameters */
  UBYTE cmdBuf;                /* buffers current command */
  UBYTE srcBuf;                /* buffers current command source */
  SHORT cId = t30ShrdPrm.cId;  /* holds call identifier */

  TRACE_FUNCTION ("cmhCC_T30_RmtCaps()");

/*
 *-------------------------------------------------------------------
 * check if connect is expected
 *-------------------------------------------------------------------
 */
  pCCCmdPrm = &cmhPrm[psaCC_ctb(cId)->curSrc].ccCmdPrm;
  cmhCC_PrepareCmdEnd (cId, &cmdBuf, &srcBuf);

  if( cmhCC_tstAndUnflagCall( cId, &(pCCCmdPrm -> mltyCncFlg)))
  {
    if( pCCCmdPrm -> mltyCncFlg EQ 0 )  /* if last call */
    {
      R_AT( RAT_OK, (T_ACI_CMD_SRC)srcBuf ) ( cmdBuf );

      /* log result */
      cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_OK,
          (T_ACI_AT_CMD)cmdBuf, (SHORT)(cId+1), BS_SPEED_NotPresent,CME_ERR_NotPresent);
    }
  }

#endif    /* of #ifdef FAX_AND_DATA */
}

#endif /* FF_FAX */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_MPTYBuild              |
+-------------------------------------------------------------------+

  PURPOSE : Multiparty successful build.

*/

GLOBAL void cmhCC_MPTYBuild( SHORT cId,
                             T_BUILD_MPTY_RES *bldMPTY )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
  T_CC_CMD_PRM * pCCCmdPrm;   /* points to CC command parameters */
  UBYTE ctbIdx;               /* holds call table index */
  UBYTE cmdBuf;               /* buffers command */
  UBYTE srcBuf;               /* buffers command source */

  TRACE_FUNCTION ("cmhCC_MPTYBuild()");

  TIMERSTOP( ACI_TMPTY );

  if( ctb->curCmd NEQ AT_CMD_CHLD )
    return;  /* wrong command context */

  /* set multiparty status for involved calls */
  for( ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
  {
    if (ccShrdPrm.ctb[ctbIdx] NEQ NULL AND
        (psaCC_ctb(ctbIdx)->mptyStat  EQ CS_ACT_REQ OR
         psaCC_ctb(ctbIdx)->mptyStat  EQ CS_ACT ))
    {
      psaCC_ctb(ctbIdx)->mptyStat = CS_ACT;
      psaCC_ctb(ctbIdx)->calStat  = CS_ACT;
    }
  }

  pCCCmdPrm = &cmhPrm[ctb->curSrc].ccCmdPrm;
  cmhCC_PrepareCmdEnd (cId, &cmdBuf, &srcBuf);

  psaCC_MPTY (cId, MNCC_MPTY_BUILD_SUCCESS);
      /* Implements Measure 184, 185 and 187 */
      cmhCC_tstAndUnflag_MPTYCall( cId, srcBuf, cmdBuf, pCCCmdPrm );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_MPTYSplit              |
+-------------------------------------------------------------------+

  PURPOSE : Multiparty successful split.

*/

GLOBAL void cmhCC_MPTYSplit( SHORT cId,
                             T_SPLIT_MPTY_RES *splMPTY )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
  T_CC_CMD_PRM * pCCCmdPrm;   /* points to CC command parameters */
  UBYTE ctbIdx;               /* holds call table index */
  UBYTE cmdBuf;               /* buffers command */
  UBYTE srcBuf;               /* buffers command source */
  UBYTE fndCal;               /* holds number of found calls */

  TRACE_FUNCTION ("cmhCC_MPTYSplit()");

  TIMERSTOP( ACI_TMPTY );

  if( ctb->curCmd NEQ AT_CMD_CHLD )
    return;  /* wrong command context */

  /* set status for involved calls */
  for( fndCal = 0, ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
  {
    if( ccShrdPrm.ctb[ctbIdx] NEQ NULL AND
        psaCC_ctb(ctbIdx)->mptyStat   EQ  CS_ACT AND
        ctbIdx                           NEQ cId )
    {
      psaCC_ctb(ctbIdx)->calStat  = CS_HLD;
      fndCal++;
    }
  }

  /* if only one other call is left, reset multiparty status */
  /* if only one other call is left, do not reset multiparty status - VO patch
  if( fndCal EQ 1 )
  {
    psaCC_ctb(fndCId)->mptyStat = CS_IDL;
  }
  */

  pCCCmdPrm = &cmhPrm[ctb->curSrc].ccCmdPrm;
  cmhCC_PrepareCmdEnd (cId, &cmdBuf, &srcBuf);
  ctb->calStat  = CS_ACT;
  ctb->mptyStat = CS_IDL;

  psaCC_MPTY (cId, MNCC_MPTY_SPLIT_SUCCESS);

  psaCC_setSpeechMode (); /* In case we split from a held mpty */
      /* Implements Measure 184, 185 and 187 */
      cmhCC_tstAndUnflag_MPTYCall( cId, srcBuf, cmdBuf, pCCCmdPrm );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_MPTYHeld               |
+-------------------------------------------------------------------+

  PURPOSE : Multiparty successful held.

*/

GLOBAL void cmhCC_MPTYHeld( SHORT cId,
                            T_HOLD_MPTY_RES *hldMPTY )
{
  T_CC_CMD_PRM * pCCCmdPrm;   /* points to CC command parameters */
  UBYTE ctbIdx;               /* holds call table index */
  UBYTE cmdBuf;               /* buffers command */
  UBYTE srcBuf;               /* buffers command source */

  TRACE_FUNCTION ("cmhCC_MPTYHeld()");

  TIMERSTOP( ACI_TMPTY );

  /* check for command context */
  switch( psaCC_ctb(cId)->curCmd )
  {
    case( AT_CMD_CHLD ):
    case( AT_CMD_D ):
      break;

    default:
      return;  /* wrong command context */
  }

  /* set call status for multiparty call member */
  for( ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
  {
    if( ccShrdPrm.ctb[ctbIdx] NEQ NULL AND
        psaCC_ctb(ctbIdx)->calStat    EQ CS_HLD_REQ AND
        psaCC_ctb(ctbIdx)->mptyStat   EQ CS_ACT )
    {
      psaCC_ctb(ctbIdx)->calStat = CS_HLD;
    }
  }

  psaCC_MPTY (cId, MNCC_MPTY_HOLD_SUCCESS);

  psaCC_setSpeechMode ();

  pCCCmdPrm = &cmhPrm[psaCC_ctb(cId)->curSrc].ccCmdPrm;
  cmhCC_PrepareCmdEnd (cId, &cmdBuf, &srcBuf);

  if( cmhCC_tstAndUnflagCall( cId, &(pCCCmdPrm -> mltyCncFlg)))
  {
    if((pCCCmdPrm -> CHLDmode EQ CHLD_MOD_HldActExc OR
        pCCCmdPrm -> CHLDmode EQ CHLD_MOD_HldActAndAcpt OR
        pCCCmdPrm -> CHLDmode EQ CHLD_MOD_HldActDial) AND
        pCCCmdPrm -> mltyCncFlg )
    {
      process_CHLDaddInfo( srcBuf, MTPTY_HELD );
    }

    if( pCCCmdPrm -> mltyCncFlg EQ 0 AND
        pCCCmdPrm -> mltyDscFlg EQ 0     )  /* if last call */
    {
      R_AT( RAT_OK, (T_ACI_CMD_SRC)srcBuf ) ( cmdBuf );

      /* log result */
      cmh_logRslt ((T_ACI_CMD_SRC)srcBuf, RAT_OK,
              (T_ACI_AT_CMD)cmdBuf, (SHORT)(cId+1),BS_SPEED_NotPresent,CME_ERR_NotPresent);
    }
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_MPTYRetrieved          |
+-------------------------------------------------------------------+

  PURPOSE : Multiparty successful retrieved.

*/

GLOBAL void cmhCC_MPTYRetrieved( SHORT cId,
                                 T_RETRIEVE_MPTY_RES *rtvMPTY )
{
  T_CC_CMD_PRM * pCCCmdPrm;   /* points to CC command parameters */
  UBYTE ctbIdx;               /* holds call table index */
  UBYTE cmdBuf;               /* buffers command */
  UBYTE srcBuf;               /* buffers command source */

  TRACE_FUNCTION ("cmhCC_MPTYRetrieved()");

  TIMERSTOP( ACI_TMPTY );

  if( psaCC_ctb(cId)->curCmd NEQ AT_CMD_CHLD )
    return;  /* wrong command context */

  /* set call status for multiparty call member */
  for( ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
  {
    if( ccShrdPrm.ctb[ctbIdx] NEQ NULL AND
        psaCC_ctb(ctbIdx)->calStat    EQ CS_ACT_REQ AND
        psaCC_ctb(ctbIdx)->mptyStat   EQ CS_ACT )
    {
      psaCC_ctb(ctbIdx)->calStat = CS_ACT;
    }
  }

  pCCCmdPrm = &cmhPrm[psaCC_ctb(cId)->curSrc].ccCmdPrm;
  cmhCC_PrepareCmdEnd (cId, &cmdBuf, &srcBuf);

  psaCC_MPTY (cId, MNCC_MPTY_RETRIEVE_SUCCESS);

  psaCC_setSpeechMode ();
      /* Implements Measure 184, 185 and 187 */
      cmhCC_tstAndUnflag_MPTYCall( cId, srcBuf, cmdBuf, pCCCmdPrm );

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_NotifySS               |
+-------------------------------------------------------------------+

  PURPOSE : SS notification received: in preamble, some
  local functions, otherwise, the function would be huge...

*/

/*
+------------------------------------------------------------------------------
|   Function    :  send_CSSX_notification
+------------------------------------------------------------------------------
|   Description :  This function sends CSSU and CSSI.
|
|   Parameters  :  ctbIdx                    - call table index
|                  cssx_code                 - CSS(X) code
|                  index                     - Index
|                  *number                   - buffer for number
|                  *toa                      - buffers type of address
|                  *subaddr                  - buffer for subaddress
|                  *tos                      - holds type of subaddress
|
|   Return      :
+------------------------------------------------------------------------------
*/

GLOBAL void send_CSSX_notification(SHORT ctbIdx,
                                   T_ACI_CSSX_CODE cssx_code,
                                   SHORT index,
                                   CHAR            *number,
                                   T_ACI_TOA       *toa,
                                   CHAR            *subaddr,
                                   T_ACI_TOS       *tos)

{
  UBYTE idx;     /* holds index counter */

  /*  Check if CC State is CS ACT REQ */
//TISH, OMAPS00130688
//start
#if 0
  if (ctbIdx > NO_ENTRY AND
      psaCC_ctb(ctbIdx)->calStat EQ CS_ACT_REQ AND 
      psaCC_ctb(ctbIdx)->calType EQ CT_MOC)
#else
  if (ctbIdx > NO_ENTRY AND
      ((psaCC_ctb(ctbIdx)->calStat EQ CS_ACT_REQ) OR 
       (psaCC_ctb(ctbIdx)->calStat EQ CS_ACT AND cssx_code EQ CSSX_CODE_CFUActive) OR
       (psaCC_ctb(ctbIdx)->calStat EQ CS_DSC_REQ)) AND
      psaCC_ctb(ctbIdx)->calType EQ CT_MOC)
#endif
//end
  {
    T_ACI_CSSI_CODE cssi_code;
    /* Implements Measure 90,91 */
    if( (cssx_code <= CSSX_CODE_DeflectedCall ) AND 
        (cssx_code >= CSSX_CODE_ForwardedCall))
    {
      cssi_code = ( T_ACI_CSSI_CODE ) cssx2I_Code_Convert[cssx_code];
    }
    else
    {
      cssi_code = CSSI_CODE_NotPresent;
    }
    if (cssi_code NEQ CSSI_CODE_NotPresent 
        /* Added as we want comparison to 255 */
        AND (cssi_code NEQ CSSI_CODE_Biggest )
       )
    {
//TISH patch for OMAPS00130689
//start
#if 0
      S8 source = psaCC_ctb(ctbIdx)->curSrc;
      R_AT( RAT_CSSI, (T_ACI_CMD_SRC)source)( cssi_code, index );
#else
      for( idx = 0; idx < CMD_SRC_MAX; idx++ )
      {
        R_AT( RAT_CSSI, idx )(cssi_code, index);
      }
#endif
//end
    }
  }
  else /* (calldirection EQ CT_MTC) */
  {
    /* Sent CSSU TO ALL */
    T_ACI_CSSU_CODE cssu_code;
    /* Implements Measure 90,91 */
    /******************
     * Since there is a one-to-one mapping between CSSU and CSSX,
     * Hence we have not implemented any table for converting 
     * CSSX codes to CSSU codes.
     ******************/
      cssu_code = CSSU_CODE_NotPresent;

    if ( cssx_code EQ CSSX_CODE_DeflectedCall )
    {
      cssu_code = CSSU_CODE_DeflectedCall;
    }
    else if ( cssx_code EQ CSSX_CODE_IncCallForwarded)
    {
      cssu_code = CSSU_CODE_IncCallForwarded;
    }
    else if ( cssx_code <= CSSX_CODE_ECTConnect AND 
              cssx_code >= CSSX_CODE_ForwardedCall )
    {
      cssu_code = ( T_ACI_CSSU_CODE ) cssx_code;
    }
    if (cssu_code != CSSU_CODE_NotPresent )
    {
      for( idx = 0; idx < CMD_SRC_MAX; idx++ )
      {
        R_AT( RAT_CSSU, (T_ACI_CMD_SRC)idx )(cssu_code, index, number, toa, subaddr, tos);
      }
    }
  }
}



LOCAL void cmhCC_NotifySS_v_ssCode( SHORT cId, T_NOTIFY_SS_INV *ntfySS )
{
  CHAR numBuf[MAX_B_SUBSCR_NUM_LEN];         /* buffer for number */
  T_ACI_TOA toa;                             /* holds type of address */
  CHAR subBuf[MAX_SUBADDR_LEN];              /* buffer for subaddress */
  T_ACI_TOS tos;                             /* holds type of subaddress */

  TRACE_FUNCTION ("cmhCC_NotifySS_v_ssCode()");

  switch( ntfySS -> notifySSArg.ssCode )
  {
     case( SS_CD_ALL_FWSS ):
     case( SS_CD_CFU ):
      if( ntfySS -> notifySSArg.v_ssStatus AND
          ntfySS -> notifySSArg.ssStatus & SSS_A )
      {
        send_CSSX_notification(cId,
                              CSSX_CODE_CFUActive,
                              ACI_NumParmNotPresent,
                              NULL, NULL, NULL, NULL);
      }
      break;

    case( SS_CD_CFB       ):
    case( SS_CD_CFNRY     ):
    case( SS_CD_CFNRC     ):
    case( SS_CD_ALL_CFWSS ):

      if( ntfySS -> notifySSArg.v_ssStatus AND
          ntfySS -> notifySSArg.ssStatus & SSS_A )
      {

         send_CSSX_notification(cId,
                              CSSX_CODE_SomeCCFActive,
                              ACI_NumParmNotPresent,
                              NULL, NULL, NULL, NULL);
      }
      break;

    case( SS_CD_ALL_CBSS ):

      if( ntfySS -> notifySSArg.v_ssStatus AND
          ntfySS -> notifySSArg.ssStatus & SSS_A )
      {
        /* RAT_CSSI ... outgoing calls are barred  */
        /* RAT_CSSI ... incoming calls are barred  */
        send_CSSX_notification(cId,
                              CSSX_CODE_OutCallsBarred,
                              ACI_NumParmNotPresent,
                              NULL, NULL, NULL, NULL);

        send_CSSX_notification(cId,
                              CSSX_CODE_IncCallsBarred,
                              ACI_NumParmNotPresent,
                              NULL, NULL, NULL, NULL);

      }
      break;

    case( SS_CD_BOC ):
    case( SS_CD_BAOC ):
    case( SS_CD_BOIC ):
    case( SS_CD_BOICXH ):

      if( ntfySS -> notifySSArg.v_ssStatus AND
          ntfySS -> notifySSArg.ssStatus & SSS_A )
      {
        /* RAT_CSSI ... outgoing calls are barred  */
        send_CSSX_notification(cId,
                              CSSX_CODE_OutCallsBarred,
                              ACI_NumParmNotPresent,
                              NULL, NULL, NULL, NULL);

      }
      break;

    case( SS_CD_BIC ):
    case( SS_CD_BAIC ):
    case( SS_CD_BICRM ):

      if( ntfySS -> notifySSArg.v_ssStatus AND
          ntfySS -> notifySSArg.ssStatus & SSS_A )
      {
        /* RAT_CSSI ... incoming calls are barred  */
     send_CSSX_notification(cId,
                              CSSX_CODE_IncCallsBarred,
                              ACI_NumParmNotPresent,
                              NULL, NULL, NULL, NULL);

}
      break;

    case( SS_CD_CD ):
      if (ntfySS->notifySSArg.v_ssNotification AND
          ntfySS->notifySSArg.ssNotification.fwdSubscriber EQ
            FWD_C_INC_CALL_FWD)
      {
        /*
         * If alerting is allowed now, send the CSSU indication.
         * Otherwise just remember that we got this notification.
         */
        if ((ccShrdPrm.TCHasg EQ TRUE) OR
            (psaCC_ctb(cId)->sigInf NEQ MNCC_SIG_NOT_PRES))

        {
          /*
           * The conditions for user alerting are fulfilled.
           * Send the +CSSU: unsolicited result code now.
           */
          send_CSSX_notification(cId,
                              CSSX_CODE_DeflectedCall,
                              ACI_NumParmNotPresent,
                              psaCC_ctbRdrAdr2Num (cId, numBuf, sizeof (numBuf)),
                              cmhCC_ctbGetRdrNumTyp (cId, &toa),
                              psaCC_ctbRdrAdr2Sub (cId, subBuf),
                              cmhCC_ctbGetRdrSubTyp (cId, &tos));

          psaCC_FreeRdrPty (cId);
          psaCC_ctb(cId)->CDStat = NO_VLD_CD;
        }
        else
        {
          /*
           * Indication of the MT call towards the MMI is deferred until
           * the alerting conditions are fulfilled (TCH assignment).
           */
          psaCC_ctb(cId)->CDStat = CD_Notified;
        }
        return;
      }

      if (ntfySS->notifySSArg.v_ssNotification AND
          ntfySS->notifySSArg.ssNotification.clgSubscriber EQ
            CLG_A_OUTG_CALL_FWD_C)
      {
        /* RAT_CSSI ... outgoing call has been deflected */
        send_CSSX_notification(cId,
                              CSSX_CODE_DeflectedCall,
                              ACI_NumParmNotPresent,
                              NULL, NULL, NULL, NULL);

      }
      break;

    default:
      TRACE_EVENT( "UNHANDLED SS-CODE IN NOTIFY" );
      break;
  }
}

LOCAL void cmhCC_NotifySS_v_ssNotification( SHORT cId, T_NOTIFY_SS_INV *ntfySS )
{

  TRACE_FUNCTION ("cmhCC_NotifySS_v_ssNotification()");

  if( ntfySS -> notifySSArg.ssNotification.clgSubscriber EQ
      CLG_A_OUTG_CALL_FWD_C )
  {
    /* RAT_CSSI ... outgoing call has been forwarded  */
    send_CSSX_notification(cId,
                              CSSX_CODE_ForwardedCall,
                              ACI_NumParmNotPresent,
                              NULL, NULL, NULL, NULL);
  }

  if( ntfySS -> notifySSArg.ssNotification.fwgSubscriber EQ
      FWG_B_INC_CALL_FWD_C )
  {
    /* RAT_CSSU ... this call is forwarded to C-Subsciber  */
    send_CSSX_notification(cId,
                           CSSX_CODE_IncCallForwarded,
                            ACI_NumParmNotPresent ,
                            NULL, NULL, NULL, NULL);

  }

  if( ntfySS -> notifySSArg.ssNotification.fwdSubscriber EQ
      FWD_C_INC_CALL_FWD )
  {
    /* RAT_CSSU ... this is a forwarded call  */
    send_CSSX_notification(cId,
                            CSSX_CODE_ForwardedCall,
                            ACI_NumParmNotPresent ,
                            NULL, NULL, NULL, NULL);

  }
}


LOCAL void cmhCC_NotifySS_v_ectIndicator( SHORT cId, T_NOTIFY_SS_INV *ntfySS )
{
  CHAR numBuf[MAX_B_SUBSCR_NUM_LEN];         /* buffer for number */
  T_ACI_TOA toa;                             /* holds type of address */
  CHAR subBuf[MAX_SUBADDR_LEN];              /* buffer for subaddress */
  T_ACI_TOS tos;                             /* holds type of subaddress */
  BOOL numFlg   = FALSE;                     /* flags a present number */
  BOOL subFlg   = FALSE;                     /* flags a present subaddress */

  TRACE_FUNCTION ("cmhCC_NotifySS_v_ectIndicator()");

  /* note that according to GSM 04.91, indications without RDN are
     possible but not without ectCallState */
  if( ntfySS -> notifySSArg.ectIndicator.v_ectCallState )
  {
    /* (G)ACI-FIX-1415 */
    if(ntfySS -> notifySSArg.ectIndicator.v_rdn )
    {
      if( ntfySS -> notifySSArg.ectIndicator.
        rdn.v_presentationAllowedAddress )
      {
        if( ntfySS -> notifySSArg.ectIndicator.
          rdn.presentationAllowedAddress.v_partyNumber AND
          ntfySS -> notifySSArg.ectIndicator.
          rdn.presentationAllowedAddress.partyNumber.c_bcdDigit )
        {
          utl_BCD2DialStr
            (ntfySS -> notifySSArg.ectIndicator.
            rdn.presentationAllowedAddress.partyNumber.bcdDigit,
            numBuf,
            ntfySS -> notifySSArg.ectIndicator.
            rdn.presentationAllowedAddress.partyNumber.c_bcdDigit);

          toa.npi = (T_ACI_TOA_NPI)ntfySS -> notifySSArg.ectIndicator.
            rdn.presentationAllowedAddress.partyNumber.npi;
          toa.ton = (T_ACI_TOA_TON)ntfySS -> notifySSArg.ectIndicator.
            rdn.presentationAllowedAddress.partyNumber.noa;

          numFlg = TRUE;
        }

        if( ntfySS -> notifySSArg.ectIndicator.
          rdn.presentationAllowedAddress.v_partySubaddress AND
          ntfySS -> notifySSArg.ectIndicator.
          rdn.presentationAllowedAddress.partySubaddress.c_subadr_str )
        {
          utl_BCD2DialStr
            (ntfySS -> notifySSArg.ectIndicator.
            rdn.presentationAllowedAddress.partySubaddress.subadr_str,
            subBuf,
            ntfySS -> notifySSArg.ectIndicator.
            rdn.presentationAllowedAddress.partySubaddress.c_subadr_str);

          tos.tos = (T_ACI_TOS_TOS)ntfySS -> notifySSArg.ectIndicator.
            rdn.presentationAllowedAddress.partySubaddress.tos;
          tos.oe  = (T_ACI_TOS_OE)ntfySS -> notifySSArg.ectIndicator.
            rdn.presentationAllowedAddress.partySubaddress.oei;

          subFlg = TRUE;
        }
      }
      /* check if the override case occured, do present RDN, (G)ACI-FIX-1434 */
      /* if presentationAllowedAddress is present then it is obsolete to look
         at presentationRestrictedAddress */
      else if( ntfySS -> notifySSArg.ectIndicator.
        rdn.v_presentationRestrictedAddress )
      {
        if( ntfySS -> notifySSArg.ectIndicator.
          rdn.presentationRestrictedAddress.v_partyNumber AND
          ntfySS -> notifySSArg.ectIndicator.
          rdn.presentationRestrictedAddress.partyNumber.c_bcdDigit )
        {
          utl_BCD2DialStr
            (ntfySS -> notifySSArg.ectIndicator.
            rdn.presentationRestrictedAddress.partyNumber.bcdDigit,
            numBuf,
            ntfySS -> notifySSArg.ectIndicator.
            rdn.presentationRestrictedAddress.partyNumber.c_bcdDigit);

          toa.npi = (T_ACI_TOA_NPI)ntfySS -> notifySSArg.ectIndicator.
            rdn.presentationRestrictedAddress.partyNumber.npi;
          toa.ton = (T_ACI_TOA_TON)ntfySS -> notifySSArg.ectIndicator.
            rdn.presentationRestrictedAddress.partyNumber.noa;

          numFlg = TRUE;
        }

        if( ntfySS -> notifySSArg.ectIndicator.
          rdn.presentationRestrictedAddress.v_partySubaddress AND
          ntfySS -> notifySSArg.ectIndicator.
          rdn.presentationRestrictedAddress.partySubaddress.c_subadr_str )
        {
          utl_BCD2DialStr
            (ntfySS -> notifySSArg.ectIndicator.
            rdn.presentationRestrictedAddress.partySubaddress.subadr_str,
            subBuf,
            ntfySS -> notifySSArg.ectIndicator.
            rdn.presentationRestrictedAddress.partySubaddress.c_subadr_str);

          tos.tos = (T_ACI_TOS_TOS)ntfySS -> notifySSArg.ectIndicator.
            rdn.presentationRestrictedAddress.partySubaddress.tos;
          tos.oe  = (T_ACI_TOS_OE)ntfySS -> notifySSArg.ectIndicator.
            rdn.presentationRestrictedAddress.partySubaddress.oei;

          subFlg = TRUE;
        }
      }
    }

    switch( ntfySS -> notifySSArg.ectIndicator.ectCallState )
    {
      case( ECT_CS_ALERTING ):
        /* RAT_CSSU ... ECT alerting */
        send_CSSX_notification(cId,
                                CSSX_CODE_ECTAlert,
                                ACI_NumParmNotPresent,
                                (numFlg)?numBuf:NULL,
                                (numFlg)?&toa:NULL,
                                (subFlg)?subBuf:NULL,
                                (subFlg)?&tos:NULL);
        break;

      case( ECT_CS_ACTIVE ):
        /* RAT_CSSU ... ECT active */
        send_CSSX_notification(cId,
                                CSSX_CODE_ECTConnect,
                                ACI_NumParmNotPresent,
                                (numFlg)?numBuf:NULL,
                                (numFlg)?&toa:NULL,
                                (subFlg)?subBuf:NULL,
                                (subFlg)?&tos:NULL);

        break;
    }
  }
}

LOCAL void cmhCC_NotifySS_v_ccbsf( T_NOTIFY_SS_INV *ntfySS )
{
  T_ACI_CCBS_SET ccbsSet;                    /* holds ccbs setting */

  TRACE_FUNCTION ("cmhCC_NotifySS_v_ccbsf()");

  if( ntfySS -> notifySSArg.ccbsf.v_ccbsIndex )
  {
    ccbsSet.idx = ntfySS -> notifySSArg.ccbsf.ccbsIndex;
  }
  else
    ccbsSet.idx = ACI_NumParmNotPresent;

  if( ntfySS -> notifySSArg.ccbsf.v_b_subscriberNumber AND
      ntfySS -> notifySSArg.ccbsf.b_subscriberNumber.c_bcdDigit)
  {
    utl_BCD2DialStr
      (ntfySS -> notifySSArg.ccbsf.b_subscriberNumber.bcdDigit,
       ccbsSet.number,
       ntfySS -> notifySSArg.ccbsf.b_subscriberNumber.c_bcdDigit);

    ccbsSet.type.npi = (T_ACI_TOA_NPI)ntfySS -> notifySSArg.ccbsf.b_subscriberNumber.npi;
    ccbsSet.type.ton = (T_ACI_TOA_TON)ntfySS -> notifySSArg.ccbsf.b_subscriberNumber.noa;
  }
  else
  {
    ccbsSet.number[0] = 0x0;
    ccbsSet.type.npi = NPI_NotPresent;
    ccbsSet.type.ton = TON_NotPresent;
  }

  if( ntfySS -> notifySSArg.ccbsf.v_b_subscriberSubaddress AND
      ntfySS -> notifySSArg.ccbsf.b_subscriberSubaddress.c_subadr_str)
  {
    utl_BCD2DialStr
      (ntfySS -> notifySSArg.ccbsf.b_subscriberSubaddress.subadr_str,
       ccbsSet.subaddr,
       ntfySS -> notifySSArg.ccbsf.b_subscriberSubaddress.c_subadr_str);

    ccbsSet.satype.tos = (T_ACI_TOS_TOS)ntfySS -> notifySSArg.ccbsf.b_subscriberSubaddress.tos;
    ccbsSet.satype.oe  = (T_ACI_TOS_OE)ntfySS -> notifySSArg.ccbsf.b_subscriberSubaddress.oei;
  }
  else
  {
    ccbsSet.subaddr[0] = 0x0;
    ccbsSet.satype.tos = TOS_NotPresent;
    ccbsSet.satype.oe  = OE_NotPresent;
  }

  if( ntfySS -> notifySSArg.ccbsf.v_basicServiceGroup )
    ccbsSet.class_type = cmhSS_GetClass( (T_basicService*)&ntfySS -> notifySSArg.ccbsf.
                            basicServiceGroup );
  else
    ccbsSet.class_type = CLASS_NotPresent;

  if( ntfySS -> notifySSArg.v_alertingPattern )
  {
    ccbsSet.alrtPtn = (T_ACI_ALRT_PTRN)ntfySS -> notifySSArg.alertingPattern;
  }
  else
  {
    ccbsSet.alrtPtn = ALPT_NotPresent;
  }

  cmhrat_ccbs( CMD_SRC_MAX, CCBS_IND_Recall,
               CCBS_STAT_NotPresent, &ccbsSet );
}


GLOBAL void cmhCC_NotifySS( SHORT cId, T_NOTIFY_SS_INV *ntfySS )
{
  UBYTE         idx;        /* holds index counter */
  T_callingName *NameId;    /* holds Name Identifier for CNAP */

  TRACE_FUNCTION ("cmhCC_NotifySS()");

  /* check SS code */
  if( ntfySS -> notifySSArg.v_ssCode )
  {
    cmhCC_NotifySS_v_ssCode( cId, ntfySS );
  }

  /* check SS notification */
  if( ntfySS -> notifySSArg.v_ssNotification )
  {
    cmhCC_NotifySS_v_ssNotification( cId, ntfySS );
  }

  /* check call is waiting indicator */
  if( ntfySS -> notifySSArg.v_callIsWaitingIndicator )
  {
    /* RAT_CSSI ... call is waiting */
    send_CSSX_notification(cId,
                              CSSX_CODE_CallWaiting,
                              ACI_NumParmNotPresent,
                              NULL, NULL, NULL, NULL);

  }

  /* check call on hold indicator */
  if( ntfySS -> notifySSArg.v_callOnHoldIndicator )
  {
    switch( ntfySS -> notifySSArg.callOnHoldIndicator )
    {
    case( CHLD_CALL_RETRIEVED ):
      send_CSSX_notification(cId,
                              CSSX_CODE_Retrieved,
                              ACI_NumParmNotPresent ,
                              NULL, NULL, NULL, NULL);

      break;

    case( CHLD_CALL_ON_HOLD ):
      /* RAT_CSSU ... call has been put on hold */
      send_CSSX_notification(cId,
                              CSSX_CODE_OnHold,
                              ACI_NumParmNotPresent ,
                              NULL, NULL, NULL, NULL);

      break;
    }
  }

  /* check multparty indicator */
  if( ntfySS -> notifySSArg.v_mptyIndicator )
  {
     /* mptyStat equal to CS_ACT_REQ set it to CS_ACT
         required to place multiparty call on hold */
      if(psaCC_ctb(cId)->mptyStat EQ CS_ACT_REQ)
      {
        psaCC_ctb(cId)->mptyStat = CS_ACT;
      }

    /* RAT_CSSU ... multiparty call entered */
    send_CSSX_notification(cId,
                              CSSX_CODE_Multiparty,
                              ACI_NumParmNotPresent ,
                              NULL, NULL, NULL, NULL);

}

  /* check CUG index */
  if( ntfySS -> notifySSArg.v_cugIndex )
  {
      send_CSSX_notification(cId,
                              CSSX_CODE_CUGCall,
                              ntfySS -> notifySSArg.cugIndex,
                              NULL, NULL, NULL, NULL);
  }

  /* check CLIR suppression */
  if( ntfySS -> notifySSArg.v_clirSuppressionRejected )
  {
    /* RAT_CSSI ... CLIR  suppression rejected */
    send_CSSX_notification(cId,
                              CSSX_CODE_CLIRSupRej,
                              ACI_NumParmNotPresent,
                              NULL, NULL, NULL, NULL);

  }

  /* check ECT indicator */
  if( ntfySS -> notifySSArg.v_ectIndicator )
  {
    cmhCC_NotifySS_v_ectIndicator( cId, ntfySS );
  }

  /* check CNAP indicator */
  if( ntfySS -> notifySSArg.v_nameIndicator )
  {
    if ( ntfySS -> notifySSArg.nameIndicator.v_callingName )
    {
      NameId = &(ntfySS -> notifySSArg.nameIndicator.callingName);
      for( idx = 0; idx < CMD_SRC_MAX; idx++ )
      {
        R_AT( RAT_CNAP, (T_ACI_CMD_SRC)idx )
          ( NameId, CNAP_SERVICE_STATUS_NOT_PRESENT);
      }
    }
  }

  /* check CCBS feature */
  if( ntfySS -> notifySSArg.v_ccbsf )
  {
    cmhCC_NotifySS_v_ccbsf( ntfySS );
  }
}


/*
+------------------------------------------------------------------------------
|   Function    :  cmhCC_CheckSS
+------------------------------------------------------------------------------
|   Description :  check SS indication received.
|
|   Parameters  :  cId -
|
|   Return      :
+------------------------------------------------------------------------------
*/
GLOBAL void cmhCC_CheckSS( SHORT cId )
{
  UBYTE idx;                      /* holds index counter */

  TRACE_FUNCTION ("cmhCC_CheckSS()");

/*
 *-------------------------------------------------------------------
 * indicate check SS
 *-------------------------------------------------------------------
 */
  /* RAT_CSSU ... forward check SS indication */
  for( idx = 0; idx < CMD_SRC_MAX; idx++ )
  {
    send_CSSX_notification( cId,
                              CSSX_CODE_FwrdCheckSS,
                              ACI_NumParmNotPresent,
                              NULL, NULL, NULL, NULL);

  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_CCBSRegistered         |
+-------------------------------------------------------------------+

  PURPOSE :  call completion to busy subscriber successfully registered.

*/

GLOBAL void cmhCC_CCBSRegistered( SHORT cId,
                                  T_ACC_REGISTER_CC_ENTRY_RES *CCBSreg )
{
  UBYTE          cmdBuf;                    /* buffers command */
  UBYTE          srcBuf;                    /* buffers command source */
  T_ACI_CCBS_SET ccbsSet;                   /* holds ccbs setting */

  TRACE_FUNCTION ("cmhCC_CCBSRegistered()");

  /* check for command context */
  if( psaCC_ctb(cId)->curCmd NEQ AT_CMD_CHLD )
  {
    TRACE_EVENT("cmhCC_CCBSRegistered: wrong command context !");
    return;
  }

  if( CCBSreg->accRegisterCCEntryRes.v_ccbsf )
  {
    if( CCBSreg->accRegisterCCEntryRes.ccbsf.v_ccbsIndex )

      ccbsSet.idx = CCBSreg->accRegisterCCEntryRes.ccbsf.ccbsIndex;
    else
      ccbsSet.idx = ACI_NumParmNotPresent;


    if( CCBSreg->accRegisterCCEntryRes.ccbsf.
        v_b_subscriberNumber                  AND
        CCBSreg->accRegisterCCEntryRes.ccbsf.
        b_subscriberNumber.c_bcdDigit)
    {
      utl_BCD2DialStr
        (CCBSreg->accRegisterCCEntryRes.ccbsf.b_subscriberNumber.
         bcdDigit,
         ccbsSet.number,
         CCBSreg->accRegisterCCEntryRes.ccbsf.b_subscriberNumber.
         c_bcdDigit);

      ccbsSet.type.npi = (T_ACI_TOA_NPI)CCBSreg->accRegisterCCEntryRes.ccbsf.
                         b_subscriberNumber.npi;
      ccbsSet.type.ton = (T_ACI_TOA_TON)CCBSreg->accRegisterCCEntryRes.ccbsf.
                         b_subscriberNumber.noa;
    }
    else
    {
      ccbsSet.number[0] = 0x0;
      ccbsSet.type.npi = NPI_NotPresent;
      ccbsSet.type.ton = TON_NotPresent;
    }

    if( CCBSreg->accRegisterCCEntryRes.ccbsf.
        v_b_subscriberSubaddress              AND
        CCBSreg->accRegisterCCEntryRes.ccbsf.
        b_subscriberSubaddress.c_subadr_str)
    {
      utl_BCD2DialStr
        (CCBSreg->accRegisterCCEntryRes.ccbsf.b_subscriberSubaddress.
         subadr_str,
         ccbsSet.subaddr,
         CCBSreg->accRegisterCCEntryRes.ccbsf.b_subscriberSubaddress.
         c_subadr_str);

      ccbsSet.satype.tos = (T_ACI_TOS_TOS)CCBSreg->accRegisterCCEntryRes.ccbsf.
                   b_subscriberSubaddress.tos;
      ccbsSet.satype.oe  = (T_ACI_TOS_OE)CCBSreg->accRegisterCCEntryRes.ccbsf.
                   b_subscriberSubaddress.oei;
    }
    else
    {
      ccbsSet.subaddr[0] = 0x0;
      ccbsSet.satype.tos = TOS_NotPresent;
      ccbsSet.satype.oe  = OE_NotPresent;
    }

    if( CCBSreg->accRegisterCCEntryRes.ccbsf.v_basicServiceGroup )
      ccbsSet.class_type = cmhSS_GetClass( (T_basicService*)&CCBSreg->accRegisterCCEntryRes.
                                       ccbsf.basicServiceGroup );
    else
      ccbsSet.class_type = CLASS_NotPresent;

    ccbsSet.alrtPtn = ALPT_NotPresent;
  }

  cmhCC_PrepareCmdEnd (cId, &cmdBuf, &srcBuf);
  cmhCC_tstAndUnflagCall( cId, &(cmhPrm[srcBuf].ccCmdPrm.mltyDscFlg));
  psaCC_FreeCtbNtry (cId);

  cmhrat_ccbs( srcBuf, CCBS_IND_Registered,
               CCBS_STAT_NotPresent, &ccbsSet );

  R_AT( RAT_OK, (T_ACI_CMD_SRC)srcBuf ) ( cmdBuf );

  /* log result */
  cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_OK,
         (T_ACI_AT_CMD)cmdBuf, (SHORT)(cId+1), BS_SPEED_NotPresent,CME_ERR_NotPresent);
}

/* sends RAT_CCBS: sometimes it is an intermediate result, and sometimes it is an
indication.
When srcId = CMD_SRC_MAX, it is an indication and shall go to all sources...
Otherwise, only to the source specified... */
GLOBAL void cmhrat_ccbs( UBYTE           srcId,
                         T_ACI_CCBS_IND  ccbs_ind,
                         T_ACI_CCBS_STAT status,
                         T_ACI_CCBS_SET  *setting )
{
#ifdef FF_ATI
  UBYTE idx;
#endif

  if( srcId EQ CMD_SRC_MAX )
  {
#if defined MFW || defined SMI OR defined FF_MMI_RIV
    rAT_PercentCCBS(ccbs_ind, status, setting);
#endif

#ifdef FF_ATI
    for( idx = CMD_SRC_ATI_1; idx < CMD_SRC_MAX; idx++ )
    {
      srcId_cb = idx;
      rCI_PercentCCBS(ccbs_ind, status, setting, FALSE);      
    }
#endif /* FF_ATI */

    return;
  }
#if defined MFW || defined SMI OR defined FF_MMI_RIV
  if( srcId EQ CMD_SRC_LCL )
  {
    rAT_PercentCCBS( ccbs_ind, status, setting );
    return;
  }
#endif

  /* else it is an ATI source */
  srcId_cb = srcId;

#ifdef FF_ATI
  rCI_PercentCCBS(ccbs_ind, status, setting, TRUE);
#endif /* FF_ATI */
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_CDRegistered           |
+-------------------------------------------------------------------+

  PURPOSE :  Call Deflection (CD) successfully registered.

*/

GLOBAL void cmhCC_CDRegistered( SHORT cId )
{
  TRACE_FUNCTION ("cmhCC_CDRegistered()");

  switch( psaCC_ctb(cId)->curCmd )
  {
    case AT_CMD_CTFR:
      psaCC_ctb(cId)->CDStat = CD_Succeeded;
      break;

    default: /* This is unexpected or network failure, ignore the event */
      break;
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_SSTransFail            |
+-------------------------------------------------------------------+

  PURPOSE : SS transaction failed.

*/

GLOBAL void cmhCC_SSTransFail   ( SHORT cId )
{
  T_CC_CMD_PRM * pCCCmdPrm;   /* points to CC command parameters */
  SHORT ctbIdx;               /* holds call table index */
  SHORT addId;                /* holds additional call id */
  UBYTE cmdBuf;               /* buffers command */
  UBYTE srcBuf;               /* buffers command source */

  TRACE_FUNCTION ("cmhCC_SSTransFail()");

/*
 *-------------------------------------------------------------------
 * check for command context
 *-------------------------------------------------------------------
 */
  switch( psaCC_ctb(cId)->curCmd )
  {
   /*
    *----------------------------------------------------------------
    * process result for D and A command
    *----------------------------------------------------------------
    */
    case( AT_CMD_D ):
    case( AT_CMD_A ):

      /* restore call status for hold request*/
      if( psaCC_ctb(cId)->calStat EQ CS_HLD_REQ )
        for( ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
        {
          if( ccShrdPrm.ctb[ctbIdx] NEQ NULL AND
              psaCC_ctb(ctbIdx)->calStat    EQ CS_HLD_REQ AND
              psaCC_ctb(ctbIdx)->mptyStat   EQ CS_ACT )
          {
            psaCC_ctb(ctbIdx)->calStat = CS_ACT;
          }
        }

      pCCCmdPrm = &cmhPrm[psaCC_ctb(cId)->curSrc].ccCmdPrm;

      cmhCC_PrepareCmdEnd (cId, &cmdBuf, &srcBuf);

      if( cmhCC_tstAndUnflagCall( cId, &(pCCCmdPrm -> mltyCncFlg)))
      {
        if( pCCCmdPrm -> CHLDmode EQ CHLD_MOD_HldActDial AND
            pCCCmdPrm -> mltyCncFlg )
        {
          for( addId = 0; !((pCCCmdPrm -> mltyCncFlg >> addId) & 0x01); addId++ )
            ;

          if( CHLDaddInfo EQ CHLD_ADD_INFO_DIAL_CAL )
          {
              CHLDaddInfo = NO_CHLD_ADD_INFO;
              psaCC_FreeCtbNtry (addId);  /* remove pending call */
          }
        }

        pCCCmdPrm -> mltyCncFlg = 0;        /* unflag calls */
//TISH, patch for OMAPS00129157
//start
		if (psaCC_ctb(cId)->failType == SSF_ERR_PRB && psaCC_ctb(cId)->errCd == 0x0a)
		    R_AT( RAT_CME, srcBuf ) ( cmdBuf, CME_ERR_OpNotAllow);
		else
//end
        R_AT( RAT_CME, (T_ACI_CMD_SRC)srcBuf ) ( cmdBuf, CME_ERR_NotPresent );

        cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_CME,
                        (T_ACI_AT_CMD)cmdBuf, -1, BS_SPEED_NotPresent, CME_ERR_NotPresent );
      }
      break;

   /*
    *----------------------------------------------------------------
    * process result for +CTFR command
    *----------------------------------------------------------------
    */
    case( AT_CMD_CTFR ):
      psaCC_ctb(cId)->CDStat = CD_Failed;
      break;

   /*
    *----------------------------------------------------------------
    * process result for +CHLD command
    *----------------------------------------------------------------
    */
    case( AT_CMD_CHLD ):

      /* determine CHLD mode */
      switch( cmhPrm[psaCC_ctb(cId)->curSrc].ccCmdPrm.CHLDmode )
      {
       /*
        *------------------------------------------------------------
        * no multiparty relation
        *------------------------------------------------------------
        */
        case( CHLD_MOD_NotPresent ):
        case( CHLD_MOD_RelHldOrUdub ):
        case( CHLD_MOD_RelActSpec ):

          TRACE_EVENT( "UNEXP CHLD MODE FOR SS ERROR COMP" );
          return;

       /*
        *------------------------------------------------------------
        * multiparty hold or retrieve
        *------------------------------------------------------------
        */
        case( CHLD_MOD_RelActAndAcpt ):
        case( CHLD_MOD_HldActAndAcpt ):

          /* restore call status for retrieve request */
          if( psaCC_ctb(cId)->calStat EQ CS_ACT_REQ )
          {
            for( ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
            {
              if( ccShrdPrm.ctb[ctbIdx] NEQ NULL AND
                  psaCC_ctb(ctbIdx)->calStat    EQ CS_ACT_REQ AND
                  psaCC_ctb(ctbIdx)->mptyStat   EQ CS_ACT )
              {
                psaCC_ctb(ctbIdx)->calStat = CS_HLD;
              }
            }
            psaCC_MPTY (cId, MNCC_MPTY_RETRIEVE_FAIL);
          }

          /* restore call status for hold request*/
          if( psaCC_ctb(cId)->calStat EQ CS_HLD_REQ )
          {
            for( ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
            {
              if( ccShrdPrm.ctb[ctbIdx] NEQ NULL AND
                  psaCC_ctb(ctbIdx)->calStat    EQ CS_HLD_REQ AND
                  psaCC_ctb(ctbIdx)->mptyStat   EQ CS_ACT )
              {
                psaCC_ctb(ctbIdx)->calStat = CS_ACT;
              }
            }
            psaCC_MPTY (cId, MNCC_MPTY_HOLD_FAIL);
          }

          break;

       /*
        *------------------------------------------------------------
        * multiparty split
        *------------------------------------------------------------
        */
        case( CHLD_MOD_HldActExc ):

          psaCC_ctb(cId)->mptyStat = CS_ACT;
          psaCC_MPTY (cId, MNCC_MPTY_SPLIT_FAIL);
          break;

       /*
        *------------------------------------------------------------
        * multiparty build
        *------------------------------------------------------------
        */
        case( CHLD_MOD_AddHld ):

          /* restore call status for build request */
          for( ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
          {
            if( ccShrdPrm.ctb[ctbIdx] NEQ NULL AND
                psaCC_ctb(ctbIdx)->mptyStat   EQ CS_ACT_REQ AND
                psaCC_ctb(ctbIdx)->calStat    EQ CS_HLD )
            {
              psaCC_ctb(ctbIdx)->mptyStat = CS_IDL;
            }
          }

          if( psaCC_ctb(cId)->mptyStat EQ CS_ACT_REQ )
            psaCC_ctb(cId)->mptyStat = CS_IDL;
          psaCC_MPTY (cId, MNCC_MPTY_BUILD_FAIL);
          break;

       /*
        *------------------------------------------------------------
        * explicit call transfer
        *------------------------------------------------------------
        */
        case( CHLD_MOD_Ect ):
          pCCCmdPrm = &cmhPrm[psaCC_ctb(cId)->curSrc].ccCmdPrm;
          TRACE_EVENT("ECT failed...");

          TIMERSTOP( ACI_TECT );

          pCCCmdPrm->CHLDmode = CHLD_MOD_NotPresent;

          /* restore call status for other call that was to be transfered */
          for( ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
          {
            if( ctbIdx EQ cId )
            {
              continue;
            }
            else if (ccShrdPrm.ctb[ctbIdx] NEQ NULL AND
                     cmhCC_tstAndUnflagCall( ctbIdx, &(pCCCmdPrm -> mltyDscFlg)) AND
                     psaCC_ctb(ctbIdx)->curCmd EQ AT_CMD_CHLD )
            {
              /* then it is the second call id */
              cmhCC_PrepareCmdEnd (ctbIdx, NULL, NULL);
            }
          }

          /* restore first call id parameters */
          cmhCC_PrepareCmdEnd (cId, NULL, &srcBuf);

          if( cmhCC_tstAndUnflagCall( cId, &(pCCCmdPrm -> mltyDscFlg)) )
          {
            R_AT( RAT_CME, (T_ACI_CMD_SRC)srcBuf )
                        ( AT_CMD_CHLD, CME_ERR_Unknown );

            cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_CME,
                          AT_CMD_CHLD, -1, BS_SPEED_NotPresent, CME_ERR_Unknown );
          }
          return;

       /*
        *------------------------------------------------------------
        * call completion to busy subscriber
        *------------------------------------------------------------
        */
        case( CHLD_MOD_Ccbs ):
          pCCCmdPrm = &cmhPrm[psaCC_ctb(cId)->curSrc].ccCmdPrm;
          TRACE_EVENT("CCBS failed...");

          /* restore first call id parameters */
          cmhCC_PrepareCmdEnd (cId, NULL, &srcBuf);

          if( psaCC_ctb(cId)->opCode EQ OPC_ACC_REGISTER_CC_ENTRY )
            psaCC_FreeCtbNtry (cId);  /* remove call table entry */

          if( cmhCC_tstAndUnflagCall( cId, &(pCCCmdPrm -> mltyDscFlg)) )
          {
            pCCCmdPrm -> mltyDscFlg = 0;        /* unflag calls */

            R_AT( RAT_CME, (T_ACI_CMD_SRC)srcBuf )
                        ( AT_CMD_CHLD, CME_ERR_Unknown );

            cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_CME,
                          AT_CMD_CHLD, -1, BS_SPEED_NotPresent, CME_ERR_Unknown );
          }
          return;
      }

      pCCCmdPrm = &cmhPrm[psaCC_ctb(cId)->curSrc].ccCmdPrm;
      cmhCC_PrepareCmdEnd (cId, NULL, &srcBuf);

      if( cmhCC_tstAndUnflagCall( cId, &(pCCCmdPrm -> mltyCncFlg)))
      {
        pCCCmdPrm -> mltyCncFlg = 0;        /* unflag calls */

        R_AT( RAT_CME, (T_ACI_CMD_SRC)srcBuf ) ( AT_CMD_CHLD, CME_ERR_Unknown );

        cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_CME,
                      AT_CMD_CHLD, -1, BS_SPEED_NotPresent, CME_ERR_Unknown );
      }

      break;
  }

  return;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCR                 |
|                                 ROUTINE : cmhCC_MPTYTimeout       |
+-------------------------------------------------------------------+

  PURPOSE : handle multiparty timeout

*/

GLOBAL void cmhCC_MPTYTimeout ( void )
{
  TRACE_FUNCTION( "cmhCC_MPTYTimeout()" );

  if (!psaCC_ctbIsValid (ccShrdPrm.cIdMPTY))
  {
    TRACE_ERROR ("ccShrdPrm.cIdMPTY invalid");
    return;
  }

/*
 *-------------------------------------------------------------------
 * SS failure
 *-------------------------------------------------------------------
 */
#ifdef SIM_TOOLKIT
  if (psaCC_ctb(ccShrdPrm.cIdMPTY)->SATinv )
  {
    psaCC_ctb(ccShrdPrm.cIdMPTY)->SATinv = FALSE;
    /* return network error cause GSM 11.14 / 12.12.3 */
    cmhSAT_NtwErr( ADD_NO_CAUSE );
  }
#endif
  cmhCC_SSTransFail(ccShrdPrm.cIdMPTY);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCR                 |
|                                 ROUTINE : cmhCC_ECTTimeout        |
+-------------------------------------------------------------------+

  PURPOSE : handle explicit call transfer timeout

*/

LOCAL SHORT search_ect_call(void)
{
  SHORT idx;
  UBYTE chld_src;

  for(idx = 0; idx < MAX_CALL_NR; idx++)
  {
    if (ccShrdPrm.ctb[idx] NEQ NULL AND
        psaCC_ctb(idx)->curCmd EQ AT_CMD_CHLD)
    {
      chld_src = psaCC_ctb(idx)->curSrc;

      if( cmhPrm[chld_src].ccCmdPrm.CHLDmode EQ CHLD_MOD_Ect )
      {
        return(idx);
      }
    }
  }
  return(NO_ENTRY);
}

GLOBAL void cmhCC_ECTTimeout ( void )
{
  SHORT ectId;

  TRACE_FUNCTION( "cmhCC_ECTTimeout()" );

  ectId = search_ect_call( );

  if (ectId NEQ NO_ENTRY)
  cmhCC_SSTransFail(ectId);
}

#ifdef FF_TTY
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_TTY_Control            |
+-------------------------------------------------------------------+

  PURPOSE : Enabling/disabling TTY for the active call

*/

LOCAL BOOL cmhCC_TTY_Proceed (SHORT cId)
{
  T_TTY_CMD ttyAction = TTY_OFF;
  T_MNCC_bcpara *bc;

  if (ccShrdPrm.ctb[cId] NEQ NULL)
  {
    /* Call table entry exists */
    bc = &psaCC_ctb(cId)->BC[psaCC_ctb(cId)->curBC];
  
    if ((bc->bearer_serv EQ MNCC_BEARER_SERV_SPEECH_CTM OR
       bc->bearer_serv EQ MNCC_BEARER_SERV_AUX_SPEECH_CTM) AND
        psaCC_ctb(cId)->calStat NEQ CS_DSC_REQ)
  {
    if ((ccShrdPrm.chMod EQ MNCC_CHM_SPEECH) OR
        (ccShrdPrm.chMod EQ MNCC_CHM_SPEECH_V2) OR
        (ccShrdPrm.chMod EQ MNCC_CHM_SPEECH_V3))
    {
      audio_set_tty (ttyAction = (T_TTY_CMD)ccShrdPrm.ttyCmd);
    }
    else
    {
      TRACE_EVENT_P1 ("TTY no speech mode: %d", (int)ccShrdPrm.chMod);
    }
  }
  else
  {
    TRACE_EVENT_P1 ("TTY wrong BCAP: %d", (int)bc->bearer_serv);
  }

  if (ttyAction NEQ (UBYTE)TTY_OFF)
  {
    cmhCC_notifyTTY (CTTY_NEG_Grant, cmhCC_getTTYtrx_state (ttyAction));
    return TRUE;
  }
  else
  {
    cmhCC_notifyTTY (((ccShrdPrm.ctmReq EQ MNCC_CTM_ENABLED)?
                      CTTY_NEG_Reject: CTTY_NEG_None),
                     CTTY_TRX_Unknown);
  }
  }
  else
  {
    TRACE_EVENT ("TTY no call table entry");
  }

  return FALSE;
}

GLOBAL void cmhCC_TTY_Control (SHORT cId, UBYTE action)
{
  TRACE_EVENT_P3 ("TTY event %d, old state %d, callId %d",
                  action, ccShrdPrm.ctmState, cId);

  if (!ccShrdPrm.TCHasg)
  {
    TRACE_EVENT ("TTY: no TCH");
  }
  switch (action)
  {
  case TTY_TCH:
    if (ccShrdPrm.ctmState EQ TTY_STATE_IDLE)
    {
      ccShrdPrm.ctmState = TTY_STATE_SYNC;
    }
    else if (ccShrdPrm.ctmState EQ TTY_STATE_BCAP)
    {
      if (cmhCC_TTY_Proceed (cId))
        ccShrdPrm.ctmState = TTY_STATE_ACTIVE;
      else
        ccShrdPrm.ctmState = TTY_STATE_WAIT;
    }
    break;

  case TTY_START:
    if (ccShrdPrm.ctmState EQ TTY_STATE_IDLE)
    {
      ccShrdPrm.ctmState = TTY_STATE_BCAP;
    }
    else if (ccShrdPrm.ctmState EQ TTY_STATE_SYNC)
    {
      if (cmhCC_TTY_Proceed (cId))
        ccShrdPrm.ctmState = TTY_STATE_ACTIVE;
      else
        ccShrdPrm.ctmState = TTY_STATE_WAIT;
    }
    break;

  case TTY_PAUSE:
    if (ccShrdPrm.ctmState EQ TTY_STATE_WAIT)
    {
      ccShrdPrm.ctmState = TTY_STATE_SYNC;
    }
    else if (ccShrdPrm.ctmState EQ TTY_STATE_ACTIVE)
    {
      audio_set_tty (TTY_OFF);
      ccShrdPrm.ctmState = TTY_STATE_SYNC;
    }
    break;

  case TTY_STOP:
    if (ccShrdPrm.ctmState EQ TTY_STATE_ACTIVE)
    {
      audio_set_tty (TTY_OFF);
      cmhCC_notifyTTY (CTTY_NEG_None, CTTY_TRX_Off);
    }
    ccShrdPrm.ctmState = (ccShrdPrm.ctmReq EQ MNCC_CTM_ENABLED)?
                         TTY_STATE_IDLE: TTY_STATE_NONE;
    break;

  default:
    break;
  }
  TRACE_EVENT_P1 ("TTY new state %d", ccShrdPrm.ctmState);
}
#endif /* FF_TTY */

#if defined (FF_WAP) || defined (FF_PPP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E) 
/* Only for WAP or RNET */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_PPP_Established        |
+-------------------------------------------------------------------+

  PURPOSE : PPP entity activated, data path established

*/

GLOBAL SHORT cmhCC_PPP_Established (ULONG ip_address, USHORT max_receive_unit,
                                    ULONG dns1, ULONG dns2)
{

  char c_ip_address[16],c_dns1[16],c_dns2[16];

#ifdef FF_PPP
  SHORT cId = raShrdPrm.cId;
#endif /* FF_PPP  */

  TRACE_FUNCTION ("cmhCC_PPP_Established()");

  ipAddress = ip_address;
  sprintf (c_ip_address, "%03u.%03u.%03u.%03u", (ip_address & 0xff000000) >> 24,
                                                (ip_address & 0x00ff0000) >> 16,
                                                (ip_address & 0x0000ff00) >> 8 ,
                                                (ip_address & 0x000000ff)        );

  sprintf(c_dns1, "%03u.%03u.%03u.%03u",       (dns1 & 0xff000000) >> 24,
                                               (dns1 & 0x00ff0000) >> 16,
                                               (dns1 & 0x0000ff00) >> 8 ,
                                               (dns1 & 0x000000ff)        );
  
  sprintf(c_dns2, "%03u.%03u.%03u.%03u",       (dns2 & 0xff000000) >> 24,
                                               (dns2 & 0x00ff0000) >> 16,
                                               (dns2 & 0x0000ff00) >> 8 ,
                                               (dns2 & 0x000000ff)        );

  TRACE_EVENT_P3("addr : %s ,dns1 : %s ,dns2: %s",c_ip_address,c_dns1,c_dns2);

  if(is_gpf_tcpip_call()) {
    pppShrdPrm.ipaddr = ip_address ;
    pppShrdPrm.dns1   = dns1 ;
    pppShrdPrm.dns2   = dns2 ;
  }
#if defined FF_PPP AND defined FAX_AND_DATA
  if (psaCC_ctbIsValid (cId) AND /* Ensure not to dereferentiate NULL */
      pppShrdPrm.is_PPP_CALL EQ TRUE)
  {
    pppShrdPrm.ipaddr = ip_address ;
    pppShrdPrm.dns1   = dns1 ;
    pppShrdPrm.dns2   = dns2 ;
    R_AT( RAT_CONNECT, raShrdPrm.owner )
      (psaCC_ctb(cId)->curCmd, 
       cmhCC_GetDataRate(&psaCC_ctb(cId)->BC[psaCC_ctb(cId)->curBC]),
       cId+1, 
       FALSE);
    return 0;
  }
#endif /*FF_PPP*/
#if defined (FF_SAT_E)
  if(cmhSAT_OpChnChckCSD(UDP))
  {
    psaTCPIP_Configure((UBYTE*)c_ip_address, NULL, NULL, 
                       (UBYTE*)c_dns1, (UBYTE*)c_dns2,max_receive_unit,
                       cmhSAT_OpChnUDPConfCsd) ;
  }
  else
#endif /* FF_SAT_E */
#if defined (FF_WAP) || defined (FF_GPF_TCPIP)
  {
    psaTCPIP_Configure((UBYTE*)c_ip_address, NULL, NULL, 
                       (UBYTE*)c_dns1, (UBYTE*)c_dns1,max_receive_unit,
                       psaTCPIP_conf_csd_callback) ;
  }
#endif /* FF_WAP OR FF_GPF_TCPIP */
  return 0;
}

#ifdef CO_UDP_IP
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_IPA_Configurated       |
+-------------------------------------------------------------------+

  PURPOSE : IPA entity activated, data path established
*/

GLOBAL SHORT cmhCC_IPA_Configurated (void)
{
  TRACE_FUNCTION ("cmhCC_IPA_Configurated()");

  psaUDPA_Config(UDPA_CONFIG_UP);

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_UDPA_Configurated      |
+-------------------------------------------------------------------+

  PURPOSE : IPA entity activated, data path established

*/

GLOBAL SHORT cmhCC_UDPA_Configurated (void)
{
  TRACE_FUNCTION ("cmhCC_UDPA_Configurated()");
#ifdef FF_WAP
  if (Wap_Call)
  {
    if ( tcpipShrdPrm.connection_type EQ TCPIP_CONNECTION_TYPE_CSD_WAP )
    {
      rAT_WAP_PPP_connected(wapId,ipAddress);
    }
    else
    {
      TRACE_EVENT_P1 ("IP: %s", tcpipShrdPrm.ipaddr);
      rAT_WAP_PPP_connected(wapId, psaTCPIP_bytes2ipv4addr(tcpipShrdPrm.ipaddr));
    }
  }
#endif      
  psaUDPIP_config_dispatch() ;
  return 0;
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_UDPA_Deconfigurated    |
+-------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL SHORT cmhCC_UDPA_Deconfigurated (void)
{
  TRACE_FUNCTION ("cmhCC_UDPA_Deconfigurated ()");

  cmhUDPA_Deactivate(tcpipShrdPrm.src_id);

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_IPA_Deconfigurated     |
+-------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL SHORT cmhCC_IPA_Deconfigurated (void)
{
  TRACE_FUNCTION ("cmhCC_IPA_Deconfigurated()");

  psaUDPA_Config(UDPA_CONFIG_DOWN);

  return 0;
}

#endif /* of WAP || SAT E */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_PPP_Terminated         |
+-------------------------------------------------------------------+

  PURPOSE : PPP entity terminated, close connection / update CC parameters

*/

GLOBAL SHORT cmhCC_PPP_Terminated ( void )
{
  TRACE_FUNCTION ("cmhCC_PPP_Terminated()");

#ifdef DTI
#if defined (FF_WAP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E) 
  if (Wap_Call EQ TRUE)
  {
    if(is_gpf_tcpip_call()) {
      GPF_TCPIP_STATEMENT(dti_cntrl_entity_disconnected(peer_link_id, 
                                                        DTI_ENTITY_TCPIP ));
    }
    else {
    dti_cntrl_entity_disconnected( peer_link_id, DTI_ENTITY_IP );
    }
    dti_cntrl_entity_disconnected( prot_link_id, DTI_ENTITY_L2R );
  }
#endif /* WAP || defined (FF_GPF_TCPIP) || SAT E */

  dti_cntrl_entity_disconnected( peer_link_id, DTI_ENTITY_PPPC );
  dti_cntrl_entity_disconnected( prot_link_id, DTI_ENTITY_PPPC );


#if defined (FF_SAT_E)
  if( cmhSAT_OpChnChckCSD(UDP))
  {
    psaTCPIP_Deactivate(cmhSAT_OpChnUDPDeactCsd) ;
  }
  else
#endif /* FF_SAT_E */
#if defined (FF_WAP) OR defined (FF_GPF_TCPIP)
  {
    psaTCPIP_Deactivate(psaTCPIP_deact_csd_callback) ;
  }
#endif /* defined (WAP) OR defined (FF_GPF_TCPIP) */
#endif /* DTI */
  return 0;
}


#endif /* of WAP or FF_PPP || defined (FF_GPF_TCPIP) or SAT E */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCR                  |
| STATE   : code                  ROUTINE : cmhCC_checkALS_Support_cb|
+--------------------------------------------------------------------+

  PURPOSE : Call-backs for ALS support check.
*/

GLOBAL void cmhCC_checkALS_Support_cb ( SHORT aId )
{
  TRACE_FUNCTION ("cmhCC_checkALS_Support_cb()");

  simShrdPrm.atb[aId].ntryUsdFlg = FALSE;

  /* Step #1: Reading of CPHS Info */
  if (simShrdPrm.atb[aId].errCode NEQ SIM_NO_ERROR OR
      simShrdPrm.atb[aId].exchData EQ NULL OR
      simShrdPrm.atb[aId].dataLen < 2)
  {
    ;  /* CPHS info not supported or invalid */
  }
  else if ((simShrdPrm.atb[aId].exchData[1] & 0x03) EQ ALLOCATED_AND_ACTIVATED)
  {
    /* continue with step 2 */
    cmhCC_checkALS_Support_2();
    return;
  }

  cmhCC_checkALS_Support_exec( 0 );
}

GLOBAL void cmhCC_checkALS_Support_cb_2 ( SHORT aId )
{
  int i;

  TRACE_FUNCTION ("cmhCC_checkALS_Support_cb_2()");

  simShrdPrm.atb[aId].ntryUsdFlg = FALSE;

  if (simShrdPrm.atb[aId].errCode NEQ SIM_NO_ERROR OR
      simShrdPrm.atb[aId].exchData EQ NULL         OR
      simShrdPrm.atb[aId].dataLen < ACI_CPHS_CSP_SIZE )
  {
    ;  /* CSP not supported or invalid */
  }
  else
  {
    for (i = 0; i < simShrdPrm.atb[aId].dataLen; i += 2)
    {
      if (simShrdPrm.atb[aId].exchData[i] EQ ALS_SERVICE_GROUP_CODE)
      {
        if (BITFIELD_CHECK(simShrdPrm.atb[aId].exchData[i+1],ALS_BIT_ON))
        {
          cmhCC_checkALS_Support_exec( 1 );
          return;
        }
        break;
      }
    }
  }

  cmhCC_checkALS_Support_exec( 0 );
}


GLOBAL void cmhCC_checkALS_Support_exec ( UBYTE flag )
{
  T_ACI_CMD_SRC ownBuf;       /* buffers current owner */
  T_ACI_ALS_MOD ALSmode;

  TRACE_FUNCTION ("cmhCC_checkALS_Support_exec()");

  ownBuf = simEntStat.entOwn;

  simEntStat.curCmd = AT_CMD_NONE;
  simShrdPrm.owner  = (T_OWN)CMD_SRC_NONE;
  simEntStat.entOwn = CMD_SRC_NONE;

  switch (ccShrdPrm.als_cmd)
  {
    case ALS_CMD_SET:
      if (flag)
      {
        cmhPrm[ownBuf].ccCmdPrm.ALSmode = ALS_MOD_AUX_SPEECH;
        R_AT( RAT_OK, ownBuf ) ( AT_CMD_ALS );
      }
      else
      {
        R_AT( RAT_CME, ownBuf ) ( AT_CMD_ALS, CME_ERR_OpNotSupp );
      }      
      break;
    case ALS_CMD_TEST:
      if (flag)
      {
        ALSmode = (T_ACI_ALS_MOD)(ALS_MOD_SPEECH | ALS_MOD_AUX_SPEECH);
      }
      else 
      {
        ALSmode = ALS_MOD_SPEECH;
      }
      R_AT( RAT_ALS, ownBuf ) ( ALSmode );
      R_AT( RAT_OK, ownBuf ) ( AT_CMD_ALS );
      break;
    default:
      TRACE_EVENT("wrong value in als_cmd");
      R_AT( RAT_CME, ownBuf ) ( AT_CMD_ALS, CME_ERR_OpNotSupp );
  }
  ccShrdPrm.als_cmd = ALS_CMD_NONE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCR                 |
|                                 ROUTINE : cmhCC_satTimeout  |
+-------------------------------------------------------------------+

  PURPOSE : handle redial timeout of SAT max duration

*/

GLOBAL void cmhCC_satTimeout( void )
{
  TRACE_FUNCTION( "cmhCC_satTimeout()" );
  /* Stop the redial timer if still running */
  TIMERSTOP (ACI_REPEAT_HND);
  /* Send Terminal response to SIM with Network unable to handle error */
  cmhSAT_NtwErr( (UBYTE)((GET_CAUSE_VALUE(ccShrdPrm.ctb[rdlPrm.rdlcId]->nrmCs) NEQ NOT_PRESENT_8BIT)? 
                           (ccShrdPrm.ctb[rdlPrm.rdlcId]->nrmCs|0x80) : ADD_NO_CAUSE));
  /* Reset the call table */
  psaCC_FreeCtbNtry (rdlPrm.rdlcId);
  /* Reset the SA maximum duration timer */
  satShrdPrm.dur = -1;
} /* End of cmhCC_satTimeout() */


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCR                 |
|                                 ROUTINE : cmhCC_redialTimeout     |
+-------------------------------------------------------------------+

  PURPOSE : handle redial timeout

*/

GLOBAL void cmhCC_redialTimeout ( void )
{
  int i;
  T_CC_CMD_PRM * pCCCmdPrm;   /* points to CC command parameters */
  T_SIM_SET_PRM * pSSetPrm;   /* points to SIM set parameters */

#ifdef SIM_TOOLKIT
  T_ACI_RETURN aciRet;
  T_ACI_SAT_TERM_RESP resp_data;
  psaSAT_InitTrmResp( &resp_data );
#endif /* SIM_TOOLKIT */

  TRACE_FUNCTION( "cmhCC_redialTimeout()" );

#ifdef SIM_TOOLKIT
  pSSetPrm = &simShrdPrm.setPrm[ccShrdPrm.ctb[rdlPrm.rdlcId]->curSrc];
  /* Check For --Envelope Call Control Always Sent To SIM Bit-- */
  if ((ccShrdPrm.ctb[rdlPrm.rdlcId]->SATinv & SAT_REDIAL) AND 
    (pSSetPrm->STKprof[0] & SAT_TP1_CC_ON_REDIAL))
  {
    /* Enable Call Control By SIM During AutoRedial */
    ccShrdPrm.ctb[rdlPrm.rdlcId]->SATinv |=  SAT_REDIAL_ECCBE;
    aciRet = cmhSAT_CalCntrlBySIM (rdlPrm.rdlcId);

    switch (aciRet)
    {
      case AT_CMPL:
      case AT_FAIL:
        /* respond with "Interaction with call control by SIM, permanent" */
        resp_data.add_content = ADD_NO_CAUSE;
      case AT_BUSY:
        /* respond with "Interaction with call control by SIM, temporary" */
        psaSAT_SendTrmResp( RSLT_CC_SIM_TMP, &resp_data );
        psaCC_FreeCtbNtry (rdlPrm.rdlcId);
        return;
      /*lint -fallthrough*/
      default:
        break;
    }
  }
#endif /* SIM_TOOLKIT */


  if (psaCC_ctbIsValid (rdlPrm.rdlcId))
  {
    pCCCmdPrm = &cmhPrm[psaCC_ctb(rdlPrm.rdlcId)->curSrc].ccCmdPrm;

    if (rdlPrm.rdlMod EQ AUTOM_REPEAT_ON)
    {
      if((psaCC_ctb(rdlPrm.rdlcId)->rdlTimIndex >= 1) AND 
         (psaCC_ctb(rdlPrm.rdlcId)->rdlTimIndex < 3))
      {
#ifdef _SIMULATION_
          TIMERSTART(ACI_REPEAT_1,ACI_REPEAT_HND);
#else
          psaCC_ctb(rdlPrm.rdlcId)->rdlTimIndex++;
          TIMERSTART(ACI_REPEAT_2_4,ACI_REPEAT_HND);
#endif
      }
      else
      {
        psaCC_ctb(rdlPrm.rdlcId)->rdlTimIndex = RDL_TIM_INDEX_NOT_PRESENT;
        psaCC_ctb(rdlPrm.rdlcId)->rdlCnt++;
        for(i = 0; i < CMD_SRC_MAX; i++)
        {
          R_AT(RAT_RDL,(T_ACI_CMD_SRC)i)(REDIAL_ATT_START);
        }
        psaCC_ctb(rdlPrm.rdlcId)->calType = CT_MOC_RDL;
        cmhCC_flagCall( rdlPrm.rdlcId, &(pCCCmdPrm->mltyCncFlg));
        /* start a new call */
        psaCC_NewCall(rdlPrm.rdlcId);
        rdlPrm.rdlcId = NO_ENTRY;
      }
    }
    else
    {
        TRACE_EVENT("cmhCC_redialTimeout: wrong cId");
    }
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_sendFie                |
+-------------------------------------------------------------------+

  PURPOSE : forward unsolicited facility responses from CC to terminal

*/
GLOBAL T_ACI_RETURN cmhCC_sendFie( T_ACI_FAC_DIR tDirection,
                                         SHORT         cId,
                                         T_MNCC_fac_inf    *fie )
{
  T_ACI_CMD_SRC tSrc;

  TRACE_EVENT( "cmhCC_sendFie()" );

  for( tSrc=CMD_SRC_LCL; tSrc<CMD_SRC_MAX; tSrc++ )  /* try over all sources */
  {
    if( cmhPrm[tSrc].ccCmdPrm.CSCNcc_mode.CcCSCNModeState EQ CC_CSCN_MOD_STATE_ON )
    {
      switch (cmhPrm[tSrc].ccCmdPrm.CSCNcc_mode.CcCSCNModeDirection)
      {
        case CC_CSCN_MOD_DIR_BOTH:
          break; /* printout regardless of direction */

        case CC_CSCN_MOD_DIR_IN:
          if( tDirection NEQ CSCN_FACILITY_DIRECTION_IN)
            continue; /* settings for source don't match, advance to next source */
          break;

        case CC_CSCN_MOD_DIR_OUT:
          if( tDirection NEQ CSCN_FACILITY_DIRECTION_OUT )
            continue; /* settings for source don't match, advance to next source */
          break;

        default:
          continue; /* illegal setting, advance to next source */
      }
      R_AT( RAT_CCCN, tSrc )( tDirection, cId, fie );
    }
  }
  return( AT_CMPL );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhCC_ChngWaitingToIncoming  |
+-------------------------------------------------------------------+

  PURPOSE : Change waiting call to incomimg call.

*/

GLOBAL void cmhCC_ChngWaitingToIncoming( void )
{
  SHORT cId;
  /* If there is a call in CS_ACT_REQ state then send ring ind to the src's */
  cId = psaCC_ctbFindCall( (T_OWN)CMD_SRC_NONE, CS_ACT_REQ, CT_MTC );
  if (cId NEQ NO_ENTRY)
  {
    if((psaCC_ctb(cId)->calStat EQ CS_ACT_REQ) AND (CHLDaddInfo EQ NO_CHLD_ADD_INFO))
    {
      cmhCC_IncomingCall(cId); /* There is a call in CS_ACT_REQ state */
    }
  }
}


/* Implements Measure 184, 185 and 187 */
/*
+------------------------------------------------------------------------------
|  Function    : cmhCC_tstAndUnflag_MPTYCall
+------------------------------------------------------------------------------
|  Purpose     : This Function will test and unflag a call for multy-call operation.
|
|  Parameters  : cId    - Call Id.
|                srcBuf - Buffers Current Command Source
|                cmdBuf - Buffers Current Command
|
|  Return      : void
+------------------------------------------------------------------------------
*/

LOCAL void cmhCC_tstAndUnflag_MPTYCall( SHORT cId, UBYTE srcBuf, UBYTE cmdBuf,
                                        T_CC_CMD_PRM *pCCCmdPrm )
{
  TRACE_FUNCTION ("cmhCC_tstAndUnflag_MPTYCall()");

  if( cmhCC_tstAndUnflagCall( cId, &(pCCCmdPrm -> mltyCncFlg)))
  {
    if( pCCCmdPrm -> mltyCncFlg EQ 0 AND
        pCCCmdPrm -> mltyDscFlg EQ 0 )
    {
      R_AT( RAT_OK, (T_ACI_CMD_SRC)srcBuf ) ( cmdBuf );

      /* log result */
      cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_OK,
                    (T_ACI_AT_CMD)cmdBuf, (SHORT)(cId+1), BS_SPEED_NotPresent,CME_ERR_NotPresent);
    }
  }
}

/* Implements Measure 32 */
/*
+------------------------------------------------------------------------------
|  Function    : cmhCC_check_isCallServiced
+------------------------------------------------------------------------------
|  Purpose     : This Function will check whether the Call was serviced or not.
|                If the call was found to be not serviced then add call 
|                to LMN Phone Book.
|
|  Parameters  : cId    - Call Id.
|
|  Return      : Returns TRUE if the Call is added to LMN Phone Book.
|                Returns FALSE if the Call is not added to LMN Phone Book.
+------------------------------------------------------------------------------
*/

LOCAL BOOL cmhCC_check_isCallServiced( SHORT cId )
{
  UBYTE idx;
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
    
  TRACE_FUNCTION ("cmhCC_check_isCallServiced()");
  
  if( ctb->calOwn EQ (T_OWN)CMD_SRC_NONE    AND
      ctb->curCmd EQ AT_CMD_NONE )
  {
    psaCC_phbAddNtry ( LMN, cId, CT_MTC, NULL );  /* add call to LMN PB */

#ifdef AT_INTERPRETER
    /* V.24 Ring Indicator Line */
    io_setRngInd (IO_RING_OFF, CRING_SERV_TYP_NotPresent, CRING_SERV_TYP_NotPresent ); /* V.24 Ring Indicator Line */
#endif

    for( idx = 0; idx < CMD_SRC_MAX; idx++ )
    {
      R_AT( RAT_CRING_OFF, (T_ACI_CMD_SRC)idx )( cId+1 );
    }
    return TRUE;
  }
  return FALSE;
}

/* Implements Measure 207 */
/*
+------------------------------------------------------------------------------
|  Function    : cmhCC_Call_Progress_Information
+------------------------------------------------------------------------------
|  Purpose     : This Function will provide the Information regarding the 
|                Progress of the call to the User.
|
|  Parameters  : cId    - Call Id.
|                msgType - Call Control Message Type
|
|  Return      : void
|+------------------------------------------------------------------------------
*/

LOCAL void cmhCC_Call_Progress_Information( SHORT cId, T_ACI_CPI_MSG msgType, USHORT cause )
{
  UBYTE idx;
  T_CC_CALL_TBL  *ctb = ccShrdPrm.ctb[cId];
  
  TRACE_FUNCTION ("cmhCC_Call_Progress_Information()");

  for( idx = 0; idx < CMD_SRC_MAX; idx++ )
  {
    R_AT( RAT_CPI, (T_ACI_CMD_SRC)idx )
      ( cId+1,
        msgType,
        (ctb->inBndTns)? CPI_IBT_True: CPI_IBT_False,
        (ccShrdPrm.TCHasg)? CPI_TCH_True: CPI_TCH_False,
        cause);
  }
}

/*==== EOF ========================================================*/

