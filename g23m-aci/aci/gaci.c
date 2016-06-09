/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  J:\g23m-aci\aci\gaci.c
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
|  Purpose :  
+----------------------------------------------------------------------------- 
*/ 
#if defined (GPRS) && defined (DTI)

#ifndef GACI_C
#define GACI_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "dti_conn_mng.h"

#include "gaci.h"
#include "gaci_cmh.h"
#include "psa.h"
#include "psa_sm.h"
#include "psa_gppp.h"
#include "psa_gmm.h"

#include "cmh.h"
#ifdef SIM_TOOLKIT
#include "psa_cc.h"
#include "psa_sat.h"
#include "cmh_sat.h"
#endif /* SIM_TOOLKIT */
#include "cmh_sm.h"
#include "cmh_gppp.h"
#include "cmh_gmm.h"
#include "gaci_srcc.h"

#include "aci_mem.h"

/*==== CONSTANTS ==================================================*/
static T_ACI_CMD_SRC _ATZ_srcId;

#ifdef FF_SAT_E
static USHORT SAT_error_cause = SAT_GPRS_INV_CAUSE;
#endif /* FF_SAT_E */

/*==== EXPORT =====================================================*/
GLOBAL T_ACI_CMD_SRC cmhSM_getSrcIdOfRunningCGACTDeactivation(U8 cid);

/*==== VARIABLES ==================================================*/
T_PDP_CONTEXT_INTERNAL *p_pdp_context_list;


/*==== PROTOTYPES =================================================*/
GLOBAL T_PDP_CONTEXT_INTERNAL *pdp_context_create_node( U8 cid );
GLOBAL int                     pdp_context_remove_node( U8 cid );
GLOBAL T_PDP_CONTEXT_INTERNAL *pdp_context_find_node_from_cid( U8 cid );
GLOBAL T_PDP_CONTEXT_INTERNAL *pdp_context_find_node_from_dti_id( U8 dti_id );
GLOBAL T_PDP_CONTEXT_INTERNAL *pdp_context_find_last_node   ( void );
#ifdef REL99
GLOBAL T_TFT_INTERNAL         *pdp_context_add_tft_pf       ( U8 cid, U8 tft_pf_id );
GLOBAL BOOL                    pdp_context_del_tft_pf       ( U8 cid, U8 tft_pf_id );
GLOBAL T_TFT_INTERNAL         *pdp_context_find_tft_pf      ( U8 cid, U8 tft_pf_id );
GLOBAL U8                      pdp_context_get_no_of_tft_pfs( U8 cid );
GLOBAL void                    pdp_context_clear_tft_active_list( U8 cid );
//GLOBAL T_TFT_MODIFICATION      pdp_context_compare_tft      ( U8 cid );
//GLOBAL T_TFT_MODIFICATION      pdp_context_get_modification_action( U8 cid );
#endif /* REL99 */
GLOBAL U8                      pdp_context_check_if_nodes_exists( U8 *cid_array );
GLOBAL U8                      pdp_context_validate_pdp_type    ( U8 *cid_array, T_PDP_TYPE pdp_type );
/*=================================================================*/



/*
+----------------------------------------------------------------------+
| PROJECT :                       MODULE  : gaci                       |
|                                 ROUTINE :                            |
+----------------------------------------------------------------------+

  PURPOSE :
*/

GLOBAL void gaci_init ( void )
{
  /* Init of intern variable */
  _ATZ_srcId = CMD_SRC_NONE;

  /* GPRS Init */
  gpppEntStat.curCmd = AT_CMD_NONE;
  gpppEntStat.entOwn = CMD_SRC_NONE;

  cmhGMM_Init();
  cmhSM_Init();

  srcc_init();

  gaci_reset();
}

GLOBAL void gaci_reset( void )
{
  cmhSM_Reset();
}

GLOBAL void gaci_ATZ_reset( void )
{
  cmhSM_ResetNonWorkingContexts();
}

GLOBAL void gaci_finit ( void )
{
  /* here will be a functionality */
}

EXTERN T_ACI_RETURN sGsmAT_Z ( T_ACI_CMD_SRC srcId );

