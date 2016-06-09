/*
+-----------------------------------------------------------------------------
|  Project :
|  Modul   :  J:\g23m-gsm\alr\alr_tim.c
+-----------------------------------------------------------------------------
|  Copyright 2002 Texas Instruments Berlin, AG
|                 All rights reserved.
|
|                 This file is confidential and a trade secret of Texas
|                 Instruments Berlin, AG
|                 The receipt of or possession of this file does not convey
|                 any rights to reproduce or disclose its contents or to
|                 manufacture, use, or sell anything it may describe, in
|                 whole, or in part, without the specific written consent of
|                 Texas Instruments Berlin, AG.
+-----------------------------------------------------------------------------
|  Purpose :
+-----------------------------------------------------------------------------
*/
#ifndef ALR_TIM_C
#define ALR_TIM_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_PL

/*==== INCLUDES ===================================================*/
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "typedefs.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_alr.h"
#include "mon_alr.h"
#include "pei.h"
#include "tok.h"
#include "pcm.h"

#ifdef GPRS
#include "alr_gprs.h"
#endif

#include "alr.h"

/*==== PRIVAT =====================================================*/
#if defined(TIMER_TRACE)
  #define TRACE_TIMER(FMT)              TRACE_EVENT((FMT))
  #define TRACE_TIMER_P1(FMT,P1)        TRACE_EVENT_P1((FMT),(P1))
  #define TRACE_TIMER_P2(FMT,P1,P2)     TRACE_EVENT_P2((FMT),(P1),(P2))
  #define TRACE_TIMER_P3(FMT,P1,P2,P3)  TRACE_EVENT_P3((FMT),(P1),(P2),(P3))
#else
  #define TRACE_TIMER(FMT)
  #define TRACE_TIMER_P1(FMT,P1)
  #define TRACE_TIMER_P2(FMT,P1,P2)
  #define TRACE_TIMER_P3(FMT,P1,P2,P2)
#endif

/* Implements Measure#32: Row 247, 248, 249, 251, 252, 259 and 260 */
static T_S2I_STRING const tim_names[] =
{
  S2I_STRING("TIM_POWERMEAS"),
  S2I_STRING("TIM_NW_SYNC_GUARD"),
#if defined(FF_HOMEZONE)
  S2I_STRING("TIM_HOMEZONE"),
#endif  /* FF_HOMEZONE */
  S2I_STRING("???")
};

/*==== EXPORT =====================================================*/


/*==== FUCTIONS ===================================================*/
#if defined(TIMER_TRACE)
void  trace_timer (USHORT index, long value)
{
  GET_INSTANCE_DATA;
  switch(value)
  {
/* Implements Measure#32: Row 247, 248, 249, 251 and 252 */
    default:/* >0: start */
      TRACE_TIMER_P2 ("T-Start:%s=%lu", S2I_STRING(tim_names[index]), value);
      break;
    case 0:/* ==0: stop */
      TRACE_TIMER_P1 ("T-Stop:%s", S2I_STRING(tim_names[index]));
      break;
    case -1:/* -1: expire */
      TRACE_TIMER_P1 ("T-Expired:%s ", S2I_STRING(tim_names[index]));
      break;
    case -2:/* -2: check */
      if(alr_data->tim_data.running[index])
      {
        TRACE_TIMER_P1 ("T-Check:%s active", S2I_STRING(tim_names[index]));
      }
      else
      {
        TRACE_TIMER_P1 ("T-Check:%s inactive", S2I_STRING(tim_names[index]));
      }
      break;
    case -3:/* -2: set expire function */
      TRACE_TIMER_P1 ("T-Set:%s expire function", S2I_STRING(tim_names[index]));
      break;
  }
}
#else
#define trace_timer(index,value)
#endif  /* TIMER_TRACE */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : RR_TIM                   |
| STATE   : code                  ROUTINE : tim_exec_timeout         |
+--------------------------------------------------------------------+

  PURPOSE : reset timeout functions and values.

*/

