/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_UDPAP
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
|             primitives sent to the protocol stack adapter by the UDPA
|             module.
+-----------------------------------------------------------------------------
*/
#ifdef DTI
#ifdef CO_UDP_IP

#ifndef PSA_UDPAP_C
#define PSA_UDPAP_C
#endif /* of PSA_UDPAP_C */

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "aci_fd.h"
#include "aci.h"
#include "psa.h"
#include "psa_l2r.h"
#include "cmh.h"
#include "cmh_ra.h"
#include "cmh_l2r.h"
#include "psa_tcpip.h"
#include "psa_cc.h"

#include "wap_aci.h"
#include "psa_sat.h"

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_UDPA                |
|                                 ROUTINE : psa_udpa_dti_cnf        |
+-------------------------------------------------------------------+

  PURPOSE : processes the UDPA_DTI_CNF primitive sent by UDP.

*/

GLOBAL void psa_udpa_dti_cnf(T_UDPA_DTI_CNF *udpa_dti_cnf)
{
  TRACE_FUNCTION ("psa_udpa_dti_cnf()");

  switch( udpa_dti_cnf->dti_conn )
  {
  case(UDPA_CONNECT_DTI):
    dti_cntrl_entity_connected( udpa_dti_cnf->link_id, DTI_ENTITY_UDP, DTI_OK );
    break;

  case(UDPA_DISCONNECT_DTI):
    dti_cntrl_entity_disconnected( udpa_dti_cnf->link_id, DTI_ENTITY_UDP );
    break;

  }

  PFREE(udpa_dti_cnf);
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_UDPA                |
|                                 ROUTINE : psa_udpa_dti_ind        |
+-------------------------------------------------------------------+

  PURPOSE : processes the UDPA_DTI_IND primitive sent by UDP.

*/

GLOBAL void psa_udpa_dti_ind(T_UDPA_DTI_IND *udpa_dti_ind)
{
  TRACE_FUNCTION ("psa_udpa_dti_ind()");

  dti_cntrl_entity_disconnected( udpa_dti_ind->link_id, DTI_ENTITY_UDP );

  PFREE(udpa_dti_ind);
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_UDPA                |
|                                 ROUTINE : psa_udpa_config_cnf     |
+-------------------------------------------------------------------+

  PURPOSE : processes the UDPA_CONFIG_CNF primitive sent by UDP.

*/

GLOBAL void psa_udpa_config_cnf(T_UDPA_CONFIG_CNF *udpa_config_cnf)
{
  TRACE_FUNCTION ("psa_udpa_config_cnf()");

  if (wap_state EQ UDPA_Configuration)
  {
    wap_state = UDPA_Configurated;
    cmhUDPA_Configurated();
  }
  else if (wap_state EQ UDPA_Deconfiguration)
  {
    wap_state = UPDA_Deconfigurated;
    cmhUDPA_Deconfigurated();
  }
  else
  {
    TRACE_EVENT("unexpected UDPA_CONFIG_CNF primitive received");
  }

  PFREE(udpa_config_cnf);
}

#endif /* CO_UDP_IP */

#endif /* DTI */
