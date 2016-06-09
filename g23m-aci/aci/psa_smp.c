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
|  Purpose :  This module defines the processing functions for the
|             primitives send to the protocol stack adapter by the
|             session management of GPRS ( SM ).
+-----------------------------------------------------------------------------
*/

#ifdef GPRS

#ifndef PSA_SMP_C
#define PSA_SMP_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "aci.h"

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "gaci.h"
#include "gaci_cmh.h"
#include "psa.h"
#include "psa_sm.h"

#include "cmh.h"
#include "cmh_sm.h"

#if defined (FF_WAP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E)
#include "psa_tcpip.h"
#include "wap_aci.h"
#endif /* defined (FF_WAP) || defined (FF_GPF_TCPIP) */

#ifdef FF_GPF_TCPIP
#include "dcm_utils.h"
#include "dcm_state.h"
#include "dcm_env.h"
#endif
#include "dcm_f.h"



/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/

/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_SM                     |
| STATE   : finished              ROUTINE : psa_smreg_pdp_activate_cnf |
+----------------------------------------------------------------------+

  PURPOSE : processes the SMREG_PDP_ACTIVATE_CNF primitive send by SM.
            this confirms a successful context activation.
*/
GLOBAL void psa_smreg_pdp_activate_cnf ( T_SMREG_PDP_ACTIVATE_CNF *smreg_pdp_activate_cnf )
{

  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION ("psa_smreg_pdp_activate_cnf()");

  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[cid_pointer] );
  if( ! p_pdp_context_node )
  {
    TRACE_ERROR( "ERROR: PDP context not found, in function psa_smreg_pdp_activate_cnf" );
    return;
  }

/*
 *-------------------------------------------------------------------
 * notify ACI
 *-------------------------------------------------------------------
 */

  smShrdPrm.pdp_cnf = smreg_pdp_activate_cnf;

  /*
   * Store negotiated QOS parameters, only used for read out
   */
  p_pdp_context_node->ctrl_neg_qos = smreg_pdp_activate_cnf->ctrl_qos;
  memcpy( &p_pdp_context_node->neg_qos, &smreg_pdp_activate_cnf->qos, sizeof(T_PS_qos) );

  if ( cmhSM_Activated(smreg_pdp_activate_cnf) < 0)
  {
    TRACE_EVENT("cmhSM_Activated( ) returned error.");
  }

  /* free the primitive buffer */
  PFREE (smreg_pdp_activate_cnf);
}



/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_SM                     |
| STATE   : finished              ROUTINE : psa_smreg_pdp_activate_rej |
+----------------------------------------------------------------------+

  PURPOSE : processes the SMREG_PDP_ACTIVATE_REJ primitive send by SM.
            this indicates a context activation failed.
*/
GLOBAL void psa_smreg_pdp_activate_rej ( T_SMREG_PDP_ACTIVATE_REJ *smreg_pdp_activate_rej )
{

  TRACE_FUNCTION ("psa_smreg_pdp_activate_rej()");

  /* update shared parameter and notify ACI */
  smShrdPrm.pdp_rej = smreg_pdp_activate_rej;

  if ( cmhSM_NoActivate( ) < 0)
  {
    TRACE_EVENT("cmhSM_NoActivate( ) returned error.");
  }

#ifdef FF_GPF_TCPIP
  if(is_gpf_tcpip_call())
  {
    T_DCM_STATUS_IND_MSG msg;
    msg.hdr.msg_id = DCM_NEXT_CMD_STOP_MSG;
    dcm_send_message(msg, DCM_SUB_WAIT_CGACT_CNF);
  }
#endif /* FF_GPF_TCPIP */

  /* free the primitive buffer */
  PFREE (smreg_pdp_activate_rej);
}

/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_SM                     |
| STATE   : developing            ROUTINE : convert_apn_to_netaddr     |
+----------------------------------------------------------------------+

  PURPOSE : converts an APN to a dotted network address

*/
LOCAL SHORT convert_apn_to_netaddr ( T_SMREG_apn *apn_src, T_SMREG_apn *apn_dest)
{

  T_SMREG_apn dummy_apn = {0,0,0};
  U8 counter1 = 0;
  U8 counter2 = 0;
  U8 dest_pos = 0;
  U8 bytecount = 0;

  TRACE_FUNCTION ("convert_apn_to_netaddr()");

  if(apn_src EQ apn_dest)
  {
    apn_dest=&dummy_apn;
  }

  while(counter1<apn_src->c_apn_buf-1)
  {
    bytecount=apn_src->apn_buf[counter1];
    for(counter2=0;counter2<bytecount;counter2++)
    {
      counter1++;
      if (counter1>=apn_src->c_apn_buf)
      {
        return -1;
      }
      else
      {
        apn_dest->apn_buf[dest_pos]=apn_src->apn_buf[counter1];
          dest_pos++;
      }
    }
    if (counter1<apn_src->c_apn_buf-1)
    {
      apn_dest->apn_buf[dest_pos]='.';
        dest_pos++;
        counter1++;
    }
    else
    {
      apn_dest->apn_buf[dest_pos]=0;
    }
  }
  if (apn_dest EQ &dummy_apn)
  {
    apn_dest=apn_src;
      memcpy(apn_dest->apn_buf,&dummy_apn.apn_buf,dest_pos+1);
      apn_dest->c_apn_buf=dest_pos;
  }
  return 0;
}

