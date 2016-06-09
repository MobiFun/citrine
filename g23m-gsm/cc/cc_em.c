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

#ifndef CC_EM_C
#define CC_EM_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_CC

/*==== INCLUDES ===================================================*/

#include <string.h>
#include "typedefs.h"
#include "pcm.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "message.h"
#include "ccdapi.h"
#include "prim.h"
#include "cnf_cc.h"
#include "mon_cc.h"
#include "pei.h"
#include "tok.h"
#include "cc.h"
#include "cc_em.h"

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

#ifdef FF_EM_MODE
GLOBAL UBYTE em_cc_event_buffer[EM_CC_BUFFER_SIZE];
GLOBAL UBYTE em_cc_buffer_write;

/* Event tracing flags for EM */
GLOBAL BOOL cc_v[EM_MAX_CC_EVENTS];

static UBYTE em_cc_trace_occured;
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
  TRACE_FUNCTION ("cc_em_write_buffer_2()");
  if (check_write_index(2))
  {
    em_cc_event_buffer[em_cc_buffer_write++] = event_no; /* Tag: Event number */
    em_cc_event_buffer[em_cc_buffer_write++] = 0;        /* Length: 0 means no value */
    return FALSE; /* Data is stored inside buffer, reset flag */
  }
  else
    return TRUE;  /* No more space inside buffer, serve flag next time */
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

