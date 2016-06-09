/*
+-----------------------------------------------------------------------------
|  Project :  GSM-F&D (8411)
|  Modul   :  SIM_PEI
+-----------------------------------------------------------------------------
|  Copyright 2004 Texas Instruments
|                 All rights reserved.
|
|                 This file is confidential and a trade secret of Texas
|                 Instruments
|                 The receipt of or possession of this file does not convey
|                 any rights to reproduce or disclose its contents or to
|                 manufacture, use, or sell anything it may describe, in
|                 whole, or in part, without the specific written consent of
|                 Texas Instruments.
+-----------------------------------------------------------------------------
|  Purpose :  This Modul defines the process body interface
|             for the component SIM of the mobile station
+-----------------------------------------------------------------------------
*/

#ifndef SIM_PEI_C
#define SIM_PEI_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_SIM

/*==== INCLUDES ===================================================*/

#include <string.h>
#include "typedefs.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_sim.h"
#include "mon_sim.h"
#include "pei.h"
#include "tok.h"
#include "sim.h"
#include "sim_em.h"
#include "cl_shrd.h"

#ifdef TI_PS_UICC_CHIPSET_15
#include "8010_136_SIMDRV_SAP_inline.h"
#endif


/*==== EXPORT =====================================================*/
#ifdef TI_PS_HCOMM_CHANGE
#ifdef FF_SAT_E
T_HANDLE hCommUDP = VSI_ERROR;           /* UDP Communication        */
#endif /* FF_SAT_E */
#if defined (GPRS)
T_HANDLE hCommGMM = VSI_ERROR;           /* SMS Communication        */
#endif /* GPRS */
T_HANDLE sim_handle;
#else
T_HANDLE hCommMMI = VSI_ERROR;           /* MMI Communication        */
T_HANDLE hCommMM  = VSI_ERROR;           /* MM  Communication        */
T_HANDLE hCommSMS = VSI_ERROR;           /* SMS Communication        */
#ifdef FF_SAT_E
T_HANDLE hCommUDP = VSI_ERROR;           /* UDP Communication        */
#endif /* FF_SAT_E */
#ifdef _SIMULATION_
T_HANDLE hCommSIM = VSI_ERROR;           /* TEST CASES: TAP Communication */
#endif
#if defined (GPRS)
T_HANDLE hCommGMM = VSI_ERROR;           /* SMS Communication        */
#endif /* GPRS */
T_HANDLE sim_handle;
#endif /* TI_PS_HCOMM_CHANGE */


#ifdef OPTION_RELATIVE
ULONG offset;
#endif

T_SIM_DATA sim_data;

/*==== PRIVATE ====================================================*/

#ifdef FF_SAT_E
#define pei_dti_getdata_req        _ENTITY_PREFIXED(pei_dti_getdata_req)
#define pei_dti_disconnect_ind     _ENTITY_PREFIXED(pei_dti_disconnect_ind)
#define pei_dti_ready_ind          _ENTITY_PREFIXED(pei_dti_ready_ind)
#define pei_dti_data_req           _ENTITY_PREFIXED(pei_dti_data_req)
#define pei_dti_data_ind           _ENTITY_PREFIXED(pei_dti_data_ind)
#define pei_dti_callback           _ENTITY_PREFIXED(pei_dti_callback)
#define pei_dti_data_test_req      _ENTITY_PREFIXED(pei_dti_data_test_req)
#define pei_dti_data_test_ind      _ENTITY_PREFIXED(pei_dti_data_test_ind)
#endif /* FF_SAT_E */

static void pei_not_supported (void * data);
#ifdef SIM_TOOLKIT
static void sim_timeout (USHORT index);
#ifdef FF_SAT_E
LOCAL void pei_dti_getdata_req(T_DTI2_GETDATA_REQ* dti_getdata_req);
LOCAL void pei_dti_disconnect_ind(T_DTI2_DISCONNECT_IND *dti_disconnect_ind);
LOCAL void pei_dti_ready_ind(T_DTI2_READY_IND* dti_ready_ind);
LOCAL void pei_dti_data_req(T_DTI2_DATA_REQ* dti_data_req);
LOCAL void pei_dti_data_ind(T_DTI2_DATA_IND* dti_data_ind);
LOCAL void pei_dti_callback(UBYTE instance,
                            UBYTE interfac,
                            UBYTE channel,
                            UBYTE reason,
                            T_DTI2_DATA_IND* dti_data_ind);
