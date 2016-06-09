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
|  Purpose :  This module provides the query functions related to the
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
#include "ati_cmd.h"
#include "aci_io.h"

#include "psa.h"
#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif
#include "cmh.h"
#include "aci_em.h"
#include "cmh_em.h"


/*==== EXPORT =====================================================*/
/*
EXTERN UBYTE em_subclass_sc            (UBYTE em_type);
EXTERN UBYTE em_subclass_sc_gprs       (UBYTE em_type);
EXTERN UBYTE em_subclass_nc            (UBYTE em_type);
EXTERN UBYTE em_subclass_loc_pag       (UBYTE em_type);
EXTERN UBYTE em_subclass_plmn          (UBYTE em_type);
EXTERN UBYTE em_subclass_ciph_hop_dtx  (UBYTE em_type);
EXTERN UBYTE em_subclass_power         (UBYTE em_type);
EXTERN UBYTE em_subclass_id            (UBYTE em_type);
EXTERN UBYTE em_subclass_version       (UBYTE em_type); */
EXTERN void  cmhEM                     (T_EM_VAL * val_tmp);
EXTERN void  cmhEMET                   (T_DRV_SIGNAL_EM_EVENT * Signal);

/*==== VARIABLES ==================================================*/

GLOBAL T_EM_SHRD_PRM emPrm;
GLOBAL T_EM_SHRD_PRM emetPrm;
GLOBAL T_EM_SHRD_PRM emetsPrm;

/*==== CONSTANTS ==================================================*/

/*==== FUNCTIONS ==================================================*/

LOCAL void em_at_cbfkt(T_DRV_SIGNAL_EM * Signal)
{
  T_EM_VAL val_tmp;

  TRACE_FUNCTION ("em_at_cbfkt()");

  memset(&val_tmp, 0x00, sizeof(T_EM_VAL));

  val_tmp.em_utype = Signal->SignalType;
  switch (Signal->SignalType)
  {
    case EM_SUBCLASS_SC:
      memcpy(&val_tmp.em_u.em_sc_val, &Signal->UserData.sc, sizeof(T_EM_SC_INFO_CNF));
      break;
    case EM_SUBCLASS_SC_GPRS:
      memcpy(&val_tmp.em_u.em_sc_gprs_val, &Signal->UserData.sc_gprs, sizeof(T_EM_SC_GPRS_INFO_CNF));
      break;
    case EM_SUBCLASS_NC:
      memcpy(&val_tmp.em_u.em_nc_val, &Signal->UserData.nc, sizeof(T_EM_NC_INFO_CNF));
      break;
    case EM_SUBCLASS_LOC_PAG:
      memcpy(&val_tmp.em_u.em_loc_val, &Signal->UserData.log_pag, sizeof(T_EM_LOC_PAG_INFO_CNF));
      break;
    case EM_SUBCLASS_PLMN:
      memcpy(&val_tmp.em_u.em_plmn_val, &Signal->UserData.plmn, sizeof(T_EM_PLMN_INFO_CNF));
      break;
    case EM_SUBCLASS_CIPH_HOP_DTX:
      memcpy(&val_tmp.em_u.em_cip_val, &Signal->UserData.cip, sizeof(T_EM_CIP_HOP_DTX_INFO_CNF));
      break;
    case EM_SUBCLASS_POWER:
      memcpy(&val_tmp.em_u.em_power_val, &Signal->UserData.power, sizeof(T_EM_POWER_INFO_CNF));
      break;
    case EM_SUBCLASS_ID:
      memcpy(&val_tmp.em_u.em_id_val, &Signal->UserData.id, sizeof(T_EM_IDENTITY_INFO_CNF));
      break;
    case EM_SUBCLASS_SW_VERSION:
      memcpy(&val_tmp.em_u.em_ver_val, &Signal->UserData.version, sizeof(T_EM_SW_VER));
      break;
    case EM_SUBCLASS_GMM:
      memcpy(&val_tmp.em_u.em_gmm_val, &Signal->UserData.gmm, sizeof(T_EM_GMM_INFO_CNF));
      break;
    case EM_SUBCLASS_GRLC:
      memcpy(&val_tmp.em_u.em_grlc_val, &Signal->UserData.grlc, sizeof(T_EM_GRLC_INFO_CNF));
      break;
    case EM_SUBCLASS_AMR:
      memcpy(&val_tmp.em_u.em_amr_val, &Signal->UserData.amr, sizeof(T_EM_AMR_INFO_CNF));
      break;
    case EM_SUBCLASS_PDP:
      break;
    default:
      break;
  }

  cmhEM( &val_tmp ); 
}