GLOBAL T_ACI_RETURN sGprsAT_Z ( T_ACI_CMD_SRC srcId )
{
  SHORT cid_array[1] = { PDP_CONTEXT_CID_INVALID };

 /*
  *-------------------------------------------------------------------
  *   rejects waiting network requests for PDP context activation
  *-------------------------------------------------------------------
  */
  if ( ( at.rngPrms.isRng EQ TRUE ) AND ( at.rngPrms.mode EQ CRING_MOD_Gprs) ) /* GPRS call */
  {
   /*
    *   brz patch: In the case of context reactivation over SMREG_PDP_ACTIVATE_IND with an used ti
    *              the GPRS ATZ command doesn't do anything!
    *
    *   Why?       Because the Windows Dial-Up Networking client send every time an ATZ after termination
    *              of the connection and with this a context reactivation was impossible. 
    */
    if ( gprs_call_table[current_gprs_ct_index].reactivation EQ GCTT_NORMAL )
    {
      sAT_PlusCGANS(srcId, CGANS_RESPONSE_REJECT, NULL, PDP_CONTEXT_CID_OMITTED );
    }
    else
    { /* Reactivation: stop GPRS ATZ */
      return sGsmAT_Z ( srcId );
    }
  }
  if ( AT_EXCT EQ sAT_PlusCGACT ( srcId, CGACT_STATE_DEACTIVATED, cid_array ))
  {
    _ATZ_srcId = srcId;    /* hold source Id */
    return( AT_EXCT );
  }
  
  srcId_cb = srcId;
  gaci_ATZ_reset();
  return sGsmAT_Z ( srcId );
}

LOCAL void endOfGprsAT_Z ( void )
{
  srcId_cb = _ATZ_srcId;
  gaci_ATZ_reset();
  if ( AT_CMPL EQ sGsmAT_Z ( _ATZ_srcId ) )
  {
    R_AT( RAT_OK, _ATZ_srcId ) ( AT_CMD_Z );

    /* log result */
    cmh_logRslt ( _ATZ_srcId,
                  RAT_OK, AT_CMD_Z, -1, BS_SPEED_NotPresent, CME_ERR_NotPresent );
  }

  _ATZ_srcId = CMD_SRC_NONE;
}

GLOBAL BOOL gaci_isATZcmd ( void )
{
  if ( _ATZ_srcId NEQ CMD_SRC_NONE )
  {
    endOfGprsAT_Z();
    return TRUE;
  }
  
  return FALSE;
}

/*
+----------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : gaci                       |
|                                 ROUTINE : gaci_get_cid_over_dti_id   |
+----------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL SHORT gaci_get_cid_over_dti_id( UBYTE dti_id )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
 
  p_pdp_context_node = p_pdp_context_list;
  while( p_pdp_context_node )
  {
  	/* Check for the link_id_new also because it will be filled for TCPIP call */
    if ( dti_id EQ EXTRACT_DTI_ID(p_pdp_context_node->internal_data.link_id) OR
         dti_id EQ EXTRACT_DTI_ID(p_pdp_context_node->internal_data.link_id_uart) OR
         dti_id EQ EXTRACT_DTI_ID(p_pdp_context_node->internal_data.link_id_new))

      return p_pdp_context_node->cid;

    p_pdp_context_node = p_pdp_context_node->p_next;
  }

  return PDP_CONTEXT_CID_INVALID;
}


GLOBAL SHORT gaci_get_cid_over_link_id ( T_DTI_CONN_LINK_ID  link_id )
{
  return gaci_get_cid_over_dti_id((UBYTE)EXTRACT_DTI_ID(link_id));
}



/*
+----------------------------------------------------------------------+
| PROJECT : -                     MODULE  : gaci                       |
|                                 ROUTINE : gaci_get_link_id_over_peer |
+----------------------------------------------------------------------+

  PURPOSE : */

/*
 *  Assumption: there is only one connection between UPM and the peer
 */
