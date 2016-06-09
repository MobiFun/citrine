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
|  Purpose :  Definitions for the Protocol Stack Entity
|             Universal Asynchronous Receiver Transmitter (UART).
+-----------------------------------------------------------------------------
*/

#ifndef UART_H
#define UART_H

/*==== MACROS ======================================================*/

/*
 * defines the user of the vsi interface
 */
#define VSI_CALLER            UART_handle,
#define VSI_CALLER_SINGLE     UART_handle

/*
 * VERSION
 *
 * Description :  The constants define the type and the value
 *                of a version identification. The version
 *                is part of the monitor struct.
 */
#define VERSION_UART    "UART 1.1"

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
 * This is just a TEMPORARY define until the issues with OPTION_MULTITHREAD
 * are settled. This define SHOULD be contained in GSM.H.
 */
#undef _ENTITY_PREFIXED
#define _ENTITY_PREFIXED(N) uart_##N

/*
 * Communication handles (see also UART_PEI.C)
 */
#define hCommMMI                uart_hCommMMI
#define hCommUART               uart_hCommUART

/*
 * Macros for error tracing
 */

#ifndef TRACE_ERROR_P1
#define TRACE_ERROR_P1(f,a1)                  vsi_o_error_ttrace(f,a1);
#endif

#ifndef TRACE_ERROR_P2
#define TRACE_ERROR_P2(f,a1,a2)               vsi_o_error_ttrace(f,a1,a2);
#endif

#ifndef TRACE_ERROR_P3
#define TRACE_ERROR_P3(f,a1,a2,a3)            vsi_o_error_ttrace(f,a1,a2,a3);
#endif

#ifndef TRACE_ERROR_P4
#define TRACE_ERROR_P4(f,a1,a2,a3,a4)          vsi_o_error_ttrace(f,a1,a2,a3,a4);
#endif

/*==== CONSTANTS ======================================================*/

#if defined (DTILIB)
/*
 * dtilib interface
 */

#define UART_DTI_UP_INTERFACE  0
#define UART_UPLINK_QUEUE_SIZE 0
#endif  /* DTILIB */
/*
 * Scaling
 */
#ifdef FF_MULTI_PORT
#define UART_INSTANCES     FF_MULTI_PORT
#else /* FF_MULTI_PORT */
#ifdef FF_TWO_UART_PORTS
#define UART_INSTANCES                 2
#else /* FF_TWO_UART_PORTS */
#define UART_INSTANCES                 1
#endif /* FF_TWO_UART_PORTS */
#endif /* FF_MULTI_PORT */

#define UART_MAX_NUMBER_OF_CHANNELS    6

/*
 * Bitoffset for encoding/decoding
 */
#define ENCODE_OFFSET                 0

/*
 * Service definitions. Used to access service data with GET/SET_STATE.
 *
 * Services with multiple incarnation have to be defined as xxx->
 * Services with only one incarnation have to be defined as xxx.
 *
 */
#define UART_SERVICE_KER        ker.
#define UART_SERVICE_RX         rx.
#define UART_SERVICE_TX         tx.
#define UART_SERVICE_DTX        dtx->
#define UART_SERVICE_DRX        drx->
#define UART_SERVICE_RT         rt.

/*
 * Service name definitions for trace purposes.
 */
#ifndef NTRACE

#define SERVICE_NAME_UART_SERVICE_KER     "KER"
#define SERVICE_NAME_UART_SERVICE_RX      "RX"
#define SERVICE_NAME_UART_SERVICE_TX      "TX"
#define SERVICE_NAME_UART_SERVICE_DRX     "DRX"
#define SERVICE_NAME_UART_SERVICE_DTX     "DTX"
#define SERVICE_NAME_UART_SERVICE_RT      "RT"

#endif /* !NTRACE */


/*
 * State definitions for each service.
 */

#define KER_DEAD                                                1
#define KER_READY                                               2
#define KER_MUX_ESTABLISH                                       3
#define KER_MUX                                                 4
#define KER_MUX_DLC_CLOSING                                     5
#define KER_MUX_CLOSING                                         6
#define KER_MUX_CLOSED                                          7

#define RX_DEAD                                                 8
#ifdef DTILIB
/*
 * we do not use the one from DTILIB here
 */
#undef RX_READY
#endif
#define RX_READY                                                9
#define RX_MUX                                                 10

#define TX_DEAD                                                11
#ifdef DTILIB
/*
 * we do not use the one from DTILIB here
 */
#undef TX_READY
#endif
#define TX_READY                                               12
#define TX_READY_FLUSHING                                      13
#define TX_MUX                                                 14
#define TX_MUX_FLUSHING                                        15

#define DRX_DEAD                                               16
#define DRX_READY                                              17
#define DRX_NOT_READY                                          18
#define DRX_FLUSHING                                           19

#define DTX_DEAD                                               20
#define DTX_READY                                              21
#define DTX_NOT_READY                                          22

#define RT_STATE                                               23

/*
 * DLC Connection states
 */
