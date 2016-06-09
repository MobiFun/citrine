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
|  Purpose :  This module defines the signalling functions of the
|             protocol stack adapter for the registration part of
|             GPRS session management.
+----------------------------------------------------------------------------- 
*/ 

#ifdef GPRS

#ifndef PSA_SMS_C
#define PSA_SMS_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci.h"
#include "psa.h"

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "gaci_cmh.h"
#include "psa_gmm.h"
#include "psa_sm.h"
#include "cmh.h"
#include "gaci.h"
#include "gaci_cmh.h"
#include "cmh_sm.h"

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_SMS                 |
| STATE   : finished              ROUTINE : psaSM_ActivateReq       |
+-------------------------------------------------------------------+

  PURPOSE : MS initiates a primary PDP context activation

*/
GLOBAL void psaSM_smreg_pdp_activate_req( U8  cid,                                     
                                     U8  hcomp,
                                     U8  dcomp)
{

  U8     no_of_tft_pf = 0;
  USHORT PCO_length   = 0;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  T_TFT_INTERNAL         *p_tft_pf_node      = NULL;
  UBYTE gprs_default_pco[] = {
                                0x80,0x80,0x21,0x10,0x01,0x01,0x00,0x10,0x81,0x06,
                                0x00,0x00,0x00,0x00,0x83,0x06,0x00,0x00,0x00,0x00
                             };


  TRACE_FUNCTION ("psaSM_smreg_pdp_activate_req()");

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );
  if( !p_pdp_context_node )
  {
    TRACE_ERROR( "ERROR: PDP context not found, invalid cid");
    return;
  }

  /* As in the previous Berlin code the default user_pco.len is 160 
   * This is not present in Alborg code, but as we have to be consitant with our
   * prious implementation we have to set the user_pco some default value 
   */
  if( p_pdp_context_node->internal_data.user_pco.len EQ 0 )
  {
    p_pdp_context_node->internal_data.user_pco.len = sizeof (gprs_default_pco);
    memcpy (p_pdp_context_node->internal_data.user_pco.pco, gprs_default_pco, sizeof (gprs_default_pco));
  }

  PCO_length   = p_pdp_context_node->internal_data.user_pco.len << 3;
  no_of_tft_pf = pdp_context_get_no_of_tft_pfs( p_pdp_context_node->cid );

