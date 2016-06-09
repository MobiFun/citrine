/* 
+------------------------------------------------------------------------------
|  File:       vsi.h
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
|  Purpose :  Definitions for Virtual System Interface.
+----------------------------------------------------------------------------- 
*/ 

#ifndef VSI_H
#define VSI_H

/*==== INCLUDES =============================================================*/

#include <stddef.h>
#include "gpfconf.h"
#include "header.h"
#include "gdi.h"
#include "drvconf.h"
#include "prf_func.h"

/*==== CONSTANTS ===============================================================*/

#ifdef _TOOLS_
  #ifdef FRAME_DLL
    #define DLL_IMPORT_DATA extern
  #else /* FRAME_DLL*/
    #define DLL_IMPORT_DATA __declspec( dllimport )
  #endif /* FRAME_DLL*/
#endif /* _TOOLS_ */
  
#ifdef MEMORY_SUPERVISION
 #define FILE_LINE              ,file,line
 #define FILE_LINE_TYPE         ,const char *file, int line
 #define FILE_LINE_MACRO        ,__FILE__,__LINE__
 #define FILE_LINE_MACRO_PASSED ,rm_path(file),line
#else
 #define FILE_LINE
 #define FILE_LINE_TYPE
 #define FILE_LINE_MACRO
 #define FILE_LINE_MACRO_PASSED ,__FILE__,__LINE__
#endif

#define NO_TASK           0

/*
 * defines if the frame is controlling a protocol stack or a tools application (TAP,PCO,...)
 */
#define ENV_STACK         0
#define ENV_TOOLS         1

#define PASSIVE_BODY        0x00000001    /* main loop in the frame compared to ACTIVE_BODY with main loop in pei_run() */
#define COPY_BY_REF         0x00000002    /* only pointers to message are exchanged between the entities                */
#define SYSTEM_PROCESS      0x00000004    /* currently used to have a non-blocking tracing behavior for TST and RCV     */
#define TRC_NO_SUSPEND      0x00000008    /* discard traces, if no memory available or test interface queue full        */
#define PARTITION_AUTO_FREE 0x00000010    /* automatically free memory after primitive processing                       */
#define PRIM_NO_SUSPEND     0x00000020    /* drop duplicated prim, if no memory available or test interface queue full  */
#define INT_DATA_TASK       0x00000040    /* allocate task stack and queue memory from internal memory                  */
#define ADD_QUEUE_SIZES     0x00000080    /* add queue size for grouped entities (default is to take biggest value)     */
#define USE_LEMU_QUEUE      0x00000100    /* use registered "os_sendtoqueue" function instead of standard function      */

/*
 * return values
 */
#define VSI_OK         0
#define VSI_TIMEOUT    1
#define VSI_ERROR     (-1)

/*
 * message types
 */
#define MSG_PRIMITIVE   1
#define MSG_SIGNAL      2
#define MSG_TIMEOUT     3

/*
 * definitions for dynamic primitive allocation
 */
#define DP_FRAME_GUESS     0xffffffff
#define DP_NO_FRAME_GUESS  0


#ifndef NTRACE
  #ifndef NTRACE_FUNC
    #define TRACE_FUNC 
  #endif
  #define TRACE_EVE
  #define TRACE_ERR
  #define TRACE_PRIM
  #ifndef NTRACE_GET_STATE
    #define TRACE_GET_STATE
  #endif
  #ifndef NTRACE_SET_STATE
    #define TRACE_SET_STATE
  #endif
  #define TRACE_IS              /* entity internal signals */
  #define TRACE_BIN
  #define TRACE_PRF
#endif

/*
 * trace bit masks
 */
#define TC_FUNC           0x00000001
#define TC_EVENT          0x00000002
#define TC_PRIM           0x00000004
#define TC_STATE          0x00000008
#define TC_SYSTEM         0x00000010
#define TC_ISIG           0x00000020
#define TC_ERROR          0x00000040
#define TC_CCD            0x00000080
#define TC_TIMER          0x00000100
#define TC_DATA           0x00000200
#define TC_SDU            0x00000400
#define TC_PROFILER       0x00000800
#define TC_ALERT_HINT     0x00001000
#define TC_ALERT_WARNING  0x00002000
#define TC_ALERT_ERROR    TC_ERROR
#define TC_ALERT_FATAL    0x00004000

#define TC_USER1          0x00010000
#define TC_USER2          0x00020000
#define TC_USER3          0x00040000
#define TC_USER4          0x00080000
#define TC_USER5          0x00100000
#define TC_USER6          0x00200000
#define TC_USER7          0x00400000
#define TC_USER8          0x00800000

/*
 * TC_ENABLE is use for sending primitives from target to tools.
 * In case these do not need to be dependent on trace filter
 * settings, TC_ENABLE must be passed to vsi_o_primsend resp. the 
 * macros calling vsi_o_primsend.
 */
#define TC_ENABLE         0xffffffff

/*
 * trace opc's
 */
#define TRACE_OPC           0x00000000

#define BIN_TRACE_OPC       0xC7654321

#define IP_TRACE_OPC        0xC0000800
#define HEX_TRACE_OPC       0xC0010800
#define SDU_TRACE_OPC       0xC0020800

#define TRACE_SAP           0x00000801

#define FUNC_TRACE_OPC      0xC0010801
#define EVENT_TRACE_OPC     0xC0020801 
#define PRIM_TRACE_OPC      0xC0030801
#define STATE_TRACE_OPC     0xC0040801
#define SYSTEM_TRACE_OPC    0xC0050801
#define ISIG_TRACE_OPC      0xC0060801
#define ERROR_TRACE_OPC     0xC0070801
#define CCD_TRACE_OPC       0xC0080801
#define TIMER_TRACE_OPC     0xC0090801
#define PROFILER_TRACE_OPC  0xC00A0801

#define USER1_TRACE_OPC     0xC00F0801
#define USER2_TRACE_OPC     0xC0100801
#define USER3_TRACE_OPC     0xC0110801
#define USER4_TRACE_OPC     0xC0120801
#define USER5_TRACE_OPC     0xC0130801
#define USER6_TRACE_OPC     0xC0140801
#define USER7_TRACE_OPC     0xC0150801
#define USER8_TRACE_OPC     0xC0160801

/* system primitive mask  */
#define SYS_MASK            0x8000             

/* declare trace queue names */
#ifdef _TOOLS_
#ifdef FRAME_DLL
  extern UBYTE SuppressOK;
  extern char FRM_SYST_NAME[RESOURCE_NAMELEN];
  extern char FRM_TST_NAME[RESOURCE_NAMELEN];
  extern char FRM_RCV_NAME[RESOURCE_NAMELEN];
  extern char FRM_PCO_NAME[RESOURCE_NAMELEN];
#else /* FRAME_DLL */
  __declspec (dllimport) char FRM_SYST_NAME[RESOURCE_NAMELEN];
  __declspec (dllimport) char FRM_TST_NAME[RESOURCE_NAMELEN];
  __declspec (dllimport) char FRM_RCV_NAME[RESOURCE_NAMELEN];
  __declspec (dllimport) char FRM_PCO_NAME[RESOURCE_NAMELEN];
#endif /* FRAME_DLL */
#else /* _TOOLS_ */
  extern char FRM_SYST_NAME[];
  extern char FRM_TST_NAME[];
  extern char FRM_RCV_NAME[];
  extern char FRM_PCO_NAME[];
#endif /* _TOOLS_ */