#ifdef _SIMULATION_
LOCAL const void pei_dti_data_test_req(T_DTI2_DATA_TEST_REQ* dti_data_test_req);
LOCAL const void pei_dti_data_test_ind(T_DTI2_DATA_TEST_IND* dti_data_test_ind);
#endif /* _SIMULATION_ */
#endif /* FF_SAT_E */
#endif /* else SIM_TOOLKIT */

#define SIM_TIMEOUT(_x_) sim_timeout(_x_)

static void sim_chk_retry_failure (void);

/*==== VARIABLES ==================================================*/
#ifdef _SIMULATION_
static BOOL              first_access = TRUE;
#endif /* _SIMULATION_ */
static T_MONITOR         sim_mon;
static BOOL  sleepTimerRunning =FALSE;
extern BOOL startTimerPollOff; 

/*==== FUNCTIONS ==================================================*/

#ifdef FF_SAT_E
LOCAL void pei_dti_getdata_req(T_DTI2_GETDATA_REQ* dti_getdata_req)
{
  dti_dti_getdata_req(sim_data.hDTI, dti_getdata_req);
} /* pei_dti_getdata_req() */

LOCAL void pei_dti_disconnect_ind(T_DTI2_DISCONNECT_IND *dti_disconnect_ind)
{
    dti_dti_disconnect_ind (sim_data.hDTI, dti_disconnect_ind);
}/* pei_dti_disconnect_ind() */

LOCAL void pei_dti_ready_ind(T_DTI2_READY_IND* dti_ready_ind)
{
  dti_dti_ready_ind(sim_data.hDTI, dti_ready_ind);
} /* pei_dti_ready_ind() */

LOCAL void pei_dti_data_req(T_DTI2_DATA_REQ* dti_data_req)
{
  dti_dti_data_req(sim_data.hDTI, dti_data_req);
} /* pei_dti_data_req() */

LOCAL void pei_dti_data_ind(T_DTI2_DATA_IND* dti_data_ind)
{
  dti_dti_data_ind(sim_data.hDTI, dti_data_ind);
} /* pei_dti_data_ind() */

LOCAL void pei_dti_callback(U8 instance,
                            U8 interfac,
                            U8 channel,
                            U8 reason,
                            T_DTI2_DATA_IND* dti_data_ind)
{
  TRACE_FUNCTION ("pei_dti_callback()"); /*###jk:tbd*/
  TRACE_EVENT_P1("pei_dti_callback: reason = %2d", reason); /*###jk:tbd*/
  switch(reason)
  {
    case DTI_REASON_CONNECTION_OPENED:
      stk_dti_connection_opened();
      break;

    case DTI_REASON_CONNECTION_CLOSED:
      stk_dti_connection_closed();
      break;

    case DTI_REASON_DATA_RECEIVED:
      stk_dti_data_received(dti_data_ind);
      break;

    case DTI_REASON_TX_BUFFER_FULL:
      stk_dti_tx_buffer_full();
      break;

    case DTI_REASON_TX_BUFFER_READY:
      stk_dti_tx_buffer_ready();
      break;
  }
} /* pei_dti_callback() */

LOCAL void pei_dti_connect_ind (
                    T_DTI2_CONNECT_IND   *dti_connect_ind
                  )
{
    dti_dti_connect_ind(sim_data.hDTI, dti_connect_ind);
}

LOCAL void pei_dti_connect_cnf (
                    T_DTI2_CONNECT_CNF   *dti_connect_cnf
                  )
{
    dti_dti_connect_cnf(sim_data.hDTI, dti_connect_cnf);
}


#ifdef _SIMULATION_
LOCAL const void pei_dti_data_test_req(T_DTI2_DATA_TEST_REQ* dti_data_test_req)
{
  dti_dti_data_test_req(sim_data.hDTI, dti_data_test_req);
} /* pei_dti_data_test_req() */

