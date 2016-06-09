/*
+--------------------------------------------------------------------+
| PROJECT : XXX                        SOURCE : CST.H                |
| AUTHOR  : XXX                        VERSION: 1.0                  |
| CREATED : 01.02.99                   STATE  : code                 |
+--------------------------------------------------------------------+

   MODULE  : CST

   PURPOSE : Definitions for the protocol stack entity CST.
*/

#ifndef CST_H
#define CST_H

#include "../../include/config.h"

/*==== TEST =====================================================*/

/*
 * Dynamic Configuration Numbers
 */
#define ID_CONFIG                 1
#define ID_MUTE                   2
#define ID_GSM_PARAMETERS         11
#define ID_L1_PARAMETERS          12

/*
 * TIMER IDs
 */
#define T_RX 0   /* request of fieldstrength */
#define TMAX 0   /* must be the last one     */

/*
 * Configuration Parameter
 */
#define TCST1                0
#define TCST2                1


#if (CHIPSET == 0)
  #define ARMIO_CLK           0x0001
  #define RIF_CLKR            0x0002
  #define RIF_CLKX            0x0004
  #define RIF_CLK13           0x0010
  #define UWIRE_CLK           0x0020
  #define SIM_CLK             0x0040
  #define TSP_CLK             0x0080
  #define UART_CLK            0x0400
#endif

#if ((CHIPSET == 2) || (CHIPSET == 3) || (CHIPSET == 4) || \
     (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || \
     (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || \
     (CHIPSET == 11) || (CHIPSET == 12))
  #define ARMIO_CLK_CUT       0x0001
  #define UWIRE_CLK_CUT       0x0002
#endif

// ADC timer expiration value defining the ADC period
// with new frame all timer values are in ms not in tdma frames one TDMA approx 4.615 ms
#define ADC_PERIOD      4615

#ifdef ALR
//#define VM_BUFFER_SIZE 10240  // 10 seconds (1024 words for about 1 second of recording)
#endif


typedef struct
{
  UBYTE                 t_mode;
  ULONG                 t_val;
} T_TIMER_CONFIG;

#ifdef OPTION_TIMER
#define CST_TSTART(i,h,v) tim_start_timer(i,h,v)
#else
#define CST_TSTART(i,h,v) vsi_t_start(VSI_CALLER h,v)
#endif

#define TIMERSTART(i,v,h) csf_alloc_timer(i,v,&h)
#define TIMERSTOP(h) csf_free_timer(h); h = VSI_ERROR;

/*==== EXPORT =====================================================*/
/*
 * CST global data declarations
 */

#define CST_ADC_TIMER  0

/*
 * Prototypes Timer Modul
 */
/*
 * If all entities are linked into one module this definitions
 * prefixes all this functions with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define tim_init_timer       _ENTITY_PREFIXED(tim_init_timer)
  #define tim_set_timeout_flag _ENTITY_PREFIXED(tim_set_timeout_flag)
  #define tim_handle_timeout   _ENTITY_PREFIXED(tim_handle_timeout)
  #define tim_config_timer     _ENTITY_PREFIXED(tim_config_timer)
  #define tim_get_config_timer _ENTITY_PREFIXED(tim_get_config_timer)
  #define tim_start_timer      _ENTITY_PREFIXED(tim_start_timer)
  #define tim_flush_fifo       _ENTITY_PREFIXED(tim_flush_fifo)
#endif

#ifdef OPTION_TIMER
  /*
   * If all entities are linked into one module this definitions
   * prefixes the global data with the entity name
   */
  #ifdef OPTION_MULTITHREAD
    #define partab    _ENTITY_PREFIXED(partab)
  #endif

  EXTERN KW_DATA       partab[];
#endif

  /*
 * If all entities are linked into one module this definitions
 * prefixes the global data with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define hCommPL        _ENTITY_PREFIXED(hCommPL)
  #define hCommL1        _ENTITY_PREFIXED(hCommL1)
#endif

EXTERN T_HANDLE  hCommPL;         /* Communication to TI++    */
EXTERN T_HANDLE  hCommL1;         /* Communication to Layer 1 */
EXTERN T_HANDLE  cst_handle;

/*
 * Prototypes Customer Spefific Functions Modul
 */
/*
 * If all entities are linked into one module this definitions
 * prefixes all this functions with the enity name
 */
GLOBAL       void     csf_adc_process             (T_CST_ADC_IND *adc_results);
EXTERN       void     adc_start                   (void);
EXTERN       void     power_down_config           (UBYTE sleep_mode, USHORT clocks);
GLOBAL 	     void     csf_aec_enable 		  (USHORT aec_ctrl_reg);

#endif // CST_H
