/*
 * tpudrv12.c (TPU driver for RF type 12) is a required part of the L1
 * code for TI GSM chipset solutions consisting of Calypso or other
 * classic (non-LoCosto) DBB, one of the classic ABB chips such as Iota
 * or Syren, and Rita RF transceiver; the number 12 refers to the latter.
 *
 * We, the FreeCalypso team, have not been able to find an original
 * source for this C module: the LoCosto source has tpudrv61.c instead,
 * supporting LoCosto RF instead of Rita, whereas the TSM30 source
 * only supports non-TI RF transceivers.  Our only available reference
 * for what this tpudrv12.c module is supposed to contain is the
 * tpudrv12.obj COFF object from the Leonardo semi-src deliverable.
 *
 * The present reconstruction has been made by copying tpudrv61.c and
 * tweaking it to match the disassembly of the reference binary object
 * named above.
 *
 * The ugly hacks to support Compal and Pirelli targets in addition to
 * classic TI/Openmoko ones are original to FreeCalypso.
 */

#define TPUDRV12_C

#include "config.h"
#include "sys_types.h"
#include "l1_confg.h"

#include "l1_macro.h"
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
#include "tpudrv12.h"
#include "l1_rf12.h"

#include "../../bsp/mem.h"
#include "../../bsp/armio.h"
#include "../../bsp/clkm.h"

// Global variables
extern T_L1_CONFIG l1_config;
extern UWORD16  AGC_TABLE[];
extern UWORD16  *TP_Ptr;
#if (L1_FF_MULTIBAND == 1)
extern const WORD8 rf_subband2band[RF_NB_SUBBANDS];
#endif

static WORD8   rf_index;      // index into rf_path[]
static UWORD16 rf_chip_band;	/* from tpudrv12.obj, not in tpudrv61.c */
static UWORD8  rfband;		/* ditto */

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
/*   Send a value to Rita RF                */
/*------------------------------------------*/
#define TSP_TO_RF(rf_data)\
	{\
	*TP_Ptr++ = TPU_MOVE(TSP_TX_REG_1, ((rf_data) >> 8) & 0xFF);	\
	*TP_Ptr++ = TPU_MOVE(TSP_TX_REG_2, (rf_data) & 0xFF);		\
	*TP_Ptr++ = TPU_MOVE(TSP_CTRL1, TC1_DEVICE_RF | 0x0F);		\
	*TP_Ptr++ = TPU_MOVE(TSP_CTRL2, TC2_WR);			\
	}

/*------------------------------------------*/
/*   Send a TSP command to ABB              */
/*------------------------------------------*/
#define TSP_TO_ABB(data)\
	{\
	*TP_Ptr++ = TPU_MOVE(TSP_TX_REG_1, (data) & 0xFF);		\
	*TP_Ptr++ = TPU_MOVE(TSP_CTRL1, TC1_DEVICE_ABB | 0x06);		\
	*TP_Ptr++ = TPU_MOVE(TSP_CTRL2, TC2_WR);			\
	}

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
    UWORD16 rf_chip_band;	/* from tpudrv12.obj, not in tpudrv61.c */
    T_TX_RX tx_rx[2];
};

#if CONFIG_TARGET_COMPAL || CONFIG_TARGET_PIRELLI
typedef UWORD16 T_RXTX_UPDOWN;
#else
typedef UWORD8  T_RXTX_UPDOWN;
#endif

struct rf_path_s {
  T_RXTX_UPDOWN rx_up;
  T_RXTX_UPDOWN rx_down;
  T_RXTX_UPDOWN tx_up;
  T_RXTX_UPDOWN tx_down;
  struct synth_s *synth;
};

const struct synth_s synth_900[] =
{
  {  0,  124, BAND_SELECT_GSM, {{ 890,   1}, { 935,   2}}},// gsm    0 - 124
  {974, 1023, BAND_SELECT_GSM, {{ 880,   1}, { 925,   2}}},// egsm 975 - 1023
};

