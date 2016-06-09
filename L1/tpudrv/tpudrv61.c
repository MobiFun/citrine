/****************** Revision Controle System Header ***********************
 *                      GSM Layer 1 software
 *              Copyright (c) Texas Instruments 1998
 *
 *        Filename tpudrv61.c
 *        Version  1.0
 *        Date     May 27th, 2005
 *
 ****************** Revision Controle System Header ***********************/

#define TPUDRV61_C

#include "rf.cfg"
#include "drp_api.h"

#include "l1_macro.h"
#include "l1_confg.h"
#include "l1_const.h"
#include "l1_types.h"
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

#if (L1_AAC == 1)
  #include "l1aac_defty.h"
#endif

#include "l1_defty.h"
#include "l1_time.h"
#include "l1_ctl.h"
#include "tpudrv.h"
#include "tpudrv61.h"
#include "l1_rf61.h"

#include "mem.h"

#include "armio.h"
#include "clkm.h"

#if (L1_RF_KBD_FIX == 1)
#include "l1_varex.h"
#endif

extern const UWORD8  drp_ref_sw[] ;
extern T_DRP_REGS_STR  *drp_regs;
extern T_DRP_SRM_API* drp_srm_api;


// Global variables
extern T_L1_CONFIG l1_config;
extern UWORD16  AGC_TABLE[];
extern UWORD16  *TP_Ptr;
#if (L1_FF_MULTIBAND == 1)
extern const WORD8 rf_subband2band[RF_NB_SUBBANDS];
#endif

static WORD8   rf_index;      // index into rf_path[]

#if( L1_TPU_DEV == 1)
WORD16 rf_rx_tpu_timings[NB_TPU_TIMINGS] =
{
// - RX up:
// The times below are offsets to when the 1st bit is at antenna
// Burst data comes here
  (PROVISION_TIME - 203 - DLT_4B - rdt ),  // TRF_R1  Set RX Synth channel
  //l1dmacro_adc_read_rx() called here requires ~ 16 tpuinst
   (PROVISION_TIME - 197 - DLT_4B - rdt ),   // TRF_R2  Select the AGC & LNA gains
   (PROVISION_TIME - 190 - DLT_4B - rdt ) ,     // TRF_R3   RX_ON
   (PROVISION_TIME -  39 - DLT_1  - rdt ),      // TRF_R4   Set RF switch for RX in selected band
   (PROVISION_TIME -  19 - DLT_1),               // TRF_R5   Enable RX_START
   (  -20 - DLT_4B  ),   // TRF_R6  Disable RX Start and RF switch
   // TRF_R6 not use, warning timing TRF_R6 > TRF_R7
   ( 2 - DLT_4B),  // TRF_R7 Power down RF (idle script)
 #if (L1_MADC_ON == 1)
   (PROVISION_TIME - 170 - DLT_4B - rdt ),    // for doing MADC
 #else
   0,
 #endif
   0,
   0,
   0, 0,0,0,0,0,0,0,0,0,
   0, 0,0,0,0,0,0,0,0,0,
   0,0
};

WORD16  rf_tx_tpu_timings[NB_TPU_TIMINGS] =
{
// - TX up:
// The times below are offsets to when TXSTART goes high
//burst data comes here
  ( - 255 - DLT_4B - tdt ),    //  TRF_T1    Set TX synth
  ( - 235 - DLT_4B - tdt ),    //  TRF_T2    Power ON TX
  ( - 225 - DLT_1        ),    //  TRF_T3
  ( - 100 - DLT_1        ),    //  TRF_T4
  ( -  30 - DLT_1        ),    //  TRF_T5
  (     0 - DLT_1        ),    //  TRF_T6
  (     8 - DLT_1        ),    //  TRF_T7
  (    16 - DLT_1        ),    //  TRF_T8
  // - TX timings ---
// - TX down:
// The times below are offsets to when TXSTART goes down
  ( -  40 - DLT_1        ),     // TRF_T9   ADC read
  (     0 - DLT_1        ),     // TRF_T10   Disable APC
  (    16 - DLT_1        ) ,    // TRF_T11  Disable PA
  (    20 - DLT_1        ) ,    // TRF_T12  Disable TXSTART
  (    30 - DLT_4B       ) ,    // TRF_T13  Power off Locosto
  0,
  0,
  0,
  0,
  0,0,0,
  0, 0,0,0,0,0,0,0,0,0,
  0,0
};

//Flexi ABB DELAYS
WORD16 rf_flexi_abb_delays[NB_ABB_DELAYS] = {
//Note: 0th element is not mapped to anything should be always 0
0, 20, (45L), 12, 0 , 0 , 0 , 12, 5, 6,
1L,  (63L + 4L),  (DL_DELAY_RF + UL_DELAY_2RF + (GUARD_BITS*4) + UL_DELAY_1RF + UL_ABB_DELAY),  2L,  20L, 10L, 20, 6,  0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0
};

#endif //TPU_DEVEL

// Internal function prototypes
void l1dmacro_rx_down (WORD32 t);

#if (L1_FF_MULTIBAND == 0)
SYS_UWORD16 Convert_l1_radio_freq(SYS_UWORD16 radio_freq);
WORD32 rf_init(WORD32 t);

// External function prototypes
UWORD8 Cust_is_band_high(UWORD16 radio_freq);
#endif


extern T_RF_BAND rf_band[];
extern T_RF rf;

/**************************************************************************/
/**************************************************************************/
/*         DEFINITION OF MACROS FOR CHIPS SERIAL PROGRAMMATION            */
/**************************************************************************/
/**************************************************************************/

/*------------------------------------------*/
/*   Is arfcn in the DCS band (512-885) ?   */
/*------------------------------------------*/
#define IS_HIGH_BAND(arfcn) (((arfcn >= 512) && (arfcn <= 885)) ? 1 : 0)


/*------------------------------------------*/
/*   Send a value to LoCosto               */
/*------------------------------------------*/
#define MOVE_REG_TSP_TO_RF(data, addr)\
	{\
	*TP_Ptr++ = TPU_MOVE(OCP_DATA_MSB, ((data)>>8) & 0x00FF);      \
	*TP_Ptr++ = TPU_MOVE(OCP_DATA_LSB, (data)     & 0x00FF);      \
	*TP_Ptr++ = TPU_MOVE(OCP_ADDRESS_MSB, ((addr)>>8) & 0x00FF);      \
	*TP_Ptr++ = TPU_MOVE(OCP_ADDRESS_LSB, (addr)     & 0x00FF);     \
	*TP_Ptr++ = TPU_MOVE(OCP_ADDRESS_START,     0x0001);                \
	}

/* RFTime environment */
#if defined (HOST_TEST)
  #include "hostmacros.h"
#endif

/*------------------------------------------*/
/*    Trace arfcn for conversion debug      */
/*------------------------------------------*/
#ifdef ARFCN_DEBUG
  // ----Debug information : record all arfcn programmed into synthesizer!
  #define MAX_ARFCN_TRACE     4096  // enough for 5 sessions of 124+374
  SYS_UWORD16 arfcn_trace[MAX_ARFCN_TRACE];
  static UWORD32 arfcn_trace_index = 0;

  void trace_arfcn(SYS_UWORD16 arfcn)
  {
    arfcn_trace[arfcn_trace_index++] = arfcn;

    // Wrap to beginning
    if (arfcn_trace_index == MAX_ARFCN_TRACE)
      arfcn_trace_index = 0;
  }
#endif


/**************************************************************************/
/**************************************************************************/
/*               DEFINITION OF HARWARE DEPENDANT CONSTANTS                */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/**************************************************************************/
/*                  INTERNAL FUNCTIONS OF TPUDRV14.C                      */
/*                 EFFECTIVE DOWNLOADING THROUGH TSP                      */
/**************************************************************************/
/**************************************************************************/
// rx & tx
typedef struct tx_rx_s
{
  UWORD16 farfcn0;
  WORD8 ou;
}
T_TX_RX;

struct synth_s {
    // common
    UWORD16 arfcn0;
    UWORD16 limit;
    T_TX_RX tx_rx[2];
};

struct rf_path_s {
  UWORD8 rx_up;
  UWORD8 rx_down;
  UWORD8 tx_up;
  UWORD8 tx_down;
  struct synth_s *synth;
};

const struct synth_s synth_900[] =
{
  {  0,  124, {{ 890,   1}, { 935,   2}}},// gsm    0 - 124
  {974, 1023, {{ 880,   1}, { 925,   2}}},// egsm 975 - 1023
};

const struct synth_s synth_1800[] =
{
  {511, 885, {{1710,  1},  {1805,   1}}}, // dcs  512 - 885
};

const struct synth_s synth_1900[] =
{
  {511, 810, {{1850,  1},  {1930,   1}}}, // pcs  512 - 810;
};

const struct synth_s synth_850[] =
{
  {127, 192, {{ 824,   2},  { 869,   2}}}, // gsm850 128 - 251   //low
  {127, 251, {{ 824,   1},  { 869,   2}}}, // gsm850 128 - 251   //high
};

#if RF_BAND_SYSTEM_INDEX == RF_QUADBAND
struct rf_path_s rf_path[] = {    //same index used as for band_config[] - 1
  { RU_900,  RD_900,  TU_900,  TD_900,  (struct synth_s *)synth_900 }, //EGSM
  { RU_1800, RD_1800, TU_1800, TD_1800, (struct synth_s *)synth_1800}, //DCS
  { RU_1900, RD_1900, TU_1900, TD_1900, (struct synth_s *)synth_1900}, //PCS
  { RU_850,  RD_850,  TU_850,  TD_850,  (struct synth_s *)synth_850 }, //GSM850
};
#endif

