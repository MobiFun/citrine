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
#ifndef ALR_EM_H
#define ALR_EM_H

#if defined (FF_EM_MODE) AND defined (ALR)


/* -------------------data declarations for EM / Variables------------------------------*/

/*
*  The offset is used to indicate the source entity the event trace is from.
*  L1/ALR = 0x00, DL = 0x2D, RR = 0x37, MM = 0x5F, CC = 0x78, SS = 0xAF, SMS = 0xBE, SIM = E1
*/
#define L1_OFFSET                   0x00

/*
*   EM_MAX_ALR_EVENTS defines maximum number of event traces for the engineering mode.
*   The number is increased by one to ensure that the event numbers defined in the
*   corresponding document are the same as in the sources.
*/
#define EM_MAX_ALR_EVENTS            40

#define EM_L1_SEM_SIZE   120     /* Max. number off all data fall on to the event tracing */

/*
*  Definitions for the additional parameters used for the event tracing.
*/
#define EM_AVAIL                       1
#define EM_NOT_AVAIL                   0
#define EM_HANDOVER_ASYNC              0
#define EM_HANDOVER_SYNC               1
#define EM_HANDOVER_PRE_SYNC           2
#define EM_TCH_CLOSE                   0
#define EM_TCH_OPEN                    1
#define EM_DAI_STOP                    0
#define EM_DAI_START                   1

/*
*  Type is combination of entity index(upper nibble) plus event number(lower nibble).
*  Bit 8  7  6  5  4  3  2  1
*      |    entity    |  event number |
*/

#define ALR_V_1             (1 + L1_OFFSET)
#define ALR_V_2             (2 + L1_OFFSET)
#define ALR_V_3             (3 + L1_OFFSET)
#define ALR_V_4             (4 + L1_OFFSET)
#define ALR_V_5             (5 + L1_OFFSET)
#define ALR_V_6             (6 + L1_OFFSET)
#define ALR_V_7             (7 + L1_OFFSET)
#define ALR_V_8             (8 + L1_OFFSET)
#define ALR_V_9             (9 + L1_OFFSET)
#define ALR_V_10            (10+ L1_OFFSET)
#define ALR_V_11            (11+ L1_OFFSET)
#define ALR_V_12            (12+ L1_OFFSET)
#define ALR_V_13            (13+ L1_OFFSET)
#define ALR_V_14            (14+ L1_OFFSET)
#define ALR_V_15            (15+ L1_OFFSET)
#define ALR_V_16            (16+ L1_OFFSET)
#define ALR_V_17            (17+ L1_OFFSET)
#define ALR_V_18            (18+ L1_OFFSET)
#define ALR_V_19            (19+ L1_OFFSET)
#define ALR_V_20            (20+ L1_OFFSET)
#define ALR_V_21            (21+ L1_OFFSET)
#define ALR_V_22            (22+ L1_OFFSET)
#define ALR_V_23            (23+ L1_OFFSET)
#define ALR_V_24            (24+ L1_OFFSET)
#define ALR_V_25            (25+ L1_OFFSET)
#define ALR_V_26            (26+ L1_OFFSET)
#define ALR_V_27            (27+ L1_OFFSET)
#define ALR_V_28            (28+ L1_OFFSET)
#define ALR_V_29            (29+ L1_OFFSET)
#define ALR_V_30            (30+ L1_OFFSET)
#define ALR_V_31            (31+ L1_OFFSET)
#define ALR_V_32            (32+ L1_OFFSET)
#define ALR_V_33            (33+ L1_OFFSET)
#define ALR_V_34            (34+ L1_OFFSET)
#define ALR_V_35            (35+ L1_OFFSET)
#define ALR_V_36            (36+ L1_OFFSET)
#define ALR_V_37            (37+ L1_OFFSET)
#define ALR_V_38            (38+ L1_OFFSET)
#define ALR_V_39            (39+ L1_OFFSET)


/* Event tracing flags for EM */
EXTERN BOOL alr_v[EM_MAX_ALR_EVENTS];

EXTERN UBYTE em_act_rlt;							/*lint -esym(526,em_act_rlt) : not defined here */
EXTERN UBYTE em_act_dlt;							/*lint -esym(526,em_act_dlt) : not defined here */


#endif /* FF_EM_MODE */

/*----------------Functions--------*/