LOCAL void em_event_at_cbfkt(T_DRV_SIGNAL_EM_EVENT * Signal)
{
  /*
  *  Copy the data in an internal variable
  */
  T_DRV_SIGNAL_EM_EVENT Signal_tmp;

  memcpy(&Signal_tmp, Signal, sizeof(T_DRV_SIGNAL_EM_EVENT));

  cmhEMET (&Signal_tmp);
}


GLOBAL T_ACI_RETURN sAT_PercentEM  ( T_ACI_CMD_SRC srcId, 
                                     T_EM_AT_MODE  mode,
                                     T_EM_AT_TYPE  type)
{
  UBYTE em_class    = 0;
  UBYTE em_subclass = 0;
  UBYTE em_type     = mode;
  
  TRACE_FUNCTION ("sAT_PercentEM ()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    return( AT_FAIL );
  }
  emPrm.srcId = srcId;

  if(mode EQ PCO_REP) /*request EM data as trace output*/
  { 
    em_subclass_pco_bitmap(type); /*function to send EM_PCO_TRACE_REQ to RR (and GMM) */
    return AT_CMPL;
  }
    
  switch(type)
  {
    case EM_AT_SC:
      em_class =     EM_CLASS_INFRA_DATA;
      em_subclass =  EM_SUBCLASS_SC;
      break;
#ifdef GPRS
    case EM_AT_SC_GPRS:
      em_class =     EM_CLASS_INFRA_DATA;
      em_subclass =  EM_SUBCLASS_SC_GPRS;
      break;
#endif /* GPRS */     
    case EM_AT_NC:
      em_class =     EM_CLASS_INFRA_DATA;
      em_subclass =  EM_SUBCLASS_NC;
      break;
    case EM_AT_LOC_PAG:
      em_class =     EM_CLASS_INFRA_DATA;
      em_subclass =  EM_SUBCLASS_LOC_PAG;
      break;
    case EM_AT_PLMN:
      em_class =     EM_CLASS_INFRA_DATA;
      em_subclass =  EM_SUBCLASS_PLMN;
      break;
    case EM_AT_CIPH_HOP_DTX:
      em_class =     EM_CLASS_INFRA_DATA;
      em_subclass =  EM_SUBCLASS_CIPH_HOP_DTX;
      break;
    case EM_AT_POWER:
      em_class =     EM_CLASS_MOBILE_DATA;
      em_subclass =  EM_SUBCLASS_POWER;
      break;
    case EM_AT_ID:
      em_class =     EM_CLASS_MOBILE_DATA;
      em_subclass =  EM_SUBCLASS_ID;
      break;
    case EM_AT_VER:
      em_class =     EM_CLASS_MOBILE_DATA;
      em_subclass =  EM_SUBCLASS_SW_VERSION;
      break;
#ifdef GPRS
    case EM_AT_GMM:
      em_class =     EM_CLASS_INFRA_DATA;
      em_subclass =  EM_SUBCLASS_GMM;
      break;
    case EM_AT_GRLC:
      em_class =     EM_CLASS_INFRA_DATA;
      em_subclass =  EM_SUBCLASS_GRLC;
      break;
#endif /* GPRS */ 
    case EM_AT_AMR:
      em_class =     EM_CLASS_INFRA_DATA;
      em_subclass =  EM_SUBCLASS_AMR;
      break;
#ifdef GPRS
    case EM_AT_PDP:
      em_class =     EM_CLASS_INFRA_DATA;
      em_subclass =  EM_SUBCLASS_PDP;
      break;
#endif
    default:
     return AT_FAIL;
  } /* switch */

  em_Read_Data_Parameter( em_class, em_subclass, em_type, em_at_cbfkt );

  if(em_subclass == EM_SUBCLASS_PDP)
    return AT_CMPL;

  return AT_EXCT;

}  /* sAT_PercentEM */

GLOBAL T_ACI_RETURN sAT_PercentEMET( T_ACI_CMD_SRC srcId, 
                                     UBYTE         type)
{
  
  TRACE_FUNCTION ("sAT_PercentEMET ()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    return( AT_FAIL );
  }
  emetPrm.srcId = srcId;
  em_Read_Event_Parameter (type, em_event_at_cbfkt);

  return AT_CMPL;
}  /* sAT_PercentEMET */

GLOBAL T_ACI_RETURN sAT_PercentEMETS( T_ACI_CMD_SRC srcId, UBYTE subclass, 
                                      ULONG bitm_h, ULONG bitm_l )
{
  
  TRACE_FUNCTION ("sAT_PercentEMETS ()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    return( AT_FAIL );
  }
  emetsPrm.srcId = srcId;

  em_Set_EventTrace (subclass, bitm_h, bitm_l);

  return AT_CMPL;
}  /* sAT_PercentEMETS */



/*+++++++++++++++++++++++++++++++++++++++++ E O F +++++++++++++++++++++++++++++++++++++++++*/


