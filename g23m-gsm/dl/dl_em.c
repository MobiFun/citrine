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

#ifndef DL_EM_C
#define DL_EM_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_DL

/*==== INCLUDES ===================================================*/
#include "typedefs.h"
#include <string.h>
#include "pconst.cdg"
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
#include "dl_em.h"

#ifdef FF_EM_MODE
#ifdef _SIMULATION_
#pragma message("this build includes engineering mode (FF_EM_MODE)")
#endif

/*==== EXPORT =====================================================*/
/*
 * These variables are used between entities. Even this is not a clean solution
 * it is a straigth forward way to reduce the overhead to a minimum. A clean
 * solution would be based on an only usage of primitives which would stress
 * the OS without aditional advantage!!
*/
GLOBAL UBYTE  em_dl_sem_buffer [EM_DL_SEM_SIZE];
GLOBAL UBYTE  em_dl_sem_index      = 0;
GLOBAL UBYTE  em_dl_sem_read_index = 0;

/* Event tracing flags for EM */
GLOBAL BOOL dl_v[EM_MAX_DL_EVENTS];

/*==== PRIVAT =====================================================*/
static T_HANDLE  sem_EM_DL;
static UBYTE em_dl_trace_occured;

#if !defined(INVOKE_SIGNAL)
LOCAL UBYTE em_dl_write  (UBYTE length, UBYTE * data);
LOCAL void  dl_em_first_event_check(void);/*for ACI notification of first EM event*/
#endif  /* !INVOKE_SIGNAL */

/*==== VARIABLES ==================================================*/



/*
+------------------------------------------------------------------------------
|  Function     : dl_em_buffer_4
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
#define THIS_EM_BUFFER_LEN  4
 UBYTE em_dl_event_buffer[THIS_EM_BUFFER_LEN];

 TRACE_EVENT_WIN ("em_write_buffer_4");

 /*
   ACI is informed about the first event trace, used for later data processing.
 */

 dl_em_first_event_check();

 em_dl_event_buffer[0] = event_no;              /* Event number */
 em_dl_event_buffer[1] = THIS_EM_BUFFER_LEN-2;  /* Value length - 0 equals no data */
 em_dl_event_buffer[2] = value1;                /* first data info */
 em_dl_event_buffer[3] = value2;                /* second data info */

 return ( em_dl_write (THIS_EM_BUFFER_LEN, em_dl_event_buffer) ); /* Data is stored inside buffer, reset flag */
#undef  THIS_EM_BUFFER_LEN
}

/*
+------------------------------------------------------------------------------
|  Function     : dl_em_write_buffer_5a
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, data value1, data value2, data cs (USHORT)
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/

GLOBAL UBYTE em_write_buffer_5a (UBYTE event_no, UBYTE value1, UBYTE value2, USHORT cs)
{
#define THIS_EM_BUFFER_LEN  6
 UBYTE em_dl_event_buffer[THIS_EM_BUFFER_LEN];

 TRACE_EVENT_WIN ("em_write_buffer_5a");

 /*
   ACI is informed about the first event trace, used for later data processing.
 */
 dl_em_first_event_check();

 em_dl_event_buffer[0] = event_no;              /* Event number */
 em_dl_event_buffer[1] = THIS_EM_BUFFER_LEN-2;  /* Value length - 0 equals no data */
 em_dl_event_buffer[2] = value1;                /* first data info */
 em_dl_event_buffer[3] = value2;                /* second data info */
 em_dl_event_buffer[4] = (UBYTE)(cs >> 8);      /* Data to be stored - MSB first */
 em_dl_event_buffer[5] = (UBYTE)(cs);           /* LSB second */

 return ( em_dl_write (THIS_EM_BUFFER_LEN, em_dl_event_buffer) );/* Data is stored inside buffer, reset flag */
#undef  THIS_EM_BUFFER_LEN
}

/*
+------------------------------------------------------------------------------
|  Function     : em_init_dl_event_trace
+------------------------------------------------------------------------------
|  Description  :  Initialize the event tracing flags
|
|  Parameters   :
|
|  Return       :
|
+------------------------------------------------------------------------------
*/
GLOBAL void em_init_dl_event_trace(void)
{
 UBYTE i;

 TRACE_EVENT_WIN ("em_init_dl_event_trace");

 for(i=1; i<EM_MAX_DL_EVENTS; i++)
 {
   dl_v[i] = 0;
 }
}

/*
+------------------------------------------------------------------------------
|  Function     : dl_em_dl_event_req
+------------------------------------------------------------------------------
|  Description  :  Set the event tracing flags according the bitmask
|
|  Parameters   :  Primitive    - Bitmask
|
|  Return       :
|
+------------------------------------------------------------------------------
*/

GLOBAL void dl_em_dl_event_req (T_EM_DL_EVENT_REQ *em_dl_event_req)
{
 UBYTE i;

 TRACE_EVENT_WIN_P1 ("dl_em_dl_event_req: bitmask_dl=%x", em_dl_event_req->bitmask_dl);
 /*
  *  The event tracing flags are set according the bitmask. dl_v[i] are
  *  the flags belonging to the event number described in 8443.601
 */
 for(i=1; i<EM_MAX_DL_EVENTS; i++)
 {
   dl_v[i] = ((em_dl_event_req->bitmask_dl & (0x01<<(i-1))) > 0) ? TRUE : FALSE;
 }

 /*
   A new event trace is generated therefor the flag is set to 0.
 */
 em_dl_trace_occured = 0;

 PFREE(em_dl_event_req);
}

