/*
+-----------------------------------------------------------------------------
|  Project :  GSM-F&D (8411)
|  Modul   :  ACI
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
|  Purpose :  Definitions for the AT Command Interpreter
+-----------------------------------------------------------------------------
*/

#ifndef ACI_H
#define ACI_H


#ifdef TI_PS_HCOMM_CHANGE
#include "cl_hComm_handle.h"
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
#define SHARED_CCD_BUF
#define MAX_FIE_CODE_BUF_LEN 220  /* FIE code buffer length */

/*==== TEST =====================================================*/
/*
 * instance management
 */

#define ACI_INSTANCES         6  /* is the same as UART_MAX_NUMBER_OF_CHANNELS in UART entity */

/*
 * PALLOC_SDU
 */

#define ENCODE_OFFSET             0

/*
 * Dynamic Configuration Numbers
 */

#define ACI_RESET                 0

#ifndef NO_ASCIIZ
#define NO_ASCIIZ
#endif

/*
 * Configuration Parameter
 */

/*
 * Dynamic Configuration Numbers
 */
#define RESET                     0
#define TIMER_SET                 1
#define TIMER_RESET               2
#define TIMER_SPEED_UP            3
#define TIMER_SLOW_DOWN           4
#define TIMER_SUPPRESS            5
#define KEY_SEQUENCE              6
#define START_AOC                 7
#define KEY_PRESS                 8
#define KEY_RELEASE               9
#define CPOL_MODE                10
#define CPOL_IDX2                11
#define ATI_VIA_TIF              12
#ifdef WIN32
#define DATA_INPUT               13
#endif /* WIN32 */
#define DATA_INPUT               13


/*
 * Timer Names
 */
#ifdef FF_ATI
#define TRING                "TRING"
#endif

#define TECT                 "TECT"
#define TMPTY                "TMPTY"
#define TFIT                 "TFIT"
#define TDTMF                "TDTMF"

#define ONE_FRAME     5
#define TEN_FRAMES    47

/*==== bit field manipulation macros ==============================*/

#define BITFIELD_CHECK(x, y)  (x & y)
#define BITFIELD_SET(x, y)    (x |= y)
#define BITFIELD_CLEAR(x, y)  (x &= (~y))

/*==== TYPES ======================================================*/

typedef struct
{
  UBYTE                 t_mode;
  ULONG                 t_val;
} T_TIMER_CONFIG;

/*
*   ACI types
*/

typedef struct
{
  UBYTE dummy;
} T_ACI_DATA;

typedef struct
{
  USHORT l_buf;
  USHORT o_buf;
  UBYTE  buf[MAX_FIE_CODE_BUF_LEN];     /* buffer for FIE coding */
} T_ACI_FIE_BUF;

#define TIMERSTART(v,h) TIMER_START(aci_handle, h, v)

#define PTIMERSTART(v0,v1,h) TIMER_PSTART (aci_handle, h, v0,v1)

#define TIMERSTOP(h) TIMER_STOP(aci_handle, h)


/*==== EXPORT =====================================================*/
/*
 * CCD decode buffer
 */
#ifdef OPTION_MULTITHREAD
  #define _decodedMsg   _ENTITY_PREFIXED(_decodedMsg)
#endif

#if !defined SHARED_CCD_BUF
#define CCD_START
#define CCD_END
#else
#define CCD_START if(!CCDbuf){_decodedMsg = ccd_begin();CCDbuf=TRUE;}
#define CCD_END   if(CCDbuf){ccd_end();CCDbuf=FALSE;}
#endif

#ifdef PSA_SSF_C

GLOBAL T_ACI_FIE_BUF ssFIECodeBuf;
GLOBAL UBYTE * ssFIEDecodeBuf;

#if !defined SHARED_CCD_BUF
#ifdef SIM_TOOLKIT
GLOBAL UBYTE _decodedMsg[MAXIMUM(MAXIMUM(MAX_MSTRUCT_LEN_FAC,
                                         MAX_MSTRUCT_LEN_SAT),
                                 MAX_MSTRUCT_LEN_CC)];
#else
GLOBAL UBYTE _decodedMsg[MAXIMUM(MAX_MSTRUCT_LEN_FAC,
                                 MAX_MSTRUCT_LEN_CC)];
#endif  /* SIM_TOOLKIT */
#else
GLOBAL UBYTE* _decodedMsg;
GLOBAL UBYTE  CCDbuf = FALSE;
#endif  /* SHARED_CCD_BUF */
#else /* PSA_SSF_C */

EXTERN T_ACI_FIE_BUF ssFIECodeBuf;
EXTERN UBYTE * ssFIEDecodeBuf;

#if !defined SHARED_CCD_BUF
#ifdef SIM_TOOLKIT
EXTERN UBYTE _decodedMsg[MAXIMUM(MAXIMUM(MAX_MSTRUCT_LEN_FAC,
                                         MAX_MSTRUCT_LEN_SAT),
                                 MAX_MSTRUCT_LEN_CC)];
#else
EXTERN UBYTE _decodedMsg[MAXIMUM(MAX_MSTRUCT_LEN_FAC,
                                 MAX_MSTRUCT_LEN_CC)];
#endif  /* SIM_TOOLKIT */
#else
EXTERN UBYTE* _decodedMsg;
EXTERN UBYTE  CCDbuf;
#endif  /* SHARED_CCD_BUF */

#endif /* PSA_SSF_C */

/*
 * Prototypes Timer Modul
 */

/*
 * If all entities are linked into one module this definitions
 * prefixes all this functions with the entity name
 */
EXTERN BOOL smi_timeout                 (USHORT              handle);

/*
 * If all entities are linked into one module this definitions
 * prefixes the global data with the entity name
 */

#ifdef OPTION_MULTITHREAD
#ifdef TI_PS_HCOMM_CHANGE
  #define hCommACI        _ENTITY_PREFIXED(hCommACI)
#if defined FF_TRACE_OVER_MTST
  #define hCommMTST       _ENTITY_PREFIXED(hCommMTST)
#endif

#ifdef FF_ESIM
  #define hCommESIM         _ENTITY_PREFIXED(hCommESIM)
#endif

#ifdef UART
  #define hCommDTI        _ENTITY_PREFIXED(hCommDTI)
#endif

#ifdef FF_GPF_TCPIP
  #define hCommTCPIP      _ENTITY_PREFIXED(hCommTCPIP)
  #define hCommDCM       _ENTITY_PREFIXED(hCommDCM)
#endif

#if defined (CO_TCPIP_TESTAPP) || defined  (CO_BAT_TESTAPP)
  #define hCommAPP        _ENTITY_PREFIXED(hCommAPP)
#endif 

#ifdef GPRS
  #define hCommGMM        _ENTITY_PREFIXED(hCommGMM)
  #define hCommSM         _ENTITY_PREFIXED(hCommSM)
  #define hCommSNDCP      _ENTITY_PREFIXED(hCommSNDCP)
#ifdef FF_PKTIO
  #define hCommPKTIO      _ENTITY_PREFIXED(hCommPKTIO)
#endif
#endif /* GPRS */
#else
  
  #define hCommACI        _ENTITY_PREFIXED(hCommACI)
#if defined FF_TRACE_OVER_MTST
  #define hCommMTST       _ENTITY_PREFIXED(hCommMTST)
#endif
  #define hCommSIM        _ENTITY_PREFIXED(hCommSIM)
  #define hCommMM         _ENTITY_PREFIXED(hCommMM)
  #define hCommCC         _ENTITY_PREFIXED(hCommCC)
#ifdef FF_ESIM
  #define hCommESIM         _ENTITY_PREFIXED(hCommESIM)
#endif
#ifdef UART
  #define hCommUART       _ENTITY_PREFIXED(hCommUART)

  #define hCommDTI        _ENTITY_PREFIXED(hCommDTI)
#endif
#ifdef FF_PSI
    #define hCommPSI          _ENTITY_PREFIXED(hCommPSI)
#endif /*FF_PSI*/
  #define hCommSS         _ENTITY_PREFIXED(hCommSS)
  #define hCommSMS        _ENTITY_PREFIXED(hCommSMS)
  #define hCommPL         _ENTITY_PREFIXED(hCommPL)
#if defined FF_EOTD
  #define hCommLC         _ENTITY_PREFIXED(hCommLC)
#endif
#ifdef FF_TCP_IP
  #define hCommAAA        _ENTITY_PREFIXED(hCommAAA)  /* SKA 11.Dec2002 */
#endif

#ifdef FAX_AND_DATA
  #define hCommL2R        _ENTITY_PREFIXED(hCommL2R)
  #define hCommTRA        _ENTITY_PREFIXED(hCommTRA)
#ifdef FF_FAX
  #define hCommT30        _ENTITY_PREFIXED(hCommT30)
#endif
#ifndef USE_L1FD_FUNC_INTERFACE
  #define hCommRA         _ENTITY_PREFIXED(hCommRA)
