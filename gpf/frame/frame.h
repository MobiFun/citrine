/* 
+------------------------------------------------------------------------------
|  File:       frame.h
+------------------------------------------------------------------------------
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
|  Purpose :  Definitions for the frame.
+----------------------------------------------------------------------------- 
*/ 

#ifndef FRAME_H
#define FRAME_H

/*==== INCLUDES =============================================================*/


/*==== TYPES ================================================================*/

typedef enum { TST_ADR, RCV_ADR, END_OF_COMP_TABLE } T_COMPONENT_ID;

typedef struct 
{
  void *RcvAdr;
  void *TstAdr;
  void *DrvListAdr;
  void *InitFuncAdr;
  UBYTE *FrameEnvAdr;
} T_CONFIGURATION_ADDRESS;

/*==== CONSTANTS ============================================================*/

#define PF_OK     0
#define PF_ERROR  -1

#define VSI_CALLER                   0,

#ifdef __FRAME_C__
  #ifndef RUN_INT_RAM
    char FRM_TST_NAME[RESOURCE_NAMELEN]  = { 'T','S','T',0x0,0x0,0x0,0x0,0x0 };
    char FRM_RCV_NAME[RESOURCE_NAMELEN]  = { 'R','C','V',0x0,0x0,0x0,0x0,0x0 };
    char FRM_SYST_NAME[RESOURCE_NAMELEN] = { 'S','Y','S','T',0x0,0x0,0x0,0x0 };
    char FRM_PCO_NAME [RESOURCE_NAMELEN] = { 'P','C','O',0x0,0x0,0x0,0x0,0x0 };
    char const *syst_wrn = "SYSTEM WARNING:";
  #else
    extern char FRM_TST_NAME[];
    extern char FRM_RCV_NAME[];
    extern char FRM_SYST_NAME[];
    extern char FRM_PCO_NAME[];
  #endif
#else
    extern char const *syst_wrn;
#endif

/*
 * Tokens of the system primitives
 */
#define SYSPRIM_REDIRECT_TOKEN   "REDIRECT"
#define SYSPRIM_CONNECT_TOKEN    "CONNECT"
#define SYSPRIM_DISCONNECT_TOKEN "DISCONNECT"
#define SYSPRIM_DUPLICATE_TOKEN  "DUPLICATE"
#define SYSPRIM_CONFIG_TOKEN     "CONFIG"
#define SYSPRIM_MEMCHECK_TOKEN   "MEMCHECK"
#define SYSPRIM_RESET_TOKEN      "RESET"
#define SYSPRIM_VERSION_TOKEN    "VERSION"
#define SYSPRIM_CLEAR_TOKEN      "CLEAR"
#define SYSPRIM_NULL_TOKEN       "NULL"
#define SYSPRIM_TRACECLASS_TOKEN "TRACECLASS"
#define SYSPRIM_DISPLAY_TOKEN    "DISPLAY"
#define SYSPRIM_BOOT_TOKEN       "BOOT"
#define SYSPRIM_SHOW_MEMORY      "MEMORY"
#define SYSPRIM_TRC_SUSPEND      "TRCSUSPEND"
#define SYSPRIM_READ_ROUTING     "ROUTING"
#define SYSPRIM_STR2IND_VERSION  "STR2INDVERSION"
#define SYSPRIM_EXIT_TOKEN       "EXIT"
#define SYSPRIM_REGISTER_TOKEN   "REGISTER"
#define SYSPRIM_WITHDRAW_TOKEN   "WITHDRAW"
#define SYSPRIM_STATUS_TOKEN     "STATUS"
#define SYSPRIM_SUPPRESS_OK      "SUPPRESS_OK"
#define SYSPRIM_GET_STACK_TIME   "GET_STACK_TIME"
#define SYSPRIM_IS_STACK_TIME    "IS_STACK_TIME"
#define SYSPRIM_READ_FFS_DAR     "READ_DAR_FILE"
#define SYSPRIM_SELECT_TIME_TDMA "TIME_TDMA"
#define SYSPRIM_CHECK_OWNER      "PPM_CHECK_OWNER"
#define SYSPRIM_TST_SYNC_REQ     "TST_SYNC_REQ"
#define SYSPRIM_TST_SYNC_CNF     "TST_SYNC_CNF"
#define SYSPRIM_TST_SYNC_REJ     "TST_SYNC_REJ"
#define SYSPRIM_ROUTE_DESCLIST   "ROUTE_DESCLIST"
#define SYSPRIM_READ_COM_MATRIX  "READ_COM_MATRIX"
#define SYSPRIM_ISOLATE_TOKEN    "ISOLATE"
#define SYSPRIM_REGISTER_ERR_IND "REG_ERROR_IND"
#define SYSPRIM_WITHDRAW_ERR_IND "WITHDRAW_ERROR_IND"
#define SYSPRIM_CHECK_DESCLIST   "CHECK_DESCLIST"
#define SYSPRIM_PCHECK           "PCHECK"


#define PERIODIC_TIMER      0x8000
#define TIMEOUT_OCCURRED    0x4000
#define TIMER_HANDLE_MASK  (~(PERIODIC_TIMER|TIMEOUT_OCCURRED))

/*
 * message prioritiey
 */
#define MSG_PRIMITIVE_PRIO   OS_NORMAL
#define MSG_SIGNAL_PRIO      OS_URGENT
#define MSG_TRACE_PRIO       OS_NORMAL
/*
 * length for traces
 */
#define ITRACE_LEN            (2*sizeof(USHORT)+1)
#define PTRACE_LEN_OPC16      18
#define PTRACE_LEN_OPC32      22
#define STRACE_LEN            80

#define TRACE_TEXT_SIZE    	  (sizeof(T_S_HEADER)+TTRACE_LEN)
#define TRACE_INDEX_SIZE      (sizeof(T_S_HEADER)+ITRACE_LEN)
#define TRACE_PRIM_SIZE       (sizeof(T_S_HEADER)+PTRACE_LEN_OPC32)
#define TRACE_STATE_SIZE    	(sizeof(T_S_HEADER)+STRACE_LEN)

/*==== PROTOTYPES ===========================================================*/

GLOBAL void pf_Init (T_CONFIGURATION_ADDRESS *ConfigAddress);
GLOBAL SHORT pf_CreateAllEntities (void);
GLOBAL SHORT pf_StartAllTasks ( void );
GLOBAL void pf_Timeout (T_HANDLE TaskHandle, T_HANDLE EntityHandle, USHORT TimerIndex );
GLOBAL void pf_ProcessSystemPrim ( T_HANDLE TaskHandle, T_VOID_STRUCT *pPrim);
GLOBAL int pf_handle_warning ( USHORT cause, const char * const format,...);
GLOBAL void InitializeTimer   (void);
GLOBAL void InitializeTrace   (void);
GLOBAL void TracePoolstatus   (T_HANDLE Caller );
GLOBAL void InitializePPM     (void);
GLOBAL void InitializeDriverConfig (void);

#ifdef _TOOLS_
extern USHORT pf_get_frameenv (void);
void set_stack_time (ULONG time);
void get_local_time (ULONG *time);
#endif /* _TOOLS_ */

#endif /* FRAME_H */               
