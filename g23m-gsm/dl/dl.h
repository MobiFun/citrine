/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  DL
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
|             Data Link Layer
+-----------------------------------------------------------------------------
*/

#ifndef __DL_H__
#define __DL_H__

#ifdef TI_PS_HCOMM_CHANGE
#include "cl_hComm_handle.h"
#endif

#if !defined(_SIMULATION_) && defined(WIN32)
#define _SIMULATION_
#endif  /* !_SIMULATION_ && WIN32 */

#if defined (_SIMULATION_)
#if defined(DL_2TO1)
#include <p_8010_148_l1test_sap.h>
#endif  /* DL_2TO1 */
#else /* _SIMULATION_ */
#if !defined(_TARGET_)
#define _TARGET_
#endif  /* !_TARGET_ */
#endif  /* _SIMULATION_ */

/*
 * the following defines have an impact on the behaviour of the layer 2
 */
/*
 * DL_TRACE_ENABLED
 * is set   : Enables the trace feature (buffered or immediately trace)
 * isn't set: Disables the trace feature
 */
#define DL_TRACE_ENABLED

/*
 * DL_IMMEDIATE_TRACE
 * operates only with enabled trace feature!
 * is set   : Enables unbuffered trace. Write out the trace immediately without
 *            buffering (or rather delayed by signalling to itself while the MS
 *            is in interrupt context state).
 * isn't set: Enables buffered trace (write out in idle mode only)
 * Attention: Due to the interrupt context of the uplink and downlink trace
 *            this feature requires very fast trace capability. The normal trace
 *            transfer rate of 115200 Baud is not sufficient!
 */
#define DL_IMMEDIATE_TRACE

/*
 * DISABLE_MEASREPORT_TRACE
 * operates only with enabled buffered trace!
 * is set   : Disables trace of measurement reports
 * isn't set: Normal trace of measurement reports
#define DISABLE_MEASREPORT_TRACE
 */
/*
 * DISABLE_EMPTY_UI
 * operates only with enabled buffered trace!
 * is set   : Disables trace of empty frames
 * isn't set: Normal trace of empty frames
 */
#define DISABLE_EMPTY_UI

/*
 * DELAYED_RELEASE_IND
 * is set   : Delayed release indication after reception of DISC to enable
 *            layer 1 the transmission of the UA acknowledge (After sent of the
 *            release indication to RR, RR stops the dedicated mode in layer 1).
 *            During the delay, DL sends only dummy frames for some uplink
 *            opportunities (see DL_FACCH_RELEASE_DELAY_VALUE and
 *            DL_SDCCH_RELEASE_DELAY_VALUE). The delay stops after the given
 *            count of uplinked dummy frames or at a downlink on this channel.
 * isn't set: No delayed release indication
 */
#define DELAYED_RELEASE_IND

#if defined(DELAYED_RELEASE_IND)
/*
 * When RR stops the dedicated mode, the TI layer is not capable for correct
 * completion of the UA response transmission. Therefore DL delays the release
 * indication to RR for some uplink opportunities to give layer 1 time to
 * transmit the frame completely before RR stops the dedicated mode.
 */
#define DL_FACCH_RELEASE_DELAY_VALUE    4 /* number of wait dummies on FACCH */
#define DL_SDCCH_RELEASE_DELAY_VALUE    1 /* number of wait dummies on SDCCH */
#endif  /* DELAYED_RELEASE_IND */

/*
 * DELAYED_SABM
 * is set   : To give layer 1 some time to switch the physical channel on FACCH
 *            DL delays the transmit of the SABM frame. After (re-)establishment
 *            request from RR, DL sends only dummies (return NULL) for the first
 *            uplink opportunities (see DL_FACCH_SABM_DELAY_VALUE).
 * isn't set: No delayed uplink of the SABM frame.
 */
#define DELAYED_SABM

#if defined(DELAYED_SABM)
#undef DELAYED_SABM
#endif  /* DELAYED_SABM */

#if defined(DELAYED_SABM)
/*
 * To give layer 1 some time to switch the physical channel on FACCH
 * DL delays the transmit of the SABM frame. After (re-)establishment request
 * from RR, DL sends only dummies (return NULL) for the first uplink
 * opportunities.
 */
#define DL_FACCH_SABM_DELAY_VALUE       1
#endif  /* DELAYED_SABM */

/*
 * LATE_LEAVING_DEDICATED
 * is set   : The leaving of dedicated mode happens in latest moment, after
 *            the reception of an UA frame after sent of DISC
 *            or before uplink of the UA frame after reception of a DISC frame.
 * isn't set: In this case the dedicated mode is leaving immediately after
 *            the detection of a release (after the incoming L3 message
 *            CHANNEL RELEASE, or after reception of a DISC frame or before
 *            the uplink of a DISC frame).
 */
#define LATE_LEAVING_DEDICATED

/*
 * IFRAME_AS_RR
 * is set   : I frame might response with I frame
 * isn't set: I frame must response with RR frame
 */
#define IFRAME_AS_RR

/*
 * INVOKE_SIGNAL
 * is set   : DL does not send primitives or traces within L1 interrupt context
 *            anymore. It sends signals instead. The primitives or traces are
 *            delayed until the entity will be in the own context.
 * isn't set: Primitves or traces are processed immediately even is DL within
 *            in L1 interrupt context.
 */
#define INVOKE_SIGNAL

/*
 * SEND_FN_TO_L2_IN_DCCH
 * is set to 1: The function dll_dcch_downlink() contains one more parameter,
                the absolute frame number FN.
 * is set to 0: No changes relating to dll_dcch_downlink.
 */
#define SEND_FN_TO_L2_IN_DCCH 1

/*
 * RR_SHORT_PD_DETECT_KNOWN_MSG_ONLY
 * is set to 1: The Bter format is detected for known message types
 *              only (the known messages are defined some lines after in
 *              this header file).
 * is set to 0: The Bter format is detected on the basis of the short L2 header
 *              format. This is a more general approach, but with the
 *              disadvantage of possibly misconstrued invalid messages.
 */
//#define RR_SHORT_PD_DETECT_KNOWN_MSG_ONLY

/* 
 * DL_2TO1 flag is used to identify new L1 interface via the MPHC entity
 * as well as the test interface of the new 2to1 stack.
 */
#ifdef DL_2TO1
#define MAX_L2_FRAME_SIZE L1_MAX_L2_FRAME_SIZE
#define T_RADIO_FRAME T_L1_RADIO_FRAME
#define T_PH_DATA_IND T_MPHC_PH_DATA_IND
#define PH_DATA_IND MPHC_PH_DATA_IND
#define L2_CHANNEL_SDCCH L1_L2_CHANNEL_SDCCH
#define L2_CHANNEL_FACCH_F L1_L2_CHANNEL_FACCH_F
#define L2_CHANNEL_FACCH_H L1_L2_CHANNEL_FACCH_H
#define L2_CHANNEL_SACCH L1_L2_CHANNEL_SACCH
#define CM_SIGNALLING_ONLY MPHC_CM_SIGNALLING_ONLY
#define CM_TCH_FS MPHC_CM_TCH_FS
#define CM_TCH_HS MPHC_CM_TCH_HS
#define VALID_BLOCK L1_VALID_BLOCK
#define DATA_VALID MPHC_DATA_VALID
#define DL_SAPI_0 PS_SAPI_0
#define DL_SAPI_3 PS_SAPI_3
#define PH_READY_TO_SEND L1TEST_CALL_MPHC_READ_DCCH
#endif /*DL_2TO1*/

