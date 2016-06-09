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
|             for the entity DL of the mobile station.
+-----------------------------------------------------------------------------
*/

#ifndef DL_PEI_C
#define DL_PEI_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_DL

/*==== INCLUDES ===================================================*/
#include "typedefs.h"
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "vsi.h"
#include "pconst.cdg"
#include "custom.h"
#include "gsm.h"
#include "mon_dl.h"
#include "prim.h"
#include "pei.h"
#include "tok.h"
#include "ccdapi.h"
#include "dl.h"
#include "dl_em.h"
#include "dl_trc.h"

/*==== EXPORT =====================================================*/
#ifdef TI_PS_HCOMM_CHANGE
#else
GLOBAL T_HANDLE hCommDL         = VSI_ERROR;    /* Self  Communication */
GLOBAL T_HANDLE hCommRR         = VSI_ERROR;    /* RR  Communication */
GLOBAL T_HANDLE hCommPL         = VSI_ERROR;    /* PL  Communication */
#ifdef FF_EM_MODE
GLOBAL T_HANDLE hCommMMI        = VSI_ERROR;    /* EM  Communication */
#endif /* FF_EM_MODE */
#endif
GLOBAL USHORT dl_handle;

/*==== PRIVATE ====================================================*/

LOCAL void pei_not_supported (void *data);
LOCAL int dl_process_signal (ULONG opc, void *signal_data);

LOCAL SHORT pei_signal (T_SIGNAL_OPC opc, void *signal_data);

/*==== VARIABLES ==================================================*/
#ifdef _SIMULATION_
static UBYTE             first_access = TRUE;
#endif /* _SIMULATION_ */

static T_MONITOR         dl_mon;

/*==== FUNCTIONS ==================================================*/
/* Please consider that the opc's are not necessarily sorted
 * in sequential order.
 * DL doesn't use the normal technique to get the primitive from the table.
 * Instead it uses the direct search and compare:
 * for (i=0; i<n; i++, table++)
 *  if (table->opc EQ opc)
 *    break;
 */
LOCAL const T_FUNC dl_table[] = {
  MAK_FUNC_S( drr_dl_establish_req      , DL_ESTABLISH_REQ     ), /* 0x80000003 */
  MAK_FUNC_N( pei_not_supported         , 0                    ), /* 0x80010003 */
  MAK_FUNC_0( drr_dl_release_req        , DL_RELEASE_REQ       ), /* 0x80020003 */
  MAK_FUNC_N( pei_not_supported         , 0                    ), /* 0x80030003 */
  MAK_FUNC_S( drr_dl_data_req           , DL_DATA_REQ          ), /* 0x80040003 */
  MAK_FUNC_N( pei_not_supported         , 0                    ), /* 0x80050003 */
  MAK_FUNC_S( drr_dl_unitdata_req       , DL_UNITDATA_REQ      ), /* 0x80060003 */
  MAK_FUNC_N( pei_not_supported         , 0                    ), /* 0x80070003 */
  MAK_FUNC_0( drr_dl_suspend_req        , DL_SUSPEND_REQ       ), /* 0x80080003 */
  MAK_FUNC_N( pei_not_supported         , 0                    ), /* 0x80090003 */
  MAK_FUNC_S( drr_dl_resume_req         , DL_RESUME_REQ        ), /* 0x800a0003 */
  MAK_FUNC_N( pei_not_supported         , 0                    ), /* 0x800b0003 */
  MAK_FUNC_S( drr_dl_reconnect_req      , DL_RECONNECT_REQ     ), /* 0x800c0003 */
  MAK_FUNC_N( pei_not_supported         , 0                    ), /* 0x800d0003 */
#if defined (DL_TRACE_ENABLED) && !defined(DL_IMMEDIATE_TRACE)
  MAK_FUNC_0( dl_trace_read_all         , DL_TRACE_REQ         ), /* 0x800e0003 */
#else
  MAK_FUNC_N( pei_not_supported         , 0                    ), /* 0x800e0003 */
#endif /* DL_TRACE_ENABLED && !DL_IMMEDIATE_TRACE */
  MAK_FUNC_N( pei_not_supported         , 0                    ), /* 0x800f0003 */
  MAK_FUNC_S( drr_dl_short_unitdata_req , DL_SHORT_UNITDATA_REQ)  /* 0x80100003 */
};

