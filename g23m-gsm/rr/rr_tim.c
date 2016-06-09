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
|  Purpose :  This Modul defines the timer handling functions
|             for the component DL of the mobile station
+-----------------------------------------------------------------------------
*/

#ifndef RR_TIM_C
#define RR_TIM_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_RR

/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stdlib.h>
#include <stddef.h>     /* offsetof */
#include <stdio.h>      /* sprintf */
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
#ifdef TI_PS_FF_AT_P_CMD_CTREG
#include "cl_shrd.h"
#endif /* TI_PS_FF_AT_P_CMD_CTREG */

/*==== EXPORT =====================================================*/
/*==== PRIVAT =====================================================*/
#if !defined(ELEMENTS)
#define ELEMENTS(array) (sizeof(array)/sizeof(array[0]))
#endif  /* !ELEMENTS */

/*==== VARIABLES ==================================================*/
EXTERN UBYTE test_house;
#ifdef TI_PS_FF_AT_P_CMD_CTREG
EXTERN T_TIME cast2T_Time(UBYTE tab_val);
#else
EXTERN T_TIME lim_service_mode_time[25];
#endif /* TI_PS_FF_AT_P_CMD_CTREG */
/*
 * In the old frame variant most of the timer handling is done
 * in RR. RR has a timer pool and holds the dependency between
 * logical timer and timer handle.
 *
 * With the new frame variant this is simplified. RR uses a constant
 * for the timer and gets back the this constant after timeout.
 *
 * This is the reason why most of the functions are only needed for
 * the old frame variant.
 */

/*==== FUNCTIONS ==================================================*/

#if defined(TIMER_TRACE)
static void  trace_timer (USHORT index, long value)
{
  GET_INSTANCE_DATA;
/* Implements Measure#32: Row */
  static T_S2I_STRING const tim_names[] =
  {
    S2I_STRING("T3110"),
    S2I_STRING("T3122"),
    S2I_STRING("T3126"),
    S2I_STRING("T_RESELECT"),
    S2I_STRING("TREG"),
    S2I_STRING("TABORT"),
    S2I_STRING("T_NO_RESELECT"),
    S2I_STRING("TIM_EXT_MEAS"),
    #if defined FF_EOTD
    S2I_STRING("TAPDU"),
    #endif /* FF_EOTD */
    S2I_STRING("TNNN"),
    S2I_STRING("TCSVALID"),
    S2I_STRING("T_DEDICATED_MODE"),
    S2I_STRING("T_PLMN_SEARCH"),
    S2I_STRING("T_FAST_CS"),
    S2I_STRING("T_NORMAL_CS")
  };
  switch(value)
  {
/* Implements Measure#32: Row 431, 432, 433 and 435 */
    default:/* >0: start */
      TRACE_TIMER_P2 ("T-Start:%s=%lu", S2I_STRING(tim_names[index]), value);
      break;
    case 0:/* EQ 0: stop */
      TRACE_TIMER_P1 ("T-Stop:%s", S2I_STRING(tim_names[index]));
      break;
    case -1:/* -1: expire */
      TRACE_TIMER_P1 ("T-Expired:%s ", S2I_STRING(tim_names[index]));
      break;
    case -2:/* -2: check */
      if(rr_data->t_running[index])
      { 
        TRACE_TIMER_P1 ("T-Check:%s active", S2I_STRING(tim_names[index]));
      }
      else
      { 
        TRACE_TIMER_P1 ("T-Check:%s inactive", S2I_STRING(tim_names[index]));
      }
      break;
  }
}
#else
#define trace_timer(index,value)
#endif  /* TIMER_TRACE */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_TIM                     |
| STATE   : code                ROUTINE : tim_t3110                  |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer T3110. Timer is started after reception
            of channel release message and controls layer 2 disconnection.
            It starts the configuration of idle mode, if layer 2 has
            not signalled in time the disconnection of the layer 2 link.

*/