/*
 * masks for different kind of info passed to the type parameter at memory allocation  
 */
#define VSI_MEM_POOL_MASK     0x000000ff
#define VSI_MEM_FLAG_MASK     0x0000ff00
#define VSI_MEM_DESC_MASK     0x00ff0000

/*
 * types of partition pools to allocate from 
 */
#define PRIM_POOL_PARTITION   PrimGroupHandle
#define DMEM_POOL_PARTITION   DmemGroupHandle

/*
 * flags to determine allocation behavior 
 */
#define VSI_MEM_NON_BLOCKING  0x00000100
  
/*
 * descriptor types for allocation 
 */
#define VSI_DESC_TYPE0        0x00000000
#define VSI_DESC_TYPE1        0x00010000
#define VSI_DESC_TYPE2        0x00020000
#define VSI_DESC_TYPE3        0x00030000

/*
 * definitions for dynamic timer configuration 
 */
#define TIMER_SET        1
#define TIMER_RESET      2
#define TIMER_SPEED_UP   3
#define TIMER_SLOW_DOWN  4
#define TIMER_SUPPRESS   5
#define TIMER_CLEAN      6

/*
 * maximum length for a text trace 
 */
#define TTRACE_LEN            100

/*
 * values do be passed to vsi_m_init() to define if the allocated
 * partitions shall be initialized with a predefined value.
 */
#define DISABLE_PARTITON_INIT   0
#define ENABLE_PARTITON_INIT    1

#define TIME_MODE_MASK    0xc0000000
#define TIME_IS_TDMA      0x80000000

#define CHECK_TRC_SUSPEND        1
#define CHECK_PRIM_SUSPEND       2

/*==== TYPES ================================================================*/

typedef ULONG T_TIME;
typedef int T_HANDLE;

#include "pei.h"
#include "alert.h"

union T_MSG
{
  struct T_Prim
  {
    T_VOID_STRUCT *Prim;
    ULONG		      PrimLen;
  } Primitive;
  struct Sig
  {
    ULONG		      SigOPC;
    T_VOID_STRUCT *SigBuffer;
    ULONG		SigLen;
  } Signal;
  struct Tim
  {
    ULONG    Index;
  } Timer;
};

typedef	struct
{
   USHORT		 MsgType;
   union T_MSG Msg;
} T_QMSG;

typedef struct 
{
   char const *Str;
   USHORT  Ind;
} T_STR_IND;

#include <stdarg.h>

typedef struct
{
  void (*trace_error)(const char * const format, va_list varpars);      
  void (*trace_assert)(const char * const format, va_list varpars);      
} T_EXT_TRACE_FUNC;

/*==== EXTERNALS ===============================================================*/

extern T_HANDLE PrimGroupHandle;
extern T_HANDLE DmemGroupHandle;
#ifdef FF_FAST_MEMORY
extern T_HANDLE FastGroupHandle;
#endif

/*==== PROTOTYPES ===============================================================*/
//TISH modified for MSIM
#ifndef FRAME_DLL

T_HANDLE        vsi_p_create      (T_HANDLE Caller, 
                                   SHORT (*pei_create)(T_PEI_INFO const ** info),
                                   void (*TaskEntry)(T_HANDLE, ULONG), 
                                   T_HANDLE MemPoolHandle );
int             vsi_p_exit        (T_HANDLE Caller, T_HANDLE TaskHandle);
int             vsi_p_delete      (T_HANDLE Caller, T_HANDLE TaskHandle);
int             vsi_p_start       (T_HANDLE Caller, T_HANDLE TaskHandle);
int             vsi_p_stop        (T_HANDLE Caller, T_HANDLE TaskHandle);
int             vsi_p_name        (T_HANDLE Caller, T_HANDLE Handle, char *Name);
T_HANDLE        vsi_p_handle      (T_HANDLE Caller, char *Name);
void            vsi_o_set_htrace  (T_HANDLE comhandle);

T_VOID_STRUCT * vsi_m_new         (ULONG Size, ULONG type FILE_LINE_TYPE);
T_VOID_STRUCT * vsi_m_new_size    (ULONG size, ULONG type, 
                                   ULONG *partition_size FILE_LINE_TYPE);
int             vsi_m_free        (T_VOID_STRUCT **Addr FILE_LINE_TYPE);
int             vsi_m_status      (T_HANDLE caller, ULONG size, USHORT type, USHORT *free, USHORT *alloc);
int             vsi_m_init        (char enable_init, char pattern);
T_VOID_STRUCT * vsi_m_cnew        (ULONG size, ULONG type FILE_LINE_TYPE);
int             vsi_m_cfree       (T_VOID_STRUCT **ptr FILE_LINE_TYPE);
int             vsi_m_attach      (T_VOID_STRUCT *ptr FILE_LINE_TYPE);
int             vsi_m_register_pool(char * name, T_HANDLE *pool_gr_id);


char          * vsi_c_init_com_matrix (int max_entities);
int             vsi_c_get_com_matrix_entry (int entry, char *dst);
int             vsi_c_get_entity_com_entry (int entry, T_HANDLE rcv, T_HANDLE *snd);
T_HANDLE        vsi_c_open        (T_HANDLE Caller, char *Name);
int             vsi_c_close       (T_HANDLE Caller, T_HANDLE ComHandle);
int             vsi_c_clear       (T_HANDLE Caller, T_HANDLE ComHandle);
T_VOID_STRUCT * vsi_c_pnew_generic(T_HANDLE Caller, ULONG Size, ULONG opc, ULONG flags FILE_LINE_TYPE);
T_VOID_STRUCT * vsi_c_pnew        (ULONG Size, ULONG opc FILE_LINE_TYPE);
T_VOID_STRUCT * vsi_c_pnew_nb     (ULONG Size, ULONG opc FILE_LINE_TYPE);
T_VOID_STRUCT * vsi_c_new_sdu     (ULONG Size, ULONG opc, USHORT sdu_len, USHORT sdu_offset, USHORT encode_offset FILE_LINE_TYPE);
T_VOID_STRUCT * vsi_c_new_sdu_generic(ULONG Size, ULONG opc, USHORT sdu_len, USHORT sdu_offset, USHORT encode_offset, ULONG flags FILE_LINE_TYPE);
T_VOID_STRUCT * vsi_c_ppass       (T_VOID_STRUCT *prim, ULONG opc FILE_LINE_TYPE);
void            vsi_c_pstore      (T_VOID_STRUCT *prim FILE_LINE_TYPE);
int             vsi_c_pattach     (T_VOID_STRUCT *prim FILE_LINE_TYPE);
int             vsi_c_pfree       (T_VOID_STRUCT **Msg FILE_LINE_TYPE);
int             vsi_c_ssend       (T_HANDLE ComHandle, ULONG opc,
                                   T_VOID_STRUCT *ptr, ULONG MsgLen FILE_LINE_TYPE);
int             vsi_c_psend       (T_HANDLE ComHandle, T_VOID_STRUCT *ptr FILE_LINE_TYPE);
int             vsi_c_psend_caller(T_HANDLE caller, T_HANDLE ComHandle, T_VOID_STRUCT *ptr FILE_LINE_TYPE);
T_VOID_STRUCT * vsi_c_reuse       (T_PRIM_HEADER *ptr, ULONG Size, 
                                   ULONG opc, USHORT sdu_len, USHORT sdu_offset, 
                                   USHORT encode_offset FILE_LINE_TYPE);