GLOBAL T_DTI_CONN_LINK_ID gaci_get_link_id_over_peer ( T_DTI_ENTITY_ID entity_id )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  p_pdp_context_node = p_pdp_context_list;

  while( p_pdp_context_node )
  {
    if( p_pdp_context_node->internal_data.entity_id EQ entity_id AND
        p_pdp_context_node->internal_data.link_id NEQ DTI_LINK_ID_NOTPRESENT ) 
    {
      return p_pdp_context_node->internal_data.link_id;
    }
  
    p_pdp_context_node = p_pdp_context_node->p_next;
  }
  return DTI_LINK_ID_NOTPRESENT;
}



#ifdef FF_SAT_E
GLOBAL void gaci_SAT_err(USHORT cause)
{
  SAT_error_cause = cause;
}
#endif /* FF_SAT_E */

GLOBAL void gaci_RAT_caller ( SHORT rat_id, SHORT cid, UBYTE cmdBuf, UBYTE cme_err )
{
  T_ACI_CMD_SRC  rat_owner = (T_ACI_CMD_SRC)get_owner_over_cid( (U8)cid );

  TRACE_FUNCTION("gaci_RAT_caller()");

#ifdef FF_SAT_E
  if ( !cmhSAT_OpChnGPRSPend( cid, OPCH_NONE ))
#endif /* FF_SAT_E */
  {
    switch ( rat_id )
    {
      case RAT_OK:
        R_AT( RAT_OK, rat_owner ) ( cmdBuf );
        break;
      case RAT_CME:
        ACI_ERR_DESC( ACI_ERR_CLASS_Cme, cme_err );     /* align aciErrDesc to cme_err */
        R_AT( RAT_CME, rat_owner ) ( cmdBuf, cme_err );
        /* log result */
        cmh_logRslt ( (T_ACI_CMD_SRC) rat_owner, RAT_CME, (T_ACI_AT_CMD) cmdBuf, 
                                     -1, BS_SPEED_NotPresent, (T_ACI_CME_ERR)cme_err );
        break;
      case RAT_NO_CARRIER:
        if (!(cmhSM_getSrcIdOfRunningCGACTDeactivation((U8)cid) EQ rat_owner))
        {
          R_AT( RAT_NO_CARRIER, rat_owner ) ( cmdBuf, 0 );
        }
        /* log result */
        cmh_logRslt ( (T_ACI_CMD_SRC) rat_owner, RAT_NO_CARRIER, (T_ACI_AT_CMD)cmdBuf, 
                                            (SHORT) 0, BS_SPEED_NotPresent,CME_ERR_NotPresent );
        break;
    }
  }
#ifdef FF_SAT_E
  else
  {
    /*
     *    SIM callback for SAT-class CE 
     */
    switch ( rat_id )
    {
      case RAT_OK:
        /* connection deactivated */
        cmhSAT_OpChnGPRSStat(SAT_GPRS_ACT, SAT_GPRS_INV_CAUSE); /* no cause given by primitive */
        break;
      case RAT_CME:
        if ( cmdBuf EQ AT_CMD_CGDATA )
        { /* 
           * Attach before ATD (UPM <-> IP <-> UDP <-> SIM) fails 
           */
          cmhSAT_OpChnGPRSStat(SAT_GPRS_ATT_FAILED, (UBYTE)SAT_error_cause);  
        }
        else
        { /* activate connection UPM <-> SIM fails */
          cmhSAT_OpChnGPRSStat(SAT_GPRS_ACT_FAILED, (UBYTE)SAT_error_cause);  
        }
        break;
      case RAT_NO_CARRIER:
          /* activate connection UPM <-> IP <-> UDP <-> SIM fails */
          cmhSAT_OpChnGPRSStat(SAT_GPRS_ACT_FAILED, (UBYTE)SAT_error_cause);
        break;
    }
  }
#endif /* FF_SAT_E */
}


