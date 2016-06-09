/* 
+------------------------------------------------------------------------------
|  File:       gdd_dio_pei.c
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
|  Purpose :  This module implements the PEI interface
|             for the entity gdd_dio.
+----------------------------------------------------------------------------- 
*/ 

#define GDD_DIO_PEI_C

#define ENTITY_GDD_DIO

/*==== INCLUDES =============================================================*/

#include "typedefs.h"   /* to get standard types */
#include "vsi.h"        /* to get a lot of macros */
#include "gsm.h"        /* to get a lot of macros */
#include "custom.h"     /* to get GDD_DIO_NAME */
#include "prim.h"       /* to get the definitions of used SAP and directions */
#include "pei.h"        /* to get PEI interface */
#include "tools.h"      /* to get common tools */
#include "mon_gdd_dio.h"    /* to get mon-definitions */
#include "gdd_dio.h"        /* to get the global entity definitions */
#include "string.h"     /* for strncmp */

#include "gdd_dio_kerf.h"

#include "gdd_dio_rxf.h"  /* Needed for gdd_dio_rx_sig_send_data() */
#include "gdd_dio_drxf.h" /* Needed for gdd_dio_drx_sig_receive_data */


/*==== CONSTS ================================================================*/

/* 
 * Wait as long as possible. This is the time in ms that is waited for a 
 * message in the input queue in the active variant. 
 * It can be decreased by the customer.  
 */   
#define GDD_DIO_TIMEOUT     0xffffffff   

/*==== TYPES =================================================================*/

/* Entry point for testing BAT via the BAT adapter (this entity) */
GLOBAL void gdd_dio_test_bat(const char * in_string);

/*==== LOCALS ================================================================*/

static  T_MONITOR     gdd_dio_mon;

/*
 * Function is needed for grr_table[]. This declaration can be removed
 * as soon as this function is no more called (i.e. all primitives are
 * handled).
 */
static void primitive_not_supported (void *data);



#ifdef _SIMULATION_
/* Function to activate/de-activate the PSI STUB for simulation */
void dio_il_sim_activate_psi_stub(BOOL b);
void dio_il_psi_stub_activate_batlib_test(BOOL b);
#endif /* _SIMULATION */


/*==== PRIVATE FUNCTIONS ====================================================*/