/*
 *  Layer1/ALR primitives Engineering Mode
 *  Bitmask for the event tracing
 */
#if defined (FF_EM_MODE) AND defined (ALR)
EXTERN void l1_em_l1_event_req      (T_EM_L1_EVENT_REQ *em_l1_event_req);

/*
 * If all entities are linked into one module this definitions
 * prefixes all this functions with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define em_write_buffer_2        _ENTITY_PREFIXED(em_write_buffer_2)
  #define em_write_buffer_3        _ENTITY_PREFIXED(em_write_buffer_3)
  #define em_write_buffer_3a       _ENTITY_PREFIXED(em_write_buffer_3a)
  #define em_write_buffer_4        _ENTITY_PREFIXED(em_write_buffer_4)
//  #define check_write_index   _ENTITY_PREFIXED(check_write_index)
#endif

EXTERN UBYTE em_write_buffer_2       (UBYTE event_no);
EXTERN UBYTE em_write_buffer_3       (UBYTE event_no, UBYTE value);
EXTERN UBYTE em_write_buffer_3a      (UBYTE event_no, USHORT value);
EXTERN UBYTE em_write_buffer_4       (UBYTE event_no, UBYTE value1, UBYTE value2);
EXTERN void  em_init_l1_event_trace(void);
EXTERN void  alr_em_error_cause(UBYTE cause, USHORT arfcn);

/*
 * Semaphor handling
 */
EXTERN      void  em_l1_sem_init        (void);
EXTERN      void  em_l1_sem_exit        (void);
#endif /* FF_EM_MODE */

/*-----------------------Macro Definition -----------*/

#if defined (FF_EM_MODE) AND defined (ALR)

/* Macros with NAME {...;} because of LINT */
#define ALR_EM_SET_EM_ACT_RLT   (em_act_rlt=alr_data->dedi_data.act_rlt)
#define ALR_EM_SET_EM_ACT_RLT_2 (em_act_rlt=dd->act_rlt)
#define ALR_EM_SET_EM_ACT_DLT   (em_act_dlt = alr_data->pch_data.act_dlt)
#define ALR_EM_ERROR_IND(cause, arfcn) alr_em_error_cause(cause, arfcn)


#define ALR_EM_POWER_MEASSUREMENT_REQUEST\
    /* Power measurement request */\
  if (alr_v[1])\
  {\
  alr_v[1] = em_write_buffer_3 (ALR_V_1 , mph_power_req->pch_interrupt);\
  }  /* alr_v[1] */

#define ALR_EM_POWER_MEASUREMENT_CONFIRM\
         /* Power measurement confirm */\
        if (alr_v[2])\
        {\
         alr_v[2] = em_write_buffer_2 (ALR_V_2 );\
        }  /* alr_v[2] */

#define ALR_EM_FIELDSTRENGTH_MEASUREMENT_REQUEST\
        /* Fieldstrength measurement request */\
      if (alr_v[3])\
      {\
        alr_v[3] = em_write_buffer_2 (ALR_V_3 );\
      }  /* alr_v[3] */

#define ALR_EM_FIELDSTRENGTH_MEASUREMENT_CONFIRM\
  /* Fieldstrength measurement confirm */\
  if (alr_v[4])\
  {\
    alr_v[4] = em_write_buffer_2 (ALR_V_4 );\
  }  /* alr_v[4] */

#define ALR_EM_BSIC_REQUEST\
        /* BSIC request */\
      if (alr_v[5])\
      {\
        alr_v[5] = em_write_buffer_3a (ALR_V_5 , (USHORT)(mph_bsic_req->arfcn&ARFCN_MASK));\
      }  /* alr_v[5] */

#define ALR_EM_BSIC_CONFIRM(x)\
        /* BSIC confirm */\
      if (alr_v[6])\
      {\
       alr_v[6] = em_write_buffer_3 (ALR_V_6, x);\
      }  /* alr_v[6] */

#define ALR_EM_BCCH_READ_ERROR\
  /* CS_BCCH_READ_ERROR */\
 if (alr_v[7])\
 {\
   alr_v[7] = em_write_buffer_3a (ALR_V_7 , arfcn);\
 }  /* alr_v[7] */\
 


