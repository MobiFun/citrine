/*
 * This module contains the LISR and HISR glue functions for L1
 * which used to be in the dl1_com module in the Leonardo version.
 * The LoCosto source from which we got our L1 code no longer has a
 * dl1_com.c module, and the ISR glue functions in question have been
 * moved into csw-system/init_common/init.c - an incredibly messy C
 * module that is mostly devoted to LoCosto BSP initialization.
 *
 * The present C code has been extracted from LoCosto's init.c,
 * guided by the disassembly of dl1_com.obj from the Leonardo version.
 */

#include "config.h"
#include "sys_types.h"
#include "../../riviera/rv/general.h"
#include "../../nucleus/nucleus.h"

/* Include Files */
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "l1_types.h"
#include "l1_confg.h"
#include "l1_const.h"

#if TESTMODE
  #include "l1tm_defty.h"
#endif // TESTMODE

#if (AUDIO_TASK == 1)
  #include "l1audio_const.h"
  #include "l1audio_cust.h"
  #include "l1audio_defty.h"
#endif // AUDIO_TASK

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
#if (L1_DYN_DSP_DWNLD == 1)
  #include "l1_dyn_dwl_defty.h"
#endif

#if (TRACE_TYPE == 4)
  #include "l1_defty.h"
#endif

#include "../../bsp/armio.h"
#include "../../bsp/timer.h"

#include "../../bsp/iq.h"
#include "../../bsp/mem.h"
#include "../../bsp/clkm.h"
#include "../../bsp/inth.h"

/*
 * The function that sets up the HISRs has an assert() macro call in it.
 * The Leonardo version was built with the TMS470 compiler's C library
 * version of assert() in it, which is not very useful.  Here I am
 * pulling in some GPF/VSI headers so we get the more useful GPF/VSI
 * version of assert() instead.
 */
#include "../../gpf/inc/typedefs.h"
#include "../../gpf/inc/vsi.h"
#include "../../gpf/inc/cust_os.h"

/*
 * Timing monitor
 */
#if (TRACE_TYPE == 4)
  extern T_L1A_L1S_COM l1a_l1s_com;
  extern T_L1S_GLOBAL  l1s;
  UNSIGNED             max_cpu, fn_max_cpu;
  unsigned short       layer_1_sync_end_time;
  unsigned short       max_cpu_flag;
  #if (DSP >= 38)
  // DSP CPU load measurement trace variables
  UWORD32              dsp_max_cpu_load_trace_array[4];
  UWORD32              dsp_max_cpu_load_idle_frame;
  unsigned short       l1_dsp_cpu_load_trace_flag;
  #endif
#endif

#define	STACK_SECTION	__attribute__ ((section ("int.ram")))

#if (L1_EXT_AUDIO_MGT == 1)
  NU_HISR  EXT_AUDIO_MGT_hisr;
  char FAR ext_audio_mgt_hisr_stack[500] STACK_SECTION;
  extern void Cust_ext_audio_mgt_hisr(void);
#endif

#if ( (L1_MP3 == 1) || (L1_MIDI == 1) || (L1_AAC == 1) || (L1_DYN_DSP_DWNLD == 1) )   // equivalent to an API_HISR flag
  extern void api_hisr(void);
  char FAR API_HISR_stack[0x400] STACK_SECTION;
  NU_HISR apiHISR;
#endif // (L1_MP3 == 1) || (L1_MIDI == 1) || (L1_DYN_DSP_DWNLD == 1)

#if (FF_L1_IT_DSP_USF == 1) || (FF_L1_IT_DSP_DTX == 1)
  char FAR API_MODEM_HISR_stack[0x400] STACK_SECTION; // stack size to be tuned
  NU_HISR api_modemHISR;
#endif // FF_L1_IT_DSP_USF

/*
 * HISR stack and semaphore needed by L1
 */
#if (OP_L1_STANDALONE == 0)
  #define LAYER_1_SYNC_STACK_SIZE	4000	/* matching Leonardo version */
  unsigned char layer_1_sync_stack[LAYER_1_SYNC_STACK_SIZE] STACK_SECTION;
#else
  #if TESTMODE
    char FAR layer_1_sync_stack[2600 /*3600*/];   // Frame interrupt task stack for EVA3
  #else
    char FAR layer_1_sync_stack[1600 /* 2600 */];   // Frame interrupt task stack for EVA3
  #endif
#endif   /* OP_L1_STANDALONE */

NU_HISR  layer_1_sync_HISR;    // Frame interrupt task stack for EVA3

/* forward declaration */
void layer_1_sync_HISR_entry (void);

