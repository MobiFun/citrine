/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_MMI
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
|  Purpose :  This module defines the processing functions for the
|             primitives send to the protocol stack adapter by the 
|             man machine interface.
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_MMIP_C
#define PSA_MMIP_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "aci.h"
#include "psa.h"
#include "psa_sms.h"
#include "psa_mmi.h"
#include "psa_mm.h"
#include "cmh.h"
#ifdef SIM_TOOLKIT
#include "psa_cc.h"     /* includes defs used in CMH_SAT.H */
#include "psa_sat.h"
#include "psa_sim.h"
#include "cmh_sat.h"
#endif /* of SIM_TOOLKIT */
#include "cmh_sms.h"
#include "cmh_mmi.h"
#include "hl_audio_drv.h"

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMIP                |
|                                 ROUTINE : psa_mmi_keypad_ind      |
+-------------------------------------------------------------------+

  PURPOSE : processes the MMI_KEYPAD_IND primitive send by MMI.

*/

GLOBAL  void psa_mmi_keypad_ind ( T_MMI_KEYPAD_IND *mmi_keypad_ind )
{
  TRACE_FUNCTION ("psa_mmi_keypad_ind()");

  /* update shared parameter and notify ACI */  
  mmiShrdPrm.keyCd = mmi_keypad_ind -> key_code;
  mmiShrdPrm.keySt = mmi_keypad_ind -> key_stat;

  TRACE_EVENT_P2("Key pressed: code: %d, status: %d", mmi_keypad_ind->key_code, mmi_keypad_ind->key_stat);
  cmhMMI_keyIndication();
  
  /* free the primitive buffer */  
  PFREE (mmi_keypad_ind);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMIP                |
|                                 ROUTINE : psa_mmi_cbch_ind        |
+-------------------------------------------------------------------+

  PURPOSE : processes the MMI_CBCH_IND primitive send by MMI.

*/

GLOBAL  void psa_mmi_cbch_ind ( T_MMI_CBCH_IND *mmi_cbch_ind )
{
  TRACE_FUNCTION ("psa_mmi_cbch_ind()");

  if (mmShrdPrm.regStat EQ RS_FULL_SRV)
  {
    cmhSMS_CBMIndication (mmi_cbch_ind);
  }

  /*
   *-------------------------------------------------------------------
   * free the primitive buffer
   *-------------------------------------------------------------------
   */  
  PFREE (mmi_cbch_ind);
}

#ifdef SIM_TOOLKIT
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMIP                |
|                                 ROUTINE : psa_mmi_cbch_dnl_ind    |
+-------------------------------------------------------------------+

  PURPOSE : processes the MMI_CBCH_DWNLD_IND primitive send by MMI.

*/

GLOBAL  void psa_sat_cbch_dnl_ind ( T_MMI_SAT_CBCH_DWNLD_IND *mmi_cbch_dnl_ind )
{
  TRACE_FUNCTION ("psa_sat_cbch_dnl_ind()");

  /*
   *-----------------------------------------------------------------
   * check if message should be transfered to SIM for SAT module
   *-----------------------------------------------------------------
   */
  if (psaSIM_ChkSIMSrvSup( SRV_DtaDownlCB )) /* check if servive allocated and enabled */
  {

    cmhSAT_DatDwnLdCB ( mmi_cbch_dnl_ind -> cbch_msg,
                        mmi_cbch_dnl_ind -> cbch_len); /* build envelope */
  }

  /*
   *-------------------------------------------------------------------
   * free the primitive buffer
   *-------------------------------------------------------------------
   */  
  PFREE (mmi_cbch_dnl_ind);
}
#endif

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMIP                |
|                                 ROUTINE : psa_mmi_rxlev_ind       |
+-------------------------------------------------------------------+

  PURPOSE : processes the MMI_RXLEV_IND primitive send by MMI.

*/

GLOBAL  void psa_mmi_rxlev_ind ( T_MMI_RXLEV_IND *mmi_rxlev_ind )
{

  TRACE_FUNCTION ("psa_mmi_rxlev_ind()");

  /* update shared parameter and notify ACI */  
  mmiShrdPrm.rxLev = mmi_rxlev_ind -> rxlev;

  cmhMMI_rxIndication();

  /* free the primitive buffer */  
  PFREE (mmi_rxlev_ind);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMIP                |
|                                 ROUTINE : psa_mmi_battery_ind     |
+-------------------------------------------------------------------+

  PURPOSE : processes the MMI_BATTERY_IND primitive send by MMI.

*/

GLOBAL  void psa_mmi_battery_ind ( T_MMI_BATTERY_IND *mmi_battery_ind )
{
  TRACE_FUNCTION ("psa_mmi_battery_ind()");

  /* update shared parameter and notify ACI */  
  mmiShrdPrm.btLev = mmi_battery_ind -> volt;

  cmhMMI_btIndication();

  /* free the primitive buffer */  
  PFREE (mmi_battery_ind);
}

#ifdef BTE_MOBILE
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMIP                |
|                                 ROUTINE : psa_mmi_keypad_ind      |
+-------------------------------------------------------------------+

  PURPOSE : processes the MMI_KEYPAD_IND primitive send by MMI.

*/

#if defined (MFW)
extern mfw_bt_cb_notify_rxd(void);
#endif

GLOBAL  void psa_mmi_bt_cb_notify_ind (T_MMI_BT_CB_NOTIFY_IND *mmi_bt_cb_notify_ind )
{
  TRACE_FUNCTION ("psa_mmi_bt_cb_notify_ind()");

  /*
  ** Do something!
  */
  #if defined (MFW)
  mfw_bt_cb_notify_rxd();
  #endif

  /* free the primitive buffer */  
  PFREE (mmi_bt_cb_notify_ind);

}
#endif


#ifndef VOCODER_FUNC_INTERFACE
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMIP                |
|                                 ROUTINE : psa_mmi_keypad_ind      |
+-------------------------------------------------------------------+

  PURPOSE : processes the MMI_KEYPAD_IND primitive send by MMI.

*/

GLOBAL  void psa_mmi_tch_vocoder_cfg_con(T_MMI_TCH_VOCODER_CFG_CON *mmi_tch_vocoder_cfg_con)
{
  TRACE_FUNCTION("psa_mmi_tch_vocoder_cfg_con()");
  hl_drv_vocoder_state_set();
  PFREE (mmi_tch_vocoder_cfg_con);
}
#endif 

/*==== EOF =========================================================*/
