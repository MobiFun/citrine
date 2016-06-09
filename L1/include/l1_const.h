 /************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_CONST.H
 *
 *        Filename l1_const.h
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

#ifdef __MSDOS__              // Running BORLANDC compiler.
  #ifdef MVC
    #define EXIT exit(0)
    #define FAR
  #else
    #define EXIT DOS_Exit(0)
    #define FAR far
  #endif
#else                         // Running ARM compiler.
  #define FAR
  #define EXIT exit(0)
  #undef  stricmp	// appease gcc
  #define stricmp strcmp
#endif


#if (CODE_VERSION != SIMULATION)
  #undef  NULL		// appease gcc
  #define NULL                0
#endif

#define NO_PAR                0

#define NO_TASK               0
#define ALL_TASK              0xffffffff
#define ALL_PARAM             0xffffffff

#define TRUE                  1
#define TRUE_L                1L
#define FALSE                 0

#define NOT_PENDING           0
#define PENDING               1

#define INACTIVE              2
#define ACTIVE                3
#define RE_ENTERED            4
#define WAIT_IQ               5

//---------------------------------------------
// MCU-DSP bit-field bit position definitions
//---------------------------------------------
#if L1_GPRS
  #define GPRS_SCHEDULER     1  // Select GPRS scheduler
#endif
#define GSM_SCHEDULER        2  // Select GSM  scheduler

//-----------------------------
// POWER MANAGEMENT............
//-----------------------------
#define MIN_SLEEP_TIME  (SETUP_FRAME+2+l1_config.params.setup_afc_and_rf) //HW WAKE-UP+MIN_SLEEP(2)+AFC RESTORE(2)
#define TPU_LOAD              01
#define TPU_FREEZE            02

// SLEEP ALGO SWITCH
#define NO_SLEEP              00   // ------ + ------ + ------
#define SMALL_SLEEP           01   // SMALL  + ------ + ------
#define BIG_SLEEP             02   // ------ +   BIG  + ------
#define DEEP_SLEEP            03   // ------ +   BIG  +  DEEP
#define ALL_SLEEP             04   // SMALL  +   BIG  +  DEEP

// GAUGING SAMPLES
#define SIZE_HIST             10
#define MAX_BAD_GAUGING        3

// GAUG_IN_32T =  (HF in clock of 13Mhz*dpll) * ( LF in Khz)
#define GAUG_IN_32T           1348   // gauging duration is 1348*T32 measured on eva4

// DSP state need to be used to enter Deep Sleep mode
#if (W_A_DSP_IDLE3 == 1)
  #define C_DSP_IDLE3           3
#endif

//-------------------------------------------------
// INIT: value is 32.768Khz at [-500 ppm, +100 ppm]
//       to face temperature variation
//
// ACQUIS: variations allowed 32.768Khz +- 50 ppm
  // 9 frames gauging is 1348*T32 (measured on eva4)
// UPDATE: variation allowed is +- 6 ppm jitter
//-------------------------------------------------

#define MCUCLK                13000       // 13 Mhz
#define LF                    32.768
#define LF_100PPM             32.7712768  // 32.768*(1+100*10E-6)
#define LF_500PPM             32.751616   // 32.768*(1-500*10E-6)
#define LF_50PPM              32.7696384  // 32.768*(1+50*10E-6)
#define LF_6PPM               32.76819661 // 32.768*(1+6*10E-6)

#define NB_INIT               5           // nbr of gauging to pass to ACQUIS
#define NB_ACQU               10          // nbr of gauging to pass to UPDATE

#if (CHIPSET ==2 || CHIPSET ==3 || CHIPSET == 5 || CHIPSET == 6 || CHIPSET == 9) // PLL is at 65 Mhz !!!!!!
  #define PLL                5           // 5*13Mhz = 65 Mhz
  //-------------------------------------------------
  // INIT: value is 32.768Khz at [-500 ppm, +100 ppm]
  //
  // ACQUIS: variations allowed 32.768Khz +- 50 ppm
  // 9 frames gauging is 1348*T32 (measured on eva4)
  // UPDATE: variation allowed is +- 6 ppm jitter
  //-------------------------------------------------
  #define C_CLK_MIN             1983     // 65000/32.7712768 = 1983.444234
  #define C_CLK_INIT_MIN        29113    // 0.444234*2^16
  #define C_CLK_MAX             1984     // 65000 / 32.751616 = 1984.634896
  #define C_CLK_INIT_MAX        41608    // 0.634896*2^16
  #define C_DELTA_HF_ACQUIS     130      // 1348/32.768-1348/32.7696384 = 0.002056632ms
                                         // 0.002056632/0.0001538 = 130 T65Mhz
  #define C_DELTA_HF_UPDATE     15       // 1348/32.768-1348/32.76819661 =0.00024691ms
                                         // 0.00024691/0.0001538 = 15 T65Mhz
#endif

#define ARMIO_CLK_CUT       0x0001
#define UWIRE_CLK_CUT       0x0002

//-----------------------------
// Neighbour cell sync. reading
//-----------------------------
#if (L1_12NEIGH)
 #define NBR_NEIGHBOURS      12
#else
 #define NBR_NEIGHBOURS       6
#endif

//-----------------------------
// LAYER 1 MEASUREMENT TASKS...
//-----------------------------
#define NBR_L1S_MEAS_TASKS    4

#define FSMS                  0
#define I_BAMS                1
#define D_BAMS                2
#define SERVMS                3

#define FSMS_MEAS             (TRUE_L << FSMS)              // Measurement task on FULL list (Cell Selection/Idle).
#define I_BAMS_MEAS           (TRUE_L << I_BAMS)            // Measurement task on BA list in Idle.
#define D_BAMS_MEAS           (TRUE_L << D_BAMS)            // Measurement task on BA list in Dedicated.
#define SERVMS_MEAS           (TRUE_L << SERVMS)            // Measurement task for Serving.

#define FSMS_MEAS_MASK        ALL_TASK ^ FSMS_MEAS
#define I_BAMS_MEAS_MASK      ALL_TASK ^ I_BAMS_MEAS
#define D_BAMS_MEAS_MASK      ALL_TASK ^ D_BAMS_MEAS
#define SERVMS_MEAS_MASK      ALL_TASK ^ SERVMS_MEAS

#define A_D_BLEN              456                           // SACCH/SDCCH data block length (GSM 5.01 $7)
#define TCH_FS_BLEN           378                           // TCH FULL SPEECH block length
#define TCH_HS_BLEN           211                           // TCH HALF SPEECH block length
#define TCH_F_D_BLEN          456                           // FACCH, TCH_DATA block length

/*
 * FreeCalypso Frankenstein: the following definition was not present in
 * our TCS211 version and we had to pull it from the LoCosto version for
 * l1_cmplx.c to compile.  However, the comment in the place where it is
 * used says that it "valuable for code running on target with DSP 3606."
 */
#define MIN_ACCEPTABLE_SNR_FOR_SB 200  // threshold under which a SB shall be considered as not found

// Define max PM/TDMA according to DSP code and TPU RAM size
//----------------------------------------------------------

// NOTE: we should use a global variable initialized at L1 start and function of rx synth setup time.

#if ((CHIPSET == 2) || (CHIPSET == 3) || (CHIPSET == 4))

  // TPU RAM size limitation

  #define NB_MEAS_MAX       4
  #define NB_MEAS_MAX_GPRS  4

#elif ((CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7)  || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))

  #if (DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36)

    // DSP code 33: upto 8 PMs with GSM and GPRS scheduler

    #define NB_MEAS_MAX       8
    #define NB_MEAS_MAX_GPRS  8

  #elif (DSP == 32)

    // DSP code prior to code 33 support upto 4 PMs with GSM scheduler
    // and 8 PMs with GPRS scheduler, 6 for DSP 32 because of CPU load

    #define NB_MEAS_MAX       4
    #define NB_MEAS_MAX_GPRS  6

  #else


    // DSP code prior to code 33 support upto 4 PMs with GSM scheduler
    // and 8 PMs with GPRS scheduler

    #define NB_MEAS_MAX       4
    #define NB_MEAS_MAX_GPRS  8

  #endif
