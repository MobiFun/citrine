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
|             for the entity CC of the mobile station.
+-----------------------------------------------------------------------------
*/

#ifndef CC_PEI_C
#define CC_PEI_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_CC
/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "typedefs.h"
#include "pcm.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "message.h"
#include "ccdapi.h"
#include "prim.h"
#include "cus_cc.h"
#include "cnf_cc.h"
#include "mon_cc.h"
#include "pei.h"
#include "tok.h"
#include "cc.h"
#include "cc_em.h"


/*==== EXPORT =====================================================*/
#ifdef TI_PS_HCOMM_CHANGE
#else
GLOBAL T_HANDLE hCommMMI  = VSI_ERROR;    /* MMI Communication */
GLOBAL T_HANDLE hCommMM   = VSI_ERROR;    /* MM  Communication */
#endif
GLOBAL T_HANDLE cc_handle = VSI_ERROR;

#ifdef OPTION_MULTIPLE_INSTANCE
GLOBAL T_CC_DATA data_base [MAX_INSTANCES];
#else
GLOBAL T_CC_DATA data_base;
#endif


/*==== PRIVATE ====================================================*/

static void pei_not_supported (void *data);

/*==== VARIABLES ==================================================*/
#ifdef _SIMULATION_
static BOOL              first_access = TRUE;
#endif /* _SIMULATION_ */
static T_MONITOR         cc_mon;

/*==== FUNCTIONS ==================================================*/

LOCAL const T_FUNC mncc_table[] = {
  MAK_FUNC_0( cc_mncc_setup_req,       MNCC_SETUP_REQ      ), /* 0x00 */
  MAK_FUNC_0( cc_mncc_setup_res,       MNCC_SETUP_RES      ), /* 0x01 */
  MAK_FUNC_0( cc_mncc_alert_req,       MNCC_ALERT_REQ      ), /* 0x02 */
  MAK_FUNC_0( cc_mncc_disconnect_req,  MNCC_DISCONNECT_REQ ), /* 0x03 */
  MAK_FUNC_0( cc_mncc_release_req,     MNCC_RELEASE_REQ    ), /* 0x04 */
  MAK_FUNC_0( cc_mncc_modify_req,      MNCC_MODIFY_REQ     ), /* 0x05 */
  MAK_FUNC_0( cc_mncc_configure_req,   MNCC_CONFIGURE_REQ  ), /* 0x06 */
  MAK_FUNC_N( pei_not_supported,  0                        ), /* 0x07 */ /*removed mncc_notify_XXX*/
  MAK_FUNC_0( cc_mncc_start_dtmf_req,  MNCC_START_DTMF_REQ ), /* 0x08 */
  MAK_FUNC_0( cc_mncc_hold_req,        MNCC_HOLD_REQ       ), /* 0x09 */
  MAK_FUNC_0( cc_mncc_retrieve_req,    MNCC_RETRIEVE_REQ   ), /* 0x0a */
  MAK_FUNC_0( cc_mncc_facility_req,    MNCC_FACILITY_REQ   ), /* 0x0b */
  MAK_FUNC_0( cc_mncc_user_req,        MNCC_USER_REQ       ), /* 0x0c */
#ifdef SIM_TOOLKIT
  MAK_FUNC_N( cc_mncc_bearer_cap_req,  MNCC_BEARER_CAP_REQ ), /* 0x0d */
#else
  MAK_FUNC_N( pei_not_supported,       0                   ), /* 0x0d */
#endif /*SIM_TOOLKIT */
  MAK_FUNC_0( cc_mncc_prompt_res,      MNCC_PROMPT_RES     ), /* 0x0e */
  MAK_FUNC_0( cc_mncc_prompt_rej,      MNCC_PROMPT_REJ     ), /* 0x0f */
  MAK_FUNC_0( cc_mncc_reject_req,      MNCC_REJECT_REQ     ), /* 0x10 */
  MAK_FUNC_0( cc_mncc_sync_req,        MNCC_SYNC_REQ       ), /* 0x11 */
  MAK_FUNC_0( cc_mncc_status_res,      MNCC_STATUS_RES     )  /* 0x12 */
};