#endif
#ifdef FF_EOTD
  #define hCommLC         _ENTITY_PREFIXED(hCommLC)
#endif
#endif /* FAX_AND_DATA */

#ifdef FF_GPF_TCPIP
  #define hCommTCPIP      _ENTITY_PREFIXED(hCommTCPIP)
  #define hCommDCM       _ENTITY_PREFIXED(hCommDCM)
#endif

#if defined (CO_TCPIP_TESTAPP) || defined  (CO_BAT_TESTAPP)
  #define hCommAPP        _ENTITY_PREFIXED(hCommAPP)
#endif

#ifdef CO_UDP_IP
  #define hCommUDP        _ENTITY_PREFIXED(hCommUDP)
  #define hCommIP         _ENTITY_PREFIXED(hCommIP)
#endif

#ifdef FF_WAP
  #define hCommWAP       _ENTITY_PREFIXED(hCommWAP)
#endif /* FF_WAP */

#if defined (FF_WAP) || defined (GPRS) || defined (FF_PPP) || defined(FF_GPF_TCPIP) || defined (FF_SAT_E)
  #define hCommPPP        _ENTITY_PREFIXED(hCommPPP)
#endif /* (FF_WAP) || (GPRS) || (FF_PPP) || defined (FF_SAT_E) */

/* used only for the EM to reduce primitive sending. Otherwise all primitives from
   ACI to RR have to be passed through MM.
*/
#ifdef FF_EM_MODE
  #define hCommRR         _ENTITY_PREFIXED(hCommRR)
#endif /* FF_EM_MODE */

#ifdef GPRS
  #define hCommGMM        _ENTITY_PREFIXED(hCommGMM)
  #define hCommSM         _ENTITY_PREFIXED(hCommSM)
  #define hCommSNDCP      _ENTITY_PREFIXED(hCommSNDCP)
  #define hCommUPM        _ENTITY_PREFIXED(hCommUPM)
#ifdef FF_PKTIO
  #define hCommPKTIO      _ENTITY_PREFIXED(hCommPKTIO)
#endif
#endif /* GPRS */
#if defined(FF_ATI) && defined(BT_ADAPTER)
  #define hCommBTI        _ENTITY_PREFIXED(hCommBTI)
#endif /* FF_ATI && BT_ADAPTER */
#define hCommL1 _ENTITY_PREFIXED(hCommL1)  
#endif /* TI_PS_HCOMM_CHANGE */
#endif


#ifdef ACI_PEI_C

#ifdef TI_PS_HCOMM_CHANGE

GLOBAL T_HANDLE         hCommACI = VSI_ERROR; /* ACI  Communication */

#ifdef FAX_AND_DATA
GLOBAL T_HANDLE         hCommTRA;        /* TRA  Communication       */
#endif

#ifdef UART
GLOBAL T_HANDLE         hCommDTI = VSI_ERROR; /* SMS  Communication */
#endif

#ifdef FF_ESIM
GLOBAL T_HANDLE         hCommESIM  = VSI_ERROR; /* ESIM   Communication */
#endif

#ifdef FF_TRACE_OVER_MTST
GLOBAL T_HANDLE         hCommMTST = VSI_ERROR;/* MTST  Communication */
#endif

#ifdef FF_GPF_TCPIP
GLOBAL T_HANDLE         hCommTCPIP = VSI_ERROR; /* TCPIP Communication */
#endif

#if defined (CO_TCPIP_TESTAPP) || defined  (CO_BAT_TESTAPP)
GLOBAL T_HANDLE         hCommAPP   = VSI_ERROR; /* APP Communication   */
#endif

#ifdef GPRS
GLOBAL T_HANDLE         hCommGMM   = VSI_ERROR; /* GMM    Communication */
GLOBAL T_HANDLE         hCommSM    = VSI_ERROR; /* SM     Communication */
GLOBAL T_HANDLE         hCommSNDCP = VSI_ERROR; /* SNDCP  Communication */
#ifdef FF_PKTIO
GLOBAL T_HANDLE         hCommPKTIO = VSI_ERROR; /* PKTIO  Communication */
#endif
#endif /* GPRS */
GLOBAL T_HANDLE         aci_handle;
#define hCommMMI        _hCommMMI
GLOBAL T_HANDLE         hCommDCM = VSI_ERROR;   /* DCM Communication */
#else

GLOBAL T_HANDLE         hCommACI = VSI_ERROR; /* ACI  Communication */
#ifdef FF_TRACE_OVER_MTST
GLOBAL T_HANDLE         hCommMTST = VSI_ERROR;/* MTST  Communication */
#endif
GLOBAL T_HANDLE         hCommSIM = VSI_ERROR; /* SIM  Communication */
GLOBAL T_HANDLE         hCommMM  = VSI_ERROR; /* MM   Communication */
GLOBAL T_HANDLE         hCommCC  = VSI_ERROR; /* CC   Communication */
#ifdef UART
GLOBAL T_HANDLE         hCommUART= VSI_ERROR; /* UART   Communication */

GLOBAL T_HANDLE         hCommDTI = VSI_ERROR; /* SMS  Communication */
#endif
GLOBAL T_HANDLE         hCommSS  = VSI_ERROR; /* SS   Communication */
#ifdef FF_ESIM
GLOBAL T_HANDLE         hCommESIM  = VSI_ERROR; /* ESIM   Communication */
#endif
GLOBAL T_HANDLE         hCommSMS = VSI_ERROR; /* SMS  Communication */
GLOBAL T_HANDLE         hCommPL  = VSI_ERROR; /* PL   Communication */
#if defined FF_EOTD
GLOBAL T_HANDLE         hCommLC  = VSI_ERROR; /* LC   Communication */
#endif
#ifdef FF_TCP_IP
GLOBAL T_HANDLE         hCommAAA = VSI_ERROR; /* AAA  Communication */ /* SKA 11.Dec2002 */
#endif

#ifdef FAX_AND_DATA
GLOBAL T_HANDLE         hCommL2R = VSI_ERROR; /* L2R  Communication */
GLOBAL T_HANDLE         hCommTRA = VSI_ERROR; /* TRA  Communication */
#ifdef FF_FAX
GLOBAL T_HANDLE         hCommT30 = VSI_ERROR; /* T30  Communication */
#endif
#ifndef USE_L1FD_FUNC_INTERFACE
GLOBAL T_HANDLE         hCommRA  = VSI_ERROR; /* RA   Communication */
#endif
#endif  /* FAX_AND_DATA */


#ifdef FF_GPF_TCPIP
GLOBAL T_HANDLE         hCommTCPIP = VSI_ERROR; /* TCPIP Communication */
#endif

#if defined (CO_TCPIP_TESTAPP) || defined  (CO_BAT_TESTAPP)
GLOBAL T_HANDLE         hCommAPP   = VSI_ERROR; /* APP Communication   */
#endif

#ifdef CO_UDP_IP 
GLOBAL T_HANDLE         hCommUDP = VSI_ERROR; /* UDP Communication */
GLOBAL T_HANDLE         hCommIP  = VSI_ERROR; /* IP  Communication */
#endif  /* defined CO_UDP_IP */

#ifdef FF_WAP
GLOBAL T_HANDLE         hCommWAP  = VSI_ERROR; /* WAP  Communication */
#endif /* FF_WAP */


#if defined (FF_WAP) || defined (GPRS) || defined (FF_PPP) || defined(FF_GPF_TCPIP)
GLOBAL T_HANDLE         hCommPPP = VSI_ERROR; /* PPP Communication */
#endif /*  (FF_WAP) || (GPRS) || (FF_PPP) || (FF_GPF_TCPIP) */

#ifdef FF_EM_MODE
GLOBAL T_HANDLE         hCommRR  = VSI_ERROR;   /* EM Communication */
#endif /* FF_EM_MODE */

#ifdef GPRS
GLOBAL T_HANDLE         hCommGMM   = VSI_ERROR; /* GMM    Communication */
GLOBAL T_HANDLE         hCommSM    = VSI_ERROR; /* SM     Communication */
GLOBAL T_HANDLE         hCommSNDCP = VSI_ERROR; /* SNDCP  Communication */
GLOBAL T_HANDLE         hCommUPM   = VSI_ERROR; /* UPM    Communication */
#ifdef FF_PKTIO
GLOBAL T_HANDLE         hCommPKTIO = VSI_ERROR; /* PKTIO  Communication */
#endif
#endif /* GPRS */
#ifdef FF_PSI
GLOBAL T_HANDLE         hCommPSI = VSI_ERROR; /* PSI Communication */
#endif /*FF_PSI*/
#if defined(FF_ATI) && defined(BT_ADAPTER)
GLOBAL T_HANDLE         hCommBTI   = VSI_ERROR; /* BTI    Communication */
#endif /* FF_ATI && BT_ADAPTER */

GLOBAL T_HANDLE         hCommL1    = VSI_ERROR; /* Layer 1 Communication */

