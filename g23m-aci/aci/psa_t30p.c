/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_T30P
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
|             primitives send to the protocol stack adapter by T30.
+----------------------------------------------------------------------------- 
*/ 

#if defined (DTI) || defined (FF_FAX)

#ifndef PSA_T30P_C
#define PSA_T30P_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "aci_fd.h"
#include "aci_io.h"
#include "aci.h"
#include "psa.h"
#include "psa_t30.h"
#include "cmh.h"
#include "cmh_t30.h"

#include "aci_lst.h"
#include "psa_uart.h"
#include "cmh_uart.h"
#ifdef FF_PSI
#include "psa_psi.h"
#include "cmh_psi.h"
#include "ati_src_psi.h"
#endif /*FF_PSI*/

#include "psa_cc.h"

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_T30                 |
|                                 ROUTINE : psa_t30_activate_cnf    |
+-------------------------------------------------------------------+

  PURPOSE : processes the T30_ACTIVATE_CNF primitive send by T30.

*/

GLOBAL void psa_t30_activate_cnf
                              (T_T30_ACTIVATE_CNF *t30_activate_cnf)
{
  TRACE_FUNCTION ("psa_t30_activate_cnf()");

  if (!psaCC_ctbIsValid (t30ShrdPrm.cId))
  {
    TRACE_ERROR ("t30ShrdPrm.cId invalid");
    PFREE (t30_activate_cnf);
    return;
  }

  t30ShrdPrm.tbs = t30_activate_cnf->buf_size_tx;
  t30ShrdPrm.rbs = t30_activate_cnf->buf_size_rx;
  t30ShrdPrm.T30_is_activated = TRUE;

  PFREE (t30_activate_cnf);

  cmhT30_Activated( );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_T30                 |
|                                 ROUTINE : psa_t30_cap_ind         |
+-------------------------------------------------------------------+

  PURPOSE : processes the T30_CAP_IND primitive send by T30.

*/

GLOBAL void psa_t30_cap_ind (T_T30_CAP_IND *t30_cap_ind)
{
  TRACE_FUNCTION ("psa_t30_cap_ind()");

  if (!psaCC_ctbIsValid (t30ShrdPrm.cId))
  {
    TRACE_ERROR ("t30ShrdPrm.cId invalid");
    PFREE (t30_cap_ind);
    return;
  }

  memcpy (&t30ShrdPrm.hdlc_rcv, &t30_cap_ind->hdlc_info,
          sizeof (T_hdlc_info));

  PFREE (t30_cap_ind);

  cmhT30_CapRmtSite( );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_T30                 |
|                                 ROUTINE : psa_t30_dti_cnf         |
+-------------------------------------------------------------------+

  PURPOSE : processes the T30_DTI_CNF primitive sent by T30.

*/

GLOBAL void psa_t30_dti_cnf (T_T30_DTI_CNF *t30_dti_cnf)
{
  T_ACI_DTI_PRC *src_infos;
  T_DTI_CONN_LINK_ID link_id = dti_cntrl_get_link_id( DTI_ENTITY_T30, DTI_INSTANCE_NOTPRESENT, DTI_SUB_NO_NOTPRESENT );

  TRACE_FUNCTION ("psa_t30_dti_cnf()");


  switch( t30_dti_cnf->dti_conn )
  {
  case( T30_CONNECT_DTI ):
    /* by now, T30 is connected with UART. It should take care of
     * disactivating flow control */
    src_infos = find_element ( uart_src_params,
                               (UBYTE)t30EntStat.entOwn,
                               cmhUARTtest_srcId );
    if (src_infos EQ NULL)
    {
      TRACE_EVENT_P1("[ERR] psa_t30_dti_cnf: t30EntStat.entOwn=%d not found",
                     t30EntStat.entOwn) ;
      return ;
    }
    BITFIELD_CLEAR (src_infos->data_cntr, UART_DTI_FLOW_OFF);
    dti_cntrl_entity_connected( link_id, DTI_ENTITY_T30, DTI_OK );

    break;

  case( T30_DISCONNECT_DTI ):
    /* it depends here on the context:
    if network is disconnecting the call: */
    if (!psaCC_ctbIsValid (t30ShrdPrm.cId) OR
        (psaCC_ctb(t30ShrdPrm.cId)->calStat EQ CS_DSC_REQ))
    {
      dti_cntrl_entity_disconnected( link_id, DTI_ENTITY_T30 );
    }
    else /* other cases */
    {
      t30EntStat.isTempDisconnected = TRUE;
      dti_cntrl_entity_disconnected( link_id, DTI_ENTITY_T30 );
    }
    break;
  }

  PFREE (t30_dti_cnf);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_T30                 |
|                                 ROUTINE : psa_t30_dti_ind         |
+-------------------------------------------------------------------+

  PURPOSE : processes the T30_DTI_IND primitive sent by T30.

*/

GLOBAL void psa_t30_dti_ind (T_T30_DTI_IND *t30_dti_ind)
{
  TRACE_FUNCTION ("psa_t30_dti_ind()");

  PFREE (t30_dti_ind);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_T30                 |
|                                 ROUTINE : psa_t30_phase_ind       |
+-------------------------------------------------------------------+

  PURPOSE : processes the T30_PHASE_IND primitive sent by T30.

*/

GLOBAL void psa_t30_phase_ind (T_T30_PHASE_IND *t30_phase_ind)
{
#ifdef FF_PSI
  T_ACI_DTI_PRC_PSI *src_infos = find_element (psi_src_params, 
                            (UBYTE)t30EntStat.entOwn, cmhPSItest_srcId);
#endif /*FF_PSI*/
  TRACE_FUNCTION ("psa_t30_phase_ind()");

  if (!psaCC_ctbIsValid (t30ShrdPrm.cId))
  {
    TRACE_ERROR ("t30ShrdPrm.cId invalid");
    PFREE (t30_phase_ind);
    return;
  }

  switch ( t30_phase_ind->phase )
  {
    case MSG_PHASE:
      /* send connect message with flow control */

      R_AT( RAT_CONNECT, t30EntStat.entOwn )
        ( t30EntStat.curCmd, cmhT30_GetDataRate(), t30ShrdPrm.cId+1, TRUE );
      if (IS_SRC_BT(t30EntStat.entOwn))
      {
        T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_BLUETOOTH, DTI_ENTITY_T30};
        dti_cntrl_est_dpath((UBYTE)t30EntStat.entOwn, entity_list, 2, SPLIT, T30_connect_dti_cb);
      }
      else
      {
        T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_T30};
        dti_cntrl_est_dpath_indirect ( (UBYTE)t30EntStat.entOwn,
                                       entity_list,
                                       1,
                                       SPLIT,
                                       T30_connect_dti_cb,
                                       DTI_CPBLTY_SER,
                                       DTI_CID_NOTPRESENT);
      }
      break;

    case BCS_PHASE:
      {
        T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_ACI};
#ifdef FF_PSI
        if (src_infos NEQ NULL)
          dti_cntrl_est_dpath_indirect ( (UBYTE)t30EntStat.entOwn,
                                       entity_list,
                                       1,
                                       SPLIT,
                                       atiPSI_dti_cb,
                                       DTI_CPBLTY_CMD,
                                       DTI_CID_NOTPRESENT);
        else 
#endif /*FF_PSI*/
          dti_cntrl_est_dpath_indirect ( (UBYTE)t30EntStat.entOwn,
                                       entity_list,
                                       1,
                                       SPLIT,
                                       atiUART_dti_cb,
                                       DTI_CPBLTY_CMD,
                                       DTI_CID_NOTPRESENT);
      }
      break;

    default:
      break;
  }
  PFREE (t30_phase_ind);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_T30                 |
|                                 ROUTINE : psa_t30_cmpl_ind        |
+-------------------------------------------------------------------+

  PURPOSE : processes the T30_CMPL_IND primitive send by T30.

*/

GLOBAL void psa_t30_cmpl_ind (T_T30_CMPL_IND *t30_cmpl_ind)
{
  UBYTE cmpl;

  TRACE_FUNCTION ("psa_t30_cmpl_ind()");

  cmpl = t30_cmpl_ind->cmpl;

  PFREE (t30_cmpl_ind);

  if (!psaCC_ctbIsValid (t30ShrdPrm.cId))
  {
    TRACE_ERROR ("t30ShrdPrm.cId invalid");
    return;
  }

  switch (cmpl)
  {
    case CMPL_DCN:
    {
      cmhT30_Disconnected( );
      break;
    }

    case CMPL_EOM:
    {
      cmhT30_NextDoc( );
      break;
    }

    case CMPL_EOP:
    {
      cmhT30_TransCmpl( );
      break;
    }

    case CMPL_PI:
    {
      cmhT30_ProcIntInst( );
      break;
    }

    default:
    {
      break;
    }
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_T30                 |
|                                 ROUTINE : psa_t30_deactivate_cnf  |
+-------------------------------------------------------------------+

  PURPOSE : processes the T30_DEACTIVATE_CNF primitive send by T30.

*/

GLOBAL void psa_t30_deactivate_cnf
                           (T_T30_DEACTIVATE_CNF *t30_deactivate_cnf)
{
  TRACE_FUNCTION ("psa_t30_deactivate_cnf()");

  PFREE (t30_deactivate_cnf);

  cmhT30_Deactivated( );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_T30                 |
|                                 ROUTINE : psa_t30_eol_ind         |
+-------------------------------------------------------------------+

  PURPOSE : processes the T30_EOL_IND primitive send by T30.

*/

GLOBAL void psa_t30_eol_ind (T_T30_EOL_IND *t30_eol_ind)
{
  TRACE_FUNCTION ("psa_t30_eol_ind()");

  t30ShrdPrm.eol = t30_eol_ind->eol;

  PFREE (t30_eol_ind);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_T30                 |
|                                 ROUTINE : psa_t30_error_ind       |
+-------------------------------------------------------------------+

  PURPOSE : processes the T30_ERROR_IND primitive send by T30.

*/

GLOBAL void psa_t30_error_ind (T_T30_ERROR_IND *t30_error_ind)
{
  T_DTI_CONN_LINK_ID link_id = dti_cntrl_get_link_id( DTI_ENTITY_T30, DTI_INSTANCE_NOTPRESENT, DTI_SUB_NO_NOTPRESENT );

  TRACE_FUNCTION ("psa_t30_error_ind()");

  if (!psaCC_ctbIsValid (t30ShrdPrm.cId))
  {
    TRACE_ERROR ("t30ShrdPrm.cId invalid");
    PFREE (t30_error_ind);
    return;
  }

  t30ShrdPrm.err_cause = t30_error_ind->cause;

  PFREE (t30_error_ind);

  /* tell dti_mng that an error occured: not connected anymore */
  dti_cntrl_entity_disconnected( link_id, DTI_ENTITY_T30 );

  cmhT30_Failure();
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_T30                 |
|                                 ROUTINE : psa_t30_preamble_ind    |
+-------------------------------------------------------------------+

  PURPOSE : processes the T30_PREAMBLE_IND primitive send by T30.

*/

GLOBAL void psa_t30_preamble_ind
                               (T_T30_PREAMBLE_IND *t30_preamble_ind)
{
  TRACE_FUNCTION ("psa_t30_preamble_ind()");

  PFREE (t30_preamble_ind);

  cmhT30_PreambleRcvd( );
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_T30                 |
|                                 ROUTINE : psa_t30_report_ind      |
+-------------------------------------------------------------------+

  PURPOSE : processes the T30_REPORT_IND primitive send by T30.

*/

GLOBAL void psa_t30_report_ind (T_T30_REPORT_IND *t30_report_ind)
{
  TRACE_FUNCTION ("psa_t30_report_ind()");

  t30ShrdPrm.report.dir   = t30_report_ind->dir;
  t30ShrdPrm.report.l_buf = t30_report_ind->sdu.l_buf;

  memcpy (t30ShrdPrm.report.buf,
          t30_report_ind->sdu.buf,
          t30_report_ind->sdu.l_buf >> 3);

  PFREE (t30_report_ind);

  cmhT30_HDLCRpt( );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_T30                 |
|                                 ROUTINE : psa_t30_sgn_ind         |
+-------------------------------------------------------------------+

  PURPOSE : processes the T30_SGN_IND primitive send by T30.

*/

GLOBAL void psa_t30_sgn_ind (T_T30_SGN_IND *t30_sgn_ind)
{
  TRACE_FUNCTION ("psa_t30_sgn_ind()");

  if (!psaCC_ctbIsValid (t30ShrdPrm.cId))
  {
    TRACE_ERROR ("t30ShrdPrm.cId invalid");
    PFREE (t30_sgn_ind);
    return;
  }

  t30ShrdPrm.sgn_snd = t30_sgn_ind->sgn;

  PFREE (t30_sgn_ind);

  switch (t30ShrdPrm.sgn_snd)
  {
    case SGN_CRP:
    case SGN_FCS_ERR:
    case SGN_NO_RES:
    {
      psaT30_Capabilities();
      break;
    }

    case SGN_FTT:
    {
      cmhT30_FTT( );
      break ;
    }

    case SGN_DCN:
    {
      cmhT30_Disconnect( );
      break;
    }

    case SGN_EOM:
    {
      cmhT30_DocReceived( );
      break;
    }

    case SGN_EOP:
    {
      switch (t30ShrdPrm.faxStat)
      {
        case FS_RCV_DOC:
          cmhT30_ProcEnd( );
          break;
        case FS_SND_DOC:
          ppmPendFlg = TRUE;
          if( psaT30_Ppm() < 0 )  /* T30 PPM request */
          {
            TRACE_EVENT( "FATAL RETURN psaT30 in cmhT30_PPMRcvd" );
          }
          ppmPendFlg = FALSE;
          t30ShrdPrm.sgn_snd = SGN_NOT_USED;
          break;
        default:
          TRACE_FUNCTION ("psa_t30_sgn_ind() - SGN_EOP ERROR");
          break;
      }
      break;
    }

    case SGN_MPS:
    {
      switch (t30ShrdPrm.faxStat)
      {
        case FS_RCV_DOC:
          cmhT30_PageReceived( );
          break;
        case FS_SND_DOC:
          pageSentFlg = TRUE;
          ppmPendFlg = TRUE;
          if( psaT30_Ppm() < 0 )  /* T30 PPM request */
          {
            TRACE_EVENT( "FATAL RETURN psaT30 in cmhT30_PPMRcvd" );
          }
          ppmPendFlg = FALSE;
          t30ShrdPrm.sgn_snd = SGN_NOT_USED;
          cmhT30_NextPage( );
          break;
        default:
          TRACE_FUNCTION ("psa_t30_sgn_ind() - SGN_MPS ERROR");
          break;
      }
      break;
    }

    case SGN_PIP:
    {
      cmhT30_ProcInt( );
      break;
    }

    case SGN_PIN:
    {
      cmhT30_ProcInt( );
      break;
    }

    case SGN_PRI_EOM:
    {
      cmhT30_DocReceivedPRI( );
      break;
    }

    case SGN_PRI_EOP:
    {
      cmhT30_ProcEndPRI( );
      break;
    }

    case SGN_PRI_MPS:
    {
      cmhT30_PageReceivedPRI( );
      break;
    }

    case SGN_RTN:
    {
      cmhT30_RTN();
      break;
    }

    case SGN_RTP:
    {
      cmhT30_RTP();
      break;
    }

    case SGN_NOT_USED:
    default:
    {
      break;
    }
  }
}
#endif /* DTI OR FF_FAX */

/*==== EOF =========================================================*/

