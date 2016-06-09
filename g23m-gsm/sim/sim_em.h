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
#ifndef SIM_EM_H
#define SIM_EM_H

#ifdef FF_EM_MODE

/* ------------ data declarations for EM ---------------- */
/*
 *  Ring buffer is by one bigger than max. number of prims, because it
 *  needs always one termination entry.
 *  To optimize the buffersize, the data is packed in the buffer as follows:
 *  type, length, value
 *  Value is the combination of the additional parameters as defined in 8443.601, coded as UBYTE.
*/
#define EM_SIM_BUFFER_SIZE            67

/*
*   EM_MAX_SIM_EVENTS defines maximum number of event traces for the engineering mode.
*   The number is increased by one to ensure that the event numbers defined in the
*   corresponding document are the same as in the sources.
*/
#define EM_MAX_SIM_EVENTS           26

/*
*  The offset is used to indicate the source entity the event trace is from.
*  L1/ALR = 0x00, DL = 0x2D, RR = 0x37, MM = 0x5F, CC = 0x78, SS = 0xAF, SMS = 0xBE, SIM = E1
*/
#define SIM_OFFSET                0xE1

/*
*  Type is combination of entity index(upper nibble) plus event number(lower nibble).
*  Bit 8  7  6  5  4  3  2  1
*  |    entity    |  event number |
*/

#define SIM_V_1              (1 + SIM_OFFSET)
#define SIM_V_2              (2 + SIM_OFFSET)
#define SIM_V_3              (3 + SIM_OFFSET)
#define SIM_V_4              (4 + SIM_OFFSET)
#define SIM_V_5              (5 + SIM_OFFSET)
#define SIM_V_6              (6 + SIM_OFFSET)
#define SIM_V_7              (7 + SIM_OFFSET)
#define SIM_V_8              (8 + SIM_OFFSET)
#define SIM_V_9              (9 + SIM_OFFSET)
#define SIM_V_10             (10+ SIM_OFFSET)
#define SIM_V_11             (11+ SIM_OFFSET)
#define SIM_V_12             (12+ SIM_OFFSET)
#define SIM_V_13             (13+ SIM_OFFSET)
#define SIM_V_14             (14+ SIM_OFFSET)
#define SIM_V_15             (15+ SIM_OFFSET)
#define SIM_V_16             (16+ SIM_OFFSET)
#define SIM_V_17             (17+ SIM_OFFSET)
#define SIM_V_18             (18+ SIM_OFFSET)
#define SIM_V_19             (19+ SIM_OFFSET)
#define SIM_V_20             (20+ SIM_OFFSET)
#define SIM_V_21             (21+ SIM_OFFSET)
#define SIM_V_22             (22+ SIM_OFFSET)
#define SIM_V_23             (23+ SIM_OFFSET)
#define SIM_V_24             (24+ SIM_OFFSET)
#define SIM_V_25             (25+ SIM_OFFSET)


/* Event tracing flags for EM */
EXTERN BOOL sim_v[EM_MAX_SIM_EVENTS];

/*---------Functions ---------*/

/*
 *  SIM primitives Engineering Mode
 *  Bitmask for the event tracing
 */

EXTERN void sim_em_sim_event_req      (T_EM_SIM_EVENT_REQ *em_sim_event_req);
EXTERN void em_init_sim_event_trace   (void);

/*
 * If all entities are linked into one module this definitions
 * prefixes all this functions with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define em_write_buffer_2        _ENTITY_PREFIXED(em_write_buffer_2)
  #define em_write_buffer_3        _ENTITY_PREFIXED(em_write_buffer_3)
  #define em_write_buffer_3a       _ENTITY_PREFIXED(em_write_buffer_3a)
  #define check_write_index   _ENTITY_PREFIXED(check_write_index)
#endif

EXTERN UBYTE em_write_buffer_2       (UBYTE event_no);
EXTERN UBYTE em_write_buffer_3       (UBYTE event_no, UBYTE value);
EXTERN UBYTE em_write_buffer_3a      (UBYTE event_no, USHORT value);
static UBYTE check_write_index  (UBYTE n);

/*-----------------------Event Macro Definition -----------*/

