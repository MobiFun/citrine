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
|  Purpose :  Declarations for the Protocol Stack Entity
|             Radio Resource
+-----------------------------------------------------------------------------
*/

#ifndef RR_H
#define RR_H

#ifdef GPRS
#include "rr_gprs.h"
#endif

#include "cl_list.h"
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

#define _SIMULATION_FFS_

/* Enum values for clearing White List */
enum clr_white_list_e
{
  CLR_WHITE_LIST_RAM = 1,
  CLR_WHITE_LIST_SIM = (CLR_WHITE_LIST_RAM << 1),
  CLR_WHITE_LIST_FFS = (CLR_WHITE_LIST_SIM << 1)
};

/* Enum values for clearing Black List
 * CSI-LLD section:4.1.1.4
 */
enum clr_black_list_e
{
  CLR_BLACK_LIST_RAM = 1,
  CLR_BLACK_LIST_FFS = (CLR_BLACK_LIST_RAM << 1)
};

/* Enum values for identifying scan mode
 * CSI-LLD section:4.4
 */
enum scan_mode_e
{
  CS_NO_SCAN,
  CS_FIRST_SCAN_FIRST_ATTEMPT,
  CS_FIRST_SCAN_SECOND_ATTEMPT,
  CS_SECOND_SCAN
};

/* Enum values for Initial PLMN search state
 * CSI-LLD section:4.1.1.4
 */
enum initial_plmn_search_e
{
  INITIAL_PLMN_SEARCH_NOT_ACTIVE,
  INITIAL_PLMN_SEARCH_ACTIVE,
  INITIAL_PLMN_SEARCH_DONE
};

/*
 * look at explanations at rr_data->mode_after_dedi!
 */
#define ARRAY_TRACE

#if defined(_SIMULATION_)
#undef  SET_STATE
#define SET_STATE(PROCESS,STATE)\
            { vsi_o_ttrace (VSI_CALLER 0x08, "%s:%s->%s (#%u %s)",\
                            PROCESS_NAME[PROCESS],\
                            PROCESS##_NAME [ ENTITY_DATA->state[PROCESS] ],\
                            PROCESS##_NAME [              STATE          ],\
                            __LINE__, __FILE10__);\
            ENTITY_DATA->state[PROCESS] = STATE;}
#endif /* _SIMULATION_ */

#if !defined(NTRACE)
#define TRACE_DATA_IND(a,fn,t)   TRACE_EVENT_P5 ("[%u] %s FN=%lu CR=%d SC=%d",\
                                  (a)&ARFCN_MASK, t, fn,\
                                  (short)(rr_data->nc_data[CR_INDEX].arfcn),\
                                  (short)(rr_data->nc_data[SC_INDEX].arfcn))
#else  /* !NTRACE */
#define TRACE_DATA_IND(a,fn,t)
#endif  /* !NTRACE */



#if !defined(NTRACE)
#define TRACE_DATA_IND(a,fn,t)   TRACE_EVENT_P5 ("[%u] %s FN=%lu CR=%d SC=%d",\
                                  (a)&ARFCN_MASK, t, fn,\
                                  (short)(rr_data->nc_data[CR_INDEX].arfcn),\
                                  (short)(rr_data->nc_data[SC_INDEX].arfcn))
#else  /* !NTRACE */
#define TRACE_DATA_IND(a,fn,t)
#endif  /* !NTRACE */

#define  RR_ALLOCATE_NEW_BA(v)  ((((v)+1) % (RR_BA_HIGH-RR_BA_LOW+1)) + RR_BA_LOW)

/*lint -esym(526,std) : not defined | defined in PCM */
EXTERN UBYTE std;

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
EXTERN const UBYTE std_bands[12];
#else
EXTERN const UBYTE std_bands[8];
#endif


/*
 * CS priorities
 */
#define CS_LOW_PRIORITY             0x00
#define CS_MID_PRIORITY             0x01
#define CS_HIGH_PRIORITY            0x02
#define CS_PRIORITY                 0x03
#define CS_NON_CHECKED_BITS         0xF7
#define CS_CHECK_FLAG               0x08
#define CS_BCCH_READ                0x10
#define CS_SET_TO_LOW_PRIORITY      0xFC
#define CS_BLACK_LIST_FLAG          0x04 /* CSI-LLD section:4.1.1.3 */

#define SC_INDEX 6
#define CR_INDEX 7
#define NCELL_SIZE 8


#define LOW_PRIORITY_CELL           0x40
#define EMERGENCY_CELL              0x20
#define CHECKED_FLAG                0x08

#define MAX_MCC_SHIELD              4

/* CSI-LLD section:4.1.1.3  */
#define MAX_SYNC_FAILURES           5
#define MAX_SFC_PER_REGION          512

#define STD_NOT_PRESENT             0x00

/*
 * Macros
 */
#ifdef FRAME_OFFSET_ZERO

#define GET_PD(s,p)               p=(UBYTE)(s.buf[3] & 0x0F)
#define GET_TI(s,t)               t=(UBYTE)(s.buf[3] >> 4  )

#define GET_UI_PD(s,p)            p=(UBYTE)(s.buf[1] & 0x0F)
#define GET_UI_TI(s,t)            t=(UBYTE)(s.buf[1] >> 4  )

#else

#define GET_PD(s,p)               ccd_decodeByte(s.buf, (USHORT)(s.o_buf+4), 4, &p)
#define GET_TI(s,t)               ccd_decodeByte(s.buf, (USHORT)(s.o_buf  ), 4, &t)

#define GET_UI_PD(s,p)            ccd_decodeByte(s.buf, (USHORT)(s.o_buf+4), 4, &p)
#define GET_UI_TI(s,t)            ccd_decodeByte(s.buf, (USHORT)(s.o_buf  ), 4, &t)

#endif

#define USE_STORED_ENTRIES()      { ENTITY_DATA->use_stored_entries = TRUE; }

/* add resp. get the std value to resp. from arfcn */
/*#define STD_ADD_TO_ARFCN(a,std)  (((a)&~STD_BIT_MASK) | ((std)<<STD_BIT_SHIFT))*/
#define STD_GET_FROM_ARFCN(a)    ((((a)&STD_BIT_MASK)>>STD_BIT_SHIFT))


/* Returns the region(european or american)the requested carrier belongs to
 * CSI-LLD section:4.1.1.2.1
 */
#define CS_GET_REGION_FROM_FREQ(arfcn) ((arfcn&US_BIT)?AMERICAN_REGION:EUROPEAN_REGION)

/* Sets the current search mode
 * CSI-LLD section:4.1.3.1.1
 */
#define CS_SET_CURRENT_SEARCH_MODE(mode) (rr_data->cs_data.current_search_mode=mode)

/* Returns the current search mode
 * CSI-LLD section:4.1.3.1.1
 */
#define CS_GET_CURRENT_SEARCH_MODE (rr_data->cs_data.current_search_mode)

/* Sets the previous search mode
 * CSI-LLD section:4.1.3.1.1
 */
#define CS_SET_PREVIOUS_SEARCH_MODE(mode) (rr_data->cs_data.previous_search_mode=mode)

/* Returns the previous search mode
 * CSI-LLD section:4.1.3.1.1
 */
#define CS_GET_PREVIOUS_SEARCH_MODE (rr_data->cs_data.previous_search_mode)

/* This macro sets Bit : 2 of attributes[  ] field n T_CS_DATA structure for the
 * carrier identified by the index. This bit indicates whether a carrier is a
 * candidate for "Black List" or not. The BLACK_LIST_FLAG in the attribute filed
 * shall be set during initial PLMN search( as indicated by the flag
 * ini-tial_plmn_search_active) whenever MS fails to synchronize to a carrier.
 * After the completion of initial PLMN search, Black list flag shall be used to
 * update the Black List database based on the outcome of initial PLMN search.
 * CSI-LLD section:4.1.1.2.2
 */
#define CS_SET_BLACK_LIST_FLAG(index) (rr_data->cs_data.attributes[index]|=CS_BLACK_LIST_FLAG)

/* Returns the value of bit:2 of attributes[]field in T_CS_DATA structure for the
 * the carrier identified by index. This bit indicates whether a carrier is a
 * candidate for "Black List" or not. This macro will be called while updating Black
 * list database following the completion of initial PLMN search.
 * CSI-LLD section:4.1.1.2.3
 */
#define CS_GET_BLACK_LIST_FLAG(index) (rr_data->cs_data.attributes[index]&CS_BLACK_LIST_FLAG)

/*
 * Protocol Discriminator and
 * Transaction Identifier of RR
 */
#define PD_RR_TI_0                      0x06
/*
 * Ignore Value
 */
#define IG                NOT_PRESENT_8BIT

/*
 * Paging Indication
 */
#define ANYCH                            0
#define SDCCHCH                          1
#define TCHFCH                           2
#define TCHHFCH                          3
#define TCHHCH                           4

/*
 * MS Capability
 */
#define FULL                             0
#define DUAL                             1
#define SIGN                             2

/*
 * Maximum value for cell ranking
 */
#define MAX_C2                         255

/*
 * Power Classes
 */
#define MAX_CLASSES                      5
#define P_CLASS_2_900                   39
#define P_CLASS_3_900                   37
#define P_CLASS_4_900                   33
#define P_CLASS_5_900                   29
#define P_CLASS_1_1800                  30
#define P_CLASS_2_1800                  24
#define P_CLASS_3_1800                  36
#define P_CLASS_1_1900                  30
#define P_CLASS_2_1900                  24
#define P_CLASS_3_1900                  33

/*
 * Values for BSIC status
 */
#define EMPTY                            0
#define NON_DECODED                      1
#define DECODED                          2

#define CELL_IS_INSERTED                 0
#define BSIC_HAS_CHANGED                 1
#define CELL_IS_NOT_INSERTED             2

#ifdef GPRS
#define CR_PAR_INVALID                   0
#define CR_PAR_VALID                     1
#endif
/*
 * Times defined by multiplies of TDMA frames
 */
#define PERIOD_1_SEC            217L
#define PERIOD_700_SEC       151680L

/*
 * Hierarchy defined by multiplies of TDMA frames
 */
#define HYPERFRAME              2715648
#define STARTING_TIME_INTERVAL    42432
#define STARTING_TIME_INTERVAL0   10808
#define STARTING_TIME_INTERVAL1   31623
/* The Starting Time IE can encode only an interval of time of 42 432 frames,
 * that is to say around 195.8 s. To remove any ambiguity, the specification
 * for a reception at time T is that the encoded interval is
 * (FN-10808, FN+31623). In rigorous terms, if we note ST the starting time:
 *
 * - if 0 <= (ST-FN) mod 42432 <= 31623, the indicated time is the next time
 *                                       when FN mod 42432 is equal to ST
 * - if 32024 <= (ST-FN) mod 42432 <= 42431, the indicated time has already
 *                                           elapsed.
 * The reception time FN is not specified here precisely. To allow room for
 * various MS implementations, the limit between the two behaviours above may
 * be anywhere within the interval defined by
 *
 * - 31624 <= (ST-FN) mod 42432 <= 32023.
 *
 * 3GPP TS 04.18, section 10.5.2.38 Starting Time
 */


/*
 * Flags for sending rr status message
 */
#define SEND_RR_STATUS            1
#define SEND_NO_RR_STATUS         0

/*
 * Bitoffset for encoding/decoding
 */
#define ENCODE_OFFSET             24

/*
 * Protocol Discrimator
 */
#define PD_CC                     3
#define PD_MM                     5
#define PD_RR                     6
#define PD_SMS                    9
#define PD_SS                     11
#define PD_TST                    15

/*
 * Test Messages
 */
#define CLOSE_TCH_LOOP_CMD        0
#define OPEN_LOOP_CMD             6
#define TEST_INTERFACE         0x14

/*
 * Delay for the CLOSE_TCH_LOOP_ACK to allow L1 switching
 */
#define DELAY_CLOSE_TCH_LOOP_ACK  35

/*
 * Release Causes
 */
#define OPTIONAL_INFO_ERROR       0x66

/*
 * System Information Type Flags
 */
#define CELL_SELECTION            0
#define CELL_RESELECTION          1
#define BACK_FROM_DEDICATED       2
#define CELL_RESELECTION_NC       3
#define BACK_FROM_DEDICATED_RLF   4
#define CELL_RESELECTION_RACH     5
#define CELL_RESELECTION_CR       6
#ifdef GPRS
#define CELL_RESELECTION_ON_GPRS_ACT 7
#endif
#define MM_ORIGINATED             TRUE
#define RR_ORIGINATED             FALSE

#define CS_PARALLEL              1
#define CS_NOT_PARALLEL          0

#define NO_SYS_INFO_READ          0

