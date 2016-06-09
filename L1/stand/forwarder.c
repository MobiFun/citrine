/*
 * This module implements the special "forwarder" (L1IF) GPF entity
 * for L1 standalone operation described in README.
 */

/*==== INCLUDES =============================================================*/

#include <string.h>
#include "typedefs.h"	/* to get Condat data types */
#include "vsi.h"        /* to get a lot of macros */
#include "pei.h"        /* to get PEI interface */
#include "tools.h"      /* to get common tools */
#include "header.h"
#include "os.h"
#include "../../gpf/frame/route.h"

/*==== LOCALS ================================================================*/

static T_HANDLE       Our_Handle;

#define VSI_CALLER Our_Handle,

static T_HANDLE       hCommTST = VSI_ERROR;

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
  Our_Handle = Handle;

  if (hCommTST < VSI_OK)
  {
    if ((hCommTST = vsi_c_open (VSI_CALLER "TST")) < VSI_OK)
      return PEI_ERROR;
  }

  return PEI_OK;
}

LOCAL SHORT pei_primitive (void *primitive)
{
  OS_QDATA OS_Msg = { 0 };
  T_VOID_STRUCT *prim = (T_VOID_STRUCT *) primitive;

  OS_Msg.data16 = MSG_PRIMITIVE;
  OS_Msg.e_id = Our_Handle;
  OS_Msg.ptr = prim;
  OS_Msg.data32 = P_OPC(prim);
  os_GetTime(0, &OS_Msg.time);

  rt_ExtPrimitive(Our_Handle, hCommTST, Our_Handle, "EXT", &OS_Msg);

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
GLOBAL SHORT l1stand_fwd_pei_create ( T_PEI_INFO **info )
{
  static T_PEI_INFO pei_info =
  {
    "L1IF",
    {
      pei_init,
      NULL,
      pei_primitive,
      NULL,             /* no timeout function */
      NULL,             /* no signal function  */
      NULL,		/* no run function */
      NULL,		/* no config function */
      NULL,		/* no monitor function */
    },
    1000,     /* Stack Size      */
    50,       /* Queue Entries   */
    20,       /* Priority        */
    0,        /* number of timer */
    PASSIVE_BODY | COPY_BY_REF | TRC_NO_SUSPEND      /* flags           */
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
