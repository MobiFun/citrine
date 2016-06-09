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
|  Purpose :  This module defines the process body interface
|             for the component RR of the mobile station
+-----------------------------------------------------------------------------
*/

#ifndef RR_PEI_C
#define RR_PEI_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_RR

/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stdlib.h>
#include <stddef.h>     /* offsetof */
#include <stdio.h>      /* sprintf */
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
#include "cnf_rr.h"
#include "tok.h"
#include "rr.h"
#include "rr_em.h"
#include "cl_shrd.h"

#if !defined(_SIMULATION_)
#include "../../services/ffs/ffs.h"
#include "ffs_coat.h"
#endif  /* !_SIMULATION_ */

/*==== EXPORT =====================================================*/
/*
 * several handles for communication with other entities
 */
#ifdef TI_PS_HCOMM_CHANGE
GLOBAL T_HANDLE rr_handle          = VSI_ERROR;
#ifdef GPRS
GLOBAL T_HANDLE hCommGRR           = VSI_ERROR;    /* GRR Communication        */
#endif
#else /* for hCommHandles backward compatibility */
GLOBAL T_HANDLE hCommDL            = VSI_ERROR;
#if defined FF_EOTD
GLOBAL T_HANDLE hCommLC            = VSI_ERROR;
GLOBAL T_HANDLE hCommRRLP          = VSI_ERROR;
#endif /* FF_EOTD */
GLOBAL T_HANDLE hCommMM            = VSI_ERROR;
GLOBAL T_HANDLE hCommPL            = VSI_ERROR;
GLOBAL T_HANDLE rr_handle          = VSI_ERROR;
#ifdef GPRS
GLOBAL T_HANDLE hCommGRR           = VSI_ERROR;    /* GRR Communication        */
#endif
GLOBAL T_HANDLE  hCommMMI          = VSI_ERROR;    /* EM  Communication  */
#ifdef FF_WAP
  GLOBAL T_HANDLE  hCommWAP          = VSI_ERROR;    /* FMM-WAP  Communication */
#endif

#endif /* TI_PS_HCOMM_CHANGE */

static UBYTE stop_reg = FALSE;

GLOBAL  T_RR_DATA  rr_data_base;

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
const UBYTE std_bands[12] =
#else
const UBYTE std_bands[8] =
#endif
  {
    BAND_GSM_900,               /* std = 1 */
    BAND_GSM_900 | BAND_E_GSM,  /* std = 2 */
    BAND_PCS_1900,              /* std = 3 */
    BAND_DCS_1800,              /* std = 4 */
    BAND_DUAL,                  /* std = 5 */
    BAND_DUAL_EXT,              /* std = 6 */
    BAND_GSM_850,               /* std = 7 */
    BAND_DUAL_US                /* std = 8 */
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
    ,BAND_GSM_850 | BAND_DCS_1800,                               /* std = 9 */
    BAND_GSM_900 | BAND_E_GSM | BAND_PCS_1900,                  /* std = 10 */
    BAND_GSM_850 | BAND_GSM_900 | BAND_E_GSM | BAND_DCS_1800,   /* std = 11 */
    BAND_GSM_850 | BAND_GSM_900 | BAND_E_GSM | BAND_PCS_1900    /* std = 12 */
#endif
  };

/*==== PRIVATE ====================================================*/

LOCAL void pei_not_supported (void *data);

/*==== VARIABLES ==================================================*/
#ifdef _SIMULATION_
LOCAL BOOL              first_access = TRUE;
#endif /* _SIMULATION_ */

/*==== FUNCTIONS ==================================================*/

LOCAL const T_FUNC dl_table[] = {
  MAK_FUNC_0( dat_dl_establish_ind,           DL_ESTABLISH_IND          ), /* 0x80004003 */
  MAK_FUNC_0( dat_dl_establish_cnf,           DL_ESTABLISH_CNF          ), /* 0x80014003 */
  MAK_FUNC_0( dat_dl_release_ind,             DL_RELEASE_IND            ), /* 0x80024003 */
  MAK_FUNC_0( dat_dl_release_cnf,             DL_RELEASE_CNF            ), /* 0x80034003 */
  MAK_FUNC_S( for_dl_data_ind,                DL_DATA_IND               ), /* 0x80044003 */
#if defined (REL99) && defined (TI_PS_FF_EMR)
  MAK_FUNC_S( for_dl_short_unitdata_ind,      DL_SHORT_UNITDATA_IND     ), /* 0x80054003 */
#else
  MAK_FUNC_0( pei_not_supported,              DL_DATA_CNF               ), /* 0x80054003 */
#endif
};

LOCAL const T_FUNC rr_table[] = {
  MAK_FUNC_0( dat_rr_abort_req,               RR_ABORT_REQ              ),
  MAK_FUNC_0( att_rr_activate_req,            RR_ACTIVATE_REQ           ),
  MAK_FUNC_S( dat_rr_data_req,                RR_DATA_REQ               ),
  MAK_FUNC_0( att_rr_deactivate_req,          RR_DEACTIVATE_REQ         ),
  MAK_FUNC_S( dat_rr_establish_req,           RR_ESTABLISH_REQ          ),
  MAK_FUNC_0( att_rr_sync_req,                RR_SYNC_REQ               ),
  MAK_FUNC_0( att_rr_sync_hplmn_req,          RR_SYNC_HPLMN_REQ         )  
};

