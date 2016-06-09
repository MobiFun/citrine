/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (8410)
|  Modul   :  MM
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
|  Purpose :  Definitions for the Protocol Stack Entity
|             Radio Resource
+----------------------------------------------------------------------------- 
*/ 

#ifndef MM_H
#define MM_H

#ifdef  GPRS
#include  "mm_gprs.h"
#endif  /* GPRS */
#if defined (TI_PS_HCOMM_CHANGE)
#include "cl_hComm_handle.h"
#endif

#ifdef TI_GSP_STR2IND_S2I_STRING
typedef unsigned int T_S2I_STRING;
#else
typedef char * T_S2I_STRING;
#define S2I_STRING(x) (x)
#endif

#ifdef TI_PS_OP_VSI_NO_CALL_ID
#define TIMER_START(C,I,T)         vsi_t_start_nc(I,T)
#define TIMER_PSTART(C,I,T,R)      vsi_t_pstart_nc(I,T,R)
#define TIMER_STOP(C,I)            vsi_t_stop_nc(I)
#define TIMER_STATUS(C,I,T)        vsi_t_status_nc(I,T)
#define SUSPEND_SELF(C,T)          vsi_t_sleep_nc(T)
#define SYSTEM_TIME(C,T)           vsi_t_time_nc(T)               
#else /* TI_PS_OP_VSI_NO_CALL_ID */
#define TIMER_START(C,I,T)         vsi_t_start(C,I,T)
#define TIMER_PSTART(C,I,T,R)      vsi_t_pstart(C,I,T,R)
#define TIMER_STOP(C,I)            vsi_t_stop(C,I)
#define TIMER_STATUS(C,I,T)        vsi_t_status(C,I,T)
#define SUSPEND_SELF(C,T)          vsi_t_sleep(C,T)
#define SYSTEM_TIME(C,T)           vsi_t_time(C,T)               
#endif /* TI_PS_OP_VSI_NO_CALL_ID */

#if !defined(_SIMULATION_) && defined(WIN32)
#define _SIMULATION_
#endif  /* !_SIMULATION_ && WIN32 */


/*==== TEST =====================================================*/

/*
 * Dynamic Configuration Numbers
 */
#define TIMER_SET                 1
#define TIMER_RESET               2
#define TIMER_SPEED_UP            3
#define TIMER_SLOW_DOWN           4
#define TIMER_SUPPRESS            5
#define T3212_CNT                 6
#define USE_STORED_BCCH           7
#define FFS_READ_EPLMN            8
#define FFS_WRITE_EPLMN           9
#define FFS_RESET_EPLMN          10
#define FFS_READ_EPLMN_INIT      11


#define ENTITY_DATA               mm_data

/*
 * PLMN constants
 */
#define MAX_FORB_PLMN_ID          8 /* Arbitrary, >= MAX_SIM_FORB_PLMN_ID */
#define MAX_SIM_FORB_PLMN_ID      4
#define UBYTES_PER_PLMN           3 /* Number of UBYTEs for packed PLMN */

#ifdef REL99
#define UBYTES_PER_PLMN_WITH_ACC_TECH 5 /* Number of UBYTEs for packed PLMN with access technology*/
#endif

#ifdef GPRS
#define MAX_GPRS_FORB_PLMN_ID     4
#endif /* #ifdef GPRS */

#define MAX_PREF_PLMN_ID          (MAX_PREF_PLMN/UBYTES_PER_PLMN)

#define SIZE_LOC_INFO             11
#define PD_MM_TI_0                PD_MM
#ifdef REL99
#define MAX_DEFINED_CAUSES        23
#else
#define MAX_DEFINED_CAUSES        22
#endif
#define TMSI_INVALID_VALUE        0xFFFFFFFFL
#define LAC_INVALID_VALUE         0xFFFE

/* 
 * initial HPLMN search after switch on delayed for ms
 */
#define HPLMN_INITIAL_DELAY       120000   /* 2 minutes */
 
/* 
 * HPLMN search constants in steps of 6 minutes
 */
#define HPLMN_REARM_DELAY         1   /* 6 minutes */
#define HPLMN_DEF_SEARCH_PERIOD   10  /* 1 hour */
#define HPLMN_MAX_SEARCH_PERIOD   80  /* 8 hours */

/*
 * Flags for creating a plmn list
 */
#define WITH_OTHER_PLMNS          0 /* Non-forbidden, RPLMN no or low prio */
#define WITH_ALL_PLMNS            1 /* All available PLMNs */
#define WITH_RPLMN                2 /* Non-forbidden */

/*
 * Timer Identifier
 */
#define T_REGISTRATION 0
#define T3210          1
#define T3211          2
#define T3212          3
#define T3213          4
#define T3220          5
#define T3230          6
#define T3240          7
#define T_HPLMN        8
#ifdef REL99
#define T3241          9
#define NUM_OF_MM_TIMERS         10
#else
#define NUM_OF_MM_TIMERS          9
#endif

#define MAX_REST_T3213         2


/*
 * CM_SERVICE_TYPES
 */
#define NO_CM_SERVICE             0
#define CALL_SERVICE              ST_MOC
#define EMERGENCY_SERVICE         ST_EMERGENCY
#define SMS_SERVICE               ST_SMS
#define SS_SERVICE                ST_SS


/*
 * MM Cause
 */
#define MM_CAUSE                  MMCM_MM_CAUSE



/*
 * COMPONENTS
 */
#define CC_COMP                   MMCM_ORG_ENTITY_CC
#define SS_COMP                   MMCM_ORG_ENTITY_SS
#define SMS_COMP                  MMCM_ORG_ENTITY_SMS

#define REG_COMP                  3
#define TIMEOUT                   4

/*
 * FAILURES
 */
#define OPTIONAL_INFO_ERROR       102

/*
 * LOC-UPD-TYPES
 */
#define NORMAL_LUP                0
#define PERIODIC_LUP              1
#define IMSI_ATTACH_LUP           2
#define NOT_RUNNING               3

/* 
 * MISC.CONSTANTS FOR QEUEING
 */
#define ENTRY_FREE                0
#define USE                       1
#define STORED                    2

/* 
 * Constants describing the stored information type
 */
#define NO_ENTRY                  0
#define PRIMITIVE_ENTRY           1
#define EVENT_ENTRY               2

/*
 * UPDATE-STATUS
 */
#define MS_UPDATED                0
#define MS_NOT_UPDATED            1
#define MS_LA_NOT_ALLOWED         3


typedef enum
{
  MSG_RR_ACT,
  MSG_MM_REE,
  MSG_MM_CIPH,
  MSG_MM_MODE
}T_MSG_TYPE;

typedef enum
{
  PRIM_EST_IND,
  PRIM_DATA_IND
}T_PRIM_TYPE;


/*
 * Macros
 */
#ifdef FRAME_OFFSET_ZERO

#define GET_PD(s,p)               p=s.buf[3] & 0x0F
#define GET_TI(s,t)               t=(s.buf[3] & 0xF0)>>4

#else

#define GET_PD(s,p)               ccd_decodeByte(s.buf, (USHORT)(s.o_buf+4), 4, &p)
#define GET_TI(s,t)               ccd_decodeByte(s.buf, s.o_buf, 4, &t)

#endif

#define MSG(TYPE)                 ((T_##TYPE *)(_decodedMsg))
#define USE_STORED_ENTRIES()      { ENTITY_DATA->use_stored_entries = TRUE; }

/*
 * The assert() macro as defined by the frame stops the task, 
 * not only in simulation but also on the target. For the 
 * simulation this is a desired behaviour, for the target it is not. 
 */
#ifndef WIN32
#undef assert
#define assert(x) if (!(x)) { TRACE_ERROR ("Assertion failed"); }
#endif

/*
 * Bitoffset for encoding/decoding
 */
#define ENCODE_OFFSET             24

/*
 * a modified version of the S/GET_STATE macros for tracing CM_INST states
 */

#define CMSTATE(PROCESS,N)               ENTITY_DATA->conn_state[(PROCESS)][(N)]

#ifdef NTRACE
  #define CM_SET_STATE(PROCESS,N,STATE)  CMSTATE((PROCESS),(N)) = (STATE)
  #define CM_GET_STATE(PROCESS,N)        CMSTATE((PROCESS),(N))
#else
#if defined (NEW_FRAME)
  #define CM_SET_STATE(PROCESS,N,STATE)\
    {\
      vsi_o_strace (VSI_CALLER\
                    PROCESS_NAME_CM[(PROCESS)],\
                    STATE_CM_NAME  [(PROCESS)] [ CMSTATE(PROCESS,N) ],\
                    STATE_CM_NAME  [(PROCESS)] [  (STATE)           ]);\
      CMSTATE((PROCESS),(N)) = (STATE);\
    }

  #define CM_GET_STATE(PROCESS,N)\
    (vsi_o_strace (VSI_CALLER\
                   PROCESS_NAME_CM[(PROCESS)],\
                   STATE_CM_NAME  [(PROCESS)] [ CMSTATE((PROCESS),(N)) ],\
                   NULL), CMSTATE((PROCESS),(N)))