#if RF_BAND_SYSTEM_INDEX == RF_EU_TRIBAND
struct rf_path_s rf_path[] = {    //same index used as for band_config[] - 1
  { RU_850,  RD_850,  TU_900,  TD_900,  (struct synth_s *)synth_900 }, //EGSM
  { RU_1800, RD_1800, TU_1800, TD_1800, (struct synth_s *)synth_1800}, //DCS
  { RU_1900, RD_1900, TU_1900, TD_1900, (struct synth_s *)synth_1900}, //PCS
  { RU_850,  RD_850,  TU_850,  TD_850,  (struct synth_s *)synth_850 }, //GSM850
};
#endif

#if RF_BAND_SYSTEM_INDEX == RF_US_DUALBAND
struct rf_path_s rf_path[] = {    //same index used as for band_config[] - 1
  { RU_900,  RD_900,  TU_900,  TD_900,  (struct synth_s *)synth_900 }, //EGSM
  { RU_1800, RD_1800, TU_1800, TD_1800, (struct synth_s *)synth_1800}, //DCS
  { RU_1800, RD_1800, TU_1900, TD_1900, (struct synth_s *)synth_1900}, //PCS
  { RU_850,  RD_850,  TU_850,  TD_850,  (struct synth_s *)synth_850 }, //GSM850
};
#endif

#if RF_BAND_SYSTEM_INDEX == RF_PCS1900_900_DUALBAND
struct rf_path_s rf_path[] = {    //same index used as for band_config[] - 1
  { RU_850,  RD_850,  TU_900,  TD_900,  (struct synth_s *)synth_900 }, //EGSM
  { RU_1800, RD_1800, TU_1800, TD_1800, (struct synth_s *)synth_1800}, //DCS
  { RU_1800, RD_1800, TU_1900, TD_1900, (struct synth_s *)synth_1900}, //PCS
  { RU_850,  RD_850,  TU_850,  TD_850,  (struct synth_s *)synth_850 }, //GSM850
};
#endif


UWORD32 calc_rf_freq(UWORD16 arfcn, UWORD8 downlink)
{
UWORD32 farfcn;
struct synth_s  *s;

  s = rf_path[rf_index].synth;
  while(s->limit < arfcn)
    s++;

  // Convert the ARFCN to the channel frequency (times 5 to avoid the decimal value)
  farfcn = 5*s->tx_rx[downlink].farfcn0 + (arfcn - s->arfcn0);

  // LoCosto DLO carrier frequency is programmed in 100kHz increments.
  // Therefore RF_FREQ = (channel frequency * 10) = (farfcn * 2)
  return ( 2*farfcn );
}

#if (L1_FF_MULTIBAND == 0)
/*------------------------------------------*/
/*          Convert_l1_radio_freq           */
/*------------------------------------------*/
/*      conversion of l1 radio_freq to      */
/*         real channel number              */
/*------------------------------------------*/
SYS_UWORD16 Convert_l1_radio_freq(SYS_UWORD16 radio_freq)
{
  switch(l1_config.std.id)
  {
    case GSM:
    case DCS1800:
    case PCS1900:
    case GSM850:
      return (radio_freq);
//omaps00090550    break;

    case DUAL:
    {
      if (radio_freq < l1_config.std.first_radio_freq_band2)
      // GSM band...
        return(radio_freq);
      else
      // DCS band...
        return (radio_freq - l1_config.std.first_radio_freq_band2 + 512);
    }
//omaps00090550    break;

    case DUALEXT:
    {
      if (radio_freq < l1_config.std.first_radio_freq_band2)
      // E-GSM band...
      {
        if(radio_freq <= 124)
        // GSM part...
          return(radio_freq);
        if(radio_freq < 174)
        // Extended part...
          return (radio_freq - 125 + 975);
        else
        // Extended part, special case of ARFCN=0
          return(0);
      }
      else
      {
      // DCS band...
        return (radio_freq - l1_config.std.first_radio_freq_band2 + 512);
      }
    }
//    break;

   case GSM_E:
    {
      if(radio_freq <= 124)
      // GSM part...
        return(radio_freq);
      else
      if(radio_freq < 174)
      // Extended part...
        return (radio_freq - 125 + 975);
      else
      // Extended part, special case of ARFCN=0
        return(0);
    }
//omaps00090550    break;

    case DUAL_US:
    {
      if (radio_freq < l1_config.std.first_radio_freq_band2)
      {
        return(radio_freq - l1_config.std.first_radio_freq + 128);
      }
      else
      {
      // PCS band...
        return (radio_freq - l1_config.std.first_radio_freq_band2 + 512);
      }
    }
//    break;

    default: // should never occur.
      return(radio_freq);
  }  // end of switch
}
#else
static const UWORD8 rf_band_idx_to_locosto_idx[] =
{
#if (GSM900_SUPPORTED == 1)
  0,
#endif
#if (GSM850_SUPPORTED == 1)
  3,
#endif
#if (DCS1800_SUPPORTED == 1)
  1,
#endif
#if (PCS1900_SUPPORTED == 1)
  2
#endif
};

SYS_UWORD16 Convert_l1_radio_freq(SYS_UWORD16 radio_freq)
{
    UWORD8 band_index;
    return(rf_convert_l1freq_to_arfcn_rfband(rf_convert_rffreq_to_l1freq(radio_freq),
        &band_index));
}

#endif

/*------------------------------------------*/
/*              rf_init                     */
/*------------------------------------------*/
/*    Initialization routine for PLL        */
/*   Effective downloading through TSP      */
/*------------------------------------------*/
WORD32 rf_init(WORD32 t)
{
//UWORD16 temp=(UWORD16)( ((UWORD32)(&drp_srm_api->control.retiming))&0xFFFF) ;
  // enable control of retiming
  MOVE_REG_TSP_TO_RF(RETIM_DISABLE,  ((UWORD16)( ((UWORD32)(&drp_srm_api->control.retiming))&0xFFFF)));

  // Power ON the regulators by sending REG_ON script
  MOVE_REG_TSP_TO_RF(START_SCRIPT(DRP_REG_ON), ((UWORD16)( ((UWORD32)(&drp_regs->SCRIPT_STARTL)))));

  return(t);
}




#if (L1_FF_MULTIBAND == 0)
UWORD8 arfcn_to_rf_index(SYS_UWORD16 arfcn)
{
  UWORD8 index;
  extern const T_STD_CONFIG std_config[];
  index = std_config[l1_config.std.id].band[0];

  if ((std_config[l1_config.std.id].band[1] != BAND_NONE) && IS_HIGH_BAND(arfcn))
    index = std_config[l1_config.std.id].band[1];

  return (index - 1);
}
#endif

/*------------------------------------------*/
/*              rf_program                  */
/*------------------------------------------*/
/*      Programs the RF synthesizer         */
/*           called each frame              */
/*      downloads NA counter value          */
/*    t = start time in the current frame   */
/*------------------------------------------*/        //change 2 UWORD8
UWORD32 rf_program(UWORD32 t, SYS_UWORD16 radio_freq, UWORD32 rx)
{
  UWORD32 rfdiv;
  SYS_UWORD16 arfcn;

  #ifdef ARFCN_DEBUG
    trace_arfcn(arfcn);
  #endif

#if (L1_FF_MULTIBAND == 0)
  arfcn = Convert_l1_radio_freq(radio_freq);

  rf_index = arfcn_to_rf_index(arfcn);

  if (rf_index == 4)
  {
    rf_index = 0;
  }


#else
{
    UWORD8 rf_band_index;
//    rf_index = rf_band_idx_to_locosto_idx[rf_convert_l1freq_to_rf_band_idx(radio_freq)];
    arfcn=rf_convert_rffreq_to_l1freq_rfband(radio_freq, &rf_band_index);
    rf_index = rf_band_idx_to_locosto_idx[rf_subband2band[rf_band_index]];
    arfcn=rf_convert_l1freq_to_arfcn_rfband(arfcn, &rf_band_index);
}
#endif
  rfdiv = calc_rf_freq(arfcn, rx);

  MOVE_REG_TSP_TO_RF(rfdiv,((UWORD16)( ((UWORD32)(&drp_regs->RF_FREQL))&0xFFFF)));

  return(t);
}

/*------------------------------------------*/
/*              rf_init_light               */
/*------------------------------------------*/
/*    Initialization routine for PLL        */
/*   Effective downloading through TSP      */
/*------------------------------------------*/
WORD32 rf_init_light(WORD32 t)
{
  // initialization for change of multi-band configuration dependent on STD
  return(t);
}


/**************************************************************************/
/**************************************************************************/
/*                    EXTERNAL FUNCTIONS CALLED BY LAYER1                 */
/*                          COMMON TO L1 and TOOLKIT                      */
/**************************************************************************/
/**************************************************************************/

void l1dmacro_afc (SYS_UWORD16 afc_value, UWORD8 win_id)
{
	MOVE_REG_TSP_TO_RF(afc_value, ((UWORD16)( ((UWORD32)(&drp_srm_api->inout.afc.input.mem_xtal))&0xFFFF)));

	MOVE_REG_TSP_TO_RF(START_SCRIPT(DRP_AFC), ((UWORD16)( ((UWORD32)(&drp_regs->SCRIPT_STARTL))&0xFFFF)));
}


#define L1_NEW_ROC_ENABLE_FLAG  (1)


#if (L1_NEW_ROC_ENABLE_FLAG == 1)

/*------------------------------------------*/
/*    cust_get_if_dco_ctl_algo              */
/*------------------------------------------*/
/*      Defines which IF and DCO            */
/*      algorythms are used                 */
/*                                          */
/*                                          */
/*------------------------------------------*/
/* NOTE: In the below code
 *  At high power levels, IF_100KHZ_DSP-> DRP->LIF_100KHZ and DSP->DCO_IF_100KHZ/DCO_IF_0_100KHZ
 *   low power levels     IF_120KHZ_DSP-> DRP->LIF_120KHZ + HPF filter and DSP->DCO_NONE/DCO_IF_0KHZ */

