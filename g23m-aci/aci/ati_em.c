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
|  Purpose :  This module provides the ati interface for the engineering mode. 
+----------------------------------------------------------------------------- 
*/ 

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#undef DUMMY_ATI_STRINGS

#include "aci_all.h"

#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_io.h"
#include "aci_cmd.h"
#include "l4_tim.h"
#include "aci_em.h"

#include "aci_mem.h"
#include "aci_prs.h"
#include "aci_lst.h"

#include "ati_int.h"

#ifdef FF_ATI_BAT

#include "typedefs.h"
#include "gdd.h"
#include "bat.h"

#include "ati_bat.h"

#endif /*FF_ATI_BAT*/

/*==== EXPORT =====================================================*/
/*==== VARIABLES ==================================================*/
/*==== FUNCTIONS ==================================================*/
/*==== CONSTANTS ==================================================*/
#define EMET_L1  0x0001
#define EMET_ALL 0x00ff

/*
+------------------------------------------------------------------------------
|  Function     :  setatPercentEM
+------------------------------------------------------------------------------
|  Description  :  AT-cmd for the EM infrastructure data 
|                   
|
|  Parameters   :  char  *cl
|                  UBYTE srcId
|
|  Return       :  char  *
+------------------------------------------------------------------------------
*/
GLOBAL T_ATI_RSLT setatPercentEM (char *cl, UBYTE srcId)
{
  T_ACI_RETURN      ret        = AT_FAIL;
  T_EM_AT_MODE      mode       = (T_EM_AT_MODE)ACI_NumParmNotPresent;
  T_EM_AT_TYPE      type       = (T_EM_AT_TYPE)ACI_NumParmNotPresent; 
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  src_params->curAtCmd = AT_CMD_EM;
  
  cl = parse (cl, "dd", &mode, &type);

  if ( (mode NEQ SIN_REP) AND (mode NEQ PCO_REP) )
  {
    cmdCmeError (CME_ERR_OpNotSupp);
    return ATI_FAIL;
  }

  if ( (mode EQ SIN_REP) AND ((type EQ ACI_NumParmNotPresent) OR ((type < EM_AT_SC) AND (type > EM_AT_PDP))) )
  { 
   cmdCmeError (CME_ERR_OpNotSupp);
   return ATI_FAIL;
  }

  if ( (mode EQ PCO_REP) AND ((type EQ ACI_NumParmNotPresent) OR (type > EM_AT_PCO_HIGHEST-1)) )
  { 
   cmdCmeError (CME_ERR_OpNotSupp);
   return ATI_FAIL;
  }
 
  if (type EQ ACI_NumParmNotPresent)
  {
   type = EM_AT_SC;
  }

  srcId_cb = srcId;

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_percent_em my_bat_set_percent_em;

  TRACE_FUNCTION("setatPercentEM() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_percent_em, 0, sizeof(my_bat_set_percent_em));
  cmd.ctrl_params = BAT_CMD_SET_PERCENT_EM;
  cmd.params.ptr_set_percent_em = &my_bat_set_percent_em;

  my_bat_set_percent_em.mode = mode;
  my_bat_set_percent_em.sin_type = type;
  my_bat_set_percent_em.pco_type = type;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }

#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPercentEM()");

  ret = sAT_PercentEM((T_ACI_CMD_SRC)srcId, mode, type);
  
  switch(ret)
  {
    case AT_CMPL:
      break;

    case AT_EXCT:
      src_params->curAtCmd    = AT_CMD_EM;
      break;

    case AT_FAIL:
      cmdCmeError(CME_ERR_Unknown);
      break;   

    default:
      cmdCmeError(CME_ERR_Unknown);
      break;
  } /* switch */
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
} 

/*
+------------------------------------------------------------------------------
|  Function     :  setatPercentEMET
+------------------------------------------------------------------------------
|  Description  :  AT-cmd for the EM event traces 
|                   
|
|  Parameters   :  char  *cl
|                  UBYTE srcId
|
|  Return       :  char  *
+------------------------------------------------------------------------------
*/
GLOBAL T_ATI_RSLT setatPercentEMET (char *cl, UBYTE srcId)
{
  T_ACI_RETURN      ret        = AT_EXCT;
  SHORT             type       = ACI_NumParmNotPresent; 
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);


  TRACE_FUNCTION("setatPercentEMET()");
  
  cl = parse (cl, "d", &type);

   /* if type is not in range then error*/
 if ((type < EMET_L1) OR (type > EMET_ALL))
 {
   cmdCmeError (CME_ERR_OpNotSupp);
   return ATI_FAIL;
 }
  
  srcId_cb = srcId;

  ret = sAT_PercentEMET((T_ACI_CMD_SRC)srcId, (UBYTE)type);
  
  switch(ret)
  {
    case AT_CMPL:
      break;

    case AT_EXCT:
      src_params->curAtCmd    = AT_CMD_EMET;
      break;

    case AT_FAIL:
      cmdCmeError(CME_ERR_Unknown);
      break;   

    default:
      cmdCmeError(CME_ERR_Unknown);
      break;
  } /* switch */
  return (map_aci_2_ati_rslt(ret));
} /* setatPercentEMET */

/*
+------------------------------------------------------------------------------
|  Function     :  setatPercentEMETS
+------------------------------------------------------------------------------
|  Description  :  AT-cmd for the EM event traces 
|                   
|
|  Parameters   :  char  *cl
|                  UBYTE srcId
|
|  Return       :  char  *
+------------------------------------------------------------------------------
*/
GLOBAL T_ATI_RSLT setatPercentEMETS (char *cl, UBYTE srcId)
{
  T_ACI_RETURN        ret        = AT_EXCT;
  SHORT               subclass    = ACI_NumParmNotPresent;
  ULONG               bitm_h      = (ULONG)ACI_NumParmNotPresent;
  ULONG               bitm_l      = (ULONG)ACI_NumParmNotPresent;
  T_ATI_SRC_PARAMS   *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("setatPercentEMETS()");

  cl = parse (cl, "dyy", &subclass, &bitm_h, &bitm_l);

  if ( (subclass < EM_L1) OR (subclass > EM_SIM) )
  {
   cmdCmeError (CME_ERR_OpNotSupp);
   return ATI_FAIL;
  }
  
  srcId_cb = srcId;

  ret = sAT_PercentEMETS((T_ACI_CMD_SRC)srcId, (UBYTE)subclass, bitm_h, bitm_l);
  
  switch(ret)
  {
    case AT_CMPL:
      break;

    case AT_EXCT:
      src_params->curAtCmd    = AT_CMD_EMETS;
      break;

    case AT_FAIL:
      cmdCmeError(CME_ERR_Unknown);
      break;   

    default:
      cmdCmeError(CME_ERR_Unknown);
      break;
  } /* switch */
  return (map_aci_2_ati_rslt(ret));
} /* setatPercentEMETS */



/*+++++++++++++++++++++++++++++++++++++++++ E O F +++++++++++++++++++++++++++++++++++++++++*/

