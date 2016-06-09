

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
|  Purpose : Engineering Mode (EM) Declarations + Macros
|
+-----------------------------------------------------------------------------
*/
#ifndef DL_EM_H
#define DL_EM_H

#ifdef FF_EM_MODE

/* ------------ data declarations for EM ---------------- */
#ifdef _SIMULATION_
#pragma message("this build includes engineering mode (FF_EM_MODE)")
#endif
/*
 *  To optimize the buffersize, the data is packed in the buffer as follows:
 *  type, length, value
 *  Value is the combination of the additional parameters as defined in 8443.601, coded as UBYTE.
 *  Buffersize is maximal size of one event
 */
#define EM_DL_BUFFER_SIZE             7


#define EM_DL_SEM_SIZE 30 /* Max. number off all data fall on to the event tracing */

/*
*   EM_MAX_DL_EVENTS defines maximum number of event traces for the engineering mode.
*   The number is increased by one to ensure that the event numbers defined in the
*   corresponding document are the same as in the sources.
*/
#define EM_MAX_DL_EVENTS             6

/*
*  The offset is used to indicate the source entity the event trace is from.
*  L1/ALR = 0x00, DL = 0x2D, RR = 0x37, MM = 0x5F, CC = 0x78, SS = 0xAF, SMS = 0xBE, SIM = E1
*/
#define DL_OFFSET                 0x2D

/*
*  Type is combination of entity index(upper nibble) plus event number(lower nibble).
*  Bit 8  7  6  5  4  3  2  1
*  |    entity    |  event number |
*/

#define DL_V_1             (1 + DL_OFFSET)
#define DL_V_2             (2 + DL_OFFSET)
#define DL_V_3             (3 + DL_OFFSET)
#define DL_V_4             (4 + DL_OFFSET)
#define DL_V_5             (5 + DL_OFFSET)

/* Event tracing flags for EM */
EXTERN BOOL dl_v[EM_MAX_DL_EVENTS];

/*---------Functions ---------*/

/*
 *  DL  primitives Engineering Mode
 *  Bitmask for the event tracing
 */
EXTERN void dl_em_dl_event_req      (T_EM_DL_EVENT_REQ *em_dl_event_req);

/*
 * If all entities are linked into one module this definitions
 * prefixes all this functions with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define em_write_buffer_4        _ENTITY_PREFIXED(em_write_buffer_4)
  #define em_write_buffer_5a       _ENTITY_PREFIXED(em_write_buffer_5a)
#if defined (TI_PS_HCOMM_CHANGE)
#else /* for hCommHandles backward compatibility */
  #define hCommMMI      _ENTITY_PREFIXED(hCommMMI)
#endif
#endif

#if defined (TI_PS_HCOMM_CHANGE)
#else /* for hCommHandles backward compatibility */
#if defined (NEW_FRAME)
  EXTERN T_HANDLE  hCommMMI;               /* EM  Communication        */
#else
 EXTERN T_VSI_CHANDLE  hCommMMI;          /* EM  Communication        */
#endif
#endif

EXTERN UBYTE em_write_buffer_4 (UBYTE event_no, UBYTE value1, UBYTE value2);
EXTERN UBYTE em_write_buffer_5a (UBYTE event_no, UBYTE value1, UBYTE value2, USHORT cs);

/*
 * Semaphor handling
 */
EXTERN void em_init_dl_event_trace (void);
EXTERN void em_dl_sem_init (void);
EXTERN void em_dl_sem_exit (void);
EXTERN void em_dl_sem_read (void);
EXTERN void em_dl_sem_reset (void);

/*-----*/

/*-----------------------Event Macro Definition -----------*/


#define DL_EM_CHANNEL_ESTABLISHMENT_PASSED\
    /* Channel establishment passed */\
  if (dl_v[1])\
  {\
    dl_v[1] = em_write_buffer_4 (DL_V_1, ch_type, sapi);\
  }  /* dl_v[1] */

#define DL_EM_CHANNEL_ESTABLISHMENT_FAILED\
     /* Channel establishment failed */\
    if (dl_v[2])\
    {\
      dl_v[2] = em_write_buffer_5a (DL_V_2, channel, sapi, NOT_PRESENT_16BIT);\
    }  /* dl_v[2] */

#define DL_EM_LINK_RELEASE\
   /* Link release */\
  if (dl_v[3])\
  {\
    dl_v[3] = em_write_buffer_4 (DL_V_3, ch_type, sapi);\
  }  /* dl_v[3] */

#define DL_EM_LINK_SUSPENDED\
      /* Link suspend */\
    if (dl_v[4])\
    {\
      dl_v[4] = em_write_buffer_4 (DL_V_4, suspend_req->ch_type, suspend_req->sapi);\
    }  /* dl_v[4] */

#define DL_EM_CHANNEL_FAILURE\
      /* Channel failure */\
    if (dl_v[5])\
    {\
      dl_v[5] = em_write_buffer_5a (DL_V_5, pcch->ch_type, pcch_i->sapi, CS_NR_SEQ_ERR);\
    }  /* dl_v[5] */

#else

#define DL_EM_CHANNEL_ESTABLISHMENT_PASSED    /*Event 1*/
#define DL_EM_CHANNEL_ESTABLISHMENT_FAILED    /*Event 2*/
#define DL_EM_LINK_RELEASE                    /*Event 3*/
#define DL_EM_LINK_SUSPENDED                  /*Event 4*/
#define DL_EM_CHANNEL_FAILURE                 /*Event 5*/

#endif /*FF_EM_MODE*/

#endif /* DL_EM_H */