void cust_get_if_dco_ctl_algo(UWORD16 *dco_algo_ctl, UWORD8 *if_ctrl,
    UWORD8 input_level_flag, UWORD8 input_level, UWORD16 radio_freq,
    UWORD8 if_threshold)
{
#if (L1_FF_MULTIBAND == 0)
  SYS_UWORD16 arfcn;
#else
  UWORD16 rffreq;
#endif

    if ((!input_level_flag) || (input_level < if_threshold))
	{
		*if_ctrl = IF_100KHZ_DSP;
    *dco_algo_ctl = DCO_IF_100KHZ;
	}
	else
	{
		*if_ctrl = IF_120KHZ_DSP;
		*dco_algo_ctl = DCO_NONE;
	}
#if (L1_FF_MULTIBAND == 0)
		arfcn = Convert_l1_radio_freq(radio_freq);

		switch(l1_config.std.id)
		{
		case GSM:
		case GSM_E:
    {
			if ((arfcn == 5) || (arfcn == 70))
			{
				if (*if_ctrl == IF_100KHZ_DSP)
					*dco_algo_ctl = DCO_IF_0KHZ_100KHZ;
				else
				*dco_algo_ctl = DCO_IF_0KHZ;
			}
			break;
    }
    case DCS1800:
    {
			if ((arfcn == 521) || (arfcn == 586) || (arfcn == 651) || (arfcn == 716) ||(arfcn == 781) ||  (arfcn == 846))
			{
				if (*if_ctrl == IF_100KHZ_DSP)
					*dco_algo_ctl = DCO_IF_0KHZ_100KHZ;
				else
			    *dco_algo_ctl = DCO_IF_0KHZ;
			}
			break;
    }
    case PCS1900:
    {
			if ((arfcn == 546) || (arfcn == 611) ||(arfcn == 676) ||  (arfcn == 741) ||(arfcn == 806) )
			{
				if (*if_ctrl == IF_100KHZ_DSP)
					*dco_algo_ctl = DCO_IF_0KHZ_100KHZ;
				else
				*dco_algo_ctl = DCO_IF_0KHZ;
			}
			break;
    }
    case GSM850:
    {
			if ((arfcn == 137) || (arfcn == 202))
			{
				if (*if_ctrl == IF_100KHZ_DSP)
					*dco_algo_ctl = DCO_IF_0KHZ_100KHZ;
				else
				*dco_algo_ctl = DCO_IF_0KHZ;
			}
			break;
    }
		 case DUAL:
		 case DUALEXT:
			 {
		      if (radio_freq < l1_config.std.first_radio_freq_band2)
				{
		      // GSM band...
				if ((arfcn == 5) ||(arfcn == 70))
					{
						if (*if_ctrl == IF_100KHZ_DSP)
							*dco_algo_ctl = DCO_IF_0KHZ_100KHZ;
						else
					*dco_algo_ctl = DCO_IF_0KHZ;
				}
				}
		      else
		     // DCS band...
				{
			     if ((arfcn == 521) || (arfcn == 586) || (arfcn == 651) || (arfcn == 716) ||(arfcn == 781) ||  (arfcn == 846))
				      {
						if (*if_ctrl == IF_100KHZ_DSP)
							*dco_algo_ctl = DCO_IF_0KHZ_100KHZ;
						else
					  *dco_algo_ctl = DCO_IF_0KHZ;
				}
				}
      break;
    }
		 case DUAL_US:
			{
		    if (radio_freq < l1_config.std.first_radio_freq_band2)
				{
				// GSM 850
				if ((arfcn == 137) || (arfcn == 202))
					{
						if (*if_ctrl == IF_100KHZ_DSP)
							*dco_algo_ctl = DCO_IF_0KHZ_100KHZ;
						else
					*dco_algo_ctl = DCO_IF_0KHZ;
				}
				}
			else
				{
				// PCS band...
					if ((arfcn == 546) || (arfcn == 611) ||(arfcn == 676) ||  (arfcn == 741) ||(arfcn == 806) )
						{
							if (*if_ctrl == IF_100KHZ_DSP)
								*dco_algo_ctl = DCO_IF_0KHZ_100KHZ;
							else
						*dco_algo_ctl = DCO_IF_0KHZ;
				}
				}
      break;
    }
    default:
      break;// should never occur.
  }  // end of switch
#else  //#if (L1_FF_MULTIBAND == 0)
  //rffreq = rf_convert_l1freq_to_rffreq(radio_freq);
  rffreq=radio_freq; // The argument passed to this function is the radio_freq and not l1_freq
  if(
#if (GSM850_SUPPORTED == 1)
    (137 == rffreq)||(202 == rffreq) ||
#endif
#if (GSM900_SUPPORTED == 1)
    (5 == rffreq)||(70 == rffreq) ||
#endif
#if (DCS1800_SUPPORTED == 1)
    (521 == rffreq)||(586 == rffreq) ||(651 == rffreq)||(716 == rffreq) ||(781 == rffreq)||(846 == rffreq) ||
#endif
#if (PCS1900_SUPPORTED == 1)
    (546+512 == rffreq) ||(611+512 == rffreq)||(676+512 == rffreq) ||(741+512 == rffreq)||(806+512 == rffreq) ||
#endif
  0)
  {
    if (*if_ctrl == IF_100KHZ_DSP)
      *dco_algo_ctl = DCO_IF_0KHZ_100KHZ;
    else
      *dco_algo_ctl = DCO_IF_0KHZ;
  }
#endif // if (L1_FF_MULTIBAND == 0)
}
#else

/*------------------------------------------*/
/*    cust_get_if_dco_ctl_algo              */
/*------------------------------------------*/
/*      Defines which IF and DCO            */
/*      algorythms are used                 */
/*                                          */
/*                                          */
/*------------------------------------------*/

/* NOTE: Below is the Old DCO algorithm in which ROC compensation was not enabled on DSP side
 * To use this algorithm we need an appropriate DRP script which functions as below.
 *   IF_100KHZ_DSP-> DRP-ZIF and DSP-DCO_ZIF
 *   IF_120KHZ_DSP-> DRP_LIF_120KHZ + HPF filter and DSP-DCO_ZIF/DCO_NONE */
void cust_get_if_dco_ctl_algo(UWORD16 *dco_algo_ctl, UWORD8 *if_ctrl, UWORD8 input_level_flag, UWORD8 input_level, UWORD16 radio_freq, UWORD8 if_threshold)
{
#if (L1_FF_MULTIBAND == 0)
  SYS_UWORD16 arfcn;
#else
  UWORD16 rffreq;
#endif

  if ((!input_level_flag) | (input_level < if_threshold))
  {
    *if_ctrl = IF_100KHZ_DSP;
    *dco_algo_ctl = DCO_IF_0KHZ;
  }
  else{
    *if_ctrl = IF_120KHZ_DSP;



    //*dco_algo_ctl = DCO_IF_100KHZ;
    *dco_algo_ctl = DCO_NONE;
#if (L1_FF_MULTIBAND == 0)

    arfcn = Convert_l1_radio_freq(radio_freq);

    switch(l1_config.std.id)
    {
    case GSM:
    case GSM_E:
      if ((arfcn == 5) |(arfcn == 70))

        *dco_algo_ctl = DCO_IF_0KHZ;
      break;

    case DCS1800:
      if ((arfcn == 521) | (arfcn == 586) | (arfcn == 651) | (arfcn == 716) |(arfcn == 781) |  (arfcn == 846))

          *dco_algo_ctl = DCO_IF_0KHZ;
      break;

    case PCS1900:
      if ((arfcn == 546) | (arfcn == 611) |(arfcn == 676) |  (arfcn == 741) |(arfcn == 806) )

        *dco_algo_ctl = DCO_IF_0KHZ;
      break;

    case GSM850:
      if ((arfcn == 137) | (arfcn == 202))

        *dco_algo_ctl = DCO_IF_0KHZ;
      break;

     case DUAL:
     case DUALEXT:
       {
          if (radio_freq < l1_config.std.first_radio_freq_band2)
        {
          // GSM band...
        if ((arfcn == 5) |(arfcn == 70))

          *dco_algo_ctl = DCO_IF_0KHZ;
        }
          else
         // DCS band...
        {
           if ((arfcn == 521) | (arfcn == 586) | (arfcn == 651) | (arfcn == 716) |(arfcn == 781) |  (arfcn == 846))

            *dco_algo_ctl = DCO_IF_0KHZ;
        }
       }
       break;

     case DUAL_US:
      {
        if (radio_freq < l1_config.std.first_radio_freq_band2)
        {
        // GSM 850
        if ((arfcn == 137) | (arfcn == 202))

          *dco_algo_ctl = DCO_IF_0KHZ;
        }
      else
        {
        // PCS band...
          if ((arfcn == 546) | (arfcn == 611) |(arfcn == 676) |  (arfcn == 741) |(arfcn == 806) )

            *dco_algo_ctl = DCO_IF_0KHZ;
        }
      }
      break;

    default:
      break;// should never occur.
    }  // end of switch
#else
  rffreq = rf_convert_l1freq_to_rffreq(radio_freq);
  if(
#if (GSM850_SUPPORTED == 1)
    (137 == rffreq)||(202 == rffreq) ||
#endif
#if (GSM900_SUPPORTED == 1)
    (5 == rffreq)||(70 == rffreq) ||
#endif
#if (DCS1800_SUPPORTED == 1)
    (521 == rffreq)||(586 == rffreq) ||(651 == rffreq)||(716 == rffreq) ||(781 == rffreq)||(846 == rffreq) ||
#endif
#if (PCS1900_SUPPORTED == 1)
    (546+512 == rffreq) ||(611+512 == rffreq)||(676+512 == rffreq) ||(741+512 == rffreq)||(806+512 == rffreq) ||
#endif
  0)
  {
    *dco_algo_ctl = DCO_IF_0KHZ;
  }
#endif
   }
}

#endif
/*------------------------------------------*/
/*                agc                       */
/*------------------------------------------*/
/*      Program a gain into IF amp          */
/*      agc_value : gain in dB              */
/*                                          */
/*   additional parameter for LNA setting   */
/*------------------------------------------*/

