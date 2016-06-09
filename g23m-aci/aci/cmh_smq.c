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
|             protocol stack adapter for GPRS session management ( SM ).
+----------------------------------------------------------------------------- 
*/ 

#ifdef GPRS
#ifndef CMH_SMQ_C
#define CMH_SMQ_C
#endif

#include "aci_all.h"
#include "cl_inline.h"

/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "pcm.h"

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "gaci.h"
#include "gaci_cmh.h"
#include "psa.h"
#include "psa_sm.h"

#include "cmh.h"
#include "cmh_sm.h"

#include "psa_gmm.h"


#include "gaci_cmh.h"

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/
EXTERN T_PDP_CONTEXT_INTERNAL *p_pdp_context_list;

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMQ                  |
| STATE   : finished              ROUTINE : qAT_PlusCGQREQ           |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CGQREG? AT
            command and returns current settings for the specified
            PDP context. The function is called for all cids.
            Note: The QoS returned is always in Release 97 format.
  RETURNS:  - AT_CMPL : Completed.
            - AT_FAIL : Command not valid for srcId.
  UPDATES:  - qos: Quality of service for cid. Not updated if cid is undefined.
            - qos_valid: Indicates whether qos is updated not.
*/
GLOBAL T_ACI_RETURN qAT_PlusCGQREQ ( T_ACI_CMD_SRC srcId, U8 cid, BOOL *qos_valid, T_PS_qos *qos)
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  
  TRACE_FUNCTION ("qAT_PlusCGQREQ()");

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
  if ( pdp_context_get_state_for_cid( cid ) EQ PDP_CONTEXT_STATE_DEFINED )
  {
    p_pdp_context_node = pdp_context_find_node_from_cid( cid );
    if( p_pdp_context_node )
    {
      if( p_pdp_context_node->ctrl_qos EQ PS_is_R97 )
      {
        memcpy( qos, &p_pdp_context_node->qos.qos_r97, sizeof(T_PS_qos_r97) ); 
      }
      else
      {
        if( !cl_qos_convert_r99_to_r97( &p_pdp_context_node->qos.qos_r99, &qos->qos_r97 ) )
        {
          /* Failed to convert to Release 97. Never end here !!!! */
          return( AT_FAIL );
        }
      }
      *qos_valid = TRUE;
    }
    else
    {
      TRACE_ERROR( "ERROR: PDP context not found" );
      *qos_valid = FALSE;
    }
  } 
  else
  {
    *qos_valid = FALSE;
  }

  return( AT_CMPL );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMQ                  |
| STATE   : finished              ROUTINE : qAT_PlusCGQMIN           |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CGQMIN? AT
            command and returns current settings for the specified
            PDP context. The function is called for all cids.
            Note: The QoS returned is always in Release 97 format.
  RETURNS:  - AT_CMPL : Completed.
            - AT_FAIL : Command not valid for srcId.
  UPDATES:  - qos: Quality of service for cid. Not updated if cid is undefined.
            - qos_valid: Indicates whether qos is updated not.
*/
GLOBAL T_ACI_RETURN qAT_PlusCGQMIN ( T_ACI_CMD_SRC srcId, U8 cid, BOOL *qos_valid, T_PS_qos *qos)
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION ("qAT_PlusCGQMIN()");

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
  if ( pdp_context_get_state_for_cid( cid ) EQ PDP_CONTEXT_STATE_DEFINED )
  {
    p_pdp_context_node = pdp_context_find_node_from_cid( cid );
    
    if( p_pdp_context_node )
    {
      if( p_pdp_context_node->ctrl_min_qos EQ (T_PS_ctrl_min_qos)PS_is_R97 )
      {
        memcpy( qos, &p_pdp_context_node->min_qos, sizeof(T_PS_qos) ); 
      }
      else
      {
        if( !cl_qos_convert_r99_to_r97( &p_pdp_context_node->min_qos.qos_r99, &qos->qos_r97 ) )
        {
          /* Failed to convert to Release 97. Never end here !!!! */
          return( AT_FAIL );
        }
      }
      *qos_valid = TRUE;
    }
    else
    {
      TRACE_ERROR( "ERROR: PDP context not found" );
      *qos_valid = FALSE;
    }
    
  }
  else
  {
    *qos_valid = FALSE;
  }
   return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMQ                  |
| STATE   : finished              ROUTINE : qAT_PlusCGDCONT          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CGCONT? AT
            command which returns current settings for each defined PDP context.
*/

GLOBAL T_ACI_RETURN qAT_PlusCGDCONT( T_ACI_CMD_SRC srcId, T_PDP_CONTEXT *p_pdp_context_array, SHORT *cid_array )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  int                     i = 0;
  
  TRACE_FUNCTION ("qAT_PlusCGDSCONT()");

