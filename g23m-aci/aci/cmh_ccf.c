/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_CCF
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
|  Purpose :  This module defines the functions used by the command
|             handler for call control.
+-----------------------------------------------------------------------------
*/

#ifndef CMH_CCF_C
#define CMH_CCF_C
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
#include "aci_mem.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif

#ifdef FF_TTY
#include "audio.h"
#endif

#include "aci_io.h"

#include "phb.h"
#include "ksd.h"
#include "aoc.h"
#include "aci.h"
#include "psa.h"
#include "psa_cc.h"
#include "psa_ss.h"
#include "psa_util.h"
#include "cmh.h"
#include "cmh_cc.h"
#include "cmh_ss.h"
#include "cmh_phb.h"

#include "aci_lst.h"

#ifdef DTI
#include "dti.h"      /* functionality of the dti library */
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"
#endif

#ifdef UART
#include "psa_uart.h"
#include "cmh_uart.h"
#endif

#ifdef FAX_AND_DATA

#include "psa_ra.h"
#include "cmh_ra.h"
#include "psa_l2r.h"
#include "cmh_l2r.h"

#ifdef FF_FAX
#include "psa_t30.h"
#include "cmh_t30.h"
#endif /* FF_FAX */

#endif /* FAX_AND_DATA */

#if defined (FF_WAP) || defined (FF_PPP) || defined(FF_GPF_TCPIP) || defined (FF_SAT_E)
#include "wap_aci.h"
#include "psa_ppp_w.h"
#include "cmh_ppp.h"
#endif /* of FF_WAP or FF_PPP  || (FF_GPF_TCPIP)|| (FF_SAT_E)*/

#ifdef FF_GPF_TCPIP
#include "dcm_utils.h"
#include "dcm_state.h"
#include "dcm_env.h"
#endif

#ifdef SIM_TOOLKIT
#include "psa_sat.h"
#include "cmh_sat.h"
#include "psa_sim.h"
#endif

#ifdef FF_PSI
#include "psa_psi.h"
#include "cmh_psi.h"
#endif /*FF_PSI*/
#include "l4_tim.h"

#if defined(_TARGET_)
#include "../../services/ffs/ffs.h"
#include "ffs_coat.h"
#endif /* _TARGET_ */

#include "dcm_f.h"

/*==== CONSTANTS ==================================================*/
#define   ACI_MAX_DIAL_SHORT_STRING_LEN     2
#ifdef TI_PS_FF_AT_P_CMD_RDLB
EXTERN T_ACI_CC_REDIAL_BLACKL * cc_blacklist_ptr;
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/
GLOBAL T_ACI_CUSCFG_PARAMS cuscfgParams;

/* Implements Measure#32: Row 73, 74, 103, 114 & 1071 */
const char * const gsm_com_path="/gsm/com";
const char * const gsm_com_redialmode_path="/gsm/com/redialMode";
const char * const gsm_com_alslock_path="/gsm/com/ALSlock";
const char * const gsm_com_rfcap_path="/gsm/com/rfcap";
/*==== FUNCTIONS ==================================================*/
/* Implements Measure 197, 198 and 199 */
LOCAL void cmhCC_redialAttempt( SHORT cId, UBYTE rdlCnt );

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH                     |
|                                 ROUTINE : cmhCC_PrepareCmdEnd     |
+-------------------------------------------------------------------+

  PURPOSE : Prepares the end of a CC related AT command.
            cId describes a valid (non-NULL) call table entry.

*/

GLOBAL void cmhCC_PrepareCmdEnd (SHORT cId, UBYTE *cmdBuf, UBYTE *srcBuf)
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

  if (cmdBuf NEQ NULL)
    *cmdBuf = ctb->curCmd;
  if (srcBuf NEQ NULL)
    *srcBuf = ctb->curSrc;
  ctb->curCmd = AT_CMD_NONE;
  ctb->curSrc = CMD_SRC_NONE;
}


GLOBAL UBYTE cmhCC_set_speech_serv (T_CC_CMD_PRM  *pCCCmdPrm)
{
#ifdef FF_TTY
  UBYTE ctmReq;

  /* check temporary inversion of CTM service request */
  if (!ccShrdPrm.ctmOvwr)
  {
    ctmReq = ccShrdPrm.ctmReq;
  }
  else
  {
    if (ccShrdPrm.ctmReq EQ MNCC_CTM_ENABLED)
    {
      ctmReq = MNCC_CTM_DISABLED;
      ccShrdPrm.ttyCmd = (UBYTE)TTY_OFF;
    }
    else
    {
      ctmReq = MNCC_CTM_ENABLED;
      ccShrdPrm.ttyCmd = (UBYTE)TTY_ALL;
    }
    /* reset temporary inversion */
    ccShrdPrm.ctmOvwr = FALSE;
  }
#endif /* FF_TTY */
#ifdef FF_TTY
  if (ctmReq EQ MNCC_CTM_ENABLED)
  {
    if (ccShrdPrm.ctmState EQ TTY_STATE_NONE)
    {
      ccShrdPrm.ctmState = TTY_STATE_IDLE;
    }
    if (pCCCmdPrm->ALSmode EQ ALS_MOD_AUX_SPEECH)
    {
      return MNCC_BEARER_SERV_AUX_SPEECH_CTM;
    }
    else
    {
      return MNCC_BEARER_SERV_SPEECH_CTM;
    }
  }
  else
#endif /* FF_TTY */
  {
#ifdef FF_TTY
    if (ccShrdPrm.ctmState EQ TTY_STATE_IDLE)
    {
      ccShrdPrm.ctmState = TTY_STATE_NONE;
    }
#endif /* FF_TTY */
    if (pCCCmdPrm->ALSmode EQ ALS_MOD_AUX_SPEECH)
    {
      return MNCC_BEARER_SERV_AUX_SPEECH;
    }
  }
  return MNCC_BEARER_SERV_SPEECH;
}