void l1dmacro_agc(SYS_UWORD16 radio_freq, WORD8 gain, UWORD8 lna_off, UWORD8 if_ctl)
{
   signed int index;
   WORD16 afe;
   UWORD16 corner_freq = SCF_270KHZ ;      //Corner frequency given in kHz
   UWORD16 gain_comp = GAIN_COMP_ENABLE;   //gain compensation scheme
   UWORD16 if_setting;
   UWORD16 lna_setting;

   UWORD16 arfcn ;


   index = gain;

   // below is inserted to prevent wraparound of gain index in testmode
   if (index >= AGC_TABLE_SIZE)
     index = AGC_TABLE_SIZE-1;
   if (index <= MIN_AGC_INDEX)
     index = MIN_AGC_INDEX;

   if(lna_off)
   	 afe = AFE_LOW_GAIN;
   else
     afe = AFE_HIGH_GAIN;

    if(if_ctl == IF_120KHZ_DSP)
		 if_setting = IF_120KHZ_DRP;
   else
		 if_setting = IF_100KHZ_DRP;


#if (L1_FF_MULTIBAND == 0)
   //LNA Changes
    arfcn = Convert_l1_radio_freq(radio_freq);
    //band_system_index = (UWORD16)    convert_arfcn_to_band(arfcn);
    lna_setting = (UWORD16) drp_generate_dbbif_setting_arfcn( (UWORD16) RF_BAND_SYSTEM_INDEX,  arfcn);
   //End LNA
#else
#if 0
  rf_band_idx = rf_convert_l1freq_to_rf_band_idx(radio_freq);
  switch(rf_band_idx)
  {
#if(GSM850_SUPPORTED)
    case RF_GSM850:
      drp_band_index = 0;
      break;
#endif
#if(DCS1800_SUPPORTED)
  case RF_DCS1800:
    drp_band_index = 2;
    break;
#endif
#if(PCS1900_SUPPORTED)
  case RF_PCS1900:
    drp_band_index = 3;
    break;
#endif
  default:
    drp_band_index = 1;
    break;
  }
#endif // if 0

  lna_setting = (UWORD16) drp_generate_dbbif_setting_arfcn( (UWORD16) RF_BAND_SYSTEM_INDEX,  radio_freq);
#endif
	//if_setting = IF_100KHZ_DRP;
   // r2: implement the register rx_in for setting the configuration of the RX path
   *TP_Ptr++ = TPU_AT(TRF_R2);
   MOVE_REG_TSP_TO_RF((  (lna_setting <<8) | (corner_freq<<7) | (afe<<6) | (AGC_TABLE[index]<<2) | (gain_comp<<1) | (if_setting)), ((UWORD16)( ((UWORD32)(&drp_srm_api->inout.rx.rxon_input))&0xFFFF)));

}

/*------------------------------------------*/
/*             l1dmacro_rx_synth            */
/*------------------------------------------*/
/*       programs RF synth for recceive     */
/*------------------------------------------*/
void l1dmacro_rx_synth(SYS_UWORD16 radio_freq)
{
   UWORD32 t;

   // Important: always use rx_synth_start_time for first TPU_AT
   // Never remove below 2 lines!!!
   t = l1_config.params.rx_synth_start_time;
   *TP_Ptr++ = TPU_FAT (t);

   t = rf_program(t, radio_freq, 1);   // direction is set to 1 for Rx
}

/*------------------------------------------*/
/*            l1dmacro_tx_synth             */
/*------------------------------------------*/
/*      programs RF synth for transmit      */
/*      programs OPLL for transmit          */
/*------------------------------------------*/
void l1dmacro_tx_synth(SYS_UWORD16 radio_freq)
{
   UWORD32 t;

   // Important: always use tx_synth_start_time for first TPU_AT
   // Never remove below 2 lines!!!
   t =   l1_config.params.tx_synth_start_time;
   *TP_Ptr++ = TPU_FAT (t);

   t = rf_program(t, radio_freq, 0); // direction set to 0 for Tx
}

/*------------------------------------------*/
/*            l1dmacro_rx_up                */
/*------------------------------------------*/
/* Open window for normal burst reception   */
/*------------------------------------------*/
#if (L1_RF_KBD_FIX == 1)
#if (L1_MADC_ON == 0)
void l1dmacro_rx_up (UWORD8 csf_filter_choice, UWORD8 kbd_config
										#if(NEW_SNR_THRESHOLD==1)
											, UWORD8 saic_flag_rx_up
										#endif
										)
{
 UWORD8 kbd_tspact_config =0;

 if (kbd_config == KBD_DISABLED)
	kbd_tspact_config =  KBD_DIS_TSPACT;

  // r3: power ON RX
  *TP_Ptr++ = TPU_AT(TRF_R3);
  MOVE_REG_TSP_TO_RF(START_SCRIPT(DRP_RX_ON), ((UWORD16)( ((UWORD32)(&drp_regs->SCRIPT_STARTL)))));

  // r3_1: disable keyboard
  *TP_Ptr++ = TPU_AT(TRF_R3_1);
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, kbd_tspact_config);

  // r4: enable TXM in Rx mode
  *TP_Ptr++ = TPU_AT(TRF_R4);
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U, rf_path[rf_index].rx_up);

  // Program CSF filter appropriately
  if(csf_filter_choice == L1_SAIC_HARDWARE_FILTER)
  	{
    MOVE_REG_TSP_TO_RF(CSF_CWL_HARDWARE_FILTER_64TAP, (UWORD16)(&drp_regs->CSF_CWL));
  	}
  else
  	{
    MOVE_REG_TSP_TO_RF(CSF_CWL_PROGRAMMABLE_FILTER_64TAP, (UWORD16)(&drp_regs->CSF_CWL));
  	}

  // r5: enable RX_START
  #if (L1_SAIC != 0)//Because for 0/2 interpolation, SAIC needs 1 additional symbol compared to legacy modem.

  #if (NEW_SNR_THRESHOLD==1)
	if(saic_flag_rx_up==1)
		{
  #endif

  #if (ONE_THIRD_INTRPOL == 1)
    *TP_Ptr++ = TPU_AT(TRF_R5-5);
  #else
    *TP_Ptr++ = TPU_AT(TRF_R5-4);
  #endif

  #if (NEW_SNR_THRESHOLD==1)
		}
	else
		{
		*TP_Ptr++ = TPU_AT(TRF_R5);
		}
  #endif

  #else
    *TP_Ptr++ = TPU_AT(TRF_R5);
  #endif

  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, kbd_tspact_config | RX_START);

}
#endif


#if (L1_MADC_ON == 1)
void l1dmacro_rx_up(UWORD8 adc_active, UWORD8 csf_filter_choice, UWORD8 kbd_config
										#if(NEW_SNR_THRESHOLD==1)
											, UWORD8 saic_flag_rx_up
										#endif
										)
{
	UWORD8 kbd_tspact_config =0;

	if (kbd_config == KBD_DISABLED)
		kbd_tspact_config =  KBD_DIS_TSPACT;

  // r3: power ON RX
  *TP_Ptr++ = TPU_AT(TRF_R3);
  MOVE_REG_TSP_TO_RF(START_SCRIPT(DRP_RX_ON), (UWORD32)(&drp_regs->SCRIPT_STARTL));

  if (adc_active == ACTIVE)
  {
        *TP_Ptr++ = TPU_AT(TRF_R8);
        *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U,START_ADC|TXM_SLEEP);
        *TP_Ptr++ = TPU_WAIT  (2);
        *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U,TXM_SLEEP);
  }

  // r3_1: disable keyboard
  *TP_Ptr++ = TPU_AT(TRF_R3_1);
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, kbd_tspact_config);

  // r4: enable TXM in Rx mode
  *TP_Ptr++ = TPU_AT(TRF_R4);
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U, rf_path[rf_index].rx_up);

  // Program CSF filter appropriately
  if(csf_filter_choice == L1_SAIC_HARDWARE_FILTER)
  	{
    MOVE_REG_TSP_TO_RF(CSF_CWL_HARDWARE_FILTER_64TAP, (UWORD32)(&drp_regs->CSF_CWL));
  	}
  else
  	{
    MOVE_REG_TSP_TO_RF(CSF_CWL_PROGRAMMABLE_FILTER_64TAP, (UWORD32)(&drp_regs->CSF_CWL));
  }
  // r5: enable RX_START
  // Remember that between TRF_R5 and TRF_R4 there should be a buffer of around 4 qbits
  #if (L1_SAIC != 0)//Because for 0/2 interpolation, SAIC needs 1 additional symbol compared to legacy modem.

  #if (NEW_SNR_THRESHOLD==1)
	if(saic_flag_rx_up==1)
		{
  #endif

  #if (ONE_THIRD_INTRPOL == 1)
    *TP_Ptr++ = TPU_AT(TRF_R5-5);
  #else
    *TP_Ptr++ = TPU_AT(TRF_R5-4);
  #endif

  #if (NEW_SNR_THRESHOLD==1)
		}
	else
		{
		*TP_Ptr++ = TPU_AT(TRF_R5);
		}
  #endif

  #else
    *TP_Ptr++ = TPU_AT(TRF_R5);
  #endif
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, kbd_tspact_config | RX_START);

}
#endif

#endif /* (L1_RF_KBD_FIX == 1) */