#define ALR_EM_CONFIGURE_CLASSMARK\
    /* Configure classmark */\
  if (alr_v[8])\
  {\
  alr_v[8] = em_write_buffer_4 (ALR_V_8 , classmark->classmark.pclass,\
                                     classmark->classmark.pclass2);\
  }  /* alr_v[8] */

#define ALR_EM_CONFIGURE_IDLE_MODE\
      /* Configure idle mode */\
      if (alr_v[9])\
      {\
       alr_v[9] = em_write_buffer_3a (ALR_V_9 , idle->arfcn);\
      }  /* alr_v[9] */ 

#define ALR_EM_CONFIGURE_CBCH_CHANNEL\
   /* Configure CBCH channel */\
  if (alr_v[10])\
  {\
   alr_v[10] = em_write_buffer_3 (ALR_V_10 , cbch->stat);\
  }  /* alr_v[10] */

#define ALR_EM_CONFIGURE_NEIGHBOUERCELL_LIST\
      /* Configure neighbourcell list */\
      if (alr_v[11])\
      {\
       alr_v[11] = em_write_buffer_3 (ALR_V_11, alr_data->nc_data.c_ba_arfcn);\
      }  /* alr_v[11] */

#define ALR_EM_PAGE_MODE_CHANGE\
  /* Page mode change */\
  if (alr_v[12])\
  {\
    alr_v[12] = em_write_buffer_3 (ALR_V_12,  alr_data->pch_data.pl_idle.page_mode);\
  }  /* alr_v[12] */

#define ALR_EM_IDLE_MODE_BCCH_PARAMETER_CHANGED\
                  /* Idle mode BCCH parameter change */\
                if (alr_v[13])\
                {\
                 alr_v[13] = em_write_buffer_3a (ALR_V_13, data_ind->radio_freq);\
                }  /* alr_v[13] */

#define ALR_EM_DOWNLINK_FAILURE\
  /* CS_DOWN_LINK_FAIL */\
  if (alr_v[14])\
  {\
    alr_v[14] = em_write_buffer_2 (ALR_V_14);\
  }  /* alr_v[14] */

#define ALR_EM_PAGING_DETECTED\
    /* Paging detected */\
  if (alr_v[15])\
  {\
   alr_v[15] = em_write_buffer_3 (ALR_V_15, id_type);\
  }  /* alr_v[15] */

#define ALR_EM_NEIGHBOURCELL_BSIC_REQUEST\
    /* NC BSIC request */\
  if (alr_v[16])\
  {\
    alr_v[16] = em_write_buffer_3a (ALR_V_16, mph_bsic_req->arfcn);\
  }  /* alr_v[16] */

#define ALR_EM_NEIGHBOURCELL_BSIC_CONFIRM(x)\
  /* NC BSIC confirm */\
    if (alr_v[17])\
    {\
      alr_v[17] = em_write_buffer_3 (ALR_V_17, x);\
    }  /* alr_v[17] */

#define ALR_EM_READ_NEIGHBOURCELL_SB\
 /* Read NC SB idle */\
  if (alr_v[18])\
  {\
    alr_v[18] = em_write_buffer_3a (ALR_V_18 , p_ncell->ba_arfcn);\
  }  /* alr_v[18] */

#define ALR_EM_NEIGHBOURCELL_SB(x)\
     /* NC SB */\
    if (alr_v[19])\
    {\
      alr_v[19] = em_write_buffer_3 (ALR_V_19, x);\
    }  /* alr_v[19] */

#define ALR_EM_READ_NEIGHBOURCELL_BCCH\
    /* Read NC BCCH */\
  if (alr_v[20])\
  {\
    alr_v[20] = em_write_buffer_3a (ALR_V_20 , bcch_req->radio_freq);\
  }  /* alr_v[20] */

#define ALR_EM_NEIGHBOURCELL_BCCH(x)\
  /* neighbourcell BCCH not available */\
   if (alr_v[21])\
   {\
     alr_v[21] = em_write_buffer_3 (ALR_V_21, x);\
   }  /* alr_v[21] */

#define ALR_EM_CONFIGURE_CELL_RESELECTION\
    /* Configure cell reselection */\
    if (alr_v[22])\
    {\
      alr_v[22] = em_write_buffer_3a (ALR_V_22 , arfcn);\
    }  /* alr_v[22] */