LOCAL const T_FUNC mphc_table[] = {
  MAK_FUNC_0( dph_ph_data_ind, PH_DATA_IND )                      /* 0x0000006d */
};


#if defined (DL_2TO1)
  #if defined(_SIMULATION_)
    LOCAL const T_FUNC l1_test_table[] = {
      MAK_FUNC_0( l1test_call_mphc_read_dcch, L1TEST_CALL_MPHC_READ_DCCH    ), /* 8000409b */
      MAK_FUNC_N( pei_not_supported         , 0                             )  /* removed */
      MAK_FUNC_N( pei_not_supported         , 0                             )  /* removed */
    };
  #endif  /* _SIMULATION_ */
#else /* DL_2TO1 */

  LOCAL const T_FUNC mdl_table[] = {
    MAK_FUNC_0( drr_mdl_release_req     , MDL_RELEASE_REQ      ) /* 0x80004004 */
  };

  #if defined (_SIMULATION_) || (defined (DL_TRACE_ENABLED) && !defined(DL_IMMEDIATE_TRACE))
    LOCAL const T_FUNC ph_table[] = {
    #if defined(_SIMULATION_)
        MAK_FUNC_0( dph_ph_ready_to_send    , PH_READY_TO_SEND     ), /* 0x4100 */
    #else
        MAK_FUNC_N( pei_not_supported       , 0                    ), /* 0x4100 */
    #endif  /* _SIMULATION_ */
        MAK_FUNC_N( pei_not_supported       , 0                    ), /* 0x4101 */
    #if defined (DL_TRACE_ENABLED) && !defined(DL_IMMEDIATE_TRACE)
        MAK_FUNC_0( dl_trace_read           , PH_TRACE_IND         )  /* 0x4102 */
    #else
        MAK_FUNC_N( pei_not_supported       , 0                    )  /* 0x4102 */
    #endif /* DL_TRACE_ENABLED && !DL_IMMEDIATE_TRACE*/
    };
  #endif  /* _SIMULATION_||(DL_TRACE_ENABLED&& !DL_IMMEDIATE_TRACE) */
#endif /* DL_2TO1 */

#ifdef FF_EM_MODE
LOCAL const T_FUNC em_ul_table[] = {
  MAK_FUNC_0( dl_em_dl_event_req      , EM_DL_EVENT_REQ      ), /* 0x3e0A*/
};
#endif /* FF_EM_MODE */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : DL_PEI              |
| STATE   : code                       ROUTINE : pei_primitive       |
+--------------------------------------------------------------------+

  PURPOSE : Process protocol specific primitive.