T_VOID_STRUCT * vsi_c_new         (T_HANDLE Caller, ULONG Size, ULONG opc FILE_LINE_TYPE);
int             vsi_c_send        (T_HANDLE Caller, T_HANDLE ComHandle, 
                                   T_QMSG *Msg FILE_LINE_TYPE);
int             vsi_c_free        (T_HANDLE Caller, T_VOID_STRUCT **Msg FILE_LINE_TYPE);
int             vsi_c_pmax_size   (void);
int             vsi_c_sync        (T_HANDLE Caller, T_TIME timeout);
int             vsi_c_alloc_send  (T_HANDLE com_handle, char* rcv, char* snd, void *prim, char* string);
T_VOID_STRUCT * vsi_drpo_new      (ULONG size, ULONG opc, ULONG guess FILE_LINE_TYPE);
T_VOID_STRUCT * vsi_drpo_new_sdu  (ULONG size, ULONG opc, USHORT sdu_len, USHORT sdu_offset, USHORT encode_offset, ULONG guess FILE_LINE_TYPE );
T_VOID_STRUCT * vsi_drp_new       (ULONG size, ULONG guess FILE_LINE_TYPE);
T_VOID_STRUCT * vsi_dp_new        (ULONG size, T_VOID_STRUCT *addr, ULONG guess FILE_LINE_TYPE);
int             vsi_drp_bind      (T_VOID_STRUCT *child, T_VOID_STRUCT *parent FILE_LINE_TYPE);
int             vsi_free          (T_VOID_STRUCT **Msg FILE_LINE_TYPE);
int             vsi_dp_sum        (T_VOID_STRUCT *addr, ULONG *size);
int             vsi_dp_max_size   (void);
int             vsi_c_await       (T_HANDLE Caller, T_HANDLE ComHandle, T_QMSG *Msg, 
                                   ULONG Timeout);
int             vsi_c_primitive   (T_HANDLE Caller, void *Prim);
int             vsi_c_awake       (T_HANDLE caller );
int             vsi_c_status      (T_HANDLE q_handle, unsigned int *used, unsigned int *free);

int             vsi_t_start       (T_HANDLE Caller, USHORT Index, T_TIME Value);
int             vsi_t_pstart      (T_HANDLE Caller, USHORT Index, T_TIME Value1, T_TIME Value2 );
int             vsi_t_stop        (T_HANDLE Caller, USHORT Index );
int             vsi_t_status      (T_HANDLE Caller, USHORT Index, T_TIME *Value );
int             vsi_t_config      (T_HANDLE Caller, USHORT Index, UBYTE Mode, ULONG Value );
int             _vsi_t_config     (T_HANDLE Caller, char *CfgString, const T_STR_IND *pTable );

T_HANDLE        vsi_s_open        (T_HANDLE Caller, char *Name, USHORT Count);
int             vsi_s_close       (T_HANDLE Caller, T_HANDLE SemHandle);
int             vsi_s_get         (T_HANDLE Caller, T_HANDLE SemHandle);
int             vsi_s_release     (T_HANDLE Caller, T_HANDLE SemHandle);
int             vsi_s_status      (T_HANDLE Caller, T_HANDLE SemHandle, USHORT *Count);

/* *** ATTENTION START: A modification of the prototypes below requires a modification of STR2IND *** */ 
int             vsi_o_func_ttrace (const char * const format, ... );
int             vsi_o_event_ttrace(const char * const format, ... );
int             vsi_o_error_ttrace(const char * const format, ... );
int             vsi_o_state_ttrace(const char * const format, ... );
int             vsi_o_class_ttrace(ULONG TraceClass, const char * const format, ... );
int             vsi_o_func_itrace (ULONG index, const char * const format, ... );
int             vsi_o_event_itrace(ULONG index, const char * const format, ... );
int             vsi_o_error_itrace(ULONG index, const char * const format, ... );
int             vsi_o_state_itrace(ULONG index, const char * const format, ... );
int             vsi_o_class_itrace(ULONG TraceClass, ULONG index, const char * const format, ... );
/* *** ATTENTION   END: A modification of the prototypes above requires a modification of STR2IND *** */ 

int             vsi_o_ttrace      (T_HANDLE Caller, ULONG TraceClass, const char * const format, ... );
int             vsi_o_itrace      (T_HANDLE Caller, ULONG TraceClass, ULONG index, const char * const format, ... );
int             vsi_o_ptrace      (T_HANDLE Caller, ULONG opc, UBYTE dir );
int             vsi_o_strace      (T_HANDLE Caller,  const char *const machine,
                                   const char *const curstate,
                                   const char *const newstate);
void            vsi_ext_trace_register (T_EXT_TRACE_FUNC * func);
int             vsi_o_memtrace    (T_HANDLE Caller, void *ptr, unsigned int len);
int             vsi_o_primsend    (T_HANDLE Caller, unsigned int mask, T_HANDLE dst, char *ext_dst, unsigned int opc, void *ptr, unsigned int len FILE_LINE_TYPE);
int             vsi_o_sdusend     (T_HANDLE caller, T_HANDLE dst, char *ext_dst, int opc, char ent, char dir, char type, void *ptr, unsigned int len FILE_LINE_TYPE);
int             vsi_o_assert      (T_HANDLE Caller, USHORT cause, const char *file, int line, const char * const format, ...);
int             vsi_settracemask  (T_HANDLE Caller, T_HANDLE Handle, ULONG Mask );
int             vsi_gettracemask  (T_HANDLE Caller, T_HANDLE Handle, ULONG *Mask );
int             vsi_trcsuspend    (T_HANDLE Caller, T_HANDLE Handle, ULONG Suspend );
int             vsi_trc_free      (T_HANDLE Caller, T_VOID_STRUCT **Msg);
ULONG           get_suspend_state (T_HANDLE caller, int type);


void            vsi_ppm_new       (T_HANDLE Caller, ULONG Size, T_PRIM_HEADER *prim, const char* file, int line );
void            vsi_ppm_rec       (T_HANDLE Caller, T_PRIM_HEADER *prim, const char* file, int line );
void            vsi_ppm_access    (T_HANDLE Caller, T_PRIM_HEADER *prim, const char* file, int line );
void            vsi_ppm_store     (T_HANDLE Caller, T_PRIM_HEADER *prim, const char* file, int line );
void            vsi_ppm_send      (T_HANDLE Caller, T_HANDLE rcv, T_PRIM_HEADER *prim, const char* file, int line );
void            vsi_ppm_reuse     (T_HANDLE Caller, T_PRIM_HEADER *prim, const char* file, int line );
void            vsi_ppm_free      (T_HANDLE Caller, T_PRIM_HEADER *prim, const char* file, int line );
void            vsi_ppm_setend    (T_PRIM_HEADER *Prim, ULONG Size );

int             vsi_gettaskname   (T_HANDLE Caller, T_HANDLE Handle, char *Name);
T_HANDLE        vsi_gettaskhandle (T_HANDLE Caller, char *Name);
int             vsi_e_name        (T_HANDLE Caller, T_HANDLE Handle, char *Name);
T_HANDLE        vsi_e_handle      (T_HANDLE Caller, char *Name);
int             vsi_gettaskflags  (T_HANDLE Caller, T_HANDLE Handle, U32 *Flags);
int             vsi_t_time        (T_HANDLE Caller, T_TIME *Value);
int             vsi_t_sleep       (T_HANDLE Caller, T_TIME Value);
int             vsi_object_info   (T_HANDLE caller, USHORT Id, USHORT Index, char *Buffer, USHORT Size);