GLOBAL T_HANDLE         aci_handle;
#define hCommMMI        aci_handle
GLOBAL T_HANDLE         hCommDCM = VSI_ERROR;   /* DCM Communication */
#endif /* TI_PS_HCOMM_CHANGE */

#else   /* ACI_PEI_C */

#ifdef TI_PS_HCOMM_CHANGE
EXTERN T_HANDLE         hCommACI;        /* ACI  Communication       */
#ifdef FF_ESIM
EXTERN T_HANDLE         hCommESIM;         /* ESIM   Communication       */
#endif

#ifdef UART
EXTERN T_HANDLE         hCommDTI;        /* DTI  Communication       */
#endif

#ifdef FAX_AND_DATA
EXTERN T_HANDLE         hCommTRA;        /* TRA  Communication       */
#endif

#ifdef FF_GPF_TCPIP
EXTERN T_HANDLE         hCommTCPIP;      /* TCPIP Communication      */
EXTERN T_HANDLE         hCommDCM;        /* DCM Communication        */
#endif

#if defined (CO_TCPIP_TESTAPP) || defined  (CO_BAT_TESTAPP)
EXTERN T_HANDLE         hCommAPP;        /* APP Communication        */
#endif

#if defined FF_TRACE_OVER_MTST
EXTERN T_HANDLE         hCommMTST;       /* MTST  Communication */
#endif

#ifdef GPRS
EXTERN T_HANDLE         hCommGMM;        /* GMM    Communication */
EXTERN T_HANDLE         hCommSM;         /* SM     Communication */
EXTERN T_HANDLE         hCommSNDCP;      /* SNDCP  Communication */
EXTERN T_HANDLE         hCommPKTIO;      /* PKTIO  Communication */
#endif /* GPRS */

EXTERN T_HANDLE         hcommDCM;    /*Application Commnunication */
EXTERN T_HANDLE         aci_handle;
#define hCommMMI        _hCommMMI
#else
EXTERN T_HANDLE         hCommACI;        /* ACI  Communication       */
#if defined FF_TRACE_OVER_MTST
EXTERN T_HANDLE         hCommMTST;       /* MTST  Communication */
#endif
EXTERN T_HANDLE         hCommSIM;        /* SIM  Communication       */
EXTERN T_HANDLE         hCommMM;         /* MM   Communication       */
EXTERN T_HANDLE         hCommCC;         /* CC   Communication       */
#ifdef UART
EXTERN T_HANDLE         hCommUART;       /* UART Communication       */

EXTERN T_HANDLE         hCommDTI;        /* DTI  Communication       */
#endif
EXTERN T_HANDLE         hCommSS;         /* SS   Communication       */
#ifdef FF_ESIM
EXTERN T_HANDLE         hCommESIM;         /* ESIM   Communication       */
#endif
EXTERN T_HANDLE         hCommSMS;        /* SMS  Communication       */
EXTERN T_HANDLE         hCommPL;         /* PL   Communication       */
#if defined FF_EOTD
EXTERN T_HANDLE         hCommLC;         /* LC   Communication       */
#endif
#ifdef FF_TCP_IP
EXTERN T_HANDLE         hCommAAA;        /* AAA  Communication */ /* SKA 11.Dec2002 */
#endif 

#ifdef FAX_AND_DATA
EXTERN T_HANDLE         hCommL2R;        /* L2R  Communication       */

EXTERN T_HANDLE         hCommTRA;        /* TRA  Communication       */
#ifdef FF_FAX
EXTERN T_HANDLE         hCommT30;        /* T30  Communication       */
#endif
#ifndef USE_L1FD_FUNC_INTERFACE
EXTERN T_HANDLE         hCommRA;         /* RA   Communication       */
#endif
#endif /* FAX_AND_DATA */

#ifdef FF_GPF_TCPIP
EXTERN T_HANDLE         hCommTCPIP;      /* TCPIP Communication      */
EXTERN T_HANDLE         hCommDCM;        /* DCM Communication        */
#endif

#if defined (CO_TCPIP_TESTAPP) || defined  (CO_BAT_TESTAPP)
EXTERN T_HANDLE         hCommAPP;        /* APP Communication        */
#endif

#ifdef CO_UDP_IP
EXTERN T_HANDLE         hCommUDP;        /* UDP Communication        */
EXTERN T_HANDLE         hCommIP;         /* IP  Communication        */
#endif

#ifdef FF_WAP
EXTERN T_HANDLE         hCommWAP;        /* WAP  Communication       */
#endif


#if defined (FF_WAP) || defined (GPRS) || defined (FF_PPP) || defined(FF_GPF_TCPIP)
EXTERN T_HANDLE         hCommPPP;        /* PPP Communication        */
#endif /*  (FF_WAP) ||  (GPRS) ||  (FF_PPP) || (FF_GPF_TCPIP) */

#ifdef FF_EM_MODE
EXTERN T_HANDLE         hCommRR;         /* EM Communication */
#endif /* FF_EM_MODE */

#ifdef GPRS
EXTERN T_HANDLE         hCommGMM;        /* GMM    Communication */
EXTERN T_HANDLE         hCommSM;         /* SM     Communication */
EXTERN T_HANDLE         hCommSNDCP;      /* SNDCP  Communication */
EXTERN T_HANDLE         hCommUPM;        /* UPM    Communication */
EXTERN T_HANDLE         hCommPKTIO;      /* PKTIO  Communication */
#endif /* GPRS */
#ifdef FF_PSI
EXTERN T_HANDLE         hCommPSI;      /* PSI  Communication */
#endif /*FF_PSI*/
#if defined(FF_ATI) && defined(BT_ADAPTER)
EXTERN T_HANDLE         hCommBTI;        /* BTI    Communication */
#endif /* FF_ATI && BT_ADAPTER */

EXTERN T_HANDLE      hcommDCM;    /*Application Commnunication */
EXTERN T_HANDLE      hCommL1; /* Layer 1 */

EXTERN T_HANDLE         aci_handle;
#define hCommMMI        aci_handle
#endif /* TI_PS_HCOMM_CHANGE */

#endif  /* ACI_PEI_C */

#ifdef SIM_PERS_OTA
#define ACI_PID_ME_DEPERSON  0x7E
#endif
/*
 * function prototypes
 */
EXTERN void aci_aci_cmd_req (T_ACI_CMD_REQ *data);
EXTERN void aci_aci_abort_req (T_ACI_ABORT_REQ *data);
#ifdef BT_ADAPTER
EXTERN void aci_aci_cmd_res (T_ACI_CMD_RES *aci_cmd_res);
EXTERN void aci_aci_init_res (T_ACI_INIT_RES *aci_init_res);
EXTERN void aci_aci_deinit_req (T_ACI_DEINIT_REQ *aci_deinit_req);
EXTERN void aci_aci_open_port_req (T_ACI_OPEN_PORT_REQ *aci_open_port_req);
EXTERN void aci_aci_close_port_req (T_ACI_CLOSE_PORT_REQ *aci_close_port_req);
/* temporary solutions for BT. primitives contain src_id. this must be enabled for all ACI primitives */
EXTERN void aci_aci_cmd_req_bt (T_ACI_CMD_REQ_BT *cmd);
EXTERN void aci_aci_abort_req_bt (T_ACI_ABORT_REQ_BT *data);
EXTERN void aci_aci_cmd_res_bt (T_ACI_CMD_RES_BT *aci_cmd_res);
#endif
EXTERN void aci_aci_trc_ind (T_ACI_TRC_IND *trc_ind);
#ifdef FF_MMI_RIV
EXTERN void aci_aci_riv_cmd_req (T_ACI_RIV_CMD_REQ *cmd_ptr);
#endif
EXTERN void aci_aci_ext_ind (T_ACI_EXT_IND *aci_ext_ind);
/*
 * If all entities are linked into one module this definitions
 * prefixes all this functions with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define psa_sim_read_cnf          _ENTITY_PREFIXED(psa_sim_read_cnf)
  #define psa_sim_update_cnf        _ENTITY_PREFIXED(psa_sim_update_cnf)
  #define psa_sim_read_record_cnf   _ENTITY_PREFIXED(psa_sim_read_record_cnf)
  #define psa_sim_update_record_cnf _ENTITY_PREFIXED(psa_sim_update_record_cnf)
  #define psa_sim_increment_cnf     _ENTITY_PREFIXED(psa_sim_increment_cnf)
  #define psa_sim_verify_pin_cnf    _ENTITY_PREFIXED(psa_sim_verify_pin_cnf)
  #define psa_sim_change_pin_cnf    _ENTITY_PREFIXED(psa_sim_change_pin_cnf)
  #define psa_sim_disable_pin_cnf   _ENTITY_PREFIXED(psa_sim_disable_pin_cnf)
  #define psa_sim_enable_pin_cnf    _ENTITY_PREFIXED(psa_sim_enable_pin_cnf)
  #define psa_sim_unblock_cnf       _ENTITY_PREFIXED(psa_sim_unblock_cnf)
  #define psa_sim_mmi_insert_ind    _ENTITY_PREFIXED(psa_sim_mmi_insert_ind)
  #define psa_sim_remove_ind        _ENTITY_PREFIXED(psa_sim_remove_ind)
  #define psa_sim_sync_cnf          _ENTITY_PREFIXED(psa_sim_sync_cnf)
  #define psa_sim_activate_cnf      _ENTITY_PREFIXED(psa_sim_activate_cnf)
  #define psa_sim_activate_ind      _ENTITY_PREFIXED(psa_sim_activate_ind)
  #define psa_sim_access_cnf        _ENTITY_PREFIXED(psa_sim_access_cnf)
#ifdef SIM_TOOLKIT
  #define psa_sim_toolkit_ind       _ENTITY_PREFIXED(psa_sim_toolkit_ind)
  #define psa_sim_toolkit_cnf       _ENTITY_PREFIXED(psa_sim_toolkit_cnf)
  #define psa_sim_file_update_ind   _ENTITY_PREFIXED(psa_sim_file_update_ind)
#endif /* SIM_TOOLKIT */
#ifdef FF_EM_MODE
  #define psa_em_sc_info_cnf          _ENTITY_PREFIXED(psa_em_sc_info_cnf)
  #define psa_em_sc_gprs_info_cnf     _ENTITY_PREFIXED(psa_em_sc_gprs_info_cnf)
  #define psa_em_nc_info_cnf          _ENTITY_PREFIXED(psa_em_nc_info_cnf)
  #define psa_em_loc_pag_info_cnf     _ENTITY_PREFIXED(psa_em_loc_pag_info_cnf)
  #define psa_em_plmn_info_cnf        _ENTITY_PREFIXED(psa_em_plmn_info_cnf)
  #define psa_em_cip_hop_dtx_info_cnf _ENTITY_PREFIXED(psa_em_cip_hop_dtx_info_cnf)
  #define psa_em_power_info_cnf       _ENTITY_PREFIXED(psa_em_power_info_cnf)
  #define psa_em_identity_info_cnf    _ENTITY_PREFIXED(psa_em_identity_info_cnf)
  #define psa_em_sw_version_info_cnf  _ENTITY_PREFIXED(psa_em_sw_version_info_cnf)
  #define psa_em_gmm_info_cnf         _ENTITY_PREFIXED(psa_em_gmm_info_cnf)
  #define psa_em_grlc_info_cnf        _ENTITY_PREFIXED(psa_em_grlc_info_cnf)
  #define psa_em_amr_info_cnf         _ENTITY_PREFIXED(psa_em_amr_info_cnf)
