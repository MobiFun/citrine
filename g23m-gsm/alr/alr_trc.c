/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  ALR_TRC
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
|  Purpose :  This Modul defines functions for the offline trace
+-----------------------------------------------------------------------------
*/

#ifndef ALR_TRC_C
#define ALR_TRC_C

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

#if defined (ALR_TRACE_ENABLED)
/*
 * The ALR Trace is a cyclic buffer for
 * debugging ALR problems.
 *
 * The buffer will be initialized at startup and will
 * be filled by the function alr_trc_dl_trace() until it is full.
 * The size of the buffer is ALR_TRC_SIZE.
 *
 * The content is
 *
 * actual_channel
 * State
 * sysclock
 * L2 data
 *
 * During IDLE mode (triggered by RX_PERIODIC_IND in ALR/TIL_main.c)
 * an output is written to the _PL.dbg (ALR_TRC_MAX_READED traces each trigger)
 */

/* prototypes */
void alr_trc_clear (void);
#define ALR_TRC_SIZE          90
#define	ALR_TRC_MAX_READED     8

#if (((ALR_TRC_SIZE-1) & (~ALR_TRC_SIZE)) == (ALR_TRC_SIZE-1))
#define POWER_OF_2
#pragma message("ALR_TRC_SIZE is power of 2")
#else
#pragma message("ALR_TRC_SIZE is NOT power of 2")
#endif

typedef struct
{
  UCHAR   event;
  UCHAR   state;
  UCHAR   actual_channel;
  USHORT  sysclock;
  UCHAR   data [ALR_TRC_DATA_SIZE];
} T_IDLE_TRACE_DATA;

T_IDLE_TRACE_DATA alr_csIDLE_Trace_buffer [ALR_TRC_SIZE];
USHORT            alr_csIDLE_Trace_index      = 0;
USHORT            alr_csIDLE_Trace_read_index = 0;

static T_HANDLE   sem_ALR_TRC;

#define	ENTER_CRITICAL_SECTION(sem) if (alr_trc_enter_critical_section(sem))return;
#define	LEAVE_CRITICAL_SECTION(sem) if (alr_trc_leave_critical_section(sem))return;


/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : ALT_TRC                        |
| STATE   : code                     ROUTINE : alr_trc_semaphore_err		  |
+-----------------------------------------------------------------------------+

  PURPOSE : Handles semaphore error situation

*/

static void alr_trc_semaphore_err (void)
{
  static UCHAR out = 0;
  if (!out)
  {
    out = 1;
    vsi_o_ttrace(VSI_CALLER TC_EVENT, "semaphore error");
  }
}//endfunc alr_trc_dl_trace_cs_err


/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : ALT_TRC                        |
| STATE   : code                     ROUTINE : alr_trc_enter_critical_section |
+-----------------------------------------------------------------------------+

  PURPOSE : Enters critical section

*/

static int alr_trc_enter_critical_section (T_HANDLE sem)
{
  if (vsi_s_get (VSI_CALLER sem) NEQ VSI_OK)
  {
    alr_trc_semaphore_err();
    return -1;
  }
  else
  {
    return 0;
  }
}//endfunc alr_trc_enter_critical_section


/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : ALT_TRC                        |
| STATE   : code                     ROUTINE : alr_trc_leave_critical_section |
+-----------------------------------------------------------------------------+

  PURPOSE : Leaves critical section

*/

static int alr_trc_leave_critical_section (T_HANDLE sem)
{
  if (vsi_s_release (VSI_CALLER sem) NEQ VSI_OK)
  {
    alr_trc_semaphore_err();
    return -1;
  }
  else
  {
    return 0;
  }
}//endfunc alr_trc_leave_critical_section


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : ALT_TRC               |
| STATE   : code                     ROUTINE : alr_trc_init			 |
+--------------------------------------------------------------------+

  PURPOSE : Init tracing

*/