#if (L1_RF_KBD_FIX == 0)
#if (L1_MADC_ON == 0)
void l1dmacro_rx_up (UWORD8 csf_filter_choice
								#if(NEW_SNR_THRESHOLD==1)
									, UWORD8 saic_flag_rx_up
								#endif
								)
{

  // r3: power ON RX
  *TP_Ptr++ = TPU_AT(TRF_R3);
  MOVE_REG_TSP_TO_RF(START_SCRIPT(DRP_RX_ON), (UWORD16)(&drp_regs->SCRIPT_STARTL));

  // r4: enable TXM in Rx mode
  *TP_Ptr++ = TPU_AT(TRF_R4);
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U, rf_path[rf_index].rx_up);

  // Program CSF filter appropriately
  if(csf_filter_choice == L1_SAIC_HARDWARE_FILTER)
  	{
    MOVE_REG_TSP_TO_RF(CSF_CWL_HARDWARE_FILTER_64TAP, (UWORD16)(&drp_regs->CSF_CWL));
   	}
  else
  	{
    MOVE_REG_TSP_TO_RF(CSF_CWL_PROGRAMMABLE_FILTER_64TAP, (UWORD16)(&drp_regs->CSF_CWL));
  	}

  // r5: enable RX_START
  #if (L1_SAIC != 0)//Because for 0/2 interpolation, SAIC needs 1 additional symbol compared to legacy modem.

  #if (NEW_SNR_THRESHOLD==1)
	if(saic_flag_rx_up==1)
		{
  #endif

  #if (ONE_THIRD_INTRPOL == 1)
    *TP_Ptr++ = TPU_AT(TRF_R5-5);
  #else
    *TP_Ptr++ = TPU_AT(TRF_R5-4);
  #endif

  #if (NEW_SNR_THRESHOLD==1)
		}
	else
		{
		*TP_Ptr++ = TPU_AT(TRF_R5);
		}
  #endif

  #else
    *TP_Ptr++ = TPU_AT(TRF_R5);
  #endif
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, RX_START);

}
#endif


#if (L1_MADC_ON == 1)
void l1dmacro_rx_up(UWORD8 adc_active, UWORD8 csf_filter_choice
										#if(NEW_SNR_THRESHOLD==1)
											, UWORD8 saic_flag_rx_up
										#endif
										)
{
  // r3: power ON RX
  *TP_Ptr++ = TPU_AT(TRF_R3);
  MOVE_REG_TSP_TO_RF(START_SCRIPT(DRP_RX_ON), (UWORD16)(&drp_regs->SCRIPT_STARTL));

  if (adc_active == ACTIVE)
  {
        *TP_Ptr++ = TPU_AT(TRF_R8);
        *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U,START_ADC);
        *TP_Ptr++ = TPU_WAIT  (2);
         *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U,0);
  }


  // r4: enable TXM in Rx mode
  *TP_Ptr++ = TPU_AT(TRF_R4);
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U, rf_path[rf_index].rx_up);

  // Program CSF filter appropriately
  if(csf_filter_choice == L1_SAIC_HARDWARE_FILTER)
  	{
    MOVE_REG_TSP_TO_RF(CSF_CWL_HARDWARE_FILTER_64TAP, (UWORD16)(&drp_regs->CSF_CWL));
   	}
  else
  	{
    MOVE_REG_TSP_TO_RF(CSF_CWL_PROGRAMMABLE_FILTER_64TAP, (UWORD16)(&drp_regs->CSF_CWL));
  	}

  // r5: enable RX_START
  #if (L1_SAIC != 0)//Because for 0/2 interpolation, SAIC needs 1 additional symbol compared to legacy modem.

  #if (NEW_SNR_THRESHOLD==1)
	if(saic_flag_rx_up==1)
		{
  #endif

  #if (ONE_THIRD_INTRPOL == 1)
    *TP_Ptr++ = TPU_AT(TRF_R5-5);
  #else
    *TP_Ptr++ = TPU_AT(TRF_R5-4);
  #endif

  #if (NEW_SNR_THRESHOLD==1)
		}
	else
		{
		*TP_Ptr++ = TPU_AT(TRF_R5);
		}
  #endif

  #else
    *TP_Ptr++ = TPU_AT(TRF_R5);
  #endif
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, RX_START);

}
#endif
#endif /* (L1_RF_KBD_FIX == 0) */


/*------------------------------------------*/
/*            l1pdmacro_rx_down             */
/*------------------------------------------*/
/* Close window for normal burst reception  */
/*------------------------------------------*/
void l1dmacro_rx_down (WORD32 t)
{

    //r6: Disable ROC script
      *TP_Ptr++ = TPU_FAT(t + TRF_R6);
   MOVE_REG_TSP_TO_RF((DRP_ROC), ((UWORD16)( ((UWORD32)(&drp_regs->SCRIPT_STARTL))&0xFFFF)));



  //r7: Disable Rx_Start & Disable RF switch & send Idle script
  *TP_Ptr++ = TPU_FAT(t + TRF_R7);
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U, rf_path[rf_index].rx_down);
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, 0);
  MOVE_REG_TSP_TO_RF(START_SCRIPT(DRP_IDLE),((UWORD16)( ((UWORD32)(&drp_regs->SCRIPT_STARTL))&0xFFFF)));

}

/*------------------------------------------*/
/*            l1dmacro_tx_up                */
/*------------------------------------------*/
/* Open transmission window for normal burst*/
/*------------------------------------------*/
#if (L1_RF_KBD_FIX == 1)
void l1dmacro_tx_up (UWORD8 kbd_config)
{
  UWORD8 kbd_tspact_config =0;

  if (kbd_config == KBD_DISABLED)
		kbd_tspact_config =  KBD_DIS_TSPACT;

  // t2: Power ON TX
  *TP_Ptr++ = TPU_AT(TRF_T2);
  MOVE_REG_TSP_TO_RF(START_SCRIPT(DRP_TX_ON), ((UWORD16)( ((UWORD32)(&drp_regs->SCRIPT_STARTL))&0xFFFF)));

   // t3: put the TXM in RX mode
  *TP_Ptr++ = TPU_AT(TRF_T3);
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U, rf_path[rf_index].tx_down);

  // t3_1: disable keyboard
  *TP_Ptr++ = TPU_AT(TRF_T3_1);
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, kbd_tspact_config);

 // t4: enable the APC LDO
  *TP_Ptr++ = TPU_AT(TRF_T4);
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, kbd_tspact_config | LDO_EN);

  // t5: enable the APC module
  *TP_Ptr++ = TPU_AT(TRF_T5);
 //SG *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, kbd_tspact_config | LDO_EN | APC_EN);

  // t6: enable TX start and enable of Vramp
  //SG*TP_Ptr++ = TPU_AT(TRF_T6);
  //SG *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, kbd_tspact_config | LDO_EN | APC_EN | TX_START | START_APC);
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, kbd_tspact_config | LDO_EN | TX_START );

  *TP_Ptr++ = TPU_AT(TRF_T6);
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, kbd_tspact_config | LDO_EN | APC_EN | TX_START );

  *TP_Ptr++ = TPU_AT(TRF_T7);
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, kbd_tspact_config |  LDO_EN | APC_EN | TX_START | START_APC);

  // t7: enable TX start and enable of Vramp - Internal mode
  //*TP_Ptr++ = TPU_AT(TRF_T7);
  //*TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, LDO_EN | APC_EN | TX_START | START_APC );

  // t7: enable TX start and enable of Vramp
  //*TP_Ptr++ = TPU_AT(TRF_T7+4);
  //*TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, LDO_EN | APC_EN | TX_START );

  // t8: enable the TXEN of the TXM
  *TP_Ptr++ = TPU_AT(TRF_T8);
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U, rf_path[rf_index].tx_up);

 }

 #endif /*(L1_RF_KBD_FIX == 1)*/

 #if (L1_RF_KBD_FIX == 0)

 void l1dmacro_tx_up (void)
 {





   // t2: Power ON TX
   *TP_Ptr++ = TPU_AT(TRF_T2);
   MOVE_REG_TSP_TO_RF(START_SCRIPT(DRP_TX_ON),((UWORD16)( ((UWORD32)(&drp_regs->SCRIPT_STARTL))&0xFFFF)));

    // t3: put the TXM in RX mode
   *TP_Ptr++ = TPU_AT(TRF_T3);
   *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U, rf_path[rf_index].tx_down);





  // t4: enable the APC LDO
   *TP_Ptr++ = TPU_AT(TRF_T4);
   *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, LDO_EN);

   // t5: enable the APC module
   *TP_Ptr++ = TPU_AT(TRF_T5);
  //SG *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, LDO_EN | APC_EN);

   // t6: enable TX start and enable of Vramp
   //SG*TP_Ptr++ = TPU_AT(TRF_T6);
   //SG *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, LDO_EN | APC_EN | TX_START | START_APC);
   *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, LDO_EN | TX_START );

   *TP_Ptr++ = TPU_AT(TRF_T6);
   *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, LDO_EN | APC_EN | TX_START );

   *TP_Ptr++ = TPU_AT(TRF_T7);
   *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, LDO_EN | APC_EN | TX_START | START_APC);

   // t7: enable TX start and enable of Vramp - Internal mode
   //*TP_Ptr++ = TPU_AT(TRF_T7);
   //*TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, LDO_EN | APC_EN | TX_START | START_APC );

   // t7: enable TX start and enable of Vramp
   //*TP_Ptr++ = TPU_AT(TRF_T7+4);
   //*TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, LDO_EN | APC_EN | TX_START );

   // t8: enable the TXEN of the TXM
   *TP_Ptr++ = TPU_AT(TRF_T8);
   *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U, rf_path[rf_index].tx_up);

  }

#endif /* (L1_RF_KBD_FIX == 0)*/

