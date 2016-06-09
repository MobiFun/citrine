/************ Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_INIT.C
 *
 *        Filename l1_init.c
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

#define  L1_INIT_C

#include "config.h"
#include "l1_confg.h"

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
  #if (L1_DYN_DSP_DWNLD == 1)
    #include "l1_dyn_dwl_proto.h"
  #endif

  #include "l1_defty.h"
  #include "cust_os.h"
  #include "l1_msgty.h"
  #include "l1_varex.h"
  #include "l1_proto.h"
  #include "l1_mftab.h"
  #include "l1_tabs.h"
  #include "l1_ver.h"
  #include "ulpd.h"

  #include "l1_proto.h"

  #if L1_GPRS
    #include "l1p_cons.h"
    #include "l1p_msgt.h"
    #include "l1p_deft.h"
    #include "l1p_vare.h"
    #include "l1p_tabs.h"
    #include "l1p_macr.h"
    #include "l1p_ver.h"
  #endif

  #if TESTMODE
    #include "l1tm_ver.h"
  #endif

  #include <stdio.h>
  #include "sim_cfg.h"
  #include "sim_cons.h"
  #include "sim_def.h"
  #include "sim_var.h"

#else // NO SIMULATION

  #include <string.h>
  /* #include "tm_defs.h" */
  #include "l1_types.h"
  #include "sys_types.h"
  #include "../dsp/leadapi.h"
  #include "l1_const.h"
  #include "l1_macro.h"
  #include "l1_time.h"
  #include "l1_signa.h"
  #if (AUDIO_TASK == 1)
    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_defty.h"
  #endif


  #include "../../bsp/abb+spi/spi_drv.h"
  #include "../../bsp/abb+spi/abb.h"
  #if (ANALOG != 11)
  #include "../../bsp/abb+spi/abb_core_inth.h"
  #endif

  #if TESTMODE
    #include "l1tm_defty.h"
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
  #if (L1_DYN_DSP_DWNLD == 1)
    #include "l1_dyn_dwl_proto.h"
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

  #if (CHIPSET == 12) || (CHIPSET == 15)
    #include "sys_inth.h"
  #else
    #include "../../bsp/mem.h"
    #include "../../bsp/inth.h"
    #include "../../bsp/dma.h"
    #include "../../bsp/iq.h"
  #endif

  #include "../../bsp/clkm.h"
  #include "../../bsp/rhea_arm.h"
  #include "../../bsp/ulpd.h"

  #include "l1_proto.h"

  #if L1_GPRS
    #include "l1p_cons.h"
    #include "l1p_msgt.h"
    #include "l1p_deft.h"
    #include "l1p_vare.h"
    #include "l1p_tabs.h"
    #include "l1p_macr.h"
    #include "l1p_ver.h"
  #endif

  #if TESTMODE
    #include "l1tm_ver.h"
  #endif

#endif // NOT SIMULATION



#if (RF_FAM == 61)
#if (DRP_FW_EXT==0)
  #include "drp_drive.h"
  #include "drp_api.h"
  #include "l1_rf61.h"
  #include "apc.h"
#else
  #include "l1_rf61.h"
  #include "l1_drp_inc.h"
#endif
#endif


#if (RF_FAM == 60)
  #include "drp_drive.h"
  #include "drp_api.h"
  #include "l1_rf60.h"
#endif

#if (TRACE_TYPE == 1)||(TRACE_TYPE == 4)
  #include "l1_trace.h"
#endif

  #include <string.h>
  #include <stdio.h>

#if (ANALOG == 11)
  #include "bspTwl3029_I2c.h"
  #include "bspTwl3029_Aud_Map.h"
  #include "bspTwl3029_Madc.h"
#endif

#if (RF_FAM == 61)
//OMAPS148175
#include "l1_drp_if.h"
#include "drp_main.h" 
#endif

#if (ANALOG == 11)
#if (L1_MADC_ON == 1)
extern BspTwl3029_MadcResults l1_madc_results;
extern void l1a_madc_callback(void);
#if(OP_L1_STANDALONE == 1 || L1_NAVC == 1 )//NAVC
extern UWORD32 Cust_navc_ctrl_status(UWORD8  d_navc_start_stop_read);//NAVC
#endif
#endif

#if (AUDIO_DEBUG == 1)
extern UWORD8 audio_reg_read_status;
#endif

#endif

#if (AUDIO_TASK == 1)
  /**************************************/
  /* External audio prototypes          */
  /**************************************/
  extern void l1audio_initialize_var  (void);
#endif

extern void l1audio_dsp_init        (void);
extern void initialize_wait_loop(void);

#if (L1_GPRS)
  // external functions from GPRS implementation
  void initialize_l1pvar(void);
  void l1pa_reset_cr_freq_list(void);
#endif // L1_GPRS
#if ((OP_L1_STANDALONE == 1) && ((DSP == 38)|| (DSP == 39))&& (CODE_VERSION != SIMULATION))
   extern void l1_api_dump(void);
#endif

#if (TRACE_TYPE==3)
    void reset_stats();
#endif // TRACE_TYPE

#if (L1_GTT == 1)
  extern void l1gtt_initialize_var(void);
#endif

#if (L1_MP3 == 1)
  extern void l1mp3_initialize_var(void);
#endif

#if (L1_MIDI == 1)
  extern void l1midi_initialize_var(void);
#endif
//ADDED FOR AAC
#if (L1_AAC == 1)
  extern void l1aac_initialize_var(void);
#endif

#if ((TRACE_TYPE==1) || (TRACE_TYPE==2) || (TRACE_TYPE==3) || (TRACE_TYPE==4) || (TRACE_TYPE==7))
   extern void L1_trace_string(char *s);
#endif

#if (RF_FAM == 60 || RF_FAM == 61)
  extern const UWORD8  drp_ref_sw[] ;
extern T_DRP_REGS_STR  *drp_regs;
extern T_DRP_SRM_API* drp_srm_api;

extern T_DRP_SW_DATA drp_sw_data_calib;
extern T_DRP_SW_DATA drp_sw_data_init;

#endif

/*-------------------------------------------------------*/
/* l1_dsp_init()                                         */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1_dsp_init(void)
{
  //int i;-OMAPS90550- new
  #if (CODE_VERSION == SIMULATION)
    // L1S <-> DSP communication...
    //====================================================
    l1s_dsp_com.dsp_ndb_ptr     = &(buf.ndb);
    l1s_dsp_com.dsp_db_r_ptr    = &(buf.mcu_rd[0]);
    l1s_dsp_com.dsp_db_w_ptr    = &(buf.mcu_wr[0]);
    l1s_dsp_com.dsp_param_ptr   = &(buf.param);
    l1s_dsp_com.dsp_w_page      = 0;
    l1s_dsp_com.dsp_r_page      = 0;
    l1s_dsp_com.dsp_r_page_used = 0;

    #if (L1_GPRS)
      l1ps_dsp_com.pdsp_ndb_ptr   = &(buf.ndb_gprs);
      l1ps_dsp_com.pdsp_db_r_ptr  = &(buf.mcu_rd_gprs[0]);
      l1ps_dsp_com.pdsp_db_w_ptr  = &(buf.mcu_wr_gprs[0]);
      l1ps_dsp_com.pdsp_param_ptr = &(buf.param_gprs);
    #endif

    // Reset DSP page bit and DSP enable bit...
    //====================================================
    l1s_tpu_com.reg_cmd->dsp_enb_bit = OFF;
    l1s_tpu_com.reg_cmd->dsp_pag_bit = 0;

    // Set EOTD bit if required
    //====================================================
    #if (L1_EOTD ==1)
       l1s_dsp_com.dsp_ndb_ptr->d_tch_mode |= B_EOTD;
    #endif


  #else // NO SIMULATION

    // L1S <-> DSP communication...
    //====================================================
    l1s_dsp_com.dsp_ndb_ptr     = (T_NDB_MCU_DSP *)   NDB_ADR;
    l1s_dsp_com.dsp_db_r_ptr    = (T_DB_DSP_TO_MCU *) DB_R_PAGE_0;
    l1s_dsp_com.dsp_db_w_ptr    = (T_DB_MCU_TO_DSP *) DB_W_PAGE_0;
    l1s_dsp_com.dsp_param_ptr   = (T_PARAM_MCU_DSP *) PARAM_ADR;
    l1s_dsp_com.dsp_w_page      = 0;
    l1s_dsp_com.dsp_r_page      = 0;
    l1s_dsp_com.dsp_r_page_used = 0;

    #if (DSP == 38) || (DSP == 39)
    l1s_dsp_com.dsp_db_common_w_ptr = (T_DB_COMMON_MCU_TO_DSP *)DB_COMMON_W_PAGE_0;
    #endif

    /* DSP CPU load measurement */
    #if (DSP == 38) || (DSP == 39)
    l1s_dsp_com.dsp_cpu_load_db_w_ptr = (T_DB_MCU_TO_DSP_CPU_LOAD *)DSP_CPU_LOAD_DB_W_PAGE_0;
    (*((volatile UWORD16 *)(DSP_CPU_LOAD_MCU_W_CTRL))) = (API)0x0001;  // enable DSP CPU load measurement
    #endif

    #if (L1_GPRS)
      l1ps_dsp_com.pdsp_ndb_ptr   = (T_NDB_MCU_DSP_GPRS *)   NDB_ADR_GPRS;
      l1ps_dsp_com.pdsp_db_r_ptr  = (T_DB_DSP_TO_MCU_GPRS *) DB_R_PAGE_0_GPRS;
      l1ps_dsp_com.pdsp_db_w_ptr  = (T_DB_MCU_TO_DSP_GPRS *) DB_W_PAGE_0_GPRS;
      l1ps_dsp_com.pdsp_param_ptr = (T_PARAM_MCU_DSP_GPRS *) PARAM_ADR_GPRS;
    #endif

    #if (DSP_DEBUG_TRACE_ENABLE == 1)
      l1s_dsp_com.dsp_db2_current_r_ptr = (T_DB2_DSP_TO_MCU *) DB2_R_PAGE_0;
      l1s_dsp_com.dsp_db2_other_r_ptr   = (T_DB2_DSP_TO_MCU *) DB2_R_PAGE_1;
    #endif

    // Reset DSP page bit and DSP enable bit...
    //====================================================

    (*(volatile UWORD16 *)l1s_tpu_com.reg_cmd) &= ~TPU_CTRL_D_ENBL;

    #if (DSP >= 33)
      l1s_dsp_com.dsp_ndb_ptr->d_dsp_page = l1s_dsp_com.dsp_w_page;
    #else
      l1s_dsp_com.dsp_param_ptr->d_dsp_page = l1s_dsp_com.dsp_w_page;
    #endif

    // NDB init : Reset buffers and set flags...
    //====================================================
    l1s_dsp_com.dsp_ndb_ptr->d_fb_mode        = FB_MODE_1;
    l1s_dsp_com.dsp_ndb_ptr->d_fb_det         = FALSE;         // D_FB_DET  =0
    l1s_dsp_com.dsp_ndb_ptr->a_cd[0]          = (1<<B_FIRE1);  // B_FIRE1 =1, B_FIRE0 =0 , BLUD =0
    l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0]        = 0;             // BLUD = 0
    l1s_dsp_com.dsp_ndb_ptr->a_dd_0[2]        = 0xffff;        // NERR = 0xffff
    l1s_dsp_com.dsp_ndb_ptr->a_dd_1[0]        = 0;             // BLUD = 0
    l1s_dsp_com.dsp_ndb_ptr->a_dd_1[2]        = 0xffff;        // NERR = 0xffff
    l1s_dsp_com.dsp_ndb_ptr->a_du_0[0]        = 0;             // BLUD = 0
    l1s_dsp_com.dsp_ndb_ptr->a_du_0[2]        = 0xffff;        // NERR = 0xffff
    l1s_dsp_com.dsp_ndb_ptr->a_du_1[0]        = 0;             // BLUD = 0
    l1s_dsp_com.dsp_ndb_ptr->a_du_1[2]        = 0xffff;        // NERR = 0xffff
    l1s_dsp_com.dsp_ndb_ptr->a_fd[0]          = (1<<B_FIRE1);  // B_FIRE1 =1, B_FIRE0 =0 , BLUD =0
    l1s_dsp_com.dsp_ndb_ptr->a_fd[2]          = 0xffff;        // NERR = 0xffff
    l1s_dsp_com.dsp_ndb_ptr->d_a5mode         = 0;

    #if ((ANALOG == 1) || (ANALOG == 2)  || (ANALOG == 3) || (ANALOG == 11))
      l1s_dsp_com.dsp_ndb_ptr->d_tch_mode     = 0x0800;  // Analog base band selected = Nausica, Iota, Syren (bit 11)
    #endif

    #if ((ANALOG == 1) || (ANALOG == 2)  || (ANALOG == 3))
      l1s_dsp_com.dsp_ndb_ptr->d_tch_mode    |= (((l1_config.params.guard_bits - 4) & 0x000F) << 7); //Bit 7..10: guard bits
    #endif
    #if (ANALOG == 11)
      l1s_dsp_com.dsp_ndb_ptr->d_tch_mode    |= (((l1_config.params.guard_bits) & 0x000F) << 7); //Bit 7..10: guard bits
    #endif

    #if (DSP == 32)
      l1s_dsp_com.dsp_ndb_ptr->d_tch_mode    |= 0x2;
    #endif // OP_WCP

    l1s_dsp_com.dsp_ndb_ptr->a_sch26[0]       = (1<<B_SCH_CRC);// B_SCH_CRC =1, BLUD =0
    l1audio_dsp_init();

  #if IDS
    l1s_dsp_com.dsp_ndb_ptr->d_ra_conf        = 0;             // IDS
    l1s_dsp_com.dsp_ndb_ptr->d_ra_act         = 0;             // IDS
    l1s_dsp_com.dsp_ndb_ptr->d_ra_test        = 0;             // IDS
    l1s_dsp_com.dsp_ndb_ptr->d_ra_statu       = 0;             // IDS
    l1s_dsp_com.dsp_ndb_ptr->d_ra_statd       = 0;             // IDS
    l1s_dsp_com.dsp_ndb_ptr->d_fax            = 0;             // IDS
  #endif