#else
  #define CM_SET_STATE(PROCESS,N,STATE)\
               CMSTATE((PROCESS),(N)) =\
                vsi_o_strace (VSI_CALLER\
                              PROCESS_NAME_CM[(PROCESS)],\
                              STATE_CM_NAME  [(PROCESS)] [ CMSTATE(PROCESS,N) ],\
                              STATE_CM_NAME  [(PROCESS)] [  (STATE)           ],\
                                                            (STATE)              )
  #define CM_GET_STATE(PROCESS,N)\
                vsi_o_strace (VSI_CALLER\
                              PROCESS_NAME_CM[(PROCESS)],\
                              STATE_CM_NAME  [(PROCESS)] [ CMSTATE((PROCESS),(N)) ],\
                                                              NULL                 ,\
                                                           CMSTATE((PROCESS),(N))    )
#endif /* NEW_FRAME */
#endif /* NTRACE    */

/*
 * buffer sizes
 */
#define SIZE_BCCH                 16
#define NUM_OF_CM_ENT              3
#define NUM_OF_CONN_PER_CM_ENT    16

/*
 * CONNECTION STATES
 */
#define CM_IDLE                        0
#define CM_PENDING                     1
#define CM_ACTIVE                      2
#define CM_REEST_PENDING               3
#define CM_STORE                       4
#define CM_NOT_IDLE                    0xff

EXTERN  const char * const * const STATE_CM_NAME[];

#define STATE_CM_CC_NAME_INIT             \
       "CM_CC_IDLE",                      \
       "CM_CC_PENDING",                   \
       "CM_CC_ACTIVE",                    \
       "CM_CC_REEST_PENDING",             \
       "CM_CC_STORE"

#define STATE_CM_SS_NAME_INIT             \
       "CM_SS_IDLE",                      \
       "CM_SS_PENDING",                   \
       "CM_SS_ACTIVE",                    \
       "CM_SS_REEST_PENDING",             \
       "CM_SS_STORE"

#define STATE_CM_SMS_NAME_INIT             \
       "CM_SMS_IDLE",                      \
       "CM_SMS_PENDING",                   \
       "CM_SMS_ACTIVE",                    \
       "CM_SMS_REEST_PENDING",             \
       "CM_SMS_STORE"

/*
 * CM manages 3 (NUM_OF_CM_ENT) states: CC, SS, SMS
 */
#define STATE_CC                       CC_COMP
#define STATE_SS                       SS_COMP
#define STATE_SMS                      SMS_COMP

#define UNSPEC                         0
#define CM_LUP_TRIGGER                 1

EXTERN  const char * const
        PROCESS_NAME_CM[];
#define PROCESS_NAME_CM_INIT           \
       "STATE_CC"   ,                  \
       "STATE_SS"   ,                  \
       "STATE_SMS"

/*
 * A definition of the states of CMSTATE is provided in MM_PEI.C
 */
#if defined(MM_PEI_C) && ! defined(NTRACE)
  LOCAL  const char * const          STATE_CC_NAME [] = { STATE_CM_CC_NAME_INIT  };
  LOCAL  const char * const          STATE_SS_NAME [] = { STATE_CM_SS_NAME_INIT  };
  LOCAL  const char * const          STATE_SMS_NAME[] = { STATE_CM_SMS_NAME_INIT };
  GLOBAL const char * const  * const STATE_CM_NAME [] = { STATE_CC_NAME,
                                                          STATE_SS_NAME,
                                                          STATE_SMS_NAME };
  GLOBAL const char * const PROCESS_NAME_CM[] = { PROCESS_NAME_CM_INIT };

#endif

/*
 * States of the Mobility Management
 */
#define MM_NULL                       0
#define MM_LUP_INITIATED              3
#define MM_WAIT_FOR_OUTG_MM_CONN      5
#define MM_CONN_ACTIVE                6
#define MM_IMSI_DETACH_INIT           7
#define MM_PROCESS_PROMPT             8
#define MM_WAIT_FOR_NW_CMD            9
#define MM_LUP_REJECTED               10
#define MM_WAIT_FOR_RR_CONN_LUP       13
#define MM_WAIT_FOR_RR_CONN_MM        14
#define MM_WAIT_FOR_RR_CONN_DETACH    15
#define MM_WAIT_FOR_REESTABLISH       17
#define MM_WAIT_FOR_RR_ACTIVE         18
#define MM_IDLE_NORMAL_SERVICE        19 /* 19.1 */
#define MM_LOCATION_UPDATING_PENDING  23 /* Not defined for GSM only stack */
#define MM_IMSI_DETACH_PENDING        24 /* Not defined for GSM only stack */
#ifdef REL99
#define MM_RR_CONN_RELEASE_NOT_ALLOWED 25
#define MM_IDLE_ATTEMPT_TO_UPDATE     26 // 19.2 
#define MM_IDLE_LIMITED_SERVICE       27 // 19.3 
#define MM_IDLE_NO_IMSI               28 // 19.4 
#define MM_IDLE_NO_CELL_AVAILABLE     29 // 19.5 
#define MM_IDLE_LUP_NEEDED            30 // 19.6, not used by GSM only stack 
#define MM_IDLE_PLMN_SEARCH           31 // 19.7 
#define MM_PLMN_SEARCH_NORMAL_SERVICE 32 // 19.8 
#else
#define MM_IDLE_ATTEMPT_TO_UPDATE     25 // 19.2 
#define MM_IDLE_LIMITED_SERVICE       26 // 19.3 
#define MM_IDLE_NO_IMSI               27 // 19.4 
#define MM_IDLE_NO_CELL_AVAILABLE     28 // 19.5 
#define MM_IDLE_LUP_NEEDED            29 // 19.6, not used by GSM only stack 
#define MM_IDLE_PLMN_SEARCH           30 // 19.7 
#define MM_PLMN_SEARCH_NORMAL_SERVICE 31 // 19.8 
#endif

/* N950 Memory Optimization - Implements Measure #39*/
EXTERN  T_S2I_STRING const STATE_MM_NAME[];

#ifdef GPRS
/* 
 * States of CM establishment if GPRS is present
 */
#define CM_GPRS_EST_IDLE    0    /* No outstanding MMGMM_CM_ESTABLISH_RES */
#define CM_GPRS_EST_PEND    1    /* MMGMM_CM_ESTABLISH_RES outstanding */
#define CM_GPRS_EMERGE_PEND 2    /* MMGMM_CM_EMERGENCY_RES outstanding */
#define CM_GPRS_EST_OK      3    /* MM is allowed to establish RR connection */

/* 
 * States for connection management. You may think of a process 
 * in terms of SDL here, but this is not obvious by looking into the C code, 
 * as the separation of state machines is not elaborated at all.
 */
EXTERN  const char * const STATE_GPRS_CM_EST_NAME[];
#define STATE_GPRS_CM_EST_INIT         \
       "CM_GPRS_EST_IDLE",             \
       "CM_GPRS_EST_PEND",             \
       "CM_GPRS_EMERGE_PEND",          \
       "CM_GPRS_EST_OK"

/* 
 * States for registration control.
 * Don't think of a state and a process in term of SDL here. 
 * The only reason why this is a state and not only a ordinary variable 
 * is because it eases debugging a lot for now.
 */
EXTERN  const char * const STATE_REG_TYPE_NAME[];
#endif /* GPRS */
       
/*
 * MM manages 1 (NUM_OF_MM_STATES) state  (GSM only)    : MM
 * MM manages 3 (NUM_OF_MM_STATES) states (GPRS present): MM, GPRS_CM_EST, REG_TYPE
 */
#ifdef OPTION_MULTITHREAD
  #define PROCESS_NAME    _ENTITY_PREFIXED(PROCESS_NAME)
#endif

EXTERN  const char * const PROCESS_NAME[];
#ifdef GPRS
#define STATE_MM                       0
#define STATE_GPRS_CM_EST              1
#define STATE_REG_TYPE                 2
#define NUM_OF_MM_STATES               3
#define PROCESS_NAME_INIT              \
       "MM",                           \
       "GPRS_CM_EST",                  \
       "REG_TYPE"
#else
#define STATE_MM                       0
#define NUM_OF_MM_STATES               1
#define PROCESS_NAME_INIT              \
       "MM"
#endif /* GPRS */

/*
 * A definition of the states is provided in MM_PEI.C
 */
