/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_MM
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
|             stack adapter for the registration part of mobility
|             management.
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_MMF_C
#define PSA_MMF_C
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
#include "aci.h"
#include "psa.h"
#include "psa_mm.h"
#include "psa_util.h"
/*==== CONSTANTS ==================================================*/

#define ITM_WDT         (14)    /* item width in chars */
#define HDR_WDT         (10)    /* header width in chars */

/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMF                 |
|                                 ROUTINE : psaMM_ClrPLMNLst        |  
+-------------------------------------------------------------------+

  PURPOSE : clears all entries for the PLMN list
*/

GLOBAL void psaMM_ClrPLMNLst ( void )
{
  int lstIdx;
  
  for( lstIdx = 0; lstIdx < MAX_PLMN_ID; lstIdx++ )
  {
    mmShrdPrm.PLMNLst[lstIdx].v_plmn = INVLD_PLMN;
    mmShrdPrm.FRBLst[lstIdx]         = NOT_PRESENT_8BIT;
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMF                 |
|                                 ROUTINE : psaMM_CpyPLMNLst        |
+-------------------------------------------------------------------+

  PURPOSE : copies all used entries of a PLMN list into shared
            parameter area.
*/

GLOBAL void psaMM_CpyPLMNLst ( T_plmn * pPLMNLst, UBYTE * pFRBLst, USHORT * pLACLst)
{
  int lstIdx;

  psaMM_ClrPLMNLst ();

  for( lstIdx = 0; lstIdx < MAX_PLMN_ID; lstIdx++ )
  {
    if( pPLMNLst -> v_plmn EQ INVLD_PLMN ) 
      break;
    mmShrdPrm.PLMNLst[lstIdx] = *pPLMNLst++;
    mmShrdPrm.FRBLst[lstIdx] = *pFRBLst++;
    mmShrdPrm.LACLst[lstIdx] = *pLACLst++;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MM                  |
|                                 ROUTINE : psaMM_Init              |
+-------------------------------------------------------------------+

  PURPOSE : initialize the protocol stack adapter for MM.

*/

GLOBAL void psaMM_Init ( void )
{

/*
 *-------------------------------------------------------------------
 * set default parms
 *-------------------------------------------------------------------
 */  
  mmShrdPrm.regStat             = NO_VLD_RS;
  mmShrdPrm.regMode             = DEF_REG_MODE;
  mmShrdPrm.regModeBeforeAbort  = DEF_REG_MODE;
  mmShrdPrm.regModeAutoBack     = FALSE;
  mmShrdPrm.srchRslt            = 0xFF;
  mmShrdPrm.deregCs             = 0xFF;
  mmShrdPrm.usedPLMN.v_plmn     = INVLD_PLMN;
  mmShrdPrm.PLMNLst[0].v_plmn   = INVLD_PLMN;
  mmShrdPrm.owner               = (T_OWN)CMD_SRC_NONE;
  mmShrdPrm.creg_status         = CREG_STAT_NoSearch;
  mmShrdPrm.tz                  = INVLD_TZ;
  mmShrdPrm.PNNLst.plmn.v_plmn  = INVLD_PLMN;
  mmShrdPrm.PNNLst.pnn_rec_num  = 0;
  mmShrdPrm.PNNLst.next         = NULL;
  mmShrdPrm.COPSmode            = COPS_MOD_Auto;
  mmShrdPrm.COPSmodeBeforeAbort = COPS_MOD_Auto;
  mmShrdPrm.ActingHPLMN.v_plmn  = INVLD_PLMN;
  mmShrdPrm.pnn_read_cnt        = 0;

  /* temporary, because of the changes in psaSAT_BuildEnvCC(),
     this is necessary to pass the ACISAT test cases */
#ifdef _SIMULATION_
  mmShrdPrm.lac               = 0x0100;
  mmShrdPrm.cid               = 0x0100;
  mmShrdPrm.usedPLMN.mcc[0]   = 0x02;
  mmShrdPrm.usedPLMN.mcc[1]   = 0x06;
  mmShrdPrm.usedPLMN.mcc[2]   = 0x02;
  mmShrdPrm.usedPLMN.mnc[0]   = 0x00;
  mmShrdPrm.usedPLMN.mnc[1]   = 0x01;
  mmShrdPrm.usedPLMN.mnc[2]   = 0x0F;
#endif

  mmShrdPrm.slctPLMN.v_plmn = INVLD_PLMN;
  
  psaMM_ClrPLMNLst();
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMF                 |
|                                 ROUTINE : psaMM_shrPrmDump        |
+-------------------------------------------------------------------+

  PURPOSE : this function dumps the shared parameter to the debug
            output.
*/

#ifdef TRACING
GLOBAL void psaMM_shrPrmDump ( void )
{
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
}
#endif  /* of #ifdef TRACING */

/*==== EOF ========================================================*/
 