LOCAL void em_dl_sem_clear (void)
{
  ENTER_CRITICAL_SECTION (sem_EM_DL);
  em_dl_sem_index = em_dl_sem_read_index = 0;
  LEAVE_CRITICAL_SECTION (sem_EM_DL);

  SYST_TRACE ("DL:em_dl_sem_index cleared");
}

GLOBAL void em_dl_sem_init (void)
{
  sem_EM_DL  = vsi_s_open (VSI_CALLER "EM_DL_SEM",1);

  if (sem_EM_DL NEQ VSI_ERROR)
  {
    em_dl_sem_clear ();
  }
  else
  {
    SYST_TRACE ("DL:can't open semaphore \"EM_DL_SEM\"");
  }

  em_dl_trace_occured = 0;
}

GLOBAL void em_dl_sem_exit (void)
{
  if (sem_EM_DL NEQ VSI_ERROR)
    vsi_s_close (VSI_CALLER sem_EM_DL);
}

/*
To be able to read the dl-buffer from aci, em_dl_sem_read occupies the semaphor with ENTER_CRITICAL_SECTION.
After read process toke place the function em_dl_sem_clear must be called to open the semaphor again and
reset the read index.
*/
GLOBAL void em_dl_sem_reset (void)
{
/*
  ENTER_CRITICAL_SECTION (sem_EM_DL);
*/
  em_dl_sem_index = em_dl_sem_read_index = 0;
  LEAVE_CRITICAL_SECTION (sem_EM_DL);

  SYST_TRACE ("DL:em_dl_sem_index reseted");
}

GLOBAL void em_dl_sem_read (void)
{
  USHORT  semCount;
  TRACE_EVENT_WIN ( "em_dl_sem_read");

  if (vsi_s_status (VSI_CALLER sem_EM_DL, &semCount) NEQ VSI_OK)
  {
    com_semaphore_err();
    return ;
  }
  if (semCount EQ 0)
  {
    SYST_TRACE ("DL:semCount == 0");
    return ;
  }
 ENTER_CRITICAL_SECTION (sem_EM_DL);
/*
 The dl semaphor will be read by the engineering mode via aci, therefore the functions em_dl_sem_read
 and em_dl_sem_reset are defined as global. To ensure that during reading only aci has access to
 the semaphor the macro LEAVE_CRITICAL_SECTION is called after the read process toke place -
 in the em_dl_sem_reset function.
*/
}

/*
  Return value TRUE/FALSE - TRUE keeps the event flag valid, FALSE indicates a successful flag handle
*/
#if defined(INVOKE_SIGNAL)
GLOBAL UBYTE em_dl_write (UBYTE length, UBYTE *data)
#else /* INVOKE_SIGNAL */
LOCAL UBYTE em_dl_write (UBYTE length, UBYTE *data)
#endif /* INVOKE_SIGNAL */
{
  USHORT  semCount;
  GET_INSTANCE_DATA; 

  TRACE_EVENT_WIN ("em_dl_write");

  if (dl_data->interrupt_context)
  {
    sig_invoke_dl_em_write (length, data);
    return FALSE;
  }

  if (vsi_s_status (VSI_CALLER sem_EM_DL, &semCount) NEQ VSI_OK)
  {
    com_semaphore_err();
    return TRUE;
  }

  if (semCount EQ 0)
  {
    TRACE_EVENT_WIN ("semCount == 0");
    return TRUE;
  }

  /*
  *  buffer overflow protection - EM_DL_SEM_SIZE = 30
  */
  if ( (em_dl_sem_index + length) > EM_DL_SEM_SIZE )
  {
    TRACE_EVENT_WIN ("dl em buffer full");
    return FALSE;
  }

  if (com_enter_critical_section (sem_EM_DL))
    return TRUE;

  memcpy (&em_dl_sem_buffer[em_dl_sem_index], data, length);
  em_dl_sem_index += length;

  com_leave_critical_section (sem_EM_DL);
  return FALSE;                                           /* indicates that flag was handled */
} /* endfunc em_dl_write */

/*
+------------------------------------------------------------------------------
|  Function     : dl_em_first_event_check()
+------------------------------------------------------------------------------
|  Description  :  Checks if first EM-Event ocured
|
|  Parameters   :  None
|
|  Return       :  None
|
+------------------------------------------------------------------------------
*/

 /*
   ACI is informed about the first event trace, used for later data processing.
 */
#if defined(INVOKE_SIGNAL)
GLOBAL void dl_em_first_event_check(void)
#else
LOCAL void  dl_em_first_event_check(void)
#endif  /* !INVOKE_SIGNAL */
{
  if(!em_dl_trace_occured)
  {

#if defined(INVOKE_SIGNAL)
    GET_INSTANCE_DATA; 
    if (dl_data->interrupt_context)
    {
      sig_invoke_dl_em_first_event_check ();
      return;
    }
#endif  /* INVOKE_SIGNAL */

    TRACE_FUNCTION("dl_em_first_event_check()");

    {
      PALLOC(em_notification, EM_DATA_IND);
      em_notification->entity = EM_DL;
      PSENDX(MMI, em_notification);
    }

    em_dl_trace_occured++;
  }
}

#endif /* FF_EM_MODE */

#endif /* DL_EM_C */
