/************* Revision Controle System Header *************
 *                  GSM Layer 1 software 
 * L1_CUST.C
 *
 *        Filename l1_cust.c
 *        Version  3.66
 *        Date     03/21/03
 *
 ************* Revision Controle System Header *************/

//#define  GLOBAL


#include "string.h"

#include "l1_confg.h"
#include "l1_const.h"
#include "ulpd.h"
#include "tm_defs.h"       
#include "l1_types.h"
#include "l1_time.h"
#include "l1_trace.h"
#include "sys_types.h"
#include "sim.h"
#include "buzzer.h"
#include "serialswitch.h"

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
#include "l1_defty.h"
#include "l1_msgty.h"
#include "l1_tabs.h"
#include "l1_varex.h"

#if (VCXO_ALGO == 1)
  #include "l1_ctl.h"
#endif

#if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3))
  #include "spi_drv.h"
#endif

#if (RF==35)
  #include "tpudrv35.h"
  #include "l1_rf35.h"
  #include "l1_rf35.c"
#endif

#if (RF==12)
  #include "tpudrv12.h"
  #include "l1_rf12.h"
  #include "l1_rf12.c"
#endif

#if (RF==10)
  #include "tpudrv10.h"
  #include "l1_rf10.h"
  #include "l1_rf10.c"
#endif

#if (RF==8)
  #include "tpudrv8.h"
  #include "l1_rf8.h"
  #include "l1_rf8.c"
#endif

#if (RF==2)
  #include "l1_rf2.h"
  #include "l1_rf2.c"
#endif

// Nucleus functions
extern INT                TMD_Timer_State;
extern UWORD32            TMD_Timer;               // for big sleep 
extern UWORD32            TCD_Priority_Groups;
extern VOID              *TCD_Current_Thread;
extern TC_HCB            *TCD_Active_HISR_Heads[TC_HISR_PRIORITIES];
extern TC_HCB            *TCD_Active_HISR_Tails[TC_HISR_PRIORITIES];
extern TC_PROTECT         TCD_System_Protect;

#if (L2_L3_SIMUL == 0)
  #define FFS_WORKAROUND 1
#else
  #define FFS_WORKAROUND 0
#endif
#if (FFS_WORKAROUND == 1)
  #include "ffs.h"
#else
  typedef signed   int   int32;
  typedef signed char effs_t;
  typedef int32  filesize_t;
  effs_t ffs_fwrite(const char *name, void *addr, filesize_t size);
  effs_t ffs_fread(const char *name, void *addr, filesize_t size);  
#endif

// Import band configuration from Flash module (need to replace by an access function)
//extern UWORD8       std;
extern T_L1_CONFIG  l1_config;
extern T_L1S_GLOBAL l1s;

#if (CODE_VERSION != SIMULATION)
  // Import serial switch configuration
  extern char ser_cfg_info[2];
#endif

void   get_cal_from_nvmem (UWORD8 *ptr, UWORD16 len, UWORD8 id);
UWORD8 save_cal_in_nvmem  (UWORD8 *ptr, UWORD16 len, UWORD8 id);
void config_rf_rw_band(char type, UWORD8 read);
void config_rf_read(char type);
void config_rf_write(char type);

//TISH20040421 Sean modified for watchdog implemention
BOOL watchdog_applied(void)
{
	return TRUE;
}
//Sean modified end

enum {
  RF_ID        = 0,
  ADC_ID       = 1
};

/*-------------------------------------------------------*/
/* Parameters:    none                                   */
/* Return:        none                                   */
/* Functionality: Defines the location of rf-struct      */
/*                for each std.                          */
/*-------------------------------------------------------*/

const static T_BAND_CONFIG band_config[] =
{ /*ffs name, default addr, max carrier, min tx pwr */
  {"",(T_RF_BAND *) 0,0,0},//undefined
  {"900", (T_RF_BAND *)&rf_900,  174, 19 },//EGSM
  {"1800",(T_RF_BAND *)&rf_1800, 374, 15 },//DCS
  {"1900",(T_RF_BAND *)&rf_1900, 299, 15 },//PCS
  {"850", (T_RF_BAND *)&rf_850,  124, 19 },//GSM850
#if (RF == 10)
  {"1900_us",(T_RF_BAND *)&rf_1900, 299, 15 },//usdual 1900 rf tables are the same as 3band 1900 rf tables at the moment
#endif
  {"900", (T_RF_BAND *)&rf_900,  124, 19 } //GSM, this should be last entry
};

/*-------------------------------------------------------*/
/* Parameters:    none                                   */
/* Return:        none                                   */
/* Functionality: Defines the indices into band_config   */
/*                for each std.                          */
/*-------------------------------------------------------*/
const T_STD_CONFIG std_config[] =
{
 /* band1 index,  band2 index, txpwr turning point, first arfcn*/
  {           0,          0,     0, 0   }, // std = 0 not used
  { BAND_GSM900,  BAND_NONE,     0, 1   }, // std = 1 GSM
  { BAND_EGSM900, BAND_NONE,     0, 1   }, // std = 2 EGSM
  { BAND_PCS1900, BAND_NONE,    21, 512 }, // std = 3 PCS
  { BAND_DCS1800, BAND_NONE,    28, 512 }, // std = 4 DCS
  { BAND_GSM900,  BAND_DCS1800, 28, 1   }, // std = 5 DUAL
  { BAND_EGSM900, BAND_DCS1800, 28, 1   }, // std = 6 DUALEXT
  { BAND_GSM850,  BAND_NONE,     0, 128 }, // std = 7 850
#if (RF == 10)
  { BAND_GSM850,  BAND_PCS1900_US, 21, 1   }  // std = 8 850/1900
#else
  { BAND_GSM850,  BAND_PCS1900, 21, 1   }  // std = 8 850/1900
#endif
};

/*-------------------------------------------------------*/
/* Prototypes of external functions used in this file.   */
/*-------------------------------------------------------*/
void l1_initialize(T_MMI_L1_CONFIG *mmi_l1_config);
WORD16 Convert_l1_radio_freq  (UWORD16 radio_freq);

