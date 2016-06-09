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
|  Purpose :  This module defines the functions for the attachment
|             capability of the module Radio Resource.
+-----------------------------------------------------------------------------
*/

#ifndef RR_ATTP_C
#define RR_ATTP_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_RR

/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stdlib.h>
#include <stddef.h>     /* offsetof */
#include "typedefs.h"
#include "pcm.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_rr.h"
#include "tok.h"
#include "rr.h"
#include "rr_em.h"
#include "cl_shrd.h"

#if defined (_SIMULATION_)
#include <stdio.h>
#endif


/*==== EXPORT =====================================================*/

/*==== PRIVATE =====================================================*/
/*==== PROTOTYPES =================================================*/

LOCAL void att_align_sc_meas (T_MPH_MEASUREMENT_IND * mph_meas_ind,
                              UBYTE                   index);

/*==== VARIABLES ==================================================*/
/*
 * -------------------------------------------------------------------
 * PRIMITIVE Processing functions
 * -------------------------------------------------------------------
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_mph_error_ind          |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_ERROR_IND received from PL.

*/

#define MAX_ERROR_CS    9
#define MAX_ERROR_IDLE   4

GLOBAL void att_mph_error_ind (T_MPH_ERROR_IND *mph_error_ind)
{
  GET_INSTANCE_DATA;
  UBYTE index;

  TRACE_FUNCTION ("att_mph_error_ind()");

  switch (GET_STATE (STATE_ATT))
  {
    case ATT_CS2:
      /*
       * during cell selection
       */
      switch (mph_error_ind->cs)
      {
        case CS_BCCH_READ_ERROR:
          /*
           * BCCH read error
           */
          rr_data->bcch_error++;
          if (rr_data->bcch_error EQ MAX_ERROR_CS)
          {
            /*
             * maximum of BCCH read errors reached
             * start next sync request if possible
             */
            rr_data->bcch_error = 0;
            srv_clear_list (&rr_data->cr_data.cd.ncell_list);
            if (cs_sync_next ())
            {
              SET_STATE (STATE_ATT, ATT_CS1);
            }
            break;
          }
          break;

        case CS_DOWN_LINK_FAIL:
          /*
           * downlink failure,
           * layer 1 has received too many undecodable paging messages
           * that means the synchronisation to the BS is lost
           */
          TRACE_EVENT ("downlink failure");

          EM_DOWNLINK_FAILURE;

          /*
           * stop the controlling timer
           */
          TIMERSTOP (T_RESELECT);

          /*
           * start cell selection, actually its questionable
           * if this ever happens, maybe except for the primitive crossing
           * case
           */
/*XY:n don't inform GRR, but call start_cs anyway */
#ifdef GPRS
          att_start_cell_selection_gprs ((UBYTE)(rr_data->sc_data.mm_started),FULL_SEARCH_MODE);
#else
          att_start_cell_selection (rr_data->sc_data.mm_started, CS_NOT_PARALLEL,FULL_SEARCH_MODE);
#endif
         break;

      }
      break;

    case ATT_CS3:
      /*
       * during cell reselection
       */
      switch (mph_error_ind->cs)
      {
        case CS_BCCH_READ_ERROR:
          /*
           * BCCH read error
           */
          rr_data->bcch_error++;
          TRACE_EVENT_P2 ("bcch_error=%u (%u)", rr_data->bcch_error, MAX_ERROR_CS);
          if (rr_data->bcch_error EQ MAX_ERROR_CS)
          {
            /*
             * the maximum of read errors is reached
             * try the next cell for cell reselection
             */
#ifdef GPRS
            if(rr_data->gprs_data.cr_pbcch_active EQ TRUE)
            {
              att_cell_reselection_gprs_failed();
            }
            else
            {
#endif
              rr_data->bcch_error = 0;
              rr_data->pag_rec    = FALSE;
              srv_clear_stored_prim (MPH_PAGING_IND);
              att_continue_cell_reselect ();
#ifdef GPRS
            }
#endif
          }
          break;

        case CS_DOWN_LINK_FAIL:
          /*
           * downlink failure,
           * layer 1 has received too many undecodable paging messages
           * that means the synchronisation to the BS is lost
           */
          TRACE_EVENT ("downlink failure");
          EM_DOWNLINK_FAILURE;

          /*
           * try the next candidate for the cell reselection
           */
          att_continue_cell_reselect () ;
          break;

        case CS_NC_SYNC_FAILED:
          /*
           * Cell has no valid synchronization information
           */
          TRACE_EVENT ("No Valid Sync info");

          /*
           * try the next candidate for the cell reselection
           */
          att_continue_cell_reselect () ;
          break;

      }
      break;

    case ATT_DEDICATED:
      /*
       * during dedicated mode
       */
      switch (mph_error_ind->cs)
      {
        case CS_RADIO_LINK_FAIL:
          /*
           * radio link failure.
           * That means layer 1 has received too many
           * undecodable SACCH blocks in the downlink
           * The synchronisation to the BS is lost.
           */
          TRACE_EVENT ("radio link failure");

          /* 
           * dynamic configuration command : IHO 
           * Lock the DUT to the cell it is already camping -
           * Ignore the Radio Link Failures.
           */

          if(rr_data->dyn_config.iho AND (rr_data->sc_data.ch_mode EQ CM_AMR))
          {
            TRACE_EVENT("MPH_ERROR_IND : IHO");
          }
          else
          {
            EM_RADIO_LINK_FAILURE;

            rr_data->net_lost = TRUE;

            /*
             * Inform MM about the radio link failure and
             * start cell reslection
             */
            att_code_rr_abort_ind (RRCS_ABORT_RAD_LNK_FAIL);
            att_stop_dedicated();
          }
          break;
      }
      break;

    case ATT_IDLE:
    case ATT_CON_EST:
      /*
       * during idle mode or connection establishment
       */
      switch (mph_error_ind->cs)
      {
        case CS_BCCH_READ_ERROR:
          /*
           * A BCCH read error has occured.
           * It is checked whether the BCCH reading
           * is done for a PLMN search activity
           * originated by RR, else it is ignored.
           */
          index = att_get_index (mph_error_ind->arfcn);
          if (index EQ CR_INDEX)
          {
            rr_data->bcch_error++;
            if (rr_data->bcch_error EQ MAX_ERROR_IDLE)
            {
              /*
               * if the maximum of read errors is reached
               * the next cell is tried.
               */
              rr_data->bcch_error = 0;
              TIMERSTOP (T_RESELECT);
              srv_clear_list (&rr_data->cr_data.cd.ncell_list);
              cs_sync_next ();
            }
            break;
          }
          break;

        case CS_DOWN_LINK_FAIL:
          /*
           * downlink failure
           */
          TRACE_EVENT ("downlink failure");
          EM_DOWNLINK_FAILURE;

          rr_data->net_lost = TRUE;

          if(GET_STATE (STATE_ATT) EQ ATT_CON_EST)
          {
            switch (rr_data->ms_data.establish_cause)
            {
#ifdef GPRS
              case ESTCS_GPRS_1P:
              case ESTCS_GPRS_SB:
              case ESTCS_GPRS_PAGING:
                SET_STATE(STATE_GPRS, GPRS_PIM_BCCH);
                break;
              case ESTCS_PAGING:
                dat_rr_release_ind(RRCS_ABNORM_UNSPEC, 0);
#else
              case ESTCS_PAGING:
#endif
                break;
              default:
                dat_rr_release_ind(RRCS_ABNORM_UNSPEC, 0);
                break;
            }
          }

          /*
           * Only start reselection, if isn´t any PLMN search active
           */
          if (rr_data->start_cell_reselection)
          {
/*XY:n inform GRR, and      wait for CR_RSP
              should only happen in PIM_BCCH */
#ifdef GPRS
            att_start_cell_reselection_gprs (CELL_RESELECTION);
#else
            att_start_cell_reselection (CELL_RESELECTION);
#endif
          }
          break;

      }
      break;

    default:
      break;
  }


  PFREE (mph_error_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_mph_sync_ind           |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_SYNC_IND received from PL.

*/

GLOBAL void att_mph_sync_ind (T_MPH_SYNC_IND *mph_sync_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_mph_sync_ind()");

  switch (GET_STATE (STATE_ATT))
  {
    case ATT_CS2:
    case ATT_CS3:
      if (mph_sync_ind->cs EQ CS_SYS_INFO_1_NOT_NEEDED)
      {
        /*
         * layer 1 indicates that system information message type 1
         * is not available. Update the bitmap of the read messages.
         */
        att_set_sys_info_read (SYS_INFO_1_READ, CR_INDEX);
      }
#ifdef GPRS
      else if(mph_sync_ind->cs EQ CS_SCELL_IS_SYNCED)
      {
        if(rr_data->gprs_data.cr_pbcch_active)
        {
          PALLOC(sync_ind, RRGRR_SYNC_IND);
          sync_ind->sync_res = SYNC_OK;
          PSENDX(GRR, sync_ind);

          SET_STATE(STATE_GPRS,GPRS_PIM_PBCCH);
          rr_data->gprs_data.cr_pbcch_active = FALSE;
          rr_data->nc_data[CR_INDEX].arfcn = NOT_PRESENT_16BIT;
          SET_STATE (STATE_ATT, ATT_IDLE);
        }
      }
#endif
      break;

#ifdef GPRS
    case ATT_IDLE:
      {
        T_GPRS_DATA          *gprs_data          = &rr_data->gprs_data;
        T_RRGRR_EXT_MEAS_REQ *rrgrr_ext_meas_req = gprs_data->rrgrr_ext_meas_req;
        T_RRGRR_EXT_MEAS_CNF *rrgrr_ext_meas_cnf = gprs_data->rrgrr_ext_meas_cnf;

        if ( mph_sync_ind->cs EQ CS_STOP_PLMN_SEARCH AND
             rrgrr_ext_meas_req NEQ NULL )
        {
          rrgrr_ext_meas_cnf->call_ref = rrgrr_ext_meas_req->call_ref;
          if ( gprs_data->ext_meas_ctrl & EXT_MEAS_RESET )
            rrgrr_ext_meas_cnf->xmeas_cause = EXT_MEAS_RESET;
          else
            rrgrr_ext_meas_cnf->xmeas_cause = EXT_MEAS_OK;

          att_ext_meas_end(FALSE);
          SET_STATE (STATE_CELL_SEL, CS_IDLE);
          PSENDX ( GRR, rrgrr_ext_meas_cnf );
        }
      }
      break;
#endif

    default:
      break;
  }
  PFREE (mph_sync_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_mph_measurement_ind    |
+--------------------------------------------------------------------+

  PURPOSE : Process MPH_MEASUREMENT_IND received from layer 1.

*/

GLOBAL void att_mph_measurement_ind (T_MPH_MEASUREMENT_IND *mph_measurement_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_mph_measurement_ind()");

  /*
   * Defines the time in frame numbers from the last measurements.
   */
  rr_data->ms_data.fn_offset = (ULONG)mph_measurement_ind->fn_offset;

  switch (GET_STATE (STATE_ATT))
  {
    case ATT_CS2:
      /*
       * FTA problem with 20.19 testcase
       * This function is used to eliminate poor serving cell
       * RX level values due to spurious measurements.
       */
#if defined(_SIMULATION_)
      TRACE_EVENT_WIN_P5 ("ATT_CS2: MPH_MEASUREMENT_IND[%u] rx=%d %svalid (NC%u[%u])",
        mph_measurement_ind->arfcn, mph_measurement_ind->rx_lev_full,
        mph_measurement_ind->valid ? "" : "in", SC_INDEX,
        rr_data->nc_data[SC_INDEX].arfcn);
#endif
      att_align_sc_meas (mph_measurement_ind, SC_INDEX);
      break;

    case ATT_CS3:
      /*
       * clear the barrier for the candidate if exists and
       * update the fieldstrength value
       */
#if defined(_SIMULATION_)
      TRACE_EVENT_WIN_P5 ("ATT_CS3: MPH_MEASUREMENT_IND[%u] rx=%d %svalid (NC%u[%u])",
        mph_measurement_ind->arfcn, mph_measurement_ind->rx_lev_full,
        mph_measurement_ind->valid ? "" : "in",
        CR_INDEX, rr_data->nc_data[CR_INDEX].arfcn);
#endif
      reset_tnnn (CR_INDEX);
      // rr_data->nc_data[CR_INDEX].rxlev = mph_measurement_ind->rx_lev_full;
      att_align_sc_meas (mph_measurement_ind, CR_INDEX);
      break;

    case ATT_DEDICATED:

      if (mph_measurement_ind->valid)
      {
        /*
         * if the measurement report is valid, store it
         * for later use (building of measurement message
         * and ranking of cells if coming back to idle mode).
         */
        memcpy (&rr_data->ms_data.measurement_report, mph_measurement_ind,
                sizeof (T_MPH_MEASUREMENT_IND));
        
        /*Store most recently received timing advance for SC*/
        rr_data->sc_data.new_ta    = rr_data->ms_data.measurement_report.otd/2;
        att_set_tim_advance_info();
        /*
         * This function is used to eliminate poor serving cell
         * RX level values due to spurious measurements.
         */
        att_align_sc_meas (mph_measurement_ind, SC_INDEX);

        /*
         * Set fieldstrength in dedicated mode, use rxlev sub due to DTX
         */
#ifdef FF_PS_RSSI
        RX_SetValue (mph_measurement_ind->rx_lev_sub,
                     mph_measurement_ind->rx_qual_sub,
                     RX_ACCE_UNAVAILABLE);
#else
        RX_SetValue (mph_measurement_ind->rx_lev_sub);
#endif
      }

      reset_tnnn (SC_INDEX);

      /*
       * increase power on time to delete forbidden
       * location area list after 12 hours
       */
      att_increase_power_on_time ((USHORT)(rr_data->ms_data.fn_offset /
                                   PERIOD_1_SEC));

      /*
       * build the uplink SACCH message Measurement Report
       */
#if defined (REL99) && defined (TI_PS_FF_EMR)
      if ( ((rr_data->sc_data.enh_para_status EQ ENH_PARA_IDLE) OR
            (rr_data->sc_data.enh_para_status EQ ENH_PARA_DEDICATED)) AND
            rr_data->sc_data.emr_data_current.rep_type EQ REP_TYPE_ENH )
        dat_code_enh_measure_report (mph_measurement_ind);
      else
#endif
      dat_code_measure_report (mph_measurement_ind);
      break;

    case ATT_IDLE:
#ifdef GPRS
#if defined (REL99) && defined (TI_PS_FF_EMR)
      rr_data->sc_data.emr_data_current.scale_used_last = 
        mph_measurement_ind->scale_used;
#endif
      if(att_check_sync_results(mph_measurement_ind))
        return;
#endif
      /*
       * special trick for FTA testcase 20.7.
       * The first paging is delayed until the
       * first measurement from layer 1 arrives.
       */
      rr_data->first_meas_received = TRUE;

      /*
       * two dynamic configs to perform
       * fast or slow cell reselection in the lab
       */
      if (rr_data->dyn_config.fcr)
        mph_measurement_ind->rx_lev_full = 0;

      if (rr_data->dyn_config.scr)
      {
        rr_data->dyn_config.scr++;
        if (rr_data->dyn_config.scr < mph_measurement_ind->rx_lev_full)
          mph_measurement_ind->rx_lev_full -= rr_data->dyn_config.scr;
        else
          mph_measurement_ind->rx_lev_full  = 0;
      }

      if (mph_measurement_ind->valid)
      {
        /*
         * if the measurement report from layer 1 is valid
         *
         * set RX value
         */
#ifdef FF_PS_RSSI
        RX_SetValue (mph_measurement_ind->rx_lev_full,
                     mph_measurement_ind->rx_qual_full,
                     rr_data->nc_data[SC_INDEX].select_para.rxlev_access_min);
#else
        RX_SetValue (mph_measurement_ind->rx_lev_full);
#endif

        /*
         * store the measurement report for later use, remove
         * bad neighbourcells and analyze the data from layer 1
         * (remove cells which are not inside the report,
         * store new cells, update counter etc.).
         */
        mph_measurement_ind->rx_lev_sub = mph_measurement_ind->rx_lev_full;
        memcpy (&rr_data->ms_data.measurement_report, mph_measurement_ind,
                 sizeof (T_MPH_MEASUREMENT_IND));

        /*  L1 always report BCCH RXLev as Zero in Idle mode, so copy 
         *  BCCH RXLev as receivied level (rx_lev_full) on BCCH
         *  This will be used when we go into dedicated mode
         *  And comback to Idle mode within 480MS Or
         *  Dedicated mode was very-short duration and
         *  RR didn't receive any valid measurement report into
         *  dedicate mode. If we don't do this Cell-Reselction 
         *  will fail as rr_data->nc_data[SC_INDEX].rxlev will be zero
         *  Because it is assigned as bcch_rxlev_of_sc of measurement_report
         *  in function att_build_back_from_dedicated. 
         */
        rr_data->ms_data.measurement_report.bcch_rxlev_of_sc = mph_measurement_ind->rx_lev_full;

        att_remove_bad_ncell (mph_measurement_ind);
        att_analyze_measure_report (mph_measurement_ind);

        if( rr_data->c_ncell_bcch EQ NOT_INITIALISED )
          rr_data->c_ncell_bcch = mph_measurement_ind->ncells.no_of_ncells;

        if( (rr_data->c_ncell_bcch EQ 0) AND (rr_data->resel_pending EQ TRUE) )
        {
          PFREE (mph_measurement_ind);
          #ifdef GPRS
          att_start_cell_reselection_gprs(CELL_RESELECTION_RACH);
          #else
          att_start_cell_reselection(CELL_RESELECTION_RACH);
          #endif
          rr_data->resel_pending = FALSE;
          rr_data->c_ncell_bcch = NOT_INITIALISED;
          return;
        }
        /*
         * check whether a paging has been stored before receiving
         * the measurement from layer 1.
         */
        if (rr_data->pag_rec)
        {
          srv_use_stored_prim ();
          rr_data->pag_rec = FALSE;
        }

        /*
         * delay cell reselection if network search is still ongoing
         * or RR initiated plmn search has started.
         */
        att_check_reselect_decision (rr_data->start_cell_reselection);

        /*
         * increase power on time to delete forbidden
         * location area list after 12 hours
         */
        att_increase_power_on_time ((USHORT)(rr_data->ms_data.fn_offset /
                                     PERIOD_1_SEC));

      }
      break;

  case ATT_CON_EST:
      /*
       * Measurements from layer 1 during connection establishment
       */
      if (mph_measurement_ind->valid)
      {
        /*
         * store the measurement report for later use, remove
         * bad neighbourcells and analyze the data from layer 1
         * (remove cells which are not inside the report,
         * store new cells, update counter etc.).
         */
        mph_measurement_ind->rx_lev_sub = mph_measurement_ind->rx_lev_full;
        memcpy (&rr_data->ms_data.measurement_report, mph_measurement_ind,
                 sizeof (T_MPH_MEASUREMENT_IND));

        /*  L1 always report BCCH RXLev as Zero in Connection Establishment
	 *  mode, so copy BCCH RXLev as receivied level (rx_lev_full).
         *  This will be used when we go into dedicated mode
         *  And comback to Idle mode within 480MS Or
         *  Dedicated mode was very-short duration and
         *  RR didn't receive any valid measurement report into
         *  dedicate mode. If we don't do this Cell-Reselction 
         *  will fail as rr_data->nc_data[SC_INDEX].rxlev will be zero
         *  Because it is assigned as bcch_rxlev_of_sc of measurement_report
         *  in function att_build_back_from_dedicated. 
         */
        rr_data->ms_data.measurement_report.bcch_rxlev_of_sc = mph_measurement_ind->rx_lev_full;

        att_remove_bad_ncell (mph_measurement_ind);
        att_analyze_measure_report (mph_measurement_ind);

        /*
         * make a cell reselection decision, but do not start it.
         */
        att_check_reselect_decision (FALSE);

        /*
         * increase power on time to delete forbidden
         * location area list after 12 hours
         */
        att_increase_power_on_time ((USHORT)(rr_data->ms_data.fn_offset /
                                     PERIOD_1_SEC));
      }
      break;

    default:
      break;
  }
  PFREE (mph_measurement_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_rr_activate_req        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive RR_ACTIVATE_REQ received from MM.

*/

GLOBAL void att_rr_activate_req (T_RR_ACTIVATE_REQ *rr_activate_req)
{
  GET_INSTANCE_DATA;
#ifdef GPRS
  T_loc_area_ident * lai = &rr_data->nc_data[SC_INDEX].lai;
#endif /* GPRS */

  TRACE_FUNCTION ("att_rr_activate_req()");

  /*
   * set cell test operation
   */
  rr_data->cell_test_operation = rr_activate_req->cell_test;
  rr_data->cs_data.check_hplmn = rr_activate_req->check_hplmn;
  TRACE_EVENT_P1 ( "Cell Test Operation =%d", rr_data->cell_test_operation );
  
  switch (GET_STATE (STATE_ATT))
  {
    case ATT_NULL:
#ifdef GPRS
      if(rr_activate_req->gprs_indication AND
         rr_data->gprs_data.start_proc EQ START_PROC_NOTHING)
      {
        att_set_gprs_indication(rr_activate_req->gprs_indication);
        att_rrgrr_cr_ind(CR_NORMAL);
        rr_data->gprs_data.start_proc = START_PROC_ACTIVATE_REQ;
        if ( srv_check_stored_prim (RR_ACTIVATE_REQ) )
          srv_clear_stored_prim(RR_ACTIVATE_REQ);

        if( !srv_store_prim ((T_PRIM *)D2P(rr_activate_req)) )
        {
          /*
           * primitive already in queue or storing failed
           */
          PFREE (rr_activate_req);
        }
        /* on CR_RSP use_stored_prim is called */
      }
      else
        att_handle_rr_act_req (TRUE, rr_activate_req);
#else
      att_handle_rr_act_req (TRUE, rr_activate_req);
#endif
      break;

    case ATT_CS_INIT:
#ifdef GPRS
        if(rr_activate_req->gprs_indication AND
           rr_data->gprs_data.start_proc EQ START_PROC_NOTHING)
        {
          TRACE_EVENT("Activating GPRS in ATT_CS_INIT");
          att_set_gprs_indication(rr_activate_req->gprs_indication);
          att_rrgrr_cr_ind(CR_NORMAL);
          rr_data->gprs_data.start_proc = START_PROC_ACTIVATE_REQ;
          /* go ahead for storing the activate_req message */
        }
        else
        {
#endif
          att_handle_rr_act_req (FALSE, rr_activate_req);
          break;
#ifdef GPRS
       }
#endif
    /*lint -fallthrough*/

    case ATT_CS1:
      if ( GET_STATE (STATE_CELL_SEL) EQ CS_IDLE_ACTIVE )    
      {
#ifdef GPRS
        if(rr_activate_req->gprs_indication AND
           rr_data->gprs_data.start_proc EQ START_PROC_NOTHING AND
           rr_data->ms_data.req_mm_service EQ FUNC_LIM_SERV_ST_SRCH)
        {
          TRACE_EVENT("Activating GPRS in ATT_CS1");
          att_set_gprs_indication(rr_activate_req->gprs_indication);
          att_rrgrr_cr_ind(CR_NORMAL);
          rr_data->gprs_data.start_proc = START_PROC_ACTIVATE_REQ;
          /* go ahead for storing the activate_req message */
        }
        else
        {
#endif
          att_handle_rr_act_req (FALSE, rr_activate_req);
          return;
 #ifdef GPRS
       }
#endif
      }
    /*lint -fallthrough*/
    case ATT_CS3:
    case ATT_DEDICATED:
      /*
       * store rr activate request during cell reselection and while dedicated
       * only store the latest instance of an ACTIVATE_REQ
       */
      if ( srv_check_stored_prim (RR_ACTIVATE_REQ) )
        srv_clear_stored_prim(RR_ACTIVATE_REQ);

      if( !srv_store_prim ((T_PRIM *)D2P(rr_activate_req)) )
      {
        /*
         * primitive already in queue or storing failed
         */
        PFREE (rr_activate_req);
      }
      break;
    case ATT_CS2:
    case ATT_NO_SERVICE:
    case ATT_IDLE:
    case ATT_CON_EST:
      
#ifdef GPRS

      if(rr_activate_req->gprs_indication AND
         rr_activate_req->op.func NEQ FUNC_NET_SRCH_BY_MMI AND
         rr_data->gprs_data.start_proc EQ START_PROC_NOTHING)
       {
         if(
            ( rr_data->ms_data.operation_mode EQ
                ( (rr_activate_req->op.ts << 7) +  (rr_activate_req->op.m  << 6) +
                  (rr_activate_req->op.sim_ins << 5)
                )
             )
             AND
             ( rr_data->ms_data.req_mm_service EQ rr_activate_req->op.func)
             AND
             ( GET_STATE(STATE_GPRS)  EQ GPRS_NULL)
             AND
             ( rr_data->nc_data[SC_INDEX].si13_loc_ind NEQ SI13_NOT_PRESENT )
             AND
             (GET_STATE(STATE_ATT) EQ ATT_IDLE )
             AND
             (dat_plmn_equal_req (  lai->mcc,
                                    lai->mnc,
                                    rr_activate_req->plmn.mcc,
                                    rr_activate_req->plmn.mnc) )
            )
         {
           TRACE_EVENT("gprs activation - cell reselection to same cell");
           rr_data->gprs_data.start_proc = START_PROC_GPRS_ACT;
           rr_data->sc_data.selection_type = CELL_RESELECTION_ON_GPRS_ACT;
           att_set_gprs_indication(rr_activate_req->gprs_indication);
           PFREE (rr_activate_req);
         }
         else
         {
           rr_data->gprs_data.start_proc = START_PROC_ACTIVATE_REQ;
           att_set_gprs_indication(rr_activate_req->gprs_indication);

           /* on CR_RSP use_stored_prim is called */
           if ( srv_check_stored_prim (RR_ACTIVATE_REQ) )
             srv_clear_stored_prim(RR_ACTIVATE_REQ);
           if( !srv_store_prim ((T_PRIM *)D2P(rr_activate_req)) )
           {

              /*
               * primitive already in queue or storing failed
               */
             PFREE (rr_activate_req);
           }
         }
        
         att_rrgrr_cr_ind(CR_NORMAL);
      }
      else if (rr_activate_req->op.func EQ FUNC_NET_SRCH_BY_MMI AND
               (GET_STATE(STATE_GPRS) EQ GPRS_PTM_BCCH  OR
                GET_STATE(STATE_GPRS) EQ GPRS_PTM_PBCCH OR
                GET_STATE(STATE_GPRS) EQ GPRS_PAM_BCCH  OR
                GET_STATE(STATE_GPRS) EQ GPRS_PAM_PBCCH   ))
      {
         rr_data->gprs_data.start_proc = START_PROC_NOTHING;
        /*
         * send Please Retry to the MMI instead of
         * storing the primitive
         */
         rr_data->sc_data.found_entries = 0;
         att_code_rr_abort_ind(RRCS_ABORT_PTM);
         PFREE (rr_activate_req);
      }
      else if (rr_activate_req->op.func EQ FUNC_NET_SRCH_BY_MMI)
      {
        /* this is a parallel search, don't inform GRR */
        rr_data->gprs_data.start_proc = START_PROC_NOTHING;
        att_handle_rr_act_req (FALSE, rr_activate_req);
      }
      else if(
               (rr_activate_req->gprs_indication EQ FALSE)  AND
               (rr_activate_req->op.func NEQ FUNC_NET_SRCH_BY_MMI) AND
               (GET_STATE(STATE_ATT) EQ ATT_IDLE ) AND
               (rr_data->ms_data.req_mm_service EQ rr_activate_req->op.func) AND
               (
                 rr_data->ms_data.operation_mode EQ
                 ((rr_activate_req->op.ts << 7)+(rr_activate_req->op.m  << 6)+(rr_activate_req->op.sim_ins << 5))
               ) AND
               (GET_STATE(STATE_GPRS) EQ GPRS_PIM_BCCH)
             )
      {
          att_set_gprs_indication(rr_activate_req->gprs_indication);
          att_set_sys_info_read(SYS_INFO_13_READ, SC_INDEX);
          att_build_idle_req(SC_INDEX, MODE_SYS_INFO_CHANGE);
          att_code_rr_act_cnf();
          PFREE(rr_activate_req);
      }
      else /* called after, the CR_RSP is received or
             if GPRS is not activated */
        att_handle_rr_act_req (FALSE, rr_activate_req);
#else
      att_handle_rr_act_req (FALSE, rr_activate_req);
#endif
      break;

    default:
      PFREE (rr_activate_req);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_rr_deactivate_req      |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive RR_DEACTIVATE_REQ received from MM.

*/

GLOBAL void att_rr_deactivate_req (T_RR_DEACTIVATE_REQ *rr_deactivate_req)
{
  GET_INSTANCE_DATA;
  UBYTE tim_idx;
  PREUSE (rr_deactivate_req, mph_deactivate_req, MPH_DEACTIVATE_REQ);

  TRACE_FUNCTION ("att_rr_deactivate_req()");

  EM_DEACTIVATION;

#if defined(_SIMULATION_FFS_)
  if(!dat_test_sim_available())
  {
    /* store the white list from RAM to FFS */
    if(rr_data->ms_data.rr_service EQ FULL_SERVICE)
    {
      cs_store_white_list();
    }

    /* store the Black list from RAM to FFS */
    cs_store_black_list();
  }
#endif
   
  /*
   * stop any timer which might be running
   */
  for (tim_idx = 0; tim_idx < NUM_OF_RR_TIMERS; tim_idx++)
    TIMERSTOP (tim_idx);
  /*
   * reset process states and initialize the
   * gsm related data.
   */
  dat_att_null ();
  att_init_gsm_data ();
#ifdef GPRS
  gprs_init_gprs_data();
#endif

  cs_init_process();
  SET_STATE (STATE_ATT, ATT_NULL);

  /*
   * deactivate layer 1
   */
  PSENDX (PL, mph_deactivate_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_rr_sync_req            |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive RR_SYNC_REQ received from MM.

*/

GLOBAL void att_rr_sync_req (T_RR_SYNC_REQ *rr_sync_req)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_rr_sync_req()");

  TRACE_EVENT_P2 ("RR_SYNC_REQ cs=%x att st=%u", rr_sync_req->synccs, GET_STATE (STATE_ATT));

  if ((rr_sync_req->synccs NEQ NOT_PRESENT_16BIT) AND
      (rr_sync_req->synccs NEQ SYNCCS_LAI_ALLOW))
  {
    rr_data->mode_after_dedi = MODE_CELL_RESELECTION;
    TRACE_EVENT ("rr_sync_req: MODE_CELL_RESELECTION");
    #if defined(DL_TRACE_ENABLED)
      {
        sprintf (dl_trace_buf, "sync_req cs=%04x ->mode=%u #%u:%s",
          rr_sync_req->synccs, rr_data->mode_after_dedi, __LINE__, __FILE__);
        DL_OFFLINE_TRACE (dl_trace_buf);
      }
    #endif  /* DL_TRACE_ENABLED */
  }

  if (rr_sync_req->kcv.v_kc)
  {
    /*
     * change of cipher parameter
     */
    memcpy (rr_data->ms_data.new_kc, &rr_sync_req->kcv.kc, KC_STRING_SIZE);
    rr_data->ms_data.cksn = rr_sync_req->cksn;
  }/*if v_kc*/

  if (rr_sync_req->tmsi_struct.v_mid)
  {
    /*
     * change of TMSI (a new value is defined)
     */
    rr_data->ms_data.tmsi_available = TRUE;
    rr_data->ms_data.tmsi_binary    = rr_sync_req->tmsi_struct.tmsi_dig;
    /*
     * forward the new TMSI to layer 1
     */
    att_mph_identity_req ();
  }/* if v_mid*/

  if (rr_sync_req->op.v_op)
  {
    /*
     * change of the operation mode, typically
     * change from automatic to manual mode
     */
    rr_data->ms_data.operation_mode =
      (rr_sync_req->op.ts << SHIFT_FOR_SIM_TYPE) +
      (rr_sync_req->op.m  << SHIFT_FOR_SEARCH_OFFSET) +
      (rr_sync_req->op.sim_ins << SHIFT_FOR_SIM_INSERTED);
    att_set_func (rr_sync_req->op.func);

    switch (GET_STATE (STATE_ATT))
    {
      case ATT_NO_SERVICE:
      case ATT_IDLE:
      case ATT_CON_EST:
        /*
         * changes from automatic to manual mode and vice versa
         * may lead to start or stop of HPLMN search.
         */
        if (rr_data->ms_data.rr_service EQ FULL_SERVICE)
          att_start_registration_timer ();
        break;
      default:
        break;
    }/*STATE_ATT*/

  }/*if v_op*/

  switch (GET_STATE (STATE_ATT))
  {
    case ATT_CS1:
      switch (rr_sync_req->synccs)
      {
        case SYNCCS_TMSI_CKSN_KC_INVAL:
        case SYNCCS_TMSI_CKSN_KC_INVAL_NO_PAG:
          /*
           * store the primitive during cell selection (power measurements)
           */
           if (!srv_store_prim ((T_PRIM *)D2P(rr_sync_req)))
           {
             PFREE (rr_sync_req);
           }/*if*/
           return;
        default:
          break;
      }/*synccs*/
      break;

    case ATT_CS2:
      switch (rr_sync_req->synccs)
      {
        case SYNCCS_TMSI_CKSN_KC_INVAL:
        case SYNCCS_TMSI_CKSN_KC_INVAL_NO_PAG:
          /*
          * location updating reject or authentication
          * failure indicates that a new start of the cell
          * selection is needed.
          */
          TIMERSTOP (T_RESELECT);
/*XY:n don't inform GRR , but call start_cs anyway */
#ifdef GPRS
          att_start_cell_selection_gprs (RR_ORIGINATED,FULL_SEARCH_MODE);
#else
          att_start_cell_selection (RR_ORIGINATED, CS_NOT_PARALLEL,FULL_SEARCH_MODE);
#endif
          break;
        default:
          break;
      }/*synccs*/
      break;

    case ATT_CS3:
    case ATT_DEDICATED:
    case ATT_NO_SERVICE:
    case ATT_IDLE:
    case ATT_CON_EST:
      switch (rr_sync_req->synccs)
      {
        case SYNCCS_TMSI_CKSN_KC_INVAL:
          /*
          * clear TMSI, CKSN and KC
          */
          att_clear_reg_without_imsi ();
          break;

        case SYNCCS_TMSI_CKSN_KC_INVAL_NO_PAG:
          /*
           * clear TMSI, IMSI, CKSN and KC
           */
          att_clear_registration_data ();
          break;
        default:
          break;
      }/*synccs*/
      break;

    default:
      break;/*state*/
  }/*STATE_ATT*/

  switch (rr_sync_req->synccs)
  {
    case SYNCCS_EPLMN_LIST:
      /*
       * If sync info contains an EPLMN list
       * then this is copied regardless of state.
       * When EPLMN is not available stored EPLMN
       * is marked as invalid.
       */
      att_copy_eplmn_list(&rr_sync_req->eq_plmn_list);
      break;

    case SYNCCS_LAI_ALLOW:
      
      /* Store Last Registered PLMN data from the white list */
      rr_data->cs_data.last_reg_plmn.v_plmn = V_PLMN_PRES;

      memcpy (rr_data->cs_data.last_reg_plmn.mcc, rr_data->cs_data.white_list.last_sc_lac.mcc, SIZE_MCC);
      memcpy (rr_data->cs_data.last_reg_plmn.mnc, rr_data->cs_data.white_list.last_sc_lac.mnc, SIZE_MNC);

      TRACE_EVENT("LRPLMN copied from white list");

      /*
       * successful end of location updating request,
       * remove lai from forbidden list if stored
       */
      att_rem_lai_from_forb_list (&rr_sync_req->plmn,
                                  rr_sync_req->lac);
      break;

    case SYNCCS_LAI_NOT_ALLOW:
      /*
       * location updating reject, lai must be added to the
       * forbidden lai list.
       */
      att_add_to_forb_list (FORBIDDEN_LIST_NORMAL,
                            &rr_sync_req->plmn,
                            rr_sync_req->lac);
      /* Specifications 24.008 (section 4.4.4.7) -
       * # 12: (Location Area not allowed);
       * The MS shall perform a cell selection when back to the MM IDLE state 
       * according to 3GPP TS 03.22 and 3GPP TS 25.304. 
       */
      rr_data->ms_data.req_mm_service = FUNC_NET_SRCH_BY_MMI;
      att_start_cell_selection (RR_ORIGINATED, CS_PARALLEL,NORMAL_SEARCH_MODE);

      break;

    case SYNCCS_LAI_NOT_ALLOW_FOR_ROAMING:
      /*
       * location updating reject, lai must be added to the
       * forbidden lai list for roaming.
       */
      att_add_to_forb_list (FORBIDDEN_LIST_ROAMING,
                            &rr_sync_req->plmn,
                            rr_sync_req->lac);
      break;

    case SYNCCS_TMSI_INVAL:
      /*
       * TMSI is invalid. Clear in RR and inform layer 1
       */
      rr_data->ms_data.tmsi_available = FALSE;
      att_mph_identity_req ();
      break;

    case SYNCCS_LIMITED_SERVICE:
      /* Plmn received matches with LRPLMN, flush LRPLMN */
      if (rr_sync_req->plmn.v_plmn EQ V_PLMN_PRES)
      {
        if(!memcmp(rr_sync_req->plmn.mcc, rr_data->cs_data.last_reg_plmn.mcc, SIZE_MCC) AND
           !memcmp(rr_sync_req->plmn.mnc, rr_data->cs_data.last_reg_plmn.mnc, SIZE_MNC)
          )        
        {
          rr_data->cs_data.last_reg_plmn.v_plmn = V_PLMN_NOT_PRES;
          TRACE_EVENT ("LRPLMN Cleared");
        }
      }

      /*
       * Indication to RR that only limited service
       * is possible due to location updating reject cause
       */
      rr_data->ms_data.rr_service = LIMITED_SERVICE;
      TRACE_EVENT_P7 ("CLEAR plmn, (old MCC/MNC=%x%x%x/%x%x%x) #%u",
        rr_data->ms_data.plmn.mcc[0],
        rr_data->ms_data.plmn.mcc[1],
        rr_data->ms_data.plmn.mcc[2],
        rr_data->ms_data.plmn.mnc[0],
        rr_data->ms_data.plmn.mnc[1],
        rr_data->ms_data.plmn.mnc[2],
        __LINE__);
      memset (&rr_data->ms_data.plmn, 0, sizeof (T_plmn));
      att_set_rr_service_info();

      switch (GET_STATE (STATE_ATT))
      {
        case ATT_NO_SERVICE:
        case ATT_IDLE:
        case ATT_CON_EST:
          att_start_registration_timer();
          break;
        default:
          break;
      }/*STATE_ATT*/

#ifdef GPRS
      if(att_gprs_is_avail())
      {
/*XY: inform GRR, and       wait for CR_RSP */
/*             att_rrgrr_cr_ind(CR_NORMAL); */
/*             rr_data->gprs_data.start_proc = START_PROC_ENTER_LIMITED; */

        /*
         * During cell reselection, we should not send the primitive 
         * RRGRR_GPRS_SI13_IND to GRR to inform Limited Service. 
         * As this primitive is also used to mark the end of cell reselection.
         */
        if( GET_STATE(STATE_ATT) NEQ ATT_CS3)
          att_signal_gprs_support();
        /* action will follow after cr_rsp */
      }
#endif

      break;

    case SYNCCS_ACCC:
      rr_data->ms_data.access_classes = rr_sync_req->accc;
      break; /* SIM's access class by SAT changed */

    default:
      if(!rr_sync_req->op.v_op  OR
         !rr_sync_req->kcv.v_kc OR
         !rr_sync_req->tmsi_struct.v_mid)
        TRACE_ERROR ("Unexpected sync event");
      break;
  }/*synccs*/

  PFREE (rr_sync_req);

}/*att_rr_sync_reg*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_mph_meas_rep_cnf            |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive mph_meas_rep_cnf from ALR.

*/

#ifdef GPRS
GLOBAL void att_mph_meas_rep_cnf(T_MPH_MEAS_REP_CNF* mph_meas_rep_cnf)
{
  UBYTE count;
  PALLOC(rrgrr_meas_rep_cnf, RRGRR_MEAS_REP_CNF);
  TRACE_FUNCTION("rrgrr_meas_rep_cnf()");
  
  /*lint -e661 (Possible access of out-of-bounds pointer (26 beyond end of data) by operator '[') */
  /*lint -e662 (Possible creation of out-of-bounds pointer (26 beyond end of data) by operator '[') */
  for(count=0;count<RRGRR_MEAS_REP_LIST_SIZE;count++)
  {
    rrgrr_meas_rep_cnf->meas_res[count].arfcn = mph_meas_rep_cnf->meas_rep[count].arfcn;
    rrgrr_meas_rep_cnf->meas_res[count].bsic  = mph_meas_rep_cnf->meas_rep[count].bsic;
    rrgrr_meas_rep_cnf->meas_res[count].rxlev = mph_meas_rep_cnf->meas_rep[count].rx_lev;
  }
  PFREE(mph_meas_rep_cnf);   
  PSENDX( GRR, rrgrr_meas_rep_cnf );
  /*lint +e662 (Possible creation of out-of-bounds pointer (26 beyond end of data) by operator '[') */
  /*lint +e661 (Possible access of out-of-bounds pointer (26 beyond end of data) by operator '[') */
}
#endif



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_mph_meas_order_cnf     |
+--------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void att_mph_meas_order_cnf (T_MPH_MEAS_ORDER_CNF* meas)
{
  PFREE(meas);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_align_sc_meas          |
+--------------------------------------------------------------------+

  PURPOSE :

  index == SC_INDEX:  In this case the function is used to eliminate
                      poor serving cell RX level values due to
                      spurious measurements.

                      This function is related to FTA test case 20.19.

  index == CR_INDEX:  In this case the function is used to set the
                      right aligned level of a delayed measurement report
                      during cell reselection (ATT_CS3) to the cell
                      with the CR_INDEX.

*/

LOCAL void att_align_sc_meas (T_MPH_MEASUREMENT_IND * mph_meas_ind,
                              UBYTE                   index)
{
  GET_INSTANCE_DATA;
  UBYTE i;

  TRACE_FUNCTION ("att_align_sc_meas()");

  if (mph_meas_ind->valid)
  {
    if (rr_data->nc_data[index].arfcn EQ mph_meas_ind->arfcn)
    {
      /*
       * update the serving cell or cr cell fieldstrength
       */
#if defined(_SIMULATION_)
      TRACE_EVENT_WIN_P3 ("att_align_sc_meas(idx=%u): SC/CR=[%u], rx=%d",
        index, mph_meas_ind->arfcn, mph_meas_ind->rx_lev_full);
#endif
      rr_data->nc_data[index].rxlev = mph_meas_ind->rx_lev_full;
    }
    else
    {
      /*
       * check all neighbourcells
       */
      for (i=0;i<mph_meas_ind->ncells.no_of_ncells;i++)
      {
        /*
         * if the cell is also inside the neighbourcells, take
         * this fieldstrength value.
         */
        if (mph_meas_ind->ncells.arfcn[i] EQ rr_data->nc_data[index].arfcn)
        {
#if defined(_SIMULATION_) 
          TRACE_EVENT_WIN_P3 ("att_align_sc_meas(idx=%u): nc[%u] found, rx=%d",
            index, mph_meas_ind->ncells.arfcn[i], mph_meas_ind->ncells.rx_lev[i]);
#endif
          rr_data->nc_data[index].rxlev = mph_meas_ind->ncells.rx_lev[i];
          return;
        }
      }
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_rr_sync_hplmn_req      |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive RR_SYNC_HPLMN_REQ received from MM.

*/

GLOBAL void att_rr_sync_hplmn_req (T_RR_SYNC_HPLMN_REQ *rr_sync_hplmn_req)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_rr_sync_hplmn_req()");
  memcpy (&rr_data->ms_data.ahplmn, &rr_sync_hplmn_req->plmn, sizeof (T_plmn));
  rr_data->ms_data.v_eq_plmn = FALSE;
  memset(&rr_data->ms_data.eq_plmn_list, 0xFF, SIZE_EPLMN);
  PFREE(rr_sync_hplmn_req);
}

#if defined FF_EOTD
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_mph_ncell_pos_ind      |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_NCELL_POS_IND received from PL.

*/

GLOBAL void att_mph_ncell_pos_ind (T_MPH_NCELL_POS_IND *mph_ncell_pos_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_mph_ncell_pos_ind()");

  if ( mph_ncell_pos_ind->eotd_res EQ EOTD_SUCC )
  {
    /* Forward input primitive to LC adding some parameters */

    T_loc_area_ident  *loc_area_ident;
    T_NC_DATA         *sc;
    int                i,n;

    PALLOC (rrlc_meas_ind, RRLC_MEAS_IND);

    sc = &rr_data->nc_data[SC_INDEX];
    loc_area_ident = &sc->lai;

    memcpy ( &rrlc_meas_ind->mcc[0], &loc_area_ident->mcc[0], 3 * sizeof rrlc_meas_ind->mcc[0]);
    memcpy ( &rrlc_meas_ind->mnc[0], &loc_area_ident->mnc[0], 3 * sizeof rrlc_meas_ind->mnc[0]);
    rrlc_meas_ind->lac              = loc_area_ident->lac;
    rrlc_meas_ind->cell_id          = sc->cell_id;
    rrlc_meas_ind->req_id           = mph_ncell_pos_ind->req_id;
    rrlc_meas_ind->tav              = mph_ncell_pos_ind->ta;
    rrlc_meas_ind->fn               = mph_ncell_pos_ind->fn;
    rrlc_meas_ind->eotd_sc_res      = mph_ncell_pos_ind->eotd_sc_res;
    rrlc_meas_ind->eotd_sc_res1     = mph_ncell_pos_ind->eotd_sc_res1;
    n =
    rrlc_meas_ind->c_eotd_nc_res    = mph_ncell_pos_ind->c_eotd_nc_res;
    for ( i = 0; i < n; ++i )
      rrlc_meas_ind->eotd_nc_res[i] = mph_ncell_pos_ind->eotd_nc_res[i];

    if ( GET_STATE (STATE_ATT) EQ ATT_DEDICATED
#ifdef GPRS
         OR (GET_STATE (STATE_ATT) EQ ATT_IDLE      AND
             GET_STATE(STATE_GPRS) EQ GPRS_PTM_BCCH AND
             GET_STATE(STATE_GPRS) EQ GPRS_PTM_PBCCH   )
#endif
       )
      rrlc_meas_ind->eotd_mode = EOTD_DEDIC;
    else
      rrlc_meas_ind->eotd_mode = EOTD_IDLE;

    /*
     * If this is a response to an explicit position measurement request,
     * then reset the flag which enables handover indications to LC.
     */

    if ( rr_data->eotd_req_id NEQ NOT_PRESENT_16BIT AND
         rr_data->eotd_req_id EQ mph_ncell_pos_ind->req_id )
         rr_data->eotd_req_id = NOT_PRESENT_16BIT;

    PSENDX (LC, rrlc_meas_ind);
  }
  else
  {
    PALLOC (rrlc_error_ind, RRLC_ERROR_IND);
    rrlc_error_ind->cause = LCS_WRONG_BTS;   /* exact cause is unclear, assume wrong BTS */
    PSENDX (LC, rrlc_error_ind);
  }
  PFREE(mph_ncell_pos_ind);
}
#endif /* FF_EOTD */

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
GLOBAL void att_mph_init_rr_ind(T_MPH_INIT_RR_IND* mph_init_rr_ind)
{
  GET_INSTANCE_DATA;
  UBYTE   freq_bands;
  T_rf_power *rfp = &rr_data->ms_data.rf_cap.rf_power;

  TRACE_FUNCTION ("att_mph_init_rr_ind()");
  rr_csf_get_freq_bands (&freq_bands);
  if (mph_init_rr_ind->freq_bands NEQ freq_bands)
  {
    TRACE_EVENT_P2 ("FFS data %x, doesn't match L1 compilation flag %x", freq_bands, mph_init_rr_ind->freq_bands);
    TRACE_ERROR ("FFS data doesn't match L1 compilation flag");
    TRACE_ASSERT (freq_bands NEQ mph_init_rr_ind->freq_bands);
  }

  if (rfp->pow_class4[IDX_PWRCLASS_900].pow_class NEQ mph_init_rr_ind->pclass_900 OR
      rfp->pow_class4[IDX_PWRCLASS_850].pow_class NEQ mph_init_rr_ind->pclass_850 OR
      rfp->pow_class4[IDX_PWRCLASS_1800].pow_class NEQ mph_init_rr_ind->pclass_1800 OR
      rfp->pow_class4[IDX_PWRCLASS_1900].pow_class NEQ mph_init_rr_ind->pclass_1900)
  {
    TRACE_EVENT ("FFS Power Class info doesn't match L1 Powe Class info");
    TRACE_ERROR ("FFS Power Class info doesn't match L1 Powe Class info");
    TRACE_ASSERT (mph_init_rr_ind->pclass_900 NEQ rfp->pow_class4[IDX_PWRCLASS_900].pow_class);
  }
  PFREE(mph_init_rr_ind);
}
#endif

#endif