/*
+------------------------------------------------------------------------------
|  Function     :  primitive_not_supported
+------------------------------------------------------------------------------
|  Description  :  This function handles unsupported primitives.
|
|  Parameters   :  data - not used
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
static void primitive_not_supported (void *data)
{
  TRACE_FUNCTION ("[GDD] primitive_not_supported()");

  PFREE (data);
}

/*
+------------------------------------------------------------------------------
|  Function     :  pei_primitive
+------------------------------------------------------------------------------
|  Description  :  Process protocol specific primitive.
|
|  Parameters   :  prim      - pointer to the received primitive
|
|  Return       :  PEI_OK    - function succeeded
|                  PEI_ERROR - function failed
+------------------------------------------------------------------------------
*/
static short pei_primitive (void * ptr)
{
T_PRIM *prim = (T_PRIM*)ptr;


  TRACE_FUNCTION ("[GDD] pei_primitive()");

  if (prim NEQ NULL)
  {
    unsigned long      opc = prim->custom.opc;
    unsigned short     n;
    const T_FUNC       *table;

    /*
     * This must be called for Partition Pool supervision. Will be replaced
     * by another macro some time.
     */
    VSI_PPM_REC (&prim->custom, __FILE__, __LINE__);

    PTRACE_IN (opc);

    switch (opc & OPC_MASK)
    {
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
        P_SDU(prim) = table->soff ? 
          (T_sdu*) (((char*)&prim->data) + table->soff) : 0;
#ifndef NO_COPY_ROUTING
        P_LEN(prim) = table->size + sizeof (T_PRIM_HEADER);
#endif /* #ifndef NO_COPY_ROUTING */
#endif /* #ifdef PALLOC_TRANSITION */
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
      vsi_c_primitive (VSI_CALLER prim);
    else
    {
      PFREE (P2D(prim));
      return PEI_ERROR;
    }
  }
  return PEI_OK;
}

/*
+------------------------------------------------------------------------------
|  Function     : pei_init
+------------------------------------------------------------------------------
|  Description  :  Initialize Protocol Stack Entity
|
|  Parameters   :  handle    - task handle
|
|  Return       :  PEI_OK    - entity initialised
|                  PEI_ERROR - entity not (yet) initialised
+------------------------------------------------------------------------------
*/
static short pei_init (T_HANDLE handle)
{
  TRACE_FUNCTION ("[GDD] pei_init()");

  /* Initialize task handle */
  GDD_DIO_handle = handle;
  
  /*
   * Open communication channels
   */
  if (hCommGDD_DIO < VSI_OK)
  {
    if ((hCommGDD_DIO = vsi_c_open (VSI_CALLER GDD_DIO_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
  /*
   * Initialize entity data (call init function of every service)
   */
  gdd_dio_ker_init();

  return (PEI_OK);
}

/*
+------------------------------------------------------------------------------
|  Function     :  pei_timeout
+------------------------------------------------------------------------------
|  Description  :  Process timeout.
|
|  Parameters   :  index     - timer index
|
|  Return       :  PEI_OK    - timeout processed
|                  PEI_ERROR - timeout not processed
+------------------------------------------------------------------------------
*/
static short pei_timeout (unsigned short index)
{
  TRACE_FUNCTION ("[GDD] pei_timeout()");

  /* Process timeout */
  switch (index)
  {
    case 0:
      /* Call of timeout routine */
      break;
    default:
      TRACE_ERROR("[GDD] Unknown Timeout()");
      return PEI_ERROR;
  }

  return PEI_OK;
}

/*
+------------------------------------------------------------------------------
|  Function     :  pei_signal
+------------------------------------------------------------------------------
|  Description  :  Process signal.
|
|  Parameters   :  opc       - signal operation code
|                  data      - pointer to primitive
|
|  Return       :  PEI_OK    - signal processed
|                  PEI_ERROR - signal not processed
+------------------------------------------------------------------------------
*/
static short pei_signal (unsigned long opc, void* data)
{
#ifdef GDD_MAKE_DTX_CONTEXT_SWITCH
  U32 con_handle = opc & GDD_DIO_SIGNAL_CON_HANDLE_MASK;
#else /* GDD_MAKE_DTX_CONTEXT_SWITCH */
  U32 con_handle = opc;
#endif /* GDD_MAKE_DTX_CONTEXT_SWITCH */ 

  TRACE_FUNCTION ("[GDD] pei_signal()");

#ifdef GDD_MAKE_DTX_CONTEXT_SWITCH
  /* Process signal */
  switch (opc & GDD_DIO_SIGNAL_MASK)
  {
  case GDD_DIO_SIGNAL_SEND_DATA:
    gdd_dio_rx_sig_send_data(con_handle, (T_dio_buffer *)data);
    break;

  case GDD_DIO_SIGNAL_RECEIVE_DATA:
    gdd_dio_drx_sig_receive_data(con_handle, (T_dio_buffer *)data);
    break;

  default:
      TRACE_ERROR("[GDD] Unknown Signal OPC");
      return PEI_ERROR;
  }
#else /* GDD_MAKE_DTX_CONTEXT_SWITCH */
  /* If no context switch is done, there is only one signal to be handled. */
  gdd_dio_drx_sig_receive_data(con_handle, (T_dio_buffer *)data);
#endif /* GDD_MAKE_DTX_CONTEXT_SWITCH */

  return PEI_OK;
}

/*
+------------------------------------------------------------------------------
|  Function     :  pei_exit
+------------------------------------------------------------------------------
|  Description  :  Close Resources and terminate.
|
|  Parameters   :            - 
|
|  Return       :  PEI_OK    - exit sucessful
+------------------------------------------------------------------------------
*/
static short pei_exit (void)
{
  TRACE_FUNCTION ("[GDD] pei_exit");

  /*
   * Close communication channels
   */
  vsi_c_close (VSI_CALLER hCommGDD_DIO);
  hCommGDD_DIO = VSI_ERROR;


  return PEI_OK;
}


/*
+------------------------------------------------------------------------------
|  Function     :  pei_config
+------------------------------------------------------------------------------
|  Description  :  Dynamic Configuration.
|
|  Parameters   :  in_string   - configuration string
|
|  Return       :  PEI_OK      - sucessful
|                  PEI_ERROR   - not successful
+------------------------------------------------------------------------------
*/
static short pei_config (char *in_string)
{
  TRACE_FUNCTION ("[GDD] pei_config()");
  TRACE_FUNCTION (in_string);

  if ( ConfigTimer ( VSI_CALLER in_string, NULL ) == VSI_OK )
    return PEI_OK;

 /*
  *  further dynamic configuration
  */
#ifdef _SIMULATION_
  if (!strncmp("sizeof(T_GDD_DIO_CON_DATA)",in_string,7))
  {
    TRACE_EVENT_P1("[GDD] sizeof(T_GDD_DIO_CON_DATA) = %d", sizeof(T_GDD_DIO_CON_DATA));
  }
  else if (!strncmp("PSI STUB ON",in_string, 11))
  {
    TRACE_EVENT("[GDD] Activating the PSI STUB");
    dio_il_sim_activate_psi_stub (TRUE);
  }
  else if (!strncmp("PSI STUB OFF",in_string, 12))
  {
    TRACE_EVENT("[GDD] Deactivating the PSI STUB");
    dio_il_sim_activate_psi_stub (FALSE);
  }
  else if (!strncmp("BATLIB TEST ON",in_string, 14))
  {
    TRACE_EVENT("[GDD] Activating BATlib testing in PSI stub");
    dio_il_psi_stub_activate_batlib_test (TRUE);
  }
  else if (!strncmp("BATLIB TEST OFF",in_string, 15))
  {
    TRACE_EVENT("[GDD] Deactivating BATlib testing in PSI stub");
    dio_il_psi_stub_activate_batlib_test (FALSE);
  }

#endif /* _SIMULATION_ */

  return PEI_OK;
}

/*
+------------------------------------------------------------------------------
|  Function     :  pei_monitor
+------------------------------------------------------------------------------
|  Description  :  Monitoring of physical Parameters.
|
|  Parameters   :  out_monitor - return the address of the data to be monitored
|
|  Return       :  PEI_OK      - sucessful (address in out_monitor is valid)
|                  PEI_ERROR   - not successful
+------------------------------------------------------------------------------
*/
static short pei_monitor (void ** out_monitor)
{
  TRACE_FUNCTION ("[GDD] pei_monitor()");

  /*
   * Version = "0.S" (S = Step).
   */
  gdd_dio_mon.version = "gdd_dio 0.1";
  *out_monitor = &gdd_dio_mon;

  return PEI_OK;
}

/*==== PUBLIC FUNCTIONS =====================================================*/

/*
+------------------------------------------------------------------------------
|  Function     :  pei_create
+------------------------------------------------------------------------------
|  Description  :  Create the Protocol Stack Entity.
|
|  Parameters   :  info        - Pointer to the structure of entity parameters
|
|  Return       :  PEI_OK      - entity created successfully
|                  
+------------------------------------------------------------------------------
*/
short gdd_dio_pei_create (T_PEI_INFO **info)
{
  static T_PEI_INFO pei_info =
  {
    GDD_DIO_NAME,  /* name */
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
      NULL,       /* No pei_run function */
      pei_config,
      pei_monitor
    },
    /*1024,*/          /* stack size */ 
    2048,          /* :TODO: check why we need to increase the stack size */
    10,            /* queue entries */
    100,           /* priority (1->low, 255->high) */
    0,             /* number of timers */
    0x03           /* flags: bit 0   active(0) body/passive(1) */
  };               /*        bit 1   com by copy(0)/reference(1) */

  /*
   * Export startup configuration data
   */
  *info = &pei_info;

  return PEI_OK;
}

/*==== END OF FILE ==========================================================*/