#endif
#if (AMR == 1)
  #define SID_UPDATE_BLEN       212                           // SID UPDATE block length
  #define RATSCCH_BLEN          212                           // RATSCCH block length
  #define TCH_AFS_BLEN          448                           // TCH Adaptative Full rate Speech block length
  // Note: the d_nerr value is calculated thanks to the bit class 1 of the block.
  // But the number AHS bit class 1 depends on the type of vocoder currently used (c.f. 5.03 &3.10.7.2)
  #define TCH_AHS_7_95_BLEN     188                           // TCH AHS 7.95 Speech block length
  #define TCH_AHS_7_4_BLEN      196                           // TCH AHS 7.4 Speech block length
  #define TCH_AHS_6_7_BLEN      200                           // TCH AHS 6.7 Speech block length
  #define TCH_AHS_5_9_BLEN      208                           // TCH AHS 5.9 Speech block length
  #define TCH_AHS_5_15_BLEN     212                           // TCH AHS 5.15 Speech block length
  #define TCH_AHS_4_75_BLEN     212                           // TCH AHS 4.75 Speech block length
#endif
//----------------------------------------
// LAYER 1 Asynchronous processes names...
//----------------------------------------
#if (TESTMODE) && !(L1_GPRS)
  #if (AUDIO_TASK == 1)
    #if (L1_GTT)
      #if (OP_L1_STANDALONE == 1)
        #define NBR_L1A_PROCESSES       45
      #else    
        #define NBR_L1A_PROCESSES       44
      #endif
    #else
      #if (OP_L1_STANDALONE == 1)
        #define NBR_L1A_PROCESSES       44
      #else    
        #define NBR_L1A_PROCESSES       43
      #endif
    #endif
  #else
    #if (L1_GTT)
      #if (OP_L1_STANDALONE == 1)
        #define NBR_L1A_PROCESSES       27
      #else
        #define NBR_L1A_PROCESSES       26
      #endif
  #else
      #if (OP_L1_STANDALONE == 1)
        #define NBR_L1A_PROCESSES       26
      #else  
        #define NBR_L1A_PROCESSES       25
      #endif
  #endif
#endif
#endif

#if (TESTMODE) && (L1_GPRS)
  #if (AUDIO_TASK == 1)
    #if (L1_GTT)
      #if (OP_L1_STANDALONE == 1)
        #define NBR_L1A_PROCESSES       46
      #else
        #define NBR_L1A_PROCESSES       45
      #endif
    #else
      #if (OP_L1_STANDALONE == 1)
        #define NBR_L1A_PROCESSES       45
      #else
        #define NBR_L1A_PROCESSES       44
      #endif
    #endif
  #else
    #if (L1_GTT)
      #if (OP_L1_STANDALONE == 1)
        #define NBR_L1A_PROCESSES       28
      #else
        #define NBR_L1A_PROCESSES       27
      #endif
  #else
      #if (OP_L1_STANDALONE == 1)
        #define NBR_L1A_PROCESSES       27
      #else
        #define NBR_L1A_PROCESSES       26
      #endif    
  #endif
#endif
#endif

#if !(TESTMODE)
  #if (AUDIO_TASK == 1)
    #if (L1_GTT)
       #if (OP_L1_STANDALONE == 1)
          #define NBR_L1A_PROCESSES       37
       #else
          #define NBR_L1A_PROCESSES       36
       #endif
    #else
      #if (OP_L1_STANDALONE == 1)
         #define NBR_L1A_PROCESSES       36
      #else
         #define NBR_L1A_PROCESSES       35
      #endif
    #endif
  #else
    #if (L1_GTT)
      #if (OP_L1_STANDALONE == 1)
          #define NBR_L1A_PROCESSES       19
      #else
          #define NBR_L1A_PROCESSES       18
      #endif
  #else
      #if (OP_L1_STANDALONE == 1)
          #define NBR_L1A_PROCESSES       18
      #else
          #define NBR_L1A_PROCESSES       17
      #endif
  #endif
#endif
#endif


#define FULL_MEAS                0   // l1a_full_list_meas_process(msg)
#define CS_NORM                  1   // l1a_cs_bcch_process(msg)
#define I_6MP                    2   // l1a_idle_6strongest_monitoring_process(msg)
#define I_SCP                    3   // l1a_idle_serving_cell_paging_process(msg)
#define I_SCB                    4   // l1a_idle_serving_cell_bcch_reading_process(msg)
#define I_SMSCB                  5   // l1a_idle_smscb_process(msg)
#define CR_B                     6   // l1a_cres_process(msg)
#define ACCESS                   7   // l1a_access_process(msg)
#define DEDICATED                8   // l1a_dedicated_process(msg)
#define I_FULL_MEAS              9   // l1a_dedicated_process(msg)
#define I_NMEAS                 10   // l1a_idle_ba_meas_process(msg)
#define DEDIC_6                 11   // l1a_dedic6_process(msg)
#define D_NMEAS                 12   // l1a_dedic_ba_list_meas_process(msg)
#define HW_TEST                 13   // l1a_test_process(msg)
#define I_BCCHN                 14   // l1a_idle_neighbour_cell_bcch_reading_process(msg)
#define I_ADC                   15   // l1a_mmi_adc_req(msg)

#if (TESTMODE) && !(L1_GPRS)
  #define TMODE_FB0                     16   // l1a_tmode_fb0_process(msg)
  #define TMODE_FB1                     17   // l1a_tmode_fb1_process(msg)
  #define TMODE_SB                      18   // l1a_tmode_sb_process(msg)
  #define TMODE_BCCH                    19   // l1a_tmode_bcch_reading_process(msg)
  #define TMODE_RA                      20   // l1a_tmode_access_process(msg)
  #define TMODE_DEDICATED               21   // l1a_tmode_dedicated_process(msg)
  #define TMODE_FULL_MEAS               22   // l1a_tmode_full_list_meas_process(msg)
  #define TMODE_PM                      23   // l1a_tmode_meas_process(msg)
  #if (AUDIO_TASK == 1)
    #define L1A_KEYBEEP_STATE           24   // l1a_mmi_keybeep_process(msg)
    #define L1A_TONE_STATE              25   // l1a_mmi_tone_process(msg)
    #define L1A_MELODY0_STATE           26   // l1a_mmi_melody0_process(msg)
    #define L1A_MELODY1_STATE           27   // l1a_mmi_melody1_process(msg)
    #define L1A_VM_PLAY_STATE           28   // l1a_mmi_vm_playing_process(msg)
    #define L1A_VM_RECORD_STATE         29   // l1a_mmi_vm_recording_process(msg)
    #define L1A_SR_ENROLL_STATE         30   // l1a_mmi_sr_enroll_process(msg)
    #define L1A_SR_UPDATE_STATE         31   // l1a_mmi_sr_update_process(msg)
    #define L1A_SR_RECO_STATE           32   // l1a_mmi_sr_reco_process(msg)
    #define L1A_SR_UPDATE_CHECK_STATE   33   // l1a_mmi_sr_update_check_process(msg)
    #define L1A_AEC_STATE               34   // l1a_mmi_aec_process(msg)
    #define L1A_FIR_STATE               35   // l1a_mmi_fir_process(msg)
    #define L1A_AUDIO_MODE_STATE        36   // l1a_mmi_audio_mode_process(msg)
    #define L1A_MELODY0_E2_STATE        37   // l1a_mmi_melody0_e2_process(msg)
    #define L1A_MELODY1_E2_STATE        38   // l1a_mmi_melody1_e2_process(msg)
    #define L1A_VM_AMR_PLAY_STATE       39   // l1a_mmi_vm_amr_playing_process(msg)
    #define L1A_VM_AMR_RECORD_STATE     40   // l1a_mmi_vm_amr_recording_process(msg)
    #define L1A_CPORT_STATE             41   // l1a_mmi_cport_process(msg)
    #if (L1_GTT == 1)
      #define L1A_GTT_STATE               42   // l1a_mmi_gtt_process(msg)
      #define INIT_L1                     43   // l1a_init_layer1_process(msg)
      #if (OP_L1_STANDALONE == 1)
        #define HSW_CONF                  44   // l1a_test_config_process(msg)
      #endif
    #else
      #define INIT_L1                     42   // l1a_init_layer1_process(msg)
      #if (OP_L1_STANDALONE == 1)
        #define HSW_CONF                  43  // l1a_test_config_process(msg)
      #endif
    #endif
  #else
    #if (L1_GTT == 1)
      #define L1A_GTT_STATE               24   // l1a_mmi_gtt_process(msg)
      #define INIT_L1                     25   // l1a_init_layer1_process(msg)
      #if (OP_L1_STANDALONE == 1)
        #define HSW_CONF                  26  // l1a_test_config_process(msg)
      #endif
    #else
      #define INIT_L1                     24   // l1a_init_layer1_process(msg)
      #if (OP_L1_STANDALONE == 1)
        #define HSW_CONF                  25  // l1a_test_config_process(msg)
      #endif
    #endif
  #endif
