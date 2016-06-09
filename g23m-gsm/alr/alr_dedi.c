/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  alr_DEDI
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
|  Purpose :  This Modul defines the SDL process Dedicated_Control.
+-----------------------------------------------------------------------------
*/

#ifndef ALR_DEDI_C
#define ALR_DEDI_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_PL

/*==== INCLUDES ===================================================*/
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "typedefs.h"
#include "pconst.cdg"
#include "mconst.cdg"
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

#include "alr.h"
#include "alr_em.h"

/*=== Constrains =======================================*/

#define TCH_F_INDEX 1
#ifdef FF_L23_A5_3
#define KLEN 8
#endif

/*==== EXPORT =====================================================*/
/*==== PRIVAT =====================================================*/
LOCAL UBYTE dedi_channel_mode (UBYTE channel_mode, UBYTE channel_type);
LOCAL void dedi_convert_chan_desc (T_channel_desc * channel_desc);
LOCAL void dedi_convert_frequency_list (T_frequency_list * frequency_list);
LOCAL UBYTE dedi_conversion_needed (void);
LOCAL void dedi_chan_ass_fail (void);
LOCAL void dedi_pdch_ass_fail (void);
LOCAL void dedi_cco_fail (void);
LOCAL void dedi_handover_fail (void);
LOCAL void dedi_async_handover (T_MPH_DEDICATED_REQ *dedicated_req);
LOCAL void dedi_chan_ass (T_MPH_DEDICATED_REQ *dedicated_req);
LOCAL void dedi_fill_ma (USHORT *ma, T_frequency_list *rf);
LOCAL void dedi_imm_ass (T_MPH_DEDICATED_REQ *dedicated_req);
LOCAL void dedi_pdch_ass (T_MPH_DEDICATED_REQ *dedicated_req);
LOCAL void dedi_pre_sync_handover (T_MPH_DEDICATED_REQ *dedicated_req);
LOCAL void dedi_sync_handover (T_MPH_DEDICATED_REQ *dedicated_req);
#if defined (REL99) && defined (FF_BHO)
LOCAL void dedi_reset_bho_data(void);
LOCAL void dedi_set_ho_type(ULONG *fn, ULONG *tn, UBYTE *ho_type);
#endif

LOCAL void dedi_chan_type_sel  (T_ch_type                 *ch_type,
                                T_chan_sel                *chan_sel,
                                T_frequency_list          *frequency_list);
LOCAL void dedi_chan_type_desc (T_MPH_DEDICATED_REQ       *dedicated_req, 
                                T_channel_desc_1_bef_sti  *channel_desc_1_bef_sti,
                                T_frequency_list          *frequency_list_bef_sti);
LOCAL void dedi_set_amr_conf   (T_amr_configuration       *amr_con, T_amr_conf *amr_conf);

/*==== VARIABLES ==================================================*/
LOCAL UBYTE actual_channel;

/*==== FUNCTIONS ==================================================*/

#if defined (WIN32)
#define TRACING
#endif

#if defined (TRACING)
#define ALR_TRACE_DEDI(a)  ALR_TRACE(a)
#else
#define ALR_TRACE_DEDI(a)
#endif

#if defined (TRACING)
#define ALR_TRACE_DEDI_DTX(a1)      TRACE_EVENT_P1 ("DTX = %d", a1);
#define ALR_TRACE_DEDI_PARA(h, rf, _ch, sub, _tno, _tsc, _ta, _st, _dtx, _maio)\
  {if (h){\
    TRACE_EVENT_P9 ("immass:hop[%04x] ch/s=%u/%u tno=%u tsc=%u ta=%u st=%u dtx=%u maio=%u",\
      rf, _ch, sub, _tno, _tsc, _ta, _st, _dtx, _maio);\
  }else{\
    TRACE_EVENT_P9 ("immass:[%u] ch/s=%u/%u tno=%u tsc=%u ta=%u st=%u dtx=%u maio=%u",\
      rf, _ch, sub, _tno, _tsc, _ta, _st, _dtx, _maio);}}
#define ALR_TRACE_DEDI_HO_1(p1,p2,p3,p4,p5,p6)  \
                             TRACE_EVENT_P6 ("d rf=%d bsic=%d sti=%d ch2=%d ch=%d tsc=%d",\
                               p1, p2, p3, p4, p5, p6)
#define ALR_TRACE_DEDI_HO_2(p1,p2,p3,p4,p5,p6) \
                             TRACE_EVENT_P6 ("d tn=%d arf=%d pow=%d ref=%d acc=%d nci=%d", \
                               p1, p2, p3, p4, p5, p6)
#define ALR_TRACE_DEDI_HO_3(p1,p2,p3,p4,p5,p6) \
                             TRACE_EVENT_P6 ("d mode=%d power=%d dtx=%d rlt=%d tav=%d pwrc=%d", \
                               p1, p2, p3, p4, p5, p6)
#define ALR_TRACE_DEDI_HO_4(p1,p2,p3)  \
                             TRACE_EVENT_P3 ("d mode=%d stat=%d algo=%d", p1, p2, p3);
#define ALR_TRACE_DEDI_HO_5(p1,p2,p3,p4,p5,p6) \
                             TRACE_EVENT_P6 ("a h=%d maio=0x%x hsn=%d chty=%d subch=%d tno=%d", \
                               p1, p2, p3, p4, p5, p6)
#define ALR_TRACE_DEDI_HO_6(p1,p2,p3,p4,p5,p6) \
                             TRACE_EVENT_P6 ("a tsc=%d txpwr=%d ho_acc=%d sti_p=%d ncc=%d bcc=%d", \
                               p1, p2, p3, p4, p5, p6)
#define ALR_TRACE_DEDI_HO_7(p1,p2,p3,p4) \
                             TRACE_EVENT_P4 ("a rf=%d chmo1=%d ciphmo=%d a5=%d", \
                               p1, p2, p3, p4)
#define ALR_TRACE_DEDI_FREQ_LIST() TRACE_EVENT_P2 ("HOP = %d %d", \
                               frequency_list->rf_chan_no.radio_freq[j], \
                               ARFCN_TO_L1 (frequency_list->rf_chan_no.radio_freq[j]))

#else

#define ALR_TRACE_DEDI_DTX(d)
#define ALR_TRACE_DEDI_PARA(hop,p1,p2,p3,p4,p5,p6,p7,p8,p9)
#define ALR_TRACE_DEDI_HO_1(p1,p2,p3,p4,p5,p6)
#define ALR_TRACE_DEDI_HO_2(p1,p2,p3,p4,p5,p6)
#define ALR_TRACE_DEDI_HO_3(p1,p2,p3,p4,p5,p6)
#define ALR_TRACE_DEDI_HO_4(p1,p2,p3)
#define ALR_TRACE_DEDI_HO_5(p1,p2,p3,p4,p5,p6)
#define ALR_TRACE_DEDI_HO_6(p1,p2,p3,p4,p5,p6)
#define ALR_TRACE_DEDI_HO_7(p1,p2,p3,p4)
#define ALR_TRACE_DEDI_FREQ_LIST()

#endif



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_DEDI                   |
| STATE   : code                ROUTINE : dedi_init                  |
+--------------------------------------------------------------------+

  PURPOSE : Initialize Dedicated Control Process.

*/

