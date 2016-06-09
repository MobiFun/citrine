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
#ifndef SMS_EM_H
#define SMS_EM_H

#ifdef FF_EM_MODE

/* ----------------- data declarations for EM ------------- */
/*
 *  Ring buffer is by one bigger than max. number of prims, because it
 *  needs always one termination entry.
 *  To optimize the buffersize, the data is packed in the buffer as follows:
 *  type, length, value
 *  Value is the combination of the additional parameters as defined in 8443.601, coded as UBYTE.
*/
#define EM_SMS_BUFFER_SIZE       110

/*
*   EM_MAX_SMS_EVENTS defines maximum number of event traces for the engineering mode.
*   The number is increased by one to ensure that the event numbers defined in the
*   corresponding document are the same as in the sources.
*/
#define EM_MAX_SMS_EVENTS      34

/*
*  The offset is used to indicate the source entity the event trace is from.
*  L1/ALR = 0x00, DL = 0x2D, RR = 0x37, MM = 0x5F, CC = 0x78, SS = 0xAF, SMS = 0xBE, SIM = E1
*/
#define SMS_OFFSET             0xBE

/*
*  Type is combination of entity index(upper nibble) plus event number(lower nibble).
*  Bit 8  7  6  5  4  3  2  1
*  |   entity    |  event number |
*/
#define SMS_V_1              (1 + SMS_OFFSET)
#define SMS_V_2              (2 + SMS_OFFSET)
#define SMS_V_3              (3 + SMS_OFFSET)
#define SMS_V_4              (4 + SMS_OFFSET)
#define SMS_V_5              (5 + SMS_OFFSET)
#define SMS_V_6              (6 + SMS_OFFSET)
#define SMS_V_7              (7 + SMS_OFFSET)
#define SMS_V_8              (8 + SMS_OFFSET)
#define SMS_V_9              (9 + SMS_OFFSET)
#define SMS_V_10             (10+ SMS_OFFSET)
#define SMS_V_11             (11+ SMS_OFFSET)
#define SMS_V_12             (12+ SMS_OFFSET)
#define SMS_V_13             (13+ SMS_OFFSET)
#define SMS_V_14             (14+ SMS_OFFSET)
#define SMS_V_15             (15+ SMS_OFFSET)
#define SMS_V_16             (16+ SMS_OFFSET)
#define SMS_V_17             (17+ SMS_OFFSET)
#define SMS_V_18             (18+ SMS_OFFSET)
#define SMS_V_19             (19+ SMS_OFFSET)
#define SMS_V_20             (20+ SMS_OFFSET)
#define SMS_V_21             (21+ SMS_OFFSET)
#define SMS_V_22             (22+ SMS_OFFSET)
#define SMS_V_23             (23+ SMS_OFFSET)
#define SMS_V_24             (24+ SMS_OFFSET)
#define SMS_V_25             (25+ SMS_OFFSET)
#define SMS_V_26             (26+ SMS_OFFSET)
#define SMS_V_27             (27+ SMS_OFFSET)
#define SMS_V_28             (28+ SMS_OFFSET)
#define SMS_V_29             (29+ SMS_OFFSET)
#define SMS_V_30             (30+ SMS_OFFSET)
#define SMS_V_31             (31+ SMS_OFFSET)
#define SMS_V_32             (32+ SMS_OFFSET)
#define SMS_V_33             (33+ SMS_OFFSET)

EXTERN BOOL sms_v[EM_MAX_SMS_EVENTS];
EXTERN UBYTE em_change_flag;
EXTERN UBYTE em_sms_event_buffer[EM_SMS_BUFFER_SIZE];
EXTERN UBYTE em_sms_buffer_write;

/*-------------------Functions------------*/

EXTERN void sms_em_sms_event_req      (T_EM_SMS_EVENT_REQ *em_sms_event_req);