void            vsi_d_callback    (T_DRV_SIGNAL *Signal );
int             vsi_d_create      (T_HANDLE Caller, T_TST_DRV_ENTRY *drv_info );
int             vsi_d_init        (T_HANDLE Caller );
int             vsi_d_exit        (T_HANDLE Caller, T_HANDLE DrvHandle );
int             vsi_d_open        (T_HANDLE Caller, char *Name );
int             vsi_d_close       (T_HANDLE Caller, T_HANDLE DrvHandle );
int             vsi_d_read        (T_HANDLE Caller, T_HANDLE DrvHandle, void *Buffer, ULONG *Size );
int             vsi_d_write       (T_HANDLE Caller, T_HANDLE DrvHandle, void *Buffer, ULONG Size );
int             vsi_d_flush       (T_HANDLE Caller, T_HANDLE DrvHandle );
int             vsi_d_setsignal   (T_HANDLE Caller, T_HANDLE DrvHandle, USHORT SignalType );
int             vsi_d_resetsignal (T_HANDLE Caller, T_HANDLE DrvHandle, USHORT SignalType );
int             vsi_d_setconfig   (T_HANDLE Caller, T_HANDLE DrvHandle, char *Config );
int             vsi_d_getconfig   (T_HANDLE Caller, T_HANDLE DrvHandle, char *Config );

void            vsi_pcheck_init   (void);
void            vsi_pcheck_register(ULONG (*func)(ULONG, void*), ULONG ret_ok);
//TISH modified for MSIM
#endif //#ifdef FRAME_DLL

/*==== MACROS ================================================================*/

#define ConfigTimer             _vsi_t_config

#ifdef MEMORY_SUPERVISION
  #define  VSI_PPM_REC(B,F,L)   
  #define  VSI_PPM_RCV(B)       vsi_ppm_rec (VSI_CALLER ((T_PRIM_HEADER*)(B)),__FILE__, __LINE__);
  #define  VSI_PPM_NEW(B,S)     vsi_ppm_new (VSI_CALLER S,((T_PRIM_HEADER*)(B)),__FILE__, __LINE__);
  #define  VSI_PPM_FREE(B)      vsi_ppm_free (VSI_CALLER ((T_PRIM_HEADER*)(B)),__FILE__, __LINE__);
  #define  VSI_PPM_REUSE(B)     vsi_ppm_reuse (VSI_CALLER (T_PRIM_HEADER*)B,__FILE__,__LINE__);
  #define  VSI_PPM_ACCESS(B)    vsi_ppm_access (VSI_CALLER (T_PRIM_HEADER*)B,__FILE__,__LINE__);
  #define  VSI_PPM_SEND(B,D)    vsi_ppm_send (VSI_CALLER D,(T_PRIM_HEADER*)B,__FILE__,__LINE__);
#else
  #define  VSI_PPM_REC(B,F,L)
  #define  VSI_PPM_RCV(B)
  #define  VSI_PPM_NEW(B)
  #define  VSI_PPM_FREE(B)
  #define  VSI_PPM_REUSE(B)
  #define  VSI_PPM_ACCESS(B)
  #define  VSI_PPM_SEND(B,D)
#endif /* MEMORY_SUPERVISION */

#define  ALIGN(S)                  (((S)+3)&~0x03)
#define  ALIGN_SIZE(S)			   (((S)+sizeof(int)-1)&~(sizeof(int)-1))	
#define  S_ALLOC_SIZE(S)           (ALIGN((S)+sizeof(T_PRIM_HEADER)+sizeof(T_S_HEADER)))
#define  S_HDR_OFFSET(S)           (ALIGN(S)>>2)
#define  D_ALLOC_SIZE(S)           (ALIGN((S)+sizeof(T_PRIM_HEADER)+sizeof(T_DP_HEADER)))
#define  D_HDR_OFFSET(S)           (ALIGN((S)-sizeof(T_DP_HEADER))>>2)

#define  P2D(P)                    ((T_PRIM_HEADER*)(P)+1)
#define  D2P(D)                    ((T_PRIM_HEADER*)(D)-1)

#define  P2D_AC(P,T)               P2D(P)

#define  P_OPC(P)                  (((T_PRIM_HEADER*)(P))->opc)
#define  P_OPC2(P)                 (((T_PRIM_HEADER*)(P))->opc2)
#define  P_LEN(P)                  (((T_PRIM_HEADER*)(P))->len)
#define  P_TID(P)                  (((T_PRIM_HEADER*)(P))->tid)
#define  P_SDU(P)                  (((T_PRIM_HEADER*)(P))->sdu)
#define  P_CNT(P)                  (((T_PRIM_HEADER*)(P))->use_cnt)
#define  P_IDX(P)                  (*(ULONG*)(P))
#define  P_PPM_IDX(P)              (*((ULONG*)(P)-PPM_OFFSET))
#define  P_PNR(P)                  (*((ULONG*)(P)-PPM_OFFSET) & 0xffff)
#define  P_PGR(P)                  ((*((ULONG*)(P)-PPM_OFFSET) >> 16) & 0xffff)
#define  P_SHDR(P)                 ((T_S_HEADER*)((ULONG*)(P)+((T_PRIM_HEADER*)(P))->sh_offset))
#define  P_RCV(P)                  (((T_S_HEADER*)P_SHDR(P))->rcv) 
#define  P_SND(P)                  (((T_S_HEADER*)P_SHDR(P))->snd) 
#define  P_TIME(P)                 (((T_S_HEADER*)P_SHDR(P))->time) 
#define  P_SHO(P)                  (((T_PRIM_HEADER*)(P))->sh_offset)
#define  P_DPHO(P)                 (((T_PRIM_HEADER*)(P))->dph_offset)
#define  P_MEMHANDLE(P)            (*(U32*)P2D(P))
#define  P_MEMHANDLE_SDU(P)        (*(((U32*)P2D(P))+1))

#define  D_OPC(D)                  P_OPC(D2P(D))
#define  D_OPC2(D)                 P_OPC2(D2P(D))
#define  D_LEN(D)                  P_LEN(D2P(D))
#define  D_CNT(D)                  P_CNT(D2P(D))
#define  D_TID(D)                  P_TID(D2P(D))
#define  D_SDU(D)                  P_SDU(D2P(D))

#define  D_SDU_LEN(D)              (D_SDU(D)->l_buf)
#define  D_SDU_OFF(D)              (D_SDU(D)->o_buf)

#define  BITS_PER_BYTE             8
#define  BYTELEN(BITLEN)           ((BITLEN)<=0?0:((((BITLEN)-1)/BITS_PER_BYTE)+1))
#define  BYTELEN_POS(BITLEN)       (((BITLEN)+7)/BITS_PER_BYTE)

#define  SDU_TRAIL                 ((char*)(((T_sdu*)0)+1)-(char*)(((T_sdu*)0)->buf))
#define  PSIZE(D)                  D_LEN(D)

#define  SIZ(T)                    ((ULONG)(sizeof(T_PRIM_HEADER)+sizeof(T)))
#define  SIZ_SDU(T,M)              ((ULONG)(SIZ(T)+BYTELEN((M)+ENCODE_OFFSET)-SDU_TRAIL))
/*  NOTE : received SDUs may have a different          ENCODE_OFFSET */

#ifndef ADD_BSIZE
#define ADD_BSIZE 0
#endif

#define  NO_SDU   0xffff