LOCAL const T_FUNC mph_table[] = {
  MAK_FUNC_N( pei_not_supported,              MPH_BCCH_IND              ),
  MAK_FUNC_0( att_mph_measurement_ind,        MPH_MEASUREMENT_IND       ),
  MAK_FUNC_0( dat_mph_dedicated_cnf,          MPH_DEDICATED_CNF         ),
  MAK_FUNC_0( dat_mph_dedicated_fail_cnf,     MPH_DEDICATED_FAIL_CNF    ),
  MAK_FUNC_0( att_mph_error_ind,              MPH_ERROR_IND             ),
  MAK_FUNC_0( dat_mph_emo_meas_ind,           MPH_EMO_MEAS_IND          ),
  MAK_FUNC_0( dat_mph_paging_ind,             MPH_PAGING_IND            ),
  MAK_FUNC_0( cs_mph_power_cnf,               MPH_POWER_CNF             ),
  MAK_FUNC_0( cs_mph_bsic_cnf,                MPH_BSIC_CNF              ),
  MAK_FUNC_S( for_mph_unitdata_ind,           MPH_UNITDATA_IND          ),
  MAK_FUNC_0( dat_mph_random_access_cnf,      MPH_RANDOM_ACCESS_CNF     ),
  MAK_FUNC_0( att_mph_sync_ind,               MPH_SYNC_IND              ),
  MAK_FUNC_0( att_mph_meas_order_cnf,         MPH_MEAS_ORDER_CNF        ),
#ifdef GPRS
  MAK_FUNC_0( att_mph_ext_meas_cnf,           MPH_EXT_MEAS_CNF          ),
#else
  MAK_FUNC_N( pei_not_supported,              0                         ),  /* 0x0D */
#endif
#if defined FF_EOTD
  MAK_FUNC_0( att_mph_ncell_pos_ind,          MPH_NCELL_POS_IND         ),
#else
  MAK_FUNC_N( pei_not_supported,              0                         ),  /* 0x0E */
#endif /* FF_EOTD */
  MAK_FUNC_0( att_mph_stop_dedicated_cnf,     MPH_STOP_DEDICATED_CNF),      /* 0x0f */
#ifdef GPRS
  MAK_FUNC_0( att_mph_meas_rep_cnf,           MPH_MEAS_REP_CNF          ) 
#else
  MAK_FUNC_N( pei_not_supported,              0                         )  /* 0x10 */
#endif 
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  ,
  MAK_FUNC_N( att_mph_init_rr_ind,            MPH_INIT_RR_IND           )  /* 0x11 */
#endif

}; 

#ifdef FF_EM_MODE
LOCAL const T_FUNC em_ul_table[] = {
  MAK_FUNC_0( dat_em_sc_info_req,             EM_SC_INFO_REQ            ), /* 0x00*/
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x01*/
  MAK_FUNC_0( dat_em_nc_info_req,             EM_NC_INFO_REQ            ), /* 0x02*/
  MAK_FUNC_0( dat_em_loc_pag_info_req,        EM_LOC_PAG_INFO_REQ       ), /* 0x03*/
  MAK_FUNC_0( dat_em_plmn_info_req,           EM_PLMN_INFO_REQ          ), /* 0x04*/
  MAK_FUNC_0( dat_em_cip_hop_dtx_info_req,    EM_CIP_HOP_DTX_INFO_REQ   ), /* 0x05*/
  MAK_FUNC_0( dat_em_mobdata_power_info_req,  EM_POWER_INFO_REQ         ), /* 0x06*/
  MAK_FUNC_0( dat_em_mobdata_id_info_req,     EM_IDENTITY_INFO_REQ      ), /* 0x07*/
  MAK_FUNC_0( dat_em_mobdata_version_info_req,EM_SW_VERSION_INFO_REQ    ), /* 0x08*/
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x09*/
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x0A*/
  MAK_FUNC_N( rr_em_rr_event_req,             EM_RR_EVENT_REQ           ), /* 0x0B*/
  MAK_FUNC_N (pei_not_supported,              0                         ), /* 0x0C */
  MAK_FUNC_N (pei_not_supported,              0                         ), /* 0x0D */
  MAK_FUNC_N (pei_not_supported,              0                         ), /* 0x0E */
  MAK_FUNC_N (pei_not_supported,              0                         ), /* 0x0F */
  MAK_FUNC_N (pei_not_supported,              0                         ), /* 0x10 */ /*SIM - event*/
  MAK_FUNC_0( rr_em_pco_trace_req,            EM_PCO_TRACE_REQ          ), /* 0x11*/ /*PCO output*/
#ifdef GPRS
  MAK_FUNC_0 (dat_em_fmm_sc_info_req,         EM_FMM_SC_INFO_REQ        ), /* 0x12 */
  MAK_FUNC_N (pei_not_supported,              0                         ), /* 0x13 */
  MAK_FUNC_0 (dat_em_fmm_nc_info_req,         EM_FMM_NC_INFO_REQ        ), /* 0x14 */
#else /*GPRS*/
  MAK_FUNC_N (pei_not_supported,              0                         ), /* 0x12 */
  MAK_FUNC_N (pei_not_supported,              0                         ), /* 0x13 */
  MAK_FUNC_N (pei_not_supported,              0                         ), /* 0x14 */
#endif /*GPRS*/
  MAK_FUNC_N (pei_not_supported,              0                         ), /* 0x15 */
  MAK_FUNC_N (pei_not_supported,              0                         ), /* 0x16 */
  MAK_FUNC_N (pei_not_supported,              0                         ), /* 0x17 */
  MAK_FUNC_N (pei_not_supported,              0                         ), /* 0x18 */
  MAK_FUNC_0 (dat_em_amr_info_req,            EM_AMR_INFO_REQ           )  /* 0x19 */
};
#endif /* FF_EM_MODE */


#if defined FF_EOTD
LOCAL const T_FUNC rrlc_table[] = {
  MAK_FUNC_0( dat_rrlc_meas_req,              RRLC_MEAS_REQ              )
};

LOCAL const T_FUNC rrrrlp_table[] = {
    MAK_FUNC_S( dat_rrrrlp_data_req,          RRRRLP_DATA_REQ            )
};
#endif /* FF_EOTD */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : RR_PEI              |
| STATE   : code                       ROUTINE : pei_primitive       |
+--------------------------------------------------------------------+

  PURPOSE : Process protocol specific primitive.

*/

LOCAL SHORT pei_primitive (void * ptr)
{
  T_PRIM *prim = ptr;

  /*
   *              |       |       |       |
   *              RR      EM     RRLC   RRRRLP            UPLINK
   *              |       |       |       |
   *      +-------v-------v-------v-------v------+
   *      |                                      |
   *      |                   RR                 |
   *      |                                      |
   *      +----------^------------------^--------+
   *                 |                  |
   *                MPH                 DL                DOWNLINK
   *                 |                  |
   *
   */

  TRACE_FUNCTION ("pei_primitive()");

  /*
   * On keypress may stop the registration timer
   */
  if (stop_reg)
  {
    stop_reg = FALSE;
    tim_reset_registration_timer ();
  }

  if (prim NEQ NULL)
  {
    ULONG            opc = prim->custom.opc;
    USHORT           n;
    const T_FUNC    *table;
    GET_INSTANCE_DATA;

    VSI_PPM_REC ((T_PRIM_HEADER*)prim, __FILE__, __LINE__);

    PTRACE_IN (opc);

    switch (SAP_NR(opc))
    {
      case SAP_NR(RR_UL)    : table =     rr_table; n = TAB_SIZE (    rr_table); break;
      case SAP_NR(DL_DL)    : table =     dl_table; n = TAB_SIZE (    dl_table); break;
      case SAP_NR(MPH_UL)   : table =    mph_table; n = TAB_SIZE (   mph_table); break;
#if defined FF_EOTD
      case SAP_NR(RRLC_UL)  : table =   rrlc_table; n = TAB_SIZE (  rrlc_table); break;
      case SAP_NR(RRRRLP_UL): table = rrrrlp_table; n = TAB_SIZE (rrrrlp_table); break;
#endif /* FF_EOTD */

#ifdef GPRS
      case RRGRR_UL:      gprs_get_table_n (&table, &n); break;
#endif /* GPRS */

#ifdef FF_EM_MODE
      case EM_Ul:         table = em_ul_table; n = TAB_SIZE (em_ul_table); break;
#endif /* FF_EM_MODE*/

      default :           table = NULL;      n = 0;                    break;
    }

    if (table NEQ NULL)
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
          srv_use_stored_prim ();
        }
      }