GLOBAL T_ACI_RETURN cmhCC_chkShortString (T_ACI_CMD_SRC srcId,
                                          SHORT         cId,
                                          T_CLPTY_PRM   *cldPty)
{
  USHORT              num_len;
  T_ACI_KSD_USSD_PRM  ksdString;

  num_len = strlen (cldPty->num);

  /* check the length */
  if (!num_len OR (num_len > ACI_MAX_DIAL_SHORT_STRING_LEN))
  {
    return (AT_EXCT);
  }

  ksdString.ussd = (UBYTE*)cldPty->num;

  /* check if MS is in a call */
  if (cId NEQ NO_ENTRY)
  {
    /* call is active */
    return (cmhSS_ksdUSSD (srcId, &ksdString));
  }

  /* check if input string is 2 digit starting with a 1 */
  if ((num_len EQ ACI_MAX_DIAL_SHORT_STRING_LEN) AND
      (*(cldPty->num) EQ '1') OR cuscfgParams.Two_digit_MO_Call)
  {
    /* normal dial */
    return (AT_EXCT);
  }

  return (cmhSS_ksdUSSD (srcId, &ksdString));
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH                     |
|                                 ROUTINE : cmhCC_find_call_for_DTMF|
+-------------------------------------------------------------------+

  PURPOSE :searches for a call where it is possible to
             send DTMF tones.
*/
GLOBAL BOOL is_call_ok_for_dtmf( SHORT cId )
{
  T_CC_CALL_TYPE calltype = cmhCC_getcalltype(cId);

  if(calltype NEQ VOICE_CALL)
  {
    TRACE_EVENT_P1("calltype NEQ VOICE_CALL: %d", calltype);
    return(FALSE);
  }

  if (ccShrdPrm.ctb[cId] EQ NULL)
  {
    TRACE_EVENT("ccShrdPrm.ctb[cId] EQ NULL");
    return(FALSE);
  }

  switch(psaCC_ctb(cId)->calStat)
  {
    case(CS_ACT):
    case(CS_ACT_REQ):
    case(CS_DSC_REQ):
    case(CS_CPL_REQ):
      return(TRUE);

    default:
      TRACE_EVENT_P1("unexpected call status: %d", psaCC_ctb(cId)->calStat);
      return(FALSE);
  }
}

GLOBAL SHORT cmhCC_find_call_for_DTMF( void )
{
  SHORT ctbIdx;        /* holds call table index */

  TRACE_FUNCTION("cmhCC_find_call_for_DTMF");

  for( ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
  {
    if( is_call_ok_for_dtmf(ctbIdx) )
    {
      return(ctbIdx);
    }
  }
  return( NO_ENTRY );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH                     |
|                                 ROUTINE : cmhCC_SendDTMFdig       |
+-------------------------------------------------------------------+

  PURPOSE : send a DTMF digit for that call.
    return value: TRUE if digit has been correctly proceeded.
                  FALSE if an error occured.
*/

GLOBAL BOOL cmhCC_SendDTMFdig( T_ACI_AT_CMD cmd, SHORT cId, CHAR digit, UBYTE mode)
{
  TRACE_FUNCTION("cmhCC_SendDTMFdig");

  TRACE_EVENT_P1("Sending DTMF tone: %d", digit);

  switch( digit )
  {
    case( '0' ):
    case( '1' ):
    case( '2' ):
    case( '3' ):
    case( '4' ):
    case( '5' ):
    case( '6' ):
    case( '7' ):
    case( '8' ):
    case( '9' ):
    case( '*' ):
    case( '#' ):
    case( 'A' ):
    case( 'B' ):
    case( 'C' ):
    case( 'D' ):
      psaCC_SendDTMF( cId, digit, mode );
      return( TRUE );

    case( 'W' ):   /* FIXME: w should prompt the user for starting sending the following DTMF tones */
    case( 'w' ):   /* since BMI does not support 'w' we assume to be the same as 'p' */
    case( 'P' ):
    case( 'p' ):
      if(cmd NEQ AT_CMD_VTS)  /* this is only valid within a number to dial */
      {
        /* p within a number string: this is a 3 seconds pause */
        TRACE_EVENT("DTMF pause requested: 3 seconds");
#if defined (NEW_FRAME)
        TIMERSTART( TDTMF_VALUE, ACI_TDTMF );
#else
        TIMERSTART( TDTMF_VALUE, t_dtmf_handle );
#endif
        return( TRUE );
      }
      /* fallthrough for else */
      /*lint -fallthrough*/
    default:
      TRACE_EVENT_P1("Invalid DTMF digit: %d", digit);
  }
  return( FALSE );
}


#ifdef FAX_AND_DATA
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : cmhCC_SelRate           |
+-------------------------------------------------------------------+

  PURPOSE : select the user rate out of the current speed setting

*/

GLOBAL UBYTE cmhCC_SelRate ( T_ACI_BS_SPEED speed )
{
  switch( speed )
  {
  case( BS_SPEED_300_V21      ):
  case( BS_SPEED_300_V110     ): return( MNCC_UR_0_3_KBIT );
  case( BS_SPEED_1200_V22     ):
  case( BS_SPEED_1200_V120    ):
  case( BS_SPEED_1200_V110    ): return( MNCC_UR_1_2_KBIT );
  case( BS_SPEED_1200_75_V23  ): return( MNCC_UR_1_2_KBIT_V23 );
  case( BS_SPEED_2400_V22bis  ):
  case( BS_SPEED_2400_V26ter  ):
  case( BS_SPEED_2400_V120    ):
  case( BS_SPEED_2400_V110    ): return( MNCC_UR_2_4_KBIT );
  case( BS_SPEED_4800_V32     ):
  case( BS_SPEED_4800_V120    ):
  case( BS_SPEED_4800_V110    ): return( MNCC_UR_4_8_KBIT );
  case( BS_SPEED_AUTO         ):
  case( BS_SPEED_9600_V32     ):
  case( BS_SPEED_9600_V34     ):
  case( BS_SPEED_9600_V120    ):
  case( BS_SPEED_9600_V110    ): return( MNCC_UR_9_6_KBIT );
  case( BS_SPEED_14400_V34    ):
  case( BS_SPEED_14400_V120   ):
  case( BS_SPEED_14400_V110   ): return( MNCC_UR_14_4_KBIT );
  default:                       return( MNCC_UR_NOT_PRES  );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : cmhCC_SelMT             |
+-------------------------------------------------------------------+

  PURPOSE : select the modem type out of the current speed setting

*/

GLOBAL UBYTE cmhCC_SelMT ( T_ACI_BS_SPEED speed )
{
  switch( speed )
  {
  case( BS_SPEED_300_V21      ): return( MNCC_MT_V21 );
  case( BS_SPEED_1200_V22     ): return( MNCC_MT_V22 );
  case( BS_SPEED_2400_V22bis  ): return( MNCC_MT_V22_BIS );
  case( BS_SPEED_1200_75_V23  ): return( MNCC_MT_V23 );
  case( BS_SPEED_2400_V26ter  ): return( MNCC_MT_V26_TER );
  case( BS_SPEED_9600_V32     ):
  case( BS_SPEED_4800_V32     ): return( MNCC_MT_V32 );
  case( BS_SPEED_1200_V120    ):
  case( BS_SPEED_2400_V120    ):
  case( BS_SPEED_4800_V120    ):
  case( BS_SPEED_9600_V120    ):
  case( BS_SPEED_14400_V120   ):
  case( BS_SPEED_2400_V110    ):
  case( BS_SPEED_4800_V110    ):
  case( BS_SPEED_9600_V110    ):
  case( BS_SPEED_14400_V110   ):
  case( BS_SPEED_300_V110     ):
  case( BS_SPEED_1200_V110    ): return( MNCC_MT_NONE );
  case( BS_SPEED_9600_V34     ):
  case( BS_SPEED_14400_V34    ): return( MNCC_MT_V34 );
  default:                       return( MNCC_MT_AUTOBAUD  );
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : cmhCC_SelTransferCap    |
+-------------------------------------------------------------------+

  PURPOSE : select the transfer capabilty on basis of speed

*/

GLOBAL UBYTE cmhCC_SelTransferCap ( T_ACI_BS_SPEED speed )
{
  TRACE_FUNCTION("cmhCC_SelTransferCap()");

  if (speed <= BS_SPEED_14400_V34)
  {
    return MNCC_ITC_NONE;
  }
  else
  {
    return MNCC_ITC_UDI;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : cmhCC_SelRateAdaption   |
+-------------------------------------------------------------------+

  PURPOSE : select the rate adaption on basis of speed selection

*/

GLOBAL UBYTE cmhCC_SelRateAdaption ( T_ACI_BS_SPEED speed)
{
  TRACE_FUNCTION("cmhCC_SelRateAdaption()");

  if ((speed >= BS_SPEED_1200_V120) AND (speed <= BS_SPEED_14400_V120))
  {
    return MNCC_RATE_ADAPT_V120;
  }
  else if ((speed >= BS_SPEED_300_V110) AND (speed <= BS_SPEED_14400_V110))
  {
    return MNCC_RATE_ADAPT_V110;
  }
  else
  {
    return MNCC_RATE_ADAPT_NONE;
  }
}
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : cmhCC_SelServ           |
+-------------------------------------------------------------------+

  PURPOSE : select the bearer service out of the current name setting

*/

GLOBAL UBYTE cmhCC_SelServ ( T_ACI_CBST_NAM name )
{
  switch( name )
  {
  case( CBST_NAM_Asynch ): return( MNCC_BEARER_SERV_ASYNC );
  case( CBST_NAM_Synch  ): return( MNCC_BEARER_SERV_SYNC );
  default:                 return( MNCC_BEARER_SERV_NOT_PRES );
  }

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : cmhCC_SelCE             |
+-------------------------------------------------------------------+

  PURPOSE : select the connection element out of the current ce setting

*/

GLOBAL UBYTE cmhCC_SelCE ( T_ACI_CBST_CE ce )
{
  switch( ce )
  {
  case( CBST_CE_Transparent      ): return( MNCC_CONN_ELEM_TRANS );
  case( CBST_CE_NonTransparent   ): return( MNCC_CONN_ELEM_NON_TRANS );
  case( CBST_CE_BothTransPref    ): return( MNCC_CONN_ELEM_TRANS_PREF );
  case( CBST_CE_BothNonTransPref ): return( MNCC_CONN_ELEM_NON_TRANS_PREF );
  default:                          return( MNCC_CONN_ELEM_NOT_PRES );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : cmhCC_SelStopBit        |
+-------------------------------------------------------------------+

  PURPOSE : select the stop bits out of the current character framing
            setting

*/

GLOBAL UBYTE cmhCC_SelStopBit ( T_ACI_CMD_SRC srcId )
{
#ifdef UART
  if (UART_IO_SB_2 EQ cmhUART_GetStopBitOverSrcID( (UBYTE) srcId ))
  {
    return( MNCC_STOP_2_BIT );
  }
#endif /*UART*/
#if defined (FF_PSI) AND defined (DTI)
  if (UART_IO_SB_2 EQ cmhPSI_GetStopBitOverSrcID( (UBYTE) srcId ))
  {
    return( MNCC_STOP_2_BIT );
  }
#endif /*FF_PSI*/

  return( MNCC_STOP_1_BIT );
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : cmhCC_SelDataBit        |
+-------------------------------------------------------------------+

  PURPOSE : select the data bits out of the current character framing
            setting

*/

GLOBAL UBYTE cmhCC_SelDataBit ( T_ACI_CMD_SRC srcId )
{
#ifdef UART
  if ( UART_IO_BPC_7 EQ cmhUART_GetDataBitOverSrcID( (UBYTE) srcId ))

  {
    return( MNCC_DATA_7_BIT );
  }
#endif /*UART*/

#if defined (FF_PSI) AND defined (DTI)
  if ( UART_IO_BPC_7 EQ cmhPSI_GetDataBitOverSrcID( (UBYTE) srcId ) )
  {
    return( MNCC_DATA_7_BIT );
  }
#endif /*FF_PSI*/

  return( MNCC_DATA_8_BIT );
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : cmhCC_SelParity         |
+-------------------------------------------------------------------+

  PURPOSE : select the data bits out of the current character framing
            setting

*/

GLOBAL UBYTE cmhCC_SelParity ( T_ACI_CMD_SRC srcId )
{
  UBYTE parity=0;
#if defined (FF_PSI) AND defined (DTI)
  T_ACI_DTI_PRC_PSI  *src_infos = find_element (psi_src_params,
                         (UBYTE)srcId, cmhPSItest_srcId);

  if (src_infos EQ NULL)
#endif /*FF_PSI*/

#ifdef UART
    parity =  cmhUART_GetParityOverSrcID( (UBYTE) srcId );
#else /*UART*/
    parity=0;
#endif /*UART*/

#if defined (FF_PSI) AND defined (DTI)
  else
    parity =  cmhPSI_GetParityOverSrcID( (UBYTE) srcId );
#endif /*FF_PSI*/
#ifdef UART
  switch( parity )
  {
    case UART_IO_PA_NONE:                 return( MNCC_PARITY_NONE );
    case UART_IO_PA_EVEN:                 return( MNCC_PARITY_EVEN );
    case UART_IO_PA_ODD:                  return( MNCC_PARITY_ODD );
    case UART_IO_PA_SPACE:                return( MNCC_PARITY_FORCED_TO_0 );
    case NOT_SUPPORTED_UART_IO_PA_MARK:   return( MNCC_PARITY_FORCED_TO_1 );
    default:                              TRACE_EVENT( "UNEXP PARITY IN ICF" );
  }
#endif /*UART*/
  return( MNCC_PARITY_NONE );

}
#endif /* FAX_AND_DATA */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : cmhCC_GetCallType       |
+-------------------------------------------------------------------+

  PURPOSE : get call type out of bearer capabilities

*/

GLOBAL SHORT cmhCC_GetCallType_from_bearer ( void * bearCap )
{
  T_MNCC_bcpara * pBC = (T_MNCC_bcpara*) bearCap;

  switch( pBC->bearer_serv )
  {
  case( MNCC_BEARER_SERV_SYNC          ):
    switch( pBC -> conn_elem )
    {
    case( MNCC_CONN_ELEM_TRANS         ): return( CRING_SERV_TYP_Sync );
    case( MNCC_CONN_ELEM_NON_TRANS     ): return( CRING_SERV_TYP_RelSync );
    default:                         TRACE_EVENT( "UNEXP CONN ELEMENT IN CTB" );
    /*lint -fallthrough*/
    case( MNCC_CONN_ELEM_NOT_PRES      ): return( CRING_SERV_TYP_NotPresent );
    }

  case( MNCC_BEARER_SERV_ASYNC         ):
    switch( pBC -> conn_elem )
    {
    case( MNCC_CONN_ELEM_TRANS         ): return( CRING_SERV_TYP_Async );
    case( MNCC_CONN_ELEM_NON_TRANS     ): return( CRING_SERV_TYP_RelAsync );
    default:                         TRACE_EVENT( "UNEXP CONN ELEMENT IN CTB" );
    /*lint -fallthrough*/
    case( MNCC_CONN_ELEM_NOT_PRES      ): return( CRING_SERV_TYP_NotPresent );
    }

  case( MNCC_BEARER_SERV_FAX           ): return( CRING_SERV_TYP_Fax );

  case( MNCC_BEARER_SERV_SPEECH_CTM    ):
  case( MNCC_BEARER_SERV_SPEECH        ): return( CRING_SERV_TYP_Voice );

  case( MNCC_BEARER_SERV_AUX_SPEECH_CTM):
  case( MNCC_BEARER_SERV_AUX_SPEECH    ): return( CRING_SERV_TYP_AuxVoice );

  default:                           TRACE_EVENT( "UNEXP BEARER SERVICE IN CTB" );
  /*lint -fallthrough*/
  case( MNCC_BEARER_SERV_PAD_ACCESS    ):
  case( MNCC_BEARER_SERV_PACKET_ACCESS ):
  case( MNCC_BEARER_SERV_NOT_PRES      ): return( CRING_SERV_TYP_NotPresent );

  }
}

#ifdef FAX_AND_DATA
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : cmhCC_GetSrvType        |
+-------------------------------------------------------------------+

  PURPOSE : get service type out of bearer capabilities

*/

GLOBAL SHORT cmhCC_GetSrvType ( void * bearCap )
{
  T_MNCC_bcpara * pBC = (T_MNCC_bcpara*) bearCap;

  switch( pBC->bearer_serv )
  {
  case( MNCC_BEARER_SERV_SYNC          ):
    switch( pBC -> conn_elem )
    {
    case( MNCC_CONN_ELEM_TRANS         ): return( CRING_SERV_TYP_Sync );
    case( MNCC_CONN_ELEM_NON_TRANS     ): return( CRING_SERV_TYP_RelSync );
    default:                         TRACE_EVENT( "UNEXP CONN ELEMENT IN CTB" );
    /*lint -fallthrough*/
    case( MNCC_CONN_ELEM_NOT_PRES      ): return( CRING_SERV_TYP_NotPresent );
    }

  case( MNCC_BEARER_SERV_ASYNC         ):
    switch( pBC -> conn_elem )
    {
    case( MNCC_CONN_ELEM_TRANS         ): return( CRING_SERV_TYP_Async );
    case( MNCC_CONN_ELEM_NON_TRANS     ): return( CRING_SERV_TYP_RelAsync );
    default:                         TRACE_EVENT( "UNEXP CONN ELEMENT IN CTB" );
    /*lint -fallthrough*/
    case( MNCC_CONN_ELEM_NOT_PRES      ): return( CRING_SERV_TYP_NotPresent );
    }

  default:                           TRACE_EVENT( "UNEXP BEARER SERVICE IN CTB" );
  /*lint -fallthrough*/
  case( MNCC_BEARER_SERV_FAX           ):
  case( MNCC_BEARER_SERV_SPEECH_CTM    ):
  case( MNCC_BEARER_SERV_AUX_SPEECH_CTM):
  case( MNCC_BEARER_SERV_SPEECH        ):
  case( MNCC_BEARER_SERV_AUX_SPEECH    ):
  case( MNCC_BEARER_SERV_PAD_ACCESS    ):
  case( MNCC_BEARER_SERV_PACKET_ACCESS ):
  case( MNCC_BEARER_SERV_NOT_PRES      ): return( CRING_SERV_TYP_NotPresent );

  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : cmhCC_GetDataRate       |
+-------------------------------------------------------------------+

  PURPOSE : get user data rate out of bearer capabilities

*/

GLOBAL SHORT cmhCC_GetDataRate ( void * bearCap )
{
  T_MNCC_bcpara * pBC = (T_MNCC_bcpara*) bearCap;

  switch( pBC->rate )
  {
  case( MNCC_UR_0_3_KBIT  ): return( BS_SPEED_300_V110   );
  case( MNCC_UR_1_2_KBIT  ): return( BS_SPEED_1200_V110  );
  case( MNCC_UR_2_4_KBIT  ): return( BS_SPEED_2400_V110  );
  case( MNCC_UR_4_8_KBIT  ): return( BS_SPEED_4800_V110  );
  case( MNCC_UR_9_6_KBIT  ): return( BS_SPEED_9600_V110  );
  case( MNCC_UR_14_4_KBIT ): return( BS_SPEED_14400_V110 );
  default:              return( BS_SPEED_NotPresent );
  }

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : cmhCC_GetFormat         |
+-------------------------------------------------------------------+

  PURPOSE : get character format out of bearer capabilities

*/

GLOBAL SHORT cmhCC_GetFormat ( void * bearCap )
{
  T_MNCC_bcpara * pBC = (T_MNCC_bcpara*) bearCap;

  if( pBC->data_bits EQ MNCC_DATA_7_BIT)
  {
    if( pBC->stop_bits EQ MNCC_STOP_1_BIT )
    {
      if( pBC->parity EQ MNCC_PARITY_NONE ) return( BS_FRM_Dat7_Par0_St1 );
      else                             return( BS_FRM_Dat7_Par1_St1 );
    }
    else return( BS_FRM_Dat7_Par0_St2 );
  }
  else
  {
    if( pBC->stop_bits EQ MNCC_STOP_1_BIT )
    {
      if( pBC->parity EQ MNCC_PARITY_NONE ) return( BS_FRM_Dat8_Par0_St1 );
      else                             return( BS_FRM_Dat8_Par1_St1 );
    }
    else return( BS_FRM_Dat8_Par0_St2 );
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : cmhCC_GetParity         |
+-------------------------------------------------------------------+

  PURPOSE : get parity out of bearer capabilities

*/

GLOBAL SHORT cmhCC_GetParity ( void * bearCap )
{
  T_MNCC_bcpara * pBC = (T_MNCC_bcpara*) bearCap;

  switch( pBC->parity )
  {
  case( MNCC_PARITY_ODD ):         return( BS_PAR_Odd );
  case( MNCC_PARITY_EVEN ):        return( BS_PAR_Even );
  case( MNCC_PARITY_FORCED_TO_0 ): return( BS_PAR_Space );
  case( MNCC_PARITY_FORCED_TO_1 ): return( BS_PAR_Mark );
  case( MNCC_PARITY_NONE ):
  default:                    return( BS_PAR_NotPresent );
  }

}
#endif /* FAX_AND_DATA */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : cmhCC_GetCallClass      |
+-------------------------------------------------------------------+

  PURPOSE : get call class out of bearer service

*/

GLOBAL T_ACI_CLASS cmhCC_GetCallClass ( SHORT cId )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

  switch( ctb->BC[ctb->curBC].bearer_serv )
  {
  case( MNCC_BEARER_SERV_SPEECH_CTM ):
  case( MNCC_BEARER_SERV_SPEECH ):     return( CLASS_Vce );
  case( MNCC_BEARER_SERV_AUX_SPEECH_CTM ):
  case( MNCC_BEARER_SERV_AUX_SPEECH ): return( CLASS_AuxVce );
  case( MNCC_BEARER_SERV_FAX    ):     return( CLASS_Fax );
  case( MNCC_BEARER_SERV_SYNC   ):     return( CLASS_Dat );
  case( MNCC_BEARER_SERV_ASYNC  ):     return( CLASS_Dat );
  default:                        return( CLASS_NotPresent );
  }

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : cmhCC_ctbGetCldNumTyp   |
+-------------------------------------------------------------------+

  PURPOSE : this function builds the type of address information out
            of the called party information.
*/

GLOBAL T_ACI_TOA* cmhCC_ctbGetCldNumTyp ( SHORT cId, T_ACI_TOA* pToaBuf )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
/*
 *-------------------------------------------------------------------
 * build type of address information
 *-------------------------------------------------------------------
 */
  if (ctb->cldPty.ton EQ MNCC_TON_NOT_PRES )

    return( NULL );

  pToaBuf -> ton = (T_ACI_TOA_TON)ctb->cldPty.ton;
  pToaBuf -> npi = (T_ACI_TOA_NPI)ctb->cldPty.npi;

  return( pToaBuf );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : cmhCC_ctbGetCldSubTyp   |
+-------------------------------------------------------------------+

  PURPOSE : this function builds the type of subaddress information
            out of the called party information.
*/

GLOBAL T_ACI_TOS* cmhCC_ctbGetCldSubTyp ( SHORT cId, T_ACI_TOS * pTosBuf )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
/*
 *-------------------------------------------------------------------
 * build type of subaddress information
 *-------------------------------------------------------------------
 */
  if( ctb->cldPtySub.tos EQ MNCC_TOS_NOT_PRES )

    return( NULL );

  pTosBuf -> tos = (T_ACI_TOS_TOS)ctb->cldPtySub.tos;
  pTosBuf -> oe  = (T_ACI_TOS_OE)ctb->cldPtySub.odd_even;

  return( pTosBuf );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : cmhCC_ctbGetClrNumTyp   |
+-------------------------------------------------------------------+

  PURPOSE : this function builds the type of address information out
            of the calling party information.
*/

GLOBAL T_ACI_TOA* cmhCC_ctbGetClrNumTyp ( SHORT cId, T_ACI_TOA* pToaBuf )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
/*
 *-------------------------------------------------------------------
 * build type of address information
 *-------------------------------------------------------------------
 */
  if (ctb->clgPty.ton EQ MNCC_TON_NOT_PRES )

    return( NULL );

  pToaBuf -> ton = (T_ACI_TOA_TON)ctb->clgPty.ton;
  pToaBuf -> npi = (T_ACI_TOA_NPI)ctb->clgPty.npi;

  return( pToaBuf );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : cmhCC_ctbGetClrSubTyp   |
+-------------------------------------------------------------------+

  PURPOSE : this function builds the type of subaddress information
            out of the calling party information.
*/

GLOBAL T_ACI_TOS* cmhCC_ctbGetClrSubTyp ( SHORT cId, T_ACI_TOS * pTosBuf )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
/*
 *-------------------------------------------------------------------
 * build type of subaddress information
 *-------------------------------------------------------------------
 */
  if( ctb->clgPtySub.tos EQ MNCC_TOS_NOT_PRES )

    return( NULL );

  pTosBuf -> tos = (T_ACI_TOS_TOS)ctb->clgPtySub.tos;
  pTosBuf -> oe  = (T_ACI_TOS_OE)ctb->clgPtySub.odd_even;

  return( pTosBuf );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : cmhCC_ctbGetRdrNumTyp   |
+-------------------------------------------------------------------+

  PURPOSE : this function builds the type of address information out
            of the redirecting party information.
*/

GLOBAL T_ACI_TOA* cmhCC_ctbGetRdrNumTyp ( SHORT cId, T_ACI_TOA* pToaBuf )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
/*
 *-------------------------------------------------------------------
 * build type of address information
 *-------------------------------------------------------------------
 */
  if( ctb->rdrPty.ton EQ MNCC_TON_NOT_PRES )

    return( NULL );

  pToaBuf -> ton = (T_ACI_TOA_TON)ctb->rdrPty.ton;
  pToaBuf -> npi = (T_ACI_TOA_NPI)ctb->rdrPty.npi;

  return( pToaBuf );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : cmhCC_ctbGetRdrSubTyp   |
+-------------------------------------------------------------------+

  PURPOSE : this function builds the type of subaddress information
            out of the redirecting party information.
*/

GLOBAL T_ACI_TOS* cmhCC_ctbGetRdrSubTyp ( SHORT cId, T_ACI_TOS * pTosBuf )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
/*
 *-------------------------------------------------------------------
 * build type of subaddress information
 *-------------------------------------------------------------------
 */
  if( ctb->rdrPtySub.tos EQ MNCC_TOS_NOT_PRES )

    return( NULL );

  pTosBuf -> tos = (T_ACI_TOS_TOS)ctb->rdrPtySub.tos;
  pTosBuf -> oe  = (T_ACI_TOS_OE)ctb->rdrPtySub.odd_even;

  return( pTosBuf );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCR                      |
|                            ROUTINE : cmhrat_no_carrier            |
+-------------------------------------------------------------------+

  PURPOSE : Send NO CARRIER to user (saves memory in comparaison to R_AT).
            It also send a log to MFW, and sends NO CARRIER to the owner
            of the call if it is necessary (owner different from call killer).

            It also sends a DCD off to UART.

*/

GLOBAL void cmhrat_calldisc( RAT_ID response, UBYTE srcId, T_ACI_AT_CMD cmdId, SHORT cId )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

  TRACE_FUNCTION("cmhrat_calldisc");

  /* CHECK response: THIS ONLY WORKS BECAUSE R_AT has the same parameters
  for RAT_NO_CARRIER, RAT_BUSY and RAT_NO_ANSWER */
  switch( response )
  {
  case( RAT_NO_CARRIER ):
  case( RAT_BUSY ):
  case( RAT_NO_ANSWER ):
    break;
  default:
    return;
  }

  if( srcId EQ (UBYTE)CMD_SRC_NONE       OR
      srcId EQ ctb->calOwn )
  {
    /* inform only call owner */

    R_AT( response, (T_ACI_CMD_SRC)ctb->calOwn ) ( AT_CMD_NONE, cId+1 );
  }
  else
  {
    /* inform both call owner and call killer */
    R_AT( response, (T_ACI_CMD_SRC)ctb->calOwn ) ( AT_CMD_NONE, cId+1 );
    R_AT( response, (T_ACI_CMD_SRC)srcId ) ( cmdId, cId+1 );
  }

#ifdef FF_ATI
  /* tlu:io_setDCD (ctb->calOwn, IO_DCD_OFF); */ /* V.24 DCD Line */
#endif

  cmh_logRslt ( (T_ACI_CMD_SRC)srcId, response, cmdId, (SHORT)(cId+1), 
                                                    BS_SPEED_NotPresent,CME_ERR_NotPresent);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : cmhCC_SndDiscRsn        |
+-------------------------------------------------------------------+

  PURPOSE : Find out the reason for disconnection and send it to the
            caller.

*/

GLOBAL void cmhCC_SndDiscRsn ( SHORT cId )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
  UBYTE cmdBuf;     /* buffers current command */
  UBYTE srcBuf;     /* buffers current command source */

  TRACE_FUNCTION ("cmhCC_SndDiscRsn()");

  cmdBuf = ctb->curCmd;
  srcBuf = ctb->curSrc;

  if (rdlPrm.rdlcId EQ NO_ENTRY)
  {
    cmhCC_PrepareCmdEnd (cId, NULL, NULL);
  }

  if( cmdBuf EQ AT_CMD_NONE )
  {
    srcBuf = ctb->calOwn; /* call owner is the only one that
                             could possibly be informed */
  }

  if( cmdBuf EQ AT_CMD_ABRT )
  {
    R_AT( RAT_OK, (T_ACI_CMD_SRC)srcBuf )( cmdBuf );
    /* log result */
    cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_OK,
                  (T_ACI_AT_CMD)cmdBuf, (SHORT)(cId+1), BS_SPEED_NotPresent,CME_ERR_NotPresent );
    return;
  }

/*
 *-------------------------------------------------------------------
 * check for reject cause
 *-------------------------------------------------------------------
 */
  /* 
   * Make use of ctb here or die. Problem with TI compiler 1.22e.
   * Seen. Exactly here. GET_CAUSE_VALUE() expression seems to become too 
   * complicated without the help of this variable. You may get a wrong 
   * ARM instruction otherwise in the generated code.
   */
  if(GET_CAUSE_VALUE(ctb->rejCs) NEQ NOT_PRESENT_8BIT)
  {
    /* when there is no SIM error, send the NO CARRIER command,
       otherwise cmhSIM_GetSIMError send the error command (CME) */
    if (GET_CAUSE_ORIGIN_ENTITY(ctb->rejCs) NEQ SIM_ORIGINATING_ENTITY OR
        cmhSIM_GetSIMError((T_ACI_CMD_SRC)srcBuf, (T_ACI_AT_CMD)cmdBuf) EQ AT_FAIL )
    {
      cmhrat_calldisc(RAT_NO_CARRIER, srcBuf, (T_ACI_AT_CMD)cmdBuf, cId);
      return;
    }
    else
    {
      cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_CME,
                   (T_ACI_AT_CMD)cmdBuf, (SHORT)(cId+1), BS_SPEED_NotPresent,CME_ERR_NotPresent  );
    }
    return;
  }

/*
 *-------------------------------------------------------------------
 * otherwise check for normal cause
 *-------------------------------------------------------------------
 */
  switch( ctb->nrmCs )
  {
  case( MNCC_CAUSE_USER_BUSY ):
    cmhrat_calldisc(RAT_BUSY, srcBuf, (T_ACI_AT_CMD)cmdBuf, cId);
    break;

  case( MNCC_CAUSE_NO_RESPONSE ):
  case( MNCC_CAUSE_ALERT_NO_ANSWER ):
    cmhrat_calldisc(RAT_NO_ANSWER, srcBuf, (T_ACI_AT_CMD)cmdBuf, cId);
    break;

  default:
    cmhrat_calldisc(RAT_NO_CARRIER, srcBuf, (T_ACI_AT_CMD)cmdBuf, cId);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : cmhCC_ChckInCallMdfy    |
+-------------------------------------------------------------------+

  PURPOSE : check the case of in-call modification. The function
            returns TRUE or FALSE, whether a modification is allowed
            or not.

*/

GLOBAL BOOL cmhCC_ChckInCallMdfy ( SHORT cId, UBYTE cmd )
{
#ifdef FAX_AND_DATA
  T_CC_CALL_TYPE calltype = cmhCC_getcalltype(cId);

  TRACE_FUNCTION ("cmhCC_ChckInCallMdfy()");

#ifdef CO_UDP_IP
  if(calltype EQ UDPIP_CALL)
  {
    /* modifying a Wap Call should not be allowed */
    return( FALSE);
  }
#endif /* CO_UDP_IP */

  /* check call mode */
  switch( psaCC_ctb(cId)->rptInd )
  {
    /* single call */
    case( MNCC_RI_NOT_PRES ):
      return( FALSE );

    /* voice followed by data */
    case( MNCC_RI_SEQUENTIAL ):
      if(calltype EQ VOICE_CALL)
      {
        switch( cmd )
        {
          case( AT_CMD_D ):       /* modify commands */
          case( AT_CMD_A ):

            TRACE_EVENT( "MODIFY FROM SPEECH" );
            return( TRUE );
        }
      }
      return( FALSE );

    /* alternating call */
    case( MNCC_RI_CIRCULAR ):
      switch(calltype)
      {
        case(VOICE_CALL):
          switch( cmd )
          {
            case( AT_CMD_D ):       /* modify commands */
            case( AT_CMD_A ):

              TRACE_EVENT( "MODIFY FROM SPEECH" );
              return( TRUE );
          }
          return( FALSE );

        case( TRANS_CALL ):
        case( NON_TRANS_CALL ):
        case( FAX_CALL ):
          if(cmd EQ AT_CMD_H )       /* modify commands */
          {
            TRACE_EVENT( "MODIFY FROM DATA/FAX" );
            return( TRUE );
          }
          break;
      }
      return( FALSE );
  }
#endif /* FAX_AND_DATA */
  return(FALSE);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : cmhCC_atdsendok         |
+-------------------------------------------------------------------+

  PURPOSE : returns TRUE if OK is to be sent directly: in case of ATD,
            if COLP is not set and source is neither LCL nor SAT nor CSSN ...
*/

GLOBAL BOOL cmhCC_atdsendok ( SHORT cId )
{
  T_CC_CALL_TYPE calltype = cmhCC_getcalltype(cId);
  T_ACI_AT_CMD   cmd      = (T_ACI_AT_CMD)psaCC_ctb(cId)->curCmd;
  SHORT          srcId    = psaCC_ctb(cId)->curSrc;
  
#ifdef FF_ATI 
  TRACE_EVENT_P6("cmhCC_atdsendok(): cmd: %d, calltype: %d, srcId: %d, COLP_stat: %d, SATinv: %d, CSSI_stat: %d",
                                     cmd,  calltype, srcId, at.flags.COLP_stat, psaCC_ctb(cId)->SATinv,
                                     ati_user_output_cfg[srcId].CSSI_stat);
#endif /* FF_ATI */ 

#ifdef FF_ATI
  if(cmd NEQ AT_CMD_D) /* can happen ONLY to ATD */
    return(FALSE);

  if (calltype NEQ VOICE_CALL      OR
      srcId EQ CMD_SRC_LCL         OR
      at.flags.COLP_stat EQ 1      OR
      ati_user_output_cfg[srcId].CSSI_stat EQ 1      OR
      ati_is_src_type((UBYTE)srcId, ATI_SRC_TYPE_SAT) OR   /* SAT run at cmd */
      psaCC_ctb(cId)->SATinv )       /* call started by SAT */
  {
    TRACE_EVENT("send OK normally: at the end");
    return(FALSE);   /* OK sent normally: after MNCC_SETUP_CNF has been received */
  }
  else
  {
    TRACE_EVENT("send OK directly after ATD");
    return(TRUE);    /* OK sent as soon as MNCC_SETUP_REQ has been sent (see ITU. V25ter) */
  }
#else
  return(FALSE);
#endif /* AT INTERPRETER */

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : cmhCC_flagCall          |
+-------------------------------------------------------------------+

  PURPOSE : flag a call for multy-call operation

*/

GLOBAL void cmhCC_flagCall ( SHORT cId, USHORT * flags )
{
  *flags |= (0x1 << cId);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : cmhCC_tstAndUnflagCall  |
+-------------------------------------------------------------------+

  PURPOSE : test and unflag a call for multy-call operation. Return
            the test result.

*/

GLOBAL BOOL cmhCC_tstAndUnflagCall ( SHORT cId, USHORT * flags )
{
  if( *flags & (1u << cId))
  {
    *flags &= ~(1u << cId);
    return( TRUE );
  }
  return( FALSE );
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : cmhCC_getcalltype       |
+-------------------------------------------------------------------+

  PURPOSE : returns the type of call of cId (voice, data, fax...).

*/

GLOBAL T_CC_CALL_TYPE cmhCC_getcalltype(SHORT cId)
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

  /* First the call table is checked before the checking for the valid CID */
  if (ctb EQ NULL)
    return (T_CC_CALL_TYPE)ccShrdPrm.callType[cId]; /* Call already gone */

  /* Check whether the cId is valid or not, if not return NO_VLD_CC_CALL_TYPE */
  if (!psaCC_ctbIsValid (cId))
    return(NO_VLD_CC_CALL_TYPE);

  /* determine type of call */
  switch( ctb->BC[ctb->curBC].bearer_serv )
  {
    default:
      return(NO_VLD_CC_CALL_TYPE);

    case( MNCC_BEARER_SERV_AUX_SPEECH ):
    case( MNCC_BEARER_SERV_SPEECH ):     /* voice call */
    case( MNCC_BEARER_SERV_AUX_SPEECH_CTM ):
    case( MNCC_BEARER_SERV_SPEECH_CTM ):     /* voice call with CTM */
      return(VOICE_CALL);

#ifdef FAX_AND_DATA
    case( MNCC_BEARER_SERV_ASYNC ):      /* asynchronous data call */
      switch( ctb->BC[ctb->curBC].conn_elem )
      {
        case( MNCC_CONN_ELEM_TRANS ):    /* transparent data call */
        case( MNCC_CONN_ELEM_NON_TRANS ):  /* non transparent data call */
 /* check if a WAP call is in process... If yes then end call */
#if defined(CO_UDP_IP) OR defined(FF_GPF_TCPIP)
          if (Wap_Call)
          {
            if (cId EQ wapId)
            {
              if(is_gpf_tcpip_call()) {
                GPF_TCPIP_STATEMENT(return(TCPIP_CALL));
              }
              else {
                UDPIP_STATEMENT(return(UDPIP_CALL));
              }
            }
          }
#endif  /* CO_UDP_IP || FF_GPF_TCPIP */
#ifdef FF_PPP
          if (pppShrdPrm.is_PPP_CALL EQ TRUE)
          {
            return (PPP_CALL);
          }
#endif
          if( ctb->BC[ctb->curBC].conn_elem EQ MNCC_CONN_ELEM_TRANS )
          {
            return(TRANS_CALL);
          }
          else
            return(NON_TRANS_CALL);
      }
      return(NO_VLD_CC_CALL_TYPE);

    case( MNCC_BEARER_SERV_FAX ):          /* fax call */
      return(FAX_CALL);

    case( MNCC_BEARER_SERV_SYNC  ):
    case( MNCC_BEARER_SERV_PAD_ACCESS  ):
    case( MNCC_BEARER_SERV_PACKET_ACCESS ):
      return(UNSUP_DATA_CALL);
#endif /* of #ifdef FAX_AND_DATA */
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCF                 |
|                                 ROUTINE : cmhCC_AcceptCall        |
+-------------------------------------------------------------------+

  PURPOSE : accept a call.
*/

GLOBAL void cmhCC_AcceptCall(SHORT cId, T_ACI_CMD_SRC srcId, T_ACI_AT_CMD cmd)
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
  UBYTE idx;                  /* holds index value */

  TRACE_FUNCTION("cmhCC_AcceptCall( )");

  if (ctb EQ NULL OR ctb->calStat NEQ CS_ACT_REQ)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown );
    R_AT(RAT_CME, srcId)(cmd, CME_ERR_Unknown);
    return;
  }

  CHLDaddInfo = NO_CHLD_ADD_INFO;
  ctb->curCmd = cmd;
  ctb->calOwn = (T_OWN) srcId;
  ctb->curSrc = srcId;

  /*
   * ETSI 11.10 26.12.4 requires "that for speech calls, the mobile station
   * shall attach the user connection at latest when sending the connect
   * message, except if there is no compatible radio resource available at
   * this time."
   */

  psaCC_phbAddNtry ( LRN, cId, CT_MTC, NULL );  /* add call to LRN PB */

#ifdef FF_ATI
  io_setRngInd ( IO_RING_OFF, CRING_SERV_TYP_NotPresent, CRING_SERV_TYP_NotPresent ); /* V.24 Ring Indicator Line */
#endif

  for( idx = 0; idx < CMD_SRC_MAX; idx++ )
  {
    R_AT( RAT_CRING_OFF,(T_ACI_CMD_SRC) idx )( cId+1 );
  }

  psaCC_AcceptCall(cId);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCF                 |
|                                 ROUTINE : cmhCC_RetrieveCall      |
+-------------------------------------------------------------------+

  PURPOSE : Retrieve either a Multyparty or a Normal Call
*/
GLOBAL void cmhCC_RetrieveCall(SHORT cId, T_ACI_CMD_SRC srcId)
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

  TRACE_FUNCTION("cmhCC_RetrieveCall( )");

  if (ctb EQ NULL OR ctb->calStat NEQ CS_HLD)
  {
    return;
  }

  CHLDaddInfo = NO_CHLD_ADD_INFO;
  ctb->curCmd = AT_CMD_CHLD;
  ctb->curSrc = srcId;

  if (ctb->mptyStat EQ CS_ACT )
  {
    psaCC_Hold_RetrieveMPTY (cId, CS_HLD, CS_ACT_REQ, MNCC_MPTY_RETRIEVE_SENT, 
                             OPC_RETRIEVE_MPTY);
  }
  else
  {
    psaCC_RetrieveCall (cId);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCF                 |
|                                 ROUTINE : cmhCC_NewCall           |
+-------------------------------------------------------------------+

  PURPOSE : Initiate a new call.
*/
GLOBAL void cmhCC_NewCall(SHORT cId, T_ACI_CMD_SRC srcId, T_ACI_AT_CMD cmd)
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

  TRACE_FUNCTION("cmhCC_NewCall( )");

  CHLDaddInfo = NO_CHLD_ADD_INFO;
  ctb->curCmd = cmd;
  ctb->calOwn = (T_OWN)srcId;
  ctb->curSrc = srcId;

  psaCC_NewCall(cId);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCF                 |
|                                 ROUTINE : cmhCC_HoldCall          |
+-------------------------------------------------------------------+

  PURPOSE : Put a call on hold (multyparty or normal call)
*/
GLOBAL void cmhCC_HoldCall(SHORT cId, T_ACI_CMD_SRC srcId, T_ACI_AT_CMD cmd)
{
  T_CC_CALL_TBL  *ctb;         /* points to CC call table */
  T_CC_CMD_PRM   *pCCCmdPrm;   /* points to CC command parameters */

  TRACE_FUNCTION("cmhCC_HoldCall( )");

  ctb = ccShrdPrm.ctb[cId];
  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;

  cmhCC_flagCall( cId, &(pCCCmdPrm->mltyCncFlg));
  ctb->curCmd = cmd;
  ctb->curSrc = srcId;

  if (ctb->mptyStat EQ CS_ACT)
  {
    psaCC_Hold_RetrieveMPTY (cId, CS_ACT, CS_HLD_REQ, MNCC_MPTY_HOLD_SENT, 
                             OPC_HOLD_MPTY);
  }
  else
  {
    psaCC_HoldCall(cId);
  }
}
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCF                 |
|                                 ROUTINE : cmhCC_ClearCall         |
+-------------------------------------------------------------------+

  PURPOSE : disconnect a call.
*/

GLOBAL void cmhCC_ClearCall ( SHORT cId,
                              USHORT cs,
                              T_ACI_CMD_SRC srcId,
                              UBYTE cmd,
                              SHORT *waitId )
{
  T_CC_CMD_PRM   *pCCCmdPrm = NULL;   /* points to CC command parameters */
  T_CC_CALL_TYPE calltype = cmhCC_getcalltype(cId);
  T_CC_CALL_TBL  *ctb = ccShrdPrm.ctb[cId];

  TRACE_FUNCTION( "cmhCC_ClearCall()" );

  if(srcId > CMD_SRC_NONE AND
     srcId < CMD_SRC_MAX)
  {
    pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;
  }

  if (ccShrdPrm.ctb[cId] EQ NULL)
  {
    TRACE_EVENT("cmhCC_ClearCall(): Not a valid cId");
    return;
  }

  TRACE_EVENT_P2("Clear Call ==> calStat: %d, calType: %d", ctb->calStat, ctb->calType NEQ CT_MOC);

  if(cmd NEQ AT_CMD_NONE) /* for instance +CHUP / H / Z */
  {
    /* Check whether an incoming call is rejected by the user. */
    if( ctb->calStat EQ CS_ACT_REQ AND
        ctb->calType EQ CT_MTC)
    {
      psaCC_phbAddNtry ( LRN, cId, CT_MTC, NULL );  /* Add call to Last Received Number (LRN) PB */
    }

    /* Check whether call is a waiting call */
    if(waitId NEQ NULL)
    {
      if( ctb->calStat EQ CS_ACT_REQ AND
          ctb->calType EQ CT_MTC     AND
          ctb->calOwn  EQ ((T_OWN)CMD_SRC_NONE) )
      {
        /* tag a found waiting call */
        TRACE_EVENT_P1("Waiting Call Found ! waitId=%d", cId);
        *waitId = cId;
        return;
      }
    }
  }

  ccShrdPrm.cIdFail = cId;
  if ( cmd NEQ AT_CMD_NONE ) /* this will decide whether a result is sent to the user or not */
  {
    /* ATZ and ATH or AT+CHUP for instance */
    if(pCCCmdPrm NEQ NULL)
    {
      cmhCC_flagCall( cId, &(pCCCmdPrm -> mltyDscFlg));
    }
    ctb->curCmd = cmd;
    ctb->curSrc = srcId;
  }
  else
  {
    /* AOC for instance */
    cmhCC_SndDiscRsn(cId);
  }
  ctb->nrmCs     = cs;

  /* determine call status */
  switch( ctb->calStat )
  {
    /* active calls */
    case( CS_ACT ):
      switch( calltype )
      {
        case( VOICE_CALL ):
          psaCC_ClearCall (cId);
          break;

#if defined FAX_AND_DATA AND defined (DTI)

        case( TRANS_CALL ):
          ccShrdPrm.datStat = DS_DSC_REQ;
          cmhTRA_Deactivate ();
          break;

        case( NON_TRANS_CALL ):
          ccShrdPrm.datStat = DS_DSC_REQ;
          cmhL2R_Deactivate ();
          break;

#ifdef FF_FAX
        case( FAX_CALL ):
          ccShrdPrm.datStat = DS_DSC_REQ;
          cmhT30_Deactivate ();
          break;
#endif /* FF_FAX */

#if defined(CO_UDP_IP) OR defined(FF_GPF_TCPIP)

#ifdef CO_UDP_IP
        case( UDPIP_CALL ):
#endif

#ifdef FF_GPF_TCPIP
        case( TCPIP_CALL ):
          {
            T_DCM_STATUS_IND_MSG msg;
            msg.hdr.msg_id = DCM_ERROR_IND_MSG;
            msg.result     = DCM_PS_CONN_BROKEN;
            dcm_send_message(msg, DCM_SUB_WAIT_SATDN_CNF);
          }
#endif
          ccShrdPrm.datStat = DS_DSC_REQ;
          /* also test whether a SAT E connection has to be shut down */
          if ( (cmd NEQ AT_CMD_NONE)
#ifdef FF_SAT_E
              OR (satShrdPrm.opchAcptSrc EQ srcId)
#endif
             )
          {
            /* set the last possible state for the next "else if" statement */
            T_ACI_WAP_STATES last_state = Wap_Not_Init;
            if(is_gpf_tcpip_call()) {
              GPF_TCPIP_STATEMENT(last_state = TCPIP_Deactivation);
            }
            else {
              last_state = UDPA_Deactivation;
            }

            /* Terminate PPP if established. UP indicate that entities under PPP are still activ. */
            if((pppShrdPrm.state EQ PPP_ESTABLISHED) OR
               (pppShrdPrm.state EQ PPP_ESTABLISH))
            {
              cmhPPP_Terminate(UP);
            }
            else if (wap_state >  Wap_Not_Init  AND
                     wap_state <= last_state)
            {
              cmhPPP_Terminated(); /* Trigger termination of SCPIP if PPP did not yet start (ACI-FIX-9317) */
            }
          }
          break;
#endif  /* defined(CO_UDP_IP) OR defined(FF_GPF_TCPIP) */
#if defined (FF_PPP) AND defined (DTI)
        case( PPP_CALL ):
          ccShrdPrm.datStat = DS_DSC_REQ;
          if (pppShrdPrm.is_PPP_CALL EQ TRUE)
          {
            dti_cntrl_close_dpath_from_src_id((UBYTE)srcId);
          }
#endif    /* DTI */
#endif    /* of #ifdef FAX_AND_DATA */
        default:
            TRACE_ERROR("Wrong call type");
    }
      break;

      /* other present calls */
    case( CS_HLD ):
    case( CS_HLD_REQ ):
    case( CS_MDF_REQ ):
    case( CS_DSC_REQ ):
    case( CS_ACT_REQ ):
    case( CS_CPL_REQ ):
      psaCC_ClearCall (cId);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCF                  |
| STATE   : code                  ROUTINE : cmhCC_chkDTMFDig         |
+--------------------------------------------------------------------+

  PURPOSE : This function is used extract DTMF digits from the dialled
            number. The DTMF digits, including separators, are stored
            in the DTMF parameters and will be send automatically after
            the connection is established.
            BOOL within_dial_string parameter to tell if DTMF tones are
            within a dial string or directly given.
            USHROT length: possibility to give the number of dtmf to be
            sent (relevant only if not within a dial string).
*/

/* next function fills up dtmf structure with extracted prms */
LOCAL void cmhCC_fillDTMFDig ( SHORT cId, CHAR *dtmf, USHORT dtmf_length )
{
  TRACE_FUNCTION("cmhCC_fillDTMFDig()");

  if( dtmf NEQ NULL )
  {
    ccShrdPrm.dtmf.cnt = MINIMUM( dtmf_length, MAX_DTMF_DIG );

    strncpy( (char *)ccShrdPrm.dtmf.dig, dtmf, ccShrdPrm.dtmf.cnt );
    ccShrdPrm.dtmf.dig[dtmf_length] = '\0';

    ccShrdPrm.dtmf.cur = 0;
    ccShrdPrm.dtmf.cId = cId;
    TRACE_EVENT_P2("ccShrdPrm.dtmf.cnt: %d, ccShrdPrm.dtmf.cId: %d", ccShrdPrm.dtmf.cnt, ccShrdPrm.dtmf.cId);
  }
  else
  {
    ccShrdPrm.dtmf.cnt = 0;
    ccShrdPrm.dtmf.cur = 0;
    ccShrdPrm.dtmf.cId = NO_ENTRY;
  }
}

GLOBAL BOOL is_digit_dtmf_separator(CHAR digit)
{
  switch(digit)
  {
    case('P'):  /* PHB_DIAL_PAUSE, PHB_DTMF_SEP */
    case('p'):
    case('W'):  /* PHB_DIAL_WAIT */
    case('w'):
      TRACE_EVENT("DTMF Separator Found");
      return(TRUE);
  }
  return(FALSE);
}

LOCAL CHAR * find_dtmf_separator(CHAR *number_string, USHORT length)
{
  int i;

  TRACE_FUNCTION("find_dtmf_separator()");

  for(i=0; i<length; i++)
  {
    if(is_digit_dtmf_separator(*number_string))
    {
      return(number_string);
    }
    number_string++;
  }
  return(NULL);
}

GLOBAL void cmhCC_chkDTMFDig ( CHAR*  num,
                               SHORT  cId,
                               USHORT length,
                               BOOL   within_dial_string )
{
  CHAR   *dtmf = num;
  USHORT len   = length;

  TRACE_FUNCTION("cmhCC_chkDTMFDig()");

  if( within_dial_string )
  {
    dtmf = find_dtmf_separator(num, (USHORT)strlen(num)); /*strchr( num, PHB_DTMF_SEP ); */

    if(dtmf NEQ NULL)
    {
      len = strlen(dtmf);
      TRACE_EVENT_P1("Call number string contains %d DTMF tones", len);
    }
  }
  else
  {
    /* already inialized for the case "not within a phone call" (e.g SAT Send DTMF) */
  }

  cmhCC_fillDTMFDig ( cId, dtmf, len );

  if(dtmf NEQ NULL)
  {
    *dtmf = '\0';  /* split dial string from DTMF tones */
  }

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCF                  |
| STATE   : code                  ROUTINE : cmhCC_chkKeySeq          |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to decode a keystroke sequence and
            to initiate an appropriate action.
*/
GLOBAL T_ACI_RETURN cmhCC_chkKeySeq ( T_ACI_CMD_SRC srcId,
                                      T_CLPTY_PRM *cldPty,
                                      T_ACI_D_TOC *callType,
                                      T_ACI_D_CLIR_OVRD *CLIRovrd,
                                      T_CC_SIM_QLF ccSIMQlf )
{
  T_KSD_SEQGRP   seqGrp = SEQGRP_UNKNOWN;
  T_KSD_SEQPARAM seqPrm;
  CHAR          *restSeq = NULL;
  T_ACI_RETURN   retVal;
  T_PHB_RECORD   phbNtry;
  T_PHB_TYPE     slctPHB;
#ifdef SIM_TOOLKIT
  T_sat_ussd     SATussd; /* to hold USSD string in case of SAT Control */
#endif /* of SIM_TOOLKIT */

  TRACE_FUNCTION ("cmhCC_chkKeySeq ()");

  if( cldPty EQ NULL         OR
    strlen (cldPty->num) EQ 0 )
  {
    return( AT_CMPL );
  }

  restSeq = cldPty->num;
  seqGrp  = SEQGRP_UNKNOWN;

#ifdef SIM_TOOLKIT
  /* check for SAT SIM Call Control of SS strings */
  if (ccSIMQlf EQ CC_SIM_YES)
  {
    if ( !ksd_isFDNCheckSeq ( cldPty-> num ) )
      ;    /* bypass *#06# from SSCntrlBySIM */
    else
    {
      retVal = AT_CMPL; /* init */
      if( ksd_isSATSscs (cldPty->num) )
      {
        retVal = cmhSAT_SSCntrlBySIM( cldPty, (UBYTE)srcId );
      }
      else if( ksd_isUSSD (cldPty->num) )
      {
        if (psaSIM_ChkSIMSrvSup(SRV_USSDsupportInCC))
        {
          SATussd.dcs        = 0x0F; /* default alphabet see GSM 03.38 Cell Broadcast dcs */
          SATussd.c_ussd_str = strlen(cldPty->num);
          SATussd.ussd_str   = (UBYTE *)cldPty->num;

          retVal = cmhSAT_USSDCntrlBySIM( &SATussd, (UBYTE)srcId );
        }
        else
        {
          retVal = cmhSAT_SSCntrlBySIM( cldPty, (UBYTE)srcId );
        }
      }

      if( retVal NEQ AT_CMPL )
        return( retVal );
    }
  }
#endif

/*
 *-------------------------------------------------------------
 * decode SS string
 *-------------------------------------------------------------
 */
  if (ksd_decode (cldPty->num,
                  psaCC_ctbAnyCallInUse(),  /* is there any call??? */
                  &seqGrp,
                  &restSeq,
                  &seqPrm) EQ TRUE)
  {
    retVal  = AT_FAIL;

  /*
   *-------------------------------------------------------------
   * perform desired action
   *-------------------------------------------------------------
   */
    switch (seqGrp)
    {
      /*
       *---------------------------------------------------------
       * handling of sequence group SEQGRP_IMEI
       *---------------------------------------------------------
       */
      case ( SEQGRP_PRSNT_IMEI ):
        retVal = cmhSS_ksdIMEI( srcId );
        break;

      /*
       *---------------------------------------------------------
       * handling of sequence group SEQGRP_DIAL
       *---------------------------------------------------------
       */
      case (SEQGRP_DIAL):
        strncpy(cldPty->num, seqPrm.dial.number, sizeof (cldPty->num) - 1);
        cldPty->num[sizeof(cldPty->num) - 1] = '\0';
        return ((T_ACI_RETURN) AT_CONT);

      /*
       *---------------------------------------------------------
       * handling of sequence group SEQGRP_CHLD
       *---------------------------------------------------------
       */
      case (SEQGRP_CHLD):
        retVal = sAT_PlusCHLD( srcId, seqPrm.chld.mode, seqPrm.chld.call );
        if (retVal NEQ AT_FAIL)
          break; /* successfull CHLD */

        /* else handle as USSD if this CHLD was unexpected/illegal */

        seqPrm.ussd.ussd = (UBYTE *)cldPty->num;
        seqGrp  = SEQGRP_USSD;
        restSeq = cldPty->num + strlen (cldPty->num);
        /* fallthrough */
      /*lint -fallthrough*/
      /*
       *---------------------------------------------------------
       * handling of sequence group SEQGRP_USSD
       *---------------------------------------------------------
       */
      case (SEQGRP_USSD):
        retVal = cmhSS_ksdUSSD( srcId, &seqPrm.ussd );
        break;

      /*
       *---------------------------------------------------------
       * handling of sequence group SEQGRP_DIAL_IDX
       *---------------------------------------------------------
       */
      case (SEQGRP_DIAL_IDX):

        if( !cmhPHB_cvtPhbType(cmhPrm[srcId].phbCmdPrm.cmhStor,
                               &slctPHB ))
          return( AT_FAIL );

#ifdef TI_PS_FFS_PHB
        if (pb_read_record (slctPHB, 
                            seqPrm.dialIdx.index, 
                            &phbNtry) EQ PHB_OK)
#else
        if( pb_read_phys_record( slctPHB, seqPrm.dialIdx.index, &phbNtry )
               EQ PHB_OK )
#endif
        {
          cmhPHB_getAdrStr( cldPty->num, sizeof (cldPty->num) - 1,
                            phbNtry.number, phbNtry.len );

          cmh_demergeTOA ( phbNtry.ton_npi, &cldPty->ton, &cldPty->npi );

          /*
           *---------------------------------------------------------
           * as long as the bearer capabilities are not read from
           * the phonebook only voice calls are supported
           *---------------------------------------------------------
           */
          *callType = D_TOC_Voice;

          return((T_ACI_RETURN) AT_CONT);
        }

        return( AT_FAIL );

      /*
       *---------------------------------------------------------
       * handling of sequence groups which relates
       * to call forwarding parameter
       *---------------------------------------------------------
       */
      case (SEQGRP_CF):
        /* type of address has already been set,
        and a possible "+" already been removed... */
        if(seqPrm.cf.num NEQ NULL AND
           seqPrm.cf.num[0] NEQ 0)   /* only set if there is actually a number */
        {
          seqPrm.cf.ton = cldPty->ton;
          seqPrm.cf.npi = cldPty->npi;
        }
        else
        {
          seqPrm.cf.ton = MNCC_TON_NOT_PRES;
          seqPrm.cf.npi = MNCC_NPI_NOT_PRES;
        }

        /* subaddress has also already been split up if existing */
        if(cldPty->sub[0] NEQ 0)
        {
          seqPrm.cf.sub = (UBYTE *)cldPty->sub;
          seqPrm.cf.tos = cldPty->tos;
          seqPrm.cf.oe  = cldPty->oe;
        }
        else
          seqPrm.cf.sub = NULL;

        retVal = cmhSS_ksdCF( srcId, &seqPrm.cf );
        break;

      /*
       *---------------------------------------------------------
       * handling of sequence groups which relates to
       * call barring parameter
       *---------------------------------------------------------
       */
      case (SEQGRP_CB):
        retVal = cmhSS_ksdCWCBStartTrans(srcId, KSD_CMD_CB, &seqPrm);
        break;

      /*
       *---------------------------------------------------------
       * handling of sequence groups which results in a change
       * of CLIR parameter
       * in temporary mode a call is set up
       *---------------------------------------------------------
       */

      case (SEQGRP_SUP_COLR):
      case (SEQGRP_INV_COLR):
      case (SEQGRP_SUP_COLP):
      case (SEQGRP_INV_COLP):
      case (SEQGRP_SUP_CLIP):
      case (SEQGRP_INV_CLIP):
        if (strlen (restSeq) EQ 0)
        {
          T_ACI_KSD_USSD_PRM ksdString;

          ksdString.ussd = (UBYTE*)cldPty->num;
          retVal = cmhSS_ksdUSSD( srcId, &ksdString );
        }
        else
        {
          /*
           *-----------------------------------------------------
           * temporary mode nonexistant on CLIR, COLR and COLP!!
           * So ignore USSD and continue calling: "Number not assigned"
           * will be indicated to the user by the network
           * This is the common behaviour as found on other phones
           *-----------------------------------------------------
           */
          return((T_ACI_RETURN) AT_CONT);
        }
        break;

      case (SEQGRP_SUP_CLIR):
      case (SEQGRP_INV_CLIR):
        if (strlen (restSeq) EQ 0)
        {
#if defined(MFW) OR defined(FF_MMI_RIV)
          if (srcId EQ CMD_SRC_LCL) /* VO: temp solution because MFW uses sAT_Dn to set CLIR */
            retVal = sAT_PlusCLIR(srcId, seqPrm.Clir.mode);
          else
#endif
          {
            T_ACI_KSD_USSD_PRM ksdString;

            ksdString.ussd = (UBYTE*)cldPty->num;
            retVal = cmhSS_ksdUSSD( srcId, &ksdString );
          }
        }
        else
        {
          /*
           *-----------------------------------------------------
           * in temporary mode a call is set up
           *-----------------------------------------------------
           */
          T_KSD_CLIR   Clir;
          T_KSD_SEQGRP nextGrp  = SEQGRP_UNKNOWN;
          CHAR*        remSeq   = NULL;

          Clir.mode = seqPrm.Clir.mode;

          if (ksd_decode (restSeq, FALSE,
                          &nextGrp, &remSeq,
                          &seqPrm)
              AND
              nextGrp EQ SEQGRP_DIAL)
          {
            *CLIRovrd = (Clir.mode EQ CLIR_MOD_Supp)?
                        D_CLIR_OVRD_Supp: D_CLIR_OVRD_Invoc;
            strncpy(cldPty->num, seqPrm.dial.number, sizeof(cldPty->num) - 1);
            cldPty->num[sizeof(cldPty->num) - 1] = '\0';
            return((T_ACI_RETURN) AT_CONT);
          }
        }
        break;

      /*
       *---------------------------------------------------------
       * handling of sequence group for TTY service
       *---------------------------------------------------------
       */
      case (SEQGRP_TTY_SERV):
        if (strlen (restSeq) EQ 0)
        {
#ifdef FF_TTY
          retVal = sAT_PercentCTTY (srcId, CTTY_MOD_NotPresent,
                                    seqPrm.ctty.req);
#else
          retVal = AT_FAIL;
#endif /* FF_TTY */
        }
        else
        {
          T_KSD_SEQGRP nextGrp  = SEQGRP_UNKNOWN;
          CHAR*        remSeq   = NULL;
#ifdef FF_TTY
          if (seqPrm.ctty.req EQ CTTY_REQ_Off
              AND ccShrdPrm.ctmReq EQ MNCC_CTM_ENABLED)
          {
            ccShrdPrm.ctmOvwr = TRUE;
          }
          else if (seqPrm.ctty.req EQ CTTY_REQ_On
                   AND ccShrdPrm.ctmReq EQ MNCC_CTM_DISABLED)
          {
            ccShrdPrm.ctmOvwr = TRUE;
          }
          else
          {
            ccShrdPrm.ctmOvwr = FALSE;
          }
#endif /* FF_TTY */
          if (ksd_decode (restSeq, FALSE,
                          &nextGrp, &remSeq,
                          &seqPrm)
              AND
              nextGrp EQ SEQGRP_DIAL)
          {
            strncpy(cldPty->num, seqPrm.dial.number, sizeof(cldPty->num) - 1);
            cldPty->num[sizeof(cldPty->num) - 1] = '\0';
           return((T_ACI_RETURN) AT_CONT);
          }
        }
        break;

      /*
       *---------------------------------------------------------
       * handling of sequence group which relates to
       * calling line supplementary services
       *---------------------------------------------------------
       */
      case (SEQGRP_CL):
        retVal = cmhSS_ksdCL( srcId, &seqPrm.cl );
        break;

      /*
       *---------------------------------------------------------
       * handling of sequence groups which results in a
       * registration of passwords
       *---------------------------------------------------------
       */
      case (SEQGRP_PWD):
        retVal = cmhSS_ksdPW( srcId, &seqPrm.pwd );
        break;

      /*
       *---------------------------------------------------------
       * handling of sequence groups which results in a
       * registration of passwords
       *---------------------------------------------------------
       */
      case (SEQGRP_UBLK):
        retVal = cmhSS_ksdUBLK( srcId, &seqPrm.ublk );
        break;

      /*
       *---------------------------------------------------------
       * handling of sequence groups which relates to
       * call waiting
       *---------------------------------------------------------
       */
      case (SEQGRP_CW):
        retVal = cmhSS_ksdCWCBStartTrans(srcId, KSD_CMD_CW, &seqPrm);
        break;

      /*
       *---------------------------------------------------------
       * handling of sequence group SEQGRP_CCBS
       *---------------------------------------------------------
       */
      case (SEQGRP_CCBS):
        retVal = cmhSS_ksdCCBS( srcId, &seqPrm.ccbs );
        break;

      /*
       *---------------------------------------------------------
       * handling of sequence group SEQGRP_UNKNOWN
       *---------------------------------------------------------
       */
      case (SEQGRP_UNKNOWN):
        TRACE_EVENT ("sequence group unknown");
        break;
    }
  }
  else
  {
    TRACE_EVENT ("ksd_decode failed");
    return( AT_FAIL );
  }

  return( retVal );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCF                  |
| STATE   : code                  ROUTINE : cmhCC_fillSetupPrm       |
+--------------------------------------------------------------------+

  PURPOSE : This function fills the call table entry with the
            necessary setup parameters.
*/

GLOBAL T_ACI_RETURN cmhCC_fillSetupPrm ( SHORT              cId,
                                         T_ACI_CMD_SRC      srcId,
                                         T_CLPTY_PRM       *cldPty,
                                         T_MNCC_bcpara          *bc,
                                         UBYTE              prio,
                                         T_ACI_D_CLIR_OVRD  clirOvrd,
                                         T_ACI_D_CUG_CTRL   cugCtrl,
                                         T_ACI_D_TOC        callType )
{
  T_CC_CALL_TBL *pfilled_ctb;
  T_CC_CMD_PRM  *pCCCmdPrm;  /* points to CC command parameters */
#ifdef FF_FAX
  T_T30_CMD_PRM *pT30CmdPrm; /* points to T30 command parameters */
#endif

  TRACE_FUNCTION ("cmhCC_fillSetupPrm()");

  /* initializing local pointers: */
  pCCCmdPrm  = &cmhPrm[srcId].ccCmdPrm;

#ifdef FF_FAX
  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;
#endif

  pfilled_ctb = ccShrdPrm.ctb[cId];

/*
 *-----------------------------------------------------------------
 * check data capabilities
 *-----------------------------------------------------------------
 */
#ifndef FAX_AND_DATA
  if( callType NEQ D_TOC_Voice )
  {
    TRACE_EVENT("No data calls supported");
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_NoDataCallSup );
    return( AT_FAIL );        /* no data calls allowed */
  }
#endif  /* of #ifndef FAX_AND_DATA */

/*
 *-----------------------------------------------------------------
 * called address
 *-----------------------------------------------------------------
 */
  if( cldPty EQ NULL )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );        /* no number */
  }

  /* priority */
  pfilled_ctb->prio = prio;

  if ((prio EQ MNCC_PRIO_NORM_CALL) AND
      (aoc_check_moc() EQ FALSE))
    /*
     * check ACM exceeds ACMmax
     * for non-emergency calls
     */
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ceer, CAUSE_MAKE(DEFBY_STD,
    ORIGSIDE_MS, MNCC_ACI_ORIGINATING_ENTITY, CEER_ACM_Max ));
    return( AT_FAIL );
  }

  /* subaddress */
  pfilled_ctb->cldPtySub.tos      = cldPty->tos;
  pfilled_ctb->cldPtySub.odd_even = cldPty->oe;
  pfilled_ctb->cldPtySub.c_subaddr
    = (UBYTE)utl_dialStr2BCD (cldPty->sub,
                              pfilled_ctb->cldPtySub.subaddr,
                              MNCC_SUB_LENGTH);

  /* address */
  if( pCCCmdPrm -> CSTAdef NEQ TRUE )
  {
    pfilled_ctb->cldPty.npi = pCCCmdPrm -> CSTAtoa.npi;
    pfilled_ctb->cldPty.ton = pCCCmdPrm -> CSTAtoa.ton;
  }
  else
  {
    pfilled_ctb->cldPty.npi = cldPty->npi;
    pfilled_ctb->cldPty.ton = cldPty->ton;
  }

  if (pfilled_ctb->cldPty.called_num NEQ NULL)
  {
    ACI_MFREE (pfilled_ctb->cldPty.called_num);
    pfilled_ctb->cldPty.called_num = NULL;
  }
  pfilled_ctb->cldPty.c_called_num =
    (UBYTE)utl_dialStr2BCD (cldPty->num, NULL, MNCC_MAX_CC_CALLED_NUMBER);
  if (pfilled_ctb->cldPty.c_called_num NEQ 0)
  {
    ACI_MALLOC (pfilled_ctb->cldPty.called_num,
                pfilled_ctb->cldPty.c_called_num);
    (void)utl_dialStr2BCD (cldPty->num,
                           pfilled_ctb->cldPty.called_num,
                           pfilled_ctb->cldPty.c_called_num);
  }

  psaCC_phbSrchNum( cId, CT_MOC);    /* get alpha identifier */

/*
 *-----------------------------------------------------------------
 * CLIR setting
 *-----------------------------------------------------------------
 */
  switch( clirOvrd )
  {
    case( D_CLIR_OVRD_Supp ):
      pfilled_ctb->CLIRsup = MNCC_CLR_SUP;
      break;

    case( D_CLIR_OVRD_Invoc ):
      pfilled_ctb->CLIRsup = MNCC_CLR_SUP_NOT;
      break;

    case( D_CLIR_OVRD_Default ):

      switch( pCCCmdPrm -> CLIRmode )
      {
        case( CLIR_MOD_Subscript ):
          pfilled_ctb->CLIRsup = MNCC_CLR_NOT_PRES;
          break;
        case( CLIR_MOD_Invoc ):
          pfilled_ctb->CLIRsup = MNCC_CLR_SUP_NOT;
          break;
        case( CLIR_MOD_Supp ):
          pfilled_ctb->CLIRsup = MNCC_CLR_SUP;
          break;
      }
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-----------------------------------------------------------------
 * CUG setting
 *-----------------------------------------------------------------
 */
  if ( cugCtrl               EQ D_CUG_CTRL_Present OR
       pCCCmdPrm -> CCUGmode EQ CCUG_MOD_EnableTmp    )
  {
    pfilled_ctb->CUGidx = pCCCmdPrm -> CCUGidx;

    switch( pCCCmdPrm -> CCUGinfo )
    {
      case( CCUG_INFO_No ):
        break;
      case( CCUG_INFO_SuppOa ):
        pfilled_ctb->OAsup = TRUE;
        break;
      case( CCUG_INFO_SuppPrefCug ):
        pfilled_ctb->CUGprf = TRUE;
        break;
      case( CCUG_INFO_SuppBoth ):
        pfilled_ctb->OAsup = TRUE;
        pfilled_ctb->CUGprf = TRUE;
        break;
    }
  }

#ifdef SIM_TOOLKIT
/*
 *-----------------------------------------------------------------
 * SAT settings
 *-----------------------------------------------------------------
 */

  /* decides if a disconnect event for a call has already been sent to SAT */
  pfilled_ctb->SatDiscEvent = FALSE;
#endif

/*
 *-----------------------------------------------------------------
 * bearer services
 *-----------------------------------------------------------------
 */
  switch( callType )
  {
    /*
     *-----------------------------------------------------------------
     * start with a voice call
     *-----------------------------------------------------------------
     */
    case( D_TOC_Voice ):

      pfilled_ctb->BC[0].rate        = MNCC_UR_NOT_PRES;
      pfilled_ctb->BC[0].bearer_serv = cmhCC_set_speech_serv (pCCCmdPrm);
      pfilled_ctb->BC[0].conn_elem   = MNCC_CONN_ELEM_NOT_PRES;

      switch( ccShrdPrm.CMODmode )
      {

      case( CMOD_MOD_Single ):             /* single voice call */
        pfilled_ctb->rptInd          = MNCC_RI_NOT_PRES;

        pfilled_ctb->BC[1].rate        = MNCC_UR_NOT_PRES;
        pfilled_ctb->BC[1].bearer_serv = MNCC_BEARER_SERV_NOT_PRES;
        pfilled_ctb->BC[1].conn_elem   = MNCC_CONN_ELEM_NOT_PRES;
        break;
#if defined  (UART) AND defined (FAX_AND_DATA)  
      case( CMOD_MOD_VoiceDat ):           /* alternating voice/data call */
        pfilled_ctb->rptInd          = MNCC_RI_CIRCULAR;

        pfilled_ctb->BC[1].rate        = cmhCC_SelRate( ccShrdPrm.CBSTspeed );
        pfilled_ctb->BC[1].bearer_serv = cmhCC_SelServ( ccShrdPrm.CBSTname );
        pfilled_ctb->BC[1].conn_elem   = cmhCC_SelCE  ( ccShrdPrm.CBSTce );
        pfilled_ctb->BC[1].modem_type  = cmhCC_SelMT  ( ccShrdPrm.CBSTspeed );

        pfilled_ctb->BC[1].stop_bits   = cmhCC_SelStopBit( srcId );
        pfilled_ctb->BC[1].data_bits   = cmhCC_SelDataBit( srcId );
        pfilled_ctb->BC[1].parity      = cmhCC_SelParity ( srcId );
        break;

      case( CMOD_MOD_VoiceFlwdDat ):       /* voice followed by data */
        pfilled_ctb->rptInd          = MNCC_RI_CIRCULAR;

        pfilled_ctb->BC[1].rate        = cmhCC_SelRate( ccShrdPrm.CBSTspeed );
        pfilled_ctb->BC[1].bearer_serv = cmhCC_SelServ( ccShrdPrm.CBSTname );
        pfilled_ctb->BC[1].conn_elem   = cmhCC_SelCE(   ccShrdPrm.CBSTce );
        pfilled_ctb->BC[1].modem_type  = cmhCC_SelMT  ( ccShrdPrm.CBSTspeed );

        pfilled_ctb->BC[1].stop_bits   = cmhCC_SelStopBit( srcId );
        pfilled_ctb->BC[1].data_bits   = cmhCC_SelDataBit( srcId );
        pfilled_ctb->BC[1].parity      = cmhCC_SelParity ( srcId );
        break;

      case( CMOD_MOD_VoiceFax ):           /* alternating voice/fax call */
        pfilled_ctb->rptInd          = MNCC_RI_CIRCULAR;

        pfilled_ctb->BC[1].rate        = cmhCC_SelRate( ccShrdPrm.CBSTspeed );
        pfilled_ctb->BC[1].bearer_serv = MNCC_BEARER_SERV_FAX;
        pfilled_ctb->BC[1].conn_elem   = MNCC_CONN_ELEM_TRANS;
        pfilled_ctb->BC[1].modem_type  = MNCC_MT_NONE;
        break;
#endif
      default:                             /* unexpected call mode */
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_DataCorrupt );
        return( AT_FAIL );
      }
      break;


    /*
     *-----------------------------------------------------------------
     * start with a data call
     *-----------------------------------------------------------------
     */
#ifdef FAX_AND_DATA

      case( D_TOC_Data ):

 switch( ccShrdPrm.CMODmode )
      {
      case( CMOD_MOD_Single ):             /* single data call */
        pfilled_ctb->rptInd          = MNCC_RI_NOT_PRES;

        pfilled_ctb->BC[0].rate        = cmhCC_SelRate( ccShrdPrm.CBSTspeed );

#ifdef FF_FAX
        if( pT30CmdPrm -> FCLASSclass EQ FCLASS_CLASS_Fax20 )
        {
          pfilled_ctb->BC[0].bearer_serv = MNCC_BEARER_SERV_FAX;
          pfilled_ctb->BC[0].conn_elem   = MNCC_CONN_ELEM_TRANS;
          pfilled_ctb->BC[0].modem_type  = MNCC_MT_NONE;
        }
        else
#endif /* FF_FAX */
        {
          pfilled_ctb->BC[0].bearer_serv = cmhCC_SelServ( ccShrdPrm.CBSTname );
          pfilled_ctb->BC[0].conn_elem   = cmhCC_SelCE(   ccShrdPrm.CBSTce );
          pfilled_ctb->BC[0].modem_type  = cmhCC_SelMT  ( ccShrdPrm.CBSTspeed );

          pfilled_ctb->BC[0].rate_adaption = cmhCC_SelRateAdaption
                                                  (ccShrdPrm.CBSTspeed);

          pfilled_ctb->BC[0].transfer_cap = cmhCC_SelTransferCap
                                                  (ccShrdPrm.CBSTspeed);

          pfilled_ctb->BC[0].stop_bits   = cmhCC_SelStopBit( srcId );
          pfilled_ctb->BC[0].data_bits   = cmhCC_SelDataBit( srcId );
          pfilled_ctb->BC[0].parity      = cmhCC_SelParity ( srcId );
        }

        pfilled_ctb->BC[1].rate        = MNCC_UR_NOT_PRES;
        pfilled_ctb->BC[1].bearer_serv = MNCC_BEARER_SERV_NOT_PRES;
        pfilled_ctb->BC[1].conn_elem   = MNCC_CONN_ELEM_NOT_PRES;
        break;

      case( CMOD_MOD_VoiceDat ):           /* alternating data/voice call */
        pfilled_ctb->rptInd          = MNCC_RI_CIRCULAR;

        pfilled_ctb->BC[0].rate        = cmhCC_SelRate( ccShrdPrm.CBSTspeed );
        pfilled_ctb->BC[0].bearer_serv = cmhCC_SelServ( ccShrdPrm.CBSTname );
        pfilled_ctb->BC[0].conn_elem   = cmhCC_SelCE(   ccShrdPrm.CBSTce );
        pfilled_ctb->BC[0].modem_type  = cmhCC_SelMT  ( ccShrdPrm.CBSTspeed );

        pfilled_ctb->BC[0].stop_bits   = cmhCC_SelStopBit( srcId );
        pfilled_ctb->BC[0].data_bits   = cmhCC_SelDataBit( srcId );
        pfilled_ctb->BC[0].parity      = cmhCC_SelParity ( srcId );

        pfilled_ctb->BC[1].rate        = MNCC_UR_NOT_PRES;
        pfilled_ctb->BC[1].bearer_serv = cmhCC_set_speech_serv (pCCCmdPrm);
        pfilled_ctb->BC[1].conn_elem   = MNCC_CONN_ELEM_NOT_PRES;
        break;

      case( CMOD_MOD_VoiceFlwdDat ):       /* voice followed by data call */

        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );                 /* operation not allowed */

      case( CMOD_MOD_VoiceFax ):           /* alternating fax/voice call */
        pfilled_ctb->rptInd          = MNCC_RI_CIRCULAR;

        pfilled_ctb->BC[0].rate        = cmhCC_SelRate( ccShrdPrm.CBSTspeed );
        pfilled_ctb->BC[0].bearer_serv = MNCC_BEARER_SERV_FAX;
        pfilled_ctb->BC[0].conn_elem   = MNCC_CONN_ELEM_TRANS;
        pfilled_ctb->BC[0].modem_type  = MNCC_MT_NONE;

        pfilled_ctb->BC[1].rate        = MNCC_UR_NOT_PRES;
        pfilled_ctb->BC[1].bearer_serv = cmhCC_set_speech_serv (pCCCmdPrm);
        pfilled_ctb->BC[1].conn_elem   = MNCC_CONN_ELEM_NOT_PRES;
        break;

      default:                             /* unexpected call mode */
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_DataCorrupt );
        return( AT_FAIL );
      }
      break;
#endif    /* of #ifdef FAX_AND_DATA */

    default:                               /* unexpected call type */
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_DataCorrupt );
      return( AT_FAIL );
  }

  return( AT_CMPL );
}

#ifdef FF_TTY
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCF                  |
| STATE   : code                  ROUTINE : cmhCC_notifyTTY          |
+--------------------------------------------------------------------+

  PURPOSE : This function converts the TTY activity to the trx state
            to be provided by '%CTYI: ...'.
*/

GLOBAL T_ACI_CTTY_TRX cmhCC_getTTYtrx_state (int ttyTrxState)
{
  switch (ttyTrxState)
  {
  case TTY_OFF:
    return CTTY_TRX_Off;
  case TTY_HCO:
    return CTTY_TRX_SendOn;
  case TTY_VCO:
    return CTTY_TRX_RcvOn;
  case TTY_ALL:
    return CTTY_TRX_RcvSendOn;
  default:
    break;
  }
  return CTTY_TRX_Unknown;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCF                  |
| STATE   : code                  ROUTINE : cmhCC_notifyTTY          |
+--------------------------------------------------------------------+

  PURPOSE : This function generates the unsolicited message '%CTYI: ...'
            for any logical channel having registered for.
*/

GLOBAL void cmhCC_notifyTTY (T_ACI_CTTY_NEG neg,
                             T_ACI_CTTY_TRX trx)
{
  int i;

  for (i = CMD_SRC_LCL; i < CMD_SRC_MAX; i++)
  {
    if (cmhPrm[i].ccCmdPrm.CTTYmode EQ CTTY_MOD_Enable)
    {
      R_AT (RAT_CTYI, (T_ACI_CMD_SRC)i) (neg, trx);
    }
  }
}
#endif /* FF_TTY */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCF                  |
| STATE   : code                  ROUTINE : cmhCC_checkALS_Support   |
+--------------------------------------------------------------------+

  PURPOSE : These function checks for ALS support in two steps
*/

GLOBAL void cmhCC_checkALS_Support()
{
  TRACE_FUNCTION ("cmhCC_checkALS_Support()");

  /* Step #1: read CPHS info first */
  cmhSIM_ReadTranspEF( CMD_SRC_NONE,
                       AT_CMD_NONE,
                       FALSE,
                       NULL,
                       SIM_CPHS_CINF,
                       0,
                       ACI_CPHS_INFO_SIZE,
                       NULL,
                       cmhCC_checkALS_Support_cb );
}

GLOBAL void cmhCC_checkALS_Support_2()
{
  TRACE_FUNCTION ("cmhCC_checkALS_Support_2()");

  /* Step #2: then read the actual CSP */
  cmhSIM_ReadTranspEF( CMD_SRC_NONE,
                       AT_CMD_NONE,
                       FALSE,
                       NULL,
                       SIM_CPHS_CSP,
                       0,
                       0,
                       NULL,
                       cmhCC_checkALS_Support_cb_2 );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCF                  |
| STATE   : code                  ROUTINE : cmhCC_redialCheck        |
+--------------------------------------------------------------------+

  PURPOSE : This function evaluates conditions for redialling
*/

GLOBAL void cmhCC_redialCheck(SHORT cId)
{
  T_CC_CALL_TBL *ctbFail = ccShrdPrm.ctb[ccShrdPrm.cIdFail];
  T_CC_CALL_TBL *ctb     = ccShrdPrm.ctb[cId];
  SHORT error_indication;
  int i;

  TRACE_FUNCTION ("cmhCC_redialCheck()");

  /* check conditions for error_indication check */
  if((rdlPrm.rdlMod EQ AUTOM_REPEAT_ON) AND (rdlPrm.rdlcId EQ NO_ENTRY))
  {/* if black list is full no further redial attempts; user has to delete the black list */
#ifdef TI_PS_FF_AT_P_CMD_RDLB
    if(cmhCC_redialChkBlackl(cId) EQ AT_CMPL)
    {
      if(cc_blacklist_ptr->blCount NEQ MAX_NUM_ENTR_BLACK_LIST)
      {
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
        if( ccShrdPrm.cIdFail < 0 OR ccShrdPrm.cIdFail > MAX_CALL_NR )
        {
          error_indication = GET_CAUSE_VALUE(CAUSE_MAKE(DEFBY_STD,
                                                        ORIGSIDE_MS, 
                                                        MNCC_ACI_ORIGINATING_ENTITY,
                                                        NOT_PRESENT_8BIT));
        }
        else if( GET_CAUSE_VALUE(ctbFail->rejCs) NEQ NOT_PRESENT_8BIT )
        {
         error_indication = GET_CAUSE_VALUE(ctbFail->rejCs);
        }
        else if( GET_CAUSE_VALUE(ctbFail->nrmCs) NEQ NOT_PRESENT_8BIT )
        {
          error_indication = GET_CAUSE_VALUE(ctbFail->nrmCs);
        }
        else if( GET_CAUSE_VALUE(ctbFail->rslt) NEQ NOT_PRESENT_8BIT )
        {
          error_indication = GET_CAUSE_VALUE(ctbFail->rslt);
        }
        else
        {/* in case network has sent no extended report */
          error_indication = GET_CAUSE_VALUE(CAUSE_MAKE(DEFBY_STD,
                                                        ORIGSIDE_MS,
                                                        MNCC_ACI_ORIGINATING_ENTITY,
                                                        NOT_PRESENT_8BIT));
        }

        /* check error_indication (ETSI 2.07, Annex A) and redial counter */
        switch (error_indication)
        {
          /*** indications of category 1 ***/
          case 17:            /* user busy */
          /*** indications of category 2 ***/
          case 18:            /* no user responding */
          case 19:            /* user alterting, no answer */
          case 27:            /* destination out of order */
          case 34:            /* no circuit/channel available */
          case 41:            /* temporary failure */
          case 42:            /* switching equipment congestion */
          case 44:            /* requested circuit/channel available */
          case 47:            /*esources unavailable, unspecified */

            /* Implements Measure 197, 198 and 199 */
            if( (ctb->rdlCnt >= 4) AND (ctb->rdlCnt < MAX_NUM_REPEAT_ATT) )
            {
#ifndef _SIMULATION_            
              ctb->rdlTimIndex = 1;
#endif
            }
            cmhCC_redialAttempt( cId, ctb->rdlCnt );
            /* else : ctb->rdlCnt = MAX_NUM_REPEAT_ATT -> entry is black list */
            break;
          /*** indications of category 3 -> only one redial attempt is allowed ***/
          case 28:            /* invalid number format */
            ctb->rdlCnt = 0;
            break;            /* for incorrect number we do not start redialling */
          case 1:             /* unassigned (unallocated) number */
          case 3:             /* no route to destination */
          case 22:            /* number changed */
          case 38:            /* network out of order */
            if(!ctb->rdlCnt)
            {/* first redial attempt */
              ctb->rdlCnt = MAX_NUM_REPEAT_ATT - 1;
              rdlPrm.rdlcId = cId;
              /* 5+5 sec for first (= only) call attempt */
              TIMERSTART(ACI_REPEAT_1+ACI_DELAY_TIMER,ACI_REPEAT_HND);
              for(i = 0; i < CMD_SRC_MAX; i++)
              {
                R_AT(RAT_RDL, (T_ACI_CMD_SRC)i)(CALL_ATTEMPT_FAILED);
                R_AT(RAT_RDL, (T_ACI_CMD_SRC)i)(REDIAL_TIM_START);
              }
            }
            else
            {/* more than one attempt is not allowed --> entry in blacklist */
              ctb->rdlCnt = MAX_NUM_REPEAT_ATT;
              for(i = 0; i < CMD_SRC_MAX; i++)
              {
                R_AT(RAT_RDL,(T_ACI_CMD_SRC) i)(CALL_ATTEMPT_FAILED);
              }
             }
             break;
          default:
            ctb->rdlCnt = 0;
            break;
        }
#ifdef TI_PS_FF_AT_P_CMD_RDLB
      }
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
#ifdef TI_PS_FF_AT_P_CMD_RDLB
      /* check redial counter against black list: add entry if the list is not full yet */
      if(ctb->rdlCnt EQ MAX_NUM_REPEAT_ATT) 
      {
        if (cc_blacklist_ptr->blCount< MAX_NUM_ENTR_BLACK_LIST)
        {
          /* add entry in black list */
          cc_blacklist_ptr->blCount++;
          cc_blacklist_ptr->blNum[cc_blacklist_ptr->blCount-1].numb_len = 
             ctb->cldPty.c_called_num;
          memcpy(cc_blacklist_ptr->blNum[cc_blacklist_ptr->blCount-1].number, 
                 ctb->cldPty.called_num,
                 ctb->cldPty.c_called_num);
          cc_blacklist_ptr->blNum[cc_blacklist_ptr->blCount-1].type.ton =
            (T_ACI_TOA_TON)ctb->cldPty.ton;
          cc_blacklist_ptr->blNum[cc_blacklist_ptr->blCount-1].type.npi =
            (T_ACI_TOA_NPI)ctb->cldPty.npi;

          ctb->rdlCnt = 0; /* reset redial counter in call table */
          ctb->rdlTimIndex = RDL_TIM_INDEX_NOT_PRESENT;
          if(cc_blacklist_ptr->blCount EQ MAX_NUM_ENTR_BLACK_LIST)
          {
            for(i = 0; i < CMD_SRC_MAX; i++)
            {
              R_AT(RAT_RDLB, (T_ACI_CMD_SRC)i)(BLACKLIST_FULL);
            }
          }
          else
          {
            for(i = 0; i < CMD_SRC_MAX; i++)
            {
              R_AT(RAT_RDLB, (T_ACI_CMD_SRC)i)(ENTRY_BLACKLISTED);
            }
          }
        }
      }
      else
      {
        return;
      }
     } /* End of if (cc_blacklist_ptr->blCount< MAX_NUM_ENTR_BLACK_LIST) */
#ifdef SIM_TOOLKIT
      cmhSAT_NtwErr( (UBYTE)((GET_CAUSE_VALUE(ccShrdPrm.ctb[cId]->nrmCs) NEQ NOT_PRESENT_8BIT)? 
                            (ccShrdPrm.ctb[cId]->nrmCs|0x80) : ADD_NO_CAUSE));
      /* 
       * If cId is blacklisted, We need to stop the SAT Max duration timer
       * and reset the SAT max duration
       */
      if(satShrdPrm.dur)
      {
        TIMERSTOP(ACI_SAT_MAX_DUR_HND);
        satShrdPrm.dur  = -1;
      }
#endif /* SIM_TOOKIT */
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
  }
}

#ifdef TI_PS_FF_AT_P_CMD_RDLB
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCF                  |
| STATE   : code                  ROUTINE : cmhCC_redialChkBlackl    |
+--------------------------------------------------------------------+

  PURPOSE : This function checks the new cId against the black list
*/

GLOBAL T_ACI_RETURN cmhCC_redialChkBlackl(SHORT cId)
{
  UBYTE bl_index = 0;
  int i;

  TRACE_FUNCTION ("cmhCC_redialChkBlackl()");

  if(cc_blacklist_ptr NEQ NULL)
  {
    for(bl_index=0; bl_index<cc_blacklist_ptr->blCount;bl_index++)
    {
      T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

      if (memcmp (cc_blacklist_ptr->blNum[bl_index].number,
                  ctb->cldPty.called_num,
                  ctb->cldPty.c_called_num) EQ 0)
      {
        for(i = 0; i < CMD_SRC_MAX; i++)
        {
          R_AT(RAT_RDLB, (T_ACI_CMD_SRC)i)(ENTRY_IN_BLACKLIST);
        }
        return( AT_FAIL );
      }
    }
  }
  return( AT_CMPL );
}
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
#if defined(_TARGET_)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCF                  |
| STATE   : code                  ROUTINE : cmhCC_rd_mode_FFS        |
+--------------------------------------------------------------------+

  PURPOSE : This function writes redial_mode in FFS
*/

GLOBAL T_ACI_RETURN cmhCC_rd_mode_FFS(T_ACI_CC_REDIAL_MODE rdl_mode,
                                      T_ACI_CC_RDL_FFS ffs_mode)
{
  T_ACI_CC_REDIAL_MODE redial_mode;

  TRACE_FUNCTION ("cmhCC_rd_mode_FFS()");

/* Implements Measure#32: Row 73 */
  switch(FFS_mkdir(gsm_com_path))
  {/* create/check ffs directory */
    case EFFS_OK:
    case EFFS_EXISTS:
      break;
    default:
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return ( AT_FAIL );
  }

  if(ffs_mode EQ WRITE_RDLmode)
  {
/* Implements Measure#32: Row 74 */
    if(FFS_fwrite(gsm_com_redialmode_path,&rdl_mode, sizeof(T_ACI_CC_REDIAL_MODE)) NEQ EFFS_OK)
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return ( AT_FAIL );
    }
    return AT_CMPL;
  }
  else if(ffs_mode EQ READ_RDLmode)
  {
/* Implements Measure#32: Row 74 */
    FFS_fread(gsm_com_redialmode_path,&redial_mode, sizeof(T_ACI_CC_REDIAL_MODE));
    switch(redial_mode)
    {
      case AUTOM_REP_NOT_PRESENT:
        rdlPrm.rdlMod = AUTOM_REPEAT_OFF;
        return AT_CMPL;
      case AUTOM_REPEAT_OFF:
      case AUTOM_REPEAT_ON:
        rdlPrm.rdlMod = redial_mode;
        return AT_CMPL;
      default:
       ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
       return ( AT_FAIL );
    }
  }
  else
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return ( AT_FAIL );
  }
}
#endif /* _TARGET_ */

/* Implements Measure 197, 198 and 199 */
/*
+------------------------------------------------------------------------------
|  Function    : cmhCC_redialAttempt
+------------------------------------------------------------------------------
|  Purpose     : This Function will Attempt for Redialling.
|
|  Parameters  : srcId  - AT command source identifier.
|                rdlCnt - Redial Counter
|
|  Return      : void
+------------------------------------------------------------------------------
*/

LOCAL void cmhCC_redialAttempt( SHORT cId, UBYTE rdlCnt )
{
  int i;

  TRACE_FUNCTION ("cmhCC_redialAttempt()");

  if(!rdlCnt)
  {
    rdlPrm.rdlcId = cId;
#ifdef SIM_TOOLKIT
    if (satShrdPrm.dur NEQ -1)
    {
      /* Start the SAT max duration timer before the first redial attempt*/
      TIMERSTART(satShrdPrm.dur,ACI_SAT_MAX_DUR_HND);
    }
#endif /* SIM_TOOLKIT */

    /* first redial attempt */
    TIMERSTART( ACI_REPEAT_1 + ACI_DELAY_TIMER, ACI_REPEAT_HND );
  }
  else if ( rdlCnt < MAX_NUM_REPEAT_ATT )
  {
    rdlPrm.rdlcId = cId;
   /* from 2nd to 4th redial attempt and from 5th to 10th redial attempt */
#ifdef _SIMULATION_
   TIMERSTART( ACI_REPEAT_1, ACI_REPEAT_HND );
#else
   TIMERSTART( ACI_REPEAT_2_4 + ACI_DELAY_TIMER, ACI_REPEAT_HND );
#endif
  }
  else
  {
    return;
  }

  for(i = 0; i < CMD_SRC_MAX; i++)
  {
    R_AT(RAT_RDL, (T_ACI_CMD_SRC)i)(CALL_ATTEMPT_FAILED);
    R_AT(RAT_RDL, (T_ACI_CMD_SRC)i)(REDIAL_TIM_START);
  }
}



/*==== EOF ========================================================*/