/*
 * If all entities are linked into one module this definitions
 * prefixes all this functions with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define em_write_buffer_2         _ENTITY_PREFIXED(em_write_buffer_2)
  #define em_write_buffer_3         _ENTITY_PREFIXED(em_write_buffer_3)
  #define em_write_buffer_3a        _ENTITY_PREFIXED(em_write_buffer_3a)
  #define em_write_buffer_4         _ENTITY_PREFIXED(em_write_buffer_4)
  #define em_write_buffer_4a        _ENTITY_PREFIXED(em_write_buffer_4a)
  #define em_write_buffer_5         _ENTITY_PREFIXED(em_write_buffer_5)
  #define em_write_buffer_6         _ENTITY_PREFIXED(em_write_buffer_6)
  #define check_write_index    _ENTITY_PREFIXED(check_write_index)
#endif

EXTERN UBYTE em_write_buffer_2       (UBYTE event_no);
EXTERN UBYTE em_write_buffer_3       (UBYTE event_no, UBYTE value);
EXTERN UBYTE em_write_buffer_3a      (UBYTE event_no, USHORT value);
EXTERN UBYTE em_write_buffer_4       (UBYTE event_no, UBYTE value1, UBYTE value2);
EXTERN UBYTE em_write_buffer_4a      (UBYTE event_no, UBYTE value,  USHORT cs);
EXTERN UBYTE em_write_buffer_5       (UBYTE event_no, UBYTE value1, UBYTE value2, UBYTE value3);
EXTERN UBYTE em_write_buffer_6       (UBYTE event_no, UBYTE value,  UBYTE value1, UBYTE value2, UBYTE value3);

EXTERN void em_init_sms_event_trace(void);


/*------------------- Macros --------------*/
#define SMS_EM_SET_CHANGE_FLAG_1\
      em_change_flag = 1;

#define SMS_EM_SET_CHANGE_FLAG_2\
      em_change_flag = 2;

#define SMS_EM_MO_SHORT_MESSAGE\
                    /* MO short message **/\
                  if (sms_v[1])\
                  {\
                    sms_v[1] = em_write_buffer_3 (SMS_V_1, SMS_TP_REF(sms_data));\
                  }  /* sms_v[1] */

#define SMS_EM_SEND_RP_DATA\
        /* SEND RP-Data*/\
        if (sms_v[2])\
        {\
           sms_v[2] = em_write_buffer_3 (SMS_V_2, msg_ref);\
        }  /* sms_v[2] */


#define SMS_EM_MM_CONNECTION_ESTABLISHMENT\
          /* MM connection establishment */\
          if (sms_v[3])\
          {\
           sms_v[3] = em_write_buffer_3 (SMS_V_3, SMS_INST.ti);\
          }  /* sms_v[3] */

#define SMS_EM_MM_CONNECTION_ESTABLISHED\
  /* MM connection established */\
  if (sms_v[4])\
  {\
   sms_v[4] = em_write_buffer_3 (SMS_V_4, establish_cnf->ti);\
  }  /* sms_v[4] */

#define SMS_EM_SEND_CP_DATA\
          /* Send CP-Data */\
        if (sms_v[5])\
        {\
         sms_v[5] = em_write_buffer_2 (SMS_V_5);\
        }  /* sms_v[5] */

#define SMS_EM_MM_CONNECTION_FAILED\
      /* MM connection failed */\
    if (sms_v[6])\
    {\
     sms_v[6] = em_write_buffer_3a(SMS_V_6, release_ind->cause);\
    }  /* sms_v[6] */

#define SMS_EM_TIMEOUT_TR1M\
  /* Timeout TR1M */\
  if (sms_v[7])\
  {\
    sms_v[7] = em_write_buffer_2 (SMS_V_7);\
  }  /* sms_v[7] */

#define SMS_EM_RECEIVE_CP_ACKNOWLEDGE\
            /* Receive CP-ACK */\
            if (sms_v[8])\
            {\
              sms_v[8] = em_write_buffer_3 (SMS_V_8, ti);\
            }  /* sms_v[8] */

#define SMS_EM_RECEIVE_CP_DATA\
            /* Receive CP-Data */\
            if (sms_v[9])\
            {\
              sms_v[9] = em_write_buffer_3 (SMS_V_9, ti);\
            }  /* sms_v[9] */