#endif

#if (TESTMODE) && (L1_GPRS)
  #define TMODE_FB0                     16   // l1a_tmode_fb0_process(msg)
  #define TMODE_FB1                     17   // l1a_tmode_fb1_process(msg)
  #define TMODE_SB                      18   // l1a_tmode_sb_process(msg)
  #define TMODE_BCCH                    19   // l1a_tmode_bcch_reading_process(msg)
  #define TMODE_RA                      20   // l1a_tmode_access_process(msg)
  #define TMODE_DEDICATED               21   // l1a_tmode_dedicated_process(msg)
  #define TMODE_FULL_MEAS               22   // l1a_tmode_full_list_meas_process(msg)
  #define TMODE_PM                      23   // l1a_tmode_meas_process(msg)
  #define TMODE_TRANSFER                24   // l1a_tmode_transfer_process(msg)
  #if (AUDIO_TASK == 1)
    #define L1A_KEYBEEP_STATE           25   // l1a_mmi_keybeep_process(msg)
    #define L1A_TONE_STATE              26   // l1a_mmi_tone_process(msg)
    #define L1A_MELODY0_STATE           27   // l1a_mmi_melody0_process(msg)
    #define L1A_MELODY1_STATE           28   // l1a_mmi_melody1_process(msg)
    #define L1A_VM_PLAY_STATE           29   // l1a_mmi_vm_playing_process(msg)
    #define L1A_VM_RECORD_STATE         30   // l1a_mmi_vm_recording_process(msg)
    #define L1A_SR_ENROLL_STATE         31   // l1a_mmi_sr_enroll_process(msg)
    #define L1A_SR_UPDATE_STATE         32   // l1a_mmi_sr_update_process(msg)
    #define L1A_SR_RECO_STATE           33   // l1a_mmi_sr_reco_process(msg)
    #define L1A_SR_UPDATE_CHECK_STATE   34   // l1a_mmi_sr_update_check_process(msg)
    #define L1A_AEC_STATE               35   // l1a_mmi_aec_process(msg)
    #define L1A_FIR_STATE               36   // l1a_mmi_fir_process(msg)
    #define L1A_AUDIO_MODE_STATE        37   // l1a_mmi_audio_mode_process(msg)
    #define L1A_MELODY0_E2_STATE        38   // l1a_mmi_melody0_e2_process(msg)
    #define L1A_MELODY1_E2_STATE        39   // l1a_mmi_melody1_e2_process(msg)
    #define L1A_VM_AMR_PLAY_STATE       40   // l1a_mmi_vm_amr_playing_process(msg)
    #define L1A_VM_AMR_RECORD_STATE     41   // l1a_mmi_vm_amr_recording_process(msg)
    #define L1A_CPORT_STATE             42   // l1a_mmi_cport_process(msg)
    #if (L1_GTT == 1)
      #define L1A_GTT_STATE             43
      #define INIT_L1                   44   // l1a_init_layer1_process(msg)
      #if (OP_L1_STANDALONE == 1)
        #define HSW_CONF                45  // l1a_test_config_process(msg)
      #endif
    #else
      #define INIT_L1                   43   // l1a_init_layer1_process(msg)
      #if (OP_L1_STANDALONE == 1)
        #define HSW_CONF                44  // l1a_test_config_process(msg)
      #endif
    #endif
  #else
    #if (L1_GTT == 1)
      #define L1A_GTT_STATE             25
      #define INIT_L1                   26   // l1a_init_layer1_process(msg)
      #if (OP_L1_STANDALONE == 1)
        #define HSW_CONF                27  // l1a_test_config_process(msg)
      #endif
    #else
      #define INIT_L1                   25   // l1a_init_layer1_process(msg)
      #if (OP_L1_STANDALONE == 1)
        #define HSW_CONF                26  // l1a_test_config_process(msg)
      #endif
    #endif
  #endif
#endif

#if !(TESTMODE) && (AUDIO_TASK == 1)
  #define L1A_KEYBEEP_STATE             16   // l1a_mmi_keybeep_process(msg)
  #define L1A_TONE_STATE                17   // l1a_mmi_tone_process(msg)
  #define L1A_MELODY0_STATE             18   // l1a_mmi_melody0_process(msg)
  #define L1A_MELODY1_STATE             19   // l1a_mmi_melody1_process(msg)
  #define L1A_VM_PLAY_STATE             20   // l1a_mmi_vm_playing_process(msg)
  #define L1A_VM_RECORD_STATE           21   // l1a_mmi_vm_recording_process(msg)
  #define L1A_SR_ENROLL_STATE           22   // l1a_mmi_sr_enroll_process(msg)
  #define L1A_SR_UPDATE_STATE           23   // l1a_mmi_sr_update_process(msg)
  #define L1A_SR_RECO_STATE             24   // l1a_mmi_sr_reco_process(msg)
  #define L1A_SR_UPDATE_CHECK_STATE     25   // l1a_mmi_sr_update_check_process(msg)
  #define L1A_AEC_STATE                 26   // l1a_mmi_aec_process(msg)
  #define L1A_FIR_STATE                 27   // l1a_mmi_fir_process(msg)
  #define L1A_AUDIO_MODE_STATE          28   // l1a_mmi_audio_mode_process(msg)
  #define L1A_MELODY0_E2_STATE          29   // l1a_mmi_melody0_e2_process(msg)
  #define L1A_MELODY1_E2_STATE          30   // l1a_mmi_melody1_e2_process(msg)
  #define L1A_VM_AMR_PLAY_STATE         31   // l1a_mmi_vm_amr_playing_process(msg)
  #define L1A_VM_AMR_RECORD_STATE       32   // l1a_mmi_vm_amr_recording_process(msg)
  #define L1A_CPORT_STATE               33   // l1a_mmi_cport_process(msg)
  #if (L1_GTT == 1)
    #define L1A_GTT_STATE               34   // l1a_mmi_tty_process(msg)
    #define INIT_L1                     35   // l1a_init_layer1_process(msg)
    #if (OP_L1_STANDALONE == 1)
      #define HSW_CONF                  36   // l1a_test_config_process(msg)
    #endif
  #else
    #define INIT_L1                     34   // l1a_init_layer1_process(msg)
    #if (OP_L1_STANDALONE == 1)
      #define HSW_CONF                  35   // l1a_test_config_process(msg)
    #endif
  #endif