#define UART_CONNECTION_DEAD                                   24
#define UART_CONNECTION_SABM_RCVD                              25
#define UART_CONNECTION_DISC_SENT                              26
#define UART_CONNECTION_OPEN                                   27

/*
 * states of TX ISR state machine
 */
#define UART_TX_BEGIN                                          28
#define UART_TX_ADDRESS                                        29
#define UART_TX_CONTROL                                        30
#define UART_TX_INFORMATION                                    31
#define UART_TX_FCS                                            32
#define UART_TX_END                                            33

/*
 * states of TX sending indicator
 */
#define UART_TX_SENDING                                        34
#define UART_TX_NOT_SENDING                                    35

/*
 * states of RX receiving indicator
 */
#define UART_RX_RECEIVING                                      36
#define UART_RX_NOT_RECEIVING                                  37

/*
 * states of RX ISR state machine
 */
#define UART_RX_ERROR                                          38
#define UART_RX_BEGIN                                          39
#define UART_RX_ADDRESS                                        40
#define UART_RX_CONTROL                                        41
#define UART_RX_INFORMATION                                    42
#define UART_RX_FCS                                            43
#define UART_RX_END                                            44

/*
 * DLC process states of reception
 */
#define UART_RX_PROCESS_STOP                                   45
#define UART_RX_PROCESS_READY                                  46
#define UART_RX_PROCESS_COMPLETE                               47

/*
 * definition of flush states for service KER
 */
#define UART_KER_NOT_FLUSHING                                  48
#define UART_KER_DRX_FLUSH                                     49
#define UART_KER_TX_FLUSH                                      50

/*
 * special channel instance values
 */
#define UART_CONTROL_INSTANCE         UART_MAX_NUMBER_OF_CHANNELS
#define UART_EMPTY_INSTANCE                                   255

/*
 * priority value
 */
#define UART_PRIORITY_CONTROL                                   0

/*
 * HDLC values
 */
#define UART_HDLC_FLAG                                       0x7e
#define UART_HDLC_ESCAPE                                     0x7d
#define UART_SABM_FRAME                                      0x3f
#define UART_UA_FRAME                                        0x73
#define UART_DM_CONTROL_FRAME                                0x1f
#define UART_DM_DATA_FRAME                                   0x0f
#define UART_DISC_FRAME                                      0x53
#define UART_UIH_CONTROL_FRAME                               0xff
#define UART_UIH_DATA_FRAME                                  0xef

/*
 * UART DLC channel values
 */
#define UART_DLCI_CONTROL                                       0
#define UART_DLCI_INVALID                                     255
#define UART_DLCI_POS                                           2

/*
 * mask values for forward parameter
 */
#define UART_FORWARD_CLD                               0x00000001
#define UART_FORWARD_FCON                              0x00000002
#define UART_FORWARD_FCOFF                             0x00000004
#define UART_FORWARD_MSC                               0x00000008
#define UART_FORWARD_SABM                              0x00000010
#define UART_FORWARD_DLC_RELEASE                       0x00000020
#define UART_FORWARD_PSC                               0x00000040
#define UART_FORWARD_RESPONSE                          0x00000080

/*
 * message type values (for UIH command frames)
 */
#define UART_MSG_TYPE_PSC_C                                  0x43
#define UART_MSG_TYPE_PSC_R                                  0x41
#define UART_MSG_TYPE_CLD_C                                  0xc3
#define UART_MSG_TYPE_CLD_R                                  0xc1
#define UART_MSG_TYPE_FCON_C                                 0xa3
#define UART_MSG_TYPE_FCON_R                                 0xa1
#define UART_MSG_TYPE_FCOFF_C                                0x63
#define UART_MSG_TYPE_FCOFF_R                                0x61
#define UART_MSG_TYPE_MSC_C                                  0xe3
#define UART_MSG_TYPE_MSC_R                                  0xe1
#define UART_MSG_TYPE_NSC_R                                  0x11
#define UART_MSG_TYPE_SNC_C                                  0xd3
#define UART_MSG_TYPE_SNC_R                                  0xd1

/*
 * message length field values
 */
#define UART_MSG_LENGTH_POS                                     1

/*
 * definitions for service negotiation
 */
#define UART_NO_SERVICE                                      0x00
#define UART_NO_VOICE_CODEC                                  0x00

/*
 * definitions for modem status commands
 */
#define UART_NO_BREAK_SIGNAL                                 0x01

/*
 * FCS calculation
 */
#define UART_INITFCS                                         0xff
#define UART_GOODFCS                                         0xcf

/*
 * offset addresses for frame handling in service KER
 * (in bytes after flag)
 */
#define UART_OFFSET_ADDRESS                                   0
#define UART_OFFSET_CONTROL                                   1
#define UART_OFFSET_INFO                                      2

/*
 * common EA and CR bit
 */
#define UART_EA                                            0x01
#define UART_CR                                            0x02
/*
 * modem status command (MSC) masks and positions
 */
#define UART_MSC_BRKLEN_POS                                   4
#define UART_MSC_DV_MASK                                   0x80
#define UART_MSC_IC_MASK                                   0x40
#define UART_MSC_RTR_MASK                                  0x08
#define UART_MSC_RTC_MASK                                  0x04
#define UART_MSC_FC_MASK                                   0x02
#define UART_MSC_BRK_MASK                                  0x02
#define UART_MSC_BRKLEN_MASK                               0xf0