/*-------------------------------------------------------*/
/* Cust_recover_Os()                                      */
/*-------------------------------------------------------*/
/*                                                       */
/* Description: adjust OS from sleep duration            */
/* ------------                                          */
/* This function fix the :                               */
/* - system clock                                        */
/* - Nucleus timers                                      */
/* - xxxxxx (customer dependant)                         */
/*-------------------------------------------------------*/
 
UWORD8 Cust_recover_Os(void)
{
#if (CODE_VERSION != SIMULATION)
  if (l1_config.pwr_mngt == PWR_MNGT)
  {    
    UWORD32 current_system_clock;

    /***************************************************/
    // Fix System clock and Nucleus Timers if any....  */
    /***************************************************/
    // Fix System clock ....
    current_system_clock  = NU_Retrieve_Clock();
    current_system_clock += l1s.pw_mgr.sleep_duration;
    NU_Set_Clock(current_system_clock);

    // Fix Nucleus timer (if needed) ....
    if (TMD_Timer_State == TM_ACTIVE)
    {
      TMD_Timer -= l1s.pw_mgr.sleep_duration;
      if (!TMD_Timer) TMD_Timer_State = TM_EXPIRED;
    }

    /***************************************************/
    // Cust dependant part                        ...  */
    /***************************************************/
    //.............
    //.............
    //..............
    return(TRUE);

  }
#endif
}



/*-------------------------------------------------------*/
/* Cust_check_system()                                    */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* GSM 1.5 :                                             */
/* - authorize UWIRE clock to be stopped                 */
/*   and write value in l1s.pw_mgr.modules_status.       */
/* - authorize ARMIO clock to be stopped if the light is */
/*   off and write value in l1s.pw_mgr.modules_status.   */
/* - check if SIM clock have been stopped                */
/*   before allowing DEEP SLEEP.                         */
/* - check if UARTs are ready to enter deep sleep        */
/* - choose the sleep mode                               */
/*                                                       */
/* Return:                                               */
/* -------                                               */
/* DO_NOT_SLEEP, FRAME_STOP or CLOCK_STOP                */     
/*-------------------------------------------------------*/
UWORD8 Cust_check_system(void)
{
  extern UWORD8 why_big_sleep;
#if (CODE_VERSION != SIMULATION)
  if (l1_config.pwr_mngt == PWR_MNGT)
  {   

#if (L2_L3_SIMUL == 0)
    // Forbid deep sleep if the light is on 
    if(LT_Status())
    {
      //cut ARMIO and UWIRE clocks in big sleep
      l1s.pw_mgr.modules_status = ARMIO_CLK_CUT | UWIRE_CLK_CUT ;
      why_big_sleep = BIG_SLEEP_DUE_TO_LIGHT_ON;
      return(FRAME_STOP);  // BIG sleep
    }

    // Forbid deep sleep if the SIM and UARTs not ready 
    if(SIM_SleepStatus())
    {
#endif
      if(SER_UartSleepStatus()) 
      {
        return(CLOCK_STOP);   // DEEP sleep
      }
      else why_big_sleep = BIG_SLEEP_DUE_TO_UART;
#if (L2_L3_SIMUL == 0)
   }
   else why_big_sleep = BIG_SLEEP_DUE_TO_SIM;
#endif
        // cut ARMIO and UWIRE clocks in big sleep
        l1s.pw_mgr.modules_status = ARMIO_CLK_CUT | UWIRE_CLK_CUT ;
        return(FRAME_STOP);  // BIG sleep
  }
#else // Simulation part
  return(CLOCK_STOP);   // DEEP sleep
#endif
}


/*-------------------------------------------------------*/
/* Parameters:    none                                   */
/* Return:        none                                   */
/* Functionality: Read the RF configuration, tables etc. */
/*                from FFS files.                        */
/*-------------------------------------------------------*/
const static T_CONFIG_FILE config_files_common[] =
{
  #if (CODE_VERSION != SIMULATION)

    // The first char is NOT part of the filename. It is used for
    // categorizing the ffs file contents:
    // f=rf-cal,  F=rf-config,
    // t=tx-cal,  T=tx-config,
    // r=rx-cal,  R=rx-config,
    // s=sys-cal, S=sys-config,
    "f/gsm/rf/afcdac",             &rf.afc.eeprom_afc, sizeof(rf.afc.eeprom_afc),
    "F/gsm/rf/stdmap",             &rf.radio_band_support, sizeof(rf.radio_band_support),
#if (VCXO_ALGO == 1)
    "F/gsm/rf/afcparams",          &rf.afc.psi_sta_inv, 4 * sizeof(UWORD32) + 4 * sizeof(WORD16),
#else
    "F/gsm/rf/afcparams",          &rf.afc.psi_sta_inv, 4 * sizeof(UWORD32),
#endif

    "R/gsm/rf/rx/agcglobals",      &rf.rx.agc, 5 * sizeof(UWORD16),
    "R/gsm/rf/rx/il2agc",          &rf.rx.agc.il2agc_pwr[0], 3 * sizeof(rf.rx.agc.il2agc_pwr),
    "R/gsm/rf/rx/agcwords",        &AGC_TABLE, sizeof(AGC_TABLE),

    "s/sys/adccal",                &adc_cal, sizeof(adc_cal),

    "S/sys/abb",                   &abb, sizeof(abb),
    "S/sys/uartswitch",            &ser_cfg_info, sizeof(ser_cfg_info),

  #endif
    NULL,                          0, 0 // terminator
};

