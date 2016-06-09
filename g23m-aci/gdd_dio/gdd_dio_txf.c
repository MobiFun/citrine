/*
+-----------------------------------------------------------------------------
|  File     : gdd_dio_txf.c
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
|  Purpose  : This modul is part of the entity gdd_dio and implements the
|             tx service functions.
+-----------------------------------------------------------------------------
*/


#define ENTITY_GDD_DIO

/*==== INCLUDES =============================================================*/

#include "typedefs.h"   /* to get Condat data types */
#include "vsi.h"        /* to get a lot of macros */

/* DIO stuff */
#include "dio.h"
#include "dio_il/dio_drv.h"


/* GDD_DIO stuff */

/* We must define P_DIO_H, in order to avoid inclusion of p_dio.h.
   This is necessary due to the special fact, that "dio.h", which is included
   in the header "dio_io/dio_drv.h", mirrors the header "p_dio.h". */
#define P_DIO_H

#include "gdd_dio.h"       /* to get the global entity definitions */
#include "gdd_dio_con_mgr.h"
#include "gdd_dio_queue.h"

#include "gdd_dio_txf.h"
#include "gdd_dio_drxf.h"

/*==== CONST ================================================================*/

/*==== LOCAL VARS ===========================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*==== PUBLIC FUNCTIONS =====================================================*/

GLOBAL void gdd_dio_tx_receive_buf(T_GDD_CON_HANDLE con_handle, T_dio_buffer * buf)
{
#ifdef GDD_MAKE_DTX_CONTEXT_SWITCH
  U32 signal = GDD_DIO_SIGNAL_RECEIVE_DATA | (U32)con_handle;
#else /* GDD_MAKE_DTX_CONTEXT_SWITCH */
  U32 signal = con_handle;
#endif /* GDD_MAKE_DTX_CONTEXT_SWITCH */

  TRACE_FUNCTION("[GDD] gdd_dio_tx_receive_buf()");

#ifdef MEMORY_SUPERVISION  
  vsi_c_ssend(hCommGDD_DIO, signal, (T_VOID_STRUCT*)buf, sizeof(T_VOID_STRUCT*), __FILE__, __LINE__);
#else
  vsi_c_ssend(hCommGDD_DIO, signal, (T_VOID_STRUCT*)buf, sizeof(T_VOID_STRUCT*));
#endif
}