#define ALR_EM_START_CONNECTION_ESTABLISHMENT\
        /* Start connection establishment */\
        if (alr_v[23])\
        {\
          alr_v[23] = em_write_buffer_2(ALR_V_23);\
        }  /* alr_v[23] */

#define ALR_EM_CONFIGURE_IMMIDIATE_ASSIGNMENT\
    /* Configure immediate assignment */\
  if (alr_v[24])\
  {\
   alr_v[24] = em_write_buffer_2(ALR_V_24);\
  }  /* alr_v[24] */

#define ALR_EM_STOP_CONNECTION_ESTABLISHMENT\
        /* Stop connection establishment */\
        if (alr_v[25])\
        {\
          alr_v[25] = em_write_buffer_2(ALR_V_25);\
        }  /* alr_v[25] */

#define ALR_EM_CONFIGURE_CHANNEL_ASSIGNMENT\
  /* Configure channel assignment */\
  if (alr_v[26])\
  {\
   alr_v[26] = em_write_buffer_2(ALR_V_26);\
  }  /* alr_v[26] */

#define ALR_EM_CONFIGURE_HANDOVER(x)\
  /* Configure asynchronous, presynchr. or synchronized handover */\
  if (alr_v[27])\
  {\
   alr_v[27] = em_write_buffer_3(ALR_V_27, x);\
  }  /* alr_v[27] */

#define ALR_EM_CONFIGURE_FREQUENCY_REDEFINITION\
    /* Configure frequency redefinition */\
  if (alr_v[28])\
  {\
   alr_v[28] = em_write_buffer_2(ALR_V_28);\
  }  /* alr_v[28] */

#define ALR_EM_CONFIGURE_CHANNEL_MODE_MODIFY\
  /* Configure channel mode modify */\
  if (alr_v[29])\
  {\
   alr_v[29] = em_write_buffer_2(ALR_V_29);\
  }  /* alr_v[29] */

#define ALR_EM_CONFIGURE_CIPHER_MODE_SETTING\
  /* Configure cipher mode setting */\
  if (alr_v[30])\
  {\
   alr_v[30] = em_write_buffer_2(ALR_V_30);\
  }  /* alr_v[30] */

#define ALR_EM_START_CBCH_READING\
  /* Start CBCH reading */\
  if (alr_v[31])\
  {\
   alr_v[31] = em_write_buffer_3(ALR_V_31,(flags & CBCH_SCHED));\
  }  /* alr_v[31] */

#define ALR_EM_RECEIVE_CBCH_MESSAGE\
    /* Receive CBCH message */\
    if (alr_v[32])\
    {\
     alr_v[32] = em_write_buffer_3(ALR_V_32, block_no);\
    }  /* alr_v[32] */

#define ALR_EM_CBCH_MESSAGE_COMPLETE\
  /* CBCH message complete */\
 if (alr_v[33])\
 {\
  alr_v[33] = em_write_buffer_2(ALR_V_33);\
 }  /* alr_v[33] */

#define ALR_EM_STOP_CBCH_READING\
      /* Stop CBCH reading */\
    if (alr_v[34])\
    {\
     alr_v[34] = em_write_buffer_2(ALR_V_34);\
    }  /* alr_v[34] */

#define ALR_EM_RECEIVE_NULL_MESSAGE\
    /* Receive NULL message */\
  if (alr_v[35])\
  {\
   alr_v[35] = em_write_buffer_2(ALR_V_35);\
  }  /* alr_v[35] */

#define ALR_EM_RECEIVE_SCHEDULE_MESSAGE\
  /* Receive scheduled message */\
  if (alr_v[36])\
  {\
   alr_v[36] = em_write_buffer_2(ALR_V_36);\
  }  /* alr_v[36] */

#define ALR_EM_CONFIGURE_TCH_LOOP(x)\
  /* Configure TCH Loop */\
  if (alr_v[37])\
  {\
   alr_v[37] = em_write_buffer_3(ALR_V_37, x);\
  }  /* alr_v[37] */

#define ALR_EM_CONFIGURE_DAI_TESTING(x)\
    /* Configure DAI testing */\
    if (alr_v[38])\
    {\
     alr_v[38] = em_write_buffer_3(ALR_V_38, x);\
    }  /* alr_v[38] */

