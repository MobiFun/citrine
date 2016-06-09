/*
+-----------------------------------------------------------------------------
|  Project :
|  Modul   :
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
|  Purpose :  This module implements the process body interface
|             for the entity Universal Asynchronous Receiver Transmitter(UART)
|
|             Exported functions:
|
|             pei_create    - Create the Protocol Stack Entity
|             pei_init      - Initialize Protocol Stack Entity
|             pei_primitive - Process Primitive
|             pei_timeout   - Process Timeout
|             pei_exit      - Close resources and terminate
|             pei_run       - Process Primitive
|             pei_config    - Dynamic Configuration
|             pei_monitor   - Monitoring of physical Parameters
+-----------------------------------------------------------------------------
*/

#define UART_PEI_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_UART

/*==== INCLUDES =============================================================*/

#ifdef _SIMULATION_
#include <stdio.h>
#endif /* _SIMULATION_ */
#ifdef WIN32
#include "nucleus.h"
#endif /* WIN32 */
#include "typedefs.h"   /* to get Condat data types */
#include "vsi.h"        /* to get a lot of macros */
#include "macdef.h"     /* to get a lot of macros */
#include "custom.h"
#include "gsm.h"        /* to get a lot of macros */
#include "cnf_uart.h"   /* to get cnf-definitions */
#include "mon_uart.h"   /* to get mon-definitions */
#include "prim.h"       /* to get the definitions of used SAP and directions */
#ifdef DTILIB
#ifdef _SIMULATION_
/* #define UART_DTI_CONFIG  -- presently not needed -- */
#include "tok.h"        /* string management for pei_config() */
#endif /* _SIMULATION_ */
#include "dti.h"        /* to get dti lib */
#endif /* DTILIB */
#include "pei.h"        /* to get PEI interface */
#ifdef FF_MULTI_PORT
#include "gsi.h"        /* to get definitions of serial driver */
#else /* FF_MULTI_PORT */
#ifdef _TARGET_
#include "../../serial/serialswitch.h"
#include "../../serial/traceswitch.h"
#else /* _TARGET_ */
#include "serial_dat.h" /* to get definitions of serial driver */
#endif /* _TARGET_ */
#endif /* FF_MULTI_PORT */
#include "uart.h"       /* to get the global entity definitions */

#include "uart_kerf.h"  /* to get ker functions */
#include "uart_kerp.h"  /* to get ker primitives */
#include "uart_kers.h"  /* to get ker signals */

#ifndef FF_MULTI_PORT
#include "uart_rxf.h"   /* to get rx functions */
#include "uart_rxp.h"   /* to get rx primitives */
#include "uart_rxs.h"   /* to get rx signals */

#include "uart_txf.h"   /* to get tx functions */
#include "uart_txp.h"   /* to get tx primitives */
#include "uart_txs.h"   /* to get tx signals */
#else
#include "uart_prxf.h"  /* to get rx functions */
#include "uart_prxp.h"  /* to get rx primitives */
#include "uart_prxs.h"  /* to get rx signals */

#include "uart_ptxf.h"  /* to get tx functions */
#include "uart_ptxp.h"  /* to get tx primitives */
#include "uart_ptxs.h"  /* to get tx signals */
#endif /* FF_MULTI_PORT */

#include "uart_drxf.h"  /* to get drx functions */
#include "uart_drxp.h"  /* to get drx primitives */
#include "uart_drxs.h"  /* to get drx signals */

#include "uart_dtxf.h"  /* to get dtx functions */
#include "uart_dtxp.h"  /* to get dtx primitives */
#include "uart_dtxs.h"  /* to get dtx signals */

#include "uart_rtf.h"   /* to get dtx functions */
#include "uart_rtp.h"   /* to get dtx primitives */
#include "uart_rts.h"   /* to get dtx signals */


/*==== DEFINITIONS ==========================================================*/

/*
 * Prototypes for DTILIB wrap functions
 */
LOCAL void pei_dti_dti_connect_req (T_DTI2_CONNECT_REQ* dti_connect_req);
LOCAL void pei_dti_dti_connect_res (T_DTI2_CONNECT_RES* dti_connect_res);
LOCAL void pei_dti_dti_disconnect_req (T_DTI2_DISCONNECT_REQ*
                                          dti_disconnect_req);
LOCAL void pei_dti_dti_getdata_req (T_DTI2_GETDATA_REQ* dti_getdata_req);
LOCAL void pei_dti_dti_data_req (T_DTI2_DATA_REQ* dti_data_req);
LOCAL void pei_dti_dti_connect_ind (T_DTI2_CONNECT_IND* dti_connect_ind);
LOCAL void pei_dti_dti_connect_cnf (T_DTI2_CONNECT_CNF* dti_connect_cnf);
LOCAL void pei_dti_dti_disconnect_ind (T_DTI2_DISCONNECT_IND*
                                          dti_disconnect_ind);
LOCAL void pei_sig_callback(U8 instance,
                            U8 interfac,
                            U8 channel,
                            U8 reason,
                            T_DTI2_DATA_IND *dti_data2_ind);
#ifdef _SIMULATION_
LOCAL void pei_dti_dti_data_test_req (T_DTI2_DATA_TEST_REQ* dti_data_test_req);
LOCAL void pei_dti_dti_ready_ind (T_DTI2_READY_IND* dti_ready_ind);
LOCAL void pei_dti_dti_data_ind (T_DTI2_DATA_IND* dti_data_ind);
#endif /* _SIMULATION_ */

