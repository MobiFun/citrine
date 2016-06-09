/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_L2RP
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
|             primitives send to the protocol stack adapter by layer
|             2 Relay.
+----------------------------------------------------------------------------- 
*/ 

#ifdef DTI

#ifndef PSA_L2RP_C
#define PSA_L2RP_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/

#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_fd.h"
#include "aci.h"

#include "dti.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "psa.h"
#include "cmh.h"
#include "cmh_ra.h"
#include "psa_cc.h"
#include "psa_l2r.h"
#include "cmh_l2r.h"

#if defined (FF_WAP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E)
#include "wap_aci.h"
#include "psa_ppp_w.h"
#include "cmh_ppp.h"
#endif /* defined (FF_WAP) || defined (FF_GPF_TCPIP) */

#include "psa_uart.h"
#include "psa_ra.h"
#include "psa_uart.h"
#include "psa_util.h"

#include "sap_dti.h"

#include "dcm_f.h"

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_L2R                 |
|                                 ROUTINE : psa_l2r_activate_cnf    |
+-------------------------------------------------------------------+

  PURPOSE : processes the L2R_L2R_ACTIVATE_CNF primitive send by L2R.

*/

GLOBAL void psa_l2r_activate_cnf
                               (T_L2R_ACTIVATE_CNF *l2r_activate_cnf)
{
  TRACE_FUNCTION ("psa_l2r_activate_cnf()");

  if (!psaCC_ctbIsValid(raShrdPrm.cId))
  {
    TRACE_ERROR ("raShrdPrm.cId invalid");
    PFREE (l2r_activate_cnf);
    return;
  }

  switch (l2rShrdPrm.state)
  {
    case L2R_ACTIVATE:
    {
      if (l2r_activate_cnf->ack_flg EQ L2R_NAK)
      {
        l2rShrdPrm.state = L2R_DEACTIVATED;

        cmhL2R_Failure();
      }
      else
      {
        PALLOC (l2r_connect_req, L2R_CONNECT_REQ);

        l2rShrdPrm.state = L2R_CONNECT;

        PSENDX (L2R, l2r_connect_req);
      }

      break;
    }

    case L2R_DEACTIVATE:
    {
      break;
    }

    default:
    {
      l2rShrdPrm.state = L2R_DEACTIVATED;

      cmhL2R_Failure();

      break;
    }
  }

  PFREE (l2r_activate_cnf);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_L2R                 |
|                                 ROUTINE : psa_l2r_connect_cnf     |
+-------------------------------------------------------------------+

  PURPOSE : processes the L2R_CONNECT_CNF primitive send by L2R.

*/

GLOBAL  void psa_l2r_connect_cnf
                                (T_L2R_CONNECT_CNF *l2r_connect_cnf)
{
  TRACE_FUNCTION ("psa_l2r_connect_cnf()");

  if (!psaCC_ctbIsValid(raShrdPrm.cId))
  {
    TRACE_ERROR ("raShrdPrm.cId invalid");
    PFREE (l2r_connect_cnf);
    return;
  }

  switch (l2rShrdPrm.state)
  {
    case L2R_CONNECT:
    {
      if (l2r_connect_cnf->ack_flg EQ L2R_NAK)
      {
        PALLOC (l2r_deactivate_req, L2R_DEACTIVATE_REQ);

        l2rShrdPrm.state = L2R_DEACTIVATE;

        cmhL2R_Failure();

        PSENDX (L2R, l2r_deactivate_req);
      }
      else
      {
        l2rShrdPrm.state = L2R_CONNECTED;

        cmhCC_L2R_or_TRA_Activated( DTI_ENTITY_L2R, raShrdPrm.cId );
      }

      break;
    }

    case L2R_DEACTIVATE:
    {
      break;
    }

    default:
    {
      PALLOC (l2r_deactivate_req, L2R_DEACTIVATE_REQ);

      l2rShrdPrm.state = L2R_DEACTIVATE;

      cmhL2R_Failure();

      PSENDX (L2R, l2r_deactivate_req);

      break;
    }
  }

  PFREE (l2r_connect_cnf);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_L2R                 |
|                                 ROUTINE : psa_l2r_connect_ind     |
+-------------------------------------------------------------------+

  PURPOSE : processes the L2R_CONNECT_IND primitive send by L2R.

*/

GLOBAL  void psa_l2r_connect_ind
                                (T_L2R_CONNECT_IND *l2r_connect_ind)
{
  TRACE_FUNCTION ("psa_l2r_connect_ind()");

  if (!psaCC_ctbIsValid(raShrdPrm.cId))
  {
    TRACE_ERROR ("raShrdPrm.cId invalid");
    PFREE (l2r_connect_ind);
    return;
  }

  switch (l2rShrdPrm.state)
  {
    case L2R_CONNECT:
    {
      l2rShrdPrm.state = L2R_CONNECTED;

      cmhCC_L2R_or_TRA_Activated( DTI_ENTITY_L2R, raShrdPrm.cId );

      break;
    }

    case L2R_CONNECTED:
    case L2R_ENABLE:
    case L2R_ENABLED:
    /*    case L2R_DISABLE:*/
    {
      PALLOC (l2r_disc_req, L2R_DISC_REQ);

      l2rShrdPrm.state = L2R_DISCONNECT;

      cmhL2R_Failure();

      PSENDX (L2R, l2r_disc_req);

      break;
    }

    case L2R_ACTIVATE:
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

  PFREE (l2r_connect_ind);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_L2R                 |
|                                 ROUTINE : psa_l2r_deactivate_cnf  |
+-------------------------------------------------------------------+

  PURPOSE : processes the L2R_DEACTIVATE_CNF primitive send by L2R.

*/

GLOBAL  void psa_l2r_deactivate_cnf
                          (T_L2R_DEACTIVATE_CNF *l2r_deactivate_cnf)
{
  TRACE_FUNCTION ("psa_l2r_deactivate_cnf()");

  switch (l2rShrdPrm.state)
  {
    default:
    {
      cmhL2R_Failure();
    }
    /*lint -fallthrough */
    case L2R_DEACTIVATE:
    {
      l2rShrdPrm.state = L2R_DEACTIVATED;

      cmhL2R_Deactivated();

      break;
    }

  }

  PFREE (l2r_deactivate_cnf);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_L2R                 |
|                                 ROUTINE : psa_l2r_disc_cnf        |
+-------------------------------------------------------------------+

  PURPOSE : processes the L2R_DISC_CNF primitive send by L2R.

*/

GLOBAL  void psa_l2r_disc_cnf (T_L2R_DISC_CNF *l2r_disc_cnf)
{
  TRACE_FUNCTION ("psa_l2r_disc_cnf()");

  switch (l2rShrdPrm.state)
  {
    default:
    {
      cmhL2R_Failure();
    }
    /*lint -fallthrough */
    case L2R_DISCONNECT:
    {
      PALLOC (l2r_deactivate_req, L2R_DEACTIVATE_REQ);

      l2rShrdPrm.state = L2R_DEACTIVATE;

      PSENDX (L2R, l2r_deactivate_req);

      break;
    }

  }

  PFREE (l2r_disc_cnf);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_L2R                 |
|                                 ROUTINE : psa_l2r_disc_ind        |
+-------------------------------------------------------------------+

  PURPOSE : processes the L2R_DISC_IND primitive send by L2R.

*/

GLOBAL  void psa_l2r_disc_ind (T_L2R_DISC_IND *l2r_disc_ind)
{
  TRACE_FUNCTION ("psa_l2r_disc_ind()");

#if defined (FF_WAP) || defined (FF_SAT_E)
  if (Wap_Call)
  {

    /*
     *  Start terminate the whole PPP Stack if PPP activ
     */
    if((pppShrdPrm.state EQ PPP_ESTABLISHED) OR (pppShrdPrm.state EQ PPP_ESTABLISH))
      cmhPPP_Terminate( DWN );
  }
  else
#endif /* of WAP or SAT E */
  {
    switch (l2rShrdPrm.state)
    {
      default:
      {
        cmhL2R_Failure();
      }
      /*lint -fallthrough  */
      case L2R_ENABLED:
      {
        T_DTI_CONN_LINK_ID link_id = dti_cntrl_get_link_id( DTI_ENTITY_L2R, DTI_INSTANCE_NOTPRESENT, DTI_SUB_NO_NOTPRESENT );
        dti_cntrl_set_dti_id_to_reconnect(EXTRACT_DTI_ID(link_id));
        dti_cntrl_entity_disconnected( link_id, DTI_ENTITY_L2R );
      }
          /* NO BREAK ! */
      /*lint -fallthrough */
      case L2R_CONNECTED:
      /*      case L2R_DISABLE:*/
      {
        PALLOC (l2r_deactivate_req, L2R_DEACTIVATE_REQ);

        l2rShrdPrm.state = L2R_DEACTIVATE;

        PSENDX (L2R, l2r_deactivate_req);

        break;
      }
    }
  }

  PFREE (l2r_disc_ind);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_L2R                 |
|                                 ROUTINE : psa_l2r_error_ind       |
+-------------------------------------------------------------------+

  PURPOSE : processes the L2R_ERROR_IND primitive send by L2R.

*/

GLOBAL  void psa_l2r_error_ind (T_L2R_ERROR_IND *l2r_error_ind)
{
  T_DTI_CONN_LINK_ID link_id;

  TRACE_FUNCTION ("psa_l2r_error_ind()");

#if defined (FF_WAP) || defined (FF_SAT_E)

  if (Wap_Call)
  {

    /*
     *  Start terminate the whole PPP Stack if PPP have established call
     *  or by establish call.
     */

    if((pppShrdPrm.state EQ PPP_ESTABLISHED) OR (pppShrdPrm.state EQ PPP_ESTABLISH))
      cmhPPP_Terminate( DWN );

    PFREE (l2r_error_ind);
    return;
  }

#endif /* of WAP or SAT E */


  link_id = dti_cntrl_get_link_id( DTI_ENTITY_L2R, DTI_INSTANCE_NOTPRESENT, DTI_SUB_NO_NOTPRESENT );

  if (dti_cntrl_is_dti_channel_connected (DTI_ENTITY_L2R, EXTRACT_DTI_ID(link_id)) EQ TRUE)
  {
    dti_cntrl_entity_disconnected( link_id, DTI_ENTITY_L2R );
  }
  {
    PALLOC (l2r_deactivate_req, L2R_DEACTIVATE_REQ);

    l2rShrdPrm.set_prm[l2rShrdPrm.owner].err_cause = l2r_error_ind->cause;

    l2rShrdPrm.state = L2R_DEACTIVATE;

    cmhL2R_Failure();

    PSENDX (L2R, l2r_deactivate_req);
  }

  PFREE (l2r_error_ind);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_L2R                 |
|                                 ROUTINE : psa_l2r_reset_ind       |
+-------------------------------------------------------------------+

  PURPOSE : processes the L2R_RESET_IND primitive send by L2R.

*/

GLOBAL  void psa_l2r_reset_ind (T_L2R_RESET_IND *l2r_reset_ind)
{
  TRACE_FUNCTION ("psa_l2r_reset_ind()");

  switch (l2rShrdPrm.state)
  {
    case L2R_CONNECTED:
    case L2R_ENABLE:
    case L2R_ENABLED:
    /*    case L2R_DISABLE:*/
    case L2R_ACTIVATE:
    case L2R_CONNECT:
    {
      l2rShrdPrm.set_prm[l2rShrdPrm.owner].reset++;

      break;
    }
    /*
    case L2R_ACTIVATE:
    case L2R_CONNECT:
    {
      PALLOC (l2r_deactivate_req, L2R_DEACTIVATE_REQ);

      l2rShrdPrm.state = L2R_DEACTIVATE;

      cmhL2R_Failure();

      PSENDX (L2R, l2r_deactivate_req);

      break;
    }
    */
    default:
    {
      cmhL2R_Failure();

      break;
    }
  }

  PFREE (l2r_reset_ind);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_L2R                 |
|                                 ROUTINE : psa_l2r_statistic_ind   |
+-------------------------------------------------------------------+

  PURPOSE : processes the L2R_STATISTIC_IND primitive send by L2R.

*/

GLOBAL  void psa_l2r_statistic_ind
                             (T_L2R_STATISTIC_IND *l2r_statistic_ind)
{
  TRACE_FUNCTION ("psa_l2r_statistic_ind()");

  l2rShrdPrm.set_prm_use.error_rate = l2r_statistic_ind->error_rate;

  PFREE (l2r_statistic_ind);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_L2R                 |
|                                 ROUTINE : psa_l2r_dti_cnf         |
+-------------------------------------------------------------------+

  PURPOSE : processes the L2R_DTI_CNF primitive send by L2R.

*/

GLOBAL  void psa_l2r_dti_cnf (T_L2R_DTI_CNF *l2r_dti_cnf)
{
  TRACE_FUNCTION ("psa_l2r_dti_cnf()");

  /* store the current DTI id for the case of redirection */
  raShrdPrm.dti_id = EXTRACT_DTI_ID(l2r_dti_cnf->link_id);        

  switch (l2rShrdPrm.state)
  {
    case L2R_ESCAPE:
    {
      if (l2r_dti_cnf->dti_conn NEQ l2rShrdPrm.uart_conn)
      {
        cmhL2R_Failure();
      }
      l2rEntStat.isTempDisconnected = TRUE;
      dti_cntrl_entity_disconnected( l2r_dti_cnf->link_id, DTI_ENTITY_L2R );

      l2rShrdPrm.state = L2R_CONNECTED;
      break;
    }

    case L2R_ENABLE:
    {
      if (l2r_dti_cnf->dti_conn NEQ l2rShrdPrm.uart_conn)
      {
        PALLOC (l2r_disc_req, L2R_DISC_REQ);

        l2rShrdPrm.state = L2R_DISCONNECT;

        cmhL2R_Failure();

        PSENDX (L2R, l2r_disc_req);
      }
      else
      {
#if defined(FF_WAP) OR defined(FF_GPF_TCPIP) OR defined (FF_SAT_E)
        T_ACI_WAP_STATES act_state = Wap_Not_Init;
        if(is_gpf_tcpip_call()) {
          GPF_TCPIP_STATEMENT(act_state = TCPIP_Activation);
        }
        else {
          act_state = IPA_Activated;
        }
#endif
        l2rShrdPrm.state = L2R_ENABLED;

#if defined(FF_WAP) OR defined(FF_GPF_TCPIP) OR defined (FF_SAT_E) OR defined (FF_PPP)
        if ((Wap_Call AND (wap_state EQ act_state)) OR pppShrdPrm.is_PPP_CALL)
        {
          dti_cntrl_entity_connected( l2r_dti_cnf->link_id, DTI_ENTITY_L2R, DTI_OK );
        }
#ifdef FF_SAT_E        
        else if (psa_search_SATSrcId() >= 0)/* sat class c  connection SIM-L2R */
        {
          dti_cntrl_entity_connected( l2r_dti_cnf->link_id, DTI_ENTITY_L2R, DTI_OK );
        }
#endif  /* FF_SAT_E */
        else
        {
          TRACE_EVENT(" error: wrong WAP state ");
        }
#else   /* WAP OR FF_GPF_TCPIP or SAT E */
      dti_cntrl_entity_connected( l2r_dti_cnf->link_id, DTI_ENTITY_L2R, DTI_OK );
#endif  /* of WAP OR FF_GPF_TCPIP or SAT E */
      }

      break;
    }

/*    case L2R_DISABLE:
    {
      if (l2r_dti_cnf->dti_conn NEQ l2rShrdPrm.uart_conn)
      {
        PALLOC (l2r_disc_req, L2R_DISC_REQ);

        l2rShrdPrm.state = L2R_DISCONNECT;

        cmhL2R_Failure();

        PSENDX (L2R, l2r_disc_req);
      }
      else
      {
        l2rShrdPrm.state = L2R_CONNECTED;

        cmhL2R_TRA_Disabled();
      }

      break;
    }*/

    case L2R_DETACH:
    {
      if (l2r_dti_cnf->dti_conn NEQ l2rShrdPrm.uart_conn)
      {
        cmhL2R_Failure();
      }

      {
        PALLOC (l2r_deactivate_req, L2R_DEACTIVATE_REQ);
        l2rShrdPrm.state = L2R_DEACTIVATE;
        /* Set the flag to reconnect the registered device to ACI */
        dti_cntrl_set_dti_id_to_reconnect(EXTRACT_DTI_ID(l2r_dti_cnf->link_id));
        dti_cntrl_entity_disconnected( l2r_dti_cnf->link_id, DTI_ENTITY_L2R );
        PSENDX (L2R, l2r_deactivate_req);
      }

      break;
    }

    case L2R_DISCONNECT:
    case L2R_DEACTIVATE:
    {
      break;
    }

    default:
    {
      PALLOC (l2r_disc_req, L2R_DISC_REQ);

      l2rShrdPrm.state = L2R_DISCONNECT;

      cmhL2R_Failure();

      PSENDX (L2R, l2r_disc_req);

      break;
    }
  }

  PFREE (l2r_dti_cnf);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_L2R                 |
|                                 ROUTINE : psa_l2r_dti_ind        |
+-------------------------------------------------------------------+

  PURPOSE : processes the L2R_DTI_IND primitive send by L2R.

*/

GLOBAL  void psa_l2r_dti_ind (T_L2R_DTI_IND *l2r_dti_ind)
{
  TRACE_FUNCTION ("psa_l2r_dti_ind()");

  /* store the current DTI id for the case of redirection */
  raShrdPrm.dti_id = EXTRACT_DTI_ID(l2r_dti_ind->link_id);        

  switch (l2rShrdPrm.state)
  {
    case L2R_ENABLED:
    {
      l2rEntStat.isTempDisconnected = TRUE;
      /* Set the flag to reconnect the registered device to ACI */
      dti_cntrl_set_dti_id_to_reconnect(EXTRACT_DTI_ID(l2r_dti_ind->link_id));
      dti_cntrl_entity_disconnected( l2r_dti_ind->link_id, DTI_ENTITY_L2R );

      l2rShrdPrm.state = L2R_CONNECTED;

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

  PFREE (l2r_dti_ind);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_L2R                 |
|                                 ROUTINE : psa_l2r_xid_ind         |
+-------------------------------------------------------------------+

  PURPOSE : processes the L2R_XID_IND primitive send by L2R.

*/

GLOBAL  void psa_l2r_xid_ind (T_L2R_XID_IND *l2r_xid_ind)
{
#define X l2rShrdPrm.set_prm_use

  TRACE_FUNCTION ("psa_l2r_xid_ind()");

  X.rlp_vers = l2r_xid_ind->rlp_vers;
  X.k_ms_iwf = l2r_xid_ind->k_ms_iwf;
  X.k_iwf_ms = l2r_xid_ind->k_iwf_ms;
  X.t1       = l2r_xid_ind->t1;
  X.t2       = l2r_xid_ind->t2;
  X.n2       = l2r_xid_ind->n2;
  X.pt       = l2r_xid_ind->pt;
  X.p0       = l2r_xid_ind->p0;
  X.p1       = l2r_xid_ind->p1;
  X.p2       = l2r_xid_ind->p2;

  PFREE (l2r_xid_ind);

#undef X
}
#endif /* DTI */

/*==== EOF =========================================================*/

