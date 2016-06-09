/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  ALR_MAIN
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
|  Purpose :  This Modul defines the functions for the SDL process
|             Main_Control of the ALR functionality.
+-----------------------------------------------------------------------------
*/

#ifndef ALR_MAIN_C
#define ALR_MAIN_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_PL

/*==== INCLUDES ===================================================*/
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "typedefs.h"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_alr.h"
#include "mon_alr.h"
#include "pei.h"
#include "tok.h"
#include "pcm.h"

#ifdef GPRS
#include "alr_gprs.h"
#endif

#if defined (WIN32)
#define TRACING
#else
/* #define TRACING */
/* #define DL_TRACE_ENABLED */
#endif

#include "alr.h"
#include "alr_em.h"

#if defined (TRACING)
#define ALR_TRACE_MAIN(a)  ALR_TRACE(a)
#else
#define ALR_TRACE_MAIN(a)
#endif

#if defined (TRACING)

#define ALR_TRACE_MAIN_BSIC_REQ(s)    TRACE_EVENT_P1 ("bsic_req state %d",s)
#define ALR_TRACE_MAIN_RAND_ACC(s,m)  TRACE_EVENT_P2 ("con est on for: %d %d times", s, m);
#define ALR_TRACE_MAIN_SCELL(o,s)     TRACE_EVENT_P2 ("SC [%d]->[%d]", o, s)
#define ALR_TRACE_MAIN_VALID_BLOCK(m) TRACE_EVENT_P1 ("VALID BLOCK %d", m)
#define ALR_TRACE_MAIN_SI_UNEQ(c)     TRACE_EVENT_P1 ("si uneq %d", c)
#define ALR_TRACE_MAIN_CR(m)          TRACE_EVENT_P1 ("VALID BLOCK CR %d",m)
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
#define ALR_TRACE_MAIN_RACH(t)        TRACE_EVENT_P1 ("powerclass txpwr %d", t)
#else
#define ALR_TRACE_MAIN_RACH(p,t)      TRACE_EVENT_P2 ("powerclass gsm %d txpwr %d", p, t)
#endif
#define ALR_TRACE_MAIN_POWER_CNF      { USHORT i; \
                                      TRACE_EVENT_P1 ("power cnf: %d channels", mph_power_cnf->num_of_chan)\
                                      for (i = 0; i < mph_power_cnf->num_of_chan; i++) { \
                                        TRACE_EVENT_P2 ("rx_lev[%4d]= %d", \
                                          mph_power_cnf->arfcn[i]&ARFCN_MASK, \
                                          mph_power_cnf->rx_lev[i]);} }
#define ALR_TRACE_MAIN_RA_CNF(t1,t2,t3) TRACE_EVENT_P3 ("af: %d %d %d", t1,t2,t3)

#else /* TRACING */

#define ALR_TRACE_MAIN_BSIC_REQ(s)
#define ALR_TRACE_MAIN_RAND_ACC(s,m)
#define ALR_TRACE_MAIN_SCELL(o,s)
#define ALR_TRACE_MAIN_VALID_BLOCK(m)
#define ALR_TRACE_MAIN_SI_UNEQ(c)
#define ALR_TRACE_MAIN_CR(m)
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
#define ALR_TRACE_MAIN_RACH(t)
#else
#define ALR_TRACE_MAIN_RACH(p,t)
#endif
#define ALR_TRACE_MAIN_POWER_CNF
#define ALR_TRACE_MAIN_RA_CNF(t1,t2,t3)

#endif  /* TRACING */

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/
LOCAL UBYTE from_dedicated = FALSE;

/*==== FUNCTIONS ==================================================*/

LOCAL BOOL ma_compare_dedi_sys_buffer (T_MPHC_DATA_IND *mphc_data_ind,
                                       UBYTE msg_t);
LOCAL void ma_fill_dedi_sys_buffer (T_MPHC_DATA_IND *mphc_data_ind,
                                    UBYTE msg_t);
LOCAL void ma_stop_rxlev_periodic_req (void);
LOCAL BOOL ma_compare_sys_buffer (T_MPHC_DATA_IND *mphc_data_ind,
                                  UBYTE msg_t);
LOCAL void ma_send_dedi_unitdata (T_MPHC_DATA_IND *data_ind);
LOCAL void ma_sync_ind (UBYTE cause, USHORT arfcn);
LOCAL void ma_set_si_bitmap (UBYTE msg_t);
LOCAL void ma_fill_sys_buffer (T_MPHC_DATA_IND *data_ind, UBYTE msg_t);
LOCAL void ma_dedi_data_out (T_PH_DATA_IND *data_ind);
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
LOCAL void compare_l1_frequency_band_with_ffs (T_MPHC_INIT_L1_CON *init_l1_cnf);
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CS                     |
| STATE   : code                ROUTINE : ma_init                    |
+--------------------------------------------------------------------+

  PURPOSE : Initialize Main Control.

*/