void alr_trc_init (void)
{
  sem_ALR_TRC  = vsi_s_open (VSI_CALLER "ALR_IDLE_TRACE",1);
  if (sem_ALR_TRC NEQ VSI_ERROR)
    alr_trc_clear ();
  else
    vsi_o_ttrace(VSI_CALLER TC_EVENT, "can't open semaphore \"ALR_IDLE_TRACE\"");
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : ALT_TRC               |
| STATE   : code                     ROUTINE : alr_trc_exit			 |
+--------------------------------------------------------------------+

  PURPOSE : Close tracing

*/

void alr_trc_exit (void)
{
  if (sem_ALR_TRC NEQ VSI_ERROR)
    vsi_s_close (VSI_CALLER sem_ALR_TRC);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : ALT_TRC               |
| STATE   : code                     ROUTINE : alr_trc_clear		 |
+--------------------------------------------------------------------+

  PURPOSE : Clears trace index

*/


void alr_trc_clear (void)
{
  ENTER_CRITICAL_SECTION (sem_ALR_TRC);
  alr_csIDLE_Trace_index = alr_csIDLE_Trace_read_index = 0;
  LEAVE_CRITICAL_SECTION (sem_ALR_TRC);

  vsi_o_ttrace(VSI_CALLER TC_EVENT, "IDLE_Trace_index reseted");
}

#if 0
static void alr_trc_sleep_mode (void)
{
  static UCHAR  sleep_mode_disabled = FALSE;

  if (!sleep_mode_disabled)
  {
    #define NO_SLEEP            0
    #define ARMIO_CLK           0x0001
    #define UWIRE_CLK           0x0020
    #define SIM_CLK             0x0040
    #define UART_CLK            0x0400

    #if defined (_TARGET_)
      #if !defined( GPRS )
        power_down_config(NO_SLEEP, ARMIO_CLK | UWIRE_CLK | SIM_CLK | UART_CLK);
      #endif
    #endif  /* _TARGET_ */
    sleep_mode_disabled = TRUE;
  }
}//endfunc alr_trc_sleep_mode
#endif /* 0|1 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : ALT_TRC               |
| STATE   : code                     ROUTINE : alr_trc_store         |
+--------------------------------------------------------------------+

  PURPOSE : Fill in a trace.

*/

void alr_trc_store (UCHAR event, UCHAR actual_channel, UCHAR state, void* data)
{
  T_IDLE_TRACE_DATA*  trace_data;
  USHORT              write_index;
  USHORT              length;
  T_TIME              sysClock;


#if 0
  {
  USHORT  semCount;

  if (vsi_s_status (VSI_CALLER sem_ALR_TRC, &semCount) NEQ VSI_OK)
  {
    alr_trc_semaphore_err();
    return;
  }
  if (semCount EQ 0)
  {
    vsi_o_ttrace(VSI_CALLER TC_EVENT, "semCount == 0");
    return;
  }
  }
#endif  /* 0|1 */

  ENTER_CRITICAL_SECTION (sem_ALR_TRC);

#if defined(POWER_OF_2)
  write_index = (alr_csIDLE_Trace_index + 1) & (ALR_TRC_SIZE - 1);  /* if ALR_TRC_SIZE power of 2 */
#else
  write_index = (alr_csIDLE_Trace_index + 1) % ALR_TRC_SIZE;        /* if ALR_TRC_SIZE not power of 2 */
#endif  /* POWER_OF_2 */
  if (write_index NEQ alr_csIDLE_Trace_read_index)
  {/* buffer is not full */
    trace_data = &alr_csIDLE_Trace_buffer[alr_csIDLE_Trace_index];

    trace_data->event = event;
    if (actual_channel >= ALR_TRC_CH_UNKNOWN)
      trace_data->actual_channel = ALR_TRC_CH_UNKNOWN;
    else
      trace_data->actual_channel = actual_channel;
    trace_data->state = state;
    vsi_t_time (VSI_CALLER &sysClock);
    trace_data->sysclock = (USHORT)sysClock;

    if (event EQ ALR_TRC_DOWNLINK OR event EQ ALR_TRC_UPLINK)
      length = 23;
    else
      length = ALR_TRC_DATA_SIZE;

    if (event EQ ALR_TRC_STRING)
      strncpy ((char*)trace_data->data, (char*)data, length);
    else
      memcpy (trace_data->data, (UCHAR*)data, length);

    alr_csIDLE_Trace_index = write_index;
  }/* endif buffer is not full */

  LEAVE_CRITICAL_SECTION (sem_ALR_TRC);
}/* endfunc alr_trc_store */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : ALT_TRC               |
| STATE   : code                     ROUTINE : alr_trc_read          |
+--------------------------------------------------------------------+

  PURPOSE : Read and output stored trace.

*/


void alr_trc_read_all (void)
{
  USHORT  write_index, read_index;

  ENTER_CRITICAL_SECTION (sem_ALR_TRC);
  write_index = alr_csIDLE_Trace_index;
  read_index = alr_csIDLE_Trace_read_index;
  LEAVE_CRITICAL_SECTION (sem_ALR_TRC);

  while (read_index NEQ write_index)
  {
    alr_trc_read (20);
    vsi_t_sleep(VSI_CALLER 0);

    ENTER_CRITICAL_SECTION (sem_ALR_TRC);
    write_index = alr_csIDLE_Trace_index;
    read_index = alr_csIDLE_Trace_read_index;
    LEAVE_CRITICAL_SECTION (sem_ALR_TRC);
  }
}/* endfunc alr_trc_dl_trace_read_all */

static const char * ALR_TRC_CHANNEL[7] =  { "  ", "Ff", "Fh", "S4", "S8", "SA", "??" };
static const char * ALR_TRC_EVENTS[4] =  { "UL", "DL", "Ev", "ev" };

void alr_trc_read  (int count)
{
  T_IDLE_TRACE_DATA trace_data;
  USHORT            write_index, read_index;
  static char       buffer[ALR_TRC_DATA_SIZE*2+50];
  UCHAR             j, o, readed = 0;

  ENTER_CRITICAL_SECTION (sem_ALR_TRC);
  write_index = alr_csIDLE_Trace_index;
  read_index = alr_csIDLE_Trace_read_index;
  LEAVE_CRITICAL_SECTION (sem_ALR_TRC);

#if 0
  if (read_index EQ write_index)
  {
    alr_trc_sleep_mode ();
    return;
  }
#endif  /* 0|1 */

  while (read_index NEQ write_index)
  {
    ENTER_CRITICAL_SECTION (sem_ALR_TRC);
    trace_data = alr_csIDLE_Trace_buffer[read_index];
    alr_csIDLE_Trace_buffer[read_index].sysclock = 0xffff; /* readed */
    alr_csIDLE_Trace_read_index++;
#if defined(POWER_OF_2)
    alr_csIDLE_Trace_read_index&= (ALR_TRC_SIZE-1); /* if power of 2 */
#else
    alr_csIDLE_Trace_read_index%= ALR_TRC_SIZE;     /* if not power of 2 */
#endif  /* POWER_OF_2 */
    read_index = alr_csIDLE_Trace_read_index;
    LEAVE_CRITICAL_SECTION (sem_ALR_TRC);

    sprintf (buffer, "[%03d]:%05d  %d %s %s ",
#if defined(POWER_OF_2)
      (write_index - read_index) & (ALR_TRC_SIZE-1), /* if ALR_TRC_SIZE power of 2 */
#else
      (ALR_TRC_SIZE + write_index - read_index) % ALR_TRC_SIZE,    /* if ALR_TRC_SIZE not power of 2 */
#endif  /* POWER_OF_2 */
    trace_data.sysclock,
    trace_data.state,
    ALR_TRC_EVENTS[trace_data.event],
    ALR_TRC_CHANNEL[trace_data.actual_channel]);
    o = strlen (buffer);

    switch (trace_data.event)
    {
    case ALR_TRC_STRING:
      strncpy (buffer+o, (char*)&trace_data.data[0], ALR_TRC_DATA_SIZE);
      buffer[o+ALR_TRC_DATA_SIZE] = 0;
      break;
    case ALR_TRC_DOWNLINK:
    case ALR_TRC_UPLINK:
      for (j=0;j<23;j++, o+=2)
      {
        sprintf (buffer+o, "%02x", trace_data.data[j]);
      }
      break;
    default:
      for (j=0;j<ALR_TRC_DATA_SIZE;j++, o+=2)
      {
        sprintf (buffer+o, "%02x", trace_data.data[j]);
      }
      break;
    }

    if (buffer[0])
    {
      vsi_o_ttrace(VSI_CALLER TC_EVENT, buffer);
    }
    else
    {
      vsi_o_ttrace(VSI_CALLER TC_EVENT, "alr_trc_read() failed");
    }

    ENTER_CRITICAL_SECTION (sem_ALR_TRC);
    write_index = (UCHAR)alr_csIDLE_Trace_index;
    LEAVE_CRITICAL_SECTION (sem_ALR_TRC);

    if (++readed >= count)
      break;
  }/* endwhile */

}//endfunc alr_trc_read */

#else   /* ALR_TRACE_ENABLED */
#endif  /* ALR_TRACE_ENABLED */
#endif  /* ALR_TRC_C */
