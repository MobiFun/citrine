/************* Revision Controle System Header *************
 *                  GSM Layer 1 software 
 * L1_AFUNC.C
 *
 *        Filename l1_afunc.c
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

#define  L1_AFUNC_C

#include "config.h"
#include "l1_confg.h"
#include "l1_macro.h"

#if (CODE_VERSION == SIMULATION)
  #include <string.h>
  #include "l1_types.h"
  #include "sys_types.h"
  #include "l1_const.h"
  #include "l1_signa.h"
  #if TESTMODE
    #include "l1tm_defty.h"
  #endif
  #if (AUDIO_TASK == 1)
    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_defty.h"
  #endif  
  #if (L1_GTT == 1)
    #include "l1gtt_const.h"
    #include "l1gtt_defty.h"
  #endif
  #if (L1_MIDI == 1)
    #include "l1midi_defty.h"
  #endif
  #if (L1_MP3 == 1)
    #include "l1mp3_defty.h"
  #endif
//ADDED FOR AAC
  #if (L1_AAC == 1)
    #include "l1aac_defty.h"
  #endif
  #include "l1_defty.h"
  #include "cust_os.h"
  #include "l1_msgty.h"
  #include "l1_varex.h"
  #include "l1_proto.h"
  #include "l1_tabs.h"
  #include "l1_time.h"
  #if L1_GPRS
    #include "l1p_cons.h"
    #include "l1p_msgt.h"
    #include "l1p_deft.h"
    #include "l1p_vare.h"
  #endif        
#else
  #include <string.h>
  #include "l1_types.h"
  #include "sys_types.h"
  #include "l1_const.h"
  #include "l1_signa.h"
  #include "l1_time.h"

  #if TESTMODE
    #include "l1tm_defty.h"
  #endif
  #if (AUDIO_TASK == 1)
    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_defty.h"
  #endif  
  #if (L1_GTT == 1)
    #include "l1gtt_const.h"
    #include "l1gtt_defty.h"
  #endif
  #if (L1_MP3 == 1) 
    #include "l1mp3_defty.h"
  #endif
  #if (L1_MIDI == 1)
    #include "l1midi_defty.h"
  #endif
//ADDED FOR AAC
  #if (L1_AAC == 1)
    #include "l1aac_defty.h"
  #endif
  #include "l1_defty.h"
  #include "../../gpf/inc/cust_os.h"
  #include "l1_msgty.h"
  #include "l1_varex.h"
  #include "l1_proto.h"
  #include "l1_tabs.h"
  #if L1_GPRS
    #include "l1p_cons.h"
    #include "l1p_msgt.h"
    #include "l1p_deft.h"
    #include "l1p_vare.h"
  #endif
#if (GSM_IDLE_RAM > 1)
#if (OP_L1_STANDALONE == 1)
  #include "csmi_simul.h"
#else
#include "csmi/csmi.h"
#endif
#endif
#endif


#if (OP_L1_STANDALONE == 1)
#if (ANALOG == 11)
#include "bspTwl3029_Madc.h"
#endif
#endif

#if (L1_MADC_ON == 1)
#if (OP_L1_STANDALONE == 1)
#if (RF_FAM == 61)
#include "drp_api.h"
#include "l1_rf61.h"
#include <string.h>
extern T_DRP_SRM_API* drp_srm_api;
#endif
#if (ANALOG == 11)
BspTwl3029_MadcResults l1_madc_results;
void l1a_madc_callback(void);
#endif
#endif
#endif //L1_MADC_ON



/*-------------------------------------------------------*/
/* l1a_reset_ba_list()                                   */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function resets the BA list content.             */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_reset_ba_list()
{
  UWORD8 i;
 
  // Rem: this reset function do not touch the "ba_id", "nbr_carrier" and 
  //      "radio_freq" fields.

  //!!! remove this initialization when BA list handling changed for dedic mode
  if(l1a_l1s_com.mode != I_MODE)
  {
    l1a_l1s_com.ba_list.next_to_ctrl = 0;         // Carrier for next power measurement control.
    l1a_l1s_com.ba_list.next_to_read = 0;         // Carrier for next power measurement result.
    l1a_l1s_com.ba_list.first_index  = 0;       // First BA index measured in current session.
  }

  // Reset of "ms_ctrl, ms_ctrl_d, msctrl_dd" is done at L1 startup 
  // and when SYNCHRO task is executed.
 
  l1a_l1s_com.ba_list.np_ctrl      = 0;         // PCH burst number.
    

  for(i=0; i<C_BA_PM_MEAS; i++)                            // 2 measurements / PCH frame...
  {
    l1a_l1s_com.ba_list.used_il   [i] = l1_config.params.il_min; // IL used in CTRL phase for AGC setting.  
    l1a_l1s_com.ba_list.used_il_d [i] = l1_config.params.il_min; // ... 1 frame delay.
    l1a_l1s_com.ba_list.used_il_dd[i] = l1_config.params.il_min; // ... 2 frames delay, used in READ phase.

    l1a_l1s_com.ba_list.used_lna   [i] = FALSE; // LNA used in CTRL phase for AGC setting.  
    l1a_l1s_com.ba_list.used_lna_d [i] = FALSE; // ... 1 frame delay.
    l1a_l1s_com.ba_list.used_lna_dd[i] = FALSE; // ... 2 frames delay, used in READ phase.
  }
  
  for(i=0; i<32+1; i++)
  {
    l1a_l1s_com.ba_list.A[i].acc   = 0;         // Reset IL accumulation.
  }
} 

