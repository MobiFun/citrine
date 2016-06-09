/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_TSTS
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
|  Purpose :  This module provides test functions for debugging
|             puposes.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_TSTS_C
#define CMH_TSTS_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci.h"
#include "aci_cmh.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif              /* of #ifdef FAX_AND_DATA */

#ifdef UART
#include "dti.h"
#include "dti_conn_mng.h"
#endif

#include "psa.h"
#include "psa_cc.h"
#include "psa_mm.h"
#include "psa_sim.h"
#include "cmh.h"
#include "cmh_cc.h"
#include "cmh_mm.h"
#include "cmh_sim.h"

#ifdef FAX_AND_DATA

#ifdef FF_FAX
#include "psa_t30.h"
#include "cmh_t30.h"
#endif

#include "psa_ra.h"
#include "cmh_ra.h"
#include "cmh_fsim.h"   /* remove after testing */

#endif /* FAX_AND_DATA */

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/
/*==== FUNCTIONS ==================================================*/

#ifdef FAX_AND_DATA

#ifdef _SIMULATE_FAX_DATA_
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_TSTS                |
|                                 ROUTINE : TST_rstSimData          |
+-------------------------------------------------------------------+

  PURPOSE : reset simulated FAX data

*/

GLOBAL void TST_rstSimData( void )
{

  TRACE_FUNCTION ("TST_rstSimData()");

  pSimBuf  = simDataBuf;    /* remove after testing ??? */
  sentData = 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_TSTS                |
|                                 ROUTINE : TST_getSimData          |
+-------------------------------------------------------------------+

  PURPOSE : get simulated FAX data

*/

GLOBAL void TST_getSimData(UBYTE *data, USHORT nomLen,
                           USHORT *actLen, UBYTE *final)
{
  SHORT seq;

  TRACE_FUNCTION ("TST_getSimData()");

  *final  = FALSE;
  *actLen = (USHORT)(((SIM_DATA_LEN - sentData)>nomLen)?
                                    nomLen:(SIM_DATA_LEN - sentData));

  for( seq = *actLen/50; seq > 0; seq-- )
  {
    memcpy( data, pSimBuf, 50 );
    pSimBuf += 50;
    data    += 50;
/*    vsi_t_sleep( VSI_CALLER 0 ); */
  }

  if( (*actLen)%50 )
  {
    memcpy( data, pSimBuf, *actLen%50 );
    pSimBuf += 50;
  }

  sentData += *actLen;

/*sprintf( buf, "DATA COPIED: %d DATA SENT: %d", *actLen, sentData );
  TRACE_EVENT( "DATA COPIED" ); */

  if( sentData >= SIM_DATA_LEN )
  {
    *final = TRUE;
  }
}

#endif /*_SIMULATE_FAX_DATA_*/

#endif

/*==== EOF ========================================================*/