/*-------------------------------------------------------*/
/* Parameters:    none                                   */
/* Return:        none                                   */
/* Functionality: Read the RF configurations for         */ 
/*                each band from FFS files. These files  */
/*                are defined for one band, and and used */
/*                for all bands.                         */
/*-------------------------------------------------------*/
const static T_CONFIG_FILE config_files_band[] =
{
    // The first char is NOT part of the filename. It is used for
    // categorizing the ffs file contents:
    // f=rf-cal,  F=rf-config,
    // t=tx-cal,  T=tx-config,
    // r=rx-cal,  R=rx-config,
    // s=sys-cal, S=sys-config,
      
    // generic for all bands
    // band[0] is used as template for all bands. 
    "t/gsm/rf/tx/ramps",     &rf_band[0].tx.ramp_tables,    sizeof(rf_band[0].tx.ramp_tables),
    "t/gsm/rf/tx/levels",    &rf_band[0].tx.levels,         sizeof(rf_band[0].tx.levels),
    "t/gsm/rf/tx/calchan",   &rf_band[0].tx.chan_cal_table, sizeof(rf_band[0].tx.chan_cal_table),
    "T/gsm/rf/tx/caltemp",   &rf_band[0].tx.temp,           sizeof(rf_band[0].tx.temp),

    "r/gsm/rf/rx/calchan",   &rf_band[0].rx.agc_bands,      sizeof(rf_band[0].rx.agc_bands),
    "R/gsm/rf/rx/caltemp",   &rf_band[0].rx.temp,           sizeof(rf_band[0].rx.temp),
    "r/gsm/rf/rx/agcparams", &rf_band[0].rx.rx_cal_params,  sizeof(rf_band[0].rx.rx_cal_params),
    NULL,                          0,                   0 // terminator
};

void config_ffs_read(char type)
{
  config_rf_read(type);
  config_rf_rw_band(type, 1);
}

void config_ffs_write(char type)
{
  config_rf_write(type);
  config_rf_rw_band(type, 0);
}

void config_rf_read(char type)
{
    const T_CONFIG_FILE *file = config_files_common;

    while (file->name != NULL)
    {
        if (type == '*' || type == file->name[0]) {
            ffs_fread(&file->name[1], file->addr, file->size);
        }
        file++;
    }
}

void config_rf_write(char type)
{
    const T_CONFIG_FILE *file = config_files_common;

    while (file->name != NULL)
    {
        if (type == '*' || type == file->name[0]) {
            ffs_fwrite(&file->name[1], file->addr, file->size);
        }
        file++;
    }
}

void config_rf_rw_band(char type, UWORD8 read)
{
    const T_CONFIG_FILE *f1 = config_files_band;
    UWORD8 i;
    WORD32 offset;
    char name[64];
    char *p;
    UWORD8 std = l1_config.std.id;
    
#if FFS_WORKAROUND == 1
    struct stat_s stat;
    UWORD16 time;
#endif        
    for (i=0; i< GSM_BANDS; i++)
    { 
      if(std_config[std].band[i] !=0 )
      {
        f1 = &config_files_band[0];
        while (f1->name != NULL)
        {
          offset = (WORD32) f1->addr - (WORD32) &rf_band[0]; //offset in bytes
          p = ((char *) &rf_band[i]) + offset;
          if (type == '*' || type == f1->name[0]) 
          {
            strcpy(name, &f1->name[1]);
            strcat(name, ".");
            strcat(name, band_config[std_config[std].band[i]].name);

            if (read == 1)
              ffs_fread(name, p, f1->size);
            else //write == 0
            {  
              ffs_fwrite(name, p, f1->size);  

              // wait until ffs write has finished 
#if FFS_WORKAROUND == 1
              stat.inode = 0;
              time = 0;

              do {
                rvf_delay(10);  // in milliseconds
                time += 10;
                ffs_stat(name, &stat);
              } while (stat.inode == 0 && time < 500);
#endif
            }  
          }
          f1++;
        }
      }
    }
}

/*-------------------------------------------------------*/
/* Cust_init_std()                                       */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality : Init Standard variable configuration  */
/*-------------------------------------------------------*/
void Cust_init_std(void)
{
  UWORD8 std = l1_config.std.id;
  UWORD8 band1, band2;
  T_RF_BAND *pt1, *pt2;
  
  band1 = std_config[std].band[0];
  band2 = std_config[std].band[1];
  
  //get these from std 
  pt1 = band_config[band1].addr;
  pt2 = band_config[band2].addr;

  // copy rf-struct from default flash to ram
  memcpy(&rf_band[0], pt1, sizeof(T_RF_BAND));

  if(std_config[std].band[1] != BAND_NONE )
    memcpy(&rf_band[1], pt2, sizeof(T_RF_BAND));

  // Read all RF and system configuration from FFS *before* we copy any of
  // the rf structure variables to other places, like L1.

  config_ffs_read('*');

  l1_config.std.first_radio_freq       = std_config[std].first_arfcn;

  if(band2!=0)
    l1_config.std.first_radio_freq_band2 = band_config[band1].max_carrier + 1;
  else  
    l1_config.std.first_radio_freq_band2 = 0; //band1 carrier + 1 else 0
    
  // if band2 is not used it is initialised with zeros
  l1_config.std.nbmax_carrier          = band_config[band1].max_carrier;
  if(band2!=0)
     l1_config.std.nbmax_carrier      += band_config[band2].max_carrier;
    
  l1_config.std.max_txpwr_band1        = band_config[band1].max_txpwr;
  l1_config.std.max_txpwr_band2        = band_config[band2].max_txpwr;
  l1_config.std.txpwr_turning_point    = std_config[std].txpwr_tp;
  l1_config.std.cal_freq1_band1        = 0;
  l1_config.std.cal_freq1_band2        = 0;
    
  l1_config.std.g_magic_band1             = rf_band[MULTI_BAND1].rx.rx_cal_params.g_magic;
  l1_config.std.lna_att_band1             = rf_band[MULTI_BAND1].rx.rx_cal_params.lna_att;
  l1_config.std.lna_switch_thr_low_band1  = rf_band[MULTI_BAND1].rx.rx_cal_params.lna_switch_thr_low;
  l1_config.std.lna_switch_thr_high_band1 = rf_band[MULTI_BAND1].rx.rx_cal_params.lna_switch_thr_high;
  l1_config.std.swap_iq_band1             = rf_band[MULTI_BAND1].swap_iq;

  l1_config.std.g_magic_band2             = rf_band[MULTI_BAND2].rx.rx_cal_params.g_magic;
  l1_config.std.lna_att_band2             = rf_band[MULTI_BAND2].rx.rx_cal_params.lna_att;
  l1_config.std.lna_switch_thr_low_band2  = rf_band[MULTI_BAND2].rx.rx_cal_params.lna_switch_thr_low;
  l1_config.std.lna_switch_thr_high_band2 = rf_band[MULTI_BAND2].rx.rx_cal_params.lna_switch_thr_high;
  l1_config.std.swap_iq_band2             = rf_band[MULTI_BAND2].swap_iq;

  l1_config.std.radio_freq_index_offset   = l1_config.std.first_radio_freq-1;

  // init variable indicating which radio bands are supported by the chosen RF
  l1_config.std.radio_band_support = rf.radio_band_support;
}


