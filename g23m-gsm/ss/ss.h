/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  SS
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
|             Supplementary Services.
+----------------------------------------------------------------------------- 
*/ 

#ifndef SS_H
#define SS_H

#include <stdlib.h>
#include <stdio.h>
#if defined (TI_PS_HCOMM_CHANGE)
#include "cl_hComm_handle.h"
#endif
/*
 * Definition whether a shared CCD buffer shall be used
 */
#define SHARED_CCD_BUF

/* #define SS_TEST */ /* set for T() debug outputs */

#define ENCODE_OFFSET            24
#define PD_SS                    11

#ifdef SS_TEST
#define T(f,v)    sprintf ((char*)trc, f, v); TRACE_FUNCTION ((char*)trc)
#else
#define T(f,v)                    ;
#endif

/*
 * Macros (common for SS and CC)
 */

#define GET_SS_STATE(s,t) ((s>>(2*t)) & 0x03)
#define SET_SS_STATE(s,t,STATE) (s=(s & (~(((ULONG)0x03)<<(2*t)))) |(STATE<<(2*t)))


#ifdef FRAME_OFFSET_ZERO

#define GET_PD(s,p)  p = s.buf[3] & 0x0F
#define GET_TI(s,t)  t = (s.buf[3] & 0xF0) >> 4
#define SET_PD(s,p)  s.buf[3] = (s.buf[3] & 0xF0) + p
#define SET_TI(s,t)  s.buf[3] = (s.buf[3] & 0x0F) + (t << 4)

#else

#define GET_PD(s,p)  ccd_decodeByte(s.buf, (USHORT)(s.o_buf + 4), 4, &p)
#define GET_TI(s,t)  ccd_decodeByte(s.buf, s.o_buf, 4, &t)
#define SET_PD(s,p)  ccd_codeByte(s.buf, (USHORT)(s.o_buf - 4), 4, p)
#define SET_TI(s,t)  ccd_codeByte(s.buf, (USHORT)(s.o_buf - 8), 4, t)

#endif

/*
 * States of the Suppl. Services
 */
#define SS_IDLE                   0
#define SS_CONNECTION_PENDING     1
#define SS_CONNECTED              2

/*
 * limits
 */
#define MAX_INST                  16
#define MAX_SIZE_SS_MESSAGE       256

/*
 * Length of uplink messages
 * These are comprised of the minimum 'safe' air i/f message length 
 * according to the SS message catalogue
 */
#define LEN_U_SS_REGISTER           (7*8)
#define LEN_U_SS_RELEASE_COMPLETE   (7*8)
#define LEN_U_SS_FACILITY           (7*8)




/*==== TYPES ======================================================*/

typedef struct
{
  BOOL                  est_flag;

  UBYTE                 pd;
  ULONG                 ss_state;
  T_PRIM              * prim [MAX_INST];
  UBYTE                 ti;
  UINT                  error;
} T_SS_DATA;

EXTERN T_SS_DATA ss_data_base;
#define GET_INSTANCE_DATA  register T_SS_DATA * ss_data = &ss_data_base

/*==== EXPORT =====================================================*/


/*
 * Prototypes Customer Specific Functions
 */

/*
 *  Prototypes SS
 */


/*
 *  SS Suppl. Services
 */

/*
 * suppl. services primitives
 */

EXTERN void ss_init_ss_data         (void);
EXTERN void ss_mmss_error_ind       (T_MMSS_ERROR_IND    *mmss_error_ind);
EXTERN void ss_mmss_establish_cnf   (T_MMSS_ESTABLISH_CNF*mmss_establish_cnf);
EXTERN void ss_mmss_release_ind     (T_MMSS_RELEASE_IND  *mmss_release_ind);
EXTERN void ss_mnss_begin_req       (T_MNSS_BEGIN_REQ    *mnss_begin_req);
EXTERN void ss_mnss_end_req         (T_MNSS_END_REQ      *mnss_end_req);
EXTERN void ss_mnss_facility_req    (T_MNSS_FACILITY_REQ *mnss_facility_req);



/*
 *  suppl. services signalling
 */
EXTERN void ss_b_ss_rel_comp            (void);
EXTERN void ss_d_ss_facility            (void);
EXTERN void ss_d_ss_register            (void);

/*
 *  suppl. services procedures
 */
EXTERN void ss_init                     (void);
EXTERN void send_rel_comp               (U8                  cause);

EXTERN BOOL ss_check_critical_error(UINT ccd_err);
/*
 *  Formatter
 */

/*
 *  formatter primitives
 */
EXTERN void for_init_ss_data         (void);
EXTERN void for_mmss_establish_ind   (T_MMSS_ESTABLISH_IND *mmss_establish_ind);
EXTERN void for_mmss_data_ind        (T_MMSS_DATA_IND     *mmss_data_ind);

/*
 *  formatter signalling
 */
EXTERN void ss_for_data_req                (USHORT               bit_size_message);


/*
 * If all entities are linked into one module this definitions
 * prefixes the global data with the enity name
 */
#if defined (TI_PS_HCOMM_CHANGE)
#if defined (NEW_FRAME)
EXTERN T_HANDLE ss_handle;
#endif
#else /* (TI_PS_HCOMM_CHANGE) */
#ifdef OPTION_MULTITHREAD
  #define hCommMM        _ENTITY_PREFIXED(hCommMM)
  #define hCommMMI       _ENTITY_PREFIXED(hCommMMI)
#endif
#if defined (NEW_FRAME)
EXTERN T_HANDLE         hCommMMI;        /* MM  Communication        */
EXTERN T_HANDLE         hCommMM;         /* MM  Communication        */
EXTERN T_HANDLE         ss_handle;
#else
EXTERN T_VSI_CHANDLE  hCommMMI;        /* MM  Communication        */
EXTERN T_VSI_CHANDLE  hCommMM;         /* MM  Communication        */
#endif
#endif /* (TI_PS_HCOMM_CHANGE) */
/*
 * If all entities are linked into one module this definitions
 * prefixes the global data with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define _decodedMsg   _ENTITY_PREFIXED(_decodedMsg)
#endif

#if !defined SHARED_CCD_BUF
#define CCD_START
#define CCD_END
EXTERN UBYTE          _decodedMsg [];
#else
EXTERN UBYTE *        _decodedMsg;
#define CCD_START _decodedMsg = ccd_begin();
#define CCD_END   ccd_end();
#endif



#endif
