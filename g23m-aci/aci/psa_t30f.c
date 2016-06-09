/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_T30F
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
|             stack adapter for T30.
+----------------------------------------------------------------------------- 
*/ 
#ifdef FF_FAX

#ifndef PSA_T30F_C
#define PSA_T30F_C
#endif

#include "aci_all.h"

#undef TRACING

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "dti.h"
#include "dti_conn_mng.h"

#include "aci_fd.h"
#include "aci.h"
#include "psa.h"
#include "psa_t30.h"
#include "cmh.h"
#include "cmh_t30.h"
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
| PROJECT : GSM-PS (6147)         MODULE  : PSA_T30                 |
|                                 ROUTINE : psaT30_Init             |
+-------------------------------------------------------------------+

  PURPOSE : initialize the protocol stack adapter for T30.

*/

GLOBAL void psaT30_Init ( void )
{
  memset (&t30ShrdPrm, 0, sizeof (T_T30_SHRD_PRM));
  t30ShrdPrm.faxStat = NO_VLD_FS;
  t30ShrdPrm.T30_is_activated = FALSE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_RAF                 |
|                                 ROUTINE : psaT30_shrPrmDump      |
+-------------------------------------------------------------------+

  PURPOSE : this function dumps the shared parameter to the debug
            output.
*/

#ifdef TRACING
GLOBAL void psaT30_shrPrmDump ( void )
{
  char  lnBuf[80];              /* holds buffer for output line */ 
  SHORT chrNr;                  /* holds number of processed chars */

  /* --- call id -----------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "   call id" );

  chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT, t30ShrdPrm.cId );
  TRACE_EVENT( lnBuf );

  /* --- FAX status --------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "FAX status" );

  chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT, t30ShrdPrm.faxStat );
  TRACE_EVENT( lnBuf );

  /* --- transmission rate -------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "trans rate" );

  chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT, t30ShrdPrm.trans_rate );
  TRACE_EVENT( lnBuf );

  /* --- half rate ---------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, " half rate" );

  chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT, t30ShrdPrm.half_rate );
  TRACE_EVENT( lnBuf );

  /* --- threshold ---------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, " threshold" );

  chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT, t30ShrdPrm.threshold );
  TRACE_EVENT( lnBuf );

  /* --- frames per prim ---------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "frame/prim" );

  chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT, t30ShrdPrm.frames_per_prim );
  TRACE_EVENT( lnBuf );

  /* --- hdlc report -------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "  HDLC rpt" );

  chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT, t30ShrdPrm.hdlc_report );
  TRACE_EVENT( lnBuf );

  /* --- final --------------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "final flag" );

  chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT, t30ShrdPrm.final );
  TRACE_EVENT( lnBuf );

  /* --- signal receive -----------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "signal rcv" );

  chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT, t30ShrdPrm.sgn_rcv );
  TRACE_EVENT( lnBuf );

  /* --- signal send --------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "signal snd" );

  chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT, t30ShrdPrm.sgn_snd );
  TRACE_EVENT( lnBuf );

  /* --- complete -----------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "  complete" );

  chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT, t30ShrdPrm.cmpl );
  TRACE_EVENT( lnBuf );

  /* --- error cause --------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, " err cause" );

  chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT, t30ShrdPrm.err_cause );
  TRACE_EVENT( lnBuf );

  /* --- eol ----------------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "       eol" );

  chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT, t30ShrdPrm.eol );
  TRACE_EVENT( lnBuf );
}
#endif /* of #ifdef TRACING */
#endif /* FF_FAX */
/*==== EOF ========================================================*/

