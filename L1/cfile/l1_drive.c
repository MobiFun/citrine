/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_DRIVE.C
 *
 *        Filename l1_drive.c
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

#define  L1_DRIVE_C

#include "config.h"
#include "l1_confg.h"

#if (RF_FAM == 61)
#include "apc.h"
#endif

#if ((W_A_WAIT_DSP_RESTART_AFTER_VOCODER_ENABLE ==1)&&(W_A_DSP_PR20037 == 1))
  #include "../../nucleus/nucleus.h"
#endif
#include "l1_macro.h"
#if (CODE_VERSION == SIMULATION)
  #include <string.h>
  #include "l1_types.h"
  #include "sys_types.h"
  #include "l1_const.h"
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
  #include "l1_varex.h"
  #include "cust_os.h"
  #include "l1_msgty.h"
  #if TESTMODE
    #include "l1tm_varex.h"
  #endif
  #if L2_L3_SIMUL
    #include "hw_debug.h"
  #endif

  #if L1_GPRS
    #include "l1p_cons.h"
    #include "l1p_msgt.h"
    #include "l1p_deft.h"
    #include "l1p_vare.h"
    #include "l1p_sign.h"
  #endif

  #include <stdio.h>
  #include "sim_cfg.h"
  #include "sim_cons.h"
  #include "sim_def.h"
  #include "sim_var.h"

  #include "l1_ctl.h"

#else

  #include <string.h>
  #include "l1_types.h"
  #include "sys_types.h"
  #include "l1_const.h"
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
  #include "l1_varex.h"
  #include "../../gpf/inc/cust_os.h"
  #include "l1_msgty.h"
  #if TESTMODE
    #include "l1tm_varex.h"
  #endif
  #if L2_L3_SIMUL
    #include "hw_debug.h"
  #endif
  #include "tpudrv.h"

  #if L1_GPRS
    #include "l1p_cons.h"
    #include "l1p_msgt.h"
    #include "l1p_deft.h"
    #include "l1p_vare.h"
    #include "l1p_sign.h"
  #endif

  #include "l1_ctl.h"

#endif

#if (RF_FAM == 61)
  #include "tpudrv61.h"
#endif



/*
 * Prototypes of external functions used in this file.
 *
 * FreeCalypso change: removed all those prototypes which appear
 * in tpudrv.h, and kept only the additional ones.
 */

#if ((REL99 == 1) && (FF_BHO == 1))
#if (L1_MADC_ON == 1)
  void  l1dmacro_rx_fbsb (SYS_UWORD16 radio_freq,UWORD8 adc_active);
#else
  void  l1dmacro_rx_fbsb (SYS_UWORD16 radio_freq);
#endif
#endif//#if ((REL99 == 1) && (FF_BHO == 1))

void Cust_get_ramp_tab(API *a_ramp, UWORD8 txpwr_ramp_up, UWORD8 txpwr_ramp_down, UWORD16 radio_freq);
#if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3) || (RF_FAM == 61))
  UWORD16 Cust_get_pwr_data(UWORD8 txpwr, UWORD16 radio_freq
  										  #if(REL99 && FF_PRF)
  										  ,UWORD8 number_uplink_timeslot
  										  #endif
  										  );
#endif

#if L1_GPRS
  void l1ps_reset_db_mcu_to_dsp(T_DB_MCU_TO_DSP_GPRS *page_ptr);
#endif

/*-------------------------------------------------------*/
/* l1ddsp_load_info()                                    */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1ddsp_load_info(UWORD32 task, API *info_ptr, UWORD8 *data)
{
  if(task == RACH_DSP_TASK)
  // RACH info. format is only 2 words...
  {
    info_ptr[0] = ((API)(data[0])) | ((API)(data[1])<<8);
  }
  else
  // Fill mcu-dsp comm. buffer.
  {
    UWORD8 i,j;

    // Fill data block Header...
    info_ptr[0] = (1 << B_BLUD);     // 1st word: Set B_BLU bit.
    info_ptr[1] = 0;                 // 2nd word: cleared.
    info_ptr[2] = 0;                 // 3rd word: cleared.

    if((info_ptr == l1s_dsp_com.dsp_ndb_ptr->a_du_0) ||
       (info_ptr == l1s_dsp_com.dsp_ndb_ptr->a_du_1))
    // DATA traffic buffers: size of buffer is 260 bit -> 17 words but DATA traffic uses
    // only a max of 240 bit (30 bytes) -> 15 words.
    {
      for (i=0, j=(3+0); j<(3+15); j++)
      {
        info_ptr[j] = ((API)(data[i])) | ((API)(data[i+1]) << 8);
        i += 2;
      }
      #if (TRACE_TYPE==3)
        if (l1_stats.type == PLAY_UL)
        {
          for (i=0, j=(3+0); j<(3+17); j++)
          {
            info_ptr[j] = ((API)(data[i])) |
                          ((API)(data[i]) << 8);
            i ++;
          }
        }
     #endif
    }
    else
    // Data block for control purpose is 184 bit length: 23 bytes: 12 words (16 bit/word).
    {
      // Copy first 22 bytes in the first 11 words after header.
      for (i=0, j=(3+0); j<(3+11); j++)
      {
        info_ptr[j] = ((API)(data[i])) | ((API)(data[i+1]) << 8);
        i += 2;
      }

      // Copy last UWORD8 (23rd) in the 12th word after header.
      info_ptr[14] = data[22];
    }
  }
}

/*-------------------------------------------------------*/
/* l1ddsp_load_monit_task()                              */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1ddsp_load_monit_task(API monit_task, API fb_mode)
{
  l1s_dsp_com.dsp_db_w_ptr->d_task_md   = monit_task;  // Write number of measurements

  if(l1a_l1s_com.mode == CS_MODE)
    l1s_dsp_com.dsp_ndb_ptr->d_fb_mode  = fb_mode;     // Write FB detection algo. mode.
  else
    l1s_dsp_com.dsp_ndb_ptr->d_fb_mode  = 1;
}


/* --------------------------------------------------- */

/* Locosto Changes....*/

// Parameters: UWORD16 afcval
// Return       :Void
//Functionality: TPU to accept the afcvalue from the MCU and copy it
//to the mem_xtal before triggering of AFC Script in DRP

#if(RF_FAM == 61)
void l1dtpu_load_afc(UWORD16 afc)
{
   l1dmacro_afc(afc, 0);
}

/* --------------------------------------------------- */

/* Locosto Changes....*/

/* Parameters: API dco_algo_ctl_sb */
/* Functionality: Loads the API  d_dco_ctl_algo_sb in the API
     This should be called after updating the value in the API via cust_Get_dco_algo_ctl(...) */

/* --------------------------------------------------- */

void l1ddsp_load_dco_ctl_algo_sb (UWORD16 dco_ctl_algo)
{
#if (DSP == 38) || (DSP == 39)
  l1s_dsp_com.dsp_db_common_w_ptr->d_dco_algo_ctrl_sb = (API) dco_ctl_algo;
#endif
}

/* --------------------------------------------------- */

/* Locosto Changes....*/

/* Parameters: API dco_algo_ctl_nb */
/* Functionality: Loads the API  d_dco_ctl_algo_nb in the API
     This should be called after updating the value in the API via cust_Get_dco_algo_ctl(...) */

/* --------------------------------------------------- */

void l1ddsp_load_dco_ctl_algo_nb (UWORD16 dco_ctl_algo)
{
#if (DSP == 38) || (DSP == 39)
  l1s_dsp_com.dsp_db_common_w_ptr->d_dco_algo_ctrl_nb = (API) dco_ctl_algo;
#endif

}
/* --------------------------------------------------- */

/* Locosto Changes....*/

/* Parameters: API dco_algo_ctl_pw  */
/* Functionality: Loads the API  d_dco_ctl_algo_pw in the API
     This should be called after updating the value in the API via cust_Get_dco_algo_ctl(...) */

/* --------------------------------------------------- */

void l1ddsp_load_dco_ctl_algo_pw (UWORD16 dco_ctl_algo)
{
#if (DSP == 38) || (DSP == 39)
  l1s_dsp_com.dsp_db_common_w_ptr->d_dco_algo_ctrl_pw = (API) dco_ctl_algo;
#endif
}

#endif

/*-------------------------------------------------------*/
/* l1ddsp_load_afc()                                     */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1ddsp_load_afc(API afc)
{
#if (L1_EOTD==1)
  // NEW !!! For EOTD measurements in IDLE mode only, cut AFC updates....
  #if (L1_GPRS)
   if ( (l1a_l1s_com.nsync.eotd_meas_session == FALSE) ||
        (l1a_l1s_com.mode == DEDIC_MODE)||
        (l1a_l1s_com.l1s_en_task[PDTCH] == TASK_ENABLED))
  #else
   if ( (l1a_l1s_com.nsync.eotd_meas_session == FALSE) ||
        (l1a_l1s_com.mode == DEDIC_MODE))
  #endif

  {
#endif
    //######################## For DSP Rom #################################
    l1s_dsp_com.dsp_db_w_ptr->d_afc = afc;                      // Write new afc command.
    #if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3) || (RF_FAM == 61))
    // NOTE: In Locosto AFC loading is w.r.t DRP not in ABB
      l1s_dsp_com.dsp_db_w_ptr->d_ctrl_abb |= (1 << B_AFC);     // Validate new afc value.
    #endif
 #if (L1_EOTD==1)
 }
#endif

}

/*-------------------------------------------------------*/
/* l1ddsp_load_txpwr()                                   */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*Notes:

While Programming APC Ramp always Program the APCDEL also
Cal+:
APCDEL1: LSB Dwn(9:5):Up(4:0)
APCDEL2: MSB Dwn(9:5):Up(4:0)

Locosto:
APCDEL1: LSB Dwn(9:5):Up(4:0)
APCDEL2: MSB Dwn(9:5):Up(4:0)
-----
Cal+
APCRAM : Dwn(51:11)Up(10:6)Forced(0)
Locosto:
APCRAM: Dwn(9:5)Up(4:0)

For AFC, APCDEL1, APCDEL2, APCRAMP the Control word d_ctl_abb is checked
i f they are reqd to be updated.

For AUXAPC (Cal+), the last bit = 1 would mean the DSP would pick it at Tx
For APCLEV (Loc), it is picked at every Tx, for dummy burst DSP would make it 0
*/