/*
 *-------------------------------------------------------------------
 * check command source - should be Serial link ?
 *-------------------------------------------------------------------
 */
   if ( !cmh_IsVldCmdSrc( srcId ) )
   {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
   }

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */

  p_pdp_context_node = p_pdp_context_list;

  while( p_pdp_context_node AND i < PDP_CONTEXT_CID_MAX )
  {

    if( p_pdp_context_node->type EQ PDP_CONTEXT_TYPE_PRIMARY )
    {

      memcpy( &p_pdp_context_array[i], &p_pdp_context_node->attributes, sizeof(T_PDP_CONTEXT) );

//      memcpy( &p_pdp_context_array[i].pdp_type, &p_pdp_context_node->attributes.pdp_type, sizeof(T_PDP_CONTEXT_PDP_TYPE) );
//      memcpy( &p_pdp_context_array[i].pdp_apn,  &p_pdp_context_node->attributes.pdp_apn,  sizeof(T_PDP_CONTEXT_APN) );
//      memcpy( &p_pdp_context_array[i].pdp_addr, &p_pdp_context_node->attributes.pdp_addr, sizeof(T_PDP_CONTEXT_ADDR) );
      
      cid_array[i] = p_pdp_context_node->cid;
      i++;
      
    }
    
    p_pdp_context_node = p_pdp_context_node->p_next;

  }

  return( AT_CMPL );
}


/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : CMH_SMQ                  |
| STATE   :                       ROUTINE : qAT_PlusCGDSCONT         |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CGCONT? AT
            command which returns current settings for each defined PDP context.
*/

GLOBAL T_ACI_RETURN qAT_PlusCGDSCONT( T_ACI_CMD_SRC srcId, T_PDP_CONTEXT *p_pdp_context_array, U8 *cid_array )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  int                     i = 0;
  
  TRACE_FUNCTION ("qAT_PlusCGDSCONT()");

/*
 *-------------------------------------------------------------------
 * check command source - should be Serial link ?
 *-------------------------------------------------------------------
 */
   if ( !cmh_IsVldCmdSrc( srcId ) )
   {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
   }

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */

  p_pdp_context_node = p_pdp_context_list;

  while( p_pdp_context_node AND i < PDP_CONTEXT_CID_MAX )
  {

    if( p_pdp_context_node->type EQ PDP_CONTEXT_TYPE_SECONDARY )
    {
      p_pdp_context_array[i].p_cid  = p_pdp_context_node->attributes.p_cid;
      p_pdp_context_array[i].d_comp = p_pdp_context_node->attributes.d_comp;
      p_pdp_context_array[i].h_comp = p_pdp_context_node->attributes.h_comp;

      cid_array[i] = p_pdp_context_node->cid;
      i++;
      
    }
    
    p_pdp_context_node = p_pdp_context_node->p_next;

  }

   return( AT_CMPL );
}

GLOBAL T_ACI_RETURN qAT_PlusCGTFT( T_ACI_CMD_SRC srcId, U8 *cid_array )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  int                     i = 0;
  
  TRACE_FUNCTION ("qAT_PlusCGTFT()");

/*
 *-------------------------------------------------------------------
 * check command source - should be Serial link ?
 *-------------------------------------------------------------------
 */
   if ( !cmh_IsVldCmdSrc( srcId ) )
   {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
   }

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */

  p_pdp_context_node = p_pdp_context_list;

  while( p_pdp_context_node AND i < PDP_CONTEXT_CID_MAX )
  {
    if( p_pdp_context_node->p_tft_pf )
    {
      cid_array[i++] = p_pdp_context_node->cid;
    }
    
    p_pdp_context_node = p_pdp_context_node->p_next;

  }

   return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMQ                  |
| STATE   : finished              ROUTINE : qAT_PlusCGACT            |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CGACT? AT
            command which returns current activation states for all defined PDP context.
*/