/*-------------------------------------------------------*/
/* Cust_init_params()                                    */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality : Init RF dependent paramters (AGC, TX) */
/*-------------------------------------------------------*/
void Cust_init_params(void)
{

#if (CODE_VERSION==SIMULATION)
  extern UWORD16  simu_RX_SYNTH_SETUP_TIME; // set in xxx.txt l3 scenario file
  extern UWORD16  simu_TX_SYNTH_SETUP_TIME; // set in xxx.txt l3 scenario file

  l1_config.params.rx_synth_setup_time   = simu_RX_SYNTH_SETUP_TIME;
  l1_config.params.tx_synth_setup_time   = simu_TX_SYNTH_SETUP_TIME;
#else
  l1_config.params.rx_synth_setup_time   = RX_SYNTH_SETUP_TIME;
  l1_config.params.tx_synth_setup_time   = TX_SYNTH_SETUP_TIME;
#endif


  // Convert SYNTH_SETUP_TIME into SPLIT.
  // We have kept a margin of 20qbit (EPSILON_MEAS) to cover offset change and Scenario closing time + margin.
  l1_config.params.rx_synth_load_split   = 1L + (l1_config.params.rx_synth_setup_time + EPSILON_MEAS) / (BP_DURATION/BP_SPLIT);
  l1_config.params.tx_synth_load_split   = 1L + (l1_config.params.tx_synth_setup_time + EPSILON_MEAS) / (BP_DURATION/BP_SPLIT);

  l1_config.params.rx_synth_start_time   = TPU_CLOCK_RANGE + PROVISION_TIME - l1_config.params.rx_synth_setup_time;
  l1_config.params.tx_synth_start_time   = TPU_CLOCK_RANGE - l1_config.params.tx_synth_setup_time;

  l1_config.params.rx_change_synchro_time = l1_config.params.rx_synth_start_time - EPSILON_SYNC;
  l1_config.params.rx_change_offset_time  = l1_config.params.rx_synth_start_time - EPSILON_OFFS;

  l1_config.params.tx_change_offset_time = TIME_OFFSET_TX - 
                                           TA_MAX - 
                                           l1_config.params.tx_synth_setup_time - 
                                           EPSILON_OFFS;

  // TX duration = ramp up time + burst duration (data + tail bits) 
  l1_config.params.tx_nb_duration = UL_ABB_DELAY + rf.tx.guard_bits*4 + NB_BURST_DURATION_UL;
  l1_config.params.tx_ra_duration = UL_ABB_DELAY + rf.tx.guard_bits*4 + RA_BURST_DURATION;

  l1_config.params.tx_nb_load_split = 1L + (l1_config.params.tx_nb_duration - rf.tx.prg_tx - NB_MARGIN) / (BP_DURATION/BP_SPLIT);
  l1_config.params.tx_ra_load_split = 1L + (l1_config.params.tx_ra_duration - rf.tx.prg_tx - NB_MARGIN) / (BP_DURATION/BP_SPLIT);

  // time for the end of RX and TX TPU scenarios
  l1_config.params.rx_tpu_scenario_ending = RX_TPU_SCENARIO_ENDING;
  l1_config.params.tx_tpu_scenario_ending = TX_TPU_SCENARIO_ENDING; 

  // FB26 anchoring time is computed backward to leave only 6 qbit margin between
  // FB26 window and next activity (RX time tracking).
  // This margin is used as follow:
  //    Serving offset restore: 1 qbit (SERV_OFFS_REST_LOAD)
  //    Tpu Sleep:              2 qbit (TPU_SLEEP_LOAD)
  //                          ---------
  //                     Total: 3 qbit
   
  l1_config.params.fb26_anchoring_time = (l1_config.params.rx_synth_start_time - 
                                          #if (CODE_VERSION == SIMULATION)
                                          // simulator: end of scenario not included in window (no serialization)
                                            1 -
                                          #else
                                          // RF dependent end of RX TPU scenario
                                            l1_config.params.rx_tpu_scenario_ending -
                                          #endif
                                          EPSILON_SYNC -               
                                          TPU_SLEEP_LOAD -
                                          SERV_OFFS_REST_LOAD -   
                                          FB26_ACQUIS_DURATION -  
                                          PROVISION_TIME +        
                                          TPU_CLOCK_RANGE) % TPU_CLOCK_RANGE;

  l1_config.params.fb26_change_offset_time  = l1_config.params.fb26_anchoring_time +
                                              PROVISION_TIME - 
                                              l1_config.params.rx_synth_setup_time -
                                              EPSILON_OFFS;

  l1_config.params.guard_bits         = rf.tx.guard_bits;

  l1_config.params.prg_tx_gsm         = rf.tx.prg_tx;
  l1_config.params.prg_tx_dcs         = rf.tx.prg_tx; //delay for dual band not implemented yet

  l1_config.params.low_agc_noise_thr  = rf.rx.agc.low_agc_noise_thr;
  l1_config.params.high_agc_sat_thr   = rf.rx.agc.high_agc_sat_thr;
  l1_config.params.low_agc            = rf.rx.agc.low_agc;
  l1_config.params.high_agc           = rf.rx.agc.high_agc;
  l1_config.params.il_min             = IL_MIN;

  l1_config.params.fixed_txpwr        = FIXED_TXPWR;
  l1_config.params.eeprom_afc         = rf.afc.eeprom_afc;
  l1_config.params.setup_afc_and_rf   = SETUP_AFC_AND_RF;

  l1_config.params.psi_sta_inv        = rf.afc.psi_sta_inv;
  l1_config.params.psi_st             = rf.afc.psi_st;
  l1_config.params.psi_st_32          = rf.afc.psi_st_32;
  l1_config.params.psi_st_inv         = rf.afc.psi_st_inv;

#if (CODE_VERSION == SIMULATION)
  #if (VCXO_ALGO == 1)
    l1_config.params.afc_algo           = ALGO_AFC_LQG_PREDICTOR; // VCXO|VCTCXO - Choosing AFC algorithm
  #endif
#else
  #if (VCXO_ALGO == 1)
    l1_config.params.afc_dac_center       = rf.afc.dac_center;      // VCXO - assuming DAC linearity
    l1_config.params.afc_dac_min          = rf.afc.dac_min;         // VCXO - assuming DAC linearity
    l1_config.params.afc_dac_max          = rf.afc.dac_max;         // VCXO - assuming DAC linearity
    l1_config.params.afc_snr_thr          = rf.afc.snr_thr;         // VCXO - SNR threshold
    l1_config.params.afc_algo             = ALGO_AFC_LQG_PREDICTOR; // VCXO|VCTCXO - Choosing AFC algorithm
    l1_config.params.afc_win_avg_size_M   = C_WIN_AVG_SIZE_M;       // VCXO - Average psi values with this value
    l1_config.params.rgap_algo            = ALGO_AFC_RXGAP;         // VCXO - Choosing Reception Gap algorithm
    l1_config.params.rgap_bad_snr_count_B = C_RGAP_BAD_SNR_COUNT_B; // VCXO - Prediction SNR count
  #endif
  #endif

  #if DCO_ALGO
    #if (RF == 10)
      // Enable DCO algorithm for direct conversion RFs
      l1_config.params.dco_enabled = TRUE;
    #else
      l1_config.params.dco_enabled = FALSE;
    #endif
  #endif

  #if (ANALOG == 1)
    l1_config.params.debug1           = C_DEBUG1;             // Enable f_tx delay of 400000 cyc DEBUG     
    l1_config.params.afcctladd        = abb[ABB_AFCCTLADD];	  // Value at reset              
    l1_config.params.vbur             = abb[ABB_VBUR];		  // Uplink gain amp 0dB, Sidetone gain to mute
    l1_config.params.vbdr             = abb[ABB_VBDR];		  // Downlink gain amp 0dB, Volume control 0 dB
    l1_config.params.bbctl            = abb[ABB_BBCTL];		  // value at reset
    l1_config.params.apcoff           = abb[ABB_APCOFF];	  // value at reset
    l1_config.params.bulioff          = abb[ABB_BULIOFF];	  // value at reset
    l1_config.params.bulqoff          = abb[ABB_BULQOFF];	  // value at reset
    l1_config.params.dai_onoff        = abb[ABB_DAI_ON_OFF];  // value at reset
    l1_config.params.auxdac           = abb[ABB_AUXDAC];	  // value at reset
    l1_config.params.vbcr             = abb[ABB_VBCR];		  // VULSWITCH=0, VDLAUX=1, VDLEAR=1
    l1_config.params.apcdel           = abb[ABB_APCDEL];	  // value at reset
  #endif
  #if (ANALOG == 2)
    l1_config.params.debug1           = C_DEBUG1;             // Enable f_tx delay of 400000 cyc DEBUG      
    l1_config.params.afcctladd        = abb[ABB_AFCCTLADD];	  // Value at reset              
    l1_config.params.vbur             = abb[ABB_VBUR];		  // Uplink gain amp 0dB, Sidetone gain to mute
    l1_config.params.vbdr             = abb[ABB_VBDR];		  // Downlink gain amp 0dB, Volume control 0 dB
    l1_config.params.bbctl            = abb[ABB_BBCTL];		  // value at reset
    l1_config.params.bulgcal          = abb[ABB_BULGCAL];    // value at reset    
    l1_config.params.apcoff           = abb[ABB_APCOFF];	  // value at reset
    l1_config.params.bulioff          = abb[ABB_BULIOFF];	  // value at reset
    l1_config.params.bulqoff          = abb[ABB_BULQOFF];	  // value at reset
    l1_config.params.dai_onoff        = abb[ABB_DAI_ON_OFF];  // value at reset
    l1_config.params.auxdac           = abb[ABB_AUXDAC];	  // value at reset
    l1_config.params.vbcr             = abb[ABB_VBCR];		  // VULSWITCH=0, VDLAUX=1, VDLEAR=1
    l1_config.params.vbcr2            = abb[ABB_VBCR2];      // MICBIASEL=0, VDLHSO=0, MICAUX=0
    l1_config.params.apcdel           = abb[ABB_APCDEL];	  // value at reset
    l1_config.params.apcdel2          = abb[ABB_APCDEL2];    // value at reset    
  #endif													 
  #if (ANALOG == 3)
    l1_config.params.debug1           = C_DEBUG1;               // Enable f_tx delay of 400000 cyc DEBUG      
    l1_config.params.afcctladd        = abb[ABB_AFCCTLADD];     // Value at reset              
    l1_config.params.vbur             = abb[ABB_VBUR];          // Uplink gain amp 0dB, Sidetone gain to mute
    l1_config.params.vbdr             = abb[ABB_VBDR];          // Downlink gain amp 0dB, Volume control 0 dB
    l1_config.params.bbctl            = abb[ABB_BBCTL];         // value at reset
    l1_config.params.bulgcal          = abb[ABB_BULGCAL];       // value at reset    
    l1_config.params.apcoff           = abb[ABB_APCOFF];        // X2 Slope 128 and APCSWP disabled
    l1_config.params.bulioff          = abb[ABB_BULIOFF];       // value at reset
    l1_config.params.bulqoff          = abb[ABB_BULQOFF];       // value at reset
    l1_config.params.dai_onoff        = abb[ABB_DAI_ON_OFF];    // value at reset
    l1_config.params.auxdac           = abb[ABB_AUXDAC];        // value at reset
    l1_config.params.vbcr             = abb[ABB_VBCR];          // VULSWITCH=0
    l1_config.params.vbcr2            = abb[ABB_VBCR2];         // MICBIASEL=0, VDLHSO=0, MICAUX=0
    l1_config.params.apcdel           = abb[ABB_APCDEL];        // value at reset
    l1_config.params.apcdel2          = abb[ABB_APCDEL2];       // value at reset    
    l1_config.params.vbpop            = abb[ABB_VBPOP];         // HSOAUTO enabled
    l1_config.params.vau_delay_init   = abb[ABB_VAUDINITD];     // 2 TDMA Frames between VDL "ON" and VDLHSO "ON"
    l1_config.params.vaud_cfg         = abb[ABB_VAUDCR];        // value at reset
    l1_config.params.vauo_onoff       = abb[ABB_VAUOCR];        // speech on AUX and EAR
    l1_config.params.vaus_vol         = abb[ABB_VAUSCR];        // value at reset
    l1_config.params.vaud_pll         = abb[ABB_VAUDPLL];       // value at reset
  #endif
  
  // global variable for access to deep sleep time
  l1_config.params.sleep_time = 0;
}


