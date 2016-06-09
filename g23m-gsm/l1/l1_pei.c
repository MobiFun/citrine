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
|             for the entity L1.
+----------------------------------------------------------------------------- 
*/ 

#define L1_PEI_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_L1

#define CUST_OS_C
/*==== INCLUDES =============================================================*/

#if 1
#include <string.h>
#if defined (NEW_FRAME)
 #include "typedefs.h"  /* to get Condat data types */
#else
 #include "stddefs.h"   /* to get Condat data types */
#endif
#include "vsi.h"        /* to get a lot of macros */
#include "custom.h"
#ifdef GPRS
#include "macdef.h"
#include "gprs.h"
#endif
#include "gsm.h"        /* to get a lot of macros */
#include "prim.h"       /* to get the definitions of used SAP and directions */
#include "pei.h"        /* to get PEI interface */
#include "tools.h"      /* to get common tools */
#include "l1.h"         /* to get the global entity definitions */
#include "cust_os.h"    /* to get cust_os definitions */

#else /* 1 */

#if defined NEW_FRAME

#include <string.h>
#include <stdio.h>
#include "typedefs.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_l1.h"
#include "mon_l1.h"
#include "cus_l1.h"
#include "pei.h"
#include "tok.h"
#include "l1.h"

#else

#include <string.h>
#include <stdio.h>
#include "stddefs.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_l1.h"
#include "mon_l1.h"
#include "vsi.h"
#include "pei.h"
#include "tok.h"
#include "l1.h"

#endif
#endif /* 1 */

/*==== CONSTS ================================================================*/

/*==== TYPES =================================================================*/

typedef struct
{
  char              *version;
} T_MONITOR;

/*==== LOCALS ================================================================*/

static  T_MONITOR     l1_mon;
static USHORT first_access = TRUE;

/*==== PROTOTYPES ============================================================*/

SHORT pei_create (T_PEI_INFO **info);

void l1_create_HISR (void);
void l1a_task(unsigned arcg, void *argv);

/*==== PRIVATE FUNCTIONS =====================================================*/


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

                          |           |
                          PL         PPC(GPRS only)    UPLINK
                          |           |
                   +------v-----------v-------+
                   |                          |
                   |         L1 ASYNC         |
                   |                          |
                   +-------------^------------+
                                 |
                               L1 SYNC                 DOWNLINK
                                 |

