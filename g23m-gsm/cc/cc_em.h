/*
+-----------------------------------------------------------------------------
|  Project :
|  Modul   :
+-----------------------------------------------------------------------------
|  Copyright 2003 Texas Instruments Berlin, AG
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

#ifndef CC_EM_H
#define CC_EM_H

#ifdef FF_EM_MODE
/* ---------------- data declarations for EM ----------------*/
/*
 *  Ring buffer is by one bigger than max. number of prims, because it
 *  needs always one termination entry.
 *  To optimize the buffersize, the data is packed in the buffer as follows:
 *  type, length, value
 *  Value is the combination of the additional parameters as defined in 8443.601, coded as UBYTE.
 */
#define EM_CC_BUFFER_SIZE        185

/*
*   EM_MAX_CC_EVENTS defines maximum number of event traces for the engineering mode.
*   The number is increased by one to ensure that the event numbers defined in the
*   corresponding document are the same as in the sources.
*/
#define EM_MAX_CC_EVENTS        50

/*
*  The offset is used to indicate the source entity the event trace is from. 
*  L1/ALR = 0x00, DL = 0x2D, RR = 0x37, MM = 0x5F, CC = 0x78, SS = 0xAF, SMS = 0xBE, SIM = E1
*/
#define CC_OFFSET              0x78

/*
*  Type is combination of entity index(upper nibble) plus event number(lower nibble). 
*  Bit  8   7   6   5   4   3   2   1
*      |    entity    |  event number |
*/

#define CC_V_1                  (1 + CC_OFFSET)
#define CC_V_2                  (2 + CC_OFFSET)
#define CC_V_3                  (3 + CC_OFFSET)
#define CC_V_4                  (4 + CC_OFFSET)
#define CC_V_5                  (5 + CC_OFFSET)
#define CC_V_6                  (6 + CC_OFFSET)
#define CC_V_7                  (7 + CC_OFFSET)
#define CC_V_8                  (8 + CC_OFFSET)
#define CC_V_9                  (9 + CC_OFFSET)
#define CC_V_10                 (10+ CC_OFFSET)
#define CC_V_11                 (11+ CC_OFFSET)
#define CC_V_12                 (12+ CC_OFFSET)
#define CC_V_13                 (13+ CC_OFFSET)
#define CC_V_14                 (14+ CC_OFFSET)
#define CC_V_15                 (15+ CC_OFFSET)
#define CC_V_16                 (16+ CC_OFFSET)
#define CC_V_17                 (17+ CC_OFFSET)
#define CC_V_18                 (18+ CC_OFFSET)
#define CC_V_19                 (19+ CC_OFFSET)
#define CC_V_20                 (20+ CC_OFFSET)
#define CC_V_21                 (21+ CC_OFFSET)
#define CC_V_22                 (22+ CC_OFFSET)
#define CC_V_23                 (23+ CC_OFFSET)
#define CC_V_24                 (24+ CC_OFFSET)
#define CC_V_25                 (25+ CC_OFFSET)
#define CC_V_26                 (26+ CC_OFFSET)
#define CC_V_27                 (27+ CC_OFFSET)
#define CC_V_28                 (28+ CC_OFFSET)
#define CC_V_29                 (29+ CC_OFFSET)
#define CC_V_30                 (30+ CC_OFFSET)
#define CC_V_31                 (31+ CC_OFFSET)
#define CC_V_32                 (32+ CC_OFFSET)
#define CC_V_33                 (33+ CC_OFFSET)
#define CC_V_34                 (34+ CC_OFFSET)
#define CC_V_35                 (35+ CC_OFFSET)
#define CC_V_36                 (36+ CC_OFFSET)
#define CC_V_37                 (37+ CC_OFFSET)
#define CC_V_38                 (38+ CC_OFFSET)
#define CC_V_39                 (39+ CC_OFFSET)
#define CC_V_40                 (40+ CC_OFFSET)
#define CC_V_41                 (41+ CC_OFFSET)
#define CC_V_42                 (42+ CC_OFFSET)
#define CC_V_43                 (43+ CC_OFFSET)
#define CC_V_44                 (44+ CC_OFFSET)
#define CC_V_45                 (45+ CC_OFFSET)
#define CC_V_46                 (46+ CC_OFFSET)
#define CC_V_47                 (47+ CC_OFFSET)
#define CC_V_48                 (48+ CC_OFFSET)
#define CC_V_49                 (49+ CC_OFFSET)

 EXTERN BOOL cc_v[EM_MAX_CC_EVENTS];

 /*------functions ------*/

 /*
 *  Call control primitives Engineering Mode
 *  Bitmask for the event tracing
 */