const struct synth_s synth_1800[] =
{
  {511, 885, BAND_SELECT_DCS, {{1710,  1},  {1805,   1}}}, // dcs  512 - 885
};

const struct synth_s synth_1900[] =
{
  {511, 810, BAND_SELECT_PCS, {{1850,  1},  {1930,   1}}}, // pcs  512 - 810;
};

const struct synth_s synth_850[] =
{
  {127, 192, BAND_SELECT_850_LO, {{ 824,   2},  { 869,   2}}}, // gsm850 low
  {127, 251, BAND_SELECT_850_HI, {{ 824,   1},  { 869,   2}}}, // gsm850 high
};

struct rf_path_s rf_path[] = {    //same index used as for band_config[] - 1
  { RU_900,  RD_900,  TU_900,  TD_900,  (struct synth_s *)synth_900 }, //EGSM
  { RU_1800, RD_1800, TU_1800, TD_1800, (struct synth_s *)synth_1800}, //DCS
  { RU_1900, RD_1900, TU_1900, TD_1900, (struct synth_s *)synth_1900}, //PCS
  { RU_850,  RD_850,  TU_850,  TD_850,  (struct synth_s *)synth_850 }, //GSM850
  { RU_900,  RD_900,  TU_900,  TD_900,  (struct synth_s *)synth_900 }, //GSM
};

/*
 * Leonardo tpudrv12.obj contains a function named calc_a_b(); there is
 * no such function in the LoCosto version, but there is a similar-looking
 * calc_rf_freq() function instead.  Let's try making our calc_a_b()
 * from LoCosto's calc_rf_freq().
 */

UWORD32 calc_a_b(UWORD16 arfcn, UWORD8 downlink)
{
  UWORD32 farfcn;	/* in 200 kHz units */
  UWORD32 n;		/* B * P + A */
  struct synth_s  *s;

  s = rf_path[rf_index].synth;
  while(s->limit < arfcn)
    s++;

  rf_chip_band = s->rf_chip_band;

  // Convert the ARFCN to the channel frequency (times 5 to avoid the decimal value)
  farfcn = 5*s->tx_rx[downlink].farfcn0 + (arfcn - s->arfcn0);
  n = farfcn * s->tx_rx[downlink].ou;

  /* magic A & B encoding for Rita */
  return((n - 4096) << 3);
}

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

/*------------------------------------------*/
/*              rf_init                     */
/*------------------------------------------*/
/*    Initialization routine for PLL        */
/*   Effective downloading through TSP      */
/*------------------------------------------*/
/* Rita and LoCosto versions look totally   */
/* different, reconstructing from disasm.   */
/*------------------------------------------*/
WORD32 rf_init(WORD32 t)
{
	*TP_Ptr++ = TPU_AT(t);
	*TP_Ptr++ = TPU_MOVE(TSP_CTRL1, 0x47);
	t += 5;
	*TP_Ptr++ = TPU_AT(t);
	*TP_Ptr++ = TPU_MOVE(TSP_ACT, RF_SER_OFF);
	t += 8;
	*TP_Ptr++ = TPU_AT(t);
	*TP_Ptr++ = TPU_MOVE(TSP_ACT, RF_SER_ON);
	t += 5;
	*TP_Ptr++ = TPU_AT(t);
	TSP_TO_RF(0x0012);
	t += 7;
	*TP_Ptr++ = TPU_AT(t);
	*TP_Ptr++ = TPU_AT(t);
	*TP_Ptr++ = TPU_AT(t);
	*TP_Ptr++ = TPU_AT(t);
	*TP_Ptr++ = TPU_AT(t);
	*TP_Ptr++ = TPU_AT(t);
	TSP_TO_RF(0x003A);
	t += 117;
	*TP_Ptr++ = TPU_AT(t);
	TSP_TO_RF(0xC003);
	t += 7;
	*TP_Ptr++ = TPU_AT(t);
	TSP_TO_RF(0x02FE);
	t += 7;
	*TP_Ptr++ = TPU_AT(t);
	TSP_TO_RF(0x401F);
	t += 7;
	*TP_Ptr++ = TPU_AT(t);
	TSP_TO_RF(0x043D);
	t += 7;
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

UWORD8 arfcn_to_rf_index(SYS_UWORD16 arfcn)
{
  UWORD8 index;
  extern const T_STD_CONFIG std_config[];
  index = std_config[l1_config.std.id].band[0];

  if ((std_config[l1_config.std.id].band[1] != BAND_NONE) && IS_HIGH_BAND(arfcn))
    index = std_config[l1_config.std.id].band[1];

  return (index - 1);
}

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

  rfband = Cust_is_band_high(radio_freq);

  arfcn = Convert_l1_radio_freq(radio_freq);
  #ifdef ARFCN_DEBUG
    trace_arfcn(arfcn);
  #endif
  rf_index = arfcn_to_rf_index(arfcn);

  rfdiv = calc_a_b(arfcn, rx);

  if (rx != 1) {
    TSP_TO_RF(rfdiv | REG_PLL);
    *TP_Ptr++ = TPU_FAT(0x1274);
    TSP_TO_RF(0x043A | rf_chip_band);
  } else {
    TSP_TO_RF(rfdiv | REG_PLL);
    *TP_Ptr++ = TPU_FAT(0x12FD);
    TSP_TO_RF(0x023A | rf_chip_band);
  }

  return(t);
}

