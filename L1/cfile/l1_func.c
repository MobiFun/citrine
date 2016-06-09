/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_FUNC.C
 *
 *        Filename l1_func.c
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

#define  L1_FUNC_C

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
  #include "cust_os.h"
  #include "l1_msgty.h"
  #include "l1_varex.h"
  #include "l1_proto.h"
  #include "l1_mftab.h"
  #include "l1_tabs.h"
  #include "l1_ver.h"

  #if L1_GPRS
    #include "l1p_cons.h"
    #include "l1p_msgt.h"
    #include "l1p_deft.h"
    #include "l1p_vare.h"
    #include "l1p_tabs.h"
    #include "l1p_macr.h"
  #endif

  #include "l1_rf2.h"
  #include <stdio.h>
  #include "sim_cfg.h"
  #include "sim_cons.h"
  #include "sim_def.h"
  #include "sim_var.h"

#else

  #include <string.h>
  #include "l1_types.h"
  #include "sys_types.h"
  #include "l1_const.h"
  #if (RF_FAM == 12)
    #include "l1_rf12.h"
  #elif (RF_FAM == 61)
    #include "l1_rf61.h"
  #endif
  #include "l1_time.h"
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
  #include "tpudrv.h"

  #include "../../bsp/mem.h"
  #include "../../bsp/inth.h"
  #include "../../bsp/clkm.h"
  #include "../../bsp/rhea_arm.h"
  #include "../../bsp/dma.h"
  #include "../../bsp/ulpd.h"
  #include "../dsp/leadapi.h"

  #if (OP_L1_STANDALONE)
  #if (CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || \
      (CHIPSET == 11) || (CHIPSET == 12) || (CHIPSET == 15)
    #include "dynamic_clock.h"
  #endif
  #endif


  #if L1_GPRS
    #include "l1p_cons.h"
    #include "l1p_msgt.h"
    #include "l1p_deft.h"
    #include "l1p_vare.h"
    #include "l1p_tabs.h"
    #include "l1p_macr.h"
  #endif

#endif
#include "l1_trace.h"

#if ((TRACE_TYPE==1) || (TRACE_TYPE==2) || (TRACE_TYPE==3) || (TRACE_TYPE==4) || (TRACE_TYPE==7))
   extern void L1_trace_string(char *s);
#endif


#if (CODE_VERSION != SIMULATION)

  /* DSP patch */
  #if (DWNLD == NO_DWNLD)
    const UWORD8  patch_array[1] = {0};
    const UWORD8  DspCode_array[1] = {0};
    const UWORD8  DspData_array[1] = {0};
  #elif (DWNLD == PATCH_DWNLD)
    extern const UWORD8  patch_array[] ;
    const UWORD8  DspCode_array[1] = {0};
    const UWORD8  DspData_array[1] = {0};
  #elif (DWNLD == DSP_DWNLD)
    const UWORD8 patch_array[1] = {0};
    extern const UWORD8 DspCode_array[] ;
    extern const UWORD8 DspData_array[];
  #else
    extern const UWORD8  patch_array[] ;
    extern const UWORD8  DspCode_array[] ;
    extern const UWORD8  DspData_array[];
  #endif

  extern const UWORD8  bootCode[] ;
   UWORD32  fn_prev; // Added as a debug stage..
  /* DSP patch */


/*-------------------------------------------------------*/
/* Prototypes of internal functions used in this file.   */
/*-------------------------------------------------------*/
void l1s_init_voice_blocks (void);

/*-------------------------------------------------------*/
/* Prototypes of external functions used in this file.   */
/*-------------------------------------------------------*/
void l1dmacro_synchro   (UWORD32 when, UWORD32 value);
void LA_ReleaseLead(void);
#if (CODE_VERSION != SIMULATION)
  void l1s_audio_path_control (UWORD16 FIR_selection, UWORD16 audio_loop);
#endif

#if (L1_GPRS)
  // external functions from GPRS implementation
  void initialize_l1pvar(void);
  void l1pa_reset_cr_freq_list(void);