#if(RF_FAM != 61)
    // interrupt rif TX on FIFO <= threshold with threshold = 0
    l1s_dsp_com.dsp_ndb_ptr->d_spcx_rif       = 0x179;
#else
//    l1s_dsp_com.dsp_ndb_ptr->d_spcx_rif       = 0x179; TBD put hte replacement here... Danny

#endif

#if (DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39)
  // Initialize V42b variables
  l1s_dsp_com.dsp_ndb_ptr->d_v42b_nego0       = 0;
  l1s_dsp_com.dsp_ndb_ptr->d_v42b_nego1       = 0;
  l1s_dsp_com.dsp_ndb_ptr->d_v42b_control     = 0;
  l1s_dsp_com.dsp_ndb_ptr->d_v42b_ratio_ind   = 0;
  l1s_dsp_com.dsp_ndb_ptr->d_mcu_control      = 0;
  l1s_dsp_com.dsp_ndb_ptr->d_mcu_control_sema = 0;

  #if !(W_A_DSP_SR_BGD)
    // Initialize background control variable to No background. Background tasks can be launch in GPRS
    // as in GSM.
    l1s_dsp_com.dsp_ndb_ptr->d_max_background    = 0;
  #endif

  #if (L1_GPRS)
    #if (DSP == 36) || (DSP == 37)
      // Initialize GEA module
      l1ps_dsp_com.pdsp_ndb_ptr->d_gea_mode = 0;
    #endif
  #endif

#else
      #if (L1_GPRS)
        // Initialize background control variable to No background
        l1ps_dsp_com.pdsp_ndb_ptr->d_max_background    = 0;
      #endif
#endif

    #if (L1_GPRS)
      l1ps_dsp_com.pdsp_ndb_ptr->d_sched_mode_gprs   = GSM_SCHEDULER;

      // Initialize the poll response buffer to "no poll request"
      l1ps_dsp_com.pdsp_ndb_ptr->a_pu_gprs[0][0]     = CS_NONE_TYPE;
    #else // L1_GPRS
      #if ((DSP == 31) || (DSP == 32) || (DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39))
        l1s_dsp_com.dsp_ndb_ptr->d_sched_mode_gprs_ovly = GSM_SCHEDULER;
      #endif
    #endif // L1_GPRS

    // Set EOTD bit if required
    //=============================================
    #if (L1_EOTD ==1)
       l1s_dsp_com.dsp_ndb_ptr->d_tch_mode |= B_EOTD;
    #endif // L1_EOTD

    #if (DSP == 33)
      #if DCO_ALGO
        // Set DCO bit
        if (l1_config.params.dco_enabled == TRUE)
          l1s_dsp_com.dsp_ndb_ptr->d_tch_mode |= B_DCO_ON;
      #endif
    #endif

    // DCO algo in case of DSP 17/32
    #if (DCO_ALGO == 1)
      #if ((DSP == 17)||(DSP == 32))
        l1s_dsp_com.dsp_ndb_ptr->d_tch_mode |= B_DCO_ON;
      #endif // DSP
    #endif // DCO_ALGO

    #if ((DSP == 34) || (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38)) || (DSP == 39)
      l1s_dsp_com.dsp_ndb_ptr->a_amr_config[0] = 0;
      l1s_dsp_com.dsp_ndb_ptr->a_amr_config[1] = 0;
      l1s_dsp_com.dsp_ndb_ptr->a_amr_config[2] = 0;
      l1s_dsp_com.dsp_ndb_ptr->a_amr_config[3] = 0;
    #endif

    #if (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39)
      l1s_dsp_com.dsp_ndb_ptr->d_thr_onset_afs      = 400; // thresh detection ONSET AFS
      l1s_dsp_com.dsp_ndb_ptr->d_thr_sid_first_afs  = 150; // thresh detection SID_FIRST AFS
      l1s_dsp_com.dsp_ndb_ptr->d_thr_ratscch_afs    = 450; // thresh detection RATSCCH AFS
      l1s_dsp_com.dsp_ndb_ptr->d_thr_update_afs     = 300; // thresh detection SID_UPDATE AFS
      l1s_dsp_com.dsp_ndb_ptr->d_thr_onset_ahs      = 200; // thresh detection ONSET AHS
      l1s_dsp_com.dsp_ndb_ptr->d_thr_sid_ahs        = 150; // thresh detection SID frames AHS
      l1s_dsp_com.dsp_ndb_ptr->d_thr_ratscch_marker = 500; // thresh detection RATSCCH MARKER
      l1s_dsp_com.dsp_ndb_ptr->d_thr_sp_dgr    = 3; // thresh detection SPEECH DEGRADED/NO_DATA
      l1s_dsp_com.dsp_ndb_ptr->d_thr_soft_bits    = 0; // thresh detection SPEECH DEGRADED/NO_DATA
    #endif

    #if ((DSP==36 || (DSP == 37) || (DSP == 38) || (DSP == 39))&&(W_A_AMR_THRESHOLDS==1))
      // init of the afs thresholds parameters
      l1s_dsp_com.dsp_ndb_ptr->a_d_macc_thr_afs[0]=0;
      l1s_dsp_com.dsp_ndb_ptr->a_d_macc_thr_afs[1]=0;
      l1s_dsp_com.dsp_ndb_ptr->a_d_macc_thr_afs[2]=0;
      l1s_dsp_com.dsp_ndb_ptr->a_d_macc_thr_afs[3]=0;
      l1s_dsp_com.dsp_ndb_ptr->a_d_macc_thr_afs[4]=0;
      l1s_dsp_com.dsp_ndb_ptr->a_d_macc_thr_afs[5]=0;
      l1s_dsp_com.dsp_ndb_ptr->a_d_macc_thr_afs[6]=0;
      l1s_dsp_com.dsp_ndb_ptr->a_d_macc_thr_afs[7]=1950;

      // init of the ahs thresholds parameters
      l1s_dsp_com.dsp_ndb_ptr->a_d_macc_thr_ahs[0]=1500;
      l1s_dsp_com.dsp_ndb_ptr->a_d_macc_thr_ahs[1]=1500;
      l1s_dsp_com.dsp_ndb_ptr->a_d_macc_thr_ahs[2]=1500;
      l1s_dsp_com.dsp_ndb_ptr->a_d_macc_thr_ahs[3]=1500;
      l1s_dsp_com.dsp_ndb_ptr->a_d_macc_thr_ahs[4]=1500;
      l1s_dsp_com.dsp_ndb_ptr->a_d_macc_thr_ahs[5]=1500;
    #endif

    // init of of the threshold for USF detection
    #if (L1_FALSE_USF_DETECTION == 1)
      l1s_dsp_com.dsp_ndb_ptr->d_thr_usf_detect = 2300;
    #else
      l1s_dsp_com.dsp_ndb_ptr->d_thr_usf_detect = 0;
    #endif

    #if (CHIPSET == 12) || (CHIPSET == 15)
      #if (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39)
        l1s_dsp_com.dsp_ndb_ptr->d_cport_init  = 0;
      #endif
    #endif

    #if ((CHIPSET == 15) || (CHIPSET == 12) || (CHIPSET == 4) || ((CHIPSET == 10) && (OP_WCP == 1))) // Calypso+ or Perseus2 or locosto
      #if (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39)
        // Note: for locosto there is only one MCSI port
        l1s_dsp_com.dsp_ndb_ptr->d_mcsi_select = MCSI_PORT1;
      #endif

      #if(DSP == 36) || (DSP == 37)
         l1s_dsp_com.dsp_ndb_ptr->d_vol_ul_level   = 0x1000;
         l1s_dsp_com.dsp_ndb_ptr->d_vol_dl_level   = 0x1000;
         l1s_dsp_com.dsp_ndb_ptr->d_vol_speed      = 0x68;
         l1s_dsp_com.dsp_ndb_ptr->d_sidetone_level = 0;
      #endif
    #endif // ((CHIPSET == 15) || (CHIPSET == 12) || (CHIPSET == 4) || ((CHIPSET == 10) && (OP_WCP == 1)))

    // DB Init DB : Reset all pages, set TX power and reset SCH buffer...
    //====================================================
    l1s_reset_db_mcu_to_dsp((T_DB_MCU_TO_DSP *) DB_W_PAGE_0);
    l1s_reset_db_mcu_to_dsp((T_DB_MCU_TO_DSP *) DB_W_PAGE_1);
    l1s_reset_db_dsp_to_mcu((T_DB_DSP_TO_MCU *) DB_R_PAGE_0);
    l1s_reset_db_dsp_to_mcu((T_DB_DSP_TO_MCU *) DB_R_PAGE_1);
    #if (DSP == 38) || (DSP == 39)
      l1s_reset_db_common_mcu_to_dsp((T_DB_COMMON_MCU_TO_DSP *) DB_COMMON_W_PAGE_0);
      l1s_reset_db_common_mcu_to_dsp((T_DB_COMMON_MCU_TO_DSP *) DB_COMMON_W_PAGE_1);
    #endif