/*==== TYPES ================================================================*/

/*
 * global table which maps a timer index to an UART instancen and timer number
 */
typedef struct /* T_UART_TIMER_TABLE */
{
  T_UART_DATA*  uart_data;      /* pointer to UART data of the instance */
  UBYTE         timer_number;   /* number of timer within the instance */
} T_UART_TIMER_TABLE;

/*==== GLOBAL VARS ==========================================================*/

/*
 * global table which maps a timer index to an UART instancen and timer number
 */
T_UART_TIMER_TABLE        uart_timer_table[ UART_TIMER_MAX ];

#ifdef DTILIB
  DTI_HANDLE uart_hDTI = D_NO_DATA_BASE;
#endif /* DTILIB */
#ifdef UART_DTI_CONFIG
  ULONG      drv_link_id = -1;
#endif /* UART_DTI_CONFIG */
/*==== LOCAL VARS ===========================================================*/

static  BOOL          first_access  = TRUE;
static  T_MONITOR     uart_mon;

/*
 * Jumptables to primitive handler functions. One table per SAP.
 *
 * Use MAK_FUNC_0 for primitives which contains no SDU.
 * Use MAK_FUNC_S for primitives which contains a SDU.
 */

/*
 * Function is needed for developing. This declaration can be removed
 * as soon as this function is no more called (i.e. all primitives are
 * handled).
 */
LOCAL void primitive_not_supported (void *data);

/*qqq hier reinkommende, mit SAP vergleichen, S: mit sdu*/
static const T_FUNC uart_table[] =
{
  MAK_FUNC_0(ker_uart_parameters_req,     UART_PARAMETERS_REQ),     /* 0x7400 */
  MAK_FUNC_0(ker_uart_dti_req,            UART_DTI_REQ),            /* 0x7401 */
  MAK_FUNC_0(ker_uart_disable_req,        UART_DISABLE_REQ),        /* 0x7402 */
  MAK_FUNC_0(ker_uart_ring_req,           UART_RING_REQ),           /* 0x7403 */
  MAK_FUNC_0(ker_uart_dcd_req,            UART_DCD_REQ),            /* 0x7404 */
  MAK_FUNC_0(ker_uart_escape_req,         UART_ESCAPE_REQ),         /* 0x7405 */
  MAK_FUNC_0(ker_uart_mux_start_req,      UART_MUX_START_REQ),      /* 0x7406 */
  MAK_FUNC_0(ker_uart_mux_dlc_establish_res,
                                       UART_MUX_DLC_ESTABLISH_RES), /* 0x7408 */
  MAK_FUNC_0(ker_uart_mux_dlc_release_req,UART_MUX_DLC_RELEASE_REQ),/* 0x7409 */
  MAK_FUNC_0(ker_uart_mux_sleep_req,      UART_MUX_SLEEP_REQ),      /* 0x740a */
  MAK_FUNC_0(ker_uart_mux_wakeup_req,     UART_MUX_WAKEUP_REQ),     /* 0x740b */
  MAK_FUNC_0(ker_uart_mux_close_req,      UART_MUX_CLOSE_REQ),      /* 0x740c */
/*
  UART_DRIVER_SENT_IND           0x740d
  UART_DRIVER_RECEIVED_IND       0x740e
  UART_DRIVER_FLUSHED_IND        0x740f
*/
};

/*
 * Jumptable for the DTI service access point for uplink data transmission.
 * Contains the processing-function addresses and opcodes of
 * request and response primitives. Use of DTILIB can be selected.
 *
 */
static const T_FUNC dti_dti_ul_table[] = {
  MAK_FUNC_0( pei_dti_dti_connect_req       ,   DTI2_CONNECT_REQ   ),
  MAK_FUNC_0( pei_dti_dti_connect_res       ,   DTI2_CONNECT_RES   ),
  MAK_FUNC_0( pei_dti_dti_disconnect_req,       DTI2_DISCONNECT_REQ),
  MAK_FUNC_0( pei_dti_dti_getdata_req       ,   DTI2_GETDATA_REQ   ),
  MAK_FUNC_0( pei_dti_dti_data_req          ,   DTI2_DATA_REQ      ),
#if defined (_SIMULATION_)
  MAK_FUNC_S( pei_dti_dti_data_test_req     ,   DTI2_DATA_TEST_REQ )
#else /* _SIMULATION_ */
  MAK_FUNC_S( primitive_not_supported       ,   DTI2_DATA_TEST_REQ )
#endif  /* _SIMULATION_ */
};
#if defined (DTILIB) && defined (_SIMULATION_)
static const T_FUNC dti_dti_dl_table[] = {
  MAK_FUNC_0( pei_dti_dti_connect_ind    ,   DTI2_CONNECT_IND   ),
  MAK_FUNC_0( pei_dti_dti_connect_cnf    ,   DTI2_CONNECT_CNF   ),
  MAK_FUNC_0( pei_dti_dti_disconnect_ind ,   DTI2_DISCONNECT_IND),
  MAK_FUNC_0( tx_dti_ready_ind           ,   DTI2_READY_IND     ),
  MAK_FUNC_0( primitive_not_supported    ,   DTI2_DATA_IND      ),
  MAK_FUNC_S( rx_dti_data_test_ind       ,   DTI2_DATA_TEST_IND ),
  MAK_FUNC_0( pei_dti_dti_ready_ind      ,   DTI2_READY_IND     ),
  MAK_FUNC_0( pei_dti_dti_data_ind       ,   DTI2_DATA_IND      ),
  MAK_FUNC_S( primitive_not_supported    ,   DTI2_DATA_TEST_IND )
};
#endif /*defined (DTILIB) && defined (_SIMULATION_)*/