EXTERN void cc_em_cc_event_req      (T_EM_CC_EVENT_REQ *em_cc_event_req);
EXTERN void em_init_cc_event_trace(void);

/*
 * If all entities are linked into one module these definitions
 * prefix all these functions with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define em_write_buffer_2          _ENTITY_PREFIXED(em_write_buffer_2)
  #define em_write_buffer_3          _ENTITY_PREFIXED(em_write_buffer_3)
  #define em_write_buffer_3a         _ENTITY_PREFIXED(em_write_buffer_3a)
  #define em_write_buffer_4          _ENTITY_PREFIXED(em_write_buffer_4)
  #define em_write_buffer_4a          _ENTITY_PREFIXED(em_write_buffer_4a)
  #define check_write_index     _ENTITY_PREFIXED(check_write_index)
#endif

EXTERN UBYTE em_write_buffer_2           (UBYTE event_no);
EXTERN UBYTE em_write_buffer_3           (UBYTE event_no, UBYTE value);
EXTERN UBYTE em_write_buffer_3a          (UBYTE event_no, USHORT value);
EXTERN UBYTE em_write_buffer_4           (UBYTE event_no, UBYTE value1, UBYTE value2);
EXTERN UBYTE em_write_buffer_4a          (UBYTE event_no, UBYTE *ptr1, UBYTE value2); 
EXTERN UBYTE check_write_index      (UBYTE n);


EXTERN UBYTE em_cc_event_buffer[EM_CC_BUFFER_SIZE];
EXTERN UBYTE em_cc_buffer_write;

/* ---------------- Macro definitions ---------------------- */
/* Please note: All called functions are stored in em_cc.c */

#define EM_CC_START_MO_CALL \
   /* Start MO call */\
  if (cc_v[1])\
  {\
        cc_v[1] = em_write_buffer_3 (CC_V_1, mmcm_establish_req->ti);\
  }  /* cc_v[1] */

#define EM_CC_MM_CONNECTION_ESTABLISHED_MT \
               /* MM connection established - MT */\
              if (cc_v[2])\
              {\
                cc_v[2] = em_write_buffer_3 (CC_V_2, cc_data->index_ti);\
              } /* cc_v[2] */

#define EM_CC_MM_CONNECTION_ESTABLISHED_MO \
   /* MM connection established - MO */\
  if (cc_v[2])\
  {\
    cc_v[2] = em_write_buffer_3 (CC_V_2, est_cnf->ti);\
  }  /* cc_v[2] */

#define EM_CC_MM_CONNECTION_FAILED \
   /* MM-Connection failed */\
  if (cc_v[3])\
  {\
    cc_v[3] = em_write_buffer_3a (CC_V_3, err_ind->cause);\
  }  /* cc_v[3] */

#define EM_CC_SENT_SETUP_MESSAGE \
      /* Sent SETUP message */\
    if (cc_v[4])\
    {\
      cc_v[4] = em_write_buffer_4a ( CC_V_4, &data->sdu.buf[CC_ENCODE_OFFSET >> 3], cc_data->ti);\
    } /* cc_v[4] */

#define EM_CC_SENT_EMERGENCY_SETUP_MESSAGE \
     /* Sent emergency setup message */\
    if (cc_v[5])\
    {\
      cc_v[5] = em_write_buffer_3 (CC_V_5, cc_data->ti);\
    } /* cc_v[5] */

#define EM_CC_TIMEOUT_T303 \
    /* Timeout T303 */\
  if (cc_v[6])\
  {\
    cc_v[6] = em_write_buffer_3 (CC_V_6, cc_data->ti);\
  }  /* cc_v[6] */

#define EM_CC_CALL_PROCEEDING_RECEIVED \
    /* Call proceeding received */\
  if (cc_v[7])\
  {\
    cc_v[7] = em_write_buffer_3 (CC_V_7, cc_data->index_ti);\
  } /* cc_v[7] */