#elif !(TESTMODE) && !(AUDIO_TASK == 1)
  #if (L1_GTT == 1)
    #define L1A_GTT_STATE               16   // l1a_mmi_tty_process(msg)
    #define INIT_L1                     17   // l1a_init_layer1_process(msg)
    #if (OP_L1_STANDALONE == 1)
      #define HSW_CONF                  18   // l1a_test_config_process(msg)
    #endif   
  #else
    #define INIT_L1                     16   // l1a_init_layer1_process(msg)
    #if (OP_L1_STANDALONE == 1)
      #define HSW_CONF                  17   // l1a_test_config_process(msg)
    #endif  
  #endif
#endif

#if TESTMODE
  #define TMODE_UPLINK            (1<<0)
  #define TMODE_DOWNLINK          (1<<1)
#endif

//------------------------------------
// LAYER 1 DOWNLINK & UPLINK TASKS...
//------------------------------------
#define TASK_DISABLED            0
#define TASK_ENABLED             1

#define SEMAPHORE_RESET          0
#define SEMAPHORE_SET            1

#define NO_NEW_TASK             -1


// Tasks in the order of their priority (low to high).

#if !L1_GPRS

  #define NBR_DL_L1S_TASKS  32

  //GSM_TASKS/
  #define HWTEST       0  // DSP checksum reading
  #define ADC_CSMODE0  1  // ADC task in CS_MODE0 mode
  #define DEDIC        2  // Global Dedicated mode switch
  #define RAACC        3  // Channel access (ul)
  #define RAHO         4  // Handover access (ul)
  #define NSYNC        5  // Global Neighbour cell synchro switch
  #define FBNEW        6  // Frequency burst search (Idle mode)
  #define SBCONF       7  // Synchro. burst confirmation
  #define SB2          8  // Synchro. burst read (1 frame uncertainty / SB position)
  #define FB26         9  // Frequency burst search, dedic/transfer mode MF26 or MF52
  #define SB26        10  // Synchro burst search, dedic/transfer mode MF26 or MF52
  #define SBCNF26     11  // Synchro burst confirmation, dedic/transfer mode MF26 or MF52
  #define FB51        12  // Frequency burst search, dedic mode MF51
  #define SB51        13  // Synchro burst search, dedic MF51
  #define SBCNF51     14  // Synchro burst confirmation, dedic MF51
  #define BCCHN       15  // BCCH Neighbor in GSM Idle
  #define ALLC        16  // All CCCH Reading
  #define EBCCHS      17  // Extended BCCH Serving Reading
  #define NBCCHS      18  // Normal BCCH ServingReading
  #define SMSCB       19  // CBCH serving Reading
  #define NP          20  // Normal paging Reading
  #define EP          21  // Extended pagingReading
  #define ADL         22  // SACCH(SDCCH) DL
  #define AUL         23  // SACCH(SDCCH) UL
  #define DDL         24  // SDCCH DL
  #define DUL         25  // SDCCH UL
  #define TCHD        26  // Dummy for TCH Half rate
  #define TCHA        27  // SACCH(TCH)
  #define TCHTF       28  // TCH Full rate
  #define TCHTH       29  // TCH Half rate
  #define BCCHN_TOP   30  // BCCH Neighbour TOP priority in Idle mode
  #define SYNCHRO     31  // synchro task: L1S reset
  //END_GSM_TASKS/

#else

  #define NBR_DL_L1S_TASKS  45

  //GPRS_TASKS/
  #define HWTEST       0   // DSP checksum reading
  #define ADC_CSMODE0  1   // ADC task in CS_MODE0 mode
  #define DEDIC        2   // Global Dedicated mode switch
  #define RAACC        3   // Channel access (ul)
  #define RAHO         4   // Handover access (ul)
  #define NSYNC        5   // Global Neighbour cell synchro switch
  #define POLL         6   // Packet Polling (Access)
  #define PRACH        7   // Packet Random Access Channel
  #define ITMEAS       8   // Interference measurements
  #define FBNEW        9   // Frequency burst search (Idle mode)
  #define SBCONF       10   // Synchro. burst confirmation
  #define SB2          11  // Synchro. burst read (1 frame uncertainty / SB position)
  #define PTCCH        12  // Packet Timing Advance control channel
  #define FB26         13  // Frequency burst search, dedic/transfer mode MF26 or MF52
  #define SB26         14  // Synchro burst search, dedic/transfer mode MF26 or MF52
  #define SBCNF26      15  // Synchro burst confirmation, dedic/transfer mode MF26 or MF52
  #define FB51         16  // Frequency burst search, dedic mode MF51
  #define SB51         17  // Synchro burst search, dedic MF51
  #define SBCNF51      18  // Synchro burst confirmation, dedic MF51
  #define PDTCH        19  // Packet Data channel
  #define BCCHN        20  // BCCH Neighbor in GSM Idle
  #define ALLC         21  // All CCCH Reading
  #define EBCCHS       22  // Extended BCCH Serving Reading
  #define NBCCHS       23  // Normal BCCH Serving Reading
  #define ADL          24  // SACCH(SDCCH) DL
  #define AUL          25  // SACCH(SDCCH) UL
  #define DDL          26  // SDCCH DL
  #define DUL          27  // SDCCH UL
  #define TCHD         28  // Dummy for TCH Half rate
  #define TCHA         29  // SACCH(TCH)
  #define TCHTF        30  // TCH Full rate
  #define TCHTH        31  // TCH Half rate
  #define PALLC        32  // All PCCCH reading
  #define SMSCB        33  // CBCH serving Reading
  #define PBCCHS       34  // PBCCH serving reading
  #define PNP          35  // Packet Normal paging Reading
  #define PEP          36  // Packet Extended paging Reading
  #define SINGLE       37  // Single Block for GPRS
  #define PBCCHN_TRAN  38  // Packet BCCH Neighbor in Packet Transfer mode.
  #define PBCCHN_IDLE  39  // Packet BCCH Neighbor in Idle mode.
  #define BCCHN_TRAN   40  // BCCH Neighbour in Packet Transfer mode
  #define NP           41  // Normal paging Reading
  #define EP           42  // Extended paging Reading
  #define BCCHN_TOP    43  // BCCH Neighbour TOP priority in Idle mode
  #define SYNCHRO      44  // synchro task: L1S reset
  //END_GPRS_TASKS/

#endif

//------------------------------------
// LAYER 1 API
//------------------------------------
#define MCSI_PORT1 0
#define MCSI_PORT2 1


//---------------------------------
// DSP vocoder Enable/ Disable
//---------------------------------

#if (L1M_WAIT_DSP_RESTART_AFTER_VOCODER_ENABLE ==1)
  #if (FF_L1_TCH_VOCODER_CONTROL == 1)
    #define TCH_VOCODER_DISABLE_REQ          0
    #define TCH_VOCODER_ENABLE_REQ           1
    #define TCH_VOCODER_ENABLED              2
    #define TCH_VOCODER_DISABLED             3

    // Number of TDMA wait frames until the DSP output is steady
    #define DSP_VOCODER_ON_TRANSITION      165   
  #endif // FF_L1_TCH_VOCODER_CONTROL
#endif 

//---------------------------------
// Handover Finished cause defines.
//---------------------------------
#define HO_COMPLETE              0
#define HO_TIMEOUT               1

//---------------------------------
// FB detection algorithm defines.
//---------------------------------
#define FB_MODE_0                0                          // FB detec. mode 0.
#define FB_MODE_1                1                          // FB detec. mode 1.

//---------------------------------
// AFC control defines.
//---------------------------------
#define AFC_INIT                 1
#define AFC_OPEN_LOOP            2
#define AFC_CLOSED_LOOP          3

// For VCXO algo.
#if (VCXO_ALGO)
#define AFC_INIT_CENTER          4
#define AFC_INIT_MAX             5
#define AFC_INIT_MIN             6
#endif
//---------------------------------
// TOA control defines.
//---------------------------------
#define TOA_INIT                 1
#define TOA_RUN                  2