/*-------------------------------------------------------*/
void l1ddsp_load_txpwr(UWORD8 txpwr, UWORD16 radio_freq)
{
  #if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3) || (RF_FAM == 61))
    UWORD16 pwr_data;
  #endif

  //config
  if (l1_config.tx_pwr_code ==0)
  {
    // Fixed TXPWR.
    l1s_dsp_com.dsp_db_w_ptr->d_power_ctl = l1_config.params.fixed_txpwr;     // GSM management disabled: Fixed TXPWR used.


    #if(RF_FAM == 61)
    //Locosto has new API for Ramp
       #if (DSP == 38) || (DSP == 39)
         Cust_get_ramp_tab(l1s_dsp_com.dsp_ndb_ptr->a_drp_ramp, txpwr, txpwr, radio_freq);
       #endif
    #else
       #if (CODE_VERSION != SIMULATION)
    /*** Reference to real ramp array (GSM: 15 power levels, 5-19, DCS: 16 power levels, 0-15) ***/
       Cust_get_ramp_tab(l1s_dsp_com.dsp_ndb_ptr->a_ramp, txpwr, txpwr, radio_freq);
       #endif
    #endif

    #if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3))
      l1s_dsp_com.dsp_db_w_ptr->d_ctrl_abb |= ( (1 << B_RAMP) | (1 << B_BULRAMPDEL) | (1 << B_BULRAMPDEL2));
    #endif

   #if(RF_FAM == 61)
      l1s_dsp_com.dsp_db_w_ptr->d_ctrl_abb |= ( (1 << B_RAMP) | (1 << B_BULRAMPDEL) | (1 << B_BULRAMPDEL2));
   #endif

  }
  else
  {
    static UWORD8 last_used_freq_band = 0;
    UWORD8 freq_band;

#if (L1_FF_MULTIBAND == 0)
    // Check whether band has changed
    // This will be used to reload ramps
    if ((l1_config.std.id == DUAL)     ||
        (l1_config.std.id == DUALEXT) ||
        (l1_config.std.id == DUAL_US))
    {
      if (radio_freq < l1_config.std.first_radio_freq_band2)
        freq_band = BAND1;
      else
        freq_band = BAND2;
    }
    else
      freq_band = BAND1;
#else

freq_band = l1_multiband_radio_freq_convert_into_effective_band_id(radio_freq);

#endif 

    // Note: txpwr = NO_TXPWR is reserved for forcing the transmitter off
    // -----           (to suppress SACCH during handover, for example)

    /*** Check to see if the TXPWR is to be suppressed (txpwr = NO_TXPWR)   ***/

    if(txpwr == NO_TXPWR)
    {
       /*** No transmit ***/
       #if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3))
         l1s_dsp_com.dsp_db_w_ptr->d_power_ctl = 0x12; // AUXAPC initialization addr 9 pg 0 Omega
         l1s_dsp_com.dsp_db_w_ptr->d_ctrl_abb |= ( (1 << B_RAMP) | (1 << B_BULRAMPDEL)  | (1 << B_BULRAMPDEL2));
       #endif

       #if(RF_FAM == 61 ) //Locosto without Syren Format
           l1s_dsp_com.dsp_db_w_ptr->d_power_ctl = (API) 0; // APCLEV
           l1s_dsp_com.dsp_db_w_ptr->d_ctrl_abb |= ( (1 << B_RAMP) | (1 << B_BULRAMPDEL)  | (1 << B_BULRAMPDEL2));
       #endif

       l1s.last_used_txpwr = NO_TXPWR;
       return;
    }
    else
    {
      /*** Get power data according to clipped TXPWR ***/
      pwr_data = Cust_get_pwr_data(txpwr, radio_freq
      									  #if(REL99 && FF_PRF)
      									  ,1
      									  #endif
      									  );

      /*** Load power control level adding the APC address register ***/
      #if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3))
        l1s_dsp_com.dsp_db_w_ptr->d_power_ctl = ((pwr_data << 6) | 0x12);
        // AUXAPC initialization addr 9 pg 0 Omega
      #endif

      #if(RF_FAM == 61)
        l1s_dsp_com.dsp_db_w_ptr->d_power_ctl = (API)(pwr_data);
      #endif
    }

      #if TESTMODE
       #if(RF_FAM == 61)
         // Currently for RF_FAM=61 Enabling APC-Ramp, APCDEL1 and APCDEL2 writing always i.e. in every TDMA frame
	 // TODO: Check whether this is okay
         if ((l1_config.TestMode) && (l1_config.tmode.rf_params.down_up & TMODE_UPLINK))
       #else
         if ((l1_config.TestMode) && (l1_config.tmode.rf_params.down_up & TMODE_UPLINK) &&
            ((l1s.last_used_txpwr != txpwr) || (l1_config.tmode.rf_params.reload_ramps_flag)))
       #endif
        {
          #if(RF_FAM == 61)
            #if (DSP == 38) || (DSP == 39)
              Cust_get_ramp_tab(l1s_dsp_com.dsp_ndb_ptr->a_drp_ramp, txpwr, txpwr, radio_freq);
            #endif
          #else
		    #if (CODE_VERSION != SIMULATION)
	    Cust_get_ramp_tab(l1s_dsp_com.dsp_ndb_ptr->a_ramp, txpwr, txpwr, radio_freq);
		    #endif
          #endif

          #if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3))
	    // Setting bit 3 of this register causes DSP to write to APCDEL1 register in Omega. However,
            // we are controlling this register from MCU through the SPI. Therefore, set it to 0.
            l1s_dsp_com.dsp_db_w_ptr->d_ctrl_abb |= ( (1 << B_RAMP) | (0 << B_BULRAMPDEL)  | (1 << B_BULRAMPDEL2));
          #endif

          #if (RF_FAM == 61)
            l1s_dsp_com.dsp_db_w_ptr->d_ctrl_abb |= ( (1 << B_RAMP) | (1 << B_BULRAMPDEL)  | (1 << B_BULRAMPDEL2));
          #endif

          l1s.last_used_txpwr = txpwr;
          l1_config.tmode.rf_params.reload_ramps_flag = 0;
        }
       else
      #endif

      if ((l1s.last_used_txpwr != txpwr) || (last_used_freq_band != freq_band))
      {
        /*** Power level or band has changed, so update the ramp, and trigger the data send to ABB ***/

        l1s.last_used_txpwr = txpwr;
        last_used_freq_band = freq_band;

        /*** Reference to real ramp array (GSM: 15 power levels, 5-19, DCS: 16 power levels, 0-15) ***/
      #if(RF_FAM == 61)
       #if (DSP == 38) || (DSP == 39)
          Cust_get_ramp_tab(l1s_dsp_com.dsp_ndb_ptr->a_drp_ramp, txpwr, txpwr, radio_freq);
       #endif
      #else
       #if (CODE_VERSION != SIMULATION)
          Cust_get_ramp_tab(l1s_dsp_com.dsp_ndb_ptr->a_ramp, txpwr, txpwr, radio_freq);
       #endif
      #endif

      #if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3) ||(RF_FAM == 61))
        l1s_dsp_com.dsp_db_w_ptr->d_ctrl_abb |= ( (1 << B_RAMP) | (1 << B_BULRAMPDEL)  | (1 << B_BULRAMPDEL2));
      #endif
    }
  }
}

#if (FF_L1_FAST_DECODING == 1)
/*-------------------------------------------------------*/
/* l1ddsp_load_fp_task()                                 */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1ddsp_load_fast_dec_task(API task, UWORD8 burst_id)
{
  if (l1s_check_fast_decoding_authorized(task))
  {
    //l1s_dsp_com.dsp_db_w_ptr->d_fast_paging_ctrl = 0x0001
    l1s_dsp_com.dsp_db_common_w_ptr->d_fast_paging_ctrl =  0x00001;
    if(burst_id == BURST_1)
    {
      l1s_dsp_com.dsp_db_common_w_ptr->d_fast_paging_ctrl |=  0x8000;
    }
  }
}
#endif /* FF_L1_FAST_DECODING */

/*-------------------------------------------------------*/
/* l1ddsp_load_rx_task()                                 */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1ddsp_load_rx_task(API rx_task, UWORD8 burst_id, UWORD8 tsq)
{
  l1s_dsp_com.dsp_db_w_ptr->d_task_d       = rx_task;      // Write RX task Identifier.
  l1s_dsp_com.dsp_db_w_ptr->d_burst_d      = burst_id;     // Write RX burst Identifier.
  l1s_dsp_com.dsp_db_w_ptr->d_ctrl_system |= tsq << B_TSQ; // Write end of task DSP state.
}

/*-------------------------------------------------------*/
/* l1ddsp_load_tx_task()                                 */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1ddsp_load_tx_task(API tx_task, UWORD8 burst_id, UWORD8 tsq)
{
  l1s_dsp_com.dsp_db_w_ptr->d_task_u       = tx_task;      // write TX task Identifier.
  l1s_dsp_com.dsp_db_w_ptr->d_burst_u      = burst_id;     // write TX burst Identifier.
  l1s_dsp_com.dsp_db_w_ptr->d_ctrl_system |= tsq << B_TSQ; // Write end of task DSP state.
}

/*-------------------------------------------------------*/
/* l1ddsp_load_ra_task()                                 */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1ddsp_load_ra_task(API ra_task)
{
  l1s_dsp_com.dsp_db_w_ptr->d_task_ra   = ra_task;   // write RA task Identifier.
}

/*-------------------------------------------------------*/
/* l1ddsp_load_tch_mode()                                */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1ddsp_load_tch_mode(UWORD8 dai_mode, BOOL dtx_allowed)
{
  // TCH mode register.
  //        bit[0]      -> b_eotd.
  //        bit[1]      -> b_audio_async only for WCP
  //        bit [2]     -> b_dtx.
  //        bit[3]      -> play_ul when set to 1
  //        bit[4]      -> play_dl when set to 1
  //        bit[5]      -> DTX selection for voice memo
  //        bit[6]      -> Reserved for ciphering debug
  //        bit[7..10]  -> Reserved for ramp up control
  //        bit[11]     -> Reserved for analog device selection

  #if (DSP == 32)
    UWORD16 mask = 0xfffb;
  #else // NO OP_WCP
    UWORD16 mask = 0xfff8;
  #endif

  l1s_dsp_com.dsp_ndb_ptr->d_tch_mode = (l1s_dsp_com.dsp_ndb_ptr->d_tch_mode & mask)
                                        | (dtx_allowed<<2);
#if (L1_EOTD == 1)
  l1s_dsp_com.dsp_ndb_ptr->d_tch_mode |= B_EOTD;
#endif
}

#if (AMR == 1)
  /*-------------------------------------------------------*/
  /* l1ddsp_load_tch_param()                               */
  /*-------------------------------------------------------*/
  /* Parameters :                                          */
  /* Return     :                                          */
  /* Functionality :                                       */
  /*-------------------------------------------------------*/
#if (FF_L1_TCH_VOCODER_CONTROL == 1)
  void l1ddsp_load_tch_param(T_TIME_INFO *next_time, UWORD8 chan_mode,
                             UWORD8       chan_type, UWORD8 subchannel,
                             UWORD8       tch_loop,  UWORD8 sync_tch,
                             UWORD8       sync_amr,
                             UWORD8       reset_sacch,
                           #if !FF_L1_IT_DSP_DTX
                             UWORD8       vocoder_on)
			   #else
                             UWORD8       vocoder_on,
                             BOOL         dtx_dsp_interrupt)
                           #endif