/*
+----------------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : gaci                             |
|                                 ROUTINE : pdp_context_create_node          |
+----------------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL T_PDP_CONTEXT_INTERNAL *pdp_context_create_node( U8 cid )
{
  T_PDP_CONTEXT_INTERNAL *p_new_pdp_context_node  = NULL;
  T_PDP_CONTEXT_INTERNAL *p_last_pdp_context_node = NULL;

  TRACE_FUNCTION("pdp_context_create_node()");
  
  if( !pdp_context_find_node_from_cid( cid ) )
  {
    /* No PDP context exist with same <cid> */
    p_last_pdp_context_node = pdp_context_find_last_node();

    if( p_last_pdp_context_node )
    {
      ACI_MALLOC( p_new_pdp_context_node, sizeof(T_PDP_CONTEXT_INTERNAL) );

      if( p_new_pdp_context_node )  
      {
        memset(p_new_pdp_context_node, 0x00, sizeof(T_PDP_CONTEXT_INTERNAL) );
        
        p_new_pdp_context_node->cid         = cid;
        p_new_pdp_context_node->p_next      = NULL;
        p_new_pdp_context_node->tft_changed = FALSE;

        /* As alborg code doesn't work on SN_SWITCH_REQ, they don't set the 
           values of link_id to 0xFF. But this is required in Berlin code.
         */

        p_new_pdp_context_node->internal_data.link_id      = DTI_LINK_ID_NOTPRESENT;
        /*p_new_pdp_context_node->internal_data.link_id_new  = DTI_LINK_ID_NOTPRESENT;*/
        p_new_pdp_context_node->internal_data.link_id_uart = DTI_LINK_ID_NOTPRESENT;

        if( p_last_pdp_context_node EQ (T_PDP_CONTEXT_INTERNAL*) &p_pdp_context_list )
          p_pdp_context_list = p_new_pdp_context_node;
        else
          p_last_pdp_context_node->p_next = p_new_pdp_context_node;

      }
    }  
  }
  
  return p_new_pdp_context_node;
  
}


/*
+----------------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : gaci                             |
|                                 ROUTINE : pdp_context_remove_node          |
+----------------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL int pdp_context_remove_node( U8 cid )
{
  SHORT result = 0;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node     = NULL;
  T_PDP_CONTEXT_INTERNAL *p_old_pdp_context_node = NULL;

  if( p_pdp_context_list )
  {
    if( p_pdp_context_list->cid NEQ cid )
    {
      p_pdp_context_node = p_pdp_context_list;
  
      /* find PDP context node for given <cid> */

      while( p_pdp_context_node->p_next AND p_pdp_context_node->p_next->cid NEQ cid )
      {
         p_pdp_context_node = p_pdp_context_node->p_next;
      }

      if( p_pdp_context_node->p_next->cid EQ cid )
      {
        p_old_pdp_context_node = p_pdp_context_node->p_next;
        p_pdp_context_node->p_next = p_pdp_context_node->p_next->p_next;
        ACI_MFREE( p_old_pdp_context_node );
        result = cid;
      }
    }
    else
    {
      p_old_pdp_context_node = p_pdp_context_list;
      
      if( p_pdp_context_list->p_next )
      {

        p_pdp_context_list = p_pdp_context_list->p_next;
      }
      else
      {
        p_pdp_context_list = NULL;
      }
      
      ACI_MFREE( p_old_pdp_context_node );
    }
    result = cid;
  }
  
  return result;
  
}