#endif
/*-------------------------------------------------------*/
/* dsp_power_on()                                        */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/* Remarq :   USART Buffer is 256 characters. While USART*/
/*            is not run during Application_Initialize   */
/*            (hisrs not served because Nucleus scheduler*/
/*             is not running yet) :                     */
/*            ==> check string size < 256 !!!!!!         */
/*-------------------------------------------------------*/
void dsp_power_on(void)
{
  UWORD16 dsp_start_address =0 ;//omaps00090550
    UWORD16 param_size;
    #if IDS
      UWORD16 param_size2;
    #endif

    API i;
    API *pt;
    volatile WORD16 j;

    T_NDB_MCU_DSP * dsp_ndb_ptr;

    #if (DSP == 34) || (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39)
      static API_SIGNED param_tab[] = {

          D_TRANSFER_RATE,

          // ..................Latencies
          D_LAT_MCU_BRIDGE,    D_LAT_MCU_HOM2SAM,

          D_LAT_MCU_BEF_FAST_ACCESS, D_LAT_DSP_AFTER_SAM,

          //...................p_gprs_install_adress
          D_HOLE,

          //...................d_misc_config
          D_MISC_CONFIG,


          //...................d_cn_sw_workaround
          C_DSP_SW_WORK_AROUND,

          //...................Reserved
          D_HOLE,              D_HOLE,
          D_HOLE,              D_HOLE,

          //...................Frequency burst
          D_FB_MARGIN_BEG,     D_FB_MARGIN_END,
          D_NSUBB_IDLE,        D_NSUBB_DEDIC,        D_FB_THR_DET_IACQ,
          D_FB_THR_DET_TRACK,
          //...................Demodulation
          D_DC_OFF_THRES,      D_DUMMY_THRES,        D_DEM_POND_GEWL,
          D_DEM_POND_RED,
          //...................TCH Full Speech
          D_MACCTHRESH1,       D_MLDT,               D_MACCTHRESH,
          D_GU,                D_GO,                 D_ATTMAX,
          D_SM,                D_B,

          //...................V42 bis
          D_V42B_SWITCH_HYST,  D_V42B_SWITCH_MIN,    D_V42B_SWITCH_MAX,
          D_V42B_RESET_DELAY,

          //...................TCH Half Speech
          D_LDT_HR,            D_MACCTRESH_HR,       D_MACCTRESH1_HR,
          D_GU_HR,             D_GO_HR,              D_B_HR,
          D_SM_HR,             D_ATTMAX_HR,

          //...................Added variables for EFR
          C_MLDT_EFR,          C_MACCTHRESH_EFR,     C_MACCTHRESH1_EFR,
          C_GU_EFR,            C_GO_EFR,             C_B_EFR,
          C_SM_EFR,            C_ATTMAX_EFR,

          //...................Full rate variables
          D_SD_MIN_THR_TCHFS,
          D_MA_MIN_THR_TCHFS,  D_MD_MAX_THR_TCHFS,   D_MD1_MAX_THR_TCHFS,

          //...................TCH Half Speech
          D_SD_MIN_THR_TCHHS, D_MA_MIN_THR_TCHHS, D_SD_AV_THR_TCHHS,
          D_MD_MAX_THR_TCHHS, D_MD1_MAX_THR_TCHHS,

          //...................TCH Enhanced Full Rate Speech
          D_SD_MIN_THR_TCHEFS,  D_MA_MIN_THR_TCHEFS,  D_MD_MAX_THR_TCHEFS,
          D_MD1_MAX_THR_TCHEFS, D_WED_FIL_INI,

          D_WED_FIL_TC,        D_X_MIN,              D_X_MAX,
          D_SLOPE,             D_Y_MIN,              D_Y_MAX,
          D_WED_DIFF_THRESHOLD,D_MABFI_MIN_THR_TCHHS,D_FACCH_THR,

          D_MAX_OVSPD_UL,      D_SYNC_THRES,        D_IDLE_THRES,
          D_M1_THRES,          D_MAX_OVSP_DL,       D_GSM_BGD_MGT
      };
      param_size = 79;

      #if (OP_L1_STANDALONE)
      #if (CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || \
          (CHIPSET == 11) || (CHIPSET == 12) || (CHIPSET == 15)
        /* Dynamic clock configuration */
        param_tab[0] = p_dynamic_clock_cfg->d_transfer_rate;
        param_tab[1] = p_dynamic_clock_cfg->d_lat_mcu_bridge;
        param_tab[2] = p_dynamic_clock_cfg->d_lat_mcu_hom2sam;
        param_tab[3] = p_dynamic_clock_cfg->d_lat_mcu_bef_fast_access;
        param_tab[4] = p_dynamic_clock_cfg->d_lat_dsp_after_sam;
      #endif
      #endif

    #elif (DSP == 33)
      static API_SIGNED param_tab[] = {

          D_TRANSFER_RATE,

          // ..................Latencies
          D_LAT_MCU_BRIDGE,    D_LAT_MCU_HOM2SAM,

          D_LAT_MCU_BEF_FAST_ACCESS, D_LAT_DSP_AFTER_SAM,

          //...................p_gprs_install_adress
          D_HOLE,

          //...................d_misc_config
          D_MISC_CONFIG,

          //...................d_cn_sw_workaround
          C_DSP_SW_WORK_AROUND,

          #if DCO_ALGO
            //...................d_cn_dco_param
            C_CN_DCO_PARAM,
          #else
            //.................. Reserved
            D_HOLE,
          #endif

          //...................Reserved
          D_HOLE,               D_HOLE,
          D_HOLE,

          //...................Frequency burst
          D_FB_MARGIN_BEG,     D_FB_MARGIN_END,
          D_NSUBB_IDLE,        D_NSUBB_DEDIC,        D_FB_THR_DET_IACQ,
          D_FB_THR_DET_TRACK,
          //...................Demodulation
          D_DC_OFF_THRES,      D_DUMMY_THRES,        D_DEM_POND_GEWL,
          D_DEM_POND_RED,
          //...................TCH Full Speech
          D_MACCTHRESH1,       D_MLDT,               D_MACCTHRESH,
          D_GU,                D_GO,                 D_ATTMAX,
          D_SM,                D_B,

          //...................V42 bis
          D_V42B_SWITCH_HYST,  D_V42B_SWITCH_MIN,    D_V42B_SWITCH_MAX,
          D_V42B_RESET_DELAY,

          //...................TCH Half Speech
          D_LDT_HR,            D_MACCTRESH_HR,       D_MACCTRESH1_HR,
          D_GU_HR,             D_GO_HR,              D_B_HR,
          D_SM_HR,             D_ATTMAX_HR,

          //...................Added variables for EFR
          C_MLDT_EFR,          C_MACCTHRESH_EFR,     C_MACCTHRESH1_EFR,
          C_GU_EFR,            C_GO_EFR,             C_B_EFR,
          C_SM_EFR,            C_ATTMAX_EFR,

          //...................Full rate variables
          D_SD_MIN_THR_TCHFS,
          D_MA_MIN_THR_TCHFS,  D_MD_MAX_THR_TCHFS,   D_MD1_MAX_THR_TCHFS,

          //...................TCH Half Speech
          D_SD_MIN_THR_TCHHS, D_MA_MIN_THR_TCHHS, D_SD_AV_THR_TCHHS,
          D_MD_MAX_THR_TCHHS, D_MD1_MAX_THR_TCHHS,

          //...................TCH Enhanced Full Rate Speech
          D_SD_MIN_THR_TCHEFS,  D_MA_MIN_THR_TCHEFS,  D_MD_MAX_THR_TCHEFS,
          D_MD1_MAX_THR_TCHEFS, D_WED_FIL_INI,

          D_WED_FIL_TC,        D_X_MIN,              D_X_MAX,
          D_SLOPE,             D_Y_MIN,              D_Y_MAX,
          D_WED_DIFF_THRESHOLD,D_MABFI_MIN_THR_TCHHS,D_FACCH_THR,

          D_MAX_OVSPD_UL,      D_SYNC_THRES,        D_IDLE_THRES,
          D_M1_THRES,          D_MAX_OVSP_DL,       D_GSM_BGD_MGT
      };
      param_size = 79;

      #if (OP_L1_STANDALONE)
      #if (CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || \
          (CHIPSET == 11) || (CHIPSET == 12)
        /* Dynamic clock configuration */
        param_tab[0] = p_dynamic_clock_cfg->d_transfer_rate;
        param_tab[1] = p_dynamic_clock_cfg->d_lat_mcu_bridge;
        param_tab[2] = p_dynamic_clock_cfg->d_lat_mcu_hom2sam;
        param_tab[3] = p_dynamic_clock_cfg->d_lat_mcu_bef_fast_access;
        param_tab[4] = p_dynamic_clock_cfg->d_lat_dsp_after_sam;
      #endif
      #endif

    #else

    #if (VOC == FR)
      static API_SIGNED param_tab[] = {
          //...................Frequency burst
          D_NSUBB_IDLE,        D_NSUBB_DEDIC,        D_FB_THR_DET_IACQ,
          D_FB_THR_DET_TRACK,
          //...................Demodulation
          D_DC_OFF_THRES,      D_DUMMY_THRES,        D_DEM_POND_GEWL,
          D_DEM_POND_RED,      D_HOLE,               D_HOLE,
          //...................TCH Full Speech
          D_MACCTHRESH1,       D_MLDT,               D_MACCTHRESH,
          D_GU,                D_GO,                 D_ATTMAX,
          D_SM,                D_B,                  D_SD_MIN_THR_TCHFS,
          D_MA_MIN_THR_TCHFS,  D_MD_MAX_THR_TCHFS,   D_MD1_MAX_THR_TCHFS,
          //...................TCH Half Speech
          D_SD_MIN_THR_TCHHS,  D_MA_MIN_THR_TCHHS,   D_SD_AV_THR_TCHHS,
          D_MD_MAX_THR_TCHHS,  D_MD1_MAX_THR_TCHHS,  D_WED_FIL_INI,
          D_WED_FIL_TC,        D_X_MIN,              D_X_MAX,
          D_SLOPE,             D_Y_MIN,              D_Y_MAX,
          D_WED_DIFF_THRESHOLD,D_MABFI_MIN_THR_TCHHS,D_FACCH_THR,
          D_DSP_TEST
      };
      param_size = 38;
    #endif

    #if (VOC == FR_HR)
      static API_SIGNED param_tab[] = {
          //...................Frequency burst
          D_NSUBB_IDLE,        D_NSUBB_DEDIC,        D_FB_THR_DET_IACQ,
          D_FB_THR_DET_TRACK,
          //...................Demodulation
          D_DC_OFF_THRES,      D_DUMMY_THRES,        D_DEM_POND_GEWL,
          D_DEM_POND_RED,      D_HOLE,               D_HOLE,
          //...................TCH Full Speech
          D_MACCTHRESH1,       D_MLDT,               D_MACCTHRESH,
          D_GU,                D_GO,                 D_ATTMAX,
          D_SM,                D_B,
          //...................TCH Half Speech
          D_LDT_HR,            D_MACCTRESH_HR,       D_MACCTRESH1_HR,
          D_GU_HR,             D_GO_HR,              D_B_HR,
          D_SM_HR,             D_ATTMAX_HR,
          //...................TCH Full Speech
          D_SD_MIN_THR_TCHFS,
          D_MA_MIN_THR_TCHFS,  D_MD_MAX_THR_TCHFS,   D_MD1_MAX_THR_TCHFS,
          //...................TCH Half Speech
          D_SD_MIN_THR_TCHHS,
          D_MA_MIN_THR_TCHHS,
          D_SD_AV_THR_TCHHS,
          D_MD_MAX_THR_TCHHS,
          D_MD1_MAX_THR_TCHHS,
          D_WED_FIL_INI,
          D_WED_FIL_TC,
          D_X_MIN,
          D_X_MAX,
          D_SLOPE,
          D_Y_MIN,
          D_Y_MAX,
          D_WED_DIFF_THRESHOLD,
          D_MABFI_MIN_THR_TCHHS,
          D_FACCH_THR,
          D_DSP_TEST
      };
      param_size = 46;
    #endif

    #if (VOC == FR_EFR)
      static API_SIGNED param_tab[] = {
          //...................Frequency burst
          D_NSUBB_IDLE,        D_NSUBB_DEDIC,        D_FB_THR_DET_IACQ,
          D_FB_THR_DET_TRACK,
          //...................Demodulation
          D_DC_OFF_THRES,      D_DUMMY_THRES,        D_DEM_POND_GEWL,
          D_DEM_POND_RED,      D_HOLE,               D_HOLE,

          //...................TCH Full Speech
          D_MACCTHRESH1,       D_MLDT,               D_MACCTHRESH,
          D_GU,                D_GO,                 D_ATTMAX,
          D_SM,                D_B,

          //...................Added variables for EFR
          C_MLDT_EFR,          C_MACCTHRESH_EFR,     C_MACCTHRESH1_EFR,
          C_GU_EFR,            C_GO_EFR,             C_B_EFR,
          C_SM_EFR,            C_ATTMAX_EFR,

          //...................Full rate variables
          D_SD_MIN_THR_TCHFS,
          D_MA_MIN_THR_TCHFS,  D_MD_MAX_THR_TCHFS,   D_MD1_MAX_THR_TCHFS,

          //...................TCH Enhanced Full Rate Speech
          D_SD_MIN_THR_TCHEFS,  D_MA_MIN_THR_TCHEFS,  D_MD_MAX_THR_TCHEFS,
          D_MD1_MAX_THR_TCHEFS, D_HOLE,               D_WED_FIL_INI,

          D_WED_FIL_TC,        D_X_MIN,              D_X_MAX,
          D_SLOPE,             D_Y_MIN,              D_Y_MAX,
          D_WED_DIFF_THRESHOLD,D_MABFI_MIN_THR_TCHHS,D_FACCH_THR,
          D_DSP_TEST
      };
      param_size = 46;
    #endif

    #if (VOC == FR_HR_EFR)
      static API_SIGNED param_tab[] = {
          //...................Frequency burst
          D_NSUBB_IDLE,        D_NSUBB_DEDIC,        D_FB_THR_DET_IACQ,
          D_FB_THR_DET_TRACK,
          //...................Demodulation
          D_DC_OFF_THRES,      D_DUMMY_THRES,        D_DEM_POND_GEWL,
          D_DEM_POND_RED,      D_HOLE,               D_TRANSFER_RATE,
          //...................TCH Full Speech
          D_MACCTHRESH1,       D_MLDT,               D_MACCTHRESH,
          D_GU,                D_GO,                 D_ATTMAX,
          D_SM,                D_B,

          //...................TCH Half Speech
          D_LDT_HR,            D_MACCTRESH_HR,       D_MACCTRESH1_HR,
          D_GU_HR,             D_GO_HR,              D_B_HR,
          D_SM_HR,             D_ATTMAX_HR,

          //...................Added variables for EFR
          C_MLDT_EFR,          C_MACCTHRESH_EFR,     C_MACCTHRESH1_EFR,
          C_GU_EFR,            C_GO_EFR,             C_B_EFR,
          C_SM_EFR,            C_ATTMAX_EFR,

          //...................Full rate variables
          D_SD_MIN_THR_TCHFS,
          D_MA_MIN_THR_TCHFS,  D_MD_MAX_THR_TCHFS,   D_MD1_MAX_THR_TCHFS,

          //...................TCH Half Speech
          D_SD_MIN_THR_TCHHS, D_MA_MIN_THR_TCHHS, D_SD_AV_THR_TCHHS,
          D_MD_MAX_THR_TCHHS, D_MD1_MAX_THR_TCHHS,

          //...................TCH Enhanced Full Rate Speech
          D_SD_MIN_THR_TCHEFS,  D_MA_MIN_THR_TCHEFS,  D_MD_MAX_THR_TCHEFS,
          D_MD1_MAX_THR_TCHEFS, D_HOLE,               D_WED_FIL_INI,

          D_WED_FIL_TC,        D_X_MIN,              D_X_MAX,
          D_SLOPE,             D_Y_MIN,              D_Y_MAX,
          D_WED_DIFF_THRESHOLD,D_MABFI_MIN_THR_TCHHS,D_FACCH_THR,
          D_HOLE,

          //...................Data patch provisions
          D_HOLE, D_HOLE, D_HOLE, D_HOLE, D_HOLE, D_HOLE, D_HOLE, D_HOLE,

          //...................Version Number, TI Number
          D_HOLE, D_HOLE,

          // ..................DSP page
          D_DSP_TEST

          #if IDS
            ,D_MAX_OVSPD_UL,      D_SYNC_THRES,        D_IDLE_THRES,
             D_M1_THRES,          D_MAX_OVSP_DL
          #endif

      };
      param_size = 67;
      #if IDS
      // Take care to not erased "d_version_number, d_ti_version and d_dsp_page" wrote by DSP before ARM
      // set PARAM memory
        param_size2 = 5;
      #endif
    #endif
    #endif // (end of DSP != 33 || DSP != 34 || DSP != 35 || DSP != 36) || (DSP != 37) || (DSP != 38) || (DSP != 39)

    // NDB pointer.
    dsp_ndb_ptr = (T_NDB_MCU_DSP *) NDB_ADR;


    //-------------
    // DSP STARTUP
    //-------------
    {
      #if (TRACE_TYPE==1) || (TRACE_TYPE==2) || (TRACE_TYPE==3) || (TRACE_TYPE==7)
          #if (CHIPSET == 1)
            L1_trace_string ("\n\r\n\rGEMINI/POLESTAR test code\n\r-------------------------");
          #elif (CHIPSET == 2)
            L1_trace_string ("\n\r\n\rHERCULES test code\n\r------------------");
          #elif (CHIPSET == 3)
            L1_trace_string ("\n\r\n\rULYSSE/ULYSSE G0 test code\n\r--------------------------");
          #elif (CHIPSET == 4)
            L1_trace_string ("\n\r\n\rSAMSON test code\n\r----------------");
          #elif (CHIPSET == 5)
            L1_trace_string ("\n\r\n\rULYSSE G1 test code 13 MHz\n\r-------------------");
          #elif (CHIPSET == 6)
            L1_trace_string ("\n\r\n\rULYSSE G1 test code 26 MHz\n\r-------------------");
          #elif (CHIPSET == 7)
            L1_trace_string ("\n\r\n\rCALYPSO Rev A test code\n\r-------------------");
          #elif (CHIPSET == 8)
            L1_trace_string ("\n\r\n\rCALYPSO Rev B test code\n\r-------------------");
          #elif (CHIPSET == 9)
            L1_trace_string ("\n\r\n\rULYSSE C035 test code\n\r-------------------");
          #elif (CHIPSET == 10) || (CHIPSET == 11)
            L1_trace_string ("\n\r\n\rCALYPSO C035 test code\n\r-------------------");
          #elif (CHIPSET == 12)
            L1_trace_string ("\n\r\n\rCALYPSO PLUS test code\n\r-------------------");
          #elif (CHIPSET == 15)
            L1_trace_string ("\n\r\n\rLOCOSTO test code\n\r-------------------");
          #endif
      #endif

#if (TRACE_TYPE==1) || (TRACE_TYPE==2) || (TRACE_TYPE==3) || (TRACE_TYPE==7)
      /* Display Audio Configuration */
      L1_trace_string ("\n\rAUDIO: ");
      #if (KEYBEEP)
        L1_trace_string ("KB ");
      #endif
      #if (TONE)
        L1_trace_string ("TN ");
      #endif
      #if (MELODY_E1)
        L1_trace_string ("E1 ");
      #endif
      #if (MELODY_E2)
        L1_trace_string ("E2 ");
      #endif
      #if (VOICE_MEMO)
        L1_trace_string ("VM ");
      #endif
      #if (L1_VOICE_MEMO_AMR)
        L1_trace_string ("VMA ");
      #endif
      #if (SPEECH_RECO)
        L1_trace_string ("SR ");
      #endif
      #if (L1_NEW_AEC)
        L1_trace_string ("NEWAEC ");
      #elif (AEC)
        L1_trace_string ("AEC ");
      #endif
      #if (L1_GTT)
        L1_trace_string ("GTT ");
      #endif
      #if (FIR)
        L1_trace_string ("FIR ");
      #endif
      #if (AUDIO_MODE)
        L1_trace_string ("AUM ");
      #endif
      #if (L1_CPORT == 1)
        L1_trace_string ("CPO ");
      #endif
      #if (L1_STEREOPATH == 1)
        L1_trace_string ("STP ");
      #endif
      #if (L1_EXT_AUDIO_MGT == 1)
        L1_trace_string ("EAM ");
      #endif
      L1_trace_string ("\n\r");
#endif
      // Release Lead reset before DSP code/patch download to insure proper reset of DSP
      LA_ReleaseLead();

      // Init PLL : PLONOFF =1, PLMU = 0010 (k=3), PLLNDIV=1, PLLDIV=0
      LA_InitialLeadBoot(bootCode); // Load the bootCode in API
      LA_StartLead(CLKSTART);       // LEAD_PLL_CNTL register (on MCU side)
                                    // On SAMSON, only the LEAD reset is released

      // GSM 1.5
      //-----------------------------------------------------------------
      // After RESET release, DSP is in SAM Mode ! while API_CNTR (0xF900)
      // register is in reset state: HOM mode, PLL off, Bridge off. No ws
      // are applied for MCU<-->API access !!!!! So, MCU must wait for
      // end of Leadboot execution before accessing API.
      wait_ARM_cycles(convert_nanosec_to_cycles(10000));  // wait 10us


      if(l1_config.dwnld == NO_DWNLD)
      // NO DOWNLOAD...
      {
        #if (TRACE_TYPE==1) || (TRACE_TYPE==2) || (TRACE_TYPE==3) || (TRACE_TYPE==7)
          L1_trace_string ("\n\r-> No download !!");
        #endif

        // Wait for READY status from DSP.
        while(*((volatile UWORD16 *)DOWNLOAD_STATUS) != LEAD_READY);

        // Set DSP start address.
        dsp_start_address = DSP_START;
      }
      else
      if(l1_config.dwnld == DSP_DWNLD)
      // DSP CODE DOWNLOAD...
      {
        WORD32 load_result;

        #if (TRACE_TYPE==1) || (TRACE_TYPE==2) || (TRACE_TYPE==3) || (TRACE_TYPE == 7)
          #if (VOC == FR)
            L1_trace_string ("\n\r-> Downloading FR DSP code...");
          #endif

          #if (VOC == FR_HR)
            L1_trace_string ("\n\r-> Downloading FR&HR DSP code...");
          #endif

          #if (VOC == FR_EFR)
            L1_trace_string ("\n\r-> Downloading FR&EFR DSP code...");
          #endif

          #if (VOC == FR_HR_EFR)
            #if IDS
              L1_trace_string ("\n\r-> Download FR&IDS DSP code...");
            #else
              L1_trace_string ("\n\r-> Downloading 3VOC DSP code...");
            #endif
          #endif
        #endif

        // Download DSP code into DSP via API / bootcode.
        load_result = LA_LoadPage(DspCode_array,0,0);

        #if (TRACE_TYPE==1) || (TRACE_TYPE==2) || (TRACE_TYPE==3) || (TRACE_TYPE == 7)
          if(load_result)
            L1_trace_string ("\n\r-> Download FAILED !!");
          else
            L1_trace_string ("\n\r-> ... finished !!");
        #endif

        #if (VOC == FR_HR) || (VOC == FR_EFR) || (VOC == FR_HR_EFR)
          #if (TRACE_TYPE==1) || (TRACE_TYPE==2) || (TRACE_TYPE==3) || (TRACE_TYPE == 7)
            #if (VOC == FR_HR)
              L1_trace_string ("\n\r-> Downloading FR&HR DSP data ROM...");
            #endif

            #if (VOC == FR_EFR)
              L1_trace_string ("\n\r-> Downloading FR&EFR DSP data ROM...");
            #endif

            #if (VOC == FR_HR_EFR)
              #if IDS
                L1_trace_string ("\n\r-> Download FR&IDS DSP Data ROM...");
              #else
                L1_trace_string ("\n\r-> Downloading 3VOC DSP DATA ROM...");
              #endif
            #endif
          #endif

          load_result = LA_LoadPage(DspData_array,1,0);

          #if (TRACE_TYPE==1) || (TRACE_TYPE==2) || (TRACE_TYPE==3) || (TRACE_TYPE == 7)
            if(load_result)
              L1_trace_string ("\n\r-> Download FAILED !!");
            else
              L1_trace_string ("\n\r-> ... finished !!");
          #endif
        #endif

        // Set DSP start address;
        dsp_start_address = DSP_START;
      }
      else
      if(l1_config.dwnld == PATCH_DWNLD)
      // DSP PATCH DOWNLOAD...
      {
        WORD32 load_result;

        #if (TRACE_TYPE==1) || (TRACE_TYPE==2) || (TRACE_TYPE==3) || (TRACE_TYPE == 7)
          L1_trace_string ("\n\r-> Downloading patch...");
        #endif

        // Download DSP patch into DSP via API / bootcode.
        load_result = LA_LoadPage(patch_array,0,0);

        #if (TRACE_TYPE==1) || (TRACE_TYPE==2) || (TRACE_TYPE==3) || (TRACE_TYPE == 7)
          if(load_result)
            L1_trace_string ("\n\r-> Download FAILED !!");
          else
            L1_trace_string ("\n\r-> ... finished !!");
        #endif

       // Catch start address always from patch_file#.c.
        dsp_start_address   = (WORD16)patch_array[3];
        dsp_start_address <<= 8;
        dsp_start_address  += (WORD16)patch_array[2];

        // if COFF2CP output, the file begins by a null tag
        if(dsp_start_address == 0)
        {
          dsp_start_address   = (WORD16)patch_array[13];
          dsp_start_address <<= 8;
          dsp_start_address  += (WORD16)patch_array[12];
        }
      }
      else
      if(l1_config.dwnld == PATCH_DSP_DWNLD)
      // DSP CODE DOWNLOAD + PATCH DOWNLOAD...
      {
        WORD32 load_result;

        #if (TRACE_TYPE==1) || (TRACE_TYPE==2) || (TRACE_TYPE==3) || (TRACE_TYPE == 7)
          #if (VOC == FR)
            L1_trace_string ("\n\r-> Downloading FR DSP code...");
          #endif

          #if (VOC == FR_HR)
            L1_trace_string ("\n\r-> Downloading FR&HR DSP code...");
          #endif

          #if (VOC == FR_EFR)
            L1_trace_string ("\n\r-> Downloading FR&EFR DSP code...");
          #endif

          #if (VOC == FR_HR_EFR)
            #if IDS
              L1_trace_string ("\n\r-> Download FR&IDS DSP code...");
            #else
              L1_trace_string ("\n\r-> Downloading 3VOC DSP code...");
            #endif
          #endif
        #endif

        // Download DSP code into DSP via API / bootcode.
        load_result = LA_LoadPage(DspCode_array,0,0);

        #if (TRACE_TYPE==1) || (TRACE_TYPE==2) || (TRACE_TYPE==3) || (TRACE_TYPE == 7)
          if(load_result)
            L1_trace_string ("\n\r-> Download FAILED !!");
          else
            L1_trace_string ("\n\r-> ... finished !!");
        #endif

        #if (TRACE_TYPE==1) || (TRACE_TYPE==2) || (TRACE_TYPE==3) || (TRACE_TYPE == 7)
          L1_trace_string ("\n\r-> Downloading patch...");
        #endif

        // Download DSP patch into DSP via API / bootcode.
        load_result = LA_LoadPage(patch_array,0,0);

        #if (TRACE_TYPE==1) || (TRACE_TYPE==2) || (TRACE_TYPE==3) || (TRACE_TYPE == 7)
          if(load_result)
            L1_trace_string ("\n\r-> Download FAILED !!");
          else
            L1_trace_string ("\n\r-> ... finished !!");
        #endif

        #if ((VOC == FR_HR) || (VOC == FR_EFR) || (VOC == FR_HR_EFR))
          #if (TRACE_TYPE==1) || (TRACE_TYPE==2) || (TRACE_TYPE==3) || (TRACE_TYPE == 7)
            #if (VOC == FR_HR)
              L1_trace_string ("\n\r-> Downloading FR&HR DSP data ROM...");
            #endif

            #if (VOC == FR_EFR)
              L1_trace_string ("\n\r-> Downloading FR&EFR DSP data ROM...");
            #endif

            #if (VOC == FR_HR_EFR)
              #if IDS
                L1_trace_string ("\n\r-> Download FR&IDS DSP data ROM...");
              #else
                L1_trace_string ("\n\r-> Downloading 3VOC DSP data ROM...");
              #endif
            #endif
          #endif

          load_result = LA_LoadPage(DspData_array,1,0);

          #if (TRACE_TYPE==1) || (TRACE_TYPE==2) || (TRACE_TYPE==3) || (TRACE_TYPE == 7)
            if(load_result)
              L1_trace_string ("\n\r-> Download FAILED !!");
            else
              L1_trace_string ("\n\r-> ... finished !!");
          #endif
        #endif


        // Catch start address always from patch_file#.c.
        dsp_start_address   = (WORD16)patch_array[3];
        dsp_start_address <<= 8;
        dsp_start_address  += (WORD16)patch_array[2];

        // if COFF2CP output, the file begins by a null tag
        if(dsp_start_address == 0)
        {
          dsp_start_address   = (WORD16)patch_array[13];
          dsp_start_address <<= 8;
          dsp_start_address  += (WORD16)patch_array[12];
        }
      }

      #if (DSP == 16 || DSP == 17 || DSP == 30 || DSP == 31 || DSP == 32)
        dsp_ndb_ptr->d_pll_clkmod1 = CLKMOD1;                         // PLL variable (multiply by 3 factor)+ Power consumpt.
        dsp_ndb_ptr->d_pll_clkmod2 = CLKMOD2;                         // PLL variable (40 us lock time)
      #endif
    }

    #if (TRACE_TYPE==1) || (TRACE_TYPE==2) || (TRACE_TYPE==3) || (TRACE_TYPE==7)
      L1_trace_string ("\n\r\n\r");
    #endif

    //--------------------------------------------------------------
    // Loading of NDB parameters.......
    //--------------------------------------------------------------

    #if (DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39)
      // Initialize background control variable to No background. Background tasks can be launch in GPRS
      // as in GSM.
      dsp_ndb_ptr->d_background_enable = 0;
      dsp_ndb_ptr->d_background_abort  = 0;
      dsp_ndb_ptr->d_background_state  = 0;
      dsp_ndb_ptr->d_debug_ptr         = 0x0074;
      dsp_ndb_ptr->d_debug_bk          = 0x0001;
      dsp_ndb_ptr->d_pll_config        = C_PLL_CONFIG;
      dsp_ndb_ptr->p_debug_buffer      = C_DEBUG_BUFFER_ADD;
      dsp_ndb_ptr->d_debug_buffer_size = C_DEBUG_BUFFER_SIZE;
      dsp_ndb_ptr->d_debug_trace_type  = C_DEBUG_TRACE_TYPE;


      #if (CHIPSET == 12) || (CHIPSET == 15)
        dsp_ndb_ptr->d_swh_flag_ndb  = 0; /* interpolation off for  non SAIC build*/
        dsp_ndb_ptr->d_swh_Clipping_Threshold_ndb = 0x0000;
        #if (DSP == 36) || (DSP == 37) || (DSP == 39)
          #if (L1_SAIC != 0)
            dsp_ndb_ptr->d_swh_flag_ndb  = SAIC_INITIAL_VALUE;
            dsp_ndb_ptr->d_swh_Clipping_Threshold_ndb = 0x4000;
          #endif
        #endif
      #endif

      #if (W_A_DSP_IDLE3 == 1)
        // Deep Sleep work around used on Calypso
        // This init is used to backward compatibility with old patch.
        dsp_ndb_ptr->d_dsp_state       = C_DSP_IDLE3;
      #endif

      dsp_ndb_ptr->d_audio_gain_ul     = 0;
      dsp_ndb_ptr->d_audio_gain_dl     = 0;

    // for patch >= 2100, use new AEC
    #if (!L1_NEW_AEC)
      dsp_ndb_ptr->d_es_level_api      = 0x5213;
    #endif
      dsp_ndb_ptr->d_mu_api            = 0x5000;
    #else
      #if L1_GPRS
      {
        T_NDB_MCU_DSP_GPRS *p_ndb_gprs = (T_NDB_MCU_DSP_GPRS *) NDB_ADR_GPRS;

        // Initialize background control variable to No background.
        p_ndb_gprs->d_background_enable = 0;
        p_ndb_gprs->d_background_abort  = 0;
        p_ndb_gprs->d_background_state  = 0;
      }
      #endif

      #if (AMR == 1)
        // Reset NDB pointer for AMR trace
        dsp_ndb_ptr->p_debug_amr = 0;
      #endif

    #endif

    //--------------------------------------------------------------
    // Loading of PARAM area.......
    //--------------------------------------------------------------
    // Load PARAM memory...
    pt = (API *) PARAM_ADR;

    for (i=0; i<param_size; i++) *pt++ = param_tab[i];
    #if (DSP < 33) && (IDS)
      pt += 3;
      for (i= param_size + 3; i<param_size + 3 + param_size2; i++) *pt++ = param_tab[i];
    #endif

    #if (DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39)
    {
      T_PARAM_MCU_DSP *pt_param = (T_PARAM_MCU_DSP *) PARAM_ADR;

      // "d_gprs_install_address" has to be set only if no PATCH is download, i.e.
      //  "d_gprs_install_address" is automatically set by DSP if a PATCH is download
      if ((l1_config.dwnld == DSP_DWNLD) || (l1_config.dwnld == NO_DWNLD))
        pt_param->d_gprs_install_address    = INSTALL_ADD;
    }
    #endif

    #if L1_GPRS
      //--------------------------------------------------------------
      // Loading of GPRS PARAM area.......
      //--------------------------------------------------------------
      // Load GPRS PARAM memory...
      {
        T_PARAM_MCU_DSP_GPRS *pt_gprs = (T_PARAM_MCU_DSP_GPRS *) PARAM_ADR_GPRS;

        // WARNING: must be configured according to the ARM & DSP clock speed.
        // The following values are required with a 13MHz ARM clock and with a 65 MIPS DSP.
        pt_gprs->d_overlay_rlcmac_cfg_gprs = 0;
        pt_gprs->d_mac_threshold           = 0x4e20;
        pt_gprs->d_sd_threshold            = 0x0016;
        pt_gprs->d_nb_max_iteration        = 0x0004;

        #if (DSP != 33) && (DSP != 34) && (DSP != 35) && (DSP != 36) && (DSP != 37) && (DSP != 38) && (DSP != 39)

          #if (OP_L1_STANDALONE)
          #if (CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || \
              (CHIPSET == 11) || (CHIPSET == 12)
            pt_gprs->d_lat_mcu_bridge          = p_dynamic_clock_cfg->d_lat_mcu_bridge;
            pt_gprs->d_lat_mcu_hom2sam         = p_dynamic_clock_cfg->d_lat_mcu_hom2sam;
          #endif
          #endif

          #if (CHIPSET == 4)
            #if (!OP_L1_STANDALONE)
              // Latency for DSP at 78 MIPS
              pt_gprs->d_lat_mcu_bridge          = 0x0009;
            #endif
            pt_gprs->d_lat_pll2div             = 0x000C;
            #if (!OP_L1_STANDALONE)
              pt_gprs->d_lat_mcu_hom2sam         = 0x000C;
            #endif
          #else
            #if (!OP_L1_STANDALONE)
              pt_gprs->d_lat_mcu_bridge          = 0x0008;
            #endif
            pt_gprs->d_lat_pll2div             = 0x000A;
            #if (!OP_L1_STANDALONE)
              pt_gprs->d_lat_mcu_hom2sam         = 0x000A;
            #endif
          #endif

          // To be removed once G0 patch process will be aligned with G1 & G2
          // i.e. "d_gprs_install_address" automatically set by DSP if a Patch is present.
          #if (DSP == 31)
            if ((l1_config.dwnld == PATCH_DSP_DWNLD) ||
                (l1_config.dwnld == PATCH_DWNLD))
              pt_gprs->d_gprs_install_address    = INSTALL_ADD_WITH_PATCH;
            else
              pt_gprs->d_gprs_install_address    = INSTALL_ADD;
          #else
            if ((l1_config.dwnld == DSP_DWNLD) || (l1_config.dwnld == NO_DWNLD))
              pt_gprs->d_gprs_install_address    = INSTALL_ADD;
          #endif
        #endif // DSP != 33 && DSP != 34 && (DSP != 35) && DSP != 36 && DSP != 37 && DSP != 38
      }
    #endif // L1_GPRS

    *(volatile UWORD16 *) DOWNLOAD_SIZE   = 0;                 // Size=0 to force DSP to start from address...
    *(volatile UWORD16 *) DOWNLOAD_ADDR   = dsp_start_address; // Start address.
    *(volatile UWORD16 *) DOWNLOAD_STATUS = BLOCK_READY;       // Start DSP...

}
#endif //#if CODE_VERSION!=SIMULATION