#define SMS_EM_RECEIVE_UNKNOWN\
              /* Receive unknown */\
            if (sms_v[10])\
            {\
               sms_v[10] = em_write_buffer_4 (SMS_V_10, ti, _decodedMsg[0]);\
            }  /* sms_v[10] */

#define SMS_EM_SEND_CP_ERROR\
        /* Send CP-Error */\
      if (sms_v[11])\
      {\
        sms_v[11] = em_write_buffer_2 (SMS_V_11);\
      }  /* sms_v[11] */

#define SMS_EM_RECEIVE_CP_ERROR\
            /* Receive CP-Error */\
            if (sms_v[12])\
            {\
              sms_v[12] = em_write_buffer_2 (SMS_V_12);\
            }  /* sms_v[12] */

#define SMS_EM_LOSS_OF_MM_CONNECTION\
    /* Loss of MM connection */\
    if (sms_v[13])\
    {\
     sms_v[13] = em_write_buffer_2 (SMS_V_13);\
    }  /* sms_v[13] */

#define SMS_EM_ABORT_OF_MM_CONNECTION\
    /* Abort of MM-Connection */\
  if (sms_v[14])\
  {\
    sms_v[14] = em_write_buffer_2 (SMS_V_14);\
  }  /* sms_v[14] */

#define SMS_EM_TIMEOUT_TC1M\
  /* Timeout TC1M */\
  if (sms_v[15])\
  {\
    sms_v[15] = em_write_buffer_2 (SMS_V_15);\
  }  /* sms_v[15] */

#define SMS_EM_TIMEOUT_TR2M\
    /* Timeout TR2M */\
  if (sms_v[16])\
  {\
   sms_v[16] = em_write_buffer_2 (SMS_V_16);\
  }  /* sms_v[16] */

#define SMS_EM_SEND_CP_ACKNOWLEDGE\
        /* Send CP ACK */\
      if (sms_v[17])\
      {\
        sms_v[17] = em_write_buffer_2 (SMS_V_17);\
      }  /* sms_v[17] */

#define SMS_EM_RECEIVE_RP_ERROR\
            /* Receive RP-Error */\
            if (sms_v[18])\
            {\
              sms_v[18] = em_write_buffer_4 (SMS_V_18, cp_user_data_dl->reference,\
                            cp_user_data_dl->rp_error.rp_cause.rp_cause_value);\
            }  /* sms_v[18] */

#define SMS_EM_RECEIVE_RP_AKNOWLEDGE\
            /* RECEIVE RP-Akn */\
            if (sms_v[19])\
            {\
              sms_v[19] = em_write_buffer_3 (SMS_V_19, cp_user_data_dl->reference);\
            }  /* sms_v[19] */

#define SMS_EM_RECEIVE_RP_DATA\
          /* Receive RP-DATA */\
          if (sms_v[20])\
          {\
            sms_v[20] = em_write_buffer_3 (SMS_V_20, cp_user_data_dl->reference);\
          }  /* sms_v[20] */

#define SMS_EM_RECEIVE_UNKNOWN_2\
              /* Receive Unknown */\
            if (sms_v[21])\
            {\
              sms_v[21] = em_write_buffer_4 (SMS_V_21, cp_user_data_dl->reference,\
                                        cp_user_data_dl->rp_mti);\
            }  /* sms_v[21] */

#define SMS_EM_SEND_RP_ERROR\
          /* Send RP-Error */\
          if (sms_v[22])\
          {\
            sms_v[22] = em_write_buffer_4 (SMS_V_22, SMS_RP_REF(sms_data),\
                                      SMS_RP_CS_MSG_NOT_COMP);\
          }  /* sms_v[22] */

#define SMS_EM_MO_SHORT_MESSAGE_COMMAND\
            /* MO short message */\
            if (sms_v[23])\
            {\
              sms_v[23] = em_write_buffer_4 (SMS_V_23, SMS_TP_REF(sms_data),\
                                        sim_pdu->tpdu.b_tpdu[4]); /* TP-MN */\
            }  /* sms_v[23] */

