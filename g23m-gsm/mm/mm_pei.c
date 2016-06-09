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
|             for the entity MM of the mobile station.
+-----------------------------------------------------------------------------
*/


#ifndef MM_PEI_C
#define MM_PEI_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_MM

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
#include "cnf_mm.h"
#include "mon_mm.h"
#include "pei.h"
#include "tok.h"
#include "mm.h"
#include "mm_em.h"

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
#include "cnf_mm.h"
#include "mon_mm.h"
#include "vsi.h"
#include "pei.h"
#include "tok.h"
#include "mm.h"
#include "mm_em.h"

#endif

/*==== TEST =====================================================*/

/*==== EXPORT =====================================================*/
#if defined (NEW_FRAME)
#ifdef TI_PS_HCOMM_CHANGE
#else
GLOBAL T_HANDLE  hCommMMI = VSI_ERROR; /* MMI  Communication  */
GLOBAL T_HANDLE  hCommCC  = VSI_ERROR; /* CC   Communication  */
GLOBAL T_HANDLE  hCommSS  = VSI_ERROR; /* SS   Communication  */
GLOBAL T_HANDLE  hCommSMS = VSI_ERROR; /* SMS  Communication  */
GLOBAL T_HANDLE  hCommSIM = VSI_ERROR; /* SIM  Communication  */
GLOBAL T_HANDLE  hCommDL  = VSI_ERROR; /* DL   Communication  */
GLOBAL T_HANDLE  hCommRR  = VSI_ERROR; /* RR   Communication  */
#endif /* TI_PS_HCOMM_CHANGE */
#ifdef GPRS
GLOBAL T_HANDLE  hCommGMM = VSI_ERROR; /* GMM  Communication  */
#endif
GLOBAL T_HANDLE  mm_handle;
#else
#ifdef TI_PS_HCOMM_CHANGE
#else
GLOBAL T_VSI_CHANDLE  hCommMMI = VSI_ERROR; /* MMI  Communication  */
GLOBAL T_VSI_CHANDLE  hCommCC  = VSI_ERROR; /* CC   Communication  */
GLOBAL T_VSI_CHANDLE  hCommSS  = VSI_ERROR; /* SS   Communication  */
GLOBAL T_VSI_CHANDLE  hCommSMS = VSI_ERROR; /* SMS  Communication  */
GLOBAL T_VSI_CHANDLE  hCommSIM = VSI_ERROR; /* SIM  Communication  */
GLOBAL T_VSI_CHANDLE  hCommDL  = VSI_ERROR; /* DL   Communication  */
GLOBAL T_VSI_CHANDLE  hCommRR  = VSI_ERROR; /* RR   Communication  */
#endif /* TI_PS_HCOMM_CHANGE */
GLOBAL T_VSI_THANDLE  mm_act_handle = VSI_ERROR;
#endif /* NEW_FRAME */
T_MM_DATA mm_data_base;

/*==== PRIVATE ====================================================*/

LOCAL void pei_not_supported (void *);

/*==== VARIABLES ==================================================*/

#ifdef _SIMULATION_
LOCAL BOOL              first_access = TRUE;
#endif /* _SIMULATION_ */
LOCAL T_MONITOR         mm_mon;

/*==== FUNCTIONS ==================================================*/