#endif /* FF_EM_MODE */
  #define psa_mmr_reg_cnf           _ENTITY_PREFIXED(psa_mmr_reg_cnf)
  #define psa_mmr_nreg_ind          _ENTITY_PREFIXED(psa_mmr_nreg_ind)
  #define psa_mmr_nreg_cnf          _ENTITY_PREFIXED(psa_mmr_nreg_cnf)
  #define psa_mmr_plmn_ind          _ENTITY_PREFIXED(psa_mmr_plmn_ind)
  #define psa_mmr_info_ind          _ENTITY_PREFIXED(psa_mmr_info_ind)
  #define psa_mmr_ciphering_ind     _ENTITY_PREFIXED(psa_mmr_ciphering_ind)
  #define psa_mmr_ahplmn_ind        _ENTITY_PREFIXED(psa_mmr_ahplmn_ind)
  #define psa_mncc_alert_ind        _ENTITY_PREFIXED(psa_mncc_alert_ind)
  #define psa_mncc_call_proceed_ind _ENTITY_PREFIXED(psa_mncc_call_proceed_ind)
  #define psa_mncc_disconnect_ind   _ENTITY_PREFIXED(psa_mncc_disconnect_ind)
  #define psa_mncc_hold_cnf         _ENTITY_PREFIXED(psa_mncc_hold_cnf)
  #define psa_mncc_progress_ind     _ENTITY_PREFIXED(psa_mncc_progress_ind)
  #define psa_mncc_reject_ind       _ENTITY_PREFIXED(psa_mncc_reject_ind)
  #define psa_mncc_release_cnf      _ENTITY_PREFIXED(psa_mncc_release_cnf)
  #define psa_mncc_release_ind      _ENTITY_PREFIXED(psa_mncc_release_ind)
  #define psa_mncc_setup_cnf        _ENTITY_PREFIXED(psa_mncc_setup_cnf)
  #define psa_mncc_setup_compl_ind  _ENTITY_PREFIXED(psa_mncc_setup_compl_ind)
  #define psa_mncc_setup_ind        _ENTITY_PREFIXED(psa_mncc_setup_ind)
  #define psa_mncc_start_dtmf_cnf   _ENTITY_PREFIXED(psa_mncc_start_dtmf_cnf)
  #define psa_mncc_sync_ind         _ENTITY_PREFIXED(psa_mncc_sync_ind)
  #define psa_mncc_bearer_cap_cnf   _ENTITY_PREFIXED(psa_mncc_bearer_cap_cnf)
  #define psa_mncc_prompt_ind       _ENTITY_PREFIXED(psa_mncc_prompt_ind)
  #define psa_mncc_recall_ind       _ENTITY_PREFIXED(psa_mncc_recall_ind)
  #define psa_mncc_status_ind       _ENTITY_PREFIXED(psa_mncc_status_ind)
#if defined FF_EOTD
  #define psa_mnlc_sms_meas_cnf       _ENTITY_PREFIXED(psa_mnlc_sms_meas_cnf)
#endif
  #define psa_mnss_begin_ind        _ENTITY_PREFIXED(psa_mnss_begin_ind)
  #define psa_mnss_facility_ind     _ENTITY_PREFIXED(psa_mnss_facility_ind)
  #define psa_mnss_end_ind          _ENTITY_PREFIXED(psa_mnss_end_ind)

  #define psa_mnsms_delete_cnf      _ENTITY_PREFIXED(psa_mnsms_delete_cnf)
  #define psa_mnsms_read_cnf        _ENTITY_PREFIXED(psa_mnsms_read_cnf)
  #define psa_mnsms_store_cnf       _ENTITY_PREFIXED(psa_mnsms_store_cnf)
  #define psa_mnsms_submit_cnf      _ENTITY_PREFIXED(psa_mnsms_submit_cnf)
  #define psa_mnsms_command_cnf     _ENTITY_PREFIXED(psa_mnsms_command_cnf)
  #define psa_mnsms_report_ind      _ENTITY_PREFIXED(psa_mnsms_report_ind)
  #define psa_mnsms_status_ind      _ENTITY_PREFIXED(psa_mnsms_status_ind)
  #define psa_mnsms_message_ind     _ENTITY_PREFIXED(psa_mnsms_message_ind)
  #define psa_mnsms_error_ind       _ENTITY_PREFIXED(psa_mnsms_error_ind)
#ifdef TI_PS_FF_AT_P_CMD_CPRSM
  #define psa_mnsms_resume_cnf      _ENTITY_PREFIXED(psa_mnsms_resume_cnf)
  #define psa_mnsms_query_cnf       _ENTITY_PREFIXED(psa_mnsms_query_cnf)
#endif /* TI_PS_FF_AT_P_CMD_CPRSM */
#ifdef REL99
  #define psa_mnsms_send_prog_ind   _ENTITY_PREFIXED(psa_mnsms_send_prog_ind)
  #define psa_mnsms_retrans_cnf     _ENTITY_PREFIXED(psa_mnsms_retrans_cnf)
#endif /* REL99 */
#ifdef GPRS
  #define psa_mnsms_mo_serv_cnf     _ENTITY_PREFIXED(psa_mnsms_mo_serv_cnf)
#endif  /* GPRS */
  #define psa_mnsms_OTA_message_ind  _ENTITY_PREFIXED(psa_mnsms_OTA_message_ind)
  #define psa_mmi_keypad_ind        _ENTITY_PREFIXED(psa_mmi_keypad_ind)
  #define psa_mmi_rxlev_ind         _ENTITY_PREFIXED(psa_mmi_rxlev_ind)
  #define psa_mmi_battery_ind       _ENTITY_PREFIXED(psa_mmi_battery_ind)
  #define psa_mmi_cbch_ind          _ENTITY_PREFIXED(psa_mmi_cbch_ind)
#ifdef BTE_MOBILE
  #define psa_mmi_bt_cb_notify_ind        _ENTITY_PREFIXED(psa_mmi_bt_cb_notify_ind)
#endif
#ifndef VOCODER_FUNC_INTERFACE
#define psa_mmi_tch_vocoder_cfg_cnf  _ENTITY_PREFIXED(psa_mmi_tch_vocoder_cfg_cnf) 
#endif /* VOCODER_FUNC_INTERFACE */

#ifdef SIM_TOOLKIT
  #define psa_sat_cbch_dnl_ind      _ENTITY_PREFIXED(psa_mmi_cbch_dnl_ind)
