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

#ifndef RR_ATTF_C
#define RR_ATTF_C

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
LOCAL void rr_unpack_plmn (T_plmn *plmn, const UBYTE *packed, USHORT index);
/* OMAPS00085607 - N950 Memory Optimization */
LOCAL void start_treg_timer(void);
#define TWO 2


static void att_build_back_from_dedicated (void);
static void att_calculate_c2            (UBYTE               index);
static SHORT att_calculate_c2_diff      (UBYTE               index);
static UBYTE att_calculate_digits       (UBYTE               *digits);


static BOOL att_cell_in_data_structures (T_MPH_MEASUREMENT_IND *report,
                                         UBYTE               index);
static UBYTE att_cell_in_measure_report  (T_MPH_MEASUREMENT_IND *report,
                                         UBYTE               index);
static void att_check_2ter_read         (UBYTE               index);
static BOOL att_check_cell              (void);
LOCAL  UBYTE att_get_txpwr_max_cch      (UBYTE index);
static void att_clear_forb_list (int list_type);
static void att_clear_parallel_search   (void);
static void att_code_prr_mm_info        (T_mm_info           *mm_info);
static void att_copy_c2_parameter       (T_C2_PARAMETER     *c2_par,
                                         T_si3_rest_oct     *rest_oct );
static void att_copy_c2_parameter_si3   (UBYTE               index,
                                         T_si3_rest_oct     *rest_oct);
static void att_copy_found_plmn         (T_FOUND_LIST * list,
                                         USHORT n_in_source_85_dBm,
                                         USHORT i_in_copy);
static void att_copy_rach_parameter     (UBYTE               index,
                                         T_rach_ctrl         *rach,
                                         UBYTE               indicate_changes);
static UBYTE att_get_highest_c2_index   (void);
static UBYTE att_get_next_highest_c2_idx(UBYTE               old_index);
static UBYTE att_get_next_highest_c2_val(void);
static UBYTE att_get_func               (void);
static UBYTE att_get_next_highest_rx    (void);
static void att_insert_cell_in_data     (T_MPH_MEASUREMENT_IND *report,
                                         UBYTE               index);
static void att_order_plmns             (void);
static USHORT att_number_of_plmns_greater_85_dBm (void);
static void att_copy_plmns_lower_or_equal_85_dBm (T_FOUND_LIST * list, USHORT i_in_copy);
static BOOL att_priority_check          (void);
static void att_reorder_mph_ncell_req   (T_MPH_NEIGHBOURCELL_REQ*
                                         mph_ncell_req);
static void att_search_cell             (void);
static void att_select_cell_dedicated (void);
static void att_try_old_cell            (void);
static void att_print_selection_type    (UBYTE                selection_type);
/* Implements Measure#32: Row 36, 39 and 40 */
static void att_print_op                (T_op                *op,
                                         T_S2I_STRING        titel);
static void att_begin_cs                (UBYTE req_mm_service);

#if defined (REL99) && defined (TI_PS_FF_EMR)
static void att_check_for_si5ter_and_enhpara (UBYTE old_index);
#endif


LOCAL void att_copy_sys_info_2bis_2ter_par(UBYTE index, T_SI_TYPE si_type,
                                           T_LIST  *new_2_bis_ter_list,
                                           BUF_neigh_cell_desc   *neigh_cell_desc,
                                           UBYTE  indicate_changes);

/*==== VARIABLES ==================================================*/
GLOBAL UBYTE test_house = FALSE;

/*==== MACROS =====================================================*/

#if !defined (NTRACE)

#define TRACE_C1(index)\
  TRACE_EVENT_P4 ("[%u]i%u C1=%-2d rxlev=%u",\
           rr_data->nc_data[index].arfcn,index,\
           rr_data->nc_data[index].c1,\
           rr_data->nc_data[index].rxlev)

#define TRACE_C2(index)\
  TRACE_EVENT_P3 ("[%u]i%u C2=%-2d AT=%d",\
    rr_data->nc_data[index].arfcn,index,\
    rr_data->nc_data[index].c2,\
    rr_data->nc_data[index].avail_time)

#define TRACE_C1_C2(index)\
  TRACE_EVENT_P6 ("[%u]i%u C1=%-2d C2=%-2d rxlev=%-2u AT=%d",\
           rr_data->nc_data[index].arfcn,index,\
           rr_data->nc_data[index].c1,\
           rr_data->nc_data[index].c2,\
           rr_data->nc_data[index].rxlev,\
           rr_data->nc_data[index].avail_time)

#define TRACE_SELECTION_TYPE(type)      att_print_selection_type(type)

/* Implements Measure#32: Row 36, 39 and 40 */
#define TRACE_OP_TYPE(op,titel)         att_print_op (op, S2I_STRING(titel))

#else   /* !NTRACE */

#define TRACE_C1(i)
#define TRACE_C2(i)
#define TRACE_C1_C2(i)
#define TRACE_SELECTION_TYPE(type)
#define TRACE_OP_TYPE(op,titel)

#endif  /* !NTRACE */

/*==== FUNCTIONS ==================================================*/

/*
 * -------------------------------------------------------------------
 * Procedures
 * -------------------------------------------------------------------
 */
/* Implements Measure#32: Row 60 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_print_forb_list
+--------------------------------------------------------------------+

  PURPOSE : After a successful location updating request MM informs
            RR about this. If the location area information is stored
            inside of one of the forbidden location area lists this
            information must be updated. In this function, the entry
            is deleted from the given forbidden list.

*/