#ifdef GPRS
#define ALL_SYS_INFO_READ         0x007f
#else
#define ALL_SYS_INFO_READ         0x003f
#endif

#define SYS_INFO_1_READ           0x0001
#define SYS_INFO_2_READ           0x0002
#define SYS_INFO_2BIS_READ        0x0004
#define SYS_INFO_2TER_READ        0x0008
#define SYS_INFO_3_READ           0x0010
#define SYS_INFO_4_READ           0x0020
#if defined (REL99) && defined (TI_PS_FF_EMR)
#define SYS_INFO_2QUATER_READ     0x2000
#endif
#define SYS_INFO_EXCEPT_2TER      (ALL_SYS_INFO_READ & (~SYS_INFO_2TER_READ))

#ifdef GPRS
#define SYS_INFO_13_READ          0x0040
#define SYS_INFO_EXCEPT_SI13      (ALL_SYS_INFO_READ & (~SYS_INFO_13_READ))
#endif

#define SYS_INFO_5_READ             0x0100
#define SYS_INFO_5BIS_READ          0x0200
#define SYS_INFO_5TER_READ          0x0400
#define ALL_DEDI_SYS_INFOS          0x0700

#define NO_AVAILABLE              255

#define SYS_INFO_2_MSG            0x1a
#define SYS_INFO_2bis_MSG         0x02
#define SYS_INFO_2ter_MSG         0x03

#if defined (REL99) && defined (TI_PS_FF_EMR)
#define MAX_MULTI_INST             16  /* max.number of instances in multi-instance msg (SI-2quater or MI)*/
#define MSG_SIZE_EMR              168 /* 21 * 8 */
#define MAND_SIZE_EMR              15  /*Max mandatory bits in EMR*/
#define SC_INFO_SIZE_EMR           23  /*size for serving cell, if included*/
#define NC_INVBSIC_EMR             18  /* size for each neighbour cell, if included in Invalid BSIC structure of EMR*/
#define NC_BMP_EMR                  7  /* size for each neighbour cell, if included in BSIC BMP structure of EMR*/
#endif

/*
 * Values for the V(SD) bit
 */
#define SET_ONLY                  0
#define SET_AND_RESET             1
#define RESET_ONLY                2

/*
 * Content Flags
 */
#define NO_CONTENT                0
#define WITH_CHANGED_CONTENT      1
#define WITH_CONTENT              2

/*
 * Establishment Causes
 */
#define ESTCS_LOCATION_UPDATING       0x0400
#define ESTCS_PAGING                  0x0480
#define ESTCS_EMERGENCY_CALL          0x04A0
#define ESTCS_REESTABLISHMENT         0x04C0
#define ESTCS_MOC_SPEECH              0x04E0
#define ESTCS_MOC_DATA                0x04E1
#define ESTCS_MOC_DATA_HR_SUFF        0x04E2
#define ESTCS_MOC_SS_SMS              0x04F0

#define MAX_RACH_RETRANS_VAL 4
#define MAX_TX_INTEGER 16

#define RX_QUAL_UNAVAILABLE 0xFF
#ifdef FF_PS_RSSI
#define RX_ACCE_UNAVAILABLE 0xFF
#endif

/*
 * Application Information: constant name synomyms
 */

#define      LAST_SEG ( L_SEG   )   /* 0x00 */
#define  NOT_LAST_SEG (NL_SEG   )   /* 0x01 */
#define     FIRST_SEG ( F_SEG   )   /* 0x00 */
#define NOT_FIRST_SEG (NF_SEG<<1)   /* 0x02 */

#define   APDU_FULL_L2_FRAME  247
#define   MAX_PRIMPART_SIZE   1600                       /* actually defined in "frm_types.h" */
#if defined FF_EOTD
#define   MAX_APDU_SIZE       (MAX_PRIMPART_SIZE-sizeof(T_PRIM_HEADER)-sizeof(T_RRRRLP_DATA_IND))
#endif /* FF_EOTD */

#define NULLSTRING ""
#define ARFCN_NOT_PRESENT       0xFFFF
#define FRAME_NUM_NOT_PRESENT   0xFFFFFFFFL

/*
 * Timer
 */
enum timer_index_e
{
    T3110,           /* Channel Release Timer                             */
    T3122,           /* Wait Indication Timer                             */
    T3126,           /* Imm Assignment Timer                              */
    T_RESELECT,      /* Reselection Timer                                 */
    TREG,            /* Registration Timer                                */
    TABORT,          /* RR_ABORT_IND Timer                                */
    T_NO_RESELECT,   /* exception of path loss criteria after reselection */
    TIM_EXT_MEAS,    /* Extended Measurement                              */
#if defined FF_EOTD
    TAPDU,           /* Application PDU Timer                             */
#endif /* FF_EOTD */
    TNNN,
    TCSVALID,        /* CS measurements are valid or not                  */
    T_DEDICATED_MODE,/* cell re-selection after leaving dedicated mode    */
    T_PLMN_SEARCH,   /* Manual cell selection timer                       */
    T_FAST_CS,       /* controls FAST search                              */
    T_NORMAL_CS,     /* controls NORMAL search                            */
    NUM_OF_RR_TIMERS
};

#define     TIMER_TRACE

#if defined(TIMER_TRACE)
  #define TRACE_TIMER(FMT)              TRACE_EVENT((FMT))
  #define TRACE_TIMER_P1(FMT,P1)        TRACE_EVENT_P1((FMT),(P1))
  #define TRACE_TIMER_P2(FMT,P1,P2)     TRACE_EVENT_P2((FMT),(P1),(P2))
  #define TRACE_TIMER_P3(FMT,P1,P2,P3)  TRACE_EVENT_P3((FMT),(P1),(P2),(P3))
#else
  #define TRACE_TIMER(FMT)
  #define TRACE_TIMER_P1(FMT,P1)
  #define TRACE_TIMER_P2(FMT,P1,P2)
  #define TRACE_TIMER_P3(FMT,P1,P2,P2)
#endif

/*
 * Dynamic Configuration Numbers
 */
#define TIMER_SET                 1
#define TIMER_RESET               2
#define TIMER_SPEED_UP            3
#define TIMER_SLOW_DOWN           4
#define TIMER_SUPPRESS            5
#define TIM_FAST                  6
#define TIM_NORMAL                7

#define NO_SYS_TIME               20
#define FCR                       21
#define SCR                       22
#define DLE                       23
#define FCA                       24
#define FRL                       25
#define FHO                       26
#define GSM_OFFSET                27
#define DCS_OFFSET                28
#define CTO                       29
#define NKC                       30
#define INIT_FFS                  31
#define WRITE_FFS                 32
#define SET_LAST_USED_SC          33
#define IHO                       34   /* lock the DUT to the cell it is already camping */
#define FBLS                      35

#define ID_RESTRICTED_BAND        50
#define ID_MULTISLOT_CLASS        51
#if defined(_TARGET_)
#define ID_FFS_CHECK              52
#endif /* _TARGET_ */
#define ID_CMSP                   54
#define SCS                       55
#define SHIELD                    56

#define RR_MT_CALL                37
#define RR_MT_SMS_0               38
#define RR_MT_SMS_2               39
#define RR_SERVICE_FULL           40
#define RR_SERVICE_LIMITED        41
#define RR_SERVICE_NO             42
#define RR_MO_CONNECT             43
#define ID_PSEUDO_SYNC_HO         44
#define ID_PCM                    45
#define DCS_PCLASS_3              46
#define BL_CS                     60
#define U_RXT                     61
#define M_RXT                     62
#define L_RXT                     63
#define SHOW_BL                   64
#define SHOW_WL                   65
#define SET_NPS_DELAY             66
#if defined (_SIMULATION_FFS_)
#define ERASE_WL                  70
#define ERASE_BL                  71
#endif
#define SET_WL                    72
#define SET_BL                    73
#define SET_WL_REGION             74
#define SET_WL_PLMN               75

#ifdef TI_PS_FF_AT_P_CMD_CTREG
/*
 * %CTREG tab_id value
 */
#define RR_MOD_NOSERVICE_TIME   1
#define RR_MOD_LIMSERVICE_TIME  2
#endif /* TI_PS_FF_AT_P_CMD_CTREG */
typedef enum{
   SI_TYPE_NONE,
   SI_TYPE_2,
   SI_TYPE_2BIS,
   SI_TYPE_2TER,
   SI_TYPE_3,
   SI_TYPE_4,
   SI_TYPE_5,
   SI_TYPE_5BIS
}T_SI_TYPE;

typedef  enum  {
   CR_STATUS_OFF = 0,
   CR_STATUS_ON
}  T_CR;
 
typedef  enum  {
   MOB_ALLOC_BEFORE = 0,
   MOB_ALLOC_AFTER
}  T_MOB_ALLOC_TYPE;


/*
 * Parameter of dynamic configuration commands
 */
#if !defined(NTRACE)
#if defined(RR_PEI_C)
/* Implements Measure#32: Row 36, 39 and 40 */
GLOBAL T_S2I_STRING const _rr_str_FUNC[] = {
  S2I_STRING("LIM"),   /* FUNC_LIM_SERV_ST_SRCH (0x0) limited service state search   */
  S2I_STRING("PLMN"),  /* FUNC_PLMN_SRCH (1) PLMN search                             */
  S2I_STRING("NET")    /* FUNC_NET_SRCH_BY_MMI (0x2) Net search by MMI               */
};
GLOBAL const char* const _rr_str_PARFUNC[] = {
  "",        /* (0) cleared                                              */
  "",        /* (1) invalid value                                        */
  "parallel PLMN " /* FUNC_NET_SRCH_BY_MMI (0x2) Net search by MMI       */
};
/* Implements Measure#32: Row 36, 39 and 40 */
GLOBAL T_S2I_STRING const _rr_str_SERVICE[] = {
  S2I_STRING("NO"),    /* NO_SERVICE (0x0) no service available                      */
  S2I_STRING("LIM"),   /* LIMITED_SERVICE (0x1) Limited service available            */
  S2I_STRING("FULL")   /* FULL_SERVICE (0x2) Full service available                  */
};
GLOBAL const char* const _rr_str_stored_bcch_info[]=
{
  "SI2", "SI2bis", "SI2ter", "SI2"
};
#else   /* RR_PEI_C */
EXTERN const char* const _rr_str_FUNC[];
EXTERN const char* const _rr_str_PARFUNC[];
EXTERN const char* const _rr_str_SERVICE[];
EXTERN const char* const _rr_str_stored_bcch_info[];
#endif  /* RR_PEI_C */
#endif  /* !NTRACE */

/*
 * States of the Attachment
 */
#define ATT_NULL                  0
#define ATT_CS1                   1 /* Power measurement, Sync (MPH_BSIC_REQ/CNF) */
#define ATT_CS2                   2 /* BCCH reading (Cell selection) */
#define ATT_CS3                   3 /* Cell reselection */
#define ATT_DEDICATED             4
#define ATT_IDLE                  5
#define ATT_NO_SERVICE            6
#define ATT_CON_EST               7
#define ATT_CS_INIT               8 /* Initial RR_ACTIVATE_REQ got and Power measurements started */

/* N950 Memory Optimization - Implements Measure #39*/
EXTERN  T_S2I_STRING  const
        STATE_ATT_NAME[];

/*
 * States of the Process Data Transfer
 */
#define DAT_NULL                  0
#define DAT_CELL_RESELECT         1
#define DAT_CHAN_ASS              2
#define DAT_CHAN_ASS_1            3
#define DAT_CHAN_ASS_2            4
#define DAT_PDCH_ASS              5
#define DAT_PDCH_ASS_1            6
#define DAT_PDCH_ASS_2            7
#define DAT_PDCH_ASS_3            8
#define DAT_PDCH_ASS_4            9
#define DAT_CCO_3                 10
#define DAT_CCO_4                 11
#define DAT_CHAN_REL              12
#define DAT_DEDICATED             13
#define DAT_HANDOVER              14
#define DAT_HANDOVER_4            15
#define DAT_HANDOVER_5            16
#define DAT_IDLE                  17
#define DAT_IMM_ASS               18
#define DAT_IMM_ASS_1             19

/* N950 Memory Optimization - Implements Measure #39*/
EXTERN  T_S2I_STRING  const
        STATE_DAT_NAME[];
/*
 * States for short message service (Sapi 3)
 */
#define SMS_IDLE                  0
#define SMS_PENDING               1
#define SMS_ESTABLISHED           2

/* N950 Memory Optimization - Implements Measure #39*/
EXTERN  T_S2I_STRING  const
        STATE_SAPI_3_NAME[];
/*
 * States for cell selection
 */