LOCAL const void pei_dti_data_test_ind(T_DTI2_DATA_TEST_IND* dti_data_test_ind)
{
  dti_dti_data_test_ind(sim_data.hDTI, dti_data_test_ind);
} /* pei_dti_data_test_ind() */
#endif /* _SIMULATION_ */
#endif /* FF_SAT_E */

LOCAL const T_FUNC sim_table[] = {
  MAK_FUNC_0 (app_sim_read_req,          SIM_READ_REQ                ), /* 0x80004005 */
  MAK_FUNC_0 (app_sim_update_req,        SIM_UPDATE_REQ              ), /* 0x80014005 */
  MAK_FUNC_0 (app_sim_read_record_req,   SIM_READ_RECORD_REQ         ), /* 0x80024005 */
  MAK_FUNC_N (pei_not_supported,         0                           ), /* 0x80034005 */
  MAK_FUNC_0 (app_sim_update_record_req, SIM_UPDATE_RECORD_REQ       ), /* 0x80044005 */
  MAK_FUNC_N (pei_not_supported,         0                           ), /* 0x80054005 */
  MAK_FUNC_N (pei_not_supported,         0                           ), /* 0x80064005 */
#if defined SIM_TOOLKIT AND defined TI_PS_FF_AT_P_CMD_CUST
  MAK_FUNC_0 (stk_sim_refresh_user_res,       SIM_REFRESH_USER_RES        ), /*  */
#else /* SIM_TOOLKIT */
  MAK_FUNC_N (pei_not_supported,         0                           ), /* 0x80074005 */
#endif /* else SIM_TOOLKIT */
  MAK_FUNC_0 (app_sim_increment_req,     SIM_INCREMENT_REQ           ), /* 0x80084005 */
  MAK_FUNC_0 (app_sim_verify_pin_req,    SIM_VERIFY_PIN_REQ          ), /* 0x80094005 */
  MAK_FUNC_0 (app_sim_change_pin_req,    SIM_CHANGE_PIN_REQ          ), /* 0x800a4005 */
  MAK_FUNC_0 (app_sim_disable_pin_req,   SIM_DISABLE_PIN_REQ         ), /* 0x800b4005 */
  MAK_FUNC_0 (app_sim_enable_pin_req,    SIM_ENABLE_PIN_REQ          ), /* 0x800c4005 */
  MAK_FUNC_0 (app_sim_unblock_req,       SIM_UNBLOCK_REQ             ), /* 0x800d4005 */
  MAK_FUNC_0 (app_sim_auth_req,          SIM_AUTHENTICATION_REQ      ), /* 0x800e4005 */
  MAK_FUNC_0 (app_sim_mm_update_req,     SIM_MM_UPDATE_REQ           ), /* 0x800f4005 */
  MAK_FUNC_0 (app_sim_sync_req,          SIM_SYNC_REQ                ), /* 0x80104005 */
  MAK_FUNC_0 (app_sim_activate_req,      SIM_ACTIVATE_REQ            ), /* 0x80114005 */
#if defined SIM_TOOLKIT
  MAK_FUNC_0 (stk_sim_toolkit_req,       SIM_TOOLKIT_REQ             ), /* 0x80124005 */
  MAK_FUNC_0 (stk_sim_toolkit_res,       SIM_TOOLKIT_RES             ), /* 0x80134005 */
#else /* SIM_TOOLKIT */
  MAK_FUNC_N (pei_not_supported,         0                           ), /* 0x80124005 */
  MAK_FUNC_N (pei_not_supported,         0                           ), /* 0x80134005 */
#endif /* else SIM_TOOLKIT */
  MAK_FUNC_0 (app_sim_access_req,        SIM_ACCESS_REQ              ), /* 0x80144005 */
#if defined SIM_TOOLKIT
  MAK_FUNC_0 (stk_file_update_res,       SIM_FILE_UPDATE_RES         ), /* 0x80154005 */
#else /* SIM_TOOLKIT */
  MAK_FUNC_N (pei_not_supported,         0                           ), /* 0x80154005 */
#endif /* else SIM_TOOLKIT */
#if defined GPRS
  MAK_FUNC_0 (gprs_sim_gmm_update_req,   SIM_GMM_UPDATE_REQ          ), /* 0x80164005 */
#else /* GPRS */
  MAK_FUNC_N (pei_not_supported,         0                           ), /* 0x80164005 */
#endif /* else GPRS */
#if defined FF_SAT_E
  MAK_FUNC_0 (stk_sim_dti_req,           SIM_DTI_REQ                 ), /* 0x80174005 */
  MAK_FUNC_0 (stk_sim_bip_req,           SIM_BIP_REQ                 ), /* 0x80184005 */
  MAK_FUNC_0 (stk_sim_bip_config_req,    SIM_BIP_CONFIG_REQ          ), /* 0x80194005 */
  MAK_FUNC_0 (stk_sim_eventlist_req,     SIM_EVENTLIST_REQ           ), /* 0x801a4005 */
#else 
  MAK_FUNC_N (pei_not_supported,         0                           ), /* 0x80174005 */
  MAK_FUNC_N (pei_not_supported,         0                           ), /* 0x80184005 */
  MAK_FUNC_N (pei_not_supported,         0                           ), /* 0x80194005 */
  MAK_FUNC_N (pei_not_supported,         0                           ), /* 0x801a4005 */
#endif /* FF_SAT_E */
};

