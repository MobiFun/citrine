/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_CCF
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
|             stack adapter for call control.
+-----------------------------------------------------------------------------
*/

#ifndef PSA_CCF_C
#define PSA_CCF_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

#undef TRACING

/*==== INCLUDES ===================================================*/

#include "l4_tim.h"
#include "ccdapi.h"

#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "phb.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "aci.h"
#include "psa.h"
#include "psa_cc.h"
#include "psa_mmi.h"
#include "psa_ss.h"
#include "cmh.h"
#include "cmh_cc.h"
#include "psa_util.h"
#include "cmh_phb.h"
#include "psa_sim.h"
#include "aci_mem.h"
#include "hl_audio_drv.h"

#include "gdi.h"
#include "rtcdrv.h"
#include "audio.h"
#include "aoc.h"

#ifdef SIM_TOOLKIT
#include "psa_sat.h"
#include "cmh_sat.h"
#endif /* SIM_TOOLKIT */

/*==== CONSTANTS ==================================================*/
#define MAX_MOCTI_NR    (7)     /* max number of ti's for MOC */
#define SA_DEL         ('-')    /* subaddress delimiter */

#define MAX_ITM         (5)     /* max number of items per line */
#define ITM_WDT         (14)    /* item width in chars */
#define HDR_WDT         (10)    /* header width in chars */

/*modify by dongfeng for ASTec28850, ASTec27950*/
#define EMRGNCY_NUM    {"112","911",""}
                                    /* default emergency numbers */

#define EMRGNCY_NUM_NS {"118","119","000","08","110","999",""}  /* additional default emergency    */
                                    /* numbers for use without SIM     */


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/
LOCAL UBYTE tiPool = 0xFF; /* holds pool of transaction identifiers */

LOCAL const CHAR * const ec_string[]    = EMRGNCY_NUM;
LOCAL const CHAR * const ec_string_ns[] = EMRGNCY_NUM_NS;

EXTERN T_ACI_CUSCFG_PARAMS cuscfgParams;
EXTERN SHORT Ext_USSD_Res_Pending_sId;

/* Implements Measure#32: Row 1245, 1277 & 1312 */
const char * const format_2X_str="%02X";

/*
 * Phonebook search list
 *
 * Description : this is the list of phonebooks, which will be searched
 *               for known entries. This is done during phonebook
 *               dialling and the indication of incoming calls. It is
 *               important to leave the EOL indicator at the end of
 *               the list to terminate the list.
 *               Available phonebooks:
 *
 *               ECC Emergency call numbers
 *               ADN Abbreviated dialing number
 *               FDN Fixed dialing number
 *               BDN Barred dialing number
 *               LDN Last dialing number
 *               LRN Last received number
 *               SDN Service dialing number
 */

const SHORT phbSrchLst[] =
{
      ADN,
      SDN,
      FDN,
/*--- EOL -------------------------------------------------------*/
      -1
};

/*==== FUNCTIONS ==================================================*/

LOCAL UBYTE psaCC_handleInternatPlus( char *number );

LOCAL CHAR *psaCC_ctbAdr2Num ( CHAR *pNumBuf, UBYTE maxSize, UBYTE *num, 
                               UBYTE c_num,   UBYTE ton);

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCS                 |
|                                 ROUTINE : psaCC_send_satevent     |
+-------------------------------------------------------------------+

  PURPOSE : Tries to send an event to SAT.

