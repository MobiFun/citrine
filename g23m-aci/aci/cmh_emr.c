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
|  Purpose :  This module provides the response functions related to the 
|             SAP for engineering mode.
+----------------------------------------------------------------------------- 
*/ 

#ifndef ACI_EM_C
#define ACI_EM_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

/*===== INCLUDES ===================================================*/
#include "aci_cmh.h"

#include "psa.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif  

#ifdef GPRS
#include "gprs.h"
#endif 

#if (defined(FAX_AND_DATA) || defined(UART) || defined(GPRS))
#include "dti.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"
#endif

#include "cmh.h"
#include "cmh_snd.h"
#include "aci_em.h"
#include "cmh_em.h"


/*==== EXPORT =====================================================*/
/*==== VARIABLES ==================================================*/

EXTERN T_EM_SHRD_PRM emPrm;
EXTERN T_EM_SHRD_PRM emetPrm;
/* unused: EXTERN T_EM_SHRD_PRM emetsPrm; */

/*==== FUNCTIONS ==================================================*/
/*==== CONSTANTS ==================================================*/

GLOBAL void cmhEM( T_EM_VAL * val_tmp )  
{
  TRACE_FUNCTION ("cmhEM()");

  R_AT( RAT_EM,(T_ACI_CMD_SRC) emPrm.srcId ) ( val_tmp );

  R_AT( RAT_OK, (T_ACI_CMD_SRC) emPrm.srcId ) ( AT_CMD_EM );
}

GLOBAL void cmhEMET ( T_DRV_SIGNAL_EM_EVENT * Signal )  
{
  TRACE_FUNCTION ("cmhEMET()");

  R_AT( RAT_EMET, (T_ACI_CMD_SRC) emetPrm.srcId ) ( Signal );

/*  R_AT( RAT_OK,   emetPrm.srcId ) ( AT_CMD_EMET );  */
}

/*
GLOBAL void cmhEMETS ( UBYTE entity )  
{
  TRACE_FUNCTION ("cmhEMETS()");

  R_AT( RAT_OK,   emetsPrm.srcId ) ( AT_CMD_EMETS );
}
*/


/*+++++++++++++++++++++++++++++++++++++++++ E O F +++++++++++++++++++++++++++++++++++++++++*/

