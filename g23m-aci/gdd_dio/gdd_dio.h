/* 
+------------------------------------------------------------------------------
|  File:       gdd_dio.h
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
|  Purpose :  Definitions for the Protocol Stack Entity gdd_dio.
+----------------------------------------------------------------------------- 
*/ 

#ifndef GDD_DIO_H
#define GDD_DIO_H

/*==== INCLUDES =============================================================*/

#include "gdd_dio_data.h"

/*==== CONSTS ===============================================================*/

#define VSI_CALLER       GDD_DIO_handle,
#define ENTITY_DATA      gdd_dio_data

#define ENCODE_OFFSET    0             /* Bitoffset for encoding/decoding */
#define hCommGDD_DIO         gdd_dio_hCommGDD_DIO  /* Communication handle */

/* Timer definitions */
#define T001             0
#define T002             1

/* Timer durations */
#define T001_VALUE       1000          /* 1 second */
#define T002_VALUE       3000          /* 3 seconds */

/* --------------  Trace classes specific to GDD_DIO entity -----------------*/

/* Trace class user 1 is used for tracing GDD and DIO signals */
#define TC_SIGNALS           TC_USER1
/* Trace class user 2 is used for function traces of the function which are
   involved in the actual data flow. Since we expect these to be hit
   frequently, this special trace class has been created. */
#define TC_FUNC_DATA_FLOW    TC_USER2

/* Trace class user 3 is for all functions in the "gdd_sys.h" header */
#define TC_SYS_INTERFACE     TC_USER3


/* Set the following #define if a context switch in DTX is to be made.
   (i.e. a context switch to avoid calling a client into PSI */
/*#define GDD_MAKE_DTX_CONTEXT_SWITCH*/

#ifdef GDD_MAKE_DTX_CONTEXT_SWITCH
/** Signals (internal to this entity)
 * The signal must also carry the connection handle, which is stored
 * in bit 7. This means that we currently limit our device range to 0..127
 */
#define GDD_DIO_SIGNAL_SEND_DATA       0x00000000
#define GDD_DIO_SIGNAL_RECEIVE_DATA    0x00000080

#define GDD_DIO_SIGNAL_MASK            0x00000080
#define GDD_DIO_SIGNAL_CON_HANDLE_MASK 0xFFFFFF7F
#endif /* GDD_MAKE_DTX_CONTEXT_SWITCH */

typedef struct
{
  T_GDD_BUF        * buf;
  T_GDD_CON_HANDLE   con_handle;
} T_GDD_DIO_SIGNAL_DATA;


/*==== TYPES =================================================================*/

/* GDD_DIO global typedefs */

typedef struct                         /* T_GDD_DIO_DATA */
{
  /* Kernel service data */  
  T_GDD_DIO_KER_DATA    ker;

  /** Connection service data array (one structure for each connection) */
  T_GDD_DIO_CON_DATA  * con_arr;

  /** Flag set when the con_arr memory was allocated in this entity */
  BOOL                  con_arr_mem_allocated;

  /** Max number of connections that client wishes to open */
  U16                   max_con;

  /** External instnace id - passed in when calling gdd_init. */
  T_GDD_INST_ID         inst_id;

  /** DIO driver number */
  T_GDD_DIO_DRV_NUM     drv_num;

  /** DIO device range start (0 if not yet initialized by DIO IL) */
  U32                   device_range_start;

  /** DIO device range end (0 if not yet initialized by DIO IL) */
  U32                   device_range_end;

  /** DIO driver handle */
  U16                   drv_handle;

  /** DIO callback */
  T_DRV_CB_FUNC         signal_callback;


} T_GDD_DIO_DATA;

/*==== EXPORTS ===============================================================*/

short pei_create (T_PEI_INFO **info);

#ifdef GDD_DIO_PEI_C

/* Entity data base */
T_GDD_DIO_DATA           gdd_dio_data_base [GDD_NUM_INSTS];
BOOL                     gdd_dio_init_flag [GDD_NUM_INSTS];

/* Communication handles */
T_HANDLE                 hCommGDD_DIO = VSI_ERROR;
T_HANDLE                 GDD_DIO_handle;

#else  /* GDD_DIO_PEI_C */

extern T_GDD_DIO_DATA    gdd_dio_data_base [];
extern BOOL              gdd_dio_init_flag [];
extern T_HANDLE          hCommGDD_DIO;
extern T_HANDLE          GDD_DIO_handle;

#endif /* GDD_DIO_PEI_C */

#endif /* !GDD_DIO_H */