#define CS_NULL                  0    /* after system start or before 'cs_start_scan'                      */
#define CS_NULL_ACTIVE           1    /* after MPH_POWER_REQ sent in CS_NULL                               */
#define CS_CCO                   2
#define CS_XMEAS                 3
#define CS_IDLE                  4    /* all ARFCNs provided by MPH_POWER_CNF are used by MPH_BSIC_REQ/CNF */
#define CS_IDLE_ACTIVE           5    /* after MPH_POWER_REQ sent in CS_IDLE or MPH_BSIC_REQ sent          */

/* N950 Memory Optimization - Implements Measure #39*/
EXTERN  T_S2I_STRING  const
        STATE_CELL_SEL_NAME[];
/*
 * RR manages 5 (MAX_STATE) states: ATT, DAT, SAPI_3, GPRS, CS
 */
#ifdef OPTION_MULTITHREAD
  #define PROCESS_NAME    _ENTITY_PREFIXED(PROCESS_NAME)
#endif

#define STATE_ATT                 0
#define STATE_DAT                 1
#define STATE_SAPI_3              2
#define STATE_CELL_SEL            3
#ifdef GPRS
#define STATE_GPRS                4
#define MAX_STATE                 5
#else
#define MAX_STATE                 4
#endif

/* N950 Memory Optimization - Implements Measure #39*/
EXTERN  T_S2I_STRING const
        PROCESS_NAME[];


#define MAX_STK_LENGTH      80
#define NOT_INITIALISED     NOT_PRESENT_8BIT /*Used for initialising element 'c_ncell_bcch'*/

/*
 * A definition is provided in RR_PEI.C
 */
#ifdef RR_PEI_C
/* N950 Memory Optimization - Implements Measure #39*/
GLOBAL T_S2I_STRING const  STATE_ATT_NAME[] =
{
  S2I_STRING("ATT_NULL")     ,      
  S2I_STRING("ATT_CS1")      ,     
  S2I_STRING("ATT_CS2")      ,       
  S2I_STRING("ATT_CS3")      ,      
  S2I_STRING("ATT_DEDICATED"),      
  S2I_STRING("ATT_IDLE")     ,     
  S2I_STRING("ATT_NO_SERVICE"),     
  S2I_STRING("ATT_CON_EST") 
};
GLOBAL T_S2I_STRING const       STATE_DAT_NAME[] =
{
  S2I_STRING("DAT_NULL")         ,
  S2I_STRING("DAT_CELL_RESELECT"),
  S2I_STRING("DAT_CHAN_ASS")     ,
  S2I_STRING("DAT_CHAN_ASS_1")   ,
  S2I_STRING("DAT_CHAN_ASS_2")   ,
  S2I_STRING("DAT_PDCH_ASS")     ,
  S2I_STRING("DAT_PDCH_ASS_1")   ,
  S2I_STRING("DAT_PDCH_ASS_2")   ,
  S2I_STRING("DAT_PDCH_ASS_3")   ,
  S2I_STRING("DAT_PDCH_ASS_4")   ,
  S2I_STRING("DAT_CCO_3")        ,
  S2I_STRING("DAT_CCO_4")        ,
  S2I_STRING("DAT_CHAN_REL")     ,
  S2I_STRING("DAT_HANDOVER")     ,
  S2I_STRING("DAT_HANDOVER_4")   ,
  S2I_STRING("DAT_HANDOVER_5")   ,
  S2I_STRING("DAT_IDLE")         ,
  S2I_STRING("DAT_IMM_ASS")      ,
  S2I_STRING("DAT_IMM_ASS_1")
};

GLOBAL T_S2I_STRING const    STATE_SAPI_3_NAME[] = 
{
  S2I_STRING("SMS_IDLE")       ,
  S2I_STRING("SMS_PENDING")    ,
  S2I_STRING("SMS_ESTABLISHED")
};

GLOBAL T_S2I_STRING const  STATE_CELL_SEL_NAME[] = 
{ 

  S2I_STRING("CS_NULL")             ,
  S2I_STRING("CS_NULL_ACTIVE")      ,
  S2I_STRING("CS_CCO")              ,
  S2I_STRING("CS_XMEAS")            ,
  S2I_STRING("CS_IDLE")             ,
  S2I_STRING("CS_IDLE_ACTIVE")
};
#ifdef GPRS
GLOBAL T_S2I_STRING const        PROCESS_NAME[] = 
{
  S2I_STRING("ATT")   ,
  S2I_STRING("DAT")   ,
  S2I_STRING("SAPI_3"),
  S2I_STRING("CELL_SEL"),
  S2I_STRING("GPRS") 
};
#else
GLOBAL T_S2I_STRING const        PROCESS_NAME[] = 
{
  S2I_STRING("ATT")   ,
  S2I_STRING("DAT")   ,
  S2I_STRING("SAPI_3"),
  S2I_STRING("CELL_SEL") 
};
#endif

#endif

/*
 * Shifts for 'operation_mode'
 */
#define SHIFT_FOR_SIM_TYPE          7
#define SHIFT_FOR_SEARCH_OFFSET     6
#define SHIFT_FOR_SIM_INSERTED      5

/*
 * To check multiband restrictions
 */
#define UNKNOWN_BAND_USED 0
#define INVALID_BAND_USED 1
#define LOW_BAND_USED     2
#define HIGH_BAND_USED    4
#define EXT_BAND_USED     8

/*
 * Defines for Equivalent PLMNs
 */
#define UBYTES_PER_PLMN    3
#define RR_EPLMNLIST_SIZE   (SIZE_EPLMN/UBYTES_PER_PLMN)

/*
 * Maximum lenght of the string that can be passed to sprintf
 */
#define MAX_SPRINTF_STRING_LEN 80

/*==== TYPES ======================================================*/
typedef struct
{
  UBYTE length;
  UBYTE count;
} T_W_PARAM;

typedef struct
{
  UBYTE                 cs;
  UBYTE                 val;
} T_ERROR;


/* range check with the fastest way for ARM */
/* corresponds with ((min <= x) AND (x <= max)) */
#define INRANGE(min, x, max)  ((unsigned) ( (x)-(min) ) <= (unsigned)( (max)-(min) ) )

/*
 * This data structure stores a list of frequencies.
 * The frequencies are stored in bitmap format.
 * In GSM the frequencies have a range of 0 - 1023
 * where some of the frequencies are not yet used.
 * For the calculation of the needs of bytes and the bit representations of
 * the channels within the T_LIST please have a look at rr_srv.c
 * (comment with 'List processing Functions')
 */

#define            L3_SDU_BUF_SIZE     30
typedef struct
{
  /*lint -esym(768,length) : not referenced */
  USHORT  length;
  USHORT  offset;
  UBYTE   buffer  [L3_SDU_BUF_SIZE];
} T_L3_SDU;

typedef enum
{
  SAI_NULL,     /* no APDU transfer is ongoing */
  SAI_SEGM      /* segmentation or re-segmentation is ongoing */
} T_APPLIC_STATE;

#if defined FF_EOTD
typedef struct
{
  T_APPLIC_STATE        state;              /* state of application procedure */
  T_RRRRLP_DATA_IND    *rrrrlp_data_ind;    /* APDU Data during re-segmentation */
} T_APPLIC_RX;
#endif /* FF_EOTD */

typedef struct
{
  UBYTE           param_ind;
  UBYTE           cbq;
  UBYTE           cell_reselect_offset;
  UBYTE           temp_offset;
  UBYTE           penalty_time;
  UBYTE           power_off_ind;
  UBYTE           power_off;
  UBYTE           two_ter;
  UBYTE           ecsc;
} T_C2_PARAMETER;

typedef struct
{
  UBYTE           ia_p;
  UBYTE           ia_maio;
  UBYTE           c_ia_mac;
  UBYTE           ia_mac[9];
} T_IA_REST;

#if defined(_SIMULATION_FFS_)
typedef struct
{
   UBYTE           mcc[MAX_MCC_SHIELD][SIZE_MCC];
   UBYTE           enabled;
   UBYTE           index;
}T_shield_mcc;
#endif

/* CSI-LLD section:3.2.1
 * Black list contains the carriers that cannot be synchronized
 * Carriers belonging to either european or american bands can
 * be part of black list
 */
typedef struct
{
  T_LIST list[MAX_REGIONS];                    /* Seperate lists for European & American regions              */
  U8     sfc[MAX_REGIONS][MAX_SFC_PER_REGION]; /* Seperate sync fail counters for European & American regions */
}T_CS_BLACK_LIST;

/* CSI-LLD 3.2.2
 * White List Data of RR.White List contains carriers that are good candidates
 * for full service.
 */
typedef struct
{
  T_LIST           list;
  T_loc_area_ident last_sc_lac;       /* stores the LAC of the last serving cell */
  U16              last_sc_arfcn;     /* stores the last serving cell ARFCN      */
  U8               region;            /* stores the region information           */
} T_CS_WHITE_LIST;

typedef struct
{
  U8               si2[BA_BITMAP_SIZE];    /* stores the BA list received in SI2      */
  U8               si2bis[BA_BITMAP_SIZE]; /* stores the BA list received in SI2BIS   */
  U8               si2ter[BA_BITMAP_SIZE]; /* stores the BA list received in SI2TER   */
} T_CR_WHITE_LIST;

/*
 * ms data of RR
 */
typedef struct
{
  UBYTE                 rr_service;        /* actual rr service mode      */
  UBYTE                 req_mm_service;    /* by mm requested service mode*/
  BOOL                  multislot_class_configured;
  BOOL                  cmsp_configured;
  T_rf_cap              rf_cap;            /* MS RF Capability            */
  T_mob_class_1         classmark1;        /* mobile station classmark 1  */
  T_mob_class_2         classmark2;        /* mobile station classmark 2  */
  T_mob_class_3         classmark3;        /* mobile station classmark 3  */
#ifdef GPRS
  T_ra_cap              ra_cap;            /* MS Radio Access Capability  */
#endif
  T_mob_ident           imei;              /* IMEI                        */
  UBYTE                 operation_mode;    /* Operation Parameters from MM*/

  /*
   * The purpose of 'parallel_net_plmn_search_type' and
   * 'current_plmn_search_type' is to record the request for a parallel search
   * (i.e. to action a network search whilst an existing search (for a
   * limited/plmn) is already underway)
   *
   * The original request is stored in 'current_plmn_search_type' and the
   * parallel net search request is stored in 'parallel_net_plmn_search_type'
   *
   * When the NET search is completed, 'parallel_net_plmn_search_type' is
   * cleared and 'mm_req_service' is set the stored search value in
   * 'current_plmn_search_type'
   */
  UBYTE                 parallel_net_plmn_search_type;
  UBYTE                 current_plmn_search_type;


  T_plmn                plmn;              /* requested network by MM     */
  BOOL                  v_eq_plmn;         /* EPLMN List is valid or not  */
  T_plmn                eq_plmn_list[RR_EPLMNLIST_SIZE];      /* eq PLMN list from MM        */
  UBYTE                 cksn;              /* cipher key sequence number  */
  UBYTE                 kc [KC_STRING_SIZE]; /* authentication parameter Kc */
  UBYTE                 new_kc [KC_STRING_SIZE]; /* new auth parameter from SIM */
  USHORT                access_classes;    /* access classes from SIM     */
  T_mob_ident           imsi;              /* IMSI from SIM card          */
  UBYTE                 imsi_available;    /* IMSI available flag         */
  ULONG                 tmsi_binary;       /* binary representation TMSI  */
  UBYTE                 tmsi_available;    /* TMSI available flag         */
  T_loc_area_ident      forb_lac_list[MAX_LAI];
                                           /* forbidden lai list          */
  T_loc_area_ident      roam_forb_lac_list[MAX_LAI];
                                           /* roaming forbidden lai list  */
  UBYTE                 c2_tab [NCELL_SIZE]; /* ranked list for C2          */
  UBYTE                 access_counter;    /* counts random bursts        */
  UBYTE                 all_conf_received; /* flag all randoms confirmed  */
  UBYTE                 channel_needed;    /* for mobile terminated call  */
  T_ERROR               error;             /* in incoming messages        */
  USHORT                establish_cause;   /* for outgoing call           */
  ULONG                 fn_offset;         /* time between measurements   */
  T_synch_ind           ho_type;           /* handover type information   */
  USHORT                index;             /* for random number generation*/
  T_L3_SDU              l3msg;             /* layer 3 message for establ. */
  UBYTE                 last_used_channel; /* for building random ref     */
  UBYTE                 max_attempt;       /* number of random bursts     */
  T_MPH_MEASUREMENT_IND measurement_report;/* last measurement report     */
  T_TIME                reg_time_gap;      /* Time gap between Non parallel search in Lim Ser */
  UBYTE                 reg_counter;       /* registration counter        */
  UBYTE                 old_cell_tried;    /* flag during cell reselection*/
  T_plmn                ahplmn;            /* Acting HPLMN ID */
#if defined (REL99) AND defined (TI_PS_FF_EMR) AND (GPRS)
  BOOL                  enable_ps_emr;
#endif
} T_MS_DATA;