//---------------------------------
// Neighbour Synchro possible status.
//---------------------------------
#define NSYNC_FREE               0
#define NSYNC_PENDING            1
#define NSYNC_COMPLETED          2
#if (L1_12NEIGH ==1)
  #define NSYNC_WAIT             3
#endif

/************************************/
/* Layer 1 constants declaration... */
/************************************/
#define MAX_FN           ((UWORD32)26*51*2048)

#if L1_GPRS
  #define MAX_BLOCK_ID   ((UWORD32) (3 * (UWORD32) (MAX_FN / 13))) // Block ID corresponding to fn = FN MAX
#endif

//--------------------------------------------------------
// standard specific constants used in l1_config.std.xxx
//--------------------------------------------------------


// GSM
#define FIRST_ARFCN_GSM               1    // 1st arfcn is 1
#define NBMAX_CARRIER_GSM           124    // 124 for GSM, 174 for E_GSM, 374 for DCS1800.
#define MAX_TXPWR_GSM                19    // lowest power ctrl level value in GSM band
// GSM_E
#define FIRST_ARFCN_EGSM              1    // 1st arfcn is 1
#define NBMAX_CARRIER_EGSM          174    // 174 carriers for GSM_E.
#define MAX_TXPWR_EGSM               19    // lowest power ctrl level value in GSM-E band
// PCS1900
#define FIRST_ARFCN_PCS             512    // 1st arfcn is 512
#define NBMAX_CARRIER_PCS           299    // 299 carriers for PCS1900.
#define MAX_TXPWR_PCS                15    // lowest power ctrl level value in PCS band
#define TXPWR_TURNING_POINT_PCS      21
// DCS1800
#define FIRST_ARFCN_DCS             512    // 1st arfcn is 512
#define NBMAX_CARRIER_DCS           374    // 374 carriers for DCS1800.
#define MAX_TXPWR_DCS                15    // lowest power ctrl level value in DCS band
#define TXPWR_TURNING_POINT_DCS      28
// GSM850
#define FIRST_ARFCN_GSM850          128    // 1st arfcn is 128
#define NBMAX_CARRIER_GSM850        124    // 124 carriers for GSM850
#define NBMEAS_GSM850                 3    // 3 measurement per frame TBD
#define MAX_TXPWR_GSM850             19    // lowest power ctrl level value in GSM band
// DUAL
#define FIRST_DCS_INDEX_DUAL        125    // 1st DCS index within the 498 continu list
#define NBMAX_CARRIER_DUAL      124+374    // 374 carriers for DCS1800 + 124 carriers for GSM900 Band
#define TXPWR_TURNING_POINT_DUAL     28
// DUALEXT
#define FIRST_DCS_INDEX_DUALEXT     175    // 1st DCS index within the 548 continu list
#define NBMAX_CARRIER_DUALEXT   174+374    // 374 carriers for DCS1800 + 174 carriers for E-GSM900 Band
#define TXPWR_TURNING_POINT_DUALEXT  28
// DUAL_US
#define FIRST_ARFCN_GSM850_DUAL_US       1    // 1st GSM850 index within the 423 continu list
#define FIRST_PCS_INDEX_DUAL_US        125    // 1st PCS index within the 423 continu list
#define NBMAX_CARRIER_DUAL_US      124+299    // 299 carriers for PCS1900 + 124 carriers for GSM850\ Band
#define NBMEAS_DUAL_US                   4    // 4 measurements per frames.
#define TXPWR_TURNING_POINT_DUAL_US     28    // TBD


#define NBMAX_CARRIER    NBMAX_CARRIER_DUALEXT //used in arrays for power measurement
                                           //non optimized!!! (dynamic memory allocation to optimize)
#define BAND1    1
#define BAND2    2

#define NO_TXPWR 255     // sentinal value used with UWORD8 type.


//--------------------------------------------------------
// Receive level values.
//--------------------------------------------------------
#define RXLEV63   63   // max value for RXLEV.
#define IL_MIN    240  // minimum input level is -120 dbm.

//--------------------------------------------------------
// Max number of cell to report in MPHC_RXLEV_IND.
// Nb cells to check to see if cell of MPHC_NETWORK_SYNC_REQ has been detected
//--------------------------------------------------------
#define MAX_MEAS_RXLEV_IND_TRACE 10
#define NB_FQ_TO_CHK 4

/*--------------------------------------------------------*/
/* Max value for GSM Paging Parameters.                   */
/*--------------------------------------------------------*/
#define MAX_AG_BLKS_RES_NCOMB   7
#define MAX_AG_BLKS_RES_COMB    2
#define MAX_PG_BLOC_INDEX_NCOMB 8
#define MAX_PG_BLOC_INDEX_COMB  2
#define MAX_BS_PA_MFRMS         9

/*--------------------------------------------------------*/
/* Position of different blocs in a MF51.                 */
/*--------------------------------------------------------*/
#define NBCCH_POSITION     2   // Normal BCCH position in a MF51.
#define EBCCH_POSITION     6   // Extended BCCH position in a MF51.
#define CCCH_0             6
#define CCCH_1            12
#define CCCH_2            16
#define CCCH_3            22
#define CCCH_4            26
#define CCCH_5            32
#define CCCH_6            36
#define CCCH_7            42
#define CCCH_8            46
#define FB_0               0
#define FB_1              10
#define FB_2              20
#define FB_3              30
#define FB_4              40
#define SB_0               1
#define SB_1              11
#define SB_2              21
#define SB_3              31
#define SB_4              41

/*--------------------------------------------------------*/
/* System information position in the "si_bit_map".       */
/*--------------------------------------------------------*/
#define SI_1              0x0001
#define SI_2              0x0002
#define SI_2BIS           0x0100
#define SI_2TER           0x0200
#define SI_3              0x0004
#define SI_4              0x0008
#define SI_7              0x0040
#define SI_8              0x0080
#define ALL_SI            SI_1 | SI_2 | SI_2BIS | SI_2TER | SI_3 | SI_4 | SI_7 | SI_8

/*--------------------------------------------------------*/
/* CBCH position in the "smscb_bit_map".                  */
/*--------------------------------------------------------*/
#define CBCH_TB1            0x0001
#define CBCH_TB2            0x0002
#define CBCH_TB3            0x0004
#define CBCH_TB5            0x0008
#define CBCH_TB6            0x0010
#define CBCH_TB7            0x0020

#define CBCH_CONTINUOUS_READING  0
#define CBCH_SCHEDULED           1
#define CBCH_INACTIVE            2

/*--------------------------------------------------------*/
/* Channel type definitions for DEDICATED mode.           */
/*--------------------------------------------------------*/

//TABLE/ CHAN TYPE
#define INVALID_CHANNEL    0
#define TCH_F              1
#define TCH_H              2
#define SDCCH_4            3
#define SDCCH_8            4
//END_TABLE/

/*--------------------------------------------------------*/
/* Channel mode definitions for DEDICATED.                */
/*--------------------------------------------------------*/
#define SIG_ONLY_MODE      0    // signalling only
#define TCH_FS_MODE        1    // speech full rate
#define TCH_HS_MODE        2    // speech half rate
#define TCH_96_MODE        3    // data 9,6 kb/s
#define TCH_48F_MODE       4    // data 4,8 kb/s full rate
#define TCH_48H_MODE       5    // data 4,8 kb/s half rate
#define TCH_24F_MODE       6    // data 2,4 kb/s full rate
#define TCH_24H_MODE       7    // data 2,4 kb/s half rate
#define TCH_EFR_MODE       8    // enhanced full rate
#define TCH_144_MODE       9    // data 14,4 kb/s half rate
#if (AMR == 1)
  #define TCH_AHS_MODE      10    // adaptative speech half rate
  #define TCH_AFS_MODE      11    // adaptative speech full rate
#endif


