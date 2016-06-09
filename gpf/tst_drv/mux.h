/* 
+------------------------------------------------------------------------------
|  File:       mux.h
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
|  Purpose :  Definitions for the Protocol Stack Entity mux.
+----------------------------------------------------------------------------- 
*/ 

#ifndef MUX_H
#define MUX_H

#ifndef _ENTITY_PREFIXED
  #define _ENTITY_PREFIXED(N) tst_##N
#endif

/*==== INCLUDES =============================================================*/

#ifndef SAP_DTI2
#define SAP_DTI2
#endif

/*==== CONSTS ===============================================================*/

#define ENCODE_OFFSET    0             /* Bitoffset for encoding/decoding */
#define hCommYYY         mux_hCommYYY  /* Communication handle */

/* Timer definitions */
#define T001             0
#define T002             1

/* Timer durations */
#define T001_VALUE       1000          /* 1 second */
#define T002_VALUE       3000          /* 3 seconds */

/*
 * information for dti library
 */
#define MTST_DTI_DN_INTERFACE  0
#define MTST_DTI_DN_CHANNEL    0
#define MTST_DTI_DN_INSTANCE   0

/* make the pei_create, .. functions unique */
#define pei_create       mux_pei_create

#define pei_dti_dti_connect_req    _ENTITY_PREFIXED(pei_dti_dti_connect_req)
#define pei_dti_dti_connect_cnf    _ENTITY_PREFIXED(pei_dti_dti_connect_cnf)
#define pei_dti_dti_connect_ind    _ENTITY_PREFIXED(pei_dti_dti_connect_ind)
#define pei_dti_dti_connect_res    _ENTITY_PREFIXED(pei_dti_dti_connect_res)
#define pei_dti_dti_disconnect_req _ENTITY_PREFIXED(pei_dti_dti_disconnect_req)
#define pei_dti_dti_disconnect_ind _ENTITY_PREFIXED(pei_dti_dti_disconnect_ind)
#define pei_dti_dti_ready_ind      _ENTITY_PREFIXED(pei_dti_dti_ready_ind)
#define pei_dti_dti_data_req       _ENTITY_PREFIXED(pei_dti_dti_data_req)
#define pei_dti_dti_data_ind       _ENTITY_PREFIXED(pei_dti_dti_data_ind)
#define pei_dti_dti_getdata_req    _ENTITY_PREFIXED(pei_dti_dti_getdata_req)
#ifdef _SIMULATION_
#define pei_dti_dti_data_test_req  _ENTITY_PREFIXED(pei_dti_dti_data_test_req)
#define pei_dti_dti_data_test_ind  _ENTITY_PREFIXED(pei_dti_dti_data_test_ind)
#endif /* _SIMLUATION_ */

#define pei_sig_callback                                    \
        _ENTITY_PREFIXED(pei_sig_callback                 )
#define sig_dti_pei_connection_opened_ind                   \
        _ENTITY_PREFIXED(sig_dti_pei_connection_opened_ind)
#define sig_dti_pei_connection_closed_ind                   \
        _ENTITY_PREFIXED(sig_dti_pei_connection_closed_ind)
#define sig_dti_pei_tx_buffer_full_ind                       \
        _ENTITY_PREFIXED(sig_dti_pei_tx_buffer_full_ind    )
#define sig_dti_pei_tx_buffer_ready_ind                      \
        _ENTITY_PREFIXED(sig_dti_pei_tx_buffer_ready_ind   )
#define sig_dti_pei_data_received_ind                        \
        _ENTITY_PREFIXED(sig_dti_pei_data_received_ind     )
        

#ifdef MUX_PEI_C
static const T_STR_IND tmr_name_to_ind[] = 
{
  { "T001",   T001 },
  { "T002",   T002 },
  { NULL,     0 }
};
#endif

#define MAX_SEND_BUF_COUNT  40 

/*==== TYPES =================================================================*/

/* MUX global typedefs */

typedef struct
{
  USHORT Handle;
  UBYTE  dti_state; /* state variable for dti library */
  USHORT EnabledSignalType;
  T_DRV_CB_FUNC Callback;
  T_HANDLE h_comm_mtst;
  char Connected;
  T_desc2 *recv_data;
  USHORT send_data_buf_count;
} T_MUX_DATA;

/*==== EXPORTS ===============================================================*/

short pei_create (T_PEI_INFO const **info);

/*
 * wrapping functions for dtilib primitives
 */

EXTERN const void pei_dti_dti_connect_ind (
              T_DTI2_CONNECT_IND   *dti_connect_ind);
EXTERN const void pei_dti_dti_connect_cnf (
              T_DTI2_CONNECT_CNF   *dti_connect_cnf);
EXTERN const void pei_dti_dti_disconnect_ind (
              T_DTI2_DISCONNECT_IND   *dti_disconnect_ind);
EXTERN const void pei_dti_dti_data_ind (
              T_DTI2_DATA_IND   *dti_data_ind);
EXTERN const void pei_dti_dti_ready_ind (
              T_DTI2_READY_IND   *dti_ready_ind);
#ifdef _SIMULATION_
EXTERN const void pei_dti_dti_data_test_ind (
              T_DTI2_DATA_TEST_IND   *dti_data_test_ind);
#endif  /* _SIMULATION_ */

/*
 * this function is called by DTILIB
 */
EXTERN void         pei_sig_callback
                          (
                            U8               instance,
                            U8               interfac,
                            U8               channel, 
                            U8               reason,
                            T_DTI2_DATA_IND *dti_data2_ind
                          );

/*
 * these functions are called by pei_sig_callback
 */

EXTERN const void         sig_dti_pei_connection_opened_ind();
EXTERN const void         sig_dti_pei_connection_closed_ind();

EXTERN const void         sig_dti_pei_tx_buffer_full_ind();
EXTERN const void         sig_dti_pei_tx_buffer_ready_ind();

EXTERN const void         sig_dti_pei_data_received_ind
                          (
                            T_DTI2_DATA_IND *dti_data_req
                          );



#ifdef MUX_PEI_C

/* Entity data base */
T_MUX_DATA               mux_data;
/* DTI connection handle for DTI library      */
DTI_HANDLE               mtst_hDTI;

/* Communication handles */

#else  /* MUX_PEI_C */

/* Entity data base */
extern T_MUX_DATA        mux_data;
/* DTI connection handle for DTI library      */
EXTERN DTI_HANDLE        mtst_hDTI;

#endif /* MUX_PEI_C */

#endif /* !MUX_H */