GLOBAL T_ACI_RETURN qAT_PlusCGACT ( T_ACI_CMD_SRC srcId, BOOL *states, SHORT *cid )
{
  T_PDP_CONTEXT_STATE state = PDP_CONTEXT_STATE_INVALID;
  U8 i = 0, 
     j = 0;

  TRACE_FUNCTION ("qAT_PlusCGACT()");

/*
 *-------------------------------------------------------------------
 * check command source - should be Serial link ?
 *-------------------------------------------------------------------
 */
   if ( !cmh_IsVldCmdSrc (srcId) ) {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
   }

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */
  for ( i=0; i < PDP_CONTEXT_CID_MAX; i++ )
  {
    state = get_state_over_cid( (U8)(i+1) );
    /*if ( state NEQ  PDP_CONTEXT_STATE_UNDEFINED AND state NEQ  PDP_CONTEXT_STATE_INVALID )*/
    if ( state NEQ  PDP_CONTEXT_STATE_INVALID )
    {
      if ( state EQ PDP_CONTEXT_STATE_ACTIVATED OR state EQ PDP_CONTEXT_STATE_DATA_LINK )
        states[j] = TRUE;
      else
        states[j] = FALSE;

      cid[j] = i + 1 ;
      j++;
    }
  }
  cid [j] = PDP_CONTEXT_CID_INVALID;

   return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMQ                  |
| STATE   : finished              ROUTINE : qAT_PlusCGAUTO           |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CGAUTO? AT
            command which returns current mode of automatic response
            to network requests.
*/

GLOBAL T_ACI_RETURN qAT_PlusCGAUTO  ( T_ACI_CMD_SRC srcId, T_CGAUTO_N *n)
{

  TRACE_FUNCTION ("qAT_PlusCGAUTO()");

/*
 *-------------------------------------------------------------------
 * check command source - should be Serial link ?
 *-------------------------------------------------------------------
 */
   if ( !cmh_IsVldCmdSrc (srcId) ) {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
   }

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */
  *n = (T_CGAUTO_N) automatic_response_mode;

  return( AT_CMPL );
}


GLOBAL T_ACI_RETURN qAT_PlusCGEREP  ( T_ACI_CMD_SRC srcId, T_CGEREP_MODE *mode, T_CGEREP_BFR *bfr )
{

  TRACE_FUNCTION ("qAT_PlusCGEREP()");

/*
 *-------------------------------------------------------------------
 * check command source - should be Serial link ?
 *-------------------------------------------------------------------
 */
   if ( !cmh_IsVldCmdSrc (srcId) ) {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
   }

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */
  *mode = sm_cgerep_mode;
  *bfr  = sm_cgerep_bfr;

  return( AT_CMPL );
}

GLOBAL T_ACI_RETURN qAT_PlusCGSMS( T_ACI_CMD_SRC srcId, T_CGSMS_SERVICE *service )
{

  TRACE_FUNCTION ("qAT_PlusCGSMS()");

/*
 *-------------------------------------------------------------------
 * check command source - should be Serial link ?
 *-------------------------------------------------------------------
 */
   if ( !cmh_IsVldCmdSrc (srcId) ) {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
   }

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */
  *service = sm_cgsms_service;

  return( AT_CMPL );
}

#ifdef REL99
/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : CMH_SMQ                  |
| STATE   : finished              ROUTINE : qAT_PlusCGEQREQ          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CGEQREG? AT
            command and returns current settings for the specified
            PDP context. The function is called for all cids.
            Note: The QoS returned is always in Release 99 format (3GPP).
  RETURNS:  - AT_CMPL : Completed.
            - AT_FAIL : Command not valid for srcId.
  UPDATES:  - qos: Quality of service for cid. Not updated if cid is undefined.
            - qos_valid: Indicates whether qos is updated not.
*/
GLOBAL T_ACI_RETURN qAT_PlusCGEQREQ( T_ACI_CMD_SRC srcId, U8 cid, BOOL *qos_valid, T_PS_qos *qos)
{ 
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION ("qAT_PlusCGEQREQ()");

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );
  if( !p_pdp_context_node )
  {
    *qos_valid = FALSE;
    return( AT_CMPL );
  }

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
  if( get_state_over_cid(cid) EQ PDP_CONTEXT_STATE_DEFINED )
  {
    if( p_pdp_context_node->ctrl_qos EQ PS_is_R99 )
    {
      memcpy( qos, &p_pdp_context_node->qos, sizeof(T_PS_qos_r99) ); 
    }
    else
    {
      /* The QoS is in Release 97 format and must be converted first. */    
      if( !cl_qos_convert_r97_to_r99( &p_pdp_context_node->qos.qos_r97, &(qos->qos_r99)) )
      {
        /* Failed to convert to Release 99. Never end here !!!! */
        return( AT_FAIL );
      }
    }
    *qos_valid = TRUE;
  }
  else
  {
    *qos_valid = FALSE;
  }

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : CMH_SMQ                  |
| STATE   : finished              ROUTINE : qAT_PlusCGEQMIN          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CGEQMIN? AT
            command and returns current settings for the specified
            PDP context. The function is called for all cids.
            The QoS returned is always in Release 99 format (3GPP).
  RETURNS:  - AT_CMPL : Completed.
            - AT_FAIL : Command not valid for srcId.
  UPDATES:  - qos: Quality of service for cid. Not updated if cid is undefined.
            - qos_valid: Indicates whether qos is updated not.
*/
GLOBAL T_ACI_RETURN qAT_PlusCGEQMIN( T_ACI_CMD_SRC srcId, U8 cid, BOOL *qos_valid, T_PS_min_qos *qos)
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION ("qAT_PlusCGEQMIN()");

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );
  if( !p_pdp_context_node )
  {
    *qos_valid = FALSE;
    return( AT_CMPL );
  }
  

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
  if ( get_state_over_cid(cid) EQ PDP_CONTEXT_STATE_DEFINED )
  {
    if( p_pdp_context_node->ctrl_min_qos EQ (T_PS_ctrl_min_qos)PS_is_R99 )
    {
      memcpy( qos, &p_pdp_context_node->min_qos, sizeof(T_PS_qos) ); 
    }
    else
    {
      /* The QoS is in Release 97 format and must be converted first. */    
      if( !cl_qos_convert_r97_to_r99(&p_pdp_context_node->min_qos.qos_r97, &qos->qos_r99) )
      {
        /* Failed to convert to Release 99. Never end here !!!! */
        return( AT_FAIL );
      }
    }
    *qos_valid = TRUE;
  }
  else
  {
    *qos_valid = FALSE;
  }

  return( AT_CMPL );
}
#endif /* REL99 */


#endif  /* GPRS */
/*==== EOF ========================================================*/
