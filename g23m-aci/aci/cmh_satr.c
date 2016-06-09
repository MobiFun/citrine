/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_SATR
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
|             the SIM application toolkit.
+----------------------------------------------------------------------------- 
*/ 

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#ifdef SIM_TOOLKIT

#ifndef CMH_SATR_C
#define CMH_SATR_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "aci_mem.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "aci.h"
#include "psa.h"
#include "psa_cc.h"
#include "psa_ss.h"
#include "psa_sat.h"
#include "psa_sim.h" /* for simShrdPrm declaration */
#include "psa_sms.h"
#include "psa_util.h"
#include "cmh.h"
#include "cmh_cc.h"
#include "cmh_sat.h"
#include "cmh_sms.h"
#include "aoc.h"

#include "phb.h"
#include "cmh_phb.h"
#include "l4_tim.h"
/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/

/*==== PROTOTYPES==================================================*/
/* Implements Measure # 57, 77 */
LOCAL void  cmhSAT_ResCntrlBySIM (T_ACI_AT_CMD        cmdBuf,
                                  UBYTE               simRslt,
                                  T_ACI_SAT_TERM_RESP *resp_data);

/* Implements Measure # 174 */
LOCAL void  cmhSAT_SIMCntrlAlertUser (BOOL            ownNotSAT,
                                      SHORT           cId
#ifdef FF_SAT_E
                                      ,T_ACI_SATA_ADD  *addPrm
#endif
                                      );

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CMH_SATR                   |
| STATE   : code                ROUTINE : cmhSAT_ModifyScaPdu        |
+--------------------------------------------------------------------+

  PURPOSE : modifies the SCA in a SUBMIT PDU

*/
LOCAL void cmhSAT_ModifyScaPdu( T_sms_sdu *sms_sdu,
                          UBYTE  *sca_buf,
                          UBYTE sca_len )
{
  UBYTE old_sca_len;
  UBYTE *rest;
  USHORT rest_len;
  UBYTE *old_sdu;
  UBYTE old_sdu_len;

  ACI_MALLOC(old_sdu, SIM_PDU_LEN);
  old_sdu_len = sms_sdu->l_buf/8;
  memcpy ( old_sdu, sms_sdu->buf, old_sdu_len );

  old_sca_len = *old_sdu+1;
  rest = old_sdu + old_sca_len;
  rest_len = old_sdu_len - old_sca_len;

  /* copy new SCA */
  memcpy(sms_sdu->buf, sca_buf, sca_len);

  /* copy the rest of the PDU */
  memcpy(sms_sdu->buf+sca_len, rest, rest_len);

  /* set the length */
  sms_sdu->l_buf = (sca_len+rest_len) * 8;

  ACI_MFREE(old_sdu);
}


/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CMH_SATR                   |
| STATE   : code                ROUTINE : cmhSAT_ModifyDaPdu         |
+--------------------------------------------------------------------+

  PURPOSE : modifies the destination address in a SUBMIT PDU