#endif /* SIM_TOOLKIT */

  #define psa_ra_activate_cnf       _ENTITY_PREFIXED(psa_ra_activate_cnf)
  #define psa_ra_deactivate_cnf     _ENTITY_PREFIXED(psa_ra_deactivate_cnf)
#ifdef FF_FAX
  #define psa_ra_modify_cnf         _ENTITY_PREFIXED(psa_ra_modify_cnf)
  #define psa_t30_cap_ind           _ENTITY_PREFIXED(psa_t30_cap_ind)
  #define psa_t30_dti_cnf           _ENTITY_PREFIXED(psa_t30_dti_cnf)
  #define psa_t30_dti_ind           _ENTITY_PREFIXED(psa_t30_dti_ind)
  #define psa_t30_phase_ind         _ENTITY_PREFIXED(psa_t30_phase_ind)
  #define psa_t30_sgn_ind           _ENTITY_PREFIXED(psa_t30_sgn_ind)
  #define psa_t30_cmpl_ind          _ENTITY_PREFIXED(psa_t30_cmpl_ind)
  #define psa_t30_report_ind        _ENTITY_PREFIXED(psa_t30_report_ind)
  #define psa_t30_error_ind         _ENTITY_PREFIXED(psa_t30_error_ind)
  #define psa_t30_eol_ind           _ENTITY_PREFIXED(psa_t30_eol_ind)
  #define psa_t30_deactivate_cnf    _ENTITY_PREFIXED(psa_t30_deactivate_cnf)
  #define psa_t30_activate_cnf      _ENTITY_PREFIXED(psa_t30_activate_cnf)
  #define psa_t30_preamble_ind      _ENTITY_PREFIXED(psa_t30_preamble_ind)
#endif
  #define psa_l2r_activate_cnf      _ENTITY_PREFIXED(psa_l2r_activate_cnf)
  #define psa_l2r_deactivate_cnf    _ENTITY_PREFIXED(psa_l2r_deactivate_cnf)
  #define psa_l2r_connect_cnf       _ENTITY_PREFIXED(psa_l2r_connect_cnf)
  #define psa_l2r_connect_ind       _ENTITY_PREFIXED(psa_l2r_connect_ind)
  #define psa_l2r_disc_cnf          _ENTITY_PREFIXED(psa_l2r_disc_cnf)
  #define psa_l2r_disc_ind          _ENTITY_PREFIXED(psa_l2r_disc_ind)
  #define psa_l2r_xid_ind           _ENTITY_PREFIXED(psa_l2r_xid_ind)
  #define psa_l2r_error_ind         _ENTITY_PREFIXED(psa_l2r_error_ind)
  #define psa_l2r_reset_ind         _ENTITY_PREFIXED(psa_l2r_reset_ind)
  #define psa_l2r_statistic_ind     _ENTITY_PREFIXED(psa_l2r_statistic_ind)
  #define psa_l2r_dti_cnf           _ENTITY_PREFIXED(psa_l2r_dti_cnf)
  #define psa_l2r_dti_ind           _ENTITY_PREFIXED(psa_l2r_dti_ind)
  #define psa_tra_activate_cnf      _ENTITY_PREFIXED(psa_tra_activate_cnf)
  #define psa_tra_deactivate_cnf    _ENTITY_PREFIXED(psa_tra_deactivate_cnf)
  #define psa_tra_dti_cnf           _ENTITY_PREFIXED(psa_tra_dti_cnf)
  #define psa_tra_dti_ind           _ENTITY_PREFIXED(psa_tra_dti_ind)

#ifdef UART
/* UART */
  #define psa_uart_parameters_cnf    _ENTITY_PREFIXED(psa_uart_parameters_cnf)
  #define psa_uart_parameters_ind    _ENTITY_PREFIXED(psa_uart_parameters_ind)
  #define psa_uart_dti_cnf           _ENTITY_PREFIXED(psa_uart_dti_cnf)
  #define psa_uart_dti_ind           _ENTITY_PREFIXED(psa_uart_dti_ind)
  #define psa_uart_disable_cnf           _ENTITY_PREFIXED(psa_uart_disable_cnf)
  #define psa_uart_ring_cnf              _ENTITY_PREFIXED(psa_uart_ring_cnf)
  #define psa_uart_dcd_cnf               _ENTITY_PREFIXED(psa_uart_dcd_cnf)
  #define psa_uart_break_cnf             _ENTITY_PREFIXED(psa_uart_break_cnf)
  #define psa_uart_break_ind             _ENTITY_PREFIXED(psa_uart_break_ind)
  #define psa_uart_error_ind             _ENTITY_PREFIXED(psa_uart_error_ind)
  #define psa_uart_mux_start_cnf         _ENTITY_PREFIXED(psa_uart_mux_start_cnf)
  #define psa_uart_mux_dlc_establish_ind _ENTITY_PREFIXED(psa_uart_mux_dlc_establish_ind)
  #define psa_uart_mux_dlc_release_ind   _ENTITY_PREFIXED(psa_uart_mux_dlc_release_ind)
  #define psa_uart_mux_close_ind         _ENTITY_PREFIXED(psa_uart_mux_close_ind)
#endif

#if defined(FF_TCP_IP) || defined(FF_ESIM)
/* AAA */
  #define psa_aaa_cmd_req              _ENTITY_PREFIXED(psa_aaa_cmd_req)
  #define psa_aaa_open_port_req      _ENTITY_PREFIXED(psa_aaa_open_port_req)
  #define psa_aaa_close_port_req       _ENTITY_PREFIXED(psa_aaa_close_port_req)
  #define psa_aaa_dti_rsp            _ENTITY_PREFIXED(psa_aaa_dti_rsp)
  #define psa_aaa_disconnect_rsp     _ENTITY_PREFIXED(psa_aaa_disconnect_rsp)
#endif

#ifdef GPRS
/* GMMREG */
  #define psa_gmmreg_attach_cnf     _ENTITY_PREFIXED(psa_gmmreg_attach_cnf)
  #define psa_gmmreg_attach_rej     _ENTITY_PREFIXED(psa_gmmreg_attach_rej)
  #define psa_gmmreg_detach_cnf     _ENTITY_PREFIXED(psa_gmmreg_detach_cnf)
  #define psa_gmmreg_detach_ind     _ENTITY_PREFIXED(psa_gmmreg_detach_ind)
  #define psa_gmmreg_plmn_ind       _ENTITY_PREFIXED(psa_gmmreg_plmn_ind)
  #define psa_gmmreg_suspend_ind    _ENTITY_PREFIXED(psa_gmmreg_suspend_ind)
  #define psa_gmmreg_resume_ind     _ENTITY_PREFIXED(psa_gmmreg_resume_ind)
  #define psa_gmmreg_info_ind       _ENTITY_PREFIXED(psa_gmmreg_info_ind)
  #define psa_gmmreg_ciphering_ind  _ENTITY_PREFIXED(psa_gmmreg_ciphering_ind)
  #define psa_gmmreg_ahplmn_ind     _ENTITY_PREFIXED(psa_gmmreg_ahplmn_ind)

/* SMREG */
  #define psa_smreg_pdp_activate_cnf    _ENTITY_PREFIXED(psa_smreg_pdp_activate_cnf)
  #define psa_smreg_pdp_activate_rej    _ENTITY_PREFIXED(psa_smreg_pdp_activate_rej)
  #define psa_smreg_pdp_activate_ind    _ENTITY_PREFIXED(psa_smreg_pdp_activate_ind)
  #define psa_smreg_pdp_deactivate_cnf  _ENTITY_PREFIXED(psa_smreg_pdp_deactivate_cnf)
  #define psa_smreg_pdp_deactivate_ind  _ENTITY_PREFIXED(psa_smreg_pdp_deactivate_ind)
  #define psa_smreg_pdp_modify_ind      _ENTITY_PREFIXED(psa_smreg_pdp_modify_ind)
  #ifdef REL99
  #define psa_smreg_pdp_modify_cnf       _ENTITY_PREFIXED(psa_smreg_pdp_modify_cnf)
  #define psa_smreg_pdp_modify_rej       _ENTITY_PREFIXED(psa_smreg_pdp_modify_rej)
  #define psa_smreg_pdp_activate_sec_cnf _ENTITY_PREFIXED(psa_smreg_pdp_activate_sec_cnf)
  #define psa_smreg_pdp_activate_sec_rej _ENTITY_PREFIXED(psa_smreg_pdp_activate_sec_rej)
  #endif


/* SN */
  #define psa_sn_dti_cnf            _ENTITY_PREFIXED(psa_sn_dti_cnf)