GLOBAL void ma_init (void)
{
  GET_INSTANCE_DATA;
  memset (alr_data, 0, sizeof (T_ALR_DATA));
  alr_data->state[STATE_MA] = MA_NULL;

  alr_data->mmi = 2;        /* EVA 4 Board */
  alr_data->keypad = 2;        /* EVA 4 Board */
  alr_data->ext_display = FALSE;    /* internal display */
  alr_data->pch_data.last_start_ccch_req.bs_pa_mfrms = NOT_PRESENT_8BIT;
  ma_clean_sys_buffer (IND_ALL_SI);
  ma_clean_dedi_sys_buffer ();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_idle_req            |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_IDLE_REQ.

*/

UBYTE v_eotd = 0;  /* for test automation */

GLOBAL void ma_mph_idle_req (T_MPH_IDLE_REQ* idle)
{
  GET_INSTANCE_DATA;
  if(idle->mod EQ MODE_CELL_SELECTION)
  {
    alr_data->nc_data.eotd_avail = idle->eotd_avail | v_eotd;
#if defined (REL99) && defined (TI_PS_FF_EMR)
    if(idle->si2quater_status EQ SI2QUATER_CONFIGURE)
    {
      alr_data->nc_data.si2quater_status = SI2QUATER_CONFIGURE;
      alr_data->nc_data.si2quater_pos = idle->si2quater_pos;
    }
    else if(idle->si2quater_status EQ SI2QUATER_ABSENT )
    {
      alr_data->nc_data.si2quater_status = SI2QUATER_ABSENT;
    }
    alr_data->nc_data.si2_count = NOT_PRESENT_8BIT;
#endif
  }

  switch (GET_STATE (STATE_MA))
  {
    case MA_DEDICATED:
      ALR_TRACE_MAIN ("leave dedi");
      ma_stop_active_procs(DONT_STOP_PCH_READING);
      alr_data->bs_pa_mfrms = idle->bs_pa_mfrms;
      nc_new_serving_cell (idle->arfcn);

      switch(idle->mod)
      {
        case MODE_CELL_RESELECTION:
          ALR_TRACE_MAIN ("CR");
          trc_state_transition(__LINE__, MA_CELL_RESELECTION);
          SET_STATE (STATE_MA, MA_CELL_RESELECTION);
#ifdef GPRS
          /*
           * if going to a PBCCH cell we have to update the
           * ncell states with GRR
           */
          gprs_alr_init();
          alr_data->nc_sync_with_grr = TRUE;
#endif
          from_dedicated = TRUE;
          rach_init ();
          rach_configure_power (idle->power);
          pch_configure (idle, PGM_REORG);
          pch_save_pgm(PGM_REORG_CS); /* reset saved pgm to REORG_CS */
          nc_suspend ();  /* to set the nc state      */
          nc_start_reselect (idle->arfcn);
          break;
        case MODE_CELL_SELECTION:
          ALR_TRACE_MAIN ("CS");
          if (idle->arfcn NEQ alr_data->old_serving_cell)
          {
            ALR_TRACE_MAIN ("CS but different cell!!!");
          }
          trc_state_transition(__LINE__, MA_IDLE);
          SET_STATE(STATE_MA, MA_IDLE);
          nc_suspend(); /* stop dedicated mode activity */
          nc_add_offset();
#ifdef GPRS
          set_gprs_support(idle->gprs_support);
#endif
          pch_configure (idle, SAVED_PGM);
          pch_start_ccch_req();
          cb_start();

          nc_start_monitoring();
          break;
#ifdef GPRS
        case MODE_CONFIG_PL: /* Cell has PBCCH */
          ma_stop_active_procs(STOP_PCH_READING);
          nc_suspend(); /* stop dedicated mode activity */
          gprs_alr_init();
          set_gprs_support(idle->gprs_support);
          alr_data->nc_sync_with_grr = TRUE; 
          alr_data->gprs_data.pbcch      = TRUE;
          nc_add_offset(); /* convert counters */

          rach_init ();
          rach_configure_power (idle->power);
          pch_configure (idle, PGM_NORMAL);
          alr_data->nc_sync_with_grr = TRUE;
          SET_STATE(STATE_MA, MA_IDLE);
          break;
#endif
        default:
          break;
      }
      break;
    case MA_CON_EST:
#ifdef GPRS
      if(idle->mod EQ MODE_PACKET_TRANSFER) /* transition to PTM */
      { /*
         * NC BCCH reading doesn't need to be stopped, because it has
         * already been stopped when entering MA_CON_EST
         */
        ma_stop_active_procs(STOP_PCH_READING);
        SET_STATE (STATE_MA, MA_PTM);
      }
      else
      {
#endif  /* GPRS */
        ma_stop_active_procs(DONT_STOP_PCH_READING);
        rach_configure_power (idle->power);
        alr_data->bs_pa_mfrms = idle->bs_pa_mfrms;
        nc_new_serving_cell (idle->arfcn);
#ifdef GPRS
        if(idle->mod EQ MODE_CONFIG_PL)
        {
          ma_stop_active_procs(STOP_PCH_READING);
          gprs_alr_init();
          set_gprs_support(idle->gprs_support);
          alr_data->gprs_data.pbcch      = TRUE;
          alr_data->nc_sync_with_grr = TRUE; 
          rach_init ();
          rach_configure_power (idle->power);
          pch_configure (idle, PGM_NORMAL);
          SET_STATE(STATE_MA, MA_IDLE);
        }
        else
        {
#endif
          if (idle->arfcn EQ alr_data->old_serving_cell)
          {
            /* coming back after RACH failure */
            trc_state_transition(__LINE__, MA_IDLE);
            SET_STATE (STATE_MA, MA_IDLE);
#ifdef GPRS
            set_gprs_support(idle->gprs_support);
#endif
            pch_configure (idle, SAVED_PGM);
            pch_start_ccch_req();
            cb_start();
            nc_start_monitoring();
          }
          else
          {
            trc_state_transition(__LINE__, MA_CELL_RESELECTION);
            SET_STATE (STATE_MA, MA_CELL_RESELECTION);
#ifdef GPRS
            /*
             * if going to a PBCCH cell we have to update the
             * ncell states with GRR
             */
            gprs_alr_init();
            alr_data->nc_sync_with_grr = TRUE;
#endif
            pch_configure (idle, PGM_REORG);
            pch_save_pgm(PGM_REORG_CS); /* reset saved pgm to REORG_CS */
            nc_start_reselect (idle->arfcn);
          }
#ifdef GPRS
        }
      }
#endif
      break;
    case MA_CELL_SELECTION:
      TRACE_EVENT_P1 ("SC[%u] in use, leave CS", idle->arfcn);

#ifdef GPRS
      if(idle->mod EQ MODE_CONFIG_PL)
      {
        ma_stop_active_procs(STOP_PCH_READING);
        gprs_alr_init();
        set_gprs_support(idle->gprs_support);
        alr_data->gprs_data.pbcch      = TRUE;
        alr_data->nc_sync_with_grr = TRUE;
        rach_init ();
        rach_configure_power (idle->power);
        nc_new_serving_cell (idle->arfcn);
        pch_configure (idle, PGM_NORMAL);
        SET_STATE(STATE_MA, MA_IDLE);
        break;
      }
#endif

      ALR_EM_CONFIGURE_IDLE_MODE;

      ma_stop_active_procs(DONT_STOP_PCH_READING);

      trc_state_transition(__LINE__, MA_IDLE);
      SET_STATE (STATE_MA, MA_IDLE);
#ifdef GPRS
      alr_data->nc_sync_with_grr=FALSE;
      set_gprs_support(idle->gprs_support);
#endif
      rach_configure_power (idle->power);
      alr_data->bs_pa_mfrms = idle->bs_pa_mfrms;
      nc_check_new_ncc_permitted (idle->ncc_permitted);
      nc_new_serving_cell (idle->arfcn);
      pch_configure (idle, PGM_REORG_CS);
      pch_save_pgm(0); /* reset saved pgm to REORG_CS */
      pch_start_ccch_req();
      /*
       * nc_start_monitoring will be called when ncell list from RR
       * is received
       */
      break;

    case MA_IDLE:
      switch (idle->mod)
      {
#ifdef GPRS
        case MODE_PACKET_TRANSFER: /* transition to PTM */
          /*NC BCCH reading will be stopped when leaving idle*/
          ma_stop_active_procs(STOP_PCH_READING | STOP_MEASUREMENTS);
          SET_STATE (STATE_MA, MA_PTM);
          break;
        case MODE_CONFIG_PL:
          ma_stop_active_procs(STOP_PCH_READING|STOP_MEASUREMENTS);
          gprs_alr_init();
          set_gprs_support(idle->gprs_support);
          /* PBCCH activated in cell */
          alr_data->nc_sync_with_grr = TRUE;
          alr_data->gprs_data.pbcch  = TRUE;
          rach_init ();
          rach_configure_power (idle->power);
          pch_configure (idle, PGM_NORMAL);
          SET_STATE(STATE_MA, MA_IDLE);
          break;
#endif  /* GPRS */

        case MODE_CELL_RESELECTION:
          ALR_TRACE_MAIN ("idle mode cr");

          ma_stop_active_procs(STOP_PCH_READING | STOP_MEASUREMENTS);

          trc_state_transition(__LINE__, MA_CELL_RESELECTION);
          SET_STATE (STATE_MA, MA_CELL_RESELECTION);
#ifdef GPRS
          /*
           * if going to a PBCCH cell we have to update the
           * ncell states with GRR
           */
          gprs_alr_init();
          alr_data->nc_sync_with_grr = TRUE;
#endif
          rach_init ();
          rach_configure_power (idle->power);
          TRACE_EVENT("IDLE_REQ : Cell found after CR");
          alr_data->bs_pa_mfrms = idle->bs_pa_mfrms;
//          nc_check_new_ncc_permitted(idle->ncc_permitted);
          nc_new_serving_cell (idle->arfcn);
          pch_configure (idle, PGM_REORG);
          pch_save_pgm(PGM_REORG_CS); /* reset saved pgm to REORG_CS */
          nc_start_reselect (idle->arfcn);
          break;
#ifdef GPRS
        case MODE_CELL_RESELECTION_SYNC_ONLY:
          if(alr_data->gprs_data.pbcch)
          {
            alr_data->gprs_data.sync_only=1;
            alr_data->gprs_data.ptm=0;
            ma_stop_active_procs(STOP_PCH_READING);
            trc_state_transition(__LINE__, MA_CELL_RESELECTION);
            SET_STATE (STATE_MA, MA_CELL_RESELECTION);
            nc_new_serving_cell (idle->arfcn);
            nc_start_reselect  (idle->arfcn);
          }
          break;
#endif
        case MODE_SYS_INFO_CHANGE:
          ALR_TRACE_MAIN ("idle mode sys info changed");
          rach_init ();
          rach_configure_power (idle->power);

          alr_data->bs_pa_mfrms = idle->bs_pa_mfrms;
          nc_check_new_ncc_permitted (idle->ncc_permitted);

          if((idle->tn/2) NEQ
             (alr_data->pch_data.pl_idle.ccch_group)
#ifdef GPRS
             OR alr_data->gprs_data.pbcch
#endif
            )
          {
            /* stop CCCH reading beforehand */
            ma_stop_active_procs(STOP_PCH_READING | STOP_MEASUREMENTS);
#ifdef GPRS
            gprs_alr_init();
            set_gprs_support(idle->gprs_support);
#endif
            pch_configure (idle, PGM_REORG_CS);
            pch_save_pgm(PGM_REORG_CS); /* reset saved pgm to REORG_CS */
            pch_start_ccch_req();

            nc_start_monitoring ();
          }
          else
          {
#ifdef GPRS
            if(idle->reorg_only EQ NORMAL_PGM AND
               alr_data->gprs_data.ign_pgm EQ TRUE)
            {
              /*
               * use last page mode read on PCH
               * when the NON_DRX_TIMER has expired
               */
              pch_configure (idle, SAVED_PGM);
              pch_start_ccch_req();
              alr_data->gprs_data.ign_pgm = FALSE;
            }
            else if(alr_data->gprs_data.ign_pgm EQ FALSE)
            {
              set_gprs_support(idle->gprs_support);
              if(gprs_alr_is_supported())
              {
                gprs_alr_init();
                set_gprs_support(idle->gprs_support);
                ma_stop_scell_bcch_req();
              }
#endif
              pch_save_pgm(0); /* save current pgm */
              pch_configure (idle, SAVED_PGM);
              pch_start_ccch_req();
#ifdef GPRS
            }
            /* other case should not happen */
#endif

          }
          break;
        case MODE_CELL_SELECTION:
          /* XXX does not seem to be used by RR*/
          break;
      default:
        break;
      }
      break;
    case MA_CELL_RESELECTION:
      switch(idle->mod)
      {
        case MODE_CELL_SELECTION:

          ALR_TRACE_MAIN ("from cr use cell");
          /* we have found a suitable cell */
          ma_stop_active_procs(DONT_STOP_PCH_READING);
          rach_init ();
          rach_configure_power (idle->power);
          TRACE_EVENT("IDLE_REQ : PM config after CR");
          alr_data->bs_pa_mfrms = idle->bs_pa_mfrms;
          nc_check_new_ncc_permitted (idle->ncc_permitted);
          if (from_dedicated)
          {
            from_dedicated = FALSE;
            nc_add_offset ();
          }
          pch_configure (idle, SAVED_PGM);

	  /*
           * For OMAPS#90692 - Move to MA_IDLE before configuring 
           * the serving cell BCCH request, so as not to reset the
           * si_bitmap again 
           */
          trc_state_transition(__LINE__, MA_IDLE);
          SET_STATE (STATE_MA, MA_IDLE);
          pch_start_ccch_req();
#ifdef GPRS
          alr_data->nc_sync_with_grr=FALSE;
          set_gprs_support(idle->gprs_support);
#endif

          /*
           * resume neighbourcell operation
           */
          nc_start_monitoring ();
          break;
        case MODE_CELL_RESELECTION:
          /* the last cell wasn't the right one, try again */
          ma_stop_active_procs(STOP_PCH_READING);
	  /* don't forget to stop all processes */
#ifdef GPRS
          gprs_alr_init();
#endif
          alr_data->bs_pa_mfrms = idle->bs_pa_mfrms;
//          nc_check_new_ncc_permitted (idle->ncc_permitted);
          nc_new_serving_cell (idle->arfcn);
          pch_configure (idle, PGM_REORG);
          pch_save_pgm(PGM_REORG_CS); /* reset saved pgm to REORG_CS */
          nc_start_reselect (idle->arfcn);
          break;
#ifdef GPRS
        case MODE_CONFIG_PL:
          ma_stop_active_procs(STOP_PCH_READING);
          if (from_dedicated) from_dedicated = FALSE;

          gprs_alr_init();
          set_gprs_support(idle->gprs_support);
          alr_data->gprs_data.pbcch      = TRUE;
          alr_data->nc_sync_with_grr = TRUE;
          rach_init ();
          rach_configure_power (idle->power);
          pch_configure (idle, PGM_NORMAL);
          SET_STATE(STATE_MA, MA_IDLE);
          break;
#endif
        default:
          break;
      }
      break;
#ifdef GPRS
    case MA_PTM:
      /* back to packet idle */
      ma_stop_active_procs(DONT_STOP_PCH_READING);
      rach_configure_power (idle->power);
      alr_data->bs_pa_mfrms      = idle->bs_pa_mfrms;
      nc_new_serving_cell (idle->arfcn);
      nc_suspend();
      if (idle->arfcn EQ alr_data->old_serving_cell)
      {
        trc_state_transition(__LINE__, MA_IDLE);
        SET_STATE(STATE_MA, MA_IDLE);
#ifdef GPRS
        set_gprs_support(idle->gprs_support);
#endif

        if(idle->reorg_only EQ REORG_ONLY)
        {
          pch_configure (idle, PGM_REORG_CS);
          alr_data->gprs_data.ign_pgm = TRUE;
        }
        else
        {
          pch_configure (idle, SAVED_PGM);
          alr_data->gprs_data.ign_pgm = FALSE;
        }
        pch_start_ccch_req();
        cb_start();
        nc_start_monitoring();
      }
      else
      {
        trc_state_transition(__LINE__, MA_CELL_RESELECTION);
        SET_STATE (STATE_MA, MA_CELL_RESELECTION);
#ifdef GPRS
        /*
         * if going to a PBCCH cell we have to update the
         * ncell states with GRR
         */
        gprs_alr_init();
        alr_data->nc_sync_with_grr = TRUE;
#endif
        pch_configure (idle, PGM_REORG);
        pch_save_pgm(PGM_REORG_CS); /* reset saved pgm to REORG_CS */
        nc_start_reselect (idle->arfcn);
      }
      break;
#endif
    default:
      break;
  }
  PFREE (idle);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_neighbourcell_req   |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_NEIGHBOURCELL_REQ.

*/

GLOBAL void ma_mph_neighbourcell_req (T_MPH_NEIGHBOURCELL_REQ* mph_neighbourcell_req)
{
  GET_INSTANCE_DATA;

  switch (GET_STATE (STATE_MA))
  {
    case MA_IDLE:
    case MA_DEDICATED:
      nc_ncell_list (mph_neighbourcell_req);

      ALR_EM_CONFIGURE_NEIGHBOUERCELL_LIST;

      break;
#ifdef GPRS
    case MA_PTM:
      /*In PTM just update the BA list. No need to start monitoring at L1 through MPHC*/
      /*
       * mix new list with old list
       */
      nc_update_ba_list (alr_data->serving_cell, mph_neighbourcell_req);
      break;
#endif

    default:
      break;
  }
  PFREE (mph_neighbourcell_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_dedicated_req       |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_DEDICATED_REQ.

*/

GLOBAL void ma_mph_dedicated_req (T_MPH_DEDICATED_REQ* dedi)
{
  GET_INSTANCE_DATA;
  switch (GET_STATE (STATE_MA))
  {
    case MA_CON_EST:
        ALR_TRACE_MAIN ("dedi on");

        ma_stop_active_procs (STOP_PCH_READING);
        trc_state_transition(__LINE__, MA_DEDICATED);
        SET_STATE (STATE_MA, MA_DEDICATED);
        dedi_req (dedi);
        ma_clean_dedi_sys_buffer ();
        nc_start_dedicated (dedi->tr_para.pwrc, dedi->tr_para.dtx);
      break;

    case MA_DEDICATED:
      dedi_req (dedi);
      break;

    default:
      break;
  }
  PFREE (dedi);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_dedicated_fail_req  |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_DEDICATED_FAIL_REQ.

*/

GLOBAL void ma_mph_dedicated_fail_req (T_MPH_DEDICATED_FAIL_REQ* mph_dedicated_fail_req)
{
  dedi_fail_req ();
  PFREE (mph_dedicated_fail_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_ciphering_req       |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_CIPHERING_REQ.

*/

GLOBAL void ma_mph_ciphering_req (T_MPH_CIPHERING_REQ* mph_ciphering_req)
{
  dedi_ciph_req (mph_ciphering_req);
  PFREE(mph_ciphering_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_freq_redef_req      |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_FREQ_REDEF_REQ.

*/

GLOBAL void ma_mph_freq_redef_req (T_MPH_FREQ_REDEF_REQ* mph_freq_redef_req)
{
  dedi_freq_redef_req(mph_freq_redef_req);
  PFREE(mph_freq_redef_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_channel_mode_req    |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_CHANNEL_MODE_REQ.

*/

GLOBAL void ma_mph_channel_mode_req (T_MPH_CHANNEL_MODE_REQ* mph_channel_mode_req)
{
  dedi_chan_mode_req (mph_channel_mode_req);
  PFREE(mph_channel_mode_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_deactivate_req      |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_DEACTIVATE_REQ.

*/

GLOBAL void ma_mph_deactivate_req (T_MPH_DEACTIVATE_REQ* mph_deactivate_req)
{
  GET_INSTANCE_DATA;
  UBYTE flags = 0;
  PREUSE (mph_deactivate_req, mphc_deactivate_req, MPHC_DEACTIVATE_REQ);

  if (alr_data->pch_data.last_start_ccch_req.bs_pa_mfrms NEQ NOT_PRESENT_8BIT)
    flags |= STOP_PCH_READING;

  switch (GET_STATE(STATE_MA))
  {
    case MA_CELL_SELECTION:
    case MA_CELL_RESELECTION:
    case MA_CON_EST:
    case MA_DEDICATED:
#ifdef GPRS
    case MA_PTM:
#endif
      break;
    case MA_IDLE:
#ifdef GPRS
      if (alr_data->gprs_data.pbcch EQ TRUE)
        break;
#endif
      /*lint -fallthrough*/
    default:
      flags |= STOP_MEASUREMENTS;
      break;
  }

  ma_stop_active_procs(flags);

  ALR_EM_DEACTIVATE_L1;

  /*
   * There is no activation request for ALR. Therefore ALR is prepared
   * here for the next powering on by stopping really all running processes
   * and reinitialising the state variables.
   */
  nc_stop ();

  cb_init ();
  cs_init ();
  pch_init ();
  dedi_init ();
  SET_STATE(STATE_MA, MA_NULL); /* it isn't clear whether the whole ma_init() should be performed */
  ma_clean_sys_buffer (IND_ALL_SI);
  ma_clean_dedi_sys_buffer ();

  PSENDX(L1, mphc_deactivate_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_classmark_req       |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_CLASSMARK_REQ.

*/

GLOBAL void ma_mph_classmark_req (T_MPH_CLASSMARK_REQ* classmark)
{
  rach_configure_class (classmark->classmark.pclass,
                        classmark->classmark.pclass2);

  ALR_EM_CONFIGURE_CLASSMARK;

  PFREE (classmark);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_ext_meas_req        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_EXT_MEAS_REQ.

*/

GLOBAL void ma_mph_ext_meas_req (T_MPH_EXT_MEAS_REQ* mph_ext_meas_req)
{
  GET_INSTANCE_DATA;
  /* Process CS must be state CS_NULL (no Cell Selection active) */

  if ( GET_STATE ( STATE_CS ) EQ CS_NULL )
  {
    mph_ext_meas_req->pch_interrupt = NO_PCH_INTERRUPT;

    /* alr_data->cs_data.mph_ext_meas_req must be NULL */

    if ( IS_EXT_MEAS_RUNNING ) /*alr_data->cs_data.mph_ext_meas_req NEQ NULL*/
    {
      PFREE ( alr_data->cs_data.mph_ext_meas_req );
    }
    alr_data->cs_data.mph_ext_meas_req    = mph_ext_meas_req;
    alr_data->cs_data.ext_meas_state_pend = CS_NULL;

    ma_mph_power_req((T_MPH_POWER_REQ*) mph_ext_meas_req);
  }
  else
  {
    PALLOC ( mph_ext_meas_cnf, MPH_EXT_MEAS_CNF );
    mph_ext_meas_cnf->num_of_chan = 0;
    PSENDX ( RR, mph_ext_meas_cnf );

    PFREE (mph_ext_meas_req);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_tch_loop_req        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_TCH_LOOP_REQ.

*/

GLOBAL void ma_mph_tch_loop_req (T_MPH_TCH_LOOP_REQ* mph_tch_loop_req)
{
  dedi_tch_loop_req (mph_tch_loop_req);
  PFREE(mph_tch_loop_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_dai_req             |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_DAI_REQ.

*/

GLOBAL void ma_mph_dai_req (T_MPH_DAI_REQ* mph_dai_req)
{
  dedi_dai_req (mph_dai_req);
  PFREE(mph_dai_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_cbch_req            |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_CBCH_REQ.

*/


GLOBAL void ma_mph_cbch_req (T_MPH_CBCH_REQ* cbch_req)
{
  cb_mph_cbch_req( &cbch_req->cbch);
  PFREE (cbch_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_identity_req        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_IDENTITY_REQ.

*/

GLOBAL void ma_mph_identity_req (T_MPH_IDENTITY_REQ* mph_identity_req)
{
  pch_identity_req (mph_identity_req);
  PFREE (mph_identity_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_sync_req            |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_SYNC_REQ.

*/

GLOBAL void ma_mph_sync_req (T_MPH_SYNC_REQ* mph_sync_req)
{
  GET_INSTANCE_DATA;
  switch (mph_sync_req->cs)
  {
    case CS_CLEAN_SYS_INFO:
      /*
       * clean stored system information messages to force
       * sending to RR.
       */
      ma_clean_dedi_sys_buffer ();
      break;

    case CS_STOP_BCCH_READING:
      switch (GET_STATE (STATE_MA))
      {
        case MA_CELL_SELECTION:
          cs_stop ();
          break;
        case MA_IDLE:
          nc_stop_rr_activity (FALSE);
          break;
      }
      break;

    case CS_STOP_PLMN_SEARCH:
      switch (GET_STATE (STATE_MA))
      {
        case MA_CELL_SELECTION:
          cs_stop ();
          break;
        case MA_IDLE:
          /*
           * PLMN search is stopped. If mobile was in REORG before
           * PLMN search, this is set again after finishing all BCCH reading.
           */
          if(alr_data->pch_data.pl_idle.page_mode EQ PGM_REORG_NC_SYNC)
          {
            alr_data->pch_data.pl_idle.page_mode = page_mode_before_hplmn_search;
            pch_start_ccch_req ();
            ALR_TRACE_MAIN ("PLMN search complete - REORG reestablished");
          }
          cs_stop ();
          nc_stop_rr_activity (TRUE);
          break;
      }
      break;
    case CS_STOP_PLMN_SEARCH_AND_DEACTIVATE:
      switch (GET_STATE (STATE_MA))
      {
        case MA_CELL_SELECTION:
          cs_stop ();
          break;
        case MA_IDLE:
          /*
           * If mobile was in REORG before PLMN search, this is set again after
           * finishing PLMN search. L1 is not configured, because PCH  reading
           * is stopped anyway
           */
          if(alr_data->pch_data.pl_idle.page_mode EQ PGM_REORG_NC_SYNC)
          {
            alr_data->pch_data.pl_idle.page_mode = page_mode_before_hplmn_search;
            ALR_TRACE_MAIN ("PLMN search complete - REORG reestablished");
          }
          cs_stop ();
          nc_stop_rr_activity (TRUE);
          ma_stop_active_procs(STOP_PCH_READING | STOP_MEASUREMENTS);
          break;
      }
      {
        PALLOC(deact_req, MPHC_DEACTIVATE_REQ);
        PSENDX(L1, deact_req);
      }
      break;
#ifdef GPRS
      case CS_START_PCCO:
      alr_data->gprs_data.pcco_active = TRUE;
      if ( GET_STATE(STATE_MA) EQ MA_PTM )
      {
          ma_stop_active_procs(STOP_MEASUREMENTS |STOP_PCH_READING);
          SET_STATE(STATE_MA, MA_IDLE);
      }
      break;
#endif
    case CS_RXLEV_VAL_REQ:
      switch (GET_STATE(STATE_MA))
      {
#ifdef GPRS
        case MA_PTM:
        case MA_IDLE:
          break;
#endif /* GPRS */
        case MA_DEDICATED:
          /* EMO */
          break;
      }
      break;
    case CS_SYS_INFO_2BIS_SUPPORT_CHANGED:
      if((GET_STATE(STATE_MA) EQ MA_IDLE) AND
          (alr_data->pch_data.pl_idle.page_mode EQ PGM_NORMAL OR
           alr_data->pch_data.pl_idle.page_mode EQ PGM_EXTENDED) AND
           (alr_data->pch_data.reorg_bcch_reading EQ FALSE))
      {
         ma_stop_scell_bcch_req ();
         if(alr_data->cs_data.si_bitmap & SI_2BIS_READ)
           alr_data->cs_data.si_bitmap &= ~(SI_2BIS_READ);
         else
           alr_data->cs_data.si_bitmap |= SI_2BIS_READ;
         sc_start_periodic();
      }
      break;
    case CS_SYS_INFO_2TER_SUPPORT_CHANGED:
      /* Dont start periodic reading if we are in PGM_REORG */
      if((GET_STATE(STATE_MA) EQ MA_IDLE) AND
          (alr_data->pch_data.pl_idle.page_mode EQ PGM_NORMAL OR
           alr_data->pch_data.pl_idle.page_mode EQ PGM_EXTENDED) AND
           (alr_data->pch_data.reorg_bcch_reading EQ FALSE))
      {
        ma_stop_scell_bcch_req ();
        if(alr_data->cs_data.si_bitmap & SI_2TER_READ)
          alr_data->cs_data.si_bitmap &= ~(SI_2TER_READ);
        else
          alr_data->cs_data.si_bitmap |= SI_2TER_READ;
        sc_start_periodic();
      }
      break;
    default:
      break;
  }

  PFREE (mph_sync_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_clean_buf_req       |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_CLEAN_BUF_REQ.

*/

GLOBAL void ma_mph_clean_buf_req (T_MPH_CLEAN_BUF_REQ* mph_clean_buf_req)
{
  ma_clean_sys_buffer(mph_clean_buf_req->si_to_clean);
  PFREE(mph_clean_buf_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_stop_dedi_req       |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_STOP_DEDICATED_REQ.

*/
GLOBAL void ma_mph_stop_dedi_req(T_MPH_STOP_DEDICATED_REQ *stop_dedi)
{
  dedi_stop();
  PFREE(stop_dedi);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_power_req           |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_POWER_REQ and MPH_EXT_MEAS_REQ.

*/

GLOBAL void ma_mph_power_req(T_MPH_POWER_REQ* mph_power_req)
{
  GET_INSTANCE_DATA;
  if ( D_OPC(mph_power_req) EQ MPH_POWER_REQ && IS_EXT_MEAS_RUNNING )
  {
    PALLOC ( mph_power_cnf, MPH_POWER_CNF );
    mph_power_cnf->num_of_chan = 0;
    PSENDX ( RR, mph_power_cnf );
    TRACE_ERROR ( "Cell Sel during Ext Meas");
    PFREE (mph_power_req);
    return;
  }

  if (mph_power_req->freq_bands EQ 0)
  {
    TRACE_ERROR ("MPH_POWER_REQ with freq_bands=0 (Invalid value)!");
    TRACE_ASSERT (mph_power_req->freq_bands EQ 0);
  }

  if(!IS_EXT_MEAS_RUNNING)
  {
    /* CSI-LLD Section : 4.1.3.4.2.2 */
    if(alr_data->cs_data.p_power_req NEQ NULL)
    {
      TRACE_ERROR("MPH_POWER_REQ buffer leakage");
      PFREE(alr_data->cs_data.p_power_req);
    }
    /* Store the MPH_POWER_REQ pointer for later usage */
    alr_data->cs_data.p_power_req = mph_power_req;
  }

  alr_data->cs_data.freq_bands = mph_power_req->freq_bands;

  ALR_EM_POWER_MEASSUREMENT_REQUEST;

  switch (mph_power_req->pch_interrupt)
  {
    case NO_PCH_INTERRUPT:
      /*
       * Full list power measurements without changing the current state
       */
      alr_data->plmn_search_running = TRUE;
      nc_suspend();
      cs_power_req (NO_PCH_INTERRUPT);
      break;
    case PCH_INTERRUPT:
      /*
       * Full list power measurements for Cell Selection
       */
      alr_data->plmn_search_running = FALSE;
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
      alr_multiband_std = NOT_PRESENT_8BIT;
#endif
#ifdef GPRS
      gprs_alr_init();
#endif
      switch (GET_STATE(STATE_MA))
      {
        case MA_NULL:
        case MA_CELL_SELECTION:
          ma_stop_active_procs(DONT_STOP_PCH_READING);
          SET_STATE(STATE_MA, MA_CELL_SELECTION);
          cs_power_req (PCH_INTERRUPT);
          break;
        case MA_CELL_RESELECTION:
          ma_stop_active_procs(STOP_PCH_READING | STOP_MEASUREMENTS);
          nc_stop();
          SET_STATE(STATE_MA, MA_CELL_SELECTION);
          cs_power_req (PCH_INTERRUPT);
          break;
        case MA_CON_EST:
        case MA_DEDICATED:
        case MA_IDLE:
#ifdef GPRS
        case MA_PTM:
          alr_data->nc_sync_with_grr = TRUE;
#endif
          ma_stop_active_procs(STOP_PCH_READING | STOP_MEASUREMENTS);
          /*
           * call this explicitly here, because stop_active_procs only
           * suspends ncell procedures
           */
          nc_stop();
          SET_STATE(STATE_MA, MA_CELL_SELECTION);
          cs_power_req (PCH_INTERRUPT);
          break;
        default:
          if(!IS_EXT_MEAS_RUNNING)
          {
            PFREE (mph_power_req);
            alr_data->cs_data.p_power_req = NULL;
            TRACE_ERROR ("MPH_POWER_REQ received in invalid MA state");
          }
            break;
      }
      break;
    default:
      if(!IS_EXT_MEAS_RUNNING)
      {
        PFREE (mph_power_req);
        alr_data->cs_data.p_power_req = NULL;
        TRACE_ERROR ("MPH_POWER_REQ: Invalid search type");
      }
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_bsic_req            |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_BSIC_REQ.

*/

GLOBAL void ma_mph_bsic_req (T_MPH_BSIC_REQ* mph_bsic_req)
{
  GET_INSTANCE_DATA;
  ALR_TRACE_MAIN_BSIC_REQ (GET_STATE(STATE_MA));

#ifdef GPRS
  if(alr_data->gprs_data.pcco_active)
  {
    cs_bsic_req(mph_bsic_req);
    PFREE (mph_bsic_req);
    return;
  }
#endif

  switch (GET_STATE (STATE_MA))
  {
    case MA_CELL_SELECTION:
      cs_bsic_req (mph_bsic_req);
      break;
    case MA_IDLE:
      nc_bsic_req (mph_bsic_req);
      break;
#ifdef GPRS
    case MA_DEDICATED:

      ma_stop_active_procs(STOP_PCH_READING | STOP_MEASUREMENTS); /* DEDI-state is now DEDI_INACTIVE */
      /* DEDI-state is now DEDI_INACTIVE */
      SET_STATE(STATE_NC, NC_NULL);
#ifdef NOT_YET
      nc_suspend(); /* stop dedicated mode activity */
#endif

      SET_STATE(STATE_MA, MA_CELL_SELECTION);
      cs_bsic_req (mph_bsic_req);
      break;
#endif
    default:
      break;
  }

  PFREE (mph_bsic_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_meas_rep_req        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_MEAS_REP_REQ.
*/

#ifdef GPRS
GLOBAL void ma_mph_meas_rep_req(T_MPH_MEAS_REP_REQ * mph_meas_rep_req)
{
  GET_INSTANCE_DATA;
  USHORT i;
  TRACE_EVENT("mph_meas_rep_req");

  switch(mph_meas_rep_req->cause)
  {
    case NC_START_MEAS:
      /*
       * Reset nc mease results database
       */
      for(i=0;i<=BA_LIST_SIZE;i++)
      {
        alr_data->nc_data.cell[i].nc_rxlev = 0;
        alr_data->nc_data.cell[i].c_nc_rxlev = 0;
      }
      /*
       * activate rx_lev averaging when NC=1 or NC=2
       */
      alr_data->nwctrl_meas_active = TRUE;
      break;
    case NC_REPORT_MEAS:
      /*
       * Calculate rx_lev average and send the report to RR. RR forwards this to GRR
       */
      if(alr_data->nwctrl_meas_active)
      {
        PALLOC(mph_meas_rep_cnf, MPH_MEAS_REP_CNF);
        memset(mph_meas_rep_cnf,0,sizeof(T_MPH_MEAS_REP_CNF));
        nc_build_nwctrl_rr_report(mph_meas_rep_cnf);
        PSENDX ( RR, mph_meas_rep_cnf);
        for(i=0;i<=BA_LIST_SIZE;i++)
        {
          alr_data->nc_data.cell[i].nc_rxlev = 0;
          alr_data->nc_data.cell[i].c_nc_rxlev = 0;
        }
      }
      break;
    case NC_STOP_MEAS:
      /*
       * De-activate rx_lev averaging for Network control
       */
      alr_data->nwctrl_meas_active = FALSE;
      break;
    default:
      break;
  }
  PFREE(mph_meas_rep_req);
}
#endif


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_random_access_req   |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_RANDOM_ACCESS_REQ.

*/

GLOBAL void ma_mph_random_access_req (T_MPH_RANDOM_ACCESS_REQ* mph_random_access_req)
{
  GET_INSTANCE_DATA;
  switch (GET_STATE (STATE_MA))
  {
    case MA_IDLE:
      ALR_TRACE_MAIN_RAND_ACC(alr_data->serving_cell, mph_random_access_req->send_mode.no);

      /*
       * suspend neighbour cell procedures
       */
      ma_stop_active_procs(DONT_STOP_PCH_READING | STOP_MEASUREMENTS);
      SET_STATE (STATE_MA, MA_CON_EST);
      /*
       * switch to full downlink CCCH listening
       */
      pch_save_pgm(0);
      pch_configure(NULL, PGM_REORG_CS);
      pch_start_ccch_req ();
      /*
       * Send uplink Random Bursts
       */
      rach_random_req (mph_random_access_req);
      break;
#ifdef GPRS
    case MA_PTM:
      ma_stop_active_procs(DONT_STOP_PCH_READING| STOP_MEASUREMENTS);
      // nc_suspend(); gprs_alr_nc_leave_ptm();
      SET_STATE (STATE_MA, MA_CON_EST);
      /*
       * switch to full downlink CCCH listening
       */
      /*pch_save_pgm(0);*/
      pch_configure(NULL, PGM_REORG_CS);
      pch_start_ccch_req ();
      /*
       * Send uplink Random Bursts
       */
      rach_random_req (mph_random_access_req);
      break;
#endif
      case MA_CON_EST:
      SET_STATE (STATE_MA, MA_CON_EST);
      rach_random_req (mph_random_access_req);
      break;

    default:
      break;
  }

  PFREE (mph_random_access_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_mph_emo_req         |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_MPH_EMO_REQ.

*/

GLOBAL void ma_mph_emo_req (T_MPH_EMO_REQ *mph_emo_req)
{
  GET_INSTANCE_DATA;
  switch (GET_STATE (STATE_MA))
  {
    case MA_DEDICATED:
    {
      UBYTE   i,n;
      USHORT *radio_freq;
      USHORT *arfcn;
      USHORT  sc = alr_data->serving_cell;
      BOOL    sc_ok;
      PALLOC (mphc_update_ba_list, MPHC_UPDATE_BA_LIST);

      alr_data->dedi_data.emo_ba_id = mph_emo_req->ba_id;

      n = mph_emo_req->c_arfcn;
      if ( n >= BA_LIST_SIZE )
        n = BA_LIST_SIZE;

      /*
       * Initialize the outgoing primitive MPHC_UPDATE_BA_LIST.
       */

      radio_freq = &mphc_update_ba_list->chan_list.radio_freq[0];
      arfcn      = &mph_emo_req->arfcn[0];
      sc_ok      = FALSE;

      for ( i = 0; i < n; i++ )
      {
        radio_freq[i] = ARFCN_TO_L1 (arfcn[i]);
        if ( radio_freq[i] EQ sc )
        {
          sc_ok = TRUE;
        }
      }
      if ( !sc_ok AND n < BA_LIST_SIZE )  /* add the serving cell ARFCN */
      {
        radio_freq[n++] = sc;
      }
      mphc_update_ba_list->num_of_chans = n;
      mphc_update_ba_list->pwrc         = alr_data->nc_data.pwrc;
      mphc_update_ba_list->dtx_allowed  = alr_data->nc_data.dtx;
      mphc_update_ba_list->ba_id        = mph_emo_req->ba_id;

      PSENDX (L1, mphc_update_ba_list);

      break;
    }
  }
  PFREE (mph_emo_req);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_mph_emo_req         |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_MPH_EMO_REQ.

*/

GLOBAL void ma_mph_ncell_pos_req (T_MPH_NCELL_POS_REQ *mph_ncell_pos_req)
{
  nc_ncell_pos_req(mph_ncell_pos_req);
}
/* ------------------------------------------------------------------
 * ------------------------------------------------------------------
 * Functions for the lower layer(MPHC) primitive processing
 * ------------------------------------------------------------------
 * ------------------------------------------------------------------
 */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mphc_rxlev_con          |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_RXLEV_CON.

*/

GLOBAL void ma_mphc_rxlev_ind (T_MPHC_RXLEV_IND* rxlev_ind)
{
  GET_INSTANCE_DATA;

  USHORT cs_state = GET_STATE (STATE_CS);
  switch (cs_state)
  {
    default:
      TRACE_EVENT_P1 ("wrong CS state (%u) in ma_mphc_rxlev_ind()", cs_state);
      /*lint -fallthrough*/
    case CS_ACTIVE_MEASURE:
      SET_STATE(STATE_CS, CS_MEASURED);
      break;
  }

  ALR_EM_FIELDSTRENGTH_MEASUREMENT_CONFIRM;
  cs_rxlev_ind (rxlev_ind);
  PFREE (rxlev_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mphc_stop_rxlev_con          |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_RXLEV_CON.

*/

GLOBAL void ma_mphc_stop_rxlev_cnf (T_MPHC_STOP_RXLEV_CON * rxlev_cnf)
{
  GET_INSTANCE_DATA;
  if ( IS_EXT_MEAS_RUNNING )
  {
     nc_stop_ext_meas_ind();
     alr_data->cs_data.ext_meas_state_pend = CS_NULL;
  }
  PFREE(rxlev_cnf);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_stop_cnf                |
+--------------------------------------------------------------------+

  PURPOSE : Processes the primitives MPHC_STOP_*_CON.

*/

GLOBAL void ma_mphc_empty_cnf (void *cnf)
{
  PFREE (cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mphc_network_sync_ind   |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_NETWORK_SYNC_IND.

*/

GLOBAL void ma_mphc_network_sync_ind (T_MPHC_NETWORK_SYNC_IND* sync_ind)
{
  TIMERSTOP(TIM_NW_SYNC_GUARD);
  cs_network_sync_ind (sync_ind);
  PFREE(sync_ind);
}

#if defined(STOP_SYNC_TASK)
/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                           |
| STATE   : code                ROUTINE : ma_mphc_stop_network_sync_req      |
+----------------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_STOP_NETWORK_SYNC_REQ.

*/

GLOBAL void ma_cs_stop_network_sync_req(void)
{
  GET_INSTANCE_DATA;
  if (alr_data->cs_data.sync_active)
  {
    PALLOC (stop_sync_task, MPHC_STOP_NETWORK_SYNC_REQ); /* T_MPHC_STOP_NETWORK_SYNC_REQ */
    TRACE_EVENT ("STOP_NETWORK_SYNC_REQ");
    PSENDX(L1, stop_sync_task);
  }
}

/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                           |
| STATE   : code                ROUTINE : ma_mphc_stop_network_sync_cnf      |
+----------------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_STOP_NETWORK_SYNC_CON.

*/

GLOBAL void ma_mphc_stop_network_sync_cnf(T_MPHC_STOP_NETWORK_SYNC_CON   *stop_network_sync_cnf)
{
  GET_INSTANCE_DATA;
  alr_data->cs_data.sync_active = FALSE;
  switch(GET_STATE(STATE_CS))
  {
    case CS_STOP_SYNC:
      SET_STATE (STATE_CS, CS_STOP_SYNC_DONE);
      cs_bsic_req(NULL);
      break;

    case CS_NW_SYNC_TIMEOUT:
      /* Inform RR that no BCCH was found */
      SET_STATE (STATE_CS, CS_ACTIVE_SYNC);

      {
        PALLOC (mph_bsic_cnf, MPH_BSIC_CNF);
        mph_bsic_cnf->cs = CS_NO_BCCH_AVAIL;
        PSENDX (RR, mph_bsic_cnf);
      }
      break;

    default:
      break;
  }

  PFREE(stop_network_sync_cnf);
}
#endif /* STOP_SYNC_TASK */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mphc_new_scell_cnf      |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_NEW_SCELL_CON.

*/

GLOBAL void ma_mphc_new_scell_cnf (T_MPHC_NEW_SCELL_CON* new_scell_cnf)
{
  GET_INSTANCE_DATA;
  /* we have a new serving cell */
  vsi_t_sleep(VSI_CALLER 10);

  switch (GET_STATE(STATE_MA))
  {
    case MA_CELL_SELECTION:
      /*
       * we are synchronized to the new cell
       * now read the BCCH to check if the cell
       * is suitable
       */
      cs_read_scell_bcch ();
      break;
    case MA_CELL_RESELECTION:
      ALR_TRACE_MAIN ("new sc in cr");
      if (alr_data->old_serving_cell NEQ alr_data->serving_cell)
      {
        ALR_TRACE_MAIN_SCELL(alr_data->old_serving_cell, alr_data->serving_cell);
        ALR_TRACE_MAIN ("update timebase");
          /*
           * update the timebase now
           */
        nc_update_list (alr_data->serving_cell);
        /*
         * removing of the old serving cell only just if
         * RR gives a positive receipt for cell selection.
         * obsolet: nc_update_serving_cell (alr_data->old_serving_cell);
         */

        /*
         * In case the serving cell has been changed, the last CCCH configuration
         * is not valid anymore! It will delete now, so that the sucessful
         * comparison inside ma_pch_start_ccch_req() will not prevent a new
         * CCCH configuration of L1.
         */
        memset (&alr_data->pch_data.last_start_ccch_req, 0,
          sizeof (alr_data->pch_data.last_start_ccch_req));
      }
#ifdef GPRS
      if(alr_data->gprs_data.sync_only)
      {
        PALLOC(sync, MPH_SYNC_IND);
        sync->cs =  CS_SCELL_IS_SYNCED;
        alr_data->gprs_data.sync_only=0;
        SET_STATE(STATE_MA, MA_IDLE);
        PSENDX (RR, sync);
      }
      else
      {
#endif
        pch_start_ccch_req();
#ifdef GPRS
      }
#endif
      break;
    case MA_IDLE:
      ALR_TRACE_MAIN ("new sc in idle");
      if (alr_data->old_serving_cell NEQ alr_data->serving_cell)
        nc_update_list (alr_data->serving_cell);
      break;
    case MA_CON_EST:
      break;
    default:
      break;
  }

  PFREE (new_scell_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mphc_ncell_bcch_ind     |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_NCELL_BCCH_IND.

*/

GLOBAL void ma_mphc_ncell_bcch_ind (T_MPHC_NCELL_BCCH_IND* ncell_bcch_ind)
{
  nc_bcch_ind (ncell_bcch_ind);
  PFREE (ncell_bcch_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mphc_ncell_sync_ind     |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_NCELL_SYNC_IND.

*/

GLOBAL void ma_mphc_ncell_sync_ind (T_MPHC_NCELL_SYNC_IND* ncell_sync_ind)
{
  nc_sync_ind (ncell_sync_ind);
  PFREE (ncell_sync_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mphc_stop_ncell_sync_cnf     |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_NCELL_SYNC_IND.

*/

GLOBAL void ma_mphc_stop_ncell_sync_cnf (T_MPHC_STOP_NCELL_SYNC_CON *cnf )
{
  GET_INSTANCE_DATA;
  if ( IS_EXT_MEAS_RUNNING )
  {
     nc_stop_ext_meas_ind();
     alr_data->cs_data.ext_meas_state_pend = CS_NULL;
  }
  PFREE (cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mphc_ncell_bcch_ind     |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_RXLEV_PERIODIC_IND.

*/

GLOBAL void ma_mphc_rxlev_periodic_ind (T_MPHC_RXLEV_PERIODIC_IND*
                                        rxlev_periodic_ind)
{
  GET_INSTANCE_DATA;

  switch (GET_STATE (STATE_MA))
  {
    case MA_CELL_RESELECTION:
    case MA_CON_EST:
    case MA_IDLE:
      nc_report (rxlev_periodic_ind);
#ifdef TI_PS_HCOMM_CHANGE
      PSIGNAL (_hCommDL, PH_TRACE_IND, NULL);
#else
      PSIGNAL (hCommDL, PH_TRACE_IND, NULL);
#endif /* TI_PS_HCOMM_CHANGE */
      alr_trc_read (8);
      break;

    default:
      break;
  }
  PFREE (rxlev_periodic_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mphc_ra_cnf             |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_RA_CON.

*/

GLOBAL void ma_mphc_ra_cnf (T_MPHC_RA_CON* ra_cnf)
{
  GET_INSTANCE_DATA;

  switch (GET_STATE (STATE_MA))
  {
    case MA_CON_EST:
      rach_ra_cnf (ra_cnf);
      break;
    default:
      break;
  }
  PFREE (ra_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_trc_data_ind            |
+--------------------------------------------------------------------+

  PURPOSE : Trace the primitive MPHC_DATA_IND.

  configure the output with the following command:
  CONFIG TRC_DATA_IND=n

  the available decimal values of the bit mask 'n' are:
  ID_TRC_DATA_IND_ALL     255
  ID_TRC_DATA_IND_VALID   127
  ID_TRC_DATA_IND_INVALID 128
  ID_TRC_DATA_IND_BCCH      1  normal and extended BCCH
  ID_TRC_DATA_IND_PCH       2  normal and extended PCH
  ID_TRC_DATA_IND_CCCH      4
  ID_TRC_DATA_IND_CBCH      8
  ID_TRC_DATA_IND_SCCH     16  SACCH and SDCCH
  ID_TRC_DATA_IND_FCCH     32  full and half rate FACCH
  ID_TRC_DATA_IND_STATE    64  MA status

  ID_TRC_DATA_IND_INIT     63  all valid frames without stati
  ID_TRC_DATA_IND_INIT2    31  ... without FACCH
*/
#if !defined(NCONFIG)
static void ma_trc_data_ind(T_MPHC_DATA_IND* data_ind)
{
  GET_INSTANCE_DATA;
  char *msg_s = "";
  UBYTE msg_t = 0;

  if (data_ind->error_flag NEQ VALID_BLOCK)
  {
    if (v_mon_trc_data_ind & ID_TRC_DATA_IND_INVALID)
    {
#if !defined(DL_TRACE_ENABLED)
      SYST_TRACE ("invalid block");
#else  /* !DL_TRACE_ENABLED */
      DL_OFFLINE_TRACE ("invalid block");
#endif  /* !DL_TRACE_ENABLED */
    }
    return;
  }

  switch (data_ind->l2_channel)
  {
    case L2_CHANNEL_SACCH:
    case L2_CHANNEL_SDCCH:
      if ((v_mon_trc_data_ind & ID_TRC_DATA_IND_SCCH) EQ 0)
        msg_t = (UBYTE)-1;
      break;
    case L2_CHANNEL_FACCH_H:
    case L2_CHANNEL_FACCH_F:
      if ((v_mon_trc_data_ind & ID_TRC_DATA_IND_FCCH) EQ 0)
        msg_t = (UBYTE)-1;
      break;
    case L2_CHANNEL_CCCH:
      if ((v_mon_trc_data_ind & ID_TRC_DATA_IND_CCCH) EQ 0)
        return;
      break;
    case L2_CHANNEL_NBCCH:
    case L2_CHANNEL_EBCCH:
      if ((v_mon_trc_data_ind & ID_TRC_DATA_IND_BCCH) EQ 0)
        msg_t = (UBYTE)-1;
      break;
    case L2_CHANNEL_PCH:
    case L2_CHANNEL_EPCH:
      if ((v_mon_trc_data_ind & ID_TRC_DATA_IND_PCH) EQ 0)
        msg_t = (UBYTE)-1;
      break;
    case L2_CHANNEL_CBCH:
      if ((v_mon_trc_data_ind & ID_TRC_DATA_IND_CBCH) EQ 0)
        msg_t = (UBYTE)-1;
      break;
    default:
        msg_t = (UBYTE)-1;
        SYST_TRACE_P((SYST, "trc_data_ind %02x ch=%d",
          v_mon_trc_data_ind, data_ind->l2_channel));
      break;
  }

  if (msg_t)
  {
    return;
  }

  if (data_ind->error_flag EQ VALID_BLOCK)
  {
    msg_t = data_ind->l2_frame.content[SI_CONTENTS_MSG_T];
    switch (msg_t)
    {
      case D_IMM_ASSIGN:
        msg_s = "IMM_ASS";
        break;
      case D_IMM_ASSIGN_EXT:
        msg_s = "IMM_ASS_EXT";
        break;
      case D_IMM_ASSIGN_REJ:
        msg_s = "IMM_ASS_REJ";
        break;
      case D_PAG_REQ_1:
        msg_s = "PAG1";
        break;
      case D_PAG_REQ_2:
        msg_s = "PAG2";
        break;
      case D_PAG_REQ_3:
        msg_s = "PAG3";
        break;
      case D_SYS_INFO_1:
        msg_s = "SI 1";
        break;
      case D_SYS_INFO_2:
        msg_s = "SI 2";
        break;
      case D_SYS_INFO_2BIS:
        msg_s = "SI 2bis";
        break;
      case D_SYS_INFO_2TER:
        msg_s = "SI 2ter";
        break;
#if defined (REL99) && defined (TI_PS_FF_EMR)
      case D_SYS_INFO_2QUATER:
        msg_s = "SI 2quater";
        break;
#endif
      case D_SYS_INFO_3:
        msg_s = "SI 3";
        /*
        TRACE_EVENT_P8 ("SI3: lai=%x%x%x %x%x%x %02x%02x",
          data_ind->l2_frame.content[5]&0x0f,
          data_ind->l2_frame.content[5]>>4,
          data_ind->l2_frame.content[6]&0x0f,

          data_ind->l2_frame.content[7]&0x0f,
          data_ind->l2_frame.content[7]>>4,
          data_ind->l2_frame.content[6]>>4,

          data_ind->l2_frame.content[8],
          data_ind->l2_frame.content[9]);
        */
        break;
      case D_SYS_INFO_4:
        msg_s = "SI 4";
        /*
        TRACE_EVENT_P8 ("SI4: lai=%x%x%x %x%x%x %02x%02x",
          data_ind->l2_frame.content[3]&0x0f,
          data_ind->l2_frame.content[3]>>4,
          data_ind->l2_frame.content[4]&0x0f,

          data_ind->l2_frame.content[5]&0x0f,
          data_ind->l2_frame.content[5]>>4,
          data_ind->l2_frame.content[4]>>4,

          data_ind->l2_frame.content[6],
          data_ind->l2_frame.content[7]);
        */
        break;
      case D_SYS_INFO_5:
        msg_s = "SI 5";
        break;
      case D_SYS_INFO_5BIS:
        msg_s = "SI 5bis";
        break;
      case D_SYS_INFO_5TER:
        msg_s = "SI 5ter";
        break;
      case D_SYS_INFO_6:
        msg_s = "SI 6";
        break;
#ifdef GPRS
      case D_SYS_INFO_13:
        msg_s = "SI 13";
        break;
#endif /* GPRS */
      default:
        if (data_ind->l2_channel EQ L2_CHANNEL_CBCH)
          msg_s = "CBCH";
        else
          msg_s = 0;
        break;
    }
  }

#if !defined(DL_TRACE_ENABLED)
  if (msg_s)
  {
    if (v_mon_trc_data_ind & ID_TRC_DATA_IND_STATE)
    {
      SYST_TRACE_P((SYST, "[%u] %s %s",
        ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK,
        msg_s, STATE_MA_NAME[GET_STATE (STATE_MA)]));
    }
    else
    {
      SYST_TRACE_P((SYST, "[%u] %s",
        ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK, msg_s));
    }
  }
  else
  {
    /* protocol discriminator */
    UBYTE pd = data_ind->l2_frame.content[SI_CONTENTS_MSG_T-1];
    char buf[47];

    if (v_mon_trc_data_ind & ID_TRC_DATA_IND_STATE)
    {
      SYST_TRACE_P((SYST, "[%u] unknown %02x (PD%02x, ch=%d) %s",
        ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK, msg_t, pd,
        data_ind->l2_channel, STATE_MA_NAME[GET_STATE (STATE_MA)]));
    }
    else
    {
      SYST_TRACE_P((SYST, "[%u] unknown %02x (PD%02x, ch=%d)",
        ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK,
        msg_t, pd, data_ind->l2_channel));
    }
    for (pd=0; pd<23; pd++)
      sprintf (buf+(pd<<1), "%02x", (UBYTE)data_ind->l2_frame.content[pd]);
    SYST_TRACE(buf);
  }
#else  /* !DL_TRACE_ENABLED */
  {
    if (msg_s)
    {
      if (v_mon_trc_data_ind & ID_TRC_DATA_IND_STATE)
      {
        sprintf (dl_trace_buf, "[%u] %s %s",
          ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK,
          msg_s, STATE_MA_NAME[GET_STATE (STATE_MA)]);
      }
      else
      {
        sprintf (dl_trace_buf, "[%u] %s",
          ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK, msg_s);
      }
    }
    else
    {
      /* protocol discriminator */
      UBYTE pd = data_ind->l2_frame.content[SI_CONTENTS_MSG_T-1];

      sprintf (dl_trace_buf, "[%u] unknown %02x (PD%02x, ch=%d)",
          ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK,
          msg_t, pd, data_ind->l2_channel);
    }
    DL_OFFLINE_TRACE (dl_trace_buf);
  }
#endif  /* !DL_TRACE_ENABLED */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : trc_state_transition		 |
+--------------------------------------------------------------------+

  PURPOSE : Traces state transition

*/


#if !defined(NTRACE) && defined(TRACE_STATE_TRANSITION)
void trc_state_transition(int line, UBYTE newstate)
{
#if defined(DL_TRACE_ENABLED)
  sprintf (dl_trace_buf, "%s->%s (#%u)",
    STATE_MA_NAME[GET_STATE (STATE_MA)], STATE_MA_NAME[newstate], line);
  DL_OFFLINE_TRACE (dl_trace_buf);
#else  /* DL_TRACE_ENABLED */
  SYST_TRACE_P((SYST, "%s->%s (#%u)",
    STATE_MA_NAME[GET_STATE (STATE_MA)], STATE_MA_NAME[newstate], line));
#endif  /* DL_TRACE_ENABLED */
}
#endif  /* !NTRACE && TRACE_STATE_TRANSITION */
#endif  /* !NCONFIG */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : trc_paging                 |
+--------------------------------------------------------------------+

  PURPOSE : Traces state transition

*/


#if !defined(NTRACE) && 0
void trc_paging(T_MPHC_DATA_IND* data_ind)
{
  if ((data_ind->error_flag EQ VALID_BLOCK) AND
      ((data_ind->l2_channel EQ L2_CHANNEL_PCH) OR
       (data_ind->l2_channel EQ L2_CHANNEL_EPCH)))
  {
    char *str;

    switch (data_ind->l2_frame.content[SI_CONTENTS_MSG_T]) // SI_CONTENTS_MSG_T=2
    {
      case D_PAG_REQ_1:
        str="D_PAG_REQ_1";
        break;
      case D_PAG_REQ_2:
        str="D_PAG_REQ_2";
        break;
      case D_PAG_REQ_3:
        str="D_PAG_REQ_3";
        break;
      default:
        str = NULL;
        break;
    }

    if (str)
    {
      /* TRACE_BINDUMP first available with FRAME_2.9.18 */
#ifdef TI_PS_HCOMM_CHANGE
      TRACE_BINDUMP(_hCommPL ,TC_EVENT, str, data_ind->l2_frame.content,
        MAX_L2_FRAME_SIZE);
#else
      TRACE_BINDUMP(hCommPL ,TC_EVENT, str, data_ind->l2_frame.content,
        MAX_L2_FRAME_SIZE);
#endif /* TI_PS_HCOMM_CHANGE */
    }
  }

}
#endif  /* !NTRACE */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mphc_data_ind           |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_DATA_IND.

*/

GLOBAL void ma_mphc_data_ind (T_MPHC_DATA_IND* data_ind)
{
  GET_INSTANCE_DATA;
  USHORT length=0, valid=0;
  UBYTE msg_t;

  alr_trc_read (1);

#if !defined(NTRACE) && 0
  trc_paging(data_ind);
#endif  /* !NTRACE */

#if !defined(NCONFIG)
  if (v_mon_trc_data_ind)
    ma_trc_data_ind(data_ind);

  /* IOT and FT trace */
  if (data_ind->l2_channel NEQ L2_CHANNEL_CBCH AND
      data_ind->l2_channel NEQ L2_CHANNEL_SACCH)
  {
/* Implements Measure#32: Row 90,...,101 */
    if(data_ind->error_flag EQ VALID_BLOCK)
    {
      length = MAX_L2_FRAME_SIZE;
      valid = 1;
    }
    switch (data_ind->l2_frame.content[SI_CONTENTS_MSG_T])
    {
      case D_SYS_INFO_1:
        TRACE_USER_CLASS_P2 (TC_USER4, "[%u] D_SYS_INFO_1 block: Valid=%d",
                  ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK, valid);
        break;
      case D_SYS_INFO_2:
        TRACE_USER_CLASS_P2 (TC_USER4, "[%u] D_SYS_INFO_2 block: Valid=%d",
                  ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK, valid);
        break;
      case D_SYS_INFO_2BIS:
        TRACE_USER_CLASS_P2 (TC_USER4, "[%u] D_SYS_INFO_2BIS block: Valid=%d",
                  ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK, valid);
        break;
      case D_SYS_INFO_2TER:
        TRACE_USER_CLASS_P2 (TC_USER4, "[%u] D_SYS_INFO_2TER block: Valid=%d",
                  ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK, valid);
        break;
#if defined (REL99) && defined (TI_PS_FF_EMR)
      case D_SYS_INFO_2QUATER:
        TRACE_USER_CLASS_P2 (TC_USER4, "[%u] D_SYS_INFO_2QUATER block: Valid=%d",
                  ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK, valid);
        break;
#endif
      case D_SYS_INFO_3:
        TRACE_USER_CLASS_P2 (TC_USER4, "[%u] D_SYS_INFO_3 block: Valid=%d",
                  ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK, valid);
        break;
      case D_SYS_INFO_4:
        TRACE_USER_CLASS_P2 (TC_USER4, "[%u] D_SYS_INFO_4 block: Valid=%d",
                  ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK, valid);
        break;
#ifdef GPRS
      case D_SYS_INFO_13:
        TRACE_USER_CLASS_P2 (TC_USER4, "[%u] D_SYS_INFO_13 block: Valid=%d",
                  ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK, valid);
        break;
#endif
      case D_IMM_ASSIGN:
        TRACE_USER_CLASS_P2 (TC_USER4, "[%u] D_IMM_ASSIGN block: Valid=%d",
                  ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK, valid);
        break;
      case D_IMM_ASSIGN_EXT:
        TRACE_USER_CLASS_P2 (TC_USER4, "[%u] D_IMM_ASSIGN_EXT block: Valid=%d",
                  ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK, valid);
        break;
      case D_IMM_ASSIGN_REJ:
        TRACE_USER_CLASS_P2 (TC_USER4, "[%u] D_IMM_ASSIGN_REJ block: Valid=%d",
                  ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK, valid);
        break;
      case D_PAG_REQ_1:
        TRACE_USER_CLASS_P2 (TC_USER4, "[%u] PAG_REQ_1 DL block: Valid=%d",
                  ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK, valid);
        break;
      case D_PAG_REQ_2:
        TRACE_USER_CLASS_P2 (TC_USER4, "[%u] PAG_REQ_2 DL block: Valid=%d",
                  ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK, valid);
        break;
      case D_PAG_REQ_3:
        TRACE_USER_CLASS_P2 (TC_USER4, "[%u] PAG_REQ_3 DL block: Valid=%d",
                  ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK, valid);
        break;
      default:
        break;
    }
    switch (data_ind->l2_frame.content[SI_CONTENTS_MSG_T])
    {
      case D_SYS_INFO_1:
#ifdef TI_PS_HCOMM_CHANGE
        TRACE_BINDUMP(_hCommPL , TC_USER4, 
                       "D_SYS_INFO_1", 
                       data_ind->l2_frame.content,
                       length);
#else /* not TI_PS_HCOMM_CHANGE */
        TRACE_BINDUMP(hCommPL , TC_USER4, 
                       "D_SYS_INFO_1", 
                       data_ind->l2_frame.content,
                       length);
#endif /* TI_PS_HCOMM_CHANGE */
        break;
      case D_SYS_INFO_2:
#ifdef TI_PS_HCOMM_CHANGE
        TRACE_BINDUMP(_hCommPL , TC_USER4, 
                  "D_SYS_INFO_2", 
                  data_ind->l2_frame.content,
                  length);
#else /* not TI_PS_HCOMM_CHANGE */
        TRACE_BINDUMP(hCommPL , TC_USER4, 
                  "D_SYS_INFO_2", 
                  data_ind->l2_frame.content,
                  length);
#endif /* TI_PS_HCOMM_CHANGE */
        break;
      case D_SYS_INFO_2BIS:
#ifdef TI_PS_HCOMM_CHANGE
        TRACE_BINDUMP(_hCommPL , TC_USER4, 
                  "D_SYS_INFO_2BIS", 
                  data_ind->l2_frame.content,
                  length);
#else /* not TI_PS_HCOMM_CHANGE */
        TRACE_BINDUMP(hCommPL , TC_USER4, 
                  "D_SYS_INFO_2BIS", 
                  data_ind->l2_frame.content,
                  length);
#endif /* TI_PS_HCOMM_CHANGE */
        break;
      case D_SYS_INFO_2TER:
#ifdef TI_PS_HCOMM_CHANGE
        TRACE_BINDUMP(_hCommPL , TC_USER4, 
                  "D_SYS_INFO_2TER", 
                  data_ind->l2_frame.content,
                  length);
#else /* not TI_PS_HCOMM_CHANGE */
        TRACE_BINDUMP(hCommPL , TC_USER4, 
                  "D_SYS_INFO_2TER", 
                  data_ind->l2_frame.content,
                  length);
#endif /* TI_PS_HCOMM_CHANGE */
        break;
#if defined (REL99) && defined (TI_PS_FF_EMR)
      case D_SYS_INFO_2QUATER:
#ifdef TI_PS_HCOMM_CHANGE
        TRACE_BINDUMP(_hCommPL , TC_USER4, 
                  "D_SYS_INFO_2QUATER", 
                  data_ind->l2_frame.content,
                  length);
#else /* not TI_PS_HCOMM_CHANGE */
        TRACE_BINDUMP(hCommPL , TC_USER4, 
                  "D_SYS_INFO_2QUATER", 
                  data_ind->l2_frame.content,
                  length);
#endif /* TI_PS_HCOMM_CHANGE */
        break;
#endif
      case D_SYS_INFO_3:
#ifdef TI_PS_HCOMM_CHANGE
        TRACE_BINDUMP(_hCommPL , TC_USER4, 
                  "D_SYS_INFO_3", 
                  data_ind->l2_frame.content,
                  length);
#else /* not TI_PS_HCOMM_CHANGE */
        TRACE_BINDUMP(hCommPL , TC_USER4, 
                  "D_SYS_INFO_3", 
                  data_ind->l2_frame.content,
                  length);
#endif /* TI_PS_HCOMM_CHANGE */
        break;
      case D_SYS_INFO_4:
#ifdef TI_PS_HCOMM_CHANGE
        TRACE_BINDUMP(_hCommPL , TC_USER4, 
                  "D_SYS_INFO_4", 
                  data_ind->l2_frame.content,
                  length);
#else /* not TI_PS_HCOMM_CHANGE */
        TRACE_BINDUMP(hCommPL , TC_USER4, 
                  "D_SYS_INFO_4", 
                  data_ind->l2_frame.content,
                  length);
#endif /* TI_PS_HCOMM_CHANGE */
        break;
#ifdef GPRS
      case D_SYS_INFO_13:
#ifdef TI_PS_HCOMM_CHANGE
        TRACE_BINDUMP(_hCommPL , TC_USER4, 
                  "D_SYS_INFO_13", 
                  data_ind->l2_frame.content,
                  length);
#else /* not TI_PS_HCOMM_CHANGE */
        TRACE_BINDUMP(hCommPL , TC_USER4, 
                  "D_SYS_INFO_13", 
                  data_ind->l2_frame.content,
                  length);
#endif /* TI_PS_HCOMM_CHANGE */
        break;
#endif
      case D_IMM_ASSIGN:
#ifdef TI_PS_HCOMM_CHANGE
        TRACE_BINDUMP(_hCommPL , TC_USER4, 
                  "D_IMM_ASSIGN", 
                  data_ind->l2_frame.content,
                  length);
#else /* not TI_PS_HCOMM_CHANGE */
        TRACE_BINDUMP(hCommPL , TC_USER4, 
                  "D_IMM_ASSIGN", 
                  data_ind->l2_frame.content,
                  length);
#endif /* TI_PS_HCOMM_CHANGE */
        break;
      case D_IMM_ASSIGN_EXT:
#ifdef TI_PS_HCOMM_CHANGE
        TRACE_BINDUMP(_hCommPL , TC_USER4, 
                  "D_IMM_ASSIGN_EXT", 
                  data_ind->l2_frame.content,
                  length);
#else /* not TI_PS_HCOMM_CHANGE */
        TRACE_BINDUMP(hCommPL , TC_USER4, 
                  "D_IMM_ASSIGN_EXT", 
                  data_ind->l2_frame.content,
                  length);
#endif /* TI_PS_HCOMM_CHANGE */
        break;
      case D_IMM_ASSIGN_REJ:
#ifdef TI_PS_HCOMM_CHANGE
        TRACE_BINDUMP(_hCommPL , TC_USER4, 
                  "D_IMM_ASSIGN_REJ", 
                  data_ind->l2_frame.content,
                  length);
#else /* not TI_PS_HCOMM_CHANGE */
        TRACE_BINDUMP(hCommPL , TC_USER4, 
                  "D_IMM_ASSIGN_REJ", 
                  data_ind->l2_frame.content,
                  length);
#endif /* TI_PS_HCOMM_CHANGE */
        break;
      case D_PAG_REQ_1:
#ifdef TI_PS_HCOMM_CHANGE
        TRACE_BINDUMP(_hCommPL , TC_USER4, 
                  "PAG_REQ_1 DL", 
                  data_ind->l2_frame.content,
                  length);
#else /* not TI_PS_HCOMM_CHANGE */
        TRACE_BINDUMP(hCommPL , TC_USER4, 
                  "PAG_REQ_1 DL", 
                  data_ind->l2_frame.content,
                  length);
#endif /* TI_PS_HCOMM_CHANGE */
        break;
      case D_PAG_REQ_2:
#ifdef TI_PS_HCOMM_CHANGE
        TRACE_BINDUMP(_hCommPL , TC_USER4, 
                  "PAG_REQ_2 DL", 
                  data_ind->l2_frame.content,
                  length);
#else /* not TI_PS_HCOMM_CHANGE */
        TRACE_BINDUMP(hCommPL , TC_USER4, 
                  "PAG_REQ_2 DL", 
                  data_ind->l2_frame.content,
                  length);
#endif /* TI_PS_HCOMM_CHANGE */
        break;
      case D_PAG_REQ_3:
#ifdef TI_PS_HCOMM_CHANGE
        TRACE_BINDUMP(_hCommPL , TC_USER4, 
                  "PAG_REQ_3 DL", 
                  data_ind->l2_frame.content,
                  length);
#else /* not TI_PS_HCOMM_CHANGE */
        TRACE_BINDUMP(hCommPL , TC_USER4, 
                  "PAG_REQ_3 DL", 
                  data_ind->l2_frame.content,
                  length);
#endif /* TI_PS_HCOMM_CHANGE */
        break;
      default:
        break;
    }
  }
#endif  /* !NCONFIG */


  if (data_ind->l2_channel EQ L2_CHANNEL_CBCH)
  {
    cb_data_ind (data_ind);
  }
  else
  {
    if (data_ind->error_flag EQ VALID_BLOCK AND
        data_ind->l2_channel EQ L2_CHANNEL_SACCH)
    {
      /*
       * store Timing Advance contained in every decodable downlink SACCH frame
       */
      nc_store_tav((USHORT)(data_ind->l2_frame.content[1] & 63));
    }

    msg_t = data_ind->l2_frame.content[SI_CONTENTS_MSG_T];
    switch (GET_STATE (STATE_MA))
    {
      case MA_NULL:
        /*
         * no message expected
         */
        break;
      case MA_CELL_SELECTION:
        if (data_ind->error_flag EQ VALID_BLOCK)
        {
          ALR_TRACE_MAIN_VALID_BLOCK(msg_t);

          switch (msg_t)
          {
            case D_SYS_INFO_1:
            case D_SYS_INFO_2:
            case D_SYS_INFO_2BIS:
            case D_SYS_INFO_2TER:
#if defined (REL99) && defined (TI_PS_FF_EMR)
            case D_SYS_INFO_2QUATER:
#endif
            case D_SYS_INFO_3:
            case D_SYS_INFO_4:
#ifdef GPRS
            case D_SYS_INFO_13:
#endif
              switch(data_ind->l2_channel)
              {
                case L2_CHANNEL_NBCCH:
                  if (data_ind->tc EQ 0 AND msg_t NEQ D_SYS_INFO_1)
                    ma_sync_ind (CS_SYS_INFO_1_NOT_NEEDED,
                                 ARFCN_TO_G23(data_ind->radio_freq));
                  /*lint -fallthrough*/
                case L2_CHANNEL_EBCCH:
                  ma_send_unitdata (data_ind);
                  ma_fill_sys_buffer (data_ind, msg_t);

                  ma_set_si_bitmap(msg_t);

                  ALR_EM_IDLE_MODE_BCCH_PARAMETER_CHANGED;
                  cs_increment_bfc();

                  break;
                default:
                  ALR_TRACE_MAIN ("stray SI");
                  break;
              }
              break;
            default:
              break;
          }
        }
        else
        {
          ALR_TRACE_MAIN ("INVALID BLOCK");
          if (data_ind->l2_channel EQ L2_CHANNEL_NBCCH OR
              data_ind->l2_channel EQ L2_CHANNEL_EBCCH)
          {
            ma_error_ind (CS_BCCH_READ_ERROR,
                          ARFCN_TO_G23(data_ind->radio_freq));
            cs_decrement_bfc();
          }
        }
        break;
      case MA_IDLE:
        if (data_ind->error_flag EQ VALID_BLOCK)
        {
          switch (msg_t)
          {
            case D_SYS_INFO_2:
              /*
               * setting ncc permitted directly after receive sys info 2
               */
              nc_check_new_ncc_permitted (data_ind->l2_frame.content[SI_CONTENTS_MSG_T+17]);
              /*lint -fallthrough*/
            case D_SYS_INFO_1:
            case D_SYS_INFO_2BIS:
            case D_SYS_INFO_2TER:
            case D_SYS_INFO_3:
            case D_SYS_INFO_4:
              if(data_ind->l2_channel NEQ L2_CHANNEL_NBCCH)
              {
                 ALR_TRACE_MAIN ("stray SI");
              }

              if (data_ind->tc EQ 0 AND msg_t NEQ D_SYS_INFO_1)
              {
                ma_sync_ind (CS_SYS_INFO_1_NOT_NEEDED,
                             ARFCN_TO_G23(data_ind->radio_freq));
                alr_data->pch_data.si_bitmap |= SYSTEM_INFO_1_READ;
              }

              if( alr_data->pch_data.reorg_bcch_reading EQ TRUE)
              {
                if(msg_t EQ D_SYS_INFO_1)
                  alr_data->pch_data.si_bitmap |= SYSTEM_INFO_1_READ;
                else if(msg_t EQ D_SYS_INFO_2)
                  alr_data->pch_data.si_bitmap |= SYSTEM_INFO_2_READ;
                else if(msg_t EQ D_SYS_INFO_3)
                  alr_data->pch_data.si_bitmap |= SYSTEM_INFO_3_READ;
                else if(msg_t EQ D_SYS_INFO_4)
                  alr_data->pch_data.si_bitmap |= SYSTEM_INFO_4_READ;

                ma_set_si_bitmap(msg_t);

#ifdef GPRS
                if( !gprs_alr_is_supported() )
#endif
                {
                  if( (alr_data->pch_data.si_bitmap & ALL_SI_READ_EXCEPT_SI13)
                      EQ  ALL_SI_READ_EXCEPT_SI13)
                  {
                    alr_data->pch_data.reorg_bcch_reading = FALSE;
                    if((alr_data->pch_data.pl_idle.page_mode EQ PGM_NORMAL) OR
                        (alr_data->pch_data.pl_idle.page_mode EQ PGM_EXTENDED))
                    {
                      sc_start_periodic();
                    }
                  }
                }
              }
              if (!ma_compare_sys_buffer (data_ind, msg_t))
              {
                ALR_TRACE_MAIN_SI_UNEQ(data_ind->l2_channel);

                ma_send_unitdata (data_ind);
                ma_fill_sys_buffer (data_ind, msg_t);

                ALR_EM_IDLE_MODE_BCCH_PARAMETER_CHANGED;

              }
#if defined (REL99) && defined (TI_PS_FF_EMR)
              if ( (msg_t EQ D_SYS_INFO_2) OR (msg_t EQ D_SYS_INFO_2BIS) OR (msg_t EQ D_SYS_INFO_2TER) )
              {
                if ( (alr_data->nc_data.si2_count NEQ NOT_PRESENT_8BIT ) AND
                     (alr_data->nc_data.si2_count NEQ 0) )
                  alr_data->nc_data.si2_count--;
                /*Required SI-2 variants have been acquired, now we will configure
                  L1 for SI-2quater if it's required to be configured*/
                if ((alr_data->nc_data.si2quater_status NEQ SI2QUATER_ABSENT ) AND
                  (alr_data->nc_data.si2quater_pos EQ SI2QUATER_ON_EBCCH) AND
                  (alr_data->nc_data.si2_count EQ 0))
                {
                  ma_scell_mon_si2quater_ebcch();
                  alr_data->nc_data.si2_count = NOT_PRESENT_8BIT;
                }
              }
              break;
            case D_SYS_INFO_2QUATER:
              ALR_TRACE_MAIN_SI_UNEQ(data_ind->l2_channel);
              ma_send_unitdata (data_ind);
#endif
              break;
#ifdef GPRS
            case D_SYS_INFO_13:
              if(gprs_alr_is_supported())
              {
                alr_data->pch_data.si_bitmap |= SYSTEM_INFO_13_READ;
                alr_data->pch_data.reorg_bcch_reading = FALSE;
              }
              if(! pch_mode_reorg() AND gprs_alr_is_supported())
              {
                /* stop scell BCCH */
                ma_stop_scell_bcch_req();
              }
              ma_send_unitdata (data_ind);
              ma_fill_sys_buffer (data_ind, msg_t);

              ALR_EM_IDLE_MODE_BCCH_PARAMETER_CHANGED;

              ALR_TRACE_MAIN ("SI13");
              break;
#endif  /* GPRS */

            case D_PAG_REQ_1:
              ALR_TRACE_MAIN("PG1");

              pch_check_pag_1 (data_ind);

              if (data_ind->l2_channel EQ L2_CHANNEL_PCH)
              {
                /*
                 * check only if it is the own paging group
                 */
#ifdef GPRS
                gprs_check_page_mode(data_ind);
#else
                pch_check_page_mode (data_ind);
#endif  /* GPRS */
                pch_increment_dlt ();
              }
              break;
            case D_PAG_REQ_2:
              ALR_TRACE_MAIN ("PG2");
              if (alr_data->pch_data.pl_idle.page_mode EQ PGM_NORMAL)
              {
                if (data_ind->l2_channel EQ L2_CHANNEL_PCH)
                  pch_check_pag_2 (data_ind);
                else
                {
                  ALR_TRACE_MAIN ("PCH block skipped");
                }
              }
             else
                pch_check_pag_2 (data_ind);
              if (data_ind->l2_channel EQ L2_CHANNEL_PCH)
              {
                /* check only if it is the own paging group */
#ifdef GPRS
                gprs_check_page_mode(data_ind);
#else
                pch_check_page_mode (data_ind);
#endif  /* GPRS */
                pch_increment_dlt ();
              }
              break;
            case D_PAG_REQ_3:
              ALR_TRACE_MAIN ("PG3");

              if (alr_data->pch_data.pl_idle.page_mode EQ PGM_NORMAL)
              {
                if (data_ind->l2_channel EQ L2_CHANNEL_PCH)
                  pch_check_pag_3 (data_ind);
                else
                {
                  ALR_TRACE_MAIN ("PCH block skipped");
                }
              }
              else
                pch_check_pag_3 (data_ind);
              if (data_ind->l2_channel EQ L2_CHANNEL_PCH)
              {
                /* check only if it is the own paging group */
#ifdef GPRS
                gprs_check_page_mode(data_ind);
#else
                pch_check_page_mode (data_ind);
#endif/* GPRS */
                pch_increment_dlt ();
              }
              break;
            case D_IMM_ASSIGN:
#ifdef GPRS
              gprs_alr_check_downlink_assign(data_ind);
              /*lint -fallthrough*/
#endif
            case D_IMM_ASSIGN_EXT:
            case D_IMM_ASSIGN_REJ:
              if (data_ind->l2_channel EQ L2_CHANNEL_PCH)
              {
                /* check only if it is the own paging group */
#ifdef GPRS
                gprs_check_page_mode(data_ind);
#else
                pch_check_page_mode (data_ind);
#endif  /* GPRS */
                pch_increment_dlt ();
              }
              break;
            default:
              break;
          } /* message type */
        }
        else
        {
          /*
           * invalid block
           */
          ALR_TRACE_MAIN ("INVALID");
          if (data_ind->l2_channel EQ L2_CHANNEL_PCH)
            pch_decrement_dlt ();
        }
        break;
#ifdef GPRS
      case MA_PTM:
        if (data_ind->error_flag EQ VALID_BLOCK)
        {
          switch (msg_t)
          {
            case D_SYS_INFO_2:
              /*
               * setting ncc permitted directly after receive sys info 2
               */
              nc_check_new_ncc_permitted (data_ind->l2_frame.content[SI_CONTENTS_MSG_T+17]);
              /*lint -fallthrough*/
            case D_SYS_INFO_1:
            case D_SYS_INFO_2BIS:
            case D_SYS_INFO_2TER:
#if defined (REL99) && defined (TI_PS_FF_EMR)
            case D_SYS_INFO_2QUATER:
#endif
            case D_SYS_INFO_3:
            case D_SYS_INFO_4:
              if(data_ind->l2_channel NEQ L2_CHANNEL_NBCCH)
              {
                 ALR_TRACE_MAIN ("stray SI");
              }
              if (data_ind->tc EQ 0 AND msg_t NEQ D_SYS_INFO_1)
                ma_sync_ind (CS_SYS_INFO_1_NOT_NEEDED,
                             ARFCN_TO_G23(data_ind->radio_freq));
              if (!ma_compare_sys_buffer (data_ind, msg_t))
              {
                ALR_TRACE_MAIN_SI_UNEQ(data_ind->l2_channel);

                ma_send_unitdata (data_ind);
                ma_fill_sys_buffer (data_ind, msg_t);

                ALR_EM_IDLE_MODE_BCCH_PARAMETER_CHANGED;

              }
              break;
            case D_SYS_INFO_13:
                /* stop scell BCCH */
              ma_stop_scell_bcch_req();

              ma_send_unitdata (data_ind);

              ALR_EM_IDLE_MODE_BCCH_PARAMETER_CHANGED;

              ma_fill_sys_buffer (data_ind, msg_t);
              break;
            case D_PAG_REQ_1:
              ALR_TRACE_MAIN("PG1");

              pch_check_pag_1 (data_ind);

              if (data_ind->l2_channel EQ L2_CHANNEL_PCH)
              {
                /*
                 * check only if it is the own paging group
                 */
                //pch_increment_dlt ();
              }
              break;
            case D_PAG_REQ_2:
              pch_check_pag_2 (data_ind);
              //pch_increment_dlt ();

              break;
            case D_PAG_REQ_3:
              pch_check_pag_3 (data_ind);
              //pch_increment_dlt ();
              break;
            default:
              break;
          }
        }
        break;
#endif  /* GPRS */
      case MA_CELL_RESELECTION:
        if (data_ind->error_flag EQ VALID_BLOCK)
        {
          ALR_TRACE_MAIN_CR(msg_t);

          switch (msg_t)
          {
            case D_IMM_ASSIGN:
#ifdef GPRS
              TRACE_EVENT_P1 ("downlink assign %02x",  data_ind->l2_frame.content[3]);
              gprs_alr_check_downlink_assign(data_ind);
              /*lint -fallthrough*/
#endif
            case D_IMM_ASSIGN_EXT:
            case D_IMM_ASSIGN_REJ:
              break;
            case D_PAG_REQ_1:
              ALR_TRACE_MAIN ("PG1");
              pch_check_pag_1 (data_ind);
              if (data_ind->l2_channel EQ L2_CHANNEL_PCH)
              {
                pch_increment_dlt ();
                pch_check_page_mode_cr (data_ind);
              }
              break;
            case D_PAG_REQ_2:
              ALR_TRACE_MAIN ("PG2");
              pch_check_pag_2 (data_ind);
              if (data_ind->l2_channel EQ L2_CHANNEL_PCH)
              {
                pch_increment_dlt ();
                pch_check_page_mode_cr (data_ind);
              }
              break;
            case D_PAG_REQ_3:
              ALR_TRACE_MAIN ("PG3");
              pch_check_pag_3 (data_ind);
              if (data_ind->l2_channel EQ L2_CHANNEL_PCH)
              {
                pch_increment_dlt ();
                pch_check_page_mode_cr (data_ind);
              }
              break;
            case D_SYS_INFO_1:
            case D_SYS_INFO_2:
            case D_SYS_INFO_2BIS:
            case D_SYS_INFO_2TER:
#if defined (REL99) && defined (TI_PS_FF_EMR)
            case D_SYS_INFO_2QUATER:
#endif
            case D_SYS_INFO_3:
            case D_SYS_INFO_4:
#ifdef GPRS
            case D_SYS_INFO_13:
#endif
              ALR_TRACE_MAIN ("cr si");
              if (data_ind->tc EQ 0 AND msg_t NEQ D_SYS_INFO_1)
                ma_sync_ind (CS_SYS_INFO_1_NOT_NEEDED, ARFCN_TO_G23(data_ind->radio_freq));
              ma_send_unitdata (data_ind);
              ma_fill_sys_buffer (data_ind, msg_t);

              ma_set_si_bitmap(msg_t);
              ALR_EM_IDLE_MODE_BCCH_PARAMETER_CHANGED;

              /*
               * When having received all SI in cell reselection state RR will
               * send an MPH_IDLE_REQ and ALR will send an MPHC_START_CCCH_REQ
               * This is not a good timing for FTA 20.19 because L1 is then
               * unable to detect a paging for some blocks. The test device will
               * not send the paging near the SI3. Therefore the
               * MPHC_START_CCCH_REQ is sent here (and may be overwritten be the
               * later when ALR receives the MPH_IDLE_REQ with other parameters).
               */
              if (msg_t EQ D_SYS_INFO_3 AND
                  alr_data->pch_data.imsi[0] AND
                  ARFCN_TO_G23(data_ind->radio_freq) EQ alr_data->serving_cell)
                pch_config_resel(data_ind);
              break;
            default:
              break;
          }
        }
        else /* invalid block */
        {
          if (data_ind->l2_channel EQ L2_CHANNEL_NBCCH OR
              data_ind->l2_channel EQ L2_CHANNEL_EBCCH)
          {
            ALR_TRACE_MAIN ("INVALID BLOCK");
            ma_error_ind (CS_BCCH_READ_ERROR,
                          ARFCN_TO_G23(data_ind->radio_freq));
            cs_decrement_bfc();
          }
        }

        break;
      case MA_CON_EST:
        if (data_ind->error_flag EQ VALID_BLOCK)
        {
          switch (msg_t)
          {
            case D_IMM_ASSIGN:
            case D_IMM_ASSIGN_EXT:
            case D_IMM_ASSIGN_REJ:
              ALR_TRACE_MAIN ("IA");
              ma_send_unitdata (data_ind);
              break;
#ifdef GPRS
          case D_PAG_REQ_1:
              pch_check_pag_1 (data_ind);
              break;
            case D_PAG_REQ_2:
              pch_check_pag_2 (data_ind);
              break;
            case D_PAG_REQ_3:
              pch_check_pag_3 (data_ind);
              break;
#endif  /* GPRS */
            default:
              break;
          }
        }
        break;
      case MA_DEDICATED:
        if (data_ind->error_flag EQ VALID_BLOCK)
        {
          switch (data_ind->l2_channel)
          {
            case L2_CHANNEL_SACCH:
              if ((data_ind->l2_frame.content[3] & 0xEF) EQ 3) /* layer 2 Control field octet */
              {
                /*
                 * check control field of l2 header
                 * UI Frame received
                 */
                UBYTE msg_t = data_ind->l2_frame.content[6]; /* layer 3 Message Type octet */

                switch (msg_t)
                {
                  case D_SYS_INFO_6:
                  /*
                   * setting ncc permitted directly after receive sys info 6
                   */
                  nc_check_new_ncc_permitted (data_ind->l2_frame.content[15]);

                  /*lint -fallthrough*/
                  case D_SYS_INFO_5:
                  case D_SYS_INFO_5BIS:
                  case D_SYS_INFO_5TER:

                    if (!ma_compare_dedi_sys_buffer (data_ind, msg_t))
                    {
                      ma_send_dedi_unitdata (data_ind);
                      ma_fill_dedi_sys_buffer (data_ind, msg_t);
                    }
                    break;

                  case D_EXT_MEAS_ORDER:
                    ma_send_dedi_unitdata (data_ind);
                    break;

                  default:
                    break;
                }
              }
              else
              {
                /*
                 * no unacknowledged frame or short PD header (format type Bter)
                 * must be handled by DL
                 * is forwarded in original primitive format
                 */
                ma_dedi_data_out ((T_PH_DATA_IND *)data_ind);
                data_ind = NULL; /* must not be freed by ALR */
              }
              dedi_increment_rlt ();
              break;
            default:
              break;
          } /*l2_channel*/
        }
        else
        {
          /* invalid block */
          if (data_ind->l2_channel EQ L2_CHANNEL_SACCH)
            dedi_decrement_rlt ();
        }
        break;
      default:
        break;
    }
  }

  if (data_ind)
  {
    PFREE (data_ind);
  }
}


/*
+----------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                     |
| STATE   : code                ROUTINE : ma_mphc_change_frequency_cnf |
+----------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_CHANGE_FREQUENCY_CON.

*/

GLOBAL void ma_mphc_change_frequency_cnf (T_MPHC_CHANGE_FREQUENCY_CON* change_frequency_cnf)
{
  PFREE (change_frequency_cnf);
  dedi_change_freq_cnf ();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mphc_async_ho_cnf       |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_ASYNC_HO_CNF.

*/

GLOBAL void ma_mphc_async_ho_cnf (T_MPHC_ASYNC_HO_CON* async_ho_cnf)
{
  PFREE (async_ho_cnf);
  dedi_ho_cnf(HO_TYPE_ASYNC);
}

/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                    |
| STATE   : code                ROUTINE : ma_mphc_channel_assign_cnf  |
+---------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_CHANNEL_ASSIGN_CON.

*/

GLOBAL void ma_mphc_channel_assign_cnf (T_MPHC_CHANNEL_ASSIGN_CON* channel_assign_cnf)
{
  PFREE (channel_assign_cnf);
#if defined(DL_TRACE_ENABLED)
  DL_OFFLINE_TRACE ("MPHC_CHANNEL_ASSIGN_CON");
#else
  SYST_TRACE ("dedi chan ass con");
#endif  /* DL_TRACE_ENABLED */
  dedi_chan_ass_cnf ();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mphc_handover_fail_cnf  |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_HANDOVER_FAIL_CON.

*/

GLOBAL void ma_mphc_handover_fail_cnf (T_MPHC_HANDOVER_FAIL_CON* handover_fail_cnf)
{
  PFREE (handover_fail_cnf);
  dedi_ho_fail_cnf ();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mphc_immed_assign_cnf   |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_IMMED_ASSIGN_CON.

*/

GLOBAL void ma_mphc_immed_assign_cnf (T_MPHC_IMMED_ASSIGN_CON* immed_assign_cnf)
{
  PFREE (immed_assign_cnf);
#if defined(DL_TRACE_ENABLED)
  DL_OFFLINE_TRACE ("MPHC_IMMED_ASSIGN_CON");
#endif  /* DL_TRACE_ENABLED */
  dedi_imm_ass_cnf ();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mphc_pre_sync_ho_cnf    |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_PRE_SYNC_HO_CON.

*/

GLOBAL void ma_mphc_pre_sync_ho_cnf (T_MPHC_PRE_SYNC_HO_CON* pre_sync_ho_cnf)
{
  PFREE (pre_sync_ho_cnf);
  dedi_ho_cnf(HO_TYPE_PRE_SYNC);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mphc_sync_ho_cnf        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_SYNC_HO_CON.

*/

GLOBAL void ma_mphc_sync_ho_cnf (T_MPHC_SYNC_HO_CON* sync_ho_cnf)
{
  PFREE (sync_ho_cnf);
  dedi_ho_cnf(HO_TYPE_SYNC);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mphc_ta_fail_ind        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_TA_FAIL_IND.

*/

GLOBAL void ma_mphc_ta_fail_ind (T_MPHC_TA_FAIL_IND* ta_fail_ind)
{
  PFREE (ta_fail_ind);
  dedi_ta_fail_ind ();
}

/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                      |
| STATE   : code                ROUTINE : ma_mphc_handover_finished_ind |
+-----------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_HANDOVER_FINISHED.

*/

GLOBAL void ma_mphc_handover_finished_ind (T_MPHC_HANDOVER_FINISHED* ho_finished)
{
  dedi_ho_finished (ho_finished);
  PFREE (ho_finished);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mphc_meas_report_ind_emo|
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_MEAS_REPORT for
            Extended Measurement procedure.

*/

LOCAL BOOL ma_mphc_meas_report_ind_emo (T_MPHC_MEAS_REPORT* mphc_meas_report)
{
  GET_INSTANCE_DATA;
  UBYTE emo_ba_id = alr_data->dedi_data.emo_ba_id;
  UBYTE rep_ba_id = mphc_meas_report->ba_id;

  if ( INRANGE(RR_BA_LOW,rep_ba_id,RR_BA_HIGH) AND
       mphc_meas_report->meas_valid  )
  {
    if ( rep_ba_id EQ emo_ba_id )
    {
      T_ncell_meas    *ncell_meas   = &mphc_meas_report->ncell_meas;
      T_res_list      *res_list     = &ncell_meas->res_list[0];
      T_meas_results  *meas_results;
      USHORT           k,n;
      UBYTE            nmax;

      PALLOC (mph_emo_meas_ind, MPH_EMO_MEAS_IND);

      meas_results = &mph_emo_meas_ind->meas_results[0];

      mph_emo_meas_ind->ba_id = mphc_meas_report->ba_id;
      mph_emo_meas_ind->dtx   = mphc_meas_report->dtx_used;

      nmax = sizeof mph_emo_meas_ind->meas_results /
            sizeof mph_emo_meas_ind->meas_results[0];

      if ( mphc_meas_report->no_of_ncells_meas > nmax )
          mphc_meas_report->no_of_ncells_meas = nmax;

      n = mph_emo_meas_ind->c_meas_results = mphc_meas_report->no_of_ncells_meas;
      for ( k = 0; k < n; k++ )
      {
        SHORT rx_lev = (SHORT)res_list[k].rxlev_acc /
                      (BYTE) res_list[k].rxlev_nbr_meas;

        if ( rx_lev < 0 )
            rx_lev = 0;

        meas_results[k].rx_lev = (UBYTE) rx_lev;
        meas_results[k].arfcn  = ARFCN_TO_G23 ( res_list[k].bcch_freq );
      }

      PSENDX(RR, mph_emo_meas_ind);
    }
    return TRUE; /* primitive handled */
  }

  return FALSE; /* indicate that the primitive has not been handled */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mphc_meas_report_ind    |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_MEAS_REPORT.

*/

GLOBAL void ma_mphc_meas_report_ind (T_MPHC_MEAS_REPORT* report)
{
  GET_INSTANCE_DATA;
  switch (GET_STATE (STATE_MA))
  {
    case MA_DEDICATED:

      if (report->meas_valid)
      {
        if ( ma_mphc_meas_report_ind_emo ( report ) )
        {
          PFREE ( report ); /*report isn't used any more*/
          return;
        }
        nc_report_dedicated (report);
      }
      else
      {
        PALLOC (rr_report, MPH_MEASUREMENT_IND);
        memset (rr_report, 0, sizeof (T_MPH_MEASUREMENT_IND));

#ifdef GPRS
        rr_report->gprs_sync = NORMAL_MEAS_REP;
#endif

        if (alr_data->nc_data.update)
          nc_fill_report_sc_dedi (rr_report, 0);
        ma_nc_report_res (rr_report);
      }
      alr_data->nc_data.update = FALSE;
      break;
    default:
      break;
  }

  PFREE (report);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : mphc_adc_ind               |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive CST_ADC_RESULT.
*/


GLOBAL void ma_mphc_adc_ind (T_MPHC_ADC_IND *adc_results)
{
  /*
   * FreeCalypso note: in the TCS211 version this function forwarded
   * the ADC results to the CST entity, but in the LoCosto version
   * that code has been deleted w/o explanation.  The proper course
   * of action for FreeCalypso remains to be decided.
   */
  PFREE (adc_results);
}

/* -------------------------------------------------------------------
 * -------------------------------------------------------------------
 * SIGNAL Processing functions
 * These are called by the other ALR processes
 * -------------------------------------------------------------------
 * -------------------------------------------------------------------
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_rach_ra_req             |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal rach_ra_req from SDL process
            RACH_Control.

*/

GLOBAL void ma_rach_ra_req (T_MPHC_RA_REQ * mph_ra_req)
{
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  ALR_TRACE_MAIN_RACH (mph_ra_req->txpwr);
#else
  ALR_TRACE_MAIN_RACH (mph_ra_req->powerclass_gsm, mph_ra_req->txpwr);
#endif
#ifdef TI_PS_HCOMM_CHANGE
  TRACE_BINDUMP(_hCommPL,
                TC_USER4,
                "CHANNEL_REQ UL",
                &mph_ra_req->channel_request,
                sizeof(mph_ra_req->channel_request));
#else
  TRACE_BINDUMP(hCommPL,
                TC_USER4,
                "CHANNEL_REQ UL",
                &mph_ra_req->channel_request,
                sizeof(mph_ra_req->channel_request));
#endif /* TI_PS_HCOMM_CHANGE */

  PSENDX (L1, mph_ra_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_cs_power_cnf            |
+--------------------------------------------------------------------+

  PURPOSE : Sends the power measurement results to RR.

*/

GLOBAL void ma_cs_power_cnf (T_MPH_POWER_CNF *mph_power_cnf)
{
  GET_INSTANCE_DATA;
#if 0 || defined(TRACING)
  int i;
  TRACE_EVENT_P1 ("mph_power_cnf: %u freq.", mph_power_cnf->num_of_chan);

  for (i = 0; i < mph_power_cnf->num_of_chan AND i<10; i++)
    TRACE_EVENT_P3("[%u] std=%u rx_lev=%u",
      mph_power_cnf->arfcn[i]&ARFCN_MASK,
      STD_GET_FROM_ARFCN(mph_power_cnf->arfcn[i]),
      mph_power_cnf->rx_lev[i]);
#endif  /* TRACING */

  if ( IS_EXT_MEAS_RUNNING ) /*alr_data->cs_data.mph_ext_meas_req NEQ NULL */
  {
    D_OPC ( mph_power_cnf ) = MPH_EXT_MEAS_CNF;
  }

  PSENDX (RR, mph_power_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_cs_rxlev_req            |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal cs_rxlev_req from SDL process
            Cell_Selection.

*/

GLOBAL void ma_cs_rxlev_req (void)
{
  GET_INSTANCE_DATA;
  T_POWER_MEAS* power_meas = NULL;

  TRACE_FUNCTION ("ma_cs_rxlev_req()");

  switch (GET_STATE (STATE_CS))
  {
    case CS_INIT_L1:
      power_meas = cs_prepare_power_req();
      /* init radio band before measurement */
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
      ma_cs_init_l1_req();
#else
      ma_cs_init_l1_req(alr_data->cs_data.std);
#endif
      SET_STATE(STATE_CS, CS_INIT_ACTIVE);
      return; /* wait of MPHC_INIT_L1_CON */
    case CS_INIT_ACTIVE:
      return; /* wait of MPHC_INIT_L1_CON */
    case CS_INIT_DONE:
      /* go through and wait or start the first resp. next measurement */
    case CS_START_MEASURE:
      /* without or after configuration of radio band */
      if (IS_TIMER_ACTIVE(TIM_POWERMEAS))
      {
        /*
         * use time to spread power measurements also to write out
         * possible available DL offline traces
         */
#ifdef TI_PS_HCOMM_CHANGE
        PSIGNAL (_hCommDL, PH_TRACE_IND, NULL);
#else
        PSIGNAL (hCommDL, PH_TRACE_IND, NULL);
#endif /* TI_PS_HCOMM_CHANGE */
        return; /* do nothing and wait for timer delay */
      }
      break; /* OK start the first resp. next measurement */
    default:
      TRACE_EVENT_P1 ("wrong CS state (%u) in ma_cs_rxlev_req()", GET_STATE (STATE_CS));
      return;
  }

  power_meas = cs_prepare_power_req();
  if (power_meas)
  {
    PALLOC(rxlev_req, MPHC_RXLEV_REQ);
    rxlev_req->shared_ptr = (ULONG) power_meas;
    SET_STATE(STATE_CS, CS_ACTIVE_MEASURE);
    PSENDX(L1, rxlev_req);
    /* start timer new */
    TIMERSTART(TIM_POWERMEAS, alr_data->cs_data.c_tim_meas);

    ALR_EM_FIELDSTRENGTH_MEASUREMENT_REQUEST;

  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_cs_init_l1_req          |
+--------------------------------------------------------------------+

  PURPOSE : set the radio band configuration.
*/

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
GLOBAL void ma_cs_init_l1_req ()
#else
GLOBAL void ma_cs_init_l1_req (UBYTE radio_band_config)
#endif
{
  PALLOC (init_l1_req, MPHC_INIT_L1_REQ); /* T_MPHC_INIT_L1_REQ */
/* No need to configure L1 for different bands. l1 is capable of scanning across different bands */  
#ifndef TI_PS_FF_QUAD_BAND_SUPPORT
  init_l1_req->radio_band_config = radio_band_config;
  TRACE_EVENT_P1 ("radio_band_config=%u", init_l1_req->radio_band_config);
#endif
  PSENDX(L1, init_l1_req);
  cs_set_wideband_sync();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_cs_network_sync_req     |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal cs_network_sync_req from SDL process
            Cell_Selection.
*/

GLOBAL void ma_cs_network_sync_req (USHORT channel)
{
  GET_INSTANCE_DATA;
  PALLOC (network_sync_req, MPHC_NETWORK_SYNC_REQ);

  network_sync_req->radio_freq = channel;
  network_sync_req->fn_offset = 0;
  network_sync_req->time_alignment = 0;
  network_sync_req->timing_validity = TV_INVALID_TIMING_INFO;
  network_sync_req->search_mode = alr_data->cs_data.search_mode;
  alr_data->cs_data.sync_active = TRUE;
  PSENDX(L1, network_sync_req);

  TIMERSTART(TIM_NW_SYNC_GUARD, TIM_NW_SYNC_GUARD_VALUE);

}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_nc_rxlev_periodic_req   |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal nc_rxlev_periodic_req from SDL process
            Idle_Neighbour_Cell.

*/

GLOBAL void ma_nc_rxlev_periodic_req (T_MPHC_RXLEV_PERIODIC_REQ* update)
{
  GET_INSTANCE_DATA;
  switch (GET_STATE (STATE_MA))
  {
    case MA_CELL_RESELECTION:
    case MA_IDLE:
      PSENDX (L1, update);
      break;

    default:
      PFREE (update);
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_nc_stop_ncell_bcch_req  |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal nc_stop_ncell_bcch_req from SDL process
            Idle_Neighbour_Cell.

*/

GLOBAL void ma_nc_stop_ncell_bcch_req (USHORT arfcn)
{
  PALLOC(stop_req, MPHC_STOP_NCELL_BCCH_REQ);

  memset (stop_req, 0, sizeof(T_MPHC_STOP_NCELL_BCCH_REQ));
  stop_req->radio_freq_array_size = 1;
  stop_req->radio_freq_array[0] = ARFCN_TO_L1(arfcn);

  PSENDX(L1, stop_req);

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_nc_stop_ncell_bcch_req  |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal nc_stop_ncell_bcch_req from SDL process
            Idle_Neighbour_Cell.

*/

GLOBAL void ma_nc_stop_ncell_sync_req (USHORT arfcn)
{
  PALLOC (mphc_stop_ncell_sync_req, MPHC_STOP_NCELL_SYNC_REQ);

  mphc_stop_ncell_sync_req->radio_freq_array_size = 1;
  mphc_stop_ncell_sync_req->radio_freq_array[0] = ARFCN_TO_L1(arfcn);

  PSENDX(L1, mphc_stop_ncell_sync_req);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_nc_sync_req             |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal nc_sync_req from SDL process
            Idle_Neighbour_Cell.

*/

GLOBAL void ma_nc_sync_req (T_MPHC_NCELL_SYNC_REQ *sync_req)
{
  PSENDX(L1, sync_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_nc_list_sync_req        |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal nc_list_sync_req from SDL process
            Idle_Neighbour_Cell.

*/

GLOBAL void ma_nc_list_sync_req (T_MPHC_NCELL_LIST_SYNC_REQ *list_sync_req)
{
  PSENDX(L1, list_sync_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_nc_bcch_req             |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal nc_bcch_req from SDL process
            Idle_Neighbour_Cell.

*/

GLOBAL void ma_nc_bcch_req (T_MPHC_NCELL_BCCH_REQ *bcch_req)
{

  ALR_EM_READ_NEIGHBOURCELL_BCCH;

  PSENDX(L1, bcch_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_nc_update_ba_list       |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal nc_update_ba_list from SDL process
            Idle_Neighbour_Cell.

*/

GLOBAL void ma_nc_update_ba_list (T_MPHC_UPDATE_BA_LIST *update_ba_list)
{
  GET_INSTANCE_DATA;
  switch (GET_STATE (STATE_MA))
  {
    case MA_DEDICATED:
      PSENDX (L1, update_ba_list);
      break;

    default:
      PFREE (update_ba_list);
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_nc_report_res           |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal nc_report_res from SDL process
            Idle_Neighbour_Cell.

*/

GLOBAL void ma_nc_report_res (T_MPH_MEASUREMENT_IND *mph_measurement_ind)
{
  GET_INSTANCE_DATA;
  TRACE_ASSERT((GET_STATE(STATE_MA) NEQ MA_NULL) OR
               (GET_STATE(STATE_MA) NEQ MA_CELL_SELECTION));

  switch (GET_STATE (STATE_MA))
  {
    case MA_IDLE:
#ifdef GPRS
    case MA_PTM:
#endif
    case MA_CON_EST:
    case MA_CELL_RESELECTION:
    case MA_DEDICATED:
      PSENDX (RR, mph_measurement_ind);
      break;
    default:
      PFREE (mph_measurement_ind);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_dedi_stop_req           |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal ma_dedi_stop_req from SDL process
            Dedicated_Control.

*/

GLOBAL void ma_dedi_stop_req (void)
{
  PALLOC (stop_dedicated, MPHC_STOP_DEDICATED_REQ);
#if defined(DL_TRACE_ENABLED)
  DL_OFFLINE_TRACE ("MPHC_STOP_DEDICATED_REQ");
#endif  /* DL_TRACE_ENABLED */
  PSENDX (L1, stop_dedicated);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_dedi_cnf                |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal ma_dedi_cnf from SDL process
            Dedicated_Control.

*/

GLOBAL void ma_dedi_cnf (UBYTE cause)
{
  PALLOC (dedicated_cnf, MPH_DEDICATED_CNF);

  dedicated_cnf->dedi_res = cause;
  PSENDX (RR, dedicated_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_dedi_chan_ass_req       |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal ma_dedi_chan_ass_req from SDL process
            Dedi Control.

*/

GLOBAL void ma_dedi_chan_ass_req (T_MPHC_CHANNEL_ASSIGN_REQ *mphc_channel_assign_req)
{
#if defined(DL_TRACE_ENABLED)
  DL_OFFLINE_TRACE ("MPHC_CHANNEL_ASSIGN_REQ");
#else
  SYST_TRACE_P ((SYST, "dedi chan ass req %d",
    mphc_channel_assign_req->channel_desc_1.channel_type));
#endif  /* DL_TRACE_ENABLED */
  PSENDX (L1, mphc_channel_assign_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_dedi_async_ho_req       |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal ma_dedi_async_ho_req from SDL process
            Dedi Control.

*/

GLOBAL void ma_dedi_async_ho_req (T_MPHC_ASYNC_HO_REQ *async_ho_req)
{
  PSENDX (L1, async_ho_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_dedi_sync_ho_req        |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal ma_dedi_sync_ho_req from SDL process
            Dedi Control.

*/

GLOBAL void ma_dedi_sync_ho_req (T_MPHC_SYNC_HO_REQ * sync_ho_req)
{
  PSENDX (L1, sync_ho_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_dedi_pre_sync_ho_req    |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal ma_dedi_pre_sync_ho_req from
            SDL process Dedi Control.

*/

GLOBAL void ma_dedi_pre_sync_ho_req (T_MPHC_PRE_SYNC_HO_REQ *pre_sync_ho_req)
{
  PSENDX (L1, pre_sync_ho_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_dedi_fail_cnf           |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal ma_dedi_fail_cnf from SDL process
            Dedicated_Control.

*/

GLOBAL void ma_dedi_fail_cnf (void)
{
  PALLOC (mph_dedicated_fail_cnf, MPH_DEDICATED_FAIL_CNF);
  PSENDX (RR, mph_dedicated_fail_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_dedi_ho_fail_req        |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal ma_dedi_ho_fail_req from SDL process
            Dedicated_Control.

*/

GLOBAL void ma_dedi_ho_fail_req (void)
{
  PALLOC (handover_fail_req, MPHC_HANDOVER_FAIL_REQ);
  PSENDX (L1, handover_fail_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_dedi_imm_ass_req        |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal ma_dedi_imm_ass_req from SDL process
            Dedicated_Control.

*/

GLOBAL void ma_dedi_imm_ass_req (T_MPHC_IMMED_ASSIGN_REQ *immed_assign_req)
{
#if defined(DL_TRACE_ENABLED)
  DL_OFFLINE_TRACE ("MPHC_IMMED_ASSIGN_REQ");
#else
  SYST_TRACE_P ((SYST, "dedi imm ass req %d",
    immed_assign_req->channel_desc.channel_type));
#endif  /* DL_TRACE_ENABLED */
  PSENDX (L1, immed_assign_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_dedi_change_freq_req    |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal ma_dedi_change_freq_req from
            SDL process Dedicated_Control.

*/

GLOBAL void ma_dedi_change_freq_req (T_MPHC_CHANGE_FREQUENCY *change_frequency)
{
  PSENDX (L1, change_frequency);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_dedi_chan_mode_req      |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal ma_dedi_chan_mode_req from
            SDL process Dedicated_Control.

*/

GLOBAL void ma_dedi_chan_mode_req (T_MPHC_CHANNEL_MODE_MODIFY_REQ *channel_mode_modify_req)
{
  PSENDX (L1, channel_mode_modify_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_dedi_ciph_req           |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal ma_dedi_ciph_req from
            SDL process Dedicated_Control.

*/

GLOBAL void ma_dedi_ciph_req (T_MPHC_SET_CIPHERING_REQ *set_ciphering_req)
{
  PSENDX (L1, set_ciphering_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_dedi_data_out           |
+--------------------------------------------------------------------+

  PURPOSE : Acknowledged frame or short PD header (format type Bter)
            Forward the message to DL. Uses the original primitive format.

*/

LOCAL void ma_dedi_data_out (T_PH_DATA_IND *data_ind)
{
  PPASS (data_ind, ph_data_ind, PH_DATA_IND);
  PSENDX (DL, ph_data_ind);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_send_dedi_unitdata      |
+--------------------------------------------------------------------+

  PURPOSE : Sends an unacknowledged message on SACCH to RR.

*/

LOCAL void ma_send_dedi_unitdata (T_MPHC_DATA_IND *data_ind)
{
  USHORT len_in_bits = 19 * 8;
  PALLOC_SDU (data_out, MPH_UNITDATA_IND, len_in_bits);

  data_out->sdu.l_buf = 18 * 8;
  data_out->sdu.o_buf = 8;
  data_out->sdu.buf[0] = 0;
  /*lint -e{419} (Warning -- Apparent data overrun) */
  memcpy (&data_out->sdu.buf[1], &data_ind->l2_frame.content[5], 18);
  data_out->arfcn = ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK;
  data_out->fn = data_ind->fn;

  PSENDX (RR, data_out);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_dedi_open_tch_loop_req  |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal ma_dedi_open_tch_loop_req from
            SDL process Dedicated_Control.

*/

GLOBAL void ma_dedi_open_tch_loop_req (void)
{
  PALLOC (oml1_open_tch_loop_req, OML1_OPEN_TCH_LOOP_REQ);
  PSENDX (L1, oml1_open_tch_loop_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_dedi_stop_dai_req       |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal ma_dedi_stop_dai_req from
            SDL process Dedicated_Control.

*/
GLOBAL void ma_dedi_stop_dai_req (void)
{
  PALLOC (oml1_stop_dai_test_req, OML1_STOP_DAI_TEST_REQ);
  PSENDX (L1, oml1_stop_dai_test_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_dedi_close_tch_loop_req |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal ma_dedi_close_tch_loop_req from
            SDL process Dedicated_Control.

*/

GLOBAL void ma_dedi_close_tch_loop_req (T_OML1_CLOSE_TCH_LOOP_REQ *oml1_close_tch_loop_req)
{
  PSENDX (L1, oml1_close_tch_loop_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_dedi_start_dai_req      |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal ma_dedi_start_dai_req from
            SDL process Dedicated_Control.

*/

GLOBAL void ma_dedi_start_dai_req (T_OML1_START_DAI_TEST_REQ *oml1_start_dai_test_req)
{
  PSENDX (L1, oml1_start_dai_test_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_rach_stop_ra_req        |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal rach_stop_ra_req from SDL process
            RACH_Control.

*/

GLOBAL void ma_rach_stop_ra_req (void)
{
  PALLOC (stop_ra_req, MPHC_STOP_RA_REQ);

  PSENDX (L1, stop_ra_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_pch_start_ccch_req      |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal pch_start_ccch_req from SDL process
            PCH_Control.

*/

GLOBAL void ma_pch_start_ccch_req (T_MPHC_START_CCCH_REQ *pl_idle)
{
  GET_INSTANCE_DATA;
  if (memcmp ( &(alr_data->pch_data.last_start_ccch_req),
               pl_idle,
               sizeof(T_MPHC_START_CCCH_REQ) ) NEQ 0)
  {
    memcpy ( &(alr_data->pch_data.last_start_ccch_req),
             pl_idle,
             sizeof(T_MPHC_START_CCCH_REQ));
    PSENDX (L1, pl_idle);
  }
  else
  {
    PFREE(pl_idle);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_rach_random_cnf         |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal rach_random_cnf from SDL process
            RACH_Control.

*/

GLOBAL void ma_rach_random_cnf (T_MPH_RANDOM_ACCESS_CNF * mph_random_access_cnf)
{
  GET_INSTANCE_DATA;
  switch (GET_STATE (STATE_MA))
  {
    case MA_CON_EST:
      ALR_TRACE_MAIN_RA_CNF(mph_random_access_cnf->frame_no.t1,
                            mph_random_access_cnf->frame_no.t2,
                            mph_random_access_cnf->frame_no.t3);

#if !defined (NCONFIG)
      if(v_cfg_rach_failure)
        memset(mph_random_access_cnf,0, sizeof(T_MPH_RANDOM_ACCESS_CNF));
#endif

      PSENDX (RR, mph_random_access_cnf);
      break;

    default:
      PFREE (mph_random_access_cnf);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_PCH                    |
| STATE   : code                ROUTINE : ma_pch_paging_ind          |
+--------------------------------------------------------------------+

  PURPOSE : Sends a paging indication to RR.

*/

GLOBAL void ma_pch_paging_ind (UBYTE id_type, UBYTE channel_needed)
{
  PALLOC (mph_paging_ind, MPH_PAGING_IND);

  ALR_EM_PAGING_DETECTED;

  mph_paging_ind->identity_type = id_type;
  mph_paging_ind->channel_needed = channel_needed;

//  vsi_o_ttrace(VSI_CALLER 0xFFFF, "PAG IND ch=%x", channel_needed);
#if defined(DL_TRACE_ENABLED)
  {
    sprintf (dl_trace_buf, "PAG%u ch=%u", id_type,channel_needed);
    DL_OFFLINE_TRACE (dl_trace_buf);
  }
#else  /* DL_TRACE_ENABLED */
  SYST_TRACE_P ((SYST, "PAG%u ch=%u", id_type,channel_needed));
#endif  /* DL_TRACE_ENABLED */

  
  PSENDX (RR, mph_paging_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_pch_stop                |
+--------------------------------------------------------------------+

  PURPOSE : Stop CCCH reading.
*/

GLOBAL void ma_pch_stop(void)
{
  GET_INSTANCE_DATA;
  PALLOC(stop_req, MPHC_STOP_CCCH_REQ);
  alr_data->pch_data.last_start_ccch_req.bs_pa_mfrms = NOT_PRESENT_8BIT;
  PSENDX(L1, stop_req);
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mmi_cbch_req            |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMI_CBCH_REQ.

*/

GLOBAL void ma_mmi_cbch_req (T_MMI_CBCH_REQ *cbch_req)
{
  TRACE_EVENT ("MMI_CBCH_REQ");
  cb_mmi_cbch_req (cbch_req);
  PFREE (cbch_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mmi_cbch_req            |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMI_CBCH_REQ.

*/

GLOBAL void ma_mmi_sat_cbch_dwnld_req (T_MMI_SAT_CBCH_DWNLD_REQ *cbch_req)
{
  TRACE_EVENT ("MMI_SAT_CBCH_DWNLD_REQ");
  cb_mmi_sat_cbch_req (cbch_req);
  PFREE (cbch_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_cb_stop_cbch_req        |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal ma_cb_stop_cbch_req.

*/
GLOBAL void ma_cb_stop_cbch_req (void)
{
  PALLOC(stop_cbch_req, MPHC_STOP_CBCH_REQ);

  stop_cbch_req->normal_cbch = CBCH_STOP;
  stop_cbch_req->extended_cbch = CBCH_STOP;
  TRACE_EVENT ("MPHC_STOP_CBCH_REQ");
  PSENDX(L1, stop_cbch_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_cb_sched_req            |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal ma_cb_sched_req.

*/
GLOBAL void ma_cb_sched_req (T_MPHC_CBCH_SCHEDULE_REQ *sched_req)
{
  /*
  TRACE_EVENT ("MPHC_CBCH_SCHEDULE_REQ");
  */
  PSENDX (L1, sched_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_cb_config_cbch          |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal ma_cb_config_cbch.

*/
GLOBAL void ma_cb_config_cbch (T_MPHC_CONFIG_CBCH_REQ *config_cbch)
{
  TRACE_EVENT ("MPHC_CONFIG_CBCH_REQ");
  PSENDX(L1, config_cbch);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_cb_mmi_cbch_ind         |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal ma_cb_mmi_cbch_ind.

*/
GLOBAL void ma_cb_mmi_cbch_ind (T_MMI_CBCH_IND *mmi_cbch_ind)
{
  TRACE_EVENT ("MMI_CBCH_IND");
  PSENDX (MMI, mmi_cbch_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_cb_info_req             |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal ma_cb_info_req.

*/
GLOBAL void ma_cb_info_req (UBYTE bitmap)
{
  PALLOC(info_req, MPHC_CBCH_INFO_REQ);

  info_req->tb_bitmap = bitmap;
  TRACE_EVENT ("MPHC_CBCH_INFO_REQ");
  PSENDX (L1, info_req);
}
/*====================================================================
 *Functions used by more than one process
 *====================================================================

+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_scell_nbcch_req         |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal ma_scell_nbcch_req .

*/

GLOBAL void ma_scell_full_nbcch (void)
{
  GET_INSTANCE_DATA;
  PALLOC(scell_nbcch_req, MPHC_SCELL_NBCCH_REQ);

  ALR_TRACE_MAIN ("start full sc bcch");

  switch(GET_STATE(STATE_MA))
  {
    case MA_CELL_SELECTION:
    case MA_CELL_RESELECTION:
      alr_data->cs_data.si_bitmap = 0;
      break;
  }
  scell_nbcch_req->schedule_array_size = 1;
  scell_nbcch_req->schedule_array[0].modulus = 1;
  scell_nbcch_req->schedule_array[0].relative_position = 0;
  ma_scell_nbcch_req(scell_nbcch_req);
}

GLOBAL void ma_scell_nbcch_req (T_MPHC_SCELL_NBCCH_REQ *nbcch_req)
{
  ALR_TRACE_MAIN ("start sc bcch");

  PSENDX (L1, nbcch_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_scell_ebcch_req         |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal cs_scell_nbcch_req from SDL process
            Cell_Selection.
*/

GLOBAL void ma_scell_ebcch_req (T_MPHC_SCELL_EBCCH_REQ *ebcch_req)
{
  PSENDX (L1, ebcch_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_stop_active_procs       |
+--------------------------------------------------------------------+

  PURPOSE : Stop all active procedures.
*/
GLOBAL void ma_stop_active_procs (UBYTE flags)
{
  GET_INSTANCE_DATA;
  if (flags & STOP_PCH_READING)
  {
    pch_stop();
  }

  if (flags & STOP_MEASUREMENTS)
  {
    ma_stop_rxlev_periodic_req();
  }

  switch (GET_STATE(STATE_MA))
  {
    case MA_CELL_SELECTION:
      /*
       * if we are doing measurements or synchronising or
       * reading the BCCH of a cell, stop the process
       */
      cs_stop();
      break;
    case MA_CELL_RESELECTION:
      ma_stop_scell_bcch_req ();
      break;
    case MA_CON_EST:
      /*
       * Stop Uplink Random Burst Sending
       */
      rach_stop();
      break;
    case MA_DEDICATED:
      dedi_stop();
      break;
    case MA_IDLE:
      /* if we are reading the FCH or SCH of a ncell
         or reading its BCCH, stop it */
      nc_suspend();
#if defined (REL99) && defined (TI_PS_FF_EMR)
      /* before stop reading the scell BCCH reset the SIq_status to configure 
         when it is in pending state, so that it is easy to reconfigure again*/
      if((alr_data->nc_data.si2quater_status EQ SI2QUATER_ACQ_PENDING) OR 
            (alr_data->nc_data.si2quater_status EQ SI2QUATER_ACQ_FULL_BCCH))
      {
        alr_data->nc_data.si2quater_status = SI2QUATER_CONFIGURE;
      }
#endif
      /* if we are reading the BCCH of the serving cell,
         stop it */
      ma_stop_scell_bcch_req();
      /* if we are reading the CBCH, stop it */
      cb_stop();
      break;
#ifdef GPRS
    case MA_PTM:
      /* if we are reading the FCH or SCH of a ncell
         or reading its BCCH, stop it */
      nc_suspend();
      /* if we are reading the BCCH of the serving cell,
         stop it */
      /*
      ma_stop_scell_bcch_req();
      */
      break;
#endif  /* GPRS */
  };
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_send_unitdata           |
+--------------------------------------------------------------------+

  PURPOSE : Cut pseudo length and send data directly to RR.

*/

GLOBAL void ma_send_unitdata (T_MPHC_DATA_IND * data_ind)
{
#define SYS_INFO_LEN 24

  USHORT len_in_bits = SYS_INFO_LEN * BITS_PER_BYTE;

  PALLOC_SDU (data_out, MPH_UNITDATA_IND, len_in_bits);

  /* No IE and Pseudo Length */
  data_out->sdu.l_buf = (SYS_INFO_LEN - 2) * BITS_PER_BYTE;
  /* Pseudo Length */
  data_out->sdu.o_buf = 1 * BITS_PER_BYTE;
  /* No IE */
  /*lint -e{419} (Warning -- Apparent data overrun) */
  memcpy (data_out->sdu.buf, &data_ind->l2_frame, SYS_INFO_LEN - 1);
  data_out->arfcn = ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK;
  data_out->fn    = data_ind->fn;

#if 0 && defined(DL_TRACE_ENABLED)
  {
    sprintf (dl_trace_buf, "DATA ind msg=%u", msg_t);
    DL_OFFLINE_TRACE (dl_trace_buf);
  }
#endif  /* DL_TRACE_ENABLED */

  PSENDX (RR, data_out);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_error_ind               |
+--------------------------------------------------------------------+

  PURPOSE : send a MPH_ERROR_IND to RR.

*/

GLOBAL void ma_error_ind (UBYTE cause, USHORT arfcn)
{
  PALLOC (error_ind, MPH_ERROR_IND);

  ALR_TRACE_MAIN ("err ind");

  ALR_EM_ERROR_IND(cause, arfcn);

  error_ind->cs = cause;
  error_ind->arfcn = arfcn;

  PSENDX (RR, error_ind);
  /*
   * These errors could mean that the AFC value in L1 might be srewed
   * up somehow. So go back to wide band search mode for next CS to be
   * safe.
   */
  if( (cause EQ CS_DOWN_LINK_FAIL) OR (cause EQ CS_RADIO_LINK_FAIL))
    cs_set_wideband_sync();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_sync_ind                |
+--------------------------------------------------------------------+

  PURPOSE : Send a synchronisation primitive to RR.

*/

LOCAL void ma_sync_ind (UBYTE       cause,
                        USHORT      arfcn)
{
  PALLOC (sync_ind, MPH_SYNC_IND);

  TRACE_FUNCTION ("ma_sync_ind()");

  sync_ind->cs = cause;
  sync_ind->arfcn = arfcn;
  PSENDX (RR, sync_ind);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_stop_scell_bcch_req     |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal cs_stop_bcch_reading from SDL processes
            Cell_Selection or Serving_Cell.

*/

GLOBAL void ma_stop_scell_bcch_req (void)
{
  PALLOC (stop_bcch_req, MPHC_STOP_SCELL_BCCH_REQ);

  ALR_TRACE_MAIN ("stop sc bcch");

  PSENDX (L1, stop_bcch_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_stop_rxlev_periodic_req |
+--------------------------------------------------------------------+

  PURPOSE : Stops the periodic fieldstrength measurements during
            idle mode.

*/


LOCAL void ma_stop_rxlev_periodic_req (void)
{
  PALLOC (stop_rxlev_req, MPHC_STOP_RXLEV_PERIODIC_REQ);

  ALR_TRACE_MAIN ("stop rxlev perio.");

  PSENDX (L1, stop_rxlev_req);
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_bsic_cnf                |
+--------------------------------------------------------------------+

  PURPOSE : Sends the synchronisation result to RR.

*/

GLOBAL void ma_bsic_cnf (T_MPH_BSIC_CNF* mph_bsic_cnf)
{
  PSENDX (RR, mph_bsic_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_new_scell_req           |
+--------------------------------------------------------------------+

  PURPOSE : Sends a new scell request to layer 1.

*/

GLOBAL void ma_new_scell_req(T_MPHC_NEW_SCELL_REQ *new_scell)
{
  GET_INSTANCE_DATA;
  alr_data->bsic = new_scell->tsc;
  PSENDX(L1, new_scell);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mphc_init_l1_cnf        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_INIT_L1_CON.

*/

GLOBAL void ma_mphc_init_l1_cnf (T_MPHC_INIT_L1_CON *init_l1_cnf)
{
  GET_INSTANCE_DATA;
  
  switch (GET_STATE (STATE_CS))
  {
  case CS_INIT_ACTIVE:
    SET_STATE (STATE_CS, CS_INIT_DONE);
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
    compare_l1_frequency_band_with_ffs (init_l1_cnf);
#endif
    ma_cs_rxlev_req ();
    break;
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  default :
    TRACE_EVENT_P1 ("MPHC_INIT_L1_CON received in the wrong CS state %d", GET_STATE (STATE_CS));
    break;
#else
  case CS_INIT_SYNC:
    SET_STATE (STATE_CS, CS_INIT_DONE);
    cs_bsic_req (NULL);
#endif
  }
  PFREE (init_l1_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mphc_stop_dedi_con        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPHC_STOP_DEDICATED_CON.

*/

GLOBAL void ma_mphc_stop_dedi_con (T_MPHC_STOP_DEDICATED_CON *stop_cnf)
{
#ifdef GPRS
  GET_INSTANCE_DATA;
#endif
  PFREE (stop_cnf);

#ifdef GPRS
  if(alr_data->dedi_data.act_mode EQ MODE_PDCH_ASSIGN)
  {
    /*
     * Notify RR about the actions performed due to PDCH Assignment.
     * Note that MPH_DEDICATED_CNF is misleading as L1 is in Idle Mode now.
     */
    ma_dedi_cnf (DEDI_RES_OK);
    trc_state_transition(0, MA_IDLE);
    SET_STATE(STATE_MA, MA_IDLE); /* allow PTM with next MPH_IDLE_REQ(PTM) */
    return;
  }
#endif
  {
    PALLOC(mph_dedi_cnf, MPH_STOP_DEDICATED_CNF);
    PSENDX ( RR, mph_dedi_cnf );
  }
}

/*
 * The following two functions convert ARFCNs between the official GSM
 * spec notation and the numbering scheme used by TI's L1.  The latter
 * scheme changed with the new way of multiband handling found in the
 * LoCosto source, but not in TCS211.
 *
 * The original L1 frequency channel numbering scheme (used in TCS211)
 * looks like this:

            Frequency Band    config.       ETSI GSM spec.  coding used in L1
            GSM 900           STD_900       1-124           1-124
            E-GSM             STD_EGSM      1-124,          1-124,
                                            975-1023, 0     125-174
            PCS 1900          STD_1900      512-810         512-810
            DCS 1800          STD_1800      512-885         512-885
            GSM 900/DCS 1800  STD_DUAL      1-124,          1-124,
                                            512-885         125-498
            E-GSM/DCS 1800    STD_DUAL_EGSM 1-124,          1-124,
                                            975-1023,0      125-174,
                                            512-885         175-548
            GSM 850           STD_850       128-251         128-251
            GSM 850/PCS 1900  STD_DUAL_US   128-251,        1-124,
                                            512-810         125-424

 * The new-fangled scheme found in the LoCosto source looks like this:

            Frequency Band    config.       ETSI GSM spec.  coding used in L1
            GSM 900           STD_900       1-124           1-124
            E-GSM             STD_EGSM      1-124,          1-124,
                                            975-1023, 0     975-1023, 0
            PCS 1900          STD_1900      512-810         1024-1322
            DCS 1800          STD_1800      512-885         512-885
            GSM 900/DCS 1800  STD_DUAL      1-124,          1-124,
                                            512-885         512-885
            E-GSM/DCS 1800    STD_DUAL_EGSM 1-124,          1-124,
                                            975-1023,0      975-1023,0
                                            512-885         512-885
            GSM 850           STD_850       128-251         128-251
            GSM 850/PCS 1900  STD_DUAL_US   128-251,        128-251,
                                            512-810         1024-1322
            GSM 850/DCS 1800  STD_850_1800  128-251,        128-251,
                                            512-885         512-885
            GSM 900/PCS 1900  STD_900_1900  1-124,          1-124,
                                            512-810         1024-1322

 * The implementation of the two functions below, taken from the
 * LoCosto source, has interspersed #ifdef TI_PS_FF_QUAD_BAND_SUPPORT
 * conditionals, selecting between the two possible configurations.
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : convert_arfcn_to_l1        |
+--------------------------------------------------------------------+

  PURPOSE : Convert_arfcn_to_l1 converts the arfcn given by the upper
            layer (ETSI GSM spec.) to the internal format of layer 1.

*/

GLOBAL USHORT convert_arfcn_to_l1 (USHORT arfcn, UBYTE local_std)
{
#ifdef WIN32
  GET_INSTANCE_DATA;
  if (!alr_data->mb_testing)
    arfcn = arfcn&ARFCN_MASK;
  else
  {
#endif
  if (local_std EQ 0)
    local_std = std;
  arfcn &= ARFCN_MASK;
  switch (local_std)
  {
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT  
    case STD_1900:
    case STD_DUAL_US:
    case STD_900_1900:
    case STD_850_900_1900:
      if ((arfcn >= LOW_CHANNEL_1900) AND (arfcn <= HIGH_CHANNEL_1900))
        arfcn += LOW_CHANNEL_1900;
      break;
#else
    case STD_DUAL:
      if (arfcn >= LOW_CHANNEL_1800)
        arfcn -= (LOW_CHANNEL_1800-125);
      break;

    case STD_EGSM:
    case STD_DUAL_EGSM:
      if (arfcn >= LOW_CHANNEL_EGSM)
        arfcn -= (LOW_CHANNEL_EGSM-125);
      else if (arfcn >= LOW_CHANNEL_1800)
        arfcn -= (LOW_CHANNEL_1800-175);
      else if (arfcn EQ CHANNEL_0)
        arfcn = 174;
      break;

    case STD_DUAL_US:
      if (arfcn >= LOW_CHANNEL_1900)
        arfcn -= (LOW_CHANNEL_1900-125);
      else
        arfcn -= (LOW_CHANNEL_850-1);
      break;
#endif
    default:
      break;
  }
#ifdef WIN32
  }
#endif
  return arfcn;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : convert_arfcn_to_g23       |
+--------------------------------------------------------------------+

  PURPOSE : Convert_arfcn_to_g23 converts the radio frequence given by
            layer 1 to the common format of the upper layer (ETSI GSM spec.).

*/

GLOBAL USHORT convert_arfcn_to_g23 (USHORT arfcn, UBYTE local_std)
{
#ifdef WIN32
  GET_INSTANCE_DATA;
  if (!alr_data->mb_testing)
    arfcn = arfcn&ARFCN_MASK;
  else
  {
#endif
  if (local_std EQ 0)
    local_std = std;
  arfcn &= ARFCN_MASK;
  switch (local_std)
  {
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT  
    case STD_1900:
    case STD_DUAL_US:
    case STD_900_1900:
    case STD_850_900_1900:
      if (arfcn >= LOW_CHANNEL_1900_INTERNAL)
        arfcn -= LOW_CHANNEL_1900;
      break;
#else
    case STD_DUAL:
      if (arfcn >=125)
        arfcn += (LOW_CHANNEL_1800-125);
      break;

    case STD_EGSM:
    case STD_DUAL_EGSM:
      if (arfcn >= 175)
        arfcn += (LOW_CHANNEL_1800-175);
      else if (arfcn EQ 174)
        arfcn = CHANNEL_0;
      else if (arfcn >= 125)
        arfcn += (LOW_CHANNEL_EGSM-125);
      break;

    case STD_DUAL_US:
      if (arfcn >= 125)
        arfcn += (LOW_CHANNEL_1900-125);
      else
        arfcn += (LOW_CHANNEL_850-1);
      /*lint -fallthrough*/
    case STD_850:
    case STD_1900:
      /* only for MPH_POWER_CNF and MPH_BSIC_CNF
      arfcn |= US_BIT;
       */
      break;
#endif
    default:
      break;
  }
#ifdef WIN32
  }
#endif
  return arfcn;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_clean_dedi_sys_buffer   |
+--------------------------------------------------------------------+

  PURPOSE : Cleans System Information Message from SACCH.

*/

GLOBAL void ma_clean_dedi_sys_buffer (void)
{
  GET_INSTANCE_DATA;
  memset (alr_data->ma_data.sys_info_5, 0, 18);
  memset (alr_data->ma_data.sys_info_5bis, 0, 18);
  memset (alr_data->ma_data.sys_info_5ter, 0, 18);
  memset (alr_data->ma_data.sys_info_6, 0, 18);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_set_si_bitmap         |
+--------------------------------------------------------------------+

  PURPOSE : Updates the si_bitmap.

*/

LOCAL void ma_set_si_bitmap (UBYTE msg_t)
{
  GET_INSTANCE_DATA;
  switch(msg_t)
  {
     case D_SYS_INFO_1:
        alr_data->cs_data.si_bitmap |= SI_1_READ;
        break;
     case D_SYS_INFO_2:
        alr_data->cs_data.si_bitmap |= SI_2_READ;
        break;
     case D_SYS_INFO_3:
        alr_data->cs_data.si_bitmap |= SI_3_READ;
        break;
     case D_SYS_INFO_4:
        alr_data->cs_data.si_bitmap |= SI_4_READ;
        break;
     case D_SYS_INFO_2BIS:
        alr_data->cs_data.si_bitmap |= SI_2BIS_READ;
        break;
     case D_SYS_INFO_2TER:
        alr_data->cs_data.si_bitmap |= SI_2TER_READ;
        break;
     default:
        break;
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_fill_sys_buffer         |
+--------------------------------------------------------------------+

  PURPOSE : Stores a System Information Message.

*/

LOCAL void ma_fill_sys_buffer (T_MPHC_DATA_IND* data_ind, UBYTE msg_t)
{
  GET_INSTANCE_DATA;
  switch (msg_t)
  {
    case D_SYS_INFO_1:
      memcpy (alr_data->ma_data.sys_info_1,
              &data_ind->l2_frame.content[1], 22);
      break;
    case D_SYS_INFO_2:
      memcpy (alr_data->ma_data.sys_info_2,
              &data_ind->l2_frame.content[1], 22);
      break;
    case D_SYS_INFO_2BIS:
      memcpy (alr_data->ma_data.sys_info_2bis,
              &data_ind->l2_frame.content[1], 22);
      break;
    case D_SYS_INFO_2TER:
      memcpy (alr_data->ma_data.sys_info_2ter,
              &data_ind->l2_frame.content[1], 22);
      break;
#if defined (REL99) && defined (TI_PS_FF_EMR)
    case D_SYS_INFO_2QUATER:
      memcpy (alr_data->ma_data.sys_info_2quater,
              &data_ind->l2_frame.content[1], 22);
      break;
#endif
    case D_SYS_INFO_3:
      memcpy (alr_data->ma_data.sys_info_3,
              &data_ind->l2_frame.content[1], 22);
      break;
    case D_SYS_INFO_4:
      memcpy (alr_data->ma_data.sys_info_4,
              &data_ind->l2_frame.content[1], 22);
      break;
#ifdef GPRS
    case D_SYS_INFO_13:
      memcpy (alr_data->ma_data.sys_info_13,
              &data_ind->l2_frame.content[1], 22);
      break;
#endif  /* GPRS */
    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_compare_sys_buffer      |
+--------------------------------------------------------------------+

  PURPOSE : Compares a System Information Message.

*/
LOCAL BOOL ma_compare_sys_buffer (T_MPHC_DATA_IND *mphc_data_ind,
                                  UBYTE msg_t)
{
  GET_INSTANCE_DATA;
  switch (msg_t)
  {
    case D_SYS_INFO_1:
      return (!memcmp (alr_data->ma_data.sys_info_1,
                       &mphc_data_ind->l2_frame.content[1], 22));
      /*break;*/
    case D_SYS_INFO_2:
      return (!memcmp (alr_data->ma_data.sys_info_2,
                       &mphc_data_ind->l2_frame.content[1], 22));
      /*break;*/
    case D_SYS_INFO_2BIS:
      return (!memcmp (alr_data->ma_data.sys_info_2bis,
                       &mphc_data_ind->l2_frame.content[1], 22));
      /*break;*/
    case D_SYS_INFO_2TER:
      return (!memcmp (alr_data->ma_data.sys_info_2ter,
                       &mphc_data_ind->l2_frame.content[1], 18));/* Ignore si2ter_rest_octets changes */
      /*break;*/
#if defined (REL99) && defined (TI_PS_FF_EMR)
    case D_SYS_INFO_2QUATER:
      return 0;
      /*break;*/
#endif
    case D_SYS_INFO_3:
      return (!memcmp (alr_data->ma_data.sys_info_3,
                       &mphc_data_ind->l2_frame.content[1], 22));
      /*break;*/
    case D_SYS_INFO_4:
      return (!memcmp (alr_data->ma_data.sys_info_4,
                       &mphc_data_ind->l2_frame.content[1], 22));
      /*break;*/
#ifdef GPRS
    case D_SYS_INFO_13:
      return (!memcmp (alr_data->ma_data.sys_info_13,
                       &mphc_data_ind->l2_frame.content[1], 22));
      /*break;*/
#endif  /* GPRS */
    default:
      break;
  }
  return 0;
}


GLOBAL void sc_start_periodic (void)
{
  GET_INSTANCE_DATA;
#ifdef GPRS
  if(gprs_check_read_si13_only())
#endif /* GPRS */
  {
    USHORT i;
    UBYTE k, si_bitmap = alr_data->cs_data.si_bitmap;
    PALLOC(scell_bcch_req, MPHC_SCELL_NBCCH_REQ);

    /* 3GPP TS 05.02 Section 6.3.1.3: If only one of SI2bis and 
     * SI2ter is there it will be sent when TC=5
     */
    if((si_bitmap&SI_2BIS_READ) EQ 0 AND
       (si_bitmap&SI_2TER_READ) NEQ 0)
        si_bitmap = (si_bitmap & ~(SI_2TER_READ)) | SI_2BIS_READ;
    
    /* we want to read all SYSTEM INFO's broadcasted, TC=0..7 */
    for (i=0,k=0; i < 8; i++ )
    {
      if( (si_bitmap&(1<<i)) != 0)
      {
        scell_bcch_req->schedule_array[k].modulus = THIRTY_SECONDS_SCELL_BCCH;
        scell_bcch_req->schedule_array[k].relative_position = i;
        k++;
      }
    }
    scell_bcch_req->schedule_array_size = k;
    ma_scell_nbcch_req(scell_bcch_req);
  }

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : alr_MAIN                   |
| STATE   : code                ROUTINE : ma_is_ptm	             |
+--------------------------------------------------------------------+

PURPOSE : Returns True if the current state is MA_PTM,
	  otherwise returns FALSE.

*/


#ifdef GPRS
GLOBAL BOOL ma_is_ptm(void)
{
  GET_INSTANCE_DATA;
  if(GET_STATE(STATE_MA) EQ MA_PTM)
    return TRUE;
  else
    return FALSE;
}
#endif  /* GPRS */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : alr_MAIN                   |
| STATE   : code                ROUTINE : ma_nc_rxlev_sc_req         |
+--------------------------------------------------------------------+

PURPOSE : A new RXLEV value of the serving cell has been measured.
          Inform GPL in case it is necessary.

*/
#ifdef GPRS

GLOBAL void ma_nc_rxlev_sc_req (T_TB_RXLEV_SC_REQ *rxlev_sc_req)
{
  GET_INSTANCE_DATA;
  if(GET_STATE (STATE_MA)     EQ MA_IDLE AND
     gprs_alr_is_supported( ) EQ TRUE        )
  {
    PSENDX (GPL, rxlev_sc_req);
  }
  else
  {
    PFREE (rxlev_sc_req);
  }
}

#endif /* #ifdef GPRS */

/*
 *--------------------------------------------------------------------
 * helper functions for main process
 * these should all be LOCAL
 *--------------------------------------------------------------------
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_clean_sys_buffer        |
+--------------------------------------------------------------------+

  PURPOSE : Cleans System Information Message from BCCH.

*/

GLOBAL void ma_clean_sys_buffer (USHORT si_mask)
{
  GET_INSTANCE_DATA;
  if ( si_mask & IND_SI_1    ) memset (alr_data->ma_data.sys_info_1,    0, 22);
  if ( si_mask & IND_SI_2    ) memset (alr_data->ma_data.sys_info_2,    0, 22);
  if ( si_mask & IND_SI_2BIS ) memset (alr_data->ma_data.sys_info_2bis, 0, 22);
  if ( si_mask & IND_SI_3    ) memset (alr_data->ma_data.sys_info_3,    0, 22);
  if ( si_mask & IND_SI_4    ) memset (alr_data->ma_data.sys_info_4,    0, 22);
  if ( si_mask & IND_SI_2TER ) memset (alr_data->ma_data.sys_info_2ter, 0, 22);
#if defined (REL99) && defined (TI_PS_FF_EMR)
  if ( si_mask & IND_SI_2QUATER ) memset (alr_data->ma_data.sys_info_2quater, 0, 22);
#endif
#ifdef GPRS
  if ( si_mask & IND_SI_13   ) memset (alr_data->ma_data.sys_info_13,   0, 22);
#endif
  if ( si_mask & IND_SI_5    ) memset (alr_data->ma_data.sys_info_5,    0, 18);
  if ( si_mask & IND_SI_5BIS ) memset (alr_data->ma_data.sys_info_5bis, 0, 18);
  if ( si_mask & IND_SI_5TER ) memset (alr_data->ma_data.sys_info_5ter, 0, 18);
  if ( si_mask & IND_SI_6    ) memset (alr_data->ma_data.sys_info_6,    0, 18);
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_fill_dedi_sys_buffer    |
+--------------------------------------------------------------------+

  PURPOSE : Stores a System Information Message from SACCH.

*/
LOCAL void ma_fill_dedi_sys_buffer (T_MPHC_DATA_IND * mphc_data_ind,
                                    UBYTE msg_t)
{
  GET_INSTANCE_DATA;
  switch (msg_t)
  {
    case D_SYS_INFO_5:
      memcpy (alr_data->ma_data.sys_info_5,
              &mphc_data_ind->l2_frame.content[5], 18);
      break;
    case D_SYS_INFO_5BIS:
      memcpy (alr_data->ma_data.sys_info_5bis,
              &mphc_data_ind->l2_frame.content[5], 18);
      break;
    case D_SYS_INFO_5TER:
      memcpy (alr_data->ma_data.sys_info_5ter,
              &mphc_data_ind->l2_frame.content[5], 18);
      break;
    case D_SYS_INFO_6:
      memcpy (alr_data->ma_data.sys_info_6,
              &mphc_data_ind->l2_frame.content[5], 18);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : alr_MAIN                   |
| STATE   : code                ROUTINE : ma_compare_dedi_sys_buffer |
+--------------------------------------------------------------------+

  PURPOSE : Compares a System Information Message on SACCH.

*/

LOCAL BOOL ma_compare_dedi_sys_buffer (T_MPHC_DATA_IND * mphc_data_ind,
                                       UBYTE msg_t)
{
  GET_INSTANCE_DATA;
  switch (msg_t)
  {
    case D_SYS_INFO_5:
      return (!memcmp (alr_data->ma_data.sys_info_5,
                       &mphc_data_ind->l2_frame.content[5], 18));
      /*break;*/
    case D_SYS_INFO_5BIS:
      return (!memcmp (alr_data->ma_data.sys_info_5bis,
                       &mphc_data_ind->l2_frame.content[5], 18));
      /*break;*/
    case D_SYS_INFO_5TER:
      return (!memcmp (alr_data->ma_data.sys_info_5ter,
                       &mphc_data_ind->l2_frame.content[5], 18));
      /*break;*/
    case D_SYS_INFO_6:
      return (!memcmp (alr_data->ma_data.sys_info_6,
                       &mphc_data_ind->l2_frame.content[5], 18));
      /*break;*/
    default:
      break;
  }
  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : RR_ATT                      |
| STATE   : code               ROUTINE : tim_stop_sync_to_nw         |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of TIM_NW_SYNC_GUARD - Time to cancel Nw
            search as it is taking too long

*/
GLOBAL void tim_stop_sync_to_nw( void )
{
  GET_INSTANCE_DATA;
  /* Change the CS State to CS_NW_SYNC_TIMEOUT only if NW Sync is active */
  if(alr_data->cs_data.sync_active)
  {
    SET_STATE(STATE_CS, CS_NW_SYNC_TIMEOUT);
    ma_cs_stop_network_sync_req();
  }
}

#ifdef _TARGET_
#ifndef FAX_AND_DATA
/*
 * Dummy functions for RA (needed when building voice-only PS w/ F&D TI
 * layer 1 libs).
 */
GLOBAL void dll_data_ul
            (
              USHORT *_ul_buffer_address,
              USHORT *_d_ra_conf,
              USHORT *_d_ra_act,
              USHORT *_d_ra_test,
              USHORT *_d_ra_statu,
              USHORT *_d_fax
            )
{
  /* do nothing */
}

GLOBAL void dll_data_dl
            (
              USHORT *_dl_buffer_address,
              USHORT *_d_ra_act,
              USHORT *_d_ra_statd
            )
{
  /* do nothing */
}
#endif /* !_FAX_AND_DATA_ */

#ifndef GPRS
/*
 * Dummy functions for GRR (needed when building non-GPRS PS w/ GPRS TI
 * layer 1 libs).
 */
typedef void  T_ul_poll_resp;
typedef void  T_ul_data;
typedef void  T_dl_struct;
typedef UBYTE UWORD8;
typedef ULONG UWORD32;
#define WORD8     UBYTE
#define UWORD16   USHORT

GLOBAL void maca_power_control ( UWORD8  assignment_id,
                                 BOOL    crc_error,
                                 WORD8   bcch_level,
                                 UWORD16 *radio_freq,
                                 WORD8  *burst_level,
                                 UWORD8 *pch )
{
  /* do nothing */
}

GLOBAL void rlc_uplink ( UWORD8   assignment_id,
                         UWORD8   tx_data_no,
                         UWORD32  fn,
                         UWORD8   timing_advance_value,
                         T_ul_poll_resp    * ul_poll_response,
                         T_ul_data         * ul_data,
                         BOOL      allocation_exhausted)
{
  /* do nothing */
}

GLOBAL void rlc_downlink ( UWORD8        assignment_id,
                           UWORD32       fn,
                           T_dl_struct   *data_ptr,
                           UWORD8        rlc_blocks_sent,
                           UWORD8        last_poll_response)
{
  /* do nothing */
}
#endif /* !GPRS */
#endif /* _TARGET_ */
#if defined (REL99) && defined (TI_PS_FF_EMR)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_enhpara_update_req  |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_ENHPARA_UPDATE_REQ.

*/

GLOBAL void ma_mph_enhpara_update_req (T_MPH_ENHPARA_UPDATE_REQ* p_enh)
{
  GET_INSTANCE_DATA;
  /*UBYTE     i=0;*/

  alr_data->nc_data.emr_data.rep_type = p_enh->rep_type;
  alr_data->nc_data.emr_data.enh_para = p_enh->enh_para;
  PFREE(p_enh);
  return;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_mph_mon_ctrl_req        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_MON_CTRL_REQ.

*/
GLOBAL void ma_mph_mon_ctrl_req(T_MPH_MON_CTRL_REQ* ctrl_req)
{
  GET_INSTANCE_DATA;
  TRACE_EVENT ("ma_mph_mon_ctrl_req");
  /*Note: Request for monitoring SI-2quater will always be
    given for serving cell and when it is in stable IDLE state
    So, no need to check for MA_STATE here*/
  if ( (ctrl_req->action EQ START_MON_NBCCH ) AND
    (ctrl_req->si_to_read EQ UPDATE_SI2QUATER ) )
  {
    if(alr_data->ma_data.sys_info_2quater[1] EQ D_SYS_INFO_2QUATER)
    {
      ma_send_stored_SI2qtr(&alr_data->ma_data.sys_info_2quater[0]);
      memset (alr_data->ma_data.sys_info_2quater,    0, 22);
    }
    ma_scell_mon_si2quater_nbcch();
  }
  else if ( (ctrl_req->action EQ START_MON_EBCCH ) AND
    (ctrl_req->si_to_read EQ UPDATE_SI2QUATER ) )
  {
    if(alr_data->ma_data.sys_info_2quater[1] EQ D_SYS_INFO_2QUATER)
    {
      ma_send_stored_SI2qtr(&alr_data->ma_data.sys_info_2quater[0]);
      memset (alr_data->ma_data.sys_info_2quater,    0, 22);
    }
    ma_scell_mon_si2quater_ebcch();
  }
  else if((ctrl_req->si_to_read EQ UPDATE_SI2QUATER_AGAIN) AND
           (ctrl_req->action EQ START_MON_EBCCH ))
  {
     ma_scell_mon_si2quater_ebcch();
  }
  else if((ctrl_req->si_to_read EQ UPDATE_SI2QUATER_AGAIN) AND
           (ctrl_req->action EQ START_MON_NBCCH ))
    {
       ma_scell_mon_si2quater_nbcch();
    }
  
  else if ( (ctrl_req->action EQ STOP_MON_BCCH)AND
    (ctrl_req->si_to_read EQ UPDATE_SI2QUATER ) )
  {
    if ( alr_data->nc_data.si2quater_status EQ SI2QUATER_ACQ_PENDING)
    {
      if(alr_data->nc_data.si2quater_pos EQ SI2QUATER_ON_EBCCH)
      {
        ma_stop_scell_bcch_req();
      }
      sc_start_periodic();
    }
    alr_data->nc_data.si2quater_status = SI2QUATER_ACQ_COMP;
  }
  else if((ctrl_req->action EQ STOP_MON_BCCH)AND
    (ctrl_req->si_to_read EQ UPDATE_SI2QUATER_AGAIN) )
    {
    if (alr_data->nc_data.si2quater_status EQ SI2QUATER_ACQ_PENDING)
    {
      if(alr_data->nc_data.si2quater_pos EQ SI2QUATER_ON_EBCCH)
      {
        ma_stop_scell_bcch_req();
      }
      sc_start_periodic();
    }
    alr_data->nc_data.si2quater_status = SI2QUATER_ACQ_WRONG_BAIND;
    }
  
#ifdef GPRS
  else
  {
    gprs_alr_mon_ctrl_req(ctrl_req);
    return;
  }
#endif

#ifdef _SIMULATION_
    switch(alr_data->nc_data.si2quater_status) {
    case SI2QUATER_ACQ_PENDING:
      TRACE_EVENT ("Status of SI-2quater : SI2QUATER_ACQ_PENDING");
    	break;
    case SI2QUATER_ACQ_FULL_BCCH:
      TRACE_EVENT ("Status of SI-2quater : SI2QUATER_ACQ_FULL_BCCH");
      break;
    case SI2QUATER_ACQ_COMP:
      TRACE_EVENT ("Status of SI-2quater : SI2QUATER_ACQ_COMP");
      break;
    case SI2QUATER_ACQ_WRONG_BAIND:
      TRACE_EVENT ("Status of SI-2quater : SI2QUATER_ACQ_WRONG_BAIND");
      break;
    default:
      TRACE_EVENT ("Status of SI-2quater : SI2QUATER_ABSENT");
    }
#endif
    PFREE(ctrl_req);
  return;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_scell_mon_si2quater_ebcch   |
+--------------------------------------------------------------------+

  PURPOSE : configure L1 for monitoring SI-2quater on EBCCH.

*/
GLOBAL void ma_scell_mon_si2quater_ebcch(void)
{
  GET_INSTANCE_DATA;
  PALLOC(req, MPHC_SCELL_EBCCH_REQ);
  TRACE_EVENT ("ma_scell_mon_si2quater_ebcch");
  req->schedule_array_size                 = 1;
  req->schedule_array[0].relative_position = 5;
  req->schedule_array[0].modulus           = 8;
  ma_scell_ebcch_req(req);
  alr_data->nc_data.si2quater_status = SI2QUATER_ACQ_PENDING;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_scell_mon_si2quater_nbcch   |
+--------------------------------------------------------------------+

  PURPOSE : configure L1 for monitoring SI-2quater on NBCCH.

*/
GLOBAL void ma_scell_mon_si2quater_nbcch(void)
{
  GET_INSTANCE_DATA;
  PALLOC(req, MPHC_SCELL_NBCCH_REQ);
  TRACE_EVENT ("ma_scell_mon_si2quater_nbcch");
  /*on N-BCCH, SI-2quater can be received on either
  TC = 5 or 4 depending on scheduling of SI-2bis/ter */
  req->schedule_array_size                 = 2;
  req->schedule_array[0].relative_position = 5;
  req->schedule_array[0].modulus           = 8;
  req->schedule_array[1].relative_position = 4;
  req->schedule_array[1].modulus           = 8;
  ma_scell_nbcch_req(req);
  alr_data->nc_data.si2quater_status = SI2QUATER_ACQ_PENDING;
  return;
}

/*                      BCCH MAPPING
====================================================================
System Information Message | Sent when TC = |     Allocation
====================================================================
      Type 1	                    0	               BCCH Norm
-----------------------------------------------------------------
      Type 2	                    1	               BCCH Norm
-----------------------------------------------------------------
      Type 2 bis                    5	               BCCH Norm
-----------------------------------------------------------------
      Type 2 ter                 5 or 4	               BCCH Norm
-----------------------------------------------------------------
      Type 2 quater                 5                  BCCH Norm
                                   or
                                  4or5                 BCCH Ext
-----------------------------------------------------------------
      Type 3	                  2 and 6              BCCH Norm
-----------------------------------------------------------------
      Type 4	                  3 and 7              BCCH Norm
-----------------------------------------------------------------
      Type 7	                    7	               BCCH Ext
-----------------------------------------------------------------
      Type 8	                    3	               BCCH Ext
-----------------------------------------------------------------
      Type 9	                    4	               BCCH Norm
-----------------------------------------------------------------
      Type 13	                    4                  BCCH Norm
                                    or
                                    0                  BCCH Ext
-----------------------------------------------------------------
      Type 16	                    6	               BCCH Ext
-----------------------------------------------------------------
      Type 17	                    2	               BCCH Ext
-----------------------------------------------------------------
*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_MAIN                   |
| STATE   : code                ROUTINE : ma_send_stored_SI2qtr      |
+--------------------------------------------------------------------+

  PURPOSE : Cut pseudo length and send the stored SI2qtr data to RR.

*/
GLOBAL void ma_send_stored_SI2qtr(UBYTE* si_2quater)
{
  GET_INSTANCE_DATA;
  USHORT len_in_bits = 22 * BITS_PER_BYTE;
  PALLOC_SDU (data_out, MPH_UNITDATA_IND, len_in_bits);

  /* No IE and Pseudo Length */
  data_out->sdu.l_buf = (22 ) * BITS_PER_BYTE;
  /* Pseudo Length */
  data_out->sdu.o_buf = 0;
  /* No IE */
  /*lint -e{419} (Warning -- Apparent data overrun) */
  memcpy (data_out->sdu.buf, &si_2quater[0], 22 );
  data_out->arfcn = alr_data->serving_cell;
  TRACE_EVENT("SI2qtr sent from the function ma_send_stored_SI2qtr");
  PSENDX (RR, data_out);
}
#endif

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
LOCAL void compare_l1_frequency_band_with_ffs (T_MPHC_INIT_L1_CON *init_l1_cnf)
{
  GET_INSTANCE_DATA;
  int i;
  U16 freq_band ;

  PALLOC(mph_init_rr_ind, MPH_INIT_RR_IND);

  for (i=0; i<NB_MAX_GSM_BANDS; i++)
  {
    if (init_l1_cnf->multiband_power_class[i].radio_band == RADIO_BAND_PGSM900)
    {
      freq_band |= BAND_GSM_900;
      mph_init_rr_ind->pclass_900 = init_l1_cnf->multiband_power_class[i].power_class;
    }
    else if (init_l1_cnf->multiband_power_class[i].radio_band == RADIO_BAND_GSM850)
    {
      freq_band |= BAND_GSM_850;
      mph_init_rr_ind->pclass_850 = init_l1_cnf->multiband_power_class[i].power_class;
    }
    else if (init_l1_cnf->multiband_power_class[i].radio_band == RADIO_BAND_PCS1900)
    {
      freq_band |= BAND_PCS_1900;
      mph_init_rr_ind->pclass_1900 = init_l1_cnf->multiband_power_class[i].power_class;
    }
    else if (init_l1_cnf->multiband_power_class[i].radio_band == RADIO_BAND_DCS1800)
    {
      freq_band |= BAND_DCS_1800;
      mph_init_rr_ind->pclass_1800 = init_l1_cnf->multiband_power_class[i].power_class;
    }
    else if (init_l1_cnf->multiband_power_class[i].radio_band == RADIO_BAND_GSM480)
    {
      freq_band |= BAND_GSM_480;
    }
    else if (init_l1_cnf->multiband_power_class[i].radio_band == RADIO_BAND_GSM450)
    {
      freq_band |= BAND_GSM_450;
    }
    else if (init_l1_cnf->multiband_power_class[i].radio_band == RADIO_BAND_EGSM900)
    {
      freq_band |= BAND_E_GSM;
      mph_init_rr_ind->pclass_900 = init_l1_cnf->multiband_power_class[i].power_class;
    }
    else if (init_l1_cnf->multiband_power_class[i].radio_band == RADIO_BAND_RGSM900)
    {
      freq_band |= BAND_R_GSM;
    }
  }
  
  mph_init_rr_ind->freq_bands = freq_band;
  PSENDX (RR, mph_init_rr_ind);
/*
  if (freq_band NEQ alr_data->cs_data.freq_bands)
  {
    TRACE_EVENT_P2 ("FFS data %x, doen't match L1 compilation flag %x", alr_data->cs_data.freq_bands, freq_band);
    TRACE_ERROR ("FFS data doesn't match L1 compilation flag");
    TRACE_ASSERT (freq_band NEQ alr_data->cs_data.freq_bands);
  }*/
}
#endif

#endif  /* ALR_MAIN_C */