/*
 * !!! for vsi_t_sleep may be changed???
 */
#define ONE_FRAME                                             0
/*
 * position and mask definitions
 */
#define BRK_LEN_POS                                      BRKLEN
#define BRK_LEN_MASK                                   0x1fe00L
#define SA_MASK                                  ( 1UL <<  SA )
#define SB_MASK                                  ( 1UL <<  SB )
#define X_MASK                                   ( 1UL <<   X )
#define BRK_MASK                                 ( 1UL << BRK )
#define ESC_MASK                                 ( 1UL << ESC )
#define RI_MASK                                  ( 1UL <<  RI )
#define DCD_MASK                                 ( 1UL << DCD )
/*
 * UART intern position and mask definitions
 */
#define UART_CTS_POS                                          0
#define UART_RTS_POS                                          1
#define UART_DSR_POS                                          2
#define UART_DTR_POS                                          3
#define UART_DCD_POS                                          4
#define UART_BRK_RX_POS                                       5
#define UART_BRK_TX_POS                                       6
#define UART_ESC_RX_POS                                       7
#define UART_ESC_TX_POS                                       8
#define UART_BRKLEN_RX_POS                                    9
#define UART_BRKLEN_TX_POS                                   17
#define UART_FC_RX_POS                                       25
#define UART_FC_TX_POS                                       26
#define UART_RI_POS                                          27
#define UART_SA_TX_POS                                       28
#define UART_SB_TX_POS                             UART_DCD_POS
#define UART_X_TX_POS                                        29
#define UART_SA_RX_POS                             UART_DTR_POS
#define UART_SB_RX_POS                                       30
#define UART_X_RX_POS                              UART_RTS_POS
#define UART_CTS_MASK                     (1UL << UART_CTS_POS)
#define UART_RTS_MASK                     (1UL << UART_RTS_POS)
#define UART_DSR_MASK                     (1UL << UART_DSR_POS)
#define UART_DTR_MASK                     (1UL << UART_DTR_POS)
#define UART_DCD_MASK                     (1UL << UART_DCD_POS)
#define UART_BRK_RX_MASK               (1UL << UART_BRK_RX_POS)
#define UART_BRK_TX_MASK               (1UL << UART_BRK_TX_POS)
#define UART_ESC_RX_MASK               (1UL << UART_ESC_RX_POS)
#define UART_ESC_TX_MASK               (1UL << UART_ESC_TX_POS)
#define UART_BRKLEN_RX_MASK       (255UL << UART_BRKLEN_RX_POS)
#define UART_BRKLEN_TX_MASK       (255UL << UART_BRKLEN_TX_POS)
#define UART_FC_RX_MASK                 (1UL << UART_FC_RX_POS)
#define UART_FC_TX_MASK                 (1UL << UART_FC_TX_POS)
#define UART_RI_MASK                       (1UL << UART_RI_POS)
#define UART_SA_TX_MASK                 (1UL << UART_SA_TX_POS)
#define UART_SB_TX_MASK                 (1UL << UART_SB_TX_POS)
#define UART_X_TX_MASK                   (1UL << UART_X_TX_POS)
#define UART_SA_RX_MASK                 (1UL << UART_SA_RX_POS)
#define UART_SB_RX_MASK                 (1UL << UART_SB_RX_POS)
#define UART_X_RX_MASK                   (1UL << UART_X_RX_POS)

/*
 * values for received primitive bitfield
 */
#define UART_PARAMETERS_REQ_MASK                     0x00000001
#define UART_DTI_REQ_MASK                            0x00000004
#define UART_DISABLE_REQ_MASK                        0x00000008
#define UART_RING_REQ_MASK                           0x00000010
#define UART_DCD_REQ_MASK                            0x00000020
#define UART_MUX_START_REQ_MASK                      0x00000080
#define UART_MUX_DLC_RELEASE_REQ_MASK                0x00000100
#define UART_MUX_SLEEP_REQ_MASK                      0x00000200
#define UART_MUX_WAKEUP_REQ_MASK                     0x00000400
#define UART_MUX_CLOSE_REQ_MASK                      0x00000800
#define UART_DTI_DATA_REQ_MASK                       0x00001000

/*
 * value for N1 in ready mode
 */
#define UART_N1_READY_MODE                                   10
/*
 * buffer size of RX buffer in multiplexer mode
 */
#define UART_RX_MUX_BUFFER_SIZE          (100 - sizeof(T_desc2))

#ifdef WIN32
#define HISR_STACK_SIZE         512
#endif
#ifdef _SIMULATION_
#define UART_TX_SIM_BUFFER_SIZE                             136
#endif /* _SIMULATION_ */

/*
 * Timer handle definitions.
 */

#define UART_RT_INDEX_T1           0
#define UART_RT_INDEX_T2           1
#define UART_RT_INDEX_T3           2
#define UART_RT_INDEX_TESD         3  /* Escape Sequence Detection */
#define UART_TIMER_PER_INSTANCE    4