#if defined(_SIMULATION_)
#define  DL_TRACE_ENABLED           /* trace during simulation */
#define  DL_IMMEDIATE_TRACE         /* is write out immediately */
#undef   DISABLE_MEASREPORT_TRACE   /* enable trace of measurement reports */
#undef   DISABLE_EMPTY_UI           /* enable trace of empty frames */
#define  DL_TRACE_WIN32

#define TRACE_EVENT_WIN(s)                                TRACE_EVENT(s)
#define TRACE_EVENT_WIN_P1(s,a1)                          TRACE_EVENT_P1(s,a1)
#define TRACE_EVENT_WIN_P2(s,a1,a2)                       TRACE_EVENT_P2(s,a1,a2)
#define TRACE_EVENT_WIN_P3(s,a1,a2,a3)                    TRACE_EVENT_P3(s,a1,a2,a3)
#define TRACE_EVENT_WIN_P4(s,a1,a2,a3,a4)                 TRACE_EVENT_P4(s,a1,a2,a3,a4)
#define TRACE_EVENT_WIN_P5(s,a1,a2,a3,a4,a5)              TRACE_EVENT_P5(s,a1,a2,a3,a4,a5)
#define TRACE_EVENT_WIN_P6(s,a1,a2,a3,a4,a5,a6)           TRACE_EVENT_P6(s,a1,a2,a3,a4,a5,a6)
#if 0
#define TRACE_EVENT_WIN_P7(s,a1,a2,a3,a4,a5,a6,a7)        TRACE_EVENT_P7(s,a1,a2,a3,a4,a5,a6,a7)
#endif /* 0 */
#define TRACE_EVENT_WIN_P8(s,a1,a2,a3,a4,a5,a6,a7,a8)     TRACE_EVENT_P8(s,a1,a2,a3,a4,a5,a6,a7,a8)
#define TRACE_EVENT_WIN_P9(s,a1,a2,a3,a4,a5,a6,a7,a8,a9)  TRACE_EVENT_P9(s,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#else  /* _SIMULATION_ */
#undef  DL_TRACE_WIN32

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
#endif  /* _SIMULATION_ */


/*==== CONSTANTS ==================================================*/
/*
 * Frame Header Macros
 */
#define GET_LENGTH_INDICATOR(s)   ((((s)[2]) & 0xFC)>>2)
#define GET_P_BIT(s)              ((((s)[1]) & 0x10)>>4)
#define GET_M_BIT(s)              ((((s)[2]) & 0x02)>>1)
#define GET_RECEIVE_NUMBER(s)     ((((s)[1]) & 0xE0)>>5)
#define GET_SEND_NUMBER(s)        ((((s)[1]) & 0x0E)>>1)
#define GET_SAPI(s)               ((((s)[0]) & 0x1C)>>2)
#define GET_EA(s)                 (((s)[0]) & 1)
#define GET_FORMAT_TYPE(s)        (((s)[1]) & 3)
#define GET_CR(s)                 ((((s)[0]) & 2)>>1)
#define GET_EL(s)                 (((s)[2]) & 1)
#define GET_S_TYPE(s)             (((s)[1]) & 0x0F)
#define GET_U_TYPE(s)             (((s)[1]) & 0xEF)
#define GET_BTER_FORMAT(s)        (((s)[0]) & (BTER_FORMAT_MASK))
#if 0
#define GET_LENGTH_FIELD(s)       (((s)[2]))
#define GET_PSEUDO_LENGTH(s)      ((((s)[0]) & 0xFC)>>2)
#endif /* 0 */



/* Boolean constants */
#if !defined(TRUE)
#define TRUE  (1 EQ 1)
#define FALSE (1 EQ 0)
#endif

#if !defined(ELEMENTS)
#define ELEMENTS(array) (sizeof(array)/sizeof(array[0]))
#endif  /* !ELEMENTS */

/*
 * Bitoffset for encoding/decoding
 */
#define ENCODE_OFFSET 24

/*
 * TI Circuit Switches Interface
 */
#define SIG_ONLY       0
#define NO_SIGNALLING  1

/*
 * Frame Sizes
 * Maximum number of octets for the information field for frames of
 * format A and B (SACCH, SDCCH, FACCH)
 */
#define N201_SDCCH     DL_N201_DCCH_A_B
#define N201_SACCH     DL_N201_SACCH_A_B
#define N201_FACCH     DL_N201_DCCH_A_B

/*
 * VTX commands
 */
#define EMPTY_CMD        0
#define RR_CMD           1
#define SABM_CMD         2
#define DISC_CMD         3
#define UA_CMD           4
#define DM_CMD           5
#define RR_RSP           6
#define REJ_CMD          7

/*
 * Format Types (Control field format: octet bits 2+1)
 */
#define I_FORMAT      0       /* information transfer format (only bit 1) */
#define I1_FORMAT     2       /* information transfer format (only bit 1) + N(S) */
#define S_FORMAT      1       /* supervisory format (bits 2+1) */
#define U_FORMAT      3       /* unnumbered format (bits 2+1) */

/*
 * Frame Types  (Control field format: octet bits 8-6,4-1)
 */
#define I_FRAME       0       /* I format (only bit 1) */
#define RR_FRAME      1       /* S format (bits 4-1) */
#define RNR_FRAME     5       /* S format (bits 4-1) */
#define REJ_FRAME     9       /* S format (bits 4-1) */
#define DISC_FRAME    0x43     /* U format (bits 8-6,4-1) */
#define SABM_FRAME    0x2F     /* U format (bits 8-6,4-1) */
#define UI_FRAME      0x03     /* U format (bits 8-6,4-1) */
#define DM_FRAME      0x0F     /* U format (bits 8-6,4-1) */
#define UA_FRAME      0x63     /* U format (bits 8-6,4-1) */

/*
 * L2 Header Values
 */
#define MS2BS_CMD    0
#define MS2BS_RSP    1
#if 0
#define BS2MS_CMD    1
#define BS2MS_RSP    0
#define ADDR_RESP_SAPI0  0x03
#define ADDR_RESP_SAPI3  0x0F
#define ADDR_CMD_SAPI0   0x01
#define ADDR_CMD_SAPI3   0x0D
#define UA_F0            0x63
#define UA_F1            0x73
#define RR_F0            0x01
#define RR_F1            0x11
#define RR_P0            0x01
#define RR_P1            0x11
#define REJ_F0           0x09
#define REJ_F1           0x19
#define DISC_P0          0x43
#define DISC_P1          0x53
#define SABM_P0          0x2F
#define SABM_P1          0x3F
#define DM_F0            0x0F
#define DM_F1            0x1F
#define UI_P0            0x03
#define I_P0             0x00
#define I_P1             0x10
#define LENGTH_ZERO      0x01
#endif /* 0 */

