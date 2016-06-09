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

#ifndef SMS_EM_C
#define SMS_EM_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_SMS

/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "typedefs.h"
#include "pcm.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "message.h"
#include "ccdapi.h"
#include "prim.h"
#include "cus_sms.h"
#include "cnf_sms.h"
#include "mon_sms.h"
#include "pei.h"
#include "tok.h"
#include "sms.h"
#include "sms_em.h"

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

#ifdef FF_EM_MODE
GLOBAL UBYTE em_sms_event_buffer[EM_SMS_BUFFER_SIZE];
GLOBAL UBYTE em_sms_buffer_write;

/* Event tracing flags for EM */
GLOBAL BOOL sms_v[EM_MAX_SMS_EVENTS];

static UBYTE em_sms_trace_occured;

GLOBAL UBYTE em_change_flag = 0;

#endif /* FF_EM_MODE */

/*==== FUNCTIONS ==================================================*/

#ifdef FF_EM_MODE
LOCAL UBYTE check_write_index  (UBYTE n);

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_2
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number and length of data
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/

GLOBAL UBYTE em_write_buffer_2 (UBYTE event_no)
{
 TRACE_FUNCTION ("sms_em_write_buffer_2()");

 if(check_write_index(2)) {                              /* Check write index */
  em_sms_event_buffer[em_sms_buffer_write++] = event_no;        /* Event number */
  em_sms_event_buffer[em_sms_buffer_write++] = 0;               /* Indicates the length of the data value */
  return FALSE;                                          /* Data is stored inside buffer, reset flag */
 }
 else
  return TRUE;                                           /* No more space inside buffer, serve flag next time */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_3
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, length of data and value
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/

GLOBAL UBYTE em_write_buffer_3  (UBYTE event_no, UBYTE value)
{
 TRACE_FUNCTION ("sms_em_write_buffer_3()");

 if(check_write_index(3)) {                             /* Check write index */
  em_sms_event_buffer[em_sms_buffer_write++] = event_no;        /* Event number */
  em_sms_event_buffer[em_sms_buffer_write++] = 1;               /* Indicates the length of the data value */
  em_sms_event_buffer[em_sms_buffer_write++] = value;           /* Data to be stored */
  return FALSE;                                          /* Data is stored inside buffer, reset flag */
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
|  Parameters   :  Event number, length of data and value (USHORT)
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/

GLOBAL UBYTE em_write_buffer_3a (UBYTE event_no, USHORT value)
{
 TRACE_FUNCTION ("sms_em_write_buffer_3a()");

 if(check_write_index(4)) {                                   /* Check write index */
  em_sms_event_buffer[em_sms_buffer_write++] = event_no;             /* Event number */
  em_sms_event_buffer[em_sms_buffer_write++] = 2;                    /* Indicates the length of the data value */
  em_sms_event_buffer[em_sms_buffer_write++] = (UBYTE)(value >> 8);  /* Data to be stored - MSB first */
  em_sms_event_buffer[em_sms_buffer_write++] = (UBYTE)(value);       /* LSB second */
  return FALSE;                                               /* Data is stored inside buffer, reset flag */
 }
 else
  return TRUE;                                                /* No more space inside buffer, serve flag next time */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_4
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, length of data, data value1 and value2
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/

GLOBAL UBYTE em_write_buffer_4  (UBYTE event_no, UBYTE value1, UBYTE value2)
{
 TRACE_FUNCTION ("sms_em_write_buffer_4()");

  if(check_write_index(4)) {                                  /* Check write index */
  em_sms_event_buffer[em_sms_buffer_write++] = event_no;             /* Event number */
  em_sms_event_buffer[em_sms_buffer_write++] = 2;                    /* Indicates the length of the data value */
  em_sms_event_buffer[em_sms_buffer_write++] = value1;               /* Value 1 */
  em_sms_event_buffer[em_sms_buffer_write++] = value2;               /* Value 2 */
  return FALSE;                                               /* Data is stored inside buffer, reset flag */
 }
 else
  return TRUE;                                                /* No more space inside buffer, serve flag next time */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_4a
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, length of data, data value and cs (USHORT)
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/

GLOBAL UBYTE em_write_buffer_4a (UBYTE event_no, UBYTE value, USHORT cs)
{
 TRACE_FUNCTION ("sms_em_write_buffer_4a()");

 if(check_write_index(5)) {                                   /* Check write index */
  em_sms_event_buffer[em_sms_buffer_write++] = event_no;             /* Event number */
  em_sms_event_buffer[em_sms_buffer_write++] = 3;                    /* Indicates the length of the data value */
  em_sms_event_buffer[em_sms_buffer_write++] = value;                /* Data */
  em_sms_event_buffer[em_sms_buffer_write++] = (UBYTE)(cs >> 8);     /* Data to be stored - MSB first */
  em_sms_event_buffer[em_sms_buffer_write++] = (UBYTE)(cs);          /* LSB second */
  return FALSE;                                               /* Data is stored inside buffer, reset flag */
 }
 else
  return TRUE;                                                /* No more space inside buffer, serve flag next time */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_5
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, length of data, data value1 and value2
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/

GLOBAL UBYTE em_write_buffer_5 (UBYTE event_no, UBYTE value1, UBYTE value2, UBYTE value3)
{
 TRACE_FUNCTION ("sms_em_write_buffer_5()");

  if(check_write_index(5)) {                                  /* Check write index */
  em_sms_event_buffer[em_sms_buffer_write++] = event_no;             /* Event number */
  em_sms_event_buffer[em_sms_buffer_write++] = 3;                    /* Indicates the length of the data value */
  em_sms_event_buffer[em_sms_buffer_write++] = value1;               /* Value 1 */
  em_sms_event_buffer[em_sms_buffer_write++] = value2;               /* Value 2 */
  em_sms_event_buffer[em_sms_buffer_write++] = value3;               /* Value 3 */
  return FALSE;                                               /* Data is stored inside buffer, reset flag */
 }
 else
  return TRUE;                                                /* No more space inside buffer, serve flag next time */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_6
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, length of data, data value and cs (USHORT)
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/

GLOBAL UBYTE em_write_buffer_6 (UBYTE event_no, UBYTE value, UBYTE value1, UBYTE value2, UBYTE value3)
{
 TRACE_FUNCTION ("sms_em_write_buffer_6()");

 if(check_write_index(6)) {                                   /* Check write index */
  em_sms_event_buffer[em_sms_buffer_write++] = event_no;             /* Event number */
  em_sms_event_buffer[em_sms_buffer_write++] = 4;                    /* Indicates the length of the data value */
  em_sms_event_buffer[em_sms_buffer_write++] = value;                /* Data */
  em_sms_event_buffer[em_sms_buffer_write++] = value1;               /* Data 1 */
  em_sms_event_buffer[em_sms_buffer_write++] = value2;               /* Data 2 */
  em_sms_event_buffer[em_sms_buffer_write++] = value3;               /* Data 3 */
  return FALSE;                                               /* Data is stored inside buffer, reset flag */
 }
 else
  return TRUE;                                                /* No more space inside buffer, serve flag next time */
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
 TRACE_FUNCTION ("sms_check_write_index()");

 if (em_sms_buffer_write + n < EM_SMS_BUFFER_SIZE)
 {
  /*
    ACI is informed about the first event trace, used for later data processing.
  */
  if(em_sms_trace_occured == 0)
  {
   PALLOC(em_notification, EM_DATA_IND);
   em_notification->entity = EM_SMS;
   PSENDX(MMI, em_notification);
   em_sms_trace_occured++;
  }
  return TRUE;
 }
 else
  return FALSE;
}

/*
+------------------------------------------------------------------------------
|  Function     : em_init_sms_event_trace
+------------------------------------------------------------------------------
|  Description  :  Initialize the event tracing flags for SMS
|
|  Parameters   :
|
|  Return       :
|
+------------------------------------------------------------------------------
*/
GLOBAL void em_init_sms_event_trace(void)
{
 UBYTE i;

 TRACE_FUNCTION ("em_init_sms_event_trace()");

 for(i=1; i<EM_MAX_SMS_EVENTS; i++)
   sms_v[i] = 0;

 em_sms_buffer_write = 0;
}

/*
+------------------------------------------------------------------------------
|  Function     : sms_em_sms_event_req
+------------------------------------------------------------------------------
|  Description  :  Set the event tracing flags according the bitmask
|
|  Parameters   :  Primitive    - Bitmask
|
|  Return       :
|
+------------------------------------------------------------------------------
*/

GLOBAL void sms_em_sms_event_req (T_EM_SMS_EVENT_REQ *em_sms_event_req)
{
 UBYTE i;

 TRACE_FUNCTION ("sms_em_sms_event_req()");
 /*
  *  The event tracing flags are set according the bitmask. sms_v[i] are
  *  the flags belonging to the event number described in 8443.601
 */
 for(i=1; i<33; i++)  {
   sms_v[i] = ((em_sms_event_req->bitmask_sms_l & (0x01<<(i-1))) > 0) ? TRUE : FALSE;
 }

 for(i=33; i<(EM_MAX_SMS_EVENTS); i++)  {
   sms_v[i] = ((em_sms_event_req->bitmask_sms_h & (0x01<<(i-1))) > 0) ? TRUE : FALSE;
 }

 /*
   A new event trace is generated therefor the flag is set to 0.
 */
 em_sms_trace_occured = 0;

 PFREE(em_sms_event_req);
}
#endif /* FF_EM_MODE */

#endif /* SMS_EM_C */