/*-------------------------------------------------------*/
/* l1s_reset_db_mcu_to_dsp()                             */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1s_reset_db_mcu_to_dsp(T_DB_MCU_TO_DSP *page_ptr)
{
  API i;
  API size = sizeof(T_DB_MCU_TO_DSP) / sizeof(API);
  API *ptr = (API *)page_ptr;

  // Clear all locations.
  for(i=0; i<size; i++) *ptr++ = 0;
}

#if (DSP == 38) || (DSP == 39)
  /*-------------------------------------------------------*/
  /* l1s_reset_db_common_mcu_to_dsp()                                         */
  /*-------------------------------------------------------*/
  /* Parameters :                                                                            */
  /* Return     :                                                                               */
  /* Functionality :                                                                           */
  /*-------------------------------------------------------*/
  void l1s_reset_db_common_mcu_to_dsp(T_DB_COMMON_MCU_TO_DSP *page_ptr)
  {
    API i;
    API size = sizeof(T_DB_COMMON_MCU_TO_DSP) / sizeof(API);
    API *ptr = (API *)page_ptr;

    // Clear all locations.
    for(i=0; i<size; i++) *ptr++ = 0;
  }
#endif
/*-------------------------------------------------------*/
/* l1s_reset_db_dsp_to_mcu()                             */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1s_reset_db_dsp_to_mcu(T_DB_DSP_TO_MCU *page_ptr)
{
  API i;
  API size = sizeof(T_DB_DSP_TO_MCU) / sizeof(API);
  API *ptr = (API *)page_ptr;

  // Clear all locations.
  for(i=0; i<size; i++) *ptr++ = 0;

  // Set crc result as "SB not found".
  page_ptr->a_sch[0]  =  (1<<B_SCH_CRC);   // B_SCH_CRC =1, BLUD =0
}

