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
|  Purpose :  Dynamic Configuration for Radio Resource
+-----------------------------------------------------------------------------
*/

#ifndef CNF_RR_H
#define CNF_RR_H

/*==== CONSTANTS ==================================================*/
/*
 * CONFIGURATION PARAMETER
 *
 * Description :  The constants define the commands for dynamic
 *                configuration proposals.
 */

#define  RR_TIMER_SET       "TIMER_SET"
#define  RR_TIMER_RESET     "TIMER_RESET"
#define  RR_TIMER_SPEED_UP  "TIMER_SPEED_UP"
#define  RR_TIMER_SLOW_DOWN "TIMER_SLOW_DOWN"
#define  RR_TIMER_SUPPRESS  "TIMER_SUPPRESS"

#define  RR_NO_SYS_TIME     "NO_SYS_TIME"
#define  RR_SET_LAST_USED_SC "SET_LAST_USED_SC"
#define  RR_INIT_FFS        "INIT_FFS"
#define  RR_WRITE_FFS       "WRITE_FFS"
#define  RR_FCR             "FCR"
#define  RR_IHO             "IHO"
#define  RR_SCR             "SCR"
#define  RR_DLE             "DLE"
#define  RR_FCA             "FCA"
#define  RR_FRL             "FRL"
#define  RR_FHO             "FHO"
#define  RR_GSM_OFFSET      "GSM_OFFSET"
#define  RR_DCS_OFFSET      "DCS_OFFSET"
#define  RR_CTO             "CTO"
#define  RR_NKC             "NKC"

#define  RR_MT_CALL_NAME    "MTC"
#define  RR_MT_SMS_0_NAME   "MT_SMS_0"
#define  RR_MT_SMS_2_NAME   "MT_SMS_2"
#define  RR_SRV_FULL_NAME   "FULL_SERVICE"
#define  RR_SRV_LIM_NAME    "LIMITED_SERVICE"
#define  RR_SRV_NO          "NO_SERVICE"
#define  RR_PSEUDO_SYNC_HO  "PSEUDO_SYNC_HO"
#define  RR_PCM             "PCM"
#define  RR_DCS_PCLASS_3   "DCS_PCLASS_3"

#define  RR_RESTRICTED_BAND "SET_BAND"
#define  RR_MULTISLOT_CLASS "MULTISLOT_CLASS"
#define  RR_CMSP            "CMSP"

#define  RR_ERASE_BL        "ERASE_BL"
#define  RR_ERASE_WL        "ERASE_WL"
#define  RR_SET_WL          "SET_WL"
#define  RR_SET_WL_PLMN     "SET_WL_PLMN"
#define  RR_SET_BL          "SET_BL"
#define  RR_SET_WL_REGION   "SET_WL_REGION"
#define  RR_SHOW_BL         "SHOW_BL"
#define  RR_SHOW_WL         "SHOW_WL"
#define  RR_SET_NPS_DELAY   "SET_NPS_DELAY"

#if defined(_TARGET_)
#define  RR_FFS_CHECK       "FFS_CHECK"
#endif /* _TARGET_ */
#define  RR_SCS             "SCS"
#define  RR_SHIELD          "SHIELD"
#define  RR_BL_CS           "BL_CS"
#define  RR_U_RXT           "U_RXT"
#define  RR_M_RXT           "M_RXT"
#define  RR_L_RXT           "L_RXT"
#define  RR_TFAST_CS        "TIM_FAST"
#define  RR_TNORMAL_CS      "TIM_NORMAL"
#define  RR_FBLS            "FBLS"

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

#endif