/************************************/
/* Automatic Gain Control           */
/************************************/

/*-------------------------------------------------------*/
/* Cust_get_agc_from_IL()                                */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality : returns agc value                     */
/*-------------------------------------------------------*/
WORD8 Cust_get_agc_from_IL(UWORD16 radio_freq, UWORD16 agc_index, UWORD8 table_id)
{
// radio_freq currently not used
// this parameter is passed in order to allow band dependent tables for specific RFs
// (e.g. dual band RF with separate AGC H/W blocks for GSM and DCS)

  if (agc_index > 120)
    agc_index = 120;    // Clip agc_index

  switch (table_id)
  {
    case MAX_ID:  return(rf.rx.agc.il2agc_max[agc_index]);
    case AV_ID:   return(rf.rx.agc.il2agc_av[agc_index]);
    case PWR_ID:  return(rf.rx.agc.il2agc_pwr[agc_index]);
  }
}

/*-------------------------------------------------------*/
/* Cust_get_agc_band                                     */
/*-------------------------------------------------------*/
/* Parameters : radio_freq                               */
/* Return     : band number                              */
/* Functionality :  Computes the band for RF calibration */
/*-------------------------------------------------------*/
/*---------------------------------------------*/


  #if (CODE_VERSION == SIMULATION)
    UWORD16 Cust_get_agc_band(UWORD16 arfcn, UWORD8 gsm_band)
  #else
    UWORD16 inline Cust_get_agc_band(UWORD16 arfcn, UWORD8 gsm_band)
  #endif
    {
      WORD32 i ;

      for (i=0;i<RF_RX_CAL_CHAN_SIZE;i++)
      {
        if (arfcn <= rf_band[gsm_band].rx.agc_bands[i].upper_bound)
          return(i);
      }
      // Should never happen!
      return(0);
    }  