/*
 * Message types for RR messsages using the RR short PD
 *
 * up/downlink          message type   channel  name
 * BSS->MS downlink    | 0 0 0 0 0 | |  SACCH   System Information Type 10
 * BSS->MS downlink    | 0 0 0 0 1 | |  FACCH   Notification/FACCH
 * BSS->MS downlink    | 0 0 0 1 0 | |   DCCH   Uplink Free
 * MS->BSS uplink      | 0 0 1 0 0 | |  SACCH   Enhanced Measurement Report
 * BSS->MS downlink    | 0 0 1 0 1 | |  SACCH   Measurement Information
 *                     |           | |
 *                     0           | |          RR short PD
 *                                 0 0          Short L2 header type 1
 */

#define RR_SHORT_PD               0
#define L2_SHORT_HEAD             0
#if defined(RR_SHORT_PD_DETECT_KNOWN_MSG_ONLY) /* detection of known messages only */
#define RR_SHORT_PD_SI10          (((RR_SHORT_PD)<<7)|(0x00<<2)|(L2_SHORT_HEAD)) /* 0x00 */
#define RR_SHORT_PD_NOTI_FACCH    (((RR_SHORT_PD)<<7)|(0x01<<2)|(L2_SHORT_HEAD)) /* 0x04 */
#define RR_SHORT_PD_UPLINK_FREE   (((RR_SHORT_PD)<<7)|(0x02<<2)|(L2_SHORT_HEAD)) /* 0x08 */
#define RR_SHORT_PD_ENH_MEAS_REP  (((RR_SHORT_PD)<<7)|(0x04<<2)|(L2_SHORT_HEAD)) /* 0x10 */
#define RR_SHORT_PD_MEAS_INFO     (((RR_SHORT_PD)<<7)|(0x05<<2)|(L2_SHORT_HEAD)) /* 0x14 */
#endif /* RR_SHORT_PD_DETECT_KNOWN_MSG_ONLY */
/* general mask for Bter format */
#define NOT_RR_SHORT_PD           ((~RR_SHORT_PD)&0x01)
#define NOT_L2_SHORT_HEAD         ((~L2_SHORT_HEAD)&0x03)
#define BTER_FORMAT_MASK          (((NOT_RR_SHORT_PD)<<7)|(0x00<<2)|(NOT_L2_SHORT_HEAD)) /* 0x83 */
#define SHORT_L2_HEADER_TYPE_1    (((RR_SHORT_PD)<<7)|(0x00<<2)|(L2_SHORT_HEAD)) /* 0x00 */
/*
 * Repetitions
 */
#define FACCH_N200_FR     34
#define FACCH_N200_HR     29
#define SDCCH_N200        23
#define SACCH_N200         5
#define N200_ESTABLISHMENT 5

#ifndef DL_2TO1
  /* align DL.sap with 8010_152_PS_Include.sap */
  #define PS_SAPI_0                     DL_SAPI_0
  #define PS_SAPI_3                     DL_SAPI_3
  /* align DL.sap with 8010_153_Cause_Include.sap */
  #define CAUSE_DL_INFO_FIELD_MISMATCH  DL_INFO_FIELD_MISMATCH
#endif /* DL_2TO1 */

#define C_DCCH0             0 /* SAPI=0: SDCCH, FACCH */
#define C_DCCH3             1 /* SAPI=3: SDCCH, SACCH */
#define C_SACCH0            2 /* SAPI=0: SACCH  */
#define MAX_CHANNELS        3

#define PROCESS_NAME_INIT          \
        "DCCH0",                   \
        "DCCH3",                   \
        "SACCH0"

/*
 * States of all Processes
 */
#define STATE_INVALID                     0
#define STATE_DISABLED                    1
#define STATE_IDLE_DL                     2
#define STATE_SUSPENDED                   3
#define STATE_CONTENTION_RESOLUTION       4
#define STATE_AWAITING_ESTABLISHMENT      4
#define STATE_MULTIPLE_FRAME_ESTABLISHED  5
#define STATE_TIMER_RECOVERY              6
#define STATE_AWAITING_RELEASE            7
#define MAX_STATES                        8

/*
 * States of the Process DCCH0, (SAPI=0: SDCCH, FACCH)
 */
#define STATE_DCCH0_NAME_INIT         \
        "invalid"                     ,\
        "DISABLED"                    ,\
        "IDLE"                        ,\
        "SUSPENDED"                   ,\
        "CONTENTION_RESOLUTION"       ,\
        "MULTIPLE_FRAME_ESTABLISHED"  ,\
        "TIMER_RECOVERY"              ,\
        "AWAITING_RELEASE"
/*
 * States of the Process DCCH3 (SAPI=3: SDCCH, SACCH)
 */
#define STATE_DCCH3_NAME_INIT         \
        "invalid"                     ,\
        "DISABLED"                    ,\
        "IDLE"                        ,\
        "SUSPENDED"                   ,\
        "AWAITING_ESTABLISHMENT"      ,\
        "MULTIPLE_FRAME_ESTABLISHED"  ,\
        "TIMER_RECOVERY"              ,\
        "AWAITING_RELEASE"

/*
 * DL manages 3 (MAX_CHANNELS) states: SACCH0, DCCH0, DCCH3
 */
#ifdef OPTION_MULTITHREAD
  #define PROCESS_NAME    _ENTITY_PREFIXED(PROCESS_NAME)
#endif

/*
 * A definition is provided in DL_PEI.C
 */
#ifdef DL_PEI_C
  GLOBAL  const char * const STATE_DCCH0_NAME[MAX_STATES] = { STATE_DCCH0_NAME_INIT };
  GLOBAL  const char * const STATE_DCCH3_NAME[MAX_STATES] = { STATE_DCCH3_NAME_INIT };
  GLOBAL  const char * const PROCESS_NAME[MAX_CHANNELS] =   { PROCESS_NAME_INIT };

#if defined(DL_TRACE_WIN32)
  GLOBAL const char * const CH_TYPE_NAME[] =
  {
    "ch=0?",
    "SACCH",                   /* 0x1 SACCH                          */
    "SDCCH",                   /* 0x2 SDCCH                          */
    "FACCH_H",                 /* 0x3 FACCH Halfrate                 */
    "FACCH_F",                 /* 0x4 FACCH Fullrate                 */
    "CCCH",                    /* 0x5 CCCH                           */
    "NBCCH",                   /* 0x6 normal BCCH                    */
    "PCH",                     /* 0x7 PCH                            */
    "EPCH",                    /* 0x8 extended PCH                   */
    "CBCH",                    /* 0x9 Cell Broadcast Channel         */
    "EBCCH"                    /* 0xa extended BCCH                  */
  };
  GLOBAL const char * const VTX_NAME[] =
  {
    "EMPTY_CMD",        /* 0 */
    "RR_CMD",           /* 1 */
    "SABM_CMD",         /* 2 */
    "DISC_CMD",         /* 3 */
    "UA_CMD",           /* 4 */
    "DM_CMD",           /* 5 */
    "RR_RSP",           /* 6 */
    "REJ_CMD"           /* 7 */
  };
  GLOBAL const char * const SEND_NAME[] =
  {
    "RETURN_NULL",      /*  0 */
    "UPLINK_NULL",      /*  1 */
    "UPLINK_EMPTY",     /*  2 */
    "UPLINK_NORMAL",    /*  3 */
    "UPLINK_UA",        /*  4 */
    "UPLINK_UA_F",      /*  5 */
    "UPLINK_IFRAME",    /*  6 */
    "UPLINK_IFRAME_P",  /*  7 */
    "UPLINK_RR",        /*  8 */
    "UPLINK_RR_F",      /*  9 */
    "UPLINK_REJ",       /* 10 */
    "UPLINK_REJ_F",     /* 11 */
    "UPLINK_DCCH3",     /* 12 */
    "UPLINK_REPORT"     /* 13 */
  };
