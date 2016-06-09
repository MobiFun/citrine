/*
 * In TI's original code these l1_rfXX.c files were not compilation units
 * in themselves, but were #included into l1_cust.c instead.  I dislike
 * that approach, so I decided to make this FreeCalypso version of l1_rf12.c
 * into its own compilation unit.
 *
 * This FreeCalypso version of l1_rf12.c is based on MV100, Leonardo and
 * Openmoko versions as explained in the comments for each struct.
 */

#include "config.h"
#include "l1_confg.h"
#include "l1_const.h"
#include "l1_types.h"
#include "../../bsp/abb+spi/abb.h"
#include "l1_rf12.h"

/*
 * The following definition of T_RF rf comes from mv100/l1_rf12.c.
 * The elaborate initialization of the structure has been compared against
 * the object blob in the Leonardo version and found to match.
 */
T_RF rf =
{
  RF_RITA_10,           //RF revision
  RF_HW_BAND_SUPPORT,   // radio_band_support E-GSM/DCS + PCS

  { //RX structure
    { //AGC structure
      140,  // low_agc_noise_thr;
      110,  // high_agc_sat_thr;
        6,  // low_agc;
       34,  // high_agc;
      //IL2AGC tables
      {  // below is: il2agc_pwr[121];
        //           il2agc_max[121];
        //           il2agc_av[121];
        // il2agc_pwr
        // Note this is shared between PCN and EGSM.
    14,          /*  EGSM_MAX  IL=0 */
    14,          /*  EGSM_MAX  IL=-1 */
    14,          /*  EGSM_MAX  IL=-2 */
    14,          /*  EGSM_MAX  IL=-3 */
    14,          /*  EGSM_MAX  IL=-4 */
    14,          /*  EGSM_MAX  IL=-5 */
    14,          /*  EGSM_MAX  IL=-6 */
    14,          /*  EGSM_MAX  IL=-7 */
    14,          /*  EGSM_MAX  IL=-8 */
    14,          /*  EGSM_MAX  IL=-9 */
    14,          /*  EGSM_MAX  IL=-10 */
    14,          /*  EGSM_MAX  IL=-11 */
    14,          /*  EGSM_MAX  IL=-12 */
    14,          /*  EGSM_MAX  IL=-13 */
    14,          /*  EGSM_MAX  IL=-14 */
    14,          /*  EGSM_MAX  IL=-15 */
    14,          /*  EGSM_MAX  IL=-16 */
    14,          /*  EGSM_MAX  IL=-17 */
    14,          /*  EGSM_MAX  IL=-18 */
    14,          /*  EGSM_MAX  IL=-19 */
    14,          /*  EGSM_MAX  IL=-20 */
    14,          /*  EGSM_MAX  IL=-21 */
    14,          /*  EGSM_MAX  IL=-22 */
    14,          /*  EGSM_MAX  IL=-23 */
    14,          /*  EGSM_MAX  IL=-24 */
    14,          /*  EGSM_MAX  IL=-25 */
    14,          /*  EGSM_MAX  IL=-26 */
    14,          /*  EGSM_MAX  IL=-27 */
    14,          /*  EGSM_MAX  IL=-28 */
    14,          /*  EGSM_MAX  IL=-29 */
    14,          /*  EGSM_MAX  IL=-30 */
    14,          /*  EGSM_MAX  IL=-31 */
    14,          /*  EGSM_MAX  IL=-32 */
    14,          /*  EGSM_MAX  IL=-33 */
    14,          /*  EGSM_MAX  IL=-34 */
    14,          /*  EGSM_MAX  IL=-35 */
    14,          /*  EGSM_MAX  IL=-36 */
    14,          /*  EGSM_MAX  IL=-37 */
    14,          /*  EGSM_MAX  IL=-38 */
    14,          /*  EGSM_MAX  IL=-39 */
    14,          /*  EGSM_MAX  IL=-40 */
    14,          /*  EGSM_MAX  IL=-41 */
    14,          /*  EGSM_MAX  IL=-42 */
    14,          /*  EGSM_MAX  IL=-43 */
    14,          /*  EGSM_MAX  IL=-44 */
    14,          /*  EGSM_MAX  IL=-45 */
    14,          /*  EGSM_MAX  IL=-46 */
    14,          /*  EGSM_MAX  IL=-47 */
    14,          /*  EGSM_MAX  IL=-48 */
    14,          /*  EGSM_MAX  IL=-49 */
    14,          /*  EGSM_MAX  IL=-50 */
    14,          /*  EGSM_MAX  IL=-51 */
    14,          /*  EGSM_MAX  IL=-52 */
    14,          /*  EGSM_MAX  IL=-53 */
    14,          /*  EGSM_MAX  IL=-54 */
    16,          /*  EGSM_MAX  IL=-55 */
    16,          /*  EGSM_MAX  IL=-56 */
    18,          /*  EGSM_MAX  IL=-57 */
    18,          /*  EGSM_MAX  IL=-58 */
    20,          /*  EGSM_MAX  IL=-59 */
    20,          /*  EGSM_MAX  IL=-60 */
    22,          /*  EGSM_MAX  IL=-61 */
    22,          /*  EGSM_MAX  IL=-62 */
    24,          /*  EGSM_MAX  IL=-63 */
    24,          /*  EGSM_MAX  IL=-64 */
    26,          /*  EGSM_MAX  IL=-65 */
    26,          /*  EGSM_MAX  IL=-66 */
    28,          /*  EGSM_MAX  IL=-67 */
    28,          /*  EGSM_MAX  IL=-68 */
    30,          /*  EGSM_MAX  IL=-69 */
    30,          /*  EGSM_MAX  IL=-70 */
    32,          /*  EGSM_MAX  IL=-71 */
    32,          /*  EGSM_MAX  IL=-72 */
    34,          /*  EGSM_MAX  IL=-73 */
    34,          /*  EGSM_MAX  IL=-74 */
    36,          /*  EGSM_MAX  IL=-75 */
    36,          /*  EGSM_MAX  IL=-76 */
    38,          /*  EGSM_MAX  IL=-77 */
    38,          /*  EGSM_MAX  IL=-78 */
    40,          /*  EGSM_MAX  IL=-79 */
    40,          /*  EGSM_MAX  IL=-80 */
    40,          /*  EGSM_MAX  IL=-81 */
    40,          /*  EGSM_MAX  IL=-82 */
    40,          /*  EGSM_MAX  IL=-83 */
    40,          /*  EGSM_MAX  IL=-84 */
    40,          /*  EGSM_MAX  IL=-85 */
    40,          /*  EGSM_MAX  IL=-86 */
    40,          /*  EGSM_MAX  IL=-87 */
    40,          /*  EGSM_MAX  IL=-88 */
    40,          /*  EGSM_MAX  IL=-89 */
    40,          /*  EGSM_MAX  IL=-90 */
    40,          /*  EGSM_MAX  IL=-91 */
    40,          /*  EGSM_MAX  IL=-92 */
    40,          /*  EGSM_MAX  IL=-93 */
    40,          /*  EGSM_MAX  IL=-94 */
    40,          /*  EGSM_MAX  IL=-95 */
    40,          /*  EGSM_MAX  IL=-96 */
    40,          /*  EGSM_MAX  IL=-97 */
    40,          /*  EGSM_MAX  IL=-98 */
    40,          /*  EGSM_MAX  IL=-99 */
    40,          /*  EGSM_MAX  IL=-100 */
    40,          /*  EGSM_MAX  IL=-101 */
    40,          /*  EGSM_MAX  IL=-102 */
    40,          /*  EGSM_MAX  IL=-103 */
    40,          /*  EGSM_MAX  IL=-104 */
    40,          /*  EGSM_MAX  IL=-105 */
    40,          /*  EGSM_MAX  IL=-106 */
    40,          /*  EGSM_MAX  IL=-107 */
    40,          /*  EGSM_MAX  IL=-108 */
    40,          /*  EGSM_MAX  IL=-109 */
    40,          /*  EGSM_MAX  IL=-110 */
    40,          /*  EGSM_MAX  IL=-111 */
    40,          /*  EGSM_MAX  IL=-112 */
    40,          /*  EGSM_MAX  IL=-113 */
    40,          /*  EGSM_MAX  IL=-114 */
    40,          /*  EGSM_MAX  IL=-115 */
    40,          /*  EGSM_MAX  IL=-116 */
    40,          /*  EGSM_MAX  IL=-117 */
    40,          /*  EGSM_MAX  IL=-118 */
    40,          /*  EGSM_MAX  IL=-119 */
    40           /*  EGSM_MAX  IL=-120 */
      },
      { // il2agc_max
        // Note this is shared between PCN and EGSM.
    14,          /*  EGSM_MAX  IL=0 */
    14,          /*  EGSM_MAX  IL=-1 */
    14,          /*  EGSM_MAX  IL=-2 */
    14,          /*  EGSM_MAX  IL=-3 */
    14,          /*  EGSM_MAX  IL=-4 */
    14,          /*  EGSM_MAX  IL=-5 */
    14,          /*  EGSM_MAX  IL=-6 */
    14,          /*  EGSM_MAX  IL=-7 */
    14,          /*  EGSM_MAX  IL=-8 */
    14,          /*  EGSM_MAX  IL=-9 */
    14,          /*  EGSM_MAX  IL=-10 */
    14,          /*  EGSM_MAX  IL=-11 */
    14,          /*  EGSM_MAX  IL=-12 */
    14,          /*  EGSM_MAX  IL=-13 */
    14,          /*  EGSM_MAX  IL=-14 */
    14,          /*  EGSM_MAX  IL=-15 */
    14,          /*  EGSM_MAX  IL=-16 */
    14,          /*  EGSM_MAX  IL=-17 */
    14,          /*  EGSM_MAX  IL=-18 */
    14,          /*  EGSM_MAX  IL=-19 */
    14,          /*  EGSM_MAX  IL=-20 */
    14,          /*  EGSM_MAX  IL=-21 */
    14,          /*  EGSM_MAX  IL=-22 */
    14,          /*  EGSM_MAX  IL=-23 */
    14,          /*  EGSM_MAX  IL=-24 */
    14,          /*  EGSM_MAX  IL=-25 */
    14,          /*  EGSM_MAX  IL=-26 */
    14,          /*  EGSM_MAX  IL=-27 */
    14,          /*  EGSM_MAX  IL=-28 */
    14,          /*  EGSM_MAX  IL=-29 */
    14,          /*  EGSM_MAX  IL=-30 */
    14,          /*  EGSM_MAX  IL=-31 */
    14,          /*  EGSM_MAX  IL=-32 */
    14,          /*  EGSM_MAX  IL=-33 */
    14,          /*  EGSM_MAX  IL=-34 */
    14,          /*  EGSM_MAX  IL=-35 */
    14,          /*  EGSM_MAX  IL=-36 */
    14,          /*  EGSM_MAX  IL=-37 */
    14,          /*  EGSM_MAX  IL=-38 */
    14,          /*  EGSM_MAX  IL=-39 */
    14,          /*  EGSM_MAX  IL=-40 */
    14,          /*  EGSM_MAX  IL=-41 */
    14,          /*  EGSM_MAX  IL=-42 */
    14,          /*  EGSM_MAX  IL=-43 */
    14,          /*  EGSM_MAX  IL=-44 */
    14,          /*  EGSM_MAX  IL=-45 */
    14,          /*  EGSM_MAX  IL=-46 */
    14,          /*  EGSM_MAX  IL=-47 */
    14,          /*  EGSM_MAX  IL=-48 */
    14,          /*  EGSM_MAX  IL=-49 */
    14,          /*  EGSM_MAX  IL=-50 */
    14,          /*  EGSM_MAX  IL=-51 */
    14,          /*  EGSM_MAX  IL=-52 */
    14,          /*  EGSM_MAX  IL=-53 */
    14,          /*  EGSM_MAX  IL=-54 */
    16,          /*  EGSM_MAX  IL=-55 */
    16,          /*  EGSM_MAX  IL=-56 */
    18,          /*  EGSM_MAX  IL=-57 */
    18,          /*  EGSM_MAX  IL=-58 */
    20,          /*  EGSM_MAX  IL=-59 */
    20,          /*  EGSM_MAX  IL=-60 */
    22,          /*  EGSM_MAX  IL=-61 */
    22,          /*  EGSM_MAX  IL=-62 */
    24,          /*  EGSM_MAX  IL=-63 */
    24,          /*  EGSM_MAX  IL=-64 */
    26,          /*  EGSM_MAX  IL=-65 */
    26,          /*  EGSM_MAX  IL=-66 */
    28,          /*  EGSM_MAX  IL=-67 */
    28,          /*  EGSM_MAX  IL=-68 */
    30,          /*  EGSM_MAX  IL=-69 */
    30,          /*  EGSM_MAX  IL=-70 */
    32,          /*  EGSM_MAX  IL=-71 */
    32,          /*  EGSM_MAX  IL=-72 */
    34,          /*  EGSM_MAX  IL=-73 */
    34,          /*  EGSM_MAX  IL=-74 */
    36,          /*  EGSM_MAX  IL=-75 */
    36,          /*  EGSM_MAX  IL=-76 */
    38,          /*  EGSM_MAX  IL=-77 */
    38,          /*  EGSM_MAX  IL=-78 */
    40,          /*  EGSM_MAX  IL=-79 */
    40,          /*  EGSM_MAX  IL=-80 */
    40,          /*  EGSM_MAX  IL=-81 */
    40,          /*  EGSM_MAX  IL=-82 */
    40,          /*  EGSM_MAX  IL=-83 */
    40,          /*  EGSM_MAX  IL=-84 */
    40,          /*  EGSM_MAX  IL=-85 */
    40,          /*  EGSM_MAX  IL=-86 */
    40,          /*  EGSM_MAX  IL=-87 */
    40,          /*  EGSM_MAX  IL=-88 */
    40,          /*  EGSM_MAX  IL=-89 */
    40,          /*  EGSM_MAX  IL=-90 */
    40,          /*  EGSM_MAX  IL=-91 */
    40,          /*  EGSM_MAX  IL=-92 */
    40,          /*  EGSM_MAX  IL=-93 */
    40,          /*  EGSM_MAX  IL=-94 */
    40,          /*  EGSM_MAX  IL=-95 */
    40,          /*  EGSM_MAX  IL=-96 */
    40,          /*  EGSM_MAX  IL=-97 */
    40,          /*  EGSM_MAX  IL=-98 */
    40,          /*  EGSM_MAX  IL=-99 */
    40,          /*  EGSM_MAX  IL=-100 */
    40,          /*  EGSM_MAX  IL=-101 */
    40,          /*  EGSM_MAX  IL=-102 */
    40,          /*  EGSM_MAX  IL=-103 */
    40,          /*  EGSM_MAX  IL=-104 */
    40,          /*  EGSM_MAX  IL=-105 */
    40,          /*  EGSM_MAX  IL=-106 */
    40,          /*  EGSM_MAX  IL=-107 */
    40,          /*  EGSM_MAX  IL=-108 */
    40,          /*  EGSM_MAX  IL=-109 */
    40,          /*  EGSM_MAX  IL=-110 */
    40,          /*  EGSM_MAX  IL=-111 */
    40,          /*  EGSM_MAX  IL=-112 */
    40,          /*  EGSM_MAX  IL=-113 */
    40,          /*  EGSM_MAX  IL=-114 */
    40,          /*  EGSM_MAX  IL=-115 */
    40,          /*  EGSM_MAX  IL=-116 */
    40,          /*  EGSM_MAX  IL=-117 */
    40,          /*  EGSM_MAX  IL=-118 */
    40,          /*  EGSM_MAX  IL=-119 */
    40           /*  EGSM_MAX  IL=-120 */
        },
        { // il2agc_av
          // Note this is shared between PCN and EGSM.
    14,          /*  EGSM_MAX  IL=0 */
    14,          /*  EGSM_MAX  IL=-1 */
    14,          /*  EGSM_MAX  IL=-2 */
    14,          /*  EGSM_MAX  IL=-3 */
    14,          /*  EGSM_MAX  IL=-4 */
    14,          /*  EGSM_MAX  IL=-5 */
    14,          /*  EGSM_MAX  IL=-6 */
    14,          /*  EGSM_MAX  IL=-7 */
    14,          /*  EGSM_MAX  IL=-8 */
    14,          /*  EGSM_MAX  IL=-9 */
    14,          /*  EGSM_MAX  IL=-10 */
    14,          /*  EGSM_MAX  IL=-11 */
    14,          /*  EGSM_MAX  IL=-12 */
    14,          /*  EGSM_MAX  IL=-13 */
    14,          /*  EGSM_MAX  IL=-14 */
    14,          /*  EGSM_MAX  IL=-15 */
    14,          /*  EGSM_MAX  IL=-16 */
    14,          /*  EGSM_MAX  IL=-17 */
    14,          /*  EGSM_MAX  IL=-18 */
    14,          /*  EGSM_MAX  IL=-19 */
    14,          /*  EGSM_MAX  IL=-20 */
    14,          /*  EGSM_MAX  IL=-21 */
    14,          /*  EGSM_MAX  IL=-22 */
    14,          /*  EGSM_MAX  IL=-23 */
    14,          /*  EGSM_MAX  IL=-24 */
    14,          /*  EGSM_MAX  IL=-25 */
    14,          /*  EGSM_MAX  IL=-26 */
    14,          /*  EGSM_MAX  IL=-27 */
    14,          /*  EGSM_MAX  IL=-28 */
    14,          /*  EGSM_MAX  IL=-29 */
    14,          /*  EGSM_MAX  IL=-30 */
    14,          /*  EGSM_MAX  IL=-31 */
    14,          /*  EGSM_MAX  IL=-32 */
    14,          /*  EGSM_MAX  IL=-33 */
    14,          /*  EGSM_MAX  IL=-34 */
    14,          /*  EGSM_MAX  IL=-35 */
    14,          /*  EGSM_MAX  IL=-36 */
    14,          /*  EGSM_MAX  IL=-37 */
    14,          /*  EGSM_MAX  IL=-38 */
    14,          /*  EGSM_MAX  IL=-39 */
    14,          /*  EGSM_MAX  IL=-40 */
    14,          /*  EGSM_MAX  IL=-41 */
    14,          /*  EGSM_MAX  IL=-42 */
    14,          /*  EGSM_MAX  IL=-43 */
    14,          /*  EGSM_MAX  IL=-44 */
    14,          /*  EGSM_MAX  IL=-45 */
    14,          /*  EGSM_MAX  IL=-46 */
    14,          /*  EGSM_MAX  IL=-47 */
    14,          /*  EGSM_MAX  IL=-48 */
    14,          /*  EGSM_MAX  IL=-49 */
    14,          /*  EGSM_MAX  IL=-50 */
    14,          /*  EGSM_MAX  IL=-51 */
    14,          /*  EGSM_MAX  IL=-52 */
    14,          /*  EGSM_MAX  IL=-53 */
    14,          /*  EGSM_MAX  IL=-54 */
    16,          /*  EGSM_MAX  IL=-55 */
    16,          /*  EGSM_MAX  IL=-56 */
    18,          /*  EGSM_MAX  IL=-57 */
    18,          /*  EGSM_MAX  IL=-58 */
    20,          /*  EGSM_MAX  IL=-59 */
    20,          /*  EGSM_MAX  IL=-60 */
    22,          /*  EGSM_MAX  IL=-61 */
    22,          /*  EGSM_MAX  IL=-62 */
    24,          /*  EGSM_MAX  IL=-63 */
    24,          /*  EGSM_MAX  IL=-64 */
    26,          /*  EGSM_MAX  IL=-65 */
    26,          /*  EGSM_MAX  IL=-66 */
    28,          /*  EGSM_MAX  IL=-67 */
    28,          /*  EGSM_MAX  IL=-68 */
    30,          /*  EGSM_MAX  IL=-69 */
    30,          /*  EGSM_MAX  IL=-70 */
    32,          /*  EGSM_MAX  IL=-71 */
    32,          /*  EGSM_MAX  IL=-72 */
    34,          /*  EGSM_MAX  IL=-73 */
    34,          /*  EGSM_MAX  IL=-74 */
    36,          /*  EGSM_MAX  IL=-75 */
    36,          /*  EGSM_MAX  IL=-76 */
    38,          /*  EGSM_MAX  IL=-77 */
    38,          /*  EGSM_MAX  IL=-78 */
    40,          /*  EGSM_MAX  IL=-79 */
    40,          /*  EGSM_MAX  IL=-80 */
    40,          /*  EGSM_MAX  IL=-81 */
    40,          /*  EGSM_MAX  IL=-82 */
    40,          /*  EGSM_MAX  IL=-83 */
    40,          /*  EGSM_MAX  IL=-84 */
    40,          /*  EGSM_MAX  IL=-85 */
    40,          /*  EGSM_MAX  IL=-86 */
    40,          /*  EGSM_MAX  IL=-87 */
    40,          /*  EGSM_MAX  IL=-88 */
    40,          /*  EGSM_MAX  IL=-89 */
    40,          /*  EGSM_MAX  IL=-90 */
    40,          /*  EGSM_MAX  IL=-91 */
    40,          /*  EGSM_MAX  IL=-92 */
    40,          /*  EGSM_MAX  IL=-93 */
    40,          /*  EGSM_MAX  IL=-94 */
    40,          /*  EGSM_MAX  IL=-95 */
    40,          /*  EGSM_MAX  IL=-96 */
    40,          /*  EGSM_MAX  IL=-97 */
    40,          /*  EGSM_MAX  IL=-98 */
    40,          /*  EGSM_MAX  IL=-99 */
    40,          /*  EGSM_MAX  IL=-100 */
    40,          /*  EGSM_MAX  IL=-101 */
    40,          /*  EGSM_MAX  IL=-102 */
    40,          /*  EGSM_MAX  IL=-103 */
    40,          /*  EGSM_MAX  IL=-104 */
    40,          /*  EGSM_MAX  IL=-105 */
    40,          /*  EGSM_MAX  IL=-106 */
    40,          /*  EGSM_MAX  IL=-107 */
    40,          /*  EGSM_MAX  IL=-108 */
    40,          /*  EGSM_MAX  IL=-109 */
    40,          /*  EGSM_MAX  IL=-110 */
    40,          /*  EGSM_MAX  IL=-111 */
    40,          /*  EGSM_MAX  IL=-112 */
    40,          /*  EGSM_MAX  IL=-113 */
    40,          /*  EGSM_MAX  IL=-114 */
    40,          /*  EGSM_MAX  IL=-115 */
    40,          /*  EGSM_MAX  IL=-116 */
    40,          /*  EGSM_MAX  IL=-117 */
    40,          /*  EGSM_MAX  IL=-118 */
    40,          /*  EGSM_MAX  IL=-119 */
    40           /*  EGSM_MAX  IL=-120 */
      }
    },
  },
  {
    {0, 0},     // ramp up and down delays
    GUARD_BITS, // number of guard bits needed for ramp up
    PRG_TX      // propagation delay PRG_TX
  },
  { //AFC parameters
    EEPROM_AFC,
    C_Psi_sta_inv,     // (1/C_Psi_sta)
    C_Psi_st,          // C_Psi_sta * 0.8 F0.16
    C_Psi_st_32,       // F0.32
    C_Psi_st_inv       // (1/C_Psi_st)

#if (VCXO_ALGO==1)
     ,C_AFC_DAC_CENTER,      // VCXO startup parameter - best guess
      C_AFC_DAC_MIN,         // VCXO startup parameter - 15ppm
      C_AFC_DAC_MAX,         // VCXO startup parameter + 15ppm
      C_AFC_SNR_THR         // snr - Default threshold value
#endif
  }
};

