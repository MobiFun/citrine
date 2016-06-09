/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_RAF
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
|             stack adapter for Rate Adaptation.
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_RAF_C
#define PSA_RAF_C
#endif

#include "aci_all.h"

#undef TRACING

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#ifdef FAX_AND_DATA
  #include "aci_fd.h"
#endif

#include "aci.h"
#include "psa.h"
#include "aci_io.h"
#include "psa_ra.h"
#include "cmh.h"
#include "cmh_ra.h"
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
| PROJECT : GSM-PS (6147)         MODULE  : PSA_RA                  |
|                                 ROUTINE : psaRA_Init              |
+-------------------------------------------------------------------+

  PURPOSE : initialize the protocol stack adapter for RA.

*/

GLOBAL void psaRA_Init ( void )
{
  extern void dll_init(void);

  memset (&raShrdPrm, 0, sizeof (T_RA_SHRD_PRM));

  dll_init ();
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_RAF                 |
|                                 ROUTINE : psaRA_shrPrmDump        |
+-------------------------------------------------------------------+

  PURPOSE : this function dumps the shared parameter to the debug
            output.
*/

#ifdef TRACING
GLOBAL void psaRA_shrPrmDump ( void )
{
  char  lnBuf[80];              /* holds buffer for output line */ 
  SHORT chrNr;                  /* holds number of processed chars */
  SHORT itmIdx;

  /* --- mode --------------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "     model" );
  for( itmIdx = 0; (itmIdx<2); itmIdx++ )
  {
    chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT, 
                      raShrdPrm.set_prm[itmIdx].model );
  }
  TRACE_EVENT( lnBuf );

  /* --- transmission rate -------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "trans rate" );
  for( itmIdx = 0; (itmIdx<2); itmIdx++ )
  {
    chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT, 
                      raShrdPrm.set_prm[itmIdx].tra_rate );
  }
  TRACE_EVENT( lnBuf );

  /* --- user rate ---------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, " user rate" );
  for( itmIdx = 0; (itmIdx<2); itmIdx++ )
  {
    chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT, 
                      raShrdPrm.set_prm[itmIdx].user_rate );
  }
  TRACE_EVENT( lnBuf );

  /* --- no. data bits -----------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, " data bits" );
  for( itmIdx = 0; (itmIdx<2); itmIdx++ )
  {
    chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT, 
                      raShrdPrm.set_prm[itmIdx].ndb );
  }
  TRACE_EVENT( lnBuf );

  /* --- no. stop bits -----------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, " stop bits" );
  for( itmIdx = 0; (itmIdx<2); itmIdx++ )
  {
    chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT, 
                      raShrdPrm.set_prm[itmIdx].nsb );
  }
  TRACE_EVENT( lnBuf );

  /* --- owner -------------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "     owner" );

  chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT, raShrdPrm.owner );
  TRACE_EVENT( lnBuf );

  /* --- call id -----------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "   call id" );

  chrNr += sprintf( lnBuf+chrNr, "%*d", ITM_WDT, raShrdPrm.cId );
  TRACE_EVENT( lnBuf );
}
#endif  /* of #ifdef TRACING */

/*==== EOF ========================================================*/