#define SMS_EM_DISPLAY_MT_SHORT_MESSAGE\
      /* Display MT short message */\
      if (sms_v[24])\
      {\
        sms_v[24] = em_write_buffer_4 (SMS_V_24, sms_deliver->tp_pid,\
                                  sms_deliver->tp_dcs);\
      }  /* sms_v[24] */

#define SMS_EM_STORE_MT_MESSAGE\
    if (record NEQ SMS_RECORD_NOT_EXIST)\
    {\
      /* Store MT message in ME/SIM */\
      if (sms_v[25])\
      {\
        sms_v[25] = em_write_buffer_4 (SMS_V_25, mem_type, record);\
      }  /* sms_v[25] */\
    }

#define SMS_EM_REPLACE_SMS_IN_ME\
          /* Replace SMS in ME */\
          if (sms_v[26])\
          {\
            sms_v[26] = em_write_buffer_6 (SMS_V_26, MEM_ME, (UBYTE)i,\
                                      sms_deliver->tp_pid,\
                                      sms_deliver->tp_dcs);\
          }  /* sms_v[26] */

#define SMS_EM_REPLACE_SMS_IN_SIM\
          /* Replace SMS in SIM */\
          if (sms_v[26])\
          {\
            sms_v[26] = em_write_buffer_6 (SMS_V_26, MEM_SM, record,\
                                      SMS_PID(sms_data), SMS_DCS(sms_data));\
          }  /* sms_v[26] */

#define SMS_EM_SMS_STATUS_MESSAGE\
        /* SMS status message  */\
        if (sms_v[27])\
        {\
          sms_v[27] = em_write_buffer_2 (SMS_V_27);\
        }  /* sms_v[27] */

#define SMS_EM_RECEIVE_SIM_TOOLKIT_DATA_DOWNLOAD\
        /* Receive SIM toolkit data download */\
        if (sms_v[28])\
        {\
          sms_v[28] = em_write_buffer_4 (SMS_V_28, SMS_DCS(sms_data), SMS_PID(sms_data));\
        }  /* sms_v[28] */

#define SMS_EM_STORE_MO_SHORT_MESSAGE\
            if (em_change_flag == 1) /* new sms */\
            {\
              /* Store SMS */\
              if (sms_v[29])\
              {\
                sms_v[29] = em_write_buffer_4 (SMS_V_29, MEM_ME, (UBYTE)index);\
              }  /* sms_v[29] */\
            }

#define SMS_EM_STORE_MO_SHORT_MESSAGE_2\
        if (em_change_flag == 1)      /* new sms */\
        {\
          /* Store SMS */\
          if (sms_v[29])\
          {\
            sms_v[29] = em_write_buffer_4 (SMS_V_29, MEM_SM, record);\
          }  /* sms_v[29] */\
        }

#define SMS_EM_CHANGE_SHORT_MESSAGE\
  if (em_change_flag == 2)  /* change  sms */\
  {\
  /* Store SMS */\
    if (sms_v[30])\
    {\
      sms_v[30] = em_write_buffer_4 (SMS_V_30, MEM_ME, (UBYTE)index);\
    }  /* sms_v[30] */\
  }

#define SMS_EM_CHANGE_SHORT_MESSAGE_2\
  if (em_change_flag == 2)   /* change  sms */\
  {\
  /* Store SMS */\
    if (sms_v[30])\
    {\
      sms_v[30] = em_write_buffer_4 (SMS_V_30, MEM_SM, record);\
    }  /* sms_v[30] */\
  }

#define SMS_EM_READ_SHORT_MESSAGE\
          /* Read short message */\
        if (sms_v[31])\
        {\
          sms_v[31] = em_write_buffer_4 (SMS_V_31, MEM_ME, (UBYTE)index);\
        }  /* sms_v[31] */

#define SMS_EM_READ_SHORT_MESSAGE_2\
            /* Read short message - MO SMS*/\
            if (sms_v[31])\
            {\
              sms_v[31] = em_write_buffer_4 (SMS_V_31, MEM_SM, record);\
            }  /* sms_v[31] */