#else
  void l1ddsp_load_tch_param(T_TIME_INFO *next_time, UWORD8 chan_mode,
                             UWORD8       chan_type, UWORD8 subchannel,
                             UWORD8       tch_loop,  UWORD8 sync_tch,
                           #if !FF_L1_IT_DSP_DTX
                             UWORD8       sync_amr)
                           #else
                             UWORD8       sync_amr,  BOOL   dtx_dsp_interrupt)
                           #endif
#endif
  {
    UWORD32  count_0;
    UWORD32  count_1;
    UWORD32  d_ctrl_tch;
    UWORD32  d_fn;

    // d_ctrl_tch
    // ----------
    //   bit [0..3]   -> b_chan_mode
    //   bit [4..7]   -> b_chan_type
    //   bit [8]      -> b_sync_tch_ul
    //   bit [9]      -> b_sync_amr
    //   bit [10]     -> b_stop_tch_ul
    //   bit [11]     -> b_stop_tch_dl
    //   bit [12..14] -> b_tch_loop
    //   bit [15]     -> b_subchannel
  #if (FF_L1_TCH_VOCODER_CONTROL == 1)
    d_ctrl_tch = (chan_mode<<B_CHAN_MODE)  | (chan_type<<B_CHAN_TYPE)  | (subchannel<<B_SUBCHANNEL) |
                 (sync_tch<<B_SYNC_TCH_UL) | (sync_amr<<B_SYNC_AMR)    |
                 (tch_loop<<B_TCH_LOOP) | (reset_sacch<<B_RESET_SACCH) | (vocoder_on<<B_VOCODER_ON);
  #else
    d_ctrl_tch = (chan_mode<<B_CHAN_MODE)  | (chan_type<<B_CHAN_TYPE)  | (subchannel<<B_SUBCHANNEL) |
                 (sync_tch<<B_SYNC_TCH_UL) | (sync_amr<<B_SYNC_AMR)    |
                 (tch_loop<<B_TCH_LOOP);
  #endif

    // d_fn
    // ----
    //   bit [0..7]  -> b_fn_report
    //   bit [8..15] -> b_fn_sid
    d_fn = (next_time->fn_in_report) | ((next_time->fn%104)<<8);

    // a_a5fn
    // ------
    //   count_0 (a_a5fn[0]), bit [0..4]  -> T2.
    //   count_0 (a_a5fn[1]), bit [5..10] -> T3.
    //   count_1 (a_a5fn[0]), bit [0..10] -> T1.
    count_0 = ((UWORD16)next_time->t3 << 5) | (next_time->t2);
    count_1 = (next_time->t1);

    l1s_dsp_com.dsp_db_w_ptr->d_fn         = d_fn;         // write both Fn_sid, Fn_report.
    l1s_dsp_com.dsp_db_w_ptr->a_a5fn[0]    = count_0;      // cyphering FN part 1.
    l1s_dsp_com.dsp_db_w_ptr->a_a5fn[1]    = count_1;      // cyphering FN part 2.
    l1s_dsp_com.dsp_db_w_ptr->d_ctrl_tch   = d_ctrl_tch;   // Channel config.
  #if FF_L1_IT_DSP_DTX
    // ### TBD: report this block below in the other instance of this function
    // DTX interrupt request is latched by DSP in TDMA3 (TCH-AFS, TCH-AHS0) or TDMA0 (TCH-AHS1)
    if ((chan_mode == TCH_AFS_MODE) || (chan_mode == TCH_AHS_MODE))
    {
      if (((next_time->fn_mod13_mod4 == 3) &&
           ((chan_mode == TCH_AFS_MODE) || ((subchannel == 0)))) ||
          ((next_time->fn_mod13_mod4 == 0) &&
           ((chan_mode == TCH_AHS_MODE) && (subchannel == 1)))
         )
      {
        if (dtx_dsp_interrupt)
          l1s_dsp_com.dsp_ndb_ptr->d_fast_dtx_enable=1;
        else
          l1s_dsp_com.dsp_ndb_ptr->d_fast_dtx_enable=0;

      }
    }
    // Fast DTX not supported
    else
    {
      // No interrupt genaration
      l1s_dsp_com.dsp_ndb_ptr->d_fast_dtx_enable=0;
    }
  #endif
  }
#else
  /*-------------------------------------------------------*/
  /* l1ddsp_load_tch_param()                               */
  /*-------------------------------------------------------*/
  /* Parameters :                                          */
  /*  Return     :                                         */
  /* Functionality :                                       */
  /*-------------------------------------------------------*/
#if (FF_L1_TCH_VOCODER_CONTROL == 1)
  void l1ddsp_load_tch_param(T_TIME_INFO *next_time,   UWORD8 chan_mode,
                             UWORD8       chan_type,   UWORD8 subchannel,
                             UWORD8       tch_loop,    UWORD8 sync_tch,
                           #if !FF_L1_IT_DSP_DTX
                             UWORD8       reset_sacch, UWORD8 vocoder_on)
			   #else
                             UWORD8       reset_sacch, UWORD8 vocoder_on,
                             BOOL         dtx_dsp_interrupt)
                           #endif
#else
  void l1ddsp_load_tch_param(T_TIME_INFO *next_time, UWORD8 chan_mode,
                             UWORD8       chan_type, UWORD8 subchannel,
                           #if !FF_L1_IT_DSP_DTX
                             UWORD8       tch_loop,  UWORD8 sync_tch)
                           #else
                             UWORD8       tch_loop,  UWORD8 sync_tch,
                             BOOL         dtx_dsp_interrupt)
                           #endif
#endif
  {
    UWORD32  count_0;
    UWORD32  count_1;
    UWORD32  d_ctrl_tch;
    UWORD32  d_fn;

    // d_ctrl_tch
    // ----------
    //   bit [0..3]   -> b_chan_mode
    //   bit [4..7]   -> b_chan_type
    //   bit [8]      -> b_sync_tch_ul
    //   bit [9]      -> b_sync_tch_dl
    //   bit [10]     -> b_stop_tch_ul
    //   bit [11]     -> b_stop_tch_dl
    //   bit [12..14] -> b_tch_loop
    //   bit [15]     -> b_subchannel
  #if (FF_L1_TCH_VOCODER_CONTROL == 1)
    d_ctrl_tch = (chan_mode<<B_CHAN_MODE)  | (chan_type<<B_CHAN_TYPE)  | (subchannel<<B_SUBCHANNEL) |
                 (sync_tch<<B_SYNC_TCH_UL) | (sync_tch<<B_SYNC_TCH_DL) |
                 (tch_loop<<B_TCH_LOOP) | (reset_sacch<<B_RESET_SACCH) | (vocoder_on<<B_VOCODER_ON);
  #else
    d_ctrl_tch = (chan_mode<<B_CHAN_MODE)  | (chan_type<<B_CHAN_TYPE)  | (subchannel<<B_SUBCHANNEL) |
                 (sync_tch<<B_SYNC_TCH_UL) | (sync_tch<<B_SYNC_TCH_DL) |
                 (tch_loop<<B_TCH_LOOP);
  #endif

    // d_fn
    // ----
    //   bit [0..7]  -> b_fn_report
    //   bit [8..15] -> b_fn_sid
    d_fn = (next_time->fn_in_report) | ((next_time->fn%104)<<8);

    // a_a5fn
    // ------
    //   count_0 (a_a5fn[0]), bit [0..4]  -> T2.
    //   count_0 (a_a5fn[1]), bit [5..10] -> T3.
    //   count_1 (a_a5fn[0]), bit [0..10] -> T1.
    count_0 = ((UWORD16)next_time->t3 << 5) | (next_time->t2);
    count_1 = (next_time->t1);

    l1s_dsp_com.dsp_db_w_ptr->d_fn         = d_fn;         // write both Fn_sid, Fn_report.
    l1s_dsp_com.dsp_db_w_ptr->a_a5fn[0]    = count_0;      // cyphering FN part 1.
    l1s_dsp_com.dsp_db_w_ptr->a_a5fn[1]    = count_1;      // cyphering FN part 2.
    l1s_dsp_com.dsp_db_w_ptr->d_ctrl_tch   = d_ctrl_tch;   // Channel config.
  }
#endif

#if (L1_VOCODER_IF_CHANGE == 0)
// TODO: to be moved in API file (see BUG3093)
BOOL enable_tch_vocoder(BOOL vocoder)
{
  #if (FF_L1_TCH_VOCODER_CONTROL == 1)
    // To enable the vocoder, we set the trigger => then handled in l1s_dedicated_mode_manager
    #if (W_A_DSP_PR20037 == 1)
      if ((vocoder==TRUE) && (l1a_l1s_com.dedic_set.start_vocoder == TCH_VOCODER_DISABLED))
      {
        l1a_l1s_com.dedic_set.start_vocoder = TCH_VOCODER_ENABLE_REQ;

        #if ( W_A_WAIT_DSP_RESTART_AFTER_VOCODER_ENABLE ==1)
          NU_Sleep(DSP_VOCODER_ON_TRANSITION); // DSP transition
        #endif
      }
      // When vocoder_on = FALSE, vocoder module is not executed
      else if ((vocoder==FALSE) && (l1a_l1s_com.dedic_set.start_vocoder == TCH_VOCODER_ENABLED))
      {
        l1a_l1s_com.dedic_set.start_vocoder = TCH_VOCODER_DISABLE_REQ;
      }
    #else // W_A_DSP_PR20037 == 0
      if (vocoder)
      {
        l1a_l1s_com.dedic_set.start_vocoder = TRUE;
      }
      // When vocoder_on = FALSE, vocoder module is not executed
      else
      {
        l1a_l1s_com.dedic_set.vocoder_on = FALSE;
      }
    #endif // W_A_DSP_PR20037

    return TRUE;
  #else
    return FALSE;
  #endif
  }
#endif // L1_VOCODER_IF_CHANGE
BOOL l1_select_mcsi_port(UWORD8 port)
{
  #if (  (CHIPSET == 12) && (RF_FAM != 61) )
    l1s_dsp_com.dsp_ndb_ptr->d_mcsi_select = (API)port;
    return TRUE;
  #else
    return FALSE;
  #endif
}

// TODO: to be moved in API file