#ifdef FF_PKTIO
  #define psa_pkt_connect_ind         _ENTITY_PREFIXED(psa_pkt_connect_ind)
  #define psa_pkt_disconnect_ind      _ENTITY_PREFIXED(psa_pkt_disconnect_ind)
  #define psa_pkt_dti_open_cnf        _ENTITY_PREFIXED(psa_pkt_dti_open_cnf)
  #define psa_pkt_modify_cnf          _ENTITY_PREFIXED(psa_pkt_modify_cnf)
  #define psa_pkt_dti_close_cnf       _ENTITY_PREFIXED(psa_pkt_dti_close_cnf)
  #define psa_pkt_dti_close_ind       _ENTITY_PREFIXED(psa_pkt_dti_close_ind)
#endif

#endif  /* GPRS */

#ifdef FF_PSI
  #define psa_psi_conn_ind         _ENTITY_PREFIXED(psa_psi_conn_ind)
  #define psa_psi_disconn_ind      _ENTITY_PREFIXED(psa_psi_disconn_ind)
  #define psa_psi_close_cnf      _ENTITY_PREFIXED(psa_psi_close_cnf)
  #define psa_psi_setconf_cnf   _ENTITY_PREFIXED(psa_psi_setconf_cnf)
  #define psa_psi_dti_open_cnf        _ENTITY_PREFIXED(psa_psi_dti_open_cnf)
  #define psa_psi_dti_close_cnf       _ENTITY_PREFIXED(psa_psi_dti_close_cnf)
  #define psa_psi_dti_close_ind       _ENTITY_PREFIXED(psa_psi_dti_close_ind)
  #define psa_psi_line_state_cnf       _ENTITY_PREFIXED(psa_psi_line_state_cnf)
  #define psa_psi_line_state_ind      _ENTITY_PREFIXED (psa_psi_line_state_ind)
#ifdef _SIMULATION_
  #define psa_psi_conn_ind_test       _ENTITY_PREFIXED(psa_psi_conn_ind_test)
#endif /* _SIMULATION_ */
#endif /*FF_PSI*/

/* PPP */
#if defined (FF_WAP) || defined (GPRS) || defined (FF_PPP) || defined(FF_GPF_TCPIP) || defined (FF_SAT_E)
  #define psa_ppp_establish_cnf         _ENTITY_PREFIXED(psa_ppp_establish_cnf)
  #define psa_ppp_terminate_ind         _ENTITY_PREFIXED(psa_ppp_terminate_ind)
  #define psa_ppp_pdp_activate_ind      _ENTITY_PREFIXED(psa_ppp_pdp_activate_ind)
  #define psa_ppp_modification_cnf      _ENTITY_PREFIXED(psa_ppp_modification_cnf)
  #define psa_ppp_dti_connected_ind     _ENTITY_PREFIXED(psa_ppp_dti_connected_ind)
#endif /* FF_WAP OR GPRS OR FF_PPP OR FF_GPF_TCPIP OR (FF_SAT_E)*/

#if defined FF_EOTD
  #define psa_mnlc_sms_meas_cnf           _ENTITY_PREFIXED(psa_mnlc_sms_meas_cnf)
#endif /* FF_EOTD */

#endif /* OPTIO_MULTITHREAD */

EXTERN void psa_sim_read_cnf          ( T_SIM_READ_CNF * );
EXTERN void psa_sim_update_cnf        ( T_SIM_UPDATE_CNF * );
EXTERN void psa_sim_read_record_cnf   ( T_SIM_READ_RECORD_CNF * );
EXTERN void psa_sim_update_record_cnf ( T_SIM_UPDATE_RECORD_CNF * );
EXTERN void psa_sim_increment_cnf     ( T_SIM_INCREMENT_CNF * );
EXTERN void psa_sim_verify_pin_cnf    ( T_SIM_VERIFY_PIN_CNF * );
EXTERN void psa_sim_change_pin_cnf    ( T_SIM_CHANGE_PIN_CNF * );
EXTERN void psa_sim_disable_pin_cnf   ( T_SIM_DISABLE_PIN_CNF * );
EXTERN void psa_sim_enable_pin_cnf    ( T_SIM_ENABLE_PIN_CNF * );
EXTERN void psa_sim_unblock_cnf       ( T_SIM_UNBLOCK_CNF * );
EXTERN void psa_sim_sync_cnf          ( T_SIM_SYNC_CNF * );
EXTERN void psa_sim_activate_cnf      ( T_SIM_ACTIVATE_CNF * );
EXTERN void psa_sim_activate_ind      ( T_SIM_ACTIVATE_IND * );
EXTERN void psa_sim_access_cnf        ( T_SIM_ACCESS_CNF * );
#ifdef SIM_TOOLKIT
EXTERN void psa_sim_toolkit_ind       ( T_SIM_TOOLKIT_IND * );
EXTERN void psa_sim_toolkit_cnf       ( T_SIM_TOOLKIT_CNF * );
EXTERN void psa_sim_file_update_ind   ( T_SIM_FILE_UPDATE_IND * );
#ifdef FF_SAT_E
#ifdef UART
EXTERN void psa_sim_bip_config_cnf    ( T_SIM_BIP_CONFIG_CNF * );
EXTERN void psa_sim_bip_cnf           ( T_SIM_BIP_CNF * );
EXTERN void psa_sim_dti_cnf           ( T_SIM_DTI_CNF * );
EXTERN void psa_sim_dti_bip_ind       ( T_SIM_DTI_BIP_IND * );
#endif /* UART */
#endif /* FF_SAT_E */
EXTERN void psa_sim_eventlist_cnf ( T_SIM_EVENTLIST_CNF *sim_eventlist_cnf );
#endif /* SIM_TOOKIT */
EXTERN void psa_sim_mmi_insert_ind    ( T_SIM_MMI_INSERT_IND * );
EXTERN void psa_sim_remove_ind        ( T_SIM_REMOVE_IND * );

EXTERN void psa_mmr_reg_cnf           ( T_MMR_REG_CNF  * );
#ifdef FF_EM_MODE
EXTERN void psa_em_sc_info_cnf          ( T_EM_SC_INFO_CNF  * );
EXTERN void psa_em_sc_gprs_info_cnf     ( T_EM_SC_GPRS_INFO_CNF  * );
EXTERN void psa_em_nc_info_cnf          ( T_EM_NC_INFO_CNF  * );
EXTERN void psa_em_loc_pag_info_cnf     ( T_EM_LOC_PAG_INFO_CNF  * );
EXTERN void psa_em_plmn_info_cnf        ( T_EM_PLMN_INFO_CNF  * );
EXTERN void psa_em_cip_hop_dtx_info_cnf ( T_EM_CIP_HOP_DTX_INFO_CNF  * );
EXTERN void psa_em_power_info_cnf       ( T_EM_POWER_INFO_CNF  * );
EXTERN void psa_em_identity_info_cnf    ( T_EM_IDENTITY_INFO_CNF  * );
EXTERN void psa_em_sw_version_info_cnf  ( T_EM_SW_VERSION_INFO_CNF  * );
EXTERN void em_event_trace_ind          ( T_EM_DATA_IND * em_data_ind);
EXTERN void psa_em_gmm_info_cnf         ( T_EM_GMM_INFO_CNF  * );
EXTERN void psa_em_grlc_info_cnf        ( T_EM_GRLC_INFO_CNF  * );
EXTERN void psa_em_amr_info_cnf         ( T_EM_AMR_INFO_CNF  * );
#endif /* FF_EM_MODE */
EXTERN void psa_mmr_nreg_cnf          ( T_MMR_NREG_CNF * );
EXTERN void psa_mmr_nreg_ind          ( T_MMR_NREG_IND * );
EXTERN void psa_mmr_plmn_ind          ( T_MMR_PLMN_IND * );
EXTERN void psa_mmr_info_ind          ( T_MMR_INFO_IND * );
EXTERN void psa_mmr_ciphering_ind     ( T_MMR_CIPHERING_IND * );
EXTERN void psa_mmr_ahplmn_ind        ( T_MMR_AHPLMN_IND * );

EXTERN void psa_mncc_setup_ind        ( T_MNCC_SETUP_IND * );
EXTERN void psa_mncc_setup_cnf        ( T_MNCC_SETUP_CNF * );
EXTERN void psa_mncc_setup_compl_ind  ( T_MNCC_SETUP_COMPL_IND * );
EXTERN void psa_mncc_reject_ind       ( T_MNCC_REJECT_IND * );
EXTERN void psa_mncc_release_ind      ( T_MNCC_RELEASE_IND * );
EXTERN void psa_mncc_release_cnf      ( T_MNCC_RELEASE_CNF * );
EXTERN void psa_mncc_disconnect_ind   ( T_MNCC_DISCONNECT_IND * );
EXTERN void psa_mncc_alert_ind        ( T_MNCC_ALERT_IND * );
EXTERN void psa_mncc_call_proceed_ind ( T_MNCC_CALL_PROCEED_IND * );
EXTERN void psa_mncc_progress_ind     ( T_MNCC_PROGRESS_IND * );
EXTERN void psa_mncc_hold_cnf         ( T_MNCC_HOLD_CNF * );
EXTERN void psa_mncc_retrieve_cnf     ( T_MNCC_RETRIEVE_CNF * );
EXTERN void psa_mncc_sync_ind         ( T_MNCC_SYNC_IND * );
EXTERN void psa_mncc_user_ind         ( T_MNCC_USER_IND * );
EXTERN void psa_mncc_start_dtmf_cnf   ( T_MNCC_START_DTMF_CNF * );
EXTERN void psa_mncc_facility_ind     ( T_MNCC_FACILITY_IND * );
EXTERN void psa_mncc_modify_ind       ( T_MNCC_MODIFY_IND * );
EXTERN void psa_mncc_modify_cnf       ( T_MNCC_MODIFY_CNF * );
EXTERN void psa_mncc_bearer_cap_cnf   ( T_MNCC_BEARER_CAP_CNF * );
EXTERN void psa_mncc_prompt_ind       ( T_MNCC_PROMPT_IND * );
EXTERN void psa_mncc_recall_ind       ( T_MNCC_RECALL_IND * );
EXTERN void psa_mncc_status_ind       ( T_MNCC_STATUS_IND * );