#endif  /* DL_TRACE_WIN32 */

#else   /* DL_PEI_C */
  EXTERN  const char * const STATE_DCCH0_NAME[MAX_STATES];
  EXTERN  const char * const STATE_DCCH3_NAME[MAX_STATES];
  EXTERN  const char * const PROCESS_NAME[MAX_CHANNELS];

#if defined(DL_TRACE_WIN32)
  EXTERN const char * const CH_TYPE_NAME[];
  EXTERN const char * const VTX_NAME[];
  EXTERN const char * const SEND_NAME[];
#endif  /* DL_TRACE_WIN32 */
#endif  /* DL_PEI_C */

/*
 * TRACE Constants
 */
#define TRACE_UPLINK    0
#define TRACE_DOWNLINK  1
#define TRACE_DL_EVENT  2
#define TRACE_CHSTATE   3
#define TRACE_PL_EVENT  4
#define TRACE_RR_EVENT  5
#define TRACE_UACK_UP   6
#define TRACE_UACK_DN   7

#define TRACE_CH_UNKNOWN  MAX_CHANNELS

/*
 * Error Causes
 */
#define T200_EXPIRED_N200_PLUS_1_TIMES        0
#define UNSOLICITED_UA_RESPONSE               2
#define UNSOLICITED_DM_RESPONSE               3
#define UNSOLICITED_DM_RESPONSE_ABNORMAL_REL  4
#define UNSOLICITED_SUPERVISORY_RESPONSE      5
#define SEQUENCE_ERROR                        6
#define U_FRAME_WITH_INCORRECT_PARAMETERS     7
#define S_FRAME_WITH_INCORRECT_PARAMETERS     8
#define I_FRAME_WITH_INCORRECT_USE_OF_M_BIT   9
#define I_FRAME_WITH_INCORRECT_LENGTH         10
#define FRAME_NOT_IMPLEMENTED                 11

/*
 * DL Uplink commands
 */
enum  uplink_enum
{
  RETURN_NULL = 0,
  UPLINK_NULL,
  UPLINK_EMPTY,
  UPLINK_NORMAL,
  UPLINK_UA,
  UPLINK_UA_F,
  UPLINK_IFRAME,
  UPLINK_IFRAME_P,
  UPLINK_RR,
  UPLINK_RR_F,
  UPLINK_REJ,
  UPLINK_REJ_F,
  UPLINK_DCCH3,
  UPLINK_REPORT
};

#if defined(__PEI_H__)  /* frame version 2.4.x */
typedef ULONG   T_SIGNAL_OPC;
#define FRAME_2_4_X
#elif defined(PEI_H)    /* frame version 2.3.x */
typedef USHORT  T_SIGNAL_OPC;
#define FRAME_2_3_X
#else                   /* frame version unknown */
#error "unknown frame version, missing PEI_H or __PEI_H__"
#endif  /* frame version */


/*==== TYPES ======================================================*/
typedef struct
{
  USHORT l_buf;
  USHORT o_buf;
  UBYTE  buf [DL_MAX_L2_FRAME_SIZE];
} T_FRAME;

/* Buffer concept:
 * Store buffer    Contains complete layer 3 messages for the uplink direction
 * Sending buffer  Contains the complete message which is just send on uplink
 * Transmit buffer Contains the segment which is just send on uplink
 * Switch buffer   Contains a layer 3 message in case of resumption or reconnection
 */
#define INDEX_MAX_STORE_BUFFER  (MAX_QUEUED_MESSAGES-1)
#define INDEX_SENDING_BUFFER    MAX_QUEUED_MESSAGES
#define INDEX_SWITCH_BUFFER     (MAX_QUEUED_MESSAGES+1)
typedef struct
{
  USHORT   no_of_stored_messages;
  T_DL_DATA_REQ * store_buffer [MAX_QUEUED_MESSAGES];
  T_DL_DATA_REQ * sending_buffer;
  T_DL_DATA_REQ * switch_buffer;
  USHORT   act_length;
  USHORT   act_offset;
  T_FRAME  transmit_buffer;
  UBYTE    m_bit;
} T_QUEUE;

typedef struct
{
  UBYTE       ch_type;
  UBYTE       vtx;
  UBYTE       T200_counter;
  UBYTE       time_flag;
  UBYTE       contention_resolution;
  UBYTE       reject_exception;
  UBYTE       acknowledge_pending;
  UBYTE       rc;
  UBYTE       f_bit;
  UBYTE       f_bit_flag;
  UBYTE       p_bit_flag;
  UBYTE       va;
  UBYTE       vr;
  UBYTE       vs;
} T_CCH;

#if defined(INVOKE_SIGNAL)
typedef struct
{
  UBYTE   ch_type;
  UBYTE   sapi;
  UBYTE   indication;
} T_DL_SIG_ESTABLISH_IND;

typedef struct
{
  UBYTE   ch_type;
  UBYTE   sapi;
} T_DL_SIG_ESTABLISH_CNF;

typedef struct
{
  UBYTE   ch_type;
  UBYTE   sapi;
  ULONG   fn;
} T_DL_SIG_DATA_IND;

typedef struct
{
  UBYTE   sapi;
} T_DL_SIG_DATA_CNF;

#if 0 /* happens in primitive context only */
typedef struct
{
  UBYTE   error_flag;
  UBYTE   layer1head[2];
  UBYTE   layer3msg[DL_N201_SACCH_A_B];
  UBYTE   length;
  ULONG   fn;
} T_DL_SIG_UNITDATA_IND;
#endif  /* 0 */

typedef struct
{
  UBYTE   ch_type;
  UBYTE   error_flag;
  UBYTE   layer1head[2];
  UBYTE   layer3msg[DL_N201_DCCH_Bter];
  UBYTE   length;
  ULONG   fn;
} T_DL_SIG_SHORT_UNITDATA_IND;

typedef struct
{
  UBYTE   ch_type;
  UBYTE   sapi;
  UBYTE   cs;
  BOOL    init;
} T_DL_SIG_RELEASE_IND;

typedef struct
{
  UBYTE   ch_type;
  UBYTE   sapi;
  BOOL    init;
} T_DL_SIG_RELEASE_CNF;

typedef struct
{
  UBYTE   ch_type;
  UBYTE   sapi;
} T_DL_SIG_ERROR_IND;

typedef struct
{
  void    *pointer;
} T_DL_SIG_FREE_POINTER;

typedef struct
{
  T_DL_DATA_IND ** in_msg;
  UBYTE   new_data_in[MAX_L2_FRAME_SIZE];
} T_DL_SIG_CONCATENATE;

