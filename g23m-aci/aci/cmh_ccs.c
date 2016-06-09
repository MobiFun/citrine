/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_CCS
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
|             protocol stack adapter for call control.
+-----------------------------------------------------------------------------
*/

#ifndef CMH_CCS_C
#define CMH_CCS_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

#include "aci_cmh.h"
#include "ksd.h"

/*==== INCLUDES ===================================================*/

/* for CCD */
#include "ccdapi.h"
#include "aci.h" /*  for CCD_END... */

/***********/

#include "ati_cmd.h"
#include "aci_cmd.h"

#ifdef DTI
#include "dti.h"      /* functionality of the dti library */
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"
#endif

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#ifdef FF_TTY
#include "audio.h"
#endif

#include "aci_mem.h"
#include "l4_tim.h"

#include "phb.h"
#include "psa.h"
#include "psa_cc.h"
#include "psa_sim.h"
#include "psa_mm.h"
#include "psa_ss.h"
#include "psa_util.h"
#include "cmh.h"
#include "cmh_cc.h"
#include "cmh_mm.h"
#include "cmh_sim.h"
#include "cmh_phb.h"

#ifdef FF_ATI
#include "aci_io.h"
#endif /* of #ifdef FF_ATI */

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
#endif

#ifdef DTI
#if defined (FF_WAP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E) 
#include "wap_aci.h"
#include "psa_ppp_w.h"
#include "cmh_ppp.h"
#endif  /* FF_WAP || FF_GPF_TCPIP || FF_SAT_E */
#endif /* DTI */

#ifdef GPRS
#include "gaci.h"
#include "gaci_cmh.h"
#include "psa_sm.h"
#include "cmh_sm.h"
#endif  /* GPRS */

#include "aoc.h"
#include "ksd.h"

#ifdef ACI
#include "cmh_mmi.h"
#endif

#ifdef FF_PSI
#include "psa_psi.h"
#include "cmh_psi.h"
#include "cmh_uart.h"
#include "ati_src_psi.h"
#endif /* FF_PSI */

/*==== CONSTANTS ==================================================*/
#ifdef TI_PS_FF_AT_CMD_P_ECC
//LOCAL char additional_ecc_numbers[ADDITIONAL_ECC_NUMBER_MAX][ADDITIONAL_ECC_NUMBER_LENGTH];
LOCAL char additional_ecc_numbers[ADDITIONAL_ECC_NUMBER_MAX][ADDITIONAL_ECC_NUMBER_LENGTH+1]; //OMAPS00117704/OMAPS00117705
#endif /* TI_PS_FF_AT_CMD_P_ECC */
#ifdef TI_PS_FF_AT_P_CMD_RDLB
GLOBAL T_ACI_CC_REDIAL_BLACKL * cc_blacklist_ptr = NULL;
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
/*==== EXPORT =====================================================*/


#if defined (GPRS) AND defined (DTI)
EXTERN T_ATI_RSLT atGD (char *cl, UBYTE srcId, BOOL *gprs_command);
#endif  /* GPRS */

/*==== VARIABLES ==================================================*/

/* Implements Measure#32: Row 89, 90, 116, 117, 1241 & 1242 */
const char * const ksd_supp_clir_str = "*31#";
const char * const ksd_inv_clir_str = "#31#";

EXTERN T_PCEER causeMod;
EXTERN SHORT causeCeer;
/* parameter block for delayed cmhCC_Dial call */
LOCAL struct cmhCC_Dial_buffer { 
  T_ACI_CMD_SRC srcId;
} cmhCC_Dial_buffer;

#ifdef TI_PS_FF_AT_P_CMD_CUSCFG
EXTERN T_ACI_CUSCFG_PARAMS cuscfgParams;
#endif /* TI_PS_FF_AT_P_CMD_CUSCFG */
/*==== PROTOTYPES =================================================*/

LOCAL UCHAR cmhCC_Dial_delayed (void *arg);
LOCAL T_ACI_RETURN chld_RelHldOrUdub(T_ACI_CMD_SRC srcId);
LOCAL T_ACI_RETURN chld_RelActAndAcpt(T_ACI_CMD_SRC srcId);
LOCAL T_ACI_RETURN chld_RelActSpec(T_ACI_CMD_SRC srcId, CHAR *call);
LOCAL T_ACI_RETURN chld_HldActAndAcpt(T_ACI_CMD_SRC srcId);
LOCAL T_ACI_RETURN chld_HldActExc(T_ACI_CMD_SRC srcId, CHAR *call);
LOCAL T_ACI_RETURN chld_AddHld(T_ACI_CMD_SRC srcId);
LOCAL T_ACI_RETURN chld_Ect(T_ACI_CMD_SRC srcId);
LOCAL T_ACI_RETURN chld_Ccbs(T_ACI_CMD_SRC srcId);
LOCAL T_ACI_RETURN chld_OnlyHold(T_ACI_CMD_SRC srcId);
LOCAL T_ACI_RETURN chld_RelDialCall(T_ACI_CMD_SRC srcId);
LOCAL T_ACI_RETURN chld_RetrieveHoldCall(T_ACI_CMD_SRC srcId);
LOCAL T_ACI_RETURN chld_RetrieveHoldCallSpec(T_ACI_CMD_SRC srcId, CHAR *call);
LOCAL T_ACI_RETURN chld_RelAnySpec(T_ACI_CMD_SRC srcId, CHAR *call);
LOCAL BOOL cmhCC_check_RedialCall         ( T_ACI_AT_CMD at_cmd_id );
LOCAL BOOL cmhCC_check_pending_satCall();
LOCAL BOOL chld_HoldActiveCalls           ( T_ACI_CMD_SRC srcId,
                                            BOOL *mptyHldFlg,
                                            BOOL *hldCalFlg, 
                                            SHORT *cId );
LOCAL T_ACI_RETURN chld_Rel_MultipartySpec( T_ACI_CMD_SRC srcId,
                                            SHORT *spcId,
                                            T_ACI_CHLD_MOD chld_mode,
                                            CHAR *call );
LOCAL void cmhCC_disconnect_waiting_call  ( T_ACI_CMD_SRC srcId,
                                            SHORT waitId,
                                            T_ACI_AT_CMD at_cmd_id,
                                            USHORT *mltyDscFlg );

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_Dm                   |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the D> AT command
            which is responsible to originate a call using the phone-
            book. The searched number can be declared either by the
            name or by giving the index of a specified phonebook. If
            an entry could be found, the ordinary AT D command is
            called.

            <str>     : name string to search.
            <mem>     : type of phonebook.
            <index>   : phonebook index.
            <clirOvrd>: CLIR override
            <cugCtrl> : CUG control
            <callType>: call type
*/

#define RETURN(x) { retVal = x; goto cleanup_exit; }
/*lint -e{801} Use of goto*/
#ifdef NO_ASCIIZ
GLOBAL T_ACI_RETURN sAT_Dm        ( T_ACI_CMD_SRC     srcId,
                                    T_ACI_PB_TEXT     *str,
                                    T_ACI_PB_STOR     mem,
                                    SHORT             index,
                                    T_ACI_D_CLIR_OVRD clirOvrd,
                                    T_ACI_D_CUG_CTRL  cugCtrl,
                                    T_ACI_D_TOC       callType )
#else  /* ifdef NO_ASCIIZ */
GLOBAL T_ACI_RETURN sAT_Dm        ( T_ACI_CMD_SRC     srcId,
                                    CHAR              *str,
                                    T_ACI_PB_STOR     mem,
                                    SHORT             index,
                                    T_ACI_D_CLIR_OVRD clirOvrd,
                                    T_ACI_D_CUG_CTRL  cugCtrl,
                                    T_ACI_D_TOC       callType )
#endif /* ifdef NO_ASCIIZ */
{
  T_CLPTY_PRM *cldPty;        /* holds calling party parameter */
  T_ACI_RETURN retVal;        /* holds function return value */
  T_PHB_RECORD phbNtry;       /* holds phonebook entry */
  T_PHB_TYPE slctPHB;         /* holds selected phonebook */
  T_ACI_PB_TEXT *dial_name;
#ifndef NO_ASCIIZ
  T_ACI_PB_TEXT dialname;
#endif /* #ifndef NO_ASCIIZ */

  ACI_MALLOC (cldPty, sizeof (T_CLPTY_PRM));

  TRACE_FUNCTION ("sAT_Dm()");
/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    RETURN( AT_FAIL )
  }

  /* init cldPty */
  cmhCC_init_cldPty( cldPty );

#ifdef NO_ASCIIZ
  dial_name = str;
#else  /* #ifdef NO_ASCIIZ */
  dial_name = &dialname;

  if ( str NEQ NULL )
  {
    USHORT len = (USHORT)strlen( str );

    cmh_cvtToDefGsm ( str, (CHAR*)dial_name->data, &len );
    dial_name->cs = CS_Sim;
    dial_name->len = (UBYTE)len;
  }
  else
  {
    dial_name->cs = CS_NotPresent;
    dial_name->len = 0;
  }
#endif /* #ifdef NO_ASCIIZ */

/*
 *-------------------------------------------------------------------
 * check for name search
 *-------------------------------------------------------------------
 */
  if( dial_name->len NEQ 0 )
  {
    if( psaCC_phbSrchName( srcId, dial_name, cldPty ) )
      RETURN( cmhCC_Dial( srcId, cldPty, clirOvrd, cugCtrl, callType ))

    else
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_NotFound );
      RETURN( AT_FAIL )
    }
  }

/*
 *-------------------------------------------------------------------
 * check for index read
 *-------------------------------------------------------------------
 */
#ifdef NO_ASCIIZ
  if( ( !str OR !str->len ) AND index NEQ ACI_NumParmNotPresent )
#else  /* #ifdef NO_ASCIIZ */
  if( !str AND index NEQ ACI_NumParmNotPresent )
#endif /* #ifdef NO_ASCIIZ */
  {
    if( !cmhPHB_cvtPhbType ((( mem EQ PB_STOR_NotPresent )?
            cmhPrm[srcId].phbCmdPrm.cmhStor : mem), &slctPHB ))
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      RETURN( AT_FAIL )
    }

#ifdef TI_PS_FFS_PHB
    if (pb_read_record (slctPHB, index, &phbNtry) EQ PHB_OK)
#else
    if( pb_read_phys_record( slctPHB, index, &phbNtry ) EQ PHB_OK )
#endif
    {
      cmhPHB_getAdrStr( cldPty->num, sizeof (cldPty->num) - 1,
                        phbNtry.number, phbNtry.len );

      cmh_demergeTOA ( phbNtry.ton_npi, &cldPty->ton, &cldPty->npi );
      RETURN( cmhCC_Dial( srcId, cldPty, clirOvrd, cugCtrl, callType ))
    }
    else
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_InvIdx );
      RETURN( AT_FAIL )
    }
  }
  RETURN( AT_FAIL )

  /* The one and only exit out of this function to avoid memory leaks */
cleanup_exit:
  ACI_MFREE (cldPty);
  return retVal;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_Dn                   |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the D AT command
            which is responsible to originate a call to a given phone
            number. The D command starts to establish a new call if no
            call is currently active. Otherwise if a call is in active
            state the D command starts to modify the active call. If the
            call mode set by the +CMOD command indicates a single call
            no modification takes place and the D command returns with
            a fail.

            <number>  : phone number.
            <clirOvrd>: CLIR override
            <cugCtrl> : CUG control
            <callType>: call type
*/

GLOBAL T_ACI_RETURN sAT_Dn       ( T_ACI_CMD_SRC srcId,
                                   CHAR * number,
                                   T_ACI_D_CLIR_OVRD clirOvrd,
                                   T_ACI_D_CUG_CTRL  cugCtrl,
                                   T_ACI_D_TOC       callType )
{
/* Implements Measure#32: Row 88  */
  CHAR trace_number_buffer[59];
  T_CLPTY_PRM *cldPty = NULL; /* holds calling party parameter */
  T_ACI_RETURN retVal;        /* holds function return value */
#if defined (GPRS) AND defined (DTI)
  BOOL              gprs_command;
  T_ATI_RSLT        r_value;
#endif  /* GPRS */

  TRACE_FUNCTION ("sAT_Dn()");
  if (number NEQ NULL)
  {
    /* char trcBuf[80]; */
/* Implements Measure#32: Row 88  */
    strncpy (&trace_number_buffer[0], number, 58);
    trace_number_buffer[58] = '\0';

    if (trace_number_buffer[55])
    {
      trace_number_buffer[55] = trace_number_buffer[56] = trace_number_buffer[57] = '.';  /* add trailing "..." if string is >=76 */
    }
    TRACE_EVENT_P1 ("sAT_Dn(): dialled nb=%s",trace_number_buffer);
  }
/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

#if defined (GPRS) AND defined (DTI)
  r_value = atGD( number, (UBYTE) srcId, &gprs_command );
  if ( gprs_command EQ TRUE)
  {
    /* SPR#1983 - SH */
    switch (r_value)
    {
      case ATI_CMPL:
        return AT_CMPL;

      case ATI_EXCT:
        return AT_EXCT;

      default:
        return AT_FAIL;
    }
    /* end SPR#1983 */
  }
#endif  /* GPRS */

  ACI_MALLOC (cldPty, sizeof (T_CLPTY_PRM));

  /* initialize called party parameter */
  if(number NEQ NULL)
  {
    cmhCC_init_cldPty( cldPty );

    cmh_bldCalPrms ( number, cldPty );
  }

/*
 *-------------------------------------------------------------------
 * start dialling
 *-------------------------------------------------------------------
 */
  retVal = cmhCC_Dial( srcId,
                      (number NEQ NULL) ? cldPty : NULL,
                      clirOvrd,
                      cugCtrl,
                      callType );
  ACI_MFREE (cldPty);
  return retVal;
}


/* initialize cldPty structure */
GLOBAL void cmhCC_init_cldPty( /*lint -e(578) */T_CLPTY_PRM *cldPty )
{
  cldPty->num[0] = 0;
  cldPty->ton = MNCC_TON_NOT_PRES;
  cldPty->npi = MNCC_NPI_NOT_PRES;
  cldPty->sub[0] = 0;
  cldPty->tos = MNCC_TOS_NOT_PRES;
  cldPty->oe  = MNCC_OE_EVEN;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : cmhCC_CHLD_Serv          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CHLD AT command
            which is responsible to handle the supplementary services
            witin a call. This function is resonsible for the +CHLD
            and %CHLD command both.
*/
LOCAL T_ACI_RETURN cmhCC_CHLD_Serv(T_ACI_CMD_SRC    srcId,
                                    T_ACI_CHLD_MOD   mode,
                                    CHAR            *call)
{
  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  switch( mode )
  {
    /* release all held calls, or UDUB for CWA */
    case( CHLD_MOD_RelHldOrUdub ):
      return(chld_RelHldOrUdub(srcId));

    /* release all active calls, and accept held or waiting call */
    case( CHLD_MOD_RelActAndAcpt ):
      return(chld_RelActAndAcpt(srcId));

    /* release a specific active call */
    case( CHLD_MOD_RelActSpec ):
      return(chld_RelActSpec(srcId, call));

    /* place all active calls on hold, and accept held or waiting call */
    case( CHLD_MOD_HldActAndAcpt ):
      return(chld_HldActAndAcpt(srcId));

    /* place all active calls on hold except the specified call */
    case( CHLD_MOD_HldActExc ):
      return(chld_HldActExc(srcId, call));

    /* add a held call to the conversation */
    case( CHLD_MOD_AddHld ):
      return(chld_AddHld(srcId));

    /* explicit call transfer */
    case( CHLD_MOD_Ect ):
      return(chld_Ect(srcId));

    /* activate call completion to busy subscriber */
    case( CHLD_MOD_Ccbs ):
      return(chld_Ccbs(srcId));

    /* place a call on hold */
    case(CHLD_MOD_OnlyHold):
      return(chld_OnlyHold(srcId));

    /* release currently dialling call only */
    case(CHLD_MOD_RelDialCall):
      return(chld_RelDialCall(srcId));

    /* retrive first held call on the list */
    case(CHLD_MOD_RetrieveHoldCall):
      return(chld_RetrieveHoldCall(srcId));

    /* retrieve specific held call */
    case(CHLD_MOD_RetrieveHoldCallSpec):
      return(chld_RetrieveHoldCallSpec(srcId, call));

    /* releases specific call */
    case(CHLD_MOD_RelAnySpec):
      return(chld_RelAnySpec(srcId, call));

    }
  /* for unexpected conditions */
  ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
  return( AT_FAIL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : cmhCC_Dial               |
+--------------------------------------------------------------------+

  PURPOSE : Start dialling.

*/

/* helper function for delivering delayed rAT_PlusCPMS callback */
LOCAL UCHAR cmhCC_Dial_delayed (void* arg)
{
  struct cmhCC_Dial_buffer* p = (struct cmhCC_Dial_buffer*) arg;
  TRACE_EVENT("delayed delivery of RAT_NO_CARRIER after cmhCC_Dial");
  R_AT( RAT_NO_CARRIER, p->srcId ) ( AT_CMD_D, 0 );
  return FALSE; /* single-shot */
}

GLOBAL T_ACI_RETURN cmhCC_Dial   ( T_ACI_CMD_SRC     srcId,
                 /*lint -e(578) */ T_CLPTY_PRM      *cldPty,
                                   T_ACI_D_CLIR_OVRD clirOvrd,
                                   T_ACI_D_CUG_CTRL  cugCtrl,
                                   T_ACI_D_TOC       callType )
{
  SHORT cId, newId;           /* holds call id */
  T_CC_CMD_PRM * pCCCmdPrm;   /* points to CC command parameters */
  T_ACI_RETURN   retVal;      /* holds return value */
  UBYTE          prio;        /* holds priority of the call */
  USHORT         dummy=0;
  UBYTE idx;

#if defined SMI OR defined MFW  OR defined FF_MMI_RIV
  T_ACI_CLOG     cmdLog;      /* holds logging info */
#endif

  TRACE_FUNCTION ("cmhCC_Dial()");

  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;

  causeMod  = P_CEER_mod;      /* Clear module which was set for ceer */
  causeCeer = CEER_NotPresent; /* Clear proprietary cause when at point of dialling. */
/*
 *-------------------------------------------------------------------
 * check for an active call
 *-------------------------------------------------------------------
 */

  cId = psaCC_ctbFindCall( OWN_SRC_INV, CS_ACT, NO_VLD_CT );

#ifdef FAX_AND_DATA

  if( cId NEQ NO_ENTRY )
  {

  /*
   *-----------------------------------------------------------------
   * check call mode to modify the call
   *-----------------------------------------------------------------
   */
    if( cmhCC_ChckInCallMdfy( cId, AT_CMD_D ) )
    {
      cmhCC_flagCall( cId, &(pCCCmdPrm->mltyCncFlg));
      psaCC_ctb(cId)->curCmd = AT_CMD_D;
      psaCC_ctb(cId)->curSrc = srcId;
      psaCC_ModifyCall(cId);

#if defined SMI OR defined MFW OR defined FF_MMI_RIV
      /* log command */
      cmdLog.atCmd                = AT_CMD_D;
      cmdLog.cmdType              = CLOG_TYPE_Set;
      cmdLog.retCode              = AT_EXCT;
      cmdLog.cId                  = cId+1;
      cmdLog.sId                  = ACI_NumParmNotPresent;
      cmdLog.cmdPrm.sD.srcId      = srcId;
      cmdLog.cmdPrm.sD.number     = NULL;
      cmdLog.cmdPrm.sD.clirOvrd   = clirOvrd;
      cmdLog.cmdPrm.sD.cugCtrl    = cugCtrl;
      cmdLog.cmdPrm.sD.callType   = callType;
#ifdef SIM_TOOLKIT
      cmdLog.cmdPrm.sD.simCallCtrl= D_SIMCC_NOT_ACTIVE;
#endif /* SIM_TOOLKIT */

      rAT_PercentCLOG( &cmdLog );
#endif

      return( AT_EXCT );
    }
  }

#endif  /* of #ifdef FAX AND_DATA */

  if( cldPty EQ NULL )
  {
    TRACE_EVENT("ERROR: cldPty is NULL");

    if (callType EQ D_TOC_Voice)
      return(AT_CMPL);
    else
    {
      if (!cmh_set_delayed_call (cmhCC_Dial_delayed, &cmhCC_Dial_buffer))
        return AT_BUSY;

      cmhCC_Dial_buffer.srcId = srcId;

      TRACE_EVENT("delayed return requested: 100 ms");
        cmh_start_delayed_call (100);

      return(AT_EXCT);
    }
  }

/*
 *-------------------------------------------------------------------
 * determine call priority
 *-------------------------------------------------------------------
 */
  if (cmhMM_ChkIgnoreECC(cldPty->num))
  {
    prio = MNCC_PRIO_NORM_CALL;
    TRACE_EVENT("cmhCC_Dial(): ECC check is ignored");
  }
  else
  {
    TRACE_EVENT("cmhCC_Dial(): ECC check is coninued");
    prio = psaCC_phbSrchECC ( cldPty->num, TRUE );
  }

/*
 *-------------------------------------------------------------------
 * check fixed dialing phonebook if enabled
 *-------------------------------------------------------------------
 */
  if( prio EQ MNCC_PRIO_NORM_CALL  AND
      (simShrdPrm.crdFun EQ SIM_FDN_ENABLED OR
       simShrdPrm.crdFun EQ SIM_FDN_BDN_ENABLED) )
  {
    if( ksd_isFDNCheckSeq ( cldPty->num ) )
    {
      if(!psaCC_phbNtryFnd( FDN, cldPty ))
      {
        TRACE_EVENT ( "FDN check failed" );
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_FdnCheck );
        return( AT_FAIL );
      }
    }
  }

/*
 *-------------------------------------------------------------------
 * Add Number or Sequence to LDN phonebook
 * This has to be done prior keystrokes to allow adding some special
 * sequences for dialing purpose with prepaid SIMs when roaming (USSD callback)
 *-------------------------------------------------------------------
 */
  if (prio NEQ MNCC_PRIO_EMERG_CALL) /* only normal calls are added to the LDN PB */
  {
    if(ksd_isLDNWriteCheckSeq(cldPty->num)) /* check whether the string needs to be written to LDN?*/
    {
      /* A call table entry does not exist here yet */
      psaCC_phbAddNtry( LDN, NO_ENTRY, CT_MOC, cldPty );  /* add call to LDN PB */
    }
  }

/*
 *-------------------------------------------------------------------
 * check for a keystroke sequence
 *-------------------------------------------------------------------
 */

  if(prio NEQ MNCC_PRIO_EMERG_CALL)
  {
    retVal = cmhCC_chkKeySeq ( srcId,
                               cldPty,
                               &callType,
                               &clirOvrd,
                               CC_SIM_YES );

    if( retVal NEQ AT_CONT )
      return( retVal );
  }
  else if (strlen(cldPty->num) > 4)
  {
    /* decode clirOvrd in case of ECC
       since cmhCC_chkKeySeq is not aware of ECC */

/* Implements Measure#32: Row 89 & 90 */
    if ( !strncmp(cldPty->num,(char*)ksd_supp_clir_str,4) OR /* KSD_SUPPRESS_CLIR */
         !strncmp(cldPty->num,(char*)ksd_inv_clir_str,4) )   /* KSD_INVOKE_CLIR */
    {
      CHAR *p1, *p2;

      if (*cldPty->num EQ '*')
        clirOvrd = D_CLIR_OVRD_Supp;
      else
        clirOvrd = D_CLIR_OVRD_Invoc;
      /* and remove clirOvrd from dial num */
      for (p1=cldPty->num, p2=cldPty->num+4; /*lint -e720 */ *p1++ = *p2++; )
        ;
    }
  }


/*
 *-------------------------------------------------------------------
 * check for a short string sequence: should also be USSD (see 02.30 chptr 4.5.3.2)
 *-------------------------------------------------------------------
 */
  if( ( prio EQ MNCC_PRIO_NORM_CALL ) AND ( !ksd_isBCDForUSBand(cldPty->num) ) )
 {
   retVal = cmhCC_chkShortString (srcId, cId, cldPty);
   if (retVal NEQ AT_EXCT)
   {
     return (retVal);
   }
 }

/*
 *-------------------------------------------------------------------
 * check redialing
 *-------------------------------------------------------------------
 */
  /* Implements Measure 64 */
  cmhCC_CheckRedialTimer( FALSE );
/*
 *-------------------------------------------------------------------
 * continue dialing
 *-------------------------------------------------------------------
 */
  newId = psaCC_ctbNewEntry();

  if( newId EQ NO_ENTRY )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_CallTabFull );
    return( AT_FAIL );        /* call table full */
  }

  if( cId NEQ NO_ENTRY )
  {

  /*
   *-----------------------------------------------------------------
   * check if there is already a call on hold
   *-----------------------------------------------------------------
   */
    if( psaCC_ctbFindCall( OWN_SRC_INV, CS_HLD, NO_VLD_CT ) NEQ NO_ENTRY )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_OneCallOnHold );
      psaCC_FreeCtbNtry (newId);
      return( AT_FAIL );
    }

  /*
   *-----------------------------------------------------------------
   * check to put active call on hold, if it is no data call and not
   * an emergency call
   *-----------------------------------------------------------------
   */
    if( psaCC_ctb(cId)->prio EQ MNCC_PRIO_NORM_CALL AND
        cmhCC_getcalltype(cId)  EQ VOICE_CALL )
    {
      if( pCCCmdPrm->mltyCncFlg NEQ 0 )
      {
        psaCC_FreeCtbNtry (newId);
        return( AT_BUSY );
      }
    }
    else
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_CallTypeNoHold );
      psaCC_FreeCtbNtry (newId);
      return( AT_FAIL );
    }
  }