/*--------------------------------------------------------*/
/* Layer 1 functional modes for "mode" setting pupose.    */
/*--------------------------------------------------------*/
#define CS_MODE0           0    // functional mode at reset.
#define CS_MODE            1    // functional mode in CELL SELECTION.
#define I_MODE             2    // functional mode in IDLE.
#define CON_EST_MODE1      3    // functional mode in ACCESS (before 1st RA, for TOA convergency).
#define CON_EST_MODE2      4    // functional mode in ACCESS (after 1st RA).
#define DEDIC_MODE         5    // functional mode in DEDICATED.
#define DEDIC_MODE_HALF_DATA 6    // used only for TOA histogram length purpose.
#if L1_GPRS
  #define PACKET_TRANSFER_MODE 7
#endif

/*--------------------------------------------------------*/
/* Error causes for MPHC_NO_BCCH message.                  */
/*--------------------------------------------------------*/
#define NO_FB_SB           0  // FB or SB not found.
#define NCC_NOT_PERMITTED  1  // Synchro OK! but PLMN not permitted.

/*--------------------------------------------------------*/
/* MFTAB constants and flags.                             */
/*--------------------------------------------------------*/
#define L1_MAX_FCT        5        /* Max number of fctions in a frame */
#define MFTAB_SIZE       20

/********************************/
/* Software register/flags      */
/* definitions.                 */
/********************************/
#define NO_CTRL       (TRUE_L << 0)
#define CTRL_MS       (TRUE_L << 1)
#define CTRL_TX       (TRUE_L << 2)
#define CTRL_RX       (TRUE_L << 3)
#define CTRL_ADC      (TRUE_L << 4)
#define CTRL_SYNC     (TRUE_L << 5)
#define CTRL_ABORT    (TRUE_L << 6)
#define CTRL_TEST     (TRUE_L << 7)
#define CTRL_SYCB     (TRUE_L << 8)
#define CTRL_FB_ABORT (TRUE_L << 9)
#if L1_GPRS
  #define CTRL_PRACH     (TRUE_L << 10)
  #define CTRL_SYSINGLE  (TRUE_L << 11)
#endif


/********************************/
/* MISC management              */
/********************************/
#define GSM_CTL           0   // DSP ctrl for a GSM task
#define MISC_CTL          1   // DSP ctrl for a MISC task
#define GSM_MISC_CTL      2   // DSP ctrl for a GSM and MISC tasks

/********************************/
/* TOA management               */
/********************************/
#define  ISH_INVALID    128          // value used to disable the toa offset

/********************************/
/* AGC management               */
/********************************/
#define DPAGC_FIFO_LEN    4
#define DPAGC_MAX_FLAG    1
#if (AMR == 1)
  #define DPAGC_AMR_FIFO_LEN 4
#endif

/********************************/
/* ADC management               */
/********************************/
#define ADC_DISABLED                  0x0000
  // Traffic part
#define ADC_MASK_RESET_TRAFFIC        0xFF00
#define ADC_NEXT_TRAFFIC_UL           0x0001
#define ADC_EACH_TRAFFIC_UL           0x0002
#define ADC_NEXT_TRAFFIC_DL           0x0004
#define ADC_EACH_TRAFFIC_DL           0x0008
#define ADC_EACH_RACH                 0x0010


  // Idle part
#define ADC_MASK_RESET_IDLE           0x00FF
#define ADC_NEXT_NORM_PAGING          0x0100
#define ADC_EACH_NORM_PAGING          0x0200
#define ADC_NEXT_MEAS_SESSION         0x0400
#define ADC_EACH_MEAS_SESSION         0x0800
#define ADC_NEXT_NORM_PAGING_REORG    0x1000
#define ADC_EACH_NORM_PAGING_REORG    0x2000


  // CS_MODE0 part
#define ADC_NEXT_CS_MODE0             0x4000
#define ADC_EACH_CS_MODE0             0x8000


/********************************/
/*   Neighbor BCCH priorities   */
/********************************/

#define TOP_PRIORITY     0
#define HIGH_PRIORITY    1
#define NORMAL_PRIORITY  2

/********************************/
/* Driver constants definitions */
/********************************/

// Used to identify the 1st and last burst for offset management in Drivers.
#define BURST_1           0
#define BURST_2           1
#define BURST_3           2
#define BURST_4           3


// Identifier for all DSP tasks.
// ...RX & TX tasks identifiers.
#define NO_DSP_TASK        0  // No task.
#define NP_DSP_TASK       21  // Normal Paging reading task.
#define EP_DSP_TASK       22  // Extended Paging reading task.
#define NBS_DSP_TASK      19  // Normal BCCH serving reading task.
#define EBS_DSP_TASK      20  // Extended BCCH serving reading task.
#define NBN_DSP_TASK      17  // Normal BCCH neighbour reading task.
#define EBN_DSP_TASK      18  // Extended BCCH neighbour reading task.
#define ALLC_DSP_TASK     24  // CCCH reading task while performing FULL BCCH/CCCH reading task.
#define CB_DSP_TASK       25  // CBCH reading task.
#define DDL_DSP_TASK      26  // SDCCH/D (data) reading task.
#define ADL_DSP_TASK      27  // SDCCH/A (SACCH) reading task.
#define DUL_DSP_TASK      12  // SDCCH/D (data) transmit task.
#define AUL_DSP_TASK      11  // SDCCH/A (SACCH) transmit task.
#define RACH_DSP_TASK     10  // RACH transmit task.
#define TCHT_DSP_TASK     13  // TCH Traffic data DSP task id (RX or TX)
#define TCHA_DSP_TASK     14  // TCH SACCH   data DSP task id (RX or TX)
#define TCHD_DSP_TASK     28  // TCH Traffic data DSP task id (RX or TX)

#define TCH_DTX_UL        15  // Replace UL task in DSP->MCU com. to say "burst not transmitted".

#if (L1_GPRS)
  // Identifier for DSP tasks Packet dedicated.
  // ...RX & TX tasks identifiers.
  //------------------------------------------------------------------------
  // WARNING ... Need to aligned following macro with MCU/DSP GPRS Interface
  //------------------------------------------------------------------------
  #define PNP_DSP_TASK      30
  #define PEP_DSP_TASK      31
  #define PALLC_DSP_TASK    32
  #define PBS_DSP_TASK      33

  #define PTCCH_DSP_TASK    33

#endif

// Identifier for measurement, FB / SB search tasks.
// Values 1,2,3 reserved for "number of measurements".
#define FB_DSP_TASK        5  // Freq. Burst reading task in Idle mode.
#define SB_DSP_TASK        6  // Sync. Burst reading task in Idle mode.
#define TCH_FB_DSP_TASK    8  // Freq. Burst reading task in Dedicated mode.
#define TCH_SB_DSP_TASK    9  // Sync. Burst reading task in Dedicated mode.
#define IDLE1              1

// Debug tasks
#define CHECKSUM_DSP_TASK 33
#define TST_NDB           35  //  Checksum DSP->MCU
#define TST_DB            36  //  DB communication check
#define INIT_VEGA         37
#define DSP_LOOP_C        38

// Identifier for measurement, FB / SB search tasks.
// Values 1,2,3 reserved for "number of measurements".
#define TCH_LOOP_A        31
#define TCH_LOOP_B        32

#if (DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36)
  #define SC_CHKSUM_VER     (DB_W_PAGE_0 + (2 * (0x08DB - 0x800)))
#else
  #define SC_CHKSUM_VER     (DB_W_PAGE_0 + (2 * (0x09A0 - 0x800)))
#endif

// bits in d_gsm_bgd_mgt - background task management
#define B_DSPBGD_RECO           1       // start of reco in dsp background
#define B_DSPBGD_UPD            2       // start of alignement update in dsp background
#define B_DSPBGD_STOP_RECO      256     // stop of reco in dsp background
#define B_DSPBGD_STOP_UPD       512     // stop of alignement update in dsp background