/*==== DIAGNOSTICS ==========================================================*/
#ifdef _DEBUG
#endif /* _DEBUG */

/*==== END DIAGNOSTICS ======================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*
+------------------------------------------------------------------------------
| Function    : primitive_not_supported
+------------------------------------------------------------------------------
| Description :  This function handles unsupported primitives.
|
| Parameters  : -
|
| Return      : -
|
+------------------------------------------------------------------------------
*/
LOCAL void primitive_not_supported (void *data)
{
  TRACE_FUNCTION ("primitive_not_supported");

  PFREE (data);
}


/*==== PUBLIC FUNCTIONS =====================================================*/

/* qqq hier malen
+------------------------------------------------------------------------------
|       Function        : pei_primitive
+------------------------------------------------------------------------------
|       Description     : This function is called by the frame when a primitive
|                         is received and needs to be processed.
|
|                            |        |
|                           ACI      DTI                 UPLINK
|                            |        |
|                   +--------v--------v--------+
|                   |                          |
|                   |           UART           |
|                   |                          |
|                   +--------------------------+
|
|
|       Parameters      :       prim      - Pointer to the received primitive
|
|       Return          :       PEI_OK    - function succeeded
|                               PEI_ERROR - function failed
|
+------------------------------------------------------------------------------
*/

/*qqq hier kucken*/
LOCAL SHORT pei_primitive (void * primptr)
{
  TRACE_FUNCTION ("pei_primitive");

  if (primptr NEQ NULL)
  {
    T_PRIM* prim = (T_PRIM*)primptr;
    USHORT  opc  = (USHORT)prim->custom.opc;
    USHORT  n;
    const T_FUNC    *table;

    /*
     * This must be called for Partition Pool supervision. Will be replaced
     * by another macro some time.
     */
    VSI_PPM_REC (&prim->custom, __FILE__, __LINE__);
    PTRACE_IN (opc);

    switch (opc & OPC_MASK)
    {
      case UART_DL:
        table = uart_table;
        n = TAB_SIZE (uart_table);
        break;
      case DTI2_UL:
#ifdef DTILIB
        table = dti_dti_ul_table;
        n = TAB_SIZE (dti_dti_ul_table);
#ifdef DTI2
        /*
         * to be able to distinguish DTI1/DTI2 opcodes,
         * the ones for DTI2 start at 0x50
         */
        opc -= 0x50;
#endif /* DTI2*/
#else  /* DTILIB */
        table = dti_ul_table;
        n = TAB_SIZE (dti_ul_table);
#endif /* DTILIB */
        break;
#ifdef _SIMULATION_
      case DTI2_DL:
#ifdef DTILIB
        table = dti_dti_dl_table;
        n = TAB_SIZE (dti_dti_dl_table);
#ifdef DTI2
        /*
         * to be able to distinguish DTI1/DTI2 opcodes,
         * the ones for DTI2 start at 0x50
         */
        opc -= 0x50;
#endif /* DTI2*/
#else  /* DTILIB */
        table = dti_dl_table;
        n = TAB_SIZE (dti_dl_table);
#endif /* DTILIB */
        break;
#endif /* _SIMULATION_ */
      default:
        table = NULL;
        n = 0;
        break;
    }

    if (table != NULL)
    {
      if ((opc & PRM_MASK) < n)
      {
        table += opc & PRM_MASK;
#ifdef PALLOC_TRANSITION
        P_SDU(prim) = table->soff ? (T_sdu*) (((char*)&prim->data)
                                                     + table->soff) : 0;
#ifndef NO_COPY_ROUTING
        P_LEN(prim) = table->size + sizeof (T_PRIM_HEADER);
#endif /* NO_COPY_ROUTING */
#endif /* PALLOC_TRANSITION */
        JUMP (table->func) (P2D(prim));
      }
      else
      {
        primitive_not_supported (P2D(prim));
      }
      return PEI_OK;
    }

    /*
     * primitive is not a GSM primitive - forward it to the environment
     */
    if (opc & SYS_MASK)
    {
      if(vsi_c_primitive (VSI_CALLER prim) NEQ VSI_OK)
      {
        TRACE_ERROR_P1("VSI entity: Can't forward a primitive, uart_pei.c(%d)",
                                                                     __LINE__);
      }
    }
    else
    {
      PFREE (P2D(prim));
      return PEI_ERROR;
    }
  }
  return PEI_OK;
}