#ifdef  GPRS
LOCAL const T_FUNC mmgmm_table[] = {
  MAK_FUNC_0 (mm_mmgmm_reg_req       , MMGMM_REG_REQ          ), /* 0x00 */
  MAK_FUNC_0 (mm_mmgmm_nreg_req      , MMGMM_NREG_REQ         ), /* 0x01 */
  MAK_FUNC_0 (mm_mmgmm_net_req       , MMGMM_NET_REQ          ), /* 0x02 */
  MAK_FUNC_0 (mm_mmgmm_plmn_res      , MMGMM_PLMN_RES         ), /* 0x03 */
  MAK_FUNC_0 (mm_mmgmm_plmn_mode_req , MMGMM_PLMN_MODE_REQ    ), /* 0x04 */
  MAK_FUNC_0 (mm_mmgmm_auth_rej_req  , MMGMM_AUTH_REJ_REQ     ), /* 0x05 */
  MAK_FUNC_0 (mm_mmgmm_cm_establish_res,MMGMM_CM_ESTABLISH_RES), /* 0x06 */
  MAK_FUNC_0 (mm_mmgmm_attach_started_req, MMGMM_ATTACH_STARTED_REQ),
  MAK_FUNC_0 (mm_mmgmm_attach_acc_req, MMGMM_ATTACH_ACC_REQ   ), /* 0x08 */
  MAK_FUNC_0 (mm_mmgmm_attach_rej_req, MMGMM_ATTACH_REJ_REQ   ), /* 0x09 */
  MAK_FUNC_0 (mm_mmgmm_detach_started_req, MMGMM_DETACH_STARTED_REQ),
  MAK_FUNC_0 (mm_mmgmm_start_t3212_req, MMGMM_START_T3212_REQ ), /* 0x0b */
  MAK_FUNC_0 (mm_mmgmm_cm_emergency_res,MMGMM_CM_EMERGENCY_RES), /* 0x0c */
  MAK_FUNC_0 (mm_mmgmm_allowed_req   ,  MMGMM_ALLOWED_REQ     ), /* 0x0d */
  MAK_FUNC_0 (pei_not_supported      ,  MMGMM_TRIGGER_REQ     )  /* 0x0e */ 
};
#else
LOCAL const T_FUNC mmreg_table[] = {
  MAK_FUNC_0 (reg_mmr_reg_req        , MMR_REG_REQ            ),
  MAK_FUNC_0 (reg_mmr_nreg_req       , MMR_NREG_REQ           ),
  MAK_FUNC_0 (reg_mmr_net_req        , MMR_NET_REQ            ),
  MAK_FUNC_0 (reg_mmr_plmn_res       , MMR_PLMN_RES           ),
  MAK_FUNC_0 (reg_mmr_plmn_mode_req  , MMR_PLMN_MODE_REQ      )
};
#endif

LOCAL const T_FUNC mmcm_table[] = {
  MAK_FUNC_0 (mm_mmcm_establish_req  , MMCM_ESTABLISH_REQ     ), /* 0x00 */
  MAK_FUNC_S (mm_mmcm_data_req       , MMCM_DATA_REQ          ), /* 0x01 */
  MAK_FUNC_0 (mm_mmcm_release_req    , MMCM_RELEASE_REQ       ), /* 0x02 */
  MAK_FUNC_0 (mm_mmcm_reestablish_req, MMCM_REESTABLISH_REQ   ), /* 0x03 */
  MAK_FUNC_0 (mm_mmcm_prompt_res     , MMCM_PROMPT_RES        ), /* 0x04 */
  MAK_FUNC_0 (mm_mmcm_prompt_rej     , MMCM_PROMPT_REJ        )  /* 0x05 */
};

LOCAL const T_FUNC mmss_table[] = {
  MAK_FUNC_0 (mm_mmss_establish_req  , MMSS_ESTABLISH_REQ     ),
  MAK_FUNC_0 (mm_mmss_release_req    , MMSS_RELEASE_REQ       ),
  MAK_FUNC_S (mm_mmss_data_req       , MMSS_DATA_REQ          )
};

LOCAL const T_FUNC mmsms_table[] = {
  MAK_FUNC_0 (mm_mmsms_establish_req , MMSMS_ESTABLISH_REQ    ),
  MAK_FUNC_0 (mm_mmsms_release_req   , MMSMS_RELEASE_REQ      ),
  MAK_FUNC_S (mm_mmsms_data_req      , MMSMS_DATA_REQ         )
};

