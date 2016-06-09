/* 
+------------------------------------------------------------------------------
|  File:       gdd_dio_data.h
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
|  Purpose :  Internal data structures for entity gdd_dio (DIO BAT Adapter)
+----------------------------------------------------------------------------- 
*/ 

#ifndef GDD_DIO_DATA_H
#define GDD_DIO_DATA_H

/*==== INCLUDES =============================================================*/

/* DIO stuff */
#include "dio.h"  /* for T_DIO_CAP_PKT */

/* We must define P_DIO_H, in order to avoid inclusion of p_dio.h.
   This is necessary due to the special fact, that "dio.h", which is included
   in the header "dio_io/dio_drv.h", mirrors the header "p_dio.h". */
#define P_DIO_H

#include "gdd.h"

#include "gdd_dio_queue.h"  /* for T_GDD_BUF_QUEUE */


/*==== DEFINITIONS ==========================================================*/

#define GDD_CON_HANDLE_USER_MASK 0xFF000000


/*==== CONSTS ===============================================================*/

/*==== TYPES =================================================================*/

/**
 * States of kernel service
 */
typedef enum 
{
	GDD_DIO_KER_DEAD = 0,
  GDD_DIO_KER_INIT,
  GDD_DIO_KER_READY
} T_GDD_DIO_KER_STATE;


/**
 * Kernel service data
 */
typedef struct
{
  T_GDD_DIO_KER_STATE         state;
} T_GDD_DIO_KER_DATA;


/**
 * States of a single connection
 */
typedef enum
{
  GDD_DIO_CON_DEAD = 1,
  GDD_DIO_CON_CONNECT,
  GDD_DIO_CON_READY,
  GDD_DIO_CON_SENDING,
  GDD_DIO_CON_CLOSE
} T_GDD_DIO_CON_STATE;

/**
 * Connection data
 */
typedef struct 
{
  T_GDD_BUF_QUEUE			   rx_queue;       /* RX buffer queue */
  T_GDD_BUF_QUEUE				 tx_queue;       /* TX buffer queue */
  UBYTE                  con_state;      /* Connection state */
  UBYTE                  wait_send_buf;  /* Flag indicating that we wait for
                                            a send buffer */
  U32                    dio_device;     /* Dio device type */
  T_GDD_RECEIVE_DATA_CB  rcv_cb;         /* Receive callback of the client */
  T_GDD_SIGNAL_CB        sig_cb;         /* Signale callback of the client */
  T_DIO_CAP_PKT          dio_cap;        /* DIO capabilities */
} T_GDD_DIO_CON_DATA;

/**
 * DIO driver number
 */
typedef U32 T_GDD_DIO_DRV_NUM;


/*==== EXPORTS ===============================================================*/

int inst_num_from_dev_id(U32 device);


#endif /* !GDD_DIO_DATA_H */
