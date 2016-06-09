/*
+-----------------------------------------------------------------------------
|  File     : gdd_dio_rxf.c
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
|             rx service functions.
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
#include "gdd_dio_rxf.h"


/*==== CONST ================================================================*/

/*==== LOCAL VARS ===========================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*==== PUBLIC FUNCTIONS =====================================================*/


void gdd_dio_rx_sig_send_data(T_GDD_CON_HANDLE con_handle,
                                    T_dio_buffer * buf)
{
  gdd_dio_send_signal_to_dio(get_con_data_from_handle(con_handle), DRV_SIGTYPE_READ);
}