#endif // NO_SIMULATION

  #if ((DSP==17)||(DSP == 32))
    // init the DC offset values
    l1s_dsp_com.dsp_ndb_ptr->d_dco_type    = 0x0000;                       // Tide off
    l1s_dsp_com.dsp_ndb_ptr->p_start_IQ    = 0x0000;
    l1s_dsp_com.dsp_ndb_ptr->d_level_off   = 0x0000;
    l1s_dsp_com.dsp_ndb_ptr->d_dco_dbg     = 0x0000;
    l1s_dsp_com.dsp_ndb_ptr->d_tide_resa   = 0x0000;
  #endif

  //Initialize DSP DCO
  #if (((DSP == 38) || (DSP == 39)) && (RF_FAM == 61))
    l1s_dsp_com.dsp_ndb_ptr->d_dco_samples_per_symbol = C_DCO_SAMPLES_PER_SYMBOL;
    l1s_dsp_com.dsp_ndb_ptr->d_dco_fcw = C_DCO_FCW;

    // APCDEL1 will be initialized on rach only ....
    l1s_dsp_com.dsp_ndb_ptr->d_apcdel1     = l1_config.params.apcdel1;
    l1s_dsp_com.dsp_ndb_ptr->d_apcdel2     = l1_config.params.apcdel2;
    // APCCTRL2 alone initialize on the next TDMA frame possible
    l1ddsp_apc_load_apcctrl2(l1_config.params.apcctrl2);

    l1dapc_init_ramp_tables();

    #if ((FF_REPEATED_SACCH == 1) || (FF_REPEATED_DL_FACCH == 1 ))

		    /* Chase combining feature flag Initialise */
		   l1s_dsp_com.dsp_ndb_ptr->d_chase_comb_ctrl |= 0x0001;
    #endif /* FF_REPEATED_SACCH or FF_REPEATED_DL_FACCH */

  #endif // DSP == 38

  // Intialize the AFC
  #if (DSP == 38) || (DSP == 39)
    #if (CODE_VERSION != SIMULATION)
      l1s_dsp_com.dsp_ndb_ptr->d_drp_afc_add_api = C_DRP_DCXO_XTAL_DSP_ADDRESS;
    #endif

    #if (L1_DRP_IQ_SCALING == 1)
      l1s_dsp_com.dsp_ndb_ptr->d_dsp_iq_scaling_factor = 1;
    #else
      l1s_dsp_com.dsp_ndb_ptr->d_dsp_iq_scaling_factor = 0;
    #endif
  #endif

}

/*-------------------------------------------------------*/
/* l1_tpu_init()                                         */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1_tpu_init(void)
{
   #if (CODE_VERSION == SIMULATION)
    // L1S -> TPU communication...
    //=============================
    l1s_tpu_com.tpu_w_page      = 0;
    l1s_tpu_com.tpu_page_ptr    = &(tpu.buf[l1s_tpu_com.tpu_w_page].line[0]);
    l1s_tpu_com.reg_cmd         = (T_reg_cmd*) &(hw.reg_cmd);
    l1s_tpu_com.reg_com_int     = &(hw.reg_com_int);
    l1s_tpu_com.offset          = &(hw.offset);

    // Reset TPU.
    //=============================
    *(l1s_tpu_com.offset)              = 0;
    *(l1s_tpu_com.reg_com_int)         = 0;
    l1s_tpu_com.reg_cmd->tpu_idle_bit  = OFF;
    l1s_tpu_com.reg_cmd->tpu_enb_bit   = OFF;
    l1s_tpu_com.reg_cmd->tpu_stat_bit  = OFF;
    l1s_tpu_com.reg_cmd->tpu_reset_bit = OFF;
    l1s_tpu_com.reg_cmd->tpu_pag_bit   = 0;

    // Init. OFFSET and SYNC registers
    //================================
    l1s_tpu_com.reg_cmd->tpu_reset_bit = ON;  // bit TPU_RESET active
    l1dmacro_synchro(IMM, 0);                 // OFFSET=SYNCHRO=0 without any AT
    l1dtpu_end_scenario();                    // Close TPU scenario

   #else
    // bit TPU_RESET set
    // OFFSET and SYNCHRO initialized at 0
    // TSP_ACT bits reset
    // Sleep added and TPU_ENABLE set...
    l1dmacro_init_hw();

    l1s_tpu_com.reg_cmd = (UWORD16 *) TPU_CTRL;
   #endif
}

void l1_tpu_init_light(void)
{
   #if (CODE_VERSION == SIMULATION)
    // L1S -> TPU communication...
    //=============================
    l1s_tpu_com.tpu_w_page      = 0;
    l1s_tpu_com.tpu_page_ptr    = &(tpu.buf[l1s_tpu_com.tpu_w_page].line[0]);
    l1s_tpu_com.reg_cmd         = (T_reg_cmd*) &(hw.reg_cmd);
    l1s_tpu_com.reg_com_int     = &(hw.reg_com_int);
    l1s_tpu_com.offset          = &(hw.offset);

    // Reset TPU.
    //=============================
    *(l1s_tpu_com.offset)              = 0;
    *(l1s_tpu_com.reg_com_int)         = 0;
    l1s_tpu_com.reg_cmd->tpu_idle_bit  = OFF;
    l1s_tpu_com.reg_cmd->tpu_enb_bit   = OFF;
    l1s_tpu_com.reg_cmd->tpu_stat_bit  = OFF;
    l1s_tpu_com.reg_cmd->tpu_reset_bit = OFF;
    l1s_tpu_com.reg_cmd->tpu_pag_bit   = 0;

    // Init. OFFSET and SYNC registers
    //================================
    l1s_tpu_com.reg_cmd->tpu_reset_bit = ON;  // bit TPU_RESET active
    l1dmacro_synchro(IMM, 0);                 // OFFSET=SYNCHRO=0 without any AT
    l1dtpu_end_scenario();                    // Close TPU scenario

   #else
    // bit TPU_RESET set
    // OFFSET and SYNCHRO initialized at 0
    // TSP_ACT bits reset
    // Sleep added and TPU_ENABLE set...
    l1dmacro_init_hw_light();

    l1s_tpu_com.reg_cmd = (UWORD16 *) TPU_CTRL;
   #endif
}

/*-------------------------------------------------------*/
/* l1_abb_power_on()                                     */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/* Initialize the global structure for spi communication */
/* with ABB.                                             */
/* Set up ABB connection (CLK 13M free)                  */
/* Aknowledge the ABB status register                    */
/* Configure ABB modules                                 */
/* Program the ramp parameters into the NDB              */
/* Load in the NDB registers' value to be programmed in  */
/* ABB at first communication it                         */
/*-------------------------------------------------------*/

