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
|             for the entity SMS of the mobile station.
+----------------------------------------------------------------------------- 
*/ 

#ifndef SMS_PEI_C
#define SMS_PEI_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_SMS

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
#include "cus_sms.h"
#include "cnf_sms.h"
#include "mon_sms.h"
#include "pei.h"
#include "tok.h"
#include "sms.h"
#include "sms_em.h"

#ifdef GPRS
#include "gprs.h"
#endif /* #ifdef GPRS */

/*==== EXPORT =====================================================*/
#ifdef TI_PS_HCOMM_CHANGE
#else
GLOBAL T_HANDLE hCommSIM   = VSI_ERROR;    /* SIM Communication */
GLOBAL T_HANDLE hCommMMI   = VSI_ERROR;    /* MMI Communication */
GLOBAL T_HANDLE hCommMM    = VSI_ERROR;    /* MM  Communication */
#endif /* TI_PS_HCOMM_CHANGE */
#ifdef GPRS
  GLOBAL T_HANDLE hCommLLC = VSI_ERROR;    /* LL  Communication */
  GLOBAL T_HANDLE hCommGMM = VSI_ERROR;    /* GMM Communication */
#endif /* #ifdef GPRS */
GLOBAL T_HANDLE sms_handle;

GLOBAL T_SMS_DATA sms_data_base;

/*==== PRIVATE ====================================================*/

static void pei_not_supported (void *data);

/*==== VARIABLES ==================================================*/
#ifdef _SIMULATION_
static BOOL              first_access = TRUE;
#endif /* _SIMULATION_ */
static T_MONITOR         sms_mon;

/*==== FUNCTIONS ==================================================*/

LOCAL const T_FUNC mnsms_table[] = {
  MAK_FUNC_0( tl_mnsms_delete_req,      MNSMS_DELETE_REQ      ),
  MAK_FUNC_0( tl_mnsms_read_req,        MNSMS_READ_REQ        ),
  MAK_FUNC_0( tl_mnsms_store_req,       MNSMS_STORE_REQ       ),
  MAK_FUNC_0( tl_mnsms_submit_req,      MNSMS_SUBMIT_REQ      ),
  MAK_FUNC_0( tl_mnsms_command_req,     MNSMS_COMMAND_REQ     ),
  MAK_FUNC_0( tl_mnsms_configure_req,   MNSMS_CONFIGURE_REQ   ),
  MAK_FUNC_0( tl_mnsms_pause_req,       MNSMS_PAUSE_REQ       ),
  MAK_FUNC_0( tl_mnsms_resume_req,      MNSMS_RESUME_REQ      ),
  MAK_FUNC_0( tl_mnsms_ack_res,         MNSMS_ACK_RES         ),
#ifdef REL99
  MAK_FUNC_0( tl_mnsms_retrans_req,     MNSMS_RETRANS_REQ     ),
#else
  MAK_FUNC_N( pei_not_supported,        0                     ),
#endif
  MAK_FUNC_N( pei_not_supported,        0                     ),
  MAK_FUNC_0( tl_mnsms_query_req,       MNSMS_QUERY_REQ       )
 
#ifdef GPRS
 ,
  MAK_FUNC_0( tl_mnsms_mo_serv_req,     MNSMS_MO_SERV_REQ     )
#endif /* #ifdef GPRS */
#ifdef SIM_PERS_OTA
,
  MAK_FUNC_0( tl_mnsms_OTA_message_res,  MNSMS_OTA_MESSAGE_RES  )
#endif
};

LOCAL const T_FUNC mmsms_table[] = {
  MAK_FUNC_S( for_mmsms_data_ind,       MMSMS_DATA_IND        ),
  MAK_FUNC_0( cp_mmsms_error_ind,       MMSMS_ERROR_IND       ),
  MAK_FUNC_0( cp_mmsms_establish_cnf,   MMSMS_ESTABLISH_CNF   ),
  MAK_FUNC_S( for_mmsms_establish_ind,  MMSMS_ESTABLISH_IND   ),
  MAK_FUNC_0( cp_mmsms_release_ind,     MMSMS_RELEASE_IND     ),
  MAK_FUNC_N( pei_not_supported,        MMSMS_UNITDATA_IND    )
};

