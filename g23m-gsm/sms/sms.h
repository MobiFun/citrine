/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (8410)
|  Modul   :  SMS
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
|             Short Message Service
+----------------------------------------------------------------------------- 
*/ 

#ifndef SMS_H
#define SMS_H

#include <stdio.h>
#ifdef TI_PS_HCOMM_CHANGE
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

/*
 * Definition whether a shared CCD buffer shall be used
 */
/* #undef SHARED_CCD_BUF  */

#ifndef _TARGET_

#define FF_CPHS   /* This flag is only required for SIMULATION defined in .xml file,
                     to remove the redefinition warning put under comment */
#endif /* _TARGET_ */

#define FF_SMS_23430

#if defined(FF_SMS_23430)
#define FF_SMS_NW_RCG_SIM
#endif

/*==== MACROS =====================================================*/

#if FALSE
#define SMS_SDU_COPY(_d_,_s_,_t_) memcpy(_d_,_s_,\
                     (offsetof(T_##_t_,sdu)+offsetof(T_sdu,buf)+\
                      BYTELEN(D_SDU_OFF(_s_)+D_SDU_LEN(_s_))))
#else
#define SMS_SDU_COPY(_d_,_s_,_t_) memcpy(&(_d_)->sdu,&(_s_)->sdu,\
                     offsetof(T_sdu,buf)+\
                     BYTELEN(D_SDU_OFF(_s_)+D_SDU_LEN(_s_)))
#endif

#define GET_SMS_INSTANCE(ti)     csf_get_sms_instance(ti)
#define GET_NEW_SMS_INSTANCE(ti) csf_get_new_sms_instance(ti)
#define FREE_SMS_INSTANCE(ti)    csf_free_sms_instance(ti)
#define GET_MO_INSTANCE(_sd_)    ((_sd_)->inst = INST_MO)
#define GET_MT_INSTANCE(_sd_)    ((_sd_)->inst = INST_MT)

/*
 * The assert() macro as defined by the frame stops the task, 
 * not only in simulation but also on the target. For the 
 * simulation this is a desired behaviour, for the target it is not. 
 */
#ifndef WIN32
#undef assert
#define assert(x) if (!(x)) { TRACE_ERROR ("Assertion failed"); }
#endif

#define CMMS_ACTIVE ((sms_data->cmms_mode EQ CMMS_MODE_ONE) \
                       OR (sms_data->cmms_mode EQ CMMS_MODE_TWO))

/*==== CONSTANTS ==================================================*/

#define DISPLAY 0
#define IGNORE  0xFE      // return RP-ACK if handled unsuccessfully
#define NOTHING 0xFF      // return RP-ERROR if handled unsuccessfully

#define MAX_SMS_CALLS    2

#define INST_MO 0
#define INST_MT 1

#define SMS_INST         sms_data->data[sms_data->inst]

#define ENTITY_DATA      sms_data

/*
 * Recognition of certain networks (SIM opearators)
 */

#define NW_SIM_NONE      0       /* no special network given */
#define NW_SIM_23430     1       /* SIM provided by operator 234-30 */

/*
 * Dynamic Configuration Numbers
 */
enum
{
   SMS_CONF_EVENT_TIMER_SET        = 1,
   SMS_CONF_EVENT_TIMER_RESET      = 2,
   SMS_CONF_EVENT_TIMER_SPEED_UP   = 3,
   SMS_CONF_EVENT_TIMER_SLOW_DOWN  = 4,
   SMS_CONF_EVENT_TIMER_SUPPRESS   = 5,
   SMS_CONF_EVENT_PAUSE            = 6,
   SMS_CONF_EVENT_RESUME           = 7,

   SMS_CONF_EVENT_END,
   SMS_CONF_EVENT_MAX = (SMS_CONF_EVENT_END - 1)
};
typedef UBYTE SMS_CONF_EVENT_ENUM_TYPE;

/*
 * Timer definitions, MAX_SMS_TIMER
 *
 * Description :  The constants defines the timers and the number of timers
 *                available in the timer pool.
 * Note        :  One CP and one RL timer per instance.
 */
#define TC1M             0
#define TR1M             1
#define TR2M             2
#define TRAM             3
#define TLCT             4   /* concatenation timer */
#define TMMS             5   /* CMMS Timer */
#define MAX_SMS_TIMER    6

/*
 * Protocoldiscriminator SMS
 */
#define ENCODE_OFFSET    32
#define BSIZE_TI_PD      8

/*
 * Protocoldiscriminator SMS
 */
#define PD_SMS           9

/*
 * Max size of SMS-message
 */
#define MAX_SMS_MESSAGE_SIZE           (MAX_BITSTREAM_LEN_SMS>>3)

/*
 * Max size of TP user data
 */
#define MAX_CHARACTERS_TPUD            141

/*
 * Max size of sim-sms-record
 */
#define SIM_LENGTH_SMS_RECORD          176

/*
 * access values
 */
#define ACCESS_EMPTY                   0

/*
 * length of U_CP_DATA
 */
#define LEN_U_CP_DATA  (252*8)

/*
 * Max length of D_CP_USER_DATA
 */
#define MAX_LEN_CP_USER_DATA  249
#define MIN_CP_LEN 3
#define MIN_CP_ACK_LEN 2

/*
 * constants for GSMS: active downlink
 */
#if defined (GPRS)
#define SMS_DOWNLINK_NONE              0  /* no downlink assigned yet             */
#define SMS_DOWNLINK_MMSMS             1  /* downlink is MM using SAP MMSMS       */
#define SMS_DOWNLINK_LL_CHECK          2  /* downlink LL is being checked         */
#define SMS_DOWNLINK_LL                3  /* downlink is LL, registered           */

/*
 * constants for GSMS: LLC flow control
 */
#define SMS_LLC_UNKNOWN             0x00
#define SMS_LLC_AVAILABLE           0x01
#define SMS_LLC_BUSY                0x02   /* LLC is busy, no instance waiting    */
#define SMS_LLC_BUSY_WAITING        0x03   /* LLC is busy and instance is waiting */

/* constants for GSMS: SMS entity flow control status towards LLC */
#define SMS_FLOW_UNKNOWN            0x00
#define SMS_FLOW_AVAILABLE          0x01  /* availability of SMS signalled to LLC */
#define SMS_FLOW_BUSY               0x02  /* SMS entity is busy */

