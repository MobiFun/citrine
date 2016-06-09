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
|  Purpose :  This Module defines the engineering mode (EM) device driver for the
|             G23 protocol stack. This driver is used to control all engineering
|             mode related functions.
+-----------------------------------------------------------------------------
*/

#ifndef SS_EM_C
#define SS_EM_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_SS

/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "typedefs.h"
#include "pcm.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_ss.h"
#include "mon_ss.h"
#include "pei.h"
#include "tok.h"
#include "ss.h"
#include "ss_em.h"

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/
#ifdef FF_EM_MODE
GLOBAL UBYTE em_ss_event_buffer[EM_SS_BUFFER_SIZE ];
GLOBAL UBYTE em_ss_buffer_write;

/* Event tracing flags for EM */
GLOBAL BOOL ss_v[EM_MAX_SS_EVENTS];

static UBYTE em_ss_trace_occured;

/*==== FUNCTIONS ==================================================*/

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_3
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, data value
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/

GLOBAL UBYTE em_write_buffer_3  (UBYTE event_no, UBYTE value)
{
  TRACE_FUNCTION ("ss_em_write_buffer_3()");

  if (check_write_index(3))
  {
    em_ss_event_buffer[em_ss_buffer_write++] = event_no; /* Tag: Event number */
    em_ss_event_buffer[em_ss_buffer_write++] = 1;        /* Length: 0 equals no value */
    em_ss_event_buffer[em_ss_buffer_write++] = value;    /* Value: Data to be stored */
    return FALSE; /* Data is stored inside buffer, reset flag */
  }
  else
    return TRUE;  /* No more space inside buffer, serve flag next time */
}


/*
+------------------------------------------------------------------------------
|  Function     : check_write_index
+------------------------------------------------------------------------------
|  Description  :  Checks the write index inside the buffer. No reset when
|                  buffer is full.
|
|  Parameters   :  Number of bytes to be stored in buffer
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/

GLOBAL UBYTE check_write_index (UBYTE n)
{
  TRACE_FUNCTION ("ss_check_write_index()");

  if (em_ss_buffer_write + n < EM_SS_BUFFER_SIZE )
  {
    /*
     * ACI is informed about the first event trace,
     * used for later data processing.
     */
    if (em_ss_trace_occured == 0)
    {
      PALLOC(em_notification, EM_DATA_IND);
      em_notification->entity = EM_SS;
      PSENDX(MMI, em_notification);
      em_ss_trace_occured++;
    }
    return TRUE;
  }
  else
    return FALSE;
}


/*
+------------------------------------------------------------------------------
|  Function     : em_init_ss_event_trace
+------------------------------------------------------------------------------
|  Description  :  Initialize the event tracing flags for SMS
|
|  Parameters   :
|
|  Return       :
|
+------------------------------------------------------------------------------
*/
GLOBAL void em_init_ss_event_trace(void)
{
  UBYTE i;

  TRACE_FUNCTION ("em_init_ss_event_trace()");

  for(i = 1; i < EM_MAX_SS_EVENTS; i++)
    ss_v[i] = 0;

  em_ss_buffer_write = 0;
}


/*
+------------------------------------------------------------------------------
|  Function     : ss_em_ss_event_req
+------------------------------------------------------------------------------
|  Description  :  Set the event tracing flags according the bitmask
|
|  Parameters   :  Primitive    - Bitmask
|
|  Return       :
|
+------------------------------------------------------------------------------
*/

GLOBAL void ss_em_ss_event_req (T_EM_SS_EVENT_REQ *em_ss_event_req)
{
  UBYTE i;

  TRACE_FUNCTION ("em_ss_em_ss_event_req()");

  /*
   *  The event tracing flags are set according the bitmask. ss_v[i] are
   *  the flags belonging to the event number described in 8443.601
   */
  for(i=1; i<(EM_MAX_SS_EVENTS); i++)
    ss_v[i] = ((em_ss_event_req->bitmask_ss & (0x01<<(i-1))) > 0) ? TRUE : FALSE;

  /*
   * A new event trace is generated therefor the flag is reset.
   */
  em_ss_trace_occured = 0;

  PFREE(em_ss_event_req);
}

#endif /* FF_EM_MODE */

#endif /* #ifndef SS_EM_C */
