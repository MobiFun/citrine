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
|             for the entity SS of the mobile station.
+-----------------------------------------------------------------------------
*/

#ifndef SS_PEI_C
#define SS_PEI_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_SS

/*==== INCLUDES ===================================================*/
#if defined (NEW_FRAME)

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "typedefs.h"
#include "pcm.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_ss.h"
#include "mon_ss.h"
#include "pei.h"
#include "tok.h"
#include "ss.h"
#include "ss_em.h"

#else

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "stddefs.h"
#include "pcm.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_ss.h"
#include "mon_ss.h"
#include "vsi.h"
#include "pei.h"
#include "tok.h"
#include "ss.h"
#include "ss_em.h"

#endif

/*==== CONST ======================================================*/
/*
 * instance management
 */
#ifdef OPTION_MULTIPLE_INSTANCE
  #define GET_INSTANCE(p)           &ss_data_base[p->custom.route.inst_no]
#else
  #define GET_INSTANCE(p)           &ss_data_base
#endif

/*==== VAR EXPORT =================================================*/
#ifdef TI_PS_HCOMM_CHANGE
#if defined (NEW_FRAME)
GLOBAL T_HANDLE ss_handle;
#endif
#else /* TI_PS_HCOMM_CHANGE */
#if defined (NEW_FRAME)
GLOBAL T_HANDLE hCommMMI = VSI_ERROR;/* MMI Communication */
GLOBAL T_HANDLE hCommMM  = VSI_ERROR;/* MM  Communication */
GLOBAL T_HANDLE ss_handle;
#else
GLOBAL T_VSI_CHANDLE hCommMMI = VSI_ERROR;/* MMI Communication */
GLOBAL T_VSI_CHANDLE hCommMM  = VSI_ERROR;/* MM  Communication */
#endif
#endif /* TI_PS_HCOMM_CHANGE */

#ifdef OPTION_MULTI_INSTANCE
GLOBAL T_SS_DATA ss_data_base [SS_INSTANCES];
#else
GLOBAL T_SS_DATA ss_data_base;
#endif


/*==== VAR LOCAL ==================================================*/
#ifdef _SIMULATION_
LOCAL BOOL              first_access = TRUE;
#endif

LOCAL T_MONITOR         ss_mon;

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : SS_PEI              |
| STATE   : code                       ROUTINE : pei_not_supported   |
+--------------------------------------------------------------------+

  PURPOSE : An unsupported primitive is received.

*/
LOCAL void pei_not_supported (void *data)
{
  TRACE_FUNCTION ("pei_not_supported()");

  PFREE (data)
}

LOCAL const T_FUNC mnss_table[] = {
  MAK_FUNC_0 (ss_mnss_begin_req   , MNSS_BEGIN_REQ   ),
  MAK_FUNC_0 (ss_mnss_facility_req, MNSS_FACILITY_REQ),
  MAK_FUNC_0 (ss_mnss_end_req     , MNSS_END_REQ     )
};

LOCAL const T_FUNC mmss_table[] = {
  MAK_FUNC_S (for_mmss_data_ind     , MMSS_DATA_IND     ),
  MAK_FUNC_0 ( ss_mmss_error_ind    , MMSS_ERROR_IND    ),
  MAK_FUNC_0 ( ss_mmss_establish_cnf, MMSS_ESTABLISH_CNF),
  MAK_FUNC_S (for_mmss_establish_ind, MMSS_ESTABLISH_IND),
  MAK_FUNC_0 ( ss_mmss_release_ind  , MMSS_RELEASE_IND  )
};

#ifdef FF_EM_MODE
LOCAL const T_FUNC em_table[] = {
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x00 */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x01 */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x02 */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x03 */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x04 */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x05 */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x06 */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x07 */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x08 */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x09 */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x0A */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x0B */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x0C */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x0D */
  MAK_FUNC_0 (ss_em_ss_event_req , EM_SS_EVENT_REQ  )  /* 0x0E */
};
#endif /* FF_EM_MODE */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : SS_PEI              |
| STATE   : code                       ROUTINE : pei_primitive       |
+--------------------------------------------------------------------+

  PURPOSE : Process protocol specific primitive.