#if defined (REL99) && defined (TI_PS_FF_EMR)
/* indicates from where the enhanced para are taken - SI2quater or MI*/
#define  ENH_PARA_IDLE                0
#define  ENH_PARA_DEDICATED           1
#define  ENH_PARA_INVALID_STATE       2
#endif

#if defined (TI_PS_FF_RTD) AND defined (REL99)
/* start of constants related to RTD */
#define  RTD_DEFAULT_INDEX             0
#define  RTD_NOT_AVAILABLE             0xFFFF
#define  RTD_12BIT                     0x8000
#define  MAX_NR_OF_NCELL               0x20
/* end of constants related to RTD */
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */


/*
 * data area for cell selection / reselection
 */
typedef struct
{
  UBYTE             v_cell_chan_desc;    /* cell chan desc flag         */
  T_LIST            cell_chan_desc;      /* cell channel description    */
  T_chan_desc       cbch_chan_desc;      /* cbch channel description    */
  UBYTE             cbch_chan_desc_avail;/* cbch available flag         */
  UBYTE             cbch_mob_alloc [65]; /* cbch mobile allocation      */
  UBYTE             ncc_permitted;       /* NCC permitted bitmap        */
  T_cell_opt_sacch  cell_options;        /* cell options                */
  UBYTE             ncell_ext;           /* neighbourcell extension flag*/
  UBYTE             dtx;                 /* dtx flag                    */
  UBYTE             dtx_half;
  UBYTE             dtx_full;
  USHORT            sys_info_read;       /* bitmap of read sys infos    */
  T_LIST            ncell_list;          /* neighbour cell list         */
#if defined (REL99) && defined (TI_PS_FF_EMR)
  UBYTE             si2quater_status;    /*Indicates the SI-2quater status on cell that can be used
                                           once the cell is selected */
  BOOL              si2quater_pos;       /* 0 - NBCCH, 1 - EBCCH*/
#endif
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  UBYTE             band_indicator;      /* 0 - 1800,  1 - 1900 */
#endif
} T_CELL_DATA;

typedef struct
{
  T_plmn            plmn;               /* Idetification of Network     */
  USHORT            lac;                /* Location Area Code           */
  USHORT            arfcn;              /* Frequency Number             */
  UBYTE             rxlev;              /* The received signal level    */
  UBYTE             cell_ok;            /* cell allows network access   */
  UBYTE             region;             /* Identification of region     */
} T_FOUND_ELEMENT;

typedef struct
{
  T_FOUND_ELEMENT   element [MAX_PLMN];
} T_FOUND_LIST;

#if defined (REL99) && defined (TI_PS_FF_EMR)
typedef struct {
  BOOL              is_data_valid; /*Indicates whether the maintained Enhanced parameters 
                                      is valid or not.*/
  UBYTE             mp_change_mark; /*Current change mark value of the measurement parameters
                                      received in Air message*/
  UBYTE             msg_count; /*Number of instances to be received*/   
  UBYTE             rep_type; /* 'Normal' or 'Enhanced': Report type to be used in dedicated mode*/
  UBYTE             grr_rep_type; /* 'Normal' or 'Enhanced': Report type to be used in PIM/PTM*/
  T_enh_para_struct enh_para;/* Actual parameters */
#ifdef GPRS
  T_nc_para_struct  nc_para; /*Network control parameters*/
#endif
  ULONG             ba2bsic_map_pending;/*1 in a bit pos indicates BA-BSIC mapping is pending for that index
                                          may be due to incomplete BA (SI-5ter awaited)*/
  UBYTE             scale_used_last;
} T_rr_enh_para;

/*In a multi instance message where this BSIC info is received,
  not all instances carry BSIC list. The instance that carries
  this info will have is_valid set to TRUE*/
typedef struct  {
  BOOL            is_valid;
  T_gprs_bsic     bsic_info;  
} T_bsic_list;
#endif

/*
 * serving cell data of RR
 */
typedef struct
{
  UBYTE             ciph_on;             /* ciphering status            */
  UBYTE             algo;                /* cipher algorithm            */
  UBYTE             ciph_received;       /* flag ciphering received     */
  UBYTE             selection_type;      /* type of cell selection      */
  BOOL              mm_started;          /* originated of cell selection*/
  UBYTE             found_entries;       /* number of found PLMNs       */
  T_FOUND_ELEMENT   found[MAX_PLMN];     /* list of found PLMNs         */
  UBYTE             first_attempt;       /* high priority attempt       */

  T_chan_desc       chan_desc;           /* actual channel description  */
  UBYTE             new_ta;              /* timing advance              */
  UBYTE             ch_mode;             /* actual channel mode         */
  UBYTE             ba_index;            /* band allocation index       */
  ULONG             observed_ta;         /* observed timing advance     */
  T_LIST            five_ter_list;       /* contains sys info 5ter list */
  T_LIST            ncell_list_idle;     /* neighbour cell list build in idle mode */
  /*
   * data area for cell selection / reselection
   */
  T_CELL_DATA       cd;                  /* cell data                   */
  T_multirate_conf  amr_conf;            /* MultiRate configuration     */
#if defined(STORE_FN)
  ULONG             fn;                  /* last indicated framenumber  */
#endif  /* STORE_FN */
#if defined (REL99) && defined (TI_PS_FF_EMR)
  /* 
   * Data area for EMR
   */
  /* 
   *  Parameters that are permanent
   */
  UBYTE             enh_para_status; /* Indicates Whether the existing enh_para are from SI-2quater or MI*/
  T_rr_enh_para     emr_data_current;    /*Parameters for EMR - for RR in dedicated
                                          mode and for GRR in PIM/PTM mode. Not required in Idle*/
  BOOL              ba_list_ded; /*Indicates whether BA list is already received in dedicated mode*/  
  BOOL              ba_list_idle;/*Indicates whether BA list is already received in Idle mode*/    
  /* 
   *  Parameters that are required only during message processing
   */
  T_rr_enh_para     emr_data_temp;    /* Temporary storage till we get all the instances of Air message*/
  UBYTE             prev_highest_index; /*To keep track of highest index*/
  ULONG             rep_bmp;          /*temporary storage of report priority in bit map form*/
  USHORT            instance_bmp; /*A bit set indicates that the instance is received. Bit 0- instance '0'
                                    bit 15 - instance '15'*/
  T_bsic_list       bsic_list[MAX_MULTI_INST];  /* To store the received BSIC lists before performing
                                                  BA-BSIC mapping*/
  UBYTE       rep_count[MAX_NEIGHBOURCELLS]; /* this is used to get how many times a cell in 
  enhanced neighbour cell list is reported. Will be of use when reporting low priority cells
  at reduced rate*/  
  UBYTE       emr_count; /*Keeps track (MOD4) of number of EMR messages sent*/    
  UBYTE              ba_index_idle; /*To store BA_INDEX used in Idle before going to Dedicated*/
  UBYTE              new_ba_index;
#endif
} T_SC_DATA;

/*
 * cell reselection data of RR (during handover or hplmn search)
 */
typedef struct
{
  UBYTE             ciph_on;             /* ciphering status            */
  UBYTE             algo;                /* cipher algorithm            */
  UBYTE             ch_mode;             /* actual channel mode         */
  T_chan_desc       chan_desc;           /* actual channel description  */
  T_CR_WHITE_LIST   cr_white_list;       /* stores si2x temporarily during BCCH reading */
  T_CELL_DATA       cd;                  /* cell data                   */
  T_multirate_conf  amr_conf;            /* MultiRate configuration     */
#if defined (REL99) && defined (TI_PS_FF_EMR)
  UBYTE             ba_index;            /* BA-IND received in BA(list) of SI-2*/  
#endif

} T_CR_DATA;
#ifdef GPRS
typedef enum
{
  NORMAL_BCCH,
  EXTENDED_BCCH,
  SI13_NOT_PRESENT
} T_SI13_LOC_IND;
#endif
/*
 * Neighbour cell data of RR
 */

/* DRAM Optimization: This structure is re-allign with 4 bytes 
 * sothat will have minimum wastage */
typedef struct
{
  T_ctrl_chan_desc  control_descr;       /* Control Channel Description */
  T_cell_select     select_para;         /* Cell Selection Parameter    */
  T_rach_ctrl       rach;                /* random control parameter    */
  T_loc_area_ident  lai;                 /* location area identification*/

  ULONG             avail_time;          /* time ncell is available     */
  ULONG             c1_counter;          /* measurement report counter  */
  ULONG             bcch_counter;

  USHORT            arfcn;               /* channel number         */
  SHORT             c1;                  /* C1 Path Loss Criterion      */
  SHORT             c2;                  /* C2 Reselection Criterion    */
  USHORT            cell_id;             /* cell identifier             */

  T_C2_PARAMETER    c2_par;              /* C2 Parameter for Calculation*/  
  UBYTE             rxlev;               /* fieldstrength               */
  UBYTE             v_acs;               /* read of sys info 7 & 8      */
  UBYTE             c2_used;             /* calculated c2 values        */

  UBYTE             bcch_error;          /* couter BCCH errors          */
  UBYTE             bcch_status;         /* status of neighbour cell    */
  UBYTE             bsic;                /* BSIC                        */
  UBYTE             tnnn;                /* 5 sec counter reselection   */

  SHORT             c2_corr;
#ifdef GPRS
  SHORT             c31;
  SHORT             c32;
  UBYTE              c32_used;
  UBYTE              v_cr_par;
  T_cr_par           cr_par;
#endif

#ifdef GPRS
  T_SI13_LOC_IND si13_loc_ind;
#endif
  
#ifdef GPRS
  UBYTE             rac;                 /* routing area color          */
#endif

#ifdef REL99
  UBYTE             mscr_flag;
#endif   

} T_NC_DATA;

/*
 * Cell Selection Process Data
 */
typedef struct
{
  UBYTE      max_arfcn;                    /* number of valid channels    */
  UBYTE      act_index;                    /* actual index                */
  BOOL       check_hplmn;                  /* check HPLMN */
  USHORT     arfcn     [MAX_CHANNELS];               /* channel number        */
  UBYTE      rxlev     [MAX_CHANNELS];               /* fieldstrength         */
  UBYTE      attributes[MAX_CHANNELS];               /* attributes of channel */
             /* Bit 7 Bcch Info List Flag
                      0 : not member of the BCCH info List
                      1 : member of the BCCH info List
                Bit 6 Low Priority Flag
                      0 : high priority flag
                      1 : low  priority flag
                Bit 5 Emergency Cell Flag
                      0 : emergency services possible
                      1 : emergency services not possible
                Bit 4 BCCH Detected Flag
                      0 : BCCH not decoded
                      1 : BCCH decoded
                Bit 3 BCCH Checked Flag
                      0 : BCCH not checked
                      1 : BCCH checked
                Bit 2 reserved
                Bit 1/0   Priority
                      0 : low priority
                      1 : middle priority
                      2 : high priority
             */
  USHORT     arfcn_sc;
  USHORT     arfcn_cr;
  T_CS_BLACK_LIST black_list;         /* Stores the black_list information */
  T_CS_WHITE_LIST white_list;         /* Stores the white list information */
  U8         previous_search_mode;    /* stores the previous search mode   */
  U8         current_search_mode;     /* stores the current search mode    */
  U8         scan_mode;               /* Identifies the scan mode          */
  U8         initial_plmn_search;     /* Identifies first FUNC_PLMN search */
  U8         black_list_search_pending;
  U8         region;                  /* This stores the region            */
  BOOL       all_freq_area;           /* Identifies presence of both Euro and American coverage */
  T_plmn     last_reg_plmn;           /* Stores the last registered PLMN */
} T_CS_DATA;

/* CSI-LLD Section 3.8
 * Dynamic configuration commands data
 */
typedef struct
{
  U32    tfast_cs_val;     /* TFAST_CS timer value in minutes   */
  U32    tnormal_cs_val;   /* TNORMAL_CS timer value in minutes */
  U8     upper_rxlev_thr[MAX_NUM_BANDS];  /* Upper RxLev threshold             */
  U8     medium_rxlev_thr[MAX_NUM_BANDS]; /* Medium RxLev threshold            */
  U8     lower_rxlev_thr[MAX_NUM_BANDS];  /* Lower RxLev threshold             */
  U8     bl_cs_en;         /* Controls Black List search        */
  U8     fcr;              /* dynamic configuration, Fast Cell Recelection */
  U8     scr;              /* dynamic configuration, Slow Cell Recelection */
  U8     fca;              /* dynamic configuration, Failed Channel Assignment */
  U8     fho;              /* dynamic configuration, Force Handover */
  U8     iho;              /* dynamic configuration, lock the DUT to the cell it is already camping */
  U8     set_band;         /* dynamic configuration, Frequency Band control */
  U8     no_sys_time;      /* dynamic configuration, control time dependent random generation */
  S16    gsm_offset;       /* dynamic configuration, offset to path loss criterium C1 */
  S16    dcs_offset;       /* dynamic configuration, offset to path loss criterium C1 */
  U8     nkc;              /* dynamic configuration, user specific, ciphering when no ciphering key available */
  T_TIME lim_ser_nps_delay;/* dynamic configuration, delay between Non parallel search in Lim ser in all freq area */
#if defined(_SIMULATION_FFS_)
  T_shield_mcc mcc_shield;    /* This structure contains the shielded MCC values*/
#endif
} T_DYNAMIC_CONFIG;