#define UART_TIMER_MAX             (UART_TIMER_PER_INSTANCE * UART_INSTANCES)

/* Dynamic Configuration Numbers */
#define RESET                0
#ifdef DEBUG_COUNTERS
#define DUMP_COUNTERS        6
#endif /* DEBUG_COUNTERS */

/*
 * definition of constants for DTX dynamic buffer management:
 */
#define UART_DTX_MAX_BUF_SIZE          1000     /* max. 1000 bytes */
#define UART_DTX_MAX_SIZE_MULTIPLIER \
                               ((USHORT)(UART_DTX_MAX_BUF_SIZE / uart_data->n1))

/*==== GLOBAL VARIABLES ======================================================*/

/*==== TYPES ======================================================*/

/*
 * UART global typedefs
 */
typedef UBYTE   T_BIT;

/*
 * definition of sending state for service DRX
 */
typedef enum
{
  UART_DRX_NOT_SENDING,
  UART_DRX_SENDING,
  UART_DRX_INVALID
} T_DRX_SENDING_STATE;

/*
 * definition of receiving states for service DTX
 */
typedef enum
{
  UART_DTX_NOT_RECEIVING,
  UART_DTX_RECEIVING,
  UART_DTX_INVALID
} T_DTX_RECEIVING_STATE;

/*
 * definition of sending/receiving state for service KER
 */
typedef enum
{
  UART_KER_NOT_SENDING,
  UART_KER_SENDING,
  UART_KER_NOT_RECEIVING,
  UART_KER_RECEIVING,
  UART_KER_INVALID
} T_KER_TRANSMISSION_STATE;

/*
 * definition of timer states for service RT
 */
typedef enum
{
  UART_RT_STARTED,
  UART_RT_STOPPED
} T_TIMER_STATE;

/*
 * definition of data flow states
 */
typedef enum
{
  UART_FLOW_DISABLED,
  UART_FLOW_ENABLED
} T_DATA_FLOW_STATE;

/*
 * definition of escape sequence detection (ESD) states
 */
typedef enum
{
  UART_ESD_NULL,          /* ESD not in progress */
  UART_ESD_CHAR_1,        /* first ESD character has been received ('+') */
  UART_ESD_CHAR_2,        /* second ESD character... */
  /* third ESD character has been received (fourth would be invalid) */
  UART_ESD_CHAR_3,
  /* escape sequence detected, but charcters are not removed from data stream */
  UART_ESD_DETECTED
} T_ESD_STATE;

/*
 * data structure for each service
 */

typedef struct /* T_KER_DATA */
{
  UBYTE                       state;
  /*
   * Required for state traces.
   */
#ifndef NTRACE
  char                        *name;
  char                        *state_name;
#endif /* !NTRACE */

  /*
   * service specific: UART parameters, normal mode
   */
#ifdef FF_MULTI_PORT
  T_GSI_DCB                   act_dcb;          /* device control block */
#else /* FF_MULTI_PORT */
  T_baudrate          act_br;     /* baud rate */
  T_bitsPerCharacter  act_bpc;    /* bit per character */
  T_stopBits          act_sb;     /* stop bits */
  T_parity            act_par;    /* parity */
  UBYTE               act_xon;    /* XOn character */
  UBYTE               act_xoff;   /* XOff character */
  T_flowCtrlMode      act_fc_rx;  /* RX flow control mode */
  T_flowCtrlMode      act_fc_tx;  /* TX flow control mode */
  UBYTE               act_ec;     /* escape character */
  USHORT              act_gp;     /* guard period */
#endif /* FF_MULTI_PORT */
  /*
   * service specific: multiplexer mode
   */
  UBYTE                       n2;               /* maximum number of retransmissions */
  /*
   * service specific: receive part, multiplexer mode
   */
  T_desc2*                    rx_data_desc;     /* data received from peer  */
  /* state of internal data receiving */
  T_KER_TRANSMISSION_STATE    receiving_state;
  /* data to be sent to peer */
  T_desc2*                    tx_data_desc;
  /* data waiting for access to tx_data_desc */
  T_desc2*                    tx_data_waiting;
  /* KER currently sending via UART? */
  T_KER_TRANSMISSION_STATE    sending_state;
  /* data flow enabled/disabled */
  T_DATA_FLOW_STATE           data_flow_tx;
  /* number of running T1 timers */
  UBYTE                       nr_t1;
  /* number of running T2 timers */
  UBYTE                       nr_t2;
  /* bitfield of received UART primitives */
  ULONG                       received_prim;
  /* state of flushing */
  UBYTE                       flush_state;
} T_KER_DATA;