LOCAL const T_FUNC sim_table[] = {
  MAK_FUNC_0 (reg_sim_read_cnf       , SIM_READ_CNF           ), /* 0x00 */
  MAK_FUNC_0 (pei_not_supported      , SIM_UPDATE_CNF         ),
  MAK_FUNC_0 (pei_not_supported      , SIM_READ_RECORD_CNF    ),
  MAK_FUNC_N (pei_not_supported      , 0                      ),
  MAK_FUNC_0 (pei_not_supported      , SIM_UPDATE_RECORD_CNF  ),
  MAK_FUNC_N (pei_not_supported      , 0                      ),
  MAK_FUNC_N (pei_not_supported      , 0                      ),
  MAK_FUNC_N (pei_not_supported      , 0                      ),
  MAK_FUNC_0 (pei_not_supported      , SIM_INCREMENT_CNF      ),
  MAK_FUNC_0 (pei_not_supported      , SIM_VERIFY_PIN_CNF     ),
  MAK_FUNC_0 (pei_not_supported      , SIM_CHANGE_PIN_CNF     ),
  MAK_FUNC_0 (pei_not_supported      , SIM_DISABLE_PIN_CNF    ),
  MAK_FUNC_0 (pei_not_supported      , SIM_ENABLE_PIN_CNF     ),
  MAK_FUNC_0 (pei_not_supported      , SIM_UNBLOCK_CNF        ),
  MAK_FUNC_0 (reg_sim_auth_cnf       , SIM_AUTHENTICATION_CNF ),
  MAK_FUNC_0 (pei_not_supported      , SIM_MMI_INSERT_IND     ),
  MAK_FUNC_0 (reg_sim_mm_insert_ind  , SIM_MM_INSERT_IND      ),
  MAK_FUNC_0 (reg_sim_remove_ind     , SIM_REMOVE_IND         ),
  MAK_FUNC_0 (reg_sim_sync_cnf       , SIM_SYNC_CNF           ),
  MAK_FUNC_0 (pei_not_supported      , SIM_ACTIVATE_CNF       ),
  MAK_FUNC_0 (pei_not_supported      , SIM_SMS_INSERT_IND     ), /* 0x14 */
  MAK_FUNC_0 (pei_not_supported      , SIM_TOOLKIT_IND        ), /* 0x15 */
  MAK_FUNC_0 (pei_not_supported      , SIM_TOOLKIT_CNF        ), /* 0x16 */
  MAK_FUNC_0 (pei_not_supported      , SIM_ACTIVATE_IND       ), /* 0x17 */
  MAK_FUNC_0 (reg_sim_mm_info_ind    , SIM_MM_INFO_IND        ), /* 0x18 */
  MAK_FUNC_0 (pei_not_supported      , SIM_ACCESS_CNF         ), /* 0x19 */
  MAK_FUNC_0 (reg_sim_file_upd_ind   , SIM_FILE_UPDATE_IND    )  /* 0x1a */
};