/* cp type of acknowledgement to be sent */
#define SMS_CP_NONE                 0x00
#define SMS_CP_ACK                  0x01
#define SMS_CP_ERROR                0x02
#endif /* GPRS */

/*
 * a modified version of the S/GET_STATE macros for tracing SMS_INST states
 */
#ifdef NTRACE
  #define SMS_INST_SET_STATE(PROCESS,STATE)    SMS_INST.state[PROCESS] = STATE
  #define SMS_INST_GET_STATE(PROCESS)          SMS_INST.state[PROCESS]
#else
  #define SMS_INST_SET_STATE(PROCESS,STATE)\
              { vsi_o_strace (VSI_CALLER\
                              PROCESS_NAME_SMS_INST[PROCESS],\
                              PROCESS##_NAME [ SMS_INST.state[PROCESS] ],\
                              PROCESS##_NAME [          STATE          ]);\
                SMS_INST.state[PROCESS] = (STATE); }
  #define SMS_INST_GET_STATE(PROCESS)\
                vsi_o_strace (VSI_CALLER\
                              PROCESS_NAME_SMS_INST[PROCESS],\
                              PROCESS##_NAME [ SMS_INST.state[PROCESS] ],\
                                                        NULL           ),\
                                               SMS_INST.state[PROCESS]
#endif /* else, #ifdef NTRACE */

/*
 * CP States
 */
#define CP_IDLE                        0  /* 3gpp, MO+MT */
#define CP_MM_CONNECTION_PENDING       1  /* 3gpp, MO    */
#define CP_WAIT_FOR_ACK                2  /* 3gpp, MO+MT */
#define CP_MM_CONNECTION_ESTABLISHED   3  /* 3gpp, MO+MT */

#if defined (GPRS)
/*
 * CP States for GSMS
 */
#define CP_GSMS_IDLE                   4  /* 3gpp */
#define CP_GSMS_MO_WAIT_FOR_CP_ACK     5  /* 3gpp */
#define CP_GSMS_MT_WAIT_FOR_CP_ACK     6  /* 3gpp */
#define CP_GSMS_MO_WAIT_FOR_CP_DATA    7  /* 3gpp */
#define CP_GSMS_MT_WAIT_FOR_RP_ACK     8  /* 3gpp */
#endif /* GPRS */

#if defined (GPRS)
EXTERN  const char * const
        STATE_CP_NAME[];
#define STATE_CP_NAME_INIT             \
       "CP_IDLE",                      \
       "CP_MM_CONNECTION_PENDING",     \
       "CP_WAIT_FOR_ACK",              \
       "CP_MM_CONNECTION_ESTABLISHED", \
       "CP_GSMS_IDLE",                 \
       "CP_GSMS_MO_WAIT_FOR_CP_ACK",   \
       "CP_GSMS_MT_WAIT_FOR_CP_ACK",   \
       "CP_GSMS_MO_WAIT_FOR_CP_DATA",  \
       "CP_GSMS_MT_WAIT_FOR_RP_ACK"
#else
EXTERN  const char * const
        STATE_CP_NAME[];
#define STATE_CP_NAME_INIT             \
       "CP_IDLE",                      \
       "CP_MM_CONNECTION_PENDING",     \
       "CP_WAIT_FOR_ACK",              \
       "CP_MM_CONNECTION_ESTABLISHED"
#endif

/*
 * RL States
 */
#define RL_IDLE                        0  /* 3gpp */
#define RL_WAIT_FOR_ACK                1  /* 3gpp */
#define RL_WAIT_FOR_SMMA_ACK           2  /* TI */
#define RL_WAIT_FOR_SEND_ACK           3  /* 3gpp */
#define RL_WAIT_FOR_RETRANS_TIMER      4  /* 3gpp */
#define RL_ESTABLISH                   5  /* TI */
#define RL_ESTABLISHED                 6  /* TI */
#define RL_WAIT_FOR_SEND_ERROR         7  /* TI */

EXTERN  const char * const
        STATE_RL_NAME[];
#define STATE_RL_NAME_INIT             \
       "RL_IDLE",                      \
       "RL_WAIT_FOR_ACK",              \
       "RL_WAIT_FOR_SMMA_ACK",         \
       "RL_WAIT_FOR_SEND_ACK",         \
       "RL_WAIT_FOR_RETRANS_TIMER",    \
       "RL_ESTABLISH",                 \
       "RL_ESTABLISHED",               \
       "RL_WAIT_FOR_SEND_ERROR"

/*
 * TL States
 */
#define TL_IDLE                        0
#define TL_SEND                        1
#define TL_COMMAND                     2
#define TL_RECEIVE                     3
#define TL_OTHER                       4
#define TL_ESTABLISH                   5
#define TL_SEND_CONTD                  6

EXTERN  const char * const
        STATE_TL_NAME[];
#define STATE_TL_NAME_INIT             \
       "TL_IDLE",                      \
       "TL_SEND",                      \
       "TL_COMMAND",                   \
       "TL_RECEIVE",                   \
       "TL_OTHER",                     \
       "TL_ESTABLISH",                 \
       "TL_SEND_CONTD"                 

/*
 * SMS manages 3 (MAX_STATE_SMS_INST) states per SMS_INST: CP, RL, TL
 */
#define STATE_CP                       0
#define STATE_RL                       1
#define STATE_TL                       2
#define MAX_STATE_SMS_INST             3

EXTERN  const char * const
        PROCESS_NAME_SMS_INST[];
#define PROCESS_NAME_SMS_INST_INIT     \
       "STATE_CP"   ,                  \
       "STATE_RL"   ,                  \
       "STATE_TL"

/*
 * A definition of the states of SMS_INST is provided in SMS_PEI.C
 */
#if defined(SMS_PEI_C) && ! defined(NTRACE)
  GLOBAL const char * const          STATE_CP_NAME[] = {          STATE_CP_NAME_INIT };
  GLOBAL const char * const          STATE_RL_NAME[] = {          STATE_RL_NAME_INIT };
  GLOBAL const char * const          STATE_TL_NAME[] = {          STATE_TL_NAME_INIT };
  GLOBAL const char * const  PROCESS_NAME_SMS_INST[] = {  PROCESS_NAME_SMS_INST_INIT };
#endif

/*
 * MMI States
 */
#define MMI_IDLE                       0
#define MMI_READ                       1
#define MMI_WRITE                      2
#define MMI_DELETE                     3
#define MMI_FIND_FIRST                 4
#define MMI_FIND_NEXT                  5
#define MMI_REPLACE                    6
#define MMI_STATE_UPDATE               7
#define MMI_READ_STATE_UPDATE          8 
#define MMI_RESUME                     9

/* N950 Memory Optimization - Implements Measure #39*/
EXTERN  T_S2I_STRING const
        STATE_MMI_NAME[];
/*
 * NET States
 */
#define NET_IDLE                       0
#define NET_WRITE                      1
#define NET_READ                       2
#define NET_23430_WRITE                3
#define NET_23430_DELETE               4

/* N950 Memory Optimization - Implements Measure #39*/
EXTERN  T_S2I_STRING const
        STATE_NET_NAME[];
/*
 * EST States - substates for connection establishment
 */
enum
{
        EST_IDLE                     = 0, /* no connection establishment */
        EST_SEND                     = 1, /* conn establishment for send */
        EST_RTX                      = 2, /* conn est for retransmission */
        EST_CMD                      = 3, /* conn est for command        */
        EST_SMMA                     = 4  /* conn est for smma message   */
};

/* N950 Memory Optimization - Implements Measure #39*/
EXTERN  T_S2I_STRING const
        STATE_EST_NAME[];
/*
 * SMS manages 3 (MAX_STATE) states: MMI, NET, EST
 */
#ifdef OPTION_MULTITHREAD
  #define PROCESS_NAME    _ENTITY_PREFIXED(PROCESS_NAME)
#endif

#define STATE_MMI                      0
#define STATE_NET                      1
#define STATE_EST                      2
#define MAX_STATE                      3

/* N950 Memory Optimization - Implements Measure #39*/
EXTERN  T_S2I_STRING const
        PROCESS_NAME[];
/*
 * A definition of the states is provided in SMS_PEI.C
 */
#if defined(SMS_PEI_C) && ! defined(NTRACE)
/* N950 Memory Optimization - Implements Measure #39*/
GLOBAL T_S2I_STRING const STATE_MMI_NAME[] = 
{

  S2I_STRING("MMI_IDLE"),             
  S2I_STRING("MMI_READ"),             
  S2I_STRING("MMI_WRITE"),            
  S2I_STRING("MMI_DELETE"),           
  S2I_STRING("MMI_FIND_FIRST"),       
  S2I_STRING("MMI_FIND_NEXT"),        
  S2I_STRING("MMI_REPLACE"),          
  S2I_STRING("MMI_STATE_UPDATE"),     
  S2I_STRING("MMI_READ_STATE_UPDATE"),
  S2I_STRING("MMI_RESUME")
};
#ifndef FF_SMS_23430
GLOBAL T_S2I_STRING const STATE_NET_NAME[] = 
{
  S2I_STRING("NET_IDLE"), 
  S2I_STRING("NET_WRITE"),
  S2I_STRING("NET_READ")
};
#else
GLOBAL T_S2I_STRING const STATE_NET_NAME[] = 
{
  S2I_STRING("NET_IDLE"),       
  S2I_STRING("NET_WRITE"),      
  S2I_STRING("NET_READ"),       
  S2I_STRING("NET_23430_WRITE"),
  S2I_STRING("NET_23430_DELETE")
};
#endif /* else, #ifndef FF_SMS_23430 */

GLOBAL T_S2I_STRING const STATE_EST_NAME[] = 
{

  S2I_STRING("EST_IDLE"),
  S2I_STRING("EST_SEND"),
  S2I_STRING("EST_RTX"), 
  S2I_STRING("EST_CMD"), 
  S2I_STRING("EST_SMMA")
};

GLOBAL T_S2I_STRING const   PROCESS_NAME[] = 
{

  S2I_STRING("MMI"),
  S2I_STRING("NET"),
  S2I_STRING("EST")
};
#endif

#define ACCESS_BY_MMI                  0x01
#define ACCESS_BY_NET                  0x02
#define ACCESS_BY_EST                  0x03

#define SIM_SMS_FREE                   (SMS_RECORD_FREE)
#define SIM_SMS_MT_READ                (SMS_RECORD_REC_READ)
#define SIM_SMS_MT_TO_BE_READ          (SMS_RECORD_REC_UNREAD)
#define SIM_SMS_MO_SENT                (SMS_RECORD_STO_SENT)
#define SIM_SMS_MO_TO_BE_SEND          (SMS_RECORD_STO_UNSENT)
#define SIM_SMS_PENDING                0xFF  /* new entry, not yet confirmed */

#define SMS_MAX_SIM_ACCESS     2

/*==== TYPES ======================================================*/

/*
 * IE Header Structure
 */
typedef struct
{
   UBYTE     iei;                   /* Information Element Identifier     */
   UBYTE     ie_length;             /* Length of Information Element      */
} T_SMS_PDU_IE_HDR;

/*
 * Conactenation header structure for 8 bit reference number
 */
typedef struct
{
   UBYTE     ref_nr;                /* Reference Number                   */
   UBYTE     max_nr;                /* Maximum number of short messages
                                       in the concatenated short message  */
   UBYTE     seq_nr;                /* Sequence number of the current 
                                                            short message */
} T_SMS_PDU_CONCAT_8BIT_HDR;

/*
 * Conactenation header structure for 16 bit reference number
 */
typedef struct
{
   USHORT    ref_nr;                /* Reference Number                   */
   UBYTE     max_nr;                /* Maximum Sequence Number            */
   UBYTE     seq_nr;                /* Sequence Number                    */
} T_SMS_PDU_CONCAT_16BIT_HDR;

/*
 * Conactenation header common structure
 */
typedef struct
{
   USHORT    ref_nr;                /* Reference Number                   */
   UBYTE     max_nr;                /* Maximum Sequence Number            */
   UBYTE     seq_nr;                /* Sequence Number                    */
} T_SMS_CONCAT_HDR;

typedef struct
{
  UBYTE      t_mode;
  ULONG      t_val;
} T_TIMER_CONFIG;


typedef struct
{
  T_MMSMS_DATA_REQ *mo_msg;
  T_sms_sdu      *sms_sdu;
  T_SIM_READ_RECORD_CNF *sim_read;
  UBYTE          state[MAX_STATE_SMS_INST];
  UBYTE          tl_retx;           /* Number of retransmissions on TL level */
#ifdef REL99
  UBYTE          failed_msg_retx;   /* stores whether MO msg is sent thru retransmission request */ 
  UBYTE          failed_msg_rec_num;/* stores record number of last failed msg */ 
  UBYTE          failed_msg_mem;    /* stores memory location of last failed msg */ 
#endif
  UBYTE          retx;
  UBYTE          retrans;
  UBYTE          msg_ref;
  UBYTE          ti;
  UBYTE          tp_mr;
  UBYTE          tp_mr_ret;         /* TP-MR to be returned to higher layer */

  /* cp_release_req() in state XXX_CP_WAIT_FOR_ACK received,
   * delay the release until the next CP state is entered. */
  BOOL           r_flag;
  /* CP_ACK pending: outstanding CP_ACK in outbound direction */
  BOOL           cp_ack_pending;
  UBYTE          act_mem;
  UBYTE          act_record;
  UBYTE          act_status;
  UBYTE          act_rdmode;
  UBYTE          act_condx;
  UBYTE          act_modifier;
  UBYTE          pid;
  UBYTE          dcs;
#if defined (GPRS)
  UBYTE          downlink;          /* downlink of this instance: LLC or MMSMS */
  UBYTE          ack_type;          /* type of message to be sent */
  UBYTE          cp_cause;          /* cause value of CP-ERROR to be sent */
  T_cp_user_data_dl *cp_user_data_dl;
#endif
  BOOL           t_running[MAX_SMS_TIMER];
} T_SMS_INST;

typedef struct
{
  UBYTE          *status_field;     /* status each record */
  UBYTE          *pid_field;        /* PID flag of each record */
  UBYTE           mem_type;         /* Type of used memory */
  UBYTE           max_record;       /* number of available records */
  UBYTE           any_valid;        /* at least one used record exists */
} T_BACKUP;

typedef struct
{
  /* SIM EF file identifier. path_info need not be stored as SMS is using 
   * only non-conflicting files 
   * Data-field identification
   */
  USHORT         datafield;
  /* record number, used only for linear fixed EFs */
  UBYTE          rec_num;
  /* flag indicating whether this entry is used */
  UBYTE          entry_used;  
} T_SMS_SIM_ACCESS_INFO;

/*
 * CONCATENATION CONTROL
 */
typedef struct
{
  BOOL           concatenation;    /* whether concatenated submits are
                                      currently processed                    */
  BOOL           release_pending;  /* release outstanding                    */
  BOOL           end;              /* end of concatenated stream             */
  BOOL           pad;              /*                                        */
  T_SMS_CONCAT_HDR nr;             /* current number                         */
} T_SMS_CONCAT_CNTRL;

/*
 * PAUSE RESUME CONTROL
 */
typedef struct
{
  U8             delivery_state;  /* pause or resume, SMS_DELIVER_STATUS_xxx */
  BOOL           mem_full_sent;   /* whether mem capacity exceeded
                                     message was sent to HLR due to
                                     user pause request                      */
  U16            save_cause;      /* safed cause value while sim updating    */
} T_SMS_PR_CNTRL;

typedef struct
{
  T_SMS_INST     data[MAX_SMS_CALLS];
  T_rp_data_dl  *rp_data_dl;        /* points to income RL data */
 /*
  * following is a hack: since we cannot pass values to timer we safe it here:
  */
  UBYTE          timer_ti;          /* TI for running timer */
  UBYTE          mhc;               /* message Handling compatibility */
  UBYTE          mt;
  UBYTE          ds;
  UBYTE          mem3;
  UBYTE          inst;
  UBYTE          ent_state;
  UBYTE          state[MAX_STATE];
  BOOL           mem_cap_avail;     /* as stated on SIM */
  UBYTE          access_fifo[2];
  UBYTE          sim_phase;         /* value of EF(Phase) */
  UBYTE          use_mem_a;
  UBYTE          use_mem_b;
  T_BACKUP       me_backup;
  T_BACKUP       sim_backup;
  
  /* In the ME status field a backup of the SMS status is stored, 
   * the ME PID field is a bitarray which stores whether a given 
   * SMS is of the replacement type or not.
   * For asynchronous memory those fields are mandatory, for 
   * synchronous memory the backup is optional.
   */
  /* UBYTE          me_status_field [MAX_RECORD_ME]; */
  /* UBYTE          me_pid_field [(MAX_RECORD_ME+7)/8]; */
  UBYTE          sim_status_field [MAX_RECORD_SIM];
  UBYTE          sim_pid_field[(MAX_RECORD_SIM+7)/8];

  /* SMS entity has finished reading SMS entries from the SIM. */
  BOOL           init_done;

  /* SMS entries changed by SAT, not begun reading records from the SIM. */
  BOOL           sim_mem_update;

  /* concatenation control field  */
  T_SMS_CONCAT_CNTRL concat_cntrl;
  /* pause resume control field   */
  T_SMS_PR_CNTRL pr_cntrl;

  UBYTE          nw_spec;
#ifdef FF_CPHS
  UBYTE          cphs_sim;          /* CPHS SIM recognized */
#endif
#ifdef SIM_TOOLKIT
  UBYTE          download_sms;
  T_SIM_FILE_UPDATE_IND *file_update_ind;
  /* MMI state backup */
  UBYTE stored_mmi_state;
  /* Backup the read_cnf primitive */
  T_SIM_READ_RECORD_CNF *stored_read_cnf;
#endif /* #ifdef SIM_TOOLKIT */
#if defined (GPRS)
  UBYTE          mo_dst_pref;       /* preferred dest. (GPRS/CCT) for MO SM */
  UBYTE          llc_flow;          /* LLC flow control */
  UBYTE          sms_flow;          /* SMS entity flow control */
#endif
  /* To indicate that release is pending for the previous connection */
  BOOL           cmms_release_pending;
  UBYTE          cmms_mode;
  T_TIME timer_values[MAX_SMS_TIMER];
  /* Used to store information on the SIM Accesses that are currently happening. 
   * The array is indexed by the req_id that was sent in SIM_XXX_REQ */
  T_SMS_SIM_ACCESS_INFO    sms_sim_access_info[SMS_MAX_SIM_ACCESS];
} T_SMS_DATA;

/*==== MACROS =====================================================*/

#define DATA_INST(_sd_)         ((_sd_)->data[(_sd_)->inst])

#define SMS_STATE(_sd_)         (DATA_INST(_sd_).state)
#define SMS_RETX(_sd_)          (DATA_INST(_sd_).retx)
#define SMS_RETRANS(_sd_)       (DATA_INST(_sd_).retrans)
#define SMS_CP_REF(_sd_)        (DATA_INST(_sd_).ti)
#define SMS_RP_REF(_sd_)        (DATA_INST(_sd_).msg_ref)
#define SMS_TP_REF(_sd_)        (DATA_INST(_sd_).tp_mr)
#define SMS_TP_REF_RET(_sd_)    (DATA_INST(_sd_).tp_mr_ret)
/* #define SMS_RP_FLAG(_sd_)       (DATA_INST(_sd_).r_flag) */ /* Not used */
#define SMS_SEL_MEM(_sd_)       (DATA_INST(_sd_).act_mem)
#define SMS_SEL_REC(_sd_)       (DATA_INST(_sd_).act_record)
#define SMS_REC_STATUS(_sd_)    (DATA_INST(_sd_).act_status)
#define SMS_READ_MODE(_sd_)     (DATA_INST(_sd_).act_rdmode)
#define SMS_CONDX(_sd_)         (DATA_INST(_sd_).act_condx)
#define SMS_MODIFY(_sd_)        (DATA_INST(_sd_).act_modifier)
#define SMS_DATA_REQ(_sd_)      (DATA_INST(_sd_).mo_msg)
#define SMS_SDU(_sd_)           (DATA_INST(_sd_).sms_sdu)
#define SMS_SIM_READ(_sd_)      (DATA_INST(_sd_).sim_read)
#define SMS_RP_RCVD(_sd_)       ((*_sd_).rp_data_dl)
#define SMS_PID(_sd_)           (DATA_INST(_sd_).pid)
#define SMS_DCS(_sd_)           (DATA_INST(_sd_).dcs)

#define SMS_ENT_STATE(_sd_)     ((_sd_)->ent_state)
#define SMS_SIM_PHASE(_sd_)     ((_sd_)->sim_phase)
#define SMS_SIM_STATE(_sd_)     ((_sd_)->state)
#define SMS_SIM_MEM_AVAIL(_sd_) ((_sd_)->mem_cap_avail)
#define SMS_SIM_ACCESS(_sd_)    ((_sd_)->access_fifo)
#define SMS_SAT_DWNLD(_sd_)     ((_sd_)->download_sms)
#define SMS_MT_ACK_MODE(_sd_)   ((_sd_)->mhc)
#define SMS_MT_HANDLING(_sd_)   ((_sd_)->mt)
#define SMS_ST_REPORT(_sd_)     ((_sd_)->ds)
#define SMS_MEM_CFG(_sd_)       ((_sd_)->mem3)
#define SMS_USE_MEMA(_sd_)      ((_sd_)->use_mem_a)
#define SMS_USE_MEMB(_sd_)      ((_sd_)->use_mem_b)
#define SMS_SIM_PROP(_sd_)      ((_sd_)->sim_backup)
#define SMS_ME_PROP(_sd_)       ((_sd_)->me_backup)
#define SMS_NETWORK(_sd_)       ((_sd_)->nw_spec)

#ifdef FF_CPHS
#define SMS_CPHS(_sd_)          ((_sd_)->cphs_sim)
#endif /* #ifdef FF_CPHS */

#if defined (GPRS)
#define SMS_ROUTE(_sd_)         (DATA_INST(_sd_).downlink)
#define SMS_CP_ACK_TYPE(_sd_)   (DATA_INST(_sd_).ack_type)
#define SMS_CP_CAUSE(_sd_)      (DATA_INST(_sd_).cp_cause)
#define SMS_CP_UDL(_sd_)        (DATA_INST(_sd_).cp_user_data_dl)
#define SMS_ROUTE_PREF(_sd_)    ((_sd_)->mo_dst_pref)
#define SMS_LLC_FLOW(_sd_)      ((_sd_)->llc_flow)
#define SMS_SMS_FLOW(_sd_)      ((_sd_)->sms_flow)
#endif


/*==== EXPORT =====================================================*/

/*
 * Prototypes Formatter
 */
EXTERN void for_init_sms             (void);

/*
 * Prototypes Transfer-Layer Primitives
 */

EXTERN void tl_init                  (void);
EXTERN void tl_init_complete         (void);
EXTERN void tl_mnsms_configure_req   (T_MNSMS_CONFIGURE_REQ   *configure_req);
#ifdef GPRS
EXTERN void tl_mnsms_mo_serv_req     (T_MNSMS_MO_SERV_REQ     *mo_serv_req);
#endif
#ifdef SIM_PERS_OTA
EXTERN void tl_mnsms_OTA_message_res  (T_MNSMS_OTA_MESSAGE_RES *mnsms_OTA_message_res);
#endif
EXTERN void tl_mnsms_command_req     (T_MNSMS_COMMAND_REQ     *command_req);
EXTERN void tl_mnsms_delete_req      (T_MNSMS_DELETE_REQ      *delete_req);
EXTERN void tl_mnsms_read_req        (T_MNSMS_READ_REQ        *read_req);
EXTERN void tl_mnsms_store_req       (T_MNSMS_STORE_REQ       *store_req);
EXTERN void tl_mnsms_submit_req      (T_MNSMS_SUBMIT_REQ      *submit_req);
EXTERN void tl_mnsms_pause_req       (T_MNSMS_PAUSE_REQ       *pause_req);
EXTERN void tl_mnsms_resume_req      (T_MNSMS_RESUME_REQ      *resume_req);
EXTERN void tl_mnsms_query_req       (T_MNSMS_QUERY_REQ       *query_req);
EXTERN void tl_sim_sms_insert_ind    (T_SIM_SMS_INSERT_IND    *insert_ind);
#if defined(SIM_TOOLKIT) OR defined(FF_CPHS)
EXTERN void tl_sim_read_cnf          (T_SIM_READ_CNF          *read_cnf);
#endif /* #if defined(SIM_TOOLKIT) OR defined(FF_CPHS) */
#ifdef SIM_TOOLKIT
EXTERN void tl_sim_file_update_ind   (T_SIM_FILE_UPDATE_IND   *file_update_ind);
#endif /* #ifdef SIM_TOOLKIT */
EXTERN void tl_sim_update_cnf        (T_SIM_UPDATE_CNF        *update_cnf);
EXTERN void tl_sim_update_record_cnf (T_SIM_UPDATE_RECORD_CNF *update_cnf);
EXTERN void tl_sim_read_record_cnf   (T_SIM_READ_RECORD_CNF   *read_cnf);
EXTERN void tl_sim_remove_ind        (T_SIM_REMOVE_IND        *remove_ind);
#if defined(SIM_TOOLKIT) AND defined(SAT_SMS_DNL_SUPPORT)
EXTERN void tl_sim_toolkit_cnf       (T_SIM_TOOLKIT_CNF       *toolkit_cnf);
#endif /* #if defined(SIM_TOOLKIT) AND defined(SAT_SMS_DNL_SUPPORT) */
EXTERN void tl_mnsms_error_ind       (USHORT                  error);
EXTERN void tl_mnsms_ack_res         (T_MNSMS_ACK_RES         *ack_res);
EXTERN UBYTE tl_check_class_2        (UBYTE                    dcs);
EXTERN BOOL  tl_build_envelope_sms_download (T_rp_data_dl     *rp_data_dl);
EXTERN void tl_mnsms_read_cnf        (UBYTE                   mem_type,
                                      UBYTE                   record,
                                      UBYTE                   *data,
                                      USHORT                  error);



EXTERN void tl_mnsms_store_cnf       (UBYTE                   mem_type,
                                      UBYTE                   record,
                                      USHORT                  error);
EXTERN void tl_mnsms_submit_cnf      (UBYTE                   mem_type,
                                      UBYTE                   record,
                                      UBYTE                   tp_mr,
                                      USHORT                  cause,
                                      T_sms_sdu               *sms_sdu);
EXTERN void tl_mnsms_command_cnf     (UBYTE                   tp_mr,
                                      USHORT                  cause,
                                      T_sms_sdu               *sms_sdu);
EXTERN void tl_mnsms_delete_cnf      (UBYTE                   mem_type,
                                      UBYTE                   record,
                                      UBYTE                   next_rec,
                                      USHORT                  error);
EXTERN void tl_mnsms_resume_cnf      (USHORT                  cause);
EXTERN void tl_mnsms_query_cnf       (U8                      query_type,
                                      U8                      status);

/*
 * Prototypes Transfer-Layer Signals
 */
EXTERN void tl_establish_cnf         (BOOL            success);
EXTERN void tl_establish_cnf_send    (void);
EXTERN void tl_establish_cnf_rtx     (void);
EXTERN void tl_establish_cnf_cmd     (void);
EXTERN void tl_data_ind              (T_rp_data_dl   *rp_data_dl);
EXTERN void tl_report_ind            (T_rp_user_data *rp_user_data,
                                      USHORT         cause);
EXTERN void tl_timeout_ind           (USHORT         timer_id);


/*
 * Prototypes Transfer-Layer Functions
 */
EXTERN void tl_establish_connection  (BOOL                incr);
EXTERN void tl_build_status_rep      (T_rp_data_dl       *rp_data_dl,
                                      T_MNSMS_STATUS_IND *status_ind);
EXTERN BOOL tl_modify_submit (T_SIM_PDU *sim_pdu,
                              UBYTE     modify,
                              T_sms_sdu *sms_sdu,
                              UBYTE     *rec_data);
EXTERN BOOL tl_prepare_submit (T_SIM_PDU   *sim_pdu,
                               T_U_CP_DATA *cp_data);
EXTERN void tl_build_deliver_rep     (T_MNSMS_ACK_RES     *ack_res,
                                      UBYTE               msg_ref,
                                      T_MMSMS_DATA_REQ    *data_req,
                                      T_U_CP_DATA         *cp_data);
EXTERN USHORT tl_convert_sim_error   (USHORT sim_error);
EXTERN BOOL tl_convert_mo_to_mem     (T_MMSMS_DATA_REQ   *data_req,
                                      UBYTE              *data);
EXTERN void tl_convert_mt_to_mem     (T_rp_data_dl       *rp_data_dl,
                                      UBYTE              *data);
EXTERN UBYTE tl_convert_sim_to_mmi   (UBYTE        mem_type,
                                      UBYTE        record,
                                      UBYTE        *data,
                                      UBYTE        length);
EXTERN void tl_message_ind_from_sim  (UBYTE        mem_type,
                                      UBYTE        record,
                                      UBYTE        max_record,
                                      UBYTE        *data);
EXTERN void tl_message_ind_from_net  (UBYTE        mem_type,
                                      UBYTE        record,
                                      UBYTE        max_record,
                                      T_rp_data_dl *rp_data_dl);
EXTERN void tl_read_me_memory        (USHORT       index,
                                      UBYTE        status);
EXTERN void tl_read_sim_memory       (USHORT       index,
                                      UBYTE        status);
EXTERN void tl_sim_read_req          (USHORT       datafield,
                                      UBYTE        length);
EXTERN void tl_sim_read_record_req   (UBYTE        record);
EXTERN void tl_sim_update_req        (UBYTE        record,
                                      UBYTE        status,
                                      UBYTE        *data);
EXTERN void tl_sim_conv_update_req   (UBYTE        record,
                                      T_rp_data_dl *rp_data_dl);
EXTERN void tl_handle_message        (T_TP_DELIVER *sms_deliver);
#ifdef FF_SMS_23430
EXTERN BOOL tl_handle_23430          (T_TP_DELIVER *sms_deliver);
#endif /* #ifdef FF_SMS_23430 */
EXTERN void tl_build_sim_update_req  (void);
EXTERN UBYTE tl_read_access_fifo     (void);
EXTERN void tl_set_access_fifo       (UBYTE        access);
EXTERN BOOL tl_sms_memo_exceeded     (BOOL         avail);
EXTERN void tl_sms_memo_pause        (void);
EXTERN void tl_sms_memo_resume       (void);
EXTERN void tl_mnsms_report_ind      (UBYTE        ent_state);
EXTERN void tl_read_sim_message      (USHORT       index);
EXTERN void tl_find_first            (UBYTE        mem_type);
EXTERN void tl_find_next             (UBYTE        mem_type,
                                      UBYTE        index);
EXTERN void tl_adjust_message_len    (UBYTE        tp_vt_mti,
                                      BUF_tpdu     *tpdu);
EXTERN void tl_get_pid_dcs           (UBYTE        status,
                                      T_sms_sdu    *sms_sdu,
                                      UBYTE        *pid,
                                      UBYTE        *dcs);
EXTERN BOOL tl_find_status_pid       (UBYTE        mem_type,
                                      UBYTE        index,
                                      UBYTE        *data,
                                      T_BACKUP     *backup);
EXTERN UBYTE tl_get_free_space       (UBYTE        mem_type);
EXTERN void  tl_store_pid            (T_BACKUP     *backup,
                                      unsigned     index,
                                      UBYTE        pid);
EXTERN void  tl_store_status         (T_BACKUP     *backup,
                                      unsigned     index,
                                      UBYTE        status);
EXTERN UBYTE tl_get_status           (T_BACKUP     *backup,
                                      unsigned     index);
EXTERN int   tl_check_mt_pid         (T_BACKUP     *backup,
                                      unsigned     index,
                                      UBYTE        pid);
EXTERN UBYTE tl_check_status_value   (UBYTE        status);
EXTERN int   tl_search_record        (T_BACKUP     *backup,
                                      USHORT       index,
                                      UBYTE        status);
EXTERN UBYTE tl_search_record_for_delete (T_BACKUP *backup,
                                          UBYTE   index,
                                          UBYTE    status);
EXTERN BOOL  tl_check_replace_entry  (UBYTE        *new_data,
                                      UBYTE        *old_data);
EXTERN void  tl_replace_message      (T_TP_DELIVER *sms_deliver);
EXTERN void  tl_sim_toolkit_confirm  (T_SIM_TOOLKIT_CNF *toolkit_cnf);
EXTERN void  tl_cmms_start           (void);
EXTERN void  tl_cmms_end             (void);
EXTERN void  tl_mnsms_cmms_end_ind   (void);
#ifdef FF_SMS_NW_RCG_SIM
EXTERN void  tl_check_network_on_sim (UBYTE        *data);
#endif /* #ifdef FF_SMS_NW_RCG_SIM */
EXTERN void  tl_pause                (void);
EXTERN void  tl_resume               (void);
EXTERN void  tl_query_deliver_status (void);
EXTERN BOOL  tl_concat_check         (T_sms_sdu    *sms_sdu);
EXTERN void  tl_concat_cntrl         (T_SMS_CONCAT_HDR *concat_hdr);

#ifdef REL99
EXTERN void tl_mnsms_retrans_req     (T_MNSMS_RETRANS_REQ     *mnsms_retrans_req);
EXTERN void tl_mnsms_retrans_cnf     (UBYTE mem_type,
                                      UBYTE record,
                                      UBYTE tp_mr,
                                      USHORT cause,
                                      T_sms_sdu *sms_sdu );
EXTERN void tl_mnsms_send_prog_ind   ();
#endif


EXTERN BOOL tl_sms_reserve_req_id (UBYTE *acc_ix_ptr);

/*
 * Prototypes Relay-Layer Primitives
 */

EXTERN void rl_init                  (void);

/*
 * Prototypes Relay-Layer Signals
 */
EXTERN void rl_establish_req    (UBYTE              ti);
EXTERN void rl_establish_cnf    (BOOL               success);
EXTERN void rl_release_req      (UBYTE              ti);
EXTERN void rl_data_ind         (T_cp_user_data_dl *cp_user_data_dl);
EXTERN BOOL rl_data_req         (UBYTE             msg_ref,
                                 T_U_CP_DATA       *cp_data);
EXTERN void rl_error_ind        (USHORT            cause);
EXTERN void rl_report_req_ack   (T_rp_user_data    *rp_user_data);
EXTERN void rl_report_req_error (USHORT            cause,
                                 T_rp_user_data    *rp_user_data);
EXTERN BOOL rl_mem_avail_req    (void);
EXTERN void rl_start_tram_req   (void);

/*
 * Prototypes Relay-Layer Functions
 */

EXTERN void rl_build_rp_error        (UBYTE        msg_ref,
                                      USHORT       error,
                                      T_U_CP_DATA  *cp_data,
                                      T_rp_user_data *rp_user_data);
EXTERN void rl_build_rp_smma         (UBYTE        msg_ref,
                                      T_U_CP_DATA  *cp_data);
EXTERN UBYTE rl_temp_failure         (UBYTE        cause);
#ifdef GPRS

EXTERN void rl_proceed               (void);

EXTERN void rl_build_rp_error_gprs   (UBYTE             ti,
                                      T_LL_UNITDATA_REQ *data_req,
                                      USHORT            error,
                                      UBYTE             msg_ref,
                                      T_U_CP_DATA       *cp_data,
                                      T_stk_cmd         *stk_cmd);
#endif /* #ifdef GPRS */

/*
 * Prototypes Control-Protocol Primitives
 */

EXTERN void cp_init                  (void);
EXTERN void cp_mmsms_error_ind       (T_MMSMS_ERROR_IND       *error_ind);
EXTERN void cp_mmsms_establish_cnf   (T_MMSMS_ESTABLISH_CNF   *establish_cnf);
EXTERN void cp_mmsms_release_ind     (T_MMSMS_RELEASE_IND     *release_ind);
EXTERN void for_mmsms_establish_ind  (T_MMSMS_ESTABLISH_IND   *establish_ind);
EXTERN void for_mmsms_data_ind       (T_MMSMS_DATA_IND        *data_ind);
#if defined (GPRS)
EXTERN void for_ll_unitdata_ind      (T_LL_UNITDATA_IND       *unitdata_ind);
EXTERN void for_ll_unitready_ind     (T_LL_UNITREADY_IND      *unitready_ind);
EXTERN void cp_gmmsms_reg_state_cnf  (T_GMMSMS_REG_STATE_CNF  *reg_state_res);
#endif

/*
 * Prototypes Control-Protocol Signals
 */
EXTERN void cp_establish_req         (UBYTE         ti);
EXTERN void cp_abort_req             (void);
EXTERN void cp_data_ind_cp_ack       (void);
EXTERN void cp_data_ind_cp_data      (T_D_CP_DATA  *cp_data);
EXTERN void cp_data_ind_cp_error     (UBYTE        errcs);
EXTERN void cp_data_ind_cp_unknown   (void);
EXTERN void cp_data_req              (T_U_CP_DATA  *cp_data);
EXTERN void cp_est_ind_cp_ack        (void);
EXTERN void cp_est_ind_cp_data       (T_D_CP_DATA  *cp_data);
EXTERN void cp_est_ind_cp_error      (UBYTE        cp_error);
EXTERN void cp_est_ind_cp_unknown    (void);
EXTERN void cp_release_req           (UBYTE         ti);
#if defined (GPRS)
EXTERN void cp_send_data_gsms        (void);
EXTERN void cp_data_req_gsms         (T_MMSMS_DATA_REQ *rp_data);
EXTERN void cp_error_req_gsms        (UBYTE        cause,
                                      BOOL         free_ti);
EXTERN void cp_send_getunitdata_req  (void);
#endif


/*
 * Prototypes Control-Protocol Functions
 */

EXTERN void cp_add_pd_ti             (UBYTE            ti,
                                      T_sdu            *sdu);
EXTERN void cp_build_cp_ack          (void);
EXTERN void cp_build_cp_error        (UBYTE            error);
EXTERN void cp_send_release_req      (UBYTE            ti);

#if defined (GPRS)
EXTERN void cp_init_ll_unitdata_req (T_LL_UNITDATA_REQ *unitdata_req);
#endif
/*
 * If all entities are linked into one module this definitions
 * prefixes all this functions with the enity name
 */

/*
 * Prototypes Timer
 */
EXTERN void sms_timer_start          (USHORT      id);
EXTERN void sms_timer_stop           (USHORT      id);
EXTERN BOOL sms_timer_check          (USHORT      id);
/*
 * If all entities are linked into one module this definitions
 * prefixes all this functions with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define tim_exec_timeout   _ENTITY_PREFIXED(tim_exec_timeout)
  #define tim_start_timer    _ENTITY_PREFIXED(tim_start_timer)
  #define tim_stop_timer     _ENTITY_PREFIXED(tim_stop_timer)
#endif /* #ifdef OPTION_MULTITHREAD */
EXTERN void tim_exec_timeout (USHORT index);
EXTERN void tim_start_timer  (USHORT index, T_TIME value);
EXTERN void tim_stop_timer   (USHORT index);

EXTERN void tim_tc1m                 (void);
EXTERN void tim_tr1m                 (void);
EXTERN void tim_tr2m                 (void);
EXTERN void tim_tram                 (void);
EXTERN void tim_tlct                 (void);
EXTERN void tim_tmms                 (void);

/*
 *  SMS primitives Engineering Mode
 *  Bitmask for the event tracing
 */

/*
 * Prototypes Customer Specific Functions
 */

EXTERN UBYTE       csf_get_new_mo_ti();
EXTERN T_SMS_DATA *csf_get_sms_instance (UBYTE ti);
EXTERN T_SMS_DATA *csf_get_new_sms_instance (UBYTE ti);
EXTERN void        csf_free_sms_instance (UBYTE ti);

#define TIMERSTART(i,v)                 tim_start_timer (i, v);
#define TIMERSTOP(i)                    tim_stop_timer (i);

/*
 * If all entities are linked into one module this definitions
 * prefixes the global data with the enity name
 */
#ifdef OPTION_MULTITHREAD
#ifdef TI_PS_HCOMM_CHANGE
#else
  #define hCommMMI       _ENTITY_PREFIXED(hCommMMI)
  #define hCommMM        _ENTITY_PREFIXED(hCommMM)
  #define hCommSIM       _ENTITY_PREFIXED(hCommSIM)
#endif /* TI_PS_HCOMM_CHANGE */ 
  #ifdef GPRS
    #define hCommLLC     _ENTITY_PREFIXED(hCommLLC)
    #define hCommGMM     _ENTITY_PREFIXED(hCommGMM)
  #endif /* GPRS */

  #define _decodedMsg   _ENTITY_PREFIXED(_decodedMsg)
  #define _CCDbuf       _ENTITY_PREFIXED(_CCDBuf)
  #define data_base     _ENTITY_PREFIXED(data_base)
#endif /* #ifdef OPTION_MULTITHREAD */
#ifdef TI_PS_HCOMM_CHANGE
#else
EXTERN T_HANDLE         hCommMMI;        /* MMI Communication        */
EXTERN T_HANDLE         hCommSIM;        /* SIM Communication        */
EXTERN T_HANDLE         hCommMM;         /* MM  Communication        */
#endif /* TI_PS_HCOMM_CHANGE */ 
#ifdef GPRS
EXTERN T_HANDLE         hCommLLC;        /* LLC Communication        */
EXTERN T_HANDLE         hCommGMM;        /* GMM Communication        */
#endif /* GPRS */
EXTERN T_HANDLE         sms_handle;

#ifndef SHARED_CCD_BUF
#define CCD_START
#define CCD_END
EXTERN UBYTE          _decodedMsg [];
#else
EXTERN UBYTE *        _decodedMsg;
EXTERN UBYTE          _CCDbuf;
#define CCD_START if(!_CCDbuf){_decodedMsg = ccd_begin();_CCDbuf=TRUE;}
#define CCD_END   if(_CCDbuf){ccd_end();_CCDbuf=FALSE;}
#endif /* else, #ifndef SHARED_CCD_BUF */

EXTERN T_SMS_DATA data_base;

#define GET_INSTANCE(p) &data_base

#define GET_INSTANCE_DATA    register T_SMS_DATA *sms_data= &data_base

#if !defined(NTRACE) && !defined(NCONFIG)
/*
 * If all entities are linked into one module this definitions
 * prefixes the global data with the entity name
 */
#ifdef OPTION_MULTITHREAD
  #define partab        _ENTITY_PREFIXED(partab)
#endif /* #ifdef OPTION_MULTITHREAD */

EXTERN const KW_DATA    partab[];
#endif /* #if !defined(NTRACE) && !defined(NCONFIG) */

/* Implements Measure#32: Row 84, 96, 87, 89, 95, 97, 104, 109 & 113 */
EXTERN const char * const ef_sms_id;

#define SMS_INST_TC1M  ((USHORT)(TC1M*MAX_SMS_CALLS+sms_data->inst))
#define SMS_INST_TR1M  ((USHORT)(TR1M*MAX_SMS_CALLS+sms_data->inst))
#define SMS_INST_TR2M  ((USHORT)(TR2M*MAX_SMS_CALLS+sms_data->inst))
#define SMS_INST_TRAM  ((USHORT)(TRAM*MAX_SMS_CALLS+sms_data->inst))
#define SMS_INST_TLCT  ((USHORT)(TLCT*MAX_SMS_CALLS+sms_data->inst))

#endif /* #ifndef SMS_H */