typedef struct
{
  UBYTE   trace_type;
  UBYTE   channel;
  UBYTE   ch_type;
  UBYTE   data_len;
  T_TIME  trace_time;
  UBYTE   data[MAX_L2_FRAME_SIZE];
} T_DL_SIG_L2TRACE;

typedef struct
{
  UBYTE    type;
  UBYTE    ch_type;
  UBYTE   *frame;
} T_DL_SIG_L3TRACE;

typedef struct
{
  UBYTE   data[7 /*EM_DL_BUFFER_SIZE*/];
  UBYTE   length;
} T_DL_SIG_EM_WRITE;


typedef union
{
  T_DL_SIG_ESTABLISH_IND             establish_ind;
  T_DL_SIG_ESTABLISH_CNF             establish_cnf;
  T_DL_SIG_DATA_IND                  data_ind;
  T_DL_SIG_DATA_CNF                  data_cnf;
#if 0 /* happens in primitive context only */
  T_DL_SIG_UNITDATA_IND              unitdata_ind;
#endif  /* 0 */
  T_DL_SIG_SHORT_UNITDATA_IND        short_unitdata_ind;
  T_DL_SIG_RELEASE_IND               release_ind;
  T_DL_SIG_RELEASE_CNF               release_cnf;
  T_DL_SIG_ERROR_IND                 error_ind;
  T_DL_SIG_FREE_POINTER              free_pointer;
  T_DL_SIG_CONCATENATE               concatenate;
  T_DL_SIG_L3TRACE                   l3trace;
#if defined(DL_TRACE_ENABLED) && defined(DL_IMMEDIATE_TRACE)
  T_DL_SIG_L2TRACE                   l2trace;
#endif  /* DL_TRACE_ENABLED && DL_IMMEDIATE_TRACE */
#if defined(FF_EM_MODE)
  T_DL_SIG_EM_WRITE                  em_write;
#endif  /* FF_EM_MODE */
} T_DL_SIGNAL_DATA_UNION;

typedef struct
{
  BOOL                               busy;
#if defined(_SIMULATION_)
  UBYTE                              idx;
#endif  /* _SIMULATION_ */
  T_DL_SIGNAL_DATA_UNION             u;
} T_DL_SIGNAL_DATA;

#if defined(DL_TRACE_ENABLED)
    #define DL_SIGNAL_DATA_ELEMENTS 9
#else /* DL_TRACE_ENABLED */
    #define DL_SIGNAL_DATA_ELEMENTS 6
#endif /* DL_TRACE_ENABLED */

/*
 * number of signal data entries must be sufficient for
 * 1 data indication or confirmation
 * 1 error indication
 * 1-2 free commands
 * 1-2 engineering command
 * 2-3 traces
 */
typedef struct
{
  int               sig_idx;
  T_DL_SIGNAL_DATA  sig_data[DL_SIGNAL_DATA_ELEMENTS];
} T_DL_SIGNAL;
#endif /* INVOKE_SIGNAL */

typedef struct
{
  BOOL                  dl_active;
  BOOL                  interrupt_context;
  UBYTE                 state[MAX_CHANNELS];
  T_CCH                 cch[MAX_CHANNELS];
  T_FRAME               sacch_act_buffer;
  T_FRAME               sacch_last_buffer;
  T_FRAME               rr_short_pd_buffer;
  UBYTE                 rr_short_pd_ch_type;
  UBYTE                 sacch_mode;
  T_DL_DATA_IND       * dcch0_in_msg;
  T_DL_DATA_IND       * dcch3_in_msg;
  T_QUEUE               dcch0_queue;
  T_QUEUE               dcch3_queue;
  BOOL                  dcch0_unserved; /* storing of indication (unserved DL-DATA_REQ's) */
  BOOL                  dcch3_unserved; /* for use with DL-ESTABLISHMENT-CNF */
  T_RADIO_FRAME         l2_frame;/* Buffer for l2_frame and empty l2 frame */
  BOOL                  RR_dedicated;/* Flag for RR dedictated mode
                                      * (necessary for measurement reports) */
  ULONG                 fn; /* last received frame number */
  UBYTE                 dcch0_ch_type;
  UBYTE                 dcch0_disc_request;/* Flags for faster sending of DISC */
  UBYTE                 dcch3_disc_request;/* after L3 msg CAHNNEL RELEASE */

  /*
   * Flag for priority arrangement on the SACCH channel.
   * DL must ensure that if a SAPI=3 frame is awaiting transmission, two SAPI=3
   * frames are not sent in consecutive SACCH frames.
   * Dl must also be ensured that any SAPI=3 frame is followed by at least one
   * SAPI=0 frame.
   * This flag controls the priority of SMS messages and measurement reports.
   */
  UBYTE                 sacch_last_uplink_sapi;
#if !defined(FF_GTI)
#if defined(DELAYED_SABM)
  UBYTE                 dcch0_sabm_flag;
  UBYTE                 dcch0_sabm_delay;
#endif  /* DELAYED_SABM */
#if defined(DELAYED_RELEASE_IND)
  UBYTE                 release_ind_ch_type;
  UBYTE                 release_ind_sapi;
  UBYTE                 release_ind_delay;
#endif  /* DELAYED_RELEASE_IND */
#endif  /* !FF_GTI */
#if defined(INVOKE_SIGNAL)
  T_DL_SIGNAL           signal_struct;
#endif /* INVOKE_SIGNAL */

} T_DL_DATA_STORE;

/*
 * Predefined mesaurement reports
 *
 * If all entities are linked into one module this definitions
 * prefixes all this functions with the enity name
 */
#ifdef OPTION_MULTITHREAD
#define meas_report_no_nc _ENTITY_PREFIXED(meas_report_no_nc)
#endif
EXTERN const T_FRAME    meas_report_no_nc;

/*==== EXPORT =====================================================*/
#if defined(WIN32)
#include <stdio.h>
#endif  /* WIN32 */

/*
 * Prototypes Distribute RR
 */
EXTERN void drr_dl_establish_req       (T_DL_ESTABLISH_REQ     * est_req);
EXTERN void drr_dl_establish_ind       (UBYTE                    ch_type,
                                        UBYTE                    sapi,
                                        UBYTE                    indication);
EXTERN void drr_dl_resume_req          (T_DL_RESUME_REQ        * resume_req);
EXTERN void drr_dl_reconnect_req       (T_DL_RECONNECT_REQ     * reconnect_req);
EXTERN void drr_dl_release_req         (T_DL_RELEASE_REQ       * release_req);
#ifndef DL_2TO1
EXTERN void drr_mdl_release_req        (T_MDL_RELEASE_REQ      * release_req);
#endif /* DL_2TO1 */
EXTERN void drr_dl_suspend_req         (T_DL_SUSPEND_REQ       * suspend_req);
EXTERN void drr_dl_data_req            (T_DL_DATA_REQ          * data_req);
EXTERN void drr_dl_data_ind            (UBYTE                    sapi,
                                        ULONG                    fn);
EXTERN void drr_dl_data_cnf            (UBYTE                    sapi);
EXTERN void drr_dl_unitdata_req        (T_DL_UNITDATA_REQ      * unitdata_req);
EXTERN void drr_dl_unitdata_ind        (UBYTE                    error_flag,
                                        UBYTE                  * layer1head,
                                        UBYTE                  * layer3msg,
                                        UBYTE                    length,
                                        ULONG                    fn);