#ifdef FF_SAT_E
LOCAL const T_FUNC dti_dl_table[] =
{
  MAK_FUNC_0(pei_dti_connect_ind,        DTI2_CONNECT_IND),     /* 0x7750 */
  MAK_FUNC_0(pei_dti_connect_cnf,        DTI2_CONNECT_CNF),     /* 0x7751 */
  MAK_FUNC_0(pei_dti_disconnect_ind,     DTI2_DISCONNECT_IND),  /* 0x7752 */
  MAK_FUNC_0(pei_dti_ready_ind,          DTI2_READY_IND),       /* 0x7753 */
  MAK_FUNC_0(pei_dti_data_ind,           DTI2_DATA_IND),        /* 0x7754 */
#ifdef _SIMULATION_
  MAK_FUNC_S(pei_dti_data_test_ind,      DTI2_DATA_TEST_IND)    /* 0x7755 */
#else 
  MAK_FUNC_S(pei_not_supported,          DTI2_DATA_TEST_IND)    /* 0x7755 */
#endif /* _SIMULATION_ */
};

LOCAL const T_FUNC dti_ul_table[] =
{
  MAK_FUNC_0(pei_not_supported,     DTI2_CONNECT_REQ),      /* 0x3750 */
  MAK_FUNC_0(pei_not_supported,     DTI2_CONNECT_RES),      /* 0x3751 */
  MAK_FUNC_0(pei_not_supported,     DTI2_DISCONNECT_REQ ),  /* 0x3752 */
  MAK_FUNC_0(pei_dti_getdata_req,   DTI2_GETDATA_REQ),      /* 0x3753 */
  MAK_FUNC_0(pei_dti_data_req,      DTI2_DATA_REQ),         /* 0x3754 */
#ifdef _SIMULATION_
  MAK_FUNC_S(pei_dti_data_test_req, DTI2_DATA_TEST_REQ)     /* 0x3755 */
#else 
  MAK_FUNC_S(pei_not_supported,     DTI2_DATA_TEST_REQ)     /* 0x3755 */
#endif /*  _SIMULATION_ */
};

LOCAL const T_FUNC udp_table[] =
{
  MAK_FUNC_0(stk_udp_bind_cnf,      UDP_BIND_CNF),      /* 0x80004046 */
  MAK_FUNC_0(stk_udp_closeport_cnf, UDP_CLOSEPORT_CNF), /* 0x80014046 */
  MAK_FUNC_0(stk_udp_error_ind,     UDP_ERROR_IND),     /* 0x80024046 */
  MAK_FUNC_0(stk_udp_shutdown_ind,  UDP_SHUTDOWN_IND)   /* 0x80034046 */
};
#endif /* FF_SAT_E */