/*-------------------------------------------------------*/
/* l1s_increment_time()                                  */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1s_increment_time(T_TIME_INFO *time, UWORD32 fn_offset)
{
  // Increment FN % MAX_FN.
  //------------------------
  IncMod(time->fn, fn_offset, MAX_FN);

  if(fn_offset == 1)
  // Frame by frame increment...
  //----------------------------
  {
    IncMod(time->t2, 1, 26);                // increment T2 % 26.
    IncMod(time->t3, 1, 51);                // increment T3 % 51.
    IncMod(time->fn_mod42432, 1, 42432);    // increment FN % 42432.
    IncMod(time->fn_mod13, 1, 13);          // increment FN % 13.
    IncMod(time->fn_mod13_mod4, 1, 4);      // increment (FN % 13) % 4.
    if(time->fn_mod13 == 0)
      time->fn_mod13_mod4 = 0;

    if(time->t3 == 0)
    // new FN is a multiple of 51.
    {
      // Increment TC ((FN/51) % 8).
      IncMod(time->tc, 1, 8);

      // New FN is a multiple of 26 and 51 -> increment T1 % 2048 (T1=FN div (26*51)).
      if(time->t2 == 0) IncMod(time->t1, 1, 2048);
    }

    #if (L1_GPRS)
      IncMod(time->fn_mod52, 1, 52);        // increment FN % 52.
      IncMod(time->fn_mod104, 1, 104);      // increment FN % 104.

      if((time->fn_mod13 == 0) || (time->fn_mod13 == 4) || (time->fn_mod13 == 8))
        IncMod(time->block_id, 1, MAX_BLOCK_ID);
    #endif

  }

  else
  // Jumping on a new serving cell.
  //-------------------------------
  {
    time->t2 = time->fn % 26;              // T2 = FN % 26.
    time->t3 = time->fn % 51;              // T3 = FN % 51.
    time->t1 = time->fn / (26L*51L);       // T1 = FN div 26*51
    time->tc = (time->fn / 51) % 8;        // TC = (FN div 51) % 8
    time->fn_mod42432 = time->fn % 42432;  // FN%42432.
    time->fn_mod13    = time->fn % 13;     // FN % 13.
    time->fn_mod13_mod4 = time->fn_mod13 % 4; // FN % 13 % 4.

    #if (L1_GPRS)
      time->fn_mod104     = time->fn % 104;     // FN % 104.

      if(time->fn_mod104 >= 52)                 // FN % 52.
        time->fn_mod52    = time->fn_mod104 - 52;
      else
        time->fn_mod52    = time->fn_mod104;

      time->block_id      = ((3 * (time->fn / 13)) + (time->fn_mod13 / 4));
    #endif

  }

  // Computes reporting period frame number according to the current FN
  if(l1a_l1s_com.l1s_en_task[DEDIC] == TASK_ENABLED)
  {
    T_CHANNEL_DESCRIPTION  *desc_ptr;
    UWORD8                  timeslot_no;
    UWORD8                  subchannel;

    // Get a meaningfull channel description.
    //---------------------------------------
    // Rem1: this is to avoid a bad setting of "fn_in_report" when synchro is performed
    //       whereas L1 is waiting for starting time and no channel discribed BEFORE STI.
    // Rem2: "fn_in_report" is computed with "CHAN1" parameters since it is the channel
    //       which carries the SACCH.
    if(l1a_l1s_com.dedic_set.aset->chan1.desc_ptr->channel_type == INVALID_CHANNEL)
      desc_ptr = &l1a_l1s_com.dedic_set.aset->chan1.desc;
    else
      desc_ptr = l1a_l1s_com.dedic_set.aset->chan1.desc_ptr;

    timeslot_no = desc_ptr->timeslot_no;
    subchannel  = desc_ptr->subchannel;
    if(desc_ptr->channel_type == TCH_H) timeslot_no = (2*(timeslot_no/2) + subchannel);


    // Compute "fn_in_report" according to the channel_type.
    //------------------------------------------------------
    if(desc_ptr->channel_type == SDCCH_4)
    // FN_REPORT for SDCCH/4 is: fn%102 in [37..36].
    {
      l1s.actual_time.fn_in_report = (UWORD8)((l1s.actual_time.fn - 37 + 102) % 102);
      l1s.next_time.fn_in_report   = (UWORD8)((l1s.next_time.fn   - 37 + 102) % 102);
    }
    else
    if(desc_ptr->channel_type == SDCCH_8)
    // FN_REPORT for SDCCH/4 is: fn%102 in [12..11].
    {
      l1s.actual_time.fn_in_report = (UWORD8)((l1s.actual_time.fn - 12 + 102) % 102);
      l1s.next_time.fn_in_report   = (UWORD8)((l1s.next_time.fn   - 12 + 102) % 102);
    }
    else
    // TCH_F or TCH_H...
    {
      // 1) (timeslot_no * 13) is computed in order to substract the considered beginning for this
      // timeslot and then always be in the range 0..103
      // 2) 104 is added in order to cope with negative numbers.
      l1s.actual_time.fn_in_report = (UWORD8)((l1s.actual_time.fn - (timeslot_no * 13) + 104) % 104);
      l1s.next_time.fn_in_report   = (UWORD8)((l1s.next_time.fn   - (timeslot_no * 13) + 104) % 104);
    }
  }
}