/*
 * l1_create_HISR
 *
 * Create L1 HISR.  This function is called from l1_pei.
 *
 * Apparently this function was originally called l1_create_ISR(),
 * as that is how it appears in the Leonardo dl1_com.obj module.
 * The LoCosto version originally had an l1_create_ISR() wrapper
 * that simply calls l1_create_HISR(), but I plan on changing
 * l1_pei to call l1_create_HISR() instead. -- Space Falcon
 */
void l1_create_HISR (void)
{
  STATUS status;

  #if (OP_L1_STANDALONE == 0)
    // Fill the entire stack with the pattern 0xFE
    memset (layer_1_sync_stack, 0xFE, LAYER_1_SYNC_STACK_SIZE);
  #endif

  status = NU_Create_HISR (&layer_1_sync_HISR,
                           "L1_HISR",
                           layer_1_sync_HISR_entry,
  #if (OP_L1_STANDALONE == 0)
                           1,
                           layer_1_sync_stack,
                           LAYER_1_SYNC_STACK_SIZE);
  #else
                           1,
                           layer_1_sync_stack,
                           sizeof(layer_1_sync_stack));
  #endif

  #if (L1_EXT_AUDIO_MGT)
    // Create HISR for Ext MIDI activity
    //==================================
    status += NU_Create_HISR(&EXT_AUDIO_MGT_hisr,
                             "H_EXT_AUDIO_MGT",
                             Cust_ext_audio_mgt_hisr,
                             2,
                             ext_audio_mgt_hisr_stack,
                             sizeof(ext_audio_mgt_hisr_stack));
  #endif

  #if ( (L1_MP3 == 1) || (L1_MIDI == 1) || (L1_AAC == 1) || (L1_DYN_DSP_DWNLD == 1) )    // equivalent to an API_HISR flag
    status += NU_Create_HISR(&apiHISR,
                             "API_HISR",
                             api_hisr,
                             2,
                             API_HISR_stack,
                             sizeof(API_HISR_stack));
  #endif // (L1_MP3 == 1) || (L1_MIDI == 1) || (L1_AAC == 1) || (L1_DYN_DSP_DWNLD == 1)

  #if (FF_L1_IT_DSP_USF == 1) || (FF_L1_IT_DSP_DTX == 1) // equivalent to an API_MODEM_HISR flag
    // Create HISR for USF  DSP interrupt !!!!. This HISR needs
    // to have the highest priority since the USF status needs
    // to be known before the next block starts.
    //========================================================
    status += NU_Create_HISR(&api_modemHISR,
                             "MODEM",
                             api_modem_hisr,
                             1,
                             API_MODEM_HISR_stack,
                             sizeof(API_MODEM_HISR_stack));
  #endif

  assert (status == 0);
}

/*
 * The versions of TP_FrameIntHandler() and layer_1_sync_HISR_entry()
 * in the Leonardo dl1_com.obj module contain CPU load measurement
 * code, but in the LoCosto version of L1 which we are using this
 * functionality has been moved into L1S proper, i.e., inside the
 * hisr() function.
 */

/*-------------------------------------------------------*/
/* TP_FrameIntHandler() Low Interrupt service routine    */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :  activate Hisr on each frame interrupt*/
/*-------------------------------------------------------*/
void TP_FrameIntHandler(void)
{

  #if (OP_L1_STANDALONE == 1)

    #if (TRACE_TYPE==1)
       if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1S_CPU_LOAD)
       {
         TM_ResetTimer (2, 0xFFFF, 1, 0);
         TM_StartTimer (2);
       }
    #endif

    #if (TRACE_TYPE==6)
       TM_ResetTimer (2, 0xFFFF, 1, 0);
       TM_StartTimer (2);
    #endif

    #if (TRACE_TYPE==7)   /* CPU_LOAD */
       l1_cpu_load_start();
    #endif

  #else

     #if (TRACE_TYPE == 4) && (TI_NUC_MONITOR != 1) && (WCP_PROF == 1)
              TM_ResetTimer (2, TIMER_RESET_VALUE, 1, 0);
              TM_StartTimer (2);
    #endif

    #if (TI_NUC_MONITOR == 1)
       /* Copy LISR buffer in Log buffer each end of HISR */
       ti_nuc_monitor_tdma_action();
    #endif

    #if WCP_PROF == 1
       prf_LogFNSwitch(l1s.actual_time.fn_mod42432);
    #endif

  #endif   /* OP_L1_STANDALONE */

  NU_Activate_HISR(&layer_1_sync_HISR);   /* Activate HISR interrupt */

  #if (OP_L1_STANDALONE == 0)
    #if (WCP_PROF == 1)
      #if (PRF_CALIBRATION == 1)
      NU_Activate_HISR(&prf_CalibrationHISR);
      #endif
    #endif
  #endif

}

/*
 * layer_1_sync_HISR_entry
 *
 * HISR associated to layer 1 sync.
 */

void layer_1_sync_HISR_entry (void)
{
   // Call Synchronous Layer1
   hisr();
}