//Locosto This funciton would change drastically  due to Triton introduction and instead of SPI we have i2c
void l1_abb_power_on(void)
{
  #if (CODE_VERSION != SIMULATION)
      #if (CHIPSET != 15)
    T_SPI_DEV *Abb;
    T_SPI_DEV init_spi_device;
    UWORD16 Abb_Status;
    T_NDB_MCU_DSP * dsp_ndb_ptr;

    Abb               = &init_spi_device;   /* Pointer initialization to device communication structure */
    Abb->PrescVal     = SPI_CLOCK_DIV_1;    /* ABB transmission parameters initialization */
    Abb->DataTrLength = SPI_WNB_15;
    Abb->DevAddLength = 5;
    Abb->DevId        = ABB;
    Abb->ClkEdge      = SPI_CLK_EDG_RISE;
    Abb->TspEnLevel   = SPI_NTSPEN_NEG_LEV;
    Abb->TspEnForm    = SPI_NTSPEN_LEV_TRIG;

    SPI_InitDev(Abb);                   /* Initialize the spi to work with ABB */

    ABB_free_13M();                     /* Set up Abb connection (CLK 13M free).*/
    Abb_Status = ABB_Read_Status();     /* Aknowledge the Abb status register.  */

    /*------------------------------------------------------------------*/
    /* Add here SW to manage Abb VRPCSTS status register informations   */
    /*------------------------------------------------------------------*/

    ABB_Read_Register_on_page(PAGE0,ITSTATREG);  /* Aknowledge the interrupt status register */
                                        /* to clear any pending interrupt */

    ABB_on(AFC | MADC, l1a_l1s_com.recovery_flag);

    // ADC init: Configuration of the channels to be converted and enable the ADC Interrupt
    ABB_Conf_ADC(ALL,EOC_INTENA);

    //in case of reset due to a recovery process do not create the HISR
    if (l1a_l1s_com.recovery_flag == FALSE)
    {
      Create_ABB_HISR();
    }

    // Load RAMP up/down in NDB memory...
    dsp_ndb_ptr = (T_NDB_MCU_DSP *) NDB_ADR;

    if (l1_config.tx_pwr_code == 0)
    {
      Cust_get_ramp_tab(dsp_ndb_ptr->a_ramp,
                        0 /* not used */,
                        0 /* not used */,
                        1 /* arbitrary value for arfcn*/);
    }
    else
    {
      Cust_get_ramp_tab(dsp_ndb_ptr->a_ramp,
                        5 /* arbitrary value working in any case */,
                        5 /* arbitrary value working in any case */,
                        1 /* arbitrary value for arfcn*/);
    }
  #endif


    #if (ANALOG == 1)
      // Omega registers values will be programmed at 1st DSP communication interrupt

      dsp_ndb_ptr->d_debug1      = l1_config.params.debug1;      // Enable f_tx delay of 400000 cyc DEBUG
      dsp_ndb_ptr->d_afcctladd   = l1_config.params.afcctladd;   // Value at reset
      dsp_ndb_ptr->d_vbuctrl     = l1_config.params.vbuctrl;     // Uplink gain amp 0dB, Sidetone gain to mute
      dsp_ndb_ptr->d_vbdctrl     = l1_config.params.vbdctrl;     // Downlink gain amp 0dB, Volume control 0 dB
      dsp_ndb_ptr->d_bbctrl      = l1_config.params.bbctrl;      // value at reset
      dsp_ndb_ptr->d_apcoff      = l1_config.params.apcoff;      // value at reset
      dsp_ndb_ptr->d_bulioff     = l1_config.params.bulioff;     // value at reset
      dsp_ndb_ptr->d_bulqoff     = l1_config.params.bulqoff;     // value at reset
      dsp_ndb_ptr->d_dai_onoff   = l1_config.params.dai_onoff;   // value at reset
      dsp_ndb_ptr->d_auxdac      = l1_config.params.auxdac;      // value at reset
      dsp_ndb_ptr->d_vbctrl      = l1_config.params.vbctrl;      // VULSWITCH=0, VDLAUX=1, VDLEAR=1.

      // APCDEL1 will be initialized on rach only ....
      dsp_ndb_ptr->d_apcdel1 =l1_config.params.apcdel1;

      #if (DSP >= 33)
        // To increase the robustness the IOTA register are reseted to 0
        // if OMEGA, NAUSICA is used
        dsp_ndb_ptr->d_bulgcal     = 0x0000;
        dsp_ndb_ptr->d_vbctrl2     = 0x0000;
        dsp_ndb_ptr->d_apcdel2     = 0x0000;
      #endif
    #endif
    #if (ANALOG == 2)
      // Iota registers values will be programmed at 1st DSP communication interrupt

      dsp_ndb_ptr->d_debug1      = l1_config.params.debug1;      // Enable f_tx delay of 400000 cyc DEBUG
      dsp_ndb_ptr->d_afcctladd   = l1_config.params.afcctladd;   // Value at reset
      dsp_ndb_ptr->d_vbuctrl     = l1_config.params.vbuctrl;     // Uplink gain amp 0dB, Sidetone gain to mute
      dsp_ndb_ptr->d_vbdctrl     = l1_config.params.vbdctrl;     // Downlink gain amp 0dB, Volume control 0 dB
      dsp_ndb_ptr->d_bbctrl      = l1_config.params.bbctrl;      // value at reset
      dsp_ndb_ptr->d_bulgcal     = l1_config.params.bulgcal;     // value at reset
      dsp_ndb_ptr->d_apcoff      = l1_config.params.apcoff;      // value at reset
      dsp_ndb_ptr->d_bulioff     = l1_config.params.bulioff;     // value at reset
      dsp_ndb_ptr->d_bulqoff     = l1_config.params.bulqoff;     // value at reset
      dsp_ndb_ptr->d_dai_onoff   = l1_config.params.dai_onoff;   // value at reset
      dsp_ndb_ptr->d_auxdac      = l1_config.params.auxdac;      // value at reset
      dsp_ndb_ptr->d_vbctrl1     = l1_config.params.vbctrl1;     // VULSWITCH=0, VDLAUX=1, VDLEAR=1.
      dsp_ndb_ptr->d_vbctrl2     = l1_config.params.vbctrl2;     // MICBIASEL=0, VDLHSO=0, MICAUX=0

      // APCDEL1 will be initialized on rach only ....
      dsp_ndb_ptr->d_apcdel1     =l1_config.params.apcdel1;
      dsp_ndb_ptr->d_apcdel2     = l1_config.params.apcdel2;
    #endif
    #if (ANALOG == 3)
      // Syren registers values will be programmed at 1st DSP communication interrupt

      dsp_ndb_ptr->d_debug1      = l1_config.params.debug1;      // Enable f_tx delay of 400000 cyc DEBUG
      dsp_ndb_ptr->d_afcctladd   = l1_config.params.afcctladd;   // Value at reset
      dsp_ndb_ptr->d_vbuctrl     = l1_config.params.vbuctrl;     // Uplink gain amp 0dB, Sidetone gain to mute
      dsp_ndb_ptr->d_vbdctrl     = l1_config.params.vbdctrl;     // Downlink gain amp 0dB, Volume control 0 dB
      dsp_ndb_ptr->d_bbctrl      = l1_config.params.bbctrl;      // value at reset
      dsp_ndb_ptr->d_bulgcal     = l1_config.params.bulgcal;     // value at reset
      dsp_ndb_ptr->d_apcoff      = l1_config.params.apcoff;      // value at reset
      dsp_ndb_ptr->d_bulioff     = l1_config.params.bulioff;     // value at reset
      dsp_ndb_ptr->d_bulqoff     = l1_config.params.bulqoff;     // value at reset
      dsp_ndb_ptr->d_dai_onoff   = l1_config.params.dai_onoff;   // value at reset
      dsp_ndb_ptr->d_auxdac      = l1_config.params.auxdac;      // value at reset
      dsp_ndb_ptr->d_vbctrl1     = l1_config.params.vbctrl1;     // VULSWITCH=0
      dsp_ndb_ptr->d_vbctrl2     = l1_config.params.vbctrl2;     // MICBIASEL=0, VDLHSO=0, MICAUX=0

      // APCDEL1 will be initialized on rach only ....
      dsp_ndb_ptr->d_apcdel1     = l1_config.params.apcdel1;
      dsp_ndb_ptr->d_apcdel2     = l1_config.params.apcdel2;

      // Additional registers management brought by SYREN
      dsp_ndb_ptr->d_vbpop          = l1_config.params.vbpop;          // HSOAUTO enabled only
      dsp_ndb_ptr->d_vau_delay_init = l1_config.params.vau_delay_init; // vaud_init_delay init 2 frames
      dsp_ndb_ptr->d_vaud_cfg       = l1_config.params.vaud_cfg;       // Init to zero
      dsp_ndb_ptr->d_vauo_onoff     = l1_config.params.vauo_onoff;     // Init to zero
#if ((L1_AUDIO_MCU_ONOFF == 1)&&(OP_L1_STANDALONE == 1)&&(CHIPSET == 12))
      ABB_Write_Register_on_page(PAGE1, VAUOCTRL, 0x0015A);
#endif // E Sample testing of audio on off
      dsp_ndb_ptr->d_vaus_vol       = l1_config.params.vaus_vol;       // Init to zero
      dsp_ndb_ptr->d_vaud_pll       = l1_config.params.vaud_pll;       // Init to zero
      dsp_ndb_ptr->d_togbr2         = 0; // TOGBR2 initial value handled by the DSP (this value doesn't nake any sense)

    #endif

    #if (ANALOG == 11)
// The following settings need to be done only in L1 StandALoen as PSP would
// do in the case of full PS Build...

        //Set the CTRL3 register
	BspTwl3029_I2c_WriteSingle(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_CTRL3_OFFSET,
	           l1_config.params.ctrl3,NULL);

#if (OP_L1_STANDALONE == 1)
	// THESE REGISTERS ARE INITIALIZED IN STANDALONE AND PS BUILDS FOR AUDIO PATH

	// ************ START REG INIT FOR PS build/STANDALONE *************
	BspTwl3029_I2c_WriteSingle(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_TOGB_OFFSET,
	           0x15,NULL);
	BspTwl3029_I2c_WriteSingle(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_VULGAIN_OFFSET,
	           l1_config.params.vulgain,NULL);
	//Set the VDLGAIN register
	BspTwl3029_I2c_WriteSingle(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_VDLGAIN_OFFSET,
	           l1_config.params.vdlgain,NULL);
	//Set the SIDETONE register
	BspTwl3029_I2c_WriteSingle(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_SIDETONE_OFFSET,
	           l1_config.params.sidetone,NULL);
	//Set the CTRL1 register
	BspTwl3029_I2c_WriteSingle(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_CTRL1_OFFSET,
	           l1_config.params.ctrl1,NULL);
	//Set the CTRL2 register
	BspTwl3029_I2c_WriteSingle(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_CTRL2_OFFSET,
	           l1_config.params.ctrl2,NULL);

	//Set the CTRL4 register
	BspTwl3029_I2c_WriteSingle(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_CTRL4_OFFSET,
	           l1_config.params.ctrl4,NULL);
	//Set the CTRL5 register
	BspTwl3029_I2c_WriteSingle(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_CTRL5_OFFSET,
	           l1_config.params.ctrl5,NULL);
	//Set the CTRL6 register
	BspTwl3029_I2c_WriteSingle(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_CTRL6_OFFSET,
	           l1_config.params.ctrl6,NULL);
	//Set the POPAUTO register
	BspTwl3029_I2c_WriteSingle(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_POPAUTO_OFFSET,
	           l1_config.params.popauto,NULL);

	// ************ END REG INIT FOR PS build/STANDALONE ****************



	BspTwl3029_I2c_WriteSingle(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_OUTEN1_OFFSET,
	           l1_config.params.outen1,NULL);
	//Set the OUTEN2 register
	BspTwl3029_I2c_WriteSingle(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_OUTEN2_OFFSET,
	           l1_config.params.outen2,NULL);
	//Set the OUTEN3 register
	BspTwl3029_I2c_WriteSingle(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_OUTEN3_OFFSET,
	           l1_config.params.outen3,NULL);



	//Set the AUDLGAIN register
	BspTwl3029_I2c_WriteSingle(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_AUDLGAIN_OFFSET,
	           l1_config.params.aulga,NULL);
	//Set the AUDRGAIN register
	BspTwl3029_I2c_WriteSingle(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_AUDRGAIN_OFFSET,
	           l1_config.params.aurga,NULL);
#endif


#if (OP_L1_STANDALONE == 1)
#if (L1_MADC_ON == 1)
	//MADC Real time initialization  for all the 11 ADCs
	bspTwl3029_Madc_enableRt( NULL, 0x7ff, l1a_madc_callback, &l1_madc_results);
#endif
#endif

    #endif
#endif //CODE_VERSION != SIMULATION
}