/*-------------------------------------------------------*/
/* l1s_encode_rxlev()                                    */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
WORD16 l1s_encode_rxlev(UWORD8 inlevel)
{
  WORD16  rxlev;

  rxlev = (221 - inlevel) / 2; // the result is divided by 2 due to
                               // the IL format is 7.1 and rxlev format is 8.0

  return(rxlev);
}

/*-------------------------------------------------------*/
/* l1s_send_ho_finished()                                */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1s_send_ho_finished(UWORD8  cause)
{
  xSignalHeaderRec *msg;

  msg = os_alloc_sig(sizeof(T_MPHC_HANDOVER_FINISHED));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = L1C_HANDOVER_FINISHED;
  ((T_MPHC_HANDOVER_FINISHED *)(msg->SigP))->cause = cause;

  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}


/*-------------------------------------------------------*/
/* l1s_get_versions()                                    */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality : return address of version structur    */
/*-------------------------------------------------------*/
T_VERSION *l1s_get_version (void)
{
  //update the fields not initialized by the sw init.

  #if (DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39)
    l1s.version.dsp_code_version  = l1s_dsp_com.dsp_ndb_ptr->d_version_number1;
    l1s.version.dsp_patch_version = l1s_dsp_com.dsp_ndb_ptr->d_version_number2;
    // Note: if l1s.version.dsp_checksum is not initialized (field set to 0)
    // use TST_TEST_HW_REQ message to initialize the whole structur.
  #else
    l1s.version.dsp_patch_version = l1s_dsp_com.dsp_param_ptr->d_version_number;
    // Note: if l1s.version.dsp_code_version and l1s.version.dsp_checksum
    // are not initialized (fields set to 0)
    // use TST_TEST_HW_REQ message to initialize the whole structur.
  #endif

  return (&l1s.version);
}

/*-------------------------------------------------------*/
/* l1s_reset_dedic_meas()                                */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1s_reset_dedic_serving_meas(void)
{
  // Reset rxlev related fields
  l1a_l1s_com.Scell_info.meas.acc      = 0;
  l1a_l1s_com.Scell_info.meas.nbr_meas = 0;
  l1a_l1s_com.Smeas_dedic.acc_sub      = 0;
  l1a_l1s_com.Smeas_dedic.nbr_meas_sub = 0;

  // Reset rxqual related fields
  l1a_l1s_com.Smeas_dedic.qual_acc_full      = 0;
  l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full = 0;
  l1a_l1s_com.Smeas_dedic.qual_acc_sub       = 0;
  l1a_l1s_com.Smeas_dedic.qual_nbr_meas_sub  = 0;


  #if REL99
  #if FF_EMR
    // Reset EMR variables
    l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc      = 0;
    l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas = 0;
    l1a_l1s_com.Smeas_dedic_emr.nbr_rcvd_blocks    = 0;
    l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc = 0;
    l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc   = 0;
    l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num = 0;
    l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num   = 0;
  #endif
  #endif


  // Reset dtx frame counter
  l1a_l1s_com.Smeas_dedic.dtx_used = 0;
}

/*-------------------------------------------------------*/
/* SwapIQ_dl()                                           */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
UWORD32 l1s_swap_iq_dl(UWORD16 radio_freq, UWORD8 task)
{
  UWORD8   swap_iq;
  UWORD32  task_tab= 0; //omaps00090550

#if (L1_FF_MULTIBAND == 0)  
  if(((l1_config.std.id == DUAL) || (l1_config.std.id == DUALEXT) || (l1_config.std.id == DUAL_US)) &&
     (radio_freq >= l1_config.std.first_radio_freq_band2))
  {
    swap_iq = l1_config.std.swap_iq_band2;
  }
  else
  {
    swap_iq = l1_config.std.swap_iq_band1;
  }
#else // L1_FF_MULTIBAND = 1 below

  UWORD16 physical_band_id;
  physical_band_id = 
    l1_multiband_radio_freq_convert_into_physical_band_id(radio_freq);
  swap_iq = rf_band[physical_band_id].swap_iq;
  
#endif // #if (L1_FF_MULTIBAND == 0) else  

  switch(swap_iq)
  {
    case 0:  /* No swap at all. */
    case 2:  /* DL, no swap.    */
      task_tab = (UWORD32)DSP_TASK_CODE[task];
    break;
    case 1:  /* DL I/Q swap.    */
    case 3:  /* DL I/Q swap.    */
       task_tab = (UWORD32)DSP_TASK_CODE[task];
       task_tab |= 0x8000L;
    break;
  }
  return(task_tab);
}

/*-------------------------------------------------------*/
/* l1s_swap_iq_ul()                                      */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
UWORD32 l1s_swap_iq_ul(UWORD16 radio_freq, UWORD8 task)
{
  UWORD8   swap_iq;
  UWORD32  task_tab = 0; //omaps00090550

#if (L1_FF_MULTIBAND == 0)

  if(((l1_config.std.id == DUAL) || (l1_config.std.id == DUALEXT) || (l1_config.std.id == DUAL_US)) &&
     (radio_freq >= l1_config.std.first_radio_freq_band2))
  {
    swap_iq = l1_config.std.swap_iq_band2;
  }
  else
  {
    swap_iq = l1_config.std.swap_iq_band1;
  }
#else // L1_FF_MULTIBAND = 1 below

 UWORD16 physical_band_id = 0;
  physical_band_id = 
    l1_multiband_radio_freq_convert_into_physical_band_id(radio_freq);
  swap_iq = rf_band[physical_band_id].swap_iq;
  
#endif // #if (L1_FF_MULTIBAND == 0) else  

  switch(swap_iq)
  {
    case 0: /* No swap at all. */
    case 1: /* UL, no swap.    */
      task_tab = (UWORD32)DSP_TASK_CODE[task];
    break;
    case 2: /* UL I/Q swap.    */
    case 3: /* UL I/Q swap.    */
       task_tab = (UWORD32)DSP_TASK_CODE[task];
       task_tab |= 0x8000L;
    break;
  }
  return(task_tab);
}


/*-------------------------------------------------------*/
/* l1s_ADC_decision_on_NP()                              */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
UWORD8 l1s_ADC_decision_on_NP(void)
{
  UWORD8  adc_active = INACTIVE;

  if (l1a_l1s_com.l1s_en_task[ALLC] == TASK_DISABLED) // no reorg mode
  {
     if (l1a_l1s_com.adc_mode & ADC_NEXT_NORM_PAGING)  // perform ADC only one time
     {
        adc_active = ACTIVE;
        l1a_l1s_com.adc_mode &= ADC_MASK_RESET_IDLE; // reset in order to have only one ADC measurement in Idle
     }
     else
     {
       if (l1a_l1s_com.adc_mode & ADC_EACH_NORM_PAGING) // perform ADC on each "period" x bloc
         if ((++l1a_l1s_com.adc_cpt)>=l1a_l1s_com.adc_idle_period) // wait for the period
         {
           adc_active = ACTIVE;
           l1a_l1s_com.adc_cpt = 0;
         }
     }
  }
  else  // ADC measurement in reorg mode
  {
     if (l1a_l1s_com.adc_mode & ADC_NEXT_NORM_PAGING_REORG)  // perform ADC only one time
     {
        adc_active = ACTIVE;
        l1a_l1s_com.adc_mode &= ADC_MASK_RESET_IDLE; // reset in order to have only one ADC measurement in Idle
     }
     else
     {
       if (l1a_l1s_com.adc_mode & ADC_EACH_NORM_PAGING_REORG) // perform ADC on each "period" x bloc
         if ((++l1a_l1s_com.adc_cpt)>=l1a_l1s_com.adc_idle_period) // wait for the period
         {
           adc_active = ACTIVE;
           l1a_l1s_com.adc_cpt = 0;
         }
     }
  }
  return(adc_active);
}


