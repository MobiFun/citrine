/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_SSF
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
|  Purpose :  This module defines the functions for the protocol
|             stack adapter for supplementary services.
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_SSF_C
#define PSA_SSF_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

#undef TRACING

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "ksd.h"
#include "aci.h"
#include "psa.h"
#include "psa_ss.h"
#include "psa_util.h"
#include "cmh_ss.h"
#include "cmh.h"

/*==== CONSTANTS ==================================================*/
#define MAX_MOSTI_NR    (7)     /* max number of ti's for MOS */

#define MAX_ITM         (5)     /* max number of items per line */
#define ITM_WDT         (14)    /* item width in chars */
#define HDR_WDT         (10)    /* header width in chars */

/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/
LOCAL UBYTE tiPool = 0xFF; /* holds pool of transaction identifiers */

/*==== FUNCTIONS ==================================================*/

LOCAL void psaSS_asmSSForBS ( UBYTE ssCode, UBYTE      bscSrvType,
                              UBYTE bscSrv, T_ssForBS *ssForBS);

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_asmEmptyRslt      |
+-------------------------------------------------------------------+

  PURPOSE : assemble an empty return result facility information
            element.

*/

GLOBAL void psaSS_asmEmptyRslt ( void )
{
  TRACE_FUNCTION("psaSS_asmEmptyRslt");

  memset( &ssFIECodeBuf, 0, sizeof( ssFIECodeBuf ));

  ssFIECodeBuf.l_buf = 8;

  ssShrdPrm.cmpType = CT_RET_RSLT;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_asmErrorRslt      |
+-------------------------------------------------------------------+

  PURPOSE : assemble an return error facility information
            element.

*/

GLOBAL void psaSS_asmErrorRslt ( SHORT sId, UBYTE err )
{
  TRACE_FUNCTION("psaSS_asmErrorRslt");

  ssShrdPrm.stb[sId].failType = SSF_SS_ERR;
  ssShrdPrm.stb[sId].errCd    = err;
  ssShrdPrm.cmpType = CT_RET_ERR;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_asmInterrogateSS  |
+-------------------------------------------------------------------+

  PURPOSE : assemble the interrogate SS facility information element.

*/

GLOBAL void psaSS_asmInterrogateSS ( UBYTE ssCode,
                                     UBYTE bscSrvType,
                                     UBYTE bscSrv  )
{
  MCAST( irgtSS, INTERROGATE_SS_INV );

  TRACE_FUNCTION("psaSS_asmInterrogateSS");

  memset( irgtSS, 0, sizeof( T_INTERROGATE_SS_INV ));

  /* set basic settings */
  irgtSS->msg_type  = INTERROGATE_SS_INV;
  irgtSS->v_ssForBS = TRUE;

  /* set service code */
  irgtSS->ssForBS.v_ssCode = TRUE;
  irgtSS->ssForBS.ssCode   = ssCode;

  /* set basic service type */
  switch( bscSrvType )
  {
    case( BS_BEAR_SRV ):

      irgtSS->ssForBS.basicService.v_bearerService = TRUE;
      irgtSS->ssForBS.basicService.bearerService   = bscSrv;
      break;

    case( BS_TELE_SRV ):

      irgtSS->ssForBS.basicService.v_teleservice = TRUE;
      irgtSS->ssForBS.basicService.teleservice   = bscSrv;
      break;

    default:
      irgtSS->ssForBS.basicService.v_bearerService = FALSE;
      irgtSS->ssForBS.basicService.v_teleservice = FALSE;
  }

  memset( &ssFIECodeBuf, 0, sizeof( ssFIECodeBuf ));
  ssFIECodeBuf.l_buf = MAX_FIE_CODE_BUF_LEN<<3;
  ccd_codeMsg (CCDENT_FAC, UPLINK,
               (T_MSGBUF *)&ssFIECodeBuf, _decodedMsg,
               NOT_PRESENT_8BIT);

  ssShrdPrm.cmpType = CT_INV;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_asmRegisterSS     |
+-------------------------------------------------------------------+

  PURPOSE : assemble the register SS facility information element.

*/

GLOBAL void  psaSS_asmRegisterSS    ( UBYTE ssCode, UBYTE bscSrvType,
                                      UBYTE bscSrv, UBYTE ton, UBYTE npi,
                                      UBYTE *num, UBYTE tos, UBYTE oe,
                                      UBYTE *sub, UBYTE time )
{
  MCAST( regSS, REGISTER_SS_INV );

  TRACE_FUNCTION("psaSS_asmRegisterSS");

  memset( regSS, 0, sizeof( T_REGISTER_SS_INV ));

  /* set basic settings */
  regSS->msg_type        = REGISTER_SS_INV;
  regSS->v_registerSSArg = TRUE;

  /* set service code */
  regSS->registerSSArg.v_ssCode = TRUE;
  regSS->registerSSArg.ssCode   = ssCode;

  /* set basic service type */
  switch( bscSrvType )
  {
    case( BS_BEAR_SRV ):

      regSS->registerSSArg.basicService.v_bearerService = TRUE;
      regSS->registerSSArg.basicService.bearerService   = bscSrv;
      break;

    case( BS_TELE_SRV ):

      regSS->registerSSArg.basicService.v_teleservice = TRUE;
      regSS->registerSSArg.basicService.teleservice   = bscSrv;
      break;
  }

  /* set forwarded to number */
  if( num NEQ NULL )
  {
    regSS->registerSSArg.v_forwardedToNumber = TRUE;
    regSS->registerSSArg.forwardedToNumber.c_bcdDigit
        = (UBYTE)utl_dialStr2BCD((char *) num,
                                 regSS->registerSSArg.forwardedToNumber.bcdDigit,
                                 MAX_PARTY_NUM);

    if(ton NEQ MNCC_TON_NOT_PRES AND
       npi NEQ MNCC_NPI_NOT_PRES)
    {
      regSS->registerSSArg.forwardedToNumber.v_noa = TRUE;
      regSS->registerSSArg.forwardedToNumber.noa   = ton;

      regSS->registerSSArg.forwardedToNumber.v_npi = TRUE;
      regSS->registerSSArg.forwardedToNumber.npi   = npi;
    }
  }

  /* set forwarded to subaddress */
  if( sub NEQ NULL )
  {
    regSS->registerSSArg.v_forwardedToSubaddress = TRUE;
    regSS->registerSSArg.forwardedToSubaddress.c_subadr_str
        = (UBYTE)utl_dialStr2BCD((char *) sub,
                                 regSS->registerSSArg.forwardedToSubaddress.subadr_str,
                                 MAX_SUBADDR_NUM);

    regSS->registerSSArg.forwardedToSubaddress.v_tos = TRUE;
    regSS->registerSSArg.forwardedToSubaddress.tos   = tos;

    regSS->registerSSArg.forwardedToSubaddress.v_oei = TRUE;
    regSS->registerSSArg.forwardedToSubaddress.oei   = oe;
  }

  /* set no reply timer */
//TISH, patch for OMAPS00130690
#if 0
  if( ssCode EQ SS_CD_CFNRY AND time NEQ KSD_TIME_NONE )
#else
  if( ((ssCode EQ SS_CD_CFNRY) OR (ssCode EQ SS_CD_ALL_FWSS) OR (ssCode EQ SS_CD_ALL_CFWSS)) AND time NEQ KSD_TIME_NONE )
#endif
//end
  {
    regSS->registerSSArg.v_noReplyConditionTime = TRUE;
    regSS->registerSSArg.noReplyConditionTime   = time;
  }

  memset( &ssFIECodeBuf, 0, sizeof( ssFIECodeBuf ));
  ssFIECodeBuf.l_buf = MAX_FIE_CODE_BUF_LEN<<3;
  ccd_codeMsg (CCDENT_FAC, UPLINK,
               (T_MSGBUF *)&ssFIECodeBuf, _decodedMsg,
               NOT_PRESENT_8BIT);

  ssShrdPrm.cmpType = CT_INV;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_asmEraseSS        |
+-------------------------------------------------------------------+

  PURPOSE : assemble the erase SS facility information element.

*/

GLOBAL void psaSS_asmEraseSS ( UBYTE ssCode, UBYTE bscSrvType,
                               UBYTE bscSrv )
{
  MCAST( ersSS, ERASE_SS_INV );

  TRACE_FUNCTION("psaSS_asmEraseSS");

  memset( ersSS, 0, sizeof( T_ERASE_SS_INV ));

  /* set basic settings */
  ersSS->msg_type  = ERASE_SS_INV;
  ersSS->v_ssForBS = TRUE;

  /* Implements Measure # 194 */
  psaSS_asmSSForBS ( ssCode, bscSrvType, bscSrv, &(ersSS->ssForBS)); 

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_asmActivateSS     |
+-------------------------------------------------------------------+

  PURPOSE : assemble the interrogate SS facility information element.

*/

GLOBAL void psaSS_asmActivateSS ( UBYTE ssCode, UBYTE bscSrvType,
                                   UBYTE bscSrv )
{
  MCAST( actSS, ACTIVATE_SS_INV );

  TRACE_FUNCTION("psaSS_asmActivateSS");

  memset( actSS, 0, sizeof( T_ACTIVATE_SS_INV ));

  /* set basic settings */
  actSS->msg_type  = ACTIVATE_SS_INV;
  actSS->v_ssForBS = TRUE;

  /* Implements Measure # 194 */
  psaSS_asmSSForBS ( ssCode, bscSrvType, bscSrv, &(actSS->ssForBS)); 

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_asmDeactivateSS   |
+-------------------------------------------------------------------+

  PURPOSE : assemble the deactivate SS facility information element.

*/

GLOBAL void psaSS_asmDeactivateSS ( UBYTE ssCode, UBYTE bscSrvType,
                                     UBYTE bscSrv )
{
  MCAST( deactSS, DEACTIVATE_SS_INV );

  TRACE_FUNCTION("psaSS_asmDeactivateSS");

  memset( deactSS, 0, sizeof( T_DEACTIVATE_SS_INV ));

  /* set basic settings */
  deactSS->msg_type  = DEACTIVATE_SS_INV;
  deactSS->v_ssForBS = TRUE;

  /* Implements Measure # 195 */
  psaSS_asmSSForBS ( ssCode, bscSrvType, bscSrv, &(deactSS->ssForBS)); 

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_asmRegisterPWD    |
+-------------------------------------------------------------------+

  PURPOSE : assemble the register password facility information
            element.

*/

GLOBAL void psaSS_asmRegisterPWD ( UBYTE ssCode )
{
  MCAST( regPWD, REGISTER_PWD_INV );

  TRACE_FUNCTION("psaSS_asmRegisterPWD");

  memset( regPWD, 0, sizeof( T_REGISTER_PWD_INV ));

  /* set service code */
  regPWD->v_ssCode = TRUE;
  regPWD->ssCode   = ssCode;
  regPWD->msg_type = REGISTER_PWD_INV;

  memset( &ssFIECodeBuf, 0, sizeof( ssFIECodeBuf ));
  ssFIECodeBuf.l_buf = MAX_FIE_CODE_BUF_LEN<<3;
  ccd_codeMsg (CCDENT_FAC, UPLINK,
               (T_MSGBUF *)&ssFIECodeBuf, _decodedMsg,
               NOT_PRESENT_8BIT);

  ssShrdPrm.cmpType = CT_INV;

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_asmVerifyPWD      |
+-------------------------------------------------------------------+

  PURPOSE : assemble the verify password facility information
            element.

*/

GLOBAL void psaSS_asmVerifyPWD ( UBYTE *pwd )
{
  MCAST( vrfPWD, GET_PWD_RES );

  TRACE_FUNCTION("psaSS_asmVerifyPWD");

  memset( vrfPWD, 0, sizeof( T_GET_PWD_RES ));

  /* set service code */
  vrfPWD->v_currPassword = TRUE;
  vrfPWD->msg_type = GET_PWD_RES;

  strncpy( (char *) vrfPWD->currPassword.digit, (char *) pwd, MAX_PWD_NUM );

  memset( &ssFIECodeBuf, 0, sizeof( ssFIECodeBuf ));
  ssFIECodeBuf.l_buf = MAX_FIE_CODE_BUF_LEN<<3;
  ccd_codeMsg (CCDENT_FAC, UPLINK,
               (T_MSGBUF *)&ssFIECodeBuf, _decodedMsg,
               NOT_PRESENT_8BIT);

  ssShrdPrm.cmpType = CT_RET_RSLT;

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_asmProcUSSDReq    |
+-------------------------------------------------------------------+

  PURPOSE : assemble the process USSD request facility information
            element.

*/

GLOBAL void psaSS_asmProcUSSDReq ( UBYTE dcs, UBYTE *ussd, UBYTE len )
{
/*  UBYTE cmpLen;*/

  MCAST( ussdReq, PROCESS_USSD_REQ_INV );

  TRACE_FUNCTION("psaSS_asmProcUSSDReq");

  memset( ussdReq, 0, sizeof( T_PROCESS_USSD_REQ_INV ));

  /* set service code */
  ussdReq->v_ussdArg = TRUE;
  ussdReq->msg_type = PROCESS_USSD_REQ_INV;

  /* set data coding scheme */
  ussdReq->ussdArg.v_ussdDataCodingScheme = TRUE;
  ussdReq->ussdArg.ussdDataCodingScheme   = dcs;

  /* set ussd response string */
/*  if( utl_getAlphabetCb( dcs ) EQ 0 ) *//* 7bit alphabet */
/*    cmpLen = utl_cvt8To7( ussd, (UBYTE)MINIMUM( MAX_USSD_STRING, len),
                          ussdReq->ussdArg.ussdString.b_ussdString, 0 );
    else*/
  {
/*    cmpLen = len;*/
    memcpy( ussdReq->ussdArg.ussdString.b_ussdString, ussd,
            MINIMUM( MAX_USSD_STRING, len));
  }

  ussdReq->ussdArg.v_ussdString = TRUE;
  ussdReq->ussdArg.ussdString.o_ussdString = 0;
  ussdReq->ussdArg.ussdString.l_ussdString = len<<3; /*cmpLen<<3;*/

  memset( &ssFIECodeBuf, 0, sizeof( ssFIECodeBuf ));
  ssFIECodeBuf.l_buf = MAX_FIE_CODE_BUF_LEN<<3;
  ccd_codeMsg (CCDENT_FAC, UPLINK,
               (T_MSGBUF *)&ssFIECodeBuf, _decodedMsg,
               NOT_PRESENT_8BIT);

  ssShrdPrm.cmpType = CT_INV;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_asmCnfUSSDReq     |
+-------------------------------------------------------------------+

  PURPOSE : assemble the confirm USSD request facility information
            element.
*/

GLOBAL void psaSS_asmCnfUSSDReq ( UBYTE dcs, UBYTE *ussd, UBYTE len )
{
  UBYTE cmpLen;
  UBYTE src_len;

  MCAST( ussdReq, USSD_REQ_RES );

  TRACE_FUNCTION("psaSS_asmCnfUSSDReq");

  memset( ussdReq, 0, sizeof( T_USSD_REQ_RES ));

  /* set service code */
  ussdReq->v_ussdRes = TRUE;
  ussdReq->msg_type = USSD_REQ_RES;

  /* set data coding scheme */
  ussdReq->ussdRes.v_ussdDataCodingScheme = TRUE;
  ussdReq->ussdRes.ussdDataCodingScheme   = dcs;

  /* set ussd response string */
  /* Implements Measure 25 */
  /* This function is more correct than utl_getAlphabetCb as it takes care 
     of reserved codings */
  if( cmh_getAlphabetCb( dcs ) EQ 0 ) /* 7bit alphabet */
  {
    src_len = (UBYTE)MINIMUM( MAX_USSD_STRING, len);
    cmpLen = utl_cvt8To7( ussd, src_len,
                          ussdReq->ussdRes.ussdString.b_ussdString, 0 );
    /* According to spec 23.038 section 6.1.2.3 for USSD packing, for bytes end with
     * (8*n)-1 i.e where n is 1,2,3....i.e byte 7, 15, 23 ... to be padded 
     * with carriage return <CR>(0xD) 
     */
    if ((src_len+1)%8 EQ 0)
    {
      ussdReq->ussdRes.ussdString.b_ussdString[cmpLen-1] |= (0xD << 1);
    }
  }
  else
  {
    cmpLen = len;
    memcpy( ussdReq->ussdRes.ussdString.b_ussdString, ussd,
            MINIMUM( MAX_USSD_STRING, len));
  }

  ussdReq->ussdRes.v_ussdString = TRUE;
  ussdReq->ussdRes.ussdString.o_ussdString = 0;
  ussdReq->ussdRes.ussdString.l_ussdString = cmpLen<<3;

  memset( &ssFIECodeBuf, 0, sizeof( ssFIECodeBuf ));
  ssFIECodeBuf.l_buf = MAX_FIE_CODE_BUF_LEN<<3;
  ccd_codeMsg (CCDENT_FAC, UPLINK,
               (T_MSGBUF *)&ssFIECodeBuf, _decodedMsg,
               NOT_PRESENT_8BIT);

  ssShrdPrm.cmpType = CT_RET_RSLT;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_USSDProt1         |
+-------------------------------------------------------------------+

  PURPOSE : send USSD according to protocol version 1.
*/

GLOBAL BOOL psaSS_asmUSSDProt1 ( SHORT sId )
{
  UBYTE  ussdBuf8Bit[MAX_USSD_STRING];   /* buffers 8 bit ussd string */
  USHORT ussdLen8Bit;

  TRACE_FUNCTION("psaSS_asmUSSDProt1");

  if( ssShrdPrm.ussdLen AND
      ssShrdPrm.stb[sId].SSver NEQ NOT_PRESENT_8BIT )
  {
    MCAST( ussdData, PROCESS_USSD_INV );

    memset( ussdData, 0, sizeof( T_PROCESS_USSD_INV ));

    /* set service code */
    ussdData->v_ssUserData = TRUE;
    ussdData->msg_type = PROCESS_USSD_INV;

    /* GSM data must be 8-bit coded */
    if (ssShrdPrm.ussdDcs EQ 0x0F)
    {
      ussdLen8Bit = utl_cvt7To8 (ssShrdPrm.ussdBuf, ssShrdPrm.ussdLen, ussdBuf8Bit, 0);
    }
    else
    {
      memcpy ( ussdBuf8Bit, ssShrdPrm.ussdBuf, ssShrdPrm.ussdLen);
               ussdLen8Bit = ssShrdPrm.ussdLen;
    }

    /* set ussd data string, if convertible */
    if( !utl_cvtGsmIra ( ussdBuf8Bit,
                         ussdLen8Bit,
                         ussdData->ssUserData.b_ssUserData,
                         MAX_USSD_DATA,
                         CSCS_DIR_GsmToIra ) )
    {
      return( FALSE );
    }

    ussdData->ssUserData.o_ssUserData = 0;
    ussdData->ssUserData.l_ssUserData = ssShrdPrm.ussdLen<<3;

    memset( &ssFIECodeBuf, 0, sizeof( ssFIECodeBuf ));
    ssFIECodeBuf.l_buf = MAX_FIE_CODE_BUF_LEN<<3;
    ccd_codeMsg (CCDENT_FAC, UPLINK,
                 (T_MSGBUF *)&ssFIECodeBuf, _decodedMsg,
                 NOT_PRESENT_8BIT);

    ssShrdPrm.cmpType = CT_INV;

    /* skip SS version indicator */
    ssShrdPrm.stb[sId].SSver = NOT_PRESENT_8BIT;

    /* repeat USSD string */
    psaSS_NewTrns(sId);

    return( TRUE );
  }

  return( FALSE );
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_asmDeactivateCCBS |
+-------------------------------------------------------------------+

  PURPOSE : assemble the deactivate CCBS facility information
            element.

*/

GLOBAL void psaSS_asmDeactivateCCBS ( UBYTE idx )
{
  MCAST( ersCCBS, ERASE_CC_ENTRY_INV );

  TRACE_FUNCTION("psaSS_asmDeactivateCCBS");

  memset( ersCCBS, 0, sizeof( T_ERASE_CC_ENTRY_INV ));

  /* set service code */
  ersCCBS->v_eraseCCEntryArg = TRUE;
  ersCCBS->msg_type = ERASE_CC_ENTRY_INV;

  ersCCBS->eraseCCEntryArg.v_ssCode = TRUE;
  ersCCBS->eraseCCEntryArg.ssCode   = SS_CD_CCBS;

  if( idx NEQ KSD_IDX_NONE )
  {
    ersCCBS->eraseCCEntryArg.v_ccbsIndex = TRUE;
    ersCCBS->eraseCCEntryArg.ccbsIndex   = idx;
  }

  memset( &ssFIECodeBuf, 0, sizeof( ssFIECodeBuf ));
  ssFIECodeBuf.l_buf = MAX_FIE_CODE_BUF_LEN<<3;
  ccd_codeMsg (CCDENT_FAC, UPLINK,
               (T_MSGBUF *)&ssFIECodeBuf, _decodedMsg,
               NOT_PRESENT_8BIT);

  ssShrdPrm.cmpType = CT_INV;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_dasmInvokeCmp     |
+-------------------------------------------------------------------+

  PURPOSE : disassemble the result component.
*/

GLOBAL void psaSS_dasmInvokeCmp ( SHORT sId, T_inv_comp *invCmp )
{
  USHORT ivId;                /* holds invoke Id */
  UBYTE  opCode;              /* holds operation code */

  TRACE_FUNCTION("psaSS_dasmInvokeCmp");

  if( invCmp -> v_inv_id )
    ivId = invCmp -> inv_id;
  else
    ivId = NOT_PRESENT_16BIT;

  if( invCmp -> v_op_code )
    opCode = invCmp -> op_code;
  else
    opCode = NOT_PRESENT_8BIT;

  /* store op code and invoke id for result */
  ssShrdPrm.stb[sId].iId = (UBYTE)ivId;
  ssShrdPrm.stb[sId].opCode = opCode;

  if( invCmp -> params.l_params )
  {
    UBYTE ccdRet;

    memcpy( ssFIEDecodeBuf, &invCmp -> params, MNCC_FACILITY_LEN );

    ccdRet = ccd_decodeMsg (CCDENT_FAC,
                            DOWNLINK,
                            (T_MSGBUF *) ssFIEDecodeBuf,
                            (UBYTE    *) _decodedMsg,
                            opCode);

    if( ccdRet NEQ ccdOK )
    {
      TRACE_EVENT_P1("CCD Decoding Error: %d",ccdRet );
      ssShrdPrm.stb[sId].failType = SSF_CCD_DEC;
      cmhSS_TransFail(sId);            
    }
  }
  else
    memset( ssFIEDecodeBuf, 0, MNCC_FACILITY_LEN );  /* put all valid flags to FALSE */

  switch( opCode )
  {
    case( OPC_GET_PASSWORD ):
      {
        MCAST( getPWD, GET_PWD_INV );

        cmhSS_getPassword( sId, getPWD );
      }
      break;

    case( OPC_UNSTRUCT_SS_NOTIFY ):
      {
        MCAST( ussdNtfy, USSD_NOTIFY_INV );

        cmhSS_USSDNotify( sId, ussdNtfy );
      }
      break;

    case( OPC_UNSTRUCT_SS_REQ ):
      {
        MCAST( ussdReq, USSD_REQ_INV );

        cmhSS_USSDRequest( sId, ussdReq );
      }
      break;

    case( OPC_FWD_CHECK_SS_IND ):
      {
        cmhSS_FwdChckSS( sId );
      }
      break;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_dasmResultCmp     |
+-------------------------------------------------------------------+

  PURPOSE : disassemble the result component.
*/

GLOBAL void psaSS_dasmResultCmp ( SHORT sId, T_res_comp *resCmp )
{
  UBYTE  opCode;              /* holds operation code */

  TRACE_FUNCTION("psaSS_dasmResultCmp");

  /* get operation code of the result */
  if( resCmp -> v_sequence         AND 
      resCmp -> sequence.v_op_code )
  {
    opCode = resCmp -> sequence.op_code;
  }
  else
    opCode = ssShrdPrm.stb[sId].opCode;

  /* decode additional parameters of result */
  if( resCmp -> v_sequence               AND 
      resCmp -> sequence.params.l_params )
  {
    UBYTE ccdRet;

    memcpy( ssFIEDecodeBuf, &resCmp -> sequence.params, MNCC_FACILITY_LEN);

    ccdRet = ccd_decodeMsg (CCDENT_FAC,
                            DOWNLINK,
                            (T_MSGBUF *) ssFIEDecodeBuf,
                            (UBYTE    *) _decodedMsg,
                            opCode);

    if( ccdRet NEQ ccdOK )
    {
/* Implements Measure#32: Row 1310 */
      TRACE_EVENT_P1("CCD Decoding Error: %d", ccdRet );
      cmhSS_SSResultFailure( sId );
      return;
    }
  }

  /* determine to which operation the result belongs to */
  switch( opCode )
  {
    case( OPC_REGISTER_SS ):
      {
        MCAST( regSS, REGISTER_SS_RES );

        cmhSS_SSRegistrated( sId, regSS );
      }
      break;

    case( OPC_ERASE_SS ):
      {
        MCAST( ersSS, ERASE_SS_RES );

        cmhSS_SSErased( sId, ersSS );
      }
      break;

    case( OPC_ACTIVATE_SS ):
      {
        MCAST( actSS, ACTIVATE_SS_RES );

        if( !(resCmp -> v_sequence) OR !(resCmp -> sequence.params.l_params) )
        {
          memset(actSS, 0, sizeof(T_ACTIVATE_SS_RES));
        }
        cmhSS_SSActivated( sId, actSS );
      }
      break;

    case( OPC_DEACTIVATE_SS ):
      {
        MCAST( deactSS, DEACTIVATE_SS_RES );

        cmhSS_SSDeactivated( sId, deactSS );
      }
      break;

    case( OPC_INTERROGATE_SS ):
      {
        MCAST( irgtSS, INTERROGATE_SS_RES );

        cmhSS_SSInterrogated( sId, irgtSS );
      }
      break;

    case( OPC_REGISTER_PASSWORD ):
      {
        MCAST( regPWD, REGISTER_PWD_RES );

        cmhSS_SSPWDRegistrated( sId, regPWD );
      }
      break;

    case( OPC_PROC_UNSTRUCT_SS_DATA ):
      {
        MCAST( prcUSSDDat, PROCESS_USSD_RES );

        cmhSS_USSDDatProcessed( sId, prcUSSDDat );
      }
      break;

    case( OPC_PROC_UNSTRUCT_SS_REQ ):
      {
        MCAST( prcUSSDReq, PROCESS_USSD_REQ_RES );

        cmhSS_USSDReqProcessed( sId, prcUSSDReq );
      }
      break;

    case( OPC_ERASE_CC_ENTRY ):
      {
        MCAST( ersCCNtry, ERASE_CC_ENTRY_RES );

        cmhSS_CCNtryErased( sId, ersCCNtry );
      }
      break;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_dasmErrorCmp      |
+-------------------------------------------------------------------+

  PURPOSE : disassemble the error component.
*/

GLOBAL T_ACI_RETURN psaSS_dasmErrorCmp ( SHORT sId, T_err_comp *errCmp, BOOL is_fac_ussd  )
{
  TRACE_EVENT_P1("psaSS_dasmErrorCmp: errCmp: %02x",errCmp -> err_code);

  if( errCmp -> v_err_code )
  {
    ssShrdPrm.stb[sId].failType = SSF_SS_ERR;
    ssShrdPrm.stb[sId].errCd    = errCmp -> err_code;

    
    switch(errCmp -> err_code)
    {
      case ERR_PWD_REG_FAIL:
        if( errCmp -> v_params AND errCmp -> params.l_params )
        {  
          T_PW_REGISTRATION_FAILURE_ERR *pwRegFail; 
            
          pwRegFail = (T_PW_REGISTRATION_FAILURE_ERR *)errCmp -> params.b_params;

          TRACE_EVENT_P1("VGK:Error reported %d", errCmp -> err_code);
          TRACE_EVENT_P1("VGK:Error params %d", pwRegFail->pwRegistrationFailureCause);

          if (pwRegFail->v_pwRegistrationFailureCause)
            ssShrdPrm.stb[sId].errPrms = pwRegFail->pwRegistrationFailureCause;
        }
        break;

      default:
        break;
    }

    /* check for protocol incompatibility for USSD */
    if( is_fac_ussd                                 AND
        errCmp -> err_code EQ ERR_FAC_NOT_SUPPORTED )
    {
      if( psaSS_asmUSSDProt1(sId) )
      {
        return( AT_CMPL );
      }
    }
    
  }
  return( AT_FAIL );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_dasmRejectCmp     |
+-------------------------------------------------------------------+

  PURPOSE : disassemble the error component.
*/

GLOBAL T_ACI_RETURN psaSS_dasmRejectCmp ( SHORT sId, T_rej_comp *rejCmp, BOOL is_fac_ussd  )
{
  TRACE_FUNCTION("psaSS_dasmRejectCmp");

  if( rejCmp -> v_gen_problem )
  {
    ssShrdPrm.stb[sId].failType = SSF_GEN_PRB;
    ssShrdPrm.stb[sId].rejPrb   = rejCmp -> gen_problem;
  }

  else if( rejCmp -> v_inv_problem )
  {
    ssShrdPrm.stb[sId].failType = SSF_INV_PRB;
    ssShrdPrm.stb[sId].rejPrb   = rejCmp -> inv_problem;

    /* check for protocol incompatibility for USSD */
    if( is_fac_ussd                                  AND
        rejCmp -> inv_problem EQ INV_PROB_UNRECOG_OP )
    {
      if( psaSS_asmUSSDProt1(sId) )
      {
        return( AT_CMPL );
      }
    }
  }

  else if( rejCmp -> v_res_problem )
  {
    ssShrdPrm.stb[sId].failType = SSF_RSL_PRB;
    ssShrdPrm.stb[sId].rejPrb   = rejCmp -> res_problem;
  }

  else if( rejCmp -> v_err_problem )
  {
    ssShrdPrm.stb[sId].failType = SSF_ERR_PRB;
    ssShrdPrm.stb[sId].rejPrb   = rejCmp -> err_problem;
  }
  return( AT_FAIL );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SATF                |
|                                 ROUTINE : psaSAT_ss_end_ind       |
+-------------------------------------------------------------------+

  PURPOSE : handles responses to user after a SS.
            is_fac_ussd TRUE if facility is a USSD
*/

GLOBAL T_ACI_RETURN psaSS_ss_end_ind ( SHORT sId, T_COMPONENT *com, BOOL is_fac_ussd )
{
  if( com->v_res_comp )
  {
    psaSS_dasmResultCmp( sId, &com->res_comp );
    return( AT_EXCT );
  }
  
  if( com->v_err_comp )
  {
    return(psaSS_dasmErrorCmp( sId, &com->err_comp, is_fac_ussd ));
  }

  if( com->v_rej_comp )
  {
    return(psaSS_dasmRejectCmp( sId, &com->rej_comp, is_fac_ussd ));
  }
  
  TRACE_EVENT( "WRONG COMPONENT TYPE RECEIVED" );
  return(AT_FAIL);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_stbNewEntry       |
+-------------------------------------------------------------------+

  PURPOSE : returns the service table index for a free entry to be
            used, otherwise the function returns -1 to indicate that
            no entry is free.
*/

GLOBAL SHORT psaSS_stbNewEntry ( void )
{
  SHORT stbIdx;             /* holds service table index */

  for( stbIdx = 0; stbIdx < MAX_SS_NR; stbIdx++ )
  {
    if( (ssShrdPrm.stb[stbIdx].ntryUsdFlg EQ FALSE) OR
        (ssShrdPrm.stb[stbIdx].srvStat    EQ SSS_IDL)    )
    {
      psaSS_InitStbNtry( stbIdx );

      return( stbIdx );
    }
  }

  return( -1 );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_stbFindTi         |
+-------------------------------------------------------------------+

  PURPOSE : returns the service table index for the entry that holds
            service parameters for the passed transaction identifier.
            Returning -1 indicates that the passed ti was not found.
*/

GLOBAL SHORT psaSS_stbFindTi ( UBYTE ti2Find )
{
  SHORT stbIdx;             /* holds service table index */

  for( stbIdx = 0; stbIdx < MAX_SS_NR; stbIdx++ )
  {
    if( ssShrdPrm.stb[stbIdx].ti EQ ti2Find )

      return( stbIdx );
  }

  return( -1 );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_stbFindInvId      |
+-------------------------------------------------------------------+

  PURPOSE : returns the service table index for the entry that holds
            service parameters for the passed invoke id.
            Returning -1 indicates that the passed ti was not found.
*/

GLOBAL SHORT psaSS_stbFindInvId ( UBYTE iId2Find )
{
  SHORT stbIdx;             /* holds service table index */

  for( stbIdx = 0; stbIdx < MAX_SS_NR; stbIdx++ )
  {
    if( ssShrdPrm.stb[stbIdx].ntryUsdFlg EQ TRUE AND
        ssShrdPrm.stb[stbIdx].iId        EQ iId2Find )

      return( stbIdx );
  }

  return( -1 );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_chgSrvTypCnt      |
+-------------------------------------------------------------------+

  PURPOSE : this function modifies the service type counter (MOS/MTS)
            defined by the passed service id by the passed delta value.
*/

GLOBAL void psaSS_chngSrvTypCnt ( SHORT sId, SHORT dlt )
{

  switch( ssShrdPrm.stb[sId].srvType )
  {
    case( ST_MOS ):

      ssShrdPrm.nrOfMOS += dlt;

      if( ssShrdPrm.nrOfMOS < 0 ) ssShrdPrm.nrOfMOS = 0;

      break;

    case( ST_MTS ):

      ssShrdPrm.nrOfMTS += dlt;

      if( ssShrdPrm.nrOfMTS < 0 ) ssShrdPrm.nrOfMTS = 0;

      break;

    default:

      TRACE_EVENT( "UNEXP SERVICE TYPE IN STB" );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_getMOSTi          |
+-------------------------------------------------------------------+

  PURPOSE : this function selects a ti out of a pool of valid ti's
            and inserts it into the passed service table entry. if no
            ti is available the function returns -1.
            a bit of the pool stands for a valid ti.
            0 indicates a used ti, 1 indicates a free ti.
*/

GLOBAL SHORT psaSS_getMOSTi( SHORT sId )
{
  UBYTE idx;               /* holds pool idx */

  for( idx = 0; idx < MAX_MOSTI_NR; idx++ )
  {
    if( tiPool & (1u << idx) )
    {
      tiPool &= ~(1u << idx);
      ssShrdPrm.stb[sId].ti = idx;
      return( 0 );
    }
  }
  return( -1 );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_retMOSTi          |
+-------------------------------------------------------------------+

  PURPOSE : this function returns a used ti to the ti pool if the
            call was a MOS. the ti is free for the next MOS
            afterwards.
            a bit of the pool stands for a valid ti.
            0 indicates a used ti, 1 indicates a free ti.
*/

GLOBAL void psaSS_retMOSTi( SHORT sId )
{
  if( ssShrdPrm.stb[sId].srvType EQ ST_MOS )

    if( ssShrdPrm.stb[sId].ti < MAX_MOSTI_NR )

      tiPool |= (0x01 << ssShrdPrm.stb[sId].ti);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SS                  |
|                                 ROUTINE : psaSS_InitStbNtry       |
+-------------------------------------------------------------------+

  PURPOSE : initialize the indexed service table entry.

*/

GLOBAL void psaSS_InitStbNtry ( SHORT idx )
{

/*
 *-------------------------------------------------------------------
 * initialize call table entry
 *-------------------------------------------------------------------
 */
  ssShrdPrm.stb[idx].ntryUsdFlg   = FALSE;
  ssShrdPrm.stb[idx].ti           = (UBYTE)-1;  /* NO_ENTRY is -1 and ti where is is assigning is UBYTE,so typecast with UBYTE */
  ssShrdPrm.stb[idx].iId          = 0;
  ssShrdPrm.stb[idx].srvStat      = NO_VLD_SSS;
  ssShrdPrm.stb[idx].srvType      = NO_VLD_ST;
  ssShrdPrm.stb[idx].orgOPC       = NOT_PRESENT_8BIT;
  ssShrdPrm.stb[idx].opCode       = NOT_PRESENT_8BIT;
  ssShrdPrm.stb[idx].ssCode       = NOT_PRESENT_8BIT;
  ssShrdPrm.stb[idx].ussdReqFlg   = FALSE;
  ssShrdPrm.stb[idx].failType     = NO_VLD_SSF;
  ssShrdPrm.stb[idx].entCs        = NOT_PRESENT_16BIT;
  ssShrdPrm.stb[idx].rejPrb       = NOT_PRESENT_8BIT;
  ssShrdPrm.stb[idx].errCd        = NOT_PRESENT_8BIT;
  ssShrdPrm.stb[idx].errPrms      = NOT_PRESENT_8BIT;
  ssShrdPrm.stb[idx].curCmd       = AT_CMD_NONE;
  ssShrdPrm.stb[idx].srvOwn       = (T_OWN)CMD_SRC_NONE;
  ssShrdPrm.stb[idx].SSver        = DEF_SS_VER;
  ssShrdPrm.stb[idx].ussd_operation = FALSE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_stbFindUssdReq    |
+-------------------------------------------------------------------+

  PURPOSE : returns the service table index for the entry that has a
            USSD request pending.
            Returning -1 indicates that no such service was found.
*/

GLOBAL SHORT psaSS_stbFindUssdReq ( void )
{
  SHORT stbIdx;             /* holds call table index */

  for( stbIdx = 0; stbIdx < MAX_SS_NR; stbIdx++ )
  {
    if( ssShrdPrm.stb[stbIdx].ntryUsdFlg EQ TRUE    AND
        ssShrdPrm.stb[stbIdx].ussdReqFlg EQ TRUE    AND
        ssShrdPrm.stb[stbIdx].srvStat    EQ SSS_ACT )
    {
          return( stbIdx );
    }
  }

  return( -1 );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_stbFindActSrv     |
+-------------------------------------------------------------------+

  PURPOSE : returns the service table index for the first entry that
            holds an active service which is not the passed index.
            Returning -1 indicates that no active service was found.
*/

GLOBAL SHORT psaSS_stbFindActSrv ( SHORT sId )
{
  SHORT stbIdx;             /* holds call table index */

  for( stbIdx = 0; stbIdx < MAX_SS_NR; stbIdx++ )
  {
    if( ssShrdPrm.stb[stbIdx].ntryUsdFlg EQ TRUE    AND
        ssShrdPrm.stb[stbIdx].srvStat    EQ SSS_ACT )
    {
      if( stbIdx NEQ sId )

        return( stbIdx );
    }
  }

  return( -1 );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SS                  |
|                                 ROUTINE : psaSS_Init              |
+-------------------------------------------------------------------+

  PURPOSE : initialize the protocol stack adapter for SS.

*/

GLOBAL void psaSS_Init ( void )
{
  UBYTE stbIdx;            /* holds index to service table */

/*
 *-------------------------------------------------------------------
 * initialize service table
 *-------------------------------------------------------------------
 */
  for( stbIdx = 0; stbIdx < MAX_SS_NR; stbIdx++ )
  {
    psaSS_InitStbNtry( stbIdx );
  }

 /*
  *-------------------------------------------------------------------
  * initialize shared parameter
  *-------------------------------------------------------------------
  */
  ssShrdPrm.nrOfMOS = 0;
  ssShrdPrm.nrOfMTS = 0;
  ssShrdPrm.iIdNxt  = 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SS                  |
|                                 ROUTINE : psaSS_DumpFIE           |
+-------------------------------------------------------------------+

  PURPOSE : Dump facility information element to debug output.

*/

GLOBAL void psaSS_DumpFIE ( T_fac_inf * fie )
{

  CHAR  strBuf[40+1];           /* holds string buffer */
  UBYTE idx, cnt, mcnt;         /* buffer index */
  CHAR  *pDest;                 /* points to destination */

/*
 *-------------------------------------------------------------------
 * format FIE
 *-------------------------------------------------------------------
 */
  TRACE_EVENT( "FIE SENT/RECEIVED:" );

  mcnt = fie->l_fac >> 3;

  if( mcnt EQ 0 )
  {
    TRACE_EVENT("Empty Facility");
    return;
  }

  for( cnt = 0; cnt < mcnt; cnt+=idx )
  {
    pDest = strBuf;

    for( idx = 0; idx < 20 AND idx+cnt < mcnt; idx++ )
    {
/* Implements Measure#32: Row 1312 */
      pDest += sprintf( pDest, format_2X_str, (UBYTE)fie->fac[idx+cnt] );
    }

    *pDest = 0x0;

    TRACE_EVENT( strBuf );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_stbDump           |
+-------------------------------------------------------------------+

  PURPOSE : this function dumps the service table to the debug output.
*/

GLOBAL void psaSS_stbDump ( void )
{
#ifdef TRACING

  SHORT stbIdx;                             /* holds service table index */
  char  lnBuf[HDR_WDT+(MAX_ITM*ITM_WDT)+1]; /* holds buffer for output line */
  char  verBuf[VERSION_LEN+1];              /* holds buffer for SS version */
  SHORT itmIdx;                             /* holds items per line index */
  SHORT chrNr;                              /* holds number of processed chars */

  for( stbIdx = 0; stbIdx < MAX_SS_NR; stbIdx+=MAX_ITM )
  {
    /* --- service id ----------------------------------------------------------*/
    chrNr = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "   srv id");

    for( itmIdx = 0; (itmIdx<MAX_ITM)AND(itmIdx+stbIdx<MAX_SS_NR); itmIdx++ )
    {
      chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT, stbIdx + itmIdx );
    }
    TRACE_EVENT( lnBuf );

    /* --- entry used flag -----------------------------------------------------*/
    chrNr = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "entry used" );

    for( itmIdx = 0; (itmIdx<MAX_ITM)AND(itmIdx<MAX_SS_NR); itmIdx++ )
    {
      chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT,
                        ssShrdPrm.stb[stbIdx+itmIdx].ntryUsdFlg );
    }
    TRACE_EVENT( lnBuf );

    /* --- transaction identifier ----------------------------------------------*/
    chrNr = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "        ti" );

    for( itmIdx = 0; (itmIdx<MAX_ITM)AND(itmIdx<MAX_SS_NR); itmIdx++ )
    {
      chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT,
                        ssShrdPrm.stb[stbIdx+itmIdx].ti );
    }
    TRACE_EVENT( lnBuf );

    /* --- service status ------------------------------------------------------*/
    chrNr = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "srv status" );

    for( itmIdx = 0; (itmIdx<MAX_ITM)AND(itmIdx<MAX_SS_NR); itmIdx++ )
    {
      chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT,
                        ssShrdPrm.stb[stbIdx+itmIdx].srvStat );
    }
    TRACE_EVENT( lnBuf );

    /* --- service type --------------------------------------------------------*/
    chrNr = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "  srv type" );

    for( itmIdx = 0; (itmIdx<MAX_ITM)AND(itmIdx<MAX_SS_NR); itmIdx++ )
    {
      chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT,
                        ssShrdPrm.stb[stbIdx+itmIdx].srvType );
    }
    TRACE_EVENT( lnBuf );

    /* --- SS version ----------------------------------------------------------*/
    chrNr = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "SS version" );

    for( itmIdx = 0; (itmIdx<MAX_ITM)AND(itmIdx<MAX_SS_NR); itmIdx++ )
    {
      chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT,
                        ssShrdPrm.stb[stbIdx+itmIdx].SSver );
    }
    TRACE_EVENT( lnBuf );

    /* --- failure type ------------------------------------------------------*/
    chrNr = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "fail. type" );

    for( itmIdx = 0; (itmIdx<MAX_ITM)AND(itmIdx<MAX_SS_NR); itmIdx++ )
    {
      chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT,
                        ssShrdPrm.stb[stbIdx+itmIdx].failType );
    }
    TRACE_EVENT( lnBuf );

    /* --- entity cause ------------------------------------------------------*/
    chrNr = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "ent. cause" );

    for( itmIdx = 0; (itmIdx<MAX_ITM)AND(itmIdx<MAX_SS_NR); itmIdx++ )
    {
      chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT,
                        ssShrdPrm.stb[stbIdx+itmIdx].entCs );
    }
    TRACE_EVENT( lnBuf );

    /* --- reject problem ----------------------------------------------------*/
    chrNr = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, " rej. prob" );

    for( itmIdx = 0; (itmIdx<MAX_ITM)AND(itmIdx<MAX_SS_NR); itmIdx++ )
    {
      chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT,
                        ssShrdPrm.stb[stbIdx+itmIdx].rejPrb );
    }
    TRACE_EVENT( lnBuf );

    /* --- error code ----------------------------------------------------*/
    chrNr = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, " err. code" );

    for( itmIdx = 0; (itmIdx<MAX_ITM)AND(itmIdx<MAX_SS_NR); itmIdx++ )
    {
      chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT,
                        ssShrdPrm.stb[stbIdx+itmIdx].errCd );
    }
    TRACE_EVENT( lnBuf );

  }

#endif  /* of #ifdef TRACING */
}
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_F                   |
|                                 ROUTINE : psaSS_GetPendingTrn     |
+-------------------------------------------------------------------+

  PURPOSE : Find Id of a pending call for multy-transaction operation

*/

GLOBAL SHORT psaSS_GetPendingTrn ( void )
{
  SHORT bit_len;
  SHORT i;

  bit_len = 8 * sizeof(ssShrdPrm.mltyTrnFlg);
  for (i = 0; i < bit_len; i++)
  {
    if ( ssShrdPrm.mltyTrnFlg & ( 0x1 << i ) )
      return i;
  }

  return -1;
}

GLOBAL void psaSS_KillAllPendingTrn ( void ) /* sId: main transaction: not to be killed */
{
  SHORT          pending_idx;

  TRACE_FUNCTION("psaSS_KillAllPendingTrn( )");

  while ( ssShrdPrm.mltyTrnFlg )
  {
    pending_idx = psaSS_GetPendingTrn( );
    ssShrdPrm.mltyTrnFlg &= ~( 1u << pending_idx );  /* unsent id flag */

    ssShrdPrm.stb[ pending_idx ].ntryUsdFlg = FALSE;  /* free entry in ss table */
    ssShrdPrm.stb[ pending_idx ].srvStat = NO_VLD_SSS; /* not sure if it's necessary...? */

    PFREE( ssShrdPrm.stb[pending_idx].save_prim ); /* free pending primitive */
  }
}

/* Implements Measure # 195 */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaSS_asmEraseSS        |
+-------------------------------------------------------------------+

  PURPOSE : Assemble the erase, activate and deactivate SS facility 
            information element.

*/

LOCAL void psaSS_asmSSForBS ( UBYTE ssCode, UBYTE      bscSrvType,
                              UBYTE bscSrv, T_ssForBS *ssForBS)
{

  TRACE_FUNCTION("psaSS_asmSSForBS( )");

  /* set service code */
  ssForBS->v_ssCode = TRUE;
  ssForBS->ssCode   = ssCode;

  /* set basic service type */
  switch( bscSrvType )
  {
    case( BS_BEAR_SRV ):

      ssForBS->basicService.v_bearerService = TRUE;
      ssForBS->basicService.bearerService   = bscSrv;
      break;

    case( BS_TELE_SRV ):

      ssForBS->basicService.v_teleservice = TRUE;
      ssForBS->basicService.teleservice   = bscSrv;
      break;
  }

  memset( &ssFIECodeBuf, 0, sizeof( ssFIECodeBuf ));
  ssFIECodeBuf.l_buf = MAX_FIE_CODE_BUF_LEN<<3;
  ccd_codeMsg (CCDENT_FAC, UPLINK,
               (T_MSGBUF *)&ssFIECodeBuf, _decodedMsg,
               NOT_PRESENT_8BIT);

  ssShrdPrm.cmpType = CT_INV;

}

/*==== EOF ========================================================*/