/*
+----------------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : gaci                             |
|                                 ROUTINE : pdp_context_find_node_from_cid   |
+----------------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL T_PDP_CONTEXT_INTERNAL *pdp_context_find_node_from_cid( U8 cid )
{

  T_PDP_CONTEXT_INTERNAL *p_found_pdp_context_node = NULL;
  T_PDP_CONTEXT_INTERNAL *p_temp_pdp_context_node  = NULL;

  TRACE_EVENT("pdp_context_find_node_from_cid()");
  if( p_pdp_context_list )
  {
    if( p_pdp_context_list->cid EQ cid )
    {
      p_found_pdp_context_node = p_pdp_context_list;
    }
    else
    {
      p_temp_pdp_context_node = p_pdp_context_list->p_next;
      
      while( p_temp_pdp_context_node AND !p_found_pdp_context_node )
      {
        if( p_temp_pdp_context_node->cid EQ cid )
          p_found_pdp_context_node = p_temp_pdp_context_node;
        else
          p_temp_pdp_context_node = p_temp_pdp_context_node->p_next;
      }
    }
  }
  
  return p_found_pdp_context_node;

}


/*
+----------------------------------------------------------------------------+
| PROJECT : UMTS                MODULE  : gaci                               |
|                               ROUTINE : pdp_context_find_node_from_dti_id  |
+----------------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL T_PDP_CONTEXT_INTERNAL *pdp_context_find_node_from_dti_id( U8 dti_id )
{
  T_PDP_CONTEXT_INTERNAL *p_found_pdp_context_node  = NULL;

  p_found_pdp_context_node = p_pdp_context_list;
    
  while (p_found_pdp_context_node)
  {
    if( ( EXTRACT_DTI_ID(p_found_pdp_context_node->internal_data.link_id) EQ dti_id ) OR
        ( EXTRACT_DTI_ID(p_found_pdp_context_node->internal_data.link_id_uart) EQ dti_id ) )
    {
      return (p_found_pdp_context_node);
    }
    p_found_pdp_context_node = p_found_pdp_context_node->p_next;
  }
  
  return NULL;
}


/*
+-------------------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : gaci                                |
|                                 ROUTINE : pdp_context_find_node_from_smreg_ti |
+-------------------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL T_PDP_CONTEXT_INTERNAL *pdp_context_find_node_from_smreg_ti( U8 smreg_ti )
{
  T_PDP_CONTEXT_INTERNAL *p_found_pdp_context_node  = NULL;
  T_PDP_CONTEXT_INTERNAL *p_temp_pdp_context_node   = NULL;

  if( p_pdp_context_list )
  {
    if( p_pdp_context_list->internal_data.smreg_ti EQ smreg_ti )
    {
      p_found_pdp_context_node = p_pdp_context_list;
    }
    else
    {
      p_temp_pdp_context_node = p_pdp_context_list->p_next;
      
      while( p_temp_pdp_context_node AND !p_found_pdp_context_node )
      {
        if( p_temp_pdp_context_node->internal_data.smreg_ti EQ smreg_ti )
          p_found_pdp_context_node = p_temp_pdp_context_node;
        else
          p_temp_pdp_context_node = p_temp_pdp_context_node->p_next;
      }
    }
  }
  
  return p_found_pdp_context_node;
  
}


/*
+----------------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : gaci                             |
|                                 ROUTINE : pdp_context_find_last_node       |
+----------------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL T_PDP_CONTEXT_INTERNAL *pdp_context_find_last_node( void )
{

  T_PDP_CONTEXT_INTERNAL *p_last_pdp_context_node = NULL;

  if( p_pdp_context_list )
  {
    if( !p_pdp_context_list->p_next )
    {
      p_last_pdp_context_node = p_pdp_context_list;
    }
    else
    {
      p_last_pdp_context_node = p_pdp_context_list;

      while( p_last_pdp_context_node->p_next )
      {
         p_last_pdp_context_node = p_last_pdp_context_node->p_next;
         
         if( !p_last_pdp_context_node )
           break;
      }
    }
  }
  else
  {
    p_last_pdp_context_node = (T_PDP_CONTEXT_INTERNAL*) &p_pdp_context_list;
  }
  
  return p_last_pdp_context_node;

}

/*
+----------------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : gaci                             |
|                                 ROUTINE : pdp_context_find_matching_node   |
+----------------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL T_PDP_CONTEXT_INTERNAL *pdp_context_find_matching_node( T_SMREG_VAL_pdp_type smreg_pdp_type, T_NAS_ctrl_ip_address ctrl_ip_addr, T_NAS_ip_address *ip_addr )
{

  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  T_PDP_TYPE             pdp_type;
  BOOL                   match = FALSE;


  switch( smreg_pdp_type )
  {
    case SMREG_PDP_PPP:
      strcpy( pdp_type, "PPP");
      break;
    
    case SMREG_PDP_IPV4:
      strcpy( pdp_type, "IP");
      break;
        
    case SMREG_PDP_IPV6:
      strcpy( pdp_type, "IPV6");
      break;
      
    default:
      return NULL;
  }

  p_pdp_context_node = p_pdp_context_list;

  while( p_pdp_context_node AND match EQ FALSE)
  {
    if( !strcmp(p_pdp_context_node->attributes.pdp_type, pdp_type) )
    {
      if( p_pdp_context_node->attributes.pdp_addr.ctrl_ip_address EQ ctrl_ip_addr )
      {
        switch( ctrl_ip_addr )
        {
          case NAS_is_ipv4:
          {
            if( !memcmp( &ip_addr->ipv4_addr,
                         &p_pdp_context_node->attributes.pdp_addr.ip_address.ipv4_addr,
                         NAS_SIZE_IPv4_ADDR ) )
                        
            {
              match = TRUE;
            }
            break;
          }

          case NAS_is_ipv6:
          {
            if( !memcmp( &ip_addr->ipv6_addr,
                         &p_pdp_context_node->attributes.pdp_addr.ip_address.ipv6_addr,
                         NAS_SIZE_IPv6_ADDR ) )
                      
            {
              match = TRUE;
            }
            break;
          }
        }
                        
      }

    }
    
    if( match EQ TRUE )
    {
      break;
    }
    else
    {
      p_pdp_context_node = p_pdp_context_node->p_next;
    }
    
  }
  
  return p_pdp_context_node;

}



GLOBAL U8 pdp_context_get_free_cid( void )
{
#if (PDP_CONTEXT_CID_MAX > 15)
  #error "Size of used_cid_list must be changed to fit the value of PDP_CONTEXT_CID_MAX"
#else
  U16                     used_cid_list      = 0;
#endif

  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  U8                      free_cid           = 0;

  p_pdp_context_node = p_pdp_context_list;

  while( p_pdp_context_node )
  {
    used_cid_list |= ( 0x0001 << (p_pdp_context_node->cid - 1) );
    p_pdp_context_node = p_pdp_context_node->p_next;
  }

  while( ( 0x0001 & (used_cid_list >> free_cid)) AND free_cid <= PDP_CONTEXT_CID_MAX )
  {
    free_cid++;
  }
  
  free_cid++;
 
  if( free_cid > PDP_CONTEXT_CID_MAX )
    free_cid = PDP_CONTEXT_CID_INVALID;

  return free_cid;
}



/*
+----------------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : gaci                             |
|                                 ROUTINE : pdp_context_add_pf               |
+----------------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL T_TFT_INTERNAL *pdp_context_add_tft_pf( U8 cid, U8 tft_pf_id )
{
  T_TFT_INTERNAL *p_tft_pf_node_tmp_1 = NULL; /* TFT_PF_NODE  n - 1    */
  T_TFT_INTERNAL *p_tft_pf_node_tmp_0 = NULL; /* TFT_PF_NODE  n        */

  T_TFT_INTERNAL *p_tft_pf_node_new   = NULL; /* TFT_PF_NODE  new      */  

  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
 
  p_pdp_context_node = pdp_context_find_node_from_cid( cid );
    
  if( p_pdp_context_node )
  {
    p_tft_pf_node_new = pdp_context_find_tft_pf( cid, tft_pf_id );

    if( ! p_tft_pf_node_new )
    {
      ACI_MALLOC( p_tft_pf_node_new, sizeof(T_TFT_INTERNAL));
      if( p_tft_pf_node_new )
      {
        p_tft_pf_node_new->pf_attributes.tft_pf_id = tft_pf_id;
        p_tft_pf_node_new->p_next                  = NULL;


        if( p_pdp_context_node->p_tft_pf )
        {
          if( p_pdp_context_node->p_tft_pf->pf_attributes.tft_pf_id > tft_pf_id )
          {
            p_tft_pf_node_new->p_next    = p_pdp_context_node->p_tft_pf;
            p_pdp_context_node->p_tft_pf = p_tft_pf_node_new;
          }
          else
          {
            if( p_pdp_context_node->p_tft_pf->p_next )
            {
              p_tft_pf_node_tmp_1 = p_pdp_context_node->p_tft_pf;
              p_tft_pf_node_tmp_0 = p_pdp_context_node->p_tft_pf->p_next;

              while( p_tft_pf_node_tmp_0 )
              {
                if( p_tft_pf_node_tmp_0->pf_attributes.tft_pf_id > tft_pf_id )
                {
                  p_tft_pf_node_new->p_next   = p_tft_pf_node_tmp_0;
                  p_tft_pf_node_tmp_1->p_next = p_tft_pf_node_new;
                  break;
                }
                else
                {
                  p_tft_pf_node_tmp_1 = p_tft_pf_node_tmp_0;
                  p_tft_pf_node_tmp_0 = p_tft_pf_node_tmp_0->p_next;
                }
              }
              p_tft_pf_node_tmp_1->p_next = p_tft_pf_node_new;
            }
            else
            {
              p_pdp_context_node->p_tft_pf->p_next = p_tft_pf_node_new;
            }
          }
        }
        else
        {
          p_pdp_context_node->p_tft_pf = p_tft_pf_node_new;
        }
      }
    }
    else
    {
      TRACE_ERROR( "ERROR: TFT PF with same PF_ID exist" );
      p_tft_pf_node_new = NULL;
    }

  }
  else
  {
    TRACE_ERROR( "ERROR: PDP context not found, in function pdp_context_add_tft_pf");
  }

  return p_tft_pf_node_new;  
  
}


