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

#ifndef ALR_EM_C
#define ALR_EM_C

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

#if 0 //#ifdef GPRS
#ifdef _TARGET_
#include "inth/iq.h"
#endif
/*
 *	obsolete (msb / 2002-04-25)
#include "armio/armio.h"
 */
#endif

#include "alr.h"
#include "alr_em.h"


/*==== IMPORT =====================================================*/

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/
#if defined (FF_EM_MODE) AND defined (ALR)

/* Event tracing flags for EM */
GLOBAL BOOL alr_v[EM_MAX_ALR_EVENTS];

LOCAL  UBYTE             em_alr_trace_occured = 0;


/*
  These variables are used between entities. Even this is not a clean solution it is a straigth forward
  way to reduce the overhead to a minimum. A clean solution would be based on an only usage of primitives
  which would stress the os with no aditional advantage!!
*/
GLOBAL volatile  UBYTE             em_l1_sem_buffer [EM_L1_SEM_SIZE];    /*lint -esym(765,em_l1_sem_buffer) | external could be made static | used externally  */
GLOBAL volatile  UBYTE             em_l1_sem_index  = 0;								 /*lint -esym(765,em_l1_sem_index)  | external could be made static | used externally  */

static T_HANDLE  sem_EM_L1;

#define ENTER_CRITICAL_SECTION(sem) if (alr_em_enter_critical_section(sem))return FALSE;
#define LEAVE_CRITICAL_SECTION(sem) if (alr_em_leave_critical_section(sem))return FALSE;

#endif /* FF_EM_MODE */

/*==== FUNCTIONS ==================================================*/
#if defined (FF_EM_MODE) AND defined (ALR)

LOCAL  UBYTE em_l1_sem       (UBYTE length, UBYTE * data);
LOCAL  int   em_l1_sem_clear (void);
LOCAL  void  alr_em_first_event_check (void); /*Check for ACI - Notification*/

static void alr_em_semaphore_err (void);

#if !defined(SYST_TRACE)
#define SYST_TRACE(a) vsi_o_ttrace(0, TC_SYSTEM, a);
#endif  /* !SYST_TRACE */

static int alr_em_enter_critical_section (T_HANDLE sem);
static int alr_em_leave_critical_section (T_HANDLE sem);


/*
+------------------------------------------------------------------------------
|  Function     : em_init_l1_event_trace
+------------------------------------------------------------------------------
|  Description  :  Initialize the event tracing flags
|
|  Parameters   :
|
|  Return       :
|
+------------------------------------------------------------------------------
*/
GLOBAL void em_init_l1_event_trace(void)
{
 UBYTE i;

 TRACE_FUNCTION("em_init_l1_event_trace ()");

 for(i=0; i<EM_MAX_ALR_EVENTS; i++)
   alr_v[i] = 0;
}

/*
+------------------------------------------------------------------------------
|  Function     : l1_em_l1_event_req
+------------------------------------------------------------------------------
|  Description  :  Set the event tracing flags according the bitmask
|
|  Parameters   :  Primitive    - Bitmask
|
|  Return       :
|
+------------------------------------------------------------------------------
*/