typedef struct /* T_RX_DATA */
{
  UBYTE                       state;
  /*
   * Required for state traces.
   */
#ifndef NTRACE
  char                        *name;
  char                        *state_name;
#endif /* !NTRACE */


#ifndef FF_MULTI_PORT
#ifdef WIN32
  NU_HISR                     rx_HISR;
#endif /* WIN32 */
#endif /* !FF_MULTI_PORT */

  /*
   * ab hier service spezif.
   */
#ifdef FF_MULTI_PORT
  T_desc2*      mux_buffer;       /* buffer in multiplexer mode */
  USHORT        mux_pos;          /* analyse position in multiplexer mode */
  BOOL          waiting_for_data; /* waiting for indication of UART driver */
#else /* FF_MULTI_PORT */
  T_BIT         cldFromIrq;       /* indicator whether it is an IRQ call */
  T_reInstMode  *reInstall;       /* reinstallation mode */
  UBYTE         nsource;          /* number of readable fragments */
  UBYTE         *source[2];       /* addresses of readable fragments */
  USHORT        *size;            /* sizes of readable fragments */
  BOOL          read_permission;  /* permission of ISR to read out data */
#endif /* FF_MULTI_PORT */

  ULONG         lines;            /* status of V.24 lines */
  ULONG         prev_lines;       /* old status of V.24 lines */
  UBYTE         dlc_instance;     /* instance of DLC table */
  BOOL          escape;           /* control escape character received */
  UBYTE         receive_state;    /* state of reception */
  UBYTE         analyze_state;    /* state of frame analysis */
  UBYTE         fcs;              /* calculated fcs */
  UBYTE         address_field;    /* frame address field */
  USHORT        stored_len;       /* length of stored information field */
#ifdef _SIMULATION_
  T_desc2*      sim_buffer;       /* to simulate driver reception buffer */
  USHORT        sim_pos;          /* position to read next byte */
  BOOL          sim_send;         /* send DTI_GETDATA_REQ */
#endif /* _SIMULATION_ */
} T_RX_DATA;


typedef struct /* T_TX_DATA */
{
  UBYTE                       state;
  /*
   * Required for state traces.
   */
#ifndef NTRACE
  char                        *name;
  char                        *state_name;
#endif /* !NTRACE */

#ifndef FF_MULTI_PORT
#ifdef WIN32
  NU_HISR       tx_HISR;
#endif
#endif /* !FF_MULTI_PORT */

  /*
   * ab hier service spezif.
   */
#ifdef FF_MULTI_PORT
  T_desc2*      mux_buffer;         /* buffer in multiplexer mode */
  USHORT        mux_pos;            /* transmit position in multiplexer mode */
  BOOL          waiting_for_space;  /* waiting for indication of UART driver */
#ifdef _SIMULATION_
  T_desc2*      sim_buffer;         /* to simulate driver transsion buffer */
  USHORT        sim_sent;           /* already sent bytes */
  BOOL          sim_waiting;        /* waiting for READY */
#endif /* _SIMULATION_ */
#else /* FF_MULTI_PORT */
  T_BIT         cldFromIrq;         /* indicator whether it is an IRQ call */
  T_reInstMode  *reInstall;         /* reinstallation mode */
  UBYTE         ndest;              /* number of writeable fragments */
  UBYTE         *dest[2];           /* addresses of writeable fragments */
  USHORT        *size;              /* sizes of writeable fragments */
#endif /* FF_MULTI_PORT */

  ULONG         lines;              /* status of V.24 lines */
  UBYTE         dlc_instance;       /* instance of global channel table */
  UBYTE         p_zero;             /* relative zero for priority counter */
  UBYTE         send_state;         /* indicator whether TX is sending */
} T_TX_DATA;


typedef struct /* T_DTX_DATA */
{
  UBYTE                       state;
  /*
   * Required for state traces.
   */
#ifndef NTRACE
  char                        *name;
  char                        *state_name;
#endif /* !NTRACE */

  /*
   * service specific:
   */
  /* stores generic data descriptor to send */
  T_desc2               *to_send_data;
  /* current size of to send data descriptor */
  USHORT                cur_desc_size;
  /* next to write position in current descriptor */
  USHORT                write_pos;
  /* multiplier to dynamically adapt cur_desc_size; min. is 3 */
  USHORT                size_multiplier;
  /* flow control state (X bit) */
  UBYTE                 st_flow;
  /* line state SA */
  UBYTE                 st_line_sa;
  /* line state SB */
  UBYTE                 st_line_sb;
  /* escape state */
  UBYTE                 st_break_len;
  /* indicator whether line states changed */
  BOOL                  lines_changed;
  /* state of internal data receiving */
  T_DTX_RECEIVING_STATE receiving_state;
  /* indicator whether data flow is enabled or disabled */
  T_DATA_FLOW_STATE     data_flow;
#if defined DTILIB
/* state of DTILIB connection */
  UBYTE                 dti_dtx_state;
#else
  /* vsi handle for uplink of this entity */
  T_HANDLE              hComm_DTX_UPLINK;
#endif
  /* dlc instance wich belongs to this DTX instance */
  UBYTE                 dlc_instance;
  /* enable/disable escape sequence detection */
  UBYTE                 detect_escape;
  /* escape sequence detection state */
  T_ESD_STATE           esd_state;
  /* escape sequence detection guard period time */
  T_TIME                esd_guard_time;
  /* escape sequence detection scan position */
  USHORT                esd_pos;
#ifdef _SIMULATION_
  char                  *esd_state_name;
#endif
} T_DTX_DATA;