/*-------------------------------------------------------*/
/* l1_pwr_mgt_init()                                     */
/*-------------------------------------------------------*/
/* Parameters  :                                         */
/* -------------                                         */
/* Return      :                                         */
/* -------------                                         */
/* Description :                                         */
/* -------------                                         */
/* This routine is used to initialize the gauging        */
/* related variables.                                    */
/*-------------------------------------------------------*/
void l1_pwr_mgt_init(void)
{

  //++++++++++++++++++++++++++++++++++++++++++
  // Power management variables
  //++++++++++++++++++++++++++++++++++++++++++

  // flags for wake-up ....
  l1s.pw_mgr.Os_ticks_required = FALSE;
  l1s.pw_mgr.frame_adjust      = FALSE;
  l1s.pw_mgr.wakeup_time       = 0;

  // variables for sleep ....
  l1s.pw_mgr.sleep_duration    = 0;
  l1s.pw_mgr.sleep_performed   = DO_NOT_SLEEP;
  l1s.pw_mgr.modules_status    = 0;            // all clocks ON
  l1s.pw_mgr.paging_scheduled  = FALSE;

#if 0	/* removed in FreeCalypso */
  // variable for afc bypass mode
  l1s.pw_mgr.afc_bypass_mode   = AFC_BYPASS_MODE;
#endif

  // 32 Khz gauging ....
  l1s.pw_mgr.gaug_count        = 0;
  l1s.pw_mgr.enough_gaug       = FALSE;
        //Nina modify to save power, not forbid deep sleep, only force gauging in next paging
  l1s.force_gauging_next_paging_due_to_CCHR = 0;
  l1s.pw_mgr.gauging_task      = INACTIVE;

  // GAUGING duration
  #if (CHIPSET == 7)  || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12) || (CHIPSET == 15)
    if (l1_config.dpll <8 )
      l1s.pw_mgr.gaug_duration = 9;    //  9 frames  (no more CTRL with DSP)
    else // with a dpll >= 104Mhz the HF counter is too small: gauging limitation to 6 frames.
	  #if(CHIPSET == 15)
    // Gauging duration could be reduced to 4 frames (from 5 frames) as fast paging (FF_L1_FAST_DECODING) is available
		l1s.pw_mgr.gaug_duration = 4;    //  4 frames
	  #else
      l1s.pw_mgr.gaug_duration = 6;    //  6 frames
	  #endif
  #else
    l1s.pw_mgr.gaug_duration = 11;   // 1CTRL + 9 frames +1CTRL
  #endif


  //-------------------------------------------------
  // INIT state:
  // 32.768Khz is in the range [-500 ppm,+100 ppm]
  // due to temperature variation.
  // LF_100PPM = 32.7712768 Khz
  // LF_500PPM = 32.751616  Khz
  //
  // ACQUIS STATE :
  // 32.768Khz variations allowed from INIT value
  // are [-50 ppm,+50ppm]. Same delta on ideal 32khz
  // during 9 frames (gauging duration) represents 1348*T32.
  // LF_50PPM = 32.7696384 Khz
  // 1348/32.768 - 1348/32.7696384 = 0.002056632 ms
  // At 78 Mhz it means : 0.002056632ms/0.000012820513ms= 160 T
  //
  // UPDATE state :
  // allowed variations are [-6 ppm,+6ppm] jitter
  // LF_6PPM = 32.76819661 Khz
  // 1348/32.768 - 1348/32.76819661 = 0.00024691 ms
  // At 78 Mhz it means :  0.00024691 / 0.000012820513ms= 19 T
  //
  //                        78 Mhz    65 Mhz   84.5 Mhz
  //                        ===========================
  //  C_CLK_MIN             2380      1983     2578
  //  C_CLK_INIT_MIN        8721      29113    31293
  //  C_CLK_MAX             2381      1984     2580
  //  C_CLK_INIT_MAX        36823     41608    1662
  //  C_DELTA_HF_ACQUIS     160       130      173
  //  C_DELTA_HF_UPDATE     19        15       20
  //-------------------------------------------------
  #if ((CHIPSET == 2) || (CHIPSET == 3) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 9))
    l1s.pw_mgr.c_clk_min          = C_CLK_MIN;
    l1s.pw_mgr.c_clk_init_min     = C_CLK_INIT_MIN;
    l1s.pw_mgr.c_clk_max          = C_CLK_MAX;
    l1s.pw_mgr.c_clk_init_max     = C_CLK_INIT_MAX;
    l1s.pw_mgr.c_delta_hf_acquis  = C_DELTA_HF_ACQUIS;
    l1s.pw_mgr.c_delta_hf_update  = C_DELTA_HF_UPDATE;
  #elif ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12) || (CHIPSET == 15))
    // 78000/32.7712768 = 2380.13308
    l1s.pw_mgr.c_clk_min      = (UWORD32)((l1_config.dpll*MCUCLK)/LF_100PPM);
    // 0.13308*2^16
    l1s.pw_mgr.c_clk_init_min =(UWORD32) ((UWORD32)((UWORD32)(((UWORD32)(l1_config.dpll*MCUCLK))-
                                          (l1s.pw_mgr.c_clk_min*LF_100PPM))*
                                           65536)/LF_100PPM);  //omaps00090550

    // 78000/32.751616 = 2381.561875
    l1s.pw_mgr.c_clk_max      = (UWORD32)((l1_config.dpll*MCUCLK)/LF_500PPM); //omaps00090550
    // 0.561875*2^16
    l1s.pw_mgr.c_clk_init_max =(UWORD32)((UWORD32)(((double)(l1_config.dpll*MCUCLK)-
                                         (double)(l1s.pw_mgr.c_clk_max*LF_500PPM))*
                                         65536)/LF_500PPM);//omaps00090550

    // remember hf is expressed in nbr of clock in hz (ex 65Mhz,104Mhz)
    l1s.pw_mgr.c_delta_hf_acquis =(UWORD32) (((GAUG_IN_32T/LF)-(GAUG_IN_32T/LF_50PPM))*(l1_config.dpll*MCUCLK));//omaps00090550
    l1s.pw_mgr.c_delta_hf_update =(UWORD32)( ((GAUG_IN_32T/LF)-(GAUG_IN_32T/LF_6PPM ))*(l1_config.dpll*MCUCLK));//omaps00090550
  #endif

} /* l1_pwr_mgt_init() */