/**************************************************************************/
/**************************************************************************/
/*                    EXTERNAL FUNCTIONS CALLED BY LAYER1                 */
/*                          COMMON TO L1 and TOOLKIT                      */
/**************************************************************************/
/**************************************************************************/

/*------------------------------------------*/
/*                agc                       */
/*------------------------------------------*/
/*      Program a gain into IF amp          */
/*      agc_value : gain in dB              */
/*                                          */
/*   additional parameter for LNA setting   */
/*------------------------------------------*/
/* Rita and LoCosto versions look totally   */
/* different, reconstructing from disasm.   */
/*------------------------------------------*/

void l1dmacro_agc(SYS_UWORD16 radio_freq, WORD8 gain, UWORD8 lna_off)
{
	int agc_table_index;
	UWORD16 rf_data;

	agc_table_index = gain - 2;
	if (agc_table_index < 0)
		agc_table_index++;
	agc_table_index >>= 1;
	if (gain >= 42)
		agc_table_index = 19;
	if (gain < 16)
		agc_table_index = 6;
	*TP_Ptr++ = TPU_FAT(0x1334);
	rf_data = REG_RX;
	if (!lna_off)
		rf_data |= RF_GAIN;
	rf_data |= AGC_TABLE[agc_table_index] << 11;
	rf_data |= RX_CAL_MODE;
	TSP_TO_RF(rf_data);
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
/* Rita version differs from LoCosto,       */
/* reconstructing from disassembly.         */
/*------------------------------------------*/
void l1dmacro_rx_up (void)
{
	*TP_Ptr++ = TPU_FAT(0x1377);
	TSP_TO_RF(0x0A3A | rf_chip_band);
	*TP_Ptr++ = TPU_FAT(0x137E);
	TSP_TO_ABB(0x10);
	*TP_Ptr++ = TPU_FAT(0x1383);
	TSP_TO_ABB(0x18);
	*TP_Ptr++ = TPU_FAT(58);
	*TP_Ptr++ = TPU_MOVE(TSP_ACT, rf_path[rf_index].rx_up | RF_SER_ON);
	*TP_Ptr++ = TPU_FAT(62);
	TSP_TO_ABB(0x14);
}

/*------------------------------------------*/
/*            l1pdmacro_rx_down             */
/*------------------------------------------*/
/* Close window for normal burst reception  */
/*------------------------------------------*/
/* Rita version differs from LoCosto,       */
/* reconstructing from disassembly.         */
/*------------------------------------------*/
void l1dmacro_rx_down (WORD32 t)
{
	*TP_Ptr++ = TPU_FAT(t - 37);
	TSP_TO_RF(0x003A);
	*TP_Ptr++ = TPU_MOVE(TSP_ACT, rf_path[rf_index].rx_down | RF_SER_ON);
	*TP_Ptr++ = TPU_FAT(t - 4);
	TSP_TO_ABB(0x00);
}

/*------------------------------------------*/
/*            l1dmacro_tx_up                */
/*------------------------------------------*/
/* Open transmission window for normal burst*/
/*------------------------------------------*/
/* Rita version differs from LoCosto,       */
/* reconstructing from disassembly.         */
/*------------------------------------------*/
void l1dmacro_tx_up (void)
{
	if (l1_config.std.id == DCS1800 ||
	    rfband == MULTI_BAND2 &&
	    (l1_config.std.id == DUAL || l1_config.std.id == DUALEXT)) {
		*TP_Ptr++ = TPU_FAT(0x127E);
		TSP_TO_RF(0x0007);
		*TP_Ptr++ = TPU_FAT(0x1288);
		TSP_TO_RF(0xC00B);
		*TP_Ptr++ = TPU_FAT(0x1292);
		TSP_TO_RF(0x3077);
	} else {
		*TP_Ptr++ = TPU_FAT(0x127E);
		TSP_TO_RF(0xC003);
	}
	*TP_Ptr++ = TPU_FAT(0x12C6);
	TSP_TO_ABB(0x80);
	*TP_Ptr++ = TPU_FAT(0x12E3);
	TSP_TO_RF(0x243A | rf_chip_band);
	*TP_Ptr++ = TPU_FAT(0x1302);
	TSP_TO_ABB(0xC0);
	*TP_Ptr++ = TPU_FAT(0x1352);
	TSP_TO_ABB(0x80);
	*TP_Ptr++ = TPU_FAT(0x1384);
	TSP_TO_ABB(0xA0);
	*TP_Ptr++ = TPU_FAT(16);
#if CONFIG_TARGET_COMPAL || CONFIG_TARGET_PIRELLI
	*TP_Ptr++ = TPU_MOVE(TSP_ACT, rf_path[rf_index].tx_up & 0xFF
					| RF_SER_ON);
	*TP_Ptr++ = TPU_MOVE(TSP_ACTX, rf_path[rf_index].tx_up >> 8);
	*TP_Ptr++ = TPU_FAT(21);
	*TP_Ptr++ = TPU_MOVE(TSP_ACT, rf_path[rf_index].tx_up & 0xFF
					| PA_ENABLE | RF_SER_ON);
#else
	*TP_Ptr++ = TPU_MOVE(TSP_ACT, rf_path[rf_index].tx_up | RF_SER_ON);
	*TP_Ptr++ = TPU_FAT(21);
	*TP_Ptr++ = TPU_MOVE(TSP_ACTX, 0x0F);
#endif
}

/*-------------------------------------------*/
/*            l1dmacro_tx_down               */
/*-------------------------------------------*/
/* Close transmission window for normal burst*/
/*-------------------------------------------*/
/* Rita version differs from LoCosto,        */
/* reconstructing from disassembly.          */
/*-------------------------------------------*/
void l1dmacro_tx_down (WORD32 t, BOOL tx_flag, UWORD8 adc_active)
{
	if (adc_active == ACTIVE)
		l1dmacro_adc_read_tx(t - 44);
	*TP_Ptr++ = TPU_FAT(t - 4);
	TSP_TO_ABB(0x80);
	*TP_Ptr++ = TPU_FAT(t + 22);
	*TP_Ptr++ = TPU_MOVE(TSP_ACTX, 0x00);
	*TP_Ptr++ = TPU_MOVE(TSP_ACT, rf_path[rf_index].tx_down | RF_SER_ON);
	*TP_Ptr++ = TPU_FAT(t + 25);
	TSP_TO_RF(0x003A);
	*TP_Ptr++ = TPU_FAT(t + 31);
	TSP_TO_ABB(0x00);
}

/*
 * l1dmacro_rx_nb
 *
 * Receive Normal burst
 */
void l1dmacro_rx_nb (SYS_UWORD16 radio_freq)
{
	l1dmacro_rx_up();
	l1dmacro_rx_down(STOP_RX_SNB);
}

/*
 * l1dmacro_rx_sb
 * Receive Synchro burst
 */
void l1dmacro_rx_sb (SYS_UWORD16 radio_freq)
{
  l1dmacro_rx_up();
  l1dmacro_rx_down (STOP_RX_SB);
}

/*
 * l1dmacro_rx_ms
 *
 * Receive Power Measurement window
 */
void l1dmacro_rx_ms (SYS_UWORD16 radio_freq)
{
  l1dmacro_rx_up();
  l1dmacro_rx_down (STOP_RX_PW_1);
}

/*
 * l1dmacro_rx_fb
 *
 * Receive Frequency burst
 */
void l1dmacro_rx_fb (SYS_UWORD16 radio_freq)
{
  l1dmacro_rx_up();

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

/*
 * l1dmacro_rx_fb26
 *
 * Receive Frequency burst for TCH.
 */
void l1dmacro_rx_fb26 (SYS_UWORD16 radio_freq)
{
  l1dmacro_rx_up();

  *TP_Ptr++ = TPU_AT(0);

  l1dmacro_rx_down (STOP_RX_FB26);
}

/*
 * l1dmacro_tx_nb
 *
 * Transmit Normal burst
 */
void l1dmacro_tx_nb (SYS_UWORD16 radio_freq, UWORD8 txpwr, UWORD8 adc_active)
{
  l1dmacro_tx_up ();
  l1dmacro_tx_down (l1_config.params.tx_nb_duration, FALSE, adc_active);
}

/*
 * l1dmacro_tx_ra
 *
 * Transmit Random Access burst
 */
void l1dmacro_tx_ra (SYS_UWORD16 radio_freq, UWORD8 txpwr, UWORD8 adc_active)
{
  l1dmacro_tx_up ();
  l1dmacro_tx_down (l1_config.params.tx_ra_duration, FALSE, adc_active);
}

#if TESTMODE
/*
 * l1dmacro_rx_cont
 *
 * Receive continuously
 */
void l1dmacro_rx_cont (SYS_UWORD16 radio_freq, UWORD8 txpwr)
{
  l1dmacro_rx_up ();
}

/*
 * l1dmacro_tx_cont
 *
 * Transmit continuously
 */
void l1dmacro_tx_cont (SYS_UWORD16 radio_freq, UWORD8 txpwr)
{
  l1dmacro_tx_up ();
}

/*
 * l1d_macro_stop_cont
 *
 * Stop continuous Tx or Rx
 */
void l1dmacro_stop_cont (void)
{
  if (l1_config.tmode.rf_params.down_up == TMODE_DOWNLINK)
    l1dmacro_rx_down(STOP_RX_SNB);
  else
    l1dmacro_tx_down(l1_config.params.tx_nb_duration, FALSE, 0);
}
#endif	/* TESTMODE */


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

   *TP_Ptr++ = TPU_MOVE(TSP_ACT, RF_SER_ON);
   *TP_Ptr++ = TPU_MOVE(TSP_ACT, RF_SER_ON | FEM_OFF);

   *TP_Ptr++ = TPU_OFFSET(servingCellOffset);
}

//  l1dmacro_RF_sleep
//  Program RF for BIG or DEEP sleep


/* Rita version differs from LoCosto, reconstructing from disassembly */
void l1dmacro_RF_sleep  (void)
{
	TSP_TO_RF(0x0002);
	*TP_Ptr++ = TPU_MOVE(TSP_ACT, RF_SER_ON);
	*TP_Ptr++ = TPU_WAIT(1);
	*TP_Ptr++ = TPU_MOVE(TSP_SPI_SET1, 0x21);
	*TP_Ptr++ = TPU_MOVE(TSP_SPI_SET2, 0x02);
	*TP_Ptr++ = TPU_MOVE(TSP_CTRL1, TC1_DEVICE_RF | 0x01);
	*TP_Ptr++ = TPU_MOVE(TSP_CTRL2, TC2_WR);
	*TP_Ptr++ = TPU_WAIT(100);
	/* code from tpudrv61.c follows, same for Rita and LoCosto */
	*TP_Ptr++ = TPU_SLEEP;
	TP_Ptr = (SYS_UWORD16 *) TPU_RAM;
	TP_Enable(1);
	/*
	 * The following call does not appear in tpudrv12.obj, and
	 * there is no TPU_wait_idle() function in Leonardo tpudrv.obj
	 * either.  But this wait operation makes sense to me, so
	 * I'm keeping it as-is from the LoCosto version for now.
	 * -- Space Falcon
	 */
	TPU_wait_idle();
}

//  l1dmacro_RF_wakeup
//* wakeup RF from BIG or DEEP sleep

/* Rita version differs from LoCosto, reconstructing from disassembly */
void l1dmacro_RF_wakeup  (void)
{
	TP_Ptr = (SYS_UWORD16 *) TPU_RAM;
	*TP_Ptr++ = TPU_MOVE(TSP_SPI_SET1, 0x01);
	*TP_Ptr++ = TPU_MOVE(TSP_SPI_SET2, 0x06);
	*TP_Ptr++ = TPU_MOVE(TSP_CTRL1, TC1_DEVICE_RF | 0x01);
	*TP_Ptr++ = TPU_MOVE(TSP_CTRL2, TC2_WR);
	*TP_Ptr++ = TPU_WAIT(100);
	*TP_Ptr++ = TPU_MOVE(TSP_ACT, rf_path[rf_index].rx_down | RF_SER_ON);
	*TP_Ptr++ = TPU_WAIT(1);
	*TP_Ptr++ = TPU_MOVE(TSP_ACT, rf_path[rf_index].rx_down | RF_SER_OFF);
	*TP_Ptr++ = TPU_WAIT(8);
	*TP_Ptr++ = TPU_MOVE(TSP_ACT, rf_path[rf_index].rx_down | RF_SER_ON);
	*TP_Ptr++ = TPU_WAIT(5);
	TSP_TO_RF(0x0012);
	*TP_Ptr++ = TPU_FAT(0);
	*TP_Ptr++ = TPU_FAT(0);
	*TP_Ptr++ = TPU_FAT(0);
	*TP_Ptr++ = TPU_FAT(0);
	*TP_Ptr++ = TPU_FAT(0);
	*TP_Ptr++ = TPU_FAT(0);
	TSP_TO_RF(0x003A);
	*TP_Ptr++ = TPU_WAIT(7);
	TSP_TO_RF(0xC003);
	*TP_Ptr++ = TPU_WAIT(7);
	TSP_TO_RF(0x02FE);
	*TP_Ptr++ = TPU_WAIT(7);
	TSP_TO_RF(0x401F);
	*TP_Ptr++ = TPU_WAIT(7);
	TSP_TO_RF(0x043D);
	*TP_Ptr++ = TPU_WAIT(7);
	*TP_Ptr++ = TPU_WAIT(117);
	/* code from tpudrv61.c follows, same for Rita and LoCosto */
	*TP_Ptr++ = TPU_SLEEP;
	TP_Ptr = (SYS_UWORD16 *) TPU_RAM;
	TP_Enable(1);
	/* same issue as in the previous function */
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
   *TP_Ptr++ = TPU_MOVE(TSP_ACT, FEM_SLEEP | RF_SER_ON);

   // TPU_SLEEP
   l1dmacro_idle();

   *TP_Ptr++ = TPU_AT(t);
   *TP_Ptr++ = TPU_SYNC(0);

   /* from disassembly, differs from LoCosto version */
   *TP_Ptr++ = TPU_MOVE(TSP_SPI_SET1, 0x20);
   *TP_Ptr++ = TPU_MOVE(TSP_SPI_SET2, 0x06);
   *TP_Ptr++ = TPU_MOVE(TSP_SPI_SET3, 0x00);

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