/*
 *-------------------------------------------------------------------
 * create and send primitive for context activation
 *-------------------------------------------------------------------
 */
   
  {
    PALLOC_SDU( p_smreg_pdp_activate_req, SMREG_PDP_ACTIVATE_REQ, PCO_length );

    p_smreg_pdp_activate_req -> comp_params.hcomp       = hcomp;
    p_smreg_pdp_activate_req -> comp_params.dcomp       = dcomp;

    p_smreg_pdp_activate_req -> pdp_type    = cmhSM_Get_pdp_type();
    p_smreg_pdp_activate_req -> nsapi       = (U8)CID_TO_NSAPI( cid );
    p_smreg_pdp_activate_req -> ti          = gprs_call_table[current_gprs_ct_index].sm_ind.ti;

    /* Set the control for the UNIONs sm_qos and sm_min_qos */

    cmhSM_Get_QOS        ( &p_smreg_pdp_activate_req -> qos );
    cmhSM_Get_QOS_min    ( &p_smreg_pdp_activate_req -> min_qos );
    cmhSM_Get_pdp_address( &p_smreg_pdp_activate_req -> ip_address, &p_smreg_pdp_activate_req->ctrl_ip_address);
    cmhSM_Get_smreg_apn  ( &p_smreg_pdp_activate_req -> apn );


    p_smreg_pdp_activate_req -> ctrl_qos     = p_pdp_context_node->ctrl_qos;
    p_smreg_pdp_activate_req -> ctrl_min_qos = p_pdp_context_node->ctrl_min_qos;
    
    /*
     * At this point the TFT should be added, waiting for a new frame release.
     * remove the line below !!!!!!!   "no_of_tft_pf = 0;"
     */

    /*no_of_tft_pf = 0;*/
    
    if( no_of_tft_pf )
    {
      p_smreg_pdp_activate_req -> v_tft = TRUE;
      p_smreg_pdp_activate_req -> tft.c_tft_pf   = no_of_tft_pf;
      p_smreg_pdp_activate_req -> tft.ptr_tft_pf = (T_NAS_tft_pf*) DRP_ALLOC( no_of_tft_pf * sizeof( T_NAS_tft_pf ), 0 );
    }
    else
    {
      p_smreg_pdp_activate_req -> v_tft = FALSE;
    }
      
    no_of_tft_pf = 0;
    
    p_tft_pf_node = p_pdp_context_node -> p_tft_pf;
    while( p_tft_pf_node AND p_smreg_pdp_activate_req -> v_tft )
    {
      memcpy( &p_smreg_pdp_activate_req->tft.ptr_tft_pf[no_of_tft_pf], &p_tft_pf_node->pf_attributes, sizeof( T_NAS_tft_pf ) );
      no_of_tft_pf++;
      p_tft_pf_node = p_tft_pf_node->p_next;
    }
    p_smreg_pdp_activate_req -> sdu.l_buf = PCO_length;
    p_smreg_pdp_activate_req -> sdu.o_buf = 0;

    if ( PCO_length )
    {
      memcpy( &p_smreg_pdp_activate_req -> sdu.buf,
              &p_pdp_context_node->internal_data.user_pco.pco,
               p_pdp_context_node->internal_data.user_pco.len );
    }

    psaGMM_NetworkRegistrationStatus( SMREG_PDP_ACTIVATE_REQ, p_smreg_pdp_activate_req );

    PSEND( hCommSM, p_smreg_pdp_activate_req );
  }
  return;
}

