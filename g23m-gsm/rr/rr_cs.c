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
|  Purpose :  This module defines the functions for Cell Selection
|             capability of Radio Resource.
+-----------------------------------------------------------------------------
*/

#ifndef RR_CS_C
#define RR_CS_C

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

/*==== EXPORT =====================================================*/

/*==== PRIVATE ====================================================*/
static void  cs_remove_co_chan     (USHORT arfcn, USHORT co_arfcn);
static void  cs_remove_co_channels (USHORT arfcn);
static UBYTE cs_check_channel      (USHORT arfcn);
static BOOL  cs_sync_next_bsic     (void);


/*==== VARIABLES ==================================================*/

EXTERN UBYTE test_house;

/*==== FUNCTIONS ==================================================*/

LOCAL  void cs_add_current_network_to_found_list ();
LOCAL  BOOL cs_check_black_list_criteria(U8 region, U16 arfcn, U8 rxlev);
LOCAL  BOOL cs_is_in_black_list(U8 region, U16 arfcn);
LOCAL  BOOL cs_is_in_white_list(U8 region, U16 arfcn);
LOCAL  U8   cs_get_sync_fail_counter(U8 region, U16 arfcn);
LOCAL  void cs_reset_sync_fail_counter(U8 region, U16 arfcn);
LOCAL  void cs_inc_sync_fail_counter(U8 region, U16 arfcn);
LOCAL  void cs_use_white_list_info(U8 num_of_chan);
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
LOCAL UBYTE cs_update_std_ffs (UBYTE new_std);
#else
LOCAL UBYTE get_band_index(USHORT arfcn);
#endif


/*
 * -------------------------------------------------------------------
 * Procedures
 * -------------------------------------------------------------------
 */

/*
 * The cell selection process bases on attributes for each channel
 * of the power campaign. This attributes are defined by a bitmap
 *
 *          Bit    1 0    priority
 *          ---------------------------------
 *                 0 0    low priority
 *                 0 1    mid priority
 *                 1 0    high priority
 *                 1 1    reserved
 *
 *          The selection of a cell for synchronisation depends on
 *          the priority and the fieldstrength. It will be the cell
 *          selected, which is not checked yet, has the highest
 *          priority and the highest fieldstrength.
 *
 *          Bit    2      reserved
 *
 *          Bit    3      checked flag
 *          ---------------------------------
 *                 0      cell is not checked
 *                 1      cell is checked
 *
 *          The flag indicates whether a cell is already checked or not.
 *
 *          Bit    4      detected flag
 *          ---------------------------------
 *                 0      bcch is not detected
 *                 1      bcch is checked
 *
 *          The flag indicates whether a BCCH is detected or not.
 *
 *          Bit    5      emergency cell flag
 *          ---------------------------------
 *                 0      emergency services possible
 *                 1      emergency services not possible
 *
 *          The flag indicates whether emergency services are possible or not.
 *
 *          Bit    6      low priority cell flag
 *          ---------------------------------
 *                 0      no low priority cell
 *                 1      low priority cell
 *
 *          The flag indicates whether a cell has a low priority or not.
 *
 *          Bit    8      BCCH info list flag
 *          ---------------------------------
 *                 0      not member of the BCCH info list
 *                 1      member of the BCCH info list
 *
 *          The flag indicates whether a cell is member of the BCCH information
 *	    list or not.
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_init_process            |
+--------------------------------------------------------------------+

  PURPOSE : Initialisation of the cell selection process.
            Called from 'pei_init' exclusively and rr_deactivate.

*/