EXTERN void drr_dl_short_unitdata_req  (T_DL_SHORT_UNITDATA_REQ * short_unitdata_req);
EXTERN void drr_dl_short_unitdata_ind  (UBYTE                    ch_type,
                                        UBYTE                    error_flag,
                                        UBYTE                  * layer1head,
                                        UBYTE                  * layer3msg,
                                        UBYTE                    length,
                                        ULONG                    fn);
EXTERN void drr_dl_establish_cnf       (UBYTE                    ch_type,
                                        UBYTE                    sapi);
EXTERN void drr_dl_release_ind         (UBYTE                    ch_type,
                                        UBYTE                    sapi,
                                        UBYTE                    cs,
                                        BOOL                     init);
EXTERN void drr_dl_release_cnf         (UBYTE                    ch_type,
                                        UBYTE                    sapi,
                                        BOOL                     init);
EXTERN void drr_error_ind              (UBYTE                    ch_type,
                                        UBYTE                    sapi);

/*
 * Prototypes Slow Associated Control Channel
 */
EXTERN void sacch0_init_dl_data        (void);
EXTERN void sacch0_reset_meas          (void);
EXTERN void sacch0_send_data           (void);
/*
 * Prototypes Slow Dedicated Control Channel (SAPI 0)
 */
EXTERN void dcch0_init_dl_data         (void);
EXTERN void dcch0_delay_sabm           (T_CCH                  * pcch);
EXTERN void dcch0_establish_req        (T_DL_ESTABLISH_REQ     * est_req);
EXTERN void dcch0_resume_req           (T_DL_RESUME_REQ        * resume_req);
EXTERN void dcch0_reconnect_req        (T_DL_RECONNECT_REQ     * reconnect_req);
EXTERN BOOL dcch0_release_req          (T_DL_RELEASE_REQ       * dl_release_req);
EXTERN void dcch0_mdl_release_req      (void);
EXTERN void dcch0_suspend_req          (T_DL_SUSPEND_REQ       * suspend_req);
EXTERN void dcch0_data_req             (T_DL_DATA_REQ          * data_req);
EXTERN int  dcch0_check_disc           (int                      send);

/*
 * Prototypes Slow Dedicated Control Channel (SAPI 3)
 */
EXTERN void dcch3_init_dl_data         (void);
EXTERN void dcch3_enable               (UBYTE                    ch_type);
EXTERN void dcch3_establish_req        (T_DL_ESTABLISH_REQ     * est_req);
EXTERN BOOL dcch3_release_req          (T_DL_RELEASE_REQ       * dl_release_req);
EXTERN void dcch3_mdl_release_req      (void);
EXTERN void dcch3_data_req             (T_DL_DATA_REQ          * data_req);
EXTERN int  dcch3_check_disc           (int                      send);

#if defined(INVOKE_SIGNAL)
EXTERN void sig_init_signal_data       (void);
EXTERN void sig_invoke_drr_dl_establish_ind (UBYTE                    ch_type,
                                        UBYTE                    sapi,
                                        UBYTE                    indication);
EXTERN void sig_invoke_drr_dl_data_cnf (UBYTE                    sapi);
#if 0 /* happens in primitive context only */
EXTERN void sig_invoke_drr_dl_unitdata_ind  (UBYTE                    error_flag,
                                        UBYTE                  * layer1head,
                                        UBYTE                  * layer3msg,
                                        UBYTE                    length,
                                        ULONG                    fn);
#endif  /* 0 */
EXTERN void sig_invoke_drr_dl_short_unitdata_ind (UBYTE                    ch_type,
                                        UBYTE                    error_flag,
                                        UBYTE                  * layer1head,
                                        UBYTE                  * layer3msg,
                                        UBYTE                    length,
                                        ULONG                    fn);
EXTERN void sig_invoke_drr_dl_establish_cnf (UBYTE                    ch_type,
                                        UBYTE                    sapi);
EXTERN void sig_invoke_drr_dl_release_ind   (UBYTE                    ch_type,
                                        UBYTE                    sapi,
                                        UBYTE                    cs,
                                        BOOL                     init);
EXTERN void sig_invoke_drr_dl_release_cnf   (UBYTE                    ch_type,
                                        UBYTE                    sapi,
                                        BOOL                     init);
EXTERN void sig_invoke_drr_error_ind   (UBYTE                    ch_type,
                                        UBYTE                    sapi);
EXTERN void sig_invoke_com_free_pointer (void                  * pointer);
EXTERN void sig_invoke_com_concatenate (T_DL_DATA_IND         ** in_msg,
                                        UBYTE                  * new_data_in);
EXTERN void sig_invoke_com_data_ind    (UBYTE                    ch_type,
                                        UBYTE                    sapi,
                                        ULONG                    fn);
EXTERN void sig_invoke_com_l3trace     (UBYTE                    type,
                                        UBYTE                    ch_type,
                                        UBYTE                  * frame);
#if defined(DL_TRACE_ENABLED) && defined(DL_IMMEDIATE_TRACE)
EXTERN void sig_invoke_com_l2trace     (UBYTE                    trace_type,
                                        UBYTE                    channel,
                                        UBYTE                    ch_type,
                                        T_TIME                   trace_time,
                                        UBYTE                  * data);
#endif  /* DL_TRACE_ENABLED && DL_IMMEDIATE_TRACE */
#if defined(FF_EM_MODE)
EXTERN void sig_invoke_dl_em_write     (UBYTE                    length,
                                        UBYTE                  * data);
EXTERN void sig_invoke_dl_em_first_event_check (void);
#endif  /* FF_EM_MODE */

EXTERN void sig_handle_drr_dl_establish_ind (T_DL_SIGNAL_DATA       * signal);
EXTERN void sig_handle_drr_dl_data_cnf      (T_DL_SIGNAL_DATA       * signal);

EXTERN void sig_handle_drr_dl_short_unitdata_ind (T_DL_SIGNAL_DATA       * signal);
EXTERN void sig_handle_drr_dl_establish_cnf (T_DL_SIGNAL_DATA       * signal);
EXTERN void sig_handle_drr_dl_release_ind   (T_DL_SIGNAL_DATA       * signal);
EXTERN void sig_handle_drr_dl_release_cnf   (T_DL_SIGNAL_DATA       * signal);
EXTERN void sig_handle_drr_error_ind        (T_DL_SIGNAL_DATA       * signal);
EXTERN void sig_handle_com_free_pointer     (T_DL_SIGNAL_DATA       * signal);
EXTERN void sig_handle_com_concatenate      (T_DL_SIGNAL_DATA       * signal);
EXTERN void sig_handle_com_free_pointer     (T_DL_SIGNAL_DATA       * signal);
EXTERN void sig_handle_com_data_ind         (T_DL_SIGNAL_DATA       * signal);
EXTERN void sig_handle_com_l3trace          (T_DL_SIGNAL_DATA       * signal);
#if defined(DL_TRACE_ENABLED) && defined(DL_IMMEDIATE_TRACE)
EXTERN void sig_handle_com_l2trace          (T_DL_SIGNAL_DATA       * signal);
#endif  /* DL_TRACE_ENABLED && DL_IMMEDIATE_TRACE */
#if defined(FF_EM_MODE)
EXTERN UBYTE em_dl_write                    (UBYTE                    length,
                                             UBYTE                  * data);