#ifdef FF_MULTI_PORT
/*
+------------------------------------------------------------------------------
| Function    : pei_uart_driver_signal
+------------------------------------------------------------------------------
| Description : The function pei_uart_driver_signal() is the callback function
|               of the UART driver to indicate events of the driver. This
|               function is called in interrupt context. It converts the given
|               opcode to a signal.
|
| Parameters  : usartNo - affected UART port
|               opcode  - opcode of the event
|
+------------------------------------------------------------------------------
*/
GLOBAL void pei_uart_driver_signal (T_DRV_SIGNAL *SigPtr)
{
  T_UART_DATA*  uart_device;

  TRACE_FUNCTION( "pei_uart_driver_signal" );

  /*
   * set UART instance
   */
  uart_device = &uart_data_base[SigPtr->DrvHandle];
  /*
   * activate signal
   */
  switch (SigPtr->SignalType)
  {
    case DRV_SIGTYPE_READ:
      PSIGNAL(hCommUART, UART_DRIVER_RECEIVED_IND, uart_device);
      break;

    case DRV_SIGTYPE_WRITE:
      PSIGNAL(hCommUART, UART_DRIVER_SENT_IND, uart_device);
      break;

    case DRV_SIGTYPE_FLUSH:
      PSIGNAL(hCommUART, UART_DRIVER_FLUSHED_IND, uart_device);
      break;

    default:
      TRACE_ERROR( "pei_uart_driver_signal: unknown signal" );
      break;
  }
} /* pei_uart_driver_signal() */
#endif /* FF_MULTI_PORT */



/*
+------------------------------------------------------------------------------
|       Function        : pei_init
+------------------------------------------------------------------------------
|       Description     : This function is called by the frame. It is used to
|                         initialise the entitiy.
|
|       Parameters      : handle    - task handle
|
|       Return          : PEI_OK    - entity initialised
|                         PEI_ERROR - entity not (yet) initialised
|
+------------------------------------------------------------------------------
*/