/*-------------------------------------------*/
/*            l1dmacro_tx_down               */
/*-------------------------------------------*/
/* Close transmission window for normal burst*/
/*-------------------------------------------*/
#if (L1_RF_KBD_FIX == 1)
void l1dmacro_tx_down (WORD32 t, BOOL tx_flag, UWORD8 adc_active, UWORD8 kbd_config)
{
  UWORD8 kbd_tspact_config =0;

  if (kbd_config == KBD_DISABLED)
		kbd_tspact_config =  KBD_DIS_TSPACT;


  if (adc_active == ACTIVE) {
    // 36qbits = (10qbits for  TPU programming) + (26qbits duration to convert the first ADC channel (= Battery))
    	if ((t)<(TRF_T8+2-TRF_T9))		//Done to enable RACH Burst Support
    		{
    		l1dmacro_adc_read_tx (TRF_T8+10, rf_path[rf_index].tx_up);
    		}
	else
		{
    l1dmacro_adc_read_tx (t + TRF_T9, rf_path[rf_index].tx_up);
  }
  }



  // t10: disable APC
  *TP_Ptr++ = TPU_FAT (t + TRF_T10);
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, kbd_tspact_config | LDO_EN | APC_EN | TX_START );

  // t11: disable PA
  *TP_Ptr++ = TPU_FAT (t + TRF_T11);
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U, rf_path[rf_index].tx_down);
	// disable Tx_Start
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, kbd_tspact_config | LDO_EN | APC_EN );

  // t12: power off Locosto: IDLE SCRIPT
  *TP_Ptr++ = TPU_FAT (t + TRF_T12);
  MOVE_REG_TSP_TO_RF(START_SCRIPT(DRP_IDLE), ((UWORD16)( ((UWORD32)(&drp_regs->SCRIPT_STARTL))&0xFFFF)));

  // t13: Switch off APC
  *TP_Ptr++ = TPU_FAT (t + TRF_T13);
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, 0);

  }
#endif /*(L1_RF_KBD_FIX == 1)*/



#if (L1_RF_KBD_FIX == 0)
void l1dmacro_tx_down (WORD32 t, BOOL tx_flag, UWORD8 adc_active)
{

  if (adc_active == ACTIVE) {
    // 36qbits = (10qbits for  TPU programming) + (26qbits duration to convert the first ADC channel (= Battery))
    l1dmacro_adc_read_tx (t + TRF_T9, rf_path[rf_index].tx_up);
  }

   // t10: disable APC
  *TP_Ptr++ = TPU_FAT (t + TRF_T10);
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, kbd_tspact_config | LDO_EN | APC_EN | TX_START );

  // t11: disable PA
  *TP_Ptr++ = TPU_FAT (t + TRF_T11);
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U, rf_path[rf_index].tx_down);
	// disable Tx_Start
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, kbd_tspact_config | LDO_EN | APC_EN );

  // t12: power off Locosto: IDLE SCRIPT
  *TP_Ptr++ = TPU_FAT (t + TRF_T12);
  MOVE_REG_TSP_TO_RF(START_SCRIPT(DRP_IDLE), (UWORD16)(&drp_regs->SCRIPT_STARTL));

  // t13: Switch off APC
  *TP_Ptr++ = TPU_FAT (t + TRF_T13);
  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, 0);

}


#endif/*(L1_RF_KBD_FIX == 0)*/

/*
 * l1dmacro_rx_nb
 *
 * Receive Normal burst
 */
#if (L1_RF_KBD_FIX == 1)
 #if (L1_MADC_ON == 1)
void l1dmacro_rx_nb (SYS_UWORD16 radio_freq, UWORD8 adc_active, UWORD8 csf_filter_choice
										#if(NEW_SNR_THRESHOLD==1)
											, UWORD8 saic_flag_rx_up
										#endif
										)
{
	l1dmacro_rx_up(adc_active, csf_filter_choice, L1_KBD_DIS_RX_NB
										#if(NEW_SNR_THRESHOLD==1)
											, saic_flag_rx_up
										#endif
										);
    l1dmacro_rx_down (STOP_RX_SNB);
	l1s.total_kbd_on_time = l1s.total_kbd_on_time - L1_KBD_DIS_RX_NB * (-TRF_R3_1 + STOP_RX_SNB - TRF_R7);
}
#else
void l1dmacro_rx_nb (SYS_UWORD16 radio_freq,UWORD8 csf_filter_choice
										#if(NEW_SNR_THRESHOLD==1)
											, UWORD8 saic_flag_rx_up
										#endif
										)
{
  l1dmacro_rx_up(csf_filter_choice, L1_KBD_DIS_RX_NB
										#if(NEW_SNR_THRESHOLD==1)
											, saic_flag_rx_up
										#endif
										);
  l1dmacro_rx_down (STOP_RX_SNB);
  l1s.total_kbd_on_time = l1s.total_kbd_on_time - L1_KBD_DIS_RX_NB * (-TRF_R3_1 + STOP_RX_SNB - TRF_R7);
}
#endif
#endif /*(L1_RF_KBD_FIX == 1)*/

#if (L1_RF_KBD_FIX == 0)
 #if (L1_MADC_ON == 1)
void l1dmacro_rx_nb (SYS_UWORD16 radio_freq, UWORD8 adc_active, UWORD8 csf_filter_choice
										#if(NEW_SNR_THRESHOLD==1)
											, UWORD8 saic_flag_rx_up
										#endif
										)
{
    l1dmacro_rx_up(adc_active, csf_filter_choice
										#if(NEW_SNR_THRESHOLD==1)
											, saic_flag_rx_up
										#endif
										);
    l1dmacro_rx_down (STOP_RX_SNB);

}
#else
void l1dmacro_rx_nb (SYS_UWORD16 radio_freq,UWORD8 csf_filter_choice
										#if(NEW_SNR_THRESHOLD==1)
											, UWORD8 saic_flag_rx_up
										#endif
										)
{
  l1dmacro_rx_up(csf_filter_choice
							#if(NEW_SNR_THRESHOLD==1)
								, saic_flag_rx_up
							#endif
										);
  l1dmacro_rx_down (STOP_RX_SNB);

}
#endif

#endif/*(L1_RF_KBD_FIX == 0)*/



/*
 * l1dmacro_rx_sb
 * Receive Synchro burst
*/
#if (L1_RF_KBD_FIX == 1)
#if (L1_MADC_ON == 1)
void l1dmacro_rx_sb (SYS_UWORD16 radio_freq,UWORD8 adc_active)
{
  l1dmacro_rx_up(adc_active, L1_SAIC_HARDWARE_FILTER, L1_KBD_DIS_RX_SB
  	#if(NEW_SNR_THRESHOLD==1)
	, SAIC_OFF
	#endif
	);

  l1dmacro_rx_down (STOP_RX_SB);
  l1s.total_kbd_on_time = l1s.total_kbd_on_time - L1_KBD_DIS_RX_SB * (-TRF_R3_1 + STOP_RX_SB - TRF_R7);
}

#else
void l1dmacro_rx_sb (SYS_UWORD16 radio_freq)
{
  l1dmacro_rx_up(L1_SAIC_HARDWARE_FILTER, L1_KBD_DIS_RX_SB
  	#if(NEW_SNR_THRESHOLD==1)
	, SAIC_OFF
	#endif
	);
  l1dmacro_rx_down (STOP_RX_SB);
  l1s.total_kbd_on_time = l1s.total_kbd_on_time - L1_KBD_DIS_RX_SB * (-TRF_R3_1 + STOP_RX_SB - TRF_R7);
}
#endif

#endif/*(L1_RF_KBD_FIX == 1)*/

#if(L1_RF_KBD_FIX == 0)
#if (L1_MADC_ON == 1)
void l1dmacro_rx_sb (SYS_UWORD16 radio_freq,UWORD8 adc_active)
{
  l1dmacro_rx_up(adc_active, L1_SAIC_HARDWARE_FILTER
  	#if(NEW_SNR_THRESHOLD==1)
	, SAIC_OFF
	#endif
	);
  l1dmacro_rx_down (STOP_RX_SB);

}

#else
void l1dmacro_rx_sb (SYS_UWORD16 radio_freq)
{
  l1dmacro_rx_up(L1_SAIC_HARDWARE_FILTER
  	#if(NEW_SNR_THRESHOLD==1)
	, SAIC_OFF
	#endif
	);
  l1dmacro_rx_down (STOP_RX_SB);

}
#endif

#endif/*(L1_RF_KBD_FIX == 0)*/

/*
 * l1dmacro_rx_ms
 *
 * Receive Power Measurement window
 */
 #if(L1_RF_KBD_FIX == 1)
 #if (L1_MADC_ON == 1)
 void l1dmacro_rx_ms (SYS_UWORD16 radio_freq,UWORD8 adc_active)
{
  l1dmacro_rx_up(adc_active, L1_SAIC_HARDWARE_FILTER, L1_KBD_DIS_RX_MS
  	#if(NEW_SNR_THRESHOLD==1)
	, SAIC_OFF
	#endif
	);
  l1dmacro_rx_down (STOP_RX_PW_1);
  l1s.total_kbd_on_time = l1s.total_kbd_on_time - L1_KBD_DIS_RX_MS * (-TRF_R3_1 + STOP_RX_PW_1 - TRF_R7);
}

 #else
void l1dmacro_rx_ms (SYS_UWORD16 radio_freq)
{
  l1dmacro_rx_up(L1_SAIC_HARDWARE_FILTER, L1_KBD_DIS_RX_MS
  	#if(NEW_SNR_THRESHOLD==1)
	, SAIC_OFF
	#endif
	);
  l1dmacro_rx_down (STOP_RX_PW_1);
  l1s.total_kbd_on_time = l1s.total_kbd_on_time - L1_KBD_DIS_RX_MS * (-TRF_R3_1 + STOP_RX_PW_1 - TRF_R7);
}
#endif
#endif/*(L1_RF_KBD_FIX == 1)*/

#if(L1_RF_KBD_FIX == 0)
#if (L1_MADC_ON == 1)
 void l1dmacro_rx_ms (SYS_UWORD16 radio_freq,UWORD8 adc_active)
{
  l1dmacro_rx_up(adc_active, L1_SAIC_HARDWARE_FILTER
  	#if(NEW_SNR_THRESHOLD==1)
	, SAIC_OFF
	#endif
	);
  l1dmacro_rx_down (STOP_RX_PW_1);

}

 #else