// bit in d_pll_config
#define B_32KHZ_CALIB      (TRUE_L << 14) // force DSP in Idle1 during 32 khz calibration
// ****************************************************************
// NDB AREA (PARAM) MCU<->DSP COMMUNICATION DEFINITIONS
// ****************************************************************
// bits in d_tch_mode
#define B_EOTD            (TRUE_L << 0) // EOTD mode
#define B_PLAY_UL         (TRUE_L << 3) // Play UL
#define B_DCO_ON          (TRUE_L << 4) // DCO ON/OFF
#define B_AUDIO_ASYNC     (TRUE_L << 1) // WCP reserved

// ****************************************************************
// PARAMETER AREA (PARAM) MCU<->DSP COMMUNICATION DEFINITIONS
// ****************************************************************
#define C_POND_RED              1L
// below values are defined in the file l1_time.h
//#define D_NSUBB_IDLE            296L
//#define D_NSUBB_DEDIC           30L
#define D_FB_THR_DET_IACQ       0x3333L
#define D_FB_THR_DET_TRACK      0x28f6L
#define D_DC_OFF_THRES          0x7fffL
#define D_DUMMY_THRES           17408L
#define D_DEM_POND_GEWL         26624L
#define D_DEM_POND_RED          20152L
#define D_HOLE                  0L
#define D_TRANSFER_RATE         0x6666L

// Full Rate vocoder definitions.
#define D_MACCTHRESH1           7872L
#define D_MLDT                  -4L
#define D_MACCTHRESH            7872L
#define D_GU                    5772L
#define D_GO                    7872L
#define D_ATTMAX                53L
#define D_SM                    -892L
#define D_B                     208L
#define D_SD_MIN_THR_TCHFS      15L                   //(24L   *C_POND_RED)
#define D_MA_MIN_THR_TCHFS      738L                  //(1200L *C_POND_RED)
#define D_MD_MAX_THR_TCHFS      1700L                 //(2000L *C_POND_RED)
#define D_MD1_MAX_THR_TCHFS     99L                   //(160L  *C_POND_RED)

#if (DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36)
  // Frequency burst definitions
  #define D_FB_MARGIN_BEG         24
  #define D_FB_MARGIN_END         22

  // V42bis definitions
  #define D_V42B_SWITCH_HYST      16L
  #define D_V42B_SWITCH_MIN       64L
  #define D_V42B_SWITCH_MAX       250L
  #define D_V42B_RESET_DELAY      10L

  // Latencies definitions
  #if (DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36)
    // C.f. BUG1404
    #define D_LAT_MCU_BRIDGE        0x000FL
  #else
  #define D_LAT_MCU_BRIDGE        0x0009L
  #endif

  #define D_LAT_MCU_HOM2SAM       0x000CL

  #define D_LAT_MCU_BEF_FAST_ACCESS 0x0005L
  #define D_LAT_DSP_AFTER_SAM     0x0004L

  // Background Task in GSM mode: Initialization.
  #define D_GSM_BGD_MGT           0L

#if (CHIPSET == 4)
  #define D_MISC_CONFIG           0L
#elif (CHIPSET == 7)  || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12)
  #define D_MISC_CONFIG           1L
#else
  #define D_MISC_CONFIG           0L
#endif

#endif

// Hall Rate vocoder and ched definitions.

#define D_SD_MIN_THR_TCHHS      37L
#define D_MA_MIN_THR_TCHHS      344L
#define D_MD_MAX_THR_TCHHS      2175L
#define D_MD1_MAX_THR_TCHHS     138L
#define D_SD_AV_THR_TCHHS       1845L
#define D_WED_FIL_TC            0x7c00L
#define D_WED_FIL_INI           4650L
#define D_X_MIN                 15L
#define D_X_MAX                 23L
#define D_Y_MIN                 703L
#define D_Y_MAX                 2460L
#define D_SLOPE                 135L
#define D_WED_DIFF_THRESHOLD    406L
#define D_MABFI_MIN_THR_TCHHS   5320L
#define D_LDT_HR                -5
#define D_MACCTRESH_HR          6500
#define D_MACCTRESH1_HR         6500
#define D_GU_HR                 2620
#define D_GO_HR                 3700
#define D_B_HR                  182
#define D_SM_HR                 -1608
#define D_ATTMAX_HR             53

// Enhanced Full Rate vocoder and ched definitions.

#define C_MLDT_EFR              -4
#define C_MACCTHRESH_EFR        8000
#define C_MACCTHRESH1_EFR       8000
#define C_GU_EFR                4522
#define C_GO_EFR                6500
#define C_B_EFR                 174
#define C_SM_EFR                -878
#define C_ATTMAX_EFR            53
#define D_SD_MIN_THR_TCHEFS     15L                   //(24L   *C_POND_RED)
#define D_MA_MIN_THR_TCHEFS     738L                  //(1200L *C_POND_RED)
#define D_MD_MAX_THR_TCHEFS     1230L                 //(2000L *C_POND_RED)
#define D_MD1_MAX_THR_TCHEFS    99L                   //(160L  *C_POND_RED)


// Integrated Data Services definitions.
#define D_MAX_OVSPD_UL          8
// Detect frames containing 90% of 1s as synchro frames
#define D_SYNC_THRES            0x3f50
// IDLE frames are only frames with 100 % of 1s
#define D_IDLE_THRES            0x4000
#define D_M1_THRES              5
#define D_MAX_OVSP_DL           8

// d_ra_act: bit field definition
#define B_F48BLK                5

// Mask for b_itc information (d_ra_conf)
#define CE_MASK                 0x04

#define D_FACCH_THR             0
#define D_DSP_TEST              0
#define D_VERSION_NUMBER        0
#define D_TI_VERSION            0


/*------------------------------------------------------------------------------*/
/*                                                                              */
/*                 DEFINITIONS FOR DSP <-> MCU COMMUNICATION.                   */
/*                 ++++++++++++++++++++++++++++++++++++++++++                   */
/*                                                                              */
/*------------------------------------------------------------------------------*/
// COMMUNICATION Interrupt definition
//------------------------------------
#define ALL_16BIT          0xffffL
#define B_GSM_PAGE         (TRUE_L << 0)
#define B_GSM_TASK         (TRUE_L << 1)
#define B_MISC_PAGE        (TRUE_L << 2)
#define B_MISC_TASK        (TRUE_L << 3)

#define B_GSM_PAGE_MASK    (ALL_16BIT ^ B_GSM_PAGE)
#define B_GSM_TASK_MASK    (ALL_16BIT ^ B_GSM_TASK)
#define B_MISC_PAGE_MASK   (ALL_16BIT ^ B_MISC_PAGE)
#define B_MISC_TASK_MASK   (ALL_16BIT ^ B_MISC_TASK)

// Common definition
//----------------------------------
// Index to *_DEMOD* arrays.
#define D_TOA                    0  // Time Of Arrival.
#define D_PM                     1  // Power Measurement.
#define D_ANGLE                  2  // Angle (AFC correction)
#define D_SNR                    3  // Signal / Noise Ratio.

// Bit name/position definitions.
#define B_FIRE0                  5  // Fire result bit 0. (00 -> NO ERROR) (01 -> ERROR CORRECTED)
#define B_FIRE1                  6  // Fire result bit 1. (10 -> ERROR)    (11 -> unused)
#define B_SCH_CRC                8  // CRC result for SB decoding. (1 for ERROR).
#define B_BLUD                  15  // Uplink,Downlink data block Present. (1 for PRESENT).
#define B_AF                    14  // Activity bit: 1 if data block is valid.
#define B_BFI                    2  // Bad Frame Indicator
#define B_UFI                    0  // UNRELIABLE FRAME Indicator
#define B_ECRC                   9  // Enhanced full rate CRC bit
#define B_EMPTY_BLOCK           10  // for voice memo purpose, this bit is used to determine

#if (DEBUG_DEDIC_TCH_BLOCK_STAT == 1)
  #define FACCH_GOOD 10
  #define FACCH_BAD  11
#endif