/*lint -e773 Info 773: Expression-like macro '...' not parenthesized 
  this message has to be disabled to avoid LINT complaining about missing parentesis in the macros
  that create a pointer and initialize it. Here we cannot set paranthesis because in this case the
  existing entity code will not compile without ';'. To disable this message is safe for these macros */

#define  DRPO_ALLOC(T,G)           (T_##T*)vsi_drpo_new(sizeof(T_##T),T,G FILE_LINE_MACRO)
#define  DRPO_ALLOC_SDU(T,N,G)     (T_##T*)vsi_drpo_new_sdu(sizeof(T_##T),T,N,offsetof(T_##T,sdu),ENCODE_OFFSET,G FILE_LINE_MACRO)
#define  DRP_ALLOC(S,G)            (T_VOID_STRUCT*)vsi_drp_new(S,G FILE_LINE_MACRO)
#define  DRP_BIND(C,P)             vsi_drp_bind((T_VOID_STRUCT*)C,(T_VOID_STRUCT*)P FILE_LINE_MACRO)
#define  DP_ALLOC(S,P,G)           (T_VOID_STRUCT*)vsi_dp_new(S,(T_VOID_STRUCT*)P,G FILE_LINE_MACRO)
#define  FREE(D)                   { T_VOID_STRUCT *z=(T_VOID_STRUCT*)D;\
                                     vsi_free((T_VOID_STRUCT **)&z FILE_LINE_MACRO);}
#define  MALLOC(P,S)               P = (void*)vsi_m_cnew(S,(PrimGroupHandle|VSI_DESC_TYPE0) FILE_LINE_MACRO)
#define  M_ALLOC(S)                (void*)vsi_m_cnew(S,(PrimGroupHandle|VSI_DESC_TYPE0) FILE_LINE_MACRO)

#ifdef FF_FAST_MEMORY
#define  FMALLOC(P,S)              P = (void*)vsi_m_cnew(S,(FastGroupHandle|VSI_DESC_TYPE0) FILE_LINE_MACRO)
#define  FM_ALLOC(S)               (void*)vsi_m_cnew(S,(FastGroupHandle|VSI_DESC_TYPE0) FILE_LINE_MACRO)
#else
#define  FMALLOC(P,S)              MALLOC(P,S)
#define  FM_ALLOC(S)               M_ALLOC(S)
#endif

#define  MALLOC_GENERIC(P,S,G,F)   P = (void*)vsi_m_cnew(S,(G|(F)) FILE_LINE_MACRO)
#define  M_ALLOC_GENERIC(S,G,F)    (void*)vsi_m_cnew(S,(G|(F)) FILE_LINE_MACRO)

#define  MALLOC_DESC1(P,S)         P = (void*)vsi_m_cnew(S,(PrimGroupHandle|VSI_DESC_TYPE1) FILE_LINE_MACRO)
#define  M_ALLOC_DESC1(S)          (void*)vsi_m_cnew(S,(PrimGroupHandle|VSI_DESC_TYPE1) FILE_LINE_MACRO)

#define  MALLOC_DESC2(P,S)         P = (void*)vsi_m_cnew(S,(PrimGroupHandle|VSI_DESC_TYPE2) FILE_LINE_MACRO)
#define  M_ALLOC_DESC2(S)          (void*)vsi_m_cnew(S,(PrimGroupHandle|VSI_DESC_TYPE2) FILE_LINE_MACRO)

#define  MALLOC_DESC3(P,S)         P = (void*)vsi_m_cnew(S,(PrimGroupHandle|VSI_DESC_TYPE3) FILE_LINE_MACRO)
#define  M_ALLOC_DESC3(S)          (void*)vsi_m_cnew(S,(PrimGroupHandle|VSI_DESC_TYPE3) FILE_LINE_MACRO)

#define  MALLOC_NB(P,S)            P = (void*)vsi_m_cnew(S,(PrimGroupHandle|VSI_DESC_TYPE0|VSI_MEM_NON_BLOCKING) FILE_LINE_MACRO)
#define  M_ALLOC_NB(S)             (void*)vsi_m_cnew(S,(PrimGroupHandle|VSI_DESC_TYPE0|VSI_MEM_NON_BLOCKING) FILE_LINE_MACRO)

#define  MALLOC_DESC1_NB(P,S)      P = (void*)vsi_m_cnew(S,(PrimGroupHandle|VSI_DESC_TYPE1|VSI_MEM_NON_BLOCKING) FILE_LINE_MACRO)
#define  M_ALLOC_DESC1_NB(S)       (void*)vsi_m_cnew(S,(PrimGroupHandle|VSI_DESC_TYPE1|VSI_MEM_NON_BLOCKING) FILE_LINE_MACRO)

#define  MALLOC_DESC2_NB(P,S)      P = (void*)vsi_m_cnew(S,(PrimGroupHandle|VSI_DESC_TYPE2|VSI_MEM_NON_BLOCKING) FILE_LINE_MACRO)
#define  M_ALLOC_DESC2_NB(S)       (void*)vsi_m_cnew(S,(PrimGroupHandle|VSI_DESC_TYPE2|VSI_MEM_NON_BLOCKING) FILE_LINE_MACRO)

#define  MALLOC_DESC3_NB(P,S)      P = (void*)vsi_m_cnew(S,(PrimGroupHandle|VSI_DESC_TYPE3|VSI_MEM_NON_BLOCKING) FILE_LINE_MACRO)
#define  M_ALLOC_DESC3_NB(S)       (void*)vsi_m_cnew(S,(PrimGroupHandle|VSI_DESC_TYPE3|VSI_MEM_NON_BLOCKING) FILE_LINE_MACRO)

#define  MATTACH(M)                vsi_m_attach((T_VOID_STRUCT*)M FILE_LINE_MACRO)
#define  M_ATTACH(M)               vsi_m_attach((T_VOID_STRUCT*)M FILE_LINE_MACRO)

#define  DMALLOC(P,S)              P = (void*)vsi_m_new(S,DmemGroupHandle FILE_LINE_MACRO)
#define  D_ALLOC(S)                (void*)vsi_m_new(S,DmemGroupHandle FILE_LINE_MACRO)

#define  DMALLOC_NB(P,S)           P = (void*)vsi_m_new(S,DmemGroupHandle|VSI_MEM_NON_BLOCKING FILE_LINE_MACRO)
#define  D_ALLOC_NB(S)             (void*)vsi_m_new(S,DmemGroupHandle|VSI_MEM_NON_BLOCKING FILE_LINE_MACRO)

#define  PALLOC_GENERIC(D,T,G,F)   T_##T  *D = (T_##T*)vsi_c_pnew_generic(0,sizeof(T_##T),T,(G|(F)) FILE_LINE_MACRO)
#define  P_ALLOC_GENERIC(T,G,F)    (T_##T*)vsi_c_pnew_generic(0,sizeof(T_##T),T,(G|(F)) FILE_LINE_MACRO )

#define  PALLOC(D,T)               T_##T  *D = (T_##T*)vsi_c_pnew(sizeof(T_##T),T FILE_LINE_MACRO)
#define  P_ALLOC(T)                (T_##T*)vsi_c_pnew(sizeof(T_##T),T FILE_LINE_MACRO )

#define  PALLOC_NB(D,T)            T_##T  *D = (T_##T*)vsi_c_pnew_nb(sizeof(T_##T),T FILE_LINE_MACRO)
#define  P_ALLOC_NB(T)             (T_##T*)vsi_c_pnew_nb(sizeof(T_##T),T FILE_LINE_MACRO )

