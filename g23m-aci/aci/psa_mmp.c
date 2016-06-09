/*  
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_MM
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
|             registrations part of mobility management.
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_MMP_C
#define PSA_MMP_C
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
#include "psa_mm.h"
#include "cmh.h"
#include "cmh_mm.h"

#include "psa_cc.h"
#include "psa_sim.h"

#ifdef FF_TIMEZONE
#include "rv/rv_general.h"
#include "rtc/rtc_tz.h"
#endif

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/

#ifdef FF_EM_MODE
EXTERN SHORT em_relcs;
#endif /* FF_EM_MODE */

/*==== VARIABLES ==================================================*/

EXTERN T_PCEER causeMod;
EXTERN SHORT causeCeer;

/*==== FUNCTIONS ==================================================*/


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMP                 |
|                                 ROUTINE : psa_mmr_reg_cnf         |
+-------------------------------------------------------------------+

  PURPOSE : processes the MMR_REG_CNF primitive send by MM. 
            this confirms a successful network registration by
            passing the selected network description.

*/

GLOBAL  void psa_mmr_reg_cnf ( T_MMR_REG_CNF *mmr_reg_cnf )
{

  TRACE_FUNCTION ("psa_mmr_reg_cnf()");


  /* Reg Cnf will be sent when RR starts the power scanning 
     and hence ignore it */
  if (mmr_reg_cnf->bootup_cause EQ PWR_SCAN_START)
  {
    TRACE_EVENT("Dummy cnf from the lower layer");
    PFREE (mmr_reg_cnf);
    return;
  }

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */
  mmShrdPrm.usedPLMN  = mmr_reg_cnf -> plmn;
  mmShrdPrm.regStat   = RS_FULL_SRV;
  mmShrdPrm.lac       = mmr_reg_cnf->lac;
  mmShrdPrm.cid       = mmr_reg_cnf->cid;

  causeMod  = P_CEER_mod;      /* Clear module which was set for ceer */
  causeCeer = CEER_NotPresent; /* Reset extended error cause */

  TRACE_EVENT_P2("MMR_REG_CNF received: LAC: %04X, CID: %04X", mmShrdPrm.lac, mmShrdPrm.cid);

  psaCC_send_satevent( EVENT_LOC_STATUS, -1, NEAR_END, FALSE );

  /* Check whether any read request has been sent to SIM */
  if(cmhMM_OpCheckName())  /*EONS: retrieves network name from SIM if needed.*/
  {
    cmhMM_Registered ();
  }

  /* free the primitive buffer */
  PFREE (mmr_reg_cnf);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMP                 |
|                                 ROUTINE : psa_mmr_nreg_cnf        |
+-------------------------------------------------------------------+

  PURPOSE : processes the MMR_NREG_CNF primitive send by MM.
      this confirms a successful network deregistration.

*/

GLOBAL  void psa_mmr_nreg_cnf ( T_MMR_NREG_CNF *mmr_nreg_cnf )
{

  TRACE_FUNCTION ("psa_mmr_nreg_cnf()");

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */
  mmShrdPrm.deregCs = mmr_nreg_cnf -> detach_cause;

  if( mmShrdPrm.deregCs EQ CS_SIM_REM )
    mmShrdPrm.regStat = RS_LMTD_SRV;
  else
    mmShrdPrm.regStat = RS_NO_SRV;

  mmShrdPrm.usedPLMN.v_plmn = INVLD_PLMN;

  psaCC_send_satevent( EVENT_LOC_STATUS, -1, NEAR_END, FALSE );

  cmhMM_Deregistered ();

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE(mmr_nreg_cnf);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMP                 |
|                                 ROUTINE : psa_mmr_nreg_ind        |
+-------------------------------------------------------------------+

  PURPOSE : processes the MMR_NREG_IND primitive send by MM.
            this indicates the loss of network registration by
            passing the cause.

*/

GLOBAL  void psa_mmr_nreg_ind ( T_MMR_NREG_IND *mmr_nreg_ind )
{

  TRACE_FUNCTION ("psa_mmr_nreg_ind()");

/*
 *-------------------------------------------------------------------
 *  update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */
  mmShrdPrm.deregCs   = mmr_nreg_ind -> service;
  causeMod  = P_CEER_mm;      /* Set module for extended error */
  causeCeer = mmr_nreg_ind -> cause; /* Set extended error cause */

#ifdef FF_EM_MODE
  em_relcs = mmr_nreg_ind -> service;
#endif /* FF_EM_MODE */

  switch( mmr_nreg_ind -> service )
  {
    case( NREG_LIMITED_SERVICE ):
      mmShrdPrm.regStat         = RS_LMTD_SRV;
      mmShrdPrm.usedPLMN.v_plmn = INVLD_PLMN;
      break;
    case( NREG_NO_SERVICE ):
      mmShrdPrm.regStat         = RS_NO_SRV;
      mmShrdPrm.usedPLMN.v_plmn = INVLD_PLMN;
      break;
    default:

#ifndef GPRS
      TRACE_EVENT_P1 ("UNEXP NREG SERVICE = %4x", mmr_nreg_ind -> service);
#endif  /* GPRS */

      mmShrdPrm.regStat = RS_NO_SRV;
  }

  psaCC_send_satevent( EVENT_LOC_STATUS, -1, NEAR_END, FALSE );
  
  if (!mmr_nreg_ind->search_running)
    cmhMM_Deregistered ();

  /* free the primitive buffer */
  PFREE(mmr_nreg_ind);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMP                 |
|                                 ROUTINE : psa_mmr_plmn_ind        |
+-------------------------------------------------------------------+

  PURPOSE : processes the MMR_PLMN_IND primitive send by MM.
            this indicates the network search result und a list of
            found plmn's.

*/

GLOBAL  void psa_mmr_plmn_ind ( T_MMR_PLMN_IND *mmr_plmn_ind )
{
  TRACE_FUNCTION ("psa_mmr_plmn_ind()");

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */
  mmShrdPrm.srchRslt = mmr_plmn_ind -> cause;

  switch( mmr_plmn_ind -> cause )
  {
    case MMCS_SUCCESS:
      psaMM_CpyPLMNLst (mmr_plmn_ind -> plmn, mmr_plmn_ind -> forb_ind, mmr_plmn_ind ->lac_list);
      if (psaSIM_ChkSIMSrvSup(SRV_PNN) AND psaSIM_ChkSIMSrvSup(SRV_OPL))
        cmhMM_OpSetPNNLst(); /*EONS: reads PNN list from SIM, cmhMM_NetworkLst() is called from there!*/
      else
        cmhMM_NetworkLst();
      break;

    case MMCS_PLMN_NOT_IDLE_MODE:
    case MMCS_SIM_REMOVED:
      cmhMM_SelNetwork (mmr_plmn_ind -> cause);
      break;

    default:
      TRACE_EVENT_P1 ("UNEXP NET RSLT = %4x", mmr_plmn_ind -> cause);
      cmhMM_SelNetwork (mmr_plmn_ind -> cause);
      break;
  }

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE(mmr_plmn_ind);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMP                 |
|                                 ROUTINE : convert_mmr_to_rtc_tz   |
+-------------------------------------------------------------------+

  PURPOSE : convert the time zone received with the MMR_INFO_IND primitive 
  into the normal time zone according to the special coding algorithm.
  
  GSM03.40 and search for the TP-Service-Centre-Time Stamp: 
  The Time Zone indicates the difference, expressed in quarters 
  of an hour, between the local time and GMT. In the first
  of the two semi-octets, the first bit (bit 3 of the seventh 
  octet of the TP-Service-Centre-Time-Stamp field) represents the
  algebraic sign of this difference (0 : positive, 1 : negative).
  
  +1:00 hour  =  +4 quarter hours = 0x04 but Nibble Swap => 0x40
  +8:00 hours = +32 quarter hours = 0x32 but Nibble Swap => 0x23
  -2:45 hours = -11 quarter hours = 0x91 but Nibble Swap => 0x19

*/
#ifdef FF_TIMEZONE
LOCAL unsigned int convert_mmr_to_rtc_tz (U8 mmr_timezone)
{
  unsigned int absolute_value = (10*(mmr_timezone & 0x7))+((mmr_timezone >> 4) & 0xf); 
  if ((mmr_timezone & 0x08))
  {
    absolute_value = ~absolute_value+1;
  }
  return (absolute_value);
}
#endif /* FF_TIMEZONE */

#if defined FF_TIMEZONE AND defined _SIMULATION_
/* The below two functions have been added for Simulation as a Stub for NITZ feature */
LOCAL T_RV_RET RTC_SetCurrentTZ(T_RTC_TZ currentTimeZone)
{
  if (currentTimeZone<RTC_GMT_NT_1200 OR currentTimeZone>RTC_GMT_PT_1200)
  {
     return RV_INVALID_PARAMETER;
  }
  return RV_OK;
}


LOCAL T_RTC_TZ RTC_GetCurrentTZ(void)
{
  T_RTC_TZ timezone = +4; /* as per values given in test case */
  return timezone;
}
#endif /* FF_TIMEZONE AND _SIMULATION_ */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMP                 |
|                                 ROUTINE : psa_mmr_info_ind        |
+-------------------------------------------------------------------+

  PURPOSE : processes the MMR_INFO_IND primitive send by MM.

*/

GLOBAL  void psa_mmr_info_ind ( T_MMR_INFO_IND *mmr_info_ind )
{
  T_ACI_CMD_SRC srcId;
#ifdef FF_TIMEZONE
  T_RTC_TZ rtc_timezone = RTC_TIME_ERROR;
  T_RV_RET rtc_return   = RV_INTERNAL_ERR;
  BOOL     tz_update    = FALSE;
#endif

  TRACE_FUNCTION ("psa_mmr_info_ind()");

  if (mmr_info_ind->ntz.v_tz)
  {
    mmShrdPrm.tz = mmr_info_ind->ntz.tz;
#ifdef FF_TIMEZONE
    rtc_timezone = (T_RTC_TZ) convert_mmr_to_rtc_tz(mmr_info_ind->ntz.tz);
#endif
  }
  else
  {
    TRACE_EVENT("psa_mmr_info_ind(): timezone invalid");
  }

  /* Check whether any of the ACI command sources have requested
     to be updated, and update if required. */
  for ( srcId = CMD_SRC_LCL; srcId < CMD_SRC_MAX; srcId++)
  {
#ifndef FF_TIMEZONE
  if ((cmhPrm[srcId].mmCmdPrm.CTZRMode EQ CTZR_MODE_ON) AND 
      mmr_info_ind->ntz.v_tz)
  {
    R_AT(RAT_CTZV,srcId)(&mmr_info_ind->ntz.tz);
  }
#else
    /*the following if statement is for time zone reporting*/
    if ((cmhPrm[srcId].mmCmdPrm.CTZRMode EQ CTZR_MODE_ON) AND 
         mmr_info_ind->ntz.v_tz)
    {
      R_AT(RAT_CTZV,srcId)(rtc_timezone);      
    }
    /*the following if statement is for time and date reporting*/
    if ((cmhPrm[srcId].mmCmdPrm.PCTZVMode EQ PCTZV_MODE_ON) AND 

         mmr_info_ind->time.v_time)
    {
      R_AT(RAT_P_CTZV, srcId)(mmr_info_ind, rtc_timezone);
    }
    /*the following if statements are for network name reporting*/
    if (cmhPrm[srcId].mmCmdPrm.CNIVMode EQ CNIV_MODE_ON)
    {
      R_AT(RAT_P_CNIV,srcId)(mmr_info_ind);
    }
    if (cmhPrm[srcId].mmCmdPrm.CTZUMode EQ CTZU_MODE_ON)
    {
      tz_update = TRUE;
    } 
#endif /* FF_TINEZONE */
  }
  
#ifdef FF_TIMEZONE
  if (tz_update AND mmr_info_ind->ntz.v_tz AND rtc_timezone NEQ RTC_TIME_ERROR)
  {
    rtc_return = RTC_SetCurrentTZ(rtc_timezone);
    if (rtc_return NEQ RV_OK)
    {
      TRACE_ERROR ("RTC setting failed...");
    }
    else
    {
      if (rtc_timezone EQ RTC_GetCurrentTZ())
      {
        TRACE_EVENT_P1("psa_mmr_info_ind(): timezone successfully set to %d", rtc_timezone);         
      }
    }
  }
#endif /* FF_TINEZONE */

  /*
   * free the primitive buffer
   */ 
  PFREE(mmr_info_ind);
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMP                 |
|                                 ROUTINE : psa_mmr_ciphering_ind   |
+-------------------------------------------------------------------+

  PURPOSE : processes the MMR_CIPHERING_IND primitive send by MM.
*/

GLOBAL  void psa_mmr_ciphering_ind
                           ( T_MMR_CIPHERING_IND *mmr_ciphering_ind )
{

  TRACE_FUNCTION ("psa_mmr_ciphering_ind()");

  cmhMM_CipheringInd(mmr_ciphering_ind->ciph);

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE(mmr_ciphering_ind);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMP                 |
|                                 ROUTINE : psa_mmr_ahplmn_ind   |
+-------------------------------------------------------------------+

  PURPOSE : processes the MMR_AHPLMN_IND primitive send by MM.
*/
GLOBAL  void psa_mmr_ahplmn_ind(T_MMR_AHPLMN_IND *mmr_ahplmn_ind)
{

  TRACE_FUNCTION("psa_mmr_ahplmn_ind()");

  mmShrdPrm.ActingHPLMN = mmr_ahplmn_ind->ahplmn;

  cmhMM_Registered ();

  PFREE(mmr_ahplmn_ind);

}


/*==== EOF =========================================================*/