/*
+----------------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : gaci                             |
|                                 ROUTINE : pdp_context_del_pf               |
+----------------------------------------------------------------------------+

  PURPOSE : Delete the specified packet filter from a TFT.

*/
GLOBAL BOOL pdp_context_del_tft_pf( U8 cid, U8 tft_pf_id )
{
  BOOL result = FALSE;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  T_TFT_INTERNAL         *p_tft_pf_node      = NULL;
  T_TFT_INTERNAL         *p_tft_pf_node_old  = NULL;

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );
  
  if( p_pdp_context_node AND p_pdp_context_node->p_tft_pf )
  {
    if( p_pdp_context_node->p_tft_pf->pf_attributes.tft_pf_id NEQ tft_pf_id )
    {
      p_tft_pf_node = p_pdp_context_node->p_tft_pf;

      while( p_tft_pf_node->p_next AND p_tft_pf_node->p_next->pf_attributes.tft_pf_id NEQ tft_pf_id )
      {
        p_tft_pf_node = p_tft_pf_node->p_next;
      }

      if( p_tft_pf_node->p_next->pf_attributes.tft_pf_id EQ tft_pf_id )
      {
        p_tft_pf_node_old = p_tft_pf_node->p_next;
        p_tft_pf_node->p_next = p_tft_pf_node->p_next->p_next;
        ACI_MFREE( p_tft_pf_node_old );
        result = TRUE;
      }
      else
      {
        TRACE_EVENT( "ERROR: PF not found for given PDP cid, in function pdp_context_del_tft_pf" );
      }
    }
    else
    {
      p_tft_pf_node_old = p_pdp_context_node->p_tft_pf;

      if( p_pdp_context_node->p_tft_pf->p_next )
      {
        p_pdp_context_node->p_tft_pf = p_pdp_context_node->p_tft_pf->p_next;
      }
      else
      {
        p_pdp_context_node->p_tft_pf = NULL;
      }

      ACI_MFREE( p_tft_pf_node_old );
      result = TRUE;
    }
  }
  else
  {
    TRACE_EVENT( "ERROR: in function pdp_context_del_tft_pf" );
  }

  return result;
  
}