/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_SM                     |
| STATE   : finished              ROUTINE : psa_smreg_pdp_activate_ind |
+----------------------------------------------------------------------+

  PURPOSE : processes the SMREG_PDP_ACTIVATE_IND primitive send by SM.
            this indicates a network asked for a PDP context activation.
*/
GLOBAL void psa_smreg_pdp_activate_ind ( T_SMREG_PDP_ACTIVATE_IND *smreg_pdp_activate_ind )
{

  TRACE_FUNCTION ("psa_smreg_pdp_activate_ind()");

  /* update shared parameter and notify ACI */
  memcpy(&smShrdPrm.act_ind, smreg_pdp_activate_ind, sizeof(T_SMREG_PDP_ACTIVATE_IND));

  convert_apn_to_netaddr(&smShrdPrm.act_ind.apn,&smShrdPrm.act_ind.apn);

  cmhSM_NetActivate( );

  /* free the primitive buffer */
  PFREE (smreg_pdp_activate_ind);
}

/*
+------------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_SM                       |
| STATE   : finished              ROUTINE : psa_smreg_pdp_deactivate_cnf |
+------------------------------------------------------------------------+

  PURPOSE : processes the SMREG_PDP_DEACTIVATE_CNF primitive send by SM.
            this confirms a successful PDP context deactivation.
*/
GLOBAL void psa_smreg_pdp_deactivate_cnf
                  ( T_SMREG_PDP_DEACTIVATE_CNF *smreg_pdp_deactivate_cnf )
{

  TRACE_FUNCTION ("psa_smreg_pdp_deactivate_cnf()");

  /* update shared parameter and notify ACI */
  smShrdPrm.nsapi_set = smreg_pdp_deactivate_cnf->nsapi_set;

  cmhSM_Deactivated();

  /* free the primitive buffer */
  PFREE (smreg_pdp_deactivate_cnf);
}

/*
+------------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_SM                       |
| STATE   : finished              ROUTINE : psa_smreg_pdp_deactivate_ind |
+------------------------------------------------------------------------+

  PURPOSE : processes the SMREG_PDP_DEACTIVATE_IND primitive send by SM.
            this indicates a network initiated PDP context deactivation.
*/
GLOBAL void psa_smreg_pdp_deactivate_ind
                  ( T_SMREG_PDP_DEACTIVATE_IND *smreg_pdp_deactivate_ind )
{

  TRACE_FUNCTION ("psa_smreg_pdp_deactivate_ind()");

  /* update shared parameter and notify ACI */
  smShrdPrm.nsapi_set = smreg_pdp_deactivate_ind->nsapi_set;

  cmhSM_NetDeactivate( );

  /* free the primitive buffer */
  PFREE (smreg_pdp_deactivate_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_SM                   |
| STATE   : finished              ROUTINE : psa_smreg_pdp_modify_ind |
+--------------------------------------------------------------------+

  PURPOSE : processes the SMREG_PDP_MODIFY_IND primitive send by SM.
            this indicates a network initiated PDP context modification.
*/
GLOBAL void psa_smreg_pdp_modify_ind
                  ( T_SMREG_PDP_MODIFY_IND *smreg_pdp_modify_ind )
{

  TRACE_FUNCTION ("psa_smreg_pdp_modify_ind()");

/*
 *-------------------------------------------------------------------
 * informs context manager
 *-------------------------------------------------------------------
 */

  cmhSM_NetModify(smreg_pdp_modify_ind);

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (smreg_pdp_modify_ind);
}



/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_SM                     |
| STATE   : finished              ROUTINE : psa_smreg_pdp_modify_cnf   |
+----------------------------------------------------------------------+

  PURPOSE : processes the SMREG_PDP_MODIFY_CNF primitive send by SM.
            this confirms a successful context modification.
*/
#ifdef REL99
GLOBAL void psa_smreg_pdp_modify_cnf ( T_SMREG_PDP_MODIFY_CNF *smreg_pdp_modify_cnf )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION ("psa_smreg_pdp_modify_cnf()");

  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[cid_pointer] );
  if( !p_pdp_context_node )
  {
    TRACE_ERROR( "ERROR: PDP context not found, in function psa_smreg_pdp_modify_cnf" );
    return;
  }

/*
 *-------------------------------------------------------------------
 * update shared parameter with new QoS and notify ACI
 *-------------------------------------------------------------------
 */
  if( smreg_pdp_modify_cnf->ctrl_qos NEQ PS_is_qos_not_present )
  {
    p_pdp_context_node->ctrl_neg_qos = smreg_pdp_modify_cnf->ctrl_qos;
    memcpy( &p_pdp_context_node->neg_qos, &smreg_pdp_modify_cnf->qos, sizeof(T_PS_qos) );
  }


  
  if( cmhSM_Modified( smreg_pdp_modify_cnf) > 0 )
  {
    TRACE_EVENT("cmhSM( SM_NTF_MOD_CNF ): not supported return value.");
  }  
/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (smreg_pdp_modify_cnf);

}


