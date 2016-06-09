/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  GSM
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
|  Purpose :  Definition of global constants, types, and macros
|             for the GSM Protocol Stack
+----------------------------------------------------------------------------- 
*/ 

#ifndef GSM_H
#define GSM_H

#ifndef CCONST_CDG
#include "mconst.cdg"   /* MAX_BITSTREAM_LEN */
#endif /* CCONST_CDG */
#ifndef PCONST_CDG
#include "pconst.cdg"   /* MAX_PSTRUCT_LEN   */
#endif /* PCONST_CDG */

/*==== CONSTANTS ==================================================*/

#define MAX_2_PRM      1
#define MAX_4_PRM      3
#define MAX_8_PRM      7
#define MAX_16_PRM    15
#define MAX_32_PRM    31

#define PRM_MASK       0x00FF             /* protocol primitive mask*/
#define OPC_MASK       0xFF00             /* entity mask            */
#define SYS_MASK       0x8000             /* system primitive mask  */

#define SYS_CONFIG_REQ 0xF000

#define L3_CODING_OFFSET   32             /* coding offset in bits */

/*
 * Not Present Value
 */

#define NOT_PRESENT_8BIT        0xFF
#define NOT_PRESENT_CHAR        (CHAR)0xFF
#define NOT_PRESENT_16BIT       0xFFFF
#define NOT_PRESENT_32BIT       0xFFFFFFFFL
#define INVALID_ADDRESS         0xFFFFFFFFL
#define SET_NOT_PRESENT(A)      ((A) = ((sizeof ((A)) EQ 1)\
                                        ? NOT_PRESENT_8BIT\
                                        : (sizeof ((A)) EQ 2)\
                                            ? NOT_PRESENT_16BIT\
                                            : NOT_PRESENT_32BIT))

#define IS_PRESENT(A)           ((A) NEQ ((sizeof ((A)) EQ 1)\
                                        ? NOT_PRESENT_8BIT\
                                        : (sizeof ((A)) EQ 2)\
                                            ? NOT_PRESENT_16BIT\
                                            : NOT_PRESENT_32BIT)) /* VK 09-12-96 */

#ifndef NEW_FRAME
/*
 * Static Configurations
 */

#ifndef TC_FUNC
#define TC_FUNC 1
#endif

#ifdef TRACE_FKT
#define TRACE_FUNCTION(a)  vsi_o_trace(VSI_CALLER TC_FUNC,a);
#else
// China change HM 6.07.00, one of this commented out
#define TRACE_FUNCTION(a)
//#define TRACE_FUNCTION(a)  vsi_o_mtrace(a);
#endif

#ifndef TC_EVE
#define TC_EVE   2
#endif

#ifdef TRACE_EVE
#define TRACE_EVENT(a)     vsi_o_trace(VSI_CALLER TC_EVENT,a);
#else
#define TRACE_EVENT(a)
#endif

#ifndef TC_ERROR
#define TC_ERROR 4
#endif
#ifdef TRACE_ERR
#define TRACE_ERROR(a)     vsi_o_trace(VSI_CALLER TC_ERROR,a);
#else
#define TRACE_ERROR(a)
#endif

#ifndef TC_PRIM
#define TC_PRIM 8
#endif

#ifdef TRACE_PRIM
#define TRACE_PRIMITIVE(a)  vsi_o_trace(VSI_CALLER TC_PRIM,a);
#else
#define TRACE_PRIMITIVE(a)
#endif

#ifndef TC_SYSTEM
#define TC_SYSTEM 16
#endif

 /*
 * Assert wrapper
 */

#if defined NDEBUG
    #define TRACE_ASSERT(e) ((void)0)
#else
  #ifdef SHARED_VSI
    #define TRACE_ASSERT(e) if ( !(e) ) vsi_o_assert("",#e,__FILE__,__LINE__)
  #else
    #define TRACE_ASSERT(e) if ( !(e) ) vsi_o_assert(#e,__FILE__,__LINE__)
  #endif
#endif

#ifdef    assert
  #undef  assert
#endif
  #define assert TRACE_ASSERT

#endif /* NEW_FRAME */

/*
 * Memory Management(I)
 */