typedef struct
{
  UBYTE                 state[MAX_STATE];
  T_MS_DATA             ms_data;
  T_SC_DATA             sc_data;
  T_CR_DATA             cr_data;
  T_NC_DATA             nc_data [NCELL_SIZE];  /* 0-5 = ncell, 6 = serving cell
                                                * 7   = cell reselection
                                                */
  T_CS_DATA             cs_data;
#ifdef GPRS
  T_GPRS_DATA           gprs_data;
  T_LIST                ba_ncell_list;  /* Used to store Original BA list, when BA list is midified by PMO */
#endif
  UBYTE                 lup_rxlev;
  UBYTE                 c1_offset;
  /*
   * primitive storing variables
   */
  T_PRIM                *stored_prim [MAX_RR_STORED_PRIM];
  UBYTE                stored_prim_in;
  UBYTE                stored_prim_out;
  UBYTE                stored_prim_read;
  UBYTE                stored_prim_write;
  /*
   * must be checked ...
   */
  UBYTE                 old_serving_cell;
  USHORT                old_cell_id;
  T_loc_area_ident      old_lai;
  UBYTE                 page_identity_type;
  USHORT                power_on_low;
  USHORT                power_on_high;
  USHORT                rel_cause;
  USHORT                dcch_stop_cause;
  UBYTE                 reselect_index;
  UBYTE                 repeat_est;
  UBYTE                 t_running[NUM_OF_RR_TIMERS];
  void                  (*t_expire[NUM_OF_RR_TIMERS])(void);
  UBYTE                 treg_pending;
  UBYTE                 tch_loop_subch;
  UBYTE                 used_channel_ref[MAX_RACH_REQ];
  T_frame_no            used_frame_no[MAX_RACH_REQ];
  UBYTE                 bcch_error;

  UBYTE                 pag_rec;
  UBYTE                 imm_ass_rej_rec;
  EF_MSCAP              mscap;
  USHORT                act_ncell_list [MAX_NEIGHBOURCELLS];
  UBYTE                 ncell_mb;
  UBYTE                 start_cell_reselection;
  UBYTE                 first_meas_received;
  UBYTE                 mode_after_dedi;        /* MODE_CELL_RESELECTION or MODE_CELL_SELECTION or MODE_CELL_CHANGE_ORDER*/
  BOOL                  net_lost;               /* a RLF Radio Link Failure or DSF Downlink Signalling Failure has occured */
  USHORT                found_channel;          /* ARFCN of requested PLMN (obtained from 'previous' Cell Selection data) */
  UBYTE                 ba_id;               /* BA identifier used with Neighbour Cell Measurement       */

#if defined FF_EOTD
  USHORT                eotd_req_id;         /* indicate explicit position measurement requests          */
  T_APPLIC_RX           applic_rx;           /* Application Information transfer                         */
#endif /* FF_EOTD */

  UBYTE                 emo_seq;             /* EMO sequence number                                      */
  UBYTE                 c_emo_arfcn;         /* number of elements in emo_arfcn                          */
  USHORT                *emo_arfcn;          /* request list of EMO ARFCNs                               */

  UBYTE                 resel_pending;       /* cell reselection was deferred */
  UBYTE                 c_ncell_bcch;        /* # of outstanding ncell BCCH info. Default=NOT_INITIALISED     */
  UBYTE                 cell_test_operation; /* dynamic configuration, as defined in TS 11.11, section 10.3.18 EFAD (Administrative data) */
  T_DYNAMIC_CONFIG      dyn_config;          /* Holds dynamic configuration data */
#ifdef GPRS
  U8                    old_rac;             /* Holds old Routing area code */
#endif
  BOOL                  cr_treselect_exp;    /* Flag to indicate whether cell reselection after TRESELECT
                                              * expiry waiting for SI2TER is suceesfull or not */
  BOOL                  use_stored_entries;  /* Use stored entries after end of transition */
} T_RR_DATA;

typedef struct
{
  USHORT l_f;
  USHORT o_f;
  UBYTE  b_f[32];
} T_f_range;


/*
 * Used for requesting location information from RR for proactive polling in sim
 */
typedef struct
{
  /*lint -esym(768,stk_class) : not referenced */
  /*lint -esym(768,stk_subclass) */
  /*lint -esym(768,stk_type) */
  UBYTE stk_class;
  UBYTE stk_subclass;
  UBYTE stk_type;
  UBYTE stk_length;
  UBYTE stk_parameter [MAX_STK_LENGTH];
} stk_data_type;

/*==== EXPORT =====================================================*/

/*
 * Prototypes Timer
 */
/*
 * If all entities are linked into one module this definitions
 * prefixes all this functions with the enity name
 */

#ifdef OPTION_MULTITHREAD
  #define tim_exec_timeout   _ENTITY_PREFIXED(tim_exec_timeout)
  #define tim_start_timer    _ENTITY_PREFIXED(tim_start_timer)
  #define tim_stop_timer     _ENTITY_PREFIXED(tim_stop_timer)
#if defined(TIMER_TRACE)
  #define tim_check_timer    _ENTITY_PREFIXED(tim_check_timer)
#endif  /* TIMER_TRACE */
#endif  /*  OPTION_MULTITHREAD*/
EXTERN void tim_exec_timeout (USHORT index);
EXTERN void tim_start_timer  (USHORT index, T_TIME value);
EXTERN void tim_stop_timer   (USHORT index);
#if defined(TIMER_TRACE)
EXTERN BOOL tim_check_timer(USHORT index);
#endif  /* TIMER_TRACE */

EXTERN void tim_t3110                   (void);
EXTERN void tim_t3122                   (void);
EXTERN void tim_t3126                   (void);
EXTERN void tim_treselect               (void);
EXTERN void tim_treg                    (void);
EXTERN void tim_ext_meas                (void);
#if defined FF_EOTD
EXTERN void tim_apdu                    (void);
#endif /* FF_EOTD */
EXTERN void tim_tnnn                    (void);
EXTERN void set_tnnn                    (int                 index,
                                         USHORT              value);
EXTERN void reset_tnnn                  (int                 index);
EXTERN int is_tnnn                      (int                 index);
EXTERN void tim_tabort                  (void);
EXTERN void tstart_tabort               (USHORT              val);
EXTERN void tim_reset_registration_timer(void);
EXTERN void tim_plmn_search_expiry      (void);
EXTERN void tim_tnormal_cs              (void);
EXTERN void tim_tfast_cs                (void);

/*
 * Prototypes Customer Specific Functions
 */

/*
 *  Cell Selection
 */
EXTERN void       cs_init_process     (void);
EXTERN void       cs_set_all          (void);
EXTERN void       cs_start_scan       (void);
EXTERN void       cs_start_sync       (void);
EXTERN void       cs_del_list         (T_LIST      * list);
EXTERN void       cs_set_list         (T_LIST      * list);
EXTERN UBYTE      cs_def_list         (UBYTE         attribute);
EXTERN BOOL       cs_sync_next        (void);
EXTERN void       cs_set_null         (void);
EXTERN void       cs_set_attributes   (UBYTE             attribute,
                                       USHORT            arfcn);
EXTERN void       cs_set_rxlev        (UBYTE rxlev, USHORT arfcn);
EXTERN void       cs_clear_attributes (UBYTE attribute, USHORT arfcn);
EXTERN void       cs_mph_power_cnf    (T_MPH_POWER_CNF * mph_power_cnf);
EXTERN void       cs_mph_bsic_cnf     (T_MPH_BSIC_CNF  * mph_bsic_cnf);
EXTERN void       cs_set_stop_active  (void);
EXTERN void       cs_set_bcch_info    (T_bcch_info * sim_bcch_info);
EXTERN void       cs_get_channel_from_found_list (void);
EXTERN void       cs_clear_channel_from_found_list (void);
EXTERN UBYTE      cs_get_freq_band (UBYTE pch_interrupt);
EXTERN void       cs_clear_white_list(U8 clr_bcch_info);

/* CSI-LLD section:4.1.1.5  */
GLOBAL void  cs_store_black_list(void);
GLOBAL void  cs_add_to_black_list(U8 region,U16 arfcn,U8 rxlev);
GLOBAL void  cs_del_from_black_list(U8 region,U16 arfcn);
GLOBAL void  cs_clear_black_list(U8 erase_type);
GLOBAL void  cs_remove_BA_MA_from_black_list(U8 region,T_LIST *source_list);
GLOBAL void  cs_update_black_list(void);
EXTERN U8    cs_get_new_search_mode(void);
EXTERN void  cs_handle_search_mode_timer(U8 search_mode);
GLOBAL void  cs_rem_inactive_carriers_from_bl(T_MPH_POWER_CNF * mph_power_cnf);
GLOBAL BOOL  cs_check_region(U8 region);
GLOBAL BOOL  cs_check_arfcn_range(U16 arfcn);
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
GLOBAL UBYTE cs_get_band_index(USHORT arfcn);
#endif

/*
 *  RR Attachment
 */

/*
 *  attachment primitives
 */
EXTERN void att_mph_error_ind           (T_MPH_ERROR_IND       * mph_error_ind);
EXTERN void att_mph_measurement_ind     (T_MPH_MEASUREMENT_IND * mph_measurement_ind);
EXTERN void dat_mph_emo_meas_ind        (T_MPH_EMO_MEAS_IND    * mph_emo_meas_ind);
EXTERN void att_rr_activate_req         (T_RR_ACTIVATE_REQ     * rr_activate_req);
EXTERN void att_rr_deactivate_req       (T_RR_DEACTIVATE_REQ   * rr_deactivate_req);
EXTERN void att_rr_sync_req             (T_RR_SYNC_REQ         * rr_sync_req);
EXTERN void att_mph_sync_ind            (T_MPH_SYNC_IND        * mph_sync_ind);
EXTERN void att_mph_meas_order_cnf      (T_MPH_MEAS_ORDER_CNF  * mph_meas_order_cnf);
EXTERN void att_rr_sync_hplmn_req       (T_RR_SYNC_HPLMN_REQ   * rr_sync_hplmn_req);
EXTERN void att_mph_stop_dedicated_cnf  (T_MPH_STOP_DEDICATED_CNF * mph_stop_dedi_cnf);
#ifdef GPRS
EXTERN void att_mph_meas_rep_cnf        (T_MPH_MEAS_REP_CNF    * mph_meas_rep_cnf);
EXTERN void att_mph_ext_meas_cnf        (T_MPH_EXT_MEAS_CNF    * mph_ext_meas_cnf);
EXTERN void att_ext_meas_end            (BOOL                    destory_rrgrr_ext_meas_cnf);
EXTERN void att_ext_meas_next_bsic      (void);
#endif
#if defined FF_EOTD
EXTERN void att_mph_ncell_pos_ind       (T_MPH_NCELL_POS_IND   * mph_ncell_pos_ind);
#endif /* FF_EOTD */
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
EXTERN void att_mph_init_rr_ind         (T_MPH_INIT_RR_IND     * mph_init_rr_ind);
#endif


/*
 *  attachment signalling
 */
EXTERN void att_dat_dedicated           (void);
EXTERN void att_dat_con_est             (void);
EXTERN void att_for_sysinfo_type1       (USHORT               arfcn,
                                         T_D_SYS_INFO_1      *sys_info_1,
                                         T_LIST              *cell_chan_desc);

EXTERN void att_for_sysinfo_type2_2bis  (USHORT               arfcn,
                                         T_VOID_STRUCT       *sys_info_2_2bis,
                                         T_LIST              *new_2_2bis_list,
                                         UBYTE                ncell_ext,
                                         T_SI_TYPE            si_type);

EXTERN void att_for_sysinfo_type2ter    (USHORT               arfcn,
                                         T_D_SYS_INFO_2TER   *sys_info_2ter,
                                         T_LIST              *new_2ter_list);
EXTERN void att_for_sysinfo_type3       (USHORT               arfcn,
                                         T_D_SYS_INFO_3      *sys_info_3);