#define SIM_EM_READ_BINARY_FILE\
  /* Read binary file */\
  if (sim_v[1])\
  {\
    sim_v[1] = em_write_buffer_3a (SIM_V_1, sim_read_req->datafield);\
  }  /* sim_v[1] */

#define SIM_EM_READ_RECORD_FILE\
    /* Read record file */\
  if (sim_v[2])\
  {\
    sim_v[2] = em_write_buffer_3a (SIM_V_2, sim_read_record_req->datafield);\
  }  /* sim_v[2] */

#define SIM_EM_UPDATE_BINARY_FILE\
 /* Update binary file */\
  if (sim_v[3])\
  {\
    sim_v[3] = em_write_buffer_3a (SIM_V_3, sim_update_req->datafield);\
  }  /* sim_v[3] */

#define SIM_EM_UPDATE_RECORD\
 /* Update record */\
  if (sim_v[4])\
  {\
    sim_v[4] = em_write_buffer_3a (SIM_V_4, sim_update_record_req->datafield);\
  }  /* sim_v[4] */

#define SIM_EM_INCREMENT_FILE\
    /* Increment file */\
  if (sim_v[5])\
  {\
    sim_v[5] = em_write_buffer_3a (SIM_V_5, sim_increment_req->datafield);\
  }  /* sim_v[5] */

#define SIM_EM_VERIFY_PIN\
    /* Verify PIN */\
  if (sim_v[6])\
  {\
    sim_v[6] = em_write_buffer_3 (SIM_V_6, sim_verify_pin_req->pin_id);\
  }  /* sim_v[6] */

#define SIM_EM_CHANGE_PIN\
   /* Change PIN */\
  if (sim_v[7])\
  {\
    sim_v[7] = em_write_buffer_3 (SIM_V_7, sim_change_pin_req->pin_id);\
  }  /* sim_v[7] */

#define SIM_EM_DISABLE_PIN\
   /* Disable PIN */\
  if (sim_v[8])\
  {\
    sim_v[8] = em_write_buffer_3 (SIM_V_8, NOT_PRESENT_8BIT);\
  }  /* sim_v[8] */

#define SIM_EM_ENABLE_PIN\
   /* Enable PIN */\
  if (sim_v[9])\
  {\
    sim_v[9] = em_write_buffer_3 (SIM_V_9, NOT_PRESENT_8BIT);\
  }  /* sim_v[9] */

#define SIM_EM_UNBLOCK_PIN\
    /* Unblock PIN */\
  if (sim_v[10])\
  {\
    sim_v[10] = em_write_buffer_3 (SIM_V_10, sim_unblock_req->pin_id);\
  }  /* sim_v[10] */

#define SIM_EM_AUTHENTICATION\
   /* Authentication */\
  if (sim_v[11])\
  {\
    sim_v[11] = em_write_buffer_2 (SIM_V_11);\
  }  /* sim_v[11] */

#define SIM_EM_SIM_ACTIVATION_STARTED\
        /* SIM activation started */\
      if (sim_v[12])\
      {\
        sim_v[12] = em_write_buffer_2 (SIM_V_12);\
      }  /* sim_v[12] */

#define SIM_EM_SIM_ACTIVATION_RESULT\
       /* SIM activation result */\
        if (sim_v[13])\
        {\
          sim_v[13] = em_write_buffer_3a (SIM_V_13, sim_activate_cnf->cause);\
        }  /* sim_v[13] */

#define SIM_EM_READ_MMI_PARAMETER\
   /* Read MMI parameters */\
  if (sim_v[14])\
  {\
    sim_v[14] = em_write_buffer_2 (SIM_V_14);\
  }  /* sim_v[14] */

#define SIM_EM_READ_MM_PARAMETER\
 /* Read MM parameters */\
  if (sim_v[15])\
  {\
    sim_v[15] = em_write_buffer_2 (SIM_V_15);\
  }  /* sim_v[15] */

#define SIM_EM_READ_SMS_PARAMETER\
   /* Read SMS parameters */\
  if (sim_v[16])\
  {\
    sim_v[16] = em_write_buffer_2 (SIM_V_16);\
  }  /* sim_v[16] */