LOCAL const T_FUNC mmcm_table[] = {
  MAK_FUNC_0( for_mmcm_est_cnf,        MMCM_ESTABLISH_CNF  ), /* 0x00 */
  MAK_FUNC_S( for_mmcm_est_ind,        MMCM_ESTABLISH_IND  ), /* 0x01 */
  MAK_FUNC_S( for_mmcm_data_ind,       MMCM_DATA_IND       ), /* 0x02 */
  MAK_FUNC_0( for_mmcm_rel_ind,        MMCM_RELEASE_IND    ), /* 0x03 */
  MAK_FUNC_0( for_mmcm_err_ind,        MMCM_ERROR_IND      ), /* 0x04 */
  MAK_FUNC_N( pei_not_supported,       0                   ), /* 0x05 */
  MAK_FUNC_0( for_mmcm_reest_cnf,      MMCM_REESTABLISH_CNF), /* 0x06 */
  MAK_FUNC_0( for_mmcm_prompt_ind,     MMCM_PROMPT_IND     ), /* 0x07 */
  MAK_FUNC_N( pei_not_supported,       0                   ), /* 0x08 */
  MAK_FUNC_0( for_mmcm_sync_ind,       MMCM_SYNC_IND       )  /* 0x09 */
};



#ifdef FF_EM_MODE
LOCAL const T_FUNC em_table[] = {
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x3E00 */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x3E01 */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x3E02 */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x3E03 */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x3E04 */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x3E05 */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x3E06 */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x3E07 */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x3E08 */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x3E09 */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x3E0A */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x3E0B */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x3E0C */
  MAK_FUNC_0 (cc_em_cc_event_req , EM_CC_EVENT_REQ  ), /* 0x3E0D */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x3E0E */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x3E0F */
  MAK_FUNC_N (pei_not_supported  , 0                ), /* 0x3E10 */
  MAK_FUNC_N (pei_not_supported  , 0                )  /* 0x3E11 */
};
#endif /* FF_EM_MODE */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : CC_PEI              |
| STATE   : code                       ROUTINE : pei_primitive       |
+--------------------------------------------------------------------+

  PURPOSE : Process protocol specific primitive.

*/