#if (AMR == 1)
/*-------------------------------------------------------*/
/* l1s_amr_get_ratscch_type()                            */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function returns the type of a RATSCCH block     */
/* Decoding is done according to ETSI spec 05.09         */
/*                                                       */
/* Input parameter:                                      */
/* ---------------                                       */
/* "a_ratscch"                                           */
/*    pointer to the RATSCCH block                       */
/*                                                       */
/* Output parameter:                                     */
/* ----------------                                      */
/* Type of RATSCCH block.                                */
/*  Can be:  C_RATSCCH_UNKNOWN                           */
/*           C_RATSCCH_CMI_PHASE_REQ                     */
/*           C_RATSCCH_AMR_CONFIG_REQ_MAIN               */
/*           C_RATSCCH_AMR_CONFIG_REQ_ALT                */
/*           C_RATSCCH_AMR_CONFIG_REQ_ALT_IGNORE         */
/*           C_RATSCCH_THRES_REQ                         */
/*                                                       */
/*-------------------------------------------------------*/
UWORD8 l1s_amr_get_ratscch_type(API *a_ratscch)
{
  // Check if the RATSCCH block is a CMI_PHASE_REQ block
  // -> if and only if bits 1, 3 through 34 are cleared and bit 2 is set
  if(((UWORD16)(a_ratscch[3] & 0xFFFE) == 0x0004) &&     // bits 1, 3-15 are cleared, bit 2 is set
     ((UWORD16)(a_ratscch[4]) == 0x0000) &&              // bits 16-31 are cleared
     ((UWORD16)(a_ratscch[5] & 0x0007) == 0x0000))       // bits 32-34 are cleared
  {
    return C_RATSCCH_CMI_PHASE_REQ;
  }

  // Check if the RATSCCH block is a THRES_REQ block
  // -> if and only if bits 31 through 34 are cleared and bit 30 is set
  if(((UWORD16)(a_ratscch[4] & 0xC000) == 0x4000) &&     // bit 30 is set, bit 31 is cleared
     ((UWORD16)(a_ratscch[5] & 0x0007) == 0x0000))       // bits 32-34 are cleared
  {
    return C_RATSCCH_THRES_REQ;
  }

  // Check if the RATSCCH block is a AMR_CONFIG_REQ block
  // -> if and only if bits 33-34 are cleared and bits 30-32 are set
  if(((UWORD16)(a_ratscch[4] & 0xC000) == 0xC000) &&     // bits 30-31 are set
     ((UWORD16)(a_ratscch[5] & 0x0007) == 0x0001))       // bit 32 is set, bits 33-34 are cleared
  {
    // Check if it's a main AMR_CONFIG_REQ block or an alternative AMR_CONFIG_REQ block
    UWORD16 ratscch_acs = (a_ratscch[4] & 0x0FF0) >> 4;  // get bits 20-27
    UWORD8 nb_coders,i;

    // Count number of active coders
    for(i=0, nb_coders=0; i<8; i++)
    {
      if((ratscch_acs & 1)==1)	 nb_coders++;
      ratscch_acs >>= 1;
    }

    // If the number of coders is 1, 2 or 3, it is a main AMR_CONFIG_REQ block
    if(nb_coders<=3)
      return C_RATSCCH_AMR_CONFIG_REQ_MAIN;

    // If the number of coders is more than 4, it is an alternate AMR_CONFIG_REQ block
    // Check if it must be ignored (block THRES_REQ pending) or not
    // -> if and only if bits 0 through 19 are set
    if(((UWORD16)(a_ratscch[3]) == 0xFFFF) &&            // bits 0-15 are set
       ((UWORD16)(a_ratscch[4] & 0x000F) == 0x000F))     // bits 16-19 are set
      return C_RATSCCH_AMR_CONFIG_REQ_ALT_IGNORE;
    else
      return C_RATSCCH_AMR_CONFIG_REQ_ALT;
  }

  // Block is not recognized
  return C_RATSCCH_UNKNOWN;
}


/*--------------------------------------------------------*/
/* l1s_amr_update_from_ratscch()                          */
/*--------------------------------------------------------*/
/*                                                        */
/* Description:                                           */
/* ------------                                           */
/* This function updates the AMR parameters modified by   */
/* the RATSCCH block received. This updates is done both  */
/* in the NDB and in the L1A/L1S communication structure  */
/* (aset pointer).                                        */
/* Data manipulation is done according to ETSI spec 05.08 */
/*                                                        */
/* Input parameter:                                       */
/* ---------------                                        */
/* "a_ratscch_dl"                                         */
/*    pointer to the RATSCCH block                        */
/*                                                        */
/* Output parameter:                                      */
/* ----------------                                       */
/*  n/a                                                   */
/*                                                        */
/*--------------------------------------------------------*/
void l1s_amr_update_from_ratscch(API *a_ratscch_dl)
{
  UWORD16 acs,hysteresis1,hysteresis2,hysteresis3,threshold1,threshold2,threshold3,icm,cmip;
  UWORD16 amr_change_bitmap=0;
  UWORD8  ratscch_type;
  BOOL    ratscch_unknown=TRUE; // No AMR parameters update

  // Get the RATSCCH block's type
  ratscch_type = l1s_amr_get_ratscch_type(a_ratscch_dl);

  // Check the RATSCCH block's type
  switch(ratscch_type)
  {
    case C_RATSCCH_CMI_PHASE_REQ:
    {
      // Copy CMIP to L1 structure
      cmip = a_ratscch_dl[3] & 0x0001;                  // bit 0
      l1a_l1s_com.dedic_set.aset->cmip=(UWORD8)cmip;
      amr_change_bitmap |= 1 << C_AMR_CHANGE_CMIP;
	  // AMR parameters update flag
      ratscch_unknown=FALSE;
    }
    break;
    case C_RATSCCH_AMR_CONFIG_REQ_MAIN:
    {
      // Copy ACS to L1 structure
      acs = (a_ratscch_dl[4] & 0x0FF0) >> 4;            // bits 20-27
      l1a_l1s_com.dedic_set.aset->amr_configuration.active_codec_set=(UWORD8)acs;
      amr_change_bitmap |= 1 << C_AMR_CHANGE_ACS;

      // Copy ICM to L1 structure
      icm = (a_ratscch_dl[4] & 0x3000) >> 12;           // bits 28-29
      l1a_l1s_com.dedic_set.aset->amr_configuration.initial_codec_mode=(UWORD8)icm;
      amr_change_bitmap |= 1 << C_AMR_CHANGE_ICM;

      // Copy hysteresis 1 to L1 structure
      hysteresis1 = (a_ratscch_dl[3] & 0x03C0) >> 6;    // bits 6-9
      l1a_l1s_com.dedic_set.aset->amr_configuration.hysteresis[0]=(UWORD8)hysteresis1;
      amr_change_bitmap |= 1 << C_AMR_CHANGE_HYST1;

      // Copy threshold 1 to L1 structure
      threshold1 = a_ratscch_dl[3] & 0x003F;            // bits 0-5
      l1a_l1s_com.dedic_set.aset->amr_configuration.threshold[0]=(UWORD8)threshold1;
      amr_change_bitmap |= 1 << C_AMR_CHANGE_THR1;

      // Copy hysteresis 2 to L1 structure
      hysteresis2 = a_ratscch_dl[4] & 0x000F;           // bits 16-19
      l1a_l1s_com.dedic_set.aset->amr_configuration.hysteresis[1]=(UWORD8)hysteresis2;
      amr_change_bitmap |= 1 << C_AMR_CHANGE_HYST2;

      // Copy threshold 2 to L1 structure
      threshold2 = (a_ratscch_dl[3] & 0xFC00) >> 10;    // bits 10-15
      l1a_l1s_com.dedic_set.aset->amr_configuration.threshold[1]=(UWORD8)threshold2;
      amr_change_bitmap |= 1 << C_AMR_CHANGE_THR2;
	  // AMR parameters update flag
      ratscch_unknown=FALSE;
    }
    break;
    case C_RATSCCH_AMR_CONFIG_REQ_ALT:
    {
      // Copy ACS to L1 structure
      acs = (a_ratscch_dl[4] & 0x0FF0) >> 4;            // bits 20-27
      l1a_l1s_com.dedic_set.aset->amr_configuration.active_codec_set=(UWORD8)acs;
      amr_change_bitmap |= 1 << C_AMR_CHANGE_ACS;

      // Copy ICM to L1 structure
      icm = (a_ratscch_dl[4] & 0x3000) >> 12;         // bits 28-29
      l1a_l1s_com.dedic_set.aset->amr_configuration.initial_codec_mode=(UWORD8)icm;
      amr_change_bitmap |= 1 << C_AMR_CHANGE_ICM;

      // Copy threshold 1 to L1 structure
      threshold1 = a_ratscch_dl[3] & 0x003F;           // bits 0-5
      l1a_l1s_com.dedic_set.aset->amr_configuration.threshold[0]=(UWORD8)threshold1;
      amr_change_bitmap |= 1 << C_AMR_CHANGE_THR1;

      // Copy threshold 2 to L1 structure
      threshold2 = (a_ratscch_dl[3] & 0x0FC0) >> 6;    // bits 6-11
      l1a_l1s_com.dedic_set.aset->amr_configuration.threshold[1]=(UWORD8)threshold2;
      amr_change_bitmap |= 1 << C_AMR_CHANGE_THR2;

      // Copy threshold 3 to L1 structure
      threshold3 = ((a_ratscch_dl[3] & 0xF000) >> 12) |  // bits 12-15
                   ((a_ratscch_dl[4] & 0x0003) << 4);    // bits 16-17
      l1a_l1s_com.dedic_set.aset->amr_configuration.threshold[2]=(UWORD8)threshold3;
      amr_change_bitmap |= 1 << C_AMR_CHANGE_THR3;

      // Copy hysteresis 1, 2 and 3 (common hysteresis) to L1 structure
      hysteresis1 = (a_ratscch_dl[4] & 0x000C) >> 2;     // bits 18-19
      hysteresis2 = hysteresis3 = hysteresis1;
      l1a_l1s_com.dedic_set.aset->amr_configuration.hysteresis[0]=
        l1a_l1s_com.dedic_set.aset->amr_configuration.hysteresis[1]=
        l1a_l1s_com.dedic_set.aset->amr_configuration.hysteresis[2]=(UWORD8)hysteresis1;
      amr_change_bitmap |= (1 << C_AMR_CHANGE_HYST1) | (1 << C_AMR_CHANGE_HYST2) | (1 << C_AMR_CHANGE_HYST3);
	  // AMR parameters update flag
      ratscch_unknown=FALSE;
    }
    break;
    case C_RATSCCH_AMR_CONFIG_REQ_ALT_IGNORE:
    {
      // Copy ACS to L1 structure
      acs = (a_ratscch_dl[4] & 0x0FF0) >> 4;            // bits 20-27
      l1a_l1s_com.dedic_set.aset->amr_configuration.active_codec_set=(UWORD8)acs;
      amr_change_bitmap |= 1 << C_AMR_CHANGE_ACS;

      // Copy ICM to L1 structure
      icm = (a_ratscch_dl[4] & 0x3000) >> 12;           // bits 28-29
      l1a_l1s_com.dedic_set.aset->amr_configuration.initial_codec_mode=(UWORD8)icm;
      amr_change_bitmap |= 1 << C_AMR_CHANGE_ICM;
	  // AMR parameters update flag
      ratscch_unknown=FALSE;
    }
    break;
    case C_RATSCCH_THRES_REQ:
    {
      // Copy hysteresis 1 to L1 structure
      hysteresis1 = (a_ratscch_dl[3] & 0x03C0) >> 6;    // bits 6-9
      l1a_l1s_com.dedic_set.aset->amr_configuration.hysteresis[0]=(UWORD8)hysteresis1;
      amr_change_bitmap |= 1 << C_AMR_CHANGE_HYST1;

      // Copy threshold 1 to L1 structure
      threshold1 = a_ratscch_dl[3] & 0x003F;            // bits 0-5
      l1a_l1s_com.dedic_set.aset->amr_configuration.threshold[0]=(UWORD8)threshold1;
      amr_change_bitmap |= 1 << C_AMR_CHANGE_THR1;

      // Copy hysteresis 2 to L1 structure
      hysteresis2 = a_ratscch_dl[4] & 0x000F;           // bits 16-19
      l1a_l1s_com.dedic_set.aset->amr_configuration.hysteresis[1]=(UWORD8)hysteresis2;
      amr_change_bitmap |= 1 << C_AMR_CHANGE_HYST2;

      // Copy threshold 2 to L1 structure
      threshold2 = (a_ratscch_dl[3] & 0xFC00) >> 10;    // bits 10-15
      l1a_l1s_com.dedic_set.aset->amr_configuration.threshold[1]=(UWORD8)threshold2;
      amr_change_bitmap |= 1 << C_AMR_CHANGE_THR2;

      // Copy hysteresis 3 to L1 structure
      hysteresis3 = (a_ratscch_dl[4] & 0x3C00) >> 10;   // bits 26-29
      l1a_l1s_com.dedic_set.aset->amr_configuration.hysteresis[2]=(UWORD8)hysteresis3;
      amr_change_bitmap |= 1 << C_AMR_CHANGE_HYST3;

      // Copy threshold 3 to L1 structure
      threshold3 = (a_ratscch_dl[4] & 0x03F0) >> 4;     // bits 20-25
      l1a_l1s_com.dedic_set.aset->amr_configuration.threshold[2]=(UWORD8)threshold3;
      amr_change_bitmap |= 1 << C_AMR_CHANGE_THR3;
	  // AMR parameters update flag
      ratscch_unknown=FALSE;
    }
    break;
    case C_RATSCCH_UNKNOWN:
    {
      // No AMR parameters update
      ratscch_unknown=TRUE;
    }
    break;
  }
  // AMR parameters update only if valid RATSCCH
  if(ratscch_unknown==FALSE)
  {
    // Update NDB with new AMR parameters
    l1ddsp_load_amr_param(l1a_l1s_com.dedic_set.aset->amr_configuration,l1a_l1s_com.dedic_set.aset->cmip);

#if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
    l1_trace_ratscch(l1s.actual_time.fn_mod42432,amr_change_bitmap);
#endif
  }
}