#define EM_CC_RELEASE_COMPLETE_RECEIVED \
   /* RELEASE COMPLETE received */\
  if ((cc_v[8]))\
  {\
    if (rel_com->v_cc_cause AND rel_com->cc_cause.v_cause) \
      cc_v[8] = em_write_buffer_3a (CC_V_8, CAUSE_MAKE(DEFBY_STD, \
                                                  ORIGSIDE_NET, \
                                                  MNCC_CC_ORIGINATING_ENTITY,\
                                                  rel_com->cc_cause.cause));\
    else\
      cc_v[8] = em_write_buffer_3a (CC_V_8, CAUSE_MAKE(DEFBY_CONDAT, \
                                                  ORIGSIDE_NET, \
                                                  MNCC_CC_ORIGINATING_ENTITY,\
                                                  NOT_PRESENT_8BIT));\
  } /* cc_v[8] */

#define EM_CC_DISCONNECT_RECEIVED \
   /* Disconnect received */\
  if ((cc_v[9]))\
  {\
    if (disconnect->cc_cause.v_cause)\
      cc_v[9] = em_write_buffer_3a (CC_V_9, CAUSE_MAKE(DEFBY_STD, \
                                                  ORIGSIDE_NET, \
                                                  MNCC_CC_ORIGINATING_ENTITY,\
                                                  disconnect->cc_cause.cause));\
     /* else we don't indicate the message as it was erroneous anyway; */\
     /* note that the concept that only correct message are indicated is not kept here */\
     /* 100% because e.g. checks for optional errors follow below; but to make it */\
     /* simpler this is felt as acceptabe (otherwise we'd had to remember the cause value*/\
     /* in a local variable because CCD_END is called pretty soon...)*/\
  } /* cc_v[9] */

#define EM_CC_CONNECT_RECEIVED\
       /* Connect received */\
      if (cc_v[10])\
      {\
        cc_v[10] = em_write_buffer_3 (CC_V_10, cc_data->index_ti);\
      } /* cc_v[10] */


#define EM_CC_ALERTING_RECEIVED \
   /* Alert received */\
  if (cc_v[11])\
  {\
    cc_v[11] = em_write_buffer_3 (CC_V_11, cc_data->index_ti);\
  } /* cc_v[11] */

#define EM_CC_PROGRESS_RECEIVED \
        /* Progress received */\
      if (cc_v[12])\
      {\
        cc_v[12] = em_write_buffer_3 (CC_V_12, cc_data->progress_desc[cc_data->index_ti]);\
      } /* cc_v[12] */

#define EM_CC_CONNECT_ACKNOWLEDGE\
    /* Connect acknowledge sent*/\
      if (cc_v[13])\
      {\
        cc_v[13] = em_write_buffer_3 (CC_V_12, cc_data->progress_desc[cc_data->index_ti]);\
      } /* cc_v[13] */


#define EM_CC_RELEASE_SENT \
     /* Release sent */\
    if (cc_v[14])\
    {\
      cc_v[14] = em_write_buffer_3a (CC_V_14, cc_data->cc_cause[cc_data->ti]);\
    }  /* cc_v[14] */

#define EM_CC_DOWNLINK_SETUP_RECEIVED \
   if ((cc_v[15]) AND (_decodedMsg[0] EQ D_SETUP))\
  {\
    /* Downlink SETUP received  */\
    cc_v[15] = em_write_buffer_4a ( CC_V_15, &data->sdu.buf[CC_ENCODE_OFFSET >> 3], cc_data->ti);\
  } /* cc_v[15] */

#define EM_CC_CALL_CONFIRM_SENT \
  if (cc_v[16])\
  {\
    cc_v[16] = em_write_buffer_4a ( CC_V_16, &data->sdu.buf[CC_ENCODE_OFFSET >> 3], cc_data->ti);\
  } /* cc_v[16] */

#define EM_CC_STATUS_SENT \
      /* Status sent */\
      if (cc_v[17])\
      {\
        /* cause is mandatory, thus no validity check of cause necessary here */\
        cc_v[17] = em_write_buffer_3a (CC_V_17, CAUSE_MAKE(DEFBY_STD, \
                                                    ORIGSIDE_MS, \
                                                    MNCC_CC_ORIGINATING_ENTITY, \
                                                    status->cc_cause.cause));\
      }  /* cc_v[17] */

#define EM_CC_RELEASE_COMPLETE_SENT \
    /* RELEASE COMPLETE sent */\
  if (cc_v[18])\
  {\
    cc_v[18] = em_write_buffer_3a (CC_V_18,\
                     /* cause of RELEASE COMPLETE is optional */\
                     /* SBK-02-02-16: don't know why MSDEV compiler needs*/\
                     /* cast to avoid warning here */\
                     (USHORT) ((rel_com->v_cc_cause) ?\
                        CAUSE_MAKE(DEFBY_STD, \
                                   ORIGSIDE_MS,\
                                   MNCC_CC_ORIGINATING_ENTITY,\
                                   rel_com->cc_cause.cause)\
                      : CAUSE_MAKE(DEFBY_STD,\
                                   ORIGSIDE_MS, \
                                   MNCC_CC_ORIGINATING_ENTITY, \
                                   NOT_PRESENT_8BIT)));\
  }  /* cc_v[18] */