EXTERN void att_for_sysinfo_type4       (USHORT               arfcn,
                                         T_D_SYS_INFO_4      *sys_info_4);

EXTERN void att_for_sysinfo_type5_5bis  (USHORT               arfcn,
                                         T_LIST              *new_5_5bis_list,
                                         UBYTE                old_index,
                                         UBYTE                ncell_ext,
                                         T_SI_TYPE            si_type);

EXTERN void att_for_sysinfo_type5ter    (USHORT               arfcn,
                                         T_LIST              *new_5ter_list);
EXTERN void att_for_sysinfo_type6       (USHORT               arfcn,
                                         T_D_SYS_INFO_6      *sys_info_6);
EXTERN void att_for_sysinfo_type7_8     (USHORT               arfcn,
                                         T_D_SYS_INFO_8      *sys_info_8);
EXTERN void att_bsic_ind                (USHORT               arfcn,
                                         UBYTE                rxlev,
                                         UBYTE                bsic);
EXTERN void att_no_bsic_ind             (void);
/*
 *  attachment procedures
 */
EXTERN UBYTE att_get_index              (USHORT              arfcn);

enum forbidden_list_type_e
{
  FORBIDDEN_LIST_NORMAL,
  FORBIDDEN_LIST_ROAMING
};
EXTERN void att_add_to_forb_list        (int list_type,
                                         const T_plmn       *plmn,
                                         USHORT              lac);
EXTERN void att_analyze_measure_report  (T_MPH_MEASUREMENT_IND *report);
EXTERN void att_bcch_status_to_decoded  (UBYTE               index);
EXTERN void att_bits_to_byte            (UBYTE               *num,
                                         UBYTE               size,
                                         UBYTE               *bits);
#ifdef REL99
EXTERN void att_config_cbch             (void);
#else
EXTERN void att_build_cbch              (void);
#endif
EXTERN void att_build_classmark_req     (void);
EXTERN void att_build_idle_req          (UBYTE               index,
                                         UBYTE               mode);
EXTERN void att_calculate_c1            (UBYTE               index);

GLOBAL BOOL att_cell_barred_status_cr_no_cr (UBYTE index);

EXTERN void att_check_bcch_carrier      (void);
EXTERN void att_check_bcch_carrier_si   (void);
EXTERN void att_check_neighbourcell     (void);
EXTERN void att_check_neighbourcell_si_reestab (void);
EXTERN void att_check_fplmn_cell        (void);
EXTERN void att_check_reselect_decision (UBYTE               start_now);
EXTERN void att_check_barred_status_in_idle (void);
EXTERN UBYTE att_check_network          (T_loc_area_ident    *lai);
EXTERN void att_clear_registration_data (void);
EXTERN void att_clear_reg_without_imsi  (void);
EXTERN void att_code_mph_ncell_req      (UBYTE               index);
EXTERN void att_code_mph_ncell_req_dedicated (void);

EXTERN void att_code_rr_abort_ind_original (T_RR_DATA *rr_data, USHORT    cause);
EXTERN void att_code_net_lost           (void);
EXTERN void att_code_rr_act_cnf         (void);
EXTERN void att_code_rr_act_ind         (void);
EXTERN void att_continue_cell_reselect  (void);

EXTERN void att_copy_sys_info_1_par     (UBYTE               index,
                                         T_D_SYS_INFO_1      *sys_info_1,
                                         T_LIST              *cell_chan_desc);
EXTERN void att_copy_sys_info_2_par     (UBYTE               index,
                                         T_D_SYS_INFO_2      *sys_info_2,
                                         T_LIST              *new_2_list,
                                         UBYTE               ncell_ext,
                                         UBYTE               indicate_changes);
EXTERN void att_copy_sys_info_2bis_par  (UBYTE               index,
                                         T_D_SYS_INFO_2BIS   *sys_info_2bis,
                                         T_LIST              *new_2bis_list,
                                         UBYTE               ncell_ext,
                                         UBYTE               indicate_changes);
EXTERN void att_copy_sys_info_2ter_par  (UBYTE              index,
                                         T_D_SYS_INFO_2TER *sys_info_2ter,
                                         T_LIST            *new_2ter_list,
                                         UBYTE              indicate_changes);
EXTERN void att_copy_sys_info_3_par     (UBYTE               index,
                                         T_D_SYS_INFO_3      *sys_info_3,
                                         UBYTE               indicate_changes);
EXTERN void att_copy_sys_info_4_par     (UBYTE               index,
                                         T_D_SYS_INFO_4      *sys_info_4,
                                         UBYTE               indicate_changes);
EXTERN void att_copy_sys_info_7_8_par   (UBYTE               index,
                                         T_D_SYS_INFO_8      *sys_info_8);
EXTERN void att_copy_c2_parameter_si4   (UBYTE               index,
                                         T_si4_rest_oct     *rest_oct);

EXTERN void att_handle_rr_act_req       (UBYTE                initial_act,
                                         T_RR_ACTIVATE_REQ   *rr_activate_req);
EXTERN void att_increase_power_on_time  (USHORT              inc);
EXTERN void att_init_cell_selection     (UBYTE               selection,
                                         BOOL                initiator);
EXTERN void att_init_gsm_data           (void);
EXTERN void att_init_pl_status          (void);
EXTERN SHORT att_max                    (SHORT               a,
                                         SHORT               b);
EXTERN T_FOUND_ELEMENT *att_plmn_in_found_list (UBYTE        *mcc,
                                         UBYTE               *mnc);
EXTERN void att_save_found_plmn         (T_FOUND_ELEMENT     *element,
                                         UBYTE               mcc[],
                                         UBYTE               mnc[],
                                         USHORT              arfcn,
                                         UBYTE               rxlev,
                                         USHORT              lac,
                                         UBYTE               cell_ok);

EXTERN void att_remove_bad_ncell        (T_MPH_MEASUREMENT_IND *report);
EXTERN void att_reset_old_lai_rac();
EXTERN void att_copy_old_lai_rac (U8 index);
#if defined(_SIMULATION_FFS_)
GLOBAL UBYTE rr_csf_mcc_present         (UBYTE* mcc_value);
#endif
EXTERN void att_clean_buf               (USHORT si_to_clean);
EXTERN void att_remove_bad_rr_data_ncells(void);
EXTERN void att_rem_lai_from_forb_list  (const T_plmn        *plmn,
                                         USHORT              lac);
EXTERN void att_set_pl_in_idle_mode     (void);
EXTERN void att_set_sys_info_read       (USHORT              mess_bit,
                                         UBYTE               index);
EXTERN void att_start_cell_reselection  (UBYTE               mode);
EXTERN void att_start_cr_in_pl          (UBYTE               index);
EXTERN void att_start_cell_selection    (BOOL    originator, BOOL parallel, U8 search_mode);

EXTERN void att_remove_multiple_channels(void);
EXTERN void att_mph_identity_req        (void);
EXTERN void att_start_registration_timer(void);
EXTERN void att_set_func                (UBYTE               func);

EXTERN void att_copy_cr_data            (void);
EXTERN void att_init_cr_data            (void);

EXTERN void att_check_reestablishment   (void);
EXTERN UBYTE att_get_power              (void);
EXTERN void att_return_to_idle          (void);
EXTERN void att_notify_stop_plmn_search (UBYTE deactivate_pl);
EXTERN void att_leave_dat_imm_ass       (void);
EXTERN void att_leave_dedicated         (void);
EXTERN void att_stop_dedicated          (void);
EXTERN void att_copy_eplmn_list (T_eq_plmn_list *eq_plmn_list);
EXTERN void att_store_plmn_in_found_list (T_loc_area_ident *lai);
#if defined (REL99) && defined (TI_PS_FF_EMR)
EXTERN void attf_send_enh_para_to_alr   (UBYTE rep_type,T_enh_para_struct *p_src);
#ifdef GPRS
EXTERN void att_send_enh_para_to_grr(T_rr_enh_para *p_src);
#endif
#endif

#if !defined(NTRACE)
/* Implements Measure#32: Row 52, 53, 124 & 125 */
EXTERN void att_print_mcc_mnc           (USHORT               arfcn,
                                         UBYTE               *mcc,
                                         UBYTE               *mnc,
                                         T_S2I_STRING        titel); /* a valid string is expected */
#else
#define att_print_mcc_mnc(arfcn,mcc,mnc,titel)
#endif  /* !NTRACE */
EXTERN void att_check_dynamic_search_mode_config(void);
EXTERN void att_full_service_found(void);
EXTERN void att_set_rr_service_info(void);
EXTERN void att_set_tim_advance_info (void);
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
EXTERN void att_update_std_band_indicator (UBYTE band_indicator);
#endif


/*
 *  Data-Transfer
 */

/*
 *  data-transfer primitives
 */
EXTERN       void dat_init_rr_data            (void);
EXTERN       void dat_dl_establish_cnf        (T_DL_ESTABLISH_CNF  *dl_establish_cnf);
EXTERN       void dat_dl_establish_ind        (T_DL_ESTABLISH_IND  *dl_establish_ind);
EXTERN       void dat_mph_random_access_cnf   (T_MPH_RANDOM_ACCESS_CNF *mph_random_access_cnf);
EXTERN       void dat_dl_release_cnf          (T_DL_RELEASE_CNF    *dl_release_cnf);
EXTERN       void dat_dl_release_ind          (T_DL_RELEASE_IND    *dl_release_ind);
EXTERN       void dat_mph_dedicated_cnf       (T_MPH_DEDICATED_CNF *mph_dedicated_cnf);
EXTERN       void dat_mph_dedicated_fail_cnf  (T_MPH_DEDICATED_FAIL_CNF *mph_dedicated_fail_cnf);
EXTERN       void dat_rr_abort_req            (T_RR_ABORT_REQ      *rr_abort_req);
EXTERN       void dat_rr_data_req             (T_RR_DATA_REQ       *rr_data_req);
EXTERN       void dat_rr_establish_req        (T_RR_ESTABLISH_REQ  *rr_establish_req);
EXTERN       void dat_mph_paging_ind          (T_MPH_PAGING_IND    *mph_paging_ind);
EXTERN       void dat_begin_start_immediate_assign (UBYTE id_type, UBYTE chan_need);
#if defined FF_EOTD
EXTERN       void dat_rrlc_meas_req           (T_RRLC_MEAS_REQ     *rrlc_meas_req);
EXTERN       void dat_rrrrlp_data_req         (T_RRRRLP_DATA_REQ   *rrrrlp_data_req);
#endif /* FF_EOTD */
#if defined (REL99) && defined (TI_PS_FF_EMR)
EXTERN void dat_update_emr_rep_para           (T_emp *p_em, T_enh_para_struct *p_enh);
#endif

/*
 *  data-transfer signalling
 */
EXTERN void dat_att_cell_selected       (void);
EXTERN void dat_att_null                (void);
EXTERN void dat_for_assign_cmd          (T_DL_DATA_IND       *dl_data_ind,
                                         T_D_ASSIGN_CMD      *assign_cmd,
                                         T_LIST              *hop_list_after,
                                         T_LIST              *hop_list_before,
                                         T_LIST              *cell_chan_desc);
EXTERN void dat_for_chan_mod            (T_DL_DATA_IND       *dl_data_ind,
                                         T_D_CHAN_MOD        *chan_mod);
EXTERN void dat_for_chan_rel            (T_DL_DATA_IND       *dl_data_ind,
                                         T_D_CHAN_REL        *chan_rel);
#ifdef REL99
EXTERN void dat_for_class_enq           (T_DL_DATA_IND       *dl_data_ind,
                                         T_D_CLASS_ENQ       *class_enq);
#else
EXTERN void dat_for_class_enq           (T_DL_DATA_IND       *dl_data_ind);
#endif
EXTERN void dat_for_close_loop_cmd      (T_DL_DATA_IND       *dl_data_ind,
                                         UBYTE                subchannel);
EXTERN void dat_for_ciph_cmd            (T_DL_DATA_IND       *dl_data_ind,
                                         T_D_CIPH_CMD        *ciph_cmd);
EXTERN void dat_for_freq_redef          (T_DL_DATA_IND       *dl_data_ind,
                                         T_D_FREQ_REDEF      *freq_redef,
                                         T_LIST              *cell_chan_desc);
EXTERN void dat_for_handov_cmd          (T_DL_DATA_IND       *dl_data_ind,
                                         T_D_HANDOV_CMD      *handov_cmd,
                                         T_LIST              *cell_chan_desc,
                                         T_LIST              *hop_list_after,
                                         T_LIST              *hop_list_before);
EXTERN void dat_for_l3_data_ind         (T_DL_DATA_IND       *dl_data_ind);
EXTERN void dat_for_open_loop_cmd       (T_DL_DATA_IND       *dl_data_ind);
EXTERN void dat_for_test_interface      (T_DL_DATA_IND       *dl_data_ind,
                                         UBYTE                device);