#define SIM_EM_SIM_REMOVE\
     /* SIM removing */\
    if (sim_v[17])\
    {\
      sim_v[17] = em_write_buffer_2 (SIM_V_17);\
    }  /* sim_v[17] */

#define SIM_EM_PARAMETER_UPDATE\
   /* Parameter update */\
  if (sim_v[18])\
  {\
    sim_v[18] = em_write_buffer_2 (SIM_V_18);\
  }  /* sim_v[18] */

#define SIM_EM_PARAMETER_SYNCHRONISATION\
   /* Parameter synchronisation */\
  if (sim_v[19])\
  {\
    sim_v[19] = em_write_buffer_2 (SIM_V_19);\
  }  /* sim_v[19] */

#define SIM_EM_FDN_ENABLE\
              /* FDN enable */\
            if (sim_v[20])\
            {\
              sim_v[20] = em_write_buffer_2 (SIM_V_20);\
            }  /* sim_v[20] */

#define SIM_EM_ADN_ENABLE\
             /* ADN enable */\
            if (sim_v[21])\
            {\
              sim_v[21] = em_write_buffer_2 (SIM_V_21);\
            }  /* sim_v[21] */

#define SIM_EM_SIM_TOOLKIT_ACTIVATION\
       /* SIM Toolkit activation */\
      if (sim_v[22])\
      {\
        sim_v[22] = em_write_buffer_2 (SIM_V_22);\
      }  /* sim_v[22] */

#define SIM_EM_PROACTIVE_COMMAND\
              /* Proactive cmd */\
            if (sim_v[23])\
            {\
              sim_v[23] = em_write_buffer_3 (SIM_V_23, cmd_type);\
            }  /* sim_v[23] */

#define SIM_EM_TERMINAL_RESPONSE\
  /* Terminal Response */\
  if (sim_v[24])\
  {\
    sim_v[24] = em_write_buffer_3a (SIM_V_24, sw1sw2);\
  }  /* sim_v[24] */

#define SIM_EM_ENVELOPE\
 /* Envelope */\
  if (sim_v[24])\
  {\
    sim_v[24] = em_write_buffer_3a (SIM_V_24, sw1sw2);\
  }  /* sim_v[24] */

#else /*FF_EM_MODE not defined*/

#define SIM_EM_READ_BINARY_FILE           /*Event  1*/
#define SIM_EM_READ_RECORD_FILE           /*Event  2*/
#define SIM_EM_UPDATE_BINARY_FILE         /*Event  3*/
#define SIM_EM_UPDATE_RECORD              /*Event  4*/
#define SIM_EM_INCREMENT_FILE             /*Event  5*/
#define SIM_EM_VERIFY_PIN                 /*Event  6*/
#define SIM_EM_CHANGE_PIN                 /*Event  7*/
#define SIM_EM_DISABLE_PIN                /*Event  8*/
#define SIM_EM_ENABLE_PIN                 /*Event  9*/
#define SIM_EM_UNBLOCK_PIN                /*Event 10*/
#define SIM_EM_AUTHENTICATION             /*Event 11*/ 
#define SIM_EM_SIM_ACTIVATION_STARTED     /*Event 12*/
#define SIM_EM_SIM_ACTIVATION_RESULT      /*Event 13*/
#define SIM_EM_READ_MMI_PARAMETER         /*Event 14*/
#define SIM_EM_READ_MM_PARAMETER          /*Event 15*/
#define SIM_EM_READ_SMS_PARAMETER         /*Event 16*/
#define SIM_EM_SIM_REMOVE                 /*Event 17*/
#define SIM_EM_PARAMETER_UPDATE           /*Event 18*/
#define SIM_EM_PARAMETER_SYNCHRONISATION  /*Event 19*/
#define SIM_EM_FDN_ENABLE                 /*Event 20*/
#define SIM_EM_ADN_ENABLE                 /*Event 21*/
#define SIM_EM_SIM_TOOLKIT_ACTIVATION     /*Event 22*/
#define SIM_EM_PROACTIVE_COMMAND          /*Event 23*/
#define SIM_EM_TERMINAL_RESPONSE          /*Event 24*/
#define SIM_EM_ENVELOPE                   /*Event 25*/

#endif /*FF_EM_MODE*/
#endif /* SIM_EM_H */