#define SMS_EM_DELETE_SHORT_MESSAGE\
    if (error EQ SIM_NO_ERROR)\
    {\
    /* Delete short message */\
      if (sms_v[32])\
      {\
        sms_v[32] = em_write_buffer_4 (SMS_V_32, mem_type, record);\
      }  /* sms_v[32] */\
    }

#define SMS_EM_UNKNOWN_TRANSACTION\
          /* Unknown Transaction */\
        if (sms_v[33])\
        {\
         sms_v[33] = em_write_buffer_5 (SMS_V_33, ti, _decodedMsg[0], pd);\
        }  /* sms_v[33] */

#else /*FF_EM_MODE */

#define SMS_EM_SET_CHANGE_FLAG_1
#define SMS_EM_SET_CHANGE_FLAG_2

#define SMS_EM_MO_SHORT_MESSAGE               /*Event  1*/
#define SMS_EM_SEND_RP_DATA                   /*Event  2*/
#define SMS_EM_MM_CONNECTION_ESTABLISHMENT    /*Event  3*/
#define SMS_EM_MM_CONNECTION_ESTABLISHED      /*Event  4*/
#define SMS_EM_SEND_CP_DATA                   /*Event  5*/
#define SMS_EM_MM_CONNECTION_FAILED           /*Event  6*/
#define SMS_EM_TIMEOUT_TR1M                   /*Event  7*/
#define SMS_EM_RECEIVE_CP_ACKNOWLEDGE         /*Event  8*/
#define SMS_EM_RECEIVE_CP_DATA                /*Event  9*/
#define SMS_EM_RECEIVE_UNKNOWN                /*Event 10*/
#define SMS_EM_SEND_CP_ERROR                  /*Event 11*/
#define SMS_EM_RECEIVE_CP_ERROR               /*Event 12*/
#define SMS_EM_LOSS_OF_MM_CONNECTION          /*Event 13*/
#define SMS_EM_ABORT_OF_MM_CONNECTION         /*Event 14*/
#define SMS_EM_TIMEOUT_TC1M                   /*Event 15*/
#define SMS_EM_TIMEOUT_TR2M                   /*Event 16*/
#define SMS_EM_SEND_CP_ACKNOWLEDGE            /*Event 17*/
#define SMS_EM_RECEIVE_RP_ERROR               /*Event 18*/
#define SMS_EM_RECEIVE_RP_AKNOWLEDGE          /*Event 19*/
#define SMS_EM_RECEIVE_RP_DATA                /*Event 20*/
#define SMS_EM_RECEIVE_UNKNOWN_2              /*Event 21*/
#define SMS_EM_SEND_RP_ERROR                  /*Event 22*/
#define SMS_EM_MO_SHORT_MESSAGE_COMMAND       /*Event 23*/
#define SMS_EM_DISPLAY_MT_SHORT_MESSAGE       /*Event 24*/
#define SMS_EM_STORE_MT_MESSAGE               /*Event 25*/
#define SMS_EM_REPLACE_SMS_IN_ME              /*Event 26*/
#define SMS_EM_REPLACE_SMS_IN_SIM             /*Event 26*/
#define SMS_EM_SMS_STATUS_MESSAGE             /*Event 27*/
#define SMS_EM_RECEIVE_SIM_TOOLKIT_DATA_DOWNLOAD /*Event 28*/
#define SMS_EM_STORE_MO_SHORT_MESSAGE         /*Event 29*/
#define SMS_EM_STORE_MO_SHORT_MESSAGE_2       /*Event 29*/
#define SMS_EM_CHANGE_SHORT_MESSAGE           /*Event 30*/
#define SMS_EM_CHANGE_SHORT_MESSAGE_2         /*Event 30*/
#define SMS_EM_READ_SHORT_MESSAGE             /*Event 31*/
#define SMS_EM_READ_SHORT_MESSAGE_2           /*Event 31*/
#define SMS_EM_DELETE_SHORT_MESSAGE           /*Event 32*/
#define SMS_EM_UNKNOWN_TRANSACTION            /*Event 33*/


#endif  /*FF_EM_MODE */
#endif  /* SMS_EM_H  */
