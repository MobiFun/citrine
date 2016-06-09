/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_TRAP
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
|             primitives send to the protocol stack adapter by TRA.
+----------------------------------------------------------------------------- 
*/ 

#ifdef DTI

#ifndef PSA_TRAP_C
#define PSA_TRAP_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "dti.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "aci_fd.h"
/*#include "aci_io.h"*/
#include "aci.h"
#include "psa.h"
#include "cmh.h"
#include "cmh_ra.h"
#include "psa_l2r.h"
#include "cmh_l2r.h"
#include "psa_ra.h"
#include "cmh_ra.h"
#include "psa_tra.h"

/*#include "aci_lst.h"
#include "psa_uart.h"
#include "cmh_uart.h" */

#include "psa_cc.h"

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_TRA                 |
|                                 ROUTINE : psa_tra_activate_cnf    |
+-------------------------------------------------------------------+

  PURPOSE : processes the TRA_ACTIVATE_CNF primitive sent by TRA.

*/

GLOBAL void psa_tra_activate_cnf
                              (T_TRA_ACTIVATE_CNF *tra_activate_cnf)
{
  TRACE_FUNCTION ("psa_tra_activate_cnf()");

  TRA_is_activated = TRUE;
  cmhCC_L2R_or_TRA_Activated ( DTI_ENTITY_TRA, raShrdPrm.cId );

  PFREE (tra_activate_cnf);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_TRA                 |
|                                 ROUTINE : psa_tra_deactivate_cnf  |
+-------------------------------------------------------------------+

  PURPOSE : processes the TRA_DEACTIVATE_CNF primitive sent by TRA.

*/

GLOBAL void psa_tra_deactivate_cnf
                              (T_TRA_DEACTIVATE_CNF *tra_deactivate_cnf)
{
  TRACE_FUNCTION ("psa_tra_deactivate_cnf()");

  TRA_is_activated = FALSE;
  cmhTRA_Deactivated();

  PFREE (tra_deactivate_cnf);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_TRA                |
|                                 ROUTINE : psa_tra_dti_cnf        |
+-------------------------------------------------------------------+

  PURPOSE : processes the TRA_DTI_CNF primitive sent by TRA.

*/

GLOBAL void psa_tra_dti_cnf
                              (T_TRA_DTI_CNF *tra_dti_cnf)
{
  TRACE_FUNCTION ("psa_tra_dti_cnf()");

  /* store the current DTI id for the case of redirection */
  raShrdPrm.dti_id = EXTRACT_DTI_ID(tra_dti_cnf->link_id);        

  switch( tra_dti_cnf->dti_conn )
  {
  case(TRA_CONNECT_DTI):
    dti_cntrl_entity_connected( tra_dti_cnf->link_id, DTI_ENTITY_TRA, DTI_OK );
    break;

  case(TRA_DISCONNECT_DTI):
    /* it depends here on the context:
    if network is disconnecting the call: */
    if(    (ccShrdPrm.datStat EQ DS_DSC_REQ)
        OR (ccShrdPrm.datStat EQ DS_STOP_REQ)
        OR (ccShrdPrm.ctb[raShrdPrm.cId] EQ NULL) ) 
    {
      dti_cntrl_entity_disconnected( tra_dti_cnf->link_id, DTI_ENTITY_TRA );
      psaTRA_Deactivate();
    }
    else /* other cases */
    {
      raEntStat.isTempDisconnected = TRUE;
      dti_cntrl_entity_disconnected( tra_dti_cnf->link_id, DTI_ENTITY_TRA );
    }

    break;
  }

  PFREE (tra_dti_cnf);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_TRA                 |
|                                 ROUTINE : psa_tra_dti_ind         |
+-------------------------------------------------------------------+

  PURPOSE : processes the TRA_DTI_IND primitive sent by TRA.

*/

GLOBAL void psa_tra_dti_ind
                              (T_TRA_DTI_IND *tra_dti_ind)
{
  TRACE_FUNCTION ("psa_tra_dti_ind()");

  PFREE (tra_dti_ind);
}
#endif /* DTI */
