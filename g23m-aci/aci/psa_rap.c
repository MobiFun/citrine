/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_RAP
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
|  Purpose :  This module defines the processing functions for the
|             primitives send to the protocol stack adapter by rate
|             adaptation.
+----------------------------------------------------------------------------- 
*/ 
#ifdef DTI

#ifndef PSA_RAP_C
#define PSA_RAP_C
#endif

#include "aci_all.h"

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
#include "psa_mmi.h"
#include "cmh.h"
#include "cmh_ra.h"

#if defined (FF_WAP) || defined (FF_SAT_E)
#include "wap_aci.h"
#endif

#ifdef FF_PPP
#include "dti.h"      /* functionality of the dti library */
#include "dti_conn_mng.h"
#include "psa_ppp_w.h"
#endif

#include "psa_cc.h"
#include "cmh_cc.h"

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_RA                  |
|                                 ROUTINE : psa_ra_activate_cnf     |
+-------------------------------------------------------------------+

  PURPOSE : processes the RA_ACTIVATE_CNF primitive send by RA.

*/

GLOBAL  void psa_ra_activate_cnf
                                 (T_RA_ACTIVATE_CNF *ra_activate_cnf)
{
  TRACE_EVENT ("psa_ra_activate_cnf()");

  if ((raShrdPrm.cId EQ NO_ENTRY) OR (ccShrdPrm.ctb[raShrdPrm.cId] EQ NULL))
  {
    TRACE_ERROR ("raShrdPrm.cId invalid");
#ifndef USE_L1FD_FUNC_INTERFACE
    PFREE (ra_activate_cnf);
#endif
    return;
  }

  if (ra_activate_cnf->ack_flg NEQ RA_NAK)
  {
    cmhRA_Activated( raShrdPrm.cId );
  }

#ifndef USE_L1FD_FUNC_INTERFACE
  PFREE (ra_activate_cnf);
#endif
}

#ifdef FF_FAX
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_RA                  |
|                                 ROUTINE : psa_ra_modify_cnf       |
+-------------------------------------------------------------------+

  PURPOSE : processes the RA_MODIFY_CNF primitive send by RA.

*/

GLOBAL  void psa_ra_modify_cnf
                             (T_RA_MODIFY_CNF *ra_modify_cnf)
{
  TRACE_EVENT ("psa_ra_modify_cnf()");

  cmhCC_RA_Modified(raShrdPrm.cId);

#ifndef USE_L1FD_FUNC_INTERFACE
  PFREE (ra_modify_cnf);
#endif
}
#endif /* FF_FAX */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_RA                  |
|                                 ROUTINE : psa_ra_deactivate_cnf   |
+-------------------------------------------------------------------+

  PURPOSE : processes the RA_DEACTIVATE_CNF primitive send by RA.

*/

GLOBAL  void psa_ra_deactivate_cnf
                             (T_RA_DEACTIVATE_CNF *ra_deactivate_cnf)
{
  TRACE_EVENT ("psa_ra_deactivate_cnf()");

  if ((raShrdPrm.cId EQ NO_ENTRY) OR (ccShrdPrm.ctb[raShrdPrm.cId] EQ NULL))
  {
    TRACE_ERROR ("raShrdPrm.cId invalid");
#ifndef USE_L1FD_FUNC_INTERFACE
    PFREE (ra_deactivate_cnf);
#endif
    return;
  }

  psaCC_ctb(raShrdPrm.cId)->curCs = MNCC_CAUSE_NO_MS_CAUSE;

  cmhRA_Deactivated( raShrdPrm.cId );

#if defined (FF_WAP) || defined (FF_SAT_E)
  if(Wap_Call)
  {
    /*
     *  Wap Protocol Stack is down
     */
    ccShrdPrm.wapStat = CC_WAP_STACK_DOWN;
  
    if(wap_state EQ Wap_Not_Init) 
    {
      /* WAP-dedicated variables shall be reinitialized */
      wapId    = NO_ENTRY;
      Wap_Call = FALSE;
    
      TRACE_EVENT ("WAP parameter reseted");
    }
  }
#endif

#ifndef USE_L1FD_FUNC_INTERFACE
  PFREE (ra_deactivate_cnf);
#endif
}
#endif /* DTI */
/*==== EOF =========================================================*/