EXTERN void sig_handle_dl_em_write          (T_DL_DATA_STORE        * dl_data,
                                             T_DL_SIGNAL_DATA       * signal);
EXTERN void dl_em_first_event_check(void);/*for ACI notification of first EM event*/
EXTERN void sig_handle_dl_em_first_event_check (void);
#endif  /* FF_EM_MODE */
#endif  /* INVOKE_SIGNAL */

/*
 * Prototypes Distribute PL
 */

#ifdef _SIMULATION_
#ifdef DL_2TO1
EXTERN void l1test_call_mphc_read_dcch (T_L1TEST_CALL_MPHC_READ_DCCH * ready);
EXTERN void l1test_return_mphc_read_dcch (T_RADIO_FRAME * frame);
EXTERN void l1test_call_mphc_read_sacch (T_L1TEST_CALL_MPHC_READ_SACCH * ready);
EXTERN void l1test_return_mphc_read_sacch (T_RADIO_FRAME* frame);
EXTERN void l1test_call_mphc_dcch_downlink (T_L1TEST_CALL_MPHC_DCCH_DOWNLINK * data_ind);
EXTERN void l1test_return_mphc_dcch_downlink (T_L1TEST_RETURN_MPHC_DCCH_DOWNLINK * resp);
#else /* DL_2TO1 */
EXTERN void dph_ph_ready_to_send (T_PH_READY_TO_SEND * ready);
#endif /* DL_2TO1 */
#endif /* _SIMULATION_ */

/* SACCH Uplink Interface according to L1M_GS011-1 v.1.22, section 23. */
EXTERN T_RADIO_FRAME *dll_read_sacch (U8 chn_mode);

/* SACCH Downlink Interface according to L1M_GS011-1 v.1.22, section 24. */
/* SACCH (SDCCH/FACCH only simulation) Downlink Test Interface  */
EXTERN void dph_ph_data_ind (T_PH_DATA_IND * data_ind);

#if defined(DL_2TO1) || defined(USE_L1M_GS001_1)
/* DCCH Uplink Interface according to L1M_GS001-1, section 25. */
EXTERN T_RADIO_FRAME * dll_read_dcch (U8 chn_mode, U8 channel_type);

/* DCCH Downlink Interface according to L1M_GS001-1, section 26. */
EXTERN void dll_dcch_downlink (U32 * data_ptr, U8 valid_flag, U8 channel_type, U32 fn);

#else  /* DL_2TO1 || USE_L1M_GS001_1 */
/* DCCH Uplink Interface according to L1M_GS001-1 v.1.22, section 25. */
EXTERN T_RADIO_FRAME * dll_read_dcch (U8 chn_mode);

/* DCCH Downlink Interface according to L1M_GS001-1 v.1.22, section 26. */
#if defined(SEND_FN_TO_L2_IN_DCCH) && (SEND_FN_TO_L2_IN_DCCH == 1)
  EXTERN void dll_dcch_downlink (U32 * data_ptr, U8 valid_flag, U32 fn);
#else /* SEND_FN_TO_L2_IN_DCCH == 1 */
  EXTERN void dll_dcch_downlink (U32 * data_ptr, U8 valid_flag);
#endif /* SEND_FN_TO_L2_IN_DCCH == 1 */
#endif  /* DL_2TO1 || USE_L1M_GS001_1 */

/* SACCH/SDCCH/FACCH Uplink Interface  */
EXTERN T_RADIO_FRAME *dl1_uplink_ind   (UBYTE                    channel_type,
                                        UBYTE                    no_signalling_flag);

/*
* Prototypes Downlink and Uplink Functions (DL_STATE.c)
*/
EXTERN int dl_downlink                 (UBYTE                    error_flag,
                                        UBYTE                    channel_type,
                                        UBYTE                  * frame,
                                        ULONG                    fn);
EXTERN void set_channel_state          (UBYTE                    channel,
                                        UBYTE                    state);
EXTERN T_RADIO_FRAME* dl_uplink        (UBYTE                    channel,
                                        UBYTE                    sapi,
                                        UBYTE                    no_signalling_mode,
                                        BOOL                     recursiv);

EXTERN int uplink_awaiting_release     (UBYTE                    channel,
                                        UBYTE                    sapi);
/*
* Prototypes Common Functions
*/
EXTERN void com_free_pointer           (void                   * pointer);
EXTERN void com_free_queue_buffer      (T_QUEUE                * queue,
                                        USHORT                   index);
EXTERN void com_clear_queue            (UBYTE                    sapi);
EXTERN void com_restore_queue          (UBYTE                    sapi,
                                        T_DL_DATA_REQ          * est_req);
EXTERN void com_recover_queue          (UBYTE                    sapi);
EXTERN void com_store_queue            (UBYTE                    sapi,
                                        T_DL_DATA_REQ          * data_req);
EXTERN void com_read_queue             (UBYTE                    ch_type,
                                        UBYTE                    sapi,
                                        UBYTE                  * m_bit);
EXTERN BOOL com_queue_awaiting_transmission   (UBYTE                    sapi);
EXTERN void com_leave_dedicated        (UBYTE                    ch_type);
EXTERN void possible_reset_dcch0_ch_type    (void);
EXTERN void com_concatenate            (T_DL_DATA_IND         ** in_msg,
                                        UBYTE                  * new_data_in);
EXTERN UBYTE com_check_nr              (UBYTE                    va,
                                        UBYTE                    vs,
                                        UBYTE                    nr);
EXTERN void com_prepare_DISC           (UBYTE                    channel,
                                        UBYTE                    sapi);
EXTERN void com_build_UA_response      (UBYTE                    ch_type,
                                        UBYTE                    sapi,
                                        UBYTE                    f_bit);
EXTERN void com_build_RR_response      (UBYTE                    ch_type,
                                        UBYTE                    sapi,
                                        UBYTE                    nr,
                                        UBYTE                    f_bit);
EXTERN void com_build_REJ_response     (UBYTE                    ch_type,
                                        UBYTE                    sapi,
                                        UBYTE                    nr,
                                        UBYTE                    f_bit);
EXTERN void com_build_DISC_command     (UBYTE                    ch_type,
                                        UBYTE                    sapi,
                                        UBYTE                    p_bit);
EXTERN void com_build_SABM             (UBYTE                    ch_type,
                                        UBYTE                    sapi,
                                        BOOL                     contention_resultion);
EXTERN void com_build_DM_response      (UBYTE                    ch_type,
                                        UBYTE                    sapi,
                                        UBYTE                    f_bit);
EXTERN void com_build_I_command        (UBYTE                    ch_type,
                                        UBYTE                    sapi,
                                        UBYTE                    ns,
                                        UBYTE                    nr,
                                        UBYTE                    p_bit,
                                        UBYTE                    m_bit,
                                        T_QUEUE                * queue);
EXTERN void com_build_UI_command       (UBYTE                    ch_type,
                                        UBYTE                    sapi,
                                        const T_FRAME          * buffer);