GLOBAL void dedi_init (void)
{
  GET_INSTANCE_DATA;
  alr_data->state[STATE_DEDI] = DEDI_INACTIVE;
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_DEDI                   |
| STATE   : code                ROUTINE : dedi_req                   |
+--------------------------------------------------------------------+

  PURPOSE : Process signal dedi_req from SDL process Main_Control.
            RR configures a dedicated channel.

*/

/*
 * Conversion Channel Type Air Interface to TI definition
 */

LOCAL const UBYTE CH_TYPE [16] =
  {
    0,           /* undefined        */
    1,           /* TCH fullrate     */
    2,           /* TCH halfrate (0) */
    2,           /* TCH halfrate (1) */
    3,           /* SDCCH/4 (0)      */
    3,           /* SDCCH/4 (1)      */
    3,           /* SDCCH/4 (2)      */
    3,           /* SDCCH/4 (3)      */
    4,           /* SDCCH/8 (0)      */
    4,           /* SDCCH/8 (1)      */
    4,           /* SDCCH/8 (2)      */
    4,           /* SDCCH/8 (3)      */
    4,           /* SDCCH/8 (4)      */
    4,           /* SDCCH/8 (5)      */
    4,           /* SDCCH/8 (6)      */
    4           /* SDCCH/8 (7)      */
  };


/*
 * Conversion Sub Channel Air Interface to TI definition
 */

LOCAL const UBYTE SUB_TYPE [16] =
  {
    0,           /* undefined        */
    0,           /* TCH fullrate     */
    0,           /* TCH halfrate (0) */
    1,           /* TCH halfrate (1) */
    0,           /* SDCCH/4 (0)      */
    1,           /* SDCCH/4 (1)      */
    2,           /* SDCCH/4 (2)      */
    3,           /* SDCCH/4 (3)      */
    0,           /* SDCCH/8 (0)      */
    1,           /* SDCCH/8 (1)      */
    2,           /* SDCCH/8 (2)      */
    3,           /* SDCCH/8 (3)      */
    4,           /* SDCCH/8 (4)      */
    5,           /* SDCCH/8 (5)      */
    6,           /* SDCCH/8 (6)      */
    7           /* SDCCH/8 (7)      */
  };

GLOBAL void dedi_req (T_MPH_DEDICATED_REQ * dedicated_req)
{
  GET_INSTANCE_DATA;
  /*
   * RR signals which procedure is processed
   */
  USHORT  mode = dedicated_req->mod;

  switch (mode)
  {
    case MODE_SYS_INFO_CHANGE:
      /*
       * Changed values for DTX, PWRC and Radio Link Timeout
       */
      ALR_TRACE_DEDI_DTX(dedicated_req->tr_para.dtx);

      nc_update_dedicated (dedicated_req->tr_para.dtx,
                           dedicated_req->tr_para.pwrc);
      alr_data->dedi_data.rlt = alr_data->dedi_data.act_rlt = (UBYTE)((dedicated_req->tr_para.rlt + 1) * 4);

      ALR_EM_SET_EM_ACT_RLT;

      break;
    case MODE_IMM_ASSIGN:
      /*
       * Immediate Assignment Procedure
       */
      dedi_imm_ass (dedicated_req);
      break;
    case MODE_CHAN_ASSIGN:
      /*
       * Intracell Handover
       */
      dedi_chan_ass (dedicated_req);
      /*
       * reset radio link timeout counter
       */
      alr_data->dedi_data.act_rlt = alr_data->dedi_data.rlt;

      ALR_EM_SET_EM_ACT_RLT;

      break;
 #ifdef GPRS
    case MODE_PDCH_ASSIGN:
      /*
       * Packet Data CHannel Assignment
       */
      dedi_pdch_ass (dedicated_req);
      break;
#endif
    case MODE_ASYNC_HANDOVER:
    case MODE_SYNC_HANDOVER:
    case MODE_PRE_SYNC_HANDOVER:
      {
#if !defined (REL99) || !defined (FF_BHO)
        ULONG dummy1, dummy2;
        if( nc_get_fn_time(dedicated_req->arfcn, &dummy1, &dummy2) EQ FALSE)
        {
          /*
           * Timing Information not present for the cell
           */
          ALR_TRACE_DEDI("Handover to a cell - whose SYNC is not done");
          ma_dedi_cnf(DEDI_RES_CELL_NOT_SYNC);
        }
        else
#endif
        {
          nc_suspend_handover();
          switch(mode)
          {
            case MODE_ASYNC_HANDOVER:
              /*
               * Asynchronous handover
               */
              dedi_async_handover (dedicated_req);
              break;
            case MODE_SYNC_HANDOVER:
              /*
               * Synchronous handover
               */
              dedi_sync_handover (dedicated_req);
              break;
            case MODE_PRE_SYNC_HANDOVER:
              /*
               * Pre-synchronous handover
               */
              dedi_pre_sync_handover (dedicated_req);
              break;
            default :
              break;
          }
          alr_data->dedi_data.act_rlt = alr_data->dedi_data.rlt;
          ALR_EM_SET_EM_ACT_RLT;
        }
      }
      break;
    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_DEDI                   |
| STATE   : code                ROUTINE : dedi_fail_req              |
+--------------------------------------------------------------------+

  PURPOSE : Process signal dedi_fail_req from SDL process Main_Control.
            RR signals that going back to the old channel after channel
            assignment or handover is necessary.

*/

GLOBAL void dedi_fail_req (void)
{
  GET_INSTANCE_DATA;
  /*
   * RR signals which procedure is running
   */
  switch (alr_data->dedi_data.act_mode)
  {
    case MODE_CHAN_ASSIGN:
      /*
       * channel assignment procedure
       */
      dedi_chan_ass_fail ();
      break;
    case MODE_PDCH_ASSIGN:
      dedi_pdch_ass_fail();
      break;
    case MODE_CELL_CHANGE_ORDER:
      dedi_cco_fail();
      break;
    case MODE_ASYNC_HANDOVER:
    case MODE_SYNC_HANDOVER:
    case MODE_PRE_SYNC_HANDOVER:
    case MODE_PSEUDO_SYNC_HANDOVER:
      /*
       * one of the handover procedures
       */
      dedi_handover_fail ();
      break;
    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_DEDI                   |
| STATE   : code                ROUTINE : dedi_ta_fail_ind           |
+--------------------------------------------------------------------+

  PURPOSE : Process signal dedi_ta_fail_ind from SDL process
            Main_Control.
            Layer 1 signals that the timing advance is out of
            range during handover. This will be forwarded to RR and
            switching back to the old channel is initiated by RR.

*/

GLOBAL void dedi_ta_fail_ind (void)
{
  ma_dedi_cnf (DEDI_RES_TA_OUT_OF_RANGE);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_DEDI                   |
| STATE   : code                ROUTINE : dedi_ho_finished           |
+--------------------------------------------------------------------+

  PURPOSE : Process signal dedi_ho_finished from SDL process
            Main_Control.
            Layer 1 signals that the handover has finished successful.
            This is forwarded to RR which starts resumption of the
            link on the new channel.

*/

GLOBAL void dedi_ho_finished (T_MPHC_HANDOVER_FINISHED* ho_finished)
{
  GET_INSTANCE_DATA;
  ma_clean_dedi_sys_buffer ();
  switch (ho_finished->cause)
  {
    case HO_COMPLETE: 
      ALR_TRACE_DEDI ("Handover to new cell complete");
#if defined (REL99) && defined (FF_BHO)
      if (alr_data->dedi_data.ho_type EQ HOT_BLIND)
      {
        T_NC* pcell;
        /*Add the new SC to BA if not present earlier*/
        if (alr_data->dedi_data.bho_cell_index EQ NOT_PRESENT_16BIT)
        {
          alr_data->dedi_data.bho_cell_index = 
          nc_find_serving_cell_entry(alr_data->serving_cell);
        }          
        pcell = &alr_data->nc_data.cell[alr_data->dedi_data.bho_cell_index];
        pcell->frame_offset = ho_finished->fn_offset;
        pcell->time_align = ho_finished->time_alignment;        
        nc_set_status(alr_data->dedi_data.bho_cell_index, FB_SB_SYNC);
        /*Now update the timing information w.r.t the blind cell*/
        nc_update_list(pcell->ba_arfcn);
        dedi_reset_bho_data();
        nc_resume_dedicated();
      }
#endif
      ma_dedi_cnf (DEDI_RES_OK);
      break;
    case HO_TIMEOUT: 
      ALR_TRACE_DEDI ("Handover failed due to T3124 expiry (=>Phy_info not received)");
#if defined (REL99) && defined (FF_BHO)
      /*Restore timing info only in case of Regular HO
        No need to restore timing info in case of Blind HO
        since we haven't updated timing info yet */
      if (alr_data->dedi_data.ho_type EQ HOT_BLIND)
        dedi_reset_bho_data();
      else
#endif
        nc_update_list (alr_data->dedi_data.old_bcch); /* Check if this is required??? */
      ma_dedi_cnf (DEDI_RES_TIMEOUT);
      break;
#if defined (REL99) && defined (FF_BHO)
    case HO_SYNC_FAIL: 
      ALR_TRACE_DEDI ("Handover failed: BSIC error");
      dedi_reset_bho_data();
      ma_dedi_cnf (DEDI_RES_CELL_NOT_SYNC);
      break;
#endif
    default:
      break;

  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_DEDI                   |
| STATE   : code                ROUTINE : dedi_chan_ass_fail         |
+--------------------------------------------------------------------+

  PURPOSE : Process Channel Assignment Failure (back to old
            dedicated channel).
            In fact this is a new configuration of the layer 1. The
            parameters of the old channel are stored in TI++.

*/

LOCAL void dedi_chan_ass_fail (void)
{
  GET_INSTANCE_DATA;
  PALLOC (chan_ass, MPHC_CHANNEL_ASSIGN_REQ);

  memset (chan_ass, 0, sizeof (T_MPHC_CHANNEL_ASSIGN_REQ));

  if (alr_data->dedi_data.redef_starting_time.start_time_present)
  {
    /*
     * Special case: frequency redefinition values ares stored.
     */
    memcpy (&chan_ass->channel_desc_1,
            &alr_data->dedi_data.redef_channel_desc,
            sizeof (T_channel_desc));
    memcpy (&chan_ass->frequency_list,
            &alr_data->dedi_data.redef_frequency_list,
            sizeof (T_frequency_list));
    memcpy (&chan_ass->channel_desc_1_bef_sti,
            &alr_data->dedi_data.old_channel_desc,
            sizeof (T_channel_desc));
    memcpy (&chan_ass->frequency_list_bef_sti,
            &alr_data->dedi_data.old_frequency_list,
            sizeof (T_frequency_list));
    memcpy (&chan_ass->starting_time,
            &alr_data->dedi_data.redef_starting_time,
            sizeof (T_starting_time));

    /*
     * convert channel numbers if needed
     */
    if (dedi_conversion_needed ())
    {
      dedi_convert_chan_desc ((T_channel_desc *)&chan_ass->channel_desc_1);
      dedi_convert_frequency_list (&chan_ass->frequency_list);
      dedi_convert_chan_desc ((T_channel_desc *)&chan_ass->channel_desc_1_bef_sti);
      dedi_convert_frequency_list ((T_frequency_list *)&chan_ass->frequency_list_bef_sti);
    }
  }
  else
  {
    /*
     * Normal case
     */
    memcpy (&chan_ass->channel_desc_1,
            &alr_data->dedi_data.old_channel_desc,
            sizeof (T_channel_desc));
    memcpy (&chan_ass->frequency_list,
            &alr_data->dedi_data.old_frequency_list,
            sizeof (T_frequency_list));

    /*
     * convert channel numbers if needed
     */
    if (dedi_conversion_needed ())
    {
      dedi_convert_chan_desc ((T_channel_desc *)&chan_ass->channel_desc_1);
      dedi_convert_frequency_list (&chan_ass->frequency_list);
    }
  }

  chan_ass->txpwr = alr_data->dedi_data.old_power;
  chan_ass->cipher_mode = alr_data->dedi_data.old_cipher_mode;
  chan_ass->channel_mode_1 = alr_data->dedi_data.old_channel_mode;
  chan_ass->a5_algorithm = alr_data->dedi_data.old_a5_algorithm;
  chan_ass->cipher_key = alr_data->dedi_data.old_cipher_key;
  chan_ass->dtx_allowed = alr_data->dedi_data.old_dtx_allowed;
  chan_ass->amr_configuration = alr_data->dedi_data.old_amr_configuration;

  /*
   * store actual channel for layer 2 channel decision
   */
  actual_channel = alr_data->dedi_data.old_channel;

  /*
   * store actual channel parameter in ALR. In fact they are
   * the old values.
   */
  if (alr_data->dedi_data.redef_starting_time.start_time_present)
  {
    /*
     * Special case: frequency redefinition values ares stored.
     */
    memcpy (&alr_data->dedi_data.act_channel_desc,
            &alr_data->dedi_data.redef_channel_desc,
            sizeof (T_channel_desc));
    memcpy (&alr_data->dedi_data.act_frequency_list,
            &alr_data->dedi_data.redef_frequency_list,
            sizeof (T_frequency_list));
  }
  else
  {
    /*
     * Normal case
     */
    memcpy (&alr_data->dedi_data.act_channel_desc,
            &alr_data->dedi_data.old_channel_desc,
            sizeof (T_channel_desc));
    memcpy (&alr_data->dedi_data.act_frequency_list,
            &alr_data->dedi_data.old_frequency_list,
            sizeof (T_frequency_list));
  }
  alr_data->dedi_data.act_power = alr_data->dedi_data.old_power;
  alr_data->dedi_data.act_cipher_mode = alr_data->dedi_data.old_cipher_mode;
  alr_data->dedi_data.act_channel_mode = alr_data->dedi_data.old_channel_mode;
  alr_data->dedi_data.act_a5_algorithm = alr_data->dedi_data.old_a5_algorithm;
  alr_data->dedi_data.act_cipher_key = alr_data->dedi_data.old_cipher_key;
  alr_data->dedi_data.act_dtx_allowed = alr_data->dedi_data.old_dtx_allowed;
  alr_data->dedi_data.act_amr_configuration = alr_data->dedi_data.old_amr_configuration;
  alr_data->dedi_data.act_mode = MODE_CHAN_ASS_FAIL;

  /*
   * send data to layer 1
   */
  ma_dedi_chan_ass_req (chan_ass);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_dedi                   |
| STATE   : code                ROUTINE : dedi_pdch_ass_fail         |
+--------------------------------------------------------------------+

  PURPOSE : PDCH Assignment Failure (back to old
            dedicated channel).

         Use the channel configuration in alr_data->dedi_data.act* with
         MPHC-IMMED-ASSIGN-REQ. With the subsequent MPHC-IMMED-ASSIGN-CON
         alr_data->dedi_data.temp* is NOT assigned to alr_data->dedi_data.act*,
         refer to function dedi_imm_ass_cnf.
         The actual switch back to the original channel is performed via
         a channel assignment procedure carried out by dedi_chan_ass_fail
         which is called in dedi_imm_ass_cnf.
*/

LOCAL void dedi_pdch_ass_fail (void)
{
  GET_INSTANCE_DATA;
  T_DEDI_DATA *d = &alr_data->dedi_data;
  PALLOC (immed_assign_req, MPHC_IMMED_ASSIGN_REQ);

  memset ( immed_assign_req, 0 , sizeof *immed_assign_req );

  immed_assign_req->channel_desc           =  d->old_channel_desc;
  immed_assign_req->timing_advance         =  (UBYTE)alr_data->nc_data.tav;
  immed_assign_req->frequency_list         =  d->old_frequency_list;
  immed_assign_req->starting_time          =  d->old_starting_time;
  immed_assign_req->frequency_list_bef_sti =  d->old_frequency_list_bef_sti;
  immed_assign_req->maio_bef_sti           =  d->old_channel_desc.chan_sel.rf_channel.maio;
/*
  immed_assign_req->bcch_allocation        =  ... not used in current implementation
  immed_assign_req->ba_id                  =  ... not used in current implementation
*/
  immed_assign_req->dtx_allowed            =  d->old_dtx_allowed;
  immed_assign_req->pwrc                   =  d->old_pwrc;

  SET_STATE (STATE_DEDI, DEDI_IMM_ASS_RECONN);
  PSENDX (L1, immed_assign_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_dedi                   |
| STATE   : code                ROUTINE : dedi_cco_fail              |
+--------------------------------------------------------------------+

  PURPOSE : Cell Change Order Failure (back to old
            dedicated channel).

*/

LOCAL void dedi_cco_fail (void)
{
  dedi_pdch_ass_fail();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_handover_fail         |
+--------------------------------------------------------------------+

  PURPOSE : Process Handover Failure (back to old
            dedicated channel).
            For handover switching back is carried out
            autonomously by layer 1. Only the configuration
            data in ALR is updated to the parameters of the
            old channel.

*/

LOCAL void dedi_handover_fail (void)
{
  GET_INSTANCE_DATA;

  /*
   * store actual channel value for layer 2 channel selection
   */
  actual_channel = alr_data->dedi_data.old_channel;

  /*
   * update actual channel parameters to the old values
   */
  alr_data->serving_cell = alr_data->dedi_data.act_bcch = alr_data->dedi_data.old_bcch;
  alr_data->dedi_data.act_channel_desc = alr_data->dedi_data.old_channel_desc;
  alr_data->dedi_data.act_frequency_list = alr_data->dedi_data.old_frequency_list;
  alr_data->dedi_data.act_cipher_mode = alr_data->dedi_data.old_cipher_mode;
  alr_data->dedi_data.act_channel_mode = alr_data->dedi_data.old_channel_mode;
  alr_data->dedi_data.act_a5_algorithm = alr_data->dedi_data.old_a5_algorithm;
  alr_data->dedi_data.act_cipher_key = alr_data->dedi_data.old_cipher_key;
  alr_data->dedi_data.act_amr_configuration = alr_data->dedi_data.old_amr_configuration;
  alr_data->dedi_data.act_mode = MODE_HANDOVER_FAIL;

  /*
   * Inform layer 1 about handover failure
   */
  nc_suspend_handover();
  ma_dedi_ho_fail_req ();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_imm_ass               |
+--------------------------------------------------------------------+

  PURPOSE : Converts MPH_DEDICATED_REQ to MPHC_IMMED_ASSIGN_REQ.
            This function is used during Immediate Assignment procedure.

*/

LOCAL void dedi_imm_ass (T_MPH_DEDICATED_REQ *dedicated_req)
{
  GET_INSTANCE_DATA;
  T_DEDI_DATA *dd = &alr_data->dedi_data;
  PALLOC (imm_ass, MPHC_IMMED_ASSIGN_REQ); /* T_MPHC_IMMED_ASSIGN_REQ */

  ALR_EM_CONFIGURE_IMMIDIATE_ASSIGNMENT;

  memset (imm_ass, 0, sizeof (T_MPHC_IMMED_ASSIGN_REQ));
  if (dedicated_req->ch_type.h EQ 1)
  {
    /*
     * with frequency hopping
     */
    imm_ass->channel_desc.chan_sel.h = 1;
    imm_ass->channel_desc.chan_sel.rf_channel.maio =
      dedicated_req->ch_type.maio;
    imm_ass->channel_desc.chan_sel.rf_channel.hsn =
      dedicated_req->ch_type.hsn;
    dedi_fill_ma (dedicated_req->ch_type.ma, &imm_ass->frequency_list);
    dedi_fill_ma (dedicated_req->ch_type2.ma,
                  (T_frequency_list *)&imm_ass->frequency_list_bef_sti);
  }
  else
  {
    ALR_TRACE_DEDI ("no hopping");
    /*
     * Avoid the union, arfcn is mapped to maio and hsn
     * in the non-hopping case.
     */
    imm_ass->channel_desc.chan_sel.h = 0;
    imm_ass->channel_desc.chan_sel.rf_channel.maio =
      (UBYTE)(dedicated_req->ch_type.arfcn & 0xFF);
    imm_ass->channel_desc.chan_sel.rf_channel.hsn =
      (UBYTE)((dedicated_req->ch_type.arfcn >> 8) & 0xFF);
  }

  /*
   * set the actual channel for layer 2 channel selection
   */
  actual_channel = imm_ass->channel_desc.channel_type =
                    CH_TYPE [dedicated_req->ch_type.ch];
  /*
   * set the rest of parameters
   */
  imm_ass->channel_desc.sub_channel           = SUB_TYPE [dedicated_req->ch_type.ch];
  imm_ass->channel_desc.timeslot_no           = dedicated_req->ch_type.tn;
  imm_ass->channel_desc.tsc                   = dedicated_req->ch_type.tsc;
  imm_ass->timing_advance                     = dedicated_req->tr_para.tav;
  nc_store_tav(dedicated_req->tr_para.tav);
  imm_ass->starting_time.start_time_present   = dedicated_req->start.v_start;
  imm_ass->starting_time.start_time.t1        = dedicated_req->start.t1;
  imm_ass->starting_time.start_time.t3        = dedicated_req->start.t3;
  imm_ass->starting_time.start_time.t2        = dedicated_req->start.t2;
  imm_ass->maio_bef_sti                       = dedicated_req->ch_type2.maio;
  imm_ass->dtx_allowed                        = dedicated_req->tr_para.dtx;
  /*
   * calculate radio link timeout value
   */
  dd->rlt = dd->act_rlt = (UBYTE)((dedicated_req->tr_para.rlt + 1) * 4);

  ALR_EM_SET_EM_ACT_RLT_2;

  memset (&imm_ass->bcch_allocation, 0, sizeof (T_bcch_allocation));
  imm_ass->ba_id = ALR_BA_LOW;
  imm_ass->pwrc = dedicated_req->tr_para.pwrc;
  /*
   * store new parameters as temporary channel configuration
   */
  dd->temp_bcch                      = dedicated_req->arfcn;
  dd->temp_channel_desc              = imm_ass->channel_desc;
  dd->temp_channel_mode              = dedi_channel_mode (MODE_SIG_ONLY, 0);
  dd->temp_frequency_list            = imm_ass->frequency_list;
  dd->temp_starting_time             = imm_ass->starting_time;
  dd->temp_frequency_list_bef_sti    = imm_ass->frequency_list_bef_sti;
  dd->temp_channel_desc_bef_sti      = imm_ass->channel_desc;
  dd->temp_channel_desc_bef_sti.chan_sel.rf_channel.maio
                                                     = imm_ass->maio_bef_sti;
  dd->temp_pwrc                      = imm_ass->pwrc;
  dd->temp_cipher_mode               = CI_NO_CIPHERING;
  dd->temp_a5_algorithm              = 0;
  memset ( &dd->temp_cipher_key, 0, sizeof (T_cipher_key) );

  SET_STATE (STATE_DEDI, DEDI_IMM_ASS);

  /*
   * convert channel numbers if needed
   */
  if (dedi_conversion_needed ())
  {
    dedi_convert_chan_desc (&imm_ass->channel_desc);
    dedi_convert_frequency_list (&imm_ass->frequency_list);
    dedi_convert_frequency_list ((T_frequency_list *)&imm_ass->frequency_list_bef_sti);
  }

  /*
   * send parameters to layer 1
   */
  ALR_TRACE_DEDI_PARA(imm_ass->channel_desc.chan_sel.h,
                      imm_ass->channel_desc.chan_sel.rf_channel.maio+
                      (imm_ass->channel_desc.chan_sel.rf_channel.hsn<<8),
                      imm_ass->channel_desc.channel_type,
                      imm_ass->channel_desc.sub_channel,
                      imm_ass->channel_desc.timeslot_no,
                      imm_ass->channel_desc.tsc,
                      imm_ass->timing_advance,
                      imm_ass->starting_time.start_time_present,
                      imm_ass->dtx_allowed,
                      imm_ass->maio_bef_sti);

  ma_dedi_imm_ass_req (imm_ass);
}

#ifdef FF_L23_A5_3
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_cyclically_repeat_kc  |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to cyclically repeat the Kc.

*/
LOCAL void dedi_cyclically_repeat_kc (UBYTE *to_kc, UBYTE *from_kc, U8 klen)
{
  UBYTE i;
  
  if (klen < KC_STRING_SIZE)
  {
  	for(i=klen ; i < KC_STRING_SIZE ; i++)
  	{
  		to_kc[i]=from_kc[i-klen];
  	}
  }	  
    
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_chan_ass              |
+--------------------------------------------------------------------+

  PURPOSE : Converts MPH_DEDICATED_REQ to MPHC_CHANNEL_ASSIGN_REQ.
            This function is used during channel assignment procedure.

*/

LOCAL void dedi_chan_ass (T_MPH_DEDICATED_REQ *dedicated_req)
{
  GET_INSTANCE_DATA;

  PALLOC (chan_ass, MPHC_CHANNEL_ASSIGN_REQ);

  ALR_EM_CONFIGURE_CHANNEL_ASSIGNMENT;

  memset (chan_ass, 0, sizeof (T_MPHC_CHANNEL_ASSIGN_REQ));
  dedi_chan_type_sel(&dedicated_req->ch_type,
                     &(chan_ass->channel_desc_1.chan_sel),
                     &chan_ass->frequency_list);
  /*
   * fill the layer 1 structure
   */
  alr_data->dedi_data.temp_channel =
    chan_ass->channel_desc_1.channel_type =
      CH_TYPE [dedicated_req->ch_type.ch];
  chan_ass->channel_desc_1.sub_channel =
    SUB_TYPE [dedicated_req->ch_type.ch];
  chan_ass->channel_desc_1.timeslot_no = dedicated_req->ch_type.tn;
  chan_ass->channel_desc_1.tsc = dedicated_req->ch_type.tsc;
  chan_ass->txpwr = cut_power_value (dedicated_req->tr_para.power,
                                     dedicated_req);
  chan_ass->channel_mode_1 = dedi_channel_mode(dedicated_req->tr_para.mode,
                             alr_data->dedi_data.temp_channel);
  chan_ass->amr_configuration.noise_suppression_control_bit = dedicated_req->amr_conf.nscb;
  chan_ass->amr_configuration.initial_codec_mode_indicator  = dedicated_req->amr_conf.icmi;
  chan_ass->amr_configuration.initial_codec_mode            = dedicated_req->amr_conf.st_mode;
  chan_ass->amr_configuration.active_codec_set              = dedicated_req->amr_conf.acs;
    /* Traces only needed for IOT session */
  TRACE_EVENT_P1("*AMR NSCB %d", chan_ass->amr_configuration.noise_suppression_control_bit);
  TRACE_EVENT_P1("*AMR ICMI %d", chan_ass->amr_configuration.initial_codec_mode_indicator);
  TRACE_EVENT_P1("*AMR ICM %d",  chan_ass->amr_configuration.initial_codec_mode);
  TRACE_EVENT_P1("*AMR ACS %d",  chan_ass->amr_configuration.active_codec_set);
  dedi_set_amr_conf(&chan_ass->amr_configuration, &dedicated_req->amr_conf);
  chan_ass->starting_time.start_time_present =
    dedicated_req->start.v_start;
  chan_ass->starting_time.start_time.t1 =
    dedicated_req->start.t1;
  chan_ass->starting_time.start_time.t3 =
    dedicated_req->start.t3;
  chan_ass->starting_time.start_time.t2 =
    dedicated_req->start.t2;

  if (dedicated_req->ch_type2.ch NEQ NOT_PRESENT_8BIT)
  {
    dedi_chan_type_sel(&dedicated_req->ch_type2,
                        &(chan_ass->channel_desc_1_bef_sti.chan_sel),
                        (T_frequency_list *)&chan_ass->frequency_list_bef_sti);
    chan_ass->channel_desc_1_bef_sti.channel_type =
      CH_TYPE [dedicated_req->ch_type2.ch];
    chan_ass->channel_desc_1_bef_sti.sub_channel =
      SUB_TYPE [dedicated_req->ch_type2.ch];
    chan_ass->channel_desc_1_bef_sti.timeslot_no =
      dedicated_req->ch_type2.tn;
    chan_ass->channel_desc_1_bef_sti.tsc =
      dedicated_req->ch_type2.tsc;
  }
  chan_ass->cipher_mode = dedicated_req->ciph.stat;
  chan_ass->a5_algorithm = dedicated_req->ciph.algo;
  memcpy (&chan_ass->cipher_key, dedicated_req->ciph.kc, KC_STRING_SIZE);
  #ifdef FF_L23_A5_3
  dedi_cyclically_repeat_kc ((UBYTE *)&chan_ass->cipher_key, (UBYTE *)dedicated_req->ciph.kc, KLEN);
  #endif
  chan_ass->dtx_allowed = dedicated_req->tr_para.dtx;

  /*
   * store the new configuration as temporary configuration
   */
  alr_data->dedi_data.temp_bcch = dedicated_req->arfcn;
  memcpy (&alr_data->dedi_data.temp_channel_desc,
          &chan_ass->channel_desc_1,
          sizeof (T_channel_desc));
  alr_data->dedi_data.temp_channel_mode = chan_ass->channel_mode_1;
  alr_data->dedi_data.temp_frequency_list = chan_ass->frequency_list;
  alr_data->dedi_data.temp_cipher_mode = chan_ass->cipher_mode;
  alr_data->dedi_data.temp_a5_algorithm = chan_ass->a5_algorithm;
  alr_data->dedi_data.temp_cipher_key = chan_ass->cipher_key;
  alr_data->dedi_data.temp_power = chan_ass->txpwr;
  alr_data->dedi_data.temp_dtx_allowed = chan_ass->dtx_allowed;
  alr_data->dedi_data.act_mode = MODE_CHAN_ASSIGN;
  alr_data->dedi_data.temp_amr_configuration = chan_ass->amr_configuration;

  /*
   * convert channel numbers if needed
   */
  if (dedi_conversion_needed ())
  {
    dedi_convert_chan_desc ((T_channel_desc *)&chan_ass->channel_desc_1);
    dedi_convert_chan_desc ((T_channel_desc *)&chan_ass->channel_desc_1_bef_sti);
    dedi_convert_frequency_list (&chan_ass->frequency_list);
    dedi_convert_frequency_list ((T_frequency_list *)&chan_ass->frequency_list_bef_sti);
  }

  /*
   * inform layer 1 about the new channel configuration
   */
  ma_dedi_chan_ass_req (chan_ass);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_dedi                   |
| STATE   : code                ROUTINE : dedi_pdch_ass              |
+--------------------------------------------------------------------+

  PURPOSE : "Deactivation of previously assigned channels (layer1)"
            according to GSM 04.18, clause 3.4.19 during a
            PDCH Assignment procedure.

*/

LOCAL void dedi_pdch_ass (T_MPH_DEDICATED_REQ *dedicated_req)
{
  GET_INSTANCE_DATA;
  T_DEDI_DATA *d = &alr_data->dedi_data;

  /*
   * The PDCH Assignment Procedure is similar to the Channel Assignment
   * Procedure. Both are perfomred in dedicated state in order to switch
   * to a 'new' channel. "Normal operation" is suspend during the procedure.
   * In case of failure the old channel is activated and the message
   * ASSIGNMENT FAILURE is sent.
   */

  ma_stop_active_procs ( STOP_PCH_READING );

  /* Current state is now DEDI_INACTIVE.
   * In Layer 1 SDCCH or TCH is stopped. In order to re-activate the channel
   * (failure to TBF establishment), the Current Channel Configuration must be
   * stored. This configuration will then be used to re-establish the channel.
   * The Current Channel Configuration is provided in the last successful
   * Immediate Assignment, Channel Assignment or Handover in the variables
   * alr_data->dedi_data.act***. The re-activation can be done with the
   * Immediate Assignment Procedure followed by a Channel Assignment Procedure.
   * However there is the prerequisite that L1 must be in Idle Mode to perform
   * Immediate Assignment. This will be achieved by sending MPH-IDLE-REQ.
   */

  d->old_channel                = actual_channel;
  d->old_bcch                   = d->act_bcch;
  d->old_channel_desc           = d->act_channel_desc;
  d->old_channel_mode           = d->act_channel_mode;
  d->old_frequency_list         = d->act_frequency_list;
  d->old_starting_time          = d->act_starting_time;
  d->old_frequency_list_bef_sti = d->act_frequency_list_bef_sti;
  d->old_channel_desc_bef_sti   = d->act_channel_desc_bef_sti;
  d->old_pwrc                   = d->act_pwrc;
  d->old_cipher_mode            = d->act_cipher_mode;
  d->old_a5_algorithm           = d->act_a5_algorithm;
  d->old_cipher_key             = d->act_cipher_key;
  d->old_power                  = d->act_power;
  d->old_dtx_allowed            = d->act_dtx_allowed;  

  /*
   * Save the activation mode for Resume / Reconnection
   */
  alr_data->dedi_data.act_mode = MODE_PDCH_ASSIGN;  
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_async_handover        |
+--------------------------------------------------------------------+

  PURPOSE : Converts MPH_DEDICATED_REQ to MPHC_ASYNC_HO_REQ.
            This function is used during asynchronous handover.

*/

LOCAL void dedi_async_handover (T_MPH_DEDICATED_REQ * dedicated_req)
{
  GET_INSTANCE_DATA;

  PALLOC (async, MPHC_ASYNC_HO_REQ);

  ALR_EM_CONFIGURE_HANDOVER(EM_HANDOVER_ASYNC);

  memset (async, 0, sizeof (T_MPHC_ASYNC_HO_REQ));
  if (dedicated_req->ch_type.h EQ 1)
  {
    ALR_TRACE_DEDI ("hopping");
    /*
     * new channel with frequency hopping.
     */
    async->handover_command.channel_desc_1.chan_sel.h = 1;
    async->handover_command.channel_desc_1.chan_sel.rf_channel.maio =
      dedicated_req->ch_type.maio;
    async->handover_command.channel_desc_1.chan_sel.rf_channel.hsn =
      dedicated_req->ch_type.hsn;
    dedi_fill_ma (dedicated_req->ch_type.ma,
                  &async->handover_command.frequency_list);
  }
  else
  {
    ALR_TRACE_DEDI ("no hop");
    /*
     * Avoid the union in the non-hopping case. Mapping
     * of arfcn to maio and hsn.
     */
    async->handover_command.channel_desc_1.chan_sel.h = 0;
    async->handover_command.channel_desc_1.chan_sel.rf_channel.maio =
           (UBYTE)(dedicated_req->ch_type.arfcn & 0xFF);
    async->handover_command.channel_desc_1.chan_sel.rf_channel.hsn =
           (UBYTE)((dedicated_req->ch_type.arfcn >> 8) & 0xFF);
  }

  /*
   * convert the parameters to the layer 1 structure
   */
  alr_data->dedi_data.temp_channel = async->handover_command.channel_desc_1.channel_type = CH_TYPE [dedicated_req->ch_type.ch];
  async->handover_command.channel_desc_1.sub_channel = SUB_TYPE [dedicated_req->ch_type.ch];
  async->handover_command.channel_desc_1.timeslot_no = dedicated_req->ch_type.tn;
  async->handover_command.channel_desc_1.tsc = dedicated_req->ch_type.tsc;
  async->handover_command.txpwr = cut_power_value (dedicated_req->ho_param.ho_pow, dedicated_req);
  async->handover_command.ho_acc = dedicated_req->ho_param.ho_ref;
  async->handover_command.starting_time.start_time_present = dedicated_req->start.v_start;
  if (dedicated_req->start.v_start)
  {
    ALR_TRACE_DEDI ("start time");

    async->handover_command.starting_time.start_time.t1 = dedicated_req->start.t1;
    async->handover_command.starting_time.start_time.t3 = dedicated_req->start.t3;
    async->handover_command.starting_time.start_time.t2 = dedicated_req->start.t2;
  }
  async->handover_command.cell_description.ncc = (UBYTE)(dedicated_req->bsic >> 3);
  async->handover_command.cell_description.bcc = (UBYTE)(dedicated_req->bsic & 7);
  async->handover_command.cell_description.bcch_carrier = ARFCN_TO_L1 (dedicated_req->arfcn);
  async->handover_command.channel_mode_1 =
    dedi_channel_mode (dedicated_req->tr_para.mode,
                       alr_data->dedi_data.temp_channel);
  async->amr_configuration.noise_suppression_control_bit = dedicated_req->amr_conf.nscb;
  async->amr_configuration.initial_codec_mode_indicator  = dedicated_req->amr_conf.icmi;
  async->amr_configuration.initial_codec_mode            = dedicated_req->amr_conf.st_mode;
  async->amr_configuration.active_codec_set              = dedicated_req->amr_conf.acs;
  /* Traces only needed for IOT session */
  TRACE_EVENT_P1("*AMR NSCB %d", async->amr_configuration.noise_suppression_control_bit);
  TRACE_EVENT_P1("*AMR ICMI %d", async->amr_configuration.initial_codec_mode_indicator);
  TRACE_EVENT_P1("*AMR ICM %d",  async->amr_configuration.initial_codec_mode);
  TRACE_EVENT_P1("*AMR ACS %d",  async->amr_configuration.active_codec_set);
  dedi_set_amr_conf( &async->amr_configuration, &dedicated_req->amr_conf);

  if (dedicated_req->start.v_start AND
      dedicated_req->ch_type2.ch NEQ NOT_PRESENT_8BIT)
  {
    ALR_TRACE_DEDI ("bef sti");
    /*
     * before starting time elements are available
     */

    dedi_chan_type_sel (&dedicated_req->ch_type2,
                       &(async->handover_command.channel_desc_1_bef_sti.chan_sel),
                       (T_frequency_list *)&async->handover_command.frequency_list_bef_sti);

    async->handover_command.channel_desc_1_bef_sti.channel_type = CH_TYPE [dedicated_req->ch_type2.ch];
    async->handover_command.channel_desc_1_bef_sti.sub_channel = SUB_TYPE [dedicated_req->ch_type2.ch];
    async->handover_command.channel_desc_1_bef_sti.timeslot_no = dedicated_req->ch_type2.tn;
    async->handover_command.channel_desc_1_bef_sti.tsc = dedicated_req->ch_type2.tsc;
  }
  async->handover_command.cipher_mode = dedicated_req->ciph.stat;
  async->handover_command.a5_algorithm = dedicated_req->ciph.algo;
  memcpy (&async->cipher_key, dedicated_req->ciph.kc, KC_STRING_SIZE);
  #ifdef FF_L23_A5_3
  dedi_cyclically_repeat_kc ((UBYTE *)&async->cipher_key, (UBYTE *)dedicated_req->ciph.kc, KLEN);
  #endif

  /*
   * store the new parameters as temporary configuration.
   */
  alr_data->dedi_data.temp_bcch = dedicated_req->arfcn;
  memcpy (&alr_data->dedi_data.temp_channel_desc, &async->handover_command.channel_desc_1, sizeof (T_channel_desc));
  alr_data->dedi_data.temp_channel_mode = async->handover_command.channel_mode_1;
  alr_data->dedi_data.temp_frequency_list = async->handover_command.frequency_list;
  alr_data->dedi_data.temp_cipher_mode = async->handover_command.cipher_mode;
  alr_data->dedi_data.temp_a5_algorithm = async->handover_command.a5_algorithm;
  alr_data->dedi_data.temp_cipher_key = async->cipher_key;
  alr_data->dedi_data.temp_power = async->handover_command.txpwr;
  alr_data->dedi_data.temp_bsic = dedicated_req->bsic;
  alr_data->dedi_data.temp_amr_configuration = async->amr_configuration;
  alr_data->dedi_data.act_mode = MODE_ASYNC_HANDOVER;

#if defined (REL99) && defined (FF_BHO)
  dedi_set_ho_type ((ULONG *)&async->fn_offset, (ULONG *)&async->time_alignmnt, &async->handover_type);
#else
  nc_get_fn_time (alr_data->dedi_data.temp_bcch, (ULONG *)&async->fn_offset, (ULONG *)&async->time_alignmnt);
#endif

  /*
   * convert channel numbers if needed
   */
  if (dedi_conversion_needed ())
  {
    dedi_convert_chan_desc ((T_channel_desc *)&async->handover_command.channel_desc_1);
    dedi_convert_chan_desc ((T_channel_desc *)&async->handover_command.channel_desc_1_bef_sti);
    dedi_convert_frequency_list (&async->handover_command.frequency_list);
    dedi_convert_frequency_list ((T_frequency_list *)&async->handover_command.frequency_list_bef_sti);
  }
  /*
   * forward the parameters to layer 1
   */
  ALR_TRACE_DEDI_HO_1 (dedicated_req->arfcn,
                       dedicated_req->bsic,
                       dedicated_req->start.v_start,
                       dedicated_req->ch_type2.ch,
                       dedicated_req->ch_type.ch,
                       dedicated_req->ch_type.tsc);
  ALR_TRACE_DEDI_HO_2 (dedicated_req->ch_type.tn,
                       dedicated_req->ch_type.arfcn,
                       dedicated_req->ho_param.ho_pow,
                       dedicated_req->ho_param.ho_ref,
                       dedicated_req->ho_param.ho_acc_type,
                       dedicated_req->ho_param.ho_nci);
  ALR_TRACE_DEDI_HO_3 (dedicated_req->tr_para.mode,
                       dedicated_req->tr_para.power,
                       dedicated_req->tr_para.dtx,
                       dedicated_req->tr_para.rlt,
                       dedicated_req->tr_para.tav,
                       dedicated_req->tr_para.pwrc);
  ALR_TRACE_DEDI_HO_4 (dedicated_req->tr_para.mode,
                       dedicated_req->ciph.stat,
                       dedicated_req->ciph.algo);
  ALR_TRACE_DEDI_HO_5 (async->handover_command.channel_desc_1.chan_sel.h,
                       async->handover_command.channel_desc_1.chan_sel.rf_channel.maio,
                       async->handover_command.channel_desc_1.chan_sel.rf_channel.hsn,
                       async->handover_command.channel_desc_1.channel_type,
                       async->handover_command.channel_desc_1.sub_channel,
                       async->handover_command.channel_desc_1.timeslot_no);
  ALR_TRACE_DEDI_HO_6 (async->handover_command.channel_desc_1.tsc,
                       async->handover_command.txpwr,
                       async->handover_command.ho_acc,
                       async->handover_command.starting_time.start_time_present,
                       async->handover_command.cell_description.ncc,
                       async->handover_command.cell_description.bcc);
  ALR_TRACE_DEDI_HO_7 (async->handover_command.cell_description.bcch_carrier,
                       async->handover_command.channel_mode_1,
                       async->handover_command.cipher_mode,
                       async->handover_command.a5_algorithm);

  ma_dedi_async_ho_req (async);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_sync_handover         |
+--------------------------------------------------------------------+

  PURPOSE : Converts MPH_DEDICATED_REQ to MPHC_SYNC_HO_REQ.
            This function is used during synchronous handover.

*/

LOCAL void dedi_sync_handover (T_MPH_DEDICATED_REQ *dedicated_req)
{
  GET_INSTANCE_DATA;

  PALLOC (sync, MPHC_SYNC_HO_REQ);

  ALR_EM_CONFIGURE_HANDOVER(EM_HANDOVER_SYNC);

  memset (sync, 0, sizeof (T_MPHC_SYNC_HO_REQ));
  dedi_chan_type_sel(&dedicated_req->ch_type,
                     &(sync->handover_command.channel_desc_1.chan_sel),
                     &(sync->handover_command.frequency_list));
  /*
   * fill layer 1 structure with the new parameters
   */
  alr_data->dedi_data.temp_channel =
    sync->handover_command.channel_desc_1.channel_type =
      CH_TYPE [dedicated_req->ch_type.ch];
  sync->handover_command.channel_desc_1.sub_channel =
    SUB_TYPE [dedicated_req->ch_type.ch];
  sync->handover_command.channel_desc_1.timeslot_no =
    dedicated_req->ch_type.tn;
  sync->handover_command.channel_desc_1.tsc = dedicated_req->ch_type.tsc;
  sync->handover_command.txpwr =
    cut_power_value (dedicated_req->ho_param.ho_pow, dedicated_req);
  sync->handover_command.ho_acc = dedicated_req->ho_param.ho_ref;
  sync->handover_command.starting_time.start_time_present =
    dedicated_req->start.v_start;
  sync->handover_command.starting_time.start_time.t1 =
    dedicated_req->start.t1;
  sync->handover_command.starting_time.start_time.t3 =
    dedicated_req->start.t3;
  sync->handover_command.starting_time.start_time.t2 =
    dedicated_req->start.t2;

  sync->nci = dedicated_req->ho_param.ho_nci;
  sync->handover_command.cell_description.ncc =
        (UBYTE)(dedicated_req->bsic >> 3);
  sync->handover_command.cell_description.bcc =
        (UBYTE)(dedicated_req->bsic & 7);
  sync->handover_command.cell_description.bcch_carrier =
    ARFCN_TO_L1(dedicated_req->arfcn);
  sync->handover_command.channel_mode_1 =
    dedi_channel_mode (dedicated_req->tr_para.mode,
                       alr_data->dedi_data.temp_channel);
  sync->amr_configuration.noise_suppression_control_bit = dedicated_req->amr_conf.nscb;
  sync->amr_configuration.initial_codec_mode_indicator  = dedicated_req->amr_conf.icmi;
  sync->amr_configuration.initial_codec_mode            = dedicated_req->amr_conf.st_mode;
  sync->amr_configuration.active_codec_set              = dedicated_req->amr_conf.acs;
  /* Traces only needed for IOT session */
  TRACE_EVENT_P1("*AMR NSCB %d", sync->amr_configuration.noise_suppression_control_bit);
  TRACE_EVENT_P1("*AMR ICMI %d", sync->amr_configuration.initial_codec_mode_indicator);
  TRACE_EVENT_P1("*AMR ICM %d",  sync->amr_configuration.initial_codec_mode);
  TRACE_EVENT_P1("*AMR ACS %d",  sync->amr_configuration.active_codec_set);
  dedi_set_amr_conf(&sync->amr_configuration, &dedicated_req->amr_conf);
  dedi_chan_type_desc (dedicated_req,
                       (T_channel_desc_1_bef_sti *)&(sync->handover_command.channel_desc_1_bef_sti),
                       (T_frequency_list *)&sync->handover_command.frequency_list_bef_sti);
  sync->handover_command.cipher_mode = dedicated_req->ciph.stat;
  sync->handover_command.a5_algorithm = dedicated_req->ciph.algo;
  memcpy (&sync->cipher_key, dedicated_req->ciph.kc, KC_STRING_SIZE);
  #ifdef FF_L23_A5_3
  dedi_cyclically_repeat_kc ((UBYTE *)&sync->cipher_key, (UBYTE *)dedicated_req->ciph.kc, KLEN);
  #endif

  /*
   * store the new values as temporary configuration
   */
  alr_data->dedi_data.temp_bcch = dedicated_req->arfcn;
  memcpy (&alr_data->dedi_data.temp_channel_desc,
          &sync->handover_command.channel_desc_1,
          sizeof (T_channel_desc));
  alr_data->dedi_data.temp_channel_mode =
    sync->handover_command.channel_mode_1;
  alr_data->dedi_data.temp_frequency_list =
    sync->handover_command.frequency_list;
  alr_data->dedi_data.temp_cipher_mode =
    sync->handover_command.cipher_mode;
  alr_data->dedi_data.temp_a5_algorithm =
    sync->handover_command.a5_algorithm;
  alr_data->dedi_data.temp_cipher_key =
    sync->cipher_key;
  alr_data->dedi_data.temp_power = sync->handover_command.txpwr;
  alr_data->dedi_data.temp_bsic  = dedicated_req->bsic;
  alr_data->dedi_data.temp_amr_configuration = sync->amr_configuration;
  alr_data->dedi_data.act_mode = MODE_SYNC_HANDOVER;

#if defined (REL99) && defined (FF_BHO)
  dedi_set_ho_type ((ULONG *)&sync->fn_offset, (ULONG *)&sync->time_alignmnt, &sync->handover_type);
#else
  nc_get_fn_time (alr_data->dedi_data.temp_bcch,
                  (ULONG *)&sync->fn_offset,
                  (ULONG *)&sync->time_alignmnt);
#endif

  /*
   * convert channel numbers if needed
   */
  if (dedi_conversion_needed ())
  {
    dedi_convert_chan_desc ((T_channel_desc *)&sync->handover_command.channel_desc_1);
    dedi_convert_chan_desc ((T_channel_desc *)&sync->handover_command.channel_desc_1_bef_sti);
    dedi_convert_frequency_list (&sync->handover_command.frequency_list);
    dedi_convert_frequency_list ((T_frequency_list *)&sync->handover_command.frequency_list_bef_sti);
  }
  /*
   * forward the new configuration to layer 1
   */
  ma_dedi_sync_ho_req (sync);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_pre_sync_handover     |
+--------------------------------------------------------------------+

  PURPOSE : Converts MPH_DEDICATED_REQ to MPHC_PRE_SYNC_HO_REQ.
            This function is used during pre-synchronous handover.

*/

LOCAL void dedi_pre_sync_handover (T_MPH_DEDICATED_REQ *dedicated_req)
{
  GET_INSTANCE_DATA;

  PALLOC (presync, MPHC_PRE_SYNC_HO_REQ);

  ALR_EM_CONFIGURE_HANDOVER(EM_HANDOVER_PRE_SYNC);

  memset (presync, 0, sizeof (T_MPHC_PRE_SYNC_HO_REQ));
  dedi_chan_type_sel(&dedicated_req->ch_type,
                     &(presync->handover_command.channel_desc_1.chan_sel),
                     &(presync->handover_command.frequency_list));

  /*
   * fill layer 1 structure with the new parameters
   */
  alr_data->dedi_data.temp_channel =
    presync->handover_command.channel_desc_1.channel_type =
      CH_TYPE [dedicated_req->ch_type.ch];
  presync->handover_command.channel_desc_1.sub_channel =
    SUB_TYPE [dedicated_req->ch_type.ch];
  presync->handover_command.channel_desc_1.timeslot_no =
    dedicated_req->ch_type.tn;
  presync->handover_command.channel_desc_1.tsc = dedicated_req->ch_type.tsc;
  presync->handover_command.txpwr =
    cut_power_value (dedicated_req->ho_param.ho_pow, dedicated_req);
  presync->handover_command.ho_acc = dedicated_req->ho_param.ho_ref;
  presync->handover_command.starting_time.start_time_present =
    dedicated_req->start.v_start;
  presync->handover_command.starting_time.start_time.t1 =
    dedicated_req->start.t1;
  presync->handover_command.starting_time.start_time.t3 =
    dedicated_req->start.t3;
  presync->handover_command.starting_time.start_time.t2 =
    dedicated_req->start.t2;
  presync->handover_command.cell_description.ncc =
           (UBYTE)(dedicated_req->bsic >> 3);
  presync->handover_command.cell_description.bcc =
           (UBYTE)(dedicated_req->bsic & 7);
  presync->handover_command.cell_description.bcch_carrier =
    ARFCN_TO_L1(dedicated_req->arfcn);
  presync->nci = dedicated_req->ho_param.ho_nci;
  presync->handover_command.channel_mode_1 =
    dedi_channel_mode (dedicated_req->tr_para.mode,
                       alr_data->dedi_data.temp_channel);
  presync->amr_configuration.noise_suppression_control_bit = dedicated_req->amr_conf.nscb;
  presync->amr_configuration.initial_codec_mode_indicator  = dedicated_req->amr_conf.icmi;
  presync->amr_configuration.initial_codec_mode            = dedicated_req->amr_conf.st_mode;
  presync->amr_configuration.active_codec_set              = dedicated_req->amr_conf.acs;
  /* Traces only needed for IOT session */
  TRACE_EVENT_P1("*AMR NSCB %d", presync->amr_configuration.noise_suppression_control_bit);
  TRACE_EVENT_P1("*AMR ICMI %d", presync->amr_configuration.initial_codec_mode_indicator);
  TRACE_EVENT_P1("*AMR ICM %d",  presync->amr_configuration.initial_codec_mode);
  TRACE_EVENT_P1("*AMR ACS %d",  presync->amr_configuration.active_codec_set);
  dedi_set_amr_conf(&presync->amr_configuration, &dedicated_req->amr_conf);
  presync->timing_advance_valid = TRUE;
  presync->timing_advance = dedicated_req->tr_para.tav;

  dedi_chan_type_desc (dedicated_req,
                       (T_channel_desc_1_bef_sti *)&(presync->handover_command.channel_desc_1_bef_sti),
                       (T_frequency_list *)&presync->handover_command.frequency_list_bef_sti);
  presync->handover_command.cipher_mode = dedicated_req->ciph.stat;
  presync->handover_command.a5_algorithm = dedicated_req->ciph.algo;
  memcpy (&presync->cipher_key, dedicated_req->ciph.kc, KC_STRING_SIZE);
  #ifdef FF_L23_A5_3
  dedi_cyclically_repeat_kc ((UBYTE *)&presync->cipher_key, (UBYTE *)dedicated_req->ciph.kc, KLEN);
  #endif

  /*
   * store the new parameters as temporary configuration.
   */
  alr_data->dedi_data.temp_bcch = dedicated_req->arfcn;
  memcpy (&alr_data->dedi_data.temp_channel_desc,
          &presync->handover_command.channel_desc_1,
          sizeof (T_channel_desc));
  alr_data->dedi_data.temp_channel_mode =
    presync->handover_command.channel_mode_1;
  alr_data->dedi_data.temp_frequency_list =
    presync->handover_command.frequency_list;
  alr_data->dedi_data.temp_cipher_mode =
    presync->handover_command.cipher_mode;
  alr_data->dedi_data.temp_a5_algorithm =
    presync->handover_command.a5_algorithm;
  alr_data->dedi_data.temp_cipher_key =
    presync->cipher_key;
  alr_data->dedi_data.temp_power = presync->handover_command.txpwr;
  alr_data->dedi_data.temp_bsic  = dedicated_req->bsic;
  alr_data->dedi_data.temp_amr_configuration = presync->amr_configuration;
  alr_data->dedi_data.act_mode = MODE_PRE_SYNC_HANDOVER;

#if defined (REL99) && defined (FF_BHO)
  dedi_set_ho_type ((ULONG *)&presync->fn_offset,
                  (ULONG *)&presync->time_alignmnt, &presync->handover_type);
#else
  nc_get_fn_time (alr_data->dedi_data.temp_bcch,
                  (ULONG *)&presync->fn_offset,
                  (ULONG *)&presync->time_alignmnt);
#endif

  /*
   * convert channel numbers if needed
   */
  if (dedi_conversion_needed ())
  {
    dedi_convert_chan_desc ((T_channel_desc *)&presync->handover_command.channel_desc_1);
    dedi_convert_chan_desc ((T_channel_desc *)&presync->handover_command.channel_desc_1_bef_sti);
    dedi_convert_frequency_list (&presync->handover_command.frequency_list);
    dedi_convert_frequency_list ((T_frequency_list *)&presync->handover_command.frequency_list_bef_sti);
  }
  /*
   * forward the parameters to layer 1
   */
  ma_dedi_pre_sync_ho_req (presync);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_freq_redef_req        |
+--------------------------------------------------------------------+

  PURPOSE : Process signal dedi_freq_redef_req from SDL Process
            alr_MAIN.
            This function is used during frequency redefinition.

*/

GLOBAL void dedi_freq_redef_req (T_MPH_FREQ_REDEF_REQ *freq)
{
  GET_INSTANCE_DATA;
  PALLOC (change, MPHC_CHANGE_FREQUENCY);

  ALR_EM_CONFIGURE_FREQUENCY_REDEFINITION;

  memset (change, 0, sizeof (T_MPHC_CHANGE_FREQUENCY));
  dedi_chan_type_sel(&freq->ch_type,
                     &(change->channel_desc.chan_sel),
                     &change->frequency_list);

  /*
   * fill layer 1 structure with the parameters.
   */
  change->channel_desc.channel_type = CH_TYPE [freq->ch_type.ch];
  change->channel_desc.sub_channel = SUB_TYPE [freq->ch_type.ch];
  change->channel_desc.timeslot_no = freq->ch_type.tn;
  change->channel_desc.tsc = freq->ch_type.tsc;
  change->starting_time.start_time_present = freq->start.v_start;
  change->starting_time.start_time.t1 = freq->start.t1;
  change->starting_time.start_time.t3 = freq->start.t3;
  change->starting_time.start_time.t2 = freq->start.t2;

  /*
   * store new values as temporary configuration
   */
  alr_data->dedi_data.redef_channel_desc = change->channel_desc;
  alr_data->dedi_data.redef_frequency_list = change->frequency_list;
  alr_data->dedi_data.redef_starting_time = change->starting_time;

  /*
   * convert channel numbers if needed
   */
  if (dedi_conversion_needed ())
  {
    dedi_convert_chan_desc (&change->channel_desc);
    dedi_convert_frequency_list (&change->frequency_list);
  }
  /*
   * forward parameters to layer 1
   */
  ma_dedi_change_freq_req (change);

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_chan_mode_req         |
+--------------------------------------------------------------------+

  PURPOSE : Process signal dedi_chan_mode_req from SDL Process
            alr_MAIN.
            This function is used during channel mode modify procedure.

*/

GLOBAL void dedi_chan_mode_req (T_MPH_CHANNEL_MODE_REQ *ch_mode)
{
  GET_INSTANCE_DATA;
  int i;
  PALLOC (chm, MPHC_CHANNEL_MODE_MODIFY_REQ);

  ALR_EM_CONFIGURE_CHANNEL_MODE_MODIFY;

  memset (chm, 0, sizeof (T_MPHC_CHANNEL_MODE_MODIFY_REQ));
  /*
   * convert parameters
   */
  chm->sub_channel = SUB_TYPE [ch_mode->ch];
  alr_data->dedi_data.act_channel_mode =
    chm->channel_mode = dedi_channel_mode (ch_mode->mode,
                                           actual_channel);

  /*
   * set multirate configuration parameters
   */
   if(ch_mode->mode EQ CM_AMR)
   {
     alr_data->dedi_data.act_amr_configuration.noise_suppression_control_bit =
       chm->amr_configuration.noise_suppression_control_bit = ch_mode->amr_conf.nscb;
     alr_data->dedi_data.act_amr_configuration.initial_codec_mode_indicator  =
       chm->amr_configuration.initial_codec_mode_indicator  = ch_mode->amr_conf.icmi;
     alr_data->dedi_data.act_amr_configuration.initial_codec_mode            =
       chm->amr_configuration.initial_codec_mode            = ch_mode->amr_conf.st_mode;
     alr_data->dedi_data.act_amr_configuration.active_codec_set              =
       chm->amr_configuration.active_codec_set              = ch_mode->amr_conf.acs;

     /* Traces only needed for IOT session */
     TRACE_EVENT_P1("*AMR NSCB %d", chm->amr_configuration.noise_suppression_control_bit);
     TRACE_EVENT_P1("*AMR ICMI %d", chm->amr_configuration.initial_codec_mode_indicator);
     TRACE_EVENT_P1("*AMR ICM %d",  chm->amr_configuration.initial_codec_mode);
     TRACE_EVENT_P1("*AMR ACS %d",  chm->amr_configuration.active_codec_set);
     for (i=0; i<ch_mode->amr_conf.c_cod_prop; i++)
     {
       memcpy(&alr_data->dedi_data.act_amr_configuration.threshold[i],  &ch_mode->amr_conf.cod_prop[i].codec_thr, 1);
       memcpy(&alr_data->dedi_data.act_amr_configuration.hysteresis[i], &ch_mode->amr_conf.cod_prop[i].codec_hyst, 1);

       memcpy(&chm->amr_configuration.threshold[i],  &ch_mode->amr_conf.cod_prop[i].codec_thr, 1);
       memcpy(&chm->amr_configuration.hysteresis[i], &ch_mode->amr_conf.cod_prop[i].codec_hyst, 1);
       TRACE_EVENT_P2("*AMR threshold[%d] %d",  i, chm->amr_configuration.threshold[i]);
       TRACE_EVENT_P2("*AMR hysteresis[%d] %d", i, chm->amr_configuration.hysteresis[i]);
     }
   }

  /*
   * forward new channel mode to layer 1
   */
  ma_dedi_chan_mode_req (chm);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_ciph_req              |
+--------------------------------------------------------------------+

  PURPOSE : Process signal dedi_ciph_req from SDL Process
            alr_MAIN.
            This function is used during cipher mode setting procedure.

*/

GLOBAL void dedi_ciph_req (T_MPH_CIPHERING_REQ *ciph_req)
{
  GET_INSTANCE_DATA;
  PALLOC (set_ciph, MPHC_SET_CIPHERING_REQ);

  ALR_EM_CONFIGURE_CIPHER_MODE_SETTING;

  memset (set_ciph, 0, sizeof (T_MPHC_SET_CIPHERING_REQ));
  /*
   * convert the parameters to the layer 1 structure
   */
  set_ciph->cipher_mode = ciph_req->ciph.stat;
  set_ciph->a5_algorithm = ciph_req->ciph.algo;
  memcpy (set_ciph->new_ciph_param.A, ciph_req->ciph.kc, KC_STRING_SIZE);
  #ifdef FF_L23_A5_3
  dedi_cyclically_repeat_kc ((UBYTE *)set_ciph->new_ciph_param.A, (UBYTE *)ciph_req->ciph.kc, KLEN);  
  #endif

  alr_data->dedi_data.act_cipher_mode = set_ciph->cipher_mode;
  alr_data->dedi_data.act_a5_algorithm = set_ciph->a5_algorithm;
  memcpy (&alr_data->dedi_data.act_cipher_key,
          &set_ciph->new_ciph_param, KC_STRING_SIZE);
  #ifdef FF_L23_A5_3
  dedi_cyclically_repeat_kc ((UBYTE *)&alr_data->dedi_data.act_cipher_key,
          (UBYTE *)&set_ciph->new_ciph_param, KLEN);
  #endif

  /*
   * forward the parameters to layer 1
   */
  ma_dedi_ciph_req (set_ciph);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_imm_ass_cnf           |
+--------------------------------------------------------------------+

  PURPOSE : Process signal dedi_imm_ass_cnf from SDL process
            Main_Control.
      Layer 1 confirms the configuration of a dedicated
      channel during the immediate assignment procedure.

*/

GLOBAL void dedi_imm_ass_cnf (void)
{
  GET_INSTANCE_DATA;
  T_DEDI_DATA * d = &alr_data->dedi_data;

  if (GET_STATE (STATE_DEDI) EQ DEDI_IMM_ASS)
  {
    /*
     * the response of layer 1 is expected
     */
    ma_clean_dedi_sys_buffer ();
    SET_STATE (STATE_DEDI, DEDI_ACTIVE);
    /*
     * store temporary parameters as actual channel configuration
     */
    d->act_bcch                   = d->temp_bcch;
    d->act_channel_desc           = d->temp_channel_desc;
    d->act_channel_mode           = d->temp_channel_mode;
    d->act_frequency_list         = d->temp_frequency_list;
    d->act_starting_time          = d->temp_starting_time;
    d->act_frequency_list_bef_sti = d->temp_frequency_list_bef_sti;
    d->act_channel_desc_bef_sti   = d->temp_channel_desc_bef_sti;
    d->act_pwrc                   = d->temp_pwrc;
    d->act_cipher_mode            = d->temp_cipher_mode;
    d->act_a5_algorithm           = d->temp_a5_algorithm;
    d->act_cipher_key             = d->temp_cipher_key;
    d->act_power                  = cut_power_value (alr_data->rach_data.max_tx_pwr_ccch, NULL);
    /*
     * Send a MPH_DEDICATED_CNF to RR to
     * start the establishment of the
     * layer 2 connection.
     */
    ma_dedi_cnf (DEDICATED_SUCCESS);
  }
  else
  if (GET_STATE (STATE_DEDI) EQ DEDI_IMM_ASS_RECONN)
  {
    ma_clean_dedi_sys_buffer ();
    dedi_chan_ass_fail();              /* act_mode will take on MODE_CHAN_ASS_FAIL */
    SET_STATE (STATE_DEDI, DEDI_ACTIVE);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_change_freq_cnf       |
+--------------------------------------------------------------------+

  PURPOSE : Process signal dedi_change_freq_cnf from SDL process
            Main_Control.
            Layer 1 confirms setting of the new parameters during
      a frequency redefinition procedure.
*/

GLOBAL void dedi_change_freq_cnf (void)
{
  GET_INSTANCE_DATA;
  T_DEDI_DATA *d = &alr_data->dedi_data;

  d->act_channel_desc = d->redef_channel_desc;
  d->act_frequency_list = d->redef_frequency_list;
  memset (&d->redef_channel_desc, 0, sizeof (T_channel_desc));
  memset (&d->redef_frequency_list, 0, sizeof (T_frequency_list));
  memset (&d->redef_starting_time, 0, sizeof (T_starting_time));
  ma_clean_dedi_sys_buffer ();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_chan_ass_cnf          |
+--------------------------------------------------------------------+

  PURPOSE : Process signal dedi_chan_ass_cnf from SDL process
            Main_Control.
            Layer 1 confirms setting of a new channel configuration
            during a channel assignment procedure.

*/

GLOBAL void dedi_chan_ass_cnf (void)
{
  GET_INSTANCE_DATA;
  if (GET_STATE (STATE_DEDI) EQ DEDI_ACTIVE)
  {
    if (alr_data->dedi_data.act_mode EQ MODE_CHAN_ASSIGN)
    {
      /*
       * the response of layer 1 was exspected in the
       * positive case of switching to a new channel.
       */
      T_DEDI_DATA *d = &alr_data->dedi_data;

      /*
       * store actual parameters as old parameters
       */
      d->old_channel = actual_channel;
      d->old_bcch = d->act_bcch;
      d->old_channel_desc = d->act_channel_desc;
      d->old_channel_mode = d->act_channel_mode;
      d->old_frequency_list = d->act_frequency_list;
      d->old_cipher_mode = d->act_cipher_mode;
      d->old_a5_algorithm = d->act_a5_algorithm;
      d->old_cipher_key = d->act_cipher_key;
      d->old_power = d->act_power;
      d->old_dtx_allowed = d->act_dtx_allowed;
      d->old_amr_configuration = d->act_amr_configuration;

      /*
       * store temporary parameters as actual channel configuration
       */
      actual_channel = d->temp_channel;
      d->act_bcch = d->temp_bcch;
      d->act_channel_desc = d->temp_channel_desc;
      d->act_channel_mode = d->temp_channel_mode;
      d->act_frequency_list = d->temp_frequency_list;
      d->act_cipher_mode = d->temp_cipher_mode;
      d->act_a5_algorithm = d->temp_a5_algorithm;
      d->act_cipher_key = d->temp_cipher_key;
      d->act_power = d->temp_power;
      d->act_dtx_allowed = d->temp_dtx_allowed;
      d->act_amr_configuration = d->temp_amr_configuration;
      ma_dedi_cnf (DEDICATED_SUCCESS);
    }
    else
      /*
       * the response of layer 1 was exspected in the
       * negative case of switching to the old channel.
       */
    {
      ma_dedi_fail_cnf ();
      memset (&alr_data->dedi_data.redef_channel_desc, 0, sizeof (T_channel_desc));
      memset (&alr_data->dedi_data.redef_frequency_list, 0, sizeof (T_frequency_list));
      memset (&alr_data->dedi_data.redef_starting_time, 0, sizeof (T_starting_time));
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_ho_fail_cnf           |
+--------------------------------------------------------------------+

  PURPOSE : Process signal dedi_ho_fail_cnf from SDL process
            Main_Control.
            Layer 1 signals that the handover has failed.

*/

GLOBAL void dedi_ho_fail_cnf (void)
{
  GET_INSTANCE_DATA;
  /*
   * recalculate all timing advances to the old bcch.
   */
  nc_update_list (alr_data->dedi_data.act_bcch);
  /*
   * signal dedicated failure to RR
   */
  nc_resume_dedicated();
  ma_dedi_fail_cnf ();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_fill_ma               |
+--------------------------------------------------------------------+

  PURPOSE : Fills a frequency list for layer 1.
            Condat's representation is a list of USHORT with
            0xFFFF as delimiter.
            TI's representation is different from this.

*/

LOCAL void dedi_fill_ma (USHORT * ma,
                          T_frequency_list * rf)
{
  UBYTE i = 0;

  memset (rf, 0, sizeof (T_frequency_list));

  while (ma[i] NEQ NOT_PRESENT_16BIT AND
         i < MAX_MA_CARRIER)
  {
    rf->rf_chan_no.radio_freq[i] = ma[i];
    i++;
    rf->rf_chan_cnt++;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_tch_loop_req          |
+--------------------------------------------------------------------+

  PURPOSE : Process signal dedi_tch_loop_req from SDL Process
            alr_MAIN.

*/

GLOBAL void dedi_tch_loop_req (T_MPH_TCH_LOOP_REQ *tloop)
{
  GET_INSTANCE_DATA;
  if (tloop->tch_loop EQ NOT_PRESENT_8BIT)
  {
    /*
     * Open TCH loop
     */
    ma_dedi_open_tch_loop_req ();
#ifdef FF_EM_MODE
    ALR_EM_CONFIGURE_TCH_LOOP(EM_TCH_OPEN);
#endif
  }
  else
  {
    /*
     * Close TCH loop
     */
    PALLOC (loop, OML1_CLOSE_TCH_LOOP_REQ);

    /*
     * Format of tch_loop in according to 3GPP 04.14, 8.1 CLOSE_TCH_LOOP_CMD
     */
    loop->sub_channel = alr_data->dedi_data.act_channel_desc.sub_channel;
    loop->frame_erasure = tloop->tch_loop;

    ma_dedi_close_tch_loop_req (loop);
#ifdef FF_EM_MODE
    ALR_EM_CONFIGURE_TCH_LOOP(EM_TCH_CLOSE);
#endif
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_dai_req               |
+--------------------------------------------------------------------+

  PURPOSE : Process signal dedi_dai_req from SDL Process
            alr_MAIN.
      RR requests configuration of the digital audio interface.

*/

GLOBAL void dedi_dai_req (T_MPH_DAI_REQ * dai)
{

  if (dai->device EQ DAI_NO_TEST)
  {
    /*
     * Stop DAI test
     */
    ma_dedi_stop_dai_req ();

    ALR_EM_CONFIGURE_DAI_TESTING(EM_DAI_STOP);

  }
  else
  {
    PALLOC (dai_req, OML1_START_DAI_TEST_REQ);

    /*
     * Start DAI test
     */
    dai_req->tested_device = dai->device;
    ma_dedi_start_dai_req (dai_req);

    ALR_EM_CONFIGURE_DAI_TESTING(EM_DAI_START);

  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_channel_mode          |
+--------------------------------------------------------------------+

  PURPOSE : The function converts the channel mode representation
            from Condat's notation (equal to GSM) to TI's values.

*/

LOCAL UBYTE dedi_channel_mode (UBYTE channel_mode, UBYTE channel_type)
{
  switch (channel_mode)
  {
    case MODE_SIG_ONLY:                 /* signalling only         */
      return CM_SIGNALLING_ONLY;

    case MODE_SPEECH_VER1:              /* Speech                  */
      if (channel_type EQ CH_TYPE[TCH_F_INDEX])   /* TCH fullrate  */
        return CM_TCH_FS;               /* TCH fullrate speech     */
      else
        return CM_TCH_HS;               /* TCH halfrate speech     */

    case MODE_DATA_12_0:                /* Data 12 k               */
      return CM_TCH_96;

    case MODE_DATA_6_0:                 /* Data 6 k                */
      if (channel_type EQ CH_TYPE[TCH_F_INDEX])   /* TCH fullrate  */
        return CM_TCH_F_48;             /* TCH fullrate data 6 k   */
      else
        return CM_TCH_H_48;             /* TCH halfrate data 6 k   */

    case MODE_DATA_3_6:                 /* Data 3.6 k              */
      if (channel_type EQ CH_TYPE[TCH_F_INDEX])   /* TCH fullrate  */
        return CM_TCH_F_24;             /* TCH fullrate data 3.6 k */
      else
        return CM_TCH_H_24;             /* TCH halfrate data 3.6 k */

    case MODE_DATA_14_5:                /* Data 14.5 k             */
      return CM_TCH_144;                /* TCH fullrate data 14.5 k*/

    case MODE_SPEECH_VER2:              /* enhanced full rate      */
      return CM_TCH_EFR;

    case MODE_SPEECH_VER3:              /* adaptiv multi-rate      */
      if (channel_type EQ CH_TYPE[TCH_F_INDEX])   /* TCH fullrate  */
        return CM_TCH_AFS;              /* TCH/AFS fullrate        */
      else
        return CM_TCH_AHS;              /* TCH/AHS halfrate        */

    default:
      return CM_SIGNALLING_ONLY;        /* all other set to signalling only */
  }
}

/*
+------------------------------          --------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_increment_rlt         |
+--------------------------------------------------------------------+

  PURPOSE : A valid block has received on SACCH. The radio
            link counter is incremented if possible.

*/

GLOBAL void dedi_increment_rlt (void)
{
  GET_INSTANCE_DATA;
  /*
   * increase radio link timeout counter if necessary
   */
  if (alr_data->dedi_data.act_rlt NEQ alr_data->dedi_data.rlt)
  {
    alr_data->dedi_data.act_rlt += 2;

    if (alr_data->dedi_data.act_rlt > alr_data->dedi_data.rlt)
      alr_data->dedi_data.act_rlt = alr_data->dedi_data.rlt;
#if !defined NTRACE
    trc_mon_counter_dedi (alr_data->dedi_data.act_rlt,
                          alr_data->dedi_data.rlt);
#endif /* (!defined NTRACE) */

    ALR_EM_SET_EM_ACT_RLT;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_decrement_rlt         |
+--------------------------------------------------------------------+

  PURPOSE : An invalid block has received on SACCH. The radio
            link counter is decremented. If the value is zero
            an error indication is send to RR.
*/

GLOBAL void dedi_decrement_rlt (void)
{
  GET_INSTANCE_DATA;

  /*
   * invalid block received. Decrement radio link timeout counter.
   */
  if (alr_data->dedi_data.act_rlt > 1)
  {
#if !defined NTRACE
    trc_mon_counter_dedi (alr_data->dedi_data.act_rlt,
                          alr_data->dedi_data.rlt);
#endif /* (!defined NTRACE */
    alr_data->dedi_data.act_rlt--;

    ALR_EM_SET_EM_ACT_RLT;

  }
  else
  {
    /*
     * radio link failure detected
     */
    ma_error_ind (CS_RADIO_LINK_FAIL, alr_data->serving_cell);
    alr_data->dedi_data.act_rlt = alr_data->dedi_data.rlt;
#if !defined NTRACE
    trc_mon_counter_dedi (alr_data->dedi_data.act_rlt,
                          alr_data->dedi_data.rlt);
#endif

    ALR_EM_SET_EM_ACT_RLT;

  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_convert_frequency_list|
+--------------------------------------------------------------------+

  PURPOSE : converts the channel numbers of a frequency list
            in the non-hopping case for DCS 1800 channel numbers
            in the dualband variants.
*/

LOCAL void dedi_convert_frequency_list (T_frequency_list * frequency_list)
{
  UBYTE i, j;

  i = (UBYTE)frequency_list->rf_chan_cnt;

  if (i >= 64)
    i = 63;

  ALR_TRACE_DEDI ("-----------");

  for (j = 0; j < i; j++)
  {
    ALR_TRACE_DEDI_FREQ_LIST ();

    frequency_list->rf_chan_no.radio_freq[j] =
      ARFCN_TO_L1 (frequency_list->rf_chan_no.radio_freq[j]);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_DEDI                   |
| STATE   : code                ROUTINE : dedi_stop                  |
+--------------------------------------------------------------------+

  PURPOSE : Process signal dedi_stop from SDL process Main_Control.
            This signal stops all dedicated activities of ALR.

*/

GLOBAL void dedi_stop (void)
{
  GET_INSTANCE_DATA;
  if (GET_STATE (STATE_DEDI) NEQ DEDI_INACTIVE)
  {
    /*
     * if dedicated mode is active,
     * the process and layer 1 are set to idle.
     */
    SET_STATE (STATE_DEDI, DEDI_INACTIVE);
    ma_dedi_stop_req ();
    alr_data->dedi_data.act_mode = NOT_PRESENT_8BIT;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_convert_chan_desc     |
+--------------------------------------------------------------------+

  PURPOSE : converts the channel number of a channel description
            in the non-hopping case for DCS 1800 channel numbers
            in the dualband variants.
*/

LOCAL void dedi_convert_chan_desc (T_channel_desc * channel_desc)
{
  USHORT channel;

  if (channel_desc->chan_sel.h EQ 0)
  {
    channel = (USHORT)(channel_desc->chan_sel.rf_channel.maio +
                      (channel_desc->chan_sel.rf_channel.hsn << 8));
    channel_desc->chan_sel.rf_channel.maio =
      (UBYTE)(ARFCN_TO_L1(channel) & 0xFF);
    channel_desc->chan_sel.rf_channel.hsn =
      (UBYTE)(ARFCN_TO_L1(channel) >> 8);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_conversion_needed	 |
+--------------------------------------------------------------------+

  PURPOSE : Checks whether conversion is needed or not.

*/

LOCAL UBYTE dedi_conversion_needed (void)
{
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  if ((std EQ STD_1900) OR
      (std EQ STD_850_900_1900) OR
      (std EQ STD_900_1900) OR
#else
  if ((std EQ STD_EGSM) OR
      (std EQ STD_DUAL) OR
      (std EQ STD_DUAL_EGSM) OR
#endif
      (std EQ STD_DUAL_US))
    return TRUE;
  else
    return FALSE;
}

#if defined (REL99) && defined (FF_BHO)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_reset_bho_data	 |
+--------------------------------------------------------------------+

  PURPOSE : To reset Blind handover variables in ALR context.
*/
LOCAL void dedi_reset_bho_data(void)
{
  GET_INSTANCE_DATA;
  alr_data->dedi_data.ho_type = HOT_NONE;
  alr_data->dedi_data.bho_cell_index =  NOT_PRESENT_16BIT;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_DEDI                   |
| STATE   : code                ROUTINE : dedi_set_ho_type	 |
+--------------------------------------------------------------------+

  PURPOSE : To reset Blind handover variables in ALR context.
*/
LOCAL void dedi_set_ho_type(ULONG *fn, ULONG *tn, UBYTE *ho_type)
{
  GET_INSTANCE_DATA;
  if(nc_get_fn_time (alr_data->dedi_data.temp_bcch, fn, tn))
  {
    *ho_type = HOT_REGULAR;
    alr_data->dedi_data.ho_type = HOT_REGULAR;
  }
  else
  {
    /*Setting Handover type as blind handover*/
    *ho_type = HOT_BLIND;
    alr_data->dedi_data.ho_type = HOT_BLIND;
    TRACE_EVENT("Handover Type is Blind");
  }
}
#endif


/*
+--------------------------------------------------------------------+
| PROJECT :               MODULE  : alr_DEDI                         |
| STATE     :code         ROUTINE : dedi_chan_type_sel               |
+--------------------------------------------------------------------+

  PURPOSE : Function replacing the common code from 
            "_chan_ass, _sync_handover, _pre__sync_handover &
            _freq_redef_req" functions.

*/
LOCAL void dedi_chan_type_sel (T_ch_type         *ch_type,
                               T_chan_sel        *chan_sel,
                               T_frequency_list  *frequency_list)
{
  ALR_TRACE_DEDI ("dedi_chan_type_sel");
  if (ch_type->h  EQ 1)
  {
    /*
     * with frequency hopping
     */
    chan_sel->h = 1;
    chan_sel->rf_channel.maio = ch_type->maio;
    chan_sel->rf_channel.hsn = ch_type->hsn;
    dedi_fill_ma (ch_type->ma, frequency_list);
  }
  else
  {
    /*
     * Avoid the union in the non-hopping case and
     * map arfcn to maio and hsn.
     */
    chan_sel->h = 0;
    chan_sel->rf_channel.maio = (UBYTE)((ch_type->arfcn) & 0xFF);
    chan_sel->rf_channel.hsn = (UBYTE)(((ch_type->arfcn) >> 8) & 0xFF);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT :                 MODULE  : alr_DEDI                       |
| STATE   :code             ROUTINE : dedi_chan_type_desc            |
+--------------------------------------------------------------------+

  PURPOSE : Function replacing the common code from 
            "_chan_ass, _sync_handover, _pre__sync_handover &
            _freq_redef_req" functions.

*/

LOCAL void dedi_chan_type_desc (T_MPH_DEDICATED_REQ       *dedicated_req, 
                                T_channel_desc_1_bef_sti  *channel_desc_1_bef_sti,
                                T_frequency_list          *frequency_list_bef_sti)  
{
  ALR_TRACE_DEDI ("dedi_chan_type_desc");
  if (dedicated_req->start.v_start AND
      dedicated_req->ch_type2.ch NEQ NOT_PRESENT_8BIT)
  {
    if(dedicated_req->ch_type2.h EQ 1)
    {  
      channel_desc_1_bef_sti->chan_sel.h = 1;
      channel_desc_1_bef_sti->chan_sel.rf_channel.maio = dedicated_req->ch_type2.maio;
      channel_desc_1_bef_sti->chan_sel.rf_channel.hsn  = dedicated_req->ch_type2.hsn;
      dedi_fill_ma (dedicated_req->ch_type2.ma, frequency_list_bef_sti);
    }
    else
    {
      /*
       *nAvoid the union !!!
       */
       channel_desc_1_bef_sti->chan_sel.h = 0;
       channel_desc_1_bef_sti->chan_sel.rf_channel.maio =
            (UBYTE)(dedicated_req->ch_type2.arfcn & 0xFF);
       channel_desc_1_bef_sti->chan_sel.rf_channel.hsn =
            (UBYTE)((dedicated_req->ch_type2.arfcn >> 8) & 0xFF);
    }
    channel_desc_1_bef_sti->channel_type = CH_TYPE [dedicated_req->ch_type2.ch];
    channel_desc_1_bef_sti->sub_channel  = SUB_TYPE [dedicated_req->ch_type2.ch];
    channel_desc_1_bef_sti->timeslot_no  = dedicated_req->ch_type2.tn;
    channel_desc_1_bef_sti->tsc          = dedicated_req->ch_type2.tsc;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT :                MODULE  : alr_DEDI                        |
| STATE   :code            ROUTINE : dedi_ho_cnf                     |
+--------------------------------------------------------------------+

  PURPOSE : Function replacing the common code from 
            "pre_sync, async & sync _ho_cnf" functions.

*/
GLOBAL void dedi_ho_cnf (T_HO_TYPE ho_type)
{
  GET_INSTANCE_DATA;
  T_DEDI_DATA *d = &alr_data->dedi_data;
  ALR_TRACE_DEDI ("dedi_ho_conf");
  /*
   * All timing advances are now calculated relative
   * to the new dedicated channel only in case of Regular HO.
   * In case of Blind HO, timing info will be updated when received
   * from L1 in HO_FINISHED primitive.
   */
#if defined (REL99) && defined (FF_BHO)
  if (alr_data->dedi_data.ho_type EQ HOT_REGULAR )
#endif
    nc_update_list (alr_data->dedi_data.temp_bcch);
  ma_clean_dedi_sys_buffer ();
  /*
   * store actual parameters as old parameters
   */
  d->old_channel = actual_channel;
  d->old_bcch = d->act_bcch;
  d->old_bsic = alr_data->bsic;
  d->old_channel_desc = d->act_channel_desc;
  d->old_channel_mode = d->act_channel_mode;
  d->old_frequency_list = d->act_frequency_list;
  d->old_cipher_mode = d->act_cipher_mode;
  d->old_a5_algorithm = d->act_a5_algorithm;
  d->old_cipher_key = d->act_cipher_key;
  d->old_power = d->act_power;
  d->old_amr_configuration = d->act_amr_configuration;
  /*
   * store temporary parameters as actual channel configuration
   */
  actual_channel = d->temp_channel;

  alr_data->serving_cell = d->act_bcch = d->temp_bcch;
  alr_data->sc_band = get_band(d->act_bcch);
  alr_data->bsic = alr_data->dedi_data.temp_bsic;
  d->act_channel_desc = d->temp_channel_desc;
  d->act_channel_mode = d->temp_channel_mode;
  d->act_frequency_list = d->temp_frequency_list;
  d->act_cipher_mode = d->temp_cipher_mode;
  d->act_a5_algorithm = d->temp_a5_algorithm;
  d->act_cipher_key = d->temp_cipher_key;
  d->act_power = d->temp_power;
  if (ho_type EQ HO_TYPE_SYNC)
    d->act_amr_configuration = d->temp_amr_configuration;
  else if (ho_type EQ HO_TYPE_PRE_SYNC OR ho_type EQ HO_TYPE_ASYNC)
    d->old_amr_configuration = d->act_amr_configuration;
  if (alr_data->nc_data.dtx)
    nc_update_dedicated (alr_data->nc_data.dtx,
                         alr_data->nc_data.pwrc);
#if defined (REL99) && defined (FF_BHO)
  if (alr_data->dedi_data.ho_type EQ HOT_REGULAR )
#endif
    nc_resume_dedicated();
}

/*
+--------------------------------------------------------------------+
| PROJECT :                   MODULE  : alr_DEDI                     |
| STATE   :code               ROUTINE : dedi_set_amr_conf            |
+--------------------------------------------------------------------+

  PURPOSE : Function replacing the common code for "amr_conf".

*/

LOCAL void dedi_set_amr_conf (T_amr_configuration  *amr_con, T_amr_conf *amr_conf)
{
  UBYTE i;
  ALR_TRACE_DEDI ("dedi_set_amr_conf");
  for (i=0; i<amr_conf->c_cod_prop; i++)
  {
    memcpy(&(amr_con->threshold[i]),  &amr_conf->cod_prop[i].codec_thr, 1);
    memcpy(&(amr_con->hysteresis[i]), &amr_conf->cod_prop[i].codec_hyst, 1);
    TRACE_EVENT_P2("*AMR threshold[%d] %d",  i, amr_con->threshold[i]);
    TRACE_EVENT_P2("*AMR hysteresis[%d] %d", i, amr_con->hysteresis[i]);
  }
}
#endif
