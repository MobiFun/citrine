/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_IPAP
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
|             primitives sent to the protocol stack adapter by the IPA
|             module.
+----------------------------------------------------------------------------- 
*/ 
#if defined (FF_WAP) || defined (FF_SAT_E)
#ifdef DTI

#ifndef PSA_IPAP_C
#define PSA_IPAP_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */

#include "aci.h"
#include "psa.h"
#include "aci_fd.h"
#include "cmh.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "wap_aci.h"

#include "psa_ra.h"	 
#include "cmh_ra.h"

#include "psa_cc.h"	 

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_IPA                      |
|                            ROUTINE : psa_ipa_dti_cnf              |
+-------------------------------------------------------------------+

  PURPOSE : processes IPA_DTI_CNF received from IPA.

*/

GLOBAL void psa_ipa_dti_cnf( T_IPA_DTI_CNF *ipa_dti_cnf )
{
  TRACE_FUNCTION ("psa_ipa_dti_cnf()");

  switch( ipa_dti_cnf->dti_conn )
  {
  case(IPA_CONNECT_DTI):
    dti_cntrl_entity_connected( ipa_dti_cnf->link_id, DTI_ENTITY_IP, DTI_OK );
    break;

  case(IPA_DISCONNECT_DTI):
    dti_cntrl_entity_disconnected( ipa_dti_cnf->link_id, DTI_ENTITY_IP );
    break;

  }
	PFREE(ipa_dti_cnf);
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_IPA                      |
|                            ROUTINE : psa_ipa_dti_ind              |
+-------------------------------------------------------------------+

  PURPOSE : processes IPA_DTI_CNF received from IPA.

*/

GLOBAL void psa_ipa_dti_ind( T_IPA_DTI_IND *ipa_dti_ind )
{
  TRACE_FUNCTION ("psa_ipa_dti_ind()");

  dti_cntrl_entity_disconnected( ipa_dti_ind->link_id, DTI_ENTITY_IP );

	PFREE(ipa_dti_ind);
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_IPA                      |
|                            ROUTINE : psa_ipa_config_cnf           |
+-------------------------------------------------------------------+

  PURPOSE : processes IPA_CONFIG_CNF received from IP.

*/

GLOBAL void psa_ipa_config_cnf( T_IPA_CONFIG_CNF *ipa_config_cnf )
{
  TRACE_FUNCTION ("psa_ipa_config_cnf()");

  if (wap_state EQ IPA_Configuration)
  {
    wap_state = IPA_Configurated;
    cmhIPA_Configurated();
  }
  else if (wap_state EQ IPA_Deconfiguration)
  {
    wap_state = IPA_Deconfigurated;
	  if (ipa_config_cnf -> all_down EQ IPA_ALLDOWN_TRUE)
    { 
      cmhIPA_Deconfigurated();
    }
    else
    {
      TRACE_EVENT("all entities not down yet");
    }
  }
  else
  {
    TRACE_EVENT("unexpected IPA_CONFIG_CNF primitive received");
  }

  PFREE(ipa_config_cnf);
}

#endif /* DTI */

#endif /* of FF_WAP or SAT E */