*/
LOCAL SHORT pei_primitive (void * ptr)
{
  GET_INSTANCE_DATA;
  T_PRIM *prim = (T_PRIM*)ptr;
  /*
   *             |            |
   *            DL           MDL
   *             |            |
   *      +------v------------v------+
   *      |                          |
   *      |            DL            |
   *      |                          |
   *      +-------------^------------+
   *                    |
   *                 MPHC/PH
   *                    |
   *
   */

  TRACE_FUNCTION ("pei_primitive()");

  if (prim NEQ NULL)
  {
    ULONG           opc = prim->custom.opc;
    USHORT          n;
    const T_FUNC    *table;

    VSI_PPM_REC ((T_PRIM_HEADER*)prim, __FILE__, __LINE__);

    PTRACE_IN (opc);

    switch (SAP_NR(opc))
    {
      case SAP_NR(DL_UL):   table =  dl_table; n = TAB_SIZE (dl_table); break;
      /*case SAP_NR(MPHC_UL):*/ /* same as MPHC_DL */
      case SAP_NR(MPHC_DL):   
        table =  mphc_table; 
        n = TAB_SIZE (mphc_table); 
        /*
         * The opcodes for MPHC_DL start at 0x6d
         */
        opc -= 0x6d; 
        break;
#if defined (DL_2TO1)
  #if defined(_SIMULATION_)
      case SAP_NR(L1TEST_UL):
      case SAP_NR(L1TEST_DL):   table =  l1_test_table; n = TAB_SIZE (l1_test_table); break;
  #endif /* _SIMULATION_ */
#else /* DL_2TO1 */
      case SAP_NR(MDL_DL):  table = mdl_table; n = TAB_SIZE (mdl_table); break;
  #if defined (_SIMULATION_) || (defined (DL_TRACE_ENABLED) && !defined(DL_IMMEDIATE_TRACE))
      case SAP_NR(PH_UL):
      case SAP_NR(PH_DL):   table =  ph_table; n = TAB_SIZE (ph_table); break;
  #endif  /* _SIMULATION_||(DL_TRACE_ENABLED&&!DL_IMMEDIATE_TRACE) */
#endif /* DL_2TO1 */

#ifdef FF_EM_MODE
      case EM_Ul:           
        table = em_ul_table; 
        n = TAB_SIZE (em_ul_table); 
        /*
         * The opcodes for EM_Ul start at 0x0a
         */        
        opc -= 0x0a; 
        break;
#endif /* FF_EM_MODE*/
      default    : table = NULL;      n = 0;                      break;
    }
    if (table)
    {

#if defined (_SIMULATION_)
      if (opc NEQ PH_READY_TO_SEND)
      /*
       * this simulates function call of layer 1 in the
       * windows environment
       */
#endif /* _SIMULATION_ */
      dl_data->dl_active = TRUE;


      if (PRIM_NR(opc)<n)
      {
        table += PRIM_NR(opc);
#ifdef PALLOC_TRANSITION
        /*lint -e661 (Warning -- Possible access of out-of-bounds) */
        P_SDU(prim) = table->soff ? (T_sdu*) (((char*)&prim->data) + table->soff) : 0;
        /*lint +e661 (Warning -- Possible access of out-of-bounds) */
#ifndef NO_COPY_ROUTING
        P_LEN(prim) = table->size + sizeof (T_PRIM_HEADER);
#endif /* NO_COPY_ROUTING */
#endif /* PALLOC_TRANSITION */

#if defined (DL_TRACE_ENABLED) && !defined(DL_2TO1)
        if ((opc EQ DL_TRACE_REQ) OR (opc EQ PH_TRACE_IND))
          /*lint -e661 (Warning -- Possible access of out-of-bounds) */
          JUMP (table->func)();
          /*lint +e661 (Warning -- Possible access of out-of-bounds) */
        else
#endif /* DL_TRACE_ENABLED && !DL_2TO1 */
          /*lint -e661 (Warning -- Possible access of out-of-bounds) */
          JUMP (table->func) (P2D(prim));
          /*lint +e661 (Warning -- Possible access of out-of-bounds) */
      }
      else
      {
        pei_not_supported (P2D(prim));
      }
      dl_data->dl_active = FALSE;

      return PEI_OK;
    }

#if defined (_SIMULATION_)
    /* TDC is not able to send signals, therefore the redirection over primitives */
    if (opc EQ DL_SIGNAL_EM_READ)
    { /* test ACI read of EM buffer only */
      return dl_process_signal(DL_SIGNAL_EM_READ, NULL);
    }
#endif /* _SIMULATION_ */

    /*
     * Primitive is no GSM Primitive
     * then forward to the environment
     */

#ifdef GSM_ONLY
    MY_PFREE (P2D(prim))

    return PEI_ERROR;
#else
    if (opc & SYS_MASK)
      vsi_c_primitive (VSI_CALLER prim);
    else
    {
      void* p = (void*)P2D(prim);
      MY_PFREE (p);
      return PEI_ERROR;
    }
#endif
  }
  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : DL_PEI              |
| STATE   : code                       ROUTINE : pei_not_supported   |
+--------------------------------------------------------------------+

  PURPOSE : An unsupported primitive is received.

*/

LOCAL void pei_not_supported (void * data)
{
  TRACE_FUNCTION ("pei_not_supported()");

  MY_PFREE (data);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : DL_PEI              |
| STATE   : code                       ROUTINE : pei_init            |
+--------------------------------------------------------------------+

  PURPOSE : Initialize Protocol Stack Entity

*/

LOCAL SHORT pei_init (T_HANDLE handle)
{
  dl_handle = handle;

  TRACE_FUNCTION ("pei_init() " __DATE__ " " __TIME__);

#ifdef TI_PS_HCOMM_CHANGE
  if (!cl_hcom_all_handles_open())
  {
    return PEI_ERROR;
  }
#else /* for hCommHandles backward compatibility */
  if (hCommDL < VSI_OK)
  {
    if ((hCommDL = vsi_c_open (VSI_CALLER DL_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

  if (hCommRR < VSI_OK)
  {
    if ((hCommRR = vsi_c_open (VSI_CALLER RR_NAME)) < VSI_OK)
      return PEI_ERROR;
  }


  if (hCommPL < VSI_OK)
  {
    if ((hCommPL = vsi_c_open (VSI_CALLER PL_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

#ifdef FF_EM_MODE
  if (hCommMMI < VSI_OK)
  {
    if ((hCommMMI = vsi_c_open (VSI_CALLER ACI_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif /* FF_EM_MODE */
#endif

  TRC_INIT ();

#if !defined(_SIMULATION_)
  TRACE_EVENT (dl_version());
  SYST_TRACE (dl_version());
#endif  /* !_SIMULATION_ */

  /*
   * Initialize data base
   */

#if defined(DL_TRACE_ENABLED) && !defined(DL_IMMEDIATE_TRACE)
  dl_trace_init ();
#endif  /* DL_TRACE_ENABLED && !DL_IMMEDIATE_TRACE*/

  com_init_data ();
  dcch0_init_dl_data ();
  sacch0_init_dl_data ();

#ifdef FF_EM_MODE
  /*
   initialise event flags
  */
  em_init_dl_event_trace();
  /*
   initialise dl semaphor for EM event tracing
  */
  em_dl_sem_init();
#endif /* FF_EM_MODE */

  return PEI_OK;
}

#ifdef _SIMULATION_
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : DL_PEI              |
| STATE   : code                       ROUTINE : pei_exit            |
+--------------------------------------------------------------------+

  PURPOSE : Close Resources and terminate

*/
LOCAL SHORT pei_exit (void)
{
  TRACE_FUNCTION ("pei_exit()");

#if defined(DL_TRACE_ENABLED) && !defined(DL_IMMEDIATE_TRACE)
  dl_trace_exit ();
#endif  /* DL_TRACE_ENABLED && !DL_IMMEDIATE_TRACE*/

#ifdef FF_EM_MODE
  em_dl_sem_exit();
#endif /* FF_EM_MODE */

  /*
   * clean up communication
   */
#ifdef TI_PS_HCOMM_CHANGE
#else /* for hCommHandles backward compatibility */
  vsi_c_close (VSI_CALLER hCommDL);
  hCommDL = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommRR);
  hCommRR = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommPL);
  hCommPL = VSI_ERROR;

#ifdef FF_EM_MODE
  vsi_c_close (VSI_CALLER hCommMMI);
  hCommMMI = VSI_ERROR;
#endif /* FF_EM_MODE */
#endif

  TRC_EXIT ();

  return PEI_OK;
}
#endif /* _SIMULATION_ */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : DL_PEI              |
| STATE   : code                       ROUTINE : pei_config          |
+--------------------------------------------------------------------+

  PURPOSE : Dynamic Configuration

*/

/* Implements Measure#36 */
#ifdef NCONFIG
#else /* NCONFIG */
LOCAL SHORT pei_config (T_PEI_CONFIG inString)
{
  return PEI_OK;
}
#endif /* NCONFIG */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : DL_PEI              |
| STATE   : code                       ROUTINE : dl_pei_config       |
+--------------------------------------------------------------------+

  PURPOSE : Dynamic Configuration

*/
/* Implements Measure#36 */
#ifdef NCONFIG
#else /* NCONFIG */
GLOBAL T_PEI_RETURN dl_pei_config ( char * inString, char * dummy )
{
  return pei_config ( inString );
}
#endif /* NCONFIG */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : DL_PEI              |
| STATE   : code                       ROUTINE : pei_monitor         |
+--------------------------------------------------------------------+

  PURPOSE : Monitoring of physical Parameters

*/

LOCAL SHORT pei_monitor (void ** monitor)
{
  TRACE_FUNCTION ("pei_monitor()");

/* Implements Measure#32: Row 50 */

  * monitor = &dl_mon;

  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : DL_PEI              |
| STATE   : code                       ROUTINE : pei_create          |
+--------------------------------------------------------------------+

  PURPOSE : Create the Protocol Stack Entity

*/

/*
 * New Frame
 */
GLOBAL T_PEI_RETURN dl_pei_create (T_PEI_INFO **info)
{
  static const T_PEI_INFO pei_info =
  {
    "DL",
    {
      pei_init,
#ifdef _SIMULATION_
      pei_exit,
#else
      NULL,
#endif
      pei_primitive,
      NULL,             /* no timeout function */
      pei_signal,       /* signal function     */
      NULL,             /* no run function     */
/* Implements Measure#36 */
#ifdef NCONFIG
      NULL,             /* no pei_config function     */
#else /* NCONFIG */
      pei_config,
#endif /* NCONFIG */
      pei_monitor,
    },
    #if defined (GPRS)
    1536,                             /* Stack Size incresed for omaps00140330      */
    #else
    #if defined (FAX_AND_DATA)
    1536,                             /* Stack Size incresed for omaps00140330      */
    #else
    1394,                            /* Stack Size incresed for omaps00140330      */
    #endif
    #endif
    10+DL_SIGNAL_DATA_ELEMENTS,     /* Queue Entries   */
    215,                            /* Priority        */
    0,                              /* number of timer */
                                    /* flags           */
#ifdef _TARGET_
#ifdef GPRS
    PASSIVE_BODY|COPY_BY_REF|TRC_NO_SUSPEND|PRIM_NO_SUSPEND
#else
    PASSIVE_BODY|COPY_BY_REF|TRC_NO_SUSPEND|PRIM_NO_SUSPEND
#endif
#else
    PASSIVE_BODY|COPY_BY_REF
#endif

  };

  TRACE_FUNCTION ("pei_create");
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
   * export startup configuration data
   */
  *info = (T_PEI_INFO *)&pei_info;
  /*
   *  Initialize entity data
   */

  /*
   *  Initialize Condat Coder Decoder and
   *  processes
   */


  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : DL_PEI              |
| STATE   : code                       ROUTINE : pei_signal          |
+--------------------------------------------------------------------+

  PURPOSE : Functional interface to signal a primitive.

*/
LOCAL SHORT pei_signal (T_SIGNAL_OPC opc, void *signal_data)
{
#ifdef TRACE_FKT
  {
    char buf[24];
    sprintf(buf, "pei_signal (0x%lx)", (ULONG)opc);
    TRACE_FUNCTION (buf);
    PTRACE_IN(opc);
  }
#elif defined(_SIMULATION_)
  TRACE_EVENT_WIN_P1 ("pei_signal (0x%lx)", (ULONG)opc);
#endif

  dl_process_signal ((ULONG)opc, signal_data);

  return PEI_OK;
}

LOCAL int dl_process_signal (ULONG opc, void *signal_data)
{
  /*TRACE_FUNCTION ("pei_process_signal()");*/
  GET_INSTANCE_DATA;
  switch (opc)
  {
#if defined(DL_TRACE_ENABLED) && !defined (DL_IMMEDIATE_TRACE) && !defined(DL_2TO1)
  case DL_TRACE_REQ:  /* RR -> DL */
    SYST_TRACE("call dl_trace_read_all()");
    dl_trace_read_all ();
    break;
  case PH_TRACE_IND:  /* ALR/TIL -> DL */
    dl_trace_read ();
    break;
#endif  /* DL_TRACE_ENABLED && !DL_IMMEDIATE_TRACE &6 !DL_2TO1 */
#if defined(INVOKE_SIGNAL)
  case DL_SIGNAL_ESTABLISH_IND:
    sig_handle_drr_dl_establish_ind ((T_DL_SIGNAL_DATA *) signal_data);
    break;
  case DL_SIGNAL_ESTABLISH_CNF:
    sig_handle_drr_dl_establish_cnf ((T_DL_SIGNAL_DATA *) signal_data);
    break;
  case DL_SIGNAL_DATA_IND:
    sig_handle_com_data_ind ((T_DL_SIGNAL_DATA *) signal_data);
    break;
  case DL_SIGNAL_DATA_CNF:
    sig_handle_drr_dl_data_cnf ((T_DL_SIGNAL_DATA *) signal_data);
    break;
#if 0 /* happens in primitive context only */
  case DL_SIGNAL_UNITDATA_IND:
    sig_handle_drr_dl_unitdata_ind (dl_data, (T_DL_SIGNAL_DATA *) signal_data);
    break;
#endif  /* 0 */
  case DL_SIGNAL_SHORT_UNITDATA_IND:
    sig_handle_drr_dl_short_unitdata_ind ((T_DL_SIGNAL_DATA *) signal_data);
    break;
  case DL_SIGNAL_RELEASE_IND:
    sig_handle_drr_dl_release_ind ((T_DL_SIGNAL_DATA *) signal_data);
    break;
  case DL_SIGNAL_RELEASE_CNF:
    sig_handle_drr_dl_release_cnf ((T_DL_SIGNAL_DATA *) signal_data);
    break;
  case DL_SIGNAL_ERROR_IND:
    sig_handle_drr_error_ind ((T_DL_SIGNAL_DATA *) signal_data);
    break;
  case DL_SIGNAL_FREE_POINTER:
    sig_handle_com_free_pointer ((T_DL_SIGNAL_DATA *) signal_data);
    break;
  case DL_SIGNAL_CONCATENATE:
    sig_handle_com_concatenate((T_DL_SIGNAL_DATA *) signal_data);
    break;
#if defined(DL_TRACE_ENABLED)
  case DL_SIGNAL_L2TRACE:
    sig_handle_com_l2trace ((T_DL_SIGNAL_DATA *) signal_data);
    break;
#endif /*DL_TRACE_ENABLED*/
  case DL_SIGNAL_L3TRACE:
    sig_handle_com_l3trace((T_DL_SIGNAL_DATA *) signal_data);
    break;
#if defined(FF_EM_MODE)
  case DL_SIGNAL_EM_IND:
    sig_handle_dl_em_first_event_check ();
    break;
  case DL_SIGNAL_EM_WRITE:
    sig_handle_dl_em_write (dl_data, (T_DL_SIGNAL_DATA *) signal_data);
    break;
#if defined(_SIMULATION_)
  case DL_SIGNAL_EM_READ:
    {
      T_MONITOR *dummy;

      em_dl_sem_read ();
      em_dl_sem_reset ();
      pei_monitor (&dummy);
      TRACE_EVENT_WIN (dummy->version);
    }
    break;
#endif  /* _SIMULATION_ */
#endif  /* FF_EM_MODE */
#endif /* INVOKE_SIGNAL */
  default:
    break;
  }

#if defined(INVOKE_SIGNAL)
  if (signal_data)
  {
    TRACE_EVENT_WIN_P1 ("pei_process_signal(): set data with idx=%u to FREE",
      ((T_DL_SIGNAL_DATA *)signal_data)->idx);

    ((T_DL_SIGNAL_DATA *)signal_data)->busy = FALSE;
  }
#endif /* INVOKE_SIGNAL */

  return PEI_OK;
}/* dl_process_signal */

GLOBAL UBYTE * dl_get_sacch_buffer (void)
{
  GET_INSTANCE_DATA;
  return dl_data->sacch_act_buffer.buf;
}

#endif  /* DL_PEI_C */