typedef struct /* T_DRX_DATA */
{
  UBYTE                       state;
  /*
   * Required for state traces.
   */
#ifndef NTRACE
  char                        *name;
  char                        *state_name;
#endif /* !NTRACE */

  /*
   * service specific:
   */
  /* DRX currently sending via UART? */
  T_DRX_SENDING_STATE sending_state;
  /* data flow enabled/disabled */
  T_DATA_FLOW_STATE   data_flow;

  /* data descriptor received via DTI */
  T_desc2*            received_data;
  /* already processed data */
  USHORT              read_pos;
#if defined DTILIB
  /* state of DTILIB connection */
  UBYTE               dti_drx_state;
#else
  /* vsi handle for uplink of this entity */
  T_HANDLE            hComm_DRX_UPLINK;
#endif
  /* dlc instance wich belongs to this DRX instance */
  UBYTE               dlc_instance;
} T_DRX_DATA;


typedef struct /* T_RT_DATA */
{
  UBYTE                       state;
  /*
   * Required for state traces.
   */
#ifndef NTRACE
  char                        *name;
  char                        *state_name;
#endif /* !NTRACE */

  /*
   * service specific:
   */
  T_TIMER_STATE               state_t1;         /* state of timer T1 */
  T_TIMER_STATE               state_t2;         /* state of timer T2 */
  T_TIMER_STATE               state_t3;         /* state of timer T3 */
  T_TIMER_STATE               state_tesd;       /* state of timer TESD */
  T_TIME                      t1;               /* start value of timer T1 */
  T_TIME                      t2;               /* start value of timer T2 */
  T_TIME                      t3;               /* start value of timer T3 */
  T_TIME                      tesd;             /* start value of timer TESD */

} T_RT_DATA;


/*
 * global channel structure
 */
typedef struct /* T_DLC */
{
  T_DRX_DATA  *drx;         /* instance of appropriate DRX service */
  T_DTX_DATA  *dtx;         /* instance of appropriate DTX service */

  UBYTE       dlci;         /* Data Link Connection Identifier */
  UBYTE       priority;     /* priority of dlc */
#if defined DTILIB
  UBYTE       dti_state;    /* state of DTILIB connection */
#else
  T_HANDLE    hCommUPLINK;  /* VSI connection handle */
#endif
  UBYTE       connection_state; /* state of DLC connection */

  /*
   * information for service TX
   */
  T_desc2* transmit_data;   /* data to transmit to the TE */
  USHORT  transmit_pos;     /* start position to transmit */
  UBYTE   p_counter;        /* priority counter to dermine send interval */

  /*
   * information for service RX
   */
  T_desc2* receive_data;    /* descriptor to write */
  USHORT  receive_pos;      /* start position to write */
  USHORT  receive_size;     /* size of write descriptor */
  UBYTE   receive_process;  /* state of receive process */

  /*
   * information for service KER
   */
  T_desc2* last_command;    /* last sent command frame */
  T_desc2* next_command;    /* next command frames to send */
  UBYTE   retransmissions;  /* number of retransmissions of command frame */
  ULONG   lines;            /* line states of DLC */
  UBYTE   service;
  UBYTE   codec;
  ULONG   received_prim;    /* bitfield of received UART primitives */
  BOOL    flushed;          /* indicator whether DLC is flushed */
} T_DLC;


/*
 * summery of all service
 */

typedef struct /* T_UART_DATA */
{

  /*
   * UART layer parameters
   */
  UBYTE                       version;

  /*
   * Service data structures
   *
   * Services with multiple incarnations require an array of structures
   * named xxx_base[] with xxx = service abbrevation, and additionally a
   * pointer named *xxx, which will be accessed instead of xxx_base.
   *
   * Services with only one incarnation just have to declare one structure
   * named xxx (no need for xxx_base[] and *xxx).
   *
   * The differentiation between the two access possibilites is made with
   * the defines of the service names above (UART_SERVICE_XXX).
   */
  T_KER_DATA                           ker;
  T_RX_DATA                            rx;
  T_TX_DATA                            tx;
  T_DRX_DATA                           drx_base[UART_MAX_NUMBER_OF_CHANNELS];
  T_DRX_DATA                           *drx;
  T_DTX_DATA                           dtx_base[UART_MAX_NUMBER_OF_CHANNELS];
  T_DTX_DATA                           *dtx;
  T_RT_DATA                            rt;

  /*
   * global UART variables
   */
  UBYTE             device;         /* index of the UART channel for this instance */
  USHORT            n1;             /* max. length of a frame in multiplexer mode */
  UBYTE             xon;            /* XON character for software flow control */
  UBYTE             xoff;           /* XOFF character for software flow control */
  T_DLC             dlc_table[UART_MAX_NUMBER_OF_CHANNELS + 1]; /* channel table */
  UBYTE             dlc_instance[64]; /* lookup table for for DLC instances */
  USHORT            timer_t1_index; /* Index of timer T1 in this instance */
  USHORT            timer_t2_index; /* Index of timer T2 in this instance */
  USHORT            timer_t3_index; /* Index of timer T3 in this instance */
  USHORT            timer_tesd_index;/* Index of timer TESD in this instance */
  UBYTE             act_ec;         /* Escape Character */
  USHORT            act_gp;         /* Guard Period */

#ifdef WIN32
  UBYTE   HISR_stack[HISR_STACK_SIZE];
#endif

  /*
   * global UART constants
   */
  UBYTE*            fcstab; /* lookup table for FCS calculation */
} T_UART_DATA;

