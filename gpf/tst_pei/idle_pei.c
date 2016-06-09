/*
+------------------------------------------------------------------------------
|  File:       idle_pei.c
+------------------------------------------------------------------------------
|                 Copyright Texas Instruments 2002
|                 All rights reserved.
|
+------------------------------------------------------------------------------
| Purpose:     PEI interface for the IDLE entity.
|
| $Identity:$
+------------------------------------------------------------------------------
*/

#define IDLE_PEI_C

/*==== INCLUDES ==============================================================*/

#include "typedefs.h"
#include "vsi.h"
#include "pei.h" 
#include "tools.h"
#include "frame.h"
#include "idle.h"

/*==== CONSTS ================================================================*/

/*==== TYPES =================================================================*/

/*==== LOCALS ================================================================*/

/*==== PRIVATE FUNCTIONS =====================================================*/

/*
+------------------------------------------------------------------------------
|	Function	:  pei_primitive
+------------------------------------------------------------------------------
|   Description	:  This function is called by the frame when a primitive is
|                  received and needs to be processed.
|
|   Parameters	:  prim      - Pointer to the received primitive
|
|   Return		:  PEI_OK    - function succeeded
|                  PEI_ERROR - function failed
+------------------------------------------------------------------------------
*/
static short pei_primitive (void * prim)
{
  if (prim != NULL)
  {
    PFREE(P2D(prim));
  }

  return PEI_OK;
}

/*
+------------------------------------------------------------------------------
|   Function	:  pei_init
+------------------------------------------------------------------------------
|   Description	:  This function is called by the frame. It is used to initialise
|                  the entitiy.
|
|   Parameters	:  handle            - task handle
|
|   Return		:  PEI_OK            - entity initialised
|                  PEI_ERROR         - entity not (yet) initialised
+------------------------------------------------------------------------------
*/
static short pei_init (T_HANDLE handle)
{
  /*
   * Initialize task handle
   */
  TRACE_FUNCTION("pei_init");

  idle_handle = handle;

  /*
   * Open communication channels
   */
  if (hCommTST < VSI_OK)
  {
    if ((hCommTST = vsi_c_open(VSI_CALLER "RCV")) < VSI_OK)
      return PEI_ERROR;
  }

  return (PEI_OK);
}

/*
+------------------------------------------------------------------------------
|   Function	:  pei_signal
+------------------------------------------------------------------------------
|   Description :  This function is called by the frame when a signal has been 
|                  received.
|
|   Parameters  :  opc            - signal operation code
|                  *data          - pointer to primitive
|
|   Return      :  PEI_OK         - signal processed
|                  PEI_ERROR      - signal not processed
|
+------------------------------------------------------------------------------
*/
static short pei_signal (U32 opc, void *data)
{
  /*
   * Process signal
   */

  //TRACE_FUNCTION("pei_signal");
  /* Generate trig in TST */
  PSIGNAL(hCommTST, IDLE_CNF, NULL);  

  return PEI_OK;
} 

/*
+------------------------------------------------------------------------------
|   Function    :  pei_exit
+------------------------------------------------------------------------------
|   Description	:  This function is called by the frame when the entity is
|                  terminated. All open resources are freed.
|
|   Parameters	:  -
|
|   Return		:  PEI_OK         - exit sucessful
|                  PEI_ERROR      - exit not sueccessful
+------------------------------------------------------------------------------
*/
static short pei_exit (void)
{
  /*
   * Close communication channels
   */

  TRACE_FUNCTION("pei_exit");

  vsi_c_close(VSI_CALLER hCommTST);
  hCommTST = VSI_ERROR;

  return PEI_OK;
}

/*
+------------------------------------------------------------------------------
|   Function	:  pei_config
+------------------------------------------------------------------------------
|   Description	:  This function is called by the frame when a primitive is
|                  received indicating dynamic configuration.
|
|                  This function is not used in this entity.
|
|   Parameters  :  handle         - Communication handle
|
|   Return      :  PEI_OK         - sucessful
|                  PEI_ERROR      - not successful
+------------------------------------------------------------------------------
*/
static short pei_config (char *inString)
{
  TRACE_FUNCTION(inString);
  return PEI_OK;
}

/*
+------------------------------------------------------------------------------
|   Function    :  pei_monitor
+------------------------------------------------------------------------------
|   Description	:  This function is called by the frame in case sudden entity
|                  specific data is requested (e.g. entity Version).
|
|   Parameters	:  out_monitor    - return the address of the data to be
|                                   monitoredCommunication handle
|
|   Return      :  PEI_OK         - sucessful (address in out_monitor is valid)
|                  PEI_ERROR      - not successful
+------------------------------------------------------------------------------
*/
static short pei_monitor (void **out_monitor)
{
  TRACE_FUNCTION ("pei_monitor");
  return PEI_OK;
}

/*
+------------------------------------------------------------------------------
|   Function    :  pei_timeout
+------------------------------------------------------------------------------
|   Description :  This function is called by the frame when a timer has expired.
|
|   Parameters  :  index          - timer index
|
|   Return      :  PEI_OK         - timeout processed
|                  PEI_ERROR      - timeout not processed
|
+------------------------------------------------------------------------------
*/
static short pei_timeout (USHORT index)
{
	TRACE_ERROR("Unknown Timeout");

  return PEI_OK;
}

/*==== PUBLIC FUNCTIONS ======================================================*/

/*
+------------------------------------------------------------------------------
|   Function    :  idle_pei_create
+------------------------------------------------------------------------------
|   Description	:  This function is called by the frame when the process is 
|                  created.
|
|   Parameters	:  out_name          - Pointer to the buffer in which to locate
|                                      the name of this entity
|
|   Return		:  PEI_OK            - entity created successfuly
|                  PEI_ERROR         - entity could not be created
+------------------------------------------------------------------------------
*/
short idle_pei_create (T_PEI_INFO **info)
{
static T_PEI_INFO pei_info =
              {
               "IDLE",         /* name */
               {              /* pei-table */
                 pei_init,
                 pei_exit,
                 pei_primitive,
                 pei_timeout,
                 pei_signal,
                 NULL,        /* no pei_run function */
                 pei_config,
                 pei_monitor
               },
               255,           /* stack size */
               10,            /* queue entries */
               2,             /* priority (1->low, 255->high) */
               0,             /* number of timers */
               0x03           /* flags: bit 0   active(0) body/passive(1) */
              };              /*        bit 1   com by copy(0)/reference(1) */

  /*
   * Export startup configuration data
   */
  *info = &pei_info;

  return PEI_OK;
}

/*==== END OF FILE ===========================================================*/