/*-------------------------------------------------------*/
/* Cust_is_band_high                                     */
/*-------------------------------------------------------*/
/* Parameters :  arfcn                                   */
/* Return     :  0 if low band                           */
/*               1 if high band                          */
/* Functionality : Generic function which return 1 if    */
/*                 arfcn is in the high band             */
/*-------------------------------------------------------*/

UWORD8 Cust_is_band_high(UWORD16 radio_freq)
{
  UWORD16 max_carrier;
  UWORD8  std = l1_config.std.id;

  max_carrier = band_config[std_config[std].band[0]].max_carrier;
  
  return(((radio_freq >= l1_config.std.first_radio_freq) && 
          (radio_freq < (l1_config.std.first_radio_freq + max_carrier))) ? MULTI_BAND1 : MULTI_BAND2);
}          
    
/*-------------------------------------------------------*/
/* l1ctl_encode_delta2()                                 */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
WORD8 l1ctl_encode_delta2(UWORD16 radio_freq)
{  
  WORD8    delta2_freq;
  UWORD16  i;
  UWORD16  arfcn;
  UWORD8   band;

  band  = Cust_is_band_high(radio_freq);
  arfcn = Convert_l1_radio_freq(radio_freq);
  
  i = Cust_get_agc_band(arfcn,band); //
  delta2_freq = rf_band[band].rx.agc_bands[i].agc_calib;

  //temperature compensation
  for (i=0;i<RF_RX_CAL_TEMP_SIZE;i++)
  {
    if ((WORD16)adc.converted[ADC_RFTEMP] <= rf_band[band].rx.temp[i].temperature)
    {
      delta2_freq += rf_band[band].rx.temp[i].agc_calib;
      break;
    }
  }

  return(delta2_freq);
}

/************************************/
/* TX Management                    */
/************************************/
/*-------------------------------------------------------*/
/* Cust_get_ramp_tab                                     */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/

void Cust_get_ramp_tab(API *a_ramp, UWORD8 txpwr_ramp_up, UWORD8 txpwr_ramp_down, UWORD16 radio_freq)
{
  UWORD16 index_up, index_down,j, arfcn;
  UWORD8 band;

  band  = Cust_is_band_high(radio_freq);
  arfcn = Convert_l1_radio_freq(radio_freq);

  index_up   = rf_band[band].tx.levels[txpwr_ramp_up].ramp_index;
  index_down = rf_band[band].tx.levels[txpwr_ramp_down].ramp_index;

  #if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3))
    for (j=0; j<16; j++)
    {
      a_ramp[j]=((rf_band[band].tx.ramp_tables[index_down].ramp_down[j])<<11) |
                ((rf_band[band].tx.ramp_tables[index_up].ramp_up[j])  << 6) |
                  0x14;
    }
  #endif 
}

/*-------------------------------------------------------*/
/* get_pwr_data                                          */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/

#if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3))
UWORD16 Cust_get_pwr_data(UWORD8 txpwr, UWORD16 radio_freq)
{  

  UWORD16 i,j;
  UWORD16 arfcn;
  UWORD8 band;

#if(ORDER2_TX_TEMP_CAL==1)
        WORD16 pwr_data;
#else
        UWORD16  pwr_data;
#endif

  band  = Cust_is_band_high(radio_freq);
  arfcn = Convert_l1_radio_freq(radio_freq);
   
  i = rf_band[band].tx.levels[txpwr].chan_cal_index;
  j=0;
  // get uncalibrated apc
  pwr_data = rf_band[band].tx.levels[txpwr].apc;

  while (arfcn > rf_band[band].tx.chan_cal_table[i][j].arfcn_limit)
    j++;

  // channel calibrate apc  
  pwr_data = ((UWORD32) (pwr_data * rf_band[band].tx.chan_cal_table[i][j].chan_cal))/128; 

  // temperature compensate apc
  {
    T_TX_TEMP_CAL *pt;

    pt = rf_band[band].tx.temp;
    while ((WORD16)adc.converted[ADC_RFTEMP] > pt->temperature)
	    pt++;
#if(ORDER2_TX_TEMP_CAL==1)
                pwr_data += (txpwr*(pt->a*txpwr + pt->b) + pt->c) / 64;      //delta apc = ax^2+bx+c
                if(pwr_data < 0) pwr_data = 0;
#else
    pwr_data += pt->apc_calib;
#endif
  }
  return(pwr_data);
}
#endif
/*-------------------------------------------------------*/
/* Cust_Init_Layer1                                      */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :  Load and boot the DSP                */
/* Initialize shared memory and L1 data structures       */
/*-------------------------------------------------------*/