GLOBAL void l1_em_l1_event_req (T_EM_L1_EVENT_REQ *em_l1_event_req)
{
 UBYTE i;

 TRACE_FUNCTION("l1_em_l1_event_req ()");

 /*
  *  The event tracing flags are set according the bitmask. alr_v[i] are
  *  the flags belonging to the event number described in 8443.601
 */
 for(i=1; i<33; i++)  {
   alr_v[i] = ((em_l1_event_req->bitmask_l1_l & (0x01<<(i-1))) > 0) ? TRUE : FALSE;
 }

 for(i=33; i<(EM_MAX_ALR_EVENTS); i++)  {
   alr_v[i] = ((em_l1_event_req->bitmask_l1_h & (0x01<<(i-1))) > 0) ? TRUE : FALSE;
 }

 /*
  * A new event trace is generated therefor the flag is set to zero.
 */
 em_alr_trace_occured = 0;

 PFREE(em_l1_event_req);
}

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
 UBYTE em_l1_event_buffer[2];
 UBYTE em_l1_buffer_write = 0;
 UBYTE length = 2;

 TRACE_FUNCTION("alr_em_write_buffer_2 ()");

 /*
   ACI is informed about the first event trace, used for later data processing.
 */
 alr_em_first_event_check();

 memset(em_l1_event_buffer, 0, 2);

 em_l1_event_buffer[em_l1_buffer_write++] = event_no;             /* Event number */
 em_l1_event_buffer[em_l1_buffer_write++] = length-2;             /* Value length - 0 equals no data */

 return (em_l1_sem (length, em_l1_event_buffer));               /* Data is stored inside buffer, reset flag */
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
 UBYTE em_l1_event_buffer[3];
 UBYTE em_l1_buffer_write = 0;
 UBYTE length = 3;

 TRACE_FUNCTION("alr_em_write_buffer_3 ()");

 /*
   ACI is informed about the first event trace, used for later data processing.
 */
 alr_em_first_event_check();

 memset(em_l1_event_buffer, 0, 3);

 em_l1_event_buffer[em_l1_buffer_write++] = event_no;               /* Event number */
 em_l1_event_buffer[em_l1_buffer_write++] = length-2;               /* Value length - 0 equals no value */
 em_l1_event_buffer[em_l1_buffer_write++] = value;                  /* Data to be stored */

 return (em_l1_sem (length, em_l1_event_buffer));                 /* Data is stored inside buffer, reset flag */
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
 UBYTE em_l1_event_buffer[4];
 UBYTE em_l1_buffer_write = 0;
 UBYTE length = 4;

 TRACE_FUNCTION("alr_em_write_buffer_4 ()");

 /*
   ACI is informed about the first event trace, used for later data processing.
 */
 alr_em_first_event_check();

 memset(em_l1_event_buffer, 0, 4);

 em_l1_event_buffer[em_l1_buffer_write++] = event_no;              /* Event number */
 em_l1_event_buffer[em_l1_buffer_write++] = length-2;              /* Value length - 0 equals no value */
 em_l1_event_buffer[em_l1_buffer_write++] = (UBYTE)(value >> 8);   /* Data to be stored - MSB first */
 em_l1_event_buffer[em_l1_buffer_write++] = (UBYTE)(value);        /* LSB second */

 return (em_l1_sem (length, em_l1_event_buffer));                /* Data is stored inside buffer, reset flag */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_4
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, data value1 & value2
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/
GLOBAL UBYTE em_write_buffer_4  (UBYTE event_no, UBYTE value1, UBYTE value2)
{
 UBYTE em_l1_event_buffer[4];
 UBYTE em_l1_buffer_write = 0;
 UBYTE length = 4;

 TRACE_FUNCTION("alr_em_write_buffer_4 ()");

 /*
   ACI is informed about the first event trace, used for later data processing.
 */
 alr_em_first_event_check();

 memset(em_l1_event_buffer, 0, 4);

 em_l1_event_buffer[em_l1_buffer_write++] = event_no;               /* Event number */
 em_l1_event_buffer[em_l1_buffer_write++] = length-2;               /* Value length - 0 equals no value */
 em_l1_event_buffer[em_l1_buffer_write++] = value1;                 /* Data to be stored */
 em_l1_event_buffer[em_l1_buffer_write++] = value2;                 /* Data to be stored */

 return (em_l1_sem (length, em_l1_event_buffer));                 /* Data is stored inside buffer, reset flag */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_l1_sem_init
+------------------------------------------------------------------------------
|  Description  :  Initialize the semaphor for alr event traces.
|
|  Parameters   :  void
|
|  Return       :  void
|
+------------------------------------------------------------------------------
*/
GLOBAL void em_l1_sem_init (void)
{
  sem_EM_L1  = vsi_s_open (VSI_CALLER "EM_L1_SEM",1);

  if (sem_EM_L1 NEQ VSI_ERROR)
    em_l1_sem_clear ();
  else
    SYST_TRACE ("D1:can't open semaphore \"EM_D1_SEM\"");
}

/*
+------------------------------------------------------------------------------
|  Function     : em_l1_sem_exit
+------------------------------------------------------------------------------
|  Description  :  Close the semaphor for alr event traces.
|
|  Parameters   :  void
|
|  Return       :  void
|
+------------------------------------------------------------------------------
*/
GLOBAL void em_l1_sem_exit (void)
{
  if (sem_EM_L1 NEQ VSI_ERROR)
    vsi_s_close (VSI_CALLER sem_EM_L1);
}

/*
+------------------------------------------------------------------------------
|  Function     : em_l1_sem_clear
+------------------------------------------------------------------------------
|  Description  :  Reset the index of the semaphor.
|
|  Parameters   :  void
|
|  Return       :  UBYTE
|
+------------------------------------------------------------------------------
*/
LOCAL int em_l1_sem_clear (void)
{
  ENTER_CRITICAL_SECTION (sem_EM_L1);
  em_l1_sem_index = 0;
  LEAVE_CRITICAL_SECTION (sem_EM_L1);

  SYST_TRACE ("L1:em_l1_sem_index cleared");
  return TRUE;
}

/*
+------------------------------------------------------------------------------
|  Function     : em_l1_sem_reset
+------------------------------------------------------------------------------
|  Description  :  Clears the content of the semaphor, must called after em_l1_sem_read.
|
|  Parameters   :  void
|
|  Return       :  UBYTE
|
+------------------------------------------------------------------------------
*/
GLOBAL int em_l1_sem_reset (void)  /*lint -esym(765,em_l1_sem_reset) | external could be made static | used externally  */
{
/*  ENTER_CRITICAL_SECTION (sem_EM_L1); */
  em_l1_sem_index = 0;
  LEAVE_CRITICAL_SECTION (sem_EM_L1);

  SYST_TRACE ("L1:em_l1_sem_index reseted");
  return TRUE;
}

/*
+------------------------------------------------------------------------------
|  Function     : em_l1_sem_read
+------------------------------------------------------------------------------
|  Description  :  Reads the content of the semaphor.
|
|  Parameters   :  void
|
|  Return       :  UBYTE
|
+------------------------------------------------------------------------------
*/
GLOBAL int em_l1_sem_read (void)  /*lint -esym(765,em_l1_sem_read) | external could be made static | used externally  */
{
  USHORT    semCount;
  TRACE_FUNCTION ("em_l1_sem_read()");

  if (vsi_s_status (VSI_CALLER sem_EM_L1, &semCount) NEQ VSI_OK)
  {
    alr_em_semaphore_err();
    return TRUE;
  }
  if (semCount EQ 0)
  {
    vsi_o_ttrace(VSI_CALLER TC_EVENT, "semCount = %d", semCount);
    SYST_TRACE ("semCount EQ 0");
    return TRUE;
  }

  ENTER_CRITICAL_SECTION (sem_EM_L1);
  /*
   * The l1/alr semaphor will be read by the engineering mode via aci,
   * therefore the functions em_l1_sem_read and em_l1_sem_reset are defined
   * as global. To ensure that during reading only aci has access to the
   * semaphor the macro LEAVE_CRITICAL_SECTION is called after the read process
   * toke place - in the em_l1_sem_resest function.
   */
  return TRUE;
}

/*
  Return value TRUE/FALSE - TRUE keeps the event flag valid, FALSE indicates a successful flag handle
*/
/*
+------------------------------------------------------------------------------
|  Function     : em_l1_sem
+------------------------------------------------------------------------------
|  Description  :  Writes the data inside the semaphor.
|
|  Parameters   :  void
|
|  Return       :  UBYTE
|
+------------------------------------------------------------------------------
*/
LOCAL UBYTE em_l1_sem (UBYTE length, UBYTE *data)
{
 USHORT    semCount;
 UBYTE     i;

 TRACE_FUNCTION ("em_l1_sem()");


 if (vsi_s_status (VSI_CALLER sem_EM_L1, &semCount) NEQ VSI_OK)
 {
   alr_em_semaphore_err();
   return TRUE;
 }
 if (semCount EQ 0)
 {
   vsi_o_ttrace(VSI_CALLER TC_EVENT, "semCount = %d", semCount);
   SYST_TRACE ("semCount EQ 0");
   return TRUE;
 }

 /*
 *  buffer overflow protection
 */
 if ( (em_l1_sem_index  + length) > EM_L1_SEM_SIZE )
 {
   TRACE_FUNCTION ("alr buffer full");
   return FALSE;
 }

 ENTER_CRITICAL_SECTION(sem_EM_L1);

 for (i=0; i<length; i++)
   em_l1_sem_buffer[em_l1_sem_index++] = *(data++);


 LEAVE_CRITICAL_SECTION (sem_EM_L1);
 return FALSE;                                           /* indicates that flag was handled */
} /* endfunc em_l1_sem */

/*
+------------------------------------------------------------------------------
|  Function     : alr_em_semaphore_err
+------------------------------------------------------------------------------
|  Description  :  Semaphor error
|
|
|  Parameters   :  void
|
|  Return       :  void
|
+------------------------------------------------------------------------------
*/
static void alr_em_semaphore_err (void)
{
 static UCHAR out = 0;
  if (!out)
  {
    out = 1;

/* Implements Measure#32: Row 56 */
    TRACE_EVENT ("semaphore error");
  }
}

/*
+------------------------------------------------------------------------------
|  Function     : alr_em_enter_critical_section
+------------------------------------------------------------------------------
|  Description  :  Check on critical section entrance
|
|
|  Parameters   :  Handle
|
|  Return       :  -1  semaphore error
|					0  Ok.
|
+------------------------------------------------------------------------------
*/

#if defined (NEW_FRAME)
static int alr_em_enter_critical_section (T_HANDLE sem)
#else
static int alr_em_enter_critical_section (T_VSI_SHANDLE sem)
#endif /* NEW_FRAME */
{
 if (vsi_s_get (VSI_CALLER sem) NEQ VSI_OK)
 {
  alr_em_semaphore_err();
  return -1;
 }
 else
 {
  return 0;
 }
}/* endfunc rr_enter_critical_section */


/*
+------------------------------------------------------------------------------
|  Function     : alr_em_leave_critical_section
+------------------------------------------------------------------------------
|  Description  :  Check on critical section exit
|
|
|  Parameters   :  Handle
|
|  Return       :  -1  semaphore error
|					0  Ok.
|
+------------------------------------------------------------------------------
*/
#if defined (NEW_FRAME)
static int alr_em_leave_critical_section (T_HANDLE sem)
#else
static int alr_em_leave_critical_section (T_VSI_SHANDLE sem)
#endif /* NEW_FRAME */
{
 if (vsi_s_release (VSI_CALLER sem) NEQ VSI_OK)
 {
  alr_em_semaphore_err();
  return -1;
 }
 else
 {
  return 0;
 }
}/* endfunc rr_leave_critical_section */

/*
+------------------------------------------------------------------------------
|  Function     : alr_em_error_cause
+------------------------------------------------------------------------------
|  Description  :  Check the error cause and store it in the event buffer
|
|  Parameters   :  Cause
|
|  Return       :  None
|
+------------------------------------------------------------------------------
*/

GLOBAL void alr_em_error_cause (UBYTE cause, USHORT arfcn)

{
  switch(cause)
  {
  case CS_BCCH_READ_ERROR:
    {  /* Power measurement request */
      ALR_EM_BCCH_READ_ERROR;
      break;
    }
  case CS_DOWN_LINK_FAIL:
  {  /* Downlink signalling failure */
    ALR_EM_DOWNLINK_FAILURE;
    break;
  }
  case CS_NO_BCCH_AVAIL:
    {  /* neighbourcell BCCH not available */
      ALR_EM_NEIGHBOURCELL_BCCH(EM_NOT_AVAIL);
      break;
    }
  }/*switch*/
}/* alr_em_error_cause */


/*
+------------------------------------------------------------------------------
|  Function     : alr_em_first_event_check()
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
LOCAL void alr_em_first_event_check (void)

{
 TRACE_FUNCTION("alr_em_first_event_check()");
 if(em_alr_trace_occured EQ 0)
 {
  PALLOC(em_notification, EM_DATA_IND);
  em_notification->entity = EM_L1;
  PSENDX(MMI, em_notification);
  em_alr_trace_occured++;
 }
}/* alr_em_first_event_check */



#endif /* FF_EM_MODE */



#endif /* ALR_EM_C */
