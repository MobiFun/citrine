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
|  Purpose :  This module implements the test fuinctions related to the
|             protocol stack adapter for GPRS session management ( SM ).
+----------------------------------------------------------------------------- 
*/ 

#ifdef GPRS

#ifndef CMH_SMT_C
#define CMH_SMT_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "gaci.h"
#include "gaci_cmh.h"
#include "psa.h"
#include "psa_sm.h"
#include "psa_gppp.h"
#include "psa_gmm.h"

#include "cmh.h"
#include "cmh_sm.h"
#include "cmh_gppp.h"

#include "cmh_gmm.h"


/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8441)         MODULE  : CMH_SMS                  |
| STATE   : finnished             ROUTINE : tAT_PlusCGPADDR          |
+--------------------------------------------------------------------+

PURPOSE : This is the functional counterpart to the +CGPADDR= AT
          command which sets the requested QOS.
*/

GLOBAL T_ACI_RETURN tAT_PlusCGPADDR ( T_ACI_CMD_SRC srcId, U8 *cids)
{
  T_PDP_CONTEXT_STATE  con_state;
  U8 cid;
  U8 index = 0;

  TRACE_FUNCTION ("tAT_PlusCGPADDR()");

  for( cid = PDP_CONTEXT_CID_MIN; cid <= PDP_CONTEXT_CID_MAX; cid++ )
  {
    con_state = get_state_over_cid(cid);
    /*if ( (con_state NEQ PDP_CONTEXT_STATE_UNDEFINED)AND
         (con_state NEQ PDP_CONTEXT_STATE_INVALID) )*/
    if ( (con_state NEQ PDP_CONTEXT_STATE_INVALID) )
    {
      cids[index ++] = cid;
    }
    cids[index] = PDP_CONTEXT_CID_INVALID;
  }

  return AT_CMPL;
}

GLOBAL T_ACI_RETURN tAT_PlusCGSMS( T_ACI_CMD_SRC srcId, SHORT *service_list)
{

  TRACE_FUNCTION ("tAT_PlusCGSMS()");

  *service_list = 15;

  return AT_CMPL;
}

#ifdef REL99
/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : CMH_SMQ                  |
| STATE   : finished              ROUTINE : qAT_PlusCGEQNEG          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CGEQNEG=? or
            +CGCMOD AT command and returns context_activated.
  RETURNS:  - AT_CMPL : Completed.
            - AT_FAIL : Command not valid for srcId.
  UPDATES:  - context_activated: true  - context is activated for cid.
                                 false - context is not activated.
*/
GLOBAL T_ACI_RETURN tAT_PlusCGEQNEG_CGCMOD ( T_ACI_CMD_SRC srcId, U8 cid, BOOL *context_activated)
{
  TRACE_FUNCTION ("tAT_PlusCGEQNEG_CGCMOD()");

/*
 *-------------------------------------------------------------------
 * check command source - should be Serial link ?
 *-------------------------------------------------------------------
 */
  if ( !cmh_IsVldCmdSrc (srcId) )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */
  if (( get_state_over_cid(cid) EQ PDP_CONTEXT_STATE_ACTIVATED ) OR ( get_state_over_cid(cid) EQ PDP_CONTEXT_STATE_DATA_LINK ))
    *context_activated = TRUE;
  else 
    *context_activated = FALSE;

  return( AT_CMPL );
}


/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : CMH_SMT                  |
| STATE   : finished              ROUTINE : tAT_PlusCGDSCONT         |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CGDSCONT=?

  RETURNS:  - AT_CMPL : Completed.
            - AT_FAIL : Command not valid for srcId.
  UPDATES:  - context_activated: true  - primary context is activated for cid.
                                 false - context is not activated or not primary.
*/
GLOBAL T_ACI_RETURN tAT_PlusCGDSCONT( T_ACI_CMD_SRC srcId, U8 cid, BOOL *context_activated )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  TRACE_FUNCTION ("tAT_PlusCGDSCONT()");

/*
 *-------------------------------------------------------------------
 * check command source - should be Serial link ?
 *-------------------------------------------------------------------
 */
  if( !cmh_IsVldCmdSrc (srcId) )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );
  if( p_pdp_context_node )
  {
    if( p_pdp_context_node->type EQ PDP_CONTEXT_TYPE_PRIMARY )
    {
      if( p_pdp_context_node->internal_data.state EQ PDP_CONTEXT_STATE_ACTIVATED OR
          p_pdp_context_node->internal_data.state EQ PDP_CONTEXT_STATE_DATA_LINK )
      {
        *context_activated = TRUE;
        return AT_CMPL;
      }
      else
      {
        *context_activated = FALSE;
        return AT_CMPL;
      }
    }
    else
    {
      *context_activated = FALSE;
      return AT_CMPL;
    }
  }
  else
  {
    *context_activated = FALSE;
    return AT_CMPL;
  }
}
#endif /* REL99 */

#endif  /* GPRS */


/*==== EOF ========================================================*/