#if defined FF_EOTD
EXTERN void psa_mnlc_sms_meas_cnf       ( T_MNLC_SMS_MEAS_CNF * );
#endif

EXTERN void psa_mnss_begin_ind        (T_MNSS_BEGIN_IND *);
EXTERN void psa_mnss_facility_ind     (T_MNSS_FACILITY_IND *);
EXTERN void psa_mnss_end_ind          (T_MNSS_END_IND *);

EXTERN void psa_mnsms_delete_cnf      (T_MNSMS_DELETE_CNF *);
EXTERN void psa_mnsms_read_cnf        (T_MNSMS_READ_CNF *);
EXTERN void psa_mnsms_store_cnf       (T_MNSMS_STORE_CNF *);
EXTERN void psa_mnsms_submit_cnf      (T_MNSMS_SUBMIT_CNF *);
EXTERN void psa_mnsms_command_cnf     (T_MNSMS_COMMAND_CNF *);
EXTERN void psa_mnsms_report_ind      (T_MNSMS_REPORT_IND *);
EXTERN void psa_mnsms_status_ind      (T_MNSMS_STATUS_IND *);
EXTERN void psa_mnsms_message_ind     (T_MNSMS_MESSAGE_IND *);
EXTERN void psa_mnsms_error_ind       (T_MNSMS_ERROR_IND *);
#ifdef TI_PS_FF_AT_P_CMD_CPRSM
EXTERN void psa_mnsms_resume_cnf      (T_MNSMS_RESUME_CNF *);
EXTERN void psa_mnsms_query_cnf       (T_MNSMS_QUERY_CNF *);
#endif /* TI_PS_FF_AT_P_CMD_CPRSM */
#ifdef REL99
EXTERN void psa_mnsms_send_prog_ind   ( T_MNSMS_SEND_PROG_IND *);
EXTERN void psa_mnsms_retrans_cnf     ( T_MNSMS_RETRANS_CNF * );
#endif /* REL99 */
#ifdef GPRS
  EXTERN void psa_mnsms_mo_serv_cnf   (T_MNSMS_MO_SERV_CNF *);
#endif  /* GPRS */
EXTERN void psa_mnsms_OTA_message_ind ( T_MNSMS_OTA_MESSAGE_IND * );

EXTERN void psa_mmi_keypad_ind        (T_MMI_KEYPAD_IND *);
EXTERN void psa_mmi_rxlev_ind         (T_MMI_RXLEV_IND *);
EXTERN void psa_mmi_battery_ind       (T_MMI_BATTERY_IND *);
EXTERN void psa_mmi_cbch_ind          (T_MMI_CBCH_IND *);
#ifdef BTE_MOBILE
EXTERN void psa_mmi_bt_cb_notify_ind        (T_MMI_BT_CB_NOTIFY_IND *);
#endif
#ifndef VOCODER_FUNC_INTERFACE
EXTERN void psa_mmi_tch_vocoder_cfg_con(T_MMI_TCH_VOCODER_CFG_CON *);
#endif

#ifdef SIM_TOOLKIT
EXTERN void psa_sat_cbch_dnl_ind      (T_MMI_SAT_CBCH_DWNLD_IND *);
#endif

#ifdef FAX_AND_DATA
EXTERN void psa_ra_activate_cnf       (T_RA_ACTIVATE_CNF *);
EXTERN void psa_ra_deactivate_cnf     (T_RA_DEACTIVATE_CNF *);

#ifdef FF_FAX
EXTERN void psa_ra_modify_cnf         (T_RA_MODIFY_CNF *);

EXTERN void psa_t30_cap_ind           (T_T30_CAP_IND *);
EXTERN void psa_t30_dti_cnf           (T_T30_DTI_CNF *);
EXTERN void psa_t30_dti_ind           (T_T30_DTI_IND *);
EXTERN void psa_t30_phase_ind         (T_T30_PHASE_IND *);
EXTERN void psa_t30_sgn_ind           (T_T30_SGN_IND *);
EXTERN void psa_t30_cmpl_ind          (T_T30_CMPL_IND *);
EXTERN void psa_t30_report_ind        (T_T30_REPORT_IND *);
EXTERN void psa_t30_error_ind         (T_T30_ERROR_IND *);
EXTERN void psa_t30_eol_ind           (T_T30_EOL_IND *);
EXTERN void psa_t30_deactivate_cnf    (T_T30_DEACTIVATE_CNF *);
EXTERN void psa_t30_activate_cnf      (T_T30_ACTIVATE_CNF *);
EXTERN void psa_t30_preamble_ind      (T_T30_PREAMBLE_IND *);
#ifdef REL99
EXTERN void psa_mnsms_send_prog_ind   ( T_MNSMS_SEND_PROG_IND *);
EXTERN void psa_mnsms_retrans_cnf     ( T_MNSMS_RETRANS_CNF * );
#endif
#endif /* FF_FAX */

#ifdef BT_ADAPTER
EXTERN void psa_btp_dti_ind( T_BTP_DTI_IND *);
EXTERN void psa_btp_dti_cnf( T_BTP_DTI_IND *);
#endif /* BT_ADAPTER */

EXTERN void psa_l2r_activate_cnf      (T_L2R_ACTIVATE_CNF   *);
EXTERN void psa_l2r_deactivate_cnf    (T_L2R_DEACTIVATE_CNF *);
EXTERN void psa_l2r_connect_cnf       (T_L2R_CONNECT_CNF *);
EXTERN void psa_l2r_connect_ind       (T_L2R_CONNECT_IND *);
EXTERN void psa_l2r_disc_cnf          (T_L2R_DISC_CNF *);
EXTERN void psa_l2r_disc_ind          (T_L2R_DISC_IND *);
EXTERN void psa_l2r_xid_ind           (T_L2R_XID_IND *);
EXTERN void psa_l2r_error_ind         (T_L2R_ERROR_IND *);
EXTERN void psa_l2r_reset_ind         (T_L2R_RESET_IND *);
EXTERN void psa_l2r_statistic_ind     (T_L2R_STATISTIC_IND *);
EXTERN void psa_l2r_dti_cnf           (T_L2R_DTI_CNF *);
EXTERN void psa_l2r_dti_ind           (T_L2R_DTI_IND *);

EXTERN void psa_tra_activate_cnf      (T_TRA_ACTIVATE_CNF   *);
EXTERN void psa_tra_deactivate_cnf    (T_TRA_DEACTIVATE_CNF *);
EXTERN void psa_tra_dti_cnf           (T_TRA_DTI_CNF *);
EXTERN void psa_tra_dti_ind           (T_TRA_DTI_IND *);
#endif /* FAX_AND_DATA */

#ifdef FF_GPF_TCPIP
EXTERN void psa_tcpip_dti_cnf          (T_TCPIP_DTI_CNF    *);
EXTERN void psa_tcpip_ifconfig_cnf     (T_TCPIP_IFCONFIG_CNF *);
EXTERN void psa_tcpip_initialize_cnf     (T_TCPIP_INITIALIZE_CNF *);
EXTERN void psa_tcpip_shutdown_cnf  (T_TCPIP_SHUTDOWN_CNF * );
#endif

#ifdef CO_UDP_IP
EXTERN void psa_udpa_dti_cnf           (T_UDPA_DTI_CNF    *);
EXTERN void psa_udpa_dti_ind           (T_UDPA_DTI_IND    *);
EXTERN void psa_udpa_config_cnf        (T_UDPA_CONFIG_CNF *);
EXTERN void psa_ipa_dti_cnf            (T_IPA_DTI_CNF     *);
EXTERN void psa_ipa_dti_ind            (T_IPA_DTI_IND     *);
EXTERN void psa_ipa_config_cnf         (T_IPA_CONFIG_CNF  *);
#endif