void l1dmacro_rx_ms (SYS_UWORD16 radio_freq)
{
  l1dmacro_rx_up(L1_SAIC_HARDWARE_FILTER
  	#if(NEW_SNR_THRESHOLD==1)
	, SAIC_OFF
	#endif
	);
  l1dmacro_rx_down (STOP_RX_PW_1);

}
#endif

#endif/*(L1_RF_KBD_FIX == 0)*/

/*
 * l1dmacro_rx_fb
 *
 * Receive Frequency burst
 */
#if(L1_RF_KBD_FIX == 1)
#if (L1_MADC_ON == 1)
void l1dmacro_rx_fb (SYS_UWORD16 radio_freq,UWORD8 adc_active)
#else
void l1dmacro_rx_fb (SYS_UWORD16 radio_freq)
#endif
{
#if (L1_MADC_ON == 1)
  l1dmacro_rx_up(adc_active, L1_SAIC_HARDWARE_FILTER, L1_KBD_DIS_RX_FB
  	#if(NEW_SNR_THRESHOLD==1)
	, SAIC_OFF
	#endif
	);
#else
  l1dmacro_rx_up(L1_SAIC_HARDWARE_FILTER, L1_KBD_DIS_RX_FB
  	#if(NEW_SNR_THRESHOLD==1)
	, SAIC_OFF
	#endif
	);
#endif
  l1s.total_kbd_on_time = 5000;
  *TP_Ptr++ = TPU_AT(0);
  *TP_Ptr++ = TPU_AT(0);
  *TP_Ptr++ = TPU_AT(0);
  *TP_Ptr++ = TPU_AT(0);
  *TP_Ptr++ = TPU_AT(0);
  *TP_Ptr++ = TPU_AT(0);
  *TP_Ptr++ = TPU_AT(0);
  *TP_Ptr++ = TPU_AT(0);
  *TP_Ptr++ = TPU_AT(0);
  *TP_Ptr++ = TPU_AT(0);
  *TP_Ptr++ = TPU_AT(0);

  l1dmacro_rx_down (STOP_RX_FB);
  l1s.total_kbd_on_time = l1s.total_kbd_on_time - L1_KBD_DIS_RX_FB * (STOP_RX_FB - TRF_R7);
}
#endif/*(L1_RF_KBD_FIX == 1)*/

#if(L1_RF_KBD_FIX == 0)
#if (L1_MADC_ON == 1)
void l1dmacro_rx_fb (SYS_UWORD16 radio_freq,UWORD8 adc_active)
#else
void l1dmacro_rx_fb (SYS_UWORD16 radio_freq)
#endif
{
#if (L1_MADC_ON == 1)
  l1dmacro_rx_up(adc_active, L1_SAIC_HARDWARE_FILTER
  	#if(NEW_SNR_THRESHOLD==1)
	, SAIC_OFF
	#endif
	);
#else
  l1dmacro_rx_up(L1_SAIC_HARDWARE_FILTER
  	#if(NEW_SNR_THRESHOLD==1)
	, SAIC_OFF
	#endif
	);
#endif

  *TP_Ptr++ = TPU_AT(0);
  *TP_Ptr++ = TPU_AT(0);
  *TP_Ptr++ = TPU_AT(0);
  *TP_Ptr++ = TPU_AT(0);
  *TP_Ptr++ = TPU_AT(0);
  *TP_Ptr++ = TPU_AT(0);
  *TP_Ptr++ = TPU_AT(0);
  *TP_Ptr++ = TPU_AT(0);
  *TP_Ptr++ = TPU_AT(0);
  *TP_Ptr++ = TPU_AT(0);
  *TP_Ptr++ = TPU_AT(0);

  l1dmacro_rx_down (STOP_RX_FB);

}

#endif/*(L1_RF_KBD_FIX == 0)*/

/*
 * l1dmacro_rx_fb26
 *
 * Receive Frequency burst for TCH.
 */
 #if(L1_RF_KBD_FIX == 1)
 #if (L1_MADC_ON == 1)
 void l1dmacro_rx_fb26 (SYS_UWORD16 radio_freq,UWORD8 adc_active)
{
  l1dmacro_rx_up(adc_active, L1_SAIC_HARDWARE_FILTER, L1_KBD_DIS_RX_FB26
  	#if(NEW_SNR_THRESHOLD==1)
	, SAIC_OFF
	#endif
	);
  l1s.total_kbd_on_time = 5000;
  *TP_Ptr++ = TPU_AT(0);

  l1dmacro_rx_down (STOP_RX_FB26);
  l1s.total_kbd_on_time = l1s.total_kbd_on_time - L1_KBD_DIS_RX_FB26 * (STOP_RX_FB26 - TRF_R7);
}

 #else
void l1dmacro_rx_fb26 (SYS_UWORD16 radio_freq)
{
  l1dmacro_rx_up(L1_SAIC_HARDWARE_FILTER, L1_KBD_DIS_RX_FB26
  	#if(NEW_SNR_THRESHOLD==1)
	, SAIC_OFF
	#endif
	);
  l1s.total_kbd_on_time = 5000;
  *TP_Ptr++ = TPU_AT(0);

  l1dmacro_rx_down (STOP_RX_FB26);
  l1s.total_kbd_on_time = l1s.total_kbd_on_time - L1_KBD_DIS_RX_FB26 * (STOP_RX_FB26 - TRF_R7);
}
#endif
#endif/*(L1_RF_KBD_FIX == 1)*/

#if(L1_RF_KBD_FIX == 0)
#if (L1_MADC_ON == 1)
 void l1dmacro_rx_fb26 (SYS_UWORD16 radio_freq,UWORD8 adc_active)
{
  l1dmacro_rx_up(adc_active, L1_SAIC_HARDWARE_FILTER
  	#if(NEW_SNR_THRESHOLD==1)
	, SAIC_OFF
	#endif
	);

  *TP_Ptr++ = TPU_AT(0);

  l1dmacro_rx_down (STOP_RX_FB26);

}

 #else
void l1dmacro_rx_fb26 (SYS_UWORD16 radio_freq)
{
  l1dmacro_rx_up(L1_SAIC_HARDWARE_FILTER
  	#if(NEW_SNR_THRESHOLD==1)
	, SAIC_OFF
	#endif
	);

  *TP_Ptr++ = TPU_AT(0);

  l1dmacro_rx_down (STOP_RX_FB26);

}
#endif
#endif/*(L1_RF_KBD_FIX == 0)*/

/*
 * l1dmacro_tx_nb
 *
 * Transmit Normal burst
 */
#if(L1_RF_KBD_FIX == 1)

 void l1dmacro_tx_nb (SYS_UWORD16 radio_freq, UWORD8 txpwr, UWORD8 adc_active)
{
  l1dmacro_tx_up (L1_KBD_DIS_TX_NB);
  l1dmacro_tx_down (l1_config.params.tx_nb_duration, FALSE, adc_active, L1_KBD_DIS_TX_NB);
  l1s.total_kbd_on_time = l1s.total_kbd_on_time - L1_KBD_DIS_TX_NB * (-TRF_T3_1 + l1_config.params.tx_nb_duration + TRF_T12);
}

#endif/*#if(L1_RF_KBD_FIX == 1)*/

#if(L1_RF_KBD_FIX == 0)
 void l1dmacro_tx_nb (SYS_UWORD16 radio_freq, UWORD8 txpwr, UWORD8 adc_active)
{
  l1dmacro_tx_up ();
  l1dmacro_tx_down (l1_config.params.tx_nb_duration, FALSE, adc_active);

}

#endif/*#if(L1_RF_KBD_FIX == 0)*/

/*
 * l1dmacro_tx_ra
 *
 * Transmit Random Access burst
 */
#if(L1_RF_KBD_FIX == 1)

void l1dmacro_tx_ra (SYS_UWORD16 radio_freq, UWORD8 txpwr, UWORD8 adc_active)
{
  l1dmacro_tx_up (L1_KBD_DIS_TX_RA);
  l1dmacro_tx_down (l1_config.params.tx_ra_duration, FALSE, adc_active, L1_KBD_DIS_TX_RA);
  l1s.total_kbd_on_time = l1s.total_kbd_on_time - L1_KBD_DIS_TX_RA * (-TRF_T3_1 + l1_config.params.tx_ra_duration + TRF_T12);
}
#endif /*#if(L1_RF_KBD_FIX == 1)*/

#if(L1_RF_KBD_FIX == 0)
void l1dmacro_tx_ra (SYS_UWORD16 radio_freq, UWORD8 txpwr, UWORD8 adc_active)
{
  l1dmacro_tx_up ();
  l1dmacro_tx_down (l1_config.params.tx_ra_duration, FALSE, adc_active);

}
#endif/*#if(L1_RF_KBD_FIX == 0)*/

  /*
   * l1dmacro_rx_cont
   *
   * Receive continuously
   */
#if(L1_RF_KBD_FIX == 1)
 #if (L1_MADC_ON == 1)
 void l1dmacro_rx_cont (SYS_UWORD16 radio_freq, UWORD8 txpwr,
                        UWORD8 adc_active, UWORD8 csf_filter_choice
										#if(NEW_SNR_THRESHOLD==1)
											, UWORD8 saic_flag_rx_up
										#endif
										)
{
  l1dmacro_rx_up (adc_active, csf_filter_choice, KBD_DISABLED
										#if(NEW_SNR_THRESHOLD==1)
											, saic_flag_rx_up
										#endif
										);
}
 #else
void l1dmacro_rx_cont (SYS_UWORD16 radio_freq, UWORD8 txpwr,
                       UWORD8 csf_filter_choice
								#if(NEW_SNR_THRESHOLD==1)
									, UWORD8 saic_flag_rx_up
								#endif
								)
{
  l1dmacro_rx_up (csf_filter_choice,KBD_DISABLED
								#if(NEW_SNR_THRESHOLD==1)
									, saic_flag_rx_up
								#endif
								);
}
#endif
#endif/*#if(L1_RF_KBD_FIX == 1)*/

