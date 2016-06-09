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
|  Purpose :  This Module defines the engineering mode (EM) device driver for
|	      the G23 protocol stack. This driver is used to control all
}	      engineering mode related functions.
+----------------------------------------------------------------------------- 
*/ 

#ifndef MM_EM_C
#define MM_EM_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_MM

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
#include "cnf_mm.h"
#include "mon_mm.h"
#include "pei.h"
#include "tok.h"
#include "mm.h"
#include "mm_em.h"

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

#ifdef FF_EM_MODE
GLOBAL UBYTE em_mm_event_buffer[EM_MM_BUFFER_SIZE];
GLOBAL UBYTE em_mm_buffer_write;

/* Event tracing flags for EM */
GLOBAL BOOL mm_v[EM_MAX_MM_EVENTS];

static UBYTE em_mm_trace_occured;
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
  TRACE_FUNCTION ("mm_em_write_buffer_2()");
 
  if (check_write_index(2))
  {                        
    em_mm_event_buffer[em_mm_buffer_write++] = event_no; /* Tag: Event number */    
    em_mm_event_buffer[em_mm_buffer_write++] = 0;        /* Length: 0 means no value */
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

GLOBAL UBYTE em_write_buffer_3  (UBYTE event_no, UBYTE value)
{
  TRACE_FUNCTION ("mm_em_write_buffer_3()");
 
  if (check_write_index(3))
  {
    em_mm_event_buffer[em_mm_buffer_write++] = event_no; /* Tag: Event number */    
    em_mm_event_buffer[em_mm_buffer_write++] = 1;        /* Length: 0 means no value */ 
    em_mm_event_buffer[em_mm_buffer_write++] = value;    /* Value: Data to be stored */   
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
  TRACE_FUNCTION ("mm_em_write_buffer_3a()");
 
  if (check_write_index(4))
  {
    em_mm_event_buffer[em_mm_buffer_write++] = event_no; /* Tag: Event number */
    em_mm_event_buffer[em_mm_buffer_write++] = 2;        /* Length: 0 means no value */
    
    /* Value: Data to be stored. MSB first, LSB second */
    em_mm_event_buffer[em_mm_buffer_write++] = (UBYTE)(value >> 8);
    em_mm_event_buffer[em_mm_buffer_write++] = (UBYTE)(value);
    return FALSE; /* Data is stored inside buffer, reset flag */
  }
  else
    return TRUE;  /* No more space inside buffer, serve flag next time */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_3b
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, data value (ULONG)
|
|  Return       :  TRUE/FALSE                               
| 
|  Pupose       :  Used for storing TMSI                                                         
+------------------------------------------------------------------------------
*/
#define RINGBUF_3B_LENGTH (2 + 4)
GLOBAL UBYTE em_write_buffer_3b (UBYTE event_no, ULONG value)
{
  TRACE_FUNCTION ("mm_em_write_buffer_3b()");
 
  if (check_write_index(RINGBUF_3B_LENGTH))
  {
    em_mm_event_buffer[em_mm_buffer_write++] = event_no;					/* Tag: Event number */
    em_mm_event_buffer[em_mm_buffer_write++] = RINGBUF_3B_LENGTH-2;        /* Length: 0 means no value */
    
    /* Value: Data to be stored. MSB first, LSB second */
    em_mm_event_buffer[em_mm_buffer_write++] = (UBYTE)(value>>24); /* MSB first */
    em_mm_event_buffer[em_mm_buffer_write++] = (UBYTE)(value>>16); /* new TMSI */
    em_mm_event_buffer[em_mm_buffer_write++] = (UBYTE)(value>> 8);
    em_mm_event_buffer[em_mm_buffer_write++] = (UBYTE)(value);     /* LSB last */
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
|  Parameters   :  Event number, data value1, data plmn (T_plmn)
|
|  Return       :  TRUE/FALSE                               
| 
|  Pupose       :  Used for storing PLMN list                                                      
+------------------------------------------------------------------------------
*/

GLOBAL UBYTE em_write_buffer_4 (UBYTE event_no, UBYTE value1, T_plmn *plmn)
{
  UBYTE i;
  UBYTE n = 2;
 
  TRACE_FUNCTION ("mm_em_write_buffer_4()");
   
  n += ((SIZE_MCC + SIZE_MNC) * value1);  /*Compute exact size of data to be stored*/ 
  
  if(check_write_index(n))
  {
    em_mm_event_buffer[em_mm_buffer_write++] = event_no;   /* Tag: Event number */   
    em_mm_event_buffer[em_mm_buffer_write++] = (n - 2);    /* Length: 0 means no value */
    for (i = 0; i < value1; i++)  /*stores MCC and MNC of every available PLMN*/
    {
      if(plmn[i].v_plmn)
      {
        memcpy(&em_mm_event_buffer[em_mm_buffer_write], plmn[i].mcc, SIZE_MCC);
        em_mm_buffer_write += SIZE_MCC;
        memcpy(&em_mm_event_buffer[em_mm_buffer_write], plmn[i].mnc, SIZE_MNC);
        em_mm_buffer_write += SIZE_MNC;
      } /*valid*/
    }  /*for*/
    return FALSE; /* Data is stored inside buffer, reset flag */
  }/* check write index*/
  else
  {
    return TRUE;  /* No more space inside buffer, serve flag next time */
  }
}
/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_4a
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, data value1 (UBYTE), data value2 (USHORT)
|
|  Return       :  TRUE/FALSE                               
| 
|  Pupose       :  Used for storing Location Update Reject                                                         
+------------------------------------------------------------------------------
*/
#define RINGBUF_4A_LENGTH (2 + 1 + 2)
GLOBAL UBYTE em_write_buffer_4a (UBYTE event_no, UBYTE value1, USHORT value2)
{
  TRACE_FUNCTION ("mm_em_write_buffer_4a()");
 
  if (check_write_index(RINGBUF_4A_LENGTH))
  {
    em_mm_event_buffer[em_mm_buffer_write++] = event_no;					/* Tag: Event number */
    em_mm_event_buffer[em_mm_buffer_write++] = RINGBUF_4A_LENGTH-2;        /* Length: 0 means no value */
    em_mm_event_buffer[em_mm_buffer_write++] = value1;
    em_mm_event_buffer[em_mm_buffer_write++] = (UBYTE)(value2 >> 8);
    em_mm_event_buffer[em_mm_buffer_write++] = (UBYTE)(value2);
  
    return FALSE; /* Data is stored inside buffer, reset flag */

  }/* check write index*/
  else
    return TRUE;  /* No more space inside buffer, serve flag next time */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_4b
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, data identity type (UBYTE), data value (UBYTE *)
|
|  Return       :  TRUE/FALSE                               
| 
|  Pupose       :  Used for storing  IMEI, IMEISV, IMSI during Identity Request/Response                                                     
+------------------------------------------------------------------------------
*/

GLOBAL UBYTE em_write_buffer_4b (UBYTE event_no, UBYTE ident_type, UBYTE *value)
{
  
  UBYTE n;
  UBYTE digits;
  
  TRACE_FUNCTION ("mm_em_write_buffer_4b()");
  
  switch (ident_type) /*Creating length of data to be stored*/
    {
      case ID_TYPE_IMEI:
        digits = 14;  /*length of IMEI */
        break;
      case ID_TYPE_IMSI:
      case ID_TYPE_IMEISV:
        digits = 16;  /*length of IMSI / IMEISV */
        break;
      default: /* Illegal mobile identity - not possible because of calling function*/
        break;
    } /*switch*/
    n = 3 + digits; /* Tag + Length + Type + Digits */ /*lint !e644*/ /*for lint: Digits are
                                              always initialized because of calling function*/
  
  if (check_write_index(n))
  {
    em_mm_event_buffer[em_mm_buffer_write++] = event_no;					/* Tag: Event number */
    em_mm_event_buffer[em_mm_buffer_write++] = (n - 2);        /* Length: 0 means no value */
    em_mm_event_buffer[em_mm_buffer_write++] = ident_type;
    memcpy (&em_mm_event_buffer[em_mm_buffer_write], value, digits);
    em_mm_buffer_write += digits;
    return FALSE; /* Data is stored inside buffer, reset flag */

  }/* check write index*/
  else
    return TRUE;  /* No more space inside buffer, serve flag next time */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_4c
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, data identity type (UBYTE), data value (ULONG)
|
|  Return       :  TRUE/FALSE                               
| 
|  Pupose       :  Used during Identity Request/Response for storing TMSI                                                       
+------------------------------------------------------------------------------
*/
#define RINGBUF_4C_LENGTH (2 + 1 + 4)
GLOBAL UBYTE em_write_buffer_4c (UBYTE event_no, UBYTE ident_type, ULONG value)
{
  TRACE_FUNCTION ("mm_em_write_buffer_4c()");
  if (check_write_index(RINGBUF_4C_LENGTH))
  {
    em_mm_event_buffer[em_mm_buffer_write++] = event_no;					/* Tag: Event number */
    em_mm_event_buffer[em_mm_buffer_write++] = (RINGBUF_4C_LENGTH - 2);        /* Length: 0 means no value */
    em_mm_event_buffer[em_mm_buffer_write++] = ident_type;
    em_mm_event_buffer[em_mm_buffer_write++] = (UBYTE)(value>>24);    /* MSB first */
    em_mm_event_buffer[em_mm_buffer_write++] = (UBYTE)(value>>16);
    em_mm_event_buffer[em_mm_buffer_write++] = (UBYTE)(value>> 8);
    em_mm_event_buffer[em_mm_buffer_write++] = (UBYTE)(value);        /* LSB last */
    return FALSE; /* Data is stored inside buffer, reset flag */
  }/* check write index*/
  else
    return TRUE;  /* No more space inside buffer, serve flag next time */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_5a
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, data value (UBYTE), data mcc (UBYTE), data mnc (UBYTE)
|
|  Return       :  TRUE/FALSE                               
| 
|  Pupose       :  Used for storing Service Mode, MCC und MNC during registration
|                  Used for storing data during cell reselection                                                         
+------------------------------------------------------------------------------
*/
#define RINGBUF_5A_LENGTH (2 + 1 + SIZE_MCC + SIZE_MNC)
GLOBAL UBYTE em_write_buffer_5a (UBYTE event_no, UBYTE value, UBYTE mcc[SIZE_MCC], UBYTE mnc[SIZE_MNC])
{
  TRACE_FUNCTION ("mm_em_write_buffer_5a()");
 
  if (check_write_index(RINGBUF_5A_LENGTH))
  {
    em_mm_event_buffer[em_mm_buffer_write++] = event_no;					/* Tag: Event number */
    em_mm_event_buffer[em_mm_buffer_write++] = RINGBUF_5A_LENGTH-2;        /* Length: 0 means no value */
    em_mm_event_buffer[em_mm_buffer_write++] = value;

/*missing check for validy*/
      memcpy(&em_mm_event_buffer[em_mm_buffer_write], mcc, SIZE_MCC);
      em_mm_buffer_write += SIZE_MCC;
      memcpy(&em_mm_event_buffer[em_mm_buffer_write], mnc, SIZE_MNC);
      em_mm_buffer_write += SIZE_MNC;

    return FALSE; /* Data is stored inside buffer, reset flag */
  }/* check write index*/
  else
    return TRUE;  /* No more space inside buffer, serve flag next time */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_6
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, data location update type (UBYTE), data lai (T_loc_area_ident)
|
|  Return       :  TRUE/FALSE                               
| 
|  Pupose       :  Used for storing Location Update Type, MCC, MNC and LAC
+------------------------------------------------------------------------------
*/
#define RINGBUF_6_LENGTH (2 + 1 + SIZE_MCC + SIZE_MNC + 2)
GLOBAL UBYTE em_write_buffer_6 (UBYTE event_no, UBYTE loc_upd_type, T_loc_area_ident lai)
{
  TRACE_FUNCTION ("mm_em_write_buffer_6()");
 
  if (check_write_index(RINGBUF_6_LENGTH))
  {
    em_mm_event_buffer[em_mm_buffer_write++] = event_no;					/* Tag: Event number */
    em_mm_event_buffer[em_mm_buffer_write++] = RINGBUF_6_LENGTH - 2;        /* Length: 0 means no value */
    em_mm_event_buffer[em_mm_buffer_write++] = loc_upd_type;
    memcpy(&em_mm_event_buffer[em_mm_buffer_write], lai.mcc, SIZE_MCC);  /*stores MCC*/
    em_mm_buffer_write += SIZE_MCC;
    memcpy(&em_mm_event_buffer[em_mm_buffer_write], lai.mnc, SIZE_MNC);  /*stores MNC*/
    em_mm_buffer_write += SIZE_MNC;
    em_mm_event_buffer[em_mm_buffer_write++] = (UBYTE)(lai.lac >> 8);    /*stores LAC*/
    em_mm_event_buffer[em_mm_buffer_write++] = (UBYTE)(lai.lac);
    return FALSE; /* Data is stored inside buffer, reset flag */
  }/* check write index*/
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
  TRACE_FUNCTION ("mm_check_write_index()");
 
  if (em_mm_buffer_write + n < EM_MM_BUFFER_SIZE)
  {
    /*
     * ACI is informed about the first event trace, 
     * used for later data processing.
     */
    if (em_mm_trace_occured EQ 0) 
    {
      PALLOC(data, EM_DATA_IND);
      data->entity = EM_MM;
      PSENDX(MMI, data);
      em_mm_trace_occured++;
    }
    return TRUE;
  }
  else
    return FALSE;
}

/*
+------------------------------------------------------------------------------
|  Function     : mm_em_mm_event_req
+------------------------------------------------------------------------------
|  Description  :  Set the event tracing flags according the bitmask for dl
|
|  Parameters   :  Primitive    - Bitmask
|
|  Return       :                                 
|                                                           
+------------------------------------------------------------------------------
*/

GLOBAL void mm_em_dl_event_req (T_EM_DL_EVENT_REQ *em_dl_event_req)
{
  TRACE_FUNCTION ("mm_em_dl_event_req()");

  PSENDX(DL, em_dl_event_req);
}

/*
+------------------------------------------------------------------------------
|  Function     : em_init_mm_event_trace
+------------------------------------------------------------------------------
|  Description  :  Initialize the event tracing flags for MM
|
|  Parameters   :  void                   
|
|  Return       :  void                           
|                                                           
+------------------------------------------------------------------------------
*/
GLOBAL void em_init_mm_event_trace(void)
{
  UBYTE i;

  TRACE_FUNCTION ("em_init_mm_event_trace()");

  for (i = 0; i < EM_MAX_MM_EVENTS; i++)
    mm_v[i] = 0;

  em_mm_buffer_write = 0;
}

/*
+------------------------------------------------------------------------------
|  Function     : mm_em_mm_event_req
+------------------------------------------------------------------------------
|  Description  :  Set the event tracing flags according the bitmask
|
|  Parameters   :  Primitive    - Bitmask
|
|  Return       :                                 
|                                                           
+------------------------------------------------------------------------------
*/

GLOBAL void mm_em_mm_event_req (T_EM_MM_EVENT_REQ *em_mm_event_req)
{
  UBYTE i;

  TRACE_FUNCTION ("mm_em_mm_event_req()");

  /*
   *  The event tracing flags are set according the bitmask. mm_v[i] are 
   *  the flags belonging to the event number described in 8443.601
   */ 
  for(i = 1; i < EM_MAX_MM_EVENTS; i++)
    mm_v[i] = ((em_mm_event_req->bitmask_mm & (0x01<<(i-1))) > 0) ? TRUE : FALSE;

  /*
   * A new event trace is generated therefore the flag is reset.
   */
  em_mm_trace_occured = 0;

  PFREE(em_mm_event_req);
}
#endif /* FF_EM_MODE */

#endif /* MM_EM_C */