LOCAL void att_print_forb_list ( int                list_type,
                                 const T_plmn     * plmn,
                                 USHORT             lac,
                                 unsigned int       index,
                                 BOOL               to_delete)
{
  if(list_type EQ FORBIDDEN_LIST_NORMAL)
  {
    if(to_delete EQ FALSE)
    {
      TRACE_EVENT_P1 ( "ADD FORB LIST %u:", index);
    }
    else
    {
      TRACE_EVENT_P1 ( "DEL FORB LIST %u:", index);
    }
  }
  else
  {
    if(to_delete EQ FALSE)
    {
      TRACE_EVENT_P1 ( "ADD FORB LIST %u:", index);
    }
    else
    {
      TRACE_EVENT_P1 ( "DEL ROAM FORB LIST %u:", index);
    }
  }
  TRACE_EVENT_P7 ( "    MCC/MNC=%x%x%x/%x%x%x LAC=%x",
      plmn->mcc[0],
      plmn->mcc[1],
      plmn->mcc[2],
      plmn->mnc[0],
      plmn->mnc[1],
      plmn->mnc[2],
      lac);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_add_to_forb_list       |
+--------------------------------------------------------------------+

  PURPOSE : RR stores a list of forbidden location area codes. After
            a specific location updating reject cause this information
            will be given by MM with a RR_SYNC_REQ. The check against
            forbidden location areas is part of the cell selection and
            cell reselection process.

*/

GLOBAL void att_add_to_forb_list (int         list_type,
                                  const T_plmn     *plmn,
                                  USHORT            lac)
{
  GET_INSTANCE_DATA;
  int i;
  T_loc_area_ident *forb_list;

  if (list_type EQ FORBIDDEN_LIST_NORMAL)
    forb_list = &rr_data->ms_data.forb_lac_list[0];
  else
    forb_list = &rr_data->ms_data.roam_forb_lac_list[0];

  TRACE_FUNCTION ("att_add_to_forb_list()");

  for (i = 0; i < MAX_LAI; i++)
  {
    /*
     * Check whether the location area code is already stored. In
     * this case it is not necessary to store it again
     */
    if ((lac EQ forb_list[i].lac) AND
        dat_plmn_equal_req (plmn->mcc, plmn->mnc,
                            forb_list[i].mcc, forb_list[i].mnc))
      break;
  }

  if (i EQ MAX_LAI)
  {
    /*
     * Location Area Code is not stored yet. Look for a free entry
     * and store the location area code.
     */
    for (i = 0; i < MAX_LAI; i++)
    {
      if (forb_list[i].lac EQ NOT_PRESENT_16BIT)
      {
        memcpy (forb_list[i].mcc, plmn->mcc, SIZE_MCC);
        memcpy (forb_list[i].mnc, plmn->mnc, SIZE_MNC);
        forb_list[i].lac = lac;
        break;
      }
    }

    if (i EQ MAX_LAI)
    {
      /*
       * all entries are used. Then delete the first (and oldest) one,
       * move each entry and store the new one in the last position.
       */
      memmove (&forb_list[0], &forb_list[1],
               sizeof (T_loc_area_ident) * (MAX_LAI-1));
      memcpy (forb_list[MAX_LAI-1].mcc, plmn->mcc, SIZE_MCC);
      memcpy (forb_list[MAX_LAI-1].mnc, plmn->mnc, SIZE_MNC);
      forb_list[MAX_LAI-1].lac = lac;
      i--;
    }
  }

/* Implements Measure#32: Row 4 */
  att_print_forb_list ( list_type, plmn, lac, i, FALSE);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_analyze_measure_report |
+--------------------------------------------------------------------+

  PURPOSE : After reception of a MPH_MEASUREMENT_IND from layer 1 the
            channels are analyzed for a cell reselection decision.

*/

GLOBAL void att_analyze_measure_report (T_MPH_MEASUREMENT_IND *report)
{
  GET_INSTANCE_DATA;
  UBYTE i = 0;

  TRACE_FUNCTION ("att_analyze_measure_report()");

  /*
   * The variable no_reselect controls the condition not to perform a
   * new cell reselection due to  C2(NC) > C2(SC) after a cell reselection
   * for 15 seconds
   */

  /*
   * The fieldtest has shown, that sometimes the serving cell was also reported
   * as a neighbourcell. This function removes double entries
   */
  att_remove_multiple_channels ();

  /*
   * All possible neighbourcells plus the serving cell are checked.
   */
  for (i = 0; i < 6; i++)
  {
    /*
     * The tnnn variable controls the condition that a cell is barred for five seconds
     * for cell reselection purposes after a random access failure.
     */
    /*
     * check the status of a cell using the data stored in RR for neighbour- and serving
     * cell.
     */
    switch (rr_data->nc_data[i].bcch_status)
    {
      case EMPTY:
        /*
         * RR has nothing stored in this area. So there is nothing to do.
         */
        break;

      case DECODED:
        /*
         * In this area RR has stored Data for a neighbourcell. The status is decoded,
         * that means RR has received a system information 3,4,7 or 8 to calculate
         * the cell reselection criterion C2.
         *
         * Now it is checked against the incoming measurement report.
         */
        switch (att_cell_in_measure_report (report, i))
        {
          case CELL_IS_INSERTED:
            /*
             * The cell is still included in the measurement report, that means
             * it is one of the six strongest carrier and layer 1 is synchronized
             * to the cell.
             *
             * The time how long the cell is already synchronized is stored up to
             * a period of 700 seconds. This informatin is used for the calculation
             * of the cell reselection criterion C2 which may has a time dependent
             * factor. In fact the frames of layer 1 are counted.
             */
            if (rr_data->nc_data[i].avail_time < PERIOD_700_SEC)
              rr_data->nc_data[i].avail_time += rr_data->ms_data.fn_offset;
            break;

          case BSIC_HAS_CHANGED:
            /*
             * The cell is still included in the measurement report, but layer1
             * has detected a change of the BSIC during re-synchronisation on the
             * SB channel. So the stored information for calculation of C2 is useless
             * and the status and the avail time is cleared.
             */

            rr_data->nc_data[i].bcch_status = NON_DECODED;
            rr_data->nc_data[i].avail_time  = 0;
            break;

          case CELL_IS_NOT_INSERTED:
            /*
             * The cell is not longer included. The area in RR is released.
             */
            rr_data->nc_data[i].bcch_status = EMPTY;
            break;
        }
        break;

      case NON_DECODED:
        /*
         * RR has already stored this cell in its data, but has not received
         * the needed information for calculation of cell reselection criterion C2.
         *
         * Now it is checked against the incoming measurement report.
         */
        switch (att_cell_in_measure_report (report, i))
        {
          case CELL_IS_INSERTED:
            /*
             * The cell is still included. For later calculation of the cell
             * reselection criterion C2 the time is counted for which the cell
             * is synchronized.
             */
            if (rr_data->nc_data[i].avail_time < PERIOD_700_SEC)
              rr_data->nc_data[i].avail_time += rr_data->ms_data.fn_offset;
            break;

          case BSIC_HAS_CHANGED:
            /*
             * The layer 1 indicated that the BSIC has changed during re-synchronisation
             * to the SB channel. The status will not change, but the avail time is
             * resetted.
             */
            rr_data->nc_data[i].avail_time = 0;
            break;

          case CELL_IS_NOT_INSERTED:
            /*
             * Layer 1 indicates that the cell is not longer one of the six strongest or
             * that the synchronisation has been lost. The storage area in RR is released.
             */
            rr_data->nc_data[i].bcch_status = EMPTY;
            rr_data->nc_data[i].avail_time  = 0;
            break;
        }
        break;
    }
  }

  /*
   * store the Fieldstrength of the serving cell and update the information
   * of the power campaign.
   */
  rr_data->nc_data[SC_INDEX].rxlev = report->rx_lev_full;
  cs_set_rxlev (report->rx_lev_full, rr_data->nc_data[SC_INDEX].arfcn);

  if (rr_data->nc_data[SC_INDEX].rxlev >
      rr_data->lup_rxlev + 10)
  {
    /*
     * A fieldstrength jump more then 10 dBm has been detected. This can be the
     * trigger for MM to restart a location updating request if needed. It indicates
     * for example that the mobile has left a tunnel area without coverage.
     *
     * This is not used by MM if a test SIM is inserted. It is an improvement for
     * the field beside the GSM specifications.
     */
    PALLOC (sync, RR_SYNC_IND);

    sync->ciph              = NOT_PRESENT_8BIT;
    sync->chm.ch_mode       = NOT_PRESENT_8BIT;
    sync->synccs            = SYNCCS_LUP_RETRY;
    sync->mm_info.valid     = FALSE;
    sync->bcch_info.v_bcch  = FALSE;

    PSENDX (MM, sync);

    /*
     * Store the new value to avoid multiple signalling of the fieldstrength jump
     */
    rr_data->lup_rxlev = rr_data->nc_data[SC_INDEX].rxlev;
  }

  /*
   * Now look for all neighbourcells in the measurement report if they are new.
   */
  for (i = 0; i < report->ncells.no_of_ncells; i++)
  {
    cs_set_rxlev (report->ncells.rx_lev[i], report->ncells.arfcn[i]);
    /*
     * If a channel is not inside the RR data structure, insert the cell
     */
    if (! att_cell_in_data_structures (report, i))
      att_insert_cell_in_data (report, i);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_bcch_status_to_decoded |
+--------------------------------------------------------------------+

  PURPOSE : After reception of the system information type 3,4,7 or 8
            the status of a cell changes from NON_DECODED to DECODED
            and it is possible to calculate the cell reselection
            criterion C2.

*/

GLOBAL void att_bcch_status_to_decoded (UBYTE index)
{
  GET_INSTANCE_DATA;
  TRACE_EVENT_P2 ("[%u]%u att_bcch_status_to_decoded()", rr_data->nc_data[index].arfcn, index);

  rr_data->nc_data[index].bcch_status  = DECODED;
  rr_data->nc_data[index].bcch_counter = 0;
  rr_data->nc_data[index].c1_counter   = 0;
  #ifdef GPRS
  if( rr_data->gprs_data.use_c31 NEQ TRUE )
  {
  #endif

    if ((rr_data->c_ncell_bcch > 0) AND (rr_data->c_ncell_bcch NEQ NOT_INITIALISED))
      rr_data->c_ncell_bcch--;

    if (rr_data->resel_pending)
    {
      if (rr_data->c_ncell_bcch EQ 0)
      {
        SET_STATE (STATE_ATT, ATT_CON_EST);
#ifdef GPRS
        att_start_cell_reselection_gprs(CELL_RESELECTION_RACH);
#else
        att_start_cell_reselection(CELL_RESELECTION_RACH);
#endif
        rr_data->resel_pending = FALSE;
        rr_data->c_ncell_bcch = NOT_INITIALISED;
      }
    }
    else
    {
      att_calculate_c2 (index);
    }
#ifdef GPRS
  }
  else
  {
    if( rr_data->nc_data[index].v_cr_par EQ CR_PAR_INVALID )
    {
      att_convert_idle_c31_cr(index);
    }
    else
    {
       TRACE_EVENT_P2(" arfcn %d, index = %d has valid CR_ PAR", rr_data->nc_data[index].arfcn, index);
    }
  }
  #endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_bits_to_byte           |
+--------------------------------------------------------------------+

  PURPOSE : The function is used to calculate the positions of 1-bits
            into a byte array. This is needed for calculation of a
            frequency hopping list for a mobile allocation. The input
            parameter are the bitmap and the size of the bitmap in bytes.
            The result is a byte array containing the positions of 1-bits
            followed by an end identifier NOT_PRESENT_8BIT (0xFF). The
            maximum size of the incoming array is 32 bytes.

            Example: a mobile allocation looks like this:

                     0x42 = 0b01000010  -> bit    15 14 13 12 11 10  9  8
                     0x81 = 0b10000001  -> bit     7  6  5  4  3  2  1  0

            The size of the input array is 2 and the output is:

            1,8,10,15,0xFF    (starting with position 1 in bit 0)

*/

GLOBAL void att_bits_to_byte (UBYTE  * num,
                              UBYTE    size,
                              UBYTE  * bits)
{
  SHORT ix;
  UBYTE bit;
  UBYTE buf_ix;

  TRACE_FUNCTION ("att_bits_to_byte()");

  /*
   * For all bytes in the input field
   */
  for (ix = 0; ix < size; ix++)
  {
    buf_ix = bits[size-1-ix];

    /*
     * for each bit inside this byte
     */
    bit = 0;
    while (8 > bit)
    {
      if (buf_ix & (1 << bit))
      {
        /*
         * If bit is set, store the position in the output field
         */
        *num++ = ((USHORT)ix << 3) + bit + 1;
      }
      bit++;
    }
  }
  /*
   * add the end identifier
   */
  *num = NOT_PRESENT_8BIT;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_build_cbch             |
+--------------------------------------------------------------------+

  PURPOSE : configure at the end of the cell selection the CBCH channel
            in layer 1. The CBCH channel is optional defined in system
            information type 4.
            When the cell has PBCCH and it is a Release-99 cell,
            the CBCH configuration should be taken from gprs database.
            This information is provided by GRR through RRGRR_CBCH_INFO_IND.
            CBCH channel configuration is received on PBCCH on PSI8.
*/

#ifdef REL99
LOCAL  void att_build_cbch (void)
#else
GLOBAL void att_build_cbch (void)
#endif
{/*lint -e813 supress info of length of T_LIST*/
  GET_INSTANCE_DATA;
  T_LIST hop_list;
  PALLOC (cbch_req, MPH_CBCH_REQ);

  TRACE_FUNCTION ("att_build_cbch()");

  /*
   * Initialize the primitive to layer 1
   */
  memset (cbch_req, 0, sizeof (T_MPH_CBCH_REQ));
  if (rr_data->ms_data.rr_service EQ FULL_SERVICE)
  {
    /*
     * CBCH is activated only if RR is in full service
     */
    if (rr_data->sc_data.cd.cbch_chan_desc_avail NEQ NO_CONTENT )
    {
      /*
       * If a CBCH channel description has been received with
       * system information tpye 4, activate the channel in layer 1.
       */
      cbch_req->cbch.stat = STAT_ACT;
      cbch_req->cbch.ch   = rr_data->sc_data.cd.cbch_chan_desc.chan_type;
      cbch_req->cbch.tn   = rr_data->sc_data.cd.cbch_chan_desc.tn;
      cbch_req->cbch.tsc  = rr_data->sc_data.cd.cbch_chan_desc.tsc;
      cbch_req->cbch.h    = rr_data->sc_data.cd.cbch_chan_desc.hop;

      switch (cbch_req->cbch.h)
      {
        case H_NO:
          /*
           * CBCH does not use frequency hopping, then configure simply the
           * channel number
           */
          cbch_req->cbch.arfcn = rr_data->sc_data.cd.cbch_chan_desc.arfcn;
          break;

        case H_FREQ:
          /*
           * CBCH uses frequency hopping, then configure MAIO and HSN
           * and create a frequency hopping list from the cell channel
           * description in system information 1 and the mobile allocation
           * in system information 4.
           */
          cbch_req->cbch.maio  = rr_data->sc_data.cd.cbch_chan_desc.maio;
          cbch_req->cbch.hsn   = rr_data->sc_data.cd.cbch_chan_desc.hsn;
          srv_create_chan_mob_alloc (&rr_data->sc_data.cd.cell_chan_desc,
                                     &hop_list,
                                     rr_data->sc_data.cd.cbch_mob_alloc);

          /* CSI-LLD section:4.1.1.11
           * This function Updates the black list with the MA list received
           * in the CBCH allocation  
           */   
          cs_remove_BA_MA_from_black_list(rr_data->cs_data.region,&hop_list);

          srv_create_list (&hop_list, cbch_req->cbch.ma, MAX_MA_CHANNELS, TRUE,
                           0);
          break;
      }
    }
    else
      /*
       * In all other cases configure layer 1 without CBCH.
       */
      cbch_req->cbch.stat = STAT_INACT;
  }

  PSENDX (PL, cbch_req);
}

#ifdef REL99
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_config_cbch            |
+--------------------------------------------------------------------+

  PURPOSE : The following conditions have to be satisfied for 
            configuring CBCH.
            1. Full service should have been enabled.
            2. Mobile should be in idle/packet mode.
            This function checks these conditions before configuring
            CBCH. If function is called during packet transfer mode,
            a flag in GPRS data is set signalling that this function
            should be called again after entering idle mode.
*/
GLOBAL void att_config_cbch (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_config_cbch()");
  /*
   * CBCH is activated only if RR is in full service
   */
  if ((rr_data->ms_data.rr_service EQ FULL_SERVICE) )
    {
#ifdef GPRS
      /* check if mobile is in PTM */
      if(!dat_gprs_cell_in_ptm())
        {
          /* mobile is NOT in packet transfer mode */

          if(!att_gprs_cell_has_pbcch() OR !att_gprs_get_nw_release())
            {
             /* This cell does not have pbcch,
              * OR this is a R - 98 or lower cell.
              * hence take info stored in SI 4 
              */
#endif /* GPRS */
              att_build_cbch();
#ifdef GPRS
            }
          else
            {
              /* This Cell has PBCCH and also this is Release-99 cell.
               * Configure ALR if CBCH info is available form GRR. 
               */
              if(rr_data->gprs_data.cbch_psi_valid)
                {
                  PALLOC (cbch_req, MPH_CBCH_REQ);
                  memcpy(&(cbch_req->cbch),
                         &(rr_data->gprs_data.cbch_psi8),
                         sizeof(T_cbch));
                  PSENDX (PL, cbch_req);
                }
              /*else wait till CBCH info is received from GRR */
            }
          rr_data->gprs_data.cbch_info_rxvd_in_ptm = FALSE;
        }
      else
        {
          rr_data->gprs_data.cbch_info_rxvd_in_ptm = TRUE;
        }
#endif /* GPRS */
    }
  else
    {
#ifdef GPRS
      if(!dat_gprs_cell_in_ptm())
#endif /* GPRS */
      {    
        PALLOC (cbch_req, MPH_CBCH_REQ);
        memset (cbch_req, 0, sizeof (T_MPH_CBCH_REQ));
        cbch_req->cbch.stat = STAT_INACT;
        PSENDX (PL, cbch_req);
      }
    }
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_build_classmark_req    |
+--------------------------------------------------------------------+

  PURPOSE : Configure at the end of the cell selection the classmark of
            the mobile to layer 1. This information is used by layer 1
            for random access.

*/

GLOBAL void att_build_classmark_req (void)
{
  GET_INSTANCE_DATA;
  PALLOC (classmark_req, MPH_CLASSMARK_REQ);

  TRACE_FUNCTION ("att_build_classmark_req()");

#if defined (_SIMULATION_)
  /*
   * In the windows simulation it is necessary to read the data
   * from the non-volantile memory (PCM) again.
   */
  rr_csf_check_rfcap (FALSE);
#endif

  if((std EQ STD_DUAL) OR (std EQ STD_DUAL_EGSM))
  {
    classmark_req->classmark.pclass = rr_data->ms_data.rf_cap.rf_power.pow_class4[IDX_PWRCLASS_900].pow_class-1;
    classmark_req->classmark.pclass2 = rr_data->ms_data.rf_cap.rf_power.pow_class4[IDX_PWRCLASS_1800].pow_class-1;
  }
  else if(std EQ STD_DUAL_US)
  {
    classmark_req->classmark.pclass = rr_data->ms_data.rf_cap.rf_power.pow_class4[IDX_PWRCLASS_850].pow_class-1;
    classmark_req->classmark.pclass2 = rr_data->ms_data.rf_cap.rf_power.pow_class4[IDX_PWRCLASS_1900].pow_class-1;
  }
  else
  {
    /*
     * In single bands only one power class is forwarded to layer 1.
     */
#ifdef GPRS

#ifdef REL99
    classmark_req->classmark.pclass = rr_data->ms_data.ra_cap.ra_cap_values.acc_cap.pow_class-1;
#else
    classmark_req->classmark.pclass = rr_data->ms_data.ra_cap.acc_cap.pow_class-1;
#endif

#else
    if (std EQ STD_1900)
    {
      classmark_req->classmark.pclass = rr_data->ms_data.classmark3.pcs1900_cap-1;
    }
    else if (std EQ STD_850)
    {
      classmark_req->classmark.pclass = rr_data->ms_data.classmark3.gsm850_cap-1;
    }
    else
    {
      classmark_req->classmark.pclass = rr_data->ms_data.classmark3.radio_cap_1-1;
    }
#endif
    classmark_req->classmark.pclass2= 0;
  }
  PSENDX (PL, classmark_req );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_build_idle_req         |
+--------------------------------------------------------------------+

  PURPOSE : The function is used to configure the idle mode in layer 1.
            Two modes are possible: cell selection and cell reselection.
            In cell selection mode RR has all information to perform
            a complete layer 1 configuration. In cell reselection mode
            layer 1 is configured only as much as needed and it is
            expected that layer 1 goes in paging reorganization mode
            to catch all pagings and BCCH messages.
*/

GLOBAL void att_build_idle_req (UBYTE     index,
                                UBYTE     mode)
{
  GET_INSTANCE_DATA;
  T_CELL_DATA *cd;
  PALLOC (idle_req, MPH_IDLE_REQ);

  TRACE_FUNCTION ("att_build_idle_req()");

  /*
   * configure the mode of layer 1 (cell selection or cell reselection
   */
  idle_req->mod       = mode;

  /*
   * set the pointer to the correct data
   * serving cell or one of the neighbourcells.
   */
  if (index EQ SC_INDEX)
  {
    cd = &rr_data->sc_data.cd;
#if defined (REL99) && defined (TI_PS_FF_EMR)
    idle_req->si2quater_status = cd->si2quater_status;
    idle_req->si2quater_pos       = cd->si2quater_pos;

    if ( (mode EQ MODE_CELL_SELECTION ) AND 
      (cd->si2quater_status EQ SI2QUATER_CONFIGURE) AND
      ((rr_data->sc_data.cd.sys_info_read & (ALL_SYS_INFO_READ | SYS_INFO_2QUATER_READ)) EQ ALL_SYS_INFO_READ))      
      rr_data->sc_data.cd.si2quater_status = SI2QUATER_ACQ_PENDING;  
#endif
  }
  else
  {
#if defined (REL99) && defined (TI_PS_FF_EMR)
    idle_req->si2quater_status = SI2QUATER_ABSENT;
#endif
    cd = &rr_data->cr_data.cd;
  }

  /*
   * look to the reached service of RR
   */
  switch (rr_data->ms_data.rr_service)
  {
    case LIMITED_SERVICE:
      /*
       * In limited mode no pagings are possible. To save
       * power the minimum number of measurements (equal
       * to a long paging period) is configured.
       * The other parameters are independent from BCCH or
       * SIM card, because paging is not possible.
       */
      idle_req->pg            = 0;
      idle_req->tn            = 0;
      idle_req->ncc_permitted = NOT_PRESENT_8BIT;
      idle_req->dlt           = 10; /* for bs_pa_mfmrs = 9 */
      idle_req->bs_pa_mfrms   = 7;  /* for bs_pa_mfmrs = 9 */
      break;

    case FULL_SERVICE:
      /*
       * The MS is in full service. In this case the paging
       * block must be correctly from the SIM and the BCCH
       * data.
       */
      idle_req->pg   = dat_calc_paging_group (index);
      idle_req->tn   = dat_calc_tn           (index);

      /*
       * The NCC permitted check in layer 1 is used to
       * avoid synchronisation to cells of other networks.
       */
#ifdef GPRS
      if(mode NEQ MODE_CELL_RESELECTION AND
         mode NEQ MODE_CELL_RESELECTION_SYNC_ONLY)
#else
      if (mode NEQ MODE_CELL_RESELECTION)
#endif
        idle_req->ncc_permitted = cd->ncc_permitted;
      else
        idle_req->ncc_permitted = NOT_PRESENT_8BIT;

      idle_req->dlt         = dat_calc_downlink_timeout (index);
      idle_req->bs_pa_mfrms = rr_data->nc_data[index].control_descr.bs_pa_mfrms;
      break;
  }
  idle_req->arfcn = rr_data->nc_data[index].arfcn;

  /* XXX
   * use ext_bcch for forwarding bsic, this solves
   * fieldtest problem of mismatch of BSICs.
   */
  idle_req->ext_bcch  = rr_data->nc_data[index].bsic;

  idle_req->comb_ccch = rr_data->nc_data[index].control_descr.ccch_conf EQ COMB_CCCH_COMB;
  idle_req->bs_ag_blocks_res =
              rr_data->nc_data[index].control_descr.bs_ag_blks_res;

  idle_req->power = att_get_txpwr_max_cch (index);

  idle_req->reorg_only = NORMAL_PGM;
  idle_req->gprs_support = NOT_PRESENT_8BIT;
#if defined FF_EOTD
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  idle_req->eotd_avail = (std EQ STD_1900 OR std EQ STD_850 OR std EQ STD_DUAL_US OR std EQ STD_850_1800 OR std EQ STD_850_900_1800 OR std EQ STD_850_900_1900);
#else
  idle_req->eotd_avail = (std EQ STD_1900 OR std EQ STD_850 OR std EQ STD_DUAL_US);
#endif
#else
  idle_req->eotd_avail = 0;
#endif

#ifdef GPRS
  att_gprs_idle_req(idle_req);
#endif

  EM_IDLE_MODE;

#if defined(_SIMULATION_)
  switch (idle_req->mod)
  {
    case MODE_CELL_SELECTION      :  /* 0x0 cell selection                 */
      TRACE_EVENT_WIN ("MODE_CELL_SELECTION");
      break;
    case MODE_CELL_RESELECTION    :  /* 0x1 cell reselection               */
      TRACE_EVENT_WIN ("MODE_CELL_RESELECTION");
      break;
#if defined(MODE_BACK_FROM_DEDICATED)
    case MODE_BACK_FROM_DEDICATED :  /* 0x2 back from dedicated            */
      TRACE_EVENT_WIN ("MODE_BACK_FROM_DEDICATED");
      break;
#endif  /* MODE_BACK_FROM_DEDICATED */
    case MODE_IMM_ASSIGN          :  /* 0x3 immediate assignment           */
      TRACE_EVENT_WIN ("MODE_IMM_ASSIGN");
      break;
    case MODE_CHAN_ASSIGN         :  /* 0x4 channel assignment             */
      TRACE_EVENT_WIN ("MODE_CHAN_ASSIGN");
      break;
    case MODE_ASYNC_HANDOVER      :  /* 0x5 asynchronous handover          */
      TRACE_EVENT_WIN ("MODE_ASYNC_HANDOVER");
      break;
    case MODE_SYNC_HANDOVER       :  /* 0x6 synchronous handover           */
      TRACE_EVENT_WIN ("MODE_SYNC_HANDOVER");
      break;
    case MODE_PRE_SYNC_HANDOVER   :  /* 0x7 pre synchronous handover       */
      TRACE_EVENT_WIN ("MODE_PRE_SYNC_HANDOVER");
      break;
    case MODE_PSEUDO_SYNC_HANDOVER:  /* 0x8 pseudo synchronous handover    */
      TRACE_EVENT_WIN ("MODE_PSEUDO_SYNC_HANDOVER");
      break;
    case MODE_SYS_INFO_CHANGE     :  /* 0x9 sys info has changed           */
      TRACE_EVENT_WIN ("MODE_SYS_INFO_CHANGE");
      break;
    case MODE_PACKET_TRANSFER     :  /* 0xa enter packet transfer mode     */
      TRACE_EVENT_WIN ("MODE_PACKET_TRANSFER");
      break;
    case MODE_PDCH_ASSIGN         :  /* 0xb PDCH assignment                */
      TRACE_EVENT_WIN ("MODE_PDCH_ASSIGN");
      break;
    case MODE_CELL_CHANGE_ORDER   :  /* 0xc Network controlled Cell Change */
      TRACE_EVENT_WIN ("MODE_CELL_CHANGE_ORDER");
      break;
    case MODE_CELL_RESELECTION_SYNC_ONLY:  /* 0xc Network controlled Cell Change */
      TRACE_EVENT_WIN ("MODE_CR_SYNC_ONLY");
      break;
    case MODE_CONFIG_PL:  /* PBCCH */
      TRACE_EVENT_WIN ("MODE_CONFIG_PL");
      break;
    default:
      TRACE_EVENT_WIN_P1 ("idle_req mode %d unknown!", idle_req->mod);
      break;
  }
#endif  /* _SIMULATION_ */
  if ((mode EQ MODE_CELL_SELECTION) AND
      (rr_data->ms_data.rr_service EQ FULL_SERVICE) AND
      (rr_data->cs_data.act_index NEQ NOT_PRESENT_8BIT))
  {
    rr_data->cs_data.act_index = NOT_PRESENT_8BIT;
    rr_csf_fit_capability ();
#if !defined(NTRACE)
    rr_csf_trace_power ();
#endif  /* !NTRACE */
  }
  PSENDX (PL, idle_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_calculate_c1           |
+--------------------------------------------------------------------+

  PURPOSE : The function calculates the path loss criterion C1 for
            cell selection and cell reselection purposes.

  REFERENCE: GSM 5.08, chapter 6.4 Criteria for cell selection and reselection

*/

/*
 * The following tables define conversion for the power in dBm depending
 * on the frequency band (GSM, DCS or PCS) and the power class (1 to 5).
 */

/*lint -esym(765,p_dcs) | external could be made static | used by GRR  */
/*lint -esym(765,p_pcs) | external could be made static | used by GRR  */
/*lint -esym(765,p_gsm) | external could be made static | used by GRR  */
/*lint -esym(765,p_control_dcs) | external could be made static | used by GRR  */
/*lint -esym(765,p_control_pcs) | external could be made static | used by GRR  */
/*lint -esym(765,p_control_gsm) | external could be made static | used by GRR  */
const SHORT p_dcs [MAX_CLASSES] = { P_CLASS_1_1800,
                                    P_CLASS_2_1800,
                                    P_CLASS_3_1800,
                                    P_CLASS_3_1800,
                                    P_CLASS_3_1800 };


const SHORT p_pcs [MAX_CLASSES] = { P_CLASS_1_1900,
                                    P_CLASS_2_1900,
                                    P_CLASS_3_1900,
                                    P_CLASS_3_1900,
                                    P_CLASS_3_1900 };

const SHORT p_gsm [MAX_CLASSES] = { P_CLASS_2_900,
                                    P_CLASS_2_900,
                                    P_CLASS_3_900,
                                    P_CLASS_4_900,
                                    P_CLASS_5_900 };


/*
 * The following tables converts the air-interface coding of
 * the system information parameter MAX_TXPWR_CCCH to a value
 * in dBm depending on the frequency standard (GSM, DCS or PCS).
 */
const UBYTE p_control_gsm [32] =
{                                   39,   /*  0 -> 39 dBm */
                                    39,   /*  1 -> 39 dBm */
                                    39,   /*  2 -> 39 dBm */
                                    37,   /*  3 -> 37 dBm */
                                    35,   /*  4 -> 35 dBm */
                                    33,   /*  5 -> 33 dBm */
                                    31,   /*  6 -> 31 dBm */
                                    29,   /*  7 -> 29 dBm */
                                    27,   /*  8 -> 27 dBm */
                                    25,   /*  9 -> 25 dBm */
                                    23,   /* 10 -> 23 dBm */
                                    21,   /* 11 -> 21 dBm */
                                    19,   /* 12 -> 19 dBm */
                                    17,   /* 13 -> 17 dBm */
                                    15,   /* 14 -> 15 dBm */
                                    13,   /* 15 -> 13 dBm */
                                    11,   /* 16 -> 11 dBm */
                                     9,   /* 17 ->  9 dBm */
                                     7,   /* 18 ->  7 dBm */
                                     5,   /* 19 ->  5 dBm */
                                     5,   /* 20 ->  5 dBm */
                                     5,   /* 21 ->  5 dBm */
                                     5,   /* 22 ->  5 dBm */
                                     5,   /* 23 ->  5 dBm */
                                     5,   /* 24 ->  5 dBm */
                                     5,   /* 25 ->  5 dBm */
                                     5,   /* 26 ->  5 dBm */
                                     5,   /* 27 ->  5 dBm */
                                     5,   /* 28 ->  5 dBm */
                                     5,   /* 29 ->  5 dBm */
                                     5,   /* 30 ->  5 dBm */
                                     5    /* 31 ->  5 dBm */
};

const UBYTE p_control_dcs [32] =
{                                   30,   /*  0 -> 30 dBm */
                                    28,   /*  1 -> 28 dBm */
                                    26,   /*  2 -> 26 dBm */
                                    24,   /*  3 -> 24 dBm */
                                    22,   /*  4 -> 22 dBm */
                                    20,   /*  5 -> 20 dBm */
                                    18,   /*  6 -> 18 dBm */
                                    16,   /*  7 -> 16 dBm */
                                    14,   /*  8 -> 14 dBm */
                                    12,   /*  9 -> 12 dBm */
                                    10,   /* 10 -> 10 dBm */
                                     8,   /* 11 ->  8 dBm */
                                     6,   /* 12 ->  6 dBm */
                                     4,   /* 13 ->  4 dBm */
                                     2,   /* 14 ->  2 dBm */
                                     0,   /* 15 ->  0 dBm */
                                     0,   /* 16 ->  0 dBm */
                                     0,   /* 17 ->  0 dBm */
                                     0,   /* 18 ->  0 dBm */
                                     0,   /* 19 ->  0 dBm */
                                     0,   /* 20 ->  0 dBm */
                                     0,   /* 21 ->  0 dBm */
                                     0,   /* 22 ->  0 dBm */
                                     0,   /* 23 ->  0 dBm */
                                     0,   /* 24 ->  0 dBm */
                                     0,   /* 25 ->  0 dBm */
                                     0,   /* 26 ->  0 dBm */
                                     0,   /* 27 ->  0 dBm */
                                     0,   /* 28 ->  0 dBm */
                                    36,   /* 29 -> 36 dBm */
                                    34,   /* 30 -> 34 dBm */
                                    32    /* 31 -> 32 dBm */
};

const UBYTE p_control_pcs [32] =
{                                   30,   /*  0 -> 30 dBm */
                                    28,   /*  1 -> 28 dBm */
                                    26,   /*  2 -> 26 dBm */
                                    24,   /*  3 -> 24 dBm */
                                    22,   /*  4 -> 22 dBm */
                                    20,   /*  5 -> 20 dBm */
                                    18,   /*  6 -> 18 dBm */
                                    16,   /*  7 -> 16 dBm */
                                    14,   /*  8 -> 14 dBm */
                                    12,   /*  9 -> 12 dBm */
                                    10,   /* 10 -> 10 dBm */
                                     8,   /* 11 ->  8 dBm */
                                     6,   /* 12 ->  6 dBm */
                                     4,   /* 13 ->  4 dBm */
                                     2,   /* 14 ->  2 dBm */
                                     0,   /* 15 ->  0 dBm */
                                     0,   /* 16 ->  0 dBm */
                                     0,   /* 17 ->  0 dBm */
                                     0,   /* 18 ->  0 dBm */
                                     0,   /* 19 ->  0 dBm */
                                     0,   /* 20 ->  0 dBm */
                                     0,   /* 21 ->  0 dBm */
                                    33,   /* 22 -> 33 dBm */
                                    33,   /* 23 -> 33 dBm */
                                    33,   /* 24 -> 33 dBm */
                                    33,   /* 25 -> 33 dBm */
                                    33,   /* 26 -> 33 dBm */
                                    33,   /* 27 -> 33 dBm */
                                    33,   /* 28 -> 33 dBm */
                                    33,   /* 29 -> 33 dBm */
                                    33,   /* 30 -> 33 dBm */
                                    32    /* 31 -> 32 dBm */
};
/*lint +e765 / used by GRR */

GLOBAL void att_calculate_c1 (UBYTE      index)
{
  GET_INSTANCE_DATA;
  SHORT a;
  SHORT b;
  const SHORT *p;
  const UBYTE *p_control;
  SHORT power_offset = 0;
  UBYTE ms_power;
  SHORT offset;

  TRACE_FUNCTION ("att_calculate_c1()");

  ms_power = att_get_power ();
  switch (std)
  {
    case STD_900:
    case STD_EGSM:
    case STD_850:
      /*
       * The table for the power class conversion is GSM 900, E-GSM or GSM 850
       */
      p         = p_gsm;
      /*
       * The table for the MAX_TXPWR_CCCH conversion is GSM 900, E-GSM or GSM 850
       */
      p_control = p_control_gsm;
      /*
       * The gsm_offset parameter can be set by a dynamic configuration command
       * and is not used.
       */
      offset    = rr_data->dyn_config.gsm_offset;
      break;

    case STD_1900:
      p         = p_pcs;
      p_control = p_control_pcs;

      /*
       * The dcs_offset parameter can be set by a dynamic configuration command
       * and is not used.
       */
      offset    = rr_data->dyn_config.dcs_offset;
      break;

    case STD_1800:
      p         = p_dcs;
      p_control = p_control_dcs;

      /* Ref 04.18 Section 10.5.2.35
       * Rest Octets IE includes parameters which are used by the mobile station 
       * for cell selection and reselection purposes. It may also include the 
       * POWER OFFSET parameter used by DCS 1800 Class 3 MS.
       */

      if (ms_power EQ 0x02 AND 
            rr_data->nc_data[index].c2_par.power_off_ind)
      {
        power_offset = rr_data->nc_data[index].c2_par.power_off << 1;
      }

      offset    = rr_data->dyn_config.dcs_offset;
      break;

    case STD_DUAL:
    case STD_DUAL_EGSM:
      /*
       * For dualband mobiles the calculation depends on the channel number
       */
      if (INRANGE(LOW_CHANNEL_1800,rr_data->nc_data[index].arfcn,HIGH_CHANNEL_1800))
      {
        /*
         * All DCS 1800 channels
         */
        p         = p_dcs;
        p_control = p_control_dcs;

        /* Ref 04.18 Section 10.5.2.35 */
        if (ms_power EQ 0x02 AND 
              rr_data->nc_data[index].c2_par.power_off_ind)
        {
          power_offset = rr_data->nc_data[index].c2_par.power_off << 1;
        }

        offset    = rr_data->dyn_config.dcs_offset;
      }
      else
      {
        /*
         * All GSM 900 and E-GSM channels
         */
        p         = p_gsm;
        p_control = p_control_gsm;
        offset    = rr_data->dyn_config.gsm_offset;
      }
      break;

    case STD_DUAL_US:
      if (rr_data->nc_data[index].arfcn < LOW_CHANNEL_1900)
      {
        /*
         * Then it is a GSM 850 channel
         */
        p         = p_gsm;
        p_control = p_control_gsm;
        offset    = rr_data->dyn_config.gsm_offset;
      }
      else
      {
        /*
         * else it is a PCS 1900 channel
         */
        p         = p_pcs;
        p_control = p_control_pcs;

        offset    = rr_data->dyn_config.dcs_offset;
      }
      break;

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT    
    case STD_850_1800:
    case STD_850_900_1800:
      if (INRANGE(LOW_CHANNEL_1800,rr_data->nc_data[index].arfcn,HIGH_CHANNEL_1800))
      {
        /*
         * Then it is a DCS 1800 channel
         */
        p         = p_dcs;
        p_control = p_control_dcs;

        if (ms_power EQ 0x02 AND 
              rr_data->nc_data[index].c2_par.power_off_ind)
        {
          power_offset = rr_data->nc_data[index].c2_par.power_off << 1;
        }

        offset    = rr_data->dyn_config.dcs_offset;
      }
      else
      {
        /*
         * Else it is a GSM 900 and E-GSM or GSM 850 channel
         */
        p         = p_gsm;
        p_control = p_control_gsm;
        offset    = rr_data->dyn_config.gsm_offset;
      }
      break;

    case STD_900_1900:
    case STD_850_900_1900:
      /*
       * For dualband mobiles the calculation depends on the channel number
       */
      if (INRANGE(LOW_CHANNEL_1900,rr_data->nc_data[index].arfcn,HIGH_CHANNEL_1900))
      {
        /*
         * All PCS 1900 channels
         */
        p         = p_pcs;
        p_control = p_control_pcs;

        offset    = rr_data->dyn_config.dcs_offset;
      }
      else
      {
        /*
         * All GSM 900 and E-GSM or GSM 850 channels
         */
        p         = p_gsm;
        p_control = p_control_gsm;
        offset    = rr_data->dyn_config.gsm_offset;
      }
      break;
#endif

    default:
      /*
       * Just for LINT
       */
      rr_data->nc_data[index].c1 = 0;
      return;
  }

  /*
   * Parameter A indicates the difference between current fieldstrength and
   * the minimum expected from the infrastructure. It indicates how good is the
   * receiving part.
   */
#ifdef GPRS
  if(rr_data->gprs_data.use_c31)
  {
     a = (SHORT) rr_data->nc_data[index].rxlev -
       (SHORT) rr_data->nc_data[index].cr_par.gprs_rxlev_access_min;
  }
  else
  {
#endif
    a = (SHORT) rr_data->nc_data[index].rxlev -
      (SHORT) rr_data->nc_data[index].select_para.rxlev_access_min;
#ifdef GPRS
  }
#endif

  /*
   * Parameter B indicates the difference between the maximum sending power
   * expected by the infrastructure and the maximum power which is possible by
   * the mobile. In the formula the maximum of B and zero is taking in account.
   * If B is negative, the MS has enough power to reach the base station,
   * because it has more power than expected and it will not have any affect
   * in the C1 calculation. If B is positive, the MS will send with less power
   * than expected by the infrastructure. Then this difference is subtracted
   * from A. The maximum power is expected at the boarder of the cell. If the
   * mobile is nearer to the base station, a lower power is needed. On the
   * other hand will the mobile receive the base station with a higher
   * fieldstrength as on the boarder. So this surplus compensates the lower
   * sending capabilities.
   */

#ifdef GPRS
  if(rr_data->gprs_data.use_c31)
  {
    b = p_control [rr_data->nc_data[index].cr_par.gprs_ms_txpwr_max_cch] - p[ms_power];
  }
  else
  {
#endif
     b = p_control [rr_data->nc_data[index].select_para.ms_txpwr_max_cch] +
       power_offset - p[ms_power];
#ifdef GPRS
  }
#endif

  /*
   * The resulting path loss criterion is C1 = A - MAX (B,0)
   *
   * An additional C1_offset and offset is not really used. This values can
   * be configured and may compensate RF problems.
   */
  rr_data->nc_data[index].c1 = a - att_max (b, 0) + rr_data->c1_offset +
       offset;

  /*
   * To avoid too many reselection the C1 of a neighbourcell is decreased,
   * that means beside the GSM specifications a threshold value is introduced.
   * If a test SIM card is inserted, this is not used.
   */
  if (index NEQ SC_INDEX AND index NEQ CR_INDEX AND !dat_test_sim_available())
    rr_data->nc_data[index].c1 -= TWO;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_calculate_c2           |
+--------------------------------------------------------------------+

  PURPOSE : The reselection criterion C2 is used for cell reselection
            only and is defined by:

            C2 = C1 + CELL_RESELECT_OFFSET -
                          TEMPORARY OFFSET * H(PENALTY_TIME - T)

            for PENALTY_TIME <> 11111

            C2 = C1 - CELL_RESELECT_OFFSET

            for PENALTY_TIME = 11111

            where

            For non-serving cells:

            H(x) = 0 for x < 0

                 = 1 for x = 0

            For serving cells:

            H(x) = 0

            T is a timer implemented for each cell in the list of strongest carriers.
            T shall be started from zero at the time the cell is placed by the MS on the
            list of strongest carriers, except when the previous serving cell is placed
            on the list of strongest carriers at cell reselection. In this, case,
            T shall be set to the value of PENALTY_TIME (i.e. expired).
            CELL_RESELECT_OFFSET applies an offset to the C2 reselection criterion for that cell.

            NOTE: CELL_RESELECT_OFFSET may be used to give different priorities to different
            bands when multiband operation is used.

            TEMPORARY_OFFSET applies a negative offset to C2 for the duration of PENALTY_TIME
            after the timer T has started for that cell.

            PENALTY_TIME is the duration for which TEMPORARY_OFFSET applies The all ones bit
            pattern on the PENALTY_TIME parameter is reserved to change the sign of
            CELL_RESELECT_OFFSET and the value of TEMPORARY_OFFSET is ignored as indicated
            by the equation defining C2.

            CELL_RESELECT_OFFSET, TEMPORARY_OFFSET, PENALTY_TIME and CELL_BAR_QUALIFY are
            optionally broadcast on the BCCH of the cell. If not broadcast, the default values
            are CELL_BAR_QUALIFY = 0, and C2 = C1. The use of C2 is described in GSM 03.22.

            These parameters are used to ensure that the MS is camped on the cell with which it
            has the highest probability of successful communication on uplink and downlink.

  REFERENCE: GSM 5.08, chapter 6.4 Criteria for cell selection and reselection

*/

static void att_calculate_c2 (UBYTE index)
{
  GET_INSTANCE_DATA;
  USHORT     h;
  T_NC_DATA *rrd;

  TRACE_FUNCTION ("att_calculate_c2()");

  /*
   * get a pointer to the data of the cell
   */
  rrd = &rr_data->nc_data[index];

  /*
   * Calculate C1 as a base if no C2 parameters are available
   */
  att_calculate_c1 (index);

  if ( rrd->c2_par.param_ind EQ 0 )
  {
    /*
     * C2 is equal C1 if no C2 parameters are available.
     */
    rrd->c2 = rrd->c1;
    TRACE_C1_C2(index);
    return;
  }

  /*
   * Special case penalty time = 0b11111
   */
  if (rrd->c2_par.penalty_time EQ 31)
  {
    rrd->c2 = rrd->c1 - 2 * rrd->c2_par.cell_reselect_offset;
    TRACE_C1_C2(index);
    return;
  }

  /*
   * Calculate x for H(x) and set H(x)
   */
  if ((USHORT)((rrd->c2_par.penalty_time + 1) * 20) >=
      (USHORT)(rrd->avail_time/PERIOD_1_SEC))
    h = 1;
  else
    h = 0;

  /*
   * for the serving cells H(x) us ever zero.
   */
  if (index EQ SC_INDEX)
    h = 0;

  /*
   * Calculate C2 using the formula
   */
  rrd->c2 = rrd->c1 + 2 * rrd->c2_par.cell_reselect_offset -
                               10 * rrd->c2_par.temp_offset * h;
  if (h EQ 1 AND rrd->c2_par.temp_offset EQ 7)
  {
    /*
     * temp offset = 7 means C2 = - infinite
     */
    rrd->c2 = -127;
  }

  TRACE_C1_C2(index);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_calculate_c2_diff      |
+--------------------------------------------------------------------+

  PURPOSE : For cell reselection decision the C2 difference between
            a neighbourcell and the serving cell is calculated. This
            function shall be called only if both cells are in a
            different location area.

            If no SIM card is inserted (that means limited service)
            the threshold value is not taken in account. This is
            beside the GSM specifications and shall ensure a better
            coverage of the mobile. It leads not to more signalling
            on the air-interface.

  REFERENCE: GSM 5.08, chapter 6.6.2 Path loss criteria and timings for
                                     cell re-selection

*/

static SHORT att_calculate_c2_diff (UBYTE index)
{
  GET_INSTANCE_DATA;
  SHORT delta;

  TRACE_FUNCTION ("att_calculate_c2_diff()");

  /*
   * calculate delta
   */
  delta = rr_data->nc_data[index].c2 - rr_data->nc_data[SC_INDEX].c2;

  /* According to 3GPP 05.08 Sec 6.8 MS is supposed to use ZERO for 
   * CELL_RESELECT_HYSTERESIS when in Limited service.
   * c)	The MS shall perform cell reselection at least among the cells of
   * the PLMN of the cell on which the MS has camped, according to the 
   * algorithm of 3GPP TS 03.22, except that a zero value of 
   * CELL_RESELECT_HYSTERESIS shall be used.
   */

  if ((rr_data->ms_data.imsi_available) AND
      (rr_data->ms_data.rr_service NEQ LIMITED_SERVICE))
    delta -= 2*rr_data->nc_data[SC_INDEX].select_para.cell_resel_hyst;

  /*
   * return the difference
   */
  return delta;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_calculate_digits       |
+--------------------------------------------------------------------+

  PURPOSE : The function calculates the number of the digits in a
            mobile identity infoelement. The input parameter contains
            one BCD digit in each field entry.

*/

static UBYTE att_calculate_digits (UBYTE *digits)
{
  UBYTE i = 0;

  TRACE_FUNCTION ("att_calculate_digits()");

  /*
   * Check whether the end is detected
   */
  while (digits[i] < 0x0A AND i < 16)
    i++;

  return i;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_cell_barred_status_cr_no_cr  |
+--------------------------------------------------------------------+

  PURPOSE : Checks whether a cell is barred or not for cell reselection
            purposes.

            CELL_BAR QUALIFY   CELL_BAR ACCESS   CS priority    Status for CR
                0                     0             normal          normal
                0                     1             barred          barred
                1                     0             low             normal (see note 2)
                1                     1             low             normal (see note 2)

            If all the following conditions are met, then the "Cell selection priority"
            and the "Status for cell reselection" shall be set to normal:

            - the cell belongs to the MS HPLMN;
            - the MS is in cell test operation mode;
            - the CELL_BAR_ACCESS is set to "1";
            - the CELL_BAR_QUALIFY is set to "0";
            - the Access Control class 15 is barred.

            NOTE 1: A low priority cell is only selected if there are no
                    suitable cells
                    of normal priority (see GSM 03.22).
            NOTE 2: Two identical semantics are used for cross phase
                    compatibility reasons.
                    This allows an operator to declare a cell always as a low
                    priority one for a phase 2 MS, but keeps the opportunity
                    for an operator to decide wether a phase 1 MS is permitted
                    to camp on such a cell or not.

  REFERENCE: GSM 5.08, chapter 9, table 1a


*/
GLOBAL BOOL att_cell_barred_status_cr_no_cr (UBYTE index)
{
  GET_INSTANCE_DATA;
 
  TRACE_FUNCTION ("att_cell_barred_status_cr_no_cr()");

  TRACE_EVENT_P5 ("[%u]%u cbq=%u cba=%u access class=%04x",
    rr_data->nc_data[index].arfcn,
    index,
    rr_data->nc_data[index].c2_par.cbq,
    rr_data->nc_data[index].rach.cell_bar_access,
    rr_data->nc_data[index].rach.ac);

  /*
   * check first the special case
   */
  if (dat_hplmn (rr_data->nc_data[index].lai.mcc,
                 rr_data->nc_data[index].lai.mnc) AND
      rr_data->cell_test_operation AND
      rr_data->nc_data[index].c2_par.cbq EQ 0 AND
      rr_data->nc_data[index].rach.cell_bar_access EQ 1)
  {
    /*
     * access class 15 is set
     */
    if (rr_data->nc_data[index].rach.ac & 0x8000)
      return FALSE;
  }

  /*
   * the cell reselection status is normal. According
   * to the table above the cell is only barred if
   * the cell bar qualifier is set to 0 and the
   * cell bar access is set to 1.
   */
  if((rr_data->nc_data[index].rach.cell_bar_access EQ BARRED_YES) AND
      rr_data->nc_data[index].c2_par.cbq EQ CBQ_NO)
      return TRUE;
  else
      return FALSE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_cell_in_data_structures|
+--------------------------------------------------------------------+

  PURPOSE : After reception of new measurement values from layer 1
            this function checks whether a neighbourcell indicated
            with the parameter index is stored inside the RR
            storage area.

*/

static BOOL att_cell_in_data_structures (T_MPH_MEASUREMENT_IND *report,
                                         UBYTE                  index)
{
  GET_INSTANCE_DATA;
  USHORT i;

  TRACE_FUNCTION ("att_cell_in_data_structures()");

  /*
   * check for all areas
   */
  for (i = 0; i < 6; i++)
  {
    /*
     * If a cell is stored in the data area
     */
    if (rr_data->nc_data[i].bcch_status NEQ EMPTY)
    {
      /*
       * check the channel number
       */
      if (rr_data->nc_data[i].arfcn EQ report->ncells.arfcn[index])
        return TRUE;
    }
  }
  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)     MODULE  : RR_ATT                       |
| STATE   : code              ROUTINE : att_remove_multiple_channels |
+--------------------------------------------------------------------+

  PURPOSE : During fieldtests it had happens that the measurements from
            layer 1 may contain the serving cell as a neighbourcell
            (which is possible during connection and can be forwarded
             to RR after connection). This leads to double stored data
            in RR.

*/

GLOBAL void att_remove_multiple_channels (void)
{
  GET_INSTANCE_DATA;
  USHORT i;

  TRACE_FUNCTION ("att_remove_multiple_channels()");

  /*
   * For all neighbourcells in the measurement report
   */
  for (i = 0; i < 6; i++)
  {
    if (rr_data->nc_data[i].arfcn EQ rr_data->nc_data[SC_INDEX].arfcn)
    {
      /*
       * If the neighbourcell channel number is equal to the serving
       * cell, clear the whole storage area.
       */
      rr_data->nc_data[i].arfcn       = 0;
      rr_data->nc_data[i].bcch_status = EMPTY;
    }
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_cell_in_measure_report |
+--------------------------------------------------------------------+

  PURPOSE : The function checks whether a cell in the RR storage
            area is still in a new measurement report and returns
            one of the three possibilities:

            CELL_IS_INSERTED      the cell is inside with the same BSIC
            BSIC_HAS_CHANGED      the cell is inside, but the BSIC has changed
            CELL_IS_NOT_INSERTED  the cell is not longer inside.

*/

static UBYTE att_cell_in_measure_report (T_MPH_MEASUREMENT_IND *report,
                                         UBYTE                  index)
{
  GET_INSTANCE_DATA;
  UBYTE  i;
  UBYTE  bsic;

  TRACE_FUNCTION ("att_cell_in_measure_report()");

  /*
   * check all neighbourcells in the measurent report
   */
  for (i = 0; i < report->ncells.no_of_ncells; i++)
  {
    /*
     * Check first the channel number
     */
    if (rr_data->nc_data[index].arfcn EQ report->ncells.arfcn[i])
    {
      /*
       * Update fieldstrength value in the RR storage area
       */
      rr_data->nc_data[index].rxlev = report->ncells.rx_lev[i];

      /*
       * Check whether the BSIC has been changed. Layer 1 sends an
       * 8 bit value, but only the lower 6 bits are significant.
       */
      bsic = rr_data->nc_data[index].bsic & 0x3F;
      if (bsic EQ (report->ncells.bsic[i] & 0x3F))
        return CELL_IS_INSERTED;
      else
      {
        /*
         * If it is a changed BSIC, update the value in the RR storage area.
         */
        rr_data->nc_data[index].bsic = report->ncells.bsic[i] & 0x3F;
        return BSIC_HAS_CHANGED;
      }
    }
  }
  /*
   * Not found
   */
  return CELL_IS_NOT_INSERTED;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_check_bcch_carrier_cco |
+--------------------------------------------------------------------+

  PURPOSE : During a Cell Change Order procedure:
            After reception of all needed system information messages
            a carrier is checked for cell selection purposes.

*/

#ifdef GPRS
LOCAL void att_check_bcch_carrier_cco (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_check_bcch_carrier_cco()");

  dat_rrgrr_data_ind (rr_data->gprs_data.dl_data_ind);
  rr_data->gprs_data.dl_data_ind = NULL;
  att_build_idle_req (CR_INDEX, MODE_CELL_SELECTION);
  SET_STATE (STATE_ATT, ATT_IDLE);
  dat_att_cell_selected();
  if (att_gprs_is_avail())
  {
    att_signal_gprs_support();
  }
  att_start_registration_timer ();
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_check_bcch_carrier_si  |
+--------------------------------------------------------------------+

  PURPOSE : After reception of all needed system information messages
            a carrier is checked for cell selection purposes.
            Perform the check on the presence of the SIs.

*/

GLOBAL void att_check_bcch_carrier_si (void)
{
  GET_INSTANCE_DATA;
  if (rr_data->cr_data.cd.sys_info_read EQ ALL_SYS_INFO_READ)
  {
    att_check_bcch_carrier ();
  }
}

GLOBAL void att_store_plmn_in_found_list (T_loc_area_ident *lai)
{
  GET_INSTANCE_DATA;
  T_FOUND_ELEMENT *found;

/* Implements RR Clone findings #8 */
  BOOL cell_ok = !att_cell_barred_status_cr_no_cr(CR_INDEX) AND
                 (rr_data->nc_data[CR_INDEX].c1 > 0) AND
                  att_priority_check ();

  if ( (found = att_plmn_in_found_list (lai->mcc, lai->mnc)) NEQ NULL )
  {
    if ( found->cell_ok  EQ 0 AND   /* old cell did not allow access */
                cell_ok NEQ 0 )     /* new cell does    allow access */
    {
      ;    /* a better cell has been found */
    }
    else
    {
      found = NULL;
    }
  }
  else
  if ( rr_data->sc_data.found_entries < MAX_PLMN AND att_check_network(lai) )
  {
    found = &rr_data->sc_data.found[rr_data->sc_data.found_entries];
    rr_data->sc_data.found_entries++;
  }
  else
  {
    TRACE_EVENT ("cannot store PLMN" );
  }

  if ( found )
  {
    att_save_found_plmn (
      found,
      lai->mcc, lai->mnc,
      rr_data->nc_data[CR_INDEX].arfcn,
      rr_data->nc_data[CR_INDEX].rxlev, lai->lac, (UBYTE)cell_ok);
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_check_bcch_carrier     |
+--------------------------------------------------------------------+

  PURPOSE : After reception of all needed system information messages
            a carrier is checked for cell selection purposes.

*/

GLOBAL void att_check_bcch_carrier (void)
{
  GET_INSTANCE_DATA;
  T_loc_area_ident * lai;
  BOOL  plmn_ok = FALSE;
  BOOL  cell_ok,prio_ok;
  UBYTE tried_to_reach_full_try_limited = (rr_data->cs_data.scan_mode EQ CS_SECOND_SCAN);

  TRACE_FUNCTION ("att_check_bcch_carrier()");

  /*
   * stop the timer which controls the reception of
   * all system information messages during cell selection.
   */
  TIMERSTOP (T_RESELECT);

  /*
   * If procedure RR Network Controlled Cell Change Order
   * is running then call a different function and return.
   */
#ifdef GPRS
  if ( rr_data->gprs_data.tbf_est EQ TBF_EST_CCO )
  {
    att_check_bcch_carrier_cco();
    return;
  }
#endif

  lai = &rr_data->nc_data[CR_INDEX].lai;

  /*
   * calculate the path loss criterion
   */
  att_calculate_c1 (CR_INDEX);
  TRACE_C1(CR_INDEX);

  /*
   * the check depends on the service which has been requested
   * by RR. This can be the same as the service requested by MM,
   */
  switch (rr_data->ms_data.req_mm_service)
  {
    case FUNC_PLMN_SRCH:
      /*
       * For full service the mobile country code and the mobile network code
       * must match with the PLMN identification requested by MM.
       */
      plmn_ok = dat_plmn_equal_req (lai->mcc,
                                    lai->mnc,
                                    rr_data->ms_data.plmn.mcc,
                                    rr_data->ms_data.plmn.mnc);

      TRACE_EVENT_P6 ( "MCC/MNC r=%x%x%x/%x%x%x",
        rr_data->ms_data.plmn.mcc[0],
        rr_data->ms_data.plmn.mcc[1],
        rr_data->ms_data.plmn.mcc[2],
        rr_data->ms_data.plmn.mnc[0],
        rr_data->ms_data.plmn.mnc[1],
        rr_data->ms_data.plmn.mnc[2]);
      TRACE_EVENT_P7 ( "MCC/MNC i=%x%x%x/%x%x%x %s",
        lai->mcc[0],
        lai->mcc[1],
        lai->mcc[2],
        lai->mnc[0],
        lai->mnc[1],
        lai->mnc[2],
        plmn_ok?"OK":"");

       
       if (!plmn_ok AND 
           (GET_STATE(STATE_ATT) EQ ATT_CS2) AND
           (rr_data->ms_data.rr_service EQ LIMITED_SERVICE OR 
            rr_data->ms_data.rr_service EQ NO_SERVICE) AND 
           (rr_data->cs_data.check_hplmn EQ TRUE) AND
           (((rr_data->ms_data.operation_mode >> SHIFT_FOR_SEARCH_OFFSET) & 1) EQ M_AUTO))
         {      
           /* Compare with HPLMN */
           if (rr_data->ms_data.imsi_available) 
           {
             plmn_ok = dat_plmn_equal_req (lai->mcc, 
                                           lai->mnc,
                                           &rr_data->ms_data.imsi.ident_dig[0],
                                           &rr_data->ms_data.imsi.ident_dig[3]);
             if (plmn_ok)
             {
               TRACE_EVENT ("HPLMN Matched");
             }
           }
           /* compare with AHPLMN */
           if(!plmn_ok AND rr_data->ms_data.ahplmn.v_plmn EQ TRUE)
           {
             plmn_ok = dat_plmn_equal_req (lai->mcc, 
                                           lai->mnc,
                                           rr_data->ms_data.ahplmn.mcc,
                                           rr_data->ms_data.ahplmn.mnc);
             if (plmn_ok)
             {
               TRACE_EVENT ("AHPLMN Matched");
             }
           }
           /* Compare with LRPLMN */
           if (!plmn_ok AND rr_data->cs_data.last_reg_plmn.v_plmn EQ V_PLMN_PRES)
           {
             plmn_ok = dat_plmn_equal_req (lai->mcc, 
                                           lai->mnc,
                                           rr_data->cs_data.last_reg_plmn.mcc,
                                           rr_data->cs_data.last_reg_plmn.mnc);
             if (plmn_ok)
             {
               TRACE_EVENT ("LRPLMN Matched");
             }
           }
          /* Compare with Last Serving Cell PLMN from White List */
           if ((!plmn_ok) AND (rr_data->sc_data.mm_started EQ RR_ORIGINATED) AND               
               (rr_data->ms_data.plmn.v_plmn EQ V_PLMN_PRES))               
           {
             plmn_ok = dat_plmn_equal_req (lai->mcc,
                                           lai->mnc,
                                           rr_data->cs_data.white_list.last_sc_lac.mcc,
                                           rr_data->cs_data.white_list.last_sc_lac.mnc);
             if (plmn_ok)
             {         
               TRACE_EVENT ("White List PLMN Matched");               
             }
           }
         }
             
      if (plmn_ok)
        tried_to_reach_full_try_limited = FALSE;
      /*lint -fallthrough*/

    case FUNC_LIM_SERV_ST_SRCH:
      if((rr_data->ms_data.req_mm_service EQ FUNC_LIM_SERV_ST_SRCH) OR
         tried_to_reach_full_try_limited)
      {
        /*
         * The mobile shall reach limited service. This is the case
         * if no full service is possible or no SIM card is inserted.
         *
         * The network check is in principle ever true. The function
         * att_check_network realises a SW shielding to reject public
         * networks in final type approvals.
         *
         * The criterion for the cell checking is a non-barred cell and
         * a positive path loss criterion.
         */
        plmn_ok = att_check_network (lai);
/* Implements RR Clone findings #8 */
        cell_ok = !att_cell_barred_status_cr_no_cr (CR_INDEX) AND
                  (rr_data->nc_data[CR_INDEX].c1 > 0);

      }
      else
      {
        /*
         * Full service is requested.
         *
         * First store PLMN of the cell in the PLMN found list
         * if the list is not stored yet and not full. SW shielding
         * is taking into account for FTA purposes.
         *
         * If the cell selection fails and no carrier was suitable,
         * at least a list of the found channels is available.
         */
         if(dat_forb_lai_check (CR_INDEX))
         {
            att_store_plmn_in_found_list (lai);
         }
         else
         {
           TRACE_EVENT ("Do not store Forbidden LAI PLMN in the found PLMN list");
         }

        /*
         * The requirements for the cell are: it must be a non-barred cell,
         * it must have a positive path loss criterion. The cell shall not
         * be inside of a forbidden location area and it must be a high priority
         * cell if it is the first scan first attempt.
         */
/* Implements RR Clone findings #8 */
        cell_ok = !att_cell_barred_status_cr_no_cr (CR_INDEX) AND
                  (rr_data->nc_data[CR_INDEX].c1 > 0);
    
                  
        prio_ok = att_priority_check ();

        TRACE_EVENT_P1 ( "cell_ok     = %d", cell_ok);

        TRACE_EVENT_P1 ( "prio_ok     = %d", prio_ok);
        /*
         * if the network and cell are okay, but the priority check fails,
         * this cell is a low priority cell and can be checked in the
         * first scan second attempt.
         */
        if (plmn_ok AND cell_ok AND !prio_ok)
          cs_set_attributes (LOW_PRIORITY_CELL,
                             rr_data->nc_data[CR_INDEX].arfcn);

        /*
         * if the bcch carrier is not suitable for full service,
         * but suitable for limited service, try it
         * for emergency services in the second attempt.
         * priority check is not required for emergency cells
         */
        if (cell_ok AND !plmn_ok)
          cs_set_attributes (EMERGENCY_CELL,
                             rr_data->nc_data[CR_INDEX].arfcn);

        /* Update the cell_ok flag with with priority check flag */
        if(cell_ok)
          cell_ok = prio_ok;

        /* Checking access class and the list for cause #12 for */
        /* selection of the cell to camp on may not meet the    */
        /* requirements of GSM 03.22 subclause 3.5.3, 3.5.4.    */
        /* However, it is perfectly correct to consider the     */
        /* forbidden location areas for roaming list here.      */


        /*
         * If cell selection has been launched by MM in manual mode
         * we don't check against the forbidden location area lists.
         * This is to allow the user to select a network manually
         * which is a member of a forbidden list.
         * Maybe the distinction manual/automatic is not worth the
         * effort, as in contradiction to the forbidden list for
         * cause #11 the user can get rid of the forbidden lists for
         * cause #12 and #13 either by a power cycle or by simply
         * waiting some hours.
         */
        if (!rr_data->sc_data.mm_started OR
            ((rr_data->ms_data.operation_mode >> SHIFT_FOR_SEARCH_OFFSET) & 1) EQ M_AUTO)
        {
          /*
           * RR started cell selection, or MM started but in automatic
           * so ensure no barred LA is entered.
           */
          if (cell_ok)
          {
            cell_ok = (dat_forb_lai_check (CR_INDEX) AND
                       dat_roam_forb_lai_check (CR_INDEX));
            
            /* Candidate for Limited service */
            if(!cell_ok)
              cs_set_attributes (EMERGENCY_CELL,
                                rr_data->nc_data[CR_INDEX].arfcn);
          }
        }

  #if defined (_SIMULATION_)
        {
          /*
           * Trace output for cell selection analysis
           */
/* Implements RR Clone findings #8 */
          BOOL cb_stat = att_cell_barred_status_cr_no_cr (CR_INDEX);
     
          BOOL for_lai = dat_forb_lai_check (CR_INDEX);
          BOOL prio    = att_priority_check();
          TRACE_EVENT_WIN_P1 ( "CB_STAT     = %d", cb_stat);
          TRACE_EVENT_WIN_P1 ( "C1          = %d", rr_data->nc_data[CR_INDEX].c1);
          TRACE_EVENT_WIN_P1 ( "FOR_LAI     = %d", for_lai);
          TRACE_EVENT_WIN_P1 ( "PRIO        = %d", prio);
        }
  #endif  /* _SIMULATION_ */
      }
      break;

    default:
      /*
       * MM has requested the list of the available PLMNs. The algorithm
       * for this is to indicate all carriers as "not suitable" for cell
       * selection. As a result the list of the available PLMNs is build
       * after scanning all carriers.
       *
       * First step is to store PLMN in plmn found list if the PLMN is not
       * stored yet and the list is not full. SW shielding is used for
       * FTA purposes.
       */
      att_store_plmn_in_found_list (lai);

      /*
       * Define the channel as a candidate for limited service.
       */
      cs_set_attributes (EMERGENCY_CELL,
                         rr_data->nc_data[CR_INDEX].arfcn);

      /*
       * Remove all channels of the neighbourcell list from the power campaign
       * list. If the configuration of the network is sensible, this cell will
       * contain also channels of the same network. So it is not necessary to
       * scan this channels. The available PLMN list is the same, it consumes
       * only time.
       */
      cs_del_list (&rr_data->cr_data.cd.ncell_list);
      plmn_ok = FALSE;
      cell_ok = FALSE;

      /*
       * start synchronisation attempt to the next cell
       */
      memset (&rr_data->nc_data[CR_INDEX], 0, sizeof (T_NC_DATA));

      if (rr_data->ms_data.rr_service EQ NO_SERVICE)
      {
        SET_STATE (STATE_ATT, ATT_CS1);
      }
      srv_clear_list (&rr_data->cr_data.cd.ncell_list);
      cs_sync_next ();
      return;
  }

  if( (!tried_to_reach_full_try_limited) AND (rr_data->cs_data.scan_mode EQ CS_SECOND_SCAN))
  {
    if(!cell_ok)
    {
      /*
       * plmn is OK, cell is not OK for full service
       * however cell may be OK for limited service
       */
      tried_to_reach_full_try_limited = TRUE; 
/* Implements RR Clone findings #8 */
      cell_ok = !att_cell_barred_status_cr_no_cr (CR_INDEX) AND
                  (rr_data->nc_data[CR_INDEX].c1 > 0);

    }    
  }

  /*
   * Trace output for cell selection analysis
   */
  TRACE_EVENT_P1 ( "req.service = %s", _rr_str_FUNC[rr_data->ms_data.req_mm_service]);
  TRACE_EVENT_P1 ( "plmn_ok     = %d", plmn_ok );
  TRACE_EVENT_P1 ( "cell_ok     = %d", cell_ok);

  /*
   * checking of the requirements has been done. The result
   * is defined by plmn_ok and cell_ok.
   */
  if (plmn_ok AND cell_ok)
  {
    /*
     * Network and Cell are okay. Just use the cell
     * set RR service according to the checked service
     */
    /* If the MS reselects back onto the same SC and T3122 is still running, it should */
    /* be left running. If a new cell is selected, this timer should be stopped. */

  if( rr_data->nc_data[SC_INDEX].arfcn NEQ rr_data->nc_data[CR_INDEX].arfcn)
    {
      TIMERSTOP(T3122);
    }

    if (rr_data->ms_data.req_mm_service EQ FUNC_LIM_SERV_ST_SRCH OR
        (tried_to_reach_full_try_limited))
    {
      rr_data->ms_data.rr_service = LIMITED_SERVICE;
    }
    else
    {
      rr_data->ms_data.rr_service = FULL_SERVICE;
    }

    TRACE_EVENT_P4 ("NEW SC [%d]->[%d] (rMM=%s RRs=%s)",
      rr_data->nc_data[SC_INDEX].arfcn,
      rr_data->nc_data[CR_INDEX].arfcn,
      _rr_str_FUNC[rr_data->ms_data.req_mm_service],
      _rr_str_SERVICE[rr_data->ms_data.rr_service]);

    /*
     * set the result in the CR_INDEX area and copy then all
     * stuff to the SC_INDEX area.
     */
    rr_data->nc_data[CR_INDEX].bcch_status  = DECODED;
    rr_data->nc_data[CR_INDEX].bcch_counter = 0;
    rr_data->nc_data[CR_INDEX].c1_counter   = 0;
    att_copy_cr_data ();
    memcpy (&rr_data->sc_data.cd, &rr_data->cr_data.cd,
            sizeof (T_CELL_DATA));
    rr_data->nc_data[CR_INDEX].arfcn = NOT_PRESENT_16BIT;
#if defined (REL99) && defined (TI_PS_FF_EMR)
    rr_data->sc_data.ba_index  = rr_data->cr_data.ba_index;
    rr_data->cr_data.ba_index = NOT_PRESENT_8BIT;
#endif

    /* CSI-LLD section:4.1.1.10 
     * This function updates the black list after the first successful 
     * FUNC_PLMN_SEARCH 
     */ 
    if(rr_data->cs_data.initial_plmn_search EQ INITIAL_PLMN_SEARCH_ACTIVE)
    { 
      cs_update_black_list();
      rr_data->cs_data.initial_plmn_search = INITIAL_PLMN_SEARCH_DONE;
    }
    
    /* Full service reached. Perform all necessary tasks */
    if(rr_data->ms_data.rr_service EQ FULL_SERVICE)
    {
      att_full_service_found();
    }

    /*
     * Indicate the result of cell selection to MM
     */
    if (tried_to_reach_full_try_limited)
    {
      /*
       * The result is not what MM has requested.
       * E.g. MM has requested full service, but RR has
       * reached only limited service. In this case
       * MM gets the list of the available PLMNs and may
       * select on of the networks for a new attempt in
       * case of automatic registration.
       */
      att_copy_old_lai_rac(CR_INDEX);
#if 0
      memcpy (&rr_data->old_lai, &rr_data->nc_data[CR_INDEX].lai,
              sizeof(T_loc_area_ident));
      rr_data->old_cell_id = rr_data->nc_data[CR_INDEX].cell_id;
#endif

#ifdef GPRS
      if(att_gprs_is_avail())
      { /* we have GPRS and have reached limited service */
        cs_set_stop_active();
        if(att_gprs_cell_has_pbcch())
        {
          att_gprs_stop_pl();
        }
        else
        {
          att_set_pl_in_idle_mode ();
        }
        att_signal_gprs_support();
        /* and wait for CR_REQ(CR_COMPLETE) from GRR */
      }
      else
      {
#endif
        /*
         * Configure layer 1
         */
        att_set_pl_in_idle_mode ();

        /*
         * Indicated the data transfer process that the MS is attached.
         */
        dat_att_cell_selected ();

        SET_STATE (STATE_ATT, ATT_IDLE);
        cs_set_stop_active();

        att_code_rr_abort_ind (RRCS_ABORT_CEL_SEL_FAIL);
        att_start_registration_timer ();
        att_mph_identity_req ();
        srv_use_stored_prim ();
#ifdef GPRS
      }
#endif

    }
    else
    {
      /*
       * the cell selection was successful. The counter
       * for failed registration attempts is resetted and
       * MM is informed.
       */

      rr_data->ms_data.reg_counter = 0;
#ifdef GPRS
      if(! att_gprs_is_avail())
      {
#endif

        /*
         * Configure layer 1
         */
        att_set_pl_in_idle_mode ();
        /*
         * Indicated the data transfer process that the MS is attached.
         */
        dat_att_cell_selected ();
        SET_STATE (STATE_ATT, ATT_IDLE);
        cs_set_stop_active();

        /* inform GRR if we have GPRS support */
#ifdef GPRS
        if(att_gprs_is_avail())
        {
          att_signal_gprs_support();
        }
#endif
        /* Inform MM */
        if (rr_data->sc_data.mm_started)
          att_code_rr_act_cnf ();
        else
          att_code_rr_act_ind ();

        srv_use_stored_prim ();
        att_start_registration_timer ();
        att_mph_identity_req ();

#ifdef GPRS
      }
      else
      { /* we have GPRS and have reached full service */
        cs_set_stop_active();
        if(att_gprs_cell_has_pbcch())
        {
          att_gprs_stop_pl();
        }
        else
        {
          att_set_pl_in_idle_mode ();
        }
        att_signal_gprs_support();
        /* and wait for CR_REQ(CR_COMPLETE) from GRR */
      }
#endif
      /* This flag will help to identify whether cell reselection following 
       * TRESELECT expiry waiting for SI2TER is successfull or not 
       */
      rr_data->cr_treselect_exp = TRUE;
    }
    return;
  }

  /*
   * BCCH carrier check has failed
   * Either cell_ok or plmn_ok is FALSE or both.
   */

  if(plmn_ok EQ FALSE)
  {
    /*
     *  PLMN is not okay, but the cell
     */
    if (rr_data->ms_data.imsi_available AND
       (dat_test_sim_available () EQ FALSE))
    {
     /*
      * with SIM card, that means full service.
      * "delete neighbourcells", that means give all
      * neighbourcells a low priority in scanning.
      *
      * It is assumed that this cell will fail also
      * because they are also member of the wrong network.
      *
      * This is not done during FTA campaign.
      */
      cs_del_list (&rr_data->cr_data.cd.ncell_list);
    }
  }

  if(plmn_ok AND 
     cell_ok EQ FALSE)
  {
    /*
     * plmn okay but not the cell
     */
    if (rr_data->ms_data.imsi_available AND
         dat_test_sim_available() EQ FALSE)
    {
      /*
       * with SIM card, that means full service.
       * "set neighbourcells", that means give all
       * neighbourcells a higher priority in scanning.
       * Not done during FTA campaign
       */
      cs_set_list (&rr_data->cr_data.cd.ncell_list);
    }
  }

  /*
   * start synchronisation attempt to the next cell
   */
  memset (&rr_data->nc_data[CR_INDEX], 0, sizeof (T_NC_DATA));

 /*
  * After BCCH Reading if the requested service is not found,
  * before sending BSIC_REQ RR has to be moved to ATT_CS1.
  */

  if(rr_data->ms_data.rr_service EQ NO_SERVICE OR
     GET_STATE(STATE_ATT) EQ ATT_CS2)
  {
    SET_STATE (STATE_ATT, ATT_CS1);
  }

  srv_clear_list (&rr_data->cr_data.cd.ncell_list);
#ifdef GPRS
  if(GET_STATE(STATE_ATT) NEQ ATT_IDLE)
    gprs_init_data_cr();
#endif
  cs_sync_next ();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_check_fplmn_cell       |
+--------------------------------------------------------------------+

  PURPOSE : If the mobile is in limited service, but MM has requested
            full service, RR starts after timeout of the registration
            timer new attempts to reach full service. After reading
            system information message 3 or 4 of the candidate, this
            function is called, to check whether the cell fullfills
            the requirements for a full service cell. In this case
            a cell reselection is started to read the whole BCCH to
            configure layer 1.

*/

GLOBAL void att_check_fplmn_cell (void)
{
  GET_INSTANCE_DATA;
  UBYTE              cell_ok;
  UBYTE              plmn_ok;
  T_loc_area_ident * lai = &rr_data->nc_data[CR_INDEX].lai;

  TRACE_FUNCTION ("att_check_fplmn_cell()");

  /*
   * Calculate the path loss criterion
   */
  att_calculate_c1 (CR_INDEX);
  TRACE_C1(CR_INDEX);

  /*
   * check that the cell is not barred and has a positive path loss
   * criterion.
   */
/* Implements RR Clone findings #8 */
  cell_ok = (!att_cell_barred_status_cr_no_cr (CR_INDEX) AND
             (rr_data->nc_data[CR_INDEX].c1 > 0)AND dat_roam_forb_lai_check (CR_INDEX));
;

  /*
   * check whether the PLMN identification is equal to the PLMN which has
   * been requested by MM.
   */
  plmn_ok = dat_plmn_equal_req (lai->mcc,
                                lai->mnc,
                                rr_data->ms_data.plmn.mcc,
                                rr_data->ms_data.plmn.mnc);

  /*
   * stop of T_RESELECT. This timer controls the reception of the initial
   * system information message.
   */
  TIMERSTOP (T_RESELECT);

  /*
   * store PLMN in plmn found list to provide this information to MM
   * at the end of a failed registration attempt.
   */
  if(dat_forb_lai_check (CR_INDEX))
  {
     att_store_plmn_in_found_list (lai);
  }
  else
  {
     TRACE_EVENT ("Do not store Forbidden LAI PLMN in the found PLMN list");
  }

  if (plmn_ok)
  {
    if (cell_ok)
    {
      /*
       * Enable nc monitoring to resume in lower layer
       * when recovering full service
       */
       att_notify_stop_plmn_search (FALSE);

      /*
       * cell and PLMN are okay, then start cell reselection to read
       * the complete BCCH to configure layer 1.
       */
/*XY:n inform GRR, and       wait for CR_RSP */
#ifdef GPRS
      att_start_cell_reselection_gprs (CELL_RESELECTION_CR);
#else
      att_start_cell_reselection (CELL_RESELECTION_CR);
#endif
      return;
    }
  }

  if (cell_ok)
  {
    /*
     * PLMN is not okay, but the cell
     *
     * "delete neighbourcells", that means give all
     * neighbourcells a low priority in scanning.
     */
    if (dat_test_sim_available () EQ FALSE)
      cs_del_list (&rr_data->cr_data.cd.ncell_list);
  }
  else
  {
    /*
     * PLMN okay but not the cell
     *
     * "set neighbourcells", that means give all
     * neighbourcells a higher priority in scanning.
     */
    if (dat_test_sim_available () EQ FALSE)
    {
      if (plmn_ok)
        /*
         * with SIM card, that means full service.
         * "set neighbourcells", that means give all
         * neighbourcells a higher priority in scanning.
         */
        cs_set_list (&rr_data->cr_data.cd.ncell_list);
      else
        /*
         * with SIM card, that means full service.
         * "delete neighbourcells", that means give all
         * neighbourcells a low priority in scanning.
         *
         * It is assumed that this cell will fail also
         * because they are also member of the wrong network.
         *
         * This is not done during FTA campaign.
         */
        cs_del_list (&rr_data->cr_data.cd.ncell_list);
    }
  }

  /*
   * start synchronisation attempt to the next cell
   */
  memset (&rr_data->nc_data[CR_INDEX], 0, sizeof (T_NC_DATA));
  cs_sync_next ();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_check_cell             |
+--------------------------------------------------------------------+

  PURPOSE : if a cell reselection decision has been taken, this function
            looks for a candidate for cell reselection taking in account
            several requirements. The reselect_index stores the index
            of the RR storage area for the neighbourcell. The return
            value indicates whether a cell has been found or not.

*/

static BOOL att_check_cell (void)
{
  GET_INSTANCE_DATA;
  BOOL cell_ok = FALSE;

  TRACE_FUNCTION ("att_check_cell()");

  /*
   * while no suitable candidate has been found,
   * but still possible cells available.
   */
  while (rr_data->reselect_index NEQ NO_AVAILABLE AND ! cell_ok)
  {
    /*
     * calculate the cell reselection criterion C2 for the candidate
     */
    att_calculate_c2 (rr_data->reselect_index);

    /*
     * check the cell requirements:
     * 1. check the cell barred status (if shall be non-barred)
     * 2. the cell shall be not temporarily excluded (tnnn e.g. after random
     *    access failure).
     * 3. the cell must have a positive cell reselection criterion C2.
     */

/* Implements RR Clone findings #8 */
    cell_ok = (! att_cell_barred_status_cr_no_cr (rr_data->reselect_index) AND
               ! is_tnnn (rr_data->reselect_index) AND
               rr_data->nc_data[rr_data->reselect_index].c2 > 0);

    /*
     * Forbidden LA for roaming shall be tested there in case a C2-base cell
     * reselection has been triggered on a cell candidate belonging to a
     * valid LA but unsuccesfull. Then RR will continue cell reselection
     * with the next candidate that might be part of a forbidden LAC.
     * Should be trapped by att_check_neighbourcell() at the end of the
     * CS3 session... but it skips one CR and secures the process.
     * Question : should this test be bypassed if MM requested limited
     * service ??
     */

    if (rr_data->ms_data.rr_service EQ FULL_SERVICE)
    {
      if (cell_ok)
        cell_ok = dat_roam_forb_lai_check (rr_data->reselect_index);
    }

    if (cell_ok)
    {
      /*
       * suitable cell has been found
       */
      TRACE_EVENT ("use neighbour cell");

      /*
       * after random access failure we found another cell,
       * that means, we will process a normal cell reselection
       * and will not come back directly to idle mode if the
       * cell reselection fails for the new cell.
       */
      if (rr_data->sc_data.selection_type EQ CELL_RESELECTION_RACH)
      {
         rr_data->sc_data.selection_type = CELL_RESELECTION;
      #ifdef GPRS
         rr_data->gprs_data.use_c31 = FALSE;
      #endif
      }

      /*
       * configure layer 1 for cell reselection
       */
      TRACE_EVENT_P2 ("config L1 for CR [%d]->[%d]",
        rr_data->nc_data[SC_INDEX].arfcn,
        rr_data->nc_data[rr_data->reselect_index].arfcn);

      #ifdef GPRS
      rr_data->gprs_data.ready_state         = FALSE;    /* Standby */
      #endif

      att_start_cr_in_pl(rr_data->reselect_index);
    }
    else
    {
      /*
       * cell is not suitable. Look for another cell.
       */
      rr_data->reselect_index =
        att_get_next_highest_c2_idx (rr_data->reselect_index);
    }
  }

  /*
   * return whether a cell has been found or not.
   */
  return cell_ok;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_clean_buf              |
+--------------------------------------------------------------------+

  PURPOSE : trigger cleaning of the SI buffers indicated by si_to_clean

*/
void att_clean_buf (USHORT si_to_clean)
{
  PALLOC(clean_buf, MPH_CLEAN_BUF_REQ);
  clean_buf->si_to_clean = si_to_clean;
  PSENDX(PL, clean_buf);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_start_cr_in_pl         |
+--------------------------------------------------------------------+

  PURPOSE : trigger CR in PL and init some necessary parameters

*/
void att_start_cr_in_pl(UBYTE index)
{
  GET_INSTANCE_DATA;
  att_init_pl_status ();
  att_build_idle_req (index, MODE_CELL_RESELECTION);

  /*
   * Clean the CR_INDEX storage area in RR to get the complete
   * BCCH of the new cell
   */
  att_init_cr_data();
  memcpy (&rr_data->nc_data[CR_INDEX],
          &rr_data->nc_data[index],
          sizeof (T_NC_DATA));
  srv_clear_list (&rr_data->cr_data.cd.ncell_list);
  SET_STATE (STATE_ATT, ATT_CS3);
  rr_data->bcch_error = 0;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_stop_dedicated        |
+--------------------------------------------------------------------+

  PURPOSE : Send primitive MPH_STOP_DEDICATED_REQ
  
*/
GLOBAL void att_stop_dedicated(void)
{
  PALLOC(mph_stop_dedi, MPH_STOP_DEDICATED_REQ);
  PSENDX(PL, mph_stop_dedi);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_leave_dedicated        |
+--------------------------------------------------------------------+

  PURPOSE : Decide if we do a CR or return to idle directly

*/
GLOBAL void att_leave_dedicated(void)
{
  GET_INSTANCE_DATA;
  if (IS_TIMER_ACTIVE(T_DEDICATED_MODE))
  { /* spent less than 30 seconds in dedicated mode */
    TIMERSTOP(T_DEDICATED_MODE);
  }
  else
  { /* spent at least 30 seconds in dedicated mode */
    rr_data->mode_after_dedi = MODE_CELL_RESELECTION;
  }

  if((rr_data->mode_after_dedi EQ MODE_CELL_SELECTION) AND
     (rr_data->sc_data.selection_type NEQ BACK_FROM_DEDICATED_RLF)) /*does not seem correct!!!*/
  {
    /*
     * go back directly to the old cell after dedicated
     * mode if we were in dedicated mode shorter than 30 seconds
     * and have not made a handover in dedicated mode
     */
#ifdef GPRS
    if(GPRS_SUSPENDED_BCCH EQ GET_STATE(STATE_GPRS))
    {
      SET_STATE(STATE_GPRS,GPRS_PIM_BCCH);
    }
    else if(GPRS_SUSPENDED_PBCCH EQ GET_STATE(STATE_GPRS))
    {
      SET_STATE(STATE_GPRS,GPRS_PIM_PBCCH);
    }
#endif
    att_return_to_idle();

    /*
     * restore the neighbour cell list which was valid
     * in idle mode (derived from SI 2x)
     * Note: SI 5x may contain other neighbour cells than SI 2x
     */
    memcpy(&rr_data->sc_data.cd.ncell_list,
           &rr_data->sc_data.ncell_list_idle,
           sizeof(T_LIST));

    /*
     * restore the sys_info_read bitmap if we are going
     * directly to idle
     */
    rr_data->sc_data.cd.sys_info_read = ALL_SYS_INFO_READ;

    if (rr_data->repeat_est)
    {
      rr_data->repeat_est = FALSE;
      dat_start_immediate_assign (rr_data->ms_data.establish_cause);
    }
    else
    {
#ifdef GPRS
      if(!att_gprs_cell_has_pbcch())
#endif
        att_code_mph_ncell_req(SC_INDEX);
    }

    /* After Dedicated mode lass than 30 seconds for a Location Area Update or
     * routing Area update.
     * start black list search to look for inactive carriers
     * Cell-Selection Improvement LLD - 4.1.4.3  
     */
    if((rr_data->ms_data.establish_cause EQ ESTCS_SERV_REQ_BY_MM) AND 
       (rr_data->cs_data.black_list_search_pending EQ TRUE))
    { 

      TRACE_EVENT("black_list_search_pending : 1");

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
        else
        {
#endif
        if(srv_is_list_set(
             &rr_data->cs_data.black_list.list[rr_data->cs_data.region]))
        {
          att_start_cell_selection(RR_ORIGINATED,CS_PARALLEL,BLACK_LIST_SEARCH_MODE);
        }
        else
        {
          TRACE_EVENT("Black List empty");
        }
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
        }
#endif
      }
    }
  }
  else
  {
#ifdef GPRS
    att_start_cell_reselection_gprs(BACK_FROM_DEDICATED);
#else
    att_start_cell_reselection(BACK_FROM_DEDICATED);
#endif
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_leave_dat_imm_ass      |
+--------------------------------------------------------------------+

  PURPOSE : return to idle mode directly and clean up 
            immediate assignment procedure

*/
GLOBAL void att_leave_dat_imm_ass(void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_leave_dat_imm_ass()");

#ifdef GPRS
  if(GPRS_SUSPENDED_BCCH EQ GET_STATE(STATE_GPRS))
  {
    SET_STATE(STATE_GPRS,GPRS_PIM_BCCH);
  }
  else if(GPRS_SUSPENDED_PBCCH EQ GET_STATE(STATE_GPRS))
  {
    SET_STATE(STATE_GPRS,GPRS_PIM_PBCCH);
  }
#endif

  att_return_to_idle();
      
  rr_data->repeat_est = FALSE;

#ifdef GPRS
  if(!att_gprs_cell_has_pbcch())
#endif
    att_code_mph_ncell_req(SC_INDEX);
 
 /*    
  * start black list search to look for inactive carriers
  * Cell-Selection Improvement LLD - 4.1.4.3  
  */
  if((rr_data->ms_data.establish_cause EQ ESTCS_SERV_REQ_BY_MM) AND 
    (rr_data->cs_data.black_list_search_pending EQ TRUE))
  { 
    
    TRACE_EVENT("black_list_search_pending : 1");
    
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
      else
      {
#endif
      if(srv_is_list_set(&rr_data->cs_data.black_list.list[rr_data->cs_data.region]))
      {
        att_start_cell_selection(RR_ORIGINATED,CS_PARALLEL,BLACK_LIST_SEARCH_MODE);
      }
      else
      {
        TRACE_EVENT("Black List empty");
      }
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
      }
#endif
    }
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_select_cell_dedicated  |
+--------------------------------------------------------------------+

  PURPOSE : The function selects a cell for cell reselection after
            coming back from a dedicated connected (normal release
            or radio link failure.

*/

static void att_select_cell_dedicated (void)
{
  GET_INSTANCE_DATA;
  U8 search_mode;

  TRACE_FUNCTION ("att_select_cell_dedicated()");

  if (rr_data->reselect_index NEQ NO_AVAILABLE)
  {
    TRACE_EVENT_P3 ("[%u] new CS/CR (mode=%u, sel.type=%d)",
      rr_data->nc_data[rr_data->reselect_index].arfcn,
      rr_data->mode_after_dedi, rr_data->sc_data.selection_type);

    /*
     * A cell has been found for coming back from dedicated mode.
     * Configure layer 1 for cell reselection.
     */
    att_start_cr_in_pl(rr_data->reselect_index);
  }
  else
  {
    /*
     * no suitable cell is available
     */
    if (rr_data->sc_data.selection_type EQ BACK_FROM_DEDICATED_RLF)
    {
      /*
       * In case of coming back after radio link failure
       * the old cell shall be used only if no other cell
       * is suitable. So the selection type is changed to
       * "normal" comeback from dedicated mode. MM is informed
       * that no reestablishment is possible and a cell is
       * searched again inclusive the old serving cell.
       */
      rr_data->sc_data.selection_type = BACK_FROM_DEDICATED;
      dat_code_reestablishment_fail ();
      TIMERSTART (T_RESELECT, TRESELECT_VALUE);
      att_search_cell ();
    }
    else
    {
      /* Cell Selection Improvements-LLD section:4.1.3.9 */
      if((rr_data->net_lost) AND (rr_data->ms_data.req_mm_service EQ FUNC_PLMN_SRCH))
      {
        /* Radio Link failure in dedicated state and cell reselection failed.
         * Start Fast search
         */
        search_mode = FAST_SEARCH_MODE;
      }  
      else
      {
        /* Normal return from dedicated mode, cell reselection failed
         * Start Normal search
         */
        search_mode = NORMAL_SEARCH_MODE;
      }

      /*
       * there is no candidate for coming back. So a cell
       * selection is started from RR.
       */
      /* XY:n inform GRR, and       wait for CR_RSP */
#ifdef GPRS
      att_start_cell_selection_gprs (RR_ORIGINATED,search_mode);
#else
      att_start_cell_selection (RR_ORIGINATED, CS_NOT_PARALLEL,search_mode);
#endif
    }
  }
  //rr_data->mode_after_dedi = MODE_CELL_SELECTION; /* reset mode */
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_check_neighbourcell    |
+--------------------------------------------------------------------+

  PURPOSE : The function checks a cell after cell reselection.

*/

GLOBAL void att_check_neighbourcell (void)
{
  GET_INSTANCE_DATA;
  BOOL               cell_ok;
  T_loc_area_ident * lai = &rr_data->nc_data[CR_INDEX].lai;

  TRACE_FUNCTION ("att_check_neighbourcell()");

#ifdef GPRS
  if(att_gprs_check_ncell())
    return;
#endif
  /*
   * Calculate the cell reselection criterion C2 or C31/C32.
   */
  #ifdef GPRS
  if( rr_data->gprs_data.use_c31)
  {
    att_calculate_c31_c32(CR_INDEX);

/* Implements RR Clone findings #8 */
    cell_ok = (!att_cell_barred_status_cr_no_cr (CR_INDEX) AND
                     att_check_c31_criterion(CR_INDEX) );
  }
  else
  {
  #endif
    att_calculate_c2 (CR_INDEX);
    /*
     * Check the cell. It shall not be barred and must have
     * a positive cell reselection criterion C2.
     */

/* Implements RR Clone findings #8 */
    cell_ok = ! att_cell_barred_status_cr_no_cr (CR_INDEX) AND
              rr_data->nc_data[CR_INDEX].c2 > 0;

  #ifdef GPRS
  }
  #endif

  /*
   * the cell can be rejected by the SW shielding function.
   * This has only an affect during type approval.
   */
  if (cell_ok)
    cell_ok = att_check_network (lai);

  /*
   * Check against the list of forbidden location areas for roaming,
   * but only if MM requested full service and we are still searching
   * for full service. Do not prevent limited service reselection.
   */
  if (cell_ok AND
      (rr_data->cs_data.scan_mode NEQ CS_SECOND_SCAN) AND
      (rr_data->ms_data.req_mm_service EQ FUNC_PLMN_SRCH))
  {
    cell_ok = dat_roam_forb_lai_check (CR_INDEX);
  }

  /*
   * Additional tests are performed depending on the cell
   * reselection type.
   */
  TRACE_SELECTION_TYPE (rr_data->sc_data.selection_type);
  switch (rr_data->sc_data.selection_type)
  {
    case CELL_RESELECTION_CR:
      /*
       * The MS was in idle mode and the check is
       * 1. from limited service to full service or
       * 2. from full service (VPLMN) to full service (HPLMN)
       *
       * The resulting RR service is set only if the cell is okay.
       */
      if (cell_ok)
      {
        if (rr_data->ms_data.req_mm_service EQ FUNC_LIM_SERV_ST_SRCH OR
            rr_data->cs_data.scan_mode EQ CS_SECOND_SCAN)
        {
          rr_data->ms_data.rr_service = LIMITED_SERVICE;
        }
        else
        {
          cell_ok = dat_plmn_equal_req(lai->mcc, lai->mnc,
                                       rr_data->ms_data.plmn.mcc,
                                       rr_data->ms_data.plmn.mnc);
          if (cell_ok)
            rr_data->ms_data.rr_service = FULL_SERVICE;
          else
            rr_data->ms_data.rr_service = LIMITED_SERVICE;
        }
      }
      break;

    case CELL_SELECTION:
    case CELL_RESELECTION:
    case CELL_RESELECTION_NC:
    case CELL_RESELECTION_RACH:
      if(cell_ok)
      {
        /*
         * 2 checks to be made
         *
         * - In case of FULL SERVICE additionally the PLMN is checked
         *
         * - We are in LIMITED service.
         *    If the req_mm_service is FUNC_LIM_SERV_ST_SRCH we
         *    will stay in LIMITED.
         *    If we are in limited because of forbidden LAI we enter
	 *    FULL SERVICE.
         */
        if(rr_data->ms_data.rr_service EQ FULL_SERVICE)
        {
          cell_ok = dat_plmn_equal_req (lai->mcc,
                                      lai->mnc,
                                      rr_data->ms_data.plmn.mcc,
                                      rr_data->ms_data.plmn.mnc);
        }
        else if((rr_data->ms_data.rr_service EQ LIMITED_SERVICE) AND
                (rr_data->ms_data.req_mm_service NEQ FUNC_LIM_SERV_ST_SRCH) AND
                 dat_plmn_equal_req (lai->mcc,
                                     lai->mnc,
                                     rr_data->ms_data.plmn.mcc,
                                     rr_data->ms_data.plmn.mnc) )
        {
          rr_data->ms_data.rr_service = FULL_SERVICE;
        }

      }
      break;
      
      case BACK_FROM_DEDICATED:
      case BACK_FROM_DEDICATED_RLF:
      /* The mobile is trying to select a cell after coming back from dedicated 
       * state. The cell is selected only if it belongs to the previously
       * selected PLMN 
       */
      if(cell_ok)
      {
        /* While selecting a cell after an emergency call, we need not check the 
         * PLMN ID of the cell */
        if(rr_data->ms_data.rr_service EQ FULL_SERVICE)
        {
          cell_ok = dat_plmn_equal_req (lai->mcc,
                                        lai->mnc,
                                        rr_data->ms_data.plmn.mcc,
                                        rr_data->ms_data.plmn.mnc);
        }

        TRACE_EVENT_P6 ("B_F_D New PLMN : MCC=%X%X%X MNC=%X%X%X",
                        lai->mcc[0],
                        lai->mcc[1],
                        lai->mcc[2],
                        lai->mnc[0],
                        lai->mnc[1],
                        lai->mnc[2]);
        TRACE_EVENT_P6 ("B_F_D Old PLMN : MCC=%X%X%X MNC=%X%X%X",
                        rr_data->ms_data.plmn.mcc[0],
                        rr_data->ms_data.plmn.mcc[1],
                        rr_data->ms_data.plmn.mcc[2],
                        rr_data->ms_data.plmn.mnc[0],
                        rr_data->ms_data.plmn.mnc[1],
                        rr_data->ms_data.plmn.mnc[2]);
      }
      if(rr_data->sc_data.selection_type EQ BACK_FROM_DEDICATED)
        break;
    
      /*
       * in case of call reestablishment check also call reestablishment flag
       */
      if (cell_ok)
      {
        if (rr_data->nc_data[CR_INDEX].rach.re NEQ REESTAB_YES)
          cell_ok = FALSE;
      }
      break;
  }

  /*
   * the requirement check is finished
   */
  if (cell_ok)
  {
    TRACE_EVENT_P2 ("NEW SC [%d]->[%d]",
      rr_data->nc_data[SC_INDEX].arfcn,
      rr_data->nc_data[CR_INDEX].arfcn);

    /*
     * Stop T_RESELECT, which controls the reception of the complete BCCH
     * during Cell Reselection.
     */
    TIMERSTOP (T_RESELECT);


    /* T3122 controls after how long the MS is able to attempt access back to the */
    /* cell from which it received a IMM ASSGN REJ. This no longer applies after a */
    /* cell reselection onto a new cell. */
    
     TIMERSTOP(T3122);

   
    /*
     * copy all data from the SC_INDEX storage area to the neighbourcell area.
     * copy all data from the CR_INDEX storage area to the SC_INDEX area.
     * old serving cell data index is stored in old_serving_cell.
     */
    att_copy_cr_data ();
    
    memcpy (&rr_data->sc_data.cd, &rr_data->cr_data.cd,
            sizeof (T_CELL_DATA));

    if(rr_data->ms_data.rr_service EQ FULL_SERVICE)
    {
      att_full_service_found();
    }

    rr_data->nc_data[CR_INDEX].arfcn = NOT_PRESENT_16BIT;
    /*
     * GSM 5.08, chapter 6.4:
     *
     * T is a timer implemented for each cell in the list of strongest
     * carriers.
     * T shall be started from zero at the time the cell is placed by the MS on
     * the list of strongest carriers, except when the previous serving cell is
     * placed on the list of strongest carriers at cell reselection. In this
     * case, T shall be set to the value of PENALTY_TIME (i.e. expired).
     */
    switch (rr_data->sc_data.selection_type)
    {
      case CELL_RESELECTION:
      case CELL_RESELECTION_NC:
      case CELL_RESELECTION_RACH:
        /*
         * set the avail time of the old serving cell to PENALTY_TIME.
         */
        if (rr_data->old_serving_cell < SC_INDEX)
          rr_data->nc_data[rr_data->old_serving_cell].avail_time =
            rr_data->nc_data[rr_data->old_serving_cell].c2_par.penalty_time;
        break;

      default:
        break;
    }


    /*
     * set a barrier of 15 seconds for the next cell reselection
     * if it was a cell reselection due to C2(NC) > C2(SC)
     */
    if (rr_data->sc_data.selection_type EQ CELL_RESELECTION_NC)
      TIMERSTART (T_NO_RESELECT, THIRTY_SEC/2);

#ifdef GPRS
    if(! att_gprs_is_avail())
    {
#endif

      /*
       * configure layer 1
       */
       att_remove_bad_rr_data_ncells();
       att_set_pl_in_idle_mode ();

      /*
       * inform data transfer process that the cell is attached.
       */
      dat_att_cell_selected ();

      /*
       * inform MM about new cell
       */
      if (rr_data->sc_data.mm_started)
        att_code_rr_act_cnf ();
      else
        att_code_rr_act_ind ();

      SET_STATE (STATE_ATT, ATT_IDLE);
      srv_use_stored_prim ();
      /*
       * Start registration timer if needed
       */
      att_start_registration_timer ();
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
#ifdef GPRS
    }
    else
    {
      if(rr_data->ms_data.rr_service NEQ FULL_SERVICE)
      {
        if(att_gprs_cell_has_pbcch())
        {
          /* set to PIM_BCCH if the cell hs a PBCCH but we
           * only have limited service
           */
          SET_STATE(STATE_GPRS, GPRS_PIM_BCCH);          
        }
      }

      if(att_gprs_cell_has_pbcch())
      {
        att_gprs_stop_pl();
      }
      else
      {
        att_remove_bad_rr_data_ncells();
        att_set_pl_in_idle_mode ();
      }
      /* inform GRR if we have GPRS support */
      att_signal_gprs_support();
      /* and wait for CR_REQ(CR_COMPLETE) from GRR */
    }
#endif
  }
  else
  {
    /*
     * the cell reselection failed for this channel and is continued
     * with the next candidate.
     */
    TRACE_EVENT_P1 ("[%u] CR failed", rr_data->nc_data[CR_INDEX].arfcn);
    rr_data->pag_rec = FALSE;
    srv_clear_stored_prim (MPH_PAGING_IND);
    att_continue_cell_reselect ();
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_check_neighbourcell_si_reestab  |
+--------------------------------------------------------------------+

  PURPOSE : This function checks if revelant sys info is present is ATT_CS3 state.
  Special case : Sys Info 2Ter is not needed if we have a pending call re-establishment

*/

GLOBAL void att_check_neighbourcell_si_reestab (void)
{
  GET_INSTANCE_DATA;
  /*
   * Sometimes the network indicates SI2Ter but never sends it.
   * If we have a call re-establishment pending, then we must not wait
   * for SI2Ter.
   */
  if( (rr_data->cr_data.cd.sys_info_read EQ ALL_SYS_INFO_READ)
       OR
      ( (rr_data->cr_data.cd.sys_info_read EQ SYS_INFO_EXCEPT_2TER) AND
        (rr_data->sc_data.selection_type EQ BACK_FROM_DEDICATED_RLF))
    )
  {
    att_check_neighbourcell ();
  }

  /*
   * In case of Radio Link Failure, check cell for call
   * re-establishment capability.
   */
  if (rr_data->sc_data.selection_type EQ BACK_FROM_DEDICATED_RLF)
    att_check_reestablishment();

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_check_reestablishment  |
+--------------------------------------------------------------------+

  PURPOSE : The function checks whether a cell has no call re-establishment
            capability. In this case the cell reselection is resumed
            with the next cell.

*/

GLOBAL void att_check_reestablishment (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_check_reestablishment()");

  if (rr_data->nc_data[CR_INDEX].rach.re NEQ REESTAB_YES)
  {
    /*
     * the cell reselection failed for this channel and is continued
     * with the next candidate due to no call re-establishment
     * capability.
     */
    TRACE_EVENT_P2 ("[%u]7 (BCCH-St=%u) have no call re-establishment capability",
      rr_data->nc_data[CR_INDEX].arfcn, rr_data->nc_data[CR_INDEX].bcch_status);

    rr_data->pag_rec = FALSE;
    srv_clear_stored_prim (MPH_PAGING_IND);
    att_continue_cell_reselect ();
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_check_reselect_decision|
+--------------------------------------------------------------------+

  PURPOSE : After reception of measurement values from layer 1 in
            idle mode the cell reselection decision is done by this
            function. It checks the measurement related criterions
            for cell reselection. The flag start_now indicates
            whether only the decision is taken or the cell reselection
            is started immediately. It is senseful to delay a cell
            reselection during a PLMN available search triggered by MM.

*/

GLOBAL void att_check_reselect_decision (UBYTE start_now)
{
  GET_INSTANCE_DATA;
  UBYTE    i;
  SHORT    delta;

  TRACE_FUNCTION ("att_check_reselect_decision()");
  #ifdef GPRS
  TRACE_EVENT_P2("check_reselect: start_cell_reselection=%u calc %s",
    rr_data->start_cell_reselection, rr_data->gprs_data.use_c31 ? "C31,C32":"C2");
  #endif
  /*
   * Calculate for all synchronized neighbourcells, which have
   * decoded the system information messages for calculation C2,
   * and for the serving cell the cell reselection criterion C2.
   */
  for (i = 0; i < 7; i++)
  {
    if (rr_data->nc_data[i].bcch_status EQ DECODED)
    {
  #ifdef GPRS
      if( rr_data->gprs_data.use_c31)
        att_calculate_c31_c32(i);
      else
  #endif
        att_calculate_c2 (i);
    }
    else if (rr_data->nc_data[i].bcch_status NEQ EMPTY AND
             rr_data->nc_data[i].arfcn NEQ NOT_PRESENT_16BIT )
    {
        TRACE_EVENT_P2("[%u]i%u no C2 (SIs missing)",
        rr_data->nc_data[i].arfcn,i);
    }
  }

  /*
   * Check C1 serving cell < 0 for more then 5 seconds
   * The c1_counter counts the number of reports where
   * this criterion has passed.
   */
  if (rr_data->nc_data[SC_INDEX].c1 < 0)
  {
    /*
     * if c1_counter has a value of at least 1 it had happened
     * for at least two successive reports, that means more
     * then five seconds.
     */
    if ( (rr_data->nc_data[SC_INDEX].c1_counter > 0)
          #ifdef GPRS
          OR
          (rr_data->gprs_data.use_c31)
          #endif
        )
    {
      /*
       * reset the counter and start !
       */
      rr_data->nc_data[SC_INDEX].c1_counter = 0;
#ifdef GPRS
      /* check if we can do a cell reselection */
      if(!is_nc2_used())
      {
#endif
        if (start_now )
        {
/*XY:n inform GRR, and       wait for CR_RSP */
#ifdef GPRS
          att_start_cell_reselection_gprs (CELL_RESELECTION);
#else
          att_start_cell_reselection (CELL_RESELECTION);
#endif
        }
#ifdef GPRS
      }
#endif
      return;
    }
    else
    {
      /*
       * the criterion has passed the first time, so
       * increase the counter and wait for the next measurement value.
       */
      rr_data->nc_data[SC_INDEX].c1_counter++;
#if defined(_SIMULATION_)
      TRACE_EVENT_WIN_P1 ("CR SC delayed (%d)", rr_data->nc_data[SC_INDEX].c1_counter);
#endif
    }
  }
  else
  {
    /*
     * The C1 of the serving cell is positive, so clear the c1_counter.
     */
    rr_data->nc_data[SC_INDEX].c1_counter = 0;
#if defined(_SIMULATION_)
    TRACE_EVENT_WIN_P1 ("CR SC c1_counter=%d", rr_data->nc_data[SC_INDEX].c1_counter);
#endif
  }
  /*
    * check if C31\C32 based reselection is required
    */
  #ifdef GPRS
  if( rr_data->gprs_data.use_c31 )
  {
    att_check_c31_reselect_decision(start_now);
    return;
  }
  #endif
  /*
   * Check C2(NC) > C2(SC) for all neighbourcells
   */
  for (i = 0; i < 6; i++)
  {
    /*
     * If data is stored for this neighbourcell
     */
    if (rr_data->nc_data[i].bcch_status EQ DECODED)
    {
/*
      TRACE_EVENT_P2 ("COMPARE NC[%d]-SC[%d]",
        rr_data->nc_data[i].arfcn,
        rr_data->nc_data[SC_INDEX].arfcn);
*/
#ifdef GPRS
      /*
       * check if the location area has changed
       * or the routing area if the scell supports GPRS
       */
      {
        BOOL la_same, ra_same;

        if (dat_plmn_equal_req (rr_data->nc_data[i].lai.mcc,
                                rr_data->nc_data[i].lai.mnc,
                                rr_data->nc_data[SC_INDEX].lai.mcc,
                                rr_data->nc_data[SC_INDEX].lai.mnc)
            AND
            rr_data->nc_data[i].lai.lac EQ
            rr_data->nc_data[SC_INDEX].lai.lac)
          la_same = TRUE;
        else
          la_same = FALSE;
        /*
         *  If gprs is not available do not care about the routing area
         *
         */
        if(att_gprs_is_avail())
        {
          if(rr_data->nc_data[i].rac EQ NOT_PRESENT_8BIT OR
             rr_data->nc_data[i].rac EQ rr_data->nc_data[SC_INDEX].rac)
          {
            ra_same = TRUE;
          }
          else
          {
            ra_same = FALSE;
          }
        }
        else
        {
          ra_same = TRUE;
        }

        if(la_same AND ra_same AND !rr_data->gprs_data.ready_state)
#else /* GPRS */
        if (dat_plmn_equal_req (rr_data->nc_data[i].lai.mcc,
                                rr_data->nc_data[i].lai.mnc,
                                rr_data->nc_data[SC_INDEX].lai.mcc,
                                rr_data->nc_data[SC_INDEX].lai.mnc)
            AND
            rr_data->nc_data[i].lai.lac EQ
            rr_data->nc_data[SC_INDEX].lai.lac)
#endif  /* GPRS */
        {
          /*
           * If it is in the same location area as the serving cell,
           * the delta is simply the difference of both cell reselection
           * criterion values C2.
           */
          TRACE_EVENT_P3 ("NC[%d]i%u-SC[%d] same LAI",
            rr_data->nc_data[i].arfcn,i,
            rr_data->nc_data[SC_INDEX].arfcn);

          delta = rr_data->nc_data[i].c2 -
            rr_data->nc_data[SC_INDEX].c2;

          /*
           * 3GPP TS 05.08, section 6.6.2:
           * in case of a cell reselection occurring within the previous 15 seconds
           * in which case the C2 value for the new cell shall exceed the C2 value
           * of the serving cell by at least 5 dB for a period of 5 seconds
           */

          if ( IS_TIMER_ACTIVE (T_NO_RESELECT) )
          {
            delta -= 5;
          }
          else
          {
            /* When C2(NC)==C2(SC) i.e. delta=0, an invalid cell reselection gets triggered below.
             * 'delta' needs to be adjusted to prevent this happening. The '=' in the '>='
             * condition below only applies when T_NO_RESELECT timer is active and
             * c2_corr has been decremented by 5 (above statement)
             */
            if(delta EQ 0 )
            {
              delta = -1;
	    }
          }
        }
        else
        {
          /*
           * If both are member of different location areas, a threshold
           * value CELL_RESELECTION_HYSTERESE is taken in account.
           */
          TRACE_EVENT_P3 ("NC[%d]i%u-SC[%d] different LAI",
            rr_data->nc_data[i].arfcn,i,
            rr_data->nc_data[SC_INDEX].arfcn);

          delta = att_calculate_c2_diff (i);
        }
#ifdef GPRS
      }
#endif
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
      if(att_cell_barred_status_cr_no_cr(i))
        delta = -1;

      /*
       * Do not trigger a cell reselection decision on meas. report
       * receipt if the cell belong to a LA not allowed for roaming.
       * According to 3.22 chapter 3.5.4 cell reselection on a
       * forbidden LA for regional provision of service is allowed.
       */
      if (rr_data->ms_data.rr_service EQ FULL_SERVICE)
      {
        if (!dat_roam_forb_lai_check(i))
          delta = -1;
      }

#if defined(_SIMULATION_)
      TRACE_EVENT_WIN_P1 ("delta=%d", delta);
#endif
      if(delta == 0) TRACE_EVENT("No reselection delta == 0");
      if (delta > 0)
      {
        /*
         * The condition C2(NC) > C2(SC) is fulfilled.
         */
        if (rr_data->nc_data[i].c1_counter > 0)
        {
          /*
           * the conditions is fulfilled at least two times ( > 5 seconds)
           * and there is no barrier to start the cell
           * reselection.
           */
#ifdef GPRS
          /* check if we can do a cell reselection */
          if(!is_nc2_used())
          {
#endif
            if (start_now)
            {
              rr_data->nc_data[i].c1_counter = 0;
/*XY:n inform GRR, and       wait for CR_RSP */
#ifdef GPRS
              att_start_cell_reselection_gprs(CELL_RESELECTION_NC);
#else
              att_start_cell_reselection (CELL_RESELECTION_NC);
#endif
            }
#ifdef GPRS
          }
#endif
          return;
        }
        else
        {
          /*
           * The condition has passed the first time or is delayed
           * by the barrier to perform a cell reselection due to this reasons
           * not faster then in periods of 15 seconds.
           */
          rr_data->nc_data[i].c1_counter++;
          TRACE_EVENT_P3 ("NC[%u]i%u CR delayed (%d)",
            rr_data->nc_data[i].arfcn,i,rr_data->nc_data[i].c1_counter);
        }
      }
      else
      {
        /*
         * the condition is not fullfilled and the counter is resetted.
         */
        rr_data->nc_data[i].c1_counter = 0;
      }
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_check_barred_status_in_idle|
+--------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void att_check_barred_status_in_idle (void)
{
  TRACE_FUNCTION ("att_check_barred_status_in_idle()");

  /*
   * check whether serving cell is barred
   */
/*XY:n inform GRR, and       wait for CR_RSP */

/* Implements RR Clone findings #8 */
  if (att_cell_barred_status_cr_no_cr (SC_INDEX) EQ TRUE)
  {
#ifdef GPRS
    att_start_cell_reselection_gprs (CELL_RESELECTION);
#else
    att_start_cell_reselection (CELL_RESELECTION);
#endif
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_clear_forb_list        |
+--------------------------------------------------------------------+

  PURPOSE : RR stores some forbidden lists. This function clears
            the given forbidden list.

*/

static void att_clear_forb_list (int list_type)
{
  GET_INSTANCE_DATA;
  int i;
  T_loc_area_ident *forb_list;

  TRACE_FUNCTION ("att_clear_forb_list()");

  if (list_type EQ FORBIDDEN_LIST_NORMAL)
    forb_list = &rr_data->ms_data.forb_lac_list[0];
  else
    forb_list = &rr_data->ms_data.roam_forb_lac_list[0];

/* Implements Measure#32: Row 29 */
  if(list_type EQ FORBIDDEN_LIST_NORMAL)
  {
    TRACE_EVENT ( "CLEAR FORB LIST");
  }
  else
  {
    TRACE_EVENT ( "CLEAR ROAM FORB LIST");
  }

  for (i = 0; i < MAX_LAI; i++)
  {
    memset (forb_list[i].mcc, NOT_PRESENT_8BIT, SIZE_MCC);
    memset (forb_list[i].mnc, NOT_PRESENT_8BIT, SIZE_MNC);
    forb_list[i].lac = NOT_PRESENT_16BIT;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_clear_registration_data|
+--------------------------------------------------------------------+

  PURPOSE : The function is called after reception of RR_SYNC_REQ,
            which is sent by MM after some Location Updating Reject
            causes which indicate that the IMSI is invalid, paging
            shall be suppressed and RR shall enter only a limited mode.

*/

GLOBAL void att_clear_registration_data (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_clear_registration_data()");

  TRACE_EVENT ("CLEAR registration data (IMSI, MCC, MNC)");

  /*
   * Clear the IMSI related data and enter the
   * limited mode.
   */
  rr_data->ms_data.operation_mode  = 0;
  att_clear_parallel_search ();
  rr_data->ms_data.current_plmn_search_type = FUNC_LIM_SERV_ST_SRCH;
  rr_data->ms_data.imsi_available  = FALSE;
  rr_data->ms_data.plmn.mcc[0]     = NOT_PRESENT_8BIT;
  rr_data->ms_data.plmn.mnc[0]     = NOT_PRESENT_8BIT;
  rr_data->ms_data.v_eq_plmn       = FALSE;

  /*we only have limited service*/
  rr_data->ms_data.rr_service = LIMITED_SERVICE;
  rr_data->ms_data.req_mm_service = FUNC_LIM_SERV_ST_SRCH;

  TRACE_EVENT_P3 ("att_clear_registration_data: current=%s, service:rMM=%s RRs=%s",
    _rr_str_FUNC[rr_data->ms_data.current_plmn_search_type],
    _rr_str_FUNC[rr_data->ms_data.req_mm_service],
    _rr_str_SERVICE[rr_data->ms_data.rr_service]);

  /*
   * clear TMSI and ciphering data
   */
  att_clear_reg_without_imsi ();
  att_set_rr_service_info();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_clear_reg_without_imsi |
+--------------------------------------------------------------------+

  PURPOSE : After some location updating reject causes MM informs RR
            that the TMSI and the ciphering data are invalid.

*/

GLOBAL void att_clear_reg_without_imsi (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_clear_reg_without_imsi()");

  /*
   * clear TMSI
   */
  rr_data->ms_data.tmsi_available = FALSE;
  rr_data->ms_data.tmsi_binary    = 0;

  /*
   * clear ciphering data
   */
  rr_data->ms_data.cksn           = CKSN_NOT_PRES;
  memset (rr_data->ms_data.kc, NOT_PRESENT_8BIT, KC_STRING_SIZE);
  memset (rr_data->ms_data.new_kc, NOT_PRESENT_8BIT, KC_STRING_SIZE);

  /*
   * update the identities in layer 1.
   */
  att_mph_identity_req ();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_reorder_mph_ncell_req  |
+--------------------------------------------------------------------+

  PURPOSE : reorders a neighbourcell list for layer 1 to make sure
            that the strongest neighbour cells are the first ones
            in the list and are therefore measured first by layer 1
*/

static void att_reorder_mph_ncell_req (T_MPH_NEIGHBOURCELL_REQ * mph_ncell_req)
{
  GET_INSTANCE_DATA;
#define MAX_STRONGEST 6 /* MAX_STRONGEST must be at least 6 but up to 8 makes sense */
  UBYTE  i_nc,
         i_nc1,
         c_str_nc = 0,
         c_str_cs = 0,
         is_old = !IS_TIMER_ACTIVE(TCSVALID),
         is_inserted, i, j;
  SHORT  arfcn,
         rxlev,
         str_arfcn[MAX_STRONGEST],
         str_rxlev[MAX_STRONGEST];

#if defined(_SIMULATION_)
  TRACE_EVENT_WIN_P6("ncell_req arfcns:%4d %4d %4d %4d %4d %4d",
    mph_ncell_req->arfcn[0] NEQ NOT_PRESENT_16BIT ?
      mph_ncell_req->arfcn[0] : -1,
    mph_ncell_req->arfcn[1] NEQ NOT_PRESENT_16BIT ?
      mph_ncell_req->arfcn[1] : -1,
    mph_ncell_req->arfcn[2] NEQ NOT_PRESENT_16BIT ?
      mph_ncell_req->arfcn[2] : -1,
    mph_ncell_req->arfcn[3] NEQ NOT_PRESENT_16BIT ?
      mph_ncell_req->arfcn[3] : -1,
    mph_ncell_req->arfcn[4] NEQ NOT_PRESENT_16BIT ?
      mph_ncell_req->arfcn[4] : -1,
    mph_ncell_req->arfcn[5] NEQ NOT_PRESENT_16BIT ?
      mph_ncell_req->arfcn[5] : -1);
#endif

  memset (str_arfcn, -1, sizeof str_arfcn);  /* for easier debugging */
  memset (str_rxlev, -1, sizeof str_rxlev);

  /*
   * search for the strongest neighbourcells and collect them in
   * the arrays str_arfcn/str_rxlev. An arfcn found in the nc_data
   * is always stored before an arfcn found in the cs_data. The reason
   * is that the nc_data are almost younger and probably more important be
   * and there may big differences between the rxlev values for the
   * same arfcn delevered by MPHC_RXLEV_IND and by MPHC_RXLEV_PERIODIC_IND
   */
  for (i_nc = 0; i_nc < MAX_NEIGHBOURCELLS AND
                mph_ncell_req->arfcn[i_nc] NEQ NOT_PRESENT_16BIT; i_nc++)
  {
    arfcn = mph_ncell_req->arfcn[i_nc];

    if (arfcn EQ rr_data->nc_data[SC_INDEX].arfcn)
      continue;  /* processing neighbour cell list in *mph_ncell_req */

    /*
     * for all channels of stored neighbour cell data
     */
    is_inserted = FALSE;
    for (i_nc1 = 0; i_nc1 < 6; i_nc1++)
    {
      if (arfcn EQ rr_data->nc_data[i_nc1].arfcn)
      {
        if (rr_data->nc_data[i_nc1].bcch_status NEQ EMPTY)
        {
          rxlev = rr_data->nc_data[i_nc1].rxlev;
          is_inserted = TRUE; /* avoid double inserting */
          for (i = c_str_nc; i > 0 AND rxlev > str_rxlev[i-1]; i--)
            ; /* search for the place to insert */
          if (i EQ c_str_nc AND c_str_nc >= MAX_STRONGEST)
            break; /* not strong enough */
          j = (c_str_cs >= MAX_STRONGEST) ? MAX_STRONGEST-1 : c_str_cs;
          for ( ; j > i; j--)
          { /* make place for current entry */
            str_rxlev[j] = str_rxlev[j-1];
            str_arfcn[j] = str_arfcn[j-1];
          }
          str_arfcn[i] = arfcn;
          str_rxlev[i] = rxlev;
          /* if (c_str_nc < MAX_STRONGEST) needed if MAX_STRONGEST < 6 */
          c_str_nc++;
          if (c_str_cs < MAX_STRONGEST)
            c_str_cs++;
        }
        break; /* scanning stored neighbour cell data */
      }
    } /* for: stored neighbour cell data */

    if (is_old OR is_inserted)
      continue;   /* CS measurements are too old */

    /*
     * for all channels of the power campaign list
     */
    for (i_nc1 = 0; i_nc1 < rr_data->cs_data.max_arfcn; i_nc1++)
    {
      if (arfcn EQ rr_data->cs_data.arfcn[i_nc1])
      {
        rxlev = rr_data->cs_data.rxlev[i_nc1];
        for (i = c_str_cs; i > c_str_nc AND rxlev > str_rxlev[i-1]; i--)
          ; /* search for the place to insert */
        if (i EQ c_str_cs AND c_str_cs >= MAX_STRONGEST)
          break; /* not strong enough */
        j = (c_str_cs >= MAX_STRONGEST) ? MAX_STRONGEST-1 : c_str_cs;
        for (; j > i; j--)
        { /* make place for current entry */
          str_rxlev[j] = str_rxlev[j-1];
          str_arfcn[j] = str_arfcn[j-1];
        }
        str_arfcn[i] = arfcn;
        str_rxlev[i] = rxlev;
        if (c_str_cs < MAX_STRONGEST)
          c_str_cs++;
        break; /* scanning power campaign list */
      }
    } /* for: power campaign list */
  } /* for: mph_ncell_req->arfcn */

  TRACE_EVENT_P7("c_str_nc=%u arfcn:%4d %4d %4d %4d %4d %4d",
    c_str_nc, str_arfcn[0], str_arfcn[1], str_arfcn[2],
              str_arfcn[3], str_arfcn[4], str_arfcn[5]);
  TRACE_EVENT_P7("c_str_cs=%u rxlev:%4d %4d %4d %4d %4d %4d",
    c_str_cs, str_rxlev[0], str_rxlev[1], str_rxlev[2],
              str_rxlev[3], str_rxlev[4], str_rxlev[5]);
  /* now reorder the arfcn's in the primitive */
  for (i_nc1 = 0; i_nc1 < c_str_cs; i_nc1++)
  {
    for (i_nc = 0; i_nc < MAX_NEIGHBOURCELLS AND
                   mph_ncell_req->arfcn[i_nc] NEQ NOT_PRESENT_16BIT; i_nc++)
    {
      if (str_arfcn[i_nc1] EQ mph_ncell_req->arfcn[i_nc])
      { /* swap */
        arfcn = mph_ncell_req->arfcn[i_nc1];
        mph_ncell_req->arfcn[i_nc1] = mph_ncell_req->arfcn[i_nc];
        mph_ncell_req->arfcn[i_nc] = arfcn;
        break;
      }
    } /* for: mph_ncell_req->arfcn */
  } /* for: str_arfcn */
  TRACE_EVENT_P6("strongest arfcns:%4d %4d %4d %4d %4d %4d",
    mph_ncell_req->arfcn[0] NEQ NOT_PRESENT_16BIT ?
      mph_ncell_req->arfcn[0] : -1,
    mph_ncell_req->arfcn[1] NEQ NOT_PRESENT_16BIT ?
      mph_ncell_req->arfcn[1] : -1,
    mph_ncell_req->arfcn[2] NEQ NOT_PRESENT_16BIT ?
      mph_ncell_req->arfcn[2] : -1,
    mph_ncell_req->arfcn[3] NEQ NOT_PRESENT_16BIT ?
      mph_ncell_req->arfcn[3] : -1,
    mph_ncell_req->arfcn[4] NEQ NOT_PRESENT_16BIT ?
      mph_ncell_req->arfcn[4] : -1,
    mph_ncell_req->arfcn[5] NEQ NOT_PRESENT_16BIT ?
      mph_ncell_req->arfcn[5] : -1);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_remove_bad_ncell         |
+--------------------------------------------------------------------+

  PURPOSE : removes outdated ncells from RR store.

*/

GLOBAL void att_remove_bad_rr_data_ncells()
{
  GET_INSTANCE_DATA;
  T_CELL_DATA * cd;
  UBYTE  i_nc, index;
  UBYTE  found;
  USHORT act_ncell_list [MAX_NEIGHBOURCELLS];
  /*
   * get the pointer to the correct list.
   */

  cd = &rr_data->sc_data.cd;
  memset (act_ncell_list, 0xFF, 2*MAX_NEIGHBOURCELLS);
  srv_create_list (&cd->ncell_list, act_ncell_list, MAX_NEIGHBOURCELLS, FALSE, 0);

  for( index = 0; index < SC_INDEX; index++)
  {
    TRACE_EVENT_P3("[%u]i%u bcch_status=%u", rr_data->nc_data[index].arfcn,
                   index, rr_data->nc_data[index].bcch_status);
    if( rr_data->nc_data[index].bcch_status NEQ EMPTY )
    {
      found = FALSE;
      for( i_nc = 0;  act_ncell_list[i_nc] NEQ NOT_PRESENT_16BIT ; i_nc++)
      {
         if( rr_data->nc_data[index].arfcn EQ act_ncell_list[i_nc] )
         {
           found = TRUE;
           break;
         }
      }
      if( ! found )
      {
        TRACE_EVENT_P1(" Removing bad ncell [%u]", rr_data->nc_data[index].arfcn );
        rr_data->nc_data[index].bcch_status = EMPTY;
        rr_data->nc_data[index].arfcn = 0;
        rr_data->nc_data[index].avail_time = 0;
      }
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_code_mph_ncell_req     |
+--------------------------------------------------------------------+

  PURPOSE : build a neighbourcell list for layer 1 in idle mode.

*/

GLOBAL void att_code_mph_ncell_req (UBYTE index)
{
  GET_INSTANCE_DATA;
  T_CELL_DATA * cd;

  PALLOC (mph_ncell_req, MPH_NEIGHBOURCELL_REQ);

  TRACE_FUNCTION ("att_code_mph_ncell_req()");

  /*
   * get the pointer to the correct list.
   */
  if (index EQ SC_INDEX)
    cd = &rr_data->sc_data.cd;
  else
    cd = &rr_data->cr_data.cd;


  cs_remove_BA_MA_from_black_list(rr_data->cs_data.region,&cd->ncell_list);

  /*
   * use neighbour cell list from system info 2/2bis/2ter
   */
  memset (mph_ncell_req->arfcn, 0xFF, 2*MAX_NEIGHBOURCELLS);
  srv_create_list (&cd->ncell_list,
                   mph_ncell_req->arfcn, MAX_NEIGHBOURCELLS, FALSE,
                   0);
  memcpy (rr_data->act_ncell_list, mph_ncell_req->arfcn,
          sizeof (rr_data->act_ncell_list));

#if defined(_SIMULATION_)
  TRACE_EVENT_WIN_P6("ncell_req arfcns:%4d %4d %4d %4d %4d %4d",
    mph_ncell_req->arfcn[0] NEQ NOT_PRESENT_16BIT ?
      mph_ncell_req->arfcn[0] : -1,
    mph_ncell_req->arfcn[1] NEQ NOT_PRESENT_16BIT ?
      mph_ncell_req->arfcn[1] : -1,
    mph_ncell_req->arfcn[2] NEQ NOT_PRESENT_16BIT ?
      mph_ncell_req->arfcn[2] : -1,
    mph_ncell_req->arfcn[3] NEQ NOT_PRESENT_16BIT ?
      mph_ncell_req->arfcn[3] : -1,
    mph_ncell_req->arfcn[4] NEQ NOT_PRESENT_16BIT ?
      mph_ncell_req->arfcn[4] : -1,
    mph_ncell_req->arfcn[5] NEQ NOT_PRESENT_16BIT ?
      mph_ncell_req->arfcn[5] : -1);
#endif

  /*
   * set the multiband parameter
   */
  mph_ncell_req->multi_band = rr_data->ncell_mb;
#ifdef GPRS
  mph_ncell_req->sync_only  = NORMAL_BA;
#endif
  /*
   * check the list against band restrictions.
   */
  srv_remove_frequencies_in_array (&mph_ncell_req->arfcn[0]);

  att_reorder_mph_ncell_req(mph_ncell_req);
  /*
   * send the list to layer 1
   */
  PSENDX (PL, mph_ncell_req);
#if defined (REL99) && defined (TI_PS_FF_EMR)
  for_check_and_configure_si2quater(index);
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147) MODULE  : RR_ATT                           |
| STATE   : code          ROUTINE : att_code_mph_ncell_req_dedicated |
+--------------------------------------------------------------------+

  PURPOSE : builds a neighbourcell list for layer 1 in dedicated mode.

*/

GLOBAL void att_code_mph_ncell_req_dedicated (void)
{
  GET_INSTANCE_DATA;
  UBYTE start_index = 0;
  UBYTE old_index;

  if ( rr_data->emo_arfcn EQ NULL )
  {
#if defined (REL99) && defined (TI_PS_FF_EMR)
    USHORT     si_read = (USHORT) (SYS_INFO_5_READ | SYS_INFO_5BIS_READ);

    rr_data->sc_data.ba_list_ded = FALSE;
    if ( (rr_data->sc_data.cd.sys_info_read & si_read ) EQ si_read )
    {
      rr_data->sc_data.ba_list_ded = TRUE;
      rr_data->sc_data.ba_list_idle = FALSE;
      old_index = rr_data->sc_data.ba_index;
      rr_data->sc_data.ba_index = rr_data->sc_data.new_ba_index;
    }      
    else if (rr_data->sc_data.ba_list_idle EQ TRUE)
      return;
#endif
    {
    PALLOC (mph_ncell_req, MPH_NEIGHBOURCELL_REQ);

    /*
     * initialise data structures,
     * clear ncell list to layer 1,
     * set multiband parameter.
     */
    memset (mph_ncell_req->arfcn, 0xFF, 2*MAX_NEIGHBOURCELLS);
    mph_ncell_req->multi_band = rr_data->ncell_mb;

    /*
     * fill system info 5 and 5bis
     */
    if (rr_data->sc_data.cd.sys_info_read & (SYS_INFO_5_READ | SYS_INFO_5BIS_READ))
      start_index = srv_create_list (&rr_data->sc_data.cd.ncell_list,
                                     mph_ncell_req->arfcn,
                                     MAX_NEIGHBOURCELLS,
                                     FALSE,
                                     0);
    /*
     * fill system info 5ter
     */
    if (rr_data->sc_data.cd.sys_info_read & SYS_INFO_5TER_READ)
      srv_create_list (&rr_data->sc_data.five_ter_list,
                       mph_ncell_req->arfcn,
                       MAX_NEIGHBOURCELLS,
                       FALSE,
                       start_index);
    /*
     * Copy complete list for position detection for uplink
     * measurement reports.
     */
    memcpy (rr_data->act_ncell_list, mph_ncell_req->arfcn,
            sizeof (rr_data->act_ncell_list));

    /*
     * check against band restrictions
     */
    srv_remove_frequencies_in_array (&mph_ncell_req->arfcn[0]);

    /*
     * send list to layer 1.
     */
    PSENDX (PL, mph_ncell_req);
  }
#if defined (REL99) && defined (TI_PS_FF_EMR)
    if (rr_data->sc_data.ba_list_ded NEQ TRUE )
      return;
    att_check_for_si5ter_and_enhpara(old_index);
#endif
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_code_prr_mm_info       |
+--------------------------------------------------------------------+

  PURPOSE : provide MM at the end of cell selection or cell reselection
            with MM relevant data from the BCCH channel. These
            informations are:

            IMSI attach / detach
            Call Reestablishment capability
            Base Station Identification Code
            Periodic Location Updating Time T3212
            and whether the MS camps in a forbidden location area.

*/

static void att_code_prr_mm_info (T_mm_info *mm_info)
{
  GET_INSTANCE_DATA;
  T_NC_DATA *rrd = &rr_data->nc_data[SC_INDEX];

  TRACE_FUNCTION ("att_code_prr_mm_info()");

  mm_info->valid          = TRUE;
  mm_info->att            = rrd->control_descr.att;
  mm_info->re             = rrd->rach.re;
  mm_info->ncc            = (rrd->bsic >> 3) & 7;
  mm_info->bcc            = rrd->bsic & 7;
  mm_info->t3212          = rrd->control_descr.t3212;
  mm_info->la             = (!(dat_forb_lai_check (SC_INDEX) AND
                               dat_roam_forb_lai_check (SC_INDEX)));

  /*
   * Parameter band needs a dummy value, otherwise component tests of RR
   * fail due to lack of ability of TAP
   */
  mm_info->band           = BND_DMY_VAL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_code_rr_abort_ind      |
+--------------------------------------------------------------------+

  PURPOSE : Indication of a failed cell selection or cell reselection
            to MM.

*/
GLOBAL void att_code_rr_abort_ind_original (T_RR_DATA *rr_dat, USHORT    cause)
{
  GET_INSTANCE_DATA;
  int i;
  PALLOC (abort_ind, RR_ABORT_IND);/* T_RR_ABORT_IND */

  TRACE_FUNCTION ("att_code_rr_abort_ind()");

  memset (abort_ind, 0, sizeof (*abort_ind));

  /*
   * Indicates the end of a plmn available search
   */
  if (rr_dat->ms_data.req_mm_service EQ FUNC_NET_SRCH_BY_MMI)
  {
    /* this sets the state of CS */
    cs_set_stop_active();
  }

  TIMERSTOP(TABORT);
   /*
    * Set the parameters for MM, especially the requested service
    * from MM and the real reached service of RR.
    */
  abort_ind->op.v_op    = 1;
  abort_ind->op.ts      = (rr_dat->ms_data.operation_mode >> SHIFT_FOR_SIM_TYPE) & 1;
  abort_ind->op.m       = (rr_dat->ms_data.operation_mode >> SHIFT_FOR_SEARCH_OFFSET) & 1;
  abort_ind->op.sim_ins = (rr_dat->ms_data.operation_mode >> SHIFT_FOR_SIM_INSERTED) & 1;
  abort_ind->op.func    = rr_dat->ms_data.req_mm_service;
  abort_ind->op.service = rr_dat->ms_data.rr_service;
  abort_ind->cause      = cause;

  if(rr_dat->ms_data.rr_service EQ NO_SERVICE)
    att_notify_stop_plmn_search (TRUE);
  else
    att_notify_stop_plmn_search (FALSE);

  /*
   * resume normal RR operation and update the previous
   * requested MM and RR services
   */
  att_clear_parallel_search ();

  rr_dat->ms_data.req_mm_service = att_get_func ();
  rr_data->cs_data.scan_mode = CS_NO_SCAN;
  TRACE_EVENT_P1 ("cause=%x", cause);

  if (cause EQ RRCS_ABORT_CEL_SEL_FAIL)
  {
#ifdef GPRS
    if (abort_ind->op.func    NEQ FUNC_NET_SRCH_BY_MMI AND
        abort_ind->op.service EQ  NO_SERVICE           AND
        att_gprs_is_avail())
    {
      att_signal_gprs_support();
    }
#endif

    /*
     * If the abort cause is cell selection failed
     * add the list of available PLMNs.
     */
    att_order_plmns ();
    abort_ind->plmn_avail = rr_dat->sc_data.found_entries;
    for (i=0; i < rr_dat->sc_data.found_entries; i++)
    {
      T_FOUND_ELEMENT *pfound = &rr_dat->sc_data.found[i];

      abort_ind->plmn[i] = pfound->plmn;
      abort_ind->rxlevel[i] = pfound->rxlev;
      abort_ind->lac_list[i] =pfound->lac; //LOL 02.01.2003: added for EONS support

      TRACE_EVENT_P9 ("RR_ABORT_IND: [%u] MCC/MNC=%x%x%x/%x%x%x lac=%04x rx=%u",
        pfound->arfcn,
        pfound->plmn.mcc[0],
        pfound->plmn.mcc[1],
        pfound->plmn.mcc[2],
        pfound->plmn.mnc[0],
        pfound->plmn.mnc[1],
        pfound->plmn.mnc[2],
        pfound->lac,
        pfound->rxlev);
    }

    EM_NET_SEARCH_PASSED;
  }
  else
  {
    /*
     * else clear this area of the primitive.
     */
    abort_ind->plmn_avail = 0;
    memset (abort_ind->plmn, 0, sizeof (abort_ind->plmn));
    memset (abort_ind->rxlevel, 0, sizeof (abort_ind->rxlevel));

    EM_NET_SEARCH_FAILED;
  }

  /*
   * Set mobile RF capability for MM messages
   */
  if (rr_dat->ms_data.rr_service EQ NO_SERVICE)
    abort_ind->power = 0;
  else
    abort_ind->power = att_get_power();

  if ((rr_data->ms_data.req_mm_service NEQ FUNC_NET_SRCH_BY_MMI) AND
      (cause EQ RRCS_ABORT_CEL_SEL_FAIL))
    att_set_rr_service_info();

  TRACE_OP_TYPE (&abort_ind->op, "RR_ABORT_IND");
  PSENDX (MM, abort_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_order_plmns            |
+--------------------------------------------------------------------+

  PURPOSE : The function orders the found PLMNs after the following
            criterions:

            all PLMNs with a fieldstrength greater then -85 dBm are
            ordered randomly. This is 50 percent of the requirement
            regarding GSM 3.22, chapter 4.4.3.1. MM checks then
            the HPLMN condition and the preferred PLMN condition.

*/

static void att_order_plmns (void)
{
  GET_INSTANCE_DATA;
  USHORT nr;
  USHORT i;
  /*
   * allocate dynamic memory to build a copy
   * of the list which is ordered randomly
   */
  T_FOUND_LIST * list;

  TRACE_FUNCTION ("att_order_plmns()");

  /*
   * initialize the copy
   */
  MALLOC (list, sizeof (T_FOUND_LIST));
  memset (list, 0, sizeof (T_FOUND_LIST));

  /*
   * get the number of PLMNs which have a fieldstrength
   * greater then -85 dBm.
   */
  i=0;
  while ((nr = att_number_of_plmns_greater_85_dBm()) NEQ 0 )
  {
    /*
     * get a value in the range 0..nr-1
     * i.e. select a PLMN greater -85 randomly
     */
    nr = dat_random (nr);

    /*
     * copy the nr-th plmn and indicate it in the source
     * as copied.
     */
    att_copy_found_plmn (list, nr, i);

    /*
     * increment the number of copied networks.
     */
    i++;
  }

  /*
   * copy all networks with a fieldstrength lower or equal -85 dBm.
   */
  att_copy_plmns_lower_or_equal_85_dBm (list, i);

  /*
   * copy back the randomly sorted list
   */
  for (i=0; i < MAX_PLMN; i++)
    rr_data->sc_data.found[i] = list->element[i];

  /*
   * de-allocate the dynamic memory
   */
  MFREE (list);
}

/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)  MODULE  : RR_ATT                             |
| STATE   : code           ROUTINE : att_number_of_plmns_greater_85_dBm |
+-----------------------------------------------------------------------+

  PURPOSE : The function calculates the number of available PLMNs with a
            fieldstrength higher then -85 dBm.

*/

static USHORT att_number_of_plmns_greater_85_dBm (void)
{
  GET_INSTANCE_DATA;
  USHORT nr = 0;
  USHORT i;

  for (i=0; i< rr_data->sc_data.found_entries;i++)
  {
    if (rr_data->sc_data.found[i].plmn.v_plmn EQ V_PLMN_PRES)
    {
      if (rr_data->sc_data.found[i].rxlev > 25)
      {
        /*
         * -85 dBm = -110 + 25 !
         *
         * increment the number of found plmns.
         */
        nr++;
      }
    }
  }

  /*
   * return the number of PLMNs with a fieldstrength greater than -85 dBm
   */
  return nr;
}

/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)  MODULE  : RR_ATT                             |
| STATE   : code           ROUTINE : att_copy_found_plmn                |
+-----------------------------------------------------------------------+

  PURPOSE : The function copies the n-th plmn with a fieldstrength
            higher then -85 dBm in the found plmn copy.

*/

static void att_copy_found_plmn (T_FOUND_LIST * list,
                                 USHORT n_in_source_85_dBm,
                                 USHORT i_in_copy)
{
  GET_INSTANCE_DATA;
  USHORT x_in_source_85_dBm = 0;
  USHORT i_in_source        = 0;
  T_FOUND_ELEMENT *in_source;

  in_source = &rr_data->sc_data.found[0];
  for (i_in_source=0; i_in_source< rr_data->sc_data.found_entries;
       i_in_source++, in_source++)
  {
    if ((in_source->plmn.v_plmn EQ V_PLMN_PRES) AND (in_source->rxlev > 25))
    {
      /*
       * -85 dBm = -110 + 25 !
       *
       * increment the number of found plmns.
       */
      if (x_in_source_85_dBm EQ n_in_source_85_dBm)
      {
        /*
         * n-th PLMN with fieldstrength greater -85 dBm found
         */
        list->element[i_in_copy] = *in_source;
        i_in_copy ++;
        in_source->plmn.v_plmn = V_PLMN_NOT_PRES;
        return;
      }
      else
        x_in_source_85_dBm++;
    }
  }
}

/*
+-------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)  MODULE  : RR_ATT                               |
| STATE   : code           ROUTINE : att_copy_plmns_lower_or_equal_85_dBm |
+-------------------------------------------------------------------------+

  PURPOSE : The function copies the remaining networks with a fieldstrength
            lower or equal -85 dBm in the found plmn copy.

*/

static void att_copy_plmns_lower_or_equal_85_dBm (T_FOUND_LIST * list,
                                                  USHORT i_in_copy)
{
  GET_INSTANCE_DATA;
  USHORT i_in_source;
  T_FOUND_ELEMENT * in_source = &rr_data->sc_data.found[0];

  for (i_in_source=0; i_in_source< rr_data->sc_data.found_entries;
       i_in_source++, in_source++)
  {
    if (in_source->plmn.v_plmn EQ V_PLMN_PRES)
    {
      /*
       * PLMN with fieldstrength less or equal -85 dBm found
       */
      list->element[i_in_copy] = *in_source;
      i_in_copy ++;
    }
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_code_net_lost          |
+--------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void att_code_net_lost (void)
{
  GET_INSTANCE_DATA;
  PALLOC (abort_ind, RR_ABORT_IND);

  TRACE_FUNCTION ("att_code_net_lost()");

  srv_clear_stored_prim (RR_ESTABLISH_REQ);

  abort_ind->op.v_op    = 1;
  abort_ind->op.ts      = (rr_data->ms_data.operation_mode >> SHIFT_FOR_SIM_TYPE) & 1;
  abort_ind->op.m       = (rr_data->ms_data.operation_mode >> SHIFT_FOR_SEARCH_OFFSET) & 1;
  abort_ind->op.sim_ins = (rr_data->ms_data.operation_mode >> SHIFT_FOR_SIM_INSERTED) & 1;
  abort_ind->op.func    = rr_data->ms_data.req_mm_service;
  abort_ind->op.service = NO_SERVICE;

  abort_ind->cause = RRCS_ABORT_CEL_SEL_FAIL;

  abort_ind->plmn_avail = 0;
  memset (abort_ind->plmn, 0, sizeof(abort_ind->plmn));
  memset (abort_ind->rxlevel, 0, sizeof (abort_ind->rxlevel));

  TRACE_OP_TYPE (&abort_ind->op, "RR_ABORT_IND");
  PSENDX (MM, abort_ind);

  rr_data->old_lai.lac = 0xFFFF;
#ifdef GPRS
  rr_data->old_rac     = NOT_PRESENT_8BIT;
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_code_rr_act_cnf        |
+--------------------------------------------------------------------+

  PURPOSE : The function indicates a successful end of cell selection.
            The cell selection was initiated by MM.

*/

GLOBAL void att_code_rr_act_cnf (void)
{
  GET_INSTANCE_DATA;
  T_NC_DATA * rrd = &rr_data->nc_data[SC_INDEX];

  PALLOC (rr_activate_cnf, RR_ACTIVATE_CNF);

  TRACE_FUNCTION ("att_code_rr_act_cnf()");

  TRACE_EVENT ("cell selection");


  rr_data->net_lost = FALSE;
  
  TIMERSTOP(TABORT);

  /*
   * indicate the requested MM service and the reached RR service
   * to MM.
   */
  rr_activate_cnf->op.v_op    = 1;
  rr_activate_cnf->op.ts      = (rr_data->ms_data.operation_mode >> SHIFT_FOR_SIM_TYPE) & 1;
  rr_activate_cnf->op.m       = (rr_data->ms_data.operation_mode >> SHIFT_FOR_SEARCH_OFFSET) & 1;
  rr_activate_cnf->op.sim_ins = (rr_data->ms_data.operation_mode >> SHIFT_FOR_SIM_INSERTED) & 1;
  rr_activate_cnf->op.func    = rr_data->ms_data.req_mm_service;
  rr_activate_cnf->op.service = rr_data->ms_data.rr_service;

  TRACE_EVENT_P9 ("RR_ACTIVATE_CNF[%u]: rMM=%s RRs=%s MCC/MNC=%x%x%x/%x%x%x",
    rrd->arfcn,
    _rr_str_FUNC[rr_data->ms_data.req_mm_service],
    _rr_str_SERVICE[rr_data->ms_data.rr_service],
    rrd->lai.mcc[0],
    rrd->lai.mcc[1],
    rrd->lai.mcc[2],
    rrd->lai.mnc[0],
    rrd->lai.mnc[1],
    rrd->lai.mnc[2]);

  if(GET_STATE(STATE_ATT) EQ ATT_CS_INIT )
  { /*Boot Time: Indicate only the start of power scan to MM*/
    memset( &rr_activate_cnf->mm_info, 0, sizeof(rr_activate_cnf->mm_info));
    memset( &rr_activate_cnf->plmn, 0, sizeof(rr_activate_cnf->plmn));
    rr_activate_cnf->lac = NOT_PRESENT_16BIT;
    rr_activate_cnf->cid = NOT_PRESENT_16BIT;
    rr_activate_cnf->gprs_indication = NOT_PRESENT_8BIT;
    rr_activate_cnf->power = NOT_PRESENT_8BIT;
  }
  else
  {
  /*
   * stop possibly parallel PLMN search
   */
  att_notify_stop_plmn_search (FALSE);

  /*
   * resume normal RR operation and update the previous
   * requested MM and RR services
   */
  att_clear_parallel_search ();
  rr_data->ms_data.req_mm_service = att_get_func ();
  rr_data->cs_data.scan_mode = CS_NO_SCAN;


  if (rr_data->ms_data.req_mm_service NEQ rr_activate_cnf->op.func)
  {
    TRACE_EVENT_P2 ("rMM=%s RRs=%s resume normal RR operation",
      _rr_str_FUNC[rr_data->ms_data.req_mm_service],
      _rr_str_SERVICE[rr_data->ms_data.rr_service]);
  }
  TRACE_OP_TYPE (&rr_activate_cnf->op, "RR_ACTIVATE_CNF");

  /*
   * indicate the relevant BCCH parameter to MM
   */
  att_code_prr_mm_info (&rr_activate_cnf->mm_info);

  /*
   * indicate the PLMN, Location Area and Cell Identity to NN
   */
  rr_activate_cnf->plmn.v_plmn = V_PLMN_PRES;
  memcpy (rr_activate_cnf->plmn.mcc, rrd->lai.mcc, SIZE_MCC);
  memcpy (rr_activate_cnf->plmn.mnc, rrd->lai.mnc, SIZE_MNC);

#if defined(_SIMULATION_)
/* Implements Measure#32: Row 40 */
  att_print_mcc_mnc(rrd->arfcn, rrd->lai.mcc, rrd->lai.mnc, S2I_STRING("RR_ACTIVATE_CNF"));
#endif

  rr_activate_cnf->lac = rrd->lai.lac;
  rr_activate_cnf->cid = rrd->cell_id;

  EM_PLMN_SRCH_PASSED;

  /*
   * store the current location area and cell id to avoid
   * double signalling to MM
   */
  att_copy_old_lai_rac(SC_INDEX);
#if 0
  memcpy (&rr_data->old_lai, &rr_data->nc_data[SC_INDEX].lai,
                sizeof(T_loc_area_ident));
  rr_data->old_cell_id = rr_data->nc_data[SC_INDEX].cell_id;
#endif

#ifdef GPRS
  /*
   * indicates whether gprs operation is provided by the cell
   */
  /* MS Patch: CQ 24473 */
  rr_activate_cnf->gprs_indication = (rr_data->nc_data[SC_INDEX].rac NEQ 
  NOT_PRESENT_8BIT);
#else
  rr_activate_cnf->gprs_indication = FALSE;
#endif

  /*
   * set the power class needed for some MM messages depending
   * on the frequency standard and the channel number
   */
  rr_activate_cnf->power = att_get_power();

  /*
   * Display the Signal bar after camping on a cell with Full service
   */
#ifdef FF_PS_RSSI
  RX_SetValue (rr_data->nc_data[SC_INDEX].rxlev,
               RX_QUAL_UNAVAILABLE,
               rr_data->nc_data[SC_INDEX].select_para.rxlev_access_min);
#else
  RX_SetValue (rr_data->nc_data[SC_INDEX].rxlev);
#endif
  att_set_rr_service_info();
  }

  PSENDX (MM, rr_activate_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_code_rr_act_ind        |
+--------------------------------------------------------------------+

  PURPOSE : The function indicates a successful end of RR originated
            cell selection or a successful end of cell reselection.

*/

GLOBAL void att_code_rr_act_ind (void)
{
  GET_INSTANCE_DATA;
  T_NC_DATA * rrd = &rr_data->nc_data[SC_INDEX];

  TRACE_FUNCTION ("att_code_rr_act_ind()");

  TRACE_EVENT ("cell reselection");

  rr_data->net_lost = FALSE;

  TIMERSTOP (TABORT);

  /* It is expected that this works, but this means that   */
  /* the la-field in T_mm_info is superflous. This is also */
  /* true with some other variables in T_mm_info.          */

  /*
   * Special case if we have reselected on a cell belonging to a forbidden
   * location area in FS : it should not be possible on a LAC barred with a
   * LU_REJ(#13) ie roaming not allowed but can happen with a LAC barred
   * for regional provision of service ie LU_REJ(#12).
   * Then a RR_ABORT_IND is sent to MM to switch the entity into limited
   * service even if RR is happy with full service. An empty list of
   * available PLMN is reported so that no PLMN selection is triggered
   * by MM if MS is in automatic mode.
   */
  if (rr_data->ms_data.rr_service EQ FULL_SERVICE)
  {
    if (!(dat_forb_lai_check (SC_INDEX) AND
          dat_roam_forb_lai_check (SC_INDEX)))
    {
      /*
       * store the current location area and cell id to notify MM
       * when we are back in service
       */
      att_copy_old_lai_rac(SC_INDEX);
#if 0
      memcpy (&rr_data->old_lai, &rr_data->nc_data[SC_INDEX].lai,
                     sizeof(T_loc_area_ident));
      rr_data->old_cell_id = rr_data->nc_data[SC_INDEX].cell_id;
#endif

      /*
       * Indicate limited service, no PLMN available to MM
       */
      rr_data->sc_data.found_entries    = 0;
      rr_data->ms_data.rr_service       = LIMITED_SERVICE;
      att_code_rr_abort_ind (RRCS_ABORT_CEL_SEL_FAIL);
      rr_data->ms_data.rr_service       = FULL_SERVICE;

      return;
    }
  }

  /* Set the black list search flag, whenever the location area or 
   * routing area changes, so that black list search is started 
   * after location/routing area update
   */
  if(rr_data->dyn_config.bl_cs_en)
  {
    if((!dat_plmn_equal_req (rrd->lai.mcc, rrd->lai.mnc,
                            rr_data->old_lai.mcc, rr_data->old_lai.mnc))
       OR
       (rrd->lai.lac NEQ rr_data->old_lai.lac ))
    {
      /* Location area has changed. This flag shall be used to start black 
       * list search after location area update
       */
      rr_data->cs_data.black_list_search_pending = TRUE;
    }
#ifdef GPRS       
    if(att_gprs_is_avail() AND (GET_STATE(STATE_GPRS) EQ GPRS_PIM_BCCH))
    {
      if((rrd->rac NEQ NOT_PRESENT_8BIT) AND (rrd->rac NEQ rr_data->old_rac))
      {
        /* Routing area has changed. This flag shall be used to start black 
         * list search after routing area update in PIM_BCCH state
         */
        rr_data->cs_data.black_list_search_pending = TRUE;
      }
    }
#endif

    if(rr_data->cs_data.black_list_search_pending)
      TRACE_EVENT("black_list_search_pending : 1");
  }

  /*
   * check the current location area identification and cell identity
   * against the last signalled one to MM. If it is not the same
   * a signalling with RR_ACTIVATE_IND is needed
   */
  if (!dat_plmn_equal_req (rrd->lai.mcc, rrd->lai.mnc,
                           rr_data->old_lai.mcc, rr_data->old_lai.mnc)
      OR
      rrd->lai.lac NEQ rr_data->old_lai.lac
      OR
      rrd->cell_id NEQ rr_data->old_cell_id)
  {
    PALLOC (rr_activate_ind, RR_ACTIVATE_IND);

    /*
     * A cell change has occurred. So a running Immediate assignment reject
     * timer is stopped.
     */
    /* If the MS reselects back onto the same SC (ARFCN) and T3122 is still
    running, it should be left running. If a new cell is selected, this timer
    should is stopped. MM is not informed, because it have already received a
    RR_ABORT_IND and left its state.*/

    TIMERSTOP (T3122);

    /*
     * indicate the requested MM service and the reached RR service
     * to MM.
     */
    rr_activate_ind->op.v_op    = 1;
    rr_activate_ind->op.ts      = (rr_data->ms_data.operation_mode >> SHIFT_FOR_SIM_TYPE) & 1;
    rr_activate_ind->op.m       = (rr_data->ms_data.operation_mode >> SHIFT_FOR_SEARCH_OFFSET) & 1;
    rr_activate_ind->op.sim_ins = (rr_data->ms_data.operation_mode >> SHIFT_FOR_SIM_INSERTED) & 1;
    if (rr_data->ms_data.req_mm_service EQ FUNC_NET_SRCH_BY_MMI)
      rr_activate_ind->op.func  = FUNC_PLMN_SRCH;
    else
      rr_activate_ind->op.func    = rr_data->ms_data.req_mm_service;
    rr_activate_ind->op.service = rr_data->ms_data.rr_service;

    TRACE_OP_TYPE (&rr_activate_ind->op, "RR_ACTIVATE_IND");

    /*
     * stop possibly parallel PLMN search
     */
    att_notify_stop_plmn_search (FALSE);

    /*
     * resume normal RR operation and update the previous
     * requested MM and RR services
     */
    att_clear_parallel_search();
    rr_data->ms_data.req_mm_service = att_get_func ();
    rr_data->cs_data.scan_mode = CS_NO_SCAN;


    /*
     * indicate the relevant BCCH parameter to MM
     */
    att_code_prr_mm_info (&rr_activate_ind->mm_info);

    /*
     * indicate the PLMN, Location Area and Cell Identity to NN
     */
    rr_activate_ind->plmn.v_plmn = V_PLMN_PRES;
    memcpy (rr_activate_ind->plmn.mcc, rrd->lai.mcc, SIZE_MCC);
    memcpy (rr_activate_ind->plmn.mnc, rrd->lai.mnc, SIZE_MNC);
#if defined(_SIMULATION_)
/* Implements Measure#32: Row 40 */
    att_print_mcc_mnc(rrd->arfcn, rrd->lai.mcc, rrd->lai.mnc, S2I_STRING("RR_ACTIVATE_IND"));
#endif
    rr_activate_ind->lac = rrd->lai.lac;
    rr_activate_ind->cid = rrd->cell_id;

    /*
     * store the current location area and cell id to avoid
     * double signalling to MM
     */
    att_copy_old_lai_rac(SC_INDEX);
#if 0
    memcpy (&rr_data->old_lai, &rr_data->nc_data[SC_INDEX].lai,
                  sizeof(T_loc_area_ident));
    rr_data->old_cell_id =  rr_data->nc_data[SC_INDEX].cell_id;
#endif

#ifdef GPRS
    /*
     * indicates whether gprs operation is provided by the cell
     */     
     /* MS Patch: CQ 24473 */
     rr_activate_ind->gprs_indication = (rr_data->nc_data[SC_INDEX].rac NEQ 
     NOT_PRESENT_8BIT);
#else
     rr_activate_ind->gprs_indication = FALSE;
#endif

    /*
     * set the power class needed for some MM messages depending
     * on the frequency standard and the channel number
     */
    rr_activate_ind->power = att_get_power();
    /*
     * Limited to full service reselection
     * TMSI/IMSI data to be refreshed in TIL otherwise no paging possible
     */
    att_mph_identity_req();

    EM_CELL_RESEL_FINISHED;

    EM_FMM_RESEL_END_IND;

    PSENDX (MM, rr_activate_ind);
  }
  else
  {
    /*
     * indicate changed imsi attach status or
     * changed periodic location updating time to MM
     * if it has changed, else it is only signalled to MM.
     */
    PALLOC (sync, RR_SYNC_IND);

    att_code_prr_mm_info (&sync->mm_info);
    sync->ciph              = NOT_PRESENT_8BIT;
    sync->chm.ch_mode       = NOT_PRESENT_8BIT;

    switch (rr_data->sc_data.selection_type)
    {
      case CELL_SELECTION:
      case CELL_RESELECTION:
      case CELL_RESELECTION_NC:
      case CELL_RESELECTION_RACH:
      case CELL_RESELECTION_CR:
        /*
         * cell reselection in idle mode
         */
        sync->synccs            = SYNCCS_IDLE_SELECTION;
        break;
      default:
        /*
         * cell reselection after dedicated mode
         */
        sync->synccs            = SYNCCS_BACK_FROM_DEDICATED;
        break;
    }

    sync->bcch_info.v_bcch  = FALSE;
    PSENDX (MM, sync);
    EM_FMM_RESEL_END_IND;
  }
  att_set_rr_service_info();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_get_power              |
+--------------------------------------------------------------------+

  PURPOSE : get the power class needed for some MM messages depending
            on the frequency standard and the channel number of the
            serving cell.

*/

GLOBAL UBYTE att_get_power (void)
{
  GET_INSTANCE_DATA;
  UBYTE power = 0;
  UBYTE power_idx = (UBYTE)-1;

  TRACE_FUNCTION ("att_get_power()");

  /* new method, use directly the rf capabilities */
  /* coding of power class according to classmark 3 [1..5] */
  switch (std)
  {
    case STD_900:
    case STD_EGSM:
      power_idx = IDX_PWRCLASS_900;
      break;
    case STD_1800:
      power_idx = IDX_PWRCLASS_1800;
      break;

    case STD_DUAL:
    case STD_DUAL_EGSM:
      if (INRANGE(LOW_CHANNEL_1800,rr_data->nc_data[SC_INDEX].arfcn,HIGH_CHANNEL_1800))
        power_idx = IDX_PWRCLASS_1800;
      else
        power_idx = IDX_PWRCLASS_900;
      break;

    case STD_1900:
      power_idx = IDX_PWRCLASS_1900;
      break;

    case STD_850:
      power_idx = IDX_PWRCLASS_850;
      break;

    case STD_DUAL_US:
      if (rr_data->nc_data[SC_INDEX].arfcn < LOW_CHANNEL_1900)
        power_idx = IDX_PWRCLASS_850;
      else
        power_idx = IDX_PWRCLASS_1900;
      break;

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
    case STD_850_1800:
      if (INRANGE(LOW_CHANNEL_1800,rr_data->nc_data[SC_INDEX].arfcn,HIGH_CHANNEL_1800))
        power_idx = IDX_PWRCLASS_1800;
      else
        power_idx = IDX_PWRCLASS_850;
      break;

    case STD_900_1900:
      if (INRANGE(LOW_CHANNEL_1900,rr_data->nc_data[SC_INDEX].arfcn,HIGH_CHANNEL_1900))
        power_idx = IDX_PWRCLASS_1900;
      else
        power_idx = IDX_PWRCLASS_900;
      break;

    case STD_850_900_1800:
      if (INRANGE(LOW_CHANNEL_1800,rr_data->nc_data[SC_INDEX].arfcn,HIGH_CHANNEL_1800))
        power_idx = IDX_PWRCLASS_1800;
      else if (INRANGE(LOW_CHANNEL_850,rr_data->nc_data[SC_INDEX].arfcn,HIGH_CHANNEL_850))
        power_idx = IDX_PWRCLASS_850;
      else
        power_idx = IDX_PWRCLASS_900;
      break;

    case STD_850_900_1900:
      if (INRANGE(LOW_CHANNEL_1900,rr_data->nc_data[SC_INDEX].arfcn,HIGH_CHANNEL_1900))
        power_idx = IDX_PWRCLASS_1900;
      else if (INRANGE(LOW_CHANNEL_850,rr_data->nc_data[SC_INDEX].arfcn,HIGH_CHANNEL_850))
        power_idx = IDX_PWRCLASS_850;
      else
        power_idx = IDX_PWRCLASS_900;
      break;
#endif
  }

  if (power_idx NEQ (UBYTE)-1)
    power = rr_data->ms_data.rf_cap.rf_power.pow_class4[power_idx].pow_class;
  else
    power = 0;

  /*
  TRACE_EVENT_P3 ("power=%u(new),  idx=%d, std=%u", power, power_idx, std);
  */

  if (power)
    power--; /* coding of power class according to classmark 1 and 2 [0..4] */

  return power;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_continue_cell_reselect |
+--------------------------------------------------------------------+

  PURPOSE : A cell reselection candidate has failed. Inside this function
            the next one is searched.

*/

GLOBAL void att_continue_cell_reselect (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_continue_cell_reselect()");

  switch (rr_data->sc_data.selection_type)
  {
    case BACK_FROM_DEDICATED:
    case BACK_FROM_DEDICATED_RLF:
      /*
       * After a dedicated connection
       */
      rr_data->reselect_index =
        att_get_next_highest_c2_idx (rr_data->reselect_index);
      att_select_cell_dedicated ();
      break;

    default:
      /*
       * In idle mode
       */
      #ifdef GPRS
      if( rr_data->gprs_data.use_c31)
      {
         rr_data->reselect_index =  att_get_next_best_c32_index(TRUE);
         if (! att_check_cell_c31())
           att_try_old_cell();
      }
      else
      {
      #endif
        rr_data->reselect_index =
          att_get_next_highest_c2_idx (rr_data->reselect_index);
        /*
          * If no other cell is available, try the old one
          */
        if (! att_check_cell ())
        {
          TRACE_EVENT ("no further cell found -> try old one");
          att_try_old_cell ();
        }
        else
        {
          TRACE_EVENT_P2 ("try next cell [%u]i%u",
            rr_data->nc_data[rr_data->reselect_index].arfcn,
            rr_data->reselect_index);
        }

      #ifdef GPRS
      }
      #endif
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_copy_rach_parameter    |
+--------------------------------------------------------------------+

  PURPOSE : After reception of system information message 1 to 4 in
            idle mode or cell selection / reselection the RACH parameters
            are stored. The flag indicate_changes defines whether
            changes in the RACH parameter shall be forwarded to MM
            (in idle mode for serving cell) or not (any other cases).

*/

static void att_copy_rach_parameter (UBYTE        index,
                                     T_rach_ctrl *rach,
                                     UBYTE        indicate_changes)
{
  GET_INSTANCE_DATA;
  T_NC_DATA *rrd = &rr_data->nc_data[index];

  TRACE_FUNCTION ("att_copy_rach_parameter()");

  if (index EQ SC_INDEX AND
      indicate_changes)
  {
    /*
     * compare only for serving cell
     */
    if (rrd->rach.re NEQ rach->re)
    {
      /*
       * change of reestablishment flag indicate to MM
       */
      PALLOC (sync, RR_SYNC_IND);
      rrd->rach.re = rach->re;

      att_code_prr_mm_info (&sync->mm_info);
      sync->ciph              = NOT_PRESENT_8BIT;
      sync->chm.ch_mode       = NOT_PRESENT_8BIT;
      sync->synccs            = SYNCCS_SYS_INFO_CHANGE;
      memset(&sync->bcch_info, 0, sizeof(T_bcch_info));
      sync->bcch_info.v_bcch  = FALSE;
      PSENDX (MM, sync);
    }

    if (rrd->rach.ac NEQ rach->ac)
    {
      /*
       * change of access classes indicate to MM
       */
      PALLOC (sync, RR_SYNC_IND);

      sync->ciph              = NOT_PRESENT_8BIT;
      sync->chm.ch_mode       = NOT_PRESENT_8BIT;
      sync->synccs            = SYNCCS_ACC_CLS_CHA;
      memset(&sync->mm_info,   0, sizeof(T_mm_info));
      sync->mm_info.valid     = FALSE;
      memset(&sync->bcch_info, 0, sizeof(T_bcch_info));
      sync->bcch_info.v_bcch  = FALSE;
      PSENDX (MM, sync);
    }
  }

  rrd->rach = *rach;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_copy_sys_info_1_par    |
+--------------------------------------------------------------------+

  PURPOSE : copy the relevant system information type 1 parameter after
            reception in idle mode or during cell selection or cell
            reselection.

*/

GLOBAL void att_copy_sys_info_1_par (UBYTE           index,
                                     T_D_SYS_INFO_1 *sys_info_1,
                                     T_LIST         *cell_chan_desc)
{
  GET_INSTANCE_DATA;
  T_CELL_DATA * cd;
  UBYTE indicate_changes = index EQ SC_INDEX;

  switch (index)
  {
    case SC_INDEX:
    case CR_INDEX:
      /*
       * set pointer to data
       */
      if (index EQ SC_INDEX)
          cd = &rr_data->sc_data.cd;
      else
          cd = &rr_data->cr_data.cd;

      /*
       * If a cell channel description is not stored for
       * this cell or the new cell channel description is
       * different, store the new one.
       */
      if (cd->v_cell_chan_desc EQ NO_CONTENT OR
          srv_compare_list (&cd->cell_chan_desc,
                            cell_chan_desc) EQ FALSE)
      {
        srv_copy_list (&cd->cell_chan_desc,
                       cell_chan_desc,
                       sizeof (T_LIST));

        cd->v_cell_chan_desc = WITH_CHANGED_CONTENT;
        if((indicate_changes) AND (GET_STATE(STATE_ATT) EQ ATT_IDLE)
            AND ( rr_data->sc_data.cd.cbch_chan_desc.hop EQ H_FREQ) )
        {
#ifdef REL99
          att_config_cbch();
#else
          att_build_cbch();
#endif
        }
      }
      /* Get the band indicator value from SI1 message */
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
      cd->band_indicator = sys_info_1->si1_rest_oct.band_indicator;
      if (index EQ SC_INDEX)
      {
        att_update_std_band_indicator (cd->band_indicator);
      }
#endif
      break;
  }

  att_copy_rach_parameter (index, &sys_info_1->rach_ctrl, indicate_changes);

  /*
   * Indicate that the system information type 1 message has been read.
   */
  att_set_sys_info_read (SYS_INFO_1_READ, index);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_copy_sys_info_2bis_par |
+--------------------------------------------------------------------+

  PURPOSE : copy the relevant system information type 2bis parameter after
            reception in idle mode (in this case changes are to detect)
            or during cell selection or cell reselection.

*/

GLOBAL void att_copy_sys_info_2bis_par (UBYTE              index,
                                        T_D_SYS_INFO_2BIS *sys_info_2bis,
                                        T_LIST            *new_2bis_list,
                                        UBYTE              ncell_ext,
                                        UBYTE              indicate_changes)
{
/* Implements RR Clone findings #3 */

  TRACE_FUNCTION ("att_copy_sys_info_2bis_par()");


/* Implements RR Clone findings #3 */
  att_copy_sys_info_2bis_2ter_par(index, SI_TYPE_2BIS, new_2bis_list,
                                  &sys_info_2bis->neigh_cell_desc,
                                  indicate_changes);



  att_copy_rach_parameter (index, &sys_info_2bis->rach_ctrl, indicate_changes);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_copy_sys_info_2_par    |
+--------------------------------------------------------------------+

  PURPOSE : copy the relevant system information type 2 parameter after
            reception in idle mode (in this case changes are to detect)
            or during cell selection or cell reselection.

*/

GLOBAL void att_copy_sys_info_2_par (UBYTE           index,
                                     T_D_SYS_INFO_2 *sys_info_2,
                                     T_LIST         *new_2_list,
                                     UBYTE           ncell_ext,
                                     UBYTE           indicate_changes)
{
  GET_INSTANCE_DATA;
  T_LIST        new_list;
  T_CELL_DATA   *cd;
  BOOL          modified = FALSE;
  
  TRACE_FUNCTION ("att_copy_sys_info_2_par()");

  switch (index)
  {
    case SC_INDEX:
    case CR_INDEX:
      if (index EQ SC_INDEX)
      {
        cd = &rr_data->sc_data.cd;
        if ((cd->sys_info_read & SYS_INFO_2_READ) EQ SYS_INFO_2_READ)
        {
          srv_copy_list (&cd->ncell_list, new_2_list, sizeof (T_LIST));
          att_clean_buf ((USHORT)(IND_SI_2BIS | IND_SI_2TER));
          cd->sys_info_read &= ~(SYS_INFO_2BIS_READ | SYS_INFO_2TER_READ);
          att_check_2ter_read (index);
        }
        else
          srv_merge_list (&cd->ncell_list, new_2_list);
    
        if( cd->ncell_ext NEQ ncell_ext )
        {
          PALLOC (mph_sync_req, MPH_SYNC_REQ);
          mph_sync_req->cs = CS_SYS_INFO_2BIS_SUPPORT_CHANGED;
          PSENDX (PL, mph_sync_req);
          cd->ncell_ext = ncell_ext;
        }
        modified = TRUE;
#ifdef GPRS
        rr_data->gprs_data.ba_bcch_modified= FALSE;
#endif
      }
      else
      {
        cd = &rr_data->cr_data.cd;
        /*
         * merge 2 list with the old neighbour cell list
         */
        srv_copy_list (&new_list, &cd->ncell_list,
                       sizeof (T_LIST));
        srv_merge_list (&new_list, new_2_list);

        if (srv_compare_list (&cd->ncell_list, &new_list) EQ FALSE)
        {
          /*
           * both lists are different
           */
          srv_copy_list (&cd->ncell_list, &new_list, sizeof (T_LIST));
          modified = TRUE;
        }
        cd->ncell_ext = ncell_ext;
      }

      /* CSI-LLD section:4.1.1.11
       * This function Updates the black list with the BA list received in si2
       */
      cs_remove_BA_MA_from_black_list(rr_data->cs_data.region,&cd->ncell_list);

      dat_store_neigh_cell_desc (SYS_INFO_2_MSG, index,
        &sys_info_2->neigh_cell_desc,&cd->ncell_list);
      /*
       * Indicate that the system information type 2 message has been read
       * and if applicable the system information type 2bis is not needed to read.
       */
      att_set_sys_info_read (SYS_INFO_2_READ, index);
      if (ncell_ext EQ 0)
        att_set_sys_info_read (SYS_INFO_2BIS_READ, index);
      /*
       * forward new neighbour cell list to layer 1 if
       * changes shall be indicated
       */
      if (modified AND indicate_changes)
      {
        att_code_mph_ncell_req (index);
      }

      /*
       * copy ncc permitted field
       */
      cd->ncc_permitted = sys_info_2->ncc_permit;
      break;
  }
  att_copy_rach_parameter (index, &sys_info_2->rach_ctrl, indicate_changes);

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_copy_sys_info_2ter_par |
+--------------------------------------------------------------------+

  PURPOSE : copy the relevant system information type 2ter parameter after
            reception in idle mode or during cell selection or cell
            reselection.

*/
GLOBAL void att_copy_sys_info_2ter_par (UBYTE              index,
                                        T_D_SYS_INFO_2TER *sys_info_2ter,
                                        T_LIST            *new_2ter_list,
                                        UBYTE              indicate_changes)
{
    
  /* Implements RR Clone findings #3 */
  
  TRACE_FUNCTION ("att_copy_sys_info_2ter_par()");

/* Implements RR Clone findings #8 */
  att_copy_sys_info_2bis_2ter_par(index, SI_TYPE_2TER, new_2ter_list,
                                   &sys_info_2ter->neigh_cell_desc,
                                   indicate_changes);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_check_2ter_read        |
+--------------------------------------------------------------------+

  PURPOSE : checks whether to wait for SI_2ter.
*/

static void att_check_2ter_read (UBYTE index)
{
  GET_INSTANCE_DATA;
  if (rr_data->nc_data[index].c2_par.two_ter EQ FALSE)
    att_set_sys_info_read (SYS_INFO_2TER_READ, index);

  /*
   * only in dualband standards 2ter is taken in account
   * in single bands it is ignored.
   */
  switch (std)
  {
    case STD_900:
    case STD_EGSM:
    case STD_1800:
    case STD_850:
    case STD_1900:
      att_set_sys_info_read (SYS_INFO_2TER_READ, index);
      break;
    default:
      break;
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_copy_sys_info_3_par    |
+--------------------------------------------------------------------+

  PURPOSE : copy the relevant system information type 3 parameter after
            reception in idle mode or during cell selection or cell
            reselection.
*/

/*
 * conversion from air interface coding for DTX usage to internal
 * notation
 */
static const UBYTE dtx_bcch [3] = { DTX_USED, DTX_USED, DTX_NOT_USED };


GLOBAL void att_copy_sys_info_3_par (UBYTE           index,
                                     T_D_SYS_INFO_3 *sys_info_3,
                                     UBYTE           indicate_changes)
{
  GET_INSTANCE_DATA;
  T_NC_DATA * rrd                = &rr_data->nc_data[index];
  UBYTE       lai_changed        = FALSE;
  UBYTE       control_changed_mm = FALSE;
  UBYTE       control_changed_l1 = FALSE;
  TRACE_FUNCTION ("att_copy_sys_info_3_par()");

#if defined(_SIMULATION_)
  TRACE_EVENT_WIN_P7 ( "MCC o/i[%d]= %x%x%x/%x%x%x",
    ((int)rrd->arfcn),
    rrd->lai.mcc[0],
    rrd->lai.mcc[1],
    rrd->lai.mcc[2],
    sys_info_3->loc_area_ident.mcc[0],
    sys_info_3->loc_area_ident.mcc[1],
    sys_info_3->loc_area_ident.mcc[2]);
  TRACE_EVENT_WIN_P7 ( "MNC o/i[%d]= %x%x%x/%x%x%x",
    ((int)rrd->arfcn),
    rrd->lai.mnc[0],
    rrd->lai.mnc[1],
    rrd->lai.mnc[2],
    sys_info_3->loc_area_ident.mnc[0],
    sys_info_3->loc_area_ident.mnc[1],
    sys_info_3->loc_area_ident.mnc[2]);
#endif

  if (index EQ SC_INDEX)
  {
    /*
     * compare only for serving cell
     *
     * check location area information and cell id
     */
    if (rrd->cell_id NEQ sys_info_3->cell_ident)
      lai_changed = TRUE;
    rrd->cell_id = sys_info_3->cell_ident;

    if (!dat_plmn_equal_req (sys_info_3->loc_area_ident.mcc,
                             sys_info_3->loc_area_ident.mnc,
                             rrd->lai.mcc,
                             rrd->lai.mnc))
      lai_changed = TRUE;
    if (rrd->lai.lac NEQ sys_info_3->loc_area_ident.lac)
      lai_changed = TRUE;
#if defined(_SIMULATION_)
    TRACE_EVENT_WIN_P3 ( "LAI[%d] changed=%d indicate_changes=%d",
      rr_data->nc_data[index].arfcn, lai_changed, indicate_changes);
#endif

    if (lai_changed AND indicate_changes)
    {
      /*
       * If a change of location area code is detected,
       * signal this as a cell reselection to MM to force
       * a location updating.
       */
      att_reset_old_lai_rac();
      memcpy (&rrd->lai, &sys_info_3->loc_area_ident,
              sizeof (T_loc_area_ident));
      att_code_rr_act_ind ();
#ifdef GPRS
      if (att_gprs_is_avail())
      {
/*XY: inform GRR, and don't wait for CR_RSP */
        att_rrgrr_cr_ind(CR_ABNORMAL);
        /*
         * we are in the process of acquiring SIs because GRR
         * has requested it. We have to set the sys_info_read bitmap
         * to complete. in order to send a GPRS_SI13_IND to GRR below.
         * The missing SIs which RR requested will still arrive from ALR
         * so it does not matter that we reset the bitmap now.
         */
        att_set_sys_info_read(ALL_SYS_INFO_READ, SC_INDEX);
        rr_data->gprs_data.start_proc = START_PROC_NOTHING;
        att_signal_gprs_support();
      }
#endif
    }

    /*
     * check imsi attach status and
     * periodic location update time
     */
    if ((rrd->control_descr.att NEQ sys_info_3->ctrl_chan_desc.att
        OR
        rrd->control_descr.t3212 NEQ sys_info_3->ctrl_chan_desc.t3212)
        AND
        indicate_changes)
      control_changed_mm = TRUE;

    /*
     * check control channel description
     */
    if (rrd->control_descr.bs_ag_blks_res NEQ
        sys_info_3->ctrl_chan_desc.bs_ag_blks_res)
      control_changed_l1 = TRUE;
    if (rrd->control_descr.ccch_conf NEQ
        sys_info_3->ctrl_chan_desc.ccch_conf)
      control_changed_l1 = TRUE;
    if (rrd->control_descr.bs_pa_mfrms NEQ
        sys_info_3->ctrl_chan_desc.bs_pa_mfrms)
      control_changed_l1 = TRUE;

    /*
     * check cell selection parameters
     */  
    if (rrd->select_para.ms_txpwr_max_cch NEQ
        sys_info_3->cell_select.ms_txpwr_max_cch)
    {
      control_changed_l1 = TRUE;
      TRACE_EVENT_P2("txpwr_max_cch changed from %d -> %d in SI3", rrd->select_para.ms_txpwr_max_cch, 
                                          sys_info_3->cell_select.ms_txpwr_max_cch);
    }
  }

  /*
   * store location information and control channel description
   */
  rrd->cell_id = sys_info_3->cell_ident;
#ifdef REL99
  rrd->mscr_flag = sys_info_3->ctrl_chan_desc.mscr;
#endif
  memcpy (&rrd->lai, &sys_info_3->loc_area_ident,
          sizeof (T_loc_area_ident));
  memcpy (&rrd->control_descr, &sys_info_3->ctrl_chan_desc,
          sizeof (T_ctrl_chan_desc));

  if (control_changed_mm)
  {
    /*
     * indicate changed imsi attach status or
     * changed periodic location updating time to MM
     */
    PALLOC (sync, RR_SYNC_IND);

    att_code_prr_mm_info (&sync->mm_info);
    sync->ciph              = NOT_PRESENT_8BIT;
    sync->chm.ch_mode       = NOT_PRESENT_8BIT;
    sync->synccs            = SYNCCS_SYS_INFO_CHANGE;
    sync->bcch_info.v_bcch  = FALSE;
    PSENDX (MM, sync);
  }

  /*
   * store the system information type 3 message parameter.
   */
  switch (index)
  {
    case SC_INDEX:
      rr_data->sc_data.cd.cell_options.pow_ctrl =
               sys_info_3->cell_opt_bcch.pow_ctrl;
      rr_data->sc_data.cd.cell_options.rlt      =
               sys_info_3->cell_opt_bcch.rlt;
      rr_data->sc_data.cd.dtx                   =
               dtx_bcch [sys_info_3->cell_opt_bcch.dtx_b];
      rr_data->sc_data.cd.dtx_half              =
               rr_data->sc_data.cd.dtx_full        =
               rr_data->sc_data.cd.dtx;
#if defined (REL99) && defined (TI_PS_FF_EMR)
      /*The SI-2quater information that we store is useful for monitoring appropriate
      BCCH periodically: no need to change status in re-acquisition*/      
      if ( sys_info_3->si3_rest_oct.v_si2quater_ind )
      {
        if ( rr_data->sc_data.cd.si2quater_status EQ SI2QUATER_ABSENT)
          rr_data->sc_data.cd.si2quater_status = SI2QUATER_CONFIGURE;
        rr_data->sc_data.cd.si2quater_pos = sys_info_3->si3_rest_oct.si2quater_ind.si2quater_pos;
      }            
#endif
      break;

    case CR_INDEX:
      rr_data->cr_data.cd.cell_options.pow_ctrl =
               sys_info_3->cell_opt_bcch.pow_ctrl;
      rr_data->cr_data.cd.cell_options.rlt      =
               sys_info_3->cell_opt_bcch.rlt;
      rr_data->cr_data.cd.dtx                   =
               dtx_bcch [sys_info_3->cell_opt_bcch.dtx_b];
      rr_data->cr_data.cd.dtx_half              =
               rr_data->cr_data.cd.dtx_full        =
               rr_data->cr_data.cd.dtx;
#if defined (REL99) && defined (TI_PS_FF_EMR)
      /*The SI-2quater information that we store is used only if this cell is selected
      In case of serving cell, this information should be useful for monitoring appropriate
      BCCH periodically*/
      rr_data->cr_data.cd.si2quater_status = SI2QUATER_ABSENT;
      if ( sys_info_3->si3_rest_oct.v_si2quater_ind )
      {
        rr_data->cr_data.cd.si2quater_status = SI2QUATER_CONFIGURE;
        rr_data->cr_data.cd.si2quater_pos = sys_info_3->si3_rest_oct.si2quater_ind.si2quater_pos;
      }
#endif
      break;
  }

  memcpy (&rrd->select_para, &sys_info_3->cell_select,
          sizeof (T_cell_select));
  att_copy_rach_parameter (index, &sys_info_3->rach_ctrl,
                           indicate_changes);


  att_copy_c2_parameter_si3 (index, &sys_info_3->si3_rest_oct);

  att_set_sys_info_read (SYS_INFO_3_READ, index);
  att_check_2ter_read (index);

  if (control_changed_l1 AND indicate_changes)
  {
    /*
     * forward changed control channel description
     * to layer 1
     */
    att_build_idle_req (SC_INDEX, MODE_SYS_INFO_CHANGE);
#ifdef REL99
    att_config_cbch ();
#else
    att_build_cbch();
#endif
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_copy_sys_info_4_par    |
+--------------------------------------------------------------------+

  PURPOSE : copy the relevant system information type 4 parameter after
            reception in idle mode or during cell selection or cell
            reselection.
*/

GLOBAL void att_copy_sys_info_4_par (UBYTE           index,
                                     T_D_SYS_INFO_4 *sys_info_4,
                                     UBYTE           indicate_changes)
{
  GET_INSTANCE_DATA;
  T_NC_DATA   * rrd         = &rr_data->nc_data[index];
  UBYTE         lai_changed = FALSE;
  UBYTE       control_changed_l1 = FALSE;
  T_CELL_DATA * cd;

  TRACE_FUNCTION ("att_copy_sys_info_4_par()");

  switch (index)
  {
    case SC_INDEX:
    case CR_INDEX:
      /*
       * set pointer to data
       */
      if (index EQ SC_INDEX)
          cd = &rr_data->sc_data.cd;
      else
          cd = &rr_data->cr_data.cd;

      if (index EQ SC_INDEX)
      {
        /*
         * compare only for serving cell
         *
         * check location information
         */
        if (!dat_plmn_equal_req (sys_info_4->loc_area_ident.mcc,
                                 sys_info_4->loc_area_ident.mnc,
                                 rrd->lai.mcc,
                                 rrd->lai.mnc))
          lai_changed = TRUE;
        if (rrd->lai.lac NEQ sys_info_4->loc_area_ident.lac)
          lai_changed = TRUE;

        if (lai_changed AND indicate_changes)
        {
          /*
           * If a change of location area code is detected,
           * signal this as a cell reselection to MM to force
           * a location updating.
           */
          att_reset_old_lai_rac();
          memcpy (&rrd->lai, &sys_info_4->loc_area_ident,
                  sizeof (T_loc_area_ident));
          att_code_rr_act_ind ();
#ifdef GPRS
         if (att_gprs_is_avail())
         {
/*XY: inform GRR, and don't wait for CR_RSP */
           att_rrgrr_cr_ind(CR_ABNORMAL);
           /*
            * we are in the process of acquiring SIs because GRR
            * has requested it. We have to set the sys_info_read bitmap
            * to complete. in order to send a GPRS_SI13_IND to GRR below.
            * The missing SIs which RR requested will still arrive from ALR
            * so it does not matter that we reset the bitmap now.
            */
           att_set_sys_info_read(ALL_SYS_INFO_READ, SC_INDEX);
           rr_data->gprs_data.start_proc = START_PROC_NOTHING;
           att_signal_gprs_support();
         }
#endif
        }

      /*
       * check cell selection parameters
       */  
       if (rrd->select_para.ms_txpwr_max_cch NEQ
           sys_info_4->cell_select.ms_txpwr_max_cch)
       {
         control_changed_l1 = TRUE;
         TRACE_EVENT_P2("txpwr_max_cch changed from %d -> %d in SI4", rrd->select_para.ms_txpwr_max_cch, 
                                            sys_info_4->cell_select.ms_txpwr_max_cch);
       }
      }

      if (sys_info_4->v_chan_desc)
      {
        /*
         * the message contains a CBCH channel description.
         */
        if(memcmp(&cd->cbch_chan_desc,&sys_info_4->chan_desc, sizeof (T_chan_desc)) NEQ 0)
        {
          memcpy (&cd->cbch_chan_desc, &sys_info_4->chan_desc, sizeof (T_chan_desc));
          cd->cbch_chan_desc_avail = WITH_CONTENT;
          att_bits_to_byte (cd->cbch_mob_alloc,
                          sys_info_4->mob_alloc.c_mac,
                          sys_info_4->mob_alloc.mac);
          if( (index EQ SC_INDEX) AND ( GET_STATE(STATE_ATT) EQ ATT_IDLE))
#ifdef REL99
            att_config_cbch();
#else
            att_build_cbch();
#endif
        }
      }
      else
      {
        memset (&cd->cbch_chan_desc, 0, sizeof (T_chan_desc));
        if( cd->cbch_chan_desc_avail NEQ NO_CONTENT)
        {
          cd->cbch_chan_desc_avail = NO_CONTENT;
          if( (index EQ SC_INDEX) AND ( GET_STATE(STATE_ATT) EQ ATT_IDLE))
#ifdef REL99
            att_config_cbch();
#else
            att_build_cbch();
#endif
        }
      }

      /*
       * set system information bits
       */
      att_set_sys_info_read (SYS_INFO_4_READ, index);
      break;
    default:
      break;
  }

  /*
   * copy the parameters
   */
  rrd->v_acs       = sys_info_4->cell_select.acs;    /* use of sys info 7&8 */
  rrd->lai         = sys_info_4->loc_area_ident;
  rrd->select_para = sys_info_4->cell_select;
  att_copy_rach_parameter (index, &sys_info_4->rach_ctrl, indicate_changes);
  
  if (control_changed_l1 AND indicate_changes)
  {
    /*
     * forward changed control channel description
     * to layer 1
     */
    att_build_idle_req (SC_INDEX, MODE_SYS_INFO_CHANGE);
    att_build_cbch ();
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_copy_sys_info_7_8_par  |
+--------------------------------------------------------------------+

  PURPOSE : copy the relevant system information type 7 and 8
            parameter after reception in idle mode for neighbourcells.
            The messages  are only expected if system information
            message 4 does not contain the C2 parameter due to a big
            CBCH channel description and system information
            message 3 is hidden by the own paging block.

*/

GLOBAL void att_copy_sys_info_7_8_par (UBYTE           index,
                                       T_D_SYS_INFO_8 *sys_info_8)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_copy_sys_info_7_8_par()");

  /*
   * system info 4 does not contain enough information
   */
  if (rr_data->nc_data[index].v_acs)
     att_copy_c2_parameter_si4 (index, (T_si4_rest_oct *)&sys_info_8->si8_rest_oct);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_copy_c2_parameter      |
+--------------------------------------------------------------------+

  PURPOSE : Rest Octets of System Information Type 3/4/7/8 are decoded
            to get the C2 parameter for cell reselection.
*/

static void att_copy_c2_parameter (T_C2_PARAMETER *c2_par,
                                   T_si3_rest_oct *rest_oct )
{
  TRACE_FUNCTION ("att_copy_c2_parameter()");

  /* Optional selection parameters */
  if(rest_oct->v_opt_sel_par)
  {
    c2_par->param_ind            = TRUE;
    c2_par->cbq                  = rest_oct->opt_sel_par.cell_bar_qual;
    c2_par->cell_reselect_offset = rest_oct->opt_sel_par.cell_resel_offs;
    c2_par->temp_offset          = rest_oct->opt_sel_par.temp_offs;
    c2_par->penalty_time         = rest_oct->opt_sel_par.penalty_time;
  }
  else
  {
    c2_par->param_ind            = FALSE;
    c2_par->cbq                  = 0;
    c2_par->cell_reselect_offset = 0;
    c2_par->temp_offset          = 0;
    c2_par->penalty_time         = 0;
  }

  /* Optional Power Offset */
  if(rest_oct->v_pow_offs)
  {
    c2_par->power_off_ind = TRUE;
    c2_par->power_off     = rest_oct->pow_offs;
  }
  else
  {
    c2_par->power_off_ind = FALSE;
    c2_par->power_off     = 0;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_copy_c2_parameter_si3  |
+--------------------------------------------------------------------+

  PURPOSE : rest octets of system information 3 are decoded
            to get the C2 parameter for cell reselection.
*/

static void att_copy_c2_parameter_si3 (UBYTE           index,
                                       T_si3_rest_oct *rest_oct)
{
  GET_INSTANCE_DATA;
  T_NC_DATA      * nc     = &rr_data->nc_data[index];
  T_C2_PARAMETER * c2_par = &nc->c2_par;

  TRACE_FUNCTION ("att_copy_c2_parameter_si3()");

  att_copy_c2_parameter (c2_par, rest_oct);

  if(index EQ SC_INDEX AND
     c2_par->two_ter NEQ rest_oct->v_si2ter_ind)
  {
    PALLOC (mph_sync_req, MPH_SYNC_REQ);
    mph_sync_req->cs = CS_SYS_INFO_2TER_SUPPORT_CHANGED;
    PSENDX (PL, mph_sync_req);
  }  
  /* System Information 2ter Indicator */
  if (rest_oct->v_si2ter_ind)
  {
    c2_par->two_ter = TRUE;
  }
  else
  {
    c2_par->two_ter = FALSE;
  }

  /* Early Classmark Sending Control */
  if (rest_oct->v_es_ind_tag)
  {
    c2_par->ecsc = TRUE;
  }
  else
  {
    c2_par->ecsc = FALSE;
  }

#ifdef GPRS
  if (rest_oct->v_gprs_indic)
    nc->rac = rest_oct->gprs_indic.ra_color;
  else
    nc->rac = NOT_PRESENT_8BIT;
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_copy_c2_parameter_si4  |
+--------------------------------------------------------------------+

  PURPOSE : Rrest Octets of System Information Type 4 are decoded
            to get the C2 parameter for cell reselection.

*/

GLOBAL void att_copy_c2_parameter_si4 (UBYTE           index,
                                       T_si4_rest_oct *rest_oct)
{
  GET_INSTANCE_DATA;
  T_NC_DATA      * nc     = &rr_data->nc_data[index];
  T_C2_PARAMETER * c2_par = &nc->c2_par;

  TRACE_FUNCTION ("att_copy_c2_parameter_si4()");

  att_copy_c2_parameter (c2_par, (T_si3_rest_oct *)rest_oct);

#ifdef GPRS
  if (rest_oct->v_gprs_indic)
    nc->rac = rest_oct->gprs_indic.ra_color;
  else
    nc->rac = NOT_PRESENT_8BIT;
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_get_highest_c2_index   |
+--------------------------------------------------------------------+

  PURPOSE : If a cell reselection shall be performed, a ranking
            of the available candidates is done. Depending on the
            calculated C2 values a C2-Index is used (because more
            than one cell can have the same C2-value or the value
            changes during cell reselection). This function looks
            for the cell with the highest c2 index.

*/

static UBYTE att_get_highest_c2_index (void)
{
  GET_INSTANCE_DATA;
  UBYTE max_result = SC_INDEX;
  UBYTE next_result;
  UBYTE result;
  UBYTE i;

  TRACE_FUNCTION ("att_get_highest_c2_index()");

  switch (rr_data->sc_data.selection_type)
  {
    case BACK_FROM_DEDICATED:
    case BACK_FROM_DEDICATED_RLF:
      /*
       * prepare cell reselection list
       */
      for (i = 0; i < 7; i++)
      {
        rr_data->nc_data[i].c2_used = FALSE;
        if (rr_data->nc_data[i].bcch_status NEQ EMPTY)
          if (rr_data->nc_data[i].rxlev > rr_data->nc_data[max_result].rxlev)
            max_result = i;
      }

      if (rr_data->sc_data.selection_type EQ BACK_FROM_DEDICATED)
      {
        /*
         * In case of normal release start with serving cell
         */
        rr_data->nc_data[SC_INDEX].c2_used = TRUE;
        max_result                         = SC_INDEX;
        rr_data->ms_data.c2_tab[SC_INDEX]  = MAX_C2;
        result = SC_INDEX;
      }
      else
      {
        /*
         * In case of call re-establishment start with highest fieldstrength
         */
        rr_data->nc_data[max_result].c2_used = TRUE;
        rr_data->ms_data.c2_tab[max_result]  = MAX_C2;
        result = max_result;
      }
      /*
       * Fill with the rest of neighbourcells
       */
      while ((next_result = att_get_next_highest_rx ())
              NEQ NO_AVAILABLE)
      {
        rr_data->ms_data.c2_tab[next_result] =
          rr_data->ms_data.c2_tab[result] - 1;
        result = next_result;
      }
      break;

    case CELL_RESELECTION_CR:
      /*
       * Start with CR cell (from idle mode to another cell)
       */
      rr_data->nc_data[CR_INDEX].c2_used = TRUE;
      max_result                         = CR_INDEX;
      result                             = max_result;
      rr_data->ms_data.c2_tab[CR_INDEX]  = MAX_C2;
      /*
       * Fill with the rest of neighbourcells
       */
      while ((next_result = att_get_next_highest_rx ())
              NEQ NO_AVAILABLE)
      {
        rr_data->ms_data.c2_tab[next_result] =
          rr_data->ms_data.c2_tab[result] - 1;
        result = next_result;
      }
      break;

    default:
      /*
       * Normal Cell reselection
       *
       * use a corrected C2 value for calculation,
       * because for cell reselection due to C2(NC) > C2(SC) a
       * threshold value must be taken in account if serving
       * and neighbourcell are members of different location areas.
       */
      att_calculate_c2 (SC_INDEX);
      rr_data->nc_data[SC_INDEX].c2_corr = rr_data->nc_data[SC_INDEX].c2;

      /*
       * Calculate the corrected C2 values for all available neighbourcells.
       */
      for (i = 0; i < 6; i++)
      {
        if (rr_data->nc_data[i].bcch_status EQ DECODED)
        {
          att_calculate_c2 (i);
          rr_data->nc_data[i].c2_corr = rr_data->nc_data[i].c2;

          /*
           * Take the cell reselection hysterese threshold value in
           * account, if there are different location areas,
           * a cell reselection due to C2(NC) > C2(SC)
           */
#ifdef GPRS
          /*
           * check if the location area has changed
           * or the routing area if the scell supports GPRS
           */
          {
            BOOL la_changed, ra_changed;

            if (!dat_plmn_equal_req (rr_data->nc_data[i].lai.mcc,
                                     rr_data->nc_data[i].lai.mnc,
                                     rr_data->nc_data[SC_INDEX].lai.mcc,
                                     rr_data->nc_data[SC_INDEX].lai.mnc)
                OR
                rr_data->nc_data[i].lai.lac NEQ
                rr_data->nc_data[SC_INDEX].lai.lac)
              la_changed = TRUE;
            else
              la_changed = FALSE;

            if(att_gprs_is_avail())
            {
              if(rr_data->nc_data[i].rac NEQ
                rr_data->nc_data[SC_INDEX].rac)
                  ra_changed = TRUE;
              else
                  ra_changed = FALSE;
             }
             else
             {
               ra_changed = FALSE;
             }

            if(la_changed OR ra_changed OR rr_data->gprs_data.ready_state)
#else
            if (!dat_plmn_equal_req (rr_data->nc_data[i].lai.mcc,
                                     rr_data->nc_data[i].lai.mnc,
                                     rr_data->nc_data[SC_INDEX].lai.mcc,
                                     rr_data->nc_data[SC_INDEX].lai.mnc)
                 OR
                 rr_data->nc_data[i].lai.lac NEQ
                 rr_data->nc_data[SC_INDEX].lai.lac)
#endif
            {
              if ((rr_data->ms_data.imsi_available) AND
                 (rr_data->ms_data.rr_service NEQ LIMITED_SERVICE) AND 
                 (rr_data->sc_data.selection_type EQ CELL_RESELECTION_NC))
              {
                  rr_data->nc_data[i].c2_corr -=
                    2*rr_data->nc_data[SC_INDEX].select_para.cell_resel_hyst;
              }
            }
            else
            {
             /*
              * in case of a cell reselection occurring within the previous 15 seconds
              * in which case the C2 value for the new cell shall exceed the C2 value
              * of the serving cell by at least 5 dB for a period of 5 seconds
              */

              if ( IS_TIMER_ACTIVE (T_NO_RESELECT) )
              {
                rr_data->nc_data[i].c2_corr -= 5;
              }
              else
              {
               /* If the C2(NC)=C2(SC), this will get mis-interpreted in the check below
                * c2_corr must be adjusted to make this condition false. The '=' in the '>='
                * condition below only applies when T_NO_RESELECT timer is active and
                * c2_corr has been decremented by 5 (above statement)
                */
                if( rr_data->nc_data[i].c2_corr EQ rr_data->nc_data[max_result].c2_corr)
                {
                  rr_data->nc_data[i].c2_corr -= 1;
                }
              }
            }
#ifdef GPRS
          }
#endif
        }
      }

      /*
       * Now find the the highest one
       */
      for (i = 0; i < 7; i++)
      {
        rr_data->nc_data[i].c2_used = FALSE;
        if (rr_data->nc_data[i].bcch_status EQ DECODED)
        {
          if (rr_data->nc_data[i].c2_corr >=
              rr_data->nc_data[max_result].c2_corr)
          max_result = i;
        }
      }

      result = max_result;

      rr_data->nc_data[result].c2_used = TRUE;
      rr_data->ms_data.c2_tab[result] = MAX_C2;

      /*
       * make the ranking for the other cells.
       */
      while ((next_result = att_get_next_highest_c2_val ())
              NEQ NO_AVAILABLE)
      {
        rr_data->ms_data.c2_tab[next_result] =
          rr_data->ms_data.c2_tab[result] - 1;
        result = next_result;
      }
      break;
  }

  /*
   * return the index of the highest C2.
   */
  return max_result;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_get_next_highest_c2_idx|
+--------------------------------------------------------------------+

  PURPOSE : During the ranking of the cell reselection list this
            function searches for the cell with the next highest c2 index.

*/

static UBYTE att_get_next_highest_c2_idx (UBYTE      old_index)
{
  GET_INSTANCE_DATA;
  UBYTE new_index = NO_AVAILABLE;
  UBYTE i;

  TRACE_FUNCTION ("att_get_next_highest_c2_idx()");

  /*
   * depending on the cell reselection type
   */
  switch (rr_data->sc_data.selection_type)
  {
    case BACK_FROM_DEDICATED:
    case BACK_FROM_DEDICATED_RLF:
      /*
       * check all cells
       */
      for (i = 0; i <= SC_INDEX; i++)
      {
        /*
         * If cell is available and is not selected yet
         */
        if (rr_data->nc_data[i].bcch_status NEQ EMPTY AND
            rr_data->ms_data.c2_tab[i] NEQ MAX_C2)
        {
          /*
           * it is the first candidate, then select it
           */
          if (new_index EQ NO_AVAILABLE)
            new_index = i;
          else
          {
            /*
             * else take the one with the higher C2
             */
            if (rr_data->ms_data.c2_tab[i] >
                rr_data->ms_data.c2_tab[new_index])
              new_index = i;
          }
        }
      }

      /*
       * If a new cell is found, mark this cell as selected
       * for the next round
       */
      if (new_index NEQ NO_AVAILABLE)
        rr_data->ms_data.c2_tab[new_index] = MAX_C2;
      break;

    default:
      /*
       * all other cell reselections
       */
      for (i = 0; i <= SC_INDEX; i++)
      {
        if (rr_data->nc_data[i].bcch_status EQ DECODED)
        {
          if (old_index NEQ i)
          {
            if (new_index EQ NO_AVAILABLE)
            {
              if (rr_data->ms_data.c2_tab[i] <
                  rr_data->ms_data.c2_tab[old_index])
              {
                new_index = i;
              }
            }
            else
            {
              if (rr_data->ms_data.c2_tab[i] >
                  rr_data->ms_data.c2_tab[new_index] AND
                  rr_data->ms_data.c2_tab[i] <
                  rr_data->ms_data.c2_tab[old_index])
              {
                new_index = i;
              }
            }
          }
        }
      }
      break;
  }
  return new_index;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_get_next_highest_c2_val|
+--------------------------------------------------------------------+

  PURPOSE : During the ranking of the cell reselection list this
            function searches for the cell with the next highest c2 value.

*/

static UBYTE att_get_next_highest_c2_val (void)
{
  GET_INSTANCE_DATA;
  UBYTE new_index = NO_AVAILABLE;
  UBYTE i;

  TRACE_FUNCTION ("att_get_next_highest_c2_val()");

  /*
   * look for all cells inclusive the serving cell
   */
  for (i = 0; i < 7; i++)
  {
    /*
     * the cell must have read the system infos to calculate c2.
     */
    if (rr_data->nc_data[i].bcch_status EQ DECODED)
    {
      /*
       * if not ranked yet
       */
      if (rr_data->nc_data[i].c2_used EQ FALSE)
      {
        /*
         * if no cell is selected yet,
         * then select this one.
         */
        if (new_index EQ NO_AVAILABLE)
        {
          new_index = i;
        }
        else
        {
          /*
           * the corrected c2 value must be higher
           * then the selected one.
           */
          if (rr_data->nc_data[i].c2_corr >
              rr_data->nc_data[new_index].c2_corr)
          {
            new_index = i;
          }
        }
      }
    }
  }

  /*
   * a cell has been selected, then mark this cell
   * to be not selected the next time.
   */
  if (new_index NEQ NO_AVAILABLE)
  {
    rr_data->nc_data[new_index].c2_used = TRUE;
  }

  /*
   * return the selected cell index
   */
  return new_index;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_get_next_highest_rx    |
+--------------------------------------------------------------------+

  PURPOSE : During the ranking of the cell reselection list this
            function searches for the cell with the next highest rx value.

*/

static UBYTE att_get_next_highest_rx (void)
{
  GET_INSTANCE_DATA;
  UBYTE new_index = NO_AVAILABLE;
  UBYTE i;

  TRACE_FUNCTION ("att_get_next_highest_rx()");

  /*
   * for all neighbourcells and the serving cell
   */
  for (i = 0; i < 7; i++)
  {
    /*
     * if something is stored for this index
     * that means at least MS is synchronized.
     */
    if (rr_data->nc_data[i].bcch_status NEQ EMPTY)
    {
      /*
       * the cell has not been ranked
       */
      if (rr_data->nc_data[i].c2_used EQ FALSE)
      {
        /*
         * if it is the first cell in this attempt,
         * then select it.
         */
        if (new_index EQ NO_AVAILABLE)
        {
          new_index = i;
        }
        else
        {
          /*
           * to select the cell it must have a higher
           * fieldstrength
           */
          if (rr_data->nc_data[i].rxlev >
              rr_data->nc_data[new_index].rxlev)
          {
            new_index = i;
          }
        }
      }
    }
  }

  /*
   * A cell has been selected, then mark it as
   * ranked to avoid selection in the next attempt.
   */
  if (new_index NEQ NO_AVAILABLE)
  {
    rr_data->nc_data[new_index].c2_used = TRUE;
  }

  /*
   * return the index of the selected cell.
   */
  return new_index;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_handle_rr_act_req      |
+--------------------------------------------------------------------+

  PURPOSE : MM requests with the primitive RR_ACTIVATE_REQ one of the
            following procedures:

            1. Full Service     - search for a specific PLMN
            2. Limited Service  - search for a cell for limited service
            3. PLMN Search      - request the PLMN available list.

*/

GLOBAL void att_handle_rr_act_req (UBYTE              initial_act,
                                   T_RR_ACTIVATE_REQ *rr_activate_req)
{
  GET_INSTANCE_DATA;
  UBYTE count = 0;
  TRACE_FUNCTION ("att_handle_rr_act_req()");

  /*
   * stop time control for an ongoing cell reselection
   */
  TIMERSTOP (T_RESELECT);
  TIMERSTOP (TABORT);

  rr_data->net_lost = FALSE;

  if (initial_act)
  {
    /*
     * read ms data from e2prom
     */
    att_clear_parallel_search();
    rr_csf_ms_cap ();
    rr_csf_check_rfcap (FALSE);
    rr_csf_read_imei (&rr_data->ms_data.imei);
    rr_data->ms_data.rr_service = NO_SERVICE;   /* set initial to no service */
  }

  for(count=0;count<MAX_MCC_SHIELD;count++)
  {
    TRACE_EVENT_P3 ( "Current software shielded MCC are =%d%d%d",
                      rr_data->dyn_config.mcc_shield.mcc[count][0],
                      rr_data->dyn_config.mcc_shield.mcc[count][1],
                      rr_data->dyn_config.mcc_shield.mcc[count][2]);
  }
                       
  /*
   * store EPLMN list, if available, for use in cell selection/reselection.
   */
  att_copy_eplmn_list(&rr_activate_req->eq_plmn_list);

  /*
   * store operation mode data
   */
  rr_data->ms_data.operation_mode =
     (rr_activate_req->op.ts << 7) +
     (rr_activate_req->op.m  << 6) +
     (rr_activate_req->op.sim_ins << 5);

  /*
   * some optimisations must be removed for the FTA campaign.
   */
  if (dat_test_sim_available() EQ TRUE)
    test_house = TRUE;

  att_set_func (rr_activate_req->op.func);
  /*
   * start cell selection procedure
   */
  rr_data->ms_data.req_mm_service = att_get_func ();

  TRACE_EVENT_P6 ("RR_ACTIVATE_REQ: %s func op=%s %scurrent=%s rMM=%s RRs=%s",
    initial_act ? "initial" : "",
    _rr_str_FUNC[rr_activate_req->op.func],
    _rr_str_PARFUNC[rr_data->ms_data.parallel_net_plmn_search_type],
    _rr_str_FUNC[rr_data->ms_data.current_plmn_search_type],
    _rr_str_FUNC[rr_data->ms_data.req_mm_service],
    _rr_str_SERVICE[rr_data->ms_data.rr_service]);

  switch (rr_data->ms_data.req_mm_service)
  {
    case FUNC_LIM_SERV_ST_SRCH:
#ifdef GPRS
      att_set_gprs_indication(rr_activate_req->gprs_indication);
#endif
      rr_data->c1_offset = C1_OFFSET;      

      att_begin_cs(FUNC_LIM_SERV_ST_SRCH);
      break;

    case FUNC_NET_SRCH_BY_MMI:
      /*
       * clear found_channel parameter from previous search
       */
      switch(GET_STATE(STATE_ATT))
      {
        case ATT_IDLE:
        case ATT_CON_EST:
          /* Cell Selection Improvements-LLD section:4.1.3.9 */
          att_start_cell_selection(MM_ORIGINATED, CS_PARALLEL,FULL_SEARCH_MODE);
          break;

        default: 
          att_begin_cs(FUNC_NET_SRCH_BY_MMI);
          break;
      }
      EM_NET_SEARCH_STARTED;
      break;

    case FUNC_PLMN_SRCH:
      /*
       * copy full service parameter from SIM
       */
#ifdef GPRS
      att_set_gprs_indication (rr_activate_req->gprs_indication);
#endif
      if (rr_data->ms_data.rr_service NEQ NO_SERVICE)
        cs_clear_attributes (CHECKED_FLAG, rr_data->nc_data[SC_INDEX].arfcn);

/* Implements Measure#32: Row 52,53 */
      att_print_mcc_mnc (NOT_PRESENT_16BIT, rr_data->ms_data.plmn.mcc,
                                            rr_data->ms_data.plmn.mnc, S2I_STRING("old_req_plmn"));
      att_print_mcc_mnc (NOT_PRESENT_16BIT, rr_activate_req->plmn.mcc,
                                            rr_activate_req->plmn.mnc, S2I_STRING("new_req_plmn"));

      memcpy (&rr_data->ms_data.plmn, &rr_activate_req->plmn, sizeof (T_plmn));
      rr_data->ms_data.cksn = rr_activate_req->cksn;
      memcpy (rr_data->ms_data.new_kc, &rr_activate_req->kcv.kc, KC_STRING_SIZE);
      rr_data->ms_data.access_classes = rr_activate_req->accc;

#if defined(_SIMULATION_)
/* Implements Measure#32: Row 52 */
      att_print_mcc_mnc(rr_data->nc_data[SC_INDEX].arfcn,
        rr_data->ms_data.plmn.mcc, rr_data->ms_data.plmn.mnc, S2I_STRING("req"));
#endif

      /*
       * copy IMSI if available
       */
      if (rr_activate_req->imsi_struct.id_type EQ TYPE_IMSI)
      {
        memset (&rr_data->ms_data.imsi, 0, sizeof (rr_data->ms_data.imsi));
        rr_data->ms_data.imsi_available = TRUE;
        rr_data->ms_data.imsi.ident_type  = TYPE_IMSI;
        rr_data->ms_data.imsi.v_ident_dig = TRUE;
        rr_data->ms_data.imsi.c_ident_dig =
          att_calculate_digits (rr_activate_req->imsi_struct.id);
        rr_data->ms_data.imsi.odd_even    =
          rr_data->ms_data.imsi.c_ident_dig & 1;
        memcpy (rr_data->ms_data.imsi.ident_dig, rr_activate_req->imsi_struct.id, 16);
      }
      else
        rr_data->ms_data.imsi_available    = FALSE;

      /*
       * copy TMSI if available
       */
      if (rr_activate_req->tmsi_struct.id_type EQ TYPE_TMSI)
      {
        rr_data->ms_data.tmsi_available = TRUE;
        rr_data->ms_data.tmsi_binary    = rr_activate_req->tmsi_struct.tmsi_dig;
      }
      else
        rr_data->ms_data.tmsi_available    = FALSE;

      if (dat_test_sim_available())
        rr_data->c1_offset = 0;
      else
        rr_data->c1_offset = C1_OFFSET;
      
      /* CSI-LLD section:4.1.1.11
       * First FUNC_PLMN_SEARCH
       */
      if(rr_data->cs_data.initial_plmn_search EQ INITIAL_PLMN_SEARCH_NOT_ACTIVE)
      {
        rr_data->cs_data.initial_plmn_search = INITIAL_PLMN_SEARCH_ACTIVE;
      } 
 
      cs_set_bcch_info(&rr_activate_req->bcch_info);

      att_begin_cs(FUNC_PLMN_SRCH);
      EM_PLMN_SRCH_STARTED;
      break;

    case FUNC_ST_PWR_SCAN:
      SET_STATE(STATE_ATT, ATT_CS_INIT ); 
      att_start_cell_selection(MM_ORIGINATED, CS_NOT_PARALLEL,FULL_SEARCH_MODE);
      break;
  }

  PFREE (rr_activate_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_begin_cs               |
+--------------------------------------------------------------------+

  PURPOSE : This function initializes a cell selection or starts BCCH
            reading depending on the ATT state on an RR_ACTIVATE_REQ
*/

static void att_begin_cs (UBYTE req_mm_service)
{
  GET_INSTANCE_DATA;
  if(GET_STATE(STATE_ATT) EQ ATT_CS_INIT )
  {
    SET_STATE(STATE_ATT, ATT_CS1 ); /*Boot Time*/

    /* Boot Time Performance Enhancement:
     * Start the TABORT timer on receiving the second activate_req
     */
    tstart_tabort(TABORT_VALUE);

    /* Invalidate whitelist if its PLMN ID does not match 
     *  with the requested PLMN 
     */
    if((rr_data->ms_data.req_mm_service EQ FUNC_PLMN_SRCH)   AND
        (!dat_plmn_equal_req(rr_data->cs_data.white_list.last_sc_lac.mcc,
                      rr_data->cs_data.white_list.last_sc_lac.mnc,
                      rr_data->ms_data.plmn.mcc,
                      rr_data->ms_data.plmn.mnc))) 
    {
      rr_data->cs_data.white_list.region = NOT_PRESENT_8BIT;
    }
    if( srv_check_stored_prim(MPH_POWER_CNF))
    {
      srv_use_stored_prim();
    }
    /*
     * reset the time for net search to finish
     * a PLMN available search during 60 seconds.
     */
    if( req_mm_service EQ FUNC_NET_SRCH_BY_MMI)
      TIMERSTART(T_PLMN_SEARCH, T_PLMN_SEARCH_VALUE);
  }
  else
  {
    /* Cell Selection Improvements-LLD section:4.1.3.9 */
    att_start_cell_selection(MM_ORIGINATED, CS_NOT_PARALLEL,FULL_SEARCH_MODE);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_increase_power_on_time |
+--------------------------------------------------------------------+

  PURPOSE : A requirement in GSM 5.08 indicates that the list of
            forbidden location areas must be cleared all 12 hours.
            Therefore this counters power_on_low and power_on_high
            are used. They are triggered by the periodic measurement
            reports in idle mode.

*/

GLOBAL void att_increase_power_on_time (USHORT inc)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_increase_power_on_time()");

  /*
   * increment power_on_low
   */
  rr_data->power_on_low += inc;
  if (rr_data->power_on_low >= 60)
  {
    /* if it is more then 60 seconds,
     * increment power_on_high = minutes
     */
    rr_data->power_on_low = 0;
    rr_data->power_on_high++;
    if (rr_data->power_on_high >= 720)
    {
      /*
       * after 12 hours = 720 minutes clear all forbidden location area lists
       */
      rr_data->power_on_low  = 0;
      rr_data->power_on_high = 0;
      att_clear_forb_list (FORBIDDEN_LIST_NORMAL);
      att_clear_forb_list (FORBIDDEN_LIST_ROAMING);
    }
  }
}


#if defined (TI_PS_FF_RTD) AND defined (REL99)
LOCAL void att_init_rtd_data(T_rr_enh_para  *p_cur)
{
  UBYTE         j,k;
  TRACE_FUNCTION ("att_init_rtd_data");

for(j = 0;j < MAX_NR_OF_NCELL; j++ )
  {
    p_cur->enh_para.enh_cell_list[j].v_rtd = FALSE;
    for(k = 0;k < MAX_NUM_OF_RTD_VALUES; k++)
      p_cur->enh_para.enh_cell_list[j].rtd[k]= RTD_NOT_AVAILABLE;
    p_cur->enh_para.enh_cell_list[j].c_rtd = 0;
  }/*for*/
return;
}
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_init_cell_selection    |
+--------------------------------------------------------------------+

  PURPOSE : The function initializes some variables needed for control
            of cell selection and cell reselection.

*/

GLOBAL void att_init_cell_selection (UBYTE     selection,
                                     BOOL      initiator)
{
  GET_INSTANCE_DATA;
#if defined (TI_PS_FF_RTD) AND defined (REL99) 
  T_rr_enh_para *p_cur = &rr_data->sc_data.emr_data_current;
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */

  TRACE_FUNCTION ("att_init_cell_selection()");

  TRACE_EVENT_P2 ("init CELL_%sSELECTION %s_ORIGINATED",
    (selection EQ CELL_SELECTION) ? ""   : "RE",
    (initiator EQ MM_ORIGINATED)  ? "MM" : "RR" );

  /*
   * store type of selection and the initiator (RR or MM).
   */
  rr_data->sc_data.selection_type   = selection;
  rr_data->sc_data.mm_started       = initiator;

  /*
   * clear the list of the found PLMNs
   */
  rr_data->sc_data.found_entries     = 0;

  /*
   * clear the bitmap indicating whether all system information
   * messages are read.
   */
  att_init_cr_data();

#if defined (TI_PS_FF_RTD) AND defined (REL99)
  att_init_rtd_data(p_cur);
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */


  /*
   * reset multiband parameter
   */
  rr_data->ncell_mb                 = 0;

  if (GET_STATE (STATE_ATT) EQ ATT_CS2 OR GET_STATE (STATE_ATT) EQ ATT_CS3)
  {
    rr_data->nc_data[SC_INDEX].lai.mcc[0] = NOT_PRESENT_8BIT;
    rr_data->nc_data[SC_INDEX].lai.mnc[0] = NOT_PRESENT_8BIT;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_init_gsm_data          |
+--------------------------------------------------------------------+

  PURPOSE : Intialize all gsm specific variables of RR.

*/

GLOBAL void att_init_gsm_data (void)
{
  GET_INSTANCE_DATA;
  SHORT i;
  UBYTE save;
  UBYTE  gprs_ms, cmsp;
  BOOL   ms_configured, cmsp_configured;

#if defined (TI_PS_FF_RTD) AND defined (REL99)
  T_rr_enh_para *p_cur = &rr_data->sc_data.emr_data_current;
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */


  TRACE_FUNCTION ("att_init_gsm_data()");

  save = rr_data->dyn_config.no_sys_time;

  /* save configured GPRS multislot class */
  ms_configured = rr_data->ms_data.multislot_class_configured;
  gprs_ms = rr_data->ms_data.rf_cap.rf_ms.gprs_ms_class;
  dat_emo_stop ( FALSE );

  /* save configured CMSP bit */
  cmsp_configured = rr_data->ms_data.cmsp_configured;
  cmsp = rr_data->ms_data.rf_cap.cmsp;

  /*
   * clar all RR data with the default value
   */
  memset (rr_data, 0, sizeof (T_RR_DATA));

  /*
   * some parameters have a different default value
   */
  rr_data->lup_rxlev    = 63;
  rr_data->dyn_config.no_sys_time  = save;
  rr_data->ms_data.cksn = CKSN_NOT_PRES;

  /* restore configured GPRS multislot class */
  if (ms_configured)
  {
    rr_data->ms_data.multislot_class_configured = TRUE;
    rr_data->ms_data.rf_cap.rf_ms.gprs_ms_class = gprs_ms;
    TRACE_EVENT_P1 ("'gprs_ms_class' configured to a value of %d", gprs_ms);
  }

  /* restore configured CMSP bit */
  if (cmsp_configured)
  {
    rr_data->ms_data.cmsp_configured = TRUE;
    rr_data->ms_data.rf_cap.cmsp= cmsp;
    TRACE_EVENT_P1 ("'cmsp' configured to a value of %d", cmsp);
  }
#if defined (REL99) AND defined (TI_PS_FF_EMR) AND defined (GPRS)
  rr_data->ms_data.enable_ps_emr = TRUE;
#endif
  for (i = 0; i < 8; i++)
  {
    rr_data->nc_data[i].lai.mcc[0] = NOT_PRESENT_8BIT;
    rr_data->nc_data[i].lai.mnc[0] = NOT_PRESENT_8BIT;
  }
#if defined (REL99) && defined (TI_PS_FF_EMR)
  /*Default initializations for EMR*/
  memset (rr_data->sc_data.rep_count, NOT_PRESENT_8BIT, MAX_NEIGHBOURCELLS);
  for_set_default_emr_data(&rr_data->sc_data.emr_data_current);
  for_set_default_emr_data(&rr_data->sc_data.emr_data_temp);
  rr_data->sc_data.enh_para_status = ENH_PARA_INVALID_STATE;
  rr_data->sc_data.ba_list_ded = rr_data->sc_data.ba_list_idle = FALSE;
  rr_data->sc_data.ba_index = NOT_PRESENT_8BIT; 
#endif
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  rr_data->sc_data.cd.band_indicator = NOT_PRESENT_8BIT;
  rr_data->cr_data.cd.band_indicator = NOT_PRESENT_8BIT;
#endif

#if defined (TI_PS_FF_RTD) AND defined (REL99)
  att_init_rtd_data(p_cur);
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */

  /*
   * clear the forbidden lists
   */
  att_clear_forb_list (FORBIDDEN_LIST_NORMAL);
  att_clear_forb_list (FORBIDDEN_LIST_ROAMING);

  /*
   * indicate all RR timer as not running
   */
  memset (rr_data->t_running, 0, sizeof (rr_data->t_running));
  rr_data->t_expire[T3110]         = tim_t3110;
  rr_data->t_expire[T3122]         = tim_t3122;
  rr_data->t_expire[T3126]         = tim_t3126;
  rr_data->t_expire[TREG]          = tim_treg;
  rr_data->t_expire[T_RESELECT]    = tim_treselect;
  rr_data->t_expire[TIM_EXT_MEAS]  = tim_ext_meas;
  rr_data->t_expire[T_NO_RESELECT] = NULL;
  rr_data->t_expire[TCSVALID]      = NULL;
  rr_data->t_expire[TABORT]        = tim_tabort;
  rr_data->t_expire[T_PLMN_SEARCH] = tim_plmn_search_expiry;
  rr_data->t_expire[T_FAST_CS]     = tim_tfast_cs;
  rr_data->t_expire[T_NORMAL_CS]   = tim_tnormal_cs;

  rr_data->net_lost = FALSE;

  #if defined FF_EOTD
  rr_data->t_expire[TAPDU] = tim_apdu;
  #endif /* FF_EOTD */
  rr_data->t_expire[TNNN] = tim_tnnn;
  rr_data->t_expire[T_DEDICATED_MODE] = NULL;
  rr_data->treg_pending = FALSE;

  rr_data->old_serving_cell           = NOT_PRESENT_8BIT;
  rr_data->ms_data.rr_service         = NO_SERVICE;

  /* tch loop "open" */
  rr_data->tch_loop_subch             = NOT_PRESENT_8BIT;

#if defined FF_EOTD
  rr_data->eotd_req_id                =  NOT_PRESENT_16BIT;
#endif /* FF_EOTD */

#ifdef _SIMULATION_
  {
    int i=0;
    for (;i<NUM_OF_RR_TIMERS;i++)
    {
     #ifdef OPTION_TIMER
         /* vsi_t_config is removed in latest GPF */ 
     #endif
    }
  }
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_init_pl_status         |
+--------------------------------------------------------------------+

  PURPOSE : the function clear the cell channel description list
            of the serving cell.

*/

GLOBAL void att_init_pl_status (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_init_pl_status()");

  rr_data->sc_data.cd.v_cell_chan_desc = NO_CONTENT;
  srv_clear_list (&rr_data->sc_data.cd.cell_chan_desc);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_insert_cell_in_data    |
+--------------------------------------------------------------------+

  PURPOSE : During the analysis of an incoming measurement report a
            new cell has been detected. The data of this cell are
            stored in RR.

*/

static void att_insert_cell_in_data (T_MPH_MEASUREMENT_IND *report,
                                    UBYTE                   index)
{
  GET_INSTANCE_DATA;
  UBYTE       i;
  T_NC_DATA * rrd;

  TRACE_FUNCTION ("att_insert_cell_in_data()");

  /*
   * search for a free entry in the RR storage area.
   */
  for (i = 0; i < 6; i++)
    if (rr_data->nc_data[i].bcch_status EQ EMPTY)
      break;

  if (i < 6)
  {
    /*
     * RR will ever find a free area, because all
     * cells which are not longer in the measurement
     * report, are removed from the RR storage.
     * The if-condition is only for security reasons
     *
     * Store the data from the measurement report and
     * wait for the system information messages.
     */
    rrd              = &rr_data->nc_data[i];
    rrd->bsic        = report->ncells.bsic[index];
    rrd->arfcn       = report->ncells.arfcn[index];
    rrd->rxlev       = report->ncells.rx_lev[index];
    rrd->bcch_status = NON_DECODED;
    rrd->avail_time  = 0;
    #ifdef GPRS
    if( rr_data->gprs_data.use_c31 )
      att_insert_c31_cr_data_in_cell(i);
    #endif
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_max                    |
+--------------------------------------------------------------------+

  PURPOSE : The function calculates the maximum from two values a and b.

*/

GLOBAL SHORT att_max (SHORT a,
                     SHORT b)
{
  TRACE_FUNCTION ("att_max()");

  return ((a > b) ? a : b);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_plmn_in_found_list     |
+--------------------------------------------------------------------+

  PURPOSE : The function checks whether a PLMN identification is already
            stored in the found list or not during cell selection / cell
            reselection.

*/

GLOBAL T_FOUND_ELEMENT *att_plmn_in_found_list (UBYTE *mcc, UBYTE *mnc)
{
  GET_INSTANCE_DATA;
  T_FOUND_ELEMENT *result = NULL;
  int    i;
  T_FOUND_ELEMENT *found;

  TRACE_FUNCTION ("att_plmn_in_found_list()");

  /*
   * look for all entries, which are already stored
   */
  /*att_print_mcc_mnc(0, mcc, mnc, "compare with");*/
  found = &rr_data->sc_data.found[0];
  for (i = 0; i < rr_data->sc_data.found_entries; i++, found++)
  {
    /*
     * compare mobile country code and mobile network code.
     */
    /*att_print_mcc_mnc(found->arfcn, found->plmn.mcc, found->plmn.mnc, "list");*/
    if (dat_plmn_equal_req (mcc, mnc,
                            found->plmn.mcc, found->plmn.mnc))
    {
      result = found;
      /*att_print_mcc_mnc(found->arfcn, mcc, mnc, "found");*/
     break;
    }
  }

  /*
   * return the result of the compare.
   */
  return result;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_save_found_plmn         |
+--------------------------------------------------------------------+

  PURPOSE : The function set all list elements of a found PLMN.
            It marks the element as present (V_PLMN_PRES)

*/

GLOBAL void att_save_found_plmn (T_FOUND_ELEMENT *element, UBYTE mcc[], UBYTE mnc[],
                                 USHORT arfcn, UBYTE rxlev, USHORT lac, UBYTE cell_ok)
{
  GET_INSTANCE_DATA;
  element->plmn.v_plmn = V_PLMN_PRES;
  memcpy (element->plmn.mcc, mcc, SIZE_MCC);
  memcpy (element->plmn.mnc, mnc, SIZE_MNC);
  element->arfcn   = arfcn;
  element->rxlev   = rxlev;
  element->lac     = lac; /* LOL 02.01.2003: added for EONS support */
  element->cell_ok = cell_ok;
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  if (rr_data->cs_data.region EQ BOTH_REGIONS)
  {
    element->region = srv_get_region_from_std_arfcn (std, arfcn);
  }
  else
  {
#endif
  element->region  = rr_data->cs_data.region;
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  }
#endif

  TRACE_EVENT_P9 ("[%u] save new PLMN (%x%x%x %x%x%x) region=%d, entries=%u",
    arfcn, mcc[0], mcc[1], mcc[2], mnc[0], mnc[1], mnc[2],element->region,
    rr_data->sc_data.found_entries+1);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_priority_check         |
+--------------------------------------------------------------------+

  PURPOSE : The function checks the priority of a cell during cell
            selection. It must be differed between first and second
            attempt.

            During the first attempt it will be differed between
            low and high (or normal) priority.

            In the first attempt the cell bar qualifier is used to
            differ between low and high priority. Beside this there
            is an exception defined in GSM 5.08 chapter 9.

            In the second attempt the priority check is ever successful.

*/

static BOOL att_priority_check (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_priority_check()");

  /*
   * During the First Scan second attempt and second scan the priority
   * check is ever successful.
   */
  if((rr_data->cs_data.scan_mode EQ CS_FIRST_SCAN_SECOND_ATTEMPT) OR 
     (rr_data->cs_data.scan_mode EQ CS_SECOND_SCAN))   
  {
    return TRUE;
  }

  /*
   * Exception condition handling according GSM 5.08, chapter 9
   */
  if (dat_hplmn (rr_data->nc_data[CR_INDEX].lai.mcc,
                 rr_data->nc_data[CR_INDEX].lai.mnc) AND
      rr_data->cell_test_operation AND
      rr_data->nc_data[CR_INDEX].c2_par.cbq EQ 0 AND
      rr_data->nc_data[CR_INDEX].rach.cell_bar_access EQ 1 AND
      (rr_data->nc_data[CR_INDEX].rach.ac & 0x8000) NEQ 0)
    return TRUE;

  /*
   * Else the cell bar qualifier indicates the priority
   */
  if (rr_data->nc_data[CR_INDEX].c2_par.cbq)
    return FALSE;
  else
    return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_remove_bad_ncell       |
+--------------------------------------------------------------------+

  PURPOSE : During fieldtest sometimes a neighbourcell in the
            measurement report was equal to a neighbourcell. This
            function deletes neighbourcell data if the channel is
            equal to the serving cell.

*/

GLOBAL void att_remove_bad_ncell (T_MPH_MEASUREMENT_IND *report)
{
  GET_INSTANCE_DATA;
  SHORT       i = 0;
  SHORT       j;
  T_ncells    *pncells = &report->ncells;
  TRACE_FUNCTION ("att_remove_bad_ncell()");

  /*
   * for all neighbourcells in the measurement report.
   */
  while (i < pncells->no_of_ncells)
  {
    /*
     * Do nothing if the channel number is different from
     * the serving cell.
     */
    if (pncells->arfcn[i] NEQ rr_data->nc_data[SC_INDEX].arfcn)
      i++;
    else
    {
      /*
       * else overwrite this entry by moving the rest one index
       */
      for (j = i + 1; j < pncells->no_of_ncells; j++)
      {
        pncells->arfcn[j-1]         = pncells->arfcn[j];
        pncells->rx_lev[j-1]        = pncells->rx_lev[j];
        pncells->bsic[j-1]          = pncells->bsic[j];
        pncells->time_alignmt[j-1]  = pncells->time_alignmt[j];
        pncells->frame_offset[j-1]  = pncells->frame_offset[j];
      }

      /*
       * clean the last entry
       */
      pncells->bsic[j-1]          = 0;
      pncells->arfcn[j-1]         = 0;
      pncells->rx_lev[j-1]        = 0;
      pncells->time_alignmt[j-1]  = 0;
      pncells->frame_offset[j-1]  = 0;
      pncells->no_of_ncells--;
    }
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_allow_lai_in_list      |
+--------------------------------------------------------------------+

  PURPOSE : After a successful location updating request MM informs
            RR about this. If the location area information is stored
            inside of one of the forbidden location area lists this
            information must be updated. In this function, the entry
            is deleted from the given forbidden list.

*/

LOCAL void att_allow_lai_in_list (int         list_type,
                           const T_plmn     * plmn,
                           USHORT             lac)
{
  GET_INSTANCE_DATA;
  unsigned int i;
  T_loc_area_ident *forb_list;

  if (list_type EQ FORBIDDEN_LIST_NORMAL)
    forb_list = &rr_data->ms_data.forb_lac_list[0];
  else
    forb_list = &rr_data->ms_data.roam_forb_lac_list[0];

  TRACE_FUNCTION ("att_allow_lai_in_list()");

  for (i = 0; i < (MAX_LAI-1); i++)
  {
    if ((lac EQ forb_list[i].lac) AND
        dat_plmn_equal_req (plmn->mcc, plmn->mnc,
                            forb_list[i].mcc, forb_list[i].mnc))
    {
      /*
       * if the location area code is stored, overwrite this information
       * and fill the last issue with a default value
       */
/* Implements Measure#32: Row 60 */
      att_print_forb_list ( list_type, plmn, lac, i, TRUE);

      memmove (&forb_list[i], &forb_list[i + 1],
               (MAX_LAI - i - 1) * sizeof (T_loc_area_ident));
      forb_list[MAX_LAI - 1].lac    =  NOT_PRESENT_16BIT;
      memset (forb_list[MAX_LAI - 1].mcc, NOT_PRESENT_8BIT, SIZE_MCC);
      memset (forb_list[MAX_LAI - 1].mnc, NOT_PRESENT_8BIT, SIZE_MNC);
    }
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_rem_lai_from_forb_list |
+--------------------------------------------------------------------+

  PURPOSE : After a successful location updating request MM informs
            RR about this. If the location area information is stored
            inside of one of the forbidden location area lists this
            information must be updated. In this function, the entry
            is deleted from all forbidden lists.

*/

GLOBAL void att_rem_lai_from_forb_list (const T_plmn * plmn,
                                        USHORT         lac)
{
  TRACE_FUNCTION ("att_rem_lai_from_forb_list()");

  att_allow_lai_in_list (FORBIDDEN_LIST_NORMAL, plmn, lac);
  att_allow_lai_in_list (FORBIDDEN_LIST_ROAMING, plmn, lac);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_search_cell            |
+--------------------------------------------------------------------+

  PURPOSE : This function searches for a cell for cell reselection.

*/

static void att_search_cell (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_search_cell()");
#if defined(_SIMULATION_)
  TRACE_SELECTION_TYPE (rr_data->sc_data.selection_type);
#endif /* _SIMULATION_ */

  /*
   * depending on the cell reselection mode.
   */
  switch (rr_data->sc_data.selection_type)
  {
    case BACK_FROM_DEDICATED:
    case BACK_FROM_DEDICATED_RLF:
      /*
       * back from dedicated use the fieldstrength of the
       * cells as the criterion
       */
      rr_data->reselect_index = att_get_highest_c2_index ();
      att_select_cell_dedicated ();
      break;

   case CELL_RESELECTION:
   case CELL_RESELECTION_NC:
      #ifdef GPRS
      if( rr_data->gprs_data.use_c31 )
      {
        SHORT i;
        TRACE_EVENT ("c32 used in att_search_cell");
        for( i = 0; i < 6; i++ )
          rr_data->nc_data[i].c32_used = FALSE;

        rr_data->ms_data.old_cell_tried = FALSE;

        rr_data->reselect_index = att_get_next_best_c32_index (FALSE);

        /*
          * if no suitable cell is available
          * try the old cell again.
          */
        if (! att_check_cell_c31 ())
          att_try_old_cell ();

        break;
      }
      #endif
      /*lint -fallthrough*/
    default:
      /*
       * cell reselection in idle mode, use the cell
       * reselection criterion C2.
       */
      rr_data->ms_data.old_cell_tried = FALSE;
      rr_data->reselect_index = att_get_highest_c2_index ();

      /*
       * if no suitable cell is available
       * try the old cell again.
       */
      if (! att_check_cell ())
        att_try_old_cell ();
      break;
  }
  EM_FMM_RESEL_START_IND;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_set_pl_in_idle_mode    |
+--------------------------------------------------------------------+

  PURPOSE : The function configures the idle mode at the end of
            cell selection and cell reselection.

*/

GLOBAL void att_set_pl_in_idle_mode (void)
{
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  GET_INSTANCE_DATA;
#endif
  TRACE_FUNCTION ("att_set_pl_in_idle_mode()");
  /* Update std value depending upon the band indicator value */
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  att_update_std_band_indicator (rr_data->sc_data.cd.band_indicator);
#endif
  /*
   * forward classmark information to layer 1 for random access procedure
   */
  att_build_classmark_req ();

  /*
   * configure the idle mode (paging group etc.)
   */
  att_build_idle_req (SC_INDEX, MODE_CELL_SELECTION);

  /*
   * configure the CBCH channel
   */
#ifdef REL99
  att_config_cbch ();
#else
  att_build_cbch();
#endif

  /*
   * configure the neighbourcell list
   */
  att_code_mph_ncell_req (SC_INDEX);

  /*
   * store the neighbourcell information in PCM and the SIM card.
   */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_set_sys_info_read      |
+--------------------------------------------------------------------+

  PURPOSE : The decision whether all system information messages are
            read during cell selection / cell reselection is done with
            this function. The variable sys_info_read is organized as
            a bitmap. Each system information message belongs to one
            bit of the bitmap.

*/

GLOBAL void att_set_sys_info_read (USHORT mess_bit, UBYTE index)
{
  GET_INSTANCE_DATA;
  T_CELL_DATA * cd;
  USHORT si;

  TRACE_FUNCTION ("att_set_sys_info_read()");

  /*
   * set pointer to data
   */
  if (index EQ SC_INDEX)
    cd = &rr_data->sc_data.cd;
  else if (index EQ CR_INDEX)
    cd = &rr_data->cr_data.cd;
  else
  {
    TRACE_EVENT ("unexpected index in att_set_sys_info_read()");
    return;
  }

  /*
   * set the bit of the bitmap according to the message type.
   */
  cd->sys_info_read |= mess_bit;

#if !defined(NTRACE)
  si = cd->sys_info_read;
/*  if (GET_STATE (STATE_ATT) EQ ATT_CS2) comment out just for debugging */
  {
    if (si EQ ALL_SYS_INFO_READ)
    {
      TRACE_EVENT ("SI complete");
    }
    else
    {
#ifdef GPRS
      TRACE_EVENT_P9 ( "await SI%s%s%s%s%s%s%s (%x) for %s",
                       (si & SYS_INFO_1_READ   ) ? "" : " 1"   ,
                       (si & SYS_INFO_2_READ   ) ? "" : " 2"   ,
                       (si & SYS_INFO_2BIS_READ) ? "" : " 2bis",
                       (si & SYS_INFO_3_READ   ) ? "" : " 3"   ,
                       (si & SYS_INFO_4_READ   ) ? "" : " 4"   ,
                       (si & SYS_INFO_2TER_READ) ? "" : " 2ter",
                       (si & SYS_INFO_13_READ  ) ? "" : " 13",
                       si,
                       (index EQ SC_INDEX      ) ? "SC_INDEX" : "CR_INDEX");
#else
      TRACE_EVENT_P8 ( "await SI%s%s%s%s%s%s (%x) for %s",
                       (si & SYS_INFO_1_READ   ) ? "" : " 1"   ,
                       (si & SYS_INFO_2_READ   ) ? "" : " 2"   ,
                       (si & SYS_INFO_2BIS_READ) ? "" : " 2bis",
                       (si & SYS_INFO_3_READ   ) ? "" : " 3"   ,
                       (si & SYS_INFO_4_READ   ) ? "" : " 4"   ,
                       (si & SYS_INFO_2TER_READ) ? "" : " 2ter",
                       si,
                       (index EQ SC_INDEX      ) ? "SC_INDEX" : "CR_INDEX");
#endif
    }
  }
#endif  /* !NTRACE */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_start_cell_reselection |
+--------------------------------------------------------------------+

  PURPOSE : The function is called at the beginning of a cell
            reselection.

*/

GLOBAL void att_start_cell_reselection (UBYTE     mode)
{
  GET_INSTANCE_DATA;
#ifdef GPRS
  T_GPRS_DATA  *gprs_data = &rr_data->gprs_data;
#endif

  TRACE_FUNCTION ("att_start_cell_reselection()");

  if( mode EQ BACK_FROM_DEDICATED_RLF)
  {
    /*
     * Allow 20 seconds for call re-establishment
     * In case of data link failure
     */
    tstart_tabort(2 * TABORT_VALUE);
  }
  else
  {
    tstart_tabort(TABORT_VALUE);
  }

#ifdef GPRS
  if ( gprs_data->rrgrr_ext_meas_req NEQ NULL )
  {
    /*
     * Save the request to perform Cell Reselection.
     * Then stop the Ext Meas procedure.
     * At the very end of the Ext Meas continue with CR.
     */
    gprs_data->ext_meas_ctrl |= EXT_MEAS_START_CR;
    rr_data->sc_data.selection_type = mode;
    att_rrgrr_ext_meas_stop_req(NULL);
    return;
  }
#endif

  /*
   * intialise some variables
   */
  rr_data->dyn_config.fcr        = 0;
  rr_data->dyn_config.scr        = 0;
  rr_data->bcch_error = 0;
  rr_data->pag_rec    = FALSE;
  srv_clear_stored_prim (MPH_PAGING_IND);

   /*
   * MM has initiated GPRS Activation. While waiting for SI13 a local reselection
   * is triggered in RR because of a better neighbour cell. Since MM initiated GPRS
   * is pending; RR should respond by RR_ACTIVATE_CNF and not by RR_ACTIVATE_IND.
   */
#ifdef GPRS
  if ( GET_STATE(STATE_GPRS) EQ GPRS_ACTIVATED AND 
       rr_data->sc_data.mm_started EQ MM_ORIGINATED AND
       rr_data->sc_data.selection_type EQ CELL_RESELECTION_ON_GPRS_ACT)
  {
    att_init_cell_selection (mode, MM_ORIGINATED);
  }
  else
#endif
  {
    att_init_cell_selection (mode, RR_ORIGINATED);
  }
  dat_att_null();
#ifdef GPRS
  gprs_init_data_cr();
#endif
  /*
   * stop any reselection related timer.
   */
  if (rr_data->sc_data.selection_type EQ BACK_FROM_DEDICATED_RLF)
  {
    /*
     * according GSM 5.08, chapter 6.7.2 call re-establishment
     * shall be finished latest during 20 seconds (=2*normal
     * cell reselection time).
     */
    TIMERSTART (T_RESELECT, 2*TRESELECT_VALUE);
  }
  else
  {
    TIMERSTART (T_RESELECT, TRESELECT_VALUE);
  }

  /*
   * depending on the cell reselection type
   */
  switch (rr_data->sc_data.selection_type)
  {
    case CELL_RESELECTION:
    case CELL_RESELECTION_RACH:
      /*
       * exclude serving cell for five seconds
       */
      set_tnnn (SC_INDEX, 5*ONE_SEC);
      break;
    case BACK_FROM_DEDICATED:
    case BACK_FROM_DEDICATED_RLF:
      /*
       * remove serving cell from neighbourcell results
       */
      att_remove_bad_ncell (&rr_data->ms_data.measurement_report);
      att_build_back_from_dedicated ();
      break;
    case CELL_RESELECTION_CR:
      /*
       * check cell to get back in full service
       */
      break;
  }

  EM_CELL_RESEL_STARTED;
  /*
   * find a suitable cell for cell reselection
   */
  att_search_cell ();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_start_cell_selection   |
+--------------------------------------------------------------------+

  PURPOSE : The function is called at the beginning of a cell selection.
*/

GLOBAL void att_start_cell_selection (BOOL originator, BOOL parallel,
                                      U8 search_mode)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_start_cell_selection()");


  TRACE_EVENT_P2 ("START %sparallel CELL SELECTION %s_ORIGINATED",
                  parallel?"":"not ", 
                  originator?"MM":"RR");

  tstart_tabort(TABORT_VALUE);

  rr_data->net_lost = FALSE;
  rr_data->cs_data.black_list_search_pending = FALSE;

  EM_CELL_SELECTION;

  /* Set the current search mode */
  CS_SET_CURRENT_SEARCH_MODE(search_mode);

  /*
   * initialize some variables
   */
  /* found channel handling */
  if(originator EQ MM_ORIGINATED)
  {
    if(rr_data->ms_data.req_mm_service EQ FUNC_PLMN_SRCH)
      cs_get_channel_from_found_list ();
    else
      cs_clear_channel_from_found_list();
  }

  att_init_cell_selection (CELL_SELECTION, originator);

  /* Reset scan mode */
  rr_data->cs_data.scan_mode = CS_FIRST_SCAN_FIRST_ATTEMPT;

  /*
   * init process and start power measurements
   */
  #ifdef GPRS
  rr_data->gprs_data.use_c31 = FALSE;
  #endif
  if(!parallel)
  {
    memset (rr_data->nc_data, 0, 6*sizeof (T_NC_DATA));
    cs_set_null ();
  }
  cs_set_all ();

    /*
     * Why should a RR caused cell selection forego the helpfully information
     * of the neighbor cell description of the last serving cell to speed up
     * the selection of the new cell?
     * But it is appropriate only for non PARALLEL search!
     * Therefore we collect first the (at cs_data) stored cell discription
     * and merge it into a channel list (cs_data.bcch_info). As second step
     * we use this list to level up the priority of the matching cells after
     * the scan execution (cs_mph_power_cnf).
     */

#ifdef GPRS
  if(att_gprs_is_avail() AND !parallel)
    gprs_init_data_cr();
#endif

  /* Cell Selection Improvements-LLD section:4.1.3.4.1.3
   * This function updates the new search mode based on the current 
   * dynamic search mode configuration
   */
  att_check_dynamic_search_mode_config();

  cs_start_scan ();

  if(!parallel)
  {
    if( GET_STATE(STATE_ATT) EQ ATT_CS_INIT )
    {
      att_code_rr_act_cnf();
    }
    else
    {
      SET_STATE (STATE_ATT, ATT_CS1);
    }
    dat_att_null();
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_try_old_cell           |
+--------------------------------------------------------------------+

  PURPOSE : if no suitable cell has been found during cell reselection
            the old serving cell is tried.

*/

static void att_try_old_cell (void)
{
  GET_INSTANCE_DATA;
  UBYTE search_mode =0;
  TRACE_FUNCTION ("att_try_old_cell()");

  /*
   * check whether the old serving cell is possible
   */

  if (rr_data->nc_data[SC_INDEX].arfcn EQ rr_data->nc_data[CR_INDEX].arfcn)
    rr_data->ms_data.old_cell_tried = TRUE;

/* Implements RR Clone findings #8 */
  if (! att_cell_barred_status_cr_no_cr (SC_INDEX) AND
      ! is_tnnn (SC_INDEX) AND
      rr_data->nc_data[SC_INDEX].c1 > 0 AND
      rr_data->ms_data.old_cell_tried EQ FALSE)
      
  {
    if (rr_data->sc_data.selection_type EQ CELL_RESELECTION_RACH)
    {
      /*
       * go direct back to the old cell after random access failure
       */
      att_return_to_idle();
    }
    else
    {
      /*
       * serving cell is possible. Configure layer 1
       * to start cell reselection
       */
      rr_data->ms_data.old_cell_tried = TRUE;
      att_start_cr_in_pl(SC_INDEX);
    }
  }
  else
  {
    /*
     * old serving cell is not possible. Then start
     * cell selection for recovering.
     */
    /* Refer Cell Selection Improvements-LLD section:4.1.3.9  */
    if(rr_data->ms_data.rr_service EQ FULL_SERVICE)
    {
      /* RR is in full service and in ATT_IDLE state. Cell reselection started
       * and failed. Start Fast search 
       */
      search_mode = FAST_SEARCH_MODE;
    }
    else
    { 
      if((rr_data->ms_data.rr_service EQ LIMITED_SERVICE) AND 
         (rr_data->ms_data.req_mm_service EQ FUNC_PLMN_SRCH))
      {
        /* RR is in Limited service and in ATT_IDLE state. The requested service 
         * by MM is Full service. Cell reselection started and failed.
         * Obtain new search mode based on the current search mode. 
         */
        search_mode = cs_get_new_search_mode();
      }
      else
      {
        /* RR is in Limited service and in ATT_IDLE state. The requested service 
         * by MM is Limited service. Cell reselection started and failed.
         * Start Normal search 
         */
        search_mode = NORMAL_SEARCH_MODE;
      }
    }
 
    /* XY:n don't inform GRR , and don't call start_cs_gprs  !!!
     * we can go here after sending a CR_IND but not being in CS3 
     */
    att_start_cell_selection (RR_ORIGINATED, CS_NOT_PARALLEL,search_mode);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_mph_identity_req       |
+--------------------------------------------------------------------+

  PURPOSE : Forwards the mobile identities to Layer 1.

*/

GLOBAL void att_mph_identity_req (void)
{
  GET_INSTANCE_DATA;
  PALLOC (mph_identity_req, MPH_IDENTITY_REQ);

  TRACE_FUNCTION ("att_mph_identity_req()");

  memset (mph_identity_req, 0, sizeof (T_MPH_IDENTITY_REQ));

  switch (rr_data->ms_data.rr_service)
  {
    case FULL_SERVICE:
      /*
       * identities are only forwarded in full service
       */
      if (rr_data->ms_data.imsi_available)
      {
        /*
         * fill in the international mobile identity, if available
         */
        mph_identity_req->mid.len_imsi = rr_data->ms_data.imsi.c_ident_dig;
        memcpy (mph_identity_req->mid.imsi, rr_data->ms_data.imsi.ident_dig,
                mph_identity_req->mid.len_imsi);
      }

      if (rr_data->ms_data.tmsi_available)
      {
        /*
         * fill in the temporary mobile identity, if available
         */
        mph_identity_req->mid.v_tmsi = TRUE;
        mph_identity_req->mid.tmsi   = rr_data->ms_data.tmsi_binary;
      }
      break;
  }
#ifdef GPRS
  if (rr_data->gprs_data.gprs_indic)
  {
    att_add_ptmsi (mph_identity_req);
    {
      PALLOC(ms_id, RRGRR_MS_ID_IND);
      ms_id->tmsi = rr_data->ms_data.tmsi_binary;
      PSENDX(GRR, ms_id);
    }
  }
#endif

  /*
   * send the valid identities to layer 1.
   */
  PSENDX (PL, mph_identity_req);

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)     MODULE  : RR_ATT                       |
| STATE   : code              ROUTINE : att_start_registration_timer |
+--------------------------------------------------------------------+

  PURPOSE : Checks whether to start, to stop or to let run the
            registration timer depending on the requested service by MM
            and the actual RR service.

*/

/*
 * Time tables with increasing periods depending on the registration attempt
 */
#ifdef TI_PS_FF_AT_P_CMD_CTREG
/* Initialize the table with default values
 * All the values are in secs / 10 format
 * Ex: T_TEN_SEC = 10 Sec / 10
 */
const UBYTE no_service_mode_time [25] =
{      T_TEN_SEC, T_TEN_SEC, T_TEN_SEC, T_TEN_SEC,
       T_TWENTY_SEC, T_TWENTY_SEC, T_TWENTY_SEC, T_TWENTY_SEC, T_TWENTY_SEC,
       T_THIRTY_SEC, T_THIRTY_SEC, T_THIRTY_SEC, T_THIRTY_SEC, T_THIRTY_SEC,
       T_SIXTY_SEC, T_SIXTY_SEC, T_SIXTY_SEC, T_SIXTY_SEC, T_SIXTY_SEC,
       T_TWO_MIN, T_TWO_MIN, T_TWO_MIN, T_TWO_MIN, T_TWO_MIN,
       T_SIX_MIN
};
#else
const T_TIME no_service_mode_time [25] =
{      TEN_SEC,    TEN_SEC,    TEN_SEC,    TEN_SEC,
       TWENTY_SEC, TWENTY_SEC, TWENTY_SEC, TWENTY_SEC, TWENTY_SEC,
       THIRTY_SEC, THIRTY_SEC, THIRTY_SEC, THIRTY_SEC, THIRTY_SEC,
       SIXTY_SEC,  SIXTY_SEC,  SIXTY_SEC,  SIXTY_SEC,  SIXTY_SEC,
       TWO_MIN,    TWO_MIN,    TWO_MIN,    TWO_MIN,    TWO_MIN,
       SIX_MIN
};
#endif /* TI_PS_FF_AT_P_CMD_CTREG */

#ifdef TI_PS_FF_AT_P_CMD_CTREG
const UBYTE lim_service_mode_time [25] =
{      T_TEN_SEC, T_TEN_SEC, T_TEN_SEC, T_TEN_SEC,
       T_TWENTY_SEC, T_TWENTY_SEC, T_TWENTY_SEC, T_TWENTY_SEC, T_TWENTY_SEC,
       T_THIRTY_SEC, T_THIRTY_SEC, T_THIRTY_SEC, T_THIRTY_SEC, T_THIRTY_SEC,
       T_SIXTY_SEC, T_SIXTY_SEC, T_SIXTY_SEC, T_SIXTY_SEC, T_SIXTY_SEC,
       T_TWO_MIN, T_TWO_MIN, T_TWO_MIN, T_TWO_MIN, T_TWO_MIN,
       T_SIX_MIN
};
#else
const T_TIME lim_service_mode_time [25] =
{      TEN_SEC,    TEN_SEC,    TEN_SEC,    TEN_SEC,
       TWENTY_SEC, TWENTY_SEC, TWENTY_SEC, TWENTY_SEC, TWENTY_SEC,
       THIRTY_SEC, THIRTY_SEC, THIRTY_SEC, THIRTY_SEC, THIRTY_SEC,
       SIXTY_SEC,  SIXTY_SEC,  SIXTY_SEC,  SIXTY_SEC,  SIXTY_SEC,
       TWO_MIN,    TWO_MIN,    TWO_MIN,    TWO_MIN,    TWO_MIN,
       SIX_MIN
};
#endif /* TI_PS_FF_AT_P_CMD_CTREG */

#ifdef TI_PS_FF_AT_P_CMD_CTREG
/*
 * Function used to convert the user given TREG timer values in secs to ms
 * Ex: User value   TREG value
 *         0           1  Sec
 *         1           10 Sec
 *         2           20 Sec etc
 */
T_TIME cast2T_Time(UBYTE tab_val)
{
  return (tab_val)? tab_val*10000: 1000;
}
#endif /* TI_PS_FF_AT_P_CMD_CTREG */

GLOBAL void att_start_registration_timer (void)
{
  GET_INSTANCE_DATA;
  T_TIME status = 0L;

#ifdef TI_PS_FF_AT_P_CMD_CTREG
  BOOL ret;
  UBYTE tab_val;
#endif /* TI_PS_FF_AT_P_CMD_CTREG */

  TRACE_FUNCTION ("att_start_registration_timer()");

  /*
   * is there a TREG expiry which couldn't be handled yet
   * so restart TREG with a short time
   */
  if (rr_data->treg_pending)
  {
    TIMERSTART (TREG, ONE_SEC);
    rr_data->treg_pending = FALSE;
    return;
  }
  /*
   * preparation to avoid the change of a running TREG
   */
  TIMER_STATUS (rr_handle, TREG, &status);
  /*
   * depending on the current service of RR
   */
  switch (rr_data->ms_data.rr_service)
  {
    case NO_SERVICE:
      /*
       * RR has no service, so it will try to come back
       * to limited or full service after timeout
       * set fieldstrength to 0.
       */
#ifdef FF_PS_RSSI
      RX_SetValue ( 0, RX_QUAL_UNAVAILABLE, RX_ACCE_UNAVAILABLE);
#else
      RX_SetValue (0);
#endif
      cs_set_all ();
      if (status)
        return;

      if (!IS_TIMER_ACTIVE(TREG))
      {
#ifdef TI_PS_HCOMM_CHANGE
        PSIGNAL (_hCommDL, DL_TRACE_REQ, NULL);
#else
        PSIGNAL (hCommDL, DL_TRACE_REQ, NULL);
#endif /* TI_PS_HCOMM_CHANGE */
      }
#ifdef TI_PS_FF_AT_P_CMD_CTREG
      ret = cl_shrd_get_treg(RR_MOD_NOSERVICE_TIME,
                                rr_data->ms_data.reg_counter,
                                &tab_val);
      TRACE_EVENT_P1("Reading of the TREG value is %s",ret? "SUCCESS":"FAILURE");
      if (!ret)
      {
        /* Use default on failure */
        tab_val = no_service_mode_time [rr_data->ms_data.reg_counter];
      }
      if (rr_data->ms_data.reg_counter < 24)
      {
        rr_data->ms_data.reg_counter++;
      }
      TIMERSTART (TREG, cast2T_Time(tab_val));
#else
      if (rr_data->ms_data.reg_counter < 24)
      {
        TIMERSTART (TREG, no_service_mode_time [rr_data->ms_data.reg_counter++]);
      }
      else
      {
        TIMERSTART (TREG, no_service_mode_time [24]);
      }
#endif /* TI_PS_FF_AT_P_CMD_CTREG */
      break;
    case LIMITED_SERVICE:
      /*
       * RR has limited service
       * The timer is only started if MM has requested Full Service.
       */
      if (rr_data->ms_data.req_mm_service EQ FUNC_PLMN_SRCH)
      {
        cs_set_all ();
        if (status)
          return;
#ifdef TI_PS_FF_AT_P_CMD_CTREG
      ret = cl_shrd_get_treg(RR_MOD_LIMSERVICE_TIME,
                                rr_data->ms_data.reg_counter,
                                &tab_val);
      TRACE_EVENT_P1("Reading of the TREG value is %s",ret? "SUCCESS":"FAILURE");
      if (!ret)
      {
        /* Use default on failure */
        tab_val = lim_service_mode_time [rr_data->ms_data.reg_counter];
      }
      if (rr_data->ms_data.reg_counter < 24)
      {
        rr_data->ms_data.reg_counter++;
      }
      TIMERSTART (TREG, cast2T_Time(tab_val));
#else
        if (rr_data->ms_data.reg_counter < 24)
        {
          TIMERSTART (TREG, lim_service_mode_time [rr_data->ms_data.reg_counter++]);
        }
        else
        {
          TIMERSTART (TREG, lim_service_mode_time [24]);
        }
#endif /* TI_PS_FF_AT_P_CMD_CTREG */
      }
      break;
    case FULL_SERVICE:
      /*
       * In full service mode, the timer is not started.
       */
      TIMERSTOP (TREG);
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_set_func               |
+--------------------------------------------------------------------+

  PURPOSE : Sets the requested MM functionality. If MM requests a
            PLMN available search (FUNC_NET_SRCH_BY_MMI) the current
            search request must be stored until the PLMN available
            search has been finished.

*/

GLOBAL void att_set_func (UBYTE func)
{
  GET_INSTANCE_DATA;

#if !defined(NTRACE)
  TRACE_EVENT_P5 ("att_set_func: func op=%s %scurrent=%s, service:rMM=%s RRs=%s",
    _rr_str_FUNC[func],
    _rr_str_PARFUNC[rr_data->ms_data.parallel_net_plmn_search_type],
    _rr_str_FUNC[rr_data->ms_data.current_plmn_search_type],
    _rr_str_FUNC[rr_data->ms_data.req_mm_service],
    _rr_str_SERVICE[rr_data->ms_data.rr_service]);

  if ((func EQ FUNC_NET_SRCH_BY_MMI) AND
      (func NEQ rr_data->ms_data.parallel_net_plmn_search_type))
  {
    TRACE_EVENT_P2 ("att_set_func: NEW %scurrent=%s",
      _rr_str_PARFUNC[func],
      _rr_str_FUNC[rr_data->ms_data.current_plmn_search_type]);
  }
  else if ((func NEQ rr_data->ms_data.current_plmn_search_type) OR
           (rr_data->ms_data.parallel_net_plmn_search_type NEQ 0))
  {
    TRACE_EVENT_P1 ("att_set_func: NEW current=%s", _rr_str_FUNC[func]);
  }
#endif  /* !NTRACE */

  switch (func)
  {
    case FUNC_LIM_SERV_ST_SRCH:
    case FUNC_PLMN_SRCH:
    case FUNC_ST_PWR_SCAN:
      rr_data->ms_data.parallel_net_plmn_search_type = 0;
      rr_data->ms_data.current_plmn_search_type  = func;
      break;

    case FUNC_NET_SRCH_BY_MMI:
      rr_data->ms_data.parallel_net_plmn_search_type = func;
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_get_func               |
+--------------------------------------------------------------------+

  PURPOSE : Requests the MM functionality. If a temporary PLMN available
            search request is active (temp_func = FUNC_NET_SRCH_BY_MMI)
            this function is returned, else the last one which was
            requested by MMI.

*/

static UBYTE att_get_func (void)
{
  GET_INSTANCE_DATA;
  switch (rr_data->ms_data.parallel_net_plmn_search_type)
  {
    case FUNC_NET_SRCH_BY_MMI:
      return (rr_data->ms_data.parallel_net_plmn_search_type);

    default:
      return (rr_data->ms_data.current_plmn_search_type);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_clear_parallel_search  |
+--------------------------------------------------------------------+

  PURPOSE : Resets the temporary requested MM functionality.

*/

static void att_clear_parallel_search (void)
{
  GET_INSTANCE_DATA;
  rr_data->ms_data.parallel_net_plmn_search_type = 0;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_init_cr_data           |
+--------------------------------------------------------------------+

  PURPOSE : The CR index contains the whole data of the new cell.
            It is cleared prior to cell reselection
*/

GLOBAL void att_init_cr_data (void)
{
  GET_INSTANCE_DATA;
  /*
   * clear sys info bitmap
   */
  rr_data->cr_data.cd.sys_info_read    = NO_SYS_INFO_READ;

  /*
  * reset BA list from SI 2, 2Bis and 2Ter
  */
  memset(&rr_data->cr_data.cr_white_list, 0 , sizeof(T_CR_WHITE_LIST));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_copy_cr_data           |
+--------------------------------------------------------------------+

  PURPOSE : The CR index contains the whole data of the new cell.
            It shall replace the content of the old serving cell.
            The content of the old serving cell is copied to the
            column of the new cell (in the neighbour cell area). The
            status is set to DECODED.

*/

GLOBAL void att_copy_cr_data (void)
{
  GET_INSTANCE_DATA;
  UBYTE i;

  /*
   * look for all neighbourcells.
   */
  for (i=0;i<6;i++)
    if (rr_data->nc_data[CR_INDEX].arfcn EQ rr_data->nc_data[i].arfcn)
    {
      /*
       * channel found, then update neighbourcell
       */
      memcpy (&rr_data->nc_data[i], &rr_data->nc_data[SC_INDEX],
              sizeof (T_NC_DATA));
      rr_data->nc_data[i].bcch_status = DECODED;
      rr_data->old_serving_cell       = i;
      break;
    }

  /*
   * update SC_INDEX column.
   */
  memcpy (&rr_data->nc_data[SC_INDEX], &rr_data->nc_data[CR_INDEX],
          sizeof (T_NC_DATA));
  /*
   * implementation problem: must be set for RR originated search
   * e.g. limited to full service
   */
  rr_data->nc_data[SC_INDEX].bcch_status = DECODED;

  /*
   * fit classmarks depend on the values of 'std' and the new serving cell
   */
  rr_csf_fit_capability ();
#if defined (REL99) && defined (TI_PS_FF_EMR)
  /*When there's no SI-2quater in the reselected cell and in the current
    cell the reporting is of enhanced type, we can no longer use these
    enhanced parameters. */
  /*Reset EMR data and indicate the same to GRR and ALR*/
  if ( rr_data->sc_data.emr_data_current.is_data_valid EQ TRUE )
  {
    memset (rr_data->sc_data.rep_count, NOT_PRESENT_8BIT, MAX_NEIGHBOURCELLS);
    for_set_default_emr_data(&rr_data->sc_data.emr_data_current);
    for_send_enh_para(&rr_data->sc_data.emr_data_current);
  }
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_copy_eplmn_list        |
+--------------------------------------------------------------------+

  PURPOSE : This function copies the EPLMN list to rr_data global area

*/

GLOBAL void att_copy_eplmn_list (T_eq_plmn_list *eq_plmn_list)
{
  GET_INSTANCE_DATA;
  UBYTE i;

  /* List only copied if valid and PLMNs greater than 1 (HPLMN included)
     else EPLMN list is marked as invalid */
  if(eq_plmn_list->v_eq_plmn)
  {
    for(i=0;i<RR_EPLMNLIST_SIZE;i++)
    {
      T_plmn local_copy;
      rr_unpack_plmn(&local_copy, eq_plmn_list->eq_plmn, i);
      rr_data->ms_data.eq_plmn_list[i] = local_copy;  /*Struct copy*/
    }

    rr_data->ms_data.v_eq_plmn = TRUE;
  }
  else
  {
    rr_data->ms_data.v_eq_plmn = FALSE;
    memset(&rr_data->ms_data.eq_plmn_list, 0xFF, SIZE_EPLMN);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : rr_unpack_plmn             |
+--------------------------------------------------------------------+

  PURPOSE : Unpacks a PLMN from compressed form to uncompressed form.

*/

LOCAL void rr_unpack_plmn (T_plmn *plmn, const UBYTE *packed, USHORT index)
{
  index *= UBYTES_PER_PLMN;
  plmn->mcc[0] = packed[index] & 0x0f;
  plmn->mcc[1] = packed[index] >> 4;
  index++;
  plmn->mcc[2] = packed[index] & 0x0f;
  plmn->mnc[2] = packed[index] >> 4;
  index++;
  plmn->mnc[0] = packed[index] & 0x0f;
  plmn->mnc[1] = packed[index] >> 4;
  index++;
  if ((plmn->mcc[0] & 0x0F) EQ 0x0F)
    plmn->v_plmn = V_PLMN_NOT_PRES;
  else
    plmn->v_plmn = V_PLMN_PRES;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : RR_ATT                        |
| STATE   : code             ROUTINE : att_build_back_from_dedicated |
+--------------------------------------------------------------------+

  PURPOSE : A list is build containing the serving cell followed by
            the synchronized neighbourcells in decreasing fieldstrenght.
            In case of call re-establishment the serving cell is not used.
            The list controls the cell reselection after dedicated mode.

*/

static void att_build_back_from_dedicated (void)
{
  GET_INSTANCE_DATA;
  UBYTE       i;
  T_NC_DATA * rrd;

  /*
   * update the neighbourcell entries
   */
  for (i=0;i<6;i++)
  {
    /*
     * clear the data for this neighbourcell
     */
    memset (&rr_data->nc_data[i], 0, sizeof (T_NC_DATA));
    rrd = &rr_data->nc_data[i];
    if (rr_data->ms_data.measurement_report.ncells.no_of_ncells > i)
    {
      /*
       * if neighbourcell data is available in the measurement report
       * copy the data.
       */
      rrd->bsic        = rr_data->ms_data.measurement_report.ncells.bsic[i];
      rrd->arfcn       = rr_data->ms_data.measurement_report.ncells.arfcn[i];
      rrd->rxlev       = rr_data->ms_data.measurement_report.ncells.rx_lev[i];
      rrd->bcch_status = NON_DECODED;
      rrd->avail_time  = 0;
    }
    else
      rrd->bcch_status = EMPTY;
  }

  /*
   * update the serving cell fieldstrength data.
   * It is important to make distinction between rxlev of the traffic channel and of the BCCH channel
   * for C1 and C2 calculations
   */
  rr_data->nc_data[SC_INDEX].rxlev = rr_data->ms_data.measurement_report.bcch_rxlev_of_sc;

  /*
   * implementation problem: must be set to have a defined status.
   */
  rr_data->nc_data[SC_INDEX].bcch_status = DECODED;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : RR_ATT                        |
| STATE   : code             ROUTINE : att_check_network             |
+--------------------------------------------------------------------+

  PURPOSE : This function is a SW shield for the FTA campaign. Due
            to bad cables and public networks with high fieldstrength
            some FTA testcases (especially MM testcases with limited
            services) fails on our Anite System. To avoid this these
            function is a SW shield, excluding all german networks.

            This function must be adapted for other countries if needed.

*/

GLOBAL UBYTE att_check_network (T_loc_area_ident * lai)
{
  GET_INSTANCE_DATA;
  UBYTE count=0; 
  /*
   * if a test SIM is inserted and "Software Shielding" is enabled then check mcc value with stored mcc value
   * (durig dynamic configuration command shield).The network exists in the list updated using dynamic
   * configauration command SHIELD then return FALSE.
   */
  if ((test_house EQ TRUE) AND (rr_data->dyn_config.mcc_shield.enabled EQ TRUE))
  {
    for(count=0;count< MAX_MCC_SHIELD;count++)
    {
      if (lai->mcc[0] EQ rr_data->dyn_config.mcc_shield.mcc[count][0] AND
          lai->mcc[1] EQ rr_data->dyn_config.mcc_shield.mcc[count][1] AND
          lai->mcc[2] EQ rr_data->dyn_config.mcc_shield.mcc[count][2] )     
       {
         TRACE_EVENT ("att_check_network() returns FALSE");
         return FALSE;
       }
     }
  }
  return TRUE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : RR_ATT                      |
| STATE   : code               ROUTINE : att_return_to_idle          |
+--------------------------------------------------------------------+

  PURPOSE : The functions configures PL for idle mode without going
            through a complete cell reselection.

*/

GLOBAL void att_return_to_idle (void)
{
  GET_INSTANCE_DATA;

  TIMERSTOP (T_RESELECT);
  /*
   * configure layer 1
   */
#ifdef GPRS
  if(att_gprs_cell_has_pbcch())
  {
    att_gprs_stop_pl();
  }
  else
  {
#endif
    att_build_idle_req (SC_INDEX, MODE_CELL_SELECTION);
#ifdef GPRS
  }
#endif

  SET_STATE (STATE_ATT, ATT_IDLE);
  dat_att_cell_selected();
  srv_use_stored_prim ();
#ifdef REL99
#ifdef GPRS  
  /* RR is returning to idle mode. If cbch info was received
   * during transfer mode, send it now.
   */ 
   if(rr_data->gprs_data.cbch_info_rxvd_in_ptm)
     {
       rr_data->gprs_data.cbch_info_rxvd_in_ptm = FALSE;
       att_config_cbch();          
     }  
#endif
#endif
  /*
   * Start registration timer if needed
   */
  att_start_registration_timer ();

  rr_data->bcch_error       = 0;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : RR_ATT                      |
| STATE   : code               ROUTINE : att_notify_stop_plmn_search |
+--------------------------------------------------------------------+

  PURPOSE : Network search has to be aborted in lower layer so that
            NC monitoring can be resumed when going back to idle mode.
*/

GLOBAL void att_notify_stop_plmn_search (UBYTE deactivate_pl)
{
  GET_INSTANCE_DATA;
  UCHAR  state = GET_STATE (STATE_ATT);

  TRACE_EVENT_P1("att_notify_stop_plmn_search - %x", rr_data->start_cell_reselection);

  if ( rr_data->ms_data.req_mm_service EQ FUNC_NET_SRCH_BY_MMI                              OR
      (rr_data->ms_data.rr_service EQ LIMITED_SERVICE AND !rr_data->start_cell_reselection) OR
      (rr_data->ms_data.rr_service EQ NO_SERVICE AND !rr_data->start_cell_reselection)      OR
       state EQ ATT_CS1                                                                     OR
       state EQ ATT_CS2                                                                     OR 
       CS_GET_CURRENT_SEARCH_MODE EQ BLACK_LIST_SEARCH_MODE)
  {
    PALLOC (mph_sync_req, MPH_SYNC_REQ);

    if(deactivate_pl)
      mph_sync_req->cs = CS_STOP_PLMN_SEARCH_AND_DEACTIVATE;
    else
      mph_sync_req->cs = CS_STOP_PLMN_SEARCH;

    PSENDX (PL, mph_sync_req);

    rr_data->start_cell_reselection = TRUE;

    TRACE_EVENT_P1("Start_cell_reselection %d",rr_data->start_cell_reselection);

  }
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : RR_ATT                      |
| STATE   : code               ROUTINE : att_reset_old_lai                  |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to reset old lai parameters
*/

GLOBAL void att_reset_old_lai_rac ()
{
  GET_INSTANCE_DATA;
  rr_data->old_cell_id = NOT_PRESENT_16BIT;
#ifdef GPRS
  rr_data->old_rac     = NOT_PRESENT_8BIT;
#endif
  memset (&rr_data->old_lai, NOT_PRESENT_8BIT, sizeof(T_loc_area_ident));

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : RR_ATT                      |
| STATE   : code               ROUTINE : att_copy_old_lai_rac        |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to copy old lai and rac parameters
*/

GLOBAL void att_copy_old_lai_rac (U8 index)
{
  GET_INSTANCE_DATA;
  memcpy (&rr_data->old_lai, &rr_data->nc_data[index].lai,
          sizeof(T_loc_area_ident));
  rr_data->old_cell_id =  rr_data->nc_data[index].cell_id;
#ifdef GPRS
  rr_data->old_rac     =  rr_data->nc_data[index].rac;
#endif
}

/*
+------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : RR_ATT                                |
| STATE   : code               ROUTINE : att_check_dynamic_search_mode_config  |
+------------------------------------------------------------------------------+

  PURPOSE : This function updates the new search mode based on the current 
            dynamic search mode configuration
            CSI-LLD section:4.1.3.4.1.8
*/

GLOBAL void att_check_dynamic_search_mode_config(void)
{
  GET_INSTANCE_DATA;
  U8 new_mode = CS_GET_CURRENT_SEARCH_MODE;

  TRACE_FUNCTION("att_check_dynamic_search_mode_config()");

  TRACE_EVENT_P1("New Search Mode : %d", CS_GET_CURRENT_SEARCH_MODE);

  if(new_mode EQ FAST_SEARCH_MODE AND !rr_data->dyn_config.tfast_cs_val)
  {
    /* If Fast search is disabled, use Normal search */
    new_mode = NORMAL_SEARCH_MODE;

    TRACE_EVENT("Fast Search is disabled");
  }

  if(new_mode EQ NORMAL_SEARCH_MODE AND !rr_data->dyn_config.tnormal_cs_val)
  {
    /* If Normal search is disabled, use Full search */
    new_mode = FULL_SEARCH_MODE;

    TRACE_EVENT("Normal Search is disabled");
  }

  if(new_mode NEQ CS_GET_CURRENT_SEARCH_MODE)
    TRACE_EVENT_P1("Allowed Search Mode : %d", new_mode);

  CS_SET_CURRENT_SEARCH_MODE(new_mode);

}


/* 
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : RR_ATT                      |
| STATE   : code               ROUTINE : att_get_txpwr_max_cch       |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to calculate the MAX power used on CCH
*/

LOCAL UBYTE att_get_txpwr_max_cch (UBYTE index)
{
  GET_INSTANCE_DATA;
  UBYTE max_cch = rr_data->nc_data[index].select_para.ms_txpwr_max_cch;
  UBYTE dbm_value;

  TRACE_FUNCTION("att_get_txpwr_max_cch()");

  switch(std)
  {
    case STD_1800:
    case STD_DUAL_EGSM:
    case STD_DUAL:
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT    
    case STD_850_1800:
    case STD_850_900_1800:
#endif
      /* Ref 04.18 Section 10.5.2.35
       * Rest Octets IE includes parameters which are used by the mobile station
       * for cell selection and reselection purposes. It may also include the 
       * POWER OFFSET parameter used by DCS 1800 Class 3 MS.
       */
      if (INRANGE(LOW_CHANNEL_1800,rr_data->nc_data[index].arfcn,HIGH_CHANNEL_1800))
      {
        /* DCS 1800 */
        if (att_get_power() EQ 0x02 AND 
               rr_data->nc_data[index].c2_par.power_off_ind)
        {
          TRACE_EVENT(" IDLE_REQ :DCS_PCLASS3");
          /* Class 3 & Valid Power offset */
          dbm_value = p_control_dcs[max_cch] +  
                        (rr_data->nc_data[index].c2_par.power_off << 1);

          /* dbm value is more than 36 return the max cch power level as 29 */
          if(dbm_value > 36) return 29;

          if(dbm_value NEQ 0)
          {
            /* map the calculated dbm value to power control value 
             * Specification : 0505  Section : 4.1.1 
             */
            max_cch =  (30 - dbm_value) >= 0 ?
                                  (30 - dbm_value) >> 1 :
                                  (31 - ((dbm_value - 32) >> 1));
          }
        } /* if att_get_power */
      } /* if INRANGE */
      break;
    default :
      break;
  } /* switch std */
  return max_cch;
}

#if !defined(NTRACE)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : RR_ATT                      |
| STATE   : code               ROUTINE : att_print_mcc_mnc           |
+--------------------------------------------------------------------+

  PURPOSE : Outputs to trace mcc and mnc arrays

*/
/* Implements Measure#32: Row 52,53, 124 & 125 */
GLOBAL void att_print_mcc_mnc (USHORT arfcn, UBYTE *mcc, UBYTE *mnc, T_S2I_STRING titel)
{
  TRACE_EVENT_P8 ( "[%d] MCC=%x%x%x MNC=%x%x%x %s",
    ((int)arfcn), mcc[0], mcc[1], mcc[2], mnc[0], mnc[1], mnc[2],
    S2I_STRING(titel)); /* A valid string is expected */
}

LOCAL void att_print_selection_type (UBYTE selection_type)
{
  GET_INSTANCE_DATA;
  switch (rr_data->sc_data.selection_type)
  {
    case CELL_SELECTION:
      TRACE_EVENT_WIN ("selection_type=CELL_SELECTION");
      break;
    case CELL_RESELECTION:
      TRACE_EVENT_WIN ("selection_type=CELL_RESELECTION");
      break;
    case BACK_FROM_DEDICATED:
      TRACE_EVENT_WIN ("selection_type=BACK_FROM_DEDICATED");
      break;
    case CELL_RESELECTION_NC:
      TRACE_EVENT_WIN ("selection_type=CELL_RESELECTION_NC");
      break;
    case BACK_FROM_DEDICATED_RLF:
      TRACE_EVENT_WIN ("selection_type=BACK_FROM_DEDICATED_RLF");
      break;
    case CELL_RESELECTION_RACH:
      TRACE_EVENT_WIN ("selection_type=CELL_RESELECTION_RACH");
      break;
    case CELL_RESELECTION_CR:
      TRACE_EVENT_WIN ("selection_type=CELL_RESELECTION_CR");
      break;
  }
}

/* Implements Measure#32: Row 36, 39 and 40 */
LOCAL void att_print_op (T_op *op, T_S2I_STRING titel)
{
  T_S2I_STRING sim, mode;
  if (op->v_op)
  {
    sim = op->sim_ins ? (op->ts ? S2I_STRING("TEST") : S2I_STRING("NORMAL")) : S2I_STRING("NO");
    mode = op->m ? S2I_STRING("MAN") : S2I_STRING("AUTO");

    TRACE_EVENT_P5 ("%s: SIM=%s mode=%s func=%s service=%s",
      S2I_STRING(titel),
      S2I_STRING(sim),
      S2I_STRING(mode),
      S2I_STRING(_rr_str_FUNC[op->func]),
      S2I_STRING(_rr_str_SERVICE[op->service]));
  }
}
#endif /* !NTRACE */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : RR_ATT                      |
| STATE   : code               ROUTINE : att_set_rr_service_info     |
+--------------------------------------------------------------------+

  PURPOSE : Copies the RR related service info(service,PLMN,lac,cell_id)
*/

GLOBAL void att_set_rr_service_info (void)
{
  GET_INSTANCE_DATA;

  T_LOC_INFO loc_info;
  loc_info.service_mode = rr_data->ms_data.rr_service;
  if(rr_data->ms_data.rr_service NEQ NO_SERVICE)
  {
    loc_info.lac = rr_data->nc_data[SC_INDEX].lai.lac;
    loc_info.cell_id = rr_data->nc_data[SC_INDEX].cell_id;
    memcpy (loc_info.mcc,rr_data->nc_data[SC_INDEX].lai.mcc,SIZE_MCC);
    memcpy (loc_info.mnc,rr_data->nc_data[SC_INDEX].lai.mnc,SIZE_MNC);
  }
  cl_shrd_set_loc(&loc_info);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : RR_ATT                      |
| STATE   : code               ROUTINE : att_set_tim_advance_info    |
+--------------------------------------------------------------------+

  PURPOSE : Copies the Timing Advance info.ME_STATUS is presently using
  only ME_STATUS_IDLE which is not as per the standard.We need to assign
  me_status correctly.
*/

GLOBAL void att_set_tim_advance_info (void)
{
  GET_INSTANCE_DATA;
  T_TIM_ADV tim_adv;
  tim_adv.me_status = ME_STATUS_IDLE;
  tim_adv.tm_adv = rr_data->sc_data.new_ta;
  cl_shrd_set_tim_adv(&tim_adv);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : RR_ATT                      |
| STATE   : code               ROUTINE : att_func1_opt               |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the system information read flag of the passed 
  si_type . It saves the neighbour cell list or Merges it to previously
  saved list if available.
*/
LOCAL void att_copy_sys_info_2bis_2ter_par(UBYTE index,T_SI_TYPE si_type,  
                                           T_LIST  *new_2_bis_ter_list,  BUF_neigh_cell_desc  *neigh_cell_desc,
                                           UBYTE   indicate_changes)
{
  GET_INSTANCE_DATA;
  T_LIST  new_list;
  T_CELL_DATA   *cd;
  BOOL          modified = FALSE;

  USHORT cur_si_type_read; /*Current SI type*/
  USHORT oth_si_type_read; /*The other SI type in this function */
  USHORT oth_si_to_clean;/*The other SI to clean in this function*/
  UCHAR si_type_msg; /* 2BIS or 2TER type msg*/

  TRACE_FUNCTION("att_copy_sys_info_2bis_2ter_par()");
 
  switch (index)
  {
    case SC_INDEX:
    case CR_INDEX:
      if(si_type EQ SI_TYPE_2BIS)
      {
        cur_si_type_read = SYS_INFO_2BIS_READ;
        oth_si_type_read = SYS_INFO_2TER_READ;
        oth_si_to_clean = IND_SI_2TER;
        si_type_msg = SYS_INFO_2bis_MSG;

      }
      else
      {
        cur_si_type_read = SYS_INFO_2TER_READ;
        oth_si_type_read = SYS_INFO_2BIS_READ;
        oth_si_to_clean = IND_SI_2BIS;
        si_type_msg = SYS_INFO_2ter_MSG;
      }
      if (index EQ SC_INDEX)
      {
        cd = &rr_data->sc_data.cd;
        if ((cd->sys_info_read & cur_si_type_read) EQ cur_si_type_read)
        {
          srv_copy_list (&cd->ncell_list, new_2_bis_ter_list, sizeof (T_LIST));
          att_clean_buf ((USHORT)(IND_SI_2 | oth_si_to_clean));
          cd->sys_info_read &= ~(SYS_INFO_2_READ | oth_si_type_read);
          att_check_2ter_read (index);
        }
        else
          srv_merge_list (&cd->ncell_list, new_2_bis_ter_list);
        modified = TRUE;
#ifdef GPRS
        rr_data->gprs_data.ba_bcch_modified= FALSE;
#endif
      }
      else
      {
        cd = &rr_data->cr_data.cd;
        /*
         * merge 2bis list with the old neighbour cell list
         */
        srv_copy_list (&new_list, &cd->ncell_list,
                       sizeof (T_LIST));
        srv_merge_list (&new_list, new_2_bis_ter_list);

        if (srv_compare_list (&cd->ncell_list, &new_list) EQ FALSE)
        {
          /*
           * both lists are different
           */
          srv_copy_list (&cd->ncell_list, &new_list, sizeof (T_LIST));
          modified = TRUE;
        }
      }
      /*
       * CSI-LLD section:4.1.1.11
       * This function Updates the black list with the BA list received
       * in si2bis/ter
       */
      cs_remove_BA_MA_from_black_list(rr_data->cs_data.region,&cd->ncell_list);

      dat_store_neigh_cell_desc (si_type_msg, index,
                                 neigh_cell_desc, &cd->ncell_list);
      /*
       * Indicate that the system information type 2bis message has been read.
       */
      att_set_sys_info_read (cur_si_type_read, index);

      /*
       * forward new neighbour cell list to layer 1
       * if changes shall be indicated
       */
      if (modified AND indicate_changes)
      {
        att_code_mph_ncell_req (index);
      }

      break;
  }
}


#if defined (REL99) && defined (TI_PS_FF_EMR)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : RR_ATT                      |
| STATE   : code               ROUTINE : attf_send_enh_para_to_alr   |
+--------------------------------------------------------------------+

  PURPOSE : This function formats and sends a primitive with enhanced
  measurement parameters.
*/
GLOBAL void attf_send_enh_para_to_alr(UBYTE rep_type, T_enh_para_struct *p_src)
{
  PALLOC(p_enh, MPH_ENHPARA_UPDATE_REQ);
  TRACE_FUNCTION ("attf_send_enh_para_to_alr");
  p_enh->rep_type = rep_type;
  p_enh->enh_para = *p_src;
  PSENDX(PL,p_enh);
  return;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : RR_ATT                      |
| STATE   : code               ROUTINE : att_print_mcc_mnc           |
+--------------------------------------------------------------------+

  PURPOSE : Outputs to trace mcc and mnc arrays

*/
LOCAL void att_check_for_si5ter_and_enhpara (UBYTE old_index)
{
  GET_INSTANCE_DATA;
  T_rr_enh_para *p_temp = &rr_data->sc_data.emr_data_temp;
  T_rr_enh_para *p_cur = &rr_data->sc_data.emr_data_current;
  
  /*we have complete BA(SACCH) list (with or without SI-5 ter ) : we will
  discard any enhanced para that are present in previous state which are
  aligned to BA(BCCH)*/
  if (rr_data->sc_data.enh_para_status EQ ENH_PARA_IDLE )
  {
    /*The enhanced parameters available are from different state, reset them*/
    for_set_default_emr_data(p_temp);
    memset (rr_data->sc_data.rep_count, NOT_PRESENT_8BIT, MAX_NEIGHBOURCELLS);
    for_set_default_emr_data(p_cur);
    for_send_enh_para(p_cur);
    rr_data->sc_data.enh_para_status = ENH_PARA_INVALID_STATE;
    return;
  }
  if(old_index EQ rr_data->sc_data.ba_index) 
  {
  /*Updation of enhanced cell list for EMR : we can receive
  SI-5ter and MI in any random order*/
  if ( (rr_data->sc_data.emr_data_temp.is_data_valid EQ TRUE) AND
    rr_data->sc_data.emr_data_temp.ba2bsic_map_pending )        
    for_att_update_ba2bsic_mapping(&rr_data->sc_data.emr_data_temp);
  else if ( (rr_data->sc_data.emr_data_current.is_data_valid EQ TRUE) AND
    rr_data->sc_data.emr_data_current.ba2bsic_map_pending )
    for_att_update_ba2bsic_mapping(&rr_data->sc_data.emr_data_current);
  
    /*When BA(SACCH) is ready, check whether there are enhanced parameters in
  temp that needs attention*/
  if ( (rr_data->sc_data.enh_para_status EQ ENH_PARA_DEDICATED) AND
    (p_temp->is_data_valid EQ TRUE ) )
  {
  /* This means enhanced parameters were received before BA list - so update 
    the enhanced list with actual ARFCN and update current EMR data*/
    if ( for_update_enh_cell_list( rr_data->act_ncell_list) EQ TRUE)
    {
      *p_cur = *p_temp; 
      if (p_cur->ba2bsic_map_pending EQ 0) /*send parameters when there are no more mapping pending*/
        for_send_enh_para(p_cur);
    }          
    /*Reset temporary, irrespective of whether updation is succesful or not*/
    for_set_default_emr_data(p_temp);      
  }    
  else if (p_cur->is_data_valid EQ TRUE AND p_cur->ba2bsic_map_pending EQ 0)
      for_send_enh_para(p_cur);
  }
  else 
  {
    for_set_default_emr_data(p_temp);
    for_set_default_emr_data(p_cur);
    for_send_enh_para(p_cur);
  } 
  return;
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : RR_ATT                        |
| STATE   : code             ROUTINE : att_update_std_band_indicator |
+--------------------------------------------------------------------+

  PURPOSE : This function updates the std value depending upon the band
            indicator value received in the SI1 or SI6 message. band 
            indicator will be ignored when received in the DCS 1800 or
            PCS 1900 frequency band. Depending upon the std value region 
            will be updated.

*/
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
GLOBAL void att_update_std_band_indicator (UBYTE band_indicator)
{
  GET_INSTANCE_DATA;
  UBYTE band_index = cs_get_band_index (rr_data->nc_data[SC_INDEX].arfcn);
  UBYTE freq_bands;
  UBYTE new_region = NOT_PRESENT_8BIT;
  UBYTE mscr = 0;     /* variable to hold msc release version*/

  TRACE_FUNCTION ("att_update_std_band_indicator");

  get_msc_release_version(&mscr);
  TRACE_EVENT_P1("mscr (MSC release) version : 0x%X", mscr);
  if (mscr NEQ MSCR_99)
  {
    TRACE_EVENT ("band indicator received when MSC release is not R99");
    TRACE_ERROR ("band indicator received when MSC release is not R99");
    return;
  }

  if ((band_index EQ B_DCS_1800) OR (band_index EQ B_PCS_1900))
  {
    TRACE_EVENT_P2 ("band indicator %x received in band %x", band_indicator, band_index);
    return;
  }

  rr_csf_get_freq_bands (&freq_bands);
  if (band_indicator NEQ NOT_PRESENT_8BIT)
  {
    if (band_indicator EQ BAND_IND_1800)
    {
      if ((freq_bands & BAND_DCS_1800) EQ BAND_DCS_1800)
      {
        if (std EQ STD_DUAL_US)
        {
          std = STD_850_1800;
          new_region = BOTH_REGIONS;
        }
        else if (std EQ STD_850_900_1900)
        {
          std = STD_850_900_1800;
          new_region = BOTH_REGIONS;
        }
      }
    }
    else
    {
      if ((freq_bands & BAND_PCS_1900) EQ BAND_PCS_1900)
      {
        if (std EQ STD_DUAL_EGSM)
        {
          std = STD_900_1900;
          new_region = BOTH_REGIONS;
        }
        else if (std EQ STD_850_900_1800)
        {
          std = STD_850_900_1900;
          new_region = BOTH_REGIONS;
        }
      }
    }
    TRACE_EVENT_P1 ("Updated std to %x", std);
  }

  if (new_region NEQ NOT_PRESENT_8BIT)
    rr_data->cs_data.region = new_region;
}
#endif

#endif  /* RR_ATTF_C */
