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
#ifndef SS_EM_H
#define SS_EM_H

#ifdef FF_EM_MODE


/*
 *  Ring buffer is by one bigger than max. number of prims, because it
 *  needs always one termination entry.
 *  To optimize the buffersize, the data is packed in the buffer as follows:
 *  type, length, value
 *  Value is the combination of the additional parameters as defined in 8443.601, coded as UBYTE.
 */
#define EM_SS_BUFFER_SIZE             31

/*
*   EM_MAX_SS_EVENTS defines maximum number of event traces for the engineering mode.
*   The number is increased by one to ensure that the event numbers defined in the
*   corresponding document are the same as in the sources.
*/
#define EM_MAX_SS_EVENTS            11

/*
*  The offset is used to indicate the source entity the event trace is from. 
*  L1/ALR = 0x00, DL = 0x2D, RR = 0x37, MM = 0x5F, CC = 0x78, SS = 0xAF, SMS = 0xBE, SIM = E1
*/
#define SS_OFFSET                   0xAF
 
/*
*  Type is combination of entity index(upper nibble) plus event number(lower nibble). 
*  Bit 8  7  6  5  4  3  2  1
*  |    entity    |  event number |  
*/
#define SS_V_1            (1 + SS_OFFSET)
#define SS_V_2            (2 + SS_OFFSET)
#define SS_V_3            (3 + SS_OFFSET)
#define SS_V_4            (4 + SS_OFFSET)
#define SS_V_5            (5 + SS_OFFSET)
#define SS_V_6            (6 + SS_OFFSET)
#define SS_V_7            (7 + SS_OFFSET)
#define SS_V_8            (8 + SS_OFFSET)
#define SS_V_9            (9 + SS_OFFSET)
#define SS_V_10           (10+ SS_OFFSET)

/* Event tracing flags for EM */
EXTERN BOOL ss_v[EM_MAX_SS_EVENTS];


/* -- Functions ----*/
/*
 *  suppl. services primitives Engineering Mode
 *  Bitmask for the event tracing
 */

EXTERN void ss_em_ss_event_req      (T_EM_SS_EVENT_REQ *em_ss_event_req);
GLOBAL void em_init_ss_event_trace  (void);

/*
 * If all entities are linked into one module this definitions
 * prefixes all this functions with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define em_write_buffer_3          _ENTITY_PREFIXED(em_write_buffer_3)  
  #define check_write_index     _ENTITY_PREFIXED(check_write_index)
#endif

EXTERN UBYTE em_write_buffer_3      (UBYTE event_no, UBYTE value);
EXTERN UBYTE check_write_index (UBYTE n);

/*---------------Event Macros ----------*/

#define MM_EM_MM_CONNECTION_STARTED\
           /* MM connection started */\
          if (ss_v[1])\
          {\
           ss_v[1]  = em_write_buffer_3 (SS_V_1 , ss_data->ti);\
          }  /* ss_v[1]  */

#define MM_EM_MM_CONNECTION_ESTABLISHED\
          /* MM connection established */\
        if (ss_v[2])\
        {\
         ss_v[2] = em_write_buffer_3 (SS_V_2 , ss_data->ti);\
        }  /* ss_v[2]  */

#define MM_EM_MM_CONNECTION_FAILED\
        /* MM connection failed */\
      if (ss_v[3])\
      {\
       ss_v[3] = em_write_buffer_3 (SS_V_3 , ss_data->ti);\
      }  /* ss_v[3]  */

#define MM_EM_MM_CONNECTION_ABORTED\
      /* MM connection aborted */\
      if (ss_v[4])\
      {\
       ss_v[4] = em_write_buffer_3 (SS_V_4 , ss_data->ti);\
      } /* ss_v[4]  */

#define MM_EM_MM_CONNECTION_RELEASED\
        /* MM connection released */\
      if (ss_v[5])\
      {\
       ss_v[5] = em_write_buffer_3 (SS_V_5 , ss_data->ti);\
      }  /* ss_v[5]  */

#define MM_EM_REGISTER_MESSAGE_RECEIVED\
          /* Register message received */\
        if (ss_v[6])\
        {\
          ss_v[6] = em_write_buffer_3 (SS_V_6 , ss_data->ti);\
        } /* ss_v[6] */


#define MM_EM_FACILITY_MESSAGE_SENT\
          /* Facility message send */\
        if (ss_v[7])\
        {\
         ss_v[7] = em_write_buffer_3 (SS_V_7 , ss_data->ti);\
        }  /* ss_v[7]  */

#define MM_EM_FACILITY_MESSAGE_RECEIVED\
            /* Facility message received */\
          if (ss_v[8])\
          {\
           ss_v[8] = em_write_buffer_3 (SS_V_8 , ss_data->ti);\
          }  /* ss_v[8]  */

#define MM_EM_MM_RELEASE_COMPLETE_SENT\
                /* Release complete sent */\
      if (ss_v[9])\
      {\
       ss_v[9] = em_write_buffer_3 (SS_V_9 , ss_data->ti);\
      }  /* ss_v[9] */

#define MM_EM_MM_RELEASE_COMPLETE_RECEIVED\
            /* Release complete received */\
          if (ss_v[10])\
          {\
           ss_v[10] = em_write_buffer_3 (SS_V_10, ss_data->ti);\
          }  /* ss_v[10] */

#else /*FF_EM_MODE not defined*/

#define MM_EM_MM_CONNECTION_STARTED         /*Event  1*/
#define MM_EM_MM_CONNECTION_ESTABLISHED     /*Event  2*/
#define MM_EM_MM_CONNECTION_FAILED          /*Event  3*/
#define MM_EM_MM_CONNECTION_ABORTED         /*Event  4*/
#define MM_EM_MM_CONNECTION_RELEASED        /*Event  5*/
#define MM_EM_REGISTER_MESSAGE_RECEIVED     /*Event  6*/
#define MM_EM_FACILITY_MESSAGE_SENT         /*Event  7*/
#define MM_EM_FACILITY_MESSAGE_RECEIVED     /*Event  8*/
#define MM_EM_MM_RELEASE_COMPLETE_SENT      /*Event  9*/
#define MM_EM_MM_RELEASE_COMPLETE_RECEIVED  /*Event 10*/


#endif /*FF_EM_MODE*/
#endif /* SS_EM_H */
