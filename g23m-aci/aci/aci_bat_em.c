/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  ACI_BAT
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
|  Purpose :  This BAT wrapper modul is ...
| 
+----------------------------------------------------------------------------- 
*/ 
 
#include "aci_all.h"     /* includes prim.h, which includes p_bat.h */ 
#include "aci_cmh.h"     /* prototypes of sAT_,qAT_,tAT_    */ 
#include "aci_bat_cmh.h" /* prototypes of sBAT_,qBAT_,tBAT_ */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentEM       |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentEM       (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_percent_em_mode mode;
  T_BAT_percent_em_sin_type type = BAT_P_EM_TYPE_NOT_PRESENT;
  
  TRACE_FUNCTION ("sBAT_PercentEM()");

  mode = cmd->params.ptr_set_percent_em->mode;
  type = cmd->params.ptr_set_percent_em->sin_type;
  
  if ( (mode NEQ BAT_P_EM_MODE_SIN ) AND (mode NEQ BAT_P_EM_MODE_SIN ) )
  {
    return ACI_BAT_FAIL;
  }

  if ( (mode EQ BAT_P_EM_MODE_SIN ) AND ((type EQ BAT_P_EM_TYPE_NOT_PRESENT) OR ((type < BAT_P_EM_TYPE_SC) AND (type > BAT_P_EM_TYPE_GRLC ))) )
  { 
    return ACI_BAT_FAIL;
  }

  if ( (mode EQ BAT_P_EM_MODE_PCO ) AND (type EQ BAT_P_EM_TYPE_NOT_PRESENT) )
  { 
    return ACI_BAT_FAIL;
  }

  if (type EQ BAT_P_EM_TYPE_NOT_PRESENT)
  {
    type = BAT_P_EM_TYPE_SC;
  }
 
  ret = (T_ACI_BAT_RSLT)sAT_PercentEM((T_ACI_CMD_SRC)src_infos_psi->srcId, 
  	          (T_EM_AT_MODE)mode,(T_EM_AT_TYPE)type);
  return(ret);
}

