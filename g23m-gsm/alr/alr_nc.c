/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  ALR_NC
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
|  Purpose :  This Modul defines the SDL process Idle_Neighbour_Cells.
+-----------------------------------------------------------------------------
*/

#ifndef ALR_NC_C
#define ALR_NC_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_PL

/*==== INCLUDES ===================================================*/
#include <string.h>
#include <stdlib.h>
#include <limits.h>
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

#if !defined(NTRACE)
#define TRACING
#endif  /* !TRACING */

#include "alr.h"
#include "alr_em.h"

#if defined (REL99) && defined (TI_PS_FF_EMR)
#define  MAX_MEAN_BEP_VAL 32
#define  MAX_CV_BEP_VAL   8

/*Here Mean BEP for GMSK is given, since ECSD is not supported
  it's assumed that 8PSK will not be required as of now*/
/* These values are derived from sec.8.2.5 of 5.08 
   The value given under the heading 'log10(actual BEP)' is multiplied
   by 2^26 to get these values*/
/*The following values are calculated assuming F6.26 format*/
const ULONG mbep_map[MAX_MEAN_BEP_VAL] = 
{ 
    /******GMSK MAPPING TABLE***/
    /* 0*/        16856941,
    /* 1*/        13389560,
    /* 2*/        10635412,
    /* 3*/        8448334,
    /* 4*/        6710886,
    /* 5*/        5330457,
    /* 6*/        4233898,
    /* 7*/        3362825,
    /* 8*/        2671603,
    /* 9*/        2121982,
    /* 10*/       1685103,
    /* 11*/       1338821,
    /* 12*/       1063004,
    /* 13*/       844229,
    /* 14*/       671088,
    /* 15*/       532844,
    /* 16*/       422785,
    /* 17*/       336215,
    /* 18*/       267093,
    /* 19*/       212064,
    /* 20*/       168443,
    /* 21*/       133546,
    /* 22*/       106032,
    /* 23*/       83886,
    /* 24*/       67108,
    /* 25*/       53016,
    /* 26*/       42278,
    /* 27*/       33554,
    /* 28*/       26172,
    /* 29*/       20803,
    /* 30*/       16777
};

/*Refer to sec.8.2.5,  5.08 
  These values are derived by multiplying CV_BEP X value by 2^8*/
/*The following values are calculated using F8.8 format: Refer to example at the end of file*/
const USHORT cvbep_map[MAX_CV_BEP_VAL] = {
    /* 0*/        512,
    /* 1*/        448,
    /* 2*/        384,
    /* 3*/        320,
    /* 4*/        256,
    /* 5*/        192,
    /* 6*/        128,
    /* 7*/        64
};
#endif

#if !defined(NTRACE) && defined(TRACING)
LOCAL const char *nc_get_nc_state_str(UBYTE status);
LOCAL const char * const alr_nc_state_trc[18] = {
  "INACT",
  "IDLE",
  "RD_BCCH",
  "FB_SB_SYNC",
  "FB_SB_FAIL",
  "RD_FB_SB",
  "RD_SB",
  "IDLE_SYNC",
  "EXCL",
  "FB_SB_SYNC_RR",
  "RD_SB_BCCH",
  "RD_BCCH_P",
  "RD_FB_SB_P",
  "RD_SB_P",
  "RD_SB_BCCH_P",
  "RD_BCCH_RR",
  "RD_BCCH_P_RR",
  "???"};
#endif /* !NTRACE && TRACING */
LOCAL const UBYTE unc_values[] =
{
  2, 4, 8, 12, 16, 22, 30, NOT_PRESENT_8BIT
};
typedef enum
{
  PRO_STA_SYNC=1,
  PRO_STA_BCCH=2
}T_PRO_STA_TYPE;
/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/
LOCAL UBYTE   nc_remove_channel_from_ba_list (USHORT                      index);
LOCAL UBYTE   nc_is_in_ncell_list            (USHORT                      channel,
                                              T_MPH_NEIGHBOURCELL_REQ   * ncell_list);
LOCAL void    nc_process_status              (void);
LOCAL void    nc_process_status_sync         (void);
LOCAL void    nc_process_status_last_bsic    (void);
LOCAL void    nc_process_status_bcch         (void);
LOCAL BOOL    nc_ncell_in_plmn_permitted     (UBYTE                       bsic);
LOCAL void    nc_find_cells                  (USHORT                    * c_found,
                                              USHORT                      max,
                                              UBYTE                       limitation,
                                              UBYTE                       min_rxlev);
LOCAL void    nc_store_rxlev                 (T_MPHC_RXLEV_PERIODIC_IND * report);
LOCAL void    nc_rank_ncells                 (void);
LOCAL void    nc_check_status                (UBYTE start_list);
LOCAL void    nc_build_rr_report_dedi        (T_MPH_MEASUREMENT_IND     * rr_report);

LOCAL void    nc_store_dedicated             (T_MPHC_MEAS_REPORT        * report);
LOCAL UBYTE   nc_convert_quality             (USHORT                      errors,
                                              USHORT                      total);
LOCAL void    nc_store_bcch                  (T_MPHC_DATA_IND           * data_ind,
                                              USHORT                      index,
                                              UBYTE                       which);
LOCAL void    nc_stop_bcch                   (USHORT                      index,
                                              UBYTE                       new_status);
LOCAL void    nc_stop_sync                   (USHORT                      index,
                                              UBYTE                       new_status);
LOCAL void    nc_clean_store_bcch            (USHORT                      index);
LOCAL UBYTE   nc_check_bsic                  (USHORT                      index,
                                              UBYTE                       bsic);
#if 0
#if !defined(NTRACE) && defined(TRACING)
LOCAL UBYTE   nc_get_status                  (USHORT                      index);
#endif  /* !NTRACE && TRACING */
#endif /* 0|1 */
LOCAL void    nc_restart_bcch                (USHORT                      index);
LOCAL void    nc_set_fb_sb_sync_initial      (USHORT                      index);
LOCAL void    nc_set_fb_sb_failed            (USHORT                      index,
                                              UBYTE                       c_sync);
LOCAL UBYTE   nc_sys_info_78_required        (USHORT                      index);
LOCAL void    nc_sync_failed_attempt         (USHORT                      index);
LOCAL void    nc_sync_failed_attempt_dedicated(USHORT                     index);
LOCAL void    nc_build_rr_report             (T_MPH_MEASUREMENT_IND*      rr_report);
LOCAL void    nc_release_bcch                (void);
LOCAL void    nc_find_list_cells             (void);
LOCAL void    nc_ncell_list_req              (void);
LOCAL void    nc_start_eotd_confirm          (void);
LOCAL void    nc_process_pos_req             (T_MPH_NCELL_POS_REQ*        pos_req);
LOCAL void    nc_check_sync_ind_eotd         (T_MPHC_NCELL_SYNC_IND*      sync_ind,
                                              USHORT                      arfcn);
LOCAL void    nc_store_eotd                  (T_eotd_sc_res*              p_res,
                                              T_MPHC_NCELL_SYNC_IND*      sync_ind,
                                              USHORT                      arfcn);
LOCAL void    nc_sync_ind_last_bsic_req      (T_MPHC_NCELL_SYNC_IND*      sync_ind,
                                              USHORT                      index,
                                              USHORT                      arfcn,
                                              UBYTE                       bsic);
LOCAL void    nc_sync_ind_ncell              (T_MPHC_NCELL_SYNC_IND*      sync_ind,
                                              USHORT                      index,
                                              USHORT                      arfcn,
                                              UBYTE                       bsic);
LOCAL void    nc_clear_last_bsic             (void);
#if 0
Symbol 'nc_stop_all_bcch(void)' 'nc_stop_all_sync(void)' and not referenced at the moment
LOCAL void    nc_stop_all_bcch               (void);
LOCAL void    nc_stop_all_sync               (void);
#endif  /* 0|1 */
LOCAL void    nc_stop_all                    (void);
LOCAL void    nc_stop_all_inactive           (void);
LOCAL void    nc_stop_if_active              (USHORT i);
LOCAL void    nc_ncell_list_req_pos          (T_ncell_list* p_ncell_list,
                                              T_ncell_eotd* asd_cell,
                                              USHORT arfcn);
LOCAL void    nc_get_timing_eotd             (T_ncell_list* p_ncell_list,
                                              T_ncell_eotd* asd_cell);

LOCAL BOOL    nc_is_sync                     (T_NC                      * p_ncell);
LOCAL void    nc_build_sync_req              (USHORT                      index);
LOCAL void    nc_init_pos_ind                (USHORT req_id);
LOCAL void    nc_enable_conf                 (void);
LOCAL void    nc_disable_conf                (BOOL bFree);
LOCAL void    nc_start_ncell_confirm         (void);
LOCAL void    nc_handle_new_strong_cells     (void);
LOCAL void    nc_check_new_strong_cell       (USHORT index, UBYTE o_1of6, UBYTE rxlev);
#ifdef GPRS
LOCAL void    nc_rxlev_sc_req                (UBYTE rxlev);
LOCAL void    nc_inform_grr_of_ncell         (USHORT index,
                                              UBYTE type);
LOCAL void    remove_ncell_and_inform_grr    (USHORT index);
LOCAL void    nc_sync_failed_gprs            (USHORT index);
LOCAL void    nc_check_bsic_pbcch(T_MPH_NEIGHBOURCELL_REQ* ncell_list);
LOCAL void    nc_process_status_sync_gprs(void);
#endif /* #ifdef GPRS */

#if defined(_SIMULATION_)
LOCAL void    trace_nc(void);
#endif /* _SIMULATION_ */
LOCAL void    nc_start_ncsync (void);
LOCAL void    nc_set_si4_si78                (USHORT                  index, 
                                              T_MPHC_NCELL_BCCH_IND  *data_ind,
                                              UBYTE                   sys_info_78);
LOCAL void nc_highst_field_strng             (USHORT                 *index, 
                                              T_PRO_STA_TYPE          nc_pro_sta_type);
#if defined (REL99) && defined (TI_PS_FF_EMR)
/*Functions related to EMR */
LOCAL void nc_fill_ncell_enh_meas_results( T_MPH_MEASUREMENT_IND *p_rep);
LOCAL void nc_sort_cells_into_bins(T_enh_bin *p_sbin,
                                   T_enh_bin *p_mbin,
                                   T_enh_bin *p_rbin);
LOCAL UBYTE get_band_index_for_emr (USHORT arfcn);
LOCAL void nc_sort_and_store_meas_results(T_NC *pcell, T_enh_bin *p_bin, UBYTE nbr_rpt, 
                                          T_MPH_MEASUREMENT_IND *p_rep);
LOCAL UBYTE nc_scale_rxlev (UBYTE scale, UBYTE no_of_cells, UBYTE *rx_lev);
LOCAL void nc_fill_serv_cell_enh_meas_results(T_MPH_MEASUREMENT_IND *p_rep);
LOCAL UBYTE nc_map_mean_bep(ULONG value);
LOCAL UBYTE nc_map_cv_bep(USHORT  value);
LOCAL void nc_process_status_sync_emr(void);
LOCAL void nc_add_to_rest(T_enh_bin *p_r, T_enh_bin *p_b, UBYTE start_index);
LOCAL void nc_sort_rest(T_MPH_MEASUREMENT_IND *p_rep, T_enh_bin *p_bin);
LOCAL void nc_swap_bin_content (T_enh_bin *p_bin, UBYTE i, UBYTE j );
#endif

#if defined (TI_PS_FF_RTD) && defined (REL99)
LOCAL UINT nc_sync_fn_offset_calculation(USHORT rtd_value);
LOCAL UINT nc_sync_time_alignment_calculation(USHORT rtd_value);
#endif
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
LOCAL BOOL nc_compare_bsic_std (UBYTE bsic_std);
#endif
/*==== CONSTANTS ==================================================*/
#define MAX_L1_SYNC_CNT         12
#define MAX_L1_BCCH_CNT          6
#define MAX_L1_SYNC_EOTD_CNT    14
#define MAX_RR_NCELL_CNT         6

#define ONLY_BCC                 7
#define ONLY_BSIC               63

#define NC_CHECK_OK             0
#define NC_CHECK_NCC_FAILED     1
#define NC_CHECK_BSIC_CHANGED   2

#define CHECK_FOR_ACQUIRE_AND_BCCH_AND_FAIL 0
#define CHECK_FOR_CONFIRM                   1

#define DONT_FREE_POS_IND                   0
#define FREE_POS_IND                        1

/*
 * Counter to realize a time interval of 10sec in terms of Control Multiframe
 *  Idle state       :  42 * 51 * 4.615ms     = 9885ms
 *  Dedicated State  :  42 * 104/2 * 4.615ms  = 10079ms
 */
#define TEN_SECONDS_NCSYNC        42

#define TEN_SECONDS        1
#define TWENTY_SECONDS     2
#define THIRTY_SECONDS     3
#define FIVE_MINUTES       30
#define C_INVALID_SYNC     (THIRTY_SECONDS+1)
/* +2 for rounding BCCH reading to confirmation boundary */
#define C_INVALID_BCCH     (FIVE_MINUTES+2+1)

#ifdef GPRS
#define GRR_SB_FOUND     (MPH_BSIC_UNKNOWN+1)
#define GRR_SB_NOT_FOUND  NOT_PRESENT_8BIT
#define GRR_SB_UNKNOWN    MPH_BSIC_UNKNOWN
#endif

/*==== VARIABLES ==================================================*/
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
UBYTE alr_multiband_std = NOT_PRESENT_8BIT;
#endif

/*==== FUNCTIONS ==================================================*/
#if defined (WIN32) && !defined (_SIMULATION_)
#define _SIMULATION_
#endif

#if defined (_SIMULATION_)
#define TRACING
#endif  /* _SIMULATION_ */

/*
 * some traces
 */
#if defined (TRACING)
#define ALR_TRACE_NC(a)  ALR_TRACE(a)
#define ALR_TRACE_NC_CNT(a,b)     TRACE_EVENT_P2 ("[%4u] c_sync %d", \
                                    ARFCN_TO_G23(a)&ARFCN_MASK, b)
#define ALR_TRACE_NC_BSIC_CNF(a)  TRACE_EVENT_P1 ("[%4u] NC BSIC CNF passed", \
                                    ARFCN_TO_G23(a)&ARFCN_MASK)
#define ALR_TRACE_NC_SB_FAILED(a) TRACE_EVENT_P1 ("[%4u] NCELL SB IND failed", \
                                    ARFCN_TO_G23(a)&ARFCN_MASK)
#define ALR_TRACE_NC_BSIC_REQ(a)  TRACE_EVENT_P4 ("[%4u] NC BSIC REQ  %d %d %d", \
                                    a&ARFCN_MASK, GET_STATE (STATE_NC), \
                                    alr_data->nc_data.cell[LAST_BSIC_REQ].status, \
                                    alr_data->nc_data.c_bcch_req)
#define ALR_TRACE_NC_BCCH_OK(f,m)  TRACE_EVENT_P2 ("[%4u] NCELL BCCH IND passed msg:%d", \
                                    f&ARFCN_MASK,m)
#define ALR_TRACE_NC_SB_IND_PASSED(f) TRACE_EVENT_P1 ("[%4u] NCELL SB IND passed", \
                                        f&ARFCN_MASK)
#define ALR_TRACE_NC_RESELECT(a)   TRACE_EVENT_P1 ("[%4u] start reselect", a&ARFCN_MASK)
#define ALR_TRACE_NC_FN_TA(_i,f,t)  TRACE_EVENT_P3 ("i %d, fn %d, ta %d ", _i, f, t)
#if 0
#define GET_NC_STATE(i)             nc_get_status(i)
#endif /* 0|1 */
#if defined(_SIMULATION_)
#define ALR_TRACE_ALL_NC()          trace_nc()
#else /* _SIMULATION_ */
#define ALR_TRACE_ALL_NC()
#endif /* _SIMULATION_ */
#else /* TRACING */
#define ALR_TRACE_NC(a)
#define ALR_TRACE_NC_CNT(a,b)
#define ALR_TRACE_NC_BSIC_CNF(a)
#define ALR_TRACE_NC_SB_FAILED(a)
#define ALR_TRACE_NC_BSIC_REQ(a)
#define ALR_TRACE_NC_BCCH_OK(f,m)
#define ALR_TRACE_NC_SB_IND_PASSED(f)
#define ALR_TRACE_NC_RESELECT(a)
#define ALR_TRACE_NC_FN_TA(i,f,t)
#define GET_NC_STATE(i)                 alr_data->nc_data.cell[i].status
#define ALR_TRACE_ALL_NC()
#undef  TRACE_EVENT
#undef  TRACE_EVENT_P1
#undef  TRACE_EVENT_P2
#undef  TRACE_EVENT_P3
#undef  TRACE_EVENT_P4
#undef  TRACE_EVENT_P5
#undef  TRACE_EVENT_P6
#undef  TRACE_EVENT_P7
#undef  TRACE_EVENT_P8
#undef  TRACE_EVENT_P9
#define TRACE_EVENT(s)
#define TRACE_EVENT_P1(s,a)
#define TRACE_EVENT_P2(s,a,b)
#define TRACE_EVENT_P3(s,a,b,c)
#define TRACE_EVENT_P4(s,a,b,c,d)
#define TRACE_EVENT_P5(s,a,b,c,d,e)
#define TRACE_EVENT_P6(s,a,b,c,d,e,f)
#define TRACE_EVENT_P7(s,a,b,c,d,e,f,g)
#define TRACE_EVENT_P8(s,a,b,c,d,e,f,g,h)
#define TRACE_EVENT_P9(s,a,b,c,d,e,f,g,h,i)
#endif /* TRACING */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_init                    |
+--------------------------------------------------------------------+

  PURPOSE : Initialize Idle Neighbour Cell Process.

*/
GLOBAL void nc_init (void)
{
  GET_INSTANCE_DATA;

#if defined (TI_PS_FF_RTD) AND defined (REL99)
  UBYTE i;
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */ 

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  alr_multiband_std                            = NOT_PRESENT_8BIT;
#endif

  nc_stop_all();
  alr_data->nc_data.c_ba_arfcn                 = 0;
  alr_data->nc_data.ba_id                      = ALR_BA_LOW;
  alr_data->nc_data.sc_included                = FALSE;
  alr_data->nc_data.cell[LAST_BSIC_REQ].status = IDLE;
  alr_data->nc_data.cell[LAST_BSIC_REQ].ba_arfcn = NOT_PRESENT_16BIT;
  alr_data->nc_data.c_reports                  = (UBYTE)-1;
  alr_data->nc_data.cr_cell.ba_arfcn           = NOT_PRESENT_16BIT;
  alr_data->nc_data.tim_state                  = NC_TIM_STOPPED;
  alr_data->nc_data.c_nc_timer                 = 0;
  if (alr_data->nc_data.ppos_ind NEQ NULL)
  {
    PFREE(alr_data->nc_data.ppos_ind);
    alr_data->nc_data.ppos_ind = NULL;
  }
  if (alr_data->nc_data.ppos_req NEQ NULL)
  {
    PFREE(alr_data->nc_data.ppos_req);
    alr_data->nc_data.ppos_req = NULL;
  }
  
  /* Initialize the counter for realizing 10sec  timer  value */
  alr_data->nc_data.c_ncsync_tim = 0;
  alr_data->nc_data.ncsync_start_tim = 0;
  
  SET_STATE(STATE_NC_PROC, NC_ACQUIRE);
  SET_STATE(STATE_NC, NC_NULL);
#if defined (REL99) && defined (TI_PS_FF_EMR)
  alr_data->nc_data.si2_count = NOT_PRESENT_8BIT;
  alr_data->nc_data.emr_data.rep_type = REP_TYPE_NORM;
#endif

#if defined (TI_PS_FF_RTD) AND defined (REL99)
  alr_data->nc_data.nxt_ind = 0;
  for(i = 0;i < MAX_NEIGHBOURCELLS;i++)
  {
    alr_data->nc_data.cell[i].frame_offset=0 ;
    alr_data->nc_data.cell[i].time_align=0;
    alr_data->nc_data.cell[i].c_sync_req=0;
  } 
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */

}

#ifdef GPRS
/*
+----------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_GPRS                     |
| STATE   : code                ROUTINE : nc_start_pbcch|
+----------------------------------------------------------------------+

  PURPOSE :Checks whether pbcch is available and sets a relevant state.

*/