#ifdef OPTION_REF
#define FREE_BUFFER(p)     vsi_c_free(VSI_CALLER (void **)&p)
/*lint -e773 Expression-like macro not parenthesized*/
#define NEW_BUFFER(p,s)    T_PRIM *p = vsi_c_new(VSI_CALLER (T_VSI_CSIZE)((s)+sizeof(T_PRIM_HEADER)))
/*lint +e773*/
#else
#define FREE_BUFFER(p)
#define NEW_BUFFER(p,s)    UBYTE buf_##p [(s)+sizeof(T_PRIM_HEADER)]; T_PRIM *p=(T_PRIM*)p1
#endif

#if 0 //!defined (_TMS470)
#define Sprintf sprintf
#else
int sprintf( char *buffer, const char *format, ... );
#endif

#ifndef NEW_FRAME

#if 1 //#ifdef _TMS470
  #define SET_PRIM_OPCODE(P,O) P->custom.opc = O; P->custom.opc2 = 0
#else
  #define SET_PRIM_OPCODE(P,O) P->custom.opc = O
#endif

#ifdef OPTION_LENGTH
  #define SEND_PRIM(E,P,O,T)       SET_PRIM_OPCODE (P,O); vsi_c_send (VSI_CALLER hComm##E, P, sizeof(T_PRIM_HEADER) + sizeof (T))
  #define SEND_PRIM_0(E,P,O)       SET_PRIM_OPCODE (P,O); vsi_c_send (VSI_CALLER hComm##E, P, sizeof(T_PRIM_HEADER))
  #define SEND_PRIM_SDU(E,P,O,T,L) SET_PRIM_OPCODE (P,O); vsi_c_send (VSI_CALLER hComm##E, P, (T_VSI_CSIZE) (sizeof(T_PRIM_HEADER) + sizeof (T) + (L) - SDU_TRAIL))
#else
  #define SEND_PRIM(E,P,O,T)       SET_PRIM_OPCODE (P,O); vsi_c_send (VSI_CALLER hComm##E, P, sizeof(T_PRIM_HEADER) + sizeof (T))
  #define SEND_PRIM_0(E,P,O)       SET_PRIM_OPCODE (P,O); vsi_c_send (VSI_CALLER hComm##E, P, sizeof(T_PRIM_HEADER))
  #define SEND_PRIM_SDU(E,P,O,T,L) SET_PRIM_OPCODE (P,O); vsi_c_send (VSI_CALLER hComm##E, P, (T_VSI_CSIZE) (sizeof(T_PRIM_HEADER) + sizeof (T) + (L) - SDU_TRAIL))
#endif

#endif /* NEW_FRAME */
/*
 * declares a pointer variable (D) of the type T
 * and initialize it with the startaddress of the data part
 * of the primitive P
 */
#ifndef ALLOC_CHECK
  #if defined (_TMS470) AND defined (NWARN)
    #define PRIM_ACCESS(P,S) &P->data
  #else
    #define PRIM_ACCESS(P,S) P->data
  #endif
#else
 #define PRIM_ACCESS(P,S) vsi_c_access(VSI_CALLER P,S)
#endif
/*lint -e773 Expression-like macro not parenthesized*/
#define PRIM_CAST(P,D,T) T *D = (T *) PRIM_ACCESS (P, sizeof (T))
/*lint +e773*/
/*
 * declares a pointer variable (D) of the type T and
 * initialize it with the start address of the global declared
 * buffer _decodedMsg[] wich contains the decoded message
 * structure before encoding or after decoding the message
 * with CCD.
 */
/*lint -e773 Expression-like macro not parenthesized*/
#define MSG_CAST(D,T)    T *D = (T *) _decodedMsg
/*lint +e773*/
/*
 * Memory Management(II)
 */

#ifndef NEW_FRAME

#if defined (_TMS470) AND defined (NWARN)
#define  P2D(P)                    (&(P)->data)
#else
#define  P2D(P)                    ((P)->data)
#endif
#define  D2P(D)                    ((T_PRIM*)((T_PRIM_HEADER*)(D)-1))

#ifdef   ALLOC_CHECK
#define  P2D_AC(P,T)               PRIM_ACCESS(P,sizeof(T))
#else
#define  P2D_AC(P,T)               P2D(P)
#endif

#define  PRIM_TO_DATA              P2D
#define  DATA_TO_PRIM              D2P