/*
+----------------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : gaci                             |
|                                 ROUTINE : pdp_context_del_tft              |
+----------------------------------------------------------------------------+

  PURPOSE : Delete all packet filters for an entire TFT.

*/
GLOBAL BOOL pdp_context_del_tft( U8 cid )
{
  BOOL result = TRUE;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );
  if( p_pdp_context_node )
  {
    while( p_pdp_context_node->p_tft_pf )
    {
      if( !pdp_context_del_tft_pf( cid, p_pdp_context_node->p_tft_pf->pf_attributes.tft_pf_id ) )
      {
        result = FALSE;
        break;
      }
    }
  }
  else
  {
    result = FALSE;
  }

  return result;
}


/*
+----------------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : gaci                             |
|                                 ROUTINE : pdp_context_find_tft_pf          |
+----------------------------------------------------------------------------+

  PURPOSE : Find the specified packet flter in a TFT and return the pointer.

*/
GLOBAL T_TFT_INTERNAL *pdp_context_find_tft_pf( U8 cid, U8 tft_pf_id )
{
  T_TFT_INTERNAL         *p_tft_pf_node      = NULL;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );

  if( p_pdp_context_node )
  {

    if( p_pdp_context_node->p_tft_pf )
    {
      p_tft_pf_node = p_pdp_context_node->p_tft_pf;
    }
    
    if( p_tft_pf_node )
    {
      while( p_tft_pf_node->pf_attributes.tft_pf_id NEQ tft_pf_id AND p_tft_pf_node->p_next )
      {
        p_tft_pf_node = p_tft_pf_node->p_next;
      }

      if( p_tft_pf_node->pf_attributes.tft_pf_id NEQ tft_pf_id )
      {
        p_tft_pf_node = NULL;
      }
      
    }
    
  }

  return p_tft_pf_node;
  
}


