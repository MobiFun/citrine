/* 
+------------------------------------------------------------------------------
|  File:       trace.c
+------------------------------------------------------------------------------
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
|  Purpose :  This Modul provides data and functions for fast in memory tracing
+----------------------------------------------------------------------------- 
*/ 

#ifndef TRACE_C
#define TRACE_C
#endif
#define FAST_TRACE

/*==== INCLUDES ===================================================*/

#include <string.h>
#include "typedefs.h"
#include "header.h"
#include "custom.h"
#include "vsi.h"
#include "gsm.h"
#include "prim.h"
#include "pei.h"
#include "tok.h"

#include "trace.h"

#include <string.h>
#include <stdio.h>
#include "message.h"

/*==== CONST ======================================================*/

#define TRC_ENTITY 0x60
#define TRC_FILE   1


#ifdef OPTION_MULTITHREAD
#define VSI_CALLER "ACI",
#define VSI_CALLER_SINGLE "ACI"
#else
#define VSI_CALLER
#define VSI_CALLER_SINGLE
#endif


/*==== TYPES ======================================================*/

/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)        MODULE  : TRACE                    |
| STATE   : code                  ROUTINE : trc_init                 |
+--------------------------------------------------------------------+

  PURPOSE : This function is called to initialize the trace function

*/

GLOBAL void trc_init
            (
              void
            )
{
  trc_p_buffer     = trc_buffer;
  trc_p_buffer_end = &trc_buffer[TRC_BUF_LEN-1];
  trc_wrap_around  = FALSE;
  trc_wrap_around_enable  = TRUE;
  trc_enabled      = TRUE;
  /*semTRC = vsi_s_open (VSI_CALLER "TRC"); */
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)        MODULE  : TRACE                    |
| STATE   : code                  ROUTINE : trc_enable_wrap_around   |
+--------------------------------------------------------------------+

  PURPOSE : This function is called to enable wrap around

*/

GLOBAL void trc_enable_wrap_around
            (
              void
            )
{
  trc_wrap_around_enable = TRUE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)        MODULE  : TRACE                    |
| STATE   : code                  ROUTINE : trc_disable_wrap_around  |
+--------------------------------------------------------------------+

  PURPOSE : This function is called to disable wrap around

*/

GLOBAL void trc_disable_wrap_around
            (
              void
            )
{
  trc_wrap_around_enable  = FALSE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)        MODULE  : TRACE                    |
| STATE   : code                  ROUTINE : trc_enable               |
+--------------------------------------------------------------------+

  PURPOSE : This function is called to enable fast trace

*/

GLOBAL void trc_enable
            (
              void
            )
{
  trc_enabled = TRUE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)        MODULE  : TRACE                    |
| STATE   : code                  ROUTINE : trc_disable              |
+--------------------------------------------------------------------+

  PURPOSE : This function is called to disable fast trace

*/

GLOBAL void trc_disable
            (
              void
            )
{
  trc_enabled  = FALSE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)        MODULE  : RA_SHM                   |
| STATE   : code                  ROUTINE : trc_dump                 |
+--------------------------------------------------------------------+

  PURPOSE : the content of the stored dump is dumped 
            into the system trace window.
  
*/

GLOBAL void trc_dump (void)
{
  T_TRC_BUF_ENTRY *p_readOut;
  ULONG *p;
  ULONG i;
  char buf [40];
  char buf1 [40];

#ifndef NEW_FRAME
  vsi_o_trace ("", 0xff, "---FASTTRACEBUFFER---");
#else
  vsi_o_ttrace (0, 0xff, "---FASTTRACEBUFFER---");
#endif
  if (trc_wrap_around)
  {
    for (p_readOut= trc_p_buffer;
         p_readOut < trc_p_buffer_end;
         p_readOut++)
    {
      p = (ULONG*)p_readOut;

      buf1[0]='\0';

      for (i = sizeof(T_TRC_BUF_ENTRY); i>=0; i-=sizeof(ULONG))
      {
        sprintf(buf1, "0x%x ", *p++);
        strcat (buf, buf1);
      }
#ifndef NEW_FRAME
      vsi_o_trace ("", 0xff, buf);
      vsi_t_sleep("", 1);
#else
      vsi_o_ttrace (0, 0xff, buf);
      vsi_t_sleep(0, 1);
#endif
    }
  }
  for (p_readOut= trc_buffer;
       p_readOut < trc_p_buffer;
       p_readOut++)
  {
    p = (ULONG*)p_readOut;

    buf1[0]='\0';

    for (i = sizeof(T_TRC_BUF_ENTRY); i>=0; i-=sizeof(ULONG))
    {
      sprintf(buf1, "0x%x ", *p++);
      strcat (buf, buf1);
    }
#ifndef NEW_FRAME
    vsi_o_trace ("", 0xff, buf);
    vsi_t_sleep("", 1);
#else
    vsi_o_ttrace (0, 0xff, buf);
    vsi_t_sleep(0, 1);
#endif
  }

#ifndef NEW_FRAME
  vsi_o_trace ("", 0xff, "---END---");
#else
  vsi_o_ttrace (0, 0xff, "---END---");
#endif
}
