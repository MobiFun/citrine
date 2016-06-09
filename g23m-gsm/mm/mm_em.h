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
  
#ifndef MM_EM_H
#define MM_EM_H

#ifdef FF_EM_MODE
/* ---------------- data declarations for EM ----------------*/

/*
 * Ring buffer is by one bigger than max. number of prims, because it
 * needs always one empty entry
 */
#define EM_MM_BUFFER_SIZE             168           

/*
 * EM_MAX_MM_EVENTS defines maximum number of event traces for the engineering 
 * mode. The number is increased by one to ensure that the event numbers 
 * defined in the corresponding document are the same as in the sources.
 */
#define EM_MAX_MM_EVENTS              19

/*
 * The offset is used to indicate the source entity the event trace is from. 
 * L1/ALR = 0x00, DL = 0x2D, RR = 0x37, MM = 0x5F, 
 * CC = 0x78, SS = 0xAF, SMS = 0xBE, SIM = E1
 */
#define MM_OFFSET                   0x5F
#define EM_ESTABLISHED                 2
#define EM_FAILED                      3
#define EM_ACCEPT                      4
#define EM_REJECT                      5
#define EM_AUTOMATIC                   0
#define EM_MANUAL                      1
#define EM_LIMITED_SERVICE             8
#define EM_FULL_SERVICE                9
#define EM_NO_SERVICE                 10
#define EM_NORMAL_LUP                 11
#define EM_PERIODIC_LUP               12
#define EM_IMSI_ATTACH_LUP            13
#define EM_REQUEST                    14
#define EM_RESPONSE                   15
#define EM_CIPHERING                  14
#define EM_COMMAND                    15

/*
 * Type is combination of entity index(upper nibble) plus 
 * event number(lower nibble). To optimize the buffersize, 
 * the data is packed in the buffer as follows: type, length, value
 * Value is the combination of the additional parameters 
 * as defined in 8443.601, coded as UBYTE.
 */
#define MM_V_1            (1+MM_OFFSET)
#define MM_V_2            (2+MM_OFFSET)
#define MM_V_3            (3+MM_OFFSET)
#define MM_V_4            (4+MM_OFFSET)
#define MM_V_5            (5+MM_OFFSET)
#define MM_V_6            (6+MM_OFFSET)
#define MM_V_7            (7+MM_OFFSET)
#define MM_V_8            (8+MM_OFFSET)
#define MM_V_9            (9+MM_OFFSET)
#define MM_V_10          (10+MM_OFFSET)
#define MM_V_11          (11+MM_OFFSET)
#define MM_V_12          (12+MM_OFFSET)
#define MM_V_13          (13+MM_OFFSET)
#define MM_V_14          (14+MM_OFFSET)
#define MM_V_15          (15+MM_OFFSET)
#define MM_V_16          (16+MM_OFFSET)
#define MM_V_17          (17+MM_OFFSET)
#define MM_V_18          (18+MM_OFFSET)

EXTERN BOOL mm_v[EM_MAX_MM_EVENTS];

EXTERN UBYTE em_mm_event_buffer[EM_MM_BUFFER_SIZE];
EXTERN UBYTE em_mm_buffer_write;

/*----------------- Functions ---------------------*/


/*
 *  Mobility management primitives Engineering Mode
 *  Bitmask for the event tracing
 */
EXTERN void mm_em_mm_event_req         (T_EM_MM_EVENT_REQ       *em_mm_event_req);
EXTERN void mm_em_dl_event_req         (T_EM_DL_EVENT_REQ       *em_dl_event_req);
EXTERN void em_init_mm_event_trace     (void);

/*
 * If all entities are linked into one module this definitions
 * prefixes all this functions with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define em_write_buffer_2        _ENTITY_PREFIXED(em_write_buffer_2)
  #define em_write_buffer_3        _ENTITY_PREFIXED(em_write_buffer_3)
  #define em_write_buffer_3a       _ENTITY_PREFIXED(em_write_buffer_3a)
  #define em_write_buffer_3b       _ENTITY_PREFIXED(em_write_buffer_3b)  
  #define em_write_buffer_4        _ENTITY_PREFIXED(em_write_buffer_4)
  #define em_write_buffer_4a       _ENTITY_PREFIXED(em_write_buffer_4a)  
  #define em_write_buffer_4b       _ENTITY_PREFIXED(em_write_buffer_4b)
  #define em_write_buffer_4c       _ENTITY_PREFIXED(em_write_buffer_4c)  
  #define em_write_buffer_5a       _ENTITY_PREFIXED(em_write_buffer_5a)
  #define em_write_buffer_6        _ENTITY_PREFIXED(em_write_buffer_6)

  #define check_write_index     _ENTITY_PREFIXED(check_write_index)
#endif /*OPTION_MULTITHREAD*/