/*
+----------------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : gaci                             |
|                                 ROUTINE : pdp_context_find_tft_pf          |
+----------------------------------------------------------------------------+

  PURPOSE : return the number of defined packet filters in a TFT.

*/
GLOBAL U8 pdp_context_get_no_of_tft_pfs( U8 cid )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  T_TFT_INTERNAL         *p_tft_pf_node      = NULL;
  U8 pf_count = 0;

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );

  if( p_pdp_context_node )
  {
    p_tft_pf_node = p_pdp_context_node->p_tft_pf;
    while( p_tft_pf_node )
    {
      pf_count++;
      p_tft_pf_node = p_tft_pf_node->p_next;
    }
  }

  return pf_count;
  
}


/*
+-------------------------------------------------------------------------------+
| PROJECT :                       MODULE  : gaci                                |
|                                 ROUTINE : pdp_context_check_if_nodes_exists   |
+-------------------------------------------------------------------------------+

  PURPOSE : Check if the PDP contexts specified in the cid_array are defined.
            The function will return 0 if all specified PDP contexts are defind
            othervise the first undefined cid in the cid_array is returned.
*/
GLOBAL U8 pdp_context_check_if_nodes_exists( U8 *cid_array )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  U8 i = 0;
  U8 result = 0;

  while( (cid_array[i] NEQ PDP_CONTEXT_CID_OMITTED) AND (i < PDP_CONTEXT_CID_MAX) )
  {
    p_pdp_context_node = pdp_context_find_node_from_cid( cid_array[i] );
    if( p_pdp_context_node )
    {
      i++;
    }
    else
    {
      result = cid_array[i];
      break;
    }
  }

  return result;
}


/*
+-------------------------------------------------------------------------------+
| PROJECT :                       MODULE  : gaci                                |
|                                 ROUTINE : pdp_context_check_if_nodes_exists   |
+-------------------------------------------------------------------------------+

  PURPOSE : Check if the pdp_type matches the specified PDP contexts in the cid_array.
            The function will return 0 if all specified PDP contexts matches
            othervise the first unmatched PDP context in the cid_array is returned.
*/
GLOBAL U8 pdp_context_validate_pdp_type( U8 *cid_array, T_PDP_TYPE pdp_type )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  U8 i = 0;
  U8 result = 0;

  while( (cid_array[i] NEQ PDP_CONTEXT_CID_OMITTED) AND (i < PDP_CONTEXT_CID_MAX) )
  {
    p_pdp_context_node = pdp_context_find_node_from_cid( cid_array[i] );
    if( p_pdp_context_node )
    {
      if( strcmp(p_pdp_context_node->attributes.pdp_type, pdp_type) )
      {
        result = cid_array[i];
        break;
      }
      i++;
    }
    else
    {

      result = cid_array[i];
      break;
    }
  }

  return result;
}


#endif  /* GPRS */