/*-------------------------------------------------------*/
/* l1ddsp_load_ciph_param()                              */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1ddsp_load_ciph_param(UWORD8 a5mode,
                            T_ENCRYPTION_KEY      *ciph_key)
{
  // Store ciphering mode (0 for no ciphering) in MCU-DSP com.
  l1s_dsp_com.dsp_ndb_ptr->d_a5mode = a5mode;  // A5 algorithm (0 for none).
  // Store ciphering key.

#if (L1_A5_3 == 1)

  if(a5mode == 3)
  {
  #if(OP_L1_STANDALONE != 1)
    l1s_dsp_com.dsp_ndb_ptr->a_a5_kc[0]  = (ciph_key->A[0]) | (ciph_key->A[1] << 8);
    l1s_dsp_com.dsp_ndb_ptr->a_a5_kc[1]  = (ciph_key->A[2]) | (ciph_key->A[3] << 8);
    l1s_dsp_com.dsp_ndb_ptr->a_a5_kc[2]  = (ciph_key->A[4]) | (ciph_key->A[5] << 8);
    l1s_dsp_com.dsp_ndb_ptr->a_a5_kc[3]  = (ciph_key->A[6]) | (ciph_key->A[7] << 8);
    l1s_dsp_com.dsp_ndb_ptr->a_a5_kc[4]  = (ciph_key->A[8]) | (ciph_key->A[9] << 8);
    l1s_dsp_com.dsp_ndb_ptr->a_a5_kc[5]  = (ciph_key->A[10]) | (ciph_key->A[11] << 8);
    l1s_dsp_com.dsp_ndb_ptr->a_a5_kc[6]  = (ciph_key->A[12]) | (ciph_key->A[13] << 8);
    l1s_dsp_com.dsp_ndb_ptr->a_a5_kc[7]  = (ciph_key->A[14]) | (ciph_key->A[15] << 8);
  #else // (OP_L1_STANDALONE == 1)
    l1s_dsp_com.dsp_ndb_ptr->a_a5_kc[0]  = (ciph_key->A[0]) | (ciph_key->A[1] << 8);
    l1s_dsp_com.dsp_ndb_ptr->a_a5_kc[1]  = (ciph_key->A[2]) | (ciph_key->A[3] << 8);
    l1s_dsp_com.dsp_ndb_ptr->a_a5_kc[2]  = (ciph_key->A[4]) | (ciph_key->A[5] << 8);
    l1s_dsp_com.dsp_ndb_ptr->a_a5_kc[3]  = (ciph_key->A[6]) | (ciph_key->A[7] << 8);
    l1s_dsp_com.dsp_ndb_ptr->a_a5_kc[4]  = (ciph_key->A[8]) | (ciph_key->A[1] << 8);
    l1s_dsp_com.dsp_ndb_ptr->a_a5_kc[5]  = (ciph_key->A[10]) | (ciph_key->A[3] << 8);
    l1s_dsp_com.dsp_ndb_ptr->a_a5_kc[6]  = (ciph_key->A[12]) | (ciph_key->A[5] << 8);
    l1s_dsp_com.dsp_ndb_ptr->a_a5_kc[7]  = (ciph_key->A[14]) | (ciph_key->A[7] << 8);
  #endif
  }
  else // a5mode == 1 or 2
  {
    l1s_dsp_com.dsp_ndb_ptr->a_a5_kc[0]  = (ciph_key->A[0]) | (ciph_key->A[1] << 8);
    l1s_dsp_com.dsp_ndb_ptr->a_a5_kc[1]  = (ciph_key->A[2]) | (ciph_key->A[3] << 8);
    l1s_dsp_com.dsp_ndb_ptr->a_a5_kc[2]  = (ciph_key->A[4]) | (ciph_key->A[5] << 8);
    l1s_dsp_com.dsp_ndb_ptr->a_a5_kc[3]  = (ciph_key->A[6]) | (ciph_key->A[7] << 8);
  }

#else

  l1s_dsp_com.dsp_ndb_ptr->a_kc[0]  = (ciph_key->A[0]) | (ciph_key->A[1] << 8);
  l1s_dsp_com.dsp_ndb_ptr->a_kc[1]  = (ciph_key->A[2]) | (ciph_key->A[3] << 8);
  l1s_dsp_com.dsp_ndb_ptr->a_kc[2]  = (ciph_key->A[4]) | (ciph_key->A[5] << 8);
  l1s_dsp_com.dsp_ndb_ptr->a_kc[3]  = (ciph_key->A[6]) | (ciph_key->A[7] << 8);

#endif
}

/*-------------------------------------------------------*/
/* l1ddsp_stop_tch()                                     */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1ddsp_stop_tch(void)
{
  // Tch channel description.
  //        bit [10]    -> b_stop_tch_ul,  stop TCH/UL.
  //        bit [11]    -> b_stop_tch_dl,  stop TCH/DL.

  l1s_dsp_com.dsp_db_w_ptr->d_ctrl_tch |= 3 << B_STOP_TCH_UL;
}

/*-------------------------------------------------------*/
/* l1ddsp_meas_read()                                    */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1ddsp_meas_read(UWORD8 nbmeas, UWORD16 *pm)
{
  UWORD8 i;

  for (i= 0; i < nbmeas; i++)
  {
    pm[i] = ((l1s_dsp_com.dsp_db_r_ptr->a_pm[i] & 0xffff));
  }

  #if TESTMODE
    if(l1_config.TestMode)
      l1tm.tmode_stats.pm_recent = l1s_dsp_com.dsp_db_r_ptr->a_pm[0] & 0xffff;
  #endif
}

#if (AMR == 1)
  /*-------------------------------------------------------*/
  /* l1ddsp_load_amr_param()                               */
  /*-------------------------------------------------------*/
  /* Parameters : AMR configuration                        */
  /* Return     : none                                     */
  /* Functionality : Download the AMR configuration to the */
  /*                 DSP via API                           */
  /*-------------------------------------------------------*/
  void l1ddsp_load_amr_param(T_AMR_CONFIGURATION amr_param, UWORD8 cmip)
  {
    // Clear the AMR API buffer
    l1s_dsp_com.dsp_ndb_ptr->a_amr_config[0] = (API)0;
    l1s_dsp_com.dsp_ndb_ptr->a_amr_config[1] = (API)0;
    l1s_dsp_com.dsp_ndb_ptr->a_amr_config[2] = (API)0;
    l1s_dsp_com.dsp_ndb_ptr->a_amr_config[3] = (API)0;

    // Set the AMR parameters
    l1s_dsp_com.dsp_ndb_ptr->a_amr_config[NSCB_INDEX]   |= (API)((amr_param.noise_suppression_bit & NSCB_MASK ) << NSCB_SHIFT);
    l1s_dsp_com.dsp_ndb_ptr->a_amr_config[ICMUL_INDEX]  |= (API)((amr_param.initial_codec_mode & ICM_MASK ) << ICMUL_SHIFT);
    l1s_dsp_com.dsp_ndb_ptr->a_amr_config[ICMDL_INDEX]  |= (API)((amr_param.initial_codec_mode & ICM_MASK ) << ICMDL_SHIFT);
    l1s_dsp_com.dsp_ndb_ptr->a_amr_config[ICMIUL_INDEX] |= (API)((amr_param.initial_codec_mode_indicator & ICMI_MASK ) << ICMIUL_SHIFT);
    l1s_dsp_com.dsp_ndb_ptr->a_amr_config[ICMIDL_INDEX] |= (API)((amr_param.initial_codec_mode_indicator & ICMI_MASK ) << ICMIDL_SHIFT);
    l1s_dsp_com.dsp_ndb_ptr->a_amr_config[ACSUL_INDEX]  |= (API)((amr_param.active_codec_set & ACS_MASK ) << ACSUL_SHIFT);
    l1s_dsp_com.dsp_ndb_ptr->a_amr_config[ACSDL_INDEX]  |= (API)((amr_param.active_codec_set & ACS_MASK ) << ACSDL_SHIFT);
    l1s_dsp_com.dsp_ndb_ptr->a_amr_config[THR1_INDEX]   |= (API)((amr_param.threshold[0] & THR_MASK ) << THR1_SHIFT);
    l1s_dsp_com.dsp_ndb_ptr->a_amr_config[THR2_INDEX]   |= (API)((amr_param.threshold[1] & THR_MASK ) << THR2_SHIFT);
    l1s_dsp_com.dsp_ndb_ptr->a_amr_config[THR3_INDEX]   |= (API)((amr_param.threshold[2] & THR_MASK ) << THR3_SHIFT);
    l1s_dsp_com.dsp_ndb_ptr->a_amr_config[HYST1_INDEX]  |= (API)((amr_param.hysteresis[0] & HYST_MASK ) << HYST1_SHIFT);
    l1s_dsp_com.dsp_ndb_ptr->a_amr_config[HYST2_INDEX]  |= (API)((amr_param.hysteresis[1] & HYST_MASK ) << HYST2_SHIFT);
    l1s_dsp_com.dsp_ndb_ptr->a_amr_config[HYST3_INDEX]  |= (API)((amr_param.hysteresis[2] & HYST_MASK ) << HYST3_SHIFT);
    l1s_dsp_com.dsp_ndb_ptr->a_amr_config[CMIP_INDEX]   |= (API)((cmip & CMIP_MASK ) << CMIP_SHIFT);
  }
#endif

#if (L1_SAIC != 0)
  /*-------------------------------------------------------*/
  /* l1ddsp_load_swh_flag()                                */
  /*-------------------------------------------------------*/
  /* Parameters : SWH (Spatial Whitening) Flag             */
  /* Return     : none                                     */
  /* Functionality : To write the d_swh_ApplyWhitening flag*/
  /*-------------------------------------------------------*/
  void l1ddsp_load_swh_flag (UWORD16 SWH_flag, UWORD16 SAIC_flag)
  {

    if(SAIC_flag)
    {
      l1s_dsp_com.dsp_db_common_w_ptr->d_swh_ctrl_db = SAIC_ENABLE_DB;
     if(SWH_flag)
     {
        l1s_dsp_com.dsp_db_common_w_ptr->d_swh_ctrl_db |= (0x01<< B_SWH_DB);
     }
    }
    else
    {
      l1s_dsp_com.dsp_db_common_w_ptr->d_swh_ctrl_db = 0;
    }
  }
#endif