EXTERN UBYTE em_write_buffer_2  (UBYTE event_no);
EXTERN UBYTE em_write_buffer_3  (UBYTE event_no, UBYTE value);
EXTERN UBYTE em_write_buffer_3a  (UBYTE event_no, USHORT value);
EXTERN UBYTE em_write_buffer_3b (UBYTE event_no, ULONG value);
EXTERN UBYTE em_write_buffer_4 (UBYTE event_no, UBYTE value1, T_plmn *plmn);
EXTERN UBYTE em_write_buffer_4a (UBYTE event_no, UBYTE value1, USHORT value2);
EXTERN UBYTE em_write_buffer_4b (UBYTE event_no, UBYTE ident_type, UBYTE *value);
EXTERN UBYTE em_write_buffer_4c (UBYTE event_no, UBYTE ident_type, ULONG value);
EXTERN UBYTE em_write_buffer_5a (UBYTE event_no, UBYTE value, UBYTE mcc[SIZE_MCC], UBYTE mnc[SIZE_MNC]);
EXTERN UBYTE em_write_buffer_6 (UBYTE event_no, UBYTE loc_upd_type, T_loc_area_ident lai);



EXTERN UBYTE check_write_index (UBYTE n);



/* ---------------- Macro definitions ---------------------- */

#define EM_SIM_INSERT \
      /* SIM Insert */\
      if (mm_v[1])\
      {\
        mm_v[1] = em_write_buffer_2 (MM_V_1 );\
      }  /* mm_v[1] */

#define EM_SIM_REMOVE \
  /* SIM remove */\
  if (mm_v[2])\
  {\
    mm_v[2] = em_write_buffer_2 (MM_V_2 );\
  } /* mm_v[2] */

#define EM_SET_PLMN_SEARCH_MODE \
  /* Set PLMN search mode */\
  if (mm_v[3])\
  {\
    mm_v[3] = em_write_buffer_3 (MM_V_3 , plmn_mode_req->mode);\
  }  /* mm_v[3] */

#define EM_START_REGISTRATION_AUTO_MODE \
  /* Start registration auto mode */\
  if (mm_v[4])\
  {\
    mm_v[4] = em_write_buffer_5a (MM_V_4, SERVICE_MODE_FULL,\
                             mm_data->reg.actual_plmn.mcc,\
                             mm_data->reg.actual_plmn.mnc);\
  } /* mm_v[4] */

#define EM_START_REGISTRATION_MANUAL_MODE \
      /* Start registration manual mode */\
      if (mm_v[4])\
      {\
        mm_v[4] = em_write_buffer_5a (MM_V_4, service_mode, mm_data->mm.lai.mcc, mm_data->mm.lai.mnc);\
      } /* mm_v[4] */

#define EM_START_PLMN_LIST_REQUEST \
     /* Start PLMN list request */\
    if (mm_v[5])\
    {\
      mm_v[5] = em_write_buffer_2 (MM_V_5 );\
    }  /* mm_v[5] */

#define EM_CELL_SELECTION_RESELECTION \
 /* Cell selection/reselection */\
  if (mm_v[6])\
  {\
    mm_v[6] = em_write_buffer_5a (MM_V_6, rr_activate_cnf->op.service, rr_activate_cnf->plmn.mcc, rr_activate_cnf->plmn.mnc);\
  }  /* mm_v[6] */

#define EM_RESULT_PLMN_LIST \
/* MM Event Tracing */\
/* Result PLMN list */\
  if (mm_v[7])\
  {\
    mm_v[7] = em_write_buffer_4(MM_V_7, rr_abort_ind->plmn_avail, rr_abort_ind->plmn);\
  }  /* mm_v[7] */

#define EM_RR_CONECTION_REQUESTED \
  /* RR connection requested */\
  if (mm_v[8])\
  {\
    mm_v[8] = em_write_buffer_6 (MM_V_8, loc_upd_type, mm_data->mm.lai);\
  }  /* mm_v[8] */

#define EM_RR_CONECTION_ESTABLISHED \
  /* RR connection established */\
  if (mm_v[9])\
  {\
    mm_v[9] = em_write_buffer_2 (MM_V_9);\
  }  /* mm_v[9] */

#define EM_RR_CONNECTION_ESTABLISHED_2 \
      if (mm_count_connections (CM_PENDING) NEQ 0)\
      {\
        /* RR connection established */\
        if (mm_v[9])\
        {\
          /* Event number flag is set */\
          mm_v[9] = em_write_buffer_3a (MM_V_9, mm_data->rej_cause);\
        } /* mm_v[9] */\
      } /* mm_count_connections (CM_PENDING) */

#define EM_LOCATION_UPDATING \
                /* Location updating */\
              if (mm_v[10])\
              {\
                mm_v[10] = em_write_buffer_3 (MM_V_10, EM_ACCEPT);\
              }  /* mm_v[10 */