GLOBAL void nc_start_pbcch(void)
{
  GET_INSTANCE_DATA;
  if(alr_data->gprs_data.ptm AND
     alr_data->gprs_data.pbcch)
  {
    SET_STATE(STATE_NC, NC_PTM_PBCCH);
  }
  else if(alr_data->gprs_data.pbcch)
  {
    SET_STATE(STATE_NC, NC_PIM_PBCCH);
  }

  nc_enable_conf();
  nc_process_status();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_GPRS                   |
| STATE   : code                ROUTINE : gprs_check_bsic_pbcch      |
+--------------------------------------------------------------------+

  PURPOSE :

*/
LOCAL void nc_check_bsic_pbcch(T_MPH_NEIGHBOURCELL_REQ* ncell_list)
{
  GET_INSTANCE_DATA;
  TRACE_EVENT("gprs_alr_check_bsic_pbcch");
  if(!alr_data->gprs_data.pbcch) return;

  if(ncell_list->sync_only EQ SYNC_LIST)
  {

    USHORT  n_rr;     /* index of RR neighbor cell list */
    USHORT  n_alr;    /* index of ALR neighbor cell list */
    UBYTE   last_alr; /* last index of ALR neighbor cell list */
    USHORT  arfcn;
    TRACE_EVENT("pbcch/check bsic");

    /*
     * remove all channels which are not longer member of the
     * neighbour cell list.
     */
    last_alr = alr_data->nc_data.c_ba_arfcn;  /* current last index */
    for (n_alr = 0; n_alr < last_alr; n_alr++)
    {
      arfcn = alr_data->nc_data.cell[n_alr].ba_arfcn;
      if (!nc_is_in_ncell_list (arfcn, ncell_list))
      /* AND
          (arfcn NEQ alr_data->serving_cell))*/
      {
        switch(alr_data->nc_data.cell[n_alr].status)
        {
          case READ_FB_SB:
          case READ_FB_SB_PENDING:
          case READ_SB_PENDING:
          case READ_SB:
          case READ_SB_BCCH: /* in case we come from BCCH */
          case INACTIVE: /* in case we returned from DEDIC */
          case IDLE:     /* in case we returned from DEDIC */
          case EXCLUDED: /* in case we returned from DEDIC */
            TRACE_EVENT_P1("rem cell i%d",n_alr);
            remove_ncell_and_inform_grr(n_alr);
            last_alr = alr_data->nc_data.c_ba_arfcn;
            n_alr--;
            break;
          case FB_SB_SYNC_RR_NOT_INFORMED:
          case READ_BCCH_RR_NOT_INFORMED:
          case READ_BCCH:
          case FB_SB_SYNC:
          case IDLE_SYNC:
          case FB_SB_FAILED:
            /*state change will be done with check_status*/
            TRACE_EVENT_P1("i%d reset 1o6", n_alr);
            alr_data->nc_data.cell[n_alr].one_of_six    = FALSE;
            alr_data->nc_data.cell[n_alr].one_of_twelve = FALSE;
            break;
          default:
            TRACE_EVENT_P3("i%d,arfcn:%d illegal status %d",n_alr,
                              alr_data->nc_data.cell[n_alr].ba_arfcn,
                              alr_data->nc_data.cell[n_alr].status);
            break;
        }
      }
      else
      { /* ncell is in both lists */
        switch(alr_data->nc_data.cell[n_alr].status)
        {
          case READ_FB_SB:
          /* case READ_FB_SB_PENDING:
            *case READ_SB_PENDING: 
            */
          case READ_SB:
            break;
          case READ_SB_BCCH: /* in case we come from BCCH */
            nc_set_status(n_alr, READ_SB);
            break;
          case INACTIVE: /* in case we returned from DEDIC */
          case IDLE:     /* in case we returned from DEDIC */
          case EXCLUDED: /* in case we returned from DEDIC */
            nc_set_status (n_alr, INACTIVE);
            /* ALR does not receive measurements */
            /* set them to idle */
            nc_set_status (n_alr, IDLE);
            /* and set it to one_of_six , the rest is done by check_status */
            nc_set_status(n_alr, READ_FB_SB);
            break;
          case FB_SB_SYNC_RR_NOT_INFORMED:
          case READ_BCCH_RR_NOT_INFORMED:
          case READ_BCCH:
            nc_set_status(n_alr, FB_SB_SYNC);
            break;
          default:
            break;
        }
        alr_data->nc_data.cell[n_alr].one_of_six    = FALSE;
        alr_data->nc_data.cell[n_alr].one_of_twelve = TRUE;
      }
    }

    /*
     * add all new channels.
     */
    for (n_rr = 0;
         (n_rr < MAX_NEIGHBOURCELLS) AND (last_alr < BA_LIST_SIZE-1);
         n_rr++)
    {
      arfcn = ncell_list->arfcn[n_rr];
      if (arfcn EQ NOT_PRESENT_16BIT)
        break;  /* no more entries in the RR ncell_list */

      n_alr = nc_get_index (arfcn);

      if (((n_alr EQ NOT_PRESENT_16BIT) OR (n_alr EQ LAST_BSIC_REQ)) AND
          (arfcn NEQ alr_data->serving_cell))
      {
        T_NC* pcell = &alr_data->nc_data.cell[last_alr];

#if defined(TRACING)
        TRACE_EVENT_P2 ("NC%u[%u] add", last_alr, arfcn);
#endif  /* TRACING */
        pcell->ba_arfcn = arfcn;
        nc_set_status (last_alr, INACTIVE);
        /* ALR does not receive measurements */
        /* set them to idle */
        nc_set_status (last_alr, IDLE);
        /* and set it to one_of_six , the rest is done by check_status */
        alr_data->nc_data.cell[last_alr].one_of_six    = FALSE;
        alr_data->nc_data.cell[last_alr].one_of_twelve = TRUE;
        alr_data->nc_data.cell[last_alr].ba_status = IN_BA;
        last_alr++; /* increment last index */
      }
    }
    alr_data->nc_data.c_ba_arfcn = last_alr;  /* store new last index */
    ALR_TRACE_ALL_NC ();

    {
      UBYTE six=0;
      last_alr = alr_data->nc_data.c_ba_arfcn;  /* current last index */
      for (n_alr = 0; n_alr < last_alr; n_alr++)
      {
        six += alr_data->nc_data.cell[n_alr].one_of_six;
      }
      /*XXX change this to handle 12 ncells */
      if(six > 6)
      {
        TRACE_ERROR("more then six candidates!");
      }
    }
#if 0 /*only for easing debugging */
    /*
     * Reorder the entries. The goal is that the propably strongest neigbour
     * cells
     * are measured in the first measurement period of a subsequent
     * MPHC_RXLEV_PERIODIC_REQ/IND. The appropriate arfcn's are the first
     * ones in
     * ncell_list->arfcn
     */
    {
      T_NC temp, *prr, *palr;

      for (n_rr = 0; n_rr < 7; n_rr++)
      {
        if(ncell_list->arfcn[n_rr] NEQ NOT_PRESENT_16BIT)
          n_alr = nc_get_index (ncell_list->arfcn[n_rr]);
        else
          break;

        if ( ((n_alr NEQ NOT_PRESENT_16BIT) OR (n_alr NEQ LAST_BSIC_REQ)) AND
             n_rr NEQ n_alr)
        {
          palr = &(alr_data->nc_data.cell[n_alr]);
          prr  = &(alr_data->nc_data.cell[n_rr]);
          memcpy(&temp, palr,  sizeof(T_NC));
          memcpy(palr,  prr,   sizeof(T_NC));
          memcpy(prr,   &temp, sizeof(T_NC));
        /*TRACE_EVENT_P4("reordered NC%u[%u] and NC%u[%u]",
                          n_rr,
                          ncell_list->arfcn[n_rr],
                          n_alr, ncell_list->arfcn[n_alr]);*/
        }
      }
    }
#endif
    nc_check_status(CHECK_FOR_ACQUIRE_AND_BCCH_AND_FAIL);

    if(alr_data->nc_sync_with_grr)
    {
      TRACE_EVENT("sync_with_grr");
      last_alr = alr_data->nc_data.c_ba_arfcn;  /* current last index */
      for (n_alr = 0; n_alr < last_alr; n_alr++)
      {
         if(alr_data->nc_data.cell[n_alr].one_of_twelve EQ FALSE)
         {
            TRACE_EVENT_P1("Cell %d not in requested list",alr_data->nc_data.cell[n_alr].ba_arfcn);
            continue;
         }
          
        switch(alr_data->nc_data.cell[n_alr].status)
        {
          case IDLE_SYNC:
          case FB_SB_SYNC:
          case READ_SB:
          case READ_SB_PENDING:
          case READ_FB_SB_PENDING:
            TRACE_EVENT_P1("sync %d",alr_data->nc_data.cell[n_alr].ba_arfcn);
            nc_inform_grr_of_ncell(n_alr, GRR_SB_FOUND);
            break;
          case FB_SB_FAILED:
          case EXCLUDED:
          case INACTIVE:
          case IDLE:
            TRACE_EVENT_P1("sync %d",alr_data->nc_data.cell[n_alr].ba_arfcn);
            nc_inform_grr_of_ncell(n_alr, GRR_SB_NOT_FOUND);
            break;
          /* READ_FB_SB ->the cell will be synchronized next
             and a result will be returnded anyway */
          default:
            break;
        }
      }
      alr_data->nc_sync_with_grr=FALSE;
    }

    switch(GET_STATE(STATE_NC))
    {
      case NC_PIM_PBCCH:
      case NC_PTM_PBCCH:
        nc_process_status();
        break;
      default:
        break;
    }
  }
  else if(ncell_list->sync_only EQ RECONFIRM_SYNC_LIST)
  {
     nc_ncsync_tim_expiry();
  }

}

#endif
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_stop                    |
+--------------------------------------------------------------------+

  PURPOSE : stop the neighbourcell process. Is called by main process
            if a recovery with power measurements is started.

*/

GLOBAL void nc_stop (void)
{
  /*
   * clean or neighbourcells of the BA and LAST_BSIC_REQ list if needed
   */
  nc_stop_all_inactive();

  /*
   * initialize neighbourcell process and stops timer
   */
  nc_init();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_is_in_ba                |
+--------------------------------------------------------------------+

  PURPOSE : Check whether given ARFCN belongs to BA list, Serving cell.

*/

GLOBAL BOOL nc_is_in_ba(USHORT arfcn)
{
  GET_INSTANCE_DATA;
  UBYTE i;
  T_NC*   pcell;

  for (i = 0, pcell = &alr_data->nc_data.cell[0]; i < alr_data->nc_data.c_ba_arfcn; i++, pcell++ )
  {
    
    if (arfcn EQ pcell->ba_arfcn OR
        arfcn EQ alr_data->serving_cell)
    {
      return TRUE;
    }
  }
  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_stop_all_inactive       |
+--------------------------------------------------------------------+

  PURPOSE : This stops all activities to bring a channel to state NC_NULL.

*/

LOCAL void nc_stop_all_inactive (void)
{
  GET_INSTANCE_DATA;
  UBYTE i, stopped_sync=0, stopped_bcch=0;
  /*
   * depending on the current status
   */
  for (i = 0; i < alr_data->nc_data.c_ba_arfcn; i++)
  {
    switch (alr_data->nc_data.cell[i].status)
    {
      case READ_FB_SB_PENDING:
      case READ_SB_PENDING:
      case READ_SB_BCCH_PENDING:
        /*
         * stop the current running request for synchronisation in layer 1.
         */
        nc_set_status (i, INACTIVE);
        stopped_sync++;
        break;

      case READ_BCCH_PENDING:
      case READ_BCCH_PENDING_RR_NOT_INFORMED:
        /*
         * stop the current running request for BCCH reading in layer 1.
         * If sys info 4 is already stored, but sys info 7 or 8 is requested
         * clear also a stored BCCH message.
         */
        nc_set_status (i, INACTIVE);
        stopped_bcch++;
        break;
      default:
        /*
         * sys infos are stored, but RR is still not informed.
         * Then clean the stored message
         */
        nc_set_status (i, INACTIVE);
        break;
    }
  }
  switch (alr_data->nc_data.cell[LAST_BSIC_REQ].status)
  {
    case READ_FB_SB_PENDING:
      nc_set_status (LAST_BSIC_REQ, IDLE);
      alr_data->nc_data.cell[LAST_BSIC_REQ].ba_arfcn = NOT_PRESENT_16BIT;
      stopped_sync++;
      break;
    case READ_BCCH_PENDING:
      nc_set_status (LAST_BSIC_REQ, IDLE);
      alr_data->nc_data.cell[LAST_BSIC_REQ].ba_arfcn = NOT_PRESENT_16BIT;
      stopped_bcch++;
      break;
  }

  /*send to L1*/
  if (stopped_sync > 0)
  { /* stop all synchronizations */
    PALLOC (stop_req, MPHC_STOP_NCELL_SYNC_REQ);
    stop_req->radio_freq_array_size = MAX_L1_SYNC_CNT;
    PSENDX(L1, stop_req);
  }
  if (stopped_bcch > 0)
  { /* stop all bcch */
    PALLOC (stop_req, MPHC_STOP_NCELL_BCCH_REQ);
    stop_req->radio_freq_array_size = MAX_L1_BCCH_CNT;
    PSENDX(L1, stop_req);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_stop_all                |
+--------------------------------------------------------------------+

  PURPOSE : This stops all activities to bring a channel
            its corresping READ_ state.

*/

LOCAL void nc_stop_all (void)
{
  GET_INSTANCE_DATA;
  UBYTE i, stopped_sync=0, stopped_bcch=0;
  /*
   * depending on the current status
   */
  for (i = 0; i < alr_data->nc_data.c_ba_arfcn; i++)
  {
    switch (alr_data->nc_data.cell[i].status)
    {
      case READ_FB_SB_PENDING:
        nc_set_status (i, READ_FB_SB);
        stopped_sync++;
        break;
      case READ_SB_PENDING:
        nc_set_status (i, READ_SB);
        stopped_sync++;
        break;
      case READ_SB_BCCH_PENDING:
        nc_set_status (i, READ_SB_BCCH);
        stopped_sync++;
        break;
      case READ_BCCH_PENDING:
        nc_set_status (i, READ_BCCH);
        stopped_bcch++;
        break;
      case READ_BCCH_PENDING_RR_NOT_INFORMED:
        nc_set_status (i, READ_BCCH_RR_NOT_INFORMED);
        stopped_bcch++;
        break;
      case FB_SB_SYNC_RR_NOT_INFORMED:
        nc_set_status (i, READ_BCCH_RR_NOT_INFORMED);
        break;
    }
  }
  switch (alr_data->nc_data.cell[LAST_BSIC_REQ].status)
  {
    case READ_FB_SB_PENDING:
      nc_set_status (LAST_BSIC_REQ, READ_FB_SB);
      stopped_sync++;
      break;
    case READ_BCCH_PENDING:
      nc_set_status (LAST_BSIC_REQ, READ_BCCH);
      stopped_bcch++;
      break;
  }
  /*send to L1*/
  if ( stopped_sync > 0 OR
       alr_data->nc_data.c_sync_req > 0 OR
       (alr_data->nc_data.eotd_avail AND GET_STATE(STATE_NC_PROC) EQ NC_CONFIRM))
  { /* stop all synchronizations if waiting for any NC synchronization (or 2nd SC in EOTD case) */
    PALLOC (stop_req, MPHC_STOP_NCELL_SYNC_REQ);
    stop_req->radio_freq_array_size = MAX_L1_SYNC_CNT;
    PSENDX(L1, stop_req);
    alr_data->nc_data.c_sync_req=0;
    if (alr_data->nc_data.eotd_avail)
      SET_STATE(STATE_NC_PROC, NC_ACQUIRE);
  }
  if(stopped_bcch > 0)
  { /* stop all bcch */
    PALLOC (stop_req, MPHC_STOP_NCELL_BCCH_REQ);
    stop_req->radio_freq_array_size = MAX_L1_BCCH_CNT;
    PSENDX(L1, stop_req);
    alr_data->nc_data.c_bcch_req=0;
  }
}

#if 0
Symbol 'nc_stop_all_bcch(void)' not referenced at the moment
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_stop_all_sync           |
+--------------------------------------------------------------------+

  PURPOSE : This stops all sync activities.

*/
LOCAL void nc_stop_all_sync (void)
{
  UBYTE i, stopped_sync=0;
  /*
   * depending on the current status
   */
  for (i = 0; i < alr_data->nc_data.c_ba_arfcn; i++)
  {
    switch (alr_data->nc_data.cell[i].status)
    {
      case READ_FB_SB_PENDING:
        nc_set_status (i, READ_FB_SB);
        stopped_sync++;
        break;
      case READ_SB_PENDING:
        nc_set_status (i, READ_SB);
        stopped_sync++;
        break;
      case READ_SB_BCCH_PENDING:
        nc_set_status (i, READ_SB_BCCH);
        stopped_sync++;
        break;
    }
  }
  switch (alr_data->nc_data.cell[LAST_BSIC_REQ].status)
  {
    case READ_FB_SB_PENDING:
      nc_set_status (LAST_BSIC_REQ, READ_FB_SB);
      stopped_sync++;
      break;
  }
  /*send to L1*/
  if (stopped_sync > 0)
  { /* stop all synchronizations */
    PALLOC (stop_req, MPHC_STOP_NCELL_SYNC_REQ);
    stop_req->radio_freq_array_size = MAX_L1_SYNC_CNT;
    PSENDX(L1, stop_req);
    alr_data->nc_data.c_sync_req=0;
  }
}
#endif  /* 0|1 */

#if 0
Symbol 'nc_stop_all_bcch(void)' not referenced at the moment
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_stop_all_bcch           |
+--------------------------------------------------------------------+

  PURPOSE : This stops all activities to bring a channel to state NC_NULL.

*/
LOCAL void nc_stop_all_bcch (void)
{
  UBYTE i, stopped_bcch=0;
  /*
   * depending on the current status
   */
  for (i = 0; i < alr_data->nc_data.c_ba_arfcn; i++)
  {
    switch (alr_data->nc_data.cell[i].status)
    {
      case READ_BCCH_PENDING_RR_NOT_INFORMED:
        nc_set_status (i, READ_BCCH_RR_NOT_INFORMED);
        stopped_bcch++;
        break;
      case READ_BCCH_PENDING:
        nc_set_status (i, READ_BCCH);
        stopped_bcch++;
        break;
    }
  }
  switch (alr_data->nc_data.cell[LAST_BSIC_REQ].status)
  {
    case READ_BCCH_PENDING:
      nc_set_status (LAST_BSIC_REQ, READ_BCCH);
      stopped_bcch++;
      break;
  }
  /*send to L1*/
  if (stopped_bcch > 0)
  { /* stop all bcch */
    PALLOC (stop_req, MPHC_STOP_NCELL_BCCH_REQ);
    stop_req->radio_freq_array_size = MAX_L1_BCCH_CNT;
    PSENDX(L1, stop_req);
    alr_data->nc_data.c_bcch_req=0;
  }
}
#endif  /* 0|1 */

LOCAL void nc_clear_last_bsic(void)
{
  GET_INSTANCE_DATA;
  switch (alr_data->nc_data.cell[LAST_BSIC_REQ].status)
  {
    case READ_BCCH_PENDING:
      nc_stop_bcch (LAST_BSIC_REQ, IDLE);
      break;
    case READ_FB_SB_PENDING:
      nc_stop_sync (LAST_BSIC_REQ, IDLE);
      break;
  }

  nc_set_status(LAST_BSIC_REQ, IDLE);
  alr_data->nc_data.cell[LAST_BSIC_REQ].ba_arfcn = NOT_PRESENT_16BIT;

}

LOCAL void nc_stop_if_active(USHORT i)
{
  GET_INSTANCE_DATA;
  /*
   * the requested channel number is member of the BA list.
   * clear a request to layer 1 if needed.
   */
  switch(alr_data->nc_data.cell[i].status)
  {
    case READ_SB_PENDING:
      nc_stop_sync (i, READ_SB);
      break;
    case READ_SB_BCCH_PENDING:
      nc_stop_sync (i, READ_SB_BCCH);
      break;
    case READ_FB_SB_PENDING:
      nc_stop_sync (i, READ_FB_SB);
      break;
    case READ_BCCH_PENDING:
      nc_stop_bcch (i, READ_BCCH);
      break;
    case READ_BCCH_PENDING_RR_NOT_INFORMED:
      nc_stop_bcch (i, READ_BCCH_RR_NOT_INFORMED);
      break;
    default:
      break;
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_ncell_list              |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a new neighbourcell list from RR.

*/

GLOBAL void nc_ncell_list (T_MPH_NEIGHBOURCELL_REQ * mph_neighbourcell_req)
{
  GET_INSTANCE_DATA;
  UBYTE i, j;
  switch (GET_STATE (STATE_NC))
  {
    case NC_DEDICATED:
      {
        UBYTE  c_ba_arfcn;
        PALLOC (upd_dedi, MPHC_UPDATE_BA_LIST);

        /*
         * mix new list with old list
         */
        nc_update_ba_list (alr_data->serving_cell,
                           mph_neighbourcell_req);

        /*
         * create new list for layer 1
         */
        c_ba_arfcn = alr_data->nc_data.c_ba_arfcn;
        for (i = 0, j = 0; i < c_ba_arfcn; i++)
        {
          if (alr_data->nc_data.cell[i].ba_status EQ IN_BA)
            upd_dedi->chan_list.radio_freq[j++] =
              ARFCN_TO_L1(alr_data->nc_data.cell[i].ba_arfcn);
        }

        /*
         * send new list to layer 1
         */
        upd_dedi->num_of_chans = j;

        alr_data->nc_data.ba_id = ALR_ALLOCATE_NEW_BA ( alr_data->nc_data.ba_id );
        upd_dedi->ba_id = alr_data->nc_data.ba_id;

        upd_dedi->pwrc = alr_data->nc_data.pwrc;
        upd_dedi->dtx_allowed = alr_data->nc_data.dtx;

        alr_data->nc_data.update = TRUE;
        ma_nc_update_ba_list (upd_dedi);

      }
      break;
    case NC_CON_EST: /*for PBCCH just update the list, is checked in
                       nc_check_bsic_pbcch */
#ifdef GPRS
    case NC_PIM_PBCCH:
    case NC_PTM_PBCCH:
      nc_check_bsic_pbcch(mph_neighbourcell_req);
#endif
      break;
    default:
      /*
       * idle mode
       */
      ALR_TRACE_NC ("ncell req in idle");

      /*
       * mix new list with old list
       */
      nc_update_ba_list (alr_data->serving_cell, mph_neighbourcell_req);

      /*
       * configure layer 1 and start monitoring on new list
       */
      nc_start_monitoring();
      break;
  }
  return;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_start_monitoring        |
+--------------------------------------------------------------------+

  PURPOSE : Configure layer 1 with a new list for neighbourcell
            measurements.

*/

static const UBYTE NO_OF_REPORTS [8] =
{
  10, 7, 5, 4, 3, 3, 2, 2
};

static const UBYTE INIT_OF_C_REPORTS [8] =
{
  5, 5, 2, 2, 1, 1, 0, 0  /* empirical for FTA 20.7, 20.19, for TC 084, 047  */
};

static BOOL first_period = FALSE, first_l1_meas = FALSE;

GLOBAL void nc_start_monitoring (void)
{
  GET_INSTANCE_DATA;
  USHORT  index, prim_index;
  UBYTE   report_idx = alr_data->bs_pa_mfrms;
  UBYTE   c_ba_arfcn = alr_data->nc_data.c_ba_arfcn;
  PALLOC (update, MPHC_RXLEV_PERIODIC_REQ);

  ALR_TRACE_NC ("nc_start_mon");

  first_period = first_l1_meas = TRUE;

  alr_data->nc_data.max_reports = NO_OF_REPORTS [report_idx];
  alr_data->nc_data.fn_offset   = (report_idx+2) * NO_OF_REPORTS[report_idx] * 51;

  if (alr_data->nc_data.c_reports EQ (UBYTE)-1) /* initial value */
    alr_data->nc_data.c_reports = INIT_OF_C_REPORTS [report_idx];
  else  /* don´t touch c_reports except for max_reports is less */
    if (alr_data->nc_data.c_reports >= alr_data->nc_data.max_reports)
      alr_data->nc_data.c_reports = alr_data->nc_data.max_reports - 1;
  index = nc_get_index(alr_data->serving_cell);
  if (index NEQ NOT_PRESENT_16BIT)
    alr_data->nc_data.cell[index].c_rxlev = NOT_PRESENT_8BIT;

  /*
   * for all neighbourcells: convert channel number
   * and fill the primitive to layer 1.
   */
  for (index = 0, prim_index = 0; index < c_ba_arfcn; index++)
  {
    if (alr_data->nc_data.cell[index].ba_status EQ IN_BA)
      update->chan_list.radio_freq[prim_index++] =
        ARFCN_TO_L1(alr_data->nc_data.cell[index].ba_arfcn);
  }

  /*
   * set number of channels and band id
   */
  update->num_of_chans = (UBYTE)prim_index;
  alr_data->nc_data.ba_id = ALR_ALLOCATE_NEW_BA ( alr_data->nc_data.ba_id );
  update->ba_id = alr_data->nc_data.ba_id;
  update->next_radio_freq_measured = 0;

#if defined(_SIMULATION_)
  {
    char buf[80], o;
    o = sprintf (buf, "RXLEV_PERIODIC: c=%d id=%02x:", update->num_of_chans, update->ba_id);
    for (index=0; index < update->num_of_chans; index++)
    {
      if (o > 80-6)
      {
        TRACE_EVENT (buf);
        o = 0;
      }
      o += sprintf (buf+o, "%u,", update->chan_list.radio_freq[index]);
    }
    buf[o-1]=0;
    TRACE_EVENT (buf);
  }
#endif  /* _SIMULATION_ */

  SET_STATE (STATE_NC, NC_IDLE);
/*
  TRACE_EVENT_P2("glob reps: %d max_report: %d",
    alr_data->nc_data.c_reports, alr_data->nc_data.max_reports);
*/

  /*
   * configure layer 1.
   */
  ma_nc_rxlev_periodic_req(update);

  nc_enable_conf();
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_enable_conf             |
+--------------------------------------------------------------------+

  PURPOSE : Maintains what happens after 10 seconds interval.
            restarts a pending ncell confirmation
*/
LOCAL void nc_enable_conf(void)
{
  GET_INSTANCE_DATA;
  UBYTE tim_state = alr_data->nc_data.tim_state;
  UBYTE st        = GET_STATE(STATE_NC);

  TRACE_EVENT("nc_enable_conf()");
  alr_data->nc_data.tim_state = NC_CONF_ENABLED;
  SET_STATE(STATE_NC_PROC, NC_ACQUIRE);
  switch(tim_state)
  {
    case NC_TIM_STOPPED:
      if (alr_data->nc_data.ppos_req)
        nc_process_pos_req (alr_data->nc_data.ppos_req);
      /* Load the 10sec timer counter  variable */
      nc_start_ncsync();
      break;
    case NC_CONF_PENDING:
      if (alr_data->nc_data.ppos_req)
        nc_process_pos_req (alr_data->nc_data.ppos_req);
      else
        nc_start_ncell_confirm();
      break;
    case NC_CONF_DISABLED:
      if (alr_data->nc_data.ppos_req)
        nc_process_pos_req (alr_data->nc_data.ppos_req);
      else if (alr_data->nc_data.c_nc_timer EQ 0)
        nc_start_ncell_confirm(); /* a confirmation is to restart */
      break;
    default:
      break;
  }
  

/*XXX add PBCCH here also */
#ifdef GPRS
  if ((st EQ NC_DEDICATED OR st EQ NC_PIM_PBCCH OR st EQ NC_PTM_PBCCH) AND
#else
  if ((st EQ NC_DEDICATED) AND
#endif
      alr_data->nc_data.c_nc_timer > TEN_SECONDS)
  {
    alr_data->nc_data.c_nc_timer = TEN_SECONDS;
  }
  else if (alr_data->nc_data.c_nc_timer EQ 0)
  {
    if( st EQ NC_DEDICATED OR alr_data->nc_data.eotd_avail
#ifdef GPRS
        OR st EQ NC_PIM_PBCCH OR st EQ NC_PTM_PBCCH
#endif
      )
      alr_data->nc_data.c_nc_timer    = TEN_SECONDS;
    else
      alr_data->nc_data.c_nc_timer    = THIRTY_SECONDS;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_disable_conf            |
+--------------------------------------------------------------------+

  PURPOSE : Maintains what happens after TIM_NCSYNC expiry.
            prevents ncell confirmation
*/
LOCAL void nc_disable_conf(BOOL bFree)
{
  GET_INSTANCE_DATA;
  TRACE_EVENT("nc_disable_conf()");
  switch(alr_data->nc_data.tim_state)
  {
    case NC_TIM_STOPPED:
    case NC_CONF_PENDING:
      TRACE_ERROR("nc_disable_conf() in unexpected state");
      break;
    default:
      break;
  }

  if(bFree AND (alr_data->nc_data.ppos_ind NEQ NULL))
  {
    PFREE(alr_data->nc_data.ppos_ind);
    alr_data->nc_data.ppos_ind = NULL;
  }

  if(GET_STATE(STATE_NC_PROC) EQ NC_CONFIRM)
  {
    /*
     * restart confirmation if it was aborted
     * after the first expiry after the POS_REQ is done.
     */
    alr_data->nc_data.c_nc_timer = 0;
  }

  SET_STATE(STATE_NC_PROC, NC_ACQUIRE);
  alr_data->nc_data.tim_state = NC_CONF_DISABLED;
}

LOCAL void nc_start_ncell_confirm(void)
{
  GET_INSTANCE_DATA;
  BOOL acquire = FALSE;

  switch(GET_STATE(STATE_NC))
  {
    case NC_DEDICATED:
    case NC_IDLE:
#ifdef GPRS
    case NC_PIM_PBCCH:
    case NC_PTM_PBCCH:
#endif
      if (alr_data->nc_data.eotd_avail AND
          alr_data->nc_data.c_nc_timer EQ 0)
      {
        alr_data->nc_data.c_nc_timer      = TEN_SECONDS;

        nc_stop_all();
        nc_check_status(CHECK_FOR_CONFIRM);
        nc_start_eotd_confirm();
      }
      else if ((alr_data->nc_data.c_nc_timer EQ 0) OR 
               (alr_data->nc_data.c_sync_intrupted EQ TRUE) )
      {
        /*
         * confirm ncells every thirty seconds in idle
         * or every 10 seconds in dedicated
         */
        if (GET_STATE(STATE_NC) EQ NC_DEDICATED
#ifdef GPRS
            OR GET_STATE(STATE_NC) EQ NC_PIM_PBCCH
            OR GET_STATE(STATE_NC) EQ NC_PTM_PBCCH
#endif
           )
          alr_data->nc_data.c_nc_timer    = TEN_SECONDS;
        else
          alr_data->nc_data.c_nc_timer    = THIRTY_SECONDS;
        nc_stop_all();
        nc_check_status(CHECK_FOR_CONFIRM);
        nc_ncell_list_req();
        /* if a list_req was send we are in NC_CONFIRM
           if no list_req was send we are in NC_ACQUIRE
           This is needed for PBCCH, because there are
           no measurements in ALR to trigger the initial
           synchronizations, so we have to check this here
           if no list_req was send, when a list_req was send
           then we continue with initial sync after the last
           sync_ind
         */
        if(GET_STATE(STATE_NC_PROC) EQ NC_ACQUIRE)
          acquire = TRUE;

      }
      else /* acquire ncells (initial or previous failed ones) or
            * read bcch
            */
      {
        acquire = TRUE;
      }

      if(acquire EQ TRUE)
      {
        SET_STATE(STATE_NC_PROC, NC_ACQUIRE);
        nc_check_status(CHECK_FOR_ACQUIRE_AND_BCCH_AND_FAIL);
        nc_process_status();
      }
      break;
    default:
      TRACE_ERROR("nc_ncsync_tim_expiry in wrong state");
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_start_ncsync              |
+--------------------------------------------------------------------+
  PURPOSE : Assigns/resets c_ncsync_tim counter
*/
LOCAL void  nc_start_ncsync (void)
{
  GET_INSTANCE_DATA;
  alr_data->nc_data.c_ncsync_tim = TEN_SECONDS_NCSYNC;
  TRACE_EVENT("NCSYNC counter loaded with 10 seconds");
  vsi_t_time(VSI_CALLER &alr_data->nc_data.ncsync_start_tim);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_ncsync_tim_expiry              |
+--------------------------------------------------------------------+
  PURPOSE : Called at the end of 10 seconds interval for NCELL synchronization

*/
GLOBAL void nc_ncsync_tim_expiry(void)
{
  GET_INSTANCE_DATA;
  UBYTE i;
  UBYTE c_ba_arfcn = alr_data->nc_data.c_ba_arfcn;
  T_NC* pcell;
  
  #ifdef GPRS
  if(!alr_data->gprs_data.pbcch)
  #endif
  {
    T_TIME tval;
    vsi_t_time(VSI_CALLER &tval);
    if(tval<alr_data->nc_data.ncsync_start_tim)
      TRACE_EVENT_P2("NCSYNC expiry at %d milliseconds, BS_PA_MFRMS = %d",
        ULONG_MAX-alr_data->nc_data.ncsync_start_tim+tval,
        alr_data->bs_pa_mfrms+2);
    else
      TRACE_EVENT_P2("NCSYNC expiry at %d milliseconds, BS_PA_MFRMS = %d",
        tval-alr_data->nc_data.ncsync_start_tim,
        alr_data->bs_pa_mfrms+2);
  }

  
  
  if (alr_data->nc_data.tim_state NEQ NC_TIM_STOPPED)
  {
    #ifdef GPRS
    if(!alr_data->gprs_data.pbcch)
    #endif
    {
      nc_start_ncsync();
    }

    if (alr_data->nc_data.c_nc_timer > 0)
      alr_data->nc_data.c_nc_timer--;
    TRACE_EVENT_P1("c_nc_timer %d", alr_data->nc_data.c_nc_timer);

    /* decrement counters */
    for (i=0,pcell = &alr_data->nc_data.cell[0]; i < c_ba_arfcn; i++,pcell++)
    {
      if (pcell->c_sync > 0 AND pcell->c_sync < C_INVALID_SYNC)
          pcell->c_sync--;
      if (pcell->c_bcch > 0 AND pcell->c_bcch < C_INVALID_BCCH)
        pcell->c_bcch--;

      if(pcell->ba_arfcn NEQ alr_data->serving_cell)
      {
        switch(pcell->ba_status)
        {
          case NOT_IN_BA_SHORT:
            pcell->ba_status = NOT_IN_BA_LONG;
#if defined(TRACING)
            TRACE_EVENT_P2 ("NC%u[%u] -> NOT_IN_BA_LONG", i, pcell->ba_arfcn);
#endif  /* TRACING */
            break;
          case NOT_IN_BA_LONG:
#if defined(TRACING)
            TRACE_EVENT_P2 ("NC%u[%u] remove", i, pcell->ba_arfcn);
            /* this trace must be performed before the removal to get right results */
#endif  /* TRACING */
            c_ba_arfcn = nc_remove_channel_from_ba_list (i);
            i--;
            pcell--;
            ALR_TRACE_ALL_NC ();
            break;
          case IN_BA:
            /* do nothing */
            break;
        }
      }
    }
  }

  switch(alr_data->nc_data.tim_state)
  {
    case NC_CONF_ENABLED:
      nc_start_ncell_confirm();
      break;
    case NC_CONF_DISABLED:
      alr_data->nc_data.tim_state = NC_CONF_PENDING;
      break;
    case NC_CONF_PENDING:
      TRACE_ERROR("TIM_NCSYNC expiry in unexpected state");
      /*
       * this should never happen, but in case ALR is waiting
       * more than 20 seconds for the last MPHC_NCELL_SYNC_IND of the
       * confirmation procedure and the reason isn't a position request,
       * then the confirmation procedure is restarted
       */
      if (GET_STATE(STATE_NC_PROC) EQ NC_CONFIRM AND
        alr_data->nc_data.ppos_req EQ NULL)
      {
        alr_data->nc_data.c_nc_timer = 0;
        nc_start_ncell_confirm();
      }
      break;
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_init_pos_ind            |
+--------------------------------------------------------------------+

  PURPOSE : initial setting of a MPH_NCELL_POS_IND

*/

LOCAL void nc_init_pos_ind (USHORT req_id)
{
  GET_INSTANCE_DATA;
  memset(&(alr_data->nc_data.ppos_ind->eotd_sc_res),  0, sizeof (T_eotd_sc_res ));
  memset(&(alr_data->nc_data.ppos_ind->eotd_sc_res1), 0, sizeof (T_eotd_sc_res1));
  alr_data->nc_data.ppos_ind->c_eotd_nc_res = 0;
  alr_data->nc_data.ppos_ind->req_id = req_id;
  alr_data->nc_data.ppos_ind->eotd_res = 0;
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_is_sync                 |
+--------------------------------------------------------------------+

  PURPOSE : checks whether a ncell is synchronized and timing values
            are valid

*/
LOCAL BOOL nc_is_sync (T_NC * p_ncell)
{
  switch (p_ncell->status)
  {
    case FB_SB_SYNC:
    case READ_SB:
    case READ_SB_BCCH:
    case READ_BCCH:
    case READ_BCCH_RR_NOT_INFORMED:
    case IDLE_SYNC:
      return TRUE;
    default:
      return FALSE;
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_ncell_pos_req           |
+--------------------------------------------------------------------+

  PURPOSE : reception of a ncell_pos_req

*/

void nc_ncell_pos_req (T_MPH_NCELL_POS_REQ* pos_req)
{
  GET_INSTANCE_DATA;
  /* if there is an earlier position request disgard it*/
  if (alr_data->nc_data.ppos_req NEQ NULL)
    PFREE(alr_data->nc_data.ppos_req);
  /* store the request until the end of the respective position indication */
  alr_data->nc_data.ppos_req = pos_req;

  /*
   * process the request immediately if confirmation is enabled or
   * another measurement is in progress
   * (otherwise start processing when confirmation becomes enabled
   */
  if ( (alr_data->nc_data.tim_state EQ NC_CONF_ENABLED) OR
       (alr_data->nc_data.tim_state EQ NC_CONF_DISABLED AND
        GET_STATE(STATE_NC_PROC) EQ NC_CONFIRM) )
    nc_process_pos_req(pos_req);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_process_pos_req         |
+--------------------------------------------------------------------+

  PURPOSE : processes a ncell_pos_req

*/

LOCAL void nc_process_pos_req (T_MPH_NCELL_POS_REQ* pos_req)
{
  GET_INSTANCE_DATA;
  UBYTE fail=1;

  if (alr_data->nc_data.ppos_ind EQ NULL)
  {
    PALLOC (pos_ind, MPH_NCELL_POS_IND);
    alr_data->nc_data.ppos_ind = pos_ind;
  }

  nc_init_pos_ind(pos_req->req_id);

#if 0 /* allow position requests also in NC_IDLE mode to be able to answer a test SMS */
  switch(GET_STATE(STATE_NC))
  {
    case NC_DEDICATED:
#endif /* 0|1 */
      if(alr_data->nc_data.eotd_avail)
      {
        T_NC*          p_ncell;
        T_ncell_eotd*  p_ncell_pos = &pos_req->ncell_eotd[0];
        T_NC*          list_of_asd_1o12 [12];
        T_ncell_eotd*  list_of_asd_ba   [15];
        T_ncell_eotd*  list_of_asd      [15];
        T_NC**         p_asd_1o12 = &list_of_asd_1o12[0];
        T_ncell_eotd** p_asd_ba   = &list_of_asd_ba[0];
        T_ncell_eotd** p_asd      = &list_of_asd[0];
        UBYTE          c_ba = alr_data->nc_data.c_ba_arfcn;
        UBYTE          i,j;
        UBYTE          c_list;
        UBYTE          found;

        fail=0;

        nc_stop_all();

        /*
         * generate and send an appropriate MPHC_NCELL_LIST_SYNC_REQ
         * 1/ take all ncells which are in ASD list and the twelve
         *    strongest synchronized of the BA list (using timing values
         *    from former synchronisation)
         * 2/ then all ncells which are in ASD list and in the BA list
         *    (using timings from the ASD list)
         * 3/ then rest of ASD cells (using timings from the ASD list)
         * 4/ then rest of BA list cells which are synchronized (using
         *    timing values from former synchronisation)
         */
        {
          PALLOC (ncell_list_req, MPHC_NCELL_LIST_SYNC_REQ);
          ncell_list_req->eotd = TRUE;

          /* for all ncells in ASD list */
          for(j=0; j < MAX_NCELL_EOTD_L1 AND j < pos_req->c_ncell_eotd;
              j++, p_ncell_pos++)
          {
            if (p_ncell_pos->arfcn EQ alr_data->serving_cell)
            {
              continue;
            }
            /* look if it is in BA list */
            found=FALSE;
            for (i = 0, p_ncell = &alr_data->nc_data.cell[0]; i < c_ba; i++, p_ncell++)
            {
              if(p_ncell_pos->arfcn EQ p_ncell->ba_arfcn)
              {
               /*
                * if it is in both list then check
                * if it is synchronized or not
                * and use the correct list
                */
                if(nc_is_sync(p_ncell))
                { /* case 1/ */
                  *(p_asd_1o12++) = p_ncell;
                  found=TRUE;
                }
                else
                { /* case 2/ */
                  *(p_asd_ba++) = p_ncell_pos;
                  found=TRUE;
                } /* is sync */
                break;
              } /*arfcn*/
            } /*ba list*/

            /* if nothing was found, then it is case 3/ */
            if(!found)
              *(p_asd++) = p_ncell_pos;

          } /* asd list */

          c_list=0;
          for(i=0; c_list < MAX_L1_SYNC_CNT AND i < p_asd_1o12-list_of_asd_1o12;i++)
          {
            nc_ncell_list_req_pos(&ncell_list_req->ncell_list[c_list++],
                                  NULL,
                                  list_of_asd_1o12[i]->ba_arfcn);
          } /*asd_1o12 */
          for(i=0 ;c_list < MAX_L1_SYNC_CNT AND i < p_asd_ba-list_of_asd_ba;i++)
          {
            nc_ncell_list_req_pos(&ncell_list_req->ncell_list[c_list++],
                                  list_of_asd_ba[i],
                                  list_of_asd_ba[i]->arfcn);

          }
          for(i=0 ;c_list < MAX_L1_SYNC_CNT AND i < p_asd-list_of_asd;i++)
          {
            nc_ncell_list_req_pos(&ncell_list_req->ncell_list[c_list++],
                                  list_of_asd[i],
                                  list_of_asd[i]->arfcn);
          }
          /*
           * fill the ncell list req with the remaining synchronized
           * neigbour cells of the BA list (case 4/ )
           */
          for (i=0, p_ncell = &alr_data->nc_data.cell[0];
               c_list < MAX_L1_SYNC_CNT AND i < c_ba;
               i++, p_ncell++)
          {
            if (p_ncell->ba_arfcn EQ alr_data->serving_cell)
            {
              continue;
            }
            if (nc_is_sync(p_ncell))
            {
              USHORT l1_arfcn = ARFCN_TO_L1(p_ncell->ba_arfcn);
              found=FALSE;
              for (j=0; j < c_list;j++)
              {
                if (ncell_list_req->ncell_list[j].radio_freq EQ l1_arfcn)
                {
                  found = TRUE;
                  break;
                }
              } /* MPHC_NCELL_LIST_SYNC_REQ */
              if (!found)
                nc_ncell_list_req_pos(&ncell_list_req->ncell_list[c_list++],
                                      NULL,
                                      p_ncell->ba_arfcn);
            } /* is sync */
          } /* BA list */

          if (c_list > 0)
          {
            ncell_list_req->list_size     = c_list;
            alr_data->nc_data.c_sync_req  = c_list;
            if (c_list < MAX_NCELL_EOTD_L1)
              memset(&(ncell_list_req->ncell_list[c_list]),
                     0,
                     (MAX_NCELL_EOTD_L1 - c_list) * sizeof(T_ncell_list));

            alr_data->nc_data.c_sync_req += 2; /* for the 2 scell ind's */
            nc_disable_conf(DONT_FREE_POS_IND);
            SET_STATE(STATE_NC_PROC, NC_CONFIRM);
            ma_nc_list_sync_req (ncell_list_req);
            TRACE_EVENT_P1("MPHC_NCELL_LIST_SYNC_REQ eotd=TRUE c_sync_req=%u", alr_data->nc_data.c_sync_req);
          }
          else
            fail = TRUE;
        }/* PALLOC list_req */
      } /* eotd_avail */
      /* else fail=1*/
#if 0
      break;
    default:
      /*fail=1*/
      break;
  } /*switch NC_STATE*/
#endif /* 0|1 */

  if(fail)
  { /* TODO complete ?? */
    alr_data->nc_data.ppos_ind->eotd_res = EOTD_REF;
    PSENDX(RR, alr_data->nc_data.ppos_ind);
    alr_data->nc_data.ppos_ind = NULL;
    PFREE(pos_req);
    alr_data->nc_data.ppos_req = NULL;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_ncell_list_req_pos      |
+--------------------------------------------------------------------+

  PURPOSE : Handles cell lists.

*/

LOCAL void nc_ncell_list_req_pos(T_ncell_list* p_ncell_list,
                                 T_ncell_eotd* asd_cell,
                                 USHORT arfcn)
{
  GET_INSTANCE_DATA;
  USHORT index = nc_get_index(arfcn);

  p_ncell_list->radio_freq       = ARFCN_TO_L1(arfcn);

  if(asd_cell)
  {
    if(index NEQ NOT_PRESENT_16BIT AND index NEQ LAST_BSIC_REQ AND
       alr_data->nc_data.cell[index].status EQ IDLE_SYNC)
    {
      p_ncell_list->fn_offset       = alr_data->nc_data.cell[index].frame_offset;
      p_ncell_list->time_alignment  = alr_data->nc_data.cell[index].time_align;
      p_ncell_list->timing_validity = TV_VALID_TIMING_INFO;
      /*TODO what about reception of ncell_sync_ind for this cell
        are timing info  and counter updated ?*/
    }
    else
    {
      nc_get_timing_eotd(p_ncell_list, asd_cell);
      p_ncell_list->timing_validity  = TV_VALID_TIMING_INFO; /* TV_APPROX_TIMING_INFO;*/
    }
  } /*asd_cell*/
  else
  {
    if(index NEQ NOT_PRESENT_16BIT AND index NEQ LAST_BSIC_REQ)
    {
      /* cell is in BA list and 1o12, so use the data we have */
      switch(alr_data->nc_data.cell[index].status)
      {
        case READ_SB_BCCH:
        case READ_BCCH:
          nc_set_status(index,READ_SB_BCCH_PENDING);
          break;
        case READ_SB:
        case FB_SB_SYNC:
          nc_set_status(index,READ_SB_PENDING);
          break;
        case READ_FB_SB:
        case READ_BCCH_RR_NOT_INFORMED:
        case FB_SB_SYNC_RR_NOT_INFORMED:
          /*TODO what about reception of ncell_sync_ind for this cell
            are timing info  and counter updated ?*/
        break;
      } /*status*/
      p_ncell_list->fn_offset       = alr_data->nc_data.cell[index].frame_offset;
      p_ncell_list->time_alignment  = alr_data->nc_data.cell[index].time_align;
      p_ncell_list->timing_validity = TV_VALID_TIMING_INFO;
    }
    else
    {
      TRACE_ERROR("nc_ncell_list_req_pos: asd_cell==0 but cell not in BA");
    }
  }/*asd_cell*/
}

LOCAL void nc_get_timing_eotd(T_ncell_list* p_ncell_list,
                              T_ncell_eotd* asd_cell)
{
  USHORT td, adjust = 0, offs;
  /*
   * The values mfrm_offset, rough_rtd and exp_otd are differences
   * between the BTS in question and the reference BTS.
   * L1 expects the differences between serving BTS and BTS in question:
   *
   *                   +----+----+----+----+----+....+----+----+----+----+----+ serving BTS
   *                   0                                                      51
   *                   |
   *             0 Net | L1 1250 (5000)
   *             +---->V<----+
   *              \         /
   *               \      /
   * +----+----+----+----+----+....+----+----+----+----+----+  BTS in question
   * 0                                                      51
   * +-----Net----->|    |<---------L1----------------------+
   *
   * time differences are indicated in bits by the net, but expected
   * in quarterbits by L1
   */

  offs = asd_cell->mfrm_offset;

  if (asd_cell->otd_type EQ BOTH_OTD)
  {
    int diff = asd_cell->rough_rtd - asd_cell->exp_otd;
    if (diff >= 850)
      adjust = 1;
    else if (diff <= -850)
      adjust = (USHORT)-1;  /* adjust is USHORT type so typecasting -1 with USHORT  */
    /*
     * GSM 04.31 section A.2.2.3 version 8.7.0 says that adjust shall be
     * equal 0 for -400 <= diff <= 400 and states an error for
     * -850 < diff < -450 and for 450 < diff < 850, but doesn't
     * describe the behaviour in this cases.
     * All this valid and invalid cases are implemented here by
     * adjust = 0
     */
  }

  if (asd_cell->otd_type EQ ROUGH_OTD)
    td = asd_cell->rough_rtd;
  else
  { /*
     * uncertainty indicates how many bits the search window
     * for measurement should be opended earlier than the expected
     * value. Because of the subsequent subtraction we have to ADD
     * the uncertainty value.
     * no special handling for uncertainty value 7 (uncertainty > 30 bits)
     * is implemented yet
     */
    td = asd_cell->exp_otd + unc_values[asd_cell->uncertainty];
    if (td >= 1250)
      offs++;
  }
  td %= 1250;

  if (td)
  {
    p_ncell_list->time_alignment = 5000 - 4 * td;
    offs++;
  }
  else
    p_ncell_list->time_alignment = 0;

  offs = (offs + adjust) % 51;

  if (offs)
    p_ncell_list->fn_offset = 51 - offs;
  else
    p_ncell_list->fn_offset = 0;
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_start_eotd_confirm      |
+--------------------------------------------------------------------+

  PURPOSE : generates an MPHC_NCELL_LIST_SYNC_REQ from the BA list.
*/
LOCAL void nc_start_eotd_confirm(void)
{
  GET_INSTANCE_DATA;
  /*
   * this check is needed for the resume
   * in the GPRS case because we need
   * to keep the states over a
   * start transition
   */
  if(alr_data->nc_data.ppos_ind EQ NULL)
  {
    PALLOC (pos_ind, MPH_NCELL_POS_IND);
    alr_data->nc_data.ppos_ind = pos_ind;
  }
  nc_init_pos_ind (NOT_PRESENT_16BIT);

  /* generate and send an appropriate MPHC_NCELL_LIST_SYNC_REQ */
  nc_ncell_list_req();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_ncell_list_req          |
+--------------------------------------------------------------------+

  PURPOSE : generates an MPHC_NCELL_LIST_SYNC_REQ based on the 12
            strongest neighbour cells of the BA

*/
LOCAL void nc_ncell_list_req (void)
{
  GET_INSTANCE_DATA;
  UBYTE c_found = 0, found, i, validity;
  USHORT c_ba = alr_data->nc_data.c_ba_arfcn;
  T_NC*   pcell;
  T_ncell_list *pnc;
  PALLOC (ncell_list_req, MPHC_NCELL_LIST_SYNC_REQ);

  if (alr_data->nc_data.c_sync_req NEQ 0)
    TRACE_EVENT_P1("nc_ncell_list_req with c_sync_req=%u before", alr_data->nc_data.c_sync_req);

  ncell_list_req->eotd = alr_data->nc_data.eotd_avail;

  nc_stop_all();
  for (i = 0, pcell = &alr_data->nc_data.cell[0]; i < c_ba; i++, pcell++)
  {
    if (alr_data->nc_data.eotd_avail AND pcell->ba_arfcn EQ alr_data->serving_cell)
    {
      continue;
    }
    if (pcell->one_of_twelve)
    {
      switch (pcell->status)
      {
        case READ_SB:
          nc_set_status(nc_get_index(pcell->ba_arfcn), READ_SB_PENDING);
          found         = TRUE;
          validity      = TV_VALID_TIMING_INFO;
          break;
        case READ_SB_BCCH:
          nc_set_status(nc_get_index(pcell->ba_arfcn), READ_SB_BCCH_PENDING);
          found         = TRUE;
          validity      = TV_VALID_TIMING_INFO;
          break;
          /* TODO(opt): if !eotd_avail maybe optimize later
        case READ_FB_SB:
        break*/
          /*
        case READ_BCCH_RR_NOT_INFORMED:
        case READ_BCCH_PENDING_RR_NOT_INFORMED:
        case READ_BCCH:
        case READ_BCCH_PENDING:
          found         = TRUE;
          validity      = TV_VALID_TIMING_INFO;
          break;*/
        default:
          found = FALSE;
          validity      = TV_INVALID_TIMING_INFO;
          break;
      }

      if (found)
      {
        pnc = &(ncell_list_req->ncell_list[c_found++]);
        pnc->radio_freq      = ARFCN_TO_L1(pcell->ba_arfcn);
        if (validity EQ TV_VALID_TIMING_INFO)
        {
          pnc->fn_offset       = pcell->frame_offset;
          pnc->time_alignment  = pcell->time_align;
        }
        else
        {
          pnc->fn_offset       = 0;
          pnc->time_alignment  = 0;
        }
        pnc->timing_validity = validity;
      }
    }
  }

  /*
   * in case there are no synchronized neighbour cells
   * send an empty MPHC_NCELL_LIST_SYNC_REQ to L1
   * (to serve the cursor task later with a position indication
   * containing measurements of the serving cell only)
   */
  if(c_found OR alr_data->nc_data.eotd_avail)
  {
    ncell_list_req->list_size = c_found;
    alr_data->nc_data.c_sync_req = c_found;
    if (c_found < MAX_NCELL_EOTD_L1)
      memset(&(ncell_list_req->ncell_list[c_found]),
             0,
             (MAX_NCELL_EOTD_L1 - c_found) * sizeof(T_ncell_list));

    if(alr_data->nc_data.eotd_avail)
      /* add the 2 scell sync_ind */
      alr_data->nc_data.c_sync_req += 2;

    nc_disable_conf(DONT_FREE_POS_IND);
    SET_STATE(STATE_NC_PROC, NC_CONFIRM);
    ma_nc_list_sync_req (ncell_list_req);
    TRACE_EVENT_P3("MPHC_NCELL_LIST_SYNC_REQ eotd=%d c_sync_req=%u SC=%u",
                    alr_data->nc_data.eotd_avail,
                    alr_data->nc_data.c_sync_req,
                    alr_data->serving_cell);
  }
  else
  {
    PFREE(ncell_list_req);
    SET_STATE(STATE_NC_PROC, NC_ACQUIRE);
    nc_enable_conf();
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_store_eotd              |
+--------------------------------------------------------------------+

  PURPOSE : copies the measurement results into the MPH_NCELL_POS_IND

*/
LOCAL void nc_store_eotd (T_eotd_sc_res         * p_res,
                          T_MPHC_NCELL_SYNC_IND * sync_ind,
                          USHORT                  arfcn)
{
  p_res->sb_flag    = sync_ind->sb_flag;
  p_res->bsic       = sync_ind->bsic;
  p_res->arfcn      = arfcn;
  memcpy(&(p_res->eotd_crosscor),
         &(sync_ind->a_eotd_crosscor),
         sizeof(sync_ind->a_eotd_crosscor));
  p_res->d_eotd_nrj = sync_ind->d_eotd_nrj;
  p_res->time_tag   = sync_ind->time_tag;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_stop_ext_meas_ind       |
+--------------------------------------------------------------------+

  PURPOSE : Notify RR about the end of the end of the Extended
            Measurement procedure.

*/
GLOBAL void nc_stop_ext_meas_ind (void)
{
  GET_INSTANCE_DATA;
  PALLOC ( mph_sync_ind, MPH_SYNC_IND );
  mph_sync_ind->cs    = CS_STOP_PLMN_SEARCH;
  mph_sync_ind->arfcn = NOT_PRESENT_16BIT;
  PSENDX ( RR, mph_sync_ind );

  if( alr_data->cs_data.p_power_cnf NEQ NULL )
  {
    PFREE(alr_data->cs_data.p_power_cnf)
    alr_data->cs_data.p_power_cnf = NULL;
  }
  if( IS_EXT_MEAS_RUNNING )/*alr_data->cs_data.mph_ext_meas_req NEQ NULL */
  {
    PFREE ( alr_data->cs_data.mph_ext_meas_req );
    alr_data->cs_data.mph_ext_meas_req = NULL;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_stop_rr_activity        |
+--------------------------------------------------------------------+

  PURPOSE : Stop BCCH reading or PLMN seach originated by RR.

*/
GLOBAL void nc_stop_rr_activity (UBYTE stop)
{
  GET_INSTANCE_DATA;
  /*
   * RR signals stop BCCH reading or stop PLMN search
   */
  if( (IS_EXT_MEAS_RUNNING) AND
       (alr_data->nc_data.cell[LAST_BSIC_REQ].status EQ READ_FB_SB_PENDING) )
  {
        /*
         * Wait for MPHC_NCELL_SYNC_IND  or MPHC_STOP_NCELL_SYNC_CON
         */
        alr_data->cs_data.ext_meas_state_pend = CS_ACTIVE_SYNC;
  }
  nc_clear_last_bsic();

  /*
   * RR signals end of PLMN search
   */
  if (stop)
  {
    alr_data->plmn_search_running = FALSE;
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
    alr_multiband_std = NOT_PRESENT_8BIT;
#endif
    nc_enable_conf();
  }

  if ( IS_EXT_MEAS_RUNNING AND (alr_data->cs_data.ext_meas_state_pend EQ CS_NULL) )
  {
      nc_stop_ext_meas_ind();
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_bsic_req                |
+--------------------------------------------------------------------+

  PURPOSE : Request of RR to search for frequency correction
            burst and synchron burst.

*/

GLOBAL void nc_bsic_req (T_MPH_BSIC_REQ *mph_bsic_req)
{
  GET_INSTANCE_DATA;
  USHORT i;

  nc_clear_last_bsic();

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  if (alr_data->plmn_search_running)
  {
    if (nc_compare_bsic_std (STD_GET_FROM_ARFCN (mph_bsic_req->arfcn)))
    {
      alr_multiband_std = STD_GET_FROM_ARFCN (mph_bsic_req->arfcn);
      TRACE_EVENT_P2 ("Received BSIC req for different std %d, %d", std, alr_multiband_std);
    }
  }
#endif

  mph_bsic_req->arfcn &= ARFCN_MASK;
  ALR_TRACE_NC_BSIC_REQ (mph_bsic_req->arfcn);

  ALR_EM_NEIGHBOURCELL_BSIC_REQUEST;


  /*
   * look if the requested channel number is also in the normal
   * BA list to stop it.
   */
  i = nc_get_index( mph_bsic_req->arfcn );

  switch(i)
  {
    case NOT_PRESENT_16BIT:
    case LAST_BSIC_REQ:
      break;
    default:
      nc_stop_if_active(i);
      TRACE_ERROR("bsic_req for arfcn which is in BA list");
      break;
  }


  /*
   * fill in parameters for BSIC request
   */
  nc_set_status (LAST_BSIC_REQ, READ_FB_SB);
  alr_data->nc_data.cell[LAST_BSIC_REQ].ba_arfcn = mph_bsic_req->arfcn;

  /*
   * forward synchronisation request to layer 1, if not full
   */
  if (alr_data->nc_data.c_sync_req < MAX_L1_SYNC_CNT)
  {
    /*
     * reset if we are in NC_CON_EST
     */

    if(GET_STATE(STATE_NC) EQ NC_CON_EST
#ifdef GPRS
       AND !alr_data->gprs_data.pbcch
#endif
      )
    {
      SET_STATE(STATE_NC, NC_IDLE);
    }
#ifdef GPRS
    else if(GET_STATE(STATE_NC) EQ NC_CON_EST AND
             alr_data->gprs_data.pbcch) /* AND */
/* add later          alr_data->gprs_data.pim) */
    {
      SET_STATE(STATE_NC, NC_PIM_PBCCH);
    }
#endif

    nc_process_status ();
  }

}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : alr_nc                     |
| STATE   : code                ROUTINE : nc_bcch_ind                |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a system information message for a neighbourcell.

*/

GLOBAL void nc_bcch_ind (T_MPHC_NCELL_BCCH_IND *data_ind)
{
  GET_INSTANCE_DATA;
  USHORT index;
  UBYTE  msg_t            = data_ind->l2_frame.content[SI_CONTENTS_MSG_T];

  if (alr_data->nc_data.cell[LAST_BSIC_REQ].status EQ READ_BCCH_PENDING AND
      alr_data->nc_data.cell[LAST_BSIC_REQ].ba_arfcn EQ ARFCN_TO_G23(data_ind->radio_freq))
  {
    /*
     * Cell selection or PLMN available search triggered by RR.
     */
    if (data_ind->error_flag NEQ VALID_BLOCK)
    {
      /*
       * Reception of an invalid block
       */
      ALR_TRACE_NC ("invalid PLMN BCCH");

      /*
       * indicate BCCH read error to RR, too many errors are controlled by RR
       */
      ma_error_ind (CS_BCCH_READ_ERROR, ARFCN_TO_G23(data_ind->radio_freq));
    }
    else
    {
      UBYTE mt = data_ind->l2_frame.content[2];

      ALR_TRACE_NC ("valid PLMN BCCH");

      /*
       * forward message to RR
       */
      ma_send_unitdata ((T_MPHC_DATA_IND *)data_ind);

      switch (mt)
      {
        case D_SYS_INFO_3:
        case D_SYS_INFO_4:
          alr_data->nc_data.cell[LAST_BSIC_REQ].blocks_required = 0;
          break;

        default:
          /*
           * Reception of any other message. Shall not happen after
           * GSM 5.02 chapter 6.3.1.3 Mapping of BCCH data, but
           * is possible for future extensions.
           * Read this tc again plus all which are not read until now.
           */
          break;
      }
    }

    /*
     * all frames received, then decrement pending BCCH request
     */
    if (alr_data->nc_data.cell[LAST_BSIC_REQ].blocks_required EQ 0)
    {

      nc_set_status (LAST_BSIC_REQ, IDLE);
      alr_data->nc_data.cell[LAST_BSIC_REQ].ba_arfcn = NOT_PRESENT_16BIT;

      /*
       * L3 may avoid sending a stop message to terminate a NCELL_BCCH process
       * if there no more pending request in L1
       */
      if (alr_data->nc_data.c_bcch_req > 0)
      {
        /*
         * stop any ongoing request in layer 1
         */
        ma_nc_stop_ncell_bcch_req(ARFCN_TO_G23(data_ind->radio_freq));

        /*
         * decrement the number of pending requests in layer 1.
         */
        alr_data->nc_data.c_bcch_req--;
      }
    }
    else
    {
      /*
       * stop pending request and start again.
       */
      nc_restart_bcch (LAST_BSIC_REQ);
    }
    return ;
  }

  /*
   * Normal Ncell Handling, get position in neighbourcell list
   */
  index = nc_get_index (ARFCN_TO_G23(data_ind->radio_freq));

  if (index NEQ NOT_PRESENT_16BIT AND
      index NEQ LAST_BSIC_REQ     AND
      (alr_data->nc_data.cell[index].status EQ READ_BCCH_PENDING OR
       alr_data->nc_data.cell[index].status EQ READ_BCCH_PENDING_RR_NOT_INFORMED))
  {
    /*
     * normal member of the neighbourcell list and an answer is expected.
     */
    if (data_ind->error_flag EQ VALID_BLOCK)
    {
      switch (msg_t)
      {
        case D_SYS_INFO_3:
          /*
           * Sys Info 3 contains the needed information in any case
           */
#if defined(TRACING)
          TRACE_EVENT_P3("NC%u[%d] BCCH ok %s",
            index,
            alr_data->nc_data.cell[index].ba_arfcn EQ NOT_PRESENT_16BIT ?
              -1 : alr_data->nc_data.cell[index].ba_arfcn&ARFCN_MASK, "si3");
#endif  /* TRACING */
          if (alr_data->nc_data.cell[index].status EQ READ_BCCH_PENDING_RR_NOT_INFORMED)
          {
            /*
             * store data if RR is not informed yet about
             * synchronisation
             */
            /*lint !e419 (Warning -- Apparent data overrun)*/
            nc_store_bcch((T_MPHC_DATA_IND *) data_ind, index,0);
            nc_set_status (index, FB_SB_SYNC_RR_NOT_INFORMED);
          }
          else
          {
            /*
             * forward information to RR
             */
            ma_send_unitdata ((T_MPHC_DATA_IND *)data_ind);
            nc_set_fb_sb_sync_initial (index);

          }
          break;

        case D_SYS_INFO_4:
          if ((data_ind->l2_frame.content[SI_CONTENTS_CS2] & ONLY_ACS) EQ 0)
          {
            nc_set_si4_si78 (index, data_ind, 0);
          }
          else
          {
#if defined(TRACING)
          TRACE_EVENT_P3("NC%u[%d] BCCH ok %s",
            index,
            alr_data->nc_data.cell[index].ba_arfcn EQ NOT_PRESENT_16BIT ?
              -1 : alr_data->nc_data.cell[index].ba_arfcn&ARFCN_MASK, "si4_n78");
#endif  /* TRACING */
            /*
             * additional information from system info 7 or 8
             * is needed for cell reselection purposes
             */
            if(alr_data->nc_data.cell[index].status EQ READ_BCCH_PENDING_RR_NOT_INFORMED)
            {
              /*
               * store data if RR is not informed yet about synchronisation
               */
              nc_store_bcch((T_MPHC_DATA_IND *) data_ind, index,0);
            }
            else
            {
              /*
               * forward information to RR
               */
              ma_send_unitdata ((T_MPHC_DATA_IND *)data_ind);
            }

            /*
             * system info 3, 7 or 8 required
             * -> tc = 2,6 for normal BCCH or 3,7 for extended BCCH
             */
            alr_data->nc_data.cell[index].blocks_required = NCELL_BCCH_SI_3_7_8;
          }
          break;

        case D_SYS_INFO_7:
        case D_SYS_INFO_8:
          if (nc_sys_info_78_required (index))
          {
            nc_set_si4_si78(index, data_ind, 1);
          }
          else
            /*
             * increment error counter and request tc again.
             */
            alr_data->nc_data.cell[index].c_error++;

          break;

        default:
          /*
           * increment error counter and request tc again.
           */
          alr_data->nc_data.cell[index].c_error++;
          break;
      }
    }
    else
    {
      /*
       * BCCH reading failed
       *
       * error counter is incremented and tc shall be read again.
       */
      alr_data->nc_data.cell[index].c_error++;
    }

    /*
     * restart next attempt
     */
    if (alr_data->nc_data.cell[index].blocks_required EQ 0)
    {
      if (alr_data->nc_data.c_bcch_req > 0)
      {
        /*
         * decrement number of pending BCCH requests in layer 1.
         */
          alr_data->nc_data.c_bcch_req--;

        /*
         * Stop the pending reading, if not all tc-s read
         */
        ma_nc_stop_ncell_bcch_req (alr_data->nc_data.cell[index].ba_arfcn);
      }
      alr_data->nc_data.cell[index].c_attempt = 0;

      /*
       * start next request to layer 1 if necessary
       */
      nc_process_status ();
    }
    else
    {
      if (alr_data->nc_data.cell[index].c_error >= 4)
      {
        /*
         * set status to failed or excluded depending on the failed
         * attempt counter and/or restart for this channel.
         */
        nc_sync_failed_attempt (index);

        /*
         * L3 may avoid sending a stop message to terminate a NCELL_BCCH process
         * if there no more pending request in L1
         */
        if (alr_data->nc_data.c_bcch_req > 0)
        {
          /*
           * decrement number of pending BCCH requests in layer 1.
           */
            alr_data->nc_data.c_bcch_req--;

          /*
           * Stop the pending reading, if not all tc-s read
           */
          ma_nc_stop_ncell_bcch_req (alr_data->nc_data.cell[index].ba_arfcn);
        }

        /*
         * start next request to layer 1 if necessary
         */
        nc_process_status ();
      }
      else
      {
        /*
         * restart the BCCH reading for this TC again.
         */
        nc_restart_bcch (index);
      }
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : alr_nc                     |
| STATE   : code                ROUTINE : nc_restart_bcch            |
+--------------------------------------------------------------------+

  PURPOSE : Stop old request and start new request to layer 1.

*/

LOCAL void nc_restart_bcch (USHORT index)
{
  GET_INSTANCE_DATA;
  PALLOC (ncell_bcch, MPHC_NCELL_BCCH_REQ);

  if (alr_data->nc_data.c_bcch_req > 0)
  {
    /*
     * if necessary stop previous request to avoid on the fly change
     */
    ma_nc_stop_ncell_bcch_req(alr_data->nc_data.cell[index].ba_arfcn);
  }

  ncell_bcch->radio_freq           = ARFCN_TO_L1(alr_data->nc_data.cell[index].ba_arfcn);
  ncell_bcch->fn_offset            = alr_data->nc_data.cell[index].frame_offset;
  ncell_bcch->time_alignment       = alr_data->nc_data.cell[index].time_align;
  ncell_bcch->tsc                  = (UBYTE)(alr_data->nc_data.cell[index].bsic & ONLY_BCC);
  ncell_bcch->bcch_blocks_required = alr_data->nc_data.cell[index].blocks_required;
#ifdef GPRS
  /*if the mobile is in PTM the GPRS_PRIORITY must be set to TOP*/
  if(alr_data->pch_data.pl_idle.page_mode EQ PGM_REORG OR
     alr_data->pch_data.pl_idle.page_mode EQ PGM_REORG_CS OR
     alr_data->pch_data.reorg_bcch_reading EQ TRUE OR
     alr_data->nc_data.cell[index].new_strong_cell OR
     !alr_data->nc_data.cell[index].c_attempt OR  // EQ 0
     ma_is_ptm())
  {
    ncell_bcch->gprs_prio            = GPRS_PRIO_TOP;
  }
  else
#endif
    ncell_bcch->gprs_prio            = GPRS_PRIO_NORM;

  /*
   * and start next request
   */
  ma_nc_bcch_req (ncell_bcch);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : alr_nc                     |
| STATE   : code                ROUTINE : nc_store_sync_ind          |
+--------------------------------------------------------------------+

  PURPOSE : stores the data of an MPHC_NCELL_SYNC_IND end enters
            a new state of ncell.

*/

LOCAL void nc_store_sync_ind (USHORT index,
                              T_MPHC_NCELL_SYNC_IND *sync_ind,
                              UBYTE new_status)
{
  GET_INSTANCE_DATA;
  T_NC* pcell = &alr_data->nc_data.cell[index];
  pcell->bsic         = (UBYTE)(sync_ind->bsic & ONLY_BSIC);
  pcell->frame_offset = sync_ind->fn_offset;
  pcell->time_align   = sync_ind->time_alignment;
  pcell->tim_valid    = TV_VALID_TIMING_INFO;
  nc_set_status (index, new_status);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : alr_nc                     |
| STATE   : code                ROUTINE : nc_sync_ind                |
+--------------------------------------------------------------------+

  PURPOSE : Confirmation for a synchronisation request to layer 1.

*/

GLOBAL void nc_sync_ind (T_MPHC_NCELL_SYNC_IND *sync_ind)
{
  GET_INSTANCE_DATA;
  USHORT  index;
  UBYTE   bsic = (UBYTE)(sync_ind->bsic & ONLY_BSIC);
  USHORT  arfcn;

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  if (alr_data->plmn_search_running AND (alr_multiband_std NEQ NOT_PRESENT_8BIT))
    arfcn = ARFCN_STD_TO_G23 (sync_ind->radio_freq, alr_multiband_std)&ARFCN_MASK;
  else
#endif
    arfcn = ARFCN_TO_G23(sync_ind->radio_freq)&ARFCN_MASK;

  index = nc_get_index (arfcn);

  switch (GET_STATE (STATE_NC))
  {
    case NC_IDLE:
    case NC_DEDICATED:
#ifdef GPRS
    case NC_PIM_PBCCH:
    case NC_PTM_PBCCH: /*XXX*/
#endif
      TRACE_EVENT_P5("nc_sync_ind[%d] sb_flag=%d fn_offset=%ld time_alignment=%ld bsic=%d",
                      arfcn, sync_ind->sb_flag, sync_ind->fn_offset,
                      sync_ind->time_alignment, sync_ind->bsic);
      if (alr_data->nc_data.c_sync_req > 0)
        alr_data->nc_data.c_sync_req--;

#if defined (REL99) && defined (TI_PS_FF_EMR)
      /*EMR :If all the previous sync requests are successful, start fresh ranking for
        sync requests*/
      if (  (alr_data->nc_data.emr_data.rep_type EQ REP_TYPE_ENH ) AND
            (alr_data->nc_data.c_sync_req EQ 0) )           
      {
        alr_data->nc_data.rank_cell_bmp = 0; 
      }
#endif
      if (alr_data->nc_data.cell[LAST_BSIC_REQ].status EQ READ_FB_SB_PENDING AND
          alr_data->nc_data.cell[LAST_BSIC_REQ].ba_arfcn EQ arfcn AND
#ifdef GPRS
          (GET_STATE(STATE_NC) EQ NC_IDLE OR GET_STATE(STATE_NC) EQ NC_PIM_PBCCH)
         )
#else
           GET_STATE(STATE_NC) EQ NC_IDLE
         )
#endif
      {
        nc_sync_ind_last_bsic_req(sync_ind,index,arfcn,bsic);
      }
      else
      {
        if (index NEQ NOT_PRESENT_16BIT AND
            index NEQ LAST_BSIC_REQ)
        {
          nc_sync_ind_ncell(sync_ind,index,arfcn,bsic);
        }   /* valid index   check */
        else if (alr_data->nc_data.eotd_avail AND
                 alr_data->nc_data.ppos_ind NEQ NULL)
          nc_check_sync_ind_eotd(sync_ind, arfcn);
           /* LAST_BSIC_REQ check */
        else if ( (index EQ NOT_PRESENT_16BIT) AND (IS_EXT_MEAS_RUNNING) AND
                    (GET_STATE(STATE_NC) EQ NC_IDLE))
        {
          if ( alr_data->cs_data.ext_meas_state_pend NEQ CS_NULL )
          {
            nc_stop_ext_meas_ind();
            alr_data->cs_data.ext_meas_state_pend = CS_NULL;
            return;
          }
        }
      }

      if( alr_data->nc_data.c_sync_intrupted EQ TRUE AND alr_data->nc_data.c_sync_req EQ 0)
      {
        nc_enable_conf();
        alr_data->nc_data.c_sync_intrupted = FALSE;
      }

      if(GET_STATE(STATE_NC_PROC) EQ NC_ACQUIRE)
      {
        nc_process_status();
      }

      break;
    default:
      break;
  } /* STATE_NC */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : alr_nc                     |
| STATE   : code                ROUTINE : nc_sync_ind_last_bsic_req  |
+--------------------------------------------------------------------+

  PURPOSE : Confirmation of the LAST_BSIC_REQ synchronisation request

*/

LOCAL void nc_sync_ind_last_bsic_req(T_MPHC_NCELL_SYNC_IND* sync_ind,
                                     USHORT index,
                                     USHORT arfcn,
                                     UBYTE bsic)
{
  GET_INSTANCE_DATA;

  /*
   * decoding of FB / SB of hplmn cell is in progress
   * send confirmation to RR.
   */
  PALLOC (mph_bsic_cnf, MPH_BSIC_CNF);

  mph_bsic_cnf->arfcn = arfcn;
  mph_bsic_cnf->bsic  = bsic;

  if (sync_ind->sb_flag EQ SB_FOUND)
  {
    ALR_TRACE_NC_BSIC_CNF(sync_ind->radio_freq);

    /*
     * FCB and SCB found, set result code
     */
    mph_bsic_cnf->cs = CS_NO_ERROR;

#if TI_PS_FF_QUAD_BAND_SUPPORT
    if (alr_data->plmn_search_running AND (alr_multiband_std NEQ NOT_PRESENT_8BIT))
    {
      mph_bsic_cnf->arfcn = STD_ADD_TO_ARFCN(arfcn, alr_multiband_std);
      /* US_BIT should be used to differentiate an US frequency channel. */
      switch (alr_multiband_std)
      {
        case STD_1900:
        case STD_850:
        case STD_DUAL_US:
          mph_bsic_cnf->arfcn |= US_BIT;
          break;
        case STD_850_1800:
        case STD_850_900_1800:
          if ((arfcn >= LOW_CHANNEL_850) && (arfcn <= HIGH_CHANNEL_850))
            mph_bsic_cnf->arfcn |= US_BIT;
          break;
        case STD_900_1900:
          if ((arfcn >= LOW_CHANNEL_1900) && (arfcn <= HIGH_CHANNEL_1900))
            mph_bsic_cnf->arfcn |= US_BIT;
          break;
        case STD_850_900_1900:
          if (arfcn >= HIGH_CHANNEL_900)
            mph_bsic_cnf->arfcn |= US_BIT;
          break;
        default:
          break;
      }
    }
#endif
    
    if ( IS_EXT_MEAS_RUNNING )
    {
      nc_set_status (LAST_BSIC_REQ, IDLE);
      alr_data->nc_data.cell[LAST_BSIC_REQ].ba_arfcn = NOT_PRESENT_16BIT;

      if ( alr_data->cs_data.ext_meas_state_pend NEQ CS_NULL )
      {
        PFREE ( mph_bsic_cnf );
        nc_stop_ext_meas_ind();
      }
      else
      {
        ma_bsic_cnf (mph_bsic_cnf);
      }
      return;
    }

    ALR_EM_NEIGHBOURCELL_BCCH(EM_AVAIL);

    /* save data for next cell reselection */
    {
      T_NC *pcr_cell = &alr_data->nc_data.cr_cell;
      pcr_cell->ba_arfcn = arfcn;
      pcr_cell->bsic = bsic;
      pcr_cell->frame_offset = sync_ind->fn_offset;
      pcr_cell->time_align = sync_ind->time_alignment;
    }

    ma_bsic_cnf (mph_bsic_cnf);

    nc_store_sync_ind (LAST_BSIC_REQ, sync_ind, READ_BCCH);
#ifdef GPRS
    /* don't do the next checks for PBCCH just return
     * maybe later TODO(opt)
     */
    if (GET_STATE (STATE_NC) EQ NC_PIM_PBCCH OR
        GET_STATE (STATE_NC) EQ NC_PTM_PBCCH)
    {
      return;
    }
#endif

    index = nc_get_index (arfcn);

    if(index NEQ LAST_BSIC_REQ AND
       index NEQ NOT_PRESENT_16BIT)
    {
      /*
       * check BSIC of the incoming BCCH message
       */
      switch (nc_check_bsic (index, bsic))
      {
        case NC_CHECK_OK:
          /*
           * channel has passed NCC permitted check.
           */
          switch (alr_data->nc_data.cell[index].status)
          {

            case IDLE:
              /* This patch helps during parallel search in Limited service.
               * Cell synchronised in the last BSIC_REQ fromm RR during parallel
               * search is also a part of BA list. In such a case, we store the
               * synchrinisation information, so that its can be used later by
               * nc_start_reselect. 
               */
              if(alr_data->nc_data.cell[index].one_of_twelve)
                nc_store_sync_ind (index, sync_ind, READ_BCCH_RR_NOT_INFORMED);
              else
                nc_store_sync_ind (index, sync_ind, IDLE_SYNC);                
              break;

            case READ_FB_SB:
            /*
               * the channel shall start synchronisation of FB/SB
               * so reading of BCCH shall be started.
               */
              if(alr_data->nc_data.cell[index].one_of_six AND
                 GET_STATE(STATE_NC) NEQ NC_DEDICATED)
                nc_store_sync_ind (index, sync_ind, READ_BCCH_RR_NOT_INFORMED);
              else if(alr_data->nc_data.cell[index].one_of_twelve)
                nc_store_sync_ind (index, sync_ind, FB_SB_SYNC);
              break;

            case READ_SB:
              /*
               * the channel shall start synchronisation of SB
               * so this is done.
               */
              if(alr_data->nc_data.cell[index].one_of_twelve)
                nc_store_sync_ind (index, sync_ind, FB_SB_SYNC);
              break;

            case FB_SB_FAILED:
              /*
               * A channel synchronisation has failed in the past.
               * Now it is synchronized again. Start BCCH reading
               * and send this information to RR after indicating
               * the synchronisation with next measurement report.
               */
              if(alr_data->nc_data.cell[index].one_of_six AND
                GET_STATE(STATE_NC) NEQ NC_DEDICATED)
                nc_store_sync_ind (index, sync_ind, READ_BCCH_RR_NOT_INFORMED);
              else if(alr_data->nc_data.cell[index].one_of_twelve)
                nc_store_sync_ind (index, sync_ind, FB_SB_SYNC);
              break;


            case READ_SB_BCCH:
              /*
               * the channel shall start synchronisation of SB
               * followed by BCCH reading. SB synchronisation
               * is done.
               */
              if(alr_data->nc_data.cell[index].one_of_six)
                nc_store_sync_ind (index, sync_ind, READ_BCCH);
              break;

            case READ_FB_SB_PENDING:
            case READ_SB_PENDING:
            case READ_SB_BCCH_PENDING:
            case READ_BCCH_PENDING:
            case READ_BCCH_PENDING_RR_NOT_INFORMED:
              /*
               * REMARK: this shall not happen, because this
               *         attempt shall be killed if RR requests
               *         procedure for the channel.
               */
              ALR_TRACE_NC ("Abnormal situation sync ind");
              break;

            default:
              break;
          }
          break;

        case NC_CHECK_BSIC_CHANGED:
          /*
           * BSIC of the channel has changed.
           */
          switch (alr_data->nc_data.cell[index].status)
          {
            case READ_SB:
              /*
               * the channel shall start synchronisation of SB
               * so this is done, but a changed BSIC is stored.
               * Read BCCH of the channel and forward to RR.
               */
              if(alr_data->nc_data.cell[index].one_of_six AND
                 GET_STATE(STATE_NC) NEQ NC_DEDICATED)
                nc_store_sync_ind (index, sync_ind, READ_BCCH_RR_NOT_INFORMED);
              else if(alr_data->nc_data.cell[index].one_of_twelve)
                nc_store_sync_ind (index, sync_ind, FB_SB_SYNC);
              break;

            case FB_SB_FAILED:
              /*
               * A channel synchronisation has failed in the past.
               * Now it is synchronized again. Start BCCH reading
               * and send this information to RR after indicating
               * the synchronisation with next measurement report.
               */
              if(alr_data->nc_data.cell[index].one_of_six AND
                 GET_STATE(STATE_NC) NEQ NC_DEDICATED)
                nc_store_sync_ind (index, sync_ind, READ_BCCH_RR_NOT_INFORMED);
              else if(alr_data->nc_data.cell[index].one_of_twelve)
                nc_store_sync_ind (index, sync_ind, FB_SB_SYNC);
              break;

            case READ_SB_BCCH:
                /*
                 * the channel shall start synchronisation of SB
                 * followed by BCCH reading. SB synchronisation
                 * is done. Read BCCH of the channel and forward to RR.
                 */
              if(alr_data->nc_data.cell[index].one_of_six)
                nc_store_sync_ind (index, sync_ind, READ_BCCH_RR_NOT_INFORMED);
              break;

            case READ_FB_SB:
            case READ_FB_SB_PENDING:
            case READ_SB_PENDING:
            case READ_SB_BCCH_PENDING:
            case READ_BCCH_PENDING:
            case READ_BCCH_PENDING_RR_NOT_INFORMED:
              /*
               * REMARK: this shall not happen, because this
               *         attempt shall be killed if RR requests
               *         procedure for the channel.
               */
              ALR_TRACE_NC ("Abnormal situation sync ind (changed BSIC)");
              break;

            default:
              break;
          }
          break;

        case NC_CHECK_NCC_FAILED:
          /*
           * ncc permitted check failed
           */
          nc_set_status (index, EXCLUDED);
          break;

        default:
          break;
      }
    }

    ALR_EM_NEIGHBOURCELL_BSIC_CONFIRM(EM_AVAIL);

  }
  else
  {
    /*
     * FCB and SCB not found, set result code,
     * mark cell[LAST_BSIC_REQ] and cr_cell as invalid
     * and send negative answer to RR
     */
    mph_bsic_cnf->cs = CS_NO_BCCH_AVAIL;
    nc_set_status (LAST_BSIC_REQ, IDLE);
    alr_data->nc_data.cell[LAST_BSIC_REQ].ba_arfcn = NOT_PRESENT_16BIT;
    alr_data->nc_data.cr_cell.ba_arfcn = NOT_PRESENT_16BIT;

    ma_bsic_cnf (mph_bsic_cnf);

    ALR_EM_NEIGHBOURCELL_BSIC_CONFIRM(EM_NOT_AVAIL);

  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : alr_nc                     |
| STATE   : code                ROUTINE : nc_sync_ind_ncell          |
+--------------------------------------------------------------------+

  PURPOSE : Confirmation of a ncell synchronisation request

*/

LOCAL void nc_sync_ind_ncell(T_MPHC_NCELL_SYNC_IND* sync_ind,
                             USHORT index,
                             USHORT arfcn,
                             UBYTE  bsic)
{
  GET_INSTANCE_DATA;
#ifdef GPRS
  BOOL bsic_chg = FALSE;
#endif

#if defined (TI_PS_FF_RTD) AND defined (REL99)
    /* Differentiate normal sync ind from the reconfirmation case,
     * decrement the sync req counter if it is not reconfirmation case.
     */
  if(alr_data->nc_data.cell[index].tim_valid EQ TV_RTD_TIMING_INFO AND 
      alr_data->nc_data.cell[index].c_sync_req > 0)
     alr_data->nc_data.cell[index].c_sync_req--;
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */


  if (GET_STATE(STATE_NC_PROC) EQ NC_CONFIRM)
  {
    /* eotd confirmation is active */
    if(alr_data->nc_data.eotd_avail)
      nc_check_sync_ind_eotd(sync_ind, arfcn);

    TRACE_EVENT_P2("sync_ind[%u] c_sync_req=%u", arfcn, alr_data->nc_data.c_sync_req);
    /*
     * check if the end of the CONFIRM procedure (LIST_REQ) is
     * reached
     * this is done here because it is independent from EOTD
     */
    if(alr_data->nc_data.c_sync_req EQ 0)
    {
      nc_enable_conf();
    }
    /*
     * skip the rest of this function for scell
     * because the status for the scell is always FB_SB_SYNC
     */
    if(alr_data->nc_data.eotd_avail AND
       arfcn EQ alr_data->serving_cell)
      return;
  }

  if (sync_ind->sb_flag EQ SB_FOUND)
  {
    /*
     * Synchronization successful for a normal member of the BA list.
     */
    ALR_TRACE_NC_SB_IND_PASSED(arfcn);

    ALR_EM_NEIGHBOURCELL_SB(EM_AVAIL);

  switch (GET_STATE (STATE_NC))
  {
    case NC_IDLE:
    case NC_DEDICATED:
      /*
       * check neighbourcell
       */
      switch (nc_check_bsic (index, bsic))
      {
        case NC_CHECK_OK:
          /*
           * channel has passed NCC permitted check.
           */
          switch (alr_data->nc_data.cell[index].status)
          {
            case READ_FB_SB_PENDING:
              /*
               * initial synchronization to FB and SB
               * if the cell is a 1o6 we read the BCCH
               * otherwise we just set it to synchronized
               */

              if(alr_data->nc_data.cell[index].one_of_six AND
                 GET_STATE(STATE_NC) NEQ NC_DEDICATED)
              {
                nc_store_sync_ind (index, sync_ind, READ_BCCH_RR_NOT_INFORMED);
              }
              else if(alr_data->nc_data.cell[index].one_of_twelve)
                nc_store_sync_ind (index, sync_ind, FB_SB_SYNC);
              else
              {
                TRACE_EVENT_P1("sync ind for not 1o12 cell %d", arfcn);
              }
              break;

            case READ_SB_PENDING:
              /*
               * confirmation of SB
               */
              alr_data->nc_data.cell[index].c_attempt = 0;
              if(alr_data->nc_data.cell[index].one_of_twelve)
                nc_store_sync_ind (index, sync_ind, FB_SB_SYNC);
              else
                nc_store_sync_ind (index, sync_ind, IDLE_SYNC);
              break;

            case READ_SB_BCCH_PENDING:
              /*
               * confirmation of SB before reading
               * neighbour cell BCCH all five minutes.
               */
              if(alr_data->nc_data.cell[index].one_of_six AND
                 GET_STATE(STATE_NC) EQ NC_IDLE)
              {
                nc_store_sync_ind (index, sync_ind, READ_BCCH);
              }
              else
              {
                alr_data->nc_data.cell[index].c_attempt = 0;
                if(alr_data->nc_data.cell[index].one_of_twelve)
                  nc_store_sync_ind (index, sync_ind, FB_SB_SYNC);
                else
                  nc_store_sync_ind (index, sync_ind, IDLE_SYNC);
              }
              break;
            case READ_BCCH_RR_NOT_INFORMED: /*TODO(opt)*/
            case READ_BCCH_PENDING_RR_NOT_INFORMED:/*TODO(opt)*/
            case READ_BCCH:/*TODO(opt)*/
            case READ_BCCH_PENDING:/*TODO(opt)*/
              TRACE_ERROR("sync_ind for ncell in READ_BCCH*");
              if(alr_data->nc_data.cell[index].one_of_six AND
                 GET_STATE(STATE_NC) EQ NC_IDLE)
              {
                nc_store_sync_ind (index, sync_ind, FB_SB_SYNC);
              }
              break;
            default:
              break;
          } /*switch(status)*/
          break;
        case NC_CHECK_BSIC_CHANGED:	
          /*
           * channel has passed NCC permitted check,
           * but a changed BSIC has been detected.
           */
          switch (alr_data->nc_data.cell[index].status)
          {
            case READ_SB_PENDING:
            case READ_SB_BCCH_PENDING:
              /*
               * confirmation of SB indicates new BSIC
               */
              if(alr_data->nc_data.cell[index].one_of_six AND
                 GET_STATE(STATE_NC) NEQ NC_DEDICATED)
              {
                nc_store_sync_ind (index, sync_ind, READ_BCCH_RR_NOT_INFORMED);
              }
              else if(alr_data->nc_data.cell[index].one_of_twelve)
              {
                alr_data->nc_data.cell[index].c_attempt = 0;
                nc_store_sync_ind (index, sync_ind, FB_SB_SYNC);
              }
              else
              {
                TRACE_EVENT_P1("sync ind for not 1o12 cell %d", arfcn);
                nc_store_sync_ind (index, sync_ind, IDLE_SYNC);
              }
              break;

            default:
              break;
          } /*switch(status)*/
          break;

        case NC_CHECK_NCC_FAILED:
          /*
           * ncc permitted check failed. Attempt again.
           * BSIC of a cell may always change. So do not
           * exclude this cell in IDLE mode meaurements
           */
          if(GET_STATE(STATE_NC) EQ NC_IDLE)
          {
            alr_data->nc_data.cell[index].c_attempt = 0;
            nc_set_fb_sb_failed (index, TEN_SECONDS);
          }
          else if(GET_STATE(STATE_NC) EQ NC_DEDICATED)
          {
            alr_data->nc_data.cell[index].c_attempt = 0;
            nc_set_fb_sb_failed (index, THIRTY_SECONDS);
          }
          else
          {
            nc_set_status (index, EXCLUDED);
          }
          break;
        }
        break;
#ifdef GPRS
      case NC_PIM_PBCCH:
      case NC_PTM_PBCCH: /*XXX*/
        TRACE_EVENT("sync_ind PBCCH");
        switch (alr_data->nc_data.cell[index].status)
        {
            case READ_FB_SB_PENDING:
              nc_store_sync_ind (index, sync_ind, FB_SB_SYNC);
              nc_inform_grr_of_ncell(index, GRR_SB_FOUND);
              break;
            case READ_SB_PENDING:
              if(bsic NEQ alr_data->nc_data.cell[index].bsic)
                bsic_chg = TRUE;
              else
                bsic_chg = FALSE;

              if(alr_data->nc_data.cell[index].one_of_twelve)
                nc_store_sync_ind (index, sync_ind, FB_SB_SYNC);
              else
                nc_store_sync_ind (index, sync_ind, IDLE_SYNC);

              if(bsic_chg)
                nc_inform_grr_of_ncell(index, GRR_SB_FOUND);
              break;
            default:
              break;
        }
        break;
      default:
        break;
#endif
    } /* NC_STATE */
  }
  else /* sb_flag EQ FALSE */
  {
    /*
     * Synchronization failed
     */
    ALR_TRACE_NC_SB_FAILED (sync_ind->radio_freq);
    if (alr_data->nc_data.cell[index].ba_status EQ IN_BA)
    {
      switch (alr_data->nc_data.cell[index].status)
      {
        case READ_FB_SB_PENDING:
        case READ_SB_PENDING:
        case READ_SB_BCCH_PENDING:

#if defined (TI_PS_FF_RTD) AND defined (REL99)
         /* 
          * NC sync with all RTD values for that ARFCN has failed,
          * need to try with normal sync with out RTD value.If any
          * of the sync indication have succeded then status would 
          * changed to BCCH reading.
          */
          if(alr_data->nc_data.cell[index].c_sync_req EQ 0 AND 
            alr_data->nc_data.cell[index].tim_valid EQ TV_RTD_TIMING_INFO)
          {
            if(alr_data->nc_data.c_sync_req < MAX_L1_SYNC_CNT)
            {
              /* invalidate the rtd information because we couldn't sync using this values */
              alr_data->nc_data.emr_data.enh_para.enh_cell_list[index].v_rtd = 0;
              alr_data->nc_data.cell[index].tim_valid = TV_INVALID_TIMING_INFO;
              nc_build_sync_req(index);
              ALR_TRACE_NC ("ncell_sync_req");
            } /*if*/
          } 
          if(alr_data->nc_data.cell[index].c_sync_req EQ 0 AND 
            (alr_data->nc_data.cell[index].tim_valid NEQ TV_RTD_TIMING_INFO ))
          {
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */


          ALR_EM_NEIGHBOURCELL_SB(EM_NOT_AVAIL);

          if(GET_STATE(STATE_NC) EQ NC_IDLE)
            nc_sync_failed_attempt(index);
          else if(GET_STATE(STATE_NC) EQ NC_DEDICATED)
            nc_sync_failed_attempt_dedicated(index);
#ifdef GPRS /*XXX*/
          else
            nc_sync_failed_gprs(index);
#endif
#if defined (TI_PS_FF_RTD) AND defined (REL99)
          }
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */

          break;
        default:
          break;
      }
    }
    else
    { /*
       * the cell currently or perhaps for a longer time doesn't
       * belong to the BA list, no more attempts to synchronize now
       */
      switch (alr_data->nc_data.cell[index].status)
      {
        case READ_FB_SB_PENDING:
          nc_set_status (index, IDLE);
          break;
        case READ_SB_PENDING:
          alr_data->nc_data.cell[index].c_attempt++;
          alr_data->nc_data.cell[index].tim_valid = TV_APPROX_TIMING_INFO;
          nc_set_status (index, READ_SB);
          break;
        case READ_SB_BCCH_PENDING:
          alr_data->nc_data.cell[index].c_attempt++;
          alr_data->nc_data.cell[index].tim_valid = TV_APPROX_TIMING_INFO;
          nc_set_status (index, READ_SB_BCCH);
          break;
        default:
          break;
      }
    }
  } /* error_flag    check */

#if defined (TI_PS_FF_RTD) AND defined (REL99)
  if((alr_data->nc_data.nxt_ind NEQ 0) AND (alr_data->nc_data.cell[index].tim_valid EQ TV_RTD_TIMING_INFO))
    SET_STATE(STATE_NC_PROC,NC_ACQUIRE);
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_check_sync_ind_eotd     |
+--------------------------------------------------------------------+

  PURPOSE : copies the measurement results to the MPH_NCELL_POS_IND
            while EOTD is active

*/
LOCAL void nc_check_sync_ind_eotd (T_MPHC_NCELL_SYNC_IND* sync_ind, USHORT arfcn)
{
  GET_INSTANCE_DATA;
  T_MPH_NCELL_POS_IND* ppos_ind;

  if ((ppos_ind = alr_data->nc_data.ppos_ind) EQ NULL)
  {
    TRACE_ERROR("nc_check_sync_ind_eotd() while alr_data->nc_data.ppos_ind==0");
    return;
  }

  if (arfcn EQ alr_data->serving_cell)
  {
    if (ppos_ind->eotd_res EQ 0) /*0 doesn't mean EOTD_SUCC here
                                   - it indicates that no SC results are present*/
    {
      nc_store_eotd(&(ppos_ind->eotd_sc_res), sync_ind, arfcn);
      ppos_ind->fn = sync_ind->fn_in_sb;
      ppos_ind->eotd_res++;
    }
    else  /*first EOTD results for SC are already present
           - now continue with second SC results*/
    {
      nc_store_eotd((T_eotd_sc_res*)(&(ppos_ind->eotd_sc_res1)), sync_ind,
                    arfcn);
      ppos_ind->eotd_res = EOTD_SUCC;
      ppos_ind->ta       = GET_STATE(STATE_NC) EQ NC_DEDICATED ?
                                alr_data->nc_data.tav : NOT_PRESENT_8BIT;
      PSENDX (RR, ppos_ind);
      alr_data->nc_data.ppos_ind = NULL;

      if (alr_data->nc_data.ppos_req NEQ NULL)
      {
        PFREE(alr_data->nc_data.ppos_req);
        alr_data->nc_data.ppos_req = NULL;
      }
    }
  }
  else
  {
    if (ppos_ind->c_eotd_nc_res < MAX_NCELL_EOTD_RES)
    {
      nc_store_eotd((T_eotd_sc_res*)(
        &(ppos_ind->eotd_nc_res[ppos_ind->c_eotd_nc_res])) , sync_ind, arfcn);
      (ppos_ind->c_eotd_nc_res)++;
    }
    else
      TRACE_ERROR("more than MAX_NCELL_EOTD_RES ncell sync indications");
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_sync_failed_attempt     |
+--------------------------------------------------------------------+

  PURPOSE : A synchronisation attempt has failed during idle mode.

*/
LOCAL void nc_sync_failed_attempt (USHORT index)
{
  GET_INSTANCE_DATA;
  switch (alr_data->nc_data.cell[index].c_attempt)
  {
    case 0:
    case 1:
    case 2:
    case 3:
      /*
       * for attempts 1 to 4 try it again
       * after ten seconds
       */
      nc_set_fb_sb_failed (index, TEN_SECONDS);
       
      break;
    default:
      /*
       * if there are more attempts, exclude
       * the cell from further attempts
       * Store last fieldstrength value
       */
      nc_set_status (index, EXCLUDED);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103) MODULE  : ALR_NC                           |
| STATE   : code          ROUTINE : nc_sync_failed_attempt_dedicated |
+--------------------------------------------------------------------+

  PURPOSE : A synchronisation attempt has failed during dedicated mode.

*/
LOCAL void nc_sync_failed_attempt_dedicated (USHORT index)
{
  GET_INSTANCE_DATA;
  /*
   * Store last fieldstrength value
   */
  switch (alr_data->nc_data.cell[index].c_attempt)
  {
    case 0:
    case 1:
    case 2:
      /*
       * for attempts 1 to 3 try it again immediately
       */
      alr_data->nc_data.cell[index].c_attempt++;
      if (alr_data->nc_data.cell[index].status EQ READ_FB_SB_PENDING)
        alr_data->nc_data.cell[index].tim_valid = TV_INVALID_TIMING_INFO;
      else
        alr_data->nc_data.cell[index].tim_valid = TV_APPROX_TIMING_INFO;
      nc_set_status (index, READ_FB_SB);
      break;
    default:
      /*
       * if there are more attempts, exclude
       * the cell from further attempts
       */
      nc_set_status (index, EXCLUDED);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_start_dedicated         |
+--------------------------------------------------------------------+

  PURPOSE : Process signal nc_start_dedicated from SDL process
            Main_Control.

*/

GLOBAL void nc_start_dedicated (UBYTE pwrc, UBYTE dtx)
{
  GET_INSTANCE_DATA;
  USHORT i;

  switch(GET_STATE(STATE_NC))
  {
    case NC_CON_EST:
      for (i = 0; i < alr_data->nc_data.c_ba_arfcn; i++)
      {
        switch (alr_data->nc_data.cell[i].status)
        {
          case READ_BCCH_RR_NOT_INFORMED:
          case FB_SB_SYNC_RR_NOT_INFORMED:
          case READ_BCCH:
            /*
             * BCCH reading is not needed during dedicated mode,
             * but SB synchronisation is already done.
             * clear a stored BCCH if needed.
             */
            nc_set_status (i, FB_SB_SYNC);
            break;

          case READ_SB_BCCH:
            nc_set_status (i, READ_SB);
            break;

          case READ_FB_SB_PENDING:
          case READ_SB_PENDING:
          case READ_SB_BCCH_PENDING:
          case READ_BCCH_PENDING_RR_NOT_INFORMED:
          case READ_BCCH_PENDING:
            TRACE_ERROR("Abnormal situation nc_start_dedi");
            nc_set_status (i, IDLE);
            break;
          case FB_SB_FAILED:
            nc_set_status (i, IDLE);
            break;
          case IDLE_SYNC:
            if (alr_data->nc_data.cell[i].c_sync > 0)
              alr_data->nc_data.cell[i].c_sync = TEN_SECONDS;
            break;
          default:
            break;
        }
      }

      nc_clear_last_bsic();

      alr_data->nc_data.pwrc = pwrc;
      alr_data->nc_data.dtx = dtx;

      SET_STATE (STATE_NC, NC_DEDICATED);
      nc_enable_conf();
      break;
    default:
      TRACE_ERROR("nc_start_dedicated in wrong state");
      break;
  }

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_suspend                 |
+--------------------------------------------------------------------+

  PURPOSE : This stops all active processes like FB/SB and BCCH reading
            if
            - the mobile start connection establishment
            - RR requests power measurements parallel to idle mode
            - leave dedicated mode.

*/

GLOBAL void nc_suspend (void)
{
  GET_INSTANCE_DATA;
  ALR_TRACE_NC( "nc_suspend");

  switch (GET_STATE(STATE_NC))
  {
    case NC_IDLE:
    case NC_DEDICATED:
      nc_clear_last_bsic();
      nc_stop_all();
      nc_disable_conf(FREE_POS_IND);

      SET_STATE(STATE_NC, NC_CON_EST);
      break;
    case NC_CON_EST:
      break;
#ifdef GPRS
    case NC_PIM_PBCCH:
      if(alr_data->nc_data.cell[LAST_BSIC_REQ].status NEQ IDLE)
        nc_clear_last_bsic();
      /*lint -fallthrough*/
    case NC_PTM_PBCCH:
      nc_stop_all();
      nc_disable_conf(FREE_POS_IND);
      SET_STATE(STATE_NC, NC_CON_EST);
      break;
#endif
    default:
      break;
  }
}

GLOBAL void nc_suspend_handover (void)
{
  GET_INSTANCE_DATA;
        
  ALR_TRACE_NC( "nc_suspend_handover");

  switch (GET_STATE(STATE_NC))
  {
    case NC_DEDICATED:
      nc_stop_all();
      nc_disable_conf(FREE_POS_IND);
      break;
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : alr_nc                     |
| STATE   : code                ROUTINE : nc_resume                  |
+--------------------------------------------------------------------+

  PURPOSE : Only needed for alr_gprs.c

*/

GLOBAL void nc_resume (void)
{
  GET_INSTANCE_DATA;

  SET_STATE (STATE_NC, NC_IDLE);
  nc_enable_conf();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_update_dedicated        |
+--------------------------------------------------------------------+

  PURPOSE : Process signal nc_update_dedicated from SDL process
            Dedi_Control.

*/

GLOBAL void nc_update_dedicated (UBYTE dtx, UBYTE pwrc)
{
  GET_INSTANCE_DATA;
  UBYTE i, j;
  UBYTE   c_ba_arfcn = alr_data->nc_data.c_ba_arfcn;

  switch(GET_STATE (STATE_NC))
  {
    case NC_DEDICATED:
      {
        PALLOC (upd_dedi, MPHC_UPDATE_BA_LIST);

        /*
         * updated values of pwrc or dtx can be configure
         * only by sending a new neighbourcell list to layer 1
         */
        alr_data->nc_data.pwrc = pwrc;
        alr_data->nc_data.dtx = dtx;

        for (i = 0, j = 0; i < c_ba_arfcn; i++)
        {
          if (alr_data->nc_data.cell[i].ba_status EQ IN_BA)
            upd_dedi->chan_list.radio_freq[j++] =
              ARFCN_TO_L1(alr_data->nc_data.cell[i].ba_arfcn);
        }

        upd_dedi->num_of_chans = j;
        alr_data->nc_data.ba_id = ALR_ALLOCATE_NEW_BA ( alr_data->nc_data.ba_id );
        upd_dedi->ba_id = alr_data->nc_data.ba_id;
        upd_dedi->pwrc = alr_data->nc_data.pwrc;
        upd_dedi->dtx_allowed = alr_data->nc_data.dtx;

        alr_data->nc_data.update = TRUE;
        ma_nc_update_ba_list (upd_dedi);
      }
      break;
    default:
      TRACE_ERROR("nc_update_dedicated in wrong state");
      break;
  }
}

GLOBAL void nc_resume_dedicated(void)
{
  nc_enable_conf();
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_get_fn_time             |
+--------------------------------------------------------------------+

  PURPOSE : Export Procedure to request frame offset and time
            alignment of neighbour cells.

            Returns FALSE if timing information not found.

*/

GLOBAL BOOL nc_get_fn_time (USHORT channel,
                            ULONG *fn,
                            ULONG *time)
{
  GET_INSTANCE_DATA;
  USHORT index;
  if( channel EQ alr_data->serving_cell)
  {
    *fn = 0;
    *time = 0;
    return TRUE;
  }

  /* Check if we have read SYNC on this channel */
  index = nc_get_index (channel);
  if (index NEQ NOT_PRESENT_16BIT)
  {
    *fn = alr_data->nc_data.cell[index].frame_offset;
    *time = alr_data->nc_data.cell[index].time_align;

    if(alr_data->nc_data.cell[index].tim_valid EQ TV_VALID_TIMING_INFO)
    {
#if defined (REL99) && defined (FF_BHO)
      TRACE_EVENT_P1("ARFCN %d - Sync Info Present in NC data", channel);
#endif
      return TRUE;
    }
    else
    {
#if defined (REL99) && defined (FF_BHO)
      alr_data->dedi_data.bho_cell_index = index;
      TRACE_EVENT_P1("ARFCN %d - Sync Info NOT Present in NC data", channel);
#endif
      return FALSE;
    }
  }
  else
  {
    *fn = NOT_PRESENT_32BIT;
    *time = NOT_PRESENT_32BIT;
#if defined (REL99) && defined (FF_BHO)
    alr_data->dedi_data.bho_cell_index = index;
    TRACE_EVENT_P1("ARFCN %d - NOT Present in NC data", channel);
#endif
    return FALSE;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_start_reselect          |
+--------------------------------------------------------------------+

  PURPOSE : Process signal nc_start_reselect from SDL process
            Main_Control.

*/

GLOBAL void nc_start_reselect (USHORT arfcn)
{
  GET_INSTANCE_DATA;
  USHORT  index;
  T_NC    *pcell;

  ALR_TRACE_NC_RESELECT(arfcn);

  index = nc_get_index (arfcn);
  if (index EQ NOT_PRESENT_16BIT)
  {
    if (arfcn NEQ alr_data->nc_data.cr_cell.ba_arfcn)
    {
      ma_error_ind (CS_BCCH_READ_ERROR, arfcn);
      return;
    }
    else
    {
      pcell = &alr_data->nc_data.cr_cell;
    }
  }
  else
  {
    pcell = &alr_data->nc_data.cell[index];

    /* Cannot to reselect to a cell for which the synchronization information
     * is not available
     */
    if (pcell->bsic EQ NOT_PRESENT_8BIT)
    {
       ma_error_ind (CS_NC_SYNC_FAILED, arfcn);
       return;
    }
  }


  {
    PALLOC(reselect, MPHC_NEW_SCELL_REQ);

    reselect->radio_freq = ARFCN_TO_L1(arfcn);
    reselect->fn_offset = (ULONG)pcell->frame_offset;
    reselect->time_alignment = (ULONG)pcell->time_align;
    reselect->tsc = pcell->bsic;
    alr_data->nc_data.channel = arfcn;

    ALR_TRACE_NC_FN_TA(index, reselect->fn_offset, reselect->time_alignment);

    ALR_EM_CONFIGURE_CELL_RESELECTION;

    ma_new_scell_req(reselect);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_report                  |
+--------------------------------------------------------------------+

  PURPOSE : Process signal nc_report from SDL process
            Main_Control.

*/

GLOBAL void nc_report (T_MPHC_RXLEV_PERIODIC_IND *rxlev_periodic_ind)
{
  GET_INSTANCE_DATA;
        
  ALR_TRACE_NC("nc_report");


  switch(GET_STATE(STATE_NC))
  {
    case NC_IDLE:
    case NC_CON_EST:
      nc_store_rxlev (rxlev_periodic_ind);

#ifdef GPRS
      nc_rxlev_sc_req (rxlev_periodic_ind->s_rxlev);
#endif
      nc_check_activity();
      break;
    default:
      TRACE_ERROR("nc_report in invalid state");
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_report_dedicated        |
+--------------------------------------------------------------------+

  PURPOSE : Process signal nc_report_dedicated from SDL process
            Main_Control.

*/

GLOBAL void nc_report_dedicated (T_MPHC_MEAS_REPORT *meas_report)
{
  GET_INSTANCE_DATA;
        
  switch(GET_STATE(STATE_NC))
  {
    case NC_DEDICATED:
      nc_store_dedicated (meas_report);
      nc_check_activity   ();
      break;
    default:
      TRACE_ERROR("nc_report_dedicated in wrong state");
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_update_list             |
+--------------------------------------------------------------------+

  PURPOSE : Procedure to update frame offset and time alignment after
            cell change. All values are relative to the actual serving
            cell.

*/

#define TDMA_FRAMES_PER_HYPERFRAME  2715648

GLOBAL void nc_update_list (USHORT channel)
{
  GET_INSTANCE_DATA;
  USHORT index;
  USHORT i;

  switch(GET_STATE(STATE_NC))
  {
    case NC_CON_EST:
    case NC_DEDICATED:
    case NC_IDLE:
      index = nc_get_index (channel);
      if (index NEQ NOT_PRESENT_16BIT)
      {
        for (i = 0; i < alr_data->nc_data.c_ba_arfcn; i++)
        {
          if ((i NEQ index) AND (alr_data->nc_data.cell[i].tim_valid NEQ TV_INVALID_TIMING_INFO))
          {
            ULONG new_frame_offset;
            ULONG new_time_align;

            new_frame_offset = (alr_data->nc_data.cell[i].frame_offset -
                                alr_data->nc_data.cell[index].frame_offset +
                                TDMA_FRAMES_PER_HYPERFRAME) %
                               TDMA_FRAMES_PER_HYPERFRAME;
            if (alr_data->nc_data.cell[i].time_align >= alr_data->nc_data.cell[index].time_align)
            {
              new_time_align = alr_data->nc_data.cell[i].time_align -
                               alr_data->nc_data.cell[index].time_align;
            }
            else
            {
              new_time_align = 5000 + alr_data->nc_data.cell[i].time_align -
                               alr_data->nc_data.cell[index].time_align;
              new_frame_offset = (new_frame_offset + 1) %
                                 TDMA_FRAMES_PER_HYPERFRAME;
            } /* time_align */
            alr_data->nc_data.cell[i].frame_offset = new_frame_offset;
            alr_data->nc_data.cell[i].time_align = new_time_align;
          } /* i NEQ index */
        } /* for(all ncells) */

        ALR_TRACE_NC("set chan to 0");

        alr_data->nc_data.cell[index].frame_offset = 0;
        alr_data->nc_data.cell[index].time_align = 0;
      } /* NOT_PRESENT */
      break;
    default:
      TRACE_ERROR("nc_update_list in invalid state");
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_add_offset              |
+--------------------------------------------------------------------+

  PURPOSE : If the mobile comes back from dedicated mode all synchronized
            neighbourcells make a status transition to READ_BCCH_RR_NOT_INFORMED.
            This triggers reading of the neighbourcell BCCH.

*/

GLOBAL void nc_add_offset (void)
{
  GET_INSTANCE_DATA;
  USHORT i;
  USHORT  c_ba_arfcn = alr_data->nc_data.c_ba_arfcn;

  for (i = 0; i < c_ba_arfcn; i++)
  {
    if (alr_data->nc_data.cell[i].ba_arfcn NEQ alr_data->serving_cell)
    {
      /*XXX c_sync should be the same in idle and dedic also for pbcchc
        so no action here */
      switch (alr_data->nc_data.cell[i].status)
      {
        case FB_SB_SYNC:
        case READ_SB:
          nc_set_status (i, READ_BCCH_RR_NOT_INFORMED);
          break;
        default:
          break;

      }
    }
    else
      nc_set_status (i, FB_SB_SYNC);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_fill_report_sc_dedi     |
+--------------------------------------------------------------------+

  PURPOSE : Fills the serving cell values for a measurement report.

*/

GLOBAL void nc_fill_report_sc_dedi (T_MPH_MEASUREMENT_IND *rr_report,
                                    UBYTE ncells)
{
  GET_INSTANCE_DATA;
  rr_report->valid = TRUE;
  rr_report->ncells.no_of_ncells = ncells;
  rr_report->arfcn = alr_data->serving_cell;
  rr_report->fn_offset = 103;   /* average of SDCCH/FACCH */
  rr_report->dtx = alr_data->nc_data.act_dtx;
  rr_report->rx_lev_full = (UBYTE)alr_data->nc_data.rxlev_full;
  if (rr_report->rx_lev_full > 63)
    rr_report->rx_lev_full = 63;
  rr_report->rx_lev_sub = (UBYTE)alr_data->nc_data.rxlev_sub;
  if (rr_report->rx_lev_sub > 63)
    rr_report->rx_lev_sub = 63;
  rr_report->rx_qual_full = alr_data->nc_data.rxqual_full;
  rr_report->rx_qual_sub = alr_data->nc_data.rxqual_sub;
  rr_report->otd = alr_data->nc_data.tav;

  if(ncells EQ 0)
  {
    /* 
     * rxlev of SC BCCH channel is not avail
     * use approx value (rxlev_sub)
     */
    rr_report->bcch_rxlev_of_sc = rr_report->rx_lev_sub;
  }
  else
  {
    USHORT index = nc_get_index(alr_data->serving_cell);
    if(index EQ NOT_PRESENT_16BIT)
    {
      TRACE_EVENT("Error SC not present in BA list");
      rr_report->bcch_rxlev_of_sc = rr_report->rx_lev_sub;
    }
    else
    {
      if((UBYTE) alr_data->nc_data.cell[index].rxlev_average > 63)
        rr_report->bcch_rxlev_of_sc = 63;
      else
        rr_report->bcch_rxlev_of_sc = (UBYTE)alr_data->nc_data.cell[index].rxlev_average;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : inform GRR                 |
+--------------------------------------------------------------------+

  PURPOSE : inform GRR

*/

#ifdef GPRS
LOCAL void nc_inform_grr_of_ncell (USHORT index, UBYTE type)
{
  GET_INSTANCE_DATA;
  PALLOC(rr_rep, MPH_MEASUREMENT_IND);
  rr_rep->ncells.no_of_ncells = 1;
  rr_rep->ncells.arfcn[0] = alr_data->nc_data.cell[index].ba_arfcn;
  if(type EQ GRR_SB_FOUND)
    rr_rep->ncells.bsic[0]  =  alr_data->nc_data.cell[index].bsic;
  else
    rr_rep->ncells.bsic[0]  = type;
  rr_rep->gprs_sync = SYNC_RESULTS;
  PSENDX(RR,rr_rep);
}

LOCAL void remove_ncell_and_inform_grr (USHORT index)
{
  nc_inform_grr_of_ncell (index, GRR_SB_UNKNOWN);
  nc_remove_channel_from_ba_list(index);
}
#endif


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_build_rr_report         |
+--------------------------------------------------------------------+

  PURPOSE : Fills a measurement report to RR with neighbour cell
            information.

*/

LOCAL void nc_build_rr_report (T_MPH_MEASUREMENT_IND *rr_report)
{
  GET_INSTANCE_DATA;
  UBYTE c_report;
  UBYTE c_ba_list;
  T_NC * pcell = alr_data->nc_data.cell;

  c_report = 0;
  rr_report->valid = TRUE;

  for (c_ba_list = 0; c_ba_list < alr_data->nc_data.c_ba_arfcn AND
       c_report < MAX_RR_NCELL_CNT; c_ba_list++, pcell++)
  {
    if(pcell->one_of_six AND pcell->ba_status EQ IN_BA)
    {
      switch (pcell->status)
      {
        case FB_SB_SYNC_RR_NOT_INFORMED:
          /*
           * the cell is synchronized but RR has no prior
           * knowledge of the cell, but we may have already
           * read data from the ncell BCCH. This check is performed
           * after sending the measurement report
           */
        case FB_SB_SYNC:
        case READ_BCCH:
        case READ_SB:
        case READ_SB_BCCH:
          /*
           * we may be currently reading this SB/FB or BCCH of the cell
           * pass up this cell also
           * if READ_FB_SB_PENDING this may be the first time that the
           * cell is read so check if the bsic of this cell has been
           * read already
           */
        case READ_SB_PENDING:
        case READ_SB_BCCH_PENDING:
        case READ_BCCH_PENDING:
        case READ_FB_SB_PENDING:
          if (pcell->ba_arfcn NEQ alr_data->serving_cell AND
              pcell->bsic NEQ NOT_PRESENT_8BIT)
          {
            rr_report->ncells.arfcn[c_report] = pcell->ba_arfcn;
            rr_report->ncells.rx_lev[c_report] = (UBYTE)pcell->rxlev_average;
  #if defined(_SIMULATION_)
            TRACE_EVENT_P3 ("rx_lev%u[%u]=%d",
              c_report, rr_report->ncells.arfcn[c_report],
              rr_report->ncells.rx_lev[c_report]);
  #endif  /* _SIMULATION_ */
            rr_report->ncells.bsic[c_report] = pcell->bsic;
            rr_report->ncells.time_alignmt[c_report] = pcell->time_align;
            rr_report->ncells.frame_offset[c_report++] = pcell->frame_offset;
          }
          break;
        case READ_BCCH_RR_NOT_INFORMED:
        case READ_BCCH_PENDING_RR_NOT_INFORMED:
        case READ_FB_SB:
          /* do nothing */
        default:
        break;
      }
    }/* not one_of_six*/
  }
  rr_report->ncells.no_of_ncells = c_report;
#ifdef FF_PS_RSSI
  rr_report->rx_qual_full = alr_data->nc_data.rxqual_full;
#endif
#if defined (REL99) && defined (TI_PS_FF_EMR)
  /*Further, if we are reporting 6 strongest and if report type is enhanced.....*/
  if ( (c_report) AND (alr_data->nc_data.emr_data.rep_type EQ REP_TYPE_ENH) )
  {
    /*...add the other cells in BA to the report: this can be used by GRR for P-EMR*/
    pcell = alr_data->nc_data.cell;
    for (c_ba_list = c_report; c_ba_list < alr_data->nc_data.c_ba_arfcn AND
       c_report < MAX_NUM_CELLS_IN_REPORT; c_ba_list++, pcell++)
    {
      /*All those which are not in six strongest*/
      if ( (!pcell->one_of_six) AND pcell->ba_status EQ IN_BA)
      {
        if (pcell->ba_arfcn NEQ alr_data->serving_cell AND 
          pcell->bsic NEQ NOT_PRESENT_8BIT)
        {
          rr_report->ncells.arfcn[c_report] = pcell->ba_arfcn;
          rr_report->ncells.rx_lev[c_report] = (UBYTE)pcell->rxlev_average;
  #if defined(_SIMULATION_)
        TRACE_EVENT_P3 ("rx_lev%u[%u]=%d",
          c_report, rr_report->ncells.arfcn[c_report],
          rr_report->ncells.rx_lev[c_report]);
  #endif  /* _SIMULATION_ */
          rr_report->ncells.bsic[c_report] = pcell->bsic;
          rr_report->ncells.time_alignmt[c_report] = pcell->time_align;
          rr_report->ncells.frame_offset[c_report++] = pcell->frame_offset;
        }
      }
    }/*for 'c_report'*/
    rr_report->ncells.no_of_ncells = c_report;
  }/*rep_type = REP_TYPE_ENH*/
#endif
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_build_nwctrl_rr_report  |
+--------------------------------------------------------------------+

  PURPOSE : Fills a measurement report to RR with neighbour cell
            information.

*/


#ifdef GPRS
GLOBAL void nc_build_nwctrl_rr_report (T_MPH_MEAS_REP_CNF *rr_report)
{
  GET_INSTANCE_DATA;
  UBYTE  c_report;
  UBYTE  c_ba_list;
  USHORT index;
  T_NC * pcell;

  /*
   * Store the rxlev average  for the serving cell
   */
  index = nc_get_index(alr_data->serving_cell);
  if ( index NEQ NOT_PRESENT_16BIT )
  {
    pcell = &alr_data->nc_data.cell[index];
    rr_report->meas_rep[0].arfcn   = alr_data->serving_cell;
    rr_report->meas_rep[0].bsic    = pcell->bsic;

    if(pcell->c_nc_rxlev)
    {
      rr_report->meas_rep[0].rx_lev = pcell->nc_rxlev/pcell->c_nc_rxlev;
    }
#if defined(_SIMULATION_)
    TRACE_EVENT_P3 ("nw_sc__rxlev[%u]=acc : %d, count:%d",rr_report->meas_rep[0].arfcn,pcell->nc_rxlev,pcell->c_nc_rxlev);
#endif  /* _SIMULATION_ */
  }

  c_report = 0;
  pcell = alr_data->nc_data.cell;

  for (c_ba_list = 0; c_ba_list < alr_data->nc_data.c_ba_arfcn AND
       c_report < MAX_RR_NCELL_CNT; c_ba_list++, pcell++)
  {
    if(pcell->one_of_six AND pcell->ba_status EQ IN_BA)
    {
      switch (pcell->status)
      {
        case FB_SB_SYNC_RR_NOT_INFORMED:
        /*
         * the cell is synchronized but RR has no prior
         * knowledge of the cell, but we may have already
         * read data from the ncell BCCH. This check is performed
         * after sending the measurement report
         */
        case FB_SB_SYNC:
        case READ_BCCH:
        case READ_SB:
        case READ_SB_BCCH:
        /*
         * we may be currently reading this SB/FB or BCCH of the cell
         * pass up this cell also
         * if READ_FB_SB_PENDING this may be the first time that the
         * cell is read so check if the bsic of this cell has been
         * read already
         */
        case READ_SB_PENDING:
        case READ_SB_BCCH_PENDING:
        case READ_BCCH_PENDING:
        case READ_FB_SB_PENDING:
          if (pcell->ba_arfcn NEQ alr_data->serving_cell AND
              pcell->bsic NEQ NOT_PRESENT_8BIT)
          {
            rr_report->meas_rep[c_report+1].arfcn = pcell->ba_arfcn;
            rr_report->meas_rep[c_report+1].bsic   = pcell->bsic;
            if(pcell->c_nc_rxlev)
            {
	      rr_report->meas_rep[c_report+1].rx_lev = pcell->nc_rxlev/pcell->c_nc_rxlev;
            }

#if defined(_SIMULATION_)
            TRACE_EVENT_P3 ("nwctrl_rxlev%u[%u]=%d",c_report+1, rr_report->meas_rep[c_report+1].arfcn,rr_report->meas_rep[c_report+1].rx_lev);
#endif  /* _SIMULATION_ */
            c_report++;
          }
          break;
        case READ_BCCH_RR_NOT_INFORMED:
        case READ_BCCH_PENDING_RR_NOT_INFORMED:
        case READ_FB_SB:
          /* do nothing */
        default:
          break;
      }
    }/* not one_of_six*/
  }

  if( (c_report + 1 )  < RR_ALR_MEAS_REPORT_SIZE )
  {
    rr_report->meas_rep[c_report+1].arfcn  = NOT_PRESENT_16BIT ;
    rr_report->meas_rep[c_report+1].rx_lev = NOT_PRESENT_8BIT ;
    rr_report->meas_rep[c_report+1].bsic   = NOT_PRESENT_8BIT ;
  }
}
#endif /* ifdef GPRS */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_store_tav               |
+--------------------------------------------------------------------+

  PURPOSE : Stores timing advance receveived with dedicated mode SI's
*/
GLOBAL void nc_store_tav(USHORT tav)
{
  GET_INSTANCE_DATA;
  alr_data->nc_data.tav = tav;
}


/*
 *========================================================================
 * Helper Functions
 *========================================================================
 */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_check_status            |
+--------------------------------------------------------------------+

  PURPOSE : Checks the status of the neighbour cells.
*/

LOCAL void nc_check_status (UBYTE start_list)
{
  GET_INSTANCE_DATA;
  USHORT i;
  USHORT c_ba_arfcn = alr_data->nc_data.c_ba_arfcn;
  T_NC * pcell = alr_data->nc_data.cell;

  for (i = 0; i < c_ba_arfcn; i++, pcell++)
  {
    if (pcell->ba_arfcn NEQ alr_data->serving_cell)
    {
      switch (GET_STATE(STATE_NC))
      {
        case NC_DEDICATED:
          switch (pcell->status)
          {
            case INACTIVE:
              break;
            case IDLE:
              if (pcell->one_of_twelve)
                nc_set_status (i, READ_FB_SB);
              if (start_list EQ CHECK_FOR_CONFIRM)
              {
                if (pcell->tim_valid EQ TV_VALID_TIMING_INFO)
                  pcell->tim_valid = TV_APPROX_TIMING_INFO;
                else if (pcell->tim_valid EQ TV_APPROX_TIMING_INFO)
                  pcell->tim_valid = TV_INVALID_TIMING_INFO;
              }
              break;
            case READ_FB_SB:
              if (pcell->one_of_twelve EQ FALSE)
              {
                nc_set_status (i, IDLE);
                if (start_list EQ CHECK_FOR_CONFIRM AND pcell->tim_valid EQ TV_APPROX_TIMING_INFO)
                  pcell->tim_valid = TV_INVALID_TIMING_INFO;
              }
              break;
            case FB_SB_SYNC:
              if (pcell->one_of_twelve EQ FALSE)
              {
                nc_set_status (i, IDLE_SYNC);
              }
              else
              {
                if ( (start_list) AND (alr_data->nc_data.c_sync_intrupted EQ FALSE))
                  nc_set_status (i, READ_SB);
              }
              break;
            case IDLE_SYNC:
              if (pcell->one_of_twelve)
              {
                if (pcell->c_sync EQ 0)
                  nc_set_status (i, READ_FB_SB);
                else
                  nc_set_status (i, FB_SB_SYNC);
              }
              else
              {
                if (pcell->c_sync EQ 0)
                {
                  nc_set_status (i, IDLE);
                }
              }
              break;
            case READ_SB:
              if (pcell->one_of_twelve EQ FALSE)
              {
                pcell->tim_valid = TV_APPROX_TIMING_INFO;
                nc_set_status (i, IDLE);
              }
              break;
            case READ_BCCH_RR_NOT_INFORMED:
            case READ_BCCH:
            case READ_SB_BCCH:
              TRACE_EVENT("unexpected STATE_NC during NC_DEDICATED");
              if (pcell->one_of_twelve)
                nc_set_status (i, READ_FB_SB);
              else
              {
                pcell->tim_valid = TV_APPROX_TIMING_INFO;
                nc_set_status (i, IDLE);
              }
              break;

            case FB_SB_FAILED:
              if (pcell->one_of_twelve EQ FALSE)
                nc_set_status (i, IDLE);
              else if (pcell->c_sync EQ 0)
                nc_set_status (i, READ_FB_SB);
              break;

            default:
              break;
          }
          break;
        case NC_IDLE:
          switch (pcell->status)
          {
            case INACTIVE:
              break;
            case IDLE:
              if (pcell->one_of_twelve)
#if 0	//Nina modify to avoid frequent NSY_R
                nc_set_status (i, READ_FB_SB);
#else
	      {
		if (pcell->rank_of_the_six < 2)
                  nc_set_status (i, READ_FB_SB);
		else
		{
		  pcell->one_of_six = 0;
		  pcell->one_of_twelve =0;
		}
	      }
#endif
              if (start_list EQ CHECK_FOR_CONFIRM)
              {
                if (pcell->tim_valid EQ TV_VALID_TIMING_INFO)
                  pcell->tim_valid = TV_APPROX_TIMING_INFO;
                else if (pcell->tim_valid EQ TV_APPROX_TIMING_INFO)
                  pcell->tim_valid = TV_INVALID_TIMING_INFO;
              }
              break;
            case READ_FB_SB:
              if (pcell->one_of_twelve EQ FALSE)
              {
                nc_set_status (i, IDLE);
                if (start_list EQ CHECK_FOR_CONFIRM AND pcell->tim_valid EQ TV_APPROX_TIMING_INFO)
                  pcell->tim_valid = TV_INVALID_TIMING_INFO;
              }
              break;
            case FB_SB_SYNC:
              if (pcell->one_of_twelve EQ FALSE)
              {
                nc_set_status (i, IDLE_SYNC);
              }
              else
              {
                if (start_list)
                  nc_set_status (i, READ_SB);
                if (pcell->one_of_six AND
                    pcell->c_bcch EQ 0)
                  nc_set_status (i, READ_SB_BCCH);
              }
              break;
            case FB_SB_SYNC_RR_NOT_INFORMED:
              if (pcell->one_of_six EQ FALSE)
              {
                if (pcell->one_of_twelve EQ FALSE)
                {
                  nc_set_status (i, IDLE_SYNC);
                }
                else
                {
                  nc_set_status (i, FB_SB_SYNC);
                }
              }
              break;
            case FB_SB_FAILED:
	      //Nina add to judge the syn failure times
	      if(pcell->syn_failure_times > 2)
	      {
		nc_set_status (i, EXCLUDED);
		break;
	      }
              if (pcell->one_of_twelve EQ FALSE)
                nc_set_status (i, IDLE);
              else if (pcell->c_sync EQ 0)
	      {
              	if(pcell->rank_of_the_six < 4)
		  nc_set_status (i, READ_FB_SB);
		else
		{
		  pcell->one_of_six = 0;
		  pcell->one_of_twelve =0;
		}
              }
              break;
            case IDLE_SYNC:
              if (pcell->one_of_twelve)
              {
                if(pcell->one_of_six)
                {
                  if (pcell->c_sync EQ 0)
                    nc_set_status (i, READ_FB_SB);
                  else
                    nc_set_status (i, READ_BCCH_RR_NOT_INFORMED);
                }
                else
                {
                  if (pcell->c_sync EQ 0)
                    nc_set_status (i, READ_FB_SB);
                  else
                    nc_set_status (i, FB_SB_SYNC);
                }
              }
              else
              {
                if (pcell->c_sync EQ 0)
                {
                  pcell->tim_valid = TV_APPROX_TIMING_INFO;
                  nc_set_status (i, IDLE);
                }
              }
              break;
            case READ_SB:
              if (pcell->one_of_twelve EQ FALSE)
              {
                pcell->tim_valid = TV_APPROX_TIMING_INFO;
                nc_set_status (i, IDLE);
              }
              else
              {
                if (pcell->c_bcch EQ 0)
                  nc_set_status (i, READ_SB_BCCH);
              }
              break;
            case READ_BCCH:
              if (pcell->one_of_six EQ FALSE)
              {
                if (pcell->one_of_twelve EQ FALSE)
                {
                  nc_set_status (i, IDLE_SYNC);
                }
                else
                {
                  nc_set_status (i, FB_SB_SYNC);
                }
              }
              break;
            case READ_BCCH_RR_NOT_INFORMED:
              if (pcell->one_of_six EQ FALSE)
              {
                if (pcell->one_of_twelve EQ FALSE)
                {
                  nc_set_status (i, IDLE_SYNC);
                }
                else
                {
                  nc_set_status (i, FB_SB_SYNC);
                }
              }
              break;

            case READ_SB_BCCH:
              if (pcell->one_of_six EQ FALSE)
              {
                if (pcell->one_of_twelve)
                  nc_set_status(i, READ_SB);
                else
                {
                  pcell->tim_valid = TV_APPROX_TIMING_INFO;
                  nc_set_status (i, IDLE);
                }
              }
              break;

            default:
              break;
          }
          break;
#ifdef GPRS
        case NC_PIM_PBCCH:
        case NC_PTM_PBCCH: /*XXX*/
        case NC_CON_EST: /*in case of an updated list in CON_EST */
          switch (pcell->status)
          {
            case IDLE:
              if(pcell->one_of_twelve)
                nc_set_status (i, READ_FB_SB);
              if (start_list EQ CHECK_FOR_CONFIRM)
              {
                if (pcell->tim_valid EQ TV_VALID_TIMING_INFO)
                  pcell->tim_valid = TV_APPROX_TIMING_INFO;
                else if (pcell->tim_valid EQ TV_APPROX_TIMING_INFO)
                  pcell->tim_valid = TV_INVALID_TIMING_INFO;
              }
              break;
            case FB_SB_SYNC:
              if(pcell->one_of_twelve EQ FALSE)
                nc_set_status (i, IDLE_SYNC);
              else
              {
                if (start_list)
                  nc_set_status (i, READ_SB);
              }
              break;
            case FB_SB_FAILED:
              if(pcell->one_of_twelve EQ FALSE)
                remove_ncell_and_inform_grr(i);
              else
                if(pcell->c_sync EQ 0)
                  nc_set_status (i, READ_FB_SB);
              break;
            case READ_FB_SB:
              break;
            case READ_SB:
              if (pcell->one_of_twelve EQ FALSE)
              {
                pcell->tim_valid = TV_APPROX_TIMING_INFO;
                nc_set_status (i, IDLE);
              }
              break;
            case IDLE_SYNC:
              if(pcell->c_sync EQ 0)
                remove_ncell_and_inform_grr(i);
              else
              {
                if(pcell->one_of_twelve)
                  nc_set_status (i, FB_SB_SYNC);
              }
              break;
            case READ_BCCH_RR_NOT_INFORMED:
            case READ_BCCH:
            case FB_SB_SYNC_RR_NOT_INFORMED:
              if (pcell->one_of_twelve EQ FALSE)
              {
                nc_set_status (i, IDLE_SYNC);
              }
              break;
            default:
              TRACE_EVENT_P4("wrong state in check_st: i%d a%d 1o6%d st%d", i,
                               pcell->ba_arfcn,
                               pcell->one_of_six,
                               pcell->status);
              break;
          }
          break;
#endif
        default:
          break;
      }/*switch state*/
    } /*if*/
  } /*for*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_build_rr_report_dedi    |
+--------------------------------------------------------------------+

  PURPOSE : Fills a measurement report to RR with neighbour cell
            information in dedicated mode.
            In dedicated mode the serving cell may be included.

*/
LOCAL void nc_build_rr_report_dedi (T_MPH_MEASUREMENT_IND *rr_report)
{
  GET_INSTANCE_DATA;
  UBYTE c_report = 0;
  UBYTE c_ba_list;
  UBYTE found = TRUE;
  UBYTE index;
  UBYTE i;
  UBYTE in_report[34];
  T_NC * pcell1;

  memset (in_report, 0, 34);

#if defined (REL99) && defined (TI_PS_FF_EMR)
  if (alr_data->nc_data.emr_data.rep_type EQ REP_TYPE_ENH )
  {    
    nc_fill_ncell_enh_meas_results(rr_report);
    nc_fill_serv_cell_enh_meas_results(rr_report);
    return;
  }
#endif

  for ( i = 0; i < 6 AND found EQ TRUE; i++)
  {
    found = FALSE;
    index = NOT_PRESENT_8BIT;
    pcell1 = alr_data->nc_data.cell;

    for (c_ba_list = 0; c_ba_list < alr_data->nc_data.c_ba_arfcn; c_ba_list++, pcell1++)
    {
      if (in_report[c_ba_list] EQ FALSE AND pcell1->ba_status EQ IN_BA)
      {
        switch (pcell1->status)
        {
          case FB_SB_SYNC:
          case READ_SB:
          case READ_SB_PENDING:
            if (nc_ncell_in_plmn_permitted(pcell1->bsic))
            {
              if ((pcell1->ba_arfcn NEQ alr_data->serving_cell AND
                   pcell1->one_of_six) OR
                  (pcell1->ba_arfcn EQ alr_data->serving_cell AND
                   alr_data->nc_data.sc_included))
              {
                if (index EQ NOT_PRESENT_8BIT)
                  index = c_ba_list;
                else
                {
                 if (pcell1->rxlev_average >
                     alr_data->nc_data.cell[index].rxlev_average)
                   index = c_ba_list;
                }
              }
            }
          break;
        }
      }
    }

    if (index NEQ NOT_PRESENT_8BIT)
    {
      T_NC* pcell = &alr_data->nc_data.cell[index];
      found = TRUE;
      in_report[index] = TRUE;

      rr_report->ncells.arfcn[c_report] = pcell->ba_arfcn;

      if ((UBYTE)pcell->rxlev_average > 63)
        rr_report->ncells.rx_lev[c_report] = 63;
      else
        rr_report->ncells.rx_lev[c_report] = (UBYTE)pcell->rxlev_average;

      rr_report->ncells.bsic[c_report] = pcell->bsic;
      rr_report->ncells.time_alignmt[c_report] = pcell->time_align;
      rr_report->ncells.frame_offset[c_report++] = pcell->frame_offset;
    }

  } /*for (up to 6 ncells)*/
  /*
   * insert serving cell values
   */
  nc_fill_report_sc_dedi (rr_report, c_report);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_rank_ncells             |
+--------------------------------------------------------------------+

  PURPOSE : Searches for the six and additionally the twelve strongest neighbourcells.

*/
LOCAL void nc_rank_ncells (void)
{
  GET_INSTANCE_DATA;
  USHORT  c_found, i;
  USHORT  last = (sizeof alr_data->nc_data.cell /
                  sizeof alr_data->nc_data.cell[0]);
  UBYTE   old_1of6[ sizeof alr_data->nc_data.cell /
                    sizeof alr_data->nc_data.cell[0] ];
  UBYTE num_1of6, hyst_rxlev, nc_conf_active = FALSE;

  num_1of6   = 0;
  hyst_rxlev = 63;
  alr_data->nc_data.new_strong_cell_detect = FALSE;

  for ( i = 0; i < last; i++ )
  {
    old_1of6[i] = alr_data->nc_data.cell[i].one_of_six;
    alr_data->nc_data.cell[i].new_strong_cell = FALSE;

    if(old_1of6[i])
    {
      num_1of6++;

      if(alr_data->nc_data.cell[i].rxlev_average < hyst_rxlev)
        hyst_rxlev = alr_data->nc_data.cell[i].rxlev_average;
    }

    alr_data->nc_data.cell[i].one_of_six = FALSE;
  }

  if(num_1of6 < MAX_RR_NCELL_CNT)
     hyst_rxlev = 0;

  if((GET_STATE(STATE_NC_PROC) EQ NC_CONFIRM) AND
     (GET_STATE(STATE_NC) EQ NC_DEDICATED) AND 
     (!alr_data->nc_data.eotd_avail))
    nc_conf_active = TRUE;

  c_found = 0;

  if ((std EQ STD_DUAL) OR (std EQ STD_DUAL_EGSM) OR (std EQ STD_DUAL_US)
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
    OR (std EQ STD_850_1800) OR (std EQ STD_900_1900) OR (std EQ STD_850_900_1800) OR (std EQ STD_850_900_1900)
#endif
    )
  {
    switch (alr_data->nc_data.multiband)
    {
      case MULTI_BAND_0:
        /* Find 6 cells from any band */
        nc_find_cells (&c_found, MAX_RR_NCELL_CNT, NO_BAND_LIMITATION, ALR_RXLEV_AVERAGE_MIN);
        break;
      case MULTI_BAND_1:
        /* Find 1 cell from bands other than Serving cell band */
        nc_find_cells (&c_found, 1, EXCLUDE_SC_BAND, ALR_RXLEV_AVERAGE_LWR_THR);

        /* Find 5 cells from the Serving cell band */
        nc_find_cells (&c_found, MAX_RR_NCELL_CNT-1, ONLY_SC_BAND, ALR_RXLEV_AVERAGE_LWR_THR);

        /* If number of found cells are less than six, find the remaining from any band.
         * Also relax the Rxlev criteria
         */
        nc_find_cells (&c_found, (USHORT)(MAX_RR_NCELL_CNT - c_found), NO_BAND_LIMITATION,
          ALR_RXLEV_AVERAGE_MIN);
        break;
      case MULTI_BAND_2:
        /* Find 2 cell from bands other than Serving cell band */
        nc_find_cells (&c_found, 2, EXCLUDE_SC_BAND, ALR_RXLEV_AVERAGE_LWR_THR);

        /* Find 4 cells from the Serving cell band */
        nc_find_cells (&c_found, MAX_RR_NCELL_CNT-2, ONLY_SC_BAND, ALR_RXLEV_AVERAGE_LWR_THR);

        /* If number of found cells are less than six, find the remaining from any band.
         * Also relax the Rxlev criteria
         */
        nc_find_cells (&c_found, (USHORT)(MAX_RR_NCELL_CNT - c_found), NO_BAND_LIMITATION,
          ALR_RXLEV_AVERAGE_MIN);
        break;
      case MULTI_BAND_3:
        /* Find 3 cell from bands other than Serving cell band */
        nc_find_cells (&c_found, 3, EXCLUDE_SC_BAND, ALR_RXLEV_AVERAGE_LWR_THR);

        /* Find 3 cells from the Serving cell band */
        nc_find_cells (&c_found, MAX_RR_NCELL_CNT-3, ONLY_SC_BAND, ALR_RXLEV_AVERAGE_LWR_THR);

        /* If number of found cells are less than six, find the remaining from any band.
         * Also relax the Rxlev criteria
         */
        nc_find_cells (&c_found, (USHORT)(MAX_RR_NCELL_CNT - c_found), NO_BAND_LIMITATION,
          ALR_RXLEV_AVERAGE_MIN);
        break;
    }
  }
  else
    nc_find_cells (&c_found, MAX_RR_NCELL_CNT, NO_BAND_LIMITATION, ALR_RXLEV_AVERAGE_MIN);

  for ( i = 0; i < last; i++ )
  {
    if (alr_data->nc_data.cell[i].one_of_six AND !old_1of6[i] AND
        GET_STATE(STATE_NC) NEQ NC_DEDICATED)
    { /* the cell becomes one of six */
      switch (alr_data->nc_data.cell[i].status)
      {
        case FB_SB_SYNC:
          /* the cell is treaded as synchronized, perform BCCH reading ASAP */
          nc_set_status(i, READ_BCCH_RR_NOT_INFORMED);
          break;
        case READ_SB:
        case IDLE_SYNC:
        case FB_SB_FAILED:
          /* the cell shall be synchronized ASAP, then BCCH info is to send to RR */
          nc_set_status(i, READ_FB_SB);
          break;
        case READ_SB_PENDING:
          /* after receipt of sync ind the BCCH is to read and to send to RR */
          nc_set_status(i, READ_FB_SB_PENDING);
          break;
        default:
          break;
      }
    }
    if (old_1of6[i] AND
        !alr_data->nc_data.cell[i].one_of_six AND
        alr_data->nc_data.cell[i].status EQ FB_SB_SYNC)
      alr_data->nc_data.cell[i].c_attempt = 0;

    /* Check the new strong cell criteria when confirmation is active */
    if(nc_conf_active)
    {
      nc_check_new_strong_cell(i, old_1of6[i], hyst_rxlev);

    }
  }

  nc_find_list_cells();

  TRACE_EVENT_P1("New strong cell detect:%d", 
                  alr_data->nc_data.new_strong_cell_detect);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_find_cells              |
+--------------------------------------------------------------------+

  PURPOSE : Searches for the six strongest neighbourcells,
            using multiband parameter if needed.
            For Quad Band MS called only in case of the value of 'std' is set to
            STD_DUAL, STD_DUAL_EGSM, STD_DUAL_US, STD_850_1800, STD_900_1900,
            STD_850_900_1800 or STD_850_900_1900. 
            For Dual Band MS called only in case of the value of 'std' is set to
            STD_DUAL, STD_DUAL_EGSM or STD_DUAL_US.

*/
LOCAL void nc_find_cells (USHORT *c_found, USHORT max, UBYTE limitation,
                          UBYTE min_rxlev)
{
  GET_INSTANCE_DATA;
  int     i, j;
  BOOL    limit;
  int     index;

  UBYTE   rank = 0;

  USHORT  c_ba_arfcn = alr_data->nc_data.c_ba_arfcn;
  T_NC*   pcell;
  UBYTE   band_nc;
  UBYTE   band_sc = get_band (alr_data->serving_cell);

  if (*c_found >= MAX_RR_NCELL_CNT)
    return;

  if (band_sc EQ BAND_E_GSM)
    band_sc = BAND_GSM_900; /* equal treatment of E-GSM and GSM900 */

  for (j = 0, index = 0; (j < max) AND (index NEQ NOT_PRESENT_16BIT); j++)
  {
    index = NOT_PRESENT_16BIT;

    for (i = 0; i < c_ba_arfcn; i++)
    {
      pcell = &alr_data->nc_data.cell[i];
      /*
       * The six strongest neighbour cells do not include cells
       * currently not belonging to BA list
       */
      if (pcell->ba_status NEQ IN_BA)
        continue;

      switch (pcell->status)
      {
        case INACTIVE:
        case EXCLUDED:
          /*
           * The six strongest neighbour cells do not include failed ncells.
           */
          break;
        default:
          if (pcell->ba_arfcn NEQ alr_data->serving_cell AND
              pcell->one_of_six EQ FALSE AND
              pcell->rxlev_average > min_rxlev)  /* > -106 dBm */
          {
            band_nc = get_band (pcell->ba_arfcn);
            if (band_nc EQ BAND_E_GSM)
              band_nc = BAND_GSM_900; /* equal treatment of E-GSM and GSM900 */

            switch (limitation)
            {
              default:
              case NO_BAND_LIMITATION:
                limit = FALSE;
                break;
              case EXCLUDE_SC_BAND:
                limit = (band_sc EQ band_nc);
                break;
              case ONLY_SC_BAND:
                limit = (band_sc NEQ band_nc);
                break;
            }

            if (!limit)
            {
              if (index EQ NOT_PRESENT_16BIT)
              {
                index = i;
              }
              else
              {
                if (pcell->rxlev_average >
                    alr_data->nc_data.cell[index].rxlev_average)
                {
                  index = i;
                }
              }
            }
          }
          break;
      } /*ncell status*/
    }

    if (index NEQ NOT_PRESENT_16BIT)
    {
      alr_data->nc_data.cell[index].one_of_six = TRUE;
      alr_data->nc_data.cell[index].rank_of_the_six = rank;
      rank++;
      (*c_found)++;
    }
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_find_list_cells         |
+--------------------------------------------------------------------+

  PURPOSE : Searches for the 6 or 12 strongest neighbourcells

*/
LOCAL void nc_find_list_cells (void)
{
  GET_INSTANCE_DATA;
  USHORT  i, j;
  UBYTE   found, c_found = 0;
  USHORT  index;
  USHORT  c_ba_arfcn = alr_data->nc_data.c_ba_arfcn;
  T_NC*   pcell;

  for (i = 0, pcell = &alr_data->nc_data.cell[0]; i < c_ba_arfcn; i++, pcell++ )
  {
    if ((pcell->one_of_twelve = pcell->one_of_six) EQ TRUE)
      c_found++;
  }
  /*
   * The flag one_of_six controls the property to include a neighbour cell
   * into the measurement report,
   * the flag one_of_twelve controls whether the ncell is included into the
   * synchronisation confirmation process. In case of EOTD up to 12 cell are 
   * included, in case of non-EOTD 6 cells are sufficient.
   */
#if defined (REL99) && defined (TI_PS_FF_EMR)
  if ( (!alr_data->nc_data.eotd_avail) AND
       (alr_data->nc_data.emr_data.rep_type NEQ REP_TYPE_ENH) )
#else
  if (!alr_data->nc_data.eotd_avail)
#endif
    return;

  for (j = c_found, found = TRUE; (j < MAX_L1_SYNC_CNT) AND (found EQ TRUE); j++)
  {
    index = NOT_PRESENT_16BIT;
    found = FALSE;

    for (i = 0; i < c_ba_arfcn; i++)
    {
      pcell = &alr_data->nc_data.cell[i];
      /*
       * The 12 strongest neighbour cells do not include ncells currently not belonging to BA list.
       */
      if (pcell->ba_status NEQ IN_BA)
        continue;

      switch (pcell->status)
      {
        case INACTIVE:
        case EXCLUDED:
          /*
           * The 12 strongest neighbour cells do not include failed ncells.
           */
          break;
        default:
          if (pcell->ba_arfcn NEQ alr_data->serving_cell AND
              pcell->one_of_twelve EQ FALSE AND
              pcell->rxlev_average > ALR_RXLEV_AVERAGE_MIN)  /* > -110 dBm */
          {
            if (index EQ NOT_PRESENT_16BIT)
            {
              index = i;
              found = TRUE;
            }
            else
            {
              if (pcell->rxlev_average >
                  alr_data->nc_data.cell[index].rxlev_average)
              {
                index = i;
                found = TRUE;
              }
            }
          }
          break;
      } /*ncell status*/
    }

    if (found AND (index NEQ NOT_PRESENT_16BIT))
      alr_data->nc_data.cell[index].one_of_twelve = TRUE;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_store_rxlev             |
+--------------------------------------------------------------------+

  PURPOSE : Stores the fieldstrength values of the neighbour cells.
            Calculates the average of the fieldstrength after five
            samples.

*/


LOCAL void nc_store_rxlev (T_MPHC_RXLEV_PERIODIC_IND *report)
{
  GET_INSTANCE_DATA;
  USHORT  i;
  USHORT  index;
  UBYTE   diff;
  int     max_attempt;
  T_NC*   pcell;

  ALR_TRACE_NC("store rxlev");
  /*
   * Patch for 20.5
   * The fieldstrength average for a new channel
   * increases too slow if more then eight channels
   * are in the neighbourcell list, because layer 1
   * sends not raw data every paging period.
   */
  if (test_house AND
      alr_data->nc_data.c_ba_arfcn > 8)
    max_attempt = 3;
  else
    max_attempt = 5;

  if (alr_data->nc_data.tim_state NEQ NC_TIM_STOPPED)
  {
    /* Decrement the 10sec timer counter variable by BS_PA_MFRMS */
    alr_data->nc_data.c_ncsync_tim = (UBYTE)(alr_data->nc_data.c_ncsync_tim - alr_data->bs_pa_mfrms - 2);
  
    if ((signed char)(alr_data->nc_data.c_ncsync_tim) < 0)
      alr_data->nc_data.c_ncsync_tim = 0;

    if( alr_data->nc_data.c_ncsync_tim EQ 0 )
      nc_ncsync_tim_expiry();         /* 10 sec have elapsed. Perform all requisite tasks */
  }

  /* serving cell rxlev storage */
  index = nc_get_index (alr_data->serving_cell);
  pcell = &alr_data->nc_data.cell[index];

  if ( (signed char)report->s_rxlev < 0)
    report->s_rxlev = 0;

  if (pcell->c_rxlev EQ NOT_PRESENT_8BIT)
  {
    pcell->c_rxlev = 0;
    memset (pcell->rxlev, report->s_rxlev, 5);
  }
  else
  {
    pcell->rxlev[pcell->c_rxlev++] = report->s_rxlev;
  }
  pcell->c_rxlev %= 5;

  /*
   * store the results seperately for averaging when NC=1 or NC=2
   */
#ifdef GPRS
  if(alr_data->nwctrl_meas_active)
  {
    pcell->nc_rxlev += report->s_rxlev;
    pcell->c_nc_rxlev++;
  }
#endif

  /*
   * ncell rxlev storage
   */
  for (i = 0; i < report->nbr_of_carriers; i++)
  {
    /*
     * The RX level from TI is signed in the SAP
     * we define it as unsigned
     */
    if ((signed char) (report->result[i].rxlev) < 0)
    {
      report->result[i].rxlev = 0;
    }

    index = nc_get_index (ARFCN_TO_G23(report->result[i].radio_freq));
    if (index NEQ NOT_PRESENT_16BIT AND
        index NEQ LAST_BSIC_REQ AND
        ARFCN_TO_G23(report->result[i].radio_freq) NEQ alr_data->serving_cell)
    {
      int     temp;
      UBYTE   rxlev = report->result[i].rxlev;
      pcell = &alr_data->nc_data.cell[index];

      if (pcell->c_rxlev EQ NOT_PRESENT_8BIT)
      {
        /*
         * if it is a new cell, build an average from the first value
         * to speed up fb sb read
         */
        pcell->rxlev[0] = rxlev;
        pcell->rxlev[1] = rxlev;
        pcell->rxlev[2] = rxlev;
        pcell->rxlev[3] = rxlev;
        pcell->rxlev[4] = rxlev;
        pcell->c_rxlev = 0;
      }
      else
      {
        pcell->rxlev[pcell->c_rxlev++] = rxlev;
        if (pcell->c_rxlev >= (UBYTE)max_attempt)
          pcell->c_rxlev = 0;
      }
      temp =  pcell->rxlev[0] +
              pcell->rxlev[1] +
              pcell->rxlev[2];

      if (max_attempt EQ 5)
      {
        temp += pcell->rxlev[3] +
                pcell->rxlev[4];
      }

      pcell->rxlev_average = (UBYTE)(temp / max_attempt);
      /*
      * store the results seperately for averaging when NC=1 or NC=2
      */
#ifdef GPRS
      if(alr_data->nwctrl_meas_active)
      {
        pcell->nc_rxlev += rxlev;
        pcell->c_nc_rxlev++;
      }
#endif

#if defined(TRACING)
      TRACE_EVENT_P4("NC%u[%d] rx=%d av=%d",
        index,
        pcell->ba_arfcn EQ NOT_PRESENT_16BIT ? -1 : pcell->ba_arfcn&ARFCN_MASK,
        report->result[i].rxlev, pcell->rxlev_average);
#endif  /* TRACING */

      switch (pcell->status)
      {
        case INACTIVE:
          nc_set_status (index, IDLE);
          break;
        case EXCLUDED:
          diff = (UBYTE)(pcell->rxlev_average - pcell->last_rxlev);
          if (diff < 128 AND diff >= 6)
          {
            /*
             * result is positive and more than 6 dBm
             */
            nc_set_status (index, IDLE);
          }
          break;
        default:
          break;
      }
    }
  }
#ifdef FF_PS_RSSI
  alr_data->nc_data.rxqual_full =
    nc_convert_quality ((USHORT)report->qual_acc_idle,
                        (USHORT)report->qual_nbr_meas_idle);
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_find_serving_cell_entry |
+--------------------------------------------------------------------+

  PURPOSE : Finds an entry for serving cell
            in the BCCH allocation list.

*/
GLOBAL USHORT nc_find_serving_cell_entry (USHORT serving_cell)
{
  GET_INSTANCE_DATA;
  T_NC*   pcell;
  USHORT  i;
  UBYTE   last_alr = alr_data->nc_data.c_ba_arfcn;

  if (last_alr < LAST_BSIC_REQ)
  {
    pcell = &alr_data->nc_data.cell[last_alr];
#if defined(TRACING)
    TRACE_EVENT_P2 ("NC%u[%u] add", last_alr, serving_cell);
#endif  /* TRACING */
    alr_data->nc_data.c_ba_arfcn = last_alr+1;  /* store new last index */
    i = last_alr;
    ALR_TRACE_ALL_NC ();
  }
  else
  { /* ALR list is full, search for an entry not in state IN_BA */
    for (i = 0, pcell = alr_data->nc_data.cell; i < LAST_BSIC_REQ; i++, pcell++)
    {
      if (pcell->ba_status NEQ IN_BA)
        break;
    } /* for (i...) */
    if (i >= LAST_BSIC_REQ)
    { /*
       * no entry found
       * reuse the last entry to make sure there is a place for serving cell
       */
      i = LAST_BSIC_REQ-1;
      pcell = &alr_data->nc_data.cell[i];
    }

    nc_stop_if_active(i);
#if defined(TRACING)
    TRACE_EVENT_P2 ("NC%u[%u] replace", i, serving_cell);
#endif  /* TRACING */
  }
  nc_set_status (i, FB_SB_SYNC);
  pcell->ba_status = IN_BA;    /* even if not in the BA but other functions need it */
  pcell->ba_arfcn  = serving_cell;
  return i;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_add_serving_cell_ba_list|
+--------------------------------------------------------------------+

  PURPOSE : Adds serving cell to neighbourcell list.
            This function is called just before nc_start_reselect
            and not after the MPHC_NEW_SCELL_CNF.

*/
LOCAL void nc_add_serving_cell_ba_list (USHORT serving_cell)
{
  GET_INSTANCE_DATA;
  USHORT  index = nc_get_index (serving_cell);
  T_NC*   pcell;

  /*
   * Add serving cell if not included
   */
  switch (index)
  {
    case NOT_PRESENT_16BIT:
    case LAST_BSIC_REQ:
      /* serving cell isn´t inside the BA list */
      index = nc_find_serving_cell_entry(serving_cell);
      pcell = &alr_data->nc_data.cell[index];
      pcell->bsic               = alr_data->nc_data.cr_cell.bsic;
      pcell->frame_offset       = alr_data->nc_data.cr_cell.frame_offset;
      pcell->time_align         = alr_data->nc_data.cr_cell.time_align;
      break;

    default:
      if (index < ELEMENTS(alr_data->nc_data.cell))
      {
        /* serving cell is already included in ba list */
        /* For GPRS the scell may not be in FB_SB_SYNC so put it there */
        pcell = &alr_data->nc_data.cell[index];
  
        /* Check whether the cell has valid sync info */
        if (pcell->bsic NEQ NOT_PRESENT_8BIT)
        {
          nc_set_status(index, FB_SB_SYNC);
          alr_data->nc_data.cell[index].one_of_six = FALSE;
          alr_data->nc_data.cell[index].ba_status  = IN_BA; /* even if not in the BA but other functions need it */
        }
      }
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_update_ba_list          |
+--------------------------------------------------------------------+

  PURPOSE : Updates the BCCH allocation list.

*/

GLOBAL void nc_update_ba_list (USHORT serving_cell, T_MPH_NEIGHBOURCELL_REQ *ncell_list)
{
  GET_INSTANCE_DATA;
  USHORT  n_rr;     /* index of RR neighbor cell list */
  USHORT  n_alr;    /* index of ALR neighbor cell list */
  UBYTE   last_alr; /* last index of ALR neighbor cell list */
  USHORT  arfcn;
  T_NC*   pcell;

  alr_data->nc_data.multiband = ncell_list->multi_band;

  /*
   * remove or mark all channels which are not longer member of the
   * neighbour cell list.
   */
  last_alr = alr_data->nc_data.c_ba_arfcn;  /* current last index */
  for (n_alr = 0; n_alr < last_alr; n_alr++)
  {
    pcell = &alr_data->nc_data.cell[n_alr];
    arfcn = pcell->ba_arfcn;
    if (!nc_is_in_ncell_list (arfcn, ncell_list) AND
        (arfcn NEQ serving_cell))
    {
      if (GET_STATE(STATE_NC) EQ NC_IDLE)
      { /* the RR neighbor cell list is complete */
        last_alr = nc_remove_channel_from_ba_list (n_alr);
#if defined(TRACING)
        TRACE_EVENT_P2 ("NC%u[%u] remove", n_alr, arfcn);
#endif  /* TRACING */
        n_alr--;
        ALR_TRACE_ALL_NC ();
      }
      else
      { /* the RR neighbor cell list may be incomplete so mark only the entry */
        if (pcell->ba_status EQ IN_BA)
        {
          pcell->ba_status = NOT_IN_BA_SHORT;

#if defined(TRACING)
          TRACE_EVENT_P2 ("NC%u[%u] -> NOT_IN_BA_SHORT", n_alr, arfcn);
#endif  /* TRACING */
        }
      }
    }
  }

  /*
   * add all new channels.
   */
  for (n_rr = 0; n_rr < MAX_NEIGHBOURCELLS; n_rr++)
  {
    arfcn = ncell_list->arfcn[n_rr];
    if (arfcn EQ NOT_PRESENT_16BIT)
      break;  /* no more entries in the RR ncell_list */

    n_alr = nc_get_index (arfcn);

    if (arfcn NEQ serving_cell)
    {
      if (n_alr EQ NOT_PRESENT_16BIT OR n_alr EQ LAST_BSIC_REQ)
      {
        if (last_alr < BA_LIST_SIZE)
        {
          pcell = &alr_data->nc_data.cell[last_alr];

#if defined(TRACING)
          TRACE_EVENT_P2 ("NC%u[%u] add", last_alr, arfcn);
#endif  /* TRACING */
          pcell->ba_arfcn = arfcn;
          nc_set_status (last_alr, INACTIVE);
          pcell->ba_status = IN_BA;
          last_alr++; /* increment last index */
        }
        else
        { /* ALR list is full, search for an entry not in state IN_BA */
          USHORT i;
          for (i = 0, pcell = alr_data->nc_data.cell; i < BA_LIST_SIZE; i++, pcell++)
          {
            if (pcell->ba_status NEQ IN_BA AND pcell->ba_arfcn NEQ serving_cell)
            {
              nc_stop_if_active(i);
#if defined(TRACING)
              TRACE_EVENT_P2 ("NC%u[%u] replace", i, arfcn);
#endif  /* TRACING */
              pcell->ba_arfcn = arfcn;
              nc_set_status (i, INACTIVE);
              pcell->ba_status = IN_BA;
              break;
            }
          } /* for (i...) */
        }
      }
      else
      {
#if defined(TRACING)
        if (alr_data->nc_data.cell[n_alr].ba_status NEQ IN_BA)
          TRACE_EVENT_P2 ("NC%u[%u] -> IN_BA", n_alr, arfcn);
#endif
        alr_data->nc_data.cell[n_alr].ba_status = IN_BA;
      }
    }  /* if (arfcn NEQ serving_cell) */
  }

  /*
   * store whether SC was included into the BA list by the net
   */
  if (nc_is_in_ncell_list (serving_cell, ncell_list))
  {
    alr_data->nc_data.sc_included = TRUE;
  }
  else
  {
    alr_data->nc_data.sc_included = FALSE;
  }
  alr_data->nc_data.c_ba_arfcn = last_alr;  /* store new last index */

  /*
   * Add serving cell if not included
   */
  n_alr = nc_get_index (serving_cell);
  switch (n_alr)
  {
    case NOT_PRESENT_16BIT:
    case LAST_BSIC_REQ:
      n_alr = nc_find_serving_cell_entry(serving_cell);
      pcell = &alr_data->nc_data.cell[n_alr];

      pcell->bsic               = alr_data->bsic;
      pcell->frame_offset       = 0;
      pcell->time_align         = 0;
      break;

    default:
      /* serving cell is already included */
      pcell = &alr_data->nc_data.cell[n_alr];
      pcell->bsic               = alr_data->bsic;
      pcell->frame_offset       = 0;
      pcell->time_align         = 0;
      pcell->ba_status          = IN_BA;          /* even if not in the BA but other functions need it */
      nc_set_status (n_alr, FB_SB_SYNC);          /* make sure the SC is reported in measurement reports if included into BA list */
      break;
  }

  /*
   * reset status of neighbour cells with failed
   * synchronisation attempts.
   */
  for (n_alr = 0; n_alr < last_alr; n_alr++)
  {
    switch (alr_data->nc_data.cell[n_alr].status)
    {
      case FB_SB_FAILED:
      case EXCLUDED:
        alr_data->nc_data.cell[n_alr].tim_valid = TV_INVALID_TIMING_INFO;
        nc_set_status (n_alr, IDLE);
        break;
    }
  }

  /*
   * Reorder the entries. The goal is that the propably strongest neigbour cells
   * are measured in the first measurement period of a subsequent
   * MPHC_RXLEV_PERIODIC_REQ/IND. The appropriate arfcn's are the first ones in
   * ncell_list->arfcn
   */
  if (last_alr > 8)
  {
    T_NC temp, *prr, *palr;

    for (n_rr = 0; n_rr < 8 && ncell_list->arfcn[n_rr] NEQ NOT_PRESENT_16BIT; n_rr++)
    {
      n_alr = nc_get_index (ncell_list->arfcn[n_rr]);
      if (n_rr NEQ n_alr)
      {
        palr = &(alr_data->nc_data.cell[n_alr]);
        prr  = &(alr_data->nc_data.cell[n_rr]);
        memcpy(&temp, palr,  sizeof(T_NC));
        memcpy(palr,  prr,   sizeof(T_NC));
        memcpy(prr,   &temp, sizeof(T_NC));
      /*TRACE_EVENT_P4("reordered NC%u[%u] and NC%u[%u]",
                        n_rr,
                        ncell_list->arfcn[n_rr],
                        n_alr, ncell_list->arfcn[n_alr]);*/
      }
    }
  }

  ALR_TRACE_ALL_NC ();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_new_serving_cell        |
+--------------------------------------------------------------------+

  PURPOSE : Store old, set new serving cell, and add to neighbourcell list.

*/
GLOBAL void nc_new_serving_cell (USHORT serving_cell)
{
  GET_INSTANCE_DATA;
  alr_data->old_serving_cell = alr_data->serving_cell;
  alr_data->serving_cell = serving_cell;
  alr_data->sc_band = get_band (alr_data->serving_cell);
  nc_add_serving_cell_ba_list(serving_cell);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_is_in_ncell_list        |
+--------------------------------------------------------------------+

  PURPOSE : Checks whether a channel is in the ncell_list of RR.

*/
LOCAL UBYTE nc_is_in_ncell_list (USHORT channel, T_MPH_NEIGHBOURCELL_REQ *ncell_list)
{
  USHORT i;

  for (i = 0; i < MAX_NEIGHBOURCELLS AND ncell_list->arfcn[i] NEQ NOT_PRESENT_16BIT; i++)
  {
    if (channel EQ ncell_list->arfcn[i])
      return TRUE;
  }

  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)   MODULE  : ALR_NC                         |
| STATE   : code            ROUTINE : nc_remove_channel_from_ba_list |
+--------------------------------------------------------------------+

  PURPOSE : Removes one channel from the BCCH allocation.

*/

LOCAL UBYTE nc_remove_channel_from_ba_list (USHORT index)
{
  GET_INSTANCE_DATA;
  USHORT  i;
  UBYTE   c_ba_arfcn;

  switch(alr_data->nc_data.cell[index].status)
  {
    case READ_SB_PENDING:
    case READ_SB_BCCH_PENDING:
      if ( alr_data->nc_data.eotd_avail AND
           ( GET_STATE(STATE_NC_PROC) EQ NC_CONFIRM) )
      {
        /* we cannot stop this
         * because we are using a list_req
         */
        nc_set_status(index, INACTIVE);
        break;
      }
      /*lint -fallthrough*/
    case READ_FB_SB_PENDING:
      /*
       * sync request is ongoing in layer 1.
       */
      nc_stop_sync (index, INACTIVE);
      break;
    case READ_BCCH_PENDING:
    case READ_BCCH_PENDING_RR_NOT_INFORMED:
      /*
       * stop pending BCCH request
       */
      nc_stop_bcch (index, INACTIVE);
      break;
    default:
      nc_set_status (index, INACTIVE);
      break;
  }

  c_ba_arfcn = alr_data->nc_data.c_ba_arfcn;
  for (i = index + 1; i < c_ba_arfcn; i++)
  {
    memcpy (&alr_data->nc_data.cell[i - 1],&alr_data->nc_data.cell[i],sizeof(T_NC));  /*shift NC*/
  }
  memset (&alr_data->nc_data.cell[i - 1],0,sizeof (T_NC));/*delete old content of obsolete ba_list entry*/

  alr_data->nc_data.c_ba_arfcn--;
  return (alr_data->nc_data.c_ba_arfcn);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_get_index               |
+--------------------------------------------------------------------+

  PURPOSE : Calculates the index in the BCCH allocation.

*/

GLOBAL USHORT nc_get_index (USHORT arfcn)
{
  GET_INSTANCE_DATA;
  USHORT  i;
  T_NC    *pcell; /* use pointer to increase processing speed */
  USHORT  last_alr; /* last index of ALR neighbor cell list */

  last_alr = alr_data->nc_data.c_ba_arfcn;
  pcell = &alr_data->nc_data.cell[0]; /* pointer to first cell */
  for (i = 0; i < last_alr; i++, pcell++)
  {
    if (arfcn EQ pcell->ba_arfcn)
      return i;
  }

  if (arfcn EQ alr_data->nc_data.cell[LAST_BSIC_REQ].ba_arfcn)
    return LAST_BSIC_REQ;

  return NOT_PRESENT_16BIT;
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_build_sync_req          |
+--------------------------------------------------------------------+

  PURPOSE : Builds an MPHC_NCELL_SYNC_REQ and sends it to L1.

*/
LOCAL void nc_build_sync_req (USHORT index)
{
  GET_INSTANCE_DATA;

#if defined (TI_PS_FF_RTD) AND defined (REL99)
  UBYTE  i,j;
  T_enh_para_struct *enh_para = &alr_data->nc_data.emr_data.enh_para;
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */

  T_NC * p_ncell = &alr_data->nc_data.cell[index];

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  USHORT l1_arfcn;
#endif

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  if (alr_data->plmn_search_running AND (index EQ LAST_BSIC_REQ) AND (alr_multiband_std NEQ NOT_PRESENT_8BIT))
  {
    l1_arfcn = ARFCN_STD_TO_L1 (p_ncell->ba_arfcn, alr_multiband_std);
    TRACE_EVENT_P2 ("Converted ARFCN %d for std %d", l1_arfcn, alr_multiband_std);
  }
  else
    l1_arfcn = ARFCN_TO_L1 (p_ncell->ba_arfcn);
#endif

#if defined (TI_PS_FF_RTD) AND defined (REL99)
  if(enh_para->enh_cell_list[index].v_rtd EQ 1)
    p_ncell->tim_valid=TV_RTD_TIMING_INFO ;
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */


#if defined (TI_PS_FF_RTD) AND defined (REL99)
  if(p_ncell->tim_valid EQ TV_RTD_TIMING_INFO)
  {
    for(j=0;j<MAX_NEIGHBOURCELLS;j++)
    { 
      if(p_ncell->ba_arfcn EQ enh_para->enh_cell_list[j].arfcn)
        break;
    } /*for*/
   if(j < MAX_NEIGHBOURCELLS)
    {	
    for(i=0;i<enh_para->enh_cell_list[j].c_rtd;i++)
    { 

      PALLOC (sync_req, MPHC_NCELL_SYNC_REQ);
      sync_req->radio_freq = ARFCN_TO_L1(p_ncell->ba_arfcn);
      sync_req->fn_offset = nc_sync_fn_offset_calculation(enh_para->enh_cell_list[index].rtd[i]);
      sync_req->time_alignment = nc_sync_time_alignment_calculation(enh_para->enh_cell_list[index].rtd[i]);
      sync_req->timing_validity =p_ncell->tim_valid ;
      alr_data->nc_data.cell[index].c_sync_req++;
      TRACE_EVENT_P4("nc_build_sync_req[%d] timing_validity=%d fn_offset=%ld, time_alignment=%ld",
                      p_ncell->ba_arfcn,
                      sync_req->timing_validity,
                      sync_req->fn_offset,
                      sync_req->time_alignment);

      ma_nc_sync_req(sync_req);
      alr_data->nc_data.c_sync_req++;

    } /*for*/
  }
   }
  else
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */
  {
    PALLOC (sync_req, MPHC_NCELL_SYNC_REQ);
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
    sync_req->radio_freq = l1_arfcn;
#else
    sync_req->radio_freq = ARFCN_TO_L1(p_ncell->ba_arfcn);
#endif

    if (p_ncell->tim_valid EQ TV_INVALID_TIMING_INFO) 
    {
      sync_req->fn_offset      = 0;
      sync_req->time_alignment = 0;
    }
    else
    {
      sync_req->fn_offset      = p_ncell->frame_offset;
      sync_req->time_alignment = p_ncell->time_align;
    }

    sync_req->timing_validity = p_ncell->tim_valid;
    TRACE_EVENT_P4("nc_build_sync_req[%d] timing_validity=%d fn_offset=%ld, time_alignment=%ld",
                  p_ncell->ba_arfcn,
                  sync_req->timing_validity,
                  sync_req->fn_offset,
                  sync_req->time_alignment);
    ma_nc_sync_req (sync_req);
    alr_data->nc_data.c_sync_req++;
  }

  ALR_EM_READ_NEIGHBOURCELL_SB;

  nc_set_status (index, READ_FB_SB_PENDING);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_process_status          |
+--------------------------------------------------------------------+

  PURPOSE : Starts a specific procedure for neighbour cells like
            synchronisation to frequency correction burst and
            synchron burst, confirmation of synchron burst or
            reading of BCCH. With the new ALR interface it's possible
            to send up to 12 SYNC_REQ's and BCCH_REQ's.

*/

LOCAL void nc_process_status (void)
{
  GET_INSTANCE_DATA;
  UBYTE st = GET_STATE(STATE_NC);

  /* Sync requests for New cells are allowed only in dedicated state
   * when eotd is not enabled. Fix for 
   */
  if(((alr_data->nc_data.eotd_avail) OR ( st NEQ NC_DEDICATED)) AND 
     (GET_STATE(STATE_NC_PROC) EQ NC_CONFIRM))
    return;

#ifdef GPRS
  if((st EQ NC_IDLE AND !ma_is_ptm()) OR
      st EQ NC_PIM_PBCCH )
    nc_process_status_last_bsic();
#else
  nc_process_status_last_bsic();
#endif


  if(alr_data->plmn_search_running)
    return;

#ifdef GPRS
  if(st EQ NC_PIM_PBCCH OR
     st EQ NC_PTM_PBCCH   )
     nc_process_status_sync_gprs();
  else
#endif
    nc_process_status_sync();

  if(st EQ NC_IDLE)
    nc_process_status_bcch();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_process_status_last_bsic|
+--------------------------------------------------------------------+

  PURPOSE : When doing idle mode cell selection we use
      LAST_BSIC_REQ for storing the information of the cell.
            This has priority over normal neighbour cell monitoring.

*/


LOCAL void nc_process_status_last_bsic(void)
{
  GET_INSTANCE_DATA;
  T_NC* pbsic = &alr_data->nc_data.cell[LAST_BSIC_REQ];

  switch (pbsic->status)
  {
    case READ_FB_SB:
      if (alr_data->nc_data.c_sync_req < MAX_L1_SYNC_CNT)
      {
        pbsic->tim_valid = TV_INVALID_TIMING_INFO;
        nc_build_sync_req (LAST_BSIC_REQ);
        ALR_TRACE_NC ("ncell_sync_req HPLMN");

   /* If the Mobile is in page mode REORG the NC search cannot be
    * performed because L1 is too busy listening to the PCH. Therefore
    * the page mode is changed from REORG to REORG_NC_SYNC (NORMAL).
    */
    if(alr_data->pch_data.pl_idle.page_mode EQ PGM_REORG OR
       alr_data->pch_data.pl_idle.page_mode EQ PGM_REORG_CS)
    {
      page_mode_before_hplmn_search = alr_data->pch_data.pl_idle.page_mode;
      alr_data->pch_data.pl_idle.page_mode = PGM_REORG_NC_SYNC;
      pch_start_ccch_req ();
      ALR_TRACE_NC ("HPLMN search cannot be performed with REORG - possible loss of pagings");
    }

      }
      return ;

    case READ_BCCH:
      if (alr_data->nc_data.c_bcch_req < MAX_L1_BCCH_CNT)
      {
        PALLOC (ncell_bcch, MPHC_NCELL_BCCH_REQ);

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
        if (alr_data->plmn_search_running AND (alr_multiband_std NEQ NOT_PRESENT_8BIT))
        {
          ncell_bcch->radio_freq = ARFCN_STD_TO_L1(pbsic->ba_arfcn, alr_multiband_std);
          alr_multiband_std = NOT_PRESENT_8BIT;
        }
        else
#endif
          ncell_bcch->radio_freq = ARFCN_TO_L1(pbsic->ba_arfcn);
        ncell_bcch->fn_offset = pbsic->frame_offset;
        ncell_bcch->time_alignment = pbsic->time_align;
        /*
         * The training sequence code on broadcast and common control channels
         * has to be equal to the base station color code see GSM 5.02
         */
        ncell_bcch->tsc = (UBYTE)(pbsic->bsic & ONLY_BCC);
        /*
         * Read SI 3/4 to get the MNC/MCC
         */
        pbsic->blocks_required =
          ncell_bcch->bcch_blocks_required = NCELL_BCCH_SI_3_4;
#ifdef GPRS
        if(alr_data->pch_data.pl_idle.page_mode EQ PGM_REORG OR
           alr_data->pch_data.pl_idle.page_mode EQ PGM_REORG_CS OR
           alr_data->pch_data.reorg_bcch_reading EQ TRUE OR
           pbsic->new_strong_cell OR
           !pbsic->c_attempt   // EQ 0
)
        {
          ncell_bcch->gprs_prio            = GPRS_PRIO_TOP;
        }
        else
#endif
        ncell_bcch->gprs_prio = GPRS_PRIO_NORM;

        ma_nc_bcch_req (ncell_bcch);
        alr_data->nc_data.c_bcch_req++;
        nc_set_status (LAST_BSIC_REQ, READ_BCCH_PENDING);
      }
      return;
    case READ_FB_SB_PENDING:
    case READ_BCCH_PENDING:
      return;
    default:
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_process_status_sync     |
+--------------------------------------------------------------------+

  PURPOSE : Processes cell synchronisation.

*/

LOCAL void nc_process_status_sync(void)
{
  GET_INSTANCE_DATA;
  /* To avoid warnings */
  USHORT index;

#if defined (TI_PS_FF_RTD) AND defined (REL99)
  USHORT i;
  T_enh_para_struct *enh_para = &alr_data->nc_data.emr_data.enh_para;
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */

  /* Sync requests for New cells are allowed only in dedicated state
   * when eotd is not enabled. Patch for 
   */
  if((!alr_data->nc_data.eotd_avail) AND 
     ( GET_STATE(STATE_NC) EQ NC_DEDICATED) AND 
     (GET_STATE(STATE_NC_PROC) EQ NC_CONFIRM))
  {
    if(alr_data->nc_data.new_strong_cell_detect)
      nc_handle_new_strong_cells();
   
    alr_data->nc_data.new_strong_cell_detect = FALSE;    
    return;
  }

#if defined (REL99) && defined (TI_PS_FF_EMR)
  if ( alr_data->nc_data.emr_data.rep_type EQ REP_TYPE_ENH )
  {
    nc_process_status_sync_emr();
    return;
  }
#endif

  while (alr_data->nc_data.c_sync_req < MAX_L1_SYNC_CNT)
  {
    /*
     * It is possible to send more sync requests to layer 1
     */

    index = NOT_PRESENT_16BIT;
     nc_highst_field_strng(&index, PRO_STA_SYNC);

    if (index NEQ NOT_PRESENT_16BIT)
    {
#if defined (TI_PS_FF_RTD) AND defined (REL99)
      if ((alr_data->nc_data.emr_data.rep_type NEQ REP_TYPE_ENH) AND (!enh_para->enh_cell_list[index].v_rtd ))
      {
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */
          /*
           * Request synchronisation for this cell
           */
          nc_build_sync_req(index);
          ALR_TRACE_NC ("ncell_sync_req");
#if defined (TI_PS_FF_RTD) AND defined (REL99)
      }
      else
      {

      for(i=0;i<MAX_NEIGHBOURCELLS;i++)
      {
        if(alr_data->nc_data.cell[index].ba_arfcn EQ enh_para->enh_cell_list[i].arfcn)
          break;
      } /*for*/ 
      if((i < MAX_NEIGHBOURCELLS) AND (enh_para->enh_cell_list[i].c_rtd < MAX_L1_SYNC_CNT - alr_data->nc_data.c_sync_req))
      { 
        /*
           * Request synchronisation for this cell
           */
          nc_build_sync_req(index);
          ALR_TRACE_NC ("ncell_sync_req_for_rtd");
          alr_data->nc_data.nxt_ind++;  
      }
      else
        break; /*while loop */
      	}
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */

    }
    else
      break; /*while loop */
  } /* while( c_sync_req < MAX_L1_SYNC_CNT) */
#if defined (TI_PS_FF_RTD) AND defined (REL99)
  /* reset the sync flag if we have sent sync req for all neighbourcells*/
  if(alr_data->nc_data.nxt_ind EQ alr_data->nc_data.c_ba_arfcn - 1) 
    alr_data->nc_data.nxt_ind = 0; 
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */

}

#if defined (REL99) && defined (TI_PS_FF_EMR)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE    : code                ROUTINE : nc_process_status_sync_emr |
+--------------------------------------------------------------------+

  PURPOSE : Processes cell synchronisation status for EMR. In EMR
  we may have to request for synchronization for more than 12 cells.
  Since L1 has limitation of handling more than 12, we have to consider
  how many are being handled by L1 already and then form the request 
  accordingly.
*/
LOCAL void nc_process_status_sync_emr(void)
{
  GET_INSTANCE_DATA;
  UBYTE             i = 0;
  T_NC*             pcell = &alr_data->nc_data.cell[0];
  UBYTE             j;
  T_ncell_list      *sync_req;
  UBYTE             k;  
  ULONG             rank_cell_bmp=alr_data->nc_data.rank_cell_bmp;
  UBYTE             num_scb = 0;
  UBYTE             num_mb = 0;
  UBYTE             num_rest = 0;
  UBYTE             total_to_req = 0;
  UBYTE             scb[4];
  UBYTE             mb[4];
  UBYTE             rest[32];
  UBYTE             max=0;
  T_enh_para_struct *p_enh = &alr_data->nc_data.emr_data.enh_para;  
  UBYTE             nc_index;
  
  memset (scb, 0, 4 * sizeof (UBYTE));
  memset (mb, 0, 4 * sizeof (UBYTE));
  memset (rest, 0, 32 * sizeof (UBYTE));
    /*First rank the cells as per EMR related criteria*/
  while ( i <alr_data->nc_data.c_ba_arfcn  )
  {
    max = 0;  
    k = i;
    for (j= 0; j < alr_data->nc_data.c_ba_arfcn; j++)
    {
    /*Do not include ranked cells in subsequent search*/
      if (( ((rank_cell_bmp >>j) &(0x00000001)) EQ FALSE ) AND
        (pcell[j].rxlev_average > max ) AND 
        ( pcell[j].status EQ READ_FB_SB))
      {
        max = pcell[j].rxlev_average;
        k = j;
      }          
    }
    /*For the highest rxlev with status READ_FB_SB :
    find into which category this cell falls into*/
    
    if ( k NEQ i )          
    {
      if ( get_band(pcell[k].ba_arfcn) EQ get_band(alr_data->serving_cell) )
      {
        /*serving band: we require only first SERVINGBAND_REPORTING number of 
       cells which will have higher priority */
        if (num_scb < p_enh->servingband_rep)
          scb[num_scb++] = k;
        else
          rest[num_rest++] = k;
      }         
      else
      {
      /*It's a cell on non-serving band: On a priority basis, we need
        to synchronize atleast first MULTIBAND_REPORTING number of cells*/
        if( num_mb < p_enh->multiband_rep)
          mb[num_mb++] = k;
        else
          rest[num_rest++] = k;
      }    
      rank_cell_bmp |= 1<<k; /*Indicate that this cell is ranked*/
    }
    i++;    
  }/*while 'i'*/
  
  alr_data->nc_data.rank_cell_bmp = rank_cell_bmp;
  total_to_req = num_scb + num_mb + num_rest;
  
  k=0;  
  if ( total_to_req > 0)
  {
    PALLOC (ncell_list_req, MPHC_NCELL_LIST_SYNC_REQ);    
       
    i = ncell_list_req->list_size = 0; 
    /*If previously some sync requests are pending, we consider this number while
    forming the current sync request*/  
    while ( (i < (MAX_L1_SYNC_CNT - alr_data->nc_data.c_sync_req)) AND
            (i < total_to_req)  )
    {
      /*Include the cells in the order - cells in serving band, cells in non-serving band,
      and the other cells with low RXLEV*/
      nc_index = NOT_PRESENT_8BIT;
      if (num_scb > 0)
      {
        nc_index = scb[k++];
        num_scb--;
        if (num_scb EQ 0)
          k = 0;
      }
      else if (num_mb > 0)
      {
        nc_index = mb[k++];
        num_mb--;
        if (num_mb EQ 0)
          k = 0;
      }
      else if (num_rest > 0)
      {
        nc_index = rest[k++];
        num_rest--;
      }        
      if (nc_index NEQ NOT_PRESENT_8BIT )
      {
        sync_req = &ncell_list_req->ncell_list[i++];
        sync_req->radio_freq  = ARFCN_TO_L1(pcell[nc_index].ba_arfcn);
        sync_req->timing_validity = pcell[nc_index].tim_valid;
        
        if (pcell[nc_index].tim_valid EQ TV_INVALID_TIMING_INFO)
        {
          sync_req->fn_offset = 0;
          sync_req->time_alignment = 0;
        }
        else
        {
          sync_req->fn_offset = pcell[nc_index].frame_offset;
          sync_req->time_alignment = pcell[nc_index].time_align;
        }      
        nc_set_status(nc_index,READ_FB_SB_PENDING);          
      }
      
    }
    ncell_list_req->list_size = i;
    alr_data->nc_data.c_sync_req += i;
    ncell_list_req->eotd = FALSE;
    ma_nc_list_sync_req(ncell_list_req);
  }  
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_process_status_sync_gprs|
+--------------------------------------------------------------------+

  PURPOSE : Processes GPRS cell synchronisation.

*/


#ifdef GPRS /*XXX*/
LOCAL void nc_process_status_sync_gprs(void)
{
  GET_INSTANCE_DATA;
  USHORT i;
#ifdef FF_RTD
  USHORT j;
  T_enh_para_struct *enh_para = &alr_data->nc_data.emr_data.enh_para;
#endif /* #ifdef FF_RTD */

  T_NC* pbsic = &alr_data->nc_data.cell[LAST_BSIC_REQ];
  USHORT  c_ba_arfcn = alr_data->nc_data.c_ba_arfcn;


  for (i = 0; i < c_ba_arfcn AND alr_data->nc_data.c_sync_req < MAX_L1_SYNC_CNT; i++)
  {
    /*
     * search for the candidate with the highest fieldstrength
     */
    T_NC* pcell = &alr_data->nc_data.cell[i];

    if ((pcell->ba_arfcn NEQ pbsic->ba_arfcn) AND
        (pcell->ba_arfcn NEQ alr_data->serving_cell) AND
        (pcell->status EQ READ_FB_SB))
    {
#ifdef FF_RTD 
      for(j=0;j<MAX_NEIGHBOURCELLS;j++)
      { 
        if(pcell->ba_arfcn EQ enh_para->enh_cell_list[j].arfcn)
        break;
      } /*for*/ 
      if(enh_para->enh_cell_list[j].c_rtd < MAX_L1_SYNC_CNT - alr_data->nc_data.c_sync_req)
      {
#endif /* #ifdef FF_RTD */

      /*
       * Request synchronisation for this cell
       */
      nc_build_sync_req(i);
      ALR_TRACE_NC ("ncell_sync_req");
#ifdef FF_RTD 
        alr_data->nc_data.nxt_ind++;
      }
      else
        break;
#endif /* #ifdef FF_RTD */
    }
  }
#ifdef FF_RTD 
  if(alr_data->nc_data.nxt_ind EQ c_ba_arfcn-1)
    alr_data->nc_data.nxt_ind = 0;
#endif /* #ifdef FF_RTD */

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_sync_failed_gprs      |
+--------------------------------------------------------------------+

  PURPOSE : Handles synchronisation failed situation

*/


LOCAL void nc_sync_failed_gprs(USHORT index)
{
  GET_INSTANCE_DATA;
  alr_data->nc_data.cell[index].c_sync = 1; /*XXX*/
  nc_set_status (index, FB_SB_FAILED);
  nc_inform_grr_of_ncell(index, GRR_SB_NOT_FOUND);
}
#endif


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_process_status_bcch     |
+--------------------------------------------------------------------+

  PURPOSE : Processes CB status

*/


LOCAL void nc_process_status_bcch(void)
{
  GET_INSTANCE_DATA;

  /* To avoid warnings */
  USHORT index;

  /* To avoid warnings */
  while (alr_data->nc_data.c_bcch_req < MAX_L1_BCCH_CNT)
  {
    /*
     * It is possible to send more BCCH request to layer 1
     */

  /* To avoid warnings */
    index = NOT_PRESENT_16BIT;
     nc_highst_field_strng(&index, PRO_STA_BCCH);

    if (index NEQ NOT_PRESENT_16BIT)
    {
      /*
       * Request BCCH reading for this cell
       */
      T_NC* pcell = &alr_data->nc_data.cell[index];
      PALLOC (ncell_bcch, MPHC_NCELL_BCCH_REQ);

      ncell_bcch->radio_freq = ARFCN_TO_L1(pcell->ba_arfcn);
      ncell_bcch->fn_offset = pcell->frame_offset;
      ncell_bcch->time_alignment = pcell->time_align;
      /*
       * The training sequence code on broadcast and common control channels
       * has to be equal to the Base Station Colour Code (BCC),
       * see 3GPP TS 5.02, section 5.2.3 and
       * 3GPP TS 03.03, section 4.3.2 Base Station Identify Code (BSIC)
       */
      ncell_bcch->tsc = (UBYTE)(pcell->bsic & ONLY_BCC);
      pcell->blocks_required =
        ncell_bcch->bcch_blocks_required = NCELL_BCCH_SI_3_4;
#ifdef GPRS
      /*if the mobile is in PTM the GPRS_PRIORITY must be set to TOP*/
      if(alr_data->pch_data.pl_idle.page_mode EQ PGM_REORG OR
         alr_data->pch_data.pl_idle.page_mode EQ PGM_REORG_CS OR
         alr_data->pch_data.reorg_bcch_reading EQ TRUE OR
         pcell->new_strong_cell OR
         !pcell->c_attempt OR  // EQ 0
         ma_is_ptm())
      {
        ncell_bcch->gprs_prio = GPRS_PRIO_TOP;
      }
      else
#endif
        ncell_bcch->gprs_prio = GPRS_PRIO_NORM;


      ma_nc_bcch_req (ncell_bcch);

      ALR_TRACE_NC("ncell_bcch_read");

      alr_data->nc_data.c_bcch_req++;
      /*
       * read a maximum of four blocks
       *
       * sys info 3 normal BCCH TC 2 or 6
       * sys info 4 normal BCCH TC 3 or 7
       */
      if(pcell->status EQ READ_BCCH)
        nc_set_status (index, READ_BCCH_PENDING);
      else
        nc_set_status (index, READ_BCCH_PENDING_RR_NOT_INFORMED);
    }
    else
      break; /*while loop */
  } /* while(c_bcch_req < MAX_L1_BCCH_CNT)*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_ncell_in_plmn_permitted |
+--------------------------------------------------------------------+

  PURPOSE : checks whether the national colour code (ncc) is member
            of the ncc permitted field.

*/
static const UBYTE ncc_bit_mask[8] =
  {
    BIT_0, BIT_1, BIT_2, BIT_3,
    BIT_4, BIT_5, BIT_6, BIT_7
  };

LOCAL BOOL nc_ncell_in_plmn_permitted (UBYTE bsic)
{
  GET_INSTANCE_DATA;
  return ((BOOL) ((UBYTE) (alr_data->ncc_permitted &
                           ncc_bit_mask[(bsic >> 3) & BIT_012])) NEQ 0);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_store_dedicated         |
+--------------------------------------------------------------------+

  PURPOSE : Stores the values of the neighbour cells
            and the serving cell.

*/
LOCAL void nc_store_dedicated (T_MPHC_MEAS_REPORT *report)
{
  GET_INSTANCE_DATA;
  USHORT i;
  USHORT index;

  if (alr_data->nc_data.tim_state NEQ NC_TIM_STOPPED)
  {
    /* Decrement the 10sec timer counter variable by 2 */
    alr_data->nc_data.c_ncsync_tim = alr_data->nc_data.c_ncsync_tim-2;
  
    if ((signed char)( alr_data->nc_data.c_ncsync_tim) < 0)
      alr_data->nc_data.c_ncsync_tim = 0;

    if( alr_data->nc_data.c_ncsync_tim EQ 0 )    /* 10 sec have elapsed. Perform all requisite tasks */
      nc_ncsync_tim_expiry();
  }
 
  for (i = 0; i < report->no_of_ncells_meas; i++)
  {
    T_NC* pcell;
    T_res_list* plist = &report->ncell_meas.res_list[i];

    index = nc_get_index (ARFCN_TO_G23(plist->bcch_freq));
    if ((index EQ NOT_PRESENT_16BIT) OR (index EQ LAST_BSIC_REQ))
      continue;
    pcell = &alr_data->nc_data.cell[index];

    if (plist->rxlev_nbr_meas EQ 0)
      pcell->rxlev_average = 0;
    else
    {
      if ((signed short)(plist->rxlev_acc) < 0)
        plist->rxlev_acc = 0;

      /* why this casting? - a "real" expression is of type int */
      pcell->rxlev_average = (UBYTE)(plist->rxlev_acc/plist->rxlev_nbr_meas);
    }
    if (pcell->status EQ INACTIVE)
      nc_set_status (index, IDLE);

    if (pcell->status EQ EXCLUDED)
    {
      int diff;

      diff = pcell->rxlev_average - pcell->last_rxlev;

      if (diff >= 6)
      {
        /*
         * result is positive and more than 6 dBm
         */
        nc_set_status (index, IDLE);
      }
    }
  }


  alr_data->nc_data.act_dtx = report->dtx_used;

  if (report->rxlev_full_nbr_meas EQ 0)
    alr_data->nc_data.rxlev_full = 0;
  else
  {
    if ((signed short)(report->rxlev_full_acc) < 0)
      report->rxlev_full_acc = 0;

    alr_data->nc_data.rxlev_full =
      (UBYTE)(report->rxlev_full_acc / report->rxlev_full_nbr_meas);
  }

  if (report->rxlev_sub_nbr_meas EQ 0)
    alr_data->nc_data.rxlev_sub = 0;
  else
  {
    if ((signed short)(report->rxlev_sub_acc) < 0)
      report->rxlev_sub_acc = 0;
    alr_data->nc_data.rxlev_sub = (UBYTE)(report->rxlev_sub_acc / report->rxlev_sub_nbr_meas);
  }

  alr_data->nc_data.rxqual_full =
    nc_convert_quality (report->rxqual_full_acc_errors,
                        report->rxqual_full_nbr_bits);

  alr_data->nc_data.rxqual_sub =
    nc_convert_quality (report->rxqual_sub_acc_errors,
                        report->rxqual_sub_nbr_bits);
#if defined (REL99) && defined (TI_PS_FF_EMR)
  /* Additions for EMR */
  if (alr_data->nc_data.emr_data.rep_type EQ REP_TYPE_ENH)
  {    
    /* store the serving cell measurements that have to be reported in
       enhanced measurement report*/
    if (report->rxlev_val_nbr_meas EQ 0)
      alr_data->nc_data.rxlev_val = 0;
    else
    {
      if ((signed short)(report->rxlev_val_acc) < 0)
      report->rxlev_val_acc = 0;
      alr_data->nc_data.rxlev_val = (UBYTE)(report->rxlev_val_acc / report->rxlev_val_nbr_meas);
    }      

    alr_data->nc_data.nbr_rcvd_blks = report->nbr_rcvd_blocks;
    alr_data->nc_data.m_bep = alr_data->nc_data.cv_bep = 0;
    if (report->mean_bep_block_num NEQ 0)
    {      
      alr_data->nc_data.m_bep = nc_map_mean_bep((ULONG) (report->mean_bep_block_acc/ report->mean_bep_block_num));      
    }
    if (report->cv_bep_block_num NEQ 0)
    {    
      alr_data->nc_data.cv_bep = nc_map_cv_bep ( (USHORT) (report->cv_bep_block_acc/ report->cv_bep_block_num));
    }    
  }
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_convert_quality         |
+--------------------------------------------------------------------+

  PURPOSE : Calculates the quality Value.

*/

LOCAL UBYTE nc_convert_quality (USHORT errors, USHORT total)
{
  USHORT quality;

  if (total EQ 0)
    quality = 0;
  else
    quality = (USHORT)((errors * 500) / total);

  if (quality EQ 0)
    return 0;          /* RX_QUAL_0 */

  if (quality EQ 1)
    return 1;          /* RX_QUAL_1 */

  if (quality < 4)
    return 2;          /* RX_QUAL_2 */

  if (quality < 8)
    return 3;          /* RX_QUAL_3 */

  if (quality < 16)
    return 4;          /* RX_QUAL_4 */

  if (quality < 32)
    return 5;          /* RX_QUAL_5 */

  if (quality < 64)
    return 6;          /* RX_QUAL_6 */

  return 7;            /* RX_QUAL_7 */

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_release_bcch            |
+--------------------------------------------------------------------+

PURPOSE : Check if we have read the BCCH of a ncell but RR has
          not yet received the data because this cell has not
          been reported to RR before.
*/

LOCAL void nc_release_bcch (void)
{
  GET_INSTANCE_DATA;
  USHORT i;
  USHORT c_ba_arfcn = alr_data->nc_data.c_ba_arfcn;
  /*
   * for every ncell
   */
  for (i = 0; i < c_ba_arfcn; i++)
  {
    T_NC* pcell = &alr_data->nc_data.cell[i];

    switch (pcell->status)
    {
      case FB_SB_SYNC_RR_NOT_INFORMED:
        /*
         * Reading of the BCCH of the neighbourcell is performed,
         * but RR has been just informed with a measurement report
         * about synchronisation.
         */
        if(pcell->mph_unitdata_ind NEQ NULL)
        {
          ALR_TRACE_NC ("send 34");

          /*
           * a system info type 3 or 4 message is stored.
           * Send the message to RR and clear the internal variable.
           */
          PSENDX(RR, pcell->mph_unitdata_ind);
          pcell->mph_unitdata_ind = NULL;
        }

        if(pcell->mph_unitdata_ind78 NEQ NULL)
        {
          ALR_TRACE_NC ("send 78");

          /*
           * a system info type 7 or 8 message is stored.
           * Send the message to RR and clear the internal variable.
           */
          PSENDX(RR, pcell->mph_unitdata_ind78);
          pcell->mph_unitdata_ind78 = NULL;
        }

        /*
         * set status to FB_SB_SYNC, that means RR is completely informed.
         */
        nc_set_fb_sb_sync_initial (i);
        break;

      default:
        break;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_store_bcch              |
+--------------------------------------------------------------------+

PURPOSE : Temporarily store BCCH data for ncell.

*/

LOCAL void nc_store_bcch (T_MPHC_DATA_IND* data_ind, USHORT index, UBYTE sys_info_78)
{
  GET_INSTANCE_DATA;
  #define SYS_INFO_LEN 24

  USHORT len_in_bits = SYS_INFO_LEN * BITS_PER_BYTE;
  T_NC* pcell = &alr_data->nc_data.cell[index];
  /*
   * Allocate a SDU for sending later to RR
   */
  PALLOC_SDU (data_out, MPH_UNITDATA_IND, len_in_bits);

  /*
   * set length and offset, but cut IE and Pseudo Length
   */
  data_out->sdu.l_buf = (SYS_INFO_LEN - 2) * BITS_PER_BYTE;
  data_out->sdu.o_buf = 1 * BITS_PER_BYTE;

  /*
   * copy content of the message
   */
  /*lint -e{419} (Warning -- Apparent data overrun)*/
  memcpy (data_out->sdu.buf, &data_ind->l2_frame, SYS_INFO_LEN - 1);

  /*
   * set BCCH frequency of the message
   */
  data_out->arfcn = ARFCN_TO_G23(data_ind->radio_freq)&ARFCN_MASK;
  data_out->fn    = data_ind->fn;

  if (sys_info_78)
  {
    ALR_TRACE_NC ("store 78");

    /*
     * if it is a system info 7 or 8 message, free the previous
     * stored message, if something is stored and store the new
     * one.
     */
    if (pcell->mph_unitdata_ind78 NEQ NULL)
    {
      ALR_TRACE_NC ("store,free old 78");

      PFREE(pcell->mph_unitdata_ind78);
    }

    pcell->mph_unitdata_ind78 = data_out;

  }
  else
  {
    ALR_TRACE_NC ("store 34");

    /*
     * if it is a system info 3 or 4 message, free the previous
     * stored message, if something is stored and store the new
     * one.
     */
    if (pcell->mph_unitdata_ind NEQ NULL)
    {
      ALR_TRACE_NC( "store,free old 34");

      PFREE(pcell->mph_unitdata_ind);
    }

    pcell->mph_unitdata_ind = data_out;

  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_sys_info_78_required    |
+--------------------------------------------------------------------+

PURPOSE : The function checks whether sys info 7 or 8 are expected
          and required.

*/

LOCAL UBYTE nc_sys_info_78_required (USHORT index)
{
  GET_INSTANCE_DATA;
  if (alr_data->nc_data.cell[index].mph_unitdata_ind NEQ 0)
  {
    T_sdu * sdu = &alr_data->nc_data.cell[index].mph_unitdata_ind->sdu;
    /*lint -e{415} -e{416} (Warning -- creation of out-of-bounds pointer/access of out-of-bounds pointer)*/
    if ((sdu->buf[SI_CONTENTS_CS2] & ONLY_ACS) NEQ 0)
      return TRUE;
  }
  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_stop_bcch               |
+--------------------------------------------------------------------+

  PURPOSE : Stop a pending BCCH.

*/

LOCAL void nc_stop_bcch (USHORT index, UBYTE new_status)
{
  GET_INSTANCE_DATA;
  /*
   * L3 may avoid sending a stop message to terminate a NCELL_BCCH process
   * if there no more pending request in L1
   */

  if (alr_data->nc_data.c_bcch_req > 0)
  {
    /*
     * stop BCCH request in layer 1.
     */
    ma_nc_stop_ncell_bcch_req(alr_data->nc_data.cell[index].ba_arfcn);

    /*
     * decrement counter of pending requests
     */
    alr_data->nc_data.c_bcch_req--;
  }

  /*
   * set new status
   */
  nc_set_status (index, new_status);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_stop_sync               |
+--------------------------------------------------------------------+

  PURPOSE : Stop a pending Sync request.

*/

LOCAL void nc_stop_sync (USHORT index, UBYTE new_status)
{
  GET_INSTANCE_DATA;
  /*
   * L3 may avoid sending a stop message to terminate a NCELL_SYNC process
   * if there no more pending request in L1
   */

  if (alr_data->nc_data.c_sync_req > 0)
  {
    /*
     * stop sync request in layer 1.
     */
    ma_nc_stop_ncell_sync_req (alr_data->nc_data.cell[index].ba_arfcn);

    /*
     * decrement counter of pending requests
     */
    alr_data->nc_data.c_sync_req--;
    /*
     * check whether the confirmation procedure is finished indirectly
     */
    if (alr_data->nc_data.c_sync_req EQ 0 AND
        alr_data->nc_data.eotd_avail EQ FALSE AND
        GET_STATE(STATE_NC_PROC) EQ NC_CONFIRM)
    {
      nc_enable_conf();
    }
  }

  /*
   * set new status
   */
  nc_set_status (index, new_status);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_clean_store_bcch        |
+--------------------------------------------------------------------+

  PURPOSE : Clean a stored BCCH.

*/

LOCAL void nc_clean_store_bcch (USHORT index)
{
  GET_INSTANCE_DATA;
  T_NC* pcell = &alr_data->nc_data.cell[index];
  if (pcell->mph_unitdata_ind NEQ NULL)
  {
    ALR_TRACE_NC ("free 34");

    /*
     * a system info type 3 or 4 message is stored
     * then free the message and clean the pointer.
     */

    PFREE(pcell->mph_unitdata_ind);
    pcell->mph_unitdata_ind = NULL;

  }

  if(pcell->mph_unitdata_ind78 NEQ NULL)
  {
    ALR_TRACE_NC ("free 78");

    /*
     * a system info type 7 or 8 message is stored
     * then free the message and clean the pointer.
     */

    PFREE(pcell->mph_unitdata_ind78);
    pcell->mph_unitdata_ind78 = NULL;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_check_bsic              |
+--------------------------------------------------------------------+

  PURPOSE : Check a base station identification code of a neighbourcell
            The following return values are possible:

            NC_CHECK_OK           : BSIC has not changed and is permitted.
            NC_CHECK_NCC_FAILED   : NCC permitted check has failed
            NC_CHECK_BSIC_CHANGED : a BSIC change has occured


*/

LOCAL UBYTE nc_check_bsic (USHORT index, UBYTE bsic)
{
  GET_INSTANCE_DATA;
  /*
   * check only the lowest 6 bits (= NCC + BCC)
   */
  bsic = (UBYTE)(bsic & ONLY_BSIC);

  /*
   * first check whether ncc is member of the ncc permitted field
   */
  if (!nc_ncell_in_plmn_permitted (bsic))
    return NC_CHECK_NCC_FAILED;

  /*
   * no bsic stored until now.
   */
  if (alr_data->nc_data.cell[index].bsic EQ NOT_PRESENT_8BIT)
    return NC_CHECK_OK;

  /*
   * check against BSIC changes
   */
  if (alr_data->nc_data.cell[index].bsic NEQ bsic)
    return NC_CHECK_BSIC_CHANGED;

  /*
   * all checks passed
   */
  return NC_CHECK_OK;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_check_new_ncc_permitted |
+--------------------------------------------------------------------+

PURPOSE : Checks whether status transitions are needed after reception
          of a changed NCC permitted field.

*/

GLOBAL void nc_check_new_ncc_permitted (UBYTE new_ncc_permitted)
{
  GET_INSTANCE_DATA;
  USHORT i;
  USHORT c_ba_arfcn = alr_data->nc_data.c_ba_arfcn;
  if (new_ncc_permitted NEQ alr_data->ncc_permitted)
  {
    /*
     * a change has occured, so store new value and check all cells
     */
    alr_data->ncc_permitted = new_ncc_permitted;

    for (i = 0; i < c_ba_arfcn; i++)
    {
      switch (alr_data->nc_data.cell[i].status)
      {
        case FB_SB_SYNC:
        case FB_SB_SYNC_RR_NOT_INFORMED:
        case READ_SB:
        case READ_BCCH:
        case IDLE_SYNC:
        case READ_SB_BCCH:
        case READ_BCCH_RR_NOT_INFORMED:
          if (nc_ncell_in_plmn_permitted(alr_data->nc_data.cell[i].bsic) EQ FALSE)
            nc_set_status (i, EXCLUDED);
          break;

        case READ_BCCH_PENDING:
        case READ_BCCH_PENDING_RR_NOT_INFORMED:
          if (nc_ncell_in_plmn_permitted(alr_data->nc_data.cell[i].bsic) EQ FALSE)
            nc_set_status (i, EXCLUDED);
          break;

        case EXCLUDED:
          /*
           * give channel a new chance
           */
          nc_set_status (i, IDLE);
          break;
      }
    }
  }
}

#if !defined(DEFINE_OLD_NC_STATUS)
LOCAL UBYTE nc_get_nc_status(UBYTE status)
{
  UBYTE st;

  switch (status)
  {
  case  INACTIVE:st=0;break;
  case  IDLE:st=1;break;
  case  READ_BCCH:st=2;break;
  case  FB_SB_SYNC:st=3;break;
  case  FB_SB_FAILED:st=4;break;
  case  READ_FB_SB:st=5;break;
  case  READ_SB:st=6;break;
  case  IDLE_SYNC:st=7;break;
  case  EXCLUDED:st=8;break;
  case  FB_SB_SYNC_RR_NOT_INFORMED:st=9;break;
  case  READ_SB_BCCH:st=10;break;
  case  READ_BCCH_PENDING:st=11;break;
  case  READ_FB_SB_PENDING:st=12;break;
  case  READ_SB_PENDING:st=13;break;
  case  READ_SB_BCCH_PENDING:st=14;break;
  case  READ_BCCH_RR_NOT_INFORMED:st=15;break;
  case  READ_BCCH_PENDING_RR_NOT_INFORMED:st=16;break;
  default:st=17;break;
  }

  return st;
}
#else  /* DEFINE_OLD_NC_STATUS */
#define nc_get_nc_status(st) (((int)(st) >= 18) ? 18 : (st))
#endif  /* DEFINE_OLD_NC_STATUS */

#if !defined(NTRACE) && defined(TRACING)
LOCAL const char * nc_get_nc_state_str(UBYTE status)
{
  return alr_nc_state_trc[nc_get_nc_status(status)];
}
#endif /* !NTRACE && TRACING */
#if 0 /* not needed currently */
#if defined(TRACING)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_get_status              |
+--------------------------------------------------------------------+

PURPOSE : The function returns the status for a channel.

*/

LOCAL UBYTE nc_get_status (USHORT index)
{
  /* use pointer to save time and ROM */
  T_NC* pcell = &alr_data->nc_data.cell[index];

  TRACE_USER_CLASS_P3 (TC_USER1, "NC%u[%d] %s",
    index,
    pcell->ba_arfcn EQ NOT_PRESENT_16BIT ? -1 : pcell->ba_arfcn&ARFCN_MASK,
    nc_get_nc_state_str(pcell->status));

  return pcell->status;
}
#endif  /* TRACING */
#endif /* 0|1 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_set_status              |
+--------------------------------------------------------------------+

PURPOSE : The function sets a new status for a channel. Depending on
          the new status several variables which are not valid in
          the new status are initialised to their default values.


*/

GLOBAL void nc_set_status (USHORT index, UBYTE new_status)
{
  GET_INSTANCE_DATA;
  /* use pointer to save time and ROM */
  T_NC* pcell = &alr_data->nc_data.cell[index];

#if defined(TRACING)
  TRACE_EVENT_P4 ("NC%u[%d] %s -> %s",
    index,
    pcell->ba_arfcn EQ NOT_PRESENT_16BIT ? -1 : pcell->ba_arfcn&ARFCN_MASK,
    nc_get_nc_state_str(pcell->status),
    nc_get_nc_state_str(new_status));
#endif  /* TRACING */

  switch (new_status)
  {
    case INACTIVE:
      pcell->bsic            = NOT_PRESENT_8BIT;
      pcell->frame_offset    = NOT_PRESENT_32BIT;
      pcell->time_align      = NOT_PRESENT_32BIT;
      pcell->last_rxlev      = 0;
      pcell->c_rxlev         = NOT_PRESENT_8BIT;
      pcell->rxlev_average   = 0;
      memset (pcell->rxlev, 0, 5);
      pcell->one_of_six = FALSE;
      pcell->one_of_twelve   = FALSE;
      pcell->c_attempt       = 0;
      pcell->c_bcch          = C_INVALID_BCCH;
      pcell->c_sync          = C_INVALID_SYNC;
      pcell->tim_valid       = TV_INVALID_TIMING_INFO;
      pcell->blocks_required = 0;
      pcell->c_error         = 0;
      pcell->syn_failure_times= 0;
      nc_clean_store_bcch (index);
      break;

    case IDLE:
      pcell->bsic            = NOT_PRESENT_8BIT;
#if 0
      pcell->frame_offset    = 0;
      pcell->time_align      = 0;
      pcell->last_rxlev      = 0;
#endif
      pcell->one_of_six      = FALSE;
      pcell->one_of_twelve   = FALSE;
      /*pcell->c_attempt       = 0;*/
      pcell->c_bcch          = C_INVALID_BCCH;
      pcell->c_sync          = C_INVALID_SYNC;
      pcell->blocks_required = 0;
      pcell->c_error         = 0;
      nc_clean_store_bcch (index);
      break;

    case READ_FB_SB:
    case READ_FB_SB_PENDING:
      pcell->bsic            = NOT_PRESENT_8BIT;
#if 0
      pcell->frame_offset    = 0;
      pcell->time_align      = 0;
#endif
      pcell->last_rxlev      = 0;
      pcell->c_bcch          = C_INVALID_BCCH;
      pcell->c_sync          = C_INVALID_SYNC;
      pcell->blocks_required = 0;
      pcell->c_error         = 0;
      nc_clean_store_bcch (index);
      break;

    case READ_BCCH:
      pcell->last_rxlev      = 0;
      /*pcell->c_attempt       = 0;*/
      pcell->c_bcch          = C_INVALID_BCCH;
      pcell->c_sync          = C_INVALID_SYNC;
      pcell->c_error         = 0;
      pcell->blocks_required = 0;
      nc_clean_store_bcch (index);
      break;
    case READ_BCCH_RR_NOT_INFORMED:
      pcell->last_rxlev      = 0;
      pcell->c_bcch          = C_INVALID_BCCH;
      pcell->c_sync          = C_INVALID_SYNC;
      pcell->c_error         = 0;
      pcell->blocks_required = 0;
      nc_clean_store_bcch (index);
      break;

    case FB_SB_SYNC_RR_NOT_INFORMED:
      pcell->last_rxlev      = 0;
      pcell->c_bcch          = C_INVALID_BCCH;
      pcell->c_sync          = C_INVALID_SYNC;
      pcell->blocks_required = 0;
      pcell->c_attempt       = 0;
      pcell->c_error         = 0;
      break;

    case FB_SB_SYNC:
      if((pcell->status == READ_FB_SB) || (pcell->status == READ_FB_SB_PENDING))
      {
	if (pcell->syn_failure_times > 0)
	  pcell->syn_failure_times --;
      }
      pcell->last_rxlev      = 0;
      pcell->blocks_required = 0;
      if (!pcell->one_of_six)
        pcell->c_attempt       = 0;
      pcell->c_error         = 0;
      pcell->c_sync          = C_INVALID_SYNC;
      pcell->tim_valid       = TV_VALID_TIMING_INFO;
      nc_clean_store_bcch (index);
      break;

    case READ_SB:
    case READ_SB_PENDING:
      pcell->last_rxlev      = 0;
      pcell->c_sync          = C_INVALID_SYNC;
      pcell->blocks_required = 0;
      /*pcell->c_attempt       = 0;*/
      pcell->c_error         = 0;
      nc_clean_store_bcch (index);
      break;

    case READ_SB_BCCH:
    case READ_SB_BCCH_PENDING:
      pcell->last_rxlev      = 0;
      pcell->c_bcch          = C_INVALID_BCCH;
      pcell->c_sync          = C_INVALID_SYNC;
      pcell->blocks_required = 0;
      pcell->c_error         = 0;
      /*pcell->c_attempt       = 0;*/
      nc_clean_store_bcch (index);
      break;


    case IDLE_SYNC:
      pcell->last_rxlev      = 0;
      pcell->c_bcch          = C_INVALID_BCCH;
      pcell->one_of_six      = FALSE;
      pcell->one_of_twelve   = FALSE;
      pcell->c_attempt       = 0;
      pcell->blocks_required = 0;
      pcell->c_error         = 0;
      pcell->c_sync          = alr_data->nc_data.c_nc_timer; /*TODO maybe move this to a function*/
      nc_clean_store_bcch (index);
      break;

    case FB_SB_FAILED:
      if((pcell->status == READ_FB_SB) || (pcell->status == READ_FB_SB_PENDING))
	pcell->syn_failure_times ++;
      pcell->bsic            = NOT_PRESENT_8BIT;
      if (pcell->status EQ READ_FB_SB_PENDING)
      {
        pcell->frame_offset  = NOT_PRESENT_32BIT;
        pcell->time_align    = NOT_PRESENT_32BIT;
        pcell->tim_valid     = TV_INVALID_TIMING_INFO;
      }
      else
        pcell->tim_valid     = TV_APPROX_TIMING_INFO;
      pcell->last_rxlev      = 0;
      pcell->c_bcch          = C_INVALID_BCCH;
      pcell->blocks_required = 0;
      pcell->c_error         = 0;
      nc_clean_store_bcch (index);
      break;

    case EXCLUDED:
      pcell->bsic            = NOT_PRESENT_8BIT;
      pcell->frame_offset    = NOT_PRESENT_32BIT;
      pcell->time_align      = NOT_PRESENT_32BIT;
      pcell->one_of_six      = FALSE;
      pcell->one_of_twelve   = FALSE;
      pcell->last_rxlev      = pcell->rxlev_average; /* TODO maybe move this to a function */
      pcell->c_attempt       = 0;
      pcell->c_bcch          = C_INVALID_BCCH;
      pcell->c_sync          = C_INVALID_SYNC;
      pcell->tim_valid       = TV_INVALID_TIMING_INFO;
      pcell->blocks_required = 0;
      pcell->c_error         = 0;
      pcell->syn_failure_times= 0;
      nc_clean_store_bcch (index);
      break;

    case READ_BCCH_PENDING:
    case READ_BCCH_PENDING_RR_NOT_INFORMED:
      pcell->last_rxlev      = 0;
      /*pcell->c_attempt       = 0;*/
      pcell->c_bcch          = C_INVALID_BCCH;
      pcell->c_sync          = C_INVALID_SYNC;
      nc_clean_store_bcch (index);
      break;
  }

  /*
   * set new status
   */
  pcell->status           = new_status;
}

GLOBAL void nc_check_activity (void)
{
  GET_INSTANCE_DATA;
  int i;
  switch(GET_STATE(STATE_NC))
  {
    case NC_IDLE:
      /* count number of reports */
      alr_data->nc_data.c_reports++;

      nc_rank_ncells   ();
      nc_check_status  (CHECK_FOR_ACQUIRE_AND_BCCH_AND_FAIL);
      nc_process_status();

      if (first_period AND !first_l1_meas)
      {
        if (alr_data->nc_data.c_reports EQ alr_data->nc_data.max_reports OR
            alr_data->nc_data.c_reports EQ alr_data->nc_data.max_reports - 1)
        {
          UBYTE all_sync = TRUE;
          /*
           * for every ncell
           */
          for (i = 0; i < alr_data->nc_data.c_ba_arfcn; i++)
          {
            if (alr_data->nc_data.cell[i].status EQ READ_BCCH_PENDING OR
                alr_data->nc_data.cell[i].status EQ READ_BCCH_PENDING_RR_NOT_INFORMED
#if (CHIPSET EQ 15) 
/* For Locosto: Give an additional period if all ncells are not yet synchronized. Gives FTA 20.7 a better chance to pass */
                OR alr_data->nc_data.cell[i].status EQ READ_FB_SB 
                OR alr_data->nc_data.cell[i].status EQ READ_FB_SB_PENDING
#endif                
                )
            {
              /* not all needed SI are read yet */
              all_sync = FALSE;
              break;
            }
          }
          if (!all_sync AND
              alr_data->nc_data.c_reports EQ alr_data->nc_data.max_reports)
          {
            /* spend an additional reporting period to give FTA 20.7 a better chance to pass */
            alr_data->nc_data.c_reports--;
            first_period = FALSE;
            ALR_TRACE_NC ("one additional period");
          }
          else if (all_sync AND
                   alr_data->nc_data.c_reports EQ alr_data->nc_data.max_reports - 1)
          {
            /* skip an additional reporting period to give FTA 20.19 a better chance to pass */
            alr_data->nc_data.c_reports++;
            first_period = FALSE;
            ALR_TRACE_NC ("one period skipped");
          }
        }
      }
      first_l1_meas = FALSE;

#if defined(_SIMULATION_)
      TRACE_EVENT_P2 ("c_reports=%u/%u",
        alr_data->nc_data.c_reports, alr_data->nc_data.max_reports);
#endif  /* WIN32 */

      /* check if 5sec have passed, we then have to inform RR */
      if (alr_data->nc_data.c_reports EQ alr_data->nc_data.max_reports)
      {
        USHORT index, average;
        T_NC* pcell;
        PALLOC (report, MPH_MEASUREMENT_IND);
        memset (report, 0, sizeof (T_MPH_MEASUREMENT_IND));

        first_period = FALSE;
#ifdef GPRS
        report->gprs_sync = NORMAL_MEAS_REP;
#endif

        report->arfcn = alr_data->serving_cell;

        average = 0;
        index = nc_get_index(alr_data->serving_cell);
        if ( index NEQ NOT_PRESENT_16BIT )
        {
          pcell = &alr_data->nc_data.cell[index];
          pcell->rxlev_average = 0;  /* Is this really needed? */
          for (i = 0; i < 5; i++)
            average += pcell->rxlev[i];
        }

        report->rx_lev_full = (UBYTE) (average / 5);
        report->fn_offset = alr_data->nc_data.fn_offset;

        alr_data->nc_data.c_reports                 = 0;

        nc_build_rr_report (report);
        ma_nc_report_res   (report);
        nc_release_bcch    ();
#if defined (REL99) && defined (TI_PS_FF_EMR)
        alr_data->nc_data.si2q_per = (alr_data->nc_data.si2q_per +1)%6;
#ifdef GPRS
        if(gprs_check_read_si13_only())
#endif
        {
        /*Periodic acquisition of SI-2quater every 30 sec, when it is
          configured on EBCCH and when GPRS is not supported*/
          if (alr_data->nc_data.si2q_per	EQ 0)
          {
            if ( (alr_data->nc_data.si2quater_status NEQ SI2QUATER_ABSENT) AND
              (alr_data->nc_data.si2quater_pos EQ SI2QUATER_ON_EBCCH) )
            {
              ma_scell_mon_si2quater_ebcch();
              alr_data->nc_data.si2quater_status = SI2QUATER_ACQ_PENDING ;
            }
          }
        } 
#endif
      }
      break;
    case NC_DEDICATED:
      nc_rank_ncells   ();
      nc_check_status  (CHECK_FOR_ACQUIRE_AND_BCCH_AND_FAIL);
      nc_process_status();
      {
        PALLOC(mph_measurement_ind, MPH_MEASUREMENT_IND);
#ifdef GPRS
        mph_measurement_ind->gprs_sync = NORMAL_MEAS_REP;
#endif
        nc_build_rr_report_dedi (mph_measurement_ind);
        ma_nc_report_res        (mph_measurement_ind);
      }
      break;
    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_set_fb_sb_sync_initial  |
+--------------------------------------------------------------------+

PURPOSE : Inital setting of status FB_SB_SYNC. Counter for SB
          confirmation and BCCH re-reading are initialized.

*/

LOCAL void nc_set_fb_sb_sync_initial (USHORT index)
{
  GET_INSTANCE_DATA;
  alr_data->nc_data.cell[index].c_bcch   = FIVE_MINUTES;

  if(!alr_data->nc_data.eotd_avail)
  {
   /*round to next confirmation boundary */
   if(alr_data->nc_data.c_nc_timer NEQ THIRTY_SECONDS)
    alr_data->nc_data.cell[index].c_bcch += alr_data->nc_data.c_nc_timer;
  }

  nc_set_status (index, FB_SB_SYNC);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)              MODULE  : ALR_NC              |
| STATE   : code                       ROUTINE : nc_set_fb_sb_failed |
+--------------------------------------------------------------------+

PURPOSE : Set the status to FB_SB_FAILED, update the attempt counter
          and set time until next attempt is started.

*/

LOCAL void nc_set_fb_sb_failed (USHORT index,
                                UBYTE  c_sync)
{
  GET_INSTANCE_DATA;
  alr_data->nc_data.cell[index].c_attempt++;
  alr_data->nc_data.cell[index].c_sync = c_sync;
  nc_set_status (index, FB_SB_FAILED);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103) MODULE  : ALR_NC                           |
| STATE   : code          ROUTINE : nc_rxlev_sc_req                  |
+--------------------------------------------------------------------+

PURPOSE : A new RXLEV value of the serving cell has been measured.
          Inform GPL in case it is necessary.

*/
#ifdef GPRS

LOCAL void nc_rxlev_sc_req (UBYTE rxlev)
{
  GET_INSTANCE_DATA;
  if(GET_STATE (STATE_NC) EQ NC_IDLE)
  {
    PALLOC (rxlev_sc_req, TB_RXLEV_SC_REQ);

    rxlev_sc_req->sc_rxlev = rxlev;

    ma_nc_rxlev_sc_req (rxlev_sc_req);
  }
}

#endif /* #ifdef GPRS */

#if defined(_SIMULATION_)
LOCAL void trace_nc(void)
{
  GET_INSTANCE_DATA;
  char buf[80];
  int  i, o;
  T_NC* pcell;
  o = sprintf (buf, "NC: ");

  for (i=0,pcell = &alr_data->nc_data.cell[0]; i < alr_data->nc_data.c_ba_arfcn; i++,pcell++)
  {
    if(pcell->ba_arfcn EQ alr_data->serving_cell)
    {
      o += sprintf (buf+o, "%u[SC=%u]%u, ", i, pcell->ba_arfcn, pcell->ba_status);
    }
    else
    {
      o += sprintf (buf+o, "%u[%u]%u, ", i, pcell->ba_arfcn, pcell->ba_status);
    }
    if (o > 60)
    {
      TRACE_EVENT (buf);
      o = sprintf (buf, "NC: ");
    }
  }
  if (o>4)
  {
    buf[o-2]=0;
    TRACE_EVENT (buf);
  }
}
#endif  /* _SIMULATION_ */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_check_new_strong_cell   |
+--------------------------------------------------------------------+

  PURPOSE : Checks the New strong cell criteria

*/

LOCAL void nc_check_new_strong_cell(USHORT index, UBYTE o_1of6, 
                                    UBYTE rxlev)
{
  GET_INSTANCE_DATA;
  T_NC* pcell = &alr_data->nc_data.cell[index];

  if((pcell->one_of_six ) AND (!o_1of6))
  {
    /* NCELL confirmation active. New 1of6 cell found */
    if((pcell->rxlev_average > rxlev) AND
       ((pcell->rxlev_average - rxlev ) >= 5))
      pcell->new_strong_cell = TRUE;
  }
  else if ((pcell->one_of_six ) AND (o_1of6) AND
           (pcell->status EQ READ_FB_SB) AND
           (pcell->c_attempt EQ 0 ))
  {
    /* NCELL confirmation interrupted the sync of last 1of6 cell
     * This has to be synchronized again
     */
    pcell->new_strong_cell = TRUE;
  }

  if(pcell->new_strong_cell)
  {
    alr_data->nc_data.new_strong_cell_detect = TRUE;
    TRACE_EVENT_P1("[%d]New strong cell",pcell->ba_arfcn);
  }

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_handle_new_strong_cells |
+--------------------------------------------------------------------+

  PURPOSE : Processes cell synchronisation.

*/

LOCAL void nc_handle_new_strong_cells(void)
{
  GET_INSTANCE_DATA;
  USHORT i;
  USHORT index;
  UBYTE  first_sync_req = TRUE;
  T_NC* pbsic = &alr_data->nc_data.cell[LAST_BSIC_REQ];

  alr_data->nc_data.c_sync_intrupted = FALSE;

  while (alr_data->nc_data.c_sync_req < MAX_L1_SYNC_CNT)
  {
    /*
     * It is possible to send more sync requests to layer 1
     */
    USHORT  c_ba_arfcn = alr_data->nc_data.c_ba_arfcn;
    index = NOT_PRESENT_16BIT;

    for (i = 0; i < c_ba_arfcn; i++)
    {
      /*
       * search for "New strong cell" with the highest fieldstrength
       */
      T_NC* pcell = &alr_data->nc_data.cell[i];

      if ((pcell->ba_arfcn NEQ pbsic->ba_arfcn) AND
          (pcell->ba_arfcn NEQ alr_data->serving_cell) AND
          (pcell->new_strong_cell))
      {
        switch (pcell->status)
        {
          case READ_FB_SB:
            if (index EQ NOT_PRESENT_16BIT)
              index = i;
            else
            {
              if (pcell->rxlev_average >
                  alr_data->nc_data.cell[index].rxlev_average)
                index = i;
            }
            break;
          default:
            break;
        }
      }
    }

    if (index NEQ NOT_PRESENT_16BIT)
    {

      if(first_sync_req EQ TRUE) 
      {
        first_sync_req = FALSE;

        if(alr_data->nc_data.c_sync_req > 0)
        {

          TRACE_EVENT("RE-SYNC interrupted");
          alr_data->nc_data.c_sync_intrupted = TRUE;

          nc_stop_all();

          if(!alr_data->nc_data.c_sync_req)
          {
            alr_data->nc_data.tim_state = NC_CONF_PENDING;
            SET_STATE(STATE_NC_PROC, NC_ACQUIRE);
          }
        }      
      } /* first_sync_req */

      /*
       * Request synchronisation for this cell
       */
      nc_build_sync_req(index);
      alr_data->nc_data.cell[index].new_strong_cell = FALSE;
      TRACE_EVENT_P1("[%d]N_S_C sync req",alr_data->nc_data.cell[index].ba_arfcn);
    }
    else
      break; /*while loop */
  } /* while( c_sync_req < MAX_L1_SYNC_CNT) */
}
/*
+--------------------------------------------------------------------+
| PROJECT :                             MODULE  : ALR_NC             |
| STATE   : code                        ROUTINE : nc_set_si4_si78    |
+--------------------------------------------------------------------+

  PURPOSE : Replaces the common  code for SI4 and SI7-8.

*/

LOCAL void  nc_set_si4_si78 ( USHORT                  index,
                              T_MPHC_NCELL_BCCH_IND  *data_ind,
                              UBYTE                   sys_info_78)
{
  GET_INSTANCE_DATA;
  ALR_TRACE_NC ("nc_set_si4_si78");
#if defined(TRACING)
    TRACE_EVENT_P4("NC%u[%d] BCCH ok %s",
                     index,alr_data->nc_data.cell[index].ba_arfcn EQ NOT_PRESENT_16BIT ? -1 
                    : alr_data->nc_data.cell[index].ba_arfcn&ARFCN_MASK, "%s",
                      (sys_info_78) ? "si78" : "si4");
#endif  /* TRACING */
  /*
   * additional cell selection info from sys info 7 or 8 is not necessary
   */
  if (alr_data->nc_data.cell[index].status EQ READ_BCCH_PENDING_RR_NOT_INFORMED)
  {
    /*
     * store data if RR is not informed yet about synchronisation
     */
    nc_store_bcch((T_MPHC_DATA_IND *) data_ind, index, sys_info_78);
    nc_set_status (index, FB_SB_SYNC_RR_NOT_INFORMED);
  }
  else
  {
    /*
     * forward information to RR
     */
    ma_send_unitdata ((T_MPHC_DATA_IND *)data_ind);
    nc_set_fb_sb_sync_initial (index);
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : ALR_NC                     |
| STATE   : code                ROUTINE : nc_highst_field_strng      |
+--------------------------------------------------------------------+

  PURPOSE : Replaces the common  code of ' searching the candidate 
            with the highest fieldstrength' from functions-
            "nc_process_status_sync" and "nc_process_status_bcch".

*/
LOCAL void nc_highst_field_strng (USHORT *index, T_PRO_STA_TYPE  nc_pro_sta_type)
{
  GET_INSTANCE_DATA;
  UBYTE  i;
  USHORT c_ba_arfcn = alr_data->nc_data.c_ba_arfcn;
  T_NC*  pbsic = &alr_data->nc_data.cell[LAST_BSIC_REQ];
  ALR_TRACE_NC ("nc_highst_field_strng");
  for (i = 0; i < c_ba_arfcn; i++)
  {
    /*
     * search for the candidate with the highest fieldstrength
     */
    T_NC* pcell = &alr_data->nc_data.cell[i];

    if ((pcell->ba_arfcn NEQ pbsic->ba_arfcn) AND
        (pcell->ba_arfcn NEQ alr_data->serving_cell))
    {
      if((pcell->status EQ READ_FB_SB AND (nc_pro_sta_type EQ PRO_STA_SYNC)) OR 
         ((pcell->status EQ READ_BCCH OR 
           pcell->status EQ READ_BCCH_RR_NOT_INFORMED) AND (nc_pro_sta_type EQ PRO_STA_BCCH)))
       {
        if (*index EQ NOT_PRESENT_16BIT)
          *index = i;
        else
        {
         if (pcell->rxlev_average >
             alr_data->nc_data.cell[*index].rxlev_average)
           *index = i;
        }
      }
    }
  }
}
#if defined (REL99) && defined (TI_PS_FF_EMR)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103) MODULE  : ALR_NC                           |
| STATE   : code          ROUTINE : nc_fill_ncell_enh_meas_results                  |
+--------------------------------------------------------------------+

PURPOSE : The purpose of this function is to handle all the specific requirements
          for EMR - sorting according to priority as given in 5.08 V8.16.0, sec.8.4.8.1,
          and scaling of rxlev values.
*/
LOCAL void nc_fill_ncell_enh_meas_results( T_MPH_MEASUREMENT_IND *p_rep)
{
  GET_INSTANCE_DATA;
  T_enh_bin           sc_bin;
  T_enh_bin           mband_bin[5]; /* 0 - 900, 1 - 1800, 2 - 400, 3 - 1900, 4 - 850*/
  T_enh_bin           rest_bin;
  UBYTE               i;  
  T_NC                *pcell = alr_data->nc_data.cell;
  T_enh_para_struct   *p_enh = &alr_data->nc_data.emr_data.enh_para;
  
  p_rep->ncells.no_of_ncells = 0;
  memset (&sc_bin,0,sizeof (T_enh_bin));
  memset(mband_bin, 0, 5 * sizeof(T_enh_bin) );
  memset(&rest_bin,0, sizeof(T_enh_bin) );
  /* Step 1: Sort cells into different bins - serving cell band bin,
    non-serving cell band bin, and the rest which can contain invalid BSIC
    cells*/
  nc_sort_cells_into_bins( &sc_bin, mband_bin, &rest_bin);

  /* Step 2: Fill the cells from each bin, after sorting in descending order
     of RXLEV, in the order of : sc_bin, mband_bin and rest_bin.
     Further number of cells from sc_bin and mband_bin is restricted by SERVING_BAND
     and MULTIBAND reporting parameters*/
  if (sc_bin.num_valid > 0)
  {
    nc_sort_and_store_meas_results(pcell, &sc_bin, p_enh->servingband_rep, p_rep);
    /*remaining cells in sc bin apart from first servingband_rep number of cells,
      will be filled into rest bin */
    if (sc_bin.num_valid > p_enh->servingband_rep)
      nc_add_to_rest(&rest_bin,&sc_bin,p_rep->ncells.no_of_ncells);
  }    
  /* Fill cells from multi band after sorting*/
  for ( i = 0; i < MAX_NUM_BANDS; i++)    
  {
    if ( (p_rep->ncells.no_of_ncells < MAX_NUM_CELLS_IN_REPORT) AND
         (mband_bin[i].num_valid > 0) )
    {
      nc_sort_and_store_meas_results(pcell, &mband_bin[i], p_enh->multiband_rep, p_rep);    
      if (mband_bin[i].num_valid > p_enh->multiband_rep )
        nc_add_to_rest(&rest_bin, &mband_bin[i], p_enh->multiband_rep );
    }         
  }
  
  /*Sort the cells in rest bin */
  if ( (p_rep->ncells.no_of_ncells < MAX_NUM_CELLS_IN_REPORT ) AND
       ( rest_bin.num_valid > 0) )
       nc_sort_rest(p_rep, &rest_bin); 

  /* This leaves us with final step in neighbour cell measurement reporting - scaling.*/
  p_rep->scale_used = nc_scale_rxlev (p_enh->scale_order, 
                                      p_rep->ncells.no_of_ncells,
                                      p_rep->ncells.rx_lev);    

  return;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103) MODULE  : ALR_NC                           |
| STATE   : code          ROUTINE : nc_sort_cells_into_bins                  |
+--------------------------------------------------------------------+

PURPOSE : The purpose of this function is to sort the cells into various
          bins. This is the first step in handling reporting priority 
          as given in 5.08 sec.8.4.8.1
*/
LOCAL void nc_sort_cells_into_bins( T_enh_bin *p_sbin,
                                   T_enh_bin *p_mbin,
                                   T_enh_bin *p_rbin)
{
  GET_INSTANCE_DATA;
  UBYTE               i = 0;
  UBYTE               j;
  USHORT              arfcn1;
  UBYTE               bsic1;  
  UBYTE               band;
  UBYTE               sc_band;
  UBYTE               rth;  
  BOOL                bsic_found;
  T_NC                *pcell = alr_data->nc_data.cell;
  T_enh_para_struct   *p_enh = &alr_data->nc_data.emr_data.enh_para;

  sc_band = get_band_index_for_emr (alr_data->serving_cell );
  p_rbin->num_valid = 0;
  p_mbin->num_valid = 0;
  p_sbin->num_valid = 0;

  while ( pcell[i].ba_arfcn NEQ NOT_PRESENT_16BIT ) 
  {
    /* Do the rest only if it is in BA list and it's a valid value of BSIC*/
    if (( pcell[i].ba_status EQ IN_BA ) AND ( pcell[i].bsic NEQ NOT_PRESENT_8BIT ) 
         AND 
         (( pcell[i].ba_arfcn NEQ alr_data->serving_cell )
           OR
           (( pcell[i].ba_arfcn EQ alr_data->serving_cell ) AND ( alr_data->nc_data.sc_included EQ TRUE )) 
         )
       )
    {
      arfcn1 = pcell[i].ba_arfcn;
      bsic1  = pcell[i].bsic;      
      bsic_found  = FALSE;
      for ( j = 0; j < p_enh->num_valid_cells; j++ )
      {
        if ( p_enh->enh_cell_list[j].arfcn EQ arfcn1 ) 
        {
          if (p_enh->enh_cell_list[j].bsic EQ bsic1 )
          {
              /*Find the band of this arfcn*/
            band = get_band_index_for_emr(arfcn1);
            rth = p_enh->enh_rep_data[band].rep_threshold; 
            if (band EQ sc_band )
            {
              /* Apply the threshold criteria for serving band*/
              if ( rth NEQ REP_THRESHOLD_INF )                  
              {
                if((rth EQ REP_THRESHOLD_NONE) OR (pcell[i].rxlev_average > (rth * 6) ) )         
                {
                  p_sbin->index[p_sbin->num_valid] = i;
                  p_sbin->nc_index[p_sbin->num_valid] = j;
                  p_sbin->priority[p_sbin->num_valid++] = p_enh->enh_cell_list[j].rep_priority;
                }      
                else
                {
                  p_rbin->index[p_rbin->num_valid] = i;
                  p_rbin->nc_index[p_rbin->num_valid] = j;
                  p_rbin->priority[p_rbin->num_valid++] = p_enh->enh_cell_list[j].rep_priority;
                }
              } /*otherwise report-never*/            
            } /* otherwise cell in non-serving band*/
            else
            {
               /* Apply the threshold criteria for this band*/
              if ( rth NEQ REP_THRESHOLD_INF )                  
              {
                if((rth EQ REP_THRESHOLD_NONE) OR (pcell[i].rxlev_average > (rth * 6) ) )
                {
                  p_mbin[band].index[p_mbin[band].num_valid] = i;
                  p_mbin[band].nc_index[p_mbin[band].num_valid] = j;
                  p_mbin[band].priority[p_mbin[band].num_valid++] = p_enh->enh_cell_list[j].rep_priority;
                }                  
                else
                {
                  p_rbin->index[p_rbin->num_valid] = i;
                  p_rbin->nc_index[p_rbin->num_valid] = j;
                  p_rbin->priority[p_rbin->num_valid++] = p_enh->enh_cell_list[j].rep_priority;
                }                  
              } /*otherwise report never*/            
            } 
            bsic_found = TRUE;
            break;
          } /* end of if on 'bsic1'*/                      
        } /* end of if on 'arfcn1'*/
      } /*end of loop on 'j'*/
      
      /* If there's no cell with the given BSIC, then it falls under Invalid BSIC category 
         */
      if ( bsic_found EQ FALSE ) 
      {
        /*check whether Invalid BSIC reporting is enabled or not and NCC part is permitted or not*/
        if ( (p_enh->inv_bsic_enabled EQ TRUE  ) AND
             ( ((p_enh->ncc_permitted) & (ncc_bit_mask[((bsic1 >> 3) & BIT_012)])) NEQ 0) )
        {
            /*put it in rest bin*/
            p_rbin->index[p_rbin->num_valid] = i;  
            p_rbin->nc_index[p_rbin->num_valid] = NOT_PRESENT_8BIT; /*Indicates that this is an invalid BSIC cell*/
            p_rbin->priority[p_rbin->num_valid++] = REP_PRIOR_HIGH;
        }
      }      
    } /* if ba_status */
    i++;
  } /* end of while */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103) MODULE  : ALR_NC                           |
| STATE   : code          ROUTINE : get_band_index_for_emr                  |
+--------------------------------------------------------------------+

PURPOSE : This is implementation specific function. This is used to
          get index into emr reporting data (reporting threshold and
          offset) array, which is band specific.
*/
LOCAL UBYTE get_band_index_for_emr (USHORT arfcn)
{
  
  if ( INRANGE(LOW_CHANNEL_900, arfcn, HIGH_CHANNEL_900) )
  {
    return 0;
  }
  else if ( INRANGE(LOW_CHANNEL_1800, arfcn, HIGH_CHANNEL_1800))
  {
    return 1;
  }
  else if (INRANGE(LOW_CHANNEL_1900, arfcn, HIGH_CHANNEL_1900))
  {
    return 3;
  }
  else if ( INRANGE(LOW_CHANNEL_850, arfcn, HIGH_CHANNEL_850))
  {
    return 4;
  }
  else
  {
    return 2;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103) MODULE  : ALR_NC                           |
| STATE   : code          ROUTINE : nc_sort_and_store_meas_results                  |
+--------------------------------------------------------------------+

PURPOSE : This function sorts the cells collected in each bin, in
          descending order of RXLEV and then fills it into the report
          that has to be sent to RR.
*/
LOCAL void nc_sort_and_store_meas_results(T_NC *pcell, T_enh_bin *p_bin, UBYTE nbr_rpt, 
                                          T_MPH_MEASUREMENT_IND *p_rep)
{
  UBYTE     n = p_rep->ncells.no_of_ncells;
  UBYTE     i;
  UBYTE     j;
  UBYTE     max_rxlev;
  UBYTE     k;

  for (i = 0; i < (p_bin->num_valid ) ; i++)
  {
    if ( (n < MAX_NUM_CELLS_IN_REPORT) AND (nbr_rpt > 0) )
    {
      max_rxlev = pcell[p_bin->index[i]].rxlev_average;
      k = i;
      for ( j = i+1; j < p_bin->num_valid ; j++ )
      {
        if ( pcell[p_bin->index[j]].rxlev_average > max_rxlev )
        {
          k = j; 
          max_rxlev = pcell[p_bin->index[j]].rxlev_average;
        }
      }
      p_rep->ncells.arfcn[n] = pcell[p_bin->index[k]].ba_arfcn;
      p_rep->ncells.bsic[n] = pcell[p_bin->index[k]].bsic;
      p_rep->ncells.rx_lev[n] = pcell[p_bin->index[k]].rxlev_average;
      p_rep->ncells.time_alignmt[n] = pcell[p_bin->index[k]].time_align;
      p_rep->ncells.frame_offset[n] = pcell[p_bin->index[k]].frame_offset;
      p_rep->nc_index[n]  = p_bin->nc_index[k];
      /*swap the contents to get the information in sorted form*/
      if ( k NEQ i)
        nc_swap_bin_content (p_bin, i,k);    
      n++;
      nbr_rpt--;
    }
    else
      break;
  }
  p_rep->ncells.no_of_ncells = n;
  return;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103) MODULE  : ALR_NC                           |
| STATE   : code          ROUTINE : nc_add_to_rest                  |
+--------------------------------------------------------------------+

PURPOSE : This function updates the rest bin by appending the left out
          cells in the other bins. These may also have to be reported
          along with the cells in rest bin.
*/
LOCAL void nc_add_to_rest(T_enh_bin *p_r, T_enh_bin *p_b, UBYTE start_index)
{
  UBYTE     i;

  for ( i = start_index; i < p_b->num_valid ; i++ )
  {
    p_r->index[p_r->num_valid] = p_b->index[i];
    p_r->nc_index[p_r->num_valid] = p_b->nc_index[i];
    p_r->priority[p_r->num_valid++] = p_b->priority[i];
  }  
  return;
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103) MODULE  : ALR_NC                           |
| STATE   : code          ROUTINE : nc_scale_rxlev                  |
+--------------------------------------------------------------------+

PURPOSE : This function performs the scaling of RXLEV.
*/
LOCAL UBYTE nc_scale_rxlev (UBYTE scale, UBYTE no_of_cells, UBYTE *rx_lev)
{
  GET_INSTANCE_DATA;
  UBYTE     i;
    /*If scaling is automatic, then we find the scaling as below:
     If the maximum RXLEV is greater than 63 (MAX_RXLEV), then we add 10dB
     and check whether it falls into the range after scaling*/
  /* Note: The assumption is L1 reports values in the following way -
     if actual value of rxlev (x) is between y and y+1 dBm, then the reported value
     is 110+x rounded off to next integer. */
  if ( scale EQ SCALE_AUTO )
  {
    UBYTE     max = alr_data->nc_data.rxlev_val; /*need to scale serving cell RXLEV also*/        
    for ( i = 0; i < no_of_cells ; i++ )
    {
      if (rx_lev[i] > max )
        max = rx_lev[i];
    }
    /*if ( (max > 63 ) AND ( (max -10) < 64) )*/
    if(max > 63 ) 
    {
      /*even if the max value is greater than 73, we still scale by 10 so that atleast
      rxlev values of some other cells will come into range*/
      scale = 10;
    }
    else
      scale = 0;
  }
  else
    scale = scale * 10;

  /*Apply the scale to the values to be reported*/
  alr_data->nc_data.rxlev_val -= scale;
  for ( i = 0; i < no_of_cells ; i++ )
  {
    if (rx_lev[i] > scale )
      rx_lev[i] -= scale;
    else
      rx_lev[i] = 0;
    if (rx_lev[i] > 63 )
      rx_lev[i] = 63;
  }
  return (scale/10);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103) MODULE  : ALR_NC                           |
| STATE   : code          ROUTINE : nc_fill_serv_cell_enh_meas_results                  |
+--------------------------------------------------------------------+

PURPOSE : This function fills the serving cell measurement results into
          the report that is sent to RR.
*/
LOCAL void nc_fill_serv_cell_enh_meas_results(T_MPH_MEASUREMENT_IND *p_rep)
{
  GET_INSTANCE_DATA;
  p_rep->rxlev_val = alr_data->nc_data.rxlev_val;
  p_rep->mean_bep = alr_data->nc_data.m_bep;
  p_rep->cv_bep = alr_data->nc_data.cv_bep;
  p_rep->nbr_rcvd_blks = alr_data->nc_data.nbr_rcvd_blks;
  p_rep->dtx = alr_data->nc_data.act_dtx;
  p_rep->rx_qual_full = alr_data->nc_data.rxqual_full;
  p_rep->valid = TRUE;  
  p_rep->arfcn = alr_data->serving_cell;
  p_rep->fn_offset = 103;   /* average of SDCCH/FACCH */
  p_rep->dtx = alr_data->nc_data.act_dtx;
  p_rep->rx_lev_full = (UBYTE)alr_data->nc_data.rxlev_full;
  if (p_rep->rx_lev_full > 63)
    p_rep->rx_lev_full = 63;
  p_rep->rx_lev_sub = (UBYTE)alr_data->nc_data.rxlev_sub;
  if (p_rep->rx_lev_sub > 63)
    p_rep->rx_lev_sub = 63;  
  p_rep->rx_qual_sub = alr_data->nc_data.rxqual_sub;
  p_rep->otd = alr_data->nc_data.tav;
  return;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103) MODULE  : ALR_NC                           |
| STATE   : code          ROUTINE : nc_sort_rest                  |
+--------------------------------------------------------------------+

PURPOSE : This function sorts cells in the rest bin which may contain both valid and 
  invalid BSIC cells. Here we use the reporting offset along with measured RXLEV,
  for sorting. According to 5.08, sec.8.4.8.1 we need to find those cells whose sum of 
  RXLEV and xxx_reporting_offset is maximum. This can be acheived by sorting the cells
  in the order of their rxlevm, where rxlevm = rxlev + offset.
*/
LOCAL void nc_sort_rest(T_MPH_MEASUREMENT_IND *p_rep, T_enh_bin *p_bin)
{
  GET_INSTANCE_DATA;
  UBYTE                 n = p_rep->ncells.no_of_ncells;    
  UBYTE                 j;
  UBYTE                 max_rxlevm;
  UBYTE                 band;
  UBYTE                 rxlevm;
  UBYTE                 k;  
  ULONG                 xtra_cells_bmp = 0;
  UBYTE                 i;
  T_NC                  *pcell = alr_data->nc_data.cell;
  T_enh_para_struct     *p_enh = &alr_data->nc_data.emr_data.enh_para;  
  
  for (i = 0; i < p_bin->num_valid ; i++)
  {
    if ( n < MAX_NUM_CELLS_IN_REPORT ) 
    {
      band = get_band_index_for_emr (pcell[p_bin->index[i]].ba_arfcn);
      max_rxlevm = pcell[p_bin->index[i]].rxlev_average + 
        (p_enh->enh_rep_data[band].rep_offset * 6) ;
      k = i;      
      for ( j = i+1; j < p_bin->num_valid ; j++ )
      {
        band = get_band_index_for_emr (pcell[p_bin->index[j]].ba_arfcn);
        rxlevm = pcell[p_bin->index[j]].rxlev_average +
          (p_enh->enh_rep_data[band].rep_offset * 6) ;
        if ( rxlevm > max_rxlevm )
        {
          k = j;
          max_rxlevm = rxlevm;            
        }
      }          
      
      /*When reduced reporting is enabled we have to report low priority cell once
      in 4 reporting periods. The RXLEV then should be average of previous two periods*/
      
      j = p_bin->index[k];
      /*swap the contents of rest_bin to push max in this iteration out of 
      next search*/
      if ( k NEQ i)      
        nc_swap_bin_content (p_bin, i,k);           
      k = i;
      
      if ( (pcell[j].one_of_six NEQ TRUE ) AND
           (p_enh->rep_rate EQ REDUCED_REP_RATE) AND 
           (p_bin->priority[k] EQ REP_PRIOR_NORM ))
      {          
        rxlevm = (pcell[j].rxlev_average + pcell[j].last_rxlev) >> 1; /*division by 2*/    
        pcell[j].last_rxlev = pcell[j].rxlev_average;
        pcell[j].rxlev_average = rxlevm;
        xtra_cells_bmp |= (1<<k); /*The lower index, has the highest rxlev due to sorting done above
                                    This ensures we don't need to sort again*/                  
      }
      else        
      {
        p_rep->ncells.arfcn[n] = pcell[j].ba_arfcn;
        p_rep->ncells.bsic[n] = pcell[j].bsic;
        p_rep->ncells.rx_lev[n] = pcell[j].rxlev_average;
        p_rep->ncells.time_alignmt[n] = pcell[j].time_align;
        p_rep->ncells.frame_offset[n] = pcell[j].frame_offset;
        p_rep->nc_index[n] = p_bin->nc_index[k];
        /*Verify whether this cell is one of six strongest and 
          it has a Invalid BSIC to set the bsic_seen value*/
        if ( (p_bin->nc_index[k] EQ NOT_PRESENT_8BIT ) AND
             (pcell[j].one_of_six EQ TRUE) )
             p_rep->bsic_seen = TRUE; 
        n++;
      }      
      
    } /*end of if 'n'*/     
  } /*end of for 'i'*/
  
    /*In case there are any low priority cells that hadn't got a chance above,
  we can report these cells here*/          
  j=0;
  while ( (n < MAX_NUM_CELLS_IN_REPORT) AND (xtra_cells_bmp NEQ 0) )
  {
    /*include these extra cells also in the report, first all those
      whose inclusion is due*/
    k = p_bin->index[j];
    if ( ((xtra_cells_bmp >>j) & 0x00000001) EQ TRUE )
    {      
      p_rep->ncells.arfcn[n] = pcell[k].ba_arfcn;
      p_rep->ncells.bsic[n] = pcell[k].bsic;
      p_rep->ncells.rx_lev[n] = pcell[k].rxlev_average;
      p_rep->ncells.time_alignmt[n] = pcell[k].time_align;
      p_rep->ncells.frame_offset[n] = pcell[k].frame_offset;
      p_rep->nc_index[n]  = p_bin->nc_index[j];
      xtra_cells_bmp &= (~ (ULONG)((1<<j) )); /*resets the bit for location p*/
      n++;
    }
    j++;
  }          
  p_rep->ncells.no_of_ncells = n;  
  return;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103) MODULE  : ALR_NC                           |
| STATE   : code          ROUTINE : nc_swap_bin_content                  |
+--------------------------------------------------------------------+

PURPOSE : 
*/
LOCAL void nc_swap_bin_content (T_enh_bin *p_bin, UBYTE i, UBYTE j )
{
  UBYTE     temp;

  temp = p_bin->index[i];
  p_bin->index[i] = p_bin->index[j];
  p_bin->index[j] = temp;
  temp = p_bin->nc_index[i];
  p_bin->nc_index[i] = p_bin->nc_index[j];
  p_bin->nc_index[j] = temp;
  temp = p_bin->priority[i];
  p_bin->priority[i] = p_bin->priority[j];
  p_bin->priority[j] = temp;

  return;
}


/*##############################################################################
  Refer sec.8.2.5 of 5.08 for mapping of mean and cv BEP values.
  Refer L1-interface documents for formats (F1.31 and F3.13) description
  An example is given at the end of these functions to illustrate the algorithm
  given here for conversion from these formats to a format for L23 processing
  #############################################################################*/
/*
+------------------------------------------------------------------------------
| Function    : nc_map_mean_bep
+------------------------------------------------------------------------------
| Description : This function is for mapping the measured and scaled BEP values
|               to the corresponding enums as defined in 5.08. The mapping table,
|               it's size are constants and the value that needs to be mapped is passed as 
|               input parameters.The function returns the mapped value .  
+------------------------------------------------------------------------------
*/
LOCAL UBYTE nc_map_mean_bep(ULONG value)
{
  UBYTE i=0;
  
  TRACE_FUNCTION( "nc_map_mean_bep" );
  while( i < MAX_MEAN_BEP_VAL )
  {
    if( value >= mbep_map[i] )
      break;
    i++;
    if(i EQ MAX_MEAN_BEP_VAL)
      return i-1; /* Any value lower than the lowest is mapped to
                     the last enum value '31'*/    
  }
  return i;
}

/*
+------------------------------------------------------------------------------
| Function    : nc_map_cv_bep
+------------------------------------------------------------------------------
| Description : This function is for mapping the measured and scaled BEP values
|               to the corresponding enums as defined in 5.08. The mapping table,
|               it's size are constants and the value that needs to be mapped is passed as 
|               input parameters. 
+------------------------------------------------------------------------------
*/
LOCAL UBYTE nc_map_cv_bep(USHORT value)
{
  UBYTE i=0;
  
  TRACE_FUNCTION( "nc_map_cv_bep" );
  while( i < MAX_CV_BEP_VAL )
  {
    if( value >= cvbep_map[i] )
      break;
    i++;
    if(i EQ MAX_CV_BEP_VAL)
    return i-1; /* Any value lower than the lowest is mapped to
                     the last enum value '7'*/
  }
  return i;
}



/*########### E X A M P L E for FORMAT F8.8 ##########
FX.Y format means: 
X MSB bits dedicated to integer part
Y LSB bits are dedicated to the floating part
So for example with F8.8, you need to divide the 16 bits word by 2^8 to have the 
equivalent floating value

Example
bep = 0x1234 (4660)  F8.8 encoded
then bep = 4660 / 2^8 = 18,203125... 

The mapping table given by "cvbep_map" contains the actual values
multiplied by 2^8
while the mapping table given by "mbep_map" contains the actual values
multiplied by 2^26
#####################################################################*/

#endif

#if defined (TI_PS_FF_RTD) AND defined (REL99)

/*
+------------------------------------------------------------------------------
| Function    : nc_sync_fn_offset_calculation
+------------------------------------------------------------------------------
| Description : This function will calculate Fn_offset for a given RTD value.
+------------------------------------------------------------------------------
*/

LOCAL UINT nc_sync_fn_offset_calculation(USHORT rtd_value)
{
  if((rtd_value & 0x8000) EQ 1) 
    return (UINT)(rtd_value / NUM_OF_TDMA_FRAMES);
  else
  {
    rtd_value &= RTD_12BIT_EXTRACT;
    return (UINT)(rtd_value / NUM_OF_TDMA_FRAMES);
  }
}

/*
+------------------------------------------------------------------------------
| Function    : nc_sync_time_alignment_calculation
+------------------------------------------------------------------------------
| Description : This function will calculate time_alignment for a RTD value
|               given in units of 1/64 TDMA frame.
+------------------------------------------------------------------------------
*/

LOCAL UINT nc_sync_time_alignment_calculation(USHORT rtd_value)
{
  UBYTE  frame_offset;
  if((rtd_value & 0x8000) EQ 1)
  {
    frame_offset = nc_sync_fn_offset_calculation(rtd_value);
    return (UINT)((rtd_value - (frame_offset * NUM_OF_TDMA_FRAMES))*MAX_NUM_OF_QBITS/NUM_OF_TDMA_FRAMES);
  }
  else
  {
    rtd_value &= RTD_12BIT_EXTRACT;
    frame_offset = nc_sync_fn_offset_calculation(rtd_value);
    return (UINT)((rtd_value - (frame_offset * NUM_OF_TDMA_FRAMES))*MAX_NUM_OF_QBITS/NUM_OF_TDMA_FRAMES);
  }
}
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
LOCAL BOOL nc_compare_bsic_std (UBYTE bsic_std)
{
  BOOL match_std = TRUE;
  switch (std)
  {
    case STD_900:
    case STD_EGSM:
    case STD_1900:
    case STD_1800:
    case STD_850:
      if (bsic_std EQ std)
        match_std = FALSE;
      break;
    case STD_DUAL:
      if ((bsic_std EQ STD_900) OR (bsic_std EQ STD_1800) OR (bsic_std EQ STD_DUAL))
        match_std = FALSE;
      break;
    case STD_DUAL_EGSM:
      if ((bsic_std EQ STD_EGSM) OR (bsic_std EQ STD_1800) OR (bsic_std EQ STD_DUAL_EGSM))
        match_std = FALSE;
      break;
    case STD_DUAL_US:
      if ((bsic_std EQ STD_850) OR (bsic_std EQ STD_1900) OR (bsic_std EQ STD_DUAL_US))
        match_std = FALSE;
      break;
    case STD_850_1800:
      if ((bsic_std EQ STD_850) OR (bsic_std EQ STD_1800) OR (bsic_std EQ STD_850_1800))
        match_std = FALSE;
      break;
    case STD_900_1900:
      if ((bsic_std EQ STD_EGSM) OR (bsic_std EQ STD_1900) OR (bsic_std EQ STD_900_1900))
        match_std = FALSE;
      break;
    case STD_850_900_1800:
      if ((bsic_std EQ STD_EGSM) OR (bsic_std EQ STD_850) OR (bsic_std EQ STD_1800) OR (bsic_std EQ STD_850_900_1800))
        match_std = FALSE;
      break;
    case STD_850_900_1900:
      if ((bsic_std EQ STD_EGSM) OR (bsic_std EQ STD_850) OR (bsic_std EQ STD_1900) OR (bsic_std EQ STD_850_900_1900))
        match_std = FALSE;
      break;
    default:
      TRACE_EVENT_P1 ("Invalid std value in nc_compare_bsic_std %d", std);
      break;
  }
  return match_std;
}
#endif

#endif