/* uninitialised rf struct for bands */
T_RF_BAND rf_band[GSM_BANDS] __attribute__ ((section ("int.bss")));

/*
 * The const T_RF_BAND rf_{900,1800,850,1900} structures that follow
 * are the versions that appear in the .const section of l1_cust.obj
 * in the l1_custom_int.lib Leonardo blob, used successfully in leo2moko.
 * As revealed with objgrep, this .const section with these uncalibrated
 * defaults in it also appears in the moko11 binary - I'm guessing that
 * Openmoko probably had no source for this part either, and used TI's
 * standard Leonardo binary lib.  The pretty C formatting presented here
 * is courtesy of the calextract utility.
 *
 * Please note that these hard-coded "calibration" values are mostly
 * decorative: when running on an actual GSM device (at least on GTA0x
 * and Pirelli targets; dunno how we'll handle Compal, if at all),
 * almost everything in these tables will be overridden with the "real"
 * calibration data read from MokoFFS or from Pirelli's factory data
 * block.  The only bits that remain from these hard-coded structs are
 * the Rx and Tx temperature compensation tables, and these have been
 * found to be identical between mv100/l1_rf12.c, the bits in the Leonardo
 * object blob (and hence in moko11 too), and the hard-coded structs
 * found in Pirelli's fw image.
 */