#define  PALLOC_MSG(D,T,M)         T_##T  *D = (T_##T*)vsi_c_new_sdu(sizeof(T_##T),T,BSIZE_##M+ADD_BSIZE,offsetof(T_##T,sdu),ENCODE_OFFSET FILE_LINE_MACRO)
#define  P_ALLOC_MSG(T,M)          (T_##T*)vsi_c_new_sdu(sizeof(T_##T),T,BSIZE_##M+ADD_BSIZE,offsetof(T_##T,sdu),ENCODE_OFFSET FILE_LINE_MACRO)

#define  PALLOC_SDU(D,T,N)         T_##T  *D = (T_##T*)vsi_c_new_sdu(sizeof(T_##T),T,N,offsetof(T_##T,sdu),ENCODE_OFFSET FILE_LINE_MACRO)
#define  P_ALLOC_SDU(T,N)          (T_##T*)vsi_c_new_sdu(sizeof(T_##T),T,N,offsetof(T_##T,sdu),ENCODE_OFFSET FILE_LINE_MACRO)

#ifdef FF_FAST_MEMORY
#define  FPALLOC_SDU(D,T,N)        T_##T  *D = (T_##T*)vsi_c_new_sdu_generic(sizeof(T_##T),T,N,offsetof(T_##T,sdu),ENCODE_OFFSET,FastGroupHandle FILE_LINE_MACRO)
#define  FP_ALLOC_SDU(T,N)         (T_##T*)vsi_c_new_sdu_generic(sizeof(T_##T),T,N,offsetof(T_##T,sdu),ENCODE_OFFSET,FastGroupHandle FILE_LINE_MACRO)
#else
#define  FPALLOC_SDU(D,T,N)        PALLOC_SDU(D,T,N)
#define  FP_ALLOC_SDU(T,N)         P_ALLOC_SDU(T,N) 
#endif