#ifndef DTILIB
/*
 * global table which maps an unique c_id to
 * pointers of DRX/DTX instances which serve this channel
 */
typedef struct /* T_UART_CID_TABLE */
{
  UBYTE         c_id;           /* channel id */
  T_UART_DATA*  uart_data;      /* pointer to UART data for this channel */
  T_DRX_DATA*   drx;            /* pointer to DRX service for this channel */
  T_DTX_DATA*   dtx;            /* pointer to DTX service for this channel */
} T_UART_CID_TABLE;
#endif /* DTILIB */

/*==== EXPORT =====================================================*/


/*
 * Entity data base
 */

#ifdef UART_PEI_C
/*
 * FCS lookup table
 */
static UBYTE uart_fcstable_base[256] = { /* reversed, 8-bit, poly=0x07 */
    0x00, 0x91, 0xE3, 0x72, 0x07, 0x96, 0xE4, 0x75,
    0x0E, 0x9F, 0xED, 0x7C, 0x09, 0x98, 0xEA, 0x7B,
    0x1C, 0x8D, 0xFF, 0x6E, 0x1B, 0x8A, 0xF8, 0x69,
    0x12, 0x83, 0xF1, 0x60, 0x15, 0x84, 0xF6, 0x67,
    0x38, 0xA9, 0xDB, 0x4A, 0x3F, 0xAE, 0xDC, 0x4D,
    0x36, 0xA7, 0xD5, 0x44, 0x31, 0xA0, 0xD2, 0x43,
    0x24, 0xB5, 0xC7, 0x56, 0x23, 0xB2, 0xC0, 0x51,
    0x2A, 0xBB, 0xC9, 0x58, 0x2D, 0xBC, 0xCE, 0x5F,
    0x70, 0xE1, 0x93, 0x02, 0x77, 0xE6, 0x94, 0x05,
    0x7E, 0xEF, 0x9D, 0x0C, 0x79, 0xE8, 0x9A, 0x0B,
    0x6C, 0xFD, 0x8F, 0x1E, 0x6B, 0xFA, 0x88, 0x19,
    0x62, 0xF3, 0x81, 0x10, 0x65, 0xF4, 0x86, 0x17,
    0x48, 0xD9, 0xAB, 0x3A, 0x4F, 0xDE, 0xAC, 0x3D,
    0x46, 0xD7, 0xA5, 0x34, 0x41, 0xD0, 0xA2, 0x33,
    0x54, 0xC5, 0xB7, 0x26, 0x53, 0xC2, 0xB0, 0x21,
    0x5A, 0xCB, 0xB9, 0x28, 0x5D, 0xCC, 0xBE, 0x2F,
    0xE0, 0x71, 0x03, 0x92, 0xE7, 0x76, 0x04, 0x95,
    0xEE, 0x7F, 0x0D, 0x9C, 0xE9, 0x78, 0x0A, 0x9B,
    0xFC, 0x6D, 0x1F, 0x8E, 0xFB, 0x6A, 0x18, 0x89,
    0xF2, 0x63, 0x11, 0x80, 0xF5, 0x64, 0x16, 0x87,
    0xD8, 0x49, 0x3B, 0xAA, 0xDF, 0x4E, 0x3C, 0xAD,
    0xD6, 0x47, 0x35, 0xA4, 0xD1, 0x40, 0x32, 0xA3,
    0xC4, 0x55, 0x27, 0xB6, 0xC3, 0x52, 0x20, 0xB1,
    0xCA, 0x5B, 0x29, 0xB8, 0xCD, 0x5C, 0x2E, 0xBF,
    0x90, 0x01, 0x73, 0xE2, 0x97, 0x06, 0x74, 0xE5,
    0x9E, 0x0F, 0x7D, 0xEC, 0x99, 0x08, 0x7A, 0xEB,
    0x8C, 0x1D, 0x6F, 0xFE, 0x8B, 0x1A, 0x68, 0xF9,
    0x82, 0x13, 0x61, 0xF0, 0x85, 0x14, 0x66, 0xF7,
    0xA8, 0x39, 0x4B, 0xDA, 0xAF, 0x3E, 0x4C, 0xDD,
    0xA6, 0x37, 0x45, 0xD4, 0xA1, 0x30, 0x42, 0xD3,
    0xB4, 0x25, 0x57, 0xC6, 0xB3, 0x22, 0x50, 0xC1,
    0xBA, 0x2B, 0x59, 0xC8, 0xBD, 0x2C, 0x5E, 0xCF
};

T_UART_DATA uart_data_base[ UART_INSTANCES ], *uart_data;