#ifdef FF_EM_MODE
LOCAL const T_FUNC em_table[] = {
  MAK_FUNC_N (pei_not_supported    , 0                ), /* 0x00 */
  MAK_FUNC_N (pei_not_supported    , 0                ), /* 0x01 */
  MAK_FUNC_N (pei_not_supported    , 0                ), /* 0x02 */
  MAK_FUNC_N (pei_not_supported    , 0                ), /* 0x03 */
  MAK_FUNC_N (pei_not_supported    , 0                ), /* 0x04 */
  MAK_FUNC_N (pei_not_supported    , 0                ), /* 0x05 */
  MAK_FUNC_N (pei_not_supported    , 0                ), /* 0x06 */
  MAK_FUNC_N (pei_not_supported    , 0                ), /* 0x07 */
  MAK_FUNC_N (pei_not_supported    , 0                ), /* 0x08 */
  MAK_FUNC_N (pei_not_supported    , 0                ), /* 0x09 */
  MAK_FUNC_N (pei_not_supported    , 0                ), /* 0x0A */
  MAK_FUNC_N (pei_not_supported    , 0                ), /* 0x0B */
  MAK_FUNC_N (pei_not_supported    , 0                ), /* 0x0C */
  MAK_FUNC_N (pei_not_supported    , 0                ), /* 0x0D */
  MAK_FUNC_N (pei_not_supported    , 0                ), /* 0x0E */
  MAK_FUNC_N (pei_not_supported    , 0                ), /* 0x0F */
  MAK_FUNC_0 (sim_em_sim_event_req , EM_SIM_EVENT_REQ )  /* 0x10 */
};
#endif /* FF_EM_MODE */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)              MODULE  : SIM_PEI             |
| STATE   : code                       ROUTINE : pei_primitive       |
+--------------------------------------------------------------------+

  PURPOSE : Process protocol specific primitive.