/*
+--------------------------------------------------------------------------+
| PROJECT : UMTS                MODULE  : PSA_SMS                          |
| STATE   : -                   ROUTINE : psaSM_smreg_pdp_activate_sec_req |
+--------------------------------------------------------------------------+

  PURPOSE : MS initiates a secondary PDP context activation

*/
#ifdef REL99
GLOBAL void psaSM_smreg_pdp_activate_sec_req( U8  cid )
                                              /*,
                                              U32 dti_linkid, 
                                              U8  dti_neighbor[NAS_SIZE_ENTITY_NAME], 
                                              U8  dti_direction )*/
{

  T_PDP_CONTEXT_INTERNAL       *p_pdp_context_prim_node = NULL;
  T_PDP_CONTEXT_INTERNAL       *p_pdp_context_sec_node  = NULL;
  T_TFT_INTERNAL               *p_tft_pf_node           = NULL;
  T_SMREG_PDP_ACTIVATE_SEC_REQ *p_smreg_pdp_activate_sec_req = NULL;
  U8                            no_of_tft_pf = 0;
  
  TRACE_FUNCTION( "psaSM_smreg_pdp_activate_sec_req()");


  /* Find secondary PDP context */
  p_pdp_context_sec_node = pdp_context_find_node_from_cid( cid );
  if( !p_pdp_context_sec_node )
  {
    TRACE_ERROR( "ERROR: Secondary PDP context not found, invalid cid");
    return;
  }

  /* Find primary PDP context */
  p_pdp_context_prim_node = pdp_context_find_node_from_cid( p_pdp_context_sec_node->attributes.p_cid );
  if( !p_pdp_context_prim_node )
  {
    TRACE_ERROR( "ERROR: Primary PDP context not found, invalid cid");
    return;
  }

  no_of_tft_pf = pdp_context_get_no_of_tft_pfs( p_pdp_context_sec_node->cid );
  
  p_smreg_pdp_activate_sec_req = DRPO_ALLOC( SMREG_PDP_ACTIVATE_SEC_REQ, (no_of_tft_pf * sizeof(T_NAS_tft_pf)) );

  p_smreg_pdp_activate_sec_req->nsapi              = (U8)CID_TO_NSAPI( cid );      
  p_smreg_pdp_activate_sec_req->pri_nsapi          = (U8)CID_TO_NSAPI( p_pdp_context_prim_node->cid ); /* !!! From primary PDP context */
  p_smreg_pdp_activate_sec_req->comp_params.dcomp  = p_pdp_context_sec_node->attributes.d_comp;
  p_smreg_pdp_activate_sec_req->comp_params.hcomp  = p_pdp_context_sec_node->attributes.h_comp;
  p_smreg_pdp_activate_sec_req->ti                 = p_pdp_context_sec_node->internal_data.smreg_ti;

  p_smreg_pdp_activate_sec_req->ctrl_qos           = p_pdp_context_sec_node->ctrl_qos;
  p_smreg_pdp_activate_sec_req->qos                = p_pdp_context_sec_node->qos;
  p_smreg_pdp_activate_sec_req->ctrl_min_qos       = p_pdp_context_sec_node->ctrl_min_qos;
  p_smreg_pdp_activate_sec_req->min_qos            = p_pdp_context_sec_node->min_qos;

  if( no_of_tft_pf )
  {
    p_smreg_pdp_activate_sec_req -> v_tft = TRUE;
    p_smreg_pdp_activate_sec_req -> tft.c_tft_pf   = no_of_tft_pf;    
    p_smreg_pdp_activate_sec_req -> tft.ptr_tft_pf = (T_NAS_tft_pf*) DP_ALLOC( (no_of_tft_pf * sizeof( T_NAS_tft_pf )), p_smreg_pdp_activate_sec_req, 0 );
   
  }
  else
  {
    p_smreg_pdp_activate_sec_req->v_tft = FALSE;
  }
    
  no_of_tft_pf = 0;

  p_tft_pf_node = p_pdp_context_sec_node->p_tft_pf;
  while( p_tft_pf_node )
  {
    memcpy( &p_smreg_pdp_activate_sec_req->tft.ptr_tft_pf[no_of_tft_pf], &p_tft_pf_node->pf_attributes, sizeof( T_NAS_tft_pf ) );
    no_of_tft_pf++;
    p_tft_pf_node = p_tft_pf_node->p_next;
  }
    
  psaGMM_NetworkRegistrationStatus( SMREG_PDP_ACTIVATE_SEC_REQ, p_smreg_pdp_activate_sec_req );

  PSEND( hCommSM, p_smreg_pdp_activate_sec_req );

}
#endif


/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_SMS                 |
| STATE   : finished              ROUTINE : psaSM_PDP_Deactivate    |
+-------------------------------------------------------------------+

  PURPOSE : MS initiates a PDP context deactivation