/*-------------------------------------------------------*/
/* l1ddsp_end_scenario()                                 */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1ddsp_end_scenario(UWORD8 type)
{
  #if (CODE_VERSION == SIMULATION)
    #if (AUDIO_SIMULATION)
      switch(type)
      {
        case GSM_CTL:
        case GSM_MISC_CTL:
        // a DSP control for a GSM task or
        // a DSP control for a GSM and a MISC tasks
        //-----------------------------
        {
          // set DSP_ENB and DSP_PAG for communication interrupt
          l1s_tpu_com.reg_cmd->dsp_pag_bit = l1s_dsp_com.dsp_w_page;
          l1s_tpu_com.reg_cmd->dsp_enb_bit = ON;

          // change DSP page pointer for next controle
          l1s_dsp_com.dsp_w_page  ^= 1;
        }
        break;

        case MISC_CTL:
        // a DSP control for a MISC task
        //------------------------------
        {
          // set only MISC task and reset MISC page
          // (don't change GSM PAGE).
          // set DSP communication Interrupt.
          // set DSP_ENB and the same DSP_PAG for communication interrupt
          l1s_tpu_com.reg_cmd->dsp_pag_bit = l1s_dsp_com.dsp_w_page^1;
          l1s_tpu_com.reg_cmd->dsp_enb_bit = ON;
        }
        break;
      }
    #else // NO AUDIO_SIMULATION
      // set DSP_ENB and DSP_PAG for communication interrupt
      l1s_tpu_com.reg_cmd->dsp_pag_bit = l1s_dsp_com.dsp_w_page;
      l1s_tpu_com.reg_cmd->dsp_enb_bit = ON;

      // change DSP page pointer for next control
      l1s_dsp_com.dsp_w_page  ^= 1;
    #endif // AUDIO_SIMULATION

  #else // NOT_SIMULATION
    UWORD32 dsp_task=0 ;//omaps00090550;
    switch(type)
    {
      case GSM_CTL:
      // a DSP control for a GSM task
      //-----------------------------
      {
      // set only GSM task and GSM page
      dsp_task = B_GSM_TASK | l1s_dsp_com.dsp_w_page;
      // change DSP page pointer for next controle
      l1s_dsp_com.dsp_w_page  ^= 1;
    }
    break;

      case MISC_CTL:
      // a DSP control for a MISC task
      //------------------------------
      {
        UWORD32  previous_page = l1s_dsp_com.dsp_w_page ^ 1;

      // set only MISC task and reset MISC page
      // (don't change GSM PAGE).
      // set DSP communication Interrupt.
      dsp_task = B_MISC_TASK | previous_page;

      // Rem: DSP makes the DB header feedback even in case
      //      of MISC task (like TONES). This created some
      //      side effect which are "work-around" passing
      //      the correct DB page to the DSP.
    }
    break;

    case GSM_MISC_CTL:
    // a DSP control for a GSM and a MISC tasks
    //-----------------------------------------
    {
      // set GSM task, MISC task and GSM page bit.....
      dsp_task =  B_GSM_TASK | B_MISC_TASK | l1s_dsp_com.dsp_w_page;
      // change DSP page pointer for next controle
      l1s_dsp_com.dsp_w_page  ^= 1;
    }
      break;
    }

    // write dsp tasks.....
    #if (DSP >= 33)
      l1s_dsp_com.dsp_ndb_ptr->d_dsp_page = (API) dsp_task;
    #else
      l1s_dsp_com.dsp_param_ptr->d_dsp_page = (API) dsp_task;
    #endif

    // Enable frame IT on next TDMA
    l1dmacro_set_frame_it();

    #if (DSP >= 38)
    // DSP CPU load measurement - write logic (provide TDMA frame number to DSP)
    (*((volatile UWORD16 *)(DSP_CPU_LOAD_MCU_W_TDMA_FN))) = (API)l1s.actual_time.fn_mod42432;
    #endif

  #endif // NOT_SIMULATION
}

/*-------------------------------------------------------*/
/* l1dtpu_meas()                                         */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */

/* Locosto : should take additional Param of task */

/*-------------------------------------------------------*/
void l1dtpu_meas(UWORD16 radio_freq,
                 WORD8   agc,
                 UWORD8  lna_off,
                 UWORD16 win_id,
                 UWORD16 tpu_synchro, UWORD8 adc_active
#if(RF_FAM == 61)
                 ,UWORD8 afc_mode
                 ,UWORD8 if_ctl
#endif
                              )

{

  WORD16  offset;
  WORD16  when;
  UWORD16 offset_chg;

  #if TESTMODE
    if (!l1_config.agc_enable)
    {
      // AGC gain can only be controlled in 2dB steps as the bottom bit (bit zero)
      // corresponds to the lna_off bit
      agc = l1_config.tmode.rx_params.agc;
      lna_off = l1_config.tmode.rx_params.lna_off;
    }
  #endif // TESTMODE

  // Compute offset
  offset_chg = ((win_id  * BP_DURATION) >> BP_SPLIT_PW2);
  offset     = tpu_synchro + offset_chg;
  if(offset >= TPU_CLOCK_RANGE) offset -= TPU_CLOCK_RANGE;

  // Compute offset change timing
  when = offset_chg + PROVISION_TIME - (l1_config.params.rx_synth_setup_time + EPSILON_OFFS);
  if(when < 0) when += TPU_CLOCK_RANGE;

  // Program TPU scenario
  l1dmacro_offset   (offset, when);            // change TPU offset according to win_id
  l1dmacro_rx_synth (radio_freq);              // pgme SYNTH.
  if(adc_active == ACTIVE)
     l1dmacro_adc_read_rx();                   // pgme ADC measurement

  l1dmacro_agc      (radio_freq, agc,lna_off
                               #if (RF_FAM == 61)
					 ,if_ctl
				   #endif
                                   ); // pgme AGC.
  #if (CODE_VERSION == SIMULATION)
    l1dmacro_rx_ms    (radio_freq, 0);           // pgm  PWR acquisition.
  #else
  #if (L1_MADC_ON == 1)
  #if (RF_FAM == 61)
   l1dmacro_rx_ms    (radio_freq,adc_active);              // pgm  PWR acquisition.
   #endif
  #else
    l1dmacro_rx_ms    (radio_freq);              // pgm  PWR acquisition.
   #endif
  #endif
  l1dmacro_offset   (tpu_synchro, IMM);        // restore offset

  //Locosto
  #if(RF_FAM == 61)
    // L1_AFC_SCRIPT_MODE - This is specific to Locosto to make AFC script run after
    // the second power measurement during FBNEW
    if ((win_id == 0)  ||  (afc_mode == L1_AFC_SCRIPT_MODE))
  #else
    if (win_id == 0)
  #endif
  {
    #if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3))
    // NOTE: In Locosto AFC is in DRP not in triton
        l1ddsp_load_afc(l1s.afc);
    #endif

    //Locosto
    #if(RF_FAM == 61)
    if(afc_mode != L1_AFC_NONE)
    {
      if(afc_mode == L1_AFC_SCRIPT_MODE)
      {
        l1dtpu_load_afc(l1s.afc);  //Load the Initial afc value to the TPU. TPU would copy it to the DRP Wrapper Mem.
      }
      else
      {
        l1ddsp_load_afc(l1s.afc);
      }
    }
    #endif
    // end Locosto
  }
}

/*-------------------------------------------------------*/
/* l1dtpu_neig_fb()                                      */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1dtpu_neig_fb(UWORD16 radio_freq, WORD8 agc, UWORD8 lna_off)
{
   #if TESTMODE
    if (!l1_config.agc_enable)
    {
      // AGC gain can only be controlled in 2dB steps as the bottom bit (bit zero)
      // corresponds to the lna_off bit
      agc = l1_config.tmode.rx_params.agc;
      lna_off = l1_config.tmode.rx_params.lna_off;
    }
  #endif

  l1dmacro_rx_synth (radio_freq);               // pgme SYNTH.
  l1dmacro_agc      (radio_freq,agc, lna_off
  	                         #if (RF_FAM == 61)
                                ,IF_120KHZ_DSP
                                 #endif
					 );  // pgme AGC.
#if (L1_MADC_ON == 1)
#if (RF_FAM == 61)
   l1dmacro_rx_fb    (radio_freq,INACTIVE);               // pgm  FB acquisition.
#endif
#else
  l1dmacro_rx_fb    (radio_freq);               // pgm  FB acquisition.
#endif
}

/*-------------------------------------------------------*/
/* l1dtpu_neig_fb26()                                    */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1dtpu_neig_fb26(UWORD16 radio_freq, WORD8 agc, UWORD8 lna_off, UWORD32 offset_serv)
{
  WORD16  offset;

  #if TESTMODE
    if (!l1_config.agc_enable)
    {
      // AGC gain can only be controlled in 2dB steps as the bottom bit (bit zero)
      // corresponds to the lna_off bit
      agc = l1_config.tmode.rx_params.agc;
      lna_off = l1_config.tmode.rx_params.lna_off;
    }
  #endif

  // Compute offset
  offset = offset_serv + l1_config.params.fb26_anchoring_time;
  if(offset >= TPU_CLOCK_RANGE) offset -= TPU_CLOCK_RANGE;

  // Program TPU scenario
  l1dmacro_offset   (offset, l1_config.params.fb26_change_offset_time);
  l1dmacro_rx_synth (radio_freq);               // pgme SYNTH.
  l1dmacro_agc      (radio_freq,agc, lna_off
  	                         #if (RF_FAM == 61)
                                ,IF_120KHZ_DSP
                                 #endif
                                 );  // pgme AGC.
#if (L1_MADC_ON == 1)
#if (RF_FAM == 61)
  l1dmacro_rx_fb26  (radio_freq, INACTIVE);               // pgm  FB acquisition.
#endif
#else
  l1dmacro_rx_fb26  (radio_freq);               // pgm  FB acquisition.
#endif
  l1dmacro_offset   (offset_serv, IMM);         // restore offset
}

/*-------------------------------------------------------*/
/* l1dtpu_neig_sb()                                      */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1dtpu_neig_sb(UWORD16 radio_freq, WORD8 agc, UWORD8 lna_off,
                    UWORD32 time_alignmt, UWORD32 offset_serv, UWORD8 reload_flag,
                    UWORD8 attempt
                   #if (RF_FAM == 61)
                   ,UWORD8 if_ctl
                   #endif
                    )
{
  UWORD16 offset_neigh;

  #if TESTMODE
    if (!l1_config.agc_enable)
    {
      // AGC gain can only be controlled in 2dB steps as the bottom bit (bit zero)
      // corresponds to the lna_off bit
      agc = l1_config.tmode.rx_params.agc;
      lna_off = l1_config.tmode.rx_params.lna_off;
    }
  #endif

  // compute offset neighbour...
  offset_neigh = offset_serv + time_alignmt;
  if(offset_neigh >= TPU_CLOCK_RANGE) offset_neigh -= TPU_CLOCK_RANGE;

  // load OFFSET with NEIGHBOUR value.
  l1dmacro_offset  (offset_neigh, l1_config.params.rx_change_offset_time);

  // Insert 1 NOP to correct the EPSILON_SYNC side effect.
  if(attempt != 2)
  if(time_alignmt >= (TPU_CLOCK_RANGE - EPSILON_SYNC))
    l1dmacro_offset  (offset_neigh, 0);  // load OFFSET with NEIGHBOUR value.

  l1dmacro_rx_synth(radio_freq);               // pgme SYNTH.
  l1dmacro_agc     (radio_freq, agc, lna_off
#if (RF_FAM == 61)
                                ,if_ctl
#endif
                                  ); // pgme AGC.
 #if (L1_MADC_ON == 1)
 #if (RF_FAM == 61)
  l1dmacro_rx_sb   (radio_freq,INACTIVE);               // pgm  SB acquisition.
 #endif
 #else
  l1dmacro_rx_sb   (radio_freq);               // pgm  SB acquisition.
 #endif

  // Restore offset with serving value.
  if(reload_flag == TRUE)
  {
    l1dmacro_offset  (offset_serv, IMM);
  }
}
/*-------------------------------------------------------*/
/* l1dtpu_neig_fbsb()                                      */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
#if ((REL99 == 1) && (FF_BHO == 1))
/*void l1dtpu_neig_fbsb(UWORD16 radio_freq, WORD8 agc, UWORD8 lna_off
                   #if (RF_FAM == 61)
			,UWORD8 if_ctl
  		   #endif
			)*/