#endif    // AMR



/*--------------------------------------------------------*/
/* l1_memcpy_16bit()                                      */
/*--------------------------------------------------------*/
/*                                                        */
/* Description:                                           */
/* ------------                                           */
/* This function is equivalemt of memcopy. Thid function  */
/* does only 8/16 bit accessed to both source and         */
/* destination                                            */
/*                                                        */
/* Input parameter:                                       */
/* ---------------                                        */
/* "src" - input pointer                                  */
/* "len" - number of bytes to copy                        */
/*                                                        */
/* Output parameter:                                      */
/* ----------------                                       */
/*  "dst" - output pointer                                */
/*                                                        */
/*--------------------------------------------------------*/
void l1_memcpy_16bit(void *dst,void* src,unsigned int len)
{
	unsigned int i;
	unsigned int tempLen;
	unsigned char *cdst,*csrc;
	unsigned short *ssrc,*sdst;

	cdst=dst;
	csrc=src;
	sdst=dst;
	ssrc=src;

  if(((unsigned int)src&0x01) || ((unsigned int)dst&0x01)){
  // if either source or destination is not 16-bit aligned do the entire memcopy
  // in 8-bit
    for(i=0;i<len;i++){
      *cdst++=*csrc++;
    }
  }
  else{
    // if both the source and destination are 16-bit aligned do the memcopy
    // in 16-bits
    tempLen = len>>1;
    for(i=0;i<tempLen;i++){
      *sdst++ = *ssrc++;
    }
    if(len & 0x1){
      // if the caller wanted to copy odd number of bytes do a last 8-bit copy
      cdst=(unsigned char*)sdst;
      csrc=(unsigned char*)ssrc;
      *cdst++ = *csrc++;
    }
  }
  return;
}

/*-----------------------------------------------------------------*/
/* l1s_restore_synchro                                             */
/*-----------------------------------------------------------------*/
/* Description:                                                    */
/* ------------                                                    */
/* This function restores TPU synchro after an actiity             */
/* using synchro/synchro back scheme.                              */
/*                                                                 */
/* Input parameters:                                               */
/* -----------------                                               */
/* None                                                            */
/*                                                                 */
/* Input parameters from globals:                                  */
/* ------------------------------                                  */
/*  l1s.tpu_offset                                                 */
/*  l1s.next_time                                                  */
/*  l1s.next_plus_time                                             */
/*                                                                 */
/* Output parameters:                                              */
/* ------------------                                              */
/*  None                                                           */
/*                                                                 */
/* Modified parameters from globals:                               */
/* ---------------------------------                               */
/*  l1s.actual_time                                                */
/*  l1s.next_time                                                  */
/*  l1s.next_plus_time                                             */
/*  l1s.tpu_ctrl_reg                                               */
/*  l1s.dsp_ctrl_reg                                               */
/*-----------------------------------------------------------------*/
void l1s_restore_synchro(void)
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

  l1s.tpu_ctrl_reg |= CTRL_SYCB;
  l1s.dsp_ctrl_reg |= CTRL_SYNC;

#if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
  trace_fct(CST_L1S_ADJUST_TIME, (UWORD32)(-1));
#endif
}

#if (FF_L1_FAST_DECODING == 1)
BOOL l1s_check_deferred_control(UWORD8 task, UWORD8 burst_id)
{
  /* Control activities are performed only if:
      - Fast decoding is not authorized
      - Fast decoding authorized, control running inside the fast HISR context and not first burst
      - Fast decoding authorized, control running inside L1S context and first burst */

  /* Running from fast API HISR? */
  BOOL fast_decoding_hisr = (l1a_apihisr_com.fast_decoding.status == C_FAST_DECODING_PROCESSING);

  if (fast_decoding_hisr && (burst_id == BURST_1))
  {
    /* Error this case shouldn't happen */
    return TRUE;
  }
  else if (!fast_decoding_hisr && (burst_id != BURST_1))
  {
    /* Currently running from L1S, control must be performed on the upcoming fast HISR */
    l1a_apihisr_com.fast_decoding.task                 = task;
    l1a_apihisr_com.fast_decoding.burst_id             = burst_id;
    /* If a tasks semaphore get SET do not do deferred control */
    if(!(l1a_l1s_com.task_param[task] == SEMAPHORE_SET))
    {
      l1a_apihisr_com.fast_decoding.deferred_control_req = TRUE;
    return TRUE;
  }
  }
  else if (!fast_decoding_hisr && (burst_id == BURST_1))
  {
    /* Control running from L1S for the first burst => Control must be performed now. */
    /* As a result, a fast API IT will be triggered on the next frame                 */

    if (l1a_apihisr_com.fast_decoding.status == C_FAST_DECODING_AWAITED)
    {
      /* A fast API IT was already awaited. It means that we are starting the fast decoding */
      /* of a new block before the previous one is finished.                                */
      /* This case is signaled through the variable below so the status can stay as awaited */
      /* for the first fast API IT of the new block.                                        */
      l1a_apihisr_com.fast_decoding.contiguous_decoding = TRUE;
    }
    else
    {
      l1a_apihisr_com.fast_decoding.status = C_FAST_DECODING_AWAITED;
    }
    l1a_apihisr_com.fast_decoding.task                 = task;
    return FALSE;
}
  /* In other cases do control now. */
  return FALSE;
} /* end function l1s_check_deferred_control */

BOOL l1s_check_fast_decoding_authorized(UWORD8 task)
{
  BOOL result = FALSE;

  /* Is a fast decoding already in progress (AWAITED or PROCESSING states)?          */
  /* Is a fast decoding complete but waiting for the read activity (COMPLETE state)? */
  /* In that case, it will continue, even if a mode change has occured.              */
  BOOL already_in_progress = (    (l1a_apihisr_com.fast_decoding.status == C_FAST_DECODING_AWAITED)
                               || (l1a_apihisr_com.fast_decoding.status == C_FAST_DECODING_PROCESSING)
                               || (l1a_apihisr_com.fast_decoding.status == C_FAST_DECODING_COMPLETE) );

  /* One variable used later that contains the status of several tasks */
  BOOL no_serving_audio_and_neighbour_tasks = (
                                            (l1a_l1s_com.l1s_en_task[EP] == TASK_DISABLED)
                                         && (l1a_l1s_com.l1s_en_task[ALLC] == TASK_DISABLED)
                                         && (l1a_l1s_com.l1s_en_task[NSYNC] == TASK_DISABLED)
                                         && (l1a_l1s_com.l1s_en_task[FBNEW] == TASK_DISABLED)
                                         && (l1a_l1s_com.l1s_en_task[SBCONF] == TASK_DISABLED)
                                         && (l1a_l1s_com.l1s_en_task[BCCHN] == TASK_DISABLED)
                                         && (l1a_l1s_com.l1s_en_task[EBCCHS] == TASK_DISABLED)
                                         //&& (l1a_l1s_com.l1s_en_task[NBCCHS] == TASK_DISABLED)
                                         && (l1a_l1s_com.l1s_en_task[BCCHN_TOP] == TASK_DISABLED)
#if (L1_GPRS)
                                         && (l1a_l1s_com.l1s_en_task[PBCCHS] == TASK_DISABLED)
                                         && (l1a_l1s_com.l1s_en_task[PEP] == TASK_DISABLED)
                                         && (l1a_l1s_com.l1s_en_task[PALLC] == TASK_DISABLED)
                                         && (l1a_l1s_com.l1s_en_task[PBCCHN_IDLE] == TASK_DISABLED)
#endif /* L1_GPRS */
                                         //&& (l1a_l1s_com.l1s_en_task[SMSCB] == TASK_DISABLED)
#if (L1_MP3 == 1)  
                                         && (l1a_apihisr_com.mp3.running == FALSE)
#endif
#if (L1_AAC == 1)      
                                         && (l1a_apihisr_com.aac.running == FALSE)
#endif
                                        );

  /* If fast decoding is already forbidden, do not enable it until the end of the block. */
  /* The forbidden status is reset at the first control of the block                     */
  if (l1a_apihisr_com.fast_decoding.status == C_FAST_DECODING_FORBIDDEN)
  {
    return FALSE;
  }

  switch(task)
  {
    case NP:
    {
      /* Enable Fast Paging (NP) except if CCCH reorg*/
      if (  ( already_in_progress == TRUE )
          ||
            (    (l1a_l1s_com.mode == I_MODE)
              && (l1a_l1s_com.l1s_en_task[NP] == TASK_ENABLED)
              && (no_serving_audio_and_neighbour_tasks == TRUE) )
         )
      {
        result = TRUE;
      }
      break;
    } /* case NP */

    case NBCCHS:
    {
      /* Enable Fast Paging (NP) except if CCCH reorg*/
      if (  ( already_in_progress == TRUE )
          ||
            (    (l1a_l1s_com.mode == I_MODE)
              && (l1a_l1s_com.l1s_en_task[NBCCHS] == TASK_ENABLED)
              && (no_serving_audio_and_neighbour_tasks == TRUE) )
         )
      {
        result = TRUE;
      }
      break;
    } /* case NBCCHS */

#if (L1_GPRS)
    case PNP:
    {
      /* Enable Fast Paging (PNP) except if PCCCH reorg*/
      if (  ( already_in_progress == TRUE )
          ||
            (    (l1a_l1s_com.mode == I_MODE)
              && (l1a_l1s_com.l1s_en_task[PNP] == TASK_ENABLED)
              && (no_serving_audio_and_neighbour_tasks == TRUE)
            )
         )
      {
        result = TRUE;
      }
      break;
    } /* case PNP */
#endif /* L1_GPRS*/

  } /* switch(task) */

#if (L1_GPRS)
  if ((result == FALSE) && ((task == NP) || (task == PNP) || (task == NBCCHS)))
#else  /* NO_GPRS*/
  if ((result == FALSE) && ((task == NP) || (task == NBCCHS)))
#endif /* L1_GPRS */
  {
    l1a_apihisr_com.fast_decoding.status = C_FAST_DECODING_FORBIDDEN;
  }

  return result;
} /* end function l1s_check_fast_decoding_authorized */