void Cust_Init_Layer1(void)
{
  T_MMI_L1_CONFIG cfg;
  
  // Get the current band configuration from the flash
  #if (OP_WCP==1)
    extern unsigned char ffs_GetBand();
    cfg.std = ffs_GetBand();
  #else // NO OP_WCP
    //  cfg.std = std;
    cfg.std = STD;
  #endif // OP_WCP

  cfg.tx_pwr_code = 1;

  // sleep management configuration
  cfg.pwr_mngt = 0;
  cfg.pwr_mngt_mode_authorized = NO_SLEEP; //Sleep mode
  cfg.pwr_mngt_clocks = 0x5ff; // list of clocks cut in Big Sleep



  #if (CODE_VERSION != SIMULATION)
    cfg.dwnld = DWNLD; //external define from makefile
  #endif

  l1_initialize(&cfg);

  get_cal_from_nvmem((UWORD8 *)&rf, sizeof(rf), RF_ID);
  get_cal_from_nvmem((UWORD8 *)&adc_cal, sizeof(adc_cal), ADC_ID);

}


/*****************************************************************************************/
/***************************     TESTMODE functions     **********************************/
/*****************************************************************************************/



  /*------------------------------------------------------*/
  /* madc_hex_2_physical                                  */
  /*------------------------------------------------------*/
  /* Parameters :                                         */
  /* Return     :                                         */
  /* Functionality : Function to convert MAD hexadecimal  */ 
  /*                 values into physical values          */
  /*------------------------------------------------------*/

void madc_hex_2_physical (UWORD16 *adc_hex, T_ADC *adc_phy)  
{
 WORD16 i; 
 UWORD16 y;
 WORD16 Smin = 0, Smax = TEMP_TABLE_SIZE-1;
 WORD16 index = (TEMP_TABLE_SIZE-1)/2;       /* y is the adc code after compensation of ADC slope error introduced by VREF error */

  //store raw ADC values
  memcpy(&adc.raw[0], adc_hex, sizeof(adc.raw));   

  // Convert Vbat [mV] : direct equation with slope and offset compensation 
  for (i = ADC_VBAT; i<ADC_RFTEMP; i++)
    adc.converted[i] = (((UWORD32)(adc_cal.a[i] * adc.raw[i])) >>10) + adc_cal.b[i];

  /*Convert RF Temperature [Celsius]: binsearch into a table*/ 
  y = ((UWORD32)(adc_cal.a[ADC_RFTEMP] * adc.raw[ADC_RFTEMP]))>>8;       /* rf.tempcal is the calibration of VREF*/
  while((Smax-Smin) > 1 )
  {
    if(y < temperature[index].adc)
      Smax=index;
    else 
      Smin=index;
       
    index = (Smax+Smin)/2;
  }
  adc.converted[ADC_RFTEMP] = temperature[index].temp;
   
  for (i = ADC_RFTEMP+1; i<ADC_INDEX_END; i++)
    adc.converted[i] = (((UWORD32)(adc_cal.a[i] * adc.raw[i])) >>10) + adc_cal.b[i];
   
  //store converted ADC values
  memcpy(adc_phy, &adc.converted[0], sizeof(adc.raw));
}


  /*------------------------------------------------------*/
  /*  get_cal_from_nvmem                                  */
  /*------------------------------------------------------*/
  /* Parameters :                                         */
  /* Return     :                                         */
  /* Functionality : Copy calibrated parameter to         */ 
  /*                 calibration structure in RAM         */
  /*------------------------------------------------------*/

void get_cal_from_nvmem (UWORD8 *ptr, UWORD16 len, UWORD8 id)
{
 
}

  /*------------------------------------------------------*/
  /*  save_cal_from_nvmem                                 */
  /*------------------------------------------------------*/
  /* Parameters :                                         */
  /* Return     :                                         */
  /* Functionality : Copy calibrated structure from RAM   */ 
  /*                 into NV memory                       */
  /*------------------------------------------------------*/

UWORD8 save_cal_in_nvmem (UWORD8 *ptr, UWORD16 len, UWORD8 id)
{
  #if (OP_WCP == 1)
    //  FFS backup implementation an Avenger 2
    // Request MPU-S to backup the FFS 
    // after full calibration of device
    extern void ffs_backup(void);
    ffs_backup();
  #endif
  return (0);
}

#if (TRACE_TYPE == 4)