#if(L1_RF_KBD_FIX == 0)
 #if (L1_MADC_ON == 1)
 void l1dmacro_rx_cont (SYS_UWORD16 radio_freq, UWORD8 txpwr,
                        UWORD8 adc_active, UWORD8 csf_filter_choice
								#if(NEW_SNR_THRESHOLD==1)
									, UWORD8 saic_flag_rx_up
								#endif
								)
{
  l1dmacro_rx_up (adc_active, csf_filter_choice
								#if(NEW_SNR_THRESHOLD==1)
									, saic_flag_rx_up
								#endif
								);
}
 #else
void l1dmacro_rx_cont (SYS_UWORD16 radio_freq, UWORD8 txpwr,
                       UWORD8 csf_filter_choice
							#if(NEW_SNR_THRESHOLD==1)
								, UWORD8 saic_flag_rx_up
							#endif
							)
{
  l1dmacro_rx_up (csf_filter_choice
						#if(NEW_SNR_THRESHOLD==1)
							, saic_flag_rx_up
						#endif
						);
}
#endif

#endif/*#if(L1_RF_KBD_FIX == 0)*/


  /*
   * l1dmacro_tx_cont
   *
   * Transmit continuously
   */
#if(L1_RF_KBD_FIX == 1)
void l1dmacro_tx_cont (SYS_UWORD16 radio_freq, UWORD8 txpwr)
{
  l1dmacro_tx_up (KBD_DISABLED);
}
#endif/*#if(L1_RF_KBD_FIX == 1)*/

#if(L1_RF_KBD_FIX == 0)
void l1dmacro_tx_cont (SYS_UWORD16 radio_freq, UWORD8 txpwr)
{
  l1dmacro_tx_up ();
}
#endif/*#if(L1_RF_KBD_FIX == 0)*/

  /*
   * l1d_macro_stop_cont
   *
   * Stop continuous Tx or Rx
   */
#if(L1_RF_KBD_FIX == 1)
void l1dmacro_stop_cont (void)
{
  if (l1_config.tmode.rf_params.down_up == TMODE_DOWNLINK)
    l1dmacro_rx_down(STOP_RX_SNB);
  else
    l1dmacro_tx_down(l1_config.params.tx_nb_duration, FALSE, 0, KBD_DISABLED);
}
#endif/*#if(L1_RF_KBD_FIX == 1)*/

#if(L1_RF_KBD_FIX == 0)
void l1dmacro_stop_cont (void)
{
  if (l1_config.tmode.rf_params.down_up == TMODE_DOWNLINK)
    l1dmacro_rx_down(STOP_RX_SNB);
  else
    l1dmacro_tx_down(l1_config.params.tx_nb_duration, FALSE, 0);
}

#endif/* */


/*------------------------------------------*/
/*             l1dmacro_reset_hw            */
/*------------------------------------------*/
/*      Reset and set OFFSET register       */
/*------------------------------------------*/

void l1dmacro_reset_hw(UWORD32 servingCellOffset)
{
   TPU_Reset(1); // reset TPU only, no TSP reset
   TPU_Reset(0);
   TP_Ptr = (UWORD16 *) TPU_RAM;

   *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U, TXM_SLEEP);
   *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_L, TXM_SLEEP);
   MOVE_REG_TSP_TO_RF(START_SCRIPT(DRP_IDLE),((UWORD16)( ((UWORD32)(&drp_regs->SCRIPT_STARTL))&0xFFFF)));

   *TP_Ptr++ = TPU_OFFSET(servingCellOffset);

}

//  l1dmacro_RF_sleep
//  Program RF for BIG or DEEP sleep


void l1dmacro_RF_sleep  (void)
{
  // sending REG_OFF script
   MOVE_REG_TSP_TO_RF(START_SCRIPT(DRP_REG_OFF), ((UWORD16)( ((UWORD32)(&drp_regs->SCRIPT_STARTL))&0xFFFF)));

  *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U, TXM_SLEEP);  //Shutdown FEM

  *TP_Ptr++ = TPU_SLEEP;
   TP_Ptr = (SYS_UWORD16 *) TPU_RAM;
   TP_Enable(1);
   TPU_wait_idle();

}


//  l1dmacro_RF_wakeup
//* wakeup RF from BIG or DEEP sleep

void l1dmacro_RF_wakeup  (void)
{
   // sending REG_ON script
   MOVE_REG_TSP_TO_RF(START_SCRIPT(DRP_REG_ON), ((UWORD16)( ((UWORD32)(&drp_regs->SCRIPT_STARTL))&0xFFFF)));

   *TP_Ptr++ = TPU_SLEEP;
   TP_Ptr = (SYS_UWORD16 *) TPU_RAM;
   TP_Enable(1);
   TPU_wait_idle();


}


//              l1dmacro_init_hw
//      Reset VEGA, then remove reset
//      Init RF/IF synthesizers

void l1dmacro_init_hw(void)
{
   WORD32 t = 100;    // start time for actions

   TP_Reset(1); // reset TPU and TSP

   // GSM 1.5 : TPU clock enable is in TPU
   //---------------------------------------
   TPU_ClkEnable(1);         // TPU CLOCK ON

   TP_Reset(0);


   TP_Ptr = (UWORD16 *) TPU_RAM;

   // Set FEM to inactive state before turning ON the RF Board
   // At this point the RF regulators are still OFF. Thus the
   // FEM command is not inverted yet => Must use the FEM "SLEEP programming"


   // TPU_SLEEP
   l1dmacro_idle();

   *TP_Ptr++ = TPU_AT(t);
   *TP_Ptr++ = TPU_SYNC(0);

	//Check Initialisation or Reset for TPU2OCP


   *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U, TXM_SLEEP);

   t = 1000;      // arbitrary start time

   t = rf_init(t); // Initialize RF Board

   *TP_Ptr++ = TPU_AT(t);

   // TPU_SLEEP
   l1dmacro_idle();

   return;
}

/*------------------------------------------*/
/*         l1dmacro_init_hw_light           */
/*------------------------------------------*/
/*      Reset VEGA, then remove reset       */
/*      Init RF/IF synthesizers             */
/*------------------------------------------*/
void l1dmacro_init_hw_light(void)
{
   UWORD32 t = 100;    // start time for actions //
   TP_Ptr = (SYS_UWORD16 *) TPU_RAM; //
   *TP_Ptr++ = TPU_AT(t);  //
   t = 1000;      // arbitrary start time //

   t = rf_init_light(t); // Initialize RF Board //

   *TP_Ptr++ = TPU_AT(t); //
   l1dmacro_idle(); //

   return;
}

//BHO added
/*
 * l1dmacro_rx_fbsb
 *
 * Receive Frequency burst
 */

#if ((REL99 == 1) && (FF_BHO == 1))
#if(L1_RF_KBD_FIX == 1)
#if (L1_MADC_ON == 1)
void l1dmacro_rx_fbsb (SYS_UWORD16 radio_freq, UWORD8 adc_active)
#else
void l1dmacro_rx_fbsb (SYS_UWORD16 radio_freq)
#endif
{
#if (L1_MADC_ON == 1)
  l1dmacro_rx_up(adc_active, L1_SAIC_HARDWARE_FILTER, L1_KBD_DIS_RX_FB
	#if(NEW_SNR_THRESHOLD==1)
		, SAIC_OFF
	#endif
		  );
#else
  l1dmacro_rx_up(L1_SAIC_HARDWARE_FILTER, L1_KBD_DIS_RX_FB);
#endif


  // same as rx_fb
  *TP_Ptr++ = TPU_AT(0); //  1
  *TP_Ptr++ = TPU_AT(0); //  2
  *TP_Ptr++ = TPU_AT(0); //  3
  *TP_Ptr++ = TPU_AT(0); //  4
  *TP_Ptr++ = TPU_AT(0); //  5
  *TP_Ptr++ = TPU_AT(0); //  6
  *TP_Ptr++ = TPU_AT(0); //  7
  *TP_Ptr++ = TPU_AT(0); //  8
  *TP_Ptr++ = TPU_AT(0); //  9
  *TP_Ptr++ = TPU_AT(0); //  10
  *TP_Ptr++ = TPU_AT(0); //  11

  // one more for SB
  *TP_Ptr++ = TPU_AT(0); //  12

  l1dmacro_rx_down (STOP_RX_FBSB);
}
#endif/*(L1_RF_KBD_FIX == 1)*/

#if(L1_RF_KBD_FIX == 0)
#if (L1_MADC_ON == 1)
void l1dmacro_rx_fbsb (SYS_UWORD16 radio_freq, UWORD8 adc_active)
#else
void l1dmacro_rx_fbsb (SYS_UWORD16 radio_freq)
#endif
{
#if (L1_MADC_ON == 1)
  l1dmacro_rx_up(adc_active, L1_SAIC_HARDWARE_FILTER);
#else
  l1dmacro_rx_up(L1_SAIC_HARDWARE_FILTER);
#endif

  // same as rx_fb
  *TP_Ptr++ = TPU_AT(0); //  1
  *TP_Ptr++ = TPU_AT(0); //  2
  *TP_Ptr++ = TPU_AT(0); //  3
  *TP_Ptr++ = TPU_AT(0); //  4
  *TP_Ptr++ = TPU_AT(0); //  5
  *TP_Ptr++ = TPU_AT(0); //  6
  *TP_Ptr++ = TPU_AT(0); //  7
  *TP_Ptr++ = TPU_AT(0); //  8
  *TP_Ptr++ = TPU_AT(0); //  9
  *TP_Ptr++ = TPU_AT(0); //  10
  *TP_Ptr++ = TPU_AT(0); //  11

  // one more for SB
  *TP_Ptr++ = TPU_AT(0); //  12

  l1dmacro_rx_down (STOP_RX_FBSB);
}
#endif/*(L1_RF_KBD_FIX == 0)*/
#endif // #if ((REL99 == 1) && (FF_BHO == 1))

////BHO