LOCAL const T_FUNC   sim_table[] = {
#if defined(SIM_TOOLKIT) OR defined(FF_CPHS)
  MAK_FUNC_0( tl_sim_read_cnf,          SIM_READ_CNF          ),
#else
  MAK_FUNC_N( pei_not_supported,        SIM_READ_CNF          ),
#endif /* else, #if defined(SIM_TOOLKIT) OR defined(FF_CPHS) */
  MAK_FUNC_0( tl_sim_update_cnf,        SIM_UPDATE_CNF        ),
  MAK_FUNC_0( tl_sim_read_record_cnf,   SIM_READ_RECORD_CNF   ),
  MAK_FUNC_N( pei_not_supported,        0                     ),
  MAK_FUNC_0( tl_sim_update_record_cnf, SIM_UPDATE_RECORD_CNF ),
  MAK_FUNC_N( pei_not_supported,        0                     ),
  MAK_FUNC_N( pei_not_supported,        0                     ),
  MAK_FUNC_N( pei_not_supported,        0                     ),
  MAK_FUNC_N( pei_not_supported,        SIM_INCREMENT_CNF     ),
  MAK_FUNC_N( pei_not_supported,        SIM_VERIFY_PIN_CNF    ),
  MAK_FUNC_N( pei_not_supported,        SIM_CHANGE_PIN_CNF    ),
  MAK_FUNC_N( pei_not_supported,        SIM_DISABLE_PIN_CNF   ),
  MAK_FUNC_N( pei_not_supported,        SIM_ENABLE_PIN_CNF    ),
  MAK_FUNC_N( pei_not_supported,        SIM_UNBLOCK_CNF       ),
  MAK_FUNC_N( pei_not_supported,        SIM_AUTHENTICATION_CNF),
  MAK_FUNC_N( pei_not_supported,        SIM_MMI_INSERT_IND    ),
  MAK_FUNC_N( pei_not_supported,        SIM_MM_INSERT_IND     ),
  MAK_FUNC_0( tl_sim_remove_ind,        SIM_REMOVE_IND        ),
  MAK_FUNC_N( pei_not_supported,        SIM_SYNC_CNF          ),
  MAK_FUNC_N( pei_not_supported,        SIM_ACTIVATE_CNF      ),
  MAK_FUNC_0( tl_sim_sms_insert_ind,    SIM_SMS_INSERT_IND    ),
#if defined(SIM_TOOLKIT) AND defined(SAT_SMS_DNL_SUPPORT)
  MAK_FUNC_N( pei_not_supported,        SIM_TOOLKIT_IND       ),
  MAK_FUNC_0( tl_sim_toolkit_cnf,       SIM_TOOLKIT_CNF       ),
#else
  MAK_FUNC_N( pei_not_supported,        SIM_TOOLKIT_IND       ),
  MAK_FUNC_N( pei_not_supported,        SIM_TOOLKIT_CNF       ),
#endif /* else, #if defined(SIM_TOOLKIT) AND defined(SAT_SMS_DNL_SUPPORT) */
  MAK_FUNC_N( pei_not_supported,        0                     ),
  MAK_FUNC_N( pei_not_supported,        0                     ),
  MAK_FUNC_N( pei_not_supported,        0                     ),
#ifdef SIM_TOOLKIT
  MAK_FUNC_0( tl_sim_file_update_ind,   SIM_FILE_UPDATE_IND   )
#else
  MAK_FUNC_N( pei_not_supported,        SIM_FILE_UPDATE_IND   )
#endif /* else, #ifdef SIM_TOOLKIT */
};

#ifdef GPRS
LOCAL const T_FUNC ll_table[] = {
  MAK_FUNC_N( pei_not_supported,        0                     ),
  MAK_FUNC_N( pei_not_supported,        0                     ),
  MAK_FUNC_N( pei_not_supported,        0                     ),
  MAK_FUNC_N( pei_not_supported,        0                     ),
  MAK_FUNC_N( pei_not_supported,        0                     ),
  MAK_FUNC_N( pei_not_supported,        0                     ),
  MAK_FUNC_N( pei_not_supported,        0                     ),
  MAK_FUNC_N( pei_not_supported,        0                     ),
  MAK_FUNC_0( for_ll_unitready_ind,     LL_UNITREADY_IND      ),
  MAK_FUNC_N( pei_not_supported,        0                     ),
  MAK_FUNC_N( pei_not_supported,        0                     ),
  MAK_FUNC_0( for_ll_unitdata_ind,      LL_UNITDATA_IND       )
};

LOCAL const T_FUNC gmmsms_table[] = {
  MAK_FUNC_0( cp_gmmsms_reg_state_cnf,  GMMSMS_REG_STATE_CNF )
};
#endif /* #ifdef GPRS */