void l1dtpu_neig_fbsb(UWORD16 radio_freq, WORD8 agc, UWORD8 lna_off)
{
#if TESTMODE
  if (!l1_config.agc_enable)
  {
    // AGC gain can only be controlled in 2dB steps as the bottom bit (bit zero)
    // corresponds to the lna_off bit
    agc = l1_config.tmode.rx_params.agc;
    lna_off = l1_config.tmode.rx_params.lna_off;
  }
#endif // TESTMODE

  l1dmacro_rx_synth (radio_freq);               // pgme SYNTH.
  /*l1dmacro_agc(radio_freq, agc, lna_off
                         #if (RF_FAM == 61)
                         ,if_ctl
                         #endif
                         );     // pgme AGC.
 */
 l1dmacro_agc      (radio_freq,agc, lna_off
  	                         #if (RF_FAM == 61)
                                ,L1_CTL_LOW_IF
                                 #endif
					 );  // pgme AGC.
#if (L1_MADC_ON == 1)
#if (RF_FAM == 61)
  l1dmacro_rx_fbsb(radio_freq,INACTIVE);               // pgm  FB acquisition.
#endif
#else
  l1dmacro_rx_fbsb(radio_freq);             // pgm  FB acquisition.- sajal commented
#endif
}
#endif  // #if ((REL99 == 1) && (FF_BHO == 1))
/*-------------------------------------------------------*/
/* l1dtpu_neig_sb26()                                    */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1dtpu_neig_sb26(UWORD16 radio_freq, WORD8 agc, UWORD8 lna_off, UWORD32 time_alignmt,
                      UWORD32 fn_offset, UWORD32 offset_serv
                 #if (RF_FAM == 61)
			,UWORD8 if_ctl
  		   #endif
					  )
{
  UWORD16 offset_neigh;

  #if TESTMODE
    if (!l1_config.agc_enable)
    {
      // AGC gain can only be controlled in 2dB steps as the bottom bit (bit zero)
      // corresponds to the lna_off bit
      agc = l1_config.tmode.rx_params.agc;
      lna_off = l1_config.tmode.rx_params.lna_off;
    }
  #endif

  // compute offset neighbour...
  offset_neigh = offset_serv + time_alignmt;
  if(offset_neigh >= TPU_CLOCK_RANGE) offset_neigh -= TPU_CLOCK_RANGE;

  if(fn_offset != 0)
    l1dmacro_offset  (offset_neigh, 0);       // 1 NOP in some case
  else
    l1dmacro_offset  (offset_neigh, l1_config.params.fb26_change_offset_time);

  l1dmacro_rx_synth(radio_freq);              // pgme SYNTH.
  l1dmacro_agc(radio_freq, agc, lna_off
                         #if (RF_FAM == 61)
                         ,if_ctl
                         #endif
                         );     // pgme AGC.
#if (L1_MADC_ON == 1)
#if (RF_FAM == 61)
 l1dmacro_rx_sb   (radio_freq, INACTIVE);              // pgm  SB acquisition.
#endif
#else
  l1dmacro_rx_sb   (radio_freq);              // pgm  SB acquisition.
#endif
  l1dmacro_offset  (offset_serv, IMM);        // Restore offset with serving value.
}

/*-------------------------------------------------------*/
/* l1dtpu_serv_rx_nb()                                   */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1dtpu_serv_rx_nb(UWORD16 radio_freq, WORD8 agc, UWORD8 lna_off,
                       UWORD32 synchro_serv,UWORD32 new_offset,BOOL change_offset,
                       UWORD8 adc_active
                      #if(RF_FAM == 61)
                     , UWORD8 csf_filter_choice
                     , UWORD8 if_ctl
                      #endif
                      #if (NEW_SNR_THRESHOLD == 1)
                     , UWORD8 saic_flag
                      #endif/* NEW_SNR_THRESHOLD == 1*/
                         )
{

  #if (CODE_VERSION == SIMULATION)
    UWORD32 tpu_w_page;

    if (hw.tpu_r_page==0)
       tpu_w_page=1;
    else
       tpu_w_page=0;

    // Give the Ts related to the L1s
    hw.rx_id[tpu_w_page][0]= ((TPU_CLOCK_RANGE+new_offset-synchro_serv)%TPU_CLOCK_RANGE)/TN_WIDTH;
    hw.num_rx[tpu_w_page][0]=1;
    hw.rx_group_id[tpu_w_page]=1;
  #endif

  #if TESTMODE
    if (!l1_config.agc_enable)
    {
      // AGC gain can only be controlled in 2dB steps as the bottom bit (bit zero)
      // corresponds to the lna_off bit
      agc = l1_config.tmode.rx_params.agc;
      lna_off = l1_config.tmode.rx_params.lna_off;
    }
  #endif

  l1dmacro_synchro (l1_config.params.rx_change_synchro_time, synchro_serv); // Adjust serving OFFSET.

  #if L2_L3_SIMUL
    #if (DEBUG_TRACE == BUFFER_TRACE_OFFSET)
      buffer_trace(3, 0x43, synchro_serv,l1s.actual_time.fn,0);
    #endif
  #endif

  // Need to slide offset to cope with the new synchro.
  if(change_offset)
    l1dmacro_offset(new_offset, l1_config.params.rx_change_offset_time);

  l1dmacro_rx_synth(radio_freq);                   // load SYNTH.
  if(adc_active == ACTIVE)
     l1dmacro_adc_read_rx();                       // pgme ADC measurement

  l1dmacro_agc     (radio_freq, agc, lna_off
                               #if (RF_FAM == 61)
                               ,if_ctl
                               #endif
                                );

  #if TESTMODE && (CODE_VERSION != SIMULATION)
    // Continuous mode: Rx continuous scenario only on START_RX state.
    if ((l1_config.TestMode) &&
        (l1_config.tmode.rf_params.tmode_continuous == TM_START_RX_CONTINUOUS))
   #if (L1_MADC_ON == 1)
   #if (RF_FAM == 61)
      l1dmacro_rx_cont   (FALSE, radio_freq,adc_active,csf_filter_choice
    #if (NEW_SNR_THRESHOLD == 1)
        ,saic_flag
    #endif /* NEW_SNR_THRESHOLD*/
          );
   #endif /* RF_FAM == 61*/
   #else  /* L1_MADC_ON == 1 */
    #if (RF_FAM == 61)
      l1dmacro_rx_cont   (FALSE, radio_freq,csf_filter_choice);
    #else
      l1dmacro_rx_cont   (FALSE, radio_freq);
    #endif
   #endif
	//TBD Danny New MAcro for Cont Tx reqd, to use only External Trigger
    else
  #endif
 #if ( L1_MADC_ON == 1)
 #if (RF_FAM == 61)
    l1dmacro_rx_nb   (radio_freq, adc_active, csf_filter_choice
 #if (NEW_SNR_THRESHOLD == 1)
        ,saic_flag
 #endif /* NEW_SNR_THRESHOLD*/
        );  // RX window for NB.
 #endif /* RF_FAM == 61*/
 #else /* L1_MADC_ON == 1*/
   #if (RF_FAM == 61)
     l1dmacro_rx_nb   (radio_freq, csf_filter_choice);	// RX window for NB.
   #else
     l1dmacro_rx_nb   (radio_freq);			// RX window for NB.
   #endif
 #endif

  #if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3))
      l1ddsp_load_afc(l1s.afc);
  #endif
  #if (RF_FAM == 61)
      l1dtpu_load_afc(l1s.afc);
  #endif


  if(change_offset)
    l1dmacro_offset(synchro_serv, IMM); // Restore offset.
}

/*-------------------------------------------------------*/
/* l1dtpu_serv_tx_nb()                                   */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1dtpu_serv_tx_nb(UWORD16 radio_freq, UWORD8 timing_advance,
                       UWORD32 offset_serv, UWORD8 txpwr, UWORD8 adc_active)
{
  WORD32    time;
  UWORD32   offset_tx;
  UWORD32   timing_advance_in_qbit = (UWORD32)timing_advance << 2;

  #if (CODE_VERSION == SIMULATION)
    UWORD32 tpu_w_page;

    if (hw.tpu_r_page==0)
       tpu_w_page=1;
    else
       tpu_w_page=0;

    hw.tx_id[tpu_w_page][0]=3;// MS synchronized on TN=0 for RX => TN=3 for TX
    hw.num_tx[tpu_w_page][0]=1;
    hw.tx_group_id[tpu_w_page]=1;
  #endif

  // Reset timing advance if TA_ALGO not enabled.
  #if !TA_ALGO
    timing_advance_in_qbit = 0;
  #endif

  // Compute offset value for TX.
  // PRG_TX has become variable, no longer contained in TIME_OFFSET_TX !
  offset_tx = (offset_serv + TIME_OFFSET_TX-l1_config.params.prg_tx_gsm - timing_advance_in_qbit) ;
  if (offset_tx >= TPU_CLOCK_RANGE) offset_tx -= TPU_CLOCK_RANGE;

  // Check that RX controle has been already installed.
  // Offset for TX must be set an immediately if RX is there else
  // it must be performed EPSILON_SYNC before current offset time.
  if( l1s.tpu_ctrl_reg & CTRL_RX )
    time = l1_config.params.tx_change_offset_time - l1_config.params.prg_tx_gsm;
  else
    time = TPU_CLOCK_RANGE - EPSILON_SYNC;

  l1dmacro_offset  (offset_tx, time);  // load OFFSET for TX before each burst.

  #if L2_L3_SIMUL
    #if (DEBUG_TRACE == BUFFER_TRACE_OFFSET)
      buffer_trace(2, offset_tx,l1s.actual_time.fn,0,0);
    #endif
  #endif

  l1dmacro_tx_synth(radio_freq);         // load SYNTH.
  #if TESTMODE && (CODE_VERSION != SIMULATION)
    // Continuous mode: Tx continuous scenario only on START_TX state.
    #if (RF_FAM != 61)
      if ((l1_config.TestMode) &&
          (l1_config.tmode.rf_params.tmode_continuous == TM_START_TX_CONTINUOUS))
        l1dmacro_tx_cont   (radio_freq, txpwr);     // TX window for NB.
      else
    #endif // RF_FAM != 61
    #if (RF_FAM == 61)
      // NOTE: In Test Mode and in TX Continuous, APC control is in manual mode
      // This is done in l1tm_async.c
      if ((l1_config.TestMode) &&
          (l1_config.tmode.rf_params.tmode_continuous == TM_START_TX_CONTINUOUS))
      {
	 // NOTE: APC is set in manual mode from l1tm_async.c
	 l1dmacro_tx_cont   (radio_freq, txpwr);     // TX window for NB.
      }
      else
    #endif // RF_FAM == 61
  #endif
  l1dmacro_tx_nb   (radio_freq, txpwr, adc_active);  // TX window for NB.
  // TX window for NB.
  l1dmacro_offset  (offset_serv, IMM);   // Restore offset with serving value.

  #if L2_L3_SIMUL
    #if (DEBUG_TRACE == BUFFER_TRACE_OFFSET)
      buffer_trace(2, offset_serv,l1s.actual_time.fn,0,0);
    #endif
  #endif
}

