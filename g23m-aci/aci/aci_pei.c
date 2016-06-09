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
|             for the AT Command Interpreter.
+-----------------------------------------------------------------------------
*/

#ifndef ACI_PEI_C
#define ACI_PEI_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

#include "ccdapi.h"
#include "cnf_aci.h"
#include "mon_aci.h"  /* mouais... */
#include "p_aci.val"    /* SKA 2002-09-02 for MAX_CMD_LEN */
#include "line_edit.h"  /* SKA 2002-09-02                 */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci.h"
#include "aci_mem.h"

#if 0 //#ifndef _SIMULATION_	// FreeCalypso change
#include "rv_swe.h"
#ifdef RVM_GIL_SWE
#include "gil/gil_gpf.h" /* MMI_GIL_IND dispatch function */
#endif /* RVM_GIL_SWE */
#endif /* _SIMULATION_ */


#include "psa.h"
#include "psa_cc.h"
#include "psa_mm.h"
#include "psa_sim.h"
#include "psa_mmi.h"
#include "psa_sms.h"
#include "aoc.h"
#include "phb.h"
#include "hl_audio_drv.h"

#include "aci_lst.h"

#ifdef DTI
  #include "dti.h"      /* functionality of the dti library */
  #include "dti_conn_mng.h"
  #include "dti_cntrl_mng.h"
#endif

#ifdef GPRS 
  #include "gprs.h"
  #include "gaci_cmh.h"
  #include "psa_gmm.h"
  #include "psa_sm.h"
  #include "psa_gppp.h"
  #include "psa_upm.h"
  #include "psa_snd.h"
#endif  /* GPRS */

#ifdef UART
#include "psa_uart.h"
#include "cmh_uart.h"
#endif

#ifdef FF_ATI
  #include "aci_io.h"
#endif

#ifdef FF_PSI
#include "psa_psi.h"
#include "cmh_psi.h"
#endif /*FF_PSI*/

#ifdef FAX_AND_DATA
  #include "aci_fd.h"
  #include "psa_ra.h"
  #include "psa_l2r.h"
#ifdef FF_FAX
  #include "psa_t30.h"
#endif
#endif

#if defined (FF_WAP) || defined (FF_PPP) || defined(FF_GPF_TCPIP)|| defined (FF_SAT_E)
  #include "psa_ppp_w.h"
#endif
#if defined (CO_UDP_IP) || defined (FF_GPF_TCP_IP)
  #include "psa_tcpip.h"
#endif /* defined (CO_UDP_IP) || defined(FF_GPF_TCPIP) */

#include "cmh.h"

#ifdef SIM_TOOLKIT
  #include "psa_sat.h"
#endif

#ifdef MFW
  #include "mfw_acie.h"
#endif

#include "gdi.h"
#include "audio.h"
#include "rx.h"
#include "pwr.h"


#ifdef BT_ADAPTER
  #include "bti.h"
  #include "bti_aci.h"
#endif

#ifdef FF_PKTIO
  #include "psa_pktio.h"
#endif
#ifdef _SIMULATION_
  #include "ati_src_tst.h"
#endif

#if defined FF_EOTD
  #include "ati_src_lc.h"
#endif

#ifdef DTI
  #include "sap_dti.h"
#endif

#include "psa_ss.h"
#ifdef FF_ESIM
#include "psa_aaa.h" /* esim */
#endif

#ifdef FF_MMI_RIV
EXTERN void acia_init(void);
#endif

#ifdef _SIMULATION_
#ifdef FF_ATI_BAT
#include "ati_bat.h"
#endif
#endif


/*==== DEFINE =====================================================*/

/*==== EXPORT =====================================================*/

GLOBAL UBYTE mode = 1;
/*
 * instance data base
 */

/*==== PRIVATE ====================================================*/

LOCAL void pei_not_supported (void *data);

/*==== VARIABLES ==================================================*/
LOCAL BOOL              first_access = TRUE;
LOCAL T_MONITOR         aci_mon;

#if defined(FF_ATI) AND defined(SIM_TOOLKIT)
EXTERN UBYTE run_at_id; 
#endif

/*==== FUNCTIONS ==================================================*/

#ifdef SMI
  EXTERN T_PEI_RETURN _pei_init (void);
  EXTERN T_PEI_RETURN _pei_primitive (T_PRIM *prim);
  EXTERN T_PEI_RETURN _pei_exit (void);
  EXTERN T_PEI_RETURN _pei_monitor (void ** monitor);
/* Implements Measure#36 */
#if !defined (NCONFIG)
  EXTERN T_PEI_RETURN _pei_config ( T_PEI_CONFIG inString );
#endif /* NCONFIG */
#endif /* SMI */
  EXTERN void tim_exec_timeout (USHORT index);

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)             MODULE  : RLP_PEI             |
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
 *
 * Use MAK_FUNC_0 for primitives which contains no SDU.
 *
 * Use MAK_FUNC_S for primitives which contains a SDU.
 */

/*
 * jumptable for the entity service access point. Contains
 * the processing-function addresses and opcodes of
 * request and response primitives.
 *
 */