#define EM_CC_ALERTING_SENT\
       /* Alerting sent */\
      if (cc_v[19])\
      {\
        cc_v[19] = em_write_buffer_3 (CC_V_19, alert->ti);\
      } /* cc_v[19] */

#define EM_CC_CONNECT_SENT\
       /* Connect sent */\
      if (cc_v[20])\
      {\
        cc_v[20] = em_write_buffer_3 (CC_V_20, setup_res->ti);\
      } /* cc_v[20] */

#define EM_CC_CONNECT_ACKNOWLEDGE_RECEIVED \
          /* Connect ackn received */\
        if (cc_v[21])\
        {\
          cc_v[21] = em_write_buffer_3 (CC_V_21, cc_data->ti);\
        }  /* cc_v[21] */

#define EM_CC_DISCONNECT_SENT \
    /* Disconnect sent */\
  if (cc_v[22])\
  {\
   cc_v[22] = em_write_buffer_3a (CC_V_22, CAUSE_MAKE(DEFBY_STD, \
                                                 ORIGSIDE_MS, \
                                                 MNCC_CC_ORIGINATING_ENTITY,\
                                                 MNCC_CAUSE_TIMER));\
  }  /* cc_v[22] */

#define EM_CC_CHANNEL_MODE_CHANGE \
   /* Channel mode change */\
  if (cc_v[23])\
  {\
    cc_v[23] = em_write_buffer_4 (CC_V_23, sync->sync_info.ch_info.ch_type, sync->sync_info.ch_info.ch_mode);\
  }  /* cc_v[23] */

#define EM_CC_NOTIFICATION_FROM_THE_NETWORK \
        /* Notification from the network */\
        if (cc_v[24])\
        {\
          cc_v[24] = em_write_buffer_3 (CC_V_24, notify->notific.nd);\
        } /* cc_v[24] */

#define EM_CC_NOTIFICATION_TO_THE_NETWORK \
      /* Notification to the network */\
      if (cc_v[25])\
      {\
        cc_v[25] = em_write_buffer_3 (CC_V_25, notify->nd);\
      } /* cc_v[25] */

#define EM_CC_FACILITY_FROM_THE_NETWORK \
      /* Facility from the network */\
      if (cc_v[26])\
      {\
        cc_v[26] = em_write_buffer_3 (CC_V_26, cc_data->index_ti);\
      } /* cc_v[26] */

#define EM_CC_FACILITY_TO_THE_NETWORK \
        /* Facility to the network */\
        if (cc_v[27])\
        {\
          cc_v[27] = em_write_buffer_3 (CC_V_27, cc_data->index_ti);\
        } /* cc_v[27] */

#define EM_CC_START_DTMF \
   /* Start DTMF */\
  if (cc_v[28])\
  {\
    cc_v[28] = em_write_buffer_3 (CC_V_28, start_dtmf->key_facility.key);\
  }  /* cc_v[28] */


#define EM_CC_START_DTMF_ACKNOWLEDGE \
        /* Start DTMF acknowledge */\
        if (cc_v[29])\
        {\
          cc_v[29] = em_write_buffer_3 (CC_V_29, cc_data->ti);\
        } /* cc_v[29] */

#define EM_CC_STOP_DTMF \
  /* Stop DTMF */\
  if (cc_v[30])\
  {\
   cc_v[30] = em_write_buffer_3a (CC_V_30, cc_data->cc_cause[cc_data->ti]);\
  }  /* cc_v[30] */

#define EM_CC_LOSS_OF_CONNECTION \
    /* Loss of connection */\
  if (cc_v[31])\
  {\
    cc_v[31] = em_write_buffer_3a (CC_V_31, rel_ind->cause);\
  } /* cc_v[31] */

#define EM_CC_REESTABLISHED_STARTED \
  /* Re-establishment started */\
  if (cc_v[32])\
  {\
    cc_v[32] = em_write_buffer_2 (CC_V_32);\
  } /* cc_v[32] */

#define EM_CC_REESTABLISHED_SUCCESSFUL \
    /* Re-establishment successful */\
  if (cc_v[33])\
  {\
    cc_v[33] = em_write_buffer_2 (CC_V_33);\
  }  /* cc_v[33] */