/*-------------------------------------------------------*/
/* l1_initialize_var()                                   */
/*-------------------------------------------------------*/
/* Parameters  :                                         */
/* -------------                                         */
/* Return      :                                         */
/* -------------                                         */
/* Description :                                         */
/* -------------                                         */
/* This routine is used to initialize the l1a, l1s and   */
/* l1a_l1s_com global structures.                        */
/*-------------------------------------------------------*/
void l1_initialize_var(void)
{
  UWORD32 i;
  UWORD8  task_id;

  //++++++++++++++++++++++++++++++++++++++++++
  // Power management variables
  //++++++++++++++++++++++++++++++++++++++++++
  l1_pwr_mgt_init();

  //++++++++++++++++++++++++++++++++++++++++++
  //  Reset "l1s" structure.
  //++++++++++++++++++++++++++++++++++++++++++

  // time counter used for debug and by L3 scenario...
  l1s.debug_time = 0;

  // L1S tasks management...
  //-----------------------------------------
  for(task_id=0; task_id<NBR_DL_L1S_TASKS; task_id++)
  {
    if (!((task_id == ADC_CSMODE0) && (l1a_l1s_com.recovery_flag != FALSE)))
    {
      l1s.task_status[task_id].new_status     = NOT_PENDING;
      l1s.task_status[task_id].current_status = INACTIVE;
    }
  }
  l1s.frame_count = 0;
  l1s.forbid_meas = 0;
#if L1_GPRS
  l1s.tcr_prog_done=0;
#endif
#if (AUDIO_DEBUG == 1)
  audio_reg_read_status=0;
#endif
  // MFTAB management variables...
  //-----------------------------------------
  l1s.afrm = 0;
  l1s_clear_mftab(l1s.mftab.frmlst);

  // Controle parameters... (miscellaneous)
  //-----------------------------------------
#if (RF_FAM != 61)
  l1s.afc             = ((WORD16)l1_config.params.eeprom_afc>>3); //F13.3 -> F16.0
 #endif
 #if (RF_FAM == 61)
  l1s.afc             = ((WORD16)l1_config.params.eeprom_afc>>2); //F13.3 -> F14.0
 #endif


  l1s.afc_frame_count = 0;

  #if (TOA_ALGO == 2)
    l1s.toa_var.toa_shift       = ISH_INVALID;
    l1s.toa_var.toa_snr_mask    = 0;
    l1s.toa_var.toa_frames_counter   = 0;
    l1s.toa_var.toa_accumul_counter  = 0;
    l1s.toa_var.toa_accumul_value    = 0;
    l1s.toa_var.toa_update_fn        = 0;
    l1s.toa_var.toa_update_flag      = FALSE;
  #else
  l1s.toa_shift       = ISH_INVALID;
  l1s.toa_snr_mask    = 0;
  #if L1_GPRS
    l1s.toa_period_count = 0;
    l1s.toa_update       = FALSE;
  #endif
  #endif

#if (L1_GPRS == 1)
   l1s.algo_change_synchro_active = FALSE;
#endif

#if (L1_RF_KBD_FIX == 1)
l1s.total_kbd_on_time = 5000;
l1s.correction_ratio = 1;
#endif
/* Initialising the repeated SACCH variables */
#if (FF_REPEATED_SACCH == 1 )
     l1s.repeated_sacch.srr = 0;/* SACCH Repetiton Request */
     l1s.repeated_sacch.sro = 0;/* SACCH Repetiton Order   */
     l1s.repeated_sacch.buffer_empty = TRUE;
#endif  /* FF_REPEATED_SACCH ==1*/

#if (FF_REPEATED_DL_FACCH == 1)
        l1s.repeated_facch.pipeline[0].buffer_empty=l1s.repeated_facch.pipeline[1].buffer_empty=TRUE;
        l1s.repeated_facch.counter_candidate=0;
        l1s.repeated_facch.counter=1;
#endif/* (FF_REPEATED_DL_FACCH == 1) */

  // Init the spurious_fb_detected flag
  l1s.spurious_fb_detected = FALSE;

  // Flag registers for RF task controle...
  //-----------------------------------------
  l1s.tpu_ctrl_reg = 0;
  l1s.dsp_ctrl_reg = 0;

  // Serving...
  //============

  // Serving frame number management.
  //---------------------------------
  if (l1a_l1s_com.recovery_flag == FALSE)
  {
    l1s.actual_time.tc           = 0;
    l1s.actual_time.fn           = 0;
    l1s.actual_time.t1           = 0;
    l1s.actual_time.t2           = 0;
    l1s.actual_time.t3           = 0;
    l1s.actual_time.fn_in_report = 0;
    l1s.actual_time.fn_mod42432  = 0;

    l1s.next_time.tc           = 0;
    l1s.next_time.fn           = 0;
    l1s.next_time.t1           = 0;
    l1s.next_time.t2           = 0;
    l1s.next_time.t3           = 0;
    l1s.next_time.fn_in_report = 0;
    l1s.next_time.fn_mod42432  = 0;

    #if L1_GPRS
      l1s.actual_time.block_id = 0;
      l1s.next_time.block_id   = 0;
      l1s.next_plus_time = l1s.next_time;
      l1s_increment_time(&(l1s.next_plus_time),1);
      l1s.ctrl_synch_before = FALSE;
      l1s.next_gauging_scheduled_for_PNP= 0;
    #endif
  }

  // TXPWR management.
  //-------------------
  l1s.reported_txpwr = 0;
  l1s.applied_txpwr  = 0;

  // Last RXQUAL value.
  //-------------------
  l1s.rxqual = 0;

  // Hardware info.
  //---------------
  l1s.tpu_offset = 0;
  l1s.tpu_offset_hw = 0;

  l1s.tpu_win    = 0;

  // Initialize TXPWR info.
  l1s.last_used_txpwr = NO_TXPWR;

#if (AMR == 1)
  // Reset DTX AMR status
  //---------------------
  l1s.dtx_amr_dl_on=FALSE;
#endif

  // Code version structure
  //-------------------------

  // DSP versions & checksum
  l1s.version.dsp_code_version  = 0;
  l1s.version.dsp_patch_version = 0;
  l1s.version.dsp_checksum      = 0;     // checksum patch+code DSP

  l1s.version.mcu_tcs_program_release = PROGRAM_RELEASE_VERSION;
  l1s.version.mcu_tcs_internal        = INTERNAL_VERSION;
  l1s.version.mcu_tcs_official        = MAINTENANCE_VERSION;

  #if TESTMODE
    l1s.version.mcu_tm_version    = TESTMODEVERSION;
  #else
    l1s.version.mcu_tm_version    = 0;
  #endif

  //++++++++++++++++++++++++++++++++++++++++++
  //  Reset "l1a" structure.
  //++++++++++++++++++++++++++++++++++++++++++

  // Downlink tasks management...
  // Uplink tasks management...
  // Measurement tasks management...
  //-----------------------------------------

  if (l1a_l1s_com.recovery_flag == FALSE)
  {
    for(i=0; i<NBR_L1A_PROCESSES; i++)
    {
      l1a.l1a_en_meas[i] = 0;
      l1a.state[i]       = 0; // RESET state.
    }
  }
  else
  {
    // L1A state for full list meas has to be maintained in case of recovery
    for(i=0; i<NBR_L1A_PROCESSES; i++)
    {
      if ((i != FULL_MEAS) && (i!= I_ADC))
      {
        l1a.l1a_en_meas[i] = 0;
        l1a.state[i]       = 0; // RESET state.
      }
    }
  }

  l1a.confirm_SignalCode = 0;

  // Flag for forward/delete message management.
  //---------------------------------------------
  if (l1a_l1s_com.recovery_flag == FALSE)
  {
    l1a.l1_msg_forwarded = 0;
  }

#if (L1_VOCODER_IF_CHANGE == 1)
  // Reset new vocoder interface L1A global variables: automatic disabling and vocoder enabling flag.
  l1a.vocoder_state.automatic_disable = FALSE;
  l1a.vocoder_state.enabled = FALSE;
#endif // if L1_VOCODER_IF_CHANGE == 1
  //++++++++++++++++++++++++++++++++++++++++++
  //  Reset "l1a_l1s_com" structure.
  //++++++++++++++++++++++++++++++++++++++++++

  l1a_l1s_com.l1a_activity_flag      = TRUE;
  l1a_l1s_com.time_to_next_l1s_task  = 0;

  // Serving Cell...
  //=================

  // Serving Cell identity and information.
  //---------------------------------------
  l1a_reset_cell_info(&(l1a_l1s_com.Scell_info));

  l1a_l1s_com.Smeas_dedic.acc_sub            = 0;
  l1a_l1s_com.Smeas_dedic.nbr_meas_sub       = 0;
  l1a_l1s_com.Smeas_dedic.qual_acc_full      = 0;
  l1a_l1s_com.Smeas_dedic.qual_acc_sub       = 0;
  l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full = 0;
  l1a_l1s_com.Smeas_dedic.qual_nbr_meas_sub  = 0;
  l1a_l1s_com.Smeas_dedic.dtx_used           = 0;

    #if REL99
    #if FF_EMR
      // Serving Cell identity EMR information.
      //---------------------------------------
      l1a_l1s_com.Smeas_dedic_emr.rxlev_val_acc      = 0;
      l1a_l1s_com.Smeas_dedic_emr.rxlev_val_nbr_meas = 0;
      l1a_l1s_com.Smeas_dedic_emr.nbr_rcvd_blocks    = 0;
      l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_acc = 0;
      l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_acc   = 0;
      l1a_l1s_com.Smeas_dedic_emr.mean_bep_block_num = 0;
      l1a_l1s_com.Smeas_dedic_emr.cv_bep_block_num   = 0;

    #endif
    #endif


  l1a_l1s_com.Scell_used_IL.input_level    = l1_config.params.il_min;
  l1a_l1s_com.Scell_used_IL_d.input_level  = l1_config.params.il_min;
  l1a_l1s_com.Scell_used_IL_dd.input_level = l1_config.params.il_min;

  l1a_l1s_com.Scell_used_IL.lna_off        = FALSE;
  l1a_l1s_com.Scell_used_IL_d.lna_off      = FALSE;
  l1a_l1s_com.Scell_used_IL_dd.lna_off     = FALSE;

  // Synchro information.
  //---------------------------------------
  l1a_l1s_com.tn_difference = 0;
  l1a_l1s_com.dl_tn         = 0;
  #if L1_FF_WA_OMAPS00099442
    l1a_l1s_com.change_tpu_offset_flag = FALSE;
  #endif

  #if L1_GPRS
    l1a_l1s_com.dsp_scheduler_mode = GSM_SCHEDULER;
  #endif

  // Idle parameters.
  //-----------------
  l1a_l1s_com.nbcchs.schedule_array_size=0;
  l1a_l1s_com.ebcchs.schedule_array_size=0;
  l1a_l1s_com.bcchn.current_list_size=0;
  l1a_l1s_com.nsync.current_list_size=0;

  #if (GSM_IDLE_RAM != 0)
    l1s.gsm_idle_ram_ctl.l1s_full_exec = TRUE;

      #if GSM_IDLE_RAM_DEBUG
        #if (CHIPSET == 10) && (OP_WCP == 1)
            l1s.gsm_idle_ram_ctl.TC_true_control=0;
        #endif // CHIPSET && OP_WCP
      #endif // GSM_IDLE_RAM_DEBUG
  #endif // GSM_IDLE_RAM

#if (L1_12NEIGH ==1)
  for (i=0;i<NBR_NEIGHBOURS+1;i++)
#else
  for (i=0;i<6;i++)
#endif
  {
    l1a_l1s_com.nsync.list[i].status=NSYNC_FREE;
  }
   for (i=0;i<6;i++)
  {
    l1a_l1s_com.bcchn.list[i].status=NSYNC_FREE;
  }

  // EOTD variables
#if (L1_EOTD==1)
  l1a_l1s_com.nsync.eotd_meas_session=FALSE;
  l1a_l1s_com.nsync.fn_sb_serv;
  l1a_l1s_com.nsync.ta_sb_serv;
#endif

  // CBCH parameters.
  // ----------------
  // nothing to reset.

  // Random Access information.
  // ----------------------------
  // nothing to reset.

  // ADC management
  //---------------
  if (l1a_l1s_com.recovery_flag == FALSE)
    l1a_l1s_com.adc_mode =  ADC_DISABLED;

  // TXPWR management.
  //-------------------
#if(L1_FF_MULTIBAND == 0)
  l1a_l1s_com.powerclass_band1 = 0;
  l1a_l1s_com.powerclass_band2 = 0;
#else
  for( i = 0; i< (NB_MAX_SUPPORTED_BANDS); i++)
  {
   l1a_l1s_com.powerclass[i] = 0;
  }
#endif

  // Dedicated parameters.
  //----------------------
  l1a_l1s_com.dedic_set.aset        = NULL;
  l1a_l1s_com.dedic_set.fset        = NULL;
  l1a_l1s_com.dedic_set.SignalCode  = 0;
  l1a_l1s_com.dedic_set.sync_tch    = 0;
  l1a_l1s_com.dedic_set.stop_tch    = 0;
  l1a_l1s_com.dedic_set.reset_facch = FALSE;
#if (FF_L1_TCH_VOCODER_CONTROL)
  l1a_l1s_com.dedic_set.reset_sacch = FALSE;
#if (L1_VOCODER_IF_CHANGE == 0)
  l1a_l1s_com.dedic_set.vocoder_on = TRUE;
  #if (W_A_DSP_PR20037 == 1)
    l1a_l1s_com.dedic_set.start_vocoder = TCH_VOCODER_ENABLE_REQ;
  #else // W_A_DSP_PR20037 == 0
    l1a_l1s_com.dedic_set.start_vocoder = FALSE;
  #endif // W_A_DSP_PR20037
#else // L1_VOCODER_IF_CHANGE
    l1a_l1s_com.dedic_set.vocoder_on = FALSE;
    l1a_l1s_com.dedic_set.start_vocoder = TCH_VOCODER_RESET_COMMAND;
#endif // L1_VOCODER_IF_CHANGE
#endif // FF_L1_TCH_VOCODER_CONTROL

  l1a_l1s_com.dedic_set.radio_freq       = 0;
  l1a_l1s_com.dedic_set.radio_freq_d     = 0;
  l1a_l1s_com.dedic_set.radio_freq_dd    = 0;
#if ((REL99 == 1) && (FF_BHO == 1))
  // blind handover params in dedic set
  // Initialize the handover type to default value that is Normal Handover.
  l1a_l1s_com.dedic_set.handover_type            = 0;
  l1a_l1s_com.dedic_set.long_rem_handover_type   = 0;
  l1a_l1s_com.dedic_set.bcch_carrier_of_nbr_cell = 0;
  l1a_l1s_com.dedic_set.fn_offset                = 0;
  l1a_l1s_com.dedic_set.time_alignment           = 0;
#endif

#if (L1_12NEIGH ==1)
  for (i=0;i<NBR_NEIGHBOURS+1;i++)
#else
  for (i=0;i<6;i++)
#endif
  {
    l1a_l1s_com.nsync.list[i].sb26_offset = 0;
  }

  l1a_l1s_com.dedic_set.pwrc        = 0;
  l1a_l1s_com.dedic_set.handover_fail_mode = FALSE;
  #if (AMR == 1)
    l1a_l1s_com.dedic_set.sync_amr = FALSE;
  #endif

  // Handover parameters.
  //---------------------
  // nothing to reset.

  // Neighbour Cells...
  //====================

  // FULL list.
  //-----------
  l1a_reset_full_list();

  // BA list.
  //---------
  l1a_reset_ba_list();
  l1a_l1s_com.ba_list.new_list_present = FALSE;

  #if L1_GPRS
    //  Packet measurement: Reset of the frequency list.
    //-------------------------------------------------
    l1pa_reset_cr_freq_list();
  #endif

  // L1S scheduler...
  //====================

  // L1S tasks management...
  //-----------------------------------------
  {
    UWORD8 mem;
    mem = l1a_l1s_com.l1s_en_task[ADC_CSMODE0];

    for(i=0; i<NBR_DL_L1S_TASKS; i++)
    {
      l1a_l1s_com.task_param[i]  = SEMAPHORE_RESET;
      l1a_l1s_com.l1s_en_task[i] = TASK_DISABLED;
    }

    // in case of recovery do not change the ADC initialization
    if (l1a_l1s_com.recovery_flag != FALSE)
      l1a_l1s_com.l1s_en_task[ADC_CSMODE0] = mem;
  }

  // Measurement tasks management...
  //-----------------------------------------
  l1a_l1s_com.meas_param  = 0;
  l1a_l1s_com.l1s_en_meas = 0;

  // L1 mode...
  //-----------------------------------------
  if (l1a_l1s_com.recovery_flag == FALSE) // do not restart from CS_MODE0 after a recovery
    l1a_l1s_com.mode = CS_MODE0;

  // Control algo variables.
  //-----------------------------------------
  l1a_l1s_com.fb_mode   = 0;
  l1a_l1s_com.toa_reset = FALSE;

#if(L1_FF_MULTIBAND == 0)
  for(i=0; i<=l1_config.std.nbmax_carrier; i++)
#else
  for(i=0; i<= NBMAX_CARRIER; i++)
#endif
  {
    l1a_l1s_com.last_input_level[i].input_level = l1_config.params.il_min;
    l1a_l1s_com.last_input_level[i].lna_off     = FALSE;
  }

  #if FF_L1_IT_DSP_DTX
    // Fast DTX variables.
    //-----------------------------------------
    // Clear DTX interrupt condition
    l1a_apihisr_com.dtx.pending              = FALSE;
    // Enable TX activity
    l1a_apihisr_com.dtx.tx_active            = TRUE;
    // No DTX status awaited
    l1a_apihisr_com.dtx.dtx_status           = DTX_AVAILABLE;
    // Fast DTX service latency timer
    l1a_apihisr_com.dtx.fast_dtx_ready_timer = 0;
    // Fast DTX service available
    l1a_apihisr_com.dtx.fast_dtx_ready       = FALSE;
  #endif
  #if L1_RECOVERY
    l1s.recovery.frame_count = 0;
  #endif

  #if (AUDIO_TASK == 1)
    l1audio_initialize_var();
  #endif

  #if (L1_GTT == 1)
    l1gtt_initialize_var();
  #endif

  #if (L1_MP3 == 1)
    l1mp3_initialize_var();
  #endif

  #if (L1_MIDI == 1)
    l1midi_initialize_var();
  #endif
//ADDED FOR AAC
  #if (L1_AAC == 1)
    l1aac_initialize_var();
  #endif
  #if (L1_DYN_DSP_DWNLD == 1)
    l1_dyn_dwnld_initialize_var();
  #endif
  #if (FF_L1_FAST_DECODING == 1)
    l1a_apihisr_com.fast_decoding.pending = FALSE;
    l1a_apihisr_com.fast_decoding.crc_error = FALSE;
    l1a_apihisr_com.fast_decoding.status = 0;
    l1a_apihisr_com.fast_decoding.deferred_control_req = FALSE;
    l1a_apihisr_com.fast_decoding.task = 0;
    l1a_apihisr_com.fast_decoding.burst_id = 0;
    l1a_apihisr_com.fast_decoding.contiguous_decoding = FALSE;
  #endif /* FF_L1_FAST_DECODING */

  
  #if(L1_CHECK_COMPATIBLE == 1)
      l1a.vcr_wait = FALSE;
      l1a.stop_req = FALSE;
      l1a.vcr_msg_param = TRUE;
      l1a.vch_auto_disable = FALSE;	  

  #endif


}