#ifndef DTILIB
/*
 * global table which maps an unique c_id to the instance
 * number of the UART instance which serves this channel
 */
T_UART_CID_TABLE          uart_cid_table[ UART_INSTANCES * \
                              UART_MAX_NUMBER_OF_CHANNELS ];
#endif /* !DTILIB */

#else /* UART_PEI_C */
EXTERN UBYTE              uart_fcstable_base[256];
EXTERN T_UART_DATA        uart_data_base[], *uart_data;
#ifndef DTILIB
EXTERN T_UART_CID_TABLE   uart_cid_table[];
#endif /* !UART_DTILIB */
/*
 * function prototypes
 */
#ifdef FF_MULTI_PORT
EXTERN void pei_uart_driver_signal (T_DRV_SIGNAL *SigPtr);
#endif /* FF_MULTI_PORT */
#endif /* UART_PEI_C */

#define ENTITY_DATA           uart_data

/*
 * prefix service functions
 */
#define ker_init              _ENTITY_PREFIXED(ker_init)
#define rx_init               _ENTITY_PREFIXED(rx_init)
#define tx_init               _ENTITY_PREFIXED(tx_init)
#define drx_init              _ENTITY_PREFIXED(drx_init)
#define dtx_init              _ENTITY_PREFIXED(dtx_init)
#define rt_init               _ENTITY_PREFIXED(rt_init)
#define rx_read_data          _ENTITY_PREFIXED(rx_read_data)
#define tx_write_data         _ENTITY_PREFIXED(tx_write_data)
#define tx_create_hdlc_frame  _ENTITY_PREFIXED(tx_create_hdlc_frame)

#if defined (DTILIB)
#ifdef _SIMULATION_
#define sig_dti_ker_connection_opened_ind                     \
         _ENTITY_PREFIXED(sig_dti_ker_connection_opened_ind)
#define sig_dti_ker_connection_closed_ind                     \
         _ENTITY_PREFIXED(sig_dti_ker_connection_closed_ind)
#define sig_dti_drx_data_received_ind                         \
         _ENTITY_PREFIXED(sig_dti_drx_data_received_ind)
#define sig_dti_dtx_tx_buffer_full_ind                        \
         _ENTITY_PREFIXED(sig_dti_dtx_tx_buffer_full_ind)
#define sig_dti_dtx_tx_buffer_ready_ind                       \
         _ENTITY_PREFIXED(sig_dti_dtx_tx_buffer_ready_ind)

#ifdef DTI2
/*
 * for the driver-simulation dti channel, the c_id parameter
 * is already in use for transmission of control information
 * (which was formerly stored in the tui .. ;), unfortunately.
 * NOTE: not yet implemented!!!
 */
#define UART_TEST_C_ID_1            0
#define UART_TEST_C_ID_2            1



/*
 * these are __in no way__ real link_ids. Instead, the variable is used for
 * communication between the entity and the test environment, here. In
 * previous versions, the tui was used for this, but since DTI2 primitives do
 * not contain such a parameter any more, the dirty work has been passed on to
 * the link_id ..
 *
 * This is directly taken from
 */

#define LINK_READDATA_PORT_1   0 /* misused link_id for read_data call */
#define LINK_DISABLE_PORT_1    1 /* misused link_id for disable call */
#define LINK_ENABLE_PORT_1     2 /* misused link_id for enable call */
#define LINK_WRITEDATA_PORT_1  3 /* misused link_id for write_data call */

#define LINK_READDATA_PORT_2  10 /* misused link_id for read_data call */
#define LINK_DISABLE_PORT_2   11 /* misused link_id for disable call */
#define LINK_ENABLE_PORT_2    12 /* misused link_id for enable call */
#define LINK_WRITEDATA_PORT_2 13 /* misused link_id for write_data call */

#define LINK_UART_OUT_PORT_1   0 /* misused link_id of UART_OUT test interface */
#define LINK_UART_OUT_PORT_2  10 /* misused link_id of UART_OUT test interface */

#define LINK_PORT_THRESHOLD	   9 /* to be able to distinguish between port 1 and port2 */

#endif /* DTI2 */

#endif  /* _SIMULATION_ */

#endif  /* DTILIB */


/*
 * Communication handles (see also UART_PEI.C)
 */
#ifdef UART_PEI_C

       T_HANDLE hCommMMI          = VSI_ERROR;
       T_HANDLE hCommUART         = VSI_ERROR;
       T_HANDLE UART_handle;

/*
 * make the pei_create function unique
 */
#define pei_create              _ENTITY_PREFIXED(pei_create)

#else /* UART_PEI_C */

  EXTERN T_HANDLE hCommMMI;
  EXTERN T_HANDLE hCommUART;
  EXTERN T_HANDLE UART_handle;
  /*
   * functions for switching of entities/instances
   */
#ifndef DTILIB
  EXTERN UBYTE    pei_select_instances( UBYTE incoming_c_id );
#endif

#endif /* UART_PEI_C */

#ifdef DTILIB
  EXTERN DTI_HANDLE uart_hDTI;
#endif

#endif /* !UART_H */
