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
|  Purpose :  This module defines the functions which are responsible
|             for the responses of the protocol stack adapter for
|             GPRS Point-to-Point Protocol ( PPP ).
+----------------------------------------------------------------------------- 
*/ 

#if defined (GPRS) && defined (DTI)

#ifndef CMH_GPPPR_C
#define CMH_GPPPR_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/

#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "aci.h"
#include "gaci.h"
#include "gaci_cmh.h"
#include "psa.h"
#include "psa_gppp.h"
#include "psa_sm.h"
#include "psa_uart.h"

#include "psa_aaa.h"

#include "cmh.h"
#include "cmh_gppp.h"
#include "cmh_sm.h"

#include "sap_dti.h"

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_PPPR                     |
| STATE   : finnished        ROUTINE : cmhGPPP_Established           |
+-------------------------------------------------------------------+

  PURPOSE : establish a PPP link was successful

*/
GLOBAL SHORT cmhGPPP_Established ( void )
{

  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION ("cmhGPPP_Established()");

  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[cid_pointer] );
  
  if( p_pdp_context_node )
  {
    switch( gpppEntStat.curCmd )
    {
      case( AT_CMD_CGDATA ):
        /* no action, because the PDP context will be terminated */
        if( get_state_working_cid() NEQ PDP_CONTEXT_STATE_ESTABLISH_3 )
        {
          return 0;
        }
        else
        {
          set_state_working_cid( PDP_CONTEXT_STATE_DATA_LINK );
        /*
         *  do we need one more context activation
         */
          if( cmhSM_next_work_cid( AT_CMD_CGDATA ) EQ FALSE )
          {
            /* todo for new DTI interface - SMNEW done 06032001... */
            R_AT( RAT_CGDATA, gpppEntStat.entOwn )( p_pdp_context_node->internal_data.link_id ); /* UACI */
            /* PHE 201201 gpppEntStat.curCmd = AT_CMD_NONE; */
          }
        }
               
      cmhSM_next_call_table_entry();
      break;
    }
  }
  else
  {
    TRACE_ERROR( "ERROR: PDP context not found, in function cmhGPPP_Established" );
  }
  return 0;
}


/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_PPPR                     |
| STATE   : code             ROUTINE : cmhGPPP_Terminated           |
+-------------------------------------------------------------------+

  PURPOSE : PPP connection is closed

*/
GLOBAL SHORT cmhGPPP_Terminated ( void )
{
  T_ACI_CMD_SRC     rat_owner;
  UBYTE               cmdBuf  = gpppEntStat.curCmd, /* buffers current command */
  cme_err = CME_ERR_Unknown;   /* error number */
  U8      cid = PDP_CONTEXT_CID_INVALID;
  SHORT   reactivation = 0,
          rat_id = RAT_MAX;
  USHORT  nsapi_set = 0;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
#ifdef FF_TCP_IP
  T_DTI_ENTITY_ID peer;
#endif
  UBYTE srcId = srcId_cb;

  TRACE_FUNCTION ("cmhGPPP_Terminated()");

  cid = gaci_get_cid_over_link_id(gaci_get_link_id_over_peer(DTI_ENTITY_PPPS));

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );

#ifdef FF_TCP_IP
  peer = dti_cntrl_get_peer(DTI_ENTITY_PPPS, 0, 0);
  if ( peer EQ DTI_ENTITY_AAA )
  {
    psaAAA_disconnect_ind (p_pdp_context_node->internal_data.link_id_uart);
  }