#define EM_LOCATION_UPDATING_REJECT \
  /* Location updating reject*/\
  if (mm_v[10])\
  {\
    mm_v[10]=em_write_buffer_4a ( MM_V_10, EM_REJECT, mm_data->limited_cause);\
  } /* mm_v[10] */

#define EM_TMSI_REALLOCATION_COMPLETE\
              /* TMSI reallocation complete */\
              if (mm_v[11])\
              {\
                mm_v[11]=em_write_buffer_3b (MM_V_11, mm_data->reg.tmsi);\
              }  /* mm_v[11] */

#define EM_IMSI_DETACH \
  /* IMSI detach */\
  if (mm_v[12])\
  { \
    mm_v[12] = em_write_buffer_2 (MM_V_12);\
  } /* mm_v[12] */

#define EM_IDENTITY_REQUEST_RESPONSE \
          /* Identity request/response */\
          if (mm_v[13])\
          {\
            switch (ident_req->ident.ident_type)\
            {\
                case ID_TYPE_IMSI:\
                case ID_TYPE_IMEI:\
                case ID_TYPE_IMEISV:  /*stores ident_type and IMSI|IMEI|IMEISV*/\
                  mm_v[13]=em_write_buffer_4b(MM_V_13, ident_req->ident.ident_type, ident_res->mob_id.ident_dig);\
                  break;\
                case ID_TYPE_TMSI:  /*stores ident_type and TMSI*/\
                  mm_v[13]=em_write_buffer_4c(MM_V_13, ident_req->ident.ident_type, mm_data->reg.tmsi);\
                  break;\
                default: /* Illegal mobile identity */\
                  break;\
            }/*switch*/\
          }  /* mm_v[13] */

#define EM_AUTHENTICATION(x) \
        /* Authentication reject*/\
        if (mm_v[14])\
        {\
          mm_v[14] = em_write_buffer_3 (MM_V_14, x);\
        }  /* mm_v[14] */

#define EM_CM_SERVICE_REQUESTED \
        /* CM service requested */\
        if (mm_v[15])\
        {\
          mm_v[15] = em_write_buffer_3 (MM_V_15, mm_data->pend_conn.comp);\
        } /* mm_v[15] */

#define EM_CM_SERVICE_ACCEPTED(x) \
          /* CM service accepted */\
          if (mm_v[16])\
          {\
            mm_v[16] = em_write_buffer_3 (MM_V_16, x);\
          }  /* mm_v[16] */

#define EM_CM_SERVICE_REJECT \
      /* CM service reject */\
      if (mm_v[17])\
      {\
        mm_v[17] = em_write_buffer_3a (MM_V_17, cm_serv_rej->rej_cause);\
      }  /* mm_v[17] */

#define EM_SERVICE_ABORTED \
        /* CM service aborted */\
        if (mm_v[18])\
        {\
         mm_v[18] = em_write_buffer_2 (MM_V_18);\
        }  /* mm_v[18] */

#else /*FF_EM_MODE not defined*/

  /*Macros will stay empty, if no EM */
#define EM_SIM_INSERT                                   /* Event 1*/
#define EM_SIM_REMOVE                                   /* Event 2*/
#define EM_SET_PLMN_SEARCH_MODE                         /* Event 3*/
#define EM_START_REGISTRATION_AUTO_MODE                 /* Event 4*/
#define EM_START_REGISTRATION_MANUAL_MODE               /* Event 4*/
#define EM_START_PLMN_LIST_REQUEST                      /* Event 5*/
#define EM_CELL_SELECTION_RESELECTION                   /* Event 6*/
#define EM_RESULT_PLMN_LIST                             /* Event 7*/
#define EM_RR_CONECTION_REQUESTED                       /* Event 8*/
#define EM_RR_CONECTION_ESTABLISHED                     /* Event 9*/
#define EM_RR_CONNECTION_ESTABLISHED_2                  /* Event 9*/
#define EM_LOCATION_UPDATING                            /* Event 10*/
#define EM_LOCATION_UPDATING_REJECT                     /* Event 10*/
#define EM_TMSI_REALLOCATION_COMPLETE                   /* Event 11*/
#define EM_IMSI_DETACH                                  /* Event 12*/
#define EM_IDENTITY_REQUEST_RESPONSE                    /* Event 13*/
#define EM_AUTHENTICATION(x)                            /* Event 14*/
#define EM_CM_SERVICE_REQUESTED                         /* Event 15*/
#define EM_CM_SERVICE_ACCEPTED(x)                       /* Event 16*/
#define EM_CM_SERVICE_REJECT                            /* Event 17*/
#define EM_SERVICE_ABORTED                              /* Event 18*/

#endif  /* FF_EM_MODE */
#endif /* MM_EM_H */