/*-------------------------------------------------------*/
/* l1a_reset_full_list()                                 */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function resets the FULL list content.           */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_reset_full_list()
{
  UWORD16 i;

  // Init power measurement multi_session process
  l1a_l1s_com.full_list.meas_1st_pass_ctrl   = 1;     // Set 1st pass flag for power measurement session in ctrl.
  l1a_l1s_com.full_list.meas_1st_pass_read   = 1;     // Set 1st pass flag for power measurement session in read.
  l1a_l1s_com.full_list.nbr_sat_carrier_ctrl = 0;     // Clear number of saturated carrier in ctrl.
  l1a_l1s_com.full_list.nbr_sat_carrier_read = 0;     // Clear number of saturated carrier in read.
                                            
  // Set global parameters for full list measurement.
  l1a_l1s_com.full_list.next_to_ctrl = 0;             // Set next carrier to control to 1st one.
  l1a_l1s_com.full_list.next_to_read = 0;             // Set next carrier to control to 1st one.

  // Reset Pipeline
  // Note: l1a_l1s_com.full_list.ms_ctrl_d is reset at the end of l1_meas_manager()
  l1a_l1s_com.full_list.ms_ctrl_dd = 0;
  l1a_l1s_com.full_list.ms_ctrl_d  = 0;

  // Reset the FULL LIST.
  #if (L1_FF_MULTIBAND == 0)
    for(i=0; i<l1_config.std.nbmax_carrier; i++)
  #else
    for(i=0; i< NBMAX_CARRIER; i++)
  #endif
  {
    l1a_l1s_com.full_list.sat_flag[i] = 0;       // Reset sat_flag
  }

  #if L1_GPRS
    // Reset PPCH burst ctrl indication
    l1pa_l1ps_com.cr_freq_list.pnp_ctrl = 0;
  #endif
}

