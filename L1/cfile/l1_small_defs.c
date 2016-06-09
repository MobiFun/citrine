/*
 * TI's original code had an l1_small.c module, containing the GSM small
 * sleep function implemented in assembly (TI assembler syntax embedded
 * in the C module via an individual asm() for each line, very poor style)
 * and C definitions for two helper variables.  In FreeCalypso we are
 * moving the small sleep assembly code into a proper assembly module;
 * this C module contains just the helper variable definitions.
 */

#include "config.h"
#include "l1_confg.h"
#include "l1_macro.h"

#if (CODE_VERSION == SIMULATION)
  #include <string.h>
  #include "l1_types.h"
  #include "sys_types.h"
  #include "l1_const.h"
  #include "l1_time.h"
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
  #include "l1_varex.h"
  #include "cust_os.h"
  #include "l1_msgty.h"
  
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
  #include "l1_time.h"

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
  #include "l1_varex.h"
  #include "../../gpf/inc/cust_os.h"
  #include "l1_msgty.h"
  #include "tpudrv.h"

#endif

UWORD8 *mode_authorized = &(l1s.pw_mgr.mode_authorized);
UWORD8 *switch_PWR_MNGT = &(l1_config.pwr_mngt);