GLOBAL void tim_init (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tim_init()");

  memset (&alr_data->tim_data, 0, sizeof (alr_data->tim_data));
  alr_data->tim_data.expire[TIM_POWERMEAS] = ma_cs_rxlev_req;
  alr_data->tim_data.expire[TIM_NW_SYNC_GUARD] = tim_stop_sync_to_nw;
#if defined(FF_HOMEZONE)
  alr_data->tim_data.expire[TIM_HOMEZONE] = cb_tim_homezone;
#endif /* FF_HOMEZONE */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : RR_TIM                   |
| STATE   : code                  ROUTINE : tim_exec_timeout         |
+--------------------------------------------------------------------+

  PURPOSE : execute timeout for the new frame variant.

*/

GLOBAL void tim_exec_timeout (USHORT index)
{
  GET_INSTANCE_DATA;
#if !defined(TIMER_TRACE)
  TRACE_FUNCTION ("tim_exec_timeout()");
#endif  /* !TIMER_TRACE */

  if (index < NUM_OF_ALR_TIMERS)
  {
    trace_timer (index, -1);
    alr_data->tim_data.running[index] = 0;

    if (alr_data->tim_data.expire[index])
      alr_data->tim_data.expire[index] ();
  }
  else
  {
    SYST_TRACE_P ((SYST, "tim_exec_timeout: index=%u (>=%u)", index, NUM_OF_ALR_TIMERS));
    TRACE_ASSERT (index < NUM_OF_ALR_TIMERS);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : RR_TIM                   |
| STATE   : code                  ROUTINE : tim_stop_timer           |
+--------------------------------------------------------------------+

  PURPOSE : stop timer in the new frame variant.

*/

GLOBAL void tim_stop_timer (USHORT index)
{
  GET_INSTANCE_DATA;
#if !defined(TIMER_TRACE)
  TRACE_FUNCTION ("tim_stop_timer()");
#endif  /* !TIMER_TRACE */

  if (index < NUM_OF_ALR_TIMERS)
  {
    trace_timer (index, 0);
    alr_data->tim_data.running[index] = 0;

    /*
     * stop physical timer.
     */
    TIMER_STOP(pl_handle, index);
  }
  else
  {
    SYST_TRACE_P ((SYST,"tim_stop_timer: index=%u (>=%u)", index, NUM_OF_ALR_TIMERS));
    TRACE_ASSERT (index < NUM_OF_ALR_TIMERS);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : RR_TIM                   |
| STATE   : code                  ROUTINE : tim_start_timer          |
+--------------------------------------------------------------------+

  PURPOSE : start timer in the new frame variant.

*/

GLOBAL void tim_start_timer (USHORT index, T_TIME value)
{
  GET_INSTANCE_DATA;
#if !defined(TIMER_TRACE)
  TRACE_FUNCTION ("tim_start_timer()");
#endif  /* !TIMER_TRACE */

  if (index < NUM_OF_ALR_TIMERS)
  {
    trace_timer (index, value);
    alr_data->tim_data.running[index] = 1;

    /*
     * start physical timer.
     */
    TIMER_START(pl_handle, index, value);
  }
  else
  {
    SYST_TRACE_P ((SYST,"tim_start_timer: index=%u (>=%u)", index, NUM_OF_ALR_TIMERS));
    TRACE_ASSERT (index < NUM_OF_ALR_TIMERS);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : RR_TIM                   |
| STATE   : code                  ROUTINE : tim_check_timer          |
+--------------------------------------------------------------------+

  PURPOSE : check timer in the new frame variant.

*/
GLOBAL BOOL tim_check_timer(USHORT index)
{
  GET_INSTANCE_DATA;
  T_TIME value = 0;

#if !defined(TIMER_TRACE)
  TRACE_FUNCTION ("tim_check_timer()");
#endif  /* !TIMER_TRACE */

  if (index < NUM_OF_ALR_TIMERS)
  {
#if defined(TIMER_TRACE)
    if (alr_data->tim_data.running[index])
    {
      SHORT vsi_ret;
      vsi_ret = TIMER_STATUS(pl_handle, index, &value);
      if ((vsi_ret EQ VSI_OK) AND (value NEQ 0))
      {
/* Implements Measure#32: Row 259 */
        TRACE_TIMER_P2 ("T-Check:%s active (%u left)", S2I_STRING(tim_names[index]), value);
      }
      else
      {
/* Implements Measure#32: Row 260 */
        TRACE_TIMER_P1 ("T-Check:%s inactive", S2I_STRING(tim_names[index]));
      }
    }
    else
    {
/* Implements Measure#32: Row 260 */
      TRACE_TIMER_P1 ("T-Check:%s inactive", S2I_STRING(tim_names[index]));
    }
#endif  /* TIMER_TRACE */

    return alr_data->tim_data.running[index];
  }
  else
  {
    SYST_TRACE_P ((SYST,"tim_check_timer: index=%u (>=%u)", index, NUM_OF_ALR_TIMERS));
    TRACE_ASSERT (index < NUM_OF_ALR_TIMERS);
    return FALSE;
  }
}

#endif  /* ALR_TIM_C */