#define  P_OPC(P)                  (P)->custom.opc
#define  P_OPC2(P)                 (P)->custom.opc2
#define  P_LEN(P)                  (P)->custom.len
#define  P_TID(P)                  (P)->custom.tid
#define  P_SDU(P)                  (P)->custom.sdu

#define  D_OPC(D)                  P_OPC(D2P(D))
#define  D_OPC2(D)                 P_OPC2(D2P(D))
#define  D_LEN(D)                  P_LEN(D2P(D))
#define  D_TID(D)                  P_TID(D2P(D))
#define  D_SDU(D)                  P_SDU(D2P(D))

#define  D_SDU_LEN(D)              D_SDU(D)->l_buf
#define  D_SDU_OFF(D)              D_SDU(D)->o_buf

#define  BITS_PER_BYTE             8
#define  BYTELEN(BITLEN)           ((BITLEN)<=0?0:((((BITLEN)-1)/BITS_PER_BYTE)+1))

#define  PRIM_SIZE(D)              (sizeof(T_PRIM_HEADER)+sizeof(*(D)))
#define  PRIM_SIZE_SDU(D)          (PRIM_SIZE(D)+BYTELEN((D)->sdu.o_buf+(D)->sdu.l_buf))

#define  PSIZE(D)                  (D_LEN(D)+((D_SDU(D)==0)?0:(BYTELEN(D_SDU_LEN(D)+D_SDU_OFF(D))-SDU_TRAIL)))

#define  SIZ(T)                    (sizeof(T_PRIM_HEADER)+sizeof(T))

#define  SIZ_SDU(T,M)              (SIZ(T)+BYTELEN((M)+ENCODE_OFFSET)-SDU_TRAIL)

#define  P_ALL(T)                  (T_PRIM *) vsi_c_new(VSI_CALLER (T_VSI_CSIZE)SIZ(T) )
#define  P_ALL_SDU(T,M)            (T_PRIM *) vsi_c_new(VSI_CALLER (T_VSI_CSIZE)SIZ_SDU(T,M))

#endif /* NEW_FRAME */

#define  OFFSETOF(P,C)             ((char*)&(P->C) - (char*)P)

#ifdef NTRACE
  #define  xxxNotify(A,B,C,D,E)
#else
  #define  xxxNotify(A,B,C,D,E)      /* not used yet */
#endif


#define  ACT_PALLOC                0x01
#define  ACT_PALLOC_SDU            0x02
#define  ACT_PALLOC_MSG            0x03
#define  ACT_PREUSE                0x04
#define  ACT_PREUSE_SDU            0x05
#define  ACT_PREUSE_MSG            0x06
#define  ACT_PFREE                 0x07
#define  ACT_PSEND                 0x08
#define  ACT_PSTORE                0x09
#define  ACT_PRETRIEVE             0x0A
#define  ACT_PACCESS               0x0B
#define  ACT_PPASS                 0x0C
#define  ACT_PCAST                 0x0D

/*** memory alloction  ***/


#ifndef NEW_FRAME

#define PSENDX PSEND