#endif /* FF_L1_FAST_DECODING */
/*-----------------------------------------------------------------*/
/* l1s_check_sacch_dl_block                                        */
/*-----------------------------------------------------------------*/
/* Description:                                                    */
/* ------------                                                    */
/* Downlink SACCH buffer comparison function for FER Traces        */
/* This is called only when there is a successfully decoded        */
/* block. The count of no of successfully decoded SACCH  blocks    */
/* is  updated.                                                    */
/*                                                                 */
/* Input parameters:                                               */
/* -----------------                                               */
/*  sacch_dl_block  "Downlink SACCH BLOCK"                         */
/*                                                                 */
/* Output parameters:                                              */
/* ------------------                                              */
/*  None                                                           */
/*                                                                 */
/*-----------------------------------------------------------------*/
#if ((FF_REPEATED_SACCH) && (TRACE_TYPE ==1 || TRACE_TYPE == 4))

void l1s_check_sacch_dl_block(API *sacch_dl_block)
{
     int i,j,repeat=1;
     if( trace_info.repeat_sacch.dl_buffer_empty == FALSE )
     {
           for(i=3,j=0;i<15;i++,j++)
           {
               if(trace_info.repeat_sacch.dl_buffer[j] != sacch_dl_block[i])
              {
                   break;
               }
           }
           if( i != 15 )
           {
                repeat=0;
           }
     }
     else /* if( trace_info.repeat_sacch.dl_buffer_empty == FALSE ) */
     {
         repeat=0;
     }   /* end else empty DL SACCH buffer*/
     if(repeat == 0)
     {
           trace_info.repeat_sacch.dl_good_norep++;
           for ( i=3 ; i<15 ; i++ )
          {
               trace_info.repeat_sacch.dl_buffer[i] = sacch_dl_block[i];// info_address[i];
          }
          trace_info.repeat_sacch.dl_buffer_empty = FALSE;
     } /* end if repeat = 0*/
     else
     {
        trace_info.repeat_sacch.dl_buffer_empty = TRUE;
     }  /* end else repeat = 1*/
} /* end function void l1s_check_sacch_dl_block */
#endif /*  ((FF_REPEATED_SACCH) && (TRACE_TYPE ==1 || TRACE_TYPE == 4)) */


/*-----------------------------------------------------------------*/
/* l1s_store_sacch_buffer                                          */
/*-----------------------------------------------------------------*/
/* Description:                                                    */
/* ------------                                                    */
/* Function to store data in case of a retransmission.             */
/*                                                                 */
/*                                                                 */
/* Input parameters:                                               */
/* -----------------                                               */
/*  sacch_ul_block  "SACCH Uplink block to be stored"              */
/*  repeat_sacch    "The buffer tocontain the stored block"        */
/*                                                                 */
/* Output parameters:                                              */
/* ------------------                                              */
/*  None                                                           */
/*                                                                 */
/*-----------------------------------------------------------------*/

#if (FF_REPEATED_SACCH == 1 )
void l1s_store_sacch_buffer(T_REPEAT_SACCH *repeat_sacch, UWORD8 *sacch_ul_block)
{
       int i=0;
       /* Store the first 11 words after header in the first 22 bytes. */
       for(i=0;i<23;i++)
       {
          repeat_sacch->buffer[i] = sacch_ul_block[i] ;
       }
       repeat_sacch->buffer_empty = FALSE;
}
#endif /* (FF_REPEATED_SACCH == 1 ) */


/*-----------------------------------------------------------------*/
/* l1s_repeated_facch_check                                                                           */
/*-----------------------------------------------------------------*/
/* Description:                                                                                               */
/* ------------                                                                                           */
/* If two successfully decoded blocks (separated by 8 or 9 frames)  are             */
/*  identical then it returns a NULL buffer otherwise a pointer to the last block      */
/*   data.                                                                                                       */
/*                                                                                                                 */
/*                                                                                                                 */
/* Input parameters:                                                                                      */
/* -----------------                                                                                  */
/*                         "FACCH  block to be stored"                                               */
/*                                                                                                                */
/* Output parameters:                                                                                  */
/* ------------------                                                                               */
/*  None                                                                                                       */
/*                                                                                                                  */
/*-----------------------------------------------------------------*/


#if ( FF_REPEATED_DL_FACCH == 1 )
API * l1s_repeated_facch_check(API *info_address)
{
      unsigned int repeat=1;
      unsigned int i,j;
      UWORD8 counter_candidate;
  
     counter_candidate=l1s.repeated_facch.counter_candidate;
     if( l1s.repeated_facch.pipeline[counter_candidate].buffer_empty == FALSE )
    {     
         for(i=3,j=0;i<15;j++,i++)
        {
            if(l1s.repeated_facch.pipeline[counter_candidate].buffer[j] != info_address[i])
           {  
               break;
            }   
         }  
         if( i != 15 ) 
        {  
            repeat=0;
         }  
    } 
    else
   {
        repeat=0;
    } /* end else buffer empty*/
#if TESTMODE
      if(l1_config.repeat_facch_dl_enable != REPEATED_FACCHDL_ENABLE)  // repeated FACCH mode is disabled
     {  
          repeat = 0;
     }   
#endif
     if(repeat == 0)
    {
         return &info_address[0];
     } 
    else
   {
    #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
         trace_info.facch_dl_repetition_block_count++; 
    #endif
    if (((l1s.actual_time.fn - fn_prev ) == 8) || ((l1s.actual_time.fn - fn_prev ) == 9 )) // added debug
    return (API)NULL;
    else
    return &info_address[0];
    }
}
#endif /*  FF_REPEATED_DL_FACCH == 1 */



#if ( FF_REPEATED_DL_FACCH == 1 )
void l1s_store_facch_buffer(T_REPEAT_FACCH *repeated_facch, API *facch_block)
{
      int i;
	UWORD8 counter_candidate; 
	fn_prev = l1s.actual_time.fn ;// added
      counter_candidate=repeated_facch->counter_candidate;
       /*  Store the first 12 words after header in the first 23 bytes. */
       for(i=0;i<13;i++)
       {
          repeated_facch->pipeline[counter_candidate].buffer[i] = facch_block[i] ;
       }
       repeated_facch->pipeline[counter_candidate].buffer_empty = FALSE;
}
#endif /* ( FF_REPEATED_DL_FACCH == 1 ) */

#if(L1_FF_MULTIBAND == 1)

#if 0

/*-------------------------------------------------------*/
/* l1_multiband_radio_freq_convert_into_effective_band_id*/
/*-------------------------------------------------------*/
/* Parameters : radio_freq the frequency to convert      */
/*                                                       */
/*                                                       */
/*                                                       */
/* Return     : the ID of the effectiev band in which    */
/* is located radio_freq                                 */
/* Functionality : compare radio_freq with the effective */
/* bands ranges, return efective_band_id                 */
/*                                                       */
/*                                                       */
/*-------------------------------------------------------*/
UWORD8  l1_multiband_radio_freq_convert_into_effective_band_id(UWORD16 radio_freq)
{
  UWORD8 effective_band_id = 0;
  while( effective_band_id < NB_MAX_EFFECTIVE_SUPPORTED_BANDS)
  {  
    if ((radio_freq >= multiband_conversion_data[effective_band_id].first_radio_freq)
       && (radio_freq < (multiband_conversion_data[effective_band_id].first_radio_freq + multiband_conversion_data[effective_band_id].nbmax_carrier)) )

    {
      return(effective_band_id);
    }
    else
    {
     effective_band_id ++;
    }
  }
  if(effective_band_id == NB_MAX_EFFECTIVE_SUPPORTED_BANDS)
  {
    l1_multiband_error_handler(radio_freq);
  }
  return(effective_band_id);

}
/*-------------------------------------------------------*/
/* l1_multiband_radio_freq_convert_into_physical_band_id */
/*-------------------------------------------------------*/
/* Parameters : radio_freq the frequency to convert      */
/*                                                       */
/*                                                       */
/*                                                       */
/* Return     : the ID of the physical_band band in which*/
/*  radio_freq is located                                */
/* Functionality : Identify effective_band_id, the ID of */
/* the effective band in whicb radio_freq is located     */
/* then derive physical_band_id from effective_band_id   */
/*-------------------------------------------------------*/

UWORD8 l1_multiband_radio_freq_convert_into_physical_band_id(UWORD16 radio_freq)
{ 
  UWORD8 effective_band_id, physical_band_id;
  effective_band_id = l1_multiband_radio_freq_convert_into_effective_band_id(radio_freq);
  physical_band_id = multiband_conversion_data[effective_band_id].physical_band_id;
  return(physical_band_id);
}

/*-------------------------------------------------------*/
/* l1_multiband_radio_freq_convert_into_operative_radio_freq*/
/*-------------------------------------------------------*/
/* Parameters : radio_freq the frequency to convert      */
/*                                                       */
/*                                                       */
/*                                                       */
/* Return     : the operative_radio_freq corresponding to radio_freq */
/* Functionality : identify effective_band_id, then      */
/* based on the relationships linking the ranges of operative_radio_freq*/
/* and radio_freq , derive operative_radio_freq           */
/*-------------------------------------------------------*/
UWORD16 l1_multiband_radio_freq_convert_into_operative_radio_freq(UWORD16 radio_freq)
{
  UWORD8 effective_band_id;
  UWORD16 operative_radio_freq;
  effective_band_id = l1_multiband_radio_freq_convert_into_effective_band_id(radio_freq);
  operative_radio_freq = radio_freq - multiband_conversion_data[effective_band_id].first_radio_freq + multiband_conversion_data[effective_band_id].first_operative_radio_freq;
  return(operative_radio_freq);
} 
/*--------------------------------------------------------*/
/* l1_multiband_map_radio_freq_into_tpu_table             */
/*--------------------------------------------------------*/
/* Parameters :                                           */
/* radio_freq the parameter to be converted               */
/*                                                        */
/* Return     : the index in table rf_band or rf_tpu_band */
/* corresponding to radio_freq                            */
/* Functionality :identify physical_band_id               */
/* then derive from physical_band_id, tpu_band_index to be*/
/* returned a physical band having the ID physical_band_id*/
/* is mapped to the table  rf_band[physical_band_id ]     */                                                    
/*--------------------------------------------------------*/
UWORD8 l1_multiband_map_radio_freq_into_tpu_table(UWORD16 radio_freq)
{
  UWORD8 tpu_table_index = 0;
  UWORD8 physical_band_id = 0;
  physical_band_id =  l1_multiband_radio_freq_convert_into_physical_band_id(radio_freq);
  /*For Neptune a band having the ID physical_band_id is mapped to multiband_rf_data[physical_band_id], rf_band[physical_band_id]*/
  /*Consequently the existence of this API for API is not necessary since it is redundant with l1_multiband_radio_freq_convert_into_physical_band_id*/
  tpu_table_index = physical_band_id;
  return(tpu_table_index);
}
/*--------------------------------------------------------*/
/* l1_multiband_error_handler                             */
/*--------------------------------------------------------*/
/* Parameters :                                           */
/* radio_freq the channel number received from the L3     */
/*                                                        */
/* Return     :                                           */
/* corresponding to radio_freq                            */
/* Functionality :handling error code of MULTIBAND        */                                                    
/*--------------------------------------------------------*/
void l1_multiband_error_handler(UWORD16 radio_freq)
{
  L1_MULTIBAND_TRACE_PARAMS(MULTIBAND_ERROR_TRACE_ID, 1);
#if (OP_L1_STANDALONE == 1)
#if(CODE_VERSION == NOT_SIMULATION)
  L1BSP_error_handler();
#endif /*if(CODE_VERSION == NOT_SIMULATION)*/
#endif
}
#endif // if 0
#endif   /*if (L1_FF_MULTIBAND == 1)*/ 

#if (OP_L1_STANDALONE == 1)

UWORD8 l1_get_pwr_mngt()
{
    return(l1_config.pwr_mngt);
}

#endif 

void l1_multiband_error_handler(UWORD16 radio_freq)
{
    while(1);
}