#define ALR_EM_DEACTIVATE_L1\
  /* Deactivate Layer 1 */\
  if (alr_v[39])\
  {\
    alr_v[39] = em_write_buffer_2(ALR_V_39);\
  }  /* alr_v[39] */


#else

#define EM_TCH_CLOSE  0/*necessary to compile*/
#define EM_TCH_OPEN   1              

#define ALR_EM_SET_EM_ACT_RLT
#define ALR_EM_SET_EM_ACT_RLT_2
#define ALR_EM_SET_EM_ACT_DLT
#define ALR_EM_ERROR_IND(cause, arfcn)
#define ALR_EM_POWER_MEASSUREMENT_REQUEST        /*Event  1*/
#define ALR_EM_POWER_MEASUREMENT_CONFIRM         /*Event  2*/
#define ALR_EM_FIELDSTRENGTH_MEASUREMENT_REQUEST /*Event  3*/
#define ALR_EM_FIELDSTRENGTH_MEASUREMENT_CONFIRM /*Event  4*/
#define ALR_EM_BSIC_REQUEST                      /*Event  5*/
#define ALR_EM_BSIC_CONFIRM(x)                   /*Event  6*/
#define ALR_EM_BCCH_READ_ERROR                   /*Event  7*/
#define ALR_EM_CONFIGURE_CLASSMARK               /*Event  8*/
#define ALR_EM_CONFIGURE_IDLE_MODE               /*Event  9*/
#define ALR_EM_CONFIGURE_CBCH_CHANNEL            /*Event 10*/
#define ALR_EM_CONFIGURE_NEIGHBOUERCELL_LIST     /*Event 11*/
#define ALR_EM_PAGE_MODE_CHANGE                  /*Event 12*/
#define ALR_EM_IDLE_MODE_BCCH_PARAMETER_CHANGED  /*Event 13*/
#define ALR_EM_DOWNLINK_FAILURE                  /*Event 14*/
#define ALR_EM_PAGING_DETECTED                   /*Event 15*/
#define ALR_EM_NEIGHBOURCELL_BSIC_REQUEST        /*Event 16*/
#define ALR_EM_NEIGHBOURCELL_BSIC_CONFIRM(x)     /*Event 17*/
#define ALR_EM_READ_NEIGHBOURCELL_SB             /*Event 18*/
#define ALR_EM_NEIGHBOURCELL_SB(x)               /*Event 19*/
#define ALR_EM_READ_NEIGHBOURCELL_BCCH           /*Event 20*/
#define ALR_EM_NEIGHBOURCELL_BCCH(x)             /*Event 21*/
#define ALR_EM_CONFIGURE_CELL_RESELECTION        /*Event 22*/
#define ALR_EM_START_CONNECTION_ESTABLISHMENT    /*Event 23*/
#define ALR_EM_CONFIGURE_IMMIDIATE_ASSIGNMENT    /*Event 24*/
#define ALR_EM_STOP_CONNECTION_ESTABLISHMENT     /*Event 25*/
#define ALR_EM_CONFIGURE_CHANNEL_ASSIGNMENT      /*Event 26*/
#define ALR_EM_CONFIGURE_HANDOVER(x)             /*Event 27*/
#define ALR_EM_CONFIGURE_FREQUENCY_REDEFINITION  /*Event 28*/
#define ALR_EM_CONFIGURE_CHANNEL_MODE_MODIFY     /*Event 29*/
#define ALR_EM_CONFIGURE_CIPHER_MODE_SETTING     /*Event 30*/
#define ALR_EM_START_CBCH_READING                /*Event 31*/
#define ALR_EM_RECEIVE_CBCH_MESSAGE              /*Event 32*/
#define ALR_EM_CBCH_MESSAGE_COMPLETE             /*Event 33*/
#define ALR_EM_STOP_CBCH_READING                 /*Event 34*/
#define ALR_EM_RECEIVE_NULL_MESSAGE              /*Event 35*/
#define ALR_EM_RECEIVE_SCHEDULE_MESSAGE          /*Event 36*/
#define ALR_EM_CONFIGURE_TCH_LOOP                /*Event 37*/
#define ALR_EM_CONFIGURE_DAI_TESTING(x)          /*Event 38*/
#define ALR_EM_DEACTIVATE_L1                     /*Event 39*/
        
#endif /*FF_EM_MODE*/        
#endif /* ALR_EM_H */    