#if ((L1_EOTD == 1) && (L1_EOTD_QBIT_ACC == 1))
/*-------------------------------------------------------*/
/* l1a_add_time_delta()                                  */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function shifts a given cell timing (given as a  */
/* couple [time_alignmt, fn_offset]) by adding           */
/* a specified new time_alignmt offset (+ve or -ve       */
/* between -4999 and +4999 qb)                           */
/* to that timing.                                       */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_add_time_delta(UWORD32 * time_alignmt, UWORD32 * fn_offset, WORD32 delta)
{
  WORD32  new_time_alignmt = *time_alignmt + delta;
  UWORD32 new_fn_offset = *fn_offset;

  if(new_time_alignmt < 0)
  {
    new_time_alignmt += TPU_CLOCK_RANGE;
    new_fn_offset = (new_fn_offset + 1) % MAX_FN;
  }
  else if(new_time_alignmt >= TPU_CLOCK_RANGE)
  {
    new_time_alignmt -= TPU_CLOCK_RANGE;
    new_fn_offset = (new_fn_offset - 1 + MAX_FN) % MAX_FN;
  }

  *time_alignmt = new_time_alignmt;
  *fn_offset = new_fn_offset;
}

/*-------------------------------------------------------*/
/* l1a_compensate_sync_ind()                             */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* Attempts to modify the time_alignmt and fn_offset   */
/* fields of an MPHC_NCELL_SYNC_IND message based on     */
/* E-OTD cross-correlation information in order to       */
/* post-correct the result. This can be used to form a   */
/* quater-bit alignment with slow drifting neighbours   */ 
/*                                                       */
/*-------------------------------------------------------*/
void l1a_compensate_sync_ind(T_MPHC_NCELL_SYNC_IND * msg)
{

  // This process can only be applied to SBCONF messages
  // with good SCH decodes and valid EOTD results.
  //
  // a_eotd_crosscor  [0] [1] [2] [3] [4] [5] [6] [7] [8]
  //
  //                       <------ Peak Range ----->  
  //
  // As long as the cross-correlation peak lies in the range
  // [1] to [7] then we can examine the slope of the correlation
  // points on either side of the peak in order to perform a 
  // positive or negative QB shift.

  if((msg->sb_flag) && (msg->eotd_data_valid))
  {
    WORD16 peak_index  = msg->d_eotd_max - msg->d_eotd_first;

    if((peak_index >= 1) && (peak_index <= 7))
    {
      UWORD32 a_power[9];
      UWORD32 pre_power, post_power, thresh_power;
      UWORD32 i;
      WORD32  shift = 0;

      // Calculate the normalised power of the cross-correlation samples
      // in a_eotd_crosscor. This could be improved to only calculate
      // the terms for [peak_index-1] [peak_index] [peak_index+1] if
      // the algorithm proves viable in the long term.

      // Normalised power[i] = real[i]^2 + imag[i]^2

      for(i=0; i<9; ++i)
      {
        //
        // Awkward looking code to square values as our compiler / assembler 
        // gets the following construct wrong. Very strange... 
        //
        // UWORD32 real = ...
        // real *= real;        <-- Assembler allocates registers incorrectly here
        //
        
        UWORD32 real = msg->a_eotd_crosscor[2*i] * msg->a_eotd_crosscor[2*i];
        UWORD32 imag = msg->a_eotd_crosscor[(2*i)+1] * msg->a_eotd_crosscor[(2*i)+1];

        // Sum of the squares...

        a_power[i] = real + imag;
      }

      // By inspection of practical examples, it appears that (peak power/3)
      // is a good threshold on which to compare the shape of the slope.

      thresh_power = a_power[peak_index] / 3;
      pre_power    = a_power[peak_index-1]; 
      post_power   = a_power[peak_index+1];
 
      // Decision on whether the gradient of the slope of the crosscor points
      // on either side of the peak is large enough to cause a (max) +/- 1QB shift
      // to the time_alignmt field.

      if( (pre_power < thresh_power) && (post_power > thresh_power) )
      {
        // Right skew on the cross corrrelation - shift time_alignmt 
        // to be one greater

        shift = 1;
      }
      else if ( (pre_power > thresh_power) && (post_power < thresh_power) )
      {
        // Left skew on the cross correlation - shift time_alignmt 
        // to be one less

        shift = -1;
      }

      l1a_add_time_delta( &(msg->time_alignmt),
                          &(msg->fn_offset),
                          shift );

    }
  }
}
#endif

/*-------------------------------------------------------*/
/* l1a_add_time_for_nb()                                 */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function shift a given cell timing (given as a   */
/* couple [time_alignmt, fn_offset]) by adding           */
/*             "SB_MARGIN - NB_MARGIN"                   */
/* to that timing.                                       */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_add_time_for_nb(UWORD32 *time_alignmt, UWORD32 *fn_offset)
{
  // Add "SB_MARGIN - NB_MARGIN" qbit to "fn_offset" and "time_alignmt".
  // Pay attention to the modulos.

  *time_alignmt += (SB_MARGIN - NB_MARGIN); 
  if(*time_alignmt >= TPU_CLOCK_RANGE)
  {
    *time_alignmt -= TPU_CLOCK_RANGE;
    *fn_offset     = (*fn_offset + MAX_FN - 1) % MAX_FN;
  }
}

/*-------------------------------------------------------*/
/* l1a_add_timeslot()                                    */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function shift a given cell timing (given as a   */
/* couple [time_alignmt, fn_offset]) by adding a number  */
/* of TIMESLOT (given as "tn") to that timing.           */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_add_timeslot(UWORD32 *time_alignmt, UWORD32 *fn_offset, UWORD8 tn)
{             
  // Add "tn" timeslot to "fn_offset" and "time_alignmt".
  // Pay attention to the modulos.
  
  *time_alignmt += tn * BP_DURATION;
  if(*time_alignmt >= TPU_CLOCK_RANGE)
  {
    *time_alignmt -= TPU_CLOCK_RANGE;
    *fn_offset     = (*fn_offset + MAX_FN - 1) % MAX_FN; 
  }
}

/*-------------------------------------------------------*/
/* l1a_sub_time_for_nb()                                 */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function shift a given cell timing (given as a   */
/* couple [time_alignmt, fn_offset]) by substacting      */
/*             "SB_MARGIN - NB_MARGIN"                   */
/* to that timing.                                       */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_sub_time_for_nb(UWORD32 *time_alignmt, UWORD32 *fn_offset)
{
  WORD32 new_time_alignmt;
  
  // Sub "SB_MARGIN - NB_MARGIN" qbit to "fn_offset" and "time_alignmt".
  // Pay attention to the modulos.

  new_time_alignmt = *time_alignmt - (SB_MARGIN - NB_MARGIN); 
  if(new_time_alignmt < 0)
  {
    new_time_alignmt += TPU_CLOCK_RANGE;
    *fn_offset        = (*fn_offset + 1) % MAX_FN; 
  }
  *time_alignmt = new_time_alignmt;
}

/*-------------------------------------------------------*/
/* l1a_sub_timeslot()                                    */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function shift a given cell timing (given as a   */
/* couple [time_alignmt, fn_offset]) by substracting a   */
/* number of TIMESLOT (given as "tn") to that timing.    */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_sub_timeslot(UWORD32 *time_alignmt, UWORD32 *fn_offset, UWORD8 tn)
{             
  WORD32 new_time_alignmt;

  // Sub "tn" timeslot to "fn_offset" and "time_alignmt".
  // Pay attention to the modulos.
  new_time_alignmt = *time_alignmt - (tn * BP_DURATION);
  if(new_time_alignmt < 0)
  {
    new_time_alignmt += TPU_CLOCK_RANGE;
    *fn_offset        = (*fn_offset + 1) % MAX_FN; 
  }
  *time_alignmt = new_time_alignmt;
}
  
/*-------------------------------------------------------*/
/* l1a_correct_timing()                                  */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/*                                                       */
/*-------------------------------------------------------*/
#if (L1_12NEIGH == 1)
void l1a_correct_timing (UWORD8 neigh_id,UWORD32 time_alignmt,UWORD32 fn_offset)
{
  // Save timing information in case of future handovers.
  l1a_l1s_com.nsync.list[neigh_id].time_alignmt_mem = time_alignmt;
  l1a_l1s_com.nsync.list[neigh_id].fn_offset_mem    = fn_offset;      

  // Sub the serving cell timeslot number to the Neigh./Serving timing 
  // difference to format it for L1S use.
  l1a_sub_timeslot(&time_alignmt, &fn_offset, l1a_l1s_com.dl_tn);
  l1a_sub_time_for_nb(&time_alignmt, &fn_offset);

  // Save neighbor information in the neighbor confirmation cell structure.
  l1a_l1s_com.nsync.list[neigh_id].time_alignmt = time_alignmt;
  l1a_l1s_com.nsync.list[neigh_id].fn_offset    = fn_offset;
}
#endif

/*-------------------------------------------------------*/
/* l1a_compute_Eotd_data()                               */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/*                                                       */
/*-------------------------------------------------------*/
#if ((L1_12NEIGH ==1) && (L1_EOTD == 1))
void l1a_compute_Eotd_data(  UWORD8 *first_scell, UWORD8 neigh_id, UWORD32 SignalCode, xSignalHeaderRec *msg)
{
  WORD32   ta_sb_neigh; 
  UWORD32  fn_sb_neigh;
  WORD16   d_eotd_first;
  WORD32   toa_correction;
  UWORD32  timetag;

  // SB case .....
  if (SignalCode == L1C_SB_INFO)
  {
    fn_sb_neigh = ((T_L1C_SB_INFO *)(msg->SigP))->fn_sb_neigh;
    d_eotd_first= ((T_L1C_SB_INFO *)(msg->SigP))->d_eotd_first;
    toa_correction = ((T_L1C_SB_INFO *)(msg->SigP))->toa_correction;
  }
  // SBCONF case .....
  else 
  {
    fn_sb_neigh = ((T_L1C_SBCONF_INFO *)(msg->SigP))->fn_sb_neigh;
    d_eotd_first= ((T_L1C_SBCONF_INFO *)(msg->SigP))->d_eotd_first;
    toa_correction = ((T_L1C_SBCONF_INFO *)(msg->SigP))->toa_correction;
  }

  // compute the true Serving/Neighbor time difference.
  //  1) update time_alignmt with (23bit - d_eotd_first) delta
  //  2) Add the serving cell timeslot number to the Serving/Neighbor time difference.
  ta_sb_neigh  = l1a_l1s_com.nsync.list[neigh_id].time_alignmt;
  ta_sb_neigh  += (d_eotd_first - (23))*4 +
                  (l1a_l1s_com.dl_tn * 625);

  // for Serving cell, timetag reference is 0
  if (*first_scell == TRUE)
  {
    l1a_l1s_com.nsync.fn_sb_serv = fn_sb_neigh;
    l1a_l1s_com.nsync.ta_sb_serv = ta_sb_neigh;

    timetag = 0;
  }
  else
  {
    UWORD32  delta_fn;
    WORD32   delta_qbit;

    delta_fn = (fn_sb_neigh - l1a_l1s_com.nsync.fn_sb_serv + MAX_FN)%MAX_FN;
    delta_qbit = ta_sb_neigh - l1a_l1s_com.nsync.ta_sb_serv;

    // Set timetag 
    timetag = (delta_fn*5000) + (WORD32)(delta_qbit) + toa_correction;

    #if (CODE_VERSION == SIMULATION)
      #if  (TRACE_TYPE==5) 
        ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->delta_fn   = delta_fn;    
        ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->delta_qbit = delta_qbit;    
      #endif
    #endif
  }
  // Set timetag
  ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->timetag = timetag;    

}
#endif

/*-------------------------------------------------------*/
/* l1a_get_free_dedic_set()                              */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
T_DEDIC_SET *l1a_get_free_dedic_set()
{
  T_DEDIC_SET *fset;
  UWORD8      i;
  
  // Get free set pointer.
  if(l1a_l1s_com.dedic_set.aset == &(l1a_l1s_com.dedic_set.set[0]))
    fset = &(l1a_l1s_com.dedic_set.set[1]);
  else
    fset = &(l1a_l1s_com.dedic_set.set[0]);
     
  // Clear free set.
  fset->achan_ptr                        = NULL;
  fset->chan1.desc.channel_type          = INVALID_CHANNEL;
  fset->chan1.desc_bef_sti.channel_type  = INVALID_CHANNEL;
  fset->chan2.desc.channel_type          = INVALID_CHANNEL;
  fset->chan2.desc_bef_sti.channel_type  = INVALID_CHANNEL;

  fset->ma.alist_ptr                     = NULL;
  fset->ma.freq_list.rf_chan_cnt         = 0;
  fset->ma.freq_list_bef_sti.rf_chan_cnt = 0;
  
  // Starting time.
  fset->serv_sti_fn = -1;
  fset->neig_sti_fn = -1;
  
  // Frequency redefinition flag.
  fset->freq_redef_flag                 = FALSE;

  // Timing Advance
  fset->timing_advance                  = 0;
  fset->new_timing_advance              = 0;

  // TXPWR
  fset->new_target_txpwr                = NO_TXPWR;

  // Cell Information
  l1a_reset_cell_info(&(fset->cell_desc));

  // Cipering.
  fset->a5mode                          = 0; // Ciphering OFF.

  // Clear O&M test variables.
  fset->dai_mode                        = 0; // No DAI test.
  fset->chan1.tch_loop                  = 0; // No TCH loop on chan1.
  fset->chan2.tch_loop                  = 0; // No TCH loop on chan2.
  
  // For handover...
  fset->ho_acc                          = 0;
  fset->ho_acc_to_send                  = 0;
  fset->t3124                           = 0;

#if ((REL99 == 1) && (FF_BHO == 1))
  // For blind handover...
  fset->report_time_diff                = FALSE;
  fset->nci                             = FALSE;
  fset->report_time_diff                = FALSE;
  fset->real_time_difference            = 0;
  fset->HO_SignalCode                   = 0;
#endif

  // Reset DPAGC fifo
  for(i=0;i<DPAGC_FIFO_LEN;i++)
  {
    fset->G_all[i]    = 200;
    fset->G_DTX[i]    = 200;
  }

  // Reset DTX_ALLOWED field.
  fset->dtx_allowed = FALSE;

  #if IDS
  // clear ids_mode: default value = speech mode
    fset->ids_mode = 0;
  #endif

#if (AMR == 1)
  // Clear the AMR ver 1.0 network settings
  fset->amr_configuration.noise_suppression_bit         = FALSE;
  fset->amr_configuration.initial_codec_mode_indicator  = FALSE;
  fset->amr_configuration.initial_codec_mode            = 0;
  fset->amr_configuration.active_codec_set              = 0;
  fset->amr_configuration.threshold[0]                  = 0;
  fset->amr_configuration.threshold[1]                  = 0;
  fset->amr_configuration.threshold[2]                  = 0;
  fset->amr_configuration.hysteresis[0]                 = 0;
  fset->amr_configuration.hysteresis[1]                 = 0;
  fset->amr_configuration.hysteresis[2]                 = 0;
  fset->cmip                                            = C_AMR_CMIP_DEFAULT;
#endif
  return(fset);
}

/*-------------------------------------------------------*/
/* l1a_fill_bef_sti_param()                              */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1a_fill_bef_sti_param(T_DEDIC_SET *set_ptr, BOOL start_time_present)
{
  if(start_time_present == TRUE)
  // There is a STARTING TIME field...
  {
    if((set_ptr->ma.freq_list_bef_sti.rf_chan_cnt != 0) || 
       (set_ptr->chan1.desc_bef_sti.channel_type != INVALID_CHANNEL) ||
       (set_ptr->chan2.desc_bef_sti.channel_type != INVALID_CHANNEL))
    // There is at least one "bef_sti" parameter given for this channel.  
    // Other empty parameters must be filled with the according "AFTER STARTING TIME" parameters.
    {
      // Fill "chan1.desc_bef_sti"
      if(set_ptr->chan1.desc_bef_sti.channel_type == INVALID_CHANNEL)
        set_ptr->chan1.desc_bef_sti = set_ptr->chan1.desc;
        
      // Fill "chan2.desc_bef_sti"
      if(set_ptr->chan2.desc_bef_sti.channel_type == INVALID_CHANNEL)
        set_ptr->chan2.desc_bef_sti = set_ptr->chan2.desc;

      // Fill "freq_list_bef_sti"
      if(set_ptr->ma.freq_list_bef_sti.rf_chan_cnt == 0)
        set_ptr->ma.freq_list_bef_sti = set_ptr->ma.freq_list;
    }
  }
}

/*-------------------------------------------------------*/
/* l1a_decode_starting_time()                            */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
WORD32 l1a_decode_starting_time(T_STARTING_TIME coded_starting_time)
{
  WORD32 starting_time; 
  
  if(coded_starting_time.start_time_present == TRUE )
  // A starting time is present.
  // ---------------------------
  {
    WORD32  tp1 = coded_starting_time.start_time.n32;
    WORD32  t2  = coded_starting_time.start_time.n26;
    WORD32  t3  = coded_starting_time.start_time.n51;

    // Compute STI. 
    starting_time = 51*((26 + t3 - t2) % 26) + t3 + (51*26*tp1) ;
  }
  else
  {
    starting_time = -1;
  }
  
  return(starting_time);
}

/*-------------------------------------------------------*/
/* l1a_reset_cell_info()                                 */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1a_reset_cell_info(T_CELL_INFO *cell_info)
{
  cell_info->bsic           = 0;         
  cell_info->fn_offset      = 0;    
  cell_info->time_alignmt   = 0; 
  cell_info->meas.acc       = 0;
  cell_info->meas.nbr_meas  = 0; 
  cell_info->attempt_count  = 0;
  cell_info->si_bit_map     = 0;

  cell_info->traffic_meas.input_level        = l1_config.params.il_min;
  cell_info->traffic_meas_beacon.input_level = l1_config.params.il_min;
  cell_info->traffic_meas.lna_off            = FALSE;
  cell_info->traffic_meas_beacon.lna_off     = FALSE;

  cell_info->buff_beacon[0] = cell_info->buff_beacon[1] = cell_info->buff_beacon[2] =
    cell_info->buff_beacon[3] = l1_config.params.il_min;
  
  #if L1_GPRS
    cell_info->transfer_meas.input_level        = l1_config.params.il_min;
    cell_info->transfer_meas.lna_off            = FALSE;
    cell_info->pb                               = 0;
  #endif
}

/*-------------------------------------------------------*/
/* l1a_send_confirmation()                               */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1a_send_confirmation(UWORD32 SignalCode,  UWORD8 queue_type)
{
  xSignalHeaderRec *msg;

  msg = os_alloc_sig(0);
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = (int)SignalCode;

  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
    l1_trace_message(msg);
  #endif


  #if (OP_L1_STANDALONE == 1)
  os_send_sig(msg, queue_type);  
  #else
 os_send_sig(msg, ((T_ENUM_OS_QUEUE)queue_type));   //omaps00090550
  #endif

 
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}

/*-------------------------------------------------------*/
/* l1a_send_result()                                     */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1a_send_result(UWORD32 SignalCode, xSignalHeaderRec *msg, UWORD8 queue)
{
  // Set flag to avoid the FREE(msg) in L1ASYNC.
  l1a.l1_msg_forwarded = TRUE;
  
  msg->SignalCode = (int)SignalCode;

  // May not be necessary -> to check

  #if (GSM_IDLE_RAM > 1)
    if (!READ_TRAFFIC_CONT_STATE)
      {
        CSMI_TrafficControllerOn();
      }
  #endif

  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
    l1_trace_message(msg);
  #endif
   
  #if (OP_L1_STANDALONE == 1)
  os_send_sig(msg, queue);  
  #else
os_send_sig(msg, ((T_ENUM_OS_QUEUE)queue));    //omaps00090550
   #endif


  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}

/*-------------------------------------------------------*/
/* l1a_encode_rxqual()                                   */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
UWORD8 l1a_encode_rxqual (UWORD32 inlevel)
{
  enum qual_thr
  {
    thr_0_2  =   4,
    thr_0_4  =   8,
    thr_0_8  =  16,
    thr_1_6  =  32,
    thr_3_2  =  64,
    thr_6_4  = 128,
    thr_12_8 = 256
  };

  UWORD8 rxqual;
  
  if (inlevel < thr_0_2)  rxqual = 0;
  else
    if (inlevel < thr_0_4)  rxqual = 1;
    else
      if (inlevel < thr_0_8)  rxqual = 2;
      else
        if (inlevel < thr_1_6)  rxqual = 3;
        else
          if (inlevel < thr_3_2)  rxqual = 4;
          else
            if (inlevel < thr_6_4)  rxqual = 5;
            else
              if (inlevel < thr_12_8)  rxqual = 6;
              else rxqual = 7;

  return((UWORD8) rxqual);
}

/*-------------------------------------------------------*/
/* l1a_report_failling_ncell_sync()                      */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1a_report_failling_ncell_sync(UWORD32 SignalCode, UWORD8 neigh_id)
{
  xSignalHeaderRec *msg;

  // Send MPHC_NCELL_SYNC_IND message to L3 with a FAILLURE indication.
  msg = os_alloc_sig(sizeof(T_MPHC_NCELL_SYNC_IND));
  DEBUGMSG(status,NU_ALLOC_ERR)
  
  msg->SignalCode = SignalCode;
  ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->sb_flag      = FALSE;
  ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->radio_freq   = l1a_l1s_com.nsync.list[neigh_id].radio_freq;
  ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->bsic         = 0;
  ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->fn_offset    = 0;
  ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->time_alignmt = 0;
  
  // For trace/debug only
  ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->neigh_id     = neigh_id;

  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
    l1_trace_message(msg);
  #endif

  os_send_sig(msg, RRM1_QUEUE);  
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}


/*-------------------------------------------------------*/
/* l1a_clip_txpwr()                                      */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
#if (L1_FF_MULTIBAND == 0)

UWORD8 l1a_clip_txpwr (UWORD8 supplied_txpwr, UWORD16 radio_freq)
{
  #define txpwr_to_compare(pwr) ((pwr<=l1_config.std.txpwr_turning_point)? pwr+32:pwr)

  switch(l1_config.std.id)
  {
    case GSM:
    case GSM_E:
    {
      // Clip LOW according to powerclass_band1.
      if ( supplied_txpwr < MIN_TXPWR_GSM[l1a_l1s_com.powerclass_band1])
        return(MIN_TXPWR_GSM[l1a_l1s_com.powerclass_band1]);

      // Clip HIGH according to GSM spec. 05.05.
      if ( supplied_txpwr > l1_config.std.max_txpwr_band1)
        return(l1_config.std.max_txpwr_band1);
    }
    break;
    
    case PCS1900:
    {
      // Clip LOW according to powerclass_band1.
      if ( txpwr_to_compare(supplied_txpwr) < 
           txpwr_to_compare(MIN_TXPWR_PCS[l1a_l1s_com.powerclass_band1]) )
        return(MIN_TXPWR_PCS[l1a_l1s_com.powerclass_band1]);

      // Clip HIGH according to GSM spec. 05.05.
      if ( txpwr_to_compare(supplied_txpwr) > 
           txpwr_to_compare(l1_config.std.max_txpwr_band1) )
        return(l1_config.std.max_txpwr_band1);
    }
    break;
    
    case DCS1800:
    {
      // Clip LOW according to powerclass_band1.
      if ( txpwr_to_compare(supplied_txpwr) < 
           txpwr_to_compare(MIN_TXPWR_DCS[l1a_l1s_com.powerclass_band1]) )
        return(MIN_TXPWR_DCS[l1a_l1s_com.powerclass_band1]);

      // Clip HIGH according to GSM spec. 05.05.
      if ( txpwr_to_compare(supplied_txpwr) > 
           txpwr_to_compare(l1_config.std.max_txpwr_band1) )
        return(l1_config.std.max_txpwr_band1);
    }
    break;
    
    case GSM850:
    {
      // Clip LOW according to powerclass_band1.
      if ( txpwr_to_compare(supplied_txpwr) < 
           txpwr_to_compare(MIN_TXPWR_GSM850[l1a_l1s_com.powerclass_band1]) )
        return(MIN_TXPWR_GSM850[l1a_l1s_com.powerclass_band1]);

      // Clip HIGH according to GSM spec. 05.05.
      if ( txpwr_to_compare(supplied_txpwr) > 
           txpwr_to_compare(l1_config.std.max_txpwr_band1) )
        return(l1_config.std.max_txpwr_band1);
    }
    break;

    case DUAL:
    case DUALEXT:
    {
      // Test which Band is used: GSM or DCS 1800
      if (radio_freq >= l1_config.std.first_radio_freq_band2)
      {
        // Clip LOW according to powerclass_band1.
        if ( txpwr_to_compare(supplied_txpwr) < 
             txpwr_to_compare(MIN_TXPWR_DCS[l1a_l1s_com.powerclass_band2]) )
          return(MIN_TXPWR_DCS[l1a_l1s_com.powerclass_band2]);

        // Clip HIGH according to GSM spec. 05.05.
        if ( txpwr_to_compare(supplied_txpwr) > 
             txpwr_to_compare(l1_config.std.max_txpwr_band2) )
          return(l1_config.std.max_txpwr_band2);
      }
      else
      {
        // Clip LOW according to powerclass_band1.
        if ( supplied_txpwr < MIN_TXPWR_GSM[l1a_l1s_com.powerclass_band1])
          return(MIN_TXPWR_GSM[l1a_l1s_com.powerclass_band1]);

        // Clip HIGH according to GSM spec. 05.05.
        if ( supplied_txpwr > l1_config.std.max_txpwr_band1)
          return(l1_config.std.max_txpwr_band1);
      }
    }
    break;
    

  case DUAL_US:
    {
      // Test which Band is used: GSM 850 or PCS1900 
      if (radio_freq >= l1_config.std.first_radio_freq_band2)
      {
        // Clip LOW according to powerclass_band1.
        if ( txpwr_to_compare(supplied_txpwr) < 
             txpwr_to_compare(MIN_TXPWR_PCS[l1a_l1s_com.powerclass_band2]) )
          return(MIN_TXPWR_PCS[l1a_l1s_com.powerclass_band2]);

        // Clip HIGH according to GSM spec. 05.05.
        if ( txpwr_to_compare(supplied_txpwr) > 
             txpwr_to_compare(l1_config.std.max_txpwr_band2) )
          return(l1_config.std.max_txpwr_band2);
      }
      else
      {
        // Clip LOW according to powerclass_band1.
        if ( supplied_txpwr < MIN_TXPWR_GSM850[l1a_l1s_com.powerclass_band1])
          return(MIN_TXPWR_GSM850[l1a_l1s_com.powerclass_band1]);

        // Clip HIGH according to GSM spec. 05.05.
        if ( supplied_txpwr > l1_config.std.max_txpwr_band1)
          return(l1_config.std.max_txpwr_band1);
      }
    }
    break;
  
  
    default: // should never occur
    {
      return(supplied_txpwr);   
    }
 // omaps00090550     break;
  }
  return(supplied_txpwr);  
}

#else /*L1_FF_MULTIBAND = 1 below */

UWORD8 l1a_clip_txpwr (UWORD8 supplied_txpwr, UWORD16 radio_freq)
{


  UWORD8 physical_band_id = 0;
  physical_band_id = l1_multiband_radio_freq_convert_into_physical_band_id(radio_freq);
  #define txpwr_to_compare(pwr) ((pwr<= multiband_rf[physical_band_id].tx_turning_point)? pwr+32:pwr)
  switch(multiband_rf[physical_band_id].gsm_band_identifier)
  {
    case RF_GSM900:
    {

      // Clip LOW according to powerclass_band1.
      if ( supplied_txpwr < MIN_TXPWR_GSM[l1a_l1s_com.powerclass[physical_band_id]])
        return(MIN_TXPWR_GSM[l1a_l1s_com.powerclass[physical_band_id]]);

      // Clip HIGH according to GSM spec. 05.05.
      if ( supplied_txpwr > multiband_rf[physical_band_id].max_txpwr)
        return(multiband_rf[physical_band_id].max_txpwr);
      break; 
    }/*case GSM900*/
    
    case RF_PCS1900:
    {
      // Clip LOW according to powerclass_band1.
      if ( txpwr_to_compare(supplied_txpwr) <
           txpwr_to_compare(MIN_TXPWR_PCS[l1a_l1s_com.powerclass[physical_band_id]]) )
        return(MIN_TXPWR_PCS[l1a_l1s_com.powerclass[physical_band_id]]);

      // Clip HIGH according to GSM spec. 05.05.
      if ( txpwr_to_compare(supplied_txpwr) >
           txpwr_to_compare(multiband_rf[physical_band_id].max_txpwr) )
        return(multiband_rf[physical_band_id].max_txpwr);
      break;
    }/*case PCS1900*/
    
    case RF_DCS1800:
    {
      // Clip LOW according to powerclass_band1.
      if ( txpwr_to_compare(supplied_txpwr) <
           txpwr_to_compare(MIN_TXPWR_DCS[l1a_l1s_com.powerclass[physical_band_id]]) )
        return(MIN_TXPWR_DCS[l1a_l1s_com.powerclass[physical_band_id]]);

      // Clip HIGH according to GSM spec. 05.05.
      if ( txpwr_to_compare(supplied_txpwr) >
           txpwr_to_compare(multiband_rf[physical_band_id].max_txpwr) )
        return(multiband_rf[physical_band_id].max_txpwr);
      break;
    }/*case DCS1800*/

    case RF_GSM850:
    {
      // Clip LOW according to powerclass_band1.
      if ( txpwr_to_compare(supplied_txpwr) <
           txpwr_to_compare(MIN_TXPWR_GSM850[l1a_l1s_com.powerclass[physical_band_id]]) )
        return(MIN_TXPWR_GSM850[l1a_l1s_com.powerclass[physical_band_id]]);

      // Clip HIGH according to GSM spec. 05.05.
      if ( txpwr_to_compare(supplied_txpwr) >
           txpwr_to_compare(multiband_rf[physical_band_id].max_txpwr) )
        return(multiband_rf[physical_band_id].max_txpwr);
      break;
    }/*case GSM850*/

    default: // should never occur
    {
      l1_multiband_error_handler();  
      return(supplied_txpwr);
       break;
    } /*default*/
   
  }/*switch(multiband_rfphysical_band_id].gsm_band_identifier)*/
  return(supplied_txpwr);

}

#endif /*L1_FF_MULTIBAND */


//MADC 

#if (L1_MADC_ON == 1)
#if (OP_L1_STANDALONE == 1)
#if (ANALOG == 11)
void   l1a_madc_callback(void)
{
	char str[40];



   xSignalHeaderRec *adc_msg;
   UWORD16 *adc_result;
   UWORD16 *madc_results;
   volatile UWORD16 *drp_temp_results;
   
#if (RF_FAM == 61)
      drp_temp_results =(volatile UWORD16 *) (&drp_srm_api->inout.temperature.output); //omaps00090550
#endif

#if 0
    sprintf(str, "Temp Measure %x  ", drp_temp_results);

    //L1_trace_string ("Temp Meas\n");
    L1_trace_string(str);
#endif

#if 0
   int i;

   adc_msg = os_alloc_sig(sizeof(BspTwl3029_MadcResults) + sizeof(UWORD16));
   adc_result = &((BspTwl3029_MadcResults*)(adc_msg->SigP))->adc1;
   madc_results =& l1_madc_results.adc1;

   //TEMP_MEAS: DRP
#if (RF_FAM == 61)
      drp_temp_results =& (drp_srm_api->inout.temperature.output);
#endif

   //copy the measured values into the the message structure.
   memcpy(adc_result,madc_results,11*sizeof(UWORD16));//11 madc
   adc_result[11] = *drp_temp_results; // 1 temp meas
   /*
   for (i=0;i<11;i++)
   	adc_result[i] = madc_results[i];
   */
   //Send the message
   adc_msg->SignalCode = CST_ADC_RESULT;
   os_send_sig(adc_msg, RRM1_QUEUE);
#endif
}
#endif // ANALOG == 11
#endif //OP_L1_STANDALONE
#endif // L1_MADC_ON

//==============================================================================================