#ifdef FF_EM_MODE
LOCAL const T_FUNC em_ul_table[] = {
  MAK_FUNC_N (pei_not_supported                  , 0                     ), /* 0x3E00 */
  MAK_FUNC_N (pei_not_supported                  , 0                     ), /* 0x3E01 */
  MAK_FUNC_N (pei_not_supported                  , 0                     ), /* 0x3E02 */
  MAK_FUNC_N (pei_not_supported                  , 0                     ), /* 0x3E03 */
  MAK_FUNC_N (pei_not_supported                  , 0                     ), /* 0x3E04 */
  MAK_FUNC_N (pei_not_supported                  , 0                     ), /* 0x3E05 */
  MAK_FUNC_N (pei_not_supported                  , 0                     ), /* 0x3E06 */
  MAK_FUNC_N (pei_not_supported                  , 0                     ), /* 0x3E07 */
  MAK_FUNC_N (pei_not_supported                  , 0                     ), /* 0x3E08 */
  MAK_FUNC_N (pei_not_supported                  , 0                     ), /* 0x3E09 */
  MAK_FUNC_0 (mm_em_dl_event_req                 , EM_DL_EVENT_REQ       ), /* 0x3E0A */
  MAK_FUNC_N (pei_not_supported                  , 0                     ), /* 0x3E0B */
  MAK_FUNC_0 (mm_em_mm_event_req                 , EM_MM_EVENT_REQ       )  /* 0x3E0C */
};

  LOCAL const T_FUNC em_dl_table[] = {
    MAK_FUNC_N (pei_not_supported                , 0                     ), /* 0x7E00 */
    MAK_FUNC_N (pei_not_supported                , 0                     ), /* 0x7E01 */
    MAK_FUNC_N (pei_not_supported                , 0                     ), /* 0x7E02 */
    MAK_FUNC_N (pei_not_supported                , 0                     ), /* 0x7E03 */
    MAK_FUNC_N (pei_not_supported                , 0                     ), /* 0x7E04 */
    MAK_FUNC_N (pei_not_supported                , 0                     ), /* 0x7E05 */
    MAK_FUNC_N (pei_not_supported                , 0                     ), /* 0x7E06 */
    MAK_FUNC_N (pei_not_supported                , 0                     ), /* 0x7E07 */
    MAK_FUNC_N (pei_not_supported                , 0                     ), /* 0x7E08 */
    MAK_FUNC_N( pei_not_supported                , 0                     ), /* 0x7E09 */
    MAK_FUNC_N( pei_not_supported                , 0                     ), /* 0x7E0A */
    MAK_FUNC_0( pei_not_supported                , EM_DATA_IND           ), /* 0x7E0B */
  };
#endif /* FF_EM_MODE */

LOCAL const T_FUNC rr_table[] = {
  MAK_FUNC_0 (mm_rr_abort_ind            , RR_ABORT_IND           ),
  MAK_FUNC_0 (mm_rr_activate_cnf         , RR_ACTIVATE_CNF        ),
  MAK_FUNC_0 (mm_rr_activate_ind         , RR_ACTIVATE_IND        ),
  MAK_FUNC_S (for_rr_data_ind            , RR_DATA_IND            ),
  MAK_FUNC_0 (mm_rr_establish_cnf        , RR_ESTABLISH_CNF       ),
  MAK_FUNC_0 (mm_rr_establish_ind        , RR_ESTABLISH_IND       ),
  MAK_FUNC_N (pei_not_supported          , 0                      ),
  MAK_FUNC_0 (mm_rr_release_ind          , RR_RELEASE_IND         ),
  MAK_FUNC_0 (mm_rr_sync_ind             , RR_SYNC_IND            )
#if defined (FF_EOTD) AND defined (REL99)
  ,
  MAK_FUNC_0 (mm_rr_rrlp_start_ind       , RR_RRLP_START_IND      ),
  MAK_FUNC_0 (mm_rr_rrlp_stop_ind        , RR_RRLP_STOP_IND       )
#else
  ,
  MAK_FUNC_N (pei_not_supported          , 0                      ),
  MAK_FUNC_N (pei_not_supported          , 0                      )
#endif /* (FF_EOTD) AND defined (REL99) */
};

LOCAL const T_FUNC mdl_table[] = {
  MAK_FUNC_0 (mm_mdl_error_ind       , MDL_ERROR_IND          )
};


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : MM_PEI              |
| STATE   : code                       ROUTINE : pei_primitive       |
+--------------------------------------------------------------------+

  PURPOSE : Process protocol specific primitive.

*/