#endif

  if( p_pdp_context_node )
  {
    cid = p_pdp_context_node->cid;

    /*
     *-------------------------------------------------------------------
     * Inform the DTI Manager about the link termination.
     *-------------------------------------------------------------------
     */
    dti_cntrl_entity_disconnected( p_pdp_context_node->internal_data.link_id , DTI_ENTITY_PPPS );
    dti_cntrl_entity_disconnected( p_pdp_context_node->internal_data.link_id_uart, DTI_ENTITY_PPPS );

  switch ( get_state_over_cid( cid ) )
  {
      case PDP_CONTEXT_STATE_INVALID:
      case PDP_CONTEXT_STATE_DEFINED:
      case PDP_CONTEXT_STATE_ATTACHING:
      case PDP_CONTEXT_STATE_ACTIVATING:
      case PDP_CONTEXT_STATE_ACTIVATED:
      case PDP_CONTEXT_STATE_DEACTIVATE_NORMAL:
        TRACE_ERROR( "cmhGPPP_Terminated(): State/Event error!!!" );
        break;
        
      case PDP_CONTEXT_STATE_ESTABLISH_2:
      case PDP_CONTEXT_STATE_ESTABLISH_3:
      case PDP_CONTEXT_STATE_ACTIVATED_ESTABLISH_1:
        set_state_over_cid( cid, PDP_CONTEXT_STATE_ABORT_ESTABLISH );
        nsapi_set  = cmhSM_Give_nsapi_set( cid );
        dti_cntrl_set_dti_id_to_reconnect(EXTRACT_DTI_ID(p_pdp_context_node->internal_data.link_id));
        break;
        
      case PDP_CONTEXT_STATE_ESTABLISH_1:
        cmhSM_contextDeactivated();

      /*lint -fallthrough*/
      case PDP_CONTEXT_STATE_ABORT_ESTABLISH:
        set_state_over_cid( cid, PDP_CONTEXT_STATE_DEFINED );
        /* Close the data path since this can not be done from CMH_SM (no activating/activated context) */
        dti_cntrl_close_dpath_from_dti_id( EXTRACT_DTI_ID(p_pdp_context_node->internal_data.link_id) );

        dti_cntrl_set_dti_id_to_reconnect(EXTRACT_DTI_ID(p_pdp_context_node->internal_data.link_id));
        if(ati_user_output_cfg[srcId].CMEE_stat EQ CMEE_MOD_Disable OR isContextDeactivationRequestedByCGACT(cid))
          rat_id = RAT_NO_CARRIER;
        else
          rat_id = RAT_CME;
        break;
        
      case PDP_CONTEXT_STATE_BREAKDOWN_LINK_NORMAL:
        set_state_over_cid( cid, PDP_CONTEXT_STATE_DEFINED );
        dti_cntrl_set_dti_id_to_reconnect(EXTRACT_DTI_ID(p_pdp_context_node->internal_data.link_id));
        rat_id = RAT_NO_CARRIER;
        break;
        
      case PDP_CONTEXT_STATE_DATA_LINK:
        set_state_over_cid( cid, PDP_CONTEXT_STATE_BREAKDOWN_LINK_NORMAL );
        nsapi_set  = cmhSM_Give_nsapi_set( cid );
        dti_cntrl_set_dti_id_to_reconnect(EXTRACT_DTI_ID(p_pdp_context_node->internal_data.link_id));
        break;
        
      case PDP_CONTEXT_STATE_REACTIVATION_1:
        set_state_over_cid( cid, PDP_CONTEXT_STATE_REACTIVATION_2 );
        dti_cntrl_set_dti_id_to_reconnect(EXTRACT_DTI_ID(p_pdp_context_node->internal_data.link_id));
        gpppEntStat.curCmd = AT_CMD_NONE;
        /* 28592 cmhSM_connection_down(EXTRACT_DTI_ID(p_pdp_context_node->internal_data.link_id)); */
        return 0;
               
      case PDP_CONTEXT_STATE_REACTIVATION_2:
        set_state_over_cid( cid, PDP_CONTEXT_STATE_DEFINED );
        dti_cntrl_set_dti_id_to_reconnect(EXTRACT_DTI_ID(p_pdp_context_node->internal_data.link_id));
        gpppEntStat.curCmd = AT_CMD_NONE;
        rat_id = RAT_NO_CARRIER;
        reactivation = 1;
        cmhSM_connection_down(EXTRACT_DTI_ID(p_pdp_context_node->internal_data.link_id)); /* 28592 */
        break;

    } /* end switch */
   
  }
  else
  {
    TRACE_ERROR( "ERROR: PDP context not found, in function cmhGPPP_Terminated" );
    return 0;
  }

  rat_owner = (T_ACI_CMD_SRC)get_owner_over_cid( cid );

  if ( reactivation EQ 0 )
  {
    /* Do we need a SMREG_PDP_DEACTIVATE_REQ? */
    if ( nsapi_set )
    {
      psaSM_PDP_Deactivate ( nsapi_set, PS_REL_IND_NO );
    }
  /*
   *-------------------------------------------------------------------
   * check for command context
   *-------------------------------------------------------------------
   */
    switch( gpppEntStat.curCmd )
    {
      case( AT_CMD_CGDATA ):
        if ( rat_owner EQ gpppEntStat.entOwn )
        {
          gpppEntStat.curCmd = AT_CMD_NONE;
        }
        /* brz: to check */
        switch(gpppShrdPrm.ppp_cause)
        {
          case PPP_TERM_OK_PEER:
          case PPP_TERM_NO_RESPONSE:
          case PPP_TERM_LOOP_BACK:
          case PPP_TERM_LCP_NOT_CONVERGE:
          case PPP_TERM_IPCP_NOT_CONVERGE:
          case PPP_TERM_IPCP_NOT_STARTED:
            cme_err = CME_ERR_GPRSBadModClass;
            break;
          /* The below cause values is commented as it is not defined in the 
           * Alborg SAP Document, and because of this it is giving compilation
           * ERROR
           */
          /*case SMREG_RC_USE_AUTHED_FAILED:*/ 
          case PPP_TERM_USE_AUTHED_FAILED:
            cme_err = CME_ERR_GPRSPdpAuth;
            break;
          default:
            cme_err = CME_ERR_Unknown;
        }
        break;
#if 0
      /*
       * These cases do never happen!
       */
      case( AT_CMD_CGACT ):
      case( AT_CMD_CGANS ):
      case( AT_CMD_A ):
      case( AT_CMD_H ):
        break;
#endif
      default:
        switch(gpppShrdPrm.ppp_cause)
        {
          case CAUSE_NWSM_SERVICE_NOT_SUPPORTED:
            cme_err = CME_ERR_GPRSSerOptNsup;
            break;
          case CAUSE_NWSM_SERVICE_NOT_SUBSCRIBED:
            cme_err = CME_ERR_GPRSSerOptNsub;
            break;
          case CAUSE_NWSM_SERVICE_TEMP_OUT_OF_ORDER:
            cme_err = CME_ERR_GPRSSerOptOOO;
            break;
          case PPP_TERM_USE_AUTHED_FAILED:
            cme_err = CME_ERR_GPRSPdpAuth;
            break;
          default:
            cme_err = CME_ERR_Unknown;
        }
    }
    if ( smEntStat.entOwn EQ rat_owner )
    {
        switch( smEntStat.curCmd )
        {
          case( AT_CMD_CGDATA ):
          case( AT_CMD_CGACT ):
            smEntStat.curCmd = AT_CMD_NONE;
            break;
        }
    }
  }

  cmhSM_connection_down(EXTRACT_DTI_ID(p_pdp_context_node->internal_data.link_id));

  if( rat_id NEQ RAT_MAX )
  {
    gaci_RAT_caller( rat_id, cid, cmdBuf, cme_err );
      
    if( reactivation EQ 0 )
    {
      work_cids[0] = PDP_CONTEXT_CID_INVALID;    
    }

    cmhSM_context_reactivation();
    cmhSM_disconnect_cid(cid, GC_TYPE_DATA_LINK);
  }

  work_cids[0] = PDP_CONTEXT_CID_INVALID;
  cid_pointer  = 0;
  if ( smEntStat.curCmd EQ AT_CMD_CGACT )
  {
    smEntStat.curCmd = AT_CMD_NONE;
  }
  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_PPPR                     |