/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_SM                     |
| STATE   : finished              ROUTINE : psa_smreg_pdp_modify_rej   |
+----------------------------------------------------------------------+

  PURPOSE : processes the SMREG_PDP_MODIFY_REJ primitive send by SM.
            this indicates a context modification failed.
*/

GLOBAL  void psa_smreg_pdp_modify_rej ( T_SMREG_PDP_MODIFY_REJ *smreg_pdp_modify_rej )
{

  TRACE_FUNCTION ("psa_smreg_pdp_modify_rej()");

/*
 *-------------------------------------------------------------------
 * notify ACI
 *-------------------------------------------------------------------
 */
  if( cmhSM_NoModify( smreg_pdp_modify_rej ) < 0 )
  {
    TRACE_EVENT("cmhSM( SM_NTF_MOD_REJ ) give not supported return value.");
  }
/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (smreg_pdp_modify_rej);
}

/*
+----------------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : PSA_SM                           |
| STATE   :                       ROUTINE : psa_smreg_pdp_activate_sec_cnf   |
+----------------------------------------------------------------------------+

  PURPOSE : processes the SMREG_PDP_ACTIVATE_SEC_CNF primitive send by SM.
*/
GLOBAL  void psa_smreg_pdp_activate_sec_cnf( T_SMREG_PDP_ACTIVATE_SEC_CNF *smreg_pdp_activate_sec_cnf )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION ("psa_smreg_pdp_activate_sec_cnf()");

  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[cid_pointer] );
  if( !p_pdp_context_node )
  {
    TRACE_ERROR( "ERROR: PDP context not found, in function psa_smreg_pdp_activate_sec_cnf" );
    return;
  }

/*
 *-------------------------------------------------------------------
 * update shared parameter with new QoS and notify ACI
 *-------------------------------------------------------------------
 */
  p_pdp_context_node->ctrl_neg_qos = smreg_pdp_activate_sec_cnf->ctrl_qos;
  memcpy( &p_pdp_context_node->neg_qos, &smreg_pdp_activate_sec_cnf->qos, sizeof(T_PS_qos) );


/*
 *-------------------------------------------------------------------
 * notify ACI
 *-------------------------------------------------------------------
 */
  if( cmhSM_ActivatedSecondary( smreg_pdp_activate_sec_cnf ) < 0 )
  {
    TRACE_EVENT("cmhSM( SM_NTF_ACT_SEC_CNF ) un-supported return value.");
  }
/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE( smreg_pdp_activate_sec_cnf );
}


/*
+----------------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : PSA_SM                           |
| STATE   :                       ROUTINE : psa_smreg_pdp_activate_sec_rej   |
+----------------------------------------------------------------------------+

  PURPOSE : processes the SMREG_PDP_ACTIVATE_SEC_REJ primitive send by SM.
*/
GLOBAL void psa_smreg_pdp_activate_sec_rej( T_SMREG_PDP_ACTIVATE_SEC_REJ *smreg_pdp_activate_sec_rej )
{

  TRACE_FUNCTION ("psa_smreg_pdp_activate_sec_rej()");

/*
 *-------------------------------------------------------------------
 * notify ACI
 *-------------------------------------------------------------------
 */
  if( cmhSM_NoActivateSecondary( smreg_pdp_activate_sec_rej ) < 0 )
  {
    TRACE_EVENT("cmhSM( SM_NTF_ACT_SEC_REJ ) un-supported return value.");
  }
/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE( smreg_pdp_activate_sec_rej );
}
#endif /* REL99 */

#endif  /* GPRS */
/*==== EOF =========================================================*/
