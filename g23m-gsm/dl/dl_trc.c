/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  DL_TRC
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
|  Purpose :  This Modul defines functions for tracing (only simulation)
+-----------------------------------------------------------------------------
*/

#ifndef DL_TRC_C
#define DL_TRC_C

#define ENTITY_DL


/*==== INCLUDES ===================================================*/
#if defined (NEW_FRAME)

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "typedefs.h"
#include "pconst.cdg"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "mon_dl.h"
#include "prim.h"
#include "pei.h"
#include "tok.h"
#include "ccdapi.h"
#include "dl.h"

#else

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "stddefs.h"
#include "pconst.cdg"
#include "custom.h"
#include "gsm.h"
#include "mon_dl.h"
#include "prim.h"
#include "vsi.h"
#include "pei.h"
#include "tok.h"
#include "ccdapi.h"
#include "dl.h"

#endif

#include "DL_trc.h"

#if defined(DL_TRACE_WIN32)

#define  DL_TRC_DELAY()    /* vsi_t_sleep (VSI_CALLER 1) */

#define  MY_ENTER_CRITICAL_SECTION(s)  dl_trc_enter_critical_section(s)
#define  MY_LEAVE_CRITICAL_SECTION(s)  dl_trc_leave_critical_section(s)

LOCAL char  local_buffer[300];
LOCAL int   idx;
#if defined (NEW_FRAME)
LOCAL T_HANDLE    sem_buf_acces;
#else
LOCAL T_VSI_SHANDLE  sem_buf_acces;
#endif

LOCAL const char * const FrameName[] =
  {
    "I",      /* 0  I */
    "RR",     /* 1  S */
    "RNR",    /* 2  S */
    "REJ",    /* 3  S */
    "SABM",   /* 4  U */
    "DM",     /* 5  U */
    "UI",     /* 6  U */
    "DISC",   /* 7  U */
    "UA",     /* 8  U */
    "unknown"
  };
LOCAL const char * const ChannelType[] =
{
  "?<", "SA", "SD", "FH", "FF", "?>"
};
enum  frame_type_e
  {
    FT_I,
    FT_RR,
    FT_RNR,
    FT_REJ,
    FT_SABM,
    FT_DM,
    FT_UI,
    FT_DISC,
    FT_UA,
    FT_UNKNOWN
  };


#if defined (NEW_FRAME)
GLOBAL int dl_trc_enter_critical_section (T_HANDLE sem)
#else
GLOBAL int dl_trc_enter_critical_section (T_VSI_SHANDLE sem)
#endif  /* NEW_FRAME */
{
  idx++;
  if (idx > 1)
  {
    sprintf (local_buffer, "idx=%d!", idx);
    TRACE_EVENT (local_buffer);
    vsi_t_sleep (VSI_CALLER 50);
  }

  if (vsi_s_get (VSI_CALLER sem) NEQ VSI_OK)
  {
    TRACE_EVENT ("dl_trc_enter_critical_section() semaphore error");
    return -1;
  }
  else
  {
    /*
    vsi_t_sleep (VSI_CALLER 1);
    TRACE_EVENT ("dl_trc_enter_critical_section()");
    */
    return 0;
  }
}/* endfunc dl_trc_enter_critical_section */

#if defined (NEW_FRAME)
GLOBAL int dl_trc_leave_critical_section (T_HANDLE sem)
#else
GLOBAL int dl_trc_leave_critical_section (T_VSI_SHANDLE sem)
#endif  /* NEW_FRAME */
{
  idx--;
  if (vsi_s_release (VSI_CALLER sem) NEQ VSI_OK)
  {
    TRACE_EVENT ("dl_trc_leave_critical_section() semaphore error");
    return -1;
  }
  else
  {
    /*
    vsi_t_sleep (VSI_CALLER 1);
    TRACE_EVENT ("dl_trc_leave_critical_section()");
    */
    return 0;
  }
}/* endfunc dl_trc_leave_critical_section */

GLOBAL void dl_trc_init ()
{
  TRACE_FUNCTION ("dl_trc_init()");
  idx = 0;
#ifdef NEW_FRAME
  sem_buf_acces  = vsi_s_open (VSI_CALLER "DL_TRC",1);
#else
  sem_buf_acces  = vsi_s_open (VSI_CALLER "DL_TRC");
#endif  /* NEW_FRAME */

  if (sem_buf_acces EQ VSI_ERROR)
  {
    TRACE_FUNCTION ("open semaphore error \"sem_buf_acces\"");
  }
#if 1
  else if (!MY_ENTER_CRITICAL_SECTION (sem_buf_acces))
  {
    TRACE_FUNCTION ("open semaphore \"sem_buf_acces\"");
    MY_LEAVE_CRITICAL_SECTION (sem_buf_acces);
  }
#endif  /* 0|1 */
}/* endfunc dl_trc_init */

GLOBAL void dl_trc_exit ()
{
  TRACE_FUNCTION ("dl_trc_exit()");
  vsi_s_close (VSI_CALLER sem_buf_acces);
}/* endfunc dl_trc_exit */


GLOBAL void dl_trc_printf (char *fmt, ...)
{
  if (!MY_ENTER_CRITICAL_SECTION (sem_buf_acces))
  {
    va_list  vl;
    char *buf = local_buffer;
    va_start( vl, fmt );  /* Initialize variable arguments. */
    vsprintf (buf, fmt, vl);
    va_end ( vl );        /* Reset variable arguments.      */

    DL_TRC_DELAY ();
    TRACE_FUNCTION (buf);
    DL_TRC_DELAY ();
    MY_LEAVE_CRITICAL_SECTION (sem_buf_acces);
  }
}

