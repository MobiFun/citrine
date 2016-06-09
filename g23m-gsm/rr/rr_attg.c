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
|  Purpose :  This module holds the functions for handling primitives
|             sent from entity GRR to entity RR and vice versa.
+-----------------------------------------------------------------------------
*/

#ifndef RR_GPRS_C
#define RR_GPRS_C

#ifdef GPRS

#define ENTITY_RR

/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stddef.h>
#include "typedefs.h"
#include "pcm.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "pei.h"
#include "tok.h"
#include "rr_gprs.h"
#include "rr.h"
#include "rr_em.h"

static void att_get_scell_info (T_serving_cell_info* sc, UBYTE idx);
static void att_gprs_store_nc_mode(T_D_SYS_INFO_13* sys_info_13);
#ifdef REL99
static void att_gprs_store_nw_release(T_D_SYS_INFO_13* sys_info_13);
#endif

static T_NC_DATA* is_nc_in_list(USHORT arfcn);
static T_NC_DATA* find_empty_slot_in_list(void);
static BOOL rr_ext_meas_can_start ( void );
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : RR_GPRS                       |
| STATE   : code             ROUTINE : att_for_sysinfo_type13        |
+--------------------------------------------------------------------+

  PURPOSE : Checks GPRS availability of cell
*/
void att_for_sysinfo_type13 (T_MPH_UNITDATA_IND* data_ind,
                             T_D_SYS_INFO_13*                  sys_info_13)
{
  GET_INSTANCE_DATA;
  /*
   * get the storage area by checking the channel number
   */
  UBYTE index = att_get_index (data_ind->arfcn);
  TRACE_FUNCTION("att_for_sysinfo_type13");
  switch(GET_STATE(STATE_ATT))
  {
    case ATT_CS2:
      /*
       * during cell selection
       * only CR_INDEX is taken in account
       */
      if (index EQ CR_INDEX)
      {
        switch(GET_STATE(STATE_GPRS))
        {
          case GPRS_ACTIVATED:
          case GPRS_PIM_BCCH:
          case GPRS_DEDI_SDCCH:
            if(sys_info_13->si13_rest_oct.v_si13_info AND
               sys_info_13->si13_rest_oct.si13_info.v_pbcch_des)
            {
              SET_STATE(STATE_GPRS, GPRS_PIM_PBCCH);
            }
            else
            {
              SET_STATE(STATE_GPRS, GPRS_PIM_BCCH);
              /*
               * check the NC mode, this is the default
               * value of the NW which can be overriden
               * by a Packet Measurement Order.
               * In CS2 we can always use the value provided
               * by the NW
               */
              att_gprs_store_nc_mode(sys_info_13);
            }
            break;
          case GPRS_PIM_PBCCH:
            TRACE_EVENT("GPRS config changed during CS2!");
            if(sys_info_13->si13_rest_oct.v_si13_info AND
               !sys_info_13->si13_rest_oct.si13_info.v_pbcch_des)
            {
              SET_STATE(STATE_GPRS, GPRS_PIM_BCCH);
            }
            break;
          default:
            break;
        }

        gprs_rrgrr_store_sdu(rr_data->gprs_data.si13_sdu, &data_ind->sdu);
        att_set_sys_info_read (SYS_INFO_13_READ, CR_INDEX);
#ifdef REL99
        att_gprs_store_nw_release(sys_info_13);
#endif
        att_check_bcch_carrier_si ();
      }
      break;
    case ATT_CS3:
      /*
       * during cell reselection
       * only CR_INDEX is taken in account
       */
      if (index EQ CR_INDEX)
      {
        switch(GET_STATE(STATE_GPRS))
        {
          case GPRS_ACTIVATED:
          case GPRS_PIM_BCCH:
            if(sys_info_13->si13_rest_oct.v_si13_info AND
               sys_info_13->si13_rest_oct.si13_info.v_pbcch_des)
            {
              SET_STATE(STATE_GPRS, GPRS_PIM_PBCCH);
            }
            else
            {
              SET_STATE(STATE_GPRS, GPRS_PIM_BCCH);
              /*
               * check the NC mode, this is the default
               * value of the NW which can be overriden
               * by a Packet Measurement Order.
               * In CS3 we can always use the value provided
               * by the NW
               */
              att_gprs_store_nc_mode(sys_info_13);
            }
            break;
          case GPRS_PIM_PBCCH:
            if(sys_info_13->si13_rest_oct.v_si13_info AND
               !sys_info_13->si13_rest_oct.si13_info.v_pbcch_des)
            {
              TRACE_EVENT("GPRS config changed during CS3!");
              SET_STATE(STATE_GPRS, GPRS_PIM_BCCH);
            }
            break;
          default:
            break;
        }

        gprs_rrgrr_store_sdu(rr_data->gprs_data.si13_sdu, &data_ind->sdu);
        att_set_sys_info_read (SYS_INFO_13_READ, CR_INDEX);
#ifdef REL99
        att_gprs_store_nw_release(sys_info_13);
#endif

        att_check_neighbourcell_si_reestab();

      }
      break;
    case ATT_CON_EST:
    case ATT_IDLE:
      switch (index)
      {
        case SC_INDEX:
          /* now we want to send the SI13 to GRR */
          if(/*rr_data->ms_data.rr_service EQ LIMITED_SERVICE OR*/
             !att_gprs_is_avail())
            return;

          switch(GET_STATE(STATE_GPRS))
          {
            case GPRS_ACTIVATED:
              if((rr_data->sc_data.selection_type EQ CELL_RESELECTION_ON_GPRS_ACT) AND
                 (rr_data->sc_data.cd.sys_info_read EQ ALL_SYS_INFO_READ))
              {
                TRACE_EVENT(" CELL_RESELECTION_ON_GPRS_ACT  should end");
                TIMERSTOP(T_RESELECT);
              }
              
              if(sys_info_13->si13_rest_oct.v_si13_info AND
                sys_info_13->si13_rest_oct.si13_info.v_pbcch_des)
              {
                if(rr_data->sc_data.cd.sys_info_read EQ ALL_SYS_INFO_READ)
                {
                  att_gprs_stop_pl();
                }
                SET_STATE(STATE_GPRS, GPRS_PIM_PBCCH);
              }
              else
              {
                SET_STATE(STATE_GPRS, GPRS_PIM_BCCH);
                if(rr_data->sc_data.cd.sys_info_read EQ ALL_SYS_INFO_READ)
                {
                  att_build_idle_req(SC_INDEX, MODE_SYS_INFO_CHANGE);
                }
                /*
                * check the NC mode, this is the default
                * value of the NW which can be overriden
                * by a Packet Measurement Order.
                *
                */
                att_gprs_store_nc_mode(sys_info_13);
              }
              break;
            case GPRS_PIM_BCCH:
            case GPRS_PTM_BCCH:
              /*
               * check the NC mode, this is the default
               * value of the NW which can be overriden
               * by a Packet Measurement Order.
               *
               */
              att_gprs_store_nc_mode(sys_info_13);
              /* XXX */
              /*
               * Right now it not clear what how the activation of PBCCH
               * should be handled in RR/GRR
               */
              if (GET_STATE(STATE_GPRS) EQ GPRS_PTM_BCCH)
                break;

              if(sys_info_13->si13_rest_oct.v_si13_info AND
                 sys_info_13->si13_rest_oct.si13_info.v_pbcch_des)
              {
                att_gprs_stop_pl();
                SET_STATE(STATE_GPRS, GPRS_PIM_PBCCH);
              }
              break;
            case GPRS_PIM_PBCCH:
              if(sys_info_13->si13_rest_oct.v_si13_info AND
                 !sys_info_13->si13_rest_oct.si13_info.v_pbcch_des)
              {
                /*XXX not possible actually */
                SET_STATE(STATE_GPRS, GPRS_PIM_BCCH);
              }
              break;
            default:
              break;
          }

          /*
           * GRR requests reading of SI13 every 30 seconds
           * pass the SI13 to GRR, for further evaluation
           */
          TRACE_EVENT("idle sc");
          TRACE_EVENT_P1("sys_info_read %d", rr_data->sc_data.cd.sys_info_read);
          gprs_rrgrr_store_sdu(rr_data->gprs_data.si13_sdu, &data_ind->sdu);
          att_set_sys_info_read (SYS_INFO_13_READ, SC_INDEX);

#ifdef REL99
          att_gprs_store_nw_release(sys_info_13);
#endif
          att_signal_gprs_support();
          break;
        case NOT_PRESENT_8BIT:
          /*
           * unexpected channel number
           */
          break;
        default:
          /*
           * neighbour cell, ignore.
           * si13 info is not needed, at first
           * maybe get PBCCH info later
           */
          break;
      }
      break;
    default:
      break;
  }
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS ()         MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : att_set_gprs_indication             |
+-----------------------------------------------------------------------------+

  PURPOSE : sets a flag indicating whether GPRS is wanted by upper layer
            or not.

*/
void att_set_gprs_indication (UBYTE gprs_indic)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_set_gprs_indication()");

  /*
   * store parameter from MM
   */
  rr_data->gprs_data.gprs_indic = gprs_indic;

  if(rr_data->gprs_data.gprs_indic)
  {
    SET_STATE(STATE_GPRS, GPRS_ACTIVATED);
    TRACE_EVENT("MM wants GPRS");
  }
  else
  {
    SET_STATE(STATE_GPRS, GPRS_NULL);
    rr_data->gprs_data.ready_state = FALSE;    /* Standby */
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : RR_GPRS                       |
| STATE   : code             ROUTINE : att_check_gprs_supp           |
+--------------------------------------------------------------------+

  PURPOSE : Checks GPRS availability of cell and sets the internal
            flag
*/
void att_check_gprs_supp (UBYTE v_gprs_ind, T_gprs_indic * data)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_check_gprs_supp ()");

  /*
   * if rest octets for GPRS are available
   * and GPRS support is requested by MM
   */
  switch(GET_STATE(STATE_GPRS))
  {
    case GPRS_NULL:
      if(GET_STATE(STATE_ATT) EQ ATT_CS2 OR
         GET_STATE(STATE_ATT) EQ ATT_CS3)
        att_set_sys_info_read(SYS_INFO_13_READ, CR_INDEX);
      else
        att_set_sys_info_read(SYS_INFO_13_READ, SC_INDEX);
      break;
    case GPRS_ACTIVATED:
      if(GET_STATE(STATE_ATT) EQ ATT_CS2 OR
         GET_STATE(STATE_ATT) EQ ATT_CS3)
      {
        if(v_gprs_ind)
        {
          SET_STATE(STATE_GPRS, GPRS_PIM_BCCH);
          /*
           * If System Info 13 is send on extended BCCH,
           * request reading, else it is reading on normal BCCH
           */
          if(data->si13_pos EQ SI13_ON_BCCH_EXT)
          {
            PALLOC(mon_ctrl, MPH_MON_CTRL_REQ);

            mon_ctrl->action     = START_MON_EBCCH;
            mon_ctrl->si_to_read = UPDATE_SI13;

            PSENDX(PL, mon_ctrl);
          }
          /* else we will read it on NBCCH */
        }
        else
        {
          /* no state change */
          att_set_sys_info_read(SYS_INFO_13_READ, CR_INDEX);
        }
      }
      /*
       * when GPRS is activated in idle this will be handled
       * when the SI13 is received
       */
      break;
    case GPRS_PIM_BCCH:
    case GPRS_PIM_PBCCH:
      if(!v_gprs_ind)
      {
        SET_STATE(STATE_GPRS, GPRS_ACTIVATED);
        att_set_sys_info_read(SYS_INFO_13_READ, SC_INDEX);
        if(GET_STATE(STATE_ATT) EQ ATT_IDLE)
        {
          /*XY:n inform GRR, and       wait for CR_RSP */
          att_rrgrr_cr_ind(CR_ABNORMAL);
          rr_data->gprs_data.start_proc = START_PROC_CFG_CHG;
        }
      }
      else
      {
        /*
         * If System Info 13 is send on extended BCCH,
         * request reading, else it is reading on normal BCCH
         */
        if(data->si13_pos EQ SI13_ON_BCCH_EXT)
        {
          PALLOC(mon_ctrl, MPH_MON_CTRL_REQ); 
          mon_ctrl->action     = START_MON_EBCCH;
          mon_ctrl->si_to_read = UPDATE_SI13;
          PSENDX(PL, mon_ctrl);
        }
        /* else we will read it on NBCCH */
      }
      break;
    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : RR_GPRS                       |
| STATE   : code             ROUTINE : att_signal_gprs_support       |
+--------------------------------------------------------------------+

  PURPOSE : Signal GPRS support to GRR
*/
void att_signal_gprs_support (void)
{
  GET_INSTANCE_DATA;
  PALLOC_SDU(rrgrr_gprs_si13_ind, RRGRR_GPRS_SI13_IND, MAX_L2_FRAME_SIZE * BITS_PER_BYTE);
  TRACE_FUNCTION ("att_signal_gprs_support()");
  /*
   * gprs_support    : Y Y X N N
   * rr_service      : F L N F L
   * sdu             : Y Y - - -
   * servng_cell_info:
   */

  if(GET_STATE(STATE_ATT) EQ ATT_IDLE AND
     rr_data->sc_data.cd.sys_info_read NEQ ALL_SYS_INFO_READ)
  {
    PFREE(rrgrr_gprs_si13_ind);
    return;
  }

  if((GET_STATE(STATE_GPRS) NEQ GPRS_ACTIVATED AND
      GET_STATE(STATE_GPRS) NEQ GPRS_NULL) AND
     !(rr_data->ms_data.rr_service EQ NO_SERVICE)
    )
  {
    rrgrr_gprs_si13_ind->cause = GPRS_SUPPORTED;

    /*
     * copy message content
     */
    gprs_rrgrr_fill_from_stored_sdu(&rrgrr_gprs_si13_ind->sdu,
                                    rr_data->gprs_data.si13_sdu);

    TRACE_EVENT_P4 ("cs=%u C[%d] bsic=%u #%u",
                    rrgrr_gprs_si13_ind->cause,
                    rrgrr_gprs_si13_ind->serving_cell_info.bcch_arfcn,
                    rrgrr_gprs_si13_ind->serving_cell_info.bcch_bsic,
                    __LINE__);

    /*
     * set system information state
     */
    rrgrr_gprs_si13_ind->si_states.si1_state  = SI1_RECEIVED;
    rrgrr_gprs_si13_ind->si_states.si3_state  = SI3_RECEIVED;
    rrgrr_gprs_si13_ind->si_states.si13_state = SI13_RECEIVED;
  }
  else
  {
    rrgrr_gprs_si13_ind->cause = GPRS_NOT_SUPPORTED;
  }

  /*When BA (BCCH) list is modified by PMO/PCCO, this is just for RR, RR should not inform
  GRR as it would have already formed the list. Sending this new list will change all the
  index, as GRR would assume it to be new BA(BCCH). When New BA(BCCH) is received then this flag
  would be reset and GRR should update the BA list*/

  if(rr_data->gprs_data.ba_bcch_modified AND (GET_STATE(STATE_ATT) NEQ ATT_CS3))
  {
    rrgrr_gprs_si13_ind->serving_cell_info.arfcn[0]= RRGRR_INVALID_ARFCN;
  }
  else
  {
  memcpy(rrgrr_gprs_si13_ind->serving_cell_info.arfcn, rr_data->act_ncell_list,
         sizeof(rrgrr_gprs_si13_ind->serving_cell_info.arfcn));
  }
  

  att_get_scell_info(&rrgrr_gprs_si13_ind->serving_cell_info, SC_INDEX);

  /* we have found a cell, reset cr flag*/
  rr_data->gprs_data.cr_pbcch_active = FALSE;

#if defined (REL99) AND defined (TI_PS_FF_EMR)
  rrgrr_gprs_si13_ind->serving_cell_info.ba_ind = rr_data->sc_data.ba_index;
#endif

  PSENDX(GRR, rrgrr_gprs_si13_ind);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : att_add_ptmsi                      |
+-----------------------------------------------------------------------------+

  PURPOSE : Add the ptmsi if GPRS is supported.

*/
void att_add_ptmsi (T_MPH_IDENTITY_REQ * mph_identity_req)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_add_ptmsi()");

  /*
   * check availability of GPRS
   */
  switch (GET_STATE(STATE_GPRS))
  {
    case GPRS_NULL:
      mph_identity_req->mid.v_ptmsi = FALSE;
      mph_identity_req->mid.ptmsi   = 0L;


      mph_identity_req->mid.v_ptmsi2 = FALSE;
      mph_identity_req->mid.ptmsi2   = 0L;
      break;
    default:
      if (rr_data->gprs_data.ptmsi EQ 0xFFFFFFFF OR
          rr_data->gprs_data.ptmsi EQ 0L)
      {
        mph_identity_req->mid.v_ptmsi = FALSE;
        mph_identity_req->mid.ptmsi   = 0L;
      }
      else
      {
        mph_identity_req->mid.v_ptmsi = TRUE;
        mph_identity_req->mid.ptmsi   = rr_data->gprs_data.ptmsi;
      }
      /*the candidate PTMSI has to be checked also*/
      if (rr_data->gprs_data.ptmsi2 EQ 0xFFFFFFFF OR
          rr_data->gprs_data.ptmsi2 EQ 0L)
      {
        mph_identity_req->mid.v_ptmsi2 = FALSE;
        mph_identity_req->mid.ptmsi2   = 0L;
      }
      else
      {
        mph_identity_req->mid.v_ptmsi2 = TRUE;
        mph_identity_req->mid.ptmsi2   = rr_data->gprs_data.ptmsi2;
      }
      break;
  }
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : att_rrgrr_cr_ind                    |
+-----------------------------------------------------------------------------+

  PURPOSE :
  IN      :
  OUT     :

*/

void att_rrgrr_cr_ind (UBYTE type)
{
  GET_INSTANCE_DATA;
  /*
   * Following cases are possible:
   *  BCCH always CR_ABNORMAL
   *   after con est fail, after dedi, normal cr in idle and ptm,
   *   plmn search,activate req, cell selection
   *  PBCCH:
   *   after con est fail(CR_REQ_CANDIDATE),
   *   activate_req (CR_NORMAL)
   *   after dedi (CR_ABNORMAL)
   *   after imm_ass_rej (CR_SUSPENDED_IDLE)
   */
  if(GET_STATE(STATE_GPRS) NEQ GPRS_NULL)
  {
    PALLOC(cr_ind, RRGRR_CR_IND);
    cr_ind->cr_type = type;
    TRACE_EVENT("cr_ind");
    PSENDX(GRR, cr_ind);
  }
  else
  {
    TRACE_EVENT("cr already active");
  }
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : att_rrgrr_cr_req                    |
+-----------------------------------------------------------------------------+

  PURPOSE :

*/
void att_rrgrr_cr_req (T_RRGRR_CR_REQ *cr_req)
{
  GET_INSTANCE_DATA;
  int gprs_state;

  TRACE_FUNCTION ("gprs_rrgrr_cell_reselection_req ()");

  gprs_state = GET_STATE(STATE_GPRS);

  TRACE_EVENT_P3 ("RRGRR_CR_REQ %u [%u] st=%u",
    cr_req->cr_type, cr_req->arfcn, gprs_state);
  TRACE_EVENT_P6 ( "r MCC/MNC=%x%x%x/%x%x%x",
    rr_data->ms_data.plmn.mcc[0],
    rr_data->ms_data.plmn.mcc[1],
    rr_data->ms_data.plmn.mcc[2],
    rr_data->ms_data.plmn.mnc[0],
    rr_data->ms_data.plmn.mnc[1],
    rr_data->ms_data.plmn.mnc[2]);

  switch(gprs_state)
  {
    case GPRS_PAM_PBCCH:
    case GPRS_ACTIVATED:
    case GPRS_PIM_PBCCH:
    case GPRS_PTM_PBCCH:
    case GPRS_SUSPENDED_PBCCH:
      switch(cr_req->cr_type)
      {
        case CR_NEW:
          {
            UBYTE i;
            UBYTE found=0;
            for(i=0; i<6;i++)
            {
              if(rr_data->nc_data[i].arfcn EQ cr_req->arfcn AND
                 rr_data->nc_data[i].bsic EQ cr_req->bsic)
              {
                found=1;
                break;
              }
            }

            if(!found) { TRACE_ERROR("cr_req(NEW): data does not match");}

            memset (&rr_data->nc_data[CR_INDEX],
                    0,
                    sizeof (T_NC_DATA));
            rr_data->gprs_data.cr_pbcch_active = TRUE;
            rr_data->nc_data[CR_INDEX].arfcn = cr_req->arfcn;
            rr_data->nc_data[CR_INDEX].bsic  = cr_req->bsic;

            if (rr_data->ms_data.req_mm_service EQ FUNC_NET_SRCH_BY_MMI)
            {
              /*inform MM and stop everything*/
              /*
               * send Please Retry to the MMI
               */
              rr_data->sc_data.found_entries = 0;
              att_code_rr_abort_ind (RRCS_ABORT_CEL_SEL_FAIL);
            }

            if( ! rr_data->gprs_data.cr_pcco_active )
              att_start_cell_reselection_pbcch(MODE_CELL_RESELECTION);
            else
            {
              att_init_cell_selection(CELL_RESELECTION, RR_ORIGINATED);
              rr_data->dyn_config.fcr        = 0;
              rr_data->dyn_config.scr        = 0;
              rr_data->bcch_error = 0;
              rr_data->pag_rec    = FALSE;
              srv_clear_stored_prim (MPH_PAGING_IND);
              gprs_init_data_cr();
              dat_att_null();

              SET_STATE (STATE_ATT, ATT_CS3);
              att_build_idle_req (CR_INDEX, MODE_CELL_RESELECTION);
            }
            break;
          }
        case CR_NEW_NOT_SYNCED:
          {
            /*
             * a Packet Cell Change Order for a cell which
             * is not in the BA list has been requested by the NW
             */
            PALLOC (mph_bsic_req, MPH_BSIC_REQ);
            {
              PALLOC (mph_sync_req, MPH_SYNC_REQ);
              mph_sync_req->cs = CS_START_PCCO;
              PSENDX (PL, mph_sync_req);
            }
            TRACE_EVENT("cco w/ not synced");

            /*
             * remember we got a CR_REQ, will be used
             * on reception of the BSIC_CNF and the UNITDATA_IND
             */
            rr_data->gprs_data.cr_pbcch_active = TRUE;
            SET_STATE (STATE_CELL_SEL, CS_CCO);
            /* avoid reacting to PAGINGs and ESTABLISH_REQ */
            dat_att_null();
            SET_STATE (STATE_ATT, ATT_CS3);

            cs_set_all();
            rr_data->gprs_data.bsic  = cr_req->bsic;
            rr_data->gprs_data.arfcn = mph_bsic_req->arfcn = cr_req->arfcn;
            PSENDX (PL, mph_bsic_req);
          }
          break;
        case CR_CS:
          if (rr_data->gprs_data.cr_pcco_active EQ FALSE)
            rr_data->gprs_data.cr_pbcch_active = TRUE;
          else
            rr_data->gprs_data.cr_pcco_active = FALSE;


          if (rr_data->ms_data.req_mm_service EQ FUNC_NET_SRCH_BY_MMI)
          {
            /*inform MM and stop everything*/
            /*
             * send Please Retry to the MMI
             */
            rr_data->sc_data.found_entries = 0;
            att_code_rr_abort_ind (RRCS_ABORT_CEL_SEL_FAIL);
          }
          /*XY:n don't inform GRR */
          att_start_cell_selection(RR_ORIGINATED, CS_NOT_PARALLEL,NORMAL_SEARCH_MODE);
          break;
        case CR_CONT:
          att_continue_cell_reselect ();
          break;
        case CR_COMPLETE:
          /*
           * we have a PBCCH and GRR has read it
           * we inform MM now
           */
             if( rr_data->gprs_data.cr_pcco_active EQ FALSE  )
            {
              PALLOC(mph_mon_ctrl_req, MPH_MON_CTRL_REQ );
              mph_mon_ctrl_req->action = ENTER_PIM_PBCCH;
              PSENDX (PL, mph_mon_ctrl_req);
            }
            rr_data->gprs_data.cr_pcco_active = FALSE;

			 /* initialize the  ba_bcch_modified value when Cell reselection is done */
            rr_data->gprs_data.ba_bcch_modified= FALSE;

            dat_att_cell_selected ();
            SET_STATE (STATE_ATT, ATT_IDLE);
            dat_send_bcchinfo_mm (rr_data->cr_data.cr_white_list.si2);



            if(rr_data->ms_data.rr_service EQ LIMITED_SERVICE AND
               rr_data->ms_data.req_mm_service EQ FUNC_PLMN_SRCH)
            {
              att_copy_old_lai_rac(SC_INDEX);
#if 0
              memcpy (&rr_data->old_lai, &rr_data->nc_data[SC_INDEX].lai,
                      sizeof(T_loc_area_ident));
              rr_data->old_cell_id =  rr_data->nc_data[SC_INDEX].cell_id;
#endif
              att_code_rr_abort_ind (RRCS_ABORT_CEL_SEL_FAIL);
            }
            else
            {
              if (rr_data->sc_data.mm_started)
                att_code_rr_act_cnf ();
              else
                att_code_rr_act_ind ();
            }

            /*
             * This instruction disdurbs test case 20.7
             * unclear why it is coded here
             * 
            rr_data->first_meas_received = TRUE;
            TRACE_EVENT("first_meas_received set to TRUE(rr_attg.c)");
            */
            /*
             * Start registration timer if needed
             */
            att_start_registration_timer ();
            att_mph_identity_req ();
#ifdef REL99
            if(rr_data->ms_data.rr_service EQ FULL_SERVICE) att_config_cbch ();
#else
            if(rr_data->ms_data.rr_service EQ FULL_SERVICE) att_build_cbch ();
#endif
            srv_use_stored_prim ();
            /*
             * if the cell reselection has been started after a dedicated connection
             * attempt with failed SABM / UA content a second attempt is started
             * immediately.
             */
            if (rr_data->repeat_est)
            {
              rr_data->repeat_est = FALSE;
              dat_start_immediate_assign (rr_data->ms_data.establish_cause);
            }
          break;
        default:
          break;

      }
      break;
    case GPRS_PTM_BCCH:
      if( (cr_req->cr_type EQ CR_NEW) ||
          (cr_req->cr_type EQ CR_NEW_NOT_SYNCED))
      {
         UBYTE i;
         UBYTE found=0;
         for(i=0; i<6;i++)
         {
           if(rr_data->nc_data[i].arfcn EQ cr_req->arfcn AND
               rr_data->nc_data[i].bsic EQ cr_req->bsic)
           {
             found=1;
             break;
           }
         }
         memset (&rr_data->nc_data[CR_INDEX], 0, sizeof (T_NC_DATA));
         rr_data->nc_data[CR_INDEX].arfcn = cr_req->arfcn;
         rr_data->nc_data[CR_INDEX].bsic  = cr_req->bsic;
         if (rr_data->ms_data.req_mm_service EQ FUNC_NET_SRCH_BY_MMI)
         {
            /*inform MM and stop everything*/
            /*
             * send Please Retry to the MMI
             */
           rr_data->sc_data.found_entries = 0;
           att_code_rr_abort_ind (RRCS_ABORT_CEL_SEL_FAIL);
         }

          rr_data->gprs_data.cr_pcco_active = TRUE;

         if(!found)
         {
           /*
             * a Packet Cell Change Order for a cell which
             * is not in the BA list has been requested by the NW
             */
            PALLOC (mph_bsic_req, MPH_BSIC_REQ);
            {
              PALLOC (mph_sync_req, MPH_SYNC_REQ);
              mph_sync_req->cs = CS_START_PCCO;
              PSENDX (PL, mph_sync_req);
            }
            TRACE_EVENT("cco w/ not synced");


            SET_STATE (STATE_CELL_SEL, CS_CCO);
            /* avoid reacting to PAGINGs and ESTABLISH_REQ */
            dat_att_null();
            SET_STATE (STATE_ATT, ATT_CS3);

            cs_set_all();
            rr_data->gprs_data.bsic  = cr_req->bsic;
            rr_data->gprs_data.arfcn = mph_bsic_req->arfcn = cr_req->arfcn;
            PSENDX (PL, mph_bsic_req);
         }
         else
         {
           att_init_cell_selection(CELL_RESELECTION, RR_ORIGINATED);
           rr_data->dyn_config.fcr        = 0;
           rr_data->dyn_config.scr        = 0;
           rr_data->bcch_error = 0;
           rr_data->pag_rec    = FALSE;
           srv_clear_stored_prim (MPH_PAGING_IND);
           gprs_init_data_cr();
           dat_att_null();

           SET_STATE (STATE_ATT, ATT_CS3);
           att_build_idle_req (CR_INDEX, MODE_CELL_RESELECTION);
         }
      }
      break;
    case GPRS_PIM_BCCH:
      /*
       * we have a BCCH
       * we have informed GRR of this (with additional info that we
       * have Full or Limited service)
       * we inform MM now
       */
      switch(cr_req->cr_type)
      {
        case CR_COMPLETE:
        case CR_NEW:
        case CR_NEW_NOT_SYNCED:
        case CR_CS:
        case CR_CONT:
          dat_att_cell_selected ();
          SET_STATE (STATE_ATT, ATT_IDLE);
          dat_send_bcchinfo_mm (rr_data->cr_data.cr_white_list.si2);

          if(rr_data->ms_data.rr_service EQ LIMITED_SERVICE AND
             rr_data->ms_data.req_mm_service EQ FUNC_PLMN_SRCH)
          {

            att_copy_old_lai_rac(SC_INDEX);
#if 0
            memcpy (&rr_data->old_lai, &rr_data->nc_data[SC_INDEX].lai,
                    sizeof(T_loc_area_ident));
            rr_data->old_cell_id =  rr_data->nc_data[SC_INDEX].cell_id;
#endif
            att_code_rr_abort_ind (RRCS_ABORT_CEL_SEL_FAIL);
          }
          else
          {
            if (rr_data->sc_data.mm_started)
              att_code_rr_act_cnf ();
            else
              att_code_rr_act_ind ();
          }

          /*
           * Start registration timer if needed
           */
          att_start_registration_timer ();
          att_mph_identity_req ();
          srv_use_stored_prim ();
          /*
           * if the cell reselection has been started after a dedicated connection
           * attempt with failed SABM / UA content a second attempt is started
           * immediately.
           */
          if (rr_data->repeat_est)
          {
            rr_data->repeat_est = FALSE;
            dat_start_immediate_assign (rr_data->ms_data.establish_cause);
          }
          break;
        default:
          break;
      }
    break;
    case GPRS_PAM_BCCH:
      TRACE_EVENT("CR during PA");
      break;
    default:
      break;
  }
  PFREE (cr_req);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : att_rrgrr_stop_mon_ccch_req         |
+-----------------------------------------------------------------------------+

  PURPOSE : Process the primitive RRGRR_STOP_MON_CCCH_REQ received from GRR.
            This primitive indicates to the RR that the monitoring of CCCH
            should be stopped. This is only a signal, no parameters needed.
            This signal is forwarded to ALR.

*/
/*
 * This is reqiured for Testmode A
 */
/*lint -esym(526,grlc_test_mode_active) : not defined | defined in GRR */
EXTERN UBYTE grlc_test_mode_active(void);

void att_rrgrr_stop_mon_ccch_req (T_RRGRR_STOP_MON_CCCH_REQ* stop_mon)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_rrgrr_stop_mon_ccch_req ()");

  switch(GET_STATE(STATE_GPRS))
  {
    /*
     * This case is necessary to stop ccch monitoring in STATE_GPRS=GPRS_PTM
     * during Testmode A/B
     */
    case GPRS_PTM_BCCH:
      if(grlc_test_mode_active())
      {
        PALLOC(mph_mon_ctrl_req, MPH_MON_CTRL_REQ );
        mph_mon_ctrl_req->action = STOP_MON_CCCH;
        PSENDX (PL, mph_mon_ctrl_req);
        TRACE_EVENT("STOP_MON_CCCH during TESTMODE !!");
      }
      break;
    case GPRS_PIM_BCCH:
    case GPRS_PIM_PBCCH:
    case GPRS_PAM_PBCCH:
    case GPRS_PTM_PBCCH:
    case GPRS_PAM_BCCH:
      TRACE_EVENT("stop ccch");
      break;
    default:
      break;
  }

  PFREE (stop_mon);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : att_rrgrr_start_mon_ccch_req        |
+-----------------------------------------------------------------------------+

  PURPOSE : Process the primitive RRGRR_START_MON_CCCH_REQ received from GRR.
            This primitive indicates to the RR that the monitoring of CCCH
            should be started. This is only a signal, no parameters needed.
            This signal is forwarded to ALR.

*/
void att_rrgrr_start_mon_ccch_req (T_RRGRR_START_MON_CCCH_REQ*
                                    mon_req)
{
  GET_INSTANCE_DATA;
  TRACE_EVENT ("att_rrgrr_start_mon_ccch_req ()");
  /*
   * store split paging cycle
   */
  if (mon_req->split_pg EQ RRGRR_NO_DRX)
    rr_data->gprs_data.split_pg=256;
  else
    rr_data->gprs_data.split_pg=mon_req->split_pg;

  switch(GET_STATE(STATE_GPRS))
  {
    case GPRS_PIM_BCCH:
      TRACE_EVENT("GPRS_PIM_BCCH");
      if(mon_req->pag_mode EQ PAG_MODE_DEFAULT AND
         rr_data->gprs_data.page_mode EQ PAG_MODE_REORG)
      {
        /*
         * go back to normal page mode
         */
        rr_data->gprs_data.page_mode = PAG_MODE_DEFAULT;
        att_build_idle_req (SC_INDEX, MODE_SYS_INFO_CHANGE);
      }
      if(mon_req->pag_mode EQ PAG_MODE_PTM_NP)
      {
        /*
         * we may have received dl assignment in reog idle mode
         * we shopuld directly switch to transfer mode with:
         * NORMAL_PAGING
         */
        SET_STATE(STATE_GPRS, GPRS_PTM_BCCH);
        SET_STATE(STATE_ATT,  ATT_IDLE);
        SET_STATE(STATE_DAT,  DAT_IDLE);

        att_build_idle_req(SC_INDEX, MODE_PACKET_TRANSFER);
        {
          PALLOC(mon_ctrl, MPH_MON_CTRL_REQ);
          mon_ctrl->action     = START_MON_CCCH;
          mon_ctrl->si_to_read = 0;
          PSENDX(PL, mon_ctrl);
        }
      }
      break;
    case GPRS_PTM_BCCH:
      /*
       * set page_mode to REORG if NON_DRX_TIMER is used
       * set page_mode to DEFAULT if NON_DRX_TIMER is not used or 0
       */
      TRACE_EVENT("GPRS_PTM");
      if(mon_req->pag_mode EQ PAG_MODE_PTM_NP)
      {
        PALLOC(mon_ctrl, MPH_MON_CTRL_REQ);
        mon_ctrl->action     = START_MON_CCCH;
        mon_ctrl->si_to_read = 0;
        PSENDX(PL, mon_ctrl);
      }
      else if(mon_req->pag_mode EQ PAG_MODE_PIM_NP)
      {
        /*
         * Go into idle mode but start ONLY CCCH reading with the following order:
         * start with REORG and then switch to NORMAL paging
         */
        TRACE_EVENT("PTM->IDLE: CCCH reorg+normal");
      }
      else
      {
        rr_data->gprs_data.page_mode = mon_req->pag_mode;
        SET_STATE (STATE_GPRS, GPRS_PIM_BCCH);
        att_return_to_idle();

        /* Transition from PTM to PIM state. PBCCH is not present in the 
         * cell. Check if the previous PTM state was used for routing Area update.
         * start black list search to look for inactive carriers
         * Cell-Selection Improvement LLD - 4.1.4.3  
         */
        if((rr_data->ms_data.establish_cause EQ ESTCS_GPRS_1P) AND 
           (rr_data->cs_data.black_list_search_pending EQ TRUE))
        { 

          rr_data->cs_data.black_list_search_pending = FALSE;
   
          if(cs_check_region(rr_data->cs_data.region))
          {
            /* Do not start Black list search,when Black list database is empty */
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
            if (rr_data->cs_data.region EQ BOTH_REGIONS)
            {
              if(srv_is_list_set(&rr_data->cs_data.black_list.list[EUROPEAN_REGION]) OR 
                srv_is_list_set(&rr_data->cs_data.black_list.list[AMERICAN_REGION]))
              {
              att_start_cell_selection(RR_ORIGINATED,CS_PARALLEL,BLACK_LIST_SEARCH_MODE);
            }
            else
            {
              TRACE_EVENT("Black List empty");
              }
            }
#else
            if(srv_is_list_set(
                 &rr_data->cs_data.black_list.list[rr_data->cs_data.region]))
            {
              att_start_cell_selection(RR_ORIGINATED,CS_PARALLEL,BLACK_LIST_SEARCH_MODE);
            }
#endif
            else
            {
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
              if(srv_is_list_set(
                   &rr_data->cs_data.black_list.list[rr_data->cs_data.region]))
              {
                att_start_cell_selection(RR_ORIGINATED,CS_PARALLEL,BLACK_LIST_SEARCH_MODE);
              }
              else
#endif
              {
                TRACE_EVENT("Black List empty");
              }
            }
          }
        }
      }
      break;
    case GPRS_PAM_BCCH:
      rr_data->gprs_data.page_mode = mon_req->pag_mode;
      SET_STATE (STATE_GPRS, GPRS_PIM_BCCH);
      att_return_to_idle();
      break;
    case GPRS_PAM_PBCCH:
      //break;
    case GPRS_PTM_PBCCH:
    case GPRS_PIM_PBCCH:
      {
        PALLOC(mon_ctrl, MPH_MON_CTRL_REQ);
        mon_ctrl->action     = START_MON_CCCH;
        mon_ctrl->si_to_read = 0;

        PSENDX(PL, mon_ctrl);
        SET_STATE(STATE_DAT, DAT_IDLE); /* allow reception of pagings */
      }
      break;
    default:
      TRACE_EVENT("start_mon_ccch_req not handled");
      break;
  }
  PFREE (mon_req);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : att_rrgrr_start_mon_bcch_req        |
+-----------------------------------------------------------------------------+

  PURPOSE : Process the primitive RRGRR_START_MON_BCCH_REQ received from GRR.
            This primitive indicates to the RR to read SI. This primitive
            contains the parameter which indicates to the SI to read on BCCH.
            This signal is forwarded to ALR.

*/
void att_rrgrr_start_mon_bcch_req (T_RRGRR_START_MON_BCCH_REQ
                                   *rrgrr_start_mon_bcch_req)
{
  GET_INSTANCE_DATA;
  TRACE_EVENT ("att_rrgrr_start_mon_bcch_req ()");
  switch (GET_STATE(STATE_GPRS))
  {
    case GPRS_PIM_BCCH:
    case GPRS_PIM_PBCCH:
    case GPRS_PAM_PBCCH:
    case GPRS_PTM_PBCCH:
    case GPRS_PTM_BCCH:
      {
        UBYTE  si_to_read    = rrgrr_start_mon_bcch_req->si_to_read;
        USHORT sys_info_read = rr_data->sc_data.cd.sys_info_read;
        PALLOC ( mph_mon_ctrl_req, MPH_MON_CTRL_REQ );

        TRACE_EVENT_P1 ( "rrgrr_start_mon_bcch_req->si_to_read = %d",
                          rrgrr_start_mon_bcch_req->si_to_read );

        mph_mon_ctrl_req->action     = START_MON_NBCCH;
        switch ( si_to_read )
        {
          case UNSPECIFIED_SI:
            sys_info_read = 0;
            break;
          case UPDATE_SI1:
             sys_info_read &= ~ SYS_INFO_1_READ;
             break;
          case UPDATE_SI2_SI2BIS_OR_SI2TER:
             sys_info_read &= ~ (SYS_INFO_2_READ | SYS_INFO_2BIS_READ | SYS_INFO_2TER_READ);
#if defined (REL99) AND defined (TI_PS_FF_EMR)
             /*Once SI-2,2Bis,2Ter are read, we can configure ALR for acquiring SI-2quater*/
             if ( (rr_data->sc_data.cd.si2quater_status NEQ SI2QUATER_ABSENT ) AND
                   (rr_data->sc_data.cd.si2quater_pos EQ SI2QUATER_ON_EBCCH) )
                   rr_data->sc_data.cd.si2quater_status = SI2QUATER_CONFIGURE;
#endif
            break;
          case UPDATE_SI3_SI4_SI7_OR_SI8:
             sys_info_read &= ~ (SYS_INFO_3_READ | SYS_INFO_4_READ );
            break;
          case UPDATE_SI9:
            /* no support of SI 9 */
            break;
          case COMPLETE_SI:
            sys_info_read = 0;
            break;
          case UPDATE_SI13:
            sys_info_read &= ~ SYS_INFO_13_READ;
            if(rr_data->nc_data[SC_INDEX].si13_loc_ind EQ EXTENDED_BCCH)
            {
                mph_mon_ctrl_req->action     = START_MON_EBCCH;
            }
            break;
          default:
            break;
        }
        rr_data->sc_data.cd.sys_info_read = sys_info_read;
        TRACE_EVENT_P1("st_bcch:sys_info_read %d",
                       rr_data->sc_data.cd.sys_info_read);
        mph_mon_ctrl_req->si_to_read = si_to_read;
        PSENDX (PL, mph_mon_ctrl_req);
      }
      break;
    case GPRS_ACTIVATED:
      if(GET_STATE(STATE_ATT) EQ ATT_IDLE)
        att_signal_gprs_support();
      break;
    default:
      break;
  }
  PFREE (rrgrr_start_mon_bcch_req);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : att_convert_idle_c31_cr                          |
+-----------------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL void att_convert_idle_c31_cr(UBYTE index)
{
  GET_INSTANCE_DATA;
  TRACE_EVENT_P2("att_convert_idle_c31_cr called for index=%d conversion = %d", index,
  rr_data->nc_data[index].v_cr_par);
  switch(rr_data->nc_data[index].v_cr_par)
  {
    case CR_PAR_VALID:

      /*
       * For the first neighbour cell in the message where the struct exists,
       * the following default values shall be used for missing parameters:
       * GPRS_RXLEV_ACCESS_MIN = Serving cell RXLEV_ACCESS_MIN
       * GPRS_MS_TXPWR_MAX_CCH = Serving cell MS_TXPWR_MAX_CCH
       * GPRS_TEMPORARY_OFFSET = Serving cell TEMPORARY_OFFSET
       * GPRS_PENALTY_TIME = Serving cell PENALTY_TIME
       * GPRS_RESELECT_OFFSET = 0
       * HCS_THR = infinity
       * PRIORITY_CLASS = undefined
       * SI13_PBCCH_LOCATION = undefined
       *
       */
      if( rr_data->nc_data[index].cr_par.gprs_rxlev_access_min EQ
          GPRS_RXLEV_ACCESS_MIN_INVALID)
        rr_data->nc_data[index].cr_par.gprs_rxlev_access_min =
          rr_data->nc_data[SC_INDEX].select_para.rxlev_access_min;

      if(rr_data->nc_data[index].cr_par.gprs_ms_txpwr_max_cch EQ
         GPRS_MS_TXPWR_MAX_CCH_INVALID)
        rr_data->nc_data[index].cr_par.gprs_ms_txpwr_max_cch =
          rr_data->nc_data[SC_INDEX].select_para.ms_txpwr_max_cch;

      if(rr_data->nc_data[index].cr_par.gprs_temporary_offset EQ
         GPRS_TEMPORARY_OFFSET_INVALID)
        rr_data->nc_data[index].cr_par.gprs_temporary_offset =
         rr_data->nc_data[SC_INDEX].c2_par.temp_offset;

       if(rr_data->nc_data[index].cr_par.gprs_penalty_time EQ
          GPRS_PENALTY_TIME_INVALID)
         rr_data->nc_data[index].cr_par.gprs_penalty_time =
           rr_data->nc_data[SC_INDEX].c2_par.penalty_time * 2;

       if(rr_data->nc_data[index].cr_par.gprs_reselect_offset EQ
          GPRS_RESEL_OFF_INVALID)
         rr_data->nc_data[index].cr_par.gprs_reselect_offset =
           GPRS_RESEL_OFF_DEFAULT;

     /*
       * 31 is the maximum 5 bit value.
       * hcs_thr = 31 , means infinity
       */
       if(rr_data->nc_data[index].cr_par.hcs_thr EQ
          GPRS_HCS_THR_INVALID)
       {
         rr_data->nc_data[index].cr_par.hcs_thr = 31;
       }
       /* priority class remains undefined */
      break;
    case CR_PAR_INVALID:
       /*
        * initialise values
        * conversion of idle mode parameters to GPRS cell reselection parameters
        *  GPRS_RXLEV_ACCESS_MIN = RXLEV_ACCESS_MIN
        *  GPRS_MS_TXPWR_MAX_CCH = MS_TXPWR_MAX_CCH
        *  C31 = 0
        *  GPRS_RESELECT_OFFSET(n) =
        *     CELL_RESELECT_OFFSET(n) - CELL_RESELECT_OFFSET(s)
        *  GPRS_TEMPORARY_OFFSET = TEMPORARY OFFSET
        *  GPRS_PENALTY_TIME = PENALTY_TIME
        *  PRIORITY_CLASS = 0
        *  C32_QUAL = 0
        *  GPRS_CELL_RESELECT_HYSTERESIS = CELL_RESELECT_HYSTERESIS
        *  RA_RESELECT_HYSTERESIS = CELL_RESELECT_HYSTERESIS
        */

       rr_data->nc_data[index].cr_par.gprs_rxlev_access_min =
        rr_data->nc_data[index].select_para.rxlev_access_min;
       rr_data->nc_data[index].cr_par.gprs_ms_txpwr_max_cch =
         rr_data->nc_data[index].select_para.ms_txpwr_max_cch;

      /*
       *  NOTE: If PENALTY_TIME = 11111 for a cell,
       *  the sign of CELL_RESELECT_OFFSET shall be changed
       *  and TEMPORARY OFFSET set to 0 for that cell.
       */
      if( rr_data->nc_data[index].c2_par.penalty_time EQ 31)
      {
        rr_data->nc_data[index].cr_par.gprs_temporary_offset =
          GPRS_RESEL_OFF_DEFAULT;
      }
      else
      {
        rr_data->nc_data[index].cr_par.gprs_temporary_offset =
          rr_data->nc_data[index].c2_par.temp_offset;
      }

      rr_data->nc_data[index].cr_par.priority_class = 0;
      rr_data->nc_data[index].cr_par.gprs_penalty_time =
        rr_data->nc_data[index].c2_par.penalty_time;
      rr_data->nc_data[index].cr_par.gprs_penalty_time *= 2;

      if( rr_data->nc_data[index].bcch_status NEQ DECODED )
        TRACE_EVENT_P1("Error BCCH in NOT DECODED for arfcn %d  in att_convert_idle_cr" ,
                                       rr_data->nc_data[index].arfcn);
      break;
    default :
      break;

  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_insert_c31_cr_data_in_cell    |
+--------------------------------------------------------------------+

  PURPOSE : To store T_cr_par in rr_data->nc_data

*/

GLOBAL void att_insert_c31_cr_data_in_cell  (UBYTE index)
{
  GET_INSTANCE_DATA;
   UBYTE i;
   TRACE_EVENT_P1("att_insert_c31_cr_data_in_cell called for arfcn=%d" ,
                                 rr_data->nc_data[index].arfcn  );


   rr_data->nc_data[index].v_cr_par = CR_PAR_INVALID;
   for( i = 0; i < rr_data->gprs_data.num_add_freq_list; i++ )
   {
     if( rr_data->nc_data[index].arfcn EQ rr_data->gprs_data.add_freq_lists[i].arfcn )
     {
        if( rr_data->gprs_data.add_freq_lists[i].v_cr_par )
        {
          TRACE_EVENT_P2(" Cr_par found for arfcn =%d, index in add freq list = %d",
                                          rr_data->nc_data[index].arfcn, i);
          rr_data->nc_data[index].v_cr_par = CR_PAR_VALID;
          memcpy( & (rr_data->nc_data[index].cr_par),
                         & (rr_data->gprs_data.add_freq_lists[i].cr_par),
                         sizeof( T_cr_par) );
          att_convert_idle_c31_cr(index);
        }
        else
        {
          TRACE_EVENT_P1("Cr_par found for arfcn =%d in add freq list", rr_data->nc_data[index].arfcn);
          if( rr_data->nc_data[index].bcch_status EQ DECODED)
            att_convert_idle_c31_cr(index);
        }
        return;
     }
   }
   if( rr_data->nc_data[index].bcch_status EQ DECODED)
     att_convert_idle_c31_cr(index);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : att_init_c31_cr_par                          |
+-----------------------------------------------------------------------------+

  PURPOSE :

*/
static void att_init_nc_data_c31_cr_par(void)
{
  GET_INSTANCE_DATA;
  UBYTE  index = 0;
  TRACE_EVENT(" att_init_nc_data_c31_cr_par called");
  for(index = 0; index <= SC_INDEX; index++)
  {
     rr_data->nc_data[index].v_cr_par = CR_PAR_INVALID;

     if( rr_data->nc_data[index].bcch_status NEQ EMPTY )
     {
       att_insert_c31_cr_data_in_cell(index);
     }
  }
}


/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : att_init_cr_c31_data                         |
+-----------------------------------------------------------------------------+

  PURPOSE :

*/
static void att_init_cr_c31_data(void)
{
  GET_INSTANCE_DATA;
  UBYTE  i = 0;
  UBYTE  first_valid_par = 0;
  UBYTE  valid_found = FALSE;

  for( ; (i < rr_data->gprs_data.num_add_freq_list) AND (valid_found EQ FALSE)  ; i++)
  {
    if( rr_data->gprs_data.add_freq_lists[i].v_cr_par)
    {
      valid_found = TRUE;
      first_valid_par = i;
    }
  }

  if( valid_found )
  {
    for( i = first_valid_par + 1 ; i < rr_data->gprs_data.num_add_freq_list    ; i++)
    {
      if( rr_data->gprs_data.add_freq_lists[i].v_cr_par EQ FALSE )
      {
        /* entire structure is missing */
        rr_data->gprs_data.add_freq_lists[i].v_cr_par = TRUE;
        memcpy(& (rr_data->gprs_data.add_freq_lists[i].cr_par),
                      & (rr_data->gprs_data.add_freq_lists[i-1].cr_par), sizeof(T_cr_par));
      }
      else
      {
         /* a part of the structure may be missing */
         if( rr_data->gprs_data.add_freq_lists[i].cr_par.gprs_rxlev_access_min EQ
              GPRS_RXLEV_ACCESS_MIN_INVALID)
           rr_data->gprs_data.add_freq_lists[i].cr_par.gprs_rxlev_access_min =
             rr_data->gprs_data.add_freq_lists[i-1].cr_par.gprs_rxlev_access_min;

         if(rr_data->gprs_data.add_freq_lists[i].cr_par.gprs_ms_txpwr_max_cch EQ
             GPRS_MS_TXPWR_MAX_CCH_INVALID)
           rr_data->gprs_data.add_freq_lists[i].cr_par.gprs_ms_txpwr_max_cch =
             rr_data->gprs_data.add_freq_lists[i-1].cr_par.gprs_ms_txpwr_max_cch;

         if(rr_data->gprs_data.add_freq_lists[i].cr_par.gprs_temporary_offset EQ
             GPRS_TEMPORARY_OFFSET_INVALID )
           rr_data->gprs_data.add_freq_lists[i].cr_par.gprs_temporary_offset =
             rr_data->gprs_data.add_freq_lists[i-1].cr_par.gprs_temporary_offset;

         if(rr_data->gprs_data.add_freq_lists[i].cr_par.gprs_penalty_time EQ
             GPRS_PENALTY_TIME_INVALID)
           rr_data->gprs_data.add_freq_lists[i].cr_par.gprs_penalty_time =
             rr_data->gprs_data.add_freq_lists[i-1].cr_par.gprs_penalty_time;

         if(rr_data->gprs_data.add_freq_lists[i].cr_par.gprs_reselect_offset EQ
            GPRS_RESEL_OFF_INVALID)
           rr_data->gprs_data.add_freq_lists[i].cr_par.gprs_reselect_offset =
             rr_data->gprs_data.add_freq_lists[i-1].cr_par.gprs_reselect_offset;

         if(rr_data->gprs_data.add_freq_lists[i].cr_par.hcs_thr EQ
            GPRS_HCS_THR_INVALID )
           rr_data->gprs_data.add_freq_lists[i].cr_par.hcs_thr =
             rr_data->gprs_data.add_freq_lists[i-1].cr_par.hcs_thr;

         if(rr_data->gprs_data.add_freq_lists[i].cr_par.priority_class EQ
            GPRS_PRIORITY_CLASS_INVALID)
           rr_data->gprs_data.add_freq_lists[i].cr_par.priority_class =
             rr_data->gprs_data.add_freq_lists[i-1].cr_par.priority_class;

        if(rr_data->gprs_data.add_freq_lists[i].cr_par.si13_location EQ
           SI13_LOC_INVALID)
          rr_data->gprs_data.add_freq_lists[i].cr_par.si13_location =
            rr_data->gprs_data.add_freq_lists[i-1].cr_par.si13_location;

        if(rr_data->gprs_data.add_freq_lists[i].cr_par.psi1_repeat_period EQ
           PSI1_RPT_PRD_INVALID )
          rr_data->gprs_data.add_freq_lists[i].cr_par.psi1_repeat_period =
            rr_data->gprs_data.add_freq_lists[i-1].cr_par.psi1_repeat_period;

        if(rr_data->gprs_data.add_freq_lists[i].cr_par.pbcch_location EQ
           PBCCH_LOC_INVALID )
          rr_data->gprs_data.add_freq_lists[i].cr_par.pbcch_location =
            rr_data->gprs_data.add_freq_lists[i-1].cr_par.pbcch_location;

      }
    }
  }

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                       |
| STATE   : code                ROUTINE : att_calculate_c31_c32         |
+--------------------------------------------------------------------+

  PURPOSE : calculates C31 and C32 criterion of cell reselection is still valid.

*/

GLOBAL void att_calculate_c31_c32 (UBYTE index)
{
  GET_INSTANCE_DATA;
  USHORT                  h;
  T_NC_DATA            *rrd;
  T_NC_DATA            *sc_ptr;

  UBYTE                     prios, prion;
  /*lint -esym(526,p_res_off) : not defined | defined in GRR */
  extern const SHORT  p_res_off[];

  /* C1     = A - MAX(B,0)                                                 */
  /* A      = RLA_P - GPRS_RXLEV _ACCESS_MIN                                */
  /* B      = GPRS_MS_TXPWR_MAX_CCH - P                                    */
  /* P      = max MS RF output power                                       */

  /* C31    = RLA_P(s) - HCS_THR(s) (serving cell)                         */
  /* C31    = RLA_P(n) - HCS_THR(n) - TO(n)*L(n) (neighbour cell)          */
  /* TO(n)  = GPRS_TEMPORARY_OFFSET(n) * H( GPRS_PENALTY_TIME(N) - T(n) )  */
  /* L(n)   = 0 if PRIORITY_CLASS(n) = PRIORITY_CLASS(s)                   */
  /*          1 if PRIORITY_CLASS(n) = PRIORITY_CLASS(s)                   */

  /* C32(s) = C1(s)                                                        */
  /* C32(n) = C1(n) + GPRS_RESELECT OFFSET(n) - TO(n)*(1-L(n))             */
  /* TO(n)  = GPRS_TEMPORARY_OFFSET(n) * H( GPRS_PENALTY_TIME(N) - T(n) )  */
  /* L(n)   = 0 if PRIORITY_CLASS(n) = PRIORITY_CLASS(s)                   */
  /*          1 if PRIORITY_CLASS(n) = PRIORITY_CLASS(s)                   */
  /* H(n)   = 0 for x <  0                                                 */
  /*          1 for x >= 0                                                 */

  TRACE_FUNCTION ("att_calculate_c31_c32()");

  /*
   * get a pointer to the data of the cell
   */
  rrd = &(rr_data->nc_data[index]);

  att_calculate_c1(index);

  TRACE_EVENT_P5("arfcn = %d, C1 = %d, rxlev = %d, GPRS_RXLEV _ACCESS_MIN = %d, GPRS_MS_TXPWR_MAX_CCH = %d",
                            rrd->arfcn, rrd->c1,
                            rrd->rxlev,
                            rrd->cr_par.gprs_rxlev_access_min,
                            rrd->cr_par.gprs_ms_txpwr_max_cch);

  TRACE_EVENT_P3 ("hcs thr = %d, prion = %d, valid = %d",
                                 rrd->cr_par.hcs_thr, rrd->cr_par.priority_class , rrd->v_cr_par);

  if( rrd->v_cr_par EQ CR_PAR_INVALID)
  {
    rrd->c31 = 0; /* conversion of idle mode to GPRS cell reselection parameters */
  }
  else
  {
    rrd->c31 = rrd->rxlev;
    if( rrd->cr_par.hcs_thr NEQ 31 )
      rrd->c31 -= ( 2 * rrd->cr_par.hcs_thr );
    else
      rrd->c31 = -32768;
  }

   rrd->c32 = rrd->c1;

    /* C31    = RLA_P(s) - HCS_THR(s) (serving cell)                         */
    /* C32(s) = C1(s)                                                        */

   TRACE_EVENT_P1("C31 = %d", rrd->c31);

   if( index EQ SC_INDEX )
     return;

   sc_ptr = &rr_data->nc_data[SC_INDEX];
   prios = (sc_ptr->cr_par.priority_class NEQ GPRS_PRIORITY_CLASS_INVALID) ?
              sc_ptr->cr_par.priority_class : (UBYTE) 0;

   prion = (rrd->cr_par.priority_class NEQ GPRS_PRIORITY_CLASS_INVALID ) ?
               rrd->cr_par.priority_class : (UBYTE) 0;

   if(  rr_data->nc_data[index].v_cr_par  EQ CR_PAR_INVALID)
   {
      /*
        * conversion of idle mode paramter to GPRS cell reselection parameters.
        * the conversion for GPRS_RESELECTION_OFFSET is done here.
        * the coversion of other parameters are done as
        * part of the function att_calculate_c1()
        *
        * GPRS_RESELECT_OFFSET(n) =
        *         CELL_RESELECT_OFFSET(n) - CELL_RESELECT_OFFSET(s)
        *
        * NOTE: If PENALTY_TIME = 11111 for a cell,
        * the sign of CELL_RESELECT_OFFSET shall be changed
        *
        */
     SHORT nc_sign = 1;
     SHORT sc_sign = 1;


     if(rrd->cr_par.gprs_penalty_time EQ 31 )
     {
       nc_sign = -1;
     }
     if(sc_ptr->cr_par.gprs_penalty_time EQ 31 )
     {
       sc_sign = -1;
     }
     rrd->c32 += (USHORT)(nc_sign * 2 * rrd->c2_par.cell_reselect_offset ) -
                 (USHORT)(sc_sign * 2 * sc_ptr->c2_par.cell_reselect_offset);
     TRACE_EVENT_P5(" nc_sign =%d, sc_sign =%d, nc_rese_off=%d, sc_res_off=%d, c31=%d",
                                     nc_sign, sc_sign, rrd->c2_par.cell_reselect_offset, sc_ptr->c2_par.cell_reselect_offset,
                                     rrd->c32);
   }
   else
   {
     /* a valid paramter for GPRS_RESELECT_OFFSET is received */
     rrd->c32 += p_res_off[rrd->cr_par.gprs_reselect_offset];
     TRACE_EVENT_P3(" gprs_reselect_offset = %d, c32 = %d, res off index =%d",
           p_res_off[rrd->cr_par.gprs_reselect_offset], rrd->c32, rrd->cr_par.gprs_reselect_offset);
   }
    /* c32_qual is not known, if there is no PCCCH */


  /*
   * Calculate x for H(x) and set H(x)
   */
  TRACE_EVENT_P2("penalty time = %d, avail time = %d",  (rrd->cr_par.gprs_penalty_time + 1) * 10, rrd->avail_time/PERIOD_1_SEC );
  if ((USHORT)((rrd->cr_par.gprs_penalty_time + 1) * 10)   >=
      (USHORT)(rrd->avail_time/PERIOD_1_SEC))
    h = TRUE;
  else
    h = FALSE;

  if (h)
  {
    SHORT *criterion;
    TRACE_EVENT_P3("penalty timer not expired: %d offset will be applied to (prios)%d NEQ (prion)%d (if 0 c31, if 1 C32 )",
                    10 * rrd->cr_par.gprs_temporary_offset,
                    prios, prion);
    if( prios NEQ prion )
    {
      criterion = & (rrd->c31);
    }
    else
    {
      criterion = & (rrd->c32);
    }
    if( rrd->cr_par.gprs_temporary_offset EQ 7 )
    {
      *criterion = -32768;
    }
    else
    {
      *criterion -= 10 * rrd->cr_par.gprs_temporary_offset;
    }
  }
  else
  {
    TRACE_EVENT("penalty timer expired");
  }
  if( rrd->cr_par.same_ra_as_serving_cell NEQ SAME_RA_AS_SCELL)
  {
    TRACE_EVENT(" cell is NOT SAME_RA_AS_SCELL");
    rrd->c32 -= 2 * rrd->select_para.cell_resel_hyst ;
  }
  else if(rr_data->gprs_data.ready_state)
  {
    /* MS in ready state*/
    TRACE_EVENT(" MS in ready state");
    rrd->c32 -= (SHORT)2 * rrd->select_para.cell_resel_hyst ;
    /* C31 hyst not known */
  }
  if ( IS_TIMER_ACTIVE (T_NO_RESELECT) )
  {
    rrd->c32 -= 5;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                             |
| STATE   : code                ROUTINE : att_get_next_best_c32_index       |
+--------------------------------------------------------------------+

  PURPOSE : This function returns the index of a target cell, for reselection.
  If no target cell is available then NO_AVAILABLE is returned.
  The parameter c31_calculated, indicates if C31 and C32 are already calculated.

*/
GLOBAL UBYTE att_get_next_best_c32_index (BOOL c31_calculated)
  {
  GET_INSTANCE_DATA;
  UBYTE max_result = SC_INDEX;
  UBYTE result = NO_AVAILABLE;
  UBYTE i;
  T_cr_par      *ncell_first, *ncell_next, *scell_ptr;
  BOOL           c31_bc, c31_nc, c31_sc;

  TRACE_FUNCTION ("att_get_next_best_c32_index()");

  switch (rr_data->sc_data.selection_type)
  {
    case BACK_FROM_DEDICATED:
    case BACK_FROM_DEDICATED_RLF:
    case CELL_RESELECTION_CR:
    case CELL_RESELECTION_RACH:

      /* not valid */
      break;

    default:
      /*
       * Normal Cell reselection
       *
       */
      if( ! c31_calculated )
      {
        att_calculate_c31_c32(SC_INDEX);
        for (i = 0; i < SC_INDEX; i++)
        {
          if (rr_data->nc_data[i].bcch_status EQ DECODED)
          {
            att_calculate_c31_c32 (i);
          }
        }
      }
      TRACE_EVENT_P1("Scell = %d", rr_data->nc_data[SC_INDEX].arfcn);
      for(i=0;i<=SC_INDEX; i++)
      {
        if( rr_data->nc_data[i].bcch_status EQ DECODED )
        {
          ncell_first = &rr_data->nc_data[i].cr_par;
          TRACE_EVENT_P6("First best Cell(arfcn=%d), C1=%d,C31=%d,C32=%d,Priority class =%d,hcs_thr=%d",
                          rr_data->nc_data[i].arfcn,
                          rr_data->nc_data[i].c1,
                          rr_data->nc_data[i].c31,
                          rr_data->nc_data[i].c32,
                          ncell_first->priority_class,
                          ncell_first->hcs_thr  );
        }
      }

      /*
       * Now find the the first one which is not previously used
       */
      for (i = 0; (i < SC_INDEX) AND (max_result EQ SC_INDEX) ; i++)
      {
        if ( (!rr_data->nc_data[i].c32_used) AND
                rr_data->nc_data[i].bcch_status EQ DECODED AND
                rr_data->nc_data[i].c1 > 0)
        {
          max_result = i;
        }
      }
      if( max_result EQ SC_INDEX )
      {
        return NO_AVAILABLE;
      }
      ncell_first = &( rr_data->nc_data[max_result].cr_par);
      c31_bc = ( (rr_data->nc_data[max_result].c31 >= 0) AND
                    (ncell_first->priority_class NEQ GPRS_PRIORITY_CLASS_INVALID) );

      /* choose the best cell */
      for (i = max_result+1; i < 6 ; i++)
      {
        if (
             (!rr_data->nc_data[i].c32_used) AND
             (rr_data->nc_data[i].bcch_status EQ DECODED) AND
             (rr_data->nc_data[i].c1) > 0
           )
        {
          ncell_next = &( rr_data->nc_data[i].cr_par);
          c31_nc = ( (rr_data->nc_data[i].c31 >= 0) AND
                           (ncell_next->priority_class NEQ GPRS_PRIORITY_CLASS_INVALID ));
          if(
              (
               c31_bc AND
               c31_nc AND
               (ncell_next->priority_class EQ ncell_first->priority_class ) AND
               (rr_data->nc_data[i].c32 > rr_data->nc_data[max_result].c32)
              )
              OR
              (
               c31_bc AND
               c31_nc AND
               (ncell_next->priority_class > ncell_first->priority_class )
              )
            )
          {
            /* prioritized cell reselection must be done */
            max_result = i;
            ncell_first = ncell_next;
          }
          else if ( !c31_bc )
          {
            if( c31_nc)
            {
              c31_bc = c31_nc;
              max_result = i;
              ncell_first = ncell_next;
            }
            else if ( rr_data->nc_data[i].c32 > rr_data->nc_data[max_result].c32 )
            {
              /* no prioritized cell found yet */
              max_result = i;
              ncell_first = ncell_next;
            }
          }
        }
      }

      /* compare with Serving cell */
      scell_ptr = &( rr_data->nc_data[SC_INDEX].cr_par);
      c31_sc = ( (rr_data->nc_data[SC_INDEX].c31 >= 0 ) AND
                 (scell_ptr->priority_class NEQ GPRS_PRIORITY_CLASS_INVALID));
      TRACE_EVENT_P2("c31_sc=%d and c31_bc=%d",c31_sc, c31_bc);
      if(
          (
             c31_sc AND
             c31_bc AND
             (ncell_first->priority_class EQ scell_ptr->priority_class) AND
             (rr_data->nc_data[max_result].c32 > rr_data->nc_data[SC_INDEX].c32)
          )
          OR
          (
             c31_sc AND
             c31_bc AND
             (ncell_first->priority_class > scell_ptr->priority_class)
          )
          OR
          ( c31_bc AND  !c31_sc )
          OR
          (
            !c31_bc AND
            !c31_sc   AND
            (rr_data->nc_data[max_result].c32 > rr_data->nc_data[SC_INDEX].c32)
          )
        )
      {
        result = max_result;
      }
      else
      {
        result = NO_AVAILABLE;
      }
  }

  /*
   * return the index of the highest C2.
   */
  return result;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_check_c31_reselect_decision|
+--------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void att_check_c31_reselect_decision (UBYTE start_now)
{
  GET_INSTANCE_DATA;
  UBYTE index;
  UBYTE delta = 1;
  TRACE_FUNCTION("att_check_c31_reselect_decision()");
  index = att_get_next_best_c32_index(TRUE);
  while( index NEQ NO_AVAILABLE )
  {
    /* found a suitable cell to reselect */
      TRACE_EVENT_P1("found a cell %d", index);
    /*
       * Check cell barred status before going
       * into Cell Reselection state.
       * This will actually also be checked again
       * in att_check_cell but the CRH value will
       * not be used for subsequent ncells leading
       * to false CR's. We check it here
       * to use CRH for the next cell if needed.
       */
/* Implements RR Clone findings #8 */
      if(att_cell_barred_status_cr_no_cr( index))
          delta =0;
   
      /*
       * Do not trigger a cell reselection decision on meas. report
       * receipt if the cell belong to a LA not allowed for roaming.
       * According to 3.22 chapter 3.5.4 cell reselection on a
       * forbidden LA for regional provision of service is allowed.
       */
      if (rr_data->ms_data.rr_service EQ FULL_SERVICE)
      {
        if (!dat_roam_forb_lai_check(index))
          delta = 0;
#if defined(_SIMULATION_)
          TRACE_EVENT_WIN_P1 ("delta=%d", delta);
#endif
      }

      if (delta > 0)
      {
          if (start_now)
          {
            att_start_cell_reselection_gprs(CELL_RESELECTION_NC);
          }
          return;
      }
      rr_data->nc_data[index].c32_used = TRUE;
      index = att_get_next_best_c32_index(TRUE);
  }
  TRACE_EVENT(" index == NO_AVAILABLE in function att_check_c31_reselect_decision");


}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_check_cell_c31              |
+--------------------------------------------------------------------+

  PURPOSE : if a cell reselection decision has been taken based on C31
            and C32 criterion , this function looks for a candidate for cell
            reselection taking in account several requirements.
            The reselect_index stores the index of the RR storage area
            for the neighbourcell. The return value indicates whether a
            cell has been found or not.
*/


GLOBAL BOOL att_check_cell_c31 (void)
{
GET_INSTANCE_DATA;
  BOOL cell_ok = FALSE;

  TRACE_FUNCTION ("att_check_cell_c31()");

  /*
   * while no suitable candidate has been found,
   * but still possible cells available.
   */
  while (rr_data->reselect_index NEQ NO_AVAILABLE AND ! cell_ok)
  {
    /*
     * calculate the cell reselection criterion C31 and C32 for the candidate
     */
    //att_calculate_c31_c32(rr_data->reselect_index);
    /*
      * attempting reselection. Mark used
      */
    rr_data->nc_data[rr_data->reselect_index].c32_used = TRUE;

    /*
      *  Cell should not be barred.
      *  If GPRS cell reselection parameters are received, look for CELL_BAR_ACCESS_2
      *  else SI3 RACH parameters would indicate cell barred
      */
    if( rr_data->nc_data[rr_data->reselect_index].v_cr_par EQ CR_PAR_VALID)
    {
      TRACE_EVENT_P2("Cell Barred arfcn = %d, barred = %d",
                                     rr_data->nc_data[rr_data->reselect_index].arfcn,
                                     rr_data->nc_data[rr_data->reselect_index].cr_par.cell_bar_access_2 );
      cell_ok = (rr_data->nc_data[rr_data->reselect_index].cr_par.cell_bar_access_2
                      EQ CBA_2_NORMAL) ? TRUE : FALSE;
    }
    else
    {
/* Implements RR Clone findings #8 */
     cell_ok = !att_cell_barred_status_cr_no_cr (rr_data->reselect_index);
    }
    /*
     * 1. the cell shall be not temporarily excluded (tnnn e.g. after random access
     *    failure).
     * 2. C31-C32 parameters are still valid
     */
    if( cell_ok )
      cell_ok = ( ! is_tnnn (rr_data->reselect_index) AND
                      att_check_c31_criterion(rr_data->reselect_index) );

     if ((cell_ok) AND (rr_data->ms_data.rr_service EQ FULL_SERVICE))
    {
      if(rr_data->nc_data[rr_data->reselect_index].v_cr_par EQ CR_PAR_VALID)
      {
        cell_ok = (rr_data->nc_data[rr_data->reselect_index].cr_par.exc_acc
                       EQ EXC_ACC_NORMAL) ? TRUE : FALSE;
      }
      else
        cell_ok = dat_roam_forb_lai_check (rr_data->reselect_index);
    }

    if (cell_ok)
    {
      /*
       * suitable cell has been found
       */
      TRACE_EVENT ("use neighbour cell");

      /*
       * configure layer 1 for cell reselection
       */
      TRACE_EVENT_P2 ("config L1 for CR [%d]->[%d]",
        rr_data->nc_data[SC_INDEX].arfcn,
        rr_data->nc_data[rr_data->reselect_index].arfcn);

      rr_data->gprs_data.ready_state         = FALSE;    /* Standby */

      att_start_cr_in_pl(rr_data->reselect_index);
    }
    else
    {
      /*
       * cell is not suitable. Look for another cell.
       */
      rr_data->reselect_index =
        att_get_next_best_c32_index(TRUE);
    }
  }

  /*
   * return whether a cell has been found or not.
   */
  return cell_ok;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                       |
| STATE   : code                ROUTINE : att_check_c31_criterion         |
+--------------------------------------------------------------------+

  PURPOSE : returns TRUE if C31 and C32 criterion of cell reselection is still valid.

*/
GLOBAL BOOL att_check_c31_criterion(UBYTE index)
{
  GET_INSTANCE_DATA;
  T_cr_par      *ncell_ptr, *scell_ptr;
  BOOL           c31_bc, c31_sc;

  att_calculate_c31_c32(index);

  ncell_ptr = & rr_data->nc_data[index].cr_par;
  scell_ptr = & rr_data->nc_data[SC_INDEX].cr_par;

  c31_bc = ((rr_data->nc_data[index].c31 >= 0) AND
                  (ncell_ptr->priority_class NEQ GPRS_PRIORITY_CLASS_INVALID ));
  c31_sc = ( (rr_data->nc_data[SC_INDEX].c31 >= 0 ) AND
                  (scell_ptr->priority_class NEQ GPRS_PRIORITY_CLASS_INVALID));
  /*
    * check for C32 values, depending on priority
    */

  if(
      (
        c31_sc AND
        c31_bc AND
        (ncell_ptr->priority_class > scell_ptr->priority_class)
      )
      OR
      (
        c31_sc AND
        c31_bc AND
        (ncell_ptr->priority_class EQ scell_ptr->priority_class) AND
        (rr_data->nc_data[index].c32 > rr_data->nc_data[SC_INDEX].c32)
      )
      OR

      (
        c31_bc AND (!c31_sc)
      )
      OR
      (
        (!c31_sc) AND
        (!c31_bc) AND
        (rr_data->nc_data[index].c32 > rr_data->nc_data[SC_INDEX].c32)
      )
     )
  {
       return TRUE;
  }
  return FALSE;
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : att_rrgrr_update_ba_req             |
+-----------------------------------------------------------------------------+

  PURPOSE :

*/
void att_rrgrr_update_ba_req(T_RRGRR_UPDATE_BA_REQ* ba_req)
{
  GET_INSTANCE_DATA;
  UBYTE         i;
  T_LIST        new_list;
  T_CELL_DATA * cd;

  TRACE_EVENT ("update_ba");
  switch(GET_STATE(STATE_GPRS))
  {
    case GPRS_PIM_BCCH:
    case GPRS_PTM_BCCH:
    case GPRS_PIM_PBCCH:
    case GPRS_PAM_PBCCH:
    case GPRS_PTM_PBCCH:
#if 0
      if (ba_req->cell_type EQ SYNC_SCELL)
#endif
        cd = &rr_data->sc_data.cd;
#if 0 /* this is not used anymore but it still in the SAp for some time */
      else
        cd = &rr_data->cr_data.cd;
#endif

      if(!rr_data->gprs_data.ba_bcch_modified)
         srv_copy_list (&rr_data->ba_ncell_list, &cd->ncell_list, sizeof (T_LIST));
      

      /* store GPRS cell reselection parameters */
      memcpy(rr_data->gprs_data.add_freq_lists, ba_req->add_freq_list,
                   sizeof(T_add_freq_list) * RRGRR_BA_LIST_SIZE );

      srv_copy_list (&new_list, &rr_data->ba_ncell_list, sizeof (T_LIST));
      

      if( ba_req->add_freq_list[0].arfcn NEQ NOT_PRESENT_16BIT )
        rr_data->gprs_data.use_c31 = FALSE;

      for(i=0;
          i<RRGRR_BA_LIST_SIZE AND ba_req->add_freq_list[i].arfcn NEQ NOT_PRESENT_16BIT;
          i++)
      {
        /*add*/
        srv_set_channel  (&new_list, ba_req->add_freq_list[i].arfcn);

        if((ba_req->add_freq_list[i].v_cr_par) AND ( ba_req->cell_type EQ SYNC_SCELL))
        {
          rr_data->gprs_data.use_c31 = TRUE;
          TRACE_EVENT("use_c31 turned TRUE");
        }
      }

      if( rr_data->gprs_data.use_c31)
      {
        rr_data->gprs_data.num_add_freq_list = i;
        TRACE_EVENT_P1("number of added arfcn = %d", i);
        att_init_cr_c31_data();
        att_init_nc_data_c31_cr_par();
      }
      else
      {
         rr_data->gprs_data.num_add_freq_list = 0;
      }

      for(i=0;
          i<RRGRR_BA_LIST_SIZE AND ba_req->rm_freq_list[i].arfcn NEQ NOT_PRESENT_16BIT;
          i++)
      {
        /*remove*/
        srv_unset_channel(&new_list, ba_req->rm_freq_list[i].arfcn);
      }

      /*
       * The NC2 indication in the Packet Measurement Order
       * overrides the setting in the SI13
       */
      rr_data->gprs_data.nc_mode_of_pmo = ba_req->nc_mode;
      rr_data->gprs_data.ba_bcch_modified=TRUE;

      if (srv_compare_list (&cd->ncell_list, &new_list) EQ FALSE)
      {
        /* both lists are different or it is the first one */
        srv_copy_list (&cd->ncell_list,
                       &new_list,
                       sizeof (T_LIST));

        /* forward new neighbour cell list to layer 1 if
         * changes shall be indicated
         */
        if (ba_req->cell_type EQ SYNC_SCELL)
        {
          att_remove_bad_rr_data_ncells();
          att_code_mph_ncell_req (SC_INDEX);
#if defined (REL99) AND defined (TI_PS_FF_EMR)
          /*BA(BCCH) is modified by GRR, this list now loses the
           indices' sync that it has with NW, so can no longer use it 
           when moved to dedicated mode, for reporting*/
          rr_data->sc_data.ba_list_idle = FALSE;
          rr_data->sc_data.enh_para_status = ENH_PARA_INVALID_STATE;
          for_set_default_emr_data(&rr_data->sc_data.emr_data_current);
          for_set_default_emr_data(&rr_data->sc_data.emr_data_temp);
#endif
        }
      }
      break;
    default:
      break;
  }
  PFREE(ba_req);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : att_rrgrr_meas_rep_req              |
+-----------------------------------------------------------------------------+

  PURPOSE :

*/

void att_rrgrr_meas_rep_req (T_RRGRR_MEAS_REP_REQ* rrgrr_meas_rep_req) 
{
  PALLOC(mph_meas_rep_req,MPH_MEAS_REP_REQ);
  TRACE_FUNCTION("rrgrr_meas_rep_req()");
  mph_meas_rep_req->cause = rrgrr_meas_rep_req->meas_cause;
  PFREE(rrgrr_meas_rep_req);
  PSENDX(PL,mph_meas_rep_req);
}
	
/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : att_check_sync_results              |
+-----------------------------------------------------------------------------+

  PURPOSE :

*/
BOOL att_check_sync_results(T_MPH_MEASUREMENT_IND* mph_measurement_ind)
{
  UBYTE i=0;
  T_NC_DATA* pnc = NULL;

  if(mph_measurement_ind->gprs_sync EQ SYNC_RESULTS)
  {
    PALLOC(bsic_ind, RRGRR_NCELL_SYNC_IND);
    TRACE_EVENT ("check_sync_res:TRUE");

    for(; i < mph_measurement_ind->ncells.no_of_ncells ;i++)
    {
      bsic_ind->sync_result[i].arfcn = mph_measurement_ind->ncells.arfcn[i];

      if(mph_measurement_ind->ncells.bsic[i] EQ NOT_PRESENT_8BIT OR
         mph_measurement_ind->ncells.bsic[i] EQ MPH_BSIC_UNKNOWN)
      {
        if((pnc = is_nc_in_list(mph_measurement_ind->ncells.arfcn[i]))
            NEQ NULL)
        {
          pnc->arfcn = NOT_PRESENT_16BIT;
          pnc->bcch_status = EMPTY;
        }
        if(mph_measurement_ind->ncells.bsic[i] EQ NOT_PRESENT_8BIT)
          bsic_ind->sync_result[i].sb_flag      = NO_SB_FOUND;
        else
          bsic_ind->sync_result[i].sb_flag      = SB_UNKNOWN; /*UNKNOWN*/

      }
      else
      {
        /* cell with BSIC */
        if( (pnc = is_nc_in_list(mph_measurement_ind->ncells.arfcn[i])) NEQ NULL)
        {
          /*found -> update if needed */
            pnc->bsic = bsic_ind->sync_result[i].bsic =
              mph_measurement_ind->ncells.bsic[i];
            pnc->bcch_status=NON_DECODED;
        }
        else
        {
          /*add*/
          pnc = find_empty_slot_in_list();
          if(pnc)
          {
             pnc->bsic = bsic_ind->sync_result[i].bsic =
              mph_measurement_ind->ncells.bsic[i];
             pnc->arfcn = mph_measurement_ind->ncells.arfcn[i];
             pnc->bcch_status=NON_DECODED;
          }
        }
        bsic_ind->sync_result[i].sb_flag      = SB_FOUND;
      }
    }
    if(i>0)
    {
      for(/*dont init i*/; i < RRGRR_MAX_RSLT_NCELL_SYNC_IND; i++)
      {
        bsic_ind->sync_result[i].arfcn = NOT_PRESENT_16BIT;
      }

      PSENDX(GRR, bsic_ind);
    }
    else
    {
      PFREE(bsic_ind);
    }

    PFREE(mph_measurement_ind);
    return TRUE;
  }
  return FALSE;
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : att_check_bsic                      |
+-----------------------------------------------------------------------------+

  PURPOSE :

*/
void att_check_bsic(T_RRGRR_NCELL_SYNC_REQ* check_bsic)
{
  GET_INSTANCE_DATA;
#if defined (TI_PS_FF_RTD) AND defined (REL99)
  T_rr_enh_para     *p_cur = &rr_data->sc_data.emr_data_current;
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */

  TRACE_EVENT ("acq_bsic");

  switch(GET_STATE(STATE_GPRS))
  {
    case GPRS_PIM_PBCCH:
    case GPRS_PTM_PBCCH:
    case GPRS_PAM_PBCCH:
    {
  #if defined (TI_PS_FF_RTD) AND defined (REL99)
      UBYTE i,j;
  #else 
      UBYTE i;
  #endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */
      PALLOC(ncell_req, MPH_NEIGHBOURCELL_REQ);
      if(check_bsic->sync_type EQ SYNC_RECONFIRM)
        ncell_req->sync_only = RECONFIRM_SYNC_LIST;
      else
        ncell_req->sync_only = SYNC_LIST;
      memset(ncell_req->arfcn, NOT_PRESENT_8BIT, 2*MAX_NEIGHBOURCELLS);

      for(i=0;
          i<RRGRR_MAX_ARFCN_NCELL_SYNC_REQ AND
  #if defined (TI_PS_FF_RTD) AND defined (REL99)
        check_bsic->ncell_sync_list[i].arfcn 
  #else 
        check_bsic->arfcn[i] 
  #endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */
        NEQ NOT_PRESENT_16BIT; i++)
      {
  #if defined (TI_PS_FF_RTD) AND defined (REL99)
        ncell_req->arfcn[i] = check_bsic->ncell_sync_list[i].arfcn;
  #else
        ncell_req->arfcn[i] = check_bsic->arfcn[i];
  #endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */
#if defined (TI_PS_FF_RTD) AND defined (REL99)
        for(j = 0;j < check_bsic->ncell_sync_list[i].c_rtd;j++)
          p_cur->enh_para.enh_cell_list[i].rtd[j] = check_bsic->ncell_sync_list[i].rtd[j];
        p_cur->enh_para.enh_cell_list[i].v_rtd = check_bsic->ncell_sync_list[i].v_rtd;
        p_cur->enh_para.enh_cell_list[i].c_rtd = check_bsic->ncell_sync_list[i].c_rtd;
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */
      }
#if defined (TI_PS_FF_RTD) AND defined (REL99)
      attf_send_enh_para_to_alr(p_cur->rep_type,&p_cur->enh_para);
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */

      PSENDX(PL, ncell_req);
    }
    break;
  default:
    break;
  }
  PFREE(check_bsic);
}

static T_NC_DATA* is_nc_in_list(USHORT arfcn)
{
  GET_INSTANCE_DATA;
  T_NC_DATA* p_nc   = &rr_data->nc_data[0];
  T_NC_DATA* p_nc_l = &rr_data->nc_data[6];
  for(; p_nc < p_nc_l;p_nc++)
    if(p_nc->arfcn EQ arfcn) return p_nc;
  return NULL;
}

static T_NC_DATA* find_empty_slot_in_list(void)
{
  GET_INSTANCE_DATA;
  T_NC_DATA* p_nc   = &rr_data->nc_data[0];
  T_NC_DATA* p_nc_l = &rr_data->nc_data[6];
  for(; p_nc < p_nc_l;p_nc++)
    if(p_nc->bcch_status EQ EMPTY) return p_nc;
  return NULL;
}
/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : att_is_gprs_avail                   |
+-----------------------------------------------------------------------------+

  PURPOSE :

*/
BOOL att_gprs_is_avail(void)
{
  GET_INSTANCE_DATA;
  if(GET_STATE(STATE_GPRS) NEQ GPRS_NULL)
    return TRUE;
  else
    return FALSE;
}

BOOL att_gprs_cell_has_pbcch(void)
{
  GET_INSTANCE_DATA;
  UBYTE st = GET_STATE(STATE_GPRS);

  switch(st)
  {
    case GPRS_PIM_PBCCH:
    case GPRS_PAM_PBCCH:
    case GPRS_PTM_PBCCH:
    case GPRS_SUSPENDED_PBCCH:
      return TRUE;
    default:
      return FALSE;
  }
}

void att_gprs_stop_pl (void)
{
  GET_INSTANCE_DATA;
  att_build_classmark_req ();
  att_build_idle_req(SC_INDEX, MODE_CONFIG_PL);

#if defined (REL99) AND defined (TI_PS_FF_EMR)
  /*reset SI-2quater information and enhanced parameters if present
    when PBCCH is present*/
  rr_data->sc_data.cd.si2quater_status = SI2QUATER_ABSENT;
  rr_data->sc_data.cd.si2quater_pos = 0;
  memset (rr_data->sc_data.rep_count, NOT_PRESENT_8BIT, MAX_NEIGHBOURCELLS);
  for_set_default_emr_data(&rr_data->sc_data.emr_data_current);
  rr_data->sc_data.enh_para_status = ENH_PARA_INVALID_STATE;
  rr_data->sc_data.ba_list_ded = rr_data->sc_data.ba_list_idle = FALSE;
#endif

  /*
   * use neighbour cell list from system info 2/2bis/2ter
   */
  memset (rr_data->act_ncell_list, 0xFF, 2*MAX_NEIGHBOURCELLS);
  srv_create_list (&rr_data->sc_data.cd.ncell_list,
                   rr_data->act_ncell_list, MAX_NEIGHBOURCELLS, FALSE, 0);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : att_gprs_cr_rsp                     |
+-----------------------------------------------------------------------------+

  PURPOSE :

*/
void att_gprs_cr_rsp(T_RRGRR_CR_RSP* cr_rsp)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION("att_gprs_cr_rsp()");
  switch(rr_data->gprs_data.start_proc)
  {
      case START_PROC_ACTIVATE_REQ:
        srv_use_stored_prim();
        break;
      case START_PROC_ENTER_LIMITED:
        att_signal_gprs_support();
        if(att_gprs_cell_has_pbcch())
        {
          /*
           * RR is now in idle and configures L1 for
           * BCCH only
           */
          SET_STATE(STATE_GPRS, GPRS_PIM_BCCH);
          switch(GET_STATE(STATE_ATT))
          {
            case ATT_IDLE:
            case ATT_CON_EST:
              att_build_idle_req(SC_INDEX, MODE_SYS_INFO_CHANGE);
              break;
            default:
              break;
          }
        }
        else
        {
          switch(GET_STATE(STATE_GPRS))
          {
            case GPRS_PTM_BCCH:
            case GPRS_PAM_BCCH:
              /*
               * Due to the CR_IND GRR has released the TBF so we
               * have to correctly enter idle mode
               * to respect transition rules
               */
              rr_data->gprs_data.page_mode = PAG_MODE_DEFAULT;
              SET_STATE (STATE_GPRS, GPRS_PIM_BCCH);
              att_return_to_idle();
              break;
            default:
              break;
          }
        }
        break;
      case START_PROC_NORMAL_CR:
        att_start_cell_reselection (rr_data->gprs_data.cr_type);
        break;
      case START_PROC_NORMAL_CS:
        att_start_cell_selection (rr_data->gprs_data.cr_orig, CS_NOT_PARALLEL,
                                  rr_data->cs_data.current_search_mode);
        break;
      case START_PROC_CFG_CHG: /*XXX not ok*/
        att_signal_gprs_support();
        break;
      case START_PROC_GPRS_ACT:
          {
            PALLOC ( mph_mon_ctrl_req, MPH_MON_CTRL_REQ );
            TRACE_EVENT("START_PROC_GPRS_ACT----- ");

             rr_data->sc_data.mm_started = MM_ORIGINATED;

             if((rr_data->sc_data.cd.sys_info_read EQ ALL_SYS_INFO_READ) OR
                ((rr_data->sc_data.cd.sys_info_read | SYS_INFO_13_READ) EQ ALL_SYS_INFO_READ))
             {
               rr_data->sc_data.cd.sys_info_read &= ~(SYS_INFO_13_READ);
               mph_mon_ctrl_req->si_to_read = UPDATE_SI13;
               
               if( rr_data->nc_data[SC_INDEX].si13_loc_ind EQ NORMAL_BCCH)
               {
                 mph_mon_ctrl_req->action     = START_MON_NBCCH;
                 mph_mon_ctrl_req->si_to_read = UPDATE_SI13_GSM;
               }
               else
               mph_mon_ctrl_req->action     = START_MON_EBCCH;
             }
             else
             {
               TRACE_EVENT ("START_PROC_GPRS_ACT: Read Complete SI");
               rr_data->sc_data.cd.sys_info_read = NO_SYS_INFO_READ;
               mph_mon_ctrl_req->si_to_read = COMPLETE_SI;
               mph_mon_ctrl_req->action     = START_MON_NBCCH;
             }
             
             PSENDX (PL, mph_mon_ctrl_req);
             TIMERSTART (T_RESELECT, TRESELECT_VALUE);
          }
          break;
      default:
       break;
  }
  rr_data->gprs_data.start_proc = START_PROC_NOTHING;
  PFREE(cr_rsp);
}

void att_gprs_sync_req( T_RRGRR_SYNC_REQ   *sync_req)
{
  GET_INSTANCE_DATA;
  switch(GET_STATE(STATE_GPRS))
  {
    case GPRS_PIM_PBCCH:
    case GPRS_PTM_PBCCH:
      {
        memset (&rr_data->nc_data[CR_INDEX],
                0,
                sizeof (T_NC_DATA));

        rr_data->nc_data[CR_INDEX].arfcn = sync_req->arfcn;
        rr_data->nc_data[CR_INDEX].bsic  = sync_req->bsic;
        att_start_cell_reselection_pbcch(MODE_CELL_RESELECTION_SYNC_ONLY);
      }
      break;
    default:
      break;
  }
  PFREE(sync_req);
}

void att_start_cell_reselection_gprs(UBYTE mode)
{
  GET_INSTANCE_DATA;
  /*set start proc */
  if(att_gprs_is_avail())
  {
    UBYTE st = GET_STATE(STATE_ATT);
    if(st NEQ ATT_CS3 AND
       st NEQ ATT_CS2 AND
       st NEQ ATT_CS1)
    {
      att_rrgrr_cr_ind(CR_ABNORMAL);
      rr_data->gprs_data.start_proc = START_PROC_NORMAL_CR;
      rr_data->gprs_data.cr_type = mode;
      return;
    }
  }
  att_start_cell_reselection(mode);
}

void att_start_cell_selection_gprs(UBYTE originator,UBYTE search_mode)
{
  GET_INSTANCE_DATA;
  /*set start proc*/
  if(att_gprs_is_avail())
  {
    UBYTE st = GET_STATE(STATE_ATT);
    if(st NEQ ATT_CS3 AND
       st NEQ ATT_CS2 AND
       st NEQ ATT_CS1)
    {
      if(originator EQ MM_ORIGINATED)
      {
        TRACE_EVENT("start_cs_gprs: MM_ORIG!");
        att_rrgrr_cr_ind(CR_NORMAL);
      }
      else
      {
        att_rrgrr_cr_ind(CR_ABNORMAL);
      EM_FMM_RESEL_START_IND;
      }

      CS_SET_CURRENT_SEARCH_MODE(search_mode);
      rr_data->gprs_data.start_proc = START_PROC_NORMAL_CS;
      rr_data->gprs_data.cr_orig = originator;
      return;
    }
  }
  att_start_cell_selection(originator, CS_NOT_PARALLEL,search_mode);
}


void att_gprs_idle_req(T_MPH_IDLE_REQ* idle_req)
{
  GET_INSTANCE_DATA;
  /*
   * At this point it is necessary to do the following things:
   * o send a CR_IND (optional)
   * o set the parameter gprs_support (if needed, the default has already
        been set before)
   * o set the parameter reorg_only (if needed)
   */
  switch(idle_req->mod)
  {
    case MODE_CELL_SELECTION:
      if(GET_STATE(STATE_GPRS) EQ GPRS_NULL OR
         GET_STATE(STATE_GPRS) EQ GPRS_ACTIVATED)
      {
        idle_req->gprs_support = MPH_GPRS_PROCS_NOT_USED;
      }
      else
      {
        idle_req->gprs_support = MPH_GPRS_PROCS_USED;

        if(rr_data->gprs_data.page_mode EQ PAG_MODE_REORG)
            idle_req->reorg_only = REORG_ONLY;
          else
            idle_req->reorg_only = NORMAL_PGM;

        
      }
      break;
    case MODE_CONFIG_PL:
      idle_req->gprs_support = MPH_GPRS_PROCS_USED;
      break;
    case MODE_SYS_INFO_CHANGE:
      if(GET_STATE(STATE_GPRS) EQ GPRS_NULL OR
         GET_STATE(STATE_GPRS) EQ GPRS_ACTIVATED OR
         rr_data->ms_data.rr_service EQ LIMITED_SERVICE)
      {
          idle_req->gprs_support = MPH_GPRS_PROCS_NOT_USED;
      }
      else
      {
          idle_req->gprs_support = MPH_GPRS_PROCS_USED;
          idle_req->reorg_only = NORMAL_PGM;
          
      }
      break;
    default:
      break;
  }
}

BOOL att_gprs_check_ncell(void)
{
  GET_INSTANCE_DATA;
  BOOL cell_ok;
  BOOL cell_barred;
  BOOL check_nw;
  BOOL roam_forb;
  BOOL plmn_equal;

  TRACE_EVENT_P1 ("att_gprs_check_ncell() cr_pbcch_active=%d",
    rr_data->gprs_data.cr_pbcch_active);

  if((rr_data->gprs_data.cr_pbcch_active) OR (rr_data->gprs_data.cr_pcco_active))
  {
  
/* Implements RR Clone findings #8 */
    cell_barred = !att_cell_barred_status_cr_no_cr (CR_INDEX);

    check_nw    = att_check_network (&rr_data->nc_data[CR_INDEX].lai);
    roam_forb   = dat_roam_forb_lai_check (CR_INDEX);
    plmn_equal  = dat_plmn_equal_req (rr_data->nc_data[CR_INDEX].lai.mcc,
                          rr_data->nc_data[CR_INDEX].lai.mnc,
                          rr_data->ms_data.plmn.mcc,
                          rr_data->ms_data.plmn.mnc);

    TRACE_EVENT_P6 ( "r MCC/MNC=%x%x%x/%x%x%x",
      rr_data->ms_data.plmn.mcc[0],
      rr_data->ms_data.plmn.mcc[1],
      rr_data->ms_data.plmn.mcc[2],
      rr_data->ms_data.plmn.mnc[0],
      rr_data->ms_data.plmn.mnc[1],
      rr_data->ms_data.plmn.mnc[2]);
    TRACE_EVENT_P6 ( "i MCC/MNC=%x%x%x/%x%x%x",
      rr_data->nc_data[CR_INDEX].lai.mcc[0],
      rr_data->nc_data[CR_INDEX].lai.mcc[1],
      rr_data->nc_data[CR_INDEX].lai.mcc[2],
      rr_data->nc_data[CR_INDEX].lai.mnc[0],
      rr_data->nc_data[CR_INDEX].lai.mnc[1],
      rr_data->nc_data[CR_INDEX].lai.mnc[2]);
    TRACE_EVENT_P5 ("[%u] cb %d nw %d ro %d pl %d", rr_data->nc_data[CR_INDEX].arfcn,
      cell_barred, check_nw, roam_forb, plmn_equal);

    cell_ok = cell_barred AND check_nw AND roam_forb AND plmn_equal;
    if (cell_ok)
    {
      TRACE_EVENT_P2 ("NEW SC [%d]->[%d]",
        rr_data->nc_data[SC_INDEX].arfcn,
        rr_data->nc_data[CR_INDEX].arfcn);

      if(rr_data->ms_data.rr_service EQ LIMITED_SERVICE AND
         rr_data->ms_data.req_mm_service NEQ FUNC_LIM_SERV_ST_SRCH)
      {
        /*
         * We are in LIMITED service.
         * If the req_mm_service is FUNC_LIM_SERV_ST_SRCH we
         * will stay in LIMITED.
         * If we are in limited because of forbidden LAI we
         * enter FULL SERVICE.
         */
        rr_data->ms_data.rr_service = FULL_SERVICE;
      }

      /* TIMERSTOP (T_RESELECT);*/
      att_copy_cr_data ();
      memcpy (&rr_data->sc_data.cd, &rr_data->cr_data.cd,
              sizeof (T_CELL_DATA));
      if (rr_data->old_serving_cell < SC_INDEX)
        rr_data->nc_data[rr_data->old_serving_cell].avail_time =
          PERIOD_700_SEC;
      rr_data->nc_data[CR_INDEX].arfcn = NOT_PRESENT_16BIT;

      rr_data->gprs_data.cr_pbcch_active = FALSE;
      if(GET_STATE(STATE_GPRS) EQ GPRS_PIM_BCCH OR
         GET_STATE(STATE_GPRS) EQ GPRS_ACTIVATED)
      {
        att_set_pl_in_idle_mode ();
      }
      else if (att_gprs_cell_has_pbcch())
      {
        att_gprs_stop_pl();
      }
      att_signal_gprs_support();

      /* XXX ???
       * set a barrier of 15 seconds for the next cell reselection
       * if it was a cell reselection due to C2(NC) > C2(SC)
       */
      if (rr_data->sc_data.selection_type EQ CELL_RESELECTION_NC)
        TIMERSTART (T_NO_RESELECT, THIRTY_SEC/2);
    }
    else
    {
     /*
      * inform GRR that the currently chosen cell
      * not suitable and wait for the next rrgrr_cr_req
      */
      att_cell_reselection_gprs_failed();
    }
    return TRUE;
  }
  else
  {
    return FALSE;
  }

}

void att_cell_reselection_gprs_failed(void)
{
  GET_INSTANCE_DATA;
  PALLOC_SDU(gprs_ind, RRGRR_GPRS_SI13_IND, MAX_L2_FRAME_SIZE * BITS_PER_BYTE);
  gprs_ind->cause = GPRS_CELL_NOT_SUITABLE;
  gprs_ind->serving_cell_info.bcch_arfcn = rr_data->nc_data[CR_INDEX].arfcn;
  gprs_ind->serving_cell_info.bcch_bsic  = rr_data->nc_data[CR_INDEX].bsic;

  TRACE_EVENT_P4 ("cs=%u C[%d] bsic=%u #%u",
    gprs_ind->cause,
    gprs_ind->serving_cell_info.bcch_arfcn,
    gprs_ind->serving_cell_info.bcch_bsic,
    __LINE__);

  SET_STATE(STATE_GPRS, GPRS_PIM_PBCCH);
  PSENDX(GRR, gprs_ind);
}

void att_gprs_start_task (T_RRGRR_START_TASK_REQ* start_task)
{
  GET_INSTANCE_DATA;
  switch(GET_STATE(STATE_GPRS))
  {
    case GPRS_PIM_PBCCH:
    case GPRS_PAM_PBCCH:
    case GPRS_PTM_PBCCH:
      switch(start_task->state)
      {
        case TASK_STATE_PTM:
          {
            PALLOC(mph_mon_ctrl_req, MPH_MON_CTRL_REQ );
            mph_mon_ctrl_req->action = ENTER_PTM_PBCCH;
            PSENDX (PL, mph_mon_ctrl_req);
            SET_STATE(STATE_GPRS, GPRS_PTM_PBCCH);
          }
          break;
        case TASK_STATE_PIM:
          {
            PALLOC(mph_mon_ctrl_req, MPH_MON_CTRL_REQ );
            mph_mon_ctrl_req->action = ENTER_PIM_PBCCH;
            PSENDX (PL, mph_mon_ctrl_req);
            SET_STATE(STATE_GPRS, GPRS_PIM_PBCCH);
#ifdef REL99
            if(rr_data->gprs_data.cbch_info_rxvd_in_ptm)
            {
              rr_data->gprs_data.cbch_info_rxvd_in_ptm = FALSE;
              att_config_cbch();          
            }
#endif
          }
          break;
        case TASK_STATE_PAM:
          {
            SET_STATE(STATE_GPRS, GPRS_PAM_PBCCH);
          }
          break;
        default:
          break;
      }
      break;
    case GPRS_PIM_BCCH:
    case GPRS_PAM_BCCH:
    case GPRS_PTM_BCCH:
      switch(start_task->state)
      {
        case TASK_STATE_PTM:
          {
            PALLOC(mph_mon_ctrl_req, MPH_MON_CTRL_REQ );
            mph_mon_ctrl_req->action = ENTER_PTM_BCCH;
            PSENDX (PL, mph_mon_ctrl_req);
          }
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }

  {
    PALLOC(stop_cnf, RRGRR_START_TASK_CNF);
    PSENDX(GRR, stop_cnf);
  }

  PFREE(start_task);
}

void att_start_cell_reselection_pbcch(UBYTE mode)
{
  GET_INSTANCE_DATA;
  att_init_cell_selection(CELL_RESELECTION, RR_ORIGINATED);
  att_init_pl_status ();
  rr_data->bcch_error = 0;
  rr_data->dyn_config.fcr = 0;
  rr_data->dyn_config.scr = 0;
  rr_data->bcch_error = 0;
  rr_data->pag_rec    = FALSE;
  srv_clear_stored_prim (MPH_PAGING_IND);
  gprs_init_data_cr();
  srv_clear_list (&rr_data->cr_data.cd.ncell_list);
  dat_att_null();
  rr_data->gprs_data.cr_pbcch_active = TRUE;
  SET_STATE (STATE_ATT, ATT_CS3);
  att_build_idle_req (CR_INDEX, mode);
}

/*
+------------------------------------------------------------------------------
| Function    : rr_ext_meas_can_start
+------------------------------------------------------------------------------
| Description : Check if the Ext Meas procedure can be performed.
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
static BOOL rr_ext_meas_can_start ( void )
{
  GET_INSTANCE_DATA;
  if( GET_STATE(STATE_ATT) EQ ATT_IDLE )
  {
    if(
        (GET_STATE(STATE_GPRS) EQ GPRS_PIM_BCCH)
          OR
        (GET_STATE ( STATE_GPRS ) EQ GPRS_PIM_PBCCH)
      )
      return TRUE;
  }
  return FALSE;
}

/*
+------------------------------------------------------------------------------
| Function    : att_rrgrr_ext_meas_req
+------------------------------------------------------------------------------
| Description : Start Extended Measurement (GPRS) at RR.
|
| Parameters  : *rrgrr_ext_meas_req - Frequencies to be measured
|
+------------------------------------------------------------------------------
*/
GLOBAL void  att_rrgrr_ext_meas_req ( T_RRGRR_EXT_MEAS_REQ *rrgrr_ext_meas_req )
{
  GET_INSTANCE_DATA;
  T_GPRS_DATA  *gprs_data = &rr_data->gprs_data;
  UBYTE         i,n;
  T_arfcn_idx  *s;
  USHORT       *d;

  /* Store the primitive 'rrgrr_ext_meas_req' during the Ext Meas procedure. */

  if ( gprs_data->rrgrr_ext_meas_req NEQ NULL AND
       gprs_data->rrgrr_ext_meas_req NEQ rrgrr_ext_meas_req )
  {
    PFREE ( gprs_data->rrgrr_ext_meas_req );
  }
  gprs_data->rrgrr_ext_meas_req = rrgrr_ext_meas_req;

  if ( rr_ext_meas_can_start() )
  {
    PALLOC ( mph_ext_meas_req, MPH_EXT_MEAS_REQ );

    SET_STATE (STATE_CELL_SEL, CS_XMEAS);

    n = rrgrr_ext_meas_req->c_arfcn_idx;
    if ( n > RRGRR_MAX_ARFCN_EXT_MEAS )
    {
      n = RRGRR_MAX_ARFCN_EXT_MEAS;
      TRACE_ERROR ( "cut freq list (in)" );
    }
    if ( n > MAX_CHANNELS )
    {
      n = MAX_CHANNELS;
      TRACE_ERROR ( "cut freq list (out)" );
    }

    mph_ext_meas_req->num_of_chan = n;
    d = &mph_ext_meas_req->arfcn[0];
    s = &rrgrr_ext_meas_req->arfcn_idx[0];

    for ( i = 0; i < n; ++i, ++s, ++d )
      *d = s->arfcn;

    /* Other elements of mph_ext_meas_req remain uninitialized here (set in ALR). */
    mph_ext_meas_req->freq_bands = cs_get_freq_band(FALSE);
    gprs_data->ext_meas_ctrl = 0;
    PSENDX (PL, mph_ext_meas_req);
  }
  else
  {
    PALLOC ( rrgrr_ext_meas_cnf, RRGRR_EXT_MEAS_CNF );
    rrgrr_ext_meas_cnf->c_xmeas_res = 0;
    rrgrr_ext_meas_cnf->xmeas_cause = EXT_MEAS_RESET;
    rrgrr_ext_meas_cnf->call_ref = rrgrr_ext_meas_req->call_ref;
    PSENDX ( GRR, rrgrr_ext_meas_cnf );
    PFREE ( rrgrr_ext_meas_req );
    gprs_data->rrgrr_ext_meas_req = NULL;
  }
  return;
}

/*
+------------------------------------------------------------------------------
| Function    : rr_ext_meas_idx
+------------------------------------------------------------------------------
| Description : Retrieve InDeX associated to ARFCN during
|               Extended Measurement (GPRS).
|
| Parameters  : ARFCN - channel number in 'rrgrr_ext_meas_req'
|
+------------------------------------------------------------------------------
*/
GLOBAL UBYTE rr_ext_meas_idx (USHORT arfcn )
{
  GET_INSTANCE_DATA;
  UBYTE i,n;
  T_arfcn_idx *arfcn_idx;
  T_RRGRR_EXT_MEAS_REQ *rrgrr_ext_meas_req;

  rrgrr_ext_meas_req = rr_data->gprs_data.rrgrr_ext_meas_req;
  if ( rrgrr_ext_meas_req EQ NULL )
  {
    return NOT_PRESENT_8BIT;
  }

  n = rrgrr_ext_meas_req->c_arfcn_idx;
  arfcn_idx = &rrgrr_ext_meas_req->arfcn_idx[0];
  for ( i = 0; i < n; ++i, ++arfcn_idx )
  {
    if ( arfcn_idx->arfcn EQ arfcn )
      return arfcn_idx->idx;
  }

  return NOT_PRESENT_8BIT;
}

/*
+------------------------------------------------------------------------------
| Function    : rr_ext_meas_end
+------------------------------------------------------------------------------
| Description : Deallocate memory used during Extended Measurement (GPRS).
|               This function is called in three scenarios:
|                  (1) System start (pei_init)
|                  (2) Cell Change triggered via gprs_init_gprs_data
|                  (3) End of Extended Measurement procedure
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void att_ext_meas_end ( BOOL destory_rrgrr_ext_meas_cnf )
{
  GET_INSTANCE_DATA;
  /* Note : If att_ext_meas_end is called via gprs_init_gprs_data and
   * Ext Meas is running, then a repsonse from Lower Layers is still
   * to be expected, which will be ignored. */

  T_GPRS_DATA  *gprs_data = &rr_data->gprs_data;

  if ( gprs_data->rrgrr_ext_meas_req NEQ NULL )
  {
    PFREE ( gprs_data->rrgrr_ext_meas_req );
            gprs_data->rrgrr_ext_meas_req = NULL;
  }

  if ( gprs_data->rrgrr_ext_meas_cnf NEQ NULL )
  {
    if ( destory_rrgrr_ext_meas_cnf )
    {
      PFREE ( gprs_data->rrgrr_ext_meas_cnf );
    }
    gprs_data->rrgrr_ext_meas_cnf = NULL;
  }

  if ( gprs_data->mph_ext_meas_cnf NEQ NULL )
  {
    PFREE ( gprs_data->mph_ext_meas_cnf );
            gprs_data->mph_ext_meas_cnf = NULL;
  }

  if ( (gprs_data->ext_meas_ctrl & EXT_MEAS_START_CR) NEQ 0 )
  {
    /* handle the outstanding Cell Reselection request */
    gprs_data->ext_meas_ctrl &= ~EXT_MEAS_START_CR;
    att_start_cell_reselection(rr_data->sc_data.selection_type);
  }

  gprs_data->ext_meas_ctrl = 0;
}

/*
+------------------------------------------------------------------------------
| Function    : att_ext_meas_next_bsic
+------------------------------------------------------------------------------
| Description : Check for next BSIC scanning during Extended Measurement.
|               If all BSICs are scanned, then a response is sent to GRR.
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void att_ext_meas_next_bsic (void)
{
  GET_INSTANCE_DATA;
  T_GPRS_DATA          *gprs_data          = &rr_data->gprs_data;
  T_RRGRR_EXT_MEAS_REQ *rrgrr_ext_meas_req = gprs_data->rrgrr_ext_meas_req;
  T_RRGRR_EXT_MEAS_CNF *rrgrr_ext_meas_cnf = gprs_data->rrgrr_ext_meas_cnf;
  T_MPH_EXT_MEAS_CNF   *mph_ext_meas_cnf   = gprs_data->mph_ext_meas_cnf;
  UBYTE                 n                  = gprs_data->mph_ext_meas_num;
  UBYTE                 stop               = gprs_data->ext_meas_ctrl & EXT_MEAS_RESET;

  if ( (rrgrr_ext_meas_req->report_type EQ REP_TYPE_1 OR
        rrgrr_ext_meas_req->report_type EQ REP_TYPE_2    ) AND
        rrgrr_ext_meas_cnf->c_xmeas_res >= 6 OR
        rrgrr_ext_meas_cnf->c_xmeas_res >= RRGRR_MAX_ARFCN_EXT_MEAS OR
        n >= mph_ext_meas_cnf->num_of_chan OR
        stop )
  {
    /* stop PLMN search at lower layers */

    PALLOC (mph_sync_req, MPH_SYNC_REQ);
    mph_sync_req->cs = CS_STOP_PLMN_SEARCH;
    PSENDX (PL, mph_sync_req);
  }
  else
  {
    PALLOC ( mph_bsic_req, MPH_BSIC_REQ );
    mph_bsic_req->arfcn = mph_ext_meas_cnf->arfcn[n];
    PSENDX ( PL, mph_bsic_req );
  }
}

/*
+------------------------------------------------------------------------------
| Function    : att_mph_ext_meas_cnf
+------------------------------------------------------------------------------
| Description : Evaluate rxlev during Start Extended Measurement (GPRS).
|
| Parameters  : *mph_ext_meas_cnf - ARFCNs and RXLEVs from MPH
|
+------------------------------------------------------------------------------
*/
GLOBAL void  att_mph_ext_meas_cnf ( T_MPH_EXT_MEAS_CNF *mph_ext_meas_cnf )
{
  GET_INSTANCE_DATA;
  T_GPRS_DATA          *gprs_data          = &rr_data->gprs_data;
  T_RRGRR_EXT_MEAS_REQ *rrgrr_ext_meas_req =  gprs_data->rrgrr_ext_meas_req;

  /* Check that Ext Meas is still active at RR */

  if ( rrgrr_ext_meas_req NEQ NULL )
  {
    PALLOC ( rrgrr_ext_meas_cnf, RRGRR_EXT_MEAS_CNF );
    gprs_data->rrgrr_ext_meas_cnf = rrgrr_ext_meas_cnf;

    /* Depending on the Reporting Type either obtain BSICs or respond immediately. */

    if ( rrgrr_ext_meas_req->report_type EQ REP_TYPE_1 OR
         rrgrr_ext_meas_req->report_type EQ REP_TYPE_2 )
    {
      /* allocate RRGRR_EXT_MEAS_CNF, save mph_ext_meas_cnf and send initial BSIC_REQ */

      rrgrr_ext_meas_cnf->c_xmeas_res = 0;

      gprs_data->mph_ext_meas_cnf = mph_ext_meas_cnf;
      gprs_data->mph_ext_meas_num = 0;

      att_ext_meas_next_bsic();
    }
    else
    {
      /* With REP_TYPE_3 no BSIC scanning is needed, RRGRR_EXT_MEAS_CNF is sent. */
      /* Note: the premature end of the Ext Meas procedure could have been requested, */
      /*       but this 'normal' procedure end is performed instead (now). */

      UBYTE         i,n;
      T_xmeas_res  *xmeas_res;
      PALLOC (mph_sync_req, MPH_SYNC_REQ);

      n = mph_ext_meas_cnf->num_of_chan;
      if ( n > RRGRR_MAX_ARFCN_EXT_MEAS )
      {
        n = RRGRR_MAX_ARFCN_EXT_MEAS;
        TRACE_ERROR ( "cut freq list (meas_cnf)" );
      }

      rrgrr_ext_meas_cnf->c_xmeas_res = n;

      xmeas_res = &rrgrr_ext_meas_cnf->xmeas_res[0];
      for ( i = 0; i < n; ++i, ++xmeas_res )
      {
        USHORT arfcn = mph_ext_meas_cnf->arfcn[i] & ARFCN_MASK;
        UBYTE  idx   = rr_ext_meas_idx ( arfcn );

        xmeas_res->arfcn_idx.arfcn = arfcn;
        xmeas_res->arfcn_idx.idx   = idx;
        xmeas_res->rxlev           = mph_ext_meas_cnf->rx_lev[i];
        xmeas_res->bsic            = 0;
      }
      PFREE(mph_ext_meas_cnf);
      mph_sync_req->cs = CS_STOP_PLMN_SEARCH;
      PSENDX (PL, mph_sync_req);
    }
  }
  else
  {
    PFREE(mph_ext_meas_cnf);
  }
  return;
}

/*
+------------------------------------------------------------------------------
| Function    : att_rrgrr_ext_meas_stop_req
+------------------------------------------------------------------------------
| Description : Premature stop of Extended Measurement (GPRS).
|
| Parameters  : *rrgrr_ext_meas_stop_req - Signal.
|
+------------------------------------------------------------------------------
*/
GLOBAL void  att_rrgrr_ext_meas_stop_req ( T_RRGRR_EXT_MEAS_STOP_REQ *rrgrr_ext_meas_stop_req)
{
  GET_INSTANCE_DATA;
  T_GPRS_DATA *gprs_data = &rr_data->gprs_data;
  T_RRGRR_EXT_MEAS_REQ *rrgrr_ext_meas_req = gprs_data->rrgrr_ext_meas_req;

  if (rrgrr_ext_meas_stop_req)
    PFREE ( rrgrr_ext_meas_stop_req );

  if ( rrgrr_ext_meas_req EQ NULL )
    return;

  gprs_data->ext_meas_ctrl |= EXT_MEAS_RESET;

  /* Continue the stop of the Ext Meas procedure with the next response from Lower Layers. */
}

/*
+------------------------------------------------------------------------------
| Function    : att_rrgrr_standby_ind
+------------------------------------------------------------------------------
| Description : Store information that MS is in Standby State.
|
| Parameters  : T_RRGRR_STANDBY_STATE_IND
|
+------------------------------------------------------------------------------
*/
void att_rrgrr_standby_ind(T_RRGRR_STANDBY_STATE_IND* stdby)
{
  GET_INSTANCE_DATA;
  rr_data->gprs_data.ready_state = FALSE;
  PFREE(stdby);
}

/*
+------------------------------------------------------------------------------
| Function    : att_rrgrr_ready_ind
+------------------------------------------------------------------------------
| Description : Store information that MS is in Ready State.
|
| Parameters  : T_RRGRR_READY_STATE_IND
|
+------------------------------------------------------------------------------
*/
void att_rrgrr_ready_ind(T_RRGRR_READY_STATE_IND* rdy)
{
  GET_INSTANCE_DATA;
  rr_data->gprs_data.ready_state = TRUE;
  PFREE(rdy);
}

#ifdef REL99
/*
+------------------------------------------------------------------------------
| Function    : att_rrgrr_cbch_info_ind
+------------------------------------------------------------------------------
| Description : Stores the CBCH configuration received from GRR. 
|               This information corresponds to the cbch channel
|               description received on PBCCH.
|               This primitive upates RR with latest CBCH information
|               whenever new PSI8, PSI2 or PSI13 is received by GRR
|               in PIM or PTM. This primitive should not come when
|               the mobile has camped on a R97/98 network.
|               
|  Parameters  : cbch_inf points to RRGRR_CBCH_INFO_IND primitive.
|
+------------------------------------------------------------------------------
*/
void att_rrgrr_cbch_info_ind(T_RRGRR_CBCH_INFO_IND* cbch_inf)
{
  GET_INSTANCE_DATA;
  /* we should now be in R-99 network and in a cell having 
   * pbcch. Otherwise this message should not be received.
   */
  TRACE_FUNCTION("att_rrgrr_cbch_info_ind");
  if(att_gprs_get_nw_release() AND att_gprs_cell_has_pbcch()) 
  {
    /* Store the CBCH configuration in serving cell context */
    TRACE_EVENT("CBCH info on PBCCH received");
    rr_data->gprs_data.cbch_psi_valid = TRUE;
    memcpy(&(rr_data->gprs_data.cbch_psi8),&(cbch_inf->cbch),sizeof(T_cbch));
    if( GET_STATE(STATE_ATT) EQ ATT_IDLE)
    {
      att_config_cbch();
    }
  }
  else
  {
    TRACE_ERROR( " CBCH Info received when PBCCH not present or when in R97 cell");
           
  }
  PFREE(cbch_inf);
}
#endif

/*
+------------------------------------------------------------------------------
| Function    : is_nc2_used
+------------------------------------------------------------------------------
| Description : Check if the cell reselection should be done
|               or not.
| Parameters  : none
| Return Value: TRUE  -> don't do cell reselection
|               FALSE -> do cell reselection
+------------------------------------------------------------------------------
*/
BOOL is_nc2_used(void)
{
  GET_INSTANCE_DATA;
  BOOL  ret = FALSE;
  UBYTE st  = GET_STATE(STATE_GPRS);

  TRACE_FUNCTION("is_nc2_used()");
  if (st EQ GPRS_NULL OR
      st EQ GPRS_ACTIVATED)
    return ret;
  /*
   * Check:
   *  Ready State
   *  NC mode of PMO
   *  NC mode of SI13
   */
  TRACE_EVENT_P3("nc2: rdy: %d, pmo_nc: %d, si13 nc: %d",
                 rr_data->gprs_data.ready_state,
                 rr_data->gprs_data.nc_mode_of_pmo,
                 rr_data->gprs_data.is_nc2_used_in_si13);

  if(rr_data->gprs_data.ready_state)
  {
    /* we are in READY */
    if(rr_data->gprs_data.nc_mode_of_pmo NEQ NC_MODE_RESET)
    {
      if(rr_data->gprs_data.nc_mode_of_pmo EQ NC2_USED)
        ret = TRUE;
      else
        ret = FALSE;
    }
    else
    {
      /* use SI13 NC mode */
      ret = rr_data->gprs_data.is_nc2_used_in_si13;
    }
  }
  else
  {
    /* we are in STANDBY */
    /* in STANDBY NC mode is not used */
    ret = FALSE;
  }
  return ret;
}

#ifdef REL99
/*
+------------------------------------------------------------------------------
| Function    : att_gprs_get_nw_release
+------------------------------------------------------------------------------
| Description : returns the network release received in SI 13
| Parameters  : none
| Return Value: RR_GPRS_R97, RR_GPRS_R99 depending on the network 
|               release.
+------------------------------------------------------------------------------
*/
UBYTE att_gprs_get_nw_release(void)
{
  GET_INSTANCE_DATA;
  return rr_data->gprs_data.nw_release;
}
#endif

/*===========================================================================*/
/*                                                                           */
/*                                                                           */
/*                             L O C A L S                                   */
/*                                                                           */
/*                                                                           */
/*===========================================================================*/

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : att_get_scell_info                  |
+-----------------------------------------------------------------------------+

  PURPOSE : fill the serving cell struct

*/
static void att_get_scell_info (T_serving_cell_info* sc, UBYTE idx)
{
  GET_INSTANCE_DATA;

  sc->gprs_ms_txpwr_max_cch = rr_data->nc_data[idx].select_para.ms_txpwr_max_cch;
  sc->gprs_rxlev_access_min = rr_data->nc_data[idx].select_para.rxlev_access_min;
  sc->bcch_arfcn        = rr_data->nc_data[idx].arfcn;
  sc->bcch_bsic         = rr_data->nc_data[idx].bsic;
  sc->ac_class          = rr_data->nc_data[idx].rach.ac;
  /* air-interface coding */
  sc->pwr_offset        = (UBYTE)(rr_data->nc_data[idx].c2_par.power_off_ind ?
                                  rr_data->nc_data[idx].c2_par.power_off : 0);

  if (!(dat_forb_lai_check (SC_INDEX) AND
        dat_roam_forb_lai_check (SC_INDEX)))
    sc->limited         = 1;
  else
    sc->limited         = (UBYTE)(rr_data->ms_data.rr_service EQ LIMITED_SERVICE);

  sc->rr_cell_env.plmn.v_plmn = TRUE;

  memcpy (sc->rr_cell_env.plmn.mcc, rr_data->nc_data[idx].lai.mcc, SIZE_MCC);
  memcpy (sc->rr_cell_env.plmn.mnc, rr_data->nc_data[idx].lai.mnc, SIZE_MNC);

  sc->rr_cell_env.lac   = rr_data->nc_data[idx].lai.lac;
  sc->rr_cell_env.cid   = rr_data->nc_data[idx].cell_id;

  sc->bs_pa_mfrms       = rr_data->nc_data[idx].control_descr.bs_pa_mfrms;
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : att_gprs_store_nc_mode              |
+-----------------------------------------------------------------------------+

  PURPOSE : store the NC mode of SI13

*/
static void att_gprs_store_nc_mode(T_D_SYS_INFO_13* sys_info_13)
{
  GET_INSTANCE_DATA;
  if( !sys_info_13->si13_rest_oct.si13_info.flag1 AND
      sys_info_13->si13_rest_oct.si13_info.nco EQ NCO_NET_RESEL)
  {
    rr_data->gprs_data.is_nc2_used_in_si13 = TRUE;
  }
  else
  {
    rr_data->gprs_data.is_nc2_used_in_si13 = FALSE;
  }
}

#ifdef REL99
/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : att_gprs_store_nw_release           |
+-----------------------------------------------------------------------------+

  PURPOSE : stores the network release received in si13

*/
static void att_gprs_store_nw_release(T_D_SYS_INFO_13* sys_info_13)
{
  GET_INSTANCE_DATA;
  if(sys_info_13->si13_rest_oct.v_si13_info)
  {
    if(sys_info_13->si13_rest_oct.si13_info.v_sgsnr)
    {
      rr_data->gprs_data.nw_release = RR_GPRS_R99;
    }
    else
    {
      rr_data->gprs_data.nw_release = RR_GPRS_R97; 
    }
  }
  else
  {
    rr_data->gprs_data.nw_release = RR_GPRS_R97; 
  }
}

#ifdef TI_PS_FF_EMR
/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : attg_send_enh_para_to_grr              |
+-----------------------------------------------------------------------------+

  PURPOSE : To format and send a primitive indicating enhanced measurement parameters
            received from SI-2quater.
*/
GLOBAL void att_send_enh_para_to_grr(T_rr_enh_para *p_src)
{
  PALLOC(p_enh,RRGRR_SI2QUATER_IND);
  TRACE_FUNCTION("att_send_enh_para_to_grr");
  p_enh->rep_type        = p_src->grr_rep_type;
  p_enh->enh_para_struct = p_src->enh_para;
  p_enh->nc_para_struct  = p_src->nc_para;
  PSENDX(GRR, p_enh);
  return;
}
#endif
#endif

#endif  /* GPRS */
#endif  /* !RR_GPRS_C */