#define  PALLOC(D,T)               T_##T  *D    = (T_##T*)P2D(P_ALL(T_##T));\
                                   D_OPC(D)     = (T);\
                                   D_OPC2(D)    = 0;\
                                   D_LEN(D)     = SIZ(T_##T);\
                                   D_SDU(D)     = 0;\
                                   xxxNotify(ACT_PALLOC,VSI_CALLER,__FILE__,__LINE__,D2P(D))

#define  PALLOC_MSG(D,T,M)         T_##T  *D    = (T_##T*)P2D(P_ALL_SDU(T_##T,BSIZE_##M));\
                                   D_OPC(D)     = (T);\
                                   D_OPC2(D)    = 0;\
                                   D_LEN(D)     = SIZ(T_##T);\
                                   D_SDU(D)     = &((D)->sdu);\
                                   D_SDU_LEN(D) = (BSIZE_##M);\
                                   D_SDU_OFF(D) = ENCODE_OFFSET;\
                                   xxxNotify(ACT_PALLOC_MSG,VSI_CALLER,__FILE__,__LINE__,D2P(D))

#define  PALLOC_SDU(D,T,N)         T_##T  *D    = (T_##T*)P2D(P_ALL_SDU(T_##T,N));\
                                   D_OPC(D)     = (T);\
                                   D_OPC2(D)    = 0;\
                                   D_LEN(D)     = SIZ(T_##T);\
                                   D_SDU(D)     = &((D)->sdu);\
                                   D_SDU_LEN(D) = (N);\
                                   D_SDU_OFF(D) = ENCODE_OFFSET;\
                                   xxxNotify(ACT_PALLOC_SDU,VSI_CALLER,__FILE__,__LINE__,D2P(D))

#define  PREUSE(D0,D,T)            T_##T  *D    = (T_##T*)P2D_AC(D2P(D0),T);\
                                   D_OPC(D)     = (T);\
                                   D_OPC2(D)    = 0;\
                                   D_LEN(D)     = SIZ(T_##T);\
                                   D_SDU(D)     = 0;\
                                   xxxNotify(ACT_REUSE,VSI_CALLER,__FILE__,__LINE__,D2P(D))

#define  PREUSE_MSG(D,T,M)         T_##T  *D    = (T_##T*)P2D_AC(D2P(D0),T);\
                                   D_OPC(D)     = (T);\
                                   D_OPC2(D)    = 0;\
                                   D_LEN(D)     = SIZ(T_##T);\
                                   D_SDU(D)     = &((D)->sdu);\
                                   D_SDU_LEN(D) = (BSIZE_##M);\
                                   D_SDU_OFF(D) = ENCODE_OFFSET;\
                                   xxxNotify(ACT_PREUSE_MSG,VSI_CALLER,__FILE__,__LINE__,D2P(D))

#define  PREUSE_SDU(D,T,N)         T_##T  *D    = (T_##T*)P2D_AC(D2P(D0),T);\
                                   USHORT BSIZE_##N;\
                                   D_OPC(D)     = (T);\
                                   D_OPC2(D)    = 0;\
                                   D_LEN(D)     = SIZ(T_##T);\
                                   D_SDU(D)     = &((D)->sdu);\
                                   D_SDU_LEN(D) = (N);\
                                   D_SDU_OFF(D) = ENCODE_OFFSET;\
                                   xxxNotify(ACT_PREUSE_SDU,VSI_CALLER,__FILE__,__LINE__,D2P(D))

#define  PFREE(D)                  { void *z=(void*)D2P(D);\
                                   vsi_c_free(VSI_CALLER (void **)&z);\
                                   xxxNotify(ACT_PFREE,VSI_CALLER,__FILE__,__LINE__,D2P(D)); }

#define  PSEND(E,D)                PTRACE_OUT(D_OPC(D));\
                                   vsi_c_send (VSI_CALLER hComm##E, D2P(D), (T_VSI_CSIZE) PSIZE(D));\
                                   xxxNotify(ACT_PSEND,VSI_CALLER,__FILE__,__LINE__,D2P(D))

#define  PPASS(D0,D,T)             T_##T  *D    = (T_##T*)P2D_AC(D2P(D0),T);\
                                   D_OPC(D)     = (T);\
                                   xxxNotify(ACT_PPASS,VSI_CALLER,__FILE__,__LINE__,D2P(D))

#define  PACCESS(D)                xxxNotify(ACT_PACCESS,VSI_CALLER,__FILE__,__LINE__,D2P(D))


#endif /* NEW_FRAME */

/*
 * to achieve backward compatibility
 */

#if defined (NEW_FRAME)
#ifdef TI_PS_HCOMM_CHANGE
#define PSENDX(A,B) PSEND(_hComm##A,B)
#define _hCommACI hCommACI
#else
#define PSENDX(A,B) PSEND(hComm##A,B)
#endif
#else
#define PSENDX PSEND
#endif



#define  PRETRIEVE                 /* NOT DEFINED YET */


#define  MCAST(D,T)                MSG_CAST(D,T_##T)
#define  PCAST(P,D,T)              T_##T  *D    = (T_##T*)P2D_AC(P,T);\
                                   P_OPC(P)=T;\
                                   xxxNotify(ACT_PCAST,VSI_CALLER,__FILE__,__LINE__,D2P(D))

#define  PCOPY(DEST,SOURCE)        memcpy(DEST,SOURCE,PSIZE(SOURCE)-sizeof(T_PRIM_HEADER))

/*** trace primitive, trace state ***/

#ifndef NEW_FRAME

#ifdef NTRACE

  #define SET_STATE(PROCESS,STATE)    ENTITY_DATA->state[PROCESS] = STATE
  #define GET_STATE(PROCESS)          ENTITY_DATA->state[PROCESS]

  #define PTRACE_IN(OPC)
  #define PTRACE_OUT(OPC)

#else

  #define SET_STATE(PROCESS,STATE)\
                ENTITY_DATA->state[PROCESS] =\
                vsi_o_strace (VSI_CALLER\
                              PROCESS_NAME[PROCESS],\
                              PROCESS##_NAME [ ENTITY_DATA->state[PROCESS] ],\
                              PROCESS##_NAME [              STATE          ],\
                                                            STATE              )

  #define GET_STATE(PROCESS)\
                vsi_o_strace (VSI_CALLER\
                              PROCESS_NAME[PROCESS],\
                              PROCESS##_NAME [ ENTITY_DATA->state[PROCESS] ],\
                                                            NULL            ,\
                                               ENTITY_DATA->state[PROCESS]     )

  #define PTRACE_IN(OPC)              vsi_o_ptrace (VSI_CALLER OPC, 0)
  #define PTRACE_OUT(OPC)             vsi_o_ptrace (VSI_CALLER OPC, 1)

#endif

#endif /* NEW_FRAME */
/*
 * Timer, Jump Table, Configuration
 */

#ifdef OPTION_TIMER
#define TSTART(i,h,v) tim_start_timer(i,h,v)
#else
#define TSTART(i,h,v) vsi_t_start(VSI_CALLER h,v)
#endif

#ifdef OPTION_RELATIVE
#define JUMP(a)  (a+offset)
#else
#define JUMP(a) (a)
#endif

#ifdef OPTION_SET_CONFIG_ONLY
#define GET_CONFIG(s,i)
#else
#define GET_CONFIG(s,i) pei_get_config(s,i)
#endif

/* Cause Concept (begin) */
/* 
 * For a definition of the cause concept please refer to document number
 * 8443.711, named in February 2002 "cause_concept.doc"
 */

/* definitions for calling the macros */
/* DefinedBy bit */
#define DEFBY_STD    0
#define DEFBY_CONDAT 1
/* OriginatingSide bit */
#define ORIGSIDE_NET 0
#define ORIGSIDE_MS  1
/* 
 * note that OriginatingEntity definitions are in the SAP GSMCOM, which is a 
 * central (pseudo) SAP and the definitions are referenced in the using 
 * "real" SAPs, like e.g. MNSS SAP for the originating entity SS
 */

/* need to decide whether to mask ORIGIN_ENTITY (max. 6 bits) in the following macro */
#define CAUSE_MAKE(DEFINED_BY, ORIGIN_SIDE, ORIGIN_ENTITY, CAUSE_VALUE) \
          (USHORT)(((CAUSE_VALUE) & 0x00FF) |\
          ((DEFINED_BY)    << 15) |\
          ((ORIGIN_SIDE)   << 14) |\
          ((ORIGIN_ENTITY) <<  8))

/* Get the cause value without the originating entity */
#define GET_CAUSE_VALUE(cause)          (UBYTE)((cause) & 0xFF)

/* Get the entity originating the cause */
#define GET_CAUSE_ORIGIN_ENTITY(cause)  (UBYTE)(((cause) >> 8) & 0x3F)

/* Get the DefinedBy bit */
#define GET_CAUSE_DEFBY(cause)  (UBYTE)((cause) >> 15)

/* Get the OriginatingSide bit */
#define GET_CAUSE_ORIGSIDE(cause)  (UBYTE)(((cause) >> 14) & 0x01)

/* Determine if a cause is invalid */
#define IS_CAUSE_INVALID(cause)   (((cause) & 0x80FF) EQ 0x80FF)

/* Cause Concept (end) */


/*==== TYPES ======================================================*/
#if defined (_TMS470) && defined (NWARN)
typedef struct
{
  UBYTE           x [MAX_PSTRUCT_LEN
                     + (
                         L3_CODING_OFFSET
                         + MAX_BITSTREAM_LEN
                        ) / 8 + 1
                    ];
} T_PARAMETER;

typedef struct
{
  T_PRIM_HEADER   custom;
  T_PARAMETER     data;
} T_PRIM;
#else
typedef struct
{
  T_PRIM_HEADER   custom;
  UBYTE           data [MAX_PSTRUCT_LEN
                     + (
                         L3_CODING_OFFSET
                         + MAX_BITSTREAM_LEN
                        ) / 8 + 1
                    ];
} T_PRIM;
#endif

/*==== EXPORT =====================================================*/



/*
 * Multithreading
 */



#ifdef OPTION_MULTITHREAD
  #if defined (ENTITY_CST)
    #define _ENTITY_PREFIXED(N) cst_##N
  #elif defined (ENTITY_ACI)
    #define _ENTITY_PREFIXED(N) aci_##N
  #elif defined (ENTITY_MFW)
    #define _ENTITY_PREFIXED(N) aci_##N
  #elif defined (ENTITY_MMI)
    #define _ENTITY_PREFIXED(N) aci_##N
  #elif defined (ENTITY_SMI)
    #define _ENTITY_PREFIXED(N) aci_##N
  #elif defined (ENTITY_SIM)
    #define _ENTITY_PREFIXED(N) sim_##N
  #elif defined (ENTITY_PL)
    #define _ENTITY_PREFIXED(N) pl_##N
  #elif defined (ENTITY_L1)
    #define _ENTITY_PREFIXED(N) l1_##N
  #elif defined (ENTITY_CC)
    #define _ENTITY_PREFIXED(N) cc_##N
  #elif defined (ENTITY_SS)
    #define _ENTITY_PREFIXED(N) ss_##N
  #elif defined (ENTITY_ESIM)
    #define _ENTITY_PREFIXED(N) esim_##N
  #elif defined (ENTITY_SMS)
    #define _ENTITY_PREFIXED(N) sms_##N
  #elif defined (ENTITY_MM)
    #define _ENTITY_PREFIXED(N) mm_##N
  #elif defined (ENTITY_RR)
    #define _ENTITY_PREFIXED(N) rr_##N
  #elif defined (ENTITY_DL)
    #define _ENTITY_PREFIXED(N) dl_##N
  #elif defined (ENTITY_L2R)
    #define _ENTITY_PREFIXED(N) l2r_##N
  #elif defined (ENTITY_TRA)
    #define _ENTITY_PREFIXED(N) tra_##N
  #elif defined (ENTITY_RLP)
    #define _ENTITY_PREFIXED(N) rlp_##N
  #elif defined (ENTITY_T30)
    #define _ENTITY_PREFIXED(N) t30_##N
  #elif defined (ENTITY_FAD)
    #define _ENTITY_PREFIXED(N) fad_##N
  #elif defined (ENTITY_RA)
    #define _ENTITY_PREFIXED(N) ra_##N
  #elif defined (ENTITY_WAP)
    #define _ENTITY_PREFIXED(N) wap_##N
  #elif defined (ENTITY_UDP)
    #define _ENTITY_PREFIXED(N) udp_##N
  #elif defined (ENTITY_IP)
    #define _ENTITY_PREFIXED(N) ip_##N
  #elif defined (ENTITY_RRLP)
    #define _ENTITY_PREFIXED(N) rrlp_##N
  #elif defined (ENTITY_PPP)
    #define _ENTITY_PREFIXED(N) ppp_##N
  #elif defined (ENTITY_BTI)
    #define _ENTITY_PREFIXED(N) bti_##N
  #elif defined (ENTITY_L1)
    #ifdef NEW_FRAME
      #include "ofe.h"
    #endif
  #elif defined (ENTITY_TAP)
    #define _ENTITY_PREFIXED(N) tap_##N
  #elif defined (ENTITY_PCO)
    #define _ENTITY_PREFIXED(N) pco_##N
  #elif defined (ENTITY_PAN)
    #define _ENTITY_PREFIXED(N) pan_##N
  #elif defined (ENTITY_TST)
    #define _ENTITY_PREFIXED(N) tst_##N
  #elif defined (ENTITY_APP)
    #define _ENTITY_PREFIXED(N) app_##N
  #endif
#endif

#endif