GLOBAL void dl_trc_print_array(unsigned char *pArray, int nLength)
{
  if (!MY_ENTER_CRITICAL_SECTION (sem_buf_acces))
  {
    int n, o;
    char *buf = local_buffer;

    for (n = 0, o = 0; (n < nLength) AND (o < ((int)sizeof (local_buffer)) - 7); n++)
    {
      o += sprintf (buf + o, "%02X ", *(pArray + n));
    }

    if (n < nLength)
    {
      o += sprintf (buf + o, "...");
    }

    DL_TRC_DELAY ();
    TRACE_FUNCTION (buf);
    DL_TRC_DELAY ();
    MY_LEAVE_CRITICAL_SECTION (sem_buf_acces);
  }
}

GLOBAL void dl_trc_frame(unsigned char ch_type, unsigned char *pFrame, int direction)
{
  /* direction 0: down, 1: up */

  if (!MY_ENTER_CRITICAL_SECTION (sem_buf_acces))
  {
    int o = 0, n, l, m;
    int  frametype;
    int cmd = 0;
    int pf = ((*(pFrame + 1)) & 0x10) >> 4;
    char *buf = local_buffer;

    if (ch_type >= 5)
      ch_type = 5;

    if (((*pFrame) & (BTER_FORMAT_MASK)) EQ (SHORT_L2_HEADER_TYPE_1))
    {
      int   len = ch_type EQ L2_CHANNEL_SACCH ? DL_N201_SACCH_Bter : DL_N201_DCCH_Bter;

      /* format type Bter */
      o += sprintf (buf+o, "--- %2s short PD: SAPI 0     ",
        ChannelType[ch_type]);
      o += sprintf (buf+o, "%2s %s %2s",
        direction ? "  " : "<-", FrameName[FT_UI], direction ? "->" : "  ");
      while (o < 60)
        buf[o++] = 0x20;
        o += sprintf (buf+o, "  l=%-2u ", len);
      for (n = 0; n < len; n++)
        o += sprintf (buf+o, " %02X", *(pFrame + n));
    }
    else
    {
      /* address field */
      cmd = ((*pFrame)&0x02) ? direction ? 0 : 1
        : direction ? 1 : 0;
      o += sprintf (buf+o, "--- %2s %02X %02X %02X: SAPI %u %s ",
        ChannelType[ch_type],
        *(pFrame + 0), *(pFrame + 1), *(pFrame + 2),
        /*direction?"UL->":"<-DL",*/
        ((*pFrame)&0x1f)>>2,
        cmd ? "cmd" : "rsp");
      /* control field */
      if ((*(pFrame + 1)) & 0x01)
      {
        if (((*(pFrame + 1)) & 0x03) EQ 0x03)
        {
          switch ((*(pFrame + 1)) & 0xef)  /* unnumbered */
          {
          case 0x2f: frametype = FT_SABM; break;
          case 0x0f: frametype = FT_DM; break;
          case 0x03: frametype = FT_UI; break;
          case 0x43: frametype = FT_DISC; break;
          case 0x63: frametype = FT_UA; break;
          default:   frametype = FT_UNKNOWN; break;
          }
          o += sprintf (buf+o, "%c %14s",
            pf ? (cmd ? 'P' : 'F') : ' ', "");
        }
        else
        {
          switch ((*(pFrame + 1)) & 0x0f) /* supervisory */
          {
          case 0x01: frametype = FT_RR; break;
          case 0x05: frametype = FT_RNR; break;
          case 0x09: frametype = FT_REJ; break;
          default:   frametype = FT_UNKNOWN; break;
          }
          o += sprintf (buf+o, "%c %s=%u %7s",
            pf ? (cmd ? 'P' : 'F') : ' ',
            direction ? "N(R)" : "n(r)",
            ((*(pFrame + 1)) & 0xe0) >> 5,
            "");
        }
      }
      else
      {
        frametype = FT_I;
        o += sprintf (buf+o, "%c %s=%u %s=%u ",
          pf ? (cmd ? 'P' : 'F') : ' ',
          direction ? "N(R)" : "n(r)",
          ((*(pFrame + 1)) & 0xe0) >> 5,
          direction ? "N(S)" : "n(s)",
          ((*(pFrame + 1)) & 0x0e) >> 1);
      }

      o += sprintf (buf+o, "%2s %s %2s",
        direction ? "  " : "<-", FrameName[frametype], direction ? "->" : "  ");

      /* length field */
      l = (*(pFrame + 2)) >> 2;
      m = (*(pFrame + 2)) & 0x02;
      if (l OR m)
      {
        while (o < 60)
          buf[o++] = 0x20;
        o += sprintf (buf+o, "%c l=%-2u ", m ? 'M':' ', l);

        for (n = 0; (n < l) AND (n < 20); n++)
          o += sprintf (buf+o, " %02X", *(pFrame + 3 + n));
      }
    }

    DL_TRC_DELAY ();
    TRACE_FUNCTION (buf);
    DL_TRC_DELAY ();
    MY_LEAVE_CRITICAL_SECTION (sem_buf_acces);
  }
}/* endfunc dl_trc_frame */
#endif  /* DL_TRACE_WIN32 */

#endif  /* DL_TRC_C */