#define EM_CC_REESTABLISHED_FAILED \
      /* Re-establishment failed */\
      if (cc_v[34])\
      {\
        cc_v[34] = em_write_buffer_2 (CC_V_34);\
      } /* cc_v[34] */

#define EM_CC_STATUS_RECEIVED \
  /* Status received */\
  if (cc_v[35])\
  {\
    cc_v[35] = em_write_buffer_3a (CC_V_35, CAUSE_MAKE(DEFBY_STD, \
                                                       ORIGSIDE_NET, \
                                                       MNCC_CC_ORIGINATING_ENTITY, \
                                                       status->cc_cause.cause));\
  } /* cc_v[35] */

#define EM_CC_STATUS_ENQUIRY_RECEIVED \
        /* Status enquiry received */\
        if (cc_v[36])\
        {\
          cc_v[36] = em_write_buffer_3 (CC_V_36, cc_data->ti);\
        } /* cc_v[36] */

#define EM_CC_CALL_HOLD \
      /* Call hold */\
      if (cc_v[37])\
      {\
        cc_v[37] = em_write_buffer_3 (CC_V_37, cc_data->index_ti);\
      } /* cc_v[37] */

#define EM_CC_CALL_HOLD_ACKNOWLEDGE \
            /* Call hold ackn */\
          if (cc_v[38])\
          {\
            cc_v[38] = em_write_buffer_3 (CC_V_38, cc_data->ti);\
          } /* cc_v[38] */

#define EM_CC_CALL_HOLD_REJECT \
        /* Call hold rej - Info: the "_2" is because the name is used earlier  */\
        if (cc_v[39])\
        {\
          cc_v[39] = em_write_buffer_3 (CC_V_39, cc_data->index_ti);\
        } /* cc_v[39] */

#define EM_CC_CALL_RETRIEVE \
       /* Call retrieve */\
      if (cc_v[40])\
      {\
        cc_v[40] = em_write_buffer_3 (CC_V_40, cc_data->index_ti);\
      } /* cc_v[40] */

#define EM_CC_CALL_RETRIEVE_ACKNOWLEDGE \
          /* Call retrieve ackn */\
          if (cc_v[41])\
          {\
            cc_v[41] = em_write_buffer_3 (CC_V_41, cc_data->index_ti);\
          } /* cc_v[41] */

#define EM_CC_CALL_RETRIEVE_REJECT \
          /* Call retrieve rej */\
          if (cc_v[42])\
          {\
            cc_v[42] = em_write_buffer_3 (CC_V_42, cc_data->index_ti);\
          } /* cc_v[42] */

#define EM_CC_MO_IN_CALL_MODIFICATION \
        /* MO in call modification */\
        if (cc_v[43])\
        {\
          cc_v[43] = em_write_buffer_3 (CC_V_43, cc_data->index_ti);\
        } /* cc_v[43] */

#define EM_CC_MO_IN_CALL_MODIFICATION_PASSED \
          /* MO in call modification passed */\
          if (cc_v[44])\
          {\
            cc_v[44] = em_write_buffer_3 (CC_V_44, cc_data->ti);\
          } /* cc_v[44] */

#define EM_CC_MO_IN_CALL_MODIFICATION_FAILED \
        /* MO in call modification failed */\
        if (cc_v[45])\
        {\
          cc_v[45] = em_write_buffer_3 (CC_V_45, cc_data->index_ti);\
        } /* cc_v[45] */

#define EM_CC_MT_IN_CALL_MODIFICATION_PASSED \
     /* MT in call modification passed */\
    if (cc_v[46])\
    {\
      cc_v[46] = em_write_buffer_3 (CC_V_46, cc_data->ti);\
    } /* cc_v[46] */



#define EM_CC_MT_IN_CALL_MODIFICATION_FAILED \
  /* MT in-call modification failed */\
  if (cc_v[47])\
  {\
    cc_v[47] = em_write_buffer_3 (CC_V_47, cc_data->ti);\
  } /* cc_v[47] */

#define EM_CC_USUER_TO_USER_DATA_SENT \
      /* User 2 user data sent */\
      if (cc_v[48])\
        {\
          cc_v[48] = em_write_buffer_3 (CC_V_48, cc_data->index_ti);\
        } /* cc_v[49] */