| STATE   : finished         ROUTINE : cmhGPPP_Activated             |
+-------------------------------------------------------------------+

  PURPOSE : activate the PDP context (only in server or transparent
  mode) and if the context is already activated then answer PPP.
  (PPP_ACTIVATE_IND is received).


*/
GLOBAL SHORT cmhGPPP_Activated ( void )
{
  U8 cid = work_cids[cid_pointer];
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION ("cmhGPPP_Activated()");

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );
  if( !p_pdp_context_node )
  {
    TRACE_ERROR("ERROR: PDP context not found, in function cmhGPPP_Activated");
    return -1;
  }

/*
 *-------------------------------------------------------------------
 * check for command context
 *-------------------------------------------------------------------
 */
  switch( gpppEntStat.curCmd )
  {
    case( AT_CMD_CGANS ):
    case( AT_CMD_CGDATA ):
    /*
     *---------------------------------------------------------------
      * check entity context state
     *---------------------------------------------------------------
     */
      switch ( get_state_working_cid() )
      {
        case PDP_CONTEXT_STATE_ESTABLISH_1:
        {

         /*
          *---------------------------------------------------------------
          * save parameter
          *---------------------------------------------------------------
          */
  
          /* Issue OMAPS00047332 : SNDCP, SM and UPM are not supporting ppp_hc and msid anymore */

          smEntStat.curCmd   = gpppEntStat.curCmd;
          smShrdPrm.owner    = gpppShrdPrm.owner;
          smEntStat.entOwn   = (T_ACI_CMD_SRC)smShrdPrm.owner;

         /*
          *---------------------------------------------------------------
          * store PCO information if it's provided PPP
          *---------------------------------------------------------------
          */

          if( (gpppShrdPrm.pdp->sdu.l_buf >> 3) > 0 )
          {
            cmhSM_set_PCO(  cid, PCO_USER, 
                  &gpppShrdPrm.pdp->sdu.buf[gpppShrdPrm.pdp->sdu.o_buf >> 3],
                 (UBYTE) (gpppShrdPrm.pdp->sdu.l_buf >> 3));
          }

    /*
     *---------------------------------------------------------------
          * PPP has requested context activation: Activate the context.
     *---------------------------------------------------------------
     */
          cmhSM_connect_context( cid, DTI_ENTITY_PPPS );
      
          set_state_working_cid( PDP_CONTEXT_STATE_ESTABLISH_2 );
        }
        break;

        case PDP_CONTEXT_STATE_ACTIVATED_ESTABLISH_1:
        {
         /*
          *---------------------------------------------------------------
          * save parameter
          *---------------------------------------------------------------
          */
          
          /* Issue OMAPS00047332 : SNDCP, SM and UPM are not supporting ppp_hc and msid anymore
             So do not send ppp_hc and msid params to ppp
           */
          smEntStat.curCmd   = gpppEntStat.curCmd;
          smShrdPrm.owner    = gpppShrdPrm.owner;
          smEntStat.entOwn   = (T_ACI_CMD_SRC)smShrdPrm.owner;

         /*
          *---------------------------------------------------------------
          * PPP has requested context activation, but the context is 
          * already activated: Answer PPP.
          *---------------------------------------------------------------
          */
          psaGPPP_PDP_Activate( &p_pdp_context_node->internal_data.pdp_address_allocated,
                                p_pdp_context_node->internal_data.network_pco.pco, 
                                (UBYTE) (p_pdp_context_node->internal_data.network_pco.len),
                                (UBYTE) CID_TO_NSAPI(cid));

          set_state_working_cid( PDP_CONTEXT_STATE_ESTABLISH_3 );
        }
          break;
        default:
          /* The PPP_PDP_ACTIVATE_IND might have crossed with a PPP_TERMINATE_REQ.
             Ignore the event by returning */
          return 0;
      }

  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_PPPR                     |
| STATE   : finished         ROUTINE : cmhGPPP_Modified             |
+-------------------------------------------------------------------+

  PURPOSE : new negotiate header compression confirmed (only in server mode)

*/
GLOBAL SHORT cmhGPPP_Modified ( void )
{
  TRACE_FUNCTION ("cmhGPPP_Modified()");

/*
 *---------------------------------------------------------------
 * nothing to do
 *---------------------------------------------------------------
 */

  return 0;
}

#endif /* GPRS */
/*==== EOF ========================================================*/

