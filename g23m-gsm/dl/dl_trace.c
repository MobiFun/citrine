/*
+-----------------------------------------------------------------------------
|  Project :
|  Modul   :
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
|  Purpose :  This Modul defines the offline trace functions
|             for the component DL of the mobile station.
+-----------------------------------------------------------------------------
*/

#ifndef DL_TRACE_C
#define DL_TRACE_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_DL

/*==== INCLUDES ===================================================*/
#include "typedefs.h"
#include <string.h>
#include "vsi.h"
#include "pconst.cdg"
#include "custom.h"
#include "gsm.h"
#include "mon_dl.h"
#include "prim.h"
#include "pei.h"
#include "tok.h"
#include "ccdapi.h"
#include "dl.h"

/*==== EXPORT =====================================================*/
/*==== PRIVAT =====================================================*/
#if defined(DL_TRACE_ENABLED)
#ifdef OPTION_MULTITHREAD
  #define TRACE_TYPE    _ENTITY_PREFIXED(TRACE_TYPE)
  #define CHANNEL       _ENTITY_PREFIXED(CHANNEL)
  #define STATES        _ENTITY_PREFIXED(STATES)
#endif  /* OPTION_MULTITHREAD */
LOCAL const char* const STATES[] =      {
                                "INVALID",
                                "DISABLED",
                                "IDLE_DL",
                                "SUSPENDED",
                                "AWAITING_ESTABLISHMENT",
                                "MULTI_FRAME_ESTABLISHED",
                                "TIMER_RECOVERY",
                                "AWAITING_RELEASE"
                             };
LOCAL const char* const TRACE_TYPE[] =  { "UL", "DL", "Ev", "St", "PL", "RR" };
LOCAL const char* const CH_TYPE[] =     { "  ", "SA", "SD", "FH", "FF", "CC", "BC", "PC", "PE", "CB", "BE" };
LOCAL const char  SAPI_TYPE[] =         { '0', '1', '2', '3', ' '};

LOCAL void  array2hex (UBYTE *inarray, char *outarray, int size);

#if !defined(DL_IMMEDIATE_TRACE)

/*==== TEST TRACE ===================================================*/
#define TEST_ENTITY_DL

/*==== VARIABLES ==================================================*/
/*==== FUNCTIONS ==================================================*/
/*
 * The Data Link Layer Trace is a cyclic buffer for
 * debugging layer 2 problems.
 *
 * The buffer will be initialized at startup and will
 * be filled by the function dl_trace() until it is full.
 * The size of the buffer is IDLE_TRACE_SIZE.
 *
 * The content is
 *
 * trace_type (uplink, downlink, event state, alr_event, rr_event)
 * Channel Type (SACCH, SDDCH, FACCH)
 * real system clock
 * State (DL states)
 * pending disc request
 * data (layer frame, eevnt strings, state)
 *
 * During IDLE mode (triggered by RX_PERIODIC_IND in ALR/TIL_main.c)
 * an output is written to as SYST trace.
 * (IDLE_TRACE_MAX_READED traces each trigger)
 */

#define IDLE_TRACE_SIZE       512
#define IDLE_TRACE_MAX_READED  16

#if (((IDLE_TRACE_SIZE-1) & (~IDLE_TRACE_SIZE)) == (IDLE_TRACE_SIZE-1))
#define POWER_OF_2
#pragma message("IDLE_TRACE_SIZE is power of 2")
#else
#pragma message("IDLE_TRACE_SIZE is NOT power of 2")
#endif

typedef struct
{
  UBYTE         trace_type;
  UBYTE         ch_type;
  UBYTE         sapi;
  T_TIME        sysClock;
  UBYTE         disc_request;
  UBYTE         state;
  UBYTE         data [MAX_L2_FRAME_SIZE];
} T_IDLE_TRACE_DATA;

#ifdef OPTION_MULTITHREAD
  #define IDLE_Trace_buffer       _ENTITY_PREFIXED(IDLE_Trace_buffer)
  #define IDLE_Trace_write_index  _ENTITY_PREFIXED(IDLE_Trace_write_index)
  #define IDLE_Trace_read_index   _ENTITY_PREFIXED(IDLE_Trace_read_index)
#endif  /* OPTION_MULTITHREAD */

GLOBAL T_IDLE_TRACE_DATA   IDLE_Trace_buffer [IDLE_TRACE_SIZE];
GLOBAL USHORT              IDLE_Trace_write_index  = 0;
GLOBAL USHORT              IDLE_Trace_read_index   = 0;

LOCAL  T_HANDLE            sem_DL_TRC;