EXTERN void com_build_UI_Bter          (UBYTE                    ch_type);
EXTERN UBYTE com_compare_L3_msg        (T_DL_DATA_REQ          * data_ind1,
                                        UBYTE                  * data_ind2);
GLOBAL void com_data_ind               (UBYTE                    ch_type,
                                        UBYTE                    sapi,
                                        ULONG                    fn);
EXTERN void com_l2trace                (UBYTE                    trace_type,
                                        UBYTE                    channel,
                                        UBYTE                    ch_type,
                                        T_TIME                   event_time,
                                        UBYTE                  * data);
EXTERN void com_l3trace                (UBYTE                    type,
                                        UBYTE                    ch_type,
                                        UBYTE                  * frame);
EXTERN void com_print_l3trace          (UBYTE                    type,
                                        UBYTE                    ch_type,
                                        UBYTE                    sapi,
                                        UBYTE                  * l3msg);
EXTERN void com_init_data              (void);


EXTERN CHAR* dl_version                (void);
EXTERN T_DL_DATA_STORE *dl_get_data    (void);

#if defined (DL_TRACE_ENABLED)
EXTERN void dl_trace                   (UCHAR                    trace_type,
                                        UCHAR                    channel,
                                        UCHAR                    ch_type,
                                        UCHAR                  * data);
#if defined(DL_IMMEDIATE_TRACE)

EXTERN void dl_fast_trace              (UBYTE                    trace_type,
                                        UBYTE                    channel,
                                        UBYTE                    ch_type,
                                        T_TIME                   trace_time,
                                        ULONG                    trace_mask,
                                        UBYTE                  * data);
#else  /* DL_IMMEDIATE_TRACE */
EXTERN void dl_trace_init              (void);
EXTERN void dl_trace_exit              (void);
EXTERN void dl_trace_read              (void);
EXTERN void dl_trace_read_all          (void);
EXTERN void dl_trace_clear             (void);
#endif  /* DL_IMMEDIATE_TRACE */
#endif  /* DL_TRACE_ENABLED */

/*
 * Prototypes Customer Specific Functions
 */


EXTERN USHORT dl_handle;

#define ENTITY_DATA                dl_data


#if defined (DL_PEI_C)
GLOBAL T_DL_DATA_STORE dl_data_base;
#else
EXTERN T_DL_DATA_STORE dl_data_base;
#endif  /* DL_PEI_C */

#define GET_INSTANCE_DATA     register T_DL_DATA_STORE *dl_data= &dl_data_base



/*
 * If all entities are linked into one module this definitions
 * prefixes the global data with the enity name
 */
#ifdef TI_PS_HCOMM_CHANGE
#define hCommDL _hCommDL
#else /* for hCommHandles backward compatibility */
#ifdef OPTION_MULTITHREAD
  #define hCommDL       _ENTITY_PREFIXED(hCommDL)
  #define hCommRR       _ENTITY_PREFIXED(hCommRR)
  #define hCommPL       _ENTITY_PREFIXED(hCommPL)
#endif

EXTERN T_HANDLE  hCommDL;                /* Self  Communication        */
EXTERN T_HANDLE  hCommRR;                /* RR  Communication        */
EXTERN T_HANDLE  hCommPL;                /* PL  Communication        */
#endif

#define SYST_TRACE(a) vsi_o_ttrace(0, 0xFFFF,a)
#define SYST           0, 0xffff
#ifdef TI_PS_HCOMM_CHANGE
#define DLTRC          _hCommDL, 0x0200
#else /* for hCommHandles backward compatibility */
#define DLTRC          hCommDL, 0x0200
#endif
#define SYST_TRACE_P(a) vsi_o_ttrace a


#if defined(DL_TRACE_ENABLED)
#if defined(DL_IMMEDIATE_TRACE)
#define DL_OFFLINE_TRACE(type, ch, ch_type, data)  \
        com_l2trace (type, (U8) (ch),(U8)(ch_type), 0, (UBYTE *)(data))
#else /* DL_IMMEDIATE_TRACE */
#define DL_OFFLINE_TRACE(type, ch, ch_type, data)  \
        dl_trace ((UCHAR)(type), (UCHAR)(ch), (UCHAR)(ch_type), (UCHAR *)(data))
#endif /* DL_IMMEDIATE_TRACE */
#else
#define DL_OFFLINE_TRACE(type, ch, ch_type, data)  /* do nothing */
#endif  /* DL_TRACE_ENABLED */

/*
 * FreeCalypso change: this __FILE10__ thing is gross, so let's try
 * removing it.  We run gcc from the directory where the source is,
 * pathname=filename, so __FILE__ should be short enough.
 */
#if 0 && !defined(_FILE10_)
#define __FILE10__  (__FILE__+strlen(__FILE__)-11)
#endif  /* !_FILE10_ */

#if defined(DL_TRACE_ENABLED) || defined(FF_EM_MODE)
EXTERN void com_semaphore_err (void);
GLOBAL int com_enter_critical_section (T_HANDLE sem);
GLOBAL int com_leave_critical_section (T_HANDLE sem);
#define  ENTER_CRITICAL_SECTION(sem)  if (com_enter_critical_section(sem))return
#define  LEAVE_CRITICAL_SECTION(sem)  if (com_leave_critical_section(sem))return
#if !defined(DL_IMMEDIATE_TRACE)
GLOBAL int com_semaphore_state (T_HANDLE sem);
#define  TEST_SEMAPHORE(sem)          if (com_semaphore_state(sem)) return
#endif /* !DL_IMMEDIATE_TRACE */
#endif  /* DL_TRACE_ENABLED || FF_EM_MODE */


#if defined (DL_TRACE_ENABLED) && defined(DL_TRACE_PFREE)
#define MY_PFREE(p)   p = my_pfree (p, __LINE__, __FILE__)
void* my_pfree(void *pointer, int line, char *file);
#else
#define MY_PFREE(p)   PFREE(p)
#endif  /* DL_TRACE_ENABLED */


#if defined(_SIMULATION_)
#define COM_FREE_QUEUE_BUFFER(q,i) \
  {TRACE_EVENT_WIN_P4 ("FREE_QUEUE_BUFFER:q=%p i=%u (%s#%u)",q,i,__FILE10__, __LINE__);\
   com_free_queue_buffer(q, i);}
#define COM_FREE_POINTER(p) \
  {TRACE_EVENT_WIN_P4 ("call com_free_pointer(,%s=%08x) (%s#%u)", #p, p, __FILE10__, __LINE__);\
    com_free_pointer ( p);}
#else /* _SIMULATION_ */
#define COM_FREE_QUEUE_BUFFER(q,i) com_free_queue_buffer(q,i)
#define COM_FREE_POINTER(p) com_free_pointer ( p)
#endif /* _SIMULATION_ */


#if 0
#define ARRAY_TRACE
#if defined(ARRAY_TRACE)
EXTERN void  rr_array_trace (UBYTE*array, int size, char *titel);
#define TRACE_ARRAY(array, size, titel) rr_array_trace(array, size, titel)
#else  /* ARRAY_TRACE */
#define TRACE_ARRAY(array, size, titel)
#endif  /* ARRAY_TRACE */
#endif /* 0 */

#endif /* !__DL_H__ */