#ifdef FF_EM_MODE
LOCAL const T_FUNC em_table[] = {
 MAK_FUNC_N (pei_not_supported   , 0               ), /* 0x00 */
 MAK_FUNC_N (pei_not_supported   , 0               ), /* 0x01 */
 MAK_FUNC_N (pei_not_supported   , 0               ), /* 0x02 */
 MAK_FUNC_N (pei_not_supported   , 0               ), /* 0x03 */
 MAK_FUNC_N (pei_not_supported   , 0               ), /* 0x04 */
 MAK_FUNC_N (pei_not_supported   , 0               ), /* 0x05 */
 MAK_FUNC_N (pei_not_supported   , 0               ), /* 0x06 */
 MAK_FUNC_N (pei_not_supported   , 0               ), /* 0x07 */
 MAK_FUNC_N (pei_not_supported   , 0               ), /* 0x08 */
 MAK_FUNC_N (pei_not_supported   , 0               ), /* 0x09 */
 MAK_FUNC_N (pei_not_supported   , 0               ), /* 0x0A */
 MAK_FUNC_N (pei_not_supported   , 0               ), /* 0x0B */
 MAK_FUNC_N (pei_not_supported   , 0               ), /* 0x0C */
 MAK_FUNC_N (pei_not_supported   , 0               ), /* 0x0D */
 MAK_FUNC_N (pei_not_supported   , 0               ), /* 0x0E */
 MAK_FUNC_0 (sms_em_sms_event_req, EM_SMS_EVENT_REQ)  /* 0x0F */
};
#endif /* #ifdef FF_EM_MODE */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)              MODULE  : SMS_PEI             |
| STATE   : code                       ROUTINE : pei_primitive       |
+--------------------------------------------------------------------+

  PURPOSE : Process protocol specific primitive.

*/

LOCAL SHORT pei_primitive (void * ptr)
{
  T_PRIM *prim = ptr;

  /*
   *            |          |     |
   *          MNSMS       SIM    EM         UPLINK
   *            |          |     |
   *      +-----v----------v-----v----+
   *      |                           |
   *      |            SMS            |
   *      |                           |
   *      +-------------^-------------+
   *                    |
   *                MMSMS/LLC               DOWNLINK
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
      case SAP_NR(MNSMS_UL): table =  mnsms_table; n = TAB_SIZE ( mnsms_table); break;
      case SAP_NR(MMSMS_DL): table =  mmsms_table; n = TAB_SIZE ( mmsms_table); break;
      case SAP_NR(SIM_UL):   table =    sim_table; n = TAB_SIZE (   sim_table); break;
#ifdef GPRS
      case LL_DL:            table =     ll_table; n = TAB_SIZE (    ll_table); break;
      case GMMSMS_DL:        table = gmmsms_table; n = TAB_SIZE (gmmsms_table); break;
#endif /* #ifdef GPRS */
#ifdef FF_EM_MODE
      case EM_Ul:            table =     em_table; n = TAB_SIZE (em_table)    ; break;
#endif /* #ifdef FF_EM_MODE */
      default :              table =         NULL; n = 0;                       break;
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
#endif /* else, #ifdef GSM_ONLY */
  }
  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)              MODULE  : SMS_PEI             |
| STATE   : code                       ROUTINE : pei_not_supported   |
+--------------------------------------------------------------------+

  PURPOSE : An unsupported primitive is received.

*/

static void pei_not_supported (void *data)
{
  TRACE_FUNCTION ("pei_not_supported()");

  PFREE (data);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)              MODULE  : SMS_PEI             |
| STATE   : code                       ROUTINE : pei_init            |
+--------------------------------------------------------------------+

  PURPOSE : Initialize Protocol Stack Entity

*/

