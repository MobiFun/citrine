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
|  Purpose :  Definitions for the processing functions of the protocol
|             stack adapter for the engineering mode.
+----------------------------------------------------------------------------- 
*/ 

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci.h"
#include "aci_em.h"
#include "aci_mem.h"

#ifdef FF_EM_MODE

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/
/*
#if !defined (WIN32)
EXTERN CHAR* l1_version(void);
EXTERN CHAR* dl_version(void);
EXTERN CHAR* rr_version(void);
EXTERN CHAR* mm_version(void);
EXTERN CHAR* cc_version(void);
EXTERN CHAR* ss_version(void);
EXTERN CHAR* sim_version(void);
EXTERN CHAR* sms_version(void);
//EXTERN CHAR* aci_version(void);
#endif
*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/


/*
+------------------------------------------------------------------------------
|  Function     :  psa_em_sc_info_cnf
+------------------------------------------------------------------------------
|  Description  :  Pars the serving cell infrastructure data back to the initial 
|                  function inside the customer specific MMI with a cb-function.
|
|  Parameters   :  Primitiv
|
|  Return       :  void
+------------------------------------------------------------------------------
*/

GLOBAL void psa_em_sc_info_cnf (T_EM_SC_INFO_CNF *em_sc_info_cnf)
{
  TRACE_FUNCTION ("psa_em_sc_info_cnf()");

  em_Received_Data(em_sc_info_cnf, EM_SUBCLASS_SC);  
}

/*
+------------------------------------------------------------------------------
|  Function     :  psa_em_sc_gprs_info_cnf
+------------------------------------------------------------------------------
|  Description  :  Pars the serving cell gprs infrastructure data back to the initial 
|                  function inside the customer specific MMI with a cb-function.
|
|  Parameters   :  Primitiv
|
|  Return       :  void
+------------------------------------------------------------------------------
*/

GLOBAL void psa_em_sc_gprs_info_cnf (T_EM_SC_GPRS_INFO_CNF *em_sc_gprs_info_cnf)
{
  TRACE_FUNCTION ("psa_em_sc_gprs_info_cnf()");

  em_Received_Data(em_sc_gprs_info_cnf, EM_SUBCLASS_SC_GPRS);  
}
 
/*
+------------------------------------------------------------------------------
|  Function     :  psa_em_nc_info_cnf
+------------------------------------------------------------------------------
|  Description  :  Pars the serving cell infrastructure data back to the initial 
|                  function inside the customer specific MMI with a cb-function.
|
|  Parameters   :  Primitiv
|
|  Return       :  void
+------------------------------------------------------------------------------
*/

GLOBAL void psa_em_nc_info_cnf (T_EM_NC_INFO_CNF *em_nc_info_cnf)
{
  TRACE_FUNCTION ("psa_em_nc_info_cnf()");

  em_Received_Data(em_nc_info_cnf, EM_SUBCLASS_NC);  
}

/*
+------------------------------------------------------------------------------
|  Function     :  psa_em_loc_pag_info_cnf
+------------------------------------------------------------------------------
|  Description  :  Pars the serving cell infrastructure data back to the initial 
|                  function inside the customer specific MMI with a cb-function.
|
|  Parameters   :  Primitiv
|
|  Return       :  void
+------------------------------------------------------------------------------
*/

GLOBAL void psa_em_loc_pag_info_cnf (T_EM_LOC_PAG_INFO_CNF *em_loc_pag_info_cnf)
{
  TRACE_FUNCTION ("psa_em_loc_pag_info_cnf()");

  em_Received_Data(em_loc_pag_info_cnf, EM_SUBCLASS_LOC_PAG);  
}

/*
+------------------------------------------------------------------------------
|  Function     :  psa_em_plmn_info_cnf
+------------------------------------------------------------------------------
|  Description  :  Pars the serving cell infrastructure data back to the initial 
|                  function inside the customer specific MMI with a cb-function.
|
|  Parameters   :  Primitiv
|
|  Return       :  void
+------------------------------------------------------------------------------
*/

GLOBAL void psa_em_plmn_info_cnf (T_EM_PLMN_INFO_CNF *em_plmn_info_cnf)
{
  TRACE_FUNCTION ("psa_em_plmn_info_cnf()");

  em_Received_Data(em_plmn_info_cnf, EM_SUBCLASS_PLMN);  
}

/*
+------------------------------------------------------------------------------
|  Function     :  psa_em_cip_hop_dtx_info_cnf
+------------------------------------------------------------------------------
|  Description  :  Pars the serving cell infrastructure data back to the initial 
|                  function inside the customer specific MMI with a cb-function.
|
|  Parameters   :  Primitiv
|
|  Return       :  void
+------------------------------------------------------------------------------
*/