*/
LOCAL void cmhSAT_ModifyDaPdu( T_sms_sdu *sms_sdu,
                         UBYTE  *da_buf,
                         UBYTE da_len )
{
  UBYTE old_sca_len;
  UBYTE old_da_len;
  UBYTE *rest;
  USHORT rest_len;
  UBYTE *old_sdu;
  UBYTE *new_sdu;
  UBYTE old_sdu_len;

  ACI_MALLOC(old_sdu, SIM_PDU_LEN);
  old_sdu_len = sms_sdu->l_buf/8;
  memcpy ( old_sdu, sms_sdu->buf, old_sdu_len );

  new_sdu = sms_sdu->buf;

  old_sca_len = *old_sdu+1;
  old_da_len  = (*(old_sdu + old_sca_len + 2)+1)/2 + 2;
  rest = old_sdu + old_sca_len + 2 + old_da_len;
  rest_len = old_sdu_len - old_sca_len - 2 - old_da_len;

  /* copy old sca, fo, mr */
  memcpy(new_sdu, old_sdu, old_sca_len+2);
  new_sdu += old_sca_len+2;

  /* copy the new DA */
  memcpy(new_sdu, da_buf, da_len);
  new_sdu += da_len;

  /* copy the rest of the PDU */
  memcpy(new_sdu, rest, rest_len);

  /* set the length */
  sms_sdu->l_buf = (old_sca_len+2+da_len+rest_len) * 8;

  ACI_MFREE(old_sdu);
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SATR                     |
|                            ROUTINE : cmhSAT_STKCmdCnf             |
+-------------------------------------------------------------------+

  PURPOSE : STK command confirmation

*/

GLOBAL SHORT cmhSAT_STKCmdCnf ( void )
{

  TRACE_FUNCTION ("cmhSAT_STKCmdCnf()");

/*
 *-------------------------------------------------------------------
 * check for command context
 *-------------------------------------------------------------------
 */
  switch( satEntStat.curCmd )
  {
    case( AT_CMD_SATE ):
     /*
      *----------------------------------------------------------------
      * process event for %SATE command
      *----------------------------------------------------------------
      */
      satEntStat.curCmd = AT_CMD_NONE;

      TRACE_EVENT_P1("SAT reponse with SIM cause: %d", satShrdPrm.stkError);

      switch (satShrdPrm.stkError)
      {
        case SIM_NO_ERROR:
          R_AT( RAT_SATE, satEntStat.entOwn )
            ( satShrdPrm.stkCmdLen>>3, satShrdPrm.stkCmd );

          R_AT( RAT_OK, satEntStat.entOwn )
            ( AT_CMD_SATE );
          break;

        case SIM_CAUSE_SAT_BUSY:
          R_AT( RAT_CME, satEntStat.entOwn )
            ( AT_CMD_SATE, CME_ERR_SimSatBusy );
          break;

        case SIM_CAUSE_PUK1_BLOCKED:
          R_AT( RAT_CME, satEntStat.entOwn )
            ( AT_CMD_SATE, CME_ERR_SimFail );
          break;

        default:
          R_AT( RAT_CME, satEntStat.entOwn )
            ( AT_CMD_SATE, CME_ERR_Unknown );
          break;
      }
      break;
  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SATR                     |
|                            ROUTINE : cmhSAT_STKCmdInd             |
+-------------------------------------------------------------------+

  PURPOSE : STK command indication

*/

GLOBAL SHORT cmhSAT_STKCmdInd ( void )
{
  SHORT idx;                 /* holds list index */

  TRACE_FUNCTION ("cmhSAT_STKCmdInd()");

/*
 *----------------------------------------------------------------
 * send unsolicited result
 *----------------------------------------------------------------
 */

  for( idx = 0; idx < CMD_SRC_MAX; idx++ )
  {
    R_AT( RAT_SATI, (T_ACI_CMD_SRC)idx )
      ( satShrdPrm.stkCmdLen>>3, satShrdPrm.stkCmd );
  }
  return 0;
}

#ifdef TI_PS_FF_AT_P_CMD_CUST
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SATR                     |
|                            ROUTINE : cmhSAT_Cust1StkCmdInd             |
+-------------------------------------------------------------------+

  PURPOSE : Used for Sending the Origianl STK command indication to a Cust1 MMI,
                   when performing Setup Call, Send SS, Send USSD or Send SMS
*/

GLOBAL SHORT cmhSAT_Cust1StkCmdInd ( void )
{
  SHORT idx;                 /* holds list index */

  TRACE_FUNCTION ("cmhSAT_Cust1StkCmdInd()");

  if (satShrdPrm.cust1StkCmd != (void *)0)
  {
      /*
       *----------------------------------------------------------------
       * send %SATI the MMI with the Stk Cmd
       *----------------------------------------------------------------
       */
       
        for( idx = 0; idx < CMD_SRC_MAX; idx++ )
        {
          R_AT( RAT_SATI,(T_ACI_CMD_SRC)idx )
            ( satShrdPrm.cust1StkCmdLen >>3, satShrdPrm.cust1StkCmd);
        }

        ACI_MFREE(satShrdPrm.cust1StkCmd);

        satShrdPrm.cust1StkCmd = (void *)0;
        satShrdPrm.cust1StkCmdLen = 0;
  }

  return 0;
}
#endif /* TI_PS_FF_AT_P_CMD_CUST */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SATR                     |
|                            ROUTINE : cmhSAT_STKUsrNtfy            |
+-------------------------------------------------------------------+

  PURPOSE : STK user notification

*/

GLOBAL SHORT cmhSAT_STKUsrNtfy ( void )
{
  SHORT idx;                 /* holds list index */

  T_ACI_SATN_CNTRL_TYPE cntrl_type = SATN_CNTRL_BY_SIM_Not_Present;

  TRACE_FUNCTION ("cmhSAT_STKUsrNtfy()");

/*
 *----------------------------------------------------------------
 * determine user notification
 *----------------------------------------------------------------
 */
  switch( satShrdPrm.ntfy )
  {
    case( USR_NTF_CC_SIM ):

#ifdef TI_PS_FF_AT_P_CMD_CUST
      if (simShrdPrm.overall_cust_mode EQ CUST_MODE_BEHAVIOUR_1)
      {
          switch (satShrdPrm.SIMCCParm.ccAct)
          {
                case CC_ACT_CAL:
                    cntrl_type = SATN_CNTRL_BY_SIM_CALL;
                    break ;

                case CC_ACT_SS:
                    cntrl_type = SATN_CNTRL_BY_SIM_SS;
                    break ;

                case CC_ACT_USSD:
                    cntrl_type = SATN_CNTRL_BY_SIM_USSD;
                    break ;

                case SMC_ACT_MO:
                    cntrl_type = SATN_CNTRL_BY_SIM_SMS;
                    break ;

                 default:
                    cntrl_type = SATN_CNTRL_BY_SIM_Not_Present;
          }
      }
#endif /* TI_PS_FF_AT_P_CMD_CUST */

      if(psa_IsVldOwnId((T_OWN)satShrdPrm.owner) AND
         satShrdPrm.owner NEQ OWN_SRC_INV      )
      {
        R_AT( RAT_SATN, (T_ACI_CMD_SRC)satShrdPrm.owner )
          ( satShrdPrm.stkCmdLen>>3, satShrdPrm.stkCmd, cntrl_type );
        break;
      }
      /* otherwise continue, no break */
      /*lint -fallthrough*/
    default:

      for( idx = 0; idx < CMD_SRC_MAX; idx++ )
      {
        R_AT( RAT_SATN, (T_ACI_CMD_SRC)idx )
        ( satShrdPrm.stkCmdLen>>3, satShrdPrm.stkCmd, cntrl_type );
      }
  }

  return 0;
}

LOCAL void send_error_to_user( BOOL  ownNotSAT,
                               UBYTE command,
                               UBYTE result,
                               UBYTE additional_result,
                               UBYTE *resId,
                               BOOL  satopc )
{
  T_ACI_SAT_TERM_RESP resp_data;

  psaSAT_InitTrmResp( &resp_data );

  if( ownNotSAT )
  {
    /* setup initiated by user */
    R_AT( RAT_CME,(T_ACI_CMD_SRC)satShrdPrm.SIMCCParm.owner )
      ( command, CME_ERR_NotPresent );
    /* log result */
    cmh_logRslt ( (T_ACI_CMD_SRC)satShrdPrm.SIMCCParm.owner, RAT_CME, 
                   (T_ACI_AT_CMD)command, -1, BS_SPEED_NotPresent, CME_ERR_NotPresent );
    return;
  }

  /* setup initiated by SAT */
  /* send SAT response */
  resp_data.add_content = additional_result;

  /* if source was open channel command */
#ifdef FF_SAT_E  
  if(satopc)
  {
    if( satShrdPrm.opchStat EQ OPCH_CCSIM_REQ )
    {
       cmhSAT_OpChnFailed( result, &resp_data );
       return;
    }
  }
#endif /* FF_SAT_E */

  if( resId NEQ NULL )
  { 
    resp_data.resCC = resId;
  }
  psaSAT_SendTrmResp( result, &resp_data );
}

/* return TRUE if key sequence accepted, FALSE otherwise */
LOCAL BOOL sim_control_send_ss( T_CLPTY_PRM *ss_clpty, 
                                UBYTE       command, 
                                BOOL        ownNotSAT, 
                                UBYTE       *resCC )
{
  T_ACI_RETURN      retVal;             /* holds return value */
  T_ACI_D_CLIR_OVRD dummy2;             /* dummy value */
  T_ACI_D_TOC       dummy1;             /* dummy value */

  /* check for busy SS condition */
  if( psaSS_stbFindActSrv( NO_ENTRY ) NEQ NO_ENTRY )
  {
    /* NOTE: shouldn't that be:
    send_error_to_user( ownNotSAT, RSLT_CC_SIM_PRM, ADD_ME_SS_BUSY, resCC, FALSE);
    or send_error_to_user( ownNotSAT, RSLT_CC_SIM_PRM, RSLT_ME_CAP, resCC, FALSE);
    ??*/
    send_error_to_user( ownNotSAT, command, RSLT_ME_UNAB_PROC, ADD_ME_SS_BUSY, resCC, FALSE);
    return FALSE;
  }

  /* send SS control string */
  retVal = cmhCC_chkKeySeq ( (T_ACI_CMD_SRC)satShrdPrm.SIMCCParm.owner,
                             ss_clpty,
                             &dummy1,
                             &dummy2,
                             CC_SIM_NO );
  if( retVal NEQ AT_EXCT )
  {
    /* string is not SS: terminate with error */
//TISH, patch for OMAPS00126322
//start
	if (retVal EQ AT_CMPL)
	{
		TRACE_ERROR("send RSLT_PERF_SUCCESS");
		send_error_to_user( ownNotSAT, command, RSLT_PERF_SUCCESS, RSLT_PERF_MDFY_SIM, resCC, FALSE);
		return(TRUE);
	}
//end
    send_error_to_user( ownNotSAT, command, RSLT_CC_SIM_PRM, RSLT_ME_CAP, resCC, FALSE);
    return(FALSE);
  }
  else
  {
#ifdef FF_SAT_E
    if(!ownNotSAT AND satShrdPrm.opchStat EQ OPCH_CCSIM_REQ )
    {
      return( FALSE );
    }
#endif /* FF_SAT_E */     
    return(TRUE);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SATR                     |
|                            ROUTINE : cmhSAT_ResCalCntrlBySIM      |
+-------------------------------------------------------------------+

  PURPOSE : Handle SIM call control result. Return TRUE if primitive
            is needed for building a terminal response.

*/
LOCAL BOOL cmhSAT_result_sim_cc_ss( SHORT cId,
                                    UBYTE *resId, 
                                    T_ccr_allw *ccr )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
  T_ACI_SAT_TERM_RESP resp_data;
  BOOL  ownNotSAT = (satShrdPrm.SIMCCParm.owner NEQ OWN_SRC_SAT);  /* flags that the owner is not SAT */
  UBYTE cmdBuf;                         /* buffer command */

  TRACE_FUNCTION("cmhSAT_result_sim_cc_ss()");

  psaSAT_InitTrmResp( &resp_data );
  
#ifdef FF_SAT_E 
  /* check for a pending OPEN CHANNEL command */
  if (ctb->calStat EQ CS_SAT_CSD_REQ)
  {
    /* free ctb entry */
    psaCC_FreeCtbNtry (cId);

    /* respond with "error, Interaction with CC by SIM, perm problem" */
    resp_data.add_content = ADD_CC_REQ_CHNG;
    resp_data.chnStat = TRUE;

    cmhSAT_OpChnFailed( RSLT_CC_SIM_PRM, &resp_data );

    return( FALSE );
  }
#endif /* FF_SAT_E */ 

  cmdBuf = ctb->curCmd;
  psaCC_FreeCtbNtry (cId);

  cmhCC_init_cldPty( &satPndSetup.clpty );

  utl_BCD2DialStr( ccr->ss_string.ss_ctrl_string,
                   satPndSetup.clpty.num,
                   (UBYTE)MINIMUM(ccr->ss_string.c_ss_ctrl_string,
                                  MAX_DIAL_LEN-1));

  satPndSetup.clpty.ton = ccr->ss_string.noa;
  satPndSetup.clpty.npi = ccr->ss_string.npi;

  if( sim_control_send_ss( &satPndSetup.clpty, cmdBuf, ownNotSAT, resId ) )
    return( !ownNotSAT );
  return( FALSE );  /* primitive not needed anymore */
}

LOCAL BOOL cmhSAT_result_sim_cc_address( SHORT cId, T_ccr_allw *ccr )
{
  SHORT retVal = 0;
  T_ACI_SAT_TERM_RESP resp_data;

  TRACE_FUNCTION("cmhSAT_result_sim_cc_address()");
  
  psaSAT_InitTrmResp( &resp_data );

  /* adjust called address */
  cmhSAT_fillSetupPrm ( cId,
                         &ccr->addr,
                        ((ccr->v_subaddr)?&ccr->subaddr:NULL));
#ifdef FF_SAT_E  
  if( psaCC_ctb(cId)->calStat NEQ CS_SAT_CSD_REQ )
#endif /* FF_SAT_E */
  {
    /* new call table entry set default bearer capabilities */
    cmhSAT_fillSetupBC ( cId, MNCC_BEARER_SERV_SPEECH, MNCC_BEARER_SERV_NOT_PRES );
  }

  /* Store the BC repeat indicator */
  if ( ccr->v_bc_rpi )


  {
    ccShrdPrm.ctb[cId]->rptInd = ccr->bc_rpi;
  }

  /* adjust bearer capabilities */
  if( ccr->v_cap_cnf_parms OR ccr->v_cap_cnf_parms_2 )
  {
    satShrdPrm.capParm.cId   = cId;
    satShrdPrm.capParm.CCres = CCR_ALLW_WITH_MDFY;
    satShrdPrm.capParm.cntxt = CTX_CC_RESULT;

    if ( ccr->v_cap_cnf_parms AND ccr->v_cap_cnf_parms_2 )
    {
      retVal= psaCC_BCapDecode( BCRI_SAT,
                                 (UINT16)(ccr->cap_cnf_parms.l_cap_cnf_parms>>3),
                                 ccr->cap_cnf_parms.b_cap_cnf_parms,
                                 (UINT16)(ccr->cap_cnf_parms_2.l_cap_cnf_parms_2>>3),
                                 ccr->cap_cnf_parms_2.b_cap_cnf_parms_2);
    }
    else if ( ccr->v_cap_cnf_parms )
    {
      retVal= psaCC_BCapDecode( BCRI_SAT,
                        (UINT16)(ccr->cap_cnf_parms.l_cap_cnf_parms>>3),
                        ccr->cap_cnf_parms.b_cap_cnf_parms,
                        0,
                        NULL);
    }
    if( retVal < 0 )
    {
      resp_data.resCC = &satShrdPrm.capParm.CCres;
      psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC,  &resp_data );
      psaCC_FreeCtbNtry (cId);
    }
    return( FALSE );
  }
  return( TRUE );
}


LOCAL BOOL cmhSAT_result_sim_cc_ussd( SHORT cId,
                                      UBYTE *resId, 
                                      T_ccr_allw *ccr  )
{
  T_ACI_SAT_TERM_RESP resp_data;
  BOOL                ownNotSAT = (satShrdPrm.SIMCCParm.owner NEQ OWN_SRC_SAT);  /* flags that the owner is not SAT */
  UBYTE               cmdBuf;                         /* buffer command */
  UBYTE               len = 0;
  T_ACI_RETURN        retVal;
  T_ACI_D_TOC         dummy1;
  T_ACI_D_CLIR_OVRD   dummy2;

  TRACE_FUNCTION("cmhSAT_result_sim_cc_ussd()");

  psaSAT_InitTrmResp( &resp_data );

  /* check for busy SS condition */
  if( psaSS_stbFindActSrv( NO_ENTRY ) NEQ NO_ENTRY )
  {
    /* respond with "error, ME currently unable to process command" */
    resp_data.add_content = ADD_ME_SS_BUSY;
    resp_data.resCC       = resId;
    psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
    return( FALSE );
  }

  /* call adr to be modified with USSD string*/
  cmdBuf = psaCC_ctb(cId)->curCmd;
  psaCC_FreeCtbNtry (cId);
  /* Implements Measure 25 */
  /* This function is more correct than utl_getAlphabetCb as it takes care 
     of reserved codings */
  if( cmh_getAlphabetCb( (UBYTE) ccr->ussd_string.dcs) EQ 0 ) /* 7bit alphabet */
  {
     len=utl_cvt7To8( ccr->ussd_string.ussd_str,
                      ccr->ussd_string.c_ussd_str,
                      (UBYTE*)&satPndSetup.clpty.num, 0);
     satPndSetup.clpty.num[len]=0x00;
  }
  else
  {
     memcpy( satPndSetup.clpty.num, ccr->ussd_string.ussd_str,
             ccr->ussd_string.c_ussd_str );
     satPndSetup.clpty.num[ccr->ussd_string.c_ussd_str]=0x00;
  }

  retVal = cmhCC_chkKeySeq ( (T_ACI_CMD_SRC)satShrdPrm.SIMCCParm.owner,
                             &satPndSetup.clpty, 
                             &dummy1, &dummy2,
                             CC_SIM_NO );

  if( retVal EQ AT_EXCT )
  {
    return( !ownNotSAT );
  }
  /* Implements Measure # 57 */
  /* Implements Measure # 57, 77 */

  /* Fill resp_data content in case it is needed for psaSAT_SendTrmResp 
     in cmhSAT_ResCntrlBySIM */
  resp_data.resCC = resId;  

  cmhSAT_ResCntrlBySIM ((T_ACI_AT_CMD)cmdBuf, RSLT_ME_CAP, &resp_data);

  return( FALSE );  /* primitive not needed anymore */
}


GLOBAL BOOL cmhSAT_ResCalCntrlBySIM( UBYTE* resId, void *ccRes )
{
  UBYTE cmdBuf;                         /* buffer command */
  SHORT cId = satShrdPrm.SIMCCParm.cId; /* holds setup call id */
  SHORT actId;                          /* holds active call id */
  T_ccr_allw * ccr;                     /* points to CC result */
  UBYTE idx;                            /* holds index */
  T_ACI_SAT_TERM_RESP resp_data;

#if defined SMI OR defined MFW
  T_ACI_CLOG     cmdLog;      /* holds logging info */
  CHAR           cmdlog_num_buf[MNCC_MAX_CC_CALLED_NUMBER];  /* buffer where number is written for cmdLog */
  USHORT         length;                       /* length of number */
#endif

  TRACE_FUNCTION("cmhSAT_ResCalCntrlBySIM()");

  psaSAT_InitTrmResp( &resp_data );

  ccr = (T_ccr_allw*)ccRes;

  /* check if queued event download */
  if (satShrdPrm.event.c_queued)
  {
    cmhSAT_EventDwn((UBYTE)-1, -1, END_UNDEFINED);	
  }

  satShrdPrm.SIMCCParm.busy = FALSE;

  if (!psaCC_ctbIsValid (cId))
  {
    TRACE_ERROR ("Call table entry disappeared");
    return FALSE;
  }

#ifdef TI_PS_FF_AT_P_CMD_CUST
  /*
  ** If this is a Cust1 MMI and The originator was a STK Cmd
  */
  if ((simShrdPrm.overall_cust_mode EQ (UBYTE)CUST_MODE_BEHAVIOUR_1) AND
       (satShrdPrm.ownSAT EQ TRUE))
  {
    /*
    ** Customised behaviour for the Cust1 MMI
    */
    if (*resId EQ CCR_NOT_ALLW)
    {
        /* Call Setup not allowed */
          send_error_to_user( !satShrdPrm.ownSAT, psaCC_ctb(cId)->curCmd, 
                            RSLT_CC_SIM_PRM, ADD_CC_NOT_ALLWD, 
                            NULL, TRUE);
    }
    else
    {
        /*
        ** This was a SAT initiated call. For the Cust1 the ACI does not yet have any
        ** actions to perform, other than the CC By SIM which has been done. So :
        ** Send the Original STK Cmd to the MMI in a %SATI for the MMI to process.
        **
        ** The second action will cause the MMI to request the approriate Call Setup, SS
        ** or USSD Request and the required Call Table entry will then be re-created.
        */
        cmhSAT_Cust1StkCmdInd();
    }
  
    /* Free the Call Table Entry
    ** The SATI indication (if sent) will cause the MMI to request the approriate Call Setup, SS
    ** or USSD Request and the required Call Table entry will then be re-created.
    */
    psaCC_FreeCtbNtry (cId);

    return( FALSE );  /* primitive not needed anymore */
  }
#endif /* TI_PS_FF_AT_P_CMD_CUST */

  /* determine result id */
  switch( *resId )
  {
    case( CCR_NOT_ALLW ):
      /* call setup not allowed */
      send_error_to_user( !satShrdPrm.ownSAT, psaCC_ctb(cId)->curCmd, 
                          RSLT_CC_SIM_PRM, ADD_CC_NOT_ALLWD, 
                          NULL, TRUE);

      /* memory for number has also been allocated in case of SAT setup call !! */
      psaCC_FreeCtbNtry (cId);
      return( FALSE );  /* primitive not needed anymore */

    case( CCR_ALLW_WITH_MDFY ):
      /* call setup allowed with modification */
      if( ccr->v_ss_string )
      {
        /* if setup was converted into a SS control string */
        return(cmhSAT_result_sim_cc_ss( cId, resId, ccr ));
      }
      else if( ccr->v_addr )
      {
        /* Replace call parameters for this cId with the ones provided by SIM */
         if ( !(cmhSAT_result_sim_cc_address( cId, ccr )) )
        {
          return TRUE;
        }
      }
      else if (ccr->v_ussd_string)
      {
        /* if setup was converted into a USSD control string */
        return(cmhSAT_result_sim_cc_ussd( cId, resId, ccr ));
      }
      else
      {} /* do not change cId parameters */
      break;

    case( CCR_ALLW_NO_MDFY ):
      /* call setup allowed no modification */
      /* use parameters already linked to cId */
      break;
  }

  /* perform call setup initiated by user */
  if( !satShrdPrm.ownSAT )
  {
    /* check for an active call */
    actId = psaCC_ctbFindCall( OWN_SRC_INV, CS_ACT, NO_VLD_CT );

    if( actId NEQ NO_ENTRY )
    {
      /* put active on hold if possible */
      if( psaCC_ctb(actId)->prio EQ MNCC_PRIO_NORM_CALL AND
          cmhCC_getcalltype(actId) EQ VOICE_CALL )
      {
//TISH, patch for OMAPS00129157
//start
#if 0
        cmhCC_HoldCall(actId, psaCC_ctb(cId)->curSrc, AT_CMD_D);
#else
	CHLDaddInfo=CHLD_ADD_INFO_DIAL_CAL;
	cmhPrm[actId].ccCmdPrm.CHLDmode = CHLD_MOD_HldActDial;
        cmhCC_HoldCall(actId, psaCC_ctb(cId)->curSrc, AT_CMD_D);
#endif
//end
      }
      /* reject call setup: already data or emergency call on going */
      else
      {
        cmdBuf = psaCC_ctb(cId)->curCmd;
        psaCC_FreeCtbNtry (cId);

        R_AT( RAT_CME, (T_ACI_CMD_SRC)satShrdPrm.SIMCCParm.owner )
          ( cmdBuf, CME_ERR_NotPresent );

        /* log result */
        cmh_logRslt ( (T_ACI_CMD_SRC)satShrdPrm.SIMCCParm.owner, RAT_CME, 
                           (T_ACI_AT_CMD)cmdBuf,-1, BS_SPEED_NotPresent, CME_ERR_NotPresent );

        return( FALSE );  /* primitive not needed anymore */
      }
    }

    /* finally set up call */
    cmhCC_flagCall( cId,
                    &(cmhPrm[psaCC_ctb(cId)->calOwn].ccCmdPrm.mltyCncFlg));

//TISH, patch for OMAPS00129157
//start
    if (CHLDaddInfo!=CHLD_ADD_INFO_DIAL_CAL || cmhPrm[actId].ccCmdPrm.CHLDmode != CHLD_MOD_HldActDial)
//end
      psaCC_NewCall (cId);


#if defined SMI OR defined MFW
    /* log command */
    cmdLog.atCmd                = AT_CMD_D;
    cmdLog.cmdType              = CLOG_TYPE_Set;
    cmdLog.retCode              = AT_EXCT;
    cmdLog.cId                  = cId+1;
    cmdLog.sId                  = ACI_NumParmNotPresent;
    cmdLog.cmdPrm.sD.srcId      = (T_ACI_CMD_SRC)psaCC_ctb(cId)->curSrc;

    cmdLog.cmdPrm.sD.number     = cmdlog_num_buf;

    /* 
     * For this block wrong code is generated by target compiler 1.22e
     * Use psaCC_ctb() to simplify pointer expression.
     */
#if 0
    if(ccShrdPrm.ctb[cId]->cldPty.c_called_num < MNCC_MAX_CC_CALLED_NUMBER)
    {
      length = ccShrdPrm.ctb[cId]->cldPty.c_called_num;
    }
    else
      length = MNCC_MAX_CC_CALLED_NUMBER;

    utl_BCD2String( cmdLog.cmdPrm.sD.number,
                   (UBYTE *) ccShrdPrm.ctb[cId]->cldPty.called_num, 
                   length);
#else
    if(psaCC_ctb(cId)->cldPty.c_called_num < MNCC_MAX_CC_CALLED_NUMBER)
    {
      length = psaCC_ctb(cId)->cldPty.c_called_num;
    }
    else
      length = MNCC_MAX_CC_CALLED_NUMBER;

    utl_BCD2String (cmdLog.cmdPrm.sD.number,
                    psaCC_ctb(cId)->cldPty.called_num, 
                    length);
#endif /* else, #if 0 */

    cmdLog.cmdPrm.sD.clirOvrd   = D_CLIR_OVRD_Default; /* ??? */
    cmdLog.cmdPrm.sD.cugCtrl    = D_CUG_CTRL_NotPresent; /* ??? */
    cmdLog.cmdPrm.sD.callType   = (T_ACI_D_TOC)psaCC_ctb(cId)->calType;
#ifdef SIM_TOOLKIT
    cmdLog.cmdPrm.sD.simCallCtrl= D_SIMCC_ACTIVE;
#endif  /* SIM tOOLKIT */
    rAT_PercentCLOG( &cmdLog );
#endif  /* SMI or MFW */
    return( FALSE );
  }
  /* perform call setup initiated by SAT */
  else /* Must be normal behaviour, otherwise it would have been trapped earlier in the function */
  {
  
#ifdef FF_SAT_E
   /* if source was open channel command */
   if( satShrdPrm.opchStat EQ OPCH_CCSIM_REQ )
   {
      if(*resId EQ CCR_ALLW_WITH_MDFY) 
        satShrdPrm.opchCCMdfy = TRUE;

      cmhSAT_OpChnAlert( cId );
      return( FALSE );
   }
#endif /* FF_SAT_E */
    
#ifdef SIM_TOOLKIT
   /* When Envelope Call Control is Enabled SATA should not go again */
   if (ccShrdPrm.ctb[cId]->SATinv & SAT_REDIAL_ECCBE)
   {
     return( *resId EQ CCR_ALLW_WITH_MDFY );
   }
#endif

   /* alert user if command details are supported */
    if( cmhSAT_ChckCmdDet() )
    {
     /* check aoc condition */
      if ((psaCC_ctb(cId)->prio EQ MNCC_PRIO_NORM_CALL) AND
          (aoc_check_moc() EQ FALSE))
        /*
         * check ACM exceeds ACMmax
         * for non-emergency calls
         */
      {
        resp_data.add_content = ADD_NO_CAUSE;
        resp_data.resCC = (*resId EQ CCR_ALLW_WITH_MDFY)? resId: NULL;


        psaSAT_SendTrmResp(RSLT_ME_UNAB_PROC, &resp_data);
        psaCC_FreeCtbNtry (cId);
        return( FALSE );
      }

      for( idx = 0; idx < CMD_SRC_MAX; idx++ )
      {

#ifdef FF_SAT_E 
         T_ACI_SATA_ADD addPrm;

         addPrm.chnType = SATA_CT_VOICE;
         addPrm.chnEst  = SATA_EST_IM;

         R_AT( RAT_SATA, (T_ACI_CMD_SRC)idx )( cId+1, satShrdPrm.dur, &addPrm );
#else          
         R_AT( RAT_SATA, (T_ACI_CMD_SRC)idx )( cId+1, satShrdPrm.dur );         
#endif /* FF_SAT_E */         
      }

      satShrdPrm.ntfy = USR_NTF_SETUP_CAL;
    }
    return( FALSE );
  }
}

LOCAL BOOL sim_control_setup_call( T_ccr_allw *ccr, BOOL ownNotSAT )
{
  SHORT cId = satShrdPrm.SIMCCParm.cId; /* holds setup call id */

#ifdef FF_SAT_E  
  T_ACI_SATA_ADD addPrm;
#endif /* FF_SAT_E */
  cId = psaCC_ctbNewEntry();

  if( cId EQ NO_ENTRY )
  {
    send_error_to_user( ownNotSAT, AT_CMD_D, RSLT_ME_UNAB_PROC, ADD_NO_CAUSE, NULL, FALSE);
    return( FALSE );  /* primitive not needed anymore */
  }

  /* fill in setup parameter for called address */
  cmhSAT_fillSetupPrm ( cId,
                        ((ccr->v_addr)?&ccr->addr:NULL),
                        ((ccr->v_subaddr)?&ccr->subaddr:NULL));
  /* new call table entry set default bearer capabilities */
  cmhSAT_fillSetupBC ( cId, MNCC_BEARER_SERV_SPEECH, MNCC_BEARER_SERV_NOT_PRES );

  /* check aoc condition */
  if ((psaCC_ctb(cId)->prio EQ MNCC_PRIO_NORM_CALL) AND
      (aoc_check_moc() EQ FALSE))
    /* check ACM exceeds ACMmax for non-emergency calls */
  {
    send_error_to_user( ownNotSAT, AT_CMD_D, RSLT_ME_UNAB_PROC, ADD_NO_CAUSE, NULL, FALSE);
    psaCC_FreeCtbNtry (cId);
    return( FALSE );
  }

  if( ccr->v_cap_cnf_parms )
  {
    /* check bearer caps, ctb not allocated */
  }

  /* declare call table entry as used and the owner and status
     of the call */
  psaCC_ctb(cId)->calOwn     = (T_OWN)satShrdPrm.SIMCCParm.owner;
  psaCC_ctb(cId)->calStat    = CS_SAT_REQ;
  psaCC_ctb(cId)->curCmd     = AT_CMD_D;
  psaCC_ctb(cId)->curSrc     = satShrdPrm.SIMCCParm.owner;

  if( !ownNotSAT ) psaCC_ctb(cId)->SATinv = TRUE;

  /* Implements Measure # 174 */
  cmhSAT_SIMCntrlAlertUser (ownNotSAT, cId
#ifdef FF_SAT_E
                          , &addPrm
#endif
    );
  return( !ownNotSAT );
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SATR                     |
|                            ROUTINE : cmhSAT_ResSSCntrlBySIM       |
+-------------------------------------------------------------------+

  PURPOSE : Handle SIM SS control result. Return TRUE if primitive
            is needed for building a terminal response.

*/

GLOBAL BOOL cmhSAT_ResSSCntrlBySIM( UBYTE* resId, void *ccRes )
{
  SHORT cId = satShrdPrm.SIMCCParm.cId; /* holds setup call id */
  T_ccr_allw * ccr;                     /* points to CC result */
  BOOL  ownNotSAT;                      /* flags that the owner is not SAT */

  TRACE_FUNCTION("cmhSAT_ResSSCntrlBySIM()");
  
  ccr = (T_ccr_allw*)ccRes;

  /* check if queued event download */
  if (satShrdPrm.event.c_queued)
  {
    cmhSAT_EventDwn( (UBYTE)-1, -1, END_UNDEFINED);
  }

  satShrdPrm.SIMCCParm.busy = FALSE;

  ownNotSAT = satShrdPrm.SIMCCParm.owner NEQ OWN_SRC_SAT;

#ifdef TI_PS_FF_AT_P_CMD_CUST
  /*
  ** If this is a Cust1 MMI and The originator was a STK Cmd
  */
  if ((simShrdPrm.overall_cust_mode EQ (UBYTE)CUST_MODE_BEHAVIOUR_1) AND
      (ownNotSAT == FALSE))
  {
      /*
      ** Customised behaviour for the Cust1 MMI
      */
      if (*resId EQ CCR_NOT_ALLW)
      {
          send_error_to_user( ownNotSAT,
                                             AT_CMD_D,
                                             RSLT_CC_SIM_PRM,
                                             ADD_CC_NOT_ALLWD,
                                             NULL, FALSE);
      }
      else
      {
          /*
          ** This was a SAT initiated Request. For the Cust1 the ACI does not yet have any
          ** actions to perform, other than the CC By SIM which has been done. So :
          ** Send the Original STK Cmd to the MMI in a %SATI for the MMI to process.
          **
          ** The second action will cause the MMI to request the approriate Call Setup, SS
          ** or USSD Request and the required Call Table entry will then be re-created.
          */
          cmhSAT_Cust1StkCmdInd();
      }
    
      /* Free the Call Table Entry
      ** The SATI indication (if sent) will cause the MMI to request the approriate Call Setup, SS
      ** or USSD Request and the required Call Table entry will then be re-created.
      */
      if (psaCC_ctbIsValid (cId))
      {
        psaCC_retMOCTi(psaCC_ctb(cId)->ti);
        psaCC_chngCalTypCnt(cId, -1);
        psaCC_FreeCtbNtry (cId);
      }
      return( FALSE );  /* primitive not needed anymore */
  }
#endif /* TI_PS_FF_AT_P_CMD_CUST */

/* determine result id */
  switch( *resId )
  {
    case( CCR_NOT_ALLW ): /* SS not allowed */
    {
      send_error_to_user( ownNotSAT, AT_CMD_D, RSLT_CC_SIM_PRM, ADD_CC_NOT_ALLWD, NULL, FALSE);
      if (ownNotSAT EQ FALSE)
      {
        /* clear call ID */
        if (psaCC_ctbIsValid (cId))
        {
          psaCC_retMOCTi(psaCC_ctb(cId)->ti);
          psaCC_chngCalTypCnt(cId, -1);
          psaCC_FreeCtbNtry (cId);
        }
      }
      return( FALSE );  /* primitive not needed anymore */
    }
    case( CCR_ALLW_WITH_MDFY ): /* SS allowed with modification */
    {
      if( ccr->v_addr )
      {
        /* if SS control string was converted into a call setup */
        return(sim_control_setup_call( ccr, ownNotSAT ));
      }
      else if( ccr->v_ss_string )
      {
        /* if SS control string was changed by sim control */

        /* adjust SS control string */
        cmhCC_init_cldPty( &satPndSetup.clpty ); /* erase previous ss string */

        utl_BCD2DialStr( ccr->ss_string.ss_ctrl_string,
                         satPndSetup.clpty.num,
                         (UBYTE)MINIMUM(ccr->ss_string.c_ss_ctrl_string,
                                        MAX_DIAL_LEN-1));
        satPndSetup.clpty.ton = ccr->ss_string.noa;
        satPndSetup.clpty.npi = ccr->ss_string.npi;

        sim_control_send_ss( &satPndSetup.clpty, AT_CMD_D, ownNotSAT, NULL );
        return(FALSE);
      }
      else  /* if no optionnal field, then assume no modification see 11.14 ?9.1.6 */
        sim_control_send_ss( &satPndSetup.clpty, AT_CMD_D, ownNotSAT, NULL );
        return(FALSE);
    }
    /* SS allowed no modification */
    default:
      TRACE_ERROR ("Unknown resID in cmhSAT_ResSSCntrlBySIM!");
      /*lint -fallthrough*/
    case( CCR_ALLW_NO_MDFY ):
      sim_control_send_ss( &satPndSetup.clpty, AT_CMD_D, ownNotSAT, NULL );
      return( FALSE );  /* primitive not needed anymore */
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SATR                     |
|                            ROUTINE : cmhSAT_ResUSSDCntrlBySIM     |
+-------------------------------------------------------------------+

  PURPOSE : Handle SIM USSD control result. Return TRUE if primitive
            is needed for building a terminal response.

*/

GLOBAL BOOL cmhSAT_ResUSSDCntrlBySIM( UBYTE* resId, void *ccRes )
{
  T_ACI_RETURN retVal;                  /* holds return value */
  T_ccr_allw * ccr;                     /* points to CC result */
  BOOL  ownNotSAT;                      /* flags that the owner is not SAT */
  SHORT sId;
  T_ACI_SAT_TERM_RESP resp_data;
#ifdef FF_SAT_E 
  T_ACI_SATA_ADD addPrm;
#endif
  UBYTE *ussdString;
  UBYTE ussdLen;
  UBYTE src_len;



  TRACE_FUNCTION ("cmhSAT_ResUSSDCntrlBySIM()");

  psaSAT_InitTrmResp( &resp_data );

  ccr = (T_ccr_allw*)ccRes;

  /* check if queued event download */
  if (satShrdPrm.event.c_queued)
  {
    cmhSAT_EventDwn((UBYTE)-1, -1, END_UNDEFINED);
  }

  satShrdPrm.SIMCCParm.busy = FALSE;

  ownNotSAT = satShrdPrm.SIMCCParm.owner NEQ OWN_SRC_SAT;

#ifdef TI_PS_FF_AT_P_CMD_CUST
  /*
  ** If this is a Cust1 MMI and The originator was a STK Cmd
  */
  if ((simShrdPrm.overall_cust_mode EQ (UBYTE)CUST_MODE_BEHAVIOUR_1) AND
      (ownNotSAT EQ FALSE))
  {
      /*
      ** Customised behaviour for the Cust1 MMI
      */
      if (*resId EQ CCR_NOT_ALLW)
      {
          /* send SAT response */
          resp_data.add_content = ADD_CC_NOT_ALLWD;
          psaSAT_SendTrmResp( RSLT_CC_SIM_PRM, &resp_data );
      }
      else
      {
          /*
          ** This was a SAT initiated Request. For the Cust1 the ACI does not yet have any
          ** actions to perform, other than the CC By SIM which has been done. So :
          ** Send the Original STK Cmd to the MMI in a %SATI for the MMI to process.
          */
          cmhSAT_Cust1StkCmdInd();
      }

      return( FALSE );  /* primitive not needed anymore */
  }
#endif /* TI_PS_FF_AT_P_CMD_CUST */
/*
 *----------------------------------------------------------------
 * determine result id
 *----------------------------------------------------------------
 */
  switch( *resId )
  {
    /*
     *------------------------------------------------------------
     * SS not allowed
     *------------------------------------------------------------
     */
    case( CCR_NOT_ALLW ):

      /* Fill resp_data content in case it is needed for psaSAT_SendTrmResp 
         in cmhSAT_ResCntrlBySIM */
      resp_data.add_content = ADD_CC_NOT_ALLWD;

      /* Implements Measure # 57, 77 */
      cmhSAT_ResCntrlBySIM (AT_CMD_D, RSLT_CC_SIM_PRM, &resp_data);
      return( FALSE );  /* primitive not needed anymore */

    /*
     *------------------------------------------------------------
     * USSD allowed with modification
     *------------------------------------------------------------
     */
    case( CCR_ALLW_WITH_MDFY ):

      /* if USSD control string was converted into a call setup */
      if( ccr->v_addr )
      {
        SHORT cId = psaCC_ctbNewEntry();

        if( cId EQ NO_ENTRY )
        {
          /* 
           * We got no free slot in the call table
           */
          /* Implements Measure # 77 */
          /* Fill resp_data content in case it is needed for psaSAT_SendTrmResp 
             in cmhSAT_ResCntrlBySIM */
          resp_data.add_content = ADD_NO_CAUSE;
          cmhSAT_ResCntrlBySIM (AT_CMD_D, RSLT_ME_UNAB_PROC, &resp_data);
          return( FALSE );  /* primitive not needed anymore */
        }

        /*
         * We have successfully allocated a slot in the call table
         */

        /* fill in setup parameter for called address */
        cmhSAT_fillSetupPrm ( cId,
                              ((ccr->v_addr)?&ccr->addr:NULL),
                              ((ccr->v_subaddr)?&ccr->subaddr:NULL));
        /* new call table entry set default bearer capabilities */
        cmhSAT_fillSetupBC ( cId, MNCC_BEARER_SERV_SPEECH, MNCC_BEARER_SERV_NOT_PRES );

       /* check aoc condition */
        if ((psaCC_ctb(cId)->prio EQ MNCC_PRIO_NORM_CALL) AND
            (aoc_check_moc() EQ FALSE))
          /*
           * check ACM exceeds ACMmax
           * for non-emergency calls
           */
        {
          /* Implements Measure # 77 */
          /* Fill resp_data content in case it is needed for psaSAT_SendTrmResp 
             in cmhSAT_ResCntrlBySIM */
          resp_data.add_content = ADD_NO_CAUSE;
          cmhSAT_ResCntrlBySIM (AT_CMD_D, RSLT_ME_UNAB_PROC, &resp_data);
          psaCC_FreeCtbNtry (cId);
          return( FALSE );
        }

        if( ccr->v_cap_cnf_parms )
        {
          /* check bearer caps, ctb not allocated */
        }


        /* declare call table entry as used and the owner and status
           of the call */
        psaCC_ctb(cId)->calOwn     = (T_OWN)satShrdPrm.SIMCCParm.owner;
        psaCC_ctb(cId)->calStat    = CS_SAT_REQ;
        psaCC_ctb(cId)->curCmd     = AT_CMD_D;
        psaCC_ctb(cId)->curSrc     = satShrdPrm.SIMCCParm.owner;

        if( !ownNotSAT )
          psaCC_ctb(cId)->SATinv = TRUE;

  /* Implements Measure # 174 */
  cmhSAT_SIMCntrlAlertUser (ownNotSAT, cId
#ifdef FF_SAT_E
                            , &addPrm
#endif
                           );

        return( !ownNotSAT );
      }

      /* --- no break, continue with next case --- */
      /*lint -fallthrough*/

    /*
     *------------------------------------------------------------
     * USSD allowed no modification
     *------------------------------------------------------------
     */
    default:
      TRACE_ERROR ("unknown resID in cmhSAT_ResUSSDCntrlBySIM");
      /*lint -fallthrough*/
    case( CCR_ALLW_NO_MDFY ):

      /* check for busy SS condition */
      if( psaSS_stbFindActSrv( NO_ENTRY ) NEQ NO_ENTRY )
      {
        /* respond with "error, ME currently unable to process command" */
        resp_data.add_content = ADD_ME_USSD_BUSY;
        psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
        return( FALSE );
      }

      /* adjust SS control string */
      if( ccr->v_ussd_string )
      {
        memcpy( &(satPndSetup.ussd_str),
                &ccr->ussd_string,
                sizeof(T_ussd_string));
      }

      /********************************************************************************/
      /*
       *-------------------------------------------------------------------
       * check if there is a USSD request pending
       *-------------------------------------------------------------------
       */
      sId = psaSS_stbFindUssdReq();

      if( sId EQ NO_ENTRY )
      {
        /* check if there is another service in progress */
        if( psaSS_stbFindActSrv( sId ) NEQ NO_ENTRY )
        {
          ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_ParallelUSSD );
          return(FALSE);
        }

        /* get new service table entry */
        sId = psaSS_stbNewEntry();
        if( sId EQ NO_ENTRY )
        {
          ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_SrvTabFull );
          return(FALSE);
        }

        CCD_START;

        MALLOC(ussdString, MAX_USSD_STRING);

        /* set data coding scheme */
        /* patch !!!!! CLB 11/12/01      */
        if( (UBYTE)satPndSetup.ussd_str.dcs EQ 0x40 )
        {
          /* 0x40 means basically default alphabet...
          yet some asian networks dont seem to accept it (although using it
          in their own STK !!!) */
          satPndSetup.ussd_str.dcs = 0x0F;
        }
        /*********************************/
        /* Implements Measure 25 */
       /* This function is more correct than utl_getAlphabetCb as it takes care 
          of reserved codings */
       if( cmh_getAlphabetCb( (UBYTE)satPndSetup.ussd_str.dcs ) EQ 0 ) /* 7bit alphabet */
       {
         src_len = (UBYTE)MINIMUM( MAX_USSD_STRING, satPndSetup.ussd_str.c_ussd_str); 
         ussdLen = utl_cvt8To7( satPndSetup.ussd_str.ussd_str,
                             src_len,
                             ussdString, 0 );
         /* According to spec 23.038 section 6.1.2.3 for USSD packing, for bytes end with
          * (8*n)-1 i.e where n is 1,2,3....i.e byte 7, 15, 23 ... to be padded 
          * with carriage return <CR>(0xD) 
          */
         if ((src_len+1)%8 EQ 0)
         {
           ussdString[ussdLen-1] |= (0xD << 1);
         }
       }
       else
       {
         ussdLen = satPndSetup.ussd_str.c_ussd_str;
         memcpy(ussdString, satPndSetup.ussd_str.ussd_str, MAX_USSD_STRING);
       }
       psaSS_asmProcUSSDReq( (UBYTE)satPndSetup.ussd_str.dcs /*dcs*/,
                              ussdString,
                              ussdLen);
//TISH, patch for OMAPS00122397
//patch for get network reject information: "A406020100810101"
//start
        ssShrdPrm.ussdLen= ussdLen;
        TRACE_EVENT_P1("ssShrdPrm.ussdLen = %d", ssShrdPrm.ussdLen);
        memcpy(ssShrdPrm.ussdBuf,ussdString,ussdLen);
        ssShrdPrm.ussdDcs=0x0f;
	  satShrdPrm.SentUSSDid=sId; 
//end


       MFREE(ussdString);

        /* start new transaction */
        ssShrdPrm.stb[sId].ntryUsdFlg = TRUE;

        if (ownNotSAT)   
        {
            ssShrdPrm.stb[sId].curCmd = AT_CMD_CUSD;
            ssShrdPrm.stb[sId].srvOwn = (T_OWN)satShrdPrm.SIMCCParm.owner;
        }
        else
        {
            ssShrdPrm.stb[sId].curCmd = AT_CMD_NONE;
            ssShrdPrm.stb[sId].srvOwn = OWN_SRC_SAT;
            satShrdPrm.SentUSSDid     = sId;
        }
        psaSS_NewTrns(sId);

        if (ownNotSAT)
        {
          R_AT( RAT_OK, (T_ACI_CMD_SRC)satShrdPrm.SIMCCParm.owner )
                  ( AT_CMD_CUSD);

          cmh_logRslt ( (T_ACI_CMD_SRC)satShrdPrm.SIMCCParm.owner,
                        RAT_OK, AT_CMD_CUSD, -1, BS_SPEED_NotPresent, CME_ERR_NotPresent);
        }

        CCD_END;

        retVal = AT_EXCT;
      }
      else
      {
        CCD_START;

        psaSS_asmCnfUSSDReq( (UBYTE)satPndSetup.ussd_str.dcs,
                             satPndSetup.ussd_str.ussd_str,
                             satPndSetup.ussd_str.c_ussd_str );

        ssShrdPrm.stb[sId].ussdReqFlg = FALSE;

        /* continue existing transaction */
        psaSS_CntTrns(sId);

        CCD_END;

        retVal = AT_CMPL;
      }
    /**********************************************************************************/

      if( retVal EQ AT_EXCT )
        return( FALSE );  /* primitive not needed anymore */
      /* Implements Measure # 57 */
      /* Fill resp_data content in case it is needed for psaSAT_SendTrmResp 
         in cmhSAT_ResCntrlBySIM */
      resp_data.resCC = resId;
      cmhSAT_ResCntrlBySIM (AT_CMD_D, RSLT_ME_CAP, &resp_data);

      return( FALSE );  /* primitive not needed anymore */
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SATR                     |
|                            ROUTINE : cmhSAT_ResSMCntrlBySIM       |
+-------------------------------------------------------------------+

  PURPOSE : Handle SIM SM  control result. Return TRUE if primitive
            is needed for building a terminal response.

*/

GLOBAL BOOL cmhSAT_ResSMCntrlBySIM( UBYTE* resId, void *smcRes )
{
  BOOL  ownNotSAT;                      /* flags that the owner is not SAT */
  T_smcr_allw *smcr;                    /* points to SM result */
  T_ACI_CMGF_MOD mode;
  UBYTE sca_buf[MAX_SMS_ADDR_DIG/2 + 2];
  UBYTE da_buf[MAX_SMS_ADDR_DIG/2 + 2];
  UBYTE sca_len = 0;
  UBYTE da_len = 0;
  T_ACI_SAT_TERM_RESP resp_data;
  UBYTE srcId = smsShrdPrm.smsEntStat.entOwn;

  TRACE_FUNCTION ("cmhSAT_ResSMCntrlBySIM()");


  psaSAT_InitTrmResp( &resp_data );

  smcr = (T_smcr_allw*)smcRes;

  /* check if queued event download */
  if (satShrdPrm.event.c_queued)
  {
    cmhSAT_EventDwn((UBYTE)-1, -1, END_UNDEFINED); 
  }

  satShrdPrm.SIMCCParm.busy = FALSE;

  ownNotSAT = satShrdPrm.SIMCCParm.owner NEQ OWN_SRC_SAT;

#ifdef TI_PS_FF_AT_P_CMD_CUST
  /*
  ** If this is a Cust1 MMI and The originator was a STK Cmd
  */
  if ((simShrdPrm.overall_cust_mode EQ (UBYTE)CUST_MODE_BEHAVIOUR_1) AND
      (ownNotSAT EQ FALSE))
  {
      /*
      ** Customised behaviour for the Cust1 MMI
      */
      if (*resId EQ CCR_NOT_ALLW)
      {
            /* send SAT response */
            resp_data.add_content = ADD_CC_NOT_ALLWD;
            psaSAT_SendTrmResp( RSLT_CC_SIM_PRM, &resp_data );
      }
      else
      {
          /*
          ** This was a SAT initiated Request. For the Cust1 the ACI does not yet have any
          ** actions to perform, other than the CC By SIM which has been done. So :
          ** Send the Original STK Cmd to the MMI in a %SATI for the MMI to process.
          **
          ** The second action will cause the MMI to request the approriate Call Setup, SS
          ** or USSD Request and the required Call Table entry will then be re-created.
          */
          cmhSAT_Cust1StkCmdInd();
      }
    
      if ( (sat_mnsms_submit_req->rec_num EQ  SMS_RECORD_NOT_EXIST) OR
           (sat_mnsms_submit_req->modify  NEQ SMS_MODIFY_NON) )
      {
        if (smsShrdPrm.tpdu.tp_submit NEQ NULL)
        {
          ACI_MFREE(smsShrdPrm.tpdu.tp_submit);
          smsShrdPrm.tpdu.tp_submit = NULL;
        }
      }
      PFREE( sat_mnsms_submit_req );

      /*
      ** Ensure that the SMS parameters are reset, so that the SMS Entity is freed to
      ** process the command later.
      */
      smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
      smsShrdPrm.owner = (T_OWN)CMD_SRC_NONE;	  
      smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;

      return( FALSE );  /* primitive not needed anymore */
  }
#endif /* TI_PS_FF_AT_P_CMD_CUST */

  if (smsShrdPrm.smsEntStat.curCmd EQ AT_CMD_CMSS)
  {
    mode = CMGF_MOD_Txt;
  }
#if defined MFW OR defined FF_MMI_RIV OR defined _CONC_TESTING_
  else if (smsShrdPrm.smsEntStat.entOwn EQ CMD_SRC_LCL)
  {
    mode = CMGF_MOD_Txt; /* since text mode (AT+CMGF=1) is never set in MMI */
  }
#endif
  else if (ownNotSAT EQ FALSE)
  {
    mode = CMGF_MOD_Pdu;
  }
  else
  {
    /*
     * request current mode
     */
    qAT_PlusCMGF((T_ACI_CMD_SRC)srcId, &mode);
  }

/*
 *----------------------------------------------------------------
 * determine result id
 *----------------------------------------------------------------
 */
  switch( *resId )
  {
    /*
     *------------------------------------------------------------
     * send message not allowed
     *------------------------------------------------------------
     */
    case( CCR_NOT_ALLW ):
       /* setup initiated by user */
      /* Implements Measure # 57 */ 
      /* Fill resp_data content in case it is needed for psaSAT_SendTrmResp 
         in cmhSAT_ResCntrlBySIM */
      resp_data.add_content = ADD_CC_NOT_ALLWD;
      cmhSAT_ResCntrlBySIM (smsShrdPrm.smsEntStat.curCmd, RSLT_CC_SIM_PRM, &resp_data);
      /* The following has not been taken into as a parameter will have to be 
       * added in all places where it is called for this setting alone */
      if( ownNotSAT )
      {
        smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
        smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;
      }
      if ( (sat_mnsms_submit_req->rec_num EQ  SMS_RECORD_NOT_EXIST) OR
           (sat_mnsms_submit_req->modify  NEQ SMS_MODIFY_NON) )
      {
        if (smsShrdPrm.tpdu.tp_submit NEQ NULL)
        {
          ACI_MFREE(smsShrdPrm.tpdu.tp_submit);
          smsShrdPrm.tpdu.tp_submit = NULL;
        }
      }
      PFREE( sat_mnsms_submit_req );
      return( FALSE );  /* primitive not needed anymore */


    case( CCR_ALLW_WITH_MDFY ):
      if ( smcr->v_sm_addr )
      {
        /*
         * RP Service Center Address
         */

        sat_mnsms_submit_req->modify |= SMS_MODIFY_SCA;

        if (mode EQ CMGF_MOD_Pdu)
        {
          /* PDU mode */
          sca_len = CodeRPAddress( sca_buf,
                                    smcr->sm_addr.c_bcdDigit,
                                    smcr->sm_addr.noa,
                                    smcr->sm_addr.npi,
                                    smcr->sm_addr.bcdDigit );

          cmhSAT_ModifyScaPdu( &sat_mnsms_submit_req->sms_sdu,
                               sca_buf,
                               sca_len );
        }
        else
        {
          /* Text mode */
          smsShrdPrm.tpdu.sc_addr.v_ton = 1;
          smsShrdPrm.tpdu.sc_addr.ton = smcr->sm_addr.noa;
          smsShrdPrm.tpdu.sc_addr.v_npi = 1;
          smsShrdPrm.tpdu.sc_addr.npi = smcr->sm_addr.npi;
          smsShrdPrm.tpdu.sc_addr.c_num = smcr->sm_addr.c_bcdDigit;
          memcpy(smsShrdPrm.tpdu.sc_addr.num, smcr->sm_addr.bcdDigit,
                 smcr->sm_addr.c_bcdDigit);
        }
      }

      if ( smcr->v_sm_addr_2)
      {
        /*
         * TP Destination Address
         */

        sat_mnsms_submit_req->modify |= SMS_MODIFY_TPOA;

        if (mode EQ CMGF_MOD_Pdu)
        {
          /* PDU  mode */
          da_len = CodeTPAddress( da_buf,
                                   smcr->sm_addr_2.c_bcdDigit,
                                   smcr->sm_addr_2.noa,
                                   smcr->sm_addr_2.npi,
                                   smcr->sm_addr_2.bcdDigit );

          cmhSAT_ModifyDaPdu( &sat_mnsms_submit_req->sms_sdu,
                              da_buf,
                              da_len );
        }
        else
        {
          /* Text mode */
          smsShrdPrm.tpdu.tp_submit->tp_da.ton    = smcr->sm_addr_2.noa;
          smsShrdPrm.tpdu.tp_submit->tp_da.npi    = smcr->sm_addr_2.npi;
          smsShrdPrm.tpdu.tp_submit->tp_da.c_num  = smcr->sm_addr_2.c_bcdDigit;
          smsShrdPrm.tpdu.tp_submit->tp_da.digits = smcr->sm_addr_2.c_bcdDigit;
          memcpy(smsShrdPrm.tpdu.tp_submit->tp_da.num, smcr->sm_addr_2.bcdDigit,
                 smcr->sm_addr_2.c_bcdDigit);
        }
      }
      /* no break needed !!!! */
      /*lint -fallthrough*/
    case( CCR_ALLW_NO_MDFY ):
      if( sat_mnsms_submit_req )
      {
        if (mode EQ CMGF_MOD_Txt) /* Text mode */
        {
          /* code sm here */
          cmhSMS_codeMsg (&sat_mnsms_submit_req->sms_sdu,
                          SMS_VT_SUBMIT,
                          &smsShrdPrm.tpdu.sc_addr,
                          SMS_SUBMIT,
                          (UBYTE*)smsShrdPrm.tpdu.tp_submit);

          if (smsShrdPrm.tpdu.tp_submit NEQ NULL)
          {
            ACI_MFREE(smsShrdPrm.tpdu.tp_submit);
            smsShrdPrm.tpdu.tp_submit = NULL;
          }
        }

        PSENDX (SMS, sat_mnsms_submit_req);
      }
      else
        TRACE_EVENT("error provoked by SAT");
      break;

    default:
      if ( (sat_mnsms_submit_req->rec_num EQ SMS_RECORD_NOT_EXIST) OR
           (sat_mnsms_submit_req->modify NEQ SMS_MODIFY_NON) )
      {
        if (smsShrdPrm.tpdu.tp_submit NEQ NULL)
        {
          ACI_MFREE(smsShrdPrm.tpdu.tp_submit);
          smsShrdPrm.tpdu.tp_submit = NULL;
        }
      }
      TRACE_EVENT("wrong type of result received from SIM");
      PFREE( sat_mnsms_submit_req );
      return( FALSE );  /* primitive not needed anymore */
  }

return FALSE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH                          |
|                            ROUTINE : cmhCC_SatDTMFsent            |
+-------------------------------------------------------------------+

  PURPOSE : confirmation for sent SAT DTMF
*/
GLOBAL void cmhCC_SatDTMFsent ( SHORT cId )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
  T_ACI_SAT_TERM_RESP resp_data;
static UBYTE          dtmf_separator_count = 0;

  TRACE_FUNCTION ("cmhCC_SatDTMFsent()");

  psaSAT_InitTrmResp( &resp_data );

  /*
   * Use ctb here because TI compiler 1.22e may have a problem otherwise here.
   * See cmhCC_SndDiscRsn() for the details.
   */  
  if (GET_CAUSE_VALUE(ctb->nrmCs) NEQ NOT_PRESENT_8BIT AND
      ctb->nrmCs                  NEQ MNCC_CAUSE_DTMF_START_SUCCESS)
  {
    TRACE_EVENT_P1("network reported error when sending DTMF: %d", ctb->nrmCs);
    
    psaSAT_SendTrmResp( RSLT_NTW_UNAB_PROC, &resp_data );
    dtmf_separator_count = 0;
    return;
  }

  if(is_digit_dtmf_separator(ccShrdPrm.dtmf.dig[ccShrdPrm.dtmf.cur]))
  {
    if (dtmf_separator_count EQ 0)
    {
      dtmf_separator_count = 1;
      if(ctb->dtmfCmd NEQ AT_CMD_VTS)  /* this is only valid within a number to dial */
       {
         /* p within a number string: this is a 3 seconds pause */
          TRACE_EVENT("DTMF pause requested: 3 seconds");
#if defined (NEW_FRAME)
          TIMERSTART( TDTMF_VALUE, ACI_TDTMF );
#else
          TIMERSTART( TDTMF_VALUE, t_dtmf_handle );
#endif
          ccShrdPrm.dtmf.cur++;  /* skip the DTMF seperator */

          return;
       }
      ccShrdPrm.dtmf.cur++;  /* skip the DTMF seperator */
    }
  }
  else
  {
    dtmf_separator_count = 0;
  }

  if (ccShrdPrm.dtmf.cur < ccShrdPrm.dtmf.cnt)
  {
    cmhSAT_sendDTMF ( NULL );
  }
  else      /* whole DTMF string has been sent */
  {
    ccShrdPrm.dtmf.cId = NO_ENTRY;               /* Reset cId after sending the whole DTMF string */
    ctb->dtmfCmd = AT_CMD_NONE;
    ctb->dtmfSrc = (T_OWN)CMD_SRC_NONE;
    psaSAT_SendTrmResp( RSLT_PERF_SUCCESS, &resp_data );
    dtmf_separator_count = 0;
  }
}

/* Implements Measure # 57,77 */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SATR                     |
|                            ROUTINE : cmhSAT_ResCntrlBySIM         |
+-------------------------------------------------------------------+
  PARAMETERS  : cmdBuf    - AT Command identifier
                simRslt   - Result of SIM operation
                resp_data - Data for terminal response     
  RETURN      : None


  PURPOSE : Handle common portion of sending reponse to SIM call or 
            USSD control result depending on the owner of call setup. 

*/
LOCAL void  cmhSAT_ResCntrlBySIM (T_ACI_AT_CMD        cmdBuf,
                                  UBYTE               simRslt,
                                  T_ACI_SAT_TERM_RESP *resp_data)
{
  BOOL ownNotSAT = (satShrdPrm.SIMCCParm.owner NEQ OWN_SRC_SAT);

  TRACE_FUNCTION ("cmhSAT_ResCntrlBySIM ()");
  if( ownNotSAT )
  {
    R_AT( RAT_CME, (T_ACI_CMD_SRC)satShrdPrm.SIMCCParm.owner )
          ( cmdBuf, CME_ERR_NotPresent );
    /* log result */
    cmh_logRslt ( (T_ACI_CMD_SRC)satShrdPrm.SIMCCParm.owner, RAT_CME,
                  (T_ACI_AT_CMD)cmdBuf,-1, BS_SPEED_NotPresent, CME_ERR_NotPresent );
  }
  /* setup initiated by SAT */
  else
  {
    /* resp_data will be filled in caller */
    psaSAT_SendTrmResp( simRslt, resp_data );
  }
}

/* Implements Measure # 174 */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SATR                     |
|                            ROUTINE : cmhSAT_SIMCntrlAlertUser     |
+-------------------------------------------------------------------+
  PARAMETERS  : ownNotSAT - Boolean flag indicating that the owner is not SAT  
                cId       - Holds setup call id
                addPrm    - additional information for SAT E
  RETURN      : None

  PURPOSE : Handle common portion of Alerting User. 

*/
LOCAL void  cmhSAT_SIMCntrlAlertUser (BOOL            ownNotSAT,
                                      SHORT           cId
#ifdef FF_SAT_E
                                      ,T_ACI_SATA_ADD  *addPrm
#endif
                                      )
{
  UBYTE idx;

  TRACE_FUNCTION ("cmhSAT_SIMCntrlAlertUser ()");

  if( ownNotSAT )
  {
#ifdef FF_SAT_E  
    addPrm->chnType = SATA_CT_VOICE;
    addPrm->chnEst  = SATA_EST_IM;

    R_AT( RAT_SATA,(T_ACI_CMD_SRC) satShrdPrm.SIMCCParm.owner )
      ( cId+1, satShrdPrm.dur, &addPrm );
#else
    R_AT( RAT_SATA, (T_ACI_CMD_SRC)satShrdPrm.SIMCCParm.owner )
      ( cId+1, satShrdPrm.dur);
#endif /* FF_SAT_E */ 

    R_AT( RAT_OK, (T_ACI_CMD_SRC)satShrdPrm.SIMCCParm.owner )
      ( AT_CMD_D );
    /* log result */
    cmh_logRslt ( (T_ACI_CMD_SRC)satShrdPrm.SIMCCParm.owner,
               RAT_OK, AT_CMD_D, -1, BS_SPEED_NotPresent, CME_ERR_NotPresent );
  }
   /* setup initiated by SAT */
  else
  {
    for( idx = 0; idx < CMD_SRC_MAX; idx++ )
    {
#ifdef FF_SAT_E  
      addPrm->chnType = SATA_CT_VOICE;
      addPrm->chnEst  = SATA_EST_IM;
      R_AT( RAT_SATA, (T_ACI_CMD_SRC)idx )
        ( cId+1, satShrdPrm.dur, &addPrm );
#else 
      R_AT( RAT_SATA, (T_ACI_CMD_SRC)idx )
        ( cId+1, satShrdPrm.dur);
#endif /* FF_SAT_E */
    }
  }
}

#endif /* #ifdef SIM_TOOLKIT */
/*==== EOF ========================================================*/
