/*
 * This is a bastardized version of L1 PEI that will be used only when
 * building FreeCalypso in the standalone L1 configuration; see README.
 */

#define L1_PEI_C

#define ENTITY_L1

#define CUST_OS_C
/*==== INCLUDES =============================================================*/

#include <string.h>
#include "typedefs.h"	/* to get Condat data types */
#include "vsi.h"        /* to get a lot of macros */
#include "pei.h"        /* to get PEI interface */
#include "tools.h"      /* to get common tools */
#include "cust_os.h"    /* to get cust_os definitions */

/*==== CONSTS ================================================================*/

/*==== TYPES =================================================================*/

typedef struct
{
  char              *version;
} T_MONITOR;

/*==== LOCALS ================================================================*/

static  T_MONITOR     l1_mon;
static USHORT first_access = TRUE;

static T_HANDLE       L1_Handle;

#define VSI_CALLER L1_Handle,

static T_HANDLE       hCommL1  = VSI_ERROR;
static T_HANDLE       hCommFWD = VSI_ERROR;

/*==== PROTOTYPES ============================================================*/

void l1_create_HISR (void);
void l1a_task(unsigned arcg, void *argv);

/*==== PRIVATE FUNCTIONS =====================================================*/


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
LOCAL SHORT pei_init ( T_HANDLE Handle )
{
  L1_Handle = Handle;

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
    if ((hCommL1 = vsi_c_open (VSI_CALLER "L1")) < VSI_OK)
      return PEI_ERROR;
    _osx_open (VSI_CALLER L1_QUEUE, hCommL1);
  }

  /*
   * Redirect all L1-> outbound messages to our special forwarder
   * entity; see README for the explanation.
   */

  if (hCommFWD < VSI_OK)
  {
    if ((hCommFWD = vsi_c_open (VSI_CALLER "L1IF")) < VSI_OK)
      return PEI_ERROR;
  }

  _osx_open (VSI_CALLER DL_QUEUE, hCommFWD);
  _osx_open (VSI_CALLER RR_QUEUE, hCommFWD);
  _osx_open (VSI_CALLER GPF_ACI_QUEUE, hCommFWD);
  _osx_open (VSI_CALLER GRR_QUEUE, hCommFWD);
  _osx_open (VSI_CALLER LLC_QUEUE, hCommFWD);
  _osx_open (VSI_CALLER SNDCP_QUEUE, hCommFWD);

  /*
   * Register VSI_CALLER as generic caller entity.
   */
  _osx_open (VSI_CALLER 0, 0);

  l1_create_HISR ();

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
LOCAL SHORT pei_run ( T_HANDLE taskhandle, T_HANDLE comhandle )
{
  hCommL1 = comhandle;

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
LOCAL SHORT pei_monitor (void ** monitor)
{
  l1_mon.version = "FC standalone L1";
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
GLOBAL SHORT l1_pei_create ( T_PEI_INFO **info )
{
  static T_PEI_INFO pei_info =
  {
    "L1",
    {
      pei_init,
      NULL,
      NULL,             /* no primitive function */
      NULL,             /* no timeout function */
      NULL,             /* no signal function  */
      pei_run,
      pei_config,
      pei_monitor,
    },
    2800,     /* Stack Size      */
    10,       /* Queue Entries   */
    235,      /* Priority        */
    0,        /* number of timer */
    COPY_BY_REF|TRC_NO_SUSPEND|PRIM_NO_SUSPEND      /* flags           */
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

/*==== END OF FILE ==========================================================*/