EXTERN void dat_for_imm_assign          (T_MPH_UNITDATA_IND  *mph_unitdata_ind,
                                         T_D_IMM_ASSIGN      *imm_assign);
EXTERN void dat_for_imm_assign_ext      (T_MPH_UNITDATA_IND  *mph_unitdata_ind,
                                         T_D_IMM_ASSIGN_EXT  *imm_assign_ext);
EXTERN void dat_for_imm_assign_rej      (T_D_IMM_ASSIGN_REJ  *imm_assign_rej);

EXTERN void dat_for_ext_meas_order      (T_D_EXT_MEAS_ORDER  *ext_meas_order);
#if defined FF_EOTD
EXTERN void rr_applic_rx_init           (T_APPLIC_RX         *applic_rx);
EXTERN void dat_for_applic_info         (T_B_APPLIC_INFO     *b_applic_info);
#endif /* FF_EOTD */
GLOBAL void dat_class_chng_data_req (void);
GLOBAL BOOL dat_hplmn (const UBYTE *mcc, const UBYTE *mnc);
#if defined (REL99) && defined (TI_PS_FF_EMR)
EXTERN BOOL dat_for_meas_inf            (T_D_MEAS_INF        *p_mi);
#endif

/*
 *  data-transfer procedures
 */
EXTERN BOOL dat_access_allowed          (USHORT              estcs);
EXTERN UBYTE dat_calc_downlink_timeout  (UBYTE               index);
EXTERN UBYTE dat_calc_paging_group      (UBYTE               index);
EXTERN UBYTE dat_calc_tn                (UBYTE               index);
EXTERN BOOL dat_check_error_flag        (BOOL                send_rr_status);
EXTERN void dat_code_channel_mode_to_mm (void);
EXTERN void dat_code_reestablishment_fail (void);
EXTERN void dat_code_ciphering_to_mm    (UBYTE               ciph_on);
EXTERN void dat_code_measure_report     (T_MPH_MEASUREMENT_IND *report);
EXTERN void dat_code_ext_meas_report    (T_MPH_EMO_MEAS_IND  *mph_emo_meas_ind);
EXTERN void dat_emo_stop                (BOOL send_ncell_req);

EXTERN void dat_code_mph_chan_mode_req  (T_D_CHAN_MOD        *chan_mod);
EXTERN void dat_code_mph_ciphering_req  (UBYTE               ciph_on,
                                         UBYTE               algo,
                                         UBYTE               *kc);
EXTERN void dat_code_mph_imm_assign_req (T_start             *start,
                                         UBYTE               power,
                                         UBYTE               maio,
                                         T_LIST              *hop_list_after,
                                         T_LIST              *hop_list_bef);
EXTERN void dat_code_sys_info_change    (UBYTE               dtx,
                                         UBYTE               pwrc,
                                         UBYTE               rlt);
EXTERN void dat_code_mph_freq_redef_req (T_start             *start,
                                         T_LIST              *hop_list);
EXTERN void dat_code_mph_old_chan_req   (void);
EXTERN void dat_code_prr_channel        (UBYTE               *ch_type,
                                         UBYTE               *sapi,
                                         UBYTE               chan_type);
EXTERN void dat_code_prr_channel_sms    (T_DL_DATA_REQ       *dl_data_req,
                                         UBYTE               chan_type);
EXTERN void dat_code_prr_bcch_info      (UBYTE                v_ba_range,
                                         T_ba_range          *ba_range);
EXTERN BOOL dat_compare_request_ref     (T_req_ref           *req_ref,
                                         UBYTE               *index);
EXTERN void dat_disconnect_link         (USHORT              cause);
EXTERN BOOL dat_forb_lai_check          (UBYTE               index);
EXTERN BOOL dat_roam_forb_lai_check     (UBYTE               index);

EXTERN BOOL dat_plmn_equal_req          (const UBYTE         *bcch_mcc,
                                         const UBYTE         *bcch_mnc,
                                         const UBYTE         *sim_mcc,
                                         const UBYTE         *sim_mnc);

EXTERN USHORT dat_random                (USHORT              n);
EXTERN void dat_release_connection      (void);
EXTERN void dat_set_last_used_channel   (T_chan_desc         *chan_desc);
EXTERN void dat_start_immediate_assign  (USHORT              cause);
EXTERN void dat_start_sabm              (void);
EXTERN BOOL dat_test_sim_available      (void);
EXTERN BOOL dat_check_sim_available     (void);
EXTERN void dat_vsd_bit_set             (T_L3_SDU            *m_buf,
                                         UBYTE               action);
EXTERN void dat_send_bcchinfo_mm        (U8 *p);
EXTERN void dat_convert_white_list      (void);
EXTERN void dat_store_neigh_cell_desc   (U8 si, U8 index, BUF_neigh_cell_desc *cd,
                                          T_LIST *new_neigh_list);

EXTERN void dat_begin_start_immediate_assign (UBYTE id_type, UBYTE chan_need);
EXTERN void dat_rr_release_ind               (USHORT relcs, UBYTE sapi);
EXTERN void dat_send_release_ind             (USHORT relcs);
EXTERN void dat_send_assign_fail_msg         (UBYTE cause);
EXTERN void dat_send_rr_status_msg           (UBYTE cause);
EXTERN void dat_send_handov_fail_msg         (UBYTE cause);
#if defined (REL99) && defined (TI_PS_FF_EMR)
EXTERN void dat_code_enh_measure_report      (T_MPH_MEASUREMENT_IND *report);
#endif

#if defined (TI_PS_FF_RTD) AND defined (REL99)
EXTERN void dat_update_rtd_data              (T_D_MEAS_INF *p_mi,T_rr_enh_para *p_temp);
EXTERN void dat_update_common_rtd_struct              (T_rtdd *rtdd_struct,T_rr_enh_para *p_temp);
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */
/*
 *  Formatter
 */

/*
 *  formatter primitives
 */
EXTERN       void for_mph_unitdata_ind        (T_MPH_UNITDATA_IND  *mph_unitdata_ind);
EXTERN       void for_dl_data_ind             (T_DL_DATA_IND       *dl_data_ind);
#if defined (REL99) && defined (TI_PS_FF_EMR)
EXTERN       void for_dl_short_unitdata_ind   (T_DL_SHORT_UNITDATA_IND *dl_short_unitdata_ind);
#endif
EXTERN       void for_rr_trace_message        (UBYTE         *payload,
                                               USHORT        length,
                                               USHORT        arfcn,
                                               ULONG         fn,
                                               UBYTE         direction);
/*
 *  formatter signalling
 */
EXTERN void for_dat_data_req            (T_DL_DATA_REQ       *data_req);
EXTERN void for_dat_est_req             (T_DL_ESTABLISH_REQ  *est_req);
EXTERN void for_dat_est_req_not_coding  (T_DL_ESTABLISH_REQ  *est_req);
EXTERN void for_dat_est_req_content     (T_DL_ESTABLISH_REQ  *est_req_content);
EXTERN void for_dat_l3_data_req         (T_DL_DATA_REQ       *dl_data_req);
EXTERN void for_dat_reconnect_req       (T_DL_RECONNECT_REQ  *reconnect_req);
EXTERN void for_dat_resume_req          (T_DL_RESUME_REQ     *resume_req);
EXTERN void for_dat_unitdata_req        (T_DL_UNITDATA_REQ   *unitdata_req);
#if defined (REL99) && defined (TI_PS_FF_EMR)
EXTERN void for_update_ba_ind                 (UBYTE index, UBYTE ba_ind);
EXTERN void for_check_and_configure_si2quater (UBYTE index);
EXTERN void for_att_update_ba2bsic_mapping    (T_rr_enh_para *p_enh);
EXTERN BOOL for_dat_process_common_emr_data (T_gprs_rep_prio *p_rep,
                                             T_gprs_bsic *p_bl,
                                             UBYTE  msg_index,
                                             BOOL ba_available);
EXTERN void for_dat_spd_unitdata_req    (T_DL_SHORT_UNITDATA_REQ *p_msg);
#endif

/*
 *  formatter procedures
 */
EXTERN void for_check_assign_cmd        (T_DL_DATA_IND       *dl_data_ind,
                                         T_D_ASSIGN_CMD      *ass_cmd);
EXTERN BOOL for_check_ba_range          (T_ba_range          *ba_range);
EXTERN void for_check_cell_descr        (T_cell_desc         *cell_desc);
EXTERN void for_check_channel_descr     (T_chan_desc         *chan_desc);
EXTERN void for_check_channel_mode      (UBYTE               ch_mod);
EXTERN void for_check_multirate_conf    (T_multirate_conf    * multirate_conf, UBYTE chan_type);
EXTERN void for_check_cipher_mode_set   (T_ciph_mode_set     *ciph);
EXTERN void for_check_handov_cmd        (T_DL_DATA_IND       *dl_data_ind,
                                         T_D_HANDOV_CMD      *handov_cmd);
EXTERN void for_create_channel_list     (T_f_range           *f_range,
                                         T_LIST              *cha_list);

EXTERN void   for_suspend_layer_2       (void);
#if defined (REL99) && defined (TI_PS_FF_EMR)
EXTERN BOOL for_process_si2quater(T_si_2qua_octets *p_si2q);
EXTERN void for_update_emr_rep_para(T_si_2qua_octets *p_si2q,T_rr_enh_para *p_em);
EXTERN void for_set_default_emr_data(T_rr_enh_para  *p_em);
EXTERN void for_send_enh_para(T_rr_enh_para *p_src);
EXTERN BOOL for_update_enh_cell_list (USHORT *p_list);
EXTERN BOOL for_process_common_emr_data (T_gprs_rep_prio *p_rep,
                                             T_gprs_bsic *p_bsic,
                                             UBYTE  msg_index,
                                             BOOL ba_available);
EXTERN void for_mon_si2quater_req(UBYTE action);
#endif

#if defined (TI_PS_FF_RTD) AND defined (REL99)
EXTERN void for_store_rtd_data (T_si_2qua_octets *p_si2q,T_rr_enh_para *p_temp);
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */


/*
 * If all entities are linked into one module this definitions
 * prefixes all this functions with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define for_set_content_error      _ENTITY_PREFIXED(for_set_content_error)
#endif

EXTERN UBYTE for_check_frequency (USHORT channel);
EXTERN void for_set_content_error       (UBYTE               value);

EXTERN void rr_csf_read_imei            (T_mob_ident         *imei);

/*----------------------------------------------------------------------
 * functions called from other entities
 *---------------------------------------------------------------------*/
/*lint -esym(759,rr_csf_get_radio_access_capability)*/
/*lint -esym(759,rr_csf_get_classmark1)*/
/*lint -esym(759,rr_csf_get_classmark2)*/
/*lint -esym(759,rr_csf_get_rf_capability)*/
#ifdef GPRS
EXTERN UBYTE rr_csf_get_radio_access_capability
                                        (T_ra_cap            *ra_cap);
#endif
EXTERN UBYTE rr_csf_get_classmark1      (T_mob_class_1       *mob_class_1);
EXTERN UBYTE rr_csf_get_classmark2      (T_mob_class_2       *mob_class_2);
EXTERN T_rf_cap* rr_csf_get_rf_capability (void);


EXTERN UBYTE rr_csf_fit_capability      (void);
EXTERN void rr_csf_ms_cap               (void);
EXTERN void rr_csf_read_rfcap           (void);
EXTERN void rr_csf_check_rfcap           (UBYTE init);
EXTERN void rr_csf_get_freq_bands       (UBYTE               *pfreq_bands);
GLOBAL void rr_csf_trace_power          (void);

#ifdef REL99
/*
 * Function added to get MSCR flag informaion that used in send sequence number
 */
EXTERN void get_msc_release_version(UBYTE* mscr);
#endif

#if defined(_SIMULATION_FFS_)
GLOBAL void rr_csf_ffs_init                (void);
GLOBAL void cs_store_white_list            (void);
GLOBAL void rr_csf_write_black_list        (T_LIST *black_list);
GLOBAL void rr_csf_read_black_list         (void);
GLOBAL void rr_csf_write_white_list        (T_CS_WHITE_LIST *white_list);
GLOBAL void rr_csf_read_white_list         (void);
GLOBAL void rr_csf_write_rxlev_thr         (UBYTE rxt, SHORT valno, char* val[MAX_NUM_BANDS]);
GLOBAL void rr_csf_read_rxlev_thr          (void);
GLOBAL void rr_csf_write_mcc_shield_to_ffs (void);
#endif /* _SIMULATION_FFS_ */

/*
 *  timer
 */