#if defined (REL99) && defined (TI_PS_FF_EMR)
      else if (PRIM_NR(opc) EQ DL_SPD_PID )
      {
        for_dl_short_unitdata_ind((T_DL_SHORT_UNITDATA_IND*) P2D(prim));
      }
#endif
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
| PROJECT : GSM-PS (6147)              MODULE  : RR_PEI              |
| STATE   : code                       ROUTINE : rr_pei_primitive    |
+--------------------------------------------------------------------+

  PURPOSE : used to restart a stored primitive.

*/
GLOBAL void rr_pei_primitive (T_PRIM * prim)
{
  pei_primitive (prim);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : RR_PEI              |
| STATE   : code                       ROUTINE : wake_up_rr          |
+--------------------------------------------------------------------+

  PURPOSE : This function can be called from the MMI or keypad driver
            to restart registration if in limited / no service condition.

*/
/*lint -esym(714,wake_up_rr) | Symbol not referenced         */
/*lint -esym(765,wake_up_rr) | external could be made static */
GLOBAL void wake_up_rr (void)
{
  GET_INSTANCE_DATA;
  T_TIME status = 0L;

  TRACE_FUNCTION ("wake_up_rr()");

  switch (rr_data->ms_data.rr_service)
  {
    case NO_SERVICE:
    case LIMITED_SERVICE:
      if (rr_data->ms_data.reg_counter >= 12)
      {
        TIMER_STATUS (rr_handle, TREG, &status);
        if (status)
        {
          stop_reg = TRUE;
          vsi_c_awake (VSI_CALLER_SINGLE);
        }
      }
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : RR_PEI              |
| STATE   : code                       ROUTINE : pei_not_supported   |
+--------------------------------------------------------------------+

  PURPOSE : An unsupported primitive is received.

*/

static void pei_not_supported (void * data)
{
  TRACE_FUNCTION ("pei_not_supported()");

  PFREE (data)
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : RR_PEI              |
| STATE   : code                       ROUTINE : pei_init            |
+--------------------------------------------------------------------+

  PURPOSE : Initialize Protocol Stack Entity

*/
LOCAL SHORT pei_init (T_HANDLE handle)
{
  GET_INSTANCE_DATA;
  rr_handle = handle;

  TRACE_FUNCTION ("pei_init()");

#ifdef TI_PS_HCOMM_CHANGE
  if (!cl_hcom_all_handles_open())
  {
    return PEI_ERROR;
  }
    
#ifdef GPRS
  if (hCommGRR < VSI_OK)
  {
    if ((hCommGRR = vsi_c_open (VSI_CALLER GRR_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif
#else /* for hCommHandles backward compatibility */
  if (hCommDL < VSI_OK)
  {
    if ((hCommDL = vsi_c_open (VSI_CALLER DL_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#if defined FF_EOTD
  if (hCommLC < VSI_OK)
  {
    if ((hCommLC = vsi_c_open (VSI_CALLER LC_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
  if (hCommRRLP < VSI_OK)
  {
    if ((hCommRRLP = vsi_c_open (VSI_CALLER RRLP_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif /* FF_EOTD */

  if (hCommMM < VSI_OK)
  {
    if ((hCommMM = vsi_c_open (VSI_CALLER MM_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

  if (hCommPL < VSI_OK)
  {
    if ((hCommPL = vsi_c_open (VSI_CALLER PL_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

#ifdef GPRS
  if (hCommGRR < VSI_OK)
  {
    if ((hCommGRR = vsi_c_open (VSI_CALLER GRR_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif

#ifdef FF_WAP /*FMM*/

  if (hCommWAP < VSI_OK)
  {
    if ((hCommWAP = vsi_c_open (VSI_CALLER WAP_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif

  if (hCommMMI < VSI_OK)
  {
    /*
     * Open MMI (Layer 4)
     */

    if ((hCommMMI = vsi_c_open (VSI_CALLER ACI_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif /* TI_PS_HCOMM_CHANGE */

  rr_data->ms_data.multislot_class_configured = FALSE;
  rr_data->ms_data.cmsp_configured = FALSE;
  /*
   *  Initialize Condat Coder Decoder and
   *  processes
   */

#if !defined(_SIMULATION_)
 // TRACE_EVENT (rr_version());
 // SYST_TRACE (rr_version());
#endif  /* !_SIMULATION_ */

  ccd_init ();
  att_init_gsm_data ();
  dat_init_rr_data ();
  cs_init_process ();
  pcm_Init ();
  cl_shrd_init(rr_handle);
 
#if defined(_SIMULATION_)
  rr_csf_ms_cap ();
  rr_csf_check_rfcap (TRUE);
#endif

#ifdef GPRS
  gprs_init_gprs_data ();
#endif

#ifdef FF_EM_MODE
  em_init_rr_event_trace();
  em_rr_sem_init();
  em_init_get_hchn ();
#endif /* FF_EM_MODE */

  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : RR_PEI              |
| STATE   : code                       ROUTINE : pei_timeout         |
+--------------------------------------------------------------------+

  PURPOSE : Process timeout

*/

LOCAL SHORT pei_timeout (USHORT index)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("pei_timeout ()");

  /*
   * Handle Timeouts
   */
  tim_exec_timeout (index);

  while (ENTITY_DATA->use_stored_entries)
  {
    ENTITY_DATA->use_stored_entries = FALSE;
    srv_use_stored_prim ();
  }

  return PEI_OK;
}

#ifdef _SIMULATION_
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : RR_PEI              |
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
  vsi_c_close (VSI_CALLER hCommDL);
  hCommDL = VSI_ERROR;

#if defined FF_EOTD
  vsi_c_close (VSI_CALLER hCommLC);
  hCommLC = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommRRLP);
  hCommRRLP = VSI_ERROR;
#endif /* FF_EOTD */

  vsi_c_close (VSI_CALLER hCommMM);
  hCommMM = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommPL);
  hCommPL = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommMMI);
  hCommMMI = VSI_ERROR;

#ifdef FF_WAP /*FFM*/
  vsi_c_close (VSI_CALLER hCommWAP);
  hCommWAP = VSI_ERROR;
#endif
#endif /* TI_PS_HCOMM_CHANGE */

#ifdef FF_EM_MODE
  em_rr_sem_exit();
#endif /* FF_EM_MODE */

  ccd_exit();
  cl_shrd_exit();

  return PEI_OK;
}
#endif /* _SIMULATION_ */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : RR_PEI              |
| STATE   : code                       ROUTINE : pei_config          |
+--------------------------------------------------------------------+

  PURPOSE : Dynamic Configuration

*/

#if !defined (NCONFIG)
LOCAL const KW_DATA kwtab[] = {
#ifdef OPTION_TIMER
                   RR_TIMER_SET,        TIMER_SET,
                   RR_TIMER_RESET,      TIMER_RESET,
                   RR_TIMER_SPEED_UP,   TIMER_SPEED_UP,
                   RR_TIMER_SLOW_DOWN,  TIMER_SLOW_DOWN,
                   RR_TIMER_SUPPRESS,   TIMER_SUPPRESS,
#endif  /* OPTION_TIMER */
                   RR_TFAST_CS,         TIM_FAST,
                   RR_TNORMAL_CS,       TIM_NORMAL,
                   RR_FCR,              FCR,
                   RR_SCR,              SCR,
                   RR_DLE,              DLE,
                   RR_FCA,              FCA,
                   RR_FRL,              FRL,
                   RR_FHO,              FHO,
                   RR_IHO,              IHO,
                   RR_NO_SYS_TIME,      NO_SYS_TIME,
                   RR_GSM_OFFSET,       GSM_OFFSET,
                   RR_DCS_OFFSET,       DCS_OFFSET,
                   RR_CTO,              CTO,
                   RR_NKC,              NKC,
                   RR_RESTRICTED_BAND,  ID_RESTRICTED_BAND,
                   RR_BL_CS,            BL_CS,
                   RR_U_RXT,            U_RXT,
                   RR_M_RXT,            M_RXT,
                   RR_L_RXT,            L_RXT,
                   RR_FBLS,             FBLS,
                   RR_SHOW_BL,          SHOW_BL,
                   RR_SHOW_WL,          SHOW_WL,
                   RR_SET_NPS_DELAY,    SET_NPS_DELAY,
#if defined (_SIMULATION_)
                   RR_MT_CALL_NAME,     RR_MT_CALL,
                   RR_MT_SMS_0_NAME,    RR_MT_SMS_0,
                   RR_MT_SMS_2_NAME,    RR_MT_SMS_2,
                   RR_SRV_FULL_NAME,    RR_SERVICE_FULL,
                   RR_SRV_LIM_NAME,     RR_SERVICE_LIMITED,
                   RR_SRV_NO,           RR_SERVICE_NO,
                   RR_PSEUDO_SYNC_HO,   ID_PSEUDO_SYNC_HO,
                   RR_PCM,              ID_PCM,
                   RR_DCS_PCLASS_3,     DCS_PCLASS_3,
#endif  /* _SIMULATION_ */
                   RR_MULTISLOT_CLASS,  ID_MULTISLOT_CLASS,
                   RR_CMSP,             ID_CMSP,
#if !defined(_SIMULATION_)
                   RR_FFS_CHECK,        ID_FFS_CHECK,
#endif
#if defined (_SIMULATION_FFS_)
                   RR_ERASE_WL,         ERASE_WL,
                   RR_ERASE_BL,         ERASE_BL,
                   RR_SHIELD,           SHIELD,
                   RR_INIT_FFS,         INIT_FFS,
                   RR_WRITE_FFS,        WRITE_FFS,
#endif /* (_SIMULATION_FFS_)*/
                   RR_SET_WL,           SET_WL,
                   RR_SET_BL,           SET_BL,
                   RR_SET_WL_REGION,    SET_WL_REGION,
                   RR_SET_LAST_USED_SC, SET_LAST_USED_SC,
                   RR_SCS,              SCS,
                   RR_SET_WL_PLMN,      SET_WL_PLMN,
                   "",                  0
                   };

static const KW_DATA partab[] = {
#ifdef OPTION_TIMER
                   T3110_NAME,     T3110,
                   T3122_NAME,     T3122,
                   T3126_NAME,     T3126,
                   TRESELECT_NAME, T_RESELECT,
                   TREG_NAME,      TREG,
                   TABORT_NAME,    TABORT,
#endif
                   "",                 0
                   };
#endif /*!defined (NCONFIG)*/
/* Implements Measure#36 */
#if defined(NCONFIG)
#else /* NCONFIG */
LOCAL SHORT pei_config (T_PEI_CONFIG inString)
{
  GET_INSTANCE_DATA;
#if !defined (NCONFIG)
  #define NMAX_VAL  20
  {
    char    * s = inString;
    SHORT     valno;
    char    * keyw;
    char    * val [NMAX_VAL];
#ifdef OPTION_TIMER
    BOOL      t_man = FALSE;
    SHORT     t_num = 0;
    LONG      t_val = 0;

#endif
#if defined(_SIMULATION_FFS_)
    UBYTE     count = 0,index = 0,mcc[SIZE_MCC]; 
    SHORT     mcc_val = 0;
#endif  /* _SIMULATION_FFS */

    TRACE_FUNCTION ("pei_config()");

    TRACE_EVENT (s);

    tok_init(s);

    /*
     * Parse next keyword and number of variables
     */
    while ((valno = tok_next(&keyw,val)) NEQ TOK_EOCS)
    {
      switch ( tok_key((KW_DATA *)kwtab,keyw))
      {
        case TOK_NOT_FOUND:
          TRACE_ERROR ("[PEI_CONFIG]: Illegal Keyword");
          break;

        case FCR:         /* fast cell reselection */
          TRACE_EVENT ("[PEI_CONFIG]: FAST CELL RESELECTION");
          rr_data->dyn_config.fcr = 1;
          break;

        case SCS:
          {
            SHORT i = 0;
            PALLOC (error_ind, MPH_ERROR_IND);

            TRACE_EVENT(" [PEI_CONFIG]: START CELL SELECTION");

            for( i=0; i < 6; i++)
              rr_data->nc_data[i].bcch_status = NON_DECODED;
             /*
              * Simulating Downlink Failure
              */
             error_ind->cs = CS_DOWN_LINK_FAIL;
             error_ind->arfcn = rr_data->nc_data[SC_INDEX].arfcn;
             pei_primitive ((T_PRIM *)D2P(error_ind));
          }
          break;

        case SCR:         /* slow cell reselection */
          TRACE_EVENT ("[PEI_CONFIG]: SLOW CELL RESELECTION");
          rr_data->dyn_config.scr = 1;
          break;

        case DLE:         /* downlink error        */
          {
            PALLOC (error_ind, MPH_ERROR_IND);

              TRACE_EVENT ("[PEI_CONFIG]: DOWNLINK ERROR");
              error_ind->cs = CS_DOWN_LINK_FAIL;
              pei_primitive ((T_PRIM *)D2P(error_ind));
          }
          break;

        case FRL:         /* force radio link timeout      */
          {
            PALLOC (error_ind, MPH_ERROR_IND);

              TRACE_EVENT ("[PEI_CONFIG]: RADIO LINK ERROR");
              error_ind->cs = CS_RADIO_LINK_FAIL;
              pei_primitive ((T_PRIM *)D2P(error_ind));
          }
          break;
        case FCA:         /* failed channel assign */
          TRACE_EVENT ("[PEI_CONFIG]: FAILED CHANNEL ASSIGN");
          rr_data->dyn_config.fca = 1;
          break;

        case FHO:         /* forced handover */
          TRACE_EVENT ("[PEI_CONFIG]: FORCED HANDOVER");
          rr_data->dyn_config.fho = 1;
          break;

        case IHO :     /* Lock/Unlock the DUT to the cell it is already camping */
          TRACE_EVENT_P1("[PEI_CONFIG]: Lock/Unlock the DUT to the cell it is already camping : %s", val[0]);
          rr_data->dyn_config.iho = atoi (val[0]);
          break;

        case GSM_OFFSET:  /* Offset for GSM channels */
          rr_data->dyn_config.gsm_offset = atoi (val[0]);
          break;

        case DCS_OFFSET:  /* Offset for DCS channels */
          rr_data->dyn_config.dcs_offset = atoi (val[0]);
          break;

        case CTO:         /* enable cell test operation mode */
          rr_data->cell_test_operation = TRUE;
          break;

        case NKC:         /* request to cipher when no ciphering key available */
          rr_data->dyn_config.nkc = TRUE;
          break;
        case BL_CS:       /* Controls Black List search */
          if(valno)
          {
            valno = atoi (val[0]);
            rr_data->dyn_config.bl_cs_en = valno ? 1 : 0;
            TRACE_EVENT_P1("[PEI_CONFIG]: BL_CS : Black List search : %s", val[0]);
          }
          else
          {
            TRACE_ERROR ("[PEI_CONFIG]: BL_CS-Wrong Number of Parameters");
          }
          break;
        case U_RXT:       /* Upper RxLev threshold Level*/
          if(valno>0 AND valno<=MAX_NUM_BANDS)
          {
            rr_csf_write_rxlev_thr( U_RXT, valno, val );
          }
          else
          {
            TRACE_ERROR ("[PEI_CONFIG]: U_RXT-Wrong Number of Parameters");
          }
          break;
        case M_RXT:       /* Medium threshold level*/ 
          if(valno>0 AND valno<=MAX_NUM_BANDS)
          {
            rr_csf_write_rxlev_thr( M_RXT, valno, val );
          }
          else
          {
            TRACE_ERROR ("[PEI_CONFIG]: M_RXT-Wrong Number of Parameters");
          }
          break;
        case L_RXT:       /* Lower threshold level*/ 
          if(valno>0 AND valno<=MAX_NUM_BANDS)
          {
            rr_csf_write_rxlev_thr( L_RXT, valno, val );
          }
          else
          {
            TRACE_ERROR ("[PEI_CONFIG]: L_RXT-Wrong Number of Parameters");
          }
          break;
        case FBLS:       /* Force Black List Search */ 
          if((GET_STATE(STATE_ATT) EQ ATT_IDLE) AND 
             (rr_data->ms_data.rr_service EQ FULL_SERVICE))
          {              
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
            if (rr_data->cs_data.region EQ BOTH_REGIONS)
            {
              if(srv_is_list_set(&rr_data->cs_data.black_list.list[EUROPEAN_REGION]) OR
                 srv_is_list_set(&rr_data->cs_data.black_list.list[AMERICAN_REGION]))
              {
                att_start_cell_selection(RR_ORIGINATED,CS_PARALLEL,BLACK_LIST_SEARCH_MODE);
              } 
              else
                TRACE_EVENT("Black List empty");
            }
            else
            {
#endif
            if(srv_is_list_set(
                  &rr_data->cs_data.black_list.list[rr_data->cs_data.region]))
            {
              att_start_cell_selection(RR_ORIGINATED,CS_PARALLEL,BLACK_LIST_SEARCH_MODE);
            } 
            else
              TRACE_EVENT("Black List empty");
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
            }
#endif
          }
          break;
        case ID_RESTRICTED_BAND:
          rr_data->dyn_config.set_band = atoi (val[0]);

          switch (GET_STATE (STATE_ATT))
          {
            case ATT_NULL:
            case ATT_NO_SERVICE:
            case ATT_CS1:
            case ATT_CS2:
            case ATT_CS3:
            case ATT_IDLE:
              rr_data->ms_data.rr_service = NO_SERVICE;
              att_code_rr_abort_ind (RRCS_ABORT_CEL_SEL_FAIL);
              SET_STATE (STATE_ATT, ATT_NO_SERVICE);

              rr_data->cs_data.scan_mode = CS_FIRST_SCAN_FIRST_ATTEMPT;

              att_reset_old_lai_rac();

              cs_set_null ();
              cs_set_all ();
              cs_start_scan ();
              /*
               * initialise cell selection parameter
               */
              rr_data->sc_data.selection_type   = CELL_SELECTION;
              rr_data->sc_data.mm_started       = FALSE;
              rr_data->sc_data.found_entries    = 0;
              att_init_cr_data();

              SET_STATE (STATE_ATT, ATT_CS1);

              switch (rr_data->dyn_config.set_band)
              {
                case 0:
                  TRACE_EVENT ("[PEI_CONFIG]: Set band to default");
                  break;
                case 1:
                  TRACE_EVENT ("[PEI_CONFIG]: Set band to GSM 900");
                  break;
                case 2:
                  TRACE_EVENT ("[PEI_CONFIG]: Set band to DCS 1800");
                  break;
              } /* switch (rr_data->set_band) */
          } /* switch (GET_STATE (STATE_ATT)) */
          break;

      case NO_SYS_TIME:
        rr_data->dyn_config.no_sys_time = TRUE;
        break;

      case SET_LAST_USED_SC:
        if(valno)
        {
          rr_data->cs_data.white_list.last_sc_arfcn = atoi (val[0]);
          TRACE_EVENT_P1("LAST SC ARFCN = %x",rr_data->cs_data.white_list.last_sc_arfcn);
        }
        else
        {
          TRACE_ERROR ("[PEI_CONFIG]: LAST_USED_SC-Wrong Number of Parameters");
        }
        break;

#if defined (_SIMULATION_FFS_)
      case INIT_FFS:
        rr_csf_ffs_init();
        TRACE_EVENT_P1("LAST SC ARFCN = %x",rr_data->cs_data.white_list.last_sc_arfcn);
        break;

      case WRITE_FFS:
        {
          rr_csf_write_black_list(&rr_data->cs_data.black_list.list[0]);
          rr_csf_write_white_list(&rr_data->cs_data.white_list);
        }
        break;
#endif  /* _SIMULATION_FFS_ */


#ifdef OPTION_TIMER
      case TIMER_SET:
        if (valno EQ 2)
        {
          t_man = TRUE;
          t_num = tok_key((KW_DATA *)partab,val[0]);
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
        if (valno EQ 1)
        {
          t_man = TRUE;                          /* Timermanipulation         */
          t_num = tok_key((KW_DATA *)partab,val[0]);
          t_val = 0L;
        }
        else
        {
          TRACE_ERROR ("[PEI_CONFIG]: Wrong Number of Parameters");
        }
        break;
      case TIMER_SPEED_UP:
      case TIMER_SLOW_DOWN:
        if (valno EQ 2)
        {
          t_man = TRUE;
          t_num = tok_key((KW_DATA *)partab,val[0]);
          t_val = atoi(val[1]);
          if (t_val <= 0L)
            t_val = 1L;
        }
        else
        {
          TRACE_ERROR ("[PEI_CONFIG]: Wrong Number of Parameters");
        }
        break;
      case TIM_FAST:
        if(valno)
        {
          rr_data->dyn_config.tfast_cs_val = atoi(val[0])*60000;
          TRACE_EVENT_P1("[PEI_CONFIG]TFAST_CS : %dms",rr_data->dyn_config.tfast_cs_val);
          if(!rr_data->dyn_config.tfast_cs_val)
          {
            if(IS_TIMER_ACTIVE(T_FAST_CS))
              TIMERSTOP(T_FAST_CS);
          }
        }
        else
        {
          TRACE_ERROR ("[PEI_CONFIG]: TFAST_CS-Wrong Number of Parameters");
        }
        break;
      case TIM_NORMAL:
        if(valno)
        {
          rr_data->dyn_config.tnormal_cs_val = atoi(val[0])*60000;
          TRACE_EVENT_P1("[PEI_CONFIG]TNORMAL_CS : %dms",rr_data->dyn_config.tnormal_cs_val);
          if(!rr_data->dyn_config.tnormal_cs_val)
          {
            if(IS_TIMER_ACTIVE(T_NORMAL_CS))
              TIMERSTOP(T_NORMAL_CS);
          }
        }
        else
        {
          TRACE_ERROR ("[PEI_CONFIG]: TNORMAL_CS-Wrong Number of Parameters");
        }
        break;
#endif

#if defined (_SIMULATION_)
      case RR_MT_CALL:
        /*
         * Start mobile terminated call
         * if simulation is enabled
         */
        break;
      case RR_MT_SMS_0:
        /*
         * Start mobile terminated SMS class 0
         * if simulation is enabled
         */
        break;
      case RR_MT_SMS_2:
        /*
         * Start mobile terminated SMS class 0
         * if simulation is enabled
         */
        break;
      case RR_SERVICE_FULL:
        /*
         * Set full service
         * if simulation is enabled
         */
        break;
      case RR_SERVICE_LIMITED:
        /*
         * Set limited service
         * if simulation is enabled
         */
        break;
      case RR_SERVICE_NO:
        /*
         * Set no service
         * if simulation is enabled
         */
        break;
      case RR_MO_CONNECT:
        break;

      case ID_PSEUDO_SYNC_HO:
        if (valno)
        {
          if (atoi (val[0]))
            rr_data->ms_data.rf_cap.ps = TRUE;
          else
            rr_data->ms_data.rf_cap.ps = FALSE;
            TRACE_EVENT_P1 ("PSEUDO_SYNC_HO capability changed (%u)",
            rr_data->ms_data.rf_cap.ps);
          rr_csf_fit_capability ();
        }
        else
        {
          TRACE_ERROR("[PEI_CONFIG]: use CONFIG PSEUDO_SYNC_HO=<value>");
        }
        break;
      case DCS_PCLASS_3 :
        TRACE_EVENT_P1("DCS_PCLASS_3 old [%d] -> new[3]", 
                        rr_data->ms_data.rf_cap.rf_power.pow_class4[IDX_PWRCLASS_1800].pow_class);
        rr_data->ms_data.rf_cap.rf_power.pow_class4[IDX_PWRCLASS_1800].pow_class = POWER_CLASS_3;
        break;
      case ID_PCM: /* sample: COMMAND ("RR CONFIG PCM=<MSCAP,0x0F,0xb7,0b10001010,0,0,0>") */
        if (valno)
        {
          UBYTE  b[NMAX_VAL-1];
          UBYTE  version,ok=1;
          USHORT i,n=valno-1;

          if ( pcm_ReadFile ( val[0], n, b, &version) EQ PCM_OK)
          {
            for ( i = 0; i < n; ++i  )
            {
              UBYTE c,*p = val[i+1];
              int   dig,z = 0;
              UBYTE base;

              if ( p[0] EQ '0' )
              {
                if ( p[1] EQ 'x' OR p[1] EQ 'X' ) { p += 2; base = 16; } else
                if ( p[1] EQ 'b' OR p[1] EQ 'B' ) { p += 2; base = 2;  } else
                  base = 8;
              }
              else
                  base = 10;

              while ( ok AND (c = *p++) != '\0' )
              {
                if ( c >= '0' AND c <= '9' ) dig = c - '0';      else
                if ( c >= 'A' AND c <= 'F' ) dig = c - 'A' + 10; else
                if ( c >= 'a' AND c <= 'f' ) dig = c - 'a' + 10; else
                {
                  ok = 0;
                  break;
                }

                if ( base EQ 2  ) ok = dig >= 0 AND dig <=  1; else
                if ( base EQ 8  ) ok = dig >= 0 AND dig <=  7; else
                if ( base EQ 10 ) ok = dig >= 0 AND dig <=  9; else
                if ( base EQ 16 ) ok = dig >= 0 AND dig <= 15;

                if ( ok )
                  z = z * base + dig;
              };

              if ( ok )
              {
                TRACE_EVENT_P4 ( "%s[%d]: 0x%02x -> 0x%02x", val[0], i, b[i], z );
                b[i] = z;
              }
              else
              {
                TRACE_EVENT_P1 ( "invalid number:%s", val[i+1] );
                break;
              }
            }

            if ( ok )
            {
              if ( pcm_WriteFile ( val[0], n, b) EQ PCM_OK)
              {
                rr_csf_read_rfcap ();
                rr_csf_fit_capability ();
                rr_csf_read_imei (&rr_data->ms_data.imei);
                rr_csf_ms_cap ();

              }
              else
              {
                TRACE_EVENT ( "cannot perform PCM write action" );
              }
            }
          }
          else
          {
            TRACE_EVENT ( "cannot perform PCM read action" );
          }

          TRACE_EVENT_P1 ( "PCM '%s' " , val[0] );
        }
        else
        {
          TRACE_ERROR("[PEI_CONFIG]: use PCM=<value>");
        }
       break;

#endif  /* _SIMULATION_ */
#if defined(_SIMULATION_FFS_)
      case SHIELD:
        if(!valno)
        {
          memset(&rr_data->dyn_config.mcc_shield,0,sizeof(T_shield_mcc));
        }
        else
        {
          rr_data->dyn_config.mcc_shield.enabled = TRUE;
          if(valno > MAX_MCC_SHIELD)
          {
            TRACE_ERROR("max configurable mcc shield values are four");
            valno = MAX_MCC_SHIELD;
          }
          index = rr_data->dyn_config.mcc_shield.index;
          for(count=0;count< valno;count++)
          {
            if(index >= MAX_MCC_SHIELD)
            {
              index = 0;
            } 
            mcc_val = atoi(val[count]);
            if(!mcc_val)
            {
              TRACE_ERROR("mcc value cannot be zero");
            }
            else
            {
              mcc[0] = (mcc_val/100);
              mcc[1] = (mcc_val/10)%10;            
              mcc[2] = (mcc_val % 10);
              if(!rr_csf_mcc_present(mcc))
              {
                memcpy(rr_data->dyn_config.mcc_shield.mcc[index],mcc,SIZE_MCC);
                index++;
              }
            } 
          }
          rr_data->dyn_config.mcc_shield.index = index;
       } 
       rr_csf_write_mcc_shield_to_ffs();        
       break;	
#endif  /* defined(_SIMULATION_FFS_)*/
      case ID_MULTISLOT_CLASS:
        if (valno EQ 1)
        {
          UBYTE ms = atoi (val[0]);
          TRACE_EVENT_P1 ("MULTISLOT_CLASS changed (%u)", ms);
          rr_data->ms_data.multislot_class_configured = TRUE;
          rr_data->ms_data.rf_cap.rf_ms.gprs_ms_class = ms;
#ifdef GPRS
#ifdef REL99
          rr_data->ms_data.ra_cap.ra_cap_values.acc_cap.v_ms_struct = SUPPORTED;
          rr_data->ms_data.ra_cap.ra_cap_values.acc_cap.ms_struct.gprs_struct.gprs_ms_class = ms;
          if (rr_data->ms_data.ra_cap.c_ra_cap_r EQ MAX_ADD_RA_CAP)
#else
          rr_data->ms_data.ra_cap.acc_cap.v_ms_struct = SUPPORTED;
          rr_data->ms_data.ra_cap.acc_cap.ms_struct.gprs_struct.gprs_ms_class = ms;
          if (rr_data->ms_data.ra_cap.v_ra_cap2)
#endif
          {
            /*
             * zero means that the same value for multislot parameters as given
             * in an earlier access capabilities field within this IE apply
             * also here.
             */
#ifdef REL99
             rr_data->ms_data.ra_cap.ra_cap_r[0].ra_cap_values.acc_cap.v_ms_struct = FALSE;
#else
             rr_data->ms_data.ra_cap.ra_cap2.acc_cap.v_ms_struct = FALSE;
#endif
          }
#endif
        }
        else
        {
          TRACE_ERROR("[PEI_CONFIG]: use CONFIG MULTISLOT_CLASS=<value>");
        }
        break;
      case ID_CMSP:
        if (valno EQ 1)
        {
          BOOL cmsp = atoi (val[0]);
          if (cmsp)
            cmsp = 1; /* boolean */
          TRACE_EVENT_P2 ("CMSP changed (%u->%u)", rr_data->ms_data.rf_cap.cmsp, cmsp);
          rr_data->ms_data.cmsp_configured = TRUE;
          rr_data->ms_data.rf_cap.cmsp = cmsp;
        }
        else
        {
          TRACE_ERROR("[PEI_CONFIG]: use CONFIG CMSP=<0|1>");
        }
        break;
#if defined(_TARGET_)
#endif /* _TARGET_ */
#if defined (_SIMULATION_FFS_)
      case ERASE_WL:
        /*lint -e655 (Warning -- bit-wise operation uses (compatible) enum's)*/
        cs_clear_white_list (CLR_WHITE_LIST_RAM | CLR_WHITE_LIST_SIM | CLR_WHITE_LIST_FFS);
        /*lint +e655 (Warning -- bit-wise operation uses (compatible) enum's)*/
        break;
      case ERASE_BL:
        /*lint -e655 (Warning -- bit-wise operation uses (compatible) enum's)*/
        cs_clear_black_list (CLR_BLACK_LIST_RAM | CLR_BLACK_LIST_FFS);
        /*lint +e655 (Warning -- bit-wise operation uses (compatible) enum's)*/
        break;
#endif  /* _SIMULATION_FFS_ */
      case SHOW_BL:
        /*lint -e655 (Warning -- bit-wise operation uses (compatible) enum's)*/
        srv_trace_black_list ();
        /*lint +e655 (Warning -- bit-wise operation uses (compatible) enum's)*/
        break;
      case SHOW_WL:
        /*lint -e655 (Warning -- bit-wise operation uses (compatible) enum's)*/
        srv_trace_white_list ();
        /*lint +e655 (Warning -- bit-wise operation uses (compatible) enum's)*/
        break;
      case SET_WL:
        if(valno < 2)
        {
          TRACE_ERROR("[PEI_CONFIG]: wrong Number of Parameters");
        }
        else
        {
          UBYTE c_region;
          USHORT c_arfcn;
          UBYTE i;

          if(valno > 6)
            valno = 6;  /* 1 to 5 ARFCNs can be configured at a time */

          c_region = atoi(val[0]);
          if(cs_check_region(c_region))
          {  
            rr_data->cs_data.white_list.region = c_region;

            for(i=1;i<valno;i++)
            {
              c_arfcn = atoi(val[i]);
              if(cs_check_arfcn_range(c_arfcn))
              {
                srv_set_channel(&rr_data->cs_data.white_list.list,c_arfcn);
                TRACE_EVENT_P1("Arfcn:%d -> WL", c_arfcn);
              } /* arfcn */
            } /* for */
          } /* region */
        } /* valno */
        break;

      case SET_BL:
        if(valno < 2)
        {
          TRACE_ERROR("[PEI_CONFIG]: wrong Number of Parameters");
        }
        else
        {
          UBYTE c_region;
          USHORT c_arfcn;
          UBYTE i;

          if(valno > 6)
            valno = 6;  /* 1 to 5 ARFCNs can be configured at a time */
        
          c_region = atoi(val[0]);
          if(cs_check_region(c_region))
          {  
            for(i=1;i<valno;i++)
            {
              c_arfcn = atoi(val[i]);
              if(cs_check_arfcn_range(c_arfcn))
              {
                srv_set_channel(&rr_data->cs_data.black_list.list[c_region],c_arfcn);
                TRACE_EVENT_P1("Arfcn:%d -> BL", c_arfcn);
              } /* arfcn */
            } /* for */
          } /* region */
        } /* valno */
        break;
      case SET_WL_REGION:
        if(valno)
        {
          if(cs_check_region((U8)atoi (val[0])))
            rr_data->cs_data.white_list.region = atoi (val[0]);
        }
        else
        {
          TRACE_ERROR ("[PEI_CONFIG]: wrong Number of Parameters");
        }        
        break;
      case SET_WL_PLMN:
        if(valno < 2)
        {
          TRACE_ERROR("[PEI_CONFIG]: wrong Number of Parameters");
        }
        else
        {
          SHORT plmn  = atoi (val[0]);
          /* copy MCC */
          rr_data->cs_data.white_list.last_sc_lac.mcc[0] = (plmn/100);
          rr_data->cs_data.white_list.last_sc_lac.mcc[1] = (plmn/10)%10;
          rr_data->cs_data.white_list.last_sc_lac.mcc[2] = (plmn % 10);

          /* Copy MNC */
          plmn  = atoi (val[1]);
          rr_data->cs_data.white_list.last_sc_lac.mnc[0] = (plmn/100);
          rr_data->cs_data.white_list.last_sc_lac.mnc[1] = (plmn/10)%10;
          rr_data->cs_data.white_list.last_sc_lac.mnc[2] = (plmn % 10);

          if(!rr_data->cs_data.white_list.last_sc_lac.mnc[2])
            rr_data->cs_data.white_list.last_sc_lac.mnc[2] = 0x0f;

          TRACE_EVENT_P6( "[PEI_CONFIG]SET_WL_PLMN MCC/MNC r=%x%x%x/%x%x%x",
            rr_data->cs_data.white_list.last_sc_lac.mcc[0],
            rr_data->cs_data.white_list.last_sc_lac.mcc[1],
            rr_data->cs_data.white_list.last_sc_lac.mcc[2],
            rr_data->cs_data.white_list.last_sc_lac.mnc[0],
            rr_data->cs_data.white_list.last_sc_lac.mnc[1],
            rr_data->cs_data.white_list.last_sc_lac.mnc[2]); 
        }
        break;
      case SET_NPS_DELAY:
        if(valno)
        {
          rr_data->dyn_config.lim_ser_nps_delay = atoi (val[0]);
        }
        else
        {
          TRACE_ERROR ("[PEI_CONFIG]: wrong Number of Parameters");
        }        
        break;
      default:
        break;
      }

#ifdef OPTION_TIMER
    /*
     * If timer manipulation
     */

      if (t_man)
      {
        t_man = FALSE;
        if (t_num >= 0 AND t_num < MAX_RR_TIMER)
        ; /* vsi_t_config is removed in latest GPF */
        else
        {
            TRACE_ERROR ("[PEI_CONFIG]: Parameter out of Range");
        }
      }
#endif
    }
  } /*!defined (NCONFIG)*/
#endif /*!defined (NCONFIG)*/

  return PEI_OK;
}
#endif /* NCONFIG */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : RR_PEI              |
| STATE   : code                       ROUTINE : rr_pei_config       |
+--------------------------------------------------------------------+

  PURPOSE : Dynamic Configuration

*/
/*lint -esym(714,rr_pei_config) | Symbol not referenced         | used by Frame  */
/*lint -esym(765,rr_pei_config) | external could be made static | used by Frame  */
/* Implements Measure#36 */
#if defined(NCONFIG)
#else /* NCONFIG */
GLOBAL SHORT rr_pei_config ( char * inString, char * dummy )
{
  pei_config ( inString );

  return PEI_OK;
}
#endif /* NCONFIG */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : RR_PEI              |
| STATE   : code                       ROUTINE : pei_create          |
+--------------------------------------------------------------------+

  PURPOSE : Create the Protocol Stack Entity

*/
/*lint -esym(714,rr_pei_create) | Symbol not referenced         | used by Frame  */
/*lint -esym(765,rr_pei_create) | external could be made static | used by Frame  */
GLOBAL SHORT rr_pei_create (T_PEI_INFO **info)
{
  static const T_PEI_INFO pei_info =
  {
    "RR",
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
      NULL,             /* no pei_config function */
#else /* NCONFIG */
      pei_config,
#endif /* NCONFIG */
      NULL,
    },
#ifdef GPRS
    1736,     /* Stack Size      */
#else
    1936,     /* Stack Size increase from 1436 to 1936 2007/5/31     */
#endif
    10,       /* Queue Entries   */
    205,      /* Priority        */
    NUM_OF_RR_TIMERS,        /* number of timer */
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
#endif