#define  PALLOC_DESC(D,T)          T_##T  *D = (T_##T*)vsi_c_pnew(sizeof(T_##T),T FILE_LINE_MACRO);\
                                   ((T_PRIM_HEADER*)D2P(D))->dph_offset = (ULONG)(((offsetof(T_##T,desc_list)+sizeof(T_PRIM_HEADER))>>2));

#define  PALLOC_DESC2(D,T)         T_##T  *D = (T_##T*)vsi_c_pnew(sizeof(T_##T),T FILE_LINE_MACRO);\
                                   ((T_PRIM_HEADER*)D2P(D))->dph_offset = (ULONG)(((offsetof(T_##T,desc_list2)+sizeof(T_PRIM_HEADER))>>2));

#define  PALLOC_DESC3(D,T)         T_##T  *D = (T_##T*)vsi_c_pnew(sizeof(T_##T),T FILE_LINE_MACRO);\
                                   ((T_PRIM_HEADER*)D2P(D))->dph_offset = (ULONG)(((offsetof(T_##T,desc_list3)+sizeof(T_PRIM_HEADER))>>2));

#define  PALLOC_DESC_NB(D,T)       T_##T  *D = (T_##T*)vsi_c_pnew_nb(sizeof(T_##T),T FILE_LINE_MACRO);\
                                   ((T_PRIM_HEADER*)D2P(D))->dph_offset = (ULONG)(((offsetof(T_##T,desc_list)+sizeof(T_PRIM_HEADER))>>2));

#define  PALLOC_DESC2_NB(D,T)      T_##T  *D = (T_##T*)vsi_c_pnew_nb(sizeof(T_##T),T FILE_LINE_MACRO);\
                                   ((T_PRIM_HEADER*)D2P(D))->dph_offset = (ULONG)(((offsetof(T_##T,desc_list2)+sizeof(T_PRIM_HEADER))>>2));

#define  PALLOC_DESC3_NB(D,T)      T_##T  *D = (T_##T*)vsi_c_pnew_nb(sizeof(T_##T),T FILE_LINE_MACRO);\
                                   ((T_PRIM_HEADER*)D2P(D))->dph_offset = (ULONG)(((offsetof(T_##T,desc_list3)+sizeof(T_PRIM_HEADER))>>2));

#define  PATTACH(D)                vsi_c_pattach((T_VOID_STRUCT*)D FILE_LINE_MACRO)
#define  P_ATTACH(D)               vsi_c_pattach((T_VOID_STRUCT*)D FILE_LINE_MACRO)

#define  PREUSE(D0,D,T)            T_##T  *D = (T_##T*)vsi_c_reuse((T_PRIM_HEADER*)D0,SIZ(T_##T),T,0,NO_SDU,0 FILE_LINE_MACRO)
#define  P_REUSE(D0,T)             (T_##T*)vsi_c_reuse((T_PRIM_HEADER*)D0,SIZ(T_##T),T,0,NO_SDU,0 FILE_LINE_MACRO)

#define  PREUSE_MSG(D,T,M)         T_##T  *D = (T_##T*)vsi_c_reuse(D0,SIZ(T_##T),T,BSIZE_##M+ADD_BSIZE,offsetof(T_##T,sdu),ENCODE_OFFSET FILE_LINE_MACRO)
#define  P_REUSE_MSG(T,M)          (T_##T*)vsi_c_reuse(D0,SIZ(T_##T),T,BSIZE_##M+ADD_BSIZE,offsetof(T_##T,sdu),ENCODE_OFFSET FILE_LINE_MACRO)

#define  PREUSE_SDU(D,T,N)         T_##T  *D = (T_##T*)vsi_c_reuse(D0,SIZ(T_##T),T,N,offsetof(T_##T,sdu),ENCODE_OFFSET FILE_LINE_MACRO)
#define  P_REUSE_SDU(T,N)          (T_##T*)vsi_c_reuse(D0,SIZ(T_##T),T,N,offsetof(T_##T,sdu),ENCODE_OFFSET FILE_LINE_MACRO)

#define  PSEND(E,D)                vsi_c_psend ( E ,(T_VOID_STRUCT*)D FILE_LINE_MACRO )
#define  P_SEND(E,D)               vsi_c_psend ( E ,(T_VOID_STRUCT*)D FILE_LINE_MACRO )

#define  PSEND_CALLER(C,E,D)       vsi_c_psend_caller ( C, E ,(T_VOID_STRUCT*)D FILE_LINE_MACRO )
#define  P_SEND_CALLER(C,E,D)      vsi_c_psend_caller ( C, E ,(T_VOID_STRUCT*)D FILE_LINE_MACRO )

#define  PSIGNAL(E,O,D)            vsi_c_ssend ( E ,O,(T_VOID_STRUCT*)D,SIZ(T_##O) FILE_LINE_MACRO )
#define  P_SIGNAL(E,O,D)           vsi_c_ssend ( E ,O,(T_VOID_STRUCT*)D,SIZ(T_##O) FILE_LINE_MACRO )

#define  PRIM_SEND_TO_PC(S,R,P)        vsi_o_primsend(S,TC_ENABLE,0,R,0,P,0 FILE_LINE_MACRO)
#define  DATA_SEND_TO_PC(S,F,R,O,P,L)  vsi_o_primsend(S,F,0,R,O,P,L FILE_LINE_MACRO)

#define  PFREE(D)                  { T_VOID_STRUCT *z=(T_VOID_STRUCT*)D;\
                                   vsi_c_pfree((T_VOID_STRUCT **)&z FILE_LINE_MACRO);}
#define  P_FREE(D)                 { T_VOID_STRUCT *z=(T_VOID_STRUCT*)D;\
                                   vsi_c_pfree((T_VOID_STRUCT **)&z FILE_LINE_MACRO);}

#define  PPASS(D0,D,T)             T_##T  *D    = (T_##T*)vsi_c_ppass ( (T_VOID_STRUCT*)D0, T FILE_LINE_MACRO )
#define  P_PASS(D0,D,T)            T_##T  *D    = (T_##T*)vsi_c_ppass ( (T_VOID_STRUCT*)D0, T FILE_LINE_MACRO )

#define  PACCESS(D)                VSI_PPM_ACCESS(D2P(D))
#define  P_ACCESS(D)               VSI_PPM_ACCESS(D2P(D))

#define  PSTORE(D)                 vsi_c_pstore ((T_VOID_STRUCT*)D FILE_LINE_MACRO)
#define  P_STORE(D)                vsi_c_pstore ((T_VOID_STRUCT*)D FILE_LINE_MACRO)

#define  MFREE(P)                  vsi_m_cfree((T_VOID_STRUCT **)&P FILE_LINE_MACRO)
#define  M_FREE(P)                 vsi_m_cfree((T_VOID_STRUCT **)&P FILE_LINE_MACRO)

#define  D_FREE(P)                 vsi_m_free((T_VOID_STRUCT **)&P FILE_LINE_MACRO)

/*lint +e773 */

#ifdef TRACE_FUNC
  #define TRACE_FUNCTION(a)                                     vsi_o_func_ttrace(a)
  #define TRACE_FUNCTION_P1(f,a1)                               vsi_o_func_ttrace(f,a1)
  #define TRACE_FUNCTION_P2(f,a1,a2)                            vsi_o_func_ttrace(f,a1,a2)
  #define TRACE_FUNCTION_P3(f,a1,a2,a3)                         vsi_o_func_ttrace(f,a1,a2,a3)
  #define TRACE_FUNCTION_P4(f,a1,a2,a3,a4)                      vsi_o_func_ttrace(f,a1,a2,a3,a4)
  #define TRACE_FUNCTION_P5(f,a1,a2,a3,a4,a5)                   vsi_o_func_ttrace(f,a1,a2,a3,a4,a5)
  #define TRACE_FUNCTION_P6(f,a1,a2,a3,a4,a5,a6)                vsi_o_func_ttrace(f,a1,a2,a3,a4,a5,a6)
  #define TRACE_FUNCTION_P7(f,a1,a2,a3,a4,a5,a6,a7)             vsi_o_func_ttrace(f,a1,a2,a3,a4,a5,a6,a7)
  #define TRACE_FUNCTION_P8(f,a1,a2,a3,a4,a5,a6,a7,a8)          vsi_o_func_ttrace(f,a1,a2,a3,a4,a5,a6,a7,a8)
  #define TRACE_FUNCTION_P9(f,a1,a2,a3,a4,a5,a6,a7,a8,a9)       vsi_o_func_ttrace(f,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#else
  #define TRACE_FUNCTION(a)                                     ((void)(0))                               
  #define TRACE_FUNCTION_P1(f,a1)                               ((void)(0))
  #define TRACE_FUNCTION_P2(f,a1,a2)                            ((void)(0))                      
  #define TRACE_FUNCTION_P3(f,a1,a2,a3)                         ((void)(0))                    
  #define TRACE_FUNCTION_P4(f,a1,a2,a3,a4)                      ((void)(0))                 
  #define TRACE_FUNCTION_P5(f,a1,a2,a3,a4,a5)                   ((void)(0))              
  #define TRACE_FUNCTION_P6(f,a1,a2,a3,a4,a5,a6)                ((void)(0))           
  #define TRACE_FUNCTION_P7(f,a1,a2,a3,a4,a5,a6,a7)             ((void)(0))        
  #define TRACE_FUNCTION_P8(f,a1,a2,a3,a4,a5,a6,a7,a8)          ((void)(0))     
  #define TRACE_FUNCTION_P9(f,a1,a2,a3,a4,a5,a6,a7,a8,a9)       ((void)(0))  
#endif

#ifdef TRACE_EVE
  #define TRACE_EVENT(a)                                        vsi_o_event_ttrace(a)
  #define TRACE_EVENT_P1(f,a1)                                  vsi_o_event_ttrace(f,a1)
  #define TRACE_EVENT_P2(f,a1,a2)                               vsi_o_event_ttrace(f,a1,a2)
  #define TRACE_EVENT_P3(f,a1,a2,a3)                            vsi_o_event_ttrace(f,a1,a2,a3)
  #define TRACE_EVENT_P4(f,a1,a2,a3,a4)                         vsi_o_event_ttrace(f,a1,a2,a3,a4)
  #define TRACE_EVENT_P5(f,a1,a2,a3,a4,a5)                      vsi_o_event_ttrace(f,a1,a2,a3,a4,a5)
  #define TRACE_EVENT_P6(f,a1,a2,a3,a4,a5,a6)                   vsi_o_event_ttrace(f,a1,a2,a3,a4,a5,a6)
  #define TRACE_EVENT_P7(f,a1,a2,a3,a4,a5,a6,a7)                vsi_o_event_ttrace(f,a1,a2,a3,a4,a5,a6,a7)
  #define TRACE_EVENT_P8(f,a1,a2,a3,a4,a5,a6,a7,a8)             vsi_o_event_ttrace(f,a1,a2,a3,a4,a5,a6,a7,a8)
  #define TRACE_EVENT_P9(f,a1,a2,a3,a4,a5,a6,a7,a8,a9)          vsi_o_event_ttrace(f,a1,a2,a3,a4,a5,a6,a7,a8,a9)
  #define TRACE_USER_CLASS(c,a)                                 vsi_o_class_ttrace(c,a)
  #define TRACE_USER_CLASS_P1(c,f,a1)                           vsi_o_class_ttrace(c,f,a1)
  #define TRACE_USER_CLASS_P2(c,f,a1,a2)                        vsi_o_class_ttrace(c,f,a1,a2)
  #define TRACE_USER_CLASS_P3(c,f,a1,a2,a3)                     vsi_o_class_ttrace(c,f,a1,a2,a3)
  #define TRACE_USER_CLASS_P4(c,f,a1,a2,a3,a4)                  vsi_o_class_ttrace(c,f,a1,a2,a3,a4)
  #define TRACE_USER_CLASS_P5(c,f,a1,a2,a3,a4,a5)               vsi_o_class_ttrace(c,f,a1,a2,a3,a4,a5)
  #define TRACE_USER_CLASS_P6(c,f,a1,a2,a3,a4,a5,a6)            vsi_o_class_ttrace(c,f,a1,a2,a3,a4,a5,a6)
  #define TRACE_USER_CLASS_P7(c,f,a1,a2,a3,a4,a5,a6,a7)         vsi_o_class_ttrace(c,f,a1,a2,a3,a4,a5,a6,a7)
  #define TRACE_USER_CLASS_P8(c,f,a1,a2,a3,a4,a5,a6,a7,a8)      vsi_o_class_ttrace(c,f,a1,a2,a3,a4,a5,a6,a7,a8)
  #define TRACE_USER_CLASS_P9(c,f,a1,a2,a3,a4,a5,a6,a7,a8,a9)   vsi_o_class_ttrace(c,f,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#else
  #define TRACE_EVENT(a)                                        ((void)(0))                              
  #define TRACE_EVENT_P1(f,a1)                                  ((void)(0))
  #define TRACE_EVENT_P2(f,a1,a2)                               ((void)(0))                     
  #define TRACE_EVENT_P3(f,a1,a2,a3)                            ((void)(0))                   
  #define TRACE_EVENT_P4(f,a1,a2,a3,a4)                         ((void)(0))                 
  #define TRACE_EVENT_P5(f,a1,a2,a3,a4,a5)                      ((void)(0))              
  #define TRACE_EVENT_P6(f,a1,a2,a3,a4,a5,a6)                   ((void)(0))           
  #define TRACE_EVENT_P7(f,a1,a2,a3,a4,a5,a6,a7)                ((void)(0))        
  #define TRACE_EVENT_P8(f,a1,a2,a3,a4,a5,a6,a7,a8)             ((void)(0))    
  #define TRACE_EVENT_P9(f,a1,a2,a3,a4,a5,a6,a7,a8,a9)          ((void)(0))  
  #define TRACE_USER_CLASS(c,a)                                 ((void)(0))                   
  #define TRACE_USER_CLASS_P1(c,f,a1)                           ((void)(0))
  #define TRACE_USER_CLASS_P2(c,f,a1,a2)                        ((void)(0))
  #define TRACE_USER_CLASS_P3(c,f,a1,a2,a3)                     ((void)(0))
  #define TRACE_USER_CLASS_P4(c,f,a1,a2,a3,a4)                  ((void)(0))
  #define TRACE_USER_CLASS_P5(c,f,a1,a2,a3,a4,a5)               ((void)(0))
  #define TRACE_USER_CLASS_P6(c,f,a1,a2,a3,a4,a5,a6)            ((void)(0))
  #define TRACE_USER_CLASS_P7(c,f,a1,a2,a3,a4,a5,a6,a7)         ((void)(0))
  #define TRACE_USER_CLASS_P8(c,f,a1,a2,a3,a4,a5,a6,a7,a8)      ((void)(0))
  #define TRACE_USER_CLASS_P9(c,f,a1,a2,a3,a4,a5,a6,a7,a8,a9)   ((void)(0))
#endif

#ifdef TRACE_BIN
  #define TRACE_MEMORY(s,p,l)                                   vsi_o_memtrace(s,p,l)
  #define TRACE_MEMORY_PRIM(s,r,o,p,l)                          vsi_o_primsend(s,TC_DATA,r,FRM_PCO_NAME,o,p,l FILE_LINE_MACRO)
  #define TRACE_USER_CLASS_MEMORY_PRIM(s,m,r,o,p,l)             vsi_o_primsend(s,m,r,FRM_PCO_NAME,o,p,l FILE_LINE_MACRO)
  #define TRACE_BINDUMP(s,m,d,p,l)                             {vsi_o_ttrace(s,m,d);vsi_o_primsend(s,m,0,FRM_PCO_NAME,BIN_TRACE_OPC,p,l FILE_LINE_MACRO);}
  #define TRACE_HEXDUMP(s,p,l)                                  vsi_o_primsend(s,TC_DATA,0,FRM_PCO_NAME,HEX_TRACE_OPC,p,l FILE_LINE_MACRO);
  #define TRACE_SDU(s,r,e,d,t,p,l)                              vsi_o_sdusend(s,r,FRM_PCO_NAME,SDU_TRACE_OPC,e,d,t,p,l FILE_LINE_MACRO)
  #define TRACE_IP(s,r,u,p,l)                                   vsi_o_primsend(s,TC_DATA,r,FRM_PCO_NAME,(IP_TRACE_OPC|u),p,l FILE_LINE_MACRO)
  #define TRACE_PRIMDUMP(s,r,o,p,l)                             vsi_o_primsend(s,TC_DATA,r,FRM_PCO_NAME,o,p,l FILE_LINE_MACRO)
#else
  #define TRACE_MEMORY(s,p,l)                                   ((void)(0))
  #define TRACE_MEMORY_PRIM(s,r,o,p,l)                          ((void)(0))
  #define TRACE_USER_CLASS_MEMORY_PRIM(s,r,o,p,l)               ((void)(0))
  #define TRACE_BINDUMP(s,m,d,p,l)                              ((void)(0))
  #define TRACE_HEXDUMP(s,p,l)                                  ((void)(0))
  #define TRACE_SDU(s,r,e,d,t,p,l)                              ((void)(0))
  #define TRACE_IP(s,r,u,p,l)                                   ((void)(0))
  #define TRACE_PRIMDUMP(s,r,o,p,l)                             ((void)(0))
#endif

#ifdef TRACE_ERR
  #define TRACE_ERROR(a)                                        vsi_o_error_ttrace(a)   
#else
  #define TRACE_ERROR(a)                                        ((void)(0))
#endif

#ifdef TRACE_IS                                                 /* entity internal signals */
  #define TRACE_ISIG(a)                                         vsi_o_class_ttrace( TC_ISIG,a)
#else
  #define TRACE_ISIG(a)                                         ((void)(0))
#endif

#ifdef TRACE_PRIM
  #define PTRACE_IN(OPC)                                        vsi_o_ptrace (VSI_CALLER OPC, 0)
  #define PTRACE_OUT(OPC)                                       vsi_o_ptrace (VSI_CALLER OPC, 1)
#else
  #define PTRACE_IN(OPC)                                        ((void)(0))
  #define PTRACE_OUT(OPC)                                       ((void)(0))
#endif

#ifdef TRACE_PRF
  #define PRF_LOG_FUNC_ENTRY(F)                                 prf_log_function_entry(F)
  #define PRF_LOG_FUNC_EXIT(F)                                  prf_log_function_exit(F)
  #define PRF_LOG_POI(P)                                        prf_log_point_of_interest(P)
#else
  #define PRF_LOG_FUNC_ENTRY(F)                                 ((void)(0))
  #define PRF_LOG_FUNC_EXIT(F)                                  ((void)(0))
  #define PRF_LOG_POI(P)                                        ((void)(0))
#endif

#undef TRACE_ASSERT
#if defined NDEBUG
  #define TRACE_ASSERT(e)                                       ((void)(0))
#else                                                           
  #define _TRACE_ASSERT(e)                                      ((void)((e)?0:vsi_o_assert(0,0x8000,__FILE__,__LINE__,#e)))
  #define TRACE_ASSERT(e)                                       ((void)((e)?0:vsi_o_assert(0,0x8000,__FILE__,__LINE__,"Assertion failed:" #e)))
#endif

#ifdef    assert
  #undef  assert
#endif

  #define assert TRACE_ASSERT                                   

#ifdef TRACE_SET_STATE
  #define SET_STATE(PROCESS,STATE)\
                { vsi_o_state_ttrace ( #PROCESS ":%s -> " #STATE,\
                                       PROCESS##_NAME [ ENTITY_DATA->state[PROCESS] ]);\
                                       ENTITY_DATA->state[PROCESS] = STATE;}
#else /* TRACE_SET_STATE */
  #define SET_STATE(PROCESS,STATE)    (ENTITY_DATA->state[PROCESS] = STATE)
#endif /* TRACE_SET_STATE */

#ifdef TRACE_GET_STATE
  #define GET_STATE(PROCESS)\
                  (vsi_o_state_ttrace (#PROCESS ":%s",\
                                       PROCESS##_NAME [ ENTITY_DATA->state[PROCESS] ]),\
                                       ENTITY_DATA->state[PROCESS] )
#else /* TRACE_GET_STATE */
  #define GET_STATE(PROCESS)           ENTITY_DATA->state[PROCESS]
#endif /* TRACE_GET_STATE */


#endif /* VSI_H */