GLOBAL void cs_init_process (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cs_init_process()");

  /*
   * set the state and clear all variables.
   */
  SET_STATE (STATE_CELL_SEL, CS_NULL);
  memset (&rr_data->cs_data, 0, sizeof (&rr_data->cs_data));

  rr_data->cs_data.arfcn_sc = NOT_PRESENT_16BIT;
  rr_data->cs_data.arfcn_cr = NOT_PRESENT_16BIT;

  /* CSI-LLD section:4.1.1.11 */
  rr_data->cs_data.initial_plmn_search = INITIAL_PLMN_SEARCH_NOT_ACTIVE;
  
  /* In all simulation testcases written prior to cell selection
   * improvements feature, US_BIT and STD values were not set in 
   * the ARFCN field 
   */
#if !defined (_SIMULATION_)
  rr_data->cs_data.region              = NOT_PRESENT_8BIT;
#endif

  rr_data->cs_data.white_list.last_sc_arfcn = NOT_PRESENT_16BIT;
  rr_data->cs_data.white_list.region        = NOT_PRESENT_8BIT;
  TRACE_EVENT_P1("white_list_region %d", rr_data->cs_data.white_list.region);

  rr_data->start_cell_reselection = TRUE;
  TRACE_EVENT_P1("start_cell_reselection %d", rr_data->start_cell_reselection);

  /* Dynamic configuration parameters */
  rr_data->dyn_config.bl_cs_en         = TRUE;
  rr_data->dyn_config.tfast_cs_val     = TFAST_CS_VALUE;
  rr_data->dyn_config.tnormal_cs_val   = TNORMAL_CS_VALUE;
  rr_data->dyn_config.lim_ser_nps_delay= DELAY_NON_PAR_SEARCH_LIM_SER;

#if defined(_SIMULATION_FFS_)
  rr_csf_ffs_init();
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_set_bcch_info           |
+--------------------------------------------------------------------+

  PURPOSE : The function merges bcch information from SIM to White list
            database
            CSI-LLD - 4.1.2.2.1
*/

GLOBAL void cs_set_bcch_info (T_bcch_info * sim_bcch_info)
{
  GET_INSTANCE_DATA;
  T_LIST tmp_list;
  BUF_neigh_cell_desc cd;
  
  TRACE_FUNCTION ("cs_set_bcch_info()");

  /* Check for availability of sim_bcch_info data */
  if(cs_check_region(rr_data->cs_data.white_list.region) AND 
     (sim_bcch_info NEQ NULL))
  {
    if(sim_bcch_info->v_bcch)
    {
      /* Store the sim_bcch_info data to white_list structure */
      memcpy(cd.b_neigh_cell_desc,sim_bcch_info->bcch,BA_BITMAP_SIZE);
      cd.o_neigh_cell_desc = 0;
      cd.l_neigh_cell_desc = NCELL_DESC_BIT_LEN;

      /* Merge BCCH info into white list */
      for_create_channel_list((T_f_range *)&cd,&tmp_list);
      srv_merge_list(&rr_data->cs_data.white_list.list,&tmp_list);
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_clear_white_list        |
+--------------------------------------------------------------------+

  PURPOSE : The function clears the bcch information (neigbour cell
            description) of sys info 2, 2bis and 2ter stored in RAM 
            and FFS.
            CSI-LLD - 4.1.2.2.6
*/

GLOBAL void cs_clear_white_list(U8 clr_bcch_info)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cs_clear_white_list()");

  if (clr_bcch_info & CLR_WHITE_LIST_SIM)
  {
    dat_send_bcchinfo_mm (NULL);

    TRACE_EVENT ("White List: SIM cleared");
  }

  if (clr_bcch_info & CLR_WHITE_LIST_RAM)
  {
    rr_data->cs_data.arfcn_sc = NOT_PRESENT_16BIT;
    rr_data->cs_data.arfcn_cr = NOT_PRESENT_16BIT;

    memset(&rr_data->cs_data.white_list, 0 , sizeof(T_CS_WHITE_LIST));

    rr_data->cs_data.white_list.last_sc_arfcn = NOT_PRESENT_16BIT;
    rr_data->cs_data.white_list.region        = NOT_PRESENT_8BIT;

    TRACE_EVENT ("White List: RAM cleared");
  }

#if defined (_SIMULATION_FFS_)
  if (clr_bcch_info & CLR_WHITE_LIST_FFS)
  {
    T_CS_WHITE_LIST white_list;
    memset(&white_list,0,sizeof(T_CS_WHITE_LIST));

    rr_data->cs_data.white_list.region = NOT_PRESENT_8BIT;
    rr_data->cs_data.white_list.last_sc_arfcn = NOT_PRESENT_16BIT;

    rr_csf_write_white_list(&white_list);

    TRACE_EVENT ("White List: FFS cleared");
  }
#endif /* _SIMULATION_FFS_ */
}


#if defined (_SIMULATION_FFS_)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_store_white_list        |
+--------------------------------------------------------------------+

  PURPOSE : The function stores while list information to FFS.
            CSI-LLD - 4.1.2.2.3
*/

GLOBAL void cs_store_white_list (void)  
{
  GET_INSTANCE_DATA;
  /* Store the white List to FFS */
  rr_csf_write_white_list(&rr_data->cs_data.white_list);
}
#endif /*_SIMULATION_FFS_*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_use_white_list_info     |
+--------------------------------------------------------------------+

  PURPOSE : The function uses the stored white list information and 
            increases the priority of these channels.
            CSI-LLD - 4.1.2.2.8
*/

LOCAL void cs_use_white_list_info(U8 num_of_chan)
{
  GET_INSTANCE_DATA;
  U8 i, priority;

  TRACE_FUNCTION ("cs_use_white_list_info()");

  /* Boot Time: If white list is invalidated by the second activate_req,
   * decrease priority of white list carriers in power_cnf
   */
  priority = (cs_check_region(rr_data->cs_data.white_list.region)) ?
             CS_HIGH_PRIORITY : CS_LOW_PRIORITY ;

  for (i=0;i<num_of_chan;i++)
  {
    /*
     * Power cnf always contains White list carriers at the top
     */
    rr_data->cs_data.attributes [i] = priority;

    TRACE_EVENT_P3 ("Whie List: change prio of [%d]%u to 0x%02x",
                     rr_data->cs_data.arfcn[i]&ARFCN_MASK,
                     i, rr_data->cs_data.attributes [i]);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_set_all                 |
+--------------------------------------------------------------------+

  PURPOSE : The function clears all attributes and all stored BCCH
            information.

*/

GLOBAL void cs_set_all (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cs_set_all()");

  /*
   * clear all attributes
   */
  memset (&rr_data->cs_data.attributes, 0, sizeof (rr_data->cs_data.attributes));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_set_freq_band           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the frequency band bit mask.

*/

GLOBAL UBYTE cs_get_freq_band (UBYTE pch_interrupt)
{
  UBYTE   freq_bands;

  rr_csf_get_freq_bands (&freq_bands);

  if (!pch_interrupt)
  {
    /*
     * Power measurements without PCH interruption => PLMN search
     * ----------------------------------------------------------
     * In this case we use the found and set value of 'std' for the frequency
     * area to mask out not available frequencies from 'freq_bands'.
     * Use only frequencies of the current area.
     */
    switch (std)
    {
      case STD_900:
        freq_bands &= BAND_GSM_900;
        break;
      case STD_EGSM:
        freq_bands &= BAND_E_GSM;     /* BAND_GSM_900|BAND_E_GSM */
        break;
      case STD_1900:
        freq_bands &= BAND_PCS_1900;
        break;
      case STD_1800:
        freq_bands &= BAND_DCS_1800;
        break;
      case STD_DUAL:
        freq_bands &= BAND_DUAL;      /* BAND_GSM_900|BAND_DCS_1800 */
        break;
      case STD_DUAL_EGSM:
        freq_bands &= BAND_DUAL_EXT;  /* BAND_GSM_900|BAND_E_GSM|BAND_DCS_1800 */
        break;
      case STD_850:
        freq_bands &= BAND_GSM_850;
        break;
      case STD_DUAL_US:
        freq_bands &= BAND_DUAL_US;   /* BAND_GSM_850|BAND_PCS_1900 */
        break;
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
      case STD_850_1800:
        freq_bands &= BAND_850_1800;      /* BAND_GSM_850|BAND_DCS_1800 */
        break;
      case STD_900_1900:
        freq_bands &= BAND_900_1900;  /* BAND_E_GSM|BAND_PCS_1900 */
        break;
      case STD_850_900_1800:
        freq_bands &= BAND_850_900_1800;  /* BAND_E_GSM|BAND_GSM_850|BAND_DCS_1800 */
        break;
      case STD_850_900_1900:
        freq_bands &= BAND_850_900_1900;   /* BAND_GSM_850|BAND_E_GSM|BAND_PCS_1900 */
        break;
#endif
    }
  }
  return freq_bands;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                     |
| STATE   : code                ROUTINE : get_band_index                   |
+--------------------------------------------------------------------+

  PURPOSE : The function extracts the frequency band index from the given
            'arfcn' parameter.
*/

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
GLOBAL UBYTE cs_get_band_index (USHORT arfcn)
#else
GLOBAL UBYTE get_band_index (USHORT arfcn)
#endif
{
  UBYTE local_std = STD_GET_FROM_ARFCN(arfcn);
  UBYTE sc_band;

  if(local_std EQ STD_NOT_PRESENT)
  {
    /* Parallel search case */
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
    local_std = std;
#else
    local_std = std_bands[std-1];
#endif
  }

  switch (local_std)
  {
    case STD_900:
      sc_band = B_GSM_900;
      break;

    case STD_EGSM:
      sc_band = B_E_GSM;
      break;

    case STD_1800:
      sc_band = B_DCS_1800;
      break;

    case STD_1900:
      sc_band = B_PCS_1900;
      break;

    case STD_850:
      sc_band = B_GSM_850;
      break;

    case STD_DUAL:
      if (arfcn >= LOW_CHANNEL_1800)
        sc_band = B_DCS_1800;
      else
        sc_band = B_GSM_900;
      break;

    case STD_DUAL_EGSM:
      if (arfcn >= LOW_CHANNEL_EGSM)
        sc_band = B_E_GSM;
      else if (arfcn >= LOW_CHANNEL_1800)
        sc_band = B_DCS_1800;
      else if (arfcn EQ CHANNEL_0)
        sc_band = B_E_GSM;
      else
        sc_band = B_GSM_900;
      break;

    case STD_DUAL_US:
      if (arfcn >= LOW_CHANNEL_1900)
        sc_band = B_PCS_1900;
      else
        sc_band = B_GSM_850;
      break;

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
    case STD_850_1800:
      if (arfcn >= LOW_CHANNEL_1800)
        sc_band = B_DCS_1800;
      else
        sc_band = B_GSM_850;
      break;

    case STD_900_1900:
      if (arfcn >= LOW_CHANNEL_EGSM)
        sc_band = B_E_GSM;
      else if (arfcn >= LOW_CHANNEL_1900)
        sc_band = B_PCS_1900;
      else if (arfcn EQ CHANNEL_0)
        sc_band = B_E_GSM;
      else
        sc_band = B_GSM_900;
      break;

    case STD_850_900_1800:
      if (arfcn >= LOW_CHANNEL_EGSM)
        sc_band = B_E_GSM;
      else if (arfcn >= LOW_CHANNEL_1800)
        sc_band = B_DCS_1800;
      else if (arfcn EQ CHANNEL_0)
        sc_band = B_E_GSM;
      else if (arfcn >= LOW_CHANNEL_850)
        sc_band = B_GSM_850;
      else
        sc_band = B_GSM_900;
      break;

    case STD_850_900_1900:
      if (arfcn >= LOW_CHANNEL_EGSM)
        sc_band = B_E_GSM;
      else if (arfcn >= LOW_CHANNEL_1900)
        sc_band = B_PCS_1900;
      else if (arfcn EQ CHANNEL_0)
        sc_band = B_E_GSM;
      else if (arfcn >= LOW_CHANNEL_850)
        sc_band = B_GSM_850;
      else
        sc_band = B_GSM_900;
      break;
#endif

    default:
      sc_band = MAX_NUM_BANDS;
      break;
  }
  /* this trace causes a lot of trace load; switch on only if needed
    TRACE_EVENT_P3 (" [%u] std=%02x, sc_band=%02x", arfcn&ARFCN_MASK, local_std, sc_band);
  */
  return sc_band;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_start_scan              |
+--------------------------------------------------------------------+

  PURPOSE : This function starts the scanning. The power measurements
            are triggered. Power measurements means that layer 1
            performs fieldstrength measurements on all carriers and
            sends a list of the best to RR back for FB/SB scanning and
            BCCH reading. There are two variants: with or without
            paging channel (PCH) interruption. If RR is already in
            idle mode, scanning without PCH interruption is performed,
            that means the MS is pageable during that time.

*/

GLOBAL void cs_start_scan ()
{
  GET_INSTANCE_DATA;
  U16 arfcn;
  U8 i=0;
  U8 region;
  PALLOC (mph_power_req, MPH_POWER_REQ);/* T_MPH_POWER_REQ */

  TRACE_FUNCTION ("cs_start_scan()");
  
  memset(mph_power_req,0,sizeof(T_MPH_POWER_REQ ));

  TIMERSTOP (TREG);
  rr_data->cs_data.act_index = NOT_PRESENT_8BIT;

  switch (GET_STATE (STATE_CELL_SEL))
  {
    case CS_NULL:
    case CS_NULL_ACTIVE:
      /*
       * start power measurements with PCH interruption
       */
      mph_power_req->pch_interrupt   = PCH_INTERRUPT;
      rr_data->first_meas_received   = FALSE;
      rr_data->resel_pending         = FALSE;
      rr_data->c_ncell_bcch          = NOT_INITIALISED;
      rr_data->cs_data.all_freq_area = FALSE;
      /* In all simulation testcases written prior to cell selection
       * improvements feature, US_BIT and STD values were not set in 
       * the ARFCN field 
       */
#if !defined (_SIMULATION_)
      rr_data->cs_data.region      = NOT_PRESENT_8BIT;
#endif

      dat_att_null ();
      SET_STATE (STATE_CELL_SEL, CS_NULL_ACTIVE);

      /* stop possibly parallel search and allow start of cell reselection */
      att_notify_stop_plmn_search (FALSE);
      break;

    case CS_IDLE:
    case CS_IDLE_ACTIVE:
      /*
       * start power measurements without PCH interruption.
       */
      mph_power_req->pch_interrupt = NO_PCH_INTERRUPT;
      SET_STATE (STATE_CELL_SEL, CS_IDLE_ACTIVE);

      /* indicate suspend state of nc monitoring in lower layer */
      rr_data->start_cell_reselection = FALSE;
      TRACE_EVENT_P1("start_cell_reselection %d", rr_data->start_cell_reselection);
      break;

    default:
      break;
  }

#ifndef TI_PS_FF_QUAD_BAND_SUPPORT
  mph_power_req->freq_bands = cs_get_freq_band (mph_power_req->pch_interrupt);
#endif

  /* CSI-LLD : 4.1.3.4.1.4 */
  /* Set the new search mode */
  mph_power_req->search_mode = CS_GET_CURRENT_SEARCH_MODE;

  /* Set Lower Rxlev threshold */
  memcpy( mph_power_req->lower_rxlevel_threshold, rr_data->dyn_config.lower_rxlev_thr, MAX_NUM_BANDS*sizeof(UBYTE));

  /* Copy the black list information */
  for(i=0;i<MAX_REGIONS;i++)
  {
    srv_copy_list ((T_LIST *)&mph_power_req->black_list.list[i],
                   &rr_data->cs_data.black_list.list[i],sizeof(T_LIST));
  }

  /* In case of Black List search, include "grey" carriers also
   * in the Black list while sending to ALR
   */
  if(CS_GET_CURRENT_SEARCH_MODE EQ BLACK_LIST_SEARCH_MODE)
  {
    TRACE_EVENT(" Black_List_Search_Started");

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
    mph_power_req->freq_bands = cs_get_freq_band (FALSE);
#endif

    region = rr_data->cs_data.region;

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
    if (region EQ BOTH_REGIONS)
    {
      for (i=0; i<MAX_REGIONS; i++)
      {
        for(arfcn=CHANNEL_0;arfcn<CHANNEL_0_INTERNAL;arfcn++)
        {
          /* Since Black List search is always a parallel search, "grey" carriers
           * belonging to the current region only have to be added 
           */
          if(cs_get_sync_fail_counter(i,arfcn))
            srv_set_channel((T_LIST*)&mph_power_req->black_list.list[i],arfcn);
        }
      }
    }
    else
    {
#endif
    for(arfcn=CHANNEL_0;arfcn<CHANNEL_0_INTERNAL;arfcn++)
    {
      /* Since Black List search is always a parallel search, "grey" carriers
       * belonging to the current region only have to be added 
       */
      if(cs_get_sync_fail_counter(region,arfcn))
        srv_set_channel((T_LIST*)&mph_power_req->black_list.list[region],arfcn);
    }
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
    }
#endif
  }
  else
  {
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
    mph_power_req->freq_bands = cs_get_freq_band (TRUE);
#endif

    TRACE_EVENT_P9 ( "White List:[%d]Reg,[%d]Arfcn, MCC/MNC r=%x%x%x/%x%x%x/%d",
      rr_data->cs_data.white_list.region,
      rr_data->cs_data.white_list.last_sc_arfcn,
      rr_data->cs_data.white_list.last_sc_lac.mcc[0],
      rr_data->cs_data.white_list.last_sc_lac.mcc[1],
      rr_data->cs_data.white_list.last_sc_lac.mcc[2],
      rr_data->cs_data.white_list.last_sc_lac.mnc[0],
      rr_data->cs_data.white_list.last_sc_lac.mnc[1],
      rr_data->cs_data.white_list.last_sc_lac.mnc[2],
      rr_data->cs_data.white_list.last_sc_lac.lac);
    
    /* Copy white list information 
     * White list cannot be used, when the region it belongs to, is not known
     * or its PLMN ID does not match with the requested PLMN 
     */
    if((!cs_check_region(rr_data->cs_data.white_list.region)) OR 
       ((rr_data->ms_data.req_mm_service NEQ FUNC_PLMN_SRCH) AND
        (GET_STATE(STATE_ATT) NEQ ATT_CS_INIT)) OR 
       ((rr_data->ms_data.req_mm_service EQ FUNC_PLMN_SRCH)   AND
        (!dat_plmn_equal_req(rr_data->cs_data.white_list.last_sc_lac.mcc,
                            rr_data->cs_data.white_list.last_sc_lac.mnc,
                            rr_data->ms_data.plmn.mcc,
                            rr_data->ms_data.plmn.mnc)))) 
    { 
      TRACE_EVENT("White List is not used");
    }
    else
    {
      /* Copy the white list information */
      srv_copy_list((T_LIST*)&mph_power_req->white_list.list,
                     &rr_data->cs_data.white_list.list,sizeof (T_LIST));

      /* Set the region */
      mph_power_req->white_list.region = rr_data->cs_data.white_list.region;
      
      /* White list is valid */
      mph_power_req->white_list.white_list_valid = TRUE;
    }
  }

  /* Handle the search mode timers based on the new search mode */
  cs_handle_search_mode_timer(CS_GET_CURRENT_SEARCH_MODE);

  /*
   * reset the time for net search to finish
   * a PLMN available search during 35 seconds.
   */
  if( rr_data->ms_data.req_mm_service EQ FUNC_NET_SRCH_BY_MMI)
    TIMERSTART(T_PLMN_SEARCH, T_PLMN_SEARCH_VALUE);

  /*
   * start power measurements in layer 1.
   */
  TRACE_EVENT_P2 ("cs_start_scan(): freq_bands=%02x Search Mode = %d", 
                   mph_power_req->freq_bands,mph_power_req->search_mode);

  PSENDX (PL, mph_power_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_start_sync              |
+--------------------------------------------------------------------+

  PURPOSE : Start the synchronisation to FB/SB without power measurements.
            Synchronisation to the frequency correction burst (FB) followed
            by a synchronisation to the synchron burst (SB) is carried
            out by layer 1. It means that the MS is synchronised to the
            base station and then able to read the BCCH to check the cell.

*/

GLOBAL void cs_start_sync (void)
{
  GET_INSTANCE_DATA;
  UBYTE i;

  TRACE_FUNCTION ("cs_start_sync()");

  /* While scanning for limited service, we scan all carriers */
  if((CS_GET_PREVIOUS_SEARCH_MODE NEQ FAST_SEARCH_MODE) AND 
     (rr_data->cs_data.scan_mode EQ CS_SECOND_SCAN))
  {
    for (i=0;i<rr_data->cs_data.max_arfcn;i++)
    /*
     * reset check bit for all channels
     */
    rr_data->cs_data.attributes[i] &= CS_NON_CHECKED_BITS;
  }

  if (cs_sync_next_bsic () EQ FALSE)
  {
    /*
     * indicate the end of scanning to attachment process
     */
    TIMERSTOP(T_PLMN_SEARCH);
    att_no_bsic_ind ();
  }
  else
  {
    /*
     * Another FB/SB scan is requested
     */
    SET_STATE (STATE_CELL_SEL, CS_IDLE_ACTIVE);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_del_list                |
+--------------------------------------------------------------------+

  PURPOSE : A list of carriers (neighbourcell list) is used to speed up
            the cell selection process. All members of the list and the
            list of the power campaign are set to low priority.

*/

GLOBAL void cs_del_list (T_LIST *list)
{
  GET_INSTANCE_DATA;
  UBYTE i;

  TRACE_FUNCTION ("cs_del_list()");

  /*
   * for all cells of the power campaign list
   */

  for (i=0;i<rr_data->cs_data.max_arfcn;i++)
  {
    /*
     * if they are member of the given list and not just checked
     */
    if (srv_get_channel (list, (USHORT)(rr_data->cs_data.arfcn[i]&ARFCN_MASK)) AND
        rr_data->cs_data.act_index NEQ i)
    {
      /*
       * set to low priority
       */
      rr_data->cs_data.attributes [i] &= CS_SET_TO_LOW_PRIORITY;
      /*TRACE_EVENT_P1("[%u] del from list",rr_data->cs_data.arfcn[i]);*/
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_set_list                |
+--------------------------------------------------------------------+

  PURPOSE : A list of carriers (neighbourcell list) is used to speed up
            the cell selection process. All members of the list and the
            list of the power campaign are set to high priority.


*/

GLOBAL void cs_set_list (T_LIST *list)
{
  GET_INSTANCE_DATA;
  UBYTE i;

  TRACE_FUNCTION ("cs_set_list()");

  /*
   * Not in the test house
   */
  if (rr_data->cell_test_operation OR
      dat_test_sim_available ())
    return;

  /*
   * for all channels in the power campaign list
   */
  for (i=0;i<rr_data->cs_data.max_arfcn;i++)
  {
    /*
     * if they are also member of the given list
     */
    if (srv_get_channel (list, (USHORT)(rr_data->cs_data.arfcn[i]&ARFCN_MASK)))
    {
       /*
        * set to high priority
        */
       rr_data->cs_data.attributes [i] &= CS_SET_TO_LOW_PRIORITY;
       rr_data->cs_data.attributes [i] += CS_HIGH_PRIORITY;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_def_list                |
+--------------------------------------------------------------------+

  PURPOSE : Checks whether a member of the list has the requested
            attributes (e.g. emergency or low priority cell)

*/

GLOBAL UBYTE cs_def_list (UBYTE attribute)
{
  GET_INSTANCE_DATA;
  UBYTE i;
  UBYTE result = FALSE;

  TRACE_FUNCTION ("cs_def_list()");

  /*
   * for all channels of the power campaign list
   */
  for (i=0;i<rr_data->cs_data.max_arfcn;i++)
  {
    /*
     * check only cells which have high/mid priority
     */
    if ((rr_data->cs_data.attributes [i] & CS_CHECK_FLAG) EQ 0 AND
        (rr_data->cs_data.attributes [i] & CS_PRIORITY) NEQ 0)
      /*
       * channel is not checked yet, so it is possible that
       * the channel has this attribute
       */
    {
      rr_data->cs_data.attributes [i] = CS_MID_PRIORITY;
      result = TRUE;
    }

    /*
     * check the attribute of the cell. If an attribute
     * like emergency or low priority cell is set, the channel
     * has been checked.
     */
    if (rr_data->cs_data.attributes[i] & attribute)
    {
      /*
       * channel has this attribute, then set priority to high
       */
      rr_data->cs_data.attributes [i] &= CS_SET_TO_LOW_PRIORITY;
      rr_data->cs_data.attributes [i] += CS_HIGH_PRIORITY;
      
      /* reset check bit for this channel */
      rr_data->cs_data.attributes[i] &= CS_NON_CHECKED_BITS;
      result = TRUE;
    }
  }

  return result;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_sync_next               |
+--------------------------------------------------------------------+

  PURPOSE : The function starts the next FB/SB synchronisation request.
            It selects the channel with the highest priority and field-
            strength which is not checked yet.

*/

GLOBAL BOOL cs_sync_next (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cs_sync_next()");

  /*
   * no further channel is possible
   */
  if (cs_sync_next_bsic () EQ FALSE)
  {
    /*
     * stop any timer which controls cell selection
     */
    TIMERSTOP (T_RESELECT);
    SET_STATE (STATE_CELL_SEL, CS_IDLE);

    /*
     * indicate end of search to attachment process
     */
    TIMERSTOP(T_PLMN_SEARCH);
    att_no_bsic_ind ();

    return FALSE;
  }

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_sync_next_bsic          |
+--------------------------------------------------------------------+

  PURPOSE : The function looks for the next candidate for a FB/SB
            sync request. The channel is not checked yet, has the
            highest priority and the highest fieldstrength.
            Return TRUE if another MPH_BSIC_REQ is sent.
*/

static BOOL cs_sync_next_bsic (void)
{
  GET_INSTANCE_DATA;
  UBYTE i;
  UBYTE x = NOT_PRESENT_8BIT;

  TRACE_FUNCTION ("cs_sync_next_bsic()");

  /*
   * try to use 'found channel' from previous cell selection
   */

  if( rr_data->found_channel NEQ NOT_PRESENT_16BIT )
  {
    for ( i = 0; i < rr_data->cs_data.max_arfcn; i++ )
    {
      if ( ((rr_data->cs_data.arfcn[i] & ARFCN_MASK) EQ
           (rr_data->found_channel     & ARFCN_MASK)) AND
           ((rr_data->cs_data.arfcn[i] & US_BIT) EQ
           (rr_data->found_channel     & US_BIT)) AND
           (rr_data->cs_data.attributes[i] & CS_CHECK_FLAG) EQ 0)
      {
        TRACE_EVENT ( "rr_found_channel in CS_DATA" );
        rr_data->found_channel = NOT_PRESENT_16BIT;
        x = i;
        break;
      }
    }
  }

  /*
   * for all channels of the power campaign list
   */
  if ( x EQ NOT_PRESENT_8BIT )
  {
    for ( i=0; i < rr_data->cs_data.max_arfcn; i++ )
    {
      if ((rr_data->cs_data.attributes[i] & CS_CHECK_FLAG) EQ 0)
      {
        /*
         * channel is not checked yet
         */
        if (x EQ NOT_PRESENT_8BIT)
        {
          /*
           * first channel which is not checked yet
           */
          x = i;

          /* High priority cell. Start sync */
          if((rr_data->cs_data.attributes[i] & CS_PRIORITY) EQ CS_HIGH_PRIORITY)
            break;
        }
        else
        {
          if ((rr_data->cs_data.attributes[i] & CS_PRIORITY) >
              (rr_data->cs_data.attributes[x] & CS_PRIORITY))
          {
            /*
             * new channel has higher priority
             */
            x = i;
          }
        }
      } /* CS_CHECK FLAG */
    } /* i < max_arfcn */
  } /* x NEQ NOT_PRESENT_8BIT */

  if (x NEQ NOT_PRESENT_8BIT)
  {
    rr_data->cs_data.act_index = x;

    if ((rr_data->ms_data.rr_service NEQ NO_SERVICE) AND
        (rr_data->cs_data.arfcn[x] EQ rr_data->nc_data[SC_INDEX].arfcn))
    {
      /* 
       * Synchronization request for serving cell. 
       * No need to send MPH_BSIC_REQ as we are already synchronized to 
       * serving cell, instead fake MPH_BSIC_CNF.
       */
      PALLOC (mph_bsic_cnf, MPH_BSIC_CNF); /* T_MPH_BSIC_CNF */
      mph_bsic_cnf->arfcn = rr_data->cs_data.arfcn[x];
      mph_bsic_cnf->bsic  = rr_data->nc_data[SC_INDEX].bsic;
      mph_bsic_cnf->cs    = CS_NO_ERROR;
      if (srv_store_prim ((T_PRIM *)D2P(mph_bsic_cnf)))
      {
        TRACE_EVENT ("Shortcut for sync to SC taken");
        USE_STORED_ENTRIES();
        return TRUE;
      }
      else
      {
        /* In the unlikely event that we could not store the primitive */
        PFREE (mph_bsic_cnf);
      }
    }
    else
    {

      /*
       *start BSIC Reading;For fast search, the first scan is only on reasonably strong carriers.i.e carrires with rxlev>MEDIUM_RXLEV_THRESHOLD
       *This is as per the cell selection improvements document. Hence dont send BSIC request on white listed low power carrires for first scan.
       *(OMAPS00109145)*/
      if( (rr_data->cs_data.previous_search_mode EQ FAST_SEARCH_MODE) AND (rr_data->cs_data.scan_mode NEQ CS_SECOND_SCAN) AND ( rr_data->cs_data.rxlev[x]<MEDIUM_RXLEV_THRESHOLD))
        return FALSE;
      else
      {
        PALLOC (mph_bsic_req, MPH_BSIC_REQ);

        mph_bsic_req->arfcn = rr_data->cs_data.arfcn[x];

        TRACE_EVENT_P3 ("BSIC REQ[%u]%u a=0x%x",
          rr_data->cs_data.arfcn[x] & ARFCN_MASK,
          x, rr_data->cs_data.attributes[x]);

        PSENDX (PL, mph_bsic_req);
      }
      return TRUE;
    }
  }
  else
    return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_set_null                |
+--------------------------------------------------------------------+

  PURPOSE : Initialize the cell selection process.

*/

GLOBAL void cs_set_null (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cs_set_null()");

  SET_STATE (STATE_CELL_SEL, CS_NULL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_set_stop_active         |
+--------------------------------------------------------------------+

  PURPOSE : The function resets the state of the cell selection
            process to an non-active state.

*/

GLOBAL void cs_set_stop_active (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cs_set_stop_active()");

  switch (GET_STATE(STATE_CELL_SEL))
  {
    case CS_NULL:
    case CS_NULL_ACTIVE:
      SET_STATE (STATE_CELL_SEL, CS_NULL);
      break;
    case CS_IDLE:
    case CS_IDLE_ACTIVE:
      SET_STATE (STATE_CELL_SEL, CS_IDLE);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_set_attributes          |
+--------------------------------------------------------------------+

  PURPOSE : The function sets an attribute for a channel (e.g.
            emergency or low priority cell).

*/

GLOBAL void cs_set_attributes (UBYTE attribute, USHORT arfcn)
{
  GET_INSTANCE_DATA;
  UBYTE i;

  TRACE_FUNCTION ("cs_set_attributes()");

  /*
   * for all channels of the power campaign list
   */
  for (i=0;i<rr_data->cs_data.max_arfcn;i++)
  {
    /*
     * if it is the given channel number, set the attributes.
     */
    if ((rr_data->cs_data.arfcn[i]&ARFCN_MASK) EQ arfcn)
      rr_data->cs_data.attributes [i] |= attribute;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_set_rxlev               |
+--------------------------------------------------------------------+

  PURPOSE : Updating of a fieldstrength value in the list of the
            power campaign.

*/

GLOBAL void cs_set_rxlev (UBYTE rxlev, USHORT arfcn)
{
  GET_INSTANCE_DATA;
  UBYTE i;

  TRACE_FUNCTION ("cs_set_rxlev()");

  /*
   * for all channels in the power campaign list
   */
  for (i=0;i<rr_data->cs_data.max_arfcn;i++)
  {
    /*
     * if it is the given channel number, update the fieldstrength.
     */
    if ((rr_data->cs_data.arfcn[i]&ARFCN_MASK) EQ arfcn)
      rr_data->cs_data.rxlev [i] = rxlev;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_clear_attributes        |
+--------------------------------------------------------------------+

  PURPOSE : The function clears an attribute for a channel (e.g.
            emergency or low priority cell).

*/

GLOBAL void cs_clear_attributes (UBYTE attribute, USHORT arfcn)
{
  GET_INSTANCE_DATA;
  UBYTE i;

  TRACE_FUNCTION ("cs_clear_attributes()");

  /*
   * for all channels in the power campaign list
   */
  for (i=0;i<rr_data->cs_data.max_arfcn;i++)
  {
    /*
     * if it is the given channel number, clear the attributes.
     */
    if ((rr_data->cs_data.arfcn[i]&ARFCN_MASK) EQ arfcn)
      rr_data->cs_data.attributes [i] &= ~attribute;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_mph_power_cnf           |
+--------------------------------------------------------------------+

  PURPOSE : The result of a power request to layer 1. Layer 1 returns
            a list of up to 40 GSM 900 + 40 DCS 1800 channels to RR.

*/

GLOBAL void cs_mph_power_cnf (T_MPH_POWER_CNF * mph_power_cnf)
{
  GET_INSTANCE_DATA;
  UBYTE   i,j,rxlev;
  USHORT  mask = 0xFFFF; /* no mask */
  U8  x;
  BOOL copy = TRUE;

  TRACE_FUNCTION ("cs_mph_power_cnf()");

  if(GET_STATE(STATE_ATT) EQ ATT_CS_INIT )
  {
    if( !srv_store_prim ((T_PRIM *)D2P(mph_power_cnf)) )
    {
      /*
       * primitive storing failed
       */
      PFREE (mph_power_cnf);
      SET_STATE(STATE_CELL_SEL, CS_NULL);
      SET_STATE(STATE_ATT, ATT_NULL);
    }
    else
    {
      /*
       * Boot Time: Do not store primitive for more than 10 s in case
       * of quick registration. If the second activate_req does not 
       * come before this time move to ATT_NULL. 
       * The T_RESELECT timer is used for a different purpose here. 
       */
      TIMERSTART(T_RESELECT, TRESELECT_VALUE);
    }
    return;
  }
  
  TRACE_EVENT_P1 ("mph_power_cnf: %u freq.", mph_power_cnf->num_of_chan);

  for (i = 0; i < mph_power_cnf->num_of_chan; i++)
  {
    TRACE_EVENT_P3("[%4u] std=%u rx_lev=%2u",
      mph_power_cnf->arfcn[i]&ARFCN_MASK,
      STD_GET_FROM_ARFCN(mph_power_cnf->arfcn[i]),
      mph_power_cnf->rx_lev[i]);
  }

  /* Remove the inactive carriers from Black List */
  if((CS_GET_CURRENT_SEARCH_MODE EQ BLACK_LIST_SEARCH_MODE) OR 
     (mph_power_cnf->num_of_chan))
  {
    /* In case of "Full", "Normal" and "Fast" search modes, remove 
     * inactive carriers from Black List, only when there is atleast
     * one strong carrier in that area 
     */
    cs_rem_inactive_carriers_from_bl(mph_power_cnf);
  }

  if(CS_GET_CURRENT_SEARCH_MODE NEQ BLACK_LIST_SEARCH_MODE)
  {
    /* Update previous search mode */
    CS_SET_PREVIOUS_SEARCH_MODE(CS_GET_CURRENT_SEARCH_MODE);

    switch (GET_STATE(STATE_CELL_SEL))
    {
      case CS_IDLE_ACTIVE:
        /* We will start parallel scan in 2 cases 
           1. After HPLMN timer expiry
           2. After TREG timer expiry
           In these both cases don't mask out std and US_BIT 
        */
#ifndef TI_PS_FF_QUAD_BAND_SUPPORT
          mask = ARFCN_MASK; /* for parallel search mask out 'std' and US_BIT. */
#endif
        /*lint -fallthrough*/
      case CS_NULL_ACTIVE:
        /*
         * copy channel list from layer 1, set the number
         * of entries in the list
         */
        rr_data->cs_data.max_arfcn = mph_power_cnf->num_of_chan;

        /*
         * for all channels coming from layer 1
         */
        for (i=0,j=0;i<mph_power_cnf->num_of_chan;i++)
        {
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
          x = cs_get_band_index(mph_power_cnf->arfcn[i]);
#else
          x = get_band_index(mph_power_cnf->arfcn[i]);
#endif
            
          /* for OMAPS00109145, in which the I-Sample is not camping on to the agilent ,once down link failure is
          occured on current cell and at power levels < -100dB, this is because in the fast search mode we are not
          considering the low rxlev carriers. So this modification is required in order to come back to the network
          even after a down link failure has happened and if another BCCH carrer of less power become active and available
          for the mobile to camp */

          rxlev = rr_data->dyn_config.lower_rxlev_thr[x];
          copy = TRUE;

          if((i >= mph_power_cnf->num_of_white_list_chan) AND
              (mph_power_cnf->rx_lev[i] < rxlev))
           
          {
             copy = FALSE;
          }
 
          /*
           * check against band restrictions
           */
          if (copy AND cs_check_channel ((USHORT)(mph_power_cnf->arfcn[i]&ARFCN_MASK)))
          {
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
            /* For parallel search mask serving cell */
            if ((GET_STATE(STATE_CELL_SEL) EQ CS_IDLE_ACTIVE) AND (rr_data->nc_data[SC_INDEX].arfcn EQ (mph_power_cnf->arfcn[i]&ARFCN_MASK)))
              rr_data->cs_data.arfcn[j] = mph_power_cnf->arfcn[i]&ARFCN_MASK;
            else
#endif
              rr_data->cs_data.arfcn[j] = mph_power_cnf->arfcn[i]&mask;
            rr_data->cs_data.rxlev[j] = mph_power_cnf->rx_lev[i];
            /*
             * set initial to middle priority
             */
            rr_data->cs_data.attributes [j++] = CS_MID_PRIORITY;
          }
          else
          {
            /*
             * a cell is not copied due to band restrictions
             */
            rr_data->cs_data.max_arfcn--;
          }
        }

        TRACE_EVENT_P2("No_chan > [%d]min_rxlev copied:%d",rxlev,rr_data->cs_data.max_arfcn); 

        /* Increase the priority of white list carriers */
        if(rr_data->cs_data.max_arfcn )
          cs_use_white_list_info(mph_power_cnf->num_of_white_list_chan);

        /*
         * add actual PLMN to the found list, if available
         */
        if (rr_data->ms_data.req_mm_service EQ FUNC_NET_SRCH_BY_MMI AND
            GET_STATE(STATE_ATT) EQ ATT_IDLE AND
	    (!rr_data->net_lost))
          cs_add_current_network_to_found_list ();

        /*
         * Select channel with the highest fieldstrength and priority
         * and start BSIC scanning, else send signal that no further
         * channel is available
         */
        if (cs_sync_next_bsic () EQ FALSE)
        {
          SET_STATE (STATE_CELL_SEL, CS_IDLE);
          TIMERSTOP(T_PLMN_SEARCH);
          att_no_bsic_ind ();
        }
        else
        {
          SET_STATE (STATE_CELL_SEL, CS_IDLE_ACTIVE);
        }

        /*
         * CS power measurements may be used to reorder arfcns'n in
         * MPH_NEIGHBOURCELL_REQ while TCSVALID is running
         */
        TIMERSTART(TCSVALID, THIRTY_SEC);
        break;
    }
  } /* NEQ black list search */
  else
  {
    
    /* Stop the parallel PLMN search */
    att_notify_stop_plmn_search(FALSE);

    SET_STATE (STATE_CELL_SEL, CS_IDLE);

    /* Black list search is a temporary search. It is not used 
     * in deciding the next search. Hence we need to update current search
     * mode when Black List search is over */
    CS_SET_CURRENT_SEARCH_MODE(CS_GET_PREVIOUS_SEARCH_MODE);


  } /* Black list search */

  PFREE (mph_power_cnf);
  srv_use_stored_prim ();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_mph_bsic_cnf            |
+--------------------------------------------------------------------+

  PURPOSE : The response of a FB/SB synchronisation request from layer 1.

*/

GLOBAL void cs_mph_bsic_cnf (T_MPH_BSIC_CNF * mph_bsic_cnf)
{
  GET_INSTANCE_DATA;
  U8 local_region;
  U16 local_arfcn;

  TRACE_FUNCTION ("cs_mph_bsic_cnf()");

  switch (GET_STATE (STATE_CELL_SEL))
  {    
    case CS_IDLE_ACTIVE:
      if (mph_bsic_cnf->cs EQ CS_NO_ERROR)
      {
        UBYTE new_std    = STD_GET_FROM_ARFCN(mph_bsic_cnf->arfcn);
        /*
         * FB / SB is decoded, cell has the attributes
         * BCCH detected and checked
         */
        /* 
         * reset std from the current MPH_BSIC_CNF(only if it is non-zero)
         */
        TRACE_EVENT_P2("reset std: new_std = %d, old_std = %d", new_std, std);
        if(new_std)
        {
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
          UBYTE new_region;

          std = cs_update_std_ffs (new_std);
          new_region = srv_get_region_from_std(std);
          TRACE_EVENT_P1("update std: new_std = %d", std);
#else
          UBYTE new_region = srv_get_region_from_std(new_std);

          std = new_std;
#endif

          /* Both American and European bands are detected in the current
           * region. Following TREG timer expiry in Limited Service state,
           * non-parallel search will be issued
           * CSI-LLD section:4.3
           */
          if(rr_data->cs_data.region NEQ NOT_PRESENT_8BIT)
          {
            if(rr_data->cs_data.region NEQ new_region)
            {
              rr_data->cs_data.all_freq_area = TRUE;
              TRACE_EVENT("all_freq_area : 1");
            }
          }

          /* CSI-LLD section:4.1.1.3.12
           * This stores region in rr_data base by extracting from std 
           */
          rr_data->cs_data.region  = new_region;
        }
        /*
         * remove co channels
         */
        if (test_house EQ FALSE)
          cs_remove_co_channels ((USHORT)(mph_bsic_cnf->arfcn&ARFCN_MASK));

        rr_data->cs_data.attributes [rr_data->cs_data.act_index] |= 
          (CS_CHECK_FLAG | CS_BCCH_READ);

        /* CSI-LLD section:4.1.1.11 
         * Synchronisation succeeded. Remove the carrier from black list
         */
        cs_del_from_black_list(rr_data->cs_data.region,
                               (U16)(mph_bsic_cnf->arfcn&ARFCN_MASK)); 

        att_bsic_ind ((USHORT)(mph_bsic_cnf->arfcn&ARFCN_MASK),
                          rr_data->cs_data.rxlev [rr_data->cs_data.act_index],
                          mph_bsic_cnf->bsic);
      }
      else
      {
        /*
         * FB / SB is not decoded, cell has the attributes
         * BCCH not detected and checked
         */
        rr_data->cs_data.attributes [rr_data->cs_data.act_index] |= CS_CHECK_FLAG;

        /* Extract region information */        
       
        local_arfcn = rr_data->cs_data.arfcn[rr_data->cs_data.act_index];

        if(STD_GET_FROM_ARFCN(local_arfcn) NEQ STD_NOT_PRESENT)
        {
          /* During Non-Parallel search, region information is available 
           * in ARFCN 
           */
          local_region = CS_GET_REGION_FROM_FREQ(local_arfcn);
        }
        else
        {
          /* Parallel search case */
          local_region =  srv_get_region_from_std(std);
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
          if (local_region EQ BOTH_REGIONS)
            local_region = srv_get_region_from_std_arfcn (std, local_arfcn);
#endif
        }

        /* CSI-LLD section:4.1.1.6 
         * Sync failed. Add the carrier to Black list
         */
        cs_add_to_black_list(local_region,
            (U16)(local_arfcn),
            rr_data->cs_data.rxlev[rr_data->cs_data.act_index]);
 
        /* CSI-LLD section:4.1.1.11  
         * Set the BLACK LIST attribute flag for this carrier 
         */
        if(rr_data->cs_data.initial_plmn_search EQ INITIAL_PLMN_SEARCH_ACTIVE)
        { 
          CS_SET_BLACK_LIST_FLAG(rr_data->cs_data.act_index);
        }

        if (cs_sync_next_bsic () EQ FALSE)
        {
          TIMERSTOP(T_PLMN_SEARCH);
          SET_STATE (STATE_CELL_SEL, CS_IDLE);
          att_no_bsic_ind ();  /* stop in RR */
        }
      }
      break;

#ifdef GPRS
    case CS_CCO:
      SET_STATE (STATE_CELL_SEL, CS_IDLE);
      if (rr_data->gprs_data.cr_pbcch_active OR rr_data->gprs_data.cr_pcco_active)
      {
        /*
         * a Packet Cell Change Order for a cell which
         * is not in the BA list has been requested by the NW
         */
        if (mph_bsic_cnf->cs    EQ CS_NO_ERROR              AND
            (mph_bsic_cnf->arfcn&ARFCN_MASK) EQ rr_data->gprs_data.arfcn AND
            mph_bsic_cnf->bsic  EQ rr_data->gprs_data.bsic    )
        {
          /*
           * PL successfuly sycnhronized to the cell
           * now RR will do a normal Cell Reselection
           */
          memset (&rr_data->nc_data[CR_INDEX],
                  0,
                  sizeof (T_NC_DATA));
          rr_data->nc_data[CR_INDEX].arfcn = mph_bsic_cnf->arfcn&ARFCN_MASK;
          rr_data->nc_data[CR_INDEX].bsic  = mph_bsic_cnf->bsic;
          
          /* CSI-LLD section:4.1.1.11 
           * Synchronisation succeeded. Remove the carrier from black list
           */
          cs_del_from_black_list(rr_data->cs_data.region,
                                 (U16)(mph_bsic_cnf->arfcn&ARFCN_MASK)); 

          if( rr_data->gprs_data.cr_pbcch_active )
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
        }
        else
        {
          /*
           * PL did not sycnhronize to the cell
           * inform GRR and wait for a CR_REQ(CR_REQ) with
           * the old cell and return to it
           */
          PALLOC(gprs_ind, RRGRR_GPRS_SI13_IND);
          rr_data->gprs_data.cr_pbcch_active = FALSE;

          gprs_ind->cause = GPRS_CELL_NOT_SUITABLE;
          gprs_ind->serving_cell_info.bcch_arfcn = rr_data->gprs_data.arfcn;
          gprs_ind->serving_cell_info.bcch_bsic  = rr_data->gprs_data.bsic;

          TRACE_EVENT_P4 ("cs=%u C[%d] bsic=%u #%u",
            gprs_ind->cause,
            gprs_ind->serving_cell_info.bcch_arfcn,
            gprs_ind->serving_cell_info.bcch_bsic,
            __LINE__);

          PSENDX(GRR, gprs_ind);
          /* wait for CR_REQ for old cell */
        }
      }
      else
        if (mph_bsic_cnf->cs    EQ CS_NO_ERROR              AND
            (mph_bsic_cnf->arfcn&ARFCN_MASK) EQ rr_data->gprs_data.arfcn AND
            mph_bsic_cnf->bsic  EQ rr_data->gprs_data.bsic    )
        {
          rr_data->gprs_data.tbf_est = TBF_EST_CCO;

          rr_data->nc_data[CR_INDEX].arfcn = rr_data->gprs_data.arfcn;
          rr_data->nc_data[CR_INDEX].rxlev = 0;
          rr_data->nc_data[CR_INDEX].bsic  = rr_data->gprs_data.bsic;

          att_init_cr_data();
          TIMERSTART (T_RESELECT, TRESELECT_VALUE);
          rr_data->nc_data[CR_INDEX].bcch_error = 0;
          srv_clear_list (&rr_data->cr_data.cd.ncell_list);
          SET_STATE (STATE_ATT, ATT_CS2);

          /* CSI-LLD section:4.1.1.11  
           * Synchronisation succeeded. Remove the carrier from black list
           */
          cs_del_from_black_list(rr_data->cs_data.region,
                                 (U16)(mph_bsic_cnf->arfcn&ARFCN_MASK));  
          /* wait for System Information Messages */
        }
        else /* initiate switch back to old cell */
        {
          dat_code_mph_old_chan_req();

          PFREE ( rr_data->gprs_data.dl_data_ind );
          rr_data->gprs_data.dl_data_ind = NULL;

          rr_data->gprs_data.reconn_cause = RRC_PROT_UNSPECIFIED;
          rr_data->gprs_data.cco_need_reconnect_cnf = FALSE;
          SET_STATE (STATE_DAT, DAT_CCO_3);
        }
      break;

    case CS_XMEAS:

      /* enter result in rr_data->gprs_data.rrgrr_ext_meas_cnf, call att_ext_meas_next_bsic */

      {
        UBYTE                 bsic                = mph_bsic_cnf->bsic;
        T_GPRS_DATA          *gprs_data           = &rr_data->gprs_data;
        T_RRGRR_EXT_MEAS_REQ *rrgrr_ext_meas_req  =  gprs_data->rrgrr_ext_meas_req;


        if (  rrgrr_ext_meas_req->report_type   EQ REP_TYPE_1 OR               /* "6 strongest carriers independent of BSIC decoding or not" */
              rrgrr_ext_meas_req->report_type   EQ REP_TYPE_2 AND               /* "BSICs for 6 carriers with decoded BSIC with allowed NCC" */
              mph_bsic_cnf->cs EQ CS_NO_ERROR AND
             (rrgrr_ext_meas_req->ncc_permitted & (1 << ((bsic>>3) & 0x07))))  /* check for "NCC allowed" */
        {
          T_RRGRR_EXT_MEAS_CNF *rrgrr_ext_meas_cnf  =  gprs_data->rrgrr_ext_meas_cnf;
          T_MPH_EXT_MEAS_CNF   *mph_ext_meas_cnf    =  gprs_data->mph_ext_meas_cnf;
          UBYTE                 n                   =  gprs_data->mph_ext_meas_num;
          USHORT                arfcn               =  mph_ext_meas_cnf->arfcn[n];
          UBYTE                 rx_lev              =  mph_ext_meas_cnf->rx_lev[n];
          UBYTE                 idx                 =  rr_ext_meas_idx ( arfcn );
          UBYTE                 k                   =  rrgrr_ext_meas_cnf->c_xmeas_res++;
          T_xmeas_res          *xmeas_res           = &rrgrr_ext_meas_cnf->  xmeas_res[k];

          /*
           * Function att_ext_meas_next_bsic checks for the maximum number of elements in the
           * array rrgrr_ext_meas_cnf->xmeas_res. Therefore this check is not performed here.
           */

          xmeas_res->arfcn_idx.arfcn = arfcn;
          xmeas_res->arfcn_idx.idx   = idx;
          xmeas_res->rxlev           = rx_lev;
          if (mph_bsic_cnf->cs EQ CS_NO_ERROR)
            xmeas_res->bsic          = bsic;
          else
            xmeas_res->bsic          = RRGRR_INVALID_BSIC;
        }
      }

      rr_data->gprs_data.mph_ext_meas_num++;
      att_ext_meas_next_bsic();
      break;
#endif
    default:
      break;
  }
  PFREE (mph_bsic_cnf);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)   MODULE  : RR_CS                          |
| STATE   : code            ROUTINE : cs_get_channel_from_found_list |
+--------------------------------------------------------------------+

  PURPOSE : If a network available list was created in former times
            (found_plmn / found_channel) and one of this networks is
            requested, the associated channel is stored to give it
            high priority to speed up cell selection.

*/
GLOBAL void cs_get_channel_from_found_list ()
{
  GET_INSTANCE_DATA;
  UBYTE i;
  T_FOUND_ELEMENT * found;

  TRACE_FUNCTION ("cs_get_channel_from_found_list()");

  /*
   * initialise the local variable
   */
  rr_data->found_channel = NOT_PRESENT_16BIT;

  /*
   * for all PLMNs stored in the found list
   */
  found = &rr_data->sc_data.found[0];
  for (i=0; i<rr_data->sc_data.found_entries; i++, found++)
  {
    /*
     * if the network in the found list is equal to the PLMN requested by MM,
     * set the found channel
     */
/* Implements Measure#32: Row 124 */
    att_print_mcc_mnc(found->arfcn, found->plmn.mcc, found->plmn.mnc, S2I_STRING("list"));
    if (dat_plmn_equal_req (found->plmn.mcc,
                            found->plmn.mnc,
                            rr_data->ms_data.plmn.mcc,
                            rr_data->ms_data.plmn.mnc))
    {
      rr_data->found_channel = found->arfcn;
       
      /* Embed region information in the found channel arfcn */
      if(found->region EQ AMERICAN_REGION)
        rr_data->found_channel |= US_BIT;
/* Implements Measure#32: Row 125 */
      att_print_mcc_mnc(found->arfcn, found->plmn.mcc, found->plmn.mnc, S2I_STRING("found_channel"));
      return;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147) MODULE  : RR_CS                            |
| STATE   : code          ROUTINE : cs_clear_channel_from_found_list |
+--------------------------------------------------------------------+

  PURPOSE : If a network available list was created in former times
            (found_plmn / rr_found_channel) and one of this networks is
            requested, the associated channel is stored to give it
            high priority to speed up cell selection. This function
            clear the stored data if a network or limited search is
            started.

*/

GLOBAL void cs_clear_channel_from_found_list (void)
{
  GET_INSTANCE_DATA;
  /*
   * initialize the found channel number
   */
  rr_data->found_channel = NOT_PRESENT_16BIT;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_remove_co_channels      |
+--------------------------------------------------------------------+

  PURPOSE : If a channel x fails and the channels x-1 and x+1 have
            a fieldstrength difference greater 9 dB, it is not necessary
            to try on this cells. Sense of this is to speed up search.

*/

static void cs_remove_co_channels (USHORT arfcn)
{
  /*
   * depending on the channel number. Some extra cases
   * for channel numbers on the boarders of the frequency Band
   */
  switch (arfcn)
  {
    case CHANNEL_0:
      /*
       * E-GSM channel 0 has no co-channels
       */
      break;

    /*
     * all LOW channel boarders
     */
    case LOW_CHANNEL_850:
    case LOW_CHANNEL_900:
    case LOW_CHANNEL_1800:
    case LOW_CHANNEL_EGSM:
      cs_remove_co_chan (arfcn, (USHORT)(arfcn+1));
      break;

    /*
     * all HIGH channel boarders
     */
    case HIGH_CHANNEL_1900:
      if (std NEQ STD_1900 AND std NEQ STD_DUAL_US)
      {
        cs_remove_co_chan (arfcn, (USHORT)(arfcn+1));/* DCS 1800 */
      }
      /*lint -fallthrough*/
    case HIGH_CHANNEL_850:
    case HIGH_CHANNEL_900:
    case HIGH_CHANNEL_1800:
    case HIGH_CHANNEL_EGSM-1:
      cs_remove_co_chan (arfcn, (USHORT)(arfcn-1));
      break;

    default:
      /*
       * all other channels inside the frequency bands
       */
      cs_remove_co_chan (arfcn, (USHORT)(arfcn+1));
      cs_remove_co_chan (arfcn, (USHORT)(arfcn-1));
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_remove_co_chan          |
+--------------------------------------------------------------------+

  PURPOSE : if co_chan - chan has a fieldstrength difference greater
            9 dBm, remove the channel.

*/

static void cs_remove_co_chan (USHORT arfcn, USHORT co_arfcn)
{
  GET_INSTANCE_DATA;
  UBYTE index_arfcn    = NOT_PRESENT_8BIT;
  UBYTE index_co_arfcn = NOT_PRESENT_8BIT;
  UBYTE i;
  SHORT rxlev_arfcn;
  SHORT rxlev_co_arfcn;
  USHORT *p_arfcn;
  TRACE_FUNCTION ("cs_remove_co_chan()");

  /*
   * calculate the index in the power campaign list
   * for the channel
   */
  p_arfcn = rr_data->cs_data.arfcn;
  for (i=0; i<rr_data->cs_data.max_arfcn; i++)
  {
    if (arfcn EQ (p_arfcn[i]&ARFCN_MASK))
    {
      index_arfcn = i;
      break;
    }
  }
  /*
   * if the channel is not inside the list,
   * then nothing is to compare else get the
   * fieldstrength.
   */
  if (index_arfcn EQ NOT_PRESENT_8BIT)
    return;

  /*
   * calculate the index in the power campaign list
   * for the co-channel
   */
  for (i=0;i<rr_data->cs_data.max_arfcn;i++)
  {
    if (co_arfcn EQ (p_arfcn[i]&ARFCN_MASK))
    {
      index_co_arfcn = i;
      break;
    }
  }
  /*
   * if the co-channel is not inside the list,
   * then nothing is to compare else get the
   * fieldstrength.
   */
  if (index_co_arfcn EQ NOT_PRESENT_8BIT)
    return;

  rxlev_arfcn = (SHORT)rr_data->cs_data.rxlev [index_arfcn];
  rxlev_co_arfcn = (SHORT)rr_data->cs_data.rxlev [index_co_arfcn];

  /*
   * if the difference is > 9 dBm,
   * set to low priority.
   */
  if ((rxlev_arfcn - rxlev_co_arfcn) > 9)
  {
    rr_data->cs_data.attributes [index_co_arfcn] &= CS_SET_TO_LOW_PRIORITY;
  }
}

/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6147) MODULE  : RR_CS                             |
| STATE   : code          ROUTINE : cs_add_current_network ....       |
+---------------------------------------------------------------------+

  PURPOSE : Adds the current PLMN to the found list if available.

*/
LOCAL void cs_add_current_network_to_found_list (void)
{
  GET_INSTANCE_DATA;
  T_loc_area_ident *lai = &rr_data->nc_data[SC_INDEX].lai;

  TRACE_FUNCTION ("cs_add_current_network_to_found_list()");

  switch (rr_data->ms_data.rr_service)
  {
    case LIMITED_SERVICE:
    case FULL_SERVICE:
      /*
       * The mobile was in full service
       * Add this PLMN to the list of found PLMN
       * and clean power measurement list.
       */
      if (! att_plmn_in_found_list (lai->mcc, lai->mnc) AND
          rr_data->sc_data.found_entries < MAX_PLMN)
      {
        att_save_found_plmn (
          &rr_data->sc_data.found[rr_data->sc_data.found_entries],
          lai->mcc, lai->mnc,
          rr_data->nc_data[SC_INDEX].arfcn,
          rr_data->nc_data[SC_INDEX].rxlev, lai->lac, 0);
        rr_data->sc_data.found_entries++;
      }
      /*
       * Consider only high priority cells to speed up search
       */
      cs_del_list (&rr_data->sc_data.cd.ncell_list);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_check_channel           |
+--------------------------------------------------------------------+

  PURPOSE : Depending on the settings (default, GSM 900, DCS 1800)
            all non-interesting channels are removed.

*/
static UBYTE cs_check_channel (USHORT arfcn)
{
  GET_INSTANCE_DATA;
  /*
   *  TRACE_FUNCTION ("cs_check_channel")
   */
  {
    switch (rr_data->dyn_config.set_band)
    {
      case 0:
        /*
         * default behaviour
         */
        return TRUE;

      case 1:
        /*
         * set to GSM 900
         */
/*
        if (arfcn <= LOW_CHANNEL_900 OR
            arfcn >= HIGH_CHANNEL_900)
*/
        if (INRANGE(LOW_CHANNEL_900,arfcn,HIGH_CHANNEL_900))
          return FALSE;
        else
          return TRUE;

      case 2:
        /*
         * set to DCS 1800
         */
/*
        if (arfcn >= LOW_CHANNEL_1800 AND
            arfcn <= HIGH_CHANNEL_1800)
*/
        if (INRANGE(LOW_CHANNEL_1800,arfcn,HIGH_CHANNEL_1800))
          return TRUE;
        else
          return FALSE;
    }
  }
  return TRUE;
}

/*
+----------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : RR_CS                        |
| STATE   : code                ROUTINE : cs_check_region              |
+----------------------------------------------------------------------+

  PURPOSE : This routine validates the region
            Cell Selection Improvements-LLD section:4.1.1.2
*/

GLOBAL BOOL cs_check_region(U8 region)
{
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  if((region EQ EUROPEAN_REGION) OR (region EQ AMERICAN_REGION) OR (region EQ BOTH_REGIONS))
#else
  if((region EQ EUROPEAN_REGION) OR (region EQ AMERICAN_REGION))
#endif
  {
    return TRUE;
  }
  else
  {
    TRACE_ERROR("Invalid region");
    return FALSE;
  }
}

/*
+----------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : RR_CS                        |
| STATE   : code                ROUTINE : cs_validate_arfcn_index      |
+----------------------------------------------------------------------+

  PURPOSE : This routine validates the ARFCN index
            Cell Selection Improvements-LLD section:4.1.1.2
*/

GLOBAL BOOL cs_check_arfcn_range(U16 arfcn)
{
  if(arfcn EQ CHANNEL_0)
    arfcn = CHANNEL_0_INTERNAL;

  if((arfcn > CHANNEL_0) AND (arfcn <= CHANNEL_0_INTERNAL))
  {
    return TRUE;
  }
  else
  {
    TRACE_ERROR("Invalid ARFCN");
    return FALSE;
  }
}

/*
+----------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : RR_CS                        |
| STATE   : code                ROUTINE : cs_inc_sync_fail_counter     |
+----------------------------------------------------------------------+

  PURPOSE : This routine increments the sync fail counter value for GSM
            carrier. Seperate counters are maintained for European and 
            American regions.The size of SFC counter is 4 bits. As a 
            result two carriers are accommodated in one byte. This function
            first converts the ARFCN range from 1-1023 to 0-511 format and
            increments the SFC accordingly. The SFC format is shown below

            CSI-LLD section:4.1.1.5.9

           ------------------------------------------
           | Index |	MSB 4 bits	 | LSB 4 bits   |
           ------------------------------------------
           |   0   |	ARFCN : 2	 |  ARFCN : 1   |
           ------------------------------------------
           |   1   |	ARFCN : 4	 |  ARFCN   3   |
           ------------------------------------------
           |   -   |        -        |      -       |
           ------------------------------------------
           |   -   |        -        |      -       |
           ------------------------------------------ 
           |  510  |	ARFCN : 1022 |  ARFCN: 1021 |
           ------------------------------------------
           |  511  |	ARFCN : 0	 |  ARFCN :1023 |
            -----------------------------------------
           ARFCN: 0 = CHANNEL_0_INTERNAL

*/

LOCAL void cs_inc_sync_fail_counter(U8 region, U16 arfcn)
{
  GET_INSTANCE_DATA;
  U16 index=0;
  U8  inc = 0;

  if(!cs_check_region(region))
    return;

  if(!cs_check_arfcn_range((U16)(arfcn & ARFCN_MASK)))
    return;

  if(arfcn EQ CHANNEL_0)
    arfcn = CHANNEL_0_INTERNAL;
 
  /* Convert ARFCN range to 0-511 from 1-1023 range */
  index = ((arfcn & ARFCN_MASK) -1) >> 1;

  if(arfcn & 0x01)
  {
    /* Increment LSB 4 bits for odd ARFCN */
    inc = 0x01;
  }
  else
  {
    /* Increment MSB 4 bits for even ARFCN */
    inc = 0x10;
  }

  /* Increment the synchronisation counter */
  rr_data->cs_data.black_list.sfc[region][index] += inc;

  TRACE_EVENT_P3("[%d]Reg,[%d]Arfcn, SFC = 0x%x",
                  region,
                  arfcn,
                  rr_data->cs_data.black_list.sfc[region][index]);
}

/*
+----------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : RR_CS                        |
| STATE   : code                ROUTINE : cs_reset_sync_fail_counter   |
+----------------------------------------------------------------------+

  PURPOSE : This routine resets the synchronisation fail counter value of
            GSM carrier.
            Seperate counters are maintained for European and American 
            regions.The size of SFC counter is 4 bits. As a result two 
            carriers are accommodated in one byte. This function
            first converts the ARFCN range from 1-1023 to 0-511 format and
            increments the SFC accordingly.The SFC format is shown below

            CSI-LLD section:4.1.1.5.10

           ------------------------------------------
           | Index |	MSB 4 bits	 | LSB 4 bits   |
           ------------------------------------------
           |   0   |	ARFCN : 2	 |  ARFCN : 1   |
           ------------------------------------------
           |   1   |	ARFCN : 4	 |  ARFCN   3   |
           ------------------------------------------
           |   -   |        -        |      -       |
           ------------------------------------------
           |   -   |        -        |      -       |
           ------------------------------------------ 
           |  510  |	ARFCN : 1022 |  ARFCN: 1021 |
           ------------------------------------------
           |  511  |	ARFCN : 0	 |  ARFCN :1023 |
            -----------------------------------------
           ARFCN: 0 = CHANNEL_0_INTERNAL
*/

LOCAL void cs_reset_sync_fail_counter(U8 region, U16 arfcn)
{
  GET_INSTANCE_DATA;
  USHORT index=0;

  if(!cs_check_region(region))
    return;

  if(!cs_check_arfcn_range((U16)(arfcn & ARFCN_MASK)))
    return;

  if(arfcn EQ CHANNEL_0)
    arfcn = CHANNEL_0_INTERNAL;

  /* Convert ARFCN range to 0-511 from 1-1023 range */
  index = ((arfcn & ARFCN_MASK) -1) >> 1;

  if(arfcn & 0x01)
  {
    /* Reset LSB 4 bits for odd ARFCN */
    rr_data->cs_data.black_list.sfc[region][index] &=0xf0;
  }
  else
  {
    /* Reset MSB 4 bits for even ARFCN */
    rr_data->cs_data.black_list.sfc[region][index] &=0x0f;
  }        

  TRACE_EVENT_P2("SFC reset for [%d]Reg,[%d]Arfcn",region,arfcn);
}

/*
+----------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : RR_CS                        |
| STATE   : code                ROUTINE : cs_get_sync_fail_counter     |
+----------------------------------------------------------------------+

  PURPOSE : This function returns the SFC counter of  GSM carriers
            Seperate counters are maintained for European and American 
            regions.The size of SFC counter is 4 bits. As a result two 
            carriers are accommodated in one byte. This function
            first converts the ARFCN range from 1-1023 to 0-511 format and
            increments the SFC accordingly.The SFC format is shown below

            CSI-LLD section:4.1.1.5.11

           ------------------------------------------
           | Index |	MSB 4 bits	 | LSB 4 bits   |
           ------------------------------------------
           |   0   |	ARFCN : 2	 |  ARFCN : 1   |
           ------------------------------------------
           |   1   |	ARFCN : 4	 |  ARFCN   3   |
           ------------------------------------------
           |   -   |        -        |      -       |
           ------------------------------------------
           |   -   |        -        |      -       |
           ------------------------------------------ 
           |  510  |	ARFCN : 1022 |  ARFCN: 1021 |
           ------------------------------------------
           |  511  |	ARFCN : 0	 |  ARFCN :1023 |
            -----------------------------------------
           ARFCN: 0 = CHANNEL_0_INTERNAL
*/

LOCAL U8 cs_get_sync_fail_counter(U8 region, U16 arfcn)
{
  GET_INSTANCE_DATA;
  USHORT index=0;
  UBYTE  sfc_count=0;

  if(!cs_check_region(region))
    return sfc_count;

  if(!cs_check_arfcn_range((U16)(arfcn & ARFCN_MASK)))
    return sfc_count;

  if(arfcn EQ CHANNEL_0)
    arfcn = CHANNEL_0_INTERNAL;

  /* Convert to 0-511 range from 1-1024 range */
  index = ((arfcn & ARFCN_MASK) -1) >> 1;
  
  if(arfcn & 0x01)
  {
    /* obtain LSB 4 bits for odd ARFCN */
    sfc_count = (rr_data->cs_data.black_list.sfc[region][index] & 0x0f);
  }
  else
  {
    /* obtain MSB 4 bits for even ARFCN */
    sfc_count = (rr_data->cs_data.black_list.sfc[region][index] & 0xf0) >> 4;
  }      

  return sfc_count;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_add_to_black_list       |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to add GSM channels to "Black List". 
            The function checks the "Black List" criteria before adding 
            it to the list. This function is called whenever MS fails 
            to synchronize to a GSM channel.
            CSI-LLD section:4.1.1.5.5
*/

GLOBAL void cs_add_to_black_list(U8 region, U16 arfcn, U8 rxlev)
{
  GET_INSTANCE_DATA;
  U16 local_arfcn = arfcn & ARFCN_MASK;

  TRACE_FUNCTION("cs_add_to_black_list()");

  /* "Region" and "arfcn" are used to index "Black List" database
   *  Check whether they are within proper range
   */
  if(!cs_check_region(region))
    return;

  if(!cs_check_arfcn_range(local_arfcn))
    return;
   
  /* Check Black List criteria */
  if(cs_check_black_list_criteria(region,arfcn,rxlev))
  {
    /* Add the channel to Black List */
    srv_set_channel(&rr_data->cs_data.black_list.list[region],local_arfcn);

    /* Reset the sync fail counter for this channel. So that when this 
     * carrier is removed from BL(BA_MA list), counter will start from zero
     * again
     */
    cs_reset_sync_fail_counter(region, local_arfcn);  

    TRACE_EVENT_P2("[%d]Reg[%d]Arfcn added to BL",region,local_arfcn);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_del_from_black_list     |
+--------------------------------------------------------------------+

  PURPOSE : This routine removes the arfcn from black list and resets the sync 
            fail counter for that arfcn
            CSI-LLD section:4.1.1.5.6  
*/

GLOBAL void cs_del_from_black_list(U8 region, U16 arfcn)
{
  GET_INSTANCE_DATA;
  USHORT local_arfcn = arfcn & ARFCN_MASK;
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  UBYTE i;
#endif

  TRACE_FUNCTION("cs_del_from_black_list()");

  /* "Region" and "arfcn" are used to index "Black List" database
   *  Check whether they are within proper range
   */
  if(!cs_check_region(region))
    return;

  if(!cs_check_arfcn_range(local_arfcn))
    return;
 
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  if (region EQ BOTH_REGIONS)
  {
    for (i=0; i<MAX_REGIONS; i++)
    {
      if(srv_get_channel(&rr_data->cs_data.black_list.list[i],local_arfcn))
      {
        /* Remove the channel from Black List */
        srv_unset_channel(&rr_data->cs_data.black_list.list[i],local_arfcn);
  
        /* Reset the sync fail counter for this channel */
        cs_reset_sync_fail_counter(i, local_arfcn);   

        TRACE_EVENT_P2("ARFCN:%d Region:%d deleted from BL",local_arfcn,i);
      }
    }
  }
  else
  {
#endif
  if(srv_get_channel(&rr_data->cs_data.black_list.list[region],local_arfcn))
  {
    /* Remove the channel from Black List */
    srv_unset_channel(&rr_data->cs_data.black_list.list[region],local_arfcn);
  
    /* Reset the sync fail counter for this channel */
    cs_reset_sync_fail_counter(region, local_arfcn);   

    TRACE_EVENT_P2("ARFCN:%d Region:%d deleted from BL",local_arfcn,region);
  }
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  }
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_clear_black_list        |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to clear "Black List" database. 
            The function clears the "Black List" database from 
            1) Only RAM 
            2) Only FFS 
            3) Both RAM and FFS.
          
            This function is called in the following cases
            1) In response to "ERASE_BL" dynamic configuration command 
            2) After Initial PLMN search based on its outcome

            CSI-LLD section:4.1.1.5.1
*/

GLOBAL void cs_clear_black_list(U8 erase_type)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cs_clear_black_list()");

  /* clear Black List from RAM */
  if(erase_type & CLR_BLACK_LIST_RAM)
  {
    memset(&rr_data->cs_data.black_list,0,sizeof(T_CS_BLACK_LIST));
    
    TRACE_EVENT ("Black List: RAM cleared");
  }

  /* Clear Black List from FFS */
#if defined(_SIMULATION_FFS_)   
  if(erase_type & CLR_BLACK_LIST_FFS)
  {
    T_LIST local_black_list[MAX_REGIONS];
 
    memset(&local_black_list[0],0,MAX_REGIONS*sizeof(T_LIST));

    rr_csf_write_black_list(&local_black_list[0]);

    TRACE_EVENT ("Black List: FFS cleared");
  } 
#endif

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)   MODULE  : RR_CS                          |
| STATE   : code            ROUTINE : cs_remove_BA_MA_from_black_list|
+--------------------------------------------------------------------+

  PURPOSE : This function is used to remove the GSM channels  present
            in MA and BA lists from the "Black List". The function 
            deletes these channels from  the "Black List" and also 
            resets their SFC counter to zero.  This function is called 
            whenever MS receives  BA and MA list information in any RR 
            message.
            CSI-LLD section:4.1.1.5.7
*/

GLOBAL void cs_remove_BA_MA_from_black_list(U8 region, T_LIST *source_list)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION("cs_remove_BA_MA_from_black_list()");

  /* "Region" is to index "Black List" database
   *  Check whether it is within proper range
   */
  if(!cs_check_region(region))
    return;

  if(source_list NEQ NULL)
  {
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
    if (region EQ BOTH_REGIONS)
    {
      cs_remove_BA_MA_from_black_list(EUROPEAN_REGION,source_list);
      cs_remove_BA_MA_from_black_list(AMERICAN_REGION,source_list);
    }
    else
#endif
    srv_unmask_list(&rr_data->cs_data.black_list.list[region],source_list);
    
    TRACE_EVENT("BL updated with BA/MA/Inactive carrier list");
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)  MODULE  : RR_CS                           |
| STATE   : code           ROUTINE : cs_rem_inactive_carriers_from_bl|
+--------------------------------------------------------------------+

  PURPOSE : This function is used to remove the inactive carriers from
            the "Black List". This is done whenever the mobile changes 
            LA/RA 
            Cell Selection Improvements-LLD section:4.1.1.3.4
*/

GLOBAL void cs_rem_inactive_carriers_from_bl(T_MPH_POWER_CNF *mph_power_cnf)
{

  TRACE_FUNCTION("cs_remove_inactive_from_black_list");

  /* Remove the inactive carriers from Black list for European region */
  cs_remove_BA_MA_from_black_list(EUROPEAN_REGION,
            (T_LIST *)&mph_power_cnf->inactive_carrier_list.list[EUROPEAN_REGION]);

  /* Remove the inactive carriers from Black list for American region */
  cs_remove_BA_MA_from_black_list(AMERICAN_REGION,
            (T_LIST *)&mph_power_cnf->inactive_carrier_list.list[AMERICAN_REGION]);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_is_in_black_list        |
+--------------------------------------------------------------------+

  PURPOSE : This routine checks whether a carrier is already part of Black
            list or not
            CSI-LLD section:4.1.1.5.12
*/

LOCAL BOOL cs_is_in_black_list(U8 region, U16 arfcn)
{
  GET_INSTANCE_DATA;
  U8 status;

  status = srv_get_channel((T_LIST*)&rr_data->cs_data.black_list.list[region],
                                    (USHORT)arfcn & ARFCN_MASK); 

  TRACE_EVENT_P3("[%d]Reg[%d]Arfcn, BL status:%d",region,arfcn,status);

  return status;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : RR_CS                      |
| STATE   : code                ROUTINE : cs_is_in_white_list        |
+--------------------------------------------------------------------+

  PURPOSE : This routine checks whether a carrier is part of White
            list or not
            CSI-LLD section:4.1.2.2.7
*/

LOCAL BOOL cs_is_in_white_list(U8 region, U16 arfcn)
{
  GET_INSTANCE_DATA;
  U8 status = FALSE;

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  if((rr_data->cs_data.white_list.region EQ BOTH_REGIONS) OR 
     (region EQ rr_data->cs_data.white_list.region)) 
#else
  if(region EQ rr_data->cs_data.white_list.region) 
#endif
  {
    status = srv_get_channel(&rr_data->cs_data.white_list.list, 
                             ((U16)arfcn & ARFCN_MASK));
  }

  TRACE_EVENT_P3("[%d]Reg[%d]Arfcn, WL status:%d", region,arfcn,status);

  return status;
}

/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : RR_CS                       |
| STATE   : code                ROUTINE : cs_check_black_list_criteria|
+---------------------------------------------------------------------+

  PURPOSE : This function checks the criteria for adding a GSM channel
            to "Black List". GSM channels are added to "Black List" only
            after they satisfy this criteria.   
            CSI-LLD section:4.1.1.5.13
*/

LOCAL BOOL cs_check_black_list_criteria(U8 region,U16 arfcn,U8 rxlev)
{
  GET_INSTANCE_DATA;
  UBYTE band_index;
  U16 local_arfcn = arfcn&ARFCN_MASK;
  TRACE_FUNCTION("cs_check_black_list_criteria()");

  /* Check if the carrier is already present in the black list */
  if(cs_is_in_black_list(region, local_arfcn))
    return FALSE;

  /* Check if the carrier is present in White List */
  if(cs_is_in_white_list(region, local_arfcn))
    return FALSE;

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  band_index = cs_get_band_index(arfcn);
#else
  band_index = get_band_index(arfcn);
#endif
  /* RxLev of carrier is above upper threshold. Add the carrier
   * to Black List immediately 
   */
  if(rxlev >= rr_data->dyn_config.upper_rxlev_thr[band_index])
  {
    return TRUE;
  }
 
  /* Reasonably strong carrier. Add to Black List only if its sync fail
   * counter criteria is satisfied 
   */
  if((rxlev >= rr_data->dyn_config.medium_rxlev_thr[band_index]) AND 
      (rxlev < rr_data->dyn_config.upper_rxlev_thr[band_index]))
  {
    cs_inc_sync_fail_counter(region, local_arfcn);

    if(cs_get_sync_fail_counter(region, local_arfcn) >= MAX_SYNC_FAILURES)
    { 
      return TRUE;
    }
  }
  return FALSE;
}

/*
+----------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : RR_CS                  |
| STATE   : code                ROUTINE : cs_update_black_list   |
+----------------------------------------------------------------+

  PURPOSE : This function is used to update "Black List" database
            after initial PLMN search.It first clears the current 
            "Black list" database from RAM and then adds some cells 
            to "Black List" according to the current attrib-utes 
            array values(BLACK_LIST_FLAG)  of the Initial PLMN 
            selection process. This function is called un-der the 
            following cases
            1.MS enters Limited or No service after Initial PLMN search
            2.MS enters Full service in a different Location area 
              from where it is switched off after initial PLMN search
            CSI-LLD section:4.1.1.5.8
*/

GLOBAL void cs_update_black_list(void)
{
  GET_INSTANCE_DATA;

  U8 update_black_list = FALSE;
  U8 i, region;

  TRACE_FUNCTION("cs_update_black_list()");
  
  /* If the outcome of first FUNC_PLMN_SEARCH is not full service.
   * Erase the Black List
   */
  if((rr_data->ms_data.req_mm_service EQ FUNC_PLMN_SRCH AND 
      rr_data->ms_data.rr_service NEQ FULL_SERVICE))
  {
    update_black_list = TRUE;
  }

  TRACE_EVENT_P8 ( "Old [%d]SC MCC/MNC r=%x%x%x/%x%x%x/%d",
    rr_data->cs_data.white_list.last_sc_arfcn,
    rr_data->cs_data.white_list.last_sc_lac.mcc[0],
    rr_data->cs_data.white_list.last_sc_lac.mcc[1],
    rr_data->cs_data.white_list.last_sc_lac.mcc[2],
    rr_data->cs_data.white_list.last_sc_lac.mnc[0],
    rr_data->cs_data.white_list.last_sc_lac.mnc[1],
    rr_data->cs_data.white_list.last_sc_lac.mnc[2],
    rr_data->cs_data.white_list.last_sc_lac.lac);

  TRACE_EVENT_P8 ( "New [%d]SC MCC/MNC r=%x%x%x/%x%x%x/%d",
    rr_data->nc_data[SC_INDEX].arfcn,
    rr_data->nc_data[SC_INDEX].lai.mcc[0],
    rr_data->nc_data[SC_INDEX].lai.mcc[1],
    rr_data->nc_data[SC_INDEX].lai.mcc[2],
    rr_data->nc_data[SC_INDEX].lai.mnc[0],
    rr_data->nc_data[SC_INDEX].lai.mnc[1],
    rr_data->nc_data[SC_INDEX].lai.mnc[2],
    rr_data->nc_data[SC_INDEX].lai.lac);
 
  /* If the current serving cell belongs to a different Location area.
   * Erase the Black List
   */
  if((rr_data->ms_data.rr_service EQ FULL_SERVICE) AND 
     ((rr_data->cs_data.white_list.last_sc_arfcn EQ NOT_PRESENT_16BIT) OR
     (!dat_plmn_equal_req(rr_data->cs_data.white_list.last_sc_lac.mcc,
                          rr_data->cs_data.white_list.last_sc_lac.mnc,
                          rr_data->nc_data[SC_INDEX].lai.mcc,
                          rr_data->nc_data[SC_INDEX].lai.mnc)) OR
     (rr_data->cs_data.white_list.last_sc_lac.lac NEQ 
      rr_data->nc_data[SC_INDEX].lai.lac)))
  {
    update_black_list = TRUE;
  } 

  if(update_black_list)
  {
    /* Erase the current Black List */
    cs_clear_black_list(CLR_BLACK_LIST_RAM);
    
    for(i=0;i<rr_data->cs_data.max_arfcn;i++)
    {
      if(CS_GET_BLACK_LIST_FLAG(i))
      {
        /* Obtain the region */
        region = CS_GET_REGION_FROM_FREQ(rr_data->cs_data.arfcn[i]);

        cs_add_to_black_list(region, 
                             (USHORT)(rr_data->cs_data.arfcn[i]),
                             rr_data->cs_data.rxlev[i]);
      }
    } /* i < max_arfcn */

    TRACE_EVENT("BL updated after inital PLMN search");
  }
}

/*
+----------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : RR_CS                        |
| STATE   : code                ROUTINE : cs_store_black_list          |
+----------------------------------------------------------------------+

  PURPOSE : This is a wrapper function for storing  "Black List" 
            information to FFS . This in turn calls 
            rr_csf_write_black_list( ) to store "Black List" to FFS. 
            This function is called during power off.
            CSI-LLD section:4.1.1.5.2
*/

GLOBAL void cs_store_black_list(void)
{
  GET_INSTANCE_DATA;
#if defined(_SIMULATION_FFS_)
  rr_csf_write_black_list(&rr_data->cs_data.black_list.list[0]);
#endif
}

/*
+----------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : RR_CS                        |
| STATE   : code                ROUTINE : cs_get_new_search_mode       |
+----------------------------------------------------------------------+

  PURPOSE : This routine returns the new search mode depending on the timer
	    active and the previous search mode
            CSI-LLD section:4.1.3.4.1.1
*/

GLOBAL U8 cs_get_new_search_mode(void)
{ 
  GET_INSTANCE_DATA;
  TRACE_FUNCTION("cs_get_new_search_mode()");

  if(IS_TIMER_ACTIVE(T_FAST_CS))
  {
    /* Fast search timer is active */
    return FAST_SEARCH_MODE;
  } 

  if(IS_TIMER_ACTIVE(T_NORMAL_CS))
  {
    /* Normal search timer is active */
    return NORMAL_SEARCH_MODE;
  } 

  TRACE_EVENT_P1("Previous Search Mode : %d",
    rr_data->cs_data.previous_search_mode);

  if((rr_data->cs_data.previous_search_mode EQ FULL_SEARCH_MODE)
     OR (rr_data->cs_data.previous_search_mode EQ FAST_SEARCH_MODE))
  {
    return NORMAL_SEARCH_MODE;   
  }  

  if(rr_data->cs_data.previous_search_mode EQ NORMAL_SEARCH_MODE)
  {
    return  FULL_SEARCH_MODE;
  }

  return FULL_SEARCH_MODE;
}
/*
+----------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : RR_CS                        |
| STATE   : code                ROUTINE : cs_handle_search_mode_timer  |
+----------------------------------------------------------------------+

  PURPOSE : This routine handles the timers for the new search modes
            CSI-LLD section:4.1.3.4.1.2
*/
GLOBAL void cs_handle_search_mode_timer(U8 search_mode)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION("cs_handle_search_mode_timer()");

  if((search_mode EQ FAST_SEARCH_MODE) AND (rr_data->dyn_config.tfast_cs_val))
  {
    if(!IS_TIMER_ACTIVE(T_FAST_CS))
    {
      TIMERSTART (T_FAST_CS,rr_data->dyn_config.tfast_cs_val);
    }
  }

  if((search_mode EQ NORMAL_SEARCH_MODE) AND (rr_data->dyn_config.tnormal_cs_val))
  {
    if(!IS_TIMER_ACTIVE(T_NORMAL_CS))
    {
      TIMERSTART (T_NORMAL_CS, rr_data->dyn_config.tnormal_cs_val);
    }
  }
  
  if(search_mode EQ FULL_SEARCH_MODE)
  {
    if(rr_data->sc_data.mm_started EQ MM_ORIGINATED AND 
       rr_data->ms_data.req_mm_service NEQ FUNC_NET_SRCH_BY_MMI)
    {
      TIMERSTOP(T_FAST_CS);
      TIMERSTOP(T_NORMAL_CS);
    }
  }
}
  
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : RR_CS                         |
| STATE   : code             ROUTINE : cs_update_std_ffs             |
+--------------------------------------------------------------------+

  PURPOSE : This function updates the std value depending upon the 
            frequency band support by MS. Frequncy band support is 
            available in the FFS.

*/
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
LOCAL UBYTE cs_update_std_ffs (UBYTE new_std)
{
  UBYTE freq_bands;
  
  rr_csf_get_freq_bands (&freq_bands);
  switch (new_std)
  {
    case STD_DUAL_EGSM:
      if ((freq_bands & BAND_GSM_850) EQ BAND_GSM_850)
      {
        return STD_850_900_1800;
      }
      break;
    case STD_DUAL_US:
      if ((freq_bands & BAND_GSM_900) EQ BAND_GSM_900)
      {
        return STD_850_900_1900;
      }
      break;
    default:
      TRACE_EVENT_P1 ("wrong std for updation using FFS %x", new_std);
  }
  return new_std;
}
#endif
#endif