#if defined(MM_PEI_C) && ! defined(NTRACE)
/* N950 Memory Optimization - Implements Measure #39*/
GLOBAL T_S2I_STRING const STATE_MM_NAME[]     =
{
  S2I_STRING("MM_NULL"),                      
  S2I_STRING("MM_1_INVALID"),                 
  S2I_STRING("MM_2_INVALID"),                 
  S2I_STRING("MM_LUP_INITIATED"),             
  S2I_STRING("MM_4_INVALID"),                 
  S2I_STRING("MM_WAIT_FOR_OUTG_MM_CONN"),     
  S2I_STRING("MM_CONN_ACTIVE"),               
  S2I_STRING("MM_IMSI_DETACH_INIT"),          
  S2I_STRING("MM_PROCESS_PROMPT"),            
  S2I_STRING("MM_WAIT_FOR_NW_CMD"),           
  S2I_STRING("MM_LUP_REJECTED"),              
  S2I_STRING("MM_11_INVALID"),                
  S2I_STRING("MM_12_INVALID"),                
  S2I_STRING("MM_WAIT_FOR_RR_CONN_LUP"),      
  S2I_STRING("MM_WAIT_FOR_RR_CONN_MM"),       
  S2I_STRING("MM_WAIT_FOR_RR_CONN_DETACH"),   
  S2I_STRING("MM_16_INVALID"),                
  S2I_STRING("MM_WAIT_FOR_REESTABLISH"),      
  S2I_STRING("MM_WAIT_FOR_RR_ACTIVE"),        
  S2I_STRING("MM_IDLE_NORMAL_SERVICE"),       
  S2I_STRING("MM_20_INVALID"),                
  S2I_STRING("MM_21_INVALID"),                
  S2I_STRING("MM_22_INVALID"),                
  S2I_STRING("MM_LOCATION_UPDATING_PENDING"), 
  S2I_STRING("MM_IMSI_DETACH_PENDING"),       
#ifdef REL99
  S2I_STRING("MM_RR_CONN_RELEASE_NOT_ALLOWED"),
#endif
  S2I_STRING("MM_IDLE_ATTEMPT_TO_UPDATE"),    
  S2I_STRING("MM_IDLE_LIMITED_SERVICE"),      
  S2I_STRING("MM_IDLE_NO_IMSI"),              
  S2I_STRING("MM_IDLE_NO_CELL_AVAILABLE"),    
  S2I_STRING("MM_IDLE_LUP_NEEDED"),           
  S2I_STRING("MM_IDLE_PLMN_SEARCH"),          
  S2I_STRING("MM_PLMN_SEARCH_NORMAL_SERVICE")
};
#ifdef GPRS
  
  GLOBAL const char * const STATE_GPRS_CM_EST_NAME[] = 
    { STATE_GPRS_CM_EST_INIT };

  GLOBAL const char * const STATE_REG_TYPE_NAME[] = 
  { "REG_GPRS_INACTIVE",
    "REG_REMOTE_CONTROLLED",
    "REG_CELL_SEARCH_ONLY" 
  };

#endif /* GPRS */
  GLOBAL const char * const   PROCESS_NAME[]    = { PROCESS_NAME_INIT };
#endif /* defined(MM_PEI_C) && ! defined(NTRACE) */

#ifndef NTRACE
#define UNEXPECTED_IN_STATE  "Unexpected in state"
#define UNEXPECTED_DEFAULT   "Unexpected default"
#define UNEXPECTED_PARAMETER "Unexpected parameter"
#define PRIMITIVE_IGNORED    "Primitive ignored"
#endif
  
/* 
 * State of elementary file after SIM_FILE_CHANGE_IND 
 */
#define SAT_UNCHANGED 0 /* SAT didn't change the file */
#define SAT_READ_FILE 1 /* SAT changed file, still to read */
#define SAT_PEND_CNF  2 /* SIM_READ_REQ underway for elementary file */    
#define SAT_PEND_ACT  3 /* File read, but not yet activated */

/* 
 * Get registration type. Use this macro to make it easy for the optimizer 
 * to optimize some expressions away if compiling a GSM only protocol stack.
 * These macros help to avoid making an #ifdef grave from MM.
 */
#ifdef  GPRS
#define mm_gsm_alone() (GET_STATE (STATE_REG_TYPE) EQ REG_GPRS_INACTIVE)
#define mm_cm_est_allowed()  (GET_STATE (STATE_GPRS_CM_EST) EQ CM_GPRS_EST_OK)
#define mm_lup_allowed_by_gmm() (GET_STATE (STATE_REG_TYPE) NEQ REG_CELL_SEARCH_ONLY)
#else
#define mm_gsm_alone()          TRUE
#define mm_cm_est_allowed()     TRUE
#define mm_lup_allowed_by_gmm() TRUE
#endif /* GPRS */

#define EPLMNLIST_SIZE       6

/******************Specially for Cingular********************/
#define MAX_CINGULAR_PLMN       14
  
/*==== TYPES ======================================================*/

typedef struct
{
  /* IMSI EPLMNList was received with */
  T_imsi_struct eqv_plmn_imsi;

  /* Contains a list of Equivalent PLMNs provided by the network. The list is 
   * replaced or deleted at the end of each location update procedure, routing 
   * area update procedure and GPRS attach procedure. */
  UBYTE         eqv_plmn_list[EPLMNLIST_SIZE*UBYTES_PER_PLMN];
} T_ffs_eplmn_imsi;


typedef struct
{
  /* Access class for RR from the SIM */
  USHORT                acc_class;
  
  /* PLMN on which MM actually tries to register */
  T_plmn                actual_plmn;

  /* Room for BCCH information from the SIM/RR. 
   * The better way would be to define this as T_bcch_info */
  UBYTE                 bcch[SIZE_BCCH];

  /* TRUE if the BCCH information shall be used. The sense of this variable
   * is questionable */
  BOOL                  bcch_encode;

  /* Cell test mode, possible values are 
   * CELL_TEST_ENABLE and CELL_TEST_DISABLE */
  UBYTE                 cell_test;

  /* The ciphering key sequence number */
  UBYTE                 cksn;

  /* The list of forbidden PLMNs, copy of SIM data */
  UBYTE                 forb_plmn[MAX_FORB_PLMN_ID * UBYTES_PER_PLMN];

#ifdef GPRS
  /* The list of forbidden PLMNs for GPRS services */
  UBYTE                 gprs_forb_plmn[MAX_GPRS_FORB_PLMN_ID * UBYTES_PER_PLMN];
#endif /* #ifdef GPRS */

  /* The PLMN which sent recently a LUP reject with cause #13 */
  T_plmn                low_prio_plmn;

  /* The IMSI, copy of SIM data */
  T_imsi_struct         imsi_struct;

  /* The ciphering key */
  UBYTE                 kc[MAX_KC];

  /* The location area where the mobile currently is registered */
  T_loc_area_ident      lai;

  /* Operation mode. Valid flag v_op set to valid after first reg. attempt 
   * Also contains information whether a sim and a test sim is inserted. */
  T_op                  op;

  /* Length of MNC in IMSI.SIM should send the length of mnc and MM should use
   * this value to see if third digit of MNC is valid or not
  */
  U8                    length_mnc;

  /* The found PLMNs during network search */
  UBYTE                 plmn[MAX_PLMN_ID * UBYTES_PER_PLMN];

  /* The LACs of the found PLMNs */
  USHORT                 plmn_lac [MAX_PLMN_ID];

  /* The reception levels of the found PLMNs */
  UBYTE                 plmn_rx [MAX_PLMN_ID];

#ifdef GPRS
  /* The GPRS capabilities of the found PLMNs */
  UBYTE                 gprs_status[MAX_PLMN_ID];

  /* The indicated value of the TMSI */
  ULONG                 indicated_tmsi;
#endif /* #ifdef GPRS */

  /* Number of found PLMN */
  UBYTE                 plmn_cnt;
  
  /* Index of currently checked PLMN in automatic mode */
  UBYTE                 plmn_index;

  /* Preferred PLMN list */
  UBYTE                 pref_plmn[MAX_PREF_PLMN_ID * UBYTES_PER_PLMN];

#ifdef REL99

  /*
   * Actual length of User controlled PLMN Selector with Access Technology (EFPLMNwAcT)
   * as indicated in sim_mm_insert_ind
   */
  USHORT                 sim_ucps_at_len;
  /*
   * Actual length of Operator controlled PLMN Selector with Access Technology (EFOPLMNwAcT)
   * as indicated in sim_mm_insert_ind
   */
  USHORT                 sim_ocps_at_len;

  /* 
   * Flag indicates which SIM EFs are being used for pref_plmn
   * TRUE: Indicated SIM EFs EFPLMNwAcT and EFOPLMNwAcT are being used.
   * FALSE: Indicated SIM EF EFPLMNsel is being used.
   */
  BOOL                  sim_uocps_at_used;
#endif
  
  /*Actual length of PLMN Selector (EFPLMNsel) as indicated in sim_mm_insert_ind*/
  USHORT                 sim_plmnsel_len;
  
  /* 
   * Flag indicates if the SIM INSERT INDICATION is in progress i.e. after SIM_MM_INSERT_IND,
   * MM is reading SIM files.
   * TRUE: SIM MM insert indication and Reading of files is in progress
   */
  BOOL                  sim_read_in_progress;
  
  /*
   * This variable will be used to decide if SIM_SYNC_REQ should be sent or not.
   * After receiving SIM_MM_INSERT_IND, MM should respond with a SIM_SYNC_REQ.
   * This SIM_SYNC_REQ will be sent after reading either EFPLMNSel or EFPLMNwAct
   * and EFOPLMNwAct
   */
  BOOL                   sim_sync_req_pending;

  /*SET true is sim file update indication is received*/
  BOOL                  sim_file_upd_ind_rec;

  /* HPLMN search period, this is forwarded to RR and not evaluated by MM */
  UBYTE                 thplmn;
  
  /* TMSI, copy of the SIM data */
  ULONG                 tmsi;

  /* The update state as it is on the SIM. Possible values are 
   * MS_UPDATED, MS_NOT_UPDATED and MS_LA_NOT_ALLOWED */
  UBYTE                 update_stat;

  /* MMXXX_REG_CNF sent to MMI only if this has not already been done 
   * after last registration attempt / last loss of service or 
   * the cell has changed. */
  BOOL                  full_service_indicated;

  /* New cell from RR after last MMXXX_REG_CNF */
  BOOL                  new_cell_ind;

  /* Access class updated by SAT, update state */
  UBYTE                 upd_sim_acc;

  /* Preferred PLMN list updated by SAT, update state */
  UBYTE                 upd_sim_plmnsel;
  
  /* HPLMN search period updated by SAT, update state */
  UBYTE                 upd_sim_hplmn;
  
  /* Forbidden PLMN list updated by SAT, update state */
  UBYTE                 upd_sim_fplmn;

#ifdef REL99

  /* User controlled PLMN Selector with Access Technology update state */
  UBYTE                 upd_sim_ucps_at;

  /* User controlled PLMN Selector with Access Technology update state */
  UBYTE                 upd_sim_ocps_at;

#endif

  /* Acting HPLMN value updated by SAT, update state */
  UBYTE                 upd_sim_act_hplmn;

  /* If NEQ NULL, MM is performing the MM Restart procedure */
  T_SIM_MM_INSERT_IND   *sim_insert_info;

  /* Contains a list of Equivalent PLMNs provided by the network. The list is 
   * replaced or deleted at the end of each location update procedure, routing 
   * area update procedure and GPRS attach procedure. */
  //UBYTE                 eqv_plmn_list[EPLMNLIST_SIZE*UBYTES_PER_PLMN];

  /*
   * EPLMN List and IMSI store in Non-volatile memory
   */
  T_ffs_eplmn_imsi      eqv_plmns;

  /* If sim supports AHPLMN and the file has valid value acting_hplmn.v_plmn is TRUE and the 
   * actual value of AHPLMN will be contained in acting_hplmn. If sim doesnt support this
   * feature or the value for AHPLMN is invalid acting_hplmn.v_plmn is FALSE and acting_hplmn
   * contains 0xff 0xff 0xff
   */

  T_plmn                acting_hplmn;

  /* If the value read for AHPLMN is FFFFFF then acting_hplmn_invalid will be 
   * true.This will only be used if AHPLMN is modified via OTA
   */  
  BOOL                  acting_hplmn_invalid;

  /* If the PLMN is registered to AHPLMN at the time of REFRESH command by SMS, 
   * then this flag should be set to true.
   */
  BOOL                  reg_plmn_equal_ahplmn;  

  /* This flag shall be used to indicate Cingular Network */
  BOOL                  is_cingular_sim;

  /* This flag enables a quick search when coming back from VPLMN to HPLMN and RR_ABORT_IND gives 
     HPLMN as a part of its List*/
  BOOL                  quick_hplmn_search;

} T_REG;