/*qqq hier Kommunikationskanaele eintragen*/
LOCAL SHORT pei_init (T_HANDLE handle)
{
  USHORT  i;
  UBYTE   instance;

  /*
   * Initialize task handle
   */
   UART_handle = handle;

   TRACE_FUNCTION ("pei_init");
  /*
   * Open communication channel
   */
  if (hCommMMI < VSI_OK)
  {
    if ((hCommMMI = vsi_c_open (VSI_CALLER ACI_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

  if (hCommUART < VSI_OK)
  {
    if ((hCommUART = vsi_c_open (VSI_CALLER UART_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

#ifdef DTILIB
  /*
   * initialize dtilib for this entity
   */
  uart_hDTI = dti_init (UART_INSTANCES * UART_MAX_NUMBER_OF_CHANNELS,
                        handle,
                        DTI_DEFAULT_OPTIONS,
                        pei_sig_callback);
  if(!uart_hDTI)
    return PEI_ERROR;

#else  /* DTILIB */
  /*
   * Initialize table with channel ids
   */
  for( i = 0; i < (UART_INSTANCES * UART_MAX_NUMBER_OF_CHANNELS); i++)
  {
    uart_cid_table[i].c_id      = 0;
    uart_cid_table[i].uart_data = NULL;
    uart_cid_table[i].dtx       = NULL;
    uart_cid_table[i].drx       = NULL;
  }
#endif /* DTILIB */

  /*
   * send parameters primitive
   */
  {
    PALLOC (uart_parameters_ind, UART_PARAMETERS_IND);
    uart_parameters_ind->uart_instances = UART_INSTANCES;
    PSEND (hCommMMI, uart_parameters_ind);
  }

  /*
   * Initialize timer table
   */
  for( i = 0; i < UART_TIMER_MAX; i++)
  {
    uart_timer_table[i].uart_data    = &(uart_data_base
                                         [(UBYTE)(i /
                                           UART_TIMER_PER_INSTANCE)]);
    uart_timer_table[i].timer_number = (UBYTE)(i % UART_TIMER_PER_INSTANCE);
  }

  for( instance = 0; instance < UART_INSTANCES; instance++ )
  {
    /*
     * Initialize global pointer uart_data. This is required to access all
     * entity data.
     */
    uart_data = &(uart_data_base[instance]);

    /*
     * Initialize DLC table
     */
    for( i = 0; i <= UART_MAX_NUMBER_OF_CHANNELS; i++)
    {
      uart_data->dlc_table[i].dlci              = UART_DLCI_INVALID;
      uart_data->dlc_table[i].priority          = 0;
#ifdef DTILIB
      uart_data->dlc_table[i].dti_state = DTI_CLOSED;
#else  /* DTILIB */
      uart_data->dlc_table[i].hCommUPLINK       = VSI_ERROR;
#endif /* DTILIB */
      uart_data->dlc_table[i].connection_state  = UART_CONNECTION_DEAD;
      if(i EQ UART_CONTROL_INSTANCE)
      {
        uart_data->dlc_table[i].drx = &uart_data->drx_base[0];
        uart_data->dlc_table[i].dtx = &uart_data->dtx_base[0];
      }
      else
      {
        uart_data->dlc_table[i].drx = &uart_data->drx_base[i];
        uart_data->dlc_table[i].dtx = &uart_data->dtx_base[i];
      }

      uart_data->dlc_table[i].transmit_data = NULL;
      uart_data->dlc_table[i].transmit_pos  = 0;
      uart_data->dlc_table[i].p_counter     = 0;

      uart_data->dlc_table[i].receive_data    = NULL;
      uart_data->dlc_table[i].receive_pos     = 0;
      uart_data->dlc_table[i].receive_size    = 0;
      uart_data->dlc_table[i].receive_process = UART_RX_PROCESS_STOP;

      uart_data->dlc_table[i].last_command    = NULL;
      uart_data->dlc_table[i].next_command    = NULL;
      uart_data->dlc_table[i].retransmissions = 0;
      uart_data->dlc_table[i].lines           = 0;
      uart_data->dlc_table[i].service         = 0;
      uart_data->dlc_table[i].codec           = 0;
      uart_data->dlc_table[i].received_prim   = 0;
      uart_data->dlc_table[i].flushed         = TRUE;
    }

    /*
     * Initialize DLCI instance table
     */
    for( i = 0; i < 64; i++ )
    {
      uart_data->dlc_instance[i] = UART_EMPTY_INSTANCE;
    }

    /*
     * timer values for this instance
     */
    uart_data->timer_t1_index = (USHORT)(instance * UART_TIMER_PER_INSTANCE)
                                + UART_RT_INDEX_T1;
    uart_data->timer_t2_index = (USHORT)(instance * UART_TIMER_PER_INSTANCE)
                                + UART_RT_INDEX_T2;
    uart_data->timer_t3_index = (USHORT)(instance * UART_TIMER_PER_INSTANCE)
                                + UART_RT_INDEX_T3;
    uart_data->timer_tesd_index = (USHORT)(instance * UART_TIMER_PER_INSTANCE)
                                + UART_RT_INDEX_TESD;
    /*
     * set device for this instance
     */
    uart_data->device   = instance;
    uart_data->fcstab   = uart_fcstable_base;

    /*
     * set device specific values
     */
    uart_data->xon  = UART_IO_XON_DEFAULT;
    uart_data->xoff = UART_IO_XOFF_DEFAULT;
    uart_data->n1   = UART_N1_READY_MODE;
    /*
     * set initial DTX/DRX services
     */
    uart_data->dtx = &(uart_data->dtx_base[0]);
    uart_data->drx = &(uart_data->drx_base[0]);
    /*
     * escape sequence detection
     */
    uart_data->act_ec    = 0x2b;         /* Escape Character '+' */
    uart_data->act_gp    = 1000;         /* Guard Period */
  }

  /*
   * qqq Initialize entity data (call init function of every service)
   * the order of function calls is important
   */
  for( i = 0; i < UART_INSTANCES; i++ )
  {
    uart_data = &(uart_data_base[i]);
    rt_init();
    ker_init();
    rx_init();
    tx_init();
    drx_init();
    dtx_init();
  }
  uart_data = &(uart_data_base[0]);

  return (PEI_OK);
}



/*
+------------------------------------------------------------------------------
|       Function        : pei_timeout
+------------------------------------------------------------------------------
|       Description     : This function is called by the frame when a timer
|                         has expired.
|
|       Parameters      : index             - timer index
|
|       Return          : PEI_OK            - timeout processed
|                         PEI_ERROR         - timeout not processed
|
+------------------------------------------------------------------------------
*/
LOCAL SHORT pei_timeout (USHORT index)
{
  TRACE_FUNCTION ("pei_timeout");

  /*
   * set UART instance
   */
  uart_data = uart_timer_table[index].uart_data;

  /*
   * Process timeout
   */
  switch (uart_timer_table[index].timer_number)
  {
    case UART_RT_INDEX_T1:
      /*
       * timer T1 expired.
       */
      rt_t1_expired();
      break;
    case UART_RT_INDEX_T2:
      /*
       * timer T2 expired.
       */
      rt_t2_expired();
      break;
    case UART_RT_INDEX_T3:
      /*
       * timer T3 expired.
       */
      rt_t3_expired();
      break;
    case UART_RT_INDEX_TESD:
      /*
       * Escape Sequence Detection timer expired.
       */
      rt_tesd_expired();
      break;
    default:
      TRACE_ERROR("pei_timeout: Unknown Timeout");
      break;
  }

  return PEI_OK;
}



/*
+------------------------------------------------------------------------------
|       Function        : pei_signal
+------------------------------------------------------------------------------
|       Description     : This function is called by the frame when a signal
|                         has been received.
|
|       Parameters      : opc               - signal operation code
|                         *data             - pointer to primitive
|
|       Return          : PEI_OK            - signal processed
|                         PEI_ERROR         - signal not processed
|
+------------------------------------------------------------------------------
*/
LOCAL SHORT pei_signal (ULONG opc, void *data)
{
  TRACE_FUNCTION ("pei_signal");

#ifdef FF_MULTI_PORT
  /*
   * Process signal
   */
  switch (opc)
  {
    case UART_DRIVER_SENT_IND:
      tx_uart_driver_sent_ind(data);
      break;

    case UART_DRIVER_RECEIVED_IND:
      rx_uart_driver_received_ind(data);
      break;

    case UART_DRIVER_FLUSHED_IND:
      tx_uart_driver_flushed_ind(data);
      break;

    default:
      TRACE_ERROR("Unknown Signal OPC");
      return PEI_ERROR;
  }
#else /* FF_MULTI_PORT */
  /*
   * Process signal
   */
  switch (opc)
  {
    case UART_DRIVER_SENT_IND:
      tx_uart_driver_sent_ind(data);
      break;

    case UART_DRIVER_RECEIVED_IND:
      rx_uart_driver_received_ind(data);
      break;

    default:
      TRACE_ERROR("Unknown Signal OPC");
      break;
  }
#endif /* FF_MULTI_PORT */

  return PEI_OK;
}



/*
+------------------------------------------------------------------------------
|       Function        : pei_exit
+------------------------------------------------------------------------------
|       Description     : This function is called by the frame when the entity
|                         is terminated. All open resources are freed.
|
|       Parameters      : -
|
|       Return          : PEI_OK            - exit sucessful
|                         PEI_ERROR         - exit not sueccessful
|
+------------------------------------------------------------------------------
*/
LOCAL SHORT pei_exit (void)
{
  TRACE_FUNCTION ("pei_exit");

#ifdef DTILIB
#ifdef _SIMULATION_

  /*
   * close test channel
   */
#endif /* _SIMULATION_ */

  /*
   * Shut down dtilib communication
   */
  dti_deinit(uart_hDTI);
#endif /* DTILIB */
  /*
   * Close communication channel to MMI
   */
  if(vsi_c_close (VSI_CALLER hCommMMI) NEQ VSI_OK)
  {
    TRACE_ERROR_P1("VSI entity: CloseComChannel to MMI failed, uart_pei.c(%d)",
                                                                     __LINE__);
    return PEI_ERROR;
  }
  hCommMMI = VSI_ERROR;


  /*
   * Free all resources
   */
  drx_free_resources();
  dtx_exit();

  return PEI_OK;
}



/*
+------------------------------------------------------------------------------
|       Function        : pei_run
+------------------------------------------------------------------------------
|       Description     : This function is called by the frame when entering
|                         the main loop. This function is only required in the
|                         active variant.
|
|               This function is not used.
|
|       Parameters      : handle            - Communication handle
|
|       Return          : PEI_OK            - sucessful
|                         PEI_ERROR         - not successful
|
+------------------------------------------------------------------------------
*/
LOCAL SHORT pei_run (T_HANDLE TaskHandle, T_HANDLE ComHandle )
{
  return PEI_OK;
} /* pei_run() */



/*
+------------------------------------------------------------------------------
|       Function        : pei_monitor
+------------------------------------------------------------------------------
|       Description     : This function is called by the frame in case sudden
|                         entity specific data is requested
|                         (e.g. entity Version).
|
|       Parameters      : out_monitor - return the address of the data to be
|                                       monitoredCommunication handle
|
|       Return          : PEI_OK      - sucessful
|                                       (address in out_monitor is valid)
|                         PEI_ERROR   - not successful
|
+------------------------------------------------------------------------------
*/
LOCAL SHORT pei_monitor (void ** out_monitor)
{
  TRACE_FUNCTION ("pei_monitor");

  /*
   * Version = "0.S" (S = Step).
   */
  uart_mon.version = VERSION_UART;
  *out_monitor = &uart_mon;

  return PEI_OK;
}


/*
+------------------------------------------------------------------------------
|       Function        : pei_config
+------------------------------------------------------------------------------
|       Description     : This function is called by the frame when a primitive
|                         is received indicating dynamic configuration.
|
|               This function is not used in this entity.
|
|       Parameters      : handle            - Communication handle
|
|       Return          : PEI_OK            - sucessful
|                         PEI_ERROR         - not successful
|
+------------------------------------------------------------------------------
*/
LOCAL SHORT pei_config (T_PEI_CONFIG inString)
{
  TRACE_FUNCTION ("pei_config");
  TRACE_FUNCTION (inString);

  return PEI_OK;
}

/*
+------------------------------------------------------------------------------
|       Function        : pei_create
+------------------------------------------------------------------------------
|       Description     : This function is called by the frame when the process
|                         is created.
|
|       Parameters      : out_name          - Pointer to the buffer in which to
|                                             locate the name of this entity
|
|       Return          : PEI_OK            - entity created successfuly
|                         PEI_ERROR         - entity could not be created
|
+------------------------------------------------------------------------------
*/
GLOBAL SHORT pei_create (T_PEI_INFO **info)
{
static T_PEI_INFO pei_info =
              {
               "UART",         /* name */
               {              /* pei-table */
                 pei_init,
#ifdef _SIMULATION_
                 pei_exit,
#else
                 NULL,
#endif
                 pei_primitive,
                 pei_timeout,
                 pei_signal,
                 pei_run,
                 pei_config,
                 pei_monitor
               },
               1024,            /* stack size */
               10,              /* queue entries */
               200,             /* priority (1->low, 255->high) */
               UART_TIMER_MAX,  /* number of timers */
               0x03|PRIM_NO_SUSPEND /* flags: bit 0   active(0) body/passive(1) */
              };                /*        bit 1   com by copy(0)/reference(1) */


  TRACE_FUNCTION ("pei_create");

  /*
   * Close Resources if open
   */
#ifdef _SIMULATION_
  if (first_access)
  {
    first_access = FALSE;
  }
  else
  {
    if(pei_exit() NEQ PEI_OK)
    {
      TRACE_ERROR_P1("UART PEI: Can't free open recources, uart_pei.c(%d)",
                                                                 __LINE__);
    }
  }
#endif

  /*
   * Export startup configuration data
   */
  *info = &pei_info;

  return PEI_OK;
}


#ifndef DTILIB
/*
+------------------------------------------------------------------------------
|    Function    : pei_select_instances
+------------------------------------------------------------------------------
|    Description : This function selects the correct UART and DTX/DRX services
|                 for an incoming DTI interface primitive. This is done by a
|                 lookup in the UART channel id table in order to determine
|                 the correct UART, DTX and DRX data structures.
|
|    Parameters  :   c_id  - channel id of incoming primitive
|
|    Return      :   TRUE  - successful
|                   FALSE - not successful
|
+------------------------------------------------------------------------------
*/
GLOBAL UBYTE pei_select_instances( UBYTE incoming_c_id )
{
  USHORT i;
  UBYTE found = FALSE;

  TRACE_FUNCTION ("pei_select_instances");

  /*
   * locate the channel id in the global cid table
   */
  for( i = 0; i < UART_INSTANCES * UART_MAX_NUMBER_OF_CHANNELS; i++ )
    if( uart_cid_table[i].c_id EQ incoming_c_id )
      if( ( uart_cid_table[i].dtx NEQ NULL ) &&
          ( uart_cid_table[i].drx NEQ NULL ) &&
          ( uart_cid_table[i].uart_data NEQ NULL ) )
      {
        /*
         *  channel id found. select service structures
         */
        uart_data       = uart_cid_table[i].uart_data;
        uart_data->dtx  = uart_cid_table[i].dtx;
        uart_data->drx  = uart_cid_table[i].drx;

        found = TRUE;
        break;
      }

  if( found EQ FALSE )
  {
    TRACE_EVENT(" pei_select_instances: unknown c_id ");
  }
#ifdef _SIMULATION_
  {
    char buf[64];
    sprintf(buf,"found incoming_c_id: 0x%02X in uart_cid_table[%d]",incoming_c_id, i);
    TRACE_EVENT(buf);
  }
#endif
  return( found );
} /* pei_select_instances */
#endif /* DTILIB */


/*
 * maps for DTILIB functions
 */

#ifdef DTILIB
/*
+------------------------------------------------------------------------------
|    Function    : pei_dti_dti_connect_req
+------------------------------------------------------------------------------
|    PURPOSE : Call the process function dti_dti_connect_req
+------------------------------------------------------------------------------
*/
LOCAL void pei_dti_dti_connect_req (T_DTI2_CONNECT_REQ* dti_connect_req)
{
  dti_dti_connect_req (uart_hDTI, dti_connect_req);
} /* pei_dti_dti_connect_req() */



/*
+------------------------------------------------------------------------------
|    Function    : pei_dti_dti_connect_res
+------------------------------------------------------------------------------
|    PURPOSE : Call the process function dti_dti_connect_res
+------------------------------------------------------------------------------
*/
LOCAL void pei_dti_dti_connect_res (T_DTI2_CONNECT_RES* dti_connect_res)
{
  dti_dti_connect_res(uart_hDTI, dti_connect_res);
} /* pei_dti_dti_connect_res() */



/*
+------------------------------------------------------------------------------
|    Function    : pei_dti_dti_disconnect_req
+------------------------------------------------------------------------------
|    PURPOSE : Call the process function dti_dti_disconnect_req
+------------------------------------------------------------------------------
*/
LOCAL void pei_dti_dti_disconnect_req (T_DTI2_DISCONNECT_REQ*
                                          dti_disconnect_req)
{
  dti_dti_disconnect_req (uart_hDTI, dti_disconnect_req);
} /* pei_dti_dti_disconnect_req() */


#ifdef _SIMULATION_
/*
+------------------------------------------------------------------------------
|    Function    : pei_dti_dti_connect_ind
+------------------------------------------------------------------------------
|    PURPOSE : Call the process function dti_dti_connect_ind
+------------------------------------------------------------------------------
*/
LOCAL void pei_dti_dti_connect_ind (T_DTI2_CONNECT_IND* dti_connect_ind)
{
  dti_dti_connect_ind (uart_hDTI, dti_connect_ind);
} /* pei_dti_dti_connect_ind() */



/*
+------------------------------------------------------------------------------
|    Function    : pei_dti_dti_connect_cnf
+------------------------------------------------------------------------------
|    PURPOSE : Call the process function dti_dti_connect_cnf
+------------------------------------------------------------------------------
*/
LOCAL void pei_dti_dti_connect_cnf (T_DTI2_CONNECT_CNF* dti_connect_cnf)
{
  dti_dti_connect_cnf(uart_hDTI, dti_connect_cnf);
} /* pei_dti_dti_connect_cnf() */



/*
+------------------------------------------------------------------------------
|    Function    : pei_dti_dti_disconnect_ind
+------------------------------------------------------------------------------
|    PURPOSE : Call the process function dti_dti_disconnect_ind
+------------------------------------------------------------------------------
*/
LOCAL void pei_dti_dti_disconnect_ind (T_DTI2_DISCONNECT_IND*
                                          dti_disconnect_ind)
{
  dti_dti_disconnect_ind (uart_hDTI, dti_disconnect_ind);
} /* pei_dti_dti_disconnect_ind() */

#endif /*_SIMULATION_*/

/*
+------------------------------------------------------------------------------
|    Function    : pei_dti_dti_data_req
+------------------------------------------------------------------------------
|    PURPOSE : Call the process function dti_dti_data_req
+------------------------------------------------------------------------------
*/
LOCAL void pei_dti_dti_data_req (T_DTI2_DATA_REQ* dti_data_req)
{
  dti_dti_data_req (uart_hDTI, dti_data_req);
} /* pei_dti_dti_data_req() */



/*
+------------------------------------------------------------------------------
|    Function    : pei_dti_dti_getdata_req
+------------------------------------------------------------------------------
|    PURPOSE : Call the process function dti_dti_getdata_req
+------------------------------------------------------------------------------
*/
LOCAL void pei_dti_dti_getdata_req (T_DTI2_GETDATA_REQ* dti_getdata_req)
{
  dti_dti_getdata_req (uart_hDTI, dti_getdata_req);
} /* pei_dti_dti_getdata_req() */


/*==== Start functions only use with Windows ===============================*/

#ifdef _SIMULATION_

/*
+------------------------------------------------------------------------------
|    Function    : pei_dti_dti_data_test_req
+------------------------------------------------------------------------------
|    PURPOSE : Call the process function dti_dti_data_test_req
+------------------------------------------------------------------------------
*/
LOCAL void pei_dti_dti_data_test_req (T_DTI2_DATA_TEST_REQ* dti_data_test_req)
{
  dti_dti_data_test_req (uart_hDTI, dti_data_test_req);
} /* pei_dti_dti_data_test_req() */

/*==== End functions only used with testing from WIN32 =====================*/



/*
+------------------------------------------------------------------------------
|    Function    : pei_dti_dti_ready_ind
+------------------------------------------------------------------------------
|    PURPOSE : Call the process function dti_dti_ready_ind
+------------------------------------------------------------------------------
*/
LOCAL void pei_dti_dti_ready_ind (T_DTI2_READY_IND* dti_ready_ind)
{
  dti_dti_ready_ind (uart_hDTI, dti_ready_ind);
} /* pei_dti_dti_ready_ind() */



/*
+------------------------------------------------------------------------------
|    Function    : pei_dti_dti_data_ind
+------------------------------------------------------------------------------
|    PURPOSE : Call the process function dti_dti_data_ind
+------------------------------------------------------------------------------
*/
LOCAL void pei_dti_dti_data_ind (T_DTI2_DATA_IND* dti_data_ind)
{
  dti_dti_data_ind (uart_hDTI, dti_data_ind);
} /* pei_dti_dti_data_ind() */
#endif /* _SIMULATION_ */

/*
+------------------------------------------------------------------------------
|    Function    : pei_sig_callback
+------------------------------------------------------------------------------
|    PURPOSE : Callback function for DTILIB
+------------------------------------------------------------------------------
*/

LOCAL void pei_sig_callback(U8 instance,
                            U8 interfac,
                            U8 channel,
                            U8 reason,
                            T_DTI2_DATA_IND *dti_data2_ind)
{
  T_DLC *dlc;

  TRACE_FUNCTION("pei_sig_callback");

  if(interfac NEQ UART_DTI_UP_INTERFACE)
  {
    TRACE_ERROR("[PEI_SIG_CALLBACK] interface not valid!");
    return; /* error, not found */
  }

  uart_data = &uart_data_base[instance];

  if (uart_hDTI NEQ D_NO_DATA_BASE)
  {
    switch (reason)
    {
      case DTI_REASON_CONNECTION_OPENED:
        dlc = &uart_data->dlc_table[channel]; /* channel is dlci */
        uart_data->drx   = dlc->drx;
        uart_data->dtx   = dlc->dtx;

        sig_dti_ker_connection_opened_ind(channel);
        break;

      case DTI_REASON_CONNECTION_CLOSED:
        dlc = &uart_data->dlc_table[channel]; /* channel is dlci */
        uart_data->drx   = dlc->drx;
        uart_data->dtx   = dlc->dtx;

        sig_dti_ker_connection_closed_ind(channel);
        break;

      case DTI_REASON_DATA_RECEIVED:
        if(!uart_data)
        {
          /*
           * instance not found, primitive was invalid, discard it
           */
          PFREE_DESC2( dti_data2_ind );
        }
        else
        {
          dlc = &uart_data->dlc_table[channel]; /* channel is dlci */
          uart_data->drx   = dlc->drx;
          uart_data->dtx   = dlc->dtx;

          sig_dti_drx_data_received_ind(dti_data2_ind);
        }
        break;

      case DTI_REASON_TX_BUFFER_FULL:
        if(!uart_data)
        {
          TRACE_ERROR("[PEI_SIG_CALLBACK] instance not valid!");
          return; /* error, not found */
        }
        dlc = &uart_data->dlc_table[channel]; /* channel is dlci */
        uart_data->drx   = dlc->drx;
        uart_data->dtx   = dlc->dtx;

        sig_dti_dtx_tx_buffer_full_ind();
        break;

      case DTI_REASON_TX_BUFFER_READY:
        if(!uart_data)
        {
          TRACE_ERROR("[PEI_SIG_CALLBACK] instance not valid!");
          return; /* error, not found */
        }
        dlc = &uart_data->dlc_table[channel]; /* channel is dlci */
        uart_data->drx   = dlc->drx;
        uart_data->dtx   = dlc->dtx;

        sig_dti_dtx_tx_buffer_ready_ind();
        break;
      default:
        TRACE_ERROR("unknown DTILIB reason parameter");
        break;
    } /* end switch */
  } /* end if */
  else
  {
    TRACE_ERROR("Pointer to DTILIB database not existing");
  }
} /* pei_sig_callback() */

/*==== End of functions only used with DTILIB =================================*/

#endif /* DTILIB */

/*==== END OF FILE ==========================================================*/