*/
GLOBAL void psaSM_PDP_Deactivate( USHORT nsapi_set, UBYTE smreg_local )
{

  TRACE_FUNCTION ("psaSM_PDP_Deactivate()");

/*
 *-------------------------------------------------------------------
 * create and send primitive for context deactivation
 *-------------------------------------------------------------------
 */
  {
    PALLOC (smreg_pdp_deactivate_req, SMREG_PDP_DEACTIVATE_REQ);

    smreg_pdp_deactivate_req -> nsapi_set   = nsapi_set;
    smreg_pdp_deactivate_req -> rel_ind     = smreg_local;//PS_REL_IND_YES;

    PSEND (hCommSM, smreg_pdp_deactivate_req);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_SMS                 |
| STATE   : finished              ROUTINE : psaSM_PDP_No_activate   |
+-------------------------------------------------------------------+

  PURPOSE : GACI is not able to set up another context

*/
GLOBAL void psaSM_PDP_No_activate ( UBYTE smreg_ti, USHORT smreg_cause )
{
  TRACE_FUNCTION ("psaSM_PDP_No_activate()");

/*
 *-------------------------------------------------------------------
 *  create and send primitive for rejection 
 *  the network requested context activation
 */
  {
    PALLOC (smreg_pdp_activate_rej_rsp, SMREG_PDP_ACTIVATE_REJ_RES);

    /* fill in primitive parameter: registration mode */
    smreg_pdp_activate_rej_rsp -> ti          = smreg_ti;
    smreg_pdp_activate_rej_rsp->ps_cause.ctrl_value = CAUSE_is_from_nwsm;
    smreg_pdp_activate_rej_rsp->ps_cause.value.sm_cause = (unsigned char)smreg_cause;

    PSEND (hCommSM, smreg_pdp_activate_rej_rsp);
  }

}


/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_SMS                 |
| STATE   : finished              ROUTINE : psaSM_PDP_Modify        |
+-------------------------------------------------------------------+

  PURPOSE : MS initiates a PDP context modification.

*/
GLOBAL void psaSM_PDP_Modify( void )
{
  T_SMREG_PDP_MODIFY_REQ *p_smreg_pdp_modify_req = NULL;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node     = NULL;
  T_TFT_INTERNAL         *p_tft_node             = NULL;
  U8                      tft_pf_count           = 0;
  

  TRACE_FUNCTION ("psaSM_PDP_Modify()");

  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[cid_pointer] );
  if( ! p_pdp_context_node )
  {
    TRACE_ERROR( "ERROR: PDP context not found, in function psaSM_PDP_Modify" );
    return;
  }

/*
 *-------------------------------------------------------------------
 * send primitive for modify context
 *-------------------------------------------------------------------
 */
   
  p_smreg_pdp_modify_req = DRPO_ALLOC( SMREG_PDP_MODIFY_REQ, 0 );

  /* fill in primitive parameter: registration mode */
  p_smreg_pdp_modify_req -> nsapi        = (U8)CID_TO_NSAPI( work_cids[cid_pointer] );
  p_smreg_pdp_modify_req -> ctrl_qos     = p_pdp_context_node->ctrl_qos;
  p_smreg_pdp_modify_req -> ctrl_min_qos = p_pdp_context_node->ctrl_min_qos;
  p_smreg_pdp_modify_req -> v_tft        = FALSE;
  cmhSM_Get_QOS    ( &p_smreg_pdp_modify_req -> qos );  
  cmhSM_Get_QOS_min( &p_smreg_pdp_modify_req -> min_qos );
  
  if( p_pdp_context_node -> tft_changed )
  {
    // Set the tft_changed bool to false since we are modifying the pdp context now.
    p_pdp_context_node -> tft_changed = FALSE;
  
    p_tft_node = p_pdp_context_node -> p_tft_pf;
 
    while( p_tft_node )
    {
      tft_pf_count ++;
      p_tft_node = p_tft_node -> p_next;
    }

    if( tft_pf_count )
    {
      p_smreg_pdp_modify_req->v_tft = TRUE;
      p_smreg_pdp_modify_req->tft.c_tft_pf   = tft_pf_count;
      p_smreg_pdp_modify_req->tft.ptr_tft_pf = (T_NAS_tft_pf*) DP_ALLOC( tft_pf_count * sizeof( T_NAS_tft_pf ), p_smreg_pdp_modify_req, 0 );
    }

    tft_pf_count = 0;
   
    p_tft_node = p_pdp_context_node -> p_tft_pf;
    while( p_tft_node )
    {
      memcpy( &p_smreg_pdp_modify_req->tft.ptr_tft_pf[tft_pf_count],
              p_tft_node,
              sizeof( T_TFT_INTERNAL ) );

      p_tft_node = p_tft_node->p_next;
      tft_pf_count++;
  
    }
  }

  PSEND( hCommSM, p_smreg_pdp_modify_req );

  return;
}



#endif  /* GPRS */
/*==== EOF ========================================================*/

