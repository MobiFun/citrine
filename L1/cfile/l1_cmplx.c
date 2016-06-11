/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_CMPLX.C
 *
 *        Filename l1_cmplx.c
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

#define  L1_CMPLX_C

//#pragma DUPLICATE_FOR_INTERNAL_RAM_START

#include "config.h"
#include "l1_confg.h"
#include "l1_macro.h"

#if (CODE_VERSION == SIMULATION)
  #include <string.h>
  #include "l1_types.h"
  #include "sys_types.h"
  #include "l1_const.h"
  #include "l1_time.h"
  #include "l1_signa.h"
  #include <l1_trace.h>

  #if TESTMODE
    #include "l1tm_defty.h"
  #endif
  #if (AUDIO_TASK == 1)
    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_signa.h"
    #include "l1audio_defty.h"
    #include "l1audio_msgty.h"
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
  #include "cust_os.h"
  #include "l1_msgty.h"
  #include "l1_varex.h"
  #include "l1_proto.h"
  #include "l1_mftab.h"
  #include "l1_tabs.h"
  #include "l1_ver.h"
  #if L2_L3_SIMUL
    #include "l2_l3.h"
    #include "hw_debug.h"
  #endif

  #if L1_GPRS
    #include "l1p_cons.h"
    #include "l1p_msgt.h"
    #include "l1p_deft.h"
    #include "l1p_vare.h"
    #include "l1p_sign.h"
  #endif

  #include "sim_cons.h"
  #include "sim_def.h"
  extern T_hw FAR hw;

#else
  #include "abb.h"
  #include <string.h>
  #include "l1_types.h"
  #include "sys_types.h"
  #include "l1_const.h"
  #include "l1_time.h"
  #include "l1_signa.h"
  #if TESTMODE
    #include "l1tm_defty.h"
    #if (RF_FAM == 60)
      #include "pld.h"
    #endif
  #endif
  #if (AUDIO_TASK == 1)
    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_signa.h"
    #include "l1audio_defty.h"
    #include "l1audio_msgty.h"
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
  #include "l1_mftab.h"
  #include "l1_tabs.h"
  #include "l1_ver.h"
  #include "l1_trace.h"
  #include "l1_ctl.h"
  #if L2_L3_SIMUL
    #include "l2_l3.h"
    #include "hw_debug.h"
    #include "l2_simul.h"
  #endif

  #if L1_GPRS
    #include "l1p_cons.h"
    #include "l1p_msgt.h"
    #include "l1p_deft.h"
    #include "l1p_vare.h"
    #include "l1p_sign.h"
  #endif
#endif

#if(RF_FAM == 61)
  #include "l1_rf61.h"
  #include "tpudrv61.h"
#endif
#include "l1_ctl.h"

#if W_A_DSP1
extern UWORD8 old_sacch_DSP_bug;
#endif

#if TESTMODE
  #include "l1tm_msgty.h"
  #include "l1tm_signa.h"
  #include "l1tm_varex.h"
  void l1tm_fill_burst (UWORD16 pattern, UWORD16 *TM_ul_data);
  #if (ANLG_FAM != 11)
  void ABB_Write_Uplink_Data(SYS_UWORD16 *TM_ul_data);
  #else
  // TODO
  #endif
#endif

#if ((TRACE_TYPE==2) || (TRACE_TYPE==3))
  extern void L1_trace_string(char *s);
  extern void L1_trace_char  (char s);
#endif

#if (GSM_IDLE_RAM != 0)
#if (OP_L1_STANDALONE == 1)
#include "csmi_simul.h"
#else
#include "csmi/sleep.h"
#endif
#endif

#if (RF_FAM == 61)
  #include "l1_rf61.h"
#if (DRP_FW_EXT==1)
  #include "l1_drp_inc.h"
#else
  #include "drp_drive.h"
#endif
#endif

/*-------------------------------------------------------*/
/* Prototypes of external functions used in this file.   */
/*-------------------------------------------------------*/
void l1dmacro_synchro        (UWORD32 when, UWORD32 value);

void l1dmacro_offset         (UWORD32 offset_value, WORD32 relative_time);
void l1dmacro_rx_synth       (UWORD16 arfcn);
void l1dmacro_agc            (UWORD16 arfcn,WORD8 gain, UWORD8 lna
	                                   #if (RF_FAM == 61)
                                            ,UWORD8 if_ctl
						#endif
							   );
void l1dmacro_rx_nb          (UWORD16 arfcn);
void l1dmacro_afc            (UWORD16 afc_value, UWORD8 win_id);
void l1dmacro_adc_read_rx    (void);
#if (CODE_VERSION != SIMULATION)
#if (L1_MADC_ON ==1)
void l1dmacro_adc_read_rx_cs_mode0(void);
#endif
#endif

#if (RF_FAM != 61)
void l1dtpu_serv_rx_nb       (UWORD16 radio_freq, WORD8 agc, UWORD8 lna_off,
                              UWORD32 synchro_serv,UWORD32 new_offset,BOOL change_offset, UWORD8 adc_active);
#endif

#if (RF_FAM == 61)
void l1dtpu_serv_rx_nb       (UWORD16 radio_freq, WORD8 agc, UWORD8 lna_off,
                              UWORD32 synchro_serv,UWORD32 new_offset,BOOL change_offset,
                              UWORD8 adc_active, UWORD8 csf_filter_choice, UWORD8 if_ctl
#if (NEW_SNR_THRESHOLD == 1)
                                  ,UWORD8 saic_flag
#endif /* NEW_SNR_THRESHOLD*/
                              );
#endif /* RF_FAM == 61*/

void l1ddsp_meas_read        (UWORD8 nbmeas, UWORD8 *pm);

#if L1_GPRS
void l1pddsp_synchro         (UWORD8 switch_mode, UWORD8  camp_timeslot);
void l1pddsp_load_bcchn_task (UWORD8 tsq,UWORD16 radio_freq);
void l1pddsp_meas_ctrl       (UWORD8 nbmeas, UWORD8 pm_pos);
void l1pddsp_meas_read       (UWORD8 nbmeas, UWORD8 *a_pm);
#if FF_L1_IT_DSP_USF
void l1pddsp_idle_rx_nb      (UWORD8 burst_nb, UWORD8 tsq, UWORD16 radio_freq,
                              UWORD8 timeslot_no, BOOL ptcch_dl, BOOL usf_interrupt);
#else
void l1pddsp_idle_rx_nb      (UWORD8 burst_nb, UWORD8 tsq, UWORD16 radio_freq,
                              UWORD8 timeslot_no, BOOL ptcch_dl);
#endif
#endif

#if (RF_FAM == 61)
void   cust_get_if_dco_ctl_algo (UWORD16* dco_algo_ctl, UWORD8* if_ctl,
  UWORD8 input_level_flag, UWORD8 input_level, UWORD16 radio_freq, UWORD8 if_threshold);
#endif

#if FEATURE_TCH_REROUTE
extern BOOL tch_reroute_downlink;
extern void tch_send_downlink_bits(API *dsp_buffer);
extern void tch_substitute_uplink(API *dsp_buffer);
#endif

//#pragma DUPLICATE_FOR_INTERNAL_RAM_END

extern UWORD16 toa_tab[4];


#if !((MOVE_IN_INTERNAL_RAM == 1) && (GSM_IDLE_RAM !=0))  // MOVE TO INTERNAL MEM IN CASE GSM_IDLE_RAM enabled
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_START         // KEEP IN EXTERNAL MEM otherwise
  UWORD16 toa_tab[4];
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END
#endif


#if TESTMODE
  UWORD16 TM_ul_data[16]; //Uplink data to be stored into Omega Uplink buffer
#endif
#if ((REL99 == 1) && (FF_BHO == 1))
  void l1dtpu_neig_fbsb(UWORD16 radio_freq, WORD8 agc, UWORD8 lna_off); // Blind handover
#endif

/*-------------------------------------------------------*/
/* l1s_ctrl_hwtest()                                     */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* task HWTEST. This function check the checksum of the  */
/* DSP.                                                  */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/* "l1s.dsp_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/DSP        */
/*   interface. This is used mainly to swap then the     */
/*   com. page at the end of a control frame.            */
/*   -> set CTRL_TEST bit in the register.               */
/*-------------------------------------------------------*/
void l1s_ctrl_hwtest(UWORD8 task, UWORD8 param2)
{
  // Flag DSP programmation.
  // ************************
  l1ddsp_load_monit_task(DSP_TASK_CODE[task],0);

  // Set "CTRL_TEST" flag in the controle flag register.
  l1s.dsp_ctrl_reg |= CTRL_TEST;
}

/*-------------------------------------------------------*/
/* l1s_read_hwtest()                                     */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* task HWTEST. This function read the checksum of the   */
/* DSP.                                                  */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/* "l1a_l1s_com.l1s_en_task"                             */
/*   L1S task enable bit register.                       */
/*   -> disable HWTEST task.                             */
/*                                                       */
/* "l1s.task_status[HWTEST].current_status"              */
/*   current task status for HWTEST task.                */
/*   -> disactivate HWTEST task.                         */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_read_hwtest(UWORD8 task, UWORD8 param2)
{
  #if (TRACE_TYPE==2) || (TRACE_TYPE==3)//OMAPS00090550
  UWORD32 flash_type = 0;
  #endif
  xSignalHeaderRec *msg;

  #if (CODE_VERSION != SIMULATION)
    #if (DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39)
      l1s.version.dsp_code_version  = l1s_dsp_com.dsp_ndb_ptr->d_version_number1;
      l1s.version.dsp_checksum      = (UWORD16) (l1s_dsp_com.dsp_db_r_ptr->a_pm[1] & 0xffff);
      l1s.version.dsp_patch_version = l1s_dsp_com.dsp_ndb_ptr->d_version_number2;
    #else
      l1s.version.dsp_code_version  = (UWORD16) (l1s_dsp_com.dsp_db_r_ptr->a_pm[0] & 0xffff);
      l1s.version.dsp_checksum      = (UWORD16) (l1s_dsp_com.dsp_db_r_ptr->a_pm[1] & 0xffff);
    //l1s.version.dsp_patch_version = (UWORD16) (l1s_dsp_com.dsp_db_r_ptr->a_pm[2] & 0xffff);
      l1s.version.dsp_patch_version = (UWORD32) *((API *) SC_CHKSUM_VER);
      //NOTE: dsp_patch_version is duplicated in d_version_number
    #endif
  #endif // NOT_SIMULATION

  // send L1_INIT_HW_CON to L1A...
  msg = os_alloc_sig(sizeof(T_TST_TEST_HW_CON));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = L1_TEST_HW_INFO;

  // added for the new naming convention
  ((T_TST_TEST_HW_CON*)(msg->SigP))->dsp_code_version  = l1s.version.dsp_code_version;
  ((T_TST_TEST_HW_CON*)(msg->SigP))->dsp_checksum      = l1s.version.dsp_checksum;
  ((T_TST_TEST_HW_CON*)(msg->SigP))->dsp_patch_version = l1s.version.dsp_patch_version;
  ((T_TST_TEST_HW_CON*)(msg->SigP))->mcu_tcs_program_release = l1s.version.mcu_tcs_program_release;
  ((T_TST_TEST_HW_CON*)(msg->SigP))->mcu_tcs_official        = l1s.version.mcu_tcs_official;
  ((T_TST_TEST_HW_CON*)(msg->SigP))->mcu_tcs_internal        = l1s.version.mcu_tcs_internal;

  os_send_sig(msg, L1C1_QUEUE);

  DEBUGMSG(status,NU_SEND_QUEUE_ERR)

  #if (TRACE_TYPE==2) || (TRACE_TYPE==3)
    uart_trace_checksum(flash_type);
  #endif

  // HWTEST task is completed, make it INACTIVE.
  // It is a 1 shot task, it must be also disabled in L1S.
  l1s.task_status[task].current_status = INACTIVE;
  l1a_l1s_com.l1s_en_task[HWTEST] = TASK_DISABLED;

  // Flag the use of the MCU/DSP dual page read interface.
  // ******************************************************

  // Set flag used to change the read page at the end of "l1_synch" only.
  l1s_dsp_com.dsp_r_page_used = TRUE;
}

/*-------------------------------------------------------*/
/* l1s_new_synchro()                                     */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* task SYNCHRO. This function mainly adapts the L1/TPU  */
/* timebase to a new setting. This new setting can come  */
/* from a timeslot change or a full change of serving    */
/* cell. This change is a big discontinuity, it requires */
/* some global variable reset. Here is a summary of the  */
/* execution:                                            */
/*                                                       */
/*  - Traces for debug.                                  */
/*  - Disables the L1S task SYNCHRO (SYNCHRO is 1 shot)  */
/*    and make it inactive (current status set to        */
/*    INACTIVE).                                         */
/*  - Compute timeshift.                                 */
/*  - Program serving cell fine timeshift for TPU.       */
/*  - Execute serving cell frame number timeshift.       */
/*  - Flag TPU programmation.                            */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/* "l1a_l1s_com.tn_difference"                           */
/*   timeslot difference between new and old setting.    */
/*   This is used when timeshift is due to a change of   */
/*   timeslot but on the same serving cell.              */
/*   -> reset to 0.                                      */
/*                                                       */
/* "l1a_l1s_com.Scell_inf.time_alignmt"                  */
/*   fine time difference between current setting and    */
/*   new setting to achieve.                             */
/*   -> reset to 0.                                      */
/*                                                       */
/* "l1a_l1s_com.Scell_inf.fn_offset"                     */
/*   frame number offset between current setting and new */
/*   setting to achieve.                                 */
/*   -> reset to 0.                                      */
/*                                                       */
/* "l1s.tpu_offset"                                      */
/*   value for SYNCHRO and OFFSET register in the TPU    */
/*   for current serving cell setting.                   */
/*   -> set to the new setting.                          */
/*                                                       */
/* "l1a_l1s_com.l1s_en_task"                             */
/*   L1S task enable bit register.                       */
/*   -> disable SYNCHRO task.                            */
/*                                                       */
/* "l1s.task_status[SYNCHRO].current_status"             */
/*   current task status for SYNCHRO task.               */
/*   -> disactivate SYNCHRO task.                        */
/*                                                       */
/* "l1s.actual_time, l1s.next_time"                      */
/*   frame number and derived numbers for current frame  */
/*   and next frame.                                     */
/*   -> update to new setting.                           */
/*                                                       */
/* "l1s.tpu_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/TPU        */
/*   interface. This is used mainly to swap then the     */
/*   com. page at the end of a control frame.            */
/*   -> set CTRL_SYNC bit in the register.               */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_new_synchro(UWORD8 param1, UWORD8 param2)
{
  WORD32       offset;
  UWORD32      tpu_offset_shift;
  T_CELL_INFO *sptr = &(l1a_l1s_com.Scell_info);

  // Traces for debug mainly used during L1 simulation.
  // ***************************************************

  #if (TRACE_TYPE!=0)
    trace_fct(CST_L1S_NEW_SYNCHRO, l1a_l1s_com.Scell_info.radio_freq);
  #endif

  #if (TRACE_TYPE==5) && FLOWCHART
    trace_flowchart_tpu(dltsk_trace[SYNCHRO].name);
  #endif

  // Disable SYNCHRO task.
  // **********************

  // SYNCHRO task is a one shot task enabled by L1A and
  // disables after its execution in L1S. Here is the disabling.
  l1a_l1s_com.l1s_en_task[SYNCHRO] = TASK_DISABLED;
  l1s.task_status[SYNCHRO].current_status = INACTIVE;

  #if 0	/* FreeCalypso TCS211 reconstruction */
  #if L1_GPRS
  //Change of mode when synchro is executed when switching from idle to transfer
  //In this case, PDTCH task has been enabled in transfer mode manager, but the mode is still not PACKET_TRANSFER_MODE
  if((l1a_l1s_com.l1s_en_task[PDTCH] == TASK_ENABLED) && (l1a_l1s_com.mode != PACKET_TRANSFER_MODE))
    l1a_l1s_com.mode = PACKET_TRANSFER_MODE;
  #endif
  #endif

  // Compute timeshift.
  // *******************

  if(l1a_l1s_com.tn_difference < 0)
  // "tn_difference" field is not 0 only when the new serving cell is the same
  // as the old one. Therefore, we are just changing the timeslot.
  // If the new timeslot if lower than the old one then the serving FN must
  // be incremented by 1. To do so, we use the "fn_offset" field which is
  // loaded with "1".
  {
    sptr->fn_offset += 1;
    l1a_l1s_com.tn_difference += 8;
  }

  // update the TPU with the new TOA if necessary
  l1ctl_update_TPU_with_toa();

  // Manage shifting value for TPU offset register...
  // if staying on the same serving cell but changing the RX timeslot (CCCH_GROUP or timeslot),
  // then the "timeslot difference" between old and new configuration is given in "tn_difference",
  // else "tn_difference" must contain 0.
  tpu_offset_shift = (sptr->time_alignmt) + (l1a_l1s_com.tn_difference * BP_DURATION);

  // Clear "timeslot difference" parameter.
  l1a_l1s_com.tn_difference = 0;

  // Get FN difference between actual synchro and the one we are going to switch to.
  // The switch (slide of OFFSET and REG_COM_INT) is performed at the time "OFFSET - epsilon".
  // If "tpu_offset_shift" is greater than "OFFSET - epsilon (called SWITCH_TIME)" then
  // the next interrupt is going to occur very soon after the switch, and new FN comes directly
  // from current FN + the "fn_offset" (minus 1 since FN has just been incremented). Else 1 frame
  // is missed and new FN comes from "fn_offset + 1" (minus 1 since FN has just been incremented).
  offset = sptr->fn_offset - 1;
  if(tpu_offset_shift <= SWITCH_TIME) offset++;
  #if L1_FF_WA_OMAPS00099442
    if(l1a_l1s_com.change_tpu_offset_flag == TRUE){
      l1s.tpu_offset = (l1s.tpu_offset + (TPU_CLOCK_RANGE >> 1) ) % TPU_CLOCK_RANGE;
      l1a_l1s_com.change_tpu_offset_flag = FALSE;
    }
  #endif

  // Shift "tpu_offset" accordingly to the computed "tpu_offset_shift" value.
  // Rem: "%" is required since the result value can be greater than 2*TPU_CLOCK_RANGE.
  l1s.tpu_offset = (l1s.tpu_offset + tpu_offset_shift) % TPU_CLOCK_RANGE;

  // Program serving cell fine timeshift for TPU.
  // *********************************************

  // Store the fine time shifting program in the MCU/TPU com.
  l1dmacro_synchro(SWITCH_TIME, l1s.tpu_offset);

  // Execute serving cell frame number timeshift.
  // *********************************************

  // Slide frame numbers and derived numbers to jump on new setting.
  l1s_increment_time(&(l1s.actual_time), offset); // Update actual_time.

  l1s.next_time      = l1s.actual_time;
  l1s_increment_time(&(l1s.next_time), 1);        // Next time is actual_time + 1

  #if L1_GPRS
    l1s.next_plus_time = l1s.next_time;
    l1s_increment_time(&(l1s.next_plus_time), 1); // Next_plus time is next_time + 1
  #endif

  #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
    trace_fct(CST_L1S_ADJUST_TIME, (UWORD32)(-1));//OMAPS00090550
  #endif

  #if (TOA_ALGO == 2)
    // Fix in case of handovers and Test Mode
    l1s.toa_var.toa_update_fn = l1s.toa_var.toa_update_fn + offset;
    if(l1s.toa_var.toa_update_fn >= MAX_FN)
    {
      l1s.toa_var.toa_update_fn-= MAX_FN;
    }
  #endif



  // the FN was changed: it could have an impact on the gauging algorithm
        //Nina modify to save power, not forbid deep sleep, only force gauging in next paging
	/* FreeCalypso Frankenstein: see l1_async.c regarding Nina's change */
#define	NINA_ADDED	0
#if NINA_ADDED
if(l1s.force_gauging_next_paging_due_to_CCHR != 1)
#endif
{
l1s.pw_mgr.enough_gaug = FALSE;  // forbid Deep sleep until next gauging
}


  // Clear Serving offset and bob.
  sptr->fn_offset    = 0;
  sptr->time_alignmt = 0;

  // Flag TPU programmation.
  // ************************

  // Set "CTRL_SYNC" flag in the controle flag register.
  l1s.tpu_ctrl_reg |= CTRL_SYNC;

  #if (CODE_VERSION == SIMULATION)
    si_scheduling(MPHC_SCELL_NBCCH_REQ, NULL, TRUE);
    si_scheduling(MPHC_SCELL_EBCCH_REQ, NULL, TRUE);
  #endif

  #if TESTMODE
    // Continuous mode: if we are in continuous mode: return to the no continuous mode.
    if ((l1_config.TestMode) && (l1_config.tmode.rf_params.tmode_continuous == TM_CONTINUOUS))
      l1_config.tmode.rf_params.tmode_continuous = TM_NO_CONTINUOUS;
  #endif

  #if L1_GPRS
    // Signals the GSM->GPRS or GPRS->GSM switch to the DSP.
    // ******************************************************
    l1pddsp_synchro(l1a_l1s_com.dsp_scheduler_mode, l1a_l1s_com.dl_tn);

    // Flag DSP programmation.
    // Set "CTRL_SYNC" flag in the controle flag register.
    l1s.dsp_ctrl_reg |= CTRL_SYNC;
  #endif

  #if (CODE_VERSION == SIMULATION)
    si_scheduling(MPHC_SCELL_NBCCH_REQ, NULL, TRUE);
    si_scheduling(MPHC_SCELL_EBCCH_REQ, NULL, TRUE);
  #endif
}

/*-------------------------------------------------------*/
/* l1s_ctrl_ADC()                                        */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* task ADC. This function program the L1/TPU in order   */
/* to perform an ADC measurement in CS_MODE0             */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_ctrl_ADC(UWORD8 param1, UWORD8 param2)
{
  // Traces and debug.
  // ******************

  #if (TRACE_TYPE!=0)
    trace_fct(CST_L1S_CTRL_ADC, (UWORD32)(-1));//OMAPS00090550
  #endif

  #if (TRACE_TYPE==5) && FLOWCHART
    trace_flowchart_dsp_tpu(dltsk_trace[task].name);
  #endif

  //  In CS_MODE0, MPHC_RXLEV_REQ is not received periodically. In case network is not found,
  //the period between 2 MPHC_RXLEV_REQ increases and can be as high as 360 seconds (Max Value)
  // To enable MADC periodically, the function l1dmacro_adc_read_rx_cs_mode0; is called
  #if (CODE_VERSION != SIMULATION)
     #if (L1_MADC_ON ==1)
            if (l1a_l1s_com.mode == CS_MODE0)
                    l1dmacro_adc_read_rx_cs_mode0();
          else
                 l1dmacro_adc_read_rx(); // ADC performed into a rx scenario to have BULON and BULENA signals off so maintaining
                         // low power consumption on ABB
     #else
           l1dmacro_adc_read_rx(); // ADC performed into a rx scenario to have BULON and BULENA signals off so maintaining
                         // low power consumption on ABB
      #endif  // End of L1_MADC_ON == 1
  #else
            l1dmacro_adc_read_rx(); // ADC performed into a rx scenario to have BULON and BULENA signals off so maintaining
                         // low power consumption on ABB
  #endif


  l1s.task_status[ADC_CSMODE0].current_status = INACTIVE;

  if (l1a_l1s_com.adc_mode & ADC_NEXT_CS_MODE0)  // performe ADC only one time
  {
     l1a_l1s_com.adc_mode &= ADC_MASK_RESET_IDLE; // reset in order to have only one ADC measurement in CS_MODE0
     l1a_l1s_com.l1s_en_task[ADC_CSMODE0] = TASK_DISABLED; // disable the ADC task in case of one shot
  }

  // Set "CTRL_MS" flag in the controle flag register.
  l1s.tpu_ctrl_reg |= CTRL_ADC;
}

#if !((MOVE_IN_INTERNAL_RAM == 1) && (GSM_IDLE_RAM !=0))  // MOVE TO INTERNAL MEM IN CASE GSM_IDLE_RAM enabled
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_START         // KEEP IN EXTERNAL MEM otherwise

/*-------------------------------------------------------*/
/* l1s_abort()                                           */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* task ABORT. When the L1S merge manager routine,       */
/* "l1s_merge_manager()", finds a conflict between a     */
/* running task and a pending task, it can come to       */
/* aborting the running one to start executing the       */
/* pending. Here is the routine which resets the comm.   */
/* between the MCU and the DSP and TPU. The DSP is also  */
/* signaled to abort any ongoing task. Here is a summary */
/* of the execution:                                     */
/*                                                       */
/*  - Traces for debug.                                  */
/*  - Reset MCU/DSP and MCU/TPU communications.          */
/*  - Signals the ABORT process to the DSP.              */
/*  - Flag DSP programmation.                            */
/*                                                       */
/* Input parameters from globals:                        */
/* ------------------------------                        */
/* "l1s.tpu_offset"                                      */
/*   OFFSET/SYNCHRO registers value for current serving  */
/*   cell setting.                                       */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/* "l1s.dsp_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/DSP        */
/*   interface. This is used mainly to swap then the     */
/*   com. page at the end of a control frame.            */
/*   -> set CTRL_ABORT bit in the register.              */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_abort(UWORD8 param1, UWORD8 param2)
{
  // Traces for debug.
  // ******************

  #if (TRACE_TYPE==5)
    trace_fct(CST_L1S_ABORT, l1a_l1s_com.Scell_info.radio_freq);
  #endif

  #if (TRACE_TYPE!=0) && (TRACE_TYPE!=5)
    trace_fct(CST_L1S_ABORT_W0_R0, (UWORD32)(-1));//OMAPS00090550
  #endif

  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
    if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1S_DEBUG)
      Trace_L1s_Abort(trace_info.abort_task);
  #endif

  // Reset MCU/DSP and MCU/TPU communications.
  // ******************************************

  // Reset Hardware...
  // Set "tpu_reset_bit" to 1.
  // Reset DSP write/read page.
  // Reset communication pointers.
  // Immediate Reload offset with Serving one.
  l1d_reset_hw(l1s.tpu_offset);

  l1s_dsp_com.dsp_db_w_ptr->d_debug = (l1s.debug_time + 2) ;

  // Signals the ABORT process to the DSP.
  // **************************************

  // Set "b_abort" to TRUE, dsp will reset current and pending tasks.
  l1s_dsp_com.dsp_db_w_ptr->d_ctrl_system |= (1 << B_TASK_ABORT);


  // Tasks are aborted on DSP side => forbid measurements during ABORT
  l1s.forbid_meas = 1;

  // Flag DSP programmation.
  // ************************

  // Set "CTRL_ABORT" flag in the controle flag register.
  l1s.dsp_ctrl_reg |= CTRL_ABORT;
#if (FF_L1_FAST_DECODING == 1)
  /* Reset fast decoding */
  l1a_apihisr_com.fast_decoding.status = C_FAST_DECODING_NONE;
  l1a_apihisr_com.fast_decoding.contiguous_decoding = FALSE;
#endif /* #if (FF_L1_FAST_DECODING == 1) */
}

//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END
#endif

/*-------------------------------------------------------*/
/* l1s_ctrl_msagc()                                      */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* tasks: BCCHN,FBNEW,SB1,SB2,SBCONF. This function is   */
/* the control function for making a power measurement   */
/* for refreshing the AGC for those tasks. It programs   */
/* the DSP and the TPU for making 1 measurement in the   */
/* next frame. Here is a summary of the execution:       */
/*                                                       */
/*  - If SEMAPHORE(task) is low.                         */
/*    - Get the cell information structure.              */
/*    - Traces and debug.                                */
/*    - Programs DSP for measurement task.               */
/*    - Programs TPU for measurement task.               */
/*  - Flag DSP and TPU programmation.                    */
/*                                                       */
/* Input parameters:                                     */
/* -----------------                                     */
/* "task"                                                */
/*   BCCHN, BCCH Neighbor reading task.                  */
/*   FBNEW, Frequency Burst detection task in Idle mode. */
/*   SB1, Synchro Burst reading task in Idle mode.       */
/*   SB2, Synchro Burst detection task in Idle mode.     */
/*   SBCONF, Synchro Burst confirmation task in Idle     */
/*   mode.                                               */
/*                                                       */
/* Input parameters from globals:                        */
/* ------------------------------                        */
/* "l1s.afc"                                             */
/*   current AFC value to be applied for FBNEW and SB2   */
/*   tasks in Cell Selection only.                       */
/*                                                       */
/* "l1a_l1s_com.task_param"                              */
/*   task semaphore bit register. Used to skip this      */
/*   control if L1A has changed or is changing some of   */
/*   the task parameters.                                */
/*                                                       */
/* "l1a_l1s_com.Ncell_info.bcch"                         */
/* "l1a_l1s_com.Ncell_info.acquis"                       */
/* "l1a_l1s_com.Ncell_info.acquis"                       */
/* "l1a_l1s_com.Ncell_info.conf"                         */
/*   cell information structure used for BCCHN,FBNEW,    */
/*   SB1/SB2,SBCONF respectively.                        */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/* "l1s.tpu_win"                                         */
/*   each frame is composed with a maximum of 3          */
/*   working/TPU windows (typically RX/TX/PW). This is   */
/*   a counter used to count the number of windows       */
/*   used.                                               */
/*   -> incremented.                                     */
/*                                                       */
/* "l1s.tpu_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/TPU        */
/*   interface. This is used mainly to swap then the     */
/*   com. page at the end of a control frame.            */
/*   -> set CTRL_RX bit in the register.                 */
/*                                                       */
/* "l1s.dsp_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/DSP        */
/*   interface. This is used mainly to swap then the     */
/*   com. page at the end of a control frame.            */
/*   -> set CTRL_RX bit in the register.                 */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_ctrl_msagc(UWORD8 task, UWORD8 param2)
{
  #if (RF_FAM == 61)
      UWORD16 dco_algo_ctl_pw = 0;
      UWORD8 if_ctl = 0;
//OMAPS00090550	  UWORD8 if_threshold = C_IF_ZERO_LOW_THRESHOLD_GSM;
  #endif

  if(!(l1a_l1s_com.task_param[task]))
  // Check the task semaphore. The control body is executed only
  // when the task semaphore is 0. This semaphore can be set to
  // 1 whenever L1A makes some changes to the task parameters.
  {
    T_NCELL_SINGLE *cell_info_ptr = NULL;
#if (L1_GPRS)
    T_NCELL_SINGLE pbcchn_cell_info;
#endif
#if ((REL99 == 1) && (FF_BHO == 1))
    T_NCELL_SINGLE bho_cell_info;
#endif

    // Get the cell information structure.
    // ************************************

    switch(task)
    {
      case BCCHN:    cell_info_ptr = &l1a_l1s_com.bcchn.list[l1a_l1s_com.bcchn.active_neigh_id_norm];break;
      case BCCHN_TOP:cell_info_ptr = &l1a_l1s_com.bcchn.list[l1a_l1s_com.bcchn.active_neigh_id_top];break;
      case FBNEW:    cell_info_ptr = &l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_fb_id];    break;
      case SB2:      cell_info_ptr = &l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_sb_id];    break;
      case SBCONF:   cell_info_ptr = &l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_sbconf_id];break;

#if ((REL99 == 1) && (FF_BHO == 1))
      case FBSB:
      {
        cell_info_ptr = &bho_cell_info;
        bho_cell_info.radio_freq = l1a_l1s_com.nsync_fbsb.radio_freq;
        bho_cell_info.fn_offset  = l1a_l1s_com.nsync_fbsb.fn_offset;
      }
      break;
#endif

      #if (L1_GPRS)
        case PBCCHN_IDLE:
        {
          cell_info_ptr               = &pbcchn_cell_info;
          pbcchn_cell_info.radio_freq = l1pa_l1ps_com.pbcchn.bcch_carrier;
          pbcchn_cell_info.fn_offset  = l1pa_l1ps_com.pbcchn.fn_offset;
        }
        break;
      #endif
      default: return;
    }

    // Traces and debug.
    // ******************

    #if (TRACE_TYPE!=0)
      trace_fct(CST_L1S_CTRL_MSAGC, cell_info_ptr->radio_freq);
    #endif

    l1s_dsp_com.dsp_db_w_ptr->d_debug = (l1s.debug_time + 2) ;


    // Programs DSP for measurement task.
    // ***********************************

    // Dsp pgm... (2 measurement).
    #if L1_GPRS
      switch (l1a_l1s_com.dsp_scheduler_mode)
      {
        case GPRS_SCHEDULER:
        {
          l1pddsp_meas_ctrl(2,0);
        } break;

        case GSM_SCHEDULER:
        {
          l1ddsp_load_monit_task(2, 0);
        } break;
      }
    #else
      l1ddsp_load_monit_task(2, 0);
    #endif

  #if (RF_FAM == 61)
   #if (PWMEAS_IF_MODE_FORCE == 0)
      cust_get_if_dco_ctl_algo (&dco_algo_ctl_pw, &if_ctl,
          (UWORD8) L1_IL_INVALID, 0,
          cell_info_ptr->radio_freq,C_IF_ZERO_LOW_THRESHOLD_GSM);//OMAPS00090550
    #else
      if_ctl = IF_120KHZ_DSP;
      dco_algo_ctl_pw = DCO_IF_0KHZ;
    #endif

    // Duplicate the outcome of DCO control as there are 2 PM
    dco_algo_ctl_pw = (((dco_algo_ctl_pw<<2) & 0x0C) | (dco_algo_ctl_pw & 0x03)); // 0000ZLZL
    l1ddsp_load_dco_ctl_algo_pw(dco_algo_ctl_pw);
  #endif

    // Programs TPU for measurement task.
    // ***********************************
         // tpu pgm: measurement only.
     if (task == FBNEW)
     {
         l1dtpu_meas(cell_info_ptr->radio_freq,
                     l1_config.params.high_agc,
                     0,                                 // 0 is set for lna_off = 0
                     l1s.tpu_win,
                     l1s.tpu_offset, INACTIVE
     #if(RF_FAM == 61)
                    ,L1_AFC_NONE
                    ,if_ctl
     #endif
     	                      );
     }
     else
     {
    	     l1dtpu_meas(cell_info_ptr->radio_freq,
                     l1_config.params.high_agc,
                     0,                                 // 0 is set for lna_off = 0
                     l1s.tpu_win,
                     l1s.tpu_offset, INACTIVE
     #if(RF_FAM == 61)
                    ,L1_AFC_SCRIPT_MODE
                    ,if_ctl
     #endif
     	                      );
     }

    #if L2_L3_SIMUL
      #if (DEBUG_TRACE == BUFFER_TRACE_OFFSET_NEIGH)
        buffer_trace(4, l1s.actual_time.fn, cell_info_ptr->radio_freq,
                        cell_info_ptr->fn_offset, l1s.tpu_win);
      #endif
    #endif

if (task == FBNEW)
{
    // Increment tpu window identifier.
    l1s.tpu_win += (l1_config.params.rx_synth_load_split + PWR_LOAD);

    // tpu pgm: measurement only.
    l1dtpu_meas(cell_info_ptr->radio_freq,
                l1_config.params.low_agc,
                0,                          // 0 is set for lna_off = 0
                l1s.tpu_win,
                l1s.tpu_offset,
                INACTIVE
#if(RF_FAM == 61)
                ,L1_AFC_SCRIPT_MODE
                ,if_ctl
#endif
	);
}
else
{
    // Increment tpu window identifier.
    l1s.tpu_win += (l1_config.params.rx_synth_load_split + PWR_LOAD);

    // tpu pgm: measurement only.
    l1dtpu_meas(cell_info_ptr->radio_freq,
                l1_config.params.low_agc,
                0,                          // 0 is set for lna_off = 0
                l1s.tpu_win,
                l1s.tpu_offset,
                INACTIVE
#if(RF_FAM == 61)
                ,L1_AFC_SCRIPT_MODE
                ,if_ctl
#endif
	);
}
    // Increment tpu window identifier.
    l1s.tpu_win += (l1_config.params.rx_synth_load_split + PWR_LOAD);
  }

  // Flag DSP and TPU programmation.
  // ********************************

  // Set "CTRL_MS" flag in the controle flag register.
  l1s.tpu_ctrl_reg |= CTRL_MS;
  l1s.dsp_ctrl_reg |= CTRL_MS;

  // This task is not compatible with Neigh. Measurement. Store task length
  // in "forbid_meas" to indicate when the task will last.
  l1s.forbid_meas = TASK_ROM_MFTAB[task].size;
}

/*-------------------------------------------------------*/
/* l1s_ctrl_fb()                                         */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* tasks: FBNEW,FB51. This function is the control       */
/* function for making a frequency burst acquisition on  */
/* a neighbor cell. It programs the DSP and the TPU for  */
/* making 1 attempt in reading the frequency burst.Here  */
/* is a summary of the execution:                        */
/*                                                       */
/*  - If SEMAPHORE(task) is low.                         */
/*    - Traces and debug.                                */
/*    - Programs DSP for FB acquisition task.            */
/*    - Programs TPU for FB acquisition task.            */
/*  - Flag DSP and TPU programmation.                    */
/*                                                       */
/* Input parameters:                                     */
/* -----------------                                     */
/* "task"                                                */
/*   FBNEW, Frequency Burst detection task in Idle mode. */
/*   FB51, Frequency Burst detection task in Dedicated   */
/*   mode.                                               */
/*                                                       */
/* Input parameters from globals:                        */
/* ------------------------------                        */
/* "l1a_l1s_com.task_param"                              */
/*   task semaphore bit register. Used to skip this      */
/*   control if L1A has changed or is changing some of   */
/*   the task parameters.                                */
/*                                                       */
/* "l1a_l1s_com.Ncell_info.acquis"                       */
/*   cell information structure used for FBNEW and FB51  */
/*   tasks.                                              */
/*                                                       */
/* "l1a_l1s_com.fb_mode"                                 */
/*   the frequency burst detection algorithm implemented */
/*   in the DSP uses 2 different modes. The mode to use  */
/*   is indicated by this global variable and is passed  */
/*   to the DSP.                                         */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/* "l1s.tpu_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/TPU        */
/*   interface. This is used mainly to swap then the     */
/*   com. page at the end of a control frame.            */
/*   -> set CTRL_RX bit in the register.                 */
/*                                                       */
/* "l1s.dsp_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/DSP        */
/*   interface. This is used mainly to swap then the     */
/*   com. page at the end of a control frame.            */
/*   -> set CTRL_RX bit in the register.                 */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_ctrl_fb(UWORD8 task, UWORD8 param2)
{
  WORD8    agc;
  UWORD8   lna_off;
  BOOL     en_task;
  BOOL     task_param;
  UWORD32  dsp_task;
#if (L1_FF_MULTIBAND == 1)
  UWORD16  operative_radio_freq;
#endif


  // Get "enable" task flag and "synchro semaphore" for current task.
  en_task    = l1a_l1s_com.l1s_en_task[task];
  task_param = l1a_l1s_com.task_param[task];

  if((en_task) && !(task_param))
  // Check the task semaphore and enable flag. The control body is executed only
  // when the task semaphore is 0 and enable flag is 1. The semaphore can be set to
  // 1 whenever L1A makes some changes to the task parameters. The enable can be
  // reset to 0 when the task is no more enabled.
  {
    T_NCELL_SINGLE  *cell_info_ptr = NULL;

    // Get the cell information structure.
    // ************************************
    cell_info_ptr = &l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_fb_id];

    // Traces and debug.
    // ******************

    #if (TRACE_TYPE!=0)
      trace_fct(CST_L1S_CTRL_FB, cell_info_ptr->radio_freq);
    #endif

    #if (TRACE_TYPE==5) && FLOWCHART
      trace_flowchart_dsp_tpu(dltsk_trace[task].name);
    #endif

    l1s_dsp_com.dsp_db_w_ptr->d_debug = (l1s.debug_time + 13) ;

    // Programs DSP for required task.
    // ********************************

    // dsp pgm...

    dsp_task = l1s_swap_iq_dl(cell_info_ptr->radio_freq,task);

    l1ddsp_load_monit_task(dsp_task, l1a_l1s_com.fb_mode);

    // Programs TPU for required task.
    // ********************************
#if (L1_FF_MULTIBAND == 0)

    // Get AGC to be applied.
    agc = Cust_get_agc_from_IL(cell_info_ptr->radio_freq,l1a_l1s_com.last_input_level[cell_info_ptr->radio_freq - l1_config.std.radio_freq_index_offset].input_level >> 1, AV_ID);
    // lna_off flag is updated ONLY in case of l1ctl_pgc2 control algo
    lna_off = l1a_l1s_com.last_input_level[cell_info_ptr->radio_freq - l1_config.std.radio_freq_index_offset].lna_off;

#else // L1_FF_MULTIBAND = 1 below

    operative_radio_freq = l1_multiband_radio_freq_convert_into_operative_radio_freq(cell_info_ptr->radio_freq );
    // lna_off flag is updated ONLY in case of l1ctl_pgc2 control algo
    lna_off = l1a_l1s_com.last_input_level[operative_radio_freq].lna_off;
    // Get AGC to be applied.
    agc = Cust_get_agc_from_IL(cell_info_ptr->radio_freq,l1a_l1s_com.last_input_level[operative_radio_freq].input_level >> 1, AV_ID);
    
#endif // #if (L1_FF_MULTIBAND == 0) else


    // tpu pgm...
    l1dtpu_neig_fb(cell_info_ptr->radio_freq, agc, lna_off);
  }

  // Flag DSP and TPU programmation.
  // ********************************

  // Set "CTRL_RX" flag in the controle flag register.
  l1s.tpu_ctrl_reg |= CTRL_RX;
  l1s.dsp_ctrl_reg |= CTRL_RX;

  // This task is not compatible with Neigh. Measurement. Store task length
  // in "forbid_meas" to indicate when the task will last.
  // Rem: Only FB51 task starts from this ctrl function.
  if(task==FB51) l1s.forbid_meas = TASK_ROM_MFTAB[task].size;
}


/*-------------------------------------------------------*/
/* l1s_ctrl_fbsb()                                      */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* tasks: FBSB. This function is the control             */
/* function for making a frequency + synchro burst       */
/* on a neighbor cell in case of blind handover          */
/* It programs the DSP and the TPU for                   */
/* making 1 attempt in reading the frequency & synchro   */
/* burst                                                 */
/* Here is a summary of the execution:                   */
/*                                                       */
/*  - If SEMAPHORE(task) is low.                         */
/*    - Traces and debug.                                */
/*    - Programs DSP for FB+SB acquisition task.         */
/*    - Programs TPU for FB+SB acquisition task.         */
/*  - Flag DSP and TPU programmation.                    */
/*                                                       */
/* Input parameters:                                     */
/* -----------------                                     */
/* "task"                                                */
/*   FBSB, Frequency + Synchro burst detection task in   */
/*        blind handover                                 */
/*                                                       */
/* Input parameters from globals:                        */
/* ------------------------------                        */
/* "l1a_l1s_com.task_param"                              */
/*   task semaphore bit register. Used to skip this      */
/*   control if L1A has changed or is changing some of   */
/*   the task parameters.                                */
/*                                                       */
/* "l1a_l1s_com.nsync_fbsb"                              */
/*   cell information structure used for FBSB tasks.     */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/* "l1s.tpu_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/TPU        */
/*   interface. This is used mainly to swap then the     */
/*   com. page at the end of a control frame.            */
/*   -> set CTRL_RX bit in the register.                 */
/*                                                       */
/* "l1s.dsp_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/DSP        */
/*   interface. This is used mainly to swap then the     */
/*   com. page at the end of a control frame.            */
/*   -> set CTRL_RX bit in the register.                 */
/*                                                       */
/*-------------------------------------------------------*/
#if ((REL99 == 1) && (FF_BHO == 1))
void l1s_ctrl_fbsb(UWORD8 task, UWORD8 param2)
{

  WORD8    agc;
  UWORD8   lna_off;
  UWORD32  dsp_task;
  //added by sajal for DCXO
  UWORD8 input_level;
  #if (RF_FAM == 61)
  UWORD16 dco_algo_ctl_sb = 0;
  UWORD8 if_ctl = 0;
  UWORD8 if_threshold = C_IF_ZERO_LOW_THRESHOLD_GSM;
  #endif
#if (L1_FF_MULTIBAND == 1)
  UWORD16 operative_radio_freq;
#endif
  


  // Traces and debug.
  // ******************

#if (TRACE_TYPE!=0)
  //  trace_fct(CST_L1S_CTRL_FBSB, l1a_l1s_com.nsync_fbsb.radio_freq);
#endif

#if (TRACE_TYPE==5) && FLOWCHART
  trace_flowchart_dsp_tpu(dltsk_trace[task].name);
#endif

  l1s_dsp_com.dsp_db_w_ptr->d_debug = (l1s.debug_time + 13) ;

  // Programs DSP for required task.
  // ********************************

  // dsp pgm...

  dsp_task = l1s_swap_iq_dl(l1a_l1s_com.nsync_fbsb.radio_freq, task);

  l1ddsp_load_monit_task(dsp_task, 1);
#if (L1_FF_MULTIBAND == 0)
  input_level = l1a_l1s_com.last_input_level[l1a_l1s_com.nsync_fbsb.radio_freq - l1_config.std.radio_freq_index_offset].input_level;
#else
	operative_radio_freq = 
	  l1_multiband_radio_freq_convert_into_operative_radio_freq(l1a_l1s_com.nsync_fbsb.radio_freq);
	input_level = 
	  l1a_l1s_com.last_input_level[operative_radio_freq].input_level;
#endif

   #if (RF_FAM == 61)   // Locosto DCO
        cust_get_if_dco_ctl_algo(&dco_algo_ctl_sb, &if_ctl, (UWORD8) L1_IL_VALID ,
                                           input_level,
                                             l1a_l1s_com.nsync_fbsb.radio_freq,if_threshold);

	l1ddsp_load_dco_ctl_algo_sb(dco_algo_ctl_sb);
   #endif

  // Programs TPU for required task.
  // ********************************
#if (L1_FF_MULTIBAND == 0)
  
  // lna_off flag is updated ONLY in case of l1ctl_pgc2 control algo
  lna_off = l1a_l1s_com.last_input_level[l1a_l1s_com.nsync_fbsb.radio_freq - l1_config.std.radio_freq_index_offset].lna_off;

  // Get AGC to be applied.
  agc = Cust_get_agc_from_IL(l1a_l1s_com.nsync_fbsb.radio_freq, l1a_l1s_com.last_input_level[l1a_l1s_com.nsync_fbsb.radio_freq - l1_config.std.radio_freq_index_offset].input_level >> 1, AV_ID);

#else // L1_FF_MULTIBAND = 1 below

  /*operative_radio_freq = 
    l1_multiband_radio_freq_convert_into_operative_radio_freq(l1a_l1s_com.nsync_fbsb.radio_freq);*/

  // lna_off flag is updated ONLY in case of l1ctl_pgc2 control algo
  lna_off = l1a_l1s_com.last_input_level[operative_radio_freq].lna_off;

  // Get AGC to be applied.
  agc = 
    Cust_get_agc_from_IL(l1a_l1s_com.nsync_fbsb.radio_freq, l1a_l1s_com.last_input_level[operative_radio_freq].input_level >> 1, AV_ID);

#endif // #if (L1_FF_MULTIBAND == 0) else 

  // tpu pgm...

  l1dtpu_neig_fbsb(l1a_l1s_com.nsync_fbsb.radio_freq, agc, lna_off);

  // Disable Task
// FTH  l1a_l1s_com.l1s_en_task[FBSB] = TASK_DISABLED;

  // Flag DSP and TPU programmation.
  // ********************************

  // Set "CTRL_RX" flag in the controle flag register.
  l1s.tpu_ctrl_reg |= CTRL_RX;
  l1s.dsp_ctrl_reg |= CTRL_RX;
}
#endif //#if ((REL99 == 1) && (FF_BHO == 1))



/*-------------------------------------------------------*/
/* l1s_ctrl_sbgen()                                      */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* tasks: SB1,SB2,SB51,SBCONF,SBCNF51. This function is  */
/* the control function for making a synchro burst       */
/* reading on a neighbor cell in Cell Selection, Idle    */
/* mode and dedicated mode SDCCH. It programs the DSP    */
/* and the TPU for making 1 attempt in reading the       */
/* synchro burst. Here is a summary of the execution:    */
/*                                                       */
/*  - If SEMAPHORE(task) is low.                         */
/*    - Get the cell information structure.              */
/*    - Traces and debug.                                */
/*    - Programs DSP for SB reading task.                */
/*    - Programs TPU for SB reading task.                */
/*  - Flag DSP and TPU programmation.                    */
/*                                                       */
/* Input parameters:                                     */
/* -----------------                                     */
/* "task"                                                */
/*   SB1, Synchro Burst reading task in Idle mode.       */
/*   SB2, Synchro Burst detection task in Idle mode.     */
/*   SBCONF, Synchro Burst confirmation task in Idle     */
/*   mode.                                               */
/*   SB51, Synchro Burst reading task in Dedicated mode. */
/*   SBCNF51, Synchro Burst confirmation task in         */
/*   Dedicated mode.                                     */
/*                                                       */
/* Input parameters from globals:                        */
/* ------------------------------                        */
/* "l1a_l1s_com.task_param"                              */
/*   task semaphore bit register. Used to skip this      */
/*   control if L1A has changed or is changing some of   */
/*   the task parameters.                                */
/*                                                       */
/* "l1a_l1s_com.Ncell_info.acquis"                       */
/*   cell information structure used for SB1, SB2 and    */
/*   SB51 tasks.                                         */
/*                                                       */
/* "l1a_l1s_com.Ncell_info.conf"                         */
/*   cell information structure used for SBCONF and      */
/*   SBCNF51 tasks.                                      */
/*                                                       */
/* "l1s.tpu_offset"                                      */
/*   value for SYNCHRO and OFFSET register in the TPU    */
/*   for current serving cell setting. It is used here   */
/*   by the synchro burst reading TPU driver since this  */
/*   driver changes the OFFSET register. At the end of   */
/*   the task it restores the serving cell offset value. */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/* "l1s.tpu_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/TPU com.   */
/*   This is used mainly to swap then the com. page at   */
/*   the end of a control frame.                         */
/*   -> set CTRL_RX bit in the register.                 */
/*                                                       */
/* "l1s.dsp_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/DSP com.   */
/*   This is used mainly to swap then the com. page at   */
/*   the end of a control frame.                         */
/*   -> set CTRL_RX bit in the register.                 */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_ctrl_sbgen(UWORD8 task, UWORD8 attempt)
{
  UWORD8   reload_serv_offset = TRUE;   // Default: offset serving reloaded.
  WORD8    agc;
  UWORD8   lna_off;
  BOOL     en_task;
  BOOL     task_param;
  UWORD32  dsp_task;
  UWORD8 input_level;
  #if (RF_FAM == 61)
      UWORD16 dco_algo_ctl_sb = 0;
      UWORD8 if_ctl = 0;
	  UWORD8 if_threshold = C_IF_ZERO_LOW_THRESHOLD_GSM;
  #endif
#if (L1_FF_MULTIBAND == 1)
  UWORD16 operative_radio_freq;
#endif


  // Get "enable" task flag and "synchro semaphore" for current task.
  en_task    = l1a_l1s_com.l1s_en_task[task];
  task_param = l1a_l1s_com.task_param[task];

  if((en_task) && !(task_param))
  // Check the task semaphore and enable flag. The control body is executed only
  // when the task semaphore is 0 and enable flag is 1. The semaphore can be set to
  // 1 whenever L1A makes some changes to the task parameters. The enable can be
  // reset to 0 when the task is no more enabled.
  {
    T_NCELL_SINGLE *cell_info_ptr = NULL;

    switch(task)
    {
      case SB2:
      {
        // Get the cell information structure.
        // ************************************
        cell_info_ptr = &l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_sb_id];

        if(attempt == 1) reload_serv_offset = FALSE; // Offset serving not reloaded on 1st CTRL.

        #if (TRACE_TYPE!=0)
          trace_fct(CST_L1S_CTRL_SB2, cell_info_ptr->radio_freq);
        #endif
      }
      break;

      case SB51:
      {
        // Get the cell information structure.
        // ************************************
        cell_info_ptr = &l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_sb_id];

        #if (TRACE_TYPE!=0)
          trace_fct(CST_L1S_CTRL_SB51, cell_info_ptr->radio_freq);
        #endif
      }
      break;

      case SBCONF:
      case SBCNF51:
      {
        // Get the cell information structure.
        // ************************************
        cell_info_ptr = &l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_sbconf_id];

        #if (TRACE_TYPE!=0)
          if(task == SBCONF)
            trace_fct(CST_L1S_CTRL_SBCONF, cell_info_ptr->radio_freq);
          else
            trace_fct(CST_L1S_CTRL_SBCNF51, cell_info_ptr->radio_freq);
        #endif
#if (L1_EOTD==1)
        // We need to trigger the TOA tracking / adjustment period
        // which logs all TOA updates after E-OTD has started...

        if(l1a_l1s_com.nsync.eotd_meas_session == TRUE)
        {
          if(    (l1a_l1s_com.nsync.eotd_toa_phase == 0)
              && (l1a_l1s_com.nsync.active_sbconf_id == 12) )
          {
            l1a_l1s_com.nsync.eotd_toa_tracking = 0;
            l1a_l1s_com.nsync.eotd_toa_phase = 1;
          }

          l1a_l1s_com.nsync.eotd_cache_toa_tracking = l1a_l1s_com.nsync.eotd_toa_tracking;
        }
#endif


      }
      break;

      default: return;
    }

    // Traces and debug.
    // ******************

    #if (TRACE_TYPE==5) && FLOWCHART
      trace_flowchart_dsp_tpu(dltsk_trace[task].name);
    #endif

    l1s_dsp_com.dsp_db_w_ptr->d_debug = (l1s.debug_time + 3) ;

    // Programs DSP for required task.
    // ********************************

    // dsp pgm...
    dsp_task = l1s_swap_iq_dl(cell_info_ptr->radio_freq,task);
    l1ddsp_load_monit_task(dsp_task, 0);

#if (L1_FF_MULTIBAND == 0)
    input_level = l1a_l1s_com.last_input_level[cell_info_ptr->radio_freq - l1_config.std.radio_freq_index_offset].input_level;
#else // L1_FF_MULTIBAND = 1 below
    operative_radio_freq = 
      l1_multiband_radio_freq_convert_into_operative_radio_freq(cell_info_ptr->radio_freq); 
    input_level = l1a_l1s_com.last_input_level[operative_radio_freq].input_level;
#endif //#if (L1_FF_MULTIBAND == 0) else

   #if (RF_FAM == 61)   // Locosto DCO
        cust_get_if_dco_ctl_algo(&dco_algo_ctl_sb, &if_ctl, (UWORD8) L1_IL_VALID ,
                                           input_level,
                                             cell_info_ptr->radio_freq,if_threshold);

     // This is a work-around for a DSP problem (OMAPS00117845)
     // The problem happens during neighbor FB/SB, when there is an
     // IDLE frame between neighbor FB and SB. 
     // Neighbor cell SB(SB2) is different from other kind of SB decode.
     // For SB2 we open the RF window for 2 frames (2 C W W R)
     // For both Control, l1s_dsp_com.dsp_db_common_w_ptr->d_dco_algo_ctrl_sb is updated.
     // However DSP copies DB value to NDB and this value is copied only once.
     // At the end of the first SB, DSP right shifts the NDB variable.
     // The fix below replicates the DCO control information 4 times
     // so that DSP has correct information even after right shifting during first SB.
   
        if(task == SB2)
	 {
	   dco_algo_ctl_sb *= 0x55;
	 }
    



	l1ddsp_load_dco_ctl_algo_sb(dco_algo_ctl_sb);
   #endif

    // Programs TPU for required task.
    // ********************************
#if (L1_FF_MULTIBAND == 0)
    
    // Get AGC to be applied.
    agc = Cust_get_agc_from_IL(cell_info_ptr->radio_freq, input_level >> 1, AV_ID);
    // lna_off flag is ONLY updated in case of l1ctl_pgc2 control algorithm
    lna_off = l1a_l1s_com.last_input_level[cell_info_ptr->radio_freq - l1_config.std.radio_freq_index_offset].lna_off;

#else // L1_FF_MULTIBAND = 0 below

    operative_radio_freq = 
      l1_multiband_radio_freq_convert_into_operative_radio_freq(cell_info_ptr->radio_freq); 
    // lna_off flag is ONLY updated in case of l1ctl_pgc2 control algorithm
    lna_off = l1a_l1s_com.last_input_level[operative_radio_freq].lna_off;
    // Get AGC to be applied.
    agc = 
      Cust_get_agc_from_IL(cell_info_ptr->radio_freq, input_level >> 1, AV_ID);

#endif // #if (L1_FF_MULTIBAND == 0) else

    // tpu pgm...
    l1dtpu_neig_sb(cell_info_ptr->radio_freq,
                   agc,
                   lna_off,
                   cell_info_ptr->time_alignmt,
                   l1s.tpu_offset,
                   reload_serv_offset,
                   attempt
                #if (RF_FAM == 61)
                   ,if_ctl
                #endif
	                         );

    #if L2_L3_SIMUL
      #if (DEBUG_TRACE == BUFFER_TRACE_OFFSET_NEIGH)
        buffer_trace(4, l1s.actual_time.fn, cell_info_ptr->radio_freq,
                        cell_info_ptr->time_alignmt,l1s.tpu_offset);
      #endif
    #endif
  }
  else
  // The task has been disabled or some parameters have changed, the serving tpu offset
  // must be restored.
  {
    if(attempt==2)
    {
      l1dmacro_offset(l1s.tpu_offset,IMM);
    }
  }

  // Flag DSP and TPU programmation.
  // ********************************

  // Set "CTRL_RX" flag in the controle flag register.
  l1s.tpu_ctrl_reg |= CTRL_RX;
  l1s.dsp_ctrl_reg |= CTRL_RX;

  // This task is not compatible with Neigh. Measurement. Store task length
  // in "forbid_meas" to indicate when the task will last.
  // Rem: Only SB51/SBCNF51 tasks start from this ctrl function.
  if((task==SB51)||(task==SBCNF51)) l1s.forbid_meas = TASK_ROM_MFTAB[task].size;
}

#if (MOVE_IN_INTERNAL_RAM == 0) // Must be followed by the pragma used to duplicate the funtion in internal RAM
//#pragma DUPLICATE_FOR_INTERNAL_RAM_START

/*-------------------------------------------------------*/
/* l1s_ctrl_fb26()                                       */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* tasks: FB26. This function is the control function    */
/* for making a frequency burst acquisition attempt on   */
/* a neighbor cell in dedicated mode TCH. It programs    */
/* the DSP and the TPU for making 1 attempt in reading   */
/* the frequency burst.Here is a summary of the          */
/* execution:                                            */
/*                                                       */
/*  - If SEMAPHORE(task) is low.                         */
/*    - Traces and debug.                                */
/*    - Programs DSP for FB acquisition task.            */
/*    - Programs TPU for FB acquisition task.            */
/*  - Flag DSP and TPU programmation.                    */
/*                                                       */
/* Input parameters from globals:                        */
/* ------------------------------                        */
/* "l1a_l1s_com.Ncell_info.acquis"                       */
/*   cell information structure used for FB26 task.      */
/*                                                       */
/* "l1s.tpu_offset"                                      */
/*   value for SYNCHRO and OFFSET register in the TPU    */
/*   for current serving cell setting. It is used here   */
/*   by the frequency burst reading TPU driver since     */
/*   this driver changes the OFFSET register. At the end */
/*   of the task it restores the serving cell offset     */
/*   value.                                              */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/* "l1s.tpu_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/TPU com.   */
/*   This is used mainly to swap then the com. page at   */
/*   the end of a control frame.                         */
/*   -> set CTRL_RX bit in the register.                 */
/*                                                       */
/* "l1s.dsp_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/DSP com.   */
/*   This is used mainly to swap then the com. page at   */
/*   the end of a control frame.                         */
/*   -> set CTRL_RX bit in the register.                 */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_ctrl_fb26(UWORD8 param1, UWORD8 param2)
{
  WORD8    agc;
  BOOL     lna_off;
  UWORD32  dsp_task;
  UWORD16  radio_freq = 0;
#if (L1_FF_MULTIBAND == 1)
  UWORD16 operative_radio_freq;
#endif
  
#if (L1_12NEIGH ==1)
  BOOL     en_task;
  BOOL     task_param;

  // Get "enable" task flag and "synchro semaphore" for current task.
  en_task    = l1a_l1s_com.l1s_en_task[param1];
  task_param = l1a_l1s_com.task_param[param1];

  if((en_task) && !(task_param))
  // Check the task semaphore and enable flag. The control body is executed only
  // when the task semaphore is 0 and enable flag is 1. The semaphore can be set to
  // 1 whenever L1A makes some changes to the task parameters. The enable can be
  // reset to 0 when the task is no more enabled.
#else
  if(!(l1a_l1s_com.task_param[FB26] == SEMAPHORE_SET))
#endif
  // Check the task semaphore. The control body is executed only
  // when the task semaphore is 0. This semaphore can be set to
  // 1 whenever L1A makes some changes to the task parameters.
  {
#if (L1_12NEIGH ==1)
    T_NCELL_SINGLE *cell_info_ptr = NULL;

    cell_info_ptr = &l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_fb_id];
    radio_freq = cell_info_ptr->radio_freq;
#else
    radio_freq = l1a_l1s_com.nsync.list[0].radio_freq;
#endif
    // Traces and debug.
    // ******************

    #if (TRACE_TYPE!=0)
      trace_fct(CST_L1S_CTRL_FB26, radio_freq);
    #endif

    #if (TRACE_TYPE==5) && FLOWCHART
      trace_flowchart_dsp_tpu(dltsk_trace[FB26].name);
    #endif

    // Programs DSP for FB26 task.
    // ****************************

    // dsp pgm...

    dsp_task = l1s_swap_iq_dl(radio_freq,FB26);

    l1ddsp_load_monit_task(dsp_task, 1);

    // Programs TPU for FB26 task.
    // ****************************
#if (L1_FF_MULTIBAND == 0)
    
    // agc is just computed from last stored IL
    agc     = Cust_get_agc_from_IL(radio_freq, l1a_l1s_com.last_input_level[radio_freq - l1_config.std.radio_freq_index_offset].input_level >> 1, AV_ID);
    lna_off = l1a_l1s_com.last_input_level[radio_freq - l1_config.std.radio_freq_index_offset].lna_off;

#else // L1_FF_MULTIBAND = 1 below

    operative_radio_freq = 
      l1_multiband_radio_freq_convert_into_operative_radio_freq(radio_freq);
    lna_off = l1a_l1s_com.last_input_level[operative_radio_freq].lna_off;
    // agc is just computed from last stored IL
    agc     = 
    Cust_get_agc_from_IL(radio_freq, l1a_l1s_com.last_input_level[operative_radio_freq].input_level >> 1, AV_ID);

#endif // #if (L1_FF_MULTIBAND == 1) else


    // tpu pgm...
    l1dtpu_neig_fb26(radio_freq,
                     agc,
                     lna_off,
                     l1s.tpu_offset);
  }

  // Flag DSP and TPU programmation.
  // ********************************

  // Set "CTRL_RX" flag in the controle flag register.
  l1s.tpu_ctrl_reg |= CTRL_RX;
  l1s.dsp_ctrl_reg |= CTRL_RX;

  // This task is not compatible with Neigh. Measurement. Store task length
  // in "forbid_meas" to indicate when the task will last.
  // Special case: we set forbid_meas to skip the measurements in the frames
  // FN%26=24 or 25.
  l1s.forbid_meas = 3;
}
//#pragma DUPLICATE_FOR_INTERNAL_RAM_END
#endif // MOVE_IN_INTERNAL_RAM

#if (MOVE_IN_INTERNAL_RAM == 0) // Must be followed by the pragma used to duplicate the funtion in internal RAM
//#pragma DUPLICATE_FOR_INTERNAL_RAM_START

/*-------------------------------------------------------*/
/* l1s_ctrl_sb26()                                       */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* tasks: SB1,SB2,SB51,SBCONF,SBCNF51. This function is  */
/* the control function for making a synchro burst       */
/* reading on a neighbor cell in Cell Selection, Idle    */
/* mode and dedicated mode SDCCH. It programs the DSP    */
/* and the TPU for making 1 attempt in reading the       */
/* synchro burst. Here is a summary of the execution:    */
/*                                                       */
/*  - If SEMAPHORE(task) is low.                         */
/*    - Get the cell information structure.              */
/*    - Traces and debug.                                */
/*    - Programs DSP for SB reading task.                */
/*    - Programs TPU for SB reading task.                */
/*  - Flag DSP and TPU programmation.                    */
/*                                                       */
/* Input parameters:                                     */
/* -----------------                                     */
/* "task"                                                */
/*   SB26, Synchro Burst reading task in Dedicated mode, */
/*   TCH.                                                */
/*   SBCNF26, Synchro Burst confirmation task in Dedic.  */
/*   mode TCH.                                           */
/*                                                       */
/* Input parameters from globals:                        */
/* ------------------------------                        */
/* "l1a_l1s_com.Ncell_info.acquis"                       */
/*   cell information structure used for SB26 task.      */
/*                                                       */
/* "l1a_l1s_com.Ncell_info.conf"                         */
/*   cell information structure used for SBCNF26 task.   */
/*                                                       */
/* "l1s.tpu_offset"                                      */
/*   value for SYNCHRO and OFFSET register in the TPU    */
/*   for current serving cell setting. It is used here   */
/*   by the synchro burst reading TPU driver since this  */
/*   driver changes the OFFSET register. At the end of   */
/*   the task it restores the serving cell offset value. */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/* "l1s.tpu_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/TPU com.   */
/*   This is used mainly to swap then the com. page at   */
/*   the end of a control frame.                         */
/*   -> set CTRL_RX bit in the register.                 */
/*                                                       */
/* "l1s.dsp_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/DSP com.   */
/*   This is used mainly to swap then the com. page at   */
/*   the end of a control frame.                         */
/*   -> set CTRL_RX bit in the register.                 */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_ctrl_sb26(UWORD8 task, UWORD8 param2)
{
  UWORD32  nb_nop = 0;
  WORD8    agc;
  BOOL     lna_off;
  UWORD32  dsp_task;
  UWORD8 input_level;
  UWORD32 temp;
#if (L1_FF_MULTIBAND == 1)
  UWORD16 operative_radio_freq;
#endif

#if (L1_12NEIGH ==1)
  BOOL     en_task;
  BOOL     task_param;
#if (RF_FAM == 61)
      UWORD16 dco_algo_ctl_sb = 0;
      UWORD8   if_ctl = 0 ;
	  UWORD8 if_threshold = C_IF_ZERO_LOW_THRESHOLD_GSM;
#endif

   // Get "enable" task flag and "synchro semaphore" for current task.
  en_task    = l1a_l1s_com.l1s_en_task[task];
  task_param = l1a_l1s_com.task_param[task];

 if((en_task) && !(task_param))
#else
  if(!(l1a_l1s_com.task_param[task]))
#endif
  // Check the task semaphore. The control body is executed only
  // when the task semaphore is 0. This semaphore can be set to
  // 1 whenever L1A makes some changes to the task parameters.
  {
    UWORD16  radio_freq = 0;
    UWORD32  time_alignmt = 0;
#if (L1_12NEIGH ==1)
    T_NCELL_SINGLE *cell_info_ptr;

    if (task == SB26)
       cell_info_ptr = &l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_sb_id];
    if (task == SBCNF26)
    {

#if (L1_EOTD==1)
       // We need to trigger the TOA tracking / adjustment period
       // which logs all TOA updates after E-OTD has started...

       if(l1a_l1s_com.nsync.eotd_meas_session == TRUE)
       {
         if(    (l1a_l1s_com.nsync.eotd_toa_phase == 0)
             && (l1a_l1s_com.nsync.active_sbconf_id == 12) )
         {
           l1a_l1s_com.nsync.eotd_toa_tracking = 0;
           l1a_l1s_com.nsync.eotd_toa_phase = 1;
         }

         l1a_l1s_com.nsync.eotd_cache_toa_tracking = l1a_l1s_com.nsync.eotd_toa_tracking;
       }
#endif

       cell_info_ptr = &l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_sbconf_id];

    }
    radio_freq   = cell_info_ptr->radio_freq;
    time_alignmt = cell_info_ptr->time_alignmt;

#else
    // Get the cell information.
    // **************************
    radio_freq   = l1a_l1s_com.nsync.list[0].radio_freq;
    time_alignmt = l1a_l1s_com.nsync.list[0].time_alignmt;
#endif
    // Traces and debug.
    // ******************

    #if (TRACE_TYPE!=0)
      switch(task)
      {
        case SB26:    trace_fct(CST_L1S_CTRL_SB26, radio_freq);    break;
        case SBCNF26: trace_fct(CST_L1S_CTRL_SBCNF26, radio_freq); break;
      }
    #endif

    #if (TRACE_TYPE==5) && FLOWCHART
      trace_flowchart_dsp_tpu(dltsk_trace[task].name);
    #endif

    // Programs DSP for required task.
    // ********************************
    // dsp pgm...

    dsp_task = l1s_swap_iq_dl(radio_freq,task);
    l1ddsp_load_monit_task(dsp_task, 0);

    // Programs TPU for required task.
    // ********************************
  temp = (UWORD32)(l1_config.params.fb26_anchoring_time - EPSILON_SYNC);
  #if (L1_12NEIGH ==1)
      if((cell_info_ptr->sb26_offset == 1) &&
         (time_alignmt >= temp)) //omaps00090550
  #else
      if((l1a_l1s_com.nsync.list[0].sb26_offset == 1) &&
         (time_alignmt >= temp)) //omaps00090550
  #endif
    // SB is in the 2nd frame of the search slot...
    // ...and SB is at the very end of the slot.
    // We insert a nop in the tpu scenario to
    // be able to jump the 1st frame.
    {
      nb_nop = 1;
    }

#if (L1_FF_MULTIBAND == 0)

    // agc is just computed from last stored IL
    input_level = l1a_l1s_com.last_input_level[radio_freq - l1_config.std.radio_freq_index_offset].input_level;
    agc     = Cust_get_agc_from_IL(radio_freq, input_level >> 1, AV_ID);
    lna_off = l1a_l1s_com.last_input_level[radio_freq - l1_config.std.radio_freq_index_offset].lna_off;

#else // L1_FF_MULTIBAND = 1 below

    operative_radio_freq = 
      l1_multiband_radio_freq_convert_into_operative_radio_freq(radio_freq); 
    // agc is just computed from last stored IL
    input_level = l1a_l1s_com.last_input_level[operative_radio_freq].input_level;
    lna_off = l1a_l1s_com.last_input_level[operative_radio_freq].lna_off;    
    agc     = Cust_get_agc_from_IL(radio_freq, input_level >> 1, AV_ID);    

#endif // #if (L1_FF_MULTIBAND == 0) else

    #if (RF_FAM == 61)   // Locosto DCO
       cust_get_if_dco_ctl_algo(&dco_algo_ctl_sb, &if_ctl, (UWORD8) L1_IL_VALID,
                                          input_level,
                                           radio_freq,if_threshold);
      	l1ddsp_load_dco_ctl_algo_sb(dco_algo_ctl_sb);
    #endif

    // tpu pgm...
    l1dtpu_neig_sb26(radio_freq,
                     agc,
                     lna_off,
                     time_alignmt,
                     nb_nop,
                     l1s.tpu_offset
                    #if (RF_FAM == 61)
                    ,if_ctl
                    #endif
                               	);
  }

  // Flag DSP and TPU programmation.
  // ********************************

  // Set "CTRL_RX" flag in the controle flag register.
  l1s.tpu_ctrl_reg |= CTRL_RX;
  l1s.dsp_ctrl_reg |= CTRL_RX;

  // This task is not compatible with Neigh. Measurement. Store task length
  // in "forbid_meas" to indicate when the task will last.
  // Special case: we set forbid_meas to skip the measurements in the frames
  // FN%26=24 or 25.
  l1s.forbid_meas = 3;
}
//#pragma DUPLICATE_FOR_INTERNAL_RAM_END
#endif // MOVE_IN_INTERNAL_RAM

/*-------------------------------------------------------*/
/* l1s_ctrl_smscb()                                      */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* tasks: SMSCB. This function is the control function   */
/* for reading a CBCH burst on the serving cell. It      */
/* shifts the OFFSET register to match the normal burst  */
/* receive task with the CBCH timeslot number (0,1,2 or  */
/* 3), programs a normal burst reading and restores the  */
/* OFFSET to the serving cell timeslot 0. On the last    */
/* control (4th burst), the SYNCHRO/OFFSET registers are */
/* shifted back to the normal idle mode PCH reading      */
/* setting. Here is a summary of the execution:          */
/*                                                       */
/*  - If SEMAPHORE(task) is low.                         */
/*    - Traces and debug.                                */
/*    - Programs DSP for SMSCB task, reading 1 burst.    */
/*    - Programs TPU for SMSCB task, reading 1 burst.    */
/*    - Shift TPU SYNCHRO/OFFSET registers back to the   */
/*      PAGING TASK timeslot.                            */
/*  - Flag DSP and TPU programmation.                    */
/*                                                       */
/* Input parameters:                                     */
/* -----------------                                     */
/* "task"                                                */
/*   SMSCB, Short Message Service Cell Broadcast.        */
/*                                                       */
/* Input parameters from globals:                        */
/* ------------------------------                        */
/* "l1a_l1s_com.cbch_desc"                               */
/*   Cell Broadcast CHannel description structure.       */
/*                                                       */
/* "l1a_l1s_com.Scell_info.bsic"                         */
/*   BSIC of the serving cell. It is used here to pass   */
/*   the training sequence number (part of BSIC) to the  */
/*   DSP.                                                */
/*                                                       */
/* "l1s.afc"                                             */
/*   current AFC value to be applied for SMSCB reading   */
/*   task.                                               */
/*                                                       */
/* "l1a_l1s_com.offset_tn0"                              */
/*   value to load in the OFFSET register to shift then  */
/*   any receive task to the timeslot 0 of the serving   */
/*   cell. This is the default setting to restore after  */
/*   any CBCH burst reading.                             */
/*                                                       */
/* "l1s.tpu_offset"                                      */
/*   value for the TPU SYNCHRO and OFFSET registers      */
/*   for current serving cell setting. It is used here   */
/*   at the end of the CBCH task controls to restore the */
/*   SYNCHRO/OFFSET registers to the normal setting in   */
/*   idle mode.                                          */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/* "l1s.actual_time, l1s.next_time"                      */
/*   frame number and derived numbers for current frame  */
/*   and next frame.                                     */
/*   -> update to cope with side effect due to synchro.  */
/*      changes/restores.                                */
/*                                                       */
/* "l1s.tpu_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/TPU com.   */
/*   This is used mainly to swap then the com. page at   */
/*   the end of a control frame.                         */
/*   -> set CTRL_RX bit in the register.                 */
/*                                                       */
/* "l1s.dsp_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/DSP com.   */
/*   This is used mainly to swap then the com. page at   */
/*   the end of a control frame.                         */
/*   -> set CTRL_RX bit in the register.                 */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_ctrl_smscb(UWORD8 task, UWORD8 burst_id)
{
  UWORD16  rx_radio_freq;
  UWORD32  offset_smscb;
  WORD8    agc;
  UWORD8   lna_off;
  UWORD32  dsp_task;
  static   WORD32 new_tpu_offset;
  static   BOOL   change_synchro;
#if 0	/* FreeCalypso TCS211 reconstruction */
  UWORD8 input_level;
#endif
#if (L1_FF_MULTIBAND == 1)
  UWORD16 operative_radio_freq;
#endif

#if (NEW_SNR_THRESHOLD == 1)
  UWORD8 saic_flag=0;
#endif /* NEW_SNR_THRESHOLD */
#if (RF_FAM == 61)
  UWORD16 dco_algo_ctl_nb = 0;
  UWORD8 if_ctl = 0;
  UWORD8 if_threshold = C_IF_ZERO_LOW_THRESHOLD_GSM;
  // By default we choose the hardware filter
  UWORD8 csf_filter_choice = L1_SAIC_HARDWARE_FILTER;
#endif

  // Needed for simulated DSP GRPS scheduler
  #if (CODE_VERSION == SIMULATION)
    UWORD32 tpu_w_page;

    if (hw.tpu_r_page==0)
     tpu_w_page=1;
    else
     tpu_w_page=0;

    hw.rx_id[tpu_w_page][0]=0;
    hw.num_rx[tpu_w_page][0]=1;
    hw.rx_group_id[tpu_w_page]=1;
  #endif


  if((l1a_l1s_com.l1s_en_task[task] == TASK_ENABLED) &&
    !(l1a_l1s_com.task_param[task] == SEMAPHORE_SET))
  // Check the task semaphore. The control body is executed only
  // when the task semaphore is 0. This semaphore can be set to
  // 1 whenever L1A makes some changes to the task parameters.
  {
    // Get ARFCN to be used for current control. Output of the hopping algorithm.
    rx_radio_freq = l1a_l1s_com.dedic_set.radio_freq;

    // Traces and debug.
    // ******************

    #if (TRACE_TYPE==5) && FLOWCHART
      trace_flowchart_dsp_tpu(dltsk_trace[task].name);
    #endif

    #if (TRACE_TYPE!=0)
      trace_fct(CST_L1S_CTRL_SMSCB, rx_radio_freq);
    #endif

    l1s_dsp_com.dsp_db_w_ptr->d_debug = (l1s.debug_time + 2) ;

    // Programs DSP for SMSCB task according to the DSP scheduler used
    // ****************************************************************

  #if L1_GPRS
    switch(l1a_l1s_com.dsp_scheduler_mode)
    {
      // dsp pgm is made using GSM scheduler...
      case GSM_SCHEDULER:
      {
        dsp_task = l1s_swap_iq_dl(rx_radio_freq, task);

        // dsp pgm...
        l1ddsp_load_rx_task(dsp_task,burst_id,l1a_l1s_com.cbch_desc.tsc);
      }
      break;

      // dsp pgm is made using GPRS scheduler...
      case GPRS_SCHEDULER:
      {
      #if FF_L1_IT_DSP_USF
        l1pddsp_idle_rx_nb(burst_id,l1a_l1s_com.cbch_desc.tsc,rx_radio_freq,0,FALSE,FALSE);
      #else
        l1pddsp_idle_rx_nb(burst_id,l1a_l1s_com.cbch_desc.tsc,rx_radio_freq,0,FALSE);
      #endif
      }
      break;
    }
  #else
    dsp_task = l1s_swap_iq_dl(rx_radio_freq, task);

    // dsp pgm...
    l1ddsp_load_rx_task(dsp_task,burst_id,l1a_l1s_com.cbch_desc.tsc);
 #endif

    // Check if "Synchro" change is needed.
    // *************************************

    // If so the synchro is changed by 4 timeslots.
    if(burst_id == BURST_1)
    {
      // This task is not compatible with Neigh. Measurement. Store task length
      // in "forbid_meas" to indicate when the task will last.
      l1s.forbid_meas = TASK_ROM_MFTAB[task].size;

      change_synchro = l1a_l1s_com.change_synchro_cbch;

      if(change_synchro)
      {
        // compute TPU offset for "current timeslot + 4 timeslot"
        new_tpu_offset = l1s.tpu_offset + (4 * TN_WIDTH);

        if(new_tpu_offset >= TPU_CLOCK_RANGE)
          new_tpu_offset -= TPU_CLOCK_RANGE;

        // Slide synchro to match current timeslot + 4 timeslot.
        l1dmacro_synchro(l1_config.params.rx_change_synchro_time, new_tpu_offset);
      }
      else
      {
        new_tpu_offset = l1s.tpu_offset;
      }
    }

    // Programs TPU for SMSCB task, reading 1 burst.
    // **********************************************

    offset_smscb = new_tpu_offset + l1a_l1s_com.tn_smscb * TN_WIDTH;
    if (offset_smscb >= TPU_CLOCK_RANGE)
      offset_smscb -= TPU_CLOCK_RANGE;

#if 1	/* FreeCalypso match TCS211 */

    // agc is set with the input_level computed from PAGC algo
    agc     = Cust_get_agc_from_IL(l1a_l1s_com.Scell_info.radio_freq, l1a_l1s_com.last_input_level[l1a_l1s_com.Scell_info.radio_freq - l1_config.std.radio_freq_index_offset].input_level >> 1, MAX_ID);
    lna_off = l1a_l1s_com.last_input_level[l1a_l1s_com.Scell_info.radio_freq - l1_config.std.radio_freq_index_offset].lna_off;

#elif (L1_FF_MULTIBAND == 0)

    // agc is set with the input_level computed from PAGC algo
    input_level =  l1a_l1s_com.last_input_level[l1a_l1s_com.Scell_info.radio_freq - l1_config.std.radio_freq_index_offset].input_level;
    agc     = Cust_get_agc_from_IL(l1a_l1s_com.Scell_info.radio_freq,input_level >> 1, MAX_ID);
    lna_off = l1a_l1s_com.last_input_level[l1a_l1s_com.Scell_info.radio_freq - l1_config.std.radio_freq_index_offset].lna_off;

#else // L1_FF_MULTIBAND = 1 below

    operative_radio_freq = 
      l1_multiband_radio_freq_convert_into_operative_radio_freq(l1a_l1s_com.Scell_info.radio_freq); 
    input_level =  l1a_l1s_com.last_input_level[operative_radio_freq].input_level;
    lna_off = l1a_l1s_com.last_input_level[operative_radio_freq].lna_off;
    agc     = Cust_get_agc_from_IL(l1a_l1s_com.Scell_info.radio_freq,input_level >> 1, MAX_ID);


#endif // #if (L1_FF_MULTIBAND == 0) else

    #if(RF_FAM == 61)   // Locosto DCO
       cust_get_if_dco_ctl_algo(&dco_algo_ctl_nb, &if_ctl, (UWORD8) L1_IL_VALID,
                                                 input_level,
                                                 l1a_l1s_com.Scell_info.radio_freq,if_threshold);

        l1ddsp_load_dco_ctl_algo_nb(dco_algo_ctl_nb);
    #endif

    // Store IL used for current CTRL in order to be able to buil IL from pm
    // in READ phase.
#if 1	/* FreeCalypso match TCS211 */

    l1a_l1s_com.Scell_used_IL.input_level = l1a_l1s_com.last_input_level[l1a_l1s_com.Scell_info.radio_freq - l1_config.std.radio_freq_index_offset].input_level;
    l1a_l1s_com.Scell_used_IL.lna_off     = l1a_l1s_com.last_input_level[l1a_l1s_com.Scell_info.radio_freq - l1_config.std.radio_freq_index_offset].lna_off;

#elif (L1_FF_MULTIBAND == 0)

    l1a_l1s_com.Scell_used_IL.input_level = input_level;
    l1a_l1s_com.Scell_used_IL.lna_off     = l1a_l1s_com.last_input_level[l1a_l1s_com.Scell_info.radio_freq - l1_config.std.radio_freq_index_offset].lna_off;

#else // L1_FF_MULTIBAND = 1 below    
    
    operative_radio_freq = 
        l1_multiband_radio_freq_convert_into_operative_radio_freq(l1a_l1s_com.Scell_info.radio_freq);
    l1a_l1s_com.Scell_used_IL.input_level = input_level;
    l1a_l1s_com.Scell_used_IL.lna_off     = l1a_l1s_com.last_input_level[operative_radio_freq].lna_off;

#endif // #if (L1_FF_MULTIBAND == 1) else    

    #if (L1_SAIC != 0)
      // If SAIC is enabled, call the low level SAIC control function
      csf_filter_choice = l1ctl_saic(l1a_l1s_com.Scell_used_IL.input_level,l1a_l1s_com.mode
    #if (NEW_SNR_THRESHOLD == 1)
          ,task
          ,&saic_flag
    #endif
          );
    #endif

    // tpu pgm...
    l1dtpu_serv_rx_nb(rx_radio_freq,
                      agc,
                      lna_off,
                      new_tpu_offset,
                      offset_smscb,
                      TRUE,
                      FALSE
                    #if (RF_FAM == 61)
                      ,csf_filter_choice
                      ,if_ctl
                    #endif
                    #if (NEW_SNR_THRESHOLD == 1)
                      ,saic_flag
                    #endif   /* NEW_SNR_THRESHOLD */
      	                            );

  } // End if(task enabled and semaphore false)

  // Remark:
  //--------
  // When the task is aborted, we must continue to make dummy
  // DSP programming to avoid communication mismatch due
  // to C/W/R pipelining.

  // We must also ensure the Synchro back since synchro change has surely be done
  // in the 1st CTRL phase.

  // Shift TPU SYNCHRO/OFFSET registers back to the default timeslot (normally (P)CCCH one).
  // ****************************************************************************************
  // When the CBCH reading control is completed (4 burst controled),
  // the SYNCHRO/OFFSET registers are shifted back to the normal idle
  // setting used for (P)CCCH reading on the serving cell.

  // Check if "Synchro" change was needed.
  // If so the synchro is changed to recover normal synchro.
  if(burst_id == BURST_4)
  {
    if(change_synchro)
    {
      // Slide synchro back to mach current serving timeslot.
      l1dmacro_synchro(SWITCH_TIME, l1s.tpu_offset);

      // Increment frame number.
      #if L1_GPRS
        l1s.actual_time    = l1s.next_time;
        l1s.next_time      = l1s.next_plus_time;
        l1s_increment_time(&(l1s.next_plus_time), 1);  // Increment "next_plus time".
      #else
        l1s.actual_time = l1s.next_time;
        l1s_increment_time(&(l1s.next_time), 1);  // Increment "next time".
      #endif

      #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
        trace_fct(CST_L1S_ADJUST_TIME, (UWORD32)(-1));//OMAPS00090550
      #endif

      l1s.tpu_ctrl_reg |= CTRL_SYCB;
      l1s.dsp_ctrl_reg |= CTRL_SYNC;
    }
  }

  // Flag DSP and TPU programmation.
  // ********************************

  // Set "CTRL_RX" flag in the controle flag register.
  l1s.tpu_ctrl_reg |= CTRL_RX;
  l1s.dsp_ctrl_reg |= CTRL_RX;
}

#if !((MOVE_IN_INTERNAL_RAM == 1) && (GSM_IDLE_RAM !=0))  // MOVE TO INTERNAL MEM IN CASE GSM_IDLE_RAM enabled
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_START         // KEEP IN EXTERNAL MEM otherwise

#if 0	/* FreeCalypso TCS211 reconstruction */
UWORD32 qual_acc_idle1[2];
#endif

/*-------------------------------------------------------*/
/* l1s_ctrl_snb_dl()                                     */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* serving cell normal burst reading tasks: NP, EP,      */
/* BCCHS, ALLC, DDL and ADL. This function is the control*/
/* function for reading a normal burst on the serving    */
/* cell. It programs the DSP and the TPU for reading a   */
/* normal burst without change on the TPU OFFSET         */
/* register and flags the reading of the normal paging   */
/* burst. This flag is used by the measurement manager   */
/* "l1s_meas_manager()" at the end of L1S. Here is a     */
/* summary of the execution:                             */
/*                                                       */
/*  - If SEMAPHORE(task) is low.                         */
/*      - Catch ARFCN and set CIPHERING reduced frame    */
/*        number.                                        */
/*      - Traces and debug.                              */
/*      - Programs DSP for required task.                */
/*      - Programs TPU for required task.                */
/*      - Flag the reading of a Normal Paging burst.     */
/*  - Flag DSP and TPU programmation.                    */
/*                                                       */
/* Input parameters:                                     */
/* -----------------                                     */
/* "task"                                                */
/*   NP, Normal paging reading task.                     */
/*   EP, Extended paging reading task.                   */
/*   BCCHS, BCCH Serving reading task.                   */
/*   ALLC, All serving cell CCCH reading task.           */
/*                                                       */
/*   DDL, SDCCH DOWNLINK reading task.                   */
/*   ADL, SACCH DOWNLINK (associated with SDCCH)reading  */
/*   task.                                               */
/*                                                       */
/* "burst_id"                                            */
/*   BURST_1, 1st burst of the task.                     */
/*   BURST_2, 2nd burst of the task.                     */
/*   BURST_3, 3rd burst of the task.                     */
/*   BURST_4, 4th burst of the task.                     */
/*                                                       */
/* Input parameters from globals:                        */
/* ------------------------------                        */
/* "l1a_l1s_com.dedic_set"                               */
/*   Dedicated channel parameter structure. It is used   */
/*   to get the ARFCN to use for SDCCH (DDL, ADL). This  */
/*   ARFCN comes from the HOPPING algorithm called just  */
/*   before calling this function.                       */
/*                                                       */
/* "l1a_l1s_com.Scell_info"                              */
/*  Serving cell information structure.                  */
/*    .radio_freq, serving cell beacon frequency.             */
/*    .bsic, BSIC of the serving cell. It is used here   */
/*           to pass the training sequence number (part  */
/*           of BSIC) to the DSP.                        */
/*                                                       */
/* "l1s.afc"                                             */
/*   current AFC value to be applied for the given task. */
/*                                                       */
/* "l1s.tpu_offset"                                      */
/*   value for the TPU SYNCHRO and OFFSET registers      */
/*   for current serving cell setting. It is used here   */
/*   to refresh the TPU SYNCHRO and OFFSET registers     */
/*   with a corrected (time tracking of the serving)     */
/*   value prior to reading a serving cell normal burst. */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/* "l1s.np_ctrl"                                         */
/*   Flag set when a normal paging burst reading is      */
/*   controled. This flag is used by the measurement     */
/*   manager "l1s_meas_manager()", at the end of L1S, to */
/*   scheduling the neighbor cell measurements.          */
/*   -> set to 1.                                        */
/*                                                       */
/* "l1s.tpu_win"                                         */
/*   each frame is composed with a maximum of 3          */
/*   working/TPU windows (typically RX/TX/PW). This is   */
/*   a counter used to count the number of windows       */
/*   used.                                               */
/*   -> incremented.                                     */
/*                                                       */
/* "l1s.tpu_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/TPU com.   */
/*   This is used mainly to swap then the com. page at   */
/*   the end of a control frame.                         */
/*   -> set CTRL_RX bit in the register.                 */
/*                                                       */
/* "l1s.dsp_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/DSP com.   */
/*   This is used mainly to swap then the com. page at   */
/*   the end of a control frame.                         */
/*   -> set CTRL_RX bit in the register.                 */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_ctrl_snb_dl(UWORD8 task, UWORD8 burst_id)
{
  UWORD8          lna_off;
  WORD8           agc;
  UWORD16         rx_radio_freq;
  UWORD8          tsc;
  T_INPUT_LEVEL  *IL_info_ptr;
  UWORD32         dsp_task;
  static  BOOL    change_synchro;
  UWORD8          adc_active = INACTIVE;
#if (L1_FF_MULTIBAND == 1)
  UWORD16 operative_radio_freq;
#endif /*L1_FF_MULTIBAND*/
  
#if L1_GPRS
  static  BOOL    algo_change_synchro_active = FALSE;
  static  BOOL    BCCHS_in_transfert = FALSE;
#endif
#if 0	/* FreeCalypso match TCS211 */
  UWORD8 input_level = 0; //omaps00090550
#endif
#if (RF_FAM == 61)
  UWORD16 dco_algo_ctl_nb = 0;
  UWORD8 if_ctl = 0;
  UWORD8 if_threshold = C_IF_ZERO_LOW_THRESHOLD_GSM;
  // By default we choose the hardware filter
  UWORD8 csf_filter_choice = L1_SAIC_HARDWARE_FILTER;
#endif
#if (NEW_SNR_THRESHOLD == 1)
  UWORD8 saic_flag=0;
#endif /* NEW_SNR_THRESHOLD */

#if (FF_L1_FAST_DECODING == 1)
  BOOL fast_decoding_authorized = FALSE;

  if ( (burst_id == BURST_1) && (l1a_apihisr_com.fast_decoding.status == C_FAST_DECODING_FORBIDDEN) )
  {
    l1a_apihisr_com.fast_decoding.status = C_FAST_DECODING_NONE;
  }

  fast_decoding_authorized = l1s_check_fast_decoding_authorized(task);

  if ( fast_decoding_authorized && l1s_check_deferred_control(task,burst_id) )
  {
    /* Control is deferred until the upcoming fast decoding IT */
    return;
  } /* if (fast_decoding_authorized)*/

  /* In all other cases, control must be performed now. */
#endif /* FF_L1_FAST_DECODING == 1 */

  if(!(l1a_l1s_com.task_param[task] == SEMAPHORE_SET))
    // Check the task semaphore. The control body is executed only
    // when the task semaphore is 0. This semaphore can be set to
    // 1 whenever L1A makes some changes to the task parameters.
  {
    // Catch ARFCN and set CIPHERING reduced frame number.
    // Catch Training sequence.
    // ****************************************************

    if((task == DDL) || (task == ADL))
      // Dedicated mode SDCCH downlink.
    {
      // Get ARFCN to be used for current control.
      rx_radio_freq = l1a_l1s_com.dedic_set.radio_freq;

      if (rx_radio_freq==l1a_l1s_com.Scell_info.radio_freq) // we are working on a beacon freq.
        IL_info_ptr = &l1a_l1s_com.Scell_info.traffic_meas_beacon;
      else
        IL_info_ptr = &l1a_l1s_com.Scell_info.traffic_meas;// we are working on a daughter freq

      // Catch training sequence code from the channel description.
      tsc = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->tsc;

      // Set CIPHERING reduced frame number.
#if (AMR == 1)
  #if (FF_L1_TCH_VOCODER_CONTROL == 1)
      l1ddsp_load_tch_param(&(l1s.next_time),
        SIG_ONLY_MODE,
        l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type,
        #if !FF_L1_IT_DSP_DTX
          0, 0, 0, 0, 0, 0);
        #else
          0, 0, 0, 0, 0, 0, 0);
        #endif
  #else
      l1ddsp_load_tch_param(&(l1s.next_time),
        SIG_ONLY_MODE,
        l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type,
        #if !FF_L1_IT_DSP_DTX
          0, 0, 0, 0);
        #else
          0, 0, 0, 0, 0);
        #endif
  #endif
#else
  #if (FF_L1_TCH_VOCODER_CONTROL == 1)
      l1ddsp_load_tch_param(&(l1s.next_time),
        SIG_ONLY_MODE,
        l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type,
        #if !FF_L1_IT_DSP_DTX
          0, 0, 0, 0, 0);
        #else
          0, 0, 0, 0, 0, 0);
        #endif
  #else
      l1ddsp_load_tch_param(&(l1s.next_time),
        SIG_ONLY_MODE,
        l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type,
        #if !FF_L1_IT_DSP_DTX
          0, 0, 0);
        #else
          0, 0, 0, 0);
        #endif
  #endif
#endif
      // for SDCCH we use DPAGC algorithm.
#if DPAGC_MAX_FLAG
      agc = Cust_get_agc_from_IL(rx_radio_freq, IL_info_ptr->input_level >> 1, MAX_ID);
#else
      agc = Cust_get_agc_from_IL(rx_radio_freq, IL_info_ptr->input_level >> 1, AV_ID);
#endif
      lna_off = IL_info_ptr->lna_off;



      // Store input_level and lna_off field  used for current CTRL in order to be able to build IL
      // from pm in READ phase.
      l1a_l1s_com.Scell_used_IL = *IL_info_ptr;
    } // end if (task == DDL) || (task == ADL)
    else
    {
      rx_radio_freq = l1a_l1s_com.Scell_info.radio_freq;

      // Catch training sequence code from serving cell BCC (part of BSIC).
      tsc = l1a_l1s_com.Scell_info.bsic & 0x0007;

      // for PCH/E_PCH/Serving BCCH and All CCCH we use
      // PAGC algorithm.
#if 1	/* FreeCalypso match TCS211 */

      agc     = Cust_get_agc_from_IL(rx_radio_freq, l1a_l1s_com.last_input_level[rx_radio_freq - l1_config.std.radio_freq_index_offset].input_level >> 1, MAX_ID);
      lna_off = l1a_l1s_com.last_input_level[rx_radio_freq - l1_config.std.radio_freq_index_offset].lna_off;

      // Store input_level and lna_off fields used for current CTRL in order to be able
      // to build IL from pm in READ phase.
      l1a_l1s_com.Scell_used_IL = l1a_l1s_com.last_input_level[rx_radio_freq - l1_config.std.radio_freq_index_offset];

#elif (L1_FF_MULTIBAND == 0)

      input_level = l1a_l1s_com.last_input_level[rx_radio_freq - l1_config.std.radio_freq_index_offset].input_level ;
      lna_off = l1a_l1s_com.last_input_level[rx_radio_freq - l1_config.std.radio_freq_index_offset].lna_off;
      agc     = Cust_get_agc_from_IL(rx_radio_freq, input_level >> 1, MAX_ID);


      // Store input_level and lna_off fields used for current CTRL in order to be able
      // to build IL from pm in READ phase.
      l1a_l1s_com.Scell_used_IL = l1a_l1s_com.last_input_level[rx_radio_freq - l1_config.std.radio_freq_index_offset];

#else // L1_FF_MULTIBAND = 1 below

    operative_radio_freq = 
      l1_multiband_radio_freq_convert_into_operative_radio_freq(rx_radio_freq);

      input_level = l1a_l1s_com.last_input_level[operative_radio_freq].input_level ;
      lna_off = l1a_l1s_com.last_input_level[operative_radio_freq].lna_off;
      agc     = Cust_get_agc_from_IL(rx_radio_freq, input_level >> 1, MAX_ID);


      // Store input_level and lna_off fields used for current CTRL in order to be able
      // to build IL from pm in READ phase.
      l1a_l1s_com.Scell_used_IL = l1a_l1s_com.last_input_level[operative_radio_freq];


#endif // #if (L1_FF_MULTIBAND == 0) else

      
    }

   #if(RF_FAM == 61)   // Locosto DCO
          cust_get_if_dco_ctl_algo(&dco_algo_ctl_nb, &if_ctl, (UWORD8) L1_IL_VALID ,
                                                    input_level,
                                                    rx_radio_freq,if_threshold);
          l1ddsp_load_dco_ctl_algo_nb(dco_algo_ctl_nb);
   #endif //RF_FAM =61

    #if (L1_SAIC != 0)
      // If SAIC is enabled, call the low level SAIC control function
      csf_filter_choice = l1ctl_saic(l1a_l1s_com.Scell_used_IL.input_level,l1a_l1s_com.mode
      #if (NEW_SNR_THRESHOLD == 1)
          ,task
          ,&saic_flag
      #endif
          );
   #endif  //L1_SAIC != 0

    // ADC measurement
    // ***************
    // check if during the 1st burst of the bloc an ADC measurement must be performed
    if ((burst_id == BURST_1) && (task == NP))
    {
#if L1_GPRS
      //In case of network mode of operation II or III, CCCH reading is possible
      //in packet idle mode and in packet transfer mode.
      //ADC measurements are already managed by comlex function of Packet idle tasks
      if (!((l1a_l1s_com.l1s_en_task[PNP]    == TASK_ENABLED) ||
        (l1a_l1s_com.l1s_en_task[PEP]    == TASK_ENABLED) ||
        (l1a_l1s_com.l1s_en_task[PALLC]  == TASK_ENABLED) ||
        (l1a_l1s_com.l1s_en_task[PDTCH]  == TASK_ENABLED) ||
        (l1a_l1s_com.l1s_en_task[SINGLE] == TASK_ENABLED)))
#endif
      {
        adc_active = l1s_ADC_decision_on_NP();
      }
    } // end if (burst_id == BURST_1) && (task == NP)

    if (task == ADL)
    {
      // ADC measurement for SACCH DL
      // ****************************

      // check if during the SACCH burst an ADC measurement must be performed
      if (l1a_l1s_com.adc_mode & ADC_NEXT_TRAFFIC_DL)  // perform ADC only one time
      {
        adc_active = ACTIVE;
        l1a_l1s_com.adc_mode &= ADC_MASK_RESET_TRAFFIC; // reset in order to have only one ADC measurement in Traffic
      }
      else
      if (l1a_l1s_com.adc_mode & ADC_EACH_TRAFFIC_DL) // perform ADC on each period bloc
      {
        if ((++l1a_l1s_com.adc_cpt)>=l1a_l1s_com.adc_traffic_period) // wait for the period
        {
          adc_active = ACTIVE;
          l1a_l1s_com.adc_cpt = 0;
        }
      }
    }

    // Traces and debug.
    // ******************

#if (TRACE_TYPE==5) && FLOWCHART
    trace_flowchart_dsp_tpu(dltsk_trace[task].name);
#endif

#if (TRACE_TYPE!=0) && (TRACE_TYPE!=5)
    trace_fct(CST_L1S_CTRL_SNB_DL_BURST0 + burst_id, (UWORD32)(-1));//OMAPS00090550
#endif

#if (TRACE_TYPE==5)
    trace_fct(CST_L1S_CTRL_SNB_DL, rx_radio_freq);
#endif

    l1s_dsp_com.dsp_db_w_ptr->d_debug = (l1s.debug_time + 2) ;

    // the l1a_l1s_com.mode variable could change during the block: So test this variable only on the 1st block
    // See BUG2237 (mode change from Idle to Transfert during BCCHS task)
#if (L1_GPRS)
    if (burst_id == BURST_1)
      BCCHS_in_transfert = ((l1a_l1s_com.mode == PACKET_TRANSFER_MODE) && ((task == EBCCHS) || (task == NBCCHS)));
#endif

    // Programs DSP according to the DSP scheduler used
    // *************************************************


#if (L1_GPRS)
    switch(l1a_l1s_com.dsp_scheduler_mode)
    {
      // dsp pgm is made using GSM scheduler...
    case GSM_SCHEDULER:
#if (FF_L1_FAST_DECODING == 1)
      l1ddsp_load_fast_dec_task(task,burst_id);
#endif
      dsp_task = l1s_swap_iq_dl(rx_radio_freq,task);
      l1ddsp_load_rx_task(dsp_task, burst_id, tsc);
      break;

      // dsp pgm is made using GPRS scheduler...
    case GPRS_SCHEDULER:
#if (FF_L1_FAST_DECODING == 1)
          l1ddsp_load_fast_dec_task(task,burst_id);
#endif
        #if FF_L1_IT_DSP_USF
          l1pddsp_idle_rx_nb(burst_id,tsc,rx_radio_freq,0,FALSE,FALSE);
        #else
          l1pddsp_idle_rx_nb(burst_id,tsc,rx_radio_freq,0,FALSE);
        #endif
      break;
    }
#else
#if (FF_L1_FAST_DECODING == 1)
    l1ddsp_load_fast_dec_task(task,burst_id);
#endif
    dsp_task = l1s_swap_iq_dl(rx_radio_freq,task);
    l1ddsp_load_rx_task(dsp_task, burst_id, tsc);
#endif

    // update the TPU with the new TOA if necessary
    l1ctl_update_TPU_with_toa();

    // Programs TPU for required task.
    // ********************************
#if (L1_GPRS)

    //In case of network mode of operation II or III, CCCH reading is possible
    //in packet idle mode and in packet transfer mode.
    // if (TS(CCCH) - TS(current task))%8 >= 4    synchro change is required
    // if not, OFFSET change is required
    //
    if (((task == EP) || (task == NP)) &&
      ((l1a_l1s_com.l1s_en_task[PNP]    == TASK_ENABLED) ||
      (l1a_l1s_com.l1s_en_task[PEP]    == TASK_ENABLED) ||
      (l1a_l1s_com.l1s_en_task[PALLC]  == TASK_ENABLED) ||
      (l1a_l1s_com.l1s_en_task[PDTCH]  == TASK_ENABLED) ||
      (l1a_l1s_com.l1s_en_task[SINGLE] == TASK_ENABLED)))
    {
      UWORD32  new_offset;
      WORD32   new_synchro;
      UWORD32  ts_ccch;

      ts_ccch =  (l1a_l1s_com.ccch_group * 2);                //timeslot CCCH burts
      new_offset = (ts_ccch  - l1a_l1s_com.dl_tn + 8) % 8;    //dl_tn is the current time slot from previous task

      if (burst_id == BURST_1)
        l1s.forbid_meas = TASK_ROM_MFTAB[task].size;

      if (new_offset >= 4)
        algo_change_synchro_active = TRUE;

      if (algo_change_synchro_active)
      {
        // compute TPU offset for "current timeslot + 4 timeslot"
        new_synchro = l1s.tpu_offset + (4 * TN_WIDTH);

        if(new_synchro >= TPU_CLOCK_RANGE)
          new_synchro -= TPU_CLOCK_RANGE;

        //compute new offset
        new_offset = (((ts_ccch + 4 - l1a_l1s_com.dl_tn)%8) * TN_WIDTH) + new_synchro;
      }
      //no synchro change required, but new offset is computed
      else
      {
        new_synchro = l1s.tpu_offset;
        new_offset = (new_offset * TN_WIDTH) + new_synchro;
      }

      if (new_offset >= TPU_CLOCK_RANGE)
        new_offset -= TPU_CLOCK_RANGE;

      // tpu pgm...
      l1dtpu_serv_rx_nb(rx_radio_freq,
        agc,
        lna_off,
        new_synchro,
        new_offset,
        TRUE,
        adc_active
        #if (RF_FAM == 61)
	   ,csf_filter_choice
	   ,if_ctl
	#endif
    #if (NEW_SNR_THRESHOLD == 1)
     ,saic_flag
    #endif /*NEW_SNR_THRESHOLD */
        );
    } // end if (task == EP) || (task == NP) in packet Idle

    // in case of EBCCHS and NBCCHS in packet transfer a change synchro is performed
    else if (BCCHS_in_transfert)
    {
      UWORD32  new_offset;
      WORD32   new_synchro;

      change_synchro = ((l1a_l1s_com.dl_tn > 0) && (l1a_l1s_com.dl_tn < 5 ));

      // if change synchro is needed
      if(change_synchro) // TS= [1,2,3,4]
      {
        // the synchro is changed by 4 timeslots.
        new_synchro = l1s.tpu_offset + (4 * TN_WIDTH);
        if(new_synchro >= TPU_CLOCK_RANGE)
          new_synchro -= TPU_CLOCK_RANGE;

        // the TPU offset is changed according to the PDTCH time slot
        // because of the new synchro above with a shift of 4TS,
        // 4TS are substract to the offset
        new_offset  = (8 - 4 - l1a_l1s_com.dl_tn) * TN_WIDTH;
      }
      else
      {
        // the synchro is unchanged
        new_synchro = l1s.tpu_offset;

        // the TPU offset is changed according to the PDTCH time slot
        new_offset  = (8 - l1a_l1s_com.dl_tn) * TN_WIDTH;
      }

      new_offset += new_synchro;
      if (new_offset >= TPU_CLOCK_RANGE)
        new_offset -= TPU_CLOCK_RANGE;

      // tpu pgm...
      #if (RF_FAM == 61)
        l1dtpu_serv_rx_nb(rx_radio_freq,
          agc,
          lna_off,
          new_synchro,
          new_offset,
          TRUE,
          adc_active,
          csf_filter_choice,
          if_ctl
          #if (NEW_SNR_THRESHOLD == 1)
          ,saic_flag
          #endif /*NEW_SNR_THRESHOLD */
          );
	#endif

	#if(RF_FAM != 61)
         l1dtpu_serv_rx_nb(rx_radio_freq,
           agc,
           lna_off,
           new_synchro,
           new_offset,
           TRUE,
           adc_active);
	#endif

    } // end if (task == EBCCHS) || (task == NBCCHS) in packet Idle
    else
#endif
    {
      // tpu pgm...
      #if (RF_FAM == 61)
        l1dtpu_serv_rx_nb(rx_radio_freq,
          agc,
          lna_off,
          l1s.tpu_offset,
          l1s.tpu_offset,
          FALSE,
          adc_active,
          csf_filter_choice,
          if_ctl
          #if (NEW_SNR_THRESHOLD == 1)
          ,saic_flag
          #endif /*NEW_SNR_THRESHOLD */
          );
	#endif
       #if (RF_FAM != 61)
         l1dtpu_serv_rx_nb(rx_radio_freq,
           agc,
           lna_off,
           l1s.tpu_offset,
           l1s.tpu_offset,
           FALSE,
           adc_active);
	#endif
    }

    // Increment tpu window identifier.
    l1s.tpu_win += (l1_config.params.rx_synth_load_split + RX_LOAD);

    // GSM DSP scheduler is not able to handle PWR too close to RX normal burst.
    // We have to oblige a min of 1 burst period between RX and PWR
    if(l1_config.params.rx_synth_load_split < BP_SPLIT)
      l1s.tpu_win += BP_SPLIT - l1_config.params.rx_synth_load_split;

   #if L2_L3_SIMUL
   #if (DEBUG_TRACE == BUFFER_TRACE_OFFSET_NEIGH)
       buffer_trace(4, l1s.actual_time.fn, rx_radio_freq,
         l1s.tpu_win,l1s.tpu_offset);
   #endif
   #endif
  }

    #if (L1_GPRS)

    //In case of network mode of operation II or III, CCCH reading is possible
    //in packet idle mode and in packet transfer mode.

    if (((task == EP) || (task == NP)) &&
        ((l1a_l1s_com.l1s_en_task[PNP]    == TASK_ENABLED) ||
         (l1a_l1s_com.l1s_en_task[PEP]    == TASK_ENABLED) ||
         (l1a_l1s_com.l1s_en_task[PALLC]  == TASK_ENABLED) ||
         (l1a_l1s_com.l1s_en_task[PDTCH]  == TASK_ENABLED) ||
         (l1a_l1s_com.l1s_en_task[SINGLE] == TASK_ENABLED)))
    {
        if((burst_id == BURST_4) && algo_change_synchro_active)
        {

            // Slide synchro back to mach current serving timeslot.
            l1dmacro_synchro(SWITCH_TIME, l1s.tpu_offset);


            // Increment frame number.
            l1s.actual_time    = l1s.next_time;
            l1s.next_time      = l1s.next_plus_time;
            l1s_increment_time (&(l1s.next_plus_time), 1);  // Increment "next_plus time".

            l1s.tpu_ctrl_reg |= CTRL_SYCB;
            l1s.dsp_ctrl_reg |= CTRL_SYNC;
            l1s.ctrl_synch_before = FALSE;
            algo_change_synchro_active = FALSE;

            #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
              trace_fct(CST_L1S_ADJUST_TIME, (UWORD32)(-1));//OMAPS00090550
            #endif
        }
    }

    // in case of EBCCHS and NBCCHS in packet transfer a change synchro is performed
    else if (BCCHS_in_transfert)
    {
      // Shift TPU SYNCHRO/OFFSET registers back to the default timeslot .
      // ****************************************************************
      // When the E/NBCCHS reading control is completed ,
      // the SYNCHRO/OFFSET registers are shifted back to the normal
      // setting used for PCCH reading on the serving cell.
      // Check if "Synchro" change was needed.
      // If so the synchro is changed to recover normal synchro.
      if(burst_id == BURST_4)
      {
        if(change_synchro) // TS= [1,2,3,4]
        {
          // Slide synchro back to mach current serving timeslot.
          l1dmacro_synchro(SWITCH_TIME, l1s.tpu_offset);

          // Increment frame number.
          l1s.actual_time    = l1s.next_time;
          l1s.next_time      = l1s.next_plus_time;
          l1s_increment_time(&(l1s.next_plus_time), 1);  // Increment "next_plus time".

          #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
            trace_fct(CST_L1S_ADJUST_TIME, (UWORD32)(-1));//OMAPS00090550
          #endif

          l1s.tpu_ctrl_reg |= CTRL_SYCB;
          l1s.dsp_ctrl_reg |= CTRL_SYNC;
        }
      }

      // This task is not compatible with Neigh. Measurement. Store task length
      // in "forbid_meas" to indicate when the task will last.
      if(burst_id == BURST_1)
        l1s.forbid_meas = TASK_ROM_MFTAB[task].size;
    }
  #endif

  // Flag the reading of a Normal Paging burst.
  // *******************************************

  // Set flag "NP contoled !!". Used in "l1_synch()" to generate meas. controles.
  if(task == NP)
     l1a_l1s_com.ba_list.np_ctrl = burst_id+1;

  // Flag DSP and TPU programmation.
  // ********************************

  // Set "CTRL_RX" flag in the controle flag register.
  l1s.tpu_ctrl_reg |= CTRL_RX;
  l1s.dsp_ctrl_reg |= CTRL_RX;
}

//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END
#endif

/*-------------------------------------------------------*/
/* l1s_ctrl_snb_ul()                                     */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* serving cell normal burst sending tasks: DUL, AUL.    */
/* This function is the control function for sending a   */
/* burst on a SDCCH channel. It programs the DSP and the */
/* TPU for sending a normal burst taking into account    */
/* the timing adavance. Here is a summary of the         */
/* execution:                                            */
/*                                                       */
/*  - Catch ARFCN.                                       */
/*  - Traces and debug.                                  */
/*  - Programs DSP for required task.                    */
/*  - Catch UL data block from DLL and gives it to DSP.  */
/*  - Programs TPU for required task.                    */
/*  - Flag DSP and TPU programmation.                    */
/*                                                       */
/* Input parameters:                                     */
/* -----------------                                     */
/* "task"                                                */
/*   DUL, SDCCH UPLINK sending task.                     */
/*   AUL, SACCH UPLINK (associated with SDCCH)sending    */
/*   task.                                               */
/*                                                       */
/* "burst_id"                                            */
/*   BURST_1, 1st burst of the task.                     */
/*   BURST_2, 2nd burst of the task.                     */
/*   BURST_3, 3rd burst of the task.                     */
/*   BURST_4, 4th burst of the task.                     */
/*                                                       */
/* Input parameters from globals:                        */
/* ------------------------------                        */
/* "l1a_l1s_com.dedic_set"                               */
/*   Dedicated channel parameter structure. It is used   */
/*   to get the ARFCN to use for SDCCH (DUL, AUL). This  */
/*   ARFCN comes from the HOPPING algorithm called just  */
/*   before calling this function.                       */
/*                                                       */
/* "l1a_l1s_com.Scell_info"                              */
/*  Serving cell information structure.                  */
/*    .bsic, BSIC of the serving cell. It is used here   */
/*           to pass the training sequence number (part  */
/*           of BSIC) to the DSP.                        */
/*                                                       */
/* "l1s.afc"                                             */
/*   current AFC value to be applied for the given task. */
/*                                                       */
/* "l1s.tpu_offset"                                      */
/*   value for the TPU SYNCHRO and OFFSET registers      */
/*   for current serving cell setting. It is used here   */
/*   to restore this value in the OFFSET register after  */
/*   the TX burst programming.                           */
/*                                                       */
/* "l1s.applied_txpwr"                                   */
/*   Applied transmit power.                             */
/*                                                       */
/* "l1s.reported_txpwr"                                  */
/*   Transmit power to report in the L1 header of the    */
/*   SACCH data block.                                   */
/*                                                       */
/* "l1a_l1s_com.dedic_set.aset"                          */
/*   Active dedicated mode parameter set.                */
/*    .timing_advance, Timing advance to apply to the UL */
/*                     burst transmission.               */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/* "l1s.tpu_win"                                         */
/*   each frame is composed with a maximum of 3          */
/*   working/TPU windows (typically RX/TX/PW). This is   */
/*   a counter used to count the number of windows       */
/*   used.                                               */
/*   -> set to TDMA_WIN3.                                */
/*                                                       */
/* "l1s.tpu_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/TPU com.   */
/*   This is used mainly to swap then the com. page at   */
/*   the end of a control frame.                         */
/*   -> set CTRL_TX bit in the register.                 */
/*                                                       */
/* "l1s.dsp_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/DSP com.   */
/*   This is used mainly to swap then the com. page at   */
/*   the end of a control frame.                         */
/*   -> set CTRL_TX bit in the register.                 */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_ctrl_snb_ul(UWORD8 task, UWORD8 burst_id)
{
  T_RADIO_FRAME  *tx_data = NULL;
  UWORD16         tx_radio_freq;
  UWORD32         dsp_task;
  UWORD8          adc_active_ul = INACTIVE;

  // Catch ARFCN.
  // *************

  // Get ARFCN to be used for current control.
  tx_radio_freq = l1a_l1s_com.dedic_set.radio_freq;

  // Traces and debug.
  // ******************

  #if (TRACE_TYPE==5) && FLOWCHART
    trace_flowchart_dsp_tpu(dltsk_trace[task].name);
    if(burst_id == BURST_1) trace_flowchart_dsptx(dltsk_trace[task].name);
  #endif

  #if (TRACE_TYPE!=0)
    trace_fct(CST_L1S_CTRL_SNB_UL, tx_radio_freq);
  #endif

  l1s_dsp_com.dsp_db_w_ptr->d_debug = (l1s.debug_time + 2) ;

  // Programs DSP for required task.
  // ********************************

  // Set CIPHERING reduced frame number.
  #if (AMR == 1)
    #if (FF_L1_TCH_VOCODER_CONTROL == 1)
      l1ddsp_load_tch_param(&(l1s.next_time),
                            SIG_ONLY_MODE,
                            l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type,
                          #if !FF_L1_IT_DSP_DTX
                            0, 0, 0, 0, 0, 0);
    #else
                            0, 0, 0, 0, 0, 0, 0);
                          #endif
    #else
      l1ddsp_load_tch_param(&(l1s.next_time),
                            SIG_ONLY_MODE,
                            l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type,
                          #if !FF_L1_IT_DSP_DTX
                            0, 0, 0, 0);
                          #else
                            0, 0, 0, 0, 0);
                          #endif
    #endif
  #else
    #if (FF_L1_TCH_VOCODER_CONTROL == 1)
      l1ddsp_load_tch_param(&(l1s.next_time),
                            SIG_ONLY_MODE,
                            l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type,
                          #if !FF_L1_IT_DSP_DTX
                            0, 0, 0, 0, 0);
                          #else
                            0, 0, 0, 0, 0, 0);
                          #endif
    #else
      l1ddsp_load_tch_param(&(l1s.next_time),
                            SIG_ONLY_MODE,
                            l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type,
                          #if !FF_L1_IT_DSP_DTX
                            0, 0, 0);
                          #else
                            0, 0, 0, 0);
                          #endif
    #endif
  #endif

  if(task == DUL)
  // SDCCH/UL task.
  {
    if(l1a_l1s_com.dedic_set.aset->ho_acc_to_send != 0)
    // "ho_acc_to_send" is a counter of Handover Access burst still to send.
    // This counter is set by "l1s_dedicated_mode_manager()" in L1S when a
    // Handover command is received from L3 through L1A.
    {
      // TX burst is a RACH.
      // ********************
      // dsp and tpu pgm...
      l1s_ctrl_rach(RAHO,NO_PAR);

      // Decrement number of HO ACCESS burst still to be sent.
      // Rem: (-1) is used for Async. HO.
      if(l1a_l1s_com.dedic_set.aset->ho_acc_to_send != -1)
        l1a_l1s_com.dedic_set.aset->ho_acc_to_send --;

      if(l1a_l1s_com.dedic_set.aset->ho_acc_to_send == 0)
      // Handover access procedure is completed.
      // -> send L1C_HANDOVER_FINISHED message with "cause = COMPLETED" to L1A.
      {
        l1s_send_ho_finished(HO_COMPLETE);
      }
    }

    else
    {
      // TX burst is a Normal Burst.
      // ****************************
      // dsp pgm...

      #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
        RTTL1_FILL_UL_NB(task, l1a_l1s_com.dedic_set.aset->timing_advance, l1s.applied_txpwr)
      #endif

      dsp_task = l1s_swap_iq_ul(tx_radio_freq,task);

      l1ddsp_load_tx_task(dsp_task,
                          burst_id,
                          l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->tsc);

      // tpu pgm...
      l1dtpu_serv_tx_nb(tx_radio_freq,
                        l1a_l1s_com.dedic_set.aset->timing_advance,
                        l1s.tpu_offset,
                        l1s.applied_txpwr,INACTIVE);

      // Catch UL data block from DLL and gives it to DSP.
      // **************************************************
      // SDCCH info.
      if(burst_id == BURST_1)
      // perform "PH_DATA_REQ" from L2...
      {
        // Get SDCCH/UL data block from L2.
        tx_data = dll_read_dcch(SIG_ONLY_MODE);

        // Store the UL data block in MCU/DSP interface.
        if(tx_data != NULL)  // NULL should never occur !!!
        {
          #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
            RTTL1_FILL_UL_DCCH
            trace_info.facch_ul_count ++;
          #endif

          l1ddsp_load_info(DSP_TASK_CODE[task], l1s_dsp_com.dsp_ndb_ptr->a_cu, &(tx_data->A[0]));
        }
      }
    }

    // In any case set TXPWR.
    l1ddsp_load_txpwr(l1s.applied_txpwr, tx_radio_freq);

  }  // End if(task == DUL)

  else
  // SACCH/UL task.
  {

    if(l1a_l1s_com.dedic_set.aset->ho_acc_to_send != 0)
    // "ho_acc_to_send" is a counter of Handover Access burst still to send.
    // This counter is set by "l1s_dedicated_mode_manager()" in L1S when a
    // Handover command is received from L3 through L1A.
    // Rem: it is not allowed to send HO ACCESS burst on SACCH/UL. We must
    // then avoid any normal burst transmission by setting txpwr=NO_TXPWR. The DSP
    // and TPU are controled normally.
    {
      // Set TXPWR.
      l1ddsp_load_txpwr(NO_TXPWR, tx_radio_freq);

      #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
        RTTL1_FILL_UL_NB(task, l1a_l1s_com.dedic_set.aset->timing_advance, NO_TXPWR)
      #endif
    }

    else
    {
      // Set TXPWR.
      l1ddsp_load_txpwr(l1s.applied_txpwr, tx_radio_freq);

      #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
        RTTL1_FILL_UL_NB(task, l1a_l1s_com.dedic_set.aset->timing_advance, l1s.applied_txpwr)
      #endif
    }

    //ADC Measurements
    //

    // Check if during the SACCH burst an ADC measurement shall be performed

    if (l1a_l1s_com.adc_mode & ADC_NEXT_TRAFFIC_UL)  // perform ADC only one time
    {
       adc_active_ul = ACTIVE;
       l1a_l1s_com.adc_mode &= ADC_MASK_RESET_TRAFFIC; // reset in order to have only one ADC measurement in Traffic
    }
    else
    {
      if (l1a_l1s_com.adc_mode & ADC_EACH_TRAFFIC_UL) // perform ADC on each period bloc
      {
        if ((++l1a_l1s_com.adc_cpt)>=l1a_l1s_com.adc_traffic_period) // wait for the period
        {
          adc_active_ul = ACTIVE;
          l1a_l1s_com.adc_cpt = 0;
        }
      }
    }


    // In any case TX burst is a Normal Burst.
    // ****************************************
    // dsp pgm...

    dsp_task = l1s_swap_iq_ul(tx_radio_freq,task);

    l1ddsp_load_tx_task(dsp_task,
                        burst_id,
                        l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->tsc);

    // tpu pgm...
    l1dtpu_serv_tx_nb(tx_radio_freq,
                      l1a_l1s_com.dedic_set.aset->timing_advance,
                      l1s.tpu_offset,
                      l1s.applied_txpwr,adc_active_ul);


    // Catch UL data block from DLL and gives it to DSP.
    // **************************************************
    // SACCH info.
    if(burst_id == BURST_1)
    // perform "PH_DATA_REQ" from L2...
    {

      #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4))
      BOOL tx_data_flag = FALSE; //omaps00090550
      #endif

          #if (FF_REPEATED_SACCH == 1)
	  	    #if TESTMODE
	  	        if(l1_config.repeat_sacch_enable != REPEATED_SACCH_ENABLE) /* disable the repeated sacch mode  */
	  	         {
	  	             l1s.repeated_sacch.sro = 0; /* set no repetition order */
	  	             l1s.repeated_sacch.buffer_empty = TRUE; /* set no buffer */
	  	         }
	  	    #endif /* TESTMODE */
	  #endif /* (FF_REPEATED_SACCH == 1) */
    #if (FF_REPEATED_SACCH == 1)
      /* Get data from PS if only no repetition order is required (1st condition)
         or no repetition candidate exists (2nd condition) */
      if(    (l1s.repeated_sacch.sro == 0) || (l1s.repeated_sacch.buffer_empty == TRUE)    )
{
    #endif  /* (FF_REPEATED_SACCH == 1) */
      tx_data = dll_read_sacch(SIG_ONLY_MODE);

      if(tx_data != NULL)  // NULL should never occur !!!
      {
#if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
        tx_data_flag =TRUE;  //omaps00090550
#endif
        // Set L1 Header...
        tx_data->A[0] = l1s.reported_txpwr;
    #if (FF_REPEATED_SACCH == 1)
		/* Include the SACCH Repetition Request (SRR) in the L1 Header */
		tx_data->A[0] |= (l1s.repeated_sacch.srr <<6);

   #endif  /* FF_REPEATED_SACCH */
        tx_data->A[1] = l1a_l1s_com.dedic_set.aset->timing_advance;

        // Store the UL data block in MCU/DSP interface.
        l1ddsp_load_info(DSP_TASK_CODE[task], l1s_dsp_com.dsp_ndb_ptr->a_cu, &(tx_data->A[0]));
           #if (FF_REPEATED_SACCH == 1 )
		        /* Store the block data in case of a retransmission order */
		        /* Retransmission is done in case of a SAPI0 and not 3    */
		          if(((tx_data->A[2]&0x1C) >> 2) == SAPI_0)
		          {
		              l1s_store_sacch_buffer( &(l1s.repeated_sacch), &(tx_data->A[0]));
		           }
		          else // FIXME FIXME NOT sure whether this needs to be done
		          {
		            /* the SACCH repetition block occurrence will always come as a consecutive pair */
		            /* To handle DL UL | DL  UL  | DL UL                                             */
		            /*            -               0 | SRO  3  | -   new data should be asked from PS old 0 cannot be repeated */
		                 l1s.repeated_sacch.buffer_empty=TRUE;
		          }
		  #endif /* FF_REPEATED_SACCH */

       }/*  end of   if(tx_data != NULL) */
#if (FF_REPEATED_SACCH == 1)
     }

      else if ((l1s.repeated_sacch.sro == 1) && (l1s.repeated_sacch.buffer_empty == FALSE))
      {
         /*  Put data block in MCU/DSP com.  */
         l1ddsp_load_info(DSP_TASK_CODE[task], l1s_dsp_com.dsp_ndb_ptr->a_cu, l1s.repeated_sacch.buffer );
         l1s.repeated_sacch.buffer_empty = TRUE;   /* Set that the buffer is now empty (only one repetition) */
      }
#endif /* FF_REPEATED_SACCH */

      #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4))
          RTTL1_FILL_UL_SACCH(tx_data_flag,l1a_l1s_com.dedic_set.aset->timing_advance, l1s.reported_txpwr)   //omaps00090550
      #endif

    }
  }

  // Set tpu window identifier for Power meas after TX.
  l1s.tpu_win = (3 * BP_SPLIT) + l1_config.params.tx_nb_load_split + l1_config.params.rx_synth_load_split;

  // Flag DSP and TPU programmation.
  // ********************************

  // Set "CTRL_TX" flag in the controle flag register.
  l1s.tpu_ctrl_reg |= CTRL_TX;
  l1s.dsp_ctrl_reg |= CTRL_TX;
}

#if (MOVE_IN_INTERNAL_RAM == 0) // Must be followed by the pragma used to duplicate the funtion in internal RAM
//#pragma DUPLICATE_FOR_INTERNAL_RAM_START

/*-------------------------------------------------------*/
/* l1s_ctrl_nnb()                                        */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* neigbor cell normal burst reading tasks: BCCHN.       */
/* This function is the control function for reading 4   */
/* normal bursts on a neighbor cell. It programs the DSP */
/* and the TPU for reading the 4 bursts taking into      */
/* account the time difference between the serving and   */
/* the neighbor cells. To this avail, it shifts the TPU  */
/* OFFSET register according to this time difference and */
/* restores the serving offset value when the 4 burst    */
/* reading are completed. Here is a summary of the       */
/* execution:                                            */
/*                                                       */
/*  - If SEMAPHORE(task) is low.                         */
/*      - Traces and debug.                              */
/*      - Programs DSP for required task.                */
/*      - Programs TPU for required task.                */
/*  - Flag DSP and TPU programmation.                    */
/*                                                       */
/* Input parameters:                                     */
/* -----------------                                     */
/* "task"                                                */
/*   BCCHN, BCCH Neighbor reading task.                  */
/*                                                       */
/* Input parameters from globals:                        */
/* ------------------------------                        */
/* "l1a_l1s_com.Ncell_info.bcch"                         */
/*   cell information structure used for BCCHN task.     */
/*                                                       */
/* "l1a_l1s_com.l1s_en_task"                             */
/*   L1S task enable bit register. Used here to check if */
/*   the Full BCCH Reading task is enabled and then to   */
/*   take the decision to reloading the serving value    */
/*   in the TPU OFFSET register.                         */
/*                                                       */
/* "l1s.tpu_offset"                                      */
/*   value for SYNCHRO and OFFSET register in the TPU    */
/*   for current serving cell setting. At the end of     */
/*   the task this value is restored in the OFFSET       */
/*   register.                                           */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/* "l1s.tpu_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/TPU com.   */
/*   This is used mainly to swap then the com. page at   */
/*   the end of a control frame.                         */
/*   -> set CTRL_RX bit in the register.                 */
/*                                                       */
/* "l1s.dsp_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/DSP com.   */
/*   This is used mainly to swap then the com. page at   */
/*   the end of a control frame.                         */
/*   -> set CTRL_RX bit in the register.                 */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_ctrl_nnb(UWORD8 task, UWORD8 param2)
{
  UWORD8           lna_off;
  WORD8            agc;
  T_NCELL_SINGLE  *cell_info_ptr = NULL;
  BOOL             en_task;
  BOOL             task_param;
  UWORD32          dsp_task;
#if(L1_FF_MULTIBAND == 1)
 UWORD16 operative_radio_freq;
 UWORD8  input_level;
#endif
  
#if (RF_FAM == 61)
       UWORD16 dco_algo_ctl_nb = 0;
       UWORD8 if_ctl = 0;
	   UWORD8 if_threshold = C_IF_ZERO_LOW_THRESHOLD_GSM;
#endif

  // Get "enable" task flag and "synchro semaphore" for current task.
  en_task    = l1a_l1s_com.l1s_en_task[task];
  task_param = l1a_l1s_com.task_param[task];



  if((en_task) && !(task_param))
  // Check the task semaphore and enable flag. The control body is executed only
  // when the task semaphore is 0 and enable flag is 1. The semaphore can be set to
  // 1 whenever L1A makes some changes to the task parameters. The enable can be
  // reset to 0 when the task is no more enabled.
  {
    // Get the cell information structure.
    // ************************************
    if (task == BCCHN)
      cell_info_ptr = &l1a_l1s_com.bcchn.list[l1a_l1s_com.bcchn.active_neigh_id_norm];
    else // BCCHN_TOP and BCCHN_TRAN tasks
      cell_info_ptr = &l1a_l1s_com.bcchn.list[l1a_l1s_com.bcchn.active_neigh_id_top];

    // Traces and debug.
    // ******************

    #if (TRACE_TYPE!=0)
      trace_fct(CST_L1S_CTRL_NNB, cell_info_ptr->radio_freq);
    #endif

    #if (TRACE_TYPE==5) && FLOWCHART
      trace_flowchart_dsp_tpu(dltsk_trace[task].name);
    #endif

    l1s_dsp_com.dsp_db_w_ptr->d_debug = (l1s.debug_time + 6) ;

    // Programs DSP for required task.
    // ********************************

    #if L1_GPRS
      switch(l1a_l1s_com.dsp_scheduler_mode)
      {
        // dsp pgm is made using GSM scheduler...
        case GSM_SCHEDULER:
          dsp_task = l1s_swap_iq_dl(cell_info_ptr->radio_freq,task);
          l1ddsp_load_rx_task(dsp_task,
                              0,
                              cell_info_ptr->tsc);
        break;

        // dsp pgm is made using GPRS scheduler...
        case GPRS_SCHEDULER:
          l1pddsp_load_bcchn_task(cell_info_ptr->tsc, cell_info_ptr->radio_freq);
        break;
      }
    #else
      // dsp pgm...
      dsp_task = l1s_swap_iq_dl(cell_info_ptr->radio_freq,task);
      l1ddsp_load_rx_task(dsp_task,
                          0,
                          cell_info_ptr->tsc);
    #endif

    // Programs TPU for required task.
    // ********************************

    // We program 4 burst reading. The OFFSET register is used to
    // cope with the time difference between the serving and the
    // neighbor cells. The serving cell offset value (l1s.tpu_offset)
    // is restored at the end of the 4 burst reading.
#if (L1_FF_MULTIBAND == 0)

    // agc is computed from PGC2 algo result.
    agc     = Cust_get_agc_from_IL(cell_info_ptr->radio_freq, l1a_l1s_com.last_input_level[cell_info_ptr->radio_freq - l1_config.std.radio_freq_index_offset].input_level >> 1, AV_ID);
    lna_off = l1a_l1s_com.last_input_level[cell_info_ptr->radio_freq - l1_config.std.radio_freq_index_offset].lna_off;

#else // L1_FF_MULTIBAND = 1 below

    operative_radio_freq = 
      l1_multiband_radio_freq_convert_into_operative_radio_freq(cell_info_ptr->radio_freq);
    lna_off = l1a_l1s_com.last_input_level[operative_radio_freq].lna_off;
    input_level = l1a_l1s_com.last_input_level[operative_radio_freq].input_level;
    // agc is computed from PGC2 algo result.
    agc     = Cust_get_agc_from_IL(cell_info_ptr->radio_freq, l1a_l1s_com.last_input_level[operative_radio_freq].input_level >> 1, AV_ID);

#endif // #if (L1_FF_MULTIBAND == 0) else



    #if(RF_FAM == 61)  // Locosto DCO
        cust_get_if_dco_ctl_algo(&dco_algo_ctl_nb, &if_ctl, (UWORD8) L1_IL_VALID,
#if (L1_FF_MULTIBAND == 0)        
                                       l1a_l1s_com.last_input_level[cell_info_ptr->radio_freq - l1_config.std.radio_freq_index_offset].input_level,
#else
                                       input_level,
#endif                                        
                                        cell_info_ptr->radio_freq,if_threshold);

        //dco_algo_ctl has 0000 00ZL
         dco_algo_ctl_nb *= 0x55;   // replicate 0000 00zL as ZLZL ZLZL

        l1ddsp_load_dco_ctl_algo_nb(dco_algo_ctl_nb);
    #endif

    #if L2_L3_SIMUL
      #if (DEBUG_TRACE == BUFFER_TRACE_OFFSET_NEIGH)
        buffer_trace(4, l1s.actual_time.fn, cell_info_ptr->radio_freq,
                        cell_info_ptr->time_alignmt, cell_info_ptr->fn_offset);
      #endif
    #endif

  #if (RF_FAM == 61)
      if ( cell_info_ptr->time_alignmt >= TPU_CLOCK_RANGE - EPSILON_SYNC)
      {
        // Insert 1 NOP to correct the EPSILON_SYNC side effect
        l1dtpu_neig_rx_nb(cell_info_ptr->radio_freq,
                          agc,
                          lna_off,
                          cell_info_ptr->time_alignmt,
                          l1s.tpu_offset,
                          FALSE, 1,
   			     if_ctl
            #if (NEW_SNR_THRESHOLD == 1)
             ,SAIC_OFF
            #endif /* NEW_SNR_THRESHOLD == 1*/
             );
      }
      else
      {
        l1dtpu_neig_rx_nb(cell_info_ptr->radio_freq,
                          agc,
                          lna_off,
                          cell_info_ptr->time_alignmt,
                          l1s.tpu_offset,
                          FALSE, 0,
   			     if_ctl
             #if (NEW_SNR_THRESHOLD == 1)
             ,SAIC_OFF
            #endif /* NEW_SNR_THRESHOLD == 1*/
             );
      }

      l1dtpu_neig_rx_nb(cell_info_ptr->radio_freq,
                        agc,
                        lna_off,
                        cell_info_ptr->time_alignmt,
                        l1s.tpu_offset,
                        FALSE, 0,
   			     if_ctl
             #if (NEW_SNR_THRESHOLD == 1)
             ,SAIC_OFF
            #endif /* NEW_SNR_THRESHOLD == 1*/
             );
      l1dtpu_neig_rx_nb(cell_info_ptr->radio_freq,
                        agc,
                        lna_off,
                        cell_info_ptr->time_alignmt,
                        l1s.tpu_offset,
                        FALSE, 0,
   			     if_ctl
            #if (NEW_SNR_THRESHOLD == 1)
             ,SAIC_OFF
            #endif /* NEW_SNR_THRESHOLD == 1*/
             );
      l1dtpu_neig_rx_nb(cell_info_ptr->radio_freq,
                        agc,
                        lna_off,
                        cell_info_ptr->time_alignmt,
                        l1s.tpu_offset,
                        TRUE, 0,
   			     if_ctl
            #if (NEW_SNR_THRESHOLD == 1)
             ,SAIC_OFF
            #endif /* NEW_SNR_THRESHOLD == 1*/
             );
  #endif // RF_FAM == 61

  #if (RF_FAM != 61)
      if ( cell_info_ptr->time_alignmt >= TPU_CLOCK_RANGE - EPSILON_SYNC)
      {
        // Insert 1 NOP to correct the EPSILON_SYNC side effect
        l1dtpu_neig_rx_nb(cell_info_ptr->radio_freq,
                          agc,
                          lna_off,
                          cell_info_ptr->time_alignmt,
                          l1s.tpu_offset,
                          FALSE, 1);
      }
      else
      {
        l1dtpu_neig_rx_nb(cell_info_ptr->radio_freq,
                          agc,
                          lna_off,
                          cell_info_ptr->time_alignmt,
                          l1s.tpu_offset,
                          FALSE, 0);
      }

      l1dtpu_neig_rx_nb(cell_info_ptr->radio_freq,
                        agc,
                        lna_off,
                        cell_info_ptr->time_alignmt,
                        l1s.tpu_offset,
                        FALSE, 0);
      l1dtpu_neig_rx_nb(cell_info_ptr->radio_freq,
                        agc,
                        lna_off,
                        cell_info_ptr->time_alignmt,
                        l1s.tpu_offset,
                        FALSE, 0);
      l1dtpu_neig_rx_nb(cell_info_ptr->radio_freq,
                        agc,
                        lna_off,
                        cell_info_ptr->time_alignmt,
                        l1s.tpu_offset,
                        TRUE, 0);
  #endif // RF_FAM != 61
  }

  // Flag DSP and TPU programmation.
  // ********************************

  // Set "CTRL_RX" flag in the controle flag register.
  l1s.tpu_ctrl_reg |= CTRL_RX;
  l1s.dsp_ctrl_reg |= CTRL_RX;

  #if L1_GPRS
  // This task is not compatible with Neigh. Measurement. Store task length
  // in "forbid_meas" to indicate when the task will last.
  if(task == BCCHN_TRAN)
  {
    // In IDLE mode, l1s.forbid_meas is setted by the AGC ctrl
    l1s.forbid_meas = TASK_ROM_MFTAB[task].size;
  }
  #endif
}
//#pragma DUPLICATE_FOR_INTERNAL_RAM_END
#endif // MOVE_IN_INTERNAL_RAM

/*-------------------------------------------------------*/
/* l1s_ctrl_rach()                                       */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* serving cell Random Access burst sending tasks: RAAC, */
/* RAHO. This function is the control function for       */
/* sending a random access burst to the serving cell.    */
/* This sending is either a Channel Request (connection  */
/* establishment) or a Handover Access burst (dedicated  */
/* mode). It programs the DSP and the TPU for sending a  */
/* random access burst with a null timing advance.       */
/* Here is a summary of the execution:                   */
/*                                                       */
/*  - Traces and debug.                                  */
/*  - Programs DSP for required task.                    */
/*  - Build RACH data block and store in MCU/DSP com.    */
/*  - Programs TPU for required task.                    */
/*  - Send confirmation msg to L1A.                      */
/*  - Flag DSP and TPU programmation.                    */
/*                                                       */
/* Input parameters:                                     */
/* -----------------                                     */
/* "task"                                                */
/*   RAAC, RACH sending task for Channel Request.        */
/*   RAHO, RACH sending task for Handover Access.        */
/*                                                       */
/* Input parameters from globals:                        */
/* ------------------------------                        */
/* "l1a_l1s_com.Scell_info"                              */
/*  Serving cell information structure.                  */
/*    .bsic, BSIC of the serving cell. It is used here   */
/*           to pass the training sequence number (part  */
/*           of BSIC) to the DSP.                        */
/*    .radio_freq, serving cell beacon frequency.             */
/*                                                       */
/* "l1s.afc"                                             */
/*   current AFC value to be applied for the given task. */
/*                                                       */
/* "l1s.tpu_offset"                                      */
/*   value for the TPU SYNCHRO and OFFSET registers      */
/*   for current serving cell setting. It is used here   */
/*   to restore this value in the OFFSET register after  */
/*   the TX burst programming.                           */
/*                                                       */
/* "l1s.applied_txpwr"                                   */
/*   Applied transmit power.                             */
/*                                                       */
/* "l1a_l1s_com.ra_info"                                 */
/*   random access task parameters.                      */
/*   .channel_request, random number sent in the case    */
/*      of Channel Request (RAAC).                       */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/* "l1s.tpu_win"                                         */
/*   each frame is composed with a maximum of 3          */
/*   working/TPU windows (typically RX/TX/PW). This is   */
/*   a counter used to count the number of windows       */
/*   used.                                               */
/*   -> set to TDMA_WIN3.                                */
/*                                                       */
/* "l1s.tpu_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/TPU com.   */
/*   This is used mainly to swap then the com. page at   */
/*   the end of a control frame.                         */
/*   -> set CTRL_TX bit in the register.                 */
/*                                                       */
/* "l1s.dsp_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/DSP com.   */
/*   This is used mainly to swap then the com. page at   */
/*   the end of a control frame.                         */
/*   -> set CTRL_TX bit in the register.                 */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_ctrl_rach(UWORD8 task, UWORD8 param2)
{
  UWORD8   tx_data[2];
  UWORD16  radio_freq=0;
  UWORD32  dsp_task;
  UWORD8   adc_active=INACTIVE;

  // Get ARFCN to be used for current control.
  // *******************************************

  if(task == RAHO)
  // Handover Access...
  {
    // The ARFCN comes from the HOPPING algorithm called
    // prior to calling any CTRL function in the current frame.
    radio_freq = l1a_l1s_com.dedic_set.radio_freq;
  }
  else
  // Network Access...
  {
    #if TESTMODE
      if (l1_config.TestMode)
      {
        // A TX_TCH task has been enabled in TestMode with burst_type=access burst.
        // Thus set radio_freq to tch_arfcn .
        radio_freq = l1_config.tmode.rf_params.tch_arfcn;
      }
      else
    #endif
    {
      // The ARFCN is the BEACON frequency.
      radio_freq = l1a_l1s_com.Scell_info.radio_freq;
    }
  }


  // ADC measurement
  // ***************

  // check if during the RACH an ADC measurement must be performed
  if (task == RAACC)
   if (l1a_l1s_com.adc_mode & ADC_EACH_RACH)  // perform ADC on each burst
       adc_active = ACTIVE;


  // Traces and debug.
  // ******************

  #if (TRACE_TYPE!=0)
    trace_fct(CST_L1S_CTRL_RACH, radio_freq);
  #endif

  #if (TRACE_TYPE==5) && FLOWCHART
    trace_flowchart_dsp_tpu(dltsk_trace[task].name);
  #endif

  l1s_dsp_com.dsp_db_w_ptr->d_debug = (l1s.debug_time + 2) ;

  #if (CODE_VERSION!=SIMULATION)
    #if (TRACE_TYPE==2 ) || (TRACE_TYPE==3)
      L1_trace_string("RA");
    #endif
  #endif

  // Programs DSP for required task.
  // ********************************

  // dsp pgm...

  dsp_task = l1s_swap_iq_ul(radio_freq,task);

  l1ddsp_load_ra_task(dsp_task);

  // Set TXPWR.
  l1ddsp_load_txpwr(l1s.applied_txpwr, radio_freq);

  // Build RACH data block and store in MCU/DSP com.
  // ************************************************

  // RACH info.
  if(task == RAACC)
  // RACH data is only the "channel_request". (BYTE format data).
  {
    tx_data[0] = (l1a_l1s_com.Scell_info.bsic << 2);
    tx_data[1] = l1a_l1s_com.ra_info.channel_request;

  }
  else
  // RACH data is only the "handover access" (BYTE format data).
  {
    tx_data[0] = (l1a_l1s_com.Scell_info.bsic << 2);
    tx_data[1] = l1a_l1s_com.dedic_set.aset->ho_acc;
  }

  // Store data block in MCU/DSP com.
  l1ddsp_load_info(DSP_TASK_CODE[task], &(l1s_dsp_com.dsp_ndb_ptr->d_rach), &(tx_data[0]));

  // Programs TPU for required task.
  // ********************************

  // tpu pgm...
  l1dtpu_serv_tx_ra(radio_freq, l1s.tpu_offset, l1s.applied_txpwr, adc_active);

  // Set tpu window identifier for Power meas if any.
  l1s.tpu_win = (3 * BP_SPLIT) + l1_config.params.tx_ra_load_split + l1_config.params.rx_synth_load_split;

  #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
    RTTL1_FILL_UL_AB(task, l1s.applied_txpwr)
  #endif

  // Flag DSP and TPU programmation.
  // ********************************

  // Set "CTRL_TX" flag in the controle flag register.
  l1s.tpu_ctrl_reg |= CTRL_TX;
  l1s.dsp_ctrl_reg |= CTRL_TX;
}

/*-------------------------------------------------------*/
/* l1s_ctrl_tchtd()                                      */
/*-------------------------------------------------------*/
/*                                                       */
/* Description: This function controls the non transmitting slot in case of Half Rate TCH  */
/* ------------                                          */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_ctrl_tchtd(UWORD8 task, UWORD8 param2)
{
  T_CHANNEL_DESCRIPTION  *desc_ptr;
  #if FF_L1_IT_DSP_DTX
    BOOL dtx_dsp_interrupt = FALSE;
  #endif

  // Traces and debug.
  // ******************

  #if (TRACE_TYPE==5) && FLOWCHART
    trace_flowchart_dsp_tpu(dltsk_trace[TCHD].name);
  #endif

  l1s_dsp_com.dsp_db_w_ptr->d_debug = (l1s.debug_time + 2) ;

  // Catch channel description.
  // ***************************

  // Catch the active channel description used along the routine.
  // It contains:
  //    "channel_type", {TCH_F, TCH_H, SDCCH_4, SDCCH_8}.
  //    "subchannel", {0, 1}. 0 is the default value for TCH_F.
  desc_ptr = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr;

  /**************************************************************************/
  /* TCH/Dummy Receive...                                                   */
  /**************************************************************************/
  #if (TRACE_TYPE!=0)
    trace_fct(CST_L1S_CTRL_TCHT_DUMMY__DL, l1a_l1s_com.dedic_set.radio_freq);
  #endif

  #if FF_L1_IT_DSP_DTX
    // Fast DTX active only in TCH AHS
    if ((l1a_l1s_com.dedic_set.aset->achan_ptr->mode == TCH_AHS_MODE)
        && (l1a_l1s_com.dedic_set.aset->dtx_allowed == TRUE))
    {
      // AHS0
      if (desc_ptr->subchannel == 0)
      {
        // DTX interrupt request for B1 and B2 (no DTX uncertainty on B0 thanks to idle frame)
        if (l1s.next_time.fn_mod13 <= 7)
          dtx_dsp_interrupt = TRUE;
      }

      // AHS1
      else
      {
        // DTX interrupt requested for ALL blocks (idle frame does not help)
        dtx_dsp_interrupt = TRUE;
      }
    }
  #endif

  /*--------------------------------------------*/
  /* Program DSP...                             */
  /*--------------------------------------------*/
  // dsp pgm.
  l1ddsp_load_rx_task(DSP_TASK_CODE[task], 0, desc_ptr->tsc);

  /*--------------------------------------------*/
  /* Flag DSP and TPU programmation...          */
  /*--------------------------------------------*/

  // Set "CTRL_RX" flag in the controle flag register.
  l1s.dsp_ctrl_reg |= CTRL_RX;


  /*----------------------------------------------*/
  /* Common for Dedicated mode: DSP parameters... */
  /*----------------------------------------------*/
  #if (AMR == 1)
    #if (FF_L1_TCH_VOCODER_CONTROL == 1)
      l1ddsp_load_tch_param(&(l1s.next_time),
                            l1a_l1s_com.dedic_set.aset->achan_ptr->mode,
                            desc_ptr->channel_type,
                            desc_ptr->subchannel,
                            l1a_l1s_com.dedic_set.aset->achan_ptr->tch_loop,
                            l1a_l1s_com.dedic_set.sync_tch,
                            0,
                            l1a_l1s_com.dedic_set.reset_sacch,
                          #if !FF_L1_IT_DSP_DTX
                            l1a_l1s_com.dedic_set.vocoder_on);
    #else
                            l1a_l1s_com.dedic_set.vocoder_on,
                            dtx_dsp_interrupt);
                          #endif
    #else
      l1ddsp_load_tch_param(&(l1s.next_time),
                            l1a_l1s_com.dedic_set.aset->achan_ptr->mode,
                            desc_ptr->channel_type,
                            desc_ptr->subchannel,
                            l1a_l1s_com.dedic_set.aset->achan_ptr->tch_loop,
                            l1a_l1s_com.dedic_set.sync_tch,
                          #if !FF_L1_IT_DSP_DTX
                            0);
  #else
                            0,
                            dtx_dsp_interrupt);
                          #endif
    #endif
  #else
    #if (FF_L1_TCH_VOCODER_CONTROL == 1)
      l1ddsp_load_tch_param(&(l1s.next_time),
                            l1a_l1s_com.dedic_set.aset->achan_ptr->mode,
                            desc_ptr->channel_type,
                            desc_ptr->subchannel,
                            l1a_l1s_com.dedic_set.aset->achan_ptr->tch_loop,
                            l1a_l1s_com.dedic_set.sync_tch,
                            l1a_l1s_com.dedic_set.reset_sacch,
                          #if !FF_L1_IT_DSP_DTX
                            l1a_l1s_com.dedic_set.vocoder_on);
                          #else
                            l1a_l1s_com.dedic_set.vocoder_on,
                            dtx_dsp_interrupt);
                          #endif
    #else
      l1ddsp_load_tch_param(&(l1s.next_time),
                            l1a_l1s_com.dedic_set.aset->achan_ptr->mode,
                            desc_ptr->channel_type,
                            desc_ptr->subchannel,
                            l1a_l1s_com.dedic_set.aset->achan_ptr->tch_loop,
                          #if !FF_L1_IT_DSP_DTX
                            l1a_l1s_com.dedic_set.sync_tch);
                          #else
                            l1a_l1s_com.dedic_set.sync_tch,
                            dtx_dsp_interrupt);
                          #endif
    #endif
  #endif

  // Clear "sync_tch" and "reset_sacch" flag to maintain normal TCH process.
  l1a_l1s_com.dedic_set.sync_tch = FALSE;
#if (FF_L1_TCH_VOCODER_CONTROL == 1)
  l1a_l1s_com.dedic_set.reset_sacch = FALSE;
#endif

  // Set tpu window identifier for Power meas in case of dummy burst in Half-rate
  l1s.tpu_win = 0;
}

/*-------------------------------------------------------*/
/* l1s_ctrl_tchth()                                      */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* dedicated mode TCH task: TCHTH. This function is the  */
/* control function for reading the DL burst and sending */
/* the UL burst. The UL burst can be a Normal Burst in   */
/* normal case or an Access Burst when starting a        */
/* Handover procedure. Both Half rate and Full rate TCH  */
/* channel are handled. The DSP and the TPU are          */
/* programmed for both the DL and UL bursts. The timing  */
/* advance is taken into account for positionning the UL */
/* burst.                                                */
/*                                                       */
/* This function accesses the L1/DLL and L1/DATA         */
/* interface ("dll_read_dcch()", "tx_tch_data()"         */
/* functions respectively) and passes then the returned  */
/* data blocks to the DSP.                               */
/*                                                       */
/* Here is a summary of the execution:                   */
/*                                                       */
/*  - Traces and debug.                                  */
/*  - Catch channel description and ARFCN.               */
/*  - TCH/T Receive...                                   */
/*      - Program DSP for RX.                            */
/*      - Program TPU for RX.                            */
/*      - Flag DSP and TPU programmation.                */
/*  - TCH/T Transmit...                                  */
/*      - If Any Handover Access burst to send           */
/*          - Call "l1s_ctrl_rach()".                    */
/*      - Else                                           */
/*          - Get DATA block if required for TCH.        */
/*          - Program DSP for TX.                        */
/*          - Program TPU for TX.                        */
/*          - Flag DSP and TPU programmation.            */
/*  - Common for DL/UL: DSP parameters.                  */
/*                                                       */
/* Input parameters:                                     */
/* -----------------                                     */
/* "task"                                                */
/*   TCHTH, Traffic Channel TCH Half rate.               */
/*                                                       */
/* Input parameters from globals:                        */
/* ------------------------------                        */
/* "l1a_l1s_com.dedic_set"                               */
/*   Dedicated channel parameter structure.              */
/*     .radio_freq, ARFCN value set by the Hopping algo.      */
/*     .aset, active dedicated parameter set.            */
/*                                                       */
/* "l1a_l1s_com.Scell_info"                              */
/*  Serving cell information structure.                  */
/*    .bsic, BSIC of the serving cell. It is used here   */
/*           to pass the training sequence number (part  */
/*           of BSIC) to the DSP.                        */
/*                                                       */
/* "l1s.afc"                                             */
/*   current AFC value to be applied for the given task. */
/*                                                       */
/* "l1s.tpu_offset"                                      */
/*   value for the TPU SYNCHRO and OFFSET registers      */
/*   for current serving cell setting.                   */
/*                                                       */
/* "l1s.applied_txpwr"                                   */
/*   Applied transmit power.                             */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/* "l1s.tpu_win"                                         */
/*   each frame is composed with a maximum of 3          */
/*   working/TPU windows (typically RX/TX/PW). This is   */
/*   a counter used to count the number of windows       */
/*   used.                                               */
/*   -> set to TDMA_WIN3.                                */
/*                                                       */
/* "l1s.tpu_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/TPU com.   */
/*   This is used mainly to swap then the com. page at   */
/*   the end of a control frame.                         */
/*   -> set CTRL_RX bit in the register.                 */
/*   -> set CTRL_TX bit in the register.                 */
/*                                                       */
/* "l1s.dsp_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/DSP com.   */
/*   This is used mainly to swap then the com. page at   */
/*   the end of a control frame.                         */
/*   -> set CTRL_RX bit in the register.                 */
/*   -> set CTRL_TX bit in the register.                 */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_ctrl_tchth(UWORD8 task, UWORD8 param2)
{
  UWORD16                 radio_freq=0;
  T_CHANNEL_DESCRIPTION  *desc_ptr;
  UWORD8                  lna_off;
  WORD8                   agc;
  T_INPUT_LEVEL          *IL_info_ptr;
  UWORD32                 dsp_task;
  UWORD32                 fn_mod_52;
  UWORD8 input_level;
#if (RF_FAM == 61)
  UWORD16 dco_algo_ctl_nb = 0;
  UWORD8 if_ctl = 0;
  UWORD8 if_threshold = C_IF_ZERO_LOW_THRESHOLD_GSM;
  // By default we choose the hardware filter
  UWORD8 csf_filter_choice = L1_SAIC_HARDWARE_FILTER;
#endif
#if FF_L1_IT_DSP_DTX
  BOOL                    dtx_dsp_interrupt=FALSE; //omaps00090550
#endif
#if (NEW_SNR_THRESHOLD == 1)
  UWORD8 saic_flag=0;
#endif /* NEW_SNR_THRESHOLD */
  // Traces and debug.
  // ******************

  #if (TRACE_TYPE==5) && FLOWCHART
    trace_flowchart_dsp_tpu(dltsk_trace[TCHTH].name);
  #endif

  l1s_dsp_com.dsp_db_w_ptr->d_debug = (l1s.debug_time + 2) ;

  // Catch channel description and ARFCN.
  // *************************************

  // Catch the active channel description used along the routine.
  // It contains:
  //    "channel_type", {TCH_F, TCH_H, SDCCH_4, SDCCH_8}.
  //    "subchannel", {0, 1}. 0 is the default value for TCH_F.
  desc_ptr = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr;

  // Get ARFCN to be used for current control. This  ARFCN comes from
  // the HOPPING algorithm called just before calling this function.
  radio_freq = l1a_l1s_com.dedic_set.radio_freq;

  if (radio_freq == l1a_l1s_com.Scell_info.radio_freq)
    IL_info_ptr = &l1a_l1s_com.Scell_info.traffic_meas_beacon;
                       // we are working on a beacon freq.
  else
    IL_info_ptr = &l1a_l1s_com.Scell_info.traffic_meas;
                       // we are working on a daughter freq.

#if FF_L1_IT_DSP_DTX
  // Skip RX DSP/RF programming part during DTX HISR
  if (l1a_apihisr_com.dtx.dtx_status != DTX_IT_DSP)
  {
#endif

  /**************************************************************************/
  /* TCH/T Receive...                                                       */
  /**************************************************************************/
  #if (TRACE_TYPE!=0)
    trace_fct(CST_L1S_CTRL_TCHTH__DL, radio_freq);
  #endif

  /*--------------------------------------------*/
  /* Program DSP...                             */
  /*--------------------------------------------*/

  dsp_task = l1s_swap_iq_dl(radio_freq,task);

  // dsp pgm.
  l1ddsp_load_rx_task(dsp_task, 0, desc_ptr->tsc);

  // Set TXPWR.
  l1ddsp_load_txpwr(l1s.applied_txpwr, radio_freq);

 input_level =  IL_info_ptr->input_level ;

  #if(RF_FAM == 61)   // Locosto DCO
    cust_get_if_dco_ctl_algo(&dco_algo_ctl_nb, &if_ctl, (UWORD8) L1_IL_VALID,
                                              input_level ,
                                              radio_freq,if_threshold);
    l1ddsp_load_dco_ctl_algo_nb(dco_algo_ctl_nb);
  #endif

  /*--------------------------------------------*/
  /* Program TPU...                             */
  /*--------------------------------------------*/
  // for TCHTH we use DPAGC algorithm.
  #if DPAGC_MAX_FLAG
    agc = Cust_get_agc_from_IL(radio_freq, input_level >> 1, MAX_ID);
  #else
    agc = Cust_get_agc_from_IL(radio_freq, input_level >> 1, AV_ID);
  #endif
  lna_off = IL_info_ptr->lna_off;


  // Store input_level and lna_off fields used for current CTRL in order to be able
  // to build IL from pm in READ phase.
  l1a_l1s_com.Scell_used_IL = *IL_info_ptr;

  #if (L1_SAIC != 0)
    // If SAIC is enabled, call the low level SAIC control function
    csf_filter_choice = l1ctl_saic(l1a_l1s_com.Scell_used_IL.input_level,l1a_l1s_com.mode
    #if (NEW_SNR_THRESHOLD == 1)
        ,task
        ,&saic_flag
    #endif
    );
  #endif

  // update the TPU with the new TOA if necessary
  l1ctl_update_TPU_with_toa();

  // Program a serving cell normal burst reading in TPU.
  #if (RF_FAM == 61)
    l1dtpu_serv_rx_nb(radio_freq,
                    agc,
                    lna_off,
                    l1s.tpu_offset,
                    l1s.tpu_offset,
                    FALSE,INACTIVE, csf_filter_choice, if_ctl
                    #if (NEW_SNR_THRESHOLD == 1)
                    ,saic_flag
                    #endif /*NEW_SNR_THRESHOLD */
                    );
  #endif
  #if (RF_FAM != 61)
    l1dtpu_serv_rx_nb(radio_freq,
                      agc,
                      lna_off,
                      l1s.tpu_offset,
                      l1s.tpu_offset,
                      FALSE,INACTIVE);
  #endif

  // Increment tpu window identifier.
  l1s.tpu_win += (l1_config.params.rx_synth_load_split + RX_LOAD);

  /*--------------------------------------------*/
  /* Flag DSP and TPU programmation...          */
  /*--------------------------------------------*/

  // Set "CTRL_RX" flag in the controle flag register.
  l1s.tpu_ctrl_reg |= CTRL_RX;
  l1s.dsp_ctrl_reg |= CTRL_RX;
#if FF_L1_IT_DSP_DTX
  } // if (l1a_apihisr_com.dtx.dtx_status != DTX_IT_DSP)
#endif

  /**************************************************************************/
  /* TCH/T Transmit...                                                      */
  /**************************************************************************/

  // Any Handover Access burst to send ?
  // ************************************

  if(l1a_l1s_com.dedic_set.aset->ho_acc_to_send != 0)
  // "ho_acc_to_send" is a counter of Handover Access burst still to send.
  // This counter is set by "l1s_dedicated_mode_manager()" in L1S when a
  // Handover command is received from L3 through L1A.
  // We must then replace the TCH UL normal burst by a RACH and decrement
  // this counter.
  {
    if(l1a_l1s_com.dedic_set.aset->ho_acc_to_send != -1)
      l1a_l1s_com.dedic_set.aset->ho_acc_to_send --;
    l1s_ctrl_rach(RAHO,NO_PAR);

    if(l1a_l1s_com.dedic_set.aset->ho_acc_to_send == 0)
    // Handover access procedure is completed.
    // -> send L1C_HANDOVER_FINISHED message with "cause = COMPLETED" to L1A.
    {
      l1s_send_ho_finished(HO_COMPLETE);
    }
  }
  else
  // TCH/UL is a normal burst.
  {
    UWORD8 channel_mode = l1a_l1s_com.dedic_set.aset->achan_ptr->mode;
    //omaps00090550 UWORD8 channel_type = desc_ptr->channel_type;
    UWORD8  subchannel = desc_ptr->subchannel;

    #if (TRACE_TYPE!=0)
      trace_fct(CST_L1S_CTRL_TCHTH__UL, radio_freq);
    #endif

#if FF_L1_IT_DSP_DTX
  // FACCH and IDS handled during L1S, have to be skipped during DTX HISR
  if (l1a_apihisr_com.dtx.dtx_status != DTX_IT_DSP)
  {
    #endif

    /*--------------------------------------------*/
    /* Get DATA block if required for TCH.        */
    /*--------------------------------------------*/
    // Half rate traffic channel...
    {
      // Rem: time to get data block is
      //      Subchannel 0: FN%26 in {0, 8, 17}.
      //      Subchannel 1: FN%26 in {1, 9, 18}.
      //   => normalised time: FN_normalised = FN%26 - subchannel, in {0, 8, 17}.
      // so CTL: must occur 2 TDMAs before, on
      //      Subchannel 0: FN%26 in {23, 6, 15}.
      //      Subchannel 1: FN%26 in {24, 7, 16}.
      UWORD8  normalised_fn = l1s.next_time.t2 - subchannel;  // FN%26 - subchannel

      if((normalised_fn == 23) || (normalised_fn == 6) || (normalised_fn == 15))
      // It is time to check if a FACCH/UL data block is available from DLL or
      // if a data block is available from the DATA interface.
      {
        T_RADIO_FRAME  *tx_data = NULL;

        // Check if any FACCH to transmit.
        // Rem: when mode is "SIGNALLING ONLY" the "dll_read_dcch()" function
        // always give back a block of FACCH (true block or dummy one).
        tx_data = dll_read_dcch(channel_mode);
        if(tx_data != NULL)
        {
          // In DTX mode in HS, all 6 FACCH 1/2 bursts must always be transmitted.
          // Note: FACCH presence is checked 1 "control" before "control" of 1st burst of FACCH due to a DSP constraint
          // i.e. 3 bursts before FACCH interleaving boundary
          // So we must wait 1 control before controlling the transmission of 6 FACCH 1/2 bursts
          l1s.facch_bursts = 7;

          #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
            RTTL1_FILL_UL_DCCH
            trace_info.facch_ul_count ++;
          #endif

          l1ddsp_load_info(DSP_TASK_CODE[task], l1s_dsp_com.dsp_ndb_ptr->a_fu, &(tx_data->A[0]));
          #if (TRACE_TYPE==5) && FLOWCHART
            trace_flowchart_dsptx(dltsk_trace[TCHTH].name);
          #endif
        }

        #if (AMR == 1)
          // Check if any DATA traffic info frame available.
          // This check is used for all full rate channels except when
          // this channel is in SIGNALLING ONLY mode or in Half Rate
          // Speech mode or in adaptative Half Rate mode.
          if((channel_mode != TCH_HS_MODE)  &&
             (channel_mode != TCH_AHS_MODE) &&
             (channel_mode != SIG_ONLY_MODE))
        #else
          // Check if any DATA traffic info frame available.
          // This check is used for all full rate channels except when
          // this channel is in SIGNALLING ONLY mode or in Half Rate
          // Speech mode.
          if((channel_mode != TCH_HS_MODE) && (channel_mode != SIG_ONLY_MODE))
        #endif
        {
          UWORD8  *tx_data = NULL;

          tx_data = tx_tch_data();
          if(tx_data != NULL)
          {
            // Store the DATA/UL data block in the MCU/DSP com. according
            // to the "subchannel".
            if(subchannel == 0) l1ddsp_load_info(DSP_TASK_CODE[task], l1s_dsp_com.dsp_ndb_ptr->a_du_0, tx_data);
            else                l1ddsp_load_info(DSP_TASK_CODE[task], l1s_dsp_com.dsp_ndb_ptr->a_du_1, tx_data);
            #if (TRACE_TYPE==5) && FLOWCHART
              trace_flowchart_dsptx(dltsk_trace[TCHTH].name);
            #endif
          }
        }
      }
    }

#if FF_L1_IT_DSP_DTX
    // Fast DTX active only in TCH AHS
    if ((l1a_l1s_com.dedic_set.aset->achan_ptr->mode == TCH_AHS_MODE)
        && (l1a_l1s_com.dedic_set.aset->dtx_allowed == TRUE))
    {
      // AHS0
      if (desc_ptr->subchannel == 0)
      {
        // DTX interrupt request for B1 and B2 (no DTX uncertainty on B0 thanks to idle frame)
        if (l1s.next_time.fn_mod13 <= 7)
          dtx_dsp_interrupt = TRUE;

        // DTX uncertainty check
        if  ((l1a_apihisr_com.dtx.fast_dtx_ready == TRUE) &&                   // Fast DTX can be used
             ((l1s.next_time.fn_mod13 == 4) || (l1s.next_time.fn_mod13 == 8))) // new block boundary
          l1a_apihisr_com.dtx.dtx_status = DTX_AWAITED;
      }

      // AHS1
      else
      {
        // DTX interrupt requested for ALL blocks (idle frame does not help)
        dtx_dsp_interrupt = TRUE;

        // DTX uncertainty check
        if  ((l1a_apihisr_com.dtx.fast_dtx_ready == TRUE) &&                   // Fast DTX can be used
             ((l1s.next_time.fn_mod13 == 1) || (l1s.next_time.fn_mod13 == 5)|| (l1s.next_time.fn_mod13 == 9))) // new block boundary
          l1a_apihisr_com.dtx.dtx_status = DTX_AWAITED;
      }
    }
    else
      l1a_apihisr_com.dtx.dtx_status = DTX_AVAILABLE;
  } // if (l1a_apihisr_com.dtx.dtx_status != DTX_IT_DSP)

  // Postpone TPU/DSP programming when DTX status not available from DSP
  if (l1a_apihisr_com.dtx.dtx_status != DTX_AWAITED)
  {
    BOOL tx_active =FALSE; //omaps00090550
#endif
    /*--------------------------------------------*/
    /* Program DSP...                             */
    /*--------------------------------------------*/

    dsp_task = l1s_swap_iq_ul(radio_freq,task);

    l1ddsp_load_tx_task(dsp_task, 0, desc_ptr->tsc);

    /*--------------------------------------------*/
    /* Program TPU...                             */
    /*--------------------------------------------*/

    fn_mod_52   = l1s.actual_time.fn % 52;

    l1s.facch_bursts--;
    if (l1s.facch_bursts < 0)
      l1s.facch_bursts = -1;
  #if FF_L1_IT_DSP_DTX
    // Condition for TX TPU programming channel mode dependant
    switch (channel_mode)
    {
      case SIG_ONLY_MODE:
      case TCH_24H_MODE:
      case TCH_48H_MODE:
        // DTX not supported
        tx_active = TRUE;
        break;

      case TCH_HS_MODE:
        if ((l1s.dtx_ul_on == FALSE) ||                                // No DTX
            ((l1s.facch_bursts >= 0) && (l1s.facch_bursts <= 5)) || // FACCH in progress
            ((subchannel == 0) && ((fn_mod_52 == 51) || (/*(fn_mod_52 >= 0) && omaps00090550*/(fn_mod_52 <= 5)))) || // SID HS0
            ((subchannel == 1) && (fn_mod_52 >= 13) && (fn_mod_52 <= 19)) // SID HS1
           )
          tx_active = TRUE;
        else
          tx_active = FALSE;
        break;

      case TCH_AHS_MODE:
        if (l1a_apihisr_com.dtx.tx_active) // DSP (Fast) DTX status
		tx_active = TRUE;
        else
          tx_active = FALSE;
        break;
    }

    // TPU TX burst programming
    if (tx_active)
  #else

    // In DTX mode, UL bursts should not be transmitted when no voice activity is detected
    // we must not call TPU scenario if dtx_on == TRUE in HS (See Technical Memo)
    // However, in DTX mode, several bursts must always be transmitted (See ETSI 05.08, 8.3)
    if ( (channel_mode != TCH_HS_MODE) ||
         (l1s.dtx_ul_on == FALSE) ||
         ( (l1s.facch_bursts >= 0) && (l1s.facch_bursts <= 5) ) ||
         ( (subchannel == 0) && ((fn_mod_52 == 51) || ((fn_mod_52 >= 0) && (fn_mod_52 <= 5))) ) ||
         ( (subchannel == 1) && (fn_mod_52 >= 13) && (fn_mod_52 <= 19) ))
  #endif
    {
        l1dtpu_serv_tx_nb(radio_freq, l1a_l1s_com.dedic_set.aset->timing_advance, l1s.tpu_offset, l1s.applied_txpwr,INACTIVE);
    }

    #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
      RTTL1_FILL_UL_NB(task, l1a_l1s_com.dedic_set.aset->timing_advance, l1s.applied_txpwr)
    #endif

  #if FF_L1_IT_DSP_DTX
    } // if (l1a_apihisr_com.dtx.dtx_status != DTX_AWAITED)
  #endif
  } // TCH/UL is a normal burst.


#if FF_L1_IT_DSP_DTX
  // Postpone TPU/DSP programming when DTX status not available from DSP
  if (l1a_apihisr_com.dtx.dtx_status != DTX_IT_DSP)
  {
#endif

  /*----------------------------------------------*/
  /* Common for Dedicated mode: DSP parameters... */
  /*----------------------------------------------*/
  #if (AMR == 1)
    #if (FF_L1_TCH_VOCODER_CONTROL == 1)
      l1ddsp_load_tch_param(&(l1s.next_time),
                            l1a_l1s_com.dedic_set.aset->achan_ptr->mode,
                            desc_ptr->channel_type,
                            desc_ptr->subchannel,
                            l1a_l1s_com.dedic_set.aset->achan_ptr->tch_loop,
                            l1a_l1s_com.dedic_set.sync_tch,
                            l1a_l1s_com.dedic_set.sync_amr,
                            l1a_l1s_com.dedic_set.reset_sacch,
                          #if !FF_L1_IT_DSP_DTX
                            l1a_l1s_com.dedic_set.vocoder_on);
    #else
                            l1a_l1s_com.dedic_set.vocoder_on,
                            dtx_dsp_interrupt);
                          #endif
    #else
      l1ddsp_load_tch_param(&(l1s.next_time),
                            l1a_l1s_com.dedic_set.aset->achan_ptr->mode,
                            desc_ptr->channel_type,
                            desc_ptr->subchannel,
                            l1a_l1s_com.dedic_set.aset->achan_ptr->tch_loop,
                            l1a_l1s_com.dedic_set.sync_tch,
                          #if !FF_L1_IT_DSP_DTX
                            l1a_l1s_com.dedic_set.sync_amr);
                          #else
                            l1a_l1s_com.dedic_set.sync_amr,
                            dtx_dsp_interrupt);
                          #endif
    #endif

    // Clear "sync_amr" flag to maintain normal TCH process.
    l1a_l1s_com.dedic_set.sync_amr = FALSE;
  #else
    #if (FF_L1_TCH_VOCODER_CONTROL == 1)
      l1ddsp_load_tch_param(&(l1s.next_time),
                            l1a_l1s_com.dedic_set.aset->achan_ptr->mode,
                            desc_ptr->channel_type,
                            desc_ptr->subchannel,
                            l1a_l1s_com.dedic_set.aset->achan_ptr->tch_loop,
                            l1a_l1s_com.dedic_set.sync_tch,
                            l1a_l1s_com.dedic_set.reset_sacch,
                          #if !FF_L1_IT_DSP_DTX
                            l1a_l1s_com.dedic_set.vocoder_on);
    #else
                            l1a_l1s_com.dedic_set.vocoder_on,
                            dtx_dsp_interrupt);
                          #endif
    #else
      l1ddsp_load_tch_param(&(l1s.next_time),
                            l1a_l1s_com.dedic_set.aset->achan_ptr->mode,
                            desc_ptr->channel_type,
                            desc_ptr->subchannel,
                            l1a_l1s_com.dedic_set.aset->achan_ptr->tch_loop,
                          #if !FF_L1_IT_DSP_DTX
                            l1a_l1s_com.dedic_set.sync_tch);
                          #else
                            l1a_l1s_com.dedic_set.sync_tch,
                            dtx_dsp_interrupt);
                          #endif
    #endif
  #endif

  // reset the FACCH header of the API buffer on the control following an ABORT to avoid decoding unwanted FACCH
  if (l1a_l1s_com.dedic_set.reset_facch == TRUE)
  {
    // Reset A_FD header.
    // B_FIRE1 =1, B_FIRE0 =0 , BLUD =0
    l1s_dsp_com.dsp_ndb_ptr->a_fd[0] = (1<<B_FIRE1);
    l1s_dsp_com.dsp_ndb_ptr->a_fd[2] = 0xffff;
  }

  // Clear "sync_tch" and "reset_sacch" flag to maintain normal TCH process.
  l1a_l1s_com.dedic_set.sync_tch = FALSE;
  l1a_l1s_com.dedic_set.reset_facch = FALSE;
#if (FF_L1_TCH_VOCODER_CONTROL == 1)
  l1a_l1s_com.dedic_set.reset_sacch = FALSE;
#endif

  // Set tpu window identifier for Power meas.
  l1s.tpu_win = (3 * BP_SPLIT) + l1_config.params.tx_nb_load_split + l1_config.params.rx_synth_load_split;

  /*--------------------------------------------*/
  /* Flag DSP and TPU programmation...          */
  /*--------------------------------------------*/

  // Set "CTRL_TX" flag in the controle flag register.
  l1s.tpu_ctrl_reg |= CTRL_TX;
  l1s.dsp_ctrl_reg |= CTRL_TX;
#if FF_L1_IT_DSP_DTX
  } //if (l1a_apihisr_com.dtx.dtx_status != DTX_IT_DSP)
#endif
}

/*-------------------------------------------------------*/
/* l1s_ctrl_tchtf()                                      */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* dedicated mode TCH task: TCHTF. This function is the  */
/* control function for reading the DL burst and sending */
/* the UL burst. The UL burst can be a Normal Burst in   */
/* normal case or an Access Burst when starting a        */
/* Handover procedure. Both Half rate and Full rate TCH  */
/* channel are handled. The DSP and the TPU are          */
/* programmed for both the DL and UL bursts. The timing  */
/* advance is taken into account for positionning the UL */
/* burst.                                                */
/*                                                       */
/* This function accesses the L1/DLL and L1/DATA         */
/* interface ("dll_read_dcch()", "tx_tch_data()"         */
/* functions respectively) and passes then the returned  */
/* data blocks to the DSP.                               */
/*                                                       */
/* Here is a summary of the execution:                   */
/*                                                       */
/*  - Traces and debug.                                  */
/*  - Catch channel description and ARFCN.               */
/*  - TCH/T Receive...                                   */
/*      - Program DSP for RX.                            */
/*      - Program TPU for RX.                            */
/*      - Flag DSP and TPU programmation.                */
/*  - TCH/T Transmit...                                  */
/*      - If Any Handover Access burst to send           */
/*          - Call "l1s_ctrl_rach()".                    */
/*      - Else                                           */
/*          - Get DATA block if required for TCH.        */
/*          - Program DSP for TX.                        */
/*          - Program TPU for TX.                        */
/*          - Flag DSP and TPU programmation.            */
/*  - Common for DL/UL: DSP parameters.                  */
/*                                                       */
/* Input parameters:                                     */
/* -----------------                                     */
/* "task"                                                */
/*   TCHTF, Traffic Channel TCH Full rate.               */
/*                                                       */
/* Input parameters from globals:                        */
/* ------------------------------                        */
/* "l1a_l1s_com.dedic_set"                               */
/*   Dedicated channel parameter structure.              */
/*     .radio_freq, ARFCN value set by the Hopping algo.      */
/*     .aset, active dedicated parameter set.            */
/*                                                       */
/* "l1a_l1s_com.Scell_info"                              */
/*  Serving cell information structure.                  */
/*    .bsic, BSIC of the serving cell. It is used here   */
/*           to pass the training sequence number (part  */
/*           of BSIC) to the DSP.                        */
/*                                                       */
/* "l1s.afc"                                             */
/*   current AFC value to be applied for the given task. */
/*                                                       */
/* "l1s.tpu_offset"                                      */
/*   value for the TPU SYNCHRO and OFFSET registers      */
/*   for current serving cell setting.                   */
/*                                                       */
/* "l1s.applied_txpwr"                                   */
/*   Applied transmit power.                             */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/* "l1s.tpu_win"                                         */
/*   each frame is composed with a maximum of 3          */
/*   working/TPU windows (typically RX/TX/PW). This is   */
/*   a counter used to count the number of windows       */
/*   used.                                               */
/*   -> set to TDMA_WIN3.                                */
/*                                                       */
/* "l1s.tpu_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/TPU com.   */
/*   This is used mainly to swap then the com. page at   */
/*   the end of a control frame.                         */
/*   -> set CTRL_RX bit in the register.                 */
/*   -> set CTRL_TX bit in the register.                 */
/*                                                       */
/* "l1s.dsp_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/DSP com.   */
/*   This is used mainly to swap then the com. page at   */
/*   the end of a control frame.                         */
/*   -> set CTRL_RX bit in the register.                 */
/*   -> set CTRL_TX bit in the register.                 */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_ctrl_tchtf(UWORD8 task, UWORD8 param2)
{
  UWORD16                 radio_freq=0;
  T_CHANNEL_DESCRIPTION  *desc_ptr;
  UWORD8                  lna_off;
  WORD8                   agc;
  T_INPUT_LEVEL          *IL_info_ptr;
  UWORD32                 dsp_task;
  UWORD32                 fn_mod_104;
#if (RF_FAM == 61)
  UWORD16 dco_algo_ctl_nb;
  UWORD8 if_ctl =0 ; //omaps00090550
  UWORD8 if_threshold = C_IF_ZERO_LOW_THRESHOLD_GSM;
  // By default we choose the hardware filter
  UWORD8 csf_filter_choice = L1_SAIC_HARDWARE_FILTER;
#endif
#if FF_L1_IT_DSP_DTX
  BOOL                    dtx_dsp_interrupt = FALSE;
#endif
#if (NEW_SNR_THRESHOLD == 1)
  UWORD8 saic_flag;
#endif
  // Traces and debug.
  // ******************

  #if (TRACE_TYPE==5) && FLOWCHART
    trace_flowchart_dsp_tpu(dltsk_trace[TCHTF].name);
  #endif

  l1s_dsp_com.dsp_db_w_ptr->d_debug = (l1s.debug_time + 2) ;

  // Catch channel description and ARFCN.
  // *************************************

  // Catch the active channel description used along the routine.
  // It contains:
  //    "channel_type", {TCH_F, TCH_H, SDCCH_4, SDCCH_8}.
  //    "subchannel", {0, 1}. 0 is the default value for TCH_F.
  desc_ptr = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr;

  // Get ARFCN to be used for current control. This  ARFCN comes from
  // the HOPPING algorithm called just before calling this function.
  radio_freq = l1a_l1s_com.dedic_set.radio_freq;

  if (radio_freq == l1a_l1s_com.Scell_info.radio_freq)
    IL_info_ptr = &l1a_l1s_com.Scell_info.traffic_meas_beacon;
                       // we are working on a beacon freq.
  else
    IL_info_ptr = &l1a_l1s_com.Scell_info.traffic_meas;
                       // we are working on a daughter freq.

#if FF_L1_IT_DSP_DTX
  // Skip RX DSP/RF programming part during DTX HISR
  if (l1a_apihisr_com.dtx.dtx_status != DTX_IT_DSP)
  {
#endif

  /**************************************************************************/
  /* TCH/T Receive...                                                       */
  /**************************************************************************/
  #if (TRACE_TYPE!=0)
    trace_fct(CST_L1S_CTRL_TCHTF__DL, radio_freq);
  #endif

  /*--------------------------------------------*/
  /* Program DSP...                             */
  /*--------------------------------------------*/
  // dsp pgm.

  dsp_task = l1s_swap_iq_dl(radio_freq,task);

  l1ddsp_load_rx_task(dsp_task, 0, desc_ptr->tsc);

  #if(RF_FAM == 61)  // Locosto DCO
      cust_get_if_dco_ctl_algo(&dco_algo_ctl_nb, &if_ctl, (UWORD8) L1_IL_VALID,
                                         IL_info_ptr->input_level ,
                                          radio_freq,if_threshold);
      l1ddsp_load_dco_ctl_algo_nb(dco_algo_ctl_nb);
  #endif

  #if TESTMODE
    // if Normal Mode or
    // if TestMode DL+UL
    // NOTE: UL only true if DL is true in TCHTF!
    if ( !l1_config.TestMode ||
        (l1_config.TestMode && (l1_config.tmode.rf_params.down_up & TMODE_UPLINK)))
  #endif
    {
      // Set TXPWR.
      l1ddsp_load_txpwr(l1s.applied_txpwr, radio_freq);
    }

  /*--------------------------------------------*/
  /* Program TPU...                             */
  /*--------------------------------------------*/

  // for TCHTF we use DPAGC algorithm.
  #if DPAGC_MAX_FLAG
    agc = Cust_get_agc_from_IL(radio_freq, IL_info_ptr->input_level >> 1, MAX_ID);
  #else
    agc = Cust_get_agc_from_IL(radio_freq, IL_info_ptr->input_level >> 1, AV_ID);
  #endif
  lna_off = IL_info_ptr->lna_off;


  // Store input_level and lna_off fields used for current CTRL in order to be able
  // to build IL from pm in READ phase.
  l1a_l1s_com.Scell_used_IL = *IL_info_ptr;

  #if (L1_SAIC != 0)
    // If SAIC is enabled, call the low level SAIC control function
    csf_filter_choice = l1ctl_saic(l1a_l1s_com.Scell_used_IL.input_level,l1a_l1s_com.mode
  #if (NEW_SNR_THRESHOLD == 1)
        ,task
        ,&saic_flag
  #endif
   );
  #endif

  #if TESTMODE
    // Continuous mode: Rx TPU programmation only in NO_CONTINUOUS or START_RX_CONTINUOUS mode.
    if ((!l1_config.TestMode)                                             ||
        (l1_config.tmode.rf_params.tmode_continuous == TM_NO_CONTINUOUS)        ||
        (l1_config.tmode.rf_params.tmode_continuous == TM_START_RX_CONTINUOUS))
  #endif
    {
      // update the TPU with the new TOA if necessary
      l1ctl_update_TPU_with_toa();

      // Program a serving cell normal burst reading in TPU.
      l1dtpu_serv_rx_nb(radio_freq,
                        agc,
                        lna_off,
                        l1s.tpu_offset,
                        l1s.tpu_offset,
                        FALSE,INACTIVE
                      #if (RF_FAM == 61)
			    ,csf_filter_choice
			    ,if_ctl
                      #endif
          #if (NEW_SNR_THRESHOLD == 1)
          ,saic_flag
          #endif /*NEW_SNR_THRESHOLD*/
			                 );
    }

  // Increment tpu window identifier.
  l1s.tpu_win += (l1_config.params.rx_synth_load_split + RX_LOAD);

  /*--------------------------------------------*/
  /* Flag DSP and TPU programmation...          */
  /*--------------------------------------------*/

  // Set "CTRL_RX" flag in the controle flag register.

  #if TESTMODE
    // Continuous mode: swap TPU page for Rx only in NO_CONTINUOUS or START_RX_CONTINUOUS mode.
    if ((!l1_config.TestMode)                                            ||
        (l1_config.tmode.rf_params.tmode_continuous == TM_NO_CONTINUOUS)       ||
        (l1_config.tmode.rf_params.tmode_continuous == TM_START_RX_CONTINUOUS))
  #endif
    {
  l1s.tpu_ctrl_reg |= CTRL_RX;
    }
  l1s.dsp_ctrl_reg |= CTRL_RX;
#if FF_L1_IT_DSP_DTX
  } // if (l1a_apihisr_com.dtx.dtx_status != DTX_IT_DSP)
#endif

  /**************************************************************************/
  /* TCH/T Transmit...                                                      */
  /**************************************************************************/

  // Any Handover Access burst to send ?
  // ************************************

  if(l1a_l1s_com.dedic_set.aset->ho_acc_to_send != 0)
  // "ho_acc_to_send" is a counter of Handover Access burst still to send.
  // This counter is set by "l1s_dedicated_mode_manager()" in L1S when a
  // Handover command is received from L3 through L1A.
  // We must then replace the TCH UL normal burst by a RACH and decrement
  // this counter.
  {
    if(l1a_l1s_com.dedic_set.aset->ho_acc_to_send != -1)
      l1a_l1s_com.dedic_set.aset->ho_acc_to_send --;
    l1s_ctrl_rach(RAHO,NO_PAR);

    if(l1a_l1s_com.dedic_set.aset->ho_acc_to_send == 0)
    // Handover access procedure is completed.
    // -> send L1C_HANDOVER_FINISHED message with "cause = COMPLETED" to L1A.
    {
      l1s_send_ho_finished(HO_COMPLETE);
    }
  }
  else
  // TCH/UL is a normal burst.
  {
    UWORD8 channel_mode = l1a_l1s_com.dedic_set.aset->achan_ptr->mode;
   //OMAPS00090550 UWORD8 channel_type = desc_ptr->channel_type;

    #if (TRACE_TYPE!=0)
      trace_fct(CST_L1S_CTRL_TCHTF__UL, radio_freq);
    #endif

#if FF_L1_IT_DSP_DTX
  // FACCH and IDS handled during L1S, have to be skipped during DTX HISR
  if (l1a_apihisr_com.dtx.dtx_status != DTX_IT_DSP)
  {
#endif
    /*--------------------------------------------*/
    /* Get DATA block if required for TCH.        */
    /*--------------------------------------------*/
    // Full rate traffic channel...
    {
      UWORD8 fn_report_mod13_mod4 = (l1s.next_time.fn_in_report % 13) % 4;

      if(fn_report_mod13_mod4 == 3)
      // It is time to check if a FACCH/UL data block is available from DLL or
      // if a data block is available from the DATA interface.
      {
        T_RADIO_FRAME  *tx_data = NULL;

        // Check if any FACCH to transmit.
        // Rem: when mode is "SIGNALLING ONLY" the "dll_read_dcch()" function
        // always gives back a block of FACCH (true block or dummy one).
        // In ETM test mode, the protocol stack is not active and hence we do not require any FACCH data from L23
        // But this change is applicable only when ETM scripts are run with PS-builds. In case of L1-SA,
        // dll_read_dcch() is called which is just a stub function (It just returns a NULL ptr for L1 SA)
	/* FreeCalypso: this logic is not present in TCS211 */
	#if 0
        #if TESTMODE
        #if (OP_L1_STANDALONE == 0)
         if(!l1_config.TestMode)
        #endif // (OP_L1_STANDALONE == 0)
        #endif // TESTMODE
	#endif
         {
        tx_data = dll_read_dcch(channel_mode);
         }

        if(tx_data != NULL)
        {
          // In DTX mode in FR and EFR, all 8 FACCH 1/2 bursts must always be transmitted.
          // Note: FACCH presence is checked 1 "control" before "control" of 1st burst of FACCH due to a DSP constraint
          // i.e. 2 bursts before FACCH interleaving boundary
          //So we must wait 1 burst before controlling the transmission of 8 FACCH 1/2 bursts
          l1s.facch_bursts = 9;

          #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
            RTTL1_FILL_UL_DCCH
            trace_info.facch_ul_count ++;
          #endif

          // Store the FACCH/UL data block in the MCU/DSP com.
          #if TRACE_TYPE==3
            if (l1_stats.type == PLAY_UL)
            {
              // load A_DU_1 in PLAY Uplink mode.
              l1ddsp_load_info(DSP_TASK_CODE[task], l1s_dsp_com.dsp_ndb_ptr->a_du_1, &(tx_data->A[0]));


              if (channel_mode == TCH_EFR_MODE)
              {
                WORD32 bit5word14, bit2word14, bit12word15, bit15word15;

                 // clear CRC bits and repetition bits
                l1s_dsp_com.dsp_ndb_ptr->a_du_1[7]  &= 0x807f;
                l1s_dsp_com.dsp_ndb_ptr->a_du_1[14] &= 0xfc24;
                l1s_dsp_com.dsp_ndb_ptr->a_du_1[15] &= 0x93ff;
                l1s_dsp_com.dsp_ndb_ptr->a_du_1[19] &= 0xff00;

                // read repetition bits
                bit5word14  = (l1s_dsp_com.dsp_ndb_ptr->a_du_1[14] >> 5)& 0x1;
                bit2word14  = (l1s_dsp_com.dsp_ndb_ptr->a_du_1[14] >> 2)& 0x1;
                bit12word15  = (l1s_dsp_com.dsp_ndb_ptr->a_du_1[15] >> 12) & 0x1;
                bit15word15 = (l1s_dsp_com.dsp_ndb_ptr->a_du_1[15] >> 15)& 0x1;

                // copy repetition bits
                l1s_dsp_com.dsp_ndb_ptr->a_du_1[14] |=
                         (bit5word14 << 4 | bit5word14 << 3 |
                          bit2word14 | bit2word14 << 1);

                l1s_dsp_com.dsp_ndb_ptr->a_du_1[15] |=
                         (bit15word15 << 13 | bit12word15 << 14 |
                          bit12word15 << 10 | bit15word15 << 11);
              }
              else
              {
                l1s_dsp_com.dsp_ndb_ptr->a_du_1[14] &= 0xfc3f;
                l1s_dsp_com.dsp_ndb_ptr->a_du_1[19] &= 0xff00;
              }

              // set PLAY Uplink bit .......
              l1s_dsp_com.dsp_ndb_ptr->d_tch_mode |= (1 << B_PLAY_UL);
            }
            else
              l1ddsp_load_info(DSP_TASK_CODE[task], l1s_dsp_com.dsp_ndb_ptr->a_fu, &(tx_data->A[0]));
          #else
            l1ddsp_load_info(DSP_TASK_CODE[task], l1s_dsp_com.dsp_ndb_ptr->a_fu, &(tx_data->A[0]));
          #endif
          #if (TRACE_TYPE==5) && FLOWCHART
            trace_flowchart_dsptx(dltsk_trace[TCHTF].name);
          #endif
        }

        #if (AMR == 1)
          // Check if any DATA traffic info frame available.
          // This check is used for all full rate channels except when
          // this channel is in SIGNALLING ONLY mode or in Full Rate
          // Speech mode or adaptative full rate mode.
          if((channel_mode != TCH_FS_MODE)   &&
             (channel_mode != SIG_ONLY_MODE) &&
             (channel_mode != TCH_EFR_MODE)  &&
             (channel_mode != TCH_AFS_MODE))
        #else
          // Check if any DATA traffic info frame available.
          // This check is used for all full rate channels except when
          // this channel is in SIGNALLING ONLY mode or in Full Rate
          // Speech mode.
          if((channel_mode != TCH_FS_MODE)   &&
             (channel_mode != SIG_ONLY_MODE) &&
             (channel_mode != TCH_EFR_MODE))
        #endif
        {
          #if IDS
          {
            UWORD8  fn_report_mod26;
            API *data_ul;

            data_ul = l1s_dsp_com.dsp_ndb_ptr->a_du_0;
            fn_report_mod26 = l1s.next_time.fn_in_report%26;

            // Set flag for UL/DL block information: for TCH/F48 mode only
            if((channel_mode == TCH_48F_MODE) && ((fn_report_mod26 == 7) || (fn_report_mod26 == 16)
                || (fn_report_mod26 == 24)))
              l1s_dsp_com.dsp_ndb_ptr->d_ra_act |= (3 << B_F48BLK);
            else
              l1s_dsp_com.dsp_ndb_ptr->d_ra_act &= ~(3 << B_F48BLK);
            dll_data_ul(l1s_dsp_com.dsp_ndb_ptr->a_data_buf_ul, &l1s_dsp_com.dsp_ndb_ptr->d_ra_conf,
                        &l1s_dsp_com.dsp_ndb_ptr->d_ra_act, &l1s_dsp_com.dsp_ndb_ptr->d_ra_test,
                        &l1s_dsp_com.dsp_ndb_ptr->d_ra_statu, &l1s_dsp_com.dsp_ndb_ptr->d_fax);

            // Fill a_du_0 data block Header.
      // Note: a_du_0 header is fill when dummy block is filled as well when data block
            //       is filled (buffer a_data_buf_ul
            data_ul[0] = (1 << B_BLUD);     // 1st word: Set B_BLU bit.
            data_ul[1] = 0;                 // 2nd word: cleared.
            data_ul[2] = 0;                 // 3rd word: cleared.

          }
          #else
          {
            UWORD8  *tx_data = NULL;

            tx_data = tx_tch_data();
            if(tx_data != NULL)
            {
              // Store the DATA/UL data block in the MCU/DSP com.
              #if TRACE_TYPE==3
                if (l1_stats.type == PLAY_UL)
                {
                  // load A_DU_1 in PLAY Uplink mode.
                  l1ddsp_load_info(DSP_TASK_CODE[task], l1s_dsp_com.dsp_ndb_ptr->a_du_1, tx_data);
                  if (channel_mode == TCH_48F_MODE)
                    l1s_dsp_com.dsp_ndb_ptr->a_du_1[10] &= 0x00ff;
                  if (channel_mode == TCH_24F_MODE)
                    l1s_dsp_com.dsp_ndb_ptr->a_du_1[7]  &= 0x00ff;
                  // set PLAY Uplink bit .......
                  l1s_dsp_com.dsp_ndb_ptr->d_tch_mode |= (1 << B_PLAY_UL);
                }
                else
                  l1ddsp_load_info(DSP_TASK_CODE[task], l1s_dsp_com.dsp_ndb_ptr->a_du_0, tx_data);
              #else
                l1ddsp_load_info(DSP_TASK_CODE[task], l1s_dsp_com.dsp_ndb_ptr->a_du_0, tx_data);
              #endif
            }

            #if (TRACE_TYPE==5) && FLOWCHART
              trace_flowchart_dsptx(dltsk_trace[TCHTF].name);
            #endif
          }
          #endif
        }
#if FEATURE_TCH_REROUTE
        else
          tch_substitute_uplink(l1s_dsp_com.dsp_ndb_ptr->a_du_1);
#endif
      }
    }

#if FF_L1_IT_DSP_DTX
    // Fast DTX active only in TCH AFS, for TDMA3 from speech block = 0, 1 [MOD 3]
    if ((l1a_l1s_com.dedic_set.aset->achan_ptr->mode == TCH_AFS_MODE)
        && (l1a_l1s_com.dedic_set.aset->dtx_allowed == TRUE))
    {
      // DTX interrupt request for B1 and B2 (no DTX uncertainty on B0 thanks to idle frame)
      if (l1s.next_time.fn_mod13 <= 7)
        dtx_dsp_interrupt = TRUE;

      // DTX uncertainty check
      if  ((l1a_apihisr_com.dtx.fast_dtx_ready == TRUE) &&                   // Fast DTX can be used
           ((l1s.next_time.fn_mod13 == 4) || (l1s.next_time.fn_mod13 == 8))) // new block boundary
        l1a_apihisr_com.dtx.dtx_status = DTX_AWAITED;
    }
    else
      l1a_apihisr_com.dtx.dtx_status = DTX_AVAILABLE;
  } // if (l1a_apihisr_com.dtx.dtx_status != DTX_IT_DSP)

  // Postpone TPU/DSP programming when DTX status not available from DSP
  if (l1a_apihisr_com.dtx.dtx_status != DTX_AWAITED)
  {
    BOOL tx_active =FALSE; //omaps00090550
#endif
    #if TESTMODE
      // if Normal Mode or
      // if TestMode and UL+DL
      // NOTE: UL only true if DL is true in TCHTF!
      if ( !l1_config.TestMode ||
          (l1_config.TestMode && (l1_config.tmode.rf_params.down_up & TMODE_UPLINK)))
    #endif
      {
        /*--------------------------------------------*/
        /* Program DSP...                             */
        /*--------------------------------------------*/

        dsp_task = l1s_swap_iq_ul(radio_freq,task);

        l1ddsp_load_tx_task(dsp_task, 0, desc_ptr->tsc);

        /*--------------------------------------------*/
        /* Program TPU...                             */
        /*--------------------------------------------*/

        fn_mod_104   = l1s.actual_time.fn % 104;

        #if TESTMODE
          if ((!l1_config.TestMode)                                      ||
              (l1_config.tmode.rf_params.tmode_continuous == TM_NO_CONTINUOUS) ||
              (l1_config.tmode.rf_params.tmode_continuous == TM_START_TX_CONTINUOUS))
        #endif
          {
            l1s.facch_bursts--;
	    if (l1s.facch_bursts < 0)
	      l1s.facch_bursts = -1;

      #if FF_L1_IT_DSP_DTX
        // Condition for TX TPU programming channel mode dependant
        switch (channel_mode)
        {
          case SIG_ONLY_MODE:
          case TCH_24F_MODE:
          case TCH_48F_MODE:
          case TCH_96_MODE:
          case TCH_144_MODE:
            // DTX not supported
            tx_active = TRUE;
            break;

          case TCH_FS_MODE:
          case TCH_EFR_MODE:
            if ((l1s.dtx_ul_on == FALSE) ||                                // No DTX
                ((l1s.facch_bursts >= 0) && (l1s.facch_bursts <= 7)) || // FACCH in progress
                ((fn_mod_104 >= 51) && (fn_mod_104 <= 58))              // SID
               )
              tx_active = TRUE;
            else
              tx_active = FALSE;
            break;

          case TCH_AFS_MODE:
            if (l1a_apihisr_com.dtx.tx_active) // DSP (Fast) DTX status
	    tx_active = TRUE;
            else
	    tx_active = FALSE;
            break;
        }

        // TPU TX burst programming
        if (tx_active)
      #else
            // In DTX mode, UL bursts should not be transmitted when no voice activity is detected
	    // we must not call TPU scenario if dtx_on == TRUE in EFR and FR (See Technical Memo)
	    // However, in DTX mode, bursts 52 to 59 (modulo 104) must always be transmitted
	    // FACCH must also be transmitted but we must wait 1 bursts before transmitting 8 1/2 bursts
	    if ( ((channel_mode != TCH_FS_MODE) && (channel_mode != TCH_EFR_MODE)) ||
                 (l1s.dtx_ul_on == FALSE) ||
                 ( (l1s.facch_bursts >= 0) && (l1s.facch_bursts <= 7) ) ||
		 ((fn_mod_104 >= 51) && (fn_mod_104 <= 58)) )
      #endif
            {
             l1dtpu_serv_tx_nb(radio_freq, l1a_l1s_com.dedic_set.aset->timing_advance, l1s.tpu_offset, l1s.applied_txpwr,INACTIVE);
            }
          }

        #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
          RTTL1_FILL_UL_NB(task, l1a_l1s_com.dedic_set.aset->timing_advance, l1s.applied_txpwr)
        #endif

    }
  #if FF_L1_IT_DSP_DTX
    } // if (l1a_apihisr_com.dtx.dtx_status != DTX_AWAITED)
  #endif
  } // TCH/UL is a normal burst.
#if FF_L1_IT_DSP_DTX
  // Postpone TPU/DSP programming when DTX status not available from DSP
  if (l1a_apihisr_com.dtx.dtx_status != DTX_IT_DSP)
  {
#endif

  /*----------------------------------------------*/
  /* Common for Dedicated mode: DSP parameters... */
  /*----------------------------------------------*/
  #if (AMR == 1)
    #if (FF_L1_TCH_VOCODER_CONTROL == 1)
      l1ddsp_load_tch_param(&(l1s.next_time),
                            l1a_l1s_com.dedic_set.aset->achan_ptr->mode,
                            desc_ptr->channel_type,
                            0,
                            l1a_l1s_com.dedic_set.aset->achan_ptr->tch_loop,
                            l1a_l1s_com.dedic_set.sync_tch,
                            l1a_l1s_com.dedic_set.sync_amr,
                            l1a_l1s_com.dedic_set.reset_sacch,
                          #if !FF_L1_IT_DSP_DTX
                            l1a_l1s_com.dedic_set.vocoder_on);
    #else
                            l1a_l1s_com.dedic_set.vocoder_on,
                            dtx_dsp_interrupt);
                          #endif
    #else
      l1ddsp_load_tch_param(&(l1s.next_time),
                            l1a_l1s_com.dedic_set.aset->achan_ptr->mode,
                            desc_ptr->channel_type,
                            0,
                            l1a_l1s_com.dedic_set.aset->achan_ptr->tch_loop,
                            l1a_l1s_com.dedic_set.sync_tch,
                          #if !FF_L1_IT_DSP_DTX
                            l1a_l1s_com.dedic_set.sync_amr);
                          #else
                            l1a_l1s_com.dedic_set.sync_amr,
                            dtx_dsp_interrupt);
                          #endif
    #endif

    // Clear "sync_amr" flag to maintain normal TCH process.
    l1a_l1s_com.dedic_set.sync_amr = FALSE;
  #else
    #if (FF_L1_TCH_VOCODER_CONTROL == 1)
      l1ddsp_load_tch_param(&(l1s.next_time),
                            l1a_l1s_com.dedic_set.aset->achan_ptr->mode,
                            desc_ptr->channel_type,
                            0,
                            l1a_l1s_com.dedic_set.aset->achan_ptr->tch_loop,
                            l1a_l1s_com.dedic_set.sync_tch,
                            l1a_l1s_com.dedic_set.reset_sacch,
                          #if !FF_L1_IT_DSP_DTX
                            l1a_l1s_com.dedic_set.vocoder_on);
    #else
                            l1a_l1s_com.dedic_set.vocoder_on,
                            dtx_dsp_interrupt);
                          #endif
    #else
      l1ddsp_load_tch_param(&(l1s.next_time),
                            l1a_l1s_com.dedic_set.aset->achan_ptr->mode,
                            desc_ptr->channel_type,
                            0,
                            l1a_l1s_com.dedic_set.aset->achan_ptr->tch_loop,
                          #if !FF_L1_IT_DSP_DTX
                            l1a_l1s_com.dedic_set.sync_tch);
                          #else
                            l1a_l1s_com.dedic_set.sync_tch,
                            dtx_dsp_interrupt);
                          #endif
    #endif
  #endif

  // reset the FACCH header of the API buffer on the control following an ABORT to avoid decoding unwanted FACCH
  if (l1a_l1s_com.dedic_set.reset_facch == TRUE)
  {
    // Reset A_FD header.
    // B_FIRE1 =1, B_FIRE0 =0 , BLUD =0
    l1s_dsp_com.dsp_ndb_ptr->a_fd[0] = (1<<B_FIRE1);
    l1s_dsp_com.dsp_ndb_ptr->a_fd[2] = 0xffff;
  }

  // Clear "sync_tch" and "reset_sacch" flag to maintain normal TCH process.
  l1a_l1s_com.dedic_set.sync_tch = FALSE;
  l1a_l1s_com.dedic_set.reset_facch = FALSE;
#if (FF_L1_TCH_VOCODER_CONTROL == 1)
  l1a_l1s_com.dedic_set.reset_sacch = FALSE;
#endif

  // Set tpu window identifier for Power meas or FS/SB search.
  l1s.tpu_win = (3 * BP_SPLIT) + l1_config.params.tx_nb_load_split + l1_config.params.rx_synth_load_split;

  /*--------------------------------------------*/
  /* Flag DSP and TPU programmation...          */
  /*--------------------------------------------*/

  #if TESTMODE
    // if Normal Mode or
    // if TestMode and UL+DL
    // NOTE: UL only true if DL is true in TCHTF!
    if ( !l1_config.TestMode ||
       (  l1_config.TestMode && (l1_config.tmode.rf_params.down_up & TMODE_UPLINK)))
  #endif
    {
      #if TESTMODE
        // Continuous mode: swap TPU page for Tx in NO_CONTINUOUS or START_TX_CONTINUOUS mode.
        if ((!l1_config.TestMode)                                      ||
            (l1_config.tmode.rf_params.tmode_continuous == TM_NO_CONTINUOUS) ||
            (l1_config.tmode.rf_params.tmode_continuous == TM_START_TX_CONTINUOUS))
      #endif
        {
          l1s.tpu_ctrl_reg |= CTRL_TX;
        }
      l1s.dsp_ctrl_reg |= CTRL_TX;
    }

  #if TESTMODE
    // Continuous mode: if end of control of START_RX/TX: go to CONTINUOUS state
    if (l1_config.TestMode && (l1_config.tmode.rf_params.tmode_continuous != TM_NO_CONTINUOUS))
      l1_config.tmode.rf_params.tmode_continuous = TM_CONTINUOUS;
  #endif
#if FF_L1_IT_DSP_DTX
  } //if (l1a_apihisr_com.dtx.dtx_status != DTX_IT_DSP)
  #endif
}

/*-------------------------------------------------------*/
/* l1s_ctrl_tcha()                                       */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* dedicated mode TCH task: TCHA. This function is the   */
/* control function for reading the DL burst and sending */
/* the UL burst on the Slow Associated Channel (SACCH)   */
/* associated with the traffic channel. The UL burst can */
/* be a Normal Burst in normal case or an Access Burst   */
/* when starting a Handover procedure. Both Half rate    */
/* and Full rate TCH channel are handled. The DSP and    */
/* the TPU are programmed for both the DL and UL bursts. */
/* The timing advance is taken into account for          */
/* positionning the UL burst.                            */
/*                                                       */
/* This function accesses the L1/DLL interface           */
/* ("dll_read_sacch()" function) and passes then the     */
/* returned data blocks to the DSP after having set the  */
/* L1 header part of the block.                          */
/*                                                       */
/* Here is a summary of the execution:                   */
/*                                                       */
/*  - Traces and debug.                                  */
/*  - Catch channel description and ARFCN.               */
/*  - TCH/SACCH Receive...                               */
/*      - Program DSP for RX.                            */
/*      - Program TPU for RX.                            */
/*      - Flag DSP and TPU programmation.                */
/*  - TCH/SACCH Transmit...                              */
/*      - If Any Handover Access burst to send           */
/*          - Call "l1s_ctrl_rach()".                    */
/*      - Else                                           */
/*          - Get DATA block from DLL if required.       */
/*          - Program DSP for TX.                        */
/*          - Program TPU for TX.                        */
/*          - Flag DSP and TPU programmation.            */
/*  - Common for DL/UL: DSP parameters.                  */
/*                                                       */
/* Input parameters:                                     */
/* -----------------                                     */
/* "task"                                                */
/*   TCHA, Associated channel task when dedicated/TCH.   */
/*                                                       */
/* Input parameters from globals:                        */
/* ------------------------------                        */
/* "l1a_l1s_com.dedic_set"                               */
/*   Dedicated channel parameter structure.              */
/*     .radio_freq, ARFCN value set by the Hopping algo.      */
/*     .aset, active dedicated parameter set.            */
/*                                                       */
/* "l1a_l1s_com.Scell_info"                              */
/*  Serving cell information structure.                  */
/*    .bsic, BSIC of the serving cell. It is used here   */
/*           to pass the training sequence number (part  */
/*           of BSIC) to the DSP.                        */
/*                                                       */
/* "l1s.afc"                                             */
/*   current AFC value to be applied for the given task. */
/*                                                       */
/* "l1s.tpu_offset"                                      */
/*   value for the TPU SYNCHRO and OFFSET registers      */
/*   for current serving cell setting.                   */
/*                                                       */
/* "l1s.applied_txpwr"                                   */
/*   Applied transmit power.                             */
/*                                                       */
/* "l1s.reported_txpwr"                                  */
/*   Transmit power to report in the L1 header of the    */
/*   SACCH data block.                                   */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/* "l1s.tpu_win"                                         */
/*   each frame is composed with a maximum of 3          */
/*   working/TPU windows (typically RX/TX/PW). This is   */
/*   a counter used to count the number of windows       */
/*   used.                                               */
/*   -> set to TDMA_WIN3.                                */
/*                                                       */
/* "l1s.tpu_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/TPU com.   */
/*   This is used mainly to swap then the com. page at   */
/*   the end of a control frame.                         */
/*   -> set CTRL_RX bit in the register.                 */
/*   -> set CTRL_TX bit in the register.                 */
/*                                                       */
/* "l1s.dsp_ctrl_reg"                                    */
/*   bit register used to know at the end of L1S if      */
/*   something has been programmed on the MCU/DSP com.   */
/*   This is used mainly to swap then the com. page at   */
/*   the end of a control frame.                         */
/*   -> set CTRL_RX bit in the register.                 */
/*   -> set CTRL_TX bit in the register.                 */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_ctrl_tcha(UWORD8 task, UWORD8 param2)
{
  UWORD16                radio_freq=0;
  T_CHANNEL_DESCRIPTION *desc_ptr;
  UWORD8                 lna_off =0;//omaps00090550
  WORD8                  agc =0; //omaps00090550
  T_INPUT_LEVEL         *IL_info_ptr;
  UWORD32                dsp_task;
  UWORD8                 adc_active_ul = INACTIVE;
  UWORD8                 adc_active_dl = INACTIVE;
#if (RF_FAM == 61)
  UWORD16 dco_algo_ctl_nb = 0;
  UWORD8 if_ctl = 0;
  UWORD8 if_threshold = C_IF_ZERO_LOW_THRESHOLD_GSM;
  // By default we choose the hardware filter
  UWORD8 csf_filter_choice = L1_SAIC_HARDWARE_FILTER;
#endif
#if (NEW_SNR_THRESHOLD == 1)
  UWORD8 saic_flag=0;
#endif /*NEW_SNR_THRESHOLD */
  // Traces and debug.
  // ******************

  #if (TRACE_TYPE==5) && FLOWCHART
    trace_flowchart_dsp_tpu(dltsk_trace[TCHA].name);
  #endif

  l1s_dsp_com.dsp_db_w_ptr->d_debug = (l1s.debug_time + 2) ;

  // Catch channel description and ARFCN.
  // *************************************

  // Catch the active channel description used along the routine.
  // It contains:
  //    "channel_type", {TCH_F, TCH_H, SDCCH_4, SDCCH_8}.
  //    "subchannel", {0, 1}. 0 is the default value for TCH_F.
  desc_ptr = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr;

  // Get ARFCN to be used for current control.
  radio_freq = l1a_l1s_com.dedic_set.radio_freq;

  if (radio_freq == l1a_l1s_com.Scell_info.radio_freq)
    IL_info_ptr = &l1a_l1s_com.Scell_info.traffic_meas_beacon;
                          // we are working on a beacon freq.
  else
    IL_info_ptr = &l1a_l1s_com.Scell_info.traffic_meas;
                          // we are working on a daughter freq.

  /**************************************************************************/
  /* SACCH Receive...                                                       */
  /**************************************************************************/

  // ADC measurement
  // ***************

  // check if during the SACCH burst an ADC measurement must be performed
   if (l1a_l1s_com.adc_mode & ADC_NEXT_TRAFFIC_DL)  // perform ADC only one time
   {
      adc_active_dl = ACTIVE;
      l1a_l1s_com.adc_mode &= ADC_MASK_RESET_TRAFFIC; // reset in order to have only one ADC measurement in Traffic
   }
   else
     if (l1a_l1s_com.adc_mode & ADC_EACH_TRAFFIC_DL) // perform ADC on each period bloc
       if (l1s.next_time.fn_in_report == 12) //periodic with each 1st SACCH burst
         if ((++l1a_l1s_com.adc_cpt)>=l1a_l1s_com.adc_traffic_period) // wait for the period
         {
           adc_active_dl = ACTIVE;
           l1a_l1s_com.adc_cpt = 0;
         }

  #if TESTMODE
    // if Normal Mode or
    // if TestMode and DL-only or DL+UL
    if ( !l1_config.TestMode ||
         (l1_config.TestMode && (l1_config.tmode.rf_params.down_up & TMODE_DOWNLINK)))
  #endif
    {
      #if (TRACE_TYPE!=0)
        trace_fct(CST_L1S_CTRL_TCHA___DL, radio_freq);
      #endif

      /*--------------------------------------------*/
      /* Program DSP...                             */
      /*--------------------------------------------*/
      // dsp pgm.

      dsp_task = l1s_swap_iq_dl(radio_freq,task);
      l1ddsp_load_rx_task(dsp_task, 0, desc_ptr->tsc);

      #if(RF_FAM == 61)    // Locosto DCO
         cust_get_if_dco_ctl_algo(&dco_algo_ctl_nb, &if_ctl, (UWORD8) L1_IL_VALID ,
                                         IL_info_ptr->input_level ,
                                          radio_freq,if_threshold);
   	l1ddsp_load_dco_ctl_algo_nb(dco_algo_ctl_nb);
      #endif

      /*--------------------------------------------*/
      /* Program TPU...                             */
      /*--------------------------------------------*/
      // for TCHA we use DPAGC algorithm.
      #if DPAGC_MAX_FLAG
        agc = Cust_get_agc_from_IL(radio_freq, IL_info_ptr->input_level >> 1, MAX_ID);
      #else
        agc = Cust_get_agc_from_IL(radio_freq, IL_info_ptr->input_level >> 1, AV_ID);
      #endif
      lna_off = IL_info_ptr->lna_off;



      // Store input_level and lna_off fields used for current CTRL in order to be able
      // to build IL from pm in READ phase.
      l1a_l1s_com.Scell_used_IL = *IL_info_ptr;

      #if (L1_SAIC != 0)
        // If SAIC is enabled, call the low level SAIC control function
        csf_filter_choice = l1ctl_saic(l1a_l1s_com.Scell_used_IL.input_level,l1a_l1s_com.mode
        #if (NEW_SNR_THRESHOLD == 1)
            ,task
            ,&saic_flag
        #endif
        );
      #endif

      #if TESTMODE
        // Continuous mode: Rx TPU programmation only in NO_CONTINUOUS or START_RX_CONTINUOUS mode.
        if ((!l1_config.TestMode)                                           ||
            (l1_config.tmode.rf_params.tmode_continuous == TM_NO_CONTINUOUS)      ||
            (l1_config.tmode.rf_params.tmode_continuous == TM_START_RX_CONTINUOUS))
      #endif
        {
          // update the TPU with the new TOA if necessary
          l1ctl_update_TPU_with_toa();

          // Program a serving cell normal burst reading in TPU.
          l1dtpu_serv_rx_nb(radio_freq,
                            agc,
                            lna_off,
                            l1s.tpu_offset,
                            l1s.tpu_offset,
                            FALSE,adc_active_dl
                          #if (RF_FAM == 61)
				,csf_filter_choice
				,if_ctl
			  #endif
                          #if (NEW_SNR_THRESHOLD == 1)
                            ,saic_flag
                          #endif /* NEW_SNR_THRESHOLD */
					 );
        }

  // Increment tpu window identifier.
  l1s.tpu_win += (l1_config.params.rx_synth_load_split + RX_LOAD);

  // Set "CTRL_RX" flag in the control flag register.
  #if TESTMODE
    // Continuous mode: swap TPU page for Rx only in NO_CONTINUOUS or START_RX_CONTINUOUS mode.
    if ((!l1_config.TestMode)                                           ||
        (l1_config.tmode.rf_params.tmode_continuous == TM_NO_CONTINUOUS)      ||
        (l1_config.tmode.rf_params.tmode_continuous == TM_START_RX_CONTINUOUS))
  #endif
    {
      l1s.tpu_ctrl_reg |= CTRL_RX;
    }
    l1s.dsp_ctrl_reg |= CTRL_RX;

  }

  /**************************************************************************/
  /* TCH/T Transmit...                                                      */
  /**************************************************************************/

  // Any Handover Access burst to send ? --> TXPWR management
  // ************************************

  if(l1a_l1s_com.dedic_set.aset->ho_acc_to_send != 0)
  // "ho_acc_to_send" is a counter of Handover Access burst still to send.
  // This counter is set by "l1s_dedicated_mode_manager()" in L1S when a
  // Handover command is received from L3 through L1A.
  // When Handover access is in progress, nothing but RACH can be transmitted.
  // RACH is not allowed on SACCH therefore TX is avoided by setting
  // the txpwr to NO_TXPWR !!!
  {
#if 0	/* LoCosto code */
    // NOTE: The spec says RACH bursts on SACCH UL is optional. hence it should not be counted
    // Refer spec 04.08
    l1s_ctrl_rach(RAHO,NO_PAR);
#else	/* TCS211 reconstruction, code taken from TSM30 */
    // Set TXPWR.
    l1ddsp_load_txpwr(NO_TXPWR, radio_freq);
 
    #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
      RTTL1_FILL_UL_NB(task, l1a_l1s_com.dedic_set.aset->timing_advance, NO_TXPWR)
    #endif
#endif
  }
  else
  // TCH/UL is a normal burst.
  // TX power must be the normal one
  {
    // Set TXPWR.
    l1ddsp_load_txpwr(l1s.applied_txpwr, radio_freq);

    #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
      RTTL1_FILL_UL_NB(task, l1a_l1s_com.dedic_set.aset->timing_advance, l1s.applied_txpwr)
    #endif

    // ADC measurement
    // ***************

    // check if during the SACCH burst an ADC measurement must be performed
    if (l1a_l1s_com.adc_mode & ADC_NEXT_TRAFFIC_UL)  // perform ADC only one time
    {
       adc_active_ul = ACTIVE;
       l1a_l1s_com.adc_mode &= ADC_MASK_RESET_TRAFFIC; // reset in order to have only one ADC measurement in Traffic
    }
    else
      if (l1a_l1s_com.adc_mode & ADC_EACH_TRAFFIC_UL) // perform ADC on each period bloc
        if (l1s.next_time.fn_in_report == 12) //periodic with each 1st SACCH burst
          if ((++l1a_l1s_com.adc_cpt)>=l1a_l1s_com.adc_traffic_period) // wait for the period
          {
            adc_active_ul = ACTIVE;
            l1a_l1s_com.adc_cpt = 0;
          }

#if 1	/* FreeCalypso TCS211 reconstruction */
  } // End of "TCH/UL is a normal burst"
#endif

    // In any case (normal TX or no TX due to Handover Access process)
    // the full TCHA task must be controled for TPU and DSP.
    {
      T_RADIO_FRAME   *tx_data = NULL;

      #if (TRACE_TYPE!=0)
        trace_fct(CST_L1S_CTRL_TCHA___UL, radio_freq);
      #endif

      /*--------------------------------------------*/
      /* Get DATA block if required for SACCH.      */
      /*--------------------------------------------*/
      if(l1s.next_time.fn_in_report == 12)
      // It is time to get a SACCH data block from DLL.
      // Call "dll_read_sacch()" to perform "PH_DATA_REQ" and pass
      // the data block to the DSP..
      {
	  #if ((FF_REPEATED_SACCH)     && ( TESTMODE))
          if(l1_config.repeat_sacch_enable != REPEATED_SACCH_ENABLE)
        {
               l1s.repeated_sacch.sro = 0;
               l1s.repeated_sacch.buffer_empty = TRUE;
          }
      #endif /* #if ((FF_REPEATED_SACCH)     && ( TESTMODE)) */
#if FF_REPEATED_SACCH
        /* Get data from PS if only no repetition order is required (1st condition) */
        /* or no repetition candidate exists (2nd condition)                        */
     if((l1s.repeated_sacch.sro == 0)  ||  (l1s.repeated_sacch.buffer_empty == TRUE))
#endif /* FF_REPEATED_SACCH */
     {
        tx_data = dll_read_sacch(SIG_ONLY_MODE);
        if(tx_data != NULL)
        {
          // Set L1 Header...
          tx_data->A[0] = l1s.reported_txpwr;
          tx_data->A[1] = l1a_l1s_com.dedic_set.aset->timing_advance;

        #if FF_REPEATED_SACCH
                /* Include the SACCH Repetition Request (SRR) in the L1 Header */
           tx_data->A[0] |= (l1s.repeated_sacch.srr <<6);
        #endif  /* FF_REPEATED_SACCH */
          // Put data block in MCU/DSP com.
          l1ddsp_load_info(DSP_TASK_CODE[task], l1s_dsp_com.dsp_ndb_ptr->a_cu, &(tx_data->A[0]));
          #if (FF_REPEATED_SACCH == 1 )
             if(((tx_data->A[2]&0x1C) >> 2) == SAPI_0)    /* Store the block data in case of a retransmission order */
            {
                 l1s_store_sacch_buffer( &(l1s.repeated_sacch), &(tx_data->A[0]));
                 /* Stores the buffer and turns of the buffer_empty flag as false */
             }
            else
            {
            /* the SACCH repetition block occurrence will always come as a consecutive pair   */
            /* To handle DL UL | DL  UL  | DL UL                                              */
            /*            -  0 | SRO  3  | -   new data should be asked from PS old 0 cannot be repeated */
                l1s.repeated_sacch.buffer_empty=TRUE;
            }
         #endif /* FF_REPEATED_SACCH */
         } /* if(tx_data != NULL) */

          #if (TRACE_TYPE==5) && FLOWCHART
            trace_flowchart_dsptx(dltsk_trace[TCHA].name);
          #endif
        }/* if((l1s.repeated_sacch.sro == 0)  ||  (l1s.repeated_sacch.buffer_empty == TRUE))*/
      #if FF_REPEATED_SACCH
	       else if ((l1s.repeated_sacch.sro == 1) && (l1s.repeated_sacch.buffer_empty == FALSE))
	       {
	            /* Put data block in MCU/DSP com. */
	            l1ddsp_load_info(DSP_TASK_CODE[task], l1s_dsp_com.dsp_ndb_ptr->a_cu, l1s.repeated_sacch.buffer );
	            l1s.repeated_sacch.buffer_empty = TRUE;     /* Set that the buffer is now empty (only one repetition) */
	       } /* end else repetition */
	  #endif /* FF_REPEATED_SACCH */
        // check to be removed
      }

      /*--------------------------------------------*/
      /* Program DSP...                             */
      /*--------------------------------------------*/
      #if TESTMODE
        // UL-only...
        // Use SPI to write to Omega uplink buffer, do NOT use DSP
        if (l1_config.TestMode && l1_config.tmode.rf_params.down_up == TMODE_UPLINK)
          {
          #if (CODE_VERSION != SIMULATION)
            // For Calyso+ & Before...
            #if (RF_FAM == 12)
            ABB_Write_Register_on_page(PAGE0, AUXAPC, Cust_get_pwr_data(l1s.applied_txpwr, radio_freq
            																			   #if(REL99 && FF_PRF)
            																			   ,1
            																			   #endif
            																			   ));
            l1tm_fill_burst(l1_config.tmode.tx_params.burst_data, &TM_ul_data[0]);
            ABB_Write_Uplink_Data(&TM_ul_data[0]);
          #endif

            //For  UppCosto, Tx Data Write is via PLD to DRP & Ramp is via the ABB Driver
       #if (RF_FAM == 60)
         ABB_Write_Register_on_page(PAGE0, AUXAPC, Cust_get_pwr_data(l1s.applied_txpwr, radio_freq
																						#if(REL99 && FF_PRF)
																						,1
																						#endif
																						));
              l1tm_fill_burst(l1_config.tmode.tx_params.burst_data, &TM_ul_data[0]);
              PLD_Write_Uplink_Data(&TM_ul_data[0]);
            #endif

           #if (RF_FAM == 61)
              // For DRP we use the DSP to write the TX Power via a new variable apclev in API
        // A new variable is required in API as DSP copies the tx_power_ctl (which is
        // normally used to pass the APCLEV value to DSP) to APCLEV ONLY when there is a
        // burst to be transmitted
        l1ddsp_apc_load_apclev(Cust_get_pwr_data(l1s.applied_txpwr, radio_freq
        															#if(REL99 && FF_PRF)
        															,1
        															#endif
                                                                    ));
              l1tm_fill_burst(l1_config.tmode.tx_params.burst_data, &TM_ul_data[0]);
              DRP_Write_Uplink_Data(&TM_ul_data[0]);
            #endif

    #endif
          }
        // Use DSP...
        // if Normal Mode or
        // if TestMode and DL+UL
        else if ( !l1_config.TestMode ||
                (l1_config.TestMode && l1_config.tmode.rf_params.down_up == (TMODE_DOWNLINK|TMODE_UPLINK)))
          {
            dsp_task = l1s_swap_iq_ul(radio_freq,task);

            l1ddsp_load_tx_task(dsp_task, 0, desc_ptr->tsc);
          }
      #else
        dsp_task = l1s_swap_iq_ul(radio_freq,task);

        l1ddsp_load_tx_task(dsp_task, 0, desc_ptr->tsc);
      #endif

        /*--------------------------------------------*/
        /* Program TPU...                             */
        /*--------------------------------------------*/
      #if TESTMODE
        // if Normal Mode or
        // if TestMode and UL-only or DL+UL
        if ( !l1_config.TestMode ||
           (l1_config.TestMode && (l1_config.tmode.rf_params.down_up & TMODE_UPLINK) &&
           (l1_config.tmode.rf_params.tmode_continuous == TM_NO_CONTINUOUS           ||
            l1_config.tmode.rf_params.tmode_continuous == TM_START_TX_CONTINUOUS)))
      #endif
            {
               l1dtpu_serv_tx_nb(radio_freq, l1a_l1s_com.dedic_set.aset->timing_advance, l1s.tpu_offset, l1s.applied_txpwr, adc_active_ul);
            }
    }

#if 0	/* FreeCalypso TCS211 reconstruction */
  } // End of "TCH/UL is a normal burst"
#endif

  /*----------------------------------------------*/
  /* Common for Dedicated mode: DSP parameters... */
  /*----------------------------------------------*/

  #if (AMR == 1)
    #if (FF_L1_TCH_VOCODER_CONTROL == 1)
      l1ddsp_load_tch_param(&(l1s.next_time),
                            l1a_l1s_com.dedic_set.aset->achan_ptr->mode,
                            desc_ptr->channel_type,
                            desc_ptr->subchannel,
                            l1a_l1s_com.dedic_set.aset->achan_ptr->tch_loop,
                            l1a_l1s_com.dedic_set.sync_tch,
                            l1a_l1s_com.dedic_set.sync_amr,
                            l1a_l1s_com.dedic_set.reset_sacch,
                          #if !FF_L1_IT_DSP_DTX
                            l1a_l1s_com.dedic_set.vocoder_on);
                          #else
                            l1a_l1s_com.dedic_set.vocoder_on,
                            0);
                          #endif
    #else // FF_L1_TCH_VOCODER_CONTROL
      l1ddsp_load_tch_param(&(l1s.next_time),
                            l1a_l1s_com.dedic_set.aset->achan_ptr->mode,
                            desc_ptr->channel_type,
                            desc_ptr->subchannel,
                            l1a_l1s_com.dedic_set.aset->achan_ptr->tch_loop,
                            l1a_l1s_com.dedic_set.sync_tch,
                          #if !FF_L1_IT_DSP_DTX
                            l1a_l1s_com.dedic_set.sync_amr);
                          #else
                            l1a_l1s_com.dedic_set.sync_amr,
                            0);
                          #endif
    #endif // FF_L1_TCH_VOCODER_CONTROL

    l1a_l1s_com.dedic_set.sync_amr    = FALSE;

  #else // AMR
    #if (FF_L1_TCH_VOCODER_CONTROL == 1)
      l1ddsp_load_tch_param(&(l1s.next_time),
                            l1a_l1s_com.dedic_set.aset->achan_ptr->mode,
                            desc_ptr->channel_type,
                            desc_ptr->subchannel,
                            l1a_l1s_com.dedic_set.aset->achan_ptr->tch_loop,
                            l1a_l1s_com.dedic_set.sync_tch,
                            l1a_l1s_com.dedic_set.reset_sacch,
                          #if !FF_L1_IT_DSP_DTX
                            l1a_l1s_com.dedic_set.vocoder_on);
    #else
                            l1a_l1s_com.dedic_set.vocoder_on,
                            0);
                          #endif
    #else
      l1ddsp_load_tch_param(&(l1s.next_time),
                            l1a_l1s_com.dedic_set.aset->achan_ptr->mode,
                            desc_ptr->channel_type,
                            desc_ptr->subchannel,
                            l1a_l1s_com.dedic_set.aset->achan_ptr->tch_loop,
                          #if !FF_L1_IT_DSP_DTX
                            l1a_l1s_com.dedic_set.sync_tch);
                          #else
                            l1a_l1s_com.dedic_set.sync_tch,
                            0);
                          #endif
    #endif
  #endif // AMR

  // Clear "sync_tch" flag to maintain normal TCH process.
  l1a_l1s_com.dedic_set.sync_tch = FALSE;
#if (FF_L1_TCH_VOCODER_CONTROL)
  l1a_l1s_com.dedic_set.reset_sacch = FALSE;
#endif

#if 0	/* FreeCalypso TCS211 reconstruction */
  if(l1a_l1s_com.dedic_set.aset->ho_acc_to_send == 0)
  {
#endif
    // Set tpu window identifier for Power meas or FS/SB search.
    l1s.tpu_win = (3 * BP_SPLIT) + l1_config.params.tx_nb_load_split + l1_config.params.rx_synth_load_split;

    /*--------------------------------------------*/
    /* Flag DSP and TPU programmation...          */
    /*--------------------------------------------*/
    #if TESTMODE
      // if Normal Mode or
      // if TestMode and UL-only or DL+UL
      if ( !l1_config.TestMode ||
         (l1_config.TestMode && (l1_config.tmode.rf_params.down_up & TMODE_UPLINK)))
    #endif
      {
        #if TESTMODE
          // Continuous mode: swap TPU page for Tx in NO_CONTINUOUS or START_TX_CONTINUOUS mode.
          if ((!l1_config.TestMode)                                           ||
              (l1_config.tmode.rf_params.tmode_continuous == TM_NO_CONTINUOUS)      ||
              (l1_config.tmode.rf_params.tmode_continuous == TM_START_TX_CONTINUOUS))
        #endif
          {
            l1s.tpu_ctrl_reg |= CTRL_TX;
          }
        l1s.dsp_ctrl_reg |= CTRL_TX;
      }

    #if TESTMODE
      // Continuous mode: if end of control of START_RX/TX: go to CONTINUOUS state
      if (l1_config.TestMode && (l1_config.tmode.rf_params.tmode_continuous != TM_NO_CONTINUOUS))
        l1_config.tmode.rf_params.tmode_continuous = TM_CONTINUOUS;
    #endif

#if 0	/* FreeCalypso TCS211 reconstruction */
  }
#endif
}

#if (MOVE_IN_INTERNAL_RAM == 0) // Must be followed by the pragma used to duplicate the funtion in internal RAM
//#pragma DUPLICATE_FOR_INTERNAL_RAM_START

/*-------------------------------------------------------*/
/* l1s_hopping_algo()                                    */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* dedicated mode tasks: DDL, DUL, ADL, AUL, TCHTH/F and */
/* TCHA. This function performs the Hopping Sequence     */
/* generation. It computes the ARFCN to use on the next  */
/* frame. When the channel does not hop, it returns      */
/* the fixe ARFCN provided in the channel description.   */
/*                                                       */
/* If the channel is hopping and the ARFCN result is the */
/* BEACON frequency, an indication flag is set to warn   */
/* the DSP ("b_bcch_freq_ind").                          */
/*                                                       */
/* (see GSM05.02 $6.2.3)                                 */
/*                                                       */
/* Input parameters from globals:                        */
/* ------------------------------                        */
/* "l1a_l1s_com.dedic_set.aset"                          */
/*   Active set of Dedicated channel parameters.         */
/*                                                       */
/* "l1s.l1s.next_time"                                   */
/*   frame number and derived numbers for next frame.    */
/*                                                       */
/* Returned parameter in globals:                        */
/* ------------------------------                        */
/*                                                       */
/* "l1a_l1s_com.dedic_set.radio_freq"                    */
/*   ARFCN to be used on the next frame.                 */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_hopping_algo(UWORD8 task, UWORD8 param2)
{
  UWORD8               mai;
  T_CHN_SEL           *chan_sel;
  T_MOBILE_ALLOCATION *alist_ptr;

  UWORD16             *ma;
  UWORD8               n;
  UWORD8               hsn;
  UWORD8               maio;
  UWORD16             *radio_freq_ptr;
  UWORD16             *beacon_channel_ptr=&l1a_l1s_com.Scell_info.radio_freq; // beacon channel of the serving cell
  T_TIME_INFO         *time_ptr;
  T_TIME_INFO         next_neighbor_time;


  switch(task)
  {

#if L1_GPRS
    case PTCCH:
    {
      chan_sel       = &(l1pa_l1ps_com.transfer.aset->freq_param.chan_sel);
      alist_ptr      = &(l1pa_l1ps_com.transfer.aset->freq_param.freq_list);
      radio_freq_ptr = &l1pa_l1ps_com.transfer.ptcch.radio_freq;
      time_ptr       = &l1s.next_time;
    }
    break;

    case PDTCH:
    case SINGLE:
    // For PDTCH, set pointers to the PACKET parameter structures.
    {
      chan_sel       = &(l1pa_l1ps_com.transfer.aset->freq_param.chan_sel);
      alist_ptr      = &(l1pa_l1ps_com.transfer.aset->freq_param.freq_list);
      radio_freq_ptr = &l1a_l1s_com.dedic_set.radio_freq;
      time_ptr       = &l1s.next_time;
    }
    break;

    case PALLC:
    case PNP:
    case PEP:
    {
      chan_sel       = &l1pa_l1ps_com.pccch.packet_chn_desc.chan_sel;
      alist_ptr      = &l1pa_l1ps_com.pccch.frequency_list;
      radio_freq_ptr = &l1pa_l1ps_com.p_idle_param.radio_freq;
      time_ptr       = &l1s.next_time;
    }
    break;

    case POLL:
    {
      // Load adequat freq. list according to the current mode:
      // SINGLE (i.e. 2 phase access) else Packet Access or Packet Idle
      if(l1a_l1s_com.l1s_en_task[SINGLE] == TASK_ENABLED)
      {
        chan_sel       = &(l1pa_l1ps_com.transfer.aset->freq_param.chan_sel);
        alist_ptr      = &(l1pa_l1ps_com.transfer.aset->freq_param.freq_list);
        radio_freq_ptr = &l1a_l1s_com.dedic_set.radio_freq;
      }
      else
      {
        chan_sel       = &l1pa_l1ps_com.pccch.packet_chn_desc.chan_sel;
        alist_ptr      = &l1pa_l1ps_com.pccch.frequency_list;
        radio_freq_ptr = &l1pa_l1ps_com.p_idle_param.radio_freq;
      }
      time_ptr       = &l1s.next_time;
    }
    break;

    case PBCCHS:
    {
      chan_sel       = &l1pa_l1ps_com.pbcchs.packet_chn_desc.chan_sel;
      alist_ptr      = &l1pa_l1ps_com.pbcchs.frequency_list;
      radio_freq_ptr = &l1pa_l1ps_com.p_idle_param.radio_freq;

      // If PBCCHS controlled one frame in advance --> correct Frame Number when PBCCH block is read
      if (l1pa_l1ps_com.pbcchs.control_offset)
        time_ptr       = &l1s.next_plus_time;
      else
      time_ptr       = &l1s.next_time;
    }
    break;

    case PBCCHN_IDLE:
    case PBCCHN_TRAN:
    {
      WORD32  next_neighbor_time_fn;

      chan_sel           = &l1pa_l1ps_com.pbcchn.packet_chn_desc.chan_sel;
      alist_ptr          = &l1pa_l1ps_com.pbcchn.frequency_list;
      radio_freq_ptr     = &l1pa_l1ps_com.p_idle_param.radio_freq;
      beacon_channel_ptr = &l1pa_l1ps_com.pbcchn.bcch_carrier;
      time_ptr           = &next_neighbor_time;

      // To review (is there any better solution?)...........
      next_neighbor_time_fn = l1s.next_time.fn + l1pa_l1ps_com.pbcchn.fn_offset;

      #if 0	/* correct code (corrected by TI for LoCosto) */
      if (next_neighbor_time_fn > ((WORD32)MAX_FN))//OMAPS00090550
      #else	/* wrong code to match TCS211 disassembly */
      if (next_neighbor_time_fn > MAX_FN)
      #endif
        next_neighbor_time.fn = (UWORD32) (next_neighbor_time_fn - MAX_FN);
      else if (next_neighbor_time_fn < 0)
        next_neighbor_time.fn = (UWORD32) (next_neighbor_time_fn + MAX_FN);
      else
        next_neighbor_time.fn = (UWORD32) (next_neighbor_time_fn);

      next_neighbor_time.t1 = next_neighbor_time.fn / (26L*51L);       // T1 = FN div 26*51
      next_neighbor_time.t2 = next_neighbor_time.fn % 26;              // T2 = FN % 26.
      next_neighbor_time.t3 = next_neighbor_time.fn % 51;              // T3 = FN % 51.
    }
    break;

    case ITMEAS:
    {
      // Packet transfer mode
      if (l1a_l1s_com.l1s_en_task[PDTCH] == TASK_ENABLED)
      {
        // We use the active TBF frequency parameters
        chan_sel  = &(l1pa_l1ps_com.transfer.aset->freq_param.chan_sel);
        alist_ptr = &(l1pa_l1ps_com.transfer.aset->freq_param.freq_list);
      }

      // Packet idle mode
      else
      {
        // We use the frequency parameters given in the MPHP_INT_MEAS_REQ message
        chan_sel     = &(l1pa_l1ps_com.itmeas.packet_intm_freq_param.chan_sel);
        alist_ptr    = &(l1pa_l1ps_com.itmeas.packet_intm_freq_param.freq_list);
      }

      radio_freq_ptr = &l1pa_l1ps_com.itmeas.radio_freq;
      time_ptr       = &l1s.next_plus_time;
    }
    break;

#endif

    case SMSCB:
    // For SMSCB, set pointers to the SMSCB parameter structures.
    {
      chan_sel       = &l1a_l1s_com.cbch_desc.chan_sel;
      alist_ptr      = &l1a_l1s_com.cbch_freq_list;
      radio_freq_ptr = &l1a_l1s_com.dedic_set.radio_freq;

      // If SMSCB is controlled one frame in advance --> correct Frame Number when SMSCB block is read
      if (l1a_l1s_com.pre_scheduled_cbch)
        time_ptr       = &l1s.next_plus_time;
      else
      time_ptr       = &l1s.next_time;
    }
    break;

    default:
    // For SDCCH/TCH, set pointers to the active channel description.
    {
      chan_sel       = &(l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->chan_sel);
      alist_ptr      = l1a_l1s_com.dedic_set.aset->ma.alist_ptr;
      radio_freq_ptr = &l1a_l1s_com.dedic_set.radio_freq;
      time_ptr       = &l1s.next_time;
    }
  } // End of switch(task)

  // Set local variables.
  ma   = &(alist_ptr->rf_chan_no.A[0]);
  n    =   alist_ptr->rf_chan_cnt;
  hsn  =   chan_sel->rf_channel.hopping_rf.hsn;
  maio =   chan_sel->rf_channel.hopping_rf.maio;

  if(chan_sel->h == FALSE)
  // Single RF channel, NOT HOPPING.
  {
    *radio_freq_ptr = chan_sel->rf_channel.single_rf.radio_freq;
  }

  else
  // Hopping channel...
  {
    /**************************************************/
    /* Perform the HOPPING algorithm.                 */
    /**************************************************/
    if(hsn == 0)
    // Cyclic hopping...
    {
      mai = (time_ptr->fn + maio) % n;
    }
    else
    {
      UWORD8  i = 0;
      UWORD8  m;
      UWORD8  mp;
      UWORD8  nbin;
      UWORD8  tp;
      UWORD8  s;
      UWORD8  t1r = (UWORD8)(time_ptr->t1 % 64);

      while(i<=6)
      {
        if((n >> i) > 0) nbin = i;
        i++;
      }
      nbin++;

      m  = time_ptr->t2 + RNTABLE[(hsn ^ t1r) + time_ptr->t3];
      mp = m % (1L << nbin);
      tp = time_ptr->t3 % (1L << nbin);

      if(mp < n) s = mp;
      else       s = (mp + tp) % n;

      mai = (s + maio) % n;
    }

    *radio_freq_ptr = ma[mai];
  }

  if(*radio_freq_ptr == *beacon_channel_ptr)
  // If ARFCN is the BEACON...
  {
    // Set "b_bcch_freq_ind" to TRUE.
    l1s_dsp_com.dsp_db_w_ptr->d_ctrl_system |= (1 << B_BCCH_FREQ_IND);

  }
}
//#pragma DUPLICATE_FOR_INTERNAL_RAM_END
#endif // MOVE_IN_INTERNAL_RAM

//===============================================================================================//

#if !((MOVE_IN_INTERNAL_RAM == 1) && (GSM_IDLE_RAM !=0))  // MOVE TO INTERNAL MEM IN CASE GSM_IDLE_RAM enabled
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_START         // KEEP IN EXTERNAL MEM otherwise

/*-------------------------------------------------------*/
/* l1s_read_dummy()                                      */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* task: ABORT. Since this task just aborts any ongoing  */
/* DSP task, there is no result returned by the DSP to   */
/* the MCU when this abort is completed, but the MCU/DSP */
/* com. read page must be switched properly. This the    */
/* only reason why we have created this function.        */
/*                                                       */
/* Modified parameter in globals:                        */
/* ------------------------------                        */
/*                                                       */
/* "l1s_dsp_com.dsp_r_page_used"                         */
/*   Flag used by the function which closes L1S          */
/*   execution ("l1s_end_manager()") to know if the      */
/*   MCU/DSP read page must be switched.                 */
/*   -> Set to 1.                                        */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_read_dummy(UWORD8 task, UWORD8 param2)
{
  l1_check_com_mismatch(task);

  #if (TRACE_TYPE!=0) && (TRACE_TYPE!=5)
    trace_fct(CST_L1S_READ_DUMMY ,(UWORD32)(-1));//OMAPS00090550
  #endif

      // task is completed, make it INACTIVE (only in case of TCHD).
  if(task == TCHD)
   l1s.task_status[task].current_status = INACTIVE;

  #if FF_L1_IT_DSP_DTX
    // Fast DTX status update
    if(task == TCHD)
    {
      UWORD8 subchannel = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->subchannel;

      // Currently used for TCH-AHS only
      if (((subchannel == 0) && (l1s.actual_time.fn_mod13_mod4 == 0)) || // FN%13 = 4, 8 and 12 for TCH/H0 (no Read on FN%13=0)
          ((subchannel == 1) && (l1s.actual_time.fn_mod13_mod4 == 1)))   // FN%13 = 1, 5 and 9  for TCH/H1
      {
        // Latch TX activity status if DTX allowed
        if ((l1a_l1s_com.dedic_set.aset->dtx_allowed == FALSE) ||                // No DTX allowed
            (l1s_dsp_com.dsp_ndb_ptr->d_fast_dtx_enc_data ) || // DTX allowed but not used
            (l1a_apihisr_com.dtx.fast_dtx_ready == FALSE))                       // Fast DTX status is invalid
          l1a_apihisr_com.dtx.tx_active = TRUE;
        else
          l1a_apihisr_com.dtx.tx_active = FALSE;
      }
    }
  #endif
  // Set flag used to change the read page at the end of "l1_synch".
  l1s_dsp_com.dsp_r_page_used = TRUE;
}

//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END
#endif

/*-------------------------------------------------------*/
/* l1s_read_msagc()                                      */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* tasks: BCCHN,FBNEW,SB1,SB2,SBCONF. This function is   */
/* the reading result function used for reading a power  */
/* measurement result used then to refreshing the AGC    */
/* for those tasks. Here is a summary of the execution:  */
/*                                                       */
/*  - If SEMAPHORE(task) is low.                         */
/*    - Get the cell information structure.              */
/*    - Traces and debug.                                */
/*    - Read receive level result from MCU/DSP interface.*/
/*  - Flag the use of the MCU/DSP dual page read         */
/*    interface.                                         */
/*                                                       */
/* Input parameters:                                     */
/* -----------------                                     */
/* "task"                                                */
/*   BCCHN, BCCH Neighbor reading task.                  */
/*   FBNEW, Frequency Burst detection task in Idle mode. */
/*   SB1, Synchro Burst reading task in Idle mode.       */
/*   SB2, Synchro Burst detection task in Idle mode.     */
/*   SBCONF, Synchro Burst confirmation task in Idle     */
/*   mode.                                               */
/*                                                       */
/* Input parameters from globals:                        */
/* ------------------------------                        */
/* "l1a_l1s_com.Ncell_info.bcch"                         */
/* "l1a_l1s_com.Ncell_info.acquis"                       */
/* "l1a_l1s_com.Ncell_info.acquis"                       */
/* "l1a_l1s_com.Ncell_info.conf"                         */
/*   cell information structure used for BCCHN,FBNEW,    */
/*   SB1/SB2,SBCONF respectively.                        */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/* "l1s_dsp_com.dsp_r_page_used"                         */
/*   Flag used by the function which closes L1S          */
/*   execution ("l1s_end_manager()") to know if the      */
/*   MCU/DSP read page must be switched.                 */
/*   -> Set to 1.                                        */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_read_msagc(UWORD8 task, UWORD8 param2)
{
  BOOL      en_task;
  BOOL      task_param;
  UWORD8    pm_level[2];
#if (L1_FF_MULTIBAND == 1)
  UWORD16  operative_radio_freq;
#endif


  // Get "enable" task flag and "synchro semaphore" for current task.
  en_task    = l1a_l1s_com.l1s_en_task[task];
  task_param = l1a_l1s_com.task_param[task];

  if((en_task) && !(task_param))
  // Check the task semaphore and the task enable bit. The reading
  // task body is executed only when the task semaphore is 0 and the
  // task is still enabled.
  // The semaphore can be set to 1 whenever L1A makes some changes
  // to the task parameters. The task can be disabled by L1A.
  {
    T_NCELL_SINGLE  *cell_info_ptr = NULL;
    #if (L1_GPRS)
      T_NCELL_SINGLE pbcchn_cell_info;
    #endif
#if ((REL99 == 1) && (FF_BHO == 1))
    T_NCELL_SINGLE bho_cell_info;
#endif

    // Get the cell information structure.
    // ************************************

    switch(task)
    {
      case BCCHN_TOP:  cell_info_ptr = &l1a_l1s_com.bcchn.list[l1a_l1s_com.bcchn.active_neigh_id_top  ]; break;
      case BCCHN:      cell_info_ptr = &l1a_l1s_com.bcchn.list[l1a_l1s_com.bcchn.active_neigh_id_norm ]; break;
      case FBNEW:  cell_info_ptr = &l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_fb_id];     break;
      case SB2:    cell_info_ptr = &l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_sb_id];     break;
      case SBCONF: cell_info_ptr = &l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_sbconf_id]; break;
#if ((REL99 == 1) && (FF_BHO == 1))
      case FBSB:
      {
        cell_info_ptr = &bho_cell_info;
        bho_cell_info.radio_freq = l1a_l1s_com.nsync_fbsb.radio_freq;
        bho_cell_info.fn_offset  = l1a_l1s_com.nsync_fbsb.fn_offset;
      }
      break;
#endif
      #if (L1_GPRS)
        case PBCCHN_IDLE:
        {
          cell_info_ptr = &pbcchn_cell_info;
          pbcchn_cell_info.radio_freq = l1pa_l1ps_com.pbcchn.bcch_carrier;
          pbcchn_cell_info.fn_offset  = l1pa_l1ps_com.pbcchn.fn_offset;
        }
        break;
      #endif

      default: return;
    }

    // Traces and debug.
    // ******************

    #if (TRACE_TYPE!=0)
      trace_fct(CST_L1S_READ_MSAGC , cell_info_ptr->radio_freq);
    #endif

    #if L2_L3_SIMUL
      #if (DEBUG_TRACE == BUFFER_TRACE_OFFSET_NEIGH)
        buffer_trace(4, l1s.actual_time.fn, cell_info_ptr->radio_freq,
                      cell_info_ptr->fn_offset, pm);
      #endif
    #endif

    l1_check_com_mismatch(MS_AGC_ID);

    // Read receive level result from MCU/DSP interface.
    // **************************************************
    // Read 2 received levels...
    #if L1_GPRS
      switch (l1a_l1s_com.dsp_scheduler_mode)
      {
        case GPRS_SCHEDULER:
        {
          // Call the reading driver using GPRS scheduler
          l1pddsp_meas_read(2, pm_level);
        } break;

        case GSM_SCHEDULER:
        {
          // Call the reading driver using GSM scheduler
          l1ddsp_meas_read(2, pm_level);
        } break;
      }
    #else
      l1ddsp_meas_read(2, pm_level);
    #endif

    // Power Measurement performed during last l1s_ctrl_msagc with HIGH_AGC
    // returned in pm_level[0]
    // Power measurement performed during last l1s_ctrl_msagc with LOW_AGC
    // returned in pm_level[1]

    l1_check_pm_error(pm_level[0], MS_AGC_ID);
    l1_check_pm_error(pm_level[1], MS_AGC_ID);

    l1ctl_pgc2(pm_level[0], pm_level[1], cell_info_ptr->radio_freq);

    #if L2_L3_SIMUL
      #if (DEBUG_TRACE == BUFFER_TRACE_LNA)
      
       #if (L1_FF_MULTIBAND == 0)
            buffer_trace (4, 22, cell_info_ptr->radio_freq,
                      l1a_l1s_com.last_input_level[cell_info_ptr->radio_freq - l1_config.std.radio_freq_index_offset].input_level,
                      l1a_l1s_com.last_input_level[cell_info_ptr->radio_freq - l1_config.std.radio_freq_index_offset].lna_off);

       #else // L1_FF_MULTIBAND = 1 below

            operative_radio_freq = 
              l1_multiband_radio_freq_convert_into_operative_radio_freq(cell_info_ptr->radio_freq);
            buffer_trace (4, 22, cell_info_ptr->radio_freq,
                     l1a_l1s_com.last_input_level[cell_info_ptr->agc_index].input_level,
                     l1a_l1s_com.last_input_level[cell_info_ptr->agc_index].lna_off); 

       #endif // #if (L1_FF_MULTIBAND == 0) else

      #endif
    #endif
  }

  // Flag the use of the MCU/DSP dual page read interface.
  // ******************************************************

  // Set flag used to change the read page at the end of "l1_synch".
  l1s_dsp_com.dsp_r_page_used = TRUE;
}

#if (MOVE_IN_INTERNAL_RAM == 0) // Must be followed by the pragma used to duplicate the funtion in internal RAM
//#pragma DUPLICATE_FOR_INTERNAL_RAM_START

/*-------------------------------------------------------*/
/* l1s_read_mon_result()                                 */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* tasks: FBNEW,SB1,SB2,SBCONF,SB51,SBCNF51,SB26,SBCNF26.*/
/* This function is the reading result function used for */
/* reading the neighbor cell monitoring results. Here is */
/* a summary of the execution:                           */
/*                                                       */
/*  - Traces and debug.                                  */
/*  - Get task result from MCU/DSP read interface.       */
/*                                                       */
/*      - case: FBNEW/FB51.                              */
/*          - If SEMAPHORE(task) is low.                 */
/*              - Update AFC if required.                */
/*              - Read FB detection results.             */
/*              - Reports results to L1A.                */
/*          - Disactivate and Disable task.              */
/*          - Reset buffers and flags in NDB.            */
/*                                                       */
/*      - case: FB26.                                    */
/*          - Read FB detection results.                 */
/*          - Reports results to L1A.                    */
/*          - Disactivate task.                          */
/*                                                       */
/*      - case: SB26/SBCNF26.                            */
/*          - Read SB reading results.                   */
/*          - Reports results to L1A.                    */
/*          - Disactivate task.                          */
/*                                                       */
/*      - case: SB1/SB2/SB51/SBCONF/SBCNF51.             */
/*          - If SEMAPHORE(task) is low.                 */
/*              - Update AFC if required.                */
/*              - Read FB detection results.             */
/*              - Reports results to L1A.                */
/*          - Disactivate task when required.            */
/*                                                       */
/*  - Flag the use of the MCU/DSP dual page read         */
/*    interface.                                         */
/*                                                       */
/* Input parameters:                                     */
/* -----------------                                     */
/* "task"                                                */
/*   FBNEW, Frequency Burst detection task in Idle mode. */
/*   SB1, Synchro Burst reading task in Idle mode.       */
/*   SB2, Synchro Burst detection task in Idle mode.     */
/*   SBCONF, Synchro Burst confirmation task in Idle     */
/*   mode.                                               */
/*   SB51, Synchro Burst reading task in SDCCH Dedicated */
/*   mode.                                               */
/*   SBCNF51, Synchro Burst confirmation task in SDCCH   */
/*   Dedicated mode.                                     */
/*   SB26, Synchro Burst reading task in TCH Dedicated   */
/*   mode.                                               */
/*   SBCNF26, Synchro Burst confirmation task in TCH     */
/*   Dedicated mode.                                     */
/*                                                       */
/* "attempt_for_sb2"                                     */
/*   Since SB2 calls twice this function, this parameter */
/*   tells the function which call it it. Used mainly    */
/*   to know when to DISACTIVATE the task.               */
/*                                                       */
/* Input parameters from globals:                        */
/* ------------------------------                        */
/* "l1a_l1s_com.task_param"                              */
/*   task semaphore bit register. Used to skip the body  */
/*   of this function if L1A has changed or is changing  */
/*   some of the task parameters.                        */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/* "l1a_l1s_com.l1s_en_task"                             */
/*   L1S task enable bit register.                       */
/*   -> disable FBNEW,FB51 task.                         */
/*                                                       */
/* "l1s.task_status[task].current_status"                */
/*   current task status. It must be reset (INACTIVE)    */
/*   when the task is completed.                         */
/*   -> disactivate task.                                */
/*                                                       */
/* "l1s_dsp_com.dsp_r_page_used"                         */
/*   Flag used by the function which closes L1S          */
/*   execution ("l1s_end_manager()") to know if the      */
/*   MCU/DSP read page must be switched.                 */
/*   -> Set to 1.                                        */
/*                                                       */
/* Use of MCU/DSP interface:                             */
/* -------------------------                             */
/* "l1s_dsp_com.dsp_ndb_ptr"                             */
/*   pointer to the non double buffered part (NDB) of    */
/*   the MCU/DSP interface. This part is R/W for both    */
/*   DSP and MCU.                                        */
/*                                                       */
/* "l1s_dsp_com.dsp_db_r_ptr"                            */
/*   pointer to the double buffered part (DB) of the     */
/*   MCU/DSP interface. This pointer points to the READ  */
/*   page.                                               */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_read_mon_result(UWORD8 task, UWORD8 attempt)
{
  UWORD32   flag=0;
  UWORD32   toa;
  UWORD32   pm;
  UWORD32   angle;
  UWORD32   snr;

  #if TESTMODE
    UWORD32   pm_fullres;
  #endif

  API      *data;

  BOOL      en_task;
  BOOL      task_param;
  UWORD32   fb_abort_flag=0;

  /*-------------------------------------------------------------------------------*/
  /* READ MONITORING TASK RESULTS FROM MCU/DSP INTERFACE...                        */
  /*-------------------------------------------------------------------------------*/
  // Get "enable" task flag and "synchro semaphore" for current task.
  en_task    = l1a_l1s_com.l1s_en_task[task];
  task_param = l1a_l1s_com.task_param[task];

  // Traces and debug.
  // ******************
    #if (TRACE_TYPE!=0)&& (TRACE_TYPE !=5)
      trace_fct(CST_L1S_READ_MON_RESULT,(UWORD32)(-1));
    #endif

  if(!(en_task) || (task_param))
  {
    #if (TRACE_TYPE!=0)
      // Current task is no more alive, L1A changed the task parameters.
      // -> Trace "ABORT" on log file and screen.
      trace_fct(CST_TASK_KILLED, (UWORD32)(-1));
    #endif
  }
  else
  // Current task is still alive, check task identifier and debug number...
  {
    #if (TRACE_TYPE!=0)
      if((task != FB26) && (task != SB26) && (task != SBCNF26)) // DB cannot be used for FB26/SB26/SBCNF26 result.
        if((UWORD32)(l1s_dsp_com.dsp_db_r_ptr->d_task_md & 0xffff) !=
           (UWORD32)DSP_TASK_CODE[task])
          // Task id. different than the one expected...
          trace_fct(CST_DL_TASKS_DO_NOT_CORRESPOND,(UWORD32)(-1));
    #endif

    if((task != FB26) && (task != SB26)
       && (task != SBCNF26) && (attempt==12)
       #if ((REL99 == 1) && (FF_BHO == 1))
         && (task != FBSB)
       #endif
      ) // DB cannot be used for FB26/SB26/SBCNF26 result.
    {
        l1_check_com_mismatch(task);
    }
  }

  // Get task result from MCU/DSP read interface.
  // *********************************************

  switch(task)
  {
    case FBNEW :
    case FB51 :
    /*---------------------------------------------------*/
    /* Frequency burst detection result...               */
    /*---------------------------------------------------*/
    {
      if((en_task) && !(task_param))
      // Check the task semaphore and the task enable bit. The reading
      // task body is executed only when the task semaphore is 0 and the
      // task is still enabled.
      // The semaphore can be set to 1 whenever L1A makes some changes
      // to the task parameters. The task can be disabled by L1A.
      {
        flag  = l1s_dsp_com.dsp_ndb_ptr->d_fb_det              & 0xffff; //  1 means FOUND.
        toa   = l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_TOA]   & 0xffff; //  Unit is BIT.
        pm    = (l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_PM]   & 0xffff) >> 5;
                                                                         //  WARNING... to be used!!!
        #if TESTMODE
          pm_fullres = l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_PM] & 0xffff;  // F26.6
        #endif
        angle = l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_ANGLE] & 0xffff; //  WARNING... to be used!!!
        snr   = l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_SNR]   & 0xffff; //  WARNING... to be used!!!


        // Check FB detection flag and attempt:
        // If no detection and attempt < 12 than continue FB search
        // Attempt=11: special case: wait for next (last) read, as
        // other task may already be programmed in MFTAB (do not flush !!!)
        if(((!flag) && (attempt < 11)) || (attempt==11))
          break;

        // If FB detection occurs before 11th attempt, abort FB search
        if((flag == TRUE) && (attempt < 11))
          fb_abort_flag=TRUE;


        if (fb_abort_flag == TRUE)
        {
          if ((l1s_dsp_com.dsp_db_r_ptr->d_debug & 0xffff) != ((l1s.debug_time + (12 - attempt)) % 65536))
            l1_check_com_mismatch(task);
        }

//       l1_check_pm_error(pm, task);

        #if TRACE_TYPE==3
          stats_samples_fb(flag,toa,pm,angle,snr);
        #endif

        #if (TRACE_TYPE==2 ) || (TRACE_TYPE==3)
          uart_trace(FB51);
        #endif

        // Update AFC: Call AFC control function (KALMAN filter).
        #if AFC_ALGO
          #if TESTMODE
            if (l1_config.afc_enable)
          #endif
            {
              WORD16 old_afc=l1s.afc;

           if((flag == TRUE) && (l1a_l1s_com.mode == CS_MODE))
           {
            #if (VCXO_ALGO == 0)
              l1s.afc = l1ctl_afc(AFC_OPEN_LOOP,
                                  &l1s.afc_frame_count,
                                  (WORD16)angle,
                                  0,
                                  l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_fb_id].radio_freq);
            #else
              l1s.afc = l1ctl_afc(AFC_OPEN_LOOP,
                                  &l1s.afc_frame_count,
                                  (WORD16)angle,
                                  0,
                                  l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_fb_id].radio_freq,l1a_l1s_com.mode);
            #endif
            #if L2_L3_SIMUL
              #if (DEBUG_TRACE == BUFFER_TRACE_AFC_OPEN)
                buffer_trace (4,(WORD16)angle,old_afc,l1s.afc,0);
              #endif
            #endif
              }
            }
        #endif

        // Call FB report function (send report msg to L1A).
        #if TESTMODE
          if (l1_config.TestMode)
            l1s_read_fb(task, flag, toa, attempt, pm_fullres, angle, snr);
          else
            l1s_read_fb(task, flag, toa, attempt, pm, angle, snr);
        #else
          l1s_read_fb(task, flag, toa, attempt, pm, angle, snr);
        #endif

        // The Frequency Burst detection task in Idle (FBNEW) and
        // Dedicated/SDCCH (FB51) are 1 shot tasks, they must be
        // disabled in L1S when they are completed. Disable it.
        l1a_l1s_com.l1s_en_task[task] = TASK_DISABLED;

        // the status is not used in D51 and D26 modes
        if (task != FB51 )
        {
          l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_fb_id].status = NSYNC_COMPLETED;
        }
      }

      if ((fb_abort_flag == TRUE) || (attempt==12))
      {
        // FB task is completed, make it INACTIVE.
        l1s.task_status[task].current_status = INACTIVE;

        // Reset buffers and flags in NDB ...
        l1s_dsp_com.dsp_ndb_ptr->d_fb_det            = FALSE;
        l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_TOA] = 0;

        // This task is not compatible with Neigh. Measurement.
        // Clear "forbid_meas" to indicate when the task is complete.
        l1s.forbid_meas = 0;
      }

      // FB search finished before 11th attempt:
      // -reset DSP R/W pages, DSP tasks and TPU
      // -flush MFTAB and reset frame count
      // -adjust debug time
      if(fb_abort_flag)
      {
        l1d_reset_hw(l1s.tpu_offset);
        l1s.tpu_ctrl_reg |= CTRL_FB_ABORT; // set CTRL bit -> tpu_end_scenario
        l1s_clear_mftab(l1s.mftab.frmlst);
        l1s.frame_count = 0;

#if 0	/* FreeCalypso TCS211 reconstruction */
        // This task is not compatible with Neigh. Measurement.
        // Clear "forbid_meas" to indicate when the task is complete.
        l1s.forbid_meas = 0;
#endif
      }
    }
    break;

    case FB26 :
    /*---------------------------------------------------*/
    /* Frequency burst detection result...               */
    /*---------------------------------------------------*/
    {
      UWORD8 neigh_id;

      // read cell identifier.
      neigh_id = l1a_l1s_com.nsync.active_fb_id;

      if((en_task) && !(task_param))
      // Check the task semaphore and the task enable bit. The reading
      // task body is executed only when the task semaphore is 0 and the
      // task is still enabled.
      // The semaphore can be set to 1 whenever L1A makes some changes
      // to the task parameters. The task can be disabled by L1A.
      {
        flag  = l1s_dsp_com.dsp_ndb_ptr->d_fb_det              & 0xffff; //  1 means FOUND.
        toa   = l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_TOA]   & 0xffff; //  Unit is BIT.
        pm    = (l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_PM]   & 0xffff) >> 5;
                                                                         //  WARNING... to be used!!!
        #if TESTMODE
          pm_fullres = l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_PM] & 0xffff;  // F10.6
        #endif

        // CQ 19836: do not check PM on FB26
        //l1_check_pm_error(pm, task);

        angle = l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_ANGLE] & 0xffff; //  WARNING... to be used!!!
        snr   = l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_SNR]   & 0xffff; //  WARNING... to be used!!!

        // Call FB report function (send report msg to L1A).
        #if TESTMODE
          if (l1_config.TestMode)
            l1s_read_fb(task, flag, toa, NO_PAR, pm_fullres, angle, snr);
          else
            l1s_read_fb(task, flag, toa, NO_PAR, pm, angle, snr);
        #else
          l1s_read_fb(task, flag, toa, NO_PAR, pm, angle, snr);
        #endif
      }

      #if (TRACE_TYPE==2 ) || (TRACE_TYPE==3)
        uart_trace(FB26);
      #endif


      // The Frequency Burst detection task in Dedicated/TCH
      // is composed with several attempts managed in L1A.
      //    -> task is completed: set INACTIVE.
      l1s.task_status[task].current_status = INACTIVE;

      // Reset buffers and flags in NDB ...
      l1s_dsp_com.dsp_ndb_ptr->d_fb_det            = FALSE;
      l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_TOA] = 0;
    }
    break;

    case SB26 :
    case SBCNF26 :
    /*---------------------------------------------------*/
    /* Synchro. burst detection result...                */
    /*---------------------------------------------------*/
    {
      if((en_task) && !(task_param))
      // Check the task semaphore and the task enable bit. The reading
      // task body is executed only when the task semaphore is 0 and the

      // task is still enabled.
      // The semaphore can be set to 1 whenever L1A makes some changes
      // to the task parameters. The task can be disabled by L1A.
      {
        flag  = !(((l1s_dsp_com.dsp_ndb_ptr->a_sch26[0] & 0xffff) & (1<<B_SCH_CRC)) >> B_SCH_CRC); //  1 means ERROR.
        toa   = l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_TOA]   & 0xffff;  //  Unit is BIT.
        pm    = (l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_PM]   & 0xffff) >> 5;
                                                                          //  WARNING... to be used!!!
        #if TESTMODE
          pm_fullres = l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_PM] & 0xffff;  // F26.6
        #endif

        angle = l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_ANGLE] & 0xffff;
        snr   = l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_SNR]   & 0xffff;
        data  = &(l1s_dsp_com.dsp_ndb_ptr->a_sch26[3]);                   // Set data block pointer (skip header).

        l1_check_pm_error(pm, task);

        // Call SB report function (send report msg to L1A).
        #if TESTMODE
          if (l1_config.TestMode)
            l1s_read_sb(task, flag, data, toa, attempt, pm_fullres, angle, snr);
          else
            l1s_read_sb(task, flag, data, toa, attempt, pm, angle, snr);
        #else
          l1s_read_sb(task, flag, data, toa, attempt, pm, angle, snr);
        #endif
      }
      #if (TRACE_TYPE==2 ) || (TRACE_TYPE==3)
        uart_trace(SB26);
      #endif


      // The Synchro Burst detection (SB26) and confirmation (SBCNF26)
      // tasks in Dedicated/TCH are enabling/disabling are fully
      // managed by L1A.
      //    -> task is completed: set INACTIVE.
      l1s.task_status[task].current_status = INACTIVE;

      // Reset buffers and flags in NDB ...
      l1s_dsp_com.dsp_ndb_ptr->a_sch26[0]    =  (1<<B_SCH_CRC);
    }
    break;

    case SB2 :
    case SBCONF :
    case SB51 :
    case SBCNF51 :
    /*---------------------------------------------------*/
    /* Synchro. burst detection result...                */
    /*---------------------------------------------------*/
    {
      if((en_task) && !(task_param))
      // Check the task semaphore and the task enable bit. The reading
      // task body is executed only when the task semaphore is 0 and the
      // task is still enabled.
      // The semaphore can be set to 1 whenever L1A makes some changes
      // to the task parameters. The task can be disabled by L1A.
      {
        UWORD8  neigh_id;

        if((task == SB2) || (task == SB51))
          neigh_id = l1a_l1s_com.nsync.active_sb_id;
        else
          neigh_id = l1a_l1s_com.nsync.active_sbconf_id;

        flag  = !(((l1s_dsp_com.dsp_db_r_ptr->a_sch[0] & 0xffff) & (1<<B_SCH_CRC)) >> B_SCH_CRC); //  1 means ERROR.
        toa   = l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_TOA]   & 0xffff;  //  Unit is BIT.
        pm    = (l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_PM]   & 0xffff) >> 5;
                                                                           //  WARNING... to be used!!!
        #if TESTMODE
          pm_fullres = l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_PM] & 0xffff;  // F26.6
        #endif

        angle = l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_ANGLE] & 0xffff;
        snr   = l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_SNR]   & 0xffff;
        data  = &(l1s_dsp_com.dsp_db_r_ptr->a_sch[3]);                     // Set data block pointer (skip header).

        #if (L1_DEBUG_IQ_DUMP == 1)
          l1ddsp_read_iq_dump(task);
        #endif

        l1_check_pm_error(pm, task);

        // CQ30474. In case SNR is too low, the SB shall be considered as failed.
        // This is valuable for code running on target with DSP 3606.

	/*
	 * FreeCalypso: despite the above comment,
	 * this code is NOT present in TCS211.
	 */
#if 0
#if (CODE_VERSION == NOT_SIMULATION)
       if ( snr < MIN_ACCEPTABLE_SNR_FOR_SB )
         flag = FALSE;
#endif
#endif

        #if L2_L3_SIMUL
          #if (DEBUG_TRACE == BUFFER_TRACE_OFFSET_NEIGH)
            buffer_trace(4, l1s.actual_time.fn, toa,pm, l1s_dsp_com.dsp_db_r_ptr->a_sch[0] & 0xffff);
          #endif
        #endif

        #if TRACE_TYPE==3
          stats_samples_sb(flag,toa,pm,angle,snr);
        #endif
        #if (TRACE_TYPE==2 ) || (TRACE_TYPE==3)
          if (task == SBCONF) uart_trace(SBCONF);
          else uart_trace(SB2);        // display result code...
        #endif

        // Update AFC: Call AFC control function (KALMAN filter).
        #if AFC_ALGO
          #if TESTMODE
            if (l1_config.afc_enable)
          #endif
            {
              WORD16 old_afc=l1s.afc;

           if((flag == TRUE) && (l1a_l1s_com.mode == CS_MODE))
           {
            #if (VCXO_ALGO == 0)
              l1s.afc = l1ctl_afc(AFC_OPEN_LOOP,
                                  &l1s.afc_frame_count,
                                  (WORD16)angle,
                                  0,
                                  l1a_l1s_com.nsync.list[neigh_id].radio_freq);
            #else
              l1s.afc = l1ctl_afc(AFC_OPEN_LOOP,
                                  &l1s.afc_frame_count,
                                  (WORD16)angle,
                                  0,
                                  l1a_l1s_com.nsync.list[neigh_id].radio_freq,l1a_l1s_com.mode);
            #endif

                #if L2_L3_SIMUL
                  #if (DEBUG_TRACE == BUFFER_TRACE_AFC_OPEN)
                    buffer_trace (4,(WORD16)angle,old_afc,l1s.afc,1);
                  #endif
                #endif
              }
            }
        #endif

        // Call SB report function (send report msg to L1A).
        #if TESTMODE
          if (l1_config.TestMode)
            l1s_read_sb(task, flag, data, toa, attempt, pm_fullres, angle, snr);
          else
            l1s_read_sb(task, flag, data, toa, attempt, pm, angle, snr);
        #else
          l1s_read_sb(task, flag, data, toa, attempt, pm, angle, snr);
        #endif

        // the status is not used in D51 and D26 modes
        if ((task != SBCNF51 ) && (task != SB51))
        {
          // SB2 activity completed for this neighbour cell.
          if((task != SB2) || ((task == SB2) && (attempt == 2)))
            l1a_l1s_com.nsync.list[neigh_id].status = NSYNC_COMPLETED;
        }
      }

      // All tasks are completed by this function except SB2 which
      // calls it twice. SB2 is then completed only when making the
      // second execution of this function.
      //    -> task is completed: set INACTIVE.
      if((task != SB2) || (task == SB2) && (attempt == 2))
      {
        l1s.task_status[task].current_status = INACTIVE;
        l1a_l1s_com.l1s_en_task[task] = TASK_DISABLED;
      }
    }
    break;
#if ((REL99 == 1) && (FF_BHO == 1))
    case FBSB :

    /*---------------------------------------------------*/
    /* Frequency + Synchro burst detection result...     */
    /*---------------------------------------------------*/
    {
      BOOL abort_flag = FALSE;

      if (l1a_l1s_com.nsync_fbsb.fb_found_attempt == 0)
      // Looking for FB
      {
        flag  = l1s_dsp_com.dsp_ndb_ptr->d_fb_det & 0xffff; //  flag = TRUE means FOUND.
        toa   = l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_TOA]   & 0xffff; //  Unit is BIT.
        pm    = (l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_PM]   & 0xffff) >> 5;
#if TESTMODE
        pm_fullres = l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_PM] & 0xffff;  // F26.6
#endif
        angle = l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_ANGLE] & 0xffff;
        snr   = l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_SNR]   & 0xffff;

        if (flag) // FB detected
        {
          // Store toa and attempt for future use
          l1a_l1s_com.nsync_fbsb.fb_toa = toa;
          l1a_l1s_com.nsync_fbsb.fb_found_attempt = attempt;

#if (TRACE_TYPE == 3)
          stats_samples_fb(flag, toa, pm, angle, snr);
#endif

#if (TRACE_TYPE == 2 ) || (TRACE_TYPE == 3)
          // uart_trace(FBSB);
#endif
        }
        else
        {
          if (attempt < 12)
          {
            // FB not found, some attempts remaining
            break;
          }
          else
          {
            // FB not found, no attempt remaining
            // Call FBSB report function (send report msg to L1A).
#if TESTMODE
            if (l1_config.TestMode)
              l1s_read_fbsb(task, attempt, FALSE, FALSE, (API *)NULL, toa, pm, angle, snr);
            else
#endif
              l1s_read_fbsb(task, attempt, FALSE, FALSE, (API *)NULL, toa, pm, angle, snr);

            abort_flag = TRUE;
          }
        }
      }
      else // if (l1a_l1s_com.nsync_fbsb.fb_found_attempt == 0)
      // Looking for SB
      {
        flag  = !(((l1s_dsp_com.dsp_db_r_ptr->a_sch[0] & 0xffff) & (1<<B_SCH_CRC)) >> B_SCH_CRC); //  //  flag = TRUE means FOUND.
        toa   = l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_TOA]   & 0xffff;  //  Unit is BIT.
        pm    = (l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_PM]   & 0xffff) >> 5;
#if TESTMODE
        pm_fullres = l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[D_PM] & 0xffff;  // F26.6
#endif
        angle = l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_ANGLE] & 0xffff;
        snr   = l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_SNR]   & 0xffff;
        data  = &(l1s_dsp_com.dsp_db_r_ptr->a_sch[3]);  		   // Set data block pointer (skip header).

        if (flag) // SB detected
        {
          // SB found report SUCCESS

#if L2_L3_SIMUL
#if (DEBUG_TRACE == BUFFER_TRACE_OFFSET_NEIGH)
          buffer_trace(4, l1s.actual_time.fn, toa,pm, l1s_dsp_com.dsp_db_r_ptr->a_sch[0] & 0xffff);
#endif
#endif

#if (TRACE_TYPE == 3)
          stats_samples_sb(flag, toa, pm, angle, snr);
#endif

#if (TRACE_TYPE == 2 ) || (TRACE_TYPE == 3)
          // uart_trace(FBSB);        // display result code...
#endif

          // Call FBSB report function (send report msg to L1A).
#if TESTMODE
  	      if (l1_config.TestMode)
  	        l1s_read_fbsb(task, attempt, TRUE, TRUE, data, toa, pm, angle, snr);
  	      else
#endif
  	        l1s_read_fbsb(task, attempt, TRUE, TRUE, data, toa, pm, angle, snr);

  	      abort_flag = TRUE;
  	    }
  	    else // if (flag)
  	    {
  	      if (attempt < (l1a_l1s_com.nsync_fbsb.fb_found_attempt + 2))
  	      {
  	        // SB not found, one attempt remaining
  	        break;
  	      }
  	      else
  	      {
  	        // SB not found, no attempt remaining
  	        // Call FBSB report function (send report msg to L1A).
#if TESTMODE
 	        if (l1_config.TestMode)
 	          l1s_read_fbsb(task, attempt, TRUE, FALSE, (API *)NULL, toa, pm, angle, snr);
 	        else
#endif
  	          l1s_read_fbsb(task, attempt, TRUE, FALSE, (API *)NULL, toa, pm, angle, snr);

  	        abort_flag = TRUE;
  	      }
  	    }
      } // if(l1a_l1s_com.nsync_fbsb.fb_found_attempt == 0)


      if(abort_flag == TRUE)
      {
        //    -> task is completed: set INACTIVE.
        l1s.task_status[task].current_status = INACTIVE;
        l1a_l1s_com.l1s_en_task[task] = TASK_DISABLED;

        if (attempt < 14)
        {
          // FBSB search finished before last attempt:
          // -reset DSP R/W pages, DSP tasks and TPU
          // -flush MFTAB and reset frame count
          // -adjust debug time
          l1d_reset_hw(l1s.tpu_offset);
          l1s.tpu_ctrl_reg |= CTRL_FBSB_ABORT; // set CTRL bit -> tpu_end_scenario
          l1s_clear_mftab(l1s.mftab.frmlst);
          l1s.frame_count = 0;
        }
      }
    }
#endif // #if ((REL99 == 1) && (FF_BHO == 1))
  }

  // Flag the use of the MCU/DSP dual page read interface.
  // ******************************************************

  // Set flag used to change the read page at the end of "l1_synch" only if not
  // in dedicated/TCH mode (FB26,SB26,SBCNF26). Those task are not following the
  // common principle. They use only the NDB part of the MCU/DSP interface, no
  // page swapping is then needed.
#if ((REL99 == 1) && (FF_BHO == 1))
  if(((task != FB26) && (task != SB26) && (task != SBCNF26) && (task != FBNEW) && (task != FB51) && (task != FBSB)) ||
#else
  if(((task != FB26) && (task != SB26) && (task != SBCNF26) && (task != FBNEW) && (task != FB51)) ||
#endif // #if ((REL99 == 1) && (FF_BHO == 1))
     ((!fb_abort_flag) && (attempt==12)))
    l1s_dsp_com.dsp_r_page_used = TRUE;
}
//#pragma DUPLICATE_FOR_INTERNAL_RAM_END
#endif // MOVE_IN_INTERNAL_RAM

#if !((MOVE_IN_INTERNAL_RAM == 1) && (GSM_IDLE_RAM !=0))  // MOVE TO INTERNAL MEM IN CASE GSM_IDLE_RAM enabled
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_START         // KEEP IN EXTERNAL MEM otherwise

/*-------------------------------------------------------*/
/* l1s_read_snb_dl()                                     */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* tasks: BCCHS,NP,EP,ALLC,SMSCB.                        */
/* This function is the reading result function used for */
/* reading a serving cell burst acquisition result in    */
/* any mode except dedicated mode. Here is a summary of  */
/* the execution:                                        */
/*                                                       */
/*  - If SEMAPHORE(task) is low and task still enabled.  */
/*      - Traces and debug.                              */
/*      - Read control results and feed control algo.    */
/*      - Read DL DATA block from MCU/DSP interface.     */
/*  - Disactivate task.                                  */
/*  - Flag the use of the MCU/DSP dual page read         */
/*    interface.                                         */
/*                                                       */
/* Input parameters:                                     */
/* -----------------                                     */
/* "task"                                                */
/*   NP, Normal paging reading task.                     */
/*   EP, Extended paging reading task.                   */
/*   BCCHS, BCCH Serving reading task.                   */
/*   ALLC, All serving cell CCCH reading task.           */
/*   SMSCB, Short Message Service Cell Broadcast task.   */
/*                                                       */
/* "burst_id"                                            */
/*   BURST_1, 1st burst of the task.                     */
/*   BURST_2, 2nd burst of the task.                     */
/*   BURST_3, 3rd burst of the task.                     */
/*   BURST_4, 4th burst of the task.                     */
/*                                                       */
/* Input parameters from globals:                        */
/* ------------------------------                        */
/* "l1a_l1s_com.task_param"                              */
/*   task semaphore bit register. Used to skip the body  */
/*   of this function if L1A has changed or is changing  */
/*   some of the task parameters.                        */
/*                                                       */
/* "l1a_l1s_com.l1s_en_task"                             */
/*   L1S task enable bit register.                       */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/* "l1s.task_status[task].current_status"                */
/*   current task status. It must be reset (INACTIVE)    */
/*   when the task is completed.                         */
/*   -> disactivate task.                                */
/*                                                       */
/* "l1s_dsp_com.dsp_r_page_used"                         */
/*   Flag used by the function which closes L1S          */
/*   execution ("l1s_end_manager()") to know if the      */
/*   MCU/DSP read page must be switched.                 */
/*   -> Set to 1.                                        */
/*                                                       */
/* Use of MCU/DSP interface:                             */
/* -------------------------                             */
/* "l1s_dsp_com.dsp_ndb_ptr"                             */
/*   pointer to the non double buffered part (NDB) of    */
/*   the MCU/DSP interface. This part is R/W for both    */
/*   DSP and MCU.                                        */
/*                                                       */
/* "l1s_dsp_com.dsp_db_r_ptr"                            */
/*   pointer to the double buffered part (DB) of the     */
/*   MCU/DSP interface. This pointer points to the READ  */
/*   page.                                               */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_read_snb_dl(UWORD8 task, UWORD8 burst_id)
{
  UWORD32   toa;
  UWORD32   pm;
  UWORD32   angle;
  UWORD32   snr;
  BOOL      en_task;
  BOOL      task_param;
  UWORD16   radio_freq=0;
  static UWORD16 pwr_level;
#if L1_FF_MULTIBAND == 1
  UWORD16  operative_radio_freq;
#endif
  
#if (FF_L1_FAST_DECODING == 1)
  UWORD8 skipped_bursts = 0;
  BOOL fast_decoding_authorized = l1s_check_fast_decoding_authorized(task);
  BOOL fast_decoded = (l1a_apihisr_com.fast_decoding.status == C_FAST_DECODING_COMPLETE);
  if (fast_decoded)
  {
    skipped_bursts = BURST_4 - burst_id;
  }
#endif /* if (FF_L1_FAST_DECODING == 1) */

  /*--------------------------------------------------------*/
  /* READ SERVING CELL RECEIVE TASK RESULTS...              */
  /*--------------------------------------------------------*/
  /* Rem: only a partial result is present in the mcu<-dsp  */
  /* communication buffer. The DATA BLOCK content itself is */
  /* in the last comm. (BURST_4)                               */
  /*--------------------------------------------------------*/
  // Get "enable" task flag and "synchro semaphore" for current task.
  en_task    = l1a_l1s_com.l1s_en_task[task];
  task_param = l1a_l1s_com.task_param[task];

  if((en_task) && !(task_param))
  // Check the task semaphore and the task enable bit. The reading
  // task body is executed only when the task semaphore is 0 and the
  // task is still enabled.
  // The semaphore can be set to 1 whenever L1A makes some changes
  // to the task parameters. The task can be disabled by L1A.
  {
    // Traces and debug.
    // ******************

    #if (TRACE_TYPE!=0) && (TRACE_TYPE!=5)
      trace_fct(CST_L1S_READ_SNB_DL , (UWORD32)(-1));//OMAPS00090550
    #endif

    #if (TRACE_TYPE!=0)
      #if L1_GPRS
         if (l1a_l1s_com.dsp_scheduler_mode == GSM_SCHEDULER)
         {
            // Check task identifier...
            if((UWORD32)(l1s_dsp_com.dsp_db_r_ptr->d_task_d & 0xffff) != (UWORD32)DSP_TASK_CODE[task])
              trace_fct(CST_DL_TASKS_DO_NOT_CORRESPOND, (UWORD32)(-1));//OMAPS00090550

            // Check burst identifier...
            if((UWORD32)(l1s_dsp_com.dsp_db_r_ptr->d_burst_d & 0xffff) != burst_id)
              trace_fct(CST_DL_BURST_DOES_NOT_CORRESPOND, (UWORD32)(-1));//OMAPS00090550
         }
         else // GPRS scheduler
         {
            // Check burst identifier...
            if(l1ps_dsp_com.pdsp_db_r_ptr->d_burst_nb_gprs != burst_id)
              trace_fct(CST_DL_BURST_DOES_NOT_CORRESPOND, (UWORD32)(-1));
         }
      #else
        // Check task identifier...
        if((UWORD32)(l1s_dsp_com.dsp_db_r_ptr->d_task_d & 0xffff) != (UWORD32)DSP_TASK_CODE[task])
          trace_fct(CST_DL_TASKS_DO_NOT_CORRESPOND, (UWORD32)(-1));

        // Check burst identifier...
        if((UWORD32)(l1s_dsp_com.dsp_db_r_ptr->d_burst_d & 0xffff) != burst_id)
          trace_fct(CST_DL_BURST_DOES_NOT_CORRESPOND,(UWORD32)( -1));
      #endif
    #endif

    l1_check_com_mismatch(task);

    // Read control results and feed control algorithms.
    // **************************************************

    // Read control information.
    #if L1_GPRS
      if (l1a_l1s_com.dsp_scheduler_mode == GSM_SCHEDULER)
      {
        toa   =  l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_TOA]   & 0xffff;
        pm    = (l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_PM]    & 0xffff) >> 5;
        angle =  l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_ANGLE] & 0xffff;
        snr   =  l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_SNR]   & 0xffff;
      }
      else
      {
        toa   =  l1ps_dsp_com.pdsp_db_r_ptr->a_burst_toa_gprs[0]   & 0xffff;
        pm    = (l1ps_dsp_com.pdsp_db_r_ptr->a_burst_pm_gprs[0]    & 0xffff) >> 5;
        angle =  l1ps_dsp_com.pdsp_db_r_ptr->a_burst_angle_gprs[0] & 0xffff;
        snr   =  l1ps_dsp_com.pdsp_db_r_ptr->a_burst_snr_gprs[0]   & 0xffff;
      }
    #else
        toa   =  l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_TOA]   & 0xffff;
        pm    = (l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_PM]    & 0xffff) >> 5;
        angle =  l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_ANGLE] & 0xffff;
        snr   =  l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_SNR]   & 0xffff;
    #endif

    l1_check_pm_error(pm, task);

    // Update AGC: Call PAGC algorithm
    radio_freq = l1a_l1s_com.Scell_info.radio_freq;

#if (L1_FF_MULTIBAND == 0)

    l1a_l1s_com.Scell_IL_for_rxlev = l1ctl_pagc((UWORD8)pm, radio_freq,
                                                &l1a_l1s_com.last_input_level[radio_freq - l1_config.std.radio_freq_index_offset]);
#else // L1_FF_MULTIBAND = 1 below

  operative_radio_freq = 
    l1_multiband_radio_freq_convert_into_operative_radio_freq(radio_freq);
  l1a_l1s_com.Scell_IL_for_rxlev = l1ctl_pagc((UWORD8)pm, radio_freq,
                                                &l1a_l1s_com.last_input_level[operative_radio_freq]);  

#endif // #if (L1_FF_MULTIBAND == 0) else


#if (FF_L1_FAST_DECODING == 1)
    if (skipped_bursts>0)
    {
      l1ctl_pagc_missing_bursts(skipped_bursts);
    }
#endif /* if (FF_L1_FAST_DECODING == 1) */

    #if L2_L3_SIMUL
      #if (DEBUG_TRACE == BUFFER_TRACE_LNA)
      
        #if (L1_FF_MULTIBAND == 0)
            buffer_trace (4, 33, radio_freq,
                      l1a_l1s_com.last_input_level[radio_freq - l1_config.std.radio_freq_index_offset].input_level,
                      l1a_l1s_com.last_input_level[radio_freq - l1_config.std.radio_freq_index_offset].lna_off);
        
        #else // L1_FF_MULTIBAND = 1 below
        
            buffer_trace (4, 33, radio_freq,
                      l1a_l1s_com.last_input_level[operative_radio_freq].input_level,
                      l1a_l1s_com.last_input_level[operative_radio_freq].lna_off);
        
        #endif // #if (L1_FF_MULTIBAND == 0) else 
        
      #endif
    #endif

    #if TRACE_TYPE==3
      stats_samples_nb(toa,pm,angle,snr,burst_id,task);
    #endif

    // Update AFC: Call AFC control function (KALMAN filter).
    #if AFC_ALGO
      #if TESTMODE
        if (l1_config.afc_enable)
      #endif
        {
		  #if L2_L3_SIMUL
          #if (DEBUG_TRACE == BUFFER_TRACE_AFC_OPEN)//omaps00090550
          WORD16 old_afc  = l1s.afc;
          WORD16 old_count= l1s.afc_frame_count;
          #endif
          #endif



          #if (VCXO_ALGO == 0)
            l1s.afc = l1ctl_afc(AFC_CLOSED_LOOP, &l1s.afc_frame_count, (WORD16)angle, snr, radio_freq);
          #else
            l1s.afc = l1ctl_afc(AFC_CLOSED_LOOP, &l1s.afc_frame_count, (WORD16)angle, snr, radio_freq,l1a_l1s_com.mode);
          #endif

          #if L2_L3_SIMUL
            #if (DEBUG_TRACE == BUFFER_TRACE_AFC_OPEN)
              buffer_trace (4,(WORD16)angle,old_count,old_afc,l1s.afc);
            #endif
            #if (DEBUG_TRACE == BUFFER_TRACE_TOA)
             if (task == NP || task == EP)
               buffer_trace(5,
                           l1s.debug_time,
                           0xf1,
                           i,
                           l1s.afc,
                           angle );
            #endif
          #endif
        }
    #endif

    //Feed TOA histogram.
    #if (TOA_ALGO != 0)
    if (task != SMSCB)
   {
      #if (TOA_ALGO == 2)
        if(l1s.toa_var.toa_snr_mask == 0)
      #else
        if(l1s.toa_snr_mask == 0)
      #endif
      #if (RF_FAM == 2) // RF 2
        #if (TOA_ALGO == 2)
			if(l1a_l1s_com.Scell_IL_for_rxlev < IL_FOR_RXLEV_SNR)
			{
           l1s.toa_var.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, snr, toa);
			}
			else
			{
				l1s.toa_var.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, 0, toa);
			}
        #else
			if(l1a_l1s_com.Scell_IL_for_rxlev <IL_FOR_RXLEV_SNR)
            {
           l1s.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, snr, toa, &l1s.toa_update, &l1s.toa_period_count
#if (FF_L1_FAST_DECODING == 1)
               , skipped_bursts
#endif
               );
			}
			else
			{
                l1s.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, 0, toa, &l1s.toa_update, &l1s.toa_period_count
#if (FF_L1_FAST_DECODING == 1)
                    , skipped_bursts
#endif
                    );
			}
        #endif

        #if L2_L3_SIMUL
          #if (DEBUG_TRACE == BUFFER_TRACE_TOA )
           if (task == NP || task == EP)
             buffer_trace(5,
                     l1s.debug_time,
                     0xf0,
                     toa,
                     snr,
                     l1s.tpu_offset );
          #endif
        #endif
      #else // RF 2
        #if (TOA_ALGO == 2)
			if(l1a_l1s_com.Scell_IL_for_rxlev < IL_FOR_RXLEV_SNR)
			{
           l1s.toa_var.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, snr, toa);
			}
			else
			{
				l1s.toa_var.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, 0, toa);
			}
        #else
			if(l1a_l1s_com.Scell_IL_for_rxlev <IL_FOR_RXLEV_SNR)
            {
           l1s.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, snr, toa, &l1s.toa_update, &l1s.toa_period_count
#if (FF_L1_FAST_DECODING == 1)
               , skipped_bursts
#endif
               );
			}
			else
			{
                l1s.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, 0, toa, &l1s.toa_update, &l1s.toa_period_count
#if (FF_L1_FAST_DECODING ==1)
                    , skipped_bursts
#endif
                    );
			}
        #endif
      #endif // RF 2
   }
  #else  // TOA_ALGO
    #if L2_L3_SIMUL
      #if (DEBUG_TRACE == BUFFER_TRACE_TOA)
        if (task == NP || task == EP)
             buffer_trace(5,
                     l1s.debug_time,
                     0xf0,
                     toa,
                     snr,
                     l1s.tpu_offset );
      #endif
    #endif
  #endif // TOA_ALGO

    #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
      RTTL1_FILL_DL_BURST(angle, snr, l1s.afc, task, pm, toa, l1a_l1s_com.Scell_IL_for_rxlev)
    #endif
    #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4)) && HAVE_L1_TRACE_BURST_PARAM
      l1_trace_burst_param(angle, snr, l1s.afc, task, pm, toa, l1a_l1s_com.Scell_IL_for_rxlev);
    #endif
    #if (BURST_PARAM_LOG_ENABLE == 1)
      l1_log_burst_param(angle, snr, l1s.afc, task, pm, toa, l1a_l1s_com.Scell_IL_for_rxlev);
    #endif

    // compute the Data bloc Power.
    // ******************************
    if(burst_id == BURST_1)
      pwr_level = 0;

    // add the burst power
    pwr_level += l1a_l1s_com.Scell_IL_for_rxlev;


    // Read downlink DATA block from MCU/DSP interface.

    if (task == NP)
    {
          toa_tab[burst_id] = toa;
  }

#if 0	/* FreeCalypso TCS211 reconstruction */
// added Enhanced RSSI
   if(l1s_dsp_com.dsp_ndb_ptr->a_cd[2] != 0xffff)
   {
        qual_acc_idle1[0]  += l1s_dsp_com.dsp_ndb_ptr->a_cd[2];
        //RX Qual value reporting- total number of decoded bits
         qual_acc_idle1[1] += 1;
   }
#endif

#if (FF_L1_FAST_DECODING == 1)
    /* Perform the reporting if
        - Burst is the 4th one (whether CRC is ok or not)
        - Fast decoding enabled and CRC already ok
    */
    if ( (burst_id == BURST_4) || fast_decoded )
#else /* #if (FF_L1_FAST_DECODING == 1) */
    if(burst_id == BURST_4)
#endif /* FF_L1_FAST_DECODING */
    {
      UWORD8 i;

      #if (TRACE_TYPE==2 ) || (TRACE_TYPE==3)
        uart_trace(task);
      #endif

      // the data power bloc = pwr_level/4.
#if (FF_L1_FAST_DECODING == 1)
      /* Data power block = pwr_level / (nb of bursts)*/
      pwr_level = pwr_level / (burst_id + 1);
#else /* #if (FF_L1_FAST_DECODING == 1) */
      // the data power bloc = pwr_level/4.
      pwr_level = pwr_level >> 2;
#endif /* #if (FF_L1_FAST_DECODING == 1) #else*/

#if (FF_L1_FAST_DECODING == 1)
      if(!fast_decoding_authorized)
      {
        /* When fast decoding wasn't used, burst_id is undefined (for the trace) */
        l1a_l1s_com.last_fast_decoding = 0;
      }
      else
      {
        l1a_l1s_com.last_fast_decoding = burst_id + 1;
      }
#endif /* #if (FF_L1_FAST_DECODING == 1) */

      // Read L3 frame block and send msg to L1A.
      #if L1_GPRS
        if (l1a_l1s_com.dsp_scheduler_mode == GSM_SCHEDULER)
          l1s_read_l3frm(pwr_level,&(l1s_dsp_com.dsp_ndb_ptr->a_cd[0]), task);
        else
          l1s_read_l3frm(pwr_level,&(l1ps_dsp_com.pdsp_ndb_ptr->a_dd_gprs[0][0]), task);
      #else
          l1s_read_l3frm(pwr_level,&(l1s_dsp_com.dsp_ndb_ptr->a_cd[0]), task);
      #endif

      #if L1_GPRS
        if (l1a_l1s_com.dsp_scheduler_mode == GSM_SCHEDULER)
      #endif
      {
        // reset buffers and flags in NDB ...
        // reset nerr....
        // reset A_CD contents.......
        l1s_dsp_com.dsp_ndb_ptr->a_cd[2]     =  0xffff;
        for (i=0;i<12;i++)
          l1s_dsp_com.dsp_ndb_ptr->a_cd[3+i] =  0x0000;
      }

    } // End if...
  } // End if...

  // The NP/EP task was enabled and could cancel a PTCCH burst
  // This incomplete PTCCH decoding block cause DSP troubles and so COM/PM errors
  // and then a recovery  => in this case restart the PTCCH from the burst 0
#if L1_GPRS
  if((task == NP)||(task == EP))
    if(l1a_l1s_com.l1s_en_task[PTCCH] == TASK_ENABLED)
      if(l1pa_l1ps_com.transfer.ptcch.activity & PTCCH_DL) // a PTCCH DL task is running
        if((l1s.actual_time.t2 >= 13) && (l1s.actual_time.t2 <= 17))  // only if the NP/EP remove a PTCCH activity
        {
          // Restart PTCCH DL task from the begining (i.e BURST 0).
          l1pa_l1ps_com.transfer.ptcch.activity ^= PTCCH_DL;  // disable PTCCH_DL activity running
          l1pa_l1ps_com.transfer.ptcch.request_dl = TRUE;     // restart PTCCH DL from the Burst0
        }
#endif

  // Deactivate task.
  // ******************

  // End of task -> task must become INACTIVE.
  // Rem: some TASKS (ALLC) can be pipelined and therefore must stay active if
  // they have already reentered the flow.
#if (FF_L1_FAST_DECODING == 1)
  if ( (burst_id == BURST_4) || fast_decoded )
#else /* #if (FF_L1_FAST_DECODING == 1) */
  if(burst_id == BURST_4)
#endif /* #if (FF_L1_FAST_DECODING == 1) #else*/

  {
#if (FF_L1_FAST_DECODING == 1)
    if((task == NP) || (task == NBCCHS))
    {
      if (l1a_apihisr_com.fast_decoding.contiguous_decoding == TRUE)
      {
        /* A new block has started, a new fast API IT is expected */
        l1a_apihisr_com.fast_decoding.contiguous_decoding = FALSE;
        l1a_apihisr_com.fast_decoding.status = C_FAST_DECODING_AWAITED;
      }
      else if(task == l1a_apihisr_com.fast_decoding.task)
      {
        /* Reset decoding status */
        l1a_apihisr_com.fast_decoding.status = C_FAST_DECODING_NONE;
      }
    }  /* end if tsk == NP */
#endif /* #if (FF_L1_FAST_DECODING == 1) */    
    if(l1s.task_status[task].current_status == RE_ENTERED)
      l1s.task_status[task].current_status = ACTIVE;
    else
      l1s.task_status[task].current_status = INACTIVE;
#if (FF_L1_FAST_DECODING == 1)
    if (burst_id != BURST_4)
    {
      /* Successful decode before the 4th burst, no other control/read activities are needed */
      l1s_clean_mftab(task, burst_id + 3);
      if(l1s.frame_count == (4 -burst_id))
      {
          l1s.frame_count = 1;
      }
    }
#endif /* #if (FF_L1_FAST_DECODING == 1) */
  }

  #if (L1_DEBUG_IQ_DUMP == 1)
    l1ddsp_read_iq_dump(task);
  #endif

  // Flag the use of the MCU/DSP dual page read interface.
  // ******************************************************

  // Set flag used to change the read page at the end of "l1_synch".
  l1s_dsp_com.dsp_r_page_used = TRUE;
}


//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END
#endif

/*-------------------------------------------------------*/
/* l1s_read_nnb()                                        */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* tasks: BCCHN.                                         */
/* This function is the reading result function used for */
/* reading a neighbor cell block acquisition result in   */
/* idle mode. Here is a summary of the execution:        */
/*                                                       */
/*  - If SEMAPHORE(task) is low and task still enabled.  */
/*      - Traces and debug.                              */
/*      - Read DL DATA block from MCU/DSP interface.     */
/*  - Disactivate task.                                  */
/*  - Flag the use of the MCU/DSP dual page read         */
/*    interface.                                         */
/*                                                       */
/* Input parameters:                                     */
/* -----------------                                     */
/* "task"                                                */
/*   BCCHN, BCCH Neighbor reading task.                  */
/*                                                       */
/* Input parameters from globals:                        */
/* ------------------------------                        */
/* "l1a_l1s_com.task_param"                              */
/*   task semaphore bit register. Used to skip the body  */
/*   of this function if L1A has changed or is changing  */
/*   some of the task parameters.                        */
/*                                                       */
/* "l1a_l1s_com.l1s_en_task"                             */
/*   L1S task enable bit register.                       */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/* "l1s.task_status[task].current_status"                */
/*   current task status. It must be reset (INACTIVE)    */
/*   when the task is completed.                         */
/*   -> disactivate task.                                */
/*                                                       */
/* "l1s_dsp_com.dsp_r_page_used"                         */
/*   Flag used by the function which closes L1S          */
/*   execution ("l1s_end_manager()") to know if the      */
/*   MCU/DSP read page must be switched.                 */
/*   -> Set to 1.                                        */
/*                                                       */
/* Use of MCU/DSP interface:                             */
/* -------------------------                             */
/* "l1s_dsp_com.dsp_ndb_ptr"                             */
/*   pointer to the non double buffered part (NDB) of    */
/*   the MCU/DSP interface. This part is R/W for both    */
/*   DSP and MCU.                                        */
/*                                                       */
/* "l1s_dsp_com.dsp_db_r_ptr"                            */
/*   pointer to the double buffered part (DB) of the     */
/*   MCU/DSP interface. This pointer points to the READ  */
/*   page.                                               */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_read_nnb(UWORD8 task, UWORD8 param)
{
  BOOL     en_task;
  BOOL     task_param;
  UWORD16  neigh_radio_freq;
  UWORD16  pwr_level;
  UWORD8   active_neigh_id;
#if (L1_FF_MULTIBAND == 1)
 UWORD16 operative_radio_freq;
#endif


  /*--------------------------------------------------------*/
  /* READ NEIGBOR CELL RECEIVE TASK RESULTS...              */
  /*--------------------------------------------------------*/
  /* Rem: the full result is present in the mcu<-dsp        */
  /* communication buffer.                                  */
  /*--------------------------------------------------------*/
  // Get "enable" task flag and "synchro semaphore" for current task.
  en_task    = l1a_l1s_com.l1s_en_task[task];
  task_param = l1a_l1s_com.task_param[task];

  if((en_task) && !(task_param))
  // Check the task semaphore and the task enable bit. The reading
  // task body is executed only when the task semaphore is 0 and the
  // task is still enabled.
  // The semaphore can be set to 1 whenever L1A makes some changes
  // to the task parameters. The task can be disabled by L1A.
  {
    // Traces and debug.
    // ******************

    #if (TRACE_TYPE!=0) && (TRACE_TYPE!=5)
      trace_fct(CST_L1S_READ_NNB ,(UWORD32)(-1));
    #endif


    #if (TRACE_TYPE!=0)
      // Check task identifier...
      #if L1_GPRS
        switch(l1a_l1s_com.dsp_scheduler_mode)
        {
          case GSM_SCHEDULER:
          {
            if((UWORD32)(l1s_dsp_com.dsp_db_r_ptr->d_task_d & 0xffff) != (UWORD32)DSP_TASK_CODE[task])
              trace_fct(CST_DL_TASKS_DO_NOT_CORRESPOND, (UWORD32)(-1));
          } break;
          case GPRS_SCHEDULER:
          {
            if((UWORD32)(l1s_dsp_com.dsp_db_r_ptr->d_task_md & 0xffff) != (UWORD32)DSP_TASK_CODE[task])
              trace_fct(CST_DL_TASKS_DO_NOT_CORRESPOND, (UWORD32)(-1));
          } break;
        }
      #else
        if((UWORD32)(l1s_dsp_com.dsp_db_r_ptr->d_task_d & 0xffff) != (UWORD32)DSP_TASK_CODE[task])
          trace_fct(CST_DL_TASKS_DO_NOT_CORRESPOND, (UWORD32)(-1));
      #endif
    #endif

    l1_check_com_mismatch(task);

    #if (TRACE_TYPE==2 ) || (TRACE_TYPE==3)
      uart_trace(task);
    #endif

    if(task == BCCHN)
      active_neigh_id = l1a_l1s_com.bcchn.active_neigh_id_norm;
    else // BCCHN_TRAN and BCCHN_TOP tasks
      active_neigh_id = l1a_l1s_com.bcchn.active_neigh_id_top;

    // the mean power level is impossible for the neighbor bloc, so the las input level is used.
    neigh_radio_freq = l1a_l1s_com.bcchn.list[active_neigh_id].radio_freq;
#if 1	/* FreeCalypso TCS211 reconstruction */
    pwr_level = l1a_l1s_com.last_input_level[neigh_radio_freq].input_level;
#elif (L1_FF_MULTIBAND == 0)
    pwr_level = l1a_l1s_com.last_input_level[neigh_radio_freq - l1_config.std.radio_freq_index_offset].input_level;
#else
    operative_radio_freq = l1_multiband_radio_freq_convert_into_operative_radio_freq(neigh_radio_freq);
    pwr_level = l1a_l1s_com.last_input_level[operative_radio_freq].input_level; 
#endif



    // Read downlink DATA block from MCU/DSP interface.
    // *************************************************

    // Read L3 frame block and send msg to L1A.
    #if L1_GPRS
      if (l1a_l1s_com.dsp_scheduler_mode == GSM_SCHEDULER)
        l1s_read_l3frm(pwr_level,&(l1s_dsp_com.dsp_ndb_ptr->a_cd[0]), task);
      else
        l1s_read_l3frm(pwr_level,&(l1ps_dsp_com.pdsp_ndb_ptr->a_dd_gprs[0][0]), task);
    #else
        l1s_read_l3frm(pwr_level,&(l1s_dsp_com.dsp_ndb_ptr->a_cd[0]), task);
    #endif
    // Disable the served TC from the TC bitmap.
    if(task == BCCHN)
      l1a_l1s_com.bcchn.list[active_neigh_id].bcch_blks_req ^=
        ((UWORD16)(1L << l1a_l1s_com.bcchn.active_neigh_tc_norm));
    else // BCCHN_TRAN and BCCHN_TOP tasks
      l1a_l1s_com.bcchn.list[active_neigh_id].bcch_blks_req ^=
        ((UWORD16)(1L << l1a_l1s_com.bcchn.active_neigh_tc_top));
  }

  // The BCCHN task was enabled and could cancel a PTCCH burst
  // This incomplete PTCCH decoding block cause DSP troubles and so COM/PM errors
  // and then a recovery (seen with ULYSS) => in this case restart the PTCCH from the burst 0
  #if L1_GPRS
    if (task == BCCHN_TRAN)
    if(l1a_l1s_com.l1s_en_task[PTCCH] == TASK_ENABLED)
    if(l1pa_l1ps_com.transfer.ptcch.activity & PTCCH_DL) // a PTCCH DL task is running
    if ((l1s.actual_time.t2 >= 13) && (l1s.actual_time.t2 <= 18))  // only if the BCCHN remove a PTCCH activity
    {
      // Restart PTCCH DL task from the begining (i.e BURST 0).
      l1pa_l1ps_com.transfer.ptcch.activity ^= PTCCH_DL;  // disable PTCCH_DL activity running
      l1pa_l1ps_com.transfer.ptcch.request_dl = TRUE;     // restart PTCCH DL from the Burst0
     }
  #endif

  // Disactivate task.
  // ******************

  // End of task -> task must become INACTIVE.
  l1s.task_status[task].current_status = INACTIVE;

  #if (L1_DEBUG_IQ_DUMP == 1)
    l1ddsp_read_iq_dump(task);
  #endif

  // Flag the use of the MCU/DSP dual page read interface.
  // ******************************************************

  // Set flag used to change the read page at the end of "l1_synch".
  l1s_dsp_com.dsp_r_page_used = TRUE;
}

/*-------------------------------------------------------*/
/* l1s_read_dedic_dl()                                   */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a "COMPLEX" function used by the L1S */
/* tasks: DDL,ADL,TCHTH,TCHTF,TCHA.                      */
/* This function is the reading result function used for */
/* dedicated mode. Here is a summary of the execution:   */
/*                                                       */
/*  - Traces and debug.                                  */
/*  - Read control results and feed control algo.        */
/*  - Read DL DATA block from MCU/DSP interface.         */
/*  - Flag the use of the MCU/DSP dual page read         */
/*    interface.                                         */
/*                                                       */
/* Input parameters:                                     */
/* -----------------                                     */
/* "task"                                                */
/*   DDL, SDCCH DOWNLINK reading task.                   */
/*   ADL, SACCH DOWNLINK (associated with SDCCH)reading  */
/*   task.                                               */
/*   TCHTH, TCH channel task when dedicated/TCH Half rate*/
/*   TCHTF, TCH channel task when dedicated/TCH Full rate*/
/*   TCHA, Associated channel task when dedicated/TCH.   */
/*                                                       */
/* "burst_id" (used only by DDL/ADL tasks).              */
/*   BURST_1, 1st burst of the task.                     */
/*   BURST_2, 2nd burst of the task.                     */
/*   BURST_3, 3rd burst of the task.                     */
/*   BURST_4, 4th burst of the task.                     */
/*                                                       */
/* Input parameters from globals:                        */
/* ------------------------------                        */
/* "l1a_l1s_com.task_param"                              */
/*   task semaphore bit register. Used to skip the body  */
/*   of this function if L1A has changed or is changing  */
/*   some of the task parameters.                        */
/*                                                       */
/* "l1a_l1s_com.l1s_en_task"                             */
/*   L1S task enable bit register.                       */
/*                                                       */
/* Modified parameters from globals:                     */
/* ---------------------------------                     */
/* "l1s.task_status[task].current_status"                */
/*   current task status. It must be reset (INACTIVE)    */
/*   when the task is completed.                         */
/*   -> disactivate task.                                */
/*                                                       */
/* "l1s_dsp_com.dsp_r_page_used"                         */
/*   Flag used by the function which closes L1S          */
/*   execution ("l1s_end_manager()") to know if the      */
/*   MCU/DSP read page must be switched.                 */
/*   -> Set to 1.                                        */
/*                                                       */
/* Use of MCU/DSP interface:                             */
/* -------------------------                             */
/* "l1s_dsp_com.dsp_ndb_ptr"                             */
/*   pointer to the non double buffered part (NDB) of    */
/*   the MCU/DSP interface. This part is R/W for both    */
/*   DSP and MCU.                                        */
/*                                                       */
/* "l1s_dsp_com.dsp_db_r_ptr"                            */
/*   pointer to the double buffered part (DB) of the     */
/*   MCU/DSP interface. This pointer points to the READ  */
/*   page.                                               */
/*                                                       */
/* RXQUAL :                                              */
/* 1) SDCCH : for RXQUAL_FULL and RXQUAL_SUB we accumu-  */
/*            -late number of estimated errors (a_cd[2]) */
/*            for ALL SACCH and SDCCH TDMA frames.       */
/* 2) TCH   : for RXQUAL_FULL in TCH_FS_MODE and         */
/*            TCH_24F_MODE, we accumulate number of      */
/*            estimated errors for ALL FACCH (a_fd[2])   */
/*            TDMA frames and ALL speech (a_dd_0[2])     */
/*            TDMA frames.                               */
/*            for RXQUAL_FULL in all data modes (except  */
/*            TCH_24F_MODE, see above) we accumulate     */
/*            number of errors for ALL FACCH (a_fd[2])   */
/*            TDMA frames and ALL data (a_dd_0[2])       */
/*            TDMA frames.                               */
/*            for RXQUAL_SUB in TCH_FS_MODE and          */
/*            TCH_24F_MODE, we only accumulate number of */
/*            estimated errors for FACCH (a_fd[2]) TDMA  */
/*            frames and speech (a_dd_0[2]) TDMA frames  */
/*            at SID block boundary position.            */
/*            for RXQUAL_SUB in all data modes (except   */
/*            TCH_24F_MODE, see above) we only accumulate*/
/*            number of estimated errors for FACCH       */
/*            (a_fd[2]) TDMA frames at SID block boundary*/
/*            position. The GSM specification 5.08 $8.4  */
/*            is not clear about data block at SID block */
/*            boundary position. Do we need to accumulate*/
/*            if L2/fill frame at this SID block boundary*/
/*            position.                                  */
/* Note: before accumulating FACCH TDMA frame we only    */
/*       check b_blud value, we don't mind about b_fire. */
/*-------------------------------------------------------*/
void l1s_read_dedic_dl(UWORD8 task, UWORD8 burst_id)
{
  UWORD32         toa;
  UWORD32         pm;
  UWORD32         angle;
  UWORD32         snr;
  BOOL            beacon;
  T_INPUT_LEVEL  *IL_info_ptr;
  UWORD16         radio_freq=0;

  #if TESTMODE
    UWORD32 pm_fullres =0;//omaps00090550
  #endif


  #if REL99
  #if FF_EMR
    T_EMR_PARAMS emr_params;    // strucutre to store pre-calculated parameter

    /*--------------------------------------------------------*/
    /* INITIALIZATION OF EMR params..                         */
    /*--------------------------------------------------------*/

    emr_params.task             = task;
    emr_params.burst_id         = burst_id;
    emr_params.facch_present    = (l1s_dsp_com.dsp_ndb_ptr->a_fd[0] & (1<<B_BLUD)) >> B_BLUD;
    emr_params.facch_fire1      = (l1s_dsp_com.dsp_ndb_ptr->a_fd[0] & (1<<B_FIRE1)) >> B_FIRE1;
    emr_params.a_dd_0_blud      = (l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] & (1<<B_BLUD)) >> B_BLUD;
    emr_params.a_dd_0_bfi       = ((l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0])&(1<<B_BFI)) >> B_BFI; // 3rd bit tells the BAD frame
    emr_params.a_dd_1_blud      = (l1s_dsp_com.dsp_ndb_ptr->a_dd_1[0] & (1<<B_BLUD)) >> B_BLUD;
    emr_params.a_dd_1_bfi       = ((l1s_dsp_com.dsp_ndb_ptr->a_dd_1[0])&(1<<B_BFI)) >> B_BFI; // 3rd bit tells the BAD frame
    emr_params.b_m1             = ((l1s_dsp_com.dsp_ndb_ptr->a_data_buf_dl[1]) &(1<<B_M1)) >> B_M1; // = 1 if second half frame for data 14.4
    emr_params.b_f48blk_dl      = ((l1s_dsp_com.dsp_ndb_ptr->d_ra_act) &(1<<B_F48BLK_DL)) >> B_F48BLK_DL; // = 1 if second half frame for data 4.8
    emr_params.b_ce             = (((l1s_dsp_com.dsp_ndb_ptr->d_ra_conf) & (1<<B_CE)) >> B_CE);
    emr_params.a_ntd            = (((l1s_dsp_com.dsp_ndb_ptr->a_data_buf_dl[1]) & (1<<B_FCS_OK)) >> B_FCS_OK);
    emr_params.a_cd_fire1       = (l1s_dsp_com.dsp_ndb_ptr->a_cd[0] & (1<<B_FIRE1)) >> B_FIRE1;
    emr_params.sid_present_sub0 = (l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] & (1<<B_SID1)) >> B_SID1; // find out whether sid1 is 0/1
    emr_params.sid_present_sub1 = (l1s_dsp_com.dsp_ndb_ptr->a_dd_1[0] & (1<<B_SID1)) >> B_SID1; // find out whether sid1 is 0/1
    #if (AMR == 1)
      emr_params.amr_facch_present= (l1s_dsp_com.dsp_ndb_ptr->a_fd[0] & (1<<B_BLUD)) >> B_BLUD;
      emr_params.amr_facch_fire1  = (l1s_dsp_com.dsp_ndb_ptr->a_fd[0] & (1<<B_FIRE1)) >> B_FIRE1;
      emr_params.b_ratscch_blud   = (l1s_dsp_com.dsp_ndb_ptr->a_ratscch_dl[0] & (1<<B_BLUD)) >> B_BLUD;
      emr_params.ratscch_rxtype   = (l1s_dsp_com.dsp_ndb_ptr->a_ratscch_dl[0] & RX_TYPE_MASK) >> RX_TYPE_SHIFT;
      emr_params.amr_rx_type_sub0 = (l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] & RX_TYPE_MASK) >> RX_TYPE_SHIFT;
      emr_params.amr_rx_type_sub1 = (l1s_dsp_com.dsp_ndb_ptr->a_dd_1[0] & RX_TYPE_MASK) >> RX_TYPE_SHIFT;
    #endif //(AMR == 1)
  #endif //FF_EMR
  #endif //REL99



  /*--------------------------------------------------------*/
  /* READ DEDICATED CHANNEL DL RESULTS...                   */
  /*--------------------------------------------------------*/

  // Traces and debug.
  // ******************
  #if (TRACE_TYPE!=0) && (TRACE_TYPE !=5)
      trace_fct(CST_L1S_READ_DEDIC_DL, -1);
  #endif

  #if (TRACE_TYPE!=0)
    // Check task identifier...
    if((UWORD32)(l1s_dsp_com.dsp_db_r_ptr->d_task_d & 0xffff) != (UWORD32)DSP_TASK_CODE[task])
      trace_fct(CST_DL_TASKS_DO_NOT_CORRESPOND, (UWORD32)(-1));
  #endif

    #if (TESTMODE)
      // WARNING!
      // Don't trace MCU-DSP mismatches during UL-only in TestMode. The DSP is not working
      // in that case so it is normal. However, if tracing happens the CPU overloads
      if (l1_config.TestMode && l1_config.tmode.rf_params.down_up & TMODE_DOWNLINK)
    #endif
      {
          l1_check_com_mismatch(task);
      }

  radio_freq = l1a_l1s_com.dedic_set.radio_freq_dd;

  if (radio_freq == l1a_l1s_com.Scell_info.radio_freq)
  {
    beacon=1;
    IL_info_ptr = &l1a_l1s_com.Scell_info.traffic_meas_beacon;
  }
  else
  {
    beacon=0;
    IL_info_ptr = &l1a_l1s_com.Scell_info.traffic_meas;
  }

  #if (AMR == 1)
  {
    // RATSCCH detection
    UWORD16 ratscch_dl_header=l1s_dsp_com.dsp_ndb_ptr->a_ratscch_dl[0];
    UWORD16 b_ratscch_dl_blud = (ratscch_dl_header & (1<<B_BLUD)) >> B_BLUD;

    if(b_ratscch_dl_blud==TRUE)
    {
      UWORD8 rx_type = (ratscch_dl_header & RX_TYPE_MASK) >> RX_TYPE_SHIFT;

      if(rx_type==C_RATSCCH_GOOD)
      {
        // RATSCCH block detected
        l1s_amr_update_from_ratscch(&l1s_dsp_com.dsp_ndb_ptr->a_ratscch_dl[0]);
      }
    }
  }
  #endif    // AMR

  switch(task)
  {
    case DDL :
    case ADL :
    /*---------------------------------------------------*/
    /* Dedicated mode: SDCCH receive task.               */
    /* Rem: only a partial result is present in the      */
    /* mcu<-dsp communication buffer. The BLOCK content  */
    /* itself is in the last comm. (BURST_4)             */
    /*---------------------------------------------------*/
    {
      UWORD8    i, IL_for_rxlev;

      #if (TRACE_TYPE!=0)
        // Check burst identifier...
        if((UWORD32)(l1s_dsp_com.dsp_db_r_ptr->d_burst_d & 0xffff) != burst_id)
          trace_fct(CST_DL_BURST_DOES_NOT_CORRESPOND, (UWORD32)(-1));
      #endif

      // Read control results and feed control algorithms.
      // **************************************************

      // Read control information.
      toa   = l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_TOA]   & 0xffff;
      pm    = (l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_PM]   & 0xffff) >> 5;
      angle = l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_ANGLE] & 0xffff;
      snr   = l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_SNR]   & 0xffff;

      l1_check_pm_error(pm, task);

      // Update AGC: Call DPAGC algorithm
      IL_for_rxlev = l1ctl_dpagc(0,beacon,(UWORD8)pm,radio_freq,IL_info_ptr); // dtx_on = 0

      // Dedicated mode serving cell measurement reading.
      #if REL99
      #if FF_EMR
        // only task,burst_id is valid in structure pointed by *emr_params
        l1s_read_dedic_scell_meas(IL_for_rxlev, 1, &emr_params);
      #endif
      #else
        l1s_read_dedic_scell_meas(IL_for_rxlev, 1);
      #endif

      #if TRACE_TYPE==3
        stats_samples_nb(toa,pm,angle,snr,burst_id,task);
      #endif

      // Update AFC: Call AFC control function (KALMAN filter).
      #if AFC_ALGO
        #if TESTMODE
          if (l1_config.afc_enable)
        #endif
          {
            #if (VCXO_ALGO == 0)
              l1s.afc = l1ctl_afc(AFC_CLOSED_LOOP, &l1s.afc_frame_count, (WORD16)angle, snr, radio_freq);
            #else
              l1s.afc = l1ctl_afc(AFC_CLOSED_LOOP, &l1s.afc_frame_count, (WORD16)angle, snr, radio_freq,l1a_l1s_com.mode);
            #endif
          }
      #endif

      //Feed TOA histogram.
      #if (TOA_ALGO != 0)
          #if (TOA_ALGO == 2)
            if(l1s.toa_var.toa_snr_mask == 0)
          #else
            if(l1s.toa_snr_mask == 0)
          #endif
          {
            #if 1	/* FreeCalypso TCS211 reconstruction */
              if (IL_for_rxlev < IL_FOR_RXLEV_SNR)
                l1s.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, snr, toa, &l1s.toa_update, &l1s.toa_period_count);
              else
                l1s.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, 0, toa, &l1s.toa_update, &l1s.toa_period_count);
            #else
              UWORD32 snr_temp;
              snr_temp = (IL_for_rxlev < IL_FOR_RXLEV_SNR)? snr: 0;
              #if (TOA_ALGO == 2)
                l1s.toa_var.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, snr_temp, toa);
              #else
                l1s.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, snr_temp, toa, &l1s.toa_update, &l1s.toa_period_count
                #if (FF_L1_FAST_DECODING ==1)
                    ,0
                #endif
                    );
              #endif
            #endif
          }
      #endif

       #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
         RTTL1_FILL_DL_BURST(angle, snr, l1s.afc, task, pm, toa, IL_for_rxlev)
       #endif
       #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4)) && HAVE_L1_TRACE_BURST_PARAM
         l1_trace_burst_param(angle, snr, l1s.afc, task, pm, toa, IL_for_rxlev);
       #endif
       #if (BURST_PARAM_LOG_ENABLE == 1)
         l1_log_burst_param(angle, snr, l1s.afc, task, pm, toa, IL_for_rxlev);
       #endif
      // Read downlink DATA block from MCU/DSP interface.
      // *************************************************

      if(burst_id == BURST_4)
      {
        #if (TRACE_TYPE==2 ) || (TRACE_TYPE==3)
          uart_trace(task);
        #endif

        if(task == DDL)
        {
          // Read DCCH DL data block from DSP, pass it to L2.
          l1s_read_dcch_dl(l1s_dsp_com.dsp_ndb_ptr->a_cd, task);
        }
        else
        {
          // Read L2 frame block and send msg to L1A.
          l1s_read_sacch_dl(l1s_dsp_com.dsp_ndb_ptr->a_cd, task);
        }

        // RXQUAL_FULL/RXQUAL_SUB : number of estimated errors, this value is contained
        // in a_cd[2] field, for every SACCH and SDDCH blocks
        l1a_l1s_com.Smeas_dedic.qual_acc_full      += l1s_dsp_com.dsp_ndb_ptr->a_cd[2]&0xffff;
        l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full += A_D_BLEN;
        l1a_l1s_com.Smeas_dedic.qual_acc_sub       += l1s_dsp_com.dsp_ndb_ptr->a_cd[2]&0xffff;
        l1a_l1s_com.Smeas_dedic.qual_nbr_meas_sub  += A_D_BLEN;

        // TEMPORARY : reset buffers and flags in NDB ...
        //             reset nerr....
        //             reset A_CD contents.......
        l1s_dsp_com.dsp_ndb_ptr->a_cd[0]   =  (1<<B_FIRE1); // B_FIRE1=1,B_FIRE0=0,BLUD=0.
        l1s_dsp_com.dsp_ndb_ptr->a_cd[2]   =  0xffff;
        for (i=0; i<12  ;i++)
          l1s_dsp_com.dsp_ndb_ptr->a_cd[3+i] =  0x0000;

        // task is completed, make it INACTIVE.
        l1s.task_status[task].current_status = INACTIVE;
      }
    }
    break;

    case TCHTH:
    /*---------------------------------------------------*/
    /* Dedicated mode: TCHTH receive task.               */
    /*                 HALF RATE                         */
    /*---------------------------------------------------*/
    {
      UWORD32   b_blud;
      UWORD8    channel_mode;
      //OMAPS00090550 UWORD8    channel_type;
      UWORD8    subchannel;
      UWORD32   l1_mode;
      UWORD32   fn_mod_104;
      UWORD32   fn_mod_52;
      //OMAPS00090550 UWORD32   fn_report_mod13_mod4;
      UWORD32   normalised_fn_report_mod13_mod4;
      UWORD32   normalised_fn_report_mod26;
      UWORD8    IL_for_rxlev = 0; //omaps00090550
      #if (AMR == 1)
        UWORD8    rx_type;
        UWORD8    b_ratscch_blud,b_facch_blud;
        UWORD8    voco_type;
        BOOL      facch_present = FALSE;
        #if REL99
        #if FF_EMR
          emr_params.amr_facch_present = FALSE;
          emr_params.amr_facch_fire1   = FALSE;
        #endif
		#endif
      #endif

      // Read control results and feed control algorithms.
      // **************************************************

      // Read control information.
      toa   = l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_TOA]   & 0xffff;
      pm    = (l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_PM]   & 0xffff) >> 5;
      angle = l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_ANGLE] & 0xffff;
      snr   = l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_SNR]   & 0xffff;

      l1_check_pm_error(pm, task);

      #if TRACE_TYPE==3
        stats_samples_tch(toa,pm,angle,snr);
      #endif
      #if (TRACE_TYPE==2 ) || (TRACE_TYPE==3)
        uart_trace(TCHTH);
      #endif

      // Update AFC: Call AFC control function (KALMAN filter).

      #if AFC_ALGO
        #if TESTMODE
          if (l1_config.afc_enable)
        #endif
          {
            #if (VCXO_ALGO == 0)
              l1s.afc = l1ctl_afc(AFC_CLOSED_LOOP, &l1s.afc_frame_count, (WORD16)angle, snr, radio_freq);
            #else
              l1s.afc = l1ctl_afc(AFC_CLOSED_LOOP, &l1s.afc_frame_count, (WORD16)angle, snr, radio_freq,l1a_l1s_com.mode);
            #endif
          }
      #endif

      // Increment number of burst not sent due to DTX.
      if((UWORD32)(l1s_dsp_com.dsp_db_r_ptr->d_task_u & 0xffff) == TCH_DTX_UL)
      {
        l1a_l1s_com.Smeas_dedic.dtx_used++;
        l1s.dtx_ul_on = TRUE;
      }
      else
      {
        l1s.dtx_ul_on = FALSE;
      }

      // Check SID frame subset...
      channel_mode = l1a_l1s_com.dedic_set.aset->achan_ptr->mode;
//OMAPS00090550      channel_type = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type;
      subchannel   = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->subchannel;
      fn_mod_104   = l1s.actual_time.fn % 104;
      fn_mod_52    = l1s.actual_time.fn % 52;

    #if REL99
    #if FF_EMR
      // Compute FN in reporting period % 13 % 4 = (((FN-subchannel)+ 13) %13) %4
      normalised_fn_report_mod13_mod4 = ((l1s.actual_time.fn - subchannel + 13)  % 13) % 4;

      // Compute FN in reporting period % 26 independently of the considered subchannel.
      normalised_fn_report_mod26 = (l1s.actual_time.fn - subchannel + 26)  % 26;

      emr_params.channel_mode                    = channel_mode;
      emr_params.subchannel                      = subchannel;
      emr_params.normalised_fn_mod13_mod4 = normalised_fn_report_mod13_mod4;
    #endif //FF_EMR
    #endif //REL99


    #if (AMR == 1)
      // Check if we're in AMR DTX mode
      if(channel_mode==TCH_AHS_MODE &&
         ( (((l1s.actual_time.fn_mod13 % 4)==3) && (subchannel==0)) ||    // AHS0: block is decoded on DSP side at fn%13%4=2
           (((l1s.actual_time.fn_mod13 % 4)==0) && (subchannel==1)) ))    // AHS1: block is decoded on DSP side at fn%13%4=3
      {
        if(subchannel==0)
        {
          b_blud  = (l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] & (1<<B_BLUD)) >> B_BLUD;
          rx_type = (l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] & RX_TYPE_MASK) >> RX_TYPE_SHIFT;
        }
        else
        {
          b_blud  = (l1s_dsp_com.dsp_ndb_ptr->a_dd_1[0] & (1<<B_BLUD)) >> B_BLUD;
          rx_type = (l1s_dsp_com.dsp_ndb_ptr->a_dd_1[0] & RX_TYPE_MASK) >> RX_TYPE_SHIFT;
        }
        b_ratscch_blud = (l1s_dsp_com.dsp_ndb_ptr->a_ratscch_dl[0] & (1<<B_BLUD)) >> B_BLUD;
        b_facch_blud   = (l1s_dsp_com.dsp_ndb_ptr->a_fd[0] & (1<<B_BLUD)) >> B_BLUD;

        // Check if AMR DTX mode is on
        if((((rx_type==SID_FIRST) || (rx_type==SID_UPDATE) || (rx_type==SID_BAD)) && b_blud==TRUE) ||
           (rx_type==AMR_NO_DATA && b_blud==FALSE))
		{
          l1s.dtx_amr_dl_on=TRUE;
		}
        else if(b_ratscch_blud==FALSE && b_facch_blud==FALSE)
		{
          l1s.dtx_amr_dl_on=FALSE;
		}
      }
    #endif

    #if (AMR == 1)
      if (channel_mode != TCH_AHS_MODE)
      {
      // This AGC and TOA update isn't applied to the adaptative half rate mode.
      if(((channel_mode == TCH_HS_MODE) && (subchannel == 0) &&
          (fn_mod_52 > 0)  && (fn_mod_52 <= 7))  ||
         ((channel_mode == TCH_HS_MODE) && (subchannel == 1) &&
          (fn_mod_52 > 14)  && (fn_mod_52 <= 21))  ||
         ((channel_mode != TCH_HS_MODE)  &&
         (subchannel == 0) && (fn_mod_104 > 56) && (fn_mod_104 <= 76)) ||
         ((channel_mode != TCH_HS_MODE)  &&
         (subchannel == 1) && (fn_mod_104 > 66) && (fn_mod_104 <= 86)))
    #else
      if(((channel_mode == TCH_HS_MODE) && (subchannel == 0) &&
          (fn_mod_52 > 0)  && (fn_mod_52 <= 7))  ||
         ((channel_mode == TCH_HS_MODE) && (subchannel == 1) &&
          (fn_mod_52 > 14)  && (fn_mod_52 <= 21))  ||
         ((channel_mode != TCH_HS_MODE) && (subchannel == 0) &&
          (fn_mod_104 > 56) && (fn_mod_104 <= 76)) ||
         ((channel_mode != TCH_HS_MODE) && (subchannel == 1) &&
          (fn_mod_104 > 66) && (fn_mod_104 <= 86)))
    #endif
      // Current results are from the TDMA frame subset always received (GSM05.08, $8.3).
      // -> pwr meas. must be used for SUB set result.
      // -> TOA filtering can be fed with SNR/TOA.
      // WARNING: TCH/H in signalling only is here processed like TCH/H data. GSM spec is
      // ======== unclear !!!!!!!!!!!!!!!1
      {
        // Update AGC: Call DPAGC algorithm
        IL_for_rxlev = l1ctl_dpagc(1,beacon,(UWORD8)pm, radio_freq, IL_info_ptr);

        // Dedicated mode serving cell measurement reading, indicate "SUB".
        #if REL99
        #if FF_EMR
          l1s_read_dedic_scell_meas(IL_for_rxlev, 1,&emr_params);
        #endif
        #else
          l1s_read_dedic_scell_meas(IL_for_rxlev, 1);
        #endif

        //Feed TOA histogram.
        #if (TOA_ALGO != 0)
          // When in 1/2 rate data, we are working on 14 SID frames (instead
          // of 12 otherwise), so we need to increment length of the histogram
          // filling period from 36 to 42.
          if (channel_mode != TCH_HS_MODE)
            l1_mode=DEDIC_MODE_HALF_DATA;
          else
            l1_mode=l1a_l1s_com.mode;

          #if (TOA_ALGO == 2)
            if(l1s.toa_var.toa_snr_mask == 0)
          #else
            if(l1s.toa_snr_mask == 0)
          #endif
          {
            #if 1	/* FreeCalypso TCS211 reconstruction */
              if (IL_for_rxlev < IL_FOR_RXLEV_SNR)
                l1s.toa_shift = l1ctl_toa(TOA_RUN, l1_mode, snr, toa, &l1s.toa_update, &l1s.toa_period_count);
              else
                l1s.toa_shift = l1ctl_toa(TOA_RUN, l1_mode, 0, toa, &l1s.toa_update, &l1s.toa_period_count);
            #else
              UWORD32 snr_temp;
              snr_temp = (IL_for_rxlev < IL_FOR_RXLEV_SNR)? snr: 0;
              #if (TOA_ALGO == 2)
                l1s.toa_var.toa_shift = l1ctl_toa(TOA_RUN, l1_mode, snr_temp, toa);
              #else
                l1s.toa_shift = l1ctl_toa(TOA_RUN, l1_mode, snr_temp, toa, &l1s.toa_update, &l1s.toa_period_count
                #if (FF_L1_FAST_DECODING == 1)
                    ,0
                #endif
                    );
              #endif
            #endif
          }
        #endif
      } // if(((channel_mode == TCH_HS_MODE) && (subchannel == 0) &&
      else
      {
        // Update AGC: Call DPAGC algorithm
        IL_for_rxlev = l1ctl_dpagc(0,beacon,(UWORD8)pm,radio_freq,IL_info_ptr);

        // Dedicated mode serving cell measurement reading, full set only.
        #if REL99
        #if FF_EMR
          l1s_read_dedic_scell_meas(IL_for_rxlev, 0, &emr_params);
        #endif
        #else
          l1s_read_dedic_scell_meas(IL_for_rxlev, 0);
        #endif
      }

    #if (AMR == 1)
      } // if (channel_mode != TCH_AHS_MODE)
    #endif

      #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
        RTTL1_FILL_DL_BURST(angle, snr, l1s.afc, task, pm, toa, IL_for_rxlev)
      #endif
      #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4)) && HAVE_L1_TRACE_BURST_PARAM
        l1_trace_burst_param(angle, snr, l1s.afc, task, pm, toa, IL_for_rxlev);
      #endif
      #if (BURST_PARAM_LOG_ENABLE == 1)
        l1_log_burst_param(angle, snr, l1s.afc, task, pm, toa, IL_for_rxlev);
      #endif

      // Read downlink DATA block from MCU/DSP interface.
      // *************************************************

      // Compute FN % 13 % 4
      //OMAPS00090550 fn_report_mod13_mod4 = (l1s.actual_time.fn_mod13) % 4;
      // Compute normalised FN % 13 %4 = (((FN-subchannel)+ 13) %13) %4
      normalised_fn_report_mod13_mod4 = ((l1s.actual_time.fn - subchannel + 13)  % 13) % 4;
      // Compute normalised FN %26 = ((FN - subchannel)+ 26) %26
      normalised_fn_report_mod26 = (l1s.actual_time.fn - subchannel + 26)  % 26;

      if((normalised_fn_report_mod26 == 16)||
         (normalised_fn_report_mod26 == 24)||
         (normalised_fn_report_mod26 == 7))
        // It is time to get FACCH/H data block.
      {
        // FACCH: Check A_FD information block.
        //-------------------------------------

      UWORD8  temp;
        b_blud = (l1s_dsp_com.dsp_ndb_ptr->a_fd[0] & (1<<B_BLUD)) >> B_BLUD;
        #if ((REL99) && (AMR == 1))
        #if FF_EMR
          emr_params.amr_facch_fire1 = (l1s_dsp_com.dsp_ndb_ptr->a_fd[0] & (1<<B_FIRE1)) >> B_FIRE1;
        #endif
        #endif

        if(b_blud == TRUE)
        {
          // Read FACCH DL data block from DSP, pass it to L2.
      #if ( FF_REPEATED_DL_FACCH == 1 )
               #if (TRACE_TYPE == 1 || TRACE_TYPE == 4)
                trace_info.facch_dl_count_all++;
               #endif
                  // if the current block is a repetition reports NULL to L2 otherwise reports the current block
                  l1s_read_dcch_dl((API*)l1s_repeated_facch_check(l1s_dsp_com.dsp_ndb_ptr->a_fd), task);
       #else
                  l1s_read_dcch_dl(l1s_dsp_com.dsp_ndb_ptr->a_fd, task);
       #endif

        #if (AMR == 1)
          if (channel_mode != TCH_AHS_MODE)
          {
        #endif

          // RXQUAL_SUB : In case of data taffic channels, accumulate number of
          // estimated errors, this value is contained in a_fd[2] field, only
          // for SID TDMA frames received as FACCH frames. (GSM 5.08 $8.4)
          if (((fn_mod_104==59) &&  (channel_mode==TCH_HS_MODE)     && (subchannel==0))    ||
              ((fn_mod_104==73) &&  (channel_mode==TCH_HS_MODE)     && (subchannel==1))    ||
              ((fn_mod_104==76) && ((channel_mode==TCH_48H_MODE)||
                                    (channel_mode==TCH_24H_MODE))   && (subchannel==0))    ||
              ((fn_mod_104==86) && ((channel_mode==TCH_48H_MODE)||
                                    (channel_mode==TCH_24H_MODE))   && (subchannel==1)))
          // last SID TDMA frame received as FACCH frames.
          {
             l1a_l1s_com.Smeas_dedic.qual_acc_sub      += l1s_dsp_com.dsp_ndb_ptr->a_fd[2] & 0xffff;
             l1a_l1s_com.Smeas_dedic.qual_nbr_meas_sub += TCH_F_D_BLEN;
          }

        #if (AMR == 1)
          } // if (channel_mode != TCH_AHS_MODE)
          else
          {
            // Indicate to AMR specific processing that burst was a FACCH
            facch_present = TRUE;
            #if ((REL99) && (AMR == 1))
            #if FF_EMR
              emr_params.amr_facch_present = facch_present;
            #endif
            #endif


            // Update AGC: Call DPAGC algorithm
            IL_for_rxlev = l1ctl_dpagc_amr(0,beacon,(UWORD8)pm, radio_freq, IL_info_ptr);

            // Dedicated mode serving cell measurement reading, indicate "FULL".
            #if REL99
            #if FF_EMR
              l1s_read_dedic_scell_meas(IL_for_rxlev, 0, &emr_params);
            #endif
            #else
              l1s_read_dedic_scell_meas(IL_for_rxlev, 0);
            #endif
          }
        #endif

          // RXQUAL_FULL : accumulate number of estimated errors, this value is
          // contained in a_fd[2] field, for each TCHT block.
          // The same for AMR
	#if (AMR == 1)
          // in AMR, l1s.dtx_amr_dl_on is FALSE if DTX mode is off
          // in non AMR TCH, l1s.dtx_amr_dl_on is always FALSE
          // In AMR DTX, DSP patch sometimes reports FACCH blocks which ARE NOT FACCH blocks
          // therefore they shouldn't be taken into account in the RXQUALL_FULL computation
          if(l1s.dtx_amr_dl_on==FALSE)
	#endif
		  {
          l1a_l1s_com.Smeas_dedic.qual_acc_full      += l1s_dsp_com.dsp_ndb_ptr->a_fd[2] & 0xffff;
          l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full += TCH_F_D_BLEN;
		  }

          // Reset A_FD header.
          // B_FIRE1 =1, B_FIRE0 =0 , BLUD =0
          l1s_dsp_com.dsp_ndb_ptr->a_fd[0] = (1<<B_FIRE1);
          l1s_dsp_com.dsp_ndb_ptr->a_fd[2] = 0xffff;

          // Rem: when FACCH is received, we must reset A_DD_0 header also.
          // Reset A_DD_0 header in NDB.
#if (AMR == 1)
          if ((channel_mode==TCH_AHS_MODE) && (subchannel==0))
          {
            l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] &= (API)(RX_TYPE_MASK);
          }
          else
#endif
          {
            l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] = 0;
          }
          l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] = 0xffff;

          // Rem: when FACCH is received, we must reset A_DD_1 header also.
          // Reset A_DD_0 header in NDB.
#if (AMR == 1)
          if ((channel_mode==TCH_AHS_MODE) && (subchannel==1))
          {
            l1s_dsp_com.dsp_ndb_ptr->a_dd_1[0] &= (API)(RX_TYPE_MASK);
          }
          else
#endif
          {
            l1s_dsp_com.dsp_ndb_ptr->a_dd_1[0] = 0;
          }
          l1s_dsp_com.dsp_ndb_ptr->a_dd_1[2] = 0xffff;

        } // if(b_blud == TRUE)

        else
        // No FACCH received at FACCH boundary frame. Nevertheless, need to read dummy
        // FACCH DL data block.
        {
          // Dummy: Read FACCH DL data block from DSP, pass it to L2.
          // Rem: this is an upper layer requirement to call this
          // function at every FACCH DL boundary.
          l1s_read_dcch_dl(NULL, task);
        }
      } // if((normalised_fn_report_mod26 == 16)|| ...

      // else we are not at FACCH boundary frame
      // We must check for the presence of a TCH/H block (even if it does fall on a FACCH boundary)
      // We use the b_blud bit to confirm presence of TCH/H (or FACCH)
      if((normalised_fn_report_mod13_mod4 == 3) && (channel_mode==TCH_HS_MODE))
      // It is time to get TCH/HS data block.
      {
        #if TRACE_TYPE==3
          if (l1_stats.type == PLAY_UL       &&
             (channel_mode  == TCH_HS_MODE))
            play_trace();
        #endif

        // Check A_DD_0 information block only if no FACCH.
        if (subchannel==0)
           b_blud = (l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] & (1<<B_BLUD)) >> B_BLUD;
        else
           b_blud = (l1s_dsp_com.dsp_ndb_ptr->a_dd_1[0] & (1<<B_BLUD)) >> B_BLUD;

        if(b_blud == TRUE)
        {
          if (subchannel==0)
          {
            // RXQUAL_SUB : In case of speech traffic channels, accumulate number of
            // estimated errors, this value is contained in a_dd_0[2] field, only
            // for SID TDMA frames. (GSM 5.08 $8.4)
            if (fn_mod_104==59)
            {
               l1a_l1s_com.Smeas_dedic.qual_acc_sub      += l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] & 0xffff;
               l1a_l1s_com.Smeas_dedic.qual_nbr_meas_sub += TCH_HS_BLEN;
            }
            // RXQUAL_FULL : accumulate number of estimated errors, this value is
            // contained in a_dd_0[2] field, for each TCHT block.
            l1a_l1s_com.Smeas_dedic.qual_acc_full += l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] & 0xffff;
            // Reset A_DD_0 header in NDB.
            l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] = 0;
            l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] = 0xffff;
          } // if (subchannel==0)
          else
          {
            // RXQUAL_SUB : In case of speech traffic channels, accumulate number of
            // estimated errors, this value is contained in a_dd_1[2] field, only
            // for SID TDMA frames. (GSM 5.08 $8.4)
            if (fn_mod_104==73)
            {
               l1a_l1s_com.Smeas_dedic.qual_acc_sub      += l1s_dsp_com.dsp_ndb_ptr->a_dd_1[2] & 0xffff;
               l1a_l1s_com.Smeas_dedic.qual_nbr_meas_sub += TCH_HS_BLEN;
            }
            l1a_l1s_com.Smeas_dedic.qual_acc_full += l1s_dsp_com.dsp_ndb_ptr->a_dd_1[2] & 0xffff;
            // Reset A_DD_1 header in NDB.
            l1s_dsp_com.dsp_ndb_ptr->a_dd_1[0] = 0;
            l1s_dsp_com.dsp_ndb_ptr->a_dd_1[2] = 0xffff;
          }
          l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full += TCH_HS_BLEN;
        } // if(b_blud == TRUE)
      } // if((normalised_fn_report_mod13_mod4 == 3) && (channel_mode==TCH_HS_MODE))

    #if (AMR == 1)
      if(((normalised_fn_report_mod26 == 20)  ||
          (normalised_fn_report_mod26 == 3)   ||
          (normalised_fn_report_mod26 == 11)) &&
         ((channel_mode == TCH_48H_MODE)      ||
          (channel_mode == TCH_24H_MODE)))
    #else
      if(((normalised_fn_report_mod26 == 20)  ||
          (normalised_fn_report_mod26 == 3)   ||
        (normalised_fn_report_mod26 == 11)) && (channel_mode!=TCH_HS_MODE))
    #endif
      // It is time to get TCH/H4.8 or TCH/H2.4 data block.
      {
        // Check A_DD_0 information block only if no FACCH.
        if (subchannel==0)
           b_blud = (l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] & (1<<B_BLUD)) >> B_BLUD;
        else
           b_blud = (l1s_dsp_com.dsp_ndb_ptr->a_dd_1[0] & (1<<B_BLUD)) >> B_BLUD;

        if(b_blud == TRUE)
        {
          if (subchannel==0)
          {
            // RXQUAL_SUB : In case of speech traffic channels, accumulate number of
            // estimated errors, this value is contained in a_dd_0[2] field, only
            // for SID TDMA frames. (GSM 5.08 $8.4)
            if (fn_mod_104==76)
            {
               l1a_l1s_com.Smeas_dedic.qual_acc_sub      += l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] & 0xffff;
               l1a_l1s_com.Smeas_dedic.qual_nbr_meas_sub += TCH_F_D_BLEN;
            }
            // RXQUAL_FULL : accumulate number of estimated errors, this value is
            // contained in a_dd_0[2] field, for each TCHT block.
            l1a_l1s_com.Smeas_dedic.qual_acc_full += l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] & 0xffff;

            // Reset A_DD_0 header in NDB.
            l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] = 0;
            l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] = 0xffff;
          }
          else
          {
            // RXQUAL_SUB : In case of speech traffic channels, accumulate number of
            // estimated errors, this value is contained in a_dd_1[2] field, only
            // for SID TDMA frames. (GSM 5.08 $8.4)
            if (fn_mod_104==86)
            {
               l1a_l1s_com.Smeas_dedic.qual_acc_sub      += l1s_dsp_com.dsp_ndb_ptr->a_dd_1[2] & 0xffff;
               l1a_l1s_com.Smeas_dedic.qual_nbr_meas_sub += TCH_F_D_BLEN;
            }
            l1a_l1s_com.Smeas_dedic.qual_acc_full += l1s_dsp_com.dsp_ndb_ptr->a_dd_1[2] & 0xffff;
            // Reset A_DD_1 header in NDB.
            l1s_dsp_com.dsp_ndb_ptr->a_dd_1[0] = 0;
            l1s_dsp_com.dsp_ndb_ptr->a_dd_1[2] = 0xffff;
          }
          // RXQUAL_FULL : accumulate number of estimated errors, this value is
          // contained in a_dd_1[2] field, for each TCHT block.
          l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full += TCH_F_D_BLEN;

          // WARNING: sequence number is not implemented in DATA half rate
          // TO BE DEFINED......
        } // if(b_blud == TRUE)
      } // if(((normalised_fn_report_mod26 == 20)  || ...
    #if (AMR == 1)
      if ((channel_mode == TCH_AHS_MODE) && (facch_present == FALSE))
      {
        // the channel is a TCH/AHS and it's time to receive a new block
        if (subchannel == 0)
        {
          // Load the bit to check if the block is valid
          b_blud = (l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] & (1<<B_BLUD)) >> B_BLUD;
        }
        else // subchannel 1
        {
          // Load the bit to check if the block is valid
          b_blud = (l1s_dsp_com.dsp_ndb_ptr->a_dd_1[0] & (1<<B_BLUD)) >> B_BLUD;
        }

        b_ratscch_blud = (l1s_dsp_com.dsp_ndb_ptr->a_ratscch_dl[0] & (1<<B_BLUD)) >> B_BLUD;

        // All frames except NO_DATA (b_blud = FALSE) and FACCH, i.e AMR speech/SID block or a RATSCCH block
        if(b_ratscch_blud==TRUE)
        {
          // RXQUAL_FULL : accumulate number of estimated errors, this value is
          // contained in a_ratscch_dl[2] field, for each TCHT block.
          l1a_l1s_com.Smeas_dedic.qual_acc_full += l1s_dsp_com.dsp_ndb_ptr->a_ratscch_dl[2] & 0xffff;

          // Reset the A_RATSCCH_DL header in NDB.
          l1s_dsp_com.dsp_ndb_ptr->a_ratscch_dl[0] = 0;
          l1s_dsp_com.dsp_ndb_ptr->a_ratscch_dl[2] = 0xffff;

          // RXQUAL_FULL : the number of bits examined for errors on serving cell depends on
          // the block received.
          l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full += RATSCCH_BLEN;

          IL_for_rxlev = l1ctl_dpagc_amr(0,beacon,(UWORD8)pm, radio_freq, IL_info_ptr);
            #if REL99
            #if FF_EMR
              l1s_read_dedic_scell_meas(IL_for_rxlev, 0, &emr_params);
            #endif
            #else
              l1s_read_dedic_scell_meas(IL_for_rxlev, 0);
            #endif
        }
        else if(b_blud==TRUE)
        {
          if (subchannel == 0)
          {
            // Load the type of the block received
            rx_type = (l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] & RX_TYPE_MASK) >> RX_TYPE_SHIFT;

            // Load the type of vocoder currently used
            voco_type = (l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] & VOCODER_TYPE_MASK) >> VOCODER_TYPE_SHIFT;

          #if (DEBUG_DEDIC_TCH_BLOCK_STAT == 1)
            Trace_dedic_tch_block_stat(rx_type, l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2], voco_type);
          #endif

            // RXQUAL_SUB : In case of adaptative traffic channel, accumulate number of estimated errors
            // is contained in the a_dd_0[2] value but the accumulation is made with SID_UPDATE frame only.
           if((rx_type==SID_UPDATE) || (rx_type==SID_BAD))

            {
              l1a_l1s_com.Smeas_dedic.qual_acc_sub      += l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] & 0xffff;
              l1a_l1s_com.Smeas_dedic.qual_nbr_meas_sub += SID_UPDATE_BLEN;
            }

            // RXQUAL_FULL : accumulate number of estimated errors, this value is
            // contained in a_dd_0[2] field, for each TCHT block.
              l1a_l1s_com.Smeas_dedic.qual_acc_full += l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] & 0xffff;









            // Reset A_DD_0 header in NDB.
            l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] &= (API)(RX_TYPE_MASK);
            l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] = 0xffff;
          }
          else // subchannel ==1
          {
            // Load the type of the block received
            rx_type = (l1s_dsp_com.dsp_ndb_ptr->a_dd_1[0] & RX_TYPE_MASK) >> RX_TYPE_SHIFT;

            // Load the type of vocoder currently used
            voco_type = (l1s_dsp_com.dsp_ndb_ptr->a_dd_1[0] & VOCODER_TYPE_MASK) >> VOCODER_TYPE_SHIFT;

          #if (DEBUG_DEDIC_TCH_BLOCK_STAT == 1)
            Trace_dedic_tch_block_stat(rx_type, l1s_dsp_com.dsp_ndb_ptr->a_dd_1[2], voco_type);
          #endif

            // RXQUAL_SUB : In case of adaptative traffic channel, accumulate number of estimated errors
            // is contained in the a_dd_1[2]  value but the accumulation is made with SID_UPDATE block only.
			if((rx_type==SID_UPDATE) || (rx_type==SID_BAD))

            {
              l1a_l1s_com.Smeas_dedic.qual_acc_sub      += l1s_dsp_com.dsp_ndb_ptr->a_dd_1[2] & 0xffff;
              l1a_l1s_com.Smeas_dedic.qual_nbr_meas_sub += SID_UPDATE_BLEN;
            }

            // RXQUAL_FULL : accumulate number of estimated errors, this value is
            // contained in a_dd_1[2] field, for each TCHT block.
              l1a_l1s_com.Smeas_dedic.qual_acc_full += l1s_dsp_com.dsp_ndb_ptr->a_dd_1[2] & 0xffff;

            // Reset A_DD_1 header in NDB.
            l1s_dsp_com.dsp_ndb_ptr->a_dd_1[0] &= (API)(RX_TYPE_MASK);
            l1s_dsp_com.dsp_ndb_ptr->a_dd_1[2] = 0xffff;
          } // subchannel == 1





          // RXQUAL_FULL : the number of bits examined for errors on serving cell depends on
          // the block received.
if((rx_type==SPEECH_GOOD) || (rx_type==SPEECH_DEGRADED) || (rx_type==SPEECH_BAD))
          {
            // The block length depens on the vocoder type
            switch (voco_type)
            {
              case AMR_CHANNEL_7_95:
              {
                // TCH-AHS 7.95
                l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full += TCH_AHS_7_95_BLEN;
              }
              break;
              case AMR_CHANNEL_7_4:
              {
                // TCH-AHS 7.4
                l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full += TCH_AHS_7_4_BLEN;
              }
              break;
              case AMR_CHANNEL_6_7:
              {
                // TCH-AHS 6.7
                l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full += TCH_AHS_6_7_BLEN;
              }
              break;
              case AMR_CHANNEL_5_9:
              {
                // TCH-AHS 5.9
                l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full += TCH_AHS_5_9_BLEN;
              }
              break;
              case AMR_CHANNEL_5_15:
              {
                // TCH-AHS 5.15
                l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full += TCH_AHS_5_15_BLEN;
              }
              break;
              case AMR_CHANNEL_4_75:
              {
                // TCH-AHS 4.75
               l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full += TCH_AHS_4_75_BLEN;
              }
              break;
            } // switch
          } // if ( (rx_type == SPEECH_GOOD) || ...
          else
			if((rx_type == SID_UPDATE) || (rx_type == SID_BAD))

          {
            // the block is a SID UPDATE
            l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full += SID_UPDATE_BLEN;
          }







          // AGC, TOA update for AMR... SUB FIFO only for SID_UPDATE frame
          if((rx_type == SID_UPDATE) || (rx_type == SID_BAD))

          {
            IL_for_rxlev = l1ctl_dpagc_amr(1,beacon,(UWORD8)pm, radio_freq, IL_info_ptr);
            #if REL99
            #if FF_EMR
              l1s_read_dedic_scell_meas(IL_for_rxlev, 1, &emr_params);
            #endif
            #else
              l1s_read_dedic_scell_meas(IL_for_rxlev, 1);
            #endif

          #if (TOA_ALGO != 0)
            #if (TOA_ALGO == 2)
              if(l1s.toa_var.toa_snr_mask == 0)
            #else
              if(l1s.toa_snr_mask == 0)
            #endif
            {
              #if 1	/* FreeCalypso TCS211 reconstruction */
                if (IL_for_rxlev < IL_FOR_RXLEV_SNR)
                  l1s.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, snr, toa, &l1s.toa_update, &l1s.toa_period_count);
                else
                  l1s.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, 0, toa, &l1s.toa_update, &l1s.toa_period_count);
              #else
                UWORD32 snr_temp;
                snr_temp = (IL_for_rxlev < IL_FOR_RXLEV_SNR)? snr: 0;
                #if (TOA_ALGO == 2)
                  l1s.toa_var.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, snr_temp, toa);
                #else
                  l1s.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, snr_temp, toa, &l1s.toa_update, &l1s.toa_period_count
                  #if (FF_L1_FAST_DECODING == 1)
                      ,0
                  #endif
                      );
                #endif
              #endif
            }
          #endif
          }
          else
          {
            IL_for_rxlev = l1ctl_dpagc_amr(0,beacon,(UWORD8)pm, radio_freq, IL_info_ptr);
            #if REL99
            #if FF_EMR
              l1s_read_dedic_scell_meas(IL_for_rxlev, 0, &emr_params);
            #endif
            #else
              l1s_read_dedic_scell_meas(IL_for_rxlev, 0);
            #endif
          }
        } // if (b_blud == TRUE)
        // simple burst or NO_DATA frame
        else
        {
          // NO_DATA is considered a bad frame
          if (normalised_fn_report_mod13_mod4 == 3)
          {
            l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full += TCH_AHS_4_75_BLEN;
            if (subchannel == 0)
            {
              l1a_l1s_com.Smeas_dedic.qual_acc_full += l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] & 0xffff;

            #if (DEBUG_DEDIC_TCH_BLOCK_STAT == 1)
              Trace_dedic_tch_block_stat(AMR_NO_DATA, l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2], 0);
            #endif

              // Reset A_DD_0 header in NDB.
              l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] &= (API)(RX_TYPE_MASK);
              l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] = 0xffff;
            }
            else
            {
              l1a_l1s_com.Smeas_dedic.qual_acc_full += l1s_dsp_com.dsp_ndb_ptr->a_dd_1[2] & 0xffff;

            #if (DEBUG_DEDIC_TCH_BLOCK_STAT == 1)
              Trace_dedic_tch_block_stat(AMR_NO_DATA, l1s_dsp_com.dsp_ndb_ptr->a_dd_1[2], 0);
            #endif

              // Reset A_DD_0 header in NDB.
              l1s_dsp_com.dsp_ndb_ptr->a_dd_1[0] &= (API)(RX_TYPE_MASK);
              l1s_dsp_com.dsp_ndb_ptr->a_dd_1[2] = 0xffff;
            }
          }
          // Update AGC: Call DPAGC AMR algorithm in order to fill the G_all buffer
          IL_for_rxlev = l1ctl_dpagc_amr(0,beacon,(UWORD8)pm,radio_freq,IL_info_ptr);

          // Dedicated mode serving cell measurement reading, full set only.
          #if REL99
          #if FF_EMR
            l1s_read_dedic_scell_meas(IL_for_rxlev, 0, &emr_params);
          #endif
          #else
            l1s_read_dedic_scell_meas(IL_for_rxlev, 0);
          #endif
        }
      } // if ((channel_mode == TCH_AHS_MODE) && (facch_present == FALSE))
    #endif

      // task is completed, make it INACTIVE.
      l1s.task_status[task].current_status = INACTIVE;
    }
    break;

    case TCHTF:
    /*---------------------------------------------------*/
    /* Dedicated mode: TCHTF receive task.               */
    /*                 FULL RATE                         */
    /*---------------------------------------------------*/
    {
      UWORD8   IL_for_rxlev = 0; //omaps00090550
      UWORD32  b_blud;

      UWORD8   channel_mode;
      //OMAPS00090550 UWORD8   channel_type;
      UWORD32  fn_mod_104;
      //OMAPS00090550 UWORD32  fn_mod_52;
      UWORD32  fn_report_mod13_mod4;
      #if (AMR == 1)
        UWORD8   rx_type;
        UWORD8   b_ratscch_blud,b_facch_blud;
        BOOL     facch_present = FALSE;
      #endif

      #if TESTMODE
        xSignalHeaderRec *msg;
      #endif

      // Read control results and feed control algorithms.
      // **************************************************

      // Read control information.
      toa   = l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_TOA]   & 0xffff;
      pm    = (l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_PM]   & 0xffff) >> 5;
      angle = l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_ANGLE] & 0xffff;
      snr   = l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_SNR]   & 0xffff;

      l1_check_pm_error(pm, task);

      #if TRACE_TYPE==3
        stats_samples_tch(toa,pm,angle,snr);
      #endif

      #if (TRACE_TYPE==2 ) || (TRACE_TYPE==3)
        uart_trace(TCHTF);
      #endif

      // Update AFC: Call AFC control function (KALMAN filter).
      #if AFC_ALGO
        #if TESTMODE
          if (l1_config.afc_enable)
        #endif
          {
            #if (VCXO_ALGO == 0)
              l1s.afc = l1ctl_afc(AFC_CLOSED_LOOP, &l1s.afc_frame_count, (WORD16)angle, snr, radio_freq);
            #else
              l1s.afc = l1ctl_afc(AFC_CLOSED_LOOP, &l1s.afc_frame_count, (WORD16)angle, snr, radio_freq,l1a_l1s_com.mode);
            #endif
          }
      #endif

      // Check SID frame subset...
      channel_mode = l1a_l1s_com.dedic_set.aset->achan_ptr->mode;
      //OMAPS00090550 channel_type = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type;
      fn_mod_104   = l1s.actual_time.fn % 104;
      //OMAPS00090550 fn_mod_52    = l1s.actual_time.fn % 52;

	  #if (AMR == 1)
      // Check if we're in AMR DTX mode
      if(channel_mode==TCH_AFS_MODE && (l1s.actual_time.fn_mod13 % 4)==0)    // AFS: block is decoded on DSP side at fn%13%4=3
      {
        b_blud         = (l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] & (1<<B_BLUD)) >> B_BLUD;
        rx_type        = (l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] & RX_TYPE_MASK) >> RX_TYPE_SHIFT;
        b_ratscch_blud = (l1s_dsp_com.dsp_ndb_ptr->a_ratscch_dl[0] & (1<<B_BLUD)) >> B_BLUD;
        b_facch_blud   = (l1s_dsp_com.dsp_ndb_ptr->a_fd[0] & (1<<B_BLUD)) >> B_BLUD;

        // Check if AMR DTX mode is on
        if((((rx_type==SID_FIRST) || (rx_type==SID_UPDATE) || (rx_type==SID_BAD)) && b_blud==TRUE) ||
           (rx_type==AMR_NO_DATA && b_blud==FALSE))
		{
          l1s.dtx_amr_dl_on=TRUE;
		}
        else if(b_ratscch_blud==FALSE && b_facch_blud==FALSE)
		{
          l1s.dtx_amr_dl_on=FALSE;
		}
      }
	 #endif

      #if REL99
      #if FF_EMR
        emr_params.channel_mode = channel_mode;
//        emr_params.fn_mod13_mod4 = l1s.actual_time.fn_mod13_mod4;
        #if (AMR == 1)
          emr_params.amr_facch_present = FALSE;
          emr_params.amr_facch_fire1 = FALSE;
        #endif
       #endif
      #endif

      // Increment number of burst not sent due to DTX.
      if((UWORD32)(l1s_dsp_com.dsp_db_r_ptr->d_task_u & 0xffff) == TCH_DTX_UL)
      {
        l1a_l1s_com.Smeas_dedic.dtx_used++;
        l1s.dtx_ul_on = TRUE;
      }
      else
      {
        // Some bursts are always sent in DTX mode. d_task_u does not give DTX_UL
        // so we must keep previous value of dtx_on
        if (! ((fn_mod_104 > 52) && (fn_mod_104 <= 60)) )
          l1s.dtx_ul_on = FALSE;
      }

    #if FF_L1_IT_DSP_DTX
      // Currently used for TCH-AFS only
      if (l1s.actual_time.fn_mod13_mod4 == 0) // FN%13 = 4, 8 and 12 (no TCH/F Read on FN%13=0)
      {
        // Latch TX activity status if DTX allowed
        if ((l1a_l1s_com.dedic_set.aset->dtx_allowed == FALSE) ||                // No DTX allowed
            (l1s_dsp_com.dsp_ndb_ptr->d_fast_dtx_enc_data) ||                    // DTX allowed but not used
            (l1a_apihisr_com.dtx.fast_dtx_ready == FALSE))                       // Fast DTX status is invalid
          l1a_apihisr_com.dtx.tx_active = TRUE;
        else
          l1a_apihisr_com.dtx.tx_active = FALSE;
      }
    #endif

    #if (AMR == 1)
      if (channel_mode != TCH_AFS_MODE)
      {
      // This AGC and TOA update isn't applied to the adaptative full rate mode
    #endif

      if((fn_mod_104 > 52) && (fn_mod_104 <= 60))
      // Current results are from the TDMA frame subset always received (GSM05.08, $8.3).
      // -> pwr meas. must be used for SUB set result.
      // -> TOA filtering can be fed with SNR/TOA.
      // This DTX is only applied to the mode EFR, FR and data.
      {
        // Update AGC: Call DPAGC algorithm
        IL_for_rxlev = l1ctl_dpagc(1,beacon,(UWORD8)pm, radio_freq, IL_info_ptr);

        // Dedicated mode serving cell measurement reading, indicate "SUB".
        #if REL99
        #if FF_EMR
          l1s_read_dedic_scell_meas(IL_for_rxlev, 1, &emr_params);
        #endif
        #else
          l1s_read_dedic_scell_meas(IL_for_rxlev, 1);
        #endif

        //Feed TOA histogram.
        #if (TOA_ALGO != 0)
          #if (TOA_ALGO == 2)
            if(l1s.toa_var.toa_snr_mask == 0)
          #else
            if(l1s.toa_snr_mask == 0)
          #endif
          {
            #if 1	/* FreeCalypso TCS211 reconstruction */
              if (IL_for_rxlev < IL_FOR_RXLEV_SNR)
                l1s.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, snr, toa, &l1s.toa_update, &l1s.toa_period_count);
              else
                l1s.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, 0, toa, &l1s.toa_update, &l1s.toa_period_count);
            #else
              UWORD32 snr_temp;
              snr_temp = (IL_for_rxlev < IL_FOR_RXLEV_SNR)? snr: 0;
              #if (TOA_ALGO == 2)
                l1s.toa_var.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, snr_temp, toa);
              #else
                l1s.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, snr_temp, toa, &l1s.toa_update, &l1s.toa_period_count
                #if (FF_L1_FAST_DECODING == 1)
                    ,0
                #endif
                    );
              #endif
            #endif
          }
        #endif
      }
      else
      {
        // Update AGC: Call DPAGC algorithm
        IL_for_rxlev = l1ctl_dpagc(0,beacon,(UWORD8)pm,radio_freq,IL_info_ptr);

        // Dedicated mode serving cell measurement reading, full set only.
        #if REL99
        #if FF_EMR
          l1s_read_dedic_scell_meas(IL_for_rxlev, 0, &emr_params);
        #endif
        #else
          l1s_read_dedic_scell_meas(IL_for_rxlev, 0);
        #endif
      }
    #if (AMR == 1)
      } // if (channel_mode != TCH_AFS_MODE)
    #endif

      #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
        RTTL1_FILL_DL_BURST(angle, snr, l1s.afc, task, pm, toa, IL_for_rxlev)
      #endif
      #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4)) && HAVE_L1_TRACE_BURST_PARAM
        l1_trace_burst_param(angle, snr, l1s.afc, task, pm, toa, IL_for_rxlev);
      #endif
      #if (BURST_PARAM_LOG_ENABLE == 1)
        l1_log_burst_param(angle, snr, l1s.afc, task, pm, toa, IL_for_rxlev);
      #endif

      // Read downlink DATA block from MCU/DSP interface.
      // *************************************************

      // Compute FN in reporting period % 13 % 4.
      fn_report_mod13_mod4 = (l1s.actual_time.fn_mod13) % 4;

      if(fn_report_mod13_mod4 == 0)
      // It is time to get FACCH/F or TCH/F2.4 or TCH/(E)FS data block.
      {
        UWORD8 temp;
        #if TRACE_TYPE==3
          if (l1_stats.type == PLAY_UL &&
             (channel_mode == TCH_FS_MODE || channel_mode == TCH_24F_MODE
              || channel_mode == TCH_EFR_MODE))
            play_trace();
        #endif

        // FACCH: Check A_FD information block.
        //-------------------------------------

        b_blud = (l1s_dsp_com.dsp_ndb_ptr->a_fd[0] & (1<<B_BLUD)) >> B_BLUD;
        #if ((REL99) && (AMR == 1))
        #if FF_EMR
          emr_params.amr_facch_fire1 = (l1s_dsp_com.dsp_ndb_ptr->a_fd[0] & (1<<B_FIRE1)) >> B_FIRE1;
        #endif
		#endif

        if(b_blud == TRUE)
        {
          // Read FACCH DL data block from DSP, pass it to L2.
        #if ( FF_REPEATED_DL_FACCH == 1 )
        #if (TRACE_TYPE == 1 || TRACE_TYPE == 4)
             trace_info.facch_dl_count_all++;
         #endif
          /* if the current block is a repetition reports NULL to L2 otherwise reports the current block */
       l1s_read_dcch_dl((API*)l1s_repeated_facch_check(l1s_dsp_com.dsp_ndb_ptr->a_fd), task);
       #else
       /* UWORD8 error_flag =*/ l1s_read_dcch_dl(l1s_dsp_com.dsp_ndb_ptr->a_fd, task);
      #endif /* ( FF_REPEATED_DL_FACCH == 1 ) */

        #if (AMR == 1)
          // Non AMR FACCH handling
          if (channel_mode != TCH_AFS_MODE)
          {
        #endif

          // RXQUAL_SUB : In case of data taffic channels, accumulate number of
          // estimated errors, this value is contained in a_fd[2] field, only
          // for SID TDMA frames received as FACCH frames. (GSM 5.08 $8.4)
          if (fn_mod_104==60)
          {
            l1a_l1s_com.Smeas_dedic.qual_acc_sub      += l1s_dsp_com.dsp_ndb_ptr->a_fd[2] & 0xffff;
            l1a_l1s_com.Smeas_dedic.qual_nbr_meas_sub += TCH_F_D_BLEN;
          }

        #if (AMR == 1)
          }
          else
          {
            // AGC, RXLEV_FULL

            // Indicate to AMR specific processing that burst was a FACCH
            facch_present = TRUE;
            #if ((REL99) && (AMR == 1))
            #if FF_EMR
              emr_params.amr_facch_present = facch_present;
            #endif
            #endif

            // Update AGC: Call DPAGC algorithm
            IL_for_rxlev = l1ctl_dpagc_amr(0,beacon,(UWORD8)pm, radio_freq, IL_info_ptr);

            // Dedicated mode serving cell measurement reading, indicate "FULL".
            #if REL99
            #if FF_EMR
              l1s_read_dedic_scell_meas(IL_for_rxlev, 0, &emr_params);
            #endif
            #else
              l1s_read_dedic_scell_meas(IL_for_rxlev, 0);
            #endif
          }
        #endif

          // RXQUAL_FULL : accumulate number of estimated errors, this value is
          // contained in a_fd[2] field, for each TCHT block.
	#if (AMR == 1)
          // in AMR, l1s.dtx_amr_dl_on is FALSE if DTX mode is off
          // in non AMR TCH, l1s.dtx_amr_dl_on is always FALSE
          // In AMR DTX, DSP patch sometimes reports FACCH blocks which ARE NOT FACCH blocks
          // therefore they shouldn't be taken into account in the RXQUALL_FULL computation
          if(l1s.dtx_amr_dl_on==FALSE)
	#endif
          {
          l1a_l1s_com.Smeas_dedic.qual_acc_full      += l1s_dsp_com.dsp_ndb_ptr->a_fd[2] & 0xffff;
          l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full += TCH_F_D_BLEN;
		  }









          // Reset A_FD header.
          // B_FIRE1 =1, B_FIRE0 =0 , BLUD =0
          l1s_dsp_com.dsp_ndb_ptr->a_fd[0] = (1<<B_FIRE1);
          l1s_dsp_com.dsp_ndb_ptr->a_fd[2] = 0xffff;

          // Rem: when FACCH is received, we must reset A_DD_0 header also.
          // Reset A_DD_0 header in NDB.
          l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] = 0;
          l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] = 0xffff;

          // Rem: when FACCH is received, we must reset A_DD_1 header also.
          // Reset A_DD_0 header in NDB.
          l1s_dsp_com.dsp_ndb_ptr->a_dd_1[0] = 0;
          l1s_dsp_com.dsp_ndb_ptr->a_dd_1[2] = 0xffff;

          #if TESTMODE
            if (l1_config.TestMode)
            {
              pm_fullres = (l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_PM]   & 0xffff); // F26.6

              // Allocate result message.
              msg = os_alloc_sig(sizeof(T_TMODE_TCH_INFO));
              DEBUGMSG(status,NU_ALLOC_ERR)
              msg->SignalCode = TMODE_TCH_INFO;

              ((T_TMODE_TCH_INFO *)(msg->SigP))->pm_fullres         = pm_fullres; // F26.6
              ((T_TMODE_TCH_INFO *)(msg->SigP))->snr                = snr;
              ((T_TMODE_TCH_INFO *)(msg->SigP))->toa                = toa;
              ((T_TMODE_TCH_INFO *)(msg->SigP))->angle              = (WORD16) angle; // signed
              ((T_TMODE_TCH_INFO *)(msg->SigP))->qual_full          = l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] & 0xffff;
              ((T_TMODE_TCH_INFO *)(msg->SigP))->qual_nbr_meas_full = TCH_FS_BLEN;

              // send TMODE_TCH_INFO message...
              os_send_sig(msg, L1C1_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            }
          #endif
        } // if (b_blud == TRUE)

        else // (if (b_blud == TRUE) FACCH
        {
          // No FACCH received.

          // Dummy: Read FACCH DL data block from DSP, pass it to L2.
          // Rem: this is an upper layer requirement to call this
          // function at every FACCH DL boundary.
          l1s_read_dcch_dl(NULL, task);

        #if (AMR == 1)
          if (channel_mode != TCH_AFS_MODE)
          {
        #endif

          // Check A_DD_0 information block for TCH/F2.4 or TCH/FS.
          // TCH/F2.4 or TCH/FS: Check A_DD_0 information block.
          //----------------------------------------------------

          b_blud = (l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] & (1<<B_BLUD)) >> B_BLUD;
          if(b_blud == TRUE)
          {
            // RXQUAL_SUB : In case of speech traffic channels, accumulate number of
            // estimated errors, this value is contained in a_dd_0[2] field, only
            // for SID TDMA frames. (GSM 5.08 $8.4)
            if (fn_mod_104==60)
            {
               l1a_l1s_com.Smeas_dedic.qual_acc_sub      += l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] & 0xffff;
               l1a_l1s_com.Smeas_dedic.qual_nbr_meas_sub += TCH_FS_BLEN;
            }

            // RXQUAL_FULL : accumulate number of estimated errors, this value is
            // contained in a_dd_0[2] field, for each TCHT block.
            l1a_l1s_com.Smeas_dedic.qual_acc_full      += l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] & 0xffff;
            l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full += TCH_FS_BLEN;

            #if TESTMODE
              if (l1_config.TestMode)
              {
                pm_fullres = (l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_PM]   & 0xffff); // F26.6

                // Allocate result message.
                msg = os_alloc_sig(sizeof(T_TMODE_TCH_INFO));
                DEBUGMSG(status,NU_ALLOC_ERR)
                msg->SignalCode = TMODE_TCH_INFO;

                ((T_TMODE_TCH_INFO *)(msg->SigP))->pm_fullres         = pm_fullres; // F26.6
                ((T_TMODE_TCH_INFO *)(msg->SigP))->snr                = snr;
                ((T_TMODE_TCH_INFO *)(msg->SigP))->toa                = toa;
                ((T_TMODE_TCH_INFO *)(msg->SigP))->angle              = (WORD16) angle; // signed
                ((T_TMODE_TCH_INFO *)(msg->SigP))->qual_full          = l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] & 0xffff;
                ((T_TMODE_TCH_INFO *)(msg->SigP))->qual_nbr_meas_full = TCH_FS_BLEN;

                // send TMODE_TCH_INFO message...
                os_send_sig(msg, L1C1_QUEUE);
                DEBUGMSG(status,NU_SEND_QUEUE_ERR)
              }
            #endif

            #if FEATURE_TCH_REROUTE
              if (tch_reroute_downlink)
                tch_send_downlink_bits(l1s_dsp_com.dsp_ndb_ptr->a_dd_0);
            #endif

            if(channel_mode == TCH_24F_MODE)
            {
              #if IDS
              // Integrated Data Services implementation
              {
                dll_data_dl(l1s_dsp_com.dsp_ndb_ptr->a_data_buf_dl,
                            &l1s_dsp_com.dsp_ndb_ptr->d_ra_act,
                             &l1s_dsp_com.dsp_ndb_ptr->d_ra_statd);
              }
              #else
              {
                // DATA traffic.
                // Pass data block to DATA ADAPTOR.
                // REM: Data packet is always given to the DATA ADAPTOR.
                // There is no RX quality check !!
                {
                  rx_tch_data(&l1s_dsp_com.dsp_ndb_ptr->a_dd_0[3], channel_mode, 0);
                }
              }
              #endif
            }

            // Reset A_DD_0 header in NDB.
            l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] = 0;
            l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] = 0xffff;
          } // if(b_blud == TRUE)
        #if (AMR == 1)
          } // if (channel_mode != TCH_AFS_MODE)
        #endif
        } // if (b_blud == TRUE) FACCH (else)
     #if (FF_REPEATED_DL_FACCH == 1)
#if 1
      temp=l1s.repeated_facch.counter_candidate;
      l1s.repeated_facch.counter_candidate=l1s.repeated_facch.counter;
      l1s.repeated_facch.counter=temp;
#else
if (l1s.repeated_facch.counter_candidate == 1)
	l1s.repeated_facch.counter_candidate = 0 ;
else if (l1s.repeated_facch.counter_candidate == 0 )
	l1s.repeated_facch.counter_candidate = 0 ;

   l1s.repeated_facch.counter++ ;
if (l1s.repeated_facch.counter == 4)
{
l1s.repeated_facch.counter = 0;
l1s.repeated_facch.pipeline[0].buffer_empty=l1s.repeated_facch.pipeline[1].buffer_empty=TRUE;
}
#endif
    #endif/*(FF_REPEATED_DL_FACCH == 1)*/
/* FACCH Full rate */
      } // if(fn_report_mod13_mod4 == 0)

      else // if(fn_report_mod13_mod4 == 0)
    #if (AMR == 1)
      if ((fn_report_mod13_mod4 == 2) && (channel_mode != TCH_AFS_MODE))
    #else
      if(fn_report_mod13_mod4 == 2)
    #endif
      // It is time to get TCH/F4.8 or TCH/F9.6 data block.
      {
        #if TRACE_TYPE==3
          if (l1_stats.type == PLAY_UL &&
             (channel_mode == TCH_48F_MODE || channel_mode == TCH_96_MODE || channel_mode == TCH_144_MODE))
            play_trace();
        #endif

        // Check A_DD_0 information block only if no FACCH.
        b_blud = (l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] & (1<<B_BLUD)) >> B_BLUD;
        if(b_blud == TRUE)
        {
          // RXQUAL_FULL : accumulate number of estimated errors, this value is
          // contained in a_dd_0[2] field, for each TCHT block.
          l1a_l1s_com.Smeas_dedic.qual_acc_full      += l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] & 0xffff;
          l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full += TCH_F_D_BLEN;

          if((channel_mode == TCH_48F_MODE) || (channel_mode == TCH_96_MODE) || (channel_mode == TCH_144_MODE))
          {
            #if IDS
            // Integrated Data Services implementation
            {
              dll_data_dl(l1s_dsp_com.dsp_ndb_ptr->a_data_buf_dl,
                          &l1s_dsp_com.dsp_ndb_ptr->d_ra_act,
                           &l1s_dsp_com.dsp_ndb_ptr->d_ra_statd);
            }
            #else
            {
              // DATA traffic.
              // Pass data block to DATA ADAPTOR.
              // REM: Data packet is always given to the DATA ADAPTOR.
              // There is no RX quality check !!
              {
                UWORD8 sequence_number;
                UWORD8 fn_report_mod26 = l1s.actual_time.fn_in_report % 26;

                // Catch sequence number. This is used in TCH/F4.8 to distinguish
                // data blocks (see GSM 5.02) received on B0,B2,B4 (sequence number 0)
                // and data blocks received on B1,B2,B3 (sequence number 1).
                if((fn_report_mod26 == 23) || (fn_report_mod26 == 6) || (fn_report_mod26 == 15))
                  sequence_number = 0;
                else
                  sequence_number = 1;
                rx_tch_data(&l1s_dsp_com.dsp_ndb_ptr->a_dd_0[3], channel_mode, sequence_number);
              }
            }
          #endif
          }
          // Reset A_DD_0 header in NDB.
          l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] = 0;
          l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] = 0xffff;
        } // if(b_blud == TRUE)
      } // if(fn_report_mod13_mod4 == 2)

    #if (AMR == 1)
      if ((channel_mode == TCH_AFS_MODE) && (facch_present == FALSE))
      {
        // Load the bit to check if the block is valid
        b_blud = (l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] & (1<<B_BLUD)) >> B_BLUD;

        // Load the bit to check if the block is a RATSCCH in case of rx_type = NO_DATA
        b_ratscch_blud = (l1s_dsp_com.dsp_ndb_ptr->a_ratscch_dl[0] & (1<<B_BLUD)) >> B_BLUD;

        // All detected AMR frames except NO_DATA (b_blud = 0) and FACCH are handled here, i.e. speech/SID/RATSCCH
        if(b_ratscch_blud==TRUE)
        {
          // RXQUAL_FULL : accumulate number of estimated errors, this value is
          // contained in a_ratscch_dl[2] field, for each TCHT block.
          l1a_l1s_com.Smeas_dedic.qual_acc_full += l1s_dsp_com.dsp_ndb_ptr->a_ratscch_dl[2] & 0xffff;

          // Reset the A_RATSCCH_DL header in NDB.
          l1s_dsp_com.dsp_ndb_ptr->a_ratscch_dl[0] = 0;
          l1s_dsp_com.dsp_ndb_ptr->a_ratscch_dl[2] = 0xffff;

          // RXQUAL_FULL : the number of bits examined for errors on serving cell depends on
          // the block received.
          l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full += RATSCCH_BLEN;

          IL_for_rxlev = l1ctl_dpagc_amr(0,beacon,(UWORD8)pm, radio_freq, IL_info_ptr);
            #if REL99
            #if FF_EMR
              l1s_read_dedic_scell_meas(IL_for_rxlev, 0, &emr_params);
            #endif
            #else
              l1s_read_dedic_scell_meas(IL_for_rxlev, 0);
            #endif
        }
        else if(b_blud==TRUE)
        {
          // Load the type of the block received
          rx_type = (l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] & RX_TYPE_MASK) >> RX_TYPE_SHIFT;

        #if (DEBUG_DEDIC_TCH_BLOCK_STAT == 1)
          Trace_dedic_tch_block_stat(rx_type, l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2], 0);
        #endif

          // RXQUAL_SUB : In case of adaptative traffic channel, accumulate number of estimated errors
          // is contained in the a_dd_0[2] value but the accumulation is made with SID_UPDATE frame only.
          // Note: SID_UPDATE frame corresponds to rx_type SID_UPDATE (b_ratscch_blud = FALSE) or SID_BAD (See Memo)
          if((rx_type==SID_UPDATE) || (rx_type==SID_BAD))

          {
            l1a_l1s_com.Smeas_dedic.qual_acc_sub      += l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] & 0xffff;
            l1a_l1s_com.Smeas_dedic.qual_nbr_meas_sub += SID_UPDATE_BLEN;
          }

          // RXQUAL_FULL : accumulate number of estimated errors, this value is
          // contained in a_dd_0[2] field, for each TCHT block.




          // Frames, which have no class1 bit (so no quality meas is possible), have d_nerr = 0
          // so we can add them
            l1a_l1s_com.Smeas_dedic.qual_acc_full += l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] & 0xffff;

          // Reset A_DD_0 header in NDB.
          l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] = 0;
          l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] = 0xffff;





          // RXQUAL_FULL : the number of bits examined for errors on serving cell depends on
          // the block received.
          if((rx_type==SPEECH_GOOD) || (rx_type==SPEECH_DEGRADED) || (rx_type==SPEECH_BAD))
          {
            // It's a speech block
            // Note: in AFS, the d_nerr value doesn't depend on the vocoder currently use
            l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full += TCH_AFS_BLEN;
          }
          else if((rx_type==SID_UPDATE) || (rx_type==SID_BAD))


          {
            // the block is a SID UPDATE frame
            l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full += SID_UPDATE_BLEN;
          }







          // AGC, TOA, RXLEV for AMR. SUB queues only for SID_UPDATE frames
          if((rx_type==SID_UPDATE) || (rx_type==SID_BAD))

          {
            IL_for_rxlev = l1ctl_dpagc_amr(1,beacon,(UWORD8)pm, radio_freq, IL_info_ptr);
            #if REL99
            #if FF_EMR
              l1s_read_dedic_scell_meas(IL_for_rxlev, 1, &emr_params);
            #endif
            #else
              l1s_read_dedic_scell_meas(IL_for_rxlev, 1);
            #endif

          #if (TOA_ALGO != 0)
            #if (TOA_ALGO == 2)
              if(l1s.toa_var.toa_snr_mask == 0)
            #else
              if(l1s.toa_snr_mask == 0)
            #endif
            {
              #if 1	/* FreeCalypso TCS211 reconstruction */
                if (IL_for_rxlev < IL_FOR_RXLEV_SNR)
                  l1s.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, snr, toa, &l1s.toa_update, &l1s.toa_period_count);
                else
                  l1s.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, 0, toa, &l1s.toa_update, &l1s.toa_period_count);
              #else
                UWORD32 snr_temp;
                snr_temp = (IL_for_rxlev < IL_FOR_RXLEV_SNR)? snr: 0;
                #if (TOA_ALGO == 2)
                  l1s.toa_var.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, snr_temp, toa);
                #else
                  l1s.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, snr_temp, toa, &l1s.toa_update, &l1s.toa_period_count
                  #if (FF_L1_FAST_DECODING == 1)
                      ,0
                  #endif
                      );
                #endif
              #endif
            }
          #endif
          }
          else
          {
            IL_for_rxlev = l1ctl_dpagc_amr(0,beacon,(UWORD8)pm, radio_freq, IL_info_ptr);
            #if REL99
            #if FF_EMR
              l1s_read_dedic_scell_meas(IL_for_rxlev, 0, &emr_params);
            #endif
            #else
              l1s_read_dedic_scell_meas(IL_for_rxlev, 0);
            #endif
          }
        } // if(b_blud==TRUE)
        // NO_DATA block detected or simple burst
        else
        {
          if (fn_report_mod13_mod4 == 0)
          {
            l1a_l1s_com.Smeas_dedic.qual_acc_full += l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] & 0xffff;
            l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full += TCH_AFS_BLEN;

          #if (DEBUG_DEDIC_TCH_BLOCK_STAT == 1)
            Trace_dedic_tch_block_stat(AMR_NO_DATA, l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2], 0);
          #endif

            // Reset A_DD_0 header in NDB.
            l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0] = 0;
            l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2] = 0xffff;
          }

          // Update AGC: Call DPAGC AMR algorithm in order to fill the G_all buffer
          IL_for_rxlev = l1ctl_dpagc_amr(0,beacon,(UWORD8)pm,radio_freq,IL_info_ptr);

          // Dedicated mode serving cell measurement reading, full set only.
          #if REL99
          #if FF_EMR
            l1s_read_dedic_scell_meas(IL_for_rxlev, 0, &emr_params);
          #endif
          #else
            l1s_read_dedic_scell_meas(IL_for_rxlev, 0);
          #endif
        }
      } // if ((channel_mode == TCH_AFS_MODE) && (facch_present == FALSE))
    #endif

      // task is completed, make it INACTIVE.
      l1s.task_status[task].current_status = INACTIVE;
    }
    break;

    case TCHA:
    /*---------------------------------------------------*/
    /* Dedicated mode: SACCH receive task.               */
    /*---------------------------------------------------*/
    {
      UWORD8   IL_for_rxlev;
      UWORD32  b_blud;

      // Read control results and feed control algorithms.
      // **************************************************

      // Read control information.
      toa   = l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_TOA]   & 0xffff;
      pm    = (l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_PM]   & 0xffff) >> 5;
      angle = l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_ANGLE] & 0xffff;
      snr   = l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[D_SNR]   & 0xffff;

      #if TESTMODE
        if (l1_config.TestMode && l1_config.tmode.rf_params.down_up == TMODE_UPLINK)
        {
          // For UL-only tasks, TCHA is scheduled in every frame. TCH_INFO message is only
          // used to count loops; no stats are collected.

          xSignalHeaderRec *msg;
          // Allocate result message.
          msg = os_alloc_sig(sizeof(T_TMODE_TCH_INFO));
          DEBUGMSG(status,NU_ALLOC_ERR)
          msg->SignalCode = TMODE_TCH_INFO;
          // send TMODE_TCH_INFO message...
          os_send_sig(msg, L1C1_QUEUE);
          DEBUGMSG(status,NU_SEND_QUEUE_ERR)
        }
        // WARNING!
        // Don't trace PM=0 during UL-only in TestMode. The DSP is not working
        // in that case so it is normal. However, if tracing happens the CPU overloads
        if (l1_config.TestMode && l1_config.tmode.rf_params.down_up & TMODE_DOWNLINK)
      #endif
        {
            l1_check_pm_error(pm, task);
        }

      #if TRACE_TYPE==3
        stats_samples_tch_sacch(toa,pm,angle,snr);
      #endif

      #if (TRACE_TYPE==2 ) || (TRACE_TYPE==3)
        uart_trace(task);
      #endif

      // Update AGC: Call DPAGC algorithm
      IL_for_rxlev = l1ctl_dpagc(1,beacon,(UWORD8)pm,radio_freq,IL_info_ptr); // dtx_on = 1

      // Dedicated mode serving cell measurement reading, indicate "SUB".
      #if REL99
      #if FF_EMR
        l1s_read_dedic_scell_meas(IL_for_rxlev, 1, &emr_params);
      #endif
      #else
        l1s_read_dedic_scell_meas(IL_for_rxlev, 1);
      #endif

      // Update AFC: Call AFC control function (KALMAN filter).
      #if AFC_ALGO
        #if TESTMODE
          if (l1_config.afc_enable)
        #endif
          {
            #if (VCXO_ALGO == 0)
              l1s.afc = l1ctl_afc(AFC_CLOSED_LOOP, &l1s.afc_frame_count, (WORD16)angle, snr, radio_freq);
            #else
              l1s.afc = l1ctl_afc(AFC_CLOSED_LOOP, &l1s.afc_frame_count, (WORD16)angle, snr, radio_freq,l1a_l1s_com.mode);
            #endif
          }
      #endif

      //Feed TOA histogram.
      #if (TOA_ALGO != 0)
        #if (TOA_ALGO == 2)
          if(l1s.toa_var.toa_snr_mask == 0)
        #else
          if(l1s.toa_snr_mask == 0)
        #endif
        {
          #if 1	/* FreeCalypso TCS211 reconstruction */
            if (IL_for_rxlev < IL_FOR_RXLEV_SNR)
              l1s.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, snr, toa, &l1s.toa_update, &l1s.toa_period_count);
            else
              l1s.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, 0, toa, &l1s.toa_update, &l1s.toa_period_count);
          #else
            UWORD32 snr_temp;
            snr_temp = (IL_for_rxlev < IL_FOR_RXLEV_SNR)? snr: 0;
            #if (TOA_ALGO == 2)
              l1s.toa_var.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, snr_temp, toa);
            #else
              l1s.toa_shift = l1ctl_toa(TOA_RUN, l1a_l1s_com.mode, snr_temp, toa, &l1s.toa_update, &l1s.toa_period_count
              #if (FF_L1_FAST_DECODING == 1)
                  ,0
              #endif
                  );
            #endif
          #endif
        }
      #endif

       #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
         RTTL1_FILL_DL_BURST(angle, snr, l1s.afc, task, pm, toa, IL_for_rxlev)
       #endif
       #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4)) && HAVE_L1_TRACE_BURST_PARAM
         l1_trace_burst_param(angle, snr, l1s.afc, task, pm, toa, IL_for_rxlev);
       #endif
       #if (BURST_PARAM_LOG_ENABLE == 1)
         l1_log_burst_param(angle, snr, l1s.afc, task, pm, toa, IL_for_rxlev);
       #endif

      // Read downlink DATA block from MCU/DSP interface.
      // *************************************************

      if(l1s.actual_time.fn_in_report == 91)
      // It's time to read a SACCH DL result from DSP.
      {
        // Check A_CD information block.
        b_blud = (l1s_dsp_com.dsp_ndb_ptr->a_cd[0] & (1<<B_BLUD)) >> B_BLUD;
        if(b_blud == TRUE)
        {
          #if W_A_DSP1
      // Temporary correction to fix a known DSP problem.  SACCH deinterleaver not
          // initialized on HO.
      //
            if (old_sacch_DSP_bug == TRUE)
            {
            // Invalidate the current sacch block - indicate it cannot be decoded
              l1s_dsp_com.dsp_ndb_ptr->a_cd[0]   =  (1<<B_FIRE1); // B_FIRE1=1,B_FIRE0=0,BLUD=0.
              old_sacch_DSP_bug = FALSE;
            }
          #endif

          // Read data block and send msg to L1A.
          l1s_read_sacch_dl(l1s_dsp_com.dsp_ndb_ptr->a_cd, task);

          // RXQUAL_FULL/RXQUAL_SUB : Accumulate number of estimated errors, this value
          // is contained in a_cd[2] field, for every SACCH block.
          l1a_l1s_com.Smeas_dedic.qual_acc_full      += l1s_dsp_com.dsp_ndb_ptr->a_cd[2] & 0xffff;
          l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full += A_D_BLEN;
          l1a_l1s_com.Smeas_dedic.qual_acc_sub       += l1s_dsp_com.dsp_ndb_ptr->a_cd[2] & 0xffff;
          l1a_l1s_com.Smeas_dedic.qual_nbr_meas_sub  += A_D_BLEN;

          // TEMPORARY : reset buffers and flags in NDB ...
          //             reset nerr....
          //             reset A_CD contents.......
          l1s_dsp_com.dsp_ndb_ptr->a_cd[0]   =  (1<<B_FIRE1); // B_FIRE1=1,B_FIRE0=0,BLUD=0.
          l1s_dsp_com.dsp_ndb_ptr->a_cd[2]   =  0xffff;
        }
      }
      #if W_A_DSP1
        else if (l1s.actual_time.fn_in_report == 13)  // TF 5/8/98 - DSP fix
        {
          // As this is the first SACCH burst the known DSP bug cannot occur on a new channel.
          old_sacch_DSP_bug = FALSE;
        }
      #endif

      // task is completed, make it INACTIVE.
      l1s.task_status[task].current_status = INACTIVE;
    }
    break;
  } // End switch...

  #if (L1_DEBUG_IQ_DUMP == 1)
    l1ddsp_read_iq_dump(task);
  #endif

  // Flag the use of the MCU/DSP dual page read interface.
  // ******************************************************

  // Set flag used to change the read page at the end of "l1_synch".
  l1s_dsp_com.dsp_r_page_used = TRUE;
}

/*-------------------------------------------------------*/
/* l1s_read_tx_result()                                  */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1s_read_tx_result(UWORD8 task, UWORD8 burst_id)
{
  /*--------------------------------------------------------*/
  /* READ TRANSMIT TASK RESULTS...                          */
  /*--------------------------------------------------------*/

  #if (TRACE_TYPE!=0)
    if(task==RAACC)
    {
      if((UWORD32)(l1s_dsp_com.dsp_db_r_ptr->d_task_ra & 0xffff) != (UWORD32)DSP_TASK_CODE[task])
        trace_fct(CST_UL_TASKS_DO_NOT_CORRESPOND, -1);
    }
    else
    {
      if(((UWORD32)(l1s_dsp_com.dsp_db_r_ptr->d_task_u & 0xffff) != (UWORD32)DSP_TASK_CODE[task]) &&
         ((UWORD32)(l1s_dsp_com.dsp_db_r_ptr->d_task_u & 0xffff) != TCH_DTX_UL))
        trace_fct(CST_UL_TASKS_DO_NOT_CORRESPOND, -1);
    }
  #endif

  l1_check_com_mismatch(task);

  switch(task)
  {
    case RAACC:
    /*---------------------------------------------------*/
    /* Serving Cell: Random Access TX task.              */
    /*---------------------------------------------------*/
    // Rem: confirmation message is sent at "CTRL" to be able to give FN%42432.
    {
      // Send confirmation msg to L1A.
      // ******************************

      // For ACCESS phase, a confirmation msg is sent to L1A.
      xSignalHeaderRec *msg;

      // send L1C_RA_DONE to L1A...
      msg = os_alloc_sig(sizeof(T_MPHC_RA_CON));
      DEBUGMSG(status,NU_ALLOC_ERR)

      if (l1s.actual_time.fn == 0)
        ((T_MPHC_RA_CON *)(msg->SigP))->fn = MAX_FN - 1;
      else
        ((T_MPHC_RA_CON *)(msg->SigP))->fn = l1s.actual_time.fn - 1;

      ((T_MPHC_RA_CON *)(msg->SigP))->channel_request = l1a_l1s_com.ra_info.channel_request;
      msg->SignalCode = L1C_RA_DONE;

      os_send_sig(msg, L1C1_QUEUE);
      DEBUGMSG(status,NU_SEND_QUEUE_ERR)

      // Desactivate the RAACC task.
      l1s.task_status[task].current_status = INACTIVE;

      #if (TRACE_TYPE!=0)
        trace_fct(CST_L1S_READ_RA, l1a_l1s_com.Scell_info.radio_freq);
      #endif
    }
    break;

    case DUL:
    /*---------------------------------------------------*/
    /* Serving Cell: SDCCH up link.                      */
    /*---------------------------------------------------*/
    {
      // Desactivate UL task.
      if(burst_id == BURST_4)
      {
        l1s.task_status[task].current_status = INACTIVE;

        #if (TRACE_TYPE == 5) // in simulation only the 4th burst is traced
          trace_fct(CST_L1S_READ_TX_NB__DUL, l1a_l1s_com.Scell_info.radio_freq);
        #endif
      }

      #if (TRACE_TYPE!=0) && (TRACE_TYPE!=5)
        trace_fct(CST_L1S_READ_TX_NB__DUL, l1a_l1s_com.Scell_info.radio_freq);
      #endif
    }
    break;

    case AUL:
    /*---------------------------------------------------*/
    /* Serving Cell: SACCH up link.                      */
    /*---------------------------------------------------*/
    {
      // Desactivate UL task.
      if(burst_id == BURST_4)
      {
        l1s.task_status[task].current_status = INACTIVE;

        #if (TRACE_TYPE == 5) // in simulation only the 4th burst is traced
          trace_fct(CST_L1S_READ_TX_NB__AUL, l1a_l1s_com.Scell_info.radio_freq);
        #endif
      }

      #if (TRACE_TYPE!=0) && (TRACE_TYPE!=5)
        trace_fct(CST_L1S_READ_TX_NB__AUL, l1a_l1s_com.Scell_info.radio_freq);
      #endif

    }
    break;

    case TCHA:
    case TCHTF:
    /*---------------------------------------------------*/
    /* Serving Cell: TCH link.                           */
    /*---------------------------------------------------*/
    {
      #if (TRACE_TYPE==5)
        if(burst_id == BURST_4) // in simulation only the 4th burst is traced
          trace_fct(CST_L1S_READ_TX_NB__TCHF, l1a_l1s_com.Scell_info.radio_freq);
      #endif

      #if (TRACE_TYPE!=0) && (TRACE_TYPE!=5)
          trace_fct(CST_L1S_READ_TX_NB__TCHF, l1a_l1s_com.Scell_info.radio_freq);
      #endif
    }
    break;

    case TCHTH:
    /*---------------------------------------------------*/
    /* Serving Cell: TCH link.                           */
    /*---------------------------------------------------*/
    {
      #if (TRACE_TYPE==5)
        if(burst_id == BURST_2) // in simulation only lates burst is traced
          trace_fct(CST_L1S_READ_TX_NB__TCHH, l1a_l1s_com.Scell_info.radio_freq);
      #endif

      #if (TRACE_TYPE!=0) && (TRACE_TYPE!=5)
          trace_fct(CST_L1S_READ_TX_NB__TCHH, l1a_l1s_com.Scell_info.radio_freq);
      #endif
    }
    break;
  }

  // Set flag used to change the read page at the end of "l1_synch".
  l1s_dsp_com.dsp_r_page_used = TRUE;
}

/*-------------------------------------------------------*/
/* l1s_read_dedic_scell_meas()                           */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/

#if REL99
#if FF_EMR
void l1s_read_dedic_scell_meas(UWORD8 input_level, UWORD8 sub_flag, T_EMR_PARAMS *emr_params)
{
  UWORD8  task;
  UWORD8  burst_id;
  UWORD8  b_blud;
  UWORD8  counter;
  UWORD8  bfi;                  // band frame indicator
  UWORD8  channel_mode;         // current channel type
  //OMAPS00090550 UWORD8  fn_report_mod_26;
  UWORD8  subchannel;           // half rate sub channel
  UWORD8  sid_present;          // indication for sid block
  UWORD16 rx_type;
  UWORD32 normalised_fn_mod13_mod4;

  UWORD16 mean_bep_lsb = 0;     //l1s_dsp_com.dsp_ndb_ptr->d_mean_bep_block_lsb;
  UWORD16 mean_bep_msb = 0;     //l1s_dsp_com.dsp_ndb_ptr->d_mean_bep_block_msb;
  UWORD32 mean_bep = 0;         //((mean_bep_msb<<WORD_SHIFT)|(mean_bep_lsb)) >> MEAN_BEP_FORMAT;
  UWORD16 cv_bep = 0;           //(l1s_dsp_com.dsp_ndb_ptr->d_cv_bep_block) >> CV_BEP_FORMAT;

  static WORD16 last_corr_decoded_burst[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  static UWORD16 cv_bep_tch4_8f_144[2]   = {0, 0};
  static UWORD32 mean_bep_tch4_8f_144[2] = {0, 0};

  WORD16 rxlev = l1s_encode_rxlev(input_level);

  if (l1s_dsp_com.dsp_r_page == 1)
  {
      cv_bep       = l1s_dsp_com.dsp_ndb_ptr->a_mean_cv_bep_page_0[D_CV_BEP];
      mean_bep_msb = l1s_dsp_com.dsp_ndb_ptr->a_mean_cv_bep_page_0[D_MEAN_BEP_MSW];
      mean_bep_lsb = l1s_dsp_com.dsp_ndb_ptr->a_mean_cv_bep_page_0[D_MEAN_BEP_LSW];
  }
  else
  {
      cv_bep       = l1s_dsp_com.dsp_ndb_ptr->a_mean_cv_bep_page_1[D_CV_BEP];
      mean_bep_msb = l1s_dsp_com.dsp_ndb_ptr->a_mean_cv_bep_page_1[D_MEAN_BEP_MSW];
      mean_bep_lsb = l1s_dsp_com.dsp_ndb_ptr->a_mean_cv_bep_page_1[D_MEAN_BEP_LSW];
  }

  mean_bep = ((mean_bep_msb<<WORD_SHIFT)|(mean_bep_lsb)) >> MEAN_BEP_FORMAT;
  cv_bep   = cv_bep >> CV_BEP_FORMAT;

  // EMR : Copy of Legacy code begins
  // Measurement must be rejected if channel is hopping, hopped on
  // the beacon frequency and PWRC is TRUE (see GSM05.08, $8.1.3).
  if(!((l1a_l1s_com.dedic_set.pwrc == TRUE) &&
       (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->chan_sel.h == TRUE) &&
       (l1a_l1s_com.dedic_set.radio_freq_dd == l1a_l1s_com.Scell_info.radio_freq)))
  {
    // Add to FULL set meas.
    l1a_l1s_com.Scell_info.meas.nbr_meas++;
    l1a_l1s_com.Scell_info.meas.acc += rxlev;

    if(sub_flag == TRUE)
    {
      // Add to SUB set meas.
      l1a_l1s_com.Smeas_dedic.nbr_meas_sub++;
      l1a_l1s_com.Smeas_dedic.acc_sub += rxlev;
    } // if(sub_flag == TRUE)
    // EMR : Copy of Legacy code ends
  } // if(!((l1a_l1s_com.dedic_set.pwrc == TRUE) && ....

  // new rxlev is received. remove the oldest rxlev.
  for(counter=0;counter<=6;counter++)
    last_corr_decoded_burst[counter] = last_corr_decoded_burst[counter+1];

  // store new rxlev.
  last_corr_decoded_burst[7] = rxlev;

  task                            = emr_params->task;
  burst_id                        = emr_params->burst_id;
  channel_mode                    = emr_params->channel_mode;
  normalised_fn_mod13_mod4 = emr_params->normalised_fn_mod13_mod4;

  // TCH FS and TCH EFR
  if((task == TCHTF) &&
     (l1s.actual_time.fn_mod13_mod4 == 0) &&
     ((channel_mode == TCH_FS_MODE) ||
      (channel_mode == TCH_EFR_MODE) ||
      (channel_mode == SIG_ONLY_MODE)))
  {
    if(emr_params->facch_present == TRUE)
    {
      // FACCH
      if(emr_params->facch_fire1 == FALSE)
      {
        // FACCH correctly decoded
        l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas +=4; // 4 bursts are accumulated

        // Accumulate BEP
        l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc += mean_bep;
        l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc   += cv_bep;
        l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num++;
        l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num++;

        //accumulation of the correctly decoded block
        l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc += last_corr_decoded_burst[4] +
                                                       last_corr_decoded_burst[5] +
                                                       last_corr_decoded_burst[6] +
                                                       last_corr_decoded_burst[7];

        if(channel_mode == SIG_ONLY_MODE)
        // accumulation of correctly decoded blocks excluding SACCH  and SID frames FACCH only for sig only mode
          l1a_l1s_com.Smeas_dedic_emr.nbr_rcvd_blocks++;

      } // if(facch_fire1 == FALSE)
    } // if(facch_present == TRUE )
    else
    {
      // NOT FACCH,
      if (emr_params->a_dd_0_blud == TRUE)
      {
        if (emr_params->a_dd_0_bfi == FALSE)
        {
          // speech correctly decoded
          l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas +=4;   // 4 bursts are accumulated

          // Accumulate BEP
          l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc += mean_bep;
          l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc   += cv_bep;
          l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num++;
          l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num++;

          //accumulation of the correctly decoded block
          l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc += last_corr_decoded_burst[4] +
                                                       last_corr_decoded_burst[5] +
                                                       last_corr_decoded_burst[6] +
                                                       last_corr_decoded_burst[7];

          if (emr_params->sid_present_sub0 == FALSE)
          {
            // accumulation of correctly decoded blocks excluding SACCH FACCH and SID frames
            l1a_l1s_com.Smeas_dedic_emr.nbr_rcvd_blocks++;
          } // if(sid_present == FALSE)
        } // if(bfi == FALSE)
      } // if(b_blud == TRUE)
    } // else part of if(facch_present == TRUE )
  } // TCH FS and TCH EFR

  // TCH 2.4F
  if((task == TCHTF) &&
     (l1s.actual_time.fn_mod13_mod4 == 0) &&
     (channel_mode == TCH_24F_MODE))
  {
    if(emr_params->facch_present == TRUE)
    {
      if(emr_params->facch_fire1 == FALSE)
      {
        // FACCH correctly decoded
        l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas +=4;   // 4 bursts are accumulated

        // Accumulate BEP
        l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc += mean_bep;
        l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc   += cv_bep;
        l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num++;
        l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num++;

        //accumulation of the correctly decoded block
        l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc += last_corr_decoded_burst[4] +
                                                     last_corr_decoded_burst[5] +
                                                     last_corr_decoded_burst[6] +
                                                     last_corr_decoded_burst[7];
      }
    } // if(facch_present == TRUE)
    else
    {
      // NOT FACCH check whether the data buffer is updated!!
      if(emr_params->a_dd_0_blud == TRUE)
      {
        // Check if transparent data or not
        if(emr_params->b_ce == TRUE)// Non Transparent
        {
          //check if correctly decoded or not
          if(emr_params->a_ntd == FALSE)//good frame detected
          {
            l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas +=4;   // 4 bursts are accumulated

            // Accumulate BEP
            l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc += mean_bep;
            l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc   += cv_bep;
            l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num++;
            l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num++;

            //accumulation of the correctly decoded block
            l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc += last_corr_decoded_burst[4] +
                                                         last_corr_decoded_burst[5] +
                                                         last_corr_decoded_burst[6] +
                                                         last_corr_decoded_burst[7];
            // accumulation of correctly decoded blocks excluding SACCH FACCH frames
            l1a_l1s_com.Smeas_dedic_emr.nbr_rcvd_blocks++;
          } // if(a_ntd == FALSE)
        } // if(b_ce == TRUE)
        else
        {
          // 2.4F transperent data (always considered as correctly decoded)
          l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas +=4;   // 4 bursts are accumulated

          // Accumulate BEP
          l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc += mean_bep;
          l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc   += cv_bep;
          l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num++;
          l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num++;

          //accumulation of the block
          l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc += last_corr_decoded_burst[4] +
                                                       last_corr_decoded_burst[5] +
                                                       last_corr_decoded_burst[6] +
                                                       last_corr_decoded_burst[7];

          // accumulation of decoded blocks excluding SACCH FACCH and SID frames
          l1a_l1s_com.Smeas_dedic_emr.nbr_rcvd_blocks++;
        } // if(b_ce == TRUE)
      } // if(b_blud == TRUE)
    } // else part of if(facch_present == TRUE)
  } // TCH 2.4F

  // TCH 9.6F
  if((task == TCHTF) &&
     (l1s.actual_time.fn_mod13_mod4 == 0) &&
     (channel_mode == TCH_96_MODE))
  {
    if(emr_params->facch_present == TRUE)
    {
      if(emr_params->facch_fire1 == FALSE)
      {
        // FACCH correctly decoded
        l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas +=4;   // 4 bursts are accumulated

        // Accumulate BEP
        l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc += mean_bep;
        l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc   += cv_bep;
        l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num++;
        l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num++;

        //accumulation of the correctly decoded block
        l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc += last_corr_decoded_burst[4] +
                                                     last_corr_decoded_burst[5] +
                                                     last_corr_decoded_burst[6] +
                                                     last_corr_decoded_burst[7];
      } //if(fire1 == FALSE)
    } //if(facch_present == TRUE)
  } // if(fn_mod13_mod4 == 0)

  if((task == TCHTF) &&
     (l1s.actual_time.fn_mod13_mod4 == 2) &&
     (channel_mode == TCH_96_MODE))
  {
    if(emr_params->a_dd_0_blud == TRUE)
    {
      // Check if transparent data or not
      if(emr_params->b_ce == TRUE)// Non Transparent
      {
        //check if correctly decoded or not
        if(emr_params->a_ntd == FALSE)//good frame detected
        {
          l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas +=4;   // 4 bursts are accumulated

          // Accumulate BEP
          l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc += mean_bep;
          l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc   += cv_bep;
          l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num++;
          l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num++;

          //accumulation of the correctly decoded block
          l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc += last_corr_decoded_burst[4] +
                                                       last_corr_decoded_burst[5] +
                                                       last_corr_decoded_burst[6] +
                                                       last_corr_decoded_burst[7];

          // accumulation of correctly decoded blocks excluding SACCH FACCH frames
          l1a_l1s_com.Smeas_dedic_emr.nbr_rcvd_blocks++;
        } // if(a_ntd == FALSE)
      } // if(b_ce == TRUE)
      else
      {
        // transparent data (always correctly decoded)
        l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas +=4;   // 4 bursts are accumulated

        // Accumulate BEP
        l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc += mean_bep;
        l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc   += cv_bep;
        l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num++;
        l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num++;

        //accumulation of decoded block
        l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc += last_corr_decoded_burst[4] +
                                                     last_corr_decoded_burst[5] +
                                                     last_corr_decoded_burst[6] +
                                                     last_corr_decoded_burst[7];

        // accumulation of decoded blocks excluding SACCH FACCH frames
        l1a_l1s_com.Smeas_dedic_emr.nbr_rcvd_blocks++;
      } // transparent data
    } // if(b_blud == TRUE)
  } // TCH F9.6

  // TCH 4.8F/14.4F
  if((task == TCHTF) &&
     (l1s.actual_time.fn_mod13_mod4 == 0) &&
     ((channel_mode == TCH_48F_MODE) || (channel_mode == TCH_144_MODE)) )
  {
    if(emr_params->facch_present == TRUE)
    {
      if(emr_params->facch_fire1 == FALSE)
      {
        // FACCH correctly decoded
        l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas +=4;// 4 bursts are accumulated

        // Accumulate BEP
        l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc += mean_bep;
        l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc   += cv_bep;
        l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num++;
        l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num++;

        //accumulation of the correctly decoded block
        l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc += last_corr_decoded_burst[4] +
                                                     last_corr_decoded_burst[5] +
                                                     last_corr_decoded_burst[6] +
                                                     last_corr_decoded_burst[7];
      } //if(fire1 == FALSE)
    } //if(facch_present == TRUE)
  } // if(fn_mod13_mod4 == 0)

  if((task == TCHTF) &&
     (l1s.actual_time.fn_mod13_mod4 == 2) &&
     ((channel_mode == TCH_48F_MODE) || (channel_mode == TCH_144_MODE)) )
  {
    // block end add new value of mean_bep and cv_bep value to mean_bep_tch4_8f_144 and
    // cv_bep_tch4_8f_144. remove the oldest value.
    mean_bep_tch4_8f_144[0] = mean_bep_tch4_8f_144[1];
    cv_bep_tch4_8f_144[0]   = cv_bep_tch4_8f_144[1];
    mean_bep_tch4_8f_144[1] = mean_bep;
    cv_bep_tch4_8f_144[1]   = cv_bep;

    if ( ((emr_params->a_dd_0_blud == TRUE) && (emr_params->b_m1) && (channel_mode == TCH_144_MODE)) ||
         ((emr_params->a_dd_0_blud == TRUE) && (emr_params->b_f48blk_dl) && (channel_mode == TCH_48F_MODE)) )
    {
      // Check if transparent data or not
      if(emr_params->b_ce == TRUE) // Non transparent
      {
        //check if correctly decoded or not. Accumulate last 8 slots
        if(emr_params->a_ntd == FALSE) //good frame detected
        {
          // two blocks are accumulated at a time, increment by 2.
          l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas+=8;   // 8 bursts are accumulated

          // Accumulate BEP
          l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc += mean_bep_tch4_8f_144[0] + mean_bep_tch4_8f_144[1];
          l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc   += cv_bep_tch4_8f_144[0] + cv_bep_tch4_8f_144[1];

          // two blocks are accumulated at a time, increment by 2.
          l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num += 2;
          l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num+=2;

          //accumulation of the correctly decoded block
          l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc += last_corr_decoded_burst[0] +
                                                       last_corr_decoded_burst[1] +
                                                       last_corr_decoded_burst[2] +
                                                       last_corr_decoded_burst[3] +
                                                       last_corr_decoded_burst[4] +
                                                       last_corr_decoded_burst[5] +
                                                       last_corr_decoded_burst[6] +
                                                       last_corr_decoded_burst[7];

          // accumulation of correctly decoded blocks excluding SACCH FACCH frames
          l1a_l1s_com.Smeas_dedic_emr.nbr_rcvd_blocks +=2;
        } // if(a_ntd == FALSE)
      } // if(b_ce == TRUE)
      else
      {
        // transparent data
        l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas +=8;   // 8 bursts are accumulated

        // Accumulate BEP
        l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc += mean_bep_tch4_8f_144[0] + mean_bep_tch4_8f_144[1];
        l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc   += cv_bep_tch4_8f_144[0] + cv_bep_tch4_8f_144[1];

        // two blocks are accumulated at a time, increment by 2.
        l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num+=2;
        l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num += 2;

        //accumulation of the correctly decoded block
        l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc += last_corr_decoded_burst[0] +
                                                     last_corr_decoded_burst[1] +
                                                     last_corr_decoded_burst[2] +
                                                     last_corr_decoded_burst[3] +
                                                     last_corr_decoded_burst[4] +
                                                     last_corr_decoded_burst[5] +
                                                     last_corr_decoded_burst[6] +
                                                     last_corr_decoded_burst[7];

        // accumulation of correctly decoded blocks excluding SACCH FACCH frames
        l1a_l1s_com.Smeas_dedic_emr.nbr_rcvd_blocks +=2;
      } // Transparent data
    } // if(b_blud == TRUE)
  } // TCH 4.8F/14.4


  // TCH HS
  if((task == TCHTH) &&
     ((channel_mode == TCH_HS_MODE)||(channel_mode == SIG_ONLY_MODE))&&
     (normalised_fn_mod13_mod4 == 3) )
  {
    UWORD8 norm_fn_mod26;

    subchannel = emr_params->subchannel;
    norm_fn_mod26 = ((l1s.actual_time.fn - subchannel + 26)  % 26);

    if(subchannel == 0)
    {
      b_blud      = emr_params->a_dd_0_blud;
      bfi         = emr_params->a_dd_0_bfi;       // 3rd bit tells the BAD frame.
      sid_present = emr_params->sid_present_sub0; // find out whether sid1 is 0/1, 1 mean
    }
    else
    {
      b_blud      = emr_params->a_dd_1_blud;
      bfi         = emr_params->a_dd_1_bfi;       // 3rd bit tells the BAD frame.
      sid_present = emr_params->sid_present_sub1; // find out whether sid1 is 0/1, 1 mean
    }

    if(norm_fn_mod26 == 7 || norm_fn_mod26 == 16|| norm_fn_mod26 == 24 )
    {
      // FACCH: Check A_FD information block.
      if(emr_params->facch_present == TRUE)
      {
        // FACCH correctly decoded
        if(emr_params->facch_fire1 == FALSE)
        {
          // Accumulate BEP
          l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc += mean_bep;
          l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc   += cv_bep;
          l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num++;
          l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num++;
          //accumulation of the correctly decoded block
          l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc += last_corr_decoded_burst[4] +
                                                       last_corr_decoded_burst[5] +
                                                       last_corr_decoded_burst[6] +
                                                       last_corr_decoded_burst[7];

          l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas +=4;   // 4 bursts are accumulated

          if((channel_mode == SIG_ONLY_MODE))
            // accumulation of correctly decoded blocks excluding SACCH
            // and SID frames FACCH only for sig only mode
            l1a_l1s_com.Smeas_dedic_emr.nbr_rcvd_blocks++;
        }  //if(fire1 == FALSE)
      }  //if(facch_present ==....)
      else
      {
        // No Facch at this positions 7,16,24
        if (b_blud == TRUE)
        {
          if(bfi == FALSE)
          {
            l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas +=4;   // 4 bursts are accumulated

            // Accumulate BEP
            l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc += mean_bep;
            l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc   += cv_bep;
            l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num++;
            l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num++;
            //accumulation of the correctly decoded block
            l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc += last_corr_decoded_burst[7] +
                                                         last_corr_decoded_burst[6] +
                                                         last_corr_decoded_burst[5] +
                                                         last_corr_decoded_burst[4] ;

            //as per standard 05.08 section 8.3
            //sid_present can become true only at (fn modulo 26) == 6
            //sid_present will be false at all other points

            if(sid_present == FALSE)
              // accumulation of correctly decoded blocks excluding SACCH
              // FACCH and SID frames
              l1a_l1s_com.Smeas_dedic_emr.nbr_rcvd_blocks++;
          } // if(bfi == FALSE)
        } // if (b_blud == TRUE)
      }  // else facch_present
    } //if(norm_fn_report_mod26 == 7 || norm_fn_report_mod26 == 16|| norm_fn_report_mod26 == 24 )
    else
    {
      //norm_fn_report_mod26 == 3 || norm_fn_report_mod26 == 11|| norm_fn_report_mod26 == 20
      if (b_blud == TRUE)
      {
        if(bfi == FALSE)
        {
          l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas +=4;   // 4 bursts are accumulated

          // Accumulate BEP
          l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc += mean_bep;
          l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc   += cv_bep;
          l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num++;
          l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num++;
          //accumulation of the correctly decoded block
          l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc += last_corr_decoded_burst[7] +
                                                       last_corr_decoded_burst[6] +
                                                       last_corr_decoded_burst[5] +
                                                       last_corr_decoded_burst[4] ;

           //as per standard 05.08 section 8.3
           //sid_present can become true only at (fn modulo 26) == 6
           //sid_present will be false at all other points

           if(sid_present == FALSE)
             // accumulation of correctly decoded blocks excluding SACCH
             // FACCH and SID frames
             l1a_l1s_com.Smeas_dedic_emr.nbr_rcvd_blocks++;
        }  //if(bfi == FALSE)
      } // if (b_blud == TRUE)
    }  //else
  }//task == TCHTH.....

  // SACCH of TCH
  if((task == TCHA))
  {
    //unconditionnal accumulation of rxlev_val
    l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas++;
    l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc += rxlev;
    if(l1s.actual_time.fn_in_report == 91)
    {
      // Set detection flag.
      if(emr_params->a_cd_fire1 == FALSE)
      {
        // rec 05.08 8.2.3: BEP value need to be accumulated on correctly received blocks
        // rec 05.08 8.4.8.2 : for SACCH of TCH, no accumulation for CV_BEP and nbr_rcvd_blocks
        l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc += mean_bep;
        l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num++;
      } // if(fire1 == FALSE)
    } // if(l1s.actual_time.fn_in_report == 91)
  } // // SACCH of TCH

  // SDCCH and SACCH of SDCCH
  if (((task == DDL) || (task == ADL)) && (burst_id == BURST_4))
  {
    //unconditional accumulation of Rxlev_val
    l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas +=4;   // 4 bursts are accumulated
    l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc += last_corr_decoded_burst[4] +
                                                 last_corr_decoded_burst[5] +
                                                 last_corr_decoded_burst[6] +
                                                 last_corr_decoded_burst[7];

    // rec 05.08 8.2.3: in SDCCH, BEP value need to be accumulated on correctly received blocks
    if(emr_params->a_cd_fire1 == FALSE)
    {
      l1a_l1s_com.Smeas_dedic_emr.nbr_rcvd_blocks++;
      l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc += mean_bep;
      l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc   += cv_bep;
      l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num++;
      l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num++;
    }
  } // SDCCH

  //AMR FS
  #if (AMR == 1)
    if((task == TCHTF) &&
       (channel_mode == TCH_AFS_MODE) &&
       (l1s.actual_time.fn_mod13_mod4 == 0))
    {
      if(emr_params->amr_facch_present == TRUE)
      {
        // FACCH present
        // FACCH correctly decoded ?
        if(emr_params->amr_facch_fire1 == FALSE)
        {
          l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas +=4;   // 4 bursts are accumulated

          // Accumulate BEP
          l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc += mean_bep;
          l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc   += cv_bep;
          l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num++;
          l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num++;

          //accumulation of the correctly decoded block
          l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc += last_corr_decoded_burst[4] +
                                                       last_corr_decoded_burst[5] +
                                                       last_corr_decoded_burst[6] +
                                                       last_corr_decoded_burst[7];
        } // if(fire1 == FALSE)
      } // if(facch_present == TRUE)
      else
      {
        // NOT FACCH
        if ((emr_params->b_ratscch_blud == TRUE) && (emr_params->ratscch_rxtype == RATSCCH_GOOD))
        {
          //RATSCCH correctly decoded
          l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas +=4;   // 4 bursts are accumulated
          // Accumulate BEP
          l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc += mean_bep;
          l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc   += cv_bep;
          l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num++;
          l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num++;

          //accumulation of the correctly decoded block
          l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc += last_corr_decoded_burst[4] +
                                                       last_corr_decoded_burst[5] +
                                                       last_corr_decoded_burst[6] +
                                                       last_corr_decoded_burst[7];
        } // if(b_ratscch_blud == TRUE)
        else if(emr_params->a_dd_0_blud == TRUE)
        {
          // Good speech frame.
          if((emr_params->amr_rx_type_sub0 == SPEECH_GOOD) ||
             (emr_params->amr_rx_type_sub0 == SPEECH_DEGRADED) ||
             (emr_params->amr_rx_type_sub0 == SID_UPDATE))
          {
            l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas +=4;   // 4 bursts are accumulated

            // Accumulate BEP
            l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc += mean_bep;
            l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc   += cv_bep;
            l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num++;
            l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num++;

            //accumulation of the correctly decoded block
            l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc += last_corr_decoded_burst[4] +
                                                         last_corr_decoded_burst[5] +
                                                         last_corr_decoded_burst[6] +
                                                         last_corr_decoded_burst[7];

            if ((emr_params->amr_rx_type_sub0 == SPEECH_GOOD) || (emr_params->amr_rx_type_sub0 == SPEECH_DEGRADED))
              //Number of correctly decoded blocks excluding SACCH FACCH RATSCCH and SID frames.
              l1a_l1s_com.Smeas_dedic_emr.nbr_rcvd_blocks++;
          } // if((rx_type == SPEECH_GOOD) || (rx_type == SID_UPDATE))
        } // else if(b_blud == TRUE)
      } // else of if(facch_present == TRUE)
    } //AMR FS

    //AMR HS
    //TCH AHS
    if((task == TCHTH) &&
       (channel_mode == TCH_AHS_MODE)&&
       (normalised_fn_mod13_mod4 == 3))
    {
      UWORD8 norm_fn_mod26;
      subchannel = emr_params->subchannel;

      if (subchannel == 0)
      {
        // Load the bit to check if the block is valid
        b_blud =  emr_params->a_dd_0_blud;
        rx_type = emr_params->amr_rx_type_sub0;
      }
      else // subchannel 1
      {
        // Load the bit to check if the block is valid
        b_blud =  emr_params->a_dd_1_blud;
        rx_type = emr_params->amr_rx_type_sub1;
      }

      norm_fn_mod26 = (l1s.actual_time.fn - subchannel +26)  % 26;

      if(norm_fn_mod26 == 7 ||
         norm_fn_mod26 == 16||
         norm_fn_mod26 == 24 )
      {
        // FACCH: Check A_FD information block.
        if(emr_params->amr_facch_present == TRUE)
        {
          // FACCH present
          // FACCH correctly decoded ?
          if(emr_params->amr_facch_fire1 == FALSE)
          {
            l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas +=4;   // 4 bursts are accumulated

            // Accumulate BEP
            l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc +=mean_bep;
            l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc   +=cv_bep;
            l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num++;
            l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num++;

            //accumulation of the correctly decoded block
            l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc += last_corr_decoded_burst[4] +
                                                         last_corr_decoded_burst[5] +
                                                         last_corr_decoded_burst[6] +
                                                         last_corr_decoded_burst[7];
          } // if(fire1 == FALSE)
        } // if(facch_present == TRUE)
        else
        {
          // NOT FACCH
          // Load the bit to check if the block is a RATSCCH in caseof rx_type = NO_DATA
          //In half rate, there are 2 consecutive frames called RATSCCH_MARKER and
          //RATSCCH_DATA, MARKER doesn't contain any CRC. So we cannot make a decision
          //whether RATSCCH_MARKER is correctly decoded. Hence ratscch_rxtype_prev
          //is not valid. Hence the inner check has to be based only on ratscch_rxtype.
          //ratscch_rxtype is updated based on the CRC of RATSCCH_DATA.
          //The following are the decisions on the outer check "if (b_ratscch_blud == TRUE)....
          //b_ratscch_blud is updated based on RATSCCH_DATA. Hence it is a valid check
          //b_ratscch_blud_prev would have been accumulated based on RATSCCH_MARKER.
          //The assumption here is that when RATSCCH_MARKER is detected, the b_blud bit of
          //a_ratscch_dl will be updated.

          if ( ((emr_params->b_ratscch_blud == TRUE) && (emr_params->ratscch_rxtype == RATSCCH_GOOD)) ||
               ((b_blud==TRUE) && (rx_type == SID_UPDATE)) ||
               ((b_blud == TRUE)&&(rx_type == SPEECH_DEGRADED)) ||
               ((b_blud == TRUE)&&(rx_type == SPEECH_GOOD)) )
          {
            //RATSCCH or SID Update or Speech block correctly decoded, increment the counter
            l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas +=4;   // 4 bursts are accumulated

            // Accumulate BEP
            l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc +=mean_bep;
            l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc   +=cv_bep;
            l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num++;
            l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num++;

            //accumulation of the correctly decoded block
            l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc +=last_corr_decoded_burst[4] +
                                                        last_corr_decoded_burst[5] +
                                                        last_corr_decoded_burst[6] +
                                                        last_corr_decoded_burst[7];
            if (((rx_type == SPEECH_GOOD) || (rx_type == SPEECH_DEGRADED)) && (emr_params->ratscch_rxtype != RATSCCH_GOOD))
              l1a_l1s_com.Smeas_dedic_emr.nbr_rcvd_blocks++;
          }
        } // else part of if(facch_present == TRUE)
      }//fnmod26 == 7||16 || 24
      else
      {
        //if (norm_fn_mod26 ==3 || norm_fn_mod26 == 11 ||
        //norm_fn_mod26 == 20)

        if ( ((emr_params->b_ratscch_blud == TRUE) && (emr_params->ratscch_rxtype == RATSCCH_GOOD)) ||
             ((b_blud==TRUE) && (rx_type == SID_UPDATE)) ||
             ((b_blud == TRUE)&&(rx_type == SPEECH_DEGRADED)) ||
             ((b_blud == TRUE)&&(rx_type == SPEECH_GOOD)) )
        {
          //RATSCCH or SID Update or Speech block correctly decoded, increment the counter
          l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas +=4;   // 4 bursts are accumulated

          // Accumulate BEP
          l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc +=mean_bep;
          l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc   +=cv_bep;
          l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num++;
          l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num++;

          //accumulation of the correctly decoded block
          l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc +=last_corr_decoded_burst[4] +
                                                      last_corr_decoded_burst[5] +
                                                      last_corr_decoded_burst[6] +
                                                      last_corr_decoded_burst[7];
          if (((rx_type == SPEECH_GOOD) || (rx_type == SPEECH_DEGRADED)) && (emr_params->ratscch_rxtype != RATSCCH_GOOD))
            l1a_l1s_com.Smeas_dedic_emr.nbr_rcvd_blocks++;
        }
      } //else fn = 3,11,20
    }//task == TCHTH.....
  #endif //(AMR == 1)
}
#endif //FF_EMR
#else //REL99

void l1s_read_dedic_scell_meas(UWORD8 input_level, UWORD8 sub_flag)
{
  WORD16 rxlev = l1s_encode_rxlev(input_level);

  // Measurement must be rejected if channel is hopping, hopped on
  // the beacon frequency and PWRC is TRUE (see GSM05.08, $8.1.3).
  if(!((l1a_l1s_com.dedic_set.pwrc == TRUE) &&
       (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->chan_sel.h == TRUE) &&
       (l1a_l1s_com.dedic_set.radio_freq_dd == l1a_l1s_com.Scell_info.radio_freq)))
  {
    // Add to FULL set meas.
    l1a_l1s_com.Scell_info.meas.nbr_meas++;
    l1a_l1s_com.Scell_info.meas.acc += rxlev;

    if(sub_flag == TRUE)
    {
      // Add to SUB set meas.
      l1a_l1s_com.Smeas_dedic.nbr_meas_sub++;
      l1a_l1s_com.Smeas_dedic.acc_sub += rxlev;
    }
  }
}
#endif //REL99
/*-------------------------------------------------------*/
/* l1s_dedic_reporting()                                 */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                        */
/*-------------------------------------------------------*/
void l1s_dedic_reporting()
{
  xSignalHeaderRec  *msg;
  UWORD8             i;
  UWORD32            nbr_carrier = l1a_l1s_com.ba_list.nbr_carrier;

  // Allocate L1C_MEAS_DONE message...
  msg = os_alloc_sig(sizeof(T_MPHC_MEAS_REPORT));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = L1C_MEAS_DONE;

  // Fill miscelaneous parameters
  //=============================
  ((T_MPHC_MEAS_REPORT*)(msg->SigP))->ba_id = l1a_l1s_com.ba_list.ba_id;

  //timing_advance...
  //txpwr...
  //meas_valid...

  // Fill msg for Neighbor Cells
  //============================

  for(i=0;i<nbr_carrier;i++)
  {
    T_MEAS_INFO  *ba_ptr = &l1a_l1s_com.ba_list.A[i];

    ((T_MPHC_MEAS_REPORT*)(msg->SigP))->ncell_meas.A[i].bcch_freq      = ba_ptr->radio_freq;
    ((T_MPHC_MEAS_REPORT*)(msg->SigP))->ncell_meas.A[i].rxlev_acc      = ba_ptr->acc;
    ((T_MPHC_MEAS_REPORT*)(msg->SigP))->ncell_meas.A[i].rxlev_nbr_meas = ba_ptr->nbr_meas;

    // Reset BA.
    ba_ptr->acc      = 0;
    ba_ptr->nbr_meas = 0;
  }
  ((T_MPHC_MEAS_REPORT*)(msg->SigP))->no_of_ncell_meas = nbr_carrier;

  // Fill msg for Serving Cell
  //==========================

  ((T_MPHC_MEAS_REPORT*)(msg->SigP))->rxlev_full_acc         = l1a_l1s_com.Scell_info.meas.acc;
  ((T_MPHC_MEAS_REPORT*)(msg->SigP))->rxlev_full_nbr_meas    = l1a_l1s_com.Scell_info.meas.nbr_meas;
  ((T_MPHC_MEAS_REPORT*)(msg->SigP))->rxlev_sub_acc          = l1a_l1s_com.Smeas_dedic.acc_sub;
  ((T_MPHC_MEAS_REPORT*)(msg->SigP))->rxlev_sub_nbr_meas     = l1a_l1s_com.Smeas_dedic.nbr_meas_sub;

  ((T_MPHC_MEAS_REPORT*)(msg->SigP))->rxqual_full_acc_errors = l1a_l1s_com.Smeas_dedic.qual_acc_full;
  ((T_MPHC_MEAS_REPORT*)(msg->SigP))->rxqual_full_nbr_bits   = l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full;
  ((T_MPHC_MEAS_REPORT*)(msg->SigP))->rxqual_sub_acc_errors  = l1a_l1s_com.Smeas_dedic.qual_acc_sub;
  ((T_MPHC_MEAS_REPORT*)(msg->SigP))->rxqual_sub_nbr_bits    = l1a_l1s_com.Smeas_dedic.qual_nbr_meas_sub;

  #if REL99
  #if FF_EMR
  ((T_MPHC_MEAS_REPORT*)(msg->SigP))->rxlev_val_acc         = l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc;
  ((T_MPHC_MEAS_REPORT*)(msg->SigP))->rxlev_val_nbr_meas    = l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas;
  ((T_MPHC_MEAS_REPORT*)(msg->SigP))->nbr_rcvd_blocks       = l1a_l1s_com.Smeas_dedic_emr.nbr_rcvd_blocks;
  ((T_MPHC_MEAS_REPORT*)(msg->SigP))->mean_bep_block_acc    = l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc;
  ((T_MPHC_MEAS_REPORT*)(msg->SigP))->cv_bep_block_acc      = l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc;
  ((T_MPHC_MEAS_REPORT*)(msg->SigP))->mean_bep_block_num    = l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num;
  ((T_MPHC_MEAS_REPORT*)(msg->SigP))->cv_bep_block_num      = l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num;
  #endif
  #endif

  if(l1a_l1s_com.dedic_set.aset->dtx_allowed == TRUE)
  // Set "dtx_used" flag according to DSP transmit report result only if
  // DTX is allowed.
  {
    if(l1a_l1s_com.Smeas_dedic.dtx_used > 0)
      ((T_MPHC_MEAS_REPORT*)(msg->SigP))->dtx_used = TRUE;
    // Set the dtx_used flag in the case of TCHF/ no signaling
    else if ((l1a_l1s_com.dedic_set.aset->chan1.mode == SIG_ONLY_MODE)
            &&(l1a_l1s_com.dedic_set.aset->chan1.desc.channel_type==TCH_F))
      ((T_MPHC_MEAS_REPORT*)(msg->SigP))->dtx_used = TRUE;
    else
      ((T_MPHC_MEAS_REPORT*)(msg->SigP))->dtx_used = FALSE;
  }
  else
  {
    ((T_MPHC_MEAS_REPORT*)(msg->SigP))->dtx_used = FALSE;
  }

  // Reset Serving Cell measurement variables.
  l1s_reset_dedic_serving_meas();


  // Give miscellaneous info to L3 (just for indication/debug).
  //===========================================================

  ((T_MPHC_MEAS_REPORT*)(msg->SigP))->timing_advance = l1a_l1s_com.dedic_set.aset->timing_advance;
  ((T_MPHC_MEAS_REPORT*)(msg->SigP))->txpwr_used     = l1s.reported_txpwr;

  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
    ((T_MPHC_MEAS_REPORT*)(msg->SigP))->facch_dl_count = trace_info.facch_dl_count;
    ((T_MPHC_MEAS_REPORT*)(msg->SigP))->facch_ul_count = trace_info.facch_ul_count;
  #if (FF_REPEATED_DL_FACCH == 1)
    ((T_MPHC_MEAS_REPORT*)(msg->SigP))->facch_dl_combined_good_count = trace_info.facch_dl_combined_good_count; /* No of good blocks after combining */
    ((T_MPHC_MEAS_REPORT*)(msg->SigP))->facch_dl_repetition_block_count = trace_info.facch_dl_repetition_block_count;
  #endif/* (FF_REPEATED_DL_FACCH == 1) */
    trace_info.facch_dl_fail_count_trace = trace_info.facch_dl_fail_count;
    trace_info.facch_dl_count = 0;
    trace_info.facch_ul_count = 0;
    trace_info.facch_dl_fail_count = 0;
   #if ( FF_REPEATED_DL_FACCH == 1 ) /* Reseting the values */
    trace_info.facch_dl_combined_good_count = 0;
    trace_info.facch_dl_repetition_block_count = 0;
   #endif/* (FF_REPEATED_DL_FACCH == 1) */
  #endif

  // send L1C_MEAS_DONE message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}

#if (MOVE_IN_INTERNAL_RAM == 0) // Must be followed by the pragma used to duplicate the funtion in internal RAM
//#pragma DUPLICATE_FOR_INTERNAL_RAM_START

/*-------------------------------------------------------*/
/* l1s_read_fb()                                         */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality : this function sends L1C_FB_INFO to L1A*/
/*-------------------------------------------------------*/
void l1s_read_fb(UWORD8 task, UWORD32 fb_flag, UWORD32 toa, UWORD32 attempt,
                 UWORD32 pm, UWORD32 angle, UWORD32 snr)
{
  xSignalHeaderRec  *msg;
  WORD32             modif_toa = 0;
  WORD32             ntdma;
  UWORD32            fn_offset;

  // For detail of the here below equation cf. BUG1558
  #define MAX_TOA_FOR_SB (D_NSUBB_DEDIC*48)+DL_ABB_DELAY/4-(SB_BURST_DURATION+DL_ABB_DELAY+SB_MARGIN)/4-2

  #if (TRACE_TYPE==5) && FLOWCHART
    trace_flowchart_dspres(dltsk_trace[task].name);
  #endif

  #if (TRACE_TYPE!=0)
    switch(task)
    {
      case FBNEW: trace_fct(CST_L1S_READ_FB, l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_fb_id].radio_freq);break;
      case FB51:  trace_fct(CST_L1S_READ_FB51, l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_fb_id].radio_freq);break;
      case FB26:  trace_fct(CST_L1S_READ_FB26, l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_fb_id].radio_freq);break;
      default:    trace_fct(CST_UNKNOWN_FB, l1a_l1s_com.nsync.list[0].radio_freq);break;
    }
  #endif

  if(fb_flag == TRUE)
  {
    switch (task)
    {
      case FBNEW:
      case FB51:
      // Compute NTDMA & TOA taking into account the 23 bit guard for next SB receive window.
      {
        modif_toa = toa - (SB_MARGIN/4);     // Rem: unit is "BIT".
      }
      break;

      case FB26:
      // Compute TOA taking into account the 23 bit guard for next SB receive window
      // and the time diff. between a the fb search and a normal serving RX slot..
      // Rem: TOA cannot be less than "SB_MARGIN/4".
      {
        // Saturate TOA to MAX_TOA_FOR_SB since it is the last position compatible with the
        // SB26 tpu programming. MAX_TOA_FOR_SB + 1 would reject the TCHT/frame0 one frame later due
        // to an overlap of TPU scenarios.
        if(toa >= MAX_TOA_FOR_SB) toa = MAX_TOA_FOR_SB;

        modif_toa = toa + ((l1_config.params.fb26_anchoring_time + PROVISION_TIME - START_RX_FB)/4) - (SB_MARGIN/4); // Rem: unit is "BIT".
      }
      break;
    } // End of switch.

    if(modif_toa < 0)
    {
      modif_toa = (modif_toa + (TPU_CLOCK_RANGE/4)) * 4; // Unit is changed from bit to qbit.
      ntdma     = - 1;
    }
    else
    {
      ntdma     = modif_toa / (TPU_CLOCK_RANGE/4);
      modif_toa = (modif_toa - ntdma * (TPU_CLOCK_RANGE/4)) * 4;  // Unit is changed from bit to qbit.
    }

    switch (task)
    {
      case FBNEW:
      // Compute NTDMA & TOA taking into account the 23 bit guard for next SB receive window.
      {
        // "fn_offset" loaded with serving frame number corresponding to FB.
        // Keep a %51 format to prepare SB scheduling.
        fn_offset = (l1s.actual_time.fn - attempt + ntdma + MAX_FN) % 51;
      }
      break;

      case FB51:
      // Compute NTDMA & TOA taking into account the 23 bit guard for next SB receive window.
      {
        // "fn_offset" loaded with serving frame number corresponding to FB.
        // Keep a full frame number to allow scheduling of SB, 2 MF51 later.
        fn_offset = (l1s.actual_time.fn - attempt + ntdma + MAX_FN) % MAX_FN;
      }
      break;

      case FB26:
      {
        // "fn_offset" loaded with serving frame number corresponding to CTL(SB26).
        fn_offset = (l1s.actual_time.fn + 52 - 3) % MAX_FN;

        if(ntdma == 1) // 2nd frame...
          l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_fb_id].sb26_offset = 1;
        else
          l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_fb_id].sb26_offset = 0;
      }
      break;
    } // End of switch.

  } // End if.

  // Store TOA and FN offset in neighbor cell structure.
  l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_fb_id].time_alignmt = modif_toa;
  l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_fb_id].fn_offset    = fn_offset;

  // Create message T_L1C_FB_INFO.
  msg = os_alloc_sig(sizeof(T_L1C_FB_INFO));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = L1C_FB_INFO;

  // Fill msg fields.
  ((T_L1C_FB_INFO *) (msg->SigP))->fb_flag    = fb_flag;
  ((T_L1C_FB_INFO *) (msg->SigP))->ntdma      = ntdma;
  ((T_L1C_FB_INFO *) (msg->SigP))->neigh_id   = l1a_l1s_com.nsync.active_fb_id;
  // Debug info or testmode
  ((T_L1C_FB_INFO *) (msg->SigP))->pm       = pm;
  ((T_L1C_FB_INFO*)(msg->SigP))->toa        = toa;
  ((T_L1C_FB_INFO*)(msg->SigP))->angle      = angle;
  ((T_L1C_FB_INFO*)(msg->SigP))->snr        = snr;
  ((T_L1C_FB_INFO *) (msg->SigP))->radio_freq = l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_fb_id].radio_freq;

  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}
//#pragma DUPLICATE_FOR_INTERNAL_RAM_END
#endif // MOVE_IN_INTERNAL_RAM

#if (MOVE_IN_INTERNAL_RAM == 0) // Must be followed by the pragma used to duplicate the funtion in internal RAM
//#pragma DUPLICATE_FOR_INTERNAL_RAM_START

/*-------------------------------------------------------*/
/* l1s_read_sb()                                         */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality : this function sends L1C_SB_INFO to L1A*/
/*-------------------------------------------------------*/
void l1s_read_sb(UWORD8 task, UWORD32 flag, API *data, UWORD32 toa, UWORD8 attempt,
                 UWORD32 pm, UWORD32 angle, UWORD32 snr)
{
  xSignalHeaderRec  *msg;
  UWORD8             bsic=0;
  UWORD32            sb;
  WORD32             modif_toa = 0;
  UWORD32            fn_offset    = 0;
  WORD32             time_alignmt = 0;
  T_NCELL_SINGLE    *cell_ptr = NULL; //omaps00090550 NULL;
  UWORD32            SignalCode=0;
  UWORD8             fn_delay     = 2; // SB result read with 2 frames delay.
  UWORD8             neigh_id=0;
  UWORD32            fn;

  switch(task)
  {
    case SB2 :
    case SB51 :
    case SB26 :
      // Get Neighbour cell ptr.
      cell_ptr = &l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_sb_id];
      neigh_id = l1a_l1s_com.nsync.active_sb_id;

      #if (TRACE_TYPE!=0)
        trace_fct(CST_L1S_READ_SB, cell_ptr->radio_freq);
      #endif

      SignalCode = L1C_SB_INFO;
      if(task == SB26) fn_delay = 3 - l1a_l1s_com.nsync.list[neigh_id].sb26_offset;
    break;

    case SBCONF :
    case SBCNF51 :
    case SBCNF26 :
      // Get Neighbour cell ptr.
      cell_ptr = &l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_sbconf_id];
      neigh_id = l1a_l1s_com.nsync.active_sbconf_id;

      #if (TRACE_TYPE!=0)
        trace_fct(CST_L1S_READ_SBCONF, cell_ptr->radio_freq);
      #endif

      SignalCode = L1C_SBCONF_INFO;
      if(task == SBCNF26) fn_delay = 3 - l1a_l1s_com.nsync.list[neigh_id].sb26_offset;
    break;
  }

  #if (TRACE_TYPE==5) && FLOWCHART
    trace_flowchart_dspres(dltsk_trace[task].name);
  #endif

  // Compute NTDMA & TOA taking into account the 23 bit guard for next SB receive window.
  modif_toa = (toa - (SB_MARGIN/4)) * 4;       // Rem: unit is "QBIT".

  // Read SB content,.
  sb = data[0] & 0xffff | ((data[1] & 0xffff) << 16);

  if (flag == TRUE)
  // SB has been found...
  // We synchronized with a NEIGHBOUR cell.
  {
    UWORD32    t1, t2, t3, t3p;

    // extract BSIC, T1, T2, T3. Compute FN.
    // bsic contains NCC & BCC (GSM05.02, p9)
    bsic   = (UWORD8) ((sb & 0x000000fc) >> 2);

    t1  = ((sb & 0x00800000) >> 23 |   // t1 low
           (sb & 0x0000ff00) >> 7  |   // t1 midle
           (sb & 0x00000003) << 9);    // t1 high
    t2  =  (sb & 0x007c0000) >> 18;
    t3p = ((sb & 0x01000000) >> 24 |   // t3p low
           (sb & 0x00030000) >> 15);   // t3p high
    t3  =  (10*(t3p) +1);
    fn  =  (51 * ((t3 - t2 + 26) % 26) + t3 + (26 * 51 * t1));

    // Due to pipeline effect (CTRL.WORK.WORK.READ), sb content is taken into account
    // "fn_delay" TDMA later: that's why we use "fn + fn_delay..." in the offset computation.
    //
    // NEIGHBOUR DOMAIN:
    // -----------------
    // |                    |<----- 1 TDMA ----->| SB                 |                    |
    // |                    |                    |XXXX                |                    |
    // |                    |                    |                    |                    |
    // |                    |                    |  FN_neighbour(SB)  | FN_neighbour(SB)+1 | FN_neighbour(SB)+2
    // |                    |                    |                    |                    |
    //                                                                            offset   |
    //                                                                              +      |
    //                                                                             BOB     |
    //                                                                       |<----------->|
    // MS DOMAIN:                                                            |
    // ----------                  |                    |                    |                    |
    //        |        CTRL        |        WORK        |        WORK        |         READ       |
    //        |                    |                    |                    |                    |
    //
    // offset = FN_neighbour(SB)+ fn_delay - FN_serving(READ).
    // Bob: fine timing difference between the Neighbour timing and the MS internal timing.
    //
    fn_offset = (fn + fn_delay + MAX_FN - l1s.actual_time.fn) % MAX_FN;

    // "time_alignmt" must be corrected (use "modif_toa" from the SB read).
    // Check that "time_alignmt" do not become bigger than "TPU_CLOCK_RANGE".
    // If so, "fn_offset" must be decremented.
    time_alignmt   = cell_ptr->time_alignmt + modif_toa;
    if(time_alignmt >= TPU_CLOCK_RANGE)
    {
      time_alignmt -= TPU_CLOCK_RANGE;  // qbp for sim. Normal value is 1250;
      fn_offset    -= 1;                // WARNING....to be checked!!!!!!
    }
    else
    if(time_alignmt < 0)
    {
      time_alignmt += TPU_CLOCK_RANGE;  // qbp for sim. Normal value is 1250;
      fn_offset    += 1;                // WARNING....to be checked!!!!!!
    }
  }

  #if L1_RECOVERY
    if(flag)
    {
      // recovery flag is reseted because the system works fine
      // this check is performed in all modes.
      l1a_l1s_com.recovery_flag = FALSE;

      // Reset error flags and counter
      l1s.recovery.frame_count  = 0;
    }
  #endif

  // In all mode send result message to L1 Async.
  msg = os_alloc_sig(sizeof(T_MPHC_NCELL_SYNC_IND));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode   = SignalCode;
  ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->sb_flag      = flag;
  ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->radio_freq   = cell_ptr->radio_freq;
  ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->bsic         = bsic;
  ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->fn_offset    = fn_offset;
  ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->time_alignmt = time_alignmt;
  ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->neigh_id     = neigh_id;
  ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->attempt      = attempt;
  ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->pm           = pm;
  ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->toa          = toa;
  ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->angle        = angle;
  ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->snr          = snr;


  #if (L1_EOTD == 1)
  // In EOTD mode read additional results
  if (l1a_l1s_com.nsync.eotd_meas_session == TRUE)
  {
    UWORD8  i;
    WORD16  d_eotd_first;
    WORD16  d_eotd_max;
    UWORD32 d_eotd_nrj;
    API     *data = &(l1s_dsp_com.dsp_ndb_ptr->a_eotd_crosscor[0]);
    UWORD32 fn_sb_neigh;

    d_eotd_first   = l1s_dsp_com.dsp_ndb_ptr->d_eotd_first & 0xffff;

    d_eotd_max     = l1s_dsp_com.dsp_ndb_ptr->d_eotd_max & 0xffff;

    fn_sb_neigh = (l1s.actual_time.fn - fn_delay + MAX_FN) % MAX_FN;
    ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->fn_sb_neigh  = fn_sb_neigh;

    d_eotd_nrj   = (l1s_dsp_com.dsp_ndb_ptr->d_eotd_nrj_low & 0xffff) |
                   ((l1s_dsp_com.dsp_ndb_ptr->d_eotd_nrj_high & 0x00ff) << 16);

    // L1 SW :
    //         CPS Cursor expects the accumulated signal level of the cross
    //         correlation (d_eotd_nrj) to be 16bit format. The DSP reports
    //         it as 24bit format (lsb aligned in a 32bit word).
    //         We scale the DSP result by right shifting by 8, hence preserving
    //         the MSBs

    d_eotd_nrj >>= 8;

    ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->eotd_data_valid = TRUE;
    ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->d_eotd_first    = d_eotd_first;
    ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->d_eotd_max      = d_eotd_max;
    ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->d_eotd_nrj      = d_eotd_nrj;
    ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->fn_in_SB        = fn;
    for (i=0; i<18; i++)
      ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->a_eotd_crosscor[i] = data[i] & 0xffff;;

    if(task == SBCNF26)
    {
      ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->toa_correction =
        l1a_l1s_com.nsync.eotd_cache_toa_tracking;
    }
    else
    {
      ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->toa_correction = 0;
    }

  }
  else
   ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->eotd_data_valid = FALSE;
#endif



  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}
//#pragma DUPLICATE_FOR_INTERNAL_RAM_END
#endif // MOVE_IN_INTERNAL_RAM

#if (MOVE_IN_INTERNAL_RAM == 0) // Must be followed by the pragma used to duplicate the funtion in internal RAM
//#pragma DUPLICATE_FOR_INTERNAL_RAM_START

/*-------------------------------------------------------*/
/* l1s_read_fbsb()                                       */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality : sends L1C_FBSB_INFO to L1A            */
/*-------------------------------------------------------*/
#if ((REL99 == 1) && (FF_BHO == 1))
void l1s_read_fbsb(UWORD8 task, UWORD8 attempt, BOOL fb_flag,  BOOL sb_flag, API *data, UWORD32 toa, UWORD32 pm, UWORD32 angle, UWORD32 snr)
{
  xSignalHeaderRec  *msg;
  UWORD8             bsic = 0;
  UWORD32            fn_offset = 0;
  UWORD32            time_alignmt = 0;

  #if (TRACE_TYPE==5) && FLOWCHART
    trace_flowchart_dspres(dltsk_trace[task].name);
  #endif

  #if (TRACE_TYPE!=0)
    // trace_fct(CST_L1S_READ_FBSB, l1a_l1s_com.nsync_fbsb.radio_freq);
  #endif

  if ((fb_flag == TRUE) && (sb_flag == TRUE))
  {
    UWORD32    toa_qbit, sb, fn, fn2, t1, t2, t3, t3p;
    UWORD8     ntdma;

    // Read SB content,.
    sb = data[0] & 0xffff | ((data[1] & 0xffff) << 16);

    // extract BSIC, T1, T2, T3. Compute FN.
    // bsic contains NCC & BCC (GSM05.02, p9)
    bsic   = (UWORD8) ((sb & 0x000000fc) >> 2);

    t1  = ((sb & 0x00800000) >> 23 |   // t1 low
           (sb & 0x0000ff00) >> 7  |   // t1 midle
           (sb & 0x00000003) << 9);    // t1 high
    t2  =  (sb & 0x007c0000) >> 18;
    t3p = ((sb & 0x01000000) >> 24 |   // t3p low
           (sb & 0x00030000) >> 15);   // t3p high
    t3  =  (10*(t3p) +1);
    fn  =  (51 * ((t3 - t2 + 26) % 26) + t3 + (26 * 51 * t1));

    // _|-----------------------------------|___  : TPU WINDOW
    //                    |FB|          |SB|
    // _|---------------->|--------->|->|
    //        toa_fb       1 frame  toa_sb
    //
    //  we also need to take into account the 23 bit guard for SB receive window.

    toa_qbit = (l1a_l1s_com.nsync_fbsb.fb_toa + toa) * 4 + TPU_CLOCK_RANGE - SB_MARGIN;

    ntdma = toa_qbit / TPU_CLOCK_RANGE;

    fn_offset = (fn - l1s.actual_time.fn + attempt - ntdma + (2 * MAX_FN))% MAX_FN;

    time_alignmt = toa_qbit - (ntdma * TPU_CLOCK_RANGE);
  }

  // Create message T_L1C_FBSB_INFO.
  msg = os_alloc_sig(sizeof(T_L1C_FB_INFO));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = L1C_FBSB_INFO;

  // Fill msg fields.
  ((T_L1C_FBSB_INFO *) (msg->SigP))->fb_flag      = fb_flag;
  ((T_L1C_FBSB_INFO *) (msg->SigP))->sb_flag      = sb_flag;
  ((T_L1C_FBSB_INFO *) (msg->SigP))->bsic         = bsic;
  ((T_L1C_FBSB_INFO *) (msg->SigP))->fn_offset    = fn_offset;
  ((T_L1C_FBSB_INFO *) (msg->SigP))->time_alignmt = time_alignmt;
  ((T_L1C_FBSB_INFO *) (msg->SigP))->pm           = pm;
  ((T_L1C_FBSB_INFO *) (msg->SigP))->toa          = toa;
  ((T_L1C_FBSB_INFO *) (msg->SigP))->angle        = angle;
  ((T_L1C_FBSB_INFO *) (msg->SigP))->snr          = snr;


  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}
#endif // #if ((REL99 == 1) && (FF_BHO == 1))
//#pragma DUPLICATE_FOR_INTERNAL_RAM_END
#endif // MOVE_IN_INTERNAL_RAM

#if !((MOVE_IN_INTERNAL_RAM == 1) && (GSM_IDLE_RAM !=0))  // MOVE TO INTERNAL MEM IN CASE GSM_IDLE_RAM enabled
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_START         // KEEP IN EXTERNAL MEM otherwise

/*-------------------------------------------------------*/
/* l1s_read_l3frm()                                      */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality : reads NB data                         */
/*                 and formate MPHC_DATA_IND message     */
/*-------------------------------------------------------*/
void l1s_read_l3frm(UWORD8 pwr_level, API *info_address, UWORD32 task_rx)
{
  xSignalHeaderRec *msg;
  UWORD32           i,j;
  UWORD32           word32;
  UWORD32           rx_fail_flag;
  //OMAPS00090550 UWORD32           b_fire0;
  UWORD32           b_fire1;
  UWORD8            tc = l1s.actual_time.tc; // Default: tc loaded with current serving TC.
  UWORD16           radio_freq = l1a_l1s_com.Scell_info.radio_freq; // Default: radio_freq load with serving cell

  #if (TRACE_TYPE==5) && FLOWCHART
    trace_flowchart_dspres(dltsk_trace[task_rx].name);
  #endif

  // Allocate result message.

  #if (GSM_IDLE_RAM == 1)         // GPF not modified for GSM_IDLE_RAM -> enable Traffic Controller in L1S
    if (!READ_TRAFFIC_CONT_STATE)
    CSMI_TrafficControllerOn();
  #endif

  msg = os_alloc_sig(sizeof(T_MPHC_DATA_IND));
  DEBUGMSG(status,NU_ALLOC_ERR)

  switch(task_rx)
  {
    case NP:
      #if (TRACE_TYPE!=0)
        trace_fct(CST_L1S_READ_L3FRM__NP, l1a_l1s_com.Scell_info.radio_freq);
      #endif

      // Fill msg signal code and L2 channel.
      msg->SignalCode = L1C_NP_INFO;
      ((T_MPHC_DATA_IND *)(msg->SigP))->l2_channel = L2_CHANNEL_PCH;
    break;

    case EP:
      #if (TRACE_TYPE!=0)
        trace_fct(CST_L1S_READ_L3FRM__EP, l1a_l1s_com.Scell_info.radio_freq);
      #endif

      // Fill msg signal code and L2 channel.
      msg->SignalCode = L1C_EP_INFO;
      ((T_MPHC_DATA_IND *)(msg->SigP))->l2_channel = L2_CHANNEL_EPCH;
    break;

    case NBCCHS:
      #if (TRACE_TYPE!=0)
        trace_fct(CST_L1S_READ_L3FRM__NBCCHS, l1a_l1s_com.Scell_info.radio_freq);
      #endif

      // Fill msg signal code and L2 channel.
      msg->SignalCode = L1C_BCCHS_INFO;
      ((T_MPHC_DATA_IND *)(msg->SigP))->l2_channel = L2_CHANNEL_NBCCH;
    break;

    case EBCCHS:
      #if (TRACE_TYPE!=0)
        trace_fct(CST_L1S_READ_L3FRM__EBCCHS, l1a_l1s_com.Scell_info.radio_freq);
      #endif

      // Fill msg signal code and L2 channel.
      msg->SignalCode = L1C_BCCHS_INFO;
      ((T_MPHC_DATA_IND *)(msg->SigP))->l2_channel = L2_CHANNEL_EBCCH;
    break;

    case BCCHN:
    #if L1_GPRS
    case BCCHN_TRAN:
    #endif
    case BCCHN_TOP:
      #if (TRACE_TYPE!=0)
        if (task_rx == BCCHN)
          trace_fct(CST_L1S_READ_L3FRM__BCCHN, l1a_l1s_com.bcchn.list[l1a_l1s_com.bcchn.active_neigh_id_norm].radio_freq);
        else // BCCHN_TRAN and BCCHN_TOP tasks
          trace_fct(CST_L1S_READ_L3FRM__BCCHN, l1a_l1s_com.bcchn.list[l1a_l1s_com.bcchn.active_neigh_id_top].radio_freq);
      #endif

      // Fill msg signal code, L2 channel and get neighbour TC.
      msg->SignalCode = L1C_BCCHN_INFO;

      // Save neighbour ID.
      // With TC and Neighbour ID, L1A can manage the remaining BCCH requests.
      if (task_rx == BCCHN)
      {
        ((T_MPHC_DATA_IND *)(msg->SigP))->neigh_id = l1a_l1s_com.bcchn.active_neigh_id_norm;
        tc = l1a_l1s_com.bcchn.active_neigh_tc_norm;
        radio_freq = l1a_l1s_com.bcchn.list[l1a_l1s_com.bcchn.active_neigh_id_norm].radio_freq;
      }
      else // BCCHN_TRAN and BCCHN_TOP tasks
      {
        ((T_MPHC_DATA_IND *)(msg->SigP))->neigh_id = l1a_l1s_com.bcchn.active_neigh_id_top;
        tc = l1a_l1s_com.bcchn.active_neigh_tc_top;
        radio_freq = l1a_l1s_com.bcchn.list[l1a_l1s_com.bcchn.active_neigh_id_top].radio_freq;
      }

      if(tc >= 8)
      {
        // Reading Extended BCCH.
        tc -= 8;
        ((T_MPHC_DATA_IND *)(msg->SigP))->l2_channel = L2_CHANNEL_EBCCH;
      }
      else
      {
        // Reading Normal BCCH.
        ((T_MPHC_DATA_IND *)(msg->SigP))->l2_channel = L2_CHANNEL_NBCCH;
      }

    break;

    case ALLC:
      #if (TRACE_TYPE!=0)
        trace_fct(CST_L1S_READ_L3FRM__ALLC, l1a_l1s_com.Scell_info.radio_freq);
      #endif

      // Fill msg signal code, L2 channel and get neighbour TC.
      msg->SignalCode = L1C_ALLC_INFO;
      ((T_MPHC_DATA_IND *)(msg->SigP))->l2_channel = L2_CHANNEL_CCCH;
    break;

    case SMSCB:
      #if (TRACE_TYPE!=0)
        trace_fct(CST_L1S_READ_L3FRM__CB, l1a_l1s_com.Scell_info.radio_freq);
      #endif

      // Fill msg signal code, L2 channel and get neighbour TC.
      msg->SignalCode = L1C_CB_INFO;
      ((T_MPHC_DATA_IND *)(msg->SigP))->l2_channel = L2_CHANNEL_CBCH;
    break;

    #if L1_GPRS

      case PNP:
        #if (TRACE_TYPE!=0)
          trace_fct(CST_L1PS_READ_L3FRM__PNP, l1a_l1s_com.Scell_info.radio_freq);
        #endif

        // Fill msg signal code and L2 channel.
        msg->SignalCode = L1P_PNP_INFO;
        ((T_MPHC_DATA_IND *)(msg->SigP))->l2_channel = L2_PCHANNEL_PPCH;
        tc = l1pa_l1ps_com.pbcchs.rel_pos_to_report;
      break;

      case PEP:
        #if (TRACE_TYPE!=0)
          trace_fct(CST_L1PS_READ_L3FRM__PEP, l1a_l1s_com.Scell_info.radio_freq);
        #endif

        // Fill msg signal code and L2 channel.
        msg->SignalCode = L1P_PEP_INFO;
        ((T_MPHC_DATA_IND *)(msg->SigP))->l2_channel = L2_PCHANNEL_PEPCH;
        tc = l1pa_l1ps_com.pbcchs.rel_pos_to_report;
      break;

      case PALLC:
        #if (TRACE_TYPE!=0)
          trace_fct(CST_L1PS_READ_L3FRM__PALLC, l1a_l1s_com.Scell_info.radio_freq);
        #endif

        // Fill msg signal code and L2 channel.
        msg->SignalCode = L1P_PALLC_INFO;
        ((T_MPHC_DATA_IND *)(msg->SigP))->l2_channel = L2_PCHANNEL_PCCCH;
        tc = l1pa_l1ps_com.pbcchs.rel_pos_to_report;
      break;

      case PBCCHS:
        #if (TRACE_TYPE!=0)
          trace_fct(CST_L1PS_READ_L3FRM__PBCCHS, l1a_l1s_com.Scell_info.radio_freq);
        #endif

        // Fill msg signal code and L2 channel.
        msg->SignalCode = L1P_PBCCHS_INFO;
        ((T_MPHC_DATA_IND *)(msg->SigP))->l2_channel = L2_PCHANNEL_PBCCH;
        tc = l1pa_l1ps_com.pbcchs.rel_pos_to_report;
      break;

      case PBCCHN_IDLE:
      case PBCCHN_TRAN:
        #if (TRACE_TYPE!=0)
          trace_fct(CST_L1PS_READ_L3FRM__PBCCHN, l1pa_l1ps_com.pbcchn.bcch_carrier);
        #endif

        // Fill msg signal code and L2 channel.
        msg->SignalCode = L1P_PBCCHN_INFO;
        ((T_MPHC_DATA_IND *)(msg->SigP))->l2_channel = L2_PCHANNEL_PBCCH;
        tc = l1pa_l1ps_com.pbcchn.relative_position;
        radio_freq = l1pa_l1ps_com.pbcchn.bcch_carrier;
      break;

      case SINGLE:
        #if (TRACE_TYPE!=0)
          trace_fct(CST_L1PS_READ_L3FRM__SINGLE, l1a_l1s_com.Scell_info.radio_freq);
        #endif

        // Fill L2 channel.
        msg->SignalCode = L1P_PACCH_INFO;
        ((T_MPHC_DATA_IND *)(msg->SigP))->l2_channel = L2_PCHANNEL_PACCH;
        tc = l1pa_l1ps_com.pbcchs.rel_pos_to_report;
      break;

    #endif

    // WARNING !!! to be removed (for CBCH debugging).
    default:
      #if (TRACE_TYPE!=0)
        trace_fct(CST_L1PS_READ_L3FRM__UNKNOWN, l1a_l1s_com.Scell_info.radio_freq);
      #endif

      // Fill msg signal code, L2 channel and get neighbour TC.
      msg->SignalCode = L1C_CB_INFO;
      ((T_MPHC_DATA_IND *)(msg->SigP))->l2_channel = L2_CHANNEL_CCCH;
  }

  #if L1_GPRS
  if (l1a_l1s_com.dsp_scheduler_mode == GSM_SCHEDULER)
  #endif
  {
    // Compute detection flag.
 //OMAPS00090550   b_fire0 = ((info_address[0] & 0xffff) & (1<<B_FIRE0)) >> B_FIRE0;
    b_fire1 = ((info_address[0] & 0xffff) & (1<<B_FIRE1)) >> B_FIRE1;
    if(b_fire1 != 1)
      rx_fail_flag = FALSE;  // information block received successfully.
    else
      rx_fail_flag = TRUE;   // information block reception failled.

    // Get 23 bytes info. from DSP.
    for (j=0, i=0; i<11; i++)
    {
      word32 = info_address[3 + i]; // Get info word, rem: skip info. header.
      ((T_MPHC_DATA_IND *)(msg->SigP))->l2_frame.A[j++] = (word32 & 0x000000ff);
      ((T_MPHC_DATA_IND *)(msg->SigP))->l2_frame.A[j++] = (word32 & 0x0000ff00) >> 8;
    }
    ((T_MPHC_DATA_IND *)(msg->SigP))->l2_frame.A[22] = (info_address[14] & 0x000000ff);

    // reset buffers and flags in NDB ...
    // B_FIRE1 =1, B_FIRE0 =0 , BLUD =0
    info_address[0] = (1<<B_FIRE1);
  }
  #if L1_GPRS
    else // GPRS scheduler
    {
      // Compute detection flag.
      rx_fail_flag = ((*info_address & 0x0100) >> 8);

      // Get 24 bytes info. from DSP: CS1 meaningful block is of size 12 UWORD16 data.
      // !!! WARNING: word32 type is for compatibility with chipset == 0.
      // Can be word16 if only chipset == 2 is used.
      for (j=0, i=0; i<12; i++)
      {
        // Data downloaded from a_dd_gprs[0][]...
        word32 = info_address[4 + i]; // Get info word, rem: skip info. header.
        #if 0	/* FreeCalypso TCS211 reconstruction */
        if(j<23)
        #endif
        ((T_MPHC_DATA_IND *)(msg->SigP))->l2_frame.A[j++] = (word32 & 0x000000ff);
        #if 0	/* FreeCalypso TCS211 reconstruction */
        if(j<23)
        #endif
        ((T_MPHC_DATA_IND *)(msg->SigP))->l2_frame.A[j++] = (word32 & 0x0000ff00) >> 8;
      }

      // reset buffers and flags in NDB ...
      // reset CS_TYPE
      info_address[0] = CS_NONE_TYPE;
    }
  #endif

  // Report detection flag.
  if((l1s_dsp_com.dsp_db_r_ptr->d_debug & 0xffff ) != (l1s.debug_time & 0xffff )) // in case of COM error the block is false
    ((T_MPHC_DATA_IND *)(msg->SigP))->error_flag = TRUE;
  else
    ((T_MPHC_DATA_IND *)(msg->SigP))->error_flag = rx_fail_flag;

  // be careful: radio_freq is setted at the beging of the code and may be modified inside the "case"
  ((T_MPHC_DATA_IND *)(msg->SigP))->radio_freq = radio_freq;

   // be careful: tc is setted at the beging of the code and may be modified inside the "case"
  ((T_MPHC_DATA_IND *)(msg->SigP))->tc = tc;

  // convert in ETSI format and send it back to L3
  ((T_MPHC_DATA_IND *)(msg->SigP))->ccch_lev = l1s_encode_rxlev(pwr_level);

#if (FF_L1_FAST_DECODING == 1)
  // Update the fn according to the number of the last decoded burst (2, 3 or 4) in case of fast paging: alignment with a block boudary
  if(l1a_l1s_com.last_fast_decoding == 0)
    // fast decoding was not not used
  ((T_MPHC_DATA_IND *)(msg->SigP))->fn       = l1s.actual_time.fn;
  else
    // fast decoding done, fn is incremented up to 2 frames (if fast decoding with 2 bursts), 0 if fast decoding with 4 bursts (normal decoding)
    ((T_MPHC_DATA_IND *)(msg->SigP))->fn       = l1s.actual_time.fn + BURST_4 + 1 - l1a_l1s_com.last_fast_decoding;
#else
    ((T_MPHC_DATA_IND *)(msg->SigP))->fn       = l1s.actual_time.fn;
#endif
  // send message...

  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}

//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END
#endif
/*-------------------------------------------------------*/
/* l1s_read_sacch_dl()                                   */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality : reads NB data                         */
/*-------------------------------------------------------*/
void l1s_read_sacch_dl(API *info_address, UWORD32 task_rx)
{
  xSignalHeaderRec *msg;
  UWORD32           i,j;
  UWORD32           word32;
  UWORD32           rx_fail_flag;
 //OMAPS00090550 UWORD32           b_fire0;
  UWORD32           b_fire1;
  #if  (FF_REPEATED_SACCH == 1)
      BOOL           b_joint= 0; /* The flag to read the DSP response on combining */
      static BOOL prevfail = 0;
  #endif /* (FF_REPEATED_SACCH == 1) */

  // Traces.
  #if (TRACE_TYPE==5) && FLOWCHART
    trace_flowchart_dspres(dltsk_trace[task_rx].name);
  #endif

  #if (TRACE_TYPE!=0)
    if(task_rx == ADL)  trace_fct(CST_L1S_READ_SACCH_DL__ADL, l1a_l1s_com.dedic_set.radio_freq_dd);
    if(task_rx == TCHA) trace_fct(CST_L1S_READ_SACCH_DL__TCHA, l1a_l1s_com.dedic_set.radio_freq_dd);
  #endif

 #if (((TRACE_TYPE==1) || (TRACE_TYPE==4)) && ((FF_REPEATED_SACCH == 1)))
    trace_info.repeat_sacch.dl_count ++;        /* It is a SACCH downlink block */

 #endif
  // Compute detection flag.
 //OMAPS00090550 b_fire0 = ((info_address[0] & 0xffff) & (1<<B_FIRE0)) >> B_FIRE0;
  b_fire1 = ((info_address[0] & 0xffff) & (1<<B_FIRE1)) >> B_FIRE1;
  if(b_fire1 != 1)
    rx_fail_flag = FALSE;  // information block received successfully.
  else
    rx_fail_flag = TRUE;   // information block reception failled.

#if  (FF_REPEATED_SACCH == 1)
     b_joint = (BOOL)(((info_address[0] & 0xffff) & (1<<B_JOINT)) >> B_JOINT);

#endif  /* (FF_REPEATED_SACCH == 1) */
  // Clear 1st word of header.
  info_address[0] = 0;

    #if ((TESTMODE) && ((FF_REPEATED_SACCH == 1)) )
	     if(l1_config.repeat_sacch_enable != REPEATED_SACCH_ENABLE)
	     {
	         l1s.repeated_sacch.srr = 0;
	         #if (TRACE_TYPE == 1 || TRACE_TYPE == 4)
	         trace_info.repeat_sacch.dl_buffer_empty = TRUE;
	         #endif /* TRACE_TYPE*/
	      }
	     else
	     {
	#endif /* ((TESTMODE) && ((FF_REPEATED_SACCH == 1))) */


	#if  (FF_REPEATED_SACCH == 1)

	      if( (b_joint==TRUE) || (rx_fail_flag==TRUE ))
	      {
	          /* chase combining  occurred or the current block was unsuccessfully decoded.*/
	          l1s.repeated_sacch.srr = 1;

	      }
	      else
	      {
	          l1s.repeated_sacch.srr = 0; // debug

	      }


	#endif /* (FF_REPEATED_SACCH == 1) */
    #if ((TESTMODE) && ((FF_REPEATED_SACCH == 1)) )
		    } /* end else l1_config.repeat_sacch_enable */
	#endif /* ((TESTMODE) && ((FF_REPEATED_SACCH == 1))) */

	#if ((TRACE_TYPE==1 || TRACE_TYPE == 4) && (FF_REPEATED_SACCH))
	      trace_info.repeat_sacch.srr = l1s.repeated_sacch.srr;
	      if(b_joint == TRUE && b_fire1!=1)
	      {
	           trace_info.repeat_sacch.dl_combined_good_count ++;
	           if (prevfail == 1)
	          trace_info.repeat_sacch.dl_error_count--;
	      }
	#endif /* ((TRACE_TYPE==1 || TRACE_TYPE == 4) && (FF_REPEATED_SACCH)) */

#if ((TRACE_TYPE==1 || TRACE_TYPE == 4) && (FF_REPEATED_SACCH))
	      trace_info.repeat_sacch.srr = l1s.repeated_sacch.srr;
	      if( rx_fail_flag == TRUE) /* Information reception failed */
	      {
	           trace_info.repeat_sacch.dl_error_count ++;
	      }

#endif /* ((TRACE_TYPE==1 || TRACE_TYPE == 4) && (FF_REPEATED_SACCH)) */

  #if TESTMODE
    // Continunous mode: the SAACH data aren't valid. Therefore don't send to L1A.
    if ((!l1_config.TestMode) || ((l1_config.TestMode) &&
        (l1_config.tmode.rf_params.tmode_continuous == TM_NO_CONTINUOUS)))
  #endif
    {
      // Allocate result message.
      msg = os_alloc_sig(sizeof(T_PH_DATA_IND));
      DEBUGMSG(status,NU_ALLOC_ERR)
      msg->SignalCode = L1C_SACCH_INFO;
      ((T_PH_DATA_IND *)(msg->SigP))->l2_channel_type = L2_CHANNEL_SACCH;

      // Catch L1 Header if SACCH/DL data block successfully received.
      if(rx_fail_flag == FALSE)
      {
        UWORD8  supplied_txpwr = info_address[3] & 0x0000001f;
        UWORD8  supplied_ta    = (info_address[3] & 0x00007f00) >> 8;

        #if (FF_REPEATED_SACCH == 1)
		    //Set SRO parameter to transmit to the UL
		    l1s.repeated_sacch.sro = (info_address[3] & 0x00000040) >> 6;
             		 /*
			 		       7   | 6  |  5      | 4   3   2   1    0
			 		    Spare  | SRO| FPC EPC | Ordered MS power level
			 		    	   |    Ordered timing advance

		             */
		#endif /* (FF_REPEATED_SACCH == 1) */
        // Check max transmit power (min txpwr) according to powerclass.
        supplied_txpwr =  l1a_clip_txpwr(supplied_txpwr,l1a_l1s_com.dedic_set.radio_freq);

        #if TESTMODE
          // Update txpwr and ta only during Normal Mode
          if (!l1_config.TestMode)
        #endif
          {
            l1a_l1s_com.dedic_set.aset->new_target_txpwr = supplied_txpwr;

            // Check if supplied TA is valid, if not keep previous value.
            if(supplied_ta < 64)
              l1a_l1s_com.dedic_set.aset->new_timing_advance = supplied_ta;
          }
           #if ((TRACE_TYPE==1 || TRACE_TYPE == 4) && (FF_REPEATED_SACCH))
		          l1s_check_sacch_dl_block(info_address);
		          trace_info.repeat_sacch.sro = l1s.repeated_sacch.sro;
		    #endif /* ((TRACE_TYPE==1 || TRACE_TYPE == 4) && (FF_REPEATED_SACCH)) */
		  } /* end if rx_fail_flag */
		  else
		  {
			  #if ((TRACE_TYPE==1 || TRACE_TYPE == 4) && (FF_REPEATED_SACCH))
		       trace_info.repeat_sacch.dl_buffer_empty = TRUE;
		      #endif
       }

      // Get 23 bytes info. from DSP.
      for (j=0, i=0; i<11; i++)
      {
        word32 = info_address[3 + i]; // Get info word, rem: skip info. header.
        ((T_PH_DATA_IND *)(msg->SigP))->l2_frame.A[j++] = (word32 & 0x000000ff);
        ((T_PH_DATA_IND *)(msg->SigP))->l2_frame.A[j++] = (word32 & 0x0000ff00) >> 8;
      }
      ((T_PH_DATA_IND *)(msg->SigP))->l2_frame.A[22] = (info_address[14] & 0x000000ff);

      // Fill msg header...
      ((T_PH_DATA_IND *)(msg->SigP))->rf_chan_num = l1a_l1s_com.Scell_info.radio_freq;
      ((T_PH_DATA_IND *)(msg->SigP))->error_cause = rx_fail_flag;
      ((T_PH_DATA_IND *)(msg->SigP))->bsic        = l1a_l1s_com.Scell_info.bsic;
      ((T_PH_DATA_IND *)(msg->SigP))->tc          = l1s.actual_time.tc;

      // send message...

      os_send_sig(msg, L1C1_QUEUE);
      DEBUGMSG(status,NU_SEND_QUEUE_ERR)

    #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
      trace_info.sacch_d_nerr = info_address[2] & 0x00ff;
    #endif
    #if  (FF_REPEATED_SACCH == 1)
       prevfail= rx_fail_flag ;
    #endif /* (FF_REPEATED_SACCH == 1) */
    }
}

/*-------------------------------------------------------*/
/* l1s_read_dcch_dl()                                    */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality : reads FACCH DL data block.            */
/*-------------------------------------------------------*/
void l1s_read_dcch_dl(API *info_address, UWORD32 task_rx)
{
  UWORD8  rx_valid_flag = FALSE;
  UWORD8 timing_advance = 255;
  BOOL  b_joint ;  /* DSP indicator to Chase Combining */
  #if (L1_SAGEM_INTERFACE == 1)
  UWORD8 channel_type = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type;
  #endif

  #if (FF_REPEATED_DL_FACCH == 1)
      b_joint = (BOOL) (((info_address[0] & 0xffff) & (1<<B_JOINT)) >> B_JOINT);

    #if TESTMODE
          if(l1_config.repeat_facch_dl_enable != REPEATED_FACCHDL_ENABLE)  /*  repeated FACCH mode is disabled  */
            b_joint = FALSE;
    #endif /* TESTMODE */

  #endif /* (FF_REPEATED_DL_FACCH == 1) */


  if(info_address != NULL)
  // A data block must be passed to L2.
  {
    UWORD32 b_fire1;

    #if (TRACE_TYPE!=0)
      if(task_rx == DDL)   trace_fct(CST_L1S_READ_DCCH_DL__DDL,   l1a_l1s_com.dedic_set.radio_freq_dd);
      if(task_rx == TCHTF) trace_fct(CST_L1S_READ_DCCH_DL__TCHTF, l1a_l1s_com.dedic_set.radio_freq_dd);
      if(task_rx == TCHTH) trace_fct(CST_L1S_READ_DCCH_DL__TCHTH, l1a_l1s_com.dedic_set.radio_freq_dd);
    #endif

    // Set detection flag.
    b_fire1 = ((info_address[0] & 0xffff) & (1<<B_FIRE1)) >> B_FIRE1;
    if(b_fire1 != 1)
      rx_valid_flag = TRUE;  // information block received successfully.
    else
      rx_valid_flag = FALSE; // information block reception failled.

    if((rx_valid_flag == TRUE) && (l1a_l1s_com.dedic_set.aset->t3124 != 0))
    // T3124 running...
    // Check for PHYSICAL INFORMATION message from FACCH.
    {
      UWORD32  message_type = info_address[5] & 0x00ff;

      if(message_type == 0x2D)
      // FACCH message is a PHYSICAL INFORMATION message.
      {
        // Catch TIMING ADVANCE information.
        timing_advance = (UWORD8)((info_address[5] & 0xff00) >> 8);
        l1a_l1s_com.dedic_set.aset->new_timing_advance = timing_advance;
        l1a_l1s_com.dedic_set.aset->timing_advance     = timing_advance;

        // Reset T3124.
        l1a_l1s_com.dedic_set.aset->t3124 = 0;

        // Stop sending Handover Access burst.
        l1a_l1s_com.dedic_set.aset->ho_acc_to_send = 0;

        // Handover access procedure is completed.
        // -> send L1C_HANDOVER_FINISHED message with "cause = COMPLETED" to L1A.
        l1s_send_ho_finished(HO_COMPLETE);
      }
    }

    // Clear 1st word of header.
    info_address[0] = 0;

    // Shift data block pointer to skip DSP header.
    info_address = &(info_address[3]);

    if (rx_valid_flag == TRUE)
    {
      #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
        trace_info.facch_dl_count ++;
      #endif
      #if (FF_REPEATED_DL_FACCH == 1)  /*Fire code is correct and information recieved successfully */
       l1s_store_facch_buffer(l1s.repeated_facch, info_address);
      #endif

	/* trace for FER calculation (Repeated FACCH mode):                          */
        /*  nb of DL FACCH blocks correctly decoded which is not a repetition  */

      #if ((TRACE_TYPE==1|| TRACE_TYPE==4) && (FF_REPEATED_DL_FACCH))
        trace_info.facch_dl_good_block_reported++;
        if(b_joint == TRUE) /*  The combined block is successfully decoded */
            trace_info.facch_dl_combined_good_count++;
      #endif /* ((TRACE_TYPE==1|| TRACE_TYPE==4) && (FF_REPEATED_DL_FACCH)) */
      #if (DEBUG_DEDIC_TCH_BLOCK_STAT == 1)
        Trace_dedic_tch_block_stat(FACCH_GOOD, l1s_dsp_com.dsp_ndb_ptr->a_fd[2], 0);
      #endif
    }
    else
    {
      #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
        trace_info.facch_dl_fail_count ++;
      #endif
      #if (DEBUG_DEDIC_TCH_BLOCK_STAT == 1)
        Trace_dedic_tch_block_stat(FACCH_BAD, l1s_dsp_com.dsp_ndb_ptr->a_fd[2], 0);
      #endif
    }
  }
     #if (FF_REPEATED_DL_FACCH == 1)
     if(rx_valid_flag == FALSE)
     {
      l1s.repeated_facch.pipeline[l1s.repeated_facch.counter_candidate].buffer_empty=TRUE;
     }
     #endif  /* (FF_REPEATED_DL_FACCH == 1) */

  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
    RTTL1_FILL_DL_DCCH(rx_valid_flag, timing_advance)
  #endif

  // Pass data block to L2.
#if (SEND_FN_TO_L2_IN_DCCH==1)
#if (L1_SAGEM_INTERFACE == 1)
    dll_dcch_downlink(info_address, rx_valid_flag, l1s.actual_time.fn, channel_type);
#else
  dll_dcch_downlink(info_address, rx_valid_flag, l1s.actual_time.fn);
#endif
#else
#if (L1_SAGEM_INTERFACE == 1)
    dll_dcch_downlink(info_address, rx_valid_flag, channel_type);
#else
  dll_dcch_downlink(info_address, rx_valid_flag);
#endif
#endif

}

#if (CHIPSET==15)
/*-------------------------------------------------------*/
/* l1s_reset_tx_ptr()                                    */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality : Reset the TX data pointer for DRP     */
/*                 after ABORT                           */
/*-------------------------------------------------------*/

#define L1_DRP_TX_PTR_RESET_SET    (0x00000020)
#define L1_DRP_TX_PTR_RESET_RESET  (~(L1_DRP_TX_PTR_RESET_SET))

void l1s_reset_tx_ptr(UWORD8 param1, UWORD8 param2)
{
    volatile UWORD32 *ptr_drp_init32;
    ptr_drp_init32 = (UWORD32 *) (DRP_API_BASE_ADDRESS + DRP_REG_SRM_CW_ADDR); //0xFFFF1E00;

    // Set the TX_PTR_RESET bit to 1 to reset TX RD and WR pointers
    (*ptr_drp_init32) = (*ptr_drp_init32)|(L1_DRP_TX_PTR_RESET_SET);

    // Reset the bit to zero as aslong as the bit is 1, pointers are in reset state
    (*ptr_drp_init32) = (*ptr_drp_init32)&(L1_DRP_TX_PTR_RESET_RESET);
}
#endif
