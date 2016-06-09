/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_CCQ
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
|  Purpose :  This module provides the query functions related to the
|             protocol stack adapter for mobility management.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_CCQ_C
#define CMH_CCQ_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

/*==== INCLUDES ===================================================*/

#include "aci_all.h"
#include "aci_cmh.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "psa.h"
#include "psa_cc.h"
#include "cmh.h"
#include "cmh_cc.h"

#include "aoc.h"
#include "audio.h"

#include "dti_conn_mng.h"
#include "psa_sim.h"
#include "cmh_sim.h"
#include "psa_mm.h"

/*==== CONSTANTS ==================================================*/
#ifdef TI_PS_FF_AT_P_CMD_RDLB
EXTERN T_ACI_CC_REDIAL_BLACKL * cc_blacklist_ptr;
#endif /* TI_PS_FF_AT_P_CMD_RDLB */

const UBYTE aci_clcc_dir [5][2] = {
{ NO_VLD_CT, NO_VLD_CT },
{ CLCC_DIR_MOC, CLCC_DIR_MOC },
{ CLCC_DIR_MTC, CLCC_DIR_MTC },
{ CLCC_DIR_MOC, CLCC_DIR_MOC_NI },
{ CLCC_DIR_MOC, CLCC_DIR_MOC_RDL }
};

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/
#ifdef TI_PS_FF_AT_P_CMD_CUSCFG
EXTERN T_ACI_CUSCFG_PARAMS cuscfgParams;
#endif /* TI_PS_FF_AT_P_CMD_CUSCFG */

GLOBAL T_PCEER causeMod = P_CEER_mod;     /* Hold which module caused the extended error */
GLOBAL SHORT causeCeer;      /* Hold extended error for sim, mm and ss */
EXTERN UBYTE std;

/*==== FUNCTIONS ==================================================*/
LOCAL T_ACI_RETURN qAT_plus_percent_CLCC( T_ACI_CMD_SRC srcId,
                                          T_ACI_CLCC_CALDESC *calLst,
                                          T_ACI_AT_CMD at_cmd_id,
                                          SHORT rdlcId );

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCSTA             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CSTA? AT command
            which returns the current setting for type of address.

            <toa>:   type of address.
*/

GLOBAL T_ACI_RETURN qAT_PlusCSTA ( T_ACI_CMD_SRC srcId,
                                   T_ACI_TOA * toa )
{
  T_CC_CMD_PRM * pCCCmdPrm; /* points to CC command parameters */

  TRACE_FUNCTION ("qAT_PlusCSTA()");

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
 * fill in parameters
 *-------------------------------------------------------------------
 */
  *toa = pCCCmdPrm -> CSTAtoa;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCMOD             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CMOD? AT command
            which returns the current setting for call mode.

            <mode>:   call mode.
*/

GLOBAL T_ACI_RETURN qAT_PlusCMOD ( T_ACI_CMD_SRC srcId,
                                   T_ACI_CMOD_MOD* mode )
{
  TRACE_FUNCTION ("qAT_PlusCMOD()");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }


  /* fill in parameters */
  *mode = ccShrdPrm.CMODmode;

  return( AT_CMPL );
}

#ifdef FAX_AND_DATA
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCBST             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CBST? AT command
            which returns the current setting for bearer service type.

*/

GLOBAL T_ACI_RETURN qAT_PlusCBST ( T_ACI_CMD_SRC srcId,
                                   T_ACI_BS_SPEED* speed,
                                   T_ACI_CBST_NAM* name,
                                   T_ACI_CBST_CE* ce)
{
  TRACE_FUNCTION ("qAT_PlusCBST()");

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
 * fill in parameters
 *-------------------------------------------------------------------
 */
  *speed = ccShrdPrm.CBSTspeed;
  *name  = ccShrdPrm.CBSTname;
  *ce    = ccShrdPrm.CBSTce;

  return( AT_CMPL );
}
#endif /* FAX_AND_DATA */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCCUG             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CCUG AT command
            which is responsible to query the parameters for closed user
            group supplementary services.

            <mode>   : CUG mode.
            <index>  : CUG index.
            <info>   : CUG info.
*/