#if defined (NEW_FRAME)
LOCAL SHORT pei_primitive (void * ptr)
#else
GLOBAL T_PEI_RETURN pei_primitive (T_PRIM * prim)
#endif
{
  GET_INSTANCE_DATA;
#if defined (NEW_FRAME)
  T_PRIM *prim = ptr;
#endif
  /*
   *          |     |     |     |     |
   *        MMREG  MMCM  MMSS  MMSMS  EM          UPLINK
   *          |     |     |     |     |
   *      +---v-----v-----v-----v-----v---+
   *      |                               |
   *      |            MM                 |
   *      |                               |
   *      +---^---------^--------^--------+
   *          |         |        |
   *         SIM        RR      MDL                DOWNLINK
   *          |         |        |
   *
   */

  TRACE_FUNCTION ("pei_primitive()");
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
      case SAP_NR(MMCM_UL):  table =   mmcm_table; n = TAB_SIZE ( mmcm_table); break;
      case SAP_NR(MMSS_UL):  table =   mmss_table; n = TAB_SIZE ( mmss_table); break;
      case SAP_NR(MMSMS_UL): table =  mmsms_table; n = TAB_SIZE (mmsms_table); break;
      case SAP_NR(SIM_UL):   table =    sim_table; n = TAB_SIZE (  sim_table); break;
#ifdef FF_EM_MODE
      case   EM_Ul:   table =  em_ul_table; n = TAB_SIZE (em_ul_table); break;
#endif /* FF_EM_MODE */
      case SAP_NR(RR_DL):    table =     rr_table; n = TAB_SIZE (   rr_table); break;
      case SAP_NR(MDL_UL):   table =    mdl_table; n = TAB_SIZE (  mdl_table); break;
#ifdef GPRS
      case  MMGMM_UL: table =  mmgmm_table; n = TAB_SIZE (mmgmm_table); break;
#else
      case SAP_NR(MMREG_UL): table =  mmreg_table; n = TAB_SIZE (mmreg_table); break;
#endif
#ifdef FF_EM_MODE
      case  EM_Dl:  table = em_dl_table;  n = TAB_SIZE (em_dl_table); break;
#endif /* FF_EM_MODE */
      default       : table =         NULL; n = 0;                      break;
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

        while (ENTITY_DATA->use_stored_entries)
        {
          ENTITY_DATA->use_stored_entries = FALSE;
          mm_use_entry();
        }

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
      PFREE (P2D(prim))
      return PEI_ERROR;
    }
#endif
  }
  return PEI_OK;
}

#if defined (NEW_FRAME)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : MM_PEI              |
| STATE   : code                       ROUTINE : mm_pei_primitive    |
+--------------------------------------------------------------------+

  PURPOSE : used to restart a stored primitive.

*/
GLOBAL void mm_pei_primitive (T_PRIM * prim)
{
  pei_primitive (prim);
}
#endif

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
| PROJECT : GSM-PS (6147)              MODULE  : MM_PEI              |
| STATE   : code                       ROUTINE : pei_init            |
+--------------------------------------------------------------------+

  PURPOSE : Initialize Protocol Stack Entity

*/