#if (AMR == 1)
  // Place of the RX type in the AMR block header
  #define RX_TYPE_SHIFT           3
  #define RX_TYPE_MASK            0x0038

  // Place of the vocoder type in the AMR block header
  #define VOCODER_TYPE_SHIFT      0
  #define VOCODER_TYPE_MASK       0x0007

  // List of the possible RX types in a_dd block
  #define SPEECH_GOOD             0
  #define SPEECH_DEGRADED         1
  #define ONSET                   2
  #define SPEECH_BAD              3
  #define SID_FIRST               4
  #define SID_UPDATE              5
  #define SID_BAD                 6
  #define AMR_NO_DATA             7
  #define AMR_INHIBIT             8

  // List of possible RX types in RATSCCH block
  #define C_RATSCCH_GOOD          5

  // List of the possible AMR channel rate
  #define AMR_CHANNEL_4_75        0
  #define AMR_CHANNEL_5_15        1
  #define AMR_CHANNEL_5_9         2
  #define AMR_CHANNEL_6_7         3
  #define AMR_CHANNEL_7_4         4
  #define AMR_CHANNEL_7_95        5
  #define AMR_CHANNEL_10_2        6
  #define AMR_CHANNEL_12_2        7

  // Types of RATSCCH blocks
  #define C_RATSCCH_UNKNOWN                   0
  #define C_RATSCCH_CMI_PHASE_REQ             1
  #define C_RATSCCH_AMR_CONFIG_REQ_MAIN       2
  #define C_RATSCCH_AMR_CONFIG_REQ_ALT        3
  #define C_RATSCCH_AMR_CONFIG_REQ_ALT_IGNORE 4    // Alternative AMR_CONFIG_REQ with updates coming in the next THRES_REQ block
  #define C_RATSCCH_THRES_REQ                 5

  // These flags define a bitmap that indicates which AMR parameters are being modified by a RATSCCH
  #define C_AMR_CHANGE_CMIP  0
  #define C_AMR_CHANGE_ACS   1
  #define C_AMR_CHANGE_ICM   2
  #define C_AMR_CHANGE_THR1  3
  #define C_AMR_CHANGE_THR2  4
  #define C_AMR_CHANGE_THR3  5
  #define C_AMR_CHANGE_HYST1 6
  #define C_AMR_CHANGE_HYST2 7
  #define C_AMR_CHANGE_HYST3 8

  // CMIP default value
  #define C_AMR_CMIP_DEFAULT 1  // According to ETSI specification 05.09, cmip is always 1 by default (new channel, handover...)

#endif
// "d_ctrl_tch" bits positions for TCH configuration.
#define B_CHAN_MODE              0
#define B_CHAN_TYPE              4
#define B_RESET_SACCH            6
#define B_VOCODER_ON             7
#define B_SYNC_TCH_UL            8
#if (AMR == 1)
  #define B_SYNC_AMR               9
#else
#define B_SYNC_TCH_DL            9
#endif
#define B_STOP_TCH_UL           10
#define B_STOP_TCH_DL           11
#define B_TCH_LOOP              12
#define B_SUBCHANNEL            15

// "d_ctrl_abb" bits positions for conditionnal loading of abb registers.
#define B_RAMP                   0
#if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3))
  #define B_BULRAMPDEL             3 // Note: this name is changed
  #define B_BULRAMPDEL2            2 // Note: this name is changed
  #define B_BULRAMPDEL_BIS         9
  #define B_BULRAMPDEL2_BIS       10
#endif
#define B_AFC                    4

// "d_ctrl_system" bits positions.
#define B_TSQ                    0
#define B_BCCH_FREQ_IND          3
#define B_TASK_ABORT            15  // Abort RF tasks for DSP.

/*
 * FreeCalypso Frankenstein: the following definition has been
 * imported from LoCosto version of l1_const.h; it is needed for
 * the LoCosto-based C code to compile.
 */
#define C_BA_PM_MEAS (2)

// ****************************************************************
// POLESTAR EVABOARD 3 REGISTERS & ADRESSES  DEFINITIONS
// ****************************************************************


  //  DSP ADRESSES
  //--------------------

  #define DB_SIZE                 (4*20L)     // 4 pages of 20 words...

  #if (DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36)
    #define DB_W_PAGE_0          0xFFD00000L   // DB page 0 write : 20 words long
    #define DB_W_PAGE_1          0xFFD00028L   // DB page 1 write : 20 words long
    #define DB_R_PAGE_0          0xFFD00050L   // DB page 0 read  : 20 words long
    #define DB_R_PAGE_1          0xFFD00078L   // DB page 1 read  : 20 words long
    #define NDB_ADR              0xFFD001A8L   // NDB start address : 268 words
    #define PARAM_ADR            0xFFD00862L   // PARAM start address  : 57 words

    #if (DSP_DEBUG_TRACE_ENABLE == 1)
      #define DB2_R_PAGE_0       0xFFD00184L
      #define DB2_R_PAGE_1       0xFFD00188L
    #endif
  #else
    #define DB_W_PAGE_0          0xFFD00000L   // DB page 0 write : 20 words long
    #define DB_W_PAGE_1          0xFFD00028L   // DB page 1 write : 20 words long
    #define DB_R_PAGE_0          0xFFD00050L   // DB page 0 read  : 20 words long
    #define DB_R_PAGE_1          0xFFD00078L   // DB page 1 read  : 20 words long
    #define NDB_ADR              0xFFD000a0L   // NDB start address : 268 words
    #define PARAM_ADR            0xFFD002b8L   // PARAM start address  : 57 words
  #endif

// ****************************************************************
// ADC reading definitions
// ****************************************************************

#define ADC_READ_PERIOD (40)   //30 * 4.615 = 140ms


// ****************************************************************
// AGC: IL table identifier used by function Cust_get_agc_from_IL
// ****************************************************************
#define MAX_ID     1
#define AV_ID      2
#define PWR_ID     3

#if TESTMODE
  // ****************************************************************
  // Testmode: State of the continous mode
  // ****************************************************************
  #define TM_NO_CONTINUOUS        1   // continuous mode isn't active
  #define TM_START_RX_CONTINUOUS  2   // start the Rx continuous mode
  #define TM_START_TX_CONTINUOUS  3   // start the Tx continuous mode
  #define TM_CONTINUOUS           4   // Rx or Tx continuous mode
#endif
#if (AMR == 1)
  // ****************************************************************
  // AMR: Position of each AMR parameters in the AMR API buffer
  // ****************************************************************
  #define NSCB_INDEX    0
  #define NSCB_SHIFT    6
  #define ICMUL_INDEX   0
  #define ICMUL_SHIFT   4
  #define ICMDL_INDEX   0
  #define ICMDL_SHIFT   1
  #define ICMIUL_INDEX  0
  #define ICMIUL_SHIFT  3
  #define ICMIDL_INDEX  0
  #define ICMIDL_SHIFT  0
  #define ACSUL_INDEX   1
  #define ACSUL_SHIFT   0
  #define ACSDL_INDEX   1
  #define ACSDL_SHIFT   8
  #define THR1_INDEX    2
  #define THR1_SHIFT    0
  #define THR2_INDEX    2
  #define THR2_SHIFT    6
  #define THR3_INDEX    3
  #define THR3_SHIFT    8
  #define HYST1_INDEX   3
  #define HYST1_SHIFT   0
  #define HYST2_INDEX   3
  #define HYST2_SHIFT   4
  #define HYST3_INDEX   2
  #define HYST3_SHIFT   12
  #define NSYNC_INDEX   3
  #define NSYNC_SHIFT   14
  #define CMIP_INDEX    3
  #define CMIP_SHIFT    15

  #define NSCB_MASK     0x0001
  #define ICM_MASK      0x0003
  #define ICMI_MASK     0x0001
  #define ACS_MASK      0x00FF
  #define THR_MASK      0x003F
  #define HYST_MASK     0x000F
  #define CMIP_MASK     0x0001
#endif