/*
This struct is used to store timer events and establishment requests
*/
typedef struct
{
  UBYTE  comp;
  UBYTE  ti;
  USHORT estcs;
} T_STORE_1; 

typedef union 
{
  T_STORE_1       event;
  T_PRIM_HEADER*  primitive;
} T_STORE_MM;

typedef struct
{
  UBYTE           use;
  U8              content_type;
  U8              info;
  T_STORE_MM      content;
} T_STORE;

typedef struct
{
  UBYTE  comp;
  UBYTE  ti;
  USHORT cause;
  UBYTE  service;
} T_PEND;

typedef struct
{
  UBYTE state;
} T_CONN;

typedef struct
{
  /* The BCCH information as delivered by the cell
   * The better way would be to define this as T_bcch_info */
  UBYTE                 bcch[SIZE_BCCH];
  
  /* Location Area Identity of last/current selected cell,
   * information will not be invalidated if coverage lost
   * until new cell is selected. */
  T_loc_area_ident      lai;
  
  /* Cell ID of last/current selected cell, 
   * information will not be invalidated if coverage lost
   * until new cell is selected. */
  USHORT                cid;

  /* Information broadcast by cell on BCCH */
  T_mm_info             mm_info;

#ifdef GPRS
  /* Information braodcast on the BCCH whether the cell supports GPRS */
  UBYTE                 gprs_indication;
#endif /* GPRS */
} T_MM;


#if defined (OPTION_TIMER)
typedef struct
{
  UBYTE                 t_mode;
  ULONG                 t_val;
} T_TIMER_CONFIG;
#endif /* #if defined (OPTION_TIMER) */



#ifdef GPRS

/* 
 * MM data solely used if GPRS is also delivered
 */
typedef struct
{
  /* TRUE if MM has to send MMGMM_REG_CNF after IDLE mode entry */
  BOOL reg_cnf_on_idle_entry;

  /* TRUE if SIM currently physically removed and SIM data not yet deleted.
   * This variable is only used if GSM is not alone, otherwise MM can perform
   * the necessary actions (e.g. IMSI detach) alone. */
  BOOL sim_physically_removed;

  /* TRUE if the MS is in a location area where combined attach is 
   * required (network mode I) and therefore MM shall not start T3212. */
  BOOL combined_procedures;

  /* The currently selected mobile class. Needed for reject cause #14. */
  UBYTE mobile_class;

  /* The current GPRS resumption state. Defaults to MMGMM_RESUMPTION_FAILURE
   * excepts if processing RR_RELEASE_IND, in this case it is set according
   * to the resumption state delivered by RR_RELEASE_IND. */
  UBYTE resumption;

} T_GPRS_DATA;
#endif /* GPRS */