/*---------------------------------------------------------*/
/* l1_dpll_init_var()                                      */
/*---------------------------------------------------------*/
/* Parameters    : None                                    */
/* Return        : None                                    */
/* Functionality : Initialize L1 DPLL variable for gauging */
/*                 processing                              */
/*---------------------------------------------------------*/
void l1_dpll_init_var(void) {

  #if (CODE_VERSION != SIMULATION)
    // Init DPLL variable
    //===================
    #if (CHIPSET == 2 || CHIPSET == 3 || CHIPSET == 5 || CHIPSET == 6 || CHIPSET == 9)
      l1_config.dpll=PLL;
    #elif ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12) || (CHIPSET == 15))
    {
      UWORD16 dpll_div;
      UWORD16 dpll_mul;
      #if (CHIPSET == 12)
        // not required for Locosto: There is NO CNTL_CLK_DSP in Locosto
        double dsp_div = CLKM_GET_DSP_DIV_VALUE;
      #endif

      dpll_div=DPLL_READ_DPLL_DIV;
      dpll_mul=DPLL_READ_DPLL_MUL;

      #if (CHIPSET == 12)
        // Not required for locsto due to the reason mentioned above.
        l1_config.dpll= ((double)(dpll_mul)/(double)(dpll_div+1))/(double)(dsp_div);
      #else
        l1_config.dpll= (double)(dpll_mul)/(double)(dpll_div+1);
      #endif
     }
    #endif
  #endif

} /* l1_dpll_init_var() */

/*-------------------------------------------------------------*/
/* FUNCTION: l1_drp_wrapper_init */

/*-------------------------------------------------------------*/

void l1_drp_wrapper_init (void)
  {
  #if(RF_FAM == 61)
    l1ddsp_apc_load_apcctrl2(l1_config.params.apcctrl2);
  #endif

  }

/*-------------------------------------------------------------*/
/* FUNCTION: l1_drp_init */
/* Params: Void */
/*
   Functionality: This function does the following
                     1. Initialize Misc variables wrt DRP
                     2a Copy the RAMP Tables into the DSP MCU API
                     2b. Initialize other APIs wrt DCO
                     3. Download Reference Software
                     4. Call the function to : Start the REG_ON Script in the DRP
 */
/*-------------------------------------------------------------*/