GLOBAL void dl_trace_init (void)
{
  sem_DL_TRC  = vsi_s_open (VSI_CALLER "DL_IDLE_TRACE",1);
  if (sem_DL_TRC NEQ VSI_ERROR)
    dl_trace_clear (0);
  else
    SYST_TRACE ("DL:can´t open semaphore \"DL_IDLE_TRACE\"");
}

GLOBAL void dl_trace_exit (void)
{
  if (sem_DL_TRC NEQ VSI_ERROR)
    vsi_s_close (VSI_CALLER sem_DL_TRC);
}

GLOBAL void dl_trace_clear ()
{
  dl_trace_read_all (0); /* first, get all remaining traces (if exists) */

  ENTER_CRITICAL_SECTION (sem_DL_TRC);
  IDLE_Trace_write_index = IDLE_Trace_read_index = 0;
  LEAVE_CRITICAL_SECTION (sem_DL_TRC);

  TRACE_EVENT ("offline trace reset");
  DL_OFFLINE_TRACE (TRACE_DL_EVENT, TRACE_CH_UNKNOWN, 0, "offline trace reset");
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : DL_COM                |
| STATE   : code                     ROUTINE : dl_trace              |
+--------------------------------------------------------------------+

  PURPOSE : Fill in a trace.

*/
GLOBAL void dl_trace (UCHAR trace_type, UCHAR channel, UCHAR ch_type, UCHAR* data)
{
  T_IDLE_TRACE_DATA*  trace_data;
  USHORT              write_index1, write_index2;/* trace_size must not be greater than 2 */
  UBYTE               trace_size;

  if (data)
  {
#if defined (DISABLE_MEASREPORT_TRACE)
    if ((ch_type EQ L2_CHANNEL_SACCH) AND
        (data[2] EQ 0x01) AND
        (data[3] EQ 0x03) AND
        (data[4] EQ 0x49))
      return;
#endif  /* DISABLE_MEASREPORT_TRACE */

#if defined (DISABLE_EMPTY_UI)
    if ((trace_type EQ TRACE_DOWNLINK) OR (trace_type EQ TRACE_UPLINK))
    {
      if ((ch_type EQ L2_CHANNEL_SACCH) AND
        (data[3] EQ 0x03) AND
        (data[4] EQ 0x01))
        return;
      else if (
        (data[1] EQ 0x03) AND
        (data[2] EQ 0x01))
        return;
    }
#endif  /* DISABLE_EMPTY_UI */
  }

  TEST_SEMAPHORE (sem_DL_TRC);
  ENTER_CRITICAL_SECTION(sem_DL_TRC);

  trace_size = 1;
  if ((trace_type EQ TRACE_PL_EVENT) OR (trace_type EQ TRACE_RR_EVENT))
  {
    if (data AND strlen ((char *)data) >= 23)
      trace_size = 2;
  }

#if defined(POWER_OF_2)
  write_index1 = (IDLE_Trace_write_index + 1) & (IDLE_TRACE_SIZE - 1);  /* if IDLE_TRACE_SIZE power of 2 */
  write_index2 = (IDLE_Trace_write_index + trace_size) & (IDLE_TRACE_SIZE - 1);  /* if IDLE_TRACE_SIZE power of 2 */
#else
  write_index1 = (IDLE_Trace_write_index + 1) % IDLE_TRACE_SIZE;      /* if IDLE_TRACE_SIZE not power of 2 */
  write_index2 = (IDLE_Trace_write_index + trace_size) % IDLE_TRACE_SIZE;      /* if IDLE_TRACE_SIZE not power of 2 */
#endif  /* POWER_OF_2 */
  if ((write_index1 NEQ IDLE_Trace_read_index) AND (write_index2 NEQ IDLE_Trace_read_index))
  { /* buffer is not full */
    trace_data = &IDLE_Trace_buffer[IDLE_Trace_write_index];

    trace_data->trace_type = trace_type;
    if ((trace_type EQ TRACE_PL_EVENT) OR (trace_type EQ TRACE_RR_EVENT))
    {
      trace_data->state = trace_size;
    }
    else
    {
      GET_INSTANCE_DATA; 
      trace_data->ch_type = ch_type;
      switch (channel)
      {
      case C_SACCH0:
      case C_DCCH0:
        trace_data->disc_request = dl_data->dcch0_disc_request;
        trace_data->state = dl_data->state [C_DCCH0];
        trace_data->sapi = PS_SAPI_0;
        break;
      case C_DCCH3:
        trace_data->disc_request = dl_data->dcch3_disc_request;
        trace_data->state = dl_data->state [C_DCCH3];
        trace_data->sapi = PS_SAPI_3;
        break;
      default:
        trace_data->disc_request = 0;
        trace_data->state = 0;
        trace_data->sapi = NOT_PRESENT_8BIT;
        break;
      }
    }

    vsi_t_time (VSI_CALLER &trace_data->sysClock);

    if (data)
    {
      memcpy (trace_data->data, data, MAX_L2_FRAME_SIZE);
      if ((trace_type NEQ TRACE_UPLINK) AND (trace_type NEQ TRACE_DOWNLINK))
      {
        trace_data->data[MAX_L2_FRAME_SIZE-1] = 0;
      }

      if (trace_size EQ 2)
      {
        if (IDLE_Trace_write_index EQ (IDLE_TRACE_SIZE - 1))/* the last buffer index ? */
          trace_data = &IDLE_Trace_buffer[0];/* -> overflow to the first buffer index */
        else
          trace_data++;
        memcpy (trace_data->data, data+MAX_L2_FRAME_SIZE-1, MAX_L2_FRAME_SIZE-1);
        trace_data->data[MAX_L2_FRAME_SIZE-1] = 0;
      }
    }

    IDLE_Trace_write_index = write_index2;
  }/* endif buffer is not full */

  LEAVE_CRITICAL_SECTION (sem_DL_TRC);

#if defined(_SIMULATION_)
  dl_trace_read (0);
#endif  /* _SIMULATION_ */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : DL_COM                |
| STATE   : code                     ROUTINE : dl_trace_read         |
+--------------------------------------------------------------------+

  PURPOSE : Fill in a trace.

*/
GLOBAL void dl_trace_read_all ()
{
  USHORT  write_index, read_index;

  do
  {
    dl_trace_read ();

    ENTER_CRITICAL_SECTION (sem_DL_TRC);
    write_index = IDLE_Trace_write_index;
    read_index = IDLE_Trace_read_index;
    LEAVE_CRITICAL_SECTION (sem_DL_TRC);
  } while (read_index NEQ write_index);
}

GLOBAL void dl_trace_read ()
{
  T_IDLE_TRACE_DATA*  trace_data;
  USHORT              write_index, read_index, left;
  UBYTE               trace_size;
  static char         buffer[80];
  UBYTE               j, o, readed = 0;

  TEST_SEMAPHORE (sem_DL_TRC);

  ENTER_CRITICAL_SECTION (sem_DL_TRC);
  write_index = IDLE_Trace_write_index;
  read_index = IDLE_Trace_read_index;
  LEAVE_CRITICAL_SECTION (sem_DL_TRC);

  if (read_index EQ write_index)
  {
    #if defined (_TARGET_) AND !defined( GPRS ) AND defined(FF_GTI)
      sleep_mode ();
    #endif  /* _TARGET_ AND !GPRS AND !FF_GTI */
    return;
  }

  while (read_index NEQ write_index)
  {
    ENTER_CRITICAL_SECTION (sem_DL_TRC);
    trace_data = &IDLE_Trace_buffer[read_index];
    LEAVE_CRITICAL_SECTION (sem_DL_TRC);

#if defined(POWER_OF_2)
    left = (write_index - read_index - 1) & (IDLE_TRACE_SIZE-1);
#else
    left = (IDLE_TRACE_SIZE + write_index - read_index - 1) % IDLE_TRACE_SIZE;
#endif  /* POWER_OF_2 */

    if (trace_data->ch_type > ELEMENTS(CH_TYPE))
      trace_data->ch_type = 0;

    if (trace_data->sapi >= ELEMENTS (SAPI_TYPE))
      trace_data->sapi = ELEMENTS (SAPI_TYPE) - 1;

    trace_size = 1;/* default */
    switch (trace_data->trace_type)
    {
    case TRACE_UPLINK:
    case TRACE_DOWNLINK:
      sprintf (buffer, "[%03d]:%07lu %c%d %s %s ",
        left,
        trace_data->sysClock,
        trace_data->disc_request?'D':' ',
        trace_data->state,
        TRACE_TYPE[trace_data->trace_type],
        CH_TYPE[trace_data->ch_type]);
      o = strlen (buffer);
      array2hex (trace_data->data, buffer+o, 23);
      break;

    case TRACE_DL_EVENT:
      sprintf (buffer, "[%03d]:%07lu %c%d Ev %s%c %s",
        left,
        trace_data->sysClock,
        trace_data->disc_request?'D':' ',
        trace_data->state,
        CH_TYPE[trace_data->ch_type],
        SAPI_TYPE[trace_data->sapi],
        trace_data->data);
      break;

    case TRACE_PL_EVENT:
    case TRACE_RR_EVENT:
      trace_size = trace_data->state;
      if (trace_size EQ 2)
      {
        T_IDLE_TRACE_DATA *trace_data2;
        if (read_index EQ (IDLE_TRACE_SIZE - 1))/* the last buffer index ? */
          trace_data2 = &IDLE_Trace_buffer[0];/* -> overflow to the first buffer index */
        else
          trace_data2 = trace_data+1;

        sprintf (buffer, "[%03d]:%07lu %d  Ev %s %s%s",
          left,
          trace_data->sysClock,
          trace_data->state,
          TRACE_TYPE[trace_data->trace_type],
          trace_data->data,
          trace_data2->data);
      }
      else
      {
        sprintf (buffer, "[%03d]:%07lu %d  Ev %s %s",
          left,
          trace_data->sysClock,
          trace_data->state,
          TRACE_TYPE[trace_data->trace_type],
          trace_data->data);
      }
      break;

    case TRACE_CHSTATE:
      sprintf (buffer, "[%03d]:%07lu %c%d ST %s%c state=%s",
        left,
        trace_data->sysClock,
        trace_data->disc_request?'D':' ',
        trace_data->state,
        CH_TYPE[trace_data->ch_type],
        SAPI_TYPE[trace_data->sapi],
        STATES[trace_data->state]);
      break;

    default:
      buffer[0] = 0;
      break;
    }

    if (buffer[0])
    {
      SYST_TRACE (buffer);
    }
    else
    {
      SYST_TRACE ("dl_trace_read() failed");
    }

    ENTER_CRITICAL_SECTION (sem_DL_TRC);
    trace_data->sysClock = 0; /* readed */
    IDLE_Trace_read_index += trace_size;
#if defined(POWER_OF_2)
    IDLE_Trace_read_index &= (IDLE_TRACE_SIZE-1);/* if power of 2 */
#else
    IDLE_Trace_read_index %= IDLE_TRACE_SIZE; /* if not power of 2 */
#endif  /* POWER_OF_2 */
    read_index = IDLE_Trace_read_index;
    write_index = IDLE_Trace_write_index;
    LEAVE_CRITICAL_SECTION (sem_DL_TRC);

    if (readed++ >= IDLE_TRACE_MAX_READED)
      break;
  }/* endwhile */
}

#else  /* DL_IMMEDIATE_TRACE */

#define IMM_TRACE_SIZE       2
typedef struct
{
  UBYTE         sapi;
  T_TIME        sysClock;
  UBYTE         disc_request;
  UBYTE         state;
  UBYTE         data [IMM_TRACE_SIZE*MAX_L2_FRAME_SIZE];
} T_TRACE_DATA;

#ifdef OPTION_MULTITHREAD
  #define print_buffer  _ENTITY_PREFIXED(print_buffer)
  #define trace_buffer  _ENTITY_PREFIXED(trace_buffer)
#endif  /* OPTION_MULTITHREAD */
LOCAL char          print_buffer[25+IMM_TRACE_SIZE*MAX_L2_FRAME_SIZE];
LOCAL T_TRACE_DATA  trace_buffer;

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : DL_COM                |
| STATE   : code                     ROUTINE : dl_trace              |
+--------------------------------------------------------------------+

  PURPOSE : Fill in a trace.

*/

GLOBAL void dl_fast_trace (UBYTE trace_type, UBYTE channel, UBYTE ch_type,
                    T_TIME trace_time, ULONG trace_mask, UBYTE* data)
{
  T_TRACE_DATA*  trace_data;
  UBYTE          o;

  /* While TC_USER2 is set, measurements and empty frames will be traced always */
  if (data AND ((trace_mask & TC_USER2) EQ 0))
  {
    if ((ch_type EQ L2_CHANNEL_SACCH) AND
        (data[2] EQ 0x01) AND
        (data[3] EQ 0x03) AND
        (data[4] EQ 0x49))
      return;

    if ((trace_type EQ TRACE_DOWNLINK) OR (trace_type EQ TRACE_UPLINK))
    {
      if ((ch_type EQ L2_CHANNEL_SACCH) AND
        (data[3] EQ 0x03) AND
        (data[4] EQ 0x01))
        return;
      else if (
        (data[1] EQ 0x03) AND
        (data[2] EQ 0x01))
        return;
    }
  }

  trace_data = &trace_buffer;
  trace_data->sysClock = trace_time;
  if (!((trace_type EQ TRACE_PL_EVENT) OR (trace_type EQ TRACE_RR_EVENT)))
  {
    GET_INSTANCE_DATA; 
    switch (channel)
    {
    case C_SACCH0:
    case C_DCCH0:
      trace_data->disc_request = dl_data->dcch0_disc_request;
      trace_data->state = dl_data->state [C_DCCH0];
      trace_data->sapi = PS_SAPI_0;
      break;
    case C_DCCH3:
      trace_data->disc_request = dl_data->dcch3_disc_request;
      trace_data->state = dl_data->state [C_DCCH3];
      trace_data->sapi = PS_SAPI_3;
      break;
    default:
      trace_data->disc_request = 0;
      trace_data->state = 0;
      trace_data->sapi = NOT_PRESENT_8BIT;
      break;
    }
  }

  if (data)
  {
    if ((trace_type EQ TRACE_UPLINK) OR (trace_type EQ TRACE_DOWNLINK))
    {
      memcpy (trace_data->data, data, MAX_L2_FRAME_SIZE);
    }
    else
    {
      strncpy ((char *)trace_data->data, (char *)data, IMM_TRACE_SIZE*MAX_L2_FRAME_SIZE-1);
      trace_data->data[IMM_TRACE_SIZE*MAX_L2_FRAME_SIZE-1] = 0;
    }
  }
  else
  {
    trace_data->data[0] = 0;
  }


  if (ch_type > ELEMENTS(CH_TYPE))
    ch_type = 0;

  if (trace_data->sapi >= ELEMENTS (SAPI_TYPE))
    trace_data->sapi = ELEMENTS (SAPI_TYPE) - 1;

  switch (trace_type)
  {
  case TRACE_UPLINK:
  case TRACE_DOWNLINK:
    sprintf (print_buffer, "DLTRC:%07lu %c%d %s %s%c ",
      trace_data->sysClock,
      trace_data->disc_request?'D':' ',
      trace_data->state,
      TRACE_TYPE[trace_type],
      CH_TYPE[ch_type],
      SAPI_TYPE[trace_data->sapi]);
    o = strlen (print_buffer);
    array2hex (trace_data->data, print_buffer+o, 23);
    break;

  case TRACE_DL_EVENT:
    sprintf (print_buffer, "DLTRC:%07lu %c%d Ev %s%c %s",
      trace_data->sysClock,
      trace_data->disc_request?'D':' ',
      trace_data->state,
      CH_TYPE[ch_type],
      SAPI_TYPE[trace_data->sapi],
      trace_data->data);
    break;

  case TRACE_PL_EVENT:
  case TRACE_RR_EVENT:
    sprintf (print_buffer, "DLTRC:%07lu %d  Ev %s %s",
      trace_data->sysClock,
      trace_data->state,
      TRACE_TYPE[trace_type],
      trace_data->data);
    break;

  case TRACE_CHSTATE:
    sprintf (print_buffer, "DLTRC:%07lu %c%d ST %s%c state=%s",
      trace_data->sysClock,
      trace_data->disc_request?'D':' ',
      trace_data->state,
      CH_TYPE[ch_type],
      SAPI_TYPE[trace_data->sapi],
      STATES[trace_data->state]);
    break;

  default:
    print_buffer[0] = 0;
    break;
  }

  TRACE_USER_CLASS (TC_USER1, print_buffer);

#if 0
  if (print_buffer[0])
  {
    #if defined(_SIMULATION_)
      TRACE_EVENT_WIN (print_buffer);
    #else  /* _SIMULATION_ */
      SYST_TRACE_P ((DLTRC,print_buffer));
    #endif  /* _SIMULATION_ */
  }
#endif /* 0 */
}
#endif  /* DL_IMMEDIATE_TRACE */

LOCAL void  array2hex (UBYTE *inarray, char *outarray, int size)
{
  int col=0, n=0;
  UBYTE b, nh, nl;

  while (n < size)
  {
    b = inarray[n++];
    nh = b>>4;
    nl = b&0x0f;
    outarray[col++] = nh > 9 ? nh + 'A' - 10 : nh + '0';
    outarray[col++] = nl > 9 ? nl + 'A' - 10 : nl + '0';
  }
  outarray[col] = 0;
}
#endif  /* DL_TRACE_ENABLED */

#if defined (DL_TRACE_PFREE)
GLOBAL void* my_pfree(void *pointer, int line, char *file)
{

  char buffer[23];
  sprintf (buffer, "%s#%u:%p", file+2, line, pointer);
  dl_trace (TRACE_DL_EVENT, TRACE_CH_UNKNOWN, buffer);

  if (pointer)
  {
    PFREE (pointer);
  }
  else
  {
    SYST_TRACE_P((SYST, "%s#%u: PFREE(NULL)", file, line));
  }

  return NULL;
}
#endif /* DL_TRACE_PFREE */

#endif  /* DL_TRACE_C */