GLOBAL UBYTE em_write_buffer_3 (UBYTE event_no, UBYTE value)
{
  TRACE_FUNCTION ("cc_em_write_buffer_3()");
 
  if (check_write_index(3))
  {
    em_cc_event_buffer[em_cc_buffer_write++] = event_no; /* Tag: Event number */
    em_cc_event_buffer[em_cc_buffer_write++] = 1;        /* Length: 0 means no value */
    em_cc_event_buffer[em_cc_buffer_write++] = value;    /* Data to be stored */
    return FALSE; /* Data is stored inside buffer, reset flag */
  }
  else
    return TRUE;  /* No more space inside buffer, serve flag next time */
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
  TRACE_FUNCTION ("cc_em_write_buffer_3a()");

  if (check_write_index(4))
  {
    em_cc_event_buffer[em_cc_buffer_write++] = event_no; /* Tag: Event number */
    em_cc_event_buffer[em_cc_buffer_write++] = 2;        /* Length: 0 means no value */

    /* Value: Data to be stored. MSB first, LSB second */
    em_cc_event_buffer[em_cc_buffer_write++] = (UBYTE)(value >> 8);
    em_cc_event_buffer[em_cc_buffer_write++] = (UBYTE)(value);
    return FALSE; /* Data is stored inside buffer, reset flag */
  }
  else
    return TRUE;  /* No more space inside buffer, serve flag next time */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_4
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, data value1, data value2 
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/

GLOBAL UBYTE em_write_buffer_4 (UBYTE event_no, UBYTE value1, UBYTE value2)
{
  TRACE_FUNCTION ("cc_em_write_buffer_4()");

  if (check_write_index(4))
  {
    em_cc_event_buffer[em_cc_buffer_write++] = event_no; /* Tag: Event number */
    em_cc_event_buffer[em_cc_buffer_write++] = 2;        /* Length: 0 means no value */
    em_cc_event_buffer[em_cc_buffer_write++] = value1;   /* Data to be stored, first */
    em_cc_event_buffer[em_cc_buffer_write++] = value2;   /* Data to be stored, second */
    return FALSE; /* Data is stored inside buffer, reset flag */
  }
  else
    return TRUE;  /* No more space inside buffer, serve flag next time */
}


/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_4a
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, data value1 (UBYTE *), data value2 
|
|  Return       :  TRUE/FALSE
|
|  Purpose      :  Stores Bearer Capabilitys
+------------------------------------------------------------------------------
*/

GLOBAL UBYTE em_write_buffer_4a (UBYTE event_no, UBYTE *ptr1, UBYTE value2)
{

  UBYTE bc_len;
  UBYTE i, em_report_len = 3;               /* Initial 2 + TI */

  TRACE_FUNCTION ("cc_em_write_buffer_4a()");

  ptr1++;                     /* Skip message type --> ptr1 shows now to BC-Tag*/
  if (*ptr1++ EQ 0x04)        /* BC tag (0x04) --> ptr1 shows now to length field*/
  {
    TRACE_FUNCTION ("EM_CC: BC tag found (0x04)");
    if(*ptr1 > MNCC_MAX_BC_LEN)   /* Field length exceeded */
    {
      ;                      /*better to insert some kind of trace here*/
      bc_len = MNCC_MAX_BC_LEN;   /*Just trace the maximum of BC's - skip the rest*/
    } 
    else                     /* Field length not exceeded */
    {
      bc_len = *ptr1;        /*trace all BC's possible */
    }
    
    ptr1++;                  /* ptr1 shows now the first BC */
    
    em_report_len += bc_len;  /* Length of encoded bearer caps */
        
    if(check_write_index((UBYTE)(em_report_len)))         /* just to ensure no buffer overflow */
    {
      em_cc_event_buffer[em_cc_buffer_write++] = event_no;
      em_cc_event_buffer[em_cc_buffer_write++] = (em_report_len - 2);
      for (i=0; i<(bc_len); i++)                          /* Starting with the bearercap IEI */
      {
        em_cc_event_buffer[em_cc_buffer_write++] = *ptr1++;      /* Bearer caps encoded */
      }
      /* Only bearer caps should be recorded, therefore decrement em_cc_buffer_write */
      em_cc_event_buffer[em_cc_buffer_write++] = value2;  /* TI */
   
      return FALSE; /* Data is stored inside buffer, reset flag */
    } /* check_write_index */
  } /* BC tag 0x04*/
  else
  {
    TRACE_FUNCTION ("EM_CC: No BC tag found");
  }/* no BC tag 0x04*/
  return TRUE;  /* No more space inside buffer or no BC tag, serve flag next time */
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
  TRACE_FUNCTION ("cc_check_write_index()");

  if (em_cc_buffer_write + n < EM_CC_BUFFER_SIZE)
  {
    /*
     * ACI is informed about the first event trace,
     * used for later data processing.
     */
    if (em_cc_trace_occured == 0) 
    {
      PALLOC(data, EM_DATA_IND);
      data->entity = EM_CC;
      PSENDX(MMI, data);
      em_cc_trace_occured++;
    }
    return TRUE;
  }
  else
  {
    TRACE_ERROR("EM_CC: buffer overflow");
    return FALSE;
  }
}


/*
+------------------------------------------------------------------------------
|  Function     : em_init_cc_event_trace
+------------------------------------------------------------------------------
|  Description  :  Initialize the event tracing flags for SMS
|
|  Parameters   :  
|
|  Return       :  
|
+------------------------------------------------------------------------------
*/

GLOBAL void em_init_cc_event_trace(void)
{
  UBYTE i;

  TRACE_FUNCTION ("em_init_cc_event_trace()");

  for (i = 1; i < EM_MAX_CC_EVENTS; i++)
    cc_v[i] = 0;

  em_cc_buffer_write = 0;
}


/*
+------------------------------------------------------------------------------
|  Function     : cc_em_cc_event_req
+------------------------------------------------------------------------------
|  Description  :  Set the event tracing flags according the bitmask
|
|  Parameters   :  Primitive    - Bitmask
|
|  Return       :  
|
+------------------------------------------------------------------------------
*/

GLOBAL void cc_em_cc_event_req (T_EM_CC_EVENT_REQ *em_cc_event_req)
{
  UBYTE i;

  TRACE_FUNCTION ("cc_em_cc_event_req()");

   /*
    * The event tracing flags are set according the bitmask. cc_v[i] are 
    * the flags belonging to the event number described in 8443.601
    */
  for(i = 1; i < 33; i++)
    cc_v[i] = ((em_cc_event_req->bitmask_cc_l & (0x01<<(i-1))) > 0) ? TRUE : FALSE;

  for(i = 33; i < EM_MAX_CC_EVENTS; i++)
    cc_v[i] = ((em_cc_event_req->bitmask_cc_h & (0x01<<(i-1))) > 0) ? TRUE : FALSE;

  /*
   * A new event trace is generated therefor the flag is reset.
   */
  em_cc_trace_occured = 0;

  PFREE(em_cc_event_req);
}

#endif /* FF_EM_MODE */

#endif /* CC_EM_C */