*/
#if defined (NEW_FRAME)
LOCAL SHORT pei_primitive (void * ptr)
#else
T_PEI_RETURN pei_primitive (T_PRIM * prim)
#endif
{
#if defined (NEW_FRAME)
  T_PRIM *prim = ptr;
#endif
  /*
   *              |          |
   *             MNSS        EM               UPLINK
   *              |          |
   *      +-------v----------v-------+
   *      |                          |
   *      |            SS            |
   *      |                          |
   *      +-------------^------------+
   *                    |
   *                   MMSS                   DOWNLINK
   *                    |
   *
   */

  TRACE_FUNCTION ("pei_primitive()");

  /*
   * No timer functionality
   */

  if (prim NEQ NULL)
  {
    ULONG            opc = prim->custom.opc;
    USHORT           n;
    const T_FUNC    *table;
#if defined (NEW_FRAME)
    VSI_PPM_REC ((T_PRIM_HEADER*)prim, __FILE__, __LINE__);
#endif

    PTRACE_IN (opc);

    switch (SAP_NR(opc))
    {
      case SAP_NR(MNSS_UL): table = mnss_table;  n = TAB_SIZE (mnss_table); break;
      case SAP_NR(MMSS_DL): table = mmss_table;  n = TAB_SIZE (mmss_table); break;
#ifdef FF_EM_MODE
      case EM_Ul:           table = em_table;    n = TAB_SIZE (  em_table); break;
#endif /* FF_EM_MODE */
      default:              table = NULL;        n = 0;                     break;
    }

    if (table != NULL)
    {
      if (PRIM_NR(opc) < n)
      {
        table += PRIM_NR(opc);
#ifdef PALLOC_TRANSITION
        P_SDU(prim) = table->soff ? (T_sdu*) (((char*)&prim->data) + table->soff) : 0;
#ifndef NO_COPY_ROUTING
        P_LEN(prim) = table->size + sizeof (T_PRIM_HEADER);
#endif /* NO_COPY_ROUTING */
#endif /* PALLOC_TRANSITION */
        JUMP (table->func) (P2D(prim));
      }
      else
      {
        pei_not_supported (P2D(prim));
      }
      return PEI_OK;
    }

    /*
     * Primitive is no GSM Primitive
     * then forward to the environment
     */

#ifdef GSM_ONLY
    PFREE (P2D(prim))

    return PEI_ERROR;
#else
    if (opc & SYS_MASK)
      vsi_c_primitive (VSI_CALLER prim);
    else
    {
      PFREE (P2D(prim));
      return PEI_ERROR;
    }
#endif
  }
  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : SS_PEI              |
| STATE   : code                       ROUTINE : pei_init            |
+--------------------------------------------------------------------+

  PURPOSE : Initialize Protocol Stack Entity

*/
#if defined (NEW_FRAME)
LOCAL SHORT pei_init (T_HANDLE handle)
#else
T_PEI_RETURN pei_init (void)
#endif
{
#ifdef OPTION_MULTI_INSTANCE
 USHORT i;
#endif

#if defined (NEW_FRAME)
  ss_handle = handle;
#endif

  TRACE_FUNCTION ("pei_init()");
#ifdef TI_PS_HCOMM_CHANGE
  if (!cl_hcom_all_handles_open())
  {
    return PEI_ERROR;
  }
#else /* for hCommHandles backward compatibility */
  if (hCommMMI < VSI_OK)
  {
    /*
     * Open MMI (Layer 4)
     */

    if ((hCommMMI = vsi_c_open (VSI_CALLER ACI_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

  if (hCommMM < VSI_OK)
  {
    if ((hCommMM = vsi_c_open (VSI_CALLER MM_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif

#ifdef OPTION_MULTI_INSTANCE
  for (i=0;i<MAX_INSTANCES;i++)
    ss_init_ss_data (&ss_data_base[i]);
#else
  ss_init_ss_data ();
#endif
  ccd_init ();

#ifdef FF_EM_MODE
  em_init_ss_event_trace();
#endif /* FF_EM_MODE */

  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : SS_PEI              |
| STATE   : code                       ROUTINE : pei_timeout         |
+--------------------------------------------------------------------+

  PURPOSE : Process timeout

*/
#if !defined (NEW_FRAME)
T_PEI_RETURN pei_timeout (T_VSI_THANDLE handle)
{
  TRACE_FUNCTION ("pei_timeout ()");
  /*
   * No Timer Functionality
   */

  return PEI_OK;
}
#endif


#ifdef _SIMULATION_
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : SS_PEI              |
| STATE   : code                       ROUTINE : pei_exit            |
+--------------------------------------------------------------------+

  PURPOSE : Close Resources and terminate

*/
#if defined (NEW_FRAME)
LOCAL SHORT pei_exit (void)
#else
T_PEI_RETURN pei_exit (void)
#endif
{
  TRACE_FUNCTION ("pei_exit()");

  /*
   * clean up communication
   */
#ifdef TI_PS_HCOMM_CHANGE
#else /* for hCommHandles backward compatibility */
  vsi_c_close (VSI_CALLER hCommMMI);
  hCommMMI = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommMM);
  hCommMM = VSI_ERROR;
#endif
  return PEI_OK;
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : SS_PEI              |
| STATE   : code                       ROUTINE : pei_run             |
+--------------------------------------------------------------------+

  PURPOSE : Process Primitives, main loop is located in the
            Protocol Stack Entity

*/
#if !defined (NEW_FRAME)
T_PEI_RETURN pei_run (T_VSI_CHANDLE handle)
{
  return PEI_OK;
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : SS_PEI              |
| STATE   : code                       ROUTINE : pei_config          |
+--------------------------------------------------------------------+

  PURPOSE : Dynamic Configuration

*/
/* Implements Measure#36 */
#ifndef NCONFIG
#if defined (NEW_FRAME)
LOCAL SHORT pei_config (T_PEI_CONFIG inString)
#else
T_PEI_RETURN pei_config (T_PEI_CONFIG inString,
                         T_PEI_CONFIG outString)
#endif
{
  return PEI_OK;
}
#endif /* !NCONFIG */

#if defined (NEW_FRAME)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : SS_PEI              |
| STATE   : code                       ROUTINE : ss_pei_config       |
+--------------------------------------------------------------------+

  PURPOSE : Dynamic Configuration

*/
/* Implements Measure#36 */
#ifndef NCONFIG
GLOBAL SHORT ss_pei_config ( char * inString, char * dummy )
{
  pei_config ( inString );

  return PEI_OK;
}
#endif /* !NCONFIG */
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : SS_PEI              |
| STATE   : code                       ROUTINE : pei_monitor         |
+--------------------------------------------------------------------+

  PURPOSE : Monitoring of physical Parameters

*/
#if defined (NEW_FRAME)
LOCAL SHORT pei_monitor (void ** monitor)
#else
T_PEI_RETURN pei_monitor (void ** monitor)
#endif
{
  TRACE_FUNCTION ("pei_monitor()");

/* Implements Measure#32: Row 12 */

  *monitor = &ss_mon;

  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : SS_PEI              |
| STATE   : code                       ROUTINE : pei_create          |
+--------------------------------------------------------------------+

  PURPOSE : Create the Protocol Stack Entity

*/
#if defined (NEW_FRAME)
GLOBAL SHORT ss_pei_create (T_PEI_INFO **info)
{
  static const T_PEI_INFO pei_info =
  {
    "SS",
    {
      pei_init,
#ifdef _SIMULATION_
      pei_exit,
#else
      NULL,
#endif
      pei_primitive,
      NULL,             /* no timeout function */
      NULL,             /* no signal function  */
      NULL,             /* no run function     */
/* Implements Measure#36 */
#ifdef NCONFIG
      NULL,             /* no pei_config function     */
#else /* NCONFIG */
      pei_config,
#endif /* NCONFIG */
      pei_monitor,
    },
    924,      /* Stack Size      */
    10,       /* Queue Entries   */
    145,      /* Priority        */
    0,        /* number of timer */
    0x03|PRIM_NO_SUSPEND /* flags           */
  };

  TRACE_FUNCTION ("pei_create()");

  /*
   *  Close Resources if open
   */

#ifdef _SIMULATION_
  if (first_access)
    first_access = FALSE;
  else
    pei_exit ();
#endif

  /*
   *  Export startup configuration data
   */

  *info = (T_PEI_INFO *)&pei_info;

  return PEI_OK;
}

#else

T_PEI_RETURN pei_create (T_VSI_CNAME * name)
{
  TRACE_FUNCTION ("pei_create()")

  /*
   *  Close Resources if open
   */

#ifdef _SIMULATION_
  if (first_access)
    first_access = FALSE;
  else
    pei_exit ();
#endif

  /*
   *  Initialize entity data
   */

  *name = SS_NAME;

  return PEI_OK;
}
#endif

#endif