#define TIMERSTART(i,v)                 tim_start_timer (i, v)
#define TIMERSTOP(i)                    tim_stop_timer (i)
#if defined(TIMER_TRACE)
#define IS_TIMER_ACTIVE(i)              tim_check_timer(i)
#else  /* TIMER_TRACE */
#define IS_TIMER_ACTIVE(i)              (rr_data->t_running[i])
#endif  /* TIMER_TRACE */
/*
 * RR services
 */
/*
 * If all entities are linked into one module this definitions
 * prefixes all this functions with the enity name
 */
#ifdef OPTION_MULTITHREAD 
  #define srv_store_prim         _ENTITY_PREFIXED(srv_store_prim)
  #define srv_use_stored_prim    _ENTITY_PREFIXED(srv_use_stored_prim)
  #define srv_clear_stored_prim  _ENTITY_PREFIXED(srv_clear_stored_prim)
  #define srv_check_stored_prim  _ENTITY_PREFIXED(srv_check_stored_prim)
/* obsolete
  #define srv_find_stored_prim   _ENTITY_PREFIXED(srv_find_stored_prim)
  #define srv_flush_stored_prim  _ENTITY_PREFIXED(srv_flush_stored_prim)
*/
#endif
EXTERN BOOL   srv_store_prim            (T_PRIM              *prim);
EXTERN void   srv_use_stored_prim       (void);
EXTERN void   srv_clear_stored_prim     (ULONG opc);
EXTERN UBYTE  srv_check_stored_prim     (ULONG opc);
/* obsolete
EXTERN UBYTE  srv_find_stored_prim      (ULONG opc);
*/

#if defined (REL99) && defined (TI_PS_FF_EMR)
#define DEFAULT_SERV_BAND_REP 3 /*Number of cells to be reported in EMR for serving band*/
#endif

EXTERN void   srv_remove_frequencies_in_array (USHORT* arfcn_list);
EXTERN UBYTE  srv_remove_frequencies_in_array_gen (USHORT* arfcn_list, UBYTE c_arfcn_list);

EXTERN int    srv_create_chan_mob_alloc      (T_LIST              *clist,
                                              T_LIST              *list,
                                              UBYTE               *mobile_alloc);
EXTERN int    srv_check_frequencies_in_list  (T_LIST             *clist);
EXTERN void   srv_trace_black_list           (void);
EXTERN void   srv_trace_white_list           (void);

/* Access function in RR to get cell alloc list. Called from GRR*/
/*lint -esym(759,srv_get_cell_alloc_list)*/
/*lint -esym(759,srv_get_cell_alloc_list)*/
EXTERN void   srv_get_cell_alloc_list     (T_LIST              *target_list);

EXTERN void   rr_pei_primitive            (T_PRIM              *prim);
/*
 * RX
 *
 */

#ifdef FF_PS_RSSI
EXTERN  void   RX_SetValue    (UBYTE new_value, UBYTE new_qual, UBYTE new_access);
#else
EXTERN void RX_SetValue (UBYTE new_value);
#endif

/*
 *  Radio resource primitives Engineering Mode
 *  Bitmask for the event tracing
 */
EXTERN  T_RR_DATA              rr_data_base;
#define GET_INSTANCE_DATA    register T_RR_DATA *rr_data= &rr_data_base

#define ENTITY_DATA                rr_data

/*
*  These Functions are only temporary valid and should replaced as soon as possible
*/
/*lint -esym(759,get_network_meas)*/
/*lint -esym(759,get_bcch_chan_list)*/
EXTERN UBYTE get_network_meas   (UBYTE * chan_list);
EXTERN UBYTE get_bcch_chan_list (stk_data_type * out_stk_data);

#if !defined (_SIMULATION_)
EXTERN UBYTE * dl_get_sacch_buffer (void);
#endif


/*
 * If all entities are linked into one module this definitions
 * prefixes the global data with the enity name
 */
#ifdef TI_PS_HCOMM_CHANGE
EXTERN T_HANDLE  rr_handle;
#else /* for hCommHandles backward compatibility */
#ifdef OPTION_MULTITHREAD
  #define hCommMM        _ENTITY_PREFIXED(hCommMM)
#if defined FF_EOTD
  #define hCommLC        _ENTITY_PREFIXED(hCommLC)
  #define hCommRRLP      _ENTITY_PREFIXED(hCommRRLP)
#endif /* FF_EOTD */
  #define hCommDL        _ENTITY_PREFIXED(hCommDL)
  #define hCommPL        _ENTITY_PREFIXED(hCommPL)
  #define hCommMMI       _ENTITY_PREFIXED(hCommMMI)
#ifdef FF_WAP
  #define hCommWAP       _ENTITY_PREFIXED(hCommWAP) /*FMM */
#endif
#endif

EXTERN T_HANDLE  hCommMM;                /* MM  Communication        */
#if defined FF_EOTD
EXTERN T_HANDLE  hCommLC;                /* LC  Communication        */
EXTERN T_HANDLE  hCommRRLP;              /* RRLP Communication       */
#endif /* FF_EOTD */
EXTERN T_HANDLE  hCommDL;                /* DL  Communication        */
EXTERN T_HANDLE  hCommPL;                /* PL  Communication        */
EXTERN T_HANDLE  rr_handle;
EXTERN T_HANDLE  hCommMMI;               /* EM  Communication       */
#ifdef FF_WAP
  EXTERN T_HANDLE  hCommWAP;               /* FMM  Communication       */
#endif
#endif /* TI_PS_HCOMM_CHANGE */

/*
 * If all entities are linked into one module this definitions
 * prefixes the global data with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define _decodedMsg   _ENTITY_PREFIXED(_decodedMsg)
#endif

#if defined (REL99) && defined (TI_PS_FF_EMR)
#define MAX_MSTRUCT_CCD_RR MAXIMUM(MAX_MSTRUCT_LEN_RR_SHORT, MAX_MSTRUCT_LEN_RR)
EXTERN UBYTE          _decodedMsg [MAX_MSTRUCT_CCD_RR];
#else
EXTERN UBYTE          _decodedMsg [MAX_MSTRUCT_LEN_RR];
#endif

#ifdef OPTION_TIMER
  /*
   * If all entities are linked into one module this definitions
   * prefixes the global data with the entity name
   */
  #ifdef OPTION_MULTITHREAD
    #define partab    _ENTITY_PREFIXED(partab)
  #endif

/*  EXTERN const KW_DATA partab[];*/
#endif

/*
 * backward compatibility for some channel modes
 */
#if !defined CM_DATA_14_4
#define CM_DATA_14_4 0x0F
#define CM_EFR       0x21
#endif

/*
*  Definitions for AMR
*/
#define CM_AMR             0x41
#define MAX_NO_ACS         0x04
/*#define MAX_CODEC_MODES    0x03*/

/* FreeCalypso change: same as in ../dl/dl.h */
#if 0 && !defined(_FILE10_)
#define __FILE10__    (__FILE__+sizeof(__FILE__)-11)
#endif  /* !_FILE10_ */

#if defined(SYST_TRACE)
#undef  SYST_TRACE
#undef  SYST
#undef  SYST_TRACE_P
#endif  /* !SYST_TRACE */
#if defined(NEW_FRAME)
#define SYST_TRACE(a) vsi_o_ttrace((T_HANDLE)0, (ULONG)-1,a)
#define SYST           (T_HANDLE)0, (ULONG)-1
#define SYST_TRACE_P(a) vsi_o_ttrace a
#else
#define SYST_TRACE(a) vsi_o_trace("", 0xFFFF,a)
#define SYST           "", 0xffff
#define SYST_TRACE_P(a) vsi_o_trace a
#endif  /* NEW_FRAME */
/*
 * use it as showed next line...
 * SYST_TRACE_P((SYST, "e.g. two parameter: %d %d", p1, p2));
 */

#if defined(DL_TRACE_ENABLED)
  EXTERN void dl_trace                   (UCHAR                    trace_type,
                                          UCHAR                    channel,
                                          UCHAR                    ch_type,
                                          UCHAR                  * data);
  #ifdef OPTION_MULTITHREAD
    #define dl_trace_buf    _ENTITY_PREFIXED(dl_trace_buf)
  #endif
  #if defined(RR_PEI_C)
    GLOBAL char dl_trace_buf[100];
  #else  /* RR_PEI_C */
    EXTERN char dl_trace_buf[100];
  #endif  /* RR_PEI_C */
    #define TRACE_RR_EVENT   5
    #define DOES_NOT_MATTER  0
    #define DL_OFFLINE_TRACE(s) \
      dl_trace (TRACE_RR_EVENT, DOES_NOT_MATTER, DOES_NOT_MATTER, (UCHAR*)s)
#endif  /* DL_TRACE_ENABLED */

#if defined(ARRAY_TRACE)
  #ifdef OPTION_MULTITHREAD
    #define array_trace    _ENTITY_PREFIXED(array_trace)
  #endif
EXTERN void  array_trace (UBYTE*array, int size, char *titel);
#define TRACE_ARRAY(array, size, titel) array_trace(array, size, titel)
#else  /* ARRAY_TRACE */
#define TRACE_ARRAY(array, size, titel)
#endif  /* ARRAY_TRACE */

#if defined(_SIMULATION_)
#define TRACE_EVENT_WIN(s)                                TRACE_EVENT(s)
#define TRACE_EVENT_WIN_P1(s,a1)                          TRACE_EVENT_P1(s,a1)
#define TRACE_EVENT_WIN_P2(s,a1,a2)                       TRACE_EVENT_P2(s,a1,a2)
#define TRACE_EVENT_WIN_P3(s,a1,a2,a3)                    TRACE_EVENT_P3(s,a1,a2,a3)
#define TRACE_EVENT_WIN_P4(s,a1,a2,a3,a4)                 TRACE_EVENT_P4(s,a1,a2,a3,a4)
#define TRACE_EVENT_WIN_P5(s,a1,a2,a3,a4,a5)              TRACE_EVENT_P5(s,a1,a2,a3,a4,a5)
#define TRACE_EVENT_WIN_P6(s,a1,a2,a3,a4,a5,a6)           TRACE_EVENT_P6(s,a1,a2,a3,a4,a5,a6)
#define TRACE_EVENT_WIN_P7(s,a1,a2,a3,a4,a5,a6,a7)        TRACE_EVENT_P7(s,a1,a2,a3,a4,a5,a6,a7)
/*#define TRACE_EVENT_WIN_P8(s,a1,a2,a3,a4,a5,a6,a7,a8)     TRACE_EVENT_P8(s,a1,a2,a3,a4,a5,a6,a7,a8)
  #define TRACE_EVENT_WIN_P9(s,a1,a2,a3,a4,a5,a6,a7,a8,a9)  TRACE_EVENT_P9(s,a1,a2,a3,a4,a5,a6,a7,a8,a9)*/

/* trace for call of att_code_rr_abort_ind() */
/* Implements Measure#32: Row 18, 43, 90, 101, 111, 201, 211, and 324
 *                        Row 17, 41, 89, 100, 110, 200, 210, and 321 */
#define att_code_rr_abort_ind_trace(c) \
{ TRACE_EVENT_WIN_P2 ("att_code_rr_abort_ind(%x=%s)", c, S2I_STRING(#c));\
  att_code_rr_abort_ind_original(rr_data, c);}


#else  /* _SIMULATION_ */
#define TRACE_EVENT_WIN(s)
#define TRACE_EVENT_WIN_P1(s,a1)
#define TRACE_EVENT_WIN_P2(s,a1,a2)
#define TRACE_EVENT_WIN_P3(s,a1,a2,a3)
#define TRACE_EVENT_WIN_P4(s,a1,a2,a3,a4)
#define TRACE_EVENT_WIN_P5(s,a1,a2,a3,a4,a5)
#define TRACE_EVENT_WIN_P6(s,a1,a2,a3,a4,a5,a6)
#define TRACE_EVENT_WIN_P7(s,a1,a2,a3,a4,a5,a6,a7)
#define TRACE_EVENT_WIN_P8(s,a1,a2,a3,a4,a5,a6,a7,a8)
#define TRACE_EVENT_WIN_P9(s,a1,a2,a3,a4,a5,a6,a7,a8,a9)

#define att_code_rr_abort_ind_trace(c) \
{ TRACE_EVENT_P4 ("att_code_rr_abort_ind(%x=%s) #%u %s", c, #c, __LINE__, __FILE__);\
  att_code_rr_abort_ind_original(rr_data, c);}


#endif  /* _SIMULATION_ */

#define att_code_rr_abort_ind att_code_rr_abort_ind_trace

/*
 * Tracing of Layer 3 RR Messages.
 */

#define RR_BINDUMP(payload,length,arfcn,fn,dir) for_rr_trace_message(payload,length,arfcn,fn,dir)

#endif