GLOBAL void tim_t3110 (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tim_t3110()");

  if (GET_STATE (STATE_DAT) EQ DAT_CHAN_REL)
  {
    /*
     * Send STOP_DEDICATED_REQUEST to L1 and wait for confirmation 
     */
    att_stop_dedicated();
  }

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_TIM                     |
| STATE   : code                ROUTINE : tim_t3122                  |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer T3122. Started after reception of an
            immediate assignment reject message. MM does not start
            any new establishment during the timer is running.
            After timeout MM is informed. New attempts from MM can
            be started.

*/

GLOBAL void tim_t3122 (void)
{
  GET_INSTANCE_DATA;
  PALLOC (sync, RR_SYNC_IND);

  TRACE_FUNCTION ("tim_t3122()");

  switch (GET_STATE (STATE_DAT))
  {
    case DAT_NULL:
    case DAT_CELL_RESELECT:
    case DAT_IDLE:
    case DAT_IMM_ASS:
    case DAT_IMM_ASS_1:
      /*
       * send the indication to MM that T3122 has
       * timed-out.
       */
      sync->ciph              = NOT_PRESENT_8BIT;
      sync->chm.ch_mode       = NOT_PRESENT_8BIT;
      sync->synccs            = SYNCCS_T3122_TIM_OUT;
      memset(&sync->mm_info, 0, sizeof(T_mm_info));
      sync->mm_info.valid     = FALSE;
      memset(&sync->bcch_info, 0, sizeof(T_bcch_info));
      sync->bcch_info.v_bcch  = FALSE;

      PSENDX (MM, sync);
      srv_use_stored_prim ();
      break;

    default:
      PFREE (sync);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_TIM                     |
| STATE   : code                ROUTINE : tim_t3126                  |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer T3126. This timer is used for two cases.
            1. It controls that all random bursts are send during a
               certain time. If (why ever) layer 1 does not confirm
               all outgoing random bursts, this timer prevents RR
               from stucking. This functionality is not described in
               GSM and is introduced for security reasons.
            2. It controls the reception of an immediate assignment
               message. This is the normal GSM functionality.

*/

GLOBAL void tim_t3126 (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tim_t3126()");

  if (GET_STATE (STATE_DAT) EQ DAT_IMM_ASS)
  {
    if (rr_data->ms_data.all_conf_received)
    {
      /*
       * Normal GSM functionality. Control of the
       * reception of immediate assignment messages.
       */
      TRACE_EVENT ("Immediate Assignment failed");

      dat_send_release_ind ( RRCS_RND_ACC_FAIL );

      /*
       * Check if we have to do a cell reselection
       * or if we can return directly to idle mode in the current
       * cell
       */
      TRACE_EVENT("check rej_rec");
      if (rr_data->imm_ass_rej_rec NEQ TRUE)
      {
        /*
         * search for idle mode cell
         * (exclude current serving cell, because
         *  the random access was unsuccessfull)
         */
#ifdef GPRS
        if(att_gprs_cell_has_pbcch())
        {
/*XY: inform GRR, and don't wait for CR_RSP we will get a CR_REQ in this case */
          att_rrgrr_cr_ind(CR_REQ_CANDIDATE);
          rr_data->gprs_data.start_proc = START_PROC_NOTHING;
        }
        else
        {
#endif

          TRACE_EVENT_P1("rr_data->c_ncell_bcch: %x",rr_data->c_ncell_bcch);

          /*There are three scenarios to be considered */
          /*
           * i   - The first MPH_MEASUREMENT_IND has not yet arrived
           * ii  - The first MPH_MEASUREMENT_IND has arrived but not all 
           *       the SIs have been received
           * (These are managed below)
           *
           * iii - The first MPH_MEASUREMENT_IND has arrived and all the 
           *       SIs have been received AND a reselection is pending
           * This is managed in 'att_bcch_status_to_decoded()'
           */

         if(((rr_data->first_meas_received EQ TRUE) AND (rr_data->c_ncell_bcch EQ 0)) OR
/* Implements RR Clone findings #8 */
              att_cell_barred_status_cr_no_cr (SC_INDEX)
              OR 
             (rr_data->nc_data[SC_INDEX].c1 <= 0))
          {
/*XY:n inform GRR, and        wait for CR_RSP */
#ifdef GPRS
            att_start_cell_reselection_gprs (CELL_RESELECTION_RACH);
#else
            att_start_cell_reselection (CELL_RESELECTION_RACH);
#endif
          }
          else
          {
            /*
             * continue camping on current SC until indication of ncell BCCH info
             * after first measurement indication
             */
             rr_data->resel_pending = TRUE;
             att_return_to_idle();
          }
#ifdef GPRS
        }
#endif
      }
      else
      {
        /*
         * we have received an IMMEDIATE ASSIGNMENT REJECT
         * (the random access procedure was successfull, meaning
         *  that an answer was received from the network, even
         *  if the answer was negative)
         * In this case we go directly back to the current
         * serving cell
         */
#ifdef GPRS
        if(att_gprs_cell_has_pbcch())
        {
/*XY: inform GRR, and don't wait for CR_RSP */
          att_rrgrr_cr_ind(CR_SUSPENDED_IDLE);
          rr_data->gprs_data.start_proc = START_PROC_NOTHING;
        }
#endif
        att_return_to_idle();
      }
    }
    else
    {
      /*
       * Layer 1 has not confirmed all outgoing random bursts
       * in time. Simulate that all confirmations have received
       * and start second part of connection establishment
       * with waiting for the immediate assignment.
       */
      TRACE_TIMER ("missing random acc cnf");

      TIMERSTART (T3126, T3126_VALUE);
      rr_data->ms_data.all_conf_received = TRUE;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_TIM                     |
| STATE   : code                ROUTINE : tim_treselect              |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer T_RESELECT. The timer controls reception
            of the whole BCCH during cell selection and cell reselection.

*/

GLOBAL void tim_treselect (void)
{ 
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tim_treselect()");

  switch (GET_STATE (STATE_ATT))
  {
    case ATT_CS2:
#ifdef GPRS
      if((rr_data->cr_data.cd.sys_info_read EQ SYS_INFO_EXCEPT_SI13) AND test_house)
      {
        if(rr_data->ms_data.rr_service EQ LIMITED_SERVICE)
        {
          /*
           * We have found a 'potential' cell for limited service 
           */
          rr_data->nc_data[CR_INDEX].si13_loc_ind = SI13_NOT_PRESENT;
          SET_STATE(STATE_GPRS, GPRS_ACTIVATED);
          att_check_bcch_carrier ();
          return;
        }
        else
        {
          /*
           * There is a problem with some R&S tests where SI13 is indicated
           * in SI3/4 but there is no SI13 transmitted by the tester. RR continues
           * to wait to for SI13 having successfully received all other SI's from L1
           *
           * The timeout of T_RESELECT would cause this cell to be ignored and 
           * RR would have instructed L1 to check the next BSIC.
           * Thus, it can happen that after a failed scan of all subsequent frequencies, 
           * MM is indicated with "no cell available" i.e. we lose a GSM-only healthy cell
           *
           * Mark the current arfcn as being eligible for LIM service, which RR can camp 
           * onto on the next pass when looking for emergency cells.
           */
          if(dat_forb_lai_check (CR_INDEX))
          {
          att_store_plmn_in_found_list (&rr_data->nc_data[CR_INDEX].lai);
          }
          else
          {
            TRACE_EVENT ("Do not store Forbidden LAI PLMN in the found PLMN list");
          }
          cs_set_attributes (EMERGENCY_CELL,  rr_data->nc_data[CR_INDEX].arfcn);
          TRACE_EVENT_P1("Setting %x as EMERG", rr_data->nc_data[CR_INDEX].arfcn);
        }
     }
#endif

      /*
       * Sometimes SI 2Ter is indicated, but it is never sent
       * With this patch the inconsistent BCCH is taken.
       * Fix for defect OMAPS00069058: SI13 is indicated on EBCCH, but never
       * received. The fix avoids ignoring this cell on TRESELECT timeout.
       */
      if ( (rr_data->cr_data.cd.sys_info_read EQ SYS_INFO_EXCEPT_2TER)
#ifdef GPRS
          OR
           (rr_data->cr_data.cd.sys_info_read EQ SYS_INFO_EXCEPT_SI13)
#endif
         )
      {
        /*
         * if at least the rest is available, check the
         * BCCH carrier to camp on a cell.
         */
#ifdef GPRS
        if ( (rr_data->cr_data.cd.sys_info_read EQ SYS_INFO_EXCEPT_SI13) AND
             (GET_STATE (STATE_GPRS) EQ GPRS_PIM_BCCH ) 
           )
        {
          rr_data->nc_data[CR_INDEX].si13_loc_ind = SI13_NOT_PRESENT;
          SET_STATE(STATE_GPRS, GPRS_ACTIVATED); 
        }
#endif
        att_check_bcch_carrier ();
      }
      else
      {
        /*
         * initialise the internal data for the next candidate,
         * because the BCCH is not completely available.
         */
        srv_clear_list (&rr_data->cr_data.cd.ncell_list);
        if (cs_sync_next ())
        {
          /*
           * start FB/SB reading for the next channel.
           */
          SET_STATE (STATE_ATT, ATT_CS1);
#ifdef GPRS
          if (GET_STATE (STATE_GPRS) EQ GPRS_PIM_BCCH )
          {
            SET_STATE(STATE_GPRS, GPRS_ACTIVATED);
          }
#endif
          rr_data->old_serving_cell = NOT_PRESENT_8BIT;
        }
      }
      break;

    case ATT_CS3:

      if (rr_data->cr_data.cd.sys_info_read EQ SYS_INFO_EXCEPT_2TER)
      {
        /*
         * if all sys infos except SI 2Ter is available, check the
         * BCCH carrier to camp on a cell.
         * When GPRS is activate, ATT_STATE is changed from ATT_CS3 to
         * ATT_IDLE following reception of rrgrr_cr_req from GRR. Hence
         * it not approprite to check for ATT_STATE for cell reselection
         * completion. A new flag has been introduced for this purpose.
         * This is s temporary solution, untill CQ 24632 is submitted
         * where the SI2TER functionality will be cleaned up.
         */

        rr_data->cr_treselect_exp = FALSE;

        att_check_bcch_carrier ();

        if(rr_data->cr_treselect_exp)
        {
          TRACE_EVENT("Cell Reselection Success");
          /* cell reselection completed */
          return;
        }
      }

      TRACE_EVENT ("cell reselection failed");

      /*
       * reset the paging received flag.
       */
      rr_data->pag_rec = FALSE;
      srv_clear_stored_prim (MPH_PAGING_IND);

      /*
       * Indicate failed reestablishment earlier
       */
      if (rr_data->sc_data.selection_type EQ BACK_FROM_DEDICATED_RLF)
        dat_code_reestablishment_fail ();

      /*
       * start a cell selection after timeout.
       */
      rr_data->old_serving_cell = NOT_PRESENT_8BIT;

    /*XY:n don't inform GRR , but call start_cs anyway */
    
    /* Cell Selection Improvements-LLD section:4.1.3.9
     * Cell reselection fails due to T_RESELECT timer expiry
     * Start Fast search
     */
#ifdef GPRS
       att_start_cell_selection_gprs (RR_ORIGINATED,FAST_SEARCH_MODE);
#else
       att_start_cell_selection (RR_ORIGINATED, CS_NOT_PARALLEL ,FAST_SEARCH_MODE);
#endif

      break;

    case ATT_IDLE:
    case ATT_CON_EST:
      /*
       * Parallel scanning for a channel in idle mode has failed.
       * Initialize the data for the next channel and start
       * FB/SB reading for this channel.
       */
       #ifdef GPRS
      if( rr_data->sc_data.selection_type EQ CELL_RESELECTION_ON_GPRS_ACT)
      {
       /* Cell Selection Improvements-LLD section:4.1.3.9           
        * GPRS activation on current cell fails due to T_RESELECT timer
        * expiry   
        * Start Fast search
        */
        att_start_cell_selection_gprs(MM_ORIGINATED,FAST_SEARCH_MODE);
      }
      else
      {
      #endif
      srv_clear_list (&rr_data->cr_data.cd.ncell_list);
      cs_sync_next ();
      #ifdef GPRS
      }
      #endif
      break;
    case ATT_CS_INIT:
       /* Boot Time Performance Enhancement:           
        * RR has not received the normal RR_ACTIVATE_REQ yet 
        * Free the stored power_cnf and move back to ATT_NULL
        */
      if( srv_check_stored_prim(MPH_POWER_CNF))
      {
        srv_clear_stored_prim(MPH_POWER_CNF);
      }
      SET_STATE(STATE_CELL_SEL, CS_NULL);
      SET_STATE(STATE_ATT, ATT_NULL);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)     MODULE  : RR_TIM                       |
| STATE   : code              ROUTINE : tim_plmn_search_expiry |
+--------------------------------------------------------------------+

  PURPOSE : This timer expires to signal end on Manual cell selection.
  
*/
GLOBAL void tim_plmn_search_expiry (void)
{
  GET_INSTANCE_DATA;
  if(rr_data->ms_data.req_mm_service EQ FUNC_NET_SRCH_BY_MMI)
  {
    /*
    * Mark all ARFCN's as Checked !
    * After the next MPH_BSIC_CNF & MPH_UNITDATA_IND,
    * cs_sync_next_bsic() will return FALSE.
    */
    UBYTE i;
    for ( i=0; i < rr_data->cs_data.max_arfcn; i++ )
    {
      rr_data->cs_data.attributes[i] |= CS_CHECK_FLAG;
    }   
  }  
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)     MODULE  : RR_TIM                       |
| STATE   : code              ROUTINE : tim_tfast_cs                 |
+--------------------------------------------------------------------+

  PURPOSE : This timer expires to signal end on Fast Search. Currently
            this function just traces the Black Listed channels
            CSI-LLD section:4.1.3.4.1.9
*/

GLOBAL void tim_tfast_cs(void)
{
  TRACE_FUNCTION("tim_tfast_cs()");

  srv_trace_black_list();
  srv_trace_white_list();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)     MODULE  : RR_TIM                       |
| STATE   : code              ROUTINE : tim_tnormal_cs               |
+--------------------------------------------------------------------+

  PURPOSE : This timer expires to signal end on Normal search.Currently
            this function just traces the Black Listed channels
            CSI-LLD section:4.1.3.4.1.10                     
*/

GLOBAL void tim_tnormal_cs(void)
{
  TRACE_FUNCTION("tim_tnormal_cs()");

  srv_trace_black_list();
  srv_trace_white_list();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)     MODULE  : RR_TIM                       |
| STATE   : code              ROUTINE : att_reset_registration_timer |
+--------------------------------------------------------------------+

  PURPOSE : This function stops the registration timer early on
            request of MMI / Keypad driver. This function can be called
            to avoid too long periods between search attempts to come
            back to coverage.

*/

GLOBAL void tim_reset_registration_timer (void)
{
  GET_INSTANCE_DATA;
  T_TIME status = 0;

  TRACE_FUNCTION ("tim_reset_registration_timer()");

  /*
   * depending on the current RR service
   */
  switch (rr_data->ms_data.rr_service)
  {
    case NO_SERVICE:
    case LIMITED_SERVICE:
      if (rr_data->ms_data.reg_counter >= 12)
      {
        /*
         * Only if RR has searched already more then 12 times,
         * that means there is a long period between search attempts
         * from some minutes.
         *
         * Get the remaining time of the timer.
         */
        TIMER_STATUS (rr_handle, TREG, &status);
        if (status)
        {
          /*
           * if the timer is just running, stop the timer,
           * reset the attempt counter and simulate timeout
           * by starting the timer again for one second.
           */
          TIMERSTOP (TREG);
          rr_data->ms_data.reg_counter = 0;
          TIMERSTART (TREG, ONE_SEC);
        }
      }
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_TIM                     |
| STATE   : code                ROUTINE : tim_treg                   |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer TREG. Used by RR to recover from
            No service to limited / full service or
            limited service to full service or
            full service (non-HPLMN) to full service (HPLMN)

*/

GLOBAL void tim_treg (void)
{
  GET_INSTANCE_DATA;
  U8 search_mode = 0;
#ifdef GPRS
  UBYTE state;
#endif /* GPRS */
#ifdef TI_PS_FF_AT_P_CMD_CTREG
  BOOL ret;
  UBYTE tab_val;
#endif /* TI_PS_FF_AT_P_CMD_CTREG */
  TRACE_FUNCTION ("tim_treg()");

  /*
   * clear old cell identification to
   * signal with RR_ACTIVATE_IND to MM to force
   * location updatings.
   */
  att_reset_old_lai_rac();
  
  /*
   * due to : "Network not recovered after a manual registration failure"
   * do nothing if measurement is running (RR is already in state ATT_CS1)
   */
  switch (GET_STATE (STATE_ATT))
  {
    case ATT_CS1:
    case ATT_CS2:
      break;

    case ATT_CS3:
    case ATT_DEDICATED:
    case ATT_CON_EST:
      /*
       * handle the expiry event later after state change (by an
       * additional expiry of TREG)
       */
      rr_data->treg_pending = TRUE;
      break;
    case ATT_IDLE:
#ifdef GPRS
      state = GET_STATE(STATE_GPRS);
      switch(state)
      {
        case GPRS_PTM_BCCH:
        case GPRS_PAM_BCCH:
        case GPRS_PAM_PBCCH:
        case GPRS_PTM_PBCCH:
          rr_data->treg_pending = TRUE;
          break;
        default:
          if(GET_STATE(STATE_CELL_SEL) EQ CS_CCO)
            rr_data->treg_pending = TRUE;
          break;
      }
      if(rr_data->treg_pending)
      {
        att_start_registration_timer();
        break;
      }
#endif
    /*lint -fallthrough*/
    default:

    /* Obtain the new search mode
     * CSI-LLD : 4.1.3.4.1.5
     */
    search_mode = cs_get_new_search_mode();

    switch (rr_data->ms_data.rr_service)
    {
      case NO_SERVICE:
        /*
         * actual there is no service. Depending on the
         * last request from MM a limited service search
         * or full service search is started.
         */
        /* XY:n inform GRR, and      wait for CR_RSP */
#ifdef GPRS
        att_start_cell_selection_gprs(RR_ORIGINATED,search_mode);
#else
        att_start_cell_selection(RR_ORIGINATED, CS_NOT_PARALLEL,search_mode);
#endif
        break;

      case LIMITED_SERVICE:
        /*
         * actual there is limited service. Depending on the
         * frequency area a limited service search or full service 
         * search is started.
         */    

          /* Both American and European carriers are detected in 
           * this area. Perform Non-Parallel search only when the 
           * time gap between search attempts exceeds 2min
           * CSI-LLD 4.3
           */
          if(rr_data->ms_data.reg_counter)
          {
#ifdef TI_PS_FF_AT_P_CMD_CTREG
            ret = cl_shrd_get_treg(RR_MOD_LIMSERVICE_TIME,
                                    (UBYTE)(rr_data->ms_data.reg_counter - 1),
                                    &tab_val);
            if (!ret)
            {
              /* Use default on failure */
              tab_val = lim_service_mode_time [rr_data->ms_data.reg_counter-1];
            }
            rr_data->ms_data.reg_time_gap += cast2T_Time(tab_val);
#else
            /* Add the time gap between search attempts */
            rr_data->ms_data.reg_time_gap += 
              lim_service_mode_time[rr_data->ms_data.reg_counter-1];
#endif /* TI_PS_FF_AT_P_CMD_CTREG */
            TRACE_EVENT_P2("[%d]reg_counter, [%d]reg_time_gap", 
              rr_data->ms_data.reg_counter,
              rr_data->ms_data.reg_time_gap);
          }

          if((rr_data->ms_data.reg_counter < 20) AND 
             (rr_data->ms_data.reg_time_gap < rr_data->dyn_config.lim_ser_nps_delay))
          {
            /* Perform parallel search if delay >= 2min. This is to allow time 
             * for emergency calls
             */
            att_start_cell_selection(RR_ORIGINATED, CS_PARALLEL,search_mode); 
          }
          else
          {
            rr_data->ms_data.reg_time_gap = 0;

            /* Perform Non-Parallel search every >= 2min */
#ifdef GPRS
            att_start_cell_selection_gprs(RR_ORIGINATED,search_mode);
#else
            att_start_cell_selection(RR_ORIGINATED, CS_NOT_PARALLEL,search_mode);
#endif          
        }
        break;

      case FULL_SERVICE:
        /*
         * actual there is full service. Nothing to do here.
         */
        break;
    }
    break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_TIM                     |
| STATE   : code                ROUTINE : tim_tnnn                   |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer TNNN

*/
GLOBAL void tim_tnnn (void)
{
  GET_INSTANCE_DATA;
  int   index;
  BOOL  tnnn_active = FALSE;

  for (index = 0; index < (int)ELEMENTS(rr_data->nc_data); index++)
  {
    if (rr_data->nc_data[index].tnnn)
    {
      rr_data->nc_data[index].tnnn--; /* decrement all active timer counts */
      if (rr_data->nc_data[index].tnnn)
        tnnn_active = TRUE;          /* at least one timer count active */
      else
      {
/* Implements Measure#32: Row 442 */
        switch(index)
        {
          case CR_INDEX:
            TRACE_TIMER_P1 ("T-Expiry:TNNN[CR%d]", rr_data->nc_data[index].arfcn);
            break;
          case SC_INDEX:
            TRACE_TIMER_P1 ("T-Expiry:TNNN[SC%d]", rr_data->nc_data[index].arfcn);
            break;
          default:
            TRACE_TIMER_P1 ("T-Expiry:TNNN[%d]", rr_data->nc_data[index].arfcn);
            break;
        }
      }
    }
  }

  if (!tnnn_active)
    TIMERSTOP (TNNN); /* no timer count active -> stop physical timer */
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_TIM                     |
| STATE   : code                ROUTINE : tim_tabort                 |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer TABORT

*/
GLOBAL void tim_tabort (void)
{
  GET_INSTANCE_DATA;
  TRACE_EVENT("tim_tabort TIMEOUT");

  /* Sometimes SI 2Ter is indicated but it is never sent.
   * Wait till expiry of T_RESELECT Timer, to give more time to read SI 2Ter.
   * Do not send RR_ABORT_IND in this special case,
   * as this will cause MMI to show "No Network"
   */
  if( (rr_data->cr_data.cd.sys_info_read EQ SYS_INFO_EXCEPT_2TER) AND
      ((GET_STATE(STATE_ATT) EQ ATT_CS2) OR (GET_STATE(STATE_ATT) EQ ATT_CS3)) )
  {
    TRACE_EVENT("SI2 Ter missing - wait for T_RESELECT expiry");
    TIMERSTART (TABORT, TABORT_VALUE);
    return;
  }

  /*If the MS is carrying out a search for a list of PLMNs, this may take */
  /*longer than 10s and the MMI is not displaying the PLMN name */
  /*Boot Time:If MM has initiated a power scan for quick registration
    *do not send RR_ABORT_IND here. 
    */
  if((rr_data->ms_data.req_mm_service NEQ FUNC_NET_SRCH_BY_MMI) AND
     (rr_data->ms_data.req_mm_service NEQ FUNC_ST_PWR_SCAN))
  {
    att_code_net_lost();
#ifdef FF_PS_RSSI
    RX_SetValue ( 0, RX_QUAL_UNAVAILABLE, RX_ACCE_UNAVAILABLE);
#else
    RX_SetValue (0);
#endif
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS              MODULE  : RR_TIM                     |
| STATE   : code                ROUTINE : check_tnnn                 |
+--------------------------------------------------------------------+

  PURPOSE : Starts timer TABORT

*/
void tstart_tabort( USHORT val )
{
  GET_INSTANCE_DATA;
  if( rr_data->net_lost )
  {
    if(!IS_TIMER_ACTIVE(TABORT))
    {
      TIMERSTART (TABORT, val);
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_TIM                     |
| STATE   : code                ROUTINE : check_tnnn                 |
+--------------------------------------------------------------------+

  PURPOSE : Checks to see if timer TNNN is running or not

*/
static int check_tnnn (void)
{
  GET_INSTANCE_DATA;
  int nc;
  int tnnn_active = FALSE;

  /* check all timer counter for active state */
  for (nc = 0; nc < (int)ELEMENTS(rr_data->nc_data); nc++)
  {
    if (rr_data->nc_data[nc].tnnn)
    {
      tnnn_active = TRUE;
      break;
    }
  }
  return tnnn_active;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_TIM                     |
| STATE   : code                ROUTINE : set_tnnn                   |
+--------------------------------------------------------------------+

  PURPOSE : Sets timer TNNN

*/
GLOBAL void set_tnnn (int index, USHORT value)
{
  GET_INSTANCE_DATA;
/* Implements Measure#32: Row 443 */
  switch (index)
  {
    case CR_INDEX:
      TRACE_TIMER_P2 ("T-Start:TNNN[CR%d]=%d", rr_data->nc_data[index].arfcn, value);
      break;
    case SC_INDEX:
      TRACE_TIMER_P2 ("T-Start:TNNN[SC%d]=%d", rr_data->nc_data[index].arfcn, value);
      break;
    default:
      TRACE_TIMER_P2 ("T-Start:TNNN[%d]=%d", rr_data->nc_data[index].arfcn, value);
      break;
  }

  if (!check_tnnn ())
  {/* up to now there are no timer counter active -> start physical timer */
    trace_timer (TNNN, value);
    rr_data->t_running[TNNN] = 1;

    /*
     * start physical periodically timer.
     */
    TIMER_PSTART (rr_handle, TNNN, ONE_SEC, ONE_SEC);
  }

  /* set the new timer counter */
  rr_data->nc_data[index].tnnn = value / ONE_SEC;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_TIM                     |
| STATE   : code                ROUTINE : reset_tnnn                 |
+--------------------------------------------------------------------+

  PURPOSE : Resets timer TNNN

*/
GLOBAL void reset_tnnn (int index)
{
  GET_INSTANCE_DATA;
  if (rr_data->nc_data[index].tnnn)
  {
/* Implements Measure#32: Row 444 */
    switch (index)
    {
      case CR_INDEX:
        TRACE_TIMER_P1 ("T-Stop:TNNN[CR%d]", rr_data->nc_data[index].arfcn);
        break;
      case SC_INDEX:
        TRACE_TIMER_P1 ("T-Stop:TNNN[SC%d]", rr_data->nc_data[index].arfcn);
        break;
      default:
        TRACE_TIMER_P1 ("T-Stop:TNNN[%d]", rr_data->nc_data[index].arfcn);
        break;
    }

    /* reset the timer counter */
    rr_data->nc_data[index].tnnn = 0;

    if (IS_TIMER_ACTIVE(TNNN) AND !check_tnnn ())
      TIMERSTOP (TNNN);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_TIM                     |
| STATE   : code                ROUTINE : is_tnnn                    |
+--------------------------------------------------------------------+

  PURPOSE : Returns value of timer TNNN

*/
GLOBAL int is_tnnn (int index)
{
  GET_INSTANCE_DATA;
  if (rr_data->nc_data[index].tnnn)
  {
/* Implements Measure#32: Row 446 */
    switch (index)
    {
      case CR_INDEX:
        TRACE_TIMER_P2 ("T-Check:TNNN[CR%d]=%d", rr_data->nc_data[index].arfcn, 
                rr_data->nc_data[index].tnnn*ONE_SEC);
        break;
      case SC_INDEX:
        TRACE_TIMER_P2 ("T-Check:TNNN[SC%d]=%d", rr_data->nc_data[index].arfcn, 
                rr_data->nc_data[index].tnnn*ONE_SEC);
        break;
      default:
        TRACE_TIMER_P2 ("T-Check:TNNN[%d]=%d", rr_data->nc_data[index].arfcn, 
                rr_data->nc_data[index].tnnn*ONE_SEC);
        break;
    }
  }
  else
  {
/* Implements Measure#32: Row 445 */
    switch (index)
    {
      case CR_INDEX:
        TRACE_TIMER_P1 ("T-Check:TNNN[CR%d] inactive", rr_data->nc_data[index].arfcn);
        break;
      case SC_INDEX:
        TRACE_TIMER_P1 ("T-Check:TNNN[SC%d] inactive", rr_data->nc_data[index].arfcn);
        break;
      default:
        TRACE_TIMER_P1 ("T-Check:TNNN[%d] inactive", rr_data->nc_data[index].arfcn);
        break;
    }
  }

  return rr_data->nc_data[index].tnnn;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_TIM                     |
| STATE   : code                ROUTINE : tim_ext_meas               |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer TIM_EXT_MEAS.

*/

GLOBAL void tim_ext_meas (void)
{
  TRACE_FUNCTION ("tim_ext_meas ()");
  dat_emo_stop ( TRUE );
}

#if defined FF_EOTD
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_TIM                     |
| STATE   : code                ROUTINE : tim_apdu                   |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer TAPDU.

*/

GLOBAL void tim_apdu (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tim_apdu ()");
  rr_applic_rx_init ( &rr_data->applic_rx );
}
#endif /* FF_EOTD */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : RR_TIM                   |
| STATE   : code                  ROUTINE : tim_exec_timeout         |
+--------------------------------------------------------------------+

  PURPOSE : execute timeout for the new frame variant.

*/

GLOBAL void tim_exec_timeout (USHORT index)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tim_exec_timeout()");

  if (index >= NUM_OF_RR_TIMERS)
  {
#if defined(TIMER_TRACE)
    SYST_TRACE_P ((SYST, "tim_exec_timeout(%u)", index));
#endif
  }
  else
  {
    if (index NEQ TNNN) /* TNNN timer has its own trace */
    {
      trace_timer (index, -1);
      rr_data->t_running[index] = 0;
    }

    if (rr_data->t_expire[index])
      rr_data->t_expire[index] ();
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : RR_TIM                   |
| STATE   : code                  ROUTINE : tim_stop_timer           |
+--------------------------------------------------------------------+

  PURPOSE : stop timer in the new frame variant.

*/

GLOBAL void tim_stop_timer (USHORT index)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tim_stop_timer()");

  if (index >= NUM_OF_RR_TIMERS)
  {
#if defined(TIMER_TRACE)
    SYST_TRACE_P ((SYST, "tim_stop_timer(%u)", index));
#endif
  }
  else
  {
    trace_timer (index, 0);
    rr_data->t_running[index] = 0;
        TIMER_STOP (rr_handle, index);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : RR_TIM                   |
| STATE   : code                  ROUTINE : tim_start_timer          |
+--------------------------------------------------------------------+

  PURPOSE : start timer in the new frame variant.

*/

GLOBAL void tim_start_timer (USHORT index, T_TIME value)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tim_start_timer()");

  if (index >= NUM_OF_RR_TIMERS)
  {
#if defined(TIMER_TRACE)
    SYST_TRACE_P ((SYST, "tim_start_timer(%u)", index));
#endif
  }
  else
  {
    trace_timer (index, value);
    rr_data->t_running[index] = 1;
    TIMER_START (rr_handle, index, value);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : RR_TIM                   |
| STATE   : code                  ROUTINE : tim_check_timer          |
+--------------------------------------------------------------------+

  PURPOSE : Checks if a specified timer is running

*/
#if defined(TIMER_TRACE)
GLOBAL BOOL tim_check_timer(USHORT index)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tim_check_timer()");

  if (index >= NUM_OF_RR_TIMERS)
  {
    SYST_TRACE_P ((SYST, "tim_check_timer(%u)", index));
    return FALSE;
  }
  else
  {
    trace_timer (index, -2);
    return (rr_data->t_running[index]);
  }
}
#endif  /* TIMER_TRACE */
#endif /* RR_TIM_C */