typedef struct
{
  /* The variable ciphering_on is a write-only variable, not to be eliminated.
   * It contains the state of ciphering on the air interface (on or off). */
  BOOL                  ciphering_on;
  
  /* The connection table. NUM_OF_CM_ENT equals to the number of CM entities, 
   * while NUM_OF_CONN_PER_CM_ENT equals to the number of parallel 
   * transactions each entitity can have */
  UBYTE                 conn_state[NUM_OF_CM_ENT][NUM_OF_CONN_PER_CM_ENT];

  /* May be zero, OPTIONAL_INFO_ERROR or RC_INVALID_MAND_MESSAGE.
   * Related to the decoding of messages */
  UBYTE                 error;

  /* This variable is TRUE until either the first location updating
   * procedure has ended either with an invalidation of the registration 
   * state or the first successful location updating. This variable should 
   * go into the registration data, but this is really only cosmetic.
   * first_attach_mem holds the state of first attach before it is set to FALSE.
   * It is used to determine if the HPLMN timer has to be started with 3 min for
   * first search of HPLMN or if the normal hplmn timer value from registration 
   * data is to be used. After this check it is reset unconditional.
   */
  BOOL                  first_attach;
  BOOL                  first_attach_mem;
  
  /* RR release cause which leaded to idle entry after failed LUP. In case 
   * this is RELCS_ACCESS_BARRED or RELCS_RND_ACC_DELAY, MM is someway in 
   * state MM_IDLE_LUP_NEEDED, but this state is not used for this purpose
   * in MM. A location updating attempt will be performed if RR informs MM 
   * by using RR_SYNC_IND with an appropriate cause. */
  USHORT                idle_entry;

  /* Contains the type of the location update in progress and the state of
   * the follow on request flag of this location update. */
  T_loc_upd_type        loc_upd_type;

  /* Contains relevant data delivered by RR about the selected cell. 
   * A better name for this type instead T_MM may be T_CELL_DATA, 
   * for the variable itself cell_data */
  T_MM                  mm;
  
  /* The state of the MM state machines, the MM main state machine and
   * the registration state machine. */
  UBYTE                 state[NUM_OF_MM_STATES];
  
  // TO_BE_DONE - write a good comment
  UBYTE                 nreg_cause;
  
  /* The variable nreg_request is TRUE if the deregistration was requested by 
   * MMR_NREG_REQ, otherwise FAISE. This is needed to decide whether MM 
   * will send an MMR_NREG_IND or MMR_NREG_CNF primitive at the end of the 
   * deregistration process. */
  BOOL                  nreg_request;
  
  /* All necessary information which is required for a pending connection 
   * is stored here. Note: There can be not more than one pending connection 
   * at a given time. */
  T_PEND                pend_conn;
  
  /* The base for the internal random number generator, used to set the value
   * for T3212 under certain cicumstances. */
  ULONG                 rand_base;
  
  /* All the data related to registration and SIM. */
  T_REG                 reg;
  
  /* In this location timer expiries and CM connection requests can be stored 
   * for later usage after a state change of one of the MM state machines */
  T_STORE               store[MAX_STORE_ENTRIES];

  /* Use stored entries after end of transition */
  BOOL                  use_stored_entries;

#if defined (NEW_FRAME)
  /* TRUE if the appropriate timer is active */
  BOOL                  t_running[NUM_OF_MM_TIMERS];
#else
  /* The handles for all the MM timers. */
  T_VSI_THANDLE         t_handle[NUM_OF_MM_TIMERS];
#endif

  /* This variable is a perfectly good substitute for missing MM state 
   * WAIT_FOR_ADDITIONAL_OUTGOING MM connection (#20). If in state 
   * MM_CONNECTION_ACTIVE (#6) and this flag is set, 
   * MM is really in state #20. */
  // Identical with (TIMERACTIVE (T3230))
  // Identical with (mm_count_connections (CM_PENDING) NEQ 0)
  // => flag could be eliminated!
  BOOL                  wait_for_accept;

  /* Copy of the power field of the last RR_ACTIVATE_CNF/RR_ACTIVATE_IND */
  UBYTE                 rf_power;
  
  /* The counter for RR_ESTABLISH_REQ retransmissions for one CM connection 
   * attempt if a random access failure occured. */
  UBYTE                 act_retrans;
  
  /* The LOCATION UPDATE attempt counter */
  UBYTE                 attempt_cnt;
  
  /* The variables reject_cause and last_reject_cause store the 
   * current and the previous reject cause during location updating. */
  USHORT                rej_cause;
  USHORT                last_rej_cause;
  
  /* Remember at RR establishment for emergency call and at start of the 
   * location update procedure the type of service state MM had. */
  UBYTE                 idle_substate;

  /* Variable solely used for debugging purposes */
  USHORT                debug_last_rej_cause;

  /* Variable solely used for debugging purposes */
  UBYTE                 mm_idle_no_imsi_marker; 
  
  /* Faked T3212 value for test only, will be used if not equal zero */
  USHORT                t3212_cfg_counter;
  
  /* This variable stores whether a timeout of T3212 has occurred and 
   * a periodic location updating procedure has to be performed ASAP. */
  BOOL                  t3212_timeout;
  
  /* counts, how many times T3213 was started. Referres to the Note in 24.008 ch. 4.4.4.9
   * "the mobile station may as an option delay the repeated attempt for up to 8 seconds 
   *  to allow cell re-selection to take place" ---> 2 times T3213 as delay implemented.
   * 0 -> first run ...
   */
  U8                    t3213_restart;

  UBYTE                 reest_ti;
  
  BOOL                  reest_cell_avail;

  /* LOCATION UPDATING REJECT cause from the network or internal 
   * cause for limited service (e.g. AUTHENTICATION REJECT received).
   * This is for MMR_NREG_IND to inform the MMI and is not intended to
   * be used by MM to control its own operation. */
  USHORT                limited_cause;

  /* A search for available networks was requested by the MMI */
  BOOL                  plmn_scan_mmi;

  /* A search for available networks was requested by MM for own operation */
  BOOL                  plmn_scan_mm;

  /* This boolean variable is used only together with available test SIM.
   * Is it set to a value of TRUE, the MS uses stored BCCH lists also
   * with test SIM. */
  BOOL                  config_use_stored_bcch;

  
 /* This variable is used to prevent an RR_ABORT_IND caused by a RR TABORT timer
  * timeout from interfering with FTA test 26.7.4.3.4 
  */
  BOOL rr_abort_prior_to_tabort;


   /* Used to store the datafield used in the last sent SIM_READ_REQ. Default value will
     be NOT_PRESENT_16BIT, if no read request has been sent to SIM*/
  USHORT               sim_read_req_data_field;

  /* Sometimes the network sends multiple (two) authentication req messages 
   * without waiting for an answer. SIM interface got modified to support 
   * identifiers for requests sent to SIM. The following variable contains the req_id 
   * in the last sent SIM_AUTHENTICATION_REQ.
   * On receiving SIM_AUTHENTICATION_CNF, response is sent to network only
   * if req_id matches the value in this variable. SIM_AUTHENTICATION_CNF with other 
   * req_ids are ignored*/
  UBYTE                last_auth_req_id;


#if defined (FF_EOTD) AND defined (REL99)
  /* This flag is used to indicate if rrlp/lcs procedure is running. This flag is set
   * to true upon reception of  RR_RRLP_START_IND. Flag will be set to false upon
   * reception of RR_RRLP_STOP_IND.
   */
  BOOL rrlp_lcs_started;
#endif /* (FF_EOTD) AND defined (REL99) */
  /* This variable is used to keep count of repetitions for the network search if
     MM receives RR_ABORT_IND with cause RRCS_ABORT_PTM. Depending on its value
     MM behaviour will be decided.
   */
  UBYTE net_search_count;

  
  /* All the data needed for GPRS support */
#ifdef GPRS
  T_GPRS_DATA           gprs;
#endif
  /* This is a bitwise mapping to indicate SIM which EFs are to be updated in SIM_MM_UPDATE_REQ */
  U8                    ef_indicator; 
} T_MM_DATA;

/*==== EXPORT =====================================================*/

#define GET_BITS(value, shift, width) ((value >> shift) & ((1 << width) - 1))

/*
 * Prototypes Timer Modul
 */
/*
 * If all entities are linked into one module this definitions
 * prefixes all this functions with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define tim_t_reg           _ENTITY_PREFIXED(tim_t_reg)
  #define tim_t3210           _ENTITY_PREFIXED(tim_t3210)
  #define tim_t3211           _ENTITY_PREFIXED(tim_t3211)
  #define tim_t3212           _ENTITY_PREFIXED(tim_t3212)
  #define tim_t3213           _ENTITY_PREFIXED(tim_t3213)
  #define tim_t3220           _ENTITY_PREFIXED(tim_t3220)
  #define tim_t3230           _ENTITY_PREFIXED(tim_t3230)
  #define tim_t3240           _ENTITY_PREFIXED(tim_t3240)
#ifdef REL99
  #define tim_t3241           _ENTITY_PREFIXED(tim_t3241)
#endif
  #define tim_t_hplmn         _ENTITY_PREFIXED(tim_t_hplmn)
#endif /* #ifdef OPTION_MULTITHREAD */

#if defined (NEW_FRAME)
#ifdef OPTION_MULTITHREAD
  #define tim_exec_timeout   _ENTITY_PREFIXED(tim_exec_timeout)
  #define tim_start_timer    _ENTITY_PREFIXED(tim_start_timer)
  #define tim_stop_timer     _ENTITY_PREFIXED(tim_stop_timer)
#endif
EXTERN void tim_exec_timeout (USHORT index);
EXTERN void tim_start_timer  (USHORT index, T_TIME value);
EXTERN void tim_stop_timer   (USHORT index);
#else

#ifdef OPTION_MULTITHREAD
  #define tim_set_timeout_flag _ENTITY_PREFIXED(tim_set_timeout_flag)
  #define tim_handle_timeout   _ENTITY_PREFIXED(tim_handle_timeout)
  #define tim_get_config_timer _ENTITY_PREFIXED(tim_get_config_timer)
  #define tim_start_timer      _ENTITY_PREFIXED(tim_start_timer)
  #define tim_flush_fifo       _ENTITY_PREFIXED(tim_flush_fifo)
#endif

EXTERN void tim_set_timeout_flag        (T_VSI_THANDLE       handle,
                                         USHORT              *t_flag);
EXTERN void tim_handle_timeout          (USHORT              *t_flag);
EXTERN void tim_get_config_timer        (char                *outString,
                                         UBYTE               t_num,
                                         KW_DATA             *t_name);
EXTERN void tim_start_timer             (UBYTE               id,
                                         T_VSI_THANDLE       handle,
                                         T_VSI_TVALUE        value);
EXTERN void tim_flush_fifo              (T_VSI_THANDLE       handle);

#endif /* else, #if defined (NEW_FRAME) */

#ifdef OPTION_MULTITHREAD
  #define tim_init_timer       _ENTITY_PREFIXED(tim_init_timer)
  #define tim_config_timer     _ENTITY_PREFIXED(tim_config_timer)
#endif

EXTERN BOOL tim_init_timer              (void);

EXTERN void tim_config_timer            (UBYTE               t_num,
                                         UBYTE               t_mod,
                                         ULONG               t_val);

/*
 *  Prototypes MM
 */

/*
 *  MM Mobility Management
 */

/*
 *  mobility management primitives
 */
EXTERN void mm_init_mm_data           (void);
EXTERN void mm_mdl_error_ind          (T_MDL_ERROR_IND           *mdl_error_ind);
EXTERN void mm_mmcm_data_req          (T_MMCM_DATA_REQ           *mmcm_data_req);
EXTERN void mm_mmcm_establish_req     (T_MMCM_ESTABLISH_REQ      *mmcm_establish_req);
EXTERN void mm_mmcm_reestablish_req   (T_MMCM_REESTABLISH_REQ    *mmcm_reestablish_req);
EXTERN void mm_mmcm_release_req       (T_MMCM_RELEASE_REQ        *mmcm_release_req);
EXTERN void mm_mmcm_prompt_rej        (T_MMCM_PROMPT_REJ         *prompt_rej);
EXTERN void mm_mmcm_prompt_res        (T_MMCM_PROMPT_RES         *prompt_rsp);
EXTERN void mm_mmss_data_req          (T_MMSS_DATA_REQ           *mmss_data_req);
EXTERN void mm_mmss_establish_req     (T_MMSS_ESTABLISH_REQ      *mmss_establish_req);
EXTERN void mm_mmss_release_req       (T_MMSS_RELEASE_REQ        *mmss_release_req);
EXTERN void mm_mmsms_data_req         (T_MMSMS_DATA_REQ          *mmsms_data_req);
EXTERN void mm_mmsms_establish_req    (T_MMSMS_ESTABLISH_REQ     *mmsms_establish_req);
EXTERN void mm_mmsms_release_req      (T_MMSMS_RELEASE_REQ       *mmsms_release_req);
EXTERN void mm_mmxx_establish_req     (UBYTE  comp, UBYTE  ti, USHORT estcs, U8 info);
EXTERN void mm_rr_abort_ind           (T_RR_ABORT_IND            *rr_abort_ind);
EXTERN void mm_rr_activate_cnf        (T_RR_ACTIVATE_CNF         *rr_activate_cnf);
EXTERN void mm_rr_activate_ind        (T_RR_ACTIVATE_IND         *rr_activate_ind);
EXTERN void mm_rr_establish_cnf       (T_RR_ESTABLISH_CNF        *rr_establish_cnf);
EXTERN void mm_rr_establish_ind       (T_RR_ESTABLISH_IND        *rr_establish_ind);
EXTERN void mm_rr_release_ind         (T_RR_RELEASE_IND          *rr_release_ind);
EXTERN void mm_rr_sync_ind            (T_RR_SYNC_IND             *rr_sync_ind);
#if defined (FF_EOTD) AND defined (REL99)
EXTERN void mm_rr_rrlp_start_ind      (T_RR_RRLP_START_IND       *rr_rrlp_start_ind);
EXTERN void mm_rr_rrlp_stop_ind       (T_RR_RRLP_STOP_IND        *rr_rrlp_stop_ind);
#endif /* (FF_EOTD) AND defined (REL99) */
EXTERN void mm_sim_set_imsi_marker    (T_MSG_TYPE                imsi_marker);


