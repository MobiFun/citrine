/*
 * This header file appears in the LoCosto version, but not in Leonardo.
 * The version of this file in the LoCosto source was full of LoCosto
 * specifics, so originally I planned on omitting it in FreeCalypso.
 * However, some of the C modules make use of some of the definitions
 * here no matter what CHIPSET, so I'm including a stripped-down version
 * that has all LoCosto-isms removed, but retains the needed definitions.
 */

#ifndef __L1_PWMGR_H__
#define __L1_PWMGR_H__

  #define FAIL_SLEEP_PERIPH_CHECK   1 /* When initial Check_Peripheral_App */
  #define FAIL_SLEEP_OSTIMERGAUGE   2 /* When checking for Osload, HWtimer or min_time_gauging */
  #define FAIL_SLEEP_PERIPH_SLEEP   3 /* When the peripherals are put to sleep */
  #define FAIL_SLEEP_L1SYNCH        4 /* Failed at l1s_synch level itself */

  #define FAIL_SLEEP_DUE_TO_OSLOAD  0
  #define FAIL_SLEEP_DUE_TO_HWTIMER 1
  #define FAIL_SLEEP_DUE_TO_MINTIMEGAUGING 2

  #define L1_PWMGR_APP_OFFSET    0x80

  typedef struct
  {
    UWORD8  fail_id;
    UWORD8  fail_ret_val;
  }T_PWMGR_DEBUG;

#endif