/*-------------------------------------------------------*/
/* l1dtpu_neig_rx_nb()                                   */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1dtpu_neig_rx_nb(UWORD16 radio_freq, WORD8 agc, UWORD8 lna_off,
                       UWORD32 time_alignmt, UWORD32 offset_serv, UWORD8 reload_flag,
                       UWORD8 nop
                     #if (RF_FAM == 61)
			    ,UWORD8 if_ctl
			 #endif
#if (NEW_SNR_THRESHOLD == 1)
                                  ,UWORD8 saic_flag
#endif /* NEW_SNR_THRESHOLD*/
					      )
{
  UWORD32 offset_neigh;
#if (RF_FAM == 61)
  // By default we choose the hardware filter for neighbour Normal Bursts
  UWORD8 csf_filter_choice = L1_SAIC_HARDWARE_FILTER;
#endif

  #if TESTMODE
    if (!l1_config.agc_enable)
    {
      // AGC gain can only be controlled in 2dB steps as the bottom bit (bit zero)
      // corresponds to the lna_off bit
      agc = l1_config.tmode.rx_params.agc;
      lna_off = l1_config.tmode.rx_params.lna_off;
    }
  #endif

 // compute offset neighbour...
  offset_neigh = (offset_serv + time_alignmt) ;
  if (offset_neigh >= TPU_CLOCK_RANGE) offset_neigh -= TPU_CLOCK_RANGE;

  l1dmacro_offset  (offset_neigh, l1_config.params.rx_change_offset_time);  // load OFFSET with NEIGHBOUR value.
  // Insert 1 NOP to correct the EPSILON_SYNC side effect
  if (nop ==1) l1dmacro_offset  (offset_neigh,0);


  l1dmacro_rx_synth(radio_freq);               // load SYNTH.
  l1dmacro_agc     (radio_freq, agc, lna_off
  	                    #if (RF_FAM == 61)
				   ,if_ctl
				#endif
				    );
#if (L1_MADC_ON == 1)
#if (RF_FAM == 61)
  l1dmacro_rx_nb   (radio_freq, INACTIVE, csf_filter_choice
#if (NEW_SNR_THRESHOLD == 1)
        ,saic_flag
#endif /* NEW_SNR_THRESHOLD*/
      )  ;     // RX window for NB.
#endif  /* RF_FAM == 61*/
#else /* L1_MADC_ON == 1*/
  #if (RF_FAM == 61)
    l1dmacro_rx_nb   (radio_freq, csf_filter_choice);	// RX window for NB.
  #else
    l1dmacro_rx_nb   (radio_freq);			// RX window for NB.
  #endif
#endif

  // Restore offset with serving value.
  if(reload_flag == TRUE)
    l1dmacro_offset  (offset_serv, IMM);

}

/*-------------------------------------------------------*/
/* l1dtpu_serv_tx_ra()                                   */
/*-------------------------------------------------------*/
/* Parameters : "burst_id" gives the burst identifier    */
/*              which is used for offset management.     */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1dtpu_serv_tx_ra(UWORD16 radio_freq, UWORD32 offset_serv, UWORD8 txpwr, UWORD8 adc_active)
{
  WORD32    time;
  UWORD32   offset_tx;

  // Compute offset value for TX.
  // Rem: Timing Advance is always 0 for a RA.
  // PRG_TX has become variable, no longer contained in TIME_OFFSET_TX !
  offset_tx = (offset_serv + TIME_OFFSET_TX-l1_config.params.prg_tx_gsm);
  if (offset_tx >= TPU_CLOCK_RANGE) offset_tx -= TPU_CLOCK_RANGE;

  // Check that RX controle has been already installed.
  // Offset for TX must be set an immediately if RX is there else
  // it must be performed EPSILON_SYNC before current offset time.
  if( l1s.tpu_ctrl_reg & CTRL_RX )
    time = l1_config.params.tx_change_offset_time - l1_config.params.prg_tx_gsm;
  else
    time = TPU_CLOCK_RANGE - EPSILON_SYNC;

  l1dmacro_offset  (offset_tx, time);  // load OFFSET for TX before each burst.
  #if L2_L3_SIMUL
    #if (DEBUG_TRACE == BUFFER_TRACE_OFFSET)
      buffer_trace(2, offset_tx,l1s.actual_time.fn,0,0);
    #endif
  #endif
  l1dmacro_tx_synth(radio_freq);            // load SYNTH.
  l1dmacro_tx_ra   (radio_freq, txpwr,adc_active);     // TX window for RA.
  l1dmacro_offset  (offset_serv, IMM); // Restore offset with serving value.
  #if L2_L3_SIMUL
    #if (DEBUG_TRACE == BUFFER_TRACE_OFFSET)
      buffer_trace(2, offset_serv,l1s.actual_time.fn,0,0);
    #endif
  #endif
}

/*-------------------------------------------------------*/
/* l1dtpu_end_scenario()                                 */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1dtpu_end_scenario(void)
{

  // write IDLE at end of TPU page
  // TPU_ENB and TPU_PAG are set in L1DMACRO_IDLE(). The TPU change
  // is executed by the TPU itself and the TPU pointer is reset to
  // start of page by l1dmacro_idle();
  l1dmacro_idle();

  #if (CODE_VERSION == SIMULATION)
    #if LOGTPU_TRACE
      log_macro();
    #endif
  #endif
  // init pointer within new TPU page at 1st line
  #if (CODE_VERSION == SIMULATION)
    // set TPU_ENB, TPU_PAG for communication interrupt
    l1s_tpu_com.reg_cmd->tpu_pag_bit = l1s_tpu_com.tpu_w_page;
    l1s_tpu_com.reg_cmd->tpu_enb_bit = ON;

    // change TPU and DSP page pointer for next control
    l1s_tpu_com.tpu_w_page  ^= 1;

    // points on new "write TPU page"...
    l1s_tpu_com.tpu_page_ptr=&(tpu.buf[l1s_tpu_com.tpu_w_page].line[0]);

  #endif

}

/*-------------------------------------------------------*/
/* l1d_reset_hw()                                        */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1d_reset_hw(UWORD32 offset_value)
{
  #if (CODE_VERSION == SIMULATION)
  // Reset DSP write/read page, Reset TPU write page, reset "used" flag.
  l1s_dsp_com.dsp_w_page      = 0;
  l1s_dsp_com.dsp_r_page      = 0;
  l1s_tpu_com.tpu_w_page      = 0;
  l1s_dsp_com.dsp_r_page_used = 0;

  // Reset communication pointers.
    l1s_dsp_com.dsp_ndb_ptr  = &(buf.ndb);                            // MCU<->DSP comm. read/write (Non Double Buffered comm. memory).
    l1s_dsp_com.dsp_db_r_ptr = &(buf.mcu_rd[l1s_dsp_com.dsp_r_page]); // MCU<->DSP comm. read  page (Double Buffered comm. memory).
    l1s_dsp_com.dsp_db_w_ptr = &(buf.mcu_wr[l1s_dsp_com.dsp_w_page]); // MCU<->DSP comm. write page (Double Buffered comm. memory).

  // Reset task commands.
  l1s_dsp_com.dsp_db_w_ptr->d_task_d  = NO_DSP_TASK; // Init. RX task to NO TASK.
  l1s_dsp_com.dsp_db_w_ptr->d_task_u  = NO_DSP_TASK; // Init. TX task to NO TASK.
  l1s_dsp_com.dsp_db_w_ptr->d_task_ra = NO_DSP_TASK; // Init. RA task to NO TASK.
  l1s_dsp_com.dsp_db_w_ptr->d_task_md = NO_DSP_TASK; // Init. MONITORING task to NO TASK.


  //Reset the TCH channel description
  l1s_dsp_com.dsp_db_w_ptr->d_ctrl_tch = 0;

    #if (L1_GPRS)
      // Reset communication pointers.
      l1ps_dsp_com.pdsp_db_r_ptr = &(buf.mcu_rd_gprs[l1s_dsp_com.dsp_r_page]);
      l1ps_dsp_com.pdsp_db_w_ptr = &(buf.mcu_wr_gprs[l1s_dsp_com.dsp_w_page]);

      // Reset MCU->DSP page.
      l1ps_reset_db_mcu_to_dsp(l1ps_dsp_com.pdsp_db_w_ptr);
    #endif // L1_GPRS

    // Direct access to TPU_RESET_BIT.
    l1s_tpu_com.reg_cmd->tpu_reset_bit = ON;           // Reset TPU.

    // Reset TPU_ENB, DSP_ENB and TPU_PAG, DSP_PAG for communication interrupt
    l1s_tpu_com.reg_cmd->tpu_pag_bit = 0;
    l1s_tpu_com.reg_cmd->dsp_pag_bit = 0;
    l1s_tpu_com.reg_cmd->tpu_enb_bit = OFF;
    l1s_tpu_com.reg_cmd->dsp_enb_bit = OFF;

    // Init pointer within TPU page 0 at 1st line
    l1s_tpu_com.tpu_page_ptr = &(tpu.buf[0].line[0]);

    // Load offset register according to serving cell.
    l1dmacro_offset(offset_value, IMM);

  #else // NOT_SIMULATION

    // Reset DSP write/read page, Reset TPU write page, reset "used" flag.
    l1s_dsp_com.dsp_w_page      = 0;
    l1s_dsp_com.dsp_r_page      = 0;
    l1s_tpu_com.tpu_w_page      = 0;
    l1s_dsp_com.dsp_r_page_used = 0;

    // Reset communication pointers.
    l1s_dsp_com.dsp_ndb_ptr  = (T_NDB_MCU_DSP *)   NDB_ADR;      // MCU<->DSP comm. read/write (Non Double Buffered comm. memory).
    l1s_dsp_com.dsp_db_r_ptr = (T_DB_DSP_TO_MCU *) DB_R_PAGE_0;  // MCU<->DSP comm. read  page (Double Buffered comm. memory).
    l1s_dsp_com.dsp_db_w_ptr = (T_DB_MCU_TO_DSP *) DB_W_PAGE_0;  // MCU<->DSP comm. write page (Double Buffered comm. memory).
    l1s_dsp_com.dsp_param_ptr= (T_PARAM_MCU_DSP *) PARAM_ADR;

    #if (DSP == 38) || (DSP == 39)
    l1s_dsp_com.dsp_db_common_w_ptr = (T_DB_COMMON_MCU_TO_DSP *) DB_COMMON_W_PAGE_0;
    #endif

    // Reset task commands.
    l1s_dsp_com.dsp_db_w_ptr->d_task_d  = NO_DSP_TASK; // Init. RX task to NO TASK.
    l1s_dsp_com.dsp_db_w_ptr->d_task_u  = NO_DSP_TASK; // Init. TX task to NO TASK.
    l1s_dsp_com.dsp_db_w_ptr->d_task_ra = NO_DSP_TASK; // Init. RA task to NO TASK.
    l1s_dsp_com.dsp_db_w_ptr->d_task_md = NO_DSP_TASK; // Init. MONITORING task to NO TASK.

    //Reset the TCH channel description
    l1s_dsp_com.dsp_db_w_ptr->d_ctrl_tch = 0;

    // Clear DSP_PAG bit
    #if (DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39)
      l1s_dsp_com.dsp_ndb_ptr->d_dsp_page = 0;
    #else
      l1s_dsp_com.dsp_param_ptr->d_dsp_page = 0;
    #endif

    #if (L1_GPRS)
      // Reset communication pointers.
      l1ps_dsp_com.pdsp_ndb_ptr  = (T_NDB_MCU_DSP_GPRS *)   NDB_ADR_GPRS;
      l1ps_dsp_com.pdsp_db_r_ptr = (T_DB_DSP_TO_MCU_GPRS *) DB_R_PAGE_0_GPRS;
      l1ps_dsp_com.pdsp_db_w_ptr = (T_DB_MCU_TO_DSP_GPRS *) DB_W_PAGE_0_GPRS;
      l1ps_dsp_com.pdsp_param_ptr= (T_PARAM_MCU_DSP_GPRS *) PARAM_ADR_GPRS;

      // Reset MCU->DSP page.
      l1ps_reset_db_mcu_to_dsp(l1ps_dsp_com.pdsp_db_w_ptr);
    #endif // L1_GPRS

    #if (DSP_DEBUG_TRACE_ENABLE == 1)
      l1s_dsp_com.dsp_db2_current_r_ptr = (T_DB2_DSP_TO_MCU *) DB2_R_PAGE_0;
      l1s_dsp_com.dsp_db2_other_r_ptr   = (T_DB2_DSP_TO_MCU *) DB2_R_PAGE_1;
    #endif

    // Reset TPU and Reload offset register with Serving value.
    // Clear TPU_PAG
    l1dmacro_reset_hw(offset_value);
  #endif // NOT_SIMULATION
}