*/
LOCAL SHORT pei_primitive (void * ptr)
{
  T_PRIM *prim = ptr;

  /*
   *                |      |
   *               DTI2   SIM               UPLINK
   *                |      |
   *      +---------v------v---------+
   *      |                          |
   *      |            SIM           |
   *      |                          |
   *      +--------------------------+
   *
   */

  TRACE_FUNCTION ("pei_primitive()");

  if (prim NEQ NULL)
  {
    ULONG         opc = prim->custom.opc;
    USHORT        n, loc_sap_nr;
    const T_FUNC *table;

    VSI_PPM_REC ((T_PRIM_HEADER*)prim, __FILE__, __LINE__);

    PTRACE_IN (opc);
    loc_sap_nr = SAP_NR(opc);
    switch (loc_sap_nr)
    {
      case SAP_NR(SIM_DL): table = sim_table; n = TAB_SIZE (sim_table); break;
#ifdef FF_SAT_E
      case SAP_NR(UDP_DL): table = udp_table; n = TAB_SIZE (udp_table); break;
      case SAP_NR(DTI2_DL): table = dti_dl_table; n = TAB_SIZE (dti_dl_table);
        opc -= 0x50; /* DTI2 opcodes start at 0x50 */
      break;
      case SAP_NR(DTI2_UL): table = dti_ul_table; n = TAB_SIZE (dti_ul_table);
        opc -= 0x50; /* DTI2 opcodes start at 0x50 */
      break;
#endif /* FF_SAT_E */
#ifdef FF_EM_MODE
      case SAP_NR(EM_Ul): table = em_table; n = TAB_SIZE (em_table); break;
#endif /* FF_EM_MODE */
      default: table = NULL; n = 0; break;
    } /* switch (SAP_NR(opc)) */

    if (table != NULL )
    {
      if (PRIM_NR(opc) < n)
      {
        if(sleepTimerRunning == TRUE)
        {
          TIMER_STOP(sim_handle,SLEEP_TIMER);
          sleepTimerRunning = FALSE;
        }
       table += PRIM_NR(opc);
       JUMP(table->func)(P2D(prim));
#ifdef SIM_TOOLKIT
       if (sim_data.chk_sat_avail)
       {
         sim_data.chk_sat_avail = FALSE;
         stk_proactive_polling();
       }
#endif /* SIM_TOOLKIT */

        sleepTimerRunning = TRUE;
        TIMER_START(sim_handle,SLEEP_TIMER,100); /* no primitives in next 100 m secs go for Sleep*/
       sim_chk_retry_failure ();
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
    }
    return PEI_ERROR;
#endif
  }
  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)              MODULE  : SIM_PEI             |
| STATE   : code                       ROUTINE : pei_not_supported   |
+--------------------------------------------------------------------+

  PURPOSE : An unsupported primitive is received.

*/

static void pei_not_supported (void * data)
{
  TRACE_FUNCTION ("pei_not_supported()");

  PFREE (data);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)            MODULE  : SIM_PEI               |
| STATE   : code                     ROUTINE : sim_chk_retry_failure |
+--------------------------------------------------------------------+

  PURPOSE : check for driver retry failure to start SIM re-initialisation.

*/

void sim_chk_retry_failure (void)
{
#ifdef TI_PS_UICC_CHIPSET_15
  U8 readerId = SIMDRV_VAL_READER_ID__RANGE_MIN;
  U8 voltageSelect = SIMDRV_REQ_VOLTAGE_SEL;
#else   
  T_SIM_CARD sim_info;
#endif
  if (SIM_IS_FLAG_SET(DRV_FAILED_RETRY))
  {

    TRACE_EVENT("SIM RETRY FAILURE");

    if (SIM_IS_FLAG_CLEARED(CALL_ACTIVE))
    {
      PALLOC (sim_remove_ind_to_mmi, SIM_REMOVE_IND);
      sim_remove_ind_to_mmi->cause = SIM_CAUSE_DRV_TEMPFAIL;
      PSENDX (MMI, sim_remove_ind_to_mmi);

#ifndef TI_PS_UICC_CHIPSET_15
      SIM_PowerOff ();
#else
      simdrv_poweroff ( readerId );
#endif

      sim_data.act_directory = NOT_PRESENT_16BIT;
      sim_data.act_field     = NOT_PRESENT_16BIT;

      TIMER_START (sim_handle, SIM_TIMER, T_DETECT_VALUE);
      SIM_CLEAR_FLAG (SIM_INSERT);

#ifndef TI_PS_UICC_CHIPSET_15
      if (SIM_Reset (&sim_info) NEQ 0)
#else
      if (simdrv_reset( readerId, voltageSelect) NEQ 0)
#endif
      {
        TIMER_STOP (sim_handle, SIM_TIMER);
        app_sim_remove();
      }
    }
    else
    {
      SIM_CLEAR_FLAG (SIM_INSERT);
      app_sim_remove();
    }
    SIM_CLEAR_FLAG(DRV_FAILED_RETRY);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)              MODULE  : SIM_PEI             |
| STATE   : code                       ROUTINE : pei_init            |
+--------------------------------------------------------------------+

  PURPOSE : Initialize Protocol Stack Entity

*/
LOCAL SHORT pei_init (T_HANDLE handle)
{
  sim_handle = handle;

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

  if (hCommSMS < VSI_OK)
  {
    if ((hCommSMS = vsi_c_open (VSI_CALLER SMS_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

#ifdef _SIMULATION_
  if (hCommSIM < VSI_OK) /* get comhandle in order to be able to use the  */
  {                      /* Funktion vsi_c_await() in the sim_csf.c module */
    if ((hCommSIM = vsi_c_open (VSI_CALLER SIM_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif  /* _SIMULATION_ */
#endif /* TI_PS_HCOMM_CHANGE */

#if defined (GPRS)
  if (hCommGMM < VSI_OK)
  {
    if ((hCommGMM = vsi_c_open (VSI_CALLER GMM_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif

#ifdef SIM_TOOLKIT
#ifdef FF_SAT_E
  /*
   * DTI initialization
   */
  sim_data.hDTI = dti_init(1, handle, DTI_DEFAULT_OPTIONS, pei_dti_callback);
  hCommUDP = VSI_ERROR;
#endif /* FF_SAT_E */
  stk_init_sim_data();
#endif /* SIM_TOOLKIT */

  app_init_sim_data ();
  ccd_init ();
#if defined (_SIMULATION_)
  sim_data.mode = 0;
#endif

#ifdef FF_EM_MODE
  em_init_sim_event_trace();
#endif /* FF_EM_MODE */
  cl_shrd_init(sim_handle);
  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)              MODULE  : SIM_PEI             |
| STATE   : code                       ROUTINE : pei_timeout         |
+--------------------------------------------------------------------+

  PURPOSE : Process timeout

*/
LOCAL SHORT pei_timeout (USHORT index)
{
  U16 sleepResult;
  TRACE_EVENT_P1 ("[pei_timeout]: index  %d",index);
  SIM_TIMEOUT (index);
  return PEI_OK;
}
      
#ifdef FF_SAT_E
#ifdef _SIMULATION_
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)              MODULE  : SIM_PEI             |
| STATE   : code                       ROUTINE : csf_sim_pei_timeout |
+--------------------------------------------------------------------+

  PURPOSE : Process timeout for the _SIMULATION_ test cases. Purpose is to
            gain global access to the local function pei_timeout()

*/
GLOBAL USHORT csf_sim_pei_timeout (USHORT index)
{
  TRACE_FUNCTION ("csf_sim_pei_timeout(index)");
  TRACE_EVENT_P1 ("[csf_sim_pei_timeout]: index  %d",index);
  return pei_timeout (index);
}
#endif /* _SIMULATION_ */
#endif /* FF_SAT_E */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)              MODULE  : SIM_PEI             |
| STATE   : code                       ROUTINE : sim_timeout         |
+--------------------------------------------------------------------+

  PURPOSE : Process timeout

*/

static void sim_timeout (USHORT index)
{
  if (index EQ SIM_TIMER)
  {
    if(sleepTimerRunning == TRUE)
    {
      TIMER_STOP(sim_handle, SLEEP_TIMER);
    }
    sleepTimerRunning = FALSE;
    app_sim_timeout (SIM_TIMER);
  }
  else if(index EQ SLEEP_TIMER)
  { 
    sleepTimerRunning = FALSE;
    app_sim_timeout (SLEEP_TIMER); 
  } 
#ifdef SIM_TOOLKIT
  else
  {
    if(sleepTimerRunning == TRUE)
    {
      TIMER_STOP(sim_handle, SLEEP_TIMER);
    }
    sleepTimerRunning = FALSE;
    stk_timeout (index); 
  } 
#endif

  if((startTimerPollOff == TRUE)&&(sim_data.idle_polling != TRUE))
  {
     startTimerPollOff = FALSE;
     sleepTimerRunning = TRUE;
     TIMER_START(sim_handle,SLEEP_TIMER,35000);
     // this will not be effective as the SIM TIMER will expire before this and
     // stop it. Just to takecare of Sleep after the POLLING_OFF STK command
  } 
}


#ifdef _SIMULATION_
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)              MODULE  : SIM_PEI             |
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
#if defined (GPRS)
  vsi_c_close (VSI_CALLER hCommGMM);
  hCommGMM = VSI_ERROR;
#endif
#else /* for hCommHandles backward compatibility */
  vsi_c_close (VSI_CALLER hCommMMI);
  hCommMMI = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommMM);
  hCommMM = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommSMS);
  hCommSMS = VSI_ERROR;

#if defined (GPRS)
  vsi_c_close (VSI_CALLER hCommGMM);
  hCommGMM = VSI_ERROR;
#endif

#ifdef _SIMULATION_
  /*
   * SIM commhandle deinitialization -> was used for the test cases
   */
  if (hCommSIM >= VSI_OK)
  {
    vsi_c_close (VSI_CALLER hCommSIM);
  }
  hCommSIM = VSI_ERROR;
#endif /* _SIMULATION_ */
#endif /* TI_PS_HCOMM_CHANGE */



#ifdef FF_SAT_E
  /*
   * DTI deinitialization
   */
  dti_deinit(sim_data.hDTI);
  if (hCommUDP >= VSI_OK)
  {
    vsi_c_close (VSI_CALLER hCommUDP);
  }
  hCommUDP = VSI_ERROR;
#endif /* FF_SAT_E */
  cl_shrd_exit();

  return PEI_OK;
}
#endif /* _SIMULATION_ */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)              MODULE  : SIM_PEI             |
| STATE   : code                       ROUTINE : pei_config          |
+--------------------------------------------------------------------+

  PURPOSE : Dynamic Configuration

*/

/* Implements Measure#36 */
#ifndef NCONFIG
LOCAL const KW_DATA kwtab[] = {
#ifdef _SIMULATION_
                   SIM_MODE,              CFG_MODE,
#endif
                   SIM_REMOVE,            CFG_REMOVE,
                   SIM_NORM_POLL,         CFG_NORM_POLL,
                   SIM_TEST_POLL,         CFG_TEST_POLL,
                   "",                    0
                  };

LOCAL SHORT pei_config (T_PEI_CONFIG inString)
{
  char       * s = inString;
  char       * keyw;
  char       * val [10];

  TRACE_FUNCTION ("pei_config()");

  TRACE_FUNCTION (s);

  tok_init(s);

  /*
   * Parse next keyword and number of variables
   */
  while ((tok_next(&keyw,val)) NEQ TOK_EOCS)
  {
    switch (tok_key((KW_DATA *)kwtab,keyw))
    {
      default:
        TRACE_ERROR ("[PEI_CONFIG]: not supported");
        break;
      case TOK_NOT_FOUND:
        TRACE_ERROR ("[PEI_CONFIG]: illegal Keyword");
        break;
#if defined (_SIMULATION_)
      case CFG_MODE:
        sim_data.mode = atoi (val[0]);
        break;
#endif
      case CFG_REMOVE:
        app_sim_remove ();
        break;
      case CFG_NORM_POLL:
        SIM_CLEAR_FLAG (TEST_SIM_INSERTED);
        SIM_CLEAR_FLAG (TEST_MODE_POLLING);
        break;
#if defined SIM_TOOLKIT
      case CFG_TEST_POLL:
        SIM_SET_FLAG (TEST_SIM_INSERTED);
        SIM_SET_FLAG (TEST_MODE_POLLING);
        break;
#endif
    }
  }
  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : SIM_PEI             |
| STATE   : code                       ROUTINE : sim_pei_config      |
+--------------------------------------------------------------------+

  PURPOSE : Dynamic Configuration

*/
GLOBAL SHORT sim_pei_config ( char * inString, char * dummy )
{
  pei_config ( inString );

  return PEI_OK;
}
#endif /* not NCONFIG */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)              MODULE  : SIM_PEI             |
| STATE   : code                       ROUTINE : pei_monitor         |
+--------------------------------------------------------------------+

  PURPOSE : Monitoring of physical Parameters

*/
LOCAL SHORT pei_monitor (void ** monitor)
{
  TRACE_FUNCTION ("pei_monitor()");

/* Implements Measure#32: Row 15 */

  *monitor = &sim_mon;

  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)              MODULE  : SIM_PEI             |
| STATE   : code                       ROUTINE : pei_create          |
+--------------------------------------------------------------------+

  PURPOSE : Create the Protocol Stack Entity

*/

GLOBAL SHORT sim_pei_create (T_PEI_INFO **info)
{
  static const T_PEI_INFO pei_info =
  {
    "SIM",
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
    0x900,                  /* Stack Size      */
    10,                     /* Queue Entries   */
    176,                    /* Priority        */
    NUM_OF_SIM_TIMERS,      /* number of timer */
    PASSIVE_BODY |
    COPY_BY_REF |
    TRC_NO_SUSPEND |
    PRIM_NO_SUSPEND         /* flags           */
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

#ifdef _TARGET_
  /*
   * bind to SIM Driver supporting ATR provision
   */
 #ifndef TI_PS_UICC_CHIPSET_15
  SIM_lock_cr17689();
 #endif
#endif
  return PEI_OK;
}

#endif