#define EM_CC_USER_TO_USER_DATA_RECEIVED \
        /* User 2 user data received */\
        if (cc_v[49])\
        {\
          cc_v[49] = em_write_buffer_3 (CC_V_49, cc_data->index_ti);\
        } /* cc_v[49] */

#else /*FF_EM_MODE not defined*/

#define EM_CC_START_MO_CALL                 /* Event 1 */
#define EM_CC_MM_CONNECTION_ESTABLISHED_MT /* Event 2 */
#define EM_CC_MM_CONNECTION_ESTABLISHED_MO /* Event 2 */
#define EM_CC_MM_CONNECTION_FAILED          /* Event 3 */
#define EM_CC_SENT_SETUP_MESSAGE            /* Event 4 */
#define EM_CC_SENT_EMERGENCY_SETUP_MESSAGE  /* Event 5 */
#define EM_CC_TIMEOUT_T303              /* Event 6 */
#define EM_CC_CALL_PROCEEDING_RECEIVED  /* Event 7 */
#define EM_CC_RELEASE_COMPLETE_RECEIVED /* Event 8 */
#define EM_CC_DISCONNECT_RECEIVED       /* Event 9 */
#define EM_CC_CONNECT_RECEIVED          /* Event 10*/
#define EM_CC_ALERTING_RECEIVED         /* Event 11*/
#define EM_CC_PROGRESS_RECEIVED         /* Event 12*/
#define EM_CC_CONNECT_ACKNOWLEDGE       /* Event 13*/
#define EM_CC_RELEASE_SENT              /* Event 14*/
#define EM_CC_DOWNLINK_SETUP_RECEIVED   /* Event 15*/
#define EM_CC_CALL_CONFIRM_SENT         /* Event 16*/
#define EM_CC_STATUS_SENT               /* Event 17*/
#define EM_CC_RELEASE_COMPLETE_SENT     /* Event 18*/
#define EM_CC_ALERTING_SENT             /* Event 19*/
#define EM_CC_CONNECT_SENT              /* Event 20*/
#define EM_CC_CONNECT_ACKNOWLEDGE_RECEIVED /* Event 21*/
#define EM_CC_DISCONNECT_SENT           /* Event 22*/
#define EM_CC_CHANNEL_MODE_CHANGE       /* Event 23*/
#define EM_CC_NOTIFICATION_FROM_THE_NETWORK/* Event 24*/
#define EM_CC_NOTIFICATION_TO_THE_NETWORK  /* Event 25*/
#define EM_CC_FACILITY_FROM_THE_NETWORK    /* Evnet 26*/
#define EM_CC_FACILITY_TO_THE_NETWORK      /* Event 27*/
#define EM_CC_START_DTMF                /* Event 28*/
#define EM_CC_START_DTMF_ACKNOWLEDGE       /* Event 29*/
#define EM_CC_STOP_DTMF                 /* Event 30*/
#define EM_CC_LOSS_OF_CONNECTION        /* Event 31*/
#define EM_CC_REESTABLISHED_STARTED     /* Event 32*/
#define EM_CC_REESTABLISHED_SUCCESSFUL  /* Event 33*/
#define EM_CC_REESTABLISHED_FAILED      /* Event 34*/
#define EM_CC_STATUS_RECEIVED              /* Event 35*/
#define EM_CC_STATUS_ENQUIRY_RECEIVED      /* Event 36*/
#define EM_CC_CALL_HOLD                    /* Event 37*/
#define EM_CC_CALL_HOLD_ACKNOWLEDGE        /* Event 38*/
#define EM_CC_CALL_HOLD_REJECT             /* Event 39*/
#define EM_CC_CALL_RETRIEVE                /* Event 40*/
#define EM_CC_CALL_RETRIEVE_ACKNOWLEDGE    /* Event 41*/
#define EM_CC_CALL_RETRIEVE_REJECT         /* Event 42*/
#define EM_CC_MO_IN_CALL_MODIFICATION      /* Event 43*/
#define EM_CC_MO_IN_CALL_MODIFICATION_PASSED /* Event 44*/
#define EM_CC_MO_IN_CALL_MODIFICATION_FAILED /* Event 45*/
#define EM_CC_MT_IN_CALL_MODIFICATION_PASSED /* Event 46*/
#define EM_CC_MT_IN_CALL_MODIFICATION_FAILED /* Event 47*/
#define EM_CC_USUER_TO_USER_DATA_SENT       /* Event 48*/
#define EM_CC_USER_TO_USER_DATA_RECEIVED   /* Event 49*/


#endif  /* FF_EM_MODE */
#endif /* CC_EM_H */
