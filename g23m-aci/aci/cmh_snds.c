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
|  Purpose :  This module implements the set fuinctions related to the
|             protocol stack adapter for the SNDCP entity.
+-----------------------------------------------------------------------------
*/

#ifdef GPRS

#ifndef CMH_SNDS_C
#define CMH_SNDS_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "dti_conn_mng.h"

#include "aci_cmh.h"

#include "cmh_snd.h"

#include "dti_cntrl_mng.h"

#include "gaci_cmh.h"
#include "gaci.h"
#include "psa.h"
#include "cmh.h"
#include "psa_sm.h"
#include "cmh_sm.h"
#include "psa_upm.h"

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/
EXTERN T_PDP_CONTEXT_INTERNAL *p_pdp_context_list;


/*==== FUNCTIONS ==================================================*/


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8441)         MODULE  : CMH_SMS                  |
| STATE   : finnished             ROUTINE : sAT_PercentSNCNT         |
+--------------------------------------------------------------------+

PURPOSE : This is the functional counterpart to the %SNCNT = AT
          command which resets the SNDCP Counter.
*/
GLOBAL T_ACI_RETURN sAT_PercentSNCNT( T_ACI_CMD_SRC srcId, BOOL reset_counter )
{
  UBYTE sndcp_id;
  BOOL  prim_sent = FALSE;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION ("sAT_PercentSNCNT()");

  sndcpShrdPrm.srcId = srcId;

  p_pdp_context_node = p_pdp_context_list;
  while( p_pdp_context_node )
  {
    if ( p_pdp_context_node->internal_data.state EQ PDP_CONTEXT_STATE_DATA_LINK OR
  	 p_pdp_context_node->internal_data.state EQ PDP_CONTEXT_STATE_ACTIVATED    )
    {
      sndcp_id = (U8)CID_TO_NSAPI( p_pdp_context_node->cid );
      psa_upm_count_req( sndcp_id, reset_counter );
      prim_sent = TRUE;

      TRACE_EVENT_P1("found PDP connection on channel %d", sndcp_id);
    }
    p_pdp_context_node = p_pdp_context_node->p_next;
  }

  if (prim_sent)
  {
    return AT_EXCT;
  }
  else
  {
    return AT_CMPL;
  }
}

#endif /* GPRS */