LOCAL SHORT pei_primitive (void * ptr)
{
  T_PRIM *prim = ptr;
  /*
   *              |          |
   *             MNCC        EM            UPLINK
   *              |          |
   *      +-------v----------v-------+  
   *      |            CC            |
   *      |                          |
   *      +-------------^------------+
   *                    |
   *                  MMCM                 DOWNLINK
   *                    |
   *
   */

  TRACE_FUNCTION ("pei_primitive()");
  
  if (prim NEQ NULL)
  {
    ULONG            opc = prim->custom.opc;
    USHORT           n;
    const T_FUNC    *table;

    VSI_PPM_REC ((T_PRIM_HEADER*)prim, __FILE__, __LINE__);

    PTRACE_IN (opc);

    switch (SAP_NR(opc))
    {
      case SAP_NR(MNCC_UL): table =  mncc_table; n = TAB_SIZE (mncc_table); break;
      case SAP_NR(MMCM_DL): table =  mmcm_table; n = TAB_SIZE (mmcm_table); break;
#ifdef FF_EM_MODE
      case   EM_Ul: table = em_table;    n = TAB_SIZE (  em_table); break;
#endif /* FF_EM_MODE */

      default     : table =  NULL;       n = 0;                     break;
    }

    if (table != NULL )
    {
      if (PRIM_NR(opc) < n)
      {
        table += PRIM_NR(opc);
#ifdef PALLOC_TRANSITION
        P_SDU(prim) = table->soff ? (T_sdu*) (((char*)&prim->data) + table->soff) : 0;
#ifndef NO_COPY_ROUTING
        P_LEN(prim) = table->size + sizeof (T_PRIM_HEADER);
#endif /* #ifndef NO_COPY_ROUTING */
#endif /* #ifdef PALLOC_TRANSITION */
        JUMP (table->func) (P2D(prim));
      }
      else
      {
        pei_not_supported (P2D(prim));
      }
      return PEI_OK;
    }

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
#endif /* else, #ifdef GSM_ONLY */
  }
  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : CC_PEI              |
| STATE   : code                       ROUTINE : cc_pei_primitive    |
+--------------------------------------------------------------------+

  PURPOSE : Re-use a stored primitive

*/

GLOBAL void cc_pei_primitive (T_PRIM * prim)
{
  pei_primitive (prim); 
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : CC_PEI              |
| STATE   : code                       ROUTINE : pei_not_supported   |
+--------------------------------------------------------------------+

  PURPOSE : An unsupported primitive is received.

*/

LOCAL void pei_not_supported (void *data)
{
  TRACE_FUNCTION ("pei_not_supported()");

  PFREE (data);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : CC_PEI              |
| STATE   : code                       ROUTINE : pei_init            |
+--------------------------------------------------------------------+

  PURPOSE : Initialize Protocol Stack Entity

*/

LOCAL SHORT pei_init (T_HANDLE handle)
{
  cc_handle = handle;

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

  /*
   * Initialize data base
   */

  ccd_init ();
  pcm_Init ();
  cc_init ();
  for_init ();

#ifdef FF_EM_MODE
  em_init_cc_event_trace();
#endif /* FF_EM_MODE */

  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : CC_PEI              |
| STATE   : code                       ROUTINE : pei_timeout         |
+--------------------------------------------------------------------+

  PURPOSE : Process timeout

*/

LOCAL SHORT pei_timeout (USHORT index)
{
  tim_exec_timeout (index);
  return PEI_OK;
}

#ifdef _SIMULATION_ 
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : CC_PEI              |
| STATE   : code                       ROUTINE : pei_exit            |
+--------------------------------------------------------------------+

  PURPOSE : Close Resources and terminate

*/
LOCAL SHORT pei_exit (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("pei_exit()");
  /*
   * Free all allocated primitives
   */
  srv_free_stored_setup ();

  if (cc_data->stored_ccbs_setup NEQ NULL)
  {
    PFREE (cc_data->stored_ccbs_setup);
    cc_data->stored_ccbs_setup = NULL;
  }

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
#endif /* _SIMULATION_ */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : CC_PEI              |
| STATE   : code                       ROUTINE : pei_config          |
+--------------------------------------------------------------------+

  PURPOSE : Dynamic Configuration

*/

#if !defined (NCONFIG)
LOCAL const KW_DATA kwtab[] = {
#if defined (WIN32)
       CC_STD,              ID_STD,
#endif
                   "",                  0
                  };
#endif /* !NCONFIG */

/* Implements Measure#36 */
#if !defined(NCONFIG)
LOCAL SHORT pei_config (T_PEI_CONFIG inString)
{
  char    * s = inString;
  SHORT     valno;
  char    * keyw;
  char    * val [10];

  TRACE_FUNCTION ("pei_config()");
  TRACE_EVENT (s);

  tok_init(s);

  /*
   * Parse next keyword and number of variables
   */
  while ((valno=tok_next(&keyw,val))  NEQ TOK_EOCS)
  {
    switch ( tok_key((KW_DATA *)kwtab,keyw))
    {
      case TOK_NOT_FOUND:
        TRACE_ERROR ("[PEI_CONFIG]: Illegal Keyword");
        TRACE_EVENT_P1("val no: %d", valno); /* Dummy trace event to avoid warning for valno*/
        break;
#ifdef WIN32
      case ID_STD:
        if (valno EQ 1)
        {
          EXTERN UBYTE pcm_read;
          std = atoi (val[0]);
          pcm_read = FALSE;
          pcm_Init ();
        }
        else
        {
          TRACE_ERROR ("[PEI_CONFIG]: Wrong Number of Parameters");
        }
        break;
#endif /* WIN32 */

      default:
        break;
    }


  }
  return PEI_OK;
}
#endif /* !NCONFIG */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : CC_PEI              |
| STATE   : code                       ROUTINE : cc_pei_config       |
+--------------------------------------------------------------------+

  PURPOSE : Dynamic Configuration

*/
/* Implements Measure#36 */
#if !defined(NCONFIG)
GLOBAL SHORT cc_pei_config ( char * inString, char * dummy )
{
  pei_config ( inString );

  return PEI_OK;
}
#endif /* !NCONFIG */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : CC_PEI              |
| STATE   : code                       ROUTINE : pei_monitor         |
+--------------------------------------------------------------------+

  PURPOSE : Monitoring of physical Parameters

*/


LOCAL SHORT pei_monitor (void ** monitor)
{
  TRACE_FUNCTION ("pei_monitor()");

/* Implements Measure#32: Row 99 */

  * monitor = &cc_mon;

  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : CC_PEI              |
| STATE   : code                       ROUTINE : pei_create          |
+--------------------------------------------------------------------+

  PURPOSE : Create the Protocol Stack Entity

*/


GLOBAL SHORT cc_pei_create (T_PEI_INFO **info)
{
  static const T_PEI_INFO pei_info =
  {
    "CC",
    {
      pei_init,
#ifdef _SIMULATION_
      pei_exit,
#else
      NULL,
#endif
      pei_primitive,
      pei_timeout,
      NULL,             /* no signal function  */
      NULL,             /* no run function     */
/* Implements Measure#36 */
#if defined(NCONFIG)
      NULL,             /* no pei_config function     */
#else /* not NCONFIG */
      pei_config,
#endif /* NCONFIG */
      pei_monitor,
    },
    1024,     /* Stack Size      */
    10,       /* Queue Entries   */
    165,      /* Priority        */
    NUM_OF_CC_TIMERS,        /* number of timer */
    0x03|PRIM_NO_SUSPEND     /* flags           */
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

#endif /* #ifndef CC_PEI_C */