GLOBAL T_ACI_RETURN qAT_PlusCCUG  ( T_ACI_CMD_SRC   srcId,
                                    T_ACI_CCUG_MOD  *mode,
                                    T_ACI_CCUG_IDX  *index,
                                    T_ACI_CCUG_INFO *info)
{
  T_CC_CMD_PRM * pCCCmdPrm; /* points to SS command parameters */

  TRACE_FUNCTION ("qAT_PlusCCUG()");

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
 * fill in parameters
 *-------------------------------------------------------------------
 */
  *mode  = pCCCmdPrm -> CCUGmode;
  *index = pCCCmdPrm -> CCUGidx;
  *info  = pCCCmdPrm -> CCUGinfo;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCLCC             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CLCC? AT command
            which returns the current call status.

*/

GLOBAL T_ACI_RETURN qAT_PlusCLCC  ( T_ACI_CMD_SRC       srcId,
                                    T_ACI_CLCC_CALDESC *calLst)
{
  /* Implements Measure 47 and 216 */
  return ( qAT_plus_percent_CLCC(srcId, calLst, AT_CMD_CLCC, rdlPrm.rdlcId) );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCQ                                        |
| STATE   : code                  ROUTINE : qAT_PercentCLCC                                     |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CLCC? AT command
            which returns the current call status.

*/

GLOBAL T_ACI_RETURN qAT_PercentCLCC  ( T_ACI_CMD_SRC       srcId,
                                    T_ACI_CLCC_CALDESC *calLst)
{
  /* Implements Measure 47 and 216 */
  return ( qAT_plus_percent_CLCC(srcId, calLst, AT_CMD_P_CLCC, NO_ENTRY) );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCEER             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CEER AT command
            which returns an extended error report.

*/

GLOBAL T_ACI_RETURN qAT_PlusCEER  ( T_ACI_CMD_SRC    srcId,
                                    USHORT          *cause)
{
  T_CC_CALL_TBL *ctbFail = ccShrdPrm.ctb[ccShrdPrm.cIdFail];

  TRACE_FUNCTION ("qAT_PlusCEER()");

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
 * get error report
 *-------------------------------------------------------------------
 */

  /* Extended error report to indicate attach related problems */
  if ((causeCeer NEQ CEER_NotPresent) AND (causeMod EQ P_CEER_mm))
  {
    *cause = CAUSE_MAKE(DEFBY_CONDAT, ORIGSIDE_NET, MM_ORIGINATING_ENTITY, causeCeer);
    return( AT_CMPL );
  }
  if ((causeCeer NEQ CEER_NotPresent) AND (causeMod EQ P_CEER_ss))
  {
    *cause = CAUSE_MAKE(DEFBY_CONDAT, ORIGSIDE_NET, SS_ORIGINATING_ENTITY, causeCeer);
    return( AT_CMPL );
  }
  if ((causeCeer EQ P_CEER_ACMMaxReachedOrExceeded) AND (causeMod EQ P_CEER_sim)) /* ACI cause check */
  {
    *cause = CAUSE_MAKE(DEFBY_CONDAT, ORIGSIDE_MS, MNCC_ACI_ORIGINATING_ENTITY, P_CEER_ACMMaxReachedOrExceeded);
    return( AT_CMPL );
  }

  if ((causeCeer EQ P_CEER_InvalidFDN) AND (causeMod EQ P_CEER_sim)) /* ACI cause check */
  {
    *cause = CAUSE_MAKE(DEFBY_CONDAT, ORIGSIDE_MS, MNCC_ACI_ORIGINATING_ENTITY, P_CEER_InvalidFDN );
    return( AT_CMPL );
  }

  if( ccShrdPrm.cIdFail < 0 OR ccShrdPrm.cIdFail > MAX_CALL_NR ) 
  {
    *cause = CAUSE_MAKE(DEFBY_STD, ORIGSIDE_MS, MNCC_ACI_ORIGINATING_ENTITY, NOT_PRESENT_8BIT);
    return( AT_CMPL );
  }

  /*
   * This code has to become better understood, probably it can be simplified
   * Be sure to use ctbFail here because TI compiler 1.22e may have a problem
   * otherwise.
   */
  if (ctbFail NEQ NULL)
  {
    /* Call table entry still existant */
    if (GET_CAUSE_VALUE(ctbFail->rejCs) NEQ NOT_PRESENT_8BIT)
    {
      *cause = ctbFail->rejCs;
    }
    else if (GET_CAUSE_VALUE(ctbFail->nrmCs) NEQ NOT_PRESENT_8BIT)
    {
      *cause = ctbFail->nrmCs;
    }
    else if (GET_CAUSE_VALUE(ctbFail->rslt) NEQ NOT_PRESENT_8BIT)
    {
      *cause = ctbFail->rslt;
    }
    else
    {
      /* in case network has sent no extended report */
      *cause = CAUSE_MAKE(DEFBY_STD, ORIGSIDE_MS, MNCC_ACI_ORIGINATING_ENTITY, NOT_PRESENT_8BIT);
    }
  }
  else
  {
    /* Call table entry already freed */
    *cause = ccShrdPrm.ccCs[ccShrdPrm.cIdFail];
  }
  return AT_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_PlusPAS              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CPAS AT command
            which returns the phone activity status.
*/

GLOBAL T_ACI_RETURN qAT_PlusCPAS  ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_CPAS_PAS  *pas)
{
  SHORT cId;            /* holds call id */
  T_ACI_CFUN_FUN cfun_stat;

  TRACE_FUNCTION ("qAT_PlusCPAS()");

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
 * get phone activity status
 *-------------------------------------------------------------------
 */

  /* check if power is off (+CFUN=0) */
  if ( qAT_PlusCFUN(srcId, &cfun_stat) EQ AT_FAIL )
  {
    return( AT_FAIL );
  }

  if ( cfun_stat EQ CFUN_FUN_Minimum )
  {
    *pas = CPAS_PAS_Asleep;
    return( AT_CMPL );
  }

  /* search for active call */
  cId = psaCC_ctbFindCall( OWN_SRC_INV, CS_ACT, NO_VLD_CT );

  if( cId NEQ NO_ENTRY )
  {
    *pas = CPAS_PAS_CallProg;
    return( AT_CMPL );
  }

  /* search for call on hold */
  cId = psaCC_ctbFindCall( OWN_SRC_INV, CS_HLD, NO_VLD_CT );

  if( cId NEQ NO_ENTRY )
  {
    *pas = CPAS_PAS_CallProg;
    return( AT_CMPL );
  }

  /* search for an incoming call */
  cId = psaCC_ctbFindCall( (T_OWN)CMD_SRC_NONE, CS_ACT_REQ, CT_MTC );

  if( cId NEQ NO_ENTRY )
  {
    *pas = CPAS_PAS_Ring;
    return( AT_CMPL );
  }

  /* ready to accept commands */
    *pas = CPAS_PAS_Ready;
    return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_PlusCSNS             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CSNS AT command
            which returns the single numbering scheme mode.
*/

GLOBAL T_ACI_RETURN qAT_PlusCSNS  ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_CSNS_MOD  *mode)
{

  TRACE_FUNCTION ("qAT_PlusCSNS()");

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
 * get SNS mode
 *-------------------------------------------------------------------
 */
  *mode = (T_ACI_CSNS_MOD)ccShrdPrm.snsMode;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCQ                  |
| STATE   : code                  ROUTINE : qAT_PercentCAL           |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CAL? AT command
            which returns the current call status.
            03.06.03 SKA: This function is called by MFW only. It uses
            this function to get the called number for emitting on display.
            But there is a protocoll between ACI and MFW which says, that
            the RAT_COLP signal sent by ACI to MFW at receiving of MNCC_SETUP_CNF
            to tell MFW/BMI whether it can emit the called number caused by the
            content of mncc_setup_cnf->connected_number.present sent by the network or not. 
            If present = PRES_PRES_REST, ACI does not send RAT_COLP to MFW, but in this
            function ACI did copy the number to the MFW callTable structure! 
            Now, ACI does not copy the called number in case of PRES_PRES_REST and
            MFW/BMI will emit an empty string on display. Therefore we could save
            a lot of memory and internal traffic between ACI and MFW, if we keep
            all the logic within ACI!
*/

GLOBAL T_ACI_RETURN qAT_PercentCAL( T_ACI_CMD_SRC    srcId,
                                    T_ACI_CAL_ENTR*  callTable )
{
  SHORT ctbIdx;               /* holds call table index */
  SHORT lstIdx;               /* holds call list index */

  TRACE_FUNCTION ("qAT_PercentCAL()");

/*
 *-------------------------------------------------------------------
 * for every call of the call table
 *-------------------------------------------------------------------
 */
  for( ctbIdx = 0, lstIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
  {
    if (ccShrdPrm.ctb[ctbIdx] NEQ NULL)
    {

    /*
     *---------------------------------------------------------------
     * fill in call type
     *---------------------------------------------------------------
     */
      switch( psaCC_ctb(ctbIdx)->calType )
      {
        case( CT_MOC ):
        case( CT_MOC_RDL ):
        case( CT_NI_MOC ):
          callTable[lstIdx].calType = CAL_TYPE_MOC;
          break;

        case( CT_MTC ):
          callTable[lstIdx].calType = CAL_TYPE_MTC;
          break;

        default:
          callTable[lstIdx].calType = CAL_TYPE_NotPresent;
      }

     /*
     *---------------------------------------------------------------
     * fill in call status
     *---------------------------------------------------------------
     */
      switch( psaCC_ctb(ctbIdx)->calStat )
      {
        case( CS_ACT_REQ ):
          switch( psaCC_ctb(ctbIdx)->calType )
          {
            case( CT_MOC ):
            case( CT_MOC_RDL ):
            case( CT_NI_MOC ):
              if( psaCC_ctb(ctbIdx)->alrtStat EQ AS_SND )
              {
                callTable[lstIdx].status = CAL_STAT_Alerting;
              }
              else
              {
                callTable[lstIdx].status = CAL_STAT_Dial;
              }
              break;

            case( CT_MTC ):
              if( psaCC_ctbFindCall ( (T_OWN)CMD_SRC_NONE, CS_ACT_REQ, NO_VLD_CT )
                  NEQ -1 )
              {
                callTable[lstIdx].status = CAL_STAT_Wait;
              }
              else
              {
                callTable[lstIdx].status = CAL_STAT_Incomming;
              }
              break;

            default:
              callTable[lstIdx].status = CAL_STAT_NotPresent;

          }
          break;

        case( CS_ACT ):
        case( CS_MDF_REQ ):
        case( CS_HLD_REQ ):
          callTable[lstIdx].status = CAL_STAT_Active;
          break;

        case( CS_HLD ):
          callTable[lstIdx].status = CAL_STAT_Held;
          break;

        case( CS_IDL ):
        case( CS_DSC_REQ ):
          callTable[lstIdx].status    = CAL_STAT_DeactiveReq;

          /*
           *  reset of all other values
           */
          callTable[lstIdx].mpty      = CAL_MPTY_NotPresent;
          callTable[lstIdx].type.ton  = TON_NotPresent;
          callTable[lstIdx].type.npi  = NPI_NotPresent;
          callTable[lstIdx].number[0] = '\0';
#ifdef NO_ASCIIZ
          callTable[lstIdx].alpha.cs  = CS_NotPresent;
          callTable[lstIdx].alpha.len = 0;
#else  /* #ifdef NO_ASCIIZ */
          callTable[lstIdx].alpha[0]  = '\0';
#endif /* #ifdef NO_ASCIIZ */
          callTable[lstIdx].calType   = CAL_TYPE_NotPresent;
          callTable[lstIdx].calMode   = CAL_MODE_NotPresent;
          callTable[lstIdx].calOwner  = CAL_OWN_NotPresent;


          /*
           *  indes and next entry, because of the continue statement
           */
          callTable[lstIdx].index     = ctbIdx+1;
          lstIdx++;
          continue;

        case( CS_CPL_REQ ):
          continue;
      }

    /*
     *---------------------------------------------------------------
     * fill in number of connected party
     *---------------------------------------------------------------
     */
      switch( psaCC_ctb(ctbIdx)->calType )
      {
        
        case( CT_MOC ):
        case( CT_MOC_RDL ):
        case( CT_NI_MOC ):
          /* presentation restricted FTA 31.1.4.2 procedure 1 */
          /* number not available due to interworking FTA 31.1.4.2 procedure 2 */
          if ((psaCC_ctb(ctbIdx)->clgPty.present NEQ MNCC_PRES_NOT_PRES)  /* any .present indicator received from MSC? */
          AND (psaCC_ctb(ctbIdx)->clgPty.c_num EQ 0) )               /* but no number available? */
          {
            /*
             * this prevents MFW/BMI to emit the called number, but it
             * ignores the NOT received COLP signal. Without a COLP signal
             * MFW/BMI should not emit the called number. (FTA 31.1.4.2)
             */
            callTable[lstIdx].number[0] = '\0';
            callTable[lstIdx].type.ton = TON_NotPresent;
            callTable[lstIdx].type.npi = NPI_NotPresent;
          }
          else
          {
            psaCC_ctbCldAdr2Num (ctbIdx, callTable[lstIdx].number,
                                 MAX_CC_ORIG_NUM_LEN);
            callTable[lstIdx].type.ton = (T_ACI_TOA_TON)psaCC_ctb(ctbIdx)->cldPty.ton;
            callTable[lstIdx].type.npi = (T_ACI_TOA_NPI)psaCC_ctb(ctbIdx)->cldPty.npi;
          }
          break;

        case( CT_MTC ):
          psaCC_ctbClrAdr2Num (ctbIdx, callTable[lstIdx].number,
                               MAX_CC_ORIG_NUM_LEN);
          callTable[lstIdx].type.ton = (T_ACI_TOA_TON)psaCC_ctb(ctbIdx)->clgPty.ton;
          callTable[lstIdx].type.npi = (T_ACI_TOA_NPI)psaCC_ctb(ctbIdx)->clgPty.npi;
          break;

        case( NO_VLD_CT ):
        default:
          callTable[lstIdx].number[0] = '\0';
          callTable[lstIdx].type.ton = TON_NotPresent;
          callTable[lstIdx].type.npi = NPI_NotPresent;
          break;
      }

#ifdef NO_ASCIIZ
      memcpy( ( UBYTE* ) &callTable[lstIdx].alpha,
              ( UBYTE* ) &psaCC_ctb(ctbIdx)->alphIdUni,
              sizeof( T_ACI_PB_TEXT ) );
#else  /* NO_ASCIIZ */
      memcpy( callTable[lstIdx].alpha,
              psaCC_ctb(ctbIdx)->alphIdUni.data,
              psaCC_ctb(ctbIdx)->alphIdUni.len );

      cmh_cvtFromDefGsm ( ( CHAR* ) psaCC_ctb(ctbIdx)->alphIdUni.data,
                          ( USHORT ) psaCC_ctb(ctbIdx)->alphIdUni.len,
                          callTable[lstIdx].alpha );
#endif /* NO_ASCIIZ */

    /*
     *---------------------------------------------------------------
     * fill in in-band tones setting
     *---------------------------------------------------------------
     */
      switch( psaCC_ctb(ctbIdx)->inBndTns )
      {
        case( TRUE ):
          callTable[lstIdx].ibtUse = CAL_IBT_TRUE;
          break;

        case( FALSE ):
          callTable[lstIdx].ibtUse = CAL_IBT_FALSE;
          break;
      }

  
    /*
     *---------------------------------------------------------------
     * fill in call mode
     *---------------------------------------------------------------
     */
      switch( cmhCC_getcalltype(ctbIdx) )
      {                  
        case( VOICE_CALL ):
          switch( psaCC_ctb(ctbIdx)->BC
                  [(psaCC_ctb(ctbIdx)->curBC EQ 0)?1:0].
                  bearer_serv)
          {
            case( MNCC_BEARER_SERV_ASYNC ):
              if( psaCC_ctb(ctbIdx)->rptInd EQ MNCC_RI_CIRCULAR )
                callTable[lstIdx].calMode = CAL_MODE_VAD_Voice;
              else
                callTable[lstIdx].calMode = CAL_MODE_VFD_Voice;
              break;

            case( MNCC_BEARER_SERV_FAX ):
              callTable[lstIdx].calMode = CAL_MODE_VAF_Voice;
              break;

            case( MNCC_BEARER_SERV_NOT_PRES ):
              callTable[lstIdx].calMode = CAL_MODE_Voice;
              break;

            default:
              callTable[lstIdx].calMode = CAL_MODE_Unknown;
          }
          break;

#ifdef FAX_AND_DATA
        case( TRANS_CALL ):
        case( NON_TRANS_CALL ):
          switch( psaCC_ctb(ctbIdx)->BC
                  [(psaCC_ctb(ctbIdx)->curBC EQ 0)?1:0].
                  bearer_serv)
          {
            case( MNCC_BEARER_SERV_SPEECH ):
              if( psaCC_ctb(ctbIdx)->rptInd EQ MNCC_RI_CIRCULAR )
                callTable[lstIdx].calMode = CAL_MODE_VAD_Data;
              else
                callTable[lstIdx].calMode = CAL_MODE_VFD_Data;
              break;

            case( MNCC_BEARER_SERV_NOT_PRES ):
              callTable[lstIdx].calMode = CAL_MODE_Data;
              break;

            default:
              callTable[lstIdx].calMode = CAL_MODE_Unknown;
          }
          break;

        case( FAX_CALL ):
          switch( psaCC_ctb(ctbIdx)->BC
                  [(psaCC_ctb(ctbIdx)->curBC EQ 0)?1:0].
                  bearer_serv)
          {
            case( MNCC_BEARER_SERV_SPEECH ):
              callTable[lstIdx].calMode = CAL_MODE_VAF_Fax;
              break;

            case( MNCC_BEARER_SERV_NOT_PRES ):
              callTable[lstIdx].calMode = CAL_MODE_Fax;
              break;

            default:
              callTable[lstIdx].calMode = CAL_MODE_Unknown;
          }
          break;
#endif /* #ifdef FAX_AND_DATA */

#if defined CO_UDP_IP || defined(FF_GPF_TCPIP)

/* SPR#1983 - SH - Identify WAP call */
#ifdef CO_UDP_IP

        case( UDPIP_CALL ):
         /*fallthrough if both defined*/
#endif
#if defined(FF_GPF_TCPIP)
        case (TCPIP_CALL):
#endif /* FF_GPF_TCPIP */
          switch( psaCC_ctb(ctbIdx)->BC
                  [(psaCC_ctb(ctbIdx)->curBC EQ 0)?1:0].
                  bearer_serv)
          {
            case( MNCC_BEARER_SERV_SPEECH ):
              if( psaCC_ctb(ctbIdx)->rptInd EQ MNCC_RI_CIRCULAR )
                callTable[lstIdx].calMode = CAL_MODE_VAD_Data;
              else
                callTable[lstIdx].calMode = CAL_MODE_VFD_Data;
              break;

            case( MNCC_BEARER_SERV_NOT_PRES ):
              callTable[lstIdx].calMode = CAL_MODE_Data;
              break;

            default:
              callTable[lstIdx].calMode = CAL_MODE_Unknown;
              break;
          }
          break;
#endif /* (CO_UDP_IP) || defined(FF_GPF_TCPIP) */
        default:
          callTable[lstIdx].calMode = CAL_MODE_Unknown;
      }
    /*
     *---------------------------------------------------------------
     * fill in call owner
     *---------------------------------------------------------------
     */
      switch( (T_ACI_CMD_SRC)psaCC_ctb(ctbIdx)->calOwn )
      {
        case( CMD_SRC_LCL ):
          callTable[lstIdx].calOwner = CAL_OWN_LCL;
          break;
#ifdef FF_ATI
        case(CMD_SRC_ATI_1):
        case(CMD_SRC_ATI_2):
        case(CMD_SRC_ATI_3):
        case(CMD_SRC_ATI_4):
          callTable[lstIdx].calOwner = CAL_OWN_RMT;
          break;
#endif /* FF_ATI */
        default:
          callTable[lstIdx].calOwner = CAL_OWN_NONE;
      }

    /*
     *---------------------------------------------------------------
     * fill in multiparty status
     *---------------------------------------------------------------
     */
      callTable[lstIdx].mpty = (psaCC_ctb(ctbIdx)->mptyStat EQ CS_ACT)?
                                  CAL_MPTY_IsMember : CAL_MPTY_NoMember;
    /*
     *---------------------------------------------------------------
     * fill in call index
     *---------------------------------------------------------------
     */
      callTable[lstIdx].index = ctbIdx+1;
      lstIdx++;
    }
  }

/*
 *-------------------------------------------------------------------
 * terminate list of calls
 *-------------------------------------------------------------------
 */
  if( lstIdx < ctbIdx )
  {
    callTable[lstIdx].index = -1;
  }

  return( AT_CMPL );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCQ                  |
| STATE   : code                  ROUTINE : qAT_CallActive           |
+--------------------------------------------------------------------+

  PURPOSE : The function checks whether at least one call is in an
            active state. This is not an official AT command !
*/

GLOBAL UBYTE qAT_CallActive ( void )
{
  SHORT ctbIdx;               /* holds call table index */

  TRACE_FUNCTION ("qAT_CallActive()");

  /*
   *-------------------------------------------------------------------
   * for every call of the call table
   *-------------------------------------------------------------------
   */
  for( ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
  {
    if (ccShrdPrm.ctb[ctbIdx] NEQ NULL)
    {
      /*
       *---------------------------------------------------------------
       * check call status
       *---------------------------------------------------------------
       */
      switch( psaCC_ctb(ctbIdx)->calStat )
      {
        case CS_ACT:
        case CS_HLD_REQ:
        case CS_HLD:
        case CS_MDF_REQ:
          return TRUE;
        default:
          break;
      }
    }
    
  }
  return FALSE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : qAT_PercentALS          |
+-------------------------------------------------------------------+

  PURPOSE : get the ALS mode for outgoing calls (voice)
            0: indicates bearer capability => BEARER_SERV_SPEECH
            1: indicates bearer capability => BEARER_SERV_AUX_SPEECH
*/

GLOBAL void cmhCC_get_active_als_mode( T_ACI_CMD_SRC srcId, T_ACI_ALS_MOD *mode )
{
  T_CC_CMD_PRM  *pCCCmdPrm;  /* points to CC command parameters */

  pCCCmdPrm  = &cmhPrm[srcId].ccCmdPrm;
  *mode = pCCCmdPrm->ALSmode;
}

GLOBAL T_ACI_RETURN qAT_PercentALS( T_ACI_CMD_SRC srcId, T_ACI_ALS_MOD *mode  )
{
  TRACE_FUNCTION("qAT_PercentALS()");

  if( !cmh_IsVldCmdSrc( srcId ) )
  {
    return( AT_FAIL );
  }

  cmhCC_get_active_als_mode( srcId, mode );

  return( AT_CMPL );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : qAT_PercentCTTY         |
+-------------------------------------------------------------------+

  PURPOSE : Query the state of the TTY Service
*/

GLOBAL T_ACI_RETURN qAT_PercentCTTY (T_ACI_CMD_SRC srcId,
                                     T_ACI_CTTY_MOD *mode,
                                     T_ACI_CTTY_REQ *req,
                                     T_ACI_CTTY_STAT *stat,
                                     T_ACI_CTTY_TRX *trx)
{
  SHORT ctNr;
  BOOL fnd_act = FALSE;;

  if( !cmh_IsVldCmdSrc( srcId ) )
  {
    return( AT_FAIL );
  }
  *mode = cmhPrm[srcId].ccCmdPrm.CTTYmode;
#ifdef FF_TTY
  if (ccShrdPrm.ctmReq EQ MNCC_CTM_DISABLED)
  {
    *req = CTTY_REQ_Off;
  }
  else switch (ccShrdPrm.ttyCmd)
  {
  case TTY_ALL:
    *req = CTTY_REQ_On;
    break;
  case TTY_HCO:
    *req = CTTY_REQ_HCO;
    break;
  case TTY_VCO:
    *req = CTTY_REQ_VCO;
    break;
  default:
    *req = CTTY_REQ_Off;
    break;
  }
  for (ctNr = 0; !fnd_act AND ctNr < MAX_CALL_NR; ctNr++)
  {
    if (ccShrdPrm.ctb[ctNr] NEQ NULL)
    {
      switch (psaCC_ctb(ctNr)->calStat)
      {
      case CS_ACT_REQ:
      case CS_ACT:
      case CS_HLD_REQ:
      case CS_HLD:
      case CS_MDF_REQ:
        *stat = (ccShrdPrm.ctmState EQ TTY_STATE_ACTIVE)?
                CTTY_STAT_On: CTTY_STAT_Off;
        if (*stat EQ CTTY_STAT_On)
        {
          *trx = cmhCC_getTTYtrx_state ((int)ccShrdPrm.ttyCmd);
        }
        else
        {
          *trx = CTTY_TRX_Unknown;
        }
        fnd_act = TRUE;
        break;
      default:
        break;
      }
    }
  }
  if (!fnd_act)
#else
  *req = CTTY_REQ_Off;
#endif  /* FF_TTY */
  {
    *stat = CTTY_STAT_Unknown;
    *trx = CTTY_TRX_Unknown;
  }
  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_CCQ            |
| STATE   : code                        ROUTINE : qAT_PercentRDL     |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %RDL? AT command
            which returns the current setting for redial mode.
*/

GLOBAL T_ACI_RETURN qAT_PercentRDL ( T_ACI_CMD_SRC srcId,
                                     T_ACI_CC_REDIAL_MODE* redial_mode,
                                     T_ACI_CC_REDIAL_NOTIF* notification)
{
  TRACE_FUNCTION ("qAT_PercentRDL()");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  /* fill in parameter */
  switch(rdlPrm.rdlMod)
  {
    case AUTOM_REPEAT_OFF:
    case AUTOM_REPEAT_ON:
        *redial_mode = rdlPrm.rdlMod;
        break;
    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }
  switch(rdlPrm.rdlModN)
  {
    case NO_NOTIF_USER:
    case NOTIF_USER:
        *notification = rdlPrm.rdlModN;
        return( AT_CMPL );
    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }
}

#ifdef TI_PS_FF_AT_P_CMD_RDLB
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)             MODULE  : CMH_CCQ              |
| STATE   : code                      ROUTINE : qAT_PercentRDLB      |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %RDLB? AT command
            which returns the current black list.
*/

GLOBAL T_ACI_RETURN qAT_PercentRDLB ( T_ACI_CMD_SRC srcId,
                                      T_ACI_CC_REDIAL_BLACKL *blackl,
                                      T_ACI_CC_REDIAL_NOTIF* notification)
{
  TRACE_FUNCTION ("qAT_PercentRDLB()");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  if (rdlPrm.rdlMod EQ AUTOM_REPEAT_ON)
  {
    if((cc_blacklist_ptr NEQ NULL) AND (cc_blacklist_ptr->blCount NEQ 0))
    {/* fill in parameter */
      memcpy(blackl,cc_blacklist_ptr,sizeof(T_ACI_CC_REDIAL_BLACKL));
    }
    *notification = rdlPrm.rdlBlN;
    return( AT_CMPL );
  }
  else
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }
}
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCQ                  |
| STATE   : code                  ROUTINE : qAT_PercentCSSD          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the AT%CSSD command
            which returns an extended error report.

*/

GLOBAL T_ACI_RETURN qAT_PercentCSSD  ( T_ACI_CMD_SRC    srcId,
                                       UBYTE           *ss_diag)
{
  T_CC_CALL_TBL *ctbFail = ccShrdPrm.ctb[ccShrdPrm.cIdFail];

  TRACE_FUNCTION ("qAT_PercentCSSD()");

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
 * get ss diagnostic
 *-------------------------------------------------------------------
 */

 /* 
  *  check whether this diagnostic is within the specified parameters 
  *  cross-check also MNCC SAP: VAL_ss_diag 
  *  (This implementation follows version V3.19.0 (2004-06))
  */
  if( (ctbFail->ssDiag >= MNCC_SS_DIAG_MOC_BAR_CUG) AND
      (ctbFail->ssDiag <= MNCC_SS_DIAG_CCBS_NOT_POSSIBLE ))
  {
    *ss_diag = ctbFail->ssDiag;    
    return( AT_CMPL );
  }
  /* not yet initialized or not defined, return no information available */
  else 
  {
    /*reset ccShrdPrm.ctb[ccShrdPrm.cIdFail].ssDiag */ 
    *ss_diag = ctbFail->ssDiag = MNCC_SS_DIAG_NOT_PROVIDED;    
    return( AT_CMPL );
  }
}

#ifdef TI_PS_FF_AT_P_CMD_CUSCFG
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCQ                  |
| STATE   : code                  ROUTINE : qAT_PercentCUSCFG          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the AT%CUSCFG query command
            which returns the customization status of the facility requested.

*/
GLOBAL T_ACI_RETURN qAT_PercentCUSCFG  ( T_ACI_CMD_SRC    srcId,
                                         T_ACI_CUSCFG_FAC facility,
                                         T_ACI_CUSCFG_STAT *status)
{
  TRACE_FUNCTION ("qAT_PercentCUSCFG()");

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
    *status = (T_ACI_CUSCFG_STAT)cuscfgParams.MO_SM_Control_SIM;
    break;

  case CUSCFG_FAC_MO_Call_Control:
    *status = (T_ACI_CUSCFG_STAT)cuscfgParams.MO_Call_Control_SIM;
    break;

  case CUSCFG_FAC_MO_SS_Control:
    *status = (T_ACI_CUSCFG_STAT)cuscfgParams.MO_SS_Control_SIM;
    break;

  case CUSCFG_FAC_MO_USSD_Control:
    *status = (T_ACI_CUSCFG_STAT)cuscfgParams.MO_USSD_Control_SIM;
    break;

  case CUSCFG_FAC_2_Digit_Call:
    *status = (T_ACI_CUSCFG_STAT)cuscfgParams.Two_digit_MO_Call;
    break;

  case CUSCFG_FAC_Ext_USSD_Res:
    *status = (T_ACI_CUSCFG_STAT)cuscfgParams.Ext_USSD_Response;
    break;

  case CUSCFG_FAC_T_MOBILE_Eons:
    *status = (T_ACI_CUSCFG_STAT)cuscfgParams.T_MOBILE_Eons;
    break;
     
  case CUSCFG_FAC_USSD_As_MO_Call:
    *status = (T_ACI_CUSCFG_STAT)cuscfgParams.USSD_As_MO_Call;
    break;


  default:
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }
  return(AT_CMPL);
}
#endif /* TI_PS_FF_AT_P_CMD_CUSCFG */

#ifdef TI_PS_FF_AT_P_CMD_STDR
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCQ                  |
| STATE   : code                  ROUTINE : qAT_PercentSTDR          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the AT%STDR command
            which returns the value of global variable <std>.

*/

GLOBAL T_ACI_RETURN qAT_PercentSTDR  ( T_ACI_CMD_SRC    srcId,
                                       UBYTE           *rvstd)
{
  TRACE_FUNCTION ("qAT_PercentSTDR()");

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
 * Read <std>
 *-------------------------------------------------------------------
 */
  *rvstd = std;

  return( AT_CMPL );
}
#endif /* TI_PS_FF_AT_P_CMD_STDR */

/* Implements Measure 47 and 216 */
/*
+------------------------------------------------------------------------------
|  Function    : qAT_plus_percent_CLCC
+------------------------------------------------------------------------------
|  Purpose     : This is common functional counterpart to the %CLCC? and +CLCC?
|
|  Parameters  : srcId      - AT command source identifier
|                calLst     - Points to CLCC current call 
|                             list element.
|                at_cmd_id  - AT Command Identifier 
|                             ( AT_CMD_CLCC or AT_CMD_P_CLCC )
|
|                rdlcId     - Redial Call Identifier , NO_ENTRY if no redial
|
|  Return      : ACI functional return codes
+------------------------------------------------------------------------------
*/

LOCAL T_ACI_RETURN qAT_plus_percent_CLCC( T_ACI_CMD_SRC srcId,
                                          T_ACI_CLCC_CALDESC *calLst,
                                          T_ACI_AT_CMD at_cmd_id, 
                                          SHORT rdlcId )
{
  SHORT ctbIdx;
  SHORT lstIdx;
  int index;

  TRACE_FUNCTION ("qAT_plus_percent_CLCC()");

  index =  ( at_cmd_id EQ AT_CMD_P_CLCC )?1:0;
  /*
   *-------------------------------------------------------------------
   * for every call of the call table
   *-------------------------------------------------------------------
   */
  for( ctbIdx = 0, lstIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
  {
    if ( (ccShrdPrm.ctb[ctbIdx] NEQ NULL) AND (rdlcId EQ NO_ENTRY) )
    {
      /*
       *---------------------------------------------------------------
       * fill in call type
       *---------------------------------------------------------------
       */
      if ( ( psaCC_ctb(ctbIdx)->calType >= CT_MOC ) AND 
           ( psaCC_ctb(ctbIdx)->calType <= CT_MOC_RDL ))
      {
        calLst[lstIdx].dir =
            (T_ACI_CLCC_DIR) aci_clcc_dir[psaCC_ctb(ctbIdx)->calType][index];
      }
      

      /*
       *---------------------------------------------------------------
       * fill in call status
       *---------------------------------------------------------------
       */
      switch( psaCC_ctb(ctbIdx)->calStat )
      {
        case( CS_ACT_REQ ):
          switch( psaCC_ctb(ctbIdx)->calType )
          {
            case( CT_MOC ):
            case( CT_MOC_RDL ):
              if( psaCC_ctb(ctbIdx)->alrtStat EQ AS_SND )
              {
                calLst[lstIdx].stat = CLCC_STAT_Alerting;
              }
              else
              {
                calLst[lstIdx].stat = CLCC_STAT_Dialing;
              }
              break;

            case( CT_MTC ):
              if (  psaCC_ctbCallInUse ( ) )
              {
                calLst[lstIdx].stat = CLCC_STAT_Waiting;
              }
              else
              {
                calLst[lstIdx].stat = CLCC_STAT_Incoming;
              }
              break;
          }
          break;

        case( CS_ACT ):
        case( CS_DSC_REQ ):
        /* 
         * Also show calls in disconnecting state since in 4.08/5.4.4.2.1.1 ii)
         * the user is still connected to the network.
         * This call is then either terminated from the network side.
         * (after the operator announcement) or user initiated by ATH 
         */
        case( CS_MDF_REQ ):
        case( CS_HLD_REQ ):
          calLst[lstIdx].stat = CLCC_STAT_Active;
          break;

        case( CS_HLD ):
          calLst[lstIdx].stat = CLCC_STAT_Held;
          break;

        case( CS_IDL ):
        case( CS_CPL_REQ ):
          continue;
      }

    /*
     *---------------------------------------------------------------
     * fill in number of connected party
     *---------------------------------------------------------------
     */
      switch( psaCC_ctb(ctbIdx)->calType )
      {
        case( CT_MOC ):
        case( CT_MOC_RDL ):
          psaCC_ctbCldAdr2Num (ctbIdx, calLst[lstIdx].number,
                               MAX_CC_ORIG_NUM_LEN);
          calLst[lstIdx].type.ton = (T_ACI_TOA_TON)psaCC_ctb(ctbIdx)->cldPty.ton;
          calLst[lstIdx].type.npi = (T_ACI_TOA_NPI)psaCC_ctb(ctbIdx)->cldPty.npi;
          break;

        case( CT_MTC ):
          psaCC_ctbClrAdr2Num (ctbIdx, calLst[lstIdx].number,
                               MAX_CC_ORIG_NUM_LEN);
          calLst[lstIdx].type.ton = (T_ACI_TOA_TON)psaCC_ctb(ctbIdx)->clgPty.ton;
          calLst[lstIdx].type.npi = (T_ACI_TOA_NPI)psaCC_ctb(ctbIdx)->clgPty.npi;
          break;
      }
#ifdef NO_ASCIIZ
      memcpy( ( UBYTE* ) &calLst[lstIdx].alpha,
              ( UBYTE* ) &psaCC_ctb(ctbIdx)->alphIdUni,
              sizeof( T_ACI_PB_TEXT ) );
#else  /* NO_ASCIIZ */
      memcpy( calLst[lstIdx].alpha,
              ccShrdPrm.ctb[ctbIdx].alphIdUni.data,
              ccShrdPrm.ctb[ctbIdx].alphIdUni.len );

      cmh_cvtFromDefGsm ( ( CHAR* ) ccShrdPrm.ctb[ctbIdx].alphIdUni.data,
                          ( USHORT ) ccShrdPrm.ctb[ctbIdx].alphIdUni.len,
                          calLst[lstIdx].alpha );
#endif /* NO_ASCIIZ */

    /*
     *---------------------------------------------------------------
     * fill in call mode
     *---------------------------------------------------------------
     */
      switch( cmhCC_getcalltype(ctbIdx) )
      {
        case( VOICE_CALL ):
          switch( psaCC_ctb(ctbIdx)->BC
                  [(psaCC_ctb(ctbIdx)->curBC EQ 0)?1:0].
                  bearer_serv)
          {
            case( MNCC_BEARER_SERV_ASYNC ):
              if( psaCC_ctb(ctbIdx)->rptInd EQ MNCC_RI_CIRCULAR )
                calLst[lstIdx].mode = CLCC_MODE_Voice;
              else
                calLst[lstIdx].mode = CLCC_MODE_VFDVoice;
              break;

            case( MNCC_BEARER_SERV_FAX ):
              calLst[lstIdx].mode = CLCC_MODE_VAFVoice;
              break;

            case( MNCC_BEARER_SERV_NOT_PRES ):
              calLst[lstIdx].mode = CLCC_MODE_Voice;
              break;

            default:
              calLst[lstIdx].mode = CLCC_MODE_Unknown;
          }
          break;

#ifdef FAX_AND_DATA
        case( TRANS_CALL ):
        case( NON_TRANS_CALL ):
          switch( psaCC_ctb(ctbIdx)->BC
                  [(psaCC_ctb(ctbIdx)->curBC EQ 0)?1:0].
                  bearer_serv)
          {
            case( MNCC_BEARER_SERV_SPEECH ):
              if( psaCC_ctb(ctbIdx)->rptInd EQ MNCC_RI_CIRCULAR )
                calLst[lstIdx].mode = CLCC_MODE_VADData;
              else
                calLst[lstIdx].mode = CLCC_MODE_VFDData;
              break;

            case( MNCC_BEARER_SERV_NOT_PRES ):
              calLst[lstIdx].mode = CLCC_MODE_Data;
              break;

            default:
              calLst[lstIdx].mode = CLCC_MODE_Unknown;
          }
          break;

        case( FAX_CALL ):
          switch( psaCC_ctb(ctbIdx)->BC
                  [(psaCC_ctb(ctbIdx)->curBC EQ 0)?1:0].
                  bearer_serv)
          {
            case( MNCC_BEARER_SERV_SPEECH ):
              calLst[lstIdx].mode = CLCC_MODE_VAFFax;
              break;

            case( MNCC_BEARER_SERV_NOT_PRES ):
              calLst[lstIdx].mode = CLCC_MODE_Fax;
              break;

            default:
              calLst[lstIdx].mode = CLCC_MODE_Unknown;
          }
          break;
#endif /* #ifdef FAX_AND_DATA */

        default:
          calLst[lstIdx].mode = CLCC_MODE_Unknown;
      }

      /*
       *---------------------------------------------------------------
       * fill in multiparty status
       *---------------------------------------------------------------
       */
      calLst[lstIdx].mpty = (psaCC_ctb(ctbIdx)->mptyStat EQ CS_ACT)?
                              CLCC_MPTY_IsMember : CLCC_MPTY_NoMember;
     /*
      *---------------------------------------------------------------
      * fill in line1 or line2
      *---------------------------------------------------------------
      */
     switch (cmhCC_GetCallType_from_bearer( &psaCC_ctb(ctbIdx)->BC[psaCC_ctb(ctbIdx)->curBC] ))
     {
        case ( CRING_SERV_TYP_Voice):
           calLst[lstIdx].class_type = CLCC_CLASS_Line1;
           break;

        case ( CRING_SERV_TYP_AuxVoice):
           calLst[lstIdx].class_type = CLCC_CLASS_Line2;
           break;

        default:
           calLst[lstIdx].class_type = CLCC_CLASS_NotPresent;
           break;
      }
      /*
       *---------------------------------------------------------------
       * fill in progress description if using BAT interface
       *---------------------------------------------------------------
       */
#ifdef FF_BAT
       if ( at_cmd_id EQ AT_CMD_P_CLCC )
       {
         calLst[lstIdx].prog_desc = psaCC_ctb(ctbIdx)->prgDesc;
       }
#endif

      /*
       *---------------------------------------------------------------
       * fill in call index
       *---------------------------------------------------------------
       */
      calLst[lstIdx].idx = ctbIdx+1;
      lstIdx++;

   }
  }

  /*
   *-------------------------------------------------------------------
   * terminate list of calls
   *-------------------------------------------------------------------
   */
  if( lstIdx < ctbIdx )
  {
    calLst[lstIdx].idx       = ACI_NumParmNotPresent;
    calLst[lstIdx].dir       = CLCC_DIR_NotPresent;
    calLst[lstIdx].stat      = CLCC_STAT_NotPresent;
    calLst[lstIdx].mode      = CLCC_MODE_NotPresent;
    calLst[lstIdx].mpty      = CLCC_MPTY_NotPresent;
    calLst[lstIdx].type.ton  = TON_NotPresent;
    calLst[lstIdx].number[0] = 0x0;
    if( at_cmd_id EQ AT_CMD_P_CLCC )
    {
      calLst[lstIdx].class_type = CLCC_CLASS_NotPresent;
#ifdef FF_BAT
      calLst[lstIdx].prog_desc = NOT_PRESENT_8BIT;
#endif
    }
#ifdef NO_ASCIIZ
    calLst[lstIdx].alpha.cs  = CS_NotPresent;
    calLst[lstIdx].alpha.len = 0;
#else  /* #ifdef NO_ASCIIZ */
    calLst[lstIdx].alpha[0]  = 0x0;
#endif /* #ifdef NO_ASCIIZ */
  }

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCVHU             |
+--------------------------------------------------------------------+

  PURPOSE : This is query call for +CVHU for the control of the voice 
            hangup
  
*/
GLOBAL T_ACI_RETURN qAT_PlusCVHU ( T_ACI_CMD_SRC srcId, T_ACI_CVHU_MODE *mode)
{
  TRACE_FUNCTION("qAT_PlusCVHU()");

  if(!cmh_IsVldCmdSrc (srcId))
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }
  
  *mode = ccShrdPrm.cvhu;

  return( AT_CMPL );
}
/*==== EOF ========================================================*/