/*
 *  mobility management signalling
 */
EXTERN void mm_abort                    (T_D_ABORT           *d_abort);
EXTERN void mm_auth_rej                 (void);
EXTERN void mm_auth_req                 (T_D_AUTH_REQ        *auth_req);
EXTERN void mm_cm_message               (UBYTE               pd,
                                         UBYTE               ti,
                                         T_RR_DATA_IND       *rr_data_ind);
EXTERN void mm_cm_serv_accept           (void);
EXTERN void mm_cm_serv_rej              (T_D_CM_SERV_REJ     *cm_serv_rej);
EXTERN void mm_ident_req                (T_D_IDENT_REQ       *ident_req);
EXTERN void mm_loc_upd_acc              (T_D_LOC_UPD_ACCEPT  *loc_upd_accept);
EXTERN void mm_lup_rej                  (T_D_LOC_UPD_REJ     *loc_upd_rej);
EXTERN void mm_mm_status                (void);
EXTERN void mm_mmr_auth_cnf             (T_SIM_AUTHENTICATION_CNF *sim_auth_cnf);
EXTERN void mm_mmr_nreg_req             (UBYTE               nreg_cause,
                                         UBYTE               detach_done);
EXTERN void mm_mmr_reg_req              (UBYTE               func);
EXTERN void mm_tmsi_realloc_cmd         (T_D_TMSI_REALLOC_CMD *tmsi_realloc_cmd);
EXTERN void mm_cm_service_prompt        (T_D_CM_SERVICE_PROMPT *cm_service_prompt);
EXTERN void mm_mm_information           (T_D_MM_INFORMATION *mm_information);

/*
 *  mobility management procedures
 */
EXTERN void mm_auto_net_reg             (void);
EXTERN void mm_abort_connection         (UBYTE               abcs);
EXTERN void mm_attach_loc_upd           (void);
EXTERN void mm_build_auth_res           (T_SIM_AUTHENTICATION_CNF *sim_auth_cnf,
                                         T_U_AUTH_RES        *auth_res);
EXTERN void mm_build_ident_res          (UBYTE               id_type,
                                         T_U_IDENT_RES       *ident_res);
EXTERN void mm_build_mm_status          (UBYTE               cause,
                                         T_B_MM_STATUS       *mm_status);
EXTERN void mm_build_rr_sync_req_mode   (void);
EXTERN void mm_build_rr_sync_req_cause  (USHORT              cause);
EXTERN void mm_build_rr_sync_req_ciph   (void);
EXTERN void mm_build_rr_sync_req        (T_MSG_TYPE           ciph);
EXTERN void mm_build_rr_sync_req_tmsi   (void);
EXTERN void mm_build_rr_sync_hplmn_req  (void);
EXTERN void mm_check_error_flag         (void);
EXTERN BOOL mm_check_critical_error     (void);
EXTERN BOOL mm_send_status_on_error     (void);
EXTERN void mm_clear_mob_ident          (T_imsi_struct       *mob_ident);
EXTERN void mm_clear_reg_data           (void);
EXTERN void mm_copy_rr_act_cnf_data     (T_RR_ACTIVATE_CNF   *rr_activate_cnf);
EXTERN SHORT mm_count_connections       (UBYTE               conn_type);
EXTERN void mm_create_est_message       (USHORT              est_cause,
                                         UBYTE               service,
                                         UBYTE               ti,
                                         USHORT              *bit_size_message);
EXTERN void mm_create_imsi_detach_message (void);
EXTERN void mm_delete_entry             (UBYTE               comp,
                                         UBYTE               ti);
EXTERN void mm_init                     (void);
EXTERN UBYTE mm_calculate_digits        (const UBYTE         *digits);
EXTERN void mm_loc_upd_rej              (void);
EXTERN void mm_lup_restart              (void);
EXTERN void mm_mdl_rel_req              (void);
EXTERN void mm_mdl_rel_req_sapi_3       (void);
EXTERN void mm_mmsms_rel_ind            (USHORT              cause,
                                         UBYTE               conn_typ);
EXTERN SHORT mm_mmxx_err_ind            (USHORT              cause);
EXTERN void mm_mmxx_est_cnf             (void);
EXTERN void mm_mmxx_release_ind         (UBYTE               comp, 
                                         UBYTE               ti, 
                                         USHORT              relcs);
EXTERN void mm_mmxx_rel_ind             (USHORT              cause,
                                         UBYTE               conn_typ);
EXTERN void mm_normal_loc_upd           (void);
EXTERN void mm_periodic_loc_upd         (void);
EXTERN void mm_power_off                (void);
EXTERN USHORT mm_random                 (USHORT              n);
EXTERN void mm_read_entry               (void);
EXTERN void mm_reest                    (UBYTE               ti);
EXTERN void mm_release_rr_connection    (UBYTE resumption);
EXTERN void mm_rr_act_req               (void);
EXTERN void mm_rr_data_req              (USHORT              est_cause,
                                         UBYTE               service,
                                         UBYTE               ti);
EXTERN void mm_rr_est_req               (USHORT              est_cause,
                                         UBYTE               service,
                                         UBYTE               ti);
EXTERN void mm_start_loc_upd            (UBYTE               lup_type);
EXTERN void mm_continue_running_update  (void);
EXTERN void mm_start_net_req            (void);
EXTERN void mm_start_t3212_bcch         (void);
EXTERN void mm_change_t3212             (void);
EXTERN void mm_use_entry                (void);
EXTERN void mm_write_entry              (UBYTE               comp,
                                         UBYTE               ti,
                                         USHORT              est_cs,
                                         U8                  content_type,
                                         void*               primitive,
                                         U8                  info);
EXTERN BOOL mm_check_lai                (const T_loc_area_ident * lai1,
                                         const T_loc_area_ident * lai2);
EXTERN BOOL mm_check_lai_from_RR        (const T_loc_area_ident * lai1,
                                         const T_plmn           * plmn2,
                                               USHORT             lac2);
EXTERN BOOL mm_normal_upd_needed        (void);
EXTERN BOOL mm_attach_upd_needed        (void);
EXTERN BOOL mm_periodic_upd_needed      (void);
EXTERN BOOL mm_set_follow_on_request    (void);
EXTERN void mm_end_of_detach            (void);
EXTERN UBYTE mm_get_service_state       (void);
EXTERN BOOL mm_full_service_pplmn_scan  (void);

/* Implements Measure 29 and streamline encoding */
EXTERN void mm_send_status              (UBYTE   cause); 

/*
 *  Registration
 */

/*
 *  registration primitives and functions
 */

#ifndef GPRS
EXTERN  void reg_mmr_net_req       (T_MMR_NET_REQ       *mmr_net_req);
EXTERN  void reg_mmr_nreg_req      (T_MMR_NREG_REQ      *mmr_nreg_req);
EXTERN  void reg_mmr_plmn_mode_req (T_MMR_PLMN_MODE_REQ *plmn_mode_req);
EXTERN  void reg_mmr_plmn_res      (T_MMR_PLMN_RES      *mmr_plmn_res);
EXTERN  void reg_mmr_reg_req       (T_MMR_REG_REQ       *mmr_reg_req);
#endif /* GPRS */

EXTERN void mm_func_mmgmm_net_req       (void);
EXTERN void mm_func_mmgmm_nreg_req      (UBYTE detach_cause,
                                         UBYTE detach_done,
                                         USHORT cs);
EXTERN void mm_func_mmgmm_plmn_mode_req (UBYTE mode);
EXTERN void mm_func_mmgmm_plmn_res      (const T_plmn *plmn, 
                                         UBYTE reg_type,
                                         UBYTE mobile_class);
EXTERN void mm_func_mmgmm_reg_req       (UBYTE service_mode,
                                         UBYTE reg_type,
                                         UBYTE mobile_class,
                                         UBYTE bootup_act);