/*
 *-------------------------------------------------------------------
 * setup call parameters
 *-------------------------------------------------------------------
 */
  cmhCC_chkDTMFDig ( cldPty->num, newId, dummy, TRUE );
  

  retVal = cmhCC_fillSetupPrm ( newId, srcId, cldPty, NULL, prio,
                                clirOvrd, cugCtrl, callType );
  if( retVal NEQ AT_CMPL  )
  {
    psaCC_FreeCtbNtry (newId);
    return( retVal );
  }

#ifdef TI_PS_FF_AT_P_CMD_RDLB
/*
 *-------------------------------------------------------------------
 * check number against black list entries
 *-------------------------------------------------------------------
 */
  if(rdlPrm.rdlMod EQ AUTOM_REPEAT_ON)
  {
    if(cmhCC_redialChkBlackl(newId) NEQ AT_CMPL)
    {
      psaCC_FreeCtbNtry (newId);
      return( AT_FAIL);
    }
  }
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
/*
 *-----------------------------------------------------------------
 * declare call table entry as used and the owner of the call
 *-----------------------------------------------------------------
 */
  psaCC_ctb(newId)->calType    = CT_MOC;
  psaCC_ctb(newId)->calOwn     = (T_OWN)srcId;
  psaCC_ctb(newId)->curCmd     = AT_CMD_D;
  psaCC_ctb(newId)->curSrc     = srcId;

/*
 *-------------------------------------------------------------------
 * check for call control by SIM toolkit
 *-------------------------------------------------------------------
 */
#ifdef SIM_TOOLKIT

  if ( prio NEQ MNCC_PRIO_EMERG_CALL ) /* don't send emergency calls to */
  {                               /* SIM toolkit                   */
    if ( !ksd_isFDNCheckSeq ( cldPty-> num ) )
      ;    /* bypass *#06# from CntrlBySIM */
    else
    {
      if( psaSIM_ChkSIMSrvSup( SRV_CalCntrl ) AND satShrdPrm.SIMCCParm.busy NEQ TRUE )
      {
        /*  get bearer capability parameter */ 
        ccShrdPrm.BC0_send_flag =  ccShrdPrm.ctb[newId]->BC[0].bearer_serv NEQ MNCC_BEARER_SERV_NOT_PRES   AND
                         ccShrdPrm.ctb[newId]->BC[0].bearer_serv NEQ MNCC_BEARER_SERV_SPEECH      AND
                         ccShrdPrm.ctb[newId]->BC[0].bearer_serv NEQ MNCC_BEARER_SERV_AUX_SPEECH;
        
        ccShrdPrm.BC1_send_flag =  ccShrdPrm.ctb[newId]->BC[1].bearer_serv NEQ MNCC_BEARER_SERV_NOT_PRES   AND
                         ccShrdPrm.ctb[newId]->BC[1].bearer_serv NEQ MNCC_BEARER_SERV_SPEECH      AND
                         ccShrdPrm.ctb[newId]->BC[1].bearer_serv NEQ MNCC_BEARER_SERV_AUX_SPEECH;
        if ( (callType EQ D_TOC_Data) AND ( ccShrdPrm.BC0_send_flag OR ccShrdPrm.BC1_send_flag) )
        {
          satShrdPrm.capParm.cId = newId;
          psaCC_BCapCode( BCRI_SAT, newId );
          return (AT_EXCT);
        }
        else
        {
          retVal = cmhSAT_CalCntrlBySIM( newId );
        }

        if( retVal NEQ AT_CMPL )
        {
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
          cmdLog.atCmd                = AT_CMD_D;
          cmdLog.cmdType              = CLOG_TYPE_Set;
          cmdLog.retCode              = AT_EXCT;
          cmdLog.cId                  = newId+1;
          cmdLog.sId                  = ACI_NumParmNotPresent;
          cmdLog.cmdPrm.sD.srcId      = srcId;
          cmdLog.cmdPrm.sD.number     = cldPty->num;
          cmdLog.cmdPrm.sD.clirOvrd   = clirOvrd;
          cmdLog.cmdPrm.sD.cugCtrl    = cugCtrl;
          cmdLog.cmdPrm.sD.callType   = callType;
          cmdLog.cmdPrm.sD.simCallCtrl= D_SIMCC_ACTIVE_CHECK;

          rAT_PercentCLOG( &cmdLog );
#endif
          return( retVal );
        }
      }
    }
  }

#endif


  if( cId NEQ NO_ENTRY )
  {
    /* put an active call on hold prior to set up the new call */
    CHLDaddInfo = CHLD_ADD_INFO_DIAL_CAL;
    pCCCmdPrm -> CHLDmode = CHLD_MOD_HldActDial;

    cmhCC_HoldCall(cId, srcId, AT_CMD_D);

    /* flag newId to start the call once active call has been held */
    cmhCC_flagCall( newId, &(pCCCmdPrm->mltyCncFlg));
  }
  else
  {
    /* start a new call */
    cmhCC_flagCall( newId, &(pCCCmdPrm->mltyCncFlg));

    psaCC_NewCall(newId);
    /* call progress information */
    for(idx = 0; idx < CMD_SRC_MAX; idx++ )
    {
      R_AT(RAT_CPI, (T_ACI_CMD_SRC)idx)( newId+1,
      CPI_MSG_MO_Setup,
      (psaCC_ctb(newId)->inBndTns)? CPI_IBT_True: CPI_IBT_False,
      (ccShrdPrm.TCHasg)? CPI_TCH_True: CPI_TCH_False,
      psaCC_ctb(newId)->curCs);
    }  
  }

/* log command */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  cmdLog.atCmd                = AT_CMD_D;
  cmdLog.cmdType              = CLOG_TYPE_Set;
  cmdLog.retCode              = AT_EXCT;
  cmdLog.cId                  = newId+1;
  cmdLog.sId                  = ACI_NumParmNotPresent;
  cmdLog.cmdPrm.sD.srcId      = srcId;
  cmdLog.cmdPrm.sD.number     = cldPty->num;
  cmdLog.cmdPrm.sD.clirOvrd   = clirOvrd;
  cmdLog.cmdPrm.sD.cugCtrl    = cugCtrl;
  cmdLog.cmdPrm.sD.callType   = callType;
#ifdef SIM_TOOLKIT
  cmdLog.cmdPrm.sD.simCallCtrl= D_SIMCC_NOT_ACTIVE;
#endif /* SIM_TOOLKIT */

  rAT_PercentCLOG( &cmdLog );

#endif

  return( AT_EXCT );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_H                    |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the H AT command
            which is responsible to terminate or modify a call.

            1. The function searches for an active call. If this
            call is modifiable, it will be modified and the function
            returns.
            2. Then the function searches for a call that is in the
            disconnect request state and indicates CCBS possibility,
            this call will be finally released.
            3. If enabled the next search looks for a pending SAT call.
            If such a call is found the call is rejected and the SIM card
            will be informed.
            4. The next step is to clear all calls of the call table
            exept a waiting call. The function returns if at least one
            call could be found.
            5. If no call was found, the function searches for a waiting
            call. If a waiting call was found, the call will be
            terminated declaring the user as busy.
            6. If none of the above situations match, the function
            returns with a fail.

*/