*/
GLOBAL void psaCC_send_satevent( UBYTE event,
                                 SHORT callId ,
                                 T_CC_INITIATER actionSrc,
                                 BOOL check_SatDiscEvent )
{
  /* DOES NOTHING IF SIM TOOLKIT IS NOT HERE */
#ifdef SIM_TOOLKIT  /* monitoring for SAT */
  T_CC_CALL_TBL * pCtbNtry;     /* holds pointer to call table entry */

  pCtbNtry = ccShrdPrm.ctb[callId];

  if( check_SatDiscEvent EQ TRUE )    /* not sure whether this is needed */
  {
    if ( !pCtbNtry -> SatDiscEvent )
    {
      pCtbNtry -> SatDiscEvent = TRUE;
      if ( psaSAT_ChkEventList(event) )
      {
        cmhSAT_EventDwn( event, callId, actionSrc );
      }
    }
  }
  else
  {
    if ( psaSAT_ChkEventList(event) )
    {
      cmhSAT_EventDwn( event, callId, actionSrc );
    }
  }
#endif  /* SIM_TOOLKIT */
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_ctbNewEntry       |
+-------------------------------------------------------------------+

  PURPOSE : returns the call table index for a free entry to be used,
            otherwise the function return -1 to indicate that no
            entry is free.

            NOTE: Having the condition to consider a call table
                  entry as be available to be reused not only
                  if ntryUsdFlg equals FALSE, but also if
                  calStat equals CS_IDL seems to be undesirable.

*/

GLOBAL SHORT psaCC_ctbNewEntry ( void )
{
  SHORT ctbIdx;             /* holds call table index */

  for (ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++)
  {
    if (ccShrdPrm.ctb[ctbIdx] EQ NULL)
    {
      psaCC_InitCtbNtry( ctbIdx );
      return( ctbIdx );
    }
  }
  return( -1 );
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_ctbFindTi         |
+-------------------------------------------------------------------+

  PURPOSE : returns the call table index for the entry that holds
            call parameters for the passed transaction identifier.
            Returning -1 indicates that the passed ti was not found.
*/

GLOBAL SHORT psaCC_ctbFindTi ( UBYTE ti2Find )
{
  SHORT ctbIdx;             /* holds call table index */

  for( ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
  {
    T_CC_CALL_TBL *ctbx = ccShrdPrm.ctb[ctbIdx];

    if (ctbx NEQ NULL AND
        ctbx->ti EQ ti2Find )

      return( ctbIdx );
  }

  return( -1 );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_ctbFindCall       |
+-------------------------------------------------------------------+

  PURPOSE : returns the call table index for the entry that holds
            call parameters for the call with the searched call owner,
            call status and call type.
            Returning -1 indicates that no such call was found.
*/

GLOBAL SHORT psaCC_ctbFindCall ( T_OWN calOwn,
                                 T_CC_CLST calStat,
                                 T_CC_CLTP calType )
{
  SHORT ctbIdx;             /* holds call table index */

  for( ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
  {
    T_CC_CALL_TBL *ctbx = ccShrdPrm.ctb[ctbIdx];

    if (ctbx NEQ NULL AND
        ctbx->calStat EQ calStat)
    {
      if( calOwn EQ OWN_SRC_INV OR
          ctbx->calOwn EQ calOwn )
      {
        if( calType EQ NO_VLD_CT OR
            ctbx->calType EQ calType )

          return( ctbIdx );
      }
    }
  }

  return( -1 );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_ctbCallInUse      |
+-------------------------------------------------------------------+

  PURPOSE : checks the call table if there is a call in use by a
            owner, or if the call table is idle. The function returns
            TRUE or FALSE.
*/

GLOBAL BOOL psaCC_ctbCallInUse ( void )
{
  SHORT ctbIdx;             /* holds call table index */

  for( ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
  {
    T_CC_CALL_TBL *ctbx = ccShrdPrm.ctb[ctbIdx];

    if (ctbx NEQ NULL)
    {
      switch( ctbx->calStat )
      {
        case( CS_ACT ):
        case( CS_ACT_REQ ):
        case( CS_HLD_REQ ):
        case( CS_HLD ):
        case( CS_DSC_REQ ):
        case( CS_CPL_REQ ):
        case( CS_MDF_REQ ):
          if( ctbx->calOwn NEQ (T_OWN)CMD_SRC_NONE )
            return( TRUE );
      }
    }
  }

  return( FALSE );
}

//TISH, patch for OMAPS00110151/OMAPS00130693
//start
GLOBAL SHORT psaCC_ctbCountActiveCall ( void )
{
  SHORT ctbIdx;             /* holds call table index */
  SHORT activeCallNum=0;
  for( ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
  {
    T_CC_CALL_TBL *ctbx = ccShrdPrm.ctb[ctbIdx];

    if (ctbx NEQ NULL && ctbx->calStat EQ CS_ACT)
	activeCallNum++;
  }
  return activeCallNum;
}
//end

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_ctbAnyCallInUse   |
+-------------------------------------------------------------------+

  PURPOSE : checks the call table if there is a any call in use,
            or if the call table is idle. The function returns
            TRUE or FALSE.
*/

GLOBAL BOOL psaCC_ctbAnyCallInUse ( void )
{
  SHORT ctbIdx;             /* holds call table index */

  for( ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
  {
    T_CC_CALL_TBL *ctbx = ccShrdPrm.ctb[ctbIdx];

    if (ctbx NEQ NULL)
    {
      switch( ctbx->calStat )
      {
        case( CS_ACT ):
        case( CS_ACT_REQ ):
        case( CS_HLD_REQ ):
        case( CS_HLD ):
        case( CS_DSC_REQ ):
        case( CS_CPL_REQ ):
        case( CS_MDF_REQ ):
          return( TRUE );
      }
    }
  }

  return( FALSE );
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_ctbDialNr2CldAdr  |
+-------------------------------------------------------------------+

  PURPOSE : this function converts a dial string into the settings
            for the called address parameters for the passed call id.
            The function returns -1 if an error occurs.
*/

GLOBAL SHORT psaCC_ctbDialNr2CldAdr ( SHORT cId, char * pDialStr )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
  char * pSubAdr;           /* points to subaddress */

/*
 *-------------------------------------------------------------------
 * seach for subaddress
 *-------------------------------------------------------------------
 */
  pSubAdr = strchr( pDialStr, SA_DEL );

  if ( pSubAdr )    /* subaddress found */
  {
    /*
     * cutoff the subaddr from the dialnumber
     */
    *pSubAdr = 0x0;
    pSubAdr++;

    /*
     * fill in subaddress information
     */
    ctb->cldPtySub.tos = MNCC_TOS_NSAP;

    ctb->cldPtySub.c_subaddr =
      (UBYTE)utl_dialStr2BCD (pSubAdr, ctb->cldPtySub.subaddr, MNCC_SUB_LENGTH);

    ctb->cldPtySub.odd_even =
      (ctb->cldPtySub.c_subaddr & 1) ? MNCC_OE_ODD : MNCC_OE_EVEN;
  }
  else            /* subaddress not found */
  {
    ctb->cldPtySub.tos = MNCC_TOS_NOT_PRES;
    ctb->cldPtySub.odd_even = MNCC_OE_EVEN;
    ctb->cldPtySub.c_subaddr = 0;
  }

/*
 *-------------------------------------------------------------------
 * fill in address information
 *-------------------------------------------------------------------
 */
  ctb->cldPty.npi = MNCC_NPI_ISDN_TEL_NUMB_PLAN;

  ctb->cldPty.ton = ((pDialStr[0] EQ '+') ? MNCC_TON_INT_NUMB : MNCC_TON_UNKNOWN);

  /*
   * 00 is not an indication for an international number
  if( pDialStr[0] EQ '0' AND pDialStr[1] EQ '0' )
  {
    pDialStr += 2;
    ctb->cldPty.ton = TON_INT_NUMB;
  }
  */

  if (ctb->cldPty.called_num NEQ NULL)
  {
    ACI_MFREE (ctb->cldPty.called_num);
    ctb->cldPty.called_num = NULL;
  }
  ctb->cldPty.c_called_num =
    (UBYTE)utl_dialStr2BCD (pDialStr, NULL, MNCC_MAX_CC_CALLED_NUMBER);
  if (ctb->cldPty.c_called_num NEQ 0)
  {
    ACI_MALLOC (ctb->cldPty.called_num,
                ctb->cldPty.c_called_num);
    (void)utl_dialStr2BCD (pDialStr,
                           ctb->cldPty.called_num,
                           ctb->cldPty.c_called_num);
  }

  return( 0 );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_ctbClrAdr2Num     |
+-------------------------------------------------------------------+

  PURPOSE : this function converts the parameters of the calling
            address for the passed call id into a dial number. The
            string is copied into the passed buffer. In case the
            buffer size is not sufficient to hold the called number,
            the number is cut to maxSize. The function returns a pointer
            to the buffer or NULL if an error occurs.
*/

GLOBAL CHAR *psaCC_ctbClrAdr2Num ( SHORT cId, CHAR *pNumBuf, UBYTE maxSize )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

  TRACE_FUNCTION ("psaCC_ctbClrAdr2Num()");

  /* Implements Measure# 188 */
  return (psaCC_ctbAdr2Num ( pNumBuf, maxSize , ctb->clgPty.num, 
                             ctb->clgPty.c_num, ctb->clgPty.ton));
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_ctbClrAdr2Sub     |
+-------------------------------------------------------------------+

  PURPOSE : this function converts the parameters of the calling
            subaddress for the passed call id into a dial subnumber.
            The string is copied into the passed buffer. The function
            returns a pointer to the buffer or 0 if an error occurs.
*/

GLOBAL CHAR *psaCC_ctbClrAdr2Sub ( SHORT cId, CHAR * pSubBuf )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

/*
 *-------------------------------------------------------------------
 * convert BCD subaddress
 *-------------------------------------------------------------------
 */
  if (ctb->clgPtySub.c_subaddr EQ 0 )
  {
    *pSubBuf = '\0';     /* empty string */
    return( NULL );
  }
  utl_BCD2DialStr (ctb->clgPtySub.subaddr, pSubBuf,
                   ctb->clgPtySub.c_subaddr);

  return( pSubBuf );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_ctbCldAdr2Num     |
+-------------------------------------------------------------------+

  PURPOSE : this function converts the parameters of the called
            address for the passed call id into a dial number. The
            string is copied into the passed buffer. In case the
            buffer size is not sufficient to hold the called number,
            the number is cut to maxSize. This is currently necessary
            to protect the protocol stack from memory overwriting, as
            the called party address for CC may have a size of up to
            80 digits, but there may not be this room foreseen in some
            phonebook entries. The function returns a pointer
            to the buffer or NULL if an error occurs.
*/

GLOBAL CHAR *psaCC_ctbCldAdr2Num ( SHORT cId, CHAR *pNumBuf, UBYTE maxSize )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

  /* Implements Measure# 188 */
  return (psaCC_ctbAdr2Num ( pNumBuf, maxSize ,ctb->cldPty.called_num, 
                             ctb->cldPty.c_called_num, ctb->cldPty.ton));
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_ctbCldAdr2Sub     |
+-------------------------------------------------------------------+

  PURPOSE : this function converts the parameters of the called
            subaddress for the passed call id into a dial subnumber.
            The string is copied into the passed buffer. The function
            returns a pointer to the buffer or 0 if an error occurs.
*/

GLOBAL CHAR *psaCC_ctbCldAdr2Sub ( SHORT cId, CHAR * pSubBuf )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

/*
 *-------------------------------------------------------------------
 * convert BCD subaddress
 *-------------------------------------------------------------------
 */
  if (ctb->cldPtySub.c_subaddr EQ 0 )
  {
    *pSubBuf = '\0';     /* empty string */
    return( NULL );
  }
  utl_BCD2DialStr (ctb->cldPtySub.subaddr, pSubBuf,
                   ctb->cldPtySub.c_subaddr);

  return( pSubBuf );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_ctbRdrAdr2Num     |
+-------------------------------------------------------------------+

  PURPOSE : this function converts the parameters of the redirecting
            address for the passed call id into a dial number. The
            string is copied into the passed buffer. In case the
            buffer size is not sufficient to hold the called number,
            the number is cut to maxSize. The function returns a pointer
            to the buffer or NULL if an error occurs.
*/

GLOBAL CHAR *psaCC_ctbRdrAdr2Num ( SHORT cId, CHAR *pNumBuf, UBYTE maxSize )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

  /* Implements Measure# 189 */
  return (psaCC_ctbAdr2Num ( pNumBuf, maxSize ,ctb->rdrPty.redir_num, 
                             ctb->rdrPty.c_redir_num, ctb->rdrPty.ton));
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_ctbRdrAdr2Sub     |
+-------------------------------------------------------------------+

  PURPOSE : this function converts the parameters of the redirecting
            subaddress for the passed call id into a dial subnumber.
            The string is copied into the passed buffer. The function
            returns a pointer to the buffer or 0 if an error occurs.
*/

GLOBAL CHAR *psaCC_ctbRdrAdr2Sub ( SHORT cId, CHAR * pSubBuf )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

/*
 *-------------------------------------------------------------------
 * convert BCD subaddress
 *-------------------------------------------------------------------
 */
  if (ctb->rdrPtySub.c_subaddr EQ 0 )
  {
    *pSubBuf = '\0';     /* empty string */
    return( NULL );
  }
  utl_BCD2DialStr (ctb->rdrPtySub.subaddr, pSubBuf,
                   ctb->rdrPtySub.c_subaddr);

  return( pSubBuf );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_ctbGetAplha       |
+-------------------------------------------------------------------+

  PURPOSE : this function returns the pointer of the aplha identifier
            in the call table for the specified call. If no alpha
            identifier is available a NULL pointer will be returned.
*/

GLOBAL T_ACI_PB_TEXT *psaCC_ctbGetAlpha ( SHORT cId )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

  if( ctb->alphIdUni.len EQ 0 )

    return( NULL );

  else

    return( &ctb->alphIdUni );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_chgCalTypCnt      |
+-------------------------------------------------------------------+

  PURPOSE : this function modifies the call type counter (MOC/MTC)
            defined by the passed call id by the passed delta value.

            NOTE: The purpose of this function is to be able to set
                  ccShrdPrm.TCHasg to FALSE if all calls are gone.
                  Nice try, but this could achieved more easily by
                  having a clean call table management.
                  Misconcepted code.

*/

GLOBAL void psaCC_chngCalTypCnt ( SHORT cId, SHORT dlt )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

  TRACE_EVENT_P3 ("MOC = %d, MTC = %d, cId = %d",
                  (int)ccShrdPrm.nrOfMOC, (int)ccShrdPrm.nrOfMTC, (int)cId);

  switch( ctb->calType )
  {
    case( CT_MOC ):

      ccShrdPrm.nrOfMOC += dlt;

      if( ccShrdPrm.nrOfMOC < 0 ) ccShrdPrm.nrOfMOC = 0;

      break;

    case( CT_MTC ):

      ccShrdPrm.nrOfMTC += dlt;

      if( ccShrdPrm.nrOfMTC < 0 ) ccShrdPrm.nrOfMTC = 0;

      break;

    case( CT_MOC_RDL):

      break;

    default:

      TRACE_EVENT( "UNEXP CALL TYPE IN CTB" );
  }

  if (ctb->calType NEQ CT_MOC_RDL)
  {
    if (ccShrdPrm.nrOfMTC EQ 0 AND    /* update state of TCH assignment */
        ccShrdPrm.nrOfMOC EQ 0)
    {
      /* If there is any call in CS_ACT_REQ state then do not disable vocoder */
      if(psaCC_ctbFindCall( (T_OWN)CMD_SRC_NONE, CS_ACT_REQ, CT_MTC ) < 0)
      {
        ccShrdPrm.TCHasg = FALSE;
      }
      cmhCC_TTY_Control (cId, TTY_STOP);
    }
  }

}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_chkPrgDesc        |
+-------------------------------------------------------------------+

  PURPOSE : this function checks for a valid progress description.
            if valid, it updates the shared parameters and notifies
            ACI.
*/

GLOBAL void psaCC_chkPrgDesc ( SHORT cId, UBYTE prgDesc, UBYTE msgType )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

  TRACE_FUNCTION("psaCC_chkPrgDesc( )");

  /* check in-band tones anouncement during call establishment
     GSM 04.08/5.5.1 */

  ccShrdPrm.msgType = msgType;
  ctb->prgDesc = prgDesc;

  if( msgType EQ MT_SETUP OR
      msgType EQ MT_ALRT  OR
      msgType EQ MT_PROC  OR
      msgType EQ MT_SYNC  OR
      msgType EQ MT_PROGR OR
      msgType EQ MT_CONN     )
  {

    if( prgDesc EQ MNCC_PROG_INBAND_AVAIL       OR
        prgDesc EQ MNCC_PROG_NO_END_TO_END_PLMN OR
        prgDesc EQ MNCC_PROG_DEST_NON_PLMN      OR
        prgDesc EQ MNCC_PROG_ORIGIN_NON_PLMN    OR
        (prgDesc >= 6 AND prgDesc <= 20) )
    {
      /* if( ccShrdPrm.TCHasg EQ TRUE ) */
      {
        TRACE_EVENT("Call with In-band tones");
        if (msgType NEQ MT_SETUP) //mars for AMR can not play with mobile MT call
        ctb->inBndTns = TRUE;      /* implicit connect to In-band tones */
      }
    }
    else if ( msgType EQ MT_ALRT )
    {
      cmhCC_CallAlerted(cId);
    }
  }

  /* check in-band tones anouncement during call disconnection
     GSM 04.08/5.4.4.1.1/5.4.4.1.2 */
  else if( msgType EQ MT_DISC )
  {
    if( prgDesc EQ MNCC_PROG_INBAND_AVAIL )
    {
      TRACE_EVENT("Call with In-band tones");
      if (msgType NEQ MT_SETUP) //mars for AMR can not play with mobile MT call
      ctb->inBndTns = TRUE;
    }
    else
      ctb->inBndTns = FALSE;
  }

  /*
   *  In the case of MT_DISC (mncc_disconnect_ind) cmhCC_CallDisconnected()
   *  called the CPI macro
   */
  if(MT_DISC NEQ msgType)
  {
    cmhCC_CallProceeding (cId);
  }

  /* Attach/Detach user connection as required */
  psaCC_setSpeechMode ();
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_getMOCTi          |
+-------------------------------------------------------------------+

  PURPOSE : this function selects a ti out of a pool of valid ti's
            and inserts it into the passed call table entry if the
            table index is valid. if no ti is available the function
            returns -1 otherwise it returns the selected ti.
            a bit of the pool stands for a valid ti.
            0 indicates a used ti, 1 indicates a free ti.
*/
GLOBAL SHORT psaCC_getMOCTi( SHORT cId )
{
  UBYTE idx;               /* holds pool idx */

  for( idx = 0; idx < MAX_MOCTI_NR; idx++ )
  {
    if( tiPool & (1u << idx) )
    {
      tiPool &= ~(1u << idx);

      if( cId >= 0 ) 
        psaCC_ctb(cId)->ti = idx;
      return( idx );
    }
  }
  return( -1 );
}
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_retMOCTi          |
+-------------------------------------------------------------------+

  PURPOSE : this function returns a used ti to the ti pool if the
            call was a MOC. the ti is free for the next MOC
            afterwards.
            a bit of the pool stands for a valid ti.
            0 indicates a used ti, 1 indicates a free ti.
*/

GLOBAL void psaCC_retMOCTi( UBYTE ti )
{
  if( ti < MAX_MOCTI_NR )

    tiPool |= (0x01 << ti);
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_setSpeechMode     |
+-------------------------------------------------------------------+

  PURPOSE : this function attachs or detachs the user connection.

*/

GLOBAL void psaCC_setSpeechMode (void)
{
  SHORT ctbIdx;             /* holds call table index */
  BOOL user_attach;         /* Attach/Detach user connection */

  TRACE_FUNCTION ("psaCC_setSpeechMode()");

  user_attach = FALSE;

  if (((ccShrdPrm.chMod EQ MNCC_CHM_SPEECH) OR
      (ccShrdPrm.chMod EQ MNCC_CHM_SPEECH_V2) OR
      (ccShrdPrm.chMod EQ MNCC_CHM_SPEECH_V3)) AND (ccShrdPrm.syncCs NEQ MNCC_CAUSE_REEST_STARTED))
  {
    /*
     * The channel mode is appropriate for speech.
     * Find any non-IDLE speech call where inband tones are available
     * or the active state has been reached. If there is such a call,
     * attach the user connection, otherwise detach it.
     */
    for (ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++)
    {
      T_CC_CALL_TBL *ctbx = ccShrdPrm.ctb[ctbIdx];

      if ((ctbx NEQ NULL) AND
          (ctbx->calStat NEQ NO_VLD_CS) AND
          (ctbx->calStat NEQ CS_IDL) AND
          (ctbx->calStat NEQ CS_HLD))
      {
        /*
         * The call is non-Idle and non-Hold. Check for inband tones,
         * active or call modification requested state.
         */
        if ( ctbx->inBndTns OR
            (ctbx->calStat EQ CS_ACT) OR
            (ctbx->calStat EQ CS_CPL_REQ) OR
            (ctbx->calStat EQ CS_MDF_REQ))
        {
//TISH: open vocoder if there is active call
          if (ctbx->calStat EQ CS_ACT)
	  {
		user_attach = TRUE;
		break;
	  }
          if (ccShrdPrm.TCHasg NEQ TRUE)
          {
              /* CCBS: 4.08/5.4.4.2ff do not connect to the in-band tone/announcement
              */
          }
          else
          {
            user_attach = TRUE;
            break;
          }
        }
      }
    }
  }

  hl_drv_set_vocoder_state(user_attach);

/* Implements Measure#32: Row 1237 & 1238 */
  if(user_attach)
  {
    TRACE_EVENT("User state: attach user");
  }
  else
  {
    TRACE_EVENT("User state: detach user");
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : psaCC_phbSrchNum        |
+-------------------------------------------------------------------+

  PURPOSE : phonebook search for number

*/

GLOBAL void psaCC_phbSrchNum ( SHORT cId, T_CC_CLTP call_type)
{
  UBYTE maxLen = MAX_ALPHA;           /* maximum length of entry   */
  CHAR  numBuf[MAX_PHB_NUM_LEN];      /* buffers number 'number\0' */

  TRACE_FUNCTION("psaCC_phbSrchNum");
  
  switch(call_type)
  {
    case CT_MOC:
    case CT_MOC_RDL:
    case CT_NI_MOC:
      if(!psaCC_ctbCldAdr2Num(cId, numBuf, MAX_PHB_NUM_LEN))
        return;
      break;
    case CT_MTC:
      if(!psaCC_ctbClrAdr2Num (cId, numBuf, MAX_PHB_NUM_LEN))
        return;
      break;
    default:
      return;
  }

  psaCC_phbSrchNumPlnTxt( numBuf, &maxLen, &psaCC_ctb(cId)->alphIdUni );

  return;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : psaCC_phbSrchNumPlnTxt  |
+-------------------------------------------------------------------+

  PURPOSE : phonebook search for number. Returns TRUE if at least 
            any matching number is found.

*/

GLOBAL BOOL psaCC_phbSrchNumPlnTxt ( CHAR          * inNum,
                                     UBYTE         * inoutMaxLen,
                                     T_ACI_PB_TEXT * outTxt )
{
#ifdef TI_PS_FFS_PHB
  SHORT        order_num;            /* Order number of entry by number */
#else
  SHORT        mtch    = 0;          /* holds number of matches    */
  SHORT        fstIdx;               /* holds first index          */
#endif
  UBYTE        lstIdx;               /* holds phonebook list index */
  T_PHB_RECORD ntry;                 /* found entry                */

  outTxt->cs  = CS_NotPresent;
  outTxt->len = 0;

  for ( lstIdx = 0; phbSrchLst[lstIdx] NEQ -1; lstIdx++ )
  {
#ifdef TI_PS_FFS_PHB
    order_num = 0;
    if (pb_search_number ((T_PHB_TYPE)phbSrchLst[lstIdx],
                          (const UBYTE*)inNum,
                          &order_num) EQ PHB_OK)
    {
      /* Implements Measure #30 */
      if (pb_sim_read_alpha_num_record ((T_PHB_TYPE)phbSrchLst[lstIdx],
                                        order_num, &ntry, 
                                        NUMBER_IDX) EQ PHB_OK)
#else
    pb_search_number ( (UBYTE)phbSrchLst[lstIdx],
                       (UBYTE*)inNum,
                       PHB_NEW_SEARCH,
                       &fstIdx, &mtch, &ntry );

    {
      if ( mtch NEQ 0 )
#endif
      {
        *inoutMaxLen = ( UBYTE ) MINIMUM ( ntry.tag_len, MAX_ALPHA );

        while ( ( outTxt->len < *inoutMaxLen ) AND
                ( ntry.tag[outTxt->len] NEQ 0xFF ) )
        {
          outTxt->data[outTxt->len] = ntry.tag[outTxt->len];
          outTxt->len++;
        }
        outTxt->cs = CS_Sim;

        return TRUE;
      }
    }
  }

  return FALSE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CMH_CC                    |
|                               ROUTINE : psaCC_phbMfwSrchNumPlnTxt |
+-------------------------------------------------------------------+

  PURPOSE : phonebook search for number. Returns TRUE if at least 
            any matching number is found.

*/

GLOBAL BOOL psaCC_phbMfwSrchNumPlnTxt ( CHAR          * inNum,
                                        T_ACI_PB_TEXT * outTxt )
{
#ifdef TI_PS_FFS_PHB
  SHORT        order_num;            /* Order number of entry by number */
#else
  SHORT        mtch    = 0;          /* holds number of matches    */
  SHORT        fstIdx;               /* holds first index          */
#endif
  UBYTE        maxLen;               /* holds max alpha length     */
  UBYTE        lstIdx;               /* holds phonebook list index */
  T_PHB_RECORD ntry;                 /* found entry                */

  TRACE_FUNCTION("psaCC_phbMfwSrchNumPlnTxt()");

  outTxt->len = 0;

  for ( lstIdx = 0; phbSrchLst[lstIdx] NEQ -1; lstIdx++ )
  {
#ifdef TI_PS_FFS_PHB
    order_num = 0;
    if (pb_search_number ((T_PHB_TYPE)phbSrchLst[lstIdx],
                          (const UBYTE*)inNum,
                          &order_num) EQ PHB_OK)
    {
      /* Implements Measure #30 */
      if (pb_sim_read_alpha_num_record ((T_PHB_TYPE)phbSrchLst[lstIdx],
                                        order_num, &ntry, 
                                        NUMBER_IDX) EQ PHB_OK)
#else
    pb_search_number ( (UBYTE)phbSrchLst[lstIdx],
                       (UBYTE*)inNum,
                       PHB_NEW_SEARCH,
                       &fstIdx, &mtch, &ntry );

    {
      if ( mtch NEQ 0 )
#endif
      {
        maxLen = ( UBYTE ) MINIMUM ( ntry.tag_len, MAX_ALPHA );

        outTxt -> len = 0;

        while ( outTxt -> len < maxLen AND ntry.tag[outTxt -> len] NEQ 0xFF )
        {
          outTxt -> data[outTxt -> len] = ntry.tag[outTxt -> len];
          outTxt -> len++;
        }

        return TRUE;
      }
    }
  }

  return FALSE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : psaCC_phbSrchName       |
+-------------------------------------------------------------------+

  PURPOSE : phonebook search for name

*/

GLOBAL BOOL psaCC_phbSrchName ( T_ACI_CMD_SRC srcId,
                                T_ACI_PB_TEXT *srchName,
                                T_CLPTY_PRM   *calPrm )
{
#ifdef TI_PS_FFS_PHB
  T_PHB_MATCH match_criteria;       /* matching criteria */
  SHORT order_num;                  /* Order number of entry by number */
#else
  SHORT fstIdx;                     /* holds first index */
  SHORT srchRslt = 0;               /* holds search result */
#endif
  UBYTE lstIdx;                     /* holds phonebook list index */
  T_PHB_RECORD phbNtry;             /* holds phonebook entry */

  memset( calPrm, 0x0, sizeof(T_CLPTY_PRM));

/*
 *-------------------------------------------------------------------
 * check for name search
 *-------------------------------------------------------------------
 */
  for( lstIdx = 0; phbSrchLst[lstIdx] NEQ -1; lstIdx++ )
  {
#ifdef TI_PS_FFS_PHB
    if (srcId EQ CMD_SRC_LCL)
      match_criteria = PHB_MATCH_GE;
    else
      match_criteria = PHB_MATCH_PARTIAL;

    order_num = 0;
    if (pb_search_name ((T_PHB_TYPE)phbSrchLst[lstIdx],
                        match_criteria,
                        srchName,
                        &order_num) EQ PHB_OK)
    {
      /* Implements Measure #30 */
      if ( pb_sim_read_alpha_num_record ((T_PHB_TYPE)phbSrchLst[lstIdx],
                                         order_num, &phbNtry, NAME_IDX) 
                                         EQ PHB_OK)
#else
    pb_search_name ( srcId,
                     (UBYTE)phbSrchLst[lstIdx], srchName, PHB_NEW_SEARCH,
                     &fstIdx, &srchRslt, &phbNtry );
    {
      if( srchRslt NEQ 0 )
#endif
      {
        cmhPHB_getAdrStr( calPrm->num, MAX_PHB_NUM_LEN-1,
                          phbNtry.number, phbNtry.len );

        cmh_demergeTOA ( phbNtry.ton_npi, &calPrm->ton, &calPrm->npi );

        return( TRUE );
      }
    }
  }

  return( FALSE );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : psaCC_phbSrchECC        |
+-------------------------------------------------------------------+

  PURPOSE : phonebook search for emergency number

*/

GLOBAL UBYTE psaCC_phbSrchECC ( CHAR* dialStr, BOOL srchECC )
{

#ifdef TI_PS_FFS_PHB
  SHORT order_num;                    /* Order number of entry by number */
  UBYTE dummy_num_len;
  UBYTE tag_len;
  UBYTE idx;
  SHORT max_rcd, used_rcd;
#else
  SHORT mtch;                         /* holds number of matches */
  SHORT fstIdx;                       /* holds first index */
  T_PHB_RECORD ntry;                  /* found entry */
  UBYTE service, tag_len;
  UBYTE idx;
  SHORT max_rcd, ecc_on_sim_count, avail_rcd;
#endif
SHORT dummy_max_ext, dummy_used_ext;

  TRACE_FUNCTION("psaCC_phbSrchECC");

  if (dialStr EQ NULL)
    return( MNCC_PRIO_NORM_CALL );

  /* remove any CLIR suppression/invocation prior checking for ECC */
/* Implements Measure#32: Row 89, 90, 116, 117, 1241 & 1242 */
  if (!strncmp( dialStr, ksd_supp_clir_str, 4) OR 
       !strncmp( dialStr, ksd_inv_clir_str, 4))
    dialStr+=4;          /* skip CLIR supression/invocation digits */

  if ( *dialStr EQ '\0' )
    return( MNCC_PRIO_NORM_CALL );  /* empty dialStr passed? */

  /* 
   * if emergency call numbers from SIM are available, only use
   * the emergency call numbers from the SIM
   */
#ifdef TI_PS_FFS_PHB
  if (pb_read_sizes (ECC, &max_rcd, &used_rcd, &dummy_num_len, &tag_len,
                     &dummy_max_ext, &dummy_used_ext) EQ PHB_OK)
  {
    if ( used_rcd )
    {
      /* SIM emergency numbers in SIM phonebook available */
      order_num = 0;
      if (pb_search_number (ECC, (const UBYTE*)dialStr, &order_num) EQ PHB_OK)
      {
        TRACE_EVENT("EMERGENCY_CALL (PHONEBOOK)!");
        return( MNCC_PRIO_EMERG_CALL );
      }

/* Implements Measure#32: Row 1239 */
      if (!strcmp(num_911_str,dialStr) OR !strcmp(num_112_str,dialStr))
      {
        TRACE_EVENT("EMERGENCY_CALL (!PHONEBOOK) 911 or 112");
        return( MNCC_PRIO_EMERG_CALL );
      }
      /* no valid emergency number */
      return( MNCC_PRIO_NORM_CALL );
    }
  }
#else
  if (pb_read_status( ECC, &service, &max_rcd, &ecc_on_sim_count, &tag_len, &avail_rcd,
  	&dummy_max_ext, &dummy_used_ext) EQ PHB_OK)
  {
    if ( ecc_on_sim_count )
    {
      /* SIM emergency numbers have been read and stored in phonebook */
      if (pb_search_number( ECC, (UBYTE*)dialStr, PHB_NEW_SEARCH, &fstIdx, &mtch, &ntry ) EQ PHB_OK)
      {
        if( mtch )
        {
          TRACE_EVENT("EMERGENCY_CALL (PHONEBOOK)!");
          return( MNCC_PRIO_EMERG_CALL );
        }
/* Implements Measure#32: Row 1239 */
        if(!strcmp(num_911_str,dialStr) OR !strcmp(num_112_str,dialStr))
        {
          TRACE_EVENT("EMERGENCY_CALL (!PHONEBOOK) 911 or 112");
            return( MNCC_PRIO_EMERG_CALL );
        }
        /* no valid emergency number */
        return( MNCC_PRIO_NORM_CALL );
      }
    }
  }
#endif

  /* check the default emergency numbers */
  for (idx=0; *ec_string[idx] NEQ '\0'; idx++)
  {
    if ( !strcmp(ec_string[idx], dialStr) )
    {
      /* if dialStr is a service number, then no emergency call */
#ifdef TI_PS_FFS_PHB
//TISH, patch for OMAPS00137002
//start
#if 0
      order_num = 0;
      if (pb_search_number (SDN, (const UBYTE*)dialStr, &order_num) EQ PHB_OK)
      {
        break;
      }
#endif      
//end  
      TRACE_EVENT("EMERGENCY_CALL! (DEFAULT)");
      return( MNCC_PRIO_EMERG_CALL );
    }
  }
#else
//TISH, patch for OMAPS00137002
//start
#if 0
      if (pb_search_number( SDN, (UBYTE*)dialStr, PHB_NEW_SEARCH, &fstIdx, &mtch, &ntry ) EQ PHB_OK)
      {
        if( mtch )
        {
          break;
        }
      }
#endif      
//end      
      TRACE_EVENT("EMERGENCY_CALL! (DEFAULT)");
      return( MNCC_PRIO_EMERG_CALL );
    }
  }
#endif

  /*if no valid SIM is inserted check the additional default emergency numbers */
  if ((simShrdPrm.SIMStat NEQ SS_OK)   AND
      (simShrdPrm.SIMStat NEQ SS_BLKD))
  {
    /* compare each additional default emergency call number */
    for (idx=0; *ec_string_ns[idx] NEQ '\0'; idx++)
    {
      if ( !strcmp(ec_string_ns[idx], dialStr) )
      {
        TRACE_EVENT("EMERGENCY_CALL! (DEFAULT WITHOUT SIM)");
        return ( MNCC_PRIO_EMERG_CALL );
      }
    }
  }

  /* search for emergency call numbers in the phonebook */
  if( srchECC )
#ifdef TI_PS_FFS_PHB
  {
    order_num = 0;
    if (pb_search_number (ECC, (const UBYTE*)dialStr, &order_num) EQ PHB_OK)
    {
      TRACE_EVENT("EMERGENCY_CALL!");
      return( MNCC_PRIO_EMERG_CALL );
    }
  }
#else
  {
    if (pb_search_number( ECC, (UBYTE*)dialStr, PHB_NEW_SEARCH, &fstIdx, &mtch, &ntry ) EQ PHB_OK)
    {
      if( mtch )
      {
        TRACE_EVENT("EMERGENCY_CALL!");
        return( MNCC_PRIO_EMERG_CALL );
      }
    }
  }
#endif

#ifdef TI_PS_FF_AT_CMD_P_ECC
  /*
   *--------------------------------------------------------------------
   * compare the dialed number with the ECC numbers already stored
   *--------------------------------------------------------------------
   */
  if(cmhCC_isNrInAdditionalECC(dialStr))
  {
    TRACE_EVENT("EMERGENCY_CALL! (additional ECC number)");
    return (MNCC_PRIO_EMERG_CALL);
  }
#endif /* TI_PS_FF_AT_CMD_P_ECC */
  /* no valid emergency number */
  return( MNCC_PRIO_NORM_CALL );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : psaCC_phbNtryFnd        |
+-------------------------------------------------------------------+

  PURPOSE : phonebook search if entry exist

*/

GLOBAL BOOL psaCC_phbNtryFnd ( UBYTE phb, T_CLPTY_PRM* calPrm )
{
#ifndef TI_PS_FFS_PHB
  SHORT mtchDmy = 0;                 /* holds number of matches */
  SHORT fstIdxDmy;                     /* holds first index */
#endif
  T_PHB_RECORD ntry;                  /* holds phonebook entry */
  SHORT order_num;                    /* Order number of entry by number */
  UBYTE toa;                          /* Type Of Address */

  TRACE_FUNCTION("psaCC_phbNtryFnd()");

  /* Problem for GSM string in FDN: if the call number includes an
     international '+', the number will not be found, because the
     FDN entry will not have a '+' for international calls, but ton
     is set to TON_International.
     Solution: copy the call parameter. If ton is not set to international,
     then check if '+' is embeded in the call number. If it has a '+',
     then remove it from the copied call number, set ton of the copy
     to international and use the copy for the FDN search.
  */

  /* search number */
  toa = cmh_mergeTOA ( calPrm -> ton, calPrm -> npi );

  if( phb EQ FDN )
  {
    T_CLPTY_PRM *calPrmCopy;

    ACI_MALLOC (calPrmCopy, sizeof (T_CLPTY_PRM));

    *calPrmCopy = *calPrm; /* Struct assignment */

    /* check  if '+' is within the string and remove it if it is */
    if (psaCC_handleInternatPlus(calPrmCopy->num))
    {
      /* set calling parameter for international */
      calPrmCopy->ton = TON_International;
      toa = cmh_mergeTOA( calPrmCopy->ton, calPrmCopy->npi );
    }
    if (calPrm->ton EQ MNCC_TON_UNKNOWN)
    {
      toa = 0;
    }
    ACI_MFREE (calPrmCopy);
    return (pb_check_fdn(toa, (const UBYTE*)calPrm->num) EQ PHB_OK);
  }
  else
#ifdef TI_PS_FFS_PHB
  {
    order_num = 0;
    if (pb_search_number ((T_PHB_TYPE)phb, (const UBYTE*)calPrm->num, &order_num) NEQ PHB_OK)
      return FALSE;

      /* Implements Measure #30 */
    if (pb_sim_read_alpha_num_record ((T_PHB_TYPE)phb, order_num, &ntry, NUMBER_IDX) 
                                      NEQ PHB_OK)
      return FALSE;
  }

  return ((toa EQ ntry.ton_npi) OR (ntry.ton_npi EQ 0xFF));
#else
  {
    pb_search_number( phb, (UBYTE*)calPrm->num, PHB_NEW_SEARCH,
                      &fstIdxDmy,&mtchDmy,&ntry );
  }

  if( mtchDmy NEQ 0 AND ((toa EQ ntry.ton_npi) OR (ntry.ton_npi EQ 0xFF) OR (toa EQ 0)) ) /* VO patch 02.03.01 */
  {
    return( TRUE );
  }
  else
  {
    return( FALSE );
  }
#endif
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : psaCC_handleInternatPlus|
+-------------------------------------------------------------------+

  PURPOSE : check if '+' is embeded in the call number. If it
            has a '+', then remove it from the call number

  RETURNS : TRUE,  if '+' has been found
            FALSE, if '+' has not been found
*/
LOCAL UBYTE psaCC_handleInternatPlus( char *number )
{
  UBYTE i;
  UBYTE str_len = strlen((char*)number);

  TRACE_FUNCTION("psaCC_handleInternatPlus()");

  /* Specific pos to check are number[4], number[5] and number[6] */
  if ( str_len > 4 )
  {
    for (i=4; (number[i] != 0) && (i<=6); i++)
    {
      if ( number[i] EQ '+' )
      {
        /* '+' for international number found */

        /* move all digits to right of '+' 1 position to the left to
           remove the '+' from the call number */
        memmove( (&number[i]), (&number[i+1]), str_len - (i+1) );
        number[str_len-1] = '\0';

        return TRUE;
      }
    }
  }
  /* no '+' for international number found */
  return FALSE;
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CC                  |
|                                 ROUTINE : psaCC_phbAddNtry        |
+-------------------------------------------------------------------+

  PURPOSE : add call table setting to specified phonebook

  NOTE:     This function can be called with a cId EQ NO_ENTRY.
            Not nice.

*/

GLOBAL void psaCC_phbAddNtry ( UBYTE phb, SHORT cId, UBYTE clTp, T_CLPTY_PRM *cldPty )
{
#ifndef TI_PS_FFS_PHB
  SHORT mtch;                         /* holds number of matches */
  SHORT fstIdx;                       /* holds first index */
#endif
  T_PHB_RECORD ntry;                  /* holds phonebook entry */
  SHORT order_num;                    /* holds first index */
  CHAR  numBuf[MAX_PHB_NUM_LEN];      /* buffers number 'number\0' */
  rtc_time_type rtc_time;             /* creation time for LDN,LRN,.. entry */

  TRACE_FUNCTION ("psaCC_phbAddNtry()");

  if( phb EQ LDN AND PBCFldn EQ PBCF_LDN_Disable ) return;
  if( phb EQ LRN AND PBCFlrn EQ PBCF_LRN_Disable ) return;
  if( phb EQ LMN AND PBCFlmn EQ PBCF_LMN_Disable ) return;

  if (cldPty NEQ NULL)  /* Explicit number passed? */
  {
    strncpy(numBuf, cldPty->num, MAX_PHB_NUM_LEN-1);
    numBuf[MAX_PHB_NUM_LEN-1]='\0';
  }
  else           /* otherwise read number from CTB and add DTMF */
  {
    switch (clTp)
    {
      case CT_MTC:
        /* Save time stamp of MT Call even if number is not known */
        psaCC_ctbClrAdr2Num( cId, numBuf, MAX_PHB_NUM_LEN );
        break;
      case CT_MOC:
      case CT_NI_MOC:   /* Maybe also for CCBS */
        if (!psaCC_ctbCldAdr2Num( cId, numBuf, MAX_PHB_NUM_LEN ))
          return;
        break;
      default:          /* invalid or no call */
        return;
    }

    if (strlen(numBuf) < MAX_PHB_NUM_LEN-1)    /* is there still room for DTMF digits? */
    {
      strncat(numBuf,
              (char *)ccShrdPrm.dtmf.dig,
              (MAX_PHB_NUM_LEN-1) - strlen(numBuf) );      /* remember the previos cut off DTMF digits */
    }
  }

  if ((phb NEQ LDN) AND (phb NEQ LRN) AND (phb NEQ LMN))
#ifdef TI_PS_FFS_PHB
  {
    order_num = 0;
    if (pb_search_number ((T_PHB_TYPE)phb, (const UBYTE*)numBuf, &order_num) EQ PHB_OK)
      return; /* entry already exist */
  }

  memset (&ntry, 0, sizeof (T_PHB_RECORD));

  ntry.phy_recno = 0; /* Search for a free record */
  memset((char *)ntry.tag, 0xFF, PHB_MAX_TAG_LEN);
  memset((char *)ntry.subaddr, 0xFF, PHB_PACKED_SUB_LEN);
#else
  {
    pb_search_number( phb, (UBYTE*)numBuf, PHB_NEW_SEARCH,
                      &fstIdx,&mtch,&ntry );

    if( mtch NEQ 0 ) return;    /* entry already exist */
  }

  ntry.index = 0;
  memset((char *)ntry.tag, 0xFF, PHB_MAX_TAG_LEN);
  memset((char *)ntry.subaddr, 0xFF, PHB_PACKED_NUM_LEN);
#endif

  if (cldPty NEQ NULL)  /* Explicit number passed? */
  {
    T_ACI_PB_TEXT tag;
    ntry.tag_len = PHB_MAX_TAG_LEN;

    if (psaCC_phbSrchNumPlnTxt ( numBuf, &ntry.tag_len, &tag ))
      memcpy(ntry.tag, tag.data, ntry.tag_len);
    else
      ntry.tag_len = 0;

    ntry.ton_npi=((cldPty->ton & 0x07) << 4 ) +
                  (cldPty->npi & 0x0F) + 0x80;
  }
  else
  {
    /* 
     * Probably it is important to use this pointer variable here!
     */
    T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

    if ( ctb->alphIdUni.cs EQ CS_Sim )
    {
      cmhPHB_getTagSim( &ctb->alphIdUni, ntry.tag, PHB_MAX_TAG_LEN );
      ntry.tag_len = MINIMUM ( ctb->alphIdUni.len, PHB_MAX_TAG_LEN );
    }
    else
      ntry.tag_len = 0;

    if( clTp EQ CT_MOC )
      ntry.ton_npi = ((ctb->cldPty.ton & 0x07) << 4 ) +
                      (ctb->cldPty.npi & 0x0F) + 0x80;

    else if ( clTp EQ CT_MTC )
      ntry.ton_npi = ((ctb->clgPty.ton & 0x07) << 4 ) +
                      (ctb->clgPty.npi & 0x0F) + 0x80;
  }

  cmhPHB_getAdrBcd ( ntry.number, &ntry.len,
                     PHB_PACKED_NUM_LEN, numBuf );

  ntry.cc_id  = 0xFF;
#ifdef TI_PS_FFS_PHB
  ntry.v_time = FALSE;
  ntry.v_line = FALSE;
#endif

  if ( ( phb EQ LDN ) OR ( phb EQ LRN ) OR ( phb EQ LMN ) )
  {
    if ( rtc_read_time ( &rtc_time ) EQ TRUE )
    {
#ifdef TI_PS_FFS_PHB
      ntry.v_time = TRUE;
      ntry.time.year   = rtc_time.year;
      ntry.time.month  = rtc_time.month;
      ntry.time.day    = rtc_time.day;
      ntry.time.hour   = rtc_time.hour;
      ntry.time.minute = rtc_time.minute;
      ntry.time.second = rtc_time.second;
#else
      ntry.year   = rtc_time.year;
      ntry.month  = rtc_time.month;
      ntry.day    = rtc_time.day;
      ntry.hour   = rtc_time.hour;
      ntry.minute = rtc_time.minute;
      ntry.second = rtc_time.second;
#endif

    }

#ifdef TI_PS_FFS_PHB
    ntry.v_line = TRUE;
#endif

    if (cId NEQ NO_ENTRY AND
        psaCC_ctb(cId)->BC[0].bearer_serv EQ MNCC_BEARER_SERV_AUX_SPEECH)
    {
      ntry.line = 2;
    }
    else
    {
      ntry.line = 1;
    }
  }
  /* ACI-SPR-16301: result might also be PHB_EXCT */
#ifdef TI_PS_FFS_PHB
  if( pb_add_record ( (T_PHB_TYPE)phb, ntry.phy_recno, &ntry ) EQ PHB_FAIL )
#else
  if( pb_add_record ( phb, ntry.index, &ntry ) EQ PHB_FAIL )
#endif
  {
    TRACE_EVENT( "PHONEBOOK ADDING FAILED" );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_asmBuildMPTY      |
+-------------------------------------------------------------------+

  PURPOSE : assemble the build multiparty SS facility
            information element. return invoke id.
*/

GLOBAL void psaCC_asmBuildMPTY ( void )
{
  TRACE_FUNCTION("psaCC_asmBuildMPTY");

  memset( &ssFIECodeBuf, 0, sizeof( ssFIECodeBuf ));

  ssFIECodeBuf.l_buf = 8;
  ssFIECodeBuf.o_buf = 0;
  ssFIECodeBuf.buf[0] = OPC_BUILD_MPTY;

  ccShrdPrm.cmpType = CT_INV;
}

/* Implements Measure #86 */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_asmSplitMPTY      |
+-------------------------------------------------------------------+

  PURPOSE : assemble the split multiparty SS facility
            information element. return invoke id.
*/

GLOBAL void psaCC_asmSplitMPTY ( void )
{
  TRACE_FUNCTION("psaCC_asmSplitMPTY");

  memset( &ssFIECodeBuf, 0, sizeof( ssFIECodeBuf ));

  ssFIECodeBuf.l_buf = 8;
  ssFIECodeBuf.o_buf = 0;
  ssFIECodeBuf.buf[0] = OPC_SPLIT_MPTY;

  ccShrdPrm.cmpType = CT_INV;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_asmECT            |
+-------------------------------------------------------------------+

  PURPOSE : assemble the explicit call transfer SS facility
            information element. return invoke id.
*/

GLOBAL void psaCC_asmECT ( void )
{
  TRACE_FUNCTION("psaCC_asmECT");

  memset( &ssFIECodeBuf, 0, sizeof( ssFIECodeBuf ));

  ssFIECodeBuf.l_buf = 8;
  ssFIECodeBuf.o_buf = 0;
  ssFIECodeBuf.buf[0] = OPC_EXPLICIT_CT;

  ccShrdPrm.cmpType = CT_INV;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_asmCUGInfo        |
+-------------------------------------------------------------------+

  PURPOSE : assemble the forward CUG info SS facility
            information element.
*/

GLOBAL void psaCC_asmCUGInfo ( SHORT cId )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
  MCAST( CUGinf, FWD_CUG_INFO_INV );

  TRACE_FUNCTION("psaCC_asmCUGInfo");

  memset( CUGinf, 0, sizeof( T_FWD_CUG_INFO_INV ));

  /* set basic settings */
  CUGinf->msg_type            = FWD_CUG_INFO_INV;
  CUGinf->v_forwardCUGInfoArg = TRUE;

  /* set CUG info */
  if( ctb->CUGidx NEQ NOT_PRESENT_8BIT )
  {
    CUGinf->forwardCUGInfoArg.v_cugIndex = TRUE;
    CUGinf->forwardCUGInfoArg.cugIndex   = ctb->CUGidx;
  }
  CUGinf->forwardCUGInfoArg.v_suppressPrefCUG = ctb->CUGprf;
  CUGinf->forwardCUGInfoArg.v_suppressOA      = ctb->OAsup;

  memset( &ssFIECodeBuf, 0, sizeof( ssFIECodeBuf ));
  ssFIECodeBuf.l_buf = MAX_FIE_CODE_BUF_LEN<<3;
  ccd_codeMsg (CCDENT_FAC, UPLINK,
               (T_MSGBUF *)&ssFIECodeBuf, _decodedMsg,
               NOT_PRESENT_8BIT);

  ccShrdPrm.cmpType = CT_INV;
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_asmCDReq          |
+-------------------------------------------------------------------+

  PURPOSE : Assemble the Invoke component for CallDeflection.
            On calling this function, it is assumed the semaphore
            for the shared CCD buffer is already set.

*/

GLOBAL void psaCC_asmCDReq ( const CHAR      *number,
                             const T_ACI_TOA *type,
                             const CHAR      *subaddr,
                             const T_ACI_TOS *satype)
{
  MCAST ( invokeCD, CALL_DEFLECTION_INV );
  UBYTE ton, tos;           /* holds type of number/subaddress */
  UBYTE npi, oe;            /* holds numbering plan/odd.even indicator */
  UBYTE ccdRet;

  TRACE_FUNCTION ("psaCC_asmCDReq()");

  memset( invokeCD, 0, sizeof( T_CALL_DEFLECTION_INV ));

  /* set basic settings */
  invokeCD->msg_type            = CALL_DEFLECTION_INV;
  invokeCD->v_callDeflectionArg = TRUE;

  /* Process T_deflectedToNumber (mandatory element) */
  if ((type EQ NULL) OR
      (type->npi EQ NPI_NotPresent) OR
      (type->ton EQ TON_NotPresent))
  {
    /* Use defaults */
    ton = (number[0] EQ '+') ? NOA_INTER_NUM : NOA_UNKNOWN;
    npi = NPI_ISDN;
  }
  else
  {
    /* Use given values */
    ton = type->ton;
    npi = type->npi;
  }
  invokeCD->callDeflectionArg.v_deflectedToNumber     = TRUE;
  invokeCD->callDeflectionArg.deflectedToNumber.v_noa = TRUE;
  invokeCD->callDeflectionArg.deflectedToNumber.noa   = ton;
  invokeCD->callDeflectionArg.deflectedToNumber.v_npi = TRUE;
  invokeCD->callDeflectionArg.deflectedToNumber.npi   = npi;
  invokeCD->callDeflectionArg.deflectedToNumber.c_bcdDigit =
    (UBYTE)utl_dialStr2BCD (number,
        invokeCD->callDeflectionArg.deflectedToNumber.bcdDigit,
        MAX_PARTY_NUM);

  /* Process T_deflectedToSubaddress (optional element) */
  if (subaddr NEQ NULL)
  {
    if ((satype EQ NULL) OR
        (satype->tos EQ TOS_NotPresent) OR
        (satype->oe  EQ OE_NotPresent))
    {
      /* Use defaults */
      tos = TOS_X213;
      oe  = OEI_EVEN;
    }
    else
    {
      /* Use given values */
      tos = satype->tos;
      oe  = satype->oe;
    }
    invokeCD->callDeflectionArg.v_deflectedToSubaddress     = TRUE;
    invokeCD->callDeflectionArg.deflectedToSubaddress.v_tos = TRUE;
    invokeCD->callDeflectionArg.deflectedToSubaddress.tos   = tos;
    invokeCD->callDeflectionArg.deflectedToSubaddress.v_oei = TRUE;
    invokeCD->callDeflectionArg.deflectedToSubaddress.oei   = oe;
    invokeCD->callDeflectionArg.deflectedToSubaddress.c_subadr_str =
      (UBYTE)utl_dialStr2BCD(subaddr,
          invokeCD->callDeflectionArg.deflectedToSubaddress.subadr_str,
          MAX_SUBADDR_NUM);
  }

  memset( &ssFIECodeBuf, 0, sizeof( ssFIECodeBuf ));
  ssFIECodeBuf.l_buf = MAX_FIE_CODE_BUF_LEN<<3;

  ccdRet = ccd_codeMsg (CCDENT_FAC, UPLINK,
                        (T_MSGBUF *)&ssFIECodeBuf, _decodedMsg,
                        NOT_PRESENT_8BIT);

  switch (ccdRet)
  {
    case ccdOK:
      break;
    case ccdWarning:
      TRACE_EVENT ("ccdWarning");
      break;
    default:
      TRACE_ERROR ("ccdError/other problem");
      break;
  }

  ccShrdPrm.cmpType = CT_INV; /* Component Type = Invoke */
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_asmComponent      |
+-------------------------------------------------------------------+

  PURPOSE : Build the componenet. The content of the component is
            expected in ssFIECodeBuf, the ready built component will
            be returned in ssFIECodeBuf.

*/

GLOBAL void psaCC_asmComponent ( SHORT cId )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
  MCAST (comp, COMPONENT);
  UBYTE ccdRet;

  TRACE_FUNCTION ("psaCC_asmComponent()");

  memset (comp, 0, sizeof (T_COMPONENT));

  comp->v_inv_comp = TRUE;
  comp->inv_comp.v_inv_id  = TRUE;
  comp->inv_comp.inv_id    = ctb->iId    = ccShrdPrm.iIdNxt++;
  comp->inv_comp.v_op_code = TRUE;
  comp->inv_comp.op_code   = ctb->opCode = ssFIECodeBuf.buf[0];
  comp->inv_comp.v_params  = TRUE;
  comp->inv_comp.params.l_params = ssFIECodeBuf.l_buf - 8;
  comp->inv_comp.params.o_params = 8;

  memcpy (comp->inv_comp.params.b_params,
          ssFIECodeBuf.buf + (ssFIECodeBuf.o_buf >> 3),
          ssFIECodeBuf.l_buf >> 3);

  ccdRet = ccd_codeMsg (CCDENT_FAC,
                        UPLINK,
                        (T_MSGBUF *)&ssFIECodeBuf,
                        _decodedMsg,
                        COMPONENT);

  if (ccdRet NEQ ccdOK)
  {
    TRACE_EVENT_P1 ("ccdRet=%d NEQ ccdOK", ccdRet);
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_asmCCBSReq        |
+-------------------------------------------------------------------+

  PURPOSE : assemble the access register CC entry SS facility
            information element.
*/

GLOBAL void psaCC_asmCCBSReq ( SHORT cId )
{
  MCAST( CCBSreq, ACC_REGISTER_CC_ENTRY_INV );

  TRACE_FUNCTION("psaCC_asmCCBSReq");

  memset( CCBSreq, 0, sizeof( T_ACC_REGISTER_CC_ENTRY_INV ));

  /* set basic settings */
  CCBSreq->msg_type                = ACC_REGISTER_CC_ENTRY_INV;
  CCBSreq->v_accRegisterCCEntryArg = TRUE;
  CCBSreq->accRegisterCCEntryArg.l_accRegisterCCEntryArg = 0;

  memset( &ssFIECodeBuf, 0, sizeof( ssFIECodeBuf ));
  ssFIECodeBuf.l_buf = MAX_FIE_CODE_BUF_LEN<<3;
  ccd_codeMsg (CCDENT_FAC, UPLINK,
               (T_MSGBUF *)&ssFIECodeBuf, _decodedMsg,
               NOT_PRESENT_8BIT);

  ccShrdPrm.cmpType = CT_INV;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_dasmInvokeCmp     |
+-------------------------------------------------------------------+

  PURPOSE : disassemble the result component.
*/

GLOBAL void psaCC_dasmInvokeCmp ( SHORT cId, T_inv_comp *invCmp )
{
  USHORT ivId;                /* holds invoke Id */
  UBYTE  opCode;              /* holds operation code */

  TRACE_FUNCTION("psaCC_dasmInvokeCmp");

  if( invCmp -> v_inv_id )
    ivId = invCmp -> inv_id;
  else
    ivId = NOT_PRESENT_16BIT;

  if( invCmp -> v_op_code )
    opCode = invCmp -> op_code;
  else
    opCode = NOT_PRESENT_8BIT;

  if( invCmp -> params.l_params )
  {
    UBYTE ccdRet;

    memcpy( &ssFIECodeBuf, &invCmp -> params, sizeof(ssFIECodeBuf));

    ccdRet = ccd_decodeMsg (CCDENT_FAC,
                            DOWNLINK,
                            (T_MSGBUF *) &ssFIECodeBuf,
                            (UBYTE    *) _decodedMsg,
                            opCode);

    if( ccdRet NEQ ccdOK )
    {
      TRACE_EVENT_P1( "CCD Decoding Error: %d",ccdRet );
      psaCC_ctb(cId)->failType = SSF_CCD_DEC;
      cmhCC_SSTransFail(cId);
    }
  }

  /* determine to which operation the invoke belongs to */
  switch( opCode )
  {
    /*
     * Advice of Charge Information received
     */
    case OPC_FWD_CHARGE_ADVICE:
      {
        MCAST( aoc_para, FWD_CHG_ADVICE_INV);

        aoc_parameter (cId, aoc_para);

        if (aoc_info (cId, AOC_START_AOC))
        {
          /*
           * build return result to infrastructure
           * directly without using CCD (it´s too simple)
           */
          PALLOC (facility_req, MNCC_FACILITY_REQ);

          facility_req->fac_inf.l_fac   = 5*8;
          facility_req->fac_inf.o_fac   = 0;
          facility_req->fac_inf.fac [0] = 0xA2;         /* component type tag     */
          facility_req->fac_inf.fac [1] = 3;            /* component type length  */
          facility_req->fac_inf.fac [2] = 2;            /* invoke id tag          */
          facility_req->fac_inf.fac [3] = 1;            /* invoke id length       */
          facility_req->fac_inf.fac [4] = (UBYTE)ivId;  /* invoke id              */

          facility_req->ti              = psaCC_ctb(cId)->ti;
          facility_req->ss_version      = NOT_PRESENT_8BIT;

          PSENDX (CC, facility_req);
        }
      }
      break;

    case( OPC_NOTIFY_SS ):
      {
        MCAST( ntfySS, NOTIFY_SS_INV );

        cmhCC_NotifySS( cId, ntfySS );
      }
      break;

    case( OPC_FWD_CHECK_SS_IND ):
      {
        cmhCC_CheckSS( cId);
      }
      break;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_dasmResultCmp     |
+-------------------------------------------------------------------+

  PURPOSE : disassemble the result component.
*/

GLOBAL void psaCC_dasmResultCmp ( SHORT cId, T_res_comp *resCmp )
{
  UBYTE  opCode;              /* holds operation code */

  TRACE_FUNCTION("psaCC_dasmResultCmp");

  /* get operation code of the result */
  if( resCmp -> v_sequence AND resCmp -> sequence.v_op_code )
    opCode = resCmp -> sequence.op_code;
  else
    opCode = psaCC_ctb(cId)->opCode;

  /* decode additional parameters of result */
  if( resCmp -> v_sequence AND resCmp -> sequence.params.l_params )
  {
    UBYTE ccdRet;

    memcpy( &ssFIECodeBuf, &resCmp -> sequence.params, sizeof(ssFIECodeBuf));

    ccdRet = ccd_decodeMsg (CCDENT_FAC,
                            DOWNLINK,
                            (T_MSGBUF *) &ssFIECodeBuf,
                            (UBYTE    *) _decodedMsg,
                            opCode);

    if( ccdRet NEQ ccdOK )
    {
      TRACE_EVENT_P1( "CCD Decoding Error: %d",ccdRet );
      return;
    }
  }

  /* determine to which operation the result belongs to */
  switch( opCode )
  {
    case( OPC_BUILD_MPTY ):
      {
        MCAST( bldMPTY, BUILD_MPTY_RES );

        cmhCC_MPTYBuild( cId, bldMPTY );
      }
      break;

    case( OPC_EXPLICIT_CT ):
      {
        TIMERSTOP( ACI_TECT );
      }
      break;

    case( OPC_SPLIT_MPTY ):
      {
        MCAST( splMPTY, SPLIT_MPTY_RES );

        cmhCC_MPTYSplit( cId, splMPTY );
      }
      break;

    case( OPC_HOLD_MPTY ):
      {
        MCAST( hldMPTY, HOLD_MPTY_RES );

        cmhCC_MPTYHeld( cId, hldMPTY );
      }
      break;

    case( OPC_RETRIEVE_MPTY ):
      {
        MCAST( rtvMPTY, RETRIEVE_MPTY_RES );

        cmhCC_MPTYRetrieved( cId, rtvMPTY );
      }
      break;

    case( OPC_ACC_REGISTER_CC_ENTRY ):
      {
        MCAST( ccbs, ACC_REGISTER_CC_ENTRY_RES );

        cmhCC_CCBSRegistered( cId, ccbs );
      }
      break;

    case( OPC_CALL_DEFLECTION ):
      {
        cmhCC_CDRegistered ( cId );
      }
      break;

  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_dasmErrorCmp      |
+-------------------------------------------------------------------+

  PURPOSE : disassemble the error component.
*/

GLOBAL void psaCC_dasmErrorCmp ( SHORT cId, T_err_comp *errCmp )
{
  TRACE_FUNCTION("psaCC_dasmErrorCmp");

  if( errCmp -> v_err_code )
  {
    psaCC_ctb(cId)->failType = SSF_ERR_PRB;
    psaCC_ctb(cId)->errCd    = errCmp -> err_code;
  }

  cmhCC_SSTransFail (cId);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSF                 |
|                                 ROUTINE : psaCC_dasmRejectCmp     |
+-------------------------------------------------------------------+

  PURPOSE : disassemble the error component.
*/

GLOBAL void psaCC_dasmRejectCmp ( SHORT cId, T_rej_comp *rejCmp )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

  TRACE_FUNCTION("psaCC_dasmRejectCmp");

  if( rejCmp -> v_gen_problem )
  {
    ctb->failType = SSF_GEN_PRB;
    ctb->rejPrb   = rejCmp -> gen_problem;
  }

  else if( rejCmp -> v_inv_problem )
  {
    ctb->failType = SSF_INV_PRB;
    ctb->rejPrb   = rejCmp -> inv_problem;
  }

  else if( rejCmp -> v_res_problem )
  {
    ctb->failType = SSF_RSL_PRB;
    ctb->rejPrb   = rejCmp -> res_problem;
  }

  else if( rejCmp -> v_err_problem )
  {
    ctb->failType = SSF_ERR_PRB;
    ctb->rejPrb   = rejCmp -> err_problem;
  }

  cmhCC_SSTransFail(cId);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_DTMFSent          |
+-------------------------------------------------------------------+

  PURPOSE : previous DTMF digit was sent

*/
GLOBAL void psaCC_DTMFSent ( SHORT cId )
{
  TRACE_FUNCTION( "psaCC_DTMFSent()" );

 
/*-------------------------------------------------------------------
 * check current command
 *-------------------------------------------------------------------
 */
  if (!psaCC_ctbIsValid (cId))
  return;


#ifdef SIM_TOOLKIT
  if ( psaCC_ctb(cId)->dtmfSrc EQ OWN_SRC_SAT )  /* confirm to SAT send DTMF cmd */
  {
    cmhCC_SatDTMFsent ( cId );
    return;
  }
  else
#endif /* SIM_TOOLKIT */
  {
    cmhCC_DTMFsent ( cId );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_StopDTMF          |
+-------------------------------------------------------------------+

  PURPOSE : stop further DTMF generation for that call

*/

GLOBAL void psaCC_StopDTMF ( SHORT cId )
{
#ifdef SIM_TOOLKIT
  T_ACI_SAT_TERM_RESP resp_data;
#endif /* SIM_TOOLKIT */

  TRACE_FUNCTION( "psaCC_StopDTMF()" );

  if( cId EQ ccShrdPrm.dtmf.cId )
  {
    T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

    TIMERSTOP( ACI_TDTMF);

#ifdef SIM_TOOLKIT
    psaSAT_InitTrmResp( &resp_data );

      /* if other dtmf were to be sent in send DTMF SAT cmd*/
    if ( ctb->dtmfSrc EQ OWN_SRC_SAT AND
         ccShrdPrm.dtmf.cnt NEQ 0 )
    {
      resp_data.add_content = ADD_ME_NO_SPCH_CALL; /* tell SIM that it failed */
      psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
    }
    else /*Stop DTMF generation before calling cmhCC_DTMFstopped */
#endif /* SIM_TOOLKIT */
    {
      cmhCC_DTMFstopped ( cId );
    }
    ctb->dtmfSrc = OWN_SRC_INV;
    ctb->dtmfCmd = AT_CMD_NONE;
    ccShrdPrm.dtmf.cId = NO_ENTRY;
    ccShrdPrm.dtmf.cnt = 0;
    ccShrdPrm.dtmf.cur = 0;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_DTMFTimeout       |
+-------------------------------------------------------------------+

  PURPOSE : handle DTMF timeout

*/

GLOBAL void psaCC_DTMFTimeout ( void )
{
  TRACE_FUNCTION( "psaCC_DTMFTimeout()" );

  psaCC_DTMFSent( ccShrdPrm.dtmf.cId );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CC                  |
|                                 ROUTINE : psaCC_InitCtbNtry       |
+-------------------------------------------------------------------+

  PURPOSE : allocate and initialize the indexed call table entry.

*/

GLOBAL void psaCC_InitCtbNtry ( SHORT idx )
{
  T_CC_CALL_TBL *ctb;

  /*
   *-------------------------------------------------------------------
   * initialize call table entry
   *-------------------------------------------------------------------
   */
  TRACE_FUNCTION ("psaCC_InitCtbNtry()");

  if (ccShrdPrm.ctb[idx] NEQ NULL)
  {
    TRACE_EVENT ("Suspicious: ccShrdPrm.ctb[idx] was not freed");
    ACI_MFREE (ccShrdPrm.ctb[idx]); /* Make sure we get no leak */
  }
  ACI_MALLOC (ccShrdPrm.ctb[idx], sizeof (T_CC_CALL_TBL));

  ctb = ccShrdPrm.ctb[idx];

  ctb->ti                 = 0xFF;
  ctb->calStat            = NO_VLD_CS;
  ctb->calType            = NO_VLD_CT;
  ctb->alrtStat           = AS_IDL;
  ctb->inBndTns           = FALSE;
  ctb->prgDesc            = MNCC_PROG_NOT_PRES;
  ctb->BC[0].rate         = DEF_BC1_UR;
  ctb->BC[0].bearer_serv  = DEF_BC1_BS;
  ctb->BC[0].conn_elem    = DEF_BC1_CE;
  ctb->BC[0].stop_bits    = DEF_BC1_SB;
  ctb->BC[0].data_bits    = DEF_BC1_DB;
  ctb->BC[0].parity       = DEF_BC1_PR;
  ctb->BC[0].flow_control = DEF_BC1_FC;
  ctb->BC[0].modem_type   = DEF_BC1_MT;
  ctb->BC[0].transfer_cap = DEF_BC1_TC;
  ctb->BC[0].rate_adaption= DEF_BC1_RA;
  ctb->BC[1].rate         = DEF_BC2_UR;
  ctb->BC[1].bearer_serv  = DEF_BC2_BS;
  ctb->BC[1].conn_elem    = DEF_BC2_CE;
  ctb->BC[1].stop_bits    = DEF_BC2_SB;
  ctb->BC[1].data_bits    = DEF_BC2_DB;
  ctb->BC[1].parity       = DEF_BC2_PR;
  ctb->BC[1].flow_control = DEF_BC2_FC;
  ctb->BC[1].modem_type   = DEF_BC2_MT;
  ctb->curBC              = 0;
  ctb->rptInd             = DEF_RPT_IND;
  ctb->sigInf             = MNCC_SIG_NOT_PRES;
  ctb->prio               = 0xFF;
  ctb->CLIRsup            = DEF_CLIR_SUP;
  ctb->mptyStat           = CS_IDL;
  ctb->iId                = 0;
  ctb->srvStat            = SSS_IDL;
  ctb->srvType            = NO_VLD_ST;
  ctb->SSver              = DEF_SS_VER;
  ctb->CUGidx             = NOT_PRESENT_8BIT;
  ctb->CUGprf             = FALSE;
  ctb->OAsup              = FALSE;
  ctb->opCode             = NOT_PRESENT_8BIT;
  ctb->rslt               = CAUSE_MAKE (DEFBY_CONDAT, 
                                        ORIGSIDE_MS,
                                        MNCC_CC_ORIGINATING_ENTITY,
                                        NOT_PRESENT_8BIT);
  ctb->nrmCs              = CAUSE_MAKE (DEFBY_CONDAT,
                                        ORIGSIDE_MS,
                                        MNCC_CC_ORIGINATING_ENTITY,
                                        NOT_PRESENT_8BIT);
  ctb->rejCs              = CAUSE_MAKE (DEFBY_CONDAT,
                                        ORIGSIDE_MS,
                                        MNCC_CC_ORIGINATING_ENTITY,
                                        NOT_PRESENT_8BIT);
  ctb->failType           = NO_VLD_SSF;
  ctb->rejPrb             = NOT_PRESENT_8BIT;
  ctb->errCd              = NOT_PRESENT_8BIT;
  ctb->ssDiag             = MNCC_SS_DIAG_NOT_PROVIDED;  
  ctb->SATinv             = FALSE;
  ctb->CCBSstat           = NO_VLD_CCBSS;
  ctb->CDStat             = NO_VLD_CD;
  ctb->curCmd             = AT_CMD_NONE;
  ctb->curSrc             = CMD_SRC_NONE;
  ctb->dtmfCmd            = AT_CMD_NONE;
  ctb->dtmfSrc            = (T_OWN)CMD_SRC_NONE;
  ctb->calOwn             = (T_OWN)CMD_SRC_NONE;
  ctb->alphIdUni.cs       = CS_NotPresent;
  ctb->alphIdUni.len      = 0;

  memset (&ctb->cldPty,    0, sizeof(T_dyn_called_party));
  memset (&ctb->cldPtySub, 0, sizeof(T_MNCC_called_party_sub));
  memset (&ctb->clgPty,    0, sizeof(T_MNCC_calling_party));
  memset (&ctb->clgPtySub, 0, sizeof(T_MNCC_calling_party_sub));
  memset (&ctb->rdrPty,    0, sizeof(T_dyn_redir_party));
  memset (&ctb->rdrPtySub, 0, sizeof(T_dyn_redir_party_sub));
  ctb->rdlCnt             = 0;
  ctb->rdlTimIndex        = RDL_TIM_INDEX_NOT_PRESENT;
  ctb->curCs              = MNCC_CAUSE_NO_MS_CAUSE;
  ccShrdPrm.ccCs[idx]     = CAUSE_MAKE (DEFBY_STD, 
                                        ORIGSIDE_MS, 
                                        MNCC_ACI_ORIGINATING_ENTITY, 
                                        NOT_PRESENT_8BIT);
  ccShrdPrm.callType[idx] = (S8)NO_VLD_CC_CALL_TYPE;
  ctb->numRawCauseBytes   = 0;
  ctb->rawCauseBytes      = NULL;
#ifdef SIM_TOOLKIT
/*
 *-----------------------------------------------------------------
 * SAT settings
 *-----------------------------------------------------------------
 */
  ctb->SatDiscEvent = FALSE;
#endif
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CC                  |
|                                 ROUTINE : psaCC_FreeRdrPty        |
+-------------------------------------------------------------------+

  PURPOSE : free (re-initialize) the redirecting party number and
            the redirecting party subaddress of the indexed call
            table entry.

*/

GLOBAL void psaCC_FreeRdrPty ( SHORT idx )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[idx];

  TRACE_FUNCTION ("psaCC_FreeRdrPty()");

  if (ctb->rdrPty.redir_num NEQ NULL)
  {
    ACI_MFREE (ctb->rdrPty.redir_num);
    memset (&ctb->rdrPty,    0, sizeof(T_dyn_redir_party));
  }
  if (ctb->rdrPtySub.subaddr NEQ NULL)
  {
    ACI_MFREE (ctb->rdrPtySub.subaddr);
    memset (&ctb->rdrPtySub, 0, sizeof(T_dyn_redir_party_sub));
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CC                  |
|                                 ROUTINE : psaCC_FreeCtbNtry       |
+-------------------------------------------------------------------+

  PURPOSE : free (re-initialize) the indexed call table entry.

*/

GLOBAL void psaCC_FreeCtbNtry ( SHORT idx )
{
  T_CC_CALL_TBL *ctb;
  USHORT listcounter;
  T_ACI_LIST *msg2send;

  TRACE_FUNCTION ("psaCC_FreeCtbNtry()");

  if (!psaCC_ctbIsValid (idx))
  {
    TRACE_ERROR ("Try to clear an invalid entry!");
    return;
  }

  /* 
   * Probably it is important to use this pointer variable here!
   */
  ctb = ccShrdPrm.ctb[idx];

  /*
   * Save the last cause, if any, for later query with qAT_PlusCEER()
   */
  if (GET_CAUSE_VALUE(ctb->rejCs) NEQ NOT_PRESENT_8BIT)
  {
    ccShrdPrm.ccCs[idx] = ctb->rejCs;
  }
  else if (GET_CAUSE_VALUE(ctb->nrmCs) NEQ NOT_PRESENT_8BIT)
  {
    ccShrdPrm.ccCs[idx] = ctb->nrmCs;
  }
  else if (GET_CAUSE_VALUE(ctb->rslt) NEQ NOT_PRESENT_8BIT)
  {
    ccShrdPrm.ccCs[idx] = ctb->rslt;
  }
  else
  {
    /* in case network has sent no extended report */
    ccShrdPrm.ccCs[idx] = CAUSE_MAKE (DEFBY_STD, 
                                      ORIGSIDE_MS, 
                                      MNCC_ACI_ORIGINATING_ENTITY, 
                                      NOT_PRESENT_8BIT);
  }

  /*
   * Save the call type. Fax/Data needs this information.
   */
  ccShrdPrm.callType[idx] = (T_CC_CALL_TYPE)cmhCC_getcalltype(idx);

  /* Deleting linked list of Facility primitives */
  if ((listcounter = get_list_count(ccShrdPrm.facility_list)) > 0)
  {
    for (;listcounter > 0 ; listcounter --)
    {
      msg2send = get_next_element( ccShrdPrm.facility_list, NULL);
      remove_first_element(ccShrdPrm.facility_list);
      PFREE(msg2send);
    }
  }
  ccShrdPrm.facility_list= NULL;
  ccShrdPrm.aocRsmpPend  = 0;

  if (ctb->cldPty.called_num NEQ NULL)
  {
    ACI_MFREE (ctb->cldPty.called_num);
    ctb->cldPty.called_num = NULL;
  }

  psaCC_FreeRdrPty (idx);

  ACI_MFREE (ccShrdPrm.ctb[idx]);
  ccShrdPrm.ctb[idx] = NULL;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH                     |
|                                 ROUTINE : psaCC_ctbIsValid        |
+-------------------------------------------------------------------+

  PURPOSE : This function returns whether a given call table entry
            is valid (means non-NULL).
            Caution with TRACE_EVENTs etc., it could be used in 
            a signal handler.

*/

GLOBAL BOOL psaCC_ctbIsValid (SHORT cId)
{
  return ((cId >= 0) AND
          (cId < MAX_CALL_NR) AND
          (ccShrdPrm.ctb[cId] NEQ NULL));
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH                     |
|                                 ROUTINE : psaCC_ctb               |
+-------------------------------------------------------------------+

  PURPOSE : This function returns a pointer to the call table
            described by the cId. It is assumed that cId is valid,
            no return of NULL pointer expected here.
            What's the sense of this function? There were problems 
            encountered with the target compiler having a too
            complicated pointer expression.

*/

GLOBAL T_CC_CALL_TBL * psaCC_ctb (SHORT cId)
{
  return (ccShrdPrm.ctb[cId]);
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CC                  |
|                                 ROUTINE : psaCC_Init              |
+-------------------------------------------------------------------+

  PURPOSE : initialize the protocol stack adapter for CC.

*/

/* initialize CC parametrs for MTC bearer capabilities */
GLOBAL void psaCC_init_mtcbearer ( void )
{
  /* wasn't really used apparently
  ccShrdPrm.mtcBC.rate         = DEF_BC1_UR;
  ccShrdPrm.mtcBC.bearer_serv  = DEF_BC1_BS;
  ccShrdPrm.mtcBC.conn_elem    = DEF_BC1_CE;
  ccShrdPrm.mtcBC.stop_bits    = DEF_BC1_SB;
  ccShrdPrm.mtcBC.data_bits    = DEF_BC1_DB;
  ccShrdPrm.mtcBC.parity       = DEF_BC1_PR;
  ccShrdPrm.mtcBC.flow_control = DEF_BC1_FC;
  ccShrdPrm.mtcBC.modem_type   = DEF_BC1_MT;*/

  /* initialize CC for MTC calls */
#ifdef FAX_AND_DATA
  ccShrdPrm.CBSTspeed          = BS_SPEED_9600_V32;
  ccShrdPrm.CBSTname           = CBST_NAM_Asynch;
  ccShrdPrm.CBSTce             = CBST_CE_NonTransparent;
#endif /* FAX_AND_DATA */
  ccShrdPrm.snsMode            = MNCC_SNS_MODE_VOICE;
#ifdef FF_TTY
  ccShrdPrm.ctmReq             = MNCC_CTM_DISABLED;
  ccShrdPrm.ctmState           = TTY_STATE_NONE;
  ccShrdPrm.ttyCmd             = (UBYTE)TTY_OFF;
  ccShrdPrm.ctmOvwr            = FALSE;
#endif /* FF_TTY */
  psaCC_Config( );
}

GLOBAL void psaCC_Init ( void )
{
  UBYTE ctbIdx;            /* holds index to call table */

  /* initialize call table */
  for( ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
  {
    ccShrdPrm.ctb[ctbIdx] = NULL;
  }

  /* initialize shared parameter */
  ccShrdPrm.cIdFail            = -1;
  ccShrdPrm.nrOfMOC            = 0;
  ccShrdPrm.nrOfMTC            = 0;
  ccShrdPrm.chMod              = NOT_PRESENT_8BIT;
  ccShrdPrm.chType             = NOT_PRESENT_8BIT;
  ccShrdPrm.syncCs             = CAUSE_MAKE(DEFBY_CONDAT, ORIGSIDE_MS, MNCC_CC_ORIGINATING_ENTITY, NOT_PRESENT_8BIT);
  ccShrdPrm.TCHasg             = FALSE;
  ccShrdPrm.datStat            = DS_IDL;
  ccShrdPrm.CMODmode           = CMOD_MOD_Single;
  ccShrdPrm.msgType            = NO_VLD_MT;
  ccShrdPrm.dtmf.cId           = NO_ENTRY;
  ccShrdPrm.dtmf.cnt           = 0;
  ccShrdPrm.dtmf.cur           = 0;
  ccShrdPrm.facility_list      = NULL;
  ccShrdPrm.cvhu               = CVHU_DropDTR_IGNORED;
#if defined (FF_WAP) || defined (FF_SAT_E)
  ccShrdPrm.wapStat            = CC_WAP_STACK_DOWN;
#endif
  cuscfgParams.MO_SM_Control_SIM = CUSCFG_STAT_Disabled;
  cuscfgParams.MO_Call_Control_SIM = CUSCFG_STAT_Disabled;
  cuscfgParams.MO_SS_Control_SIM = CUSCFG_STAT_Disabled;
  cuscfgParams.MO_USSD_Control_SIM = CUSCFG_STAT_Disabled;
  cuscfgParams.Two_digit_MO_Call = CUSCFG_STAT_Disabled;
  cuscfgParams.Ext_USSD_Response = CUSCFG_STAT_Disabled;
  cuscfgParams.USSD_As_MO_Call = CUSCFG_STAT_Disabled;

  Ext_USSD_Res_Pending_sId = NOT_PRESENT_8BIT;

  psaCC_init_mtcbearer( );
#ifdef TI_PS_FF_AT_CMD_P_ECC
  /* Initialize additional ECC numbers */
  cmhCC_additional_ecc_numbers_initialize();
#endif /* TI_PS_FF_AT_CMD_P_ECC */
}

/*
+-------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCP                       |
|                                 ROUTINE : psaCC_ProcessCmp              |
+-------------------------------------------------------------------------+

  PURPOSE : processes the each component sent by the function 
            psa_mncc_facility_ind().

*/


GLOBAL void psaCC_ProcessCmp ( T_MNCC_FACILITY_IND *mncc_facility_ind )
{
  SHORT cId;                 /* holds call id */
  BOOL  facility_stored = FALSE;
  
  TRACE_FUNCTION ("psaCC_ProcessCmp()");

/*
 *-------------------------------------------------------------------
 * find call in call table
 *-------------------------------------------------------------------
 */
  cId = psaCC_ctbFindTi( mncc_facility_ind -> ti );

  if( cId < 0 )
  {
   /*
    * ignore primitive, due to not found transaction identifier.
    */
    TRACE_EVENT ("primitive rejected due to unused ti");
    PFREE(mncc_facility_ind);
    return;
  }

  psaCC_DumpFIE (&mncc_facility_ind -> fac_inf );
  cmhCC_sendFie( CSCN_FACILITY_DIRECTION_IN,
                  cId,
                  &mncc_facility_ind -> fac_inf );

  /* decode component type */
  CCD_START;
  {
    UBYTE ccdRet;

    MCAST( com, COMPONENT );
    memset( com, 0, sizeof( T_COMPONENT ));

    ccdRet = ccd_decodeMsg (CCDENT_FAC,
                            DOWNLINK,
                            (T_MSGBUF *) &mncc_facility_ind -> fac_inf,
                            (UBYTE    *) _decodedMsg,
                            COMPONENT);

    if( ccdRet NEQ ccdOK )
    {
      TRACE_EVENT_P1("CCD Decoding Error: %d",ccdRet );
      psaCC_ctb(cId)->failType = SSF_CCD_DEC;
      cmhCC_SSTransFail(cId);
    }
    else
    {
      /* Buffer MNCC_FACILITY_IND primitive 
          if the Call direction is MTC 
          and no TCHasg Flag 
          and RING is not sent yet 
          and no signal information

      */
      if (psaCC_ctb(cId)->calType EQ  CT_MTC AND !ccShrdPrm.TCHasg AND 
        psaCC_ctb(cId)->calStat EQ CS_ACT_REQ AND psaCC_ctb(cId)->sigInf EQ MNCC_SIG_NOT_PRES) 
      {
        if ( ccShrdPrm.facility_list EQ NULL)
        {
          ccShrdPrm.facility_list = new_list ();
          insert_list(ccShrdPrm.facility_list, mncc_facility_ind);
        }
        else
        {
          insert_list(ccShrdPrm.facility_list, mncc_facility_ind);
        }
        facility_stored = TRUE;
      }
      else
      {
         if( com->v_inv_comp )
        {
          psaCC_dasmInvokeCmp( cId, &com->inv_comp );
        }
        else if( com->v_res_comp )
        {
          psaCC_dasmResultCmp( cId, &com->res_comp );
        }
        else if( com->v_err_comp )
        {
#ifdef SIM_TOOLKIT
          if( psaCC_ctb(cId)->SATinv )
          {
            psaCC_ctb(cId)->SATinv = FALSE;
            psaSAT_SSErrComp((T_fac_inf *) &mncc_facility_ind -> fac_inf, FALSE );
          }
#endif
          psaCC_dasmErrorCmp( cId, &com->err_comp );
        }
        else if( com->v_rej_comp )
        {
#ifdef SIM_TOOLKIT
          if( psaCC_ctb(cId)->SATinv )
          {
            psaCC_ctb(cId)->SATinv = FALSE;
            psaSAT_SSRejComp( RSLT_NTW_UNAB_PROC );
          }
#endif
          psaCC_dasmRejectCmp( cId, &com->rej_comp );
        }
        else
        {
          TRACE_EVENT( "WRONG COMPONENT TYPE RECEIVED" );
          cmhCC_SSTransFail (cId);
        }
      }
    }
  }
  CCD_END;

/*
 *-------------------------------------------------------------------
 * free the primitive buffer if not stored
 *-------------------------------------------------------------------
 */
  if (!facility_stored)
  {
    PFREE (mncc_facility_ind);
  }

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CC                  |
|                                 ROUTINE : psaCC_DumpFIE           |
+-------------------------------------------------------------------+

  PURPOSE : Dump facility information element to debug output.

*/

GLOBAL void psaCC_DumpFIE ( T_MNCC_fac_inf * fie )
{
  CHAR strBuf[40+1];           /* holds string buffer */
  UBYTE idx, cnt,mcnt;         /* buffer index */
  CHAR *pDest;                 /* points to destination */

  TRACE_EVENT( "FIE SENT/RECEIVED:" );

  mcnt = fie->l_fac >> 3;
  /* G23 uses ever offset 0 !!! */

  /* format FIE */
  for( cnt = 0; cnt < mcnt; cnt+=idx )
  {
    pDest = strBuf;

    for( idx = 0; idx < 20 AND idx+cnt <  mcnt; idx++ )
    {
/* Implements Measure#32: Row 1245 */
      pDest += sprintf( pDest, format_2X_str, (UBYTE)fie->fac[idx+cnt] );
    }

    *pDest = 0x0;

    TRACE_EVENT_P1("%s", strBuf );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_ctbDump           |
+-------------------------------------------------------------------+

  PURPOSE : this function dumps the call table to the debug output.
*/

#ifdef TRACING
// What you are searching for has been removed by Conquest issue 
// ACI-ENH-30932 for the Locosto project in the assumption noone
// is using it anymore. If I was wrong get the function back from 
// the version control system and let me know.
// HM, 4-May-2005
#endif  /* of #ifdef TRACING */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_ctbDumpBC         |
+-------------------------------------------------------------------+

  PURPOSE : this function dumps the BC to the debug output.
*/

#ifdef TRACING
// What you are searching for has been removed by Conquest issue 
// ACI-ENH-30932 for the Locosto project in the assumption noone
// is using it anymore. If I was wrong get the function back from 
// the version control system and let me know.
// HM, 4-May-2005
#endif  /* of #ifdef TRACING */

/* Implements Measure# 188 & 189*/
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCF                 |
|                                 ROUTINE : psaCC_ctbClrAdr2Num     |
+-------------------------------------------------------------------+

  PURPOSE : this function converts the parameters of the calling
            address for the passed call id into a dial number. The
            string is copied into the passed buffer. In case the
            buffer size is not sufficient to hold the called number,
            the number is cut to maxSize. The function returns a pointer
            to the buffer or NULL if an error occurs.
*/

LOCAL CHAR *psaCC_ctbAdr2Num ( CHAR *pNumBuf, UBYTE maxSize, UBYTE *num, 
                               UBYTE c_num,   UBYTE ton)
{
  TRACE_FUNCTION ("psaCC_ctbAdr2Num()");
/*
 *-------------------------------------------------------------------
 * convert BCD address
 *-------------------------------------------------------------------
 */
  if (c_num EQ 0)
  {
    *pNumBuf = '\0';     /* empty string */
    return( NULL );
  }

  maxSize -= 1; /* for trailing '\0' */

  /*
   *  International call add + at the beginning
   */
  if (ton EQ MNCC_TON_INT_NUMB)
  {
    *pNumBuf = '+';
    utl_BCD2DialStr (num, pNumBuf+1,
                     (UBYTE)MINIMUM(maxSize-1, c_num));
  }
  else
  {
    utl_BCD2DialStr (num, pNumBuf,
                     (UBYTE)MINIMUM(maxSize, c_num));
  }
  return( pNumBuf );
}

/*==== EOF ========================================================*/