EXTERN void reg_sim_auth_cnf      (T_SIM_AUTHENTICATION_CNF *sim_auth_cnf);
EXTERN void reg_sim_mm_insert_ind (T_SIM_MM_INSERT_IND *sim_mm_insert_ind);
EXTERN void reg_sim_remove_ind    (T_SIM_REMOVE_IND    *sim_remove_ind);
EXTERN void reg_sim_mm_info_ind   (T_SIM_MM_INFO_IND   *sim_mm_info_ind);
EXTERN void reg_sim_file_upd_ind  (T_SIM_FILE_UPDATE_IND *file_upd);
EXTERN void reg_sim_read_cnf      (T_SIM_READ_CNF *sim_read_cnf);
EXTERN void reg_sim_sync_cnf      (T_SIM_SYNC_CNF    *sim_sync_cnf);
EXTERN void reg_send_sim_sync_req (void);
/*
 *  registration signalling
 */
EXTERN void reg_mmr_auth_ind            (T_SIM_AUTHENTICATION_REQ *sim_auth_req);
EXTERN void reg_mm_success              (UBYTE                service);
EXTERN void reg_mm_failure              (UBYTE                forb_ind);
EXTERN void reg_net_list                (const T_RR_ABORT_IND *rr_abort_ind);
EXTERN void reg_mm_cell_selected        (void);

/*
 *  registration procedures
 */
EXTERN BOOL reg_best_plmn_in_country    (const T_plmn *bcch_plmn);
EXTERN void reg_check_hplmn_tim         (UBYTE decihours);

EXTERN void reg_build_sim_update        (void);
EXTERN void reg_copy_sim_data           (const T_SIM_MM_INSERT_IND *sim_mm_insert_ind);
EXTERN void reg_create_plmn_list        (const T_RR_ABORT_IND *rr_abort_ind,
                                         UBYTE               include_flag);
EXTERN void reg_extract_hplmn           (T_plmn              *plmn);
EXTERN void reg_init                    (void);
EXTERN void reg_plmn_add_bad            (UBYTE               *forb_plmn_list, 
                                         USHORT              list_size, 
                                         const T_plmn        *plmn);
EXTERN void reg_plmn_bad_del            (UBYTE               *forb_plmn_list, 
                                         USHORT              list_size, 
                                         const T_plmn        *plmn);
EXTERN BOOL reg_plmn_empty              (const T_plmn        *plmn);

EXTERN BOOL reg_sim_ef_plmn_field_empty (UBYTE               *plmn);

EXTERN BOOL reg_plmn_equal_sim          (const T_plmn        *bcch_plmn,
                                         const T_plmn        *sim_plmn);

#ifdef REL99
EXTERN BOOL reg_read_plmn_present       (UBYTE*           plmn_bytes);
EXTERN BOOL reg_read_plmn_support_acctec(UBYTE*           plmn_bytes);
#endif

EXTERN BOOL reg_plmn_equal_eqv          (const T_plmn        *bcch_plmn,
                                         const T_plmn        *req_plmn);
EXTERN BOOL reg_plmn_is_NA_plmn         (const T_plmn        *bcch_plmn);
EXTERN BOOL reg_plmn_equal_hplmn        (const T_plmn        *bcch_plmn);
EXTERN BOOL reg_plmn_equal_rplmn        (T_plmn   *plmn);
EXTERN BOOL reg_imsi_equal              (const T_imsi_struct *imsi1,
                                         const T_imsi_struct *imsi2);
EXTERN void reg_plmn_select             (UBYTE forb_ind);
EXTERN void reg_read_bcch_info          (const T_SIM_MM_INSERT_IND *sim_mm_insert_ind);

EXTERN BOOL reg_sim_files_to_be_read    (const T_SIM_MM_INSERT_IND *sim_mm_insert_ind);

EXTERN void reg_read_forb_plmn          (const T_forb_plmn   *forb_plmn);
EXTERN void reg_read_imsi               (T_imsi_struct       *imsi_struct, 
                                         const T_imsi_field  *imsi_field);
EXTERN void reg_read_kc_cksn            (const T_SIM_MM_INSERT_IND *sim_mm_insert_ind);
EXTERN void reg_read_loc_info           (const T_loc_info    *loc_info);

EXTERN void reg_read_pref_plmn          (UBYTE*              data,
                                         USHORT              length);
#ifdef REL99
EXTERN void reg_read_ucps_acctec        (UBYTE*              data,
                                         USHORT              length);
EXTERN void reg_read_ocps_acctec        (UBYTE*              data,
                                         USHORT              length);
#endif

EXTERN void reg_copy_sim_ahplmn         (const T_SIM_MM_INSERT_IND *sim_mm_insert_ind);
EXTERN void reg_set_bcch_info           (T_SIM_MM_UPDATE_REQ *sim_mm_update_req);
EXTERN void reg_set_forb_plmns          (T_SIM_MM_UPDATE_REQ *sim_mm_update_req);
EXTERN void reg_set_kc                  (T_SIM_MM_UPDATE_REQ *sim_mm_update_req);
EXTERN void reg_set_loc_info            (T_SIM_MM_UPDATE_REQ *sim_mm_update_req);
EXTERN void reg_set_pref_plmns          (T_SIM_MM_UPDATE_REQ *sim_mm_update_req); // #####
EXTERN void reg_clear_plmn              (T_plmn              *plmn);
EXTERN BOOL reg_clear_plmn_list         (UBYTE               *plmn_list,
                                         USHORT              list_size);
EXTERN void reg_unpack_plmn             (T_plmn              *plmn,
                                         const UBYTE         *packed,
                                         USHORT              index);
EXTERN void reg_pack_plmn               (UBYTE               *packed, 
                                         USHORT              index,
                                         const T_plmn        *plmn);
EXTERN BOOL reg_plmn_in_list            (const UBYTE         *forb_plmn_list, 
                                         USHORT              list_size, 
                                         const T_plmn        *plmn);
EXTERN void reg_rr_failure              (T_RR_ABORT_IND      *rr_abort_ind);
EXTERN BOOL reg_read_next_sim_file      (void);
EXTERN void reg_end_of_deregistration   (UBYTE nreg_cause,
                                         UBYTE service);
#ifdef REL99
EXTERN void reg_invalidate_upd_state    (UBYTE new_update_state, BOOL tmsi_cksn_kc_not_deleted);
#else
EXTERN void reg_invalidate_upd_state    (UBYTE new_update_state);
#endif
GLOBAL void reg_select_network          (const T_plmn *plmn);
EXTERN BOOL reg_plmn_in_pref_list       (const T_plmn        *plmn);
EXTERN BOOL reg_same_country_plmn       (const T_plmn *plmn1,
                                         const T_plmn *plmn2);
EXTERN void reg_check_plmn_search       (USHORT cause,
                                         const T_RR_ABORT_IND *rr_abort_ind);
EXTERN void reg_check_hplmn_tim         (UBYTE decihours);
EXTERN void reg_stop_hplmn_tim          (void);
/*
 *  Equivalent plmn procedures
 */
EXTERN BOOL reg_store_eqv_plmns         (T_eqv_plmn_list *eqv_plmn_list, T_plmn *plmn);
EXTERN void reg_read_acting_hplmn       (const U8  acting_hplmn[]);

EXTERN BOOL valid_acting_hplmn          (T_plmn *acting_hplmn);
GLOBAL void mm_read_ffs_init            (void);
GLOBAL void mm_write_eplmn_to_ffs       (void);
GLOBAL void mm_display_eplmn            (void);

GLOBAL void mm_reset_ffs                (void);
GLOBAL BOOL mm_read_ffs                 (void);
GLOBAL void check_if_cingular_sim       (void);

/*
 *  Formatter
 */

/*
 *  formatter primitives
 */
#if 0 /* Not referenced */
EXTERN       void for_init_mm_data      (void);
#endif /* #if 0 */
EXTERN void for_rr_data_ind       (T_RR_DATA_IND       *rr_data_ind);

/*
 *  formatter signalling
 */
/*
 * If all entities are linked into one module this definitions
 * prefixes the global data with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define for_est_req _ENTITY_PREFIXED(for_est_req)
#endif /*OPTION_MULTITHREAD*/
EXTERN void for_cm_message              (T_RR_DATA_REQ       *rr_data_req);
EXTERN void for_data_req                (USHORT              bit_size_message);
EXTERN void for_est_req                 (USHORT              est_cause,
                                         USHORT              bit_size_message);

/*
 *  formatter procedures
 */
/*
 * If all entities are linked into one module this definitions
 * prefixes the global data with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define for_check_mobile_identity _ENTITY_PREFIXED(for_check_mobile_identity)
#endif /*OPTION_MULTITHREAD*/
EXTERN BOOL  for_check_identity_type     (UBYTE               id);
EXTERN BOOL  for_check_mobile_identity   (T_mob_id           *mob_ident);
EXTERN UBYTE for_check_reject_cause      (UBYTE               cause);
EXTERN void  mm_for_set_error            (U8 cause);
#if 0 /* Not referenced; body bag; 02.10.2003 */
EXTERN void  for_set_mandatory_error     (void);
EXTERN void  for_set_content_error       (void);
EXTERN void  for_set_optional_error      (UBYTE               iei);
#endif /* #if 0 */