/*------------------------------------------------------*/
/*  l1_cst_l1_parameters                                */
/*------------------------------------------------------*/
/* Parameters :  s: pointer on configuration string     */
/* Return     :  nothing: global var are set            */
/* Functionality : Set global L1 vars for dynamic trace */ 
/*                 and configuration                    */
/*                                                      */
/*   This function is called when a CST message is sent */
/*   from the Condat Panel.                             */
/*------------------------------------------------------*/
void l1_cst_l1_parameters(char *s)
{
  /* 
    a sample command string can be: 
    L1_PARAMS=<1,2,3,4,5> or
    L1_PARAMS=<1,23,3E32,4,5>
    with n parameters (here: 5 params); n>=1
    parameters are decoded as hexadecimal unsigned integers (UWORD16)
  */

  UWORD8  uNParams = 0;  /* Number of parameters */
  UWORD32 aParam[10];    /* Parameters array */
  UWORD8  uIndex = 0;
  
  /* ***  retrieve all parameters *** */
  while (s[uIndex] != '<') uIndex++;
  uIndex++;
  aParam[0] = 0;
  
  /* uIndex points on 1st parameter */
 
  while (s[uIndex] != '>')
  {
    if (s[uIndex] == ',') 
    {
      uNParams++;
      aParam[uNParams] = 0;
    }
    else
    {
      /* uIndex points on a parameter char */
      UWORD8 uChar = s[uIndex];
      aParam[uNParams] = aParam[uNParams] << 4; /* shift 4 bits left */
      if ((uChar>='0') && (uChar<='9'))
        aParam[uNParams] += (uChar - '0');        /* retrieve value */
      else if ((uChar>='A') && (uChar<='F'))
        aParam[uNParams] += (10 + uChar - 'A');        /* retrieve value */
      else if ((uChar>='a') && (uChar<='f'))
        aParam[uNParams] += (10 + uChar - 'a');        /* retrieve value */
    }
    
    uIndex++; /* go to next char */
  }

  /* increment number of params */  
  uNParams++;

  /* *** handle parameters *** */
  /*
    1st param: command type
    2nd param: argument for command type
  */
  switch (aParam[0])
  {
    case 0: /* Trace setting */ 
      /* The 2nd parameter contains the trace bitmap*/
      if (uNParams >=2)
        trace_info.current_config->l1_dyn_trace = aParam[1];
      else 
        trace_info.current_config->l1_dyn_trace = 0; /* error case: disable all trace */
      Trace_dyn_trace_change();
      break;
    default: /* ignore it */
      break;
  } // switch
}

#endif

#if ((CHIPSET == 2) || (CHIPSET == 3) || (CHIPSET == 4) || \
     (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || \
     (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || \
     (CHIPSET == 11) || (CHIPSET == 12))    
/*-------------------------------------------------------*/
/* power_down_config() : temporary implementation !!!    */
/*-------------------------------------------------------*/
/* Parameters : sleep_mode (NO, SMALL, BIG, DEEP or ALL) */
/*              clocks to be cut in BIG sleep            */
/* Return     :                                          */
/* Functionality : set the l1s variables 				 */
/* l1s.pw_mgr.mode_authorized and l1s.pw_mgr.clocks      */
/* according to the desired mode.	                     */
/*-------------------------------------------------------*/
void power_down_config(UWORD8 sleep_mode, UWORD16 clocks)
{ 
#if (OP_L1_STANDALONE == 1)
  if(sleep_mode != NO_SLEEP)
#endif
  {
    l1_config.pwr_mngt = PWR_MNGT;
    l1s.pw_mgr.mode_authorized = sleep_mode;
    l1s.pw_mgr.clocks = clocks;
  }
  
#if (OP_L1_STANDALONE == 0)
  l1s.pw_mgr.enough_gaug = FALSE;
#endif
}
#endif

/* glowing,2004-06-16, import from M188 */
void layer1_em_get_rxlevqual(WORD32 *l1_rxlev_scell,WORD32 *l1_rxlev_dedic_sub,
	WORD32 *l1_rxqual_dedic,WORD32 *l1_rxqual_dedic_sub)
{
/*
	xmzhou_trace_string_value("values***********",(UINT32)(l1a_l1s_com.Scell_info.meas.acc));
	xmzhou_trace_string_value("values***********",(UINT32)(l1a_l1s_com.Scell_info.meas.nbr_meas));
	xmzhou_trace_string_value("values***********",(UINT32)(l1a_l1s_com.Smeas_dedic.acc_sub));
	xmzhou_trace_string_value("values***********",(UINT32)(l1a_l1s_com.Smeas_dedic.nbr_meas_sub));
	xmzhou_trace_string_value("values***********",(UINT32)(l1a_l1s_com.Smeas_dedic.qual_acc_full));
	xmzhou_trace_string_value("values***********",(UINT32)(l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full));
	xmzhou_trace_string_value("values***********",(UINT32)(l1a_l1s_com.Smeas_dedic.qual_acc_sub));
	xmzhou_trace_string_value("values***********",(UINT32)(l1a_l1s_com.Smeas_dedic.qual_nbr_meas_sub));
*/
	if(l1a_l1s_com.Scell_info.meas.nbr_meas != 0){
	*l1_rxlev_scell=(WORD32)(l1a_l1s_com.Scell_info.meas.acc)/(WORD32)(l1a_l1s_com.Scell_info.meas.nbr_meas);
	}else{
	*l1_rxlev_scell=(WORD32)(l1a_l1s_com.Scell_info.meas.acc)/(WORD32)(4);
	}
//	xmzhou_trace_string_value("l1_rxlev_scell",(UINT32)(*l1_rxlev_scell));

	if(l1a_l1s_com.Smeas_dedic.nbr_meas_sub !=0){
	*l1_rxlev_dedic_sub=(WORD32)(l1a_l1s_com.Smeas_dedic.acc_sub)/(WORD32)(l1a_l1s_com.Smeas_dedic.nbr_meas_sub);
	}else{
	*l1_rxlev_dedic_sub=0;
	}
//	xmzhou_trace_string_value("l1_rxlev_dedic_sub",(UINT32)(*l1_rxlev_dedic_sub));

	if(l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full !=0){
	*l1_rxqual_dedic=(WORD32)(l1a_l1s_com.Smeas_dedic.qual_acc_full)/(WORD32)(l1a_l1s_com.Smeas_dedic.qual_nbr_meas_full);
	}else{
	*l1_rxqual_dedic=0;
	}
//	xmzhou_trace_string_value("l1_rxqual_dedic",(UINT32)(*l1_rxqual_dedic));

	if(l1a_l1s_com.Smeas_dedic.qual_nbr_meas_sub){
	*l1_rxqual_dedic_sub=(WORD32)(l1a_l1s_com.Smeas_dedic.qual_acc_sub)/(WORD32)(l1a_l1s_com.Smeas_dedic.qual_nbr_meas_sub);
	}else{
	*l1_rxqual_dedic_sub=0;
	}
//	xmzhou_trace_string_value("l1_rxqual_dedic_sub",(UINT32)(*l1_rxqual_dedic_sub));
}

void layer1_em_get_mode(WORD32 *l1_mode)
{
	*l1_mode=l1a_l1s_com.mode;
}

/*glowing,2004-06-16, end of import */

