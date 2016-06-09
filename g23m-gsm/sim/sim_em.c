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

#ifndef SIM_EM_C
#define SIM_EM_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_SIM

/*==== INCLUDES ===================================================*/
#include <string.h>
#include "typedefs.h"
#include "pconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "cnf_sim.h"
#include "mon_sim.h"
#include "prim.h"
#include "pei.h"
#include "tok.h"
/* #include "dti.h"  removed, but included in SIM.H */
#include "sim.h"
#include "sim_em.h"

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

#ifdef FF_EM_MODE
GLOBAL UBYTE em_sim_event_buffer[EM_SIM_BUFFER_SIZE];
GLOBAL UBYTE em_sim_buffer_write;

/* Event tracing flags for EM */
GLOBAL BOOL sim_v[EM_MAX_SIM_EVENTS];

static UBYTE em_sim_trace_occured;
#endif /* FF_EM_MODE */

/*==== FUNCTIONS ==================================================*/

#ifdef FF_EM_MODE

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_2
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/

GLOBAL UBYTE em_write_buffer_2 (UBYTE event_no)
{

 TRACE_FUNCTION ("sim_em_write_buffer_2()");

 if(check_write_index(2)) {                              /* Check write index */
  em_sim_event_buffer[em_sim_buffer_write++] = event_no;          /* Event number */
  em_sim_event_buffer[em_sim_buffer_write++] = 0;                /* Value length - 0 equals no value */
  return FALSE;                                           /* Data is stored inside buffer, reset flag */
 }
 else
  return TRUE;                                            /* No more space inside buffer, serve flag next time */
}

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

 TRACE_FUNCTION ("sim_em_write_buffer_3()");

 if(check_write_index(3)) {                            /* Check write index */
  em_sim_event_buffer[em_sim_buffer_write++] = event_no;        /* Event number */
  em_sim_event_buffer[em_sim_buffer_write++] = 1;               /* Value length - 0 equals no value */
  em_sim_event_buffer[em_sim_buffer_write++] = value;           /* Data to be stored */
  return FALSE;                                         /* Data is stored inside buffer, reset flag */
 }
 else
  return TRUE;                                           /* No more space inside buffer, serve flag next time */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_3a
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, data value (USHORT)
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/

GLOBAL UBYTE em_write_buffer_3a (UBYTE event_no, USHORT value)
{

 TRACE_FUNCTION ("sim_em_write_buffer_3a()");

 if(check_write_index(4)) {                                 /* Check write index */
  em_sim_event_buffer[em_sim_buffer_write++] = event_no;             /* Event number */
  em_sim_event_buffer[em_sim_buffer_write++] = 2;                   /* Value length - 0 equals no value */
  em_sim_event_buffer[em_sim_buffer_write++] = (UBYTE)(value >> 8);  /* Data to be stored - MSB first */
  em_sim_event_buffer[em_sim_buffer_write++] = (UBYTE)(value);       /* LSB second */
  return FALSE;                                              /* Data is stored inside buffer, reset flag */
 }
 else
  return TRUE;                                               /* No more space inside buffer, serve flag next time */
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

LOCAL UBYTE check_write_index (UBYTE n)
{

 TRACE_FUNCTION ("sim_check_write_index()");

 if ((em_sim_buffer_write + n) < EM_SIM_BUFFER_SIZE)
 {
  /*
    ACI is informed about the first event trace, used for later data processing.
  */
  if(em_sim_trace_occured == 0)
  {
   PALLOC(em_notification, EM_DATA_IND);
   em_notification->entity = EM_SIM;
   PSENDX(MMI, em_notification);
  em_sim_trace_occured++;
  }
  return TRUE;
 }
 else
  return FALSE;
}

/*
+------------------------------------------------------------------------------
|  Function     : em_init_sim_event_trace
+------------------------------------------------------------------------------
|  Description  :  Initialize the event tracing flags for SMS
|
|  Parameters   :
|
|  Return       :
|
+------------------------------------------------------------------------------
*/
GLOBAL void em_init_sim_event_trace(void)
{
 UBYTE i;

 TRACE_FUNCTION ("em_init_sim_event_trace()");

 for(i=1; i<EM_MAX_SIM_EVENTS; i++)
   sim_v[i] = 0;

 em_sim_buffer_write = 0;
}

/*
+------------------------------------------------------------------------------
|  Function     : sim_em_sim_event_req
+------------------------------------------------------------------------------
|  Description  :  Set the event tracing flags according the bitmask
|
|  Parameters   :  Primitive    - Bitmask
|
|  Return       :
|
+------------------------------------------------------------------------------
*/

GLOBAL void sim_em_sim_event_req (T_EM_SIM_EVENT_REQ *em_sim_event_req)
{
 UBYTE i;

 TRACE_FUNCTION ("sim_em_sim_event_req()");
 /*
  *  The event tracing flags are set according the bitmask. sim_v[i] are
  *  the flags belonging to the event number described in 8443.601
 */
 for(i=1; i<EM_MAX_SIM_EVENTS; i++)
   sim_v[i] = ((em_sim_event_req->bitmask_sim & (0x01<<(i-1))) > 0) ? TRUE : FALSE;

 /*
   A new event trace is generated therefor the flag is set to 0.
 */
 em_sim_trace_occured = 0;

 PFREE(em_sim_event_req);
}
#endif /* FF_EM_MODE */


#endif /* SIM_EM_C */