/*
 *  csf procedures
 */
EXTERN void csf_read_mobile_class_1     (T_mob_class_1   *mob_class_1);
EXTERN void csf_read_mobile_class_2     (T_mob_class_2   *mob_class_2);
EXTERN void csf_read_imei               (T_imsi_struct   *imei_struct);

/*
 * timer procedures
 */
EXTERN void tim_t3210                   (void);
EXTERN void tim_t3211                   (void);
EXTERN void tim_t3212                   (void);
EXTERN void tim_t3213                   (void);
EXTERN void tim_t3220                   (void);
EXTERN void tim_t3230                   (void);
EXTERN void tim_t3240                   (void);
#ifdef REL99
EXTERN void tim_t3241                   (void);
#endif
EXTERN void tim_t_reg                   (void);
EXTERN void tim_t_hplmn                 (void);

/*
 *  timer
 */
#if defined (NEW_FRAME)
#define TIMERSTART(i,v)                 tim_start_timer (i, v)
#define TIMERSTOP(i)                    tim_stop_timer (i)
#define TIMERACTIVE(i)                  (mm_data->t_running[i])
#else
#define TIMERSTART(t, v)  csf_alloc_timer (t, v, &mm_data->t_handle[t])
#define TIMERSTOP(t) \
  { csf_free_timer (mm_data->t_handle[t]); mm_data->t_handle[t] = VSI_ERROR; }
#define TIMERACTIVE(t)  (mm_data->t_handle[t] NEQ VSI_ERROR)

#define T_TIME  T_VSI_TVALUE

#endif /*(NEW_FRAME)*/



#ifdef GPRS
/*
 * Prototypes for the primitive interface MMGMM (MM to GMM) 
 */
EXTERN void mm_send_mmgmm_reg_cnf       (UBYTE bootup_cause);
EXTERN void mm_mmgmm_reg_cnf            (void);
EXTERN void mm_mmgmm_nreg_ind           (UBYTE service, 
                                         UBYTE search_running,
                                         UBYTE forb_ind);
EXTERN void mm_mmgmm_nreg_cnf           (UBYTE detach_cause);
EXTERN void mm_mmgmm_plmn_ind           (USHORT cause,
                                   const T_RR_ABORT_IND *rr_abort_ind);
EXTERN void mm_mmgmm_ahplmn_ind          (T_plmn   *acting_hplmn);
#else
/* 
 * Prototypes for the  primitive interface MMR (MM to ACI)
 */
EXTERN void reg_send_mmr_reg_cnf       (UBYTE bootup_cause);
EXTERN void reg_build_mmr_reg_cnf       (void);
EXTERN void reg_build_mmr_nreg_ind      (UBYTE service, 
                                         UBYTE search_running,
                                         UBYTE forb_ind);
EXTERN void reg_build_mmr_nreg_cnf      (UBYTE detach_cause);
EXTERN void reg_build_mmr_plmn_ind      (USHORT cause,
                                   const T_RR_ABORT_IND *rr_abort_ind);
EXTERN void reg_build_mmr_ahplmn_ind    (T_plmn   *acting_hplmn);

/* 
 * By using this definitions, we avoid having a lot of ifdefs 
 * in the code itself.
 */
#define mm_send_mmgmm_reg_cnf(b)        reg_send_mmr_reg_cnf(b)
#define mm_mmgmm_reg_cnf()              reg_build_mmr_reg_cnf()
#define mm_mmgmm_nreg_ind(a,b,c)        reg_build_mmr_nreg_ind(a,b,c)
#define mm_mmgmm_nreg_cnf(detach_cause) reg_build_mmr_nreg_cnf(detach_cause)
#define mm_mmgmm_plmn_ind(cause,ab)     reg_build_mmr_plmn_ind(cause,ab)
#define mm_mmgmm_ahplmn_ind(ahplmn)     reg_build_mmr_ahplmn_ind(ahplmn)  
#endif /* GPRS */

#ifdef  GPRS
/* 
 * These functions are only used in the GPRS protocol stack. 
 */
EXTERN void mm_mmgmm_auth_rej_ind     (void);
EXTERN void mm_mmgmm_cm_establish_ind (void);
EXTERN void mm_mmgmm_cm_emergency_ind (void);
EXTERN void mm_mmgmm_cm_release_ind   (UBYTE resumption);
EXTERN void mm_mmgmm_activate_ind     (UBYTE status);
EXTERN void mm_mmgmm_t3212_val_ind    (void);
EXTERN void mm_mmgmm_lup_accept_ind   (void);
EXTERN void mm_mmgmm_lup_needed_ind   (UBYTE reason);
EXTERN void mm_mmgmm_tmsi_ind         (ULONG tmsi);
#else
/*
 * Define the functions only needed in GPRS to nothing.
 * This will help the compiler's optimizer to generate 
 * efficient code for a GSM only protocol stack.
 */
#define mm_mmgmm_auth_rej_ind()
#define mm_mmgmm_cm_establish_ind()
#define mm_mmgmm_cm_emergency_ind()
#define mm_mmgmm_cm_release_ind(resumption)
#define mm_mmgmm_activate_ind(status)
#define mm_mmgmm_t3212_val_ind()
#define mm_mmgmm_lup_accept_ind(plmn, lac)
#define mm_mmgmm_lup_needed_ind(reason)
#define mm_mmgmm_tmsi_ind(tmsi)

/* 
 * There are some additional parameters in some function calls which 
 * are only used by GPRS. This have no meaning for GSM, but it is undesirable 
 * to #ifdef out all places where they are used, so they will be defined here.
 */
#define MMGMM_RESUMPTION_FAILURE 1
#define MMGMM_PERFORM_DETACH     0
#define REG_GPRS_INACTIVE        0
#define MMGMM_NO_ERROR           0xff
#define MMGMM_CLASS_CC           5 
#endif  /* else, #ifdef GPRS */

EXTERN T_MM_DATA mm_data_base;
#define GET_INSTANCE_DATA    register T_MM_DATA *mm_data= &mm_data_base


/*
 * If all entities are linked into one module this definitions
 * prefixes the global data with the enity name
 */
#ifdef OPTION_MULTITHREAD
#if defined (TI_PS_HCOMM_CHANGE)
#else
  #define hCommMMI        _ENTITY_PREFIXED(hCommMMI)
  #define hCommCC         _ENTITY_PREFIXED(hCommCC)
  #define hCommSS         _ENTITY_PREFIXED(hCommSS)
  #define hCommSMS        _ENTITY_PREFIXED(hCommSMS)
  #define hCommRR         _ENTITY_PREFIXED(hCommRR)
  #define hCommDL         _ENTITY_PREFIXED(hCommDL)
  #define hCommSIM        _ENTITY_PREFIXED(hCommSIM)
#endif
#endif /* OPTION_MULTITHREAD */

#if defined (NEW_FRAME)
#if defined (TI_PS_HCOMM_CHANGE)
#else
EXTERN T_HANDLE  hCommMMI;        /* MMI  Communication       */
EXTERN T_HANDLE  hCommCC;         /* CC   Communication       */
EXTERN T_HANDLE  hCommSS;         /* SS   Communication       */
EXTERN T_HANDLE  hCommSMS;        /* SMS  Communication       */
EXTERN T_HANDLE  hCommRR;         /* RR   Communication       */
EXTERN T_HANDLE  hCommDL;         /* DL   Communication       */
EXTERN T_HANDLE  hCommSIM;        /* SIM  Communication       */
#endif /* (TI_PS_HCOMM_CHANGE) */
EXTERN T_HANDLE  mm_handle;
#else
#if defined (TI_PS_HCOMM_CHANGE)
#else
EXTERN T_VSI_CHANDLE  hCommMMI;        /* MMI  Communication       */
EXTERN T_VSI_CHANDLE  hCommCC;         /* CC   Communication       */
EXTERN T_VSI_CHANDLE  hCommSS;         /* SS   Communication       */
EXTERN T_VSI_CHANDLE  hCommSMS;        /* SMS  Communication       */
EXTERN T_VSI_CHANDLE  hCommRR;         /* RR   Communication       */
EXTERN T_VSI_CHANDLE  hCommDL;         /* DL   Communication       */
EXTERN T_VSI_CHANDLE  hCommSIM;        /* SIM  Communication       */
#endif /* (TI_PS_HCOMM_CHANGE) */
EXTERN T_VSI_THANDLE  mm_act_handle;
#endif /*NEW FRAME*/
/*
 * If all entities are linked into one module this definitions
 * prefixes the global data with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define _decodedMsg   _ENTITY_PREFIXED(_decodedMsg)
#endif /*OPTION_MULTITHREAD*/

EXTERN UBYTE          _decodedMsg [];

#ifdef OPTION_TIMER
  /*
   * If all entities are linked into one module this definitions
   * prefixes the global data with the entity name
   */
  #ifdef OPTION_MULTITHREAD
    #define partab    _ENTITY_PREFIXED(partab)
  #endif /*OPTION_MULTITHREAD*/

  EXTERN const KW_DATA partab[];
#endif /*OPTION_TIMER*/

#ifndef _TMS470
int sprintf( char *buffer, const char *format, ... );
#define Sprintf sprintf
#endif /*_TMS470*/

#endif
