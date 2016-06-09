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
|  Purpose :  This module defines the functions for the data transfer
|             capability of the module Radio Resource.
+-----------------------------------------------------------------------------
*/

#ifndef RR_DATF_C
#define RR_DATF_C

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

/*==== EXPORT =====================================================*/

/*==== PRIVATE ====================================================*/
static void  dat_fill_mobile_identity    (USHORT              fill_type,
                                          T_mob_ident         *moid);
static BOOL  dat_eplmn_equal_req         (const UBYTE *mcc,
                                          const UBYTE *mnc);
static void  dat_get_background_and_bits (UBYTE               *background,
                                          UBYTE               *bits);
static UBYTE dat_get_burst               (UBYTE               background,
                                          UBYTE               bits);
static UBYTE dat_get_delta               (UBYTE               i);
static UBYTE dat_get_ncell_pos           (USHORT              channel);
static BOOL  dat_hplmn_country           (const UBYTE         *mcc);
static SHORT dat_imsi_mod_1000           (void);
static SHORT dat_no_of_paging_blocks     (UBYTE               index);
static BOOL  dat_owner_of_auth_0_to_9    (void);
static BOOL  dat_owner_of_auth_11_to_15  (void);
static void  dat_send_random_bursts      (void);

typedef struct  CODE_TABLE
{
  USHORT  cause;
  UBYTE   last_channel;
  UBYTE   neci_flag;
  UBYTE   channel_needed;
  UBYTE   ms_capability;
  UBYTE   tch_f_needed;
  UBYTE   background;
  UBYTE   bits;
} CODE_TABLE ;

typedef struct CONVER_TXINTEGER
{
  USHORT  t;
  USHORT  s_non_combined;
  USHORT  s_combined;
} CONVERT_TXINTEGER;

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/


/*
 * -------------------------------------------------------------------
 * Procedures
 * -------------------------------------------------------------------
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_access_allowed         |
+--------------------------------------------------------------------+

  PURPOSE : The function decides whether the access to the network is 
            allowed or not. 
            (GSM 4.18, chapter 3.3.1.1.1 Permission to access the network).

*/

