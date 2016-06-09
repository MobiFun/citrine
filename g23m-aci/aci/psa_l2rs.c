/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_L2RS
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
|             protocol stack adapter for layer 2 relay.
+-----------------------------------------------------------------------------
*/

#ifdef DTI

#ifndef PSA_L2RS_C
#define PSA_L2RS_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "dti.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci.h"

#include "psa.h"
#include "psa_l2r.h"
#include "cmh.h"
#include "aci_fd.h"
#include "cmh_ra.h"
#include "cmh_l2r.h"
#include "psa_uart.h"

#if defined (FF_WAP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E)
#include "wap_aci.h"
#include "psa_ppp_w.h"
#endif /* defined (FF_WAP) || defined (FF_GPF_TCPIP) */
#include "aci_lst.h"

#include "aci_lst.h"

#ifdef FF_PSI
#include "psa_psi.h"
#include "cmh_psi.h"
#include "ati_src_psi.h"
#endif /*FF_PSI*/
#include "psa_util.h"

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_L2R                      |
|                            ROUTINE : psaL2R_Activate              |
+-------------------------------------------------------------------+

  PURPOSE : Activate L2R.

*/

GLOBAL SHORT psaL2R_Activate(UBYTE owner)
{
#define X l2rShrdPrm.set_prm[owner]

  TRACE_FUNCTION ("psaL2R_Activate()");

  /* owner belongs to PSA and not to CMH */
  l2rShrdPrm.owner = owner;

  switch (l2rShrdPrm.state)
  {
    case L2R_DEACTIVATED:
    {
      PALLOC (l2r_activate_req, L2R_ACTIVATE_REQ);

      l2r_activate_req->k_ms_iwf        = X.k_ms_iwf;
      l2r_activate_req->k_iwf_ms        = X.k_iwf_ms;
      l2r_activate_req->t1              = X.t1;
      l2r_activate_req->t2              = L2R_T2_DEF;
      l2r_activate_req->n2              = X.n2;
      l2r_activate_req->pt              = L2R_PT_DEF;
      l2r_activate_req->p0              = X.p0;
      l2r_activate_req->p1              = X.p1;
      l2r_activate_req->p2              = X.p2;
      l2r_activate_req->uil2p           = X.uil2p;
      l2r_activate_req->bytes_per_prim  = L2R_BYTES_PER_PRIM_DEF;
      l2r_activate_req->buffer_size     = L2R_BUFFER_SIZE_DEF;
      l2r_activate_req->rate            = X.rate;

      l2rShrdPrm.set_prm[l2rShrdPrm.owner].reset = 0;

      memcpy (&X, &l2rShrdPrm.set_prm_use, sizeof (T_L2R_SET_PRM));

      l2rShrdPrm.state = L2R_ACTIVATE;

      PSENDX (L2R, l2r_activate_req);

      break;
    }

    default:
    {
      cmhL2R_Failure();

      return -1;
    }
  }

  return 0;

#undef X
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_L2R                      |
|                            ROUTINE : psaL2R_Deactivate            |
+-------------------------------------------------------------------+

  PURPOSE : Deactivate L2R.

*/

GLOBAL SHORT psaL2R_Deactivate(void)
{
  TRACE_FUNCTION ("psaL2R_Deactivate()");

  switch (l2rShrdPrm.state)
  {
    case L2R_ENABLED:
    {
      T_DTI_CONN_LINK_ID link_id = dti_cntrl_get_link_id( DTI_ENTITY_L2R, DTI_INSTANCE_NOTPRESENT, DTI_SUB_NO_NOTPRESENT );
      PALLOC (l2r_dti_req, L2R_DTI_REQ);

      l2r_dti_req->dti_conn = L2R_DISCONNECT_DTI;
      l2rShrdPrm.uart_conn = L2R_DISCONNECT_DTI;
      l2r_dti_req->link_id = link_id;
      l2rShrdPrm.state = L2R_DETACH;

      PSENDX (L2R, l2r_dti_req);
      break;
    }

    case L2R_CONNECTED:
    case L2R_ENABLE:
    case L2R_ACTIVATE:
    case L2R_CONNECT:
    {
      PALLOC (l2r_deactivate_req, L2R_DEACTIVATE_REQ);

      l2rShrdPrm.state = L2R_DEACTIVATE;

      PSENDX (L2R, l2r_deactivate_req);
      break;
    }

/* Changed for OMAPS00049111 */
#if defined (FF_SAT_E) || defined (_SIMULATION_)
    case L2R_DEACTIVATED:
      return -1;
#endif /* FF_SAT_E */
  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_L2R                      |
|                            ROUTINE : psaL2R_Enable                |
+-------------------------------------------------------------------+

  PURPOSE : Enable L2R.

*/

GLOBAL SHORT psaL2R_Enable( T_DTI_CONN_LINK_ID  link_id, UBYTE peer )
{
  UBYTE dti_id = EXTRACT_DTI_ID( link_id );

  TRACE_EVENT ("psaL2R_Enable()");

  switch (l2rShrdPrm.state)
  {
    case L2R_CONNECTED:
    {
      PALLOC (l2r_dti_req, L2R_DTI_REQ);

      l2r_dti_req->dti_conn = L2R_CONNECT_DTI;
      l2rShrdPrm.uart_conn  = L2R_CONNECT_DTI;
      l2r_dti_req->link_id  = link_id;

      l2rShrdPrm.state = L2R_ENABLE;

#if defined (FF_WAP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E)
      if (Wap_Call)
      {
        strcpy((char*)l2r_dti_req->entity_name, PPP_NAME);
        l2r_dti_req->dti_direction = DTI_CHANNEL_TO_HIGHER_LAYER;
      }
      else
      {
#endif /* FF_WAP OR FF_GPF_TCPIP or SAT E */
        if (IS_SRC_BT(dti_id))
        {
          TRACE_EVENT("psa_l2rs - BT_ADAPTER Act");
          strcpy((char*)l2r_dti_req->entity_name, BTI_NAME);
          l2r_dti_req->dti_direction   = DTI_CHANNEL_TO_HIGHER_LAYER;
        }
        else if ((peer EQ DTI_ENTITY_PPPC) OR
                  (peer EQ DTI_ENTITY_AAA)
#ifdef FF_SAT_E
                  OR (peer EQ DTI_ENTITY_SIM)
#endif /* FF_SAT_E */
                  )
        {
          strcpy((char*)l2r_dti_req->entity_name, dti_entity_name[peer].name);
          l2r_dti_req->dti_direction = DTI_CHANNEL_TO_HIGHER_LAYER;
        }
        else
        {
          if (peer EQ DTI_ENTITY_UART)
            strcpy((char*)l2r_dti_req->entity_name,UART_NAME);
          else
            strcpy((char*)l2r_dti_req->entity_name,PSI_NAME);
          l2r_dti_req->dti_direction   = DTI_CHANNEL_TO_LOWER_LAYER;
        }
#if defined (FF_WAP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E)
      }
#endif /* FF_WAP || FF_GPF_TCPIP || SAT E */

      if (dti_cntrl_set_conn_parms(link_id, DTI_ENTITY_L2R, DTI_INSTANCE_NOTPRESENT, DTI_SUB_NO_NOTPRESENT) EQ FALSE)
      {
        return -1;
      }

      PSENDX (L2R, l2r_dti_req);

      break;
    }

    case L2R_ENABLE:
    {
      PALLOC (l2r_disc_req, L2R_DISC_REQ);

      l2rShrdPrm.state = L2R_DISCONNECT;

      cmhL2R_Failure();

      PSENDX (L2R, l2r_disc_req);

      break;
    }

    case L2R_ACTIVATE:
    case L2R_CONNECT:
    {
      PALLOC (l2r_deactivate_req, L2R_DEACTIVATE_REQ);

      l2rShrdPrm.state = L2R_DEACTIVATE;

      cmhL2R_Failure();

      PSENDX (L2R, l2r_deactivate_req);

      break;
    }

    default:
    {
      cmhL2R_Failure();

      return -1;
    }
  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_L2R                      |
|                            ROUTINE : psaL2R_Disable               |
+-------------------------------------------------------------------+

  PURPOSE : Disable L2R.

*/

/* Not sure whether this function is needed or not !!

  GLOBAL SHORT psaL2R_Disable(void)
{
  TRACE_FUNCTION ("psaL2R_Disable()");

  switch (l2rShrdPrm.state)
  {
    case L2R_ENABLED:
    {
      PALLOC (l2r_dti_req, L2R_DTI_REQ);

      l2r_dti_req->dti_conn = L2R_DISCONNECT_DTI;
      l2rShrdPrm.uart_conn = L2R_DISCONNECT_DTI;

      l2rShrdPrm.state = L2R_DISABLE;

      PSENDX (L2R, l2r_dti_req);

      break;
    }

    case L2R_CONNECTED:
    case L2R_ENABLE:
    {
      PALLOC (l2r_disc_req, L2R_DISC_REQ);

      l2rShrdPrm.state = L2R_DISCONNECT;

      cmhL2R_Failure();

      PSENDX (L2R, l2r_disc_req);

      break;
    }

    case L2R_ACTIVATE:
    case L2R_CONNECT:
    {
      PALLOC (l2r_deactivate_req, L2R_DEACTIVATE_REQ);

      l2rShrdPrm.state = L2R_DEACTIVATE;

      cmhL2R_Failure();

      PSENDX (L2R, l2r_deactivate_req);

      break;
    }

    default:
    {
      cmhL2R_Failure();

      return -1;
    }
  }

  return 0;
}  */



/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_L2R                      |
|                            ROUTINE : psaL2R_ESC                   |
+-------------------------------------------------------------------+

  PURPOSE : ESC.

*/

GLOBAL SHORT psaL2R_ESC( UBYTE src_id )
{
#ifdef FF_PSI
  T_ACI_DTI_PRC_PSI *src_infos = find_element (psi_src_params, src_id, cmhPSItest_srcId);
#endif /*FF_PSI*/
  TRACE_EVENT ("psaL2R_ESC()");


  switch (l2rShrdPrm.state)
  {
    case L2R_ENABLED:
    {
    T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_ACI};
#ifdef FF_PSI
      if (src_infos NEQ NULL)
        dti_cntrl_est_dpath_indirect ( src_id,
                                     entity_list,
                                     1,
                                     SPLIT,
                                     atiPSI_dti_cb,
                                     DTI_CPBLTY_CMD,
                                     DTI_CID_NOTPRESENT);
      else
#endif /*FF_PSI*/
        dti_cntrl_est_dpath_indirect ( src_id,
                                     entity_list,
                                     1,
                                     SPLIT,
                                     atiUART_dti_cb,
                                     DTI_CPBLTY_CMD,
                                     DTI_CID_NOTPRESENT);

      l2rShrdPrm.state = L2R_ESCAPE;
      break;
    }
    case L2R_CONNECTED:
    case L2R_ENABLE:
    {
      PALLOC (l2r_disc_req, L2R_DISC_REQ);

      l2rShrdPrm.state = L2R_DISCONNECT;

      cmhL2R_Failure();

      PSENDX (L2R, l2r_disc_req);

      break;
    }

    case L2R_ACTIVATE:
    case L2R_CONNECT:
    {
      PALLOC (l2r_deactivate_req, L2R_DEACTIVATE_REQ);

      l2rShrdPrm.state = L2R_DEACTIVATE;

      cmhL2R_Failure();

      PSENDX (L2R, l2r_deactivate_req);

      break;
    }

    default:
    {
      cmhL2R_Failure();

      break;
    }
  }

  return 0;
}

#endif /* DTI */
/*==== EOF ========================================================*/