#ifdef FF_WAP
EXTERN void psa_wap_mmi_ind   (T_WAP_MMI_IND *);
EXTERN void psa_wap_mmi_req   (T_WAP_MMI_REQ *);
EXTERN void psa_wap_mmi_cnf   (T_WAP_MMI_CNF *);
EXTERN void psa_wap_dti_cnf   (T_WAP_DTI_CNF *);
EXTERN void psa_wap_dti_ind   (T_WAP_DTI_IND *);
#endif

#ifdef UART
/* UART */
  EXTERN void psa_uart_parameters_cnf       (T_UART_PARAMETERS_CNF   *);
  EXTERN void psa_uart_parameters_ind       (T_UART_PARAMETERS_IND   *);
  EXTERN void psa_uart_escape_cnf           (T_UART_ESCAPE_CNF       *);
  EXTERN void psa_uart_dti_cnf              (T_UART_DTI_CNF          *);
  EXTERN void psa_uart_dti_ind              (T_UART_DTI_IND          *);
  EXTERN void psa_uart_detected_ind         (T_UART_DETECTED_IND     *);
  EXTERN void psa_uart_disable_cnf          (T_UART_DISABLE_CNF      *);
  EXTERN void psa_uart_ring_cnf             (T_UART_RING_CNF         *);
  EXTERN void psa_uart_dcd_cnf              (T_UART_DCD_CNF          *);
  EXTERN void psa_uart_mux_start_cnf        (T_UART_MUX_START_CNF    *);
  EXTERN void psa_uart_error_ind            (T_UART_ERROR_IND        *);
  EXTERN void psa_uart_mux_dlc_establish_ind   (T_UART_MUX_DLC_ESTABLISH_IND *);
  EXTERN void psa_uart_mux_dlc_release_ind  (T_UART_MUX_DLC_RELEASE_IND *);
  EXTERN void psa_uart_mux_close_ind        (T_UART_MUX_CLOSE_IND    *);
#endif

#if defined(FF_TCP_IP) || defined (FF_ESIM)
/* AAA */
  EXTERN void  psa_aaa_cmd_req            (T_AAA_CMD_REQ         *);
  EXTERN void  psa_aaa_open_port_req    (T_AAA_OPEN_PORT_REQ   *);
  EXTERN void  psa_aaa_close_port_req   (T_AAA_CLOSE_PORT_REQ  *);
  EXTERN void  psa_aaa_dti_rsp          (T_AAA_DTI_RES         *);
  EXTERN void  psa_aaa_disconnect_rsp   (T_AAA_DISCONNECT_RES  *);
#endif

#ifdef GPRS
/* GMMREG */
  EXTERN void psa_gmmreg_attach_cnf         (T_GMMREG_ATTACH_CNF    *);
  EXTERN void psa_gmmreg_attach_rej         (T_GMMREG_ATTACH_REJ    *);
  EXTERN void psa_gmmreg_detach_cnf         (T_GMMREG_DETACH_CNF    *);
  EXTERN void psa_gmmreg_detach_ind         (T_GMMREG_DETACH_IND    *);
  EXTERN void psa_gmmreg_plmn_ind           (T_GMMREG_PLMN_IND      *);
  EXTERN void psa_gmmreg_suspend_ind        (T_GMMREG_SUSPEND_IND   *);
  EXTERN void psa_gmmreg_resume_ind         (T_GMMREG_RESUME_IND    *);
  EXTERN void psa_gmmreg_info_ind           (T_GMMREG_INFO_IND      *);
  EXTERN void psa_gmmreg_ciphering_ind      (T_GMMREG_CIPHERING_IND *);
  EXTERN void psa_gmmreg_ahplmn_ind         (T_GMMREG_AHPLMN_IND    *);

/* SMREG */
  EXTERN void psa_smreg_pdp_activate_cnf    (T_SMREG_PDP_ACTIVATE_CNF   *);
  EXTERN void psa_smreg_pdp_activate_rej    (T_SMREG_PDP_ACTIVATE_REJ   *);
  EXTERN void psa_smreg_pdp_activate_ind    (T_SMREG_PDP_ACTIVATE_IND   *);
  EXTERN void psa_smreg_pdp_deactivate_cnf  (T_SMREG_PDP_DEACTIVATE_CNF *);
  EXTERN void psa_smreg_pdp_deactivate_ind  (T_SMREG_PDP_DEACTIVATE_IND *);
  EXTERN void psa_smreg_pdp_modify_ind      (T_SMREG_PDP_MODIFY_IND     *);

 #ifdef REL99
 EXTERN void psa_smreg_pdp_modify_cnf       (T_SMREG_PDP_MODIFY_CNF *);
 EXTERN void psa_smreg_pdp_modify_rej       (T_SMREG_PDP_MODIFY_REJ *);
 EXTERN void psa_smreg_pdp_activate_sec_cnf (T_SMREG_PDP_ACTIVATE_SEC_CNF *);
 EXTERN void psa_smreg_pdp_activate_sec_rej (T_SMREG_PDP_ACTIVATE_SEC_REJ *); 
 #endif


#ifdef FF_PKTIO
  EXTERN void psa_pkt_connect_ind    ( T_PKT_CONNECT_IND *);
  EXTERN void psa_pkt_disconnect_ind ( T_PKT_DISCONNECT_IND *);
  EXTERN void psa_pkt_dti_close_cnf  ( T_PKT_DTI_CLOSE_CNF *);
  EXTERN void psa_pkt_dti_close_ind  ( T_PKT_DTI_CLOSE_IND *);
  EXTERN void psa_pkt_dti_open_cnf   ( T_PKT_DTI_OPEN_CNF *);
  EXTERN void psa_pkt_modify_cnf     ( T_PKT_MODIFY_CNF *);
#endif
#endif  /* GPRS */

#ifdef FF_PSI
  EXTERN void psa_psi_conn_ind    ( T_PSI_CONN_IND *);
  EXTERN void psa_psi_disconn_ind ( T_PSI_DISCONN_IND *);
  EXTERN void psa_psi_dti_close_cnf  ( T_PSI_DTI_CLOSE_CNF *);
  EXTERN void psa_psi_dti_close_ind  ( T_PSI_DTI_CLOSE_IND *);
  EXTERN void psa_psi_dti_open_cnf   ( T_PSI_DTI_OPEN_CNF *);
  EXTERN void psa_psi_close_cnf  (T_PSI_CLOSE_CNF *);
  EXTERN void psa_psi_setconf_cnf (T_PSI_SETCONF_CNF *);
  EXTERN void psa_psi_line_state_cnf  (T_PSI_LINE_STATE_CNF *);
  EXTERN void psa_psi_line_state_ind  (T_PSI_LINE_STATE_IND *);


/* SN */
EXTERN void psa_sn_dti_cnf      (T_SN_DTI_CNF    *);
#ifdef _SIMULATION_
  EXTERN  const void psa_psi_conn_ind_test   ( T_PSI_CONN_IND_TEST *);
#endif /* _SIMULATION_ */
#endif /*FF_PSI*/

/* PPP */
#if defined (FF_WAP) || defined (GPRS) || defined (FF_PPP) || defined(FF_GPF_TCPIP) || defined (FF_SAT_E) 
  EXTERN void psa_ppp_establish_cnf         (T_PPP_ESTABLISH_CNF     *);
  EXTERN void psa_ppp_terminate_ind         (T_PPP_TERMINATE_IND     *);
  EXTERN void psa_ppp_dti_connected_ind     (T_PPP_DTI_CONNECTED_IND *);
  EXTERN void psa_ppp_pdp_activate_ind      (T_PPP_PDP_ACTIVATE_IND  *);
  EXTERN void psa_ppp_modification_cnf      (T_PPP_MODIFICATION_CNF  *);
#endif /* FF_WAP or GPRS OR FF_PPP OR FF_GPF_TCPIP OR || (FF_SAT_E) */

/*DCM*/
#ifdef FF_GPF_TCPIP
  EXTERN void psa_dcm_open_conn_req(T_DCM_OPEN_CONN_REQ    *);
  EXTERN void psa_dcm_close_conn_req(T_DCM_CLOSE_CONN_REQ  *);
  EXTERN void psa_dcm_get_current_conn_req(T_DCM_GET_CURRENT_CONN_REQ *);
#endif

#if defined FF_EOTD
EXTERN void psa_mnlc_sms_meas_cnf           (T_MNLC_SMS_MEAS_CNF *);
#endif /* FF_EOTD */
/*
 * UTILITY module
 */

/*
 * Prototypes Customer Specific Functions
 */

/*
 * If all entities are linked into one module this definitions
 * prefixes all this functions with the enity name
 */
#ifdef TI_PS_HCOMM_CHANGE
#define PSENDX(A,B) PSEND(_hComm##A,B)
#else
#define PSENDX(A,B) PSEND(hComm##A,B)
#endif /* TI_PS_HCOMM_CHANGE */

/* Implements Measure#32: Row 89, 90, 116, 117, 1241 & 1242 */
EXTERN const char * const ksd_supp_clir_str;
EXTERN const char * const ksd_inv_clir_str;

#endif