#if (L1_DRP == 1)
#if (DRP_FW_EXT==1)
#pragma DATA_SECTION(l1_drp_int_mem, ".drp_ptr")
void * l1_drp_int_mem;
#pragma DATA_SECTION(l1_drp_ext_mem, ".drp_ptr")
void *l1_drp_ext_mem;
#endif
  void l1_drp_init()
  {
  //int i;- OMAPS90550-new
#if (DRP_FW_EXT==1)
  uint32 size_int=0;
  uint32 size_ext=0;
#endif
 #if (RF_FAM == 61)
   volatile UWORD16 *ptr_drp_init16;
   UWORD16 drp_maj_version;
   UWORD16 drp_min_version;

//Initialize the following SRM_API, REG related address drp_srm_data =  DRP_SRM_DATA_ADD,
//drp_regs =  DRP_REGS_BASE_ADD;,  drp_srm_api  =   DRP_SRM_API_ADD

     drp_api_addr_init();

#if (DRP_FW_EXT==1)     
   drp_maj_version = (drp_ref_sw_ver >> 8) & 0xFF;
   drp_min_version = (drp_ref_sw_ver & 0xFF);
#endif

   //Initialize the following variables... TBD Danny
   //SRM_CW = 0x00000040, IRQ_CNT= 0x00000040 , TX_PTR_START_END_ADDR = 0X00200025,
   //RX_PTR_START_END_ADDR = 0X0000001F , 0XFFFE0806= 16
   //The registers are 32 bit since its a RHEA peripheral has to be writtin in 16 bit writes
   // This is done by the DRP script download

   // The counter for # of DRP_DBB_RX_IRQs (in the wrapper) to be masked
    ptr_drp_init16 = (UWORD16 *) (DRP_DBB_RX_IRQ_MASK);
      (*ptr_drp_init16) = DRP_DBB_RX_IRQ_COUNT;

#endif //RF_FAM == 61
    l1s.boot_result=0;
#if (DRP_FW_EXT==1)
    if(!((drp_min_version >= L1_DRP_COMPAT_MINOR_VER) && (drp_maj_version == L1_DRP_COMPAT_MAJOR_VER))) {
      l1s.boot_result = 1;
      return;
    }  
    drp_get_memory_size(&size_int,&size_ext);
    /* FIXME  FIXME  ERROR handling for memory allocation failure */
    if(size_int)
    {
        l1_drp_int_mem=os_alloc_sig(size_int);
        if(l1_drp_int_mem==NULL)
        {  
            /*FIXME Error Handling Here */
            l1s.boot_result = 1;
            return;
        }
    }
    if(size_ext)
    {
        l1_drp_ext_mem=os_alloc_sig(size_ext);

        if(l1_drp_ext_mem==NULL)
        {  
            /*FIXME Error Handling Here */
            l1s.boot_result = 1;
            return;
        }
    }

    // Populate pointers
    if(drpfw_init(&modem_func_jump_table,&modem_var_jump_table))
    {
      // This condition should not be reached in phase 1 of DRP FW 
      // Extraction. DRP and L1 software should always be compatible
      l1s.boot_result = 1;
      return;
    }
    
    ((T_DRP_ENV_INT_BLK *)l1_drp_int_mem)->g_pcb_config = RF_BAND_SYSTEM_INDEX;  //OMAPS148175
    
#endif // DRP_FW_EXT==1    
    // This function would takes care of drp_ref_sw download till that is in place this would be a dummy function
// Testing PLD_WriteRegister(0x0440, 0x165c);
    #if (RF_FAM == 60)   // PLD board
      // for PLD board script downloading will happen through USP driver
      // load ref_sw_main
//      drp_ref_sw_upload(drp_ref_sw);
       drp_copy_ref_sw_to_drpsrm( (unsigned char *) drp_ref_sw);
    #elif (RF_FAM == 61) // Locosto based board
      // load ref_sw_main
     //      drp_ref_sw_upload(drp_ref_sw); // TBD replace with DRP Copy function...
    drp_copy_ref_sw_to_drpsrm( (unsigned char *) drp_ref_sw);
#endif

#if (L1_DRP_DITHERING == 1)
    (*(volatile UINT8 *)CONF_MUX_VIEW8) = 0x01;
    (*(volatile UINT8 *)CONF_DEBUG_SEL_TST_8) = 0x07;
    (*(volatile UINT8 *)CONF_GPIO_17) = 0x02;
    (*(volatile UINT8 *)CONF_LOCOSTO_DEBUG) = 0x00;
#endif

  }
#endif // L1_DRP

/*-------------------------------------------------------*/
/* l1_initialize()                                       */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1_initialize(T_MMI_L1_CONFIG *mmi_l1_config)
{
  #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5)
    l1_trace_init();
  #endif

  // this is not a recovery initialization .
  l1a_l1s_com.recovery_flag = FALSE;

  // initialize the ratio of the wait loop
  // must be initialized before using the wait_ARM_cycles() function !!!
  #if (CODE_VERSION != SIMULATION)
    initialize_wait_loop();
  #endif

  // Init Layer 1 configuration
  //===========================
#if(L1_FF_MULTIBAND  == 0)  
  l1_config.std.id            = mmi_l1_config->std;
#endif

  l1_config.tx_pwr_code       = mmi_l1_config->tx_pwr_code;
  #if IDS
    l1_config.ids_enable      = mmi_l1_config->ids_enable;
  #endif
  l1_config.facch_test.enable = mmi_l1_config->facch_test.enable;
  l1_config.facch_test.period = mmi_l1_config->facch_test.period;
  l1_config.dwnld             = mmi_l1_config->dwnld;

  #if TESTMODE
    // Initialize TestMode params: must be done after Omega power-on
    l1_config.TestMode = FALSE;
    // Enable control algos and ADC
    l1_config.agc_enable = 1;
    l1_config.afc_enable = 1;
    l1_config.adc_enable = 1;
  #if (FF_REPEATED_SACCH == 1)
  l1_config.repeat_sacch_enable = 1;  /* Repeated SACCH mode enabled */
  #endif /* (FF_REPEATED_SACCH == 1) */
 #if (FF_REPEATED_DL_FACCH == 1)
  l1_config.repeat_facch_dl_enable = 1;  /* Repeated SACCH mode enabled */
  #endif /* ( FF_REPEATED_DL_FACCH  == 1) */
  #endif

  // sleep management configuration
  //===============================
  l1s.pw_mgr.mode_authorized  = mmi_l1_config->pwr_mngt_mode_authorized;
  l1s.pw_mgr.clocks           = mmi_l1_config->pwr_mngt_clocks;
  l1_config.pwr_mngt          = mmi_l1_config->pwr_mngt;

  Cust_init_std();
  Cust_init_params();



  // Init DPLL variable
  //===================
  l1_dpll_init_var();

  // Reset hardware (DSP,Analog Baseband device , TPU) ....
  //========================================================
  #if (CODE_VERSION != SIMULATION)
    dsp_power_on();
    l1_abb_power_on();
    #if (L1_DRP == 1)
      l1_drp_init();
      //required for interworking with Isample 2.1 and Isample 2.5
#if (DRP_FW_EXT == 1)      
      if (!l1s.boot_result)
      {  
#endif      
     //for DRP Calibration
     Cust_init_params_drp();
     drp_efuse_init();
#if (DRP_FW_EXT == 1)      
      } /* end if boot_result != 0 */
#endif      

  #endif

  #endif

  // Initialize hardware....(DSP, TPU)....
  //=================================================
  l1_tpu_init();
  l1_dsp_init();

  // Initialize L1 variables (l1a, l1s, l1a_l1s_com).
  //=================================================
  l1_initialize_var();

  // API check function
  #if ((OP_L1_STANDALONE == 1) && ((DSP == 38) || (DSP == 39)) && (CODE_VERSION != SIMULATION))
     l1_api_dump();
  #endif

#if (L1_GPRS)
  // Initialize L1 variables used in packet mode (l1pa, l1ps, l1pa_l1ps_com).
  //========================================================================
  initialize_l1pvar();
#endif

  // Initialize statistics mode.......
  //=================================================
  #if TRACE_TYPE==3
    reset_stats();
  #endif
  #if(OP_L1_STANDALONE == 1 || L1_NAVC == 1 )//NAVC
    Cust_navc_ctrl_status(1);//start - NAVC
  #endif//end of (OP_L1_STANDALONE == 1 || L1_NAVC == 1 )

  #if FEATURE_TCH_REROUTE
    feature_tch_reroute_init();
  #endif
}

/*-------------------------------------------------------*/
/* l1_initialize_for_recovery                            */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality : This function is called for L1        */
/* recovery after a Crash. When there are 100 COM  error */
/* or if ther are 100 PM =0 from the DSP Successively.   */
/* The Layer 1 Crashes. The next time the Protocol stack */
/* requests for Full Rx Measurement (viz Cell selection) */
/* This function gets called and the L1 recovery is      */
/* initiated.                                            */
/*-------------------------------------------------------*/
#if L1_RECOVERY
  void l1_initialize_for_recovery(void)
  {
    LA_ResetLead(); // set DSP in reset mode
    initialize_wait_loop();

    dsp_power_on();    // the reset mode is disabled here
    l1_abb_power_on();
    #if (L1_DRP == 1)
      l1_drp_init();
      //Required for interworking with Isample 2.1 and Isample 2.5
      Cust_init_params_drp();
      drp_efuse_init();
    #endif
    l1_tpu_init();
    wait_ARM_cycles(convert_nanosec_to_cycles(11000000));  // wait of 5.5 msec
    l1_dsp_init();
    l1_initialize_var();

    #if L1_GPRS
      initialize_l1pvar();
    #endif

    l1a_l1s_com.recovery_flag = FALSE;

    // clear pending IQ_FRAME it and enable it
    #if (CHIPSET >= 4 )
      #if (CHIPSET == 12) || (CHIPSET == 15)
          F_INTH_RESET_ONE_IT(C_INTH_FRAME_IT);
      #else
          * (volatile UWORD16 *) INTH_IT_REG1 &= ~(1 << IQ_FRAME); // clear TDMA IRQ
      #endif
    #else
      * (volatile UWORD16 *) INTH_IT_REG  &= ~(1 << IQ_FRAME); // clear TDMA IRQ
    #endif

  }
#endif