LOCAL const T_FUNC sim_table[] = {
  MAK_FUNC_0( psa_sim_read_cnf,               SIM_READ_CNF              ),
  MAK_FUNC_0( psa_sim_update_cnf,             SIM_UPDATE_CNF            ),
  MAK_FUNC_0( psa_sim_read_record_cnf,        SIM_READ_RECORD_CNF       ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_0( psa_sim_update_record_cnf,      SIM_UPDATE_RECORD_CNF     ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_0( psa_sim_increment_cnf,          SIM_INCREMENT_CNF         ),
  MAK_FUNC_0( psa_sim_verify_pin_cnf,         SIM_VERIFY_PIN_CNF        ),
  MAK_FUNC_0( psa_sim_change_pin_cnf,         SIM_CHANGE_PIN_CNF        ),
  MAK_FUNC_0( psa_sim_disable_pin_cnf,        SIM_DISABLE_PIN_CNF       ),
  MAK_FUNC_0( psa_sim_enable_pin_cnf,         SIM_ENABLE_PIN_CNF        ),
  MAK_FUNC_0( psa_sim_unblock_cnf,            SIM_UNBLOCK_CNF           ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_0( psa_sim_mmi_insert_ind,         SIM_MMI_INSERT_IND        ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_0( psa_sim_remove_ind,             SIM_REMOVE_IND            ),
  MAK_FUNC_0( psa_sim_sync_cnf,               SIM_SYNC_CNF              ),
  MAK_FUNC_0( psa_sim_activate_cnf,           SIM_ACTIVATE_CNF          ),
  MAK_FUNC_N( pei_not_supported,              0                         ),/*20*/
#ifdef SIM_TOOLKIT
  MAK_FUNC_0( psa_sim_toolkit_ind,            SIM_TOOLKIT_IND           ),
  MAK_FUNC_0( psa_sim_toolkit_cnf,            SIM_TOOLKIT_CNF           ),
#else
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
#endif
  MAK_FUNC_0( psa_sim_activate_ind,           SIM_ACTIVATE_IND          ),/*23*/
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_0( psa_sim_access_cnf,             SIM_ACCESS_CNF            ),
#ifdef SIM_TOOLKIT
  MAK_FUNC_0( psa_sim_file_update_ind,        SIM_FILE_UPDATE_IND       ),
#else
  MAK_FUNC_N( pei_not_supported,              0                         ),
#endif
  MAK_FUNC_N( pei_not_supported,              0                         ),/* SIM_GMM_INSERT_IND */
#ifdef FF_SAT_E
  MAK_FUNC_0( psa_sim_dti_cnf,                SIM_DTI_CNF               ),/*28*/
  MAK_FUNC_0( psa_sim_bip_cnf,                SIM_BIP_CNF               ),/*29*/
  MAK_FUNC_0( psa_sim_bip_config_cnf,         SIM_BIP_CONFIG_CNF        ),/*30*/
  MAK_FUNC_0( psa_sim_dti_bip_ind,            SIM_DTI_BIP_IND           ),/*31*/
  MAK_FUNC_0( psa_sim_eventlist_cnf,          SIM_EVENTLIST_CNF         ),/*32*/
#else
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
#endif
  MAK_FUNC_N( pei_not_supported,              0                         ) /*33*/
};

LOCAL const T_FUNC mmr_table[] = {
  MAK_FUNC_0( psa_mmr_reg_cnf,                MMR_REG_CNF               ),
  MAK_FUNC_0( psa_mmr_nreg_ind,               MMR_NREG_IND              ),
  MAK_FUNC_0( psa_mmr_nreg_cnf,               MMR_NREG_CNF              ),
  MAK_FUNC_0( psa_mmr_plmn_ind,               MMR_PLMN_IND              ),
  MAK_FUNC_0( psa_mmr_info_ind,               MMR_INFO_IND              ),
  MAK_FUNC_0( psa_mmr_ciphering_ind,          MMR_CIPHERING_IND         ),
  MAK_FUNC_0( psa_mmr_ahplmn_ind,             MMR_AHPLMN_IND            )
};

LOCAL const T_FUNC mncc_table[] = {
  MAK_FUNC_0( psa_mncc_alert_ind,             MNCC_ALERT_IND            ),
  MAK_FUNC_0( psa_mncc_call_proceed_ind,      MNCC_CALL_PROCEED_IND     ),
  MAK_FUNC_0( psa_mncc_disconnect_ind,        MNCC_DISCONNECT_IND       ),
  MAK_FUNC_0( psa_mncc_hold_cnf,              MNCC_HOLD_CNF             ),
  MAK_FUNC_0( psa_mncc_modify_cnf,            MNCC_MODIFY_CNF           ),
  MAK_FUNC_0( psa_mncc_modify_ind,            MNCC_MODIFY_IND           ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_0( psa_mncc_progress_ind,          MNCC_PROGRESS_IND         ),
  MAK_FUNC_0( psa_mncc_reject_ind,            MNCC_REJECT_IND           ),
  MAK_FUNC_0( psa_mncc_release_cnf,           MNCC_RELEASE_CNF          ),
  MAK_FUNC_0( psa_mncc_release_ind,           MNCC_RELEASE_IND          ),
  MAK_FUNC_0( psa_mncc_retrieve_cnf,          MNCC_RETRIEVE_CNF         ),
  MAK_FUNC_0( psa_mncc_setup_cnf,             MNCC_SETUP_CNF            ),
  MAK_FUNC_0( psa_mncc_setup_compl_ind,       MNCC_SETUP_COMPL_IND      ),
  MAK_FUNC_0( psa_mncc_setup_ind,             MNCC_SETUP_IND            ),
  MAK_FUNC_0( psa_mncc_start_dtmf_cnf,        MNCC_START_DTMF_CNF       ),
  MAK_FUNC_0( psa_mncc_sync_ind,              MNCC_SYNC_IND             ),
  MAK_FUNC_0( psa_mncc_user_ind,              MNCC_USER_IND             ),
  MAK_FUNC_0( psa_mncc_facility_ind,          MNCC_FACILITY_IND         ),
  MAK_FUNC_0( psa_mncc_bearer_cap_cnf,        MNCC_BEARER_CAP_CNF       ),
  MAK_FUNC_0( psa_mncc_prompt_ind,            MNCC_PROMPT_IND           ),
  MAK_FUNC_0( psa_mncc_recall_ind,            MNCC_RECALL_IND           ),
  MAK_FUNC_0( psa_mncc_status_ind,            MNCC_STATUS_IND           )
};

LOCAL const T_FUNC mnss_table[] = {
  MAK_FUNC_0( psa_mnss_begin_ind,             MNSS_BEGIN_IND            ),
  MAK_FUNC_0( psa_mnss_facility_ind,          MNSS_FACILITY_IND         ),
  MAK_FUNC_0( psa_mnss_end_ind,               MNSS_END_IND              )
};

LOCAL const T_FUNC mnsms_table[] = {
  MAK_FUNC_0( psa_mnsms_delete_cnf,           MNSMS_DELETE_CNF          ),
  MAK_FUNC_0( psa_mnsms_read_cnf,             MNSMS_READ_CNF            ),
  MAK_FUNC_0( psa_mnsms_store_cnf,            MNSMS_STORE_CNF           ),
  MAK_FUNC_0( psa_mnsms_submit_cnf,           MNSMS_SUBMIT_CNF          ),
  MAK_FUNC_0( psa_mnsms_command_cnf,          MNSMS_COMMAND_CNF         ),
#ifdef REL99
  MAK_FUNC_0( psa_mnsms_retrans_cnf,          MNSMS_RETRANS_CNF         ),
#else
  MAK_FUNC_N( pei_not_supported,              0                         ),
#endif /* REL99 */
  MAK_FUNC_0( psa_mnsms_report_ind,           MNSMS_REPORT_IND          ),
  MAK_FUNC_0( psa_mnsms_status_ind,           MNSMS_STATUS_IND          ),
  MAK_FUNC_0( psa_mnsms_message_ind,          MNSMS_MESSAGE_IND         ),
  MAK_FUNC_0( psa_mnsms_error_ind,            MNSMS_ERROR_IND           ),
#ifdef TI_PS_FF_AT_P_CMD_CPRSM
  MAK_FUNC_0( psa_mnsms_resume_cnf,           MNSMS_RESUME_CNF          ),
  MAK_FUNC_0( psa_mnsms_query_cnf,            MNSMS_QUERY_CNF           ),
#else
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
#endif
#ifdef GPRS
  MAK_FUNC_0( psa_mnsms_mo_serv_cnf,          MNSMS_MO_SERV_CNF         ),
#else
  MAK_FUNC_N( pei_not_supported,              0                         ),
#endif /* GPRS */
  MAK_FUNC_0( psa_mnsms_OTA_message_ind,      MNSMS_OTA_MESSAGE_IND    ),
#ifdef REL99
  MAK_FUNC_0( psa_mnsms_send_prog_ind,       MNSMS_SEND_PROG_IND        )
#else
  MAK_FUNC_N( pei_not_supported,              0                         )
#endif

 };

LOCAL const T_FUNC mmi_table[] = {
  MAK_FUNC_0( psa_mmi_keypad_ind,             MMI_KEYPAD_IND            ),
  MAK_FUNC_0( psa_mmi_cbch_ind,               MMI_CBCH_IND              ),
  MAK_FUNC_0( psa_mmi_rxlev_ind,              MMI_RXLEV_IND             ),
  MAK_FUNC_0( psa_mmi_battery_ind,            MMI_BATTERY_IND           ),
#ifdef SIM_TOOLKIT
  MAK_FUNC_0( psa_sat_cbch_dnl_ind,           MMI_SAT_CBCH_DWNLD_IND    ),
#else
  MAK_FUNC_N( pei_not_supported,              0                         ),
#endif
#ifdef BTE_MOBILE
  MAK_FUNC_0( psa_mmi_bt_cb_notify_ind,       MMI_BT_CB_NOTIFY_IND      ),
#else
  MAK_FUNC_N( pei_not_supported,              0                         ),
#endif
#ifdef RVM_GIL_SWE
  MAK_FUNC_N( pei_not_supported,              0                         ), /* RPD MSG ? */
  MAK_FUNC_0( gil_gpf_dispatch_message,       MMI_GIL_IND               ),
#else
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
#endif /* RVM_GIL_SWE */
#ifndef VOCODER_FUNC_INTERFACE
  MAK_FUNC_0( psa_mmi_tch_vocoder_cfg_con,    MMI_TCH_VOCODER_CFG_CON   )
#else
  MAK_FUNC_N( pei_not_supported,              0                         )
#endif
};

#ifdef DTI
LOCAL const T_FUNC dti_dl_table[] = {
  MAK_FUNC_0( dti_lib_dti_dti_connect_ind     ,    DTI2_CONNECT_IND    ),  /* 7700x */
  MAK_FUNC_0( dti_lib_dti_dti_connect_cnf     ,    DTI2_CONNECT_CNF    ),  /* 7701x */
  MAK_FUNC_0( dti_lib_dti_dti_disconnect_ind  ,    DTI2_DISCONNECT_IND ),  /* 7702x */
  MAK_FUNC_0( dti_lib_dti_dti_ready_ind       ,    DTI2_READY_IND      ),  /* 7703x */
  MAK_FUNC_0( dti_lib_dti_dti_data_ind        ,    DTI2_DATA_IND       ),  /* 7704x */
#if defined (_SIMULATION_)
  MAK_FUNC_S( dti_lib_dti_dti_data_test_ind   ,    DTI2_DATA_TEST_IND )
#else /* _SIMULATION_ */
  MAK_FUNC_N( pei_not_supported               ,    0)
#endif /* _SIMULATION_ */
};
#endif /* UART */

#ifdef FF_EM_MODE
LOCAL const T_FUNC em_table[] = {
  MAK_FUNC_0( psa_em_sc_info_cnf,             EM_SC_INFO_CNF            ), /* 0x7E00 */
#ifdef GPRS
  MAK_FUNC_0( psa_em_sc_gprs_info_cnf,        EM_SC_GPRS_INFO_CNF       ), /* 0x7E01 */
#else
  MAK_FUNC_N( pei_not_supported,              EM_SC_GPRS_INFO_CNF       ), /* 0x7E01 */
#endif /* GPRS */
  MAK_FUNC_0( psa_em_nc_info_cnf,             EM_NC_INFO_CNF            ), /* 0x7E02 */
  MAK_FUNC_0( psa_em_loc_pag_info_cnf,        EM_LOC_PAG_INFO_CNF       ), /* 0x7E03 */
  MAK_FUNC_0( psa_em_plmn_info_cnf,           EM_PLMN_INFO_CNF          ), /* 0x7E04 */
  MAK_FUNC_0( psa_em_cip_hop_dtx_info_cnf,    EM_CIP_HOP_DTX_INFO_CNF   ), /* 0x7E05 */
  MAK_FUNC_0( psa_em_power_info_cnf,          EM_POWER_INFO_CNF         ), /* 0x7E06 */
  MAK_FUNC_0( psa_em_identity_info_cnf,       EM_IDENTITY_INFO_CNF      ), /* 0x7E07 */
  MAK_FUNC_0( psa_em_sw_version_info_cnf,     EM_SW_VERSION_INFO_CNF    ), /* 0x7E08 */
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x7E09 */
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x7E0A */
  MAK_FUNC_0( em_event_trace_ind,             EM_DATA_IND               ), /* 0x7E0B */
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x7E0C */
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x7E0D */
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x7E0E */
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x7E0F */
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x7E10 */
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x7E11 */
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x7E12 */
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x7E13 */
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x7E14 */
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x7E15 */
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x7E16 */
#ifdef GPRS
  MAK_FUNC_0( psa_em_gmm_info_cnf,            EM_GMM_INFO_CNF           ), /* 0x7E17 */ /*GMM_INFO*/
  MAK_FUNC_0( psa_em_grlc_info_cnf,           EM_GRLC_INFO_CNF          ), /* 0x7E18 */ /*GRLC_INFO*/
#else
  MAK_FUNC_N( pei_not_supported,              EM_GMM_INFO_CNF           ), /* 0x7E17 */
  MAK_FUNC_N( pei_not_supported,              EM_GRLC_INFO_CNF          ), /* 0x7E18 */
#endif /* GPRS */
  MAK_FUNC_0( psa_em_amr_info_cnf,            EM_AMR_INFO_CNF           )  /* 0x7E19 */
};
#endif /* FF_EM_MODE */

#ifdef FF_ATI
/* ES!! #if !defined (MFW) */
LOCAL const T_FUNC aci_table[] = {
  MAK_FUNC_0( aci_aci_cmd_req,                ACI_CMD_REQ               ),
  MAK_FUNC_0( aci_aci_abort_req,              ACI_ABORT_REQ             ),
#ifdef BT_ADAPTER
  MAK_FUNC_0( aci_aci_cmd_res       ,         ACI_CMD_RES               ),
  MAK_FUNC_0( aci_aci_init_res      ,         ACI_INIT_RES              ),
  MAK_FUNC_0( aci_aci_deinit_req    ,         ACI_DEINIT_REQ            ),
  MAK_FUNC_0( aci_aci_open_port_req ,         ACI_OPEN_PORT_REQ         ),
  MAK_FUNC_0( aci_aci_close_port_req,         ACI_CLOSE_PORT_REQ        ),
  MAK_FUNC_0( aci_aci_cmd_req_bt,             ACI_CMD_REQ_BT            ),
  MAK_FUNC_0( aci_aci_cmd_res_bt,             ACI_CMD_RES_BT            ),
  MAK_FUNC_0( aci_aci_abort_req_bt,           ACI_ABORT_REQ_BT          ),
#else
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
#endif /* BT_ADAPTER */
  MAK_FUNC_0( aci_aci_trc_ind,                ACI_TRC_IND               ),
  MAK_FUNC_0( aci_aci_ext_ind,                ACI_EXT_IND               ),
#ifdef FF_MMI_RIV
  MAK_FUNC_0( aci_aci_riv_cmd_req,            ACI_RIV_CMD_REQ           )
#else
  MAK_FUNC_N( pei_not_supported,              0                         )
#endif /* FF_MMI_RIV */
};
/* ES!! #endif */
#endif

#ifdef FAX_AND_DATA

LOCAL const T_FUNC ra_table[] = {
  MAK_FUNC_N( pei_not_supported,              0 /* RA_READY_IND       */),
  MAK_FUNC_N( pei_not_supported,              0 /* RA_DATA_IND        */),
  MAK_FUNC_0( psa_ra_activate_cnf,            RA_ACTIVATE_CNF           ),
  MAK_FUNC_0( psa_ra_deactivate_cnf,          RA_DEACTIVATE_CNF         ),
  MAK_FUNC_N( pei_not_supported,              0 /* RA_BREAK_IND       */)
#ifdef FF_FAX
  ,
  MAK_FUNC_0( psa_ra_modify_cnf,              RA_MODIFY_CNF             )
#endif
};

#ifdef DTI
LOCAL const T_FUNC l2r_table[] = {
  MAK_FUNC_0( psa_l2r_activate_cnf,           L2R_ACTIVATE_CNF          ),
  MAK_FUNC_0( psa_l2r_deactivate_cnf,         L2R_DEACTIVATE_CNF        ),
  MAK_FUNC_0( psa_l2r_connect_cnf,            L2R_CONNECT_CNF           ),
  MAK_FUNC_0( psa_l2r_connect_ind,            L2R_CONNECT_IND           ),
  MAK_FUNC_0( psa_l2r_disc_cnf,               L2R_DISC_CNF              ),
  MAK_FUNC_0( psa_l2r_disc_ind,               L2R_DISC_IND              ),
  MAK_FUNC_N( pei_not_supported,              0 /* L2R_READY_IND      */),
  MAK_FUNC_N( pei_not_supported,              0 /* L2R_DATA_IND       */),
  MAK_FUNC_N( pei_not_supported,              0 /* L2R_BREAK_CNF      */),
  MAK_FUNC_N( pei_not_supported,              0 /* L2R_BREAK_IND      */),
  MAK_FUNC_N( psa_l2r_xid_ind,                L2R_XID_IND               ),
  MAK_FUNC_0( psa_l2r_error_ind,              L2R_ERROR_IND             ),
  MAK_FUNC_0( psa_l2r_reset_ind,              L2R_RESET_IND             ),
  MAK_FUNC_0( psa_l2r_statistic_ind,          L2R_STATISTIC_IND         ),
  MAK_FUNC_0( psa_l2r_dti_cnf,                L2R_DTI_CNF               ),
  MAK_FUNC_0( psa_l2r_dti_ind,                L2R_DTI_IND               )
};

#ifdef FF_FAX
LOCAL const T_FUNC t30_table[] = {
  MAK_FUNC_0( psa_t30_cap_ind,                T30_CAP_IND               ),
  MAK_FUNC_0( psa_t30_sgn_ind,                T30_SGN_IND               ),
  MAK_FUNC_0( psa_t30_cmpl_ind,               T30_CMPL_IND              ),
  MAK_FUNC_S( psa_t30_report_ind,             T30_REPORT_IND            ),
  MAK_FUNC_0( psa_t30_error_ind,              T30_ERROR_IND             ),
  MAK_FUNC_0( psa_t30_deactivate_cnf,         T30_DEACTIVATE_CNF        ),
  MAK_FUNC_0( psa_t30_activate_cnf,           T30_ACTIVATE_CNF          ),
  MAK_FUNC_0( psa_t30_preamble_ind,           T30_PREAMBLE_IND          ),
  MAK_FUNC_0( psa_t30_dti_ind,                T30_DTI_IND               ),
  MAK_FUNC_0( psa_t30_dti_cnf,                T30_DTI_CNF               ),
  MAK_FUNC_0( psa_t30_phase_ind,              T30_PHASE_IND             ),
  MAK_FUNC_0( psa_t30_eol_ind,                T30_EOL_IND               ),
  MAK_FUNC_N( pei_not_supported,              0                         )
};
#endif /* FF_FAX */

LOCAL const T_FUNC tra_table[] = {
  MAK_FUNC_0( psa_tra_activate_cnf,           TRA_ACTIVATE_CNF          ),
  MAK_FUNC_0( psa_tra_deactivate_cnf,         TRA_DEACTIVATE_CNF        ),
  MAK_FUNC_0( psa_tra_dti_cnf,                TRA_DTI_CNF               ),
  MAK_FUNC_0( psa_tra_dti_ind,                TRA_DTI_IND               ),
};
#endif /* DTI */
#endif /* FAX_AND_DATA */

#ifdef FF_GPF_TCPIP
LOCAL const T_FUNC tcpip_table[] = {
  MAK_FUNC_0 (psa_tcpip_initialize_cnf,       TCPIP_INITIALIZE_CNF ),  /* 0x00 */
  MAK_FUNC_N (psa_tcpip_shutdown_cnf,           TCPIP_SHUTDOWN_CNF ),  /* 0x01 */
  MAK_FUNC_0 (psa_tcpip_ifconfig_cnf,           TCPIP_IFCONFIG_CNF ),  /* 0x02 */
  MAK_FUNC_0 (psa_tcpip_dti_cnf,                     TCPIP_DTI_CNF )   /* 0x03 */
};
#endif

#ifdef CO_UDP_IP
LOCAL const T_FUNC udpa_table[] = {
  MAK_FUNC_0( psa_udpa_dti_cnf,               UDPA_DTI_CNF              ),
  MAK_FUNC_0( psa_udpa_dti_ind,               UDPA_DTI_IND              ),
  MAK_FUNC_0( psa_udpa_config_cnf,            UDPA_CONFIG_CNF           ),
};
LOCAL const T_FUNC ipa_table[] = {
  MAK_FUNC_0( psa_ipa_dti_cnf,                IPA_DTI_CNF               ),
  MAK_FUNC_0( psa_ipa_dti_ind,                IPA_DTI_IND               ),
  MAK_FUNC_0( psa_ipa_config_cnf,             IPA_CONFIG_CNF            ),
};
#endif /* CO_UDP_IP */

#ifdef FF_WAP
LOCAL const T_FUNC wap_table[] = {
  MAK_FUNC_S( psa_wap_mmi_ind,      WAP_MMI_IND         ),
  MAK_FUNC_S( psa_wap_mmi_req,      WAP_MMI_REQ         ),
  MAK_FUNC_S( psa_wap_mmi_cnf,      WAP_MMI_CNF         ),
  MAK_FUNC_0( psa_wap_dti_cnf,      WAP_DTI_CNF         ),
  MAK_FUNC_0( psa_wap_dti_ind,      WAP_DTI_IND         )
};
#endif /* FF_WAP */

#ifdef FF_GPF_TCPIP
LOCAL const T_FUNC dcm_table[] = {
  MAK_FUNC_0( psa_dcm_open_conn_req,            DCM_OPEN_CONN_REQ       ), /* 0x8000401c */
  MAK_FUNC_0( psa_dcm_close_conn_req,           DCM_CLOSE_CONN_REQ      ), /* 0x8001401c */
  MAK_FUNC_0( psa_dcm_get_current_conn_req ,    DCM_GET_CURRENT_CONN_REQ), /* 0x8002001c */
  MAK_FUNC_N( pei_not_supported,                0                       )
};
#endif /* FF_GPF_TCPIP */

/*LOCAL const T_FUNC ppp_table[] = {
  MAK_FUNC_0( psa_ppp_establish_cnf,          PPP_ESTABLISH_CNF         ),
  MAK_FUNC_0( psa_ppp_terminate_ind,          PPP_TERMINATE_IND         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_0( psa_ppp_dti_connected_ind,       PPP_DTI_CONNECTED_IND    )
};*/
#if defined (UART) AND defined (DTI)
LOCAL const T_FUNC uart_table[] = {
  MAK_FUNC_0( psa_uart_parameters_cnf,    UART_PARAMETERS_CNF   ), /* 0x3400 */
  MAK_FUNC_0( psa_uart_parameters_ind,    UART_PARAMETERS_IND   ), /* 0x3401 */
  MAK_FUNC_0( psa_uart_dti_cnf,           UART_DTI_CNF          ), /* 0x3402 */
  MAK_FUNC_0( psa_uart_dti_ind,  UART_DTI_IND ), /* 0x3403 */
  MAK_FUNC_0( psa_uart_disable_cnf,           UART_DISABLE_CNF          ), /* 0x3404 */
  MAK_FUNC_0( psa_uart_ring_cnf,              UART_RING_CNF             ), /* 0x3405 */
  MAK_FUNC_0( psa_uart_dcd_cnf,               UART_DCD_CNF              ), /* 0x3406 */
  MAK_FUNC_0( psa_uart_escape_cnf,            UART_ESCAPE_CNF           ), /* 0x3407 */
  MAK_FUNC_0( psa_uart_detected_ind,          UART_DETECTED_IND         ), /* 0x3408 */
  MAK_FUNC_0( psa_uart_error_ind,             UART_ERROR_IND            ), /* 0x3409 */
  MAK_FUNC_0( psa_uart_mux_start_cnf,         UART_MUX_START_CNF        ),
  MAK_FUNC_0( psa_uart_mux_dlc_establish_ind, UART_MUX_DLC_ESTABLISH_IND),
  MAK_FUNC_0( psa_uart_mux_dlc_release_ind,   UART_MUX_DLC_RELEASE_IND  ),
  MAK_FUNC_N( pei_not_supported,              0                         ), /* sleep ind */
  MAK_FUNC_N( pei_not_supported,              0                         ), /* wake up ind */
  MAK_FUNC_0( psa_uart_mux_close_ind,         UART_MUX_CLOSE_IND        ),
  MAK_FUNC_N( pei_not_supported,              0                         )
};
#endif

#ifdef GPRS
/* GMMREG */
LOCAL const T_FUNC gmm_table[] = {
  MAK_FUNC_0( psa_gmmreg_attach_cnf,           GMMREG_ATTACH_CNF        ),
  MAK_FUNC_0( psa_gmmreg_attach_rej,           GMMREG_ATTACH_REJ        ),
  MAK_FUNC_0( psa_gmmreg_detach_cnf,           GMMREG_DETACH_CNF        ),
  MAK_FUNC_0( psa_gmmreg_detach_ind,           GMMREG_DETACH_IND        ),
  MAK_FUNC_0( psa_gmmreg_plmn_ind,             GMMREG_PLMN_IND          ),
  MAK_FUNC_0( psa_gmmreg_suspend_ind,          GMMREG_SUSPEND_IND       ),
  MAK_FUNC_0( psa_gmmreg_resume_ind,           GMMREG_RESUME_IND        ),
  MAK_FUNC_0( psa_gmmreg_info_ind,             GMMREG_INFO_IND          ),
  MAK_FUNC_0( psa_gmmreg_ciphering_ind,        GMMREG_CIPHERING_IND     ),
  MAK_FUNC_0( psa_gmmreg_ahplmn_ind,           GMMREG_AHPLMN_IND        )
};  

/* UPM */
LOCAL const T_FUNC upm_table[] = {
  MAK_FUNC_N( pei_not_supported, 0          ),
  MAK_FUNC_N( psa_upm_count_cnf, UPM_COUNT_CNF )
};


/* SMREG */
LOCAL const T_FUNC sm_table[] = {
  MAK_FUNC_0( psa_smreg_pdp_activate_cnf,      SMREG_PDP_ACTIVATE_CNF   ),
  MAK_FUNC_0( psa_smreg_pdp_activate_rej,      SMREG_PDP_ACTIVATE_REJ   ),
  MAK_FUNC_0( psa_smreg_pdp_activate_ind,      SMREG_PDP_ACTIVATE_IND   ),
  MAK_FUNC_0( psa_smreg_pdp_deactivate_cnf,    SMREG_PDP_DEACTIVATE_CNF ),
  MAK_FUNC_0( psa_smreg_pdp_deactivate_ind,    SMREG_PDP_DEACTIVATE_IND ),
  MAK_FUNC_0( psa_smreg_pdp_modify_ind,        SMREG_PDP_MODIFY_IND     )
#ifdef REL99
  ,MAK_FUNC_0( psa_smreg_pdp_modify_cnf,       SMREG_PDP_MODIFY_CNF ),
  MAK_FUNC_0( psa_smreg_pdp_modify_rej,        SMREG_PDP_MODIFY_REJ ),
  MAK_FUNC_0( psa_smreg_pdp_activate_sec_cnf,  SMREG_PDP_ACTIVATE_SEC_CNF ),
  MAK_FUNC_0( psa_smreg_pdp_activate_sec_rej,  SMREG_PDP_ACTIVATE_SEC_REJ )
#endif /* !REL99 */
};


/* SN */
LOCAL const T_FUNC sndcp_table[] = {
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( psa_sn_dti_cnf,                 SN_DTI_CNF )
};

/* PPP */
/*LOCAL const T_FUNC ppp_table[] = {
  MAK_FUNC_0( psa_ppp_establish_cnf,           PPP_ESTABLISH_CNF        ),
  MAK_FUNC_0( psa_ppp_terminate_ind,           PPP_TERMINATE_IND        ),
  MAK_FUNC_0( psa_ppp_pdp_activate_ind,        PPP_PDP_ACTIVATE_IND     ),
  MAK_FUNC_0( psa_ppp_modification_cnf,        PPP_MODIFICATION_CNF     ),
  MAK_FUNC_0( psa_ppp_dti_connected_ind,       PPP_DTI_CONNECTED_IND    )
};*/

#ifdef FF_PKTIO
LOCAL const T_FUNC mnpkt_table[] = {
  MAK_FUNC_0( psa_pkt_connect_ind,             PKT_CONNECT_IND          ),
  MAK_FUNC_0( psa_pkt_disconnect_ind,          PKT_DISCONNECT_IND       ),
  MAK_FUNC_0( psa_pkt_dti_open_cnf,            PKT_DTI_OPEN_CNF         ),
  MAK_FUNC_0( psa_pkt_modify_cnf,              PKT_MODIFY_CNF           ),
  MAK_FUNC_0( psa_pkt_dti_close_cnf,           PKT_DTI_CLOSE_CNF        ),
  MAK_FUNC_0( psa_pkt_dti_close_ind,           PKT_DTI_CLOSE_IND        )
};
#endif
#endif  /* GPRS */
#if defined (FF_PSI ) AND defined (DTI)
LOCAL const T_FUNC mnpsi_table[] = {
  MAK_FUNC_0( psa_psi_conn_ind,                      PSI_CONN_IND             ),
  MAK_FUNC_0( psa_psi_disconn_ind,             PSI_DISCONN_IND          ),
  MAK_FUNC_0( psa_psi_dti_open_cnf,            PSI_DTI_OPEN_CNF         ),
  MAK_FUNC_0( psa_psi_dti_close_cnf,           PSI_DTI_CLOSE_CNF        ),
  MAK_FUNC_0( psa_psi_dti_close_ind,           PSI_DTI_CLOSE_IND        ),
  MAK_FUNC_0( psa_psi_setconf_cnf,             PSI_SETCONF_CNF          ),
  MAK_FUNC_0( psa_psi_line_state_cnf,          PSI_LINE_STATE_CNF       ),
  MAK_FUNC_0( psa_psi_line_state_ind,          PSI_LINE_STATE_IND       ),
  MAK_FUNC_0( psa_psi_close_cnf,               PSI_CLOSE_CNF            )
#ifdef _SIMULATION_
  ,
  MAK_FUNC_0( psa_psi_conn_ind_test,            PSI_CONN_IND_TEST)
#endif /*_SIMULATION_ */
};
#endif /*FF_PSI*/
/* PPP */

#if defined FF_WAP || defined GPRS || defined FF_GPF_TCPIP || defined (FF_SAT_E) || defined (FF_PPP)
    
LOCAL const T_FUNC ppp_table[] = {
  MAK_FUNC_0( psa_ppp_establish_cnf,           PPP_ESTABLISH_CNF        ),
  MAK_FUNC_0( psa_ppp_terminate_ind,           PPP_TERMINATE_IND        ),
  MAK_FUNC_0( psa_ppp_pdp_activate_ind,        PPP_PDP_ACTIVATE_IND     ),
  MAK_FUNC_0( psa_ppp_modification_cnf,        PPP_MODIFICATION_CNF     ),
  MAK_FUNC_0( psa_ppp_dti_connected_ind,       PPP_DTI_CONNECTED_IND    )
};
#endif /* FF_WAP || GPRS || FF_GPF_TCPIP || FF_SAT_E */

#if defined FF_EOTD
LOCAL const T_FUNC mnlc_table[] = {
  MAK_FUNC_0( psa_mnlc_sms_meas_cnf,             MNLC_SMS_MEAS_CNF        )
};
#endif /* FF_EOTD */

/* BTI */
#ifdef BT_ADAPTER
LOCAL const T_FUNC btp_table[] = {
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_0( psa_btp_dti_ind,                T30_DTI_IND               ),
  MAK_FUNC_0( psa_btp_dti_cnf,                T30_DTI_CNF               )
};
#endif /* BT_ADAPTER */

#if defined(FF_TCP_IP) || defined(FF_ESIM)
LOCAL const T_FUNC aaa_table[] = {
  MAK_FUNC_0( psa_aaa_cmd_req,                AAA_CMD_REQ               ),
  MAK_FUNC_0( psa_aaa_open_port_req,          AAA_OPEN_PORT_REQ         ),
  MAK_FUNC_0( psa_aaa_close_port_req,         AAA_CLOSE_PORT_REQ        ),
  MAK_FUNC_0( psa_aaa_dti_rsp,                AAA_DTI_RES               ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_N( pei_not_supported,              0                         ),
  MAK_FUNC_0( psa_aaa_disconnect_rsp,         AAA_DISCONNECT_RES        )
};
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)               MODULE  : RLP_PEI           |
| STATE   : code                         ROUTINE : pei_primitive     |
+--------------------------------------------------------------------+

  PURPOSE : Process protocol specific primitive.

*/

LOCAL SHORT pei_primitive (void *ptr)
{
  T_PRIM *prim = ptr;
#ifdef SMI
  T_PEI_RETURN subReturn = PEI_ERROR;
#endif

  /*
   *
   *                                  ACI                                       UPLINK
   *                                   |
   *   +-------------------------------v--------------------------------------+
   *   |                                                                      |
   *   |                              ACI                                     |
   *   |                                                                      |
   *   +--^----^-----^----^----^----^---^---^---^----^------^----^---^----^---+
   *      |    |     |    |    |    |   |   |   |    |      |    |   |    |
   *     SIM MMREG MNCC MNSS MNSMS MMI T30 L2R  RA GMMREG SMREG PPP UART BTI     DOWNLINK
   *      |    |     |    |    |    |   |   |   |    |      |    |   |    |
   *
   */

#ifdef MFW
  extern void mmeFlagHandler(void);
  extern USHORT mfwMmeDrvFlag;
#endif

/*  TRACE_FUNCTION ("pei_primitive()");*/
#ifdef MFW
  if (mfwMmeDrvFlag)
    mmeFlagHandler();
#endif

  if (prim NEQ NULL)
  {
    ULONG            opc = prim->custom.opc;
    USHORT           n;
    const T_FUNC    *table;

    VSI_PPM_REC ((T_PRIM_HEADER*)prim, __FILE__, __LINE__);

#ifndef FF_EM_MODE
    /*
     * To ensure true msc directions for the engineering mode additional
     * information (from which entity the prim is sent) must be provided
     * before PTRACE_IN is called.
    */
    PTRACE_IN (opc);
#endif /* FF_EM_MODE */

#ifdef MFW
    /*
     * MFW "checks" if it needs to handle primitves coming from below before they are
     * potentially still to be handled by ACI
     */
    if (aci_check_primitive (opc, (void*)(&(prim->data))))
    {
      PFREE (P2D(prim));
      return PEI_OK;
    }
#endif

 /*   TRACE_EVENT_P1("opcode: %d", opc);*/
    switch (SAP_NR(opc))
    {
#ifdef FF_ATI
/* ES!! #if !defined (MFW) */
      case ACI_DL:
      case ACI_UL:    table =  aci_table;   n = TAB_SIZE (aci_table);   break;
/* ES!! #endif */
#endif
      case SAP_NR(SIM_UL):   table =  sim_table;   n = TAB_SIZE (sim_table);   break;
      case SAP_NR(MMREG_DL): table =  mmr_table;   n = TAB_SIZE (mmr_table);   break;
      case SAP_NR(MNCC_DL):  table =  mncc_table;  n = TAB_SIZE (mncc_table);  break;
      case SAP_NR(MNSS_DL):  table =  mnss_table;  n = TAB_SIZE (mnss_table);  break;
      case SAP_NR(MNSMS_DL): table =  mnsms_table; n = TAB_SIZE (mnsms_table); break;
#if defined (FF_PKTIO) AND defined (DTI)
      case SAP_NR(PKT_DL):   table =  mnpkt_table; n = TAB_SIZE (mnpkt_table); break;
#endif
#if defined (FF_PSI) AND defined (DTI)
      case SAP_NR(PSI_DL):   table =  mnpsi_table; n = TAB_SIZE (mnpsi_table); break;
#endif /*FF_PSI*/

#ifdef DTI
#if defined(FF_TCP_IP) || defined(FF_ESIM)
      case SAP_NR(AAA_DL):   table =  aaa_table;   n = TAB_SIZE (aaa_table);   break;
#endif
#endif
      case MMI_DL:    table =  mmi_table;   n = TAB_SIZE (mmi_table);   break;
#ifdef DTI
      case DTI2_DL:
        {
          table = dti_dl_table;     n = TAB_SIZE (dti_dl_table);
          /*
           * to be able to distinguish DTI1/DTI2 opcodes,
           * the ones for DTI2 start at 0x50
           */
          opc -= 0x50;
        }
        break;
#endif /* UART */
#ifdef FF_EM_MODE
      case EM_Dl:             table =  em_table;    n = TAB_SIZE (em_table);    break;
#endif /* FF_EM_MODE */

#if defined (FAX_AND_DATA) AND defined (DTI)
      case SAP_NR(RA_DL):     table =  ra_table;    n = TAB_SIZE (ra_table);    break;
      case SAP_NR(L2R_DL):    table =  l2r_table;   n = TAB_SIZE (l2r_table);   break;
#ifdef FF_FAX
      case SAP_NR(T30_DL):    table =  t30_table;   n = TAB_SIZE (t30_table);   break;
#endif
      case SAP_NR(TRA_DL):    table =  tra_table;   n = TAB_SIZE (tra_table);   break;
#endif

#ifdef FF_GPF_TCPIP
      case SAP_NR(TCPIP_DL):  table = tcpip_table;  n = TAB_SIZE (tcpip_table); break;
      case SAP_NR(DCM_DL):    table = dcm_table;    n = TAB_SIZE (dcm_table);   break;
#endif

#ifdef CO_UDP_IP
      case SAP_NR(UDPA_DL):   table =  udpa_table;  n = TAB_SIZE (udpa_table);  break;
      case SAP_NR(IPA_DL):    table =  ipa_table;   n = TAB_SIZE (ipa_table);   break;
#endif

#ifdef FF_WAP
      case SAP_NR(WAP_DL):    table =  wap_table;   n = TAB_SIZE (wap_table);   break;
#endif

#if defined (FF_WAP) || defined (GPRS) || defined (FF_SAT_E)
      case PPP_UL:    table =  ppp_table;   n = TAB_SIZE (ppp_table);   break;
#endif

#ifdef GPRS
      case GMMREG_DL: table =  gmm_table;   n = TAB_SIZE (gmm_table);   break;
      case SMREG_DL:  table =  sm_table;    n = TAB_SIZE (sm_table);    break;
      case SAP_NR(UPM_DL):    table =  upm_table;   n = TAB_SIZE (upm_table);   break;
      case SAP_NR(SN_DL):     table =  sndcp_table; n = TAB_SIZE (sndcp_table); break;

#endif  /* GPRS */

#if defined (UART) AND defined (DTI)
      case UART_UL:   table =  uart_table;  n = TAB_SIZE (uart_table);  break;
#endif /* UART */
#ifdef BT_ADAPTER
      /* BTI DL */
      case BTP_G:     table =  btp_table;   n = TAB_SIZE (btp_table);   break;
#endif /* BT_ADAPTER */
#if defined FF_EOTD
      case SAP_NR(MNLC_DL):   table =  mnlc_table;  n = TAB_SIZE (mnlc_table);  break;
#endif

      default:        table =  NULL;        n = 0;                      break;
    }

#ifdef FF_EM_MODE
    /*
     * Only valid for the engineering mode as described above.
    */
    PTRACE_IN (opc);
#endif /* FF_EM_MODE */

    if (table NEQ NULL )
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

#if !defined (SMI) AND !defined (MFW)
        if (mode EQ 0 AND
            table NEQ aci_table)
        {
          PSENDX (ACI, P2D(prim));
        }
        else
#endif
        {
#ifdef SMI
          if (table->func EQ (T_VOID_FUNC) pei_not_supported)
          {
            subReturn = _pei_primitive (prim);
          }
          else
#endif
            JUMP (table->func) (P2D(prim));
        }
      }
      else
      {
#ifndef SMI
        pei_not_supported (P2D(prim));
#else
        subReturn = _pei_primitive (prim);
#endif
      }
      return PEI_OK;
    }
#ifdef SMI
    else
    {
      subReturn = _pei_primitive (prim);
    }
#endif

    /*
     * Primitive is no GSM Primitive
     * then forward to the environment
     */
#ifdef SMI
    if (subReturn EQ PEI_ERROR)
    {
#endif
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
#ifdef SMI
    }
#endif
  }
#ifdef SMI
  else
  {
    subReturn = _pei_primitive (prim);
  }
#endif

  return PEI_OK;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PEI            |
| STATE   : code                        ROUTINE : pei_init           |
+--------------------------------------------------------------------+

  PURPOSE : Initialize Protocol Stack Entity

*/
LOCAL SHORT pei_init (T_HANDLE handle)
{
#ifdef FF_ATI
#ifdef UART
  EXTERN void urt_init (void);
#endif
#endif

  aci_handle = handle;

  TRACE_FUNCTION ("pei_init()");

#ifdef TI_PS_HCOMM_CHANGE
  if (!cl_hcomm_open_all_handles())
  {
    return PEI_ERROR;
  }
    
  if (hCommACI < VSI_OK)
  {
    if ((hCommACI = vsi_c_open (VSI_CALLER ACI_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

#ifdef FAX_AND_DATA
  if (hCommTRA < VSI_OK)
  {
    /* TRA_NAME i.e "L2R" on account of TRA being no real entity */
    if ((hCommTRA = vsi_c_open (VSI_CALLER TRA_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif

#ifdef FF_GPF_TCPIP
  if (hCommTCPIP < VSI_OK)
  {
    if ((hCommTCPIP = vsi_c_open (VSI_CALLER TCPIP_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif

#if defined (CO_TCPIP_TESTAPP) || defined  (CO_BAT_TESTAPP)
  if (hCommAPP < VSI_OK)
  {
    if ((hCommAPP = vsi_c_open (VSI_CALLER APP_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif

#if defined FF_TRACE_OVER_MTST
  if (hCommMTST < VSI_OK)
  {
    if ((hCommMTST = vsi_c_open (VSI_CALLER "MTST")) < VSI_OK)
      return PEI_ERROR;
  }
#endif

#ifdef FF_ESIM
  if (hCommESIM < VSI_OK) /* open channel to ESIM entity */
  {
    if ((hCommESIM = vsi_c_open (VSI_CALLER ESIM_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif

#if defined (GPRS) AND defined (DTI)
  if (hCommGMM < VSI_OK)
  {
    if ((hCommGMM = vsi_c_open (VSI_CALLER GMM_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
  if (hCommSM < VSI_OK)
  {
    if ((hCommSM = vsi_c_open (VSI_CALLER SM_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
  if (hCommSNDCP < VSI_OK)
  {
    if ((hCommSNDCP = vsi_c_open (VSI_CALLER SNDCP_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

#ifdef FF_PKTIO
  if (hCommPKTIO < VSI_OK)
  {
    if ((hCommPKTIO = vsi_c_open (VSI_CALLER PKTIO_NAME)) < VSI_OK)
    {
      TRACE_EVENT ("cannot open PKTIO");
      return PEI_ERROR;
    }
  }
#endif
#endif /* GPRS */

#ifdef FF_GPF_TCPIP
  if (hCommDCM < VSI_OK)
  {
    if ((hCommDCM = vsi_c_open (VSI_CALLER DCM_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif /* FF_GPF_TCPIP */

#else /* for hCommHandles backward compatibility */
  if (hCommSIM < VSI_OK)
  {
    if ((hCommSIM = vsi_c_open (VSI_CALLER SIM_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

#ifdef FF_ESIM
  if (hCommESIM < VSI_OK) /* open channel to ESIM entity */
  {
    if ((hCommESIM = vsi_c_open (VSI_CALLER ESIM_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif
  if (hCommMM < VSI_OK)
  {
    if ((hCommMM = vsi_c_open (VSI_CALLER MM_NAME)) < VSI_OK)
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

#ifdef UART
  if (hCommUART < VSI_OK)
  {
    if ((hCommUART = vsi_c_open (VSI_CALLER UART_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif

  if (hCommPL < VSI_OK)
  {
    if ((hCommPL = vsi_c_open (VSI_CALLER PL_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

#if defined FF_EOTD
  if (hCommLC < VSI_OK)
  {
    if ((hCommLC = vsi_c_open (VSI_CALLER LC_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif

#ifdef FF_TCP_IP
/*#ifndef _SIMULATION_*/
  if (hCommAAA < VSI_OK)
  {
    if ((hCommAAA = vsi_c_open (VSI_CALLER AAA_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
/*#endif*/
#endif

#if defined FF_TRACE_OVER_MTST
  if (hCommMTST < VSI_OK)
  {
    if ((hCommMTST = vsi_c_open (VSI_CALLER "MTST")) < VSI_OK)
      return PEI_ERROR;
  }
#endif

  if (hCommACI < VSI_OK)
  {
    if ((hCommACI = vsi_c_open (VSI_CALLER ACI_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

#ifdef FAX_AND_DATA
#ifndef USE_L1FD_FUNC_INTERFACE
  if (hCommRA < VSI_OK)
  {
    if ((hCommRA = vsi_c_open (VSI_CALLER RA_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif
  if (hCommL2R < VSI_OK)
  {
    if ((hCommL2R = vsi_c_open (VSI_CALLER L2R_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

  if (hCommTRA < VSI_OK)
  {
    /* TRA_NAME i.e "L2R" on account of TRA being no real entity */
    if ((hCommTRA = vsi_c_open (VSI_CALLER TRA_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

#ifdef FF_FAX
  if (hCommT30 < VSI_OK)
  {
    if ((hCommT30 = vsi_c_open (VSI_CALLER T30_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif /* FF_FAX */
#endif /* FAX_AND_DATA */

#ifdef FF_GPF_TCPIP
  if (hCommTCPIP < VSI_OK)
  {
    if ((hCommTCPIP = vsi_c_open (VSI_CALLER TCPIP_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif

#if defined (CO_TCPIP_TESTAPP) || defined  (CO_BAT_TESTAPP)
  if (hCommAPP < VSI_OK)
  {
    if ((hCommAPP = vsi_c_open (VSI_CALLER APP_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif

#ifdef CO_UDP_IP
  if (hCommUDP < VSI_OK)
  {
    if ((hCommUDP = vsi_c_open (VSI_CALLER UDP_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
  if (hCommIP < VSI_OK)
  {
    if ((hCommIP = vsi_c_open (VSI_CALLER IP_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif 

#ifdef FF_WAP
  if (hCommWAP < VSI_OK)
  {
    if ((hCommWAP = vsi_c_open (VSI_CALLER WAP_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif

#if defined (FF_WAP) || defined (GPRS) || defined (FF_SAT_E) 
  if (hCommPPP < VSI_OK)
  {
    if ((hCommPPP = vsi_c_open (VSI_CALLER PPP_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif

#ifdef FF_EM_MODE
  if (hCommRR < VSI_OK)
  {
    if ((hCommRR = vsi_c_open (VSI_CALLER RR_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif /* FF_EM_MODE */

#if defined (GPRS) AND defined (DTI)
  if (hCommGMM < VSI_OK)
  {
    if ((hCommGMM = vsi_c_open (VSI_CALLER GMM_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
  if (hCommSM < VSI_OK)
  {
    if ((hCommSM = vsi_c_open (VSI_CALLER SM_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
  if (hCommUPM < VSI_OK)
  {
    if ((hCommUPM = vsi_c_open (VSI_CALLER UPM_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
  if (hCommSNDCP < VSI_OK)
  {
    if ((hCommSNDCP = vsi_c_open (VSI_CALLER SNDCP_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

#ifdef FF_PKTIO
  if (hCommPKTIO < VSI_OK)
  {
    if ((hCommPKTIO = vsi_c_open (VSI_CALLER PKTIO_NAME)) < VSI_OK)
    {
      TRACE_EVENT ("cannot open PKTIO");
      return PEI_ERROR;
    }
  }
#endif
#endif /* GPRS */
#ifdef FF_PSI
  if (hCommPSI < VSI_OK)
  {
    if ((hCommPSI = vsi_c_open (VSI_CALLER PSI_NAME)) < VSI_OK)
    {
      TRACE_EVENT ("cannot open PSI");
      return PEI_ERROR;
    }
  }
#endif /*FF_PSI*/
#ifdef BT_ADAPTER
  if (hCommBTI < VSI_OK)
  {
    if ((hCommBTI = vsi_c_open (VSI_CALLER BTI_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif /* BT_ADAPTER */

#ifdef FF_GPF_TCPIP
  if (hCommDCM < VSI_OK)
  {
    if ((hCommDCM = vsi_c_open (VSI_CALLER DCM_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif /* FF_GPF_TCPIP */

  if (hCommL1 < VSI_OK)
  {
  if ((hCommL1 = vsi_c_open (VSI_CALLER L1_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

#endif /* TI_PS_HCOMM_CHANGE */

#ifdef DTI
#ifdef _SIMULATION_
  /*
   * initialize dtilib for this entity
   */
  aci_hDTI = dti_init (
    4,
    handle,
    DTI_DEFAULT_OPTIONS,
    aci_pei_sig_callback
    );
#else
  /*
   * initialize dtilib for this entity
   */
  aci_hDTI = dti_init (
    ACI_INSTANCES * UART_INSTANCES,
    handle,
    DTI_DEFAULT_OPTIONS,
    aci_pei_sig_callback
    );
#endif
#endif /* DTI */

#ifdef DTI
  if(aci_hDTI EQ NULL)
  {
    TRACE_EVENT ("ACI DTI handle is 0");
    return PEI_ERROR;
  }
#endif

#ifdef SMI
  /*
   * initialize the slim man machine interface
   */
  if (_pei_init () EQ PEI_ERROR)
    return PEI_ERROR;
#endif

#ifdef ACI
  audio_Init ( NULL );
/* rx_Init( ) is not needed at the moment for ATI only version...
   causes the software not to start because it needs CST stack to be started first...
  rx_Init    ( NULL ); */
  pwr_Init   ( NULL );
#endif


  /*
   * Initialize BT_ADAPTER
   */
#if defined(FF_ATI) && defined(BT_ADAPTER)
  if (btiaci_init(aci_handle) EQ BTI_NAK)
    return PEI_ERROR;
  btiaci_at_init_req();
#endif

#ifdef FF_ATI
  init_ati ();
#endif /* FF_ATI */

#ifdef UART
  cmhUART_lst_init ();
#endif
#ifdef DTI  
  psaACI_Init();
  dti_cntrl_init();
#endif
#if defined (FF_PSI) AND defined (DTI)
  cmhPSI_lst_init();
#endif /*FF_PSI*/
  /*
   *  initialize CMH's
   */
  cmh_Init  ();
  cmh_Reset ( CMD_SRC_LCL, FALSE );
#ifdef FF_ATI
  cmh_Reset ( CMD_SRC_ATI_1, FALSE );
  cmh_Reset ( CMD_SRC_ATI_2, FALSE );
  cmh_Reset ( CMD_SRC_ATI_3, FALSE );
  cmh_Reset ( CMD_SRC_ATI_4, FALSE );
#ifdef SIM_TOOLKIT
  cmh_Reset ( CMD_SRC_ATI_5, FALSE );
#endif /* SIM_TOOLKIT */
#if defined FF_EOTD OR defined _SIMULATION_
  cmh_Reset ( CMD_SRC_ATI_6, FALSE );
#endif /* FF_EOTD */
#endif /* FF_ATI */

  /*
   *  initialize PSA's
   */
#if defined (SIM_TOOLKIT)
  psaSAT_Init();    /* has to be done first! */
#endif
  psaSS_Init();
#ifdef ACI
  psaMMI_Init();
#endif
  psaCC_Init();
  psaMM_Init();
  psaSIM_Init(ACI_INIT_TYPE_ALL);
  psaMMI_Init();
  psaSMS_Init();

#ifdef FAX_AND_DATA
  psaRA_Init();
  psaL2R_Init();
#ifdef FF_FAX
  psaT30_Init();
#endif /* FF_FAX */
#endif /* FAX_AND_DATA */

#ifdef GPRS
  psa_GPRSInit();
#endif /* GPRS */

#if defined (FF_WAP) || defined (FF_PPP) || defined(FF_GPF_TCPIP)|| defined (FF_SAT_E)
  psaPPP_Init();
#endif /* (FF_WAP) (FF_PPP) (FF_GPF_TCPIP) (FF_SAT_E) */

#if defined(CO_UDP_IP) || defined(FF_GPF_TCPIP)
  psaTCPIP_Init();
#endif /* CO_UDP_IP || FF_GPF_TCPIP */

#if defined(FF_GPF_TCPIP)
  dcm_init() ;
#endif /* FF_GPF_TCPIP */


#ifdef AT_ADAPTER
  psaBTI_Init();
#endif /* AT_ADAPTER */

  /*
   *  Initialize Message Coder Decoder
   */
  ccd_init ();
  /*
   * Initialize the UART module
   */
#ifdef FF_ATI
#ifdef UART
  urt_init ();
#endif
  ati_cmd_init ();
#endif

  /*
   * Initialize Advice of Charge Module
   */
  aoc_init_calltable ();

  /*
   * Initialize phonebook
   */
#ifdef TI_PS_FFS_PHB
  pb_init ();
#else
  phb_Init ();
#endif
  rdlPrm_init();

  /* The high level audio driver is initialised here */
  hl_audio_drv_init();

#if defined(_TARGET_)
  cmhCC_rd_mode_FFS(AUTOM_REP_NOT_PRESENT,READ_RDLmode); /* read redial mode from FFS */
#endif /* _TARGET_*/

#ifdef DTI
#ifdef _SIMULATION_
#ifdef FF_ATI_BAT
  ati_bat_reset();
#endif
  ati_src_tst_init( CMD_SRC_EXT );
#endif
#endif

#if defined FF_EOTD
  ati_src_lc_init( CMD_SRC_EXT );
#endif /*  FF_EOTD */

#ifdef MFW
  /*
   * initialise ACI extension handler
   * and start MMI Logic
   */
  aci_ext_init ();
  mmi_main ();
#endif

#ifdef FF_MMI_RIV
  acia_init();
#endif

  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PEI            |
| STATE   : code                        ROUTINE : pei_timeout        |
+--------------------------------------------------------------------+

  PURPOSE : Process timeout

*/

LOCAL SHORT pei_timeout (USHORT index)
{
  tim_exec_timeout (index);

  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)             MODULE  : ACI_PEI             |
| STATE   : code                       ROUTINE : mmi_pei_signal      |
+--------------------------------------------------------------------+

  PURPOSE : Functional interface to signal a primitive.

*/

LOCAL SHORT pei_signal ( ULONG  opc, void *primData )
{
#ifdef OPTION_SIGNAL
#ifndef _TMS470
  TRACE_FUNCTION ("pei_signal ()");
#endif /* _TMS470 */

  switch (opc)
  {
#ifdef FF_MMI_RIV
    case ACI_RIV_CMD_REQ:
      aci_aci_riv_cmd_req ((T_ACI_RIV_CMD_REQ *) primData);
      break;
#endif
#ifdef FAX_AND_DATA
    case RA_ACTIVATE_CNF:
      psa_ra_activate_cnf
      (
        (T_RA_ACTIVATE_CNF *) primData
      );
      break;
    case RA_DEACTIVATE_CNF:
      psa_ra_deactivate_cnf
      (
        (T_RA_DEACTIVATE_CNF *) primData
      );
      break;
#ifdef FF_FAX
    case RA_MODIFY_CNF:
      psa_ra_modify_cnf
      (
        (T_RA_MODIFY_CNF *) primData
      );
      break;
#endif /* FF_FAX */
#endif /* FAX_AND_DATA */

#ifdef FF_ATI
/* ES!! #if !defined (MFW) */
    case ACI_CMD_REQ:
      aci_aci_cmd_req
      (
        (T_ACI_CMD_REQ *) primData
      );
      break;
    case ACI_ABORT_REQ:
      aci_aci_abort_req
      (
        (T_ACI_ABORT_REQ *) primData
      );
      break;
#endif /* FF_ATI */
  }
#endif /* OPTION_SIGNAL */

  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PEI            |
| STATE   : code                        ROUTINE : pei_exit           |
+--------------------------------------------------------------------+

  PURPOSE : Close Resources and terminate

*/
LOCAL SHORT pei_exit (void)
{
  U8 i;

  TRACE_FUNCTION ("pei_exit()");

  /*
   * clean up communication
   */
#ifdef DTI
  /* close all open dti channels */
  dti_cntrl_close_all_connections();

  /*
   * Shut down dtilib communication
   */
  dti_deinit(aci_hDTI);
#endif

#ifdef TI_PS_HCOMM_CHANGE
  cl_hcomm_close_all_handles();

#ifdef FF_ESIM
  vsi_c_close (VSI_CALLER hCommESIM);
  hCommESIM = VSI_ERROR;
#endif

#ifdef UART
  vsi_c_close (VSI_CALLER hCommDTI);
  hCommDTI = VSI_ERROR;
#endif

  
#ifdef FF_TRACE_OVER_MTST
  vsi_c_close (VSI_CALLER hCommMTST);
  hCommMTST = VSI_ERROR;
#endif

#ifdef FF_GPF_TCPIP
  vsi_c_close (VSI_CALLER hCommTCPIP);
  hCommTCPIP = VSI_ERROR;
#endif

#if defined (CO_TCPIP_TESTAPP) || defined  (CO_BAT_TESTAPP)
  vsi_c_close (VSI_CALLER hCommAPP);
  hCommAPP = VSI_ERROR;
#endif

#ifdef GPRS
  vsi_c_close (VSI_CALLER hCommGMM);
  hCommGMM = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommSM);
  hCommSM = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommSNDCP);
  hCommSNDCP = VSI_ERROR;
#ifdef FF_PKTIO
  vsi_c_close (VSI_CALLER hCommPKTIO);
  hCommPKTIO = VSI_ERROR;
#endif
#endif /* GPRS */
#else
#ifdef FF_TRACE_OVER_MTST
  vsi_c_close (VSI_CALLER hCommMTST);
  hCommMTST = VSI_ERROR;
#endif

  vsi_c_close (VSI_CALLER hCommSIM);
  hCommSIM = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommSIM);
  hCommSIM = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommMM);
  hCommMM = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommCC);
  hCommCC = VSI_ERROR;

#ifdef FF_ESIM
  vsi_c_close (VSI_CALLER hCommESIM);
  hCommESIM = VSI_ERROR;
#endif

  vsi_c_close (VSI_CALLER hCommSS);
  hCommSS = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommSMS);
  hCommSMS = VSI_ERROR;

#ifdef UART
  vsi_c_close (VSI_CALLER hCommDTI);
  hCommDTI = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommUART);
  hCommUART = VSI_ERROR;
#endif
#ifdef FF_PSI
  vsi_c_close (VSI_CALLER hCommPSI);
  hCommPSI = VSI_ERROR;
#endif /*FF_PSI*/
/*#if defined SMI */
/*  vsi_c_close (VSI_CALLER hCommSMI);*/
/*  hCommSMI = VSI_ERROR;*/
/*#elif defined MFW */
/*  vsi_c_close (VSI_CALLER hCommMMI);*/
/*  hCommMMI = VSI_ERROR;*/
/*#elif defined ACI*/
  vsi_c_close (VSI_CALLER hCommACI);
  hCommACI = VSI_ERROR;
/*#endif*/

  vsi_c_close (VSI_CALLER hCommPL);
  hCommPL = VSI_ERROR;

#if defined FF_EOTD
  vsi_c_close (VSI_CALLER hCommLC);
  hCommLC = VSI_ERROR;
#endif

#ifdef FAX_AND_DATA
#ifndef USE_L1FD_FUNC_INTERFACE
  vsi_c_close (VSI_CALLER hCommRA);
  hCommRA = VSI_ERROR;
#endif
  vsi_c_close (VSI_CALLER hCommL2R);
  hCommL2R = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommTRA);
  hCommTRA = VSI_ERROR;

#ifdef FF_FAX
  vsi_c_close (VSI_CALLER hCommT30);
  hCommT30 = VSI_ERROR;
#endif /* FF_FAX */
#endif /* FAX_AND_DATA */

#ifdef FF_GPF_TCPIP
  vsi_c_close (VSI_CALLER hCommTCPIP);
  hCommTCPIP = VSI_ERROR;
#endif

#if defined (CO_TCPIP_TESTAPP) || defined  (CO_BAT_TESTAPP)
  vsi_c_close (VSI_CALLER hCommAPP);
  hCommAPP = VSI_ERROR;
#endif

#ifdef CO_UDP_IP
  vsi_c_close (VSI_CALLER hCommUDP);
  hCommUDP = VSI_ERROR;
  vsi_c_close (VSI_CALLER hCommIP);
  hCommIP = VSI_ERROR;
#endif

#ifdef FF_WAP
  vsi_c_close (VSI_CALLER hCommWAP);
  hCommWAP = VSI_ERROR;
#endif

#if defined (FF_WAP) || defined (GPRS) || defined (FF_SAT_E)
  vsi_c_close (VSI_CALLER hCommPPP);
  hCommPPP = VSI_ERROR;
#endif

#ifdef GPRS
  vsi_c_close (VSI_CALLER hCommGMM);
  hCommGMM = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommSM);
  hCommSM = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommUPM);
  hCommUPM = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommSNDCP);
  hCommSNDCP = VSI_ERROR;
#ifdef FF_PKTIO
  vsi_c_close (VSI_CALLER hCommPKTIO);
  hCommPKTIO = VSI_ERROR;
#endif
#endif /* GPRS */

#ifdef BT_ADAPTER
  vsi_c_close (VSI_CALLER hCommBTI);
  hCommBTI = VSI_ERROR;
#endif /* BT_ADAPTER */

  vsi_c_close (VSI_CALLER hCommL1);
  hCommL1 = VSI_ERROR;

#endif /* TI_PS_HCOMM_CHANGE */

#ifdef SMI
  _pei_exit ();
#endif

  /*
   * deallocate all channels
   * for non existing channels, this will just return
   */
 #ifdef FF_ATI
  for (i = 1; i < CMD_SRC_MAX; i++)
  {
    ati_finit(i);
  }
#endif /* FF_ATI */

  /* Free still occupied pointers in ccShrdPrm */
  for (i = 0; i < MAX_CALL_NR; i++)
  {
    if (ccShrdPrm.ctb[i] NEQ NULL)
    {
      psaCC_FreeCtbNtry (i);
    }
  }

  /* Deallocation of elements (2nd to last) in linked list PNN */
  while (mmShrdPrm.PNNLst.next NEQ NULL)
  {
    T_pnn_name *nextnext = mmShrdPrm.PNNLst.next->next;
    ACI_MFREE (mmShrdPrm.PNNLst.next);
    mmShrdPrm.PNNLst.next = nextnext;
  }

#ifdef DTI

#if defined(FF_ATI) AND defined(SIM_TOOLKIT)
     run_at_id = 0xFF;
#endif  /* FF_ATI*/

#ifdef _SIMULATION_
  ati_src_tst_finit();
#endif /* _SIMULATION_ */
#endif /* DTI */

#if defined FF_EOTD
  ati_src_lc_finit();
#endif /*  FF_EOTD */

  rdlPrm_exit();

#ifdef UART
  cmhUART_CleanComParameterList();
#endif


#ifdef FF_PSI
  cmhPSI_clean_all_elem();
  cmhPSI_CleanDcbParameterList();
#endif

#ifdef UART
  cmhUART_lst_exit();
#endif
  
  return PEI_OK;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PEI            |
| STATE   : code                        ROUTINE : pei_config         |
+--------------------------------------------------------------------+

  PURPOSE : Dynamic Configuration

*/

#if !defined (NCONFIG)

LOCAL const KW_DATA kwtab[] =
{
  ACI_KEY_SEQUENCE,     KEY_SEQUENCE,
  ACI_KEY_PRESS,        KEY_PRESS,
  ACI_KEY_RELEASE,      KEY_RELEASE,
  ACI_START_AOC,        START_AOC,
  ACI_CPOL_MODE,        CPOL_MODE,
  ACI_CPOL_IDX2,        CPOL_IDX2,
  ACI_ATI_VIA_TIF,      ATI_VIA_TIF,
/* new here*/
#ifdef _SIMULATION_
  ACI_DATA_INPUT,       DATA_INPUT,
#endif /* _SIMULATION_ */
  "",                   0
};

#endif

#ifdef _SIMULATION_

#ifdef UART
EXTERN void cmhUART_startConnection (UBYTE srcId,
                                     T_ACI_DEVICE_TYPE device_type);
#endif /* UART */
EXTERN UBYTE uart_new_source (UBYTE device, UBYTE dlci);
EXTERN void uart_new_source_for_aci_src_tst( UBYTE src_id, UBYTE device, UBYTE dlci );
#endif

/* Implements Measure#36 */
#if !defined (NCONFIG)
LOCAL SHORT pei_config (char *inString)
{
#ifdef FF_ATI
  BOOL alreadyCalled = FALSE;
#endif

  char    *s = inString;
  SHORT    valno;
  char    *keyw;
  char    *val[10];

  TRACE_FUNCTION ("pei_config()");

#ifdef UART
  if (!strcmp(inString, "UART_19200"))
  {
    cmhUART_SetDataRate((UBYTE) CMD_SRC_ATI, (T_ACI_BD_RATE)UART_IO_SPEED_19200);
    return PEI_OK;
  }
#endif

  if (!strcmp(inString, "ATI_TRC"))
  { /* AT%CPRIM="MMI","CONFIG ATI_TRC" */
    EXTERN BOOL _g_ati_trc_enabled;
    _g_ati_trc_enabled = TRUE; /* only switch on possible */
    return PEI_OK;
  }

#ifdef _SIMULATION_

#ifdef UART
  if (!strcmp(inString, "START_DTI"))
  {
    int i;
    UBYTE srcId;

    for (i=0;i<UART_INSTANCES;i++)
    {
      srcId = uart_new_source ((UBYTE)i, UART_DLCI_NOT_MULTIPLEXED);
      cmhUART_startConnection (srcId, DEVICE_TYPE_URT);
    }
    return PEI_OK;
  }
  if(!strcmp(inString, "EXPAND_ATI_SRC_TST"))
  {
    UBYTE srcId = ati_src_tst_get_src_id( (UBYTE) CMD_SRC_EXT );

    if ( srcId EQ DTI_MNG_ID_NOTPRESENT )
      return PEI_OK;

    uart_new_source_for_aci_src_tst( srcId, 0, NOT_PRESENT_8BIT );
    cmhUART_startConnection (srcId, DEVICE_TYPE_URT);
    return PEI_OK;
  }

  if(!strncmp(inString, "ADD_TST_SRC", 11)) /* parameter [UART] */
  {
    char *end = inString + 11;
    UBYTE srcId;

    srcId = ati_src_tst_get_src_id( (UBYTE) CMD_SRC_UNKNOWN );

    if ( srcId EQ DTI_MNG_ID_NOTPRESENT )
      return PEI_OK;

    while ( *end EQ ' ' )
      end ++;

    if ( !strcmp( end, "UART"))
    {
      uart_new_source_for_aci_src_tst( srcId, srcId, NOT_PRESENT_8BIT );
      cmhUART_startConnection (srcId, DEVICE_TYPE_URT);
    }

    if ( !strcmp( end, "UART_DP"))  /* for dual port testing */
    {
      static UBYTE device=0;  /* lint: at least init to something */
      uart_new_source_for_aci_src_tst( srcId, device, UART_DLCI_NOT_MULTIPLEXED );
      cmhUART_startConnection (srcId, DEVICE_TYPE_URT);
      device++;
    }

    return PEI_OK;
  }
#endif /* UART */

#ifdef DTI
  if (!strcmp(inString, "STOP_SRC_TST"))
  {
EXTERN void ati_src_tst_finit (void);
    ati_src_tst_finit ();
  }
#endif

#if (defined SMI OR defined MFW) AND defined TI_PS_FF_CONC_SMS
  if (!strcmp(inString, "CONC_SMS_TST"))
  {
EXTERN void concSMS_InitForTesting();
      concSMS_InitForTesting();
      return PEI_OK;
  }
#endif /*#if (defined SMI OR defined MFW) AND defined TI_PS_FF_CONC_SMS */

  if (!strcmp(inString, "ENABLE_VOCODER_TST"))
  {
EXTERN void hl_audio_drv_initForTest();
       hl_audio_drv_initForTest();
     return PEI_OK;
  }

#endif /* _SIMULATION_ */

#ifdef GPRS
    /*
     * Class modification
     * Author: ANS
     * initial: 07-Nov-00
     */
    if(!strcmp(inString,"CLASS_CC"))
    {
      default_mobile_class = GMMREG_CLASS_CC;
      return PEI_OK;
    }
    if(!strcmp(inString,"CLASS_CG"))
    {
      default_mobile_class = GMMREG_CLASS_CG;
      return PEI_OK;
    }
    if(!strcmp(inString,"CLASS_BC"))
    {
      default_mobile_class = GMMREG_CLASS_BC;
      return PEI_OK;
    }
    if(!strcmp(inString,"CLASS_BG"))
    {
      default_mobile_class = GMMREG_CLASS_BG;
      return PEI_OK;
    }
    if(!strcmp(inString,"CLASS_B"))
    {
      default_mobile_class = GMMREG_CLASS_B;
      return PEI_OK;
    }
    if(!strcmp(inString,"CLASS_A"))
    {
      default_mobile_class = GMMREG_CLASS_A;
      return PEI_OK;
    }
    /*
     * End Class modification
     */

    /*
     * Attach mode modification
     * Author: BRZ
     * initial: 14-Apr-01
     */
    if(!strcmp(inString,"AUTO_ATTACH"))
    {
      automatic_attach_mode = CGAATT_ATTACH_MODE_AUTOMATIC;
      return PEI_OK;
    }
    if(!strcmp(inString,"MAN_ATTACH"))
    {
      automatic_attach_mode = CGAATT_ATTACH_MODE_MANUAL;
      return PEI_OK;
    }
    if(!strcmp(inString,"AUTO_DETACH"))
    {
      automatic_detach_mode = CGAATT_DETACH_MODE_ON;
      return PEI_OK;
    }
    if(!strcmp(inString,"MAN_DETACH"))
    {
      automatic_detach_mode = CGAATT_DETACH_MODE_OFF;
      return PEI_OK;
    }
    /*
     * End Attach mode modification
     */
#endif

#ifdef MFW
    if (!strncmp("MFWMOB ",s,7))
    {
        mfwExtIn(s+7);
        return PEI_OK;
    }
#endif

#ifdef _SIMULATION_
#ifdef MFW
    /*
     * Only for Windows: define several variants
     * of SIM LOCK pcm content
     */
    if (!strncmp("SIMLOCK",s,7))
    {
      EXTERN UBYTE sim_lock_mode;

      sim_lock_mode = atoi (s+8);
      return PEI_OK;
    }
#endif
#endif

#ifdef FF_TIMEZONE
    /*
     * simulate an incoming MMR_INFO_IND
     */
    if (!strncmp("TIMEZONE_TEST",s,13))
    {
      TRACE_EVENT ("pei_config() TIMEZONE_TEST");
      {
        PALLOC (mmr_info_ind, MMR_INFO_IND); /* is freed in psa_mmr_info_ind */        
        memset (mmr_info_ind, 0, sizeof (T_MMR_INFO_IND)); /* clear out all */
        memcpy (mmr_info_ind->short_name.text, "RFT", 3);  /* pseudo short name */
        mmr_info_ind->short_name.v_name  = 1;
        mmr_info_ind->short_name.c_text  = 3;
        memcpy (mmr_info_ind->full_name.text, "Ramsch Fusch Tinneff", 20);  /* pseudo long name  */
        mmr_info_ind->full_name.v_name  = 1;
        mmr_info_ind->full_name.c_text  = 20;
        mmr_info_ind->ntz.v_tz          = 1;
        mmr_info_ind->ntz.tz            = 0x19;  /* simulate -2:45h away from GMT */
        mmr_info_ind->time.v_time       = 1;
        mmr_info_ind->time.day          = 30;    /* 30. February 2004 ;~) */
        mmr_info_ind->time.month        = 02;
        mmr_info_ind->time.year         = 04;
        mmr_info_ind->time.hour         = 11;
        mmr_info_ind->time.minute       = 55;    /* this stuff is really 5 before 12 */
        mmr_info_ind->time.second       = 59;
        mmr_info_ind->plmn.v_plmn       = 1;
        mmr_info_ind->plmn.mcc[0]       = 0x02;
        mmr_info_ind->plmn.mcc[1]       = 0x06;
        mmr_info_ind->plmn.mcc[2]       = 0x02;
        mmr_info_ind->plmn.mnc[0]       = 0x00;
        mmr_info_ind->plmn.mnc[1]       = 0x01;
        mmr_info_ind->plmn.mnc[2]       = 0x0F;
        
        psa_mmr_info_ind(mmr_info_ind);
        return PEI_OK;
      }
    }
#endif

  TRACE_EVENT_P1("pei_config() %s", s);

  tok_init(s);

  /*
   * Parse next keyword and number of variables
   */
  while ((valno = tok_next(&keyw,val)) NEQ TOK_EOCS)
  {
     switch (tok_key((KW_DATA *)kwtab,keyw))
    {
#ifdef MFW
      case KEY_SEQUENCE:
      {
        if (valno EQ 1)
        {
          mfw_keystroke (val[0]);
        }
        else
        {
          TRACE_ERROR ("[PEI_CONFIG]: Wrong Number of Parameters");
        }
        break;
      }
#endif

#ifdef MFW
      case KEY_PRESS:
      {
        if (valno EQ 1)
        {
          mfw_keystroke_long (val[0], 1);
        }
        else
        {
          TRACE_ERROR ("[PEI_CONFIG]: Wrong Number of Parameters");
        }
        break;
      }
#endif

#ifdef MFW
      case KEY_RELEASE:
      {
        if (valno EQ 1)
        {
          mfw_keystroke_long (val[0], 0);
        }
        else
        {
          TRACE_ERROR ("[PEI_CONFIG]: Wrong Number of Parameters");
        }
        break;
      }
#endif

      case START_AOC:
      {
        T_FWD_CHG_ADVICE_INV    charge;
        T_chargingInformation * aoc_para;

        aoc_para = &charge.forwardChargeAdviceArg.chargingInformation;

        TRACE_EVENT ("Set AoC Parameter");

        aoc_para->v_e1 =  TRUE;
        aoc_para->e1.c_e_val  = 1;
        aoc_para->e1.e_val[0] = 60;
        aoc_para->v_e2 =  TRUE;
        aoc_para->e2.c_e_val  = 1;
        aoc_para->e2.e_val[0] = 140;
        aoc_para->v_e3 =  TRUE;
        aoc_para->e3.c_e_val  = 1;
        aoc_para->e3.e_val[0] = 100;
        aoc_para->v_e4 =  TRUE;
        aoc_para->e4.c_e_val  = 1;
        aoc_para->e4.e_val[0] = 250;
        aoc_para->v_e5 =  FALSE;
        aoc_para->e5.c_e_val  = 0;
        aoc_para->e5.e_val[0] = 0;
        aoc_para->v_e6 =  FALSE;
        aoc_para->e6.c_e_val  = 0;
        aoc_para->e6.e_val[0] = 0;
        aoc_para->v_e7 =  TRUE;
        aoc_para->e7.c_e_val  = 2;
        aoc_para->e7.e_val[0] = 0x2;
        aoc_para->e7.e_val[1] = 0x58;

        aoc_parameter(0, &charge);
        aoc_info (0, AOC_START_AOC);
        break;
      }

#ifdef _SIMULATION_
      case CPOL_MODE:
      {
#ifdef FF_ATI
        if (valno EQ 1)
        {
          cpolMode = atoi(val[0]);
        }
        else
        {
          TRACE_ERROR ("[PEI_CONFIG]: Wrong Number of Parameters");
        }
#else
        TRACE_ERROR ("[PEI_CONFIG]: AT Command Interpreter Not Supported");
#endif
        break;
      }

      case CPOL_IDX2:
      {
#ifdef FF_ATI
        if (valno EQ 1)
        {
          cpolIdx2 = atoi(val[0]);
        }
        else
        {
          TRACE_ERROR ("[PEI_CONFIG]: Wrong Number of Parameters");
        }
#else
        TRACE_ERROR ("[PEI_CONFIG]: AT Command Interpreter Not Supported");
#endif
        break;
      }
#endif

#ifdef FF_ATI
#ifdef UART
      case ATI_VIA_TIF:
      {
        EXTERN BOOL atiViaTif;

        if (valno EQ 1)
        {
          if (strcmp (val[0], "ON") EQ 0)
            atiViaTif = TRUE;
          else if (strcmp (val[0], "OFF") EQ 0)
            atiViaTif = FALSE;
          else
            TRACE_ERROR ("[PEI_CONFIG]: Wrong Parameter Value");
        }
        else
        {
          TRACE_ERROR ("[PEI_CONFIG]: Wrong Number of Parameters");
        }
        break;
      }
#endif /* UART */
#endif

      default:
      {
#ifdef FF_ATI
        if ( ( toupper ( inString[0] ) ) EQ 'A' AND
             ( toupper ( inString[1] ) ) EQ 'T'     )
        {
          USHORT len = 0;

          /*
           *---------------------------------------------------------
           * parsing algorithm is not suitable for transparent data
           * transfer, call aciCommand only once
           *---------------------------------------------------------
           */
          if (!alreadyCalled)
          {
            while (inString[len] NEQ ' ' AND inString[len] NEQ '\0')
              len++;

            inString[len] = '\0';

            ati_execute_config_cmd ((UBYTE*)inString, len);
          }

          alreadyCalled = TRUE;
        }
        else
#endif
        {
#ifdef SMI
          return _pei_config (inString);
#else
          TRACE_ERROR ("[PEI_CONFIG]: Illegal Keyword");
#endif
        }
        break;
      }
    }
  }

  return PEI_OK;
}
#endif /* NCONFIG */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PEI            |
| STATE   : code                        ROUTINE : pei_monitor        |
+--------------------------------------------------------------------+

  PURPOSE : Monitoring of physical Parameters

*/
LOCAL SHORT pei_monitor (void ** monitor)
{
  TRACE_FUNCTION ("pei_monitor()");

#ifdef SMI
  _pei_monitor (monitor);
#else
/* Implements Measure#32: Row 41 */
#endif

  *monitor = &aci_mon;

  return PEI_OK;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PEI            |
| STATE   : code                        ROUTINE : pei_create         |
+--------------------------------------------------------------------+

  PURPOSE : Create the Protocol Stack Entity

*/

GLOBAL SHORT aci_pei_create (T_PEI_INFO **info)
{
  static const T_PEI_INFO pei_info =
  {
    ACI_NAME,
    {
      pei_init,
#ifdef _SIMULATION_
      pei_exit,
#else
      NULL,
#endif
      pei_primitive,
      pei_timeout,
      pei_signal,
      NULL,             /* no run function     */
/* Implements Measure#36 */
#if defined(NCONFIG)
      NULL,             /* no pei_config function     */
#else /* NCONFIG */
      pei_config,
#endif /* NCONFIG */
      pei_monitor,
    },
#if defined (ACI)
    3072,     /* Stack Size      */
#endif
#if defined (SMI)
    3072,     /* Stack Size      */
#endif
/* Increased 300 bytes because when DCM is used for GPRS call 
   more number of bytes need in the stack */
#if defined (MFW)
    4396,     /* Stack Size      */
#endif
#if defined (FF_MMI_RIV)
    3072,     /* Stack Size      */
#endif
    20,       /* Queue Entries   */
#if defined (ACI)
    100,      /* Priority        */
#endif
#if defined (SMI)
    100,      /* Priority        */
#endif
#if defined (MFW)
    100,      /* Priority        */
#endif
#if defined (FF_MMI_RIV)
    100,      /* Priority        */
#endif

    MAX_ACI_TIMER, /* number of timer */
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


#endif  /* ACI_PEI_C */