#if(RF_FAM == 61)

/*-------------------------------------------------------*/
/* l1apc_init_ramp_tables()                              */
/*-------------------------------------------------------*/
/* Parameters :        void                              */
/* Return     :        void                              */
/* Functionality :     This would copy the Ramp table    */
/*                     values to the MCU DSP API         */
/*-------------------------------------------------------*/
void l1dapc_init_ramp_tables(void)
{

#if (CODE_VERSION == SIMULATION)
// Do Nothing there is no APC task
 #else
   #if ( DSP == 38) || (DSP == 39)
      // Load RAMP up/down in NDB memory...
      if (l1_config.tx_pwr_code == 0)
      {
        Cust_get_ramp_tab( l1s_dsp_com.dsp_ndb_ptr->a_drp_ramp,
                          0 /* not used */,
                          0 /* not used */,
                          1 /* arbitrary value for arfcn*/);

      }
      else
      {
        Cust_get_ramp_tab( l1s_dsp_com.dsp_ndb_ptr->a_drp_ramp,
                          5 /* arbitrary value working in any case */,
                          5 /* arbitrary value working in any case */,
                          1 /* arbitrary value for arfcn*/);
      }
  #endif
   // Is it required to load ramptables for GPRS a_drp2_ramp_gprs

#endif

}



/*-------------------------------------------------------*/
/* l1ddsp_apc_load_apcctrl2                              */
/*-------------------------------------------------------*/
/* Parameters :        void                              */
/* Return     :        void                              */
/* Functionality :     This would copy the Ramp table    */
/*                     values to the MCU DSP API         */
/*-------------------------------------------------------*/
void l1ddsp_apc_load_apcctrl2(UWORD16 apcctrl2)
{
  l1s_dsp_com.dsp_ndb_ptr->d_apcctrl2 = ((apcctrl2) | (0x8000));
}

/*-------------------------------------------------------*/
/* l1ddsp_apc_set_manual_mode                            */
/*-------------------------------------------------------*/
/* Parameters :        void                              */
/* Return     :        void                              */
/* Functionality :     This would set the APC in manual  */
/*                     OR external trigger mode          */
/*-------------------------------------------------------*/
void l1ddsp_apc_set_manual_mode(void)
{
  l1s_dsp_com.dsp_ndb_ptr->d_apcctrl2 |= ((APC_APC_MODE) | (0x8000));
}

/*-------------------------------------------------------*/
/* l1ddsp_apc_set_automatic_mode                         */
/*-------------------------------------------------------*/
/* Parameters :        void                              */
/* Return     :        void                              */
/* Functionality :     This would set APC in automatic   */
/*                     OR internal sequencer mode        */
/*-------------------------------------------------------*/
void l1ddsp_apc_set_automatic_mode(void)
{
  l1s_dsp_com.dsp_ndb_ptr->d_apcctrl2 &=  ~(APC_APC_MODE);
  l1s_dsp_com.dsp_ndb_ptr->d_apcctrl2 |=    (0x8000);
}

#ifdef TESTMODE

/*-------------------------------------------------------*/
/* l1ddsp_apc_load_apclev                                */
/*-------------------------------------------------------*/
/* Parameters :    void                                  */
/* Return     :    void                                  */
/* Functionality : This function writes the apclev       */
/*                 val into the APCLEV register via DSP  */
/* NOTE:  Used only in TESTMODE and only when            */
/* l1_config.tmode.rf_params.down_up == TMODE_UPLINK;    */
/*-------------------------------------------------------*/
void l1ddsp_apc_load_apclev(UWORD16 apclev)
{
  l1s_dsp_com.dsp_ndb_ptr->d_apclev = ((apclev) | (0x8000));
}


#endif // TESTMODE

#endif
#if FF_L1_IT_DSP_DTX
/*-------------------------------------------------------*/
/* l1ddsp_dtx_interrupt_pending()                        */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :    DTX interrupt status                  */
/* Functionality : Test and clear the DTX IT pending     */
/*                 flag for DSP ISR screening purpose    */
/*-------------------------------------------------------*/
BOOL l1ddsp_dtx_interrupt_pending(void)
{
  if (l1s_dsp_com.dsp_ndb_ptr->d_dsp_hint_flag & (2 << B_DTX_HINT_ISSUED))
  {
    // Flag HISR to be scheduled
    l1a_apihisr_com.dtx.pending = TRUE;
    // Clear API ISR condition
    l1s_dsp_com.dsp_ndb_ptr->d_dsp_hint_flag &= ~(2 << B_DTX_HINT_ISSUED);
    return TRUE;
  }
  else
    return FALSE;
}
#endif


#define L1_DEBUG_IQ_DUMP  0

#if (L1_DEBUG_IQ_DUMP == 1)

#define IQ_DUMP_MAX_LOG_SIZE  (400) /* i.e. 200 I-Q Sample Pair */
#define IQ_DUMP_BUFFER_SIZE  (1280)
#define L1_DSP_DUMP_IQ_BUFFER_PAGE0 (0xFFD00000 + ((0x2000 - 0x800)*2))
#define L1_DSP_DUMP_IQ_BUFFER_PAGE1 (0xFFD00000 + ((0x2190 - 0x800)*2))

typedef struct
{
  UWORD8  task;
  UWORD8  hole;
  UWORD16 size;
  UWORD16 fn_mod42432;
  UWORD16 iq_sample[IQ_DUMP_MAX_LOG_SIZE];
}T_IQ_LOG_BUFFER;

#pragma DATA_SECTION(iq_dump_buffer,".debug_data");
T_IQ_LOG_BUFFER iq_dump_buffer[IQ_DUMP_BUFFER_SIZE];

UWORD32         iq_dump_buffer_log_index = 0;
UWORD32         iq_overflow_ind=0;

#endif

void l1ddsp_read_iq_dump(UWORD8 task)
{

#if (L1_DEBUG_IQ_DUMP == 1)
  UWORD16 *p_dsp_iq_buffer_ptr;
  UWORD16 size;
  int i;

  /* get the page logic*/
  p_dsp_iq_buffer_ptr = (UWORD16 *)(L1_DSP_DUMP_IQ_BUFFER_PAGE0);
  if(l1s_dsp_com.dsp_r_page){
    p_dsp_iq_buffer_ptr = (UWORD16 *)(L1_DSP_DUMP_IQ_BUFFER_PAGE1);
  }

  /* */
  size = *p_dsp_iq_buffer_ptr;

  if(size == 0)
    return;

  /* size given by DSP is in units of I-Q sample pair */
  if(size > (IQ_DUMP_MAX_LOG_SIZE /2)){
    size = (IQ_DUMP_MAX_LOG_SIZE/2);
  }

  /* make size as zero again */
  *p_dsp_iq_buffer_ptr++ = 0;

  if(iq_dump_buffer_log_index >= IQ_DUMP_BUFFER_SIZE){
    iq_overflow_ind=1;
    iq_dump_buffer_log_index = 0;
  }

  iq_dump_buffer[iq_dump_buffer_log_index].task = task;
  iq_dump_buffer[iq_dump_buffer_log_index].hole = 0;
  iq_dump_buffer[iq_dump_buffer_log_index].size = size;
  iq_dump_buffer[iq_dump_buffer_log_index].fn_mod42432 = l1s.actual_time.fn_mod42432;

  memcpy(&iq_dump_buffer[iq_dump_buffer_log_index].iq_sample[0],
      p_dsp_iq_buffer_ptr,
      size*2*2); /* size * 2 (as size is in IQsample pair) * 2 (to convert to bytes) */

  iq_dump_buffer_log_index = iq_dump_buffer_log_index + 1;

#endif
}