GLOBAL void psa_em_cip_hop_dtx_info_cnf (T_EM_CIP_HOP_DTX_INFO_CNF *em_cip_hop_dtx_info_cnf)
{
  TRACE_FUNCTION ("psa_em_cip_hop_dtx_info_cnf()");

  em_Received_Data(em_cip_hop_dtx_info_cnf, EM_SUBCLASS_CIPH_HOP_DTX);  
}

/*
+------------------------------------------------------------------------------
|  Function     :  psa_em_power_info_cnf
+------------------------------------------------------------------------------
|  Description  :  Pars the serving cell infrastructure data back to the initial 
|                  function inside the customer specific MMI with a cb-function.
|
|  Parameters   :  Primitiv
|
|  Return       :  void
+------------------------------------------------------------------------------
*/

GLOBAL void psa_em_power_info_cnf (T_EM_POWER_INFO_CNF *em_power_info_cnf)
{
  TRACE_FUNCTION ("psa_em_power_info_cnf()");

  em_Received_Data(em_power_info_cnf, EM_SUBCLASS_POWER);  
}

/*
+------------------------------------------------------------------------------
|  Function     :  psa_em_identity_info_cnf
+------------------------------------------------------------------------------
|  Description  :  Pars the serving cell infrastructure data back to the initial 
|                  function inside the customer specific MMI with a cb-function.
|
|  Parameters   :  Primitiv
|
|  Return       :  void
+------------------------------------------------------------------------------
*/

GLOBAL void psa_em_identity_info_cnf (T_EM_IDENTITY_INFO_CNF *em_identity_info_cnf)
{
  TRACE_FUNCTION ("psa_em_identity_info_cnf()");

  em_Received_Data(em_identity_info_cnf, EM_SUBCLASS_ID);  
}

/*
+------------------------------------------------------------------------------
|  Function     :  psa_em_sw_version_info_cnf
+------------------------------------------------------------------------------
|  Description  :  Pars the serving cell infrastructure data back to the initial 
|                  function inside the customer specific MMI with a cb-function.
|
|  Parameters   :  Primitiv
|
|  Return       :  void
+------------------------------------------------------------------------------
*/

GLOBAL void psa_em_sw_version_info_cnf (T_EM_SW_VERSION_INFO_CNF *em_sw_version_info_cnf)
{
  TRACE_FUNCTION ("psa_em_sw_version_info_cnf()");

  em_Received_Data(em_sw_version_info_cnf, EM_SUBCLASS_SW_VERSION);  
}

/*
+------------------------------------------------------------------------------
|  Function     :  psa_em_gmm_info_cnf
+------------------------------------------------------------------------------
|  Description  :  Pars the serving cell gmm data back to the initial 
|                  function inside the customer specific MMI with a cb-function.
|
|  Parameters   :  Primitiv
|
|  Return       :  void
+------------------------------------------------------------------------------
*/

GLOBAL void psa_em_gmm_info_cnf (T_EM_GMM_INFO_CNF *em_gmm_info_cnf)
{
  TRACE_FUNCTION ("psa_em_gmm_info_cnf()");

  em_Received_Data(em_gmm_info_cnf, EM_SUBCLASS_GMM);  
}
 /*
+------------------------------------------------------------------------------
|  Function     :  psa_em_grlc_info_cnf
+------------------------------------------------------------------------------
|  Description  :  Pars the serving cell grlc data back to the initial 
|                  function inside the customer specific MMI with a cb-function.
|
|  Parameters   :  Primitiv
|
|  Return       :  void
+------------------------------------------------------------------------------
*/

GLOBAL void psa_em_grlc_info_cnf (T_EM_GRLC_INFO_CNF *em_grlc_info_cnf)
{
  TRACE_FUNCTION ("psa_em_grlc_info_cnf()");

  em_Received_Data(em_grlc_info_cnf, EM_SUBCLASS_GRLC);  
}
 

/*
+------------------------------------------------------------------------------
|  Function     :  psa_em_amr_info_cnf
+------------------------------------------------------------------------------
|  Description  :  Pars the AMR information data back to the initial 
|                  function inside the customer specific MMI with a cb-function.
|
|  Parameters   :  Primitive
|
|  Return       :  void
+------------------------------------------------------------------------------
*/

GLOBAL void psa_em_amr_info_cnf (T_EM_AMR_INFO_CNF *em_amr_info_cnf)
{
  TRACE_FUNCTION ("psa_em_amr_info_cnf()");

  em_Received_Data(em_amr_info_cnf, EM_SUBCLASS_AMR);  
}
#endif /* FF_EM_MODE */

/*+++++++++++++++++++++++++++++++++++++++++ E O F +++++++++++++++++++++++++++++++++++++++++*/