#if defined (NEW_FRAME)
LOCAL SHORT pei_init (T_HANDLE handle)
#else
GLOBAL T_PEI_RETURN pei_init (void)
#endif
{
#if defined (NEW_FRAME)
  mm_handle = handle;
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

  if (hCommCC < VSI_OK)
  {
    if ((hCommCC = vsi_c_open (VSI_CALLER CC_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

  if (hCommSS < VSI_OK)
  {
    if ((hCommSS = vsi_c_open (VSI_CALLER SS_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

  if (hCommSMS < VSI_OK)
  {
    if ((hCommSMS = vsi_c_open (VSI_CALLER SMS_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

  if (hCommRR < VSI_OK)
  {
    if ((hCommRR = vsi_c_open (VSI_CALLER RR_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

  if (hCommDL < VSI_OK)
  {
    if ((hCommDL = vsi_c_open (VSI_CALLER DL_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

  if (hCommSIM < VSI_OK)
  {
    if ((hCommSIM = vsi_c_open (VSI_CALLER SIM_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif /* TI_PS_HCOMM_CHANGE */
#ifdef GPRS
  if (hCommGMM < VSI_OK)
  {
    if ((hCommGMM = vsi_c_open (VSI_CALLER GMM_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif

  /*
   *  Open Timer Resources
   */

  mm_init_mm_data ();

#ifdef FF_EM_MODE
  em_init_mm_event_trace();
#endif /* FF_EM_MODE */

#if defined (OPTION_TIMER)
  tim_init_timer ();
#endif
  ccd_init ();

  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : MM_PEI              |
| STATE   : code                       ROUTINE : pei_timeout         |
+--------------------------------------------------------------------+

  PURPOSE : Process timeout

*/
#if defined (NEW_FRAME)
LOCAL SHORT pei_timeout (USHORT index)
{
  GET_INSTANCE_DATA;

  tim_exec_timeout (index);

  while (ENTITY_DATA->use_stored_entries)
  {
    ENTITY_DATA->use_stored_entries = FALSE;
    mm_use_entry();
  }

  return PEI_OK;
}

#else

GLOBAL T_PEI_RETURN pei_timeout (T_VSI_THANDLE handle)
{
  /*
   * Set Timeout Flag according
   * to timer handle
   */
  if (mm_act_handle NEQ handle)
    tim_set_timeout_flag (handle, &t_flag);

#ifdef OPTION_TIMEOUT_SYNC
  while (t_flag)
  {
    /*
     * Handle Timeouts
     */
    tim_handle_timeout (&t_flag);
  }
#else
  vsi_c_awake (VSI_CALLER_SINGLE);
#endif

  while (ENTITY_DATA->use_stored_entries)
  {
    ENTITY_DATA->use_stored_entries = FALSE;
    mm_use_entry();
  }

  return PEI_OK;
}
#endif

#ifdef _SIMULATION_
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : MM_PEI              |
| STATE   : code                       ROUTINE : pei_exit            |
+--------------------------------------------------------------------+

  PURPOSE : Close Resources and terminate

*/
#if defined (NEW_FRAME)
LOCAL SHORT pei_exit (void)
#else
GLOBAL T_PEI_RETURN pei_exit (void)
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

  vsi_c_close (VSI_CALLER hCommCC);
  hCommCC = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommSS);
  hCommSS = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommSMS);
  hCommSMS = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommRR);
  hCommRR = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommDL);
  hCommDL = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommSIM);
  hCommSIM = VSI_ERROR;
#endif /* TI_PS_HCOMM_CHANGE */
#ifdef GPRS
  vsi_c_close (VSI_CALLER hCommGMM);
  hCommGMM = VSI_ERROR;
#endif

  return PEI_OK;
}
#endif /* _SIMULATION_ */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : MM_PEI              |
| STATE   : code                       ROUTINE : pei_config          |
+--------------------------------------------------------------------+

  PURPOSE : Dynamic Configuration

*/

/* Implements Measure#36 */
#if !defined (NCONFIG)
LOCAL const KW_DATA kwtab[] = {
#ifdef OPTION_TIMER
                   MM_TIMER_SET,        TIMER_SET,
                   MM_TIMER_RESET,      TIMER_RESET,
                   MM_TIMER_SPEED_UP,   TIMER_SPEED_UP,
                   MM_TIMER_SLOW_DOWN,  TIMER_SLOW_DOWN,
                   MM_TIMER_SUPPRESS,   TIMER_SUPPRESS,
#endif
                   MM_T3212_CNT,        T3212_CNT,
                   MM_USE_STORED_BCCH,  USE_STORED_BCCH,
                   MM_FFS_RESET_EPLMN,  FFS_RESET_EPLMN,
                   MM_FFS_READ_EPLMN,   FFS_READ_EPLMN,
                   MM_FFS_READ_EPLMN_INIT, FFS_READ_EPLMN_INIT,
                   MM_FFS_WRITE_EPLMN,  FFS_WRITE_EPLMN,
                   "",                    0
                  };
#endif /* (NCONFIG) */

GLOBAL const KW_DATA partab[] = {
#ifdef OPTION_TIMER
                   T_REG_NAME,     T_REGISTRATION,
                   T3210_NAME,     T3210,
                   T3211_NAME,     T3211,
                   T3212_NAME,     T3212,
                   T3213_NAME,     T3213,
                   T3220_NAME,     T3220,
                   T3230_NAME,     T3230,
                   T3240_NAME,     T3240,
                   T_HPLMN_NAME,   T_HPLMN,
#ifdef REL99
                   T3241_NAME,     T3241,
#endif
#endif
                   "",                 0
                  };

/* Implements Measure#36 */
#if !defined (NCONFIG)
#if defined (NEW_FRAME)
LOCAL SHORT pei_config (T_PEI_CONFIG inString)
#else
GLOBAL T_PEI_RETURN pei_config (T_PEI_CONFIG inString,
                                T_PEI_CONFIG outString)
#endif
{
  GET_INSTANCE_DATA;
  {
    char    * s = inString;
    SHORT     valno;
    SHORT     keyno;
    char    * keyw;
    char    * val [10];
#if defined (OPTION_TIMER)
    BOOL      t_man = FALSE;
    SHORT     t_num;
    LONG      t_val;
    UBYTE     t_mod;
#endif /* #if defined (OPTION_TIMER) */

    TRACE_FUNCTION ("pei_config()");

    TRACE_EVENT (s);

#if defined TM_SPECIAL
    if (!strcmp (inString, "RESET_2"))
    {
      /*
       * Hardware Reset
       */
      csf_hardware_reset_2 ();
      return PEI_OK;
    }

    if (!strcmp (inString, "RESET_3"))
    {
      /*
       * Init RF
       */
      csf_hardware_reset_3 ();
      return PEI_OK;
    }
#endif /* #if defined TM_SPECIAL */

    tok_init(s);

    /*
     * Parse next keyword and number of variables
     */
    while ((valno = tok_next(&keyw,val)) != TOK_EOCS)
    {
      switch ((keyno = tok_key((KW_DATA *)kwtab,keyw)))
      {
        case TOK_NOT_FOUND:
          TRACE_ERROR ("[PEI_CONFIG]: Illegal Keyword");
          break;

#if defined (OPTION_TIMER)
        case TIMER_SET:
          if (valno EQ 2)
          {
            t_man = TRUE;
            t_num = tok_key((KW_DATA *)partab,val[0]);
            t_mod = TIMER_SET;
            t_val = atoi(val[1]);
            if (t_val < 0L)
              t_val = 0L;
          }
          else
          {
            TRACE_ERROR ("[PEI_CONFIG]: Wrong Number of Parameters");
          }
          break;
        case TIMER_RESET:
        case TIMER_SUPPRESS:
          if (valno == 1)
          {
            t_man = TRUE;                          /* Timermanipulation         */
            t_num = tok_key((KW_DATA *)partab,val[0]);
            t_mod = (UBYTE)keyno;
            t_val = 0L;
          }
          else
          {
            TRACE_ERROR ("[PEI_CONFIG]: Wrong Number of Parameters");
          }
          break;
        case TIMER_SPEED_UP:
        case TIMER_SLOW_DOWN:
          if (valno == 2)
          {
            t_man = TRUE;
            t_num = tok_key((KW_DATA *)partab,val[0]);
            t_mod = (UBYTE)keyno;
            t_val = atoi(val[1]);
            if (t_val <= 0L)
              t_val = 1L;
          }
          else
          {
            TRACE_ERROR ("[PEI_CONFIG]: Wrong Number of Parameters");
          }
          break;
#endif /* #if defined (OPTION_TIMER) */
        case T3212_CNT:
          if (valno == 1)
            mm_data->t3212_cfg_counter = atoi(val[0]);
          else
          {
            TRACE_ERROR ("[PEI_CONFIG]: Wrong Number of Parameters");
          }
          break;

        case USE_STORED_BCCH:
          mm_data->config_use_stored_bcch = TRUE;
          TRACE_EVENT ("MS uses stored BCCH lists also with test SIM");
          break;

        case FFS_RESET_EPLMN:
          mm_reset_ffs();
          break;

        case FFS_READ_EPLMN:
          mm_read_ffs();
          mm_display_eplmn();
          break;

        case FFS_READ_EPLMN_INIT:
          mm_read_ffs_init();
          break;

        case FFS_WRITE_EPLMN:
          {
            UBYTE samplelist[18] ={'S','A','M','P','L','E',' ','L','I','S','T',' ','E','P','L','M','N'};
            memcpy(mm_data->reg.eqv_plmns.eqv_plmn_list, samplelist, 18);
            mm_write_eplmn_to_ffs();
          }
          break;

        default:
          break;
      } /* switch */

#if defined (OPTION_TIMER)
      /*
       * If timer manipulation
       */

      if (t_man)
      {
        t_man = FALSE;
        /*lint -e644 variable may not have been initialized*/
        if (t_num >= 0)
          tim_config_timer ( (UBYTE) t_num, t_mod, t_val);
        else
        {
            TRACE_ERROR ("[PEI_CONFIG]: Parameter out of Range");
        }
        /*lint +e644 variable may not have been initialized*/
      } /* if (t_man) */
#endif /* #if defined (OPTION_TIMER) */
    } /* while */
  }
  return PEI_OK;
}
#endif /* (NCONFIG) */


#if defined (NEW_FRAME)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : MM_PEI              |
| STATE   : code                       ROUTINE : mm_pei_config       |
+--------------------------------------------------------------------+

  PURPOSE : Dynamic Configuration

*/
/* Implements Measure#36 */
#if !defined (NCONFIG)
GLOBAL SHORT mm_pei_config ( char * inString, char * dummy )
{
  pei_config ( inString );

  return PEI_OK;
}
#endif /* (NCONFIG) */
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : MM_PEI              |
| STATE   : code                       ROUTINE : pei_monitor         |
+--------------------------------------------------------------------+

  PURPOSE : Monitoring of physical Parameters

*/
#if defined (NEW_FRAME)
LOCAL SHORT pei_monitor (void ** monitor)
#else
GLOBAL T_PEI_RETURN pei_monitor (void ** monitor)
#endif
{
  TRACE_FUNCTION ("pei_monitor()");

/* Implements Measure#32: Row 76 */

  *monitor = &mm_mon;

  return PEI_OK;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : MM_PEI              |
| STATE   : code                       ROUTINE : pei_create          |
+--------------------------------------------------------------------+

  PURPOSE : Create the Protocol Stack Entity

*/


#if defined (NEW_FRAME)

GLOBAL SHORT mm_pei_create (T_PEI_INFO **info)
{
  static const T_PEI_INFO pei_info =
  {
    "MM",
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
#else /* not (NCONFIG) */
      pei_config,
#endif /* (NCONFIG) */
      pei_monitor,
    },
    1024,     /* Stack Size      */
    10,       /* Queue Entries   */
    195,      /* Priority        */
    NUM_OF_MM_TIMERS,        /* number of timer */
    PASSIVE_BODY|COPY_BY_REF|PRIM_NO_SUSPEND /* flags */
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

  pcm_Init ();

  return PEI_OK;
}

#else

T_PEI_RETURN pei_create (T_VSI_CNAME * name)
{
  TRACE_FUNCTION ("pei_create()")

  /*
   * Close Resources if open
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

  *name = MM_NAME;

  pcm_Init ();

  return PEI_OK;
}

#endif

#endif