#if (ORDER2_TX_TEMP_CAL != 1)
#error "Hard-coded T_RF_BAND structs expect ORDER2_TX_TEMP_CAL to be 1"
#endif

const T_RF_BAND rf_900 = {
  { /* Rx structure */
    { /* T_RX_CAL_PARAMS */
       193,
        40,
        40,
        44,
    },
    { /* T_RF_AGC_BANDs */
      {   10,     0},
      {   30,     0},
      {   51,     0},
      {   71,     0},
      {   90,     0},
      {  112,     0},
      {  124,     0},
      {  991,     0},
      {  992,     0},
      { 1023,     0},
    },
    { /* Rx temperature compensation */
      {   -15,     0},
      {    -5,     0},
      {     6,     0},
      {    16,     0},
      {    25,     0},
      {    35,     0},
      {    45,     0},
      {    56,     0},
      {    66,     0},
      {    75,     0},
      {   100,     0},
    },
  },
  { /* Tx structure */
    { /* levels */
      {  465,  0,  0}, /* 0 */
      {  465,  0,  0}, /* 1 */
      {  465,  0,  0}, /* 2 */
      {  465,  0,  0}, /* 3 */
      {  465,  0,  0}, /* 4 */
      {  465,  0,  0}, /* 5 */
      {  387,  1,  0}, /* 6 */
      {  324,  2,  0}, /* 7 */
      {  260,  3,  0}, /* 8 */
      {  210,  4,  0}, /* 9 */
      {  170,  5,  0}, /* 10 */
      {  138,  6,  0}, /* 11 */
      {  113,  7,  0}, /* 12 */
      {   92,  8,  0}, /* 13 */
      {   76,  9,  0}, /* 14 */
      {   62, 10,  0}, /* 15 */
      {   51, 11,  0}, /* 16 */
      {   42, 12,  0}, /* 17 */
      {   34, 13,  0}, /* 18 */
      {   27, 14,  0}, /* 19 */
      {   27, 14,  0}, /* 20 */
      {   27, 14,  0}, /* 21 */
      {   27, 14,  0}, /* 22 */
      {   27, 14,  0}, /* 23 */
      {   27, 14,  0}, /* 24 */
      {   27, 14,  0}, /* 25 */
      {   27, 14,  0}, /* 26 */
      {   27, 14,  0}, /* 27 */
      {   27, 14,  0}, /* 28 */
      {   27, 14,  0}, /* 29 */
      {   27, 14,  0}, /* 30 */
      {   27, 14,  0}, /* 31 */
    },
    { /* channel calibration tables */
      { /* calibration table 0 */
	{   40,   128},
	{   80,   128},
	{  124,   128},
	{  586,   128},
	{  661,   128},
	{  736,   128},
	{  885,   128},
	{ 1023,   128},
      },
      { /* calibration table 1 */
	{   40,   128},
	{   80,   128},
	{  124,   128},
	{  586,   128},
	{  661,   128},
	{  736,   128},
	{  885,   128},
	{ 1023,   128},
      },
      { /* calibration table 2 */
	{   40,   128},
	{   80,   128},
	{  124,   128},
	{  586,   128},
	{  661,   128},
	{  736,   128},
	{  885,   128},
	{ 1023,   128},
      },
      { /* calibration table 3 */
	{   40,   128},
	{   80,   128},
	{  124,   128},
	{  586,   128},
	{  661,   128},
	{  736,   128},
	{  885,   128},
	{ 1023,   128},
      },
    },
    { /* ramps */
      { /* profile 0 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  0,  0,  9, 18, 25, 31, 30, 15,  0,  0},
	/* ramp-down */
	{  0, 11, 31, 31, 31, 24,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 1 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  1,  1,  7, 16, 28, 31, 31, 13,  0,  0},
	/* ramp-down */
	{  0,  8, 31, 31, 31, 27,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 2 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  1,  1,  8, 16, 29, 31, 31, 11,  0,  0},
	/* ramp-down */
	{  0,  8, 28, 31, 31, 30,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 3 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  2,  0,  6, 18, 28, 31, 31, 12,  0,  0},
	/* ramp-down */
	{  0,  9, 24, 31, 31, 31,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 4 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  3,  0,  5, 19, 31, 31, 31,  8,  0,  0},
	/* ramp-down */
	{  0,  7, 31, 31, 31, 28,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 5 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  2,  0,  7, 18, 31, 31, 31,  8,  0,  0},
	/* ramp-down */
	{  0,  7, 31, 31, 31, 28,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 6 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  3,  0,  5, 20, 31, 31, 31,  7,  0,  0},
	/* ramp-down */
	{  0, 10, 21, 31, 31, 31,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 7 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  4,  0,  9, 23, 22, 31, 31,  8,  0,  0},
	/* ramp-down */
	{  0,  9, 24, 30, 31, 30,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 8 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  5,  0,  8, 21, 24, 31, 31,  8,  0,  0},
	/* ramp-down */
	{  0,  8, 23, 31, 31, 31,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 9 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  5,  0,  3,  1, 27, 22, 31, 31,  8,  0,  0},
	/* ramp-down */
	{  0,  8, 27, 25, 26, 31, 11,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 10 */
	/* ramp-up */
	{  0,  0,  0,  0,  5,  0,  0,  2,  7, 22, 23, 31, 31,  7,  0,  0},
	/* ramp-down */
	{  0,  7, 25, 30, 31, 31,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 11 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  5,  0,  4,  8, 21, 21, 31, 31,  7,  0,  0},
	/* ramp-down */
	{  0,  8, 21, 31, 31, 31,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 12 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  7,  0,  0, 12, 22, 25, 31, 27,  4,  0,  0},
	/* ramp-down */
	{  0,  9, 12, 21, 31, 31, 24,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 13 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  7,  0,  8, 15, 31, 31, 31,  5,  0,  0},
	/* ramp-down */
	{  0,  6, 14, 23, 31, 31, 23,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 14 */
	/* ramp-up */
	{  0,  0,  0,  0,  0, 20,  0,  0,  8, 15, 14, 31, 31,  9,  0,  0},
	/* ramp-down */
	{  0,  7, 31, 31, 31, 28,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 15 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	/* ramp-down */
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
    },
    { /* Tx temperature compensation */
      {   -11,     0,     0,     0},
      {     9,     0,     0,     0},
      {    39,     0,     0,     0},
      {    59,     0,     0,     0},
      {   127,     0,     0,     0},
    },
  },
  //IQ swap
  SWAP_IQ_GSM,
};

const T_RF_BAND rf_1800 = {
  { /* Rx structure */
    { /* T_RX_CAL_PARAMS */
       188,
        40,
        40,
        44,
    },
    { /* T_RF_AGC_BANDs */
      {  548,     0},
      {  622,     0},
      {  680,     0},
      {  745,     0},
      {  812,     0},
      {  860,     0},
      {  885,     0},
      {  991,     0},
      {  992,     0},
      { 1023,     0},
    },
    { /* Rx temperature compensation */
      {   -15,     0},
      {    -5,     0},
      {     6,     0},
      {    16,     0},
      {    25,     0},
      {    35,     0},
      {    45,     0},
      {    56,     0},
      {    66,     0},
      {    75,     0},
      {   100,     0},
    },
  },
  { /* Tx structure */
    { /* levels */
      {  436,  0,  0}, /* 0 */
      {  363,  1,  0}, /* 1 */
      {  310,  2,  0}, /* 2 */
      {  253,  3,  0}, /* 3 */
      {  205,  4,  0}, /* 4 */
      {  168,  5,  0}, /* 5 */
      {  138,  6,  0}, /* 6 */
      {  113,  7,  0}, /* 7 */
      {   93,  8,  0}, /* 8 */
      {   76,  9,  0}, /* 9 */
      {   61, 10,  0}, /* 10 */
      {   50, 11,  0}, /* 11 */
      {   40, 12,  0}, /* 12 */
      {   32, 13,  0}, /* 13 */
      {   26, 14,  0}, /* 14 */
      {   20, 15,  0}, /* 15 */
      {   20, 15,  0}, /* 16 */
      {   20, 15,  0}, /* 17 */
      {   20, 15,  0}, /* 18 */
      {   20, 15,  0}, /* 19 */
      {   20, 15,  0}, /* 20 */
      {   20, 15,  0}, /* 21 */
      {   20, 15,  0}, /* 22 */
      {   20, 15,  0}, /* 23 */
      {   20, 15,  0}, /* 24 */
      {   20, 15,  0}, /* 25 */
      {   20, 15,  0}, /* 26 */
      {   20, 15,  0}, /* 27 */
      {   20, 15,  0}, /* 28 */
      {   20,  0,  0}, /* 29 */
      {   20,  0,  0}, /* 30 */
      {   20,  0,  0}, /* 31 */
    },
    { /* channel calibration tables */
      { /* calibration table 0 */
	{  554,   128},
	{  722,   128},
	{  746,   128},
	{  774,   128},
	{  808,   128},
	{  851,   128},
	{  870,   128},
	{  885,   128},
      },
      { /* calibration table 1 */
	{  554,   128},
	{  722,   128},
	{  746,   128},
	{  774,   128},
	{  808,   128},
	{  851,   128},
	{  870,   128},
	{  885,   128},
      },
      { /* calibration table 2 */
	{  554,   128},
	{  722,   128},
	{  746,   128},
	{  774,   128},
	{  808,   128},
	{  851,   128},
	{  870,   128},
	{  885,   128},
      },
      { /* calibration table 3 */
	{  554,   128},
	{  722,   128},
	{  746,   128},
	{  774,   128},
	{  808,   128},
	{  851,   128},
	{  870,   128},
	{  885,   128},
      },
    },
    { /* ramps */
      { /* profile 0 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  2,  3,  5, 16, 31, 31, 31,  9,  0,  0},
	/* ramp-down */
	{  0, 11, 31, 31, 31, 10, 11,  3,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 1 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  2,  3,  4, 17, 30, 31, 31, 10,  0,  0},
	/* ramp-down */
	{  0, 10, 31, 31, 31, 13,  9,  3,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 2 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  4,  2,  2, 18, 31, 31, 31,  9,  0,  0},
	/* ramp-down */
	{  0, 10, 26, 31, 31, 16, 10,  4,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 3 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  3,  4,  4, 15, 31, 31, 31,  9,  0,  0},
	/* ramp-down */
	{  0,  9, 31, 31, 31, 13,  6,  7,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 4 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  4,  3,  7, 11, 31, 31, 31, 10,  0,  0},
	/* ramp-down */
	{  0,  8, 31, 31, 31, 11,  9,  7,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 5 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  4,  3,  2,  7, 14, 25, 31, 31, 11,  0,  0},
	/* ramp-down */
	{  0, 14, 31, 31, 31,  9,  8,  4,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 6 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  7,  1,  3, 10, 12, 25, 31, 31,  8,  0,  0},
	/* ramp-down */
	{  0,  7, 30, 31, 31, 14,  4,  6,  5,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 7 */
	/* ramp-up */
	{  0,  0,  0,  0,  3,  5,  0,  5,  8, 12, 26, 31, 31,  7,  0,  0},
	/* ramp-down */
	{  0,  7, 31, 31, 31, 15,  0,  8,  5,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 8 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  9,  0,  3, 10, 16, 21, 31, 31,  7,  0,  0},
	/* ramp-down */
	{  0, 11, 28, 31, 27, 10, 11,  0, 10,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 9 */
	/* ramp-up */
	{  0,  0,  0,  0,  0, 10,  0,  6,  9, 15, 22, 29, 31,  6,  0,  0},
	/* ramp-down */
	{  0,  9, 22, 31, 31, 12,  5,  0, 18,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 10 */
	/* ramp-up */
	{  0,  0,  0,  0, 14,  0,  0,  8,  6, 20, 21, 29, 24,  6,  0,  0},
	/* ramp-down */
	{  0,  8, 28, 29, 26, 14,  6,  0, 17,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 11 */
	/* ramp-up */
	{  0,  0,  0,  0, 16,  0,  3,  5,  8, 16, 31, 28, 18,  3,  0,  0},
	/* ramp-down */
	{  0,  6, 18, 26, 31, 16,  9,  7,  0, 15,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 12 */
	/* ramp-up */
	{  0,  0,  0,  0, 19,  0,  3,  6,  8, 21, 24, 31, 14,  2,  0,  0},
	/* ramp-down */
	{  0,  0, 12, 31, 31, 27,  4,  0, 23,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 13 */
	/* ramp-up */
	{  0,  0,  0,  0,  0, 14, 14,  0,  0, 24, 31, 31, 14,  0,  0,  0},
	/* ramp-down */
	{  0,  0, 11, 31, 31, 22, 11,  3, 19,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 14 */
	/* ramp-up */
	{  0,  0,  0,  0,  0, 30,  1,  4,  8, 18, 31, 31,  5,  0,  0,  0},
	/* ramp-down */
	{  0,  0,  8, 31, 31, 22,  5,  0, 31,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 15 */
	/* ramp-up */
	{  0,  0,  0,  0,  0, 31, 13,  0,  0, 14, 31, 31,  8,  0,  0,  0},
	/* ramp-down */
	{  0,  0,  4, 31, 31, 25,  5,  0,  5, 26,  1,  0,  0,  0,  0,  0},
      },
    },
    { /* Tx temperature compensation */
      {   -11,     0,     0,     0},
      {     9,     0,     0,     0},
      {    39,     0,     0,     0},
      {    59,     0,     0,     0},
      {   127,     0,     0,     0},
    },
  },
  //IQ swap
  SWAP_IQ_DCS
};

const T_RF_BAND rf_850 = {
  { /* Rx structure */
    { /* T_RX_CAL_PARAMS */
       181,
        40,
        40,
        44,
    },
    { /* T_RF_AGC_BANDs */
      {   10,     0},
      {   30,     0},
      {   51,     0},
      {   71,     0},
      {   90,     0},
      {  112,     0},
      {  124,     0},
      {  991,     0},
      {  992,     0},
      { 1023,     0},
    },
    { /* Rx temperature compensation */
      {   -15,     0},
      {    -5,     0},
      {     6,     0},
      {    16,     0},
      {    25,     0},
      {    35,     0},
      {    45,     0},
      {    56,     0},
      {    66,     0},
      {    75,     0},
      {   100,     0},
    },
  },
  { /* Tx structure */
    { /* levels */
      {  507,  0,  0}, /* 0 */
      {  507,  0,  0}, /* 1 */
      {  507,  0,  0}, /* 2 */
      {  507,  0,  0}, /* 3 */
      {  507,  0,  0}, /* 4 */
      {  507,  0,  0}, /* 5 */
      {  417,  1,  0}, /* 6 */
      {  350,  2,  0}, /* 7 */
      {  282,  3,  0}, /* 8 */
      {  226,  4,  0}, /* 9 */
      {  183,  5,  0}, /* 10 */
      {  148,  6,  0}, /* 11 */
      {  121,  7,  0}, /* 12 */
      {   98,  8,  0}, /* 13 */
      {   80,  9,  0}, /* 14 */
      {   66, 10,  0}, /* 15 */
      {   54, 11,  0}, /* 16 */
      {   44, 12,  0}, /* 17 */
      {   36, 13,  0}, /* 18 */
      {   29, 14,  0}, /* 19 */
      {   29, 14,  0}, /* 20 */
      {   29, 14,  0}, /* 21 */
      {   29, 14,  0}, /* 22 */
      {   29, 14,  0}, /* 23 */
      {   29, 14,  0}, /* 24 */
      {   29, 14,  0}, /* 25 */
      {   29, 14,  0}, /* 26 */
      {   29, 14,  0}, /* 27 */
      {   29, 14,  0}, /* 28 */
      {   29, 14,  0}, /* 29 */
      {   29, 14,  0}, /* 30 */
      {   29, 14,  0}, /* 31 */
    },
    { /* channel calibration tables */
      { /* calibration table 0 */
	{   40,   128},
	{   80,   128},
	{  124,   128},
	{  586,   128},
	{  661,   128},
	{  736,   128},
	{  885,   128},
	{ 1023,   128},
      },
      { /* calibration table 1 */
#if 0
	/*
	 * This bogon appears in the l1_custom_int.lib Leonardo blob
	 * and in Openmoko's official firmwares, from which it has
	 * propagated into the /gsm/rf/tx/calchan.850 file programmed
	 * into every produced GTA02 unit, or at least the units
	 * without the 850 MHz band.  It also appears in mv100/l1_rf12.c
	 * and in the "dead" rf_850 table in Pirelli's fw binary.
	 * Whew!
	 */
	{  554,   130},
	{  722,   128},
	{  746,   129},
	{  774,   131},
	{  808,   132},
	{  851,   134},
	{  870,   138},
	{  885,   140},
#else
	/* make it the same as the others */
	{   40,   128},
	{   80,   128},
	{  124,   128},
	{  586,   128},
	{  661,   128},
	{  736,   128},
	{  885,   128},
	{ 1023,   128},
#endif
      },
      { /* calibration table 2 */
	{   40,   128},
	{   80,   128},
	{  124,   128},
	{  586,   128},
	{  661,   128},
	{  736,   128},
	{  885,   128},
	{ 1023,   128},
      },
      { /* calibration table 3 */
	{   40,   128},
	{   80,   128},
	{  124,   128},
	{  586,   128},
	{  661,   128},
	{  736,   128},
	{  885,   128},
	{ 1023,   128},
      },
    },
    { /* ramps */
      { /* profile 0 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  0,  0,  9, 18, 25, 31, 30, 15,  0,  0},
	/* ramp-down */
	{  0, 11, 31, 31, 31, 24,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 1 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  1,  1,  7, 16, 28, 31, 31, 13,  0,  0},
	/* ramp-down */
	{  0,  8, 31, 31, 31, 27,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 2 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  1,  1,  8, 16, 29, 31, 31, 11,  0,  0},
	/* ramp-down */
	{  0,  8, 28, 31, 31, 30,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 3 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  2,  0,  6, 18, 28, 31, 31, 12,  0,  0},
	/* ramp-down */
	{  0,  9, 24, 31, 31, 31,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 4 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  3,  0,  5, 19, 31, 31, 31,  8,  0,  0},
	/* ramp-down */
	{  0,  7, 31, 31, 31, 28,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 5 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  2,  0,  7, 18, 31, 31, 31,  8,  0,  0},
	/* ramp-down */
	{  0,  7, 31, 31, 31, 28,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 6 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  3,  0,  5, 20, 31, 31, 31,  7,  0,  0},
	/* ramp-down */
	{  0, 10, 21, 31, 31, 31,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 7 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  4,  0,  9, 23, 22, 31, 31,  8,  0,  0},
	/* ramp-down */
	{  0,  9, 24, 30, 31, 30,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 8 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  5,  0,  8, 21, 24, 31, 31,  8,  0,  0},
	/* ramp-down */
	{  0,  8, 23, 31, 31, 31,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 9 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  5,  0,  3,  1, 27, 22, 31, 31,  8,  0,  0},
	/* ramp-down */
	{  0,  8, 27, 25, 26, 31, 11,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 10 */
	/* ramp-up */
	{  0,  0,  0,  0,  5,  0,  0,  2,  7, 22, 23, 31, 31,  7,  0,  0},
	/* ramp-down */
	{  0,  7, 25, 30, 31, 31,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 11 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  5,  0,  4,  8, 21, 21, 31, 31,  7,  0,  0},
	/* ramp-down */
	{  0,  8, 21, 31, 31, 31,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 12 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  7,  0,  0, 12, 22, 25, 31, 27,  4,  0,  0},
	/* ramp-down */
	{  0,  9, 12, 21, 31, 31, 24,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 13 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  7,  0,  8, 15, 31, 31, 31,  5,  0,  0},
	/* ramp-down */
	{  0,  6, 14, 23, 31, 31, 23,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 14 */
	/* ramp-up */
	{  0,  0,  0,  0,  0, 20,  0,  0,  8, 15, 14, 31, 31,  9,  0,  0},
	/* ramp-down */
	{  0,  7, 31, 31, 31, 28,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 15 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	/* ramp-down */
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      },
    },
    { /* Tx temperature compensation */
      {   -11,     0,     0,     0},
      {     9,     0,     0,     0},
      {    39,     0,     0,     0},
      {    59,     0,     0,     0},
      {   127,     0,     0,     0},
    },
  },
  //IQ swap
  SWAP_IQ_GSM850,
};

const T_RF_BAND rf_1900 = {
  { /* Rx structure */
    { /* T_RX_CAL_PARAMS */
       188,
        40,
        40,
        44,
    },
    { /* T_RF_AGC_BANDs */
      {  548,     0},
      {  622,     0},
      {  680,     0},
      {  745,     0},
      {  812,     0},
      {  860,     0},
      {  885,     0},
      {  991,     0},
      {  992,     0},
      { 1023,     0},
    },
    { /* Rx temperature compensation */
      {   -15,     0},
      {    -5,     0},
      {     6,     0},
      {    16,     0},
      {    25,     0},
      {    35,     0},
      {    45,     0},
      {    56,     0},
      {    66,     0},
      {    75,     0},
      {   100,     0},
    },
  },
  { /* Tx structure */
    { /* levels */
      {  429,  0,  0}, /* 0 */
      {  353,  1,  0}, /* 1 */
      {  302,  2,  0}, /* 2 */
      {  246,  3,  0}, /* 3 */
      {  200,  4,  0}, /* 4 */
      {  164,  5,  0}, /* 5 */
      {  135,  6,  0}, /* 6 */
      {  111,  7,  0}, /* 7 */
      {   91,  8,  0}, /* 8 */
      {   75,  9,  0}, /* 9 */
      {   60, 10,  0}, /* 10 */
      {   49, 11,  0}, /* 11 */
      {   40, 12,  0}, /* 12 */
      {   33, 13,  0}, /* 13 */
      {   26, 14,  0}, /* 14 */
      {   26, 15,  0}, /* 15 */
      {   26, 15,  0}, /* 16 */
      {   26, 15,  0}, /* 17 */
      {   26, 15,  0}, /* 18 */
      {   26, 15,  0}, /* 19 */
      {   26, 15,  0}, /* 20 */
      {   26, 15,  0}, /* 21 */
      {   26, 15,  0}, /* 22 */
      {   26, 15,  0}, /* 23 */
      {   26, 15,  0}, /* 24 */
      {   26, 15,  0}, /* 25 */
      {   26, 15,  0}, /* 26 */
      {   26, 15,  0}, /* 27 */
      {   26, 15,  0}, /* 28 */
      {   26,  0,  0}, /* 29 */
      {   26,  0,  0}, /* 30 */
      {   26,  0,  0}, /* 31 */
    },
    { /* channel calibration tables */
      { /* calibration table 0 */
	{  554,   128},
	{  722,   128},
	{  746,   128},
	{  774,   128},
	{  808,   128},
	{  810,   128},
	{  810,   128},
	{  810,   128},
      },
      { /* calibration table 1 */
	{  554,   128},
	{  722,   128},
	{  746,   128},
	{  774,   128},
	{  808,   128},
	{  810,   128},
	{  810,   128},
	{  810,   128},
      },
      { /* calibration table 2 */
	{  554,   128},
	{  722,   128},
	{  746,   128},
	{  774,   128},
	{  808,   128},
	{  810,   128},
	{  810,   128},
	{  810,   128},
      },
      { /* calibration table 3 */
	{  554,   128},
	{  722,   128},
	{  746,   128},
	{  774,   128},
	{  808,   128},
	{  810,   128},
	{  810,   128},
	{  810,   128},
      },
    },
    { /* ramps */
      { /* profile 0 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  7,  0,  0, 16, 31, 31, 31, 12,  0,  0},
	/* ramp-down */
	{  0, 13, 31, 31, 31, 18,  0,  4,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 1 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  2,  3,  4, 17, 30, 31, 31, 10,  0,  0},
	/* ramp-down */
	{  0, 10, 31, 31, 31, 13,  9,  3,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 2 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  4,  2,  2, 18, 31, 31, 31,  9,  0,  0},
	/* ramp-down */
	{  0, 10, 26, 31, 31, 16, 10,  4,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 3 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  3,  4,  4, 15, 31, 31, 31,  9,  0,  0},
	/* ramp-down */
	{  0,  9, 31, 31, 31, 13,  6,  7,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 4 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  0,  4,  3,  0, 18, 31, 31, 31, 10,  0,  0},
	/* ramp-down */
	{  0,  8, 31, 31, 31, 11,  9,  7,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 5 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  4,  3,  2,  7, 14, 25, 31, 31, 11,  0,  0},
	/* ramp-down */
	{  0, 14, 31, 31, 31,  9,  8,  4,  0,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 6 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  7,  1,  3, 10, 12, 25, 31, 31,  8,  0,  0},
	/* ramp-down */
	{  0,  7, 30, 31, 31, 14,  4,  6,  5,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 7 */
	/* ramp-up */
	{  0,  0,  0,  0,  3,  5,  0,  5,  8, 12, 26, 31, 31,  7,  0,  0},
	/* ramp-down */
	{  0,  7, 31, 31, 31, 15,  0,  8,  5,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 8 */
	/* ramp-up */
	{  0,  0,  0,  0,  0,  9,  0,  3, 10, 16, 21, 31, 31,  7,  0,  0},
	/* ramp-down */
	{  0, 11, 28, 31, 27, 10, 11,  0, 10,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 9 */
	/* ramp-up */
	{  0,  0,  0,  0,  0, 10,  0,  6,  9, 15, 22, 29, 31,  6,  0,  0},
	/* ramp-down */
	{  0,  9, 22, 31, 31, 12,  5,  0, 18,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 10 */
	/* ramp-up */
	{  0,  0,  0,  0, 14,  0,  0,  4, 10, 20, 21, 29, 24,  6,  0,  0},
	/* ramp-down */
	{  0,  8, 28, 29, 26, 14,  6,  0, 17,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 11 */
	/* ramp-up */
	{  0,  0,  0,  0, 16,  0,  3,  5,  8, 16, 31, 28, 18,  3,  0,  0},
	/* ramp-down */
	{  0,  6, 18, 26, 31, 16,  9,  7,  0, 15,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 12 */
	/* ramp-up */
	{  0,  0,  0,  0, 19,  0,  3,  6,  8, 21, 24, 31, 14,  2,  0,  0},
	/* ramp-down */
	{  0,  0, 12, 31, 31, 27,  4,  0, 23,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 13 */
	/* ramp-up */
	{  0,  0,  0,  0,  0, 14, 14,  0,  0, 24, 31, 31, 14,  0,  0,  0},
	/* ramp-down */
	{  0,  0, 11, 31, 31, 22, 11,  3, 19,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 14 */
	/* ramp-up */
	{  0,  0,  0,  0,  0, 30,  1,  4,  8, 18, 31, 31,  5,  0,  0,  0},
	/* ramp-down */
	{  0,  0,  8, 31, 31, 22,  5,  0, 31,  0,  0,  0,  0,  0,  0,  0},
      },
      { /* profile 15 */
	/* ramp-up */
	{  0,  0,  0,  0,  0, 30,  1,  4,  8, 18, 31, 31,  5,  0,  0,  0},
	/* ramp-down */
	{  0,  0,  8, 31, 31, 22,  5,  0, 31,  0,  0,  0,  0,  0,  0,  0},
      },
    },
    { /* Tx temperature compensation */
      {   -11,     0,     0,     0},
      {     9,     0,     0,     0},
      {    39,     0,     0,     0},
      {    59,     0,     0,     0},
      {   127,     0,     0,     0},
    },
  },
  //IQ swap
  SWAP_IQ_PCS
};

/*
 * The following part is unchanged from mv100/l1_rf12.c; the initialization
 * values come from l1_rf12.h, and we got that header from the Leonardo
 * version, so we trust it to be correct.  Some of the constants got
 * renamed between the two versions, though.
 */

/*------------------------------------------*/
/* ABB Initialization words
/*------------------------------------------*/
#if (ANALOG == 1)
  UWORD16 abb[ABB_TABLE_SIZE] =
  {
    C_AFCCTLADD,  // Value at reset
    C_VBUCTRL,    // Uplink gain amp 0dB, Sidetone gain to mute
    C_VBDCTRL,    // Downlink gain amp 0dB, Volume control 0 dB
    C_BBCTRL,     // value at reset
    C_APCOFF,     // value at reset
    C_BULIOFF,    // value at reset
    C_BULQOFF,    // value at reset
    C_DAI_ON_OFF, // value at reset
    C_AUXDAC,     // value at reset
    C_VBCTRL,     // VULSWITCH=0, VDLAUX=1, VDLEAR=1
    C_APCDEL1     // value at reset
};
#elif (ANALOG == 2)
  UWORD16 abb[ABB_TABLE_SIZE] =
  {
    C_AFCCTLADD,
    C_VBUCTRL,
    C_VBDCTRL,
    C_BBCTRL,
    C_BULGCAL,
    C_APCOFF,
    C_BULIOFF,
    C_BULQOFF,
    C_DAI_ON_OFF,
    C_AUXDAC,
    C_VBCTRL1,
    C_VBCTRL2,
    C_APCDEL1,
    C_APCDEL2
  };

#elif (ANALOG == 3)
  UWORD16 abb[ABB_TABLE_SIZE] =
  {
    C_AFCCTLADD,
    C_VBUCTRL,
    C_VBDCTRL,
    C_BBCTRL,
    C_BULGCAL,
    C_APCOFF,
    C_BULIOFF,
    C_BULQOFF,
    C_DAI_ON_OFF,
    C_AUXDAC,
    C_VBCTRL1,
    C_VBCTRL2,
    C_APCDEL1,
    C_APCDEL2,
    C_VBPOP,
    C_VAUDINITD,
    C_VAUDCR,
    C_VAUOCR,
    C_VAUSCR,
    C_VAUDPLL
  };

#endif

/*------------------------------------------*/
/*             Gain table                   */
/*  specified in the TRF6053 spec           */
/*     2 dB steps - LNA always ON       */
/*------------------------------------------*/
UWORD16 AGC_TABLE[AGC_TABLE_SIZE] =
{
  0x00,  //reserved
  0x01,  //reserved
  0x02,  //reserved
  0x03,  //reserved
  0x04,  //reserved
  0x05,  //reserved
  0x06,  //14 dB
  0x07,  //16
  0x08,  //18
  0x09,  //20
  0x0a,  //22
  0x0b,  //24
  0x0c,  //26
  0x0d,  //28
  0x0e,  //30
  0x0f,  //32
  0x10,  //34
  0x11,  //36
  0x12,  //38
  0x13,  //40
  /*
  0x14,  //reserved
  0x15,  //reserved
  0x16,  //reserved
  0x17,  //reserved
  0x18,  //reserved
  0x19,  //reserved
  0x1a,  //reserved
  0x1b,  //reserved
  0x1c,  //reserved
  0x1d,  //reserved
  0x1e,  //reserved
  0x1f,  //reserved
  */
};

// structure for ADC conversion (4 Internal channel + 5 Ext channels max.)
T_ADC adc __attribute__ ((section ("int.bss")));

// MADC calibration structure
T_ADCCAL adc_cal=
{ // a: 0,..,8
  // b, 0,..,8
  // cal_a = 4*1750 is the Typical value 1.75 V ref voltage , divide by 4
  7000, 8750, 7000, 7000, 7000, 7000, 7000, 256, 7000,
     0,    0,    0,    0,    0,    0,   0,    0,    0
};

#if (BOARD == 41)
// table which converts ADC value into RF temperature
T_TEMP temperature[TEMP_TABLE_SIZE] =
{
// Temperature compensation for EVARITA - S.Glock, J.Demay 04/23/2003
  582, -40,
  640, -10,
  698, 25,
  756, 60,
  815, 90
};
#else
// table which converts ADC value into RF temperature
T_TEMP temperature[TEMP_TABLE_SIZE] =
{
  7, -35,
  7, -34,
  8, -33,
  8, -32,
  9, -31,
  9, -30,
  10, -29,
  11, -28,
  11, -27,
  12, -26,
  13, -25,
  14, -24,
  14, -23,
  15, -22,
  16, -21,
  17, -20,
  18, -19,
  19, -18,
  21, -17,
  22, -16,
  23, -15,
  24, -14,
  26, -13,
  27, -12,
  29, -11,
  30, -10,
  32, -9,
  34, -8,
  36, -7,
  37, -6,
  39, -5,
  41, -4,
  44, -3,
  46, -2,
  48, -1,
  51, 0,
  53, 1,
  56, 2,
  59, 3,
  61, 4,
  64, 5,
  68, 6,
  71, 7,
  74, 8,
  78, 9,
  81, 10,
  85, 11,
  89, 12,
  93, 13,
  97, 14,
  101, 15,
  105, 16,
  110, 17,
  115, 18,
  119, 19,
  124, 20,
  130, 21,
  135, 22,
  140, 23,
  146, 24,
  152, 25,
  158, 26,
  164, 27,
  170, 28,
  176, 29,
  183, 30,
  190, 31,
  197, 32,
  204, 33,
  211, 34,
  219, 35,
  226, 36,
  234, 37,
  242, 38,
  250, 39,
  259, 40,
  267, 41,
  276, 42,
  285, 43,
  294, 44,
  303, 45,
  312, 46,
  322, 47,
  331, 48,
  341, 49,
  351, 50,
  361, 51,
  371, 52,
  382, 53,
  392, 54,
  403, 55,
  413, 56,
  424, 57,
  435, 58,
  446, 59,
  458, 60,
  469, 61,
  480, 62,
  492, 63,
  503, 64,
  515, 65,
  527, 66,
  539, 67,
  550, 68,
  562, 69,
  574, 70,
  586, 71,
  598, 72,
  611, 73,
  623, 74,
  635, 75,
  647, 76,
  659, 77,
  671, 78,
  683, 79,
  696, 80,
  708, 81,
  720, 82,
  732, 83,
  744, 84,
  756, 85,
  768, 86,
  780, 87,
  792, 88,
  804, 89,
  816, 90,
  827, 91,
  839, 92,
  851, 93,
  862, 94,
  873, 95
};
#endif