*/
#if defined (NEW_FRAME)
LOCAL SHORT pei_primitive (void * ptr)
#else
EXPORT T_PEI_RETURN pei_primitive (T_PRIM * prim)
#endif
{
#if defined (NEW_FRAME)
  T_PRIM *prim = (T_PRIM*)ptr;
#endif

  USHORT opc = prim->custom.opc;

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
#if defined (NEW_FRAME)
LOCAL SHORT pei_init ( T_HANDLE Handle )
#else
T_PEI_RETURN pei_init (void)
#endif
{
#if defined (NEW_FRAME)
  L1_Handle = Handle;
#endif

  

#ifdef TI_PS_HCOMM_CHANGE
#ifdef NEW_FRAME
  if ( first_access == TRUE )
  {
    first_access = FALSE;
    _osx_init();
  }
#endif

  if (!cl_hcom_all_handles_open()) return PEI_ERROR;

#ifdef NEW_FRAME
  _osx_open (VSI_CALLER L1_QUEUE, _hCommL1);
#endif
  _osx_open (VSI_CALLER DL_QUEUE, _hCommPL);
  _osx_open (VSI_CALLER RR_QUEUE, _hCommPL);

  _osx_open (VSI_CALLER GPF_ACI_QUEUE, _hCommMMI);
#else
#ifdef NEW_FRAME
  if ( first_access == TRUE )
  {
    first_access = FALSE;
    _osx_init();
  }
  /*
   * Associate own "queue" handle (a task handle in the end) with L1_QUEUE,
   * in order to be able to receive primitives.
   */
  if (hCommL1 < VSI_OK)
  {
    if ((hCommL1 = vsi_c_open (VSI_CALLER L1_NAME)) < VSI_OK)
      return PEI_ERROR;
    _osx_open (VSI_CALLER L1_QUEUE, hCommL1);
  }
#endif

  if (hCommPL < VSI_OK)
  {
    if ((hCommPL = vsi_c_open (VSI_CALLER PL_NAME)) < VSI_OK)
      return PEI_ERROR;
    _osx_open (VSI_CALLER DL_QUEUE, hCommPL);
    _osx_open (VSI_CALLER RR_QUEUE, hCommPL);
  }

  if (hCommACI < VSI_OK)
  {
    if ((hCommACI = vsi_c_open (VSI_CALLER ACI_NAME)) < VSI_OK)
      return PEI_ERROR;
    _osx_open (VSI_CALLER GPF_ACI_QUEUE, hCommACI);
  }
#endif /* TI_PS_HCOMM_CHANGE */

#ifdef GPRS
  if (hCommGRR < VSI_OK)
  {
    if ((hCommGRR = vsi_c_open (VSI_CALLER GRR_NAME)) < VSI_OK)
      return PEI_ERROR;
    _osx_open (VSI_CALLER GRR_QUEUE, hCommGRR);
  }

  if (hCommLLC < VSI_OK)
  {
    if ((hCommLLC = vsi_c_open (VSI_CALLER LLC_NAME)) < VSI_OK)
      return PEI_ERROR;
    _osx_open (VSI_CALLER LLC_QUEUE, hCommLLC);
  }

  if (hCommSNDCP < VSI_OK)
  {
    if ((hCommSNDCP = vsi_c_open (VSI_CALLER SNDCP_NAME)) < VSI_OK)
      return PEI_ERROR;
    _osx_open (VSI_CALLER SNDCP_QUEUE, hCommSNDCP);
  }
#endif

  /*
   * Register VSI_CALLER as generic caller entity.
   */
  _osx_open (VSI_CALLER 0, 0);

  l1_create_HISR ();

  return PEI_OK;
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
#if !defined (NEW_FRAME)

T_PEI_RETURN l1_pei_timeout (T_VSI_THANDLE handle)
{
  TRACE_FUNCTION ("pei_timeout()")

  return PEI_OK;
}

#endif

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
#if defined (NEW_FRAME)
LOCAL SHORT pei_exit (void)
#else
T_PEI_RETURN l1_pei_exit (void)
#endif
{

  

#ifdef TI_PS_HCOMM_CHANGE
#else
  vsi_c_close (VSI_CALLER hCommPL);
  hCommPL = VSI_ERROR;
#endif
  return PEI_OK;
}

/*
+------------------------------------------------------------------------------
|  Function     :  pei_run
+------------------------------------------------------------------------------
|  Description  :  Process Primitives, main loop is located in the
|                  Protocol Stack Entity.
|                  Only needed in active body variant
|
|  Parameters   :  taskhandle  - handle of current process
|                  comhandle   - queue handle of current process
|
|  Return       :  PEI_OK      - sucessful
|                  PEI_ERROR   - not successful
+------------------------------------------------------------------------------
*/
#if defined (NEW_FRAME)
LOCAL SHORT pei_run ( T_HANDLE taskhandle, T_HANDLE comhandle )
#else
T_PEI_RETURN l1_pei_run (T_VSI_CHANDLE comhandle)
#endif
{
  
#ifdef TI_PS_HCOMM_CHANGE
#else
  hCommL1 = comhandle;
#endif
  

  l1a_task (0, NULL);
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
LOCAL SHORT pei_config (T_PEI_CONFIG inString)
{
  

  if ( _osx_config ( inString ) == OSX_OK )
    return PEI_OK;
  else
    return PEI_ERROR;
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
#if defined (NEW_FRAME)
LOCAL SHORT pei_monitor (void ** monitor)
#else
T_PEI_RETURN l1_pei_monitor (void ** monitor)
#endif
{
  

  l1_mon.version = "ALR 2.0.0";
  *monitor = &l1_mon;

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
#if defined (NEW_FRAME)

GLOBAL SHORT l1_pei_create ( T_PEI_INFO **info )
{
  static T_PEI_INFO pei_info =
  {
    "L1",
    {
      pei_init,
#ifdef _SIMULATION_
      pei_exit,
#else
      NULL,
#endif
      NULL,             /* no primitive function */
      NULL,             /* no timeout function */
      NULL,             /* no signal function  */
      pei_run,
      pei_config,
      pei_monitor,
    },
    #if defined (GPRS)
    2800,     /* Stack Size      */
    #else 
    #if defined (FAX_AND_DATA)
    2800,     /* Stack Size      */
    #else
    1805,     /* Stacksize       */
    #endif
    #endif
    10,       /* Queue Entries   */
    235,      /* Priority        */
    0,        /* number of timer */
#ifdef GPRS    
    COPY_BY_REF|TRC_NO_SUSPEND|PRIM_NO_SUSPEND      /* flags           */
#else
    COPY_BY_REF|TRC_NO_SUSPEND|PRIM_NO_SUSPEND      /* flags           */
#endif    
  };

  /*
   * export startup configuration data
   */
  *info = &pei_info;
  /*
   *  Initialize entity data
   */

  return PEI_OK;
}

#else /* NEW_FRAME */

T_PEI_RETURN pei_create (T_VSI_CNAME * name)
{
  TRACE_FUNCTION ("pei_create()")

  *name = L1_NAME;

  return PEI_OK;
}

#endif /* NEW_FRAME */

/*==== END OF FILE ==========================================================*/