GLOBAL T_ACI_RETURN sAT_H ( T_ACI_CMD_SRC srcId )
{
  SHORT cId;                  /* holds call id */
  SHORT waitId = NO_ENTRY;    /* holds call waiting id */
  T_CC_CMD_PRM * pCCCmdPrm;   /* points to CC command parameters */
#if defined (GPRS) AND defined (DTI)
  T_ACI_RETURN  ret_value;
#endif  /* GPRS */

#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  T_ACI_CLOG     cmdLog;      /* holds logging info */
#endif

  TRACE_FUNCTION ("sAT_H()");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;


#if defined (GPRS) AND defined (DTI)
  /*  handle command for GPRS */
  if ( TRUE EQ cmhSM_sAT_H( srcId, &ret_value ) )
    return ret_value;
#endif  /* GPRS */

/*  prepare log command */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  cmdLog.atCmd                = AT_CMD_H;
  cmdLog.cmdType              = CLOG_TYPE_Set;
  cmdLog.retCode              = AT_EXCT;
  cmdLog.sId                  = ACI_NumParmNotPresent;
  cmdLog.cmdPrm.sH.srcId      = srcId;
#endif

#ifdef ACI /* for ATI only version */
  cmhMMI_handleAudioTone ( AT_CMD_H, RAT_OK, CPI_MSG_NotPresent );
#endif

/*
 *-------------------------------------------------------------------
 * find an active call and check call mode in case of
 * in-call modification
 *-------------------------------------------------------------------
 */
#if defined (FAX_AND_DATA) AND defined (DTI)
  cId = psaCC_ctbFindCall( OWN_SRC_INV, CS_ACT, NO_VLD_CT );

  if( cId NEQ NO_ENTRY AND
      cmhCC_ChckInCallMdfy( cId, AT_CMD_H ) )
  {
    switch( cmhCC_getcalltype(cId) )
    {
      case( TRANS_CALL ):
        ccShrdPrm.datStat = DS_MDF_REQ;
        cmhTRA_Deactivate();

#if defined SMI OR defined MFW OR defined FF_MMI_RIV
        cmdLog.cId = cId+1;
        rAT_PercentCLOG( &cmdLog );
#endif
        return( AT_EXCT );

      case( NON_TRANS_CALL ):
        ccShrdPrm.datStat = DS_MDF_REQ;
        cmhL2R_Deactivate();

#if defined SMI OR defined MFW OR defined FF_MMI_RIV
        cmdLog.cId = cId+1;
        rAT_PercentCLOG( &cmdLog );
#endif
        return( AT_EXCT );

#ifdef FF_FAX
      case( FAX_CALL ):
        ccShrdPrm.datStat = DS_MDF_REQ;
        cmhT30_Deactivate ();

#if defined SMI OR defined MFW OR defined FF_MMI_RIV
        cmdLog.cId = cId+1;
        rAT_PercentCLOG( &cmdLog );
#endif
        return( AT_EXCT );
#endif /* FF_FAX */
    }
  }
#endif /* FAX_AND_DATA */

  /* 
   * 3GPP standard 27007 
   * (+CVHU)
   *
   * FreeCalypso note: +CVHU support was not present in the TCS211 version
   * of ACI, but it appears in the version we got from the LoCosto source.
   * The following stanza is a new addition with this LoCosto version,
   * and it was broken for our sans-PSI configuration: the reference to
   * to psiShrdPrm only works when PSI is there, and it is a compilation
   * failure otherwise.  The conditional on FF_PSI and the #else version
   * have been added by Space Falcon; the correctness of the latter is
   * not yet known.
   */
#ifdef FF_PSI
  if( ccShrdPrm.cvhu EQ CVHU_DropDTR_ATH_IGNORED OR
    ((psiShrdPrm.dtr_clearcall EQ TRUE) AND 
     (ccShrdPrm.cvhu EQ CVHU_DropDTR_IGNORED)) )
  {
    psiShrdPrm.dtr_clearcall = FALSE;  
    return (AT_CMPL);
  }
#else
  if( ccShrdPrm.cvhu EQ CVHU_DropDTR_ATH_IGNORED )
    return (AT_CMPL);
#endif

/*
 *-------------------------------------------------------------------
 * check for a call with CCBS possible
 *-------------------------------------------------------------------
 */
  if( pCCCmdPrm -> mltyDscFlg NEQ 0 )
    return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 * check for a redial call
 *-------------------------------------------------------------------
 */
  /* Implements Measure 80 */
  if (cmhCC_check_RedialCall(AT_CMD_H) EQ TRUE )
  {
    return( AT_CMPL );
  }

  cId = psaCC_ctbFindCall( OWN_SRC_INV, CS_ACT_REQ, CT_MOC );

  if (cId EQ NO_ENTRY)
    cId = psaCC_ctbFindCall( OWN_SRC_INV, CS_DSC_REQ, CT_MOC );    /* also search for calls in disconnect state */

  if( cId NEQ NO_ENTRY )
  {
    if( psaCC_ctb(cId)->CCBSstat EQ CCBSS_PSSBL )
    {
      pCCCmdPrm -> mltyDscFlg = 0;

      cmhCC_flagCall( cId, &(pCCCmdPrm -> mltyDscFlg));
      psaCC_ctb(cId)->nrmCs  = MNCC_CAUSE_CALL_CLEAR;
      psaCC_ctb(cId)->curCmd = AT_CMD_H;
      psaCC_ctb(cId)->curSrc = srcId;
      psaCC_ClearCall (cId);

#if defined SMI OR defined MFW OR defined FF_MMI_RIV
      cmdLog.cId = cId+1;
      rAT_PercentCLOG( &cmdLog );
#endif
      return( AT_EXCT );
    }
  }

/*
 *-------------------------------------------------------------------
 * check for a CCBS recall
 *-------------------------------------------------------------------
 */
  cId = psaCC_ctbFindCall( (T_OWN)CMD_SRC_NONE, CS_ACT_REQ, CT_NI_MOC );

  if( cId NEQ NO_ENTRY )
  {
    psaCC_ctb(cId)->nrmCs  = MNCC_CAUSE_CALL_REJECT;
    psaCC_ClearCall (cId);

#if defined SMI OR defined MFW OR defined FF_MMI_RIV
    cmdLog.cId = cId+1;
    cmdLog.retCode = AT_CMPL;
    rAT_PercentCLOG( &cmdLog );
#endif

    psaCC_FreeCtbNtry (cId);
    return( AT_CMPL );
  }

/*
 *-------------------------------------------------------------------
 * check for a pending SAT call
 *-------------------------------------------------------------------
 */
#ifdef SIM_TOOLKIT
  if (cmhCC_check_pending_satCall() EQ TRUE )
  {
    return( AT_CMPL );
  }
#endif /* SIM_TOOLKIT */
 
/*
 *-------------------------------------------------------------------
 * clear all calls except a waiting call
 *-------------------------------------------------------------------
 */
  pCCCmdPrm -> mltyDscFlg = 0;

  for( cId = 0; cId < MAX_CALL_NR; cId++ )
  {
    cmhCC_ClearCall( cId, MNCC_CAUSE_CALL_CLEAR, srcId, AT_CMD_H, &waitId );
  }

  if( pCCCmdPrm -> mltyDscFlg )
  {
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
    cmdLog.cId = cId+1;
    rAT_PercentCLOG( &cmdLog );
#endif
    return( AT_EXCT );
  }

/*
 *-------------------------------------------------------------------
 * disconnect a waiting call with user determined user busy
 *-------------------------------------------------------------------
 */
  /* Implements Measure 164 */
  if ( waitId NEQ NO_ENTRY )
  {
    cmhCC_disconnect_waiting_call ( srcId, waitId, AT_CMD_H,
                                    &(pCCCmdPrm -> mltyDscFlg) );
    return( AT_EXCT );
  }

/*
 *-------------------------------------------------------------------
 * nothing to act on
 *-------------------------------------------------------------------
 */
  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_PlusCHUP             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CHUP AT command
            which is responsible to terminate a call.

            1. The next step is to clear all calls of the call table
            exept a waiting call. The function returns if at least one
            call could be found.
            2. If no call was found, the function searches for a waiting
            call. If a waiting call was found, the call will be
            terminated declaring the user as busy.
            3. If none of the above situations match, the function
            returns with a fail.

*/

GLOBAL T_ACI_RETURN sAT_PlusCHUP ( T_ACI_CMD_SRC srcId )
{
  SHORT cId;                  /* holds call id */
  SHORT waitId = NO_ENTRY;    /* holds call waiting id */
  T_CC_CMD_PRM * pCCCmdPrm;   /* points to CC command parameters */

#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  T_ACI_CLOG     cmdLog;      /* holds logging info */
#endif
  TRACE_FUNCTION ("sAT_CHUP()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;

/*
 *-------------------------------------------------------------------
 * prepare log command
 *-------------------------------------------------------------------
 */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  cmdLog.atCmd                = AT_CMD_CHUP;
  cmdLog.cmdType              = CLOG_TYPE_Set;
  cmdLog.retCode              = AT_EXCT;
  cmdLog.sId                  = ACI_NumParmNotPresent;
  cmdLog.cmdPrm.sCHUP.srcId   = srcId;
#endif
/*
 *-------------------------------------------------------------------
 * check for a redial call
 *-------------------------------------------------------------------
 */
  /* Implements Measure 80 */
  if (cmhCC_check_RedialCall(AT_CMD_CHUP) EQ TRUE )
  {
    return( AT_CMPL );
  }

/*
 *-------------------------------------------------------------------
 * check for a pending SAT call
 *-------------------------------------------------------------------
 */
#ifdef SIM_TOOLKIT
  if (cmhCC_check_pending_satCall() EQ TRUE )
  {
    return( AT_CMPL );
  }
#endif /* SIM_TOOLKIT */
 
/*
 *-------------------------------------------------------------------
 * clear only active call
 *-------------------------------------------------------------------
 */
  if( pCCCmdPrm -> mltyDscFlg NEQ 0 )
    return( AT_BUSY );

  cId = psaCC_ctbFindCall( OWN_SRC_INV, CS_ACT, NO_VLD_CT);

  if( cId NEQ NO_ENTRY )
  {
    cmhCC_ClearCall(cId, MNCC_CAUSE_CALL_CLEAR, srcId, AT_CMD_CHUP, &waitId );
  }

  if( pCCCmdPrm -> mltyDscFlg )
  {
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
    cmdLog.cId = cId+1;
    rAT_PercentCLOG( &cmdLog );
#endif
    return( AT_EXCT );
  }

/*
 *-------------------------------------------------------------------
 * disconnect a waiting call with user determined user busy
 *-------------------------------------------------------------------
 */
  /* Implements Measure 164 */
  if ( waitId NEQ NO_ENTRY )
  {
    cmhCC_disconnect_waiting_call ( srcId, waitId, AT_CMD_CHUP,
                                    &(pCCCmdPrm -> mltyDscFlg) );
    return( AT_EXCT );
  }

/*
 *-------------------------------------------------------------------
 * nothing to act on
 *-------------------------------------------------------------------
 */
  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_A                    |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the A AT command
            which is responsible to accept an incoming call. The A
            command accepts an incoming call if no call is currently
            active. Otherwise if a call is in active state the A command
            starts to modify the active call. If the call mode set by
            the +CMOD command indicates a single call no modification
            takes place and the A command returns with a fail.

*/

GLOBAL T_ACI_RETURN sAT_A ( T_ACI_CMD_SRC srcId )
{
  SHORT cId;                /* holds call id */
  T_CC_CMD_PRM * pCCCmdPrm; /* points to CC command parameters */

#if defined (GPRS) AND defined (DTI)
  T_ACI_RETURN  ret_value;
#endif  /* GPRS */

#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  T_ACI_CLOG     cmdLog;      /* holds logging info */
#endif

  TRACE_FUNCTION ("sAT_A()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;

/*
 *-------------------------------------------------------------------
 * handle command for GPRS
 *-------------------------------------------------------------------
 */
#if defined (GPRS) AND defined (DTI)

  if ( TRUE EQ cmhSM_sAT_A( srcId, &ret_value ) )
    return ret_value;

#endif  /* GPRS */

/*
 *-------------------------------------------------------------------
 * prepare log command
 *-------------------------------------------------------------------
 */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  cmdLog.atCmd                = AT_CMD_A;
  cmdLog.cmdType              = CLOG_TYPE_Set;
  cmdLog.retCode              = AT_EXCT;
  cmdLog.sId                  = ACI_NumParmNotPresent;
  cmdLog.cmdPrm.sA.srcId      = srcId;
#endif

/*
 *-------------------------------------------------------------------
 * check for an active call
 *-------------------------------------------------------------------
 */
#ifdef FAX_AND_DATA

  cId = psaCC_ctbFindCall( OWN_SRC_INV, CS_ACT, NO_VLD_CT );

  if( cId NEQ NO_ENTRY )
  {

  /*
   *-----------------------------------------------------------------
   * check call mode to modify the call
   *-----------------------------------------------------------------
   */
    if( cmhCC_ChckInCallMdfy( cId, AT_CMD_A ) )
    {
      cmhCC_flagCall( cId, &(pCCCmdPrm->mltyCncFlg));
      psaCC_ctb(cId)->curCmd = AT_CMD_A;
      psaCC_ctb(cId)->curSrc = srcId;
      psaCC_ModifyCall(cId);

#if defined SMI OR defined MFW OR defined FF_MMI_RIV
      cmdLog.cId = cId+1;
      rAT_PercentCLOG( &cmdLog );
#endif
      return( AT_EXCT );
    }
  /*
   *----------------------------------------------------------------------------------
   * ATA can be used to answer the second SAT call when the first call is active
   *----------------------------------------------------------------------------------
   */
#ifdef SIM_TOOLKIT
    cId = psaCC_ctbFindCall( OWN_SRC_INV, CS_SAT_REQ, NO_VLD_CT );
#ifdef FF_SAT_E
    if( cId EQ NO_ENTRY )
    {
      cId = psaCC_ctbFindCall( OWN_SRC_INV, CS_SAT_CSD_REQ, NO_VLD_CT );
    }
#endif /* FF_SAT_E */
    if( cId EQ NO_ENTRY )
    {
      TRACE_FUNCTION ("Already a call is active !!!"); 
      return( AT_FAIL);
    }
#else
   /* Added Below Two lines to fix ACI-SPR-22325*/
   TRACE_FUNCTION ("Already a call is active !!!"); 
   return( AT_FAIL );
#endif
  }
#endif    /* of #ifdef FAX_AND_DATA */


/*
 *-------------------------------------------------------------------
 * check for an incoming call to accept
 *-------------------------------------------------------------------
 */
  if( pCCCmdPrm -> mltyCncFlg NEQ 0 )

    return( AT_BUSY );

  cId = psaCC_ctbFindCall( (T_OWN)CMD_SRC_NONE, CS_ACT_REQ, CT_MTC );

  if( cId NEQ NO_ENTRY )
  {
    /* accept the call */
    cmhCC_flagCall( cId, &(pCCCmdPrm->mltyCncFlg));

    cmhCC_AcceptCall(cId, srcId, AT_CMD_A);

#if defined SMI OR defined MFW OR defined FF_MMI_RIV
    cmdLog.cId = cId+1;
    rAT_PercentCLOG( &cmdLog );
#endif
    return( AT_EXCT );
  }

/*
 *-------------------------------------------------------------------
 * check for a CCBS recall condition
 *-------------------------------------------------------------------
 */
  cId = psaCC_ctbFindCall( (T_OWN)CMD_SRC_NONE, CS_ACT_REQ, CT_NI_MOC );

  if( cId NEQ NO_ENTRY )
  {
    /* accept the call */
    {
      cmhCC_flagCall( cId, &(pCCCmdPrm->mltyCncFlg));

      cmhCC_NewCall(cId, srcId, AT_CMD_A);

#if defined SMI OR defined MFW OR defined FF_MMI_RIV
      cmdLog.cId = cId+1;
      rAT_PercentCLOG( &cmdLog );
#endif
      return( AT_EXCT );
    }
  }

/*
 *-------------------------------------------------------------------
 * check for a pending SAT call
 *-------------------------------------------------------------------
 */
#ifdef SIM_TOOLKIT
  /* triggered by SETUP CALL command */
    cId = psaCC_ctbFindCall( OWN_SRC_INV, CS_SAT_REQ, NO_VLD_CT );

#ifdef FF_SAT_E
  if( cId EQ NO_ENTRY )
    /* triggered by OPEN CHANNEL command */
    cId = psaCC_ctbFindCall( OWN_SRC_INV, CS_SAT_CSD_REQ, NO_VLD_CT );
#endif /* FF_SAT_E */

  if( cId NEQ NO_ENTRY )
  {
    if( psaCC_ctb(cId)->SATinv )
    {
      if( !cmhSAT_UserAcptCall( cId, (UBYTE)srcId ) )
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_CmdDetailsSAT );
        return( AT_FAIL );
      }
    }
    else
    {
      cmhCC_flagCall( cId, &(pCCCmdPrm->mltyCncFlg));
      cmhCC_NewCall(cId, srcId, AT_CMD_A);
    }

#if defined SMI OR defined MFW OR defined FF_MMI_RIV
    cmdLog.cId = cId+1;
    rAT_PercentCLOG( &cmdLog );
#endif

    return( AT_EXCT );
  }
#if defined (GPRS) AND defined (FF_SAT_E) AND defined (DTI)
  else
  {
    /* check for a pending SAT GPRS channel */
    if( cmhSAT_OpChnGPRSPend(PDP_CONTEXT_CID_INVALID, OPCH_WAIT_CNF))
    {
      if( !cmhSAT_UserAcptCntxt( (UBYTE)srcId ) )
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_CmdDetailsSAT );
        return( AT_FAIL );
      }
      return( AT_EXCT );
    }
  }
#endif  /* GPRS AND FF_SAT_E */
#endif  /* SIM_TOOLKIT */


/*
 *-------------------------------------------------------------------
 * call not found
 *-------------------------------------------------------------------
 */
  ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_CallNotFound );
  return( AT_FAIL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_PlusCLIR             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CLIR AT command
            which is responsible to enable or disable the presentation
            of the own calling id for mobile originated calls.

*/

GLOBAL T_ACI_RETURN sAT_PlusCLIR ( T_ACI_CMD_SRC srcId,
                                   T_ACI_CLIR_MOD mode )
{
  T_CC_CMD_PRM * pCCCmdPrm;   /* points to CC command parameters */

  TRACE_FUNCTION ("sAT_PlusCLIR()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;

/*
 *-------------------------------------------------------------------
 * process the mode parameter
 *-------------------------------------------------------------------
 */
  switch( mode )
  {
    case( CLIR_MOD_NotPresent ):
      break;

    case( CLIR_MOD_Subscript ):
    case( CLIR_MOD_Invoc     ):
    case( CLIR_MOD_Supp      ):
      pCCCmdPrm -> CLIRmode = mode;
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_PlusCMOD             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CMOD AT command
            which is responsible to set the mode for mobile originated
            calls.

*/

GLOBAL T_ACI_RETURN sAT_PlusCMOD ( T_ACI_CMD_SRC srcId,
                                   T_ACI_CMOD_MOD mode )
{
  TRACE_FUNCTION ("sAT_PlusCMOD()");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  /* process the mode parameter */
  switch( mode )
  {
    case( CMOD_MOD_NotPresent ):
      break;

    case( CMOD_MOD_Single       ):

#ifdef FAX_AND_DATA
    case( CMOD_MOD_VoiceFax     ):
    case( CMOD_MOD_VoiceDat     ):
    case( CMOD_MOD_VoiceFlwdDat ):
#endif    /* of #ifdef FAX_AND_DATA */

      ccShrdPrm.CMODmode = mode;
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

  return( AT_CMPL );
}

#ifdef FAX_AND_DATA
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_PlusCBST             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CBST AT command
            which is responsible to set the bearer service parameters
            for following mobile originated calls.

*/

GLOBAL T_ACI_RETURN sAT_PlusCBST ( T_ACI_CMD_SRC srcId,
                                   T_ACI_BS_SPEED speed,
                                   T_ACI_CBST_NAM name,
                                   T_ACI_CBST_CE ce)
{
  TRACE_FUNCTION ("sAT_PlusCBST()");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  /* process the speed parameter */
  switch( speed )
  {
    case( BS_SPEED_NotPresent ):
      speed = ccShrdPrm.CBSTspeed;
      break;

    case( BS_SPEED_AUTO         ):
    case( BS_SPEED_300_V21      ):
    case( BS_SPEED_1200_V22     ):
    case( BS_SPEED_1200_75_V23  ):
    case( BS_SPEED_2400_V22bis  ):
    case( BS_SPEED_2400_V26ter  ):
    case( BS_SPEED_4800_V32     ):
    case( BS_SPEED_9600_V32     ):
    case( BS_SPEED_9600_V34     ):
    case( BS_SPEED_14400_V34    ):
/*  case( BS_SPEED_1200_V120    ): This layer 1 protocol is not supported
    case( BS_SPEED_2400_V120    ):
    case( BS_SPEED_4800_V120    ):
    case( BS_SPEED_9600_V120    ):
    case( BS_SPEED_14400_V120   ): */
    case( BS_SPEED_300_V110     ):
    case( BS_SPEED_1200_V110    ):
    case( BS_SPEED_2400_V110    ):
    case( BS_SPEED_4800_V110    ):
    case( BS_SPEED_9600_V110    ):
    case( BS_SPEED_14400_V110   ):
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

  /* process the name parameter */
  switch( name )
  {
    case( CBST_NAM_NotPresent ):
      name = ccShrdPrm.CBSTname;
      break;

    case( CBST_NAM_Asynch ):
    case( CBST_NAM_Synch  ):
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

  /* process the ce parameter */
  switch( ce )
  {
    case( CBST_CE_NotPresent ):
      ce = ccShrdPrm.CBSTce;
      break;

    case( CBST_CE_Transparent      ):
    case( CBST_CE_NonTransparent   ):
    case( CBST_CE_BothTransPref    ):
    case( CBST_CE_BothNonTransPref ):
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

  /* assign the parameters */
  ccShrdPrm.CBSTspeed = speed;
  ccShrdPrm.CBSTname  = name;
  ccShrdPrm.CBSTce    = ce;

  /* update CC setting for MTC */
  psaCC_Config();
  return( AT_CMPL );

}
#endif /* FAX_AND_DATA */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_PlusCSTA             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CSTA AT command
            which is responsible to set the type of address for further
            dialing commands.

*/

GLOBAL T_ACI_RETURN sAT_PlusCSTA ( T_ACI_CMD_SRC srcId,
                                   T_ACI_TOA * toa )
{
  T_CC_CMD_PRM * pCCCmdPrm; /* points to CC command parameters */

  TRACE_FUNCTION ("sAT_PlusCSTA()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;

/*
 *-------------------------------------------------------------------
 * process the toa parameter
 *-------------------------------------------------------------------
 */
  if( ! toa ) return( AT_CMPL );

  switch( toa -> ton )
  {
    case( TON_Unknown       ):
    case( TON_International ):
    case( TON_National      ):
    case( TON_NetSpecific   ):
    case( TON_DedAccess     ):
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

  switch( toa -> npi )
  {
    case( NPI_Unknown       ):
    case( NPI_IsdnTelephony ):
    case( NPI_Data          ):
    case( NPI_Telex         ):
    case( NPI_National      ):
    case( NPI_Private       ):
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

  pCCCmdPrm -> CSTAtoa.ton = toa -> ton;
  pCCCmdPrm -> CSTAtoa.npi = toa -> npi;
  pCCCmdPrm -> CSTAdef     = FALSE;

  return( AT_CMPL );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_DMY                  |
| STATE   : code                  ROUTINE : sAT_PlusCTFR             |
+--------------------------------------------------------------------+

  PURPOSE : This refers to a service that causes an incoming alerting
            call to be forwarded to a specified number. Action command
            does this.

*/

GLOBAL T_ACI_RETURN sAT_PlusCTFR  ( T_ACI_CMD_SRC    srcId,
                                    CHAR            *number,
                                    T_ACI_TOA       *type,
                                    CHAR            *subaddr,
                                    T_ACI_TOS       *satype)
{
  T_CC_CMD_PRM * pCCCmdPrm; /* points to CC command parameters */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  T_ACI_CLOG     cmdLog;      /* holds logging info */
#endif /* #if defined SMI OR defined MFW OR defined FF_MMI_RIV */
  SHORT cId;                /* holds call id */
  SHORT dscId;              /* holds call disconnect id */
  UBYTE idx;                /* holds index value */

  TRACE_FUNCTION ("sAT_PlusCTFR()");

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
   * Check that number contains a valid dial string.
   * A dial string is valid if it contains at least one valid dial
   * character, garbage within the dial string is ignored (also spaces).
   */
  if (strpbrk (number, "0123456789*#AaBbCc") EQ NULL)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;

  /*
   *-------------------------------------------------------------------
   * prepare log command
   *-------------------------------------------------------------------
   */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  cmdLog.atCmd                = AT_CMD_CTFR;
  cmdLog.cmdType              = CLOG_TYPE_Set;
  cmdLog.retCode              = AT_EXCT;
  cmdLog.sId                  = ACI_NumParmNotPresent;
  cmdLog.cId                  = NOT_PRESENT_8BIT;
  cmdLog.cmdPrm.sCTFR.srcId   = srcId;
  cmdLog.cmdPrm.sCTFR.number  = number;
  cmdLog.cmdPrm.sCTFR.type    = type;
  cmdLog.cmdPrm.sCTFR.subaddr = subaddr;
  cmdLog.cmdPrm.sCTFR.satype  = satype;
#endif /* #if defined SMI OR defined MFW OR defined FF_MMI_RIV */

  /*
   *-------------------------------------------------------------------
   * find the call for which Call Deflection shall be invoked
   *-------------------------------------------------------------------
   */
  dscId = -1;

  for (cId = 0; cId < MAX_CALL_NR; cId++)
  {
    if ((ccShrdPrm.ctb[cId] NEQ NULL) AND
        (psaCC_ctb(cId)->calStat    EQ CS_ACT_REQ) AND
        (psaCC_ctb(cId)->calType    EQ CT_MTC))
    {
      dscId = cId;
      break;
    }
  }

  if ((dscId >= 0) AND
      (psaCC_ctb(dscId)->curCmd NEQ AT_CMD_NONE))
  {
    return( AT_BUSY );
  }

  /*
   * There is no check here whether CD is applicable for the specific
   * telecommunication service (22.004 Normative Annex A), as CD is
   * applicable for all CC telecommunication services.
   * GSM 07.07 says CD was only applicable for teleservice 11, but this
   * seems to be wrong.
   */

  /*
   *-------------------------------------------------------------------
   * Clear the incoming call with facility invoke component for CD
   *-------------------------------------------------------------------
   */
  /*lint -e{661} cId causes out of bounds access, it does not! */
  if( dscId >= 0 ) /* Implies cId also >= 0 */
  {
    cmhCC_flagCall( dscId, &(pCCCmdPrm -> mltyDscFlg));
    psaCC_ctb(cId)->nrmCs    = MNCC_CAUSE_CALL_CLEAR;
    psaCC_ctb(cId)->curCmd   = AT_CMD_CTFR;
    psaCC_ctb(cId)->curSrc   = srcId;

#ifdef FF_ATI
    io_setRngInd (IO_RING_OFF, CRING_SERV_TYP_NotPresent, CRING_SERV_TYP_NotPresent ); /* V.24 Ring Indicator Line */
#endif
    for (idx = 0; idx < CMD_SRC_MAX; idx++)
    {
      R_AT (RAT_CRING_OFF, (T_ACI_CMD_SRC)idx)(dscId + 1);
    }

    CCD_START;
    {
      psaCC_asmCDReq (number, type, subaddr, satype);

      psaCC_asmComponent (dscId);

      psaCC_ClearCall (dscId);
    }
    CCD_END;

    psaCC_ctb(cId)->CDStat   = CD_Requested;

#if defined SMI OR defined MFW OR defined FF_MMI_RIV
    cmdLog.cId = dscId + 1;
    rAT_PercentCLOG( &cmdLog );
#endif /* #if defined SMI OR defined MFW OR defined FF_MMI_RIV */

    return( AT_EXCT );
  }

  /*
   * No call with matching criteria has been found in the call table.
   */
  ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
  return AT_FAIL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : find_waiting_held_call   |
+--------------------------------------------------------------------+

  PURPOSE : find waiting held calls

*/

LOCAL T_ACI_RETURN find_waiting_held_call(SHORT *cwaId,
                                          SHORT *hldId,
                                          SHORT *rclId)
{
  SHORT cId;

  /* reinitialize */
  *cwaId = -1;
  *hldId = -1;
  *rclId = -1;

  for( cId = 0; cId < MAX_CALL_NR; cId++ )
  {
    if (ccShrdPrm.ctb[cId] NEQ NULL)
    {
      if( psaCC_ctb(cId)->calStat  EQ CS_ACT_REQ     AND
          psaCC_ctb(cId)->calOwn   EQ ((T_OWN)CMD_SRC_NONE)   AND
          psaCC_ctb(cId)->calType  EQ CT_MTC         )
      {
        *cwaId = cId;
      }

      if( psaCC_ctb(cId)->calStat EQ CS_HLD )
      {
        if(*hldId EQ -1)
        {
          /* take only first found call on hold (ex: in case of multiparty) */
          *hldId = cId;
        }
      }

      if( psaCC_ctb(cId)->calStat  EQ CS_ACT_REQ     AND
          psaCC_ctb(cId)->calOwn   EQ ((T_OWN)CMD_SRC_NONE)   AND
          psaCC_ctb(cId)->calType  EQ CT_NI_MOC      )
      {
        *rclId = cId;
      }
    }
  }

  if((*cwaId >= 0 AND psaCC_ctb(*cwaId)->curCmd NEQ AT_CMD_NONE) OR
     (*hldId >= 0 AND psaCC_ctb(*hldId)->curCmd NEQ AT_CMD_NONE) OR
     (*rclId >= 0 AND psaCC_ctb(*rclId)->curCmd NEQ AT_CMD_NONE) )
  {
    return( AT_BUSY );
  }
  return(AT_CMPL);
}

/* SEND LOG info to MFW */
LOCAL void chld_ratlog( T_ACI_CMD_SRC  srcId,
                        T_ACI_CHLD_MOD mode,
                        CHAR           *call,
                        T_ACI_CHLD_ACT act,
                        SHORT          cId,
                        T_ACI_RETURN   acireturn )
{
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  T_ACI_CLOG     cmdLog;      /* holds logging info */

  /* prepare log command */
  cmdLog.atCmd                = AT_CMD_CHLD;
  cmdLog.cmdType              = CLOG_TYPE_Set;
  cmdLog.sId                  = ACI_NumParmNotPresent;
  cmdLog.cmdPrm.sCHLD.srcId   = srcId;
  cmdLog.cmdPrm.sCHLD.mode    = mode;
  cmdLog.cmdPrm.sCHLD.call    = call;

  cmdLog.cmdPrm.sCHLD.act = act;
  cmdLog.cId              = cId+1;
  cmdLog.retCode          = acireturn;


  rAT_PercentCLOG( &cmdLog );
#endif  /* SMI OR defined MFW */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : chld_RelHldOrUdub        |
+--------------------------------------------------------------------+

  PURPOSE : ???
*/
LOCAL T_ACI_RETURN chld_RelHldOrUdub(T_ACI_CMD_SRC srcId)
{
  T_CC_CMD_PRM *pCCCmdPrm; /* points to CC command parameters */
  SHORT        cId;
  UBYTE        idx;        /* holds index value */
  SHORT        dscId;      /* holds call disconnect id */

  TRACE_FUNCTION("chld_RelHldOrUdub");

  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;

/*
 *---------------------------------------------------------------
 * clear a waiting call
 *---------------------------------------------------------------
 */
  pCCCmdPrm -> mltyCncFlg = 0;
  pCCCmdPrm -> mltyDscFlg = 0;

  cId = psaCC_ctbFindCall( (T_OWN)CMD_SRC_NONE, CS_ACT_REQ, CT_MTC );

  if( cId >= 0 )
  {
    if( psaCC_ctb(cId)->curCmd NEQ AT_CMD_NONE )
      return( AT_BUSY );

    pCCCmdPrm -> CHLDmode = CHLD_MOD_RelHldOrUdub;
    CHLDaddInfo           = NO_CHLD_ADD_INFO;

    cmhCC_ClearCall(cId, MNCC_CAUSE_USER_BUSY, srcId, AT_CMD_CHLD, NULL);

    /* inform MFW */
    chld_ratlog( srcId, pCCCmdPrm -> CHLDmode, NULL, CHLD_ACT_Release, cId, AT_EXCT );

#ifdef FF_ATI
    io_setRngInd (IO_RING_OFF, CRING_SERV_TYP_NotPresent, CRING_SERV_TYP_NotPresent ); /* V.24 Ring Indicator Line */
#endif
    for( idx = 0; idx < CMD_SRC_MAX; idx++ )
    {
      R_AT( RAT_CRING_OFF, (T_ACI_CMD_SRC)idx )( cId+1 );
    }

    return( AT_EXCT );
  }

/*
 *---------------------------------------------------------------
 * clear a CCBS recall
 *---------------------------------------------------------------
 */
  cId = psaCC_ctbFindCall( (T_OWN)CMD_SRC_NONE, CS_ACT_REQ, CT_NI_MOC );

  if( cId >= 0 )
  {
    if( psaCC_ctb(cId)->curCmd NEQ AT_CMD_NONE )

      return( AT_BUSY );

    psaCC_ctb(cId)->nrmCs  = MNCC_CAUSE_USER_BUSY;

    psaCC_ClearCall (cId);

    /* inform MFW */
    chld_ratlog( srcId, pCCCmdPrm -> CHLDmode, NULL, CHLD_ACT_Release, cId, AT_CMPL );

    psaCC_FreeCtbNtry (cId);
    return( AT_CMPL );
  }

/*
 *---------------------------------------------------------------
 * or clear all held calls
 *---------------------------------------------------------------
 */
  dscId = -1;
  for( cId = 0; cId < MAX_CALL_NR; cId++ )
  {
    if (ccShrdPrm.ctb[cId] NEQ NULL AND
        psaCC_ctb(cId)->calStat    EQ CS_HLD AND
        psaCC_ctb(cId)->curCmd     EQ AT_CMD_NONE)
    {
      cmhCC_ClearCall( cId, MNCC_CAUSE_CALL_CLEAR, srcId, AT_CMD_CHLD, NULL);
      dscId = cId;
    }
  }

  if( ! pCCCmdPrm -> mltyDscFlg )
  {
    return( AT_BUSY );
  }
  else if (dscId >= 0)
  {
    pCCCmdPrm -> CHLDmode = CHLD_MOD_RelHldOrUdub;
    CHLDaddInfo           = NO_CHLD_ADD_INFO;

    /* inform MFW */
    chld_ratlog( srcId, pCCCmdPrm->CHLDmode, NULL, CHLD_ACT_Release, dscId, AT_EXCT );
    return( AT_EXCT );
  }
  else
  {
    return( AT_FAIL );
  }
}

/*-----------------------------------------------------------------
 * release all active calls, and accept held or waiting call
 *-----------------------------------------------------------------*/
LOCAL T_ACI_RETURN chld_RelActAndAcpt(T_ACI_CMD_SRC srcId)
{
  SHORT        cwaId;      /* holds call waiting id */
  SHORT        hldId;      /* holds call hold id */
  SHORT        rclId;      /* holds recall id */
  SHORT        dscId = -1; /* will be set if a call is disconnected (with its cId) */
  SHORT        cId;
  T_ACI_RETURN ret;
  T_CC_CMD_PRM *pCCCmdPrm; /* points to CC command parameters */

  TRACE_FUNCTION("chld_RelActAndAcpt");

  /* find the waiting or held call */
  ret = find_waiting_held_call(&cwaId, &hldId, &rclId);

  TRACE_EVENT_P1("cwaId = %d", cwaId);
  if(ret EQ AT_BUSY)
    return(AT_BUSY);

  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;

/*
 *---------------------------------------------------------------
 * clear all active calls
 *---------------------------------------------------------------
 */
  pCCCmdPrm -> mltyCncFlg = 0;
  pCCCmdPrm -> mltyDscFlg = 0;

  for( cId = 0; cId < MAX_CALL_NR; cId++ )
  {
    if (ccShrdPrm.ctb[cId] NEQ NULL AND
        psaCC_ctb(cId)->calStat    EQ CS_ACT     AND
        psaCC_ctb(cId)->curCmd     EQ AT_CMD_NONE)
    {
      psaCC_StopDTMF (cId); /* HM 27.07.00 */
      dscId = cId;

      cmhCC_ClearCall( cId, MNCC_CAUSE_CALL_CLEAR, srcId, AT_CMD_CHLD, NULL );
    }
  }

/*
 *---------------------------------------------------------------
 * accept the waiting call
 *---------------------------------------------------------------
 */
  if( cwaId >= 0 )
  {
    cmhCC_flagCall( cwaId, &(pCCCmdPrm -> mltyCncFlg));
    pCCCmdPrm -> CHLDmode = CHLD_MOD_RelActAndAcpt;

    if( dscId NEQ -1 )
    {
      CHLDaddInfo = CHLD_ADD_INFO_ACC_CAL;
    }
    else
    {
      cmhCC_AcceptCall(cwaId, srcId, AT_CMD_CHLD);
    }

    /* inform MFW */
    chld_ratlog( srcId, pCCCmdPrm->CHLDmode, NULL, CHLD_ACT_Accept, cwaId, AT_EXCT );
    return( AT_EXCT );
  }

/*
 *---------------------------------------------------------------
 * accept the CCBS recall
 *---------------------------------------------------------------
 */
  if( rclId >= 0 )
  {
    cmhCC_flagCall( rclId, &(pCCCmdPrm -> mltyCncFlg));
    pCCCmdPrm -> CHLDmode = CHLD_MOD_RelActAndAcpt;

    /*
     * The MSC in GSM 04.93, figure 4.3.2 says the SETUP for the CCBS
     * call is sent immediately after the DISCONNECT for the existing
     * call was sent into the network, they do not wait for a RELEASE
     * from the network. This seems to be different for the call waiting
     * case, compare this with the MSC in GSM 11.10 clause 31.3.1.2.
     */
    cmhCC_NewCall(rclId, srcId, AT_CMD_D);

    /* inform MFW */
    chld_ratlog( srcId, pCCCmdPrm->CHLDmode, NULL, CHLD_ACT_Accept, rclId, AT_EXCT );
    return( AT_EXCT );
  }

/*
 *---------------------------------------------------------------
 * retrieve the held call
 *---------------------------------------------------------------
 */
  if( hldId >= 0 )
  {
    cmhCC_flagCall( hldId, &(pCCCmdPrm -> mltyCncFlg));
    pCCCmdPrm -> CHLDmode = CHLD_MOD_RelActAndAcpt;

    if( dscId NEQ -1 )
    {
      CHLDaddInfo = CHLD_ADD_INFO_RTV_CAL;
    }
    else
    {
      cmhCC_RetrieveCall(hldId, srcId);
    }

    /* inform MFW */
    chld_ratlog( srcId,
                 pCCCmdPrm->CHLDmode,
                 NULL,
                 (psaCC_ctb(hldId)->mptyStat EQ CS_ACT)? CHLD_ACT_RetrieveMpty:CHLD_ACT_Retrieve,
                 hldId,
                 AT_EXCT );

    return( AT_EXCT );
  }

/*
 *---------------------------------------------------------------
 * at least one call was disconnected
 *---------------------------------------------------------------
 */
  if( dscId NEQ -1 )
  {
    pCCCmdPrm -> CHLDmode = CHLD_MOD_RelActAndAcpt;
    CHLDaddInfo           = NO_CHLD_ADD_INFO;

    /* inform MFW */
    chld_ratlog( srcId, pCCCmdPrm->CHLDmode, NULL, CHLD_ACT_Release, dscId, AT_EXCT );
    return( AT_EXCT );
  }
  return (AT_FAIL);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : chld_RelActSpec          |
+--------------------------------------------------------------------+

  PURPOSE : release a specific active call
*/
LOCAL T_ACI_RETURN chld_RelActSpec(T_ACI_CMD_SRC srcId, CHAR *call)
{
  T_CC_CMD_PRM *pCCCmdPrm; /* points to CC command parameters */
  SHORT        spcId;      /* holds specified call id */

  T_ACI_RETURN retCode; 
  
  TRACE_FUNCTION("chld_RelActSpec");

  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;
  /* Implements Measure 117 */
  retCode = chld_Rel_MultipartySpec( srcId, &spcId,
                                     CHLD_MOD_RelActSpec, call );
  if ( retCode NEQ AT_CMPL )
  {
    return retCode;
  }
/*
 *---------------------------------------------------------------
 * clear the specific active call if possible
 *---------------------------------------------------------------
 */
  if (ccShrdPrm.ctb[spcId] EQ NULL OR psaCC_ctb(spcId)->calStat NEQ CS_ACT)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return( AT_FAIL );        /* specific call does not match condition */
  }

  cmhCC_ClearCall ( spcId, MNCC_CAUSE_CALL_CLEAR, srcId, AT_CMD_CHLD, NULL );

  pCCCmdPrm -> CHLDmode = CHLD_MOD_RelActSpec;
  CHLDaddInfo           = NO_CHLD_ADD_INFO;

  /* inform MFW */
  chld_ratlog( srcId, pCCCmdPrm->CHLDmode, call, CHLD_ACT_Release, spcId, AT_EXCT );
  return( AT_EXCT );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : chld_RelAnySpec          |
+--------------------------------------------------------------------+

  PURPOSE : release a specific  call
*/
LOCAL T_ACI_RETURN chld_RelAnySpec(T_ACI_CMD_SRC srcId, CHAR *call)
{
  T_CC_CMD_PRM *pCCCmdPrm; /* points to CC command parameters */
  SHORT        spcId;      /* holds specified call id */
  SHORT        waitId = NO_ENTRY; /* holds call waiting id */
  UBYTE        idx    = 0; /* temporary counter */

  T_ACI_RETURN retCode; 

  TRACE_FUNCTION("chld_RelAnySpec");

  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;
  /* Implements Measure 117 */
  retCode = chld_Rel_MultipartySpec( srcId, &spcId,
                                     CHLD_MOD_RelAnySpec, call );
  if ( retCode NEQ AT_CMPL )
  {
    return retCode;
  }
/*
 *---------------------------------------------------------------
 * clear the specific call if possible
 *---------------------------------------------------------------
 */
  if (ccShrdPrm.ctb[spcId] EQ NULL)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return( AT_FAIL );        /* specific call does not match condition */
  }

  cmhCC_ClearCall ( spcId, MNCC_CAUSE_CALL_CLEAR, srcId, AT_CMD_CHLD, &waitId );

  /* The call that shall be released is a waiting call (stop ringing) */
  if( waitId NEQ NO_ENTRY )
  {
    cmhCC_flagCall( waitId, &(pCCCmdPrm -> mltyDscFlg));
    psaCC_ctb(waitId)->nrmCs  = MNCC_CAUSE_USER_BUSY;
    psaCC_ctb(waitId)->curCmd = AT_CMD_CHLD;
    psaCC_ctb(waitId)->curSrc = srcId;
    psaCC_ClearCall (waitId);
    
#ifdef AT_INTERPRETER
    io_setRngInd (IO_RING_OFF, CRING_SERV_TYP_NotPresent, CRING_SERV_TYP_NotPresent ); /* V.24 Ring Indicator Line */
#endif
    for( idx = 0; idx < CMD_SRC_MAX; idx++ )
    {
      R_AT( RAT_CRING_OFF, (T_ACI_CMD_SRC)idx )( waitId+1 );
    }
  }

  pCCCmdPrm->CHLDmode   = CHLD_MOD_RelAnySpec;
  CHLDaddInfo           = NO_CHLD_ADD_INFO;

  /* inform MFW */
  chld_ratlog( srcId, pCCCmdPrm->CHLDmode, call, CHLD_ACT_Release, spcId, AT_EXCT );
  return( AT_EXCT );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : chld_HldActAndAcpt       |
+--------------------------------------------------------------------+

  PURPOSE : place all active calls on hold, and accept held or waiting
            call
*/
LOCAL T_ACI_RETURN chld_HldActAndAcpt(T_ACI_CMD_SRC srcId)
{
  T_CC_CMD_PRM *pCCCmdPrm; /* points to CC command parameters */
  SHORT        cId;                /* holds call id */
  T_ACI_RETURN ret;
  SHORT        cwaId;              /* holds call waiting id */
  SHORT        hldId;              /* holds call hold id */
  SHORT        rclId;              /* holds recall id */
  BOOL         hldCalFlg  = FALSE; /* flags a held call */
  BOOL         mptyHldFlg = FALSE; /* flags a multiparty held call */
  T_ACI_CHLD_ACT chldact;          /* contains the type of CHLD activity when informing MFW */
  TRACE_FUNCTION("chld_HldActAndAcpt");

  /* find the waiting or held call */
  ret = find_waiting_held_call(&cwaId, &hldId, &rclId);

  if(ret EQ AT_BUSY)
    return(AT_BUSY);

  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;

//Spec 3GPP TS 22.083 Sec 1.6.83.2 which says - Although the call is offered to the subscriber, she cannot accept the call as long as she has one active call and one call on hold.
#if 0
  if( cwaId >=0 AND hldId >=0 )
  {
    /* check for Active Call if already Held Call and Waiting Call */
    for( cId = 0; cId < MAX_CALL_NR; cId++ )
    {
      if (ccShrdPrm.ctb[cId] NEQ NULL AND
          psaCC_ctb(cId)->calStat    EQ CS_ACT AND
          psaCC_ctb(cId)->curCmd     EQ AT_CMD_NONE )
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_OneCallOnHold );
        return( AT_FAIL );    /* only one call could be on hold,
                                 no chance for the waiting call */
      }
    }
  }
#endif
/*
 *---------------------------------------------------------------
 * put all active calls on hold
 *---------------------------------------------------------------
 */
  /* Implements Measure 181 and 182 */
  if ( chld_HoldActiveCalls( srcId, &mptyHldFlg, &hldCalFlg, &cId ) EQ FALSE )
  {
    return( AT_FAIL ); /* no data calls supported */
  }  

/*
 *-------------------------------------------------------------------------
 * retrieve the held call first, if there is a active call and a hold call
 *-------------------------------------------------------------------------
 */
  if( hldCalFlg AND hldId >= 0 )
  {
    cmhCC_flagCall( hldId, &(pCCCmdPrm -> mltyCncFlg));
    pCCCmdPrm -> CHLDmode = CHLD_MOD_HldActAndAcpt;

    cmhCC_RetrieveCall(hldId, srcId);

    /* inform MFW */
    if( hldCalFlg )
    {
      chldact = (mptyHldFlg)? CHLD_ACT_SwapMpty:CHLD_ACT_Swap;
    }
    else
    {
      chldact = (psaCC_ctb(hldId)->mptyStat EQ CS_ACT)? CHLD_ACT_RetrieveMpty :CHLD_ACT_Retrieve;
    }
    chld_ratlog( srcId, pCCCmdPrm->CHLDmode, NULL, chldact, hldId, AT_EXCT );
    return( AT_EXCT );
  }

/*
 *---------------------------------------------------------------
 * accept the waiting call
 *---------------------------------------------------------------
 */
  if( cwaId >= 0 )
  {
    cmhCC_flagCall( cwaId, &(pCCCmdPrm -> mltyCncFlg));
    pCCCmdPrm -> CHLDmode = CHLD_MOD_HldActAndAcpt;

    if( hldCalFlg )
    {
      CHLDaddInfo = CHLD_ADD_INFO_ACC_CAL;
    }
    else
    {
      cmhCC_AcceptCall(cwaId, srcId, AT_CMD_CHLD);
    }

    /* inform MFW */
    chld_ratlog( srcId, pCCCmdPrm->CHLDmode, NULL, CHLD_ACT_Accept, cwaId, AT_EXCT );
    return( AT_EXCT );
  }

/*
 *---------------------------------------------------------------
 * accept the CCBS recall
 *---------------------------------------------------------------
 */
  if( rclId >= 0 )
  {
    cmhCC_flagCall( rclId, &(pCCCmdPrm -> mltyCncFlg));
    pCCCmdPrm -> CHLDmode = CHLD_MOD_HldActAndAcpt;

    if( hldCalFlg )
    {
      CHLDaddInfo = CHLD_ADD_INFO_DIAL_CAL;
    }
    else
    {
      cmhCC_NewCall(rclId, srcId, AT_CMD_CHLD);
    }
    /* inform MFW */
    chld_ratlog( srcId, pCCCmdPrm->CHLDmode, NULL, CHLD_ACT_Accept, rclId, AT_EXCT );
    return( AT_EXCT );
  }

/*
 *---------------------------------------------------------------
 * retrieve the held call
 *---------------------------------------------------------------
 */
  if( hldId >= 0 )
  {
    cmhCC_flagCall( hldId, &(pCCCmdPrm -> mltyCncFlg));
    pCCCmdPrm -> CHLDmode = CHLD_MOD_HldActAndAcpt;

    cmhCC_RetrieveCall(hldId, srcId);

    /* inform MFW */
    if( hldCalFlg )
    {
      chldact = (mptyHldFlg)? CHLD_ACT_SwapMpty:CHLD_ACT_Swap;
    }
    else
    {
      chldact = (psaCC_ctb(hldId)->mptyStat EQ CS_ACT)? CHLD_ACT_RetrieveMpty :CHLD_ACT_Retrieve;
    }
    chld_ratlog( srcId, pCCCmdPrm->CHLDmode, NULL, chldact, hldId, AT_EXCT );
    return( AT_EXCT );
  }

/*
 *---------------------------------------------------------------
 * at least one call was put on hold
 *---------------------------------------------------------------
 */
  if( hldCalFlg )
  {
    pCCCmdPrm -> CHLDmode = CHLD_MOD_HldActAndAcpt;
    CHLDaddInfo           = NO_CHLD_ADD_INFO;

    /* inform MFW */
    chld_ratlog( srcId, pCCCmdPrm->CHLDmode,
                 NULL,
                 (mptyHldFlg)? CHLD_ACT_HoldMpty:CHLD_ACT_Hold,
                 cId, AT_EXCT );
    return( AT_EXCT );
  }
  return( AT_FAIL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : chld_HldActExc           |
+--------------------------------------------------------------------+

  PURPOSE : put all active calls on hold except the specified call
*/
LOCAL T_ACI_RETURN chld_HldActExc(T_ACI_CMD_SRC srcId, CHAR *call)
{
  T_CC_CMD_PRM *pCCCmdPrm; /* points to CC command parameters */
  SHORT        spcId;      /* holds specified call id */
  SHORT        cId;        /* holds call id */
  BOOL         hldCalFlg  = FALSE; /* flags a held call */
  BOOL         mptyHldFlg = FALSE; /* flags a multiparty held call */
  T_ACI_CHLD_ACT chld_act;

  TRACE_FUNCTION("chld_HldActExc");

  if( call EQ NULL )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );  /* no call specified */
  }

  spcId = atoi( call );

  if( spcId EQ 0 OR spcId > MAX_CALL_NR )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  spcId--;                        /* adapt to call table index */

  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;

  pCCCmdPrm -> mltyCncFlg = 0;
  pCCCmdPrm -> mltyDscFlg = 0;

  /*
   *-------------------------------------------------------------
   * if specified call is active and no multiparty
   *-------------------------------------------------------------
   */
    /* no action needed */

  /*
   *-------------------------------------------------------------
   * if specified call is active and member of multiparty
   *-------------------------------------------------------------
   */
  if (ccShrdPrm.ctb[spcId] NEQ NULL AND
      psaCC_ctb(spcId)->calStat    EQ CS_ACT AND
      psaCC_ctb(spcId)->mptyStat   EQ CS_ACT AND
      psaCC_ctb(spcId)->curCmd     EQ AT_CMD_NONE)
  {
    /* If this is a multiparty with only one call left then we must not split! */
    if (psaCC_CountMPTY() > 1)
    {
      cmhCC_flagCall( spcId, &(pCCCmdPrm -> mltyCncFlg));

      psaCC_ctb(spcId)->curCmd = AT_CMD_CHLD;
      psaCC_ctb(spcId)->curSrc = srcId;

      psaCC_SplitMPTY(spcId);

      chld_act = CHLD_ACT_SplitMpty;
    }
    else
    {
      /* call is already active, so command has no effect */
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Unknown );
      return( AT_FAIL );
    }
  }

  /*
   *-------------------------------------------------------------
   * if specified call is on hold and no multiparty
   *-------------------------------------------------------------
   */
  else if (ccShrdPrm.ctb[spcId] NEQ NULL AND
           psaCC_ctb(spcId)->calStat    EQ CS_HLD AND
           psaCC_ctb(spcId)->mptyStat   EQ CS_IDL AND
           psaCC_ctb(spcId)->curCmd     EQ AT_CMD_NONE)
  {
    for( cId = 0; cId < MAX_CALL_NR; cId++ )
    {
      if (ccShrdPrm.ctb[cId] NEQ NULL AND
          psaCC_ctb(cId)->calStat    EQ CS_ACT AND
          psaCC_ctb(cId)->curCmd     EQ AT_CMD_NONE)
      {
        if( cmhCC_getcalltype(cId) NEQ VOICE_CALL )
        {
          pCCCmdPrm -> mltyCncFlg = 0;
          ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_CallTypeNoHold );
          return( AT_FAIL );    /* no data calls supported */
        }

        hldCalFlg = TRUE;
        if( psaCC_ctb(cId)->mptyStat EQ CS_ACT )
        {
          mptyHldFlg = TRUE;
        }
        cmhCC_HoldCall(cId, srcId, AT_CMD_CHLD);
      }
    }

    cmhCC_flagCall( spcId, &(pCCCmdPrm -> mltyCncFlg));
    pCCCmdPrm -> CHLDmode = CHLD_MOD_HldActExc;

    cmhCC_RetrieveCall(spcId, srcId);

    if( hldCalFlg )
    {
      chld_act = (mptyHldFlg)?
                       CHLD_ACT_SwapMpty:CHLD_ACT_Swap;
    }
    else
      chld_act = (mptyHldFlg)?
                       CHLD_ACT_RetrieveMpty:CHLD_ACT_Retrieve;
  }

  /*
   *-------------------------------------------------------------
   * if specified call is on hold and member of multiparty
   *-------------------------------------------------------------
   */
  else if (ccShrdPrm.ctb[spcId] NEQ NULL AND
           psaCC_ctb(spcId)->calStat    EQ CS_HLD AND
           psaCC_ctb(spcId)->mptyStat   EQ CS_ACT AND
           psaCC_ctb(spcId)->curCmd     EQ AT_CMD_NONE)
  {
    for( cId = 0; cId < MAX_CALL_NR; cId++ )
    {
      if (ccShrdPrm.ctb[cId] NEQ NULL AND
          psaCC_ctb(cId)->calStat    EQ CS_ACT AND
          psaCC_ctb(cId)->curCmd     EQ AT_CMD_NONE)
      {
        if( cmhCC_getcalltype(cId) NEQ VOICE_CALL )
        {
          pCCCmdPrm -> mltyCncFlg = 0;
          ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_CallTypeNoHold );
          return( AT_FAIL );    /* no data calls supported */
        }

        hldCalFlg = TRUE;

        cmhCC_HoldCall(cId, srcId, AT_CMD_CHLD);
      }
    }

    cmhCC_flagCall( spcId, &(pCCCmdPrm -> mltyCncFlg));
    pCCCmdPrm -> CHLDmode = CHLD_MOD_HldActExc;

    CHLDaddInfo           = NO_CHLD_ADD_INFO;

    psaCC_ctb(spcId)->curCmd = AT_CMD_CHLD;
    psaCC_ctb(spcId)->curSrc = srcId;

    /* If this is a multiparty with only one call left then we must not split! */
    if (psaCC_CountMPTY() > 1)
    {
      psaCC_SplitMPTY(spcId);
      chld_act = CHLD_ACT_SplitMpty;
    }
    else
    {
      cmhCC_RetrieveCall(spcId, srcId);
      chld_act = CHLD_ACT_Retrieve;
    }
  }

  /*
   *-------------------------------------------------------------
   * if other command is running on specified call
   *-------------------------------------------------------------
   */
  else if (ccShrdPrm.ctb[spcId] NEQ NULL AND
           psaCC_ctb(spcId)->curCmd NEQ AT_CMD_NONE)
  {
    return( AT_BUSY );
  }

  /*
   *-------------------------------------------------------------
   * unknown condition
   *-------------------------------------------------------------
   */
  else
  {
    TRACE_ERROR ("Unknown condition");
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Unknown );
    return( AT_FAIL );
  }


  /*
   *-------------------------------------------------------------
   * send return code
   *-------------------------------------------------------------
   */
  if( pCCCmdPrm -> mltyCncFlg NEQ 0 )
  {
    pCCCmdPrm -> CHLDmode = CHLD_MOD_HldActExc;

    /* inform MFW */
    chld_ratlog( srcId, pCCCmdPrm->CHLDmode, call, chld_act, spcId, AT_EXCT );
    return( AT_EXCT );
  }
  else
  {
    return( AT_CMPL );
  }
}

/*-----------------------------------------------------------------
 * add a held call to the conversation
 *-----------------------------------------------------------------*/
LOCAL T_ACI_RETURN chld_AddHld(T_ACI_CMD_SRC srcId)
{
  SHORT        actId;              /* holds call active id */
  T_CC_CMD_PRM *pCCCmdPrm; /* points to CC command parameters */

  TRACE_FUNCTION("chld_AddHld( )");

  /* search a held call */
  if( psaCC_ctbFindCall( OWN_SRC_INV, CS_HLD, NO_VLD_CT ) EQ NO_ENTRY )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return( AT_FAIL );
  }

  actId = psaCC_ctbFindCall( OWN_SRC_INV, CS_ACT, NO_VLD_CT );

  if( actId EQ NO_ENTRY )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return( AT_FAIL );
  }

  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;

  cmhCC_flagCall( actId, &(pCCCmdPrm -> mltyCncFlg));

  psaCC_ctb(actId)->curCmd = AT_CMD_CHLD;
  psaCC_ctb(actId)->curSrc = srcId;

  psaCC_BuildMPTY(actId);

  pCCCmdPrm -> CHLDmode = CHLD_MOD_AddHld;

  /* inform MFW */
  chld_ratlog( srcId, pCCCmdPrm->CHLDmode, NULL, CHLD_ACT_BuildMpty, actId, AT_EXCT );
  return( AT_EXCT );
}

/*-----------------------------------------------------------------
 * explicit call transfer
 *-----------------------------------------------------------------*/
LOCAL T_ACI_RETURN chld_Ect(T_ACI_CMD_SRC srcId)
{
  SHORT cId;                /* holds call id */
  SHORT actId;              /* holds call active id */
  SHORT hldId;              /* holds call hold id */
  T_CC_CMD_PRM * pCCCmdPrm; /* points to CC command parameters */

  TRACE_FUNCTION("chld_Ect");
/*
 *---------------------------------------------------------------
 * find the active(req) or held call
 *---------------------------------------------------------------
 */
  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;

  actId = hldId = -1;

  for( cId = 0; cId < MAX_CALL_NR; cId++ )
  {
    if (ccShrdPrm.ctb[cId] NEQ NULL)
    {
      if( cmhCC_getcalltype(cId) NEQ VOICE_CALL )
      {
        /* not a valid id for ECT */
      }
      else if (psaCC_ctb(cId)->calStat  EQ CS_ACT OR
                (psaCC_ctb(cId)->calStat EQ CS_ACT_REQ AND
                 psaCC_ctb(cId)->calType EQ CT_MOC))
      {
        if( actId EQ -1 ) actId = cId;
      }
      else if( psaCC_ctb(cId)->calStat EQ CS_HLD )
      {
        if( hldId EQ -1 ) hldId = cId;
      }
    }
  }

  /* curCmd of actId will be updated only if hldId is valid. */
  if( hldId >= 0 AND
    psaCC_ctb(hldId)->curCmd NEQ AT_CMD_NONE )
  {
    TRACE_EVENT_P1("CHLD ECT: hldId busy with %d", psaCC_ctb(hldId)->curCmd);
    return( AT_BUSY );
  }
  /*
   * if command state is not idle and update the curCmd only if it is NONE
   */
  if(actId >= 0 AND hldId >= 0)
  {
    switch(psaCC_ctb(actId)->curCmd)
    {
    case(AT_CMD_NONE):
      psaCC_ctb(actId)->curCmd = AT_CMD_CHLD;
      psaCC_ctb(actId)->curSrc = srcId;
      break;
    case(AT_CMD_D):
      /* command state where actId might find itself */
      break;
    default:
      TRACE_EVENT_P1("CHLD ECT: actId busy with %d", psaCC_ctb(actId)->curCmd);
      return( AT_BUSY );
    }
  }

  if( actId >= 0 AND hldId >= 0 )
  {
    if( psaCC_ECT(hldId) NEQ 0 )
    {
      return(AT_FAIL);
    }
    /* Update params only if the facility message is successfully
       sent to the network */
    cmhCC_flagCall( actId, &(pCCCmdPrm -> mltyDscFlg));
    cmhCC_flagCall( hldId, &(pCCCmdPrm -> mltyDscFlg));
    psaCC_ctb(hldId)->curCmd = AT_CMD_CHLD;
    psaCC_ctb(hldId)->curSrc = srcId;
    ccShrdPrm.cIdMPTY        = actId;

    pCCCmdPrm -> CHLDmode = CHLD_MOD_Ect;

    /* inform MFW */
    chld_ratlog( srcId, pCCCmdPrm->CHLDmode, NULL, CHLD_ACT_ECT, actId, AT_EXCT );
    return( AT_EXCT );
  }

  TRACE_EVENT("CHLD: ECT: could not find interesting call ids");
  ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
  return( AT_FAIL );
}

/*-----------------------------------------------------------------
 * activate call completion to busy subscriber
 *-----------------------------------------------------------------*/
LOCAL T_ACI_RETURN chld_Ccbs(T_ACI_CMD_SRC srcId)
{
  T_CC_CMD_PRM *pCCCmdPrm; /* points to CC command parameters */
  SHORT        dscId = -1; /* will be set if a call is disconnected (with its cId) */
  SHORT cId;               /* holds call id */

  TRACE_FUNCTION("chld_Ccbs");

  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;

/*
 *---------------------------------------------------------------
 * find the call with CCBS possible
 *---------------------------------------------------------------
 */
  for( cId = 0; cId < MAX_CALL_NR; cId++ )
  {
    if (ccShrdPrm.ctb[cId] NEQ NULL AND
        psaCC_ctb(cId)->calStat  EQ CS_DSC_REQ AND
        psaCC_ctb(cId)->calType  EQ CT_MOC     AND
        psaCC_ctb(cId)->CCBSstat EQ CCBSS_PSSBL)
    {
      dscId = cId;
      break;
    }
  }

  if( dscId >= 0 AND
      psaCC_ctb(dscId)->curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );

/*
 *---------------------------------------------------------------
 * clear a call with CCBS possible
 *---------------------------------------------------------------
 */
  /*lint -e{661} cId causes out of bounds access, it does not! */
  if( dscId >= 0 )
  {
    pCCCmdPrm -> CHLDmode = CHLD_MOD_Ccbs;
    CHLDaddInfo           = NO_CHLD_ADD_INFO;
    psaCC_ctb(cId)->CCBSstat = CCBSS_REQ;

    cmhCC_ClearCall ( dscId, MNCC_CAUSE_CALL_CLEAR, srcId, AT_CMD_CHLD, NULL );

    /* inform MFW */
    chld_ratlog( srcId, pCCCmdPrm->CHLDmode, NULL, CHLD_ACT_CCBS, dscId, AT_EXCT );
    return( AT_EXCT );
  }

  ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
  return( AT_FAIL );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : chld_OnlyHold            |
+--------------------------------------------------------------------+

  PURPOSE : Put a call on hold (and nothing else ==> do not accept 
            any other call)
*/
LOCAL T_ACI_RETURN chld_OnlyHold(T_ACI_CMD_SRC srcId)
{
  SHORT hldId;              /* holds call hold id */
  SHORT cId;                /* holds call id */
  T_CC_CMD_PRM *pCCCmdPrm; /* points to CC command parameters */
  BOOL  hldCalFlg  = FALSE; /* flags a held call */
  BOOL  mptyHldFlg = FALSE; /* flags a multiparty held call */

  TRACE_FUNCTION("chld_OnlyHold");

  /* find held call */
  hldId = -1;

  for( cId = 0; cId < MAX_CALL_NR; cId++ )
  {
    if (ccShrdPrm.ctb[cId] NEQ NULL)
    {
      if( psaCC_ctb(cId)->calStat EQ CS_HLD )
      {
        if( hldId EQ -1 ) hldId = cId;
      }
    }
  }

  if( hldId >=0 )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_OneCallOnHold );
    return( AT_FAIL );    /* only one call could be on hold */
  }

  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;

  /* put all active calls on hold */
  /* Implements Measure 181 and 182 */
  if ( chld_HoldActiveCalls( srcId, &mptyHldFlg, &hldCalFlg, &cId ) EQ FALSE )
  {
    return( AT_FAIL ); /* no data calls supported */
  }

  /* at least one call was put on hold */
  if( hldCalFlg )
  {
    pCCCmdPrm -> CHLDmode = CHLD_MOD_OnlyHold;
    CHLDaddInfo           = NO_CHLD_ADD_INFO;

    /* inform MFW */
    chld_ratlog( srcId, pCCCmdPrm->CHLDmode,
                 NULL,
                 (mptyHldFlg)? CHLD_ACT_HoldMpty:CHLD_ACT_Hold,
                 cId, AT_EXCT );
    return( AT_EXCT );
  }
  return( AT_FAIL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : chld_RelDialCall         |
+--------------------------------------------------------------------+

  PURPOSE : release diealling call (special function for FAE's
*/
LOCAL T_ACI_RETURN chld_RelDialCall(T_ACI_CMD_SRC srcId)
{
  T_CC_CMD_PRM *pCCCmdPrm; /* points to CC command parameters */
  SHORT        cId;
  
  TRACE_FUNCTION("chld_RelDialCall");

  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;

/*
 *---------------------------------------------------------------
 * clear the dialling call
 *---------------------------------------------------------------
 */
  pCCCmdPrm -> mltyCncFlg = 0;
  pCCCmdPrm -> mltyDscFlg = 0;

  cId = psaCC_ctbFindCall( (T_OWN)srcId, CS_ACT_REQ, CT_MOC );

  TRACE_EVENT_P1("Call Id of dialling call = %d",cId);

  if( cId >= 0 )
  {
    pCCCmdPrm -> CHLDmode = CHLD_MOD_RelDialCall;
    CHLDaddInfo           = NO_CHLD_ADD_INFO;

    cmhCC_ClearCall(cId, MNCC_CAUSE_CALL_CLEAR, srcId, AT_CMD_CHLD, NULL);

    /* inform MFW */
    chld_ratlog( srcId, pCCCmdPrm -> CHLDmode, NULL, CHLD_ACT_Release, cId, AT_EXCT );

    return( AT_EXCT );
  }

  /* Unable to find call */
  return( AT_FAIL);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : chld_RetrieveHoldCall    |
+--------------------------------------------------------------------+

  PURPOSE : retrieve an held call (and nothing else)
            - waiting call's still knocked on
            - will hold active call
*/
LOCAL T_ACI_RETURN chld_RetrieveHoldCall(T_ACI_CMD_SRC srcId)
{
  T_CC_CMD_PRM  *pCCCmdPrm;          /* points to CC command parameters */
  SHORT          cId;                /* call id */
  SHORT          cwaId      = -1;    /* holds call waiting id */
  SHORT          hldId      = -1;    /* holds call hold id */
  SHORT          rclId      = -1;    /* holds recall id */
  BOOL           mptyHldFlg = FALSE; /* flags a multiparty held call */
  BOOL           hldCalFlg  = FALSE; /* flags a held call */
  T_ACI_CHLD_ACT chldact;            /* contains the type of CHLD activity when informing MFW */

  TRACE_FUNCTION("chld_RetrieveHoldCall");

  /* find the waiting or held call */
  if( find_waiting_held_call(&cwaId, &hldId, &rclId) EQ AT_BUSY)
  {
    return(AT_BUSY);
  }

  if( hldId < 0 )
  { /* no held call found */
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_CallNotFound );
    return( AT_FAIL );
  }

  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm; /* temp copy */

/*
 *---------------------------------------------------------------
 * put all active calls on hold
 *---------------------------------------------------------------
 */
  /* Implements Measure 181 and 182 */
  if ( chld_HoldActiveCalls( srcId, &mptyHldFlg, &hldCalFlg, &cId ) EQ FALSE )
  {
    return( AT_FAIL ); /* no data calls supported */
  }

  /* maybe, one or more calls were put on hold */
  if( hldCalFlg )
  {
    pCCCmdPrm -> CHLDmode = CHLD_MOD_HldActAndAcpt;
    CHLDaddInfo           = NO_CHLD_ADD_INFO;

    /* inform MFW */
    chld_ratlog( srcId, pCCCmdPrm->CHLDmode,
                 NULL,
                 (mptyHldFlg)? CHLD_ACT_HoldMpty:CHLD_ACT_Hold,
                 cId, AT_EXCT );
  }

/*
 *---------------------------------------------------------------
 * retrieve the held call
 *---------------------------------------------------------------
 */
  cmhCC_flagCall(hldId, &(pCCCmdPrm -> mltyCncFlg));
  pCCCmdPrm -> CHLDmode = CHLD_MOD_RetrieveHoldCall;

  cmhCC_RetrieveCall(hldId, srcId);

  /* inform MFW */
  chldact = (psaCC_ctb(hldId)->mptyStat EQ CS_ACT)? CHLD_ACT_RetrieveMpty :CHLD_ACT_Retrieve;
  chld_ratlog( srcId, pCCCmdPrm->CHLDmode, NULL, chldact, hldId, AT_EXCT );
  return( AT_EXCT );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : chld_RetrieveHoldCallSpec|
+--------------------------------------------------------------------+

  PURPOSE : retrieve specific held call (and nothing else)
            - release waiting call
            - will hold active call
*/
LOCAL T_ACI_RETURN chld_RetrieveHoldCallSpec(T_ACI_CMD_SRC srcId, CHAR *call)
{
  T_CC_CMD_PRM  *pCCCmdPrm;          /* points to CC command parameters */
  SHORT          cId;                /* call id */
  SHORT          temp_cId;
  BOOL           mptyHldFlg = FALSE; /* flags a multiparty held call */
  BOOL           hldCalFlg  = FALSE; /* flags a held call */
  T_ACI_CHLD_ACT chldact;            /* contains the type of CHLD activity when informing MFW */

  TRACE_FUNCTION("chld_RetrieveHoldCallSpec");

  if( call EQ NULL )
  {
    TRACE_ERROR("CALL parameter is needed !!!");
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );  /* no call specified */
  }

  cId = atoi( call ); /* char --> int */

  if( (cId EQ 0) OR (cId > MAX_CALL_NR) )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  cId--; /* adapt to call table index */

  pCCCmdPrm  = &cmhPrm[srcId].ccCmdPrm;

  /* test whether specified call is a held call */
  if ((ccShrdPrm.ctb[cId] EQ NULL)                  OR
      (psaCC_ctb(cId)->calStat NEQ CS_HLD)      OR
      (psaCC_ctb(cId)->curCmd  NEQ AT_CMD_NONE) OR
      (cmhCC_getcalltype(cId) NEQ VOICE_CALL))
  { /* no held call or no voice call --> error */
    pCCCmdPrm -> mltyCncFlg = 0;
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_CallTypeNoHold );
    return( AT_FAIL );    /* no data calls supported */
  }


/*
 *---------------------------------------------------------------
 * put all active calls on hold
 *---------------------------------------------------------------
 */
  pCCCmdPrm -> mltyCncFlg = 0;
  pCCCmdPrm -> mltyDscFlg = 0;

  for( temp_cId = 0; temp_cId < MAX_CALL_NR; temp_cId++ )
  {
    T_CC_CALL_TBL *pCallTable = ccShrdPrm.ctb[temp_cId];

    if( pCallTable NEQ NULL              AND
        pCallTable->calStat    EQ CS_ACT AND
        pCallTable->curCmd     EQ AT_CMD_NONE )
    {
      if( cmhCC_getcalltype(temp_cId) NEQ VOICE_CALL )
      {
        pCCCmdPrm -> mltyCncFlg = 0;
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_CallTypeNoHold );
        return( AT_FAIL );    /* no data calls supported */
      }

      if( (pCallTable->mptyStat EQ CS_IDL) OR
          ((pCallTable->mptyStat EQ CS_ACT) AND (mptyHldFlg EQ FALSE)) )
      {
        hldCalFlg = TRUE;
      }

      /* if active call is a multiparty call */
      if( pCallTable->mptyStat EQ CS_ACT )
      {
        mptyHldFlg = TRUE;
      }

      cmhCC_HoldCall(temp_cId, srcId, AT_CMD_CHLD);
    }
  }

  /* maybe, one or more calls were put on hold */
  if( hldCalFlg )
  {
    pCCCmdPrm->CHLDmode = CHLD_MOD_HldActAndAcpt;
    CHLDaddInfo = NO_CHLD_ADD_INFO;

    /* inform MFW */
    chld_ratlog( srcId, pCCCmdPrm->CHLDmode,
                 NULL,
                 (mptyHldFlg)? CHLD_ACT_HoldMpty:CHLD_ACT_Hold,
                 cId, AT_EXCT );
  }

/*
 *---------------------------------------------------------------
 * retrieve the held call
 *---------------------------------------------------------------
 */
  cmhCC_flagCall(cId, &(pCCCmdPrm -> mltyCncFlg));
  pCCCmdPrm -> CHLDmode = CHLD_MOD_RetrieveHoldCallSpec;

  cmhCC_RetrieveCall(cId, srcId);

  /* inform MFW */
  if (psaCC_ctb(cId)->mptyStat EQ CS_ACT)
    chldact = CHLD_ACT_RetrieveMpty;
  else
    chldact = CHLD_ACT_Retrieve;
  chld_ratlog( srcId, pCCCmdPrm->CHLDmode, NULL, chldact, cId, AT_EXCT );
  return( AT_EXCT );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_PlusCHLD             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CHLD AT command
            which is responsible to handle the supplementary services
            witin a call
*/
GLOBAL T_ACI_RETURN sAT_PlusCHLD  ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_CHLD_MOD   mode,
                                    CHAR            *call)
{
  TRACE_FUNCTION ("sAT_PlusCHLD()");

  if( mode > CHLD_MOD_RelDialCall )
  { /* not allowed modes inside '+'-command */
    TRACE_EVENT("this mode is not allowed for sAT_PlusCHLD()");
    return( AT_FAIL );
  }
  else
  {
    return( cmhCC_CHLD_Serv(srcId, mode, call) );
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_PercentCHLD          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CHLD AT command
            which is responsible to handle the supplementary services
            witin a call
*/
GLOBAL T_ACI_RETURN sAT_PercentCHLD  ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_CHLD_MOD   mode,
                                    CHAR            *call)
{
  TRACE_FUNCTION ("sAT_PercentCHLD()");

  return( cmhCC_CHLD_Serv(srcId, mode, call) );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_PlusCCUG             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CCUG AT command
            which is responsible to set the parameters for closed user
            group supplementary services.

            <mode>   : CUG mode.
            <index>  : CUG index.
            <info>   : CUG info.
*/

GLOBAL T_ACI_RETURN sAT_PlusCCUG  ( T_ACI_CMD_SRC   srcId,
                                    T_ACI_CCUG_MOD  mode,
                                    T_ACI_CCUG_IDX  index,
                                    T_ACI_CCUG_INFO info)
{
  T_CC_CMD_PRM * pCCCmdPrm; /* points to CC command parameters */

  TRACE_FUNCTION ("sAT_PlusCCUG()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;

/*
 *-------------------------------------------------------------------
 * process the mode parameter
 *-------------------------------------------------------------------
 */
  switch( mode )
  {
    case( CCUG_MOD_NotPresent ):

      mode = pCCCmdPrm -> CCUGmode;
      break;

    case( CCUG_MOD_DisableTmp ):
    case( CCUG_MOD_EnableTmp  ):

      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the index parameter
 *-------------------------------------------------------------------
 */
  switch( index )
  {
    case( CCUG_IDX_NotPresent ):

      index = pCCCmdPrm -> CCUGidx;
      break;

    case( CCUG_IDX_0  ):
    case( CCUG_IDX_1  ):
    case( CCUG_IDX_2  ):
    case( CCUG_IDX_3  ):
    case( CCUG_IDX_4  ):
    case( CCUG_IDX_5  ):
    case( CCUG_IDX_6  ):
    case( CCUG_IDX_7  ):
    case( CCUG_IDX_8  ):
    case( CCUG_IDX_9  ):
    case( CCUG_IDX_No ):

      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the info parameter
 *-------------------------------------------------------------------
 */
  switch( info )
  {
    case( CCUG_INFO_NotPresent ):

      info = pCCCmdPrm -> CCUGinfo;
      break;

    case( CCUG_INFO_No          ):
    case( CCUG_INFO_SuppOa      ):
    case( CCUG_INFO_SuppPrefCug ):
    case( CCUG_INFO_SuppBoth    ):

      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * assign the parameters
 *-------------------------------------------------------------------
 */
  pCCCmdPrm -> CCUGmode = mode;
  pCCCmdPrm -> CCUGidx  = index;
  pCCCmdPrm -> CCUGinfo = info;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_PlusVTS              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +VTS AT command
            which is responsible to send DTMF tones.
*/

GLOBAL T_ACI_RETURN sAT_PlusVTS   ( T_ACI_CMD_SRC    srcId,
                                    CHAR             dtmf,
                                    T_ACI_VTS_MOD    mode )
{
  SHORT        cId;                 /* holds call id */
  BOOL         param_ok;

  TRACE_FUNCTION ("sAT_PlusVTS()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }


  cId = cmhCC_find_call_for_DTMF( );
  if (cId EQ NO_ENTRY)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return( AT_FAIL );
  }

  /* allow to send DTMF tone one after another quickly
  if( psaCC_ctb(cId)->dtmfCmd NEQ AT_CMD_NONE )
    return( AT_BUSY );*/

  /* if DTMF are already being sent */
  if (( ccShrdPrm.dtmf.cnt AND mode NEQ VTS_MOD_ManStop ) OR
    ( ccShrdPrm.dtmf.cur AND (mode NEQ VTS_MOD_ManStop)))
  {
    TRACE_EVENT("DTMF are already being sent !");
    return( AT_BUSY );
  }
  else if ( !ccShrdPrm.dtmf.cur AND mode EQ VTS_MOD_ManStop )
  {
    TRACE_EVENT("Cannot stop a DTMF tone that hasn't been started!");
    return( AT_FAIL );
  }
  else if ( ccShrdPrm.dtmf.cur AND (ccShrdPrm.dtmf.cur NEQ dtmf ))
  {
    TRACE_EVENT("Cannot stop a different DTMF tone than the one that is started!");
    return( AT_FAIL );
  }

  /* process mode parameter */
  switch( mode )
  {
    case( VTS_MOD_Auto ):
      psaCC_ctb(cId)->dtmfMode = MNCC_DTMF_MOD_AUTO;
      break;

    case( VTS_MOD_ManStart ):
      psaCC_ctb(cId)->dtmfMode = MNCC_DTMF_MOD_MAN_START;
      ccShrdPrm.dtmf.cur = dtmf;
      break;

    case( VTS_MOD_ManStop ):
      psaCC_ctb(cId)->dtmfMode = MNCC_DTMF_MOD_MAN_STOP;
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
      return( AT_FAIL );
  }

  psaCC_ctb(cId)->dtmfCmd = AT_CMD_VTS; /* wait for confirmation */
  psaCC_ctb(cId)->dtmfSrc = (T_OWN)srcId;
  /* has to remember tone sent in case of an abort */
  ccShrdPrm.dtmf.cnt = 1;
  ccShrdPrm.dtmf.dig[0] = dtmf;
  ccShrdPrm.dtmf.cId = cId;

  param_ok = cmhCC_SendDTMFdig ( AT_CMD_VTS, cId, dtmf, psaCC_ctb(cId)->dtmfMode);

  if( param_ok )
  {
    return( AT_EXCT );
  }
  else
  {
    ccShrdPrm.dtmf.cnt = 0;
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_PlusCSNS             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CSNS AT command
            which is responsible to set the parameters for single
            numbering scheme.
*/

GLOBAL T_ACI_RETURN sAT_PlusCSNS  ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_CSNS_MOD   mode)
{

  TRACE_FUNCTION ("sAT_PlusCSNS()");

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
 *-------------------------------------------------------------------
 * check parameter mode
 *-------------------------------------------------------------------
 */
  switch( mode )
  {
    case( CSNS_MOD_NotPresent ):

      mode = CSNS_MOD_Voice;
      break;

    case( CSNS_MOD_Voice ):
#ifdef FAX_AND_DATA
    case( CSNS_MOD_VAFVoice ):
    case( CSNS_MOD_Fax ):
    case( CSNS_MOD_VADVoice ):
    case( CSNS_MOD_Data ):
    case( CSNS_MOD_VAFFax ):
    case( CSNS_MOD_VADData ):
    case( CSNS_MOD_VFD ):
#endif /* FAX_AND_DATA */
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

  ccShrdPrm.snsMode = mode;

  psaCC_Config( );

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_AndF                 |
+--------------------------------------------------------------------+

  PURPOSE : Reset all values to defaults.
*/

GLOBAL T_ACI_RETURN sAT_AndF      ( T_ACI_CMD_SRC srcId,
                                    SHORT         value)
{

  TRACE_FUNCTION ("sAT_AndF()");

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
 *-------------------------------------------------------------------
 * check parameter value
 *-------------------------------------------------------------------
 */
  if( value NEQ 0 AND value NEQ ACI_NumParmNotPresent )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * reset value to defaults
 *-------------------------------------------------------------------
 */
  cmh_Reset ( srcId, TRUE );

  return( AT_CMPL );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sGsmAT_Z                 |
+--------------------------------------------------------------------+

  PURPOSE : Reset all values to factory defaults in the GSM part of ACI.
*/

GLOBAL T_ACI_RETURN sGsmAT_Z ( T_ACI_CMD_SRC srcId )
{
  SHORT cId;                  /* holds call id */
  SHORT waitId = NO_ENTRY;    /* holds call waiting id */
  T_CC_CMD_PRM * pCCCmdPrm;   /* points to CC command parameters */

#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  T_ACI_CLOG     cmdLog;      /* holds logging info */
#endif

  TRACE_FUNCTION ("sGsmAT_Z()");

/*
 *-------------------------------------------------------------------
 * command source is checked
 *-------------------------------------------------------------------
 */
  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;

/*
 *-------------------------------------------------------------------
 * prepare log command
 *-------------------------------------------------------------------
 */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  cmdLog.atCmd                = AT_CMD_Z;
  cmdLog.cmdType              = CLOG_TYPE_Set;
  cmdLog.retCode              = AT_EXCT;
  cmdLog.sId                  = ACI_NumParmNotPresent;
  cmdLog.cmdPrm.sH.srcId      = srcId;
#endif  /* End of SMI Or defined MFW*/

/*
 *-------------------------------------------------------------------
 * clear all calls except a waiting call
 *-------------------------------------------------------------------
 */
  pCCCmdPrm -> mltyDscFlg = 0;

  for( cId = 0; cId < MAX_CALL_NR; cId++ )
  {
    /* Clear only calls in use ! */
    if (ccShrdPrm.ctb[cId] NEQ NULL)
    {
      cmhCC_ClearCall(cId,MNCC_CAUSE_CALL_CLEAR, srcId, AT_CMD_Z, &waitId);
    }
  }

 if( pCCCmdPrm -> mltyDscFlg )
  {
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
    cmdLog.cId = cId+1;
    rAT_PercentCLOG( &cmdLog );
#endif
    return( AT_EXCT );
  }

/*
 *-------------------------------------------------------------------
 * disconnect a waiting call with user determined user busy
 *-------------------------------------------------------------------
 */
  /* Implements Measure 165 */
  if ( waitId NEQ NO_ENTRY )
  {
    cmhCC_disconnect_waiting_call ( srcId, waitId, AT_CMD_Z,
                                    &(pCCCmdPrm -> mltyDscFlg) );
    return( AT_EXCT );
  }

/*
 *-------------------------------------------------------------------
 *   reset value to default if no call is active
 *-------------------------------------------------------------------
 */
  cmh_Reset ( srcId, TRUE );

  R_AT(RAT_Z, srcId)();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_Z                    |
+--------------------------------------------------------------------+

  PURPOSE : Reset all values to factory defaults.
*/

GLOBAL T_ACI_RETURN sAT_Z ( T_ACI_CMD_SRC srcId,
                            SHORT         value)
{

  TRACE_FUNCTION ("sAT_Z()");

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
 *-------------------------------------------------------------------
 * check parameter value
 *-------------------------------------------------------------------
 */
  if( value NEQ 0 AND value NEQ ACI_NumParmNotPresent )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 *   clear all calls and reset all value to default
 *-------------------------------------------------------------------
 */
  
#if defined (GPRS) AND defined (DTI)    
  return sGprsAT_Z ( srcId );
#else
  return sGsmAT_Z ( srcId );
#endif  /* GPRS */
}

#if defined (FAX_AND_DATA) AND defined (DTI)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_O                    |
+--------------------------------------------------------------------+

  PURPOSE : Return to online mode
*/

GLOBAL T_ACI_RETURN sAT_O ( T_ACI_CMD_SRC srcId )
{
  SHORT           cId;           /* holds call id */
  T_DTI_CONN_CB  *dti_conn_cb;
  T_DTI_ENTITY_ID entity_list[2]; 
#ifdef FF_PSI
  T_ACI_DTI_PRC_PSI *src_infos_psi = NULL;
#endif /* FF_PSI */
  TRACE_FUNCTION("sAT_O()");

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
 *-------------------------------------------------------------------
 * find active call
 *-------------------------------------------------------------------
 */
  cId = psaCC_ctbFindCall( OWN_SRC_INV, CS_ACT, NO_VLD_CT );

  if( cId EQ NO_ENTRY )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_CallNotFound );
    return( AT_FAIL );
  }

  switch( cmhCC_getcalltype(cId) )
  {
    case( TRANS_CALL ):
      entity_list[0] = DTI_ENTITY_TRA;
      dti_conn_cb = TRA_connect_dti_cb;
      break;

    case( NON_TRANS_CALL ):
      entity_list[0] = DTI_ENTITY_L2R;
      dti_conn_cb = L2R_connect_dti_cb;
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
      return( AT_FAIL );
  }

  if( psaCC_ctb(cId)->curCmd NEQ AT_CMD_NONE ) 
    return( AT_BUSY );

  R_AT( RAT_CONNECT, srcId )
    ( AT_CMD_O, cmhCC_GetDataRate(&psaCC_ctb(cId)->
                                   BC[psaCC_ctb(cId)->curBC]),
      cId+1, FALSE );

  psaCC_ctb(cId)->curCmd = AT_CMD_O;
  psaCC_ctb(cId)->curSrc = srcId;


  if (IS_SRC_BT(srcId))
  {
    entity_list[1] = DTI_ENTITY_BLUETOOTH;
    dti_cntrl_est_dpath((UBYTE)srcId, entity_list, 2, SPLIT, dti_conn_cb);
  }
  else
  {
#ifdef FF_PSI
    src_infos_psi = find_element(psi_src_params,(UBYTE) srcId, cmhPSItest_srcId);
    memset(&psi_ato,0,sizeof(T_ACI_PSI_CALL_TYPE));
    if (src_infos_psi NEQ NULL)
    {
      psi_ato.src_id = (UBYTE)srcId;
      psi_ato.entity_to_conn = entity_list[0];
      psi_ato.num_entities = 1;
      psi_ato.mode = SPLIT;
      psi_ato.cb = dti_conn_cb;
      psi_ato.capability = DTI_CPBLTY_SER;
      psi_ato.cid = DTI_CID_NOTPRESENT;
      psi_ato.last_cmd = AT_CMD_O;
    }
    else
    {
#endif /* FF_PSI */
      dti_cntrl_est_dpath_indirect ( (UBYTE)srcId,
                                      entity_list,
                                      1,
                                      SPLIT,
                                      dti_conn_cb,
                                      DTI_CPBLTY_SER,
                                      DTI_CID_NOTPRESENT);
#ifdef FF_PSI
    }
#endif /* FF_PSI */
  }

  return( AT_EXCT );
}
#endif

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                 |
|                                 ROUTINE : sAT_PercentALS          |
+-------------------------------------------------------------------+

  PURPOSE : set the ALS mode for outgoing calls (voice)
            ALS_MOD_SPEECH:
               indicates bearer capability => BEARER_SERV_SPEECH
            ALS_MOD_AUX_SPEECH:
               indicates bearer capability => BEARER_SERV_AUX_SPEECH
*/

GLOBAL T_ACI_RETURN sAT_PercentALS( T_ACI_CMD_SRC srcId,
                                    T_ACI_ALS_MOD mode   )
{
  TRACE_FUNCTION("sAT_PercentALS()");

  if( !cmh_IsVldCmdSrc( srcId ) )
  {
    return( AT_FAIL );
  }

  /* line will really change? */
  if (mode EQ cmhPrm[srcId].ccCmdPrm.ALSmode)
  {
    return (AT_CMPL);
  }

  if ( ALSlock NEQ ALS_MOD_NOTPRESENT )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return( AT_FAIL );
  }

  if ( mode EQ ALS_MOD_SPEECH )
  {
    cmhPrm[srcId].ccCmdPrm.ALSmode = ALS_MOD_SPEECH;
    return( AT_CMPL );
  }
  else if ( mode EQ ALS_MOD_AUX_SPEECH )
  {
    /* E-Plus SIM-Card inserted (mcc=0x262, mnc=0x03) ? */
    if (cmhSIM_plmn_is_hplmn(0x262, 0x03F))
    {
      cmhPrm[srcId].ccCmdPrm.ALSmode = ALS_MOD_AUX_SPEECH;
      return( AT_CMPL );
    }
    else
    {
      simEntStat.curCmd = AT_CMD_ALS;
      simShrdPrm.owner = (T_OWN)srcId;
      simEntStat.entOwn = srcId;

      ccShrdPrm.als_cmd = ALS_CMD_SET;

      cmhCC_checkALS_Support();
      return (AT_EXCT);
    }
  }
  else
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                 |
|                                 ROUTINE : sAT_PercentCTTY         |
+-------------------------------------------------------------------+

  PURPOSE : Handling TTY
*/

GLOBAL T_ACI_RETURN sAT_PercentCTTY (T_ACI_CMD_SRC srcId,
                                     T_ACI_CTTY_MOD mode,
                                     T_ACI_CTTY_REQ req)
{

#ifdef FF_TTY
  SHORT       cId;             /* holds call id */
  T_TTY_CMD   ttyAction = TTY_OFF;
  T_MNCC_bcpara   *bc;

  TRACE_FUNCTION("sAT_PercentCTTY()");

   if (!cmh_IsVldCmdSrc( srcId ))
  {
    return AT_FAIL;
  }
  
  /*
   * find if any call is active
   */
  cId = psaCC_ctbFindCall( OWN_SRC_INV, CS_ACT, NO_VLD_CT);
  
  if(cId NEQ NO_ENTRY )
  {
    T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

    TRACE_EVENT ("sAT_PercentCTTY() Active call found ");

    /*
     * if command state is not idle
     */
    switch(ctb->curCmd)
    {
    case(AT_CMD_NONE):
      break;
    default:
      TRACE_EVENT_P1("CTTY: cId busy with %d", ctb->curCmd);
      return( AT_BUSY );
    }

    /*
     * Check if the active call is a TTY call
     */
    bc = &ctb->BC[ctb->curBC];

    if (bc->bearer_serv EQ MNCC_BEARER_SERV_SPEECH_CTM OR
        bc->bearer_serv EQ MNCC_BEARER_SERV_AUX_SPEECH_CTM)
    {
      if ((mode EQ CTTY_MOD_Disable) OR (mode EQ CTTY_MOD_Enable))
      {
        cmhPrm[srcId].ccCmdPrm.CTTYmode = mode;
      }

      switch (req)
      {
      case CTTY_REQ_Off:
        /*
         * stop the running TTY, a normal voice call
         */
        ccShrdPrm.ctmReq = MNCC_CTM_DISABLED;
        ccShrdPrm.ttyCmd = (UBYTE)TTY_OFF;
        
        cmhCC_TTY_Control (cId, TTY_STOP);
        return( AT_CMPL );

      case CTTY_REQ_On:
        ccShrdPrm.ctmReq = MNCC_CTM_ENABLED;
        ccShrdPrm.ttyCmd = (UBYTE)TTY_ALL;
        break;
      case CTTY_REQ_HCO:
        ccShrdPrm.ctmReq = MNCC_CTM_ENABLED;
        ccShrdPrm.ttyCmd = (UBYTE)TTY_HCO;
        break;
      case CTTY_REQ_VCO:
        ccShrdPrm.ctmReq = MNCC_CTM_ENABLED;
        ccShrdPrm.ttyCmd = (UBYTE)TTY_VCO;
        break;
      default:
        ACI_ERR_DESC (ACI_ERR_CLASS_Ext, EXT_ERR_Parameter);
        return AT_FAIL;
      }
      
      audio_dyn_set_tty (ttyAction = (T_TTY_CMD)ccShrdPrm.ttyCmd);

      /*
       * This is for the case when mode is changed from Voice to any other
       */
      ccShrdPrm.ctmState = TTY_STATE_ACTIVE;

      cmhCC_notifyTTY (CTTY_NEG_Grant, cmhCC_getTTYtrx_state (ttyAction));

      return AT_CMPL; 
    }
    else
    {
      /*
       * If the active call is a normal GSM call then its a wrong bearer cap
       */
      TRACE_EVENT_P1 ("TTY wrong BCAP: %d", (int)bc->bearer_serv);
      return (AT_FAIL);
    }
  }
  else
  {
    /*
     * If no active call then its a static switching set the bearer cap
     *  for next and subsequent calls
     */
    TRACE_EVENT ("sAT_PercentCTTY() No active call found ");
    
    if ((mode EQ CTTY_MOD_Disable) OR (mode EQ CTTY_MOD_Enable))
    {
      cmhPrm[srcId].ccCmdPrm.CTTYmode = mode;
    }
    switch (req)
    {
    case CTTY_REQ_Off:
      ccShrdPrm.ctmReq = MNCC_CTM_DISABLED;
      ccShrdPrm.ttyCmd = (UBYTE)TTY_OFF;
      psaCC_Config ();
      break;
    case CTTY_REQ_On:
      ccShrdPrm.ctmReq = MNCC_CTM_ENABLED;
      ccShrdPrm.ttyCmd = (UBYTE)TTY_ALL;
      psaCC_Config ();
      break;
    case CTTY_REQ_HCO:
      ccShrdPrm.ctmReq = MNCC_CTM_ENABLED;
      ccShrdPrm.ttyCmd = (UBYTE)TTY_HCO;
      psaCC_Config ();
      break;
    case CTTY_REQ_VCO:
      ccShrdPrm.ctmReq = MNCC_CTM_ENABLED;
      ccShrdPrm.ttyCmd = (UBYTE)TTY_VCO;
      psaCC_Config ();
      break;
    default:
      ACI_ERR_DESC (ACI_ERR_CLASS_Ext, EXT_ERR_Parameter);
      return AT_FAIL;
    }
    if (req EQ CTTY_REQ_Off)
    {
      if (ccShrdPrm.ctmState EQ TTY_STATE_IDLE)
      {
        ccShrdPrm.ctmState = TTY_STATE_NONE;
      }
    }
    else
    {
      if (ccShrdPrm.ctmState EQ TTY_STATE_NONE)
      {
        ccShrdPrm.ctmState = TTY_STATE_IDLE;
      }
    }
    return AT_CMPL;
  }
#else
  ACI_ERR_DESC (ACI_ERR_CLASS_Cme, CME_ERR_OpNotSupp);
  return AT_FAIL;
#endif  /* FF_TTY */
}

#ifdef DTI
#if defined (FF_WAP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_PercentWAP           |
+--------------------------------------------------------------------+

  PURPOSE : set next call as a WAP call / unsent WAP call
*/

GLOBAL T_ACI_RETURN sAT_PercentWAP ( T_ACI_CMD_SRC srcId , SHORT setflag )
{
  TRACE_FUNCTION("sAT_PercentWAP()");

  switch (setflag)
  {
  case (0): /* unsent WAP call */
    /* End of processing of WAP entities              */
    /* WAP-dedicated variables shall be reinitialized */
    wapId     = NO_ENTRY;
    Wap_Call  = FALSE;
    wap_state = Wap_Not_Init;
    if ( wap_dti_id NEQ DTI_DTI_ID_NOTPRESENT )
    {
      dti_cntrl_erase_entry (wap_dti_id);
      wap_dti_id = DTI_DTI_ID_NOTPRESENT;
    }
    break;

  case (1): /* next call will be a WAP call */
    if (!Wap_Call)
    {
      Wap_Call = TRUE;
#ifdef FF_PPP
      /* FST: not possible in the moment -> see #ifdef before function definition */
      
      /* reset is_PPP_CALL */
      pppShrdPrm.is_PPP_CALL = FALSE;
#endif /* FF_PPP */
    }
    else
    {
      TRACE_EVENT("ERROR: a WAP Call is currently in progress");
      return AT_FAIL;
    }
    break;
  }

  return AT_CMPL;
}
#endif /* of WAP || FF_GPF_TCPIP || SAT E */
#endif /* DTI */

#if defined MFW AND defined TI_PS_FF_AT_P_CMD_MMITEST
/* MMI TEST AT COMMAND */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_PercentMMITEST       |
+--------------------------------------------------------------------+

  PURPOSE : This command has been introduced in order to use the AT command interface for some MMI
 specific testing. It shoudnt be compiled without MMI.

*/


GLOBAL T_ACI_RETURN sAT_PercentMMITEST(T_ACI_CMD_SRC srcId, char *param)
{

  TRACE_FUNCTION ("sAT_PercentMMITEST ()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  rAT_PercentMMITEST(param);

  return AT_CMPL;
}  /* sAT_PercentMMITEST */

#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_CCS            |
| STATE   : code                        ROUTINE : rdlPrm_init        |
+--------------------------------------------------------------------+

  PURPOSE : initializing of redial parameter
*/
GLOBAL void rdlPrm_init(void)
{  
  TRACE_FUNCTION ("rdlPrm_init ()");

#ifdef TI_PS_FF_AT_P_CMD_RDLB
  rdlPrm.rdlBlN =   NO_NOTIF_USER;
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
  rdlPrm.rdlcId =   NO_ENTRY;
  rdlPrm.rdlMod =   AUTOM_REPEAT_OFF;
  rdlPrm.rdlModN =  NO_NOTIF_USER;
  rdlPrm.rdlBlMod = BLMODE_NO_PRESENT;


#if defined(_TARGET_)
  cmhCC_rd_mode_FFS(AUTOM_REP_NOT_PRESENT,READ_RDLmode); /* read redial mode from FFS */
  if(rdlPrm.rdlMod EQ AUTOM_REPEAT_ON)
  {
#ifdef TI_PS_FF_AT_P_CMD_RDLB
    if(cc_blacklist_ptr EQ NULL)
    {
      ACI_MALLOC(cc_blacklist_ptr,sizeof(T_ACI_CC_REDIAL_BLACKL));
      memset(cc_blacklist_ptr, 0 , sizeof(T_ACI_CC_REDIAL_BLACKL));
    }
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
  }
#endif /* _TARGET_ */  
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_CCS            |
| STATE   : code                        ROUTINE : rdlPrm_exit        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rdlPrm_exit(void)
{  
  TRACE_FUNCTION ("rdlPrm_exit ()");

#ifdef TI_PS_FF_AT_P_CMD_RDLB
  rdlPrm.rdlBlN =   NO_NOTIF_USER;
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
  rdlPrm.rdlcId =   NO_ENTRY;
  rdlPrm.rdlMod =   AUTOM_REP_NOT_PRESENT;
  rdlPrm.rdlModN =  NO_NOTIF_USER;
  rdlPrm.rdlBlMod = BLMODE_NO_PRESENT;

#ifdef TI_PS_FF_AT_P_CMD_RDLB
  if(cc_blacklist_ptr NEQ NULL)
  {
    ACI_MFREE(cc_blacklist_ptr);
  }
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_CCS            |
| STATE   : code                        ROUTINE : sAT_PercentRDL  |
+--------------------------------------------------------------------+

  PURPOSE : This command has been introduced in order to set the 
            redial mode and notification state referring to outgoing 
            calls on ON/OFF
*/
GLOBAL T_ACI_RETURN sAT_PercentRDL(T_ACI_CMD_SRC srcId, 
                                      T_ACI_CC_REDIAL_MODE redial_mode,
                                      T_ACI_CC_REDIAL_NOTIF notification)
{ 
 int i;

  TRACE_FUNCTION ("sAT_PercentRDL ()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    return( AT_FAIL );
  }

  if(redial_mode EQ AUTOM_REP_NOT_PRESENT)
  {
    redial_mode = rdlPrm.rdlMod;
  }
  
  switch(notification)
  {
    case NOTIF_NO_PRESENT:
      notification = rdlPrm.rdlModN;
      break;
    case NOTIF_USER:
    case NO_NOTIF_USER:      
      rdlPrm.rdlModN = notification;
      break;
    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL ); 
  }
  
  switch(redial_mode)
  {
      case AUTOM_REPEAT_OFF:
#if defined(_TARGET_)
        /* store redial mode in FFS */
        cmhCC_rd_mode_FFS(redial_mode,WRITE_RDLmode);
#endif /* _TARGET_ */
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
          if(satShrdPrm.dur)
          {
            TIMERSTOP(ACI_SAT_MAX_DUR_HND);
            satShrdPrm.dur  = -1;
          }
#endif /* SIM_TOOLKIT */
          if(rdlPrm.rdlcId NEQ -1)
          {/* clear call id entry in call table if marked as used */
            psaCC_FreeCtbNtry (rdlPrm.rdlcId);
            for(i = 0; i < CMD_SRC_MAX; i++)
            {
              R_AT(RAT_RDL, (T_ACI_CMD_SRC)i)(REDIAL_STOP);
            }
          }
        }
        /* reset redial parameter */
        rdlPrm_init();
        return AT_CMPL;        
      case AUTOM_REPEAT_ON:
        rdlPrm.rdlMod = redial_mode;
#if defined(_TARGET_)
        /* store redial mode in FFS */
        cmhCC_rd_mode_FFS(redial_mode,WRITE_RDLmode);
#endif /* _TARGET_ */
#ifdef TI_PS_FF_AT_P_CMD_RDLB        
        /* allocate blacklist for phone numbers forbidden to call */
        if(cc_blacklist_ptr EQ NULL)
        {
          /* Check if we have enough RAM for the following ACI_MALLOC */
          USHORT free, alloc;
          int ret;
          ret = vsi_m_status ( hCommACI,
                               sizeof(T_ACI_CC_REDIAL_BLACKL),
                               PRIM_POOL_PARTITION,
                               &free,
                               &alloc );
          if (ret EQ VSI_ERROR || free EQ 0)
          {
            ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_MemFull);
            return (AT_FAIL);
          }
          ACI_MALLOC(cc_blacklist_ptr,sizeof(T_ACI_CC_REDIAL_BLACKL));
          memset(cc_blacklist_ptr, 0 , sizeof(T_ACI_CC_REDIAL_BLACKL));
        }
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
        return AT_CMPL;
      default:
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );      
  }  
}

#ifdef TI_PS_FF_AT_P_CMD_RDLB
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)             MODULE  : CMH_CCS              |
| STATE   : code                      ROUTINE : sAT_PercentRDLB  |
+--------------------------------------------------------------------+

  PURPOSE : This command has been introduced in order to delete the 
            black list entries  (forbidden outgoing phone numbers)
*/
GLOBAL T_ACI_RETURN sAT_PercentRDLB(T_ACI_CMD_SRC srcId, 
                                        T_ACI_CC_REDIAL_BLMODE blacklist_mode,
                                        T_ACI_CC_REDIAL_NOTIF notification)
{    
  TRACE_FUNCTION ("sAT_PercentRDLB ()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    return( AT_FAIL );
  }
 
  if(rdlPrm.rdlMod EQ AUTOM_REPEAT_ON)
  {
  
    switch(blacklist_mode)
    {
      case BLMODE_NO_PRESENT:
        rdlPrm.rdlBlMod = BL_NO_DELETE; /* if no parameter is given the black list is not deleted */
        break;
      case BL_NO_DELETE:
      case BL_DELETE:
      /* store black list mode */
        rdlPrm.rdlBlMod = blacklist_mode;
        break;
      default:
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
    }
  
    /* store black list notification mode */
    switch(notification)
    {
      case NOTIF_NO_PRESENT:
        notification = rdlPrm.rdlBlN;
        break;
      case NOTIF_USER:
      case NO_NOTIF_USER:      
        rdlPrm.rdlBlN = notification;
        break;
      default:
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL ); 
    } 
     /* reset black list entries */
    if(rdlPrm.rdlBlMod EQ BL_DELETE)
    {
      cc_blacklist_ptr->blCount = 0;
    }
    return AT_CMPL;   
  }
  else
  {/* redial mode switched OFF */
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }
}
#endif /* TI_PS_FF_AT_P_CMD_RDLB */

#ifdef TI_PS_FF_AT_P_CMD_CUSCFG
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCQ                  |
| STATE   : code                  ROUTINE : sAT_PercentCUSCFG          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the AT%CUSCFG set command
            which sets the customization state of the facility specified.

*/
GLOBAL T_ACI_RETURN sAT_PercentCUSCFG(T_ACI_CMD_SRC srcId, 
                                        T_ACI_CUSCFG_FAC facility,
                                        T_ACI_CUSCFG_MOD mode,
                                        CHAR  *        value)
{
  TRACE_FUNCTION ("sAT_PercentCUSCFG ()");

  /*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  switch(facility)
  {
  case CUSCFG_FAC_MO_SM_Control:
    cuscfgParams.MO_SM_Control_SIM = mode;
    break;

  case CUSCFG_FAC_MO_Call_Control:
    cuscfgParams.MO_Call_Control_SIM = mode;
    break;

  case CUSCFG_FAC_MO_SS_Control:
    cuscfgParams.MO_SS_Control_SIM = mode;
    break;

  case CUSCFG_FAC_MO_USSD_Control:
    cuscfgParams.MO_USSD_Control_SIM = mode;
    break;

  case CUSCFG_FAC_2_Digit_Call:
    cuscfgParams.Two_digit_MO_Call = mode;
    break;

  case CUSCFG_FAC_Ext_USSD_Res:
    cuscfgParams.Ext_USSD_Response = mode;
    break;

  case CUSCFG_FAC_T_MOBILE_Eons:
    cuscfgParams.T_MOBILE_Eons  = mode;
    break;

  case CUSCFG_FAC_USSD_As_MO_Call:
    cuscfgParams.USSD_As_MO_Call = mode;
    break;

  default:
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }
  return(AT_CMPL);
}
#endif /* TI_PS_FF_AT_P_CMD_CUSCFG */

/* Implements Measure 80 */
/*
+------------------------------------------------------------------------------
|  Function    : cmhCC_check_RedialCall
+------------------------------------------------------------------------------
|  Purpose     : Checks for a Redial Call. 
|
|  Parameters  : at_cmd_id - AT Command Identifier.
|                            (AT_CMD_H OR AT_CMD_CHUP)
|
|  Return      : BOOL
+------------------------------------------------------------------------------
*/

LOCAL BOOL cmhCC_check_RedialCall( T_ACI_AT_CMD at_cmd_id )
{
  SHORT cId;
  int i;
  
  TRACE_FUNCTION ("cmhCC_check_RedialCall()");
  
  cId = psaCC_ctbFindCall( OWN_SRC_INV, CS_ACT_REQ, CT_MOC_RDL);

  if( cId NEQ NO_ENTRY )
  {
#ifdef SIM_TOOLKIT
    if( ( ccShrdPrm.ctb[cId]->SATinv & SAT_REDIAL ) )
    { /* This is the call invoked by SAT */
      T_ACI_SAT_TERM_RESP resp_data;
      psaSAT_InitTrmResp( &resp_data );
      psaSAT_SendTrmResp( RSLT_USR_CLR_DWN, &resp_data );
    }
#endif /* SIM_TOOLKIT */

    ccShrdPrm.ctb[cId]->calType = CT_MOC;

    if(rdlPrm.rdlMod EQ AUTOM_REPEAT_ON)
    {
      for(i = 0; i < CMD_SRC_MAX; i++)
      {
        R_AT(RAT_RDL, (T_ACI_CMD_SRC)i)(REDIAL_STOP);
      }
      rdlPrm.rdlcId = NO_ENTRY;
    }
    if (at_cmd_id EQ AT_CMD_H)
    {
      ccShrdPrm.ctb[cId]->nrmCs  = MNCC_CAUSE_CALL_CLEAR;
      psaCC_ClearCall (cId);
      psaCC_FreeCtbNtry (cId);
      return( TRUE );
    } 
  }
  else
  {/* redial timer is running */
    if ((rdlPrm.rdlMod EQ AUTOM_REPEAT_ON) AND (rdlPrm.rdlcId NEQ NO_ENTRY))
    {
#ifdef SIM_TOOLKIT
        if( ( ccShrdPrm.ctb[rdlPrm.rdlcId]->SATinv & SAT_REDIAL ) )
        { /* This is the call invoked by SAT */
          T_ACI_SAT_TERM_RESP resp_data;

          psaSAT_InitTrmResp( &resp_data );
          psaSAT_SendTrmResp( RSLT_USR_CLR_DWN, &resp_data );
        }
        /* Stop the SAT maximum duration timer */
        if(satShrdPrm.dur)
        {
          TIMERSTOP(ACI_SAT_MAX_DUR_HND);
          satShrdPrm.dur  = -1;
        }
#endif /* SIM_TOOLKIT */
      /* clear call id entry in call table if marked as used */
      psaCC_FreeCtbNtry (rdlPrm.rdlcId);

      for(i = 0; i < CMD_SRC_MAX; i++)
      {
        R_AT(RAT_RDL, (T_ACI_CMD_SRC)i)(REDIAL_STOP);
      }
      /* reset some redial parameter */
      rdlPrm.rdlcId =   NO_ENTRY;
      return( TRUE );
    }
  }
  return( FALSE );
}

/*
+------------------------------------------------------------------------------
|  Function    : cmhCC_check_pending_satCall()
+------------------------------------------------------------------------------
|  Purpose     : Checks for a Redial Call. 
|
|  Parameters  : None
|
|  Return      : BOOL
+------------------------------------------------------------------------------
*/
LOCAL BOOL cmhCC_check_pending_satCall( )
{
  SHORT cId;                  /* holds call id */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  T_ACI_CLOG     cmdLog;      /* holds logging info */
#endif
  /* triggered by SETUP CALL command */
  cId = psaCC_ctbFindCall( OWN_SRC_INV, CS_SAT_REQ, NO_VLD_CT );
#ifdef FF_SAT_E
  if( cId EQ NO_ENTRY )
  {
    /* triggered by OPEN CHANNEL command */
    cId = psaCC_ctbFindCall( OWN_SRC_INV, CS_SAT_CSD_REQ, NO_VLD_CT );
  }
#endif
  if( cId NEQ NO_ENTRY )
  {
    if( ( ccShrdPrm.ctb[cId]->SATinv & SAT_REDIAL ) )
    { /* This is the call invoked by SAT */
      T_ACI_SAT_TERM_RESP resp_data;
      psaSAT_InitTrmResp( &resp_data );
      psaSAT_SendTrmResp( RSLT_USR_CLR_DWN, &resp_data );
    }
    if( psaCC_ctb(cId)->SATinv ) 
      cmhSAT_UserRejCall(psaCC_ctb(cId)->calStat);

#if defined SMI OR defined MFW OR defined FF_MMI_RIV
    cmdLog.cId = cId+1;
    cmdLog.retCode = AT_CMPL;
    rAT_PercentCLOG( &cmdLog );
#endif
    psaCC_FreeCtbNtry (cId);
    return( TRUE );
  }
  #if defined (GPRS) AND defined (FF_SAT_E) AND defined (DTI)
  /* check for pending SAT GPRS context */
  else
  {
    if( cmhSAT_OpChnGPRSPend(PDP_CONTEXT_CID_INVALID, OPCH_WAIT_CNF))
    {
      cmhSAT_UserRejCntxt();

#if defined SMI OR defined MFW OR defined FF_MMI_RIV
      cmdLog.cId = cId+1;
      cmdLog.retCode = AT_CMPL;
      rAT_PercentCLOG( &cmdLog );
#endif
      return( TRUE );
    }
  }
#endif  /* GPRS AND FF_SAT_E */
  psaCC_FreeCtbNtry (cId);
  return( FALSE );
}

/* Implements Measure 181 and 182 */
/*
+------------------------------------------------------------------------------
|  Function    : chld_HoldActiveCalls
+------------------------------------------------------------------------------
|  Purpose     : Puts All Active Calls on Hold.
|
|  Parameters  : srcId - AT command source identifier.
|                mptyHldFlg - Points to a Flag that Helds a Call 
|                hldCalFlg  - Points to a Flag that Helds a MultiParty Call
|                cId       - context Id 
|
|  Return      : BOOL
+------------------------------------------------------------------------------
*/

LOCAL BOOL chld_HoldActiveCalls( T_ACI_CMD_SRC srcId,
                                 BOOL *mptyHldFlg, BOOL *hldCalFlg, SHORT *cId)
{
  T_CC_CMD_PRM *pCCCmdPrm;
  
  TRACE_FUNCTION("chld_HoldActiveCalls()");

  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;
  pCCCmdPrm -> mltyCncFlg = 0;
  pCCCmdPrm -> mltyDscFlg = 0;

  for( *cId = 0; *cId < MAX_CALL_NR; (*cId)++ )
  {
    if (ccShrdPrm.ctb[*cId] NEQ NULL AND
        ccShrdPrm.ctb[*cId]->calStat    EQ CS_ACT AND
        ccShrdPrm.ctb[*cId]->curCmd     EQ AT_CMD_NONE)
    {
      if( cmhCC_getcalltype(*cId) NEQ VOICE_CALL )
      {
        pCCCmdPrm -> mltyCncFlg = 0;
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_CallTypeNoHold );
        return( FALSE ); /* no data calls supported */
      }

      if( (ccShrdPrm.ctb[*cId]->mptyStat EQ CS_IDL) OR
          ((ccShrdPrm.ctb[*cId]->mptyStat EQ CS_ACT) AND ((*mptyHldFlg) EQ FALSE)))
      {
        *hldCalFlg = TRUE;
      }

      /* if active call is a multiparty call */
      if( ccShrdPrm.ctb[*cId]->mptyStat EQ CS_ACT )
      {
        *mptyHldFlg = TRUE;
      }

      cmhCC_HoldCall(*cId, srcId, AT_CMD_CHLD);
    }
  }
  return( TRUE );
}

/* Implements Measure 117 */
/*
+------------------------------------------------------------------------------
|  Function    : chld_Rel_MultipartySpec
+------------------------------------------------------------------------------
|  Purpose     : Releases a Specific Multi Party Calls
|
|  Parameters  : srcId - AT command source identifier.
|                spcId - Holds specified call id.
|                chld_mode - CHLD Command Mode.
|                            (CHLD_MOD_RelAnySpec OR CHLD_MOD_RelActSpec)
|                call
|
|  Return      : T_ACI_RETURN (ACI Functional Return Codes)
+------------------------------------------------------------------------------
*/

LOCAL T_ACI_RETURN chld_Rel_MultipartySpec( T_ACI_CMD_SRC srcId, SHORT *spcId,
                                            T_ACI_CHLD_MOD chld_mode,
                                            CHAR *call )
{
  T_CC_CMD_PRM *pCCCmdPrm;
  SHORT cId;
  SHORT dscId = -1;

  TRACE_FUNCTION( "chld_Rel_MultypartySpec()" );

  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;

  /* get specific call id */
  if( call EQ NULL )
  {
    TRACE_ERROR("CALL parameter is needed !!!");
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );  /* no call specified */
  }

  *spcId = atoi( call );

  if( (*spcId) EQ 0 OR (*spcId) > MAX_CALL_NR )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  (*spcId)--;  /* adapt to call table index */

  pCCCmdPrm -> mltyCncFlg = 0;
  pCCCmdPrm -> mltyDscFlg = 0;

  if( (*spcId) EQ -1 )
  {
    for( cId = 0; cId < MAX_CALL_NR; cId++ )
    {
      if (ccShrdPrm.ctb[cId] NEQ NULL AND
          psaCC_ctb(cId)->mptyStat   EQ CS_ACT AND
          psaCC_ctb(cId)->curCmd     EQ AT_CMD_NONE)
      {
        psaCC_StopDTMF (cId);

        cmhCC_ClearCall ( cId, MNCC_CAUSE_CALL_CLEAR, srcId, AT_CMD_CHLD, NULL );
        dscId = cId;
      }
    }

    if( dscId NEQ -1 )
    {
      pCCCmdPrm -> CHLDmode = chld_mode;
      CHLDaddInfo           = NO_CHLD_ADD_INFO;

      /* inform MFW */
      chld_ratlog( srcId, pCCCmdPrm->CHLDmode, call, CHLD_ACT_ReleaseMpty, dscId, AT_EXCT );
      return( AT_EXCT );
    }
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return(AT_FAIL);
  }
  return(AT_CMPL);
}

/* Implements Measure 164 and 165 */
/*
+------------------------------------------------------------------------------
|  Function    : cmhCC_disconnect_waiting_call
+------------------------------------------------------------------------------
|  Purpose     : Disconnect a waiting call with user determined user busy
|
|  Parameters  : waitId     - Holds call waiting id.
|                at_cmd_id  - AT Command Identifier.
|                             (AT_CMD_H/AT_CMD_CHUP/AT_CMD_Z)
|                mltyDscFlg - Flag for MultiCall Operation
|
|  Return      : void
+------------------------------------------------------------------------------
*/

LOCAL void cmhCC_disconnect_waiting_call( T_ACI_CMD_SRC srcId,
                                          SHORT waitId,
                                          T_ACI_AT_CMD at_cmd_id,
                                          USHORT *mltyDscFlg )
{
  UBYTE          idx;
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  T_ACI_CLOG     cmdLog;
#endif

  TRACE_FUNCTION ("cmhCC_disconnect_waiting_call()");

  cmhCC_flagCall( waitId, mltyDscFlg );
  ccShrdPrm.ctb[waitId]->nrmCs  = MNCC_CAUSE_USER_BUSY;
  ccShrdPrm.ctb[waitId]->curCmd = at_cmd_id;
  ccShrdPrm.ctb[waitId]->curSrc = srcId;
  psaCC_ClearCall (waitId);

#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  cmdLog.cId = waitId+1;
  rAT_PercentCLOG( &cmdLog );
#endif

#ifdef FF_ATI
  io_setRngInd (IO_RING_OFF, CRING_SERV_TYP_NotPresent, CRING_SERV_TYP_NotPresent ); /* V.24 Ring Indicator Line */
#endif
  for( idx = 0; idx < CMD_SRC_MAX; idx++ )
  {
    R_AT( RAT_CRING_OFF, (T_ACI_CMD_SRC)idx )( waitId+1 );
  }
}

/* Implements Measure 64 */
/*
+------------------------------------------------------------------------------
|  Function    : cmhCC_CheckRedialTimer
+------------------------------------------------------------------------------
|  Purpose     : Checks the Redial Timer and Stops it, if neccessary
|
|  Parameters  : sim_toolkit_enable 
|
|  Return      : void
+------------------------------------------------------------------------------
*/

GLOBAL void cmhCC_CheckRedialTimer( BOOL sim_toolkit_enable )
{
  int i;

  TRACE_FUNCTION ( "cmhCC_CheckRedialTimer()" );

  /* stop redialling timer if necessary */
  if (rdlPrm.rdlcId NEQ NO_ENTRY)
  {
    TIMERSTOP(ACI_REPEAT_HND);

#ifdef SIM_TOOLKIT
    if( sim_toolkit_enable )
    {
      if( ( ccShrdPrm.ctb[rdlPrm.rdlcId]->SATinv & SAT_REDIAL ) )
      { /* This is the call invoked by SAT */
        T_ACI_SAT_TERM_RESP resp_data;

        psaSAT_InitTrmResp( &resp_data );
        psaSAT_SendTrmResp( RSLT_USR_CLR_DWN, &resp_data );
      }
      /* Stop SAT maximum redial duration timer */
      if(satShrdPrm.dur)
      {
        TIMERSTOP(ACI_SAT_MAX_DUR_HND);
        satShrdPrm.dur  = -1;
      }
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

}

#ifdef TI_PS_FF_AT_CMD_P_ECC
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_PercentECC           |
+--------------------------------------------------------------------+
|  Description  : This is the functional counterpart to the AT%ECC 
|                 set command which sets additional ECC to local array
|                  in ACI
|  Parameters   : <srcId>     : Source Id
|                 <index>     : Index
|                 <ecc_numer> : ECC number to be stored
|
|  Return       : T_ACI_RETURN
+---------------------------------------------------------------------
*/

GLOBAL T_ACI_RETURN sAT_PercentECC( T_ACI_CMD_SRC srcId,
                                    U8 index,
                                    char *ecc_number)
{
  TRACE_FUNCTION ("sAT_PercentECC ()");

  /*
   *-------------------------------------------------------------------
   * check command source
   *-------------------------------------------------------------------
   */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }
  /*
   *-------------------------------------------------------------------
   * check for valid index
   *-------------------------------------------------------------------
   */
  if (index >= ADDITIONAL_ECC_NUMBER_MAX)
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme, CME_ERR_InvIdx);
    return( AT_FAIL );
  }
  /*
   *-------------------------------------------------------------------
   * check for valid length of ECC number
   *-------------------------------------------------------------------
   */
  if ((!strlen(ecc_number)) OR (strlen(ecc_number) > ADDITIONAL_ECC_NUMBER_LENGTH)) 
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme, CME_ERR_TxtToLong);
    return( AT_FAIL );
  }
  
  strncpy(additional_ecc_numbers[index], ecc_number, strlen(ecc_number));
  return(AT_CMPL);
}

/*
+-----------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                                 |
| STATE   : code                  ROUTINE : cmhCC_additional_ecc_numbers_initialize |
+-----------------------------------------------------------------------------------+
|  Description  : Initializes array which stores ECC numbers to default values
|
|  Parameters   : void
|  Return       : void
+------------------------------------------------------------------------------------
*/

GLOBAL void cmhCC_additional_ecc_numbers_initialize(void)
{
  TRACE_FUNCTION("cmhCC_additional_ecc_numbers_initialize()");
  //memset(additional_ecc_numbers, 0x00, ADDITIONAL_ECC_NUMBER_MAX*(ADDITIONAL_ECC_NUMBER_LENGTH));
  memset(additional_ecc_numbers, 0x00, ADDITIONAL_ECC_NUMBER_MAX*(ADDITIONAL_ECC_NUMBER_LENGTH+1)); //OMAPS00117704/OMAPS00117705
}

/*
+---------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                         |
| STATE   : code                  ROUTINE : cmhCC_isNrInAdditionalECC       |
+---------------------------------------------------------------------------+
|  Description  : Checks the passed number with additional ECC numbers
|
|  Parameters   : <number> : Dialed number
|  Return       : BOOL
+----------------------------------------------------------------------------
*/

GLOBAL BOOL cmhCC_isNrInAdditionalECC(char *number)
{
  U8 idx;

  TRACE_FUNCTION("cmhCC_isNrInAdditionalECC()");

  for (idx=0; idx < ADDITIONAL_ECC_NUMBER_MAX; idx++) 
  {
    if (!strcmp(number, additional_ecc_numbers[idx]))
    {
      return (TRUE);
    }
  }
  return (FALSE);
}
#endif /* TI_PS_FF_AT_CMD_P_ECC */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_PlusCVHU             |
+--------------------------------------------------------------------+

  PURPOSE : This is a set call for +CVHU for the control of the voice 
  hangup
  
*/
GLOBAL T_ACI_RETURN sAT_PlusCVHU ( T_ACI_CMD_SRC srcId, T_ACI_CVHU_MODE mode)
{
  TRACE_FUNCTION("sAT_PlusCVHU()");

  /*
   *-------------------------------------------------------------------
   * check command source
   *-------------------------------------------------------------------
   */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  switch(mode)
  {
  case CVHU_DropDTR_IGNORED:
  case CVHU_DropDTR_ATH_IGNORED:
  case CVHU_DropDTR_Same_AndD:
    ccShrdPrm.cvhu = mode;
    return(AT_CMPL);
  default:
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }
}
/*==== EOF ========================================================*/