LOCAL SHORT pei_init (T_HANDLE handle)
{
  sms_handle = handle;

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

  if (hCommSIM < VSI_OK)
  {
    if ((hCommSIM = vsi_c_open (VSI_CALLER SIM_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif /* TI_PS_HCOMM_CHANGE */
#if defined (GPRS)
  /*
   * Open LLC for GPRS GSMS handling
   */
  if (hCommLLC < VSI_OK)
  {
    if ((hCommLLC = vsi_c_open (VSI_CALLER LLC_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
  /*
   * Open GMM for GPRS registration inquiry
   */
  if (hCommGMM < VSI_OK)
  {
    if ((hCommGMM = vsi_c_open (VSI_CALLER GMM_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif /* GPRS */

  /*
   * Initialize data base
   */
  cp_init ();
  rl_init ();
  tl_init ();
  for_init_sms ();
  pcm_Init ();

  ccd_init ();

#ifdef FF_EM_MODE
  em_init_sms_event_trace();
#endif /* #ifdef FF_EM_MODE */

  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)              MODULE  : SMS_PEI             |
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
| PROJECT : GSM-PS (8410)              MODULE  : SMS_PEI             |
| STATE   : code                       ROUTINE : pei_exit            |
+--------------------------------------------------------------------+

  PURPOSE : Close Resources and terminate

*/

LOCAL SHORT pei_exit (void)
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

  vsi_c_close (VSI_CALLER hCommSIM);
  hCommSIM = VSI_ERROR;
#endif /* TI_PS_HCOMM_CHANGE */
#ifdef GPRS
  vsi_c_close (VSI_CALLER hCommLLC);
  hCommLLC = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommGMM);
  hCommGMM = VSI_ERROR;
#endif /* #ifdef GPRS */

  return PEI_OK;
}
#endif /* _SIMULATION_ */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)              MODULE  : SMS_PEI             |
| STATE   : code                       ROUTINE : pei_config          |
+--------------------------------------------------------------------+

  PURPOSE : Dynamic Configuration

*/

#ifndef NCONFIG
LOCAL const KW_DATA kwtab[] = {
   SMS_CONF_STRING_PAUSE,            SMS_CONF_EVENT_PAUSE,
   SMS_CONF_STRING_RESUME,           SMS_CONF_EVENT_RESUME,
                   "",                   0
                  };
#endif /* #ifndef NCONFIG */

#if !defined(NTRACE) && !defined(NCONFIG)
GLOBAL const KW_DATA partab[] = {
                   TC1M_NAME,  TC1M,
                   TR1M_NAME,  TR1M,
                   TR2M_NAME,  TR2M,
                   TRAM_NAME,  TRAM,
                   TLCT_NAME,  TLCT,
                   TMMS_NAME,  TMMS,
                   "",         0
                  };
#endif /* #if !defined(NTRACE) && !defined(NCONFIG) */

/* Implements Measure#36 */
#ifndef NCONFIG
LOCAL SHORT pei_config (T_PEI_CONFIG inString)
{
#ifndef NCONFIG
  char    * s = inString;
  char    * keyw;
  char    * val [10];

  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("pei_config()");

  TRACE_FUNCTION (s);

  tok_init(s);

  /*
   * Parse next keyword and number of variables
   */
  while ((tok_next(&keyw,val)) NEQ TOK_EOCS)
  {
    switch ((tok_key((KW_DATA *)kwtab,keyw)))
    {
      case TOK_NOT_FOUND:
        TRACE_ERROR ("[PEI_CONFIG]: Illegal Keyword");
        break;

      case SMS_CONF_EVENT_PAUSE:
         TRACE_EVENT("SMS_CONF_EVENT_PAUSE");
         tl_pause();
         break;
      case SMS_CONF_EVENT_RESUME:
         TRACE_EVENT("SMS_CONF_EVENT_RESUME");
         /* tl_resume(sms_data); */

         GET_MO_INSTANCE(sms_data);
        /*
         * TL  state transition TL_ESTABLISH
         * EST state transition EST_SMMA
         * MMI state transition MMI_RESUME
         * 
         */
         SET_STATE (STATE_MMI, MMI_RESUME);
         SET_STATE (STATE_EST, EST_SMMA);
         SMS_INST_SET_STATE (STATE_TL, TL_ESTABLISH);
        /*
         * 1st shot
         */
         SMS_INST.retrans  = FALSE;
        /*
         * establish connection
         */
         tl_establish_connection(FALSE);
         break;
      default:
        break;
    }
  }
#endif /* #ifndef NCONFIG */

  return PEI_OK;
}
#endif /*  NCONFIG */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : SMS_PEI             |
| STATE   : code                       ROUTINE : sms_pei_config      |
+--------------------------------------------------------------------+

  PURPOSE : Dynamic Configuration

*/
/* Implements Measure#36 */
#ifndef NCONFIG
GLOBAL SHORT sms_pei_config ( char * inString, char * dummy )
{
  pei_config ( inString );

  return PEI_OK;
}
#endif /* NCONFIG */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)              MODULE  : SMS_PEI             |
| STATE   : code                       ROUTINE : pei_monitor         |
+--------------------------------------------------------------------+

  PURPOSE : Monitoring of physical Parameters

*/

LOCAL SHORT pei_monitor (void ** monitor)
{
  TRACE_FUNCTION ("pei_monitor()");

/* Implements Measure#32: Row 20 */

  * monitor = &sms_mon;

  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)              MODULE  : SMS_PEI             |
| STATE   : code                       ROUTINE : pei_create          |
+--------------------------------------------------------------------+

  PURPOSE : Create the Protocol Stack Entity

*/

GLOBAL SHORT sms_pei_create (T_PEI_INFO **info)
{
  static const T_PEI_INFO pei_info =
  {
    "SMS",
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
#ifdef NCONFIG
      NULL,             /* no pei_config function     */
#else /* not NCONFIG */
      pei_config,
#endif /* NCONFIG */
      pei_monitor,
    },
    #if defined (GPRS)
    1792,                         /* Stack Size      */
    #else
    #if defined (FAX_AND_DATA)
    1792,                         /* Stack Size      */
    #else
    1500,                         /* Stacksize       */
    #endif
    #endif
    10,                           /* Queue Entries   */
    135,                          /* Priority        */
    MAX_SMS_TIMER*MAX_SMS_CALLS,  /* number of timer */
    0x03|PRIM_NO_SUSPEND          /* flags           */
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

#endif /* #ifndef SMS_PEI_C */