GLOBAL BOOL dat_access_allowed (USHORT    establish_cause)
{
  GET_INSTANCE_DATA;
  BOOL  result      = TRUE;

  TRACE_FUNCTION ("dat_access_allowed()");

#if defined (_SIMULATION_)
  TRACE_EVENT_P1 ("ACC CLASS BCCH = %4x", rr_data->nc_data[SC_INDEX].rach.ac);
  TRACE_EVENT_P1 ("ACC CLASS SIM  = %4x", rr_data->ms_data.access_classes);
#endif

  if (establish_cause EQ ESTCS_EMERGENCY_CALL)
  {
    /*
     * if it is an emergency call the corresponding flag
     * in the random access control parameters must be set
     */
    if (rr_data->nc_data[SC_INDEX].rach.ac & 0x0400)
      result = FALSE;

    /*
     * or the MS must be member of one of the special classes
     * defined on the SIM card.
     */
    if (result EQ FALSE)
    {
      result = dat_owner_of_auth_11_to_15 ();
    }
  }
  else
  {
    /*
     * for non-ememrgency calls
     */

    /*
     * Normally, the whole check here should not be necessary as MM
     * should know about limited service condition and not try to
     * update in automatic mode or to establish a non-emergency CM service.
     * So the check here is superflous, but if it catches, MM has a problem
     * as it will think the cell is temporary barred and will wait for the
     * barr state to change, not indicating limited service to the MMI under
     * certain circumstances. It seems so that here something was fixed at
     * the wrong place.
     * The old condition was: if (dat_forbidden_lai_check (SC_INDEX))
     */
    if(
         (establish_cause EQ ESTCS_LOCATION_UPDATING)
           AND
         ((rr_data->ms_data.operation_mode & 0x40) EQ 0x40)
      )
    {
      /*
        * Do not barr access for MM procedure establishment if we are in manual
        * mode. This was previously bypassed within dat_forbidden_lai_check()
        * function but was leading to incorrect cell reselection whilst RR
        * was in manual.
        * If MM wants to perform a LU at this place it means that user manually
        * selected this network and is then allowed to attempt registration. If
        * we reseleted on a LAC forbidden for provision of regional service, MM
        * is switched to limited service so no normal LU will be triggered.
        */
      if (dat_owner_of_auth_0_to_9 ())
        result = TRUE;
      else
        result = dat_owner_of_auth_11_to_15 ();
    }
    else if ( rr_data->ms_data.rr_service EQ LIMITED_SERVICE)
    {
      /*
        * RR is in limited service.
        * Only emergency calls are allowed in this state
        */
       result = FALSE;
    }
    else if (
                 dat_forb_lai_check (SC_INDEX)
                   AND
                 dat_roam_forb_lai_check (SC_INDEX)
               )
    {
      /*
       * if RR is not inside of a forbidden location area,
       * the MS must be member of one of the normal or one
       * of the special classes stored on the SIM card.
       */
      if (dat_owner_of_auth_0_to_9 ())
        result = TRUE;
      else
        result = dat_owner_of_auth_11_to_15 ();

    }
    else
    {
      /*
       * if RR is inside of a forbidden location area
       * no normal calls are allowed
       */
      TRACE_ERROR ("Unexpected, MM doesn't know its service state.");
      result = FALSE;
    }
  }

  return (result);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_calc_downlink_timeout  |
+--------------------------------------------------------------------+

  PURPOSE : The downlink signalling failure criterion is based on the
            downlink signalling failure counter DSC. When the MS
            camps on a cell, DSC shall be initialized to a value equal
            to the nearest integer to 90/N where N is the BS_PA_MFRMS
            parameter for that cell (GSM 5.08, chapter 6.5 Downlink
            Signalling Failure).

*/

GLOBAL UBYTE dat_calc_downlink_timeout (UBYTE     index)
{
  GET_INSTANCE_DATA;
  UBYTE divisor;
  UBYTE dl=0;

  TRACE_FUNCTION ("dat_calc_downlink_timeout()");

  /*
   * The stored value for BS_PA_MFRMS is in air-interface coding.
   * To get the real value a value of 2 must be added
   */
  divisor = rr_data->nc_data[index].control_descr.bs_pa_mfrms + 2;

  /*
   * calculate the initial value for the downlink signalling counter.
   */
  TRACE_ASSERT(divisor NEQ 0);
  if(divisor NEQ 0)
  {
    dl = 90 / divisor;
  }

  /*
   * correct rounding failures
   *
   * BS_PA_MFRMS =   2   -> 90/2   = 45
   *                 3   -> 90/3   = 30
   *                 4   -> 90/4   = 22.5   -> 23
   *                 5   -> 90/5   = 18
   *                 6   -> 90/6   = 15
   *                 7   -> 90/7   = 12.85  -> 13
   *                 8   -> 90/8   = 11.25
   *                 9   -> 90/9   = 10
   */
  if (divisor EQ 4 OR divisor EQ 7)
  {
    dl++;
  }
  return (dl);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_calc_paging_group      |
+--------------------------------------------------------------------+

  PURPOSE : Calculation of paging group is described in GSM 5.02,
            chapter 6.5.2.

            PAGING_GROUP (0 .. N-1) = ((IMSI mod 1000) mod (BS_CC_CHANS x N)) mod N

            where

            N = number of paging blocks "available" on one CCCH =
                (number of paging blocks "available" in a 51-multiframe
                 on one CCCH) x BS_PA_MFRMS.

            IMSI = International Mobile Subscriber Identity, as defined in GSM 03.03.

            mod = Modulo.

*/

GLOBAL UBYTE dat_calc_paging_group (UBYTE     index)
{
  GET_INSTANCE_DATA;
  /*
   * calculation of the number of paging blocks
   */
  SHORT n = dat_no_of_paging_blocks (index);

  /*
   * calculation of IMSI modulo 1000
   */
  SHORT a = dat_imsi_mod_1000 ();

  /*
   * calculation of BS_CC_CHANS * N (GSM 5.02 section 3.3.2.3)
   */
  SHORT b = ((rr_data->nc_data[index].control_descr.ccch_conf / 2) + 1) * n;

  TRACE_FUNCTION ("dat_calc_paging_group()");

  /*
   * calculation of the paging group
   */
  return ((UBYTE) ((a % b) % n));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_calc_tn                |
+--------------------------------------------------------------------+

  PURPOSE : Determination of the timeslot of the paging block for the
            MS in idle mode is described in GSM 5.02, chapter 6.5.2

            The formula for calculation of the CCCH group must be used.
            The dependency between timeslot and CCCH group is

            tn = 2 * CCCH_GROUP

            CCCH_GROUP (0 .. BS_CC_CHANS-1) = ((IMSI mod 1000) mod (BS_CC_CHANS x N)) div N

            where

            N = number of paging blocks "available" on one CCCH =
                (number of paging blocks "available" in a 51-multiframe
                on one CCCH) x BS_PA_MFRMS.

            IMSI = International Mobile Subscriber Identity, as defined in GSM 03.03.

            mod = Modulo.

            div = Integer division.

*/

GLOBAL UBYTE dat_calc_tn (UBYTE      index)
{
  GET_INSTANCE_DATA;
  /*
   * calculate the number of paging blocks
   */
  SHORT n = dat_no_of_paging_blocks (index);

  /*
   * calculate IMSI modulo 1000
   */
  SHORT a = dat_imsi_mod_1000 ();

  /*
   * calculate BS_CC_CHANS * N (GSM 5.02 section 3.3.2.3)
   */
  SHORT b = ((rr_data->nc_data[index].control_descr.ccch_conf / 2) + 1) * n;

  TRACE_FUNCTION ("dat_calc_tn()");

  /*
   * calculate the timeslot
   */
  return ((UBYTE) ((a % b) / n) * 2);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_check_error_flag       |
+--------------------------------------------------------------------+

  PURPOSE : In the formatter module several tests are performed to
            check the syntax and the semantic of the incoming messages.
            The results of this checks are stored in the error variables.
            This function sends a RR STATUS message back to the network
            if a mandatory or conditional error has been detected for
            a message received in acknowledged mode. It indicates to the
            calling function whether the message shall be ignored or not.

*/

GLOBAL BOOL dat_check_error_flag (BOOL      send_rr_status)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_check_error_flag()");

  /*
   * if an error unequal to optional info error has occured
   */
  if (rr_data->ms_data.error.cs NEQ 0 AND
      rr_data->ms_data.error.cs NEQ OPTIONAL_INFO_ERROR)
  {
    if (send_rr_status)
    {
      /*
       * if the message has been received in acknowledged mode,
       * answer to the network with a RR STATUS message.
       */
/* Implements RR Clone findings #23 */
      dat_send_rr_status_msg(rr_data->ms_data.error.cs);
    }
  }

  switch (rr_data->ms_data.error.cs)
  {
    /* case RRC_INVALID_MAN_INFO: this value is currently never set */
    case RRC_INCORRECT_MSG:
    case RRC_COND_IE_ERROR:
      /*
       * Major failure in the message, ignore it
       */
      return FALSE;

    default:
      /*
       * minor or no failure, process the message
       */
      return TRUE;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_code_channel_mode_to_mm|
+--------------------------------------------------------------------+

  PURPOSE : A changed channel mode is signalled to MM.

*/

GLOBAL void dat_code_channel_mode_to_mm  (void)
{
  GET_INSTANCE_DATA;
  PALLOC (sync_ind, RR_SYNC_IND);

  TRACE_FUNCTION ("dat_code_channel_mode_to_mm()");

  /*
   * set the channel type
   */
  switch (rr_data->sc_data.chan_desc.chan_type)
  {
    case CH_TCH_F:
      TRACE_EVENT ("TCH/F configured");
      sync_ind->chm.ch_type  = CH_TCH_F;
      break;
    case CH_TCH_H_1:
    case CH_TCH_H_2:
      TRACE_EVENT ("TCH/H configured");
      sync_ind->chm.ch_type  = CH_TCH_H;
      break;
    default:
      TRACE_EVENT ("SDCCH configured");
      sync_ind->chm.ch_type = CH_SDCCH;
      break;
  }

  /*
   * set the rest of the parameters
   */
  sync_ind->ciph             = NOT_PRESENT_8BIT;
  sync_ind->chm.ch_mode      = rr_data->sc_data.ch_mode;
  memset(&sync_ind->mm_info, 0, sizeof(T_mm_info));
  sync_ind->mm_info.valid    = FALSE;
  memset(&sync_ind->bcch_info, 0, sizeof(T_bcch_info));
  sync_ind->bcch_info.v_bcch = FALSE;
  sync_ind->synccs           = NOT_PRESENT_16BIT;

  PSENDX(MM, sync_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_code_ciphering_to_mm   |
+--------------------------------------------------------------------+

  PURPOSE : A changed ciphering mode is signalled to MM.

*/

GLOBAL void dat_code_ciphering_to_mm (UBYTE ciph_on)
{
  PALLOC (sync_ind, RR_SYNC_IND);

  TRACE_FUNCTION ("dat_code_ciphering_to_mm()");

  /*
   * set the new cipher mode
   */
  sync_ind->ciph             = ciph_on;

  /*
   * clear the rest of the parameters
   */
  sync_ind->chm.ch_mode      = NOT_PRESENT_8BIT;
  memset(&sync_ind->mm_info, 0, sizeof(T_mm_info));
  sync_ind->mm_info.valid    = FALSE;
  memset(&sync_ind->bcch_info, 0, sizeof(T_bcch_info));
  sync_ind->bcch_info.v_bcch = FALSE;
  sync_ind->synccs           = NOT_PRESENT_16BIT;

  PSENDX(MM, sync_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : RR_DAT                        |
| STATE   : code             ROUTINE : dat_code_reestablishment_fail |
+--------------------------------------------------------------------+

  PURPOSE : Indicate a failed call reestablishment to MM. If a radio
            link failure has occured, RR performs a cell reselection
            to come back to idle mode and then the reestablishment may
            starts. If no suitable cell is available, it may takes
            a long time until no service is signalled. This function is
            used to finish the reestablish earlier, if no candidate
            is available during coming back from dedicated.

*/

GLOBAL void dat_code_reestablishment_fail (void)
{
  GET_INSTANCE_DATA;
  T_NC_DATA *rrd = &rr_data->nc_data[SC_INDEX];

  PALLOC (sync_ind, RR_SYNC_IND);

  TRACE_FUNCTION ("dat_code_reestablishment_fail()");

  sync_ind->mm_info.valid    = TRUE;
  sync_ind->mm_info.att      = rrd->control_descr.att;
  /*
   * No reestablishment
   */
  sync_ind->mm_info.re       = 1;
  sync_ind->mm_info.ncc      = (rrd->bsic >> 3) & 7;
  sync_ind->mm_info.bcc      = rrd->bsic & 7;
  sync_ind->mm_info.t3212    = rrd->control_descr.t3212;
  /*sync_ind->mm_info.la       = !dat_forb_lai_check (SC_INDEX);*/
  sync_ind->mm_info.la       = (!(dat_forb_lai_check (SC_INDEX) AND
                                  dat_roam_forb_lai_check (SC_INDEX)));

  sync_ind->ciph             = NOT_PRESENT_8BIT;;
  sync_ind->chm.ch_mode      = NOT_PRESENT_8BIT;
  sync_ind->bcch_info.v_bcch = FALSE;
  sync_ind->synccs           = NOT_PRESENT_16BIT;

  PSENDX (MM, sync_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_code_measure_report    |
+--------------------------------------------------------------------+

  PURPOSE : RR builds a RR MEASUREMENT REPORT message for the uplink
            sacch, whenever it receives a measurement report from the
            layer 1.

*/

GLOBAL void dat_code_measure_report (T_MPH_MEASUREMENT_IND *report)
{
  GET_INSTANCE_DATA;
  USHORT                i;

  MCAST (meas, U_MEAS_REP);
  PALLOC_MSG (dl_unitdata_req, DL_UNITDATA_REQ, U_MEAS_REP);/* T_DL_UNITDATA_REQ */

  TRACE_FUNCTION ("dat_code_measure_report()");
  memset (&dl_unitdata_req->sdu.buf[0], 0, dl_unitdata_req->sdu.o_buf / BITS_PER_BYTE);

  /*
   * initialize C-structure for the Uplink message
   */
  memset (meas, 0, sizeof (T_U_MEAS_REP));

  /*
   * set message type
   */
  meas->msg_type                = U_MEAS_REP;

  if (report->valid)
  {
    /*
     * measurement report from layer 1 is valid,
     * then copy data to C-structure for message
     */
    meas->meas_result.ba_used     = rr_data->sc_data.ba_index;
    meas->meas_result.dtx_used    = report->dtx;
    meas->meas_result.meas_valid  = 0;

    if (rr_data->dyn_config.fho)
    {
      /*
       * forced handover, special test feature to simulate
       * a bad serving cell
       */
      meas->meas_result.rxlev_full = 0;
      meas->meas_result.rxlev_sub  = 0;
    }
    else
    {
      /*
       * take values from layer 1
       */
      meas->meas_result.rxlev_full  = report->rx_lev_full;
      meas->meas_result.rxlev_sub   = report->rx_lev_sub;
    }

    meas->meas_result.rxqual_full = report->rx_qual_full;
    meas->meas_result.rxqual_sub  = report->rx_qual_sub;

    /*
     * copy neighbourcell values
     */
    meas->meas_result.num_ncell   = report->ncells.no_of_ncells;

#if defined (REL99) && defined (TI_PS_FF_EMR)
    if ( (rr_data->sc_data.ba_list_idle EQ TRUE) OR
         (rr_data->sc_data.ba_list_ded  EQ TRUE) )   
#else
    /*
     * report ncells only if BA complete or
     * it's an expansion (5ter) and the 5/5bis are sent before
     */
    if ( (rr_data->sc_data.cd.sys_info_read &  (SYS_INFO_5_READ | SYS_INFO_5BIS_READ))
                                            EQ (SYS_INFO_5_READ | SYS_INFO_5BIS_READ) )
#endif
    {
      /*
       * fill ncells to the measurement report
       */
      for (i=0;i<meas->meas_result.num_ncell;i++)
      {
        meas->meas_result.ncell[i].bsic = report->ncells.bsic[i];

        /*
         * set position in neighbourcell list instead of channel number
         */
        meas->meas_result.ncell[i].bcch_ncell = dat_get_ncell_pos (report->ncells.arfcn[i]);
        meas->meas_result.ncell[i].rx_lev_ncell = report->ncells.rx_lev[i];

        TRACE_EVENT_P4 ("MR:%u[%4u] p=%u rxl=%u",
          i, report->ncells.arfcn[i],
          meas->meas_result.ncell[i].bcch_ncell,
          meas->meas_result.ncell[i].rx_lev_ncell); /* +++ */

      }
    }
    else
    {
      /*
       * Table 10.5.47/GSM 04.08: Measurement Results information element
       * Range: 0 to 7 (See GSM 05.08)
       * NO-NCELL-M, Number of neighbouring cell measurements (octets 4 and 5)
       *
       * No neighbour cell measurement result := 0
       * Neighbour cell information not available for serving cell := 7
       */
      meas->meas_result.num_ncell = 7;
    }

  }
  else
  {
    /*
     * measurement report from layer 1 is invalid
     */
    TRACE_EVENT ("invalid Meas");
    meas->meas_result.meas_valid = 1;
  }

  /*
   * code message and send to layer 2.
   */
  for_dat_unitdata_req (dl_unitdata_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_code_ext_meas_report   |
+--------------------------------------------------------------------+

  PURPOSE : RR builds a RR EXTENDED MEASUREMENT REPORT message for the
            uplink sacch.
*/

GLOBAL void dat_code_ext_meas_report (T_MPH_EMO_MEAS_IND  *mph_emo_meas_ind)
{
  GET_INSTANCE_DATA;
  USHORT          *emo_arfcn = rr_data->  emo_arfcn;
  UBYTE          c_emo_arfcn = rr_data->c_emo_arfcn;
  T_ext_meas_res  *ext_meas_res;
  UBYTE           *rx_lev_ncell;
  T_meas_results  *meas_results;
  UBYTE           i,k;

  MCAST (u_ext_meas_report, U_EXT_MEAS_REPORT);
  PALLOC_MSG (dl_unitdata_req, DL_UNITDATA_REQ, U_EXT_MEAS_REPORT);

  TRACE_FUNCTION ("dat_code_ext_meas_report()");

  /*
   * initialize C-structure for the Uplink message
   */
  memset (u_ext_meas_report, 0, sizeof (T_U_EXT_MEAS_REPORT));

  ext_meas_res = &u_ext_meas_report->ext_meas_res;

  /*
   * set message type
   */
  u_ext_meas_report->msg_type = U_EXT_MEAS_REPORT;

  ext_meas_res->sc_used  = rr_data->emo_seq;
  ext_meas_res->dtx_used = mph_emo_meas_ind->dtx;

  rx_lev_ncell = &ext_meas_res->rx_lev_ncell[0];
  meas_results = &mph_emo_meas_ind->meas_results[0];

  TRACE_ASSERT( c_emo_arfcn <=  MAX_EMO_CHANNELS);
  TRACE_ASSERT( mph_emo_meas_ind->c_meas_results <= (MAX_EMO_CHANNELS +1));

  for ( k = 0; k < c_emo_arfcn; k++ )
  {
    for ( i = 0; i < mph_emo_meas_ind->c_meas_results; i++ )
    {
      if ( emo_arfcn[k] EQ meas_results[i].arfcn )
      {
        rx_lev_ncell[k] = meas_results[i].rx_lev;
      }
    }
  }

  /*
   * code message and send to layer 2.
   */
  for_dat_unitdata_req (dl_unitdata_req);

}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_code_mph_chan_mode_req |
+--------------------------------------------------------------------+

  PURPOSE : configure a new channel mode to layer 1.

*/

GLOBAL void dat_code_mph_chan_mode_req (T_D_CHAN_MOD *chan_mod)
{
  GET_INSTANCE_DATA;
  PALLOC (channel_mode_req, MPH_CHANNEL_MODE_REQ);

  TRACE_FUNCTION ("dat_code_mph_chan_mode_req()");

  memset(channel_mode_req, 0, sizeof(T_MPH_CHANNEL_MODE_REQ));

  /*
   *  configure layer 1 with multi-rate configuration if present
   */
  if ( rr_data->sc_data.ch_mode EQ CM_AMR )
  {
    int i;
    channel_mode_req->amr_conf.nscb    = rr_data->sc_data.amr_conf.nscb;
    channel_mode_req->amr_conf.icmi    = rr_data->sc_data.amr_conf.icmi;
    channel_mode_req->amr_conf.st_mode = rr_data->sc_data.amr_conf.st_mode;
    channel_mode_req->amr_conf.acs     = rr_data->sc_data.amr_conf.set_amr;

    channel_mode_req->amr_conf.v_cod_prop = rr_data->sc_data.amr_conf.v_cod_prop;
    if(channel_mode_req->amr_conf.v_cod_prop)
    {
      channel_mode_req->amr_conf.c_cod_prop = rr_data->sc_data.amr_conf.c_cod_prop;
      for (i=0; i< channel_mode_req->amr_conf.c_cod_prop; i++)
        memcpy(&channel_mode_req->amr_conf.cod_prop[i],
               &rr_data->sc_data.amr_conf.cod_prop[i], sizeof(T_cod_prop));
    }
  }

  /*
   * set new channel mode
   */
  channel_mode_req->ch   = chan_mod->chan_desc.chan_type;
  channel_mode_req->mode = chan_mod->chan_mode;
  PSENDX (PL, channel_mode_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_code_mph_ciphering_req |
+--------------------------------------------------------------------+

  PURPOSE : configure new cipher parameter to layer 1.

*/

GLOBAL void dat_code_mph_ciphering_req (UBYTE       ciph_on,
                                        UBYTE       algo,
                                        UBYTE     * kc)
{
  PALLOC (mph_ciphering_req, MPH_CIPHERING_REQ);

  TRACE_FUNCTION ("dat_code_mph_ciphering_req()");

  if (ciph_on)
  {
    /*
     * ciphering is on, then set cipher algorithm
     * and Kc value.
     */
    mph_ciphering_req->ciph.stat    = CIPH_ON;
    mph_ciphering_req->ciph.algo    = algo;
    memcpy (mph_ciphering_req->ciph.kc, kc, KC_STRING_SIZE);
  }
  else
  {
    /*
     * ciphering is off, then set default values
     */
    mph_ciphering_req->ciph.stat    = CIPH_OFF;
    mph_ciphering_req->ciph.algo    = 0;
    memset (mph_ciphering_req->ciph.kc, 0, KC_STRING_SIZE);
  }

  PSENDX (PL, mph_ciphering_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_code_sys_info_change   |
+--------------------------------------------------------------------+

  PURPOSE : A change in parameters of system information type 6
            has been detected. The parameters are configured
            in layer 1.

*/

GLOBAL void dat_code_sys_info_change  (UBYTE     dtx,
                                       UBYTE     pwrc,
                                       UBYTE     rlt)
{
  PALLOC (dedicated_req, MPH_DEDICATED_REQ);

  TRACE_FUNCTION ("dat_code_sys_info_change()");

  memset (dedicated_req, 0, sizeof (T_MPH_DEDICATED_REQ));

  /*
   * set new dtx, rlt, pwrc and ncc_permitted values
   */
  dedicated_req->tr_para.dtx   = dtx;
  dedicated_req->tr_para.rlt   = rlt;
  dedicated_req->tr_para.pwrc  = pwrc;
  dedicated_req->mod           = MODE_SYS_INFO_CHANGE;

  PSENDX (PL, dedicated_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : RR_DAT                      |
| STATE   : code               ROUTINE : dat_code_mph_imm_assign_req |
+--------------------------------------------------------------------+

  PURPOSE : configure layer 1 after reception of an immediate assignment
            or immediate assignment extended message.

*/

GLOBAL void dat_code_mph_imm_assign_req (T_start      *start,
                                         UBYTE         power,
                                         UBYTE         maio,
                                         T_LIST       *freq_after_sti,
                                         T_LIST       *freq_bef_sti)
{
  GET_INSTANCE_DATA;

  PALLOC (dedicated_req, MPH_DEDICATED_REQ);

  TRACE_FUNCTION ("dat_code_mph_imm_assign_req()");

  memset (dedicated_req, 0, sizeof (T_MPH_DEDICATED_REQ));

  dedicated_req->mod = MODE_IMM_ASSIGN;

  /*
   * set starting time if available
   */
  memcpy (&dedicated_req->start, start,
          sizeof (T_start));

  /*
   * set channel type
   */
  dedicated_req->ch_type.ch    = rr_data->sc_data.chan_desc.chan_type;
  dedicated_req->ch_type.tn    = rr_data->sc_data.chan_desc.tn;
  dedicated_req->ch_type.tsc   = rr_data->sc_data.chan_desc.tsc;
  dedicated_req->ch_type.h     = rr_data->sc_data.chan_desc.hop;

  if (rr_data->sc_data.chan_desc.hop EQ H_NO)
  {
    /*
     * set channel number if no hopping is configured
     */
    dedicated_req->ch_type.arfcn = rr_data->sc_data.chan_desc.arfcn;
  }
  else
  {
    /*
     * set maio, hsn and hopping list, if hopping is configured
     */
    dedicated_req->ch_type.maio  = rr_data->sc_data.chan_desc.maio;
    dedicated_req->ch_type.hsn   = rr_data->sc_data.chan_desc.hsn;

    /* CSI-LLD section:4.1.1.11
     * This function Updates the black list with the MA list received
     * in immediate_assignment req
     */
    cs_remove_BA_MA_from_black_list(rr_data->cs_data.region,freq_after_sti);

    srv_create_list (freq_after_sti, dedicated_req->ch_type.ma,
                     MAX_MA_CHANNELS, TRUE, 0);
  }

  /*
   * set channel type 2 (only maio and mobile allocation)
   */
  dedicated_req->ch_type2.maio  = maio;

  /* CSI-LLD section:4.1.1.11
   * This function Updates the black list with the MA list received
   * in immediate_assignment req
   */
  cs_remove_BA_MA_from_black_list(rr_data->cs_data.region,freq_bef_sti);

  srv_create_list (freq_bef_sti, dedicated_req->ch_type2.ma, MAX_MA_CHANNELS,
                   TRUE, 0);

  dedicated_req->arfcn         = rr_data->nc_data[SC_INDEX].arfcn;

  /*
   * set power, dtx, rlt, pwrc, timing advance and channel mode
   */
  dedicated_req->tr_para.power = power;
  dedicated_req->tr_para.dtx   = rr_data->sc_data.cd.dtx;
  dedicated_req->tr_para.rlt   = rr_data->sc_data.cd.cell_options.rlt;
  dedicated_req->tr_para.pwrc  = rr_data->sc_data.cd.cell_options.pow_ctrl;
  dedicated_req->tr_para.tav   = rr_data->sc_data.new_ta;
  dedicated_req->tr_para.mode  = rr_data->sc_data.ch_mode;

  RR_EM_GET_HOPPING_CHANNEL(dedicated_req->ch_type.ma,dedicated_req->ch_type2.ma,
                            dedicated_req->start.v_start,maio);

  PSENDX (PL, dedicated_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_code_mph_freq_redef_req|
+--------------------------------------------------------------------+

  PURPOSE : configure a new hopping list after reception of a frequency
            redefinition message.

*/

GLOBAL void dat_code_mph_freq_redef_req (T_start   *start,
                                         T_LIST    *hop_list)
{
  GET_INSTANCE_DATA;

  T_chan_desc * chan_desc = &rr_data->sc_data.chan_desc;

  PALLOC (freq_redef_req, MPH_FREQ_REDEF_REQ);

  TRACE_FUNCTION ("dat_code_mph_freq_redef_req()");

  /*
   * copy start time
   */
  memcpy (&freq_redef_req->start, start, sizeof (T_start));

  /*
   * set new hopping list
   */
  srv_create_list (hop_list, freq_redef_req->ch_type.ma, MAX_MA_CHANNELS, TRUE,0);

  /* CSI-LLD section:4.1.1.11
   * This function Updates the black list with the MA list received
   * in Frequency redifinition message
   */
  cs_remove_BA_MA_from_black_list(rr_data->cs_data.region,hop_list);

  /*
   * set channel type, timeslot, training sequence code,
   * hopping indication, maio and hsn.
   */
  freq_redef_req->ch_type.ch   = chan_desc->chan_type;
  freq_redef_req->ch_type.tn   = chan_desc->tn;
  freq_redef_req->ch_type.tsc  = chan_desc->tsc;
  freq_redef_req->ch_type.h    = chan_desc->hop;
  freq_redef_req->ch_type.maio = chan_desc->maio;
  freq_redef_req->ch_type.hsn  = chan_desc->hsn;

  rr_data->mode_after_dedi = MODE_CELL_RESELECTION;

  EM_FREQ_REDEF;

  RR_EM_GET_HOPPING_CHANNEL (freq_redef_req->ch_type.ma, freq_redef_req->ch_type.ma, FALSE,0);

  PSENDX (PL, freq_redef_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_code_mph_old_chan_req  |
+--------------------------------------------------------------------+

  PURPOSE : During channel assignment or handover the layer 2 link is
            suspended and a new channel is configured. Then the layer 2
            connection is resumed on the new channel. If this fails,
            RR switches back to the old channel. The trigger for this
            is this function.

*/

GLOBAL void dat_code_mph_old_chan_req (void)
{
  PALLOC ( mph_dedicated_fail_req, MPH_DEDICATED_FAIL_REQ);

  TRACE_FUNCTION ("dat_code_mph_old_chan_req()");

  PSENDX (PL, mph_dedicated_fail_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_code_prr_channel       |
+--------------------------------------------------------------------+

  PURPOSE : Set the channel type and the service access point
            identification according to the configured channel type.

*/

GLOBAL void dat_code_prr_channel (UBYTE     *ch_type,
                                  UBYTE     *sapi,
                                  UBYTE      chan_type)
{
  TRACE_FUNCTION ("dat_code_prr_channel()");

  /*
   * SAPI is always 0
   */
  *sapi    = SAPI_0;

  /*
   * convert air-interface coding of channel type
   * to internal values.
   */
  switch (chan_type)
  {
    case CH_TCH_F:
      *ch_type = L2_CHANNEL_FACCH_F;
      break;
    case CH_TCH_H_1:
    case CH_TCH_H_2:
      *ch_type = L2_CHANNEL_FACCH_H;
      break;
    default:
      *ch_type = L2_CHANNEL_SDCCH;
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_code_prr_channel_sms   |
+--------------------------------------------------------------------+

  PURPOSE : Set the channel type and the service access point
            identification according to the configured channel type
            for short messages.

*/

GLOBAL void dat_code_prr_channel_sms (T_DL_DATA_REQ *dl_data_req,
                                      UBYTE         chan_type)
{
  TRACE_FUNCTION ("dat_code_prr_channel_sms()");

  /*
   * sapi is always 3
   */
  dl_data_req->sapi    = SAPI_3;

  /*
   * channel type is SACCH if the main channel (sapi = 0)
   * is FACCH, else it is SDCCH.
   */
  dl_data_req->ch_type = (chan_type < CH_SDCCH_4_0)
                         ? L2_CHANNEL_SACCH : L2_CHANNEL_SDCCH;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_code_prr_bcch_info     |
+--------------------------------------------------------------------+

  PURPOSE : create a list of channels from the ba_range information
            element of a channel release message.

*/

GLOBAL void dat_code_prr_bcch_info (UBYTE        v_ba_range,
                                    T_ba_range * ba_range)
{
  T_LIST   list;
  USHORT   i;
  USHORT   j;
  USHORT   x1;
  USHORT   x2;

  TRACE_FUNCTION ("dat_code_prr_bcch_info()");

  /*
   * initialization : the list is empty.
   */
  srv_clear_list (&list);

  if (v_ba_range)
  {
    /*
     * only if the information element is inside the
     * channel release message.
     * Then for all ranges inside the information element.
     */
    for (i=0;i<ba_range->c_freq_range;i++)
    {
      x1 = ba_range->freq_range[i].freq_lower;
      x2 = ba_range->freq_range[i].freq_higher;
      /*
       *   set interval borders
       */
      if (x1 > HIGH_CHANNEL_900)
        x1 = HIGH_CHANNEL_900;
      if (x2 > HIGH_CHANNEL_900)
        x2 = HIGH_CHANNEL_900;

      if (x1 EQ x2)
      {
        /*
         * add x1 to channel list if both boarders have the same value
         */
        srv_set_channel (&list, x1);
      }

      if (x1 < x2)
      {
        /*
         * add x1..x2 to channel list if the boarders define a range
         */
        for (j=x1;j<=x2;j++)
          srv_set_channel (&list, j);
      }

      if (x1 > x2)
      {
        /*
         * add LOW_CHANNEL_900..x2 and x1..HIGH_CHANNEL_900 to channel list
         */
        for (j=LOW_CHANNEL_900;j<=HIGH_CHANNEL_900;j++)
          if (j <= x2 OR j>=x1)
            srv_set_channel (&list, j);
      }
    }

    /*
     * send the resulting list to the SIM card or store it inside the PCM
     */
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_send_bcchinfo_mm       |
+--------------------------------------------------------------------+

  PURPOSE : Send neigbour cell description to MM

*/

GLOBAL void dat_send_bcchinfo_mm (UBYTE *p)
{
  PALLOC (sync_ind, RR_SYNC_IND);
  sync_ind->ciph             = NOT_PRESENT_8BIT;
  sync_ind->chm.ch_mode      = NOT_PRESENT_8BIT;
  sync_ind->mm_info.valid    = FALSE;
  sync_ind->bcch_info.v_bcch = TRUE;
  sync_ind->synccs           = NOT_PRESENT_16BIT;
  if (p)
  {
    memcpy (sync_ind->bcch_info.bcch, p, BA_BITMAP_SIZE);
    TRACE_EVENT_P4 ("BCCHINFO: send 16 byte to MM/SIM (%x,%x,%x,%x,...)", p[0], p[1], p[2], p[3]);
  }
  else
  {
    memset (sync_ind->bcch_info.bcch, 0, BA_BITMAP_SIZE);
    TRACE_EVENT ("BCCHINFO: clear SIM");
  }
  PSENDX (MM, sync_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_convert_white_list     |
+--------------------------------------------------------------------+

  PURPOSE : Updates White List after cell selection and cell reselection
            in Full service
            CSI-LLD section:4.1.2.2.2
*/

GLOBAL void dat_convert_white_list(void)
{
  GET_INSTANCE_DATA;
  U8 *p = NULL;
  U8 i;
  T_LIST tmp_list;
  BUF_neigh_cell_desc cd;

  TRACE_FUNCTION("dat_convert_white_list()");

  if(rr_data->ms_data.rr_service EQ FULL_SERVICE)
  {
    TRACE_EVENT("CR white list -> CS white list");

    /* Clear the old White List info */
    memset(&rr_data->cs_data.white_list, 0x00, sizeof(T_CS_WHITE_LIST));

    /* copy the serving cell ARFCN */
    rr_data->cs_data.white_list.last_sc_arfcn = rr_data->nc_data[SC_INDEX].arfcn;

    /* copy the serving cell region */
    rr_data->cs_data.white_list.region = rr_data->cs_data.region;

    /* Copy the serving cell location area identity */
    memcpy(&rr_data->cs_data.white_list.last_sc_lac,&rr_data->nc_data[SC_INDEX].lai,
            sizeof(T_loc_area_ident));

    /* Convert CR white list into T_LIST format and store the same */
    for(i=0;i<=32;i+=BA_BITMAP_SIZE)
    {
      switch(i)
      {
        case 0:
         p = rr_data->cr_data.cr_white_list.si2;
         break;
        case 16:
         p = rr_data->cr_data.cr_white_list.si2bis;
         break;
        case 32:
         p = rr_data->cr_data.cr_white_list.si2ter;
         break;
        default:
          continue;
      }

      if(p NEQ NULL)
      {
        memcpy(cd.b_neigh_cell_desc,p,BA_BITMAP_SIZE);
        cd.o_neigh_cell_desc = 0;
        cd.l_neigh_cell_desc = NCELL_DESC_BIT_LEN;

        for_create_channel_list((T_f_range *)&cd,&tmp_list);
        srv_merge_list(&rr_data->cs_data.white_list.list,&tmp_list);
      }
    }

    /* Use last serving cell information also */
    if(rr_data->cs_data.white_list.last_sc_arfcn NEQ NOT_PRESENT_16BIT)
    {
      srv_set_channel(&rr_data->cs_data.white_list.list,
                      rr_data->cs_data.white_list.last_sc_arfcn&ARFCN_MASK);
     }

    TRACE_EVENT_P9 ( "White List:[%d]Reg,[%d]Arfcn MCC/MNC r=%x%x%x/%x%x%x/%d",
      rr_data->cs_data.white_list.region,
      rr_data->cs_data.white_list.last_sc_arfcn,
      rr_data->cs_data.white_list.last_sc_lac.mcc[0],
      rr_data->cs_data.white_list.last_sc_lac.mcc[1],
      rr_data->cs_data.white_list.last_sc_lac.mcc[2],
      rr_data->cs_data.white_list.last_sc_lac.mnc[0],
      rr_data->cs_data.white_list.last_sc_lac.mnc[1],
      rr_data->cs_data.white_list.last_sc_lac.mnc[2],
      rr_data->cs_data.white_list.last_sc_lac.lac);

  } /* Full service */
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_store_neigh_cell_desc  |
+--------------------------------------------------------------------+

  PURPOSE : Store neigbour cell description in case of full service

*/

GLOBAL void dat_store_neigh_cell_desc (UBYTE si, UBYTE index,
                                       BUF_neigh_cell_desc *cd,
                                       T_LIST *new_neigh_list)
{
  GET_INSTANCE_DATA;
  U8 *p = NULL;
  U16 o;
  U8 att_state = GET_STATE (STATE_ATT);

#if defined(_SIMULATION_)
  {
    BOOL  plmn_ok;

    plmn_ok = dat_plmn_equal_req (rr_data->nc_data[index].lai.mcc,
                                  rr_data->nc_data[index].lai.mnc,
                                  rr_data->ms_data.plmn.mcc,
                                  rr_data->ms_data.plmn.mnc);
    TRACE_EVENT_P8 ("dat_store_neigh_cell_desc(): srv:%s op:%s tried:%u, st:%s, NC%u plmn:%u CR:%d SC:%d",
      _rr_str_SERVICE[rr_data->ms_data.rr_service],
      _rr_str_FUNC[rr_data->ms_data.req_mm_service],
      (rr_data->cs_data.scan_mode EQ CS_SECOND_SCAN),
      STATE_ATT_NAME[att_state], index, plmn_ok,
      ((int)rr_data->nc_data[CR_INDEX].arfcn),
      ((int)rr_data->nc_data[SC_INDEX].arfcn));
  }
#endif  /* 0|1 */

  if (rr_data->ms_data.req_mm_service EQ FUNC_PLMN_SRCH)
  {
    /*
     * In states ATT_CS2 and ATT_CS3, store BA lists from SI2, 2Bis and 2Ter
     * inside rr_data->cr_data
     */
    if ((att_state EQ ATT_CS2 OR att_state EQ ATT_CS3) AND (index EQ CR_INDEX))
    {
      switch (si)
      {
        case SYS_INFO_2_MSG:
          p = rr_data->cr_data.cr_white_list.si2;
          break;
        case SYS_INFO_2bis_MSG:
          p = rr_data->cr_data.cr_white_list.si2bis;
          break;
        case SYS_INFO_2ter_MSG:
          p = rr_data->cr_data.cr_white_list.si2ter;
          break;
        default:
          return;
      }

      TRACE_EVENT_P3 ("BCCHINFO: store cd of [%d]i%u: si=%02x ",
                       rr_data->nc_data[index].arfcn, index, si);

      /* compare; store and indicate only if changed */
      o = cd->o_neigh_cell_desc>>3;
      if (p NEQ NULL)
      {
        if (memcmp (p, &cd->b_neigh_cell_desc[o], BA_BITMAP_SIZE))
        {
          memcpy (p, &cd->b_neigh_cell_desc[o], BA_BITMAP_SIZE);
        }
      }
    } /* CR_INDEX */

    else if((att_state EQ ATT_IDLE) AND (index EQ SC_INDEX) AND
            (rr_data->ms_data.rr_service EQ FULL_SERVICE))
    {
      /* In state ATT_IDLE, store BA list directly inside the white list
       * (only if we are in Full Service)
       */
      srv_copy_list (&rr_data->cs_data.white_list.list, new_neigh_list,
                     sizeof (T_LIST));

      /* Add current serving cell to White List */
      srv_set_channel(&rr_data->cs_data.white_list.list,
                      rr_data->nc_data[SC_INDEX].arfcn&ARFCN_MASK);

      if(si EQ SYS_INFO_2_MSG)
      {
        /* In case it is the description of system information 2 (and only then)
         * it should be stored in the SIM card.
         */
        o = cd->o_neigh_cell_desc>>3;
        dat_send_bcchinfo_mm (&cd->b_neigh_cell_desc[o]);
      }
    }

    /* additional storing of current serving cell and cell-re-selection cell */
    rr_data->cs_data.arfcn_sc = rr_data->nc_data[SC_INDEX].arfcn;
    rr_data->cs_data.arfcn_cr = rr_data->nc_data[CR_INDEX].arfcn;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_compare_request_ref    |
+--------------------------------------------------------------------+

  PURPOSE : Reference is GSM 4.08, chapter 3.3.1.1.3.1 On receipt of a
            CHANNEL REQUEST message

            On receipt of an IMMEDIATE ASSIGNMENT or IMMEDIATE ASSIGNMENT
            EXTENDED message corresponding to one of its 3 last CHANNEL
            REQUEST messages, the message shall be identified as the
            channel description for the MS.

*/

GLOBAL BOOL dat_compare_request_ref (T_req_ref *req_ref, UBYTE * index)
{
  GET_INSTANCE_DATA;
  UBYTE        from;
  UBYTE        i;

  TRACE_FUNCTION ("dat_compare_request_ref()");

  /*
   * RR stores the request references for all outgoing
   * channel requests. The variable from indicates the
   * beginning of the maximum 3 last channel requests.
   */

  from = (rr_data->ms_data.access_counter > 2) ?
          rr_data->ms_data.access_counter - 3 : 0;

  TRACE_EVENT_P4 ("compare: %d %d %d 0x%02x",
    req_ref->t1, req_ref->t2, req_ref->t3, req_ref->ra);

  TRACE_ASSERT(  rr_data->ms_data.access_counter <=  MAX_RACH_REQ);

  for (i = from; i < rr_data->ms_data.access_counter; i++)
  {
    /*
     * RR checks the sending time T1/T2/T3 of the channel
     * request message and the content of the message.
     * If all matches the immediate assignment (extended)
     * message is identified for the MS.
     */
    TRACE_EVENT_P5 ("with[%u]: %d %d %d 0x%02x", i,
      rr_data->used_frame_no[i].t1, rr_data->used_frame_no[i].t2,
      rr_data->used_frame_no[i].t3, rr_data->used_channel_ref[i]);

    if (rr_data->used_frame_no[i].t1 EQ req_ref->t1 AND
        rr_data->used_frame_no[i].t2 EQ req_ref->t2 AND
        rr_data->used_frame_no[i].t3 EQ req_ref->t3 AND
        rr_data->used_channel_ref[i] EQ req_ref->ra)
    {
      *index = i;
      return TRUE;
    }
  }

  /*
   * The message is not for the MS
   */
  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_disconnect_link        |
+--------------------------------------------------------------------+

  PURPOSE : After reception of a channel release message this function
            starts the disconnection of the link in layer 2.

*/

GLOBAL void dat_disconnect_link (USHORT cause)
{
  GET_INSTANCE_DATA;
  UBYTE  ch_type;

  PALLOC (dl_release_req, DL_RELEASE_REQ);

  TRACE_FUNCTION ("dat_disconnect_link()");

  rr_data->rel_cause      = cause;

  /*
   * set channel type and sapi according the configured channel
   * configuration
   */
  dat_code_prr_channel (&dl_release_req->ch_type,
                        &dl_release_req->sapi,
                        rr_data->sc_data.chan_desc.chan_type);

  ch_type = dl_release_req->ch_type;
  dl_release_req->mode = DL_NORMAL_RELEASE;
  PSENDX (DL, dl_release_req);

  /*
   * control layer 2 release. The timer shall be set in a way
   * that layer 2 has enough time for at least two DISC frames.
   * So the value of the timer depends on the channel type
   * (SDCCH or FACCH).
   */
  if (ch_type EQ L2_CHANNEL_SDCCH)
  {
    TIMERSTART (T3110, T3110_SDCCH_VALUE);
  }
  else
  {
    TIMERSTART (T3110, T3110_VALUE);
  }

  SET_STATE (STATE_DAT, DAT_CHAN_REL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_fill_mobile_identity   |
+--------------------------------------------------------------------+

  PURPOSE : In several update message the mobile identity must be set.
            For mobile terminated message the mobile identity of
            the paging message must be used.
            For mobile originated connections the mobile identity type
            depends on the availability of the mobile identities in
            this order:  TMSI, IMSI, no identity

*/

static void dat_fill_mobile_identity (USHORT      fill_type,
                                     T_mob_ident *moid)
{
  GET_INSTANCE_DATA;
  UBYTE mobile_type;

  TRACE_FUNCTION ("dat_fill_mobile_identity()");

  memset (moid, 0, sizeof (T_mob_ident));

  /*
   * for MTC set the type according the paging identity type.
   */
  if (fill_type EQ ESTCS_PAGING)
    mobile_type = rr_data->page_identity_type;
  else
  {
    /*
     * for MOC set the type according the availability
     */
    if (rr_data->ms_data.tmsi_available)
      mobile_type = TYPE_TMSI;
    else
      mobile_type = (rr_data->ms_data.imsi_available)
                    ? TYPE_IMSI : TYPE_NO_ID;
  }

  /*
   * fill the identity according the calculated type
   */
  switch (mobile_type)
  {
    case TYPE_TMSI:
      TRACE_EVENT ("FILL TMSI");
      moid->ident_type      = TYPE_TMSI;
      moid->tmsi_1.l_tmsi_1   = 32;
      moid->tmsi_1.o_tmsi_1   = 0;
      moid->odd_even        = 0;
      moid->v_tmsi_1        = TRUE;
      ccd_codeByte (moid->tmsi_1.b_tmsi_1, 0, 8, (UBYTE)(rr_data->ms_data.tmsi_binary >> 24));
      ccd_codeByte (moid->tmsi_1.b_tmsi_1, 8, 8, (UBYTE)(rr_data->ms_data.tmsi_binary >> 16));
      ccd_codeByte (moid->tmsi_1.b_tmsi_1, 16, 8, (UBYTE)(rr_data->ms_data.tmsi_binary >> 8));
      ccd_codeByte (moid->tmsi_1.b_tmsi_1, 24, 8, (UBYTE)rr_data->ms_data.tmsi_binary);
      break;

    case TYPE_IMSI:
      TRACE_EVENT ("FILL IMSI");
      memcpy (moid, &rr_data->ms_data.imsi, sizeof (T_mob_ident));
      break;

    default:
      TRACE_EVENT ("FILL NOTHING");
      memset (moid, 0, sizeof (T_mob_ident));
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_check_forb_list        |
+--------------------------------------------------------------------+

  PURPOSE : In automatic mode a cell shall be only selected if it is
            not member of a forbidden location area list.
            This function checks the membership in a forbidden LAI
            independant of the selected mode (manual or automatic).

*/

LOCAL BOOL dat_check_forb_list (int list_type, UBYTE index)
{
  GET_INSTANCE_DATA;
  int i;
  T_loc_area_ident *forb_list;

  TRACE_FUNCTION ("dat_check_forb_list()");

  if (list_type EQ FORBIDDEN_LIST_NORMAL)
    forb_list = &rr_data->ms_data.forb_lac_list[0];
  else
    forb_list = &rr_data->ms_data.roam_forb_lac_list[0];

  TRACE_ASSERT(index < NCELL_SIZE);
  
  /*
   *
   * check only in automatic mode
   *
   * if ((rr_data->ms_data.operation_mode & 0x40) EQ 0) M_MAN
   */
  {
    /*
     * check all entries of this list
     */
    for (i = 0; i < MAX_LAI; i++)
    {
      if ((rr_data->nc_data[index].lai.lac EQ forb_list[i].lac) AND
          dat_plmn_equal_req (rr_data->nc_data[index].lai.mcc,
                              rr_data->nc_data[index].lai.mnc,
                              forb_list[i].mcc,
                              forb_list[i].mnc))/*lint !e661 !e662 (possible access/creation of out-of-bounds pointer)*/
      {
        /*
         * the check is failed if the location area code is stored.
         */
        return FALSE;
      }
    }
  }
  /*
   * the check has passed.
   */
  return TRUE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_forb_lai_check         |
+--------------------------------------------------------------------+

  PURPOSE : In automatic mode a cell shall be only selected if it is
            not member of a forbidden location area list.

*/

GLOBAL BOOL dat_forb_lai_check (UBYTE index)
{
  TRACE_FUNCTION ("dat_forb_lai_check()");

  return dat_check_forb_list (FORBIDDEN_LIST_NORMAL, index);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_roam_forb_lai_check    |
+--------------------------------------------------------------------+

  PURPOSE : In automatic mode a cell shall be only selected if it is
            not member of a forbidden location area list.

*/

GLOBAL BOOL dat_roam_forb_lai_check (UBYTE index)
{
  TRACE_FUNCTION ("dat_roam_forb_lai_check()");

  return dat_check_forb_list (FORBIDDEN_LIST_ROAMING, index);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_get_background_and_bits|
+--------------------------------------------------------------------+

  PURPOSE : The content of a channel request consists of two parts:
            The establishment cause and a random value. The function
            calculates the establishment cause (here defined as background)
            and the number of bits for the random value.

            The establishment cause inside the channel request message
            depends on several parameters:

            the internal used establishment cause,
            the last used channel
            the NECI-Flag in the system information message
            the needed channel indicated in a paging message
            the mobile station capabilities and
            whether a TCH fullrate is needed

            The abbreviation IG means Ignore. In this case the parameter
            is not relevant for the calculation.

*/
#ifdef GPRS
#define TABLE_SIZE 30
#else
#define TABLE_SIZE 21
#endif

static const CODE_TABLE coding_table [TABLE_SIZE] =
/*
 *  establish cause         last NECI  channel   MS    TCH/F   Back-  Bits
 *                       channel        needed  cap.  needed  ground
 */
{ ESTCS_EMERGENCY_CALL,       IG,  IG,      IG,   IG,     IG,  0xA0,    5,
  ESTCS_REESTABLISHMENT,  TCHFCH,  IG,      IG,   IG,     IG,  0xC0,    5,
  ESTCS_REESTABLISHMENT,  TCHHCH,   0,      IG,   IG,     IG,  0xC0,    5,
  ESTCS_REESTABLISHMENT,  TCHHCH,   1,      IG,   IG,     IG,  0x68,    2,
#ifdef GPRS
  ESTCS_GPRS_PAGING,          IG,  IG,   ANYCH,   IG,     IG,  0x80,    5,
  ESTCS_GPRS_PAGING,          IG,  IG,  TCHFCH, FULL,     IG,  0x80,    5,
  ESTCS_GPRS_PAGING,          IG,  IG, TCHHFCH, FULL,     IG,  0x80,    5,
  ESTCS_GPRS_PAGING,          IG,  IG,  TCHFCH, DUAL,     IG,  0x20,    4,
  ESTCS_GPRS_PAGING,          IG,  IG, TCHHFCH, DUAL,     IG,  0x30,    4,
  ESTCS_GPRS_PAGING,          IG,  IG, SDCCHCH,   IG,     IG,  0x10,    4,
  ESTCS_GPRS_PAGING,          IG,  IG,      IG, SIGN,     IG,  0x10,    4,
#endif
  ESTCS_PAGING,               IG,  IG,   ANYCH,   IG,     IG,  0x80,    5,
  ESTCS_PAGING,               IG,  IG,  TCHFCH, FULL,     IG,  0x80,    5,
  ESTCS_PAGING,               IG,  IG, TCHHFCH, FULL,     IG,  0x80,    5,
  ESTCS_PAGING,               IG,  IG,  TCHFCH, DUAL,     IG,  0x20,    4,
  ESTCS_PAGING,               IG,  IG, TCHHFCH, DUAL,     IG,  0x30,    4,
  ESTCS_PAGING,               IG,  IG, SDCCHCH,   IG,     IG,  0x10,    4,
  ESTCS_PAGING,               IG,  IG,      IG, SIGN,     IG,  0x10,    4,
  ESTCS_MOC_SPEECH,           IG,   0,      IG,   IG,     IG,  0xE0,    5,
  ESTCS_MOC_SPEECH,           IG,   1,      IG, DUAL,  FALSE,  0x40,    4,
  ESTCS_MOC_SPEECH,           IG,  IG,      IG,   IG,     IG,  0xE0,    5,
  ESTCS_MOC_DATA,             IG,   0,      IG,   IG,     IG,  0xE0,    5,
  ESTCS_MOC_DATA_HR_SUFF,     IG,   1,      IG, DUAL,  FALSE,  0x50,    4,
  ESTCS_MOC_DATA,             IG,  IG,      IG,   IG,     IG,  0xE0,    5,
  ESTCS_LOCATION_UPDATING,    IG,   0,      IG,   IG,     IG,  0x00,    5,  /* new */
  ESTCS_LOCATION_UPDATING,    IG,   1,      IG,   IG,     IG,  0x00,    4,
  ESTCS_MOC_SS_SMS,           IG,   0,      IG,   IG,     IG,  0xE0,    5,
#ifndef GPRS
  ESTCS_MOC_SS_SMS,           IG,   1,      IG,   IG,     IG,  0x10,    4
#else
  ESTCS_MOC_SS_SMS,           IG,   1,      IG,   IG,     IG,  0x10,    4,
  ESTCS_GPRS_1P,              IG,  IG,      IG,   IG,     IG,  0x78,    3,
  ESTCS_GPRS_SB,              IG,  IG,      IG,   IG,     IG,  0x70,    3
#endif
};

static void dat_get_background_and_bits (UBYTE     *background,
                                         UBYTE     *bits)
{
  GET_INSTANCE_DATA;

  UBYTE  i;
  USHORT ms_capability;

  TRACE_FUNCTION ("dat_get_background_and_bits()");

  /*
   * initialise the output parameter
   */
  *background   = 0;
  *bits         = 5;
  ms_capability = SIGN;

  /*
   * check the support of vocoder
   */
  if (FldGet(rr_data->mscap.chnMode, VocSup))
  {
    ms_capability = (FldGet (rr_data->mscap.chnMode, hrSup)) ? DUAL : FULL;
  }

  /*
   * go through the table until all criterions are passed (or can be ignored).
   */
  for (i = 0; i < TABLE_SIZE; i++)
  {
    /*
     * check internal establishment cause
     */
    if (rr_data->ms_data.establish_cause NEQ coding_table[i].cause)
      continue;

    /*
     * check last used channel if applicable
     */
    if (coding_table[i].last_channel NEQ IG)
    {
      if (rr_data->ms_data.last_used_channel NEQ
          coding_table[i].last_channel)
        continue;
    }

    /*
     * check new establishment cause indication flag if applicable
     */
    if (coding_table[i].neci_flag NEQ IG)
    {
      if (rr_data->nc_data[SC_INDEX].select_para.neci NEQ
          coding_table[i].neci_flag)
        continue;
    }

    /*
     * check the channel needed indication of the paging messages
     * if applicable
     */
    if (coding_table[i].channel_needed NEQ IG)
    {
      if (rr_data->ms_data.channel_needed NEQ
          coding_table[i].channel_needed)
        continue;
    }

    /*
     * check the MS capability if applicable
     */
    if (coding_table[i].ms_capability NEQ IG)
    {
      if (ms_capability NEQ coding_table[i].ms_capability)
        continue;
    }

    /*
     * check the TCH Fullrate needed flag is applicable.
     */
    if (coding_table[i].tch_f_needed NEQ IG)
    {
      if ((rr_data->ms_data.establish_cause NEQ ESTCS_MOC_SPEECH) AND
          (rr_data->ms_data.establish_cause NEQ ESTCS_MOC_DATA_HR_SUFF))
        continue;
    }

    /*
     * add this point all criterions are passed, so use the table contents.
     */
    *background = coding_table[i].background;
    *bits       = coding_table[i].bits;
    return;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_get_burst              |
+--------------------------------------------------------------------+

  PURPOSE : This functions adds a random value to the channel request
            content. The random value is calculated from a well distributed
            table.

*/
static const UBYTE random_values[32] = {  9, 27, 17,  6, 10, 15,  2, 23,
                                         29, 14,  4, 26, 18,  0, 31, 13,
                                         21,  1, 30, 22,  5, 24, 20,  8,
                                          7, 28, 16, 11, 25, 12,  3, 19
                                        };
static const UBYTE mask[]            = { 3, 7, 15, 31 };

static UBYTE dat_get_burst (UBYTE     background,
                            UBYTE     bits)
{
  GET_INSTANCE_DATA;
  UBYTE random_value;

  TRACE_FUNCTION ("dat_get_burst()");

  rr_data->ms_data.index++;
  rr_data->ms_data.index %= 32;
  random_value = random_values[rr_data->ms_data.index];

#ifdef GPRS
  if(
      (rr_data->ms_data.establish_cause EQ ESTCS_GPRS_1P)
        AND
      ((random_value & mask[bits - 2]) EQ 7)
    )
  {
    return (background + 6);
  }
#endif

  return (background + (random_value & mask[bits - 2]));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_get_delta              |
+--------------------------------------------------------------------+

  PURPOSE : Reference GSM 4.08, chapter 3.3.1.1.2 Initiation of the
            immediate assignment procedure

            The RR entity of the mobile station initiates the immediate
            assignment procedure by scheduling the sending on the
            RACH and leaving idle mode.

            It then sends maximally M + 1 CHANNEL REQUEST messages on
            the RACH in a way such that:

            - the number of slots belonging to the mobile station's RACH
            between initiation of the immediate assignment procedure and
            the first CHANNEL REQUEST message (excluding the slot containing
            the message itself) is a random value drawn randomly for each
            new initial assignment initiation with uniform probability
            distribution in the set {0, 1, ..., max (T,8) - 1};

            - the number of slots belonging to the mobile station's RACH
            between two successive CHANNEL REQUEST messages (excluding
            the slots containing the messages themselves) is a random value
            drawn randomly for each new transmission with uniform probability
            distribution in the set {S, S + 1, ..., S + T - 1};

            Here, T is the value of the parameter "Tx-integer" broadcast on the BCCH;

            M is the value of the parameter "max retrans" broadcast on the BCCH;

            S is a parameter depending on the CCCH configuration and on the value
            of Tx-integer as defined in table 3.1/GSM 04.08.

            Table 3.1/GSM 04.08: Values of parameter S

            +-----------------------------------------------------+
            + TX-integer + non combined CCCH + combined CCH/SDCCH +
            +------------+-------------------+--------------------+
            + 3,8,14,50  +         55        +          41        +
            + 4,9,16     +         76        +          52        +
            + 5,10,20    +        109        +          58        +
            + 6,11,25    +        163        +          86        +
            + 7,12,32    +        217        +         115        +
            +-----------------------------------------------------+

*/
static const CONVERT_TXINTEGER  convert_table[MAX_TX_INTEGER] =
  /*
   *      T   S non combined   S combined Tx-integer
   */
    {
          3,  55,              41,        /*  0 */
          4,  76,              52,        /*  1 */
          5, 109,              58,        /*  2 */
          6, 163,              86,        /*  3 */
          7, 217,             115,        /*  4 */
          8,  55,              41,        /*  5 */
          9,  76,              52,        /*  6 */
         10, 109,              58,        /*  7 */
         11, 163,              86,        /*  8 */
         12, 217,             115,        /*  9 */
         14,  55,              41,        /* 10 */
         16,  76,              52,        /* 11 */
         20, 109,              58,        /* 12 */
         25, 163,              86,        /* 13 */
         32, 217,             115,        /* 14 */
         50,  55,              41         /* 15 */
    };

static UBYTE dat_get_delta (UBYTE i)
{
  GET_INSTANCE_DATA;

  USHORT index = rr_data->nc_data[SC_INDEX].rach.tx_integer;
  USHORT n;
  USHORT result=0;

  TRACE_FUNCTION ("dat_get_delta()");

  /*
   * calculate righ boarder of the interval.
   */
  TRACE_ASSERT( index < MAX_TX_INTEGER );
  if( index < MAX_TX_INTEGER)
  {
    n = (i EQ 0) ? att_max (convert_table[index].t, 8) - 1
               : convert_table[index].t - 1;

    /*
     * calculate random value
     */
    result = dat_random ((USHORT)(n+1));

    /*
     * If it is not the first value, add left boarder S
     */
    if (i NEQ 0)
    {
      /*
       * in according to GSM 4.08, section 10.5.2.11 (Control Channel Description)
       * and section 3.3.1.1.2 (Initiation of the immediate assignment procedure)
       */
      result +=
        (rr_data->nc_data[SC_INDEX].control_descr.ccch_conf EQ COMB_CCCH_COMB)
          ? convert_table[index].s_combined
          : convert_table[index].s_non_combined;
    }
  }
  return (UBYTE)result;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_get_ncell_pos          |
+--------------------------------------------------------------------+

  PURPOSE : for uplink SACCH RR MEASUREMENT REPORT message the
            position of a cell inside the neighbourcell list must
            be calculated. This is done by this function.

*/

static UBYTE dat_get_ncell_pos (USHORT channel)
{
  GET_INSTANCE_DATA;
  UBYTE i;

  TRACE_FUNCTION ("dat_get_ncell_pos()");

  /*
   * the loop counter i is the position inside the actual
   * neighbourcell list.
   */
  for (i=0; i<MAX_NEIGHBOURCELLS;i++)
  {
    if (channel EQ rr_data->act_ncell_list[i])
    {
      /*
       * channel is found, then return the position.
       */
      return i;
    }
  }
  return 0;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_eplmn_equal_req        |
+--------------------------------------------------------------------+

  PURPOSE : Compare found PLMN with all PLMNs in EPLMN list 


*/

static BOOL dat_eplmn_equal_req(const UBYTE *mcc, const UBYTE *mnc)
{
  GET_INSTANCE_DATA;
  UBYTE i;

  TRACE_FUNCTION("dat_eplmn_equal_req");

  if(rr_data->ms_data.v_eq_plmn)
  {
    for(i = 0; i < RR_EPLMNLIST_SIZE; i++)
    {
      if(!memcmp(rr_data->ms_data.eq_plmn_list[i].mcc, mcc, SIZE_MCC) &&
         !memcmp(rr_data->ms_data.eq_plmn_list[i].mnc, mnc, SIZE_MNC))
        return TRUE;
    }
  }
  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_plmn_equal_req         |
+--------------------------------------------------------------------+

  PURPOSE : The function checks whether the given mobile country code
            and the given mobile network code describes the requested
            PLMN, eg. the HPLMN distilled from the IMSI.
            This is not exactly the algorithm as shown for HPLMN
            matching as shown in 03.22 Normative Annex A, this version
            here is more universal.

*/

GLOBAL BOOL dat_plmn_equal_req (const UBYTE *bcch_mcc, const UBYTE *bcch_mnc,
                                const UBYTE *mm_mcc,  const UBYTE *mm_mnc)
{
  GET_INSTANCE_DATA;
  if(((rr_data->ms_data.operation_mode >> SHIFT_FOR_SEARCH_OFFSET) & 1) EQ M_AUTO &&
     rr_data->ms_data.req_mm_service NEQ FUNC_LIM_SERV_ST_SRCH)    
  {
    /* Check for equivalent EPLMNs */
    if(dat_eplmn_equal_req(bcch_mcc, bcch_mnc))
      return TRUE;
  }

  /* Check MCC */
  if (memcmp (mm_mcc, bcch_mcc, SIZE_MCC) NEQ 0)
    return FALSE;

  /* Check first 2 MNC digits */
  if (memcmp (mm_mnc, bcch_mnc, 2) NEQ 0)
    return FALSE;

  /* Check for full match */
  if (mm_mnc[2] EQ bcch_mnc[2])
    return TRUE;

  /* The 3rd digit of the MNC differs */
  if ((bcch_mcc[0] EQ 3) AND
      (bcch_mcc[1] EQ 1) AND
      INRANGE(0,bcch_mcc[2],6))
  {
    /*
     * The MCC is in the range 310..316, this means North America.
     * The zero suffix rule applies.
     */
    return (((mm_mnc[2] EQ 0xf) AND (bcch_mnc[2] EQ 0x0)) OR
            ((mm_mnc[2] EQ 0x0) AND (bcch_mnc[2] EQ 0xf)));
  }
  return (bcch_mnc[2] EQ 0xf);
}

GLOBAL BOOL dat_hplmn (const UBYTE *mcc, const UBYTE *mnc)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_hplmn()");

  /*
   * only if a SIM is inserted and an IMSI is available
   */
  if (!rr_data->ms_data.imsi_available)
    return FALSE;

  if ((rr_data->ms_data.ahplmn.v_plmn EQ V_PLMN_PRES) AND
      (rr_data->ms_data.req_mm_service EQ FUNC_PLMN_SRCH))
  {
    /*
     * If AHPLMN is available
     * do not compare the mcc, mnc from IMSI for the HPLMN
     * use the AHPLMN
     */
    return dat_plmn_equal_req (mcc, mnc,
                               rr_data->ms_data.ahplmn.mcc,
                               rr_data->ms_data.ahplmn.mnc);
  }

  return dat_plmn_equal_req (mcc, mnc,
                             &rr_data->ms_data.imsi.ident_dig[0],
                             &rr_data->ms_data.imsi.ident_dig[3]);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_hplmn_country          |
+--------------------------------------------------------------------+

  PURPOSE : The function checks whether the given mobile country
            code is equal to the first three digits of the IMSI.
            The first three digits of the IMSI are equal to the
            mobile country code of the HPLMN.

*/

static BOOL dat_hplmn_country (const UBYTE *mcc)
{
  GET_INSTANCE_DATA;
  SHORT i;
  UBYTE mcc_digit;

  TRACE_FUNCTION ("dat_hplmn_country()");

  /*
   * Only if an IMSI is available
   */
  if (rr_data->ms_data.imsi_available)
  {
/*    EM_HPLMN_SEARCH_STARTED; - Not supported*/
    for (i = 0; i < SIZE_MCC; i++)
    {
      
      if((rr_data->ms_data.ahplmn.v_plmn EQ V_PLMN_PRES) AND
         (rr_data->ms_data.req_mm_service EQ FUNC_PLMN_SRCH))
      {
        /*
         * If AHPLMN is available
         * do not compare the mcc from IMSI for the HPLMN
         * use the MM req HPLMN
         */
        mcc_digit = rr_data->ms_data.ahplmn.mcc[i];
      }      
      else
      {
        mcc_digit = rr_data->ms_data.imsi.ident_dig[i];
      }

      /*
       * if one of the three first digits of the IMSI is unequal to
       * the mobile country code, it is not the HPLMN country.
       */
      if (mcc[i] NEQ mcc_digit)
      {
        return FALSE;
      }
    }
    
    EM_HPLMN_SEARCH_PASSED;
    
    return TRUE;
  }
  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_imsi_mod_1000          |
+--------------------------------------------------------------------+

  PURPOSE : Calculate IMSI modulo 1000. That means take the last three
            digits of the IMSI. The length of the IMSI (and so the number
            of digits overall) is variable.

            This value is needed for calculation of the paging group.

*/

static SHORT dat_imsi_mod_1000 (void)
{
  GET_INSTANCE_DATA;
  SHORT i      = 0;
  SHORT ret    = 0;
  int   n;

  TRACE_FUNCTION ("dat_imsi_mod_1000()");

  while (rr_data->ms_data.imsi.ident_dig[i] < 0x0A)
    i++;

  if (i)
  {
    for (n = MAXIMUM (i-3, 0); n <= MAXIMUM (i-1, 0); n++)
    {
      ret = ret * 10 + rr_data->ms_data.imsi.ident_dig[n];
    }
  }
  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_no_of_paging_blocks    |
+--------------------------------------------------------------------+

  PURPOSE : The function calculates the number of paging blocks.

            For 51-Multiframe the following condition exists:

            A non-combined CCCH configuration has 9 CCCH Blocks.
            A combined CCCH configuration has 3 CCCH Blocks.

            From this n CCCH Blocks BS_AG_BLKS_RES blocks are reserved
            for AGCH (access grant channels for immediate assignment).
            The rest is reserved for PCH (Paging Channel).

            This number must be multiplied by the BS_PA_MFRMS parameter
            (internally stored is the air-interface coding, add 2 for
            the real value).

            The BS_PA_MFRMS parameter defines the number of 51-Multiframes
            until the paging blocks are repeated.

*/

static SHORT dat_no_of_paging_blocks (UBYTE     index)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_no_of_paging_blocks()");

  /* in according to GSM 4.08 section 10.5.2.11, table 10.5.33 */
  if (rr_data->nc_data[index].control_descr.ccch_conf EQ COMB_CCCH_COMB)
  {
    /*
     * combined CCCH,
     *
     * number of paging blocks = (3 - BS_AG_BLKS_RES) * BS_PA_MFRMS
     *
     * Maximum function only for security reasons, BCCH coding range is 0..7,
     * but allowed is only 0..2.
     */
    return ((att_max (1, (UBYTE)(3 - rr_data->nc_data[index].control_descr.
                                                bs_ag_blks_res))) *
            ((UBYTE)(2 + rr_data->nc_data[index].control_descr.bs_pa_mfrms)));
  }
  else
  {
    /*
     * non-combined CCCH,
     *
     * number of paging blocks = (9 - BS_AG_BLKS_RES) * BS_PA_MFRMS
     */
    return ((9 - rr_data->nc_data[index].control_descr.bs_ag_blks_res) *
            (2 + rr_data->nc_data[index].control_descr.bs_pa_mfrms));
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_owner_of_auth_0_to_9   |
+--------------------------------------------------------------------+

  PURPOSE : It is checked whether the MS is owner of at least one
            of the normal classes 0 to 9. The coding on the SIM card
            is in opposite to the coding on the BCCH.

*/

static BOOL dat_owner_of_auth_0_to_9 (void)
{
  GET_INSTANCE_DATA;
  BOOL  result            = FALSE;

  /*
   * get bits 0 to 9 from SIM card and BCCH
   */
  USHORT ms_classes = rr_data->ms_data.access_classes & 0x3FF;
  USHORT plmn_classes = rr_data->nc_data[SC_INDEX].rach.ac & 0x3FF;

  TRACE_FUNCTION ("dat_owner_of_auth_0_to_9()");

  /*
   * check only if IMSI is available
   * else no valid classes of the SIM card available
   */
  if (rr_data->ms_data.imsi_available)
  {
    /*
     * check classes
     */
    if ((ms_classes & (~plmn_classes)) NEQ 0)
      result = TRUE;
  }
  return result;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_owner_of_auth_11_to_15 |
+--------------------------------------------------------------------+

PURPOSE : It is checked whether the MS is owner of at least one
          of the normal classes 11 to 15. The coding on the SIM card
          is in opposite to the coding on the BCCH.

*/

static BOOL dat_owner_of_auth_11_to_15 (void)
{
  GET_INSTANCE_DATA;
  BOOL  result            = FALSE;

  /*
   * get bits 11 to 15 from the SIM card and the BCCH.
   */
  USHORT ms_classes   = rr_data->ms_data.access_classes & 0xF800;
  USHORT plmn_classes = rr_data->nc_data[SC_INDEX].rach.ac & 0xF800;

  TRACE_FUNCTION ("dat_owner_of_auth_11_to_15()");

  /*
   * check only if IMSI is available
   * else no valid classes of the SIM card available
   */
  if (rr_data->ms_data.imsi_available)
  {
    if (! dat_hplmn (rr_data->nc_data[SC_INDEX].lai.mcc, 
                     rr_data->nc_data[SC_INDEX].lai.mnc))
    {
      /*
       * ignore bit 11 and 15 if not in the HPLMN
       */
      ms_classes   = ms_classes & 0x7000;
      plmn_classes = plmn_classes & 0x7000;
    }

    if (! dat_hplmn_country (rr_data->nc_data[SC_INDEX].lai.mcc))
    {
      /*
       * ignore bit 12 to 14 if not in the HPLMN country
       */
      ms_classes   = ms_classes & 0x8F00;
      plmn_classes = plmn_classes & 0x8F00;
    }

    /*
     * check classes
     */
    if ((ms_classes & (~plmn_classes)) NEQ 0)
      result = TRUE;
  }

  return result;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_random                 |
+--------------------------------------------------------------------+

  PURPOSE : The function calculates a random value in the range 0..n-1.
            The algorithm uses the system time as a base for the random
            value calculation.

*/

GLOBAL USHORT dat_random (USHORT n)
{
  GET_INSTANCE_DATA;
  T_TIME time_val;
  static USHORT random_value = 0;

  TRACE_FUNCTION ("dat_random()");

  /*
   * for module testing the random component can be switched off
   */
  if (rr_data->dyn_config.no_sys_time)
    time_val = 0;
  else
    vsi_t_time (VSI_CALLER &time_val);

  /*
   * increment the base of the calculation by the system time.
   */
  random_value += (USHORT) time_val;

  /*
   * calculate the value in the range 0...n.1
   */
  return (random_value % n);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_release_connection     |
+--------------------------------------------------------------------+

  PURPOSE : The function is called after receiving MPH_STOP_DEDICATED_CNF
            from L1 following disconnection of L2 connection.

*/

GLOBAL void dat_release_connection (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_release_connection()");

  switch (GET_STATE (STATE_DAT))
  {
    case DAT_IMM_ASS:
      /* 
       * Layer 2 disconnection is initiated by MM through RR_ABORT_REQ 
       */
      if(rr_data->rel_cause EQ RRCS_MM_ABORTED)
      {
        dat_rr_release_ind(RRCS_MM_ABORTED, SAPI_0);
      }
      break;

    case DAT_IMM_ASS_1:
      /* 
       * Layer 2 disconnection is initiated by MM through RR_ABORT_REQ 
       */
      if(rr_data->rel_cause EQ RRCS_MM_ABORTED)
      {
        dat_rr_release_ind(RRCS_MM_ABORTED, SAPI_0);
        break;
      }
      /* 
       * DL establishment failure during immediate assignment
       */
      switch (rr_data->ms_data.establish_cause)
      {
#ifdef GPRS
        case ESTCS_GPRS_PAGING:
          dat_stop_dcch_ind ((UBYTE)rr_data->dcch_stop_cause);
          break;
        case ESTCS_PAGING:
          if(!rr_data->repeat_est)
            dat_rr_release_ind(rr_data->rel_cause, SAPI_0);
#else
        case ESTCS_PAGING:
#endif
          break;
        default:
          dat_rr_release_ind(rr_data->rel_cause, SAPI_0);
          break;
      }
      break;

    default:
      dat_rr_release_ind (rr_data->rel_cause, SAPI_0);
      break;
  }

  /*
   * clear state of data transfer process and start
   * cell reselection to come back to idle mode.
   * Inform GRR, and wait for CR_RSP 
   */
  att_leave_dedicated();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_send_random_bursts     |
+--------------------------------------------------------------------+

  PURPOSE : The function configures layer 1 to send 2 to 8 random
            bursts during connection establishment

*/

/*
 * conversion of the air-interface coding (0..3) for the
 * number of random bursts to be send
 */
static const UBYTE max_attempt[MAX_RACH_RETRANS_VAL] = { 2, 3, 5, MAX_RACH_REQ};

static void dat_send_random_bursts (void)
{
  GET_INSTANCE_DATA;
  UBYTE i;
  UBYTE background;
  UBYTE bits;

  PALLOC (mph_random_access_req, MPH_RANDOM_ACCESS_REQ);

  TRACE_FUNCTION ("dat_send_random_bursts()");

  /*
   * calculate the background and the number of bits for the
   * random part depending on the establishment cause.
   */
  dat_get_background_and_bits (&background, &bits);
  TRACE_EVENT("reset rej_rec");
  rr_data->imm_ass_rej_rec = FALSE;
  /*
   * initialize the primitive and parameter for calculation
   */
  rr_data->ms_data.access_counter  = 0;
  TRACE_ASSERT( rr_data->nc_data[SC_INDEX].rach.max_retrans < 
                                     MAX_RACH_RETRANS_VAL );
  rr_data->ms_data.max_attempt     = max_attempt[rr_data->nc_data
                                     [SC_INDEX].rach.max_retrans];
  memset (&mph_random_access_req->send_mode, 0, sizeof (T_send_mode));
  mph_random_access_req->send_mode.no = rr_data->ms_data.max_attempt;

  /*
   * for all random bursts
   */
  TRACE_ASSERT( rr_data->ms_data.max_attempt <= MAX_RACH_REQ );
  for (i = 0; i < rr_data->ms_data.max_attempt; i++)
  {
    /*
     * calculate time until the random burst must be send
     */
    mph_random_access_req->send_mode.delta[i] = dat_get_delta (i);

    /*
     * calculate the random burst content and store it for
     * later comparision with the incoming immediate assignment
     * messages.
     */
    mph_random_access_req->send_mode.rach[i] = rr_data->used_channel_ref[i] =
        dat_get_burst (background, bits);
    /*
    TRACE_EVENT_P3 ("RA %u: ref=0x%02x delta=%u",
      i, rr_data->used_channel_ref[i],
      mph_random_access_req->send_mode.delta[i]);
    */
  }

  /*
   * configure layer 1.
   */
  PSENDX (PL, mph_random_access_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_set_last_used_channel  |
+--------------------------------------------------------------------+

  PURPOSE : The last used channel during connection must be stored
            for the case of call re-establishment.

*/

GLOBAL void dat_set_last_used_channel (T_chan_desc *chan_desc)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_set_last_used_channel()");

  /*
   * depending on the air-interface coding for the channel type
   */
  switch (chan_desc->chan_type)
  {
    case CH_TCH_F:
      /*
       * Traffic channel full rate
       */
      rr_data->ms_data.last_used_channel = TCHFCH;
      break;

    case CH_TCH_H_1:
    case CH_TCH_H_2:
      /*
       * Traffic channel half rate
       */
      rr_data->ms_data.last_used_channel = TCHHCH;
      break;

    default:
      /*
       * SDCCH
       */
      rr_data->ms_data.last_used_channel = SDCCHCH;
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_start_immediate_assign |
+--------------------------------------------------------------------+

  PURPOSE : start of the immediate assignment procedure after getting
            a RR_ESTABLISH_REQ from MM for a mobile originated connection
            or a MPH_PAGING_IND from layer 1 for a mobile terminated
            connection.

*/

GLOBAL void dat_start_immediate_assign (USHORT    cause)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_start_immediate_assign()");

  /*
   * synchronize attachment process to connection establishment state
   */
  att_dat_con_est ();

  /*
   * initialize parameters for random access procedure
   */
  rr_data->ms_data.index             = dat_random (32);
  rr_data->ms_data.establish_cause   = cause;
  rr_data->ms_data.access_counter    = 0;
  rr_data->ms_data.all_conf_received = FALSE;

  /*
   * if connection is too short to get one measurement report from layer 1,
   * initialize the structure with the previous idle value
   */
  rr_data->ms_data.measurement_report.rx_lev_full = rr_data->nc_data[SC_INDEX].rxlev;

  TRACE_EVENT_P1 ("imm ass SC=[%u]",rr_data->nc_data[SC_INDEX].arfcn);

  /*
   * send random bursts and wait for immediate assignment
   */
  dat_send_random_bursts ();
  SET_STATE (STATE_DAT, DAT_IMM_ASS);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_start_sabm             |
+--------------------------------------------------------------------+

  PURPOSE : After successful immediate assignment procedure the layer 1
            is configured on a dedicated control channel (SDCCH or FACCH).
            Now the layer 2 connection must be established. This is done
            by this function.

            Layer 2 sends a Paging response in case of mobile terminated
            connection piggy-backed on the SABM.

            In case of mobile terminated connection the piggy-backed layer 3
            message is coming from MM.

*/

GLOBAL void dat_start_sabm (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_start_sabm()");

  if (rr_data->ms_data.establish_cause EQ ESTCS_PAGING
#ifdef GPRS
      OR rr_data->ms_data.establish_cause EQ ESTCS_GPRS_PAGING
#endif
     )
  {
    MCAST (pag_res, U_PAG_RES);
    PALLOC_MSG (establish_req, DL_ESTABLISH_REQ, U_PAG_RES);

    dat_code_prr_channel (&establish_req->ch_type,
                          &establish_req->sapi,
                          rr_data->sc_data.chan_desc.chan_type);
    /*
     * if it is a mobile terminated connection,
     * fill the C-Structure for a PAGING RESPONSE message.
     */
    pag_res->msg_type               = U_PAG_RES;
    pag_res->ciph_key_num.key_seq   = rr_data->ms_data.cksn;
    pag_res->mob_class_2            = rr_data->ms_data.classmark2;
    pag_res->mob_class_2.rf_pow_cap = att_get_power ();

    /*
     * fill mobile identity and send it to layer 2
     */
    dat_fill_mobile_identity (ESTCS_PAGING, &pag_res->mob_ident);
    for_dat_est_req_content  (establish_req);
  }
  else
  {
    /*
     * mobile originated connection. set sdu with message from MM
     * and send it to layer 2.
     */
    PALLOC_SDU (establish_req, DL_ESTABLISH_REQ, ((MAX_L2_FRAME_SIZE * BITS_PER_BYTE) - ENCODE_OFFSET));

    dat_code_prr_channel (&establish_req->ch_type,
                          &establish_req->sapi,
                          rr_data->sc_data.chan_desc.chan_type);

    /*lint -e419 (Warning -- Apparent data overrun for function memcpy exceeds argument 1)*/
    establish_req->sdu.o_buf = rr_data->ms_data.l3msg.offset;
    establish_req->sdu.l_buf = rr_data->ms_data.l3msg.length; 
    memcpy (establish_req->sdu.buf, rr_data->ms_data.l3msg.buffer, MAX_L2_FRAME_SIZE);
    /*lint +e419 (Warning -- Apparent data overrun for function memcpy exceeds argument 1)*/
    for_dat_est_req_not_coding (establish_req);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_test_sim_available     |
+--------------------------------------------------------------------+

  PURPOSE : The function checks whether a test SIM is available or not.

*/

GLOBAL BOOL dat_test_sim_available (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_test_sim_available()");

#if defined (_SIMULATION_)
  /*
   * some traces for debugging.
   */
  if (rr_data->ms_data.operation_mode & 0x80)
  {
      TRACE_FUNCTION ("Test SIM available");
  }
  else
  {
      TRACE_FUNCTION ("No Test SIM available");
  }
#endif

  return ((rr_data->ms_data.operation_mode >> SHIFT_FOR_SIM_TYPE) & 1);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_check_sim_available    |
+--------------------------------------------------------------------+

  PURPOSE : The function checks whether the SIM is available or not.

*/

GLOBAL BOOL dat_check_sim_available (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_check_sim_available()");

  return ((rr_data->ms_data.operation_mode >> SHIFT_FOR_SIM_INSERTED) & 1);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_vsd_bit_set            |
+--------------------------------------------------------------------+

  PURPOSE : To set the send state variable V(S) and the
            corresponding bit for SAPI 0 messages on FACCH or SDCCH.
            The following possibilities are available:

            SET_ONLY:      V(S) in incremented and the message bit is set
            RESET_ONLY:    V(S) is initialized and the message bit is not set
            SET_AND_RESET: V(S) is initialized and the message bit is set.

*/

GLOBAL void dat_vsd_bit_set (T_L3_SDU  *m_buf,
                            UBYTE     action)
{
  static UBYTE         vsd = 0;   /* Send state variable V(S)                    */
         UBYTE          pd;       /* protocol discriminator of the message       */
         UBYTE        * msp;      /* pointer to message type for setting the bit */
#ifdef REL99
         UBYTE         mscr = 0;     /* variable to hold msc release version*/
#endif
  TRACE_FUNCTION ("dat_vsd_bit_set()");

  /*
   * VSD shall be initialized
   */
  if (action NEQ SET_ONLY)
    vsd = 0;

  /*
   * message bit shall be set
   */
  if (action NEQ RESET_ONLY)
  {
    /*
     * calculate message type pointer and protocol discriminator
     */
    TRACE_ASSERT( (m_buf->offset >> 3) < L3_SDU_BUF_SIZE );
    msp = &m_buf->buffer[m_buf->offset >> 3];
    pd  = *msp++ & 0x0F;
#ifdef REL99
    /*
     * Get MSCR of the serving MSC
     */
    get_msc_release_version(&mscr);
    TRACE_EVENT_P1("mscr (MSC release) version : 0x%X", mscr);
#endif
    switch (pd)
    {
      case PD_SMS:
        /*
         * SMS on SDCCH is a SAPI 3 message and shall not be set.
         */
        break;
      case PD_CC:
      case PD_SS:
      case PD_MM:
#ifdef REL99
        if (mscr EQ MSCR_99)
        {           
          /* for MSC release R99 or above modulo 4 will be done.
           * SAPI 0 message: set bit 7th and 8th of MSG Type IE 
           * and increment V(S) modulo 4.
           */
          *msp |= (vsd++ << 6);
          vsd   = vsd & 0x03;
        }
        else
#endif
        {
          /* for MSC release R98 or old modulo 2 will be done.
           * SAPI 0 message: set bit if vsd is 1, else no effect
           * and increment V(S) modulo 2.
           */
          *msp |= (vsd++ << 6);
          vsd &= 1;
        }
    }
  }
}

/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)  MODULE  : RR_DAT                           |
| STATE   : code           ROUTINE : dat_begin_start_immediate_assign |
+---------------------------------------------------------------------+

  PURPOSE : initiate immediate assignment procedure by sending
            channel request

*/
GLOBAL void dat_begin_start_immediate_assign (UBYTE id_type, UBYTE chan_need)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_begin_start_immediate_assign()");

  rr_data->page_identity_type = id_type;
  rr_data->ms_data.channel_needed     = chan_need;
  dat_vsd_bit_set (&rr_data->ms_data.l3msg, RESET_ONLY);
  rr_data->sc_data.first_attempt = TRUE;
  rr_data->repeat_est    = FALSE;
  dat_start_immediate_assign (ESTCS_PAGING);
}

/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)  MODULE  : RR_DAT                           |
| STATE   : code           ROUTINE : dat_rr_release_ind               |
+---------------------------------------------------------------------+

  PURPOSE : send RR_RELEASE_IND to MM

*/
GLOBAL void dat_rr_release_ind (USHORT relcs, UBYTE sapi)
{
    PALLOC (rr_release_ind, RR_RELEASE_IND);

    rr_release_ind->cause = relcs;
    rr_release_ind->sapi  = sapi;
#ifdef GPRS
    dat_set_gprs_resump(rr_release_ind);
#endif

    PSENDX (MM, rr_release_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_TIM                     |
| STATE   : code                ROUTINE : dat_send_release_ind       |
+--------------------------------------------------------------------+

  PURPOSE : Send RR_REELASE_IND to MM during the Access Procedure

*/

GLOBAL void dat_send_release_ind (USHORT cause)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_send_release_ind()");

  switch (rr_data->ms_data.establish_cause)
  {
#ifdef GPRS
    case ESTCS_GPRS_1P:
    case ESTCS_GPRS_SB:
    case ESTCS_GPRS_PAGING:
      SET_STATE(STATE_GPRS, GPRS_PIM_BCCH);
      break;
    case ESTCS_PAGING:
      dat_rr_release_ind(RRCS_INT_NOT_PRESENT, SAPI_0);
      /*
       *   After sending a release_ind RR shall consider that GRR leaves SUSPENDED state
       */
      if(GPRS_SUSPENDED_BCCH EQ GET_STATE(STATE_GPRS))
      {
        SET_STATE(STATE_GPRS,GPRS_PIM_BCCH);
      }
      else if(GPRS_SUSPENDED_PBCCH EQ GET_STATE(STATE_GPRS))
      {
        SET_STATE(STATE_GPRS,GPRS_PIM_PBCCH);
      }
#else
    case ESTCS_PAGING:
#endif
      break;

    default:
#ifdef GPRS
      /*
       *   After sending a release_ind RR shall consider that GRR leaves SUSPENDED state
       */
      if(GPRS_SUSPENDED_BCCH EQ GET_STATE(STATE_GPRS))
      {
        SET_STATE(STATE_GPRS,GPRS_PIM_BCCH);
      }
      else if(GPRS_SUSPENDED_PBCCH EQ GET_STATE(STATE_GPRS))
      {
        SET_STATE(STATE_GPRS,GPRS_PIM_PBCCH);
      }
#endif
      if ( cause EQ RRCS_RND_ACC_FAIL )
      {
        if (! IS_TIMER_ACTIVE (T3122))
        {
          dat_rr_release_ind(RRCS_RND_ACC_FAIL, SAPI_0);
        }
        else
        {
          dat_rr_release_ind(RRCS_RND_ACC_DELAY, SAPI_0);
        }
      }
      else
      {
        dat_rr_release_ind(cause, SAPI_0);
      } 
      break;
  }
}

/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)  MODULE  : RR_DAT                           |
| STATE   : code           ROUTINE : dat_send_assign_fail_msg         |
+---------------------------------------------------------------------+

  PURPOSE : send ASSIGNMENT FAILURE to network.

*/
void dat_send_assign_fail_msg(UBYTE cause)
{
  GET_INSTANCE_DATA;
  MCAST (assign_fail, U_ASSIGN_FAIL);
  PALLOC_MSG (dl_reconnect_req, DL_RECONNECT_REQ, U_ASSIGN_FAIL);
  assign_fail->msg_type = U_ASSIGN_FAIL;
  assign_fail->rr_cause = cause;
  dat_code_prr_channel (&dl_reconnect_req->ch_type,
                        &dl_reconnect_req->sapi,
                        rr_data->sc_data.chan_desc.chan_type);

  /*
   * start reconnection in layer 2.
   */
  for_dat_reconnect_req (dl_reconnect_req);
}

/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)  MODULE  : RR_DAT                           |
| STATE   : code           ROUTINE : dat_send_rr_status_msg           |
+---------------------------------------------------------------------+

  PURPOSE : send RR STATUS to network

*/
void dat_send_rr_status_msg(UBYTE cause)
{
  GET_INSTANCE_DATA;

  MCAST (rr_status, B_RR_STATUS);
  PALLOC_MSG (dl_data_req, DL_DATA_REQ, B_RR_STATUS);

  /*
   * set channel type and SAPI
   */
  dat_code_prr_channel (&dl_data_req->ch_type,
                        &dl_data_req->sapi,
                        rr_data->sc_data.chan_desc.chan_type);

  rr_status->msg_type = B_RR_STATUS;
  rr_status->rr_cause = cause;

  EM_RR_STATUS_SEND;

  for_dat_data_req (dl_data_req);
}

/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)  MODULE  : RR_DAT                           |
| STATE   : code           ROUTINE : dat_send_handov_fail_msg         |
+---------------------------------------------------------------------+

  PURPOSE : send HANDOVER FAILURE to network

*/
void dat_send_handov_fail_msg(UBYTE cause)
{
  GET_INSTANCE_DATA;
  MCAST (handov_fail, U_HANDOV_FAIL);
  PALLOC_MSG (dl_reconnect_req, DL_RECONNECT_REQ, U_HANDOV_FAIL);

  /*
   * set channel type and sapi for the reconnection.
   */
  dat_code_prr_channel (&dl_reconnect_req->ch_type,
                        &dl_reconnect_req->sapi,
                        rr_data->sc_data.chan_desc.chan_type);

  handov_fail->rr_cause = cause;
  handov_fail->msg_type = U_HANDOV_FAIL;

  /*
   * reconnect layer 2 link.
   */
  for_dat_reconnect_req (dl_reconnect_req);
}

#if defined (REL99) && defined (TI_PS_FF_EMR)
/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)  MODULE  : RR_DAT                           |
| STATE   : code           ROUTINE : dat_update_emr_data         |
+---------------------------------------------------------------------+

  PURPOSE : Updates the enhanced measurement parameters, other than
  BSIC and reporting priority.

*/
GLOBAL void dat_update_emr_rep_para(T_emp *p_em, T_enh_para_struct *p_enh)
{  
  p_enh->scale_order = p_em->scale_ord;
  if (p_em->v_serv_band_rep EQ TRUE)
    p_enh->servingband_rep = p_em->serv_band_rep;
  if (p_em->v_mr EQ TRUE )
    p_enh->multiband_rep = p_em->mr;
    
  /* Update reporting thresholds and reporting offsets*/
  if (p_em->v_report_900 EQ TRUE)
  {
    p_enh->enh_rep_data[0].rep_offset = p_em->report_900.rep_offset_900;
    p_enh->enh_rep_data[0].rep_offset = p_em->report_900.th_rep_900;
  }
  if (p_em->v_report_1800 EQ TRUE)
  {
    p_enh->enh_rep_data[1].rep_offset = p_em->report_1800.rep_offset_1800;
    p_enh->enh_rep_data[1].rep_offset = p_em->report_1800.th_rep_1800;
  }
  if (p_em->v_report_400 EQ TRUE)
  {
    p_enh->enh_rep_data[2].rep_offset = p_em->report_400.rep_offset_400;
    p_enh->enh_rep_data[2].rep_offset = p_em->report_400.th_rep_400;
  }
  if (p_em->v_report_1900 EQ TRUE)
  {
    p_enh->enh_rep_data[3].rep_offset = p_em->report_1900.rep_offset_1900;
    p_enh->enh_rep_data[3].rep_offset = p_em->report_1900.th_rep_1900;
  }
  if (p_em->v_report_850 EQ TRUE)
  {
    p_enh->enh_rep_data[4].rep_offset = p_em->report_850.rep_offset_850;
    p_enh->enh_rep_data[4].rep_offset = p_em->report_850.th_rep_850;
  }
  return;
}

/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)  MODULE  : RR_DAT                           |
| STATE   : code           ROUTINE : dat_code_enh_measure_report         |
+---------------------------------------------------------------------+

  PURPOSE : Forms enhanced measurement report and sends it to DL as a
  message with short PD.
*/
GLOBAL void dat_code_enh_measure_report(T_MPH_MEASUREMENT_IND *report)
{
  GET_INSTANCE_DATA;
  UBYTE     size_avail = MSG_SIZE_EMR - MAND_SIZE_EMR;
  MCAST (meas_emr, U_EMR);
  PALLOC_MSG (dl_data, DL_SHORT_UNITDATA_REQ, U_EMR);
  
  TRACE_FUNCTION ("dat_code_enh_measure_report()");
  memset (&dl_data->sdu.buf[0], 0, dl_data->sdu.o_buf / BITS_PER_BYTE); 
  /* initialize C-structure for the Uplink message */
  memset (meas_emr, 0, sizeof (T_U_EMR));
  meas_emr->msg_type = U_EMR;
  meas_emr->sl2h     = SL2H_0;

  if (report->valid EQ TRUE)
  {
    T_enh_para_struct *src = &rr_data->sc_data.emr_data_current.enh_para;       
    
    /* Fill the non-measurement paramaters */    
    meas_emr->ba_ind = rr_data->sc_data.ba_index;
    meas_emr->scale = report->scale_used;
    /* Now fill serving cell data */
    if (report->rxlev_val NEQ 0)
    {
      meas_emr->v_scdata = TRUE;
      meas_emr->scdata.rxlev = report->rxlev_val;
      meas_emr->scdata.rxqual_full = report->rx_qual_full;
      meas_emr->scdata.mean_bep = report->mean_bep;
      meas_emr->scdata.cv_bep = report->cv_bep;
      meas_emr->scdata.nr_rcvd_bl = report->nbr_rcvd_blks;
      meas_emr->scdata.dtx_used = report->dtx;
      size_avail -= SC_INFO_SIZE_EMR; 
    }  
    
    /* Fill neighbour cell measurements */
    if (report->ncells.no_of_ncells > 0)
    {
      UBYTE         i;
      UBYTE         j;
      UBYTE         highest_index = 0;
      UBYTE         index_0 = 1;/*To decrement size by 1 when index is 0*/
      ULONG         bit_map=0;
      
      
      meas_emr->em_rep.c_rep_q_arr  = meas_emr->c_i_bsic_i = 0;
      for (i =0; i < report->ncells.no_of_ncells; i++ )
      {
        j = report->nc_index[i];
        /*Decide whether it is a valid BSIC cell or not*/
        if ( j NEQ NOT_PRESENT_8BIT )
        {
          /*This is an valid BSIC cell*/
          /*Check the priority of the cell*/
          if ( (src->rep_rate EQ REDUCED_REP_RATE) AND 
           (src->enh_cell_list[j].rep_priority EQ REP_PRIOR_NORM ))
          {
            /*Proceed further only if the cell has not been included-
              in any report till now or if it's the time to include this
              cell in the report*/
            if ( (rr_data->sc_data.rep_count[j] NEQ rr_data->sc_data.emr_count ) AND
                 (rr_data->sc_data.rep_count[j] NEQ NOT_PRESENT_8BIT ) )
            {
              bit_map |= (1<<i);
              continue; /* Include this cell at the end only if size is available*/
            }                 
          }
          meas_emr->em_rep.rep_q_arr[j].rep_q   = report->ncells.rx_lev[i];
          
          if ( j > highest_index ) 
          {
          /*When the place where the RXLEV has to be filled requires
          additional bits in bit map, then we have to account for
            these single bits and additional 6 bits for RXLEV*/
            if (size_avail >= (j - highest_index)+6 + index_0 )
            {
            /*This means we require atleast j-highest_index+6 bits in bit map to 
              include this rxlev*/
              /* 6 bits for RXLEV itself*/                
              size_avail = size_avail- (j-highest_index+6+index_0) ;
              highest_index = j;
              meas_emr->em_rep.c_rep_q_arr =  j+1; /*counter is index+1*/
              index_0 = 0;  
              meas_emr->em_rep.rep_q_arr[j].v_rep_q = TRUE;  
              rr_data->sc_data.rep_count[j] = rr_data->sc_data.emr_count;
            }             
          }
          else if (size_avail >= 6 + index_0)
          {              
            size_avail -= (6+index_0); /* size for bit map is already accounted for*/              
            meas_emr->em_rep.c_rep_q_arr = highest_index +1;
            index_0 = 0;   
            meas_emr->em_rep.rep_q_arr[j].v_rep_q = TRUE;  
            rr_data->sc_data.rep_count[j] = rr_data->sc_data.emr_count;
          }
        } /* if j NEQ NOT_PRESENT_8BIT  */
        else
        {
          /*This is a Invalid BSIC cell*/
          /*fill in invalid BSIC list since cell is not present in the neighbour cell
          list. Here the index that needs to be filled is index of the ARFCN in BA(list)*/
          if ( size_avail > NC_INVBSIC_EMR )
          {            
            meas_emr->i_bsic_i[meas_emr->c_i_bsic_i].ba_start_bsic = 
              dat_get_ncell_pos (report->ncells.arfcn[i]);
            meas_emr->i_bsic_i[meas_emr->c_i_bsic_i].bsic     = report->ncells.bsic[i] ;
            meas_emr->i_bsic_i[meas_emr->c_i_bsic_i++].rxlev  = report->ncells.rx_lev[i];
            size_avail -= NC_INVBSIC_EMR;            
          }   
        }
        if (size_avail < 6) /*no more cells can be included*/
          break;
      } /* for 'i'*/    

      /*All the low priority cells have to be filled in valid BSIC bmp reporting,
       if still there's size available*/
      i = 0;
      while ( (bit_map NEQ 0) AND (size_avail >= 6) )
      {
        if ( ((bit_map >> i ) & (NOT_PRESENT_32BIT)) EQ TRUE )
        {
          j = report->nc_index[i];
          meas_emr->em_rep.rep_q_arr[j].rep_q   = report->ncells.rx_lev[i];          
          if ( j > highest_index ) 
          {
            /*When the place where the RXLEV has to be filled requires
            additional bits in bit map, then we have to account for
            these single bits and additional 6 bits for RXLEV*/
            if (size_avail >= (j - highest_index)+6 + index_0 )
            {
            /*This means we require atleast j-highest_index+6 bits in bit map to 
              include this rxlev*/
              /* 6 bits for RXLEV itself*/                
              size_avail = size_avail- (j-highest_index+6+index_0) ;
              highest_index = j;
              meas_emr->em_rep.c_rep_q_arr =  j+1; /*counter is index+1*/
              index_0 = 0;  
              meas_emr->em_rep.rep_q_arr[j].v_rep_q = TRUE;  
              rr_data->sc_data.rep_count[j] = rr_data->sc_data.emr_count;
            }             
          }
          else if (size_avail >= 6 + index_0)
          {              
            size_avail -= (6+index_0); /* size for bit map is already accounted for*/              
            meas_emr->em_rep.c_rep_q_arr = highest_index +1;
            index_0 = 0;   
            meas_emr->em_rep.rep_q_arr[j].v_rep_q = TRUE;  
            rr_data->sc_data.rep_count[j] = rr_data->sc_data.emr_count;
          }
          else
            break;
          bit_map = bit_map &  ( ~ ((ULONG)( 1 << i))); /*reset the corresponding bit in bit map*/
        }
      }/*while bit_map*/
      if (meas_emr->em_rep.c_rep_q_arr > 0)
        meas_emr->v_em_rep = TRUE;
      if (meas_emr->c_i_bsic_i > 0)
      {
        meas_emr->v_i_bsic_i = TRUE;      
        meas_emr->bsic_seen = report->bsic_seen;
      }        
    } /* if 'report->ncells.no_of_ncells'*/     
    
#if defined (REL99) && defined (TI_PS_FF_EMR)
    /*Fill the report with '0's till the end of message or upto 96 cells  */    
    while ( (size_avail > 0) AND (meas_emr->em_rep.c_rep_q_arr < 96 ) )
    {
      /* There are cells in GSM NC list for which bit map has to be set to '0' 
         earlier memset takes care of this. we just need to set the counter appropriately*/
      meas_emr->em_rep.c_rep_q_arr++;
      size_avail--;
    }
#else
    if ( meas_emr->em_rep.c_rep_q_arr < src->num_valid_cells )
    {
      /* There are cells in GSM NC list for which bit map has to be set to '0' 
         earlier memset takes care of this. we just need to set the counter appropriately*/
      meas_emr->em_rep.c_rep_q_arr = src->num_valid_cells;      
    }
    dl_data->ccd_assist = meas_emr->em_rep.nnc = meas_emr->em_rep.c_rep_q_arr; 
#endif
    rr_data->sc_data.emr_count = (rr_data->sc_data.emr_count +1) & 0x03; /*MOD4 addition*/    
  }
  else
  {   
    /*
     * measurement report from layer 1 is invalid
     */
    TRACE_EVENT ("invalid meas_emr");  
    /*A dummy msg with all options as FALSE will go in this case, to network.
     may be useful to maintain periodicity*/
  }
  /* Send the message to DL */
  
  for_dat_spd_unitdata_req(dl_data);
}
#endif

#if defined (TI_PS_FF_RTD) AND defined (REL99)
/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)  MODULE  : RR_DAT                           |
| STATE   : code           ROUTINE : dat_update_rtd_data         |
+---------------------------------------------------------------------+

  PURPOSE : Updates the real time difference parameters received on MEAS_INFO message.
*/
GLOBAL void dat_update_rtd_data(T_D_MEAS_INF *p_mi,T_rr_enh_para *p_temp)
{
T_rtdd *rtdd_struct= &p_mi->rtdd;
dat_update_common_rtd_struct(rtdd_struct,p_temp);

} /* end dat_update_rtd_data()*/


/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)  MODULE  : RR_DAT                           |
| STATE   : code           ROUTINE : dat_update_common_rtd_struct     |
+---------------------------------------------------------------------+

  PURPOSE : Common function called from for_store_rtd_data() to store 
  rtd parameters received on si2 quater message and update the same on receiving 
  meas_info message via function dat_update_rtd_data().
*/

GLOBAL void dat_update_common_rtd_struct(T_rtdd *rtdd,T_rr_enh_para *p_temp)
{
UBYTE     i,j,rtd_index,max_rtd_values;
  
  if(rtdd->v_rtdd6 EQ TRUE)
  {
    if(rtdd->rtdd6.v_ba_start_rtd EQ TRUE)
      rtd_index = rtdd->rtdd6.ba_start_rtd;
    else
      rtd_index = RTD_DEFAULT_INDEX;
    
    if( rtd_index < MAX_NR_OF_NCELL)
    {
      p_temp->enh_para.enh_cell_list[rtd_index].v_rtd =  TRUE;
      max_rtd_values = rtdd->rtdd6.rtds6.c_rtd6 > MAX_NUM_OF_RTD_VALUES ?
                      MAX_NUM_OF_RTD_VALUES : rtdd->rtdd6.rtds6.c_rtd6;
      for(i = 0;i < max_rtd_values;i++)
        p_temp->enh_para.enh_cell_list[rtd_index].rtd[i] = rtdd->rtdd6.rtds6.rtd6[i];
      p_temp->enh_para.enh_cell_list[rtd_index].c_rtd= max_rtd_values;
    } /*if*/
    
    for(j = 0;j < rtdd->rtdd6.c_rtds6_add;j++)
    {
      rtd_index++;
      if( rtd_index < MAX_NR_OF_NCELL)
      {
        p_temp->enh_para.enh_cell_list[rtd_index].v_rtd = TRUE;
        max_rtd_values = rtdd->rtdd6.rtds6_add[j].c_rtd6 > MAX_NUM_OF_RTD_VALUES ?
                           MAX_NUM_OF_RTD_VALUES : rtdd->rtdd6.rtds6_add[j].c_rtd6;
        for(i = 0;i < max_rtd_values;i++)
          p_temp->enh_para.enh_cell_list[rtd_index].rtd[i] = rtdd->rtdd6.rtds6_add[j].rtd6[i];
        p_temp->enh_para.enh_cell_list[rtd_index].c_rtd= max_rtd_values;
      } /*if*/
    } /*for*/
  } /*if*/
  if(rtdd->v_rtdd12 EQ TRUE)
  {
    if(rtdd->rtdd12.v_ba_start_rtd EQ TRUE)
      rtd_index = rtdd->rtdd12.ba_start_rtd;
    else
      rtd_index = RTD_DEFAULT_INDEX;
    
    if( rtd_index < MAX_NR_OF_NCELL)
    {
      p_temp->enh_para.enh_cell_list[rtd_index].v_rtd = TRUE;
      max_rtd_values = rtdd->rtdd12.rtds12.c_rtd12 > MAX_NUM_OF_RTD_VALUES ?
                        MAX_NUM_OF_RTD_VALUES: rtdd->rtdd12.rtds12.c_rtd12;
      for(i = 0;i < max_rtd_values;i++)
      {
        p_temp->enh_para.enh_cell_list[rtd_index].rtd[i] =RTD_12BIT;
        p_temp->enh_para.enh_cell_list[rtd_index].rtd[i] |= rtdd->rtdd12.rtds12.rtd12[i];
      } /*for*/
      p_temp->enh_para.enh_cell_list[rtd_index].c_rtd= max_rtd_values;
    } /*if*/
    for(j = 0;j < rtdd->rtdd12.c_rtds12_add;j++)
    {
      rtd_index++;
      if(rtd_index < MAX_NR_OF_NCELL)
      {
        p_temp->enh_para.enh_cell_list[rtd_index].v_rtd = TRUE;
        max_rtd_values = rtdd->rtdd12.rtds12_add[j].c_rtd12 > MAX_NUM_OF_RTD_VALUES ?
                          MAX_NUM_OF_RTD_VALUES : rtdd->rtdd12.rtds12_add[j].c_rtd12;
        for(i = 0;i < max_rtd_values;i++)
        {
          p_temp->enh_para.enh_cell_list[rtd_index].rtd[i] = RTD_12BIT;
          p_temp->enh_para.enh_cell_list[rtd_index].rtd[i] |= rtdd->rtdd12.rtds12_add[j].rtd12[i];
        } /*for*/
        p_temp->enh_para.enh_cell_list[rtd_index].c_rtd= max_rtd_values;
      } /*if*/
    } /*for*/
  } /*if*/
} /* end of dat_update_common_rtd_struct() */

#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */

#endif
