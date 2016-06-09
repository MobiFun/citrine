/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  
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
|             stack adapter for the registration part of session
|             management ( SM ).
+----------------------------------------------------------------------------- 
*/ 

#if defined (GPRS) && defined (DTI)

#ifndef PSA_SMF_C
#define PSA_SMF_C
#endif

#include "aci_all.h"

#undef TRACING
/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci.h"

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "gaci.h"
#include "psa.h"
#include "gaci_cmh.h"
#include "psa_sm.h"
#include "cmh.h"
#include "cmh_sm.h"


/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/


/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_GMMF                |
|                                 ROUTINE : psaGMM_Init             |
+-------------------------------------------------------------------+

  PURPOSE : initialize the protocol stack adapter for GMM.

*/


GLOBAL void psaSM_Init ( void )
{
/*  UBYTE LpCnt;*/            /* holds loop counter for macro */

/*
 *-------------------------------------------------------------------
 * set default parms
 *-------------------------------------------------------------------
 */  
  smShrdPrm.owner        = (UBYTE) CMD_SRC_NONE;
}

#if 0
GLOBAL void psaSNDCP_Dti_Req( T_DTI_CONN_LINK_ID  link_id, UBYTE peer )
{
  TRACE_FUNCTION("psaSNDCP_Dti_Req()");

  switch ( peer )
  {
    case DTI_ENTITY_PPPS:
      /* do nothing, because this will be implicit doing psaGPPPS_Dti_Req( link_id, UNIT_UART ) */
      break;
#ifdef FF_TCP_IP
    case DTI_ENTITY_AAA:
#endif /* FF_TCP_IP */
#ifdef FF_GPF_TCPIP
  case DTI_ENTITY_TCPIP:
#endif
    case DTI_ENTITY_NULL:
    case DTI_ENTITY_IP:
#ifdef FF_SAT_E      
    case DTI_ENTITY_SIM:
#endif /* FF_SAT_E */       
    case DTI_ENTITY_PKTIO:
    case DTI_ENTITY_PSI:
      pdp_context[work_cids[cid_pointer] - 1].link_id_new = link_id;
      {
        UBYTE ppp_hc = 0;
        UBYTE msid = 0;
        cmhSM_connect_context(gaci_get_cid_over_link_id( link_id ), 
                              (T_DTI_ENTITY_ID)peer, ppp_hc, msid);
      }
    break;
  }
}


#if 0
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SMF                 |
|                                 ROUTINE : psaSM_shrPrmDump        |
+-------------------------------------------------------------------+

  PURPOSE : this function dumps the shared parameter to the debug
            output.
*/

GLOBAL void psaSM_shrPrmDump ( void )
{
#ifdef TRACING

  char  lnBuf [80];             /* holds buffer for output line */ 
  char  mccBuf[SIZE_MCC + 1];   /* MCC converted to printable C-string */
  char  mncBuf[SIZE_MNC + 1];   /* MNC converted to printable C-string */
  SHORT chrNr;                  /* holds number of processed chars */
  SHORT cnt;                    /* holds a counter */

  /* --- PLMN list ------------------------------------------------*/
  for( cnt = 0; cnt<MAX_PLMN_ID AND 
                mmShrdPrm.PLMNLst[cnt].v_plmn NEQ INVLD_PLMN; cnt++ )
  {
    chrNr  = sprintf( lnBuf, "%*.*s[%2d]", HDR_WDT, HDR_WDT, " PLMN list",cnt );
    utl_BCD2String (mccBuf, mmShrdPrm.PLMNLst[cnt].mcc, SIZE_MCC);
    utl_BCD2String (mncBuf, mmShrdPrm.PLMNLst[cnt].mnc, SIZE_MNC);
    chrNr += sprintf( lnBuf+chrNr, "%*s %*s", 
                      ITM_WDT/2, ITM_WDT/2, mccBuf, mncBuf);
    TRACE_EVENT( lnBuf );
  }

  /* --- used PLMN ------------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, " used PLMN" );
  if( mmShrdPrm.usedPLMN.v_plmn EQ VLD_PLMN )
  {
    utl_BCD2String (mccBuf, mmShrdPrm.usedPLMN.mcc, SIZE_MCC);
    utl_BCD2String (mncBuf, mmShrdPrm.usedPLMN.mnc, SIZE_MNC);
    chrNr += sprintf( lnBuf+chrNr, "%*s %*s", 
                      ITM_WDT/2, ITM_WDT/2, mccBuf, mncBuf);
  }
  else
  {
    chrNr += sprintf( lnBuf+chrNr, "%*s", ITM_WDT, "none" );
  }
  TRACE_EVENT( lnBuf );

  /* --- registration mode ----------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "rgstr mode" );
  chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT, 
                                         mmShrdPrm.setPrm[0].regMode );
  TRACE_EVENT( lnBuf );

  /* --- registration status --------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "rgstr stat" );
  chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT,
                                         mmShrdPrm.regStat );
  TRACE_EVENT( lnBuf );

  /* --- search result --------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, " srch rslt" );
  chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT,
                                         mmShrdPrm.srchRslt );
  TRACE_EVENT( lnBuf );

  /* --- de-registration cause ------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "dereg caus" );
  chrNr += sprintf( lnBuf+chrNr, "%*X", ITM_WDT,
                                        mmShrdPrm.deregCs );
  TRACE_EVENT( lnBuf );

#endif  /* of #ifdef TRACING */
}
#endif /* #if 0 */
#endif /* #if 0 */
#endif  /* GPRS */
/*==== EOF ========================================================*/
 
